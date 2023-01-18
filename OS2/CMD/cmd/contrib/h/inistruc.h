// INISTRUC.H - INI file data structure for 4xxx / TCMD products
//   Copyright 1997  JP Software Inc.  All rights reserved


// INI file constants
#define INI_DEFKEYS 4
#define INI_SIG 0x4DD4
#define INI_STRMAX 4096
#define INI_KEYMAX 64
#define INI_KEYS_SIZE (2 * sizeof(UINT) * INI_KEYMAX)
#define INI_PRODUCT 0x80
#define INI_PRIMARY 0x40
#define INI_SECONDARY 0x20
#define INI_TOTAL_BYTES (sizeof (INIFILE) + INI_STRMAX + INI_KEYS_SIZE)
#define INI_EMPTYSTR ((unsigned int)-1)

#define OPTBITS 384
#define OBSIZE  ((OPTBITS + 7) / 8)

// structures and macros for token lists
typedef struct {
    int num_entries;
    int entry_size;
    char **elist;
} TL_HEADER;

#define TOKEN_LIST(tl_name, tlist) TL_HEADER tl_name = { (sizeof(tlist) / sizeof(tlist[0])), sizeof(tlist[0]), (char **)&tlist }


// INI file data structure
// This structure is maintained in the following order:
//    Header
//    INI file data for:
//       All products
//       Character mode products
//       GUI products
//       Character/GUI product crossovers
//
// Within each section of INI file data, there is a further subdivision:
//    String pointers
//    Integers
//    Choices (2-valued)
//    Characters / bytes (includes 3+ valued choices)
//
// Within each of those sections, there is a further division:
//    INI file data, alphabetically by directive name
//    Internal data, alphabetically by structure element name
//

