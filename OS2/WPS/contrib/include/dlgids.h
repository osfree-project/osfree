
/*
 *@@sourcefile dlgids.h:
 *      this file declares all the dialog id's common to
 *      all XWorkplace components, but only those which are needed
 *      by both the XFolder code and the NLS resource DLLs.
 *
 *      This file is #include'd by the XFolder C code itself
 *      and all the .RC and .DLG files.
 *
 *      Changes for V0.9.0:
 *      -- lots of new IDs, of course, for all the new dialogs.
 *         Note: even the existing IDs have mostly been redefined.
 *      -- greatly rearranged this file, because I was finally
 *         unable to find anything in here any more.
 *      -- dialog ID's for Treesize and NetscapeDDE have been moved
 *         into this file to allow for NLS for these two programs.
 *
 *      Since these dialog ID's are shared across all of XWorkplace,
 *      we need to have "number spaces" for the different developers.
 *      If you add resources to XFLDRxxx.DLL, add your "number space"
 *      to the list below. Do not add resources which are part of
 *      another developer's number space, or we'll get into trouble.
 *
 *      NOTE: The ID's in this file are limited to 12000. All resource
 *            ID's must be below that number because numbers above that
 *            are reserved for other XWorkplace parts. For example,
 *            the animated mouse pointers use ID's 0x7000 (28672) and
 *            above. V0.9.4 (2000-06-15) [umoeller]
 *
 *      Current number spaces:
 *      -- Ulrich M”ller:  200-12000
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

#ifndef DLGIDS_HEADER_INCLUDED
    #define DLGIDS_HEADER_INCLUDED

/* Naming conventions:
   All ID's (= def's for integers) begin with "ID_",
   then comes an abbreviation of the context of the ID:
   -    XF   for XFolder,
   -    XS   for XWorkplace settings (XFldSystem, XFldWPS, Desktop),
   -    OS   for "OS/2 Kernel" settings
   -    XC   for XWPSetup object ("XWorkplace Setup", new with V0.9.0),
   -    XL   for XWPClassList object ("WPS Class List", new with V0.9.0)
   -    CR   for XCenter object (V0.9.7 (2000-11-39) [umoeller])
   -    AM   for XWPAdmin V0.9.11 (2001-04-22) [umoeller],
   -    SD   for XShutdown,
   -    XT   for XWPTrashCan,
   -    FN   for fonts,
   -    WP   for previously undocumented WPS ID's,
   -    TS   for Treesize;
   -    ND   for NetscapeDDE
   -    DM   for Daemon
   -    FX   for xfix
   then comes an abbreviation of the ID type:
   -    M    for a menu template,
   -    MI   for a menu item,
   -    D    for a dialog template,
   -    DI   for a dialog item;
   -    H    for a help panel res id;
   -    SI   for a string resource.

   Example:
        ID_SDDI_SKIPAPP is a dlg item ID for use with XShutdown
                        (the "Skip" button in the status window).
*/

/* ******************************************************************
 *
 *   Version info
 *
 ********************************************************************/

// XFolder version flags; since this file is
// #include'd in the NLS DLLs also, this string
// will be readable as a resource in the NLS DLL
#define XFOLDER_VERSION        "1.1.0"

#define XFOLDER_MAJOR          1
#define XFOLDER_MINOR          1
#define XFOLDER_REVISION       0

// this sets the minimum version number that
// the current version's XCenter plugin DLLs
// require
// V1.0.1 (2002-12-14) [umoeller]
#define XFOLDER_MAJOR          1
#define XFOLDER_MINOR          1
#define XFOLDER_REVISION       0

// this sets the minimum version number for NLS DLLS
// which XFolder will accept
#define MINIMUM_NLS_VERSION    "1.1.0"

#define MINIMUM_NLS_MAJOR      1
#define MINIMUM_NLS_MINOR      1
#define MINIMUM_NLS_REVISION   0

/* ******************************************************************
 *
 *   Bitmap and icon IDs
 *
 ********************************************************************/

#define ID_ICON1               100
#define ID_ICON2               101
#define ID_ICONDLG             102
#define ID_ICONSHUTDOWN        103

#define ID_STARTICON1          104
#define ID_STARTICON2          105
#define ID_SHUTICON1           106
#define ID_SHUTICON2           107

#define ID_ICONSYS             108
#define ID_ICONWPS             109

#define ID_ICONSDANIM1         110
#define ID_ICONSDANIM2         111
#define ID_ICONSDANIM3         112
#define ID_ICONSDANIM4         113
#define ID_ICONSDANIM5         114

// #define ID_ICONMENUARROW4      115 removed V0.9.7 (2001-01-26) [umoeller]
// #define ID_ICONMENUARROW3      116

#define ID_ICONXWPCONFG        117
#define ID_ICONXWPLIST         118
#define ID_ICONXWPSOUND        119
#define ID_ICONXWPSCREEN       120

#define ID_ICONXWPTRASHEMPTY   121
#define ID_ICONXWPTRASHFILLED  122

#ifndef __XWPLITE__
#define ID_ICONXWPMEDIA        123
#endif
#define ID_ICONXWPSTRING       124
#define ID_ICONXMMCDPLAY       125
#define ID_ICONXMMVOLUME       126
#define ID_ICONXCENTER         127

#define ID_BMPXMINI            128          // changed V0.9.19 (2002-04-14) [umoeller]

#ifndef __XWPLITE__
#define ID_ICONXWPADMIN        129  // added V0.9.9 (2001-02-08) [umoeller]
#endif

// #define ID_XFLDRBITMAP         130       // removed V0.9.16 (2001-11-10) [umoeller]

#define ID_XWPBIGLOGO          131

#define ID_ICON_CDEJECT        140
#define ID_ICON_CDNEXT         141
#define ID_ICON_CDPAUSE        142
#define ID_ICON_CDPLAY         143
#define ID_ICON_CDPREV         144
#define ID_ICON_CDSTOP         145

#define ID_ICONXWPFONTCLOSED   150
#define ID_ICONXWPFONTOPEN     151
#define ID_ICONXWPFONTOBJ      152
#define ID_ICONXWPFONTFILE     153          // added V0.9.16 (2001-11-25) [umoeller]
#define ID_ICONXWPVCARD        154          // added V0.9.16 (2002-01-05) [umoeller]

#define ID_ICONXWPSHADOWOVERLAY 155         // added V0.9.20 (2002-07-31) [umoeller]

// icons used by "diskfree" widget
#define ID_ICON_DRIVE          160
#define ID_POINTER_HAND        161
#define ID_ICON_DRIVE_CD       162
#define ID_ICON_DRIVE_LAN      163
#define ID_ICON_DRIVE_NORMAL   164

// icons used by monitor widget
#define ID_POWER_AC            170
#define ID_POWER_BATTERY       171

// icons used by tray widget
// #define ID_ICON_TRAY           180
#define ID_BITMAP_TRAY         180          // replaced V0.9.16 (2001-10-28) [umoeller]

// default toolbar icons V1.0.1 (2002-12-08) [umoeller]
#define ID_TB_DETAILS          190
#define ID_TB_FIND             191
#define ID_TB_HELP             192
#define ID_TB_MULTIPLECOLUMNS  193
#define ID_TB_RERESH           194
#define ID_TB_SMALLICONS       195

/* ******************************************************************
 *
 *   Generic dialog IDs < 100
 *
 ********************************************************************/

/* notebook buttons; these must be < 100
   so that they can be moved for Warp 4
   notebooks */
#define DID_APPLY              98
#define DID_HELP               97
#define DID_UNDO               96
#define DID_DEFAULT            95
#define DID_OPTIMIZE           94
#define DID_REFRESH            93
#define DID_SHOWTOOLTIPS       92           // always used for tooltip controls, V0.9.0
#define DID_TOOLTIP            91           // always used for tooltip controls, V0.9.0
#define DID_DETAILS            90           // V0.9.16 (2001-10-15) [umoeller]
#define DID_CLOSE              89
#define DID_YES                88
#define DID_YES2ALL            87
#define DID_NO                 86
#define DID_ABORT              85
#define DID_RETRY              84
#define DID_IGNORE             83
#define DID_OPEN               82           // V1.0.0 (2002-09-13) [umoeller]
#define DID_SAVE               81           // V1.0.0 (2002-09-13) [umoeller]
#define DID_CLEAR              80           // moved this here from Treesize section V1.0.0 (2002-11-23) [umoeller]

/* DID_OK and DID_CANCEL are def'd somewhere in os2.h */

/* Notebook pages */
// #define ID_XSD_SET5INTERNALS        409  // removed (V0.9.0)
// #define ID_XSD_SETTINGS_DTP2        407  // removed (V0.9.0)
// #define ID_XCD_FILEOPS              412  // removed (V0.9.0)

/******************************************
 *  common controls which are _not_ notebook buttons
 ******************************************/

#define DID_BROWSE              101

// the following added with V0.9.19 (2002-04-17) [umoeller]
#define DID_ADD                 102
#define DID_EDIT                103
#define DID_REMOVE              104

// Xfix dialog control added V1.0.4 (2005-02-24) [chennecke]
#define DID_ENTER               105

/* ******************************************************************
 *
 *   Miscellaneous dialogs >= 150
 *
 ********************************************************************/

// batch rename dialog V0.9.19 (2002-06-18) [umoeller]
#define ID_XFDI_BATCHRENAME_TITLE       150
#define ID_XFDI_BATCH_INTROTXT          151
#define ID_XFDI_BATCH_SOURCETXT         152
#define ID_XFDI_BATCH_SOURCEEF          153
#define ID_XFDI_BATCH_TARGETTXT         154
#define ID_XFDI_BATCH_TARGETEF          155
#define ID_XFDI_BATCH_GO                156
#define ID_XFDI_BATCH_SELONLYCB         157

// replacement "Paste" dialog V0.9.20 (2002-08-08) [umoeller]
#define ID_XFDI_PASTE_TITLE             170
#define ID_XFDI_PASTE_PREVIEW_TXT       171
#define ID_XFDI_PASTE_PREVIEW_PANE      172
#define ID_XFDI_PASTE_OBJTITLE_TXT      173
#define ID_XFDI_PASTE_OBJTITLE_DROP     174
#define ID_XFDI_PASTE_FORMAT_TXT        175
#define ID_XFDI_PASTE_FORMAT_DROP       176
#define ID_XFDI_PASTE_CLASS_TXT         177
#define ID_XFDI_PASTE_CLASS_DROP        178

// for product info
// #define ID_XFD_PRODINFO                 200      // removed V0.9.16 (2001-11-10) [umoeller]
#define ID_XFD_PRODLOGO                 201
// #define ID_XFDI_XFLDVERSION             202      // removed V0.9.16 (2001-11-10) [umoeller]
#define ID_XSDI_INFO_TITLE              202     // added V0.9.16 (2002-01-13) [umoeller]
#define ID_XSDI_INFO_STRING             203
#define ID_XSDI_INFO_MAINMEM_TXT        204     // added V0.9.20 (2002-08-10) [umoeller]
#define ID_XSDI_INFO_MAINMEM_VALUE      205     // added V0.9.20 (2002-08-10) [umoeller]
#define ID_XSDI_INFO_FREEMEM_TXT        206     // added V0.9.20 (2002-08-10) [umoeller]
#define ID_XSDI_INFO_FREEMEM_VALUE      207     // added V0.9.20 (2002-08-10) [umoeller]
#define ID_XSDI_INFO_BUGREPORT          208     // added V1.0.0 (2002-08-28) [umoeller]

// "Select by name" dlg items
#define ID_XFDI_SELECTSOME_TITLE        210      // V0.9.19 (2002-04-17) [umoeller]
#define ID_XFDI_SOME_ENTRYFIELD         211
#define ID_XFDI_SOME_SELECT             212
#define ID_XFDI_SOME_DESELECT           213
#define ID_XFDI_SOME_SELECTALL          214
#define ID_XFDI_SOME_DESELECTALL        215
#define ID_XFDI_SOME_INTROTXT           216
#define ID_XFDI_SOME_REGEXP_CP          217
#define ID_XFDI_SOME_CASESENSITIVE_CB   218

// generic dlg text removed V0.9.18 (2002-02-06) [umoeller]
// #define ID_XFD_GENERICDLG               220
// #define ID_XFDI_GENERICDLGTEXT          221
// #define ID_XFDI_GENERICDLGICON          222

#define ID_XFD_EMPTYDLG                 225     // V0.9.16 (2001-09-29) [umoeller]

// "Title clash" dlg
#define ID_XFD_TITLECLASH               230
#define ID_XFDI_CLASH_TXT1              231
#define ID_XFDI_CLASH_RENAMENEW         232
#define ID_XFDI_CLASH_RENAMENEWTXT      233
#define ID_XFDI_CLASH_REPLACE           234
#define ID_XFDI_CLASH_APPEND            235
#define ID_XFDI_CLASH_RENAMEOLD         236
#define ID_XFDI_CLASH_RENAMEOLDTXT      237
#define ID_XFDI_CLASH_DATEOLD           238
#define ID_XFDI_CLASH_TIMEOLD           239
#define ID_XFDI_CLASH_SIZEOLD           240
#define ID_XFDI_CLASH_DATENEW           241
#define ID_XFDI_CLASH_TIMENEW           242
#define ID_XFDI_CLASH_SIZENEW           243
#define ID_XFDI_CLASH_OLDOBJECT         244
#define ID_XFDI_CLASH_NEWOBJECT         245

// bootup status
#define ID_XFD_BOOTUPSTATUS             250
#define ID_XFD_ARCHIVINGSTATUS          251
#define ID_XFDI_BOOTUPSTATUSTEXT        252

/* Dialog box templates */
// #define ID_XFD_NOCONFIG                 270      removed V0.9.18 (2002-02-06) [umoeller]
// #define ID_XFD_NOOBJECT                 271      removed V0.9.12 (2001-05-18) [umoeller]
#define ID_XFD_LIMITREACHED             272
// #define ID_XFD_WRONGVERSION             273      removed V0.9.19 (2002-04-24) [umoeller]

// #define ID_XFD_NOTEXTCLIP               280      removed V0.9.19 (2002-04-24) [umoeller]
// #define ID_XFD_NOICONVIEW               281      removed V0.9.2 (2000-03-04) [umoeller]

// #define ID_XFD_WELCOME                  290      removed V0.9.16 (2001-11-10) [umoeller]
#define ID_XFD_CREATINGCONFIG           291

#define ID_XFD_STARTUPSTATUS            292

#define ID_XFD_FILEOPSSTATUS            293
#define ID_XSDI_SOURCEFOLDER            294
#define ID_XSDI_SOURCEOBJECT            295
#define ID_XSDI_SUBOBJECT               296
#define ID_XSDI_TARGETFOLDER            297
#define ID_XSDI_TARGETFOLDER_TXT        298

// generic container page
#define ID_XFD_CONTAINERPAGE            300
#define ID_XFDI_CNR_CNR                 301
#define ID_XFDI_CNR_GROUPTITLE          302

// "Run" dialog V0.9.9 (2001-03-07) [umoeller]
// #define ID_XFD_RUN                      350      removed V0.9.19 (2002-04-25) [umoeller]
#define ID_XFD_RUN_TITLE                348
#define ID_XFD_RUN_INTRO                349
#define ID_XFD_RUN_STARTUPDIR_TXT       350
#define ID_XFD_RUN_COMMAND              351
#define ID_XFD_RUN_STARTUPDIR           352
#define ID_XFD_RUN_FULLPATH             353
#define ID_XFD_RUN_MINIMIZED            354
#define ID_XFD_RUN_AUTOCLOSE            355
// V0.9.14 (2001-08-18) [pr]
#define ID_XFD_RUN_FULLSCREEN           356
#define ID_XFD_RUN_ENHANCED             357
#define ID_XFD_RUN_SEPARATE             358
// #define ID_XFD_RUN_BROWSE               359      removed V0.9.19 (2002-04-25) [umoeller]
#define ID_XFD_RUN_WINOS2_GROUP         360

// "Startup panic" dialog, V0.9.0
// all IDs raised with V0.9.16 (2001-10-08) [umoeller]
// #define ID_XFD_STARTUPPANIC             260
#define ID_XFDI_PANIC_TITLE             400     // V0.9.16 (2001-10-08) [umoeller]
#ifndef __NOBOOTLOGO__
#define ID_XFDI_PANIC_SKIPBOOTLOGO      401
#endif
#ifndef __NOXWPSTARTUP__
#define ID_XFDI_PANIC_SKIPXFLDSTARTUP   402
#endif
#ifndef __NOQUICKOPEN__
#define ID_XFDI_PANIC_SKIPQUICKOPEN     403
#endif
#define ID_XFDI_PANIC_NOARCHIVING       404
#define ID_XFDI_PANIC_DISABLEFEATURES   405
#ifndef __NOICONREPLACEMENTS__
#define ID_XFDI_PANIC_DISABLEREPLICONS  406
#endif
#define ID_XFDI_PANIC_REMOVEHOTKEYS     407
#define ID_XFDI_PANIC_DISABLEPAGER   408
#define ID_XFDI_PANIC_DISABLEMULTIMEDIA 409
#define ID_XFDI_PANIC_CONTINUE          410     // V0.9.16 (2001-10-08) [umoeller]
#define ID_XFDI_PANIC_CONTINUE_TXT      411     // V0.9.16 (2001-10-08) [umoeller]
#define ID_XFDI_PANIC_XFIX              412     // V0.9.16 (2001-10-08) [umoeller]
#define ID_XFDI_PANIC_XFIX_TXT          413     // V0.9.16 (2001-10-08) [umoeller]
#define ID_XFDI_PANIC_CMD               414     // V0.9.16 (2001-10-08) [umoeller]
#define ID_XFDI_PANIC_CMD_TXT           415     // V0.9.16 (2001-10-08) [umoeller]
#define ID_XFDI_PANIC_SHUTDOWN          416     // V0.9.16 (2001-10-08) [umoeller]
#define ID_XFDI_PANIC_SHUTDOWN_TXT      417     // V0.9.16 (2001-10-08) [umoeller]
#define ID_XFDI_PANIC_DISABLEREPLREFRESH 418    // V0.9.16 (2001-10-08) [umoeller]
#ifndef __NOTURBOFOLDERS__
#define ID_XFDI_PANIC_DISABLETURBOFOLDERS 419   // V0.9.16 (2001-10-25) [umoeller]
#endif
#ifndef __NEVERCHECKDESKTOP__
#define ID_XFDI_PANIC_DISABLECHECKDESKTOP   420    // V0.9.17 (2002-02-05) [umoeller]
#endif

#define ID_XSD_STARTUPFOLDER        442     // "Startup" page in XFldStartup;
                                            // new with V0.9.0

/* ******************************************************************
 *
 *   XFldWPS                        >= 450
 *
 ********************************************************************/

// all the following are new with V0.9.19 (2002-04-17) [umoeller]
#define ID_XSDI_MENU_SETTINGS           450
#define ID_XSDI_MENU_ITEMS              451

#define ID_XSDI_MENU_STYLE_GROUP        452
#define ID_XSDI_MENUS_BARS              453
#define ID_XSDI_MENUS_SHORT             454
#define ID_XSDI_MENUS_LONG              455
#define ID_XSDI_LOCKINPLACE_NOSUB       456

#define ID_XSDI_MENU_EDIT_GROUP         457
#define ID_XSDI_MENU_EDIT_CNR           458
#define ID_XSDI_MENU_EDIT_CAT_TXT       459
#define ID_XSDI_MENU_EDIT_CAT_DROP      460

#define ID_XSDI_MENU_EDIT_CAT_FOLDERS   461
#define ID_XSDI_MENU_EDIT_CAT_DESKTOP   462
#define ID_XSDI_MENU_EDIT_CAT_DISKS     463
#define ID_XSDI_MENU_EDIT_CAT_FILES     464
#define ID_XSDI_MENU_EDIT_CAT_OBJECTS   465

#define ID_XSDI_MENU_MENUSTRING         467

#define ID_XSDI_MENU_OPENAS             469
#define ID_XSDI_MENU_PROPERTIES         470
#define ID_XSDI_MENU_OPENPARENT         471
#define ID_XSDI_MENU_CREATEANOTHER      472
#define ID_XSDI_MENU_MOVE               473
#define ID_XSDI_MENU_COPY               474
#define ID_XSDI_MENU_PASTE              475
#define ID_XSDI_MENU_CREATESHADOW       476
#define ID_XSDI_MENU_DELETE             477
#define ID_XSDI_MENU_PICKUP             478
#define ID_XSDI_MENU_FIND               479
#define ID_XSDI_MENU_VIEW               480
#define ID_XSDI_MENU_ICONVIEW           481
#define ID_XSDI_MENU_TREEVIEW           482
#define ID_XSDI_MENU_DETAILSVIEW        483
#define ID_XSDI_MENU_SORT               484
#define ID_XSDI_MENU_ARRANGE            485
#define ID_XSDI_MENU_PRINT              486
#define ID_XSDI_MENU_LOCKUP             487
#define ID_XSDI_MENU_LOGOFFNETWORKNOW   488
#define ID_XSDI_MENU_SHUTDOWN           489
#define ID_XSDI_MENU_SYSTEMSETUP        490
#define ID_XSDI_MENU_CHKDSK             491
#define ID_XSDI_MENU_FORMAT             492
#define ID_XSDI_MENU_COPYDSK            493
#define ID_XSDI_MENU_LOCKDISK           494
#define ID_XSDI_MENU_EJECTDISK          495
#define ID_XSDI_MENU_UNLOCKDISK         496
#define ID_XSDI_MENU_BATCHRENAME        497         // V0.9.19 (2002-06-18) [umoeller]

