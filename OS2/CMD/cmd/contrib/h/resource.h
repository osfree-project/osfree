// RESOURCE.H -- Resource IDs for Take Command/16, Take Command/32, and
// Take Command for OS/2

//  Copyright 1995  Rex Conn and JP Software Inc.  All rights reserved

// Lines beginning with //NUM are designed for processing by RENUM.BTM.
// DO NOT remove these lines!  See RENUM.BTM for syntax documentation.

// ---------------------------------- General --------------------------
// Hexadecimal IDs won't work for controls (due to limitations in Win16
// Resource Compiler)
//NUM DEC ON
//NUM NEXT 100
// General resource IDs
#define CLIACCEL                100
#define NONCUAACCEL             101
#define CLIICON                 102
#define IDC_BUTTON              103
#define ID_APPNAME              104
// Identifier for all notebook windows
#define ID_NB                   105

// ---------------------------------- Dialog box IDs -------------------
//NUM NEXT 200
#define SETUPDLGBOX             200
#define PRINTDLGBOX             201
#define ENVDLGBOX               202
#define TOOLBARDLGBOX           203
#define DESCRIBEDLGBOX          204
#define ABOUTDLGBOX             205
#define SIGNONDLGBOX1           206
#define SIGNONDLGBOX2           207
#define SIGNONDLGBOX3           208
#define SIGNONDLGBOX4           209
#define FFINDDLGBOX             210
#define FFINDINFODLGBOX         211
#define RUNDLGBOX               212
#define TASKDLGBOX              213
#define TASKINFODLGBOX          214
#define PMGROUPSDLGBOX          215
#define PMITEMSDLGBOX           216
#define INIMASTERDLGBOX         217
#define ERRORDLGBOX             218
#define CAVEMANDLGBOX           221
#define LISTFINDDLGBOX          222
#define LISTGOTODLGBOX          223
#define QUERYDLGBOX             224
#define SSDEBUGDLGBOX           225

// Notebook dialog box IDs
//NUM ROUND
#define CONFIG_NOTEBOOKDLGBOX   230
#define CONFIG_STARTUPDLGBOX    231
#define CONFIG_DISPLAYDLGBOX    232
#define CONFIG_CMDLINEDLGBOX    233
#define CONFIG_CMDLINE2DLGBOX   234
#define CONFIG_OPT1DLGBOX       235
#define CONFIG_OPT2DLGBOX       236
#define CONFIG_CMDDLGBOX        237
#define TTYAPPSDLGBOX           238

// ---------------------------------- Menus ----------------------------
//NUM NEXT 300
// File menu
#define IDM_SAVE_WINDOW         300
#define IDM_PRINT_WINDOW        301
#define IDM_PRINT_SETUP         302
#define IDM_REFRESH             303
#if _PM
#define IDM_SHUTDOWN_OS2        307
#endif
#define IDM_EXIT                308

// Edit menu
//NUM ROUND
#define IDM_CUT                 320
#define IDM_COPY                321
#define IDM_PASTE               322
#define IDM_CLEAR               323
#define IDM_COPY_CMDLINE        324
#define IDM_SELECT_ALL          325

// Utilities menu
//NUM ROUND
#define IDM_ALIASES             330
#define IDM_ENVIRONMENT         331
#define IDM_TOOLBAR             332
#define IDM_DESCRIBE            333
#define IDM_EDITOR              334
#define IDM_RECORDER            335
#define IDM_PLAYBACK            336

// Options menu
//NUM ROUND
#define IDM_LOCATE              340
#define IDM_TOOLBARON           341
#define IDM_STATUSBARON         342
#define IDM_SETUP               343
#define IDM_FONT                344
#define IDM_TOOLBAR_FONT        345
#define IDM_STATBAR_FONT        346
#define IDM_COLOR               347
#define IDM_VMSETUP             348
#define IDM_CMSETUP             349

// Options / log menu
//NUM ROUND
#define IDM_COMMAND_LOG         360
#define IDM_HISTORY_LOG         361
#define IDM_PRINT_LOG           362

// Window menu
//NUM ROUND
#define IDM_CASCADE             370
#define IDM_TILE                371
#define IDM_ARRANGE             372

