
/*
 *@@sourcefile common.h:
 *      header file for common.c.
 *
 *      This prototypes functions that are common to all
 *      parts of XWorkplace.
 *
 *      This file also declares all kinds of structures, id's,
 *      flags, strings, commands etc. which are used by
 *      all XWorkplace components.
 *
 *      As opposed to the declarations in dlgids.h, these
 *      declarations are NOT used by the NLS resource DLLs.
 *      The declarations have been separated to avoid
 *      unnecessary recompiles. (Not that it helps a whole
 *      lot.)
 *
 *      Note that with V0.9.0, all the debugging #define's have
 *      been moved to include\setup.h.
 *
 *@@include #define INCL_WINWINDOWMGR
 *@@include #define INCL_DOSMODULEMGR
 *@@include #include <os2.h>
 *@@include #include "helpers\xstring.h"    // only for setup sets and cmnDescribeError
 *@@include #include <wpfolder.h>           // only for some features
 *@@include #include "shared\common.h"
 */

/*
 *      Copyright (C) 1997-2012 Ulrich M”ller.
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

#ifndef COMMON_HEADER_INCLUDED
    #define COMMON_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Global declarations
     *
     ********************************************************************/

    // moved these here from various other locations
    // V1.0.0 (2002-08-26) [umoeller]

    // Toolkit 4 declarations

    #ifndef OBJSTYLE_LOCKEDINPLACE
        #define OBJSTYLE_LOCKEDINPLACE  0x00020000
    #endif

    // found these two in the DDK headers
    #ifndef WM_QUERYOBJECTPTR
        #define WM_QUERYOBJECTPTR       0x0407
    #endif

    #ifndef WM_SETOBJECTPTR
        #define WM_SETOBJECTPTR         0x0408
    #endif

    /*
     *  All these constants are declared as "extern" in
     *  common.h. They all used to be #define's in common.h,
     *  which put a lot of duplicates of them into the .obj
     *  files (and also stress on the compiler, since it had
     *  to do comparisons on them... and didn't even know that
     *  they were really constant).
     *
     *  These have been moved here with V0.9.7 (2001-01-17) [umoeller]
     *  and converted to DECLARE_CMN_STRING macros with
     *  V0.9.14 V0.9.14 (2001-07-31) [umoeller].
     *
     *  As a result, string constants declared here go into the
     *  final binary EXACTLY ONCE to save space.
     *
     *  DECLARE_CMN_STRING is a handy macro which saves us from
     *  keeping two string lists in both the .h and the .c file.
     *  If common.h is included from common.c, INCLUDE_COMMON_PRIVATE
     *  is #define'd, and the string is _defined_ as a global
     *  variable. Otherwise it is only _declared_ as "extern PCSZ"
     *  so other files can see it but reference the one and only global
     *  variable.
     */

    #ifdef INCLUDE_COMMON_PRIVATE
        #define DECLARE_CMN_STRING(str, def) PCSZ str = def
    #else
        #define DECLARE_CMN_STRING(str, def) extern PCSZ str
    #endif

    /********************************************************************
     *
     *   INI keys
     *
     ********************************************************************/

    /*
     * XWorkplace application:
     *
     */

    // INI key used with V0.9.1 and above
    DECLARE_CMN_STRING(INIAPP_XWORKPLACE, "XWorkplace");

    // INI key used by XFolder and XWorkplace 0.9.0;
    // this is checked for if INIAPP_XWORKPLACE is not
    // found and converted
    DECLARE_CMN_STRING(INIAPP_OLDXFOLDER, "XFolder");

    /*
     * XWorkplace keys:
     *      Add the keys you are using for storing your data here.
     *      Note: If anything has been marked as "removed" here,
     *      do not use that string, because it might still exist
     *      in a user's OS2.INI file.
     */

    // DECLARE_CMN_STRING(INIKEY_DEFAULTTITLE, "DefaultTitle");       removed V0.9.0
    DECLARE_CMN_STRING(INIKEY_GLOBALSETTINGS, "GlobalSettings");

    #ifdef __DEBUG__
        DECLARE_CMN_STRING(INIKEY_DEBUGFLAGS, "DebugSettings");
    #endif

    // DECLARE_CMN_STRING(INIKEY_XFOLDERPATH, "XFolderPath");        removed V0.81 (I think)
    DECLARE_CMN_STRING(INIKEY_ACCELERATORS, "Accelerators");
    DECLARE_CMN_STRING(INIKEY_LANGUAGECODE, "Language");
    DECLARE_CMN_STRING(INIKEY_THEMESDIR, "ThemesDir");
#ifndef __XWPLITE__
    DECLARE_CMN_STRING(INIKEY_JUSTINSTALLED, "JustInstalled");
#endif
    // DECLARE_CMN_STRING(INIKEY_DONTDOSTARTUP, "DontDoStartup");      removed V0.84 (I think)
    // DECLARE_CMN_STRING(INIKEY_LASTPID, "LastPID");            removed V0.84 (I think)
#ifndef __NOFOLDERCONTENTS__
    DECLARE_CMN_STRING(INIKEY_FAVORITEFOLDERS, "FavoriteFolders");
#endif
#ifndef __NOQUICKOPEN__
    DECLARE_CMN_STRING(INIKEY_QUICKOPENFOLDERS, "QuickOpenFolders");
#endif

    DECLARE_CMN_STRING(INIKEY_WNDPOSSTARTUP, "WndPosStartup");
    DECLARE_CMN_STRING(INIKEY_WNDPOSNAMECLASH, "WndPosNameClash");
    DECLARE_CMN_STRING(INIKEY_NAMECLASHFOCUS, "NameClashLastFocus");

    DECLARE_CMN_STRING(INIKEY_STATUSBARFONT, "SB_Font");
#ifndef __NOCFGSTATUSBARS__
    DECLARE_CMN_STRING(INIKEY_SBTEXTNONESEL, "SB_NoneSelected");
    DECLARE_CMN_STRING(INIKEY_SBTEXT_WPOBJECT, "SB_WPObject");
    DECLARE_CMN_STRING(INIKEY_SBTEXT_WPPROGRAM, "SB_WPProgram");
    DECLARE_CMN_STRING(INIKEY_SBTEXT_WPFILESYSTEM, "SB_WPDataFile");
    DECLARE_CMN_STRING(INIKEY_SBTEXT_WPURL, "SB_WPUrl");
    DECLARE_CMN_STRING(INIKEY_SBTEXT_WPDISK, "SB_WPDisk");
    DECLARE_CMN_STRING(INIKEY_SBTEXT_WPFOLDER, "SB_WPFolder");
    DECLARE_CMN_STRING(INIKEY_SBTEXTMULTISEL, "SB_MultiSelected");
    DECLARE_CMN_STRING(INIKEY_SB_LASTCLASS, "SB_LastClass");
#endif

    DECLARE_CMN_STRING(INIKEY_DLGFONT, "DialogFont");

    DECLARE_CMN_STRING(INIKEY_BOOTMGR, "RebootTo");
    DECLARE_CMN_STRING(INIKEY_AUTOCLOSE, "AutoClose");
    DECLARE_CMN_STRING(INIKEY_LASTDESKTOPPATH, "LastDesktopPath");
            // V0.9.16 (2001-10-25) [umoeller]

    DECLARE_CMN_STRING(DEFAULT_LANGUAGECODE, "001");

    // window position of "WPS Class list" window (V0.9.0)
    DECLARE_CMN_STRING(INIKEY_WNDPOSCLASSINFO, "WndPosClassInfo");

    // last directory used on "Sound" replacement page (V0.9.0)
    DECLARE_CMN_STRING(INIKEY_XWPSOUNDLASTDIR, "XWPSound:LastDir");
    // last sound scheme selected (V0.9.0)
    DECLARE_CMN_STRING(INIKEY_XWPSOUNDSCHEME, "XWPSound:Scheme");

    // boot logo .BMP file (V0.9.0)
#ifndef __NOBOOTLOGO__
    DECLARE_CMN_STRING(INIKEY_BOOTLOGOFILE, "BootLogoFile");