// new menu items
// #define ID_XSD_SET25ADDMENUS            530      // removed V0.9.16 (2001-09-29) [umoeller]
// #define ID_XSDI_FILEMENUS_GROUP         529     // removed V0.9.19 (2002-04-17) [umoeller]
// #define ID_XSDI_MOVE4REFRESH            533      // removed V0.9.19 (2002-04-17) [umoeller]
#define ID_XSDI_FLDRVIEWS               535
#ifndef __NOFOLDERCONTENTS__
#define ID_XSDI_FC_SHOWICONS            537
#endif
// #define ID_XSDI_FOLDERMENUS_GROUP       539      // removed V0.9.19 (2002-04-17) [umoeller]

// "XFolder Internals": removed with V0.9.0

// "Config folder menu items" page
// #define ID_XSD_SET26CONFIGMENUS         540      removed V0.9.19 (2002-04-24) [umoeller]
#define ID_XSDI_MENUS_CONFIGFDR_GROUP   540
#define ID_XSDI_CASCADE                 541
#define ID_XSDI_REMOVEX                 542
#define ID_XSDI_APPDPARAM               543

#define ID_XSDI_TPL_DONOTHING           544
#define ID_XSDI_TPL_EDITTITLE           545
#define ID_XSDI_TPL_OPENSETTINGS        546
#define ID_XSDI_TPL_POSITION            547

#define ID_XSDI_TPL_GROUP               548     // V0.9.19 (2002-04-24) [umoeller]

// "snap to grid" page
#ifndef __NOSNAPTOGRID__
#define ID_XSD_SET3SNAPTOGRID           550
#define ID_XSDI_SNAPTOGRID              551     // changed V0.9.12, these were duplicates
#define ID_XSDI_GRID_X                  552     // changed V0.9.12, these were duplicates
#define ID_XSDI_GRID_Y                  553     // changed V0.9.12, these were duplicates
#define ID_XSDI_GRID_CX                 554     // changed V0.9.12, these were duplicates
#define ID_XSDI_GRID_CY                 555     // changed V0.9.12, these were duplicates
#endif

// "folder hotkeys" page
#define ID_XSD_SET4ACCELS               560
#define ID_XSDI_ACCELERATORS            561
#define ID_XSDI_HOTKEYS_GROUP           562
#define ID_XSDI_CNR                     563
//#define ID_XSDI_DESCRIPTION_TX1         564     // text
//#define ID_XSDI_CLEARACCEL              565
#define ID_XSDI_SHOWINMENUS             566     // V0.9.2 (2000-03-08) [umoeller]
//#define ID_XSDI_SETACCEL                567     // V0.9.9 (2001-04-04) [umoeller]

// "status bars" page 1
// #define ID_XSD_SET27STATUSBARS          570  // removed V0.9.19 (2002-04-24) [umoeller]
#define ID_XSDI_STYLE_GROUP             569     // added V0.9.19 (2002-04-24) [umoeller]
#define ID_XSDI_VISIBLEIN_GROUP         570     // added V0.9.19 (2002-04-24) [umoeller]
#define ID_XSDI_ENABLESTATUSBAR         571
#define ID_XSDI_SBFORICONVIEWS          572
#define ID_XSDI_SBFORTREEVIEWS          573
#define ID_XSDI_SBFORDETAILSVIEWS       574
#define ID_XSDI_SBSTYLE_3RAISED         575
#define ID_XSDI_SBSTYLE_3SUNKEN         576
#define ID_XSDI_SBSTYLE_4RECT           577
#define ID_XSDI_SBSTYLE_4MENU           578
#define ID_XSDI_SBFORSPLITVIEWS         579     // added V1.0.1 (2002-11-30) [umoeller]

// "status bars" page 2
#ifndef __NOCFGSTATUSBARS__
#define ID_XSD_SET28STATUSBARS2         580
#define ID_XSDI_SBTEXTNONESEL           581
#define ID_XSDI_SBCURCLASS              582
#define ID_XSDI_SBSELECTCLASS           583
#define ID_XSDI_SBTEXT1SEL              584
#define ID_XSDI_SBTEXTMULTISEL          585
#define ID_XSDI_DEREFSHADOWS_SINGLE     586     // new V0.9.5 (2000-10-07) [umoeller]
#define ID_XSDI_DEREFSHADOWS_MULTIPLE   587     // new V0.9.5 (2000-10-07) [umoeller]
#define ID_XSDI_SBKEYSNONESEL           595
#define ID_XSDI_SBKEYS1SEL              596
#define ID_XSDI_SBKEYSMULTISEL          597
#endif

// extended "sort" page
#define ID_XSD_SETTINGS_FLDRSORT        590
#define ID_XSDI_ALWAYSSORT              591
#define ID_XSDI_SORTLISTBOX             592
// #define ID_XSDI_REPLACESORT      675    // removed (V0.9.0)
// #define ID_XSDI_SORTTEXT                593      // removed V0.9.12 (2001-05-19) [umoeller]
#define ID_XSDI_SORTFOLDERSFIRST        594     // added V0.9.12 (2001-05-19) [umoeller]

// 595-597 used above

// "File types" page in "Workplace Shell" (new with V0.9.0)
// #define ID_XSD_FILETYPES                600     // removed V0.9.20 (2002-08-04) [umoeller]
#define ID_XSDI_FT_GROUP                601
#define ID_XSDI_FT_CONTAINER            602
#define ID_XSDI_FT_FILTERS_TXT          603
#define ID_XSDI_FT_FILTERSCNR           604
#define ID_XSDI_FT_ASSOCS_TXT           605
#define ID_XSDI_FT_ASSOCSCNR            606
#define ID_XSDI_FT_CREATEDATAFILEHANDLE 607     // added V0.9.20 (2002-08-04) [umoeller]

// "Import WPS Filters" dialog (V0.9.0
#define ID_XSD_IMPORTWPS                610     // "Import WPS filter" dlg (V0.9.0)
#define ID_XSDI_FT_TYPE                 611
#define ID_XSDI_FT_FILTERLIST           612
#define ID_XSDI_FT_NEW                  613
#define ID_XSDI_FT_SELALL               614
#define ID_XSDI_FT_DESELALL             615
#define ID_XSDI_FT_UNKNOWNONLY          616

// "New file type" dialog (V0.9.0)
#define ID_XSD_NEWFILETYPE              620     // "New File type" dlg (V0.9.0)
#define ID_XSD_NEWFILTER                621     // "New Filter" dlg (V0.9.0)
#define ID_XSDI_FT_ENTRYFIELD           622
#define ID_XSDI_FT_TITLE                623

// "View" page (added V0.9.0)
// all IDs raised V0.9.16 (2001-10-11) [umoeller]
// #define ID_XSD_FOLDERVIEWS              650  // removed V0.9.16 (2001-10-11) [umoeller]
// #define ID_XSDI_ADDINTERNALS            501  // removed (V0.9.0)
// #define ID_XSDI_REPLICONS               501  // removed (V0.9.0)
#define ID_XSD_FOLDERVIEWGROUP          650
#define ID_XSDI_FULLPATH                651
#define ID_XSDI_KEEPTITLE               652
#define ID_XSDI_MAXPATHCHARS            653
#define ID_XSDI_MAXPATHCHARS_TX1        654
#define ID_XSDI_MAXPATHCHARS_TX2        655
#define ID_XSDI_TREEVIEWAUTOSCROLL      656
#ifndef __NOFDRDEFAULTDOCS__
#define ID_XSDI_FDRDEFAULTDOC           657
#define ID_XSDI_FDRDEFAULTDOCVIEW       658
#endif
#define ID_XSDI_FDRAUTOREFRESH          659     // added V0.9.9 (2001-02-06) [umoeller]
#define ID_XSDI_FDRVIEWDEFAULT_GROUP    660
#define ID_XSDI_FDRVIEW_INHERIT         661     // added V0.9.12 (2001-04-30) [umoeller]
#define ID_XSDI_FDRVIEW_ICON            662
#define ID_XSDI_FDRVIEW_TREE            663
#define ID_XSDI_FDRVIEW_DETAILS         664
#define ID_XSDI_FDRVIEW_LAZYICONS       665     // added V0.9.20 (2002-08-04) [umoeller]
#define ID_XSDI_FDRVIEW_SHADOWOVERLAY   666     // added V0.9.20 (2002-08-04) [umoeller]
#define ID_XSDI_FDRVIEW_THUMBNAILS      667     // added V1.0.1 (2003-01-31) [umoeller]

// "Add/Edit hotkey" dialog (added V1.0.0) [lafaix]
#define ID_XSDI_HOTKEY_ADDTITLE         670
#define ID_XSDI_HOTKEY_EDITTITLE        671
#define ID_XSDI_HOTKEY_HOTKEY           672
#define ID_XSDI_HOTKEY_ACTION           673
#define ID_XSDI_HOTKEY_PARAM            674
#define ID_XSDI_HOTKEY_ACTION_DROP      675
#define ID_XSDI_HOTKEY_PARAM_DROP       676
#define ID_XSDI_HOTKEY_HOTKEY_EF        677
#define ID_XSDI_HOTKEY_GROUP            678

// tool bars page V1.0.1 (2002-12-08) [umoeller]
#define ID_XSD_SETTOOLBARS1             680
#define ID_XSDI_ENABLETOOLBAR           681
#define ID_XSDI_TBSTYLE_TEXT            682
#define ID_XSDI_TBSTYLE_MINI            683
#define ID_XSDI_TBSTYLE_FLAT            684
#define ID_XSDI_TBSTYLE_HILITE          685
#define ID_XSDI_TBSTYLE_TOOLTIPS        686

/* ******************************************************************
 *
 *   Instance settings notebooks        >= 700
 *
 ********************************************************************/

// XFolder page in folder notebook
// (also uses some ID's def'd above)
// #define ID_XSD_SETTINGS_FLDR1           710      // removed V0.9.16 (2001-09-29) [umoeller]
#define ID_XSSI_GENERALVIEWPAGE         710     // V0.9.16 (2001-10-23) [umoeller]
#ifndef __NOFOLDERCONTENTS__
#define ID_XSDI_FAVORITEFOLDER          711
#endif
#ifndef __NOQUICKOPEN__
#define ID_XSDI_QUICKOPEN               712
#endif

// "Internals" page in all object notebooks
/* #define ID_XSD_OBJECTDETAILS            720
#define ID_XSDI_DTL_CNR                 721
#define ID_XSDI_DTL_HOTKEY              722
#define ID_XSDI_DTL_HOTKEY_TXT          723
#define ID_XSDI_DTL_CLEAR               724
#define ID_XSDI_DTL_SETUP_ENTRY         725
#define ID_XSDI_DTL_SET                 726
*/  // all removed V0.9.16 (2001-10-15) [umoeller]

// "File" page replacement (XFolder, XFldDataFile, V0.9.0)
// #define ID_XSD_FILESPAGE1               730      // removed V0.9.19 (2002-04-13) [umoeller]
#define ID_XSDI_FILES_REALNAME              731
#define ID_XSDI_FILES_CREATIONDATE          732
#define ID_XSDI_FILES_CREATIONTIME          733
#define ID_XSDI_FILES_LASTWRITEDATE         734
#define ID_XSDI_FILES_LASTWRITETIME         735
#define ID_XSDI_FILES_LASTACCESSDATE        736
#define ID_XSDI_FILES_LASTACCESSTIME        737
#define ID_XSDI_FILES_ATTR_ARCHIVED         738
#define ID_XSDI_FILES_ATTR_READONLY         739
#define ID_XSDI_FILES_ATTR_HIDDEN           740
#define ID_XSDI_FILES_ATTR_SYSTEM           741
#define ID_XSDI_FILES_SUBJECT               742
#define ID_XSDI_FILES_COMMENTS              743
#define ID_XSDI_FILES_KEYPHRASES            744
#define ID_XSDI_FILES_VERSION               745
#define ID_XSDI_FILES_FILESIZE              746
#define ID_XSDI_FILES_EASIZE                747
#define ID_XSDI_FILES_WORKAREA              748     // added V0.9.1 (99-12-20) [umoeller]
// the following on this page added with V0.9.19 (2002-04-13) [umoeller]
#define ID_XSDI_FILES_REALNAME_TXT          749
#define ID_XSDI_FILES_FILESIZE_TXT          750
#define ID_XSDI_FILES_DATETIME_GROUP        751
#define ID_XSDI_FILES_CREATIONDATE_TXT      752
#define ID_XSDI_FILES_LASTWRITEDATE_TXT     753
#define ID_XSDI_FILES_LASTACCESSDATE_TXT    754
#define ID_XSDI_FILES_ATTR_GROUP            755
#define ID_XSDI_FILES_INFO_GROUP            756
#define ID_XSDI_FILES_SUBJECT_TXT           757
#define ID_XSDI_FILES_COMMENTS_TXT          758
#define ID_XSDI_FILES_KEYPHRASES_TXT        759

#ifndef __NOFILEPAGE2__
#define ID_XSD_FILESPAGE2               760
#define ID_XSDI_FILES_EALIST            761
#define ID_XSDI_FILES_EAINFO            762
#define ID_XSDI_FILES_EACONTENTS        763
#endif

// "Details" page replacement (XFldDisk, V0.9.0)
#define ID_XSD_DISK_DETAILS             770
#define ID_XSDI_DISK_LABEL              771
#define ID_XSDI_DISK_FILESYSTEM         772
#define ID_XSDI_DISK_SECTORSIZE         775
#define ID_XSDI_DISK_TOTAL_SECTORS      776
#define ID_XSDI_DISK_TOTAL_BYTES        777
#define ID_XSDI_DISK_ALLOCATED_SECTORS  778
#define ID_XSDI_DISK_ALLOCATED_BYTES    779
#define ID_XSDI_DISK_AVAILABLE_SECTORS  780
#define ID_XSDI_DISK_AVAILABLE_BYTES    781
#define ID_XSDI_DISK_CHART              782

// "Module" page (XWPProgramFile, V0.9.0)
// all changed, we had duplicates, and new items added
// V0.9.12 (2001-05-19) [umoeller]
#define ID_XSD_PGMFILE_MODULE           790
#define ID_XSDI_PROG_MAINGROUP          791
#define ID_XSDI_PROG_FILENAME           792
#define ID_XSDI_PROG_FILENAME_TXT       793
#define ID_XSDI_PROG_FORMAT_GROUP       794
#define ID_XSDI_PROG_EXEFORMAT          795
#define ID_XSDI_PROG_EXEFORMAT_TXT      796
#define ID_XSDI_PROG_TARGETOS           797
#define ID_XSDI_PROG_TARGETOS_TXT       798
#define ID_XSDI_PROG_DESCRIPTION_GROUP  799
#define ID_XSDI_PROG_DESCRIPTION        800

// "Types" page in XFldDataFile V0.9.9 (2001-03-27) [umoeller]
#define ID_XSD_DATAF_TYPES              820
#define ID_XSDI_DATAF_AVAILABLE_TXT     821
#define ID_XSDI_DATAF_AVAILABLE_CNR     822
#define ID_XSDI_DATAF_GROUP             825

// new "Icon" page for XFldObject V0.9.16 (2001-10-15) [umoeller]
#define ID_XSDI_ICON_TITLE_TEXT         840
#define ID_XSDI_ICON_TITLE_EF           841
#define ID_XSDI_ICON_GROUP              842
#define ID_XSDI_ICON_STATIC             843
#define ID_XSDI_ICON_EXPLANATION_TXT    844
#define ID_XSDI_ICON_EDIT_BUTTON        845
#define ID_XSDI_ICON_RESET_BUTTON       846
#define ID_XSDI_ICON_LOCKPOSITION_CB    847
#define ID_XSDI_ICON_TEMPLATE_CB        848
#define ID_XSDI_ICON_HOTKEY_TEXT        849
#define ID_XSDI_ICON_HOTKEY_EF          850
#define ID_XSDI_ICON_HOTKEY_CLEAR       851
#define ID_XSDI_ICON_HOTKEY_SET         852
#define ID_XSDI_ICON_EXTRAS_GROUP       853

// object "Details" dialog V0.9.16 (2001-10-15) [umoeller]
#define ID_XSDI_DETAILS_DIALOG          880
#define ID_XSDI_DETAILS_GROUP           881
#define ID_XSDI_DETAILS_CONTAINER       882
#define ID_XSDI_DETAILS_SETUPSTR_GROUP  883
#define ID_XSDI_DETAILS_SETUPSTR_EF     884

/* ******************************************************************
 *
 *   XWPSound                           >= 1000
 *
 ********************************************************************/

#define ID_XSD_XWPSOUND                 1000    // new "Sounds" page (XWPSound, V0.9.0)
#define ID_XSD_NEWSOUNDSCHEME           1001    // "New Sound Scheme" dlg, V0.9.0

#define ID_XSDI_SOUND_ENABLE            1002
#define ID_XSDI_SOUND_SCHEMES_DROPDOWN  1003
#define ID_XSDI_SOUND_SCHEMES_SAVEAS    1004
#define ID_XSDI_SOUND_SCHEMES_DELETE    1005
#define ID_XSDI_SOUND_EVENTSLISTBOX     1006
#define ID_XSDI_SOUND_FILE              1007
#define ID_XSDI_SOUND_BROWSE            1008
#define ID_XSDI_SOUND_PLAY              1009
#define ID_XSDI_SOUND_COMMONVOLUME      1010
#define ID_XSDI_SOUND_VOLUMELEVER       1011

/* ******************************************************************
 *
 *   XWPSetup                           >= 1100
 *
 ********************************************************************/

// XWPSetup info page (V0.9.0)
// all IDs changed V0.9.9 (2001-03-07) [umoeller]
// NOOOO... these IDs must NEVER be changed, or the
// whole system will blow up if the NLS DLL is changed
// to an outdated version... going back to old codes
// V0.9.9 (2001-04-04) [umoeller]
#define ID_XCD_STATUS                   1100
#define ID_XCDI_INFO_KERNEL_RELEASE     1101
// #define ID_XCDI_INFO_KERNEL_BUILD       1102     removed V0.9.2 (2000-02-20) [umoeller]
// #define ID_XCDI_INFO_KERNEL_LOCALE      1103
#define ID_XCDI_INFO_AWAKEOBJECTS       1103
#define ID_XCDI_INFO_WPSTHREADS         1104
#define ID_XCDI_INFO_WPSRESTARTS        1105
// #define ID_XCDI_INFO_WORKERSTATUS       1106     removed V0.9.9 (2001-03-07)
// #define ID_XCDI_INFO_FILESTATUS         1107     removed V0.9.9 (2001-03-07)
// #define ID_XCDI_INFO_QUICKSTATUS        1108     removed V0.9.9 (2001-03-07)
#define ID_XCDI_INFO_SOUNDSTATUS        1109
#define ID_XCDI_INFO_HOOKSTATUS         1110
#define ID_XCDI_INFO_LANGUAGE           1111
#define ID_XCDI_INFO_NLS_RELEASE        1112
#define ID_XCDI_INFO_NLS_AUTHOR         1113

// 1120 is occupied below... we can't change that V0.9.9 (2001-04-04) [umoeller]
#define ID_XCDI_INFO_KERNEL_GROUP       1121
#define ID_XCDI_INFO_KERNEL_RELEASE_TXT 1122
#define ID_XCDI_INFO_AWAKEOBJECTS_TXT   1123
#define ID_XCDI_INFO_WPSTHREADS_TXT     1124
#define ID_XCDI_INFO_WPSRESTARTS_TXT    1125
#define ID_XCDI_INFO_SOUNDSTATUS_TXT    1126
#define ID_XCDI_INFO_HOOKSTATUS_TXT     1127
#define ID_XCDI_INFO_LANGUAGE_GROUP     1129
#define ID_XCDI_INFO_LANGUAGE_TXT       1130
#define ID_XCDI_INFO_NLS_RELEASE_TXT    1131
#define ID_XCDI_INFO_NLS_AUTHOR_TXT     1132

