
/*
 *@@sourcefile filedlg.c:
 *      replacement file dlg with full WPS support.
 *
 *      The entry point is fdlgFileDialog, which has the
 *      same syntax as WinFileDlg and should return about
 *      the same values. 100% compatibility cannot be
 *      achieved because of window subclassing and all those
 *      things.
 *
 *      This file is ALL new with V0.9.9.
 *
 *      Function prefix for this file:
 *      --  fdlr*
 *
 *@@added V0.9.9 (2001-03-11) [umoeller]
 *@@header "filesys\filedlg.h"
 */

/*
 *      Copyright (C) 2001-2003 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#pragma strings(readonly)

/*
 *  Suggested #include order:
 *  1)  os2.h
 *  2)  C library headers
 *  3)  setup.h (code generation and debugging options)
 *  4)  headers in helpers\
 *  5)  at least one SOM implementation header (*.ih)
 *  6)  dlgids.h, headers in shared\ (as needed)
 *  7)  headers in implementation dirs (e.g. filesys\, as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSERRORS
#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WININPUT
#define INCL_WINTIMER
#define INCL_WINPOINTERS
#define INCL_WINDIALOGS
#define INCL_WINBUTTONS
#define INCL_WINSTATICS
#define INCL_WINENTRYFIELDS
#define INCL_WINLISTBOXES
#define INCL_WINSTDFILE
#define INCL_WINSTDCNR
#define INCL_WINSHELLDATA       // Prf* functions
#define INCL_WINSYS

#define INCL_GPILOGCOLORTABLE
#define INCL_GPIBITMAPS
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h
#include <ctype.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\threads.h"            // thread helpers
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
#include "xfldr.ih"
#include "xfdisk.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\cnrsort.h"             // container sort comparison functions
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\kernel.h"              // XWorkplace Kernel

#include "filesys\filedlg.h"            // replacement file dialog implementation
#include "filesys\folder.h"             // XFolder implementation
#include "filesys\fdrsubclass.h"        // folder subclassing engine
#include "filesys\fdrviews.h"           // common code for folder views
#include "filesys\fdrsplit.h"           // folder split views

// other SOM headers
#pragma hdrstop                         // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Declarations
 *
 ********************************************************************/

const char *WC_FILEDLGCLIENT    = "XWPFileDlgClient";

#define IDDI_TYPESTXT           100
#define IDDI_TYPESCOMBO         101
                // directory statics:
#define IDDI_DIRTXT             102
#define IDDI_DIRVALUE           103
                // file static/entryfield:
#define IDDI_FILETXT            104
#define IDDI_FILEENTRY          105

/*
 *@@ FILEDLGDATA:
 *      central "window data" structure allocated on
 *      the stack in fdlgFileDlg. The main frame and
 *      the main client receive pointers to this
 *      and store them in QWL_USER.
 *
 *      This pretty much holds all the data the
 *      file dlg needs while it is running.
 *
 *@@changed V0.9.16 (2001-10-19) [umoeller]: added UNC support
 */

typedef struct _FILEDLGDATA
{
    // ptr to original FILEDLG structure passed into fdlgFileDlg
    PFILEDLG        pfd;

    SPLITCONTROLLER ctl;

    LINKLIST        llDisks;            // linked list of all WPDisk* objects
                                        // so that we can quickly find them for updating
                                        // the dialog; no auto-free

    // controls in hwndMainControl

    HWND
                    hwndTreeCnrTxt,     // child of sv.hwndMainControl (static text above cnr)
                    hwndFilesCnrTxt,    // child of sv.hwndMainControl (static text above cnr)
                    // types combo:
                    hwndTypesTxt,
                    hwndTypesCombo,
                    // directory statics:
                    hwndDirTxt,
                    hwndDirValue,
                    // file static/entryfield:
                    hwndFileTxt,
                    hwndFileEntry,
                    // buttons:
                    hwndOK,
                    hwndCancel,
                    hwndHelp;

    LINKLIST        llDialogControls;       // list of dialog controls for focus etc.

    // full file name etc., parsed and set by ParseFileString()
    CHAR        szDrive[CCHMAXPATH];        // e.g. "C:" if local drive,
                                            // or "\\SERVER\RESOURCE" if UNC
    BOOL        fUNCDrive;                  // TRUE if szDrive specifies something UNC
    CHAR        szDir[CCHMAXPATH],          // e.g. "\whatever\subdir"; a null string
                                            // if we're in the root directory
                szFileMask[CCHMAXPATH],     // e.g. "*.TXT"
                szFileName[CCHMAXPATH];     // e.g. "test.txt"

} FILEDLGDATA, *PFILEDLGDATA;

MRESULT EXPENTRY fnwpSubclassedFilesFrame(HWND hwndFrame, ULONG msg, MPARAM mp1, MPARAM mp2);

/* ******************************************************************
 *
 *   Helper funcs
 *
 ********************************************************************/

#define FFL_DRIVE           0x0001
#define FFL_PATH            0x0002
#define FFL_FILEMASK        0x0004
#define FFL_FILENAME        0x0008

/*
 *@@ ParseFileString:
 *      parses the given full-file string and
 *      sets up the members in pWinData accordingly.
 *
 *      This returns a ULONG bitset with any combination
 *      of the following:
 *
 *      -- FFL_DRIVE: drive (or UNC resource) was specified,
 *         and FILEDLGDATA.ulLogicalDrive has been updated.
 *
 *      -- FFL_PATH: directory was specified,
 *         and FILEDLGDATA.szDir has been updated.
 *
 *      -- FFL_FILEMASK: a file mask was given ('*' or '?'
 *         characters found in filename), and
 *         FILEDLGDATA.szFileMask has been updated.
 *         FILEDLGDATA.szFileName is set to a null byte.
 *         This never comes with FFL_FILENAME.
 *
 *      -- FFL_FILENAME: a file name without wildcards was
 *         given, and FILEDLGDATA.szFileName has been updated.
 *         FILEDLGDATA.is unchanged.
 *         This never comes with FFL_FILEMASK.
 *
 *@@changed V0.9.18 (2002-02-06) [umoeller]: fixed a bunch of bugs
 *@@changed V0.9.19 (2002-06-15) [umoeller]: fixed broken file detection (double-clicks)
 */