// Help menu
//NUM ROUND
#define IDM_HELP                380
#define IDM_HELP_CONTENTS       381
#define IDM_HELP_GENERAL        382
#define IDM_HELP_ON_HELP        383
#define IDM_HELP_USING          384
#define IDM_HELP_SEARCH         385
#define IDM_HELP_KEYS           386
#define IDM_HELP_ON_LIST        387
#define IDM_HELP_ON_SELECT      388
#define IDM_ABOUT               389

// Apps menu - *** MUST BE LAST MENU DEF!!! ***
//NUM ROUND
#define IDM_RUN                 400
#define IDM_CONSOLE             404
#define IDM_VIEW_CONSOLE        405
#if _PM
#define IDM_DESKTOP             406
#define IDM_OS2BOX              407
#define IDM_DOSBOX              408
#define IDM_WINDOWSBOX          409
#define IDM_TTYAPPS             410
#define IDM_TASK                411
#endif
#if _PM == 0
#define IDM_MORE_GROUPS         412
#define IDM_MORE_ITEMS          413
#endif
#define IDM_GROUP_ITEMS         414
#define IDM_WINDOW_GROUPS       415


// ---------------------- Dialog box and child window items ------------
//NUM NEXT 1000
// Global items
#define IDD_BROWSE              1000
#define IDD_HELP                1001
#define IDD_ADD                 1002
#define IDD_EDIT                1003
#define IDD_DELETE              1004
#define IDD_LIST                1005
#define IDD_FNAME               1006

// About dialog
//NUM ROUND
#define IDD_ABOUTICON           1010
#define IDD_TITLELINE           1011
#define IDD_VERSION             1012
#define IDD_BRAND               1013
#define IDD_SHAREWARE1          1014
#define IDD_SHAREWARE2          1015
//#define IDD_REGNAME 9999
#define IDD_WINDOWSMODE         1016
#define IDD_FREEMEM             1017
#define IDD_RESOURCES           1018

// Run dialog
//NUM ROUND
#define IDD_RUN_NORMAL          1030
#define IDD_RUN_MINIMIZED       1031
#define IDD_RUN_MAXIMIZED       1032

// Task list dialog
//NUM ROUND
#define IDT_TASKINFO_NAME       1040
#define IDT_TASKINFO_CMDLINE    1041
#define IDT_TASKINFO_DIRECTORY  1042
#define IDT_TASKINFO_PARENT     1043
#define IDL_TASKS               1044
#define IDL_WINDOWS             1045
#define IDL_PMGROUP             1046
#define IDL_PMITEM              1047
#define IDB_SWITCHTO            1048
#define IDB_CLOSE_WINDOW        1049
#define IDB_CANCEL              1050
#define IDB_TASKINFO            1051
#define IDB_TASK_TERMINATE      1052

// FFIND dialog
//NUM ROUND
#define IDD_FIND_CURDIR         1060
#define IDD_FIND_FILESPEC       1061
#define IDD_FIND_DRIVESPEC      1062
#define IDD_FIND_TEXT           1063
#define IDD_FIND_TOTALS         1064
#define IDD_FIND_CASE           1065
#define IDD_FIND_HEX            1066
#define IDD_FIND_HIDDEN         1067
#define IDD_FIND_SHOWALL        1068
#define IDD_FIND_SRCHDIR        1069
#define IDD_FIND_SRCHSUBDIR     1070
#define IDD_FIND_SRCHALL        1071
#define IDD_FIND_SRCHALL_DISKS  1072
#define IDD_FIND_OUTLIST        1073
#define IDD_FIND_FILEINFO       1074
#define IDB_FFIND_EXPORT        1075

// FFIND info dialog
#define IDD_FFINDINFO_FILENAME  1076
#define IDD_FFINDINFO_SIZE      1077
#define IDD_FFINDINFO_LASTWRITE 1078
#define IDD_FFINDINFO_LASTACCESS 1079
#define IDD_FFINDINFO_CREATED   1080
#define IDD_FFINDINFO_DESCRIPTION 1081
#define IDB_FFINDINFO_RUN       1082
#define IDB_FFINDINFO_LIST      1083
#define IDB_FFINDINFO_EDIT      1084