// XWPSetup "Features" page (V0.9.0)
#define ID_XCD_FEATURES                 1120
#define ID_XCDI_CONTAINER               1140
#define ID_XCDI_SETUP                   1141

// XWPSetup first page V0.9.6 (2000-11-04) [umoeller]
#define ID_XCD_FIRST                    1150


// XWPSetup "File operations" page (V0.9.0)
// #define ID_XCD_FILEOPS                  1150
// #define ID_XCDI_EXTASSOCS               1151
// #define ID_XCDI_IGNOREFILTERS           1152
// #define ID_XCDI_CLEANUPINIS             1153
// #define ID_XCDI_REPLFILEEXISTS          1154
// #define ID_XCDI_REPLDRIVENOTREADY       1155

// XWPSetup "Paranoia" page (V0.9.0)
// #define ID_XCD_PARANOIA                 1160 removed V0.9.19 (2002-04-17) [umoeller]
#define ID_XCDI_PARANOIA_GROUP          1158
#define ID_XCDI_PARANOIA_INTRO          1159
#define ID_XCDI_VARMENUOFFSET_TXT       1160
#define ID_XCDI_VARMENUOFFSET           1161
// #define ID_XCDI_NOFREAKYMENUS           1162     removed V1.0.0 (2002-08-26) [umoeller]
// #ifndef __ALWAYSSUBCLASS__
// #define ID_XCDI_NOSUBCLASSING           1163     removed V1.0.0 (2002-09-12) [umoeller]
// #endif
// #define ID_XCDI_NOWORKERTHREAD          1164
        // removed this setting V0.9.16 (2001-01-04) [umoeller]
#define ID_XCDI_USE8HELVFONT            1165
#define ID_XCDI_NOEXCPTBEEPS            1166
#define ID_XCDI_WORKERPRTY_SLIDER       1167
#define ID_XCDI_WORKERPRTY_BEEP         1168
// the following two are for the descriptive texts;
// they must have higher IDs, or the help panels won't work
#define ID_XCDI_WORKERPRTY_TEXT1        1169
#define ID_XCDI_WORKERPRTY_TEXT2        1170
#define ID_XCDI_WORKERPRTY_GROUP        1171        // V0.9.19 (2002-04-17) [umoeller]

// XWPSetup "Objects" page (V0.9.0)
#define ID_XCD_OBJECTS                  1180
#define ID_XCD_OBJECTS_SYSTEM           1181
#define ID_XCD_OBJECTS_XWORKPLACE       1182
#define ID_XCD_OBJECTS_CONFIGFOLDER     1183

// logo window
#define ID_XFDI_LOGOBITMAP              1190

// "Installed XWorkplace Classes" dlg
// all removed V0.9.14 (2001-07-31) [umoeller]
/* #define ID_XCD_XWPINSTALLEDCLASSES      1250

#define ID_XCDI_XWPCLS_XFLDOBJECT       1251
#define ID_XCDI_XWPCLS_XFOLDER          1252
#define ID_XCDI_XWPCLS_XFLDDISK         1253
#define ID_XCDI_XWPCLS_XFLDDESKTOP      1254
#define ID_XCDI_XWPCLS_XFLDDATAFILE     1255
#define ID_XCDI_XWPCLS_XFLDPROGRAMFILE  1256
#define ID_XCDI_XWPCLS_XWPSOUND         1257
#define ID_XCDI_XWPCLS_XWPMOUSE         1258
#define ID_XCDI_XWPCLS_XWPKEYBOARD      1259

#define ID_XCDI_XWPCLS_XWPSETUP         1260
#define ID_XCDI_XWPCLS_XFLDSYSTEM       1261
#define ID_XCDI_XWPCLS_XFLDWPS          1262
#define ID_XCDI_XWPCLS_XFLDSTARTUP      1263
#define ID_XCDI_XWPCLS_XFLDSHUTDOWN     1264
#define ID_XCDI_XWPCLS_XWPCLASSLIST     1265
#define ID_XCDI_XWPCLS_XWPTRASHCAN      1266

// new classes with V0.9.3
#define ID_XCDI_XWPCLS_XWPSCREEN        1267
#define ID_XCDI_XWPCLS_XWPSTRING        1268

// new classes with V0.9.4
#define ID_XCDI_XWPCLS_XWPMEDIA         1269

// new classes with V0.9.7
#define ID_XCDI_XWPCLS_XCENTER          1270

// new classes with V0.9.10
#define ID_XCDI_XWPCLS_XWPFONTS         1271
*/

// new codes V0.9.16 (2002-01-13) [umoeller]
#define ID_XCD_CLASSES_TITLE            1280
#define ID_XCD_CLASSES_REPLACEMENTS     1281
#define ID_XCD_CLASSES_NEW              1282

/* ******************************************************************
 *
 *   XFldSystem                         >= 1500
 *
 ********************************************************************/

#define ID_OSD_SETTINGS_KERNEL1         1500
#define ID_OSD_SETTINGS_KERNEL2         1501
#define ID_OSDI_CURRENTTHREADS          1502
#define ID_OSDI_MAXTHREADS              1503
#define ID_OSDI_MAXWAIT                 1504
#define ID_OSDI_PRIORITYDISKIO          1505
#define ID_OSDI_CURRENTSWAPSIZE         1506
#define ID_OSDI_PHYSICALMEMORY          1507
#define ID_OSDI_MINSWAPSIZE             1508
#define ID_OSDI_MINSWAPFREE             1509
#define ID_OSDI_SWAPPATH                1510
#define ID_OSDI_HIMEM_GROUP             1511
#define ID_OSDI_HIMEM_VALIMIT_TXT1      1512
#define ID_OSDI_HIMEM_VALIMIT_SLIDER    1513
#define ID_OSDI_HIMEM_VALIMIT_TXT2      1514
#define ID_OSDI_HIMEM_DLLBASINGOFF      1515
#define ID_OSDI_HIMEM_JAVAHIMEM         1516
#define ID_OSDI_HIMEM_EARLYMEMINIT      1517  // V1.0.8 (2008-03-04) [chennecke]
#define ID_OSDI_CURRENTPROCESSES        1518  // V1.0.8 (2008-03-16) [chennecke]
#define ID_OSDI_MAXPROCESSES            1519  // V1.0.8 (2008-03-16) [chennecke]

// FAT page; this is also used by the driver dialogs
#define ID_OSD_SETTINGS_FAT             1521
#define ID_OSDI_FSINSTALLED             1522
#define ID_OSDI_CACHESIZE               1523
#define ID_OSDI_CACHESIZE_TXT           1524
#define ID_OSDI_CACHESIZE_AUTO          1525
#define ID_OSDI_CACHE_THRESHOLD         1526
#define ID_OSDI_CACHE_THRESHOLD_TXT     1527
#define ID_OSDI_CACHE_LAZYWRITE         1528        // jfs too
#define ID_OSDI_CACHE_MAXAGE            1529        // jfs too
#define ID_OSDI_CACHE_BUFFERIDLE        1530        // jfs too
#define ID_OSDI_CACHE_DISKIDLE          1531
#define ID_OSDI_AUTOCHECK               1532        // jfs too
#define ID_OSDI_AUTOCHECK_PROPOSE       1533
#define ID_OSDI_HPFS386INI_GROUP        1534
#define ID_OSDI_HPFS386INI_CNR          1535
#define ID_OSDI_CACHE_SYNCHTIME         1536        // jfs, V0.9.13 (2001-06-27) [umoeller]

#define ID_OSD_SETTINGS_ERRORS          1540
#define ID_OSDI_AUTOFAIL                1541
#define ID_OSDI_SUPRESSPOPUPS           1542
#define ID_OSDI_SUPRESSP_DRIVE          1543
#define ID_OSDI_REIPL                   1544

#define ID_OSD_SETTINGS_WPS             1550
#define ID_OSDI_AUTO_PROGRAMS           1551
#define ID_OSDI_AUTO_TASKLIST           1552
#define ID_OSDI_AUTO_CONNECTIONS        1553
#define ID_OSDI_AUTO_LAUNCHPAD          1554
#define ID_OSDI_AUTO_WARPCENTER         1555
#define ID_OSDI_RESTART_YES             1556
#define ID_OSDI_RESTART_NO              1557
#define ID_OSDI_RESTART_FOLDERS         1558
#define ID_OSDI_RESTART_REBOOT          1559
#define ID_OSDI_AUTOREFRESHFOLDERS      1560
#define ID_OSDI_SHELLHANDLESINC         1561  // V1.0.8 (2008-03-21) [chennecke]

// "System paths" page (V0.9.0)
#define ID_OSD_SETTINGS_SYSPATHS        1570
#define ID_OSD_NEWSYSPATH               1571   // "New System Path" dlg
#define ID_OSDI_PATHDROPDOWN            1572
#define ID_OSDI_PATHLISTBOX             1573
#define ID_OSDI_PATHNEW                 1574
#define ID_OSDI_PATHDELETE              1575
#define ID_OSDI_PATHEDIT                1576
#define ID_OSDI_PATHUP                  1577
#define ID_OSDI_PATHDOWN                1578
#define ID_OSDI_VALIDATE                1579
#define ID_OSDI_PATHINFOTXT             1580
#define ID_OSDI_DOUBLEFILES             1581

#define ID_OSD_FILELIST                 1582
#define ID_OSDI_FILELISTSYSPATH1        1583
#define ID_OSDI_FILELISTSYSPATH2        1584
#define ID_OSDI_FILELISTCNR             1585

// "Drivers" page (V0.9.0)
#define ID_OSD_SETTINGS_DRIVERS         1590
#define ID_OSDI_DRIVR_CNR               1591
#define ID_OSDI_DRIVR_STATICDATA        1592
#define ID_OSDI_DRIVR_PARAMS            1593
#define ID_OSDI_DRIVR_CONFIGURE         1594
#define ID_OSDI_DRIVR_APPLYTHIS         1595 // "Apply", non-notebook button
#define ID_OSDI_DRIVR_GROUP1            1596
#define ID_OSDI_DRIVR_GROUP2            1597
#define ID_OSDI_DRIVR_PARAMS_TXT        1598

#define ID_OSD_DRIVER_HPFS386           1600
#define ID_OSD_DRIVER_FAT               1601
#define ID_OSD_DRIVER_HPFS              1602

#define ID_OSD_DRIVER_CDFS              1610
#define ID_OSDI_CDFS_JOLIET             1611
#define ID_OSDI_CDFS_KANJI              1612
#define ID_OSDI_CDFS_CACHESLIDER        1613
#define ID_OSDI_CDFS_CACHETXT           1614
#define ID_OSDI_CDFS_SECTORSSLIDER      1615
#define ID_OSDI_CDFS_SECTORSTXT         1616
#define ID_OSDI_CDFS_INITDEFAULT        1617
#define ID_OSDI_CDFS_INITQUIET          1618
#define ID_OSDI_CDFS_INITVERBOSE        1619

// "IBM1S506" dialog; do not modify these IDs, because
// the TMF file uses these too
#define ID_OSD_DRIVER_IBM1S506          1620
#define ID_OSDI_S506_INITQUIET          1621
#define ID_OSDI_S506_INITVERBOSE        1622
#define ID_OSDI_S506_INITWAIT           1623
#define ID_OSDI_S506_DSG                1624
#define ID_OSDI_S506_ADAPTER0           1625
#define ID_OSDI_S506_ADAPTER1           1626
#define ID_OSDI_S506_A_IGNORE           1627
#define ID_OSDI_S506_A_RESET            1628
#define ID_OSDI_S506_A_BASEADDR_CHECK   1629
#define ID_OSDI_S506_A_BASEADDR_ENTRY   1630
#define ID_OSDI_S506_A_IRQ_CHECK        1631
#define ID_OSDI_S506_A_IRQ_SLIDER       1632
#define ID_OSDI_S506_A_IRQ_TXT          1633
#define ID_OSDI_S506_A_DMA_CHECK        1634
#define ID_OSDI_S506_A_DMA_SPIN         1635
#define ID_OSDI_S506_A_DSGADDR_CHECK    1636
#define ID_OSDI_S506_A_DSGADDR_ENTRY    1637
#define ID_OSDI_S506_A_BUSMASTER        1638
#define ID_OSDI_S506_UNIT0              1639
#define ID_OSDI_S506_UNIT1              1640
#define ID_OSDI_S506_UNIT2              1641
#define ID_OSDI_S506_UNIT3              1642
#define ID_OSDI_S506_U_BUSMASTER        1643
#define ID_OSDI_S506_U_RECOVERY_CHECK   1644
#define ID_OSDI_S506_U_RECOVERY_SLIDER  1645
#define ID_OSDI_S506_U_RECOVERY_TXT     1646
#define ID_OSDI_S506_U_GEO_CHECK        1647
#define ID_OSDI_S506_U_GEO_ENTRY        1648
#define ID_OSDI_S506_U_SMS              1652
#define ID_OSDI_S506_U_LBA              1653
#define ID_OSDI_S506_U_DASD             1654
#define ID_OSDI_S506_U_FORCE            1655
#define ID_OSDI_S506_U_ATAPI            1656
#define ID_OSDI_S506_NEWPARAMS          1657

#define ID_OSDI_DANIS506_CLOCK_CHECK        1660
#define ID_OSDI_DANIS506_CLOCK_SLIDER       1661
#define ID_OSDI_DANIS506_CLOCK_TXT          1662
#define ID_OSDI_DANIS506_GBM                1663
#define ID_OSDI_DANIS506_FORCEGBM           1664
#define ID_OSDI_DANIS506_MGAFIX             1665
#define ID_OSDI_DANIS506_U_TIMEOUT_CHECK    1666
#define ID_OSDI_DANIS506_U_TIMEOUT_SPIN     1667
#define ID_OSDI_DANIS506_U_RATE_CHECK       1668
#define ID_OSDI_DANIS506_U_RATE_UDMA_TXT    1669
#define ID_OSDI_DANIS506_U_RATE_UDMA_SPIN   1670
#define ID_OSDI_DANIS506_U_RATE_MWDMA_TXT   1671
#define ID_OSDI_DANIS506_U_RATE_MWDMA_SPIN  1672
#define ID_OSDI_DANIS506_U_RATE_PIO_TXT     1673
#define ID_OSDI_DANIS506_U_RATE_PIO_SPIN    1674
#define ID_OSDI_DANIS506_U_REMOVEABLE       1675

// syslevel page V0.9.2 (2000-03-08) [umoeller]
            // uses generic cnr page
// #define ID_OSD_SETTINGS_SYSLEVEL            1900
// #define ID_OSDI_SYSLEVEL_CNR                1901

/* ******************************************************************
 *
 *   XWPClassList                       >= 2000
 *
 ********************************************************************/

// class list dialog (left part of split view)
#define ID_XLD_CLASSLIST                2000

// class info dialog (top right part of split view)
#define ID_XLD_CLASSINFO                2001

// method info dialog (bottom right part of split view)
#define ID_XLD_METHODINFO               2002

// other dlgs
#define ID_XLD_SELECTCLASS              2003
#define ID_XLD_REGISTERCLASS            2004

// class list dlg items
#define ID_XLDI_BYTESPERINSTANCETXT     2008            // V0.9.20 (2002-08-04) [umoeller]
#define ID_XLDI_BYTESPERINSTANCE        2009            // V0.9.20 (2002-08-04) [umoeller]
#define ID_XLDI_INTROTEXT               2010
#define ID_XLDI_CNR                     2011
#define ID_XLDI_TEXT2                   2012
#define ID_XLDI_CLASSICON               2014
#define ID_XLDI_CLASSNAME               2015
#define ID_XLDI_REPLACEDBY              2016
#define ID_XLDI_CLASSTITLE              2017
#define ID_XLDI_CLASSMODULE             2018
#define ID_XLDI_CLASSNAMETXT            2019
#define ID_XLDI_REPLACEDBYTXT           2020
#define ID_XLDI_CLASSTITLETXT           2021
#define ID_XLDI_CLASSMODULETXT          2022
#define ID_XLDI_DLL                     2023
#define ID_XLDI_BROWSE                  2024
#define ID_XLDI_ICON                    2025
#define ID_XLDI_ICONTXT                 2026
#define ID_XLDI_RADIO_CLASSMETHODS      2027
#define ID_XLDI_RADIO_INSTANCEMETHODS   2028

// class list notebook settings page
#define ID_XLD_SETTINGS                 2030
#define ID_XLDI_SHOWSOMOBJECT           2031    // corresponds to IDL instance setting
#define ID_XLDI_SHOWMETHODS             2032    // corresponds to IDL instance setting

/* ******************************************************************
 *
 *   XFldDesktop                        >= 2100
 *
 ********************************************************************/

// "Menu items" page (V0.9.0)
// #define ID_XSD_DTP_MENUITEMS            2100     // removed V0.9.16 (2002-01-09) [umoeller]
// all removed V0.9.19 (2002-04-17) [umoeller]
// #define ID_XSDI_DTP_MENUITEMSGROUP      2100        // added V0.9.16 (2002-01-09) [umoeller]
// #define ID_XSDI_DTP_SORT                2101
// #define ID_XSDI_DTP_ARRANGE             2102
// #define ID_XSDI_DTP_SYSTEMSETUP         2103
// #define ID_XSDI_DTP_LOCKUP              2104
// #ifndef __NOXSHUTDOWN__
// #define ID_XSDI_DTP_SHUTDOWN            2105
// #define ID_XSDI_DTP_SHUTDOWNMENU        2106
// #endif
// #define ID_XSDI_DTP_LOGOFFNETWORKNOW    2107 // V0.9.7 (2000-12-13) [umoeller]

// XFldDesktop "Startup" page  (V0.9.0)
// #define ID_XSD_DTP_STARTUP              2110     // removed V0.9.16 (2001-10-08) [umoeller]
#ifndef __NOBOOTLOGO__
#define ID_XSDI_DTP_LOGOGROUP           2108    // added V0.9.16 (2001-10-08) [umoeller]
#define ID_XSDI_DTP_LOGOSTYLEGROUP      2109    // added V0.9.16 (2001-10-08) [umoeller]
#define ID_XSDI_DTP_BOOTLOGO            2111
#define ID_XSDI_DTP_LOGO_TRANSPARENT    2112
#define ID_XSDI_DTP_LOGO_BLOWUP         2113
#define ID_XSDI_DTP_LOGOFRAME           2114
#define ID_XSDI_DTP_LOGOBITMAP          2115
#define ID_XSDI_DTP_TESTLOGO            2116
#endif
#ifndef __NOXWPSTARTUP__
#define ID_XSDI_DTP_CREATESTARTUPFLDR   2117
#endif
#ifndef __NOBOOTLOGO__
#define ID_XSDI_DTP_LOGOFILETXT         2118
#define ID_XSDI_DTP_LOGOFILE            2119
// #define ID_XSDI_DTP_LOGO_BROWSE         2120     // removed V0.9.16 (2001-10-15) [umoeller]
#endif
#ifndef __NOBOOTUPSTATUS__
#define ID_XSDI_DTP_BOOTUPSTATUS        2121
#endif
#define ID_XSDI_DTP_NUMLOCKON           2122  // added V0.9.1 (2000-02-09) [umoeller]
#define ID_XSDI_DTP_WRITEXWPSTARTLOG    2123  // added V0.9.14 (2001-08-21) [umoeller]

// XFldDesktop "Archives" replacement page (V0.9.0)
#define ID_XSDI_ARC_RESTORE_GROUP       2150  // added V1.0.4 (2005-10-17) [bvl]
#define ID_XSDI_ARC_RESTORE_ALWAYS      2151  // added V1.0.4 (2005-10-17) [bvl]
#define ID_XSDI_ARC_RESTORE_TXT1        2152  // added V1.0.4 (2005-10-17) [bvl]
#define ID_XSDI_ARC_RESTORE_TXT2        2153  // added V1.0.4 (2005-10-17) [bvl]
#define ID_XSDI_ARC_RESTORE_SPIN        2154  // added V1.0.4 (2005-10-17) [bvl]
// #define ID_XSD_DTP_ARCHIVES             2155     // removed V0.9.16 (2001-11-22) [umoeller]
#define ID_XSDI_ARC_CRITERIA_GROUP      2155        // V0.9.16 (2001-11-22) [umoeller]
#define ID_XSDI_ARC_ENABLE              2156
#define ID_XSDI_ARC_ALWAYS              2157
#define ID_XSDI_ARC_NEXT                2158
#define ID_XSDI_ARC_INI                 2159
#define ID_XSDI_ARC_INI_SPIN            2160
#define ID_XSDI_ARC_INI_SPINTXT1        2161
#define ID_XSDI_ARC_DAYS                2162
#define ID_XSDI_ARC_DAYS_SPIN           2163
#define ID_XSDI_ARC_DAYS_SPINTXT1       2164
#define ID_XSDI_ARC_SHOWSTATUS          2165
#define ID_XSDI_ARC_ARCHIVES_GROUP      2166        // V0.9.16 (2001-11-22) [umoeller]
#define ID_XSDI_ARC_ARCHIVES_NO_TXT1    2167
#define ID_XSDI_ARC_ARCHIVES_NO_SPIN    2168
#define ID_XSDI_ARC_ARCHIVES_NO_TXT2    2169