STATIC ULONG ParseFileString(PFILEDLGDATA pWinData,
                             const char *pcszFullFile)
{
    ULONG   flChanged = 0;

    PCSZ    p = pcszFullFile;
    ULONG   ulDriveSpecLen = 0;
    APIRET  arc2;

    PMPF_SPLITVIEW(("entering"));

    if (    (!pcszFullFile)
         || (!*pcszFullFile)
       )
        return 0;

    PMPF_SPLITVIEW(("parsing %s", pcszFullFile));

    if (!(arc2 = doshGetDriveSpec(pcszFullFile,
                                  pWinData->szDrive,
                                  &ulDriveSpecLen,
                                  &pWinData->fUNCDrive)))
    {
        // drive specified (local or UNC):
        p += ulDriveSpecLen,
        flChanged |= FFL_DRIVE;
    }

    PMPF_SPLITVIEW(("  doshGetDriveSpec returned %d, len: %d",
            arc2, ulDriveSpecLen));

    if (    (arc2)
            // continue if no drive spec given
         && (arc2 != ERROR_INVALID_PARAMETER)
       )
        // some error was detected:
        return 0;

    // get path from there
    if (!*p)
    {
        // nothing after the drive spec (that is, something like "K:"
        // was specified: make that a root directory
        // V1.0.0 (2002-09-13) [umoeller]
        pWinData->szDir[0] = '\0';
        flChanged |= FFL_PATH;

        PMPF_SPLITVIEW(("  szDrive \"%s\", szDir \"%s\"",
                                    pWinData->szDrive,
                                    pWinData->szDir));
    }
    else
    {
        // something else after the drive spec:
        BOOL    fMustBeDir = FALSE; // V0.9.18 (2002-02-06) [umoeller]

        // p3 = last backslash
        PCSZ    pStartOfFile;
        PCSZ    p3;
        if (p3 = strrchr(p, '\\'))
        {
            // path specified:
            // @@todo handle relative paths here
            ULONG cb = p3 - p;

            PMPF_SPLITVIEW(("  checking if path \"%s\" is dir", p));

            // check if the last character is a '\';
            // the spec _must_ be a directory then
            if (p[cb] == '\0')      // fixed V0.9.19 (2002-06-15) [umoeller]
            {
                PMPF_SPLITVIEW(("  ends in \\, must be dir"));
                fMustBeDir = TRUE;
                // p2++;           // points to null char now --> no file spec
            }

            strhncpy0(pWinData->szDir,
                      p,        // start: either first char or after drive
                      cb);
            PMPF_SPLITVIEW(("  got path %s", pWinData->szDir));
            flChanged |= FFL_PATH;
            pStartOfFile = p3 + 1;      // after the last backslash
        }
        else
            // no path specified:
            pStartOfFile = p;

        PMPF_SPLITVIEW(("  pStartOfFile is \"%s\"", pStartOfFile));

        // check if the following is a file mask
        // or a real file name
        if (    (strchr(pStartOfFile, '*'))
             || (strchr(pStartOfFile, '?'))
           )
        {
            // get file name (mask) after that
            strlcpy(pWinData->szFileMask,
                    pStartOfFile,
                    sizeof(pWinData->szFileMask));

            PMPF_SPLITVIEW(("  new mask is %s", pWinData->szFileMask));
            flChanged |= FFL_FILEMASK;
        }
        else
        {
            // name only:

            // compose full file name
            CHAR szFull[CCHMAXPATH];
            FILESTATUS3 fs3;
            BOOL fIsDir = FALSE;
            PSZ pszThis = szFull;

            pszThis += sprintf(pszThis,
                               "%s%s",
                               pWinData->szDrive,      // either C: or \\SERVER\RESOURCE
                               pWinData->szDir);
            if (*pStartOfFile)
                // we have a file spec left:
                pszThis += sprintf(pszThis,
                                   "\\%s",
                                   pStartOfFile);        // entry

            PMPF_SPLITVIEW(("   checking %s", szFull));
            if (!(arc2 = DosQueryPathInfo(szFull,
                                          FIL_STANDARD,
                                          &fs3,
                                          sizeof(fs3))))
            {
                // this thing exists:
                // is it a file or a directory?
                if (fs3.attrFile & FILE_DIRECTORY)
                    fIsDir = TRUE;
            }

            PMPF_SPLITVIEW(("   DosQueryPathInfo rc = %d, fIsDir %d, fMustBeDir %d",
                        arc2, fIsDir, fMustBeDir));

            if (fIsDir)
            {
                // user specified directory:
                // append to existing and say "path changed"
                if (*pStartOfFile)
                {
                    strcat(pWinData->szDir, "\\");
                    strcat(pWinData->szDir, pStartOfFile);
                }
                PMPF_SPLITVIEW(("  new path is %s", pWinData->szDir));
                flChanged |= FFL_PATH;
            }
            else
            {
                // this is not a directory:
                if (fMustBeDir)
                {
                    // but it must be (because user terminated string with "\"):
                    PMPF_SPLITVIEW(("  not dir, but must be!"));
                }
                else
                {
                    // this doesn't exist, or it is a file:
                    if (*pStartOfFile)
                    {
                        // and it has a length: V0.9.18 (2002-02-06) [umoeller]
                        strlcpy(pWinData->szFileName,
                                pStartOfFile,
                                sizeof(pWinData->szFileName));
                        PMPF_SPLITVIEW(("  new filename is %s", pWinData->szFileName));
                        flChanged |= FFL_FILENAME;
                    }
                }
            }
        }
    }

    PMPF_SPLITVIEW(("leaving, returning %s %s %s %s",
                            (flChanged & FFL_DRIVE) ? "FFL_DRIVE" : "",
                            (flChanged & FFL_PATH) ? "FFL_PATH" : "",
                            (flChanged & FFL_FILEMASK) ? "FFL_FILEMASK" : "",
                            (flChanged & FFL_FILENAME) ? "FFL_FILENAME" : ""
                          ));

    return flChanged;
}

/*
 *@@ UpdateDlgWithFullFile:
 *      updates the dialog according to the
 *      current directory/path/file fields
 *      in pWinData.
 *
 *      Returns TRUE if we already initiated
 *      the full populate of a folder.
 *
 *@@changed V0.9.18 (2002-02-06) [umoeller]: mostly rewritten for better thread synchronization
 */

