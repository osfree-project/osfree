// INIFILE.H -- Include file for INI file and key parsing structures, macros,
// constants, and strings
//    Copyright 1992-1998 JP Software Inc., All Rights Reserved


// Control types for controls in INI file dialog boxes
#define INI_CTL_NULL     0
#define INI_CTL_INT      1
#define INI_CTL_UINT     2
#define INI_CTL_TEXT     3
#define INI_CTL_CHECK    4
#define INI_CTL_RADIO    5
#define INI_CTL_COLOR    6
#define INI_CTL_BOX      7
#define INI_CTL_TAB      8
#define INI_CTL_STATIC   9
#define INI_CTL_BUTTON   10
#define INI_CTL_COMBO    11

#define MAX_INI_NEST 3

// INI file item list
typedef struct {
    char *pszItemName;
    unsigned char cParseType;
    unsigned int uDefValue;
    void *pValidate;
    void *pItemData;
    unsigned int uControlID;
    unsigned char cControlType;
    unsigned char cRadioCnt;
} INI_ITEM;

// ASCII and scan codes for all keys

#define K_SCAN    0x100
#define K_CtlAt   0x00
#define K_Null    K_CtlAt
#define K_CtlA    0x01
#define K_CtlB    0x02
#define K_CtlC    0x03
#define K_CtlD    0x04
#define K_CtlE    0x05
#define K_CtlF    0x06
#define K_CtlG    0x07
#define K_CtlH    0x08
#define K_Bksp    K_CtlH
#define K_CtlI    0x09
#define K_Tab     K_CtlI
#define K_CtlJ    0x0A
#define K_LF      K_CtlJ
#define K_CtlEnt  K_CtlJ
#define K_CtlK    0x0B
#define K_CtlL    0x0C
#define K_CtlM    0x0D
#define K_Enter   K_CtlM
#define K_CR      K_CtlM
#define K_CtlN    0x0E
#define K_CtlO    0x0F
#define K_CtlP    0x10
#define K_CtlQ    0x11
#define K_CtlR    0x12
#define K_CtlS    0x13
#define K_CtlT    0x14
#define K_CtlU    0x15
#define K_CtlV    0x16
#define K_CtlW    0x17
#define K_CtlX    0x18
#define K_CtlY    0x19
#define K_CtlZ    0x1A
#define K_Esc     0x1B
#define K_FS      0x1C
#define K_GS      0x1D
#define K_RS      0x1E
#define K_US      0x1F
#define K_Space   0x20
#define K_Excl    0x21
#define K_DQuote  0x22
#define K_Pound   0x23
#define K_Dollar  0x24
#define K_Pct     0x25
#define K_And     0x26
#define K_SQuote  0x27
#define K_LPar    0x28
#define K_RPar    0x29
#define K_Star    0x2A
#define K_Plus    0x2B
#define K_Comma   0x2C
#define K_Dash    0x2D
#define K_Period  0x2E
#define K_Slash   0x2F
#define K_0       0x30
#define K_1       0x31
#define K_2       0x32
#define K_3       0x33
#define K_4       0x34
#define K_5       0x35
#define K_6       0x36
#define K_7       0x37
#define K_8       0x38
#define K_9       0x39
#define K_Colon   0x3A
#define K_Semi    0x3B
#define K_LAngle  0x3C
#define K_Equal   0x3D
#define K_RAngle  0x3E
#define K_Quest   0x3F
#define K_AtSign  0x40
#define K_A       0x41
#define K_B       0x42
#define K_C       0x43
#define K_D       0x44
#define K_E       0x45
#define K_F       0x46
#define K_G       0x47
#define K_H       0x48
#define K_I       0x49
#define K_J       0x4A
#define K_K       0x4B
#define K_L       0x4C
#define K_M       0x4D
#define K_N       0x4E
#define K_O       0x4F
#define K_P       0x50
#define K_Q       0x51
#define K_R       0x52
#define K_S       0x53
#define K_T       0x54
#define K_U       0x55
#define K_V       0x56
#define K_W       0x57
#define K_X       0x58
#define K_Y       0x59
#define K_Z       0x5A
#define K_LSqr    0x5B
#define K_BSlash  0x5C
#define K_RSqr    0x5D
#define K_Caret   0x5E
#define K_Under   0x5F
#define K_BQuote  0x60
#define K_a       0x61
#define K_b       0x62
#define K_c       0x63
#define K_d       0x64
#define K_e       0x65
#define K_f       0x66
#define K_g       0x67
#define K_h       0x68
#define K_i       0x69
#define K_j       0x6A
#define K_k       0x6B
#define K_l       0x6C
#define K_m       0x6D
#define K_n       0x6E
#define K_o       0x6F
#define K_p       0x70
#define K_q       0x71
#define K_r       0x72
#define K_s       0x73
#define K_t       0x74
#define K_u       0x75
#define K_v       0x76
#define K_w       0x77
#define K_x       0x78
#define K_y       0x79
#define K_z       0x7A
#define K_LBrace  0x7B
#define K_Bar     0x7C
#define K_RBrace  0x7D
#define K_Tilde   0x7E
#define K_CtlBS   0x7F
#define K_255     0xFF
#define K_AltA    0x1E + K_SCAN
#define K_AltB    0x30 + K_SCAN
#define K_AltC    0x2E + K_SCAN
#define K_AltD    0x20 + K_SCAN
#define K_AltE    0x12 + K_SCAN
#define K_AltF    0x21 + K_SCAN
#define K_AltG    0x22 + K_SCAN
#define K_AltH    0x23 + K_SCAN
#define K_AltI    0x17 + K_SCAN
#define K_AltJ    0x24 + K_SCAN
#define K_AltK    0x25 + K_SCAN
#define K_AltL    0x26 + K_SCAN
#define K_AltM    0x32 + K_SCAN
#define K_AltN    0x31 + K_SCAN
#define K_AltO    0x18 + K_SCAN
#define K_AltP    0x19 + K_SCAN
#define K_AltQ    0x10 + K_SCAN
#define K_AltR    0x13 + K_SCAN
#define K_AltS    0x1F + K_SCAN
#define K_AltT    0x14 + K_SCAN
#define K_AltU    0x16 + K_SCAN
#define K_AltV    0x2F + K_SCAN
#define K_AltW    0x11 + K_SCAN
#define K_AltX    0x2D + K_SCAN
#define K_AltY    0x15 + K_SCAN
#define K_AltZ    0x2C + K_SCAN
#define K_Alt0    0x81 + K_SCAN
#define K_Alt1    0x78 + K_SCAN
#define K_Alt2    0x79 + K_SCAN
#define K_Alt3    0x7A + K_SCAN
#define K_Alt4    0x7B + K_SCAN
#define K_Alt5    0x7C + K_SCAN
#define K_Alt6    0x7D + K_SCAN
#define K_Alt7    0x7E + K_SCAN
#define K_Alt8    0x7F + K_SCAN
#define K_Alt9    0x80 + K_SCAN
#define K_AltBS   0x0E + K_SCAN
#define K_AltTab  0xA5 + K_SCAN
#define K_ShfTab  0x0F + K_SCAN
#define K_CtlTab  0x94 + K_SCAN
#define K_Left    0x4B + K_SCAN
#define K_CtlLft  0x73 + K_SCAN
#define K_Right   0x4D + K_SCAN
#define K_CtlRt   0x74 + K_SCAN
#define K_Up      0x48 + K_SCAN
#define K_CtlUp   0x8D + K_SCAN
#define K_Down    0x50 + K_SCAN
#define K_CtlDn   0x91 + K_SCAN
#define K_Home    0x47 + K_SCAN
#define K_CtlHm   0x77 + K_SCAN
#define K_End     0x4F + K_SCAN
#define K_CtlEnd  0x75 + K_SCAN
#define K_PgUp    0x49 + K_SCAN
#define K_CtlPgU  0x84 + K_SCAN
#define K_PgDn    0x51 + K_SCAN
#define K_CtlPgD  0x76 + K_SCAN
#define K_Ins     0x52 + K_SCAN
#define K_Del     0x53 + K_SCAN
#define K_CtlIns  0x92 + K_SCAN
#define K_CtlDel  0x93 + K_SCAN
#define K_F1      0x3B + K_SCAN
#define K_F2      0x3C + K_SCAN
#define K_F3      0x3D + K_SCAN
#define K_F4      0x3E + K_SCAN
#define K_F5      0x3F + K_SCAN
#define K_F6      0x40 + K_SCAN
#define K_F7      0x41 + K_SCAN
#define K_F8      0x42 + K_SCAN
#define K_F9      0x43 + K_SCAN
#define K_F10     0x44 + K_SCAN
#define K_F11     0x85 + K_SCAN
#define K_F12     0x86 + K_SCAN
#define K_ShfF1   0x54 + K_SCAN
#define K_ShfF2   0x55 + K_SCAN
#define K_ShfF3   0x56 + K_SCAN
#define K_ShfF4   0x57 + K_SCAN
#define K_ShfF5   0x58 + K_SCAN
#define K_ShfF6   0x59 + K_SCAN
#define K_ShfF7   0x5A + K_SCAN
#define K_ShfF8   0x5B + K_SCAN
#define K_ShfF9   0x5C + K_SCAN
#define K_ShfF10  0x5D + K_SCAN
#define K_ShfF11  0x87 + K_SCAN
#define K_ShfF12  0x88 + K_SCAN
#define K_CtlF1   0x5E + K_SCAN
#define K_CtlF2   0x5F + K_SCAN
#define K_CtlF3   0x60 + K_SCAN
#define K_CtlF4   0x61 + K_SCAN
#define K_CtlF5   0x62 + K_SCAN
#define K_CtlF6   0x63 + K_SCAN
#define K_CtlF7   0x64 + K_SCAN
#define K_CtlF8   0x65 + K_SCAN
#define K_CtlF9   0x66 + K_SCAN
#define K_CtlF10  0x67 + K_SCAN
#define K_CtlF11  0x89 + K_SCAN
#define K_CtlF12  0x8A + K_SCAN
#define K_AltF1   0x68 + K_SCAN
#define K_AltF2   0x69 + K_SCAN
#define K_AltF3   0x6A + K_SCAN
#define K_AltF4   0x6B + K_SCAN
#define K_AltF5   0x6C + K_SCAN
#define K_AltF6   0x6D + K_SCAN
#define K_AltF7   0x6E + K_SCAN
#define K_AltF8   0x6F + K_SCAN
#define K_AltF9   0x70 + K_SCAN
#define K_AltF10  0x71 + K_SCAN
#define K_AltF11  0x8B + K_SCAN
#define K_AltF12  0x8C + K_SCAN


