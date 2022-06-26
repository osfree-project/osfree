
/*
 *@@sourcefile xsetup.c:
 *      this file contains the implementation of the XWPSetup
 *      class. This is in the shared directory because this
 *      affects all installed classes.
 *
 *@@header "shared\xsetup.h"
 */

/*
 *      Copyright (C) 1997-2009 Ulrich M”ller.
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

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSMODULEMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINPOINTERS
#define INCL_WINMENUS
#define INCL_WINDIALOGS
#define INCL_WINBUTTONS
#define INCL_WINSTATICS
#define INCL_WINLISTBOXES
#define INCL_WINSTDCNR
#define INCL_WINSTDSLIDER
#define INCL_WINSTDSPIN
#define INCL_WINSYS
#define INCL_WINSHELLDATA       // Prf* functions

#define INCL_GPILOGCOLORTABLE
#define INCL_GPIBITMAPS
#include <os2.h>

// C library headers
#include <stdio.h>
#include <locale.h>
#include <setjmp.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\nlscache.h"           // NLS string cache
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\procstat.h"           // DosQProcStat handling
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\syssound.h"           // system sound helper routines
#include "helpers\threads.h"            // thread helpers
#include "helpers\winh.h"               // PM helper routines
#include "helpers\wphandle.h"           // file-system object handles
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
#include "xwpsetup.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "xwpapi.h"                     // public XWorkplace definitions

#include "shared\cnrsort.h"             // container sort comparison functions
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\errors.h"              // private XWorkplace error codes
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\notebook.h"            // generic XWorkplace notebook handling

#include "config\hookintf.h"            // daemon/hook interface
#include "config\sound.h"               // XWPSound implementation

#include "filesys\fileops.h"            // file operations implementation
#include "filesys\xthreads.h"           // extra XWorkplace threads

#include "media\media.h"                // XWorkplace multimedia support

#include "startshut\apm.h"              // APM power-off for XShutdown

#include "hook\xwphook.h"

// other SOM headers
#pragma hdrstop
#include <wpfsys.h>             // WPFileSystem
#include "xtrash.h"

/* ******************************************************************
 *
 *   Globals
 *
 ********************************************************************/

#ifndef __NOXWPSETUP__

/*
 *@@ FEATURESITEM:
 *      structure used for feature checkboxes
 *      on the "Features" page. Each item
 *      represents one record in the container.
 */

typedef struct _FEATURESITEM
{
    USHORT  usFeatureID;
                // string ID (dlgids.h, *.rc file in NLS DLL) for feature;
                // this also identifies the item for processing
    USHORT  usParentID;
                // string ID of the parent record or null if root record.
                // If you specify a parent record, this must appear before
                // the child record in FeaturesItemsList.
    ULONG   ulStyle;
                // style flags for the record; OR the following:
                // -- WS_VISIBLE: record is visible
                // -- BS_AUTOCHECKBOX: give the record a checkbox
                // For parent records (without checkboxes), use 0 only.
    PSZ     pszNLSString;
                // resolved NLS string; this must be NULL initially.
} FEATURESITEM, *PFEATURESITEM;

/*
 * FeatureItemsList:
 *      array of FEATURESITEM's which are inserted into
 *      the container on the "Features" page.
 *
 *added V0.9.1 (99-12-19) [umoeller]
 */

static FEATURESITEM G_FeatureItemsList[] =
        {
            // general features
            ID_XCSI_GENERALFEATURES, 0, 0, NULL,
#ifndef __NOICONREPLACEMENTS__
            ID_XCSI_REPLACEICONS, ID_XCSI_GENERALFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif
#ifndef __ALWAYSREPLACEHELP__
            ID_XCSI_REPLACEHELP, ID_XCSI_GENERALFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif
#ifndef __ALWAYSFIXCLASSTITLES__
            ID_XCSI_FIXCLASSTITLES, ID_XCSI_GENERALFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif
// #ifndef __ALWAYSRESIZESETTINGSPAGES__        // setting removed
//             ID_XCSI_RESIZESETTINGSPAGES, ID_XCSI_GENERALFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
// #endif
#ifndef __ALWAYSREPLACEICONPAGE__
            ID_XCSI_REPLACEICONPAGE, ID_XCSI_GENERALFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif
#ifndef __ALWAYSREPLACEFILEPAGE__
            ID_XCSI_REPLACEFILEPAGE, ID_XCSI_GENERALFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif
#ifndef __NOXSYSTEMSOUNDS__
            ID_XCSI_XSYSTEMSOUNDS, ID_XCSI_GENERALFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif

            // folder features
            ID_XCSI_FOLDERFEATURES, 0, 0, NULL,
#ifndef __NOCFGSTATUSBARS__
            ID_XCSI_ENABLESTATUSBARS, ID_XCSI_FOLDERFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif

            // V1.0.1 (2002-11-30) [umoeller]
            ID_XCSI_ENABLETOOLBARS, ID_XCSI_FOLDERFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,

#ifndef __NOSNAPTOGRID__
            ID_XCSI_ENABLESNAP2GRID, ID_XCSI_FOLDERFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif
#ifndef __ALWAYSFDRHOTKEYS__
            ID_XCSI_ENABLEFOLDERHOTKEYS, ID_XCSI_FOLDERFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif
#ifndef __ALWAYSEXTSORT__
            ID_XCSI_EXTFOLDERSORT, ID_XCSI_FOLDERFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif
#ifndef __ALWAYSREPLACEREFRESH__
            ID_XCSI_REPLACEREFRESH, ID_XCSI_FOLDERFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif
#ifndef __NOTURBOFOLDERS__
            ID_XCSI_TURBOFOLDERS, ID_XCSI_FOLDERFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif
            ID_XCSI_FDRSPLITVIEWS, ID_XCSI_FOLDERFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
                    // V1.0.0 (2002-09-09) [umoeller]

            // mouse/keyboard features
            ID_XCSI_MOUSEKEYBOARDFEATURES, 0, 0, NULL,
#ifndef __ALWAYSHOOK__
            ID_XCSI_XWPHOOK, ID_XCSI_MOUSEKEYBOARDFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif
#ifndef __ALWAYSOBJHOTKEYS__
            ID_XCSI_GLOBALHOTKEYS, ID_XCSI_MOUSEKEYBOARDFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif
/*
#ifndef __NOPAGER__
            ID_XCSI_PAGER, ID_XCSI_MOUSEKEYBOARDFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif      removed V0.9.19 (2002-05-28) [umoeller]
*/
            // startup/shutdown features
            ID_XCSI_STARTSHUTFEATURES, 0, 0, NULL,
#ifndef __ALWAYSREPLACEARCHIVING__
            ID_XCSI_ARCHIVING, ID_XCSI_STARTSHUTFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif
#ifndef __NOXSHUTDOWN__
            ID_XCSI_RESTARTWPS, ID_XCSI_STARTSHUTFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
            ID_XCSI_XSHUTDOWN, ID_XCSI_STARTSHUTFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif
#ifndef __NEVERCHECKDESKTOP__
            ID_XCSI_CHECKDESKTOP, ID_XCSI_STARTSHUTFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
            ID_XCSI_PREPOPULATEDESKTOP, ID_XCSI_STARTSHUTFEATURES, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif
            // file operations
            ID_XCSI_FILEOPERATIONS, 0, 0, NULL,

// #ifndef __NEVEREXTASSOCS__       removed V1.0.1 (2002-12-15) [umoeller]
//             ID_XCSI_EXTASSOCS, ID_XCSI_FILEOPERATIONS, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
// #endif
            // ID_XCSI_CLEANUPINIS, ID_XCSI_FILEOPERATIONS, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
                    // removed for now V0.9.12 (2001-05-15) [umoeller]
#ifndef __ALWAYSREPLACEFILEEXISTS__
            ID_XCSI_REPLFILEEXISTS, ID_XCSI_FILEOPERATIONS, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif
#ifndef __NEVERREPLACEDRIVENOTREADY__
            ID_XCSI_REPLDRIVENOTREADY, ID_XCSI_FILEOPERATIONS, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif

            // V0.9.20 (2002-08-08) [umoeller]
            ID_XCSI_REPLACEPASTE, ID_XCSI_FILEOPERATIONS, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,

#ifndef __ALWAYSTRASHANDTRUEDELETE__
            ID_XCSI_REPLACEDELETE, ID_XCSI_FILEOPERATIONS, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif

#ifdef __REPLHANDLES__
            ID_XCSI_REPLHANDLES, ID_XCSI_FILEOPERATIONS, WS_VISIBLE | BS_AUTOCHECKBOX, NULL,
#endif
            // ID_XCSI_REPLACEREFRESH, ID_XCSI_FILEOPERATIONS, WS_VISIBLE | BS_AUTOCHECKBOX, NULL
                // moved this up to the folders group
                // V0.9.16 (2001-10-25) [umoeller]
#ifndef __NEVERNEWFILEDLG__
            ID_XCSI_NEWFILEDLG, ID_XCSI_FILEOPERATIONS, WS_VISIBLE | BS_AUTOCHECKBOX, NULL
#endif
        };

static PCHECKBOXRECORDCORE G_pFeatureRecordsList = NULL;

#endif // __NOXWPSETUP__

/*
 *@@ STANDARDOBJECT:
 *      structure used for XWPSetup "Objects" page.
 *      Each of these represents an object to be
 *      created from the menu buttons.
 *
 *@@changed V0.9.4 (2000-07-15) [umoeller]: added pExists
 *@@changed V0.9.9 (2001-04-07) [pr]: added pcszLocation
 */

typedef struct _STANDARDOBJECT
{
    const char  **ppcszDefaultID;   // e.g. <WP_DRIVES>;
    const char  **ppcszObjectClass;   // e.g. "WPDrives"
    const char  *pcszLocation;      // e.g. "<WP_CONFIG>"
    const char  *pcszSetupString;   // e.g. "DETAILSFONT=..."; if present, _always_
                                    // put a semicolon at the end, because "OBJECTID=xxx"
                                    // will always be added
    USHORT      usMenuID;           // corresponding menu ID in xfldr001.rc resources

    WPObject    *pExists;           // != NULL if object exists already
} STANDARDOBJECT, *PSTANDARDOBJECT;

#define OBJECTSIDFIRST 100      // first object menu ID, inclusive
#define OBJECTSIDLAST  230      // last object menu ID, inclusive

// array of objects for "Standard Desktop objects" menu button
static STANDARDOBJECT G_WPSObjects[] =
    {
            &WPOBJID_KEYB, &G_pcszWPKeyboard, "<WP_CONFIG>", "", 100, 0,
            &WPOBJID_MOUSE, &G_pcszWPMouse, "<WP_CONFIG>", "", 101, 0,
            &WPOBJID_CNTRY, &G_pcszWPCountry, "<WP_CONFIG>", "", 102, 0,
            &WPOBJID_SOUND, &G_pcszWPSound, "<WP_CONFIG>", "", 103, 0,
            &WPOBJID_SYSTEM, &G_pcszWPSystem, "<WP_CONFIG>",
                    "HELPPANEL=9259;", 104, 0, // V0.9.9
            &WPOBJID_POWER, &G_pcszWPPower, "<WP_CONFIG>", "", 105, 0,
            &WPOBJID_WINCFG, &G_pcszWPWinConfig, "<WP_CONFIG>", "", 106, 0,

            &WPOBJID_HIRESCLRPAL, &G_pcszWPColorPalette, "<WP_CONFIG>",
                    "NODELETETE=YES;AUTOSETUP=HIRES;", 110, 0,
            &WPOBJID_LORESCLRPAL, &G_pcszWPColorPalette, "<WP_CONFIG>",
                    "NODELETETE=YES;AUTOSETUP=LORES;", 111, 0,
            &WPOBJID_FNTPAL, &G_pcszWPFontPalette, "<WP_CONFIG>",
                    "NODELETE=YES;", 112, 0,
            &WPOBJID_SCHPAL96, &G_pcszWPSchemePalette, "<WP_CONFIG>",
                    "NODELETE=YES;AUTOSETUP=YES;", 113, 0,

            &WPOBJID_LAUNCHPAD, &G_pcszWPLaunchPad, "<WP_OS2SYS>", "", 120, 0,
            &WPOBJID_WARPCENTER, &G_pcszSmartCenter, "<WP_OS2SYS>", "", 121, 0,

            &WPOBJID_SPOOL, &G_pcszWPSpool, "<WP_CONFIG>", "", 130, 0,
            &WPOBJID_VIEWER, &G_pcszWPMinWinViewer, "<WP_OS2SYS>",
                    "ALWAYSSORT=YES;", 131, 0,
            &WPOBJID_SHRED, &G_pcszWPShredder, "<WP_DESKTOP>", "", 132, 0,
            &WPOBJID_CLOCK, &G_pcszWPClock, "<WP_CONFIG>", "", 133, 0,

            &WPOBJID_START, &G_pcszWPStartup, "<WP_OS2SYS>",
                    "HELPPANEL=8002;NODELETE=YES;", 140, 0,
            &WPOBJID_TEMPS, &G_pcszWPTemplates, "<WP_OS2SYS>",
                    "HELPPANEL=15680;NODELETE=YES;", 141, 0,
            &WPOBJID_DRIVES, &G_pcszWPDrives, "<WP_CONNECTIONSFOLDER>",
                    "ALWAYSSORT=YES;NODELETE=YES;DEFAULTVIEW=ICON;", 142, 0
    },

// array of objects for "XWorkplace objects" menu button
    G_XWPObjects[] =
    {
            &XFOLDER_WPSID, &G_pcszXFldWPS, "<WP_CONFIG>",
                    "",
                    200, 0,
#ifndef __NOOS2KERNEL__
            &XFOLDER_KERNELID, &G_pcszXFldSystem, "<WP_CONFIG>",
                    "",
                    201, 0,
#endif
            &XFOLDER_SCREENID, &G_pcszXWPScreen, "<WP_CONFIG>",
                    "",
                    203, 0,
#ifndef __XWPLITE__
            &XFOLDER_MEDIAID, &G_pcszXWPMedia, "<WP_CONFIG>",
                    "",
                    204, 0,
            &XFOLDER_CLASSLISTID, &G_pcszXWPClassList, "<WP_CONFIG>",
                    "",
                    202, 0,
#endif

            &XFOLDER_CONFIGID, &G_pcszWPFolder, "<XWP_MAINFLDR>",
                    "ICONVIEW=NONFLOWED,MINI;ALWAYSSORT=NO;",
                    210, 0,
#ifndef __NOXWPSTARTUP__
            &XFOLDER_STARTUPID, &G_pcszXFldStartup, "<XWP_MAINFLDR>",
                    "ICONVIEW=NONFLOWED,MINI;ALWAYSSORT=NO;",
                    211, 0,
#endif
#ifndef __NOXSHUTDOWN__
            &XFOLDER_SHUTDOWNID, &G_pcszXFldShutdown, "<XWP_MAINFLDR>",
                    "ICONVIEW=NONFLOWED,MINI;ALWAYSSORT=NO;",
                    212, 0,
#endif
            &XFOLDER_FONTFOLDERID, &G_pcszXWPFontFolder, "<WP_CONFIG>", // V0.9.9
                    "DEFAULTVIEW=DETAILS;DETAILSCLASS=XWPFontObject;SORTCLASS=XWPFontObject;",  // added SORTCLASS V0.9.9 (2001-04-07) [umoeller]
                    213, 0,

            &XFOLDER_TRASHCANID, &G_pcszXWPTrashCan, "<WP_DESKTOP>",
                    "DEFAULTVIEW=DETAILS;ALWAYSSORT=YES;DETAILSCLASS=XWPTrashObject;SORTCLASS=XWPTrashObject;",
                                // added DEFAULTVIEW=DETAILS;ALWAYSSORT=YES
                    220, 0,
            &XFOLDER_STRINGTPLID, &G_pcszXWPString, "<XWP_MAINFLDR>", // V0.9.9
                    "TEMPLATE=YES;",
                    221, 0,
            &XFOLDER_XCENTERID, &G_pcszXCenterReal, "<XWP_MAINFLDR>",
                    "",
                    230, 0
    };

/* ******************************************************************
 *
 *   XWPSetup helper functions
 *
 ********************************************************************/

#ifndef __NOXWPSETUP__

/*
 *@@ AddResourceDLLToLB:
 *      this loads a given DLL temporarily in order to
 *      find out if it's an XFolder NLS DLL; if so,
 *      its language string is loaded and a descriptive
 *      string is inserted into a given list box
 *      (used for "XFolder Internals" settings page).
 *
 *@@changed V0.8.5 [umoeller]: language string now initialized to ""
 */

STATIC VOID AddResourceDLLToLB(HWND hwndDlg,                   // in: dlg with listbox
                               ULONG idLB,                     // in: listbox item ID
                               PSZ pszXFolderBasePath,         // in: from cmnQueryXWPBasePath
                               PSZ pszFileName)
{
    CHAR    szLBEntry[2*CCHMAXPATH] = "",   // changed V0.85
            szResourceModuleName[2*CCHMAXPATH];
    HMODULE hmodDLL = NULLHANDLE;
    APIRET  arc;

    PMPF_LANGCODES(("entering, %s", pszFileName));

    strlcpy(szResourceModuleName, pszXFolderBasePath, sizeof(szResourceModuleName));
    strlcat(szResourceModuleName, "\\bin\\", sizeof(szResourceModuleName));
    strlcat(szResourceModuleName, pszFileName, sizeof(szResourceModuleName));

    arc = DosLoadModule(NULL, 0,
                        szResourceModuleName,
                        &hmodDLL);

    PMPF_LANGCODES(("    Loading module '%s', arc: %d", szResourceModuleName, arc));

    if (arc == NO_ERROR)
    {
        PMPF_LANGCODES(("    Testing for language string"));

        if (WinLoadString(WinQueryAnchorBlock(hwndDlg),
                          hmodDLL,
                          ID_XSSI_DLLLANGUAGE,
                          sizeof(szLBEntry), szLBEntry))
        {
            PMPF_LANGCODES(("      --> found %s", szLBEntry));

            strcat(szLBEntry, " -- ");
            strcat(szLBEntry, pszFileName);

            WinSendDlgItemMsg(hwndDlg, idLB,
                              LM_INSERTITEM,
                              (MPARAM)LIT_SORTASCENDING,
                              (MPARAM)szLBEntry);
        }
        else
            PMPF_LANGCODES(("      --> language string not found"));

        DosFreeModule(hmodDLL);
    }
    else
        PMPF_LANGCODES(("    Error %d", arc));
}

