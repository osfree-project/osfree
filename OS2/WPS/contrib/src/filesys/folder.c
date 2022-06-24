
/*
 *@@sourcefile folder.c:
 *      implementation file for the XFolder class. The code
 *      in here gets called from the various XFolder methods
 *      overrides in classes\xfldr.c.
 *
 *      This file is ALL new with V0.9.0.
 *
 *      Function prefix for this file:
 *      --  fdr*
 *
 *@@added V0.9.0 [umoeller]
 *@@header "filesys\folder.h"
 */

/*
 *      Copyright (C) 1997-2006 Ulrich M”ller.
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
#define INCL_DOSSEMAPHORES
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINFRAMEMGR
#define INCL_WININPUT
#define INCL_WINRECTANGLES
#define INCL_WINSYS             // needed for presparams
#define INCL_WINMENUS
#define INCL_WINTIMER
#define INCL_WINDIALOGS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINENTRYFIELDS
#define INCL_WINLISTBOXES
#define INCL_WINSTDCNR
#define INCL_WINSHELLDATA       // Prf* functions

#define INCL_WINCLIPBOARD
#define INCL_WINATOM

#define INCL_GPILOGCOLORTABLE
#define INCL_GPIPRIMITIVES
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\comctl.h"             // common controls (window procs)
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\eah.h"                // extended attributes helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\nls.h"                // National Language Support helpers
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\regexp.h"             // extended regular expressions
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\syssound.h"           // system sound helper routines
#include "helpers\threads.h"            // thread helpers
#include "helpers\winh.h"               // PM helper routines
#include "helpers\wphandle.h"           // file-system object handles
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
#include "xfldr.ih"
// #include "xfobj.ih"
#include "xfdisk.ih"
#include "xfdataf.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\classtest.h"           // some cheap funcs for WPS class checks
#include "shared\cnrsort.h"             // container sort comparison functions
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\errors.h"              // private XWorkplace error codes
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\notebook.h"            // generic XWorkplace notebook handling
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)

#include "filesys\fileops.h"            // file operations implementation
#include "filesys\folder.h"             // XFolder implementation
// #include "filesys\fdrmenus.h"           // shared folder menu logic
#include "filesys\object.h"             // XFldObject implementation
#include "filesys\statbars.h"           // status bar translation logic
#include "filesys\xthreads.h"           // extra XWorkplace threads

// other SOM headers
#pragma hdrstop                     // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Setup strings
 *
 ********************************************************************/

/*
 *@@ fdrHasShowAllInTreeView:
 *      returns TRUE if the folder has the
 *      SHOWALLINTREEVIEW setting set. On
 *      Warp 3, always returns FALSE.
 *
 *@@added V0.9.14 (2001-07-28) [umoeller]
 */

BOOL fdrHasShowAllInTreeView(WPFolder *somSelf)
{
    XFolderData *somThis = XFolderGetData(somSelf);

    return (    G_fIsWarp4
             && (((PIBMFOLDERDATA)_pvWPFolderData)->fShowAllInTreeView)
           );
    /*  V1.0.0 (2002-08-24) [umoeller]
    return (    (_pulFolderShowAllInTreeView) // only != NULL on Warp 4
             && (*_pulFolderShowAllInTreeView)
           );
    */
}


/*
 *@@ fdrSetup:
 *      implementation for XFolder::wpSetup.
 *
 *@@added V0.9.9 (2001-04-04) [umoeller]
 *@@changed V0.9.12 (2001-05-20) [umoeller]: adjusted for new folder sorting
 *@@changed V1.0.0 (2002-08-24) [umoeller]: added OPEN=SPLITVIEW
 *@@changed V1.0.1 (2002-12-11) [umoeller]: fixed erroneous WOM_UPDATEALLSTATUSBARS post
 */

BOOL fdrSetup(WPFolder *somSelf,
              const char *pszSetupString)
{
    XFolderData *somThis = XFolderGetData(somSelf);

    BOOL        rc = TRUE,
                fChanged = FALSE;       // instance data changed

    LONG        lDefaultSort,
                lFoldersFirst,
                lAlwaysSort;

    CHAR        szValue[CCHMAXPATH + 1];
    ULONG       cbValue = sizeof(szValue);

    if (cmnQuerySetting(sfFdrSplitViews))
    {
        cbValue = sizeof(szValue);
        if (_wpScanSetupString(somSelf,
                               (PSZ)pszSetupString,
                               "OPEN",
                               szValue,
                               &cbValue))
        {
            if (!stricmp(szValue, "SPLITVIEW"))
                krnPostThread1ObjectMsg(T1M_OPENOBJECTFROMPTR,
                                        (MPARAM)somSelf,
                                        (MPARAM)(*G_pulVarMenuOfs + ID_XFMI_OFS_SPLITVIEW)
                                       );
        }

        // V1.0.6 (2006-08-22) [pr]: added DEFAULTVIEW=SPLITVIEW @@fixes 827
        cbValue = sizeof(szValue);
        if (_wpScanSetupString(somSelf,
                               (PSZ)pszSetupString,
                               "DEFAULTVIEW",
                               szValue,
                               &cbValue))
        {
            if (!stricmp(szValue, "SPLITVIEW"))
            {
                rc = _wpSetDefaultView(somSelf,
                                       *G_pulVarMenuOfs + ID_XFMI_OFS_SPLITVIEW);
                fChanged = TRUE;
            }
        }
    }

#ifndef __NOSNAPTOGRID__
    cbValue = sizeof(szValue);  // V1.0.6 (2006-08-22) [pr]: was missing
    if (_wpScanSetupString(somSelf,
                           (PSZ)pszSetupString,
                           "SNAPTOGRID",
                           szValue,
                           &cbValue))
    {
        rc = TRUE;
        fChanged = TRUE;
        if (!strnicmp(szValue, "NO", 2))
            _bSnapToGridInstance = 0;
        else if (!strnicmp(szValue, "YES", 3))
            _bSnapToGridInstance = 1;
        else if (!strnicmp(szValue, "DEFAULT", 7))
            _bSnapToGridInstance = 2;
        else if (!strnicmp(szValue, "EXEC", 4))
        {
            fdrSnapToGrid(somSelf, FALSE);
            fChanged = FALSE;
        }
        else
        {
            fChanged = FALSE;
            rc = FALSE;
        }
    }
#endif

    cbValue = sizeof(szValue);
    if (_wpScanSetupString(somSelf,
                           (PSZ)pszSetupString,
                           "FULLPATH",
                           szValue,
                           &cbValue))
    {
        fChanged = TRUE;
        rc = TRUE;
        if (!strnicmp(szValue, "NO", 2))
            _bFullPathInstance = 0;
        else if (!strnicmp(szValue, "YES", 3))
            _bFullPathInstance = 1;
        else if (!strnicmp(szValue, "DEFAULT", 7))
            _bFullPathInstance = 2;

        fdrUpdateAllFrameWindows(somSelf, FDRUPDATE_TITLE);
    }

    cbValue = sizeof(szValue);
    if (_wpScanSetupString(somSelf,
                           (PSZ)pszSetupString,
                           "ACCELERATORS",
                           szValue,
                           &cbValue))
    {
        fChanged = TRUE;
        rc = TRUE;
        if (!strnicmp(szValue, "NO", 2))
            _bFolderHotkeysInstance = 0;
        else if (!strnicmp(szValue, "YES", 3))
            _bFolderHotkeysInstance = 1;
        else if (!strnicmp(szValue, "DEFAULT", 7))
            _bFolderHotkeysInstance = 2;
    }

    cbValue = sizeof(szValue);
    if (_wpScanSetupString(somSelf,
                           (PSZ)pszSetupString,
                           "FAVORITEFOLDER",
                           szValue,
                           &cbValue))
    {
        rc = TRUE;
        if (!strnicmp(szValue, "NO", 2))
            _xwpMakeFavoriteFolder(somSelf, FALSE);
        else if (!strnicmp(szValue, "YES", 3))
            _xwpMakeFavoriteFolder(somSelf, TRUE);
        // fChanged = TRUE;
    }

    cbValue = sizeof(szValue);
    if (_wpScanSetupString(somSelf,
                           (PSZ)pszSetupString,
                           "QUICKOPEN",
                           szValue,
                           &cbValue))
    {
        rc = TRUE;
        if (!strnicmp(szValue, "NO", 2))
            _xwpSetQuickOpen(somSelf, FALSE);
        else if (!strnicmp(szValue, "YES", 3))
            _xwpSetQuickOpen(somSelf, TRUE);
        else if (!strnicmp(szValue, "IMMEDIATE", 3))  // V0.9.6 (2000-10-16) [umoeller]
            fdrQuickOpen(somSelf,
                         NULL,
                         0);     // no callback
    }

    if (somSelf != cmnQueryActiveDesktop())
    {
        cbValue = sizeof(szValue);
        if (_wpScanSetupString(somSelf,
                               (PSZ)pszSetupString,
                               "STATUSBAR",
                               szValue,
                               &cbValue))
        {
            rc = TRUE;
            if (!strnicmp(szValue, "NO", 2))
                _bStatusBarInstance = STATUSBAR_OFF;
            else if (!strnicmp(szValue, "YES", 3))
                _bStatusBarInstance = STATUSBAR_ON;
            else if (!strnicmp(szValue, "DEFAULT", 7))
                _bStatusBarInstance = STATUSBAR_DEFAULT;

            // moved this code inside the brackets,
            // this posted the msg no matter if the
            // string was present V1.0.1 (2002-12-11) [umoeller]
            PMPF_STATUSBARS(("posting WOM_UPDATEALLSTATUSBARS"));

            xthrPostWorkerMsg(WOM_UPDATEALLSTATUSBARS,
                              (MPARAM)1,  // show/hide flag
                              MPNULL);
            fChanged = TRUE;
        }
    }

#ifndef __ALWAYSEXTSORT__
    if (cmnQuerySetting(sfExtendedSorting))
#endif
    {
        cbValue = sizeof(szValue);
        if (_wpScanSetupString(somSelf,
                               (PSZ)pszSetupString,
                               "ALWAYSSORT",
                               szValue,
                               &cbValue))
        {
            rc = TRUE;
            _xwpQueryFldrSort(somSelf,
                              &lDefaultSort,
                              &lFoldersFirst,
                              &lAlwaysSort);

            if (!strnicmp(szValue, "NO", 2))
                lAlwaysSort = 0;
            else if (!strnicmp(szValue, "YES", 3))
                lAlwaysSort = 1;
            else if (!strnicmp(szValue, "DEFAULT", 7))
                lAlwaysSort = SET_DEFAULT;
            _xwpSetFldrSort(somSelf,
                            lDefaultSort,
                            lFoldersFirst,
                            lAlwaysSort);
        }
        cbValue = sizeof(szValue);
        if (_wpScanSetupString(somSelf,
                               (PSZ)pszSetupString,
                               "SORTFOLDERSFIRST",
                               szValue,
                               &cbValue))
        {
            rc = TRUE;
            _xwpQueryFldrSort(somSelf,
                              &lDefaultSort,
                              &lFoldersFirst,
                              &lAlwaysSort);

            if (!strnicmp(szValue, "NO", 2))
                lFoldersFirst = 0;
            else if (!strnicmp(szValue, "YES", 3))
                lFoldersFirst = 1;
            else if (!strnicmp(szValue, "DEFAULT", 7))
                lFoldersFirst = SET_DEFAULT;
            _xwpSetFldrSort(somSelf,
                            lDefaultSort,
                            lFoldersFirst,
                            lAlwaysSort);
        }

        cbValue = sizeof(szValue);
        if (_wpScanSetupString(somSelf,
                               (PSZ)pszSetupString,
                               "DEFAULTSORT",
                               szValue,
                               &cbValue))
        {
            LONG lValue;

            rc = TRUE;
            _xwpQueryFldrSort(somSelf,
                              &lDefaultSort,
                              &lFoldersFirst,
                              &lAlwaysSort);

            sscanf(szValue, "%d", &lValue);
            if ( (lValue >= -4) && (lValue <= 100) )
                lDefaultSort = lValue;
            else
                lDefaultSort = SET_DEFAULT;
            _xwpSetFldrSort(somSelf,
                            lDefaultSort,
                            lFoldersFirst,
                            lAlwaysSort);
        }

        cbValue = sizeof(szValue);
        if (_wpScanSetupString(somSelf,
                               (PSZ)pszSetupString,
                               "SORTNOW",
                               szValue,
                               &cbValue))
        {
            USHORT usSort;
            LONG lValue;

            rc = TRUE;

            sscanf(szValue, "%d", &lValue);
            if ( (lValue >= -4) && (lValue <= 100) )
                usSort = lValue;
            else
                usSort = SET_DEFAULT;

            fdrForEachOpenInstanceView(somSelf,
                                       fdrSortAllViews,
                                       usSort);
        }

#ifdef __DEBUG__
        cbValue = sizeof(szValue);
        if (_wpScanSetupString(somSelf,
                               (PSZ)pszSetupString,
                               "ISFSAWAKE",
                               szValue,
                               &cbValue))
        {
            rc = !!fdrQueryAwakeFSObject(szValue);
        }
#endif
    }

    if (fChanged)
        _wpSaveDeferred(somSelf);

    return rc;
}

/*
 *@@ fdrQuerySetup:
 *      implementation of XFolder::xwpQuerySetup2.
 *      See remarks there.
 *
 *      This returns the length of the XFolder
 *      setup string part only.
 *
 *@@added V0.9.1 (2000-01-17) [umoeller]
 *@@changed V0.9.3 (2000-04-09) [umoeller]: bitmaps on boot drive are returned with "?:\" now
 *@@changed V0.9.3 (2000-05-30) [umoeller]: ICONSHADOWCOLOR was reported as TREESHADOWCOLOR. Fixed.
 *@@changed V0.9.12 (2001-05-20) [umoeller]: adjusted for new folder sorting
 *@@changed V0.9.20 (2002-07-12) [umoeller]: fixed MENUBAR defaults
 *@@changed V1.0.1 (2002-12-20) [umoeller]: fixed trap if instance background settings existed, but had no bitmap
 */