#endif

    // last ten selections in "Select some" (V0.9.0)
    DECLARE_CMN_STRING(INIKEY_LAST10SELECTSOME, "SelectSome");

    // last ten selections in "Batch rename" V0.9.19 (2002-06-18) [umoeller]
    DECLARE_CMN_STRING(INIKEY_LAST10BATCHSOURCE, "BatchRenameSource");
    DECLARE_CMN_STRING(INIKEY_LAST10BATCHTARGET, "BatchRenameTarget");

    // last ten filenames in "Paste" dialog V0.9.20 (2002-08-08) [umoeller]
    DECLARE_CMN_STRING(INIKEY_LAST10PASTETITLES, "PasteTitles");
    DECLARE_CMN_STRING(INIKEY_LASTPASTEFORMAT, "PasteFormat");
    DECLARE_CMN_STRING(INIKEY_LASTPASTECLASS, "PasteClass");

    // supported drives in XWPTrashCan (V0.9.1 (99-12-14) [umoeller])
    DECLARE_CMN_STRING(INIKEY_TRASHCANDRIVES, "TrashCan::Drives");

    // window pos of file operations status window V0.9.1 (2000-01-30) [umoeller]
    DECLARE_CMN_STRING(INIKEY_FILEOPSPOS, "WndPosFileOpsStatus");

    // window pos of "Partitions" view V0.9.2 (2000-02-29) [umoeller]
    DECLARE_CMN_STRING(INIKEY_WNDPOSPARTITIONS, "WndPosPartitions");

    // window position of XMMVolume control V0.9.6 (2000-11-09) [umoeller]
    DECLARE_CMN_STRING(INIKEY_WNDPOSXMMVOLUME, "WndPosXMMVolume");

    // window position of XMMCDPlayer V0.9.7 (2000-12-20) [umoeller]
    DECLARE_CMN_STRING(INIKEY_WNDPOSXMMCDPLAY, "WndPosXMMCDPlayer::");
                    // object handle appended

    // font samples (XWPFontObject) V0.9.7 (2001-01-17) [umoeller]
    DECLARE_CMN_STRING(INIKEY_FONTSAMPLEWNDPOS, "WndPosFontSample");
    DECLARE_CMN_STRING(INIKEY_FONTSAMPLESTRING, "FontSampleString");
    DECLARE_CMN_STRING(INIKEY_FONTSAMPLEHINTS, "FontSampleHints");

    // XFldStartup V0.9.9 (2001-03-19) [pr]
    DECLARE_CMN_STRING(INIKEY_XSTARTUPFOLDERS, "XStartupFolders");
    DECLARE_CMN_STRING(INIKEY_XSAVEDSTARTUPFOLDERS, "XSavedStartupFolders");

    // file dialog V0.9.11 (2001-04-18) [umoeller]
    DECLARE_CMN_STRING(INIKEY_WNDPOSFILEDLG, "WndPosFileDlg");
    DECLARE_CMN_STRING(INIKEY_FILEDLGSETTINGS, "FileDlgSettings");

    // archiving application and keys in OS2.INI
    DECLARE_CMN_STRING(INIKEY_ARCHIVE_SETTINGS, "ArchiveSettings");
    DECLARE_CMN_STRING(INIKEY_ARCHIVE_LASTBACKUP, "ArchiveLastBackup");

    // V0.9.16 (2001-10-19) [umoeller]
    DECLARE_CMN_STRING(INIKEY_ICONPAGE_LASTDIR, "IconPageLastDir");

    // keyboard macros V0.9.20 (2002-07-03) [umoeller]
    DECLARE_CMN_STRING(INIKEY_KEYBOARD_MACROS, "KeybMacros");

    // split view window positions V1.0.0 (2002-08-21) [umoeller]
    DECLARE_CMN_STRING(INIAPP_FDRSPLITVIEWPOS, "XWorkplace:SplitViews");

    /*
     * file type hierarchies:
     *
     */

    // application for file type hierarchies
    DECLARE_CMN_STRING(INIAPP_XWPFILETYPES, "XWorkplace:FileTypes");   // added V0.9.0
    DECLARE_CMN_STRING(INIAPP_XWPFILEFILTERS, "XWorkplace:FileFilters"); // added V0.9.0

    DECLARE_CMN_STRING(INIAPP_REPLACEFOLDERREFRESH, "ReplaceFolderRefresh");
                                        // V0.9.9 (2001-01-31) [umoeller]

    /*
     * XCenter INI keys:
     *
     * added V0.9.14 (2001-08-23) [pr]
     */

    DECLARE_CMN_STRING(INIAPP_XCENTER, "XWorkplace:XCenter");
    DECLARE_CMN_STRING(INIKEY_RUNHISTORY, "RunHistory");

    // V0.9.16 (2001-10-02) [umoeller]:
    // moved many strings to include\shared\wphandle.h

    /********************************************************************
     *
     *   XWorkplace object IDs
     *
     ********************************************************************/

    // all of these have been redone with V0.9.2

    // folders
    DECLARE_CMN_STRING(XFOLDER_MAINID, "<XWP_MAINFLDR>");
    DECLARE_CMN_STRING(XFOLDER_CONFIGID, "<XWP_CONFIG>");

    DECLARE_CMN_STRING(XFOLDER_STARTUPID, "<XWP_STARTUP>");
    DECLARE_CMN_STRING(XFOLDER_SHUTDOWNID, "<XWP_SHUTDOWN>");
    DECLARE_CMN_STRING(XFOLDER_FONTFOLDERID, "<XWP_FONTFOLDER>");

    DECLARE_CMN_STRING(XFOLDER_WPSID, "<XWP_WPS>");
#ifndef __NOOS2KERNEL__
    DECLARE_CMN_STRING(XFOLDER_KERNELID, "<XWP_KERNEL>");
#endif
    DECLARE_CMN_STRING(XFOLDER_SCREENID, "<XWP_SCREEN>");

#ifndef __XWPLITE__
    DECLARE_CMN_STRING(XFOLDER_MEDIAID, "<XWP_MEDIA>");
    DECLARE_CMN_STRING(XFOLDER_CLASSLISTID, "<XWP_CLASSLIST>");
#endif

    DECLARE_CMN_STRING(XFOLDER_TRASHCANID, "<XWP_TRASHCAN>");
    DECLARE_CMN_STRING(XFOLDER_XCENTERID, "<XWP_XCENTER>");
    DECLARE_CMN_STRING(XFOLDER_STRINGTPLID, "<XWP_STRINGTPL>"); // V0.9.9

    DECLARE_CMN_STRING(XFOLDER_INTROID, "<XWP_INTRO>");
    DECLARE_CMN_STRING(XFOLDER_USERGUIDE, "<XWP_REF>");

    // DECLARE_CMN_STRING(XWORKPLACE_ARCHIVE_MARKER, "xwparchv.tmp");
                // archive marker file in Desktop directory V0.9.4 (2000-08-03) [umoeller]
                // removed V0.9.13 (2001-06-14) [umoeller]

    /********************************************************************
     *
     *   WPS class names V0.9.14 (2001-07-31) [umoeller]
     *
     ********************************************************************/

    DECLARE_CMN_STRING(G_pcszXFldObject, "XFldObject");
    DECLARE_CMN_STRING(G_pcszXWPFileSystem, "XWPFileSystem");
    DECLARE_CMN_STRING(G_pcszXFolder, "XFolder");
    DECLARE_CMN_STRING(G_pcszXFldDisk, "XFldDisk");
    DECLARE_CMN_STRING(G_pcszXFldDesktop, "XFldDesktop");
    DECLARE_CMN_STRING(G_pcszXFldDataFile, "XFldDataFile");
    DECLARE_CMN_STRING(G_pcszXWPVCard, "XWPVCard");
    DECLARE_CMN_STRING(G_pcszXWPImageFile, "XWPImageFile");
    DECLARE_CMN_STRING(G_pcszXWPProgramFile, "XWPProgramFile");
    DECLARE_CMN_STRING(G_pcszXWPSound, "XWPSound");
    DECLARE_CMN_STRING(G_pcszXWPMouse, "XWPMouse");
    DECLARE_CMN_STRING(G_pcszXWPKeyboard, "XWPKeyboard");

    DECLARE_CMN_STRING(G_pcszXWPSetup, "XWPSetup");
#ifndef __NOOS2KERNEL__
    DECLARE_CMN_STRING(G_pcszXFldSystem, "XFldSystem");
#endif
    DECLARE_CMN_STRING(G_pcszXFldWPS, "XFldWPS");
    DECLARE_CMN_STRING(G_pcszXWPScreen, "XWPScreen");
#ifndef __XWPLITE__
    DECLARE_CMN_STRING(G_pcszXWPMedia, "XWPMedia");
#endif
#ifndef __NOXWPSTARTUP__
    DECLARE_CMN_STRING(G_pcszXFldStartup, "XFldStartup");
#endif
#ifndef __NOXSHUTDOWN__
    DECLARE_CMN_STRING(G_pcszXFldShutdown, "XFldShutdown");
#endif
    DECLARE_CMN_STRING(G_pcszXWPClassList, "XWPClassList");
    DECLARE_CMN_STRING(G_pcszXWPTrashCan, "XWPTrashCan");
    DECLARE_CMN_STRING(G_pcszXWPTrashObject, "XWPTrashObject");
    DECLARE_CMN_STRING(G_pcszXWPString, "XWPString");

    DECLARE_CMN_STRING(G_pcszXCenterReal, "XCenter");

    DECLARE_CMN_STRING(G_pcszXWPFontFolder, "XWPFontFolder");
    DECLARE_CMN_STRING(G_pcszXWPFontFile, "XWPFontFile");
    DECLARE_CMN_STRING(G_pcszXWPFontObject, "XWPFontObject");

