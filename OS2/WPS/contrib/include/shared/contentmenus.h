
/*
 *@@sourcefile contentmenus.h:
 *      header file for contentmenus.c.
 *      Some declarations for menus.c are still in common.h however.
 *
 *@@include #define INCL_WINMENUS
 *@@include #include <os2.h>
 *@@include #include <wpfolder.h> // WPFolder
 *@@include #include "filesys\contentmenus.h"
 */

/*
 *      Copyright (C) 1997-2003 Ulrich M”ller.
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

#ifndef CONTENTMENUS_HEADER_INCLUDED
    #define CONTENTMENUS_HEADER_INCLUDED

    #ifndef INCL_WINMENUS
        #error "INCL_WINMENUS needs to be #define'd before including contentmenus.h"
    #endif

    /* ******************************************************************
     *
     *   Declarations
     *
     ********************************************************************/

    // markers for XFolder variable menu items:
    // these flags are used in the global linked list
    // of menu items which are inserted into folder
    // context menus to determine what we need to do
    // if a menu item gets selected
    #define OC_PROGRAM              2     // program object
    #define OC_FOLDER               3     // folder: insert submenu
    #define OC_ACTION               4     // not used
    #define OC_TEMPLATE             5     // template: create new object
    #define OC_OTHER                1     // all others: simply open

    #define OC_CONTENT              7     // this item is part of the "folder content" menu
    #define OC_CONTENTFOLDER        8     // the same, but it's a folder

    #define OC_SEPARATOR            10    // program object, but separator only

    #define OC_XWPSTRING            11    // XWPString object V0.9.14 (2001-07-14) [umoeller]

    #ifdef SOM_WPObject_h

        /*
         *@@ VARMENULISTITEM:
         *      linked list item for variable menu items
         *      inserted into folder context menus; these
         *      list items are created for both config folder
         *      and folder content items.
         *
         *      This is returned also from cmnuGetVarItem.
         *
         *@@changed V0.9.19 (2002-06-18) [umoeller]: now allocating size dynamically
         */

        typedef struct _VARMENULISTITEM
        {
            WPObject    *pObject;       // object pointer
            ULONG       ulObjType;
                    // for folder content menus, one of the following:
                    // -- OC_CONTENTFOLDER: content submenu (folder)
                    // -- OC_CONTENT: non-folder item (any class)
                    // for other menus, any of the OC_* flags

            ULONG       ulTitleLen;     // strlen(szTitle)

            PSZ         pszTitle;       // object title; points to
                                        // after aptlPositions
                                        // NOTE: can be NULL!

            POINTL      aptlPositions[1];
                                // array of ulTitleLen POINTL structs for
                                // the GpiQueryCharString call in
                                // cmnuMeasureItem (points to after szTitle)
        } VARMENULISTITEM, *PVARMENULISTITEM;

        /*
         *@@ CONTENTLISTITEM:
         *      list item structure for holding one item
         *      in the config folders.
         *
         *@@added V0.9.0 [umoeller]
         */

        typedef struct _CONTENTLISTITEM
        {
            WPObject        *pObject;           // object pointer
            CHAR            szTitle[100];        // object title (beautified)
            ULONG           ulObjectType;       // OC_* flags
            PLINKLIST       pllFolderContent;   // if != NULL, pObject is a folder
                                                // and this is another list of CONTENTLISTITEMs
        } CONTENTLISTITEM, *PCONTENTLISTITEM;

        /*
         *@@ CONTENTMENULISTITEM:
         *      additional linked list item for
         *      "folder content" menus.
         */

        typedef struct _CONTENTMENULISTITEM
        {
            WPFolder                    *pFolder;
            SHORT                       sMenuId;
        } CONTENTMENULISTITEM, *PCONTENTMENULISTITEM;

    #endif // SOM_WPObject_h

    /* ******************************************************************
     *
     *   Global variables
     *
     ********************************************************************/

    // original wnd proc for folder content menus,
    // which we must subclass (var sits in folder.c)
    // extern PFNWP G_pfnwpFolderContentMenuOriginal;

    extern SHORT G_sNextMenuId;

    extern ULONG G_ulVarItemCount;

    /* extern BOOL G_fFldrContentMenuMoved,
                G_fFldrContentMenuButtonDown; */

    /* ******************************************************************
     *
     *   Functions
     *
     ********************************************************************/

    VOID cmnuInitItemCache(VOID); // PCGLOBALSETTINGS pGlobalSettings);

    BOOL cmnuAppendMi2List(WPObject *pObject, ULONG ulObjType);

    VOID cmnuAppendFldr2ContentList(WPFolder *pFolder, SHORT sMenuId);

    ULONG cmnuInsertOneObjectMenuItem(HWND hAddToMenu,
                                      USHORT iPosition,
                                      PCSZ pcszNewItemString,
                                      USHORT afStyle,
                                      WPObject *pObject,
                                      ULONG ulObjType);

    VOID cmnuSetPositionBelow(PPOINTL pptlBelow);

    SHORT cmnuPrepareContentSubmenu(WPFolder *somSelf,
                                    HWND hwndMenu,
                                    PCSZ pcszTitle,
                                    USHORT iPosition,
                                    BOOL fOwnerDraw);

    VOID cmnuInsertObjectsIntoMenu(WPFolder *pFolder,
                                   HWND hwndMenu);

    VOID cmnuFillContentSubmenu(SHORT sMenuId,
                                HWND hwndMenu);

    PVARMENULISTITEM cmnuGetVarItem(ULONG ulOfs);

    /* ******************************************************************
     *
     *   Functions for folder content menu ownerdraw
     *
     ********************************************************************/

    VOID cmnuPrepareOwnerDraw(HWND hwndMenuMsg);

    MRESULT cmnuMeasureItem(POWNERITEM poi);
                            // PCGLOBALSETTINGS pGlobalSettings);

    BOOL cmnuDrawItem(MPARAM mp1, MPARAM mp2);

#endif