BOOL fdrQuerySetup(WPObject *somSelf,
                   PVOID pstrSetup)
{
    BOOL brc = TRUE;

    TRY_LOUD(excpt1)
    {
        // flag defined in
        #define WP_GLOBAL_COLOR         0x40000000

        XFolderData *somThis = XFolderGetData(somSelf);

        // temporary buffer for building the setup string
        XSTRING     strView;
        ULONG       ulValue = 0;
        PSZ         pszValue = 0,
                    pszDefaultValue = 0;
        SOMClass    *pClassObject = 0;
        BOOL        fTreeIconsInvisible = FALSE,
                    fIconViewColumns = FALSE;

        CHAR        szTemp[1000] = "";
        BOOL        fDefaultMenuBar = FALSE;
        PIBMFOLDERDATA pData = (PIBMFOLDERDATA)_pvWPFolderData;
        PBYTE       pbColor;

        // xstrInit(pstrSetup, 400);
        xstrInit(&strView, 200);

        // WORKAREA
        if (_wpQueryFldrFlags(somSelf) & FOI_WORKAREA)
            xstrcat(pstrSetup, "WORKAREA=YES;", 0);

        // MENUBAR
        // V0.9.20 (2002-07-12) [umoeller]: this reported MENUBAR=NO
        // for the desktop, which is not necessary
        ulValue = _xwpQueryMenuBarVisibility(somSelf);
        if (!cmnIsADesktop(somSelf))
            fDefaultMenuBar = _xwpclsQueryMenuBarVisibility(_XFolder);
        if (ulValue != fDefaultMenuBar)
            // non-default value:
            if (ulValue)
                xstrcat(pstrSetup, "MENUBAR=YES;", 0);
            else
                xstrcat(pstrSetup, "MENUBAR=NO;", 0);

        /*
         * folder sort settings
         *
         */

        // SORTBYATTR... don't think we need this

#ifndef __ALWAYSEXTSORT__
        if (cmnQuerySetting(sfExtendedSorting))
#endif
        {
            if (_lAlwaysSort != SET_DEFAULT)
            {
                if (_lAlwaysSort)
                    xstrcat(pstrSetup, "ALWAYSSORT=YES;", 0);
                else
                    xstrcat(pstrSetup, "ALWAYSSORT=NO;", 0);
            }

            if (_lFoldersFirst != SET_DEFAULT)
            {
                if (_lFoldersFirst)
                    xstrcat(pstrSetup, "SORTFOLDERSFIRST=YES;", 0);
                else
                    xstrcat(pstrSetup, "SORTFOLDERSFIRST=NO;", 0);
            }

            if (_lDefSortCrit != SET_DEFAULT)
            {
                sprintf(szTemp, "DEFAULTSORT=%d;", _lDefSortCrit);
                xstrcat(pstrSetup, szTemp, 0);
            }
        } // end V0.9.12 (2001-05-20) [umoeller]

        // SORTCLASS
        pClassObject = _wpQueryFldrSortClass(somSelf);
        if (pClassObject != _WPFileSystem)
        {
            sprintf(szTemp, "SORTCLASS=%s;", _somGetName(pClassObject));
            xstrcat(pstrSetup, szTemp, 0);
        }

        /*
         * folder view settings
         *
         */

        // BACKGROUND, rewritten V1.0.0 (2002-08-28) [umoeller]
        if (pData)
        {
            // the folder has instance background data
            // if the pCurrentBackground member points
            // to the instance data
            if (pData->pCurrentBackground == &pData->Background)
            {
                CHAR    cType = 'S';

                PSZ     pszBitmap,
                        pszBitmapCopy = NULL;
                PCSZ    pcszUseFile = "(none)";
                CHAR    cImageMode;

                // fixed trap here if pszBitmapFile is NULL V1.0.1 (2002-12-20) [umoeller]
                if (    (pszBitmap = pData->Background.BkgndStore.pszBitmapFile)
                     && (pszBitmapCopy = strdup(pszBitmap))
                   )
                {
                    CHAR cBootDrive = doshQueryBootDrive();

                    nlsUpper(pszBitmapCopy);

                    if (*pszBitmapCopy == cBootDrive)
                        // file on boot drive:
                        // replace with '?' to make it portable
                        *pszBitmapCopy = '?';

                    pcszUseFile = pszBitmapCopy;
                }

                switch (pData->Background.BkgndStore.usTiledOrScaled)
                {
                    case BKGND_TILED:
                        cImageMode = 'T';
                    break;

                    case BKGND_SCALED:
                        cImageMode = 'S';
                    break;

                    default: // case BKGND_NORMAL:
                        cImageMode = 'N';
                    break;
                }

                sprintf(szTemp,
                        "BACKGROUND=%s,%c,%d,%c,%d %d %d;",
                        pcszUseFile,     // image name; set to "(none)" if there's none
                        cImageMode,
                        pData->Background.BkgndStore.usScaleFactor,
                        (pData->Background.BkgndStore.usColorOrBitmap == BKGND_BITMAP)
                            ? 'I'
                            : 'C', // I = image, C = color only
                        GET_RED(pData->Background.BkgndStore.lcolBackground),
                        GET_GREEN(pData->Background.BkgndStore.lcolBackground),
                        GET_BLUE(pData->Background.BkgndStore.lcolBackground));
                xstrcat(pstrSetup, szTemp, 0);

                if (pszBitmapCopy)
                    free(pszBitmapCopy);
            }
        }

        // DEFAULTVIEW: already handled by XFldObject

        /*
         * Icon view
         *
         */

        // ICONFONT
        pszValue = _wpQueryFldrFont(somSelf, OPEN_CONTENTS);
        if (pszDefaultValue = prfhQueryProfileData(HINI_USER,
                                                   PMINIAPP_SYSTEMFONTS, // "PM_SystemFonts",
                                                   PMINIKEY_ICONTEXTFONT, // "IconText",
                                                   NULL))
        {
            if (strcmp(pszValue, pszDefaultValue))
            {
                sprintf(szTemp, "ICONFONT=%s;", pszValue);
                xstrcat(pstrSetup, szTemp, 0);
            }
            free(pszDefaultValue);
        }

        // ICONNFILE: cannot be retrieved!
        // ICONFILE: cannot be retrieved!
        // ICONNRESOURCE: cannot be retrieved!

        // ICONVIEWPOS

        // ICONGRIDSIZE

        // ICONTEXTVISIBLE

        // ICONVIEW
        ulValue = _wpQueryFldrAttr(somSelf, OPEN_CONTENTS);
        switch (ulValue & (CV_NAME | CV_FLOW | CV_ICON | CV_TEXT))
        {
            case (CV_NAME | CV_FLOW): // but not CV_ICON or CV_TEXT
                xstrcat(&strView, "FLOWED", 0);
                fIconViewColumns = TRUE;        // needed for colors below
            break;

            case (CV_NAME): // but not CV_ICON | CV_FLOW or CV_TEXT
                xstrcat(&strView, "NONFLOWED", 0);
                fIconViewColumns = TRUE;        // needed for colors below
            break;

            case (CV_TEXT): // but not CV_ICON
                xstrcat(&strView, "INVISIBLE", 0);
            break;
        }

        if (ulValue & CV_MINI)
        {
            // ICONVIEW=MINI
            if (strView.ulLength)
                xstrcatc(&strView, ',');

            xstrcat(&strView, "MINI", 0);
        }

        if (strView.ulLength)
        {
            sprintf(szTemp, "ICONVIEW=%s;", strView.psz);
            xstrcat(pstrSetup, szTemp, 0);
        }

        xstrClear(&strView);

        // ICONTEXTBACKGROUNDCOLOR

        // ICONTEXTCOLOR
        if (pData)
        {
            if (!fIconViewColumns)
                pbColor = (PBYTE)&pData->LongArray.rgbIconViewTextColAsPlaced;
            else
                pbColor = (PBYTE)&pData->LongArray.rgbIconViewTextColColumns;

            if (!pbColor[3])       // use default?
            {
                sprintf(szTemp,
                        "ICONTEXTCOLOR=%d %d %d;",
                        pbColor[2],
                        pbColor[1],
                        pbColor[0]);
                xstrcat(pstrSetup, szTemp, 0);
            }
        }

        // ICONSHADOWCOLOR
        if (pData)
            if (G_fIsWarp4)
            {
                pbColor = (PBYTE)&pData->LongArray.rgbIconViewShadowCol;

                if (!pbColor[3])       // use default?
                {
                    sprintf(szTemp,
                            "ICONSHADOWCOLOR=%d %d %d;",
                            pbColor[2],
                            pbColor[1],
                            pbColor[0]);
                    xstrcat(pstrSetup, szTemp, 0);
                }
            }

        /*
         * Tree view
         *
         */

        // TREEFONT
        pszValue = _wpQueryFldrFont(somSelf, OPEN_TREE);
        if (pszDefaultValue = prfhQueryProfileData(HINI_USER,
                                                   PMINIAPP_SYSTEMFONTS, // "PM_SystemFonts",
                                                   PMINIKEY_ICONTEXTFONT, // "IconText",
                                                   NULL))
        {
            if (strcmp(pszValue, pszDefaultValue) != 0)
            {
                sprintf(szTemp, "TREEFONT=%s;", pszValue);
                xstrcat(pstrSetup, szTemp, 0);
            }

            free(pszDefaultValue);
        }

        // TREETEXTVISIBLE
        // if this is NO, the WPS displays no tree text in tree icon view;
        // setting this does not affect the CV_* flags, so apparently this
        // is done via record owner-draw. There must be some flag in the
        // instance data for this.

        // TREEVIEW
        ulValue = _wpQueryFldrAttr(somSelf, OPEN_TREE);
        switch (ulValue & (CV_TREE | CV_NAME | CV_ICON | CV_TEXT))
        {
            // CV_TREE | CV_TEXT means text only, no icons (INVISIBLE)
            // CV_TREE | CV_ICON means icons and text and +/- buttons (default)
            // CV_TREE | CV_NAME is apparently not used by the WPS

            case (CV_TREE | CV_TEXT):
                xstrcat(&strView, "INVISIBLE", 0);
                fTreeIconsInvisible = TRUE;         // needed for tree text colors below
            break;
        }

        if (G_fIsWarp4)
        {
            // on Warp 4, mini icons in Tree view are the default
            if ((ulValue & CV_MINI) == 0)
            {
                // TREEVIEW=MINI
                if (strView.ulLength)
                    xstrcatc(&strView, ',');

                xstrcat(&strView, "NORMAL", 0);
            }
        }
        else
            // Warp 3:
            if ((ulValue & CV_MINI) != 0)
            {
                // TREEVIEW=MINI
                if (strView.ulLength)
                    xstrcatc(&strView, ',');

                xstrcat(&strView, "MINI", 0);
            }

        if ((ulValue & CA_TREELINE) == 0)
        {
            // TREEVIEW=NOLINES
            if (strView.ulLength)
                xstrcatc(&strView, ',');

            xstrcat(&strView, "NOLINES", 0);
        }

        if (strView.ulLength)
        {
            sprintf(szTemp, "TREEVIEW=%s;", strView);
            xstrcat(pstrSetup, szTemp, 0);
        }

        xstrClear(&strView);

        if (pData)
        {
            if (fTreeIconsInvisible)
                pbColor = (PBYTE)&pData->LongArray.rgbTreeViewTextColTextOnly;
            else
                pbColor = (PBYTE)&pData->LongArray.rgbTreeViewTextColIcons;

            if (!pbColor[3])       // use default?
            {
                sprintf(szTemp,
                        "TREETEXTCOLOR=%d %d %d;",
                        pbColor[2],
                        pbColor[1],
                        pbColor[0]);
                xstrcat(pstrSetup, szTemp, 0);
            }

            // TREESHADOWCOLOR
            // only Warp 4 has these fields, so check size of array
            if (G_fIsWarp4)
            {
                pbColor = (PBYTE)&pData->LongArray.rgbTreeViewShadowCol;

                if (!pbColor[3])       // use default?
                {
                    sprintf(szTemp,
                            "TREESHADOWCOLOR=%d %d %d;",
                            pbColor[2],
                            pbColor[1],
                            pbColor[0]);
                    xstrcat(pstrSetup, szTemp, 0);
                }
            }
        }

        // SHOWALLINTREEVIEW
        if (fdrHasShowAllInTreeView(somSelf))
            xstrcat(pstrSetup, "SHOWALLINTREEVIEW=YES;", 0);

        /*
         * Details view
         *
         */

        // DETAILSCLASS
        pClassObject = _wpQueryFldrDetailsClass(somSelf);
        if (pClassObject != _WPFileSystem)
        {
            sprintf(szTemp, "DETAILSCLASS=%s;", _somGetName(pClassObject));
            xstrcat(pstrSetup, szTemp, 0);
        }

        // DETAILSFONT
        pszValue = _wpQueryFldrFont(somSelf, OPEN_DETAILS);
        if (pszDefaultValue = prfhQueryProfileData(HINI_USER,
                                                   PMINIAPP_SYSTEMFONTS, // "PM_SystemFonts",
                                                   PMINIKEY_ICONTEXTFONT, // "IconText",
                                                   NULL))
        {
            if (strcmp(pszValue, pszDefaultValue) != 0)
            {
                sprintf(szTemp, "DETAILSFONT=%s;", pszValue);
                xstrcat(pstrSetup, szTemp, 0);
            }

            free(pszDefaultValue);
        }

        // DETAILSTODISPLAY

        // DETAILSVIEW

        // DETAILSTEXTCOLOR
        if (pData)
        {
            pbColor = (PBYTE)&pData->LongArray.rgbDetlViewTextCol;

            if (!pbColor[3])       // use default?
            {
                sprintf(szTemp,
                        "DETAILSTEXTCOLOR=%d %d %d;",
                        pbColor[2],
                        pbColor[1],
                        pbColor[0]);
                xstrcat(pstrSetup, szTemp, 0);
            }

            // DETAILSSHADOWCOLOR
            // only Warp 4 has these fields, so check size of array
            if (G_fIsWarp4)
            {
                pbColor = (PBYTE)&pData->LongArray.rgbDetlViewShadowCol;

                if (!pbColor[3])       // use default?
                {
                    sprintf(szTemp,
                            "DETAILSSHADOWCOLOR=%d %d %d;",
                            pbColor[2],
                            pbColor[1],
                            pbColor[0]);
                    xstrcat(pstrSetup, szTemp, 0);
                }
            }
        }

        /*
         * additional XFolder setup strings
         *
         */

        switch (_bFolderHotkeysInstance)
        {
            case 0:
                xstrcat(pstrSetup, "ACCELERATORS=NO;", 0);
            break;

            case 1:
                xstrcat(pstrSetup, "ACCELERATORS=YES;", 0);
            break;

            // 2 means default
        }

        switch (_bSnapToGridInstance)
        {
            case 0:
                xstrcat(pstrSetup, "SNAPTOGRID=NO;", 0);
            break;

            case 1:
                xstrcat(pstrSetup, "SNAPTOGRID=YES;", 0);
            break;

            // 2 means default
        }

        switch (_bFullPathInstance)
        {
            case 0:
                xstrcat(pstrSetup, "FULLPATH=NO;", 0);
            break;

            case 1:
                xstrcat(pstrSetup, "FULLPATH=YES;", 0);
            break;

            // 2 means default
        }

        switch (_bStatusBarInstance)
        {
            case 0:
                xstrcat(pstrSetup, "STATUSBAR=NO;", 0);
            break;

            case 1:
                xstrcat(pstrSetup, "STATUSBAR=YES;", 0);
            break;

            // 2 means default
        }

        if (_xwpIsFavoriteFolder(somSelf))
            xstrcat(pstrSetup, "FAVORITEFOLDER=YES;", 0);

        /*
         * append string
         *
         */

        /* if (strTemp.ulLength)
        {
            // return string if buffer is given
            if ((pszSetupString) && (cbSetupString))
                strhncpy0(pszSetupString,   // target
                          strTemp.psz,      // source
                          cbSetupString);   // buffer size

            // always return length of string
            ulReturn = strTemp.ulLength;
        }

        xstrClear(&strTemp); */
    }
    CATCH(excpt1)
    {
        brc = FALSE;
    } END_CATCH();

    return brc;
}