// XFldDesktop "Shutdown" page (V0.9.0)
// all IDs raised V0.9.16 (2002-01-04) [umoeller]
#define ID_SDDI_ANIMATE_TXT             2199    // V0.9.19 (2002-06-18) [umoeller]
#ifndef __EASYSHUTDOWN__
#define ID_SDDI_REBOOT                  2200
#endif
#define ID_SDDI_ANIMATE_SHUTDOWN        2201
#define ID_SDDI_ANIMATE_REBOOT          2202
#define ID_SDDI_POWEROFF                2203        // V1.0.5 (2006-06-24) [pr]
#define ID_SDDI_DELAY                   2204
#ifndef __EASYSHUTDOWN__
#define ID_SDDI_CONFIRM                 2205
#define ID_SDDI_WARPCENTERFIRST         2206
#endif
#define ID_SDDI_AUTOCLOSEVIO            2207
#define ID_SDDI_LOG                     2208

#define ID_SDDI_CREATESHUTDOWNFLDR      2209
#define ID_SDDI_SHOWSTARTUPPROGRESS     2210

#define ID_SDDI_STARTUP_INITDELAY_TXT1  2211
#define ID_SDDI_STARTUP_INITDELAY_SLID  2212
#define ID_SDDI_STARTUP_INITDELAY_TXT2  2213
#define ID_SDDI_STARTUP_OBJDELAY_TXT1   2214
#define ID_SDDI_STARTUP_OBJDELAY_SLID   2215
#define ID_SDDI_STARTUP_OBJDELAY_TXT2   2216
#define ID_SDDI_STARTUP_REBOOTSONLY     2217
#define ID_SDDI_STARTUP_EVERYWPSRESTART 2218

#define ID_SDDI_REBOOTEXT               2219
#define ID_SDDI_AUTOCLOSEDETAILS        2220
#define ID_SDDI_APMVERSION              2221
#define ID_SDDI_APMVERSION_TXT          2222
#define ID_SDDI_ACPIVERSION             2223        // V1.0.5 (2006-06-24) [pr]
#define ID_SDDI_ACPIVERSION_TXT         2224        // V1.0.5 (2006-06-24) [pr]

#ifndef __EASYSHUTDOWN__
#define ID_SDDI_SAVEINIS_TXT            2225
#define ID_SDDI_SAVEINIS_LIST           2226
#endif

#define ID_SDDI_SHUTDOWNGROUP           2227        // V0.9.16 (2001-10-08) [umoeller]
#define ID_SDDI_SHAREDGROUP             2228

#define ID_SDDI_CANDESKTOPALTF4         2229        // V0.9.16 (2002-01-04) [umoeller]

#define ID_SDDI_APMPOWEROFF             2230        // V1.0.5 (2006-06-24) [pr]
#define ID_SDDI_ACPIPOWEROFF            2231        // V1.0.5 (2006-06-24) [pr]

// "extended reboot" / "auto-close" dlg items
// all IDs moved down here V1.0.2 (2003-12-03) [umoeller]
// #define ID_XSD_REBOOTEXT                2170
#define ID_XSD_REBOOTEXT_TITLE          2240        // changed V1.0.2 (2003-12-03) [umoeller]
#define ID_XSDI_XRB_LISTBOX             2241
#define ID_XSDI_XRB_NEW                 2242
#define ID_XSDI_XRB_DELETE              2243
#define ID_XSDI_XRB_UP                  2244
#define ID_XSDI_XRB_DOWN                2245
#define ID_XSDI_XRB_PARTITIONS          2246     // added  (V0.9.0)
#define ID_XSDI_XRB_INTRO               2247        // added V1.0.2 (2003-12-03) [umoeller]

#define ID_XSD_AUTOCLOSE                2248
#define ID_XSDI_ACL_WMCLOSE             2249
// #define ID_XSDI_ACL_CTRL_C              2182 removed V0.9.19 (2002-05-23) [umoeller]
#define ID_XSDI_ACL_KILLSESSION         2250
#define ID_XSDI_ACL_SKIP                2251

#define ID_XSDI_ACL_STORE               2252    // added V0.9.1 (99-12-10)
#define ID_XSDI_ACL_INTRO               2253
#define ID_XSDI_ACL_SESSIONTITLE        2254
#define ID_XSDI_ACL_DOWHATGROUP         2255

#define ID_XSDI_PARTITIONSFIRST         2256    // menu item of first submenu on "Partitions" button (V0.9.0)

#define ID_XSDI_XRB_ITEMNAME_TXT        2257
#define ID_XSDI_XRB_ITEMNAME            2258
#define ID_XSDI_XRB_COMMAND_TXT         2259
#define ID_XSDI_XRB_COMMAND             2260

/* ******************************************************************
 *
 *   XWPTrashCan                            >= 3000
 *
 ********************************************************************/

// #define ID_XTD_SETTINGS                 3000     // removed V0.9.19 (2002-04-14) [umoeller]
// #define ID_XTDI_DELETE                  3001
// #define ID_XTDI_EMPTYSTARTUP            3002
// #define ID_XTDI_EMPTYSHUTDOWN           3003
#define ID_XTDI_OPTIONSGROUP            3003        // V0.9.19 (2002-04-14) [umoeller]
#define ID_XTDI_CONFIRMEMPTY            3004
#define ID_XTDI_CONFIRMDESTROY          3005
#define ID_XTDI_ALWAYSTRUEDELETE        3006        // V0.9.19 (2002-04-14) [umoeller]

#define ID_XTD_DRIVES                   3020
#define ID_XTDI_UNSUPPORTED_LB          3021
#define ID_XTDI_SUPPORTED_LB            3022
#define ID_XTDI_ADD_SUPPORTED           3023
#define ID_XTDI_REMOVE_SUPPORTED        3024

#define ID_XTD_ICONPAGE                 3030
#define ID_XTDI_ICON_TITLEMLE           3031

/* ******************************************************************
 *
 *   XWPKeyboard                            >= 3200
 *
 ********************************************************************/

// #define ID_XSD_KEYB_OBJHOTKEYS          3200
// #define ID_XSDI_HOTK_CNR                3201

// #define ID_XSD_KEYB_FUNCTIONKEYS        3220
// #define ID_XSDI_FUNCK_CNR               3221

#define ID_XSD_KEYB_EDITFUNCTIONKEY     3230
#define ID_XSDI_FUNCK_DESCRIPTION_EF    3231
#define ID_XSDI_FUNCK_SCANCODE_EF       3232
// #define ID_XSDI_FUNCK_MODIFIER          3233
            // removed V0.9.12 (2001-05-17) [umoeller]

/* ******************************************************************
 *
 *   XWPMouse                               >= 3400
 *
 ********************************************************************/

// #define ID_XSD_MOUSE_MOVEMENT           3400     // V0.9.16 (2001-12-06) [umoeller]
#ifndef __NOSLIDINGFOCUS__
#define ID_XSDI_MOUSE_SLIDINGFOCUS_GRP  3400
#define ID_XSDI_MOUSE_SLIDINGFOCUS      3401
#define ID_XSDI_MOUSE_FOCUSDELAY_TXT1   3402
#define ID_XSDI_MOUSE_FOCUSDELAY_SLIDER 3403
#define ID_XSDI_MOUSE_FOCUSDELAY_TXT2   3404
#define ID_XSDI_MOUSE_BRING2TOP         3405
#define ID_XSDI_MOUSE_IGNORESEAMLESS    3406
#define ID_XSDI_MOUSE_IGNOREDESKTOP     3407
// #define ID_XSDI_MOUSE_IGNOREPAGER    3408    // removed V0.9.19 (2002-05-07) [umoeller]
// #define ID_XSDI_MOUSE_IGNOREXCENTER     3409    // V0.9.7 (2000-12-08) [umoeller]
                                                // removed V0.9.19 (2002-05-07) [umoeller]
#endif
// ID_XSDI_MOUSE_AUTOHIDE_* moved to ID_XSD_MOUSE_MOVEMENT2
// V0.9.14 (2001-08-02) [lafaix]
#define ID_XSDI_MOUSE_SLIDINGMENU_GRP   3413
#define ID_XSDI_MOUSE_SLIDINGMENU       3414
#define ID_XSDI_MOUSE_MENUDELAY_TXT1    3415
#define ID_XSDI_MOUSE_MENUDELAY_SLIDER  3416
#define ID_XSDI_MOUSE_MENUDELAY_TXT2    3417
#define ID_XSDI_MOUSE_MENUHILITE        3418
#define ID_XSDI_MOUSE_CONDCASCADE       3419    // V0.9.6 (2000-10-27) [umoeller]

#define ID_XSD_MOUSE_CORNERS            3430
#define ID_XSDI_MOUSE_RADIO_TOPLEFT     3431
#define ID_XSDI_MOUSE_RADIO_TOPRIGHT    3432
#define ID_XSDI_MOUSE_RADIO_BOTTOMLEFT  3433
#define ID_XSDI_MOUSE_RADIO_BOTTOMRIGHT 3434
#define ID_XSDI_MOUSE_RADIO_TOP         3435
#define ID_XSDI_MOUSE_RADIO_LEFT        3436
#define ID_XSDI_MOUSE_RADIO_RIGHT       3437
#define ID_XSDI_MOUSE_RADIO_BOTTOM      3438
#define ID_XSDI_MOUSE_INACTIVEOBJ       3439
#define ID_XSDI_MOUSE_SPECIAL_CHECK     3440
#define ID_XSDI_MOUSE_SPECIAL_DROP      3441
#define ID_XSDI_MOUSE_OPEN_CHECK        3442
#define ID_XSDI_MOUSE_OPEN_CNR          3443
#define ID_XSDI_MOUSE_CORNERSIZE_SLIDER 3444

// #define ID_XSD_MOUSEMAPPINGS2           3450     removed V0.9.19 (2002-05-28) [umoeller]
#define ID_XSDI_MOUSE_MAPPINGS2GROUP    3450
#define ID_XSDI_MOUSE_CHORDWINLIST      3451
#define ID_XSDI_MOUSE_SYSMENUMB2        3452
#define ID_XSDI_MOUSE_MB3SCROLL         3453
#define ID_XSDI_MOUSE_MB3PIXELS_TXT1    3454
#define ID_XSDI_MOUSE_MB3PIXELS_SLIDER  3455
#define ID_XSDI_MOUSE_MB3PIXELS_TXT2    3456
#define ID_XSDI_MOUSE_MB3LINEWISE       3457
#define ID_XSDI_MOUSE_MB3AMPLIFIED      3458
#define ID_XSDI_MOUSE_MB3AMP_TXT1       3459
#define ID_XSDI_MOUSE_MB3AMP_SLIDER     3460
#define ID_XSDI_MOUSE_MB3AMP_TXT2       3461
#define ID_XSDI_MOUSE_MB3SCROLLREVERSE  3462
//#define ID_XSDI_MOUSE_MB3CLK2MB1DBLCLK  3463
#define ID_XSDI_MOUSE_MB3CLICK_TXT      3464
#define ID_XSDI_MOUSE_MB3CLICK_DROP     3465    // added V0.9.9 (2001-03-14) [lafaix]

#ifndef __NOMOVEMENT2FEATURES__
#define ID_XSD_MOUSE_MOVEMENT2          3470    // added V0.9.14 (2001-08-02) [lafaix]
#define ID_XSDI_MOUSE_AUTOHIDE_CHECK    3471
#define ID_XSDI_MOUSE_AUTOHIDE_TXT1     3472
#define ID_XSDI_MOUSE_AUTOHIDE_SLIDER   3473
#define ID_XSDI_MOUSE_AUTOHIDE_TXT2     3474
#define ID_XSDI_MOUSE_AUTOHIDE_CHECKMNU 3475
#define ID_XSDI_MOUSE_AUTOHIDE_CHECKBTN 3476
#define ID_XSDI_MOUSE_AUTOMOVE_CHECK    3477
#define ID_XSDI_MOUSE_AUTOMOVE_TXT1     3478
#define ID_XSDI_MOUSE_AUTOMOVE_SLIDER   3479
#define ID_XSDI_MOUSE_AUTOMOVE_TXT2     3480
#define ID_XSDI_MOUSE_AUTOMOVE_ANIMATE  3481
#define ID_XSDI_MOUSE_AUTOMOVE_CENTER   3482    // added V0.9.19 (2002-04-11) [lafaix]
#endif

/* ******************************************************************
 *
 *   XCenter                            >= 3600
 *
 ********************************************************************/

// #define ID_CRD_SETTINGS_VIEW            3600 removed V0.9.19 (2002-05-07) [umoeller]
#define ID_XRDI_VIEW_FRAMEGROUP         3600    // added V0.9.19 (2002-05-07) [umoeller]
#define ID_CRDI_VIEW_TOPOFSCREEN        3601
#define ID_CRDI_VIEW_BOTTOMOFSCREEN     3602
#define ID_CRDI_VIEW_ALWAYSONTOP        3603
#define ID_CRDI_VIEW_ANIMATE            3604
#define ID_CRDI_VIEW_AUTOHIDE           3605
#define ID_CRDI_VIEW_POSITION_GROUP     3606    // added V0.9.19 (2002-05-07) [umoeller]
#define ID_CRDI_VIEW_PRTY_GROUP         3607    // added V0.9.19 (2002-05-07) [umoeller]
#define ID_CRDI_VIEW_PRTY_SLIDER        3608
#define ID_CRDI_VIEW_PRTY_TEXT          3609
#define ID_CRDI_VIEW_REDUCEWORKAREA     3610
#define ID_CRDI_VIEW_AUTOHIDE_TXT1      3611    // added V0.9.9 (2001-03-09) [umoeller]
#define ID_CRDI_VIEW_AUTOHIDE_SLIDER    3612    // added V0.9.9 (2001-03-09) [umoeller]
#define ID_CRDI_VIEW_AUTOHIDE_TXT2      3613    // added V0.9.9 (2001-03-09) [umoeller]
#define ID_CRDI_VIEW_AUTOHIDE_CLICK     3614    // added V0.9.14 (2001-08-21) [umoeller]
#define ID_CRDI_VIEW_AUTOSCREENBORDER   3615    // added V0.9.14 (2001-08-21) [umoeller]

// #define ID_CRD_SETTINGS_VIEW2           3650 // removed V0.9.16 (2001-10-24) [umoeller]
#define ID_CRDI_VIEW2_3DBORDER_GROUP    3650
#define ID_CRDI_VIEW2_3DBORDER_SLIDER   3651
#define ID_CRDI_VIEW2_3DBORDER_TEXT     3652
#define ID_CRDI_VIEW2_BDRSPACE_SLIDER   3653
#define ID_CRDI_VIEW2_BDRSPACE_TEXT     3654
#define ID_CRDI_VIEW2_WGTSPACE_SLIDER   3655
#define ID_CRDI_VIEW2_WGTSPACE_TEXT     3656
#define ID_CRDI_VIEW2_FLATBUTTONS       3657
#define ID_CRDI_VIEW2_SUNKBORDERS       3658
#define ID_CRDI_VIEW2_ALL3DBORDERS      3659
#define ID_CRDI_VIEW2_SIZINGBARS        3660
#define ID_CRDI_VIEW2_SPACINGLINES      3661
#define ID_CRDI_VIEW2_BDRSPACE_GROUP    3662    // V0.9.16 (2001-10-24) [umoeller]
#define ID_CRDI_VIEW2_WGTSPACE_GROUP    3663    // V0.9.16 (2001-10-24) [umoeller]
#define ID_CRDI_VIEW2_DEFSTYLES_GROUP   3664    // V0.9.16 (2001-10-24) [umoeller]
#define ID_CRDI_VIEW2_HATCHINUSE        3665    // V0.9.16 (2001-10-24) [umoeller]

#define ID_CRD_WINLISTWGT_SETTINGS      3700
#define ID_CRDI_FILTERS_CURRENTLB       3701
#define ID_CRDI_FILTERS_REMOVE          3702
#define ID_CRDI_FILTERS_NEWCOMBO        3703
#define ID_CRDI_FILTERS_ADD             3704

#define ID_CRD_HEALTHWGT_SETTINGS       3710
#define ID_CRDI_SETUP_STRING            3711

#define ID_CRD_DISKFREEWGT_SETTINGS     3720

/* ******************************************************************
 *
 *   XWPString                          >= 3800
 *
 ********************************************************************/

#define ID_XSD_XWPSTRING_PAGE           3800
#define ID_XSD_XWPSTRING_STRING_MLE     3801
#define ID_XSD_XWPSTRING_OBJ_CNR        3802
#define ID_XSD_XWPSTRING_OBJ_CLEAR      3803
#define ID_XSD_XWPSTRING_CONFIRM        3804

/* ******************************************************************
 *
 *   XWPFont                            >= 3900
 *
 ********************************************************************/

#define ID_FND_SAMPLETEXT               3900
#define ID_FNDI_SAMPLETEXT_MLE          3901

/* ******************************************************************
 *
 *   XWPAdmin                           >= 4000
 *
 ********************************************************************/

// all IDs changed V1.0.1 (2003-01-10) [umoeller]
#define ID_AMDI_USER_LOCAL_GROUP            4000
#define ID_AMDI_USER_USERNAME_TXT           4001
#define ID_AMDI_USER_USERNAME_DATA          4002
#define ID_AMDI_USER_USERID_TXT             4003
#define ID_AMDI_USER_USERID_DATA            4004
#define ID_AMDI_USER_FULLNAME_TXT           4005
#define ID_AMDI_USER_FULLNAME_DATA          4006
#define ID_AMDI_USER_PASS_TXT               4007
#define ID_AMDI_USER_PASS_DATA              4008
#define ID_AMDI_USER_CONFIRMPASS_TXT        4009
#define ID_AMDI_USER_CONFIRMPASS_DATA       4010
#define ID_AMDI_USER_GROUPS_TXT             4011
#define ID_AMDI_USER_GROUPS_DATA            4012
#define ID_AMDI_USER_CONTEXTS_TXT           4013
#define ID_AMDI_USER_CONTEXTS_DATA          4014
#define ID_AMDI_USER_LOCALSEC_TXT           4015
#define ID_AMDI_USER_LOCALSEC_DATA          4016
#define ID_AMDI_USER_ALLOCBYTES_TXT         4017
#define ID_AMDI_USER_ALLOCBYTES_DATA        4018
#define ID_AMDI_USER_BUFALLOCS_TXT          4019
#define ID_AMDI_USER_BUFALLOCS_DATA         4020
#define ID_AMDI_USER_EVENTS_TXT             4021
#define ID_AMDI_USER_EVENTS_DATA            4022
#define ID_AMDI_USER_AUTHORIZATIONS_TXT     4023
#define ID_AMDI_USER_AUTHORIZATIONS_DATA    4024
#define ID_AMSI_LOCAL_USER                  4025
#define ID_AMSI_ALL_USERS                   4026
#define ID_AMSI_ALL_GROUPS                  4027
#define ID_AMDI_F3_PERMISSIONS_TXT          4028
#define ID_AMDI_F3_PERMISSIONS_DATA         4029
#define ID_AMSI_PROCESSES                   4030    // added V1.0.2 (2003-11-13) [umoeller]

/* ******************************************************************
 *
 *   XShutdown                          >= 4100
 *
 ********************************************************************/

#define ID_SDICON                       4100   // shutdown icon

/* dlg templates */
#define ID_SDD_MAIN                     4200
#define ID_SDD_STATUS                   4201
// #define ID_SDD_CONFIRM                  4202     // removed V1.0.0 (2002-09-17) [umoeller]
#define ID_SDD_CAD                      4203
#define ID_SDD_CLOSEVIO                 4204
// #define ID_SDD_CONFIRMWPS               4205     // removed V0.9.16 (2002-01-13) [umoeller]
#define ID_SDD_BOOTMGR                  4206