#ifndef __XWPLITE__
    DECLARE_CMN_STRING(G_pcszXMMCDPlayer, "XMMCDPlayer");
#endif
    DECLARE_CMN_STRING(G_pcszXMMVolume, "XMMVolume");

    DECLARE_CMN_STRING(G_pcszXWPNetwork, "XWPNetwork");
    DECLARE_CMN_STRING(G_pcszXWPNetServer, "XWPNetServer");

    DECLARE_CMN_STRING(G_pcszXWPProgram, "XWPProgram");

    DECLARE_CMN_STRING(G_pcszXWPLink, "XWPLink");               // V1.0.2 (2003-02-07) [umoeller]

    DECLARE_CMN_STRING(G_pcszWPObject, "WPObject");
    DECLARE_CMN_STRING(G_pcszWPFileSystem, "WPFileSystem");
    DECLARE_CMN_STRING(G_pcszWPFolder, "WPFolder");
    DECLARE_CMN_STRING(G_pcszWPDisk, "WPDisk");
    DECLARE_CMN_STRING(G_pcszWPDesktop, "WPDesktop");
    DECLARE_CMN_STRING(G_pcszWPDataFile, "WPDataFile");
    DECLARE_CMN_STRING(G_pcszWPProgram, "WPProgram");
    DECLARE_CMN_STRING(G_pcszWPProgramFile, "WPProgramFile");
    DECLARE_CMN_STRING(G_pcszWPKeyboard, "WPKeyboard");
    DECLARE_CMN_STRING(G_pcszWPMouse, "WPMouse");
    DECLARE_CMN_STRING(G_pcszWPCountry, "WPCountry");
    DECLARE_CMN_STRING(G_pcszWPSound, "WPSound");
    DECLARE_CMN_STRING(G_pcszWPSystem, "WPSystem");
    DECLARE_CMN_STRING(G_pcszWPPower, "WPPower");
    DECLARE_CMN_STRING(G_pcszWPWinConfig, "WPWinConfig");
    DECLARE_CMN_STRING(G_pcszWPColorPalette, "WPColorPalette");
    DECLARE_CMN_STRING(G_pcszWPFontPalette, "WPFontPalette");
    DECLARE_CMN_STRING(G_pcszWPSchemePalette, "WPSchemePalette");

    DECLARE_CMN_STRING(G_pcszWPLaunchPad, "WPLaunchPad");
    DECLARE_CMN_STRING(G_pcszSmartCenter, "SmartCenter");

    DECLARE_CMN_STRING(G_pcszWPSpool, "WPSpool");
    DECLARE_CMN_STRING(G_pcszWPMinWinViewer, "WPMinWinViewer");
    DECLARE_CMN_STRING(G_pcszWPShredder, "WPShredder");
    DECLARE_CMN_STRING(G_pcszWPClock, "WPClock");

    DECLARE_CMN_STRING(G_pcszWPStartup, "WPStartup");
    DECLARE_CMN_STRING(G_pcszWPTemplates, "WPTemplates");
    DECLARE_CMN_STRING(G_pcszWPDrives, "WPDrives");

    // ObjectDesktop classes
    // V0.9.19 (2002-04-17) [umoeller]
    DECLARE_CMN_STRING(G_pcszTabLaunchPad, "TSTabFolder");
    DECLARE_CMN_STRING(G_pcszControlCenter, "TSCPad");

    /********************************************************************
     *
     *   Thread object windows
     *
     ********************************************************************/

    // object window class names (added V0.9.0)
    DECLARE_CMN_STRING(WNDCLASS_WORKEROBJECT, "XWPWorkerObject");
    DECLARE_CMN_STRING(WNDCLASS_QUICKOBJECT, "XWPQuickObject");
    DECLARE_CMN_STRING(WNDCLASS_FILEOBJECT, "XWPFileObject");

    DECLARE_CMN_STRING(WNDCLASS_THREAD1OBJECT, "XWPThread1Object");
    DECLARE_CMN_STRING(WNDCLASS_SUPPLOBJECT, "XWPSupplFolderObject");
    DECLARE_CMN_STRING(WNDCLASS_APIOBJECT, "XWPAPIObject");

    // ID's of XWorkplace object windows (added V0.9.0)
    #define ID_THREAD1OBJECT        0x1234
    #define ID_WORKEROBJECT         0x1235
    #define ID_QUICKOBJECT          0x1236
    #define ID_FILEOBJECT           0x1237

    /********************************************************************
     *
     *   Other constants
     *
     ********************************************************************/

    #ifndef __XWPLITE__
        #define XWORKPLACE_STRING "XWorkplace"

        DECLARE_CMN_STRING(ENTITY_OS2, "OS/2");
        DECLARE_CMN_STRING(ENTITY_WINOS2, "Win-OS/2");
        DECLARE_CMN_STRING(ENTITY_WARPCENTER, "WarpCenter");
        DECLARE_CMN_STRING(ENTITY_XCENTER, "XCenter");
        DECLARE_CMN_STRING(ENTITY_XBUTTON, "X-Button");
        DECLARE_CMN_STRING(ENTITY_XSHUTDOWN, "XShutdown");
        DECLARE_CMN_STRING(ENTITY_PAGER, "XPager");

        #define F_ALLOW_BOOTROOT_LOGFILE  TRUE

    #else
        #define XWORKPLACE_STRING "eComStation"

        DECLARE_CMN_STRING(ENTITY_OS2, "eComStation");
        DECLARE_CMN_STRING(ENTITY_WINOS2, "Win16");
        DECLARE_CMN_STRING(ENTITY_WARPCENTER, "eComCenter");
        DECLARE_CMN_STRING(ENTITY_XCENTER, "eCenter");
        DECLARE_CMN_STRING(ENTITY_XBUTTON, "eButton");
        DECLARE_CMN_STRING(ENTITY_XSHUTDOWN, "eShutdown");
        DECLARE_CMN_STRING(ENTITY_PAGER, "ePager");

        #define F_ALLOW_BOOTROOT_LOGFILE  FALSE

    #endif

    #define STARTUPLOG      "wpstart.log"

    DECLARE_CMN_STRING(ENTITY_XWORKPLACE, XWORKPLACE_STRING);

    DECLARE_CMN_STRING(WC_WPFOLDERWINDOW, "wpFolder window");

    /********************************************************************
     *
     *   Various other identifiers/flag declarations
     *
     ********************************************************************/

    // offset by which the controls should be moved
    // when converting buttons to Warp 4 notebook style
    // (using winhAssertWarp4Notebook); this is in
    // "dialog units"
    #define WARP4_NOTEBOOK_OFFSET   14

    // miscellaneae
    #define LANGUAGECODELENGTH      30

    /*
     *  XWorkplace WM_USER message spaces:
     *      Even though the various object windows could use the
     *      same WM_USER messages since they only have to be
     *      unique for each object window, just to be sure, we
     *      make sure each message has a unique value throughout
     *      the system. This avoids problems in case someone sends
     *      a message to the wrong object window.
     *
     *                up to WM_USER+100:    common.h
     *      WM_USER+100 ... WM_USER+149:    folder.h, statbars.h
     *      WM_USER+150 ... WM_USER+179:    Worker thread, xthreads.h
     *      WM_USER+180 ... WM_USER+199:    Quick thread, xthreads.h
     *      WM_USER+200 ... WM_USER+249:    File thread, xthreads.h
     *      WM_USER+250 ... WM_USER+269:    media thread, media.h
     *      WM_USER+270 ... WM_USER+299:    thread-1 obj wnd, kernel.h
     *      WM_USER+300 ... WM_USER+499:    hook, Daemon and XPager
     */

    // common dlg msgs for settings notebook dlg funcs
    #define XM_SETTINGS2DLG         (WM_USER + 90)    // set controls
    #define XM_DLG2SETTINGS         (WM_USER + 91)    // read controls
    #define XM_ENABLEITEMS          (WM_USER + 92)    // enable/disable controls

    // misc
    #define XM_UPDATE               (WM_USER + 93) // in dlgs
    // #define XM_SETLONGTEXT          (WM_USER+94) // for cmnMessageBox
            // removed V0.9.13 (2001-06-23) [umoeller]
    #define XM_CRASH                (WM_USER + 95) // test exception handlers

    // fill container; used with class list dialogs
    #define WM_FILLCNR              (WM_USER + 96)
                // value changed; moved this here from classes.h
                // (thanks Martin Lafaix)
                // V0.9.6 (2000-11-07) [umoeller]

    // notebook.c messages: moved here V0.9.6 (2000-11-07) [umoeller]
    #define XNTBM_UPDATE            (WM_USER + 97)  // update

    #define XM_DISPLAYERROR         (WM_USER + 98)
            // V0.9.16 (2001-10-19) [umoeller]

    // common value for indicating that a Global Setting
    // is to be used instead of an instance's one
    #define SET_DEFAULT             255

    // new XWorkplace system sounds indices
    // (in addition to those def'd by helpers\syssound.h)