/* ******************************************************************
 *
 *   Folder view helpers
 *
 ********************************************************************/

/*
 *@@ fdrForEachOpenViewInstance:
 *      this instance method goes through all open views of a folder and calls
 *      pfnwpCallback for each them (as opposed to fdrForEachOpenGlobalView,
 *      which goes through all open folders on the system).
 *
 *      The following params are then passed to pfnwpCallback:
 *      --   HWND       hwnd: the hwnd of the view frame window;
 *      --   ULONG      mp1:  the view type (as def'd in wpOpen)
 *      --   XFolder*   mp2:  somSelf.
 *
 *      This method does not return until all views have been processed.
 *      You might want to call this method in a different thread if the task
 *      will take long.
 *
 *      This method returns TRUE if the callback returned TRUE at least once.
 *      Note on disk objects/root folders: the WPS does not maintain an open
 *      view list for root folders, but only for the corresponding disk object.
 *      xwpForEachOpenView will call open disk views also, but the callback
 *      will still be passed the root folder in pFolder!#
 *
 *@@changed V0.9.1 (2000-02-04) [umoeller]: this used to be XFolder::xwpForEachOpenView
 *@@changed V0.9.19 (2002-06-13) [umoeller]: this broke for root folders, fixed
 *@@changed V1.0.0 (2002-08-28) [umoeller]: adjusted for new callback prototype; optimized
 */

BOOL fdrForEachOpenInstanceView(WPFolder *somSelf,
                                PFNFOREACHVIEWCALLBACK pfnCallback,
                                ULONG ulMsg)
{
    BOOL brc = FALSE;
    WPObject *somSelf2;
    // XFolderData *somThis = XFolderGetData(somSelf);
    // XFolderMethodDebug("XFolder","xf_xwpForEachOpenView");

    if (ctsIsRootFolder(somSelf))
        // for disk/root folder views: root folders have no
        // open view, instead the disk object is registered
        // to have the open view. Duh. So we need to find
        // the disk object first
        // V0.9.19 (2002-06-13) [umoeller]: this code was disabled...
        // it used an XFldDisk class method that I removed ages
        // ago, but apparently wpQueryDisk works as well.
        // This code is needed by xf_xwpSetFldrSort, among other
        // things, to update root folder views after sort criteria
        // change!
        somSelf2 = _wpQueryDisk(somSelf);
    else
        somSelf2 = somSelf;

    if (somSelf2)
    {
        if (_wpFindUseItem(somSelf2, USAGE_OPENVIEW, NULL))
        {
            // folder has an open view;
            // now we go search the open views of the folder and get the
            // frame handle of the desired view (ulView)
            /* PVIEWITEM   pViewItem;   replaced V1.0.0 (2002-08-28) [umoeller]
            for (pViewItem = _wpFindViewItem(somSelf2, VIEW_ANY, NULL);
                 pViewItem;
                 pViewItem = _wpFindViewItem(somSelf2, VIEW_ANY, pViewItem)) */

            PUSEITEM pui;
            for (pui = _wpFindUseItem(somSelf2, USAGE_OPENVIEW, NULL);
                 pui;
                 pui = _wpFindUseItem(somSelf2, USAGE_OPENVIEW, pui))
            {
                PVIEWITEM pvi = (PVIEWITEM)(pui + 1);
                // even if we have found a disk object
                // above, we need to pass it the root folder
                // pointer, because otherwise the callback
                // might get into trouble
                if (pfnCallback(somSelf,
                                pvi->handle,
                                pvi->view,
                                ulMsg))
                    brc = TRUE;
            } // end for
        } // end if
    }
    return brc;
}

/*
 *@@ fdrForEachOpenGlobalView:
 *      this class method goes through all open folder windows and calls
 *      pfnwpCallback for each open view of each open folder.
 *      As opposed to fdrForEachOpenInstanceView, this goes thru really
 *      all open folders views on the system.
 *
 *      The following params will be passed to pfnwpCallback:
 *      -- HWND       hwnd: the hwnd of the view frame window;
 *      -- ULONG      msg:  ulMsg, as passed to this method
 *      -- ULONG      mp1:  the view type (as def'd in wpOpen)
 *      -- XFolder*   mp2:  the currently open folder.
 *
 *      This method does not return until all views have been processed.
 *      You might want to call this method in a different thread if the task
 *      will take long.
 *
 *@@changed V0.9.1 (2000-02-04) [umoeller]: this used to be M_XFolder::xwpclsForEachOpenView
 *@@changed V1.0.0 (2002-08-28) [umoeller]: adjusted for new callback prototype
 *@@changed V1.0.1 (2002-11-30) [umoeller]: now using new method to include split views
 */

BOOL fdrForEachOpenGlobalView(PFNFOREACHVIEWCALLBACK pfnCallback,
                              ULONG ulMsg)
{
    M_WPFolder  *pWPFolderClass = _XFolder;
    XFolder     *pFolder = NULL;

    /* for (pFolder = _wpclsQueryOpenFolders(pWPFolderClass, NULL, QC_FIRST, FALSE);
         pFolder;
         pFolder = _wpclsQueryOpenFolders(pWPFolderClass, pFolder, QC_NEXT, FALSE))
    */

    // replaced V1.0.1 (2002-11-30) [umoeller]

    PMPF_STATUSBARS(("calling _xwpclsQueryOpenFolders"));

    while (pFolder = _xwpclsQueryOpenFolders(pWPFolderClass, pFolder, FALSE))
    {
        if (_somIsA(pFolder, pWPFolderClass))
            fdrForEachOpenInstanceView(pFolder, pfnCallback, ulMsg);
    }

    return TRUE;
}

/* ******************************************************************
 *
 *   Full path in title
 *
 ********************************************************************/

/*
 *@@ fdrSetOneFrameWndTitle:
 *      this changes the window title of a given folder frame window
 *      to the full path of the folder, according to the global and/or
 *      folder settings.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V0.9.4 (2000-08-02) [umoeller]: added "keep title" instance setting
 */

BOOL fdrSetOneFrameWndTitle(WPFolder *somSelf,
                            HWND hwndFrame)
{
    PSZ                 pFirstSlash = 0,
                        pSrchSlash = 0,
                        pNextSlash = 0;
    CHAR                szTemp[CCHMAXPATH] = "";
    XFolderData         *somThis = XFolderGetData(somSelf);
    BOOL                brc = FALSE;

    if (    (_bFullPathInstance == 1)
         || ((_bFullPathInstance == 2) && (cmnQuerySetting(sfFullPath)))
       )
    {
        // settings allow full path in title for this folder:

        // get real name (= full path), if allowed)
        _wpQueryFilename(somSelf, szTemp, TRUE);

        // now truncate path if it's longer than allowed by user
        if (    (pFirstSlash = strchr(szTemp, '\\'))
             && (cmnQuerySetting(sulMaxPathChars) > 10)
           )
        {
            pSrchSlash = pFirstSlash + 3;
            while (strlen(szTemp) > cmnQuerySetting(sulMaxPathChars))
            {
                if (pNextSlash = strchr(pSrchSlash, '\\'))
                {
                    strcpy(pFirstSlash + 4, pNextSlash);
                    pFirstSlash[1]
                    = pFirstSlash[2]
                    = pFirstSlash[3]
                    = '.';
                    pSrchSlash = pFirstSlash + 5;
                }
                else
                    break;
            }
        }

        // now either append the full path in brackets to or replace window title
        if (    (_bKeepTitleInstance == 1)
             || ((_bKeepTitleInstance == 2) && (cmnQuerySetting(sfKeepTitle)))
           ) // V0.9.4 (2000-08-02) [umoeller]
        {
            CHAR szFullPathTitle[CCHMAXPATH*2] = "";
            sprintf(szFullPathTitle, "%s (%s)",
                    _wpQueryTitle(somSelf),
                    szTemp);
            WinSetWindowText(hwndFrame, szFullPathTitle);
        }
        else
            WinSetWindowText(hwndFrame, szTemp);

        brc = TRUE;
    }
    else
    {
        // settings DON'T allow full path in title for this folder:
        // set to title only
        WinSetWindowText(hwndFrame, _wpQueryTitle(somSelf));
        brc = FALSE;
    }

    return brc;
}

/*
 *@@ fdrUpdateAllFrameWindows:
 *      this function sets the frame wnd titles for all currently
 *      open views of a given folder to the folder's full path.
 *
 *      This gets called on the Worker thread after XFolder's
 *      replacements of wpMoveObject, wpSetTitle, or wpRefresh have
 *      been called.
 *
 *      If ulAction is 0, this calls fdrSetOneFrameWndTitle in turn.
 *
 *      If ulAction is 1, we simply invalidate the folder windows.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V0.9.2 (2000-03-04) [umoeller]: this didn't work for multiple identical views
 *@@changed V0.9.2 (2000-03-06) [umoeller]: added object mutex protection
 *@@changed V0.9.20 (2002-08-08) [umoeller]: renamed, added ulAction
 */

BOOL fdrUpdateAllFrameWindows(WPFolder *somSelf,
                              ULONG ulAction)
{
    BOOL        brc = FALSE;

    WPObject *pobjLock = NULL;
    TRY_LOUD(excpt1)
    {
        if (pobjLock = cmnLockObject(somSelf))
        {
            PUSEITEM    pUseItem = NULL;
            for (pUseItem = _wpFindUseItem(somSelf, USAGE_OPENVIEW, NULL);
                 pUseItem;
                 pUseItem = _wpFindUseItem(somSelf, USAGE_OPENVIEW, pUseItem))
            {
                PVIEWITEM pViewItem = (PVIEWITEM)(pUseItem + 1);
                if (    (pViewItem->view == OPEN_CONTENTS)
                     || (pViewItem->view == OPEN_DETAILS)
                     || (pViewItem->view == OPEN_TREE)
                   )
                {
                    switch (ulAction)
                    {
                        case 0:
                            fdrSetOneFrameWndTitle(somSelf,
                                                   pViewItem->handle);
                        break;

                        case 1:
                            WinInvalidateRect(pViewItem->handle,
                                              NULL,
                                              TRUE);
                        break;
                    }
                }
            }
        } // end if fFolderLocked
    }
    CATCH(excpt1) {} END_CATCH();

    if (pobjLock)
        _wpReleaseObjectMutexSem(pobjLock);

    ntbUpdateVisiblePage(somSelf, SP_XFOLDER_FLDR);

    return brc;
}

/* ******************************************************************
 *
 *   Folder refresh
 *
 ********************************************************************/

/*
 *@@ fntFdrRefresh:
 *      transient thread which refreshes a folder.
 *      Started from fdrForceRefresh only.
 *
 *      This thread is created with a PM message queue.
 *
 *@@added V1.0.0 (2002-08-26) [umoeller]
 */

STATIC VOID _Optlink fntFdrRefresh(PTHREADINFO ptiMyself)
{
    TRY_LOUD(excpt1)
    {
        WPFolder *pFolder = (WPFolder*)ptiMyself->ulData;

        _wpRefresh(pFolder, NULLHANDLE, NULL);
    }
    CATCH(excpt1) {} END_CATCH();
}

/*
 *@@ fdrForceRefresh:
 *      forces a full folder refresh.
 *
 *      If the folder has an open view, the refresh is
 *      started on a second thread. Otherwise we only
 *      set the folder flags to have the refresh happen
 *      when the folder is opened for the next time.
 *
 *@@added V1.0.0 (2002-08-26) [umoeller]
 */

BOOL fdrForceRefresh(WPFolder *pFolder)
{
    ULONG   fl = _wpQueryFldrFlags(pFolder);

    // refresh the folder if it's not currently refreshing
    if (0 == (fl & (FOI_POPULATEINPROGRESS | FOI_REFRESHINPROGRESS)))
    {
        // we need a full refresh on the folder...
        // set FOI_ASYNCREFRESHONOPEN, clear
        // FOI_POPULATEDWITHFOLDERS | FOI_POPULATEDWITHALL,
        // which will cause the folder contents to be refreshed
        // on open
        _wpModifyFldrFlags(pFolder,
                           FOI_ASYNCREFRESHONOPEN
                                | FOI_POPULATEDWITHFOLDERS
                                | FOI_POPULATEDWITHALL
                                | FOI_TREEPOPULATED,
                           FOI_ASYNCREFRESHONOPEN);

        // if the folder is currently open, do a
        // full refresh NOW
        if (_wpFindViewItem(pFolder,
                            VIEW_ANY,
                            NULL))
        {
            return !!thrCreate(NULL,
                               fntFdrRefresh,
                               NULL,
                               "FdrRefresh",
                               THRF_PMMSGQUEUE | THRF_TRANSIENT,
                               // thread param: folder pointer
                               (ULONG)pFolder);
        }
    }

    return FALSE;
}

/* ******************************************************************
 *
 *   Quick Open
 *
 ********************************************************************/

/*
 *@@ fdrQuickOpen:
 *      implementation for the "Quick open" feature.
 *      This populates the specified folder and loads
 *      the icons for all its objects.
 *
 *      This gets called on the Worker thread upon
 *      WOM_QUICKOPEN, but can be called separately
 *      as well. For example, XFolder::wpSetup calls
 *      this on "QUICKOPEN=IMMEDIATE" now.
 *
 *@@added V0.9.6 (2000-10-16) [umoeller]
 */

BOOL fdrQuickOpen(WPFolder *pFolder,
                  PFNCBQUICKOPEN pfnCallback,
                  ULONG ulCallbackParam)
{
    BOOL        brc = TRUE;
    WPObject    *pObject = NULL;
    ULONG       ulNow = 0,
                ulMax = 0;
    BOOL        fFolderLocked = FALSE;

    // populate folder
    fdrCheckIfPopulated(pFolder,
                        FALSE);        // full populate

    TRY_LOUD(excpt1)
    {
        // lock folder contents
        if (fFolderLocked = !_wpRequestFolderMutexSem(pFolder, 5000))
        {
            // count objects
            // V0.9.20 (2002-07-31) [umoeller]: now using get_pobjNext SOM attribute
            for (   pObject = _wpQueryContent(pFolder, NULL, (ULONG)QC_FIRST);
                    (pObject);
                    pObject = *__get_pobjNext(pObject)
                )
            {
                ulMax++;
            }

            // collect icons for all objects
            // V0.9.20 (2002-07-31) [umoeller]: now using get_pobjNext SOM attribute
            for (   pObject = _wpQueryContent(pFolder, NULL, (ULONG)QC_FIRST);
                    (pObject);
                    pObject = *__get_pobjNext(pObject)
                )
            {
                // get the icon
                _wpQueryIcon(pObject);

                if (pfnCallback)
                {
                    // callback
                    brc = pfnCallback(pFolder,
                                      pObject,
                                      ulNow,
                                      ulMax,
                                      ulCallbackParam);
                    if (!brc)
                        break;
                }
                ulNow++;
            }
        }
    }
    CATCH(excpt1)
    {
        brc = FALSE;
    } END_CATCH();

    if (fFolderLocked)
        _wpReleaseFolderMutexSem(pFolder);

    return brc;
}