/* ******************************************************************
 *
 *   XWPSetup "Installed classes" dialog
 *
 ********************************************************************/

typedef const char  ***REQ;

/*
 *@@ XWPCLASSITEM:
 *
 *@@added V0.9.14 (2001-07-31) [umoeller]
 */

typedef struct _XWPCLASSITEM
{
    const char  **ppcszClassName;           // XWorkplace class name (e.g. "XWPProgram");
                                            // ptr to global string in common.h
    const char  **ppcszReplacesClass;       // if this replaces a class (e.g. "WPProgram"),
                                            // ptr to global string in common.h

    REQ         pRequirements;              // ptr to an array of const char** ptrs
                                            // if this class requires other
                                            // classes to be installed; NULL otherwise
    ULONG       cRequirements;              // count of items in that array or 0

    ULONG       ulToolTipID;                // TMF msg ID for tooltip or 0

} XWPCLASSITEM, *PXWPCLASSITEM;

typedef const struct _XWPCLASSITEM *PCXWPCLASSITEM;

/*
 *@@ XWPCLASSES:
 *      structure used for fnwpXWorkplaceClasses
 *      (the "XWorkplace Classes" setup dialog).
 */

typedef struct _XWPCLASSES
{
    HWND    hwndTooltip;
    PSZ     pszTooltipString;
} XWPCLASSES, *PXWPCLASSES;

/*
 *@@ RegisterArray:
 *
 *@@added V0.9.14 (2001-07-31) [umoeller]
 */

STATIC VOID RegisterArray(HWND hwndDlg,
                          HWND hwndTooltip,
                          PTOOLINFO pti,
                          ULONG ulFirstID,
                          PBYTE pObjClass,
                          PCXWPCLASSITEM paClasses,
                          ULONG cClasses)
{
    ULONG ul;
    for (ul = 0;
         ul < cClasses;
         ul++)
    {
        PCXWPCLASSITEM pThis = &paClasses[ul];
        HWND hwndCtl;
        if (hwndCtl = WinWindowFromID(hwndDlg, ulFirstID + ul))
        {
            // add tool to tooltip control
            pti->hwndTool = hwndCtl;
            WinSendMsg(hwndTooltip,
                       TTM_ADDTOOL,
                       (MPARAM)0,
                       pti);
        }
        winhSetDlgItemChecked(hwndDlg,
                              ulFirstID + ul,
                              (winhQueryWPSClass(pObjClass,
                                                 *(pThis->ppcszClassName))
                                    != 0));
    }
}

static const char **G_RequirementsXFldStartupShutdown[] =
    {
        &G_pcszXFldDesktop,
        &G_pcszXFolder
    };

static const char **G_RequirementsXWPTrashCan[] =
    {
        &G_pcszXFolder,
        &G_pcszXWPTrashObject
    };

static const char **G_RequirementsXWPFontFolder[] =
    {
        &G_pcszXFolder,
        &G_pcszXWPFontObject,
        &G_pcszXWPFontFile
    };

static const char **G_RequiresXFolderOnly[] =
    {
        &G_pcszXFolder
    };

/*
 *@@ G_aClasses:
 *      the static array of all classes that XWorkplace
 *      is made of.
 *
 *      This was redone with V0.9.14. All information that
 *      the "Classes" dialog knows about is now contained
 *      in this one array. The dialog is now dynamically
 *      formatted so adding or removing classes is very
 *      easy now... just change this array.
 *
 *@@added V0.9.14 (2001-07-31) [umoeller]
 */

static const XWPCLASSITEM G_aClasses[] =
    {
        // class replacements
        &G_pcszXFldObject, &G_pcszWPObject,
            (REQ)-1, 0,
            1251,
        &G_pcszXWPFileSystem, &G_pcszWPFileSystem,
            NULL, 0,
            1272,
        &G_pcszXFolder, &G_pcszWPFolder,
            NULL, 0,
            1252,
        &G_pcszXFldDisk, &G_pcszWPDisk,
            G_RequiresXFolderOnly, ARRAYITEMCOUNT(G_RequiresXFolderOnly),
            1253,
        &G_pcszXFldDesktop, &G_pcszWPDesktop,
            G_RequiresXFolderOnly, ARRAYITEMCOUNT(G_RequiresXFolderOnly),
            1254,
        &G_pcszXFldDataFile, &G_pcszWPDataFile,
            NULL, 0,
            1255,
        &G_pcszXWPProgram, &G_pcszWPProgram,
            NULL, 0,
            1273,           // V0.9.16 (2001-11-25) [umoeller]
        &G_pcszXWPProgramFile, &G_pcszWPProgramFile,
            NULL, 0,
            1256,
        &G_pcszXWPSound, &G_pcszWPSound,
            NULL, 0,
            1257,
        &G_pcszXWPMouse, &G_pcszWPMouse,
            NULL, 0,
            1258,
        &G_pcszXWPKeyboard, &G_pcszWPKeyboard,
            NULL, 0,
            1259,

        // new classes
        &G_pcszXWPSetup, NULL,
            (REQ)-1, 0,
            1260,
#ifndef __NOOS2KERNEL__
        &G_pcszXFldSystem, NULL,
            NULL, 0,
            1261,
#endif
        &G_pcszXFldWPS, NULL,
            (REQ)-1, 0,
            1262,
        &G_pcszXWPScreen, NULL,
            NULL, 0,
            1267,
#ifndef __XWPLITE__
        &G_pcszXWPMedia, NULL,
            NULL, 0,
            1269,
#endif
#ifndef __NOXWPSTARTUP__
        &G_pcszXFldStartup, NULL,
            G_RequirementsXFldStartupShutdown, ARRAYITEMCOUNT(G_RequirementsXFldStartupShutdown),
            1263,
#endif
#ifndef __NOXSHUTDOWN__
        &G_pcszXFldShutdown, NULL,
            G_RequirementsXFldStartupShutdown, ARRAYITEMCOUNT(G_RequirementsXFldStartupShutdown),
            1264,
#endif

#ifndef __XWPLITE__
        &G_pcszXWPClassList, NULL,
            NULL, 0,
            1265,
#endif
        &G_pcszXWPTrashCan, NULL,
            G_RequirementsXWPTrashCan, ARRAYITEMCOUNT(G_RequirementsXWPTrashCan),
            1266,
        &G_pcszXWPTrashObject, NULL,
            G_RequiresXFolderOnly, ARRAYITEMCOUNT(G_RequiresXFolderOnly),
            1266,
        &G_pcszXWPString, NULL,
            NULL, 0,
            1268,
        &G_pcszXCenterReal, NULL,
            NULL, 0,
            1270,
        &G_pcszXWPFontFolder, NULL,
            G_RequirementsXWPFontFolder, ARRAYITEMCOUNT(G_RequirementsXWPFontFolder),
            1271,
        &G_pcszXWPFontFile, NULL,
            NULL, 0,
            1271,
        &G_pcszXWPFontObject, NULL,
            NULL, 0,
            1271,

        // XWPVCard added with V0.9.16 (2002-01-05) [umoeller]
        &G_pcszXWPVCard, NULL,
            NULL, 0,
            1274
    };

#define ID_CLASSES_FIRST         1000

/*
 *@@ HandleEnableItems:
 *
 *@@added V0.9.14 (2001-07-31) [umoeller]
 */

STATIC VOID HandleEnableItems(HWND hwndDlg)
{
    HPOINTER    hptrOld = winhSetWaitPointer();
    PBYTE pObjClass = winhQueryWPSClassList();
    PBOOL pafDisables = malloc(sizeof(BOOL) * ARRAYITEMCOUNT(G_aClasses));
    ULONG ul;

    memset(pafDisables, 0, sizeof(BOOL) * ARRAYITEMCOUNT(G_aClasses));

    for (ul = 0;
         ul < ARRAYITEMCOUNT(G_aClasses);
         ul++)
    {
        // if the class is installed and requires others,
        // disable that other class
        PCXWPCLASSITEM pThis = &G_aClasses[ul];
        if (    (pThis->pRequirements)
             && (winhIsDlgItemChecked(hwndDlg,
                                      ID_CLASSES_FIRST + ul))
           )
        {
            if (pThis->pRequirements == (REQ)-1)
                pafDisables[ul] = TRUE;
            else
            {
                // go thru the requirements
                ULONG ulR;
                for (ulR = 0;
                     ulR < pThis->cRequirements;
                     ulR++)
                {
                    PCSZ pcszRequirementThis = *(pThis->pRequirements[ulR]);

                    // go find that class in the array
                    ULONG ul2;
                    for (ul2 = 0;
                         ul2 < ARRAYITEMCOUNT(G_aClasses);
                         ul2++)
                    {
                        if (!strcmp(*(G_aClasses[ul2].ppcszClassName),
                                    pcszRequirementThis))
                        {
                            pafDisables[ul2] = TRUE;
                            break;
                        }
                    }
                }
            }
        }
    }

    for (ul = 0;
         ul < ARRAYITEMCOUNT(G_aClasses);
         ul++)
    {
        WinEnableControl(hwndDlg,
                         ID_CLASSES_FIRST + ul,
                         !(pafDisables[ul]));
        if (pafDisables[ul])
            winhSetDlgItemChecked(hwndDlg,
                                  ID_CLASSES_FIRST + ul,
                                  TRUE);
    }

    free(pObjClass);
    free(pafDisables);
    WinSetPointer(HWND_DESKTOP, hptrOld);
}

/*
 *@@ HandleTooltip:
 *
 *@@added V0.9.14 (2001-07-31) [umoeller]
 */

STATIC VOID HandleTooltip(HWND hwndDlg,
                          MPARAM mp2)
{
    PXWPCLASSES     pxwpc = WinQueryWindowPtr(hwndDlg, QWL_USER);
    PTOOLTIPTEXT pttt = (PTOOLTIPTEXT)mp2;
    ULONG   ulDlgID = WinQueryWindowUShort(pttt->hwndTool,
                                           QWS_ID);
    LONG lIndex = ulDlgID - ID_CLASSES_FIRST;
    ULONG   ulWritten = 0;
    APIRET  arc = 0;

    if (pxwpc->pszTooltipString)
    {
        // old string allocated:
        free(pxwpc->pszTooltipString);
        pxwpc->pszTooltipString = NULL;
    }

    if (    (lIndex >= 0)
         && (lIndex < ARRAYITEMCOUNT(G_aClasses))
       )
    {
        ULONG ulID;
        if (ulID = G_aClasses[lIndex].ulToolTipID)
        {
            CHAR    szMessageID[200];
            XSTRING str;
            xstrInit(&str, 0);

            sprintf(szMessageID,
                    "XWPCLS_%04d",
                    ulID);
            cmnGetMessageExt(NULL,                   // pTable
                             0,                      // cTable
                             &str,
                             szMessageID);           // pszMessageName

            pxwpc->pszTooltipString = str.psz;
                    // do not free!
        }
    }

    if (!pxwpc->pszTooltipString)
        // error:
        pxwpc->pszTooltipString = strdup("No help available yet.");

    pttt->ulFormat = TTFMT_PSZ;
    pttt->pszText = pxwpc->pszTooltipString;
}

/*
 *@@ HandleOKButton:
 *
 *@@added V0.9.14 (2001-07-31) [umoeller]
 */

STATIC BOOL HandleOKButton(HWND hwndDlg)
{
    BOOL            fDismiss = TRUE;
    XSTRING         strDereg,
                    strReg,
                    strReplace,
                    strUnreplace;
    BOOL            fDereg = FALSE,
                    fReg = FALSE;

    PBYTE           pObjClass = winhQueryWPSClassList();

    ULONG           ul = 0;

    xstrInit(&strDereg, 0);
    xstrInit(&strReg, 0);
    xstrInit(&strReplace, 0);
    xstrInit(&strUnreplace, 0);

    for (ul = 0;
         ul < ARRAYITEMCOUNT(G_aClasses);
         ul++)
    {
        PCXWPCLASSITEM pThis = &G_aClasses[ul];
        PCSZ pcszClassName = *(pThis->ppcszClassName);
        BOOL fChecked = winhIsDlgItemChecked(hwndDlg,
                                             ID_CLASSES_FIRST + ul);
        BOOL fInstalled = (winhQueryWPSClass(pObjClass,
                                             pcszClassName)
                                != 0);
        if (fChecked && !fInstalled)
        {
            // register
            xstrcat(&strReg, pcszClassName, 0);
            xstrcatc(&strReg, '\n');

            if (pThis->ppcszReplacesClass)
            {
                // replace
                xstrcat(&strReplace, *(pThis->ppcszReplacesClass), 0);
                xstrcatc(&strReplace, ' ');
                xstrcat(&strReplace, pcszClassName, 0);
                xstrcatc(&strReplace, '\n');
                        // "WPObject XFldObject\n"
            }
        }
        else if (!fChecked && fInstalled)
        {
            // deregister
            xstrcat(&strDereg, pcszClassName, 0);
            xstrcatc(&strDereg, '\n');

            if (pThis->ppcszReplacesClass)
            {
                // replace
                xstrcat(&strUnreplace, *(pThis->ppcszReplacesClass), 0);
                xstrcatc(&strUnreplace, ' ');
                xstrcat(&strUnreplace, pcszClassName, 0);
                xstrcatc(&strUnreplace, '\n');
                        // "WPObject XFldObject\n"
            }
        }
    }

    // check if we have anything to do
    fReg = (strReg.ulLength != 0);
    fDereg = (strDereg.ulLength != 0);

    if ((fReg) || (fDereg))
    {
        // OK, class selections have changed:
        XSTRING         strMessage;
        XSTRING         strTemp;

        xstrInit(&strMessage, 100);
        xstrInit(&strTemp, 0);
        // compose confirmation string
        cmnGetMessage(NULL, 0,
                      &strMessage,
                      142); // "You have made the following changes to the XWorkplace class setup:"
        xstrcatc(&strMessage, '\n');
        if (fReg)
        {
            xstrcat(&strMessage, "\n", 0);
            cmnGetMessage(NULL, 0,
                          &strTemp,
                          143); // "Register the following classes:"
            xstrcats(&strMessage, &strTemp);
            xstrcat(&strMessage, "\n\n", 0);
            xstrcats(&strMessage, &strReg);
            if (strReplace.ulLength)
            {
                xstrcatc(&strMessage, '\n');
                cmnGetMessage(NULL, 0,
                              &strTemp,
                              144); // "Replace the following classes:"
                xstrcats(&strMessage, &strTemp);
                xstrcat(&strMessage, "\n\n", 0);
                xstrcats(&strMessage, &strReplace);
            }
        }
        if (fDereg)
        {
            xstrcatc(&strMessage, '\n');
            cmnGetMessage(NULL, 0,
                          &strTemp,
                          145); // "Deregister the following classes:"
            xstrcats(&strMessage, &strTemp);
            xstrcat(&strMessage, "\n\n", 0);
            xstrcats(&strMessage, &strDereg);
            if (strUnreplace.ulLength)
            {
                xstrcatc(&strMessage, '\n');
                cmnGetMessage(NULL, 0,
                              &strTemp,
                              146); // "Undo the following class replacements:"
                xstrcats(&strMessage, &strTemp);
                xstrcat(&strMessage, "\n\n", 0);
                xstrcats(&strMessage, &strUnreplace);
            }
        }
        xstrcatc(&strMessage, '\n');
        cmnGetMessage(NULL, 0,
                      &strTemp,
                      147); // "Are you sure you want to do this?"
        xstrcats(&strMessage, &strTemp);

        // confirm class list changes
        if (cmnMessageBox(hwndDlg,
                          "XWorkplace Setup",
                          strMessage.psz,
                          NULLHANDLE, // no help
                          MB_YESNO) == MBID_YES)
        {
            // "Yes" pressed: go!!
            PSZ         p = NULL;
            XSTRING     strFailing;
            HPOINTER    hptrOld = winhSetWaitPointer();

            xstrInit(&strFailing, 0);

            // unreplace classes
            p = strUnreplace.psz;
            if (p)
                while (*p)
                {
                    // string components: "OldClass NewClass\n"
                    PSZ     pSpace = strchr(p, ' '),
                            pEOL = strchr(pSpace, '\n');
                    if ((pSpace) && (pEOL))
                    {
                        PSZ     pszReplacedClass = strhSubstr(p, pSpace),
                                pszReplacementClass = strhSubstr(pSpace + 1, pEOL);
                        if ((pszReplacedClass) && (pszReplacementClass))
                        {
                            if (!WinReplaceObjectClass(pszReplacedClass,
                                                       pszReplacementClass,
                                                       FALSE))  // unreplace!
                            {
                                // error: append to string list
                                xstrcat(&strFailing, pszReplacedClass, 0);
                                xstrcatc(&strFailing, '\n');
                            }
                        }
                        if (pszReplacedClass)
                            free(pszReplacedClass);
                        if (pszReplacementClass)
                            free(pszReplacementClass);
                    }
                    else
                        break;

                    p = pEOL + 1;
                }

            // deregister classes
            if (p = strDereg.psz)
                while (*p)
                {
                    PSZ     pEOL;
                    if (pEOL = strchr(p, '\n'))
                    {
                        PSZ     pszClass;
                        if (pszClass = strhSubstr(p, pEOL))
                        {
                            if (!WinDeregisterObjectClass(pszClass))
                            {
                                // error: append to string list
                                xstrcat(&strFailing, pszClass, 0);
                                xstrcatc(&strFailing, '\n');
                            }
                            free(pszClass);
                        }
                    }
                    else
                        break;

                    p = pEOL + 1;
                }

            // register new classes
            if (p = strReg.psz)
                while (*p)
                {
                    APIRET  arc = NO_ERROR;
                    CHAR    szRegisterError[300];

                    PSZ     pEOL;
                    if (pEOL = strchr(p, '\n'))
                    {
                        PSZ     pszClass;
                        if (pszClass = strhSubstr(p, pEOL))
                        {
                            arc = winhRegisterClass(pszClass,
                                                    cmnQueryMainCodeModuleFilename(),
                                                            // XFolder module
                                                    szRegisterError,
                                                    sizeof(szRegisterError));
                            if (arc != NO_ERROR)
                            {
                                // error: append to string list
                                xstrcat(&strFailing, pszClass, 0);
                                xstrcatc(&strFailing, '\n');
                            }
                            free(pszClass);
                        }
                    }
                    else
                        break;

                    p = pEOL + 1;
                }

            // replace classes
            if (p = strReplace.psz)
                while (*p)
                {
                    // string components: "OldClass NewClass\n"
                    PSZ     pSpace = strchr(p, ' '),
                            pEOL = strchr(pSpace, '\n');
                    if ((pSpace) && (pEOL))
                    {
                        PSZ     pszReplacedClass = strhSubstr(p, pSpace),
                                pszReplacementClass = strhSubstr(pSpace + 1, pEOL);
                        if ((pszReplacedClass) && (pszReplacementClass))
                        {
                            if (!WinReplaceObjectClass(pszReplacedClass,
                                                       pszReplacementClass,
                                                       TRUE))  // replace!
                            {
                                // error: append to string list
                                xstrcat(&strFailing, pszReplacedClass, 0);
                                xstrcatc(&strFailing, '\n');
                            }
                        }
                        if (pszReplacedClass)
                            free(pszReplacedClass);
                        if (pszReplacementClass)
                            free(pszReplacementClass);
                    }
                    else
                        break;

                    p = pEOL + 1;
                }

            WinSetPointer(HWND_DESKTOP, hptrOld);

            // errors?
            if (strFailing.ulLength)
            {
                PCSZ pcsz = strFailing.psz;
                cmnMessageBoxExt(hwndDlg,
                                    148, // "XWorkplace Setup",
                                    &pcsz, 1,
                                    149,  // "errors... %1"
                                    MB_OK);
            }
            else
                cmnMessageBoxExt(hwndDlg,
                                 148, // "XWorkplace Setup",
                                 NULL, 0,
                                 150, // "restart Desktop"
                                 MB_OK);

            xstrClear(&strFailing); // V0.9.14 (2001-07-31) [umoeller]
        }
        else
            // "No" pressed:
            fDismiss = FALSE;

        xstrClear(&strMessage); // V0.9.14 (2001-07-31) [umoeller]
        xstrClear(&strTemp);

    } // end if ((fReg) || (fDereg))

    xstrClear(&strDereg);
    xstrClear(&strReg);
    xstrClear(&strReplace);
    xstrClear(&strUnreplace);

    free(pObjClass);

    return fDismiss;
}

