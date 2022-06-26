
/*
 *@@sourcefile filesys_pages.c:
 *      implementation of the replacement "File" pages for
 *      folders and data files.
 *
 *      This used to be in filesys.c, but has been separated
 *      with V1.0.1 (2003-01-05) [umoeller].
 *
 *      Function prefix for this file:
 *      --  fsys*
 *
 *@@added V0.9.0 [umoeller]
 *@@header "filesys\filesys.h"
 */

/*
 *      Copyright (C) 1997-2010 Ulrich M”ller.
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
#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSRESOURCES
#define INCL_DOSERRORS

#define INCL_WINPOINTERS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINENTRYFIELDS
#define INCL_WINLISTBOXES
#define INCL_WINMLE
#include <os2.h>

// C library headers
#include <stdio.h>
#include <setjmp.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\eah.h"                // extended attributes helper routines
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
#include "xfldr.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\notebook.h"            // generic XWorkplace notebook handling

#include "filesys\filesys.h"            // various file-system object implementation code

#include "helpers\xwpsecty.h"           // XWorkplace Security base

// other SOM headers
#pragma hdrstop                 // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   "File" page 1 replacement in WPDataFile/WPFolder
 *
 ********************************************************************/

#define INIT_DATE_TBR     "00.00.0000  "
#define INIT_TIME_TBR     "00:00:00"

#define LEFT_COLUMN_WIDTH           50

#define DATETIME_TABLE_WIDTH        125
            // specified with the datetime group; dialog.c adds spacing around that
#define DATETIME_ACTUAL_WIDTH   (DATETIME_TABLE_WIDTH + (2 * COMMON_SPACING) + (2 * GROUP_INNER_SPACING_X))
            // this is the actual size produced by the dialog formatter

#define ATTR_TABLE_WIDTH            65
            // specified with the attribs group; dialog.c adds spacing around that
#define ATTR_ACTUAL_WIDTH       (ATTR_TABLE_WIDTH + (2 * COMMON_SPACING) + (2 * GROUP_INNER_SPACING_X))
            // this is the actual size produced by the dialog formatter

// now calculate the size of the "information" table; we must specify
// the size of the table, not of the PM group control, so calc reversely
// 1) actual width of the group is the same as the above two actual width
#define INFO_ACTUAL_WIDTH        (DATETIME_ACTUAL_WIDTH + ATTR_ACTUAL_WIDTH)
// 2) inner table width (to be specified) is that without the group spacings
#define INFO_TABLE_WIDTH        (INFO_ACTUAL_WIDTH - (4 * COMMON_SPACING) - (2 * GROUP_INNER_SPACING_X))

#define MLE_WIDTH               ((INFO_TABLE_WIDTH - 2 * COMMON_SPACING) / 2)
#define MLE_HEIGHT              25

#define REAL_NAME_WIDTH         (INFO_ACTUAL_WIDTH - LEFT_COLUMN_WIDTH - 2 * COMMON_SPACING)

static const CONTROLDEF
    RealNameTxt = CONTROLDEF_TEXT(
                            LOAD_STRING,
                            ID_XSDI_FILES_REALNAME_TXT,
                            LEFT_COLUMN_WIDTH,
                            -1),
    RealNameData = CONTROLDEF_TEXT(
                            "W",
                            ID_XSDI_FILES_REALNAME,
                            REAL_NAME_WIDTH,
                            -1),
    SizeTxt = CONTROLDEF_TEXT(
                            LOAD_STRING,
                            ID_XSDI_FILES_FILESIZE_TXT,
                            LEFT_COLUMN_WIDTH,
                            -1),
    SizeData = CONTROLDEF_TEXT(
                            LOAD_STRING,
                            ID_XSDI_FILES_FILESIZE,
                            -1,
                            -1),
    WorkAreaCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_FILES_WORKAREA),
    DateTimeGroup = CONTROLDEF_GROUP(
                            LOAD_STRING,
                            ID_XSDI_FILES_DATETIME_GROUP,
                            DATETIME_TABLE_WIDTH,
                            -1),
    CreationTxt = CONTROLDEF_TEXT(
                            LOAD_STRING,
                            ID_XSDI_FILES_CREATIONDATE_TXT,
                            LEFT_COLUMN_WIDTH,
                            -1),
    CreationDate = CONTROLDEF_TEXT(
                            INIT_DATE_TBR,
                            ID_XSDI_FILES_CREATIONDATE,
                            -1,
                            -1),
    CreationTime = CONTROLDEF_TEXT(
                            INIT_TIME_TBR,
                            ID_XSDI_FILES_CREATIONTIME,
                            -1,
                            -1),
    LastWriteTxt = CONTROLDEF_TEXT(
                            LOAD_STRING,
                            ID_XSDI_FILES_LASTWRITEDATE_TXT,
                            LEFT_COLUMN_WIDTH,
                            -1),
    LastWriteDate = CONTROLDEF_TEXT(
                            INIT_DATE_TBR,
                            ID_XSDI_FILES_LASTWRITEDATE,
                            -1,
                            -1),
    LastWriteTime = CONTROLDEF_TEXT(
                            INIT_TIME_TBR,
                            ID_XSDI_FILES_LASTWRITETIME,
                            -1,
                            -1),
    LastAccessTxt = CONTROLDEF_TEXT(
                            LOAD_STRING,
                            ID_XSDI_FILES_LASTACCESSDATE_TXT,
                            LEFT_COLUMN_WIDTH,
                            -1),
    LastAccessDate = CONTROLDEF_TEXT(
                            INIT_DATE_TBR,
                            ID_XSDI_FILES_LASTACCESSDATE,
                            -1,
                            -1),
    LastAccessTime = CONTROLDEF_TEXT(
                            INIT_TIME_TBR,
                            ID_XSDI_FILES_LASTACCESSTIME,
                            -1,
                            -1),
    AttrGroup = CONTROLDEF_GROUP(
                            LOAD_STRING,
                            ID_XSDI_FILES_ATTR_GROUP,
                            ATTR_TABLE_WIDTH,
                            -1),
    AttrArchivedCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_FILES_ATTR_ARCHIVED),
    AttrReadOnlyCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_FILES_ATTR_READONLY),
    AttrSystemCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_FILES_ATTR_SYSTEM),
    AttrHiddenCB = LOADDEF_AUTOCHECKBOX(ID_XSDI_FILES_ATTR_HIDDEN),
    InfoGroup = CONTROLDEF_GROUP(
                            LOAD_STRING,
                            ID_XSDI_FILES_INFO_GROUP,
                            INFO_TABLE_WIDTH,
                            -1),
    SubjectTxt = CONTROLDEF_TEXT(
                            LOAD_STRING,
                            ID_XSDI_FILES_SUBJECT_TXT,
                            LEFT_COLUMN_WIDTH,
                            -1),
    SubjectEF = CONTROLDEF_ENTRYFIELD(
                            NULL,
                            ID_XSDI_FILES_SUBJECT,
                            INFO_TABLE_WIDTH - LEFT_COLUMN_WIDTH - 2 * COMMON_SPACING,
                            -1),
    CommentsTxt = CONTROLDEF_TEXT(
                            LOAD_STRING,
                            ID_XSDI_FILES_COMMENTS_TXT,
                            -1,
                            -1),
    CommentsMLE = CONTROLDEF_MLE(
                            NULL,
                            ID_XSDI_FILES_COMMENTS,
                            MLE_WIDTH,
                            MLE_HEIGHT),
    KeyphrasesTxt = CONTROLDEF_TEXT(
                            LOAD_STRING,
                            ID_XSDI_FILES_KEYPHRASES_TXT,
                            -1,
                            -1),
    KeyphrasesMLE = CONTROLDEF_MLE(
                            NULL,
                            ID_XSDI_FILES_KEYPHRASES,
                            MLE_WIDTH,
                            MLE_HEIGHT);