#ifndef __NOSNAPTOGRID__

/* ******************************************************************
 *
 *   Snap To Grid
 *
 ********************************************************************/

/*
 *@@ fdrSnapToGrid:
 *      makes all objects in the folder "snap" on a grid whose
 *      coordinates are to be defined in the global settings.
 *
 *      This function checks if an Icon view of the folder is
 *      currently open; if not and fNotify == TRUE, it displays
 *      a message box.
 *
 *@@changed V0.9.0 [umoeller]: this used to be an instance method (xfldr.c)
 */

BOOL fdrSnapToGrid(WPFolder *somSelf,
                   BOOL fNotify)
{
    HWND                hwndFrame = 0,
                        hwndCnr = 0;
    PMINIRECORDCORE     pmrc = 0;
    LONG                lNewX = 0,
                        lNewY = 0;
    BOOL                brc = FALSE;
    // if Shift is pressed, move all the objects, otherwise
    // only the selected ones
    BOOL                fShiftPressed = doshQueryShiftState();

    // first we need the frame handle of a currently open icon view;
    // all others don't make sense
    if (hwndFrame = wpshQueryFrameFromView(somSelf, OPEN_CONTENTS))
    {

        // now get the container handle
        if (hwndCnr = WinWindowFromID(hwndFrame, FID_CLIENT))
        {
            // now begin iteration over the folder's objects; we don't
            // use the WPS method (wpQueryContent) because this is too
            // slow. Instead, we query the container directly.

            pmrc = NULL;
            do
            {
                if (fShiftPressed)
                    // shift pressed: move all objects, so loop
                    // thru the whole container content
                    pmrc
                        = (PMINIRECORDCORE)WinSendMsg(hwndCnr,
                                                      CM_QUERYRECORD,
                                                      (MPARAM)pmrc,  // NULL at first loop
                                                      MPFROM2SHORT(
                                                          (pmrc)
                                                              ? CMA_NEXT  // not first loop: get next object
                                                              : CMA_FIRST, // first loop: get first objecct
                                                          CMA_ITEMORDER)
                                                      );
                else
                    // shift _not_ pressed: move selected objects
                    // only, so loop thru these objects
                    pmrc
                        = (PMINIRECORDCORE)WinSendMsg(hwndCnr,
                                                      CM_QUERYRECORDEMPHASIS,
                                                      (pmrc)   // NULL at first loop
                                                          ? (MPARAM)pmrc
                                                          : (MPARAM)CMA_FIRST, // flag for getting first selected
                                                      (MPARAM)(CRA_SELECTED)
                                                      );
                if (pmrc)
                {
                    // record found:
                    // the WPS shares records among views, so we need
                    // to update the record core info first
                    WinSendMsg(hwndCnr,
                               CM_QUERYRECORDINFO,
                               (MPARAM)&pmrc,
                               (MPARAM)1);         // one record only
                    // un-display the new object at the old (default) location
                    WinSendMsg(hwndCnr,
                               CM_ERASERECORD,
                                    // this only changes the visibility of the
                                    // record without changing the recordcore;
                                    // this msg is intended for drag'n'drop and such
                               (MPARAM)pmrc,
                               NULL);

                    // now play with the objects coordinates
                    lNewX = ( ( (   (pmrc->ptlIcon.x - cmnQuerySetting(sulGridX))
                                  + (cmnQuerySetting(sulGridCX) / 2)
                                ) / cmnQuerySetting(sulGridCX)
                              ) * cmnQuerySetting(sulGridCX)
                            ) + cmnQuerySetting(sulGridX);
                    lNewY = ( ( (   (pmrc->ptlIcon.y - cmnQuerySetting(sulGridY))
                                  + (cmnQuerySetting(sulGridCY) / 2)
                                ) / cmnQuerySetting(sulGridCY)
                              ) * cmnQuerySetting(sulGridCY)
                            ) + cmnQuerySetting(sulGridY);

                    // update the record core
                    if ( (lNewX) && (lNewX != pmrc->ptlIcon.x) )
                        pmrc->ptlIcon.x = lNewX;         // X
                    if ( (lNewY) && (lNewY != pmrc->ptlIcon.y) )
                        pmrc->ptlIcon.y = lNewY;         // Y

                    // repaint at new position
                    WinSendMsg(hwndCnr,
                               CM_INVALIDATERECORD,
                               (MPARAM)&pmrc,
                               MPFROM2SHORT(1,     // one record only
                                   CMA_REPOSITION | CMA_ERASE));
                }
            } while (pmrc);

            brc = TRUE; // "OK" flag
        } // end if (hwndCnr)
    } // end if (hwndFrame)

    return brc;
}

#endif

/*
 * GetICONPOS:
 *
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 */

STATIC PICONPOS GetICONPOS(PORDEREDLISTITEM poli,
                           PSORTBYICONPOS psip)
{
    PICONPOS                    pip;
    CHAR                        *p;
    USHORT usStartPos = 21;     // OS/2 2.1 and above, says Henk

    // ICONPOS is defined in PMWP.H as folllows:
    //     typedef struct _ICONPOS
    //     {
    //        POINTL  ptlIcon;
    //        CHAR    szIdentity[1];
    //     } ICONPOS;
    //     typedef ICONPOS *PICONPOS;

    // now compare all the objects in the .ICONPOS structure
    // to the identity string of the search object

    for (   pip = (PICONPOS)(psip->pICONPOS + usStartPos);
            (PBYTE)pip < (psip->pICONPOS + psip->usICONPOSSize);
        )
    {   // pip now points to an ICONPOS structure

        // go beyond the class name
        if (p = strchr(pip->szIdentity, ':'))
        {
            PMPF_ORDEREDLIST(("      Identities: %s and %s...", p, poli->szIdentity));

            if (!stricmp(p, poli->szIdentity))
                // object found: return the ICONPOS address
                return pip;
            else
                // not identical: go to next ICONPOS structure
                pip = (PICONPOS)(   (PBYTE)pip
                                  + sizeof(POINTL)
                                  + strlen(pip->szIdentity)
                                  + 1);
        }
        else
            break;
    }
    return NULL;
}

/*
 * fdrSortByICONPOS:
 *      callback sort function for lstSort to sort the
 *      menu items according to a folder's ICONPOS EAs.
 *      pICONPOS points to the ICONPOS data.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 */

SHORT XWPENTRY fdrSortByICONPOS(PVOID pItem1, PVOID pItem2, PVOID psip)
{
    PMPF_ORDEREDLIST(("    Comparing %s and %s...",
            _wpQueryTitle(((PORDEREDLISTITEM)pItem1)->pObj),
            _wpQueryTitle(((PORDEREDLISTITEM)pItem2)->pObj)
        ));

    if ((pItem1) && (pItem2))
    {
        PICONPOS pip1 = GetICONPOS(((PORDEREDLISTITEM)pItem1), psip),
                 pip2 = GetICONPOS(((PORDEREDLISTITEM)pItem2), psip);

        if ((pip1) && (pip2))
            if (pip1 < pip2)
                return -1;
            else
                return 1;
        else if (pip1)
            return -1;
        else if (pip2)
            return 1;
    }

    return 0;
}

/* ******************************************************************
 *
 *   "Select some" dialog
 *
 ********************************************************************/

#define EF_LIMIT        CCHMAXPATH

/*
 *@@ AddEntryToDropDown:
 *      copies the current contents of the drop-down
 *      entry field to the list, making sure it's unique.
 *
 *@@added V0.9.19 (2002-06-18) [umoeller]
 */

VOID AddEntryToDropDown(HWND hwndDropDown,
                        BOOL fSelect)       // in: select entry field too?
{
    CHAR    szMask[EF_LIMIT];

    if (WinQueryWindowText(hwndDropDown,
                           sizeof(szMask),
                           szMask))
    {
        SHORT sLBIndex;

        if (fSelect)
        {
            WinSetFocus(HWND_DESKTOP, hwndDropDown);
            // select entire string in drop-down
            winhEntryFieldSelectAll(hwndDropDown);
        }

        // add file mask to list box (V0.9.0);
        // if it exists, delete the old one first
        sLBIndex = (SHORT)WinSendMsg(hwndDropDown,
                                     LM_SEARCHSTRING,
                                     MPFROM2SHORT(0,       // no flags
                                                  LIT_FIRST),
                                     szMask);
        if (sLBIndex != LIT_NONE)
            // found: remove
            WinDeleteLboxItem(hwndDropDown,
                              sLBIndex);

        WinInsertLboxItem(hwndDropDown,
                          0,            // at beginning
                          szMask);
    }
}

/*
 *@@ DoSelect:
 *      does the actual selection from fnwpSelectSome.
 *
 *@@added V0.9.19 (2002-04-17) [umoeller]
 */

STATIC VOID DoSelect(HWND hwndDlg,
                     BOOL fSelect,
                     BOOL fRegExp)
{
    CHAR    szMask[EF_LIMIT];

    HWND    hwndFrame;
    HWND    hwndCnr;

    if (    (hwndFrame = WinQueryWindowULong(hwndDlg, QWL_USER))
         && (hwndCnr = WinWindowFromID(hwndFrame, FID_CLIENT))
       )
    {
        HWND  hwndDropDown = WinWindowFromID(hwndDlg, ID_XFDI_SOME_ENTRYFIELD);
        SHORT sLBIndex = 0;

        WinQueryWindowText(hwndDropDown,
                           sizeof(szMask),
                           szMask);

        if (strlen(szMask))
        {
            PMINIRECORDCORE pmrc = NULL;
            ERE *pEre = NULL;
            int rc = 0;

            if (fRegExp)
                if (!(pEre = rxpCompile(szMask,
                                        0,
                                        &rc)))
                {
                    cmnErrorMsgBox(hwndDlg,
                                   rc,
                                   234,
                                   MB_OK,
                                   TRUE);

                    WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwndDlg, ID_XFDI_SOME_ENTRYFIELD));

                    return;
                }

            // now go through all the container items in hwndCnr
            // and select / deselct them accordingly
            do
            {
                pmrc =
                    (PMINIRECORDCORE)WinSendMsg(hwndCnr,
                                                CM_QUERYRECORD,
                                                (MPARAM)pmrc,
                                                MPFROM2SHORT(
                                                    (pmrc)
                                                        ? CMA_NEXT
                                                        : CMA_FIRST,
                                                    CMA_ITEMORDER)
                                                );
                if (pmrc)
                {
                    BOOL fMatch;
                    int pos, length;
                    ERE_MATCHINFO   mi;

                    if (fRegExp)
                        fMatch = (    (rxpMatch_fwd(pEre,
                                                    0,
                                                    pmrc->pszIcon,
                                                    0,
                                                    &pos,
                                                    &length,
                                                    &mi))
                                   && (pos == 0)
                                   && (length = strlen(pmrc->pszIcon))
                                 );
                    else
                        fMatch = doshMatch(szMask, pmrc->pszIcon);

                    if (fMatch)
                    {
                        WinSendMsg(hwndCnr,
                                   CM_SETRECORDEMPHASIS,
                                   pmrc,
                                   MPFROM2SHORT(
                                       // select or deselect flag
                                       fSelect,
                                       CRA_SELECTED
                                   ));
                    }
                }
            } while (pmrc);

            if (pEre)
                rxpFree(pEre);

        }

        AddEntryToDropDown(hwndDropDown,
                           TRUE);       // select
    }
}

/*
 *@@ FillDropDownFromIni:
 *      Shared by both fnwpSelectSome and fnwpBatchRename.
 *
 *@@added V0.9.19 (2002-06-18) [umoeller]
 */

STATIC VOID FillDropDownFromIni(HWND hwndDropDown,
                                PCSZ pcszIniKey) // in: XWorkplace ini key
{
    PSZ     pszLast10 = NULL;
    ULONG   cbLast10 = 0;

    // load last 10 selections from OS2.INI (V0.9.0)
    if (pszLast10 = prfhQueryProfileData(HINI_USER,
                                         INIAPP_XWORKPLACE,
                                         pcszIniKey,
                                         &cbLast10))
    {
        // something found:
        PSZ     p = pszLast10;
        while (p < (pszLast10 + cbLast10))
        {
            WinInsertLboxItem(hwndDropDown, LIT_END, p);

            p += strlen(p) + 1; // go beyond null byte
        }

        free(pszLast10);
    }
}

/*
 *@@ WriteDropDownToIni:
 *      reverse to FillDropDownFromIni, this writes
 *      up to ten entries from the drop-down back
 *      to OS2.INI.
 *
 *      Shared by both fnwpSelectSome and fnwpBatchRename.
 *
 *@@added V0.9.19 (2002-06-18) [umoeller]
 */

VOID WriteDropDownToIni(HWND hwndDropDown,
                        PCSZ pcszIniKey)
{
    ULONG   ul;
    PSZ     pszToSave = NULL;
    ULONG   cbToSave = 0;

    for (ul = 0;
         ul < 10;
         ul++)
    {
        CHAR    szEntry[EF_LIMIT];
        if (WinQueryLboxItemText(hwndDropDown,
                                 ul,
                                 szEntry,
                                 sizeof(szEntry))
            < 1)
            break;

        strhArrayAppend(&pszToSave,
                        szEntry,
                        0,
                        &cbToSave);
    }

    if (cbToSave)
        PrfWriteProfileData(HINI_USER,
                            (PSZ)INIAPP_XWORKPLACE,
                            (PSZ)pcszIniKey,
                            pszToSave,
                            cbToSave);

    if (pszToSave)
        free(pszToSave);
}

/*
 *@@ fnwpSelectSome:
 *      dlg proc for "Select by name" window, used by
 *      fdrShowSelectSome.
 *
 *      This selects or deselects items in the corresponding
 *      folder window, which is stored in QWL_USER.
 *
 *@@changed V0.9.0 [umoeller]: added drop-down box with history
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V0.9.1 (2000-02-01) [umoeller]: WinDestroyWindow was never called; fixed
 *@@changed V0.9.19 (2002-04-17) [umoeller]: added regexp; fixed entry field length
 */