STATIC BOOL UpdateDlgWithFullFile(PFILEDLGDATA pWinData)
{
    PMINIRECORDCORE precDiskSelect = NULL;
    WPFolder        *pRootFolder = NULL;
    BOOL            brc = FALSE;

    if (pWinData->fUNCDrive)
    {
         // @@todo
    }
    else
    {
        // we currently have a local drive:
        // go thru the disks list and find the WPDisk*
        // which matches the current logical drive
        PLISTNODE pNode = lstQueryFirstNode(&pWinData->llDisks);

        PMPF_SPLITVIEW(("pWinData->szDrive = %s",
               pWinData->szDrive));

        while (pNode)
        {
            WPDisk *pDisk = (WPDisk*)pNode->pItemData;

            if (_wpQueryLogicalDrive(pDisk) == pWinData->szDrive[0] - 'A' + 1)
            {
                precDiskSelect = _wpQueryCoreRecord(pDisk);
                pRootFolder = _XFldDisk    // V1.0.5 (2006-06-10) [pr]: fix crash
                              ? _xwpSafeQueryRootFolder(pDisk, FALSE, NULL)
                              : _wpQueryRootFolder(pDisk);
                break;
            }

            pNode = pNode->pNext;
        }

        PMPF_SPLITVIEW(("    precDisk = 0x%lX", precDiskSelect));
    }

    if ((precDiskSelect) && (pRootFolder))
    {
        // we got a valid disk and root folder:
        WPFolder *pFullFolder;

        CHAR szFull[CCHMAXPATH];

        // populate and expand the current disk
        fdrPostFillFolder(&pWinData->ctl,
                          precDiskSelect,
                          FFL_FOLDERSONLY | FFL_EXPAND);

        sprintf(szFull,
                "%s%s",
                pWinData->szDrive,      // C: or \\SERVER\RESOURCE
                pWinData->szDir);
        pFullFolder = _wpclsQueryFolder(_WPFolder, szFull, TRUE);
                    // this also awakes all folders in between
                    // the root folder and the full folder

        // now go for all the path particles, starting with the
        // root folder
        if (pFullFolder)
        {
            CHAR szComponent[CCHMAXPATH];
            PMINIRECORDCORE precParent = precDiskSelect; // start with disk
            WPFolder *pFdrThis = pRootFolder;
            const char *pcThis = &pWinData->szDir[1];   // start after root '\'

            PMPF_SPLITVIEW(("    got folder %s for %s",
                    _wpQueryTitle(pFullFolder),
                    szFull));

            while (    (pFdrThis)
                    && (*pcThis)
                  )
            {
                const char *pBacksl = strchr(pcThis, '\\');
                WPFileSystem *pobj;

                PMPF_SPLITVIEW(("       remaining: %s", pcThis));

                if (!pBacksl)
                    strlcpy(szComponent, pcThis, sizeof(szComponent));
                else
                {
                    ULONG c = (pBacksl - pcThis);
                    memcpy(szComponent,
                           pcThis,
                           c);
                    szComponent[c] = '\0';
                }

                // now szComponent contains the current
                // path component;
                // e.g. if szDir was "F:\OS2\BOOK", we now have "OS2"

                PMPF_SPLITVIEW(("       checking component %s", szComponent));

                // find this component in pFdrThis
                if (    (pobj = fdrSafeFindFSFromName(pFdrThis,
                                                      szComponent))
                     && (_somIsA(pobj, _WPFolder))
                   )
                {
                    // got that folder:
                    POINTL ptlIcon = {0, 0};
                    PMINIRECORDCORE pNew;
                    ULONG fl;

                    PMPF_SPLITVIEW(("        -> got %s", _wpQueryTitle(pobj)));

                    pNew = _wpCnrInsertObject(pobj,
                                              pWinData->ctl.cvTree.hwndCnr,
                                              &ptlIcon,
                                              precParent,  // parent == previous folder
                                              NULL); // next available position

                    if (!pBacksl)
                    {
                        // this was the last component: then
                        // populate fully and scroll to the thing
                        fl = FFL_SCROLLTO;
                        precDiskSelect = NULL;
                        // tell caller we fully populated
                        brc = TRUE;
                    }
                    else
                    {
                        // not the last component:
                        // then we'll need to expand the thing
                        // and add the first child to each subfolder
                        fl = FFL_FOLDERSONLY | FFL_EXPAND;
                    }

                    if (pNew)
                    {
                        fdrPostFillFolder(&pWinData->ctl,
                                          pNew,
                                          fl);

                        precParent = pNew;
                        // precDiskSelect = precParent;
                    }
                    else
                        break;

                    if (pBacksl)
                    {
                        // OK, go on with that folder
                        pFdrThis = pobj;
                        pcThis = pBacksl + 1;
                    }
                    else
                        break;
                }
                else
                    break;
            } // while (pFdrThis && *pcThis)

        } // if (pFullFolder)
    } // if ((precSelect) && (pRootFolder))

    if (precDiskSelect)
    {
        // this only happens if a root drive was selected...
        // then we still need to fully populate and scroll:
        fdrPostFillFolder(&pWinData->ctl,
                          precDiskSelect,
                          FFL_SCROLLTO);
        // tell caller we fully populated
        brc = TRUE;
    }

    PMPF_SPLITVIEW(("exiting"));

    return brc;
}

/*
 *@@ ParseAndUpdate:
 *      parses the specified new file or file
 *      mask and updates the display.
 *
 *      Gets called on startup with the initial
 *      directory, file, and file mask passed to
 *      the dialog, and later whenever the user
 *      enters something in the entry field.
 *
 *      If pcszFullFile contains a full file name,
 *      we post WM_SYSCOMMAND + SC_CLOSE to the
 *      frame to dismiss the dialog.
 */

STATIC VOID ParseAndUpdate(PFILEDLGDATA pWinData,
                           const char *pcszFullFile)
{
    // parse the new file string
    ULONG   fl = ParseFileString(pWinData,
                                 pcszFullFile);

    if (fl & FFL_FILENAME)
    {
        // no wildcard, but file specified:
        pWinData->pfd->lReturn = DID_OK;

        // close the damn thing
        WinPostMsg(pWinData->ctl.hwndMainFrame,
                   WM_SYSCOMMAND,
                   (MPARAM)SC_CLOSE,
                   0);

        // PM frame proc then posts WM_CLOSE to
        // the client (the split controller),
        // which sends WM_CONTROL with SN_FRAMECLOSE
        // back to the frame; the main msg loop in
        // fdlgFileDlg detects that and exits
        // V1.0.0 (2002-09-13) [umoeller]

        // now get outta here
        return;
    }

    if (fl & (FFL_DRIVE | FFL_PATH | FFL_FILEMASK))
    {
        // drive or path specified:
        // expand that

        // set this to NULL so that main control will refresh
        pWinData->ctl.precTreeSelected
            = pWinData->ctl.precFilesShowing = NULL;

        UpdateDlgWithFullFile(pWinData);
    }
}

/*
 *@@ BuildDisksList:
 *      builds a linked list of all WPDisk* objects
 *      in pRootFolder, which better be the real
 *      "Drives" folder.
 */

STATIC VOID BuildDisksList(WPFolder *pRootFolder,
                           PLINKLIST pllDisks)
{
    if (fdrCheckIfPopulated(pRootFolder,
                            FALSE))     // folders only?
    {
        BOOL fFolderLocked = FALSE;

        TRY_LOUD(excpt1)
        {
            if (fFolderLocked = !_wpRequestFolderMutexSem(pRootFolder, SEM_INDEFINITE_WAIT))
            {
                WPObject *pObject;
                // 1) count objects
                // V0.9.20 (2002-07-31) [umoeller]: now using get_pobjNext SOM attribute
                for (   pObject = _wpQueryContent(pRootFolder, NULL, QC_FIRST);
                        (pObject);
                        pObject = *__get_pobjNext(pObject)
                    )
                {
                    if (_somIsA(pObject, _WPDisk))
                        lstAppendItem(pllDisks, pObject);
                }
            }
        }
        CATCH(excpt1) {} END_CATCH();

        if (fFolderLocked)
            _wpReleaseFolderMutexSem(pRootFolder);
    }
}

/* ******************************************************************
 *
 *   File dialog frame
 *
 ********************************************************************/

/*
 *@@ CreateControlFont:
 *
 *@@added V1.0.0 (2002-09-13) [umoeller]
 */

HWND CreateControlFont(HWND hwndParent,
                       HWND hwndOwner,
                       PCSZ pcszClass,
                       PCSZ pcszTitle,
                       ULONG flStyle,
                       ULONG ulID)
{
    HWND hwnd;
    if (hwnd = winhCreateControl(hwndParent,
                                 hwndOwner,
                                 pcszClass,
                                 pcszTitle,
                                 flStyle,
                                 ulID))
        winhSetWindowFont(hwnd,
                          cmnQueryDefaultFont());

    return hwnd;
}

/*
 *@@ FrameCreateControls:
 *
 *@@changed V1.0.0 (2002-08-21) [umoeller]: extracted fdrSetupSplitView
 */