// Environment / alias dialog
//NUM ROUND
#define IDD_ENV_LIST            1090
#define IDD_ENV_VARNAME         1091
#define IDD_ENV_VARVALUE        1092
#define IDD_ENV_ADD             1093
#define IDD_ENV_DELETE          1094
#define IDD_ENV_IMPORT          1095
#define IDD_ENV_EXPORT          1096

// Toolbar dialog
//NUM ROUND
#define IDD_TOOL_LIST           1100
#define IDD_TOOL_LABEL          1101
#define IDD_TOOL_COMMAND        1102
#define IDD_TOOL_ECHO           1103
#define IDD_TOOL_ECHOCR         1104
#define IDD_TOOL_NOECHOCR       1105
#define IDD_TOOL_FONTSIZE       1106
#define IDD_TOOL_DELETE         1107

#if _PM
// PM Descriptions dialog
#define IDD_FILENAME_TXT        1108
#define IDD_FILENAME_ED         1109
#define IDD_DRIVE_TXT           1110
#define IDD_DRIVE_CB            1111
#define IDD_FILTER_TXT          1112
#define IDD_FILTER_CB           1113
#define IDD_DIRECTORY_TXT       1114
#define IDD_DIRECTORY_LB        1115
#define IDD_FILES_TXT           1116
#define IDD_FILES_LB            1117
#define IDD_HELP_PB             1118
#define IDD_APPLY_PB            1119
#define IDD_OK_PB               1120
#define IDD_CANCEL_PB           1121
#define IDD_DESCRIPT_ED         1122
#define IDD_DESCRIPT_TXT        1123
#endif

// Registration dialog
//NUM ROUND
#define IDD_REGISTER            1130

#if _PM
// PM Print dialog
//NUM ROUND
#define DID_PRINTER_CB          1170
#define DID_SELECTION           1171
#define DID_COPIES_SP           1172
#define DID_PRIORITY_SP         1173
#define DID_PROPERTIES          1174
#endif

// LIST toolbar
//NUM ROUND
#define IDB_LISTOPEN            1180
#define IDB_LISTINFO            1181
#define IDB_LISTPRINT           1182
#define IDB_LISTEXIT            1183
#define IDB_LISTCONTINUE        1184
#define IDB_LISTPREVIOUS        1185
#define IDB_LISTHEX             1186
#define IDB_LISTWRAP            1187
#define IDB_LISTHIBIT           1188
#define IDB_LISTFIND            1189
#define IDB_LISTNEXT            1190
#define IDB_LISTGOTO            1191

// LIST find dialog
//NUM ROUND
#define IDD_LIST_FIND_WHAT      1200
#define IDD_LIST_SEARCH_GROUPBOX 1201
#define IDD_LIST_SEARCHSTART    1202
#define IDD_LIST_SEARCHCURRENT  1203
#define IDD_LIST_SEARCHCASE     1204
#define IDD_LIST_SEARCHHEX      1205
#define IDD_LIST_SEARCHREVERSE  1206

// LIST goto dialog
#define IDD_LIST_GOTO           1207

// SELECT toolbar
//NUM ROUND
#define IDB_SELECTGO            1220
#define IDB_SELECTQUIT          1221
#define IDB_SELECTTOGGLE        1222
#define IDB_SELECTCLEAR         1223

#if _PM
//NUM ROUND
// PM Exception handler dialog
#define DLG_EXCEPTION           1230
#define IT_CSEIP                1231
#define IT_EXCEPTTYPE           1232
#define IT_EAX                  1233
#define IT_EBX                  1234
#define IT_ECX                  1235
#define IT_EDX                  1236
#define IT_EBP                  1237
#define IT_EDI                  1238
#define IT_ESI                  1239
#define IT_ESP                  1240
#define IT_CS                   1241
#define IT_DS                   1242
#define IT_ES                   1243
#define IT_FS                   1244
#define IT_GS                   1245
#define IT_SS                   1246
#define IT_FLAGS                1247
#define IT_FILE                 1248
#define IT_DEBUGINFO            1249
#define IT_PFNSTART             1250
#define IT_PFNEND               1251
#define IT_PID                  1252
#define IT_TID                  1253
#define IT_DETAIL               1254
#define IT_STATUS               1255
#endif