typedef struct
{
    /*
    **********************************************************************
    ** Header (all products)
    **********************************************************************
    */
    char *StrData;          // (internal) pointer to strings data
    unsigned int StrMax;        // (internal) maximum string bytes
    unsigned int StrUsed;       // (internal) actual string bytes
    unsigned int *Keys;     // (internal) pointer to key list
    unsigned int KeyMax;        // (internal) maximum key count
    unsigned int KeyUsed;       // (internal) actual key count
    unsigned int SecFlag;       // (internal) INI file section bit flags
    unsigned int OBCount;       // (internal) OPTION bit flag count
    unsigned char OptBits[OBSIZE];  // (internal) OPTION bit flags

    /*
    **********************************************************************
    ** All Products
    **********************************************************************
    */
        // String pointers
    unsigned int DescriptName;  // alternative name for DESCRIPT.ION
    unsigned int DirColor;      // DirColors
    unsigned int FC;        // Filename completion
    unsigned int FSPath;        // 4StartPath / TCStartPath
    unsigned int HistLogName;   // HistoryLogName
    unsigned int LogName;       // LogName
    unsigned int Printer;       // Printer
    unsigned int TreePath;      // CD / CDD tree index file

        // Integers
    unsigned int AliasSize;     // Alias
    unsigned int AliasNew;      // Alias (new from OPTION, in 4DOS)
    unsigned int BeepDur;       // BeepLength
    unsigned int BeepFreq;      // BeepFreq
    unsigned int CDDHeight;     // CDDWinHeight
    unsigned int CDDLeft;       // CDDWinLeft
    unsigned int CDDTop;        // CDDWinTop
    unsigned int CDDWidth;      // CDDWinWidth
    int CDWithoutBackslash;     // change directory without backslash is allowed
    int CursI;          // CursorIns
    int CursO;          // CursorOver
    unsigned int DescriptMax;   // DescriptionMax
    unsigned int DirHistorySize;    // DirHistory
    unsigned int DirHistoryNew; // DirHistory (new from OPTION, in 4DOS)
    unsigned int EnvSize;       // Environment
    unsigned int EnvNew;            // Environment (new from OPTION, in 4DOS)
    unsigned int EvalMax;       // Maximum # of EVAL decimal places
    unsigned int EvalMin;       // Minimum # of EVAL decimal places
    unsigned int FuzzyCD;       // fuzzy CD completion style
    unsigned int HistMin;       // HistMin
    unsigned int HistorySize;   // History
    unsigned int HistoryNew;    // History (new from OPTION, in 4DOS)
    unsigned int InputColor;    // color used for command input
    unsigned int ListColor;     // ListColors
    unsigned int ListRowStart;
    int NewByteDisp;            // dir command - new byte display level
    unsigned int PWHeight;      // PopupWinHeight
    unsigned int PWLeft;        // PopupWinLeft
    int PWTop;                  // PopupWinTop
    int PopupPosRelative;       // PopupWinPositionIsRelative
    unsigned int PWWidth;       // PopupWinWidth
    unsigned int SelectColor;   // SelectColors
    unsigned int StdColor;      // StdColors
    unsigned int Tabs;      // tabstops (for LIST)
    int TitleIsCurDir;          // window title is current directory

    unsigned int INIDebug;      // (internal) debug bit flags
    int ShellLevel;         // (internal) -1 if /C, 0 if /P, 1 otherwise
    int ShellNum;           // (internal) shell number

        // Choices
    unsigned char AppendDir;    // Append backslash to directory on tab
    unsigned char BatEcho;      // BatchEcho
    unsigned char ChangeTitle;  // ChangeTitle
    unsigned char DecimalChar;  // decimal character for @EVAL
    unsigned char Descriptions; // Descriptions
    unsigned char EditMode;     // EditMode
    unsigned char HistoryCopy;  // HistCopy
    unsigned char HistoryMove;  // HistMove
    unsigned char HistoryWrap;  // HistWrap
    unsigned char HistNoDups;    // HistNoDups
    unsigned char INIQuery;     // INIQuery
    unsigned char NoClobber;    // NoClobber
    unsigned char PathExt;      // PathExt
    unsigned char PauseErr;     // PauseOnError
    unsigned char ThousandsChar;    // decimal character for @EVAL
    unsigned char TimeFmt;      // AmPm
    unsigned char UnixPaths;    // UnixPaths
    unsigned char Upper;        // UpperCase
    unsigned char Year4Digit;   // Show year as 4 digita
    unsigned char Day2Digit;    // Show all days as 2 digits

        // Characters / bytes
    unsigned char CmdSep;       // CommandSep
    unsigned char EscChr;       // EscapeChar
    unsigned char ParamChr;     // ParameterChar

    unsigned char BootDrive;    // (internal) boot drive letter (upper case)
    unsigned char Expansion;    // (internal) alias/variable expansion flag
    unsigned char LogOn;        // (internal) command logging flag
    unsigned char HistLogOn;    // (internal) history logging flag
    unsigned char SingleStep;   // (internal) batch file single step flag
    unsigned char SwChr;        // (internal) SwitchChar


    /*
    **********************************************************************
    ** Character Mode Products
    **********************************************************************
    */
    // ---------------------------------
    // -- All Character Mode Products
    // ---------------------------------
        // Integers
    unsigned int CDDColor;      // CDDWinColor
    unsigned int LBBar;     // ListboxBarColors
    unsigned int ListStatusColor;   // ListStatusColors
    unsigned int PWColor;       // PopupWinColor
    unsigned int SelectStatusColor; // SelectStatusColors

        // Choices
    unsigned char BrightBG;     // BrightBG
    unsigned char LineIn;       // LineInput

    /*
    **********************************************************************
    ** Special
    **********************************************************************
    */

        // Integers
    unsigned int WindowHeight;  // height, in pels
    unsigned int WindowState;   // WindowState
    unsigned int WindowWidth;   // width, in pels
    unsigned int WindowX;       // top left, in pels
    unsigned int WindowY;

        // Choices
    unsigned char ExecWait;     // if !=0, wait for apps at cmd line

        // String pointers
    unsigned int NextININame;   // NextINIFile
    unsigned int PrimaryININame;    // (internal) primary INI file name
    unsigned int DateFmt;      // Internal date format
    unsigned int TmSmpFmt;      // Internal date format

    // Choices
    unsigned char DupBugs;      // duplicate CMD.EXE bugs?
    unsigned char CMDDirMvCpy;   // use CMD's move and copy style for dirictories
    unsigned char LocalAliases; // LocalAliases
    unsigned char LocalDirHistory;  // LocalDirHistory
    unsigned char LocalHistory; // LocalHistory

    unsigned char ANSI;     // ANSI escapes enabled/disabled

        // String pointers
    unsigned int HelpBook;      // HelpBook
    unsigned int SwapPath;      // path to swap file

        // Integers
    unsigned int Rows;      // ScreenRows

    /*
    **********************************************************************
    ** Trailer (all products)
    **********************************************************************
    */
    unsigned int INIBuild;      // (internal) current internal build number
    unsigned int INISig;        // (internal) INI file signature
} INIFILE;
