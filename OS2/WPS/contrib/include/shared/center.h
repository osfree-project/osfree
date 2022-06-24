
/*
 *@@sourcefile center.h:
 *      public header file for the XCenter and plugins.
 *
 *      This contains all declarations which are needed by
 *      all parts of the XCenter and to implement widget
 *      plugin DLLs.
 *
 *      WARNING: The XCenter is still work in progress. The
 *      definitions in this file are still subject to change,
 *      even though I try to maintain backwards compatibility.
 *
 *      If you are looking at this file from the "toolkit\shared"
 *      directory of a binary XWorkplace release, this is an
 *      exact copy of the file in "include\shared" from the
 *      XWorkplace sources.
 *
 *@@include #include "shared\center.h"
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller,
 *                              Martin Lafaix.
 *
 *      This file is part of the XWorkplace source package.
 *      Even though XWorkplace is free software under the GNU General
 *      Public License version 2 (GPL), permission is granted, free
 *      of charge, to use this file for the purpose of creating software
 *      that integrates into XWorkplace or eComStation, even if that
 *      software is not published under the GPL.
 *
 *      This permission  extends to this single file only, but NOT to
 *      other files of the XWorkplace sources, not even those that
 *      are based on or include this file.
 *
 *      As a special exception to the GPL, using this file for the
 *      purpose of creating software that integrates into XWorkplace
 *      or eComStation is not considered creating a derivative work
 *      in the sense of the GPL. In plain English, you are not bound
 *      to the terms and conditions of the GPL if you use only this
 *      file for that purpose. You are bound by the GPL however if
 *      you incorporate code from GPL'ed XWorkplace source files where
 *      this notice is not present.
 *
 *      This file is distributed in the hope that it will be useful,
 *      but still WITHOUT ANY WARRANTY; without even the implied
 *      warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *      In no event shall the authors and/or copyright holders be
 *      liable for any damages or other claims arising from the use
 *      of this software.
 */