// QUERYBOX dialog
//NUM ROUND
#define IDD_QUERYPROMPT         1260
#define IDD_QUERYTEXT           1261
#define IDD_QUERYTEXT2          1262


//NUM ROUND
// Batch Single Step debugger dialog
#define IDD_SS_BATCHFILE        1270
#define IDD_SS_LINE             1271
#define IDD_SS_TRACE            1272
#define IDD_SS_STEP             1273
#define IDD_SS_JUMP             1274
#define IDD_SS_EXPAND           1275
#define IDD_SS_VARIABLES        1276
#define IDD_SS_ALIASES          1277
#define IDD_SS_LIST             1278
#define IDD_SS_HELP             1279
#define IDD_SS_QUIT             1280
#define IDD_SS_COMMAND          1281


// ---------------------- INI file dialogs ------------------
//NUM OFF 2
#define IDI_NULL 0
#define IDI_BASE 1500
//NUM NEXT 1501

// Constants for all pages
#define IDI_NOTEBOOK            1501
#define IDI_OK                  1502
#define IDI_SAVE                1503
#define IDI_CANCEL              1504
#define IDI_HELP                1505


// Startup
//NUM ROUND
#define IDI_STARTUP_START IDI_AutoExecPath
#define IDI_STARTUP_FOCUS IDI_4StartPath
#define IDI_AutoExecPath        1521
#define IDI_AutoExecParms       1522
#define IDI_UMBLoad             1523
#define IDI_UMBHistory          1524
#define IDI_UMBDirHistory       1525
#define IDI_UMBEnvironment      1526
#define IDI_UMBAlias            1527
#define IDI_LocalAliases        1528
#define IDI_LocalDirHistory     1529
#define IDI_LocalHistory        1530
#define IDI_LoadAssoc           1531
#define IDI_Alias               1532
#define IDI_EnvFree             1533
#define IDI_Environment         1534
#define IDI_Swapping            1535
#define IDI_InstallPath         1536

#define IDI_4STARTGROUP         1537
#define IDI_PATHLABEL           1538
#define IDI_4StartPath          1539
#define IDI_INIGROUP            1540
#define IDI_PauseOnError        1541
#define IDI_STARTUPOPTIONSGROUP 1542
#define IDI_INIQuery            1543
#define IDI_BUFFERGROUP         1544
#define IDI_CMDHISTLABEL        1545
#define IDI_History             1546
#define IDI_DIRHISTLABEL        1547
#define IDI_DirHistory          1548
#define IDI_CURSORGROUP         1549
#define IDI_IBeamNo             1550
#define IDI_IBeamYes            1551
#define IDI_DISPSTARTGROUP      1552
#define IDI_WindowStateStd      1553
#define IDI_WindowStateMax      1554
#define IDI_WindowStateMin      1555
#define IDI_WindowStateCustom   1556
#define IDI_XPOSLABEL           1557
#define IDI_WindowX             1558
#define IDI_YPOSLABEL           1559
#define IDI_WindowY             1560
#define IDI_WINWIDTHLABEL       1561
#define IDI_WindowWidth         1562
#define IDI_WINHEIGHTLABEL      1563
#define IDI_WindowHeight        1564
#define IDI_CONSOLEGROUP        1565
#define IDI_CONROWSLABEL        1566
#define IDI_ConsoleRows         1567
#define IDI_CONCOLSLABEL        1568
#define IDI_ConsoleColumns      1569
#define IDI_HideConsole         1570
#define IDI_StackSize           1571
#define IDI_STARTUP_END IDI_StackSize