/* dlg items */
#define ID_SDDI_LISTBOX                 4301
#define ID_SDDI_BEGINSHUTDOWN           4302
#define ID_SDDI_CANCELSHUTDOWN          4303
#define ID_SDDI_TEXTSHUTTING            4304
#define ID_SDDI_PROGRESSBAR             4305
#define ID_SDDI_STATUS                  4306
#define ID_SDDI_SKIPAPP                 4307
#define ID_SDDI_PERCENT                 4309
#ifndef __NOXSHUTDOWN__
#ifndef __EASYSHUTDOWN__
#define ID_SDDI_MESSAGEAGAIN            4310
#endif
#endif
#define ID_SDDI_VDMAPPTEXT              4311
#define ID_SDDI_WPS_CLOSEWINDOWS        4312
#define ID_SDDI_ICON                    4313
#define ID_SDDI_BOOTMGR                 4314
#ifndef __NOXWPSTARTUP__
#define ID_SDDI_WPS_STARTUPFOLDER       4315
#endif
#define ID_SDDI_SHUTDOWNONLY            4316
#define ID_SDDI_STANDARDREBOOT          4317
#define ID_SDDI_REBOOTTO                4318
#define ID_SDDI_EMPTYTRASHCAN           4319
#define ID_SDDI_CONFIRMWPS_TEXT         4320    // V1.0.0 (2002-09-17) [umoeller]
#define ID_SDDI_CONFIRMWPS_TITLE        4205    // V0.9.16 (2002-01-13) [umoeller]
#define ID_SDDI_PROGRESS1               4321    // V0.9.17 (2002-02-05) [pr]
#define ID_SDDI_PROGRESS2               4322
#define ID_SDDI_COMPLETE                4323
#define ID_SDDI_SWITCHOFF               4324
#define ID_SDDI_ARCHIVEONCE             4325    // V0.9.19 (2002-04-17) [umoeller]

#define ID_SDDI_CONFIRM_TITLE           4326
#define ID_SDDI_CONFIRM_TEXT            4327    // V0.9.5 (2000-08-10) [umoeller]
#define ID_SDDI_REBOOTOPT_GROUP         4328
#ifndef __NOXWPSTARTUP__
#define ID_SDDI_WPS_RUNSHUTDOWNFDR      4329    // V1.0.1 (2003-01-29) [umoeller]
#endif
#define ID_SDDI_PROGRESS0               4330    // V1.0.9 (2010-04-25) [ataylor]

/* command defs (used in the Shutdown wnd proc) */
#define ID_SDMI_CLOSEITEM               4400
#define ID_SDMI_UPDATESHUTLIST          4402
#define ID_SDMI_UPDATEPROGRESSBAR       4403
// #define ID_SDMI_FLUSHBUFFERS            4404     removed V0.9.9 (2001-04-04) [umoeller]
#define ID_SDMI_CLOSEVIO                4405
// #define ID_SDMI_PREPARESAVEWPS          4406     removed V0.9.9 (2001-04-04) [umoeller]
// #define ID_SDMI_SAVEWPSITEM             4407     removed V0.9.9 (2001-04-04) [umoeller]
// #define ID_SDMI_SAVEWPS                 4407     removed V0.9.9 (2001-04-04) [umoeller]
// #define ID_SDMI_BEGINCLOSINGITEMS       4408     removed V0.9.12 (2001-04-29) [umoeller]
// #define ID_SDMI_CLEANUPANDQUIT          4409     removed V0.9.9 (2001-04-04) [umoeller]

/* ******************************************************************
 *
 *   XWPScreen                          >= 4500
 *
 ********************************************************************/

/* All IDs were raised from >= 1400 to >= 4500 with
   V0.9.19 (2002-04-17) [umoeller] because we were
   running out of IDs in the old range.  */

#ifndef __NOPAGER__
    // #define ID_SCD_PAGER_GENERAL            4500     removed V0.9.19 (2002-05-23) [umoeller]
    #define ID_SCDI_PGR1_GROUP             4500
    #define ID_SCDI_PGR1_X_SLIDER          4501
    #define ID_SCDI_PGR1_X_TEXT2           4502
    #define ID_SCDI_PGR1_Y_SLIDER          4503
    #define ID_SCDI_PGR1_Y_TEXT2           4504
    #define ID_SCDI_PGR1_ENABLE            4505
    // 1405 to 1414 moved to ID_SCD_PAGER_WINDOW
    #define ID_SCDI_PGR1_ARROWHOTKEYS      4515
    #define ID_SCDI_PGR1_HOTKEYS_CTRL      4516
    #define ID_SCDI_PGR1_HOTKEYS_SHIFT     4517
    #define ID_SCDI_PGR1_HOTKEYS_ALT       4518
    #define ID_SCDI_PGR1_WRAPAROUND        4519
    #define ID_SCDI_PGR1_VALUESET          4520
    #define ID_SCDI_PGR1_FOLLOWFOCUS       4521     // V0.9.19 (2002-06-02) [umoeller]
    #define ID_SCDI_PGR1_WINDOWS_KEYS      4522     // V1.0.3 (2004-10-14) [bird]

    // #define ID_SCD_PAGER_COLORS             4530     // removed V0.9.19 (2002-05-07) [umoeller]
    #define ID_SCDI_PGR2_DISABLED_INFO     4529     // added V0.9.19 (2002-05-28) [umoeller]
    #define ID_SCDI_PGR2_DTP_INACTIVE_1    4530
    #define ID_SCDI_PGR2_DTP_INACTIVE_2    4531
    #define ID_SCDI_PGR2_DTP_ACTIVE        4532
    #define ID_SCDI_PGR2_DTP_GRID          4533
    #define ID_SCDI_PGR2_WIN_INACTIVE      4534
    #define ID_SCDI_PGR2_WIN_ACTIVE        4535
    #define ID_SCDI_PGR2_WIN_BORDER        4536
    #define ID_SCDI_PGR2_TXT_INACTIVE      4537
    #define ID_SCDI_PGR2_TXT_ACTIVE        4538
    #define ID_SCDI_PGR2_COLORS_GROUP      4540
    #define ID_SCDI_PGR2_COLORS_INFO       4541
    #define ID_SCDI_PGR2_COLORS_INACTIVE_1 4542
    #define ID_SCDI_PGR2_COLORS_INACTIVE_2 4543
    #define ID_SCDI_PGR2_COLORS_ACTIVE     4544
    #define ID_SCDI_PGR2_COLORS_BORDERS    4545
    #define ID_SCDI_PGR2_COLORS_BACKGROUND 4546
    #define ID_SCDI_PGR2_COLORS_MINIWINDOW 4547
    #define ID_SCDI_PGR2_COLORS_TITLE      4548

    // added V0.9.19 (2002-04-17) [umoeller]
    #define ID_SCDI_STICKY_GROUP            4550
    #define ID_SCDI_STICKY_CNR              4551

    #define ID_SCDI_STICKY_INCLUDE          4560
    #define ID_SCDI_STICKY_EXCLUDE          4561
    #define ID_SCDI_STICKY_CONTAINS         4562
    #define ID_SCDI_STICKY_BEGINSWITH       4563
    #define ID_SCDI_STICKY_ENDSWITH         4564
    #define ID_SCDI_STICKY_EQUALS           4565
    #define ID_SCDI_STICKY_MATCHES          4566
    #define ID_SCDI_STICKY_TITLEATTRIBUTE   4567
    #define ID_SCDI_STICKY_CRITERIA         4568
    #define ID_SCDI_STICKY_ATTRIBUTE        4569
    #define ID_SCDI_STICKY_OPERATOR         4570
    #define ID_SCDI_STICKY_VALUE            4571
    #define ID_SCDI_STICKY_ADDTITLE         4572
    #define ID_SCDI_STICKY_EDITTITLE        4573

    #define ID_SCDI_STICKY_CRITERIAGROUP    4580
    #define ID_SCDI_STICKY_ATTRIBUTE_DROP   4581
    #define ID_SCDI_STICKY_OPERATOR_DROP    4582
    #define ID_SCDI_STICKY_VALUE_DROP       4583
    #define ID_SCDI_STICKY_MATCHINGGROUP    4584
    #define ID_SCDI_STICKY_RADIO_INCLUDE    4585
    #define ID_SCDI_STICKY_RADIO_EXCLUDE    4586

    // removed V0.9.19 (2002-04-15) [lafaix]
    //#define ID_SCD_PAGER_NEWSTICKY       1460
    //#define ID_SCD_PAGER_COMBO_STICKIES  1461

    // #define ID_SCD_PAGER_WINDOW             4600 removed V0.9.19 (2002-04-17) [umoeller]
    #define ID_SCDI_PGR1_WINDOW_GROUP      4600
    #define ID_SCDI_PGR1_SHOWWINDOWS       4601
    #define ID_SCDI_PGR1_SHOWWINTITLES     4602
    #define ID_SCDI_PGR1_CLICK2ACTIVATE    4603
    // #define ID_SCDI_PGR1_TITLEBAR          4604     removed V0.9.19 (2002-05-07) [umoeller]
    #define ID_SCDI_PGR1_PRESERVEPROPS     4605
    #define ID_SCDI_PGR1_STAYONTOP         4606
    #define ID_SCDI_PGR1_FLASHTOTOP        4607
    #define ID_SCDI_PGR1_FLASH_TXT1        4608
    #define ID_SCDI_PGR1_FLASH_SPIN        4609
    #define ID_SCDI_PGR1_FLASH_TXT2        4610
    #define ID_SCDI_PGR1_SHOWSECONDARY     4611    // added V0.9.19 (2002-04-11) [lafaix]
    #define ID_SCDI_PGR1_SHOWSTICKY        4612    // added V0.9.19 (2002-04-11) [lafaix]
    #define ID_SCDI_PGR1_SHOWWINICONS      4613     // added V0.9.19 (2002-06-13) [umoeller]
    #define ID_SCDI_PGR1_STICKIESTOGGLE    4614    // added V1.0.0 (2002-09-14) [lafaix]
#endif

/* ******************************************************************
 *
 *   String IDs I                           >= 5000
 *
 ********************************************************************/

// XFolder uses the following ID's for language-
// dependent strings. These correspond to the
// string definitions in the NLS .RC file.
// Note that XFolder also uses a message file
// (.MSG) for messages which might turn out to
// be longer than 256 characters.

// These IDs should not have "gaps" in the
// numbers, because OS/2 loads string resources
// in blocks of 16 strings, which does not
// work if the IDs don't have following numbers.

// Note: All string IDs have been raised with V0.9.0.

// More string IDs (part II) are above 7000 because
// unfortunately I didn't reserve enough space when
// declaring the XWPSetup feature strings as >= 6000.

#define ID_XSS_MAIN               5000
#define ID_XSSI_NOTDEFINED        5001
#define ID_XSSI_PRODUCTINFO       5002
#define ID_XSSI_REFRESHNOW        5003
#define ID_XSSI_SNAPTOGRID        5004
#define ID_XSSI_DLLLANGUAGE       5005
#define ID_XSSI_XFOLDERVERSION    5006

#define ID_XSSI_FLDRCONTENT       5007
#define ID_XSSI_COPYFILENAME      5008
#define ID_XSSI_BORED             5009
#define ID_XSSI_FLDREMPTY         5010
#define ID_XSSI_SELECTSOME        5011

// startup/shutdown folder context menu
#define ID_XSSI_PROCESSCONTENT    5012

#define ID_XFSI_QUICKSTATUS       5013

#define ID_XSSI_POPULATING        5014
#define ID_XSSI_SBTEXTNONESEL     5015
#define ID_XSSI_SBTEXTMULTISEL    5016
#define ID_XSSI_SBTEXTWPDATAFILE  5017
#define ID_XSSI_SBTEXTWPDISK      5018
#define ID_XSSI_SBTEXTWPPROGRAM   5019
#define ID_XSSI_SBTEXTWPOBJECT    5020

#define ID_XSSI_NLS_AUTHOR        5021      // new with V0.9.0
// #define ID_XSSI_KERNEL_BUILD      5022      // new with V0.9.0; for /main/xfldr.rc
        // removed V0.9.7 (2000-12-14) [umoeller]
#define ID_XSSI_RUN               5022      // new with V0.9.14

// copy filename submenu V1.0.0 (2002-11-09) [umoeller]
#define ID_XSSI_COPYFILENAME_SHORTSP 5023
#define ID_XSSI_COPYFILENAME_FULLSP  5024
#define ID_XSSI_COPYFILENAME_SHORTNL 5025
#define ID_XSSI_COPYFILENAME_FULLNL  5026
#define ID_XSSI_COPYFILENAME_SHORT1  5027
#define ID_XSSI_COPYFILENAME_FULL1   5028

// folder hotkeys: key descriptions
#define ID_XSSI_KEY_CTRL             5201
#define ID_XSSI_KEY_Alt              5202
#define ID_XSSI_KEY_SHIFT            5203

#define ID_XSSI_KEY_BACKSPACE        5204
#define ID_XSSI_KEY_TAB              5205
#define ID_XSSI_KEY_BACKTABTAB       5206
#define ID_XSSI_KEY_ENTER            5207
#define ID_XSSI_KEY_ESC              5208
#define ID_XSSI_KEY_SPACE            5209
#define ID_XSSI_KEY_PAGEUP           5210
#define ID_XSSI_KEY_PAGEDOWN         5211
#define ID_XSSI_KEY_END              5212
#define ID_XSSI_KEY_HOME             5213
#define ID_XSSI_KEY_LEFT             5214
#define ID_XSSI_KEY_UP               5215
#define ID_XSSI_KEY_RIGHT            5216
#define ID_XSSI_KEY_DOWN             5217
#define ID_XSSI_KEY_PRINTSCRN        5218
#define ID_XSSI_KEY_INSERT           5219
#define ID_XSSI_KEY_DELETE           5220
#define ID_XSSI_KEY_SCRLLOCK         5221
#define ID_XSSI_KEY_NUMLOCK          5222

#define ID_XSSI_KEY_WINLEFT          5223
#define ID_XSSI_KEY_WINRIGHT         5224
#define ID_XSSI_KEY_WINMENU          5225

// folder hotkeys: action descriptions
#define ID_XSSI_LB_REFRESHNOW           5500
#define ID_XSSI_LB_SNAPTOGRID           5501
#define ID_XSSI_LB_SELECTALL            5502
#define ID_XSSI_LB_OPENPARENTFOLDER     5503

#define ID_XSSI_LB_OPENSETTINGSNOTEBOOK 5504
#define ID_XSSI_LB_OPENNEWDETAILSVIEW   5505
#define ID_XSSI_LB_OPENNEWICONVIEW      5506
#define ID_XSSI_LB_DESELECTALL          5507
#define ID_XSSI_LB_OPENNEWTREEVIEW      5508

#define ID_XSSI_LB_FIND                 5509

#define ID_XSSI_LB_PICKUP               5510
#define ID_XSSI_LB_PICKUPCANCELDRAG     5511

#define ID_XSSI_LB_SORTBYNAME           5512
#define ID_XSSI_LB_SORTBYSIZE           5513
#define ID_XSSI_LB_SORTBYTYPE           5514
#define ID_XSSI_LB_SORTBYREALNAME       5515
#define ID_XSSI_LB_SORTBYWRITEDATE      5516
#define ID_XSSI_LB_SORTBYACCESSDATE     5517
#define ID_XSSI_LB_SORTBYCREATIONDATE   5518

#define ID_XSSI_LB_SWITCHTOICONVIEW     5519
#define ID_XSSI_LB_SWITCHTODETAILSVIEW  5520
#define ID_XSSI_LB_SWITCHTOTREEVIEW     5521

#define ID_XSSI_LB_ARRANGEDEFAULT       5522
#define ID_XSSI_LB_ARRANGEFROMTOP       5523
#define ID_XSSI_LB_ARRANGEFROMLEFT      5524
#define ID_XSSI_LB_ARRANGEFROMRIGHT     5525
#define ID_XSSI_LB_ARRANGEFROMBOTTOM    5526
#define ID_XSSI_LB_ARRANGEPERIMETER     5527
#define ID_XSSI_LB_ARRANGEHORIZONTALLY  5528
#define ID_XSSI_LB_ARRANGEVERTICALLY    5529

#define ID_XSSI_LB_INSERT               5530

#define ID_XSSI_LB_SORTBYEXTENSION      5531
#define ID_XSSI_LB_SORTFOLDERSFIRST     5532
#define ID_XSSI_LB_SORTBYCLASS          5533
#define ID_XSSI_LB_OPENPARENTFOLDERANDCLOSE     5534
#define ID_XSSI_LB_SELECTSOME           5535

#define ID_XSSI_LB_CLOSEWINDOW          5536

#define ID_XSSI_LB_CONTEXTMENU          5537
#define ID_XSSI_LB_TASKLIST             5538

#define ID_XSSI_LB_COPYFILENAME_SHORT   5539
#define ID_XSSI_LB_COPYFILENAME_FULL    5540

#define ID_XSSI_LB_PICKUPDROPCOPY       5541  // added V1.0.0 (2002-09-05) [lafaix]
#define ID_XSSI_LB_PICKUPDROPMOVE       5542  // added V1.0.0 (2002-09-05) [lafaix]
#define ID_XSSI_LB_PICKUPDROPLINK       5543  // added V1.0.0 (2002-09-05) [lafaix]

// FIRST and LAST are used be the notebook func to
// calculate corresponding items
#define ID_XSSI_LB_FIRST                5500
#define ID_XSSI_LB_LAST                 5543

// shutdown strings
#define ID_SDSI_FLUSHING                5600
#define ID_SDSI_CAD                     5601
#define ID_SDSI_REBOOTING               5602
#define ID_SDSI_CLOSING                 5603
#define ID_SDSI_SHUTDOWN                5604
#define ID_SDSI_RESTARTWPS              5605
#define ID_SDSI_RESTARTINGWPS           5606
#define ID_SDSI_SAVINGDESKTOP           5607
#define ID_SDSI_SAVINGPROFILES          5608
#define ID_SDSI_STARTING                5609
#define ID_SDSI_DEFAULT                 5610

// settings page titles (for notebook tabs)
// #define ID_XSSI_1GENERIC                5620         removed V0.9.20 (2002-07-25) [umoeller]
// #define ID_XSSI_2REMOVEITEMS            5621         removed V0.9.20 (2002-07-25) [umoeller]
// #define ID_XSSI_25ADDITEMS              5622         removed V0.9.20 (2002-07-25) [umoeller]
#define ID_XSSI_FILEPAGE                5619            // changed V1.0.1 (2003-01-10) [umoeller]
#define ID_XSSI_FILEPAGE_GENERAL        5620            // added V1.0.1 (2003-01-10) [umoeller]
#define ID_XSSI_FILEPAGE_EAS            5621            // added V1.0.1 (2003-01-10) [umoeller]
#define ID_XSSI_FILEPAGE_PERMISSIONS    5622            // added V1.0.1 (2003-01-10) [umoeller]
#define ID_XSSI_26CONFIGITEMS           5623
#define ID_XSSI_27STATUSBAR             5624
#define ID_XSSI_3SNAPTOGRID             5625
#define ID_XSSI_4ACCELERATORS           5626
#define ID_XSSI_TOOLBARS                5627            // added V1.0.1 (2002-12-08) [umoeller]
// #define ID_XSSI_5INTERNALS              5627         removed V0.9.20 (2002-07-25) [umoeller]
// #define ID_XSSI_FILEOPS                 5628         removed V0.9.20 (2002-07-25) [umoeller]
#define ID_XSSI_SORT                    5629
// #define ID_XSSI_INTERNALS               5630         removed V0.9.20 (2002-07-25) [umoeller]
#define ID_XSSI_WPSCLASSES              5631
#define ID_XSSI_XWPSTATUS               5632
#define ID_XSSI_FEATURES                5633
#define ID_XSSI_PARANOIA                5634
#define ID_XSSI_OBJECTS                 5635
#define ID_XSSI_DETAILSPAGE             5637
#define ID_XSSI_XSHUTDOWNPAGE           5638
#define ID_XSSI_STARTUPPAGE             5639
#define ID_XSSI_DTPMENUPAGE             5640
#define ID_XSSI_FILETYPESPAGE           5641
#define ID_XSSI_SOUNDSPAGE              5642
#define ID_XSSI_VIEWPAGE                5643
#define ID_XSSI_ARCHIVESPAGE            5644
#define ID_XSSI_PGMFILE_MODULE          5645
#define ID_XSSI_OBJECTHOTKEYSPAGE       5646
#define ID_XSSI_FUNCTIONKEYSPAGE        5647
#define ID_XSSI_MOUSEHOOKPAGE           5648
#define ID_XSSI_MAPPINGSPAGE            5649

// sort criteria
#define ID_XSSI_SV_NAME                 5650
#define ID_XSSI_SV_TYPE                 5651
#define ID_XSSI_SV_CLASS                5652
#define ID_XSSI_SV_REALNAME             5653
#define ID_XSSI_SV_SIZE                 5654
#define ID_XSSI_SV_WRITEDATE            5655
#define ID_XSSI_SV_ACCESSDATE           5656
#define ID_XSSI_SV_CREATIONDATE         5657
#define ID_XSSI_SV_EXT                  5658
#define ID_XSSI_SV_FOLDERSFIRST         5659

#define ID_XSSI_SV_ALWAYSSORT           5660

// message box strings
// #define ID_XSSI_DLG_CONFIRMCONFIGSYS1   4000
// #define ID_XSSI_DLG_CONFIRMCONFIGSYS2   4001

