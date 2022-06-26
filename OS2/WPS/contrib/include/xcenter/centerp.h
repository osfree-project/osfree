
/*
 *@@sourcefile centerp.h:
 *      private header file for the XCenter.
 *      See src\shared\center.c for an introduction to
 *      the XCenter.
 *
 *@@include #include "helpers\linklist.h"
 *@@include #include "shared\errors.h"
 *@@include #include "classes\xcenter.h"
 *@@include #include "xcenter\center.h"
 *@@include #include "xcenter\centerp.h"
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
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

#ifndef CENTERP_HEADER_INCLUDED
    #define CENTERP_HEADER_INCLUDED

    #ifndef CENTER_HEADER_INCLUDED
        #error shared\center.h must be included before xcenter\centerp.h.
    #endif

    #ifndef SOM_XCenter_h
        #error classes\xcenter.h must be included before xcenter\centerp.h.
    #endif

    /* ******************************************************************
     *
     *   Global variables (ctr_setup.c)
     *
     ********************************************************************/

    extern const XWPSETUPENTRY    G_XCenterSetupSet[];

    extern ULONG                  G_cXCenterSetupSetEntries;

    /* ******************************************************************
     *
     *   XCenter model (ctr_model.c)
     *
     ********************************************************************/

    #define TRAY_WIDGET_CLASS_NAME "Tray"

    #ifdef LINKLIST_HEADER_INCLUDED

        typedef struct _TRAYSETTING *PTRAYSETTING;

        /*
         *@@ PRIVATEWIDGETSETTING:
         *      private wrapper around XCENTERWIDGETSETTING
         *      to allow for additional data to be stored
         *      with widget settings.
         *
         *      This was added with V0.9.13 to allow for
         *      subwidget settings in a tray setting to
         *      be stored with together with the regular
         *      tray settings.
         *
         *      This way we can keep XCenter's clean
         *      separation between widget _settings_
         *      and widget _views_ even though (tray)
         *      widgets can have subwidgets too.
         *
         *      The subwidget views are stored in the tray
         *      widget's private view data (see TRAYWIDGETPRIVATE)
         *      since they are only needed if the tray widget
         *      is currently visible.
         *
         *@@added V0.9.13 (2001-06-21) [umoeller]
         */

        typedef struct _PRIVATEWIDGETSETTING
        {
            XCENTERWIDGETSETTING    Public;
                        // public definition, has widget
                        // class and setup string

            PTRAYSETTING            pOwningTray;
                        // if this != NULL, this is a subwidget
                        // setting and this contains the owning
                        // tray;
                        // otherwise (for root widgets) this is NULL

            ULONG                   ulCurrentTray;
                        // for tray widgets only: the current tray
                        // or -1 if no tray is currently active
                        // V0.9.14 (2001-08-01) [umoeller]

            PLINKLIST               pllTraySettings;
                        // for tray widgets only: linked list of
                        // TRAYSETTING structures with the tray
                        // and subwidget definitions for each tray
                        // (no auto-free)

        } PRIVATEWIDGETSETTING, *PPRIVATEWIDGETSETTING;

        /*
         *@@ TRAYSETTING:
         *      describes one tray in a tray widget definition.
         *
         *      This has a linked list of TRAYSUBWIDGET structures
         *      in turn, which describe the subwidgets for that
         *      tray.
         */

        typedef struct _TRAYSETTING
        {
            PSZ         pszTrayName;    // tray name (malloc'd)

            LINKLIST    llSubwidgetSettings;
                                    // linked list of PRIVATEWIDGETSETTING structs,
                                    // no auto-free

        } TRAYSETTING;

    #endif

    /* ******************************************************************
     *
     *   XCenter widget class management
     *
     ********************************************************************/

    APIRET ctrpRegisterCategory(VOID);

    BOOL ctrpLockClasses(VOID);

    VOID ctrpUnlockClasses(VOID);

    VOID ctrpLoadClasses(VOID);

    #ifdef LINKLIST_HEADER_INCLUDED
        PLINKLIST ctrpQueryClasses(VOID);
    #endif

    VOID ctrpFreeClasses(VOID);

    APIRET ctrpFindClass(PCSZ pcszWidgetClass,
                         BOOL fMustBeTrayable,
                         PCXCENTERWIDGETCLASS *ppClass);

    HWND ctrpAddWidgetsMenu(XCenter *somSelf,
                            HWND hwndMenu,
                            SHORT sPosition,
                            PCSZ pcszMenuName,
                            BOOL fTrayableOnly);

    PXCENTERWIDGETCLASS ctrpFindClassFromMenuCommand(USHORT usCmd);

    /* ******************************************************************
     *
     *   Widget settings management
     *
     ********************************************************************/

    APIRET ctrpCheckClass(PCSZ pcszWidgetClass,
                          BOOL fMustBeTrayable);

    #ifdef LINKLIST_HEADER_INCLUDED

        APIRET XWPENTRY ctrpCreateWidgetSetting(XCenter *somSelf,
                                                PTRAYSETTING pTray,
                                                PCSZ pcszWidgetClass,
                                                PCSZ pcszSetupString,
                                                ULONG ulBeforeIndex,
                                                PPRIVATEWIDGETSETTING *ppNewSetting,
                                                PULONG pulNewItemCount,
                                                PULONG pulNewWidgetIndex);

        APIRET ctrpFindWidgetSetting(XCenter *somSelf,
                                     PWIDGETPOSITION pPosition,
                                     PPRIVATEWIDGETSETTING *ppSetting,
                                     PXCENTERWIDGET *ppViewData);

        VOID XWPENTRY ctrpFreeSettingData(PPRIVATEWIDGETSETTING *ppSetting);

        APIRET ctrpRemoveWidgetSetting(XCenter *somSelf,
                                       PPRIVATEWIDGETSETTING *ppSetting);

        PTRAYSETTING XWPENTRY ctrpCreateTraySetting(PPRIVATEWIDGETSETTING ppws,
                                                    PCSZ pcszTrayName,
                                                    PULONG pulIndex);

        APIRET ctrpFindTraySetting(XCenter *somSelf,
                                   ULONG ulTrayWidgetIndex,
                                   ULONG ulTrayIndex,
                                   PPRIVATEWIDGETSETTING *ppTrayWidgetSetting,
                                   PTRAYSETTING *ppTraySetting,
                                   PXCENTERWIDGET *ppTrayWidget);

        BOOL XWPENTRY ctrpDeleteTraySetting(XCenter *somSelf,
                                            PPRIVATEWIDGETSETTING ppws,
                                            ULONG ulIndex);

        PLINKLIST ctrpQuerySettingsList(XCenter *somSelf);

    #endif

    ULONG ctrpQueryWidgetsCount(XCenter *somSelf);

    VOID ctrpFreeWidgets(XCenter *somSelf);

    PSZ ctrpStuffSettings(XCenter *somSelf,
                          PULONG pcbSettingsArray);

    ULONG ctrpUnstuffSettings(XCenter *somSelf);

    /* ******************************************************************
     *
     *   XCenter views (ctr_engine.c)
     *
     ********************************************************************/

    // PM window class names for built-in widgets
    #define WNDCLASS_WIDGET_OBJBUTTON   "XWPCenterObjButtonWidget"
    #define WNDCLASS_WIDGET_PULSE       "XWPCenterPulseWidget"
    #define WNDCLASS_WIDGET_TRAY        "XWPCenterTrayWidget"

    MRESULT EXPENTRY fnwpObjButtonWidget(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
    MRESULT EXPENTRY fnwpPulseWidget(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
    MRESULT EXPENTRY fnwpTrayWidget(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

    // undocumented window style for "topmost"
    #ifndef WS_TOPMOST
        #define WS_TOPMOST  0x00200000L
    #endif

    #define TIMERID_XTIMERSET       1

    #define TIMERID_UNFOLDFRAME     2
    #define TIMERID_SHOWWIDGETS     3
    #define TIMERID_AUTOHIDE_START  4
    #define TIMERID_AUTOHIDE_RUN    5

    #define MAX_UNFOLDFRAME         500     // ms total unfold time
    #define MAX_AUTOHIDE            500     // ms total autohide animation time

    #ifdef LINKLIST_HEADER_INCLUDED

        /*
         *@@ PRIVATEWIDGETVIEW:
         *      private structure stored in the XCenter's
         *      XCENTERWINDATA.llWidgets list for each
         *      open widget.
         *
         *      This contains the XCENTERWIDGET structure
         *      that the widget is allowed to see. A
         *      direct pointer into that structure is
         *      passed to the widget on WM_CREATE, while
         *      we can use this to cache additional data
         *      that the widget itself should not see.
         *
         *      Since XCENTERWIDGET is the first member of
         *      this structure, one can use the QWL_USER
         *      window ptr for getting this structure
         *      from a widget window as well.
         *
         *@@added V0.9.7 (2000-12-14) [umoeller]
         */

        typedef struct _PRIVATEWIDGETVIEW
        {
            XCENTERWIDGET   Widget;
                        // widget's public data; this must be
                        // the first member of this structure,
                        // or WM_DESTROY in ctrDefWidgetProc
                        // won't work, which relies on these
                        // two structs having the same address

            SIZEL           szlWanted;
                        // the widget's desired size (as was
                        // last queried using WM_CONTROL with
                        // XN_QUERYSIZE)
            SIZEL           szlCurrent;
                        // widget's current width and height;
                        // the width is normally the same as
                        // szlWanted.cx, except if the widget
                        // is sizeable

            LONG            xCurrent;
                        // widget's current position; y is fixed currently
            LONG            xSizingBar;
                        // position of sizing bar to draw after this
                        // widget, if Widget.fSizeable == TRUE;
                        // otherwise this is 0

            struct _PRIVATEWIDGETVIEW *pOwningTrayWidget;
                        // NULL if this widget is a direct child
                        // of the XCenter client; otherwise this
                        // has a pointer to the tray widget which
                        // owns this widget
                        // V0.9.13 (2001-06-19) [umoeller]
                        // renamed V0.9.16 (2001-10-18) [umoeller]

            PLINKLIST        pllSubwidgetViews;
                        // if this is a tray widget, linked list of
                        // PRIVATEWIDGETVIEW structures containing
                        // the subwidget views for the current tray,
                        // similar to XCenter's own list
                        // (invisible trays never have subwidget views)
                        // V0.9.13 (2001-06-23) [umoeller]

        } PRIVATEWIDGETVIEW, *PPRIVATEWIDGETVIEW;

        /*
         *@@ XCENTERWINDATA:
         *      general view-specific data for the XCenter.
         *      This is stored in QWL_USER of both the
         *      XCenter frame (fnwpXCenterMainFrame) and
         *      the client (fnwpXCenterMainClient).
         *
         *      This is allocated via wpAllocMem in
         *      ctrpCreateXCenterView and destroyed on
         *      WM_DESTROY in fnwpXCenterMainClient.
         *
         *      This structure is private and not seen
         *      by the widgets. However, this contains
         *      the XCENTERGLOBALS member, whose pointer
         *      is passed to the widgets in each XCENTERWIDGET
         *      structure.
         */

        typedef struct _XCENTERWINDATA
        {
            USHORT              cbSize;             // size of struct (req. by PM)
            XCenter             *somSelf;           // XCenter instance

            // TID                 tidXCenter;         // thread ID of XCenter GUI thread
                    // removed V0.9.13 (2001-06-21) [umoeller], we have this in instance data
            HEV                 hevRunning;         // event sem posted once XCenter is created

            USEITEM             UseItem;            // use item; immediately followed by view item
            VIEWITEM            ViewItem;           // view item

            HSWITCH             hsw;                // switch entry of XCenter pseudo-frame
                                                    // V0.9.19 (2002-05-28) [umoeller]

            XCENTERGLOBALS      Globals;            // public data; a ptr to this is stored in
                                                    // each created XCENTERWIDGET

            LINKLIST            llWidgets;          // linked list of PPRIVATEWIDGETVIEW pointers;
                                                    // list is not auto-free (ctrpCreateXCenterView)

            PFNWP               pfnwpFrameOrig;     // original frame window proc (subclassed)

                    // NOTE: the view settings (border width, spacing) are in XCENTERGLOBALS.

            LONG                yFrame;             // current frame y pos
            ULONG               cxFrame,            // always screen width
                                cyFrame;            // XCenter frame height
                                                    // (even if hidden)

            HWND                hwndContextMenu;    // if != NULLHANDLE, a context menu is showing

            BOOL                fShowingSettingsDlg; // if TRUE, a widget settings dlg is showing

            ULONG               ulStartTime;        // for animation 1 (TIMERID_UNFOLDFRAME)
            ULONG               ulWidgetsShown;     // for animation 2 (TIMERID_SHOWWIDGETS)

            ULONG               idTimerAutohideStart; // if != 0, TIMERID_AUTOHIDE_START is running
            ULONG               idTimerAutohideRun; // if != 0, TIMERID_AUTOHIDE_RUN is running

            BOOL                fFrameFullyShown;   // FALSE while the "unfold" animation is still
                                                    // running; always TRUE afterwards.

            BOOL                fFrameAutoHidden;   // if TRUE, frame is currently auto-hidden;
                                                    // this changes frequently

            BOOL                fHasEmphasis;       // TRUE if emphasis has been added and client
                                                    // needs repaint V0.9.9 (2001-03-10) [umoeller]

            BOOL                fClickWatchRunning; // TRUE while XWPDAEMN.EXE is running a click watch
                                                    // for this XCenter V0.9.14 (2001-08-21) [umoeller]
        } XCENTERWINDATA, *PXCENTERWINDATA;

        /*
         *@@ WGTSETTINGSTEMP:
         *      temporaray structure for ctrpShowSettingsDlg.
         *      This is needed to store a bunch of temporary
         *      data while a settings dialog is open to
         *      allow ctrSetSetupString to work.
         *
         *@@added V0.9.7 (2000-12-07) [umoeller]
         */

        typedef struct _WGTSETTINGSTEMP
        {
            XCenter                 *somSelf;
                        // changed V0.9.11 (2001-04-25) [umoeller]

            PPRIVATEWIDGETSETTING   pSetting;
                        // ptr to internal settings list item

            PXCENTERWIDGET          pWidget;
                        // if != NULL, ptr to open widget view

            // ULONG                   ulIndex;
                        // index of the widget for which settings dlg is
                        // shown
        } WGTSETTINGSTEMP, *PWGTSETTINGSTEMP;
    #endif // LINKLIST_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Desktop workarea resizing
     *
     ********************************************************************/

    APIRET ctrpDesktopWorkareaSupported(VOID);

    /* ******************************************************************
     *
     *   Timer wrappers
     *
     ********************************************************************/

    USHORT APIENTRY tmrStartTimer(HWND hwnd,
                                  USHORT usTimerID,
                                  ULONG ulTimeout);

    BOOL APIENTRY tmrStopTimer(HWND hwnd,
                               USHORT usTimerID);

    /* ******************************************************************
     *
     *   Widget window formatting
     *
     ********************************************************************/

    #ifdef LINKLIST_HEADER_INCLUDED

        ULONG ctrpBroadcastWidgetNotify(PLINKLIST pllWidgets,
                                        USHORT usNotifyCode,
                                        MPARAM mp2);

        ULONG ctrpPositionWidgets(PXCENTERGLOBALS pGlobals,
                                  PLINKLIST pllWidgets,
                                  ULONG x,
                                  ULONG y,
                                  ULONG cxPerGreedy,
                                  ULONG cyAllWidgets,
                                  BOOL fShow);

        VOID ctrpGetWidgetSizes(PLINKLIST pllWidgets);

        ULONG ctrpQueryMaxWidgetCY(PLINKLIST pllWidgets);

        VOID ctrpReformat(PXCENTERWINDATA pXCenterData,
                          ULONG ulFlags);

    /* ******************************************************************
     *
     *   Settings dialogs
     *
     ********************************************************************/

        VOID ctrpShowSettingsDlg(XCenter *somSelf,
                                 HWND hwndOwner,
                                 PWIDGETPOSITION pPosition);

    /* ******************************************************************
     *
     *   Drag'n'drop
     *
     ********************************************************************/

        // define a new rendering mechanism, which only
        // our own container supports (this will make
        // sure that we can only do d'n'd within this
        // one container)
        #define WIDGET_DRAG_MECH "DRM_XCENTERWIDGET"
        #define WIDGET_DRAG_RMF \
            "(" WIDGET_DRAG_MECH ",DRM_OS2FILE,DRM_DISCARD)x(DRF_UNKNOWN)"

        VOID ctrpDrawEmphasis(PXCENTERWINDATA pXCenterData,
                              BOOL fRemove,
                              PPRIVATEWIDGETVIEW pWidget,
                              HWND hwnd,
                              HPS hpsPre);

        VOID ctrpRemoveDragoverEmphasis(HWND hwndClient);

        HWND ctrpDragWidget(HWND hwnd,
                            PXCENTERWIDGET pWidget);

        MRESULT ctrpDragOver(HWND hwndClient,
                             HWND hwndTrayWidget,
                             PDRAGINFO pdrgInfo);

        APIRET ctrpDrop(HWND hwndClient,
                        HWND hwndTrayWidget,
                        PDRAGINFO pdrgInfo);

        BOOL ctrpVerifyType(PDRAGITEM pdrgInfo,
                            PCSZ pcszType);

    /* ******************************************************************
     *
     *   Widget window creation
     *
     ********************************************************************/

        #define WPOPFL_NOTEBOOKMENU         0x0001
        #define WPOPFL_REMOVEWIDGETHELP     0x0002

        HWND ctrpLoadWidgetPopupMenu(HWND hwndOwner,
                                     PCXCENTERWIDGETCLASS pWidgetClass,
                                     ULONG fl);

        PPRIVATEWIDGETVIEW ctrpCreateWidgetWindow(PXCENTERWINDATA pXCenterData,
                                                  PXCENTERWIDGET pOwningTrayWidget,
                                                  PPRIVATEWIDGETSETTING pSetting,
                                                  ULONG ulIndex);

    #endif

    /* ******************************************************************
     *
     *   XCenter view implementation
     *
     ********************************************************************/

    APIRET ctrpQueryWidgetIndexFromHWND(XCenter *somSelf,
                                        HWND hwndWidget,
                                        PWIDGETPOSITION pPosition);

    APIRET ctrpCreateWidget(XCenter *somSelf,
                            PCSZ pcszWidgetClass,
                            PCSZ pcszSetupString,
                            ULONG ulTrayWidgetIndex,
                            ULONG ulTrayIndex,
                            ULONG ulWidgetIndex);

    BOOL ctrpMoveWidget(XCenter *somSelf,
                        ULONG ulIndex2Move,
                        ULONG ulBeforeIndex);

    BOOL ctrpModifyPopupMenu(XCenter *somSelf,
                            HWND hwndMenu);

    HWND ctrpCreateXCenterView(XCenter *somSelf,
                               HAB hab,
                               ULONG ulView,
                               PVOID *ppvOpenView);

    /* ******************************************************************
     *
     *   Private messages _to_ XCenter client
     *
     ********************************************************************/

    // WM_USER to WM_USER + 2 are used by shared\center.h

    /*
     *@@ XCM_CREATEWIDGETWINDOW:
     *      creates a widget window. This must be _sent_
     *      to the client... it's an internal message
     *      and may only be sent after the widget setting
     *      has been set up.
     *
     *      Parameters:
     *
     *      -- PPRIVATEWIDGETSETTING mp1: setting for
     *              widget to be created.
     *
     *      -- ULONG mp2: widget index.
     *
     *@@added V0.9.7 (2001-01-03) [umoeller]
     *@@changed V0.9.19 (2002-05-04) [umoeller]: msg renamed
     */

    #define XCM_CREATEWIDGETWINDOW      (WM_USER + 3)

    /*
     *@@ XCM_DESTROYWIDGETWINDOW:
     *      destroys a widget window. This is an internal
     *      message and must only be sent when a widget
     *      setting is being been destroyed.
     *
     *      What this does depends on where it is sent to:
     *
     *      --  If sent to the XCenter client, this simply
     *          calls WinDestroyWindow on the GUI thread.
     *          ctrDefWindowProc does additional cleanup
     *          on destroy then.
     *
     *      --  If sent to a tray widget, this calls
     *          WinDestroyWindow on the specified subwidget
     *          and cleans up the subwidget views list
     *          in fnwpTrayWidget.
     *
     *      This does _not_ reformat the client or the
     *      tray.
     *
     *      NEVER send this yourself.
     *      Use XCenter::xwpRemoveWidget instead.
     *
     *      Parameters:
     *
     *      -- HWND mp1: widget window to be destroyed.
     *
     *      -- mp2: reserved, must be 0.
     *
     *@@added V0.9.7 (2001-01-03) [umoeller]
     *@@changed V0.9.19 (2002-05-04) [umoeller]: msg renamed
     */

    #define XCM_DESTROYWIDGETWINDOW     (WM_USER + 4)

    /*
     * XCM_DELETESUBWIDGET:
     *      removes a subwidget from a tray and also
     *      destroys the window. This may only be sent
     *      to a tray widget.
     *
     *      Parameters:
     *
     *      -- PPRIVATEWIDGETVIEW mp1: widget to be destroyed.
     *
     *      -- mp2: reserved, must be 0.
     *
     *      Returns TRUE if the widget was found and destroyed.
     *
     *added V0.9.13 (2001-06-19) [umoeller]
     *changed V0.9.19 (2002-05-04) [umoeller]: msg removed
     */

    // #define XCM_DELETESUBWIDGET         (WM_USER + 5)

    /*
     *@@ XCM_SWITCHTOTRAY:
     *      switches to the specified tray.
     *      This may only be posted or sent to a tray widget.
     *
     *      Parameters:
     *
     *      -- ULONG mp1: tray index to switch to or -1
     *              for no tray.
     *
     *      -- mp2: reserved, must be 0.
     *
     *@@added V0.9.13 (2001-06-19) [umoeller]
     */

    #define XCM_SWITCHTOTRAY            (WM_USER + 6)

    /*
     * XCM_CREATEOBJECTBUTTON:
     *      sent to a tray widget by the engine's d'n'd
     *      code if an object button widget should be
     *      created in the current tray.
     *
     *      Parameters:
     *
     *      -- PSZ mp1: setup string for new object
     *         button.
     *
     *      -- ULONG mp2: index where to insert (-1
     *         for rightmost).
     *
     *added V0.9.13 (2001-06-23) [umoeller]
     *removed V0.9.19 (2002-05-04) [umoeller]
     */

    // #define XCM_CREATEOBJECTBUTTON      (WM_USER + 7)

    /*
     *@@ XCM_CREATESUBWIDGET:
     *      sent to a tray widget by the engine's d'n'd
     *      code if a widget should be created in the current
     *      tray.
     *
     *      Parameters:
     *
     *      -- PSZ mp1: extended setup string for new widget.
     *         It contains the widget class name, followed by
     *         a \r\n pair, and the (possibly empty) setup
     *         string.
     *
     *      -- ULONG mp2: index where to insert (-1
     *         for rightmost).
     *
     *@@added V0.9.14 (2001-07-31) [lafaix]
     */

    // #define XCM_CREATESUBWIDGET      (WM_USER + 8)

    /*
     *@@ XCM_MOUSECLICKED:
     *      private message used for notifications from
     *      XWPDAEMN.EXE for auto-hide and mouse clicks.
     *
     *      See XDM_ADDCLICKWATCH.
     *
     *@@added V0.9.14 (2001-08-21) [umoeller]
     */

    #define XCM_MOUSECLICKED         (WM_USER + 9)

    /*
     *@@ XCM_TRAYSCHANGED:
     *      private message posted to a tray
     *      widget after any tray settings have
     *      changed.
     *
     *      Parameters: none.
     *
     *@@added V0.9.19 (2002-04-25) [umoeller]
     */

    #define XCM_TRAYSCHANGED         (WM_USER + 10)

    /* ******************************************************************
     *
     *   XCenter setup set (ctr_notebook.c)
     *
     ********************************************************************/

    VOID ctrpInitData(XCenter *somSelf);

    #ifdef LINKLIST_HEADER_INCLUDED
    #ifdef XSTRING_HEADER_INCLUDED
        VOID ctrpAppendWidgetSettings(PXSTRING pstrSetup,
                                      PPRIVATEWIDGETSETTING pSetting,
                                      PBOOL pfFirstWidget,
                                      PXSTRING pstrTemp);
    #endif
    #endif

    BOOL ctrpQuerySetup(XCenter *somSelf,
                         PVOID pstrSetup);

    BOOL ctrpSetup(XCenter *somSelf,
                   PSZ pszSetupString);

    BOOL ctrpSetupOnce(XCenter *somSelf,
                       PSZ pszSetupString);

    BOOL ctrpRestoreState(XCenter *somSelf);

    BOOL ctrpSaveState(XCenter *somSelf);

    #define DRT_WIDGET "Widget settings"

    BOOL ctrpSaveToFile(PCSZ pszDest, PCSZ pszClass, PCSZ pszSetup);

    APIRET ctrpReadFromFile(PCSZ pszSource,
                            PSZ *ppszClass,
                            PSZ *ppszSetup);

    /* ******************************************************************
     *
     *   XCenter notebook callbacks (notebook.c)
     *
     ********************************************************************/

    #ifdef NOTEBOOK_HEADER_INCLUDED

        VOID XWPENTRY ctrpView1InitPage(PNOTEBOOKPAGE pnbp,
                                        ULONG flFlags);

        MRESULT XWPENTRY ctrpView1ItemChanged(PNOTEBOOKPAGE pnbp,
                                     ULONG ulItemID, USHORT usNotifyCode,
                                     ULONG ulExtra);

        VOID XWPENTRY ctrpView2InitPage(PNOTEBOOKPAGE pnbp,
                                        ULONG flFlags);

        MRESULT XWPENTRY ctrpView2ItemChanged(PNOTEBOOKPAGE pnbp,
                                     ULONG ulItemID, USHORT usNotifyCode,
                                     ULONG ulExtra);

        VOID XWPENTRY ctrpWidgetsInitPage(PNOTEBOOKPAGE pnbp,
                                          ULONG flFlags);

        MRESULT XWPENTRY ctrpWidgetsItemChanged(PNOTEBOOKPAGE pnbp,
                                       ULONG ulItemID, USHORT usNotifyCode,
                                       ULONG ulExtra);

        VOID XWPENTRY ctrpClassesInitPage(PNOTEBOOKPAGE pnbp,
                                          ULONG flFlags);

        MRESULT XWPENTRY ctrpClassesItemChanged(PNOTEBOOKPAGE pnbp,
                                       ULONG ulItemID, USHORT usNotifyCode,
                                       ULONG ulExtra);
    #endif

    /* ******************************************************************
     *
     *   Settings dialogs for built-in widgets
     *
     ********************************************************************/

    VOID EXPENTRY OwgtShowXButtonSettingsDlg(PWIDGETSETTINGSDLGDATA pData);
    VOID EXPENTRY PwgtShowSettingsDlg(PWIDGETSETTINGSDLGDATA pData);

#endif