/*
 * fnwpXWorkplaceClasses:
 *      dialog procedure for the "XWorkplace Classes" dialog.
 *
 *@@changed V0.9.3 (2000-04-26) [umoeller]: added generic fonts support
 *@@changed V0.9.3 (2000-04-26) [umoeller]: added new classes
 *@@changed V0.9.5 (2000-08-23) [umoeller]: XWPMedia wasn't working, fixed
 *@@changed V0.9.10 (2001-04-11) [pr]: added XWPFont* classes
 */

MRESULT EXPENTRY fnwpXWorkplaceClasses(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        /*
         * WM_INITDLG:
         *
         */

        case WM_INITDLG:
        {
            PBYTE           pObjClass;
            // ULONG           ul;
            HPOINTER    hptrOld = winhSetWaitPointer();

            // allocate two XWPCLASSES structures and store them in
            // QWL_USER; initially, both structures will be the same,
            // but only the second one will be altered
            PXWPCLASSES     pxwpc = NEW(XWPCLASSES);
            ZERO(pxwpc);
            WinSetWindowPtr(hwndDlg, QWL_USER, pxwpc);

            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);

            cmnSetControlsFont(hwndDlg, 0, 5000);
                    // added V0.9.3 (2000-04-26) [umoeller]

            // load WPS class list
            pObjClass = winhQueryWPSClassList();

            // register tooltip class
            ctlRegisterTooltip(WinQueryAnchorBlock(hwndDlg));

            // create tooltip
            if (pxwpc->hwndTooltip = WinCreateWindow(HWND_DESKTOP,  // parent
                                                     WC_CCTL_TOOLTIP, // wnd class
                                                     "",            // window text
                                                     XWP_TOOLTIP_STYLE,
                                                          // tooltip window style (common.h)
                                                     10, 10, 10, 10,    // window pos and size, ignored
                                                     hwndDlg,       // owner window -- important!
                                                     HWND_TOP,      // hwndInsertBehind, ignored
                                                     DID_TOOLTIP, // window ID, optional
                                                     NULL,          // control data
                                                     NULL))         // presparams
            {
                // tooltip successfully created:
                // add tools (i.e. controls of the dialog)
                // according to the usToolIDs array
                TOOLINFO    ti = {0};
                // HWND        hwndCtl;
                ti.ulFlags = TTF_CENTER_X_ON_TOOL | TTF_POS_Y_BELOW_TOOL | TTF_SUBCLASS;
                ti.hwndToolOwner = hwndDlg;
                ti.pszText = PSZ_TEXTCALLBACK;  // send TTN_NEEDTEXT

                RegisterArray(hwndDlg,
                              pxwpc->hwndTooltip,
                              &ti,
                              ID_CLASSES_FIRST,
                              pObjClass,
                              G_aClasses,
                              ARRAYITEMCOUNT(G_aClasses));

                // set timers
                WinSendMsg(pxwpc->hwndTooltip,
                           TTM_SETDELAYTIME,
                           (MPARAM)TTDT_AUTOPOP,
                           (MPARAM)(20*1000));        // 20 secs for autopop (hide)
            }


            free(pObjClass);

            WinPostMsg(hwndDlg, XM_ENABLEITEMS, 0, 0);

            WinSetPointer(HWND_DESKTOP, hptrOld);
        }
        break;

        /*
         * XM_ENABLEITEMS:
         *
         */

        case XM_ENABLEITEMS:
            HandleEnableItems(hwndDlg);
        break;

        /*
         * WM_COMMAND:
         *
         */

        case WM_COMMAND:
        {
            BOOL    fDismiss = TRUE;

            if ((USHORT)mp1 == DID_OK)
                // "OK" button pressed:
                fDismiss = HandleOKButton(hwndDlg);

            if (fDismiss)
                // dismiss dialog
                mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        }
        break;

        /*
         * WM_CONTROL:
         *
         */

        case WM_CONTROL:
        {
            USHORT  usItemID = SHORT1FROMMP(mp1),
                    usNotifyCode = SHORT2FROMMP(mp1);

            /*
             * DID_TOOLTIP:
             *      "toolinfo" control (comctl.c)
             */

            if (usItemID == DID_TOOLTIP)
            {
                if (usNotifyCode == TTN_NEEDTEXT)
                {
                    HandleTooltip(hwndDlg,
                                  mp2);
                }
            }
            else if (usNotifyCode == BN_CLICKED)
            {
                WinPostMsg(hwndDlg, XM_ENABLEITEMS, 0, 0);
            }
        }
        break;

        /*
         * WM_HELP:
         *
         */

        case WM_HELP:
            cmnDisplayHelp(NULL,        // active desktop
                           ID_XSH_XWP_CLASSESDLG);
        break;

        /*
         * WM_DESTROY:
         *
         */

        case WM_DESTROY:
        {
            PXWPCLASSES     pxwpcOld = WinQueryWindowPtr(hwndDlg, QWL_USER);
            if (pxwpcOld->pszTooltipString)
                free(pxwpcOld->pszTooltipString);

            winhDestroyWindow(&pxwpcOld->hwndTooltip);

            // free two XWPCLASSES structures
            free(pxwpcOld);
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
        }
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ AppendClassesGroup:
 *
 *@@added V0.9.14 (2001-07-31) [umoeller]
 */

STATIC VOID AppendClassesGroup(const CONTROLDEF *pClsOneClass,
                               CONTROLDEF **ppControlDefThis,
                               DLGHITEM **ppDlgItemThis,
                               BOOL fReplacements)
{
    ULONG ul;

    for (ul = 0;
         ul < ARRAYITEMCOUNT(G_aClasses);
         ul++)
    {
        if (    (fReplacements && G_aClasses[ul].ppcszReplacesClass)
             || (!fReplacements && !G_aClasses[ul].ppcszReplacesClass)
           )
        {
            PCONTROLDEF pDef;

            (*ppDlgItemThis)->Type = TYPE_START_NEW_ROW;
            (*ppDlgItemThis)->ul2 = ROW_VALIGN_CENTER;      // V1.0.0 (2002-08-18) [umoeller]
            (*ppDlgItemThis)++;

            // fill the controldef
            pDef = *ppControlDefThis;
            memcpy(pDef,
                   pClsOneClass,
                   sizeof(CONTROLDEF));
            pDef->pcszText = *(G_aClasses[ul].ppcszClassName);
            pDef->usID = ID_CLASSES_FIRST + ul;

            // and append the controldef
            (*ppDlgItemThis)->Type = TYPE_CONTROL_DEF;
            // (*ppDlgItemThis)->pCtlDef = *ppControlDefThis;   V1.0.0 (2002-08-18) [umoeller]
            (*ppDlgItemThis)->ul1 = (ULONG)*ppControlDefThis;
            (*ppDlgItemThis)++;
            (*ppControlDefThis)++;
        }
    }
}

static CONTROLDEF
        ClsOKButton = CONTROLDEF_DEFPUSHBUTTON(
                    NULL,
                    DID_OK,
                    STD_BUTTON_WIDTH, STD_BUTTON_HEIGHT),
        ClsCancelButton = CONTROLDEF_PUSHBUTTON(
                    NULL,
                    DID_CANCEL,
                    STD_BUTTON_WIDTH, STD_BUTTON_HEIGHT),
        ClsHelpButton = CONTROLDEF_HELPPUSHBUTTON(
                    NULL,
                    DID_HELP,
                    STD_BUTTON_WIDTH, STD_BUTTON_HEIGHT),
        ClsReplGroup = LOADDEF_GROUP(
                    ID_XCD_CLASSES_REPLACEMENTS,
                    SZL_AUTOSIZE),
        ClsNewGroup = LOADDEF_GROUP(
                    ID_XCD_CLASSES_NEW,
                    SZL_AUTOSIZE),
        ClsOneClass = CONTROLDEF_AUTOCHECKBOX(
                    NULL,       // text, to be replaced
                    0,          // ID, to be replaced
                    SZL_AUTOSIZE,
                    SZL_AUTOSIZE);

static const DLGHITEM
    dlgClassesFront[] =
    {
        START_TABLE,
            START_ROW(ROW_VALIGN_TOP),
                START_GROUP_TABLE(&ClsReplGroup)
    },

    // here the class replacements are inserted

    dlgClassesMiddle[] =
    {
                END_TABLE,
                START_GROUP_TABLE(&ClsNewGroup)
    },

    // here the new classes are inserted

    dlgClassesTail[] =
    {
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&ClsOKButton),
                CONTROL_DEF(&ClsCancelButton),
                CONTROL_DEF(&ClsHelpButton),
        END_TABLE
    };

/*
 *@@ ShowClassesDlg:
 *      displays the "XWorkplace classes" dialog.
 *
 *      This has been completely rewritten with V0.9.14
 *      to use dynamic dialog formatting now.
 *
 *@@added V0.9.14 (2001-07-31) [umoeller]
 */

STATIC VOID ShowClassesDlg(HWND hwndOwner)
{
    TRY_LOUD(excpt1)
    {
        HWND hwndDlg = NULLHANDLE;
        APIRET arc;

        CONTROLDEF  *paControlDefs = malloc(   (ARRAYITEMCOUNT(G_aClasses) + 2)
                                             * sizeof(CONTROLDEF)),
                    *pControlDefThis = paControlDefs;
        ULONG       cDlgItems =   ARRAYITEMCOUNT(dlgClassesFront)
                                + ARRAYITEMCOUNT(dlgClassesMiddle)
                                + ARRAYITEMCOUNT(dlgClassesTail)
                                  // we need 2 extra items for START_GROUP_TABLE
                                  // for replacements and new classes each
                                // + 4
                                  // for each class, we need a START_ROW(0)
                                  // plus a check box, i.e. 2 per class
                                + 2 * ARRAYITEMCOUNT(G_aClasses);
        DLGHITEM    *paDlgItems = malloc(sizeof(DLGHITEM) * cDlgItems),
                    *pDlgItemThis = paDlgItems;
        ULONG       ul;

        ClsOKButton.pcszText = cmnGetString(DID_OK);
        ClsCancelButton.pcszText = cmnGetString(DID_CANCEL);
        ClsHelpButton.pcszText = cmnGetString(DID_HELP);

        // copy front
        for (ul = 0;
             ul < ARRAYITEMCOUNT(dlgClassesFront);
             ul++)
        {
            memcpy(pDlgItemThis, &dlgClassesFront[ul], sizeof(DLGHITEM));
            pDlgItemThis++;
        }

        // now go create the items for the class replacements
        AppendClassesGroup(&ClsOneClass,
                           &pControlDefThis,
                           &pDlgItemThis,
                           TRUE);

        // copy separator (middle)
        for (ul = 0;
             ul < ARRAYITEMCOUNT(dlgClassesMiddle);
             ul++)
        {
            memcpy(pDlgItemThis, &dlgClassesMiddle[ul], sizeof(DLGHITEM));
            pDlgItemThis++;
        }

        // and for the new classes
        AppendClassesGroup(&ClsOneClass,
                           &pControlDefThis,
                           &pDlgItemThis,
                           FALSE);

        // copy tail
        for (ul = 0;
             ul < ARRAYITEMCOUNT(dlgClassesTail);
             ul++)
        {
            memcpy(pDlgItemThis, &dlgClassesTail[ul], sizeof(DLGHITEM));
            pDlgItemThis++;
        }

        if (!(arc = dlghCreateDlg(&hwndDlg,
                                  hwndOwner,
                                  FCF_FIXED_DLG,
                                  fnwpXWorkplaceClasses,
                                  cmnGetString(ID_XCD_CLASSES_TITLE),
                                  paDlgItems,
                                  cDlgItems,
                                  NULL,
                                  cmnQueryDefaultFont())))
        {
            winhCenterWindow(hwndDlg);
            WinProcessDlg(hwndDlg);
            winhDestroyWindow(&hwndDlg);
        }

        free(paDlgItems);
        free(paControlDefs);
    }
    CATCH(excpt1) {} END_CATCH();
}

/* ******************************************************************
 *
 *   XWPSetup "Logo" page notebook callbacks (notebook.c)
 *
 ********************************************************************/

#ifndef __XWPLITE__

/*
 *@@ XWPSETUPLOGODATA:
 *      window data structure for XWPSetup "Logo" page.
 *
 *@@added V0.9.6 (2000-11-04) [umoeller]
 */

typedef struct _XWPSETUPLOGODATA
{
    HBITMAP     hbmLogo;
    SIZEL       szlLogo;
} XWPSETUPLOGODATA, *PXWPSETUPLOGODATA;

/*
 *@@ setLogoInitPage:
 *      notebook callback function (notebook.c) for the
 *      XWPSetup "Logo" page.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@added V0.9.6 (2000-11-04) [umoeller]
 */

VOID setLogoInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                     ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        HPS hpsTemp;
        PXWPSETUPLOGODATA pLogoData;
        if (    (hpsTemp = WinGetPS(pnbp->hwndDlgPage))
             && (pLogoData = malloc(sizeof(XWPSETUPLOGODATA)))
           )
        {
            memset(pLogoData, 0, sizeof(XWPSETUPLOGODATA));
            pnbp->pUser = pLogoData;

            if (pLogoData->hbmLogo = GpiLoadBitmap(hpsTemp,
                                                   cmnQueryMainResModuleHandle(),
                                                   ID_XWPBIGLOGO,
                                                   0,
                                                   0))
            {
                BITMAPINFOHEADER2 bmih2;
                bmih2.cbFix = sizeof(bmih2);
                if (GpiQueryBitmapInfoHeader(pLogoData->hbmLogo, &bmih2))
                {
                    pLogoData->szlLogo.cx = bmih2.cx;
                    pLogoData->szlLogo.cy = bmih2.cy;
                }
            }
        }

        WinReleasePS(hpsTemp);
    }

    if (flFlags & CBI_DESTROY)
    {
        PXWPSETUPLOGODATA pLogoData = (PXWPSETUPLOGODATA)pnbp->pUser;
        if (pLogoData)
        {
            GpiDeleteBitmap(pLogoData->hbmLogo);
        }
        // pLogoData is freed automatically
    }
}

/*
 *@@ setFeaturesMessages:
 *      notebook callback function (notebook.c) for the
 *      XWPSetup "Logo" page.
 *      This gets really all the messages from the dlg.
 *
 *@@added V0.9.6 (2000-11-04) [umoeller]
 */

BOOL setLogoMessages(PNOTEBOOKPAGE pnbp,
                     ULONG msg, MPARAM mp1, MPARAM mp2,
                     MRESULT *pmrc)
{
    BOOL    brc = FALSE;

    switch (msg)
    {
        case WM_COMMAND:
            switch ((USHORT)mp1)
            {
                case DID_HELP:
                    cmnShowProductInfo(pnbp->hwndDlgPage,
                                       MMSOUND_SYSTEMSTARTUP);
            }
        break;

        case WM_PAINT:
        {
            PXWPSETUPLOGODATA pLogoData = (PXWPSETUPLOGODATA)pnbp->pUser;
            RECTL   rclDlg,
                    rclPaint;
            HPS     hps = WinBeginPaint(pnbp->hwndDlgPage,
                                        NULLHANDLE,
                                        &rclPaint);
            // switch to RGB
            GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULL);
            WinFillRect(hps,
                        &rclPaint,
                        0x00CCCCCC); // 204, 204, 204 -> light gray; that's in the bitmap,
                                     // and it's also the SYSCLR_DIALOGBACKGROUND,
                                     // but just in case the user changed it...
            WinQueryWindowRect(pnbp->hwndDlgPage,
                               &rclDlg);
            if (pLogoData)
            {
                POINTL ptl;
                // center bitmap:
                ptl.x       =   ((rclDlg.xRight - rclDlg.xLeft)
                                  - pLogoData->szlLogo.cx) / 2;
                ptl.y       =   ((rclDlg.yTop - rclDlg.yBottom)
                                  - pLogoData->szlLogo.cy) / 2;
                WinDrawBitmap(hps,
                              pLogoData->hbmLogo,
                              NULL,
                              &ptl,
                              0, 0, DBM_NORMAL);
            }
            WinReleasePS(hps);
            brc = TRUE;
        }
        break;
    }

    return brc;
}

#endif

/* ******************************************************************
 *
 *   XWPSetup "Features" page notebook callbacks (notebook.c)
 *
 ********************************************************************/

