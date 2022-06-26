
/*
 *@@sourcefile sound.c:
 *      XWorkplace extended system sounds implementation.
 *      Note that the actual playing of the sounds occurs
 *      at various parts of the XWorkplace code, such as
 *      fnwpSubclassedWPFolderWindow.
 *
 *      This also has some implementation code for XWPSound
 *      class.
 *
 *      Note: The original (WPS) MMSOUND_* constants are
 *            defined in helpers\syssound.h. The new
 *            (XWorkplace) MMSOUND_XFLD_* sound constants
 *            are defined in shared\common.h instead.
 *
 *      This file is ALL new with V0.9.0.
 *
 *@@added V0.9.0 [umoeller]
 *@@header "config\sound.h"
 */

/*
 *      Copyright (C) 1997-2005 Ulrich M”ller.
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

#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSSESMGR
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINDIALOGS
#define INCL_WINSHELLDATA
#define INCL_WINBUTTONS
#define INCL_WINLISTBOXES
#define INCL_WINENTRYFIELDS
#define INCL_WINCIRCULARSLIDER
#define INCL_WINSTDFILE

#define INCL_GPILOGCOLORTABLE
#define INCL_GPIPRIMITIVES
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h
#include <io.h>                 // access etc.
#include <string.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\stringh.h"            // string helper routines
#include "helpers\syssound.h"           // system sound helper routines
#include "helpers\winh.h"               // PM helper routines

// SOM headers which don't crash with prec. header files
#include "xwpsound.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\notebook.h"            // generic XWorkplace notebook handling

#include "config\sound.h"               // XWPSound implementation

#include "media\media.h"                // XWorkplace multimedia support

// other SOM headers
#pragma hdrstop

/* ******************************************************************
 *
 *   Declarations
 *
 ********************************************************************/

#ifndef __NOXSYSTEMSOUNDS__

/*
 * aulAddtlSystemSounds:
 *      array of all the additional XWorkplace system sounds.
 *      Those constants are decl'd in common.h.
 */

static ULONG aulAddtlSystemSounds[] =
        {
            MMSOUND_XFLD_SHUTDOWN,
            MMSOUND_XFLD_RESTARTWPS,
            MMSOUND_XFLD_CTXTOPEN,
            MMSOUND_XFLD_CTXTOPEN,
            MMSOUND_XFLD_CTXTSELECT,
            MMSOUND_XFLD_CNRDBLCLK
        };
#endif

typedef struct _SOUNDPAGEDATA
{
    HAB     hab;            // V0.9.19 (2002-05-23) [umoeller]

    CHAR    szMMPM[CCHMAXPATH];
                // path of ?:\MMPM2\MMPM.INI

    // "Sounds enabled"
    BOOL    fSoundsEnabled;

    // "Sound schemes" drop-down
    HWND    hwndSchemesDropDown;
    SHORT   sSchemeSelected;
                // list box index of currently selected scheme (or LIT_NONE)

    BOOL    fDontConfirmSchemeSelection;
                // anti-recursion flag

    // "Events" listbox
    HWND    hwndEventsListbox;
    SHORT   sEventSelected;
                // index of selected list box item (should never be LIT_NONE)

    // data for selected list box item:
    ULONG   ulSoundIndex;
                // decimal index in MMPM.INI
    CHAR    szDescription[300];
                // e.g. "Open window"
    CHAR    szFile[CCHMAXPATH];
                // full path of WAV file
    ULONG   ulVolume;
                // 0-100; this _always_ has the individual sound
                // volume, even if (fCommonVolume == TRUE);

    // "File" entry field
    HWND    hwndSoundFile;
    PFNWP   pfnwpSoundFileOriginal;
                // original entry field window proc (subclassed)
    CHAR    szPathDropped[CCHMAXPATH],
            szFileDropped[CCHMAXPATH];
                // drag'n'drop buffer
    BOOL    fSoundFileChanged;

    // "Common volume"
    BOOL    fCommonVolume;
    ULONG   ulCommonVolume;
                // 0-100; this _always_ has the common sound
                // volume, even if (fCommonVolume == FALSE);
} SOUNDPAGEDATA, *PSOUNDPAGEDATA;

/* ******************************************************************
 *
 *   Additional system sounds
 *
 ********************************************************************/

#ifndef __NOXSYSTEMSOUNDS__

/*
 *@@ sndAddtlSoundsInstalled:
 *      returns TRUE if the additional system
 *      sounds are installed in MMPM.INI.
 *
 *@@added V0.9.1 (99-12-30) [umoeller]
 *@@changed V0.9.6 (2000-10-16) [umoeller]: added proper HAB param
 */

BOOL sndAddtlSoundsInstalled(HAB hab)
{
    BOOL brc = TRUE;

    ULONG ul = 0;

    for (ul = 0;
         ul < (sizeof(aulAddtlSystemSounds) / sizeof(ULONG)); // sizeof array
         ul++)
    {
        if (!sndQuerySystemSound(hab, aulAddtlSystemSounds[ul], NULL, NULL, NULL))
        {
            // not found: return FALSE
            brc = FALSE;
            break;
        }
    }

    return brc;
}

/*
 *@@ sndInstallAddtlSounds:
 *      this installs or de-installs the additional
 *      XWorkplace sounds into MMPM.INI by calling
 *      the NLS REXX script in the XWOrkplace bin\
 *      directory.
 *
 *      Returns FALSE upon errors, e.g. if the
 *      REXX script wasn't found.
 *
 *@@added V0.9.1 (99-12-30) [umoeller]
 *@@changed V0.9.6 (2000-10-16) [umoeller]: added proper HAB param
 *@@changed V0.9.6 (2000-10-16) [umoeller]: forgot to close MMPM.INI, fixed
 *@@changed V1.0.5 (2006-04-01) [pr]: added title parameter to doshQuickStartSession
 */