static const DLGHITEM G_dlgFile1[] =
    {
        START_TABLE,            // root table, required
            START_ROW(0),
                CONTROL_DEF(&RealNameTxt),
                CONTROL_DEF(&RealNameData),
            START_ROW(0),
                CONTROL_DEF(&SizeTxt),
                CONTROL_DEF(&SizeData),
            START_ROW(0),
                START_TABLE,
                    START_ROW(0),
                        CONTROL_DEF(&WorkAreaCB),
                    START_ROW(0),
                        START_GROUP_TABLE(&DateTimeGroup),
                            START_ROW(ROW_VALIGN_CENTER),
                                CONTROL_DEF(&CreationTxt),
                                CONTROL_DEF(&CreationDate),
                                CONTROL_DEF(&CreationTime),
                            START_ROW(ROW_VALIGN_CENTER),
                                CONTROL_DEF(&LastWriteTxt),
                                CONTROL_DEF(&LastWriteDate),
                                CONTROL_DEF(&LastWriteTime),
                            START_ROW(ROW_VALIGN_CENTER),
                                CONTROL_DEF(&LastAccessTxt),
                                CONTROL_DEF(&LastAccessDate),
                                CONTROL_DEF(&LastAccessTime),
                        END_TABLE,
                END_TABLE,
                START_GROUP_TABLE(&AttrGroup),
                    START_ROW(0),
                        CONTROL_DEF(&AttrArchivedCB),
                    START_ROW(0),
                        CONTROL_DEF(&AttrReadOnlyCB),
                    START_ROW(0),
                        CONTROL_DEF(&AttrSystemCB),
                    START_ROW(0),
                        CONTROL_DEF(&AttrHiddenCB),
                END_TABLE,
            START_ROW(0),
                START_GROUP_TABLE(&InfoGroup),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&SubjectTxt),
                        CONTROL_DEF(&SubjectEF),
                    START_ROW(0),
                        START_TABLE,
                            START_ROW(0),
                                CONTROL_DEF(&CommentsTxt),
                            START_ROW(0),
                                CONTROL_DEF(&CommentsMLE),
                        END_TABLE,
                        START_TABLE,
                            START_ROW(0),
                                CONTROL_DEF(&KeyphrasesTxt),
                            START_ROW(0),
                                CONTROL_DEF(&KeyphrasesMLE),
                        END_TABLE,
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE
    };