// Definitions for INI file parsing (INIPARSE.C)

// INI file parsing types
#define INI_BYTE      0
#define INI_CHAR      1
#define INI_INT       2
#define INI_UINT      3
#define INI_CHOICE    4
#define INI_COLOR     5
#define INI_KEY       6
#define INI_STR       7
#define INI_PATH      8
#define INI_KEY_MAP   9
#define INI_INCLUDE  10

#define INI_PTMASK   0x7F
#define INI_NOMOD       0x80


// bits for cvtkey, used in key mapping directives
#define D_EXTKEY 0x0100
#define D_MAP_GEN 0x0200
#define D_MAP_EDIT 0x0400
#define D_MAP_HWIN 0x0800
#define D_MAP_LIST 0x1000
#define D_NORM_KEY 0x8000

#define VNULL (void *)0


// Data for INI file parsing (INIPARSE.C)

#ifdef INIPARSE

// Validation ranges, token lists, and functions for INI file items

static int V_HstRng[] = {256, 32767};           // history range
static int V_DirHstRng[] = {256, 32767};        // directory history range
static int V_DesRng[] = {20, 511};              // description max range
static int V_CurRng[] = {-1, 100};              // cursor shape range
static int V_HMRng[] = {0, 1024};               // history minimum save range
static int V_WPRng[] = {0, 2048};               // window position range
static int V_WSRng[] = {0, 4096};               // window size range