STATIC VOID FrameCreateControls(PFILEDLGDATA pWinData)
{
    PFILEDLG    pfd = pWinData->pfd;
    PCSZ        pcszOKButton;

    // create static on top of left tree view
    pWinData->hwndTreeCnrTxt
        = CreateControlFont(pWinData->ctl.hwndMainControl,           // parent
                            pWinData->ctl.hwndMainFrame,
                            WC_STATIC,
                            cmnGetString(ID_XFSI_FDLG_DRIVES),
                            WS_VISIBLE | SS_TEXT | DT_LEFT | DT_VCENTER | DT_MNEMONIC,
                            IDDI_TYPESTXT);
    lstAppendItem(&pWinData->llDialogControls, (PVOID)pWinData->hwndTreeCnrTxt);

    // append container next (tab order!)
    lstAppendItem(&pWinData->llDialogControls, (PVOID)pWinData->ctl.cvTree.hwndCnr);

    // create static on top of right files cnr
    pWinData->hwndFilesCnrTxt
        = CreateControlFont(pWinData->ctl.hwndMainControl,           // parent
                            pWinData->ctl.hwndMainFrame,
                            WC_STATIC,
                            cmnGetString(ID_XFSI_FDLG_FILESLIST),
                            WS_VISIBLE | SS_TEXT | DT_RIGHT | DT_VCENTER | DT_MNEMONIC,
                            IDDI_TYPESTXT);
    lstAppendItem(&pWinData->llDialogControls, (PVOID)pWinData->hwndFilesCnrTxt);

    // append container next (tab order!)
    lstAppendItem(&pWinData->llDialogControls, (PVOID)pWinData->ctl.cvFiles.hwndCnr);

    /*
     *  create the other controls
     *
     */

    pWinData->hwndTypesTxt
        = CreateControlFont(pWinData->ctl.hwndMainControl,           // parent
                            pWinData->ctl.hwndMainFrame,
                            WC_STATIC,
                            cmnGetString(ID_XFSI_FDLG_TYPES),
                            WS_VISIBLE | SS_TEXT | DT_LEFT | DT_VCENTER | DT_MNEMONIC,
                            IDDI_TYPESTXT);
    lstAppendItem(&pWinData->llDialogControls, (PVOID)pWinData->hwndTypesTxt);

    pWinData->hwndTypesCombo
        = CreateControlFont(pWinData->ctl.hwndMainControl,           // parent
                            pWinData->ctl.hwndMainFrame,
                            WC_ENTRYFIELD,
                            "",
                            WS_VISIBLE | ES_LEFT | ES_AUTOSCROLL | ES_MARGIN,
                            IDDI_TYPESCOMBO);
    ctlComboFromEntryField(pWinData->hwndTypesCombo,
                           CBS_DROPDOWNLIST);
    lstAppendItem(&pWinData->llDialogControls, (PVOID)pWinData->hwndTypesCombo);

    pWinData->hwndDirTxt
        = CreateControlFont(pWinData->ctl.hwndMainControl,           // parent
                            pWinData->ctl.hwndMainFrame,
                            WC_STATIC,
                            cmnGetString(ID_XFSI_FDLG_DIRECTORY),
                            WS_VISIBLE | SS_TEXT | DT_LEFT | DT_VCENTER | DT_MNEMONIC,
                            IDDI_DIRTXT);

    pWinData->hwndDirValue
        = CreateControlFont(pWinData->ctl.hwndMainControl,           // parent
                            pWinData->ctl.hwndMainFrame,
                            WC_STATIC,
                            cmnGetString(ID_XFSI_FDLG_WORKING),
                            WS_VISIBLE | SS_TEXT | DT_LEFT | DT_VCENTER,
                            IDDI_DIRVALUE);

    pWinData->hwndFileTxt
        = CreateControlFont(pWinData->ctl.hwndMainControl,           // parent
                            pWinData->ctl.hwndMainFrame,
                            WC_STATIC,
                            cmnGetString(ID_XFSI_FDLG_FILE),
                            WS_VISIBLE | SS_TEXT | DT_LEFT | DT_VCENTER | DT_MNEMONIC,
                            IDDI_FILETXT);
    lstAppendItem(&pWinData->llDialogControls, (PVOID)pWinData->hwndFileTxt);

    pWinData->hwndFileEntry
        = CreateControlFont(pWinData->ctl.hwndMainControl,           // parent
                            pWinData->ctl.hwndMainFrame,
                            WC_ENTRYFIELD,
                            "",             // initial text... we set this later
                            WS_VISIBLE | ES_LEFT | ES_AUTOSCROLL | ES_MARGIN,
                            IDDI_FILEENTRY);
    winhSetEntryFieldLimit(pWinData->hwndFileEntry, CCHMAXPATH - 1);
    lstAppendItem(&pWinData->llDialogControls, (PVOID)pWinData->hwndFileEntry);

    // text for the OK button depends
    if (!(pcszOKButton = pfd->pszOKButton))
        // not specified by caller:
        // use "Open" or "Save" then ("OK" is meaningless, if you ask me)
        // V1.0.0 (2002-09-13) [umoeller]
        pcszOKButton = cmnGetString( (pfd->fl & FDS_SAVEAS_DIALOG)
                                        ? DID_SAVE
                                        : DID_OPEN);

    pWinData->hwndOK
        = CreateControlFont(pWinData->ctl.hwndMainControl,           // parent
                            pWinData->ctl.hwndMainFrame,
                            WC_BUTTON,
                            (pfd->pszOKButton)
                              ? pfd->pszOKButton
                              : cmnGetString(DID_OK),
                            WS_VISIBLE | BS_PUSHBUTTON | BS_DEFAULT,
                            DID_OK);
    lstAppendItem(&pWinData->llDialogControls, (PVOID)pWinData->hwndOK);

    pWinData->hwndCancel
        = CreateControlFont(pWinData->ctl.hwndMainControl,           // parent
                            pWinData->ctl.hwndMainFrame,
                            WC_BUTTON,
                            cmnGetString(DID_CANCEL),
                            WS_VISIBLE | BS_PUSHBUTTON,
                            DID_CANCEL);
    lstAppendItem(&pWinData->llDialogControls, (PVOID)pWinData->hwndCancel);

    if (pfd->fl & FDS_HELPBUTTON)
    {
        pWinData->hwndHelp
            = CreateControlFont(pWinData->ctl.hwndMainControl,           // parent
                                pWinData->ctl.hwndMainFrame,
                                WC_BUTTON,
                                cmnGetString(DID_HELP),
                                WS_VISIBLE | BS_PUSHBUTTON | BS_HELP,
                                DID_HELP);
        lstAppendItem(&pWinData->llDialogControls, (PVOID)pWinData->hwndHelp);
    }
}

/*
 *@@ FrameChar:
 *      implementation for WM_CHAR in fnwpMainControl.
 *
 *@@added V0.9.9 (2001-03-13) [umoeller]
 */