STATIC MRESULT EXPENTRY fnwpSelectSome(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;
    BOOL    fWriteAndClose = FALSE;

    switch (msg)
    {
        /*
         * WM_INITDLG:
         *
         */

        case WM_INITDLG:
        {
            HWND    hwndDropDown = WinWindowFromID(hwndDlg, ID_XFDI_SOME_ENTRYFIELD);

            WinSetWindowULong(hwndDlg, QWL_USER, (ULONG)mp2); // owner frame hwnd;

            winhSetEntryFieldLimit(hwndDropDown, EF_LIMIT);
                    // was missing V0.9.19 (2002-04-17) [umoeller]

            FillDropDownFromIni(hwndDropDown,
                                INIKEY_LAST10SELECTSOME);

            // give the drop-down the focus
            WinSetFocus(HWND_DESKTOP, hwndDropDown);

            // select entire string in drop-down
            winhEntryFieldSelectAll(hwndDropDown);

            // return TRUE because we've set the focus
            mrc = (MPARAM)TRUE;
        }
        break;

        /*
         * WM_COMMAND:
         *
         */

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                /*
                 * ID_XFDI_SOME_SELECT / DESELECT:
                 *      these are the "select" / "deselect" buttons
                 */

                case ID_XFDI_SOME_SELECT:
                case ID_XFDI_SOME_DESELECT:
                    DoSelect(hwndDlg,
                             (SHORT1FROMMP(mp1) == ID_XFDI_SOME_SELECT),
                             winhIsDlgItemChecked(hwndDlg, ID_XFDI_SOME_REGEXP_CP));
                break;

                case ID_XFDI_SOME_SELECTALL:
                case ID_XFDI_SOME_DESELECTALL:
                {
                    HWND hwndFrame, hwndCnr;
                    if (    (hwndFrame = WinQueryWindowULong(hwndDlg, QWL_USER))
                         && (hwndCnr = WinWindowFromID(hwndFrame, FID_CLIENT))
                       )
                    {
                        PMINIRECORDCORE pmrc = NULL;
                        do {
                            pmrc =
                                (PMINIRECORDCORE)WinSendMsg(hwndCnr,
                                                            CM_QUERYRECORD,
                                                            (MPARAM)pmrc,
                                                            MPFROM2SHORT(
                                                                (pmrc)
                                                                    ? CMA_NEXT
                                                                    : CMA_FIRST,
                                                                CMA_ITEMORDER)
                                                            );
                            if (pmrc)
                            {
                                WinSendMsg(hwndCnr,
                                           CM_SETRECORDEMPHASIS,
                                           pmrc,
                                           MPFROM2SHORT(
                                               // select or deselect flag
                                               (SHORT1FROMMP(mp1) == ID_XFDI_SOME_SELECTALL),
                                               CRA_SELECTED
                                           ));
                            }
                        } while (pmrc);

                        winhSetDlgItemFocus(hwndDlg, ID_XFDI_SOME_ENTRYFIELD);
                        WinSendDlgItemMsg(hwndDlg, ID_XFDI_SOME_ENTRYFIELD,
                                          EM_SETSEL,
                                          MPFROM2SHORT(0, 1000), // select all
                                          MPNULL);
                    }
                }
                break;

                case DID_CANCEL:
                case DID_CLOSE:
                    WinPostMsg(hwndDlg, WM_CLOSE, 0, 0);
                break;

                /*
                 * default:
                 */

                default:
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
                break;
            }
        break;

        case WM_HELP:
            cmnDisplayHelp(NULL,
                           ID_XFH_SELECTSOME);
        break;

        /*
         * WM_CLOSE:
         *
         */

        case WM_CLOSE:
            fWriteAndClose = TRUE;
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }

    if (fWriteAndClose)
    {
        // dialog closing:
        // write drop-down entries back to OS2.INI (V0.9.0)
        WriteDropDownToIni(WinWindowFromID(hwndDlg, ID_XFDI_SOME_ENTRYFIELD),
                           INIKEY_LAST10SELECTSOME);

        winhDestroyWindow(&hwndDlg);
    }

    return mrc;
}

#define BUTTON_WIDTH        75

static const CONTROLDEF
    SomeIntroTxt = CONTROLDEF_TEXT_WORDBREAK(
                            LOAD_STRING,
                            ID_XFDI_SOME_INTROTXT,
                            -100),
    SomeMaskEF = CONTROLDEF_DROPDOWN(
                            ID_XFDI_SOME_ENTRYFIELD,
                            -100,
                            175),
    SomeRegExpCB = LOADDEF_AUTOCHECKBOX(ID_XFDI_SOME_REGEXP_CP),
    SomeCaseSensitiveCB = LOADDEF_AUTOCHECKBOX(ID_XFDI_SOME_CASESENSITIVE_CB),
    SomeSelectButton = CONTROLDEF_DEFNOFOCUSBUTTON(
                            LOAD_STRING,
                            ID_XFDI_SOME_SELECT,
                            BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT),
    SomeDeselectButton = CONTROLDEF_NOFOCUSBUTTON(
                            LOAD_STRING,
                            ID_XFDI_SOME_DESELECT,
                            BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT),
    SomeSelectAllButton = CONTROLDEF_NOFOCUSBUTTON(
                            LOAD_STRING,
                            ID_XFDI_SOME_SELECTALL,
                            BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT),
    SomeDeselectAllButton = CONTROLDEF_NOFOCUSBUTTON(
                            LOAD_STRING,
                            ID_XFDI_SOME_DESELECTALL,
                            BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT),
    SomeCloseButton = CONTROLDEF_NOFOCUSBUTTON(
                            LOAD_STRING,
                            DID_CLOSE,
                            BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT),
    SomeHelpButton = CONTROLDEF_HELPPUSHBUTTON(
                            LOAD_STRING,
                            DID_HELP,
                            BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT);

static const DLGHITEM G_dlgSelectSome[] =
    {
        START_TABLE,
            START_ROW(0),
                CONTROL_DEF(&SomeIntroTxt),
            START_ROW(0),
                CONTROL_DEF(&SomeMaskEF),
            START_ROW(0),
                CONTROL_DEF(&SomeRegExpCB),
            START_ROW(0),
                CONTROL_DEF(&SomeCaseSensitiveCB),
            START_ROW(0),
                CONTROL_DEF(&SomeSelectButton),
                CONTROL_DEF(&SomeDeselectButton),
            START_ROW(0),
                CONTROL_DEF(&SomeSelectAllButton),
                CONTROL_DEF(&SomeDeselectAllButton),
            START_ROW(0),
                CONTROL_DEF(&SomeCloseButton),
                CONTROL_DEF(&SomeHelpButton),
        END_TABLE,
    };

/*
 *@@ IsNotTreeView:
 *      returns TRUE if the FID_CLIENT of hwndFrame
 *      is not currently in tree view. Necessary for
 *      both "select some" and "batch rename" in
 *      order to prevent opening them via folder hotkeys.
 *
 *@@added V1.0.0 (2002-08-31) [umoeller]
 */

STATIC BOOL IsNotTreeView(HWND hwndFrame)
{
    HWND    hwndCnr;
    BOOL    brc = FALSE;

    if (hwndCnr = WinWindowFromID(hwndFrame, FID_CLIENT))
    {
        CNRINFO ci;
        cnrhQueryCnrInfo(hwndCnr, &ci);
        if (!(ci.flWindowAttr & CV_TREE))
            brc = TRUE;
    }

    return brc;
}

/*
 *@@ fdrShowSelectSome:
 *      shows the "Select some" window.
 *
 *@@added V0.9.19 (2002-04-17) [umoeller]
 *@@changed V0.9.19 (2002-06-18) [umoeller]: fixed leak
 *@@changed V1.0.0 (2002-08-31) [umoeller]: added folder title to window title
 *@@changed V1.0.0 (2002-08-31) [umoeller]: fixed open in tree view via fdr hotkey
 */

VOID fdrShowSelectSome(HWND hwndFrame)
{
    HWND hwndSelectSome;

    // get the folder from the frame to produce the title V1.0.0 (2002-08-31) [umoeller]
    WPFolder *pFolder;
    if (    IsNotTreeView(hwndFrame)
         && (pFolder = (WPFolder*)WinSendMsg(hwndFrame,
                                             WM_QUERYOBJECTPTR,
                                             0,
                                             0))
       )
    {
        XSTRING strTitle;
        xstrInitCopy(&strTitle, _wpQueryTitle(pFolder), 0);
        xstrcat(&strTitle, " - ", 0);
        xstrcat(&strTitle, cmnGetString(ID_XFDI_SELECTSOME_TITLE), 0);

        if (!dlghCreateDlg(&hwndSelectSome,
                           hwndFrame,         // owner
                           FCF_FIXED_DLG,
                           fnwpSelectSome,
                           strTitle.psz,
                           G_dlgSelectSome,
                           ARRAYITEMCOUNT(G_dlgSelectSome),
                           (PVOID)hwndFrame,    // dlg params
                           cmnQueryDefaultFont()))
        {
            winhPlaceBesides(hwndSelectSome,
                             WinWindowFromID(hwndFrame, FID_CLIENT),
                             PLF_SMART);
            WinShowWindow(hwndSelectSome, TRUE);
        }

        xstrClear(&strTitle);
    }
}

/* ******************************************************************
 *
 *   "Batch rename" dialog
 *
 ********************************************************************/

static const CONTROLDEF
    BatchIntroTxt = CONTROLDEF_TEXT_WORDBREAK(
                            LOAD_STRING,
                            ID_XFDI_BATCH_INTROTXT,
                            -100),
    BatchSourceTxt = LOADDEF_TEXT(
                            ID_XFDI_BATCH_SOURCETXT),
    BatchSourceEF = CONTROLDEF_DROPDOWN(
                            ID_XFDI_BATCH_SOURCEEF,
                            -100,
                            175),
    BatchTargetTxt = LOADDEF_TEXT(
                            ID_XFDI_BATCH_TARGETTXT),
    BatchTargetEF = CONTROLDEF_DROPDOWN(
                            ID_XFDI_BATCH_TARGETEF,
                            -100,
                            175),
    BatchSelectedOnlyCB = LOADDEF_AUTOCHECKBOX(ID_XFDI_BATCH_SELONLYCB),
    BatchGoButton = CONTROLDEF_DEFPUSHBUTTON(
                            LOAD_STRING,
                            ID_XFDI_BATCH_GO,
                            BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT),
    BatchCloseButton = CONTROLDEF_NOFOCUSBUTTON(
                            LOAD_STRING,
                            DID_CLOSE,
                            BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT);

static const DLGHITEM G_dlgBatchRename[] =
    {
        START_TABLE,
            START_ROW(0),
                CONTROL_DEF(&BatchIntroTxt),
            START_ROW(0),
                CONTROL_DEF(&BatchSourceTxt),
            START_ROW(0),
                CONTROL_DEF(&BatchSourceEF),
            START_ROW(0),
                CONTROL_DEF(&BatchTargetTxt),
            START_ROW(0),
                CONTROL_DEF(&BatchTargetEF),
            START_ROW(0),
                CONTROL_DEF(&SomeCaseSensitiveCB),
            START_ROW(0),
                CONTROL_DEF(&BatchSelectedOnlyCB),
            START_ROW(0),
                CONTROL_DEF(&BatchGoButton),
                CONTROL_DEF(&BatchCloseButton),
                CONTROL_DEF(&SomeHelpButton),
        END_TABLE,
    };

/*
 *@@ CompileRegexp:
 *      calls rxpCompile and displays an error box
 *      if applicable.
 *
 *@@added V0.9.19 (2002-06-18) [umoeller]
 */

ERE* CompileRegexp(HWND hwndDlg,    // in: dlg with entry field
                   ULONG ulID,      // in: entry field ID
                   PSZ psz,         // out: entry field string
                   ULONG cb,        // in: size of *psz
                   BOOL fCaseSense) // in: case sensitive match?
{
    ERE*    pEre = NULL;
    int     rc = 0;

    HWND    hwndEF;

    if (hwndEF = WinWindowFromID(hwndDlg, ulID))
    {
        if (WinQueryWindowText(hwndEF, cb, psz))
        {
            if (!fCaseSense)
                nlsUpper(psz);

            if (!(pEre = rxpCompile(psz,
                                    0,
                                    &rc)))
                cmnErrorMsgBox(hwndDlg,
                               rc,
                               234,
                               MB_OK,
                               TRUE);
        }

        if (!pEre)
            WinSetFocus(HWND_DESKTOP, hwndEF);
    }

    return pEre;
}

/*
 *@@ ConfirmRename:
 *      displays a slightly more sophisticated message
 *      box for the rename operation. Returns MBID_YES,
 *      MBID_NO, or MBID_CANCEL.
 *
 *@@added V0.9.19 (2002-06-18) [umoeller]
 */

ULONG ConfirmRename(HWND hwndOwner,
                    PCSZ pcszDlgTitle,
                    PXSTRING pstrMsg,
                    WPObject *pobj,
                    PCSZ pcszNewTitle,
                    BOOL *pfYes2All)        // in/out: "yes to all" pressed?
{
    ULONG ulrc = MBID_CANCEL;       // if something goes wrong

    if (*pfYes2All)
        ulrc = MBID_YES;
    else
    {
        #define ICON_WIDTH      50

        CONTROLDEF
            Icon1 = CONTROLDEF_ICON(NULLHANDLE, 0),
            Spacing = CONTROLDEF_TEXT(NULL, 0, 1, 1),
            RenameTxt = CONTROLDEF_TEXT_WORDBREAK(NULL, 10, MSGBOX_TEXTWIDTH - 50),
            YesButton = LOADDEF_DEFPUSHBUTTON(DID_YES),
            Yes2AllButton = LOADDEF_PUSHBUTTON(DID_YES2ALL),
            NoButton = LOADDEF_PUSHBUTTON(DID_NO),
            CancelButton = LOADDEF_PUSHBUTTON(DID_CANCEL),
            GroupObject = CONTROLDEF_GROUP(NULL, -1, SZL_AUTOSIZE, SZL_AUTOSIZE),
            IconObject = CONTROLDEF_ICON(NULLHANDLE, ICON_WIDTH),
            IconTitle = CONTROLDEF_TEXT(NULLHANDLE, 0, ICON_WIDTH, SZL_AUTOSIZE);

        DLGHITEM dlgConfirm[] =
            {
                // outer table: two colums (1 == dlg icon, 2== all the rest)
                START_TABLE,
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&Icon1),
                    START_TABLE,
                        // all the rest table: two rows
                        // 1 == inner table,
                        // 2 == buttons
                        START_ROW(ROW_VALIGN_CENTER),
                            START_TABLE,
                                // inner table:
                                // two columns,
                                // 1 == text,
                                // 2 == object's icon
                                START_ROW(0),
                                    START_TABLE,
                                        START_ROW(ROW_VALIGN_CENTER),
                                            CONTROL_DEF(&Spacing),
                                        START_ROW(ROW_VALIGN_CENTER),
                                            CONTROL_DEF(&RenameTxt),
                                        START_ROW(ROW_VALIGN_CENTER),
                                            CONTROL_DEF(&Spacing),
                                    END_TABLE,
                                    START_GROUP_TABLE(&GroupObject),
                                        START_ROW(0),
                                            CONTROL_DEF(&IconObject),
                                        START_ROW(0),
                                            CONTROL_DEF(&IconTitle),
                                    END_TABLE,
                            END_TABLE,
                        START_ROW(ROW_VALIGN_CENTER),
                            CONTROL_DEF(&YesButton),
                            CONTROL_DEF(&Yes2AllButton),
                            CONTROL_DEF(&NoButton),
                            CONTROL_DEF(&CancelButton),
                    END_TABLE,
                END_TABLE
            };

        PCSZ apcsz[2] =
                {
                    _wpQueryTitle(pobj),
                    pcszNewTitle
                };

        HWND        hwndConfirm;

        cmnGetMessage(apcsz, 2,
                      pstrMsg,
                      238);  // rename %1 to %2?

        // set controls
        Icon1.pcszText = (PCSZ)cmnQueryDlgIcon();
        RenameTxt.pcszText = pstrMsg->psz;
        IconObject.pcszText = (PCSZ)_wpQueryIcon(pobj);
        IconTitle.pcszText = apcsz[0];

        if (!dlghCreateDlg(&hwndConfirm,
                           hwndOwner,
                           FCF_FIXED_DLG,
                           WinDefDlgProc,
                           pcszDlgTitle,
                           dlgConfirm,
                           ARRAYITEMCOUNT(dlgConfirm),
                           NULL,
                           cmnQueryDefaultFont()))
        {
            winhCenterWindow(hwndConfirm);
            switch (WinProcessDlg(hwndConfirm))
            {
                case DID_YES:
                    ulrc = MBID_YES;
                break;

                case DID_YES2ALL:
                    ulrc = MBID_YES;
                    *pfYes2All = TRUE;
                break;

                case DID_NO:
                    ulrc = MBID_NO;
                break;

                case DID_CANCEL:
                    ulrc = MBID_CANCEL;
                break;
            }

            winhDestroyWindow(&hwndConfirm);
        }
    }

    return ulrc;
}