static MPARAM G_ampFile1Page[] =
    {
        MPFROM2SHORT(ID_XSDI_FILES_REALNAME_TXT, XAC_MOVEY),
        MPFROM2SHORT(ID_XSDI_FILES_REALNAME, XAC_MOVEY | XAC_SIZEX),
        MPFROM2SHORT(ID_XSDI_FILES_FILESIZE_TXT, XAC_MOVEY),
        MPFROM2SHORT(ID_XSDI_FILES_FILESIZE, XAC_MOVEY | XAC_SIZEX),
        MPFROM2SHORT(ID_XSDI_FILES_WORKAREA, XAC_MOVEY),
        MPFROM2SHORT(ID_XSDI_FILES_DATETIME_GROUP, XAC_MOVEY | XAC_SIZEX),
        MPFROM2SHORT(ID_XSDI_FILES_CREATIONDATE_TXT, XAC_MOVEY | XAC_SIZEX),
        MPFROM2SHORT(ID_XSDI_FILES_CREATIONDATE, XAC_MOVEY | XAC_MOVEX),
        MPFROM2SHORT(ID_XSDI_FILES_CREATIONTIME, XAC_MOVEY | XAC_MOVEX),
        MPFROM2SHORT(ID_XSDI_FILES_LASTWRITEDATE_TXT, XAC_MOVEY | XAC_SIZEX),
        MPFROM2SHORT(ID_XSDI_FILES_LASTWRITEDATE, XAC_MOVEY | XAC_MOVEX),
        MPFROM2SHORT(ID_XSDI_FILES_LASTWRITETIME, XAC_MOVEY | XAC_MOVEX),
        MPFROM2SHORT(ID_XSDI_FILES_LASTACCESSDATE_TXT, XAC_MOVEY | XAC_SIZEX),
        MPFROM2SHORT(ID_XSDI_FILES_LASTACCESSDATE, XAC_MOVEY | XAC_MOVEX),
        MPFROM2SHORT(ID_XSDI_FILES_LASTACCESSTIME, XAC_MOVEY | XAC_MOVEX),
        MPFROM2SHORT(ID_XSDI_FILES_ATTR_GROUP, XAC_MOVEX | XAC_MOVEY),
        MPFROM2SHORT(ID_XSDI_FILES_ATTR_ARCHIVED, XAC_MOVEX | XAC_MOVEY),
        MPFROM2SHORT(ID_XSDI_FILES_ATTR_READONLY, XAC_MOVEX | XAC_MOVEY),
        MPFROM2SHORT(ID_XSDI_FILES_ATTR_SYSTEM, XAC_MOVEX | XAC_MOVEY),
        MPFROM2SHORT(ID_XSDI_FILES_ATTR_HIDDEN, XAC_MOVEX | XAC_MOVEY),
        MPFROM2SHORT(ID_XSDI_FILES_INFO_GROUP, XAC_SIZEX | XAC_SIZEY),
        MPFROM2SHORT(ID_XSDI_FILES_SUBJECT_TXT, XAC_MOVEY),
        MPFROM2SHORT(ID_XSDI_FILES_SUBJECT, XAC_SIZEX | XAC_MOVEY),
        MPFROM2SHORT(ID_XSDI_FILES_COMMENTS_TXT, XAC_MOVEY),
        MPFROM2SHORT(ID_XSDI_FILES_COMMENTS, XAC_SIZEX | XAC_SIZEY),
        MPFROM2SHORT(ID_XSDI_FILES_KEYPHRASES_TXT, XAC_MOVEX | XAC_MOVEY),
        MPFROM2SHORT(ID_XSDI_FILES_KEYPHRASES, XAC_MOVEX | XAC_SIZEY)
    };

STATIC ULONG G_cFile1Page = ARRAYITEMCOUNT(G_ampFile1Page);

/*
 *@@ FILEPAGEDATA:
 *      structure used for backing up file page data
 *      (Undo button on "File" page).
 */

typedef struct _FILEPAGEDATA
{
    // file attributes backup
    ULONG       ulAttr;
    // EA backups
    PSZ         pszSubject,
                pszComments,
                pszKeyphrases;
} FILEPAGEDATA, *PFILEPAGEDATA;

/*
 *@@ SetDlgDateTime:
 *
 *@@added V0.9.18 (2002-02-06) [umoeller]
 */

STATIC VOID SetDlgDateTime(HWND hwndDlg,           // in: dialog
                           ULONG idDate,           // in: dialog item ID for date string
                           ULONG idTime,           // in: dialog item ID for time string
                           PFDATE pfDate,          // in: file info
                           PFTIME pfTime,
                           PCOUNTRYSETTINGS2 pcs)   // in: country settings
{
    CHAR    szTemp[100];

    nlsFileDate(szTemp,
                pfDate,
                pcs);
    WinSetDlgItemText(hwndDlg,
                      idDate,
                      szTemp);

    nlsFileTime(szTemp,
                pfTime,
                pcs);
    WinSetDlgItemText(hwndDlg,
                      idTime,
                      szTemp);
}

/*
 *@@ fsysFile1InitPage:
 *      first "File" page notebook callback function (notebook.c).
 *      Sets the controls on the page according to a file's
 *      filesystem characteristics.
 *
 *      This is used by both XFolder and XFldDataFile.
 *
 *      Notes about the EAs which are parsed here (also see the
 *      "Extended Attributes" section in CPREF):
 *
 *      --  The "Subject" field on the page corresponds to
 *          the .SUBJECT EA. This is a single-type EAT_ASCII EA.
 *
 *      --  The "Comments" field corresponds to the .COMMENTS EA.
 *          This is multi-value, multi-type (EAT_MVMT), but all the
 *          subvalues are of EAT_ASCII. All lines in the default
 *          WPS "File" multi-line entry field terminated by CR/LF
 *          are put in one of those subvalues.
 *
 *      --  The "Keyphrases" field corresponds to .KEYPHRASES.
 *          This is also EAT_MVMT and used like .COMMENTS.
 *
 *@@changed V0.9.1 (2000-01-22) [umoeller]: renamed from fsysFileInitPage
 *@@changed V0.9.18 (2002-03-19) [umoeller]: now refreshing page when turned back to
 *@@changed V0.9.19 (2002-04-13) [umoeller]: now using dialog formatter, made page resizeable
 *@@changed V1.0.9 (2010-07-17) [pr]: added large file support @@fixes 586
 */

