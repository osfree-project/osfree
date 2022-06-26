
/*
 *@@sourcefile notebook.h:
 *      header file for notebook.c. See notes there. New with V0.82.
 *
 *      All the functions in this file have the ntb* prefix.
 *
 *@@include #define INCL_DOSMODULEMGR
 *@@include #define INCL_WINWINDOWMGR
 *@@include #define INCL_WINSTDCNR
 *@@include #include <os2.h>
 *@@include #include <wpobject.h>
 *@@include #include "shared\notebook.h"
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

#ifndef NOTEBOOK_HEADER_INCLUDED
    #define NOTEBOOK_HEADER_INCLUDED

    /********************************************************************
     *
     *   Notebook settings page IDs
     *
     ********************************************************************/

    // XWorkplace settings page IDs; these are used by
    // the following:
    // --  the notebook.c functions to identify open
    //     pages;
    // --  the settings functions in common.c to
    //     identify the default settings to be set.

    // If you add a settings page using notebook.c, define a new
    // ID here. Use any ULONG you like.

    // Groups of settings pages:
    // 1) in "Workplace Shell"
    #define SP_WPS_FOLDERVIEWS      1       // renamed from SP_1GENERIC V0.9.20 (2002-07-31) [umoeller]
    // #define SP_2REMOVEITEMS         2    // removed V0.9.19 (2002-04-17) [umoeller]
    // #define SP_25ADDITEMS           3    // removed V0.9.19 (2002-04-17) [umoeller]
    #define SP_MENUSETTINGS         2       // added V0.9.19 (2002-04-17) [umoeller]
    #define SP_MENUITEMS            3       // added V0.9.19 (2002-04-17) [umoeller]
    #define SP_26CONFIGITEMS        4
    // #define SP_27STATUSBAR          5    // replaced with SP_STATUSBARS1 V1.0.1 (2002-12-08) [umoeller]
    #define SP_3SNAPTOGRID          6
    #define SP_4ACCELERATORS        7
    // #define SP_5INTERNALS           8    // removed (V0.9.0)
    // #define SP_DTP2                 10   // removed (V0.9.0)
    // #define SP_28STATUSBAR2         11   // replaced with SP_STATUSBARS2 V1.0.1 (2002-12-08) [umoeller]
    // #define SP_FILEOPS              12   // removed (V0.9.0)
    #define SP_FILETYPES            13      // new with V0.9.0 (XFldWPS)

    #define SP_STATUSBARS1          15      // V1.0.1 (2002-12-08) [umoeller]
    #define SP_STATUSBARS2          16      // V1.0.1 (2002-12-08) [umoeller]
    #define SP_TOOLBARS1            17      // V1.0.1 (2002-12-08) [umoeller]

    // 2) in "OS/2 Kernel"
    #define SP_SCHEDULER            20
    #define SP_MEMORY               21
    // #define SP_HPFS                 22   this is dead! we now have a settings dlg
    #define SP_FAT                  23
    #define SP_ERRORS               24
    #define SP_WPS                  25
    #define SP_SYSPATHS             26      // new with V0.9.0
    #define SP_DRIVERS              27      // new with V0.9.0
    #define SP_SYSLEVEL             28      // new with V0.9.2 (2000-03-08) [umoeller]

    // 3) in "XWorkplace Setup"
    #define SP_SETUP_INFO           30      // new with V0.9.0
    #define SP_SETUP_FEATURES       31      // new with V0.9.0
    #define SP_SETUP_PARANOIA       32      // new with V0.9.0
    #define SP_SETUP_OBJECTS        33      // new with V0.9.0
    #define SP_SETUP_XWPLOGO        34      // new with V0.9.6 (2000-11-04) [umoeller]
    #define SP_SETUP_THREADS        35      // new with V0.9.9 (2001-03-07) [umoeller]
    #ifdef __DEBUG__
        #define SP_SETUP_DEBUG      36      // new with V1.0.0 (2002-09-02) [umoeller]
    #endif

    // 4) "Sort" pages both in folder notebooks and
    //    "Workplace Shell"
    #define SP_FLDRSORT_FLDR        40
    #define SP_FLDRSORT_GLOBAL      41

    // 5) "XFolder" page in folder notebooks
    #define SP_XFOLDER_FLDR         45      // fixed V0.9.1 (99-12-06)

    // 6) "Startup" page in XFldStartup notebook
    #define SP_STARTUPFOLDER        50      // new with V0.9.0

    // 7) "File" page in XFldDataFile/XFolder
    #define SP_FILE1                60      // new with V0.9.0
    #define SP_FILE2                61      // new with V0.9.1 (2000-01-22) [umoeller]
    #define SP_DATAFILE_TYPES       62      // XFldDataFile "Types" page V0.9.9 (2001-03-27) [umoeller]
    #define SP_FILE3_PERMISSIONS    63      // new with V1.0.1 (2003-01-10) [umoeller]

    // 8) "Sounds" page in XWPSound
    #define SP_SOUNDS               70

    // 9) pages in XFldDesktop
    #define SP_DTP_MENUITEMS        80      // new with V0.9.0
    #define SP_DTP_STARTUP          81      // new with V0.9.0
    #define SP_DTP_SHUTDOWN         82      // new with V0.9.0
    #define SP_DTP_ARCHIVES         83      // new with V0.9.0

    // 10) pages for XWPTrashCan
    #define SP_TRASHCAN_SETTINGS    90      // new with V0.9.0; renamed V0.9.1 (99-12-12)
    #define SP_TRASHCAN_DRIVES      91      // new with V0.9.1 (99-12-12)
    #define SP_TRASHCAN_ICON        92      // new with V0.9.4 (2000-08-03) [umoeller]

    // 11) "Details" pages
    #define SP_DISK_DETAILS         100     // new with V0.9.0
    #define SP_PROG_DETAILS         101     // new with V0.9.0
    #define SP_PROG_RESOURCES       102     // new with V0.9.7 (2000-12-17) [lafaix]
    #define SP_PROG_DETAILS1        103
    #define SP_PROG_DETAILS2        104

    // 12) XWPClassList
    #define SP_CLASSLIST            110     // new with V0.9.0

    // 13) XWPKeyboard
    #define SP_KEYB_OBJHOTKEYS      120     // new with V0.9.0
    #define SP_KEYB_FUNCTIONKEYS    121     // new with V0.9.3 (2000-04-18) [umoeller]
    #define SP_KEYB_MACROS          122     // new with V0.9.20 (2002-07-03) [umoeller]

    // 13) XWPMouse
    #define SP_MOUSE_MOVEMENT       130     // new with V0.9.2 (2000-02-26) [umoeller]
    #define SP_MOUSE_CORNERS        131     // new with V0.9.2 (2000-02-26) [umoeller]
    #define SP_MOUSE_MAPPINGS2      132     // new with V0.9.1
    #define SP_MOUSE_MOVEMENT2      133     // new with V0.9.14 (2001-08-02) [lafaix]

    // 14) XWPScreen
    #define SP_PAGER_MAIN           140     // new with V0.9.3 (2000-04-09) [umoeller]
    #define SP_PAGER_WINDOW         141     // new with V0.9.9 (2001-03-27) [umoeller]
    #define SP_PAGER_STICKY         142     // new with V0.9.3 (2000-04-09) [umoeller]
    #define SP_PAGER_COLORS         143     // new with V0.9.3 (2000-04-09) [umoeller]

    // 15) XWPString
    #define SP_XWPSTRING            150     // new with V0.9.3 (2000-04-27) [umoeller]

    // 16) XWPMedia
    #define SP_MEDIA_DEVICES        160     // new with V0.9.3 (2000-04-29) [umoeller]
    #define SP_MEDIA_CODECS         161     // new with V0.9.3 (2000-04-29) [umoeller]
    #define SP_MEDIA_IOPROCS        162     // new with V0.9.3 (2000-04-29) [umoeller]

    // 17) XCenter
    #define SP_XCENTER_VIEW1        170     // new with V0.9.7 (2000-12-05) [umoeller]
    #define SP_XCENTER_VIEW2        171     // new with V0.9.7 (2001-01-18) [umoeller]
    #define SP_XCENTER_WIDGETS      172     // new with V0.9.7 (2000-12-05) [umoeller]
    #define SP_XCENTER_CLASSES      173     // new with V0.9.9 (2001-03-09) [umoeller]

    // 18) WPProgram/WPProgramFile
    #define SP_PGM_ASSOCS           180     // new with V0.9.9 (2001-03-07) [umoeller]
    #define SP_PGMFILE_ASSOCS       181     // new with V0.9.9 (2001-03-07) [umoeller]

    // 19) XWPFontFolder
    #define SP_FONT_SAMPLETEXT      190     // new with V0.9.9 (2001-03-27) [umoeller]

    // 20) XWPAdmin
    #define SP_ADMIN_LOCAL_USER     200     // new with V0.9.19 (2002-04-02) [umoeller]
    #define SP_ADMIN_ALL_USERS      201     // new with V0.9.19 (2002-04-02) [umoeller]
    #define SP_ADMIN_ALL_GROUPS     202     // new with V0.9.19 (2002-04-02) [umoeller]
    #define SP_ADMIN_PROCESSES      203     // new with V1.0.2 (2003-11-13) [umoeller]

    // 21) XFldObject
    #define SP_OBJECT_ICONPAGE1     220     // new with V0.9.16 (2001-10-15) [umoeller]
    #define SP_OBJECT_ICONPAGE2     221     // new with V0.9.16 (2001-10-15) [umoeller]
                // this is really a WPFolder page...
    #define SP_OBJECT_ICONPAGE1_X   222
                // icon page where wpAddObjectGeneralPage returned
                // SETTINGS_PAGE_REMOVED (WPSharedDir)
                // new with V0.9.19 (2002-06-15) [umoeller]

    // 22) XWPVCard V0.9.16 (2002-02-02) [umoeller]
    #define SP_VCARD_SUMMARY        230
    #define SP_VCARD_NAME           231
    #define SP_VCARD_ADDRESSES      232
    #define SP_VCARD_PHONE          233

    /********************************************************************
     *
     *   Declarations
     *
     ********************************************************************/

    // resize information for ID_XFD_CONTAINERPAGE, which is used
    // by many settings pages
    extern MPARAM *G_pampGenericCnrPage;
    extern ULONG G_cGenericCnrPage;

    // forward-declare the NOTEBOOKPAGE types, because
    // these are needed by the function prototypes below
    typedef struct _NOTEBOOKPAGE *PNOTEBOOKPAGE;

    // some callback function prototypes:

    // 1)  init-page callback
    typedef VOID XWPENTRY FNCBACTION(PNOTEBOOKPAGE, ULONG);
    typedef FNCBACTION *PFNCBACTION;

    // 2)  item-changed callback
    typedef MRESULT XWPENTRY FNCBITEMCHANGED(PNOTEBOOKPAGE,
                                      ULONG,    // ulItemID
                                            // V0.9.9 (2001-03-27) [umoeller]: turned USHORT into ULONG
                                      USHORT,   // usNotifyCode
                                      ULONG);   // ulExtra
    typedef FNCBITEMCHANGED *PFNCBITEMCHANGED;

    // 3)  message callback
    typedef BOOL XWPENTRY FNCBMESSAGE(PNOTEBOOKPAGE, ULONG, MPARAM, MPARAM, MRESULT*);
    typedef FNCBMESSAGE *PFNCBMESSAGE;

    /*
     *  here come the ORed flags which are passed to
     *  the INIT callback
     */

    #define CBI_INIT        0x01        // only set on first call (WM_INITDLG)
    #define CBI_SET         0x02        // controls should be set
    #define CBI_ENABLE      0x04        // controls should be en/disabled
    #define CBI_DESTROY     0x08        // notebook page is destroyed
    #define CBI_SHOW        0x10        // notebook page is turned to
    #define CBI_HIDE        0x20        // notebook page is turned away from

    // #define XNTBM_UPDATE     (WM_USER)  // update
                // moved to common.h

    #ifndef BKA_MAJOR
       #define BKA_MAJOR                0x0040
    #endif

    #ifndef BKA_MINOR
       #define BKA_MINOR                0x0080
    #endif

    #pragma pack(1)

    /*
     *@@ INSERTNOTEBOOKPAGE:
     *      this structure must be passed to ntbInsertPage
     *      and specifies lots of data according to which
     *      fnwpPageCommon will react.
     *
     *      Always zero the entire structure and then fill
     *      in only the fields that you need. The top fields
     *      listed below are required, all the others are
     *      optional and have safe defaults.
     *
     *      See ntbInsertPage for how to use this.
     *
     *@@changed V0.9.0 [umoeller]: typedef was missing, thanks Rdiger Ihle
     *@@changed V0.9.4 (2000-07-11) [umoeller]: added fPassCnrHelp
     *@@changed V0.9.18 (2002-02-23) [umoeller]: renamed from CREATENOTEBOOKPAGE, removed non-input data
     *@@changed V1.0.1 (2003-01-05) [umoeller]: added ulEditCnrID
     */

    typedef struct _INSERTNOTEBOOKPAGE
    {
        // 1) REQUIRED input to ntbInsertPage
        HWND        hwndNotebook;   // hwnd of Notebook control; set this to the
                                    // (misnamed) hwndDlg parameter in the
                                    // _wpAddSettingsPages method
        WPObject    *somSelf;       // object whose Settings notebook is opened;
                                    // set this to somSelf of _wpAddSettingsPages
        HMODULE     hmod;           // module of dlg resource
        ULONG       ulDlgID;        // ID of dlg resource (in hmod)
        PCSZ        pcszName;       // title of page (in notebook tab)

        // 2) OPTIONAL input to ntbInsertPage; all of these can be null
        USHORT      ulPageID;       // the page identifier, which should be set to
                                    // uniquely identify the notebook page (e.g. for
                                    // ntbQueryOpenPages); XWorkplace uses the SP_*
                                    // IDs def'd in common.h.
        USHORT      usPageStyleFlags; // any combination or none of the following:
                                    // -- BKA_MAJOR
                                    // -- BKA_MINOR
                                    // BKA_STATUSTEXTON will always be added.
        BOOL        fEnumerate;     // if TRUE: add "page 1 of 3"-like thingies
        PCSZ        pcszMinorName;  // if != NULL, subtitle to add to notebook context
                                    // menu V0.9.16 (2001-10-23) [umoeller]
                                    // (useful with fEnumerate)
        BOOL        fPassCnrHelp;   // if TRUE: CN_HELP is not intercepted, but sent
                                    // to "item changed" callback;
                                    // if FALSE: CN_HELP is processed like WM_HELP
                                    // V0.9.4 (2000-07-11) [umoeller]
        USHORT      ulDefaultHelpPanel; // default help panel ID for the whole page
                                    // in the XFolder help file;
                                    // this will be displayed when WM_HELP comes in
                                    // and if no subpanel could be found
        USHORT      ulTimer;        // if !=0, a timer will be started and pfncbTimer
                                    // will be called with this frequency (in ms)
        MPARAM      *pampControlFlags; // if != NULL, winhAdjustControls will be
                                    // called when the notebook gets resized with
                                    // the array of MPARAM's specified here; this
                                    // allows for automatic resizing of notebook
                                    // pages
        ULONG       cControlFlags;  // if (pampControlFlags != NULL), specify the
                                    // array item count here

        ULONG       ulCnrOwnerDraw; // CODFL_* flags for container owner draw,
                                    // if CA_OWNERDRAW is set for a container.
                                    // If this is != 0, cnrhOwnerDrawRecord is
                                    // called for painting records with these
                                    // flags; see cnrhOwnerDrawRecord for valid
                                    // values.
                                    // V0.9.16 (2001-09-29) [umoeller]

        USHORT      ulEditCnrID;    // ID of cnr window for which to add context
                                    // menu support. This is for those pages
                                    // which have the typical add/edit/remove
                                    // buttons, such as the pager sticky windows
                                    // page. It is still the responsibility of
                                    // the caller to supply and create the controls,
                                    // but the notebook proc will automatically
                                    // load a context menu then and post DID_ADD,
                                    // DID_EDIT, and DID_REMOVE commands to the
                                    // "item changed" callback, where preccSource
                                    // will be set to the correctly selected
                                    // record in the container or NULL if the
                                    // click was on whitespace. It will also
                                    // automatically enable and disable the
                                    // buttons with those three IDs correctly.
                                    // To enable this functionality, set this
                                    // member to the ID of the container.
                                    // added V1.0.1 (2003-01-05) [umoeller]

        // 3)  Here follow the callback functions. If any of these is NULL,
        //     it will not be called. As a result, you may selectively install
        //     callbacks, depending on how much functionality you need.

        PFNCBACTION pfncbInitPage;
                // callback function for initializing the page.
                // This is required and gets called (at least) when
                // the page is initialized (WM_INITDLG comes in).
                // See ntbInsertPage for details.

        PFNCBITEMCHANGED pfncbItemChanged;
                // callback function if an item on the page has changed; you
                // should update your data in memory then.
                // See ntbInsertPage for details.

        PFNCBACTION pfncbTimer;
                // optional callback function if INSERTNOTEBOOKPAGE.ulTimer != 0;
                // this callback gets called every INSERTNOTEBOOKPAGE.ulTimer
                // milliseconds then.

        PFNCBMESSAGE pfncbMessage;
                // optional callback function thru which all dialog messages are going.
                // You can use this if you need additional handling which the above
                // callbacks do not provide for. This gets really all the messages
                // which go thru fnwpPageCommon.
                //
                // This callback gets called _after_ all other message processing
                // (i.e. the "item changed" and "timer" callbacks).
                //
                // Parameters:
                //     PNOTEBOOKPAGE pcnbp      notebook info struct
                //     msg, mp1, mp2            usual message parameters.
                //     MRESULT* pmrc            return value, if TRUE is returned.
                //
                // If the callback returns TRUE, *pmrc is returned from the
                // common notebook page proc.

    } INSERTNOTEBOOKPAGE, *PINSERTNOTEBOOKPAGE;

    /*
     *@@ NOTEBOOKPAGE:
     *
     *@@added V0.9.18 (2002-02-23) [umoeller]
     */

    typedef struct _NOTEBOOKPAGE
    {
        INSERTNOTEBOOKPAGE  inbp;

        PVOID       pUser,
                    pUser2;         // user data; since you can access this structure
                // from the "pcnbp" parameter which is always passed to the notebook
                // callbacks, you can use this for backing up data for the "Undo" button
                // in the INIT callback, or for whatever other data you might need.
                // Simply allocate memory using malloc() and store it here.
                // When the notebook page is destroyed, both pointers are checked and
                // will automatically be free()'d if != NULL.

        // 4) The following fields are not intended for _input_ to ntbInsertPage.
        //    Instead, these contain additional data which can be evaluated from
        //    the callbacks. These fields are only set by fnwpPageCommon
        //    _after_ the page has been initialized.
        ULONG       ulNotebookPageID; // the PM notebook page ID, as returned by
                                      // wpInsertSettingsPage
        ULONG       flPage;           // any combination of the following:
                                      // V0.9.19 (2002-04-24) [umoeller]
                          #define NBFL_PAGE_INITED          0x0001
                                    // TRUE after the INIT callback has been called
                          #define NBFL_PAGE_SHOWING         0x0002
                                    // TRUE if the page is currently turned to
                                    // in the notebook
        HWND        hwndDlgPage;      // hwnd of dlg page in notebook; this
                                      // is especially useful to get control HWND's:
                                      // use WinWindowFromID(pnbp->hwndDlgPage, YOUR_ID).
        HWND        hwndFrame;        // frame window (to which hwndNotebook belongs);
                                      // use this as the owner for subdialogs to lock
                                      // the notebook
        HWND        hwndControl;      // this always has the current control window handle
                                      // when the "item changed" callback is called.
                                      // In the callback, this is equivalent to
                                      // calling WinWindowFromID(pnbp->hwndDlgPage, usControlID).
        HWND        hwndSourceCnr;    // see next
        PRECORDCORE preccSource;      // this can be set to a container record
                                      // core in hwndSourceCnr which will be removed
                                      // source emphasis from when WM_MENUEND
                                      // is received; useful for CN_CONTEXTMENU.
                                      // This gets initialized to -1, because
                                      // NULL means container whitespace.
        POINTL      ptlMenuMousePos;  // for CN_CONTEXTMENU, this has the
                                      // last mouse position (in Desktop coords).
        BOOL        fShowWaitPointer; // while TRUE, fnwpPageCommon shows the "Wait" pointer;
                                      // only meaningful if another thread is preparing data
        HWND        hwndTooltip;      // if this is != NULL, this window gets destroyed
                                      // automatically when the notebook page is destroyed.
                                      // Useful with tooltip controls, which are not destroyed
                                      // automatically otherwise.

        // 5) Internal use only, do not mess with these.
        PVOID       pnbli;
        PRECORDCORE preccLastSelected;
        PRECORDCORE preccExpanded;      // for tree-view auto scroll
        HWND        hwndExpandedCnr;    // for tree-view auto scroll
        PVOID       pxac;               // ptr to XADJUSTCTRLS if (pampControlFlags != NULL)
        HWND        hmenuSel,
                    hmenuWhitespace;    // cnr context menus

    } NOTEBOOKPAGE;

    #pragma pack()

    /*
     *@@ NOTEBOOKPAGELISTITEM:
     *      list item structure (linklist.c) for maintaining
     *      a list of currently open notebook pages.
     */

    typedef struct _NOTEBOOKPAGELISTITEM
    {
        ULONG                   ulSize;
        PNOTEBOOKPAGE           pnbp;
    } NOTEBOOKPAGELISTITEM, *PNOTEBOOKPAGELISTITEM;

    /*
     *@@ SUBCLNOTEBOOKLISTITEM:
     *      list item structure (linklist.c) for
     *      maintaining subclassed notebooks controls.
     *
     *@@added V0.9.1 (99-12-06) [umoeller]
     */

    typedef struct _SUBCLNOTEBOOKLISTITEM
    {
        HWND            hwndNotebook;
        PFNWP           pfnwpNotebookOrig;
    } SUBCLNOTEBOOKLISTITEM, *PSUBCLNOTEBOOKLISTITEM;

    /********************************************************************
     *
     *   Prototypes
     *
     ********************************************************************/

    ULONG ntbInsertPage(PINSERTNOTEBOOKPAGE pinbp);

    #ifdef DIALOG_HEADER_INCLUDED
        APIRET ntbFormatPage(HWND hwndDlg,
                             PCDLGHITEM paDlgItems,
                             ULONG cDlgItems);
    #endif

    PNOTEBOOKPAGE ntbQueryOpenPages(PNOTEBOOKPAGE pnbp);

    ULONG ntbUpdateVisiblePage(WPObject *somSelf, ULONG ulPageID);

    BOOL ntbTurnToPage(HWND hwndNotebook,
                       ULONG ulPageID);

    BOOL ntbOpenSettingsPage(PCSZ pcszObjectID,
                             ULONG ulPageID);

#endif