// "Yes", "no", etc.
// all removed again V0.9.19 (2002-04-17) [umoeller]
// #define ID_XSSI_DLG_UNDO                5797
// #define ID_XSSI_DLG_DEFAULT             5798
// #define ID_XSSI_DLG_HELP                5799
// #define ID_XSSI_DLG_YES                 5800
// #define ID_XSSI_DLG_YES2ALL             5801
// #define ID_XSSI_DLG_NO                  5802
// #define ID_XSSI_DLG_OK                  5803
// #define ID_XSSI_DLG_CANCEL              5804
// #define ID_XSSI_DLG_ABORT               5805
// #define ID_XSSI_DLG_RETRY               5806
// #define ID_XSSI_DLG_IGNORE              5807

// "status bars" page
#define ID_XSSI_SB_CLASSMNEMONICS       5808
#define ID_XSSI_SB_CLASSNOTSUPPORTED    5809

// "WPS Classes" page
#define ID_XSSI_WPSCLASSLOADED          5810
#define ID_XSSI_WPSCLASSLOADINGFAILED   5811
#define ID_XSSI_WPSCLASSREPLACEDBY      5812
#define ID_XSSI_WPSCLASSORPHANS         5813
#define ID_XSSI_WPSCLASSORPHANSINFO     5814

// CONFIG.SYS pages
#define ID_XSSI_SCHEDULER               5815
#define ID_XSSI_MEMORY                  5816
#define ID_XSSI_ERRORS                  5817
#define ID_XSSI_WPS                     5818
#define ID_XSSI_SYSPATHS                5819    // new with V0.9.0
#define ID_XSSI_DRIVERS                 5820    // new with V0.9.0
#define ID_XSSI_DRIVERCATEGORIES        5821    // new with V0.9.0

// settings submenu strings
#define ID_XFSI_SETTINGS                5831
#define ID_XFSI_SETTINGSNOTEBOOK        5832
#define ID_XFSI_ATTRIBUTES              5833
#define ID_XFSI_ATTR_ARCHIVE            5834
#define ID_XFSI_ATTR_SYSTEM             5835
#define ID_XFSI_ATTR_HIDDEN             5836
#define ID_XFSI_ATTR_READONLY           5837

#define ID_XFSI_FLDRSETTINGS            5838
#define ID_XFSI_SMALLICONS              5839
#define ID_XFSI_FLOWED                  5840
#define ID_XFSI_NONFLOWED               5841
#define ID_XFSI_NOGRID                  5842

#define ID_XFSI_WARP4MENUBAR            5843
#define ID_XFSI_SHOWSTATUSBAR           5844

// "WPS Class List" (XWPClassList, new with V0.9.0)
#define ID_XFSI_OPENCLASSLIST           5845
#define ID_XFSI_XWPCLASSLIST            5846
#define ID_XFSI_REGISTERCLASS           5847

// XWPSound (new with V0.9.0)
#define ID_XSSI_SOUNDSCHEMENONE         5849
// "System paths" page
#define ID_XSSI_ITEMSSELECTED           5850    // new with V0.9.0

// Trash can (XWPTrashCan, XWPTrashObject, new with V0.9.0)
#define ID_XTSI_TRASHEMPTY              5851
#define ID_XTSI_TRASHRESTORE            5852
#define ID_XTSI_TRASHDESTROY            5853

#define ID_XTSI_TRASHCAN                5854
#define ID_XTSI_TRASHOBJECT             5855

#define ID_XTSI_TRASHSETTINGSPAGE       5856
#define ID_XTSI_TRASHDRIVESPAGE         5857

#define ID_XTSI_ORIGFOLDER              5858
#define ID_XTSI_DELDATE                 5859
#define ID_XTSI_DELTIME                 5860
#define ID_XTSI_SIZE                    5861
#define ID_XTSI_ORIGCLASS               5862

#define ID_XTSI_STB_POPULATING          5863
#define ID_XTSI_STB_OBJCOUNT            5864

// Details view columns on XWPKeyboard "Hotkeys" page; V0.9.1 (99-12-03)
#define ID_XSSI_HOTKEY_TITLE            5865
#define ID_XSSI_HOTKEY_FOLDER           5866
#define ID_XSSI_HOTKEY_HANDLE           5867
#define ID_XSSI_HOTKEY_HOTKEY           5868

// Method info columns for XWPClassList; V0.9.1 (99-12-03)
#define ID_XSSI_CLSLIST_INDEX           5869
#define ID_XSSI_CLSLIST_METHOD          5870
#define ID_XSSI_CLSLIST_ADDRESS         5871
#define ID_XSSI_CLSLIST_CLASS           5872
#define ID_XSSI_CLSLIST_OVERRIDDENBY    5873

// "Special functions" on XWPMouse "Movement" page
#define ID_XSSI_SPECIAL_WINDOWLIST      5903
#define ID_XSSI_SPECIAL_DESKTOPPOPUP    5904

// default title of XWPScreen class V0.9.2 (2000-02-23) [umoeller]
#define ID_XSSI_XWPSCREENTITLE          5905

// "Partitions" item in WPDrives "open" menu V0.9.2 (2000-02-29) [umoeller]
#define ID_XSSI_OPENPARTITIONS          5906

// "Syslevel" page title in "OS/2 kernel" V0.9.3 (2000-04-01) [umoeller]
#define ID_XSSI_SYSLEVELPAGE            5907

#define ID_XTSI_CALCULATING             5908

#define ID_MMSI_DEVICETYPE              5909
#define ID_MMSI_DEVICEINDEX             5910
#define ID_MMSI_DEVICEINFO              5911

#define ID_MMSI_TYPE_IMAGE              5912
#define ID_MMSI_TYPE_AUDIO              5913
#define ID_MMSI_TYPE_MIDI               5914
#define ID_MMSI_TYPE_COMPOUND           5915
#define ID_MMSI_TYPE_OTHER              5916
#define ID_MMSI_TYPE_UNKNOWN            5917
#define ID_MMSI_TYPE_VIDEO              5918
#define ID_MMSI_TYPE_ANIMATION          5919
#define ID_MMSI_TYPE_MOVIE              5920

#define ID_MMSI_TYPE_STORAGE            5921
#define ID_MMSI_TYPE_FILE               5922
#define ID_MMSI_TYPE_DATA               5923

#define ID_MMSI_DEVTYPE_VIDEOTAPE       5924
#define ID_MMSI_DEVTYPE_VIDEODISC       5925
#define ID_MMSI_DEVTYPE_CD_AUDIO        5926
#define ID_MMSI_DEVTYPE_DAT             5927
#define ID_MMSI_DEVTYPE_AUDIO_TAPE      5928
#define ID_MMSI_DEVTYPE_OTHER           5929
#define ID_MMSI_DEVTYPE_WAVEFORM_AUDIO  5930
#define ID_MMSI_DEVTYPE_SEQUENCER       5931
#define ID_MMSI_DEVTYPE_AUDIO_AMPMIX    5932
#define ID_MMSI_DEVTYPE_OVERLAY         5933
#define ID_MMSI_DEVTYPE_ANIMATION       5934
#define ID_MMSI_DEVTYPE_DIGITAL_VIDEO   5935
#define ID_MMSI_DEVTYPE_SPEAKER         5936
#define ID_MMSI_DEVTYPE_HEADPHONE       5937
#define ID_MMSI_DEVTYPE_MICROPHONE      5938
#define ID_MMSI_DEVTYPE_MONITOR         5939
#define ID_MMSI_DEVTYPE_CDXA            5940
#define ID_MMSI_DEVTYPE_FILTER          5941
#define ID_MMSI_DEVTYPE_TTS             5942

#define ID_MMSI_COLMN_FOURCC            5943
#define ID_MMSI_COLMN_NAME              5944
#define ID_MMSI_COLMN_IOPROC_TYPE       5945
#define ID_MMSI_COLMN_MEDIA_TYPE        5946
#define ID_MMSI_COLMN_EXTENSION         5947
#define ID_MMSI_COLMN_DLL               5948
#define ID_MMSI_COLMN_PROCEDURE         5949

#define ID_MMSI_PAGETITLE_DEVICES       5950
#define ID_MMSI_PAGETITLE_IOPROCS       5951
#define ID_MMSI_PAGETITLE_CODECS        5952

#define ID_XSSI_PAGETITLE_PAGER         5953

#define ID_XSSI_XWPSTRING_PAGE          5954
#define ID_XSSI_XWPSTRING_OPENMENU      5955

#define ID_XSSI_COLMN_SYSL_COMPONENT    5956
#define ID_XSSI_COLMN_SYSL_FILE         5957
#define ID_XSSI_COLMN_SYSL_VERSION      5958
#define ID_XSSI_COLMN_SYSL_LEVEL        5959
#define ID_XSSI_COLMN_SYSL_PREVIOUS     5960

#define ID_XSSI_DRIVERS_VERSION         5961
#define ID_XSSI_DRIVERS_VENDOR          5962

// removed these duplicates V0.9.18 (2002-02-13) [umoeller]
// #define ID_XSSI_FUNCKEY_DESCRIPTION     5963
// #define ID_XSSI_FUNCKEY_SCANCODE        5964
// #define ID_XSSI_FUNCKEY_MODIFIER        5965

// default documents V0.9.4 (2000-06-09) [umoeller]
#define ID_XSSI_DATAFILEDEFAULTDOC      5966
#define ID_XSSI_FDRDEFAULTDOC           5967

// XCenter V0.9.4 (2000-06-10) [umoeller]
#define ID_XSSI_XCENTERPAGE1            5968

// file operations V0.9.4 (2000-07-27) [umoeller]
#define ID_XSSI_FOPS_MOVE2TRASHCAN      5969
#define ID_XSSI_FOPS_RESTOREFROMTRASHCAN 5970
#define ID_XSSI_FOPS_TRUEDELETE         5971
#define ID_XSSI_FOPS_EMPTYINGTRASHCAN   5972

#define ID_XSSI_ICONPAGE                5973

// XShutdown INI save strings V0.9.5 (2000-08-16) [umoeller]
#define ID_XSSI_XSD_SAVEINIS_NEW        5974
#define ID_XSSI_XSD_SAVEINIS_OLD        5975
#define ID_XSSI_XSD_SAVEINIS_NONE       5976

// logoff V0.9.5 (2000-09-28) [umoeller]
#define ID_XSSI_XSD_LOGOFF              5977
#define ID_XSSI_XSD_CONFIRMLOGOFFTITLE  5978        // V1.0.1 (2003-01-05) [umoeller] below IDs raised
#define ID_XSSI_XSD_CONFIRMLOGOFFMSG    5979

// "bytes" strings for status bars V0.9.6 (2000-11-23) [umoeller]
#define ID_XSSI_BYTE                    5980
#define ID_XSSI_BYTES                   5981

// title of "Resources" page V0.9.7 (2000-12-20) [umoeller]
#define ID_XSSI_PGMFILE_RESOURCES       5982

// title of program(file) "Associations" page V0.9.9 (2001-03-07) [umoeller]
#define ID_XSSI_PGM_ASSOCIATIONS        5983

// sigh... more new strings below, after the features page...
// not enough room here

/* ******************************************************************
 *
 *   XWPSetup features                  >= 6000
 *
 ********************************************************************/

// The following identifiers are used BOTH
// for loading string resources and for
// identifying check box container record
// cores (helpers/cnrh.c).

#define ID_XCSI_GENERALFEATURES         5999
#ifndef __NOICONREPLACEMENTS__
#define ID_XCSI_REPLACEICONS            6000
#endif
// #ifndef __ALWAYSRESIZESETTINGSPAGES__            // setting removed V1.0.1 (2002-12-14) [umoeller]
// #define ID_XCSI_RESIZESETTINGSPAGES     6001
// #endif
#ifndef __ALWAYSREPLACEICONPAGE__
#define ID_XCSI_REPLACEICONPAGE         6002        // renamed V0.9.16 (2001-10-15) [umoeller]
#endif
#ifndef __ALWAYSREPLACEFILEPAGE__
#define ID_XCSI_REPLACEFILEPAGE         6003
#endif
#ifndef __NOXSYSTEMSOUNDS__
#define ID_XCSI_XSYSTEMSOUNDS           6004
#endif
#ifndef __ALWAYSFIXCLASSTITLES__
#define ID_XCSI_FIXCLASSTITLES          6005        // V0.9.12 (2001-05-22) [umoeller]
#endif
#ifndef __ALWAYSREPLACEHELP__
#define ID_XCSI_REPLACEHELP             6006        // added V0.9.20 (2002-07-19) [umoeller]
#endif

#define ID_XCSI_FOLDERFEATURES          6010
#ifndef __NOCFGSTATUSBARS__
#define ID_XCSI_ENABLESTATUSBARS        6011
#endif
#ifndef __NOSNAPTOGRID__
#define ID_XCSI_ENABLESNAP2GRID         6012
#endif
#ifndef __ALWAYSFDRHOTKEYS__
#define ID_XCSI_ENABLEFOLDERHOTKEYS     6013
#endif
#ifndef __ALWAYSEXTSORT__
#define ID_XCSI_EXTFOLDERSORT           6014
#endif
#ifndef __NOTURBOFOLDERS__
#define ID_XCSI_TURBOFOLDERS            6015        // V0.9.16 (2001-10-25) [umoeller]
#endif
#define ID_XCSI_FDRSPLITVIEWS           6016        // V1.0.0 (2002-09-09) [umoeller]

#define ID_XCSI_ENABLETOOLBARS          6017        // V1.0.1 (2002-11-30) [umoeller]

#define ID_XCSI_STARTSHUTFEATURES       6020
#ifndef __ALWAYSREPLACEARCHIVING__
#define ID_XCSI_ARCHIVING               6021
#endif
#ifndef __NOXSHUTDOWN__
#define ID_XCSI_RESTARTWPS              6022
#define ID_XCSI_XSHUTDOWN               6023
#endif
#ifndef __NEVERCHECKDESKTOP__
#define ID_XCSI_CHECKDESKTOP            6024
#define ID_XCSI_PREPOPULATEDESKTOP      6025
#endif

#define ID_XCSI_FILEOPERATIONS          6040
// #ifndef __NEVEREXTASSOCS__                       // removed V1.0.1 (2002-12-15) [umoeller]
// #define ID_XCSI_EXTASSOCS               6041
// #endif

#define ID_XCSI_REPLACEPASTE            6042        // V0.9.20 (2002-08-08) [umoeller]

#ifndef __ALWAYSREPLACEFILEEXISTS__
#define ID_XCSI_REPLFILEEXISTS          6043
#endif
#ifndef __NEVERREPLACEDRIVENOTREADY__
#define ID_XCSI_REPLDRIVENOTREADY       6044
#endif
#ifndef __ALWAYSTRASHANDTRUEDELETE__
#define ID_XCSI_REPLACEDELETE           6045    // changed V0.9.19 (2001-04-13) [umoeller]
#endif
#define ID_XCSI_REPLHANDLES             6047
#ifndef __ALWAYSREPLACEREFRESH__
#define ID_XCSI_REPLACEREFRESH          6048
#endif
#ifndef __NEVERNEWFILEDLG__
#define ID_XCSI_NEWFILEDLG              6049
#endif

#define ID_XCSI_MOUSEKEYBOARDFEATURES   6050
#ifndef __ALWAYSHOOK__
#define ID_XCSI_XWPHOOK                 6051
#endif
#ifndef __ALWAYSOBJHOTKEYS__
#define ID_XCSI_GLOBALHOTKEYS           6053
#endif
#ifndef __NOPAGER__
// #define ID_XCSI_PAGER                6054        removed V0.9.19 (2002-05-28) [umoeller]
#endif

/* ******************************************************************
 *
 *   String IDs II                      >= 6097
 *
 ********************************************************************/

#define ID_XSSI_ADDWIDGETTOTRAY         6097        // "~ADD WIDGET to tray"
#define ID_XSSI_STYLEPAGE               6098
#define ID_XSSI_CLASSESPAGE             6099
#define ID_XSSI_WIDGETSPAGE             6100
#define ID_XSSI_ADDWIDGET               6101        // "~ADD WIDGET"
#define ID_XSSI_WIDGETCLASS             6102        // "Class";
#define ID_XSSI_WIDGETSETUP             6103        // "Setup";

#define ID_XSSI_FONTFOLDER              6104
#define ID_XSSI_FONTFILE                6105
#define ID_XSSI_FONTFAMILY              6106
#define ID_XSSI_FONTERRORS              6107
#define ID_XSSI_FONTOBJECT              6108
#define ID_XSSI_CDPLAYERVIEW            6109
#define ID_XSSI_CDPLAYER                6110
#define ID_XSSI_VOLUMEVIEW              6111
#define ID_XSSI_VOLUME                  6112
#define ID_XSSI_ADMINISTRATOR           6113
#define ID_XSSI_THREADSPAGE             6114

#define ID_XSSI_CLSLISTTOKEN            6120

#define ID_XSSI_DRIVERVERSION           6121           // "Version: "
#define ID_XSSI_DRIVERVENDOR            6122           // "Vendor: "

#define ID_XSSI_FONTSAMPLEVIEW          6123
#define ID_XSSI_FONTDEINSTALL           6124           // "~Deinstall..."

#define ID_XSSI_KEYDESCRIPTION          6125           // "Key description"
#define ID_XSSI_SCANCODE                6126           // "Hardware scan code";
#define ID_XSSI_MODIFIER                6127           // "Modifier";

#define ID_XSSI_DEFAULTSHUTDOWN         6128           // "DEFAULT OS/2 SHUTDOWN..."
#define ID_XSSI_INSTALLINGFONTS         6129           // "Installing fonts...";

#define ID_XSSI_PARAMETERS              6130           // title of variable prompts dlg
#define ID_XSSI_CLOSE                   6131           // "~CLOSE"

#define ID_XSSI_DROPPED1                6132           // "... %D OBJECTS DROPPED,"
#define ID_XSSI_DROPPED2                6133           // "OPEN FOLDER TO SEE THEM"

#define ID_XSSI_THREADSTHREAD           6134           // "Thread";
#define ID_XSSI_THREADSTID              6135           // "TID";
#define ID_XSSI_THREADSPRIORITY         6136           // "Priority";
#define ID_XSSI_THREADSGROUPTITLE       6137           // "XWorkplace threads"

#define ID_XSSI_ARCRESTORED             6138           // "Archive was just restored."
#define ID_XSSI_ARCDAYSPASSED           6139           // "%d days passed since last backup."
#define ID_XSSI_ARCDAYSLIMIT            6140           // "Limit: %d days."
#define ID_XSSI_ARCINICHECKING          6141            // "Checking INI files..."
#define ID_XSSI_ARCINICHANGED           6142            // "INI files changed %f %%."
#define ID_XSSI_ARCINILIMIT             6143            // "Limit: %f %%."
#define ID_XSSI_ARCENABLED              6144            // "Desktop archiving enabled."
#define ID_XSSI_ARCNOTNECC              6145            // "Desktop archiving not necessary."

#define ID_XSSI_PGMFILE_MODULE1         6146           // "Imported modules"
#define ID_XSSI_PGMFILE_MODULE2         6147           // "Exported functions"
#define ID_XSSI_COLMN_MODULENAME        6148           // "Name"
#define ID_XSSI_COLMN_EXPORTORDINAL     6149           // "Ordinal"
#define ID_XSSI_COLMN_EXPORTTYPE        6150           // "Type"
#define ID_XSSI_COLMN_EXPORTNAME        6151           // "Name"
#define ID_XSSI_COLMN_RESOURCEICON      6152           // "Icon"
#define ID_XSSI_COLMN_RESOURCEID        6153           // "ID"
#define ID_XSSI_COLMN_RESOURCETYPE      6154           // "Type"
#define ID_XSSI_COLMN_RESOURCESIZE      6155           // "Size"
#define ID_XSSI_COLMN_RESOURCEFLAGS     6156           // "Flags"

#ifndef __NOPAGER__
#define ID_XSSI_SPECIAL_PAGERUP      6157           // "One screen up"
#define ID_XSSI_SPECIAL_PAGERRIGHT   6158           // "One screen right"
#define ID_XSSI_SPECIAL_PAGERDOWN    6159           // "One screen down"
#define ID_XSSI_SPECIAL_PAGERLEFT    6160           // "One screen left"
#endif

#define ID_XSSI_MB3_AUTOSCROLL          6161           // "AutoScroll"
#define ID_XSSI_MB3_DBLCLICK            6162           // "Double click"
#define ID_XSSI_MB3_NOCONVERSION        6163           // "MB3 click"
#define ID_XSSI_MB3_PUSHTOBOTTOM        6164           // "Push to bottom"

#define ID_XSSI_SCREENBORDERSPAGE       6165           // "Screen borders"

