
/*
 *@@sourcefile fonts.h:
 *      header file for fonts.c (font folder implementation).
 *
 *      This file is ALL new with V0.9.7.
 *
 *@@include #include <os2.h>
 *@@include #include "classes\xfont.h"
 *@@include #include "config\fonts.h"
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

#ifndef FONTS_HEADER_INCLUDED
    #define FONTS_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Declarations
     *
     ********************************************************************/

    #define HINTS_MAX_ASCENDER_DESCENDER_GRAYRECT   0x0001
    #define HINTS_INTERNALLEADING_GRAYRECT          0x0002
    #define HINTS_BASELINE_REDLINE                  0x0004
    #define HINTS_LOWERCASEASCENT_REDRECT           0x0008

    extern ULONG G_ulFontSampleHints;

    /* ******************************************************************
     *
     *   General font management
     *
     ********************************************************************/

    APIRET fonGetFontDescription(HAB hab,
                                 PCSZ pcszFilename,
                                 PSZ pszFamily,
                                 PSZ pszFace,
                                 ULONG cbBufs);

    #if defined (SOM_XWPFontFile_h) && defined (SOM_XWPFontFolder_h)
        APIRET fonInstallFont(HAB hab,
                              XWPFontFolder *pFontFolder,
                              XWPFontFile *pNewFontFile,
                              WPObject **ppNewFontObj);
    #endif

    #if defined (SOM_XWPFontObject_h) && defined (SOM_XWPFontFolder_h)
        APIRET fonDeInstallFont(HAB hab,
                                XWPFontFolder *pFontFolder,
                                XWPFontObject *pFontObject);
    #endif

    /* ******************************************************************
     *
     *   Font folder implementation
     *
     ********************************************************************/

    #ifdef SOM_XWPFontFolder_h
        VOID fonPopulateFirstTime(XWPFontFolder *pFolder);

        MRESULT fonDragOver(XWPFontFolder *pFontFolder,
                            PDRAGINFO pdrgInfo);

        MRESULT fonDrop(XWPFontFolder *pFontFolder,
                        PDRAGINFO pdrgInfo);

        BOOL fonProcessViewCommand(WPFolder *somSelf,
                                   USHORT usCommand,
                                   HWND hwndCnr,
                                   WPObject* pFirstObject,
                                   ULONG ulSelectionFlags);
    #endif

    #ifdef NOTEBOOK_HEADER_INCLUDED
        VOID XWPENTRY fonSampleTextInitPage(PNOTEBOOKPAGE pnbp,
                                            ULONG flFlags);

        MRESULT XWPENTRY fonSampleTextItemChanged(PNOTEBOOKPAGE pnbp,
                                         ULONG ulItemID,
                                         USHORT usNotifyCode,
                                         ULONG ulExtra);
    #endif

    /* ******************************************************************
     *
     *   Font object implementation
     *
     ********************************************************************/

    #ifdef SOM_XWPFontObject_h
        VOID fonModifyFontPopupMenu(XWPFontObject *somSelf,
                                    HWND hwndMenu);

        BOOL fonMenuItemSelected(XWPFontObject *somSelf,
                                 HWND hwndFrame,
                                 ULONG ulMenuId);

        BOOL fonMenuItemHelpSelected(XWPFontObject *somSelf,
                                     ULONG ulMenuId);

        HWND fonCreateFontSampleView(XWPFontObject *somSelf,
                                     HAB hab,
                                     ULONG ulView);
    #endif

    ULONG fonInvalidateAllOpenSampleViews(VOID);

#endif