VOID fsysFile1InitPage(PNOTEBOOKPAGE pnbp,    // notebook info struct
                       ULONG flFlags)                // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        // first call: backup instance data for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        CHAR            szFilename[CCHMAXPATH];
        PFILEPAGEDATA   pfpd = (PFILEPAGEDATA)malloc(sizeof(FILEPAGEDATA));
        memset(pfpd, 0, sizeof(FILEPAGEDATA));
        pnbp->pUser = pfpd;
        pfpd->ulAttr = _wpQueryAttr(pnbp->inbp.somSelf);
        _wpQueryFilename(pnbp->inbp.somSelf, szFilename, TRUE);
        pfpd->pszSubject = fsysQueryEASubject(pnbp->inbp.somSelf);
        pfpd->pszComments = fsysQueryEAComments(pnbp->inbp.somSelf);
        pfpd->pszKeyphrases = fsysQueryEAKeyphrases(pnbp->inbp.somSelf);

        // insert the controls using the dialog formatter
        // V0.9.19 (2002-04-14) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      G_dlgFile1,
                      ARRAYITEMCOUNT(G_dlgFile1));

        if (doshIsFileOnFAT(szFilename))
        {
            // on FAT: hide fields
            winhShowDlgItem(pnbp->hwndDlgPage, ID_XSDI_FILES_CREATIONDATE,
                            FALSE);
            winhShowDlgItem(pnbp->hwndDlgPage, ID_XSDI_FILES_LASTACCESSDATE,
                            FALSE);
        }

        if (!_somIsA(pnbp->inbp.somSelf, _WPFolder))
        {
            // this page is not for a folder, but
            // a data file:
            // hide "Work area" item
            winhShowDlgItem(pnbp->hwndDlgPage, ID_XSDI_FILES_WORKAREA, FALSE);
        }
        else if (cmnIsADesktop(pnbp->inbp.somSelf))
            // for the Desktop, disable work area;
            // this must not be changed
            WinEnableControl(pnbp->hwndDlgPage, ID_XSDI_FILES_WORKAREA,
                              FALSE);

        // Even though CPREF says that the .SUBJECT EA was limited to
        // 40 chars altogether, this is wrong apparently, as many users
        // have said after V0.9.1; so limit the entry field to 260 chars
        WinSendDlgItemMsg(pnbp->hwndDlgPage, ID_XSDI_FILES_SUBJECT,
                          EM_SETTEXTLIMIT,
                          (MPARAM)(260), MPNULL);
    }

    if (flFlags & (CBI_SET | CBI_SHOW))     // refresh page when turned back to
                                            // V0.9.18 (2002-03-19) [umoeller]
    {
        // prepare file date/time etc. for display in window
        CHAR    szFilename[CCHMAXPATH];
        CHAR    szTemp[100];
        PSZ     pszString = NULL;
        ULONG   ulAttr;
        FILESTATUS3L fs3;

        // on network drives, this can take a second or so
        // V0.9.19 (2002-04-24) [umoeller]
        HPOINTER hptrOld = winhSetWaitPointer();

        PCOUNTRYSETTINGS2 pcs = cmnQueryCountrySettings(FALSE);

        // get file-system object information
        // (we don't use the WPS method because the data
        // in there is frequently outdated)
        _wpQueryFilename(pnbp->inbp.somSelf, szFilename, TRUE);
        DosQueryPathInfo(szFilename,
                        FIL_STANDARDL,
                        &fs3, sizeof(fs3));

        // real name
        WinSetDlgItemText(pnbp->hwndDlgPage,
                          ID_XSDI_FILES_REALNAME,
                          szFilename);

        // file size
        nlsThousandsDouble(szTemp,
                           65536.0 * 65536.0 * fs3.cbFile.ulHi + fs3.cbFile.ulLo,
                           pcs->cs.cThousands);  // V1.0.9
        WinSetDlgItemText(pnbp->hwndDlgPage, ID_XSDI_FILES_FILESIZE, szTemp);

        // for folders: set work-area flag
        if (_somIsA(pnbp->inbp.somSelf, _WPFolder))
            // this page is not for a folder, but
            // a data file:
            // hide "Work area" item
            winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_FILES_WORKAREA,
                                  ((_wpQueryFldrFlags(pnbp->inbp.somSelf) & FOI_WORKAREA) != 0));

        // creation date/time
        SetDlgDateTime(pnbp->hwndDlgPage,
                       ID_XSDI_FILES_CREATIONDATE,
                       ID_XSDI_FILES_CREATIONTIME,
                       &fs3.fdateCreation,
                       &fs3.ftimeCreation,
                       pcs);
        // last write date/time
        SetDlgDateTime(pnbp->hwndDlgPage,
                       ID_XSDI_FILES_LASTWRITEDATE,
                       ID_XSDI_FILES_LASTWRITETIME,
                       &fs3.fdateLastWrite,
                       &fs3.ftimeLastWrite,
                       pcs);
        // last access date/time
        SetDlgDateTime(pnbp->hwndDlgPage,
                       ID_XSDI_FILES_LASTACCESSDATE,
                       ID_XSDI_FILES_LASTACCESSTIME,
                       &fs3.fdateLastAccess,
                       &fs3.ftimeLastAccess,
                       pcs);

        // attributes
        ulAttr = _wpQueryAttr(pnbp->inbp.somSelf);
        winhSetDlgItemChecked(pnbp->hwndDlgPage,
                              ID_XSDI_FILES_ATTR_ARCHIVED,
                              ((ulAttr & FILE_ARCHIVED) != 0));
        winhSetDlgItemChecked(pnbp->hwndDlgPage,
                              ID_XSDI_FILES_ATTR_READONLY,
                              ((ulAttr & FILE_READONLY) != 0));
        winhSetDlgItemChecked(pnbp->hwndDlgPage,
                              ID_XSDI_FILES_ATTR_HIDDEN,
                              ((ulAttr & FILE_HIDDEN) != 0));
        winhSetDlgItemChecked(pnbp->hwndDlgPage,
                              ID_XSDI_FILES_ATTR_SYSTEM,
                              ((ulAttr & FILE_SYSTEM) != 0));

        // .SUBJECT EA; this is plain text
        pszString = fsysQueryEASubject(pnbp->inbp.somSelf);
        WinSetDlgItemText(pnbp->hwndDlgPage, ID_XSDI_FILES_SUBJECT,
                          pszString);
        if (pszString)
            free(pszString);

        // .COMMENTS EA; this is multi-value multi-type, but all
        // of the sub-types are EAT_ASCII. We need to convert
        // the sub-items into one string and separate the items
        // with CR/LF.
        pszString = fsysQueryEAComments(pnbp->inbp.somSelf);
        WinSetDlgItemText(pnbp->hwndDlgPage, ID_XSDI_FILES_COMMENTS,
                          pszString);
        if (pszString)
            free(pszString);

        // .KEYPHRASES EA; this is multi-value multi-type, but all
        // of the sub-types are EAT_ASCII. We need to convert
        // the sub-items into one string and separate the items
        // with CR/LF.
        pszString = fsysQueryEAKeyphrases(pnbp->inbp.somSelf);
        WinSetDlgItemText(pnbp->hwndDlgPage, ID_XSDI_FILES_KEYPHRASES,
                        pszString);
        if (pszString)
            free(pszString);

        WinSetPointer(HWND_DESKTOP, hptrOld);
    }

    if (flFlags & CBI_DESTROY)
    {
        // notebook page is being destroyed:
        // free the backup EAs we created before
        PFILEPAGEDATA   pfpd = pnbp->pUser;

        if (pfpd->pszSubject)
            free(pfpd->pszSubject);
        if (pfpd->pszComments)
            free(pfpd->pszComments);
        if (pfpd->pszKeyphrases)
            free(pfpd->pszKeyphrases);

        // the pnbp->pUser field itself is free()'d automatically
    }
}

