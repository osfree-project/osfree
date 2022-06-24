
/*
 *@@sourcefile helppanels.h:
 *      shared include file for XWorkplace help panel IDs.
 *
 *      NOTE: If you look at this file from the toolkit\shared
 *      directory of a binary XWorkplace distribution, this
 *      file is called helpids.h to conform to FAT 8+3
 *      naming conventions.
 *
 *      This is now also used by h2i.exe to finally get
 *      rid of the stupid hand calculations for getting
 *      the help panels right. However, this must be
 *      in a separate file because h2i isn't good at
 *      fully parsing C syntax. So the help panel IDs
 *      have been extracted from common.h into this file.
 *
 *      To reference (show) a help panel from the XWorkplace
 *      sources, do the following:
 *
 *      --  For each new help panel, add a unique resid
 *          definition below. The only requirement is that
 *          this be < 10000 because h2i.exe will assign
 *          automatic resids with values 10000 and higher.
 *
 *      --  Write a new HTML file in 001\xwphelp.
 *
 *      --  Add a link to the new help file from
 *          001\xwphelp\xfldr001.html. Otherwise your file
 *          won't be translated by h2i.exe.
 *
 *      --  To the top <HTML> tag in your new help file,
 *          add a "RESID=&resid;" attribute to explicitly
 *          assign the resid for that file (with resid
 *          being the identifier used in the #define below).
 *
 *          Example:
 *
 +              <HTML RESID=&ID_XFH_BORED;>
 *
 *      --  In the C sources, call cmnDisplayHelp(ID_XFH_BORED)
 *          to display the help panel (or specify ID_XFH_BORED
 *          in the callbacks to ntbInsertPage).
 *
 *      For historical reasons, the resids below are defined
 *      in ascending order. Since some external scripts rely
 *      on certain resids, do not change the existing ones.
 *      However, there is no requirement for new resids to
 *      be in ascending order any more, as long as they are
 *      unique.
 */