BOOL sndInstallAddtlSounds(HAB hab,
                           BOOL fInstall) // in: TRUE: install sounds; FALSE: de-install sounds
{
    BOOL    brc = FALSE;
    CHAR    szPath[CCHMAXPATH], szCmdFile[CCHMAXPATH];

    if (fInstall)
    {
        // install system sounds:
        // call install\soundxxx.cmd in the
        // XWorkplace install directory;
        // this is necessary because of NLS
        if (cmnQueryXWPBasePath(szPath))
        {
            PID     pid;
            ULONG   sid;
            sprintf(szCmdFile,
                    "/c %s\\install\\sound%s.cmd",
                    szPath,
                    cmnQueryLanguageCode());
            if (!doshQuickStartSession("cmd.exe",
                                       szCmdFile,
                                       szCmdFile + 3,               // title
                                       SSF_TYPE_DEFAULT,            // session type
                                       FALSE,                       // background
                                       SSF_CONTROL_INVISIBLE,       // but auto-close
                                       TRUE,                        // wait
                                       &sid,
                                       &pid,
                                       NULL))
                brc = TRUE;
        }
    }
    else
    {
        // deinstall system sounds:
        HINI hiniMmpm = sndOpenMmpmIni(hab);
        if (hiniMmpm)       // V0.9.6 (2000-10-16) [umoeller]
        {
            ULONG ul = 0;

            for (ul = 0;
                 ul < (sizeof(aulAddtlSystemSounds) / sizeof(ULONG)); // sizeof array
                 ul++)
            {
                sndWriteSoundData(hiniMmpm,
                                  aulAddtlSystemSounds[ul],
                                  NULL, NULL, 0);
                brc = TRUE;
            }
            PrfCloseProfile(hiniMmpm);  // V0.9.6 (2000-10-16) [umoeller]
        }
    }

    return brc;
}

#endif

/* ******************************************************************
 *
 *   Helper funcs
 *
 ********************************************************************/

/*
 *@@ FillSchemesDropDown:
 *      this fills any given drop-down box (listboxes
 *      should work also) with all the sound schemes that
 *      we can find in OS2SYS.INI.
 *
 *      Note: this does NOT select an item. You must do
 *      this manually afterwards (using SelectSoundScheme).
 */

STATIC VOID FillDropDownWithSchemes(HWND hwndDropDown)
{
    APIRET arc = NO_ERROR;
    PSZ pszSchemes = NULL;

    // first empty listbox (this might get called later again)
    WinSendMsg(hwndDropDown, LM_DELETEALL, MPNULL, MPNULL);

    if (!(arc = prfhQueryKeysForApp(HINI_SYSTEM,
                                    MMINIKEY_SOUNDSCHEMES, // "PM_SOUND_SCHEMES_LIST"
                                    &pszSchemes)))
    {
        PSZ     pKey2 = pszSchemes;

        while (*pKey2 != 0)
        {
            // combo box supports list box messages too
            WinInsertLboxItem(hwndDropDown,
                              LIT_SORTASCENDING,
                              pKey2);
            // next sound index
            pKey2 += strlen(pKey2)+1;
        }

        free(pszSchemes);
    }
}

/*
 *@@ SelectSoundScheme:
 *      this selects a sound scheme in the drop-down
 *      box. If (sIndex == LIT_NONE), we manipulate
 *      the box's entry field to display "<none>".
 */

STATIC VOID SelectSoundScheme(PSOUNDPAGEDATA pspd,
                              SHORT sIndex)
{
    winhSetLboxSelectedItem(pspd->hwndSchemesDropDown,
                            sIndex,
                            TRUE);
    if (sIndex == LIT_NONE)
    {
        // "none" selected:
        // set drop-down box entry field to "none"
        // (each combo box has an entry field child
        // with the CBID_EDIT ID).
        HWND hwndDropDownEntryField = WinWindowFromID(pspd->hwndSchemesDropDown,
                                                      CBID_EDIT); // pmwin.h
        // PNLSSTRINGS pNLSStrings = cmnQueryNLSStrings();

        WinSetWindowText(hwndDropDownEntryField, cmnGetString(ID_XSSI_SOUNDSCHEMENONE)) ; // pszSoundSchemeNone
                    // "<none>");

        // delete current scheme in OS2.INI
        PrfWriteProfileString(HINI_USER,
                              (PSZ)INIAPP_XWORKPLACE,
                              (PSZ)INIKEY_XWPSOUNDSCHEME,  // "XWPSound:Scheme"
                              NULL);                  // delete key

        pspd->sSchemeSelected = LIT_NONE;
    }
}

/*
 *@@ UpdateMMPMINI:
 *      this updates one sound key in MMPM.INI according
 *      to the current data on the "Sounds" notebook page.
 *      We evaluate SOUNDPAGEDATA to find out more.
 *      This does _not_ update the global flags in that
 *      profile.
 *
 *      After MMPM.INI has been updated, we set the
 *      current sound scheme to "none".
 */

STATIC VOID UpdateMMPMINI(PNOTEBOOKPAGE pnbp)
{
    PSOUNDPAGEDATA pspd = (PSOUNDPAGEDATA)pnbp->pUser;

    if (pspd->sEventSelected != LIT_NONE)
    {
        HINI hiniMMPM = PrfOpenProfile(pspd->hab,
                                       pspd->szMMPM);

        if (hiniMMPM)
        {
            sndWriteSoundData(hiniMMPM,
                              pspd->ulSoundIndex,
                              pspd->szDescription,
                              pspd->szFile,
                              pspd->ulVolume);
            PrfCloseProfile(hiniMMPM);
        }

        // this only gets called when a sound has
        // changed, so we select "none" in the
        // schemes drop-down
        if (pspd->sSchemeSelected != LIT_NONE) // pspd->sCustomSchemeIndex)
        {
            SelectSoundScheme(pspd, LIT_NONE);
        }
    }
}

/*
 *@@ SaveSoundSchemeAs:
 *      this opens the "New Sound Scheme" dialog
 *      and calls sndCreateSoundScheme with the
 *      new scheme name that was entered by the user.
 *
 *      This checks for whether that sound scheme
 *      exists and prompts if necessary.
 *
 *      Afterwards, the "Schemes" drop-down is updated,
 *      and, if (fSelectNew == TRUE), the new item
 *      is selected.
 *
 *      Returns TRUE only if we actually copied, that
 *      is, the user didn't press Cancel and no errors
 *      occurred.
 *
 *@@changed V0.9.20 (2002-07-03) [umoeller]: check for scheme exists is case-insensitive now
 */