/*
 *@@ DoRename:
 *      does the actual rename operation from fnwpBatchRename
 *      by throwing the regexps at every record core in the
 *      folder.
 *
 *@@added V0.9.19 (2002-06-18) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: fixed wrong "rename everything to upper case"
 *@@changed V1.0.1 (2002-12-19) [umoeller]: raised output buffer size to avoid "string too long"
 */

VOID DoRename(HWND hwndDlg)
{
    HWND    hwndFrame;
    HWND    hwndCnr,
            hwndSource,
            hwndTarget;
    PMINIRECORDCORE pmrc = NULL;
    CHAR    szSource[EF_LIMIT],
            szTarget[EF_LIMIT];
    ERE     *pEreSource = NULL;

    PCSZ    pcszDlgTitle = cmnGetString(ID_XFDI_BATCHRENAME_TITLE);

    BOOL    fSelectedOnly = winhIsDlgItemChecked(hwndDlg, ID_XFDI_BATCH_SELONLYCB),
            fCaseSense = winhIsDlgItemChecked(hwndDlg, ID_XFDI_SOME_CASESENSITIVE_CB);

    if (    (hwndFrame = WinQueryWindowULong(hwndDlg, QWL_USER))
         && (hwndCnr = WinWindowFromID(hwndFrame, FID_CLIENT))
         && (hwndSource = WinWindowFromID(hwndDlg, ID_XFDI_BATCH_SOURCEEF))
         && (hwndTarget = WinWindowFromID(hwndDlg, ID_XFDI_BATCH_TARGETEF))
         && (pEreSource = CompileRegexp(hwndDlg,
                                        ID_XFDI_BATCH_SOURCEEF,
                                        szSource,
                                        sizeof(szSource),
                                        fCaseSense))
         && (WinQueryWindowText(hwndTarget,
                                sizeof(szTarget),
                                szTarget))
       )
    {
        // now go through all the container items in hwndCnr
        // and match them against pEreSource
        BOOL    fCancelled = FALSE,
                fSaveTarget = TRUE;
        XSTRING strMsg;
        ULONG   cMatched = 0,
                cRenamed = 0;

        BOOL    fYes2All = FALSE;       // set to TRUE by ConfirmRename

        xstrInit(&strMsg, 0);

        while (!fCancelled)
        {
            WPObject *pobjThis;
            PCSZ     pcszTitleOrig,
                     pcszTitleMatch;
            if (    (pmrc = (PMINIRECORDCORE)WinSendMsg(hwndCnr,
                                                        CM_QUERYRECORD,
                                                        (MPARAM)pmrc,
                                                        MPFROM2SHORT(
                                                            (pmrc)
                                                                ? CMA_NEXT
                                                                : CMA_FIRST,
                                                            CMA_ITEMORDER)
                                                        ))
                 && (pobjThis = OBJECT_FROM_PREC(pmrc))
                 && (pcszTitleOrig = _wpQueryTitle(pobjThis))
                            // record might not be up to date... better make sure
               )
            {
                BOOL    fMatch;
                int     pos,
                        length;
                ERE_MATCHINFO   mi;

                CHAR    szThis[CCHMAXPATH];

                if (!fCaseSense)
                {
                    strlcpy(szThis,
                            pcszTitleOrig,
                            sizeof(szThis));
                    nlsUpper(szThis);
                    pcszTitleMatch = szThis;
                }
                else
                    pcszTitleMatch = pcszTitleOrig;

                fMatch = (    (    (!fSelectedOnly)
                                || (pmrc->flRecordAttr & CRA_SELECTED)
                              )
                           && (rxpMatch_fwd(pEreSource,
                                            0,
                                            pcszTitleMatch,
                                            0,
                                            &pos,
                                            &length,
                                            &mi))
                           && (pos == 0)
                           && (length = strlen(pmrc->pszIcon))
                         );

                if (fMatch)
                {
                    // source matches:
                    // create target filename
                    CHAR szNewTitle[3 * CCHMAXPATH];        // raised V1.0.1 (2002-12-19) [umoeller]
                    int rc;
                    if (!rxpSubsWith(pcszTitleOrig, // pcszTitleMatch,
                                            // use orig title, or everything ends up in upper case
                                            // V0.9.20 (2002-07-25) [umoeller]
                                     pos,
                                     length,
                                     &mi,
                                     // replacement regexp:
                                     szTarget,
                                     // output buffer:
                                     szNewTitle,
                                     sizeof(szNewTitle),
                                     &rc))
                    {
                        // substition error:
                        cmnErrorMsgBox(hwndDlg,
                                       rc,
                                       239,
                                       MB_CANCEL,
                                       TRUE);

                        // we can't continue
                        fCancelled = TRUE;

                        // and don't save target
                        fSaveTarget = FALSE;
                    }
                    else
                    {
                        // count matches, even if no rename
                        ++cMatched;

                        switch (ConfirmRename(hwndDlg,
                                              pcszDlgTitle,
                                              &strMsg,
                                              pobjThis,
                                              szNewTitle,
                                              &fYes2All))
                        {
                            case MBID_YES:
                                if (fopsRenameObjectConfirmed(pobjThis,
                                                              szNewTitle))
                                    // count renamed too
                                    ++cRenamed;
                            break;

                            case MBID_NO:
                            break;

                            case MBID_CANCEL:
                                fCancelled = TRUE;
                            break;
                        }
                    }
                }
            }
            else
                // no more records:
                break;
        }; // while (!fCancelled)

        if (!cMatched)
        {
            if (!fCancelled)
            {
                // if nothing _matched_, report that, or the user
                // gets no response at all (note, no response if
                // no _rename_)
                cmnGetMessage(NULL, 0,
                              &strMsg,
                              240);     // no objects
                cmnMessageBox(hwndDlg,
                              pcszDlgTitle,
                              strMsg.psz,
                              0,
                              MB_OK);
            }
        }
        else
        {
            // something matched:
            // report results if not cancelled
            if (!fCancelled)
            {
                CHAR szMatched[30], szRenamed[30];
                PCSZ apcsz[2] =
                        {
                            szMatched,
                            szRenamed
                        };
                PCOUNTRYSETTINGS2 pcs = cmnQueryCountrySettings(FALSE);
                nlsThousandsULong(szMatched,
                                  cMatched,
                                  pcs->cs.cThousands);
                nlsThousandsULong(szRenamed,
                                  cRenamed,
                                  pcs->cs.cThousands);

                cmnGetMessage(apcsz, 2,
                              &strMsg,
                              241);     // %1 object(s) matched your find criteria. %2 object(s) were renamed.

                cmnMessageBox(hwndDlg,
                              pcszDlgTitle,
                              strMsg.psz,
                              0,
                              MB_OK);
            }

            // save if we had _matches_ (irrespective of renames),
            // and even if we cancelled
            AddEntryToDropDown(hwndSource, TRUE);
            rxpFree(pEreSource);

            // we get here also if the target failed, so check
            if (fSaveTarget)
                AddEntryToDropDown(hwndTarget, FALSE);
        }

        xstrClear(&strMsg);
    }
}

/*
 *@@ fnwpBatchRename:
 *      dlg proc for "batch rename" window, used by
 *      fdrShowBatchRename.
 *
 *      This selects or deselects items in the corresponding
 *      folder window, which is stored in QWL_USER.
 *
 *@@added V0.9.19 (2002-06-18) [umoeller]
 *@@changed V0.9.20 (2002-07-03) [umoeller]: "start renaming" button wasn't enabled properly, fixed
 */

STATIC MRESULT EXPENTRY fnwpBatchRename(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;
    BOOL    fWriteAndClose = FALSE;

    switch (msg)
    {
        case WM_INITDLG:
        {
            HWND    hwndDropDown;

            WinSetWindowULong(hwndDlg, QWL_USER, (ULONG)mp2); // owner frame hwnd;

            hwndDropDown = WinWindowFromID(hwndDlg, ID_XFDI_BATCH_SOURCEEF);
            winhSetEntryFieldLimit(hwndDropDown, EF_LIMIT);
            FillDropDownFromIni(hwndDropDown,
                                INIKEY_LAST10BATCHSOURCE);

            hwndDropDown = WinWindowFromID(hwndDlg, ID_XFDI_BATCH_TARGETEF);
            winhSetEntryFieldLimit(hwndDropDown, EF_LIMIT);
            FillDropDownFromIni(hwndDropDown,
                                INIKEY_LAST10BATCHTARGET);

            WinPostMsg(hwndDlg, XM_ENABLEITEMS, 0, 0);
        }
        break;

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case ID_XFDI_BATCH_GO:
                    DoRename(hwndDlg);
                break;

                case DID_CLOSE:
                case DID_CANCEL:
                    WinPostMsg(hwndDlg, WM_CLOSE, 0, 0);
                break;
            }
        break;

        case WM_CONTROL:
            switch (SHORT1FROMMP(mp1))
            {
                case ID_XFDI_BATCH_SOURCEEF:
                case ID_XFDI_BATCH_TARGETEF:
                    if (SHORT2FROMMP(mp1) == CBN_EFCHANGE)
                            // V0.9.20 (2002-07-03) [umoeller]
                    {
                        WinPostMsg(hwndDlg, XM_ENABLEITEMS, 0, 0);
                    }
                break;

                default:
                    mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
            }
        break;

        case XM_ENABLEITEMS:
            WinEnableControl(hwndDlg,
                             ID_XFDI_BATCH_GO,
                             (    (WinQueryDlgItemTextLength(hwndDlg, ID_XFDI_BATCH_SOURCEEF))
                               && (WinQueryDlgItemTextLength(hwndDlg, ID_XFDI_BATCH_TARGETEF))
                             ));
        break;

        case WM_HELP:
            cmnDisplayHelp(NULL,
                           ID_XFH_BATCHRENAME);
        break;

        /*
         * WM_CLOSE:
         *
         */

        case WM_CLOSE:
            fWriteAndClose = TRUE;
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }

    if (fWriteAndClose)
    {
        // dialog closing:
        WriteDropDownToIni(WinWindowFromID(hwndDlg, ID_XFDI_BATCH_SOURCEEF),
                           INIKEY_LAST10BATCHSOURCE);
        WriteDropDownToIni(WinWindowFromID(hwndDlg, ID_XFDI_BATCH_TARGETEF),
                           INIKEY_LAST10BATCHTARGET);
        winhDestroyWindow(&hwndDlg);
    }

    return mrc;
}

/*
 *@@ fdrShowBatchRename:
 *      shows the "Batch rename" window.
 *
 *@@added V0.9.19 (2002-06-18) [umoeller]
 *@@changed V1.0.0 (2002-08-31) [umoeller]: added folder title to window title
 *@@changed V1.0.0 (2002-08-31) [umoeller]: fixed open in tree view via fdr hotkey
 */

VOID fdrShowBatchRename(HWND hwndFrame)
{
    HWND hwndSelectSome;

    // get the folder from the frame to produce the title V1.0.0 (2002-08-31) [umoeller]
    WPFolder *pFolder;
    if (    IsNotTreeView(hwndFrame)
         && (pFolder = (WPFolder*)WinSendMsg(hwndFrame,
                                             WM_QUERYOBJECTPTR,
                                             0,
                                             0))
       )
    {
        XSTRING strTitle;
        xstrInitCopy(&strTitle, _wpQueryTitle(pFolder), 0);
        xstrcat(&strTitle, " - ", 0);
        xstrcat(&strTitle, cmnGetString(ID_XFDI_BATCHRENAME_TITLE), 0);

        if (!dlghCreateDlg(&hwndSelectSome,
                           hwndFrame,         // owner
                           FCF_FIXED_DLG,
                           fnwpBatchRename,
                           strTitle.psz,
                           G_dlgBatchRename,
                           ARRAYITEMCOUNT(G_dlgBatchRename),
                           (PVOID)hwndFrame,    // dlg params
                           cmnQueryDefaultFont()))
        {
            winhPlaceBesides(hwndSelectSome,
                             WinWindowFromID(hwndFrame, FID_CLIENT),
                             PLF_SMART);
            WinShowWindow(hwndSelectSome, TRUE);
        }

        xstrClear(&strTitle);
    }
}

/* ******************************************************************
 *
 *   Replacement "Paste" dialog
 *
 ********************************************************************/

/*
 *@@ PASTEDLGDATA:
 *
 *@@added V0.9.20 (2002-08-08) [umoeller]
 */

typedef struct _PASTEDLGDATA
{
    HWND            hwndFrame,      // folder frame
                    hwndDlg;        // paste dlg

    HAB             hab;

    HWND            hwndPreview,
                    hwndObjTitle,
                    hwndFormat,
                    hwndClass;

    WPFolder        *pFolder;       // folder to paste to

} PASTEDLGDATA, *PPASTEDLGDATA;

HWND        G_hwndOpenPasteDlg = NULLHANDLE;
                    // there's only one clipboard, so allow one paste dialog only

typedef ULONG _System ENUMCLIPBOARDCLASSES(M_WPDataFile *somSelf, ULONG aCBFormat, ULONG aPrevious);
typedef ULONG _System RENDERFROMCLIPBOARD(WPDataFile *somSelf, ULONG aRenderAs);

/*
 *@@ winhInsertLboxItemHandle:
 *
 *@@added V0.9.20 (2002-08-08) [umoeller]
 */

SHORT winhInsertLboxItemHandle(HWND hwndListbox,
                               SHORT sIndex,
                               PCSZ pcsz,
                               ULONG ulHandle)
{
    SHORT s;

    s = (SHORT)WinSendMsg(hwndListbox,
                          LM_INSERTITEM,
                          (MPARAM)sIndex,
                          (MPARAM)pcsz);
    WinSendMsg(hwndListbox,
               LM_SETITEMHANDLE,
               (MPARAM)s,
               (MPARAM)ulHandle);

    return s;
}

/*
 *@@ PasteQuerySelectedFormat:
 *
 *@@added V0.9.20 (2002-08-08) [umoeller]
 */

ULONG PasteQuerySelectedFormat(PPASTEDLGDATA pData)
{
    SHORT s = winhQueryLboxSelectedItem(pData->hwndFormat, LIT_FIRST);
    return winhQueryLboxItemHandle(pData->hwndFormat, s);
}