#define ID_XSSI_CLASSTITLE_FOLDER       6167
#define ID_XSSI_CLASSTITLE_DISK         6168
#define ID_XSSI_CLASSTITLE_DATAFILE     6169

#define ID_XSSI_CLOSETHISVIEW           6170           // "close this view
#define ID_XSSI_CLOSEALLTHISFDR         6171           // "close all of this fdr"
#define ID_XSSI_CLOSEALLSUBFDRS         6172           // "close all subfdrs"

#define ID_XSSI_INVALID_OBJECT          6173           // "invalid object"
#define ID_XSSI_XSHUTDOWN               6174           // "~XShutdown"
#define ID_XSSI_XC_CLASSTITLE           6175           // class title
#define ID_XSSI_XC_VERSION              6176           // version
#define ID_XSSI_XC_OBJBUTTONWIDGET      6177           // obj button widget

#define ID_XSSI_ARCENABLEDONCE          6178
#define ID_MMSI_INSTALLNAME             6179           // "install name";
#define ID_MMSI_LOGICALNAME             6180           // "logical name";
#define ID_MMSI_ALIAS                   6181           // "alias";
#define ID_MMSI_MCD                     6182
#define ID_MMSI_VSD                     6183
#define ID_MMSI_PDD                     6184
#define ID_MMSI_PARAMS                  6185

#define ID_CRSI_ADDTRAY                 6186           // "Add tray"
#define ID_CRSI_RENAMETRAY              6187           // "Rename tray"
#define ID_CRSI_REMOVETRAY              6188           // "Delete tray"

#define ID_CRSI_TRAY                    6189           // "Tray %d"
#define ID_CRSI_NOTRAYACTIVE            6190           // "No active tray"

#define ID_CRSI_ENTERNEWTRAY            6191           // "Please enter a name for the tray to be created:"
#define ID_CRSI_ENTERRENAMETRAY         6192           // "Please enter a new name for this tray:"

#define ID_XSSI_WPSFDRVIEWPAGE          6193           // "Folder views"

// the following added with V0.9.16 (2002-01-01) [umoeller]
#define ID_CRSI_SENTINELTOOLTIP         6194
#define ID_CRSI_BUILTINCLASS            6195
#define ID_CRSI_OWGT_MENUITEMS          6196
#define ID_CRSI_PWGT_TITLE              6197
#define ID_CRSI_PWGT_CPUGRAPHCOLORS     6198
#define ID_CRSI_PWGT_CPUXUSERCOLOR      6199
#define ID_CRSI_PWGT_IRQLOADCOLOR       6200
#define ID_CRSI_PWGT_OTHERCOLORS        6201
#define ID_CRSI_PWGT_BACKGROUNDCOLOR    6202
#define ID_CRSI_PWGT_TEXTCOLOR          6203
#define ID_CRSI_PWGT_TOOLTIP1           6204
#define ID_CRSI_PWGT_TOOLTIP2           6205

#define ID_XFSI_FDLG_DRIVES             6206
#define ID_XFSI_FDLG_FILESLIST          6207
#define ID_XFSI_FDLG_TYPES              6208
#define ID_XFSI_FDLG_DIRECTORY          6209
#define ID_XFSI_FDLG_WORKING            6210
#define ID_XFSI_FDLG_FILE               6211
#define ID_XFSI_FDLG_SAVEFILEAS         6212
#define ID_XFSI_FDLG_OPENFILE           6213
#define ID_XFSI_FDLG_ALLTYPES           6214

// the following added with V0.9.18 (2002-02-12) [pr]
#define ID_XSSI_SPECIAL_SCREENWRAP      6215    // "Screen wrap"

// all the following added with V0.9.19 (2002-04-02) [umoeller]
#define ID_XSSI_DOUBLEFILES_FILENAME    6216
#define ID_XSSI_DOUBLEFILES_DIR         6217
#define ID_XSSI_DOUBLEFILES_SIZE        6218
#define ID_XSSI_DOUBLEFILES_DATE        6219
#define ID_XSSI_DOUBLEFILES_TIME        6220

#define ID_NDSI_STARTNETSCAPE           6221
#define ID_NDSI_SYNTAX                  6222

/* moved to admin section V1.0.1 (2003-01-10) [umoeller]
#define ID_XSSI_ADMIN_LOCAL_USER        6223
#define ID_XSSI_ADMIN_ALL_USERS         6224
#define ID_XSSI_ADMIN_ALL_GROUPS        6225
*/

#define ID_CRSI_OWGT_BITMAPFILE         6226

#define ID_CRSI_WIDGET_OBJBUTTON        6227
#define ID_CRSI_WIDGET_XBUTTON          6228
#define ID_CRSI_WIDGET_PULSE            6229
#define ID_CRSI_WIDGET_TRAY             6230
#define ID_CRSI_WIDGET_DISKFREE_WC      6231
#define ID_CRSI_WIDGET_DATE             6232
#define ID_CRSI_WIDGET_TIME             6233
#define ID_CRSI_WIDGET_FREEMEM          6234
#define ID_CRSI_WIDGET_POWER            6235
#define ID_CRSI_WIDGET_DISKFREE_COND    6236
#define ID_CRSI_WIDGET_SENTINEL         6237
#define ID_CRSI_WIDGET_WINDOWLIST       6238
#define ID_CRSI_WIDGET_HEALTH           6239
#define ID_CRSI_WIDGET_IPMONITOR        6240

#define ID_XSSI_FONT_BASELINE           6241
#define ID_XSSI_FONT_MAXASCENDER        6242
#define ID_XSSI_FONT_INTERNALLEADING    6243
#define ID_XSSI_FONT_LOWERCASEASCENT    6244

#define ID_XSSI_OBJDET_OBJECTID         6245
#define ID_XSSI_OBJDET_NONESET          6246
#define ID_XSSI_OBJDET_OBJECTID_ORIG    6247
#define ID_XSSI_OBJDET_DEFAULTVIEW      6248
#define ID_XSSI_OBJDET_HANDLE           6249
#define ID_XSSI_OBJDET_STYLEGROUP       6250
#define ID_XSSI_OBJDET_CUSTOMICON       6251
#define ID_XSSI_OBJDET_NOTDEFAULTICON   6252
#define ID_XSSI_OBJDET_NOCOPY           6253
#define ID_XSSI_OBJDET_NODELETE         6254
#define ID_XSSI_OBJDET_NODRAG           6255
#define ID_XSSI_OBJDET_NODROPON         6256
#define ID_XSSI_OBJDET_NOLINK           6257
#define ID_XSSI_OBJDET_NOMOVE           6258
#define ID_XSSI_OBJDET_NOPRINT          6259
#define ID_XSSI_OBJDET_NORENAME         6260
#define ID_XSSI_OBJDET_NOSETTINGS       6261
#define ID_XSSI_OBJDET_NOTVISIBLE       6262
#define ID_XSSI_OBJDET_TEMPLATE         6263
#define ID_XSSI_OBJDET_LOCKEDINPLACE    6264
#define ID_XSSI_OBJDET_PROGRAMDATA      6265
#define ID_XSSI_OBJDET_PROGRAMTYPE      6266
#define ID_XSSI_OBJDET_ENVIRONMENT      6267
#define ID_XSSI_OBJDET_FOLDERFLAGS      6268
#define ID_XSSI_OBJDET_FOLDERVIEWFLAGS  6269
#define ID_XSSI_OBJDET_OBJUSAGE         6270
#define ID_XSSI_OBJDET_OPENVIEWS        6271
#define ID_XSSI_OBJDET_ALLOCMEM         6272
#define ID_XSSI_OBJDET_AWAKESHADOWS     6273
#define ID_XSSI_OBJDET_FOLDERWINDOWS    6274
#define ID_XSSI_OBJDET_CNRHWND          6275
#define ID_XSSI_OBJDET_APPSOPEN         6276
#define ID_XSSI_OBJDET_APPHANDLE        6277
#define ID_XSSI_OBJDET_MENUID           6278

#define ID_CRSI_WINLIST_WIDGET          6279
#define ID_CRSI_WINLIST_RESTORE         6280
#define ID_CRSI_WINLIST_MOVE            6281
#define ID_CRSI_WINLIST_SIZE            6282
#define ID_CRSI_WINLIST_MINIMIZE        6283
#define ID_CRSI_WINLIST_MAXIMIZE        6284
#define ID_CRSI_WINLIST_HIDE            6285
#define ID_CRSI_WINLIST_KILL1           6286
#define ID_CRSI_WINLIST_KILL2           6287
#define ID_CRSI_WINLIST_CLOSE           6288        // V0.9.20 (2002-08-10) [umoeller]
#define ID_CRSI_WINLIST_FILTER          6289        // V0.9.20 (2002-08-10) [umoeller]

#define ID_CRSI_IPWGT_GRAPHCOLORS       6290
#define ID_CRSI_IPWGT_GRAPHINCOLOR      6291
#define ID_CRSI_IPWGT_GRAPHOUTCOLOR     6292
#define ID_CRSI_IPWGT_TOTAL             6293        // V 1.0.8 (2008-05-28)

#define ID_CRSI_SENTINELTOOLTIP2        6294        // V1.0.0 (2002-08-21) [umoeller]
#define ID_CRSI_MEMORYTOOLTIP           6295        // V1.0.0 (2002-08-21) [umoeller]
#define ID_CRSI_BATTERYTOOLTIP          6296        // V1.0.0 (2002-08-21) [umoeller]
#define ID_CRSI_DISKFREECONDTOOLTIP     6297        // V1.0.0 (2002-08-21) [umoeller]
#define ID_CRMI_IPWIDGET_SOURCE         6298        // V1.0.0 (2002-08-21) [umoeller]

#define ID_XFSI_FDR_SPLITVIEW           6299        // V1.0.0 (2002-08-21) [umoeller]

#define ID_XFSI_XWPSTARTUPFDR           6300        // V1.0.0 (2002-08-31) [umoeller]
#define ID_XFSI_XWPSHUTDOWNFDR          6301        // V1.0.0 (2002-08-31) [umoeller]

#define ID_XSSI_DEINSTALLINGFONTS       6302        // V1.0.1 (2002-12-15) [pr]

/* Sentinel widget with support for color settings V1.0.4 (2005-03-27) [chennecke] */
#define ID_CRSI_SWGT_PHYS               6303
#define ID_CRSI_SWGT_PHYSFREE           6304
#define ID_CRSI_SWGT_PHYSINUSE          6305
#define ID_CRSI_SWGT_SWAPFILE           6306
#define ID_CRSI_SWGT_SWAP               6307
#define ID_CRSI_SWGT_SWAPFREE           6308

/* ******************************************************************
 *
 *   String IDs for status bar mnemonics        >= 7000
 *   (all new with V0.9.14)
 *
 ********************************************************************/

#define ID_XSSI_SBMNC_1             7000         // "in bytes"
#define ID_XSSI_SBMNC_2             7001         // "in kBytes"
#define ID_XSSI_SBMNC_3             7002         // "in KBytes"
#define ID_XSSI_SBMNC_4             7003         // "in mBytes"
#define ID_XSSI_SBMNC_5             7004         // "in MBytes"
#define ID_XSSI_SBMNC_6             7005         // "in bytes/kBytes/mBytes/gBytes"
#define ID_XSSI_SBMNC_7             7006         // "in bytes/KBytes/MBytes/GBytes"
#define ID_XSSI_SBMNC_000           7007         // "no. of selected objects"
#define ID_XSSI_SBMNC_001           7008         // "total object count"
#define ID_XSSI_SBMNC_010           7009         // "free space on drive"
#define ID_XSSI_SBMNC_020           7010         // "total size of drive"
#define ID_XSSI_SBMNC_030           7011         // "size of selected objects in bytes"
#define ID_XSSI_SBMNC_040           7012         // "size of folder content in bytes"
#define ID_XSSI_SBMNC_100           7013         // "object title"
#define ID_XSSI_SBMNC_110           7014         // "WPS class default title"
#define ID_XSSI_SBMNC_120           7015         // "WPS class name"
#define ID_XSSI_SBMNC_200           7016         // "file system type (HPFS, FAT, CDFS, ...)"
#define ID_XSSI_SBMNC_210           7017         // "drive label"
#define ID_XSSI_SBMNC_220           7018         // "total space on drive in bytes"
#define ID_XSSI_SBMNC_300           7019         // "object's real name"
#define ID_XSSI_SBMNC_310           7020         // "object type (.TYPE EA)"
#define ID_XSSI_SBMNC_320           7021         // "object creation date"
#define ID_XSSI_SBMNC_330           7022         // "object creation time"
#define ID_XSSI_SBMNC_340           7023         // "object attributes"
#define ID_XSSI_SBMNC_350           7024         // "EA size in bytes"
#define ID_XSSI_SBMNC_360           7025         // "EA size in kBytes"
#define ID_XSSI_SBMNC_370           7026         // "EA size in KBytes"
#define ID_XSSI_SBMNC_380           7027         // "EA size in bytes/kBytes"
#define ID_XSSI_SBMNC_390           7028         // "EA size in bytes/KBytes"
#define ID_XSSI_SBMNC_400           7029         // "URL"
#define ID_XSSI_SBMNC_500           7030         // "executable program file"
#define ID_XSSI_SBMNC_510           7031         // "parameter list"
#define ID_XSSI_SBMNC_520           7032         // "working directory"

/* ******************************************************************
 *
 *   String IDs for daemon-specific resources       >= 8000
 *   (all new with V1.0.0)
 *
 *   If there ever happens to be more than 12 daemon-specific
 *   string resources, we will have to update the XDM_NLSCHANGED
 *   mechanism (in src/shared/common.c and src/Daemon/xwpdaemn.h).
 *
 *   added V1.0.0 (2002-09-15) [lafaix]
 *
 ********************************************************************/

#define ID_DMSI_FIRST               8000
#define ID_DMSI_STICKYTOGGLE        8000         // "Sticky"
#define ID_DMSI_LAST                8000

/* ******************************************************************
 *
 *   Treesize                           >= 10000
 *
 ********************************************************************/

#define ID_TSD_MAIN                     10001
#define ID_TSDI_CNR                     10002
#define ID_TSDI_TEXT1                   10003
#define ID_TSDI_TEXT2                   10004
#define ID_TSDI_ICON                    10005

// #define ID_TSD_PRODINFO                 10007        removed V0.9.16 (2001-12-02) [umoeller]

#define ID_TSM_CONTEXT                  10008
#define ID_TSM_SORT                     10009
#define ID_TSMI_SORTBYNAME              10010
#define ID_TSMI_SORTBYSIZE              10011
#define ID_TSMI_SORTBYEASIZE            10012
#define ID_TSMI_SORTBYFILESCOUNT        10013
#define ID_TSMI_COLLECTEAS              10014
#define ID_TSMI_LOWPRTY                 10015
#define ID_TSM_SIZES                    10016
#define ID_TSMI_SIZE_BYTES              10017
#define ID_TSMI_SIZE_KBYTES             10018
#define ID_TSMI_SIZE_MBYTES             10019

#define ID_TSMI_HELP                    10020           // changed V0.9.16 (2001-12-02) [umoeller]

#define ID_TS_ICON                      10021

// added V1.0.4 (2005-02-24) [chennecke]: strings for display of file and EA size in container
#define ID_TSSI_LARGESTFILES            10022
#define ID_TSSI_BYTES                   10023
#define ID_TSSI_KBYTES                  10024
#define ID_TSSI_MBYTES                  10025
#define ID_TSSI_WORKING                 10026
#define ID_TSSI_FILES                   10027

/* ******************************************************************
 *
 *   Netscape DDE                       >= 10100
 *
 ********************************************************************/

// #define ID_NDD_EXPLAIN                  10100        // removed V0.9.16 (2001-10-02) [umoeller]
// #define ID_NDD_QUERYSTART               10101        // removed V0.9.19 (2002-04-02) [umoeller]
                                                        // this was hideously ugly
#define ID_NDD_CONTACTING               10102
#define ID_NDD_STARTING                 10103

#define ID_ND_ICON                      10110

/* ******************************************************************
 *
 *   Xfix                               >= 11000
 *
 ********************************************************************/

#define ID_FXM_MAIN                          11000
#define ID_FX_ICON                           11001

#define ID_FXM_OBJIDS                        11050

#define ID_FXM_XFIX                          11100
#define ID_FXMI_RESCAN                       11101
#define ID_FXMI_WRITETOINI                   11102
#define ID_FXMI_CLOSETHIS                    11103
#define ID_FXMI_EXIT                         11104

#define ID_FXM_VIEW                          11130
#define ID_FXMI_VIEW_OBJIDS                  11131
#define ID_FXMI_VIEW_ABSTRACTS               11132

#define ID_FXM_SORT                          11150
#define ID_FXMI_SORT_INDEX                   11151
#define ID_FXMI_SORT_STATUS                  11152
#define ID_FXMI_SORT_TYPE                    11153
#define ID_FXMI_SORT_HANDLE                  11154
#define ID_FXMI_SORT_PARENT                  11155
#define ID_FXMI_SORT_SHORTNAME               11156
#define ID_FXMI_SORT_CHILDREN                11157
#define ID_FXMI_SORT_DUPS                    11158
#define ID_FXMI_SORT_REFCS                   11159
#define ID_FXMI_SORT_LONGNAME                11160

#define ID_FXMI_SORT_ID                      11170

#define ID_FXM_ACTIONS                       11200
#define ID_FXMI_ACTIONS_INTEGRITY            11201
#define ID_FXMI_ACTIONS_FILES                11202

#define ID_FXM_SELECT                        11300
#define ID_FXMI_SELECT_INVALID               11301
#define ID_FXMI_SELECT_BYNAME                11302

#define ID_FXM_HELP                          11400
#define ID_FXMI_HELP_USINGHELP               11401
#define ID_FXMI_HELP_GENERAL                 11402
#define ID_FXMI_HELP_PRODINFO                11403

#define ID_FXM_RECORDSELMULTI                11800
#define ID_FXM_RECORDSELSINGLE               11801
#define ID_FXMI_NUKEFOLDERPOS                11802
#define ID_FXMI_NUKEOBJID                    11803
#define ID_FXMI_DELETE                       11804
#define ID_FXMI_MOVEABSTRACTS                11805
#define ID_FXMI_CUT                          11806
#define ID_FXMI_PASTE                        11807

#define ID_FXDI_MAIN                         11500

#define ID_FXDI_OBJIDS                       11501

#define ID_FXSI_NUMBER                       11510
#define ID_FXSI_TYPE                         11511
#define ID_FXSI_NODEOFS                      11512
#define ID_FXSI_HANDLE                       11513
#define ID_FXSI_PARENT                       11514
#define ID_FXSI_SHORTNAME                    11515
#define ID_FXSI_CHILDREN                     11516
#define ID_FXSI_DUPLICATES                   11517
#define ID_FXSI_REFERENCES                   11518
#define ID_FXSI_LONGNAME                     11519

#define ID_FXSI_STATUS                       11520
#define ID_FXSI_ID                           11521

#define ID_FXSI_NODESTAT_DUPLICATEDRIV       11530
#define ID_FXSI_NODESTAT_ISDUPLICATE         11531
#define ID_FXSI_NODESTAT_INVALIDDRIVE        11532
#define ID_FXSI_NODESTAT_INVALIDPARENT       11533
#define ID_FXSI_NODESTAT_FILENOTFOUND        11534
#define ID_FXSI_NODESTAT_PARENTNOTFOLDER     11535
#define ID_FXSI_NODESTAT_FOLDERPOSNOTFOLDER  11536
#define ID_FXSI_NODESTAT_ABSTRACTSNOTFOLDER  11537

#define ID_FXSI_TYPE_DRIVE                   11538
#define ID_FXSI_TYPE_NODE                    11539

#define ID_FXSI_REFERENCES_ABSTRACTS         11540
#define ID_FXSI_REFERENCES_FOLDERPOS         11541

#define ID_FXSI_LONGNAME_ABSTRACTVALID       11542
#define ID_FXSI_LONGNAME_ABSTRACTINVALID     11543

#define ID_FXSI_STATBAR_LOADEDINSERTED       11560
#define ID_FXSI_STATBAR_HANDLESSELECTED      11561
#define ID_FXSI_STATBAR_DONE                 11562
#define ID_FXSI_STATBAR_SECONDS              11563
#define ID_FXSI_STATBAR_DUPWARNING           11564
#define ID_FXSI_STATBAR_IDSLOADED            11565
#define ID_FXSI_STATBAR_RESOLVINGPERCENT     11566
#define ID_FXSI_STATBAR_PARSINGPERCENT       11567
#define ID_FXSI_STATBAR_CHECKINGPERCENT      11568
#define ID_FXSI_STATBAR_RESOLVING            11569
#define ID_FXSI_STATBAR_DONECHECKING         11570
#define ID_FXSI_STATBAR_PARSINGHANDLES       11571