/*
 *@@ fsysFile1ItemChanged:
 *      first "File" page notebook callback function (notebook.c).
 *      Reacts to changes of any of the dialog controls.
 *
 *      This is used by both XFolder and XFldDataFile.
 *
 *@@changed V0.9.1 (2000-01-22) [umoeller]: renamed from fsysFile1InitPage
 *@@changed V0.9.19 (2002-04-13) [umoeller]: now using dialog formatter, made page resizeable
 */

MRESULT fsysFile1ItemChanged(PNOTEBOOKPAGE pnbp,    // notebook info struct
                             ULONG ulItemID,
                             USHORT usNotifyCode,
                             ULONG ulExtra)      // for checkboxes: contains new state
{
    BOOL fUpdate = TRUE;

    switch (ulItemID)
    {
        case ID_XSDI_FILES_WORKAREA:
            if (_somIsA(pnbp->inbp.somSelf, _WPFolder))
            {
                ULONG ulFlags = _wpQueryFldrFlags(pnbp->inbp.somSelf);
                if (ulExtra)
                    // checked:
                    ulFlags |= FOI_WORKAREA;
                else
                    ulFlags &= ~FOI_WORKAREA;
                _wpSetFldrFlags(pnbp->inbp.somSelf, ulFlags);
            }
        break;

        /*
         * ID_XSDI_FILES_ATTR_ARCHIVED:
         * ID_XSDI_FILES_ATTR_READONLY:
         * ID_XSDI_FILES_ATTR_HIDDEN:
         * ID_XSDI_FILES_ATTR_SYSTEM:
         *      any "Attributes" flag clicked:
         *      change file attributes
         */

        case ID_XSDI_FILES_ATTR_ARCHIVED:
        case ID_XSDI_FILES_ATTR_READONLY:
        case ID_XSDI_FILES_ATTR_HIDDEN:
        case ID_XSDI_FILES_ATTR_SYSTEM:
        {
            ULONG       ulFileAttr;

            ulFileAttr = _wpQueryAttr(pnbp->inbp.somSelf);

            // toggle file attribute
            ulFileAttr ^= // XOR flag depending on item checked
                      (ulItemID == ID_XSDI_FILES_ATTR_ARCHIVED) ? FILE_ARCHIVED
                    : (ulItemID == ID_XSDI_FILES_ATTR_SYSTEM  ) ? FILE_SYSTEM
                    : (ulItemID == ID_XSDI_FILES_ATTR_HIDDEN  ) ? FILE_HIDDEN
                    : FILE_READONLY;

            _wpSetAttr(pnbp->inbp.somSelf, ulFileAttr);
        }
        break;

        /*
         * ID_XSDI_FILES_SUBJECT:
         *      when focus leaves .SUBJECT entry field,
         *      rewrite plain EAT_ASCII EA
         */

        case ID_XSDI_FILES_SUBJECT:
            if (usNotifyCode == EN_KILLFOCUS)
            {
                PSZ         pszSubject = NULL;
                pszSubject = winhQueryWindowText(WinWindowFromID(pnbp->hwndDlgPage,
                                                                 ID_XSDI_FILES_SUBJECT));
                fsysSetEASubject(pnbp->inbp.somSelf, pszSubject);
                if (pszSubject)
                    free(pszSubject);
            }
        break;

        /*
         * ID_XSDI_FILES_COMMENTS:
         *      when focus leaves .COMMENTS MLE,
         *      rewrite EAT_MVMT EA
         */

        case ID_XSDI_FILES_COMMENTS:
            if (usNotifyCode == MLN_KILLFOCUS)
            {
                HWND    hwndMLE = WinWindowFromID(pnbp->hwndDlgPage, ulItemID);
                PSZ     pszText = winhQueryWindowText(hwndMLE);
                fsysSetEAComments(pnbp->inbp.somSelf, pszText);
                if (pszText)
                    free(pszText);
            }
        break;

        /*
         * ID_XSDI_FILES_KEYPHRASES:
         *      when focus leaves .KEYPHRASES MLE,
         *      rewrite EAT_MVMT EA
         */

        case ID_XSDI_FILES_KEYPHRASES:
            if (usNotifyCode == MLN_KILLFOCUS)
            {
                HWND    hwndMLE = WinWindowFromID(pnbp->hwndDlgPage, ulItemID);
                PSZ     pszText = winhQueryWindowText(hwndMLE);
                fsysSetEAKeyphrases(pnbp->inbp.somSelf, pszText);
                if (pszText)
                    free(pszText);
            }
        break;

        case DID_UNDO:
            if (pnbp->pUser)
            {
                // restore the file's data from the backup data
                PFILEPAGEDATA   pfpd = (PFILEPAGEDATA)pnbp->pUser;

                // reset attributes
                _wpSetAttr(pnbp->inbp.somSelf, pfpd->ulAttr);

                // reset EAs
                fsysSetEASubject(pnbp->inbp.somSelf, pfpd->pszSubject); // can be NULL
                fsysSetEAComments(pnbp->inbp.somSelf, pfpd->pszComments); // can be NULL
                fsysSetEAKeyphrases(pnbp->inbp.somSelf, pfpd->pszKeyphrases); // can be NULL

                // have the page updated by calling the callback above
                fsysFile1InitPage(pnbp, CBI_SET | CBI_ENABLE);
            }
        break;

        case DID_DEFAULT:
        {
            // "Default" button:
            ULONG           ulAttr = 0;
            // EABINDING       eab;
            if (_somIsA(pnbp->inbp.somSelf, _WPFolder))
                ulAttr = FILE_DIRECTORY;
            // reset attributes
            _wpSetAttr(pnbp->inbp.somSelf, ulAttr);

            // delete EAs
            fsysSetEASubject(pnbp->inbp.somSelf, NULL);
            fsysSetEAComments(pnbp->inbp.somSelf, NULL);
            fsysSetEAKeyphrases(pnbp->inbp.somSelf, NULL);

            // have the page updated by calling the callback above
            fsysFile1InitPage(pnbp, CBI_SET | CBI_ENABLE);
        }
        break;

        default:
            fUpdate = FALSE;
        break;
    }

    if (fUpdate)
        _wpSaveDeferred(pnbp->inbp.somSelf);

    return (MPARAM)-1;
}