/*
 *@@ XWPFEATURESDATA:
 *      window data structure for XWPSetup "Features" page.
 *
 *@@added V0.9.9 (2001-04-05) [pr]
 *@@changed V1.0.5 (2006-05-28) [pr]: removed dummy ___LAST_SETTING
 */

typedef struct _XWPFEATURESDATA
{
    PSETTINGSBACKUP     pBackup;
#ifndef __ALWAYSOBJHOTKEYS__
    BOOL                bObjectHotkeys;
#endif
#ifndef __ALWAYSREPLACEREFRESH__
    BOOL                bReplaceRefresh;
#endif
} XWPFEATURESDATA, *PXWPFEATURESDATA;

static const XWPSETTING G_FeaturesBackup[] =
    {
#ifndef __NOICONREPLACEMENTS__
        sfIconReplacements,
#endif
#ifndef __ALWAYSREPLACEHELP__
        sfHelpReplacements,
#endif
// #ifndef __ALWAYSRESIZESETTINGSPAGES__        setting removed V1.0.1 (2002-12-14) [umoeller]
//         sfResizeSettingsPages,
// #endif
#ifndef __ALWAYSREPLACEICONPAGE__
        sfReplaceIconPage,
#endif
#ifndef __ALWAYSREPLACEFILEPAGE__
        sfReplaceFilePage,
#endif
#ifndef __NOXSYSTEMSOUNDS__
        sfXSystemSounds,
#endif
#ifndef __ALWAYSFIXCLASSTITLES__
        sfFixClassTitles,
#endif
#ifndef __NOCFGSTATUSBARS__
        sfStatusBars,
#endif
        sfToolBars,             // V1.0.1 (2002-11-30) [umoeller]
#ifndef __NOSNAPTOGRID__
        sfSnap2Grid,
#endif
#ifndef __ALWAYSFDRHOTKEYS__
        sfFolderHotkeys,
#endif
#ifndef __ALWAYSEXTSORT__
        sfExtendedSorting,
#endif
#ifndef __NOTURBOFOLDERS__
        sfTurboFolders,
#endif
        sfFdrSplitViews
#ifndef __ALWAYSHOOK__
        ,sfXWPHook
#endif
#ifndef __NOPAGER__
        ,sfEnableXPager
#endif

#ifndef __ALWAYSREPLACEARCHIVING__
        ,sfReplaceArchiving
#endif
#ifndef __NOXSHUTDOWN__
        ,sfRestartDesktop
        ,sfXShutdown
#endif

// #ifndef __NEVEREXTASSOCS__       removed V1.0.1 (2002-12-15) [umoeller]
//         ,sfExtAssocs
// #endif
#ifdef __REPLHANDLES__
        ,sfReplaceHandles
#endif
#ifndef __ALWAYSREPLACEFILEEXISTS__
        ,sfReplaceFileExists
#endif
#ifndef __NEVERREPLACEDRIVENOTREADY__
        ,sfReplaceDriveNotReady
#endif

#ifndef __ALWAYSREPLACEPASTE__
        ,sfReplacePaste             // V0.9.20 (2002-08-08) [umoeller]
#endif

#ifndef __ALWAYSTRASHANDTRUEDELETE__
        ,sfReplaceDelete
#endif

#ifndef __NEVERNEWFILEDLG__
        ,sfNewFileDlg
#endif
    };

static MPARAM G_ampFeaturesPage[] =
    {
        MPFROM2SHORT(ID_XFDI_CNR_GROUPTITLE, XAC_SIZEX | XAC_SIZEY),
        MPFROM2SHORT(ID_XCDI_CONTAINER, XAC_SIZEX | XAC_SIZEY)
    };

/*
 *@@ setFeaturesInitPage:
 *      notebook callback function (notebook.c) for the
 *      XWPSetup "Features" page.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@changed V0.9.1 (2000-02-01) [umoeller]: added global hotkeys flag
 *@@changed V0.9.9 (2001-01-31) [umoeller]: added "replace folder refresh"
 *@@changed V0.9.9 (2001-04-05) [pr]: fix undo
 *@@changed V0.9.12 (2001-05-12) [umoeller]: removed "Cleanup INIs" for now
 *@@changed V0.9.16 (2001-10-25) [umoeller]: added "turbo folders"
 *@@changed V1.0.1 (2002-12-14) [umoeller]: removed "resize settings pages" setting @@fixes 285, 286
 */

VOID setFeaturesInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                         ULONG flFlags)        // CBI_* flags (notebook.h)
{
    PCKERNELGLOBALS  pKernelGlobals = krnQueryGlobals();

    HWND hwndFeaturesCnr = WinWindowFromID(pnbp->hwndDlgPage,
                                           ID_XCDI_CONTAINER);

    if (flFlags & CBI_INIT)
    {
        PCHECKBOXRECORDCORE preccThis,
                            preccParent;
        ULONG               ul,
                            cRecords;
        HAB                 hab = WinQueryAnchorBlock(pnbp->hwndDlgPage);
        HMODULE             hmodNLS = cmnQueryNLSModuleHandle(FALSE);

        PXWPFEATURESDATA pFeaturesData;
        // first call: backup Global Settings for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        pnbp->pUser = pFeaturesData = malloc(sizeof(XWPFEATURESDATA));
        pFeaturesData->pBackup = cmnBackupSettings(G_FeaturesBackup,
                                                   ARRAYITEMCOUNT(G_FeaturesBackup));
#ifndef __ALWAYSOBJHOTKEYS__
        pFeaturesData->bObjectHotkeys = hifObjectHotkeysEnabled();
#endif
#ifndef __ALWAYSREPLACEREFRESH__
        pFeaturesData->bReplaceRefresh = krnReplaceRefreshEnabled();
#endif

        if (!ctlMakeCheckboxContainer(pnbp->hwndDlgPage,
                                      ID_XCDI_CONTAINER))
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                   "ctlMakeCheckboxContainer failed.");
        else
        {
            cRecords = sizeof(G_FeatureItemsList) / sizeof(FEATURESITEM);

            G_pFeatureRecordsList
                = (PCHECKBOXRECORDCORE)cnrhAllocRecords(hwndFeaturesCnr,
                                                        sizeof(CHECKBOXRECORDCORE),
                                                        cRecords);
            // insert feature records:
            // start for-each-record loop
            preccThis = G_pFeatureRecordsList;
            ul = 0;
            while (preccThis)
            {
                // load NLS string for feature
                nlsLoadString(G_FeatureItemsList[ul].usFeatureID, // in: string ID
                              &(G_FeatureItemsList[ul].pszNLSString), // out: NLS string
                              NULL);

                // copy FEATURESITEM to record core
                preccThis->ulStyle = G_FeatureItemsList[ul].ulStyle;
                preccThis->ulItemID = G_FeatureItemsList[ul].usFeatureID;
                preccThis->usCheckState = 0;        // unchecked
                preccThis->recc.pszTree = G_FeatureItemsList[ul].pszNLSString;

                preccParent = NULL;

                // find parent record if != 0
                if (G_FeatureItemsList[ul].usParentID)
                {
                    // parent specified:
                    // search records we have prepared so far
                    ULONG ul2 = 0;
                    PCHECKBOXRECORDCORE preccThis2 = G_pFeatureRecordsList;
                    for (ul2 = 0; ul2 < ul; ul2++)
                    {
                        if (preccThis2->ulItemID == G_FeatureItemsList[ul].usParentID)
                        {
                            preccParent = preccThis2;
                            break;
                        }
                        preccThis2 = (PCHECKBOXRECORDCORE)preccThis2->recc.preccNextRecord;
                    }
                }

                cnrhInsertRecords(hwndFeaturesCnr,
                                  (PRECORDCORE)preccParent,
                                  (PRECORDCORE)preccThis,
                                  TRUE, // invalidate
                                  NULL,
                                  CRA_RECORDREADONLY,
                                  1);

                // next record
                preccThis = (PCHECKBOXRECORDCORE)preccThis->recc.preccNextRecord;
                ul++;
            }
        } // end if (ctlMakeCheckboxContainer(inbp.hwndPage,

        // register tooltip class
        if (!ctlRegisterTooltip(hab))
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                   "ctlRegisterTooltip failed.");
        else
        {
            // create tooltip
            if (!(pnbp->hwndTooltip = WinCreateWindow(HWND_DESKTOP,  // parent
                                                      WC_CCTL_TOOLTIP, // wnd class
                                                      "",            // window text
                                                      XWP_TOOLTIP_STYLE,
                                                           // tooltip window style (common.h)
                                                      10, 10, 10, 10,    // window pos and size, ignored
                                                      pnbp->hwndDlgPage, // owner window -- important!
                                                      HWND_TOP,      // hwndInsertBehind, ignored
                                                      DID_TOOLTIP, // window ID, optional
                                                      NULL,          // control data
                                                      NULL)))         // presparams
            {
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "WinCreateWindow failed creating tooltip.");
            }
            else
            {
                // tooltip successfully created:
                // add tools (i.e. controls of the dialog)
                // according to the usToolIDs array
                TOOLINFO    ti = {0};
                ti.ulFlags = /* TTF_CENTERBELOW | */ TTF_SUBCLASS;
                ti.hwndToolOwner = pnbp->hwndDlgPage;
                ti.pszText = PSZ_TEXTCALLBACK;  // send TTN_NEEDTEXT
                // add cnr as tool to tooltip control
                ti.hwndTool = hwndFeaturesCnr;
                WinSendMsg(pnbp->hwndTooltip,
                           TTM_ADDTOOL,
                           (MPARAM)0,
                           &ti);

                // set timers
                WinSendMsg(pnbp->hwndTooltip,
                           TTM_SETDELAYTIME,
                           (MPARAM)TTDT_AUTOPOP,
                           (MPARAM)(40*1000));        // 40 secs for autopop (hide)
            }
        }
    }

    if (flFlags & CBI_SET)
    {
#ifndef __NOICONREPLACEMENTS__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_REPLACEICONS,
                cmnQuerySetting(sfIconReplacements));
#endif
#ifndef __ALWAYSREPLACEHELP__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_REPLACEHELP,
                cmnQuerySetting(sfHelpReplacements));
#endif
// #ifndef __ALWAYSRESIZESETTINGSPAGES__        setting removed V1.0.1 (2002-12-14) [umoeller]
//         ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_RESIZESETTINGSPAGES,
//                 cmnQuerySetting(sfResizeSettingsPages));
// #endif
#ifndef __ALWAYSREPLACEICONPAGE__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_REPLACEICONPAGE,
                cmnQuerySetting(sfReplaceIconPage));
#endif
#ifndef __ALWAYSREPLACEFILEPAGE__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_REPLACEFILEPAGE,
                cmnQuerySetting(sfReplaceFilePage));
#endif
#ifndef __NOXSYSTEMSOUNDS__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_XSYSTEMSOUNDS,
                cmnQuerySetting(sfXSystemSounds));
#endif
#ifndef __ALWAYSFIXCLASSTITLES__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_FIXCLASSTITLES,
                cmnQuerySetting(sfFixClassTitles));   // added V0.9.12 (2001-05-22) [umoeller]
#endif
#ifndef __NOCFGSTATUSBARS__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_ENABLESTATUSBARS,
                cmnQuerySetting(sfStatusBars));
#endif

        // V1.0.1 (2002-11-30) [umoeller]
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_ENABLETOOLBARS,
                cmnQuerySetting(sfToolBars));

#ifndef __NOSNAPTOGRID__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_ENABLESNAP2GRID,
                cmnQuerySetting(sfSnap2Grid));
#endif
#ifndef __ALWAYSFDRHOTKEYS__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_ENABLEFOLDERHOTKEYS,
                cmnQuerySetting(sfFolderHotkeys));
#endif
#ifndef __ALWAYSEXTSORT__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_EXTFOLDERSORT,
                cmnQuerySetting(sfExtendedSorting));
#endif
        // ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_MONITORCDROMS,
           //      cmnQuerySetting(sMonitorCDRoms));

#ifndef __NOTURBOFOLDERS__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_TURBOFOLDERS,
                // return the current global setting;
                // cmnQuerySetting would return the initial
                // WPS startup setting
                cmnQuerySetting(sfTurboFolders));
#endif
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_FDRSPLITVIEWS,
                // return the current global setting;
                // cmnQuerySetting would return the initial
                // WPS startup setting
                cmnQuerySetting(sfFdrSplitViews));
#ifndef __ALWAYSHOOK__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_XWPHOOK,
                cmnQuerySetting(sfXWPHook));
#endif
#ifndef __ALWAYSOBJHOTKEYS__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_GLOBALHOTKEYS,
                hifObjectHotkeysEnabled());
#endif
/*  removed V0.9.19 (2002-05-28) [umoeller]
#ifndef __NOPAGER__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_PAGER,
                cmnQuerySetting(sfEnableXPager));
#endif
*/

#ifndef __ALWAYSREPLACEARCHIVING__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_ARCHIVING,
                cmnQuerySetting(sfReplaceArchiving));
#endif
#ifndef __NOXSHUTDOWN__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_RESTARTWPS,
                cmnQuerySetting(sfRestartDesktop));
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_XSHUTDOWN,
                cmnQuerySetting(sfXShutdown));
#endif
#ifndef __NEVERCHECKDESKTOP__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_CHECKDESKTOP,
                cmnQuerySetting(sfCheckDesktop));
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_PREPOPULATEDESKTOP,
                cmnQuerySetting(sfPrePopulateDesktop));
#endif

// #ifndef __NEVEREXTASSOCS__       // removed V1.0.1 (2002-12-15) [umoeller]
//         ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_EXTASSOCS,
//                 cmnQuerySetting(sfExtAssocs));
// #endif
        // ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_CLEANUPINIS,
           //      cmnQuerySetting(sCleanupINIs));
                // removed for now V0.9.12 (2001-05-15) [umoeller]

#ifdef __REPLHANDLES__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_REPLHANDLES,
                cmnQuerySetting(sfReplaceHandles));
#endif
#ifndef __ALWAYSREPLACEFILEEXISTS__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_REPLFILEEXISTS,
                cmnQuerySetting(sfReplaceFileExists));
#endif
#ifndef __NEVERREPLACEDRIVENOTREADY__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_REPLDRIVENOTREADY,
                cmnQuerySetting(sfReplaceDriveNotReady));
#endif

#ifndef __ALWAYSREPLACEPASTE__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_REPLACEPASTE,
                cmnQuerySetting(sfReplacePaste));
#endif

#ifndef __ALWAYSTRASHANDTRUEDELETE__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_REPLACEDELETE,
                (cmnTrashCanReady() && cmnQuerySetting(sfReplaceDelete)));
#endif

#ifndef __ALWAYSREPLACEREFRESH__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_REPLACEREFRESH,
                krnReplaceRefreshEnabled());
#endif

#ifndef __NEVERNEWFILEDLG__
        ctlSetRecordChecked(hwndFeaturesCnr, ID_XCSI_NEWFILEDLG,
                cmnQuerySetting(sfNewFileDlg));
#endif
    }

    if (flFlags & CBI_ENABLE)
    {
        BOOL        fXFolder = krnIsClassReady(G_pcszXFolder),
                    fXFldDesktop = krnIsClassReady(G_pcszXFldDesktop),
                    fXFldDataFile = krnIsClassReady(G_pcszXFldDataFile),
                    fXFldDisk = krnIsClassReady(G_pcszXFldDisk),
                    fXWPProgramFile = krnIsClassReady(G_pcszXWPProgramFile),
                    fXWPFileSystem = krnIsClassReady(G_pcszXWPFileSystem);

#ifndef __ALWAYSREPLACEFILEPAGE__
        ctlEnableRecord(hwndFeaturesCnr, ID_XCSI_REPLACEFILEPAGE,
                (    (fXFolder)
                  || (fXFldDataFile)
                ));
#endif
#ifndef __NOCFGSTATUSBARS__
        ctlEnableRecord(hwndFeaturesCnr, ID_XCSI_ENABLESTATUSBARS,
                (fXFolder));
#endif
        ctlEnableRecord(hwndFeaturesCnr, ID_XCSI_ENABLETOOLBARS,
                (fXFolder));
#ifndef __NOSNAPTOGRID__
        ctlEnableRecord(hwndFeaturesCnr, ID_XCSI_ENABLESNAP2GRID,
                (fXFolder));
#endif
#ifndef __ALWAYSFDRHOTKEYS__
        ctlEnableRecord(hwndFeaturesCnr, ID_XCSI_ENABLEFOLDERHOTKEYS,
                (fXFolder));
#endif
#ifndef __ALWAYSEXTSORT__
        ctlEnableRecord(hwndFeaturesCnr, ID_XCSI_EXTFOLDERSORT,
                (fXFolder));
#endif
#ifndef __NOTURBOFOLDERS__
        ctlEnableRecord(hwndFeaturesCnr, ID_XCSI_TURBOFOLDERS,
                (    fXFolder
                  && fXWPFileSystem
                  && fXWPProgramFile            // V0.9.16 (2001-12-08) [umoeller]
                ));
#endif

#ifndef __ALWAYSHOOK__
        ctlEnableRecord(hwndFeaturesCnr, ID_XCSI_XWPHOOK,
                           (G_pXwpGlobalShared)     // fixed V1.0.1 (2003-01-25) [umoeller]
                        && (G_pXwpGlobalShared->hwndDaemonObject)
                       );
#endif
#ifndef __ALWAYSOBJHOTKEYS__
        ctlEnableRecord(hwndFeaturesCnr, ID_XCSI_GLOBALHOTKEYS,
                hifXWPHookReady());
#endif
/*
#ifndef __NOPAGER__
        ctlEnableRecord(hwndFeaturesCnr, ID_XCSI_PAGER,
                hifXWPHookReady());
#endif      removed V0.9.19 (2002-05-28) [umoeller]
*/
#ifndef __NOXSYSTEMSOUNDS__
        ctlEnableRecord(hwndFeaturesCnr, ID_XCSI_XSYSTEMSOUNDS,
                (   (fXFolder)
                 || (fXFldDesktop)
                ));
#endif

#ifndef __NOXSHUTDOWN__
        ctlEnableRecord(hwndFeaturesCnr, ID_XCSI_RESTARTWPS,
                (fXFldDesktop));
        ctlEnableRecord(hwndFeaturesCnr, ID_XCSI_XSHUTDOWN,
                (fXFldDesktop));
#endif

// #ifndef __NEVEREXTASSOCS__       removed V1.0.1 (2002-12-15) [umoeller]
//         ctlEnableRecord(hwndFeaturesCnr, ID_XCSI_EXTASSOCS,
//                 (fXFldDataFile));
// #endif
        /* ctlEnableRecord(hwndFeaturesCnr, ID_XCSI_CLEANUPINIS,
                !(cmnQuerySetting(sNoWorkerThread))); */

#ifndef __NEVERREPLACEDRIVENOTREADY__
        ctlEnableRecord(hwndFeaturesCnr, ID_XCSI_REPLDRIVENOTREADY,
                (fXFldDisk));
#endif
    }
}