/*
 *@@ PasteFillClassesForFormat:
 *
 *@@added V0.9.20 (2002-08-08) [umoeller]
 */

STATIC BOOL PasteFillClassesForFormat(PPASTEDLGDATA pData)
{
    BOOL    brc = FALSE;
    HATOMTBL hat;
    ENUMCLIPBOARDCLASSES *_wpclsEnumClipboardClasses;

    ULONG   ulFormat = PasteQuerySelectedFormat(pData);

    CHAR    szLastClass[100];
    BOOL    fSelected = FALSE;

    PrfQueryProfileString(HINI_USER,
                          (PSZ)INIAPP_XWORKPLACE,
                          (PSZ)INIKEY_LASTPASTECLASS,
                          "",
                          szLastClass,
                          sizeof(szLastClass));

    winhDeleteAllItems(pData->hwndClass);
    WinSetWindowText(pData->hwndClass, "");

    if (    (hat = WinQuerySystemAtomTable())
         && (_wpclsEnumClipboardClasses = (ENUMCLIPBOARDCLASSES*)wpshResolveFor(_WPDataFile,
                                                                                NULL,
                                                                                "wpclsEnumClipboardClasses"))
       )
    {
        ULONG ulClass;
        for (ulClass = _wpclsEnumClipboardClasses(_WPDataFile, ulFormat, 0);
             ulClass;
             ulClass = _wpclsEnumClipboardClasses(_WPDataFile, ulFormat, ulClass))
        {
            CHAR szTemp[100];
            if (WinQueryAtomName(hat,
                                 ulClass,
                                 szTemp,
                                 sizeof(szTemp)))
            {
                SHORT s;

                s = winhInsertLboxItemHandle(pData->hwndClass,
                                             LIT_END,
                                             szTemp,
                                             // remember the ulClass as the listbox item handle
                                             ulClass);

                if (!strcmp(szTemp, szLastClass))
                {
                    winhSetLboxSelectedItem(pData->hwndClass,
                                            s,
                                            TRUE);
                    fSelected = TRUE;
                }
            }

            brc = TRUE;
        }

        if (!fSelected)
            winhSetLboxSelectedItem(pData->hwndClass,
                                    0,
                                    TRUE);
    }

    return brc;
}

/*
 *@@ PasteFillControls:
 *
 *      Preconditions:
 *
 *      --  Caller must have opened the clipboard, which
 *          is closed here.
 *
 *@@added V0.9.20 (2002-08-08) [umoeller]
 */

STATIC BOOL PasteFillControls(PPASTEDLGDATA pData)
{
    BOOL    brc = FALSE,
            fClip = TRUE;

    HATOMTBL hat;

    if (hat = WinQuerySystemAtomTable())
    {

        // 1) enumerate clipboard formats

        ULONG   ulFormat,
                ulCurrentFormat = -1,
                cbCurrentFormat = sizeof(ulCurrentFormat),
                cFormats = 0;
        CHAR    szTemp[300];
        SHORT   s;
        BOOL    fSelectedFormat = FALSE;

        PrfQueryProfileData(HINI_USER,
                            (PSZ)INIAPP_XWORKPLACE,
                            (PSZ)INIKEY_LASTPASTEFORMAT,
                            &ulCurrentFormat,
                            &cbCurrentFormat);

        winhDeleteAllItems(pData->hwndFormat);

        for (ulFormat = WinEnumClipbrdFmts(pData->hab, 0);
             ulFormat;
             ulFormat = WinEnumClipbrdFmts(pData->hab, ulFormat))
        {
            SHORT   sIndex;
            PCSZ    pcszFormatName = NULL;

            switch (ulFormat)
            {
                case CF_BITMAP:
                    pcszFormatName = "Bitmap";
                break;

                case CF_METAFILE:
                    pcszFormatName = "Metafile";
                break;

                case CF_TEXT:
                    pcszFormatName = "Plain text";

                    WinSetWindowText(pData->hwndPreview,
                                     (PSZ)WinQueryClipbrdData(pData->hab, CF_TEXT));
                break;

                default:
                    if (WinQueryAtomName(hat,
                                         ulFormat,
                                         szTemp,
                                         sizeof(szTemp)))
                        pcszFormatName = szTemp;
            }

            if (pcszFormatName)
            {
                ++cFormats;

                s = winhInsertLboxItemHandle(pData->hwndFormat,
                                             LIT_END,
                                             pcszFormatName,
                                             // remember the ulFormat as the listbox item handle
                                             ulFormat);

                if (ulCurrentFormat == ulFormat)
                {
                    winhSetLboxSelectedItem(pData->hwndFormat,
                                            s,
                                            TRUE);
                    fSelectedFormat = TRUE;
                }
            }
        }

        if (!fSelectedFormat)
            winhSetLboxSelectedItem(pData->hwndFormat,
                                    0,
                                    TRUE);

        WinCloseClipbrd(pData->hab);
        fClip = FALSE;

        // 2) enumerate available classes

        if (cFormats)
            brc = PasteFillClassesForFormat(pData);
    }

    if (fClip)
        WinCloseClipbrd(pData->hab);

    return brc;
}

PFNWP G_pfnwpPreviewStaticOrig = NULL;

/*
 *@@ fnwpPreviewer:
 *      window proc for the subclassed static control
 *      that paints itself as text or bitmap depending
 *      on the current clipboard format.
 *
 *@@added V0.9.20 (2002-08-10) [umoeller]
 */

STATIC MRESULT EXPENTRY fnwpPreviewer(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_PAINT:
        {
            HPS hps;
            if (hps = WinBeginPaint(hwnd, NULLHANDLE, NULL))
            {
                HAB hab = WinQueryAnchorBlock(hwnd);
                if (WinOpenClipbrd(hab))
                {
                    RECTL   rcl;
                    PSZ     pszText;
                    HBITMAP hbm;

                    WinQueryWindowRect(hwnd, &rcl);
                    WinFillRect(hps, &rcl, CLR_WHITE);

                    if (pszText = (PSZ)WinQueryClipbrdData(hab, CF_TEXT))
                    {
                        winhDrawFormattedText(hps,
                                              &rcl,
                                              pszText,
                                              DT_TOP | DT_LEFT);
                    }
                    else if (hbm = (HBITMAP)WinQueryClipbrdData(hab, CF_BITMAP))
                    {
                        gpihStretchBitmap(hps,
                                          hbm,
                                          NULL,
                                          &rcl,
                                          TRUE);        // proportional
                    }

                    WinCloseClipbrd(hab);
                }

                WinEndPaint(hps);
            }
        }
        break;

        default:
            mrc = G_pfnwpPreviewStaticOrig(hwnd, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ fnwpPaste:
 *      window proc for the replacement "Paste" dialog.
 *
 *      This not only has the same fields as the WPS "Paste"
 *      dialog, but remembers the values that were used
 *      between several paste operations so that you don't
 *      have to re-select the same format and class every
 *      time. I found that especially annoying with pasting
 *      URLs from the Mozilla URL bar.
 *
 *      We also remember the last ten object titles that
 *      were used.
 *
 *      In addition, we set ourselves as the clipboard viewer
 *      while we're visible. This is not only handy for the
 *      user to see what he's pasting actually, but allows
 *      us to detect when the clipboard data has changed
 *      (which the WPS dialog simply won't know about).
 *
 *@@added V0.9.20 (2002-08-08) [umoeller]
 */

STATIC MRESULT EXPENTRY fnwpPaste(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        /*
         * WM_INITDLG:
         *      mp2 has PPASTEDLGDATA.
         */

        case WM_INITDLG:
        {
            PPASTEDLGDATA pData = (PPASTEDLGDATA)mp2;
            WinSetWindowPtr(hwndDlg, QWL_USER, pData);

            pData->hab = WinQueryAnchorBlock(hwndDlg);

            pData->hwndDlg = hwndDlg;

            pData->hwndPreview = WinWindowFromID(hwndDlg, ID_XFDI_PASTE_PREVIEW_PANE);
            pData->hwndObjTitle = WinWindowFromID(hwndDlg, ID_XFDI_PASTE_OBJTITLE_DROP);
            pData->hwndFormat = WinWindowFromID(hwndDlg, ID_XFDI_PASTE_FORMAT_DROP);
            pData->hwndClass = WinWindowFromID(hwndDlg, ID_XFDI_PASTE_CLASS_DROP);

            WinSetWindowPtr(pData->hwndPreview, QWL_USER, pData);
            G_pfnwpPreviewStaticOrig = WinSubclassWindow(pData->hwndPreview,
                                                         fnwpPreviewer);

            winhSetPresColor(pData->hwndPreview,
                             PP_BACKGROUNDCOLOR,
                             RGBCOL_WHITE);
            winhSetPresColor(pData->hwndPreview,
                             PP_FOREGROUNDCOLOR,
                             RGBCOL_BLACK);
            winhSetWindowFont(pData->hwndPreview,
                              "5.System VIO");

            // fill formats and classes
            PasteFillControls(pData);

            // fill titles
            FillDropDownFromIni(pData->hwndObjTitle,
                                INIKEY_LAST10PASTETITLES);

            winhSetLboxSelectedItem(pData->hwndObjTitle, 0, TRUE);
            WinSetFocus(HWND_DESKTOP, pData->hwndObjTitle);
            mrc = (MRESULT)TRUE;        // we changed focus
            winhEntryFieldSelectAll(pData->hwndObjTitle);

            // set the previewer as the clipboard viewer
            WinSetClipbrdViewer(pData->hab,
                                hwndDlg);

            WinPostMsg(hwndDlg, XM_ENABLEITEMS, 0, 0);
        }
        break;

        /*
         * WM_DRAWCLIPBOARD:
         *      since we're the clipboard viewer, we receive
         *      this message whenever the clipboard changes.
         */

        case WM_DRAWCLIPBOARD:
        {
            PPASTEDLGDATA pData = WinQueryWindowPtr(hwndDlg, QWL_USER);

            // refresh the drop-downs
            PasteFillControls(pData);

            // have the preview static repaint itself
            WinInvalidateRect(pData->hwndPreview, NULL, FALSE);
        }
        break;

        case WM_CONTROL:
            switch (SHORT1FROMMP(mp1))
            {
                case ID_XFDI_PASTE_OBJTITLE_DROP:
                    if (SHORT2FROMMP(mp1) == CBN_EFCHANGE)
                        WinPostMsg(hwndDlg, XM_ENABLEITEMS, 0, 0);
                break;

                case ID_XFDI_PASTE_FORMAT_DROP:
                    if (SHORT2FROMMP(mp1) == CBN_LBSELECT)
                    {
                        // refresh the classes if a new format is selected
                        PPASTEDLGDATA pData = WinQueryWindowPtr(hwndDlg, QWL_USER);
                        PasteFillClassesForFormat(pData);

                        WinPostMsg(hwndDlg, XM_ENABLEITEMS, 0, 0);
                    }
                break;

                case ID_XFDI_PASTE_CLASS_DROP:
                    if (SHORT2FROMMP(mp1) == CBN_LBSELECT)
                        WinPostMsg(hwndDlg, XM_ENABLEITEMS, 0, 0);
                break;
            }
        break;

        case XM_ENABLEITEMS:
        {
            PPASTEDLGDATA pData = WinQueryWindowPtr(hwndDlg, QWL_USER);
            WinEnableControl(hwndDlg,
                             DID_OK,
                                WinQueryWindowTextLength(pData->hwndObjTitle)
                             && WinQueryWindowTextLength(pData->hwndFormat)
                             && WinQueryWindowTextLength(pData->hwndClass)
                            );
        }
        break;

        case WM_HELP:
        {
            PPASTEDLGDATA pData = WinQueryWindowPtr(hwndDlg, QWL_USER);
            cmnDisplayHelp(pData->pFolder, ID_XSH_PASTEDLG);
        }
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }

    return mrc;
}

#define CX_RIGHT        120

static const CONTROLDEF
    PastePreviewTxt = LOADDEF_TEXT(ID_XFDI_PASTE_PREVIEW_TXT),
    PastePreviewPane = CONTROLDEF_TEXT_WORDBREAK_CY("", ID_XFDI_PASTE_PREVIEW_PANE, CX_RIGHT, 30),
    PasteObjTitleTxt = LOADDEF_TEXT(ID_XFDI_PASTE_OBJTITLE_TXT),
    PasteObjTitleDrop = CONTROLDEF_DROPDOWN(ID_XFDI_PASTE_OBJTITLE_DROP, CX_RIGHT, 50),
    PasteFormatTxt = LOADDEF_TEXT(ID_XFDI_PASTE_FORMAT_TXT),
    PasteFormatDrop = CONTROLDEF_DROPDOWNLIST(ID_XFDI_PASTE_FORMAT_DROP, CX_RIGHT, 50),
    PasteClassTxt = LOADDEF_TEXT(ID_XFDI_PASTE_CLASS_TXT),
    PasteClassDrop = CONTROLDEF_DROPDOWNLIST(ID_XFDI_PASTE_CLASS_DROP, CX_RIGHT, 50);

static const DLGHITEM G_dlgPaste[] =
    {
        START_TABLE,
            START_ROW(0),
                START_TABLE_ALIGN,
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&PastePreviewTxt),
                        CONTROL_DEF(&PastePreviewPane),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&PasteObjTitleTxt),
                        CONTROL_DEF(&PasteObjTitleDrop),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&PasteFormatTxt),
                        CONTROL_DEF(&PasteFormatDrop),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&PasteClassTxt),
                        CONTROL_DEF(&PasteClassDrop),
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&G_OKButton),
                CONTROL_DEF(&G_CancelButton),
                CONTROL_DEF(&G_HelpButton),
        END_TABLE
    };

/*
 *@@ DoPaste:
 *      pastes the clipboard contents to a new file
 *      in the given folder.
 *
 *      This creates a new file of the given class
 *      and calls _wpRenderFromClipboard (Warp 4
 *      method) on it with the given ulFormat.
 *
 *      We also call _wpConfirmObjectTitle to have
 *      title conflicts resolved.
 *
 *      Returns the new object that was created,
 *      which has been locked once.
 *
 *@@added V0.9.20 (2002-08-08) [umoeller]
 */