#ifndef CENTER_HEADER_INCLUDED
    #define CENTER_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Public definitions
     *
     ********************************************************************/

    // button types (src\xcenter\w_objbutton.c)
    #define BTF_OBJBUTTON       1
    #define BTF_XBUTTON         2

    /*
     *@@ XCENTERGLOBALS:
     *      global data for a running XCenter instance.
     *      This is created once per open XCenter.
     *
     *      A pointer to this one structure exists in each
     *      XCENTERWIDGET instance so that the widgets
     *      can access some of the data of the XCenter
     *      they belong to.
     *
     *      In this structure, an XCenter instance
     *      passes a number of variables to its member
     *      widgets so that they can access some things
     *      more quickly for convenience. This structure
     *      is fully initialized at the time the widgets
     *      are created, but values may change while the
     *      XCenter is open (e.g. if the user changes
     *      view settings in the XCenter settings notebook).
     *
     *      "Globals" isn't really a good name since
     *      one of these structures is created for
     *      each open XCenter (and there can be several),
     *      but this is definitely more global than the
     *      widget-specific structures.
     */

    typedef struct _XCENTERGLOBALS
    {
        HAB                 hab;
                    // anchor block of the XCenter thread and thus
                    // all XCenter windows (including widgets)
        HWND                hwndFrame,
                    // be XCenter frame window (constant)
                            hwndClient;
                    // XCenter client (child of XCenter frame) (constant)

        PVOID               pCountrySettings;
                    // country settings; this really points to a
                    // COUNTRYSETTINGS structure (include\helpers\prfh.h)
                    // and might be useful if you want to use settings
                    // from the "Country" object (constant)

        ULONG               cyInnerClient;
                    // height of inner client (same as height of all widgets!).
                    // This is normally == cyWidgetmax unless the user has
                    // resized the XCenter, but it will always be >= cyWidgetMax.
                    // The "inner client" plus the 3D border width plus the
                    // border spacing make up the full height of the XCenter.

                    // This can change while the XCenter is open.
                    // This was changed with V0.9.9 (2001-03-09) [umoeller];
                    // this field still always has the client height, but this
                    // is no longer neccessarily the same as the tallest
                    // widget (see cyWidgetMax below).

        ULONG               cxMiniIcon;
                    // system mini-icon size (for convenience); either 16 or 20
                    // (constant)

        LONG                lcol3DDark,
                            lcol3DLight;
                    // system colors for 3D frames (for convenience; RGB!) (constant)

        // the following are the width settings from the second "View"
        // settings page;
        // a widget may or may not want to consider these.
        ULONG               flDisplayStyle;
                    // XCenter display style flags;
                    // a widget may or may not want to consider these.
                    // Can be changed by the user while the XCenter is open.
                    // These flags can be any combination of the following:

                    #define XCS_FLATBUTTONS         0x0001
                    // -- XCS_FLATBUTTONS: paint buttons flat. If not set,
                    //      paint them raised.

                    #define XCS_SUNKBORDERS         0x0002
                    // -- XCS_SUNKBORDERS: paint static controls (e.g. CPU meter)
                    //      with a "sunk" 3D frame. If not set, do not.

                    #define XCS_SIZINGBARS          0x0004
                    // -- XCS_SIZINGBARS: XCenter should automatically paint
                    //      sizing bars for sizeable widgets.

                    #define XCS_ALL3DBORDERS        0x0008
                    // -- XCS_ALL3DBORDERS: XCenter should draw all four 3D
                    //      borders around itself. If not set, it will only
                    //      draw one border (the one towards the screen).

                    #define XCS_SPACINGLINES        0x0010      // added V0.9.13 (2001-06-19) [umoeller]
                    // -- XCS_SPACINGLINES: XCenter should draw small 3D lines
                    //      between the widgets too (V0.9.13 (2001-06-19) [umoeller]).

                    #define XCS_NOHATCHINUSE        0x0020      // added V0.9.16 (2001-10-24) [umoeller]
                    // -- XCS_NOHATCHINUSE: XCenter should not add hatching to
                    //      object widgets which represent open objects
                    //      (V0.9.16 (2001-10-24) [umoeller])

        ULONG               ulPosition;
                    // XCenter position on screen, if a widget cares...
                    // can be changed by the user while the XCenter is open.
                    // This is _one_ of the following:
                    #define XCENTER_BOTTOM          0
                    #define XCENTER_TOP             1
                    // Left and right are not yet supported.

        ULONG               ul3DBorderWidth;
                    // 3D border width; can be changed by user at any time
        ULONG               ulBorderSpacing;
                    // border spacing (added to 3D border width);
                    // can be changed by user at any time
        ULONG               ulWidgetSpacing;
                    // spacing between widgets; can be changed by user at any time;
                    // if flDisplayStyle also has XCS_SPACINGLINES set,
                    // an extra 2 pixels will be added internally to this
                    // value

        /*
         *      The following fields have been added with
         *      releases later than V0.9.7. Since they have
         *      been added at the bottom, the structure is
         *      still backward-compatible with old plugin
         *      binaries... but you cannot use these new
         *      fields unless you also require the corresponding
         *      XCenter revision by using the "version" export @3.
         */

        PVOID               pvXTimerSet;
                    // XCenter timer set, which can be used by widgets
                    // to start XTimers instead of regular PM timers.
                    // This was added V0.9.9 (2001-03-07) [umoeller].
                    // See src\helpers\timer.c for an introduction.
                    // This pointer is really an XTIMERSET pointer but
                    // has been declared as PVOID to avoid having to
                    // #include include\helpers\timer.h all the time.
                    // This is constant while the XCenter is open.

        ULONG               cyWidgetMax;
                    // height of tallest widget == minimum height of client
                    // V0.9.9 (2001-03-09) [umoeller]
                    // This may change while the XCenter is open if
                    // widgets are added or removed.

        LONG                lcolClientBackground;
                    // background color of the XCenter client area,
                    // in case a widget is interested.
                    // V0.9.13 (2001-06-19) [umoeller]

        PVOID               pvSomSelf;
                    // XCenter's somSelf object pointer. This is a
                    // XCenter* pointer really, but has been declared
                    // as PVOID so you won't have to include the WPS
                    // SOM headers.
                    // V0.9.13 (2001-06-19) [umoeller]

        HWND                hwndTooltip;
                    // XCenter's tooltip control (comctl.c). This is
                    // the one tooltip that is created for all widgets.
                    // This member has been made public so widgets may
                    // choose to work with the tooltip... but do not
                    // mess with this unless you know what you're doing,
                    // since all widgets share this one tooltip.
                    // V0.9.13 (2001-06-21) [umoeller]

    } XCENTERGLOBALS, *PXCENTERGLOBALS;

    typedef const struct _XCENTERGLOBALS *PCXCENTERGLOBALS;

    // forward declaration
    typedef struct _XCENTERWIDGET *PXCENTERWIDGET;

    BOOL APIENTRY ctrSetSetupString(LHANDLE hSetting, PCSZ pcszNewSetupString);
    typedef BOOL APIENTRY CTRSETSETUPSTRING(LHANDLE hSetting, PCSZ pcszNewSetupString);
    typedef CTRSETSETUPSTRING *PCTRSETSETUPSTRING;

    /*
     *@@ WIDGETSETTINGSDLGDATA:
     *      a bunch of data passed to a "show settings
     *      dialog" function (WGTSHOWSETTINGSDLG), if
     *      the widget has specified such a thing.
     *
     *      XCenter widget settings dialogs basically
     *      work as follows:
     *
     *      1.  You must write a function that displays a modal
     *          dialog. This function must have the following
     *          prototype:
     *
     +              VOID EXPENTRY ShowSettingsDlg(PWIDGETSETTINGSDLGDATA pData)
     *
     *      2.  In the "init module" export, store that function
     *          in the XCENTERWIDGETCLASS.pShowSettingsDlg field
     *          for your widget class.
     *
     *          This will enable the "Properties" menu item
     *          for the widget.
     *
     *      3.  Your function gets called when the user selects
     *          "Properties". What that function does, doesn't
     *          matter... it should however display a modal
     *          dialog and update the widget's settings string
     *          and call ctrSetSetupString with the "hSettings"
     *          handle that was given to it in the
     *          WIDGETSETTINGSDLGDATA structure. This will give
     *          the widget the new settings.
     *
     *          The address of the ctrSetSetupString helper is
     *          given to you in this structure so that you
     *          won't have to import it from XFLDR.DLL.
     *
     *      If a widget class supports settings dialogs,
     *      it must specify this in its XCENTERWIDGETCLASS.
     *
     *@@added V0.9.7 (2000-12-07) [umoeller]
     *@@changed V0.9.9 (2001-02-06) [umoeller]: added pctrSetSetupString
     */

    typedef struct _WIDGETSETTINGSDLGDATA
    {
        HWND                    hwndOwner;
                    // proposed owner for settings dialog; this
                    // will either be the HWND of the XCenter frame
                    // or some other HWND, if the settings dialog
                    // is displayed from elsewhere

        const char              *pcszSetupString;
                    // present setup string (do not change)

        const LHANDLE           hSettings;
                    // widget setting handle; this must
                    // be passed to ctrSetSetupString;
                    // DO NOT CHANGE

        PCXCENTERGLOBALS        pGlobals;
                    // if != NULL, currently open XCenter
                    // for which widget data is being changed.
                    // If NULL, the XCenter isn't currently
                    // open.

        PXCENTERWIDGET          pView;
                    // if != NULL, currently open view
                    // of the widget. If NULL, the widget
                    // isn't currently open.

        PVOID                   pUser;
                    // some room for additional data the
                    // settings dialog might want

        /*
         *      The following fields have been added with
         *      releases later than V0.9.7. Since they have
         *      been added at the bottom, the structure is
         *      still backward-compatible with old plugin
         *      binaries... but you cannot use these new
         *      fields unless you also require the corresponding
         *      XCenter revision by using the "version" export @3.
         */

        PCTRSETSETUPSTRING      pctrSetSetupString;
                    // ptr to ctrSetSetupString function in
                    // src\shared\center.c; this must be
                    // called by the settings dialog to
                    // change the widget's setup string.
                    // This pointer has been added with V0.9.9
                    // to allow using settings dialog without
                    // having to import this from XFLDR.DLL.

    } WIDGETSETTINGSDLGDATA, *PWIDGETSETTINGSDLGDATA;

    typedef VOID EXPENTRY WGTSHOWSETTINGSDLG(PWIDGETSETTINGSDLGDATA);
    typedef WGTSHOWSETTINGSDLG *PWGTSHOWSETTINGSDLG;

    /*
     *@@ XCENTERWIDGETCLASS:
     *      describes one widget class which can be
     *      used in an XCenter. Such a class can either
     *      be internal or in a widget plugin DLL.
     *
     *      This structure _must_ have the same layout as
     *      PLUGINCLASS.  It can redefine the ulUser* fields
     *      to its liking, though.
     *
     *      The window procedure which implements the widget
     *      class must be registered by a plug-in DLL when
     *      its INIT callback gets called by the XCenter.
     *
     *      There are a few rules which must be followed
     *      with the window procedures of the widget classes:
     *
     *      -- At the very least, the widget's window proc must
     *         implement WM_CREATE, WM_PAINT, and (if cleanup
     *         is necessary) WM_DESTROY. WM_PRESPARAMCHANGED
     *         would also be nice to support fonts and colors
     *         dropped on the widget.
     *
     *      -- On WM_CREATE, the widget receives a pointer to
     *         its XCENTERWIDGET structure in mp1.
     *
     *         The first thing the widget _must_ do on WM_CREATE
     *         is to store the XCENTERWIDGET pointer (from mp1)
     *         in its QWL_USER window word by calling:
     *
     +              WinSetWindowPtr(hwnd, QWL_USER, mp1);
     *
     *      -- The XCenter communicates with the widget using
     *         WM_CONTROL messages. SHORT1FROMMP(mp1), the
     *         source window ID, is always ID_XCENTER_CLIENT.
     *         SHORT2FROMMP(mp1), the notification code, can
     *         be:
     *
     *         --  XN_QUERYSIZE: the XCenter wants to know the
     *             widget's size.
     *
     *         --  XN_SETUPCHANGED: widget's setup string has
     *             changed.
     *
     *         --  XN_BEGINANIMATE: the widget is about to be
     *             shown/hidden.
     *
     *         --  XN_ENDANIMATE: the widget is now fully shown/hidden.
     *
     *         --  XN_HITTEST:  for transparent widgets, the XCenter
     *             wants to know if the specified location is covered.
     *
     *      -- All unprocessed notifications must be routed
     *         to ctrDefWidgetProc instead of WinDefWindowProc.
     *         Do not swallow unprocessed messages.
     *
     *      -- You must never use WinSetWindowPos on your widget
     *         window yourself because this will solidly confuse
     *         the XCenter widget formatting. If you want your
     *         widget window to be resized, post XCM_SETWIDGETSIZE
     *         to the widget's parent instead, which will resize
     *         your widget properly and also reformat the other
     *         widgets, if necessary.
     *
     *      -- All unprocessed messages should be routed
     *         to ctrDefWidgetProc instead of WinDefWindowProc.
     *
     *      -- WM_MENUEND must _always_ be passed on after your
     *         own processing (if any) to remove source emphasis
     *         for popup menus.
     *
     *      -- WM_DESTROY must _always_ be passed on after
     *         your own cleanup code to avoid memory leaks,
     *         because this function performs important cleanup
     *         as well.
     *
     *      -- WM_COMMAND command values above 0x7f00 are reserved.
     *         If you extend the context menu given to you in
     *         XCENTERWIDGET.hwndContextMenu, you must use
     *         menu item IDs < 0x7f00. This has been changed
     *         with V0.9.11 (2001-04-25) [umoeller] to avoid
     *         conflicts with the WPS menu item IDs, sorry.
     *
     *      <B>Resource management</B>
     *
     *      You should always be aware that your widget is a PM
     *      window in the WPS process. As a result, all resource
     *      leaks that you produce in your widget code will affect
     *      the entire WPS, and all crashes will too.
     *
     *      As a general rule, the XCenter will free all widget
     *      resources that it has allocated itself (such as the
     *      structures that were created for it). By contrast,
     *      if you allocate something yourself, it is your
     *      responsibility to free it. This applies especially
     *      to XCENTERWIDGET.pUser, which is your general pointer
     *      to store widget-specific things.
     *
     *      Of course the same rules apply to PM resources such
     *      as icons, bitmaps, regions, memory PS's etc. You can
     *      quickly bring down the WPS if you forget to free those.
     *      Same thing if you start additional threads in your
     *      widget code.
     *
     *      This is especially important because widget windows
     *      can be created and destroyed without the widgets
     *      cooperation. For example, if the widget is in a tray,
     *      it will be created and destroyed when the current
     *      tray changes. If you leak something in this situation,
     *      this can quickly become a major problem.
     *
     *      I also recommend to use exception handling in your
     *      widget code to properly clean up things if something
     *      goes wrong. As said in center.c, do not use exit
     *      lists because that will cause DLL unloading to fail.
     *
     *      <B>Tooltip support</B>
     *
     *      If your widget class has the WGTF_TOOLTIP flag set,
     *      it will receive WM_CONTROL with TTN_* notifications
     *      and the source window ID set to ID_XCENTER_TOOLTIP.
     *      If it responds to TTN_NEEDTEXT, it must fill the
     *      TOOLTIPTEXT pointed to by mp2. See include\helpers\comctl.h
     *      for details.
     *
     *      In addition, the widget will also receive the TTN_SHOW
     *      and TTN_POP notifications when the tooltip is being
     *      shown or hidden above it.
     *
     *      <B>Trayable widgets</B>
     *
     *      If your widget is supposed to be trayable (i.e.
     *      the WGTF_TRAYABLE flag is set in the XCENTERWIDGETCLASS),
     *      it must conform to a few extra rules:
     *
     *      -- Trayable widgets cannot be sizeable or greedy
     *         because the tray widget itself is sizeable.
     *
     *      -- You must never assume that the parent window of
     *         your widget really is the XCenter client. It can
     *         also be a tray widget. Always use
     *         WinQueryWindow(hwndWidget, QW_PARENT) to find your
     *         parent window, which will properly give you either
     *         the XCenter client or the tray widget.
     *
     *      -- If you use XCM_* messages, never post them to the
     *         XCenter client window (whose handle is available
     *         thru XCENTERGLOBALS). All XCM_* essages are understood
     *         by the tray widget also, and if your widget is in a tray,
     *         it must post them to the tray instead of the client.
     *
     *      -- Your widget must be prepared for being created or
     *         destroyed at times other than XCenter creation or
     *         destruction because the tray widget will do this
     *         when the current tray is switched.
     */

    typedef struct _XCENTERWIDGETCLASS
    {
        const char      *pcszPMClass;
                // PM window class name of this widget class (can be shared among
                // several widget classes). A plugin DLL is responsible for
                // registering this window class when it's loaded.
                // Naturally, this must be unique on the system.

        ULONG           ulExtra;
                // additional identifiers the class might need if the
                // same PM window class is used for several widget classes.
                // This is not used by the XCenter, but you can access it
                // during WM_CREATE so you can differentiate between several
                // widget classes in the same window proc. You must copy this
                // to your private widget data then.
                // For example, MONITORS.DLL uses this to differentiate
                // between the various monitor widget classes.

        const char      *pcszWidgetClass;
                // internal widget class name; this is used to identify
                // the class. This must be unique on the system and must
                // not contain special characters like commas, brackets,
                // etc. Besides, since this must work with all codepages,
                // use only ASCII characters <= 127, and use an English
                // name always.
                // A valid name would be "MySampleClass".
                // This is stored internally in the XCenter data and is
                // used whenever the XCenter is opened to create all the
                // widgets from the respective widget classes. In other
                // words, this is the class identifier and must never
                // change between several releases or different NLS versions
                // of your plugin DLL, or otherwise widget creation will fail.
                // Choose this name carefully.

        const char      *pcszClassTitle;
                // explanatory widget class title, which is shown to the
                // user in the "Add widget" popup menu. Example: "Sample widget".
                // This is not used to identify the class internally and may
                // change between releases and NLS versions. So you can set
                // this to a language-specific string.
                // Note: If this has the highest bit set, it is assumed to
                // be a string resource ID in the XWorkplace NLS DLL instead
                // of a real string:
                #define XCENTER_STRING_RESOURCE     0x80000000
                            // V0.9.19 (2002-05-07) [umoeller]

        ULONG           ulClassFlags;
                // WGTF_* flags; any combination of the following:

                #define WGTF_SIZEABLE               0x0001
                // -- WGTF_SIZEABLE: widget window can be resized with
                //    the mouse by the user. A sizing bar is automatically
                //    painted by the XCenter engine then.

                #define WGTF_NOUSERCREATE           0x0002
                // -- WGTF_NOUSERCREATE: do not show this class in
                //    the "add widget" menu, and do not allow creating
                //    instances of this in the XCenter settings notebook.
                //    This is used for the object button widget, which
                //    can only be created through drag'n'drop. This flag
                //    isn't terribly useful for plug-in DLLs because
                //    without support in the XCenter engine the widget
                //    could then not be created at all.

                #define WGTF_UNIQUEPERXCENTER       0x0004
                // -- WGTF_UNIQUEPERXCENTER: only one widget of this class
                //    should be created per XCenter.

                #define WGTF_UNIQUEGLOBAL          (0x0008 + 0x0004)
                // -- WGTF_UNIQUEGLOBAL: only one widget of this class
                //    should be created in all XCenters on the system.
                //    This implies WGTF_UNIQUEPERXCENTER.

                #define WGTF_TOOLTIP                0x0010
                // -- WGTF_TOOLTIP: if set, the widget has a tooltip
                //    and will receive WM_CONTROL messages with the
                //    TTN_NEEDTEXT, TTN_SHOW, or TTN_POP notification codes
                //    (see helpers\comctl.h).
                //    The window ID of the tooltip control is ID_XCENTER_TOOLTIP.
                //    See XCENTERWIDGETCLASS for tooltip handling.

                #define WGTF_TOOLTIP_AT_MOUSE      (0x0020 + 0x0010)
                // -- WGTF_TOOLTIP_AT_MOUSE: like WGTF_TOOPTIP, but the
                //    tooltip is not centered above the widget, but put
                //    at the mouse position instead.
                //    This implies WGTF_TOOLTIP.

                #define WGTF_TRANSPARENT            0x0040
                // -- WGTF_TRANSPARENT: some parts of the widget window
                //    are transparent.  The widget will receive WM_CONTROL
                //    messages with the XN_HITTEST notification code.
                //    If the widget returns FALSE to the notification,
                //    the action will be forwarded to its parent (the
                //    XCenter client or a tray widget).

                #define WGTF_NONFOCUSTRAVERSABLE    0x0100
                //    Reserved for future use.

                #define WGTF_TRAYABLE               0x0200
                // -- WGTF_TRAYABLE: widget is "trayable", that is, it
                //    supports being created inside a tray widget.
                //    Note: Restrictions apply if you want your widget
                //    to be trayable. See above.

                #define WGTF_CANTAKECENTERHOTKEY    0x0400
                // -- WGTF_CANTAKECENTERHOTKEY: signals that the widget is
                //    willing to take action when the XCenter gets
                //    resurfaced via its object hotkey.
                //    The widget will then receive WM_CONTROL with
                //    the XN_CENTERHOTKEYPRESSED code; see remarks for
                //    XN_CENTERHOTKEYPRESSED.
                //    V0.9.19 (2002-04-17) [umoeller]

                #define WGTF_CONFIRMREMOVE          0x0800
                // -- WGTF_CONFIRMREMOVE: adds "..." to the "Remove widget"
                //    context menu item and confirms removal if the user
                //    selects that.
                //    V0.9.20 (2002-08-08) [umoeller]

        PWGTSHOWSETTINGSDLG pShowSettingsDlg;
                // if the widget supports a settings dialog,
                // it must set this func pointer to a procedure
                // that will show that dialog. If this is != NULL,
                // the "Properties" menu item and the button in
                // the widgets list of the XCenter settings notebook
                // will be enabled. See WIDGETSETTINGSDLGDATA for
                // details about how to implement widget settings dialogs.

    } XCENTERWIDGETCLASS, *PXCENTERWIDGETCLASS;

    typedef const struct _XCENTERWIDGETCLASS *PCXCENTERWIDGETCLASS;

    /*
     *@@ XCENTERWIDGETSETTING:
     *      describes one widget to be created. One instance
     *      of this is created for each widget that the
     *      user has configured for a widget.
     *
     *      An array of these is stored in packed form in the
     *      XCenter instance data and describes all widgets
     *      in the XCenter. Widget classes normally don't need
     *      this.
     *
     *      An array of these is returned by XCenter::xwpQueryWidgets.
     */

    typedef struct _XCENTERWIDGETSETTING
    {
        PSZ             pszWidgetClass;   // widget class name;
                    // we cannot use the binary PXCENTERWIDGETCLASS pointer
                    // here because these structures are dynamically unloaded...
                    // use ctrpFindClass to find the XCENTERWIDGETCLASS ptr.
        PSZ             pszSetupString;
                    // widget-class-specific setup string; can be NULL

            // Note: both pointers are assumed to be allocated
            // using malloc() or strdup() and are automatically
            // freed.

    } XCENTERWIDGETSETTING, *PXCENTERWIDGETSETTING;

    /*
     *@@ XCENTERWIDGET:
     *      public structure to hold data for an XCenter
     *      widget, i.e. a PM window in the XCenter client
     *      area.
     *
     *      Each XCenter widget is a separate window,
     *      being a child window of the XCenter client,
     *      which in turn is a child of the XCenter frame.
     *      The exception is a widget in a tray, which is
     *      a child of a tray widget, which in turn is a
     *      child of the XCenter client.
     *
     *      This structure is created once for each widget
     *      and passed to each widget in mp1 of WM_CREATE.
     *      The first thing a widget must do on WM_CREATE
     *      is to store a pointer to this structure in its
     *      QWL_USER window word.
     *
     *      The widget can allocate another widget-specific
     *      buffer and store its pointer into the pUser
     *      field. This is strongly recommended because
     *      otherwise your widget will have no place to
     *      store its instance data.
     *
     *      This structure is automatically freed by the
     *      XCenter when the widget is destroyed. However,
     *      if you allocated more memory for pUser, it is
     *      your own responsibility to free that on WM_DESTROY.
     *      General rule: The widget must clean up what it
     *      allocated itself.
     *
     *      See XCENTERWIDGETCLASS for rules that a widget
     *      window proc must conform to.
     */

    typedef struct _XCENTERWIDGET
    {
        /*
         *  Informational fields:
         *      all these are meant for reading only
         *      and set up by the XCenter for the widget.
         */

        HWND        hwndWidget;
                // window handle of this widget; this is valid
                // only _after_ WM_CREATE.

        HAB         habWidget;
                // widget's anchor block (copied for convenience).

        PFNWP       pfnwpDefWidgetProc;
                // address of default widget window procedure. This
                // always points to ctrDefWidgetProc (in
                // src/shared/center.c). The widget's own window
                // proc must pass all unprocessed messages (and a
                // few more) to this instead of WinDefWindowProc.
                // See XCENTERWIDGETCLASS for details.

        PCXCENTERGLOBALS pGlobals;
                // ptr to client/frame window data (do not change)

        PCXCENTERWIDGETCLASS pWidgetClass;
                // widget class this widget was created from;
                // this ptr is only valid during WM_CREATE and
                // always NULL afterwards

        PCSZ        pcszWidgetClass;
                // internal name of the widget's class; this
                // is the same as pWidgetClass->pcszWidgetClass,
                // but this is valid after WM_CREATE.
                // DO NOT CHANGE it, the XCenter relies on this!
        ULONG       ulClassFlags;
                // class flags copied from XCENTERWIDGETCLASS.

        PCSZ        pcszSetupString;
                // class-specific setup string. This field
                // is only valid during WM_CREATE and then holds
                // the setup string which was last stored with
                // the widget. The pointer can be NULL if no
                // setup string exists. After WM_CREATE, the
                // pointer is set to NULL always.
                // The widget should use this string to initialize
                // itself. When its instance data changes, it
                // should send XCM_SAVESETUP to its parent
                // so that the XCenter can record the new setup
                // string.

        /*
         *  Setup fields:
         *      all these should be set up by the widget
         *      (normally during WM_CREATE). The XCenter
         *      reads these fields to find out more about
         *      the widget's wishes.
         *
         *      All these fields are initialized to safe
         *      defaults, which are probably not suitable
         *      for most widgets though.
         */

        BOOL        fSizeable;
                // if TRUE, the widget is sizeable with the mouse.
                // This is initially set to TRUE if the widget's
                // class has the WGTF_SIZEABLE flag, but can be
                // changed by the widget at any time.
                // NOTE: If your widget is "greedy", i.e. wants
                // all remaining size on the XCenter bar (by
                // returning -1 for cx with WM_CONTROL and
                // XCN_QUERYSIZE), it cannot be sizeable.

        HWND        hwndContextMenu;
                // default context menu for this widget.
                // The XCenter loads a the same standard context menu
                // for each widget, which is displayed on WM_CONTEXTMENU
                // in ctrDefWidgetProc. Normally, the widget needs not
                // do anything to have the context menu displayed;
                // ctrDefWidgetProc takes care of this. However, you may
                // safely manipulate this context menu if you want to
                // insert/remove items.
                // The menu in here gets destroyed using WinDestroyWindow
                // when the widget window is destroyed.
                // This is only valid after WM_CREATE.

        PCSZ        pcszHelpLibrary;
        ULONG       ulHelpPanelID;
                // if these two are specified, the XCenter will
                // enable the "Help" item in the widget's context
                // menu and display the specified help panel when
                // the menu item is selected. Both default to NULL.

        /*
         *  Additional fields:
         */

        PVOID       pUser;
                // user data allocated by window class; this is
                // initially NULL, but you can use this for your
                // own data (which you must clean up yourself on
                // WM_DESTROY).

        PVOID       pvWidgetSetting;
                // private pointer for tray widget. DO NOT USE THIS.
                // V0.9.13 (2001-06-21) [umoeller]

    } XCENTERWIDGET;

    /* ******************************************************************
     *
     *   Public widget helpers (exported from XFLDR.DLL)
     *
     ********************************************************************/

    // All these are specified as both simple prototypes and
    // function pointer declarations so that widget classes
    // can import these functions from XFLDR.DLL in the init
    // export.

    PSZ APIENTRY ctrScanSetupString(PCSZ pcszSetupString,
                                    PCSZ pcszKeyword);
    typedef PSZ APIENTRY CTRSCANSETUPSTRING(PCSZ pcszSetupString,
                                            PCSZ pcszKeyword);
    typedef CTRSCANSETUPSTRING *PCTRSCANSETUPSTRING;

    LONG APIENTRY ctrParseColorString(PCSZ p);
    typedef LONG APIENTRY CTRPARSECOLORSTRING(PCSZ p);
    typedef CTRPARSECOLORSTRING *PCTRPARSECOLORSTRING;

    VOID APIENTRY ctrFreeSetupValue(PSZ p);
    typedef VOID APIENTRY CTRFREESETUPVALUE(PSZ p);
    typedef CTRFREESETUPVALUE *PCTRFREESETUPVALUE;

    // ctrSetSetupString has been defined above

    BOOL APIENTRY ctrDisplayHelp(PCXCENTERGLOBALS pGlobals,
                                 PCSZ pcszHelpFile,
                                 ULONG ulHelpPanelID);
    typedef BOOL APIENTRY CTRDISPLAYHELP(PCXCENTERGLOBALS pGlobals,
                                         PCSZ pcszHelpFile,
                                         ULONG ulHelpPanelID);
    typedef CTRDISPLAYHELP *PCTRDISPLAYHELP;

    VOID APIENTRY ctrShowContextMenu(PXCENTERWIDGET pWidget, HWND hwndContextMenu);
    typedef VOID APIENTRY CTRSHOWCONTEXTMENU(PXCENTERWIDGET pWidget, HWND hwndContextMenu);
    typedef CTRSHOWCONTEXTMENU *PCTRSHOWCONTEXTMENU;

    VOID APIENTRY ctrPaintStaticWidgetBorder(HPS hps, PXCENTERWIDGET pWidget);
    typedef VOID APIENTRY CTRPAINTSTATICWIDGETBORDER(HPS hps, PXCENTERWIDGET pWidget);
    typedef CTRPAINTSTATICWIDGETBORDER *PCTRPAINTSTATICWIDGETBORDER;

    VOID APIENTRY ctrDrawWidgetEmphasis(PXCENTERWIDGET pWidget, BOOL fRemove);
    typedef VOID APIENTRY CTRDRAWWIDGETEMPHASIS(PXCENTERWIDGET pWidget, BOOL fRemove);
    typedef CTRDRAWWIDGETEMPHASIS *PCTRDRAWWIDGETEMPHASIS;

    VOID APIENTRY ctrPlaceAndPopupMenu(HWND hwndOwner, HWND hwndMenu, BOOL fAbove);
    typedef VOID APIENTRY CTRPLACEANDPOPUPMENU(HWND hwndOwner, HWND hwndMenu, BOOL fAbove);
    typedef CTRPLACEANDPOPUPMENU *PCTRPLACEANDPOPUPMENU;

    MRESULT EXPENTRY ctrDefWidgetProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
                // a pointer to this is in XCENTERWIDGET if the widget
                // is a non-container widget
    typedef MRESULT EXPENTRY CTRDEFWIDGETPROC(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
    typedef CTRDEFWIDGETPROC *PCTRDEFWIDGETPROC;

    /* ******************************************************************
     *
     *   Open XCenter views
     *
     ********************************************************************/

    BOOL APIENTRY ctrIsXCenterView(HWND hwndFrame);
    typedef BOOL APIENTRY CTRISXCENTERVIEW(HWND hwndFrame);
    typedef CTRISXCENTERVIEW *PCTRISXCENTERVIEW;
                // this function was added with V0.9.16

    /* ******************************************************************
     *
     *   WM_CONTROL notification codes _from_ XCenter client
     *
     ********************************************************************/

    #define ID_XCENTER_CLIENT           7000
    #define ID_XCENTER_TOOLTIP          7001

    /*
     *@@ XN_QUERYSIZE:
     *      notification code for WM_CONTROL sent from
     *      the XCenter to a widget when it needs to
     *      know its desired size. This comes in once
     *      after WM_CREATE and may come in again later
     *      if the user changes XCenter view settings.
     *
     *      Parameters:
     *
     *      -- SHORT1FROMMP(mp1): ID, always ID_XCENTER_CLIENT.
     *
     *      -- SHORT2FROMMP(mp1): notify code (XN_QUERYSIZE).
     *
     *      -- PSIZEL mp2: pointer to a SIZEL structure in which
     *                     the widget must fill in its desired
     *                     size.
     *
     *      The widget must return TRUE if it has put its
     *      desired size into the SIZEL structure. Otherwise
     *      the XCenter will assume some dumb default for
     *      the widget size.
     *
     *      As a special case, a widget can put -1 into the
     *      cx field of the SIZEL structure. It will then
     *      receive all remaining space on the XCenter bar
     *      ("greedy" widgets). This is what the window list
     *      widget does, for example.
     *
     *      If several widgets request to be "greedy", the
     *      remaining space on the XCenter bar is evenly
     *      distributed among the greedy widgets.
     *
     *      After all widgets have been created, the XCenter
     *      (and all widgets) are resized to have the largest
     *      cy requested. As a result, your window proc cannot
     *      assume that it will always have the size it
     *      requested.
     *
     *@@added V0.9.7 (2000-12-14) [umoeller]
     */

    #define XN_QUERYSIZE                1

    /*
     *@@ XN_SETUPCHANGED:
     *      notification code for WM_CONTROL sent from
     *      the XCenter to a widget when its setup string
     *      has changed.
     *
     *      Parameters:
     *
     *      -- SHORT1FROMMP(mp1): ID, always ID_XCENTER_CLIENT.
     *
     *      -- SHORT2FROMMP(mp1): notify code (XN_SETUPCHANGED).
     *
     *      -- const char* mp2: pointer to a new zero-termianted
     *                          setup string.
     *
     *      The widget must return TRUE if it has processed
     *      the setup string successfully.
     *
     *      This gets sent to an open widget when
     *      ctrSetSetupString has been invoked on it to allow
     *      it to update its display. This normally happens
     *      when its settings dialog saves a new setup string.
     *
     *@@added V0.9.7 (2000-12-13) [umoeller]
     */

    #define XN_SETUPCHANGED             2

    /*
     *@@ XN_OBJECTDESTROYED:
     *      notification code for WM_CONTROL posted (!)
     *      to a widget if it has registered itself with
     *      XFldObject::xwpSetWidgetNotify.
     *
     *      This is a last chance for the widget to clean
     *      up itself when an object that it relies on gets
     *      destroyed (deleted or made dormant). This message
     *      is posted from XFldObject::wpUnInitData.
     *
     *      Parameters:
     *
     *      -- SHORT1FROMMP(mp1): ID, always ID_XCENTER_CLIENT.
     *
     *      -- SHORT2FROMMP(mp1): notify code (XN_OBJECTDESTROYED).
     *
     *      -- WPObject* mp2: SOM object pointer of object
     *                        being destroyed. NOTE: This pointer
     *                        is no longer valid. Do not invoke
     *                        any SOM methods on it.
     *
     *@@added V0.9.7 (2001-01-03) [umoeller]
     */

    #define XN_OBJECTDESTROYED          3

    /*
     * XN_QUERYSETUP:
     *      notification code for WM_CONTROL sent to a widget
     *      when the sender needs to know the widget's setup string.
     *
     *      Parameters:
     *
     *      -- SHORT1FROMMP(mp1): ID, always ID_XCENTER_CLIENT.
     *
     *      -- SHORT2FROMMP(mp1): notify code (XN_QUERYSETUP).
     *
     *      -- char *mp2: buffer into which the setup string is
     *                    copied.  It can be NULL, in which case
     *                    nothing is copied.  Otherwise, it is
     *                    expected to contain enough room for the
     *                    whole setup string.
     *
     *      The widget must return the minimum required size needed
     *      to store the setup string (even if mp2 is NULL).
     *
     *added V0.9.9 (2001-03-01) [lafaix]
     */

    // #define XN_QUERYSETUP               4

    // flags for XN_BEGINANIMATE and XN_ENDANIMATE
    #define XAF_SHOW                    1
    #define XAF_HIDE                    2

    /*
     *@@ XN_BEGINANIMATE:
     *      notification code for WM_CONTROL sent to a widget from
     *      an XCenter when it is about to begin animating.
     *
     *      Parameters:
     *
     *      -- SHORT1FROMMP(mp1): ID, always ID_XCENTER_CLIENT.
     *
     *      -- SHORT2FROMMP(mp1): notify code (XN_BEGINANIMATE).
     *
     *      -- ULONG mp2: XAF_SHOW if the parent initiate a 'show' animation,
     *                    XAF_HIDE if the parent initiate a 'hide' animation.
     *
     *      This notification is sent regardless of whether the XCenter
     *      actually does animation.  If it does no animation,
     *      XN_ENDANIMATE immediately follows XN_BEGINANIMATE.
     *
     *      An active widget can react to this message to start or
     *      stop doing something.  For example, a gauge widget can
     *      choose to stop running when the container is hidden, to
     *      save CPU cycles.
     *
     *@added V0.9.9 (2001-03-01) [lafaix]
     */

    #define XN_BEGINANIMATE             5

    /*
     *@@ XN_ENDANIMATE:
     *      notification code for WM_CONTROL sent to a widget from
     *      an XCenter when it has ended animating.
     *
     *      Parameters:
     *
     *      -- SHORT1FROMMP(mp1): ID, always ID_XCENTER_CLIENT.
     *
     *      -- SHORT2FROMMP(mp1): notify code (XN_ENDANIMATE).
     *
     *      -- ULONG mp2: XAF_SHOW if the parent ended a 'show' animation,
     *                    XAF_HIDE if the parent ended a 'hide' animation.
     *
     *      This notification is sent regardless of whether the XCenter
     *      actually does animation.  If it does no animation,
     *      XN_ENDANIMATE immediately follows XN_BEGINANIMATE.
     *
     *      An active widget can react to this message to start or
     *      stop doing something.  For example, a gauge widget can
     *      choose to stop running when the container is hidden, to
     *      save CPU cycles.
     *
     *@added V0.9.9 (2001-03-01) [lafaix]
     */

    #define XN_ENDANIMATE               6

    /*
     * XN_QUERYWIDGETCOUNT:
     *      notification code for WM_CONTROL sent from the XCenter
     *      to a widget when it needs to know how many elements a
     *      container-widget contains.
     *
     *      Parameters:
     *
     *      -- SHORT1FROMMP(mp1): ID, always ID_XCENTER_CLIENT.
     *
     *      -- SHORT2FROMMP(mp1): notify code (XN_QUERYWIDGETCOUNT).
     *
     *      -- PULONG mp2: pointer to an ULONG in which the widget
     *                     must fill in its widget count.
     *
     *      The widgets count must only include first level elements.
     *      That is, if a container contains other containers, the
     *      elements in those sub-containers should not be included.
     *
     *      The widget must return TRUE if it has put its count in the
     *      ULONG.  Otherwise, the XCenter will assume some dumb default
     *      for the count.
     *
     *added V0.9.9 (2001-02-23) [lafaix]
     *changed V0.9.9 (2001-03-11) [lafaix]: uses a PULONG to return the count.
     */

    // #define XN_QUERYWIDGETCOUNT         7

    /*
     * XN_QUERYWIDGET:
     *      notification code for WM_CONTROL sent from the XCenter
     *      to a widget when it needs to know the widget present at
     *      a given position.
     *
     *      Parameters:
     *
     *      -- SHORT1FROMMP(mp1): ID, always ID_XCENTER_CLIENT.
     *
     *      -- SHORT2FROMMP(mp1): notify code (XN_QUERYWIDGET).
     *
     *      -- ULONG mp2: widget index (0 is the first widget).
     *
     *      The widget must return 0 if no widget exists at that index.
     *      Otherwise it must return a pointer to the corresponding
     *      XCENTERWIDGET structure.
     *
     *added V0.9.9 (2001-02-23) [lafaix]
     */

    // #define XN_QUERYWIDGET              8

    // structure needed for XN_INSERTWIDGET
    typedef struct _WIDGETINFO
    {
        SHORT          sOffset;
                   // either WGT_END or the 0-based offset
        PXCENTERWIDGET pWidget;
                   // the widget to be inserted
    } WIDGETINFO, *PWIDGETINFO;

    // flags and return values for XN_INSERTWIDGET:
    #define WGT_END                     (-1)
    #define WGT_ERROR                   (-1)

    /*
     * XN_INSERTWIDGET:
     *      notification code for WM_CONTROL sent from the XCenter
     *      to a widget when it needs to add a widget at a specified
     *      offset to a container-widget.
     *
     *      Parameters:
     *
     *      -- SHORT1FROMMP(mp1): ID, always ID_XCENTER_CLIENT.
     *
     *      -- SHORT2FROMMP(mp1): notify code (XN_INSERTWIDGET).
     *
     *      -- PWIDGETINFO mp2: a pointer to a WIDGETINFO structure
     *                          that details the insertion.
     *
     *      The widget must return WGT_ERROR if the insertion failed.
     *      Otherwise it must return the offset of the widget following
     *      the inserted one.
     *
     *added V0.9.9 (2001-02-23) [lafaix]
     */

    // #define XN_INSERTWIDGET             9

    /*
     * XN_DELETEWIDGET:
     *      notification code for WM_CONTROL sent from the XCenter
     *      to a widget when it needs to remove a widget at a specified
     *      offset.
     *
     *      Parameters:
     *
     *      -- SHORT1FROMMP(mp1): ID, always ID_XCENTER_CLIENT.
     *
     *      -- SHORT2FROMMP(mp1): notify code (XN_INSERTWIDGET).
     *
     *      -- SHORT mp2: the to be removed widget's offset.
     *
     *      The widget must return the count of remaining widgets.
     *
     *added V0.9.9 (2001-02-23) [lafaix]
     */

    // #define XN_DELETEWIDGET             10

    /*
     *@@ XN_HITTEST:
     *      notification code for WM_CONTROL sent from the XCenter
     *      to a widget when it needs to know whether a specific
     *      location is covered by the widget.
     *
     *      Parameters:
     *
     *      -- SHORT1FROMMP(mp1): ID, always ID_XCENTER_CLIENT.
     *
     *      -- SHORT2FROMMP(mp1): notify code (XN_HITTEST).
     *
     *      -- POINTS mp2: the location to test.
     *
     *      The widget must return TRUE if the location is covered, or
     *      FALSE otherwise.
     *
     *@@added V0.9.13 (2001-06-23) [umoeller]
     */

    #define XN_HITTEST                  11

    /*
     *@@ XN_DISPLAYSTYLECHANGED:
     *      notification code for WM_CONTROL sent (!)
     *      to all widgets whenever an XCenter display
     *      style changes.
     *
     *      If your widget caches data from XCENTERGLOBALS
     *      locally somewhere, or needs to rearrange its
     *      display based on those settings, you should
     *      intercept this and reformat yourself.
     *
     *      It is not necessary to invalidate your widget
     *      display though since this will be done
     *      automatically by the XCenter when display
     *      styles change. So after this notification,
     *      you can be sure that WM_PAINT will come in
     *      anyway.
     *
     *      Parameters:
     *
     *      -- SHORT1FROMMP(mp1): ID, always ID_XCENTER_CLIENT.
     *
     *      -- SHORT2FROMMP(mp1): notify code (XN_DISPLAYSTYLECHANGED).
     *
     *      -- mp2: unused, always NULL.
     *
     *@@added V0.9.13 (2001-06-21) [umoeller]
     */

    #define XN_DISPLAYSTYLECHANGED      12

    /*
     *@@ XN_INUSECHANGED:
     *      notification code for WM_CONTROL posted (!)
     *      to a widget if it has registered itself with
     *      XFldObject::xwpSetWidgetNotify.
     *
     *      This is posted from XFldObject::wpCnrSetEmphasis
     *      when the object's in-use emphasis changes.
     *
     *      Parameters:
     *
     *      -- SHORT1FROMMP(mp1): ID, always ID_XCENTER_CLIENT.
     *
     *      -- SHORT2FROMMP(mp1): notify code (XN_INUSECHANGED).
     *
     *      -- WPObject* mp2: SOM object pointer of object whose
     *                  emphasis changed.
     *
     *@@added V0.9.13 (2001-06-21) [umoeller]
     */

    #define XN_INUSECHANGED             13

    /*
     *@@ XN_CENTERHOTKEYPRESSED:
     *      notification code for WM_CONTROL sent (!)
     *      to the first widget that has the WGTF_CANTAKECENTERHOTKEY
     *      class flag when the XCenter hotkey is pressed.
     *
     *      Presently this is only used by the X-button to
     *      open its menu when the XCenter gets activated via
     *      hotkey, but other widget classes could behave
     *      accordingly.
     *
     *      Parameters:
     *
     *      -- SHORT1FROMMP(mp1): ID, always ID_XCENTER_CLIENT.
     *
     *      -- SHORT2FROMMP(mp1): notify code (XN_CENTERHOTKEYPRESSED).
     *
     *      -- mp2: unused, always NULL.
     *
     *@@added V0.9.19 (2002-04-17) [umoeller]
     */

    #define XN_CENTERHOTKEYPRESSED      14

    /* ******************************************************************
     *
     *   Public messages _to_ XCenter client
     *
     ********************************************************************/

    /*
     *@@ XCM_SETWIDGETSIZE:
     *      this msg can be posted by a widget
     *      to its parent if it wants to change
     *      its size, e.g. because its display
     *      has changed and it needs more room.
     *
     *      The widget's parent will usually be
     *      the XCenter client. However, if you
     *      want your widget to be trayable, you
     *      should never use XCENTERGLOBALS.hwndClient
     *      but use WinQueryWindow(hwndWidget, QW_PARENT)
     *      instead, because if your widget is in
     *      a tray, its parent will be a tray widget,
     *      which understands this message as well.
     *
     *      Parameters:
     *
     *      -- HWND mp1: widget's window.
     *
     *      -- ULONG mp2: the new width that the
     *         widget wants to have.
     *
     *      Note: _Post_, do not send this message.
     *      This causes excessive redraw of possibly
     *      all widgets and will cause a flurry of
     *      messages being sent back to your widget.
     *
     *      Restrictions: This doesn't work during WM_CREATE
     *      of your widget.
     */

    #define XCM_SETWIDGETSIZE           WM_USER

    // formatting flags
    #define XFMF_DISPLAYSTYLECHANGED    0x0002
    #define XFMF_GETWIDGETSIZES         0x0001
    #define XFMF_RECALCHEIGHT           0x0004
    #define XFMF_REPOSITIONWIDGETS      0x0008
    #define XFMF_SHOWWIDGETS            0x0010
    #define XFMF_RESURFACE              0x0020
    #define XFMF_FOCUS2FIRSTWIDGET      0x0040

    /*
     *@@ XCM_REFORMAT:
     *      posted by a widget to its parent to reformat
     *      the XCenter (or a tray) and all sibling widgets.
     *      This gets posted by ctrDefWidgetProc when a widget
     *      gets destroyed, but can be posted by anyone.
     *
     *      The widget's parent will usually be
     *      the XCenter client. However, if you
     *      want your widget to be trayable, you
     *      should never use XCENTERGLOBALS.hwndClient
     *      but use WinQueryWindow(hwndWidget, QW_PARENT)
     *      instead, because if your widget is in
     *      a tray, its parent will be a tray widget,
     *      which understands this message as well.
     *
     *      Parameters:
     *
     *      -- ULONG mp1: reformat flags. Any combination
     *         of the following:
     *
     *          --  XFMF_GETWIDGETSIZES: ask each widget for its
     *              desired size.
     *
     *          --  XFMF_DISPLAYSTYLECHANGED: display style has
     *              changed, repaint everything.
     *
     *          --  XFMF_RECALCHEIGHT: recalculate the XCenter's
     *              height, e.g. if a widget has been added or
     *              removed or vertically resized.
     *
     *          --  XFMF_REPOSITIONWIDGETS: reposition all widgets.
     *              This is necessary if a widget's horizontal size
     *              has changed.
     *
     *          --  XFMF_SHOWWIDGETS: set WS_VISIBLE on all widgets.
     *
     *          --  XFMF_RESURFACE: resurface XCenter to HWND_TOP.
     *
     *          --  XFMF_FOCUS2FIRSTWIDGET: implies XFMF_RESURFACE,
     *              but will open the X-Button widget's menu, if
     *              appplicable.
     *
     *          Even if you specify 0, the XCenter will be re-shown
     *          if it is currently auto-hidden.
     *
     *      -- mp2: reserved, must be 0.
     *
     *      Note: _Post_, do not send this message.
     *      This causes excessive redraw of possibly
     *      all widgets and will cause a flurry of
     *      messages being sent back to your widget.
     *
     *      Restrictions: This doesn't work during WM_CREATE
     *      of your widget.
     */

    #define XCM_REFORMAT                (WM_USER + 1)

    /*
     *@@ XCM_SAVESETUP:
     *      this msg can be sent (!) by a widget to
     *      its parent if its settings have been
     *      changed and it wants these settings to
     *      be saved with the XCenter instance data.
     *
     *      The widget's parent will usually be
     *      the XCenter client. However, if you
     *      want your widget to be trayable, you
     *      should never use XCENTERGLOBALS.hwndClient
     *      but use WinQueryWindow(hwndWidget, QW_PARENT)
     *      instead, because if your widget is in
     *      a tray, its parent will be a tray widget,
     *      which understands this message as well.
     *
     *      This is useful when fonts or colors have
     *      been dropped on the widget and no settings
     *      dialog is currently open (and ctrSetSetupString
     *      therefore won't work).
     *
     *      Note: Saving widget setup strings in the XCenter
     *      data is a possibly expensive operation. Use
     *      this message economically.
     *
     *      Restrictions: This doesn't work during WM_CREATE
     *      of your widget.
     *
     *      Parameters:
     *
     *      -- HWND mp1: widget's window.
     *
     *      -- const char* mp2: zero-terminated setup string.
     *
     *      Returns: TRUE if the setup string was successfully
     *      saved.
     *
     *@@added V0.9.7 (2000-12-04) [umoeller]
     */

    #define XCM_SAVESETUP               (WM_USER + 2)

    // WM_USER + 3 to WM_USER + 10 are reserved

    /* ******************************************************************
     *
     *   Widget Plugin DLL Exports
     *
     ********************************************************************/

    // init-module export (ordinal 1)
    // WARNING: THIS PROTOTYPE HAS CHANGED WITH V0.9.9
    // IF QUERY-VERSION IS EXPORTED (@3, see below) THE NEW
    // PROTOTYPE IS USED
    typedef ULONG EXPENTRY FNWGTINITMODULE_OLD(HAB hab,
                                               HMODULE hmodXFLDR,
                                               PCXCENTERWIDGETCLASS *ppaClasses,
                                               PSZ pszErrorMsg);
    typedef FNWGTINITMODULE_OLD *PFNWGTINITMODULE_OLD;

    typedef ULONG EXPENTRY FNWGTINITMODULE_099(HAB hab,
                                               HMODULE hmodPlugin,
                                               HMODULE hmodXFLDR,
                                               PCXCENTERWIDGETCLASS *ppaClasses,
                                               PSZ pszErrorMsg);
    typedef FNWGTINITMODULE_099 *PFNWGTINITMODULE_099;

    // un-init-module export (ordinal 2)
    typedef VOID EXPENTRY FNWGTUNINITMODULE(VOID);
    typedef FNWGTUNINITMODULE *PFNWGTUNINITMODULE;

    // query version (ordinal 3; added V0.9.9 (2001-02-01) [umoeller])
    // IF QUERY-VERSION IS EXPORTED,THE NEW PROTOTYPE FOR
    // INIT_MODULE (above) WILL BE USED
    typedef VOID EXPENTRY FNWGTQUERYVERSION(PULONG pulMajor,
                                            PULONG pulMinor,
                                            PULONG pulRevision);
    typedef FNWGTQUERYVERSION *PFNWGTQUERYVERSION;
#endif