/*
 *@@ setFeaturesChanged:
 *      notebook callback function (notebook.c) for the
 *      XWPSetup "Features" page.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@changed V0.9.1 (2000-02-01) [umoeller]: added global hotkeys flag
 *@@changed V0.9.1 (2000-02-09) [umoeller]: fixed mutex hangs while dialogs were displayed
 *@@changed V0.9.2 (2000-03-19) [umoeller]: "Undo" and "Default" created duplicate records; fixed
 *@@changed V0.9.6 (2000-11-11) [umoeller]: removed extassocs warning
 *@@changed V0.9.7 (2001-01-18) [umoeller]: removed pager warning
 *@@changed V0.9.7 (2001-01-22) [umoeller]: now enabling "object" page with hotkeys automatically
 *@@changed V0.9.9 (2001-01-31) [umoeller]: added "replace folder refresh"
 *@@changed V0.9.9 (2001-03-27) [umoeller]: adjusted for notebook.c change with CHECKBOXRECORDCORE notifications
 *@@changed V0.9.9 (2001-04-05) [pr]: fixed very broken Undo, Default, Setup Classes
 *@@changed V0.9.12 (2001-05-12) [umoeller]: removed "Cleanup INIs" for now
 *@@changed V0.9.14 (2001-07-31) [umoeller]: "Classes" dlg mostly rewritten
 *@@changed V1.0.1 (2002-12-14) [umoeller]: removed "resize settings pages" setting @@fixes 285, 286
 *@@changed V1.0.5 (2006-05-28) [pr]: fixed Undo again
 */

MRESULT setFeaturesItemChanged(PNOTEBOOKPAGE pnbp,
                               ULONG ulItemID, USHORT usNotifyCode,
                               ULONG ulExtra)      // for checkboxes: contains new state
{
    BOOL fSave = TRUE;

    // flags for delayed dialog showing (after unlocking)
    BOOL fShowClassesSetup = FALSE,
         fUpdateMouseMovementPage = FALSE;

    ULONG       ulNotifyMsg = 0;            // if set, a message is displayed
                                            // after unlocking
                                            // V0.9.16 (2001-10-25) [umoeller]

    signed char cAskSoundsInstallMsg = -1,  // 1 = installed, 0 = deinstalled
                cEnableTrashCan = -1;       // 1 = installed, 0 = deinstalled

    ULONG ulUpdateFlags = 0;
            // if set to != 0, this will run the INIT callback with
            // the specified CBI_* flags

    if (    (ulItemID == ID_XCDI_CONTAINER)
         && (usNotifyCode == CN_RECORDCHECKED)
       )
    {
        PCHECKBOXRECORDCORE precc = (PCHECKBOXRECORDCORE)ulExtra;

        switch (precc->ulItemID)
        {
#ifndef __NOICONREPLACEMENTS__
            case ID_XCSI_REPLACEICONS:
                cmnSetSetting(sfIconReplacements, precc->usCheckState);
            break;
#endif

#ifndef __ALWAYSREPLACEHELP__
            case ID_XCSI_REPLACEHELP:
                cmnSetSetting(sfHelpReplacements, precc->usCheckState);
            break;
#endif

// #ifndef __ALWAYSRESIZESETTINGSPAGES__        setting removed V1.0.1 (2002-12-14) [umoeller]
//             case ID_XCSI_RESIZESETTINGSPAGES:
//                 cmnSetSetting(sfResizeSettingsPages, precc->usCheckState);
//             break;
// #endif

#ifndef __ALWAYSREPLACEICONPAGE__
            case ID_XCSI_REPLACEICONPAGE:
                cmnSetSetting(sfReplaceIconPage, precc->usCheckState);
            break;
#endif

#ifndef __ALWAYSHOOK__
            case ID_XCSI_XWPHOOK:
            {
                if (hifEnableHook(precc->usCheckState) == precc->usCheckState)
                {
                    // success:
                    cmnSetSetting(sfXWPHook, precc->usCheckState);

                    if (precc->usCheckState)
                        ulNotifyMsg = 157;
                    else
                        ulNotifyMsg = 158;

                    // re-enable controls on this page
                    ulUpdateFlags = CBI_SET | CBI_ENABLE;
                }
            }
            break;
#endif

#ifndef __ALWAYSREPLACEFILEPAGE__
            case ID_XCSI_REPLACEFILEPAGE:
                cmnSetSetting(sfReplaceFilePage, precc->usCheckState);
            break;
#endif

#ifndef __NOXSYSTEMSOUNDS__
            case ID_XCSI_XSYSTEMSOUNDS:
                cmnSetSetting(sfXSystemSounds, precc->usCheckState);
                // check if sounds are to be installed or de-installed:
                if (sndAddtlSoundsInstalled(WinQueryAnchorBlock(pnbp->hwndDlgPage))
                             != precc->usCheckState)
                    // yes: set msg for "ask for sound install"
                    // at the bottom when the global semaphores
                    // are unlocked
                    cAskSoundsInstallMsg = precc->usCheckState;
            break;
#endif

#ifndef __ALWAYSFIXCLASSTITLES__
            case ID_XCSI_FIXCLASSTITLES: // added V0.9.12 (2001-05-22) [umoeller]
                cmnSetSetting(sfFixClassTitles, precc->usCheckState);
            break;
#endif

#ifndef __NOCFGSTATUSBARS__
            case ID_XCSI_ENABLESTATUSBARS:
                cmnSetSetting(sfStatusBars, precc->usCheckState);
                // update status bars for open folders
                xthrPostWorkerMsg(WOM_UPDATEALLSTATUSBARS,
                                  (MPARAM)1,
                                  MPNULL);
                // and open settings notebooks
                ntbUpdateVisiblePage(NULL,   // all somSelf's
                                     SP_XFOLDER_FLDR);
            break;
#endif

            case ID_XCSI_ENABLETOOLBARS:
                cmnSetSetting(sfToolBars, precc->usCheckState);
                // update tool bars for open folders
                xthrPostWorkerMsg(WOM_UPDATEALLSTATUSBARS,
                                  (MPARAM)1,
                                  MPNULL);
                // and open settings notebooks
                ntbUpdateVisiblePage(NULL,   // all somSelf's
                                     SP_XFOLDER_FLDR);
            break;

#ifndef __NOSNAPTOGRID__
            case ID_XCSI_ENABLESNAP2GRID:
                cmnSetSetting(sfSnap2Grid, precc->usCheckState);
                // update open settings notebooks
                ntbUpdateVisiblePage(NULL,   // all somSelf's
                                     SP_XFOLDER_FLDR);
            break;
#endif

#ifndef __ALWAYSFDRHOTKEYS__
            case ID_XCSI_ENABLEFOLDERHOTKEYS:
                cmnSetSetting(sfFolderHotkeys, precc->usCheckState);
                // update open settings notebooks
                ntbUpdateVisiblePage(NULL,   // all somSelf's
                                     SP_XFOLDER_FLDR);
            break;
#endif

#ifndef __ALWAYSEXTSORT__
            case ID_XCSI_EXTFOLDERSORT:
                cmnSetSetting(sfExtendedSorting, precc->usCheckState);
            break;
#endif

#ifndef __ALWAYSREPLACEREFRESH__
            case ID_XCSI_REPLACEREFRESH:
                krnEnableReplaceRefresh(precc->usCheckState);
                if (precc->usCheckState)
                    ulNotifyMsg = 212;
                else
                    ulNotifyMsg = 207;
            break;
#endif

#ifndef __NOTURBOFOLDERS__
            case ID_XCSI_TURBOFOLDERS:
                cmnSetSetting(sfTurboFolders, precc->usCheckState);
                if (precc->usCheckState)
                    ulNotifyMsg = 223;
                else
                    ulNotifyMsg = 224;
            break;
#endif

            case ID_XCSI_FDRSPLITVIEWS:
                cmnSetSetting(sfFdrSplitViews, precc->usCheckState);
            break;

#ifndef __ALWAYSOBJHOTKEYS__
            case ID_XCSI_GLOBALHOTKEYS:
                hifEnableObjectHotkeys(precc->usCheckState);
#ifndef __ALWAYSREPLACEICONPAGE__
                if (precc->usCheckState)
                    // enable object page also, or user can't find hotkeys
                    cmnSetSetting(sfReplaceIconPage, TRUE);
#endif
                ulUpdateFlags = CBI_SET | CBI_ENABLE;
            break;
#endif

/*
#ifndef __NOPAGER__
            case ID_XCSI_PAGER:
                if (hifEnableXPager(precc->usCheckState) == precc->usCheckState)
                {
                    cmnSetSetting(sfEnableXPager, precc->usCheckState);
                    // update "Mouse movement" page
                    fUpdateMouseMovementPage = TRUE;
                }

                ulUpdateFlags = CBI_SET | CBI_ENABLE;
            break;
#endif
*/

#ifndef __ALWAYSREPLACEARCHIVING__
            case ID_XCSI_ARCHIVING:
                cmnSetSetting(sfReplaceArchiving, precc->usCheckState);
            break;
#endif

#ifndef __NOXSHUTDOWN__
            case ID_XCSI_RESTARTWPS:
                cmnSetSetting(sfRestartDesktop, precc->usCheckState);
            break;

            case ID_XCSI_XSHUTDOWN:
                cmnSetSetting(sfXShutdown, precc->usCheckState);
                // update "Desktop" menu page
                ntbUpdateVisiblePage(NULL,   // all somSelf's
                                     SP_DTP_MENUITEMS);
                if (precc->usCheckState)
                    ulNotifyMsg = 190;
            break;
#endif

#ifndef __NEVERCHECKDESKTOP__
            case ID_XCSI_CHECKDESKTOP:
                cmnSetSetting(sfCheckDesktop, precc->usCheckState);
            break;

            case ID_XCSI_PREPOPULATEDESKTOP:
                cmnSetSetting(sfPrePopulateDesktop, precc->usCheckState);
            break;
#endif

// #ifndef __NEVEREXTASSOCS__
//             case ID_XCSI_EXTASSOCS:
//                 cmnSetSetting(sfExtAssocs, precc->usCheckState);
//                 // re-enable controls on this page
//                 ulUpdateFlags = CBI_ENABLE;
//
//                 if (precc->usCheckState)
//                     ulNotifyMsg = 208;
//             break;
// #endif

#ifndef __ALWAYSREPLACEFILEEXISTS__
            case ID_XCSI_REPLFILEEXISTS:
                cmnSetSetting(sfReplaceFileExists, precc->usCheckState);
            break;
#endif
#ifndef __NEVERREPLACEDRIVENOTREADY__
            case ID_XCSI_REPLDRIVENOTREADY:
                cmnSetSetting(sfReplaceDriveNotReady, precc->usCheckState);
            break;
#endif
            /* case ID_XCSI_CLEANUPINIS:
                cmnSetSetting(sCleanupINIs, precc->usCheckState);
            break; */       // removed for now V0.9.12 (2001-05-15) [umoeller]

#ifndef __ALWAYSREPLACEPASTE__
            case ID_XCSI_REPLACEPASTE:
                cmnSetSetting(sfReplacePaste, precc->usCheckState);
            break;
#endif

#ifndef __ALWAYSTRASHANDTRUEDELETE__
            case ID_XCSI_REPLACEDELETE:
                cEnableTrashCan = precc->usCheckState;
            break;
#endif

    #ifdef __REPLHANDLES__
            case ID_XCSI_REPLHANDLES:
                cmnSetSetting(sfReplaceHandles, precc->usCheckState);
            break;
    #endif

#ifndef __NEVERNEWFILEDLG__
            case ID_XCSI_NEWFILEDLG:
                cmnSetSetting(sfNewFileDlg, precc->usCheckState);
            break;
#endif

            default:            // includes "Classes" button
                fSave = FALSE;
        }
    } // end if (ulItemID == ID_XCDI_CONTAINER)

    switch (ulItemID)
    {
        /*
         * ID_XCDI_SETUP:
         *      "Classes" button
         */

        case ID_XCDI_SETUP:
            fShowClassesSetup = TRUE;
            fSave = FALSE;
        break;

        case DID_UNDO:
        {
            // "Undo" button: get pointer to backed-up Global Settings
            PXWPFEATURESDATA pFeaturesData = (PXWPFEATURESDATA)pnbp->pUser;

            cmnRestoreSettings(pFeaturesData->pBackup,
                               ARRAYITEMCOUNT(G_FeaturesBackup));

            // V1.0.5 (2006-05-28) [pr]: added these missing items
#ifndef __ALWAYSHOOK__
            hifEnableHook(cmnQuerySetting(sfXWPHook));
#endif
#ifndef __ALWAYSOBJHOTKEYS__
            hifEnableObjectHotkeys(pFeaturesData->bObjectHotkeys);
#endif
#ifndef __NOPAGER__
            if (hifEnableXPager(cmnQuerySetting(sfEnableXPager)) == cmnQuerySetting(sfEnableXPager))
            {
                // update "Mouse movement" page
                fUpdateMouseMovementPage = TRUE;
            }
#endif

#ifndef __ALWAYSTRASHANDTRUEDELETE__
            cEnableTrashCan = cmnQuerySetting(sfReplaceDelete);
#endif

#ifndef __ALWAYSREPLACEREFRESH__
            krnEnableReplaceRefresh(pFeaturesData->bReplaceRefresh);
#endif
            // update the display by calling the INIT callback
            ulUpdateFlags = CBI_SET | CBI_ENABLE;
        }
        break;

        case DID_DEFAULT:
        {
            // set the default settings for this settings page
            // (this is in common.c because it's also used at Desktop startup)
            cmnSetDefaultSettings(pnbp->inbp.ulPageID);

#ifndef __ALWAYSHOOK__
            hifEnableHook(cmnQuerySetting(sfXWPHook));
#endif
#ifndef __ALWAYSOBJHOTKEYS__
            hifEnableObjectHotkeys(0);
#endif
#ifndef __NOPAGER__
            if (hifEnableXPager(cmnQuerySetting(sfEnableXPager)) == cmnQuerySetting(sfEnableXPager))
            {
                // update "Mouse movement" page
                fUpdateMouseMovementPage = TRUE;
            }
#endif

#ifndef __ALWAYSTRASHANDTRUEDELETE__
            cEnableTrashCan = cmnQuerySetting(sfReplaceDelete);
#endif

#ifndef __ALWAYSREPLACEREFRESH__
            krnEnableReplaceRefresh(0);
#endif

            // update the display by calling the INIT callback
            ulUpdateFlags = CBI_SET | CBI_ENABLE;
        }
        break;
    }

    if (fShowClassesSetup)
        // "classes" dialog to be shown (classes button):
        ShowClassesDlg(pnbp->hwndFrame);
    else if (ulNotifyMsg)
        // show a notification msg:
        cmnMessageBoxExt(pnbp->hwndFrame,
                         148,       // "XWorkplace Setup"
                         NULL, 0,
                         ulNotifyMsg,
                         MB_OK);
#ifndef __NOXSYSTEMSOUNDS__
    else if (cAskSoundsInstallMsg != -1)
    {
        if (cmnMessageBoxExt(pnbp->hwndFrame,
                             148,       // "XWorkplace Setup"
                             NULL, 0,
                             (cAskSoundsInstallMsg)
                                ? 166   // "install?"
                                : 167,  // "de-install?"
                             MB_YESNO)
                == MBID_YES)
        {
            sndInstallAddtlSounds(WinQueryAnchorBlock(pnbp->hwndDlgPage),
                                  cAskSoundsInstallMsg);
        }
    }
#endif
    else if (cEnableTrashCan != -1)
    {
        cmnEnableTrashCan(pnbp->hwndFrame,
                          cEnableTrashCan);
        ulUpdateFlags = CBI_SET | CBI_ENABLE;
    }

    if (ulUpdateFlags)
        pnbp->inbp.pfncbInitPage(pnbp, ulUpdateFlags);

    if (fUpdateMouseMovementPage)
        // update "Mouse movement" page
        ntbUpdateVisiblePage(NULL,
                             SP_MOUSE_MOVEMENT);
    return 0;
}

/*
 *@@ setFeaturesMessages:
 *      notebook callback function (notebook.c) for the
 *      XWPSetup "Features" page.
 *      This gets really all the messages from the dlg.
 *
 *@@added V0.9.1 (99-11-30) [umoeller]
 */

BOOL setFeaturesMessages(PNOTEBOOKPAGE pnbp,
                         ULONG msg, MPARAM mp1, MPARAM mp2,
                         MRESULT *pmrc)
{
    BOOL    brc = FALSE;

    switch (msg)
    {
        case WM_CONTROL:
        {
            USHORT  usItemID = SHORT1FROMMP(mp1),
                    usNotifyCode = SHORT2FROMMP(mp1);

            switch (usItemID)
            {
                case DID_TOOLTIP:

                    if (usNotifyCode == TTN_NEEDTEXT)
                    {
                        PTOOLTIPTEXT pttt = (PTOOLTIPTEXT)mp2;
                        PCHECKBOXRECORDCORE precc;
                        HWND         hwndCnr = WinWindowFromID(pnbp->hwndDlgPage,
                                                               ID_XCDI_CONTAINER);
                        POINTL       ptlMouse;

                        // we use pUser2 for the Tooltip string
                        if (pnbp->pUser2)
                        {
                            free(pnbp->pUser2);
                            pnbp->pUser2 = NULL;
                        }

                        // find record under mouse
                        WinQueryMsgPos(WinQueryAnchorBlock(pnbp->hwndDlgPage),
                                       &ptlMouse);
                        if (precc = (PCHECKBOXRECORDCORE)cnrhFindRecordFromPoint(
                                                            hwndCnr,
                                                            &ptlMouse,
                                                            NULL,
                                                            CMA_ICON | CMA_TEXT,
                                                            FRFP_SCREENCOORDS))
                        {
                            if (precc->ulStyle & WS_VISIBLE)
                            {
                                CHAR        szMessageID[200];
                                XSTRING     str;

                                ULONG       ulWritten = 0;
                                APIRET      arc = 0;

                                xstrInit(&str, 0);

                                sprintf(szMessageID,
                                        "FEATURES_%04d",
                                        precc->ulItemID);

                                cmnGetMessageExt(NULL,                   // pTable
                                                 0,                      // cTable
                                                 &str,
                                                 szMessageID);

                                pnbp->pUser2 = str.psz;
                                        // freed later

                                pttt->ulFormat = TTFMT_PSZ;
                                pttt->pszText = pnbp->pUser2;
                            }
                        }

                        brc = TRUE;
                    }
                break;
            }
        }
    }

    return brc;
}