#ifndef __NOXSYSTEMSOUNDS__
    #define MMSOUND_XFLD_SHUTDOWN   555     // shutdown
    #define MMSOUND_XFLD_RESTARTWPS 556     // restart Desktop
    #define MMSOUND_XFLD_CTXTOPEN   558     // context (sub)menu opened
    #define MMSOUND_XFLD_CTXTSELECT 559     // menu item selected
    #define MMSOUND_XFLD_CNRDBLCLK  560     // folder container double-click
    #define MMSOUND_XFLD_HOTKEYPRSD 561     // XWP global hotkey pressed
                                            // added V0.9.3 (2000-04-20) [umoeller]
#endif

    // default style used for XWorkplace tooltip controls
    #ifdef COMCTL_HEADER_INCLUDED
        #define XWP_TOOLTIP_STYLE (TTS_SHADOW /* | TTS_ROUNDED */ | TTF_SHYMOUSE | TTS_ALWAYSTIP)
    #endif

    /********************************************************************
     *
     *   Global variables (read-only)
     *
     ********************************************************************/

    // this variable is FALSE only if Warp 3 is running
    // (set from shared\init.c)
    extern BOOL     G_fIsWarp4;

    extern ULONG    *G_pulVarMenuOfs;       // V1.0.0 (2002-08-28) [umoeller]

    /********************************************************************
     *
     *   XWorkplace Global Settings
     *
     ********************************************************************/

    // shutdown settings bits: composed by the
    // Shutdown settings pages, stored in
    // cmnQuerySetting(sfXShutdown,)
    // passed to xfInitiateShutdown

    /* the following removed with V0.9.0;
       there are new flags in GLOBALSETTINGS for this
    #define XSD_DTM_SYSTEMSETUP     0x00001
    #define XSD_DTM_SHUTDOWN        0x00002
    #define XSD_DTM_LOCKUP          0x00004 */

    // #define XSD_ENABLED             0x00010
    // #define XSD_CONFIRM             0x000020     // removed V0.9.16 (2002-01-09) [umoeller]
    #define XSD_REBOOT              0x000040
    // #define XSD_RESTARTWPS          0x00100
    #define XSD_DEBUG               0x001000
    #define XSD_AUTOCLOSEVIO        0x002000
    #define XSD_WPS_CLOSEWINDOWS    0x004000
    #define XSD_LOG                 0x008000
    #define XSD_ANIMATE_SHUTDOWN    0x010000     // renamed V0.9.3 (2000-05-22) [umoeller]
    #define XSD_POWEROFF            0x020000     // renamed V1.0.5 (2006-06-26) [pr]
    #define XSD_DELAY               0x040000     // renamed V1.0.5 (2006-06-26) [pr]
    #define XSD_ANIMATE_REBOOT      0x080000     // added V0.9.3 (2000-05-22) [umoeller]
    #define XSD_EMPTY_TRASH         0x100000     // added V0.9.4 (2000-08-03) [umoeller]
#ifndef __EASYSHUTDOWN__
    #define XSD_WARPCENTERFIRST     0x200000     // added V0.9.7 (2000-12-08) [umoeller]
#endif
    #define XSD_CANDESKTOPALTF4     0x400000     // added V0.9.16 (2002-01-04) [umoeller]
#ifndef __EASYSHUTDOWN__
    #define XSD_NOCONFIRM           0x800000     // added V0.9.16 (2002-01-09) [umoeller]
#endif
    #define XSD_ACPIPOWEROFF        0x1000000    // added V1.0.5 (2006-06-26) [pr]

    // flags for GLOBALSETTINGS.ulConfirmEmpty
    #define TRSHCONF_EMPTYTRASH     0x00000001
    #define TRSHCONF_DESTROYOBJ     0x00000002

#ifndef __NOCFGSTATUSBARS__
    // flags for GLOBALSETTINGS.bDereferenceShadows
    #define STBF_DEREFSHADOWS_SINGLE        0x01
    #define STBF_DEREFSHADOWS_MULTIPLE      0x02