static int V_FuzzyCD[] = {0, 3};            // Fuzzy completion style
static int V_BaseRng[] = {0, 1, 1};             // Base 0 or 1 for 1st element
static int V_EvalRng[] = {0, 8, 1};             // @EVAL precision range
static int V_BFRng[] = {0, 20000, 1000};        // beep frequency range
static int V_BLRng[] = {0, 54, 1};              // beep length range
static int V_RCRng[] = {0, 2048, 1};            // row / column count range
static int V_RelRng[] = {-100, 2048, 1};        // window relative to cursor range
static int V_Tabs[] = {1, 32, 1};                // tabstops
static int V_012[] = {0, 2};                    // dir - new byte display style, 0...old style, 1...with MB/GB and without allocated space, 2...with MB/GB and allocated space
                                                // ChangeDirWithout trailing /, 0...no, 1...before search on extern executable, 2...after extern executable

static char *YNList[] = {"No", "Yes"};      // yes or no
TOKEN_LIST(V_YesNo, YNList);

static char *YNAList[] = {"Auto", "Yes", "No"};      // yes, no, or auto
TOKEN_LIST(V_YesNoAuto, YNAList);

static char *OnOff[] = {"Off", "On"};      // off or on
TOKEN_LIST(V_OnOff, OnOff);
                                    // edit modes
static char *EMList[] = {"Overstrike", "Insert"};
TOKEN_LIST(V_EMList, EMList);
                                    // window states
static char *WState[] = {"Standard", "Maximize", "Minimize", "Custom"};
TOKEN_LIST(V_WState, WState);

static char *DTList[] = {"Auto", ".", ","};      // decimal / thousands chars
TOKEN_LIST(V_DTChar, DTList);

static char *SecNames[] = {"4OS2", "Primary", "Secondary"};  // section names
// FIXME - Is this stuff used in OPTION???
TOKEN_LIST(SectionNames, SecNames);

// Take Command dialog box control number macros
//    Embed Dialog ID in high byte, control type in third nibble, and
//    ID count for radio buttons in fourth nibble.
//#define DLG_DATA(CtlID, CtlType, RadioCnt) ((unsigned int)(((CtlID - IDI_BASE) << 8) | (CtlType << 4) | RadioCnt)),
#define DLG_DATA(CtlID, CtlType, RadioCnt) (unsigned int)(CtlID - IDI_BASE),  CtlType, RadioCnt,
#define DLG_NULL 0, 0, 0,