/* ******************************************************************
 *
 *   XWPSetup "Threads" page notebook callbacks (notebook.c)
 *
 ********************************************************************/

/*
 *@@ THREADRECORD:
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 */

typedef struct _THREADRECORD
{
    RECORDCORE      recc;
    PSZ             pszThreadName;
    PSZ             pszTID;
    PSZ             pszPriority;
} THREADRECORD, *PTHREADRECORD;

/*
 *@@ ClearThreads:
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 */

STATIC VOID ClearThreads(HWND hwndCnr)
{
    PTHREADRECORD prec;
    while (    (prec = (PTHREADRECORD)WinSendMsg(hwndCnr,
                                                 CM_QUERYRECORD,
                                                 (MPARAM)NULL,
                                                 MPFROM2SHORT(CMA_FIRST,
                                                              CMA_ITEMORDER)))
            && ((LONG)prec != -1)
          )
    {
        if (prec->pszThreadName)
            free(prec->pszThreadName);
        if (prec->pszTID)
            free(prec->pszTID);
        if (prec->pszPriority)
            free(prec->pszPriority);

        WinSendMsg(hwndCnr,
                   CM_REMOVERECORD,
                   (MPARAM)&prec,
                   MPFROM2SHORT(1,
                                CMA_FREE));
    }
    cnrhInvalidateAll(hwndCnr);
}

/*
 *@@ setThreadsInitPage:
 *      notebook callback function (notebook.c) for the
 *      XWPSetup "Threads" page.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 *@@changed V0.9.10 (2001-04-08) [umoeller]: fixed memory leak
 */

VOID setThreadsInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                        ULONG flFlags)        // CBI_* flags (notebook.h)
{
    HWND hwndCnr = WinWindowFromID(pnbp->hwndDlgPage,
                                   ID_XFDI_CNR_CNR);

    if (flFlags & CBI_INIT)
    {
        // PNLSSTRINGS pNLSStrings = cmnQueryNLSStrings();
        XFIELDINFO      xfi[5];
        PFIELDINFO      pfi = NULL;
        int             i = 0;

        // set up cnr details view
        xfi[i].ulFieldOffset = FIELDOFFSET(THREADRECORD, pszThreadName);
        xfi[i].pszColumnTitle = cmnGetString(ID_XSSI_THREADSTHREAD);  // "Thread"; // pszThreadsThread
        xfi[i].ulDataType = CFA_STRING;
        xfi[i++].ulOrientation = CFA_LEFT;

        xfi[i].ulFieldOffset = FIELDOFFSET(THREADRECORD, pszTID);
        xfi[i].pszColumnTitle = cmnGetString(ID_XSSI_THREADSTID);  // "TID"; // pszThreadsTID
        xfi[i].ulDataType = CFA_STRING;
        xfi[i++].ulOrientation = CFA_LEFT;

        xfi[i].ulFieldOffset = FIELDOFFSET(THREADRECORD, pszPriority);
        xfi[i].pszColumnTitle = cmnGetString(ID_XSSI_THREADSPRIORITY);  // "Priority"; // pszThreadsPriority
        xfi[i].ulDataType = CFA_STRING;
        xfi[i++].ulOrientation = CFA_LEFT;

        pfi = cnrhSetFieldInfos(hwndCnr,
                                xfi,
                                i,             // array item count
                                TRUE,          // draw lines
                                0);            // return first column

        BEGIN_CNRINFO()
        {
            cnrhSetView(CV_DETAIL | CA_DETAILSVIEWTITLES);
            cnrhSetSplitBarAfter(pfi);
            cnrhSetSplitBarPos(200);
        } END_CNRINFO(hwndCnr);

        WinSetDlgItemText(pnbp->hwndDlgPage,
                          ID_XFDI_CNR_GROUPTITLE,
                          cmnGetString(ID_XSSI_THREADSGROUPTITLE)) ; // "XWorkplace threads") // pszThreadsGroupTitle
    }

    if (flFlags & (CBI_SET | CBI_SHOW))
    {
        CHAR szTemp[1000];
        PTHREADINFO paThreadInfos;
        ULONG cThreadInfos = 0;

        ClearThreads(hwndCnr);

        if (paThreadInfos = thrListThreads(&cThreadInfos))
        {
            // we got thread infos:
            PQPROCSTAT16 pps;

            if (!prc16GetInfo(&pps))
            {
                ULONG ul;

                for (ul = 0;
                     ul < cThreadInfos;
                     ul++)
                {
                    PTHREADINFO pThis = &paThreadInfos[ul];
                    PTHREADRECORD prec = (PTHREADRECORD)cnrhAllocRecords(hwndCnr,
                                                                         sizeof(THREADRECORD),
                                                                         1);
                    if (prec)
                    {
                        ULONG ulpri = prc16QueryThreadPriority(pps,
                                                               doshMyPID(),
                                                               pThis->tid);
                        XSTRING str;
                        prec->pszThreadName = strdup(pThis->pcszThreadName);
                        sprintf(szTemp, "%d (%02lX)", pThis->tid, pThis->tid);
                        prec->pszTID = strdup(szTemp);

                        sprintf(szTemp, "0x%04lX (", ulpri);
                        xstrInitCopy(&str, szTemp, 0);
                        switch (ulpri & 0x0F00)
                        {
                            case 0x0100:
                                xstrcat(&str, "Idle", 0);
                            break;

                            case 0x0200:
                                xstrcat(&str, "Regular", 0);
                            break;

                            case 0x0300:
                                xstrcat(&str, "Time-critical", 0);
                            break;

                            case 0x0400:
                                xstrcat(&str, "Foreground server", 0);
                            break;
                        }

                        sprintf(szTemp, " +%d)", ulpri & 0xFF);
                        xstrcat(&str, szTemp, 0);

                        prec->pszPriority = str.psz;
                    }
                    else
                        break;

                    cnrhInsertRecords(hwndCnr,
                                      NULL,
                                      (PRECORDCORE)prec,
                                      FALSE,        // invalidate?
                                      NULL,
                                      CRA_RECORDREADONLY,
                                      1);
                }

                cnrhInvalidateAll(hwndCnr);

                prc16FreeInfo(pps);
            }

            free(paThreadInfos);    // V0.9.10 (2001-04-08) [umoeller]
        }
    } // end if (flFlags & CBI_SET)

    if (flFlags & CBI_ENABLE)
    {
    }

    if (flFlags & CBI_DESTROY)
    {
        ClearThreads(hwndCnr);
    }
}

/* ******************************************************************
 *
 *   XWPSetup "Status" page notebook callbacks (notebook.c)
 *
 ********************************************************************/

/*
 *@@ setStatusInitPage:
 *      notebook callback function (notebook.c) for the
 *      XWPSetup "Status" page.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@changed V0.9.2 (2000-02-20) [umoeller]: changed build string handling
 *@@changed V0.9.7 (2000-12-14) [umoeller]: removed kernel build
 *@@changed V0.9.9 (2001-03-07) [umoeller]: extracted "Threads" page
 */

VOID setStatusInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                       ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
    }

    if (flFlags & CBI_SET)
    {
        HMODULE         hmodNLS = cmnQueryNLSModuleHandle(FALSE);
        CHAR            szXFolderBasePath[CCHMAXPATH],
                        szSearchMask[2*CCHMAXPATH];
                        // szTemp[200];
        HDIR            hdirFindHandle = HDIR_SYSTEM;
        FILEFINDBUF3    FindBuffer     = {0};      // Returned from FindFirst/Next
        ULONG           ulResultBufLen = sizeof(FILEFINDBUF3);
        ULONG           ulFindCount    = 1;        // Look for 1 file at a time
        APIRET          rc             = NO_ERROR; // Return code

        HAB             hab = WinQueryAnchorBlock(pnbp->hwndDlgPage);

        ULONG           ulSoundStatus = xmmQueryStatus();

        // kernel version number V0.9.7 (2000-12-14) [umoeller]
        sprintf(szSearchMask,
                "%s (%s)",
                XFOLDER_VERSION,
                __DATE__);

        WinSetDlgItemText(pnbp->hwndDlgPage,
                          ID_XCDI_INFO_KERNEL_RELEASE,
                          szSearchMask);

        // sound status
        strlcpy(szSearchMask,       // @@todo localize
                (ulSoundStatus == MMSTAT_UNKNOWN)
                        ? "not initialized"
                : (ulSoundStatus == MMSTAT_WORKING)
                        ? "OK"
                : (ulSoundStatus == MMSTAT_MMDIRNOTFOUND)
                        ? "MMPM/2 directory not found"
                : (ulSoundStatus == MMSTAT_DLLNOTFOUND)
                        ? "MMPM/2 DLLs not found"
                : (ulSoundStatus == MMSTAT_IMPORTSFAILED)
                        ? "MMPM/2 imports failed"
                : (ulSoundStatus == MMSTAT_CRASHED)
                        ? "Media thread crashed"
                : (ulSoundStatus == MMSTAT_DISABLED)
                        ? "Disabled"
                : "unknown",
                sizeof(szSearchMask)
               );
        WinSetDlgItemText(pnbp->hwndDlgPage, ID_XCDI_INFO_SOUNDSTATUS,
                          szSearchMask);

        // language drop-down box
        WinSendDlgItemMsg(pnbp->hwndDlgPage, ID_XCDI_INFO_LANGUAGE, LM_DELETEALL, 0, 0);

        if (cmnQueryXWPBasePath(szXFolderBasePath))
        {
            sprintf(szSearchMask, "%s\\bin\\xfldr*.dll", szXFolderBasePath);

            PMPF_LANGCODES(("  szSearchMask: %s", szSearchMask));
            PMPF_LANGCODES(("  DosFindFirst"));

            rc = DosFindFirst(szSearchMask,         // file pattern
                              &hdirFindHandle,      // directory search handle
                              FILE_NORMAL,          // search attribute
                              &FindBuffer,          // result buffer
                              ulResultBufLen,       // result buffer length
                              &ulFindCount,         // number of entries to find
                              FIL_STANDARD);        // return level 1 file info

            if (rc != NO_ERROR)
                winhDebugBox(pnbp->hwndFrame,
                             "XWorkplace",
                             "XWorkplace was unable to find any National Language Support DLLs. You need to re-install XWorkplace.");
            else
            {
                // no error:
                PMPF_LANGCODES(("  Found file: %s", FindBuffer.achName));

                AddResourceDLLToLB(pnbp->hwndDlgPage,
                                   ID_XCDI_INFO_LANGUAGE,
                                   szXFolderBasePath,
                                   FindBuffer.achName);

                // keep finding files
                while (rc != ERROR_NO_MORE_FILES)
                {
                    ulFindCount = 1;                      // reset find count

                    rc = DosFindNext(hdirFindHandle,      // directory handle
                                     &FindBuffer,         // result buffer
                                     ulResultBufLen,      // result buffer length
                                     &ulFindCount);       // number of entries to find

                    if (rc == NO_ERROR)
                    {
                        AddResourceDLLToLB(pnbp->hwndDlgPage,
                                           ID_XCDI_INFO_LANGUAGE,
                                           szXFolderBasePath,
                                           FindBuffer.achName);

                        PMPF_LANGCODES(("  Found next: %s", FindBuffer.achName));
                    }
                } // endwhile

                rc = DosFindClose(hdirFindHandle);    // close our find handle
                if (rc != NO_ERROR)
                    cmnLog(__FILE__, __LINE__, __FUNCTION__,
                           "DosFindClose error");

                PMPF_LANGCODES(("  Selecting: %s", cmnQueryLanguageCode()));

                WinSendDlgItemMsg(pnbp->hwndDlgPage, ID_XCDI_INFO_LANGUAGE,
                                  LM_SELECTITEM,
                                  WinSendDlgItemMsg(pnbp->hwndDlgPage,
                                                    ID_XCDI_INFO_LANGUAGE, // find matching item
                                                    LM_SEARCHSTRING,
                                                    MPFROM2SHORT(LSS_SUBSTRING, LIT_FIRST),
                                                    (MPARAM)cmnQueryLanguageCode()),
                                  (MPARAM)TRUE); // select
            }
        } // end if (cmnQueryXWPBasePath...

        // NLS info
        if (WinLoadString(hab, hmodNLS, ID_XSSI_XFOLDERVERSION,
                    sizeof(szSearchMask), szSearchMask))
            WinSetDlgItemText(pnbp->hwndDlgPage, ID_XCDI_INFO_NLS_RELEASE,
                    szSearchMask);

        if (WinLoadString(hab, hmodNLS, ID_XSSI_NLS_AUTHOR,
                    sizeof(szSearchMask), szSearchMask))
            WinSetDlgItemText(pnbp->hwndDlgPage, ID_XCDI_INFO_NLS_AUTHOR,
                    szSearchMask);
    } // end if (flFlags & CBI_SET)
}

/*
 *@@ setStatusItemChanged:
 *      notebook callback function (notebook.c) for the
 *      XWPSetup "Status" page.
 *      Reacts to changes of any of the dialog controls.
 */

MRESULT setStatusItemChanged(PNOTEBOOKPAGE pnbp,
                             ULONG ulItemID, USHORT usNotifyCode,
                             ULONG ulExtra)      // for checkboxes: contains new state
{
    CHAR szTemp[200];

    switch (ulItemID)
    {
        // language drop-down box: load/unload resource modules
        case ID_XCDI_INFO_LANGUAGE:
        {
            if (usNotifyCode == LN_SELECT)
            {
                CHAR   szOldLanguageCode[LANGUAGECODELENGTH];
                // LONG   lTemp2 = 0;
                CHAR   szTemp2[10];
                PSZ    p;

                strcpy(szOldLanguageCode, cmnQueryLanguageCode());

                WinQueryDlgItemText(pnbp->hwndDlgPage, ID_XCDI_INFO_LANGUAGE,
                                    sizeof(szTemp),
                                    szTemp);
                p = strhistr(szTemp, " -- XFLDR") + 9; // my own case-insensitive routine
                if (p)
                {
                    strncpy(szTemp2, p, 3);
                    szTemp2[3] = '\0';
                    cmnSetLanguageCode(szTemp2);
                }

                // did language really change?
                if (strncmp(szOldLanguageCode, cmnQueryLanguageCode(), 3) != 0)
                {
                    // enforce reload of resource DLL
                    if (cmnQueryNLSModuleHandle(TRUE)    // reload flag
                             == NULLHANDLE)
                    {
                        // error occurred loading the module: restore
                        //   old language
                        cmnSetLanguageCode(szOldLanguageCode);
                        // update display
                        pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
                    }
                    else
                    {
                        HWND      hwndSystemFrame,
                                  hwndCurrent = pnbp->hwndDlgPage;
                        HWND      hwndDesktop
                                = WinQueryDesktopWindow(WinQueryAnchorBlock(HWND_DESKTOP),
                                                        NULLHANDLE);

                        // "closing system window"
                        cmnMessageBoxExt(pnbp->hwndFrame,
                                         102,
                                         NULL, 0,
                                         103,
                                         MB_OK);

                        // find frame window handle of "Workplace Shell" window
                        while ((hwndCurrent) && (hwndCurrent != hwndDesktop))
                        {
                            hwndSystemFrame = hwndCurrent;
                            hwndCurrent = WinQueryWindow(hwndCurrent, QW_PARENT);
                        }

                        if (hwndCurrent)
                            WinPostMsg(hwndSystemFrame,
                                       WM_SYSCOMMAND,
                                       (MPARAM)SC_CLOSE,
                                       MPFROM2SHORT(0, 0));
                    }
                } // end if (strcmp(szOldLanguageCode, szLanguageCode) != 0)
            } // end if (usNotifyCode == LN_SELECT)
        }
        break;
    }

    return (MPARAM)-1;
}

/*
 *@@ setStatusTimer:
 *      notebook callback function (notebook.c) for the
 *      XWPSetup "Status" page.
 *      This gets called every two seconds to update
 *      variable data on the page.
 *
 *@@changed V0.9.1 (99-12-29) [umoeller]: added "Desktop restarts" field
 *@@changed V0.9.4 (2000-07-03) [umoeller]: "Desktop restarts" was 1 too large; fixed
 *@@changed V0.9.20 (2002-07-25) [umoeller]: added thousands separator to awake objects
 */

VOID setStatusTimer(PNOTEBOOKPAGE pnbp,   // notebook info struct
                    ULONG ulTimer)
{
    PCKERNELGLOBALS  pKernelGlobals = krnQueryGlobals();
    PTIB            ptib;
    PPIB            ppib;

    CHAR            szTemp[200];

    // awake Desktop objects
    // added thousands sep V0.9.20 (2002-07-25) [umoeller]
    nlsThousandsULong(szTemp,
                      G_cAwakeObjects,
                               // xfobj.c global variable, declared in kernel.h
                               // V0.9.20 (2002-07-25) [umoeller]
                      cmnQueryThousandsSeparator());

    WinSetDlgItemText(pnbp->hwndDlgPage, ID_XCDI_INFO_AWAKEOBJECTS,
                      szTemp);

    if (DosGetInfoBlocks(&ptib, &ppib) == NO_ERROR)
    {
        PQPROCSTAT16 pps;

        if (!prc16GetInfo(&pps))
        {
            PRCPROCESS       prcp;
            // WPS thread count
            prc16QueryProcessInfo(pps, doshMyPID(), &prcp);
            WinSetDlgItemShort(pnbp->hwndDlgPage, ID_XCDI_INFO_WPSTHREADS,
                               prcp.usThreads,
                               FALSE);  // unsigned
            prc16FreeInfo(pps);
        }
    }

    // XWPHook status
    {
        PCSZ    pcsz = "Disabled";

        if (G_pXwpGlobalShared)
        {
            // Desktop restarts V0.9.1 (99-12-29) [umoeller]
            WinSetDlgItemShort(pnbp->hwndDlgPage, ID_XCDI_INFO_WPSRESTARTS,
                               G_pXwpGlobalShared->ulWPSStartupCount - 1,
                               FALSE);  // unsigned

            if (G_pXwpGlobalShared->fAllHooksInstalled)
                pcsz = "Loaded, OK";
            else
                pcsz = "Not loaded";
        }

        WinSetDlgItemText(pnbp->hwndDlgPage, ID_XCDI_INFO_HOOKSTATUS,
                          pcsz);
    }
}