STATIC BOOL SaveSoundSchemeAs(PNOTEBOOKPAGE pnbp,
                              BOOL fSelectNew)
{
    BOOL    brc = FALSE;
    PSOUNDPAGEDATA pspd = (PSOUNDPAGEDATA)pnbp->pUser;

    HWND hwndDlg;
    if (hwndDlg = cmnLoadDlg(pnbp->hwndFrame,  // owner
                             WinDefDlgProc,
                             ID_XSD_NEWSOUNDSCHEME,   // "New Sound Scheme" dlg
                             NULL))
    {
        WinSendDlgItemMsg(hwndDlg, ID_XSDI_FT_ENTRYFIELD,
                          EM_SETTEXTLIMIT,
                          (MPARAM)100,
                          MPNULL);

        // add all the schemes to the drop-down box in that
        // dialog too
        FillDropDownWithSchemes(WinWindowFromID(hwndDlg, ID_XSDI_FT_ENTRYFIELD));

        if (WinProcessDlg(hwndDlg) == DID_OK)
        {
            CHAR    szNewScheme[100];
            BOOL    fOverwrite = TRUE;
            PSZ     pszDelete = NULL;

            // get new scheme name from dlg
            WinQueryDlgItemText(hwndDlg,
                                ID_XSDI_FT_ENTRYFIELD,
                                sizeof(szNewScheme) - 1,
                                szNewScheme);

            if (strlen(szNewScheme) < 3)
                winhDebugBox(pnbp->hwndFrame,
                         "Error",
                         "This is not a valid scheme name.");

            // check in OS2SYS.INI's scheme list whether that
            // scheme exists already
            if (sndDoesSchemeExist(szNewScheme,
                                   &pszDelete))     // V0.9.20 (2002-07-03) [umoeller]
            {
                PCSZ     psz = szNewScheme;
                // exists: have user confirm this
                if (cmnMessageBoxExt(pnbp->hwndFrame,
                                     151,  // "Sound"
                                     &psz, 1,
                                     152,
                                     MB_YESNO)
                        == MBID_YES)
                {
                    sndDestroySoundScheme(pszDelete);   // V0.9.20 (2002-07-03) [umoeller]
                }
                else
                    fOverwrite = FALSE;

                free(pszDelete);
            }

            // shall we proceed?
            if (fOverwrite)
            {
                HINI hiniMMPM;
                if (hiniMMPM = PrfOpenProfile(pspd->hab,
                                              pspd->szMMPM))
                {
                    SHORT sNewSchemeIndex = LIT_NONE;
                    // write new sound scheme
                    APIRET arc;

                    if (!(arc = sndCreateSoundScheme(hiniMMPM,
                                                     szNewScheme)))
                    {
                        // OK, everything has worked out fine:
                        // update the "Schemes" drop-down
                        pspd->fDontConfirmSchemeSelection = TRUE;
                        FillDropDownWithSchemes(pspd->hwndSchemesDropDown);

                        if (fSelectNew)
                        {
                            // find the scheme that was just created
                            sNewSchemeIndex = (SHORT)WinSendMsg(pspd->hwndSchemesDropDown,
                                                                LM_SEARCHSTRING,
                                                                MPFROM2SHORT(LSS_CASESENSITIVE,
                                                                             LIT_FIRST),
                                                                (MPARAM)szNewScheme);
                            // and select it
                            SelectSoundScheme(pspd, sNewSchemeIndex);
                        }

                        brc = TRUE;
                    }
                    else
                    {
                        CHAR szTemp[100];
                        sprintf(szTemp, "Error code: %d", arc);
                        winhDebugBox(pnbp->hwndFrame,
                                 "Error saving scheme",
                                 szTemp);
                    }

                    PrfCloseProfile(hiniMMPM);
                } // end if (hiniMMPM)
            } // end if (fOverwrite)

        } // end if (WinProcessDlg(hwndDlg) == DID_OK)
    } // end if (hwndDlg)

    return brc;
}

/*
 *@@ LoadSoundSchemeFrom:
 *      as opposed to SaveSoundSchemeAs, this will
 *      load a sound scheme (as specified in the
 *      drop-down box) to MMPM.INI (by calling
 *      sndLoadSoundScheme).
 *
 *      This gets called from sndSoundsItemChanged
 *      when a new scheme has been selected in the
 *      "Schemes" drop-down box.
 *
 *      No more confirmations in this function.
 */

STATIC BOOL LoadSoundSchemeFrom(PNOTEBOOKPAGE pnbp)
{
    BOOL    brc = FALSE;
    PSOUNDPAGEDATA pspd = (PSOUNDPAGEDATA)pnbp->pUser;

    HINI hiniMMPM;

    if (hiniMMPM = PrfOpenProfile(pspd->hab,
                                  pspd->szMMPM))
    {
        CHAR szSchemeName[200];
        APIRET arc;

        // get scheme name from drop-down
        WinQueryWindowText(pspd->hwndSchemesDropDown,
                           sizeof(szSchemeName),
                           szSchemeName);

        arc = sndLoadSoundScheme(hiniMMPM,
                                 szSchemeName);
        if (arc == NO_ERROR)
            brc = TRUE;
        else
        {
            CHAR szTemp[100];
            sprintf(szTemp, "Error code: %d", arc);
            winhDebugBox(pnbp->hwndFrame,
                     "Error loading scheme",
                     szTemp);
        }

        PrfCloseProfile(hiniMMPM);
    } // end if (hiniMMPM)

    return brc;
}

/*
 *@@ DrawTargetEmphasis:
 *      this is used to draw target emphasis around the
 *      entryfield.
 */