#define ID_FXDI_FILEMASKSELECT               11580
#define ID_FXDI_FILEMASKSELECT_MASK          11581
#define ID_FXDI_FILEMASKSELECT_SELECT        11582
#define ID_FXDI_FILEMASKSELECT_DESELECT      11583
#define ID_FXDI_FILEMASKSELECT_SELECTALL     11584
#define ID_FXDI_FILEMASKSELECT_DESELECTALL   11585

#define ID_FXSI_PRODINFO                     11590

#define ID_FXSI_NUKE_ABSTRACTS               11600
#define ID_FXSI_NUKE_AND                     11601
#define ID_FXSI_NUKE_FOLDERPOS               11602
#define ID_FXSI_NUKE_OBJECTIDS               11603
#define ID_FXSI_NUKE_SCHEDULED               11604
#define ID_FXSI_NUKE_HANDLEPROBLEM           11605
#define ID_FXSI_NUKE_HANDLESLEFT             11606

#define ID_FXSI_WRITEBACK_INTRO              11610
#define ID_FXSI_WRITEBACK_FIRSTFIRST         11611
#define ID_FXSI_WRITEBACK_FIRSTSECOND        11612
#define ID_FXSI_WRITEBACK_SECOND             11613
#define ID_FXSI_WRITEBACK_THIRD              11614
#define ID_FXSI_WRITEBACK_CONFIRM            11615

#define ID_FXSI_WRITEBACKABORTED             11620
#define ID_FXSI_WRITEBACKSUCCESS_BLOCKS      11621
#define ID_FXSI_WRITEBACKSUCCESS_OBJMOVED    11622
#define ID_FXSI_WRITEBACKSUCCESS_KEYSDELETED 11623
#define ID_FXSI_WRITEBACKSUCCESS_EXIT        11624

#define ID_FXSI_REMOVEHANDLES                11630

#define ID_FXSI_NUKEFOLDERPOS                11640

#define ID_FXSI_DELETEHANDLES                11650

#define ID_FXSI_MOVEABSTRACTS_DESKTOPID      11660
#define ID_FXSI_MOVEABSTRACTS_CONFIRM        11661

#define ID_FXSI_MOVEABSTRACTSSUCCESS         11670

#define ID_FXSI_ERROR_INVALIDHANDLE          11700
#define ID_FXSI_ERROR_INVALIDPARENTHANDLE    11701
#define ID_FXSI_ERROR_DEFAULT                11702
#define ID_FXSI_ERROR_ABSFOLDERHANDLELOST    11703
#define ID_FXSI_ERROR_INVALIDFOLDERHANDLE    11704
#define ID_FXSI_ERROR_CANTGETACTIVEHANDLES   11705
#define ID_FXSI_ERROR_ERROROCCURED           11706
#define ID_FXSI_ERROR_MOVEABSTRACTS          11707
#define ID_FXSI_ERROR_PARSEHANDLES           11708
#define ID_FXSI_ERROR_PROFILENOTEXIST        11709
#define ID_FXSI_ERROR_PROFILEZEROBYTES       11710
#define ID_FXSI_ERROR_CANTOPENPROFILE        11711
#define ID_FXSI_ERROR_USER                   11712
#define ID_FXSI_ERROR_SYSTEM                 11713
#define ID_FXSI_ERROR_INVALIDPARMC           11714
#define ID_FXSI_ERROR_INVALIDPARMS           11715
#define ID_FXSI_ERROR_NOTAVAILABLE           11716
#define ID_FXSI_ERROR_FATALLOADHANDLES       11717
#define ID_FXSI_ERROR_BUILDHANDLESCACHE      11718

/* ******************************************************************
 *
 *   Menu items
 *
 ********************************************************************/

// generic help menu item in various menus
#define ID_XFMI_HELP                    (WPMENUID_USER+1000) // added V0.9.0

// context menu in "WPS Classes" container;
// all the identifiers have changed with V0.9.0
#define ID_XLM_CLASS_SEL                (WPMENUID_USER+1001)
// #define ID_XLM_CLASS_NOSEL              (WPMENUID_USER+1002)
#define ID_XLMI_REGISTER                (WPMENUID_USER+1003)
#define ID_XLMI_DEREGISTER              (WPMENUID_USER+1004)
#define ID_XLMI_REPLACE                 (WPMENUID_USER+1005)
#define ID_XLMI_UNREPLACE               (WPMENUID_USER+1006)
#define ID_XLMI_CREATEOBJECT            (WPMENUID_USER+1007)
#define ID_XLM_METHOD_SEL               (WPMENUID_USER+1010)
#define ID_XLM_METHOD_NOSEL             (WPMENUID_USER+1011)
#define ID_XLM_METHOD_SORT              (WPMENUID_USER+1012)
#define ID_XLMI_METHOD_SORT_INDEX       (WPMENUID_USER+1013)
#define ID_XLMI_METHOD_SORT_NAME        (WPMENUID_USER+1014)
#define ID_XLMI_METHOD_SORT_INTRO       (WPMENUID_USER+1015)
#define ID_XLMI_METHOD_SORT_OVERRIDE    (WPMENUID_USER+1016)
#define ID_XLMI_REFRESH_VIEW            (WPMENUID_USER+1017) // V0.9.6 (2000-11-12) [umoeller]
#define ID_XLMI_FIND_CLASS              (WPMENUID_USER+1018) // V0.9.21 (2002-08-25) [jsmall]

// "File types" container
#define ID_XSM_FILETYPES_SEL            (WPMENUID_USER+1020)
#define ID_XSM_FILETYPES_NOSEL          (WPMENUID_USER+1021)
#define ID_XSMI_FILETYPES_DELETE        (WPMENUID_USER+1022)
#define ID_XSMI_FILETYPES_NEW           (WPMENUID_USER+1023)
#define ID_XSMI_FILETYPES_PICKUP        (WPMENUID_USER+1024)
#define ID_XSMI_FILETYPES_DROP          (WPMENUID_USER+1025)
#define ID_XSMI_FILETYPES_CANCELDRAG    (WPMENUID_USER+1026)
#define ID_XSMI_FILETYPES_EXPORT        (WPMENUID_USER+1027)
#define ID_XSMI_FILETYPES_IMPORT        (WPMENUID_USER+1028)

// "Filters" container on "File types" page
#define ID_XSM_FILEFILTER_SEL           (WPMENUID_USER+1030)
#define ID_XSM_FILEFILTER_NOSEL         (WPMENUID_USER+1031)
#define ID_XSMI_FILEFILTER_DELETE       (WPMENUID_USER+1035)
#define ID_XSMI_FILEFILTER_NEW          (WPMENUID_USER+1036)
#define ID_XSMI_FILEFILTER_IMPORTWPS    (WPMENUID_USER+1037)

// "Associations" container on "File types" page
#define ID_XSM_FILEASSOC_SEL            (WPMENUID_USER+1040)
#define ID_XSM_FILEASSOC_NOSEL          (WPMENUID_USER+1041)
#define ID_XSMI_FILEASSOC_DELETE        (WPMENUID_USER+1042)
#define ID_XSMI_FILEASSOC_SETTINGS      (WPMENUID_USER+1043)
#define ID_XSMI_FILEASSOC_OPENFDR       (WPMENUID_USER+1044)

// "Objects" menus on XWPSetup "Objects" page
#define ID_XSM_OBJECTS_SYSTEM           (WPMENUID_USER+1050)
#define ID_XSM_OBJECTS_XWORKPLACE       (WPMENUID_USER+1051)

// "Driver" menus on XFldSystem "Drivers" page
#define ID_XSM_DRIVERS_SEL              (WPMENUID_USER+1100)
#define ID_XSM_DRIVERS_NOSEL            (WPMENUID_USER+1101)
#define ID_XSMI_DRIVERS_CMDREFHELP      (WPMENUID_USER+1102)
// added V1.0.4 (2005-06-16) [chennecke]: menu items to invoke
// ConfigTool help and search in all help files
#define ID_XSMI_DRIVERS_CONFIGTOOLHELP  (WPMENUID_USER+1103)
#define ID_XSMI_DRIVERS_ALLHELP         (WPMENUID_USER+1104)

// "Hotkey" menus on XWPKeyboard "Hotkeys" page
#define ID_XSM_HOTKEYS_SEL              (WPMENUID_USER+1120)
#define ID_XSMI_HOTKEYS_PROPERTIES      (WPMENUID_USER+1121)
#define ID_XSMI_HOTKEYS_OPENFOLDER      (WPMENUID_USER+1122)
#define ID_XSMI_HOTKEYS_REMOVE          (WPMENUID_USER+1123)

// "Function keys" menus on XWPKeyboard "Function keys" page
// V0.9.3 (2000-04-18) [umoeller]
#define ID_XSM_FUNCTIONKEYS_SEL         (WPMENUID_USER+1130)
#define ID_XSMI_FUNCK_EDIT              (WPMENUID_USER+1131)
#define ID_XSMI_FUNCK_DELETE            (WPMENUID_USER+1132)

#define ID_XSM_FUNCTIONKEYS_NOSEL       (WPMENUID_USER+1133)
#define ID_XSMI_FUNCK_NEW               (WPMENUID_USER+1134)

// "Sticky windows" context menus on XWPScreen "XPager Sticky" page
// V0.9.4 (2000-07-10) [umoeller]
// ids and identifiers changed V1.0.1 (2003-01-05) [umoeller]
#define ID_XFM_CNRITEM_NOSEL            (WPMENUID_USER+1135)
#define ID_XFMI_CNRITEM_NEW             (WPMENUID_USER+1136)
#define ID_XFM_CNRITEM_SEL              (WPMENUID_USER+1137)
#define ID_XFMI_CNRITEM_DELETE          (WPMENUID_USER+1138)
#define ID_XFMI_CNRITEM_EDIT            (WPMENUID_USER+1139) // added V0.9.19 (2002-04-15) [lafaix]
#define ID_XFMI_CNRITEM_DELETEALL       (WPMENUID_USER+1140)

/* XCenter main button menu V0.9.7 (2000-11-30) [umoeller] */
/* changed all IDs to above WPMENUID_USER (0x6500) because
   otherwise we cause conflicts with the WPS menu IDs
   V0.9.11 (2001-04-25) [umoeller] */
#define ID_CRM_XCENTERBUTTON            (WPMENUID_USER+1144)
// menu items; these only need to be unique within the menu
#define ID_CRMI_SEP1                    0x7f00
#define ID_CRMI_SEP2                    0x7f01
#define ID_CRMI_SUSPEND                 0x7f02
#define ID_CRMI_LOGOFF                  0x7f03
#define ID_CRMI_RESTARTWPS              0x7f04
#define ID_CRMI_SHUTDOWN                0x7f05
#define ID_CRMI_RUN                     0x7f06

#define ID_CRM_WIDGET                   0x7f07
#define ID_CRMI_PROPERTIES              0x7f08
#define ID_CRMI_HELP                    0x7f09      // for widget
#define ID_CRMI_REMOVEWGT               0x7f0a

#define ID_CRMI_LOCKUPNOW               0x7f0b

#define ID_CRMI_ADDTRAY                 0x7f0c
#define ID_CRMI_RENAMETRAY              0x7f0d
#define ID_CRMI_REMOVETRAY              0x7f0e

#define ID_CRMI_SEP0                    0x7f0f
#define ID_CRMI_WIDGETCLASSNAME         0x7f10

#define ID_CRMI_SEP3                    0x7f11      // added V0.9.19 (2002-04-02) [umoeller]
#define ID_CRMI_SEP4                    0x7f12      // added V0.9.19 (2002-04-02) [umoeller]

#define ID_CRM_XCSUB                    0x7f13      // added V0.9.19 (2002-04-14) [umoeller]
#define ID_CRMI_XCSUB_PROPERTIES        0x7f14      // added V0.9.19 (2002-04-14) [umoeller]
#define ID_CRMI_XCSUB_CLOSE             0x7f15      // added V0.9.19 (2002-04-14) [umoeller]

#define ID_CRMI_HELPSUBMENU             0x7f10      // added V0.9.20 (2002-08-10) [umoeller]
#define ID_CRMI_HELP_XCENTER            0x7f11      // added V0.9.20 (2002-08-10) [umoeller]

#define ID_FNM_SAMPLE                   (WPMENUID_USER+1145)

/* split view menu bar V1.0.0 (2002-08-28) [umoeller] */
#define ID_XFM_SPLITVIEWBAR             (WPMENUID_USER+1150)
#define ID_XFM_BAR_FOLDER               (WPMENUID_USER+1151)
#define ID_XFM_BAR_EDIT                 (WPMENUID_USER+1152)
#define ID_XFM_BAR_VIEW                 (WPMENUID_USER+1153)
#define ID_XFM_BAR_SELECTED             (WPMENUID_USER+1154)
#define ID_XFM_BAR_HELP                 (WPMENUID_USER+1155)

#define ID_XSM_PAGERCTXTMENU            220

// The following menu ID's (with _OFS_ in their names)
// are _variable_ menu ID's: XFolder will add the value
// on the "XFolder Internals" page ("menu item id offset")
// to them to avoid ID conflicts. This value is stored
// internally with the other global settings.
#define ID_XFMI_OFS_SEPARATOR           (WPMENUID_USER+1)
#define ID_XFMI_OFS_PRODINFO            (WPMENUID_USER+2)
#define ID_XFMI_OFS_REFRESH             (WPMENUID_USER+3)
#define ID_XFMI_OFS_SNAPTOGRID          (WPMENUID_USER+4)
#define ID_XFMI_OFS_OPENPARENT          (WPMENUID_USER+5)
#define ID_XFMI_OFS_OPENPARENTANDCLOSE  (WPMENUID_USER+6)
#define ID_XFMI_OFS_CLOSE               (WPMENUID_USER+7)
// #define ID_XFM_OFS_SHUTDOWNMENU         (WPMENUID_USER+9)       // new with V0.9.0
                                        // removed V1.0.1 (2002-12-08) [umoeller]
// #define ID_XFMI_OFS_XSHUTDOWN           (WPMENUID_USER+10)      // new with V0.9.0
                                                    // removed V0.9.3 (2000-04-26) [umoeller]
#define ID_XFMI_OFS_OS2_SHUTDOWN        (WPMENUID_USER+10)      // new with V0.9.3 (2000-04-26) [umoeller]
#define ID_XFMI_OFS_RESTARTWPS          (WPMENUID_USER+11)

#define ID_XFMI_OFS_FLDRCONTENT         (WPMENUID_USER+12)
#define ID_XFMI_OFS_DUMMY               (WPMENUID_USER+13)
#define ID_XFM_OFS_COPYFILENAME         (WPMENUID_USER+14)      // submenu (changed V1.0.0 (2002-11-09) [umoeller])
#define ID_XFMI_OFS_COPYFILENAME_SHORTSP (WPMENUID_USER+15)      // short name with spaces changed V1.0.0 (2002-11-09) [umoeller]
#define ID_XFMI_OFS_COPYFILENAME_FULLSP  (WPMENUID_USER+16)      // full name with spaces V1.0.0 (2002-11-09) [umoeller]
#define ID_XFMI_OFS_BORED               (WPMENUID_USER+17)
#define ID_XFMI_OFS_SELECTSOME          (WPMENUID_USER+18)
#define ID_XFMI_OFS_PROCESSCONTENT      (WPMENUID_USER+19)

#define ID_XFMI_OFS_CONTEXTMENU         (WPMENUID_USER+20)

#define ID_XFMI_OFS_SORTBYCLASS         (WPMENUID_USER+21)
#define ID_XFMI_OFS_SORTBYEXT           (WPMENUID_USER+27)
#define ID_XFMI_OFS_SORTFOLDERSFIRST    (WPMENUID_USER+28)
#define ID_XFMI_OFS_ALWAYSSORT          (WPMENUID_USER+29)

#define ID_XFM_OFS_ATTRIBUTES           (WPMENUID_USER+31)
#define ID_XFMI_OFS_ATTR_ARCHIVED       (WPMENUID_USER+32)
#define ID_XFMI_OFS_ATTR_SYSTEM         (WPMENUID_USER+33)
#define ID_XFMI_OFS_ATTR_HIDDEN         (WPMENUID_USER+34)
#define ID_XFMI_OFS_ATTR_READONLY       (WPMENUID_USER+35)

// #define ID_XFM_OFS_WARP3FLDRVIEW        (WPMENUID_USER+36)   // removed V1.0.1 (2002-11-30) [umoeller]
#define ID_XFMI_OFS_SMALLICONS          (WPMENUID_USER+37)
#define ID_XFMI_OFS_FLOWED              (WPMENUID_USER+38)
#define ID_XFMI_OFS_NONFLOWED           (WPMENUID_USER+39)
#define ID_XFMI_OFS_NOGRID              (WPMENUID_USER+40)

#define ID_XFMI_OFS_WARP4MENUBAR        (WPMENUID_USER+41)  // added V0.9.0
#define ID_XFMI_OFS_SHOWSTATUSBAR       (WPMENUID_USER+42)

// new view item in "Open" submenu... this is used for XWPClassList, Partitions,
// XCenter, XWPFontObject, ...
#define ID_XFMI_OFS_XWPVIEW             (WPMENUID_USER+43)

// Trash can (XWPTrashCan, XWPTrashObject, new with V0.9.0)
#define ID_XFMI_OFS_TRASHEMPTY          (WPMENUID_USER+44)
#define ID_XFMI_OFS_TRASHRESTORE        (WPMENUID_USER+45)
#define ID_XFMI_OFS_TRASHDESTROY        (WPMENUID_USER+46)

// "Deinstall" menu item in XWPFontObject V0.9.7 (2001-01-13) [umoeller]
#define ID_XFMI_OFS_FONT_DEINSTALL      (WPMENUID_USER+47)

// "Default document" item in WPFolder "Open" and WPDataFile main context menu V0.9.4 (2000-06-09) [umoeller]
#define ID_XFMI_OFS_FDRDEFAULTDOC       (WPMENUID_USER+48)

// "Logoff" menu item in XFldDesktop V0.9.5 (2000-08-10) [umoeller]
#define ID_XFMI_OFS_LOGOFF              (WPMENUID_USER+50)

// extended "close" menu V0.9.12 (2001-05-22) [umoeller]
#define ID_XFMI_OFS_CLOSESUBMENU        (WPMENUID_USER+51)      // submenu
// "this view" has regular WPMENUID_CLOSE id
#define ID_XFMI_OFS_CLOSEALLTHISFDR     (WPMENUID_USER+52)      // "close all of this fdr"
#define ID_XFMI_OFS_CLOSEALLSUBFDRS     (WPMENUID_USER+53)      // "close all subfdrs"

// "Run" menu item in XFolder/XFldDisk V0.9.14 (2001-08-07) [pr]
#define ID_XFMI_OFS_RUN                 (WPMENUID_USER+54)

// folder split view V1.0.0 (2002-08-28) [umoeller]
#define ID_XFMI_OFS_SPLITVIEW           (WPMENUID_USER+55)
                // ID_XFMI_OFS_SPLITVIEW is used for the main frame of
                // a split view
#define ID_XFMI_OFS_SPLIT_SUBTREE       (WPMENUID_USER+56)
                // ID_XFMI_OFS_SPLIT_SUBTREE is used for the subframe
                // of ID_XFMI_OFS_SPLITVIEW that has the tree on the left
                // V1.0.1 (2002-12-08) [umoeller]
#define ID_XFMI_OFS_SPLIT_SUBFILES      (WPMENUID_USER+57)
                // ID_XFMI_OFS_SPLIT_SUBFILES is used for the subframe
                // of ID_XFMI_OFS_SPLITVIEW that has the list on the right
                // V1.0.1 (2002-12-08) [umoeller]

// batch rename V0.9.19 (2002-06-18) [umoeller]
#define ID_XFMI_OFS_BATCHRENAME         (WPMENUID_USER+58)      // raised V1.0.1 (2002-12-08) [umoeller]

#define ID_XFMI_OFS_COPYFILENAME_SHORTNL (WPMENUID_USER+59)     // short name with newlines, added V1.0.0 (2002-11-09) [umoeller]
                                                                // raised V1.0.1 (2002-12-08) [umoeller]
#define ID_XFMI_OFS_COPYFILENAME_FULLNL  (WPMENUID_USER+60)     // full name with newlines, added V1.0.0 (2002-11-09) [umoeller]
                                                                // raised V1.0.1 (2002-12-08) [umoeller]

// this is the value used for variable menu items, i.e.
// those inserted according to the config folder and by
// the "folder content" functions; XFolder will use this
// value (i.e. WPMENUID_USER + "menu item id offset"
// + FIRST_VARIABLE) and increment it until 0x8000 is reached
#define FIRST_VARIABLE                  65      // raised V1.0.0 (2002-11-09) [umoeller]
#define ID_XFMI_OFS_VARIABLE            (WPMENUID_USER+FIRST_VARIABLE)

#endif