/*
 *      Copyright (C) 1997-2003 Ulrich M”ller.
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

#ifndef HELPPANELS_HEADER_INCLUDED
    #define HELPPANELS_HEADER_INCLUDED

    /********************************************************************
     *
     *   Help panels in XFDLRxxx.HLP
     *
     ********************************************************************/

    // The following are constant (I hope) help panel IDs
    // for the various XWorkplace menu items, settings pages,
    // and dialog items therein.

    // All help panel IDs have been changed with V0.9.3 (2000-05-04) [umoeller]
    // because I've rearranged the help HTML files.

    // help panel IDs for various dlg boxes
    #define ID_XFH_BORED            2
    // #define ID_XFH_NOCONFIG         3        // removed
    // #define ID_XFH_NOOBJECT         4        // removed V0.9.16 (2001-10-11) [umoeller]
    #define ID_XFH_LIMITREACHED     5
    #define ID_XFH_NOTEXTCLIP       6
    #define ID_XFH_REBOOTEXT        7
    #define ID_XFH_AUTOCLOSEDETAILS 8
    #define ID_XFH_SELECTCLASS      9
    #define ID_XFH_REGISTERCLASS    10
    #define ID_XFH_TITLECLASH       11

    #define ID_XMH_VARIABLE         12
    #define ID_XMH_CONFIGFOLDER     13
    #define ID_XMH_REFRESH          14
    #define ID_XMH_SNAPTOGRID       15
    #define ID_XMH_COPYFILENAME     16
    #define ID_XMH_XSHUTDOWN        17
    #define ID_XMH_RESTARTWPS       18

    #define ID_XSH_SETTINGS1                 19
    // #define ID_XSH_SETTINGS_REMOVEMENUS      20      removed V0.9.19 (2002-04-24) [umoeller]
    // #define ID_XSH_SETTINGS_ADDMENUS         21      removed V0.9.19 (2002-04-24) [umoeller]
    #define ID_XSH_SETTINGS_STATUSBARS1      22
    #define ID_XSH_SETTINGS_SNAPTOGRID       23
    #define ID_XSH_SETTINGS_FOLDERHOTKEYS    24
    #define ID_XSH_SETTINGS_PARANOIA         25
    #define ID_XSH_SETTINGS_DTP_MENUITEMS    26
    #define ID_XSH_SETTINGS_DTP_SHUTDOWN     27     // XShutdown
    #define ID_XSH_SETTINGS_FLDRSORT         28
    #define ID_XSH_SETTINGS_FLDR1            29
    #define ID_XSH_SETTINGS_SB2              30
    #define ID_XSH_SETTINGS_WPSCLASSES       31
    #define ID_XSH_SETTINGS_OBJINTERNALS     32
    #define ID_XSH_SETTINGS_KERNEL1          33
    #define ID_XSH_SETTINGS_KERNEL2          34
    #define ID_XMH_STARTUPSHUTDOWN           35
    #define ID_XMH_FOLDERCONTENT             36
    #define ID_XSH_SETTINGS_HPFS             37
    #define ID_XSH_SETTINGS_FAT              37     // same page as HPFS
    #define ID_XSH_SETTINGS_CFGM             38     // "Config folder menu items"
    // #define ID_XSH_SETTINGS_FILEOPS          39     // "File operations" no longer used
    #define ID_XSH_SETTINGS_WPS              40
    #define ID_XSH_SETTINGS_ERRORS           41
    #define ID_XSH_SETTINGS_SYSPATHS         42     // V0.9.0: "System paths"
    #define ID_XSH_SETTINGS_FILETYPES        43     // V0.9.0: "File Types"
    #define ID_XSH_SETTINGS_FILET_MERGE      44
    #define ID_XSH_SETTINGS_SOUND            45
    #define ID_XSH_SETTINGS_TRASHCAN         46     // V0.9.0: XWPTrashCan (two pages)
    #define ID_XSH_SETTINGS_TRASHCAN2        47     // V0.9.0: XWPTrashCan (two pages)
    #define ID_XSH_SETTINGS_XC_INFO          48     // V0.9.0: XWPSetup "Status" page
    #define ID_XSH_SETTINGS_XC_FEATURES      49     // V0.9.0: XWPSetup "Features" page
    #define ID_XSH_SETTINGS_FILEPAGE1        50     // V0.9.0: file-system objects "File" page replacment
    #define ID_XSH_SETTINGS_FILEPAGE2        51     // V0.9.1 (2000-01-22) [umoeller]
    #define ID_XSH_SETTINGS_DISKDETAILS      52     // V0.9.0: disk "Details" replacement
    #define ID_XSH_SETTINGS_DTP_STARTUP      53     // V0.9.0: disk "Details" replacement
    #define ID_XSH_SETTINGS_DTP_ARCHIVES     54     // V0.9.0: disk "Details" replacement
    #define ID_XSH_SETTINGS_XFLDSTARTUP      55     // V0.9.0: startup folder settings page
    #define ID_XSH_SETTINGS_PGMFILE_MODULE   56     // V0.9.0: progfile "Module" page
    #define ID_XSH_SETTINGS_CLASSLIST        57     // V0.9.0: XWPClassList settings page
    #define ID_XSH_XFLDWPS                   58     // V0.9.0: default help for XFldWPS
    #define ID_XSH_XFLDSYSTEM                59     // V0.9.0: default help for XFldSystem
    #define ID_XSH_XWPSETUP                  60     // V0.9.0: default help for XWPSetup
    #define ID_XSH_SETTINGS_OBJECTS          61     // V0.9.0: XWPSetup "Objects" page
    #define ID_XSH_SETTINGS_DRIVERS          62     // V0.9.0: XFldSystem "Drivers" page
    #define ID_XSH_DRIVER_HPFS               63     // V0.9.0: XFldSystem "Drivers" page
    #define ID_XSH_DRIVER_CDFS               64     // V0.9.0: XFldSystem "Drivers" page
    #define ID_XSH_DRIVER_S506               65     // V0.9.0: XFldSystem "Drivers" page
    #define ID_XSH_KEYB_OBJHOTKEYS           66     // V0.9.0: XWPKeyboard "Object hotkeys" page
    #define ID_XSH_MOUSE_MOVEMENT            67     // V0.9.0: XWPMouse "Movement" page 1
    #define ID_XSH_MOUSEMAPPINGS2            68     // V0.9.1: XWPMouse "Mappings" page 2
    #define ID_XSH_XWPSCREEN                 69     // V0.9.2: default help for XWPScreen
    #define ID_XSH_MOUSE_CORNERS             70     // V0.9.2: XWPMouse "Movement" page 2
    #define ID_XSH_SETTINGS_TRASH_DRIVES     71     // V0.9.2: XWPTrashCan "Drives" page
    #define ID_XSH_SETTINGS_SYSLEVEL         72     // V0.9.3: XFldSystem "Syslevel" page
    #define ID_XSH_SETTINGS_PAGER_GENERAL 73     // V0.9.3: XWPScreen "XPager General" page
    #define ID_XSH_SETTINGS_PAGER_COLORS  74     // V0.9.3: XWPScreen "XPager Colors" page
    #define ID_XSH_SETTINGS_PAGER_MOUSE   75
    #define ID_XSH_SETTINGS_FUNCTIONKEYS     76     // V0.9.3: XWPKeyboard "Function keys" page
    #define ID_XSH_SETTINGS_FUNCTIONKEYSDLG  77
    #define ID_XSH_SETTINGS_XWPSTRING_MAIN   78     // V0.9.3: XWPString main help
    #define ID_XSH_SETTINGS_XWPSTRING_PAGE   79     // V0.9.3: XWPString settings page
    #define ID_XSH_MEDIA_DEVICES             80     // V0.9.3: XWPMedia "Devices" page
    #define ID_XSH_MEDIA_CODECS              81     // V0.9.3: XWPMedia "Codecs" page
    #define ID_XSH_MEDIA_IOPROCS             82     // V0.9.3: XWPMedia "IOProcs" page
    #define ID_XSH_SETTINGS_TRASHCAN_DRIVES  83     // V0.9.4: XWPTrashCan "Drives" page
    #define ID_XSH_XWP_INSTALL_FOLDER        84     // installation folder default help;
                                                    // set by install script, do not change
    #define ID_XSH_SETTINGS_PAGER_STICKY  85     // V0.9.3: XWPScreen "XPager Sticky" page;
    #define ID_XSH_SETTINGS_PAGER_STICKY2 86     // V0.9.3: XWPScreen "XPager Sticky" page;
    #define ID_XSH_SETTINGS_PAGER_STICKY3 87     // V0.9.3: XWPScreen "XPager Sticky" page;
    #define ID_XSH_SETTINGS_TRASHCAN_ICON    88     // V0.9.4: XWPTrashCan "Icon" page
    #define ID_XSH_XSHUTDOWN_CONFIRM         89     // V0.9.4: shutdown confirm dlg
    #define ID_XFH_SELECTSOME                90     // V0.9.4: changed this to have it assigned a fixed no. finally
    #define ID_XFH_VIEW_MENU_ITEMS           91     // V0.9.4: added for XFolder "View" submenu items
    #define ID_XSH_DRIVER_HPFS386            92     // V0.9.5: HPFS386 driver dialog help

    #define ID_XSH_XWPMEDIA                  93     // V0.9.5: XWPMedia main panel

    #define ID_XSH_XWP_CLASSESDLG            94     // V0.9.5: XWP "Classes" dlg

    #define ID_XFH_CLOSEVIO                  95     // V0.9.6: was missing

    #define ID_XSH_SETTINGS_PGMFILE_RESOURCES 96    // V0.9.7: progfile "Resources" page

    #define ID_XSH_XCENTER_MAIN              97     // V0.9.7: XCenter default help
    #define ID_XSH_XCENTER_VIEW1             98     // V0.9.7: XCenter "View" page
    #define ID_XSH_XCENTER_VIEW2             99     // V0.9.7: XCenter "View" page
    #define ID_XSH_XCENTER_WIDGETS          100     // V0.9.7: XCenter "Widgets" page
    #define ID_XSH_XCENTER_CLASSES          101     // V0.9.9: XCenter "Classes" page

    #define ID_XSH_WIDGET_CLOCK_MAIN        102     // V0.9.7: Winlist widget main help
    #define ID_XSH_WIDGET_MEMORY_MAIN       103     // V0.9.7: Memory widget main help
    #define ID_XSH_WIDGET_OBJBUTTON_MAIN    104     // V0.9.7: Object button widget main help
    #define ID_XSH_WIDGET_PULSE_MAIN        105     // V0.9.7: Pulse widget main help
    #define ID_XSH_WIDGET_SWAP_MAIN         106     // V0.9.7: Swapper widget main help
    #define ID_XSH_WIDGET_WINLIST_MAIN      107     // V0.9.7: Winlist widget main help
    #define ID_XSH_WIDGET_WINLIST_SETTINGS  108     // V0.9.7: Winlist widget properties
    #define ID_XSH_WIDGET_XBUTTON_MAIN      109     // V0.9.7: X-Button widget main help
    #define ID_XSH_WIDGET_SENTINEL_MAIN     110     // V0.9.9: Sentinal widget main help
    #define ID_XSH_WIDGET_HEALTH_MAIN       111     // V0.9.9: Health widget main help
    #define ID_XSH_WIDGET_HEALTH_SETTINGS   112     // V0.9.9: Health widget main help

    #define ID_XSH_FONTFOLDER               113
    #define ID_XSH_FONTFOLDER_TEXT          114     // V0.9.9 (2001-03-27) [umoeller]
    #define ID_XSH_FONTFILE                 115
    #define ID_XSH_FONTOBJECT               116
    #define ID_XSH_FONTSAMPLEVIEW           117
    #define ID_XSH_FONTSAMPLEHINTS          118

    #define ID_XSH_XFIX_INTRO               119

    #define ID_XSH_RUN                      120

    #define ID_XSH_SETTINGS_PGM_ASSOCIATIONS 121
    #define ID_XSH_SETTINGS_XC_THREADS      122
    #define ID_XSH_SETTINGS_PGMFILE_MODULE1 123
    #define ID_XSH_SETTINGS_PGMFILE_MODULE2 124

    #define ID_XSH_SETTINGS_PAGER_WINDOW 125     // V0.9.9: XWPScreen "XPager" page V0.9.9 (2001-03-15) [lafaix]

    #define ID_XSH_DATAFILE_TYPES           126     // V0.9.9: data file types page

    #define ID_XSH_PAGER_INTRO              128     // V0.9.11: XWPAdmin "Users" page

    #define ID_XSH_SORTPAGE                 129     // V0.9.12: sort page (instance or global)

    #define ID_XSH_WIDGET_POWER_MAIN        130     // V0.9.12 (2001-05-26) [umoeller]

    #define ID_XSH_WIDGET_TRAY              131     // V0.9.13 (2001-06-21) [umoeller]

    #define ID_XSH_DRIVER_JFS               132     // V0.9.13 (2001-06-27) [umoeller]

    #define ID_XSH_WIDGET_DISKFREE_WC       133     // V0.9.14 (2001-08-01) [umoeller]
    #define ID_XSH_WIDGET_DISKFREE_COND     134     // V0.9.14 (2001-08-01) [umoeller]
    #define ID_XSH_MOUSE_MOVEMENT2          135     // V0.9.14 (2001-08-02) [lafaix]

    #define ID_XSH_NETSCAPEDDE              136     // V0.9.16 (2001-10-02) [umoeller]
    #define ID_XSH_TREESIZE                 137

    #define ID_XSH_OBJICONPAGE1             138     // V0.9.16 (2001-10-15) [umoeller]
    #define ID_XSH_OBJICONPAGE2             139     // V0.9.16 (2001-10-15) [umoeller]

    #define ID_XSH_XWPNETWORK_MAIN          140     // V0.9.16 (2001-10-19) [umoeller]
    #define ID_XSH_XWPNETSERVER_MAIN        141     // V0.9.16 (2001-10-19) [umoeller]

    #define ID_XSH_DATAFILE_MAIN            142     // V0.9.16 (2002-01-13) [umoeller]
    #define ID_XSH_PROGRAMFILE_MAIN         143     // V0.9.16 (2002-01-13) [umoeller]
    #define ID_XSH_PROGRAMFILE_DLL          144     // V0.9.16 (2002-01-13) [umoeller]
    #define ID_XSH_PROGRAMFILE_DRIVER       145     // V0.9.16 (2002-01-13) [umoeller]

    #define ID_XSH_VCARD_MAIN               146     // V0.9.16 (2002-02-02) [umoeller]
    #define ID_XSH_VCARD_PAGE               147     // V0.9.16 (2002-02-02) [umoeller]

    #define ID_XSH_FOPS_DELETE              148     // V0.9.19 (2002-04-02) [umoeller]

    #define ID_XSH_ADMIN_LOCAL_USER         149     // V0.9.19 (2002-04-02) [umoeller]
    #define ID_XSH_ADMIN_ALL_USERS          150     // V0.9.19 (2002-04-02) [umoeller]
    #define ID_XSH_ADMIN_ALL_GROUPS         151     // V0.9.19 (2002-04-02) [umoeller]

    #define ID_XSH_WIDGET_XBUTTON_SETTINGS  152     // V0.9.19 (2002-04-14) [umoeller]

    #define ID_XSH_RESTARTWPS_CONFIRM       153     // V0.9.19 (2002-04-17) [umoeller]

    #define ID_XSH_FOLDER_MAIN              154     // V0.9.19 (2002-04-17) [umoeller]
    #define ID_XSH_FOLDER_VIEWS             155     // V0.9.19 (2002-04-17) [umoeller]

    #define ID_XSH_SETTINGS_MENUITEMS       156     // V0.9.19 (2002-04-17) [umoeller]
    #define ID_XSH_SETTINGS_MENUSETTINGS    157     // V0.9.19 (2002-04-17) [umoeller]

    #define ID_XSH_CANNOT_FIND_TRASHCAN     158     // V0.9.19 (2002-04-17) [umoeller]
    #define ID_XSH_TRASH_NODRIVESUPPORT     159     // V0.9.19 (2002-04-17) [umoeller]

    #define ID_XSH_WIDGET_IPMON_MAIN        160     // V0.9.19 (2002-06-05) [umoeller]
    #define ID_XSH_OBJICONPAGE1_X           161     // V0.9.19 (2002-06-15) [umoeller]

    #define ID_XFH_BATCHRENAME              162     // V0.9.19 (2002-06-18) [umoeller]

    #define ID_XSH_SETTINGS_MACROS          163     // V0.9.20 (2002-07-03) [umoeller]

    // all the following added with V0.9.20 (2002-07-12) [umoeller]
    #define ID_XSH_OS2SYSSETUPMENUITEM      164
    #define ID_XSH_FDR_OPENPARENT           165
    #define ID_XSH_CRANOTHERMENUITEM        166
    #define ID_XSH_COPYMENUITEM             167
    #define ID_XSH_MOVEMENUITEM             168
    #define ID_XSH_CRSHADOWMENUITEM         169
    #define ID_XMH_CHECKDISK                170
    #define ID_XMH_FORMATDISK               171
    #define ID_XMH_MANAGEVOLUMES            172
    #define ID_XSH_PROGRAMOBJ_MAIN          173
    #define ID_XSH_PROGRAMOBJ_TEMPLATE      174
    #define ID_XSH_STARTUPFOLDER            175
    #define ID_XSH_FOLDER_TEMPLATE          176
    #define ID_XSH_OS2TEMPLATES             177
    #define ID_XSH_OS2MINWINV               178
    #define ID_XSH_PROGRAMFILE_BATCH        179
    #define ID_XSH_PROGRAMFILE_AUTOEXEC     180
    #define ID_XSH_WPBITMAP                 181
    #define ID_XSH_WPBITMAP_TEMPLATE        182
    #define ID_XSH_WPICON                   183
    #define ID_XSH_WPICON_TEMPLATE          184
    #define ID_XSH_WPPOINTER                185
    #define ID_XSH_WPPOINTER_TEMPLATE       186
    #define ID_XSH_WPPIF                    187
    #define ID_XSH_WPPIF_TEMPLATE           188
    #define ID_XSH_WPMET                    189
    #define ID_XSH_WPMET_TEMPLATE           190
    #define ID_XSH_OS2SYSTEM                191
    #define ID_XSH_OS2SYSSETUPFDR           192
    #define ID_XSH_OS2CMDPROMPTSFDR         193
    #define ID_XSH_OS2CMD_OS2FULL           194
    #define ID_XSH_OS2CMD_OS2WIN            195
    #define ID_XSH_OS2CMD_DOSFULL           196
    #define ID_XSH_OS2CMD_DOSWIN            197
    #define ID_XSH_OS2CMD_WIN16             198
    #define ID_XSH_OS2DRIVESFDR             199
    #define ID_XSH_DISK_DISKETTE            200
    #define ID_XSH_DISK_HARDDISK            201
    #define ID_XSH_DISK_CDROM               202
    #define ID_XSH_DISK_NETWORK             203
    #define ID_XSH_OS2INFORMATIONFDR        204
    #define ID_XSH_OS2CMDREFINF             205
    #define ID_XSH_GAMESFDR                 206
    #define ID_XSH_UTILITIESFDR             207
    #define ID_XSH_CLIPVIEW                 208
    #define ID_XSH_EPM                      209
    #define ID_XSH_ICONEDIT                 210
    #define ID_XSH_EEXE                     211
    #define ID_XSH_PICVIEW                  212
    #define ID_XSH_CPUMONITOR               213
    #define ID_XSH_SEEKANDSCAN              214
    #define ID_XSH_WARPCENTER               215
    #define ID_XSH_SHREDDER                 216
    #define ID_XSH_DESKTOP                  217
    #define ID_XSH_DESKTOP_SECONDARY        218
    #define ID_XSH_PROGRAMSFDR              219
    #define ID_XSH_PRINTERSFDR              220
    #define ID_XSH_INSTALLREMOVEFDR         221
    #define ID_XSH_SYSSETUP_LOOK            222
    #define ID_XSH_SYSSETUP_NET             223
    #define ID_XSH_SYSSETUP_LOCALE          224
    #define ID_XSH_SYSSETUP_LVMGUI          225
    #define ID_XSH_SYSSETUP_MMPM2           226
    #define ID_XSH_SYSSETUP_REGEDIT         227
    #define ID_XSH_SYSSETUP_ESTYLER         228
    #define ID_XSH_SYSSETUP_THEMEMGR        229
    #define ID_XSH_SYSSETUP_ITHEME          230
    #define ID_XSH_SYSSETUP_WINOS2          231
    #define ID_XSH_CONNECTIONSFDR           232
    #define ID_XSH_INTERNETFDR              233
    #define ID_XSH_WPNETWORK                234
    #define ID_XSH_WPNETGRP                 235
    #define ID_XSH_DRIVES_REFRESHMEDIA      236
    #define ID_XSH_PASTEDLG                 237

    #define ID_XSH_SETTINGS_FDRHOTKEYS_DLG  238     // V1.0.0 (2002-09-12) [umoeller]

    #define ID_XSH_SETTINGS_TOOLBARS1       239     // V1.0.1 (2002-12-08) [umoeller]

    #define ID_XSH_SETTINGS_FILEPAGE3       240     // V1.0.1 (2003-01-10) [umoeller]

    #define ID_XSH_ADMIN_PROCESSES          241     // V1.0.2 (2003-11-13) [umoeller]
#endif