INI_ITEM gaINIItemList[] = {

    // All products
    "AmPm", INI_CHOICE, 2, &V_YesNoAuto, &gaInifile.TimeFmt, DLG_DATA(IDI_AmPm, INI_CTL_RADIO, 3)
    "AppendToDir", INI_CHOICE, 0, &V_YesNo, &gaInifile.AppendDir, DLG_DATA(IDI_AppendToDir, INI_CTL_CHECK, 0)
    //"Base", INI_UINT, 0, &V_BaseRng, &gaInifile.Base, DLG_NULL
    "BatchEcho", INI_CHOICE, 1, &V_YesNo, &gaInifile.BatEcho, DLG_DATA(IDI_BatchEcho, INI_CTL_CHECK, 0)
    "BeepFreq", INI_UINT, 440, &V_BFRng, &gaInifile.BeepFreq, DLG_DATA(IDI_BeepFreq, INI_CTL_INT, 0)
    "BeepLength", INI_UINT, 2, &V_BLRng, &gaInifile.BeepDur, DLG_DATA(IDI_BeepLength, INI_CTL_INT, 0)
    "CDDWinLeft", INI_UINT, 3, &V_RCRng, &gaInifile.CDDLeft, DLG_DATA(IDI_CDDWinLeft, INI_CTL_INT, 0)
    "CDDWinTop", INI_UINT, 3, &V_RCRng, &gaInifile.CDDTop, DLG_DATA(IDI_CDDWinTop, INI_CTL_INT, 0)
    "CDDWinWidth", INI_UINT, 72, &V_RCRng, &gaInifile.CDDWidth, DLG_DATA(IDI_CDDWinWidth, INI_CTL_INT, 0)
    "CDDWinHeight", INI_UINT, 16, &V_RCRng, &gaInifile.CDDHeight, DLG_DATA(IDI_CDDWinHeight, INI_CTL_INT, 0)
    "CDWithoutBackslash", INI_INT, 1, &V_012, &gaInifile.CDWithoutBackslash, DLG_NULL
    "ChangeTitle", INI_CHOICE, 1, &V_YesNo, &gaInifile.ChangeTitle, DLG_DATA(IDI_ChangeTitle, INI_CTL_CHECK, 0)
    "ClearKeyMap", INI_KEY_MAP, 0, VNULL, NULL, DLG_NULL
    "ColorDir", INI_STR, 0, VNULL, &gaInifile.DirColor, DLG_DATA(IDI_ColorDir, INI_CTL_TEXT, 0)
    "Debug", INI_UINT, 0, VNULL, &gaInifile.INIDebug, DLG_NULL
    "DecimalChar", INI_CHOICE, 0, &V_DTChar, &gaInifile.DecimalChar, DLG_DATA(IDI_DecimalChar, INI_CTL_RADIO, 3)
    "DescriptionMax", INI_UINT, 512, &V_DesRng, &gaInifile.DescriptMax, DLG_DATA(IDI_DescriptionMax, INI_CTL_INT, 0)
    "DescriptionName", INI_STR, 0, VNULL, &gaInifile.DescriptName, DLG_NULL
    "Descriptions", INI_CHOICE, 1, &V_YesNo, &gaInifile.Descriptions, DLG_DATA(IDI_Descriptions, INI_CTL_CHECK, 0)
    // Kludge - we use two entries here so that changes made by OPTION
    // get into the .INI file with the correct name, but do not affect the
    // values for the current session
    // 2013-04-25 SHL increase default from 512 to 4092 to match DosAllocSharedMem semantics and internal usage
    "DirHistory", (INI_UINT | INI_NOMOD), 4092, &V_DirHstRng, &gaInifile.DirHistorySize, DLG_NULL
    "DirHistory", INI_UINT, 4092, &V_DirHstRng, &gaInifile.DirHistoryNew, DLG_DATA(IDI_DirHistory, INI_CTL_INT, 0)
    //"ErrorColors", INI_COLOR, 0, VNULL, &gaInifile.ErrorColor, DLG_NULL
    "EvalMax", INI_UINT, 8, &V_EvalRng, &gaInifile.EvalMax, DLG_DATA(IDI_EvalMax, INI_CTL_INT, 0)
    "EvalMin", INI_UINT, 0, &V_EvalRng, &gaInifile.EvalMin, DLG_DATA(IDI_EvalMin, INI_CTL_INT, 0)
    "FileCompletion", INI_STR, 0, VNULL, &gaInifile.FC, DLG_DATA(IDI_FileCompletion, INI_CTL_TEXT, 0)
    "FuzzyCD", INI_UINT, 0, &V_FuzzyCD, &gaInifile.FuzzyCD, DLG_DATA(IDI_FuzzyCD, INI_CTL_RADIO, 4)
    "HistMin", INI_UINT, 0, &V_HMRng, &gaInifile.HistMin, DLG_DATA(IDI_HistMin, INI_CTL_INT, 0)
    // Kludge - see note under DirHistory directive above
    // 2013-04-25 SHL increase default from 1024 to 4092 to match DosAllocSharedMem semantics and internal usage
    "History", (INI_UINT | INI_NOMOD), 4092, &V_HstRng, &gaInifile.HistorySize, DLG_NULL
    "History", INI_UINT, 4092, &V_HstRng, &gaInifile.HistoryNew, DLG_DATA(IDI_History, INI_CTL_INT, 0)
    "HistCopy", INI_CHOICE, 0, &V_YesNo, &gaInifile.HistoryCopy, DLG_DATA(IDI_HistCopy, INI_CTL_CHECK, 0)
    "HistMove", INI_CHOICE, 0, &V_YesNo, &gaInifile.HistoryMove, DLG_DATA(IDI_HistMove, INI_CTL_CHECK, 0)
    "HistNoDups", INI_CHOICE, 0, &V_YesNo, &gaInifile.HistNoDups, DLG_DATA(IDI_HistNoDups, INI_CTL_CHECK, 0)
    "HistLogName", INI_PATH, 0, (void *)0x4000, &gaInifile.HistLogName, DLG_DATA(IDI_HistLogName, INI_CTL_TEXT, 0)
    "HistLogOn", INI_CHOICE, 0, &V_YesNo, &gaInifile.HistLogOn, DLG_DATA(IDI_HistLogOn, INI_CTL_CHECK, 0)
    "HistWrap", INI_CHOICE, 1, &V_YesNo, &gaInifile.HistoryWrap, DLG_DATA(IDI_HistWrap, INI_CTL_CHECK, 0)
    "Include", INI_INCLUDE, 0, VNULL, NULL, DLG_NULL
    "INIQuery", INI_CHOICE, 0, &V_YesNo, &gaInifile.INIQuery, DLG_NULL
    "InputColors", INI_COLOR, 0, VNULL, &gaInifile.InputColor, DLG_DATA(IDI_InputColors, INI_CTL_COLOR, 0)
//FIXME, need dialog info
    "ListColors", INI_COLOR, 0, VNULL, &gaInifile.ListColor, DLG_DATA(IDI_ListColors, INI_CTL_COLOR, 0)
    "ListRowStart", INI_UINT, 1, &V_BaseRng, &gaInifile.ListRowStart, DLG_NULL
    "LogName", INI_PATH, 0, (void *)0x4000, &gaInifile.LogName, DLG_DATA(IDI_LogName, INI_CTL_TEXT, 0)
    "LogOn", INI_CHOICE, 0, &V_YesNo, &gaInifile.LogOn, DLG_DATA(IDI_LogOn, INI_CTL_CHECK, 0)
    "NewByteDisp", INI_INT, 1, &V_012, &gaInifile.NewByteDisp, DLG_NULL
    "NoClobber", INI_CHOICE, 0, &V_YesNo, &gaInifile.NoClobber, DLG_DATA(IDI_NoClobber, INI_CTL_CHECK, 0)
    "PathExt", INI_CHOICE, 0, &V_YesNo, &gaInifile.PathExt, DLG_NULL
    "PopupWinHeight", INI_UINT, 12, &V_RCRng, &gaInifile.PWHeight, DLG_DATA(IDI_PopupWinHeight, INI_CTL_INT, 0)
    "PopupWinTop", INI_INT, 1, &V_RelRng, &gaInifile.PWTop, DLG_DATA(IDI_PopupWinTop, INI_CTL_INT, 0)
    "PopupWinPosRelative", INI_CHOICE, 1, &V_YesNo, &gaInifile.PopupPosRelative, DLG_NULL // ToDo: 20090422 AB DLG_DATA(IDI_PopupWinPosRelative, INI_CTL_CHECK, 0)
    "ScreenRows", INI_UINT, 0, &V_RCRng, &gaInifile.Rows, DLG_DATA(IDI_ScreenRows, INI_CTL_INT, 0)
    "SelectColors", INI_COLOR, 0, VNULL, &gaInifile.SelectColor, DLG_DATA(IDI_SelectColors, INI_CTL_COLOR, 0)
    "StdColors", INI_COLOR, 0, VNULL, &gaInifile.StdColor, DLG_DATA(IDI_StdColors, INI_CTL_COLOR, 0)
    "TabStops", INI_UINT, 8, &V_Tabs, &gaInifile.Tabs, DLG_DATA(IDI_Tabs, INI_CTL_INT, 0)
    "TitleIsCurDir", INI_CHOICE, 1, &V_YesNo, &gaInifile.TitleIsCurDir, DLG_NULL
    "ThousandsChar", INI_CHOICE, 0, &V_DTChar, &gaInifile.ThousandsChar, DLG_DATA(IDI_ThousandsChar, INI_CTL_RADIO, 3)
    "TreePath", INI_PATH, 0, VNULL, &gaInifile.TreePath, DLG_DATA(IDI_TreePath, INI_CTL_TEXT, 0)
    "SwitchChar", INI_CHAR, '/', VNULL, &gaInifile.SwChr, DLG_NULL
    "UnixPaths", INI_CHOICE, 0, &V_YesNo, &gaInifile.UnixPaths, DLG_NULL
    "UpperCase", INI_CHOICE, 0, &V_YesNo, &gaInifile.Upper, DLG_DATA(IDI_UpperCase, INI_CTL_CHECK, 0)

    "CommandSep", INI_CHAR, '&', VNULL, &gaInifile.CmdSep, DLG_DATA(IDI_CommandSep, INI_CTL_TEXT, 0)
    "EscapeChar", INI_CHAR, '^', VNULL, &gaInifile.EscChr, DLG_DATA(IDI_EscapeChar, INI_CTL_TEXT, 0)
    "ParameterChar", INI_CHAR, '$', VNULL, &gaInifile.ParamChr, DLG_DATA(IDI_ParameterChar, INI_CTL_TEXT, 0)

    // OS/2 only
    "HelpBook", INI_STR, 0, VNULL, &gaInifile.HelpBook, DLG_DATA(IDI_HelpBook, INI_CTL_TEXT, 0)
    "SwapFilePath", INI_PATH, 0, (void *)0x4000, &gaInifile.SwapPath, DLG_DATA(IDI_SwapPath, INI_CTL_TEXT, 0)

    "4StartPath", INI_PATH, 0, VNULL, &gaInifile.FSPath, DLG_DATA(IDI_4StartPath, INI_CTL_TEXT, 0)
    "CursorIns", INI_INT, 100, &V_CurRng, &gaInifile.CursI, DLG_DATA(IDI_CursorIns, INI_CTL_INT, 0)
    "CursorOver", INI_INT, 15, &V_CurRng, &gaInifile.CursO, DLG_DATA(IDI_CursorOver, INI_CTL_INT, 0)
    "EditMode", INI_CHOICE, 0, &V_EMList, &gaInifile.EditMode, DLG_DATA(IDI_EditMode, INI_CTL_RADIO, 2)
    "CDDWinColors", INI_COLOR, 0, VNULL, &gaInifile.CDDColor, DLG_DATA(IDI_CDDWinColors, INI_CTL_COLOR, 0)
    "ListboxBarColors", INI_COLOR, 0, VNULL, &gaInifile.LBBar, DLG_DATA(IDI_ListBoxBarColors, INI_CTL_COLOR, 0)
    "PopupWinColors", INI_COLOR, 0, VNULL, &gaInifile.PWColor, DLG_DATA(IDI_PopupWinColors, INI_CTL_COLOR, 0)
    "PopupWinLeft", INI_UINT, 40, &V_RCRng, &gaInifile.PWLeft, DLG_DATA(IDI_PopupWinLeft, INI_CTL_INT, 0)
    "PopupWinWidth", INI_UINT, 36, &V_RCRng, &gaInifile.PWWidth, DLG_DATA(IDI_PopupWinWidth, INI_CTL_INT, 0)
    "Printer", INI_STR, 0, VNULL, &gaInifile.Printer, DLG_DATA(IDI_Printer, INI_CTL_TEXT, 0)

    "ExecWait", INI_CHOICE, 0, &V_YesNo, &gaInifile.ExecWait, DLG_DATA(IDI_ExecWait, INI_CTL_CHECK, 0)
    "WindowHeight", INI_UINT, 0, &V_WSRng, &gaInifile.WindowHeight, DLG_DATA(IDI_WindowHeight, INI_CTL_INT, 0)
    "WindowState", INI_CHOICE, 0, &V_WState, &gaInifile.WindowState, DLG_DATA(IDI_WindowState, INI_CTL_RADIO, 4)
    "WindowWidth", INI_UINT, 0, &V_WSRng, &gaInifile.WindowWidth, DLG_DATA(IDI_WindowWidth, INI_CTL_INT, 0)
    "WindowX", INI_UINT, 0, &V_WPRng, &gaInifile.WindowX, DLG_DATA(IDI_WindowX, INI_CTL_INT, 0)
    "WindowY", INI_UINT, 0, &V_WPRng, &gaInifile.WindowY, DLG_DATA(IDI_WindowY, INI_CTL_INT, 0)

    "BrightBG", INI_CHOICE, 2, &V_YesNo, &gaInifile.BrightBG, DLG_DATA(IDI_BrightBG, INI_CTL_CHECK, 0)
    "LineInput", INI_CHOICE, 0, &V_YesNo, &gaInifile.LineIn, DLG_NULL

    "ListStatBarColors", INI_COLOR, 0, VNULL, &gaInifile.ListStatusColor, DLG_DATA(IDI_ListStatBarColors, INI_CTL_COLOR, 0)
    "PauseOnError", INI_CHOICE, 1, &V_YesNo, &gaInifile.PauseErr, DLG_NULL
    "SelectStatBarColors", INI_COLOR, 0, VNULL, &gaInifile.SelectStatusColor, DLG_DATA(IDI_SelectStatBarColors, INI_CTL_COLOR, 0)

    // Everything but TCMD/16
    "DuplicateBugs", INI_CHOICE, 1, &V_YesNo, &gaInifile.DupBugs, DLG_NULL
    "CMDDirMvCpy", INI_CHOICE, 0, &V_YesNo, &gaInifile.CMDDirMvCpy, DLG_NULL
    "LocalAliases", INI_CHOICE, 0, &V_YesNo, &gaInifile.LocalAliases, DLG_DATA(IDI_LocalAliases, INI_CTL_CHECK, 0)
    "LocalDirHistory", INI_CHOICE, 0, &V_YesNo, &gaInifile.LocalDirHistory, DLG_DATA(IDI_LocalDirHistory, INI_CTL_CHECK, 0)
    "LocalHistory", INI_CHOICE, 0, &V_YesNo, &gaInifile.LocalHistory, DLG_DATA(IDI_LocalHistory, INI_CTL_CHECK, 0)
    "NextINIFile", INI_STR, 0, VNULL, &gaInifile.NextININame, DLG_NULL
    "DateFmt", INI_STR, 0, VNULL, &gaInifile.DateFmt, DLG_NULL
    "TmSmpFmt", INI_STR, 0, VNULL, &gaInifile.TmSmpFmt, DLG_NULL
    "Year4Digit", INI_CHOICE, 0, &V_YesNo, &gaInifile.Year4Digit, DLG_DATA(IDI_HistNoDups, INI_CTL_CHECK, 0)
    "Day2Digit", INI_CHOICE, 0, &V_YesNo, &gaInifile.Day2Digit, DLG_DATA(IDI_HistNoDups, INI_CTL_CHECK, 0)

    // Dummy item to hold name of primary INI file, set up here so
    // INIStr will move it around as needed, but with blank name so user
    // can't modify it
    " ", INI_STR, 0, VNULL, &gaInifile.PrimaryININame, DLG_NULL

    // Key mapping items (same in all products)
    "AddFile", INI_KEY, (K_F10 + D_MAP_EDIT), VNULL, NULL, DLG_NULL
    "AliasExpand", INI_KEY, (K_CtlF + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "Backspace", INI_KEY, (K_Bksp + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "BeginLine", INI_KEY, (K_Home + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "CommandEscape", INI_KEY, (K_255 + D_MAP_EDIT), VNULL, NULL, DLG_NULL
    "Del", INI_KEY, (K_Del + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "DelHistory", INI_KEY, (K_CtlD + D_MAP_EDIT), VNULL, NULL, DLG_NULL
    "DelToBeginning", INI_KEY, (K_CtlHm + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "DelToEnd", INI_KEY, (K_CtlEnd + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "DelWordLeft", INI_KEY, (K_CtlL + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "DelWordRight", INI_KEY, (K_CtlR + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "Down", INI_KEY, (K_Down + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "EndHistory", INI_KEY, (K_CtlE + D_MAP_EDIT), VNULL, NULL, DLG_NULL
    "EndLine", INI_KEY, (K_End + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "EraseLine", INI_KEY, (K_Esc + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "ExecLine", INI_KEY, (K_Enter + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "Help", INI_KEY, (K_F1 + D_MAP_EDIT), VNULL, NULL, DLG_NULL
    "PopupWinBegin", INI_KEY, (K_CtlPgU + D_MAP_HWIN), VNULL, NULL, DLG_NULL
    "PopupWinDel", INI_KEY, (K_CtlD + D_MAP_HWIN), VNULL, NULL, DLG_NULL
    "PopupWinEdit", INI_KEY, (K_CtlEnt + D_MAP_HWIN), VNULL, NULL, DLG_NULL
    "PopupWinEnd", INI_KEY, (K_CtlPgD + D_MAP_HWIN), VNULL, NULL, DLG_NULL
    "PopupWinExec", INI_KEY, (K_Enter + D_MAP_HWIN), VNULL, NULL, DLG_NULL
    "Ins", INI_KEY, (K_Ins + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "Left", INI_KEY, (K_Left + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "LFNToggle", INI_KEY, (K_CtlA + D_MAP_EDIT), VNULL, NULL, DLG_NULL
    "LineToEnd", INI_KEY, (K_CtlEnt + D_MAP_EDIT), VNULL, NULL, DLG_NULL
    "ListContinue", INI_KEY, (K_C + D_MAP_LIST), VNULL, NULL, DLG_NULL
    "ListExit", INI_KEY, (K_Esc + D_MAP_LIST), VNULL, NULL, DLG_NULL
    "ListFind", INI_KEY, (K_F + D_MAP_LIST), VNULL, NULL, DLG_NULL
    "ListFindReverse", INI_KEY, (K_CtlF + D_MAP_LIST), VNULL, NULL, DLG_NULL
    "ListHex", INI_KEY, (K_X + D_MAP_LIST), VNULL, NULL, DLG_NULL
    "ListHighBit", INI_KEY, (K_H + D_MAP_LIST), VNULL, NULL, DLG_NULL
    "ListInfo", INI_KEY, (K_I + D_MAP_LIST), VNULL, NULL, DLG_NULL
    "ListNext", INI_KEY, (K_N + D_MAP_LIST), VNULL, NULL, DLG_NULL
    "ListPrevious", INI_KEY, (K_CtlN + D_MAP_LIST), VNULL, NULL, DLG_NULL
    "ListPrint", INI_KEY, (K_P + D_MAP_LIST), VNULL, NULL, DLG_NULL
    "ListWrap", INI_KEY, (K_W + D_MAP_LIST), VNULL, NULL, DLG_NULL
    "NextFile", INI_KEY, (K_F9 + D_MAP_EDIT), VNULL, NULL, DLG_NULL
    "NormalEditKey", INI_KEY, (D_NORM_KEY + D_MAP_EDIT), VNULL, NULL, DLG_NULL
    "NormalHWinKey", INI_KEY, (D_NORM_KEY + D_MAP_HWIN), VNULL, NULL, DLG_NULL
    "NormalKey", INI_KEY, (D_NORM_KEY + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "NormalListKey", INI_KEY, (D_NORM_KEY + D_MAP_LIST), VNULL, NULL, DLG_NULL
    "NormalPopupKey", INI_KEY, (D_NORM_KEY + D_MAP_HWIN), VNULL, NULL, DLG_NULL
    "PopFile", INI_KEY, (K_F7 + D_MAP_EDIT), VNULL, NULL, DLG_NULL
    "PrevFile", INI_KEY, (K_F8 + D_MAP_EDIT), VNULL, NULL, DLG_NULL
    "Right", INI_KEY, (K_Right + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "SaveHistory", INI_KEY, (K_CtlK + D_MAP_EDIT), VNULL, NULL, DLG_NULL
    "Up", INI_KEY, (K_Up + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "WordLeft", INI_KEY, (K_CtlLft + D_MAP_GEN), VNULL, NULL, DLG_NULL
    "WordRight", INI_KEY, (K_CtlRt + D_MAP_GEN), VNULL, NULL, DLG_NULL

    "DirWinOpen", INI_KEY, (K_CtlPgU + D_MAP_EDIT), VNULL, NULL, DLG_NULL
    "HistWinOpen", INI_KEY, (K_PgUp + D_MAP_EDIT), VNULL, NULL, DLG_NULL
    "NextHistory", INI_KEY, (K_Down + D_MAP_EDIT), VNULL, NULL, DLG_NULL
    "PrevHistory", INI_KEY, (K_Up + D_MAP_EDIT), VNULL, NULL, DLG_NULL
};

TOKEN_LIST(INIItems, gaINIItemList);

unsigned int guINIItemCount = (sizeof(gaINIItemList) / sizeof(gaINIItemList[0]));

#else

extern INI_ITEM gaINIItemList[];
extern unsigned int guINIItemCount;

#endif


// INI file parsing errors
#define E_SECNAM  "Invalid section name"
#define E_BADNAM  "Invalid item name"
#define E_BADNUM  "Invalid numeric value for"
#define E_BADCHR  "Invalid character value for"
#define E_BADCHC  "Invalid choice value for"
#define E_BADKEY  "Invalid key substitution for (key not used by 4OS2)"
#define E_KEYFUL  "Keystroke substitution table full"
#define E_BADCOL  "Invalid color for"
#define E_BADPTH  "Invalid path or file name for"
#define E_STROVR  "String area overflow"
#define E_INCL    "Include file not found"
#define E_NEST    "Include files nested too deep"
#define E_LIVEMOD "Value can only be changed at startup"

#define CONTEXT_BITS(keyval) (keyval & 0xFF00 & (~(D_EXTKEY | D_NORM_KEY)))


#ifdef KEYPARSE

// Data for key parsing (KEYPARSE.C)

// key prefixes with function key base scan codes
struct {
    char *prefixstr;
    unsigned char F1Pref;      // scan code base for F1 - F10
    unsigned char F11Pref;     // scan code base for F11 - F12, minus 10
} KeyPrefixList[] = {
    "@", 0x3B, (0x85 - 10),       // no prefix ("@" is checked before prefix),
                                 // F1, F11
    "Alt", 0x68, (0x8B - 10),     // Alt-F1, Alt-F11
    "Ctrl", 0x5E, (0x89 - 10),    // Ctrl-F1, Ctrl-F11
    "Shift", 0x54, (0x87 - 10),   // Shift-F1, Shift-F11
};
TOKEN_LIST(KeyPrefixes, KeyPrefixList);

// names and codes for non-printing keys
struct {
    char *namestr;
    unsigned int NPStd;      // standard ASCII or scan code
    unsigned int NPSecond;   // secondary ASCII or scan code
} KeyNameList[] = {           // second key:
    "Esc", K_Esc, 0,              // none
    "Bksp", K_Bksp, K_CtlBS,      // Ctrl-Bksp
    "Tab", K_Tab, K_ShfTab,       // Shift-Tab
    "Enter", K_Enter, K_CtlEnt,   // Ctrl-Enter
    "Up", K_Up, K_CtlUp,          // Ctrl-Up
    "Down", K_Down, K_CtlDn,      // Ctrl-Down
    "Left", K_Left, K_CtlLft,     // Ctrl-Left
    "Right", K_Right, K_CtlRt,    // Ctrl-Right
    "PgUp", K_PgUp, K_CtlPgU,     // Ctrl-PgUp
    "PgDn", K_PgDn, K_CtlPgD,     // Ctrl-PgDn
    "Home", K_Home, K_CtlHm,      // Ctrl-Home
    "End", K_End, K_CtlEnd,       // Ctrl-End
    "Ins", K_Ins, K_CtlIns,       // Ctrl-Ins
    "Del", K_Del, K_CtlDel,       // Ctrl-Del
};
TOKEN_LIST(KeyNames, KeyNameList);

static char ALT_ALPHA_KEYS[] = {
   K_AltA - K_SCAN,
   K_AltB - K_SCAN,
   K_AltC - K_SCAN,
   K_AltD - K_SCAN,
   K_AltE - K_SCAN,
   K_AltF - K_SCAN,
   K_AltG - K_SCAN,
   K_AltH - K_SCAN,
   K_AltI - K_SCAN,
   K_AltJ - K_SCAN,
   K_AltK - K_SCAN,
   K_AltL - K_SCAN,
   K_AltM - K_SCAN,
   K_AltN - K_SCAN,
   K_AltO - K_SCAN,
   K_AltP - K_SCAN,
   K_AltQ - K_SCAN,
   K_AltR - K_SCAN,
   K_AltS - K_SCAN,
   K_AltT - K_SCAN,
   K_AltU - K_SCAN,
   K_AltV - K_SCAN,
   K_AltW - K_SCAN,
   K_AltX - K_SCAN,
   K_AltY - K_SCAN,
   K_AltZ - K_SCAN
};

static char ALT_DIGIT_KEYS[] = {
   K_Alt0 - K_SCAN,
   K_Alt1 - K_SCAN,
   K_Alt2 - K_SCAN,
   K_Alt3 - K_SCAN,
   K_Alt4 - K_SCAN,
   K_Alt5 - K_SCAN,
   K_Alt6 - K_SCAN,
   K_Alt7 - K_SCAN,
   K_Alt8 - K_SCAN,
   K_Alt9 - K_SCAN
};

#endif