STATIC MRESULT FrameChar(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    BOOL brc = FALSE;               // not processed
    PFILEDLGDATA pWinData = WinQueryWindowPtr(hwnd, QWL_USER);

    if (pWinData)
    {
        USHORT usFlags    = SHORT1FROMMP(mp1);
        USHORT usch       = SHORT1FROMMP(mp2);
        USHORT usvk       = SHORT2FROMMP(mp2);

        // key down msg?
        if ((usFlags & KC_KEYUP) == 0)
        {
            PMPF_SPLITVIEW(("usFlags = 0x%lX, usch = %d, usvk = %d",
                        usFlags, usch, usvk));

            if (usFlags & KC_VIRTUALKEY)
            {
                switch (usvk)
                {
                    case VK_TAB:
                        // find next focus window
                        dlghSetNextFocus(&pWinData->llDialogControls);
                    break;

                    case VK_BACKTAB:
                        // note: shift+tab produces this!!
                        dlghSetPrevFocus(&pWinData->llDialogControls);
                    break;

                    case VK_BACKSPACE:
                    {
                        // get current selection in drives view
                        PMINIRECORDCORE prec = (PMINIRECORDCORE)WinSendMsg(
                                                pWinData->ctl.cvTree.hwndCnr,
                                                CM_QUERYRECORD,
                                                (MPARAM)pWinData->ctl.precTreeSelected,
                                                MPFROM2SHORT(CMA_PARENT,
                                                             CMA_ITEMORDER));
                        if (prec)
                            // not at root already:
                            cnrhSelectRecord(pWinData->ctl.cvTree.hwndCnr,
                                             prec,
                                             TRUE);
                    }
                    break;

                    case VK_ESC:
                        WinPostMsg(hwnd,
                                   WM_COMMAND,
                                   (MPARAM)DID_CANCEL,
                                   0);
                    break;

                    case VK_NEWLINE:        // this comes from the main key
                    case VK_ENTER:          // this comes from the numeric keypad
                        dlghEnter(&pWinData->llDialogControls);
                    break;
                } // end switch
            } // end if (usFlags & KC_VIRTUALKEY)
            else
            {
                // no virtual key:
                // find the control for the keypress
                dlghProcessMnemonic(&pWinData->llDialogControls, usch);
            }
        }
    }

    return (MPARAM)brc;
}

/*
 *@@ FrameRepositionControls:
 *      part of the implementation of WM_SIZE in
 *      fnwpMainControl. This resizes all subwindows
 *      of the main client -- the split window and
 *      the controls on bottom.
 *
 *      Repositioning the split window will in turn
 *      cause the two container frames to be adjusted.
 */

STATIC VOID FrameRepositionControls(PFILEDLGDATA pWinData)
{
    #define OUTER_SPACING       5

    #define BUTTON_WIDTH        100
    #define BUTTON_HEIGHT       30
    #define BUTTON_SPACING      10

    #define STATICS_HEIGHT      20
    #define STATICS_LEFTWIDTH   100

    #define SPACE_BOTTOM        (BUTTON_HEIGHT + OUTER_SPACING \
                                 + 3*STATICS_HEIGHT + 3*OUTER_SPACING)
    SWP     aswp[12];       // three buttons
                            // plus two for types
                            // plus two statics for directory
                            // plus two controls for file
                            // plus split window
    ULONG   i = 0;
    SWP     swpMainControl;
    SHORT   cx, cy;

    PMPF_SPLITVIEW(("entering"));

    memset(aswp, 0, sizeof(aswp));

    WinQueryWindowPos(pWinData->ctl.hwndMainControl,
                      &swpMainControl);

    cx = swpMainControl.cx;
    cy = swpMainControl.cy;

    // "Drives" static on top:
    aswp[i].fl = SWP_MOVE | SWP_SIZE;
    aswp[i].x = OUTER_SPACING;
    aswp[i].y =    cy                   // new win cy
                 - OUTER_SPACING
                 - STATICS_HEIGHT;
    aswp[i].cx =   (cx                  // new win cx
                        - (2 * OUTER_SPACING))
                   / 2;
    aswp[i].cy = STATICS_HEIGHT;
    aswp[i++].hwnd = pWinData->hwndTreeCnrTxt;

    // "Files list" static on top:
    aswp[i].fl = SWP_MOVE | SWP_SIZE;
    aswp[i].x =   OUTER_SPACING
                + (cx               // new win cx
                        - (2 * OUTER_SPACING))
                   / 2;
    aswp[i].y =    cy     // new win cy
                 - OUTER_SPACING
                 - STATICS_HEIGHT;
    aswp[i].cx =   (cx    // new win cx
                        - (2 * OUTER_SPACING))
                   / 2;
    aswp[i].cy = STATICS_HEIGHT;
    aswp[i++].hwnd = pWinData->hwndFilesCnrTxt;

    // split window
    aswp[i].fl = SWP_MOVE | SWP_SIZE;
    aswp[i].x = OUTER_SPACING;
    aswp[i].y = OUTER_SPACING + SPACE_BOTTOM;
    aswp[i].cx =   cx   // new win cx
                 - 2 * OUTER_SPACING;
    aswp[i].cy = cy     // new win cy
                 - 3 * OUTER_SPACING
                 - STATICS_HEIGHT
                 - SPACE_BOTTOM;
    aswp[i++].hwnd = pWinData->ctl.hwndSplitWindow;

    // "Types:" static
    aswp[i].fl = SWP_MOVE | SWP_SIZE;
    aswp[i].x = OUTER_SPACING;
    aswp[i].y = BUTTON_HEIGHT + 4 * OUTER_SPACING + 2 * STATICS_HEIGHT;
    aswp[i].cx = STATICS_LEFTWIDTH;
    aswp[i].cy = STATICS_HEIGHT;
    aswp[i++].hwnd = pWinData->hwndTypesTxt;

    // types combobox:
    aswp[i].fl = SWP_MOVE | SWP_SIZE | SWP_ZORDER;
    aswp[i].x = 2*OUTER_SPACING + STATICS_LEFTWIDTH;
    aswp[i].y = BUTTON_HEIGHT + 4 * OUTER_SPACING + 2 * STATICS_HEIGHT;
    aswp[i].cx = cx   // new win cx
                    - (3*OUTER_SPACING + STATICS_LEFTWIDTH);
    aswp[i].cy = STATICS_HEIGHT;
    aswp[i].hwndInsertBehind = HWND_TOP;
    aswp[i++].hwnd = pWinData->hwndTypesCombo;

    // "Directory:" static
    aswp[i].fl = SWP_MOVE | SWP_SIZE;
    aswp[i].x = OUTER_SPACING;
    aswp[i].y = BUTTON_HEIGHT + 3 * OUTER_SPACING + STATICS_HEIGHT;
    aswp[i].cx = STATICS_LEFTWIDTH;
    aswp[i].cy = STATICS_HEIGHT;
    aswp[i++].hwnd = pWinData->hwndDirTxt;

    // directory value static
    aswp[i].fl = SWP_MOVE | SWP_SIZE;
    aswp[i].x = 2*OUTER_SPACING + STATICS_LEFTWIDTH;
    aswp[i].y = BUTTON_HEIGHT + 3 * OUTER_SPACING + STATICS_HEIGHT;
    aswp[i].cx = cx   // new win cx
                    - (3*OUTER_SPACING + STATICS_LEFTWIDTH);
    aswp[i].cy = STATICS_HEIGHT;
    aswp[i++].hwnd = pWinData->hwndDirValue;

    // "File:" static
    aswp[i].fl = SWP_MOVE | SWP_SIZE;
    aswp[i].x = OUTER_SPACING;
    aswp[i].y = BUTTON_HEIGHT + 2 * OUTER_SPACING;
    aswp[i].cx = STATICS_LEFTWIDTH;
    aswp[i].cy = STATICS_HEIGHT;
    aswp[i++].hwnd = pWinData->hwndFileTxt;

    // file entry field
    aswp[i].fl = SWP_MOVE | SWP_SIZE;
    aswp[i].x = 2*OUTER_SPACING + STATICS_LEFTWIDTH;
    aswp[i].y = BUTTON_HEIGHT + 2 * OUTER_SPACING + 2;
    aswp[i].cx = cx   // new win cx
                    - (3*OUTER_SPACING + STATICS_LEFTWIDTH);
    aswp[i].cy = STATICS_HEIGHT - 2 * 2;
    aswp[i++].hwnd = pWinData->hwndFileEntry;

    // "OK" button
    aswp[i].fl = SWP_MOVE | SWP_SIZE;
    aswp[i].x = OUTER_SPACING;
    aswp[i].y = OUTER_SPACING;
    aswp[i].cx = BUTTON_WIDTH;
    aswp[i].cy = BUTTON_HEIGHT;
    aswp[i++].hwnd = pWinData->hwndOK;

    // "Cancel" button
    aswp[i].fl = SWP_MOVE | SWP_SIZE;
    aswp[i].x = OUTER_SPACING + BUTTON_SPACING + BUTTON_WIDTH;
    aswp[i].y = OUTER_SPACING;
    aswp[i].cx = BUTTON_WIDTH;
    aswp[i].cy = BUTTON_HEIGHT;
    aswp[i++].hwnd = pWinData->hwndCancel;

    // "Help" button
    if (pWinData->hwndHelp)
    {
        aswp[i].fl = SWP_MOVE | SWP_SIZE;
        aswp[i].x = OUTER_SPACING + 2*BUTTON_SPACING + 2*BUTTON_WIDTH;
        aswp[i].y = OUTER_SPACING;
        aswp[i].cx = BUTTON_WIDTH;
        aswp[i].cy = BUTTON_HEIGHT;
        aswp[i++].hwnd = pWinData->hwndHelp;
    }

    if (!WinSetMultWindowPos(pWinData->ctl.habGUI,
                             aswp,
                             i))
        PMPF_SPLITVIEW(("WinSetMultWindowPos failed!!"));

    PMPF_SPLITVIEW(("leaving"));
}