#endif

    /*
     *@@ XWPSETTING:
     *      enumeration for checking if XWorkplace features
     *      are enabled. This replaces the global settings
     *      present before V0.9.16.
     *
     *      See cmnQuerySetting() for details.
     *
     *@@added V0.9.16 (2001-10-11) [umoeller]
     *@@changed V1.0.4 (2005-08-09) [pr]: Removed a load of conditional compiles to make eWP work
     */

    typedef enum _XWPSETTING
    {
        NULLSETTING = 0,

#ifndef __NOICONREPLACEMENTS__
        sfIconReplacements,
#endif
        sfHelpReplacements,              // added V0.9.20 (2002-07-19) [umoeller]
        // sfMoveRefreshNow,        removed V0.9.19 (2002-04-17) [umoeller]

// #ifndef __ALWAYSSUBCLASS__
//         sfNoSubclassing,         removed V1.0.0 (2002-09-12) [umoeller]
// #endif

#ifndef __NOFOLDERCONTENTS__
        // sfAddFolderContentItem,  removed V0.9.19 (2002-04-17) [umoeller]
        sfFolderContentShowIcons,
#endif
#ifndef __NOFDRDEFAULTDOCS__
        sfFdrDefaultDoc,
            // folder default documents enabled?
            // "Workplace Shell" "View" page
        sfFdrDefaultDocView,
            // "default doc = folder default view"
            // "Workplace Shell" "View" page
#endif
#ifndef __NOBOOTLOGO__
        sfBootLogo,
        sulBootLogoStyle,
            // XFldDesktop "Startup" page:
            // boot logo style:
            //      0 = transparent
            //      1 = blow-up
#endif
        sfReplaceFilePage,
#ifndef __NOCFGSTATUSBARS__
        sfStatusBars,
#endif

        sfToolBars,             // added V1.0.1 (2002-11-30) [umoeller]

#ifndef __NEVERCHECKDESKTOP__
        sfCheckDesktop,
        sfPrePopulateDesktop,
#endif
#ifndef __NOSNAPTOGRID__
        sfSnap2Grid,
        sfAddSnapToGridDefault,
            // V0.9.0, was: AddSnapToGridItem
            // default setting for adding "Snap to grid";
            // can be overridden in XFolder instance settings

        // "snap to grid" values
        sulGridX,
        sulGridY,
        sulGridCX,
        sulGridCY,
#endif
        sfFolderHotkeys,
        sfFolderHotkeysDefault,
            // V0.9.0, was: FolderHotkeysDefault
            // default setting for enabling folder hotkeys;
            // can be overridden in XFolder instance settings
        sfShowHotkeysInMenus,
            // on XFldWPS "Hotkeys" page

// #ifndef __ALWAYSRESIZESETTINGSPAGES__        removed this setting V1.0.1 (2002-12-14) [umoeller]
//         sfResizeSettingsPages,
// #endif

        sfReplaceIconPage,
        sfReplaceFileExists,
        sfFixClassTitles,
        sfReplaceArchiving,
#ifndef __NEVERNEWFILEDLG__
        sfNewFileDlg,
#endif
#ifndef __NOXSHUTDOWN__
        sfXShutdown,
        sfRestartDesktop,
        sflXShutdown,
            // XSD_* shutdown settings
        sulSaveINIS,
            // XShutdown: save-INIs method:
            // -- 0: new method (xprf* APIs)
            // -- 1: old method (Prf* APIs)
            // -- 2: do not save
#endif
        sfExtendedSorting,
        sfXWPHook,
#ifndef __NOPAGER__
        sfEnableXPager,
            // XWPSetup "XPager virtual desktops"; this will cause
            // XDM_STARTSTOPPAGER to be sent to the daemon
#endif
// #ifndef __NEVEREXTASSOCS__
//         sfExtAssocs,                // removed V1.0.1 (2002-12-15) [umoeller]
//         sfDatafileOBJHANDLE,        // added V0.9.20 (2002-08-04) [umoeller]
//                                     // moved to turbo folders setting V1.0.1 (2002-12-15) [umoeller]
// #endif

        // replacement owner-draw flags; if any are set, we
        // run our own code in the subclassed folder winproc
        // to paint icons
        // V0.9.20 (2002-07-25) [umoeller]
        sflOwnerDrawIcons,
            // OWDRFL_LAZYICONS | OWDRFL_SHADOWOVERLAY;
            // all OWDRFL_* flags are defined as passthru in xfobj.idl

#ifndef __NEVERREPLACEDRIVENOTREADY__
        sfReplaceDriveNotReady,
#endif

        sfReplacePaste,             // V0.9.20 (2002-08-08) [umoeller]

        /* sfTrashDelete,
        sfReplaceTrueDelete, */
        // replaced these
        // V0.9.19 (2001-04-13) [umoeller]
        sfReplaceDelete,            // in XWPSetup
        sfAlwaysTrueDelete,         // in trash can settings notebook; only
                                    // effective if sfReplaceDelete == TRUE
#ifndef __NOBOOTUPSTATUS__
        sfShowBootupStatus,
#endif

#ifndef __NOTURBOFOLDERS__
        sfTurboFolders,             // warning: this will return the setting
                                    // that was once determined on WPS startup
        sfDatafileOBJHANDLE,        // added V0.9.20 (2002-08-04) [umoeller]
                                    // moved to turbo folders setting V1.0.1 (2002-12-15) [umoeller]#endif
#endif

        sfFdrSplitViews,            // new folder split views
                                    // V1.0.0 (2002-09-09) [umoeller]

        // menu settings
        sulVarMenuOfs,
            // variable menu offset, "Paranoia" page

        sfMenuCascadeMode,

/* all removed V0.9.19 (2002-04-17) [umoeller]
        sflDefaultMenuItems,
            // ready-made CTXT_* flags for wpFilterPopupMenu

        sfFileAttribs,
            // add attributes menu

        sfRemoveLockInPlaceItem,
            // XFldObject, Warp 4 only
        sfRemoveFormatDiskItem,
            // XFldDisk
        sfRemoveCheckDiskItem,
            // XFldDisk
        sfRemoveViewMenu,
            // XFolder, Warp 4 only
        sfRemovePasteItem,
            // XFldObject, Warp 4 only
        sfAddCopyFilenameItem,
            // default setting for "Copy filename" (XFldDataFile)
            // can be overridden in XFolder instance settings
        sfAddSelectSomeItem,
            // XFolder: enable "Select by name"
        sfExtendFldrViewMenu,
            // XFolder: extend Warp 4 "View" submenu
*/

        sfFixLockInPlace,
            // "Workplace Shell" menus p3: submenu, checkmark

/* all removed V0.9.19 (2002-04-17) [umoeller]

#ifndef __NOXSHUTDOWN__
        sfDTMShutdownMenu,
#endif

        // Desktop menu items
        sfDTMSort,
        sfDTMArrange,
        sfDTMSystemSetup,
        sfDTMLockup,
#ifndef __NOXSHUTDOWN__
        sfDTMShutdown,
#endif
        sfDTMLogoffNetwork,
            // "Logoff network now" desktop menu item (XFldDesktop)
*/

        // folder view settings
        sfFullPath,
            // enable "full path in title"
        sfKeepTitle,
            // "full path in title": keep existing title
        sulMaxPathChars,
            // maximum no. of chars for "full path in title"
        sfRemoveX,
        sfAppdParam,
        sulTemplatesOpenSettings,
            // open settings after creating from template;
            // 0: do nothing after creation
            // 1: open settings notebook
            // 2: make title editable
        sfTemplatesReposition,
            // reposition new objects after creating from template
        sfTreeViewAutoScroll,
            // XFolder

        // status bar settings
        sfDefaultStatusBarVisibility,
            // V0.9.0, was: StatusBar;
            // default visibility of status bars (XFldWPS),
            // can be overridden in XFolder instance settings
            // (unlike fEnableStatusBars below, XWPSetup)
        sulSBStyle,
            // status bar style
        slSBBgndColor,
        slSBTextColor,
            // status bar colors; can be changed via drag'n'drop
        sflSBForViews,
            // XFldWPS: SBV_xxx flags
#ifndef __NOCFGSTATUSBARS__
        sflDereferenceShadows,
            // XFldWPS "Status bars" page 2:
            // deference shadows flag
            // changed V0.9.5 (2000-10-07) [umoeller]: now bit flags...
            // -- STBF_DEREFSHADOWS_SINGLE        0x01
            // -- STBF_DEREFSHADOWS_MULTIPLE      0x02
#endif

        // tool bar settings V1.0.1 (2002-12-08) [umoeller]
        sfDefaultToolBarVisibility,
        sflToolBarStyle,
                    // a combination of the following tool bar style flags:
                    // -- TBBS_TEXT
                    // -- TBBS_MINIICON or TBBS_BIGICON
                    // -- TBBS_FLAT
                    // -- TBBS_HILITE
        sflTBForViews,
            // XFldWPS: SBV_xxx flags
        sfTBToolTips,

        // startup settings
        sfShowStartupProgress,
            // XFldStartup
        sulStartupInitialDelay,
            // XFldStartup: initial delay
        sulStartupObjectDelay,
            // was: ulStartupDelay;
            // there's a new ulStartupInitialDelay with V0.9.4 (bottom)
            // XFldStartup
        sfNumLockStartup,
            // XFldDesktop "Startup": set NumLock to ON on Desktop startup
        sfWriteXWPStartupLog,
            // V0.9.14 (2001-08-21) [umoeller]

        // folder sort settings
        sfAlwaysSort,
            // default "always sort" flag (BOOL)
        sfFoldersFirst,
            // global sort setting for "folders first"
            // (TRUE or FALSE)
        slDefSortCrit,
            // new global sort criterion (moved this down here
            // because the value is incompatible with the earlier
            // setting above, which has been disabled);
            // this is a LONG because it can have negative values
            // (see XFolder::xwpSetFldrSort)

#ifndef __NOPARANOIA__
        // paranoia settings
        sfNoExcptBeeps,
            // XWPSetup "Paranoia": disable exception beeps
        sfUse8HelvFont,
            // XWPSetup "Paranoia": use "8.Helv" font for dialogs;
            // on Warp 3, this is enabled per default
        sulDefaultWorkerThreadPriority,
            // XWPSetup "Paranoia": default priority of Worker thread:
            //      0: idle +/-0
            //      1: idle +31
            //      2: regular +/-0
        sfWorkerPriorityBeep,
            // XWPSetup "Paranoia": beep on priority change
        // sfNoFreakyMenus,     // removed V1.0.0 (2002-08-26) [umoeller]
            // on XWPSetup "Paranoia" page
#endif

        // misc
#ifndef __NOXSYSTEMSOUNDS__
        sfXSystemSounds,
            // XWPSetup: enable extended system sounds
#endif

        susLastRebootExt,
            // XShutdown: last extended reboot item
        sflTrashConfirmEmpty,
            // TRSHEMPTY_* flags
        sflIntroHelpShown,
            // HLPS_* flags for various classes, whether
            // an introductory help page has been shown
            // the first time it's been opened
                #define HLPS_NOSHOWCLASSLIST    0x00000001
                #define HLPS_NOSHOWXCENTER      0x00000002      // added V1.0.1 (2003-02-02) [umoeller]
#ifdef __XWPLITE__
                #define HLPS_NOSHOWDESKTOP      0x00000004      // added V1.0.1 (2003-02-02) [umoeller]
#endif

        sfFdrAutoRefreshDisabled,
            // "Folder auto-refresh" on "Workplace Shell" "View" page;
            // this only has an effect if folder auto-refresh has
            // been replaced in XWPSetup in the first place

        sulDefaultFolderView,
            // "default folder view" on XFldWPS "View" page:
            // -- 0: inherit from parent (default, standard WPS)
            // -- OPEN_CONTENTS (1): icon view
            // -- OPEN_TREE (101): tree view
            // -- OPEN_DETAILS (102): details view

        // the following are new with V0.9.19
        sflMenuObjectWPS,       // CTXT_* flags, bit set means remove item
        sflMenuObjectXWP,       // XWPCTXT_* flags, bit set means remove item
        sflMenuFileWPS,         // CTXT_* flags, bit set means remove item
        sflMenuFileXWP,         // XWPCTXT_* flags, bit set means remove item
        sflMenuFolderWPS,       // CTXT_* flags, bit set means remove item
        sflMenuFolderXWP,       // XWPCTXT_* flags, bit set means remove item
        sflMenuDesktopWPS,      // CTXT_* flags, bit set means remove item
        sflMenuDesktopXWP,      // XWPCTXT_* flags, bit set means remove item
        sflMenuDiskWPS,         // CTXT_* flags, bit set means remove item
        sflMenuDiskXWP,         // XWPCTXT_* flags, bit set means remove item

        ___LAST_SETTING
    } XWPSETTING;

    /*
     *@@ SETTINGINFO:
     *      gives detailed information about an
     *      XWPSETTING. This is used to find the
     *      INI key for loading and writing the
     *      data as well as converting the old
     *      XWorkplace GLOBALSETTINGS structure
     *      into an XWPSETTING array. Besides,
     *      for each setting, this gives a default
     *      value if it was not found at startup,
     *      or to reset it from a notebook page.
     *
     *      An array of these structures exists
     *      as a static, private, global variable.
     *
     *@@added V0.9.16 (2002-01-05) [umoeller]
     */

    typedef struct _SETTINGINFO
    {
        XWPSETTING      s;                  // setting this item relates to
        ULONG           ulOffsetIntoOld;    // offset into OLDGLOBALSETTINGS;
                                            // if -1, no corresponding entry exists
        BYTE            cbOld;              // count of bytes in OLDGLOBALSETTINGS (1, 2, or 4)
        ULONG           ulSettingsPageID;   // SP_* settings page ID of this setting
        ULONG           ulDefaultValue;     // default value for this setting
        PCSZ            pcszIniKey;         // INI key for this setting
    } SETTINGINFO;

    typedef const struct _SETTINGINFO *PCSETTINGINFO;

    PCSETTINGINFO cmnFindSettingInfo(XWPSETTING s);

    ULONG XWPENTRY cmnLoadOneSetting(PCSETTINGINFO pThis);

    BOOL XWPENTRY cmnSetDefaultSettings(USHORT usSettingsPage);

    #ifndef __DEBUG__
        ULONG XWPENTRY cmnQuerySetting(XWPSETTING s);
    #else
        #define cmnQuerySetting(s) cmnQuerySettingDebug(s, __FILE__, __LINE__, __FUNCTION__)
        ULONG XWPENTRY cmnQuerySettingDebug(XWPSETTING s,
                                            PCSZ pcszSourceFile,
                                            ULONG ulLine,
                                            PCSZ pcszFunction);
    #endif

    BOOL XWPENTRY cmnSetSetting(XWPSETTING s, ULONG ulValue);

    PCSZ XWPENTRY cmnQueryStatusBarSetting(USHORT usSetting);

    BOOL XWPENTRY cmnSetStatusBarSetting(USHORT usSetting, PSZ pszSetting);

    ULONG XWPENTRY cmnQueryStatusBarHeight(VOID);

    /*
     *@@ SETTINGSBACKUP:
     *
     */

    typedef struct _SETTINGSBACKUP
    {
        XWPSETTING      s;
        ULONG           ul;
    } SETTINGSBACKUP, *PSETTINGSBACKUP;

    PSETTINGSBACKUP XWPENTRY cmnBackupSettings(const XWPSETTING *paSettings,
                                               ULONG cItems);

    VOID XWPENTRY cmnRestoreSettings(PSETTINGSBACKUP paSettingsBackup,
                                     ULONG cItems);

    /* ******************************************************************
     *
     *   Modules and paths
     *
     ********************************************************************/

    HMODULE XWPENTRY cmnQueryMainCodeModuleHandle(VOID);

    #define cmnQueryMainModuleHandle #error Func prototype has changed.

    PCSZ XWPENTRY cmnQueryMainCodeModuleFilename(VOID);

    HMODULE XWPENTRY cmnQueryMainResModuleHandle(VOID);
    typedef HMODULE XWPENTRY CMNQUERYMAINRESMODULEHANDLE(VOID);
    typedef CMNQUERYMAINRESMODULEHANDLE *PCMNQUERYMAINRESMODULEHANDLE;

    BOOL XWPENTRY cmnQueryXWPBasePath(PSZ pszPath);

    PCSZ XWPENTRY cmnQueryLanguageCode(VOID);

    BOOL XWPENTRY cmnSetLanguageCode(PCSZ pcszLanguage);

    PCSZ XWPENTRY cmnQueryHelpLibrary(VOID);
    typedef PCSZ XWPENTRY CMNQUERYHELPLIBRARY(VOID);
    typedef CMNQUERYHELPLIBRARY *PCMNQUERYHELPLIBRARY;

    VOID cmnHelpNotFound(ULONG ulPanelID);

    #ifdef SOM_WPObject_h
        BOOL XWPENTRY cmnDisplayHelp(WPObject *somSelf,
                                     ULONG ulPanelID);
    #endif

    PCSZ XWPENTRY cmnQueryMessageFile(VOID);