STATIC WPDataFile* DoPaste(WPFolder *pFolder,
                           PSZ pszClass,               // in: WPS class to use for new file
                           PSZ pszFilename,            // in: filename for new file
                           ULONG cbFilename,           // in: filename buffer size (needed for _wpConfirmObjectTitle)
                           ULONG ulFormat)             // in: clipboard format for _wpRenderFromClipboard
{
    WPObject *pobjNew = NULL;

    somId   somidClassName;
    RENDERFROMCLIPBOARD *_wpRenderFromClipboard;

    if (somidClassName = somIdFromString(pszClass))
    {
        SOMClass *pClassObject;
        if (pClassObject = _somFindClass(SOMClassMgrObject,
                                         somidClassName,
                                         0,
                                         0))
        {
            if (pobjNew = _wpclsNew(pClassObject,
                                    pszFilename,
                                    NULL,
                                    pFolder,
                                    TRUE))            // lock
            {
                WPObject *pobj2 = pobjNew;
                switch (_wpConfirmObjectTitle(pobjNew,
                                              pFolder,
                                              &pobj2,
                                              pszFilename,
                                              cbFilename,
                                              110))     // rename code
                {
                    case NAMECLASH_RENAME:
                        _wpSetTitle(pobjNew, pszFilename);
                    break;

                    case NAMECLASH_CANCEL:
                        _wpFree(pobjNew);
                        pobjNew = NULL;
                    break;
                }

                if (    (pobjNew)
                     && (_wpRenderFromClipboard = (RENDERFROMCLIPBOARD*)wpshResolveFor(pobjNew,
                                                                                       NULL,
                                                                                       "wpRenderFromClipboard"))
                   )
                {
                    if (!_wpRenderFromClipboard(pobjNew,
                                                ulFormat))
                    {
                        _wpFree(pobjNew);
                        pobjNew = NULL;
                    }
                }
            }
        }

        SOMFree(somidClassName);
    }

    return pobjNew;
}

/*
 *@@ fdrShowPasteDlg:
 *      called from mnuMenuItemSelected when WPMENUID_PASTE
 *      comes in to show our replacement "Paste" dialog.
 *
 *      See fnwpPaste.
 *
 *@@added V0.9.20 (2002-08-08) [umoeller]
 */

VOID fdrShowPasteDlg(WPFolder *pFolder,
                     HWND hwndFrame)
{
    PPASTEDLGDATA pData;

    // if we have another open paste dialog,
    // active it and return
    if (G_hwndOpenPasteDlg)
    {
        WinAlarm(HWND_DESKTOP, WA_WARNING);
        WinSetActiveWindow(HWND_DESKTOP, G_hwndOpenPasteDlg);
        return;
    }

    // we open the clipboard here, which is closed during
    // WM_INITDLG in PasteFillControls; we shouldn't even
    // show the dialog if the clipboard is empty
    if (    (WinOpenClipbrd(winhMyAnchorBlock()))
         && (pData = NEW(PASTEDLGDATA))
       )
    {
        PSZ pszTitle = strdup(cmnGetString(ID_XFDI_PASTE_TITLE));
        ULONG ulOfs = 0;

        pData->hwndFrame = hwndFrame;
        pData->pFolder = pFolder;

        strhFindReplace(&pszTitle, &ulOfs, "%1", _wpQueryTitle(pFolder));

        if (!dlghCreateDlg(&G_hwndOpenPasteDlg,
                           hwndFrame,         // owner
                           FCF_FIXED_DLG,
                           fnwpPaste,
                           pszTitle,
                           G_dlgPaste,
                           ARRAYITEMCOUNT(G_dlgPaste),
                           (PVOID)pData,
                           cmnQueryDefaultFont()))
        {
            ULONG ulrc;

            winhPlaceBesides(G_hwndOpenPasteDlg,
                             WinWindowFromID(hwndFrame, FID_CLIENT),
                             PLF_SMART);
            ulrc = WinProcessDlg(G_hwndOpenPasteDlg);

            WinSetClipbrdViewer(pData->hab,
                                NULLHANDLE);

            if (DID_OK == ulrc)
            {
                CHAR    szFilename[CCHMAXPATH];
                CHAR    szClass[100];
                ULONG   ulFormat = PasteQuerySelectedFormat(pData);

                WinQueryWindowText(pData->hwndObjTitle, sizeof(szFilename), szFilename);
                WinQueryWindowText(pData->hwndClass, sizeof(szClass), szClass);

                if (DoPaste(pData->pFolder,
                            szClass,
                            szFilename,
                            sizeof(szFilename),
                            ulFormat))
                {
                    AddEntryToDropDown(pData->hwndObjTitle, FALSE);
                    WriteDropDownToIni(pData->hwndObjTitle,
                                       INIKEY_LAST10PASTETITLES);

                    PrfWriteProfileData(HINI_USER,
                                        (PSZ)INIAPP_XWORKPLACE,
                                        (PSZ)INIKEY_LASTPASTEFORMAT,
                                        &ulFormat,
                                        sizeof(ulFormat));

                    PrfWriteProfileString(HINI_USER,
                                          (PSZ)INIAPP_XWORKPLACE,
                                          (PSZ)INIKEY_LASTPASTECLASS,
                                          szClass);
                }
            }

            winhDestroyWindow(&G_hwndOpenPasteDlg);
        }

        free(pData);
    }
}

/* ******************************************************************
 *
 *   Start folder contents
 *
 ********************************************************************/

/*
 *@@ PROCESSFOLDER:
 *      structure on the stack of fdrStartFolderContents
 *      while the "start folder contents" thread is
 *      running synchronously.
 *
 *@@added V0.9.12 (2001-04-29) [umoeller]
 */

typedef struct _PROCESSFOLDER
{
    // input parameters:
    WPFolder        *pFolder;
    ULONG           ulTiming;
    HWND            hwndStatus;             // status window or NULLHANDLE

    BOOL            fCancelled;

    // private data:
    WPObject        *pObject;
    ULONG           henum;
    ULONG           cTotalObjects;
    ULONG           ulObjectThis;
    ULONG           ulFirstTime;            // sysinfo first time
} PROCESSFOLDER, *PPROCESSFOLDER;

/*
 *@@ fnwpStartupDlg:
 *      dlg proc for the Startup status window, which
 *      runs on the main PM thread (krn_fnwpThread1Object).
 */

MRESULT EXPENTRY fnwpStartupDlg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc;

    switch (msg)
    {
        case WM_INITDLG:
            // WinSetWindowULong(hwnd, QWL_USER, (ULONG)mp2);
                // we don't need this here, it's done by krn_fnwpThread1Object
            ctlProgressBarFromStatic(WinWindowFromID(hwnd, ID_SDDI_PROGRESSBAR),
                                     PBA_ALIGNCENTER | PBA_BUTTONSTYLE);
            mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
        break;

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case DID_CANCEL:
                {
                    PPROCESSFOLDER ppf;
                    if (ppf = (PPROCESSFOLDER)WinQueryWindowPtr(hwnd, QWL_USER))
                        ppf->fCancelled = TRUE;
                }
                break;
            }
        break;

        case WM_SYSCOMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case SC_CLOSE:
                case SC_HIDE:
                    cmnSetSetting(sfShowStartupProgress, 0);
                    mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
                break;

                default:
                    mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
            }
        break;

        default:
            mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ fntProcessStartupFolder:
 *      synchronous thread started from fdrStartFolderContents
 *      for each startup folder that is to be processed.
 *
 *@@added V0.9.12 (2001-04-29) [umoeller]
 *@@changed V1.0.1 (2003-01-29) [umoeller]: added proper return code
 */

void _Optlink fntProcessStartupFolder(PTHREADINFO ptiMyself)
{
    APIRET          arc = NO_ERROR;     // V1.0.1 (2003-01-29) [umoeller]

    PPROCESSFOLDER  ppf = (PPROCESSFOLDER)ptiMyself->ulData;
    WPFolder        *pFolder = ppf->pFolder;

    while (!arc)
    {
        BOOL    fOKGetNext = FALSE;
        HWND    hwndCurrentView = NULLHANDLE;       // for wait mode

        if (ppf->ulObjectThis == 0)
        {
            // first iteration: initialize structure
            ppf->cTotalObjects = 0;
            fdrCheckIfPopulated(pFolder,
                                FALSE);        // full populate
            // now count objects
            for (   ppf->pObject = _wpQueryContent(pFolder, NULL, QC_FIRST);
                    (ppf->pObject);
                    ppf->pObject = _wpQueryContent(pFolder, ppf->pObject, QC_NEXT)
                )
            {
                ppf->cTotalObjects++;
            }

            // get first object
            ppf->henum = _xwpBeginEnumContent(pFolder);
        }

        if (!ppf->henum)
            arc = 999;
        else
        {
            // in any case, get first or next object
            ppf->pObject = _xwpEnumNext(pFolder, ppf->henum);

            ppf->ulObjectThis++;

            // now process that object
            if (ppf->pObject)
            {
                // this is not the last object: start it

                // resolve shadows... this never worked for
                // shadows V0.9.12 (2001-04-29) [umoeller]
                ppf->pObject = _xwpResolveIfLink(ppf->pObject);

                if (wpshCheckObject(ppf->pObject))
                {
                    // open the object:

                    // 1) update the status window
                    if (cmnQuerySetting(sfShowStartupProgress))
                    {
                        CHAR szStarting2[500], szTemp[500];
                        // update status text ("Starting xxx")
                        strlcpy(szTemp, _wpQueryTitle(ppf->pObject), sizeof(szTemp));
                        strhBeautifyTitle(szTemp);
                        sprintf(szStarting2,
                                cmnGetString(ID_SDSI_STARTING),
                                szTemp);
                        WinSetDlgItemText(ppf->hwndStatus, ID_SDDI_STATUS, szStarting2);
                    }

                    // V0.9.19 (2002-04-11) [pr]: Must open on thread 1!!!
                    // have the object opened on thread-1
                    hwndCurrentView = (HWND)krnSendThread1ObjectMsg(T1M_OPENOBJECTFROMPTR,
                                                                    (MPARAM)ppf->pObject,
                                                                    (MPARAM)OPEN_DEFAULT);

                    // update status bar
                    if (cmnQuerySetting(sfShowStartupProgress))
                        WinSendDlgItemMsg(ppf->hwndStatus, ID_SDDI_PROGRESSBAR,
                                          WM_UPDATEPROGRESSBAR,
                                          MPFROMLONG(ppf->ulObjectThis),
                                          MPFROMLONG(ppf->cTotalObjects));
                }

                ppf->ulFirstTime = doshQuerySysUptime();
            }
            else
                // no more objects:
                // break out of the while loop
                break;
        }

        // now wait until we should process the
        // next object
        while (!arc && !fOKGetNext)
        {
            if (ppf->ulTiming == 0)
            {
                // "wait for close" mode:
                // check if the view we opened is still alive

                // now, for all XFolder versions up to now
                // we used wpWaitForClose... I am very unsure
                // what this method does, so I have now replaced
                // this with my own loop here.
                // V0.9.12 (2001-04-28) [umoeller]
                BOOL fStillOpen = FALSE;
                WPObject *pobjLock = NULL;

                TRY_LOUD(excpt1)
                {
                    if (pobjLock = cmnLockObject(ppf->pObject))
                    {
                        PUSEITEM    pUseItem = NULL;

                        PMPF_STARTUP(("  WOM_WAITFORPROCESSNEXT: checking open views"));
                        PMPF_STARTUP(("  obj %s",
                                        _wpQueryTitle(ppf->pObject)));

                        for (pUseItem = _wpFindUseItem(ppf->pObject, USAGE_OPENVIEW, NULL);
                             pUseItem;
                             pUseItem = _wpFindUseItem(ppf->pObject, USAGE_OPENVIEW, pUseItem))
                        {
                            PVIEWITEM pvi = (PVIEWITEM)(pUseItem + 1);

                            PMPF_STARTUP(("    got view 0x%lX", pvi->handle));

                            if (pvi->handle == hwndCurrentView)
                            {
                                fStillOpen = TRUE;
                                break;
                            }
                        }
                    }
                }
                CATCH(excpt1) {} END_CATCH();

                if (pobjLock)
                    _wpReleaseObjectMutexSem(pobjLock);

                fOKGetNext = !fStillOpen;
            } // end if (ppf->ulTiming == 0)
            else
            {
                // timing mode
                ULONG ulNowTime = doshQuerySysUptime();
                if (ulNowTime > (ppf->ulFirstTime + ppf->ulTiming))
                    fOKGetNext = TRUE;
            }

            // fOKGetNext is TRUE if the next object should be
            // processed now
            if (fOKGetNext)
            {
                // removed lock here V0.9.12 (2001-04-28) [umoeller]
                PCKERNELGLOBALS pKernelGlobals = krnQueryGlobals();
                if (pKernelGlobals)
                {
                    // ready to go for next object:
                    // make sure the damn PM hard error windows are not visible,
                    // because this locks any other PM activity
                    if (    (WinIsWindowVisible(pKernelGlobals->hwndHardError))
                         || (WinIsWindowVisible(pKernelGlobals->hwndSysError))
                       )
                    {
                        DosBeep(250, 100);
                        // wait a little more and try again
                        fOKGetNext = FALSE;
                    }
                }
            }

            // fixed cancel V1.0.1 (2003-01-29) [umoeller]
            if (ppf->fCancelled)
            {
                arc = ERROR_INTERRUPT;
                break;
            }

            if (!fOKGetNext)
                // not ready yet:
                // sleep awhile (we could simply sleep for the
                // wait time, but then "cancel" would not be
                // very responsive)
                DosSleep(100);
        } // while (!fOKGetNext)

    } // end while (!arc)

    // done or cancelled:
    _xwpEndEnumContent(pFolder, ppf->henum);

    if (arc == 999)
        // no objects is not an error, just for breaking out above
        arc = NO_ERROR;

    // tell thrRunSync that we're done
    WinPostMsg(ptiMyself->hwndNotify,
               WM_USER,
               (MPARAM)arc,         // V1.0.1 (2003-01-29) [umoeller]
               0);
}

/*
 *@@ fdrStartFolderContents:
 *      implementation for XFolder::xwpStartFolderContents.
 *
 *@@added V0.9.12 (2001-04-29) [umoeller]
 *@@changed V0.9.13 (2001-06-27) [umoeller]: now setting status title to folder's
 */

ULONG fdrStartFolderContents(WPFolder *pFolder,
                             ULONG ulTiming)
{
    ULONG ulrc = 0;

    PROCESSFOLDER       pf;

    memset(&pf, 0, sizeof(pf));

    pf.ulTiming = ulTiming;

    pf.hwndStatus = cmnLoadDlg(NULLHANDLE,
                               fnwpStartupDlg,
                               ID_XFD_STARTUPSTATUS,
                               NULL);
    // store struct in window words so the dialog can cancel
    WinSetWindowPtr(pf.hwndStatus, QWL_USER, &pf);

    // set title V0.9.13 (2001-06-27) [umoeller]
    WinSetWindowText(pf.hwndStatus, _wpQueryTitle(pFolder));

    if (cmnQuerySetting(sfShowStartupProgress))
    {
        // get last window position from INI
        winhRestoreWindowPos(pf.hwndStatus,
                             HINI_USER,
                             INIAPP_XWORKPLACE, INIKEY_WNDPOSSTARTUP,
                             // move only, no resize
                             SWP_MOVE | SWP_SHOW | SWP_ACTIVATE);
    }

    pf.pFolder = pFolder;

    ulrc = thrRunSync(WinQueryAnchorBlock(pf.hwndStatus),
                      fntProcessStartupFolder,
                      "ProcessStartupFolder",
                      (ULONG)&pf);

    winhSaveWindowPos(pf.hwndStatus, HINI_USER, INIAPP_XWORKPLACE, INIKEY_WNDPOSSTARTUP);
    winhDestroyWindow(&pf.hwndStatus);

    return ulrc;
}