/* ******************************************************************
 *
 *   "File" page 2 replacement in WPDataFile/WPFolder
 *
 ********************************************************************/

#ifndef __NOFILEPAGE2__

/*
 *@@ fsysFile2InitPage:
 *      second "File" page notebook callback function (notebook.c).
 *      Sets the controls on the page according to a file's
 *      filesystem characteristics.
 *
 *      This is used by both XFolder and XFldDataFile.
 *
 *@@added V0.9.1 (2000-01-22) [umoeller]
 *@@changed V0.9.18 (2002-03-19) [umoeller]: now refreshing page when turned back to
 */

VOID fsysFile2InitPage(PNOTEBOOKPAGE pnbp,    // notebook info struct
                       ULONG flFlags)                // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        HWND hwndContents = WinWindowFromID(pnbp->hwndDlgPage, ID_XSDI_FILES_EACONTENTS);
        winhSetWindowFont(hwndContents, "8.Courier");
        WinSendMsg(hwndContents, MLM_SETREADONLY, (MPARAM)TRUE, 0);
    }

    if (flFlags & (CBI_SET | CBI_SHOW))     // refresh page when turned back to
                                            // V0.9.18 (2002-03-19) [umoeller]
    {
        CHAR szFilename[CCHMAXPATH];
        if (_wpQueryFilename(pnbp->inbp.somSelf, szFilename, TRUE))
        {
            PEALIST peal = eaPathReadAll(szFilename),
                    pealThis = peal;
            HWND hwndEAList = WinWindowFromID(pnbp->hwndDlgPage, ID_XSDI_FILES_EALIST);
            winhDeleteAllItems(hwndEAList);         // V0.9.18 (2002-03-19) [umoeller]
            while (pealThis)
            {
                PEABINDING peabThis = pealThis->peab;
                if (peabThis)
                {
                    WinInsertLboxItem(hwndEAList,
                                      LIT_END,
                                      peabThis->pszName);
                }

                pealThis = pealThis->next;
            }

            eaFreeList(peal);

            // clear info fields, this might be a refresh
            // V0.9.18 (2002-03-19) [umoeller]
            WinSetDlgItemText(pnbp->hwndDlgPage,
                              ID_XSDI_FILES_EAINFO,
                              NULL);
            WinSetDlgItemText(pnbp->hwndDlgPage,
                              ID_XSDI_FILES_EACONTENTS,
                              NULL);
        }
    }
}

/*
 *@@ fsysFile2ItemChanged:
 *      second "File" page notebook callback function (notebook.c).
 *      Reacts to changes of any of the dialog controls.
 *
 *      This is used by both XFolder and XFldDataFile.
 *
 *@@added V0.9.1 (2000-01-22) [umoeller]
 */