STATIC VOID DrawTargetEmphasis(HWND hwnd, BOOL fEmphasis)
{
    HPS                hps;
    POINTL             ptl;
    RECTL              rcl;
    static BOOL        fLastEmphasis = FALSE;

    // prevent multiple draws of the same kind
    if (fLastEmphasis == fEmphasis)
        return;
    fLastEmphasis = fEmphasis;

    // if (bFlag == TRUE && bDraw == FALSE)
    //        return;

    WinQueryWindowRect(hwnd, &rcl);

    hps = DrgGetPS(hwnd);

    ptl.x = ptl.y = 5;
    GpiMove(hps, &ptl);

    ptl.x = rcl.xRight - rcl.xLeft - 5;
    ptl.y = rcl.yTop - rcl.yBottom - 5;

    GpiSetColor(hps, CLR_RED);
    GpiSetMix(hps, FM_XOR);
    GpiSetBackMix(hps, BM_LEAVEALONE);

    GpiBox(hps, DRO_OUTLINE, &ptl, 0L, 0L);
    DrgReleasePS(hps);
}

/*
 *@@ fnwpSubclassedSoundFile:
 *      subclassed window proc for the "File" entry
 *      field. This accepts files via drag'n'drop.
 *
 *      The QWL_USER window word points to the notebook
 *      page's CREATENOTEBOOKPAGE structure.
 */

STATIC MRESULT EXPENTRY fnwpSubclassedSoundFile(HWND hwndEntryField,
                                                ULONG msg,
                                                MPARAM mp1,
                                                MPARAM mp2)
{
    PNOTEBOOKPAGE pnbp = (PNOTEBOOKPAGE)(WinQueryWindowPtr(hwndEntryField, QWL_USER));
    PSOUNDPAGEDATA pspd = (PSOUNDPAGEDATA)pnbp->pUser;
    MRESULT mrc = 0;

    switch (msg)
    {
        /*
         * DM_DRAGOVER:
         *      something is being dragged over our
         *      entry field. We check if it's a
         *      file system object and allow dropping
         *      or not.
         */

        case DM_DRAGOVER:
        {
            PDRAGINFO   pdrgInfo = (PDRAGINFO)mp1;
            // default return values
            USHORT      usIndicator = DOR_NEVERDROP,
                            // cannot be dropped, and don't send
                            // DM_DRAGOVER again
                        usOp = DO_UNKNOWN;
                            // target-defined drop operation:
                            // user operation (we don't want
                            // the WPS to copy anything)

            // reset global variable
            pspd->szFileDropped[0] = 0;

            DrawTargetEmphasis(hwndEntryField, TRUE);

            // go!
            if (DrgAccessDraginfo(pdrgInfo))
            {
                // assert default drop operation (no
                // modifier keys are pressed)
                if (pdrgInfo->usOperation == DO_DEFAULT)
                {
                    PMPF_SOUNDS(("DM_DRAGOVER: cditem == %d", pdrgInfo->cditem));

                    // accept only one item at a time
                    if (pdrgInfo->cditem == 1)
                    {
                        // access drag item;
                        // now check render mechanism and format.
                        // The WPS gives us DRM_OS2FILE for "real" files only,
                        // but DRM_OBJECT for abstract objects.
                        // Moreover, the format is DRF_TEXT for all files,
                        // but folders have DRF_UNKNOWN instead.
                        // So by checking for DRM_OS2FILE and DRF_TEXT,
                        // we should get data files only (and exclude
                        // abstract objects).
                        // Note that the WPS automatically de-references
                        // shadows here. :-)
                        PDRAGITEM pdrgItem = DrgQueryDragitemPtr(pdrgInfo, 0);

                        {
                            CHAR szRMF[200];
                            DrgQueryStrName(pdrgItem->hstrRMF,
                                        sizeof(szRMF), szRMF);
                            PMPF_SOUNDS(("   hstrRMF: %s", szRMF));
                        }

                        if (DrgVerifyRMF(pdrgItem, "DRM_OS2FILE", "DRF_WAVE"))
                                // this works on Warp 4
                        {
                            // valid file-system object:
                            usIndicator = DOR_DROP;
                            // get the path and file name
                            DrgQueryStrName(pdrgItem->hstrContainerName,
                                        sizeof(pspd->szPathDropped), pspd->szPathDropped);
                                // e.g. for "F:\SOUNDS\TEST.WAV" this would be "F:\SOUNDS\"

                            // this gives us the filename of the dragged folder
                            DrgQueryStrName(pdrgItem->hstrSourceName,
                                        sizeof(pspd->szFileDropped), pspd->szFileDropped);
                                // e.g. for "F:\SOUNDS\TEST.WAV" this would be "TEST.WAV"
                        }
                    }
                }
                else
                    // non-default drop op:
                    usIndicator = DOR_NODROP;
                    // but do send DM_DRAGOVER again

                // clean up
                DrgFreeDraginfo(pdrgInfo);
            }

            // and return the drop flags
            mrc = (MRFROM2SHORT(usIndicator, usOp));
        }
        break;

        /*
         * DM_DROP:
         *      something's dropped upon our entry field
         *      window. We set our own text then.
         */

        case DM_DROP:
        {
            PDRAGINFO   pdrgInfo = (PDRAGINFO)mp1;

            // go!
            if (DrgAccessDraginfo(pdrgInfo))
            {
                // check if we had something valid above
                if (pspd->szFileDropped[0] != 0)
                        // global variable set in DM_DRAGOVER
                {
                    PDRAGITEM pdrgItem = DrgQueryDragitemPtr(pdrgInfo, 0);
                    CHAR szNewFileComplete[2*CCHMAXPATH];

                    // set our own text to the new file name
                    sprintf(szNewFileComplete, "%s%s",
                                pspd->szPathDropped,
                                pspd->szFileDropped);
                    WinSetWindowText(hwndEntryField, szNewFileComplete);
                    UpdateMMPMINI(pnbp);

                    // tell the source (the WPS or whoever) that
                    // we're done; otherwise the source will wait
                    // forever for the drag operation to complete
                    DrgSendTransferMsg(pdrgItem->hwndItem,
                                       DM_ENDCONVERSATION,
                                       (MPARAM)pdrgItem->ulItemID, // recc
                                       (MPARAM)DMFL_TARGETSUCCESSFUL);
                }

                // clean up
                DrgDeleteDraginfoStrHandles(pdrgInfo);
                DrgFreeDraginfo(pdrgInfo);
            }

            // always return 0
            mrc = (MRESULT)0;
        }
        break;

        default:
            mrc = (*(pspd->pfnwpSoundFileOriginal))(hwndEntryField, msg, mp1, mp2);
    }

    return mrc;
}