/* ******************************************************************
 *
 *   XWPSetup "Objects" page notebook callbacks (notebook.c)
 *
 ********************************************************************/

/*
 *@@ setFindExistingObjects:
 *      this goes thru one of the STANDARDOBJECT arrays
 *      and checks all objects for their existence by
 *      setting each pExists member pointer to the object
 *      or NULL.
 *
 *@@added V0.9.4 (2000-07-15) [umoeller]
 */

VOID setFindExistingObjects(BOOL fStandardObj)      // in: if FALSE, XWorkplace objects;
                                                    // if TRUE, standard Desktop objects
{
    PSTANDARDOBJECT pso2;
    ULONG ul, ulMax;

    if (fStandardObj)
    {
        // Desktop objects:
        pso2 = G_WPSObjects;
        ulMax = sizeof(G_WPSObjects) / sizeof(STANDARDOBJECT);
    }
    else
    {
        // XWorkplace objects:
        pso2 = G_XWPObjects;
        ulMax = sizeof(G_XWPObjects) / sizeof(STANDARDOBJECT);
    }

    // go thru array
    for (ul = 0;
         ul < ulMax;
         ul++)
    {
        pso2->pExists = cmnQueryObjectFromID(*(pso2->ppcszDefaultID));

        // next item
        pso2++;
    }
}

#endif // __NOXWPSETUP__

/*
 *@@ setCreateStandardObject:
 *      this creates a default WPS/XWP object from the
 *      given menu item ID, after displaying a confirmation
 *      box (XWPSetup "Objects" page).
 *      Returns TRUE if the menu ID was found in the given array.
 *
 *@@changed V0.9.1 (2000-02-01) [umoeller]: renamed prototype; added hwndOwner
 *@@changed V0.9.4 (2000-07-15) [umoeller]: now storing object pointer to disable menu item in time
 *@@changed V0.9.9 (2001-04-07) [pr]: added location field for creating objects
 *@@changed V0.9.10 (2001-04-09) [pr]: modified location handling, fixed message box location
 *@@changed V0.9.19 (2002-04-02) [umoeller]: added error checking for class object
 *@@changed V0.9.19 (2002-04-02) [umoeller]: fixed empty titles
 *@@changed V0.9.19 (2002-04-24) [umoeller]: added fConfirm
 */

BOOL setCreateStandardObject(HWND hwndOwner,         // in: for dialogs
                             USHORT usMenuID,        // in: selected menu item
                             BOOL fConfirm,          // in: confirm creation?
                             BOOL fStandardObj)      // in: if FALSE, XWorkplace object;
                                                     // if TRUE, standard Desktop object
{
    BOOL    brc = FALSE;
    ULONG   ul = 0;

    PSTANDARDOBJECT pso2;
    ULONG ulMax;

    if (fStandardObj)
    {
        // Desktop objects:
        pso2 = G_WPSObjects;
        ulMax = sizeof(G_WPSObjects) / sizeof(STANDARDOBJECT);
    }
    else
    {
        // XWorkplace objects:
        pso2 = G_XWPObjects;
        ulMax = sizeof(G_XWPObjects) / sizeof(STANDARDOBJECT);
    }

    // go thru array
    for (ul = 0;
         ul < ulMax;
         ul++)
    {
        if (pso2->usMenuID == usMenuID)
        {
            CHAR    szSetupString[2000],
                    szLocationPath[CCHMAXPATH];
            PCSZ     apsz[3] = {  NULL,              // will be title
                                 szLocationPath,
                                 szSetupString
                              };
            PCSZ    pcszLocation;
            WPObject *pObjLocation;

            // get class's class object
            PCSZ        pcszClassName = *(pso2->ppcszObjectClass);
            somId       somidThis = somIdFromString((PSZ)pcszClassName);
            SOMClass    *pClassObject;

            if (!(pClassObject = _somFindClass(SOMClassMgrObject, somidThis, 0, 0)))
            {
                // class object is dead:
                // V0.9.19 (2002-04-02) [umoeller]
                apsz[0] = pcszClassName;
                cmnMessageBoxExt(hwndOwner,
                                 148, // "XWorkplace Setup",
                                 apsz,
                                 1,
                                 233,
                                 MB_CANCEL);
            }
            else
            {
                sprintf(szSetupString, "%sOBJECTID=%s",
                        pso2->pcszSetupString,       // can be empty or ";"-terminated string
                        *(pso2->ppcszDefaultID));

                if (pClassObject)
                    // get class's default title
                    apsz[0] = _wpclsQueryTitle(pClassObject);

                // some classes return empty titles, so use strhlen
                // V0.9.19 (2002-04-02) [umoeller]
                if (!strhlen(apsz[0]))
                    // title not found: use class name then
                    apsz[0] = pcszClassName;

                pcszLocation = pso2->pcszLocation;
                strcpy(szLocationPath, pcszLocation);

                if (!(pObjLocation = cmnQueryObjectFromID(pcszLocation)))
                {
                    pcszLocation = WPOBJID_DESKTOP;
                    pObjLocation = cmnQueryObjectFromID(pcszLocation);
                }

                if (pObjLocation && _somIsA(pObjLocation, _WPFileSystem))
                    _wpQueryFilename(pObjLocation,
                                     szLocationPath,
                                     TRUE);

                if (    (!fConfirm)     // V0.9.19 (2002-04-24) [umoeller]
                     || (cmnMessageBoxExt(hwndOwner,
                                          148, // "XWorkplace Setup",
                                          apsz,
                                          3,
                                          163,        // "create object?"
                                          MB_YESNO)
                               == MBID_YES)
                   )
                {
                    HOBJECT hobj;

                    if (hobj = WinCreateObject((PSZ)pcszClassName,
                                               (PSZ)apsz[0],                     // title
                                               szSetupString,               // setup
                                               (PSZ)pcszLocation,           // location
                                               CO_REPLACEIFEXISTS)) // CO_FAILIFEXISTS))
                    {
                        // alright, got it:
                        // store in array so the menu item will be
                        // disabled next time
                        pso2->pExists = _wpclsQueryObject(_WPObject,
                                                          hobj);

                        cmnMessageBoxExt(hwndOwner,
                                         148, // "XWorkplace Setup",
                                         apsz, 1,
                                         164, // "success"
                                         MB_OK);
                        brc = TRUE;
                    }
                    else
                        cmnMessageBoxExt(hwndOwner,
                                         148, // "XWorkplace Setup",
                                         apsz, 1,
                                         165, // "failed!"
                                         MB_OK);
                }
            }

            SOMFree(somidThis);
            break;
        }
        // not found: try next item
        pso2++;
    }

    return brc;
}

#ifndef __NOXWPSETUP__

/*
 *@@ DisableObjectMenuItems:
 *      helper function for setObjectsItemChanged
 *      (XWPSetup "Objects" page) to disable items
 *      in the "Objects" menu buttons if objects
 *      exist already.
 *
 *@@changed V0.9.4 (2000-07-15) [umoeller]: now storing object pointer to disable menu item in time
 */

STATIC VOID DisableObjectMenuItems(HWND hwndMenu,          // in: button menu handle
                                   PSTANDARDOBJECT pso,    // in: first menu array item
                                   ULONG ulMax)            // in: size of menu array
{
    ULONG   ul = 0;
    PSTANDARDOBJECT pso2 = pso;

    for (ul = 0;
         ul < ulMax;
         ul++)
    {
        XSTRING strMenuItemText;
        xstrInit(&strMenuItemText, 300);
        xstrset(&strMenuItemText, winhQueryMenuItemText(hwndMenu,
                                                        pso2->usMenuID));
        xstrcat(&strMenuItemText, " (", 0);
        if (pso2->ppcszObjectClass)
            xstrcat(&strMenuItemText, *(pso2->ppcszObjectClass), 0);

        if (pso2->pExists)
        {
            // object found (exists already):
            // append the path to the menu item
            WPFolder    *pFolder = _wpQueryFolder(pso2->pExists);
            CHAR        szFolderPath[CCHMAXPATH] = "";
            _wpQueryFilename(pFolder, szFolderPath, TRUE);      // fully qualified
            xstrcat(&strMenuItemText, ", ", 0);
            xstrcat(&strMenuItemText, szFolderPath, 0);

            // disable menu item
            WinEnableMenuItem(hwndMenu, pso2->usMenuID, FALSE);
        }

        xstrcatc(&strMenuItemText, ')');
        if (pso2->pExists == NULL)
            // append "...", because we'll have a message box then
            xstrcat(&strMenuItemText, "...", 0);

        // on Warp 3, disable WarpCenter also
        if (   (!G_fIsWarp4)
            && (!strcmp(*(pso2->ppcszDefaultID), WPOBJID_WARPCENTER))
           )
            WinEnableMenuItem(hwndMenu, pso2->usMenuID, FALSE);

        WinSetMenuItemText(hwndMenu,
                           pso2->usMenuID,
                           strMenuItemText.psz);
        xstrClear(&strMenuItemText);

        pso2++;
    }
}

/*
 *@@ setObjectsInitPage:
 *      notebook callback function (notebook.c) for the
 *      XWPSetup "Objects" page.
 *      Sets the controls on the page according to the
 *      Global Settings.
 */

VOID setObjectsInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                        ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        // collect all existing objects
        setFindExistingObjects(FALSE);  // XWorkplace objects
        setFindExistingObjects(TRUE);   // Desktop objects

        ctlMakeMenuButton(WinWindowFromID(pnbp->hwndDlgPage, ID_XCD_OBJECTS_SYSTEM),
                          0, 0);        // query for menu

        ctlMakeMenuButton(WinWindowFromID(pnbp->hwndDlgPage, ID_XCD_OBJECTS_XWORKPLACE),
                          0, 0);
    }
}

/*
 *@@ setObjectsItemChanged:
 *      notebook callback function (notebook.c) for the
 *      XWPSetup "Objects" page.
 *      Reacts to changes of any of the dialog controls.
 */

MRESULT setObjectsItemChanged(PNOTEBOOKPAGE pnbp,
                              ULONG ulItemID, USHORT usNotifyCode,
                              ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT mrc = 0;

    switch (ulItemID)
    {
        /*
         * ID_XCD_OBJECTS_SYSTEM:
         *      system objects menu button
         */

        case ID_XCD_OBJECTS_SYSTEM:
        {
            HPOINTER hptrOld = winhSetWaitPointer();
            HWND    hwndMenu = WinLoadMenu(WinWindowFromID(pnbp->hwndDlgPage, ulItemID),
                                           cmnQueryNLSModuleHandle(FALSE),
                                           ID_XSM_OBJECTS_SYSTEM);
            DisableObjectMenuItems(hwndMenu,
                                   &G_WPSObjects[0],
                                   sizeof(G_WPSObjects) / sizeof(STANDARDOBJECT)); // array item count
            mrc = (MRESULT)hwndMenu;
            WinSetPointer(HWND_DESKTOP, hptrOld);
        }
        break;

        /*
         * ID_XCD_OBJECTS_XWORKPLACE:
         *      XWorkplace objects menu button
         */

        case ID_XCD_OBJECTS_XWORKPLACE:
        {
            HPOINTER hptrOld = winhSetWaitPointer();
            HWND    hwndMenu = WinLoadMenu(WinWindowFromID(pnbp->hwndDlgPage, ulItemID),
                                           cmnQueryNLSModuleHandle(FALSE),
                                           ID_XSM_OBJECTS_XWORKPLACE);
            DisableObjectMenuItems(hwndMenu,
                                   &G_XWPObjects[0],
                                   sizeof(G_XWPObjects) / sizeof(STANDARDOBJECT)); // array item count
            mrc = (MRESULT)hwndMenu;
            WinSetPointer(HWND_DESKTOP, hptrOld);
        }
        break;

        /*
         * ID_XCD_OBJECTS_CONFIGFOLDER:
         *      recreate config folder
         */

        case ID_XCD_OBJECTS_CONFIGFOLDER:
            if (cmnMessageBoxExt(pnbp->hwndFrame,
                                 148,       // XWorkplace Setup
                                 NULL, 0,
                                 161,       // config folder?
                                 MB_YESNO)
                    == MBID_YES)
                xthrPostFileMsg(FIM_RECREATECONFIGFOLDER,
                                (MPARAM)RCF_DEFAULTCONFIGFOLDER,
                                MPNULL);
        break;

        /*
         * default:
         *      check for button menu item IDs
         */

        default:
            if (    (ulItemID >= OBJECTSIDFIRST)
                 && (ulItemID <= OBJECTSIDLAST)
               )
            {
                if (!setCreateStandardObject(pnbp->hwndFrame,
                                             ulItemID,
                                             TRUE,      // confirm
                                             FALSE))
                    // Desktop objects not found:
                    // try XDesktop objects
                    setCreateStandardObject(pnbp->hwndFrame,
                                            ulItemID,
                                            TRUE,   // confirm
                                            TRUE);
            }
    }

    return mrc;
}

/* ******************************************************************
 *
 *   XWPSetup "Paranoia" page notebook callbacks (notebook.c)
 *
 ********************************************************************/

static const XWPSETTING G_ParanoiaBackup[] =
    {
        sulVarMenuOfs,
        // sfNoFreakyMenus,     removed V1.0.0 (2002-08-26) [umoeller]
// #ifndef __ALWAYSSUBCLASS__
//         sfNoSubclassing,
// #endif
        sfUse8HelvFont,
        sfNoExcptBeeps,
        sfWorkerPriorityBeep,
        sulDefaultWorkerThreadPriority
    };

SLDCDATA
        WorkerPrtyCData =
             {
                     sizeof(SLDCDATA),
                     3,          // scale 1 increments
                     0,         // scale 1 spacing
                     1,          // scale 2 increments
                     0           // scale 2 spacing
             };

static const CONTROLDEF
    ParanoiaGroup = LOADDEF_GROUP(ID_XCDI_PARANOIA_GROUP, SZL_AUTOSIZE),
    ParanoiaIntro = CONTROLDEF_TEXT_WORDBREAK(
                            LOAD_STRING,
                            ID_XCDI_PARANOIA_INTRO,
                            -100),
    VarMenuOfsTxt = LOADDEF_TEXT(ID_XCDI_VARMENUOFFSET_TXT),
    VarMenuSpin = CONTROLDEF_SPINBUTTON(
                            ID_XCDI_VARMENUOFFSET,
                            50,
                            STD_SPIN_HEIGHT),
    Use8HelvCB = LOADDEF_AUTOCHECKBOX(ID_XCDI_USE8HELVFONT),
    NoExcptBeepsCB = LOADDEF_AUTOCHECKBOX(ID_XCDI_NOEXCPTBEEPS),
    WorkerPrtyGroup = LOADDEF_GROUP(ID_XCDI_WORKERPRTY_GROUP, SZL_AUTOSIZE),
    WorkerPrtyTxt1 = LOADDEF_TEXT(ID_XCDI_WORKERPRTY_TEXT1),
    WorkerPrtySlider = CONTROLDEF_SLIDER(
                            ID_XCDI_WORKERPRTY_SLIDER,
                            50,
                            12,
                            &WorkerPrtyCData),
    WorkerPrtyTxt2 = CONTROLDEF_TEXT(
                            "A",
                            ID_XCDI_WORKERPRTY_TEXT2,
                            25,
                            -1),
    WorkerPrtyBeepCB = CONTROLDEF_AUTOCHECKBOX(
                            LOAD_STRING,
                            ID_XCDI_WORKERPRTY_BEEP,
                            200,
                            -1);

static const DLGHITEM dlgParanoia[] =
    {
        START_TABLE,
            START_ROW(0),
                START_GROUP_TABLE(&ParanoiaGroup),
                    START_ROW(0),
                        CONTROL_DEF(&ParanoiaIntro),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&VarMenuOfsTxt),
                        CONTROL_DEF(&VarMenuSpin),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&Use8HelvCB),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&NoExcptBeepsCB),
                END_TABLE,
            START_ROW(0),
                START_GROUP_TABLE(&WorkerPrtyGroup),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&WorkerPrtyTxt1),
                        CONTROL_DEF(&WorkerPrtySlider),
                        CONTROL_DEF(&WorkerPrtyTxt2),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&WorkerPrtyBeepCB),
                END_TABLE,
            START_ROW(0),       // notebook buttons (will be moved)
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE
    };
/*
 *@@ setParanoiaInitPage:
 *      notebook callback function (notebook.c) for the
 *      XWPSetup "Paranoia" page.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@changed V0.9.2 (2000-03-28) [umoeller]: added freaky menus setting
 */

VOID setParanoiaInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                         ULONG flFlags)        // CBI_* flags (notebook.h)
{
    if (flFlags & CBI_INIT)
    {
        // first call: backup Global Settings for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        pnbp->pUser = cmnBackupSettings(G_ParanoiaBackup,
                                        ARRAYITEMCOUNT(G_ParanoiaBackup));

        // insert the controls using the dialog formatter
        // V0.9.19 (2002-04-17) [umoeller]
        ntbFormatPage(pnbp->hwndDlgPage,
                      dlgParanoia,
                      ARRAYITEMCOUNT(dlgParanoia));

        // set up slider
        winhSetSliderTicks(WinWindowFromID(pnbp->hwndDlgPage, ID_XCDI_WORKERPRTY_SLIDER),
                           (MPARAM)0, 6,
                           (MPARAM)-1, -1);
    }

    if (flFlags & CBI_SET)
    {
        // variable menu ID offset spin button
        winhSetDlgItemSpinData(pnbp->hwndDlgPage, ID_XCDI_VARMENUOFFSET,
                                                100, 2000,
                                                cmnQuerySetting(sulVarMenuOfs));
        // winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XCDI_NOFREAKYMENUS,
        //                                        cmnQuerySetting(sfNoFreakyMenus));
        // removed V1.0.0 (2002-08-26) [umoeller]

// #ifndef __ALWAYSSUBCLASS__
//         winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XCDI_NOSUBCLASSING,
//                                                cmnQuerySetting(sfNoSubclassing));
// #endif
        // winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XCDI_NOWORKERTHREAD,
           //                                     cmnQuerySetting(sNoWorkerThread));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XCDI_USE8HELVFONT,
                                               cmnQuerySetting(sfUse8HelvFont));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XCDI_NOEXCPTBEEPS,
                                               cmnQuerySetting(sfNoExcptBeeps));
        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XCDI_WORKERPRTY_BEEP,
                                               cmnQuerySetting(sfWorkerPriorityBeep));

        winhSetSliderArmPosition(WinWindowFromID(pnbp->hwndDlgPage, ID_XCDI_WORKERPRTY_SLIDER),
                                 SMA_INCREMENTVALUE,
                                 cmnQuerySetting(sulDefaultWorkerThreadPriority));
    }

    /*
    if (flFlags & CBI_ENABLE)
    {
        WinEnableControl(pnbp->hwndDlgPage, ID_XCDI_WORKERPRTY_TEXT1,
                        !(cmnQuerySetting(sNoWorkerThread)));
        WinEnableControl(pnbp->hwndDlgPage, ID_XCDI_WORKERPRTY_SLIDER,
                        !(cmnQuerySetting(sNoWorkerThread)));
        WinEnableControl(pnbp->hwndDlgPage, ID_XCDI_WORKERPRTY_TEXT2,
                        !(cmnQuerySetting(sNoWorkerThread)));
        WinEnableControl(pnbp->hwndDlgPage, ID_XCDI_WORKERPRTY_BEEP,
                        !(cmnQuerySetting(sNoWorkerThread)));
    }
    */
}