MRESULT fsysFile2ItemChanged(PNOTEBOOKPAGE pnbp,    // notebook info struct
                             ULONG ulItemID,
                             USHORT usNotifyCode,
                             ULONG ulExtra)      // for checkboxes: contains new state
{
    switch (ulItemID)
    {
        /*
         * ID_XSDI_FILES_EALIST:
         *      EAs list box.
         */

        case ID_XSDI_FILES_EALIST:
            if (usNotifyCode == LN_SELECT)
            {
                CHAR szFilename[CCHMAXPATH];
                HWND hwndEAList = WinWindowFromID(pnbp->hwndDlgPage, ID_XSDI_FILES_EALIST);
                ULONG ulSelection = (ULONG)WinSendMsg(hwndEAList,
                                                      LM_QUERYSELECTION,
                                                      MPNULL,
                                                      MPNULL);
                if (    (ulSelection != LIT_NONE)
                     && (_wpQueryFilename(pnbp->inbp.somSelf, szFilename, TRUE))
                   )
                {
                    PSZ pszEAName;
                    if (pszEAName = winhQueryLboxItemText(hwndEAList,
                                                          ulSelection))
                    {
                        PEABINDING peab;
                        if (peab = eaPathReadOneByName(szFilename,
                                                       pszEAName))
                        {
                            PSZ     pszContents = NULL;
                            XSTRING strInfo;
                            USHORT  usEAType = eaQueryEAType(peab);
                            CHAR    szTemp[100];
                            BOOL    fDumpBinary = TRUE;

                            xstrInit(&strInfo, 200);
                            xstrcpy(&strInfo, pszEAName, 0);

                            switch (usEAType)
                            {
                                case EAT_BINARY:
                                    xstrcat(&strInfo, " (EAT_BINARY", 0);
                                break;

                                case EAT_ASCII:
                                    xstrcat(&strInfo, " (EAT_ASCII", 0);
                                    pszContents = eaCreatePSZFromBinding(peab);
                                    fDumpBinary = FALSE;
                                break;

                                case EAT_BITMAP:
                                    xstrcat(&strInfo, " (EAT_BITMAP", 0);
                                break;

                                case EAT_METAFILE:
                                    xstrcat(&strInfo, " (EAT_METAFILE", 0);
                                break;

                                case EAT_ICON:
                                    xstrcat(&strInfo, " (EAT_ICON", 0);
                                break;

                                case EAT_EA:
                                    xstrcat(&strInfo, " (EAT_EA", 0);
                                break;

                                case EAT_MVMT:
                                    xstrcat(&strInfo, " (EAT_MVMT", 0);
                                break;

                                case EAT_MVST:
                                    xstrcat(&strInfo, " (EAT_MVST", 0);
                                break;

                                case EAT_ASN1:
                                    xstrcat(&strInfo, " (EAT_ASN1", 0);
                                break;

                                default:
                                {
                                    sprintf(szTemp, " (type 0x%lX", usEAType);
                                    xstrcat(&strInfo, szTemp, 0);
                                }
                            }

                            sprintf(szTemp, ", %d bytes)", peab->usValueLength);
                            xstrcat(&strInfo, szTemp, 0);

                            if (fDumpBinary)
                            {
                                pszContents = strhCreateDump(peab->pszValue,
                                                             peab->usValueLength,
                                                             0);
                            }

                            // set static above MLE
                            WinSetDlgItemText(pnbp->hwndDlgPage,
                                              ID_XSDI_FILES_EAINFO,
                                              strInfo.psz);

                            // set MLE; this might be empty
                            WinSetDlgItemText(pnbp->hwndDlgPage,
                                              ID_XSDI_FILES_EACONTENTS,
                                              pszContents);

                            eaFreeBinding(peab);
                            xstrClear(&strInfo);
                            if (pszContents)
                                free(pszContents);
                        }
                        free(pszEAName);
                    }
                }
            }
        break;
    }

    return 0;
}

#endif

/* ******************************************************************
 *
 *   "File" page 3 (permissions)
 *
 ********************************************************************/

static const CONTROLDEF
    PermText = LOADDEF_TEXT(ID_AMDI_F3_PERMISSIONS_TXT),
    PermData = CONTROLDEF_TEXT("R W C X D A P",
                               ID_AMDI_F3_PERMISSIONS_DATA,
                               SZL_AUTOSIZE,
                               SZL_AUTOSIZE);


static const DLGHITEM G_dlgFile3[] =
    {
        START_TABLE,
            START_ROW(0),
                CONTROL_DEF(&PermText),
                CONTROL_DEF(&PermData),
            START_ROW(0),
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE
    };

/*
 *@@ fsysFile3InitPage:
 *      third "File" page notebook callback function (notebook.c).
 *      Sets the controls on the page according to a file's
 *      permissions.
 *
 *      This is used by both XFolder and XFldDataFile.
 *
 *@@added V1.0.1 (2003-01-10) [umoeller]
 */

VOID fsysFile3InitPage(PNOTEBOOKPAGE pnbp,    // notebook info struct
                       ULONG flFlags)                // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        // insert the controls using the dialog formatter
        // V0.9.19 (2002-04-14) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      G_dlgFile3,
                      ARRAYITEMCOUNT(G_dlgFile3));
    }

    if (flFlags & CBI_SET)
    {
        CHAR    szFilename[CCHMAXPATH];
        ULONG   flAccess;
        if (    (_wpQueryFilename(pnbp->inbp.somSelf, szFilename, TRUE))
             && (!xsecQueryPermissions(szFilename,
                                       &flAccess))
           )
        {
            CHAR szPerms[] = "R W C X D A P";
            //                0123456789012

            static const struct
            {
                ULONG   fl;
                CHAR    c;
            } aPerms[] =
            {
                XWPACCESS_READ,     'r',
                XWPACCESS_WRITE,    'w',
                XWPACCESS_CREATE,   'c',
                XWPACCESS_EXEC,     'x',
                XWPACCESS_DELETE,   'd',
                XWPACCESS_ATRIB,    'a',
                XWPACCESS_PERM,     'p'
            };

            ULONG ul;
            for (ul = 0;
                 ul < ARRAYITEMCOUNT(aPerms);
                 ++ul)
            {
                if (!(flAccess & aPerms[ul].fl))
                    szPerms[ul * 2] = aPerms[ul].c;
            }

            WinSetDlgItemText(pnbp->hwndDlgPage,
                              ID_AMDI_F3_PERMISSIONS_DATA,
                              szPerms);
        }
    }
}