/* ******************************************************************
 *
 *   XWPSound notebook callbacks (notebook.c)
 *
 ********************************************************************/

/*
 *@@ sndSoundsInitPage:
 *      "Sounds" page notebook callback function (notebook.c).
 *      Sets the controls on the page.
 *
 *@@changed V0.9.10 (2001-04-16) [umoeller]: fixed fixed MMPM.INI path location
 */

VOID sndSoundsInitPage(PNOTEBOOKPAGE pnbp,           // notebook info struct
                       ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        // create SOUNDPAGEDATA structure;
        // this will be free()'d automatically
        // when the notebook page is destroyed
        if (pnbp->pUser = malloc(sizeof(SOUNDPAGEDATA)))
        {
            PSZ     pszLastScheme;
            SHORT   sSchemeToSelect;

            PSOUNDPAGEDATA pspd = (PSOUNDPAGEDATA)(pnbp->pUser);
            memset(pspd, 0, sizeof(SOUNDPAGEDATA));

            pspd->hab = WinQueryAnchorBlock(pnbp->hwndDlgPage);

            // sound schemes
            pspd->hwndSchemesDropDown = WinWindowFromID(pnbp->hwndDlgPage,
                                                        ID_XSDI_SOUND_SCHEMES_DROPDOWN);
            // events
            pspd->hwndEventsListbox = WinWindowFromID(pnbp->hwndDlgPage,
                                                      ID_XSDI_SOUND_EVENTSLISTBOX);
            pspd->sEventSelected = LIT_NONE;

            // others
            pspd->hwndSoundFile = WinWindowFromID(pnbp->hwndDlgPage, ID_XSDI_SOUND_FILE);

            // set text length for entry field
            winhSetEntryFieldLimit(pspd->hwndSoundFile, 255);
            // store CREATENOTEBOOKPAGE in entry field's QWL_USER
            WinSetWindowULong(pspd->hwndSoundFile, QWL_USER, (ULONG)pnbp);
            // subclass the entry field to support drag'n'drop
            pspd->pfnwpSoundFileOriginal = WinSubclassWindow(pspd->hwndSoundFile,
                                                             fnwpSubclassedSoundFile);

            // sprintf(pspd->szMMPM, "%c:\\MMOS2\\MMPM.INI", doshQueryBootDrive());
            sndQueryMmpmIniPath(pspd->szMMPM);
                    // V0.9.10 (2001-04-16) [umoeller]

            // create circular slider
            winhReplaceWithCircularSlider(pnbp->hwndDlgPage, pnbp->hwndDlgPage,
                                          // hwndInsertAfter:
                                          WinWindowFromID(pnbp->hwndDlgPage,
                                                          ID_XSDI_SOUND_COMMONVOLUME),
                                          ID_XSDI_SOUND_VOLUMELEVER,
                                          CSS_NOTEXT
                                            | CSS_POINTSELECT
                                            | CSS_MIDPOINT
                                            | CSS_PROPORTIONALTICKS,
                                          0, 100,       // range
                                          5, 20);       // increments

            // fill "Sound schemes" drop-down
            // Note: We do this within CBI_INIT because we
            // really only want this to be done exactly once.
            // When sound schemes change, we call this function
            // again manually later.
            pspd->fDontConfirmSchemeSelection = TRUE;
            FillDropDownWithSchemes(pspd->hwndSchemesDropDown);

            // now query last selected scheme from OS2.INI
            if (pszLastScheme = prfhQueryProfileData(HINI_USER,
                                                     INIAPP_XWORKPLACE,
                                                     INIKEY_XWPSOUNDSCHEME,
                                                     NULL))
            {
                sSchemeToSelect = (SHORT)WinSendMsg(pspd->hwndSchemesDropDown,
                                                    LM_SEARCHSTRING,
                                                    MPFROM2SHORT(LSS_CASESENSITIVE,
                                                                 LIT_FIRST),
                                                    (MPARAM)pszLastScheme);
                free(pszLastScheme);
            }
            else
                // none saved: select "none"
                sSchemeToSelect = LIT_NONE; // pspd->sCustomSchemeIndex;

            SelectSoundScheme(pspd, sSchemeToSelect);
        }
    }

    if (flFlags & CBI_SET)
    {
        PSOUNDPAGEDATA pspd = (PSOUNDPAGEDATA)pnbp->pUser;
        HINI    hiniMMPM;

        if (hiniMMPM = PrfOpenProfile(pspd->hab,
                                      pspd->szMMPM))
        {
            CHAR szData[100];
            PSZ pszSysSounds;
            APIRET arc = NO_ERROR;

            // "enabled" in MMPM.INI
            PrfQueryProfileString(hiniMMPM,
                                  MMINIKEY_SOUNDSETTINGS,
                                  "EnableSounds",
                                  "TRUE",    // default string (tested)
                                  szData, sizeof(szData));
            pspd->fSoundsEnabled = (strcmp(szData, "TRUE") == 0);

            // "common volume" in MMPM.INI
            // 1) flag
            PrfQueryProfileString(hiniMMPM,
                                  MMINIKEY_SOUNDSETTINGS,
                                  "ApplyVolumeToAll",
                                  "FALSE",   // default string (tested)
                                  szData, sizeof(szData));
            pspd->fCommonVolume = (strcmp(szData, "FALSE") != 0);

            // 2) value
            PrfQueryProfileString(hiniMMPM,
                                  MMINIKEY_SOUNDSETTINGS,
                                  "Volume",
                                  "100",
                                  szData, sizeof(szData));
            sscanf(szData, "%d", &(pspd->ulCommonVolume));

            // first empty listbox (this might get called later again)
            WinSendMsg(pspd->hwndEventsListbox, LM_DELETEALL, MPNULL, MPNULL);

            // fill "Events" listbox
            if (!(arc = prfhQueryKeysForApp(hiniMMPM,
                                            MMINIKEY_SYSSOUNDS, // "MMPM2_AlarmSounds"
                                            &pszSysSounds)))
            {
                // go thru system sounds
                PSZ     pKey2 = pszSysSounds;

                while (*pKey2)
                {
                    // pKey2 has the current key now
                    ULONG   cbData = 0;
                    PSZ     pszSoundData;
                    if (pszSoundData = prfhQueryProfileData(hiniMMPM,
                                                            MMINIKEY_SYSSOUNDS,
                                                            pKey2,
                                                            &cbData))
                    {
                        CHAR    szDescription[200];

                        if (sndParseSoundData(pszSoundData,
                                              szDescription,
                                              NULL,
                                              NULL))
                        {
                            // szDescription now has the sound description
                            // for pKey2
                            ULONG   ulSoundIndex;
                            SHORT   sIndex = WinInsertLboxItem(pspd->hwndEventsListbox,
                                                               LIT_SORTASCENDING,
                                                               szDescription);

                            PMPF_SOUNDS(("  Descr: %s", szDescription));

                            // store the sound index as an item "handle"
                            // with the list box item, so we can retrieve
                            // it later
                            sscanf(pKey2, "%d", &ulSoundIndex);
                            WinSendMsg(pspd->hwndEventsListbox,
                                       LM_SETITEMHANDLE,
                                       (MPARAM)sIndex,
                                       (MPARAM)ulSoundIndex);
                        }

                        free(pszSoundData);
                    }

                    // next sound index
                    pKey2 += strlen(pKey2) + 1;
                }

                // select first item
                winhSetLboxSelectedItem(pspd->hwndEventsListbox, 0, TRUE);

                free(pszSysSounds);
            }

            PrfCloseProfile(hiniMMPM);
        }

        // "enabled" checkbox
        winhSetDlgItemChecked(pnbp->hwndDlgPage,
                              ID_XSDI_SOUND_ENABLE,
                              pspd->fSoundsEnabled);

        // "common volume" checkbox
        winhSetDlgItemChecked(pnbp->hwndDlgPage,
                              ID_XSDI_SOUND_COMMONVOLUME,
                              pspd->fCommonVolume);
    }

    if (flFlags & CBI_ENABLE)
    {
        PSOUNDPAGEDATA pspd = (PSOUNDPAGEDATA)pnbp->pUser;
        BOOL fFileExists = (access(pspd->szFile, 0) == 0);

        // enable "Save as" (sound scheme)
        // only if current scheme is "none"
        /* WinEnableControl(pnbp->hwndDlgPage,
                          ID_XSDI_SOUND_SCHEMES_SAVEAS,
                          (pspd->ulSchemeSelectedHandle != 0)); */
                                // this is != 0 for "pseudo schemes" ("none" or "no sounds")

        // disable "Delete" (sound scheme) button
        // if current scheme is "none"
        WinEnableControl(pnbp->hwndDlgPage,
                          ID_XSDI_SOUND_SCHEMES_DELETE,
                          (pspd->sSchemeSelected != LIT_NONE));

        // enable "Play" button if the current sound file exists
        WinEnableControl(pnbp->hwndDlgPage,
                          ID_XSDI_SOUND_PLAY,
                          (     (pspd->sEventSelected != LIT_NONE)
                             && (fFileExists)
                          )
                      );

        // enable volume knob if either the current sound file
        // exists or "global volume" is enabled
        /* WinEnableControl(pnbp->hwndDlgPage,
                          ID_XSDI_SOUND_VOLUMELEVER,
                          (     (   (pspd->sEventSelected != LIT_NONE)
                                &&  (fFileExists)
                                )
                             || (pspd->fCommonVolume)
                          )
                      ); */
    }

    if (flFlags & CBI_DESTROY)
    {
        // destroy subclassed entry field explicitly;
        // this is necessary because PM destroys the
        // parent first and _then_ the children, so
        // that the entry field will then crash because
        // SOUNDPAGEDATA is gone
        PSOUNDPAGEDATA pspd = (PSOUNDPAGEDATA)pnbp->pUser;
        WinDestroyWindow(pspd->hwndSoundFile);
    }
}