// Display
//NUM ROUND
#define IDI_DISPLAY_START IDI_TEXTLABEL
#define IDI_DISPLAY_FOCUS IDI_Tabs
#define IDI_TEXTLABEL           1581
#define IDI_TEXTWIDTHLABEL      1582
#define IDI_ScreenColumns       1583
#define IDI_TEXTHEIGHTLABEL     1584
#define IDI_ScreenRows          1585
#define IDI_TEXTTABSLABEL       1586
#define IDI_Tabs                1587
#define IDI_WINCFGGROUP         1588
#define IDI_TOOLBARLABEL        1589
#define IDI_ToolBarOn           1590
#define IDI_STATUSBARLABEL      1591
#define IDI_StatusBarOn         1592
#define IDI_SCROLLGROUP         1593
#define IDI_SCROLLBUFLABEL      1594
#define IDI_ScreenBufSize       1595
#define IDI_SCROLLLINESLABEL    1596
#define IDI_ScrollLines         1597
#define IDI_COLORSGROUP         1598
#define IDI_STARTFGLABEL        1599
#define IDI_STARTBGLABEL        1600
#define IDI_STDCOLORSLABEL      1601
#define IDI_StdColorsFG         1602
#define IDI_StdColorsBG         1603
#define IDI_INPUTCOLORSLABEL    1604
#define IDI_InputColorsFG       1605
#define IDI_InputColorsBG       1606
#define IDI_LISTBOXCOLORSGROUP  1607
#define IDI_LISTBOXCOLORSLABEL  1608
#define IDI_LISTBOXFGLABEL      1609
#define IDI_LISTBOXBGLABEL      1610
#define IDI_ListBoxBarColorsFG  1611
#define IDI_ListBoxBarColorsBG  1612
#define IDI_TBFONTLABEL         1613
#define IDI_ToolBarText         1614
#define IDI_SBFONTLABEL         1615
#define IDI_StatusBarText       1616
#define IDI_ANSILABEL           1617
#define IDI_ANSI                1618
#define IDI_DISPLAYOPTIONSGROUP 1619
#define IDI_BrightBG            1620
#define IDI_ChangeTitle         1621
#define IDI_DISPLAY_END IDI_ChangeTitle

// Command line 1
//NUM ROUND
#define IDI_CMDLINE_START IDI_EDITGROUP
#define IDI_CMDLINE_FOCUS IDI_EditModeOver
#define IDI_EDITGROUP           1631
#define IDI_EDITMODELABEL       1632
#define IDI_EditModeOver        1633
#define IDI_EditModeIns         1634
#define IDI_CURSORLABEL         1635
#define IDI_CURSORINSLABEL      1636
#define IDI_CursorIns           1637
#define IDI_CURSORINSPCT        1638
#define IDI_CURSOROVERLABEL     1639
#define IDI_CursorOver          1640
#define IDI_CURSOROVERPCT       1641
#define IDI_CMDHISTGROUP        1642
#define IDI_SWAPSCROLLLABEL     1643
#define IDI_NormalScrollKeys    1644
#define IDI_SwapScrollKeys      1645
#define IDI_HISTMINLABEL        1646
#define IDI_HistMin             1647
#define IDI_HISTMINLABEL2       1648
#define IDI_HistCopy            1649
#define IDI_FILECOMPLETIONGROUP 1650
#define IDI_FILECOMPLETIONLABEL 1651
#define IDI_FileCompletion      1652
#define IDI_HistMove            1653
#define IDI_HistWrap            1654
#define IDI_AppendToDir         1655
#define IDI_HistNoDups          1656
#define IDI_CMDLINE_END IDI_HistNoDups

// Command line 2
//NUM ROUND
#define IDI_CMDLINE2_START IDI_POPUPWINGROUP
#define IDI_CMDLINE2_FOCUS IDI_PopupWinLeft
#define IDI_POPUPWINGROUP       1661
#define IDI_POPUPWINPOSLABEL    1662
#define IDI_POPUPWINLEFTLABEL   1663
#define IDI_PopupWinLeft        1664
#define IDI_POPUPWINTOPLABEL    1665
#define IDI_PopupWinTop         1666
#define IDI_POPUPWINSIZELABEL   1667
#define IDI_POPUPWINWIDTHLABEL  1668
#define IDI_PopupWinWidth       1669
#define IDI_POPUPWINHEIGHTLABEL 1670
#define IDI_PopupWinHeight      1671
#define IDI_POPUPWINCOLORLABEL  1672
#define IDI_POPUPWINFGLABEL     1673
#define IDI_POPUPWINBGLABEL     1674
#define IDI_PopupWinColorsFG    1675
#define IDI_PopupWinColorsBG    1676
#define IDI_POPUPWINLABEL       1677
#define IDI_CDDWINLABEL         1678
#define IDI_CDDWinLeft          1679
#define IDI_CDDWinTop           1680
#define IDI_CDDWinWidth         1681
#define IDI_CDDWinHeight        1682
#define IDI_CDDWINCOLORLABEL    1683
#define IDI_CDDWinColorsFG      1684
#define IDI_CDDWinColorsBG      1685
#define IDI_FUZZYCDGROUP        1686
#define IDI_FUZZYCDLABEL        1687
#define IDI_FuzzyCD0            1688
#define IDI_FuzzyCD1            1689
#define IDI_FuzzyCD2            1690
#define IDI_FuzzyCD3            1691
#define IDI_TREEPATHLABEL       1694
#define IDI_TreePath            1695
#define IDI_CMDLINE2_END IDI_TreePath

