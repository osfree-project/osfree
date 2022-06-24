
/*
 *@@sourcefile wpsh.h:
 *      header file for wpsh.c ("pseudo SOM methods").
 *      See remarks there.
 *
 *      Note: before #include'ing this header, you must
 *      have at least wpobject.h #include'd. Functions which
 *      operate on other WPS classes will only be declared
 *      if the necessary header has been #include'd already.
 *
 *@@include #define INCL_WINWINDOWMGR
 *@@include #include <os2.h>
 *@@include #include <wpdisk.h>     // only for some funcs
 *@@include #include <wpfolder.h>   // only for some funcs
 *@@include #include <wpobject.h>   // only if no other WPS header is included
 *@@include #include "wpsh.h"
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

#ifndef XWPS_HEADER_INCLUDED
    #define XWPS_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   SOM helpers
     *
     ********************************************************************/

    PVOID wpshResolveFor(SOMObject *somSelf,
                         SOMClass *pClass,
                         PCSZ pcszMethodName);

    PVOID wpshParentNumResolve(SOMClass *pClass,
                               somMethodTabs parentMTab,
                               PCSZ pcszMethodName);

    BOOL wpshOverrideStaticMethod(SOMClass *somSelf,
                                  PCSZ pcszMethodName,
                                  somMethodPtr pMethodPtr);

    /* ******************************************************************
     *
     *   WPObject helpers
     *
     ********************************************************************/

    APIRET wpshStore(WPObject *somSelf,
                     PSZ *ppszTarget,
                     PCSZ pcszSource,
                     PULONG pulLength);

    BOOL wpshCheckObject(WPObject *pObject);

    ULONG wpshQueryView(WPObject* somSelf,
                        HWND hwndFrame);

    BOOL wpshIsViewCnr(WPObject *somSelf,
                       HWND hwndCnr);

    WPObject* wpshQuerySourceObject(WPFolder *somSelf,
                                    HWND hwndCnr,
                                    BOOL fKeyboardMode,
                                    PULONG pulSelection);

    WPObject* wpshQueryNextSourceObject(HWND hwndCnr,
                                        WPObject *pObject);

    BOOL wpshCloseAllViews(WPObject *pObject);

    ULONG wpshQueryDraggedObject(PDRAGITEM pdrgItem,
                                 WPObject **ppObjectFound);

    MRESULT wpshQueryDraggedObjectCnr(PCNRDRAGINFO pcdi,
                                      HOBJECT *phObject);

    /* ******************************************************************
     *
     *   WPFolder helpers
     *
     ********************************************************************/

    #ifdef SOM_WPDisk_h
        WPFolder* wpshQueryRootFolder(WPDisk* somSelf, BOOL bForceMap,
                                      APIRET *parc);
    #endif

    #ifdef SOM_WPFolder_h
        double wpshQueryDiskFreeFromFolder(WPFolder *somSelf);

        double wpshQueryDiskSizeFromFolder(WPFolder *somSelf);

        BOOL wpshResidesBelow(WPObject *pChild, WPFolder *pFolder);

        WPFileSystem*  wpshContainsFile(WPFolder *pFolder, PCSZ pcszRealName);

        WPObject* wpshCreateFromTemplate(WPObject *pTemplate,
                                         WPFolder* pFolder,
                                         HWND hwndFrame,
                                         USHORT usOpenSettings,
                                         BOOL fReposition,
                                         POINTL* pptlMenuMousePos);

        HWND wpshQueryFrameFromView(WPFolder *somSelf, ULONG ulView);

    #endif

    ULONG wpshQueryLogicalDisk(WPObject *somSelf);

    /* ******************************************************************
     *
     *   Additional WPObject method prototypes
     *
     ********************************************************************/

    /*
     *  For each method, we declare xfTP_methodname for the
     *  the prototype and xfTD_methodname for a function pointer.
     *
     *  IMPORTANT NOTE: Make sure these things have the _System
     *  calling convention. Normally, SOM uses #pragma linkage
     *  in the headers to ensure this, but we must do this manually.
     *  The usual SOMLINK does _not_ suffice.
     */

    /*
     * xfTP_wpModifyMenu:
     *      prototype for WPObject::wpModifyMenu.
     *
     *      See the Warp 4 Toolkit documentation for details.
     *
     *      From my testing (after overriding _all_ WPDataFile methods...),
     *      I found out that wpDisplayMenu apparently calls the following
     *      methods in this order:
     *
     *      --  wpFilterMenu (Warp-4-specific);
     *      --  wpFilterPopupMenu;
     *      --  wpModifyPopupMenu;
     *      --  wpModifyMenu (Warp-4-specific).
     */

    typedef BOOL _System xfTP_wpModifyMenu(WPObject*,
                                           HWND,
                                           HWND,
                                           ULONG,
                                           ULONG,
                                           ULONG,
                                           ULONG);
    typedef xfTP_wpModifyMenu *xfTD_wpModifyMenu;

    /* ******************************************************************
     *
     *   Method call helpers
     *
     ********************************************************************/

    /* BOOL wpshParentQuerySetup2(WPObject *somSelf,
                               SOMClass *pClass,
                               PVOID pstrSetup);
            removed V1.0.1 (2002-12-08) [umoeller]
            */
#if 0

    /*
     *@@ xfTP_get_pobjNext:
     *      prototype for WPObject::_get_pobjNext (note the
     *      extra underscore).
     *
     *      Each WPObject apparently has a "pobjNext" member,
     *      which points to the "next" object if that object
     *      has been inserted in a folder. This is how the
     *      WPFolder::wpQueryContent method works... it keeps
     *      calling _get_pobjNext on the previous object.
     *
     *      Apparently, pobjNext is a SOM "attribute", which
     *      is an instance variable for which SOM automatically
     *      generates "get" methods so that they can be accessed.
     *      The nice thing about this is that this also allows
     *      us to get a direct pointer to the address of the
     *      attribute (i.e. the instance variable).
     *
     *      See the SOM Programming guide for more about attributes.
     *
     *      As a consequence, calling _get_pobjNext gives us
     *      the address of the "pobjNext" instance variable,
     *      which is a WPObject pointer.
     *
     *      Note that WPFolder also defines the _get_FirstObj
     *      and _get_LastObj attributes, which define the head
     *      and the tail of the folder contents list.
     *
     *@@added V0.9.7 (2001-01-13) [umoeller]
     */

    typedef WPObject** _System xfTP_get_pobjNext(WPObject*);
    typedef xfTP_get_pobjNext *xfTD_get_pobjNext;

    /*
     *@@ xfTP_get_FirstObj:
     *      prototype for WPFolder::_get_FirstObj (note the
     *      extra underscore).
     *
     *      See xfTP_get_pobjNext for explanations.
     *
     *@@added V0.9.7 (2001-01-13) [umoeller]
     */

    typedef WPObject** _System xfTP_get_FirstObj(WPFolder*);
    typedef xfTP_get_FirstObj *xfTD_get_FirstObj;

#endif

#endif