/*
 *@@ FrameClientControl:
 *      implementation for WM_CONTROL from FID_CLIENT in
 *      fnwpFileDlgFrame.
 *
 *@@added V1.0.0 (2002-11-24) [umoeller]
 */

STATIC MRESULT FrameClientControl(HWND hwndFrame, PFILEDLGDATA pWinData, MPARAM mp1, MPARAM mp2)
{

    MRESULT mrc = 0;

    CHAR szTemp[CCHMAXPATH];

    switch (SHORT2FROMMP(mp1))
    {
        case SN_FOLDERCHANGING:
            if (mp2)
            {
                // say "populating"
                PMPF_SPLITVIEW(("SN_FOLDERCHANGING"));
                WinSetWindowText(pWinData->hwndDirValue,
                                 cmnGetString(ID_XFSI_FDLG_WORKING));

                // update our file path etc.
                if (_wpQueryFilename((WPFolder*)mp2,
                                     szTemp,
                                     TRUE))
                {
                    ParseFileString(pWinData,
                                    szTemp);
                }
            }
            else
                // populate failed:
                WinSetWindowText(pWinData->hwndDirValue, "");

        break;

        case SN_FOLDERCHANGED:
        {
            PMPF_SPLITVIEW(("SN_FOLDERCHANGED"));
            sprintf(szTemp,
                    "%s%s\\%s",
                    pWinData->szDrive,
                    pWinData->szDir,
                    pWinData->szFileMask);

            WinSetWindowText(pWinData->hwndDirValue,
                             szTemp);
        }
        break;

        case SN_OBJECTSELECTED:
        case SN_OBJECTENTER:
        {
            WPFileSystem *pobjFS;

            // this resolves shadows
            if (    (pobjFS = fdrvGetFSFromRecord((PMINIRECORDCORE)mp2,
                                                  FALSE))
                 && (pWinData = WinQueryWindowPtr(hwndFrame, QWL_USER))
               )
            {
                // if this points to some other folder, paste
                // the fully qualified filename (files and folders)
                _wpQueryFilename(pobjFS,
                                 szTemp,
                                 (_wpQueryFolder(pobjFS) != pWinData->ctl.psfvFiles->somSelf));

                // set entry field to filename only if the target
                // object is not a folder or we'll overwrite a
                // "save as" filename every time the user clicks
                // on a folder
                if (!_somIsA(pobjFS, _WPFolder))
                    WinSetWindowText(pWinData->hwndFileEntry,
                                     szTemp);

                // on double-clicks, refresh the dialog
                // (as if "OK" was clicked)
                if (SHORT2FROMMP(mp1) == SN_OBJECTENTER)
                    ParseAndUpdate(pWinData,
                                   szTemp);
            }

            // prevent the split view from opening the object
            mrc = (MRESULT)TRUE;
        }
        break;

            /* WinPostMsg(hwndFrame,
                       WM_COMMAND,
                       (MPARAM)DID_OK,
                       NULL); */

        case SN_VKEY:
        {
            mrc = (MRESULT)TRUE;

            switch ((USHORT)mp2)
            {
                case VK_TAB:
                    // find next focus window
                    dlghSetNextFocus(&pWinData->llDialogControls);
                break;

                case VK_BACKTAB:
                    // note: shift+tab produces this!!
                    dlghSetPrevFocus(&pWinData->llDialogControls);
                break;

                case VK_ESC:
                    WinPostMsg(hwndFrame,
                               WM_COMMAND,
                               (MPARAM)DID_CANCEL,
                               0);
                break;

                case VK_NEWLINE:        // this comes from the main key
                case VK_ENTER:          // this comes from the numeric keypad
                    dlghEnter(&pWinData->llDialogControls);
                break;

                default:
                    mrc = (MRESULT)FALSE;       // not processed
            } // end switch
        }
        break;

        // SN_FRAMECLOSE gets handled by main msg loop
    }

    return mrc;
}

/*
 *@@ fnwpFileDlgFrame:
 *
 *@@added V1.0.0 (2002-08-21) [umoeller]
 */