/*
 *@@ setParanoiaItemChanged:
 *      notebook callback function (notebook.c) for the
 *      XWPSetup "Paranoia" page.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@changed V0.9.2 (2000-03-28) [umoeller]: added freaky menus setting
 *@@changed V0.9.9 (2001-04-01) [pr]: fixed freaky menus undo
 */

MRESULT setParanoiaItemChanged(PNOTEBOOKPAGE pnbp,
                               ULONG ulItemID, USHORT usNotifyCode,
                               ULONG ulExtra)      // for checkboxes: contains new state
{
    BOOL fSave = TRUE,
         fUpdateOtherPages = FALSE;

    switch (ulItemID)
    {
        case ID_XCDI_VARMENUOFFSET:
            cmnSetSetting(sulVarMenuOfs, ulExtra);
        break;

        // case ID_XCDI_NOFREAKYMENUS:
        //     cmnSetSetting(sfNoFreakyMenus, ulExtra);
        // break;       removed V1.0.0 (2002-08-26) [umoeller]

        case ID_XCDI_USE8HELVFONT:
            cmnSetSetting(sfUse8HelvFont, ulExtra);
        break;

        case ID_XCDI_NOEXCPTBEEPS:
            cmnSetSetting(sfNoExcptBeeps, ulExtra);
        break;

        case ID_XCDI_WORKERPRTY_SLIDER:
        {
            PSZ pszNewInfo = "error";

            LONG lSliderIndex = winhQuerySliderArmPosition(
                                            WinWindowFromID(pnbp->hwndDlgPage, ID_XCDI_WORKERPRTY_SLIDER),
                                            SMA_INCREMENTVALUE);

            switch (lSliderIndex)
            {
                case 0:     pszNewInfo = "Idle ñ0"; break;
                case 1:     pszNewInfo = "Idle +31"; break;
                case 2:     pszNewInfo = "Regular ñ0"; break;
            }

            WinSetDlgItemText(pnbp->hwndDlgPage,
                              ID_XCDI_WORKERPRTY_TEXT2,
                              pszNewInfo);

            if (lSliderIndex != cmnQuerySetting(sulDefaultWorkerThreadPriority))
            {
                // update the global settings
                cmnSetSetting(sulDefaultWorkerThreadPriority, lSliderIndex);

                xthrResetWorkerThreadPriority();
            }
        }
        break;

        case ID_XCDI_WORKERPRTY_BEEP:
            cmnSetSetting(sfWorkerPriorityBeep, ulExtra);
            break;

        case DID_UNDO:
        {
            // "Undo" button: get pointer to backed-up Global Settings
            cmnRestoreSettings(pnbp->pUser,
                               ARRAYITEMCOUNT(G_ParanoiaBackup));

            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            // set flag to iterate over other notebook pages
            fUpdateOtherPages = TRUE;
        }
        break;

        case DID_DEFAULT:
        {
            // set the default settings for this settings page
            // (this is in common.c because it's also used at
            // Desktop startup)
            cmnSetDefaultSettings(pnbp->inbp.ulPageID);
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            // set flag to iterate over other notebook pages
            fUpdateOtherPages = TRUE;
        }
        break;

        default:
            fSave = FALSE;
    }

    if (fUpdateOtherPages)
    {
        PNOTEBOOKPAGE pnbp2 = NULL;
        // iterate over all currently open notebook pages
        while (pnbp2 = ntbQueryOpenPages(pnbp2))
        {
            if (    (pnbp2->flPage & NBFL_PAGE_INITED)
                 && (pnbp2->inbp.pfncbInitPage)
               )
                // enable/disable items on visible page
                pnbp2->inbp.pfncbInitPage(pnbp2, CBI_ENABLE);
        }
    }

    return (MPARAM)-1;
}

/* ******************************************************************
 *
 *   XWPSetup "Debug" page
 *
 ********************************************************************/

#ifdef __DEBUG__

static const struct
    {
        DEBUGGINGFLAGS  i;
        PCSZ            pcsz;
    } G_aDebugDescrs[] =
    {
        #define DEBUGSETTING(s) s, # s

        DEBUGSETTING(DBGSET_LANGCODES),
        DEBUGSETTING(DBGSET_NOTEBOOKS),
        DEBUGSETTING(DBGSET_RESTOREDATA),
        DEBUGSETTING(DBGSET_ICONREPLACEMENTS),
        DEBUGSETTING(DBGSET_STARTUP),
        DEBUGSETTING(DBGSET_SHUTDOWN),
        DEBUGSETTING(DBGSET_ORDEREDLIST),
        DEBUGSETTING(DBGSET_CNRCONTENT),
        DEBUGSETTING(DBGSET_STATUSBARS),
        DEBUGSETTING(DBGSET_SORT),
        DEBUGSETTING(DBGSET_KEYS),
        DEBUGSETTING(DBGSET_MENUS),
        DEBUGSETTING(DBGSET_TURBOFOLDERS),
        DEBUGSETTING(DBGSET_CNRBITMAPS),
        DEBUGSETTING(DBGSET_SPLITVIEW),
        DEBUGSETTING(DBGSET_DISK),
        DEBUGSETTING(DBGSET_TITLECLASH),
        DEBUGSETTING(DBGSET_ASSOCS),
        DEBUGSETTING(DBGSET_FOPS),
        DEBUGSETTING(DBGSET_TRASHCAN),
        DEBUGSETTING(DBGSET_PROGRAMSTART),
        DEBUGSETTING(DBGSET_SOUNDS),
        DEBUGSETTING(DBGSET_OBJLISTS),
        DEBUGSETTING(DBGSET_SOMFREAK),       // added V1.0.1 (2003-02-01) [umoeller]
        DEBUGSETTING(DBGSET_SOMMETHODS),  // V1.0.5 (2006-06-04) [pr]
        DEBUGSETTING(DBGSET_CLASSLIST)    // V1.0.5 (2006-06-04) [pr]
    };

static MPARAM G_ampDebugPage[] =
    {
        MPFROM2SHORT(ID_XFDI_CNR_GROUPTITLE, XAC_SIZEX | XAC_SIZEY),
        MPFROM2SHORT(ID_XFDI_CNR_CNR, XAC_SIZEX | XAC_SIZEY)
    };

/*
 *@@ setDebugInitPage:
 *
 *@@added V1.0.0 (2002-09-02) [umoeller]
 *@@changed V1.0.1 (2002-11-30) [umoeller]: now sorting alphabetically
 */

VOID setDebugInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                      ULONG flFlags)        // CBI_* flags (notebook.h)
{
    HWND hwndFeaturesCnr = WinWindowFromID(pnbp->hwndDlgPage,
                                           ID_XFDI_CNR_CNR);

    if (flFlags & CBI_INIT)
    {
        PCHECKBOXRECORDCORE preccThis,
                            pFeatureRecordsList;
        ULONG               ul,
                            cRecords;

        // first call: backup Global Settings for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        pnbp->pUser = malloc(sizeof(G_aDebugs));        // setup.h

        WinSetDlgItemText(pnbp->hwndDlgPage,
                          ID_XFDI_CNR_GROUPTITLE,
                          "Each box enables a group of Pmprintf calls.");

        if (!ctlMakeCheckboxContainer(pnbp->hwndDlgPage,
                                      ID_XFDI_CNR_CNR))
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                   "ctlMakeCheckboxContainer failed.");
        else
        {
            cRecords = ARRAYITEMCOUNT(G_aDebugDescrs);

            // sort alphabetically V1.0.1 (2002-11-30) [umoeller]
            BEGIN_CNRINFO()
            {
                cnrhSetSortFunc(fnCompareName);
            } END_CNRINFO(hwndFeaturesCnr);

            pFeatureRecordsList
                = (PCHECKBOXRECORDCORE)cnrhAllocRecords(hwndFeaturesCnr,
                                                        sizeof(CHECKBOXRECORDCORE),
                                                        cRecords);
            // insert feature records:
            // start for-each-record loop
            preccThis = pFeatureRecordsList;
            ul = 0;
            while (preccThis)
            {
                ULONG i = G_aDebugDescrs[ul].i;

                // copy FEATURESITEM to record core
                preccThis->ulStyle = WS_VISIBLE | BS_AUTOCHECKBOX;
                preccThis->ulItemID = 1000 + i;
                preccThis->usCheckState = G_aDebugs[i];
                preccThis->recc.pszIcon     // or sort won't work V1.0.1 (2002-12-08) [umoeller]
                = preccThis->recc.pszTree
                = strdup(G_aDebugDescrs[ul].pcsz);

                cnrhInsertRecords(hwndFeaturesCnr,
                                  NULL,
                                  (PRECORDCORE)preccThis,
                                  TRUE, // invalidate
                                  NULL,
                                  CRA_RECORDREADONLY,
                                  1);

                // next record
                preccThis = (PCHECKBOXRECORDCORE)preccThis->recc.preccNextRecord;
                ++ul;
            }

        } // end if (ctlMakeCheckboxContainer(inbp.hwndPage,
    }

}

/*
 *@@ setDebugItemChanged:
 *
 *@@added V1.0.0 (2002-09-02) [umoeller]
 */

MRESULT setDebugItemChanged(PNOTEBOOKPAGE pnbp,
                            ULONG ulItemID,
                            USHORT usNotifyCode,
                            ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT mrc = 0;

    if (    (ulItemID == ID_XFDI_CNR_CNR)
         && (usNotifyCode == CN_RECORDCHECKED)
       )
    {
        PCHECKBOXRECORDCORE precc = (PCHECKBOXRECORDCORE)ulExtra;
        ULONG   ul;

        for (ul = 0;
             ul < ARRAYITEMCOUNT(G_aDebugDescrs);
             ++ul)
        {
            ULONG i = G_aDebugDescrs[ul].i;
            if (precc->ulItemID - 1000 == i)
            {
                G_aDebugs[i] = precc->usCheckState;     // 1 or 0

                PrfWriteProfileData(HINI_USER,
                                    (PSZ)INIAPP_XWORKPLACE,
                                    (PSZ)INIKEY_DEBUGFLAGS,
                                    G_aDebugs,
                                    sizeof(G_aDebugs));
                break;
            }
        }
    }

    return mrc;
}

#endif // __DEBUG__

/* ******************************************************************
 *
 *   Pages manager
 *
 ********************************************************************/

/*
 *@@ setInsertNotebookPages:
 *
 *@@added V [umoeller]
 */

ULONG setInsertNotebookPages(XWPSetup *somSelf,
                             HWND hwndDlg)
{
    INSERTNOTEBOOKPAGE  inbp;
    HMODULE             savehmod = cmnQueryNLSModuleHandle(FALSE);

#ifndef __XWPLITE__
    // insert "Paranoia" page
    memset(&inbp, 0, sizeof(INSERTNOTEBOOKPAGE));
    inbp.somSelf = somSelf;
    inbp.hwndNotebook = hwndDlg;
    inbp.hmod = savehmod;
    inbp.usPageStyleFlags = BKA_MAJOR;
    inbp.pcszName = cmnGetString(ID_XSSI_PARANOIA);  // pszParanoia
    inbp.ulDlgID = ID_XFD_EMPTYDLG; // ID_XCD_PARANOIA; V0.9.19 (2002-04-17) [umoeller]
    // inbp.usFirstControlID = ID_XCDI_VARMENUOFFSET;
    inbp.ulDefaultHelpPanel  = ID_XSH_SETTINGS_PARANOIA;
    inbp.ulPageID = SP_SETUP_PARANOIA;
    inbp.pfncbInitPage    = setParanoiaInitPage;
    inbp.pfncbItemChanged = setParanoiaItemChanged;
    ntbInsertPage(&inbp);

    // insert "objects" page
    memset(&inbp, 0, sizeof(INSERTNOTEBOOKPAGE));
    inbp.somSelf = somSelf;
    inbp.hwndNotebook = hwndDlg;
    inbp.hmod = savehmod;
    inbp.usPageStyleFlags = BKA_MAJOR;
    inbp.pcszName = cmnGetString(ID_XSSI_OBJECTS);  // pszObjects
    inbp.ulDlgID = ID_XCD_OBJECTS;
    inbp.ulDefaultHelpPanel  = ID_XSH_SETTINGS_OBJECTS;
    inbp.ulPageID = SP_SETUP_OBJECTS;
    inbp.pfncbInitPage    = setObjectsInitPage;
    inbp.pfncbItemChanged = setObjectsItemChanged;
    ntbInsertPage(&inbp);

    // insert "Threads" page
    memset(&inbp, 0, sizeof(INSERTNOTEBOOKPAGE));
    inbp.somSelf = somSelf;
    inbp.hwndNotebook = hwndDlg;
    inbp.hmod = savehmod;
    inbp.usPageStyleFlags = BKA_MAJOR;
    inbp.pcszName = cmnGetString(ID_XSSI_THREADSPAGE);  // pszThreadsPage
    inbp.ulDlgID = ID_XFD_CONTAINERPAGE; // generic cnr page;
    inbp.ulDefaultHelpPanel  = ID_XSH_SETTINGS_XC_THREADS;
    inbp.ulPageID = SP_SETUP_THREADS;
    inbp.pampControlFlags = G_pampGenericCnrPage;
    inbp.cControlFlags = G_cGenericCnrPage;
    inbp.pfncbInitPage    = setThreadsInitPage;
    ntbInsertPage(&inbp);

    // insert "XWorkplace Info" page
    memset(&inbp, 0, sizeof(INSERTNOTEBOOKPAGE));
    inbp.somSelf = somSelf;
    inbp.hwndNotebook = hwndDlg;
    inbp.hmod = savehmod;
    inbp.usPageStyleFlags = BKA_MAJOR;
    inbp.pcszName = cmnGetString(ID_XSSI_XWPSTATUS);  // pszXWPStatus
    inbp.ulDlgID = ID_XCD_STATUS;
    inbp.ulDefaultHelpPanel  = ID_XSH_SETTINGS_XC_INFO;
    inbp.ulPageID = SP_SETUP_INFO;
    inbp.pfncbInitPage    = setStatusInitPage;
    inbp.pfncbItemChanged = setStatusItemChanged;
    // for this page, start a timer
    inbp.ulTimer = 1000;
    inbp.pfncbTimer       = setStatusTimer;
    ntbInsertPage(&inbp);
#endif

    // insert "XWorkplace Features" page
    memset(&inbp, 0, sizeof(INSERTNOTEBOOKPAGE));
    inbp.somSelf = somSelf;
    inbp.hwndNotebook = hwndDlg;
    inbp.hmod = savehmod;
    inbp.usPageStyleFlags = BKA_MAJOR;
    inbp.pcszName = cmnGetString(ID_XSSI_FEATURES);  // pszFeatures
    inbp.ulDlgID = ID_XCD_FEATURES;
    inbp.ulDefaultHelpPanel  = ID_XSH_SETTINGS_XC_FEATURES;
    inbp.ulPageID = SP_SETUP_FEATURES;
    inbp.pampControlFlags = G_ampFeaturesPage;
    inbp.cControlFlags = sizeof(G_ampFeaturesPage) / sizeof(G_ampFeaturesPage[0]);
    inbp.pfncbInitPage    = setFeaturesInitPage;
    inbp.pfncbItemChanged = setFeaturesItemChanged;
    inbp.pfncbMessage = setFeaturesMessages;
#ifndef __XWPLITE__
    ntbInsertPage(&inbp);

    // insert logo page  V0.9.6 (2000-11-04) [umoeller]
    memset(&inbp, 0, sizeof(INSERTNOTEBOOKPAGE));
    inbp.somSelf = somSelf;
    inbp.hwndNotebook = hwndDlg;
    inbp.hmod = savehmod;
    inbp.usPageStyleFlags = BKA_MAJOR;
    inbp.pcszName = "XWorkplace";
    inbp.ulDlgID = ID_XCD_FIRST;
    inbp.ulPageID = SP_SETUP_XWPLOGO;
    inbp.pfncbInitPage    = setLogoInitPage;
    inbp.pfncbMessage = setLogoMessages;
#endif

    #ifdef __DEBUG__
        ntbInsertPage(&inbp);

        memset(&inbp, 0, sizeof(INSERTNOTEBOOKPAGE));
        inbp.somSelf = somSelf;
        inbp.hwndNotebook = hwndDlg;
        inbp.hmod = savehmod;
        inbp.usPageStyleFlags = BKA_MAJOR;
        inbp.pcszName = "Debug";
        inbp.ulDlgID = ID_XFD_CONTAINERPAGE;
        inbp.ulDefaultHelpPanel  = 0;
        inbp.ulPageID = SP_SETUP_DEBUG;
        inbp.pampControlFlags = G_ampDebugPage;
        inbp.cControlFlags = ARRAYITEMCOUNT(G_ampDebugPage);
        inbp.pfncbInitPage    = setDebugInitPage;
        inbp.pfncbItemChanged = setDebugItemChanged;
    #endif // __DEBUG__

    return ntbInsertPage(&inbp);
}

#endif // __NOXWPSETUP__