/*
 *@@ sndSoundsItemChanged:
 *      "Sounds" page notebook callback function (notebook.c).
 *      Reacts to changes of any of the dialog controls.
 *
 *      This is a real monster function, because we have so
 *      many controls interacting.
 */

MRESULT sndSoundsItemChanged(PNOTEBOOKPAGE pnbp,  // notebook info
                             ULONG ulItemID, USHORT usNotifyCode,
                             ULONG ulExtra)      // for checkboxes: contains new state
{
    PSOUNDPAGEDATA pspd = (PSOUNDPAGEDATA)pnbp->pUser;

    switch (ulItemID)
    {

        /*
         * ID_XSDI_SOUND_ENABLE:
         *      "Enable system sounds" checkbox.
         *      ulExtra has new checkbox state;
         *      update SOUNDPAGEDATA and MMPM.INI
         */

        case ID_XSDI_SOUND_ENABLE:
        {
            HINI hiniMMPM = PrfOpenProfile(pspd->hab,
                                           pspd->szMMPM);
            pspd->fSoundsEnabled = ulExtra;
            if (hiniMMPM)
            {
                // "enabled" in MMPM.INI
                PrfWriteProfileString(hiniMMPM,
                        MMINIKEY_SOUNDSETTINGS, "EnableSounds",
                        (ulExtra) ? "TRUE" : "FALSE");
                PrfCloseProfile(hiniMMPM);
            }
            // re-enable controls
            pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
        }
        break;

        /*
         * ID_XSDI_SOUND_SCHEMES_DROPDOWN:
         *      "Schemes" drop-down box.
         *      New scheme selected:
         *      update SOUNDPAGEDATA and other display
         */

        case ID_XSDI_SOUND_SCHEMES_DROPDOWN:
        {
            if (usNotifyCode == LN_SELECT)
            {
                BOOL    fOverwrite = TRUE;
                SHORT   sNewSchemeSelected = winhQueryLboxSelectedItem(
                                                            pspd->hwndSchemesDropDown,
                                                            LIT_FIRST);
                // ULONG   ulSchemeHandle = 0;
                CHAR    szSchemeSelected[200];

                PMPF_SOUNDS(("Entering ID_XSDI_SOUND_SCHEMES_DROPDOWN"));
                PMPF_SOUNDS(("  sNewSchemeSelected: %d", sNewSchemeSelected));

                // filter out the very first call
                if (!pspd->fDontConfirmSchemeSelection) // pspd->sSchemeSelected != LIT_NONE)
                {

                    if (    (sNewSchemeSelected != LIT_NONE)
                         && (sNewSchemeSelected != pspd->sSchemeSelected)
                       )
                    {
                        // new scheme selected:


                        if (pspd->sSchemeSelected == LIT_NONE) // pspd->sCustomSchemeIndex)
                        {
                            // no sound scheme selected so far:
                            // this means that the data has not been saved
                            // in a sound scheme yet, so we better have the
                            // user confirm this

                            ULONG ulAnswer = cmnMessageBoxExt(pnbp->hwndFrame,
                                                              151,
                                                              NULL, 0,
                                                              153, // "save first"?
                                                              MB_YESNOCANCEL);

                            switch (ulAnswer)
                            {
                                case MBID_YES:
                                    if (SaveSoundSchemeAs(pnbp,
                                                          FALSE))
                                                    // do not select the new item
                                        fOverwrite = TRUE;
                                    else
                                        // error or cancelled:
                                        fOverwrite = FALSE;
                                break;

                                case MBID_NO:
                                    fOverwrite = TRUE;
                                break;

                                default:        // cancel
                                    fOverwrite = FALSE;
                            }
                        }

                        PMPF_SOUNDS(("  fOverwrite: %d", fOverwrite));

                        if (fOverwrite)
                        {
                            PMPF_SOUNDS(("  Calling LoadSoundSchemeFrom"));
                            LoadSoundSchemeFrom(pnbp);

                            // now update the whole damn page
                            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET);
                        }
                        else
                        {
                            // changing aborted for some reason:
                            // re-select LIT_NONE, which we had
                            // before

                            PMPF_SOUNDS(("  Calling SelectSoundScheme LIT_NONE"));
                            SelectSoundScheme(pspd, LIT_NONE);
                            sNewSchemeSelected = LIT_NONE;
                                // for the following
                        }
                    } // end if (sNewSchemeSelected ...
                } // end if (!pspd->fDontConfirmSchemeSelection)
                else
                    pspd->fDontConfirmSchemeSelection = FALSE;

                // store new selection
                pspd->sSchemeSelected = sNewSchemeSelected;
                // pspd->ulSchemeSelectedHandle = ulSchemeHandle;

                // re-enable items
                pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);

                // save current scheme in OS2.INI
                if (pspd->sSchemeSelected != LIT_NONE) // pspd->sCustomSchemeIndex)
                {
                    WinQueryWindowText(pspd->hwndSchemesDropDown,
                                       sizeof(szSchemeSelected),
                                       szSchemeSelected);
                    PrfWriteProfileString(HINI_USER,
                                          (PSZ)INIAPP_XWORKPLACE,
                                          (PSZ)INIKEY_XWPSOUNDSCHEME,  // "XWPSound:Scheme"
                                          szSchemeSelected);
                }
                PMPF_SOUNDS(("End of ID_XSDI_SOUND_SCHEMES_DROPDOWN"));
                PMPF_SOUNDS(("  pspd->sSchemeSelected: %d", pspd->sSchemeSelected));
            }
        }
        break;

        /*
         * ID_XSDI_SOUND_SCHEMES_SAVEAS:
         *      "Save as..." button:
         *      store new sound scheme in OS2SYS.INI
         */

        case ID_XSDI_SOUND_SCHEMES_SAVEAS:
            SaveSoundSchemeAs(pnbp,
                              TRUE); // select the new scheme
        break;

        /*
         * ID_XSDI_SOUND_SCHEMES_DELETE:
         *      "Delete..." button:
         *      delete current sound scheme
         *      (button is disabled if none is selected)
         */

        case ID_XSDI_SOUND_SCHEMES_DELETE:
        {
            CHAR    szCurrentScheme[200];
            PCSZ     // pszCorrespondingINIApp = 0,
                    pszTemp = szCurrentScheme;
            WinQueryWindowText(pspd->hwndSchemesDropDown,
                               sizeof(szCurrentScheme),
                               szCurrentScheme);

            if (cmnMessageBoxExt(pnbp->hwndFrame,
                                    151,
                                    &pszTemp, 1,
                                    154,    // "sure?"
                                    MB_YESNO)
                    == MBID_YES)
            {
                sndDestroySoundScheme(szCurrentScheme);

                // update drop-down list
                pspd->fDontConfirmSchemeSelection = TRUE;
                FillDropDownWithSchemes(pspd->hwndSchemesDropDown);

                // select "none" pseudo-scheme
                SelectSoundScheme(pspd, LIT_NONE);
                // now re-enable controls
                pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
            }
        }
        break;

        /*
         * ID_XSDI_SOUND_EVENTSLISTBOX:
         *      item selected in "Events" listbox:
         *      update SOUNDPAGEDATA and controls
         */

        case ID_XSDI_SOUND_EVENTSLISTBOX:
        {
            HINI hiniMMPM = PrfOpenProfile(pspd->hab,
                                           pspd->szMMPM);
            if (hiniMMPM)
            {
                ULONG   cbData = 0;
                PSZ     pszSoundData;
                CHAR    szSoundIndex[20];

                pspd->sEventSelected = winhQueryLboxSelectedItem(pspd->hwndEventsListbox,
                                                              LIT_FIRST);

                // retrieve the list box item handle we stored
                // previously; this is the sound index into MMPM.INI
                pspd->ulSoundIndex = (ULONG)WinSendMsg(pspd->hwndEventsListbox,
                                                       LM_QUERYITEMHANDLE,
                                                       (MPARAM)pspd->sEventSelected,
                                                       (MPARAM)NULL);

                sprintf(szSoundIndex, "%d", pspd->ulSoundIndex);

                if (pszSoundData = prfhQueryProfileData(hiniMMPM,
                                                        MMINIKEY_SYSSOUNDS,
                                                        szSoundIndex,
                                                        &cbData))
                {
                    if (sndParseSoundData(pszSoundData,
                            pspd->szDescription,       // we don't need the description
                            pspd->szFile,
                            &pspd->ulVolume))
                    {
                        // set entry field to sound file
                        WinSetDlgItemText(pnbp->hwndDlgPage,
                                          ID_XSDI_SOUND_FILE,
                                          pspd->szFile);
                        // set volume lever to either global
                        // or individual sound volume
                        WinSendDlgItemMsg(pnbp->hwndDlgPage,
                                          ID_XSDI_SOUND_VOLUMELEVER,
                                          CSM_SETVALUE,
                                          (MPARAM)((pspd->fCommonVolume)
                                                // "common volume" flag set?
                                                ? pspd->ulCommonVolume
                                                : pspd->ulVolume),
                                          (MPARAM)NULL);
                    }

                    free(pszSoundData);
                }

                PrfCloseProfile(hiniMMPM);
            }
        }
        break;

        /*
         * ID_XSDI_SOUND_FILE:
         *      "File" entry field changed
         */

        case ID_XSDI_SOUND_FILE:
            switch (usNotifyCode)
            {
                case EN_SETFOCUS:
                    pspd->fSoundFileChanged = FALSE;
                break;

                case EN_KILLFOCUS:
                    // focus leaves entry field:
                    // update MMPM.INI with the new sound file
                    if (pspd->fSoundFileChanged)
                        UpdateMMPMINI(pnbp);
                break;

                case EN_CHANGE:
                    // text has changed: update SOUNDPAGEDATA
                    WinQueryWindowText(pspd->hwndSoundFile,
                                       sizeof(pspd->szFile),
                                       pspd->szFile);
                    pspd->fSoundFileChanged = TRUE;
                    // and re-enable controls
                    pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
                break;
            }
        break;

        /*
         * ID_XSDI_SOUND_BROWSE:
         *      "Browse" button: open file dialog
         */

        case ID_XSDI_SOUND_BROWSE:
        {
            CHAR szFile[CCHMAXPATH] = "*";

            if (cmnFileDlg(pnbp->hwndFrame,        // V0.9.16 (2001-10-19) [umoeller]
                           szFile,
                           WINH_FOD_INILOADDIR | WINH_FOD_INISAVEDIR,
                           HINI_USER,
                           INIAPP_XWORKPLACE,
                           INIKEY_XWPSOUNDLASTDIR)) // "XWPSound:LastDir"
            {
                // copy file from FOD to page
                WinSetDlgItemText(pnbp->hwndDlgPage,
                                  ID_XSDI_SOUND_FILE,
                                  szFile);
                // rewrite that one sound MMPM.INI
                UpdateMMPMINI(pnbp);
            }
        }
        break;

        /*
         * ID_XSDI_SOUND_PLAY:
         *      "Play" button:
         *      have the Speedy thread play this file
         */

        case ID_XSDI_SOUND_PLAY:
            xmmPostPartyMsg(XMM_PLAYSOUND,
                            (MPARAM)strdup(pspd->szFile),
                                    // the quick thread wants to free()
                                    // the PSZ passed to it
                            (MPARAM)pspd->ulVolume);

        break;

        /*
         * ID_XSDI_SOUND_COMMONVOLUME:
         *      ulExtra has new checkbox state;
         *      update SOUNDPAGEDATA and MMPM.INI
         */

        case ID_XSDI_SOUND_COMMONVOLUME:
        {
            HINI hiniMMPM = PrfOpenProfile(pspd->hab,
                                           pspd->szMMPM);
            pspd->fCommonVolume = ulExtra;
            if (hiniMMPM)
            {
                // "enabled" in MMPM.INI
                PrfWriteProfileString(hiniMMPM,
                        MMINIKEY_SOUNDSETTINGS, "ApplyVolumeToAll",
                        (ulExtra) ? "TRUE" : "FALSE");
                PrfCloseProfile(hiniMMPM);
            }

            // adjust volume lever
            WinSendDlgItemMsg(pnbp->hwndDlgPage,
                              ID_XSDI_SOUND_VOLUMELEVER,
                              CSM_SETVALUE,
                              (MPARAM)((pspd->fCommonVolume)
                                    // "common volume" flag set?
                                    ? pspd->ulCommonVolume
                                    : pspd->ulVolume),
                              (MPARAM)NULL);

            // re-enable controls
            pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
        }
        break;

        /*
         * ID_XSDI_SOUND_VOLUMELEVER:
         *      "Volume" circular slider:
         *      ulExtra has new value;
         *      update SOUNDPAGEDATA
         */

        case ID_XSDI_SOUND_VOLUMELEVER:
        {
            if (usNotifyCode == CSN_CHANGED)
            {
                if (pspd->fCommonVolume)
                {
                    // "common volume" flag set:

                    // write it back to MMPM.INI
                    HINI hiniMMPM = PrfOpenProfile(pspd->hab,
                                                   pspd->szMMPM);

                    if (hiniMMPM)
                    {
                        CHAR szTemp[100];
                        sprintf(szTemp, "%d", ulExtra);
                        PrfWriteProfileString(hiniMMPM,
                                MMINIKEY_SOUNDSETTINGS,
                                "Volume",
                                szTemp);
                        PrfCloseProfile(hiniMMPM);
                    }
                    // update common volume
                    pspd->ulCommonVolume = ulExtra;
                }
                else
                {
                    // update individual sound volume
                    pspd->ulVolume = ulExtra;

                }
            }
            else if (usNotifyCode == CSN_SETFOCUS)
                // ulExtra is FALSE only if we're losing the focus
                if (ulExtra == FALSE)
                    UpdateMMPMINI(pnbp);
        }
        break;

        case DID_UNDO:
            // "Undo" button:
            if (pnbp->pUser)
                // have the page updated by calling the callback above
                sndSoundsInitPage(pnbp, CBI_SHOW | CBI_ENABLE);
        break;

        case DID_DEFAULT:
            // "Default" button:
            // have the page updated by calling the callback above
            sndSoundsInitPage(pnbp, CBI_SET | CBI_ENABLE);
        break;
    }

    return (MPARAM)0;
}