// #ifndef __NOICONREPLACEMENTS__
//     HMODULE XWPENTRY cmnQueryIconsDLL(VOID);
// #endif

#ifndef __NOBOOTLOGO__
    PSZ XWPENTRY cmnQueryBootLogoFile(VOID);
#endif

    HMODULE XWPENTRY cmnQueryNLSModuleHandle(BOOL fEnforceReload);
    typedef HMODULE XWPENTRY CMNQUERYNLSMODULEHANDLE(BOOL fEnforceReload);
    typedef CMNQUERYNLSMODULEHANDLE *PCMNQUERYNLSMODULEHANDLE;

    HWND XWPENTRY cmnLoadDlg(HWND hwndOwner, PFNWP pfnwp, ULONG idResource, PVOID pvCreateParam);
    typedef HWND XWPENTRY CMNLOADDLG(HWND hwndOwner, PFNWP pfnwp, ULONG idResource, PVOID pvCreateParam);
    typedef CMNLOADDLG *PCMNLOADDLG;

    HWND XWPENTRY cmnLoadMenu(HWND hwndOwner,
                              HMODULE hmod,
                              ULONG id);

    /* ******************************************************************
     *
     *   Error logging
     *
     ********************************************************************/

    VOID XWPENTRY cmnLog(PCSZ pcszSourceFile,
                         ULONG ulLine,
                         PCSZ pcszFunction,
                         PCSZ pcszFormat,
                         ...);

    /* ******************************************************************
     *
     *   NLS strings
     *
     ********************************************************************/

    VOID XWPENTRY cmnInitEntities(VOID);

    #if 0       // removed, use nls* functions instead
        void XWPENTRY cmnLoadString(HAB habDesktop,
                                    HMODULE hmodResource,
                                    ULONG ulID,
                                    PSZ *ppsz,
                                    PULONG pulLength);
    #endif

    PCSZ XWPENTRY cmnGetString(ULONG ulStringID);
    typedef PCSZ XWPENTRY CMNGETSTRING(ULONG ulStringID);
    typedef CMNGETSTRING *PCMNGETSTRING;

    ULONG XWPENTRY cmnGetString2(PSZ pszBuf,
                                 ULONG ulStringID,
                                 ULONG cbBuf);

    ULONG XWPENTRY cmnGetStringNoMnemonic(PSZ pszBuf,
                                          ULONG ulStringID,
                                          ULONG cbBuf);

    VOID XWPENTRY cmnLoadDaemonNLSStrings(VOID);

    #ifdef DIALOG_HEADER_INCLUDED
        extern const CONTROLDEF
                          G_OKButton,
                          G_CancelButton,
                          G_UndoButton,
                          G_DefaultButton,
                          G_HelpButton,
                          G_Spacing,
                          G_AddButton,
                          G_EditButton,
                          G_RemoveButton;

        APIRET cmnLoadDialogStrings(PCDLGHITEM paDlgItems,
                                    ULONG cDlgItems,
                                    PDLGHITEM *ppaNew);
    #endif

    /* ******************************************************************
     *
     *   Pointers
     *
     ********************************************************************/

    PCSZ cmnQueryThemeDirectory(VOID);

    #define STDICON_SHADOWBROKEN         1      // V1.0.2 (2003-02-07) [umoeller]
    #define STDICON_SHADOWOVERLAY        2      // V0.9.20 (2002-07-31) [umoeller]
    #define STDICON_TEMPLATE             3      // V0.9.20 (2002-08-04) [umoeller]
    #define STDICON_PM                   4
    #define STDICON_WIN16                5
    #define STDICON_WIN32                6
    #define STDICON_OS2WIN               7
    #define STDICON_OS2FULLSCREEN        8
    #define STDICON_DOSWIN               9
    #define STDICON_DOSFULLSCREEN       10
    #define STDICON_DLL                 11
    #define STDICON_DRIVER              12
    #define STDICON_PROG_UNKNOWN        13
    #define STDICON_DATAFILE            14
    #define STDICON_TRASH_EMPTY         15
    #define STDICON_TRASH_FULL          16
    #define STDICON_DESKTOP_CLOSED      17
    #define STDICON_DESKTOP_OPEN        18
    #define STDICON_FOLDER_CLOSED       19
    #define STDICON_FOLDER_OPEN         20

    // tool bar icons V1.0.1 (2002-11-30) [umoeller]
    #define STDICON_TB_REFRESH          21
    #define STDICON_TB_FIND             22
    #define STDICON_TB_HELP             23
    #define STDICON_TB_MULTIPLECOLUMNS  24
    #define STDICON_TB_DETAILS          25
    #define STDICON_TB_SMALLICONS       26

    APIRET cmnGetStandardIcon(ULONG ulStdIcon,
                              HPOINTER *phptr,
                              PULONG pcbIconInfo,
                              PICONINFO pIconInfo);

    BOOL cmnIsStandardIcon(HPOINTER hptrIcon);

    /* ******************************************************************
     *
     *   Object setup sets V0.9.9 (2001-01-29) [umoeller]
     *
     ********************************************************************/

    // settings types for XWPSETUPENTRY.ulType
    #define STG_LONG_DEC    1
    #define STG_BOOL        2
    #define STG_BITFLAG     3
    #define STG_PSZ         4       // V0.9.9 (2001-03-07) [umoeller]
    #define STG_PSZARRAY    5
    #define STG_BINARY      6       // V0.9.12 (2001-05-24) [umoeller]
    #define STG_LONG_RGB    7       // V0.9.16 (2002-01-26) [umoeller]

    /*
     *@@ XWPSETUPENTRY:
     *      describes an entry in an object's setup set.
     *
     *      A "setup set" is an array of XWPSETUPENTRY
     *      structures, each of which represents an object
     *      instance variable together with its setup
     *      string, variable type, default value, and
     *      value limits.
     *
     *      A setup set can be quickly
     *
     *      -- initialized to the default values in
     *         wpInitData (see cmnSetupInitData);
     *
     *      -- built a setup string from during xwpQuerySetup
     *         (see cmnSetupBuildString);
     *
     *      -- updated from a setup string during wpSetup
     *         (see cmnSetupScanString);
     *
     *      -- stored during wpSaveState (see cmnSetupSave);
     *
     *      -- restored during wpRestoreState (see cmnSetupRestore).
     *
     *      Setup sets have been introduced because when new
     *      instance variables are added to a WPS class, one
     *      always has to go through the same dull procedure
     *      of adding that instance variable to all these
     *      methods. So there is always the danger that a
     *      variable is not safely initialized, saved, or
     *      restored, or that default values get messed up
     *      somewhere. The cmnSetup* functions are intended
     *      to aid in getting that synchronized.
     *
     *      To use these, set up a "setup set" (an array of
     *      XWPSETUPENTRY structs) for your class as a global
     *      variable with your class implementation and use the
     *      cmnSetup* function calls in your method overrides.
     *
     *      In order to support any type of variable, ulOfsOfData
     *      does not specify the absolute address of the variable,
     *      but the offset in bytes within a structure which is
     *      then passed with the somThis pointer to the cmnSetup*
     *      functions. While this _can_ be a "true" somThis pointer
     *      from a SOM object, it can really be any structure.
     *
     *@@added V0.9.9 (2001-01-29) [umoeller]
     *@@changed V0.9.16 (2002-01-26) [umoeller]: added STG_LONG_RGB
     */

    typedef struct _XWPSETUPENTRY
    {
        ULONG       ulType;
                        // describes the type of the variable specified
                        // by ulOfsOfData. One of:

                        // -- STG_LONG_DEC: LONG value; in that case,
                        //      ulMin and ulMax apply and the setup
                        //      string gets the long value appended
                        //      in decimal format.

                        // -- STG_LONG_RGB: LONG value representing
                        //      an RGB color; in that case, the
                        //      corresponding setup string is in
                        //      "red green blue" format.

                        // -- STG_BOOL: BOOL value; in that case,
                        //      the setup string gets either YES or NO.

                        // -- STG_BITFLAG: a bitflag value; in that case,
                        //      the data is assumed to be a ULONG and
                        //      ulBitflag applies; the setup string
                        //      gets either YES or NO also for each entry.
                        //      NOTE: For bitfields, always set them up
                        //      as follows:
                        //      1) a STG_LONG entry for the entire bit
                        //         field with the default value and a
                        //         save/restore key, but no setup string;
                        //      2) for each bit flag, a STG_BITFLAG
                        //         entry afterwards with each flag's
                        //         default value and the setup string,
                        //         but NO save/restore key.
                        //      This ensures that on save/restore, the
                        //      bit field is flushed once only and that
                        //      the bit field is initialized on cmnInitData,
                        //      but each flag can be set/cleared individually
                        //      with a setup string.

                        // -- STG_PSZ: a null-terminated string. Note
                        //      that this requires memory management.
                        //      In that case, ulOfsOfData must point to
                        //      a PSZ pointer, and lDefault must also
                        //      be a PSZ to the default value.

                        // -- STG_PSZARRAY: an array of null-terminated
                        //      strings, where the last string is terminated
                        //      with two zero bytes. Note that this requires
                        //      memory management.
                        //      In that case, ulOfsOfData must point to
                        //      a PSZ pointer. The default value will be
                        //      a NULL string.

                        // -- STG_BINARY: a binary structure. There is
                        //      no setup string support, of course, and
                        //      lMax must specify the size of the
                        //      structure.

        // build/scan setup string values:

        const char  *pcszSetupString;
                        // setup string keyword; e.g. "CCVIEW";
                        // if this is NULL, no setup string is supported
                        // for this entry, and it is not scanned/built.

        // data description:

        ULONG       ulOfsOfData;
                        // offset of the data in an object's instance
                        // data (this is added to the somThis pointer);
                        // the size of the data depends on the setting
                        // type (usually a ULONG).
                        // You can use the FIELDOFFSET macro to determine
                        // this value, e.g. FIELDOFFSET(somThis, ulVariable).

        // save/restore values:
        ULONG       ulKey;
                        // key to be used with wpSaveState/wpRestoreState;
                        // if 0, value is not saved/restored.
                        // NOTE: For STG_BITFLAG, set this to 0 always.
                        // Define a preceding STG_LONG for the bitflag
                        // instead.

        // defaults/limits:
        LONG        lDefault;   // default value; a setup string is only
                                // built if the value is different from
                                // this. This is also used for cmnSetupInitData.

        ULONG       ulExtra;
                        // -- with STG_BITFLAG, the mask for the
                        // ULONG data pointed to by ulOfsOfData
                        // -- with STG_BINARY, the size of the structure

        LONG        lMin,       // only with STG_LONG, the min and max
                    lMax;       // values allowed

    } XWPSETUPENTRY, *PXWPSETUPENTRY;

    VOID XWPENTRY cmnSetupInitData(const XWPSETUPENTRY *paSettings,
                                   ULONG cSettings,
                                   PVOID somThis);

    #ifdef XSTRING_HEADER_INCLUDED
        VOID XWPENTRY cmnSetupBuildString(const XWPSETUPENTRY *paSettings,
                                          ULONG cSettings,
                                          PVOID somThis,
                                          PXSTRING pstr);
    #endif

    #ifdef SOM_WPObject_h
        BOOL XWPENTRY cmnSetupScanString(WPObject *somSelf,
                                         const XWPSETUPENTRY *paSettings,
                                         ULONG cSettings,
                                         PVOID somThis,
                                         PSZ pszSetupString,
                                         PULONG pcSuccess);

        BOOL XWPENTRY cmnSetupSave(WPObject *somSelf,
                                   const XWPSETUPENTRY *paSettings,
                                   ULONG cSettings,
                                   PCSZ pcszClassName,
                                   PVOID somThis);

        BOOL XWPENTRY cmnSetupRestore(WPObject *somSelf,
                                      const XWPSETUPENTRY *paSettings,
                                      ULONG cSettings,
                                      PCSZ pcszClassName,
                                      PVOID somThis);
    #endif

    ULONG XWPENTRY cmnSetupSetDefaults(const XWPSETUPENTRY *paSettings,
                                       ULONG cSettings,
                                       PULONG paulOffsets,
                                       ULONG cOffsets,
                                       PVOID somThis);

    ULONG XWPENTRY cmnSetupRestoreBackup(PULONG paulOffsets,
                                         ULONG cOffsets,
                                         PVOID somThis,
                                         PVOID pBackup);

    /* ******************************************************************
     *
     *   Object locks
     *
     ********************************************************************/

    #ifdef SOM_WPObject_h
        WPObject* cmnLockObject(WPObject *somSelf);
    #endif

    /* ******************************************************************
     *
     *   Trash can setup
     *
     ********************************************************************/

    BOOL XWPENTRY cmnTrashCanReady(VOID);

    BOOL XWPENTRY cmnEnableTrashCan(HWND hwndOwner,
                                    BOOL fEnable);

    #ifdef SOM_WPObject_h
        BOOL XWPENTRY cmnDeleteIntoDefTrashCan(WPObject *pObject);
    #endif

    BOOL XWPENTRY cmnEmptyDefTrashCan(HAB hab,
                                      PULONG pulDeleted,
                                      HWND hwndConfirmOwner);

    /********************************************************************
     *
     *   Product info
     *
     ********************************************************************/

    #ifndef __XWPLITE__
    #ifdef SOM_WPFolder_h
        BOOL XWPENTRY cmnAddProductInfoMenuItem(WPFolder *somSelf, HWND hwndMenu);
    #endif
    #endif

    VOID XWPENTRY cmnShowProductInfo(HWND hwndOwner, ULONG ulSound);

    /********************************************************************
     *
     *   Miscellaneae
     *
     ********************************************************************/

    #ifdef NLS_HEADER_INCLUDED
        PCOUNTRYSETTINGS2 XWPENTRY cmnQueryCountrySettings(BOOL fReload);
    #endif

    CHAR XWPENTRY cmnQueryThousandsSeparator(VOID);

    BOOL XWPENTRY cmnIsValidHotkey(USHORT usFlags,
                                   USHORT usKeyCode);

    BOOL cmnDescribeKey(PSZ pszBuf,
                        USHORT usFlags,
                        USHORT usKeyCode,
                        PCSZ pszDescription,
                        ULONG cbBuf);

    VOID cmnInsertSeparator(HWND hwndMenu,
                            SHORT sPosition);

    VOID XWPENTRY cmnAddCloseMenuItem(HWND hwndMenu);

    #ifdef SOM_WPObject_h
        ULONG cmnQueryFCF(WPObject *somSelf);

        BOOL XWPENTRY cmnRegisterView(WPObject *somSelf,
                                      PUSEITEM pUseItem,
                                      ULONG ulViewID,
                                      HWND hwndFrame,
                                      PCSZ pcszViewTitle);
    #endif