/*
 *@@ fsysFile3ItemChanged:
 *      third "File" page notebook callback function (notebook.c).
 *      Reacts to changes of any of the dialog controls.
 *
 *      This is used by both XFolder and XFldDataFile.
 *
 *@@added V1.0.1 (2003-01-10) [umoeller]
 */

MRESULT fsysFile3ItemChanged(PNOTEBOOKPAGE pnbp,    // notebook info struct
                             ULONG ulItemID,
                             USHORT usNotifyCode,
                             ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT mrc = 0;

    return mrc;
}

/* ******************************************************************
 *
 *   Entry point
 *
 ********************************************************************/

/*
 *@@ fsysInsertFilePages:
 *      shared code between XFldDataFile and XFolder
 *      to insert the new "File" pages if this is
 *      enabled. This gets called from the respective
 *      wpAddFile1Page methods.
 *
 *@@added V0.9.5 (2000-08-14) [umoeller]
 *@@changed V1.0.1 (2003-01-10) [umoeller]: added page subtitles
 *@@changed V1.0.1 (2003-01-10) [umoeller]: added permissions page
 */

ULONG fsysInsertFilePages(WPObject *somSelf,    // in: must be a WPFileSystem, really
                          HWND hwndNotebook)    // in: from wpAddFile1Page
{
    INSERTNOTEBOOKPAGE inbp;

    if (!xsecQueryStatus(NULL))
    {
        // page 3 (permissions) V1.0.1 (2003-01-10) [umoeller]
        memset(&inbp, 0, sizeof(INSERTNOTEBOOKPAGE));
        inbp.somSelf = somSelf;
        inbp.hwndNotebook = hwndNotebook;
        inbp.hmod = cmnQueryNLSModuleHandle(FALSE);
        inbp.ulDlgID = ID_XFD_EMPTYDLG;
        inbp.ulPageID = SP_FILE3_PERMISSIONS;
        inbp.pcszName = cmnGetString(ID_XSSI_FILEPAGE);
        inbp.pcszMinorName = cmnGetString(ID_XSSI_FILEPAGE_PERMISSIONS);    // V1.0.1 (2003-01-10) [umoeller]
        inbp.fEnumerate = TRUE;
        inbp.ulDefaultHelpPanel  = ID_XSH_SETTINGS_FILEPAGE3;
        inbp.pfncbInitPage    = (PFNCBACTION)fsysFile3InitPage;
        inbp.pfncbItemChanged = (PFNCBITEMCHANGED)fsysFile3ItemChanged;

        ntbInsertPage(&inbp);
    }

#ifndef __NOFILEPAGE2__
    // page 2
    memset(&inbp, 0, sizeof(INSERTNOTEBOOKPAGE));
    inbp.somSelf = somSelf;
    inbp.hwndNotebook = hwndNotebook;
    inbp.hmod = cmnQueryNLSModuleHandle(FALSE);
    inbp.ulDlgID = ID_XSD_FILESPAGE2;
    inbp.ulPageID = SP_FILE2;
    inbp.pcszName = cmnGetString(ID_XSSI_FILEPAGE);
    inbp.pcszMinorName = cmnGetString(ID_XSSI_FILEPAGE_EAS);    // V1.0.1 (2003-01-10) [umoeller]
    inbp.fEnumerate = TRUE;
    inbp.ulDefaultHelpPanel  = ID_XSH_SETTINGS_FILEPAGE2;
    inbp.pfncbInitPage    = (PFNCBACTION)fsysFile2InitPage;
    inbp.pfncbItemChanged = (PFNCBITEMCHANGED)fsysFile2ItemChanged;

    ntbInsertPage(&inbp);
#endif

    // page 1
    memset(&inbp, 0, sizeof(INSERTNOTEBOOKPAGE));
    inbp.somSelf = somSelf;
    inbp.hwndNotebook = hwndNotebook;
    inbp.hmod = cmnQueryNLSModuleHandle(FALSE);
    inbp.ulDlgID = ID_XFD_EMPTYDLG; // ID_XSD_FILESPAGE1; V0.9.19 (2002-04-13) [umoeller]
    inbp.ulPageID = SP_FILE1;
    inbp.usPageStyleFlags = BKA_MAJOR;
    inbp.pcszName = cmnGetString(ID_XSSI_FILEPAGE);  // pszFilePage
    inbp.pcszMinorName = cmnGetString(ID_XSSI_FILEPAGE_GENERAL);   // V1.0.1 (2003-01-10) [umoeller]
    inbp.fEnumerate = TRUE;
    inbp.ulDefaultHelpPanel  = ID_XSH_SETTINGS_FILEPAGE1;
    inbp.pfncbInitPage    = (PFNCBACTION)fsysFile1InitPage;
    inbp.pfncbItemChanged = (PFNCBITEMCHANGED)fsysFile1ItemChanged;
    // make this page sizeable V0.9.19 (2002-04-13) [umoeller]
    inbp.pampControlFlags = G_ampFile1Page;
    inbp.cControlFlags = G_cFile1Page;

    return ntbInsertPage(&inbp);
}