MRESULT EXPENTRY fnwpFileDlgFrame(HWND hwndFrame, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    PFILEDLGDATA pWinData;

    if (pWinData = WinQueryWindowPtr(hwndFrame, QWL_USER))
    {
        BOOL    fCallDefault = FALSE;

        switch (msg)
        {
            /*
             * WM_WINDOWPOSCHANGED:
             *      since we have set SPLIT_NOAUTOPOSITION, we
             *      must manually position everything.
             */

            case WM_WINDOWPOSCHANGED:
                mrc = pWinData->ctl.xfc.pfnwpOrig(hwndFrame, msg, mp1, mp2);

                if (((PSWP)mp1)->fl & SWP_SIZE)
                    FrameRepositionControls(pWinData);
            break;

            /*
             * WM_CONTROL:
             *      controller wants to tell us something.
             */

            case WM_CONTROL:
                if (SHORT1FROMMP(mp1) == FID_CLIENT)
                    mrc = FrameClientControl(hwndFrame, pWinData, mp1, mp2);
            break;

            /*
             * WM_COMMAND:
             *      button pressed.
             */

            case WM_COMMAND:
                if (    (pWinData = WinQueryWindowPtr(hwndFrame, QWL_USER))
                     && (pWinData->ctl.fSplitViewReady)
                   )
                    switch ((ULONG)mp1)
                    {
                        case DID_OK:
                        {
                            // "OK" is tricky... we first need to
                            // get the thing from the entry field
                            // and check if it contains a wildcard.
                            // For some reason people have become
                            // accustomed to this.
                            PSZ pszFullFile;

                            PMPF_SPLITVIEW(("DID_OK"));

                            if (pszFullFile = winhQueryWindowText(pWinData->hwndFileEntry))
                            {
                                // simulate the case that the user entered
                                // something into the entry field; if the
                                // user double-clicked on a file, this will
                                // dismiss the dialog, otherwise we'll expand
                                // the tree to show the folder
                                ParseAndUpdate(pWinData,
                                               pszFullFile);

                                free(pszFullFile);
                            }

                        }
                        break;

                        case DID_CANCEL:

                            PMPF_SPLITVIEW(("DID_CANCEL"));

                            WinPostMsg(pWinData->ctl.hwndMainFrame,
                                       WM_SYSCOMMAND,
                                       (MPARAM)SC_CLOSE,
                                       0);

                            // PM frame proc then posts WM_CLOSE to
                            // the client (the split controller),
                            // which sends WM_CONTROL with SN_FRAMECLOSE
                            // back to the frame; the main msg loop in
                            // fdlgFileDlg detects that and exits
                            // V1.0.0 (2002-09-13) [umoeller]
                        break;
                    }
            break;

            case WM_CHAR:
                mrc = FrameChar(hwndFrame, mp1, mp2);
            break;

            default:
                fCallDefault = TRUE;
        }

        if (fCallDefault)
            mrc = pWinData->ctl.xfc.pfnwpOrig(hwndFrame, msg, mp1, mp2);
    }

    return mrc;
}

/* ******************************************************************
 *
 *   File dialog API
 *
 ********************************************************************/

/*
 *@@ fdlgFileDlg:
 *      main entry point into all this mess. This displays a
 *      file dialog with full WPS support, including shadows,
 *      WPS context menus, and all that, while being mostly
 *      parameter-compatible with WinFileDlg.
 *
 *      See fnwpMainControl for the (complex) window hierarchy.
 *
 *      Supported file-dialog flags in FILEDLG.fl:
 *
 *      -- FDS_FILTERUNION: when this flag is set, the dialog
 *         uses the union of the string filter and the
 *         extended-attribute type filter when filtering files
 *         for the Files list box. When this flag is not set,
 *         the list box, by default, uses the intersection of the
 *         two. @@todo
 *
 *      -- FDS_HELPBUTTON: a "Help" button is added which will
 *         post WM_HELP to hwndOwner.
 *
 *      -- FDS_MULTIPLESEL: when this flag is set, the Files container
 *         for the dialog is set to allow multiple selections. When
 *         this flag is not set, CCS_SINGLESEL is enabled. @@todo
 *
 *      -- FDS_OPEN_DIALOG or FDS_SAVEAS_DIALOG: one of the two
 *         should be set.
 *
 *      The following "fl" flags are ignored: FDS_APPLYBUTTON,
 *      FDS_CENTER, FDS_CUSTOM, FDS_ENABLEFILELB, FDS_INCLUDE_EAS,
 *      FDS_MODELESS, FDS_PRELOAD_VOLINFO.
 *
 *      FILEDLG.pszIType, papszITypeList, and sEAType are supported. @@todo
 *
 *      When FDS_MULTIPLESEL has been specified, FILEDLG.papszFQFileName
 *      receives the array of selected file names, as with WinFileDlg,
 *      and FILEDLG.ulFQFileCount receives the count. @@todo
 *
 *      The following other FILEDLG fields are ignored: ulUser,
 *      pfnDlgProc, pszIDrive, pszIDriveList, hMod, usDlgID, x, y.
 *
 *@@changed V1.0.0 (2002-09-13) [umoeller]: reworked greatly to work with new split view code
 */