// Options 1
//NUM ROUND
#define IDI_CONFIG1_START IDI_DESCRIPGROUP
#define IDI_CONFIG1_FOCUS IDI_Descriptions
#define IDI_DESCRIPGROUP        1701
#define IDI_Descriptions        1702
#define IDI_DESCLENLABEL        1703
#define IDI_DescriptionMax      1704
#define IDI_SPECCHARGROUP       1705
#define IDI_CMDSEPLABEL         1706
#define IDI_CommandSep          1707
#define IDI_ESCCHARLABEL        1708
#define IDI_EscapeChar          1709
#define IDI_PARCHARLABEL        1710
#define IDI_ParameterChar       1711
#define IDI_DECCHARLABEL        1712
#define IDI_DecCharAuto         1713
#define IDI_DecCharPeriod       1714
#define IDI_DecCharComma        1715
#define IDI_THOUCHARLABEL       1716
#define IDI_ThouCharAuto        1717
#define IDI_ThouCharPeriod      1718
#define IDI_ThouCharComma       1719
#define IDI_OPTIONSGROUP        1720
#define IDI_UpperCase           1721
#define IDI_BatchEcho           1722
#define IDI_NoClobber           1723
#define IDI_Win95SFNSearch      1724
#define IDI_TIMELABEL           1725
#define IDI_AmPmCtry            1726
#define IDI_AmPm12              1727
#define IDI_AmPm24              1728
#define IDI_CUALABEL            1729
#define IDI_CUANo               1730
#define IDI_CUAYes              1731
#define IDI_DescriptionName     1732
#define IDI_BEEPGROUP           1733
#define IDI_BEEPLENLABEL        1734
#define IDI_BeepLength          1735
#define IDI_BEEPMSLABEL         1736
#define IDI_BEEPFREQLABEL       1737
#define IDI_BeepFreq            1738
#define IDI_BEEPHZLABEL         1739
#define IDI_CONFIG1_END IDI_BEEPHZLABEL

// Options 2
//NUM ROUND
#define IDI_CONFIG2_START IDI_LOGGROUP
#define IDI_CONFIG2_FOCUS IDI_LogOn
#define IDI_LOGGROUP            1751
#define IDI_LogOn               1752
#define IDI_LOGFILELABEL        1753
#define IDI_LogName             1754
#define IDI_HistLogOn           1755
#define IDI_HLOGFILELABEL       1756
#define IDI_HistLogName         1757
#define IDI_PROGMANGROUP        1758
#define IDI_DDELABEL            1759
#define IDI_ProgmanDDEAuto      1760
#define IDI_ProgmanDDEEnable    1761
#define IDI_ProgmanDDEDisable   1762
#define IDI_EVALGROUP           1763
#define IDI_EVALMINLABEL        1764
#define IDI_EvalMin             1765
#define IDI_EVALMINDIGLABEL     1766
#define IDI_EVALPRECISIONLABEL  1767
#define IDI_EVALMAXLABEL        1768
#define IDI_EvalMax             1769
#define IDI_EVALMAXDIGLABEL     1770
#define IDI_EXTPROGGROUP        1771
#define IDI_ExecWait            1772
#define IDI_TASKLISTGROUP       1773
#define IDI_TCMDTaskList        1774
#define IDI_HelpPath            1775
#define IDI_HelpOptions         1776
#define IDI_HELPBOOKGROUP       1777
#define IDI_HelpBook            1778
#define IDI_SWAPPATHGROUP       1779
#define IDI_SwapPath            1780
#define IDI_REXXPath            1781
#define IDI_CONFIG2_END IDI_REXXPath