#ifndef __NOXSYSTEMSOUNDS__
    BOOL XWPENTRY cmnPlaySystemSound(USHORT usIndex);
#endif

    #ifdef SOM_WPObject_h
        BOOL cmnIsADesktop(WPObject *somSelf);

        WPObject* XWPENTRY cmnQueryActiveDesktop(VOID);

        BOOL cmnIsObjectFromForeignDesktop(WPObject *somSelf);
    #endif

    HWND XWPENTRY cmnQueryActiveDesktopHWND(VOID);
    typedef HWND XWPENTRY CMNQUERYACTIVEDESKTOPHWND(VOID);
    typedef CMNQUERYACTIVEDESKTOPHWND *PCMNQUERYACTIVEDESKTOPHWND;

    #ifdef SOM_WPObject_h
        WPObject* cmnQueryObjectFromID(PCSZ pcszObjectID);
    #endif

    /* ******************************************************************
     *
     *   "Run" dialog
     *
     ********************************************************************/

    #define RUN_MAXITEMS 20

    HAPP XWPENTRY cmnRunCommandLine(HWND hwndOwner,
                                    PCSZ pcszStartupDir);

    PCSZ XWPENTRY cmnQueryDefaultFont(VOID);
    typedef PCSZ XWPENTRY CMNQUERYDEFAULTFONT(VOID);
    typedef CMNQUERYDEFAULTFONT *PCMNQUERYDEFAULTFONT;

    VOID XWPENTRY cmnSetControlsFont(HWND hwnd, SHORT usIDMin, SHORT usIDMax);
    typedef VOID XWPENTRY CMNSETCONTROLSFONT(HWND hwnd, SHORT usIDMin, SHORT usIDMax);
    typedef CMNSETCONTROLSFONT *PCMNSETCONTROLSFONT;

    HPOINTER XWPENTRY cmnQueryDlgIcon(VOID);

    ULONG XWPENTRY cmnMessageBox(HWND hwndOwner,
                                 PCSZ pcszTitle,
                                 PCSZ pcszMessage,
                                 ULONG ulHelpPanel,
                                 ULONG flStyle);
    typedef ULONG XWPENTRY CMNMESSAGEBOX(HWND hwndOwner,
                                         PCSZ pcszTitle,
                                         PCSZ pcszMessage,
                                         ULONG ulHelpPanel,
                                         ULONG flStyle);
    typedef CMNMESSAGEBOX *PCMNMESSAGEBOX;

    #ifdef XSTRING_HEADER_INCLUDED
    APIRET XWPENTRY cmnGetMessageExt(PCSZ *pTable,
                                     ULONG ulTable,
                                     PXSTRING pstr,
                                     PCSZ pcszMsgID);

    APIRET XWPENTRY cmnGetMessage(PCSZ *pTable,
                                  ULONG ulTable,
                                  PXSTRING pstr,
                                  ULONG ulMsgNumber);
    #else
        #define cmnGetMessage #error xstring.h not included
        #define cmnGetMessageExt #error xstring.h not included
    #endif

    ULONG cmnMessageBoxHelp(HWND hwndOwner,
                            ULONG ulTitle,
                            PCSZ *pTable,
                            ULONG ulTable,
                            ULONG ulMessage,
                            ULONG ulHelpPanel,
                            ULONG flStyle);

    ULONG XWPENTRY cmnMessageBoxExt(HWND hwndOwner,
                                    ULONG ulTitle,
                                    PCSZ *pTable,
                                    ULONG ulTable,
                                    ULONG ulMessage,
                                    ULONG flStyle);

    ULONG XWPENTRY cmnErrorMsgBox(HWND hwndOwner,
                                  APIRET arc,
                                  ULONG ulMsg,
                                  ULONG flFlags,
                                  BOOL fShowExplanation);

    ULONG XWPENTRY cmnDosErrorMsgBox(HWND hwndOwner,
                                     PSZ pszReplString,
                                     PCSZ pcszTitle,
                                     PCSZ pcszPrefix,
                                     APIRET arc,
                                     PCSZ pcszSuffix,
                                     ULONG ulFlags,
                                     BOOL fShowExplanation);

    #ifdef XSTRING_HEADER_INCLUDED
        VOID cmnDescribeError(PXSTRING pstr,
                              APIRET arc,
                              PSZ pszReplString,
                              BOOL fShowExplanation);
    #endif

    #ifdef SOM_WPObject_h
        ULONG cmnProgramErrorMsgBox(HWND hwndOwner,
                                    WPObject *pProgram,
                                    PSZ pszFailingName,
                                    APIRET arc);
    #endif

    PSZ XWPENTRY cmnTextEntryBox(HWND hwndOwner,
                                 PCSZ pcszTitle,
                                 PCSZ pcszDescription,
                                 PCSZ pcszDefault,
                                 ULONG ulMaxLen,
                                 ULONG fl);

    VOID XWPENTRY cmnSetDlgHelpPanel(ULONG ulHelpPanel);

    MRESULT EXPENTRY cmn_fnwpDlgWithHelp(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

    BOOL XWPENTRY cmnFileDlg2(HWND hwndOwner,
                              PSZ pszFile,
                              ULONG flFlags,
                              HINI hini,
                              PCSZ pcszApplication,
                              PCSZ pcszKey,
                              BOOL fUseNewFileDlg);

    BOOL XWPENTRY cmnFileDlg(HWND hwndOwner,
                             PSZ pszFile,
                             ULONG flFlags,
                             HINI hini,
                             PCSZ pcszApplication,
                             PCSZ pcszKey);

    /* ******************************************************************
     *
     *   WPS functions imported from PMWP.DLL
     *
     ********************************************************************/

    // The following functions are imported into XFLDR.DLL
    // via our DEF file (see src\shared\xwp.def)

    /*
     *@@ ShlAllocMem:
     *      allocates a chunk from the WPS heap, apparently.
     *      This is required for some of our stuff to work
     *      since the WPS might be trying to be smart and
     *      free it later.
     *
     *      The PMWP.SYM files and http://home.clara.net/orac/os2/pmwp.htm
     *      call this "AllocMem" only, but I found that a
     *      bit confusing, so I added the Shl* prefix.
     *
     *@@added V1.0.1 (2003-01-25) [umoeller]
     */

    PVOID EXPENTRY ShlAllocMem(ULONG cb, ULONG *pulError);

    /*
     *@@ ShlFreeMem:
     *      the reverse to ShlAllocMem.
     *
     *@@added V1.0.1 (2003-01-25) [umoeller]
     */

    BOOL EXPENTRY ShlFreeMem(PVOID pv);

    /* ******************************************************************
     *
     *   WPS debugging
     *
     ********************************************************************/

#ifdef __DEBUG__
    // window proc wrapper with an excpt handler so we can figure out
    // why we get traps in WPS folder window procs V1.0.1 (2002-12-19) [umoeller]
    MRESULT XWPENTRY cmnDebugWindowProc(PCSZ pcszSourceFile,
                                        ULONG ulLine,
                                        PCSZ pcszFunction,
                                        PFNWP pfnwp,
                                        HWND hwnd,
                                        ULONG msg,
                                        MPARAM mp1,
                                        MPARAM mp2);
    #define CMN_CALLWINPROC(pfnwp, hwnd, msg, mp1, mp2) cmnDebugWindowProc(__FILE__, __LINE__, __FUNCTION__, (pfnwp), (hwnd), (msg), (mp1), (mp2))
#else
    #define CMN_CALLWINPROC(pfnwp, hwnd, msg, mp1, mp2) pfnwp((hwnd), (msg), (mp1), (mp2))
#endif

    PCSZ cmnIdentifyView(ULONG ulView);

    #ifdef __DEBUG__
        PCSZ cmnIdentifyRestoreID(PSZ pszClass,
                                  ULONG ulKey);

        #ifdef SOM_WPObject_h
            VOID cmnDumpTaskRec(WPObject *somSelf,
                                PCSZ pcszMethodName,
                                PTASKREC pTaskRec);
        #endif
    #else
        #define cmnIdentifyRestoreID(psz, ul) ""
        #define cmnDumpTaskRec(somSelf, pszMethodName, pTaskRec)
    #endif

#endif