HWND fdlgFileDlg(HWND hwndOwner,
                 const char *pcszStartupDir,        // in: current directory or NULL
                 PFILEDLG pfd)
{
    HWND        hwndReturn = NULLHANDLE;
    HPOINTER    hptrOld = NULLHANDLE;

    // static windata used by all components
    FILEDLGDATA WinData;

    memset(&WinData, 0, sizeof(WinData));

    WinData.pfd = pfd;

    lstInit(&WinData.llDialogControls, FALSE);

    lstInit(&WinData.llDisks, FALSE);

    // set the default return code for closing the window
    // this only ever gets set to DID_OK if the user
    // entered a valid file and pressed the OK button (or
    // double-clicked on one); we evaluate this in the
    // main msg loop below
    pfd->lReturn = DID_CANCEL;

    TRY_LOUD(excpt1)
    {
        CHAR        szCurDir[CCHMAXPATH] = "";
        PCSZ        pcszDlgTitle;

        ULONG       flInitialParse = 0;

        WPFolder    *pDrivesFolder;

        ULONG       flSplit;

        // set wait pointer, since this may take a second
        hptrOld = winhSetWaitPointer();

        /*
         *  PATH/FILE MASK SETUP
         *
         */

        // OK, here's the trick. We first call ParseFile
        // string with the current directory plus the "*"
        // file mask to make sure all fields are properly
        // initialized;
        // we then call it a second time with
        // full path string given to us in FILEDLG.
        if (pcszStartupDir && *pcszStartupDir)
            strlcpy(szCurDir, pcszStartupDir, sizeof(szCurDir));
        else
            // startup not specified:
            doshQueryCurrentDir(szCurDir);

        if (strlen(szCurDir) > 3)
            strcat(szCurDir, "\\*");
        else
            strcat(szCurDir, "*");
        ParseFileString(&WinData,
                        szCurDir);

        PMPF_SPLITVIEW(("pfd->szFullFile is %s", pfd->szFullFile));

        flInitialParse = ParseFileString(&WinData,
                                         pfd->szFullFile);
                            // store the initial parse flags so we
                            // can set the entry field properly below

        if (!(pcszDlgTitle = pfd->pszTitle))
            // no user title specified:
            if (pfd->fl & FDS_SAVEAS_DIALOG)
                pcszDlgTitle = cmnGetString(ID_XFSI_FDLG_SAVEFILEAS);
            else
                pcszDlgTitle = cmnGetString(ID_XFSI_FDLG_OPENFILE);

        flSplit = SPLIT_NOAUTOPOSITION | SPLIT_NOAUTOPOPOPULATE;

        if (pfd->fl & FDS_MULTIPLESEL)
            flSplit |= SPLIT_MULTIPLESEL;

        PMPF_SPLITVIEW(("getting drives folder"));

        // find the folder whose contents to
        // display in the left tree
        if (    (!(pDrivesFolder = _wpclsQueryFolder(_WPFolder,
                                                    (PSZ)WPOBJID_DRIVES,
                                                    TRUE)))
             || (!_somIsA(pDrivesFolder, _WPFolder))
           )
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                   "Cannot get drives folder.");
        else
        {
            PMPF_SPLITVIEW(("building disks list"));

            BuildDisksList(pDrivesFolder,
                           &WinData.llDisks);

            PMPF_SPLITVIEW(("calling fdrSplitCreateFrame"));

            if (fdrSplitCreateFrame(pDrivesFolder,
                                    pDrivesFolder,
                                    &WinData.ctl,
                                    FCF_NOBYTEALIGN
                                          | FCF_TITLEBAR
                                          | FCF_SYSMENU
                                          | FCF_MAXBUTTON
                                          | FCF_SIZEBORDER
                                          | FCF_AUTOICON,
                                    fnwpFileDlgFrame,
                                    pcszDlgTitle,
                                    flSplit,     // SPLIT_* styles
                                    WinData.szFileMask,
                                    30))       // split bar pos
            {
                QMSG    qmsg;

                if (hwndOwner)
                    if (!WinSetOwner(WinData.ctl.hwndMainFrame,
                                     hwndOwner))
                        cmnLog(__FILE__, __LINE__, __FUNCTION__,
                               "WinSetOwner(frame, 0x%lX failed",
                               hwndOwner);

                // populate the drives tree once we're running;
                // this is our replacement for what fdrSplitCreateFrame
                // would do if we hadn't specified SPLIT_NOAUTOPOPOPULATE,
                // because we want the tree only to be populated
                fdrPostFillFolder(&WinData.ctl,
                                  _wpQueryCoreRecord(pDrivesFolder),
                                  FFL_FOLDERSONLY | FFL_EXPAND);

                FrameCreateControls(&WinData);

                PMPF_SPLITVIEW(("subclassing hwndMainFrame"));

                WinSetWindowPtr(WinData.ctl.hwndMainFrame,
                                QWL_USER,
                                &WinData);

                // set up types combo
                if (pfd->papszITypeList)
                {
                    ULONG   ul = 0;
                    while (TRUE)
                    {
                        PSZ     *ppszTypeThis = pfd->papszITypeList[ul];

                        PMPF_SPLITVIEW(("pszTypeThis[%d] = %s", ul,
                                            (*ppszTypeThis) ? *ppszTypeThis : "NULL"));

                        if (!*ppszTypeThis)
                            break;

                        WinInsertLboxItem(WinData.hwndTypesCombo,
                                          LIT_SORTASCENDING,
                                          *ppszTypeThis);
                        ul++;
                    }
                }

                WinInsertLboxItem(WinData.hwndTypesCombo,
                                  0,                // first item always
                                  cmnGetString(ID_XFSI_FDLG_ALLTYPES));

                winhSetLboxSelectedItem(WinData.hwndTypesCombo,
                                        0,
                                        TRUE);

                // position dialog now
                if (!winhRestoreWindowPos(WinData.ctl.hwndMainFrame,
                                          HINI_USER,
                                          INIAPP_XWORKPLACE,
                                          INIKEY_WNDPOSFILEDLG,
                                          SWP_MOVE | SWP_SIZE)) // no show yet
                {
                    // no position stored yet:
                    WinSetWindowPos(WinData.ctl.hwndMainFrame,
                                    HWND_TOP,
                                    0, 0, 600, 400,
                                    SWP_SIZE);
                    winhCenterWindow(WinData.ctl.hwndMainFrame);       // still invisible
                }

                WinSetWindowPos(WinData.ctl.hwndMainFrame,
                                HWND_TOP,
                                0, 0, 0, 0,
                                SWP_SHOW | SWP_ACTIVATE | SWP_ZORDER);

                // expand the tree so that the current
                // directory/file mask etc. is properly
                // expanded
                UpdateDlgWithFullFile(&WinData);
                        // this will expand the tree and
                        // select the current directory;
                        // pllToExpand receives records to
                        // add first children to

                // set the entry field's initial contents:
                // a) if this is an "open" dialog, it should receive
                //    the file mask (FILEDLG.szFullFile normally
                //    contains something like "C:\path\*")
                // b) if this is a "save as" dialog, it should receive
                //    the filename that the application proposed
                //    (FILEDLG.szFullFile normally has "C:\path\filename.ext")
                WinSetWindowText(WinData.hwndFileEntry,     // WinFileDlg
                                 (flInitialParse & FFL_FILEMASK)
                                    ? WinData.szFileMask
                                    : WinData.szFileName);

                WinSetFocus(HWND_DESKTOP, WinData.hwndFileEntry);

                WinData.ctl.fSplitViewReady = TRUE;

                WinSetPointer(HWND_DESKTOP, hptrOld);

                /*
                 *  PM MSG LOOP
                 *
                 */

                // standard PM message loop... we stay in here
                // (simulating a modal dialog) until the split
                // controller receives WM_CLOSE and then posts (!)
                // WM_CONTROL with SN_FRAMECLOSE to the main frame

                while (WinGetMsg(WinData.ctl.habGUI, &qmsg, NULLHANDLE, 0, 0))
                {
                    if (    (qmsg.hwnd == WinData.ctl.hwndMainFrame)
                         && (qmsg.msg == WM_CONTROL)
                         && (SHORT1FROMMP(qmsg.mp1) == FID_CLIENT)
                         && (SHORT2FROMMP(qmsg.mp1) == SN_FRAMECLOSE)
                       )
                    {
                        // main file dlg client got WM_CLOSE:
                        // terminate the modal loop then...
                        // pfd->lReturn is still DID_CANCEL
                        // from above UNLESS the user selected
                        // a valid file and pressed "OK"

                        PMPF_SPLITVIEW(("WM_CONTROL + SN_FRAMECLOSE"));

                        winhSaveWindowPos(WinData.ctl.hwndMainFrame,
                                          HINI_USER,
                                          INIAPP_XWORKPLACE,
                                          INIKEY_WNDPOSFILEDLG);

                        if (pfd->lReturn == DID_OK)
                        {
                            sprintf(pfd->szFullFile,
                                    "%s%s\\%s",
                                    WinData.szDrive,     // C: or \\SERVER\RESOURCE
                                    WinData.szDir,
                                    WinData.szFileName);
                            pfd->ulFQFCount = 1;
                                // @@todo multiple selections
                            pfd->sEAType = -1;
                                // @@todo set this to the offset for "save as"

                            hwndReturn = (HWND)TRUE;
                        }

                        break;  // while (WinGetMsg...
                    }
                    else
                        WinDispatchMsg(WinData.ctl.habGUI, &qmsg);
                }
            }
        }
    }
    CATCH(excpt1)
    {
        // crash: return error
        hwndReturn = NULLHANDLE;
    }
    END_CATCH();

    /*
     *  CLEANUP
     *
     */

    hptrOld = winhSetWaitPointer();

    if (hwndOwner)
        WinSetActiveWindow(HWND_DESKTOP, hwndOwner);

    fdrSplitDestroyFrame(&WinData.ctl);

    lstClear(&WinData.llDisks);
    lstClear(&WinData.llDialogControls);

    PMPF_SPLITVIEW(("exiting, pfd->lReturn is %d", pfd->lReturn));
    PMPF_SPLITVIEW(("  pfd->szFullFile is %s", pfd->szFullFile));
    PMPF_SPLITVIEW(("  returning 0x%lX", hwndReturn));

    WinSetPointer(HWND_DESKTOP, hptrOld);

    return hwndReturn;
}