// Commands
//NUM ROUND
#define IDI_CMD_START IDI_DIRGROUP
#define IDI_CMD_FOCUS IDI_ColorDir
#define IDI_DIRGROUP            1791
#define IDI_ColorDir            1792
#define IDI_LISTGROUP           1793
#define IDI_LISTCOLORSLABEL     1794
#define IDI_LISTFGLABEL         1795
#define IDI_LISTBGLABEL         1796
#define IDI_ListColorsFG        1797
#define IDI_ListColorsBG        1798
#define IDI_LISTSTATCOLORSLABEL 1799
#define IDI_ListStatBarColorsFG 1800
#define IDI_ListStatBarColorsBG 1801
#define IDI_PRINTERLABEL        1802
#define IDI_Printer             1803
#define IDI_SELGROUP            1804
#define IDI_SELCOLORSLABEL      1805
#define IDI_SELFGLABEL          1806
#define IDI_SELBGLABEL          1807
#define IDI_SelectColorsFG      1808
#define IDI_SelectColorsBG      1809
#define IDI_SELECTSTATCOLORSLABEL 1810
#define IDI_SelectStatBarColorsFG 1811
#define IDI_SelectStatBarColorsBG 1812
#define IDI_EDITORGROUP         1813
#define IDI_EDITORLABEL         1814
#define IDI_Editor              1815
#define IDI_CMD_END IDI_Editor

#define IDI_ID_MAX IDI_CMD_END


// Symbols used in INIFILE.H for items with multiple controls for a single
// INI file directive (radio button or color items)
#define IDI_IBeamCursor IDI_IBeamNo
#define IDI_WindowState IDI_WindowStateStd
#define IDI_StdColors IDI_StdColorsFG
#define IDI_InputColors IDI_InputColorsFG
#define IDI_EditMode IDI_EditModeOver
#define IDI_PopupWinColors IDI_PopupWinColorsFG
#define IDI_CDDWinColors IDI_CDDWinColorsFG
#define IDI_AmPm IDI_AmPmCtry
#define IDI_ProgmanDDE IDI_ProgmanDDEAuto
#define IDI_ListColors IDI_ListColorsFG
#define IDI_ListBoxBarColors IDI_ListBoxBarColorsFG
#define IDI_SelectColors IDI_SelectColorsFG
#define IDI_FuzzyCD IDI_FuzzyCD0
#define IDI_CUA IDI_CUANo
#define IDI_EvalDecimal IDI_EvalDecimalPeriod
#define IDI_ListStatBarColors IDI_ListStatBarColorsFG
#define IDI_SelectStatBarColors IDI_SelectStatBarColorsFG
#define IDI_DecimalChar IDI_DecCharAuto
#define IDI_ThousandsChar IDI_ThouCharAuto


// TTY Apps
//NUM ROUND
#define IDD_TTY_LIST            1820
#define IDD_TTY_SAVE            1821
#define IDD_TTY_ADD             1822
#define IDD_TTY_DELETE          1823
#define IDD_TTY_COMMAND         1824
#define IDD_TTY_OS2             1825
#define IDD_TTY_DOS             1826
#define IDD_TTY_ENABLE          1827

#define IDD_CAVE_DEFAULT        1828
#define IDD_CAVE_LIST           1829
#define IDD_CAVE_SAVE           1830
#define IDD_CAVE_ADD            1831
#define IDD_CAVE_DELETE         1832
#define IDD_CAVE_COMMAND        1833
#define IDD_CAVE_ENABLE         1834
#define IDD_CAVE_TTY            1835

// text depending on configuration
#define IDI_POPUPWINTOPLABEL_TOP_TEXT       "Top"
#define IDI_POPUPWINTOPLABEL_ABOVE_TEXT     "Above"


