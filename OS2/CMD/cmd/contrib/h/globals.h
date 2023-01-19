// Global variables for 4xxx / TCMD family
//   Copyright 1992 - 1997 by Rex C. Conn

#ifdef DEFINE_GLOBALS

char AUTOSTART[] = "C:\\4START";
char AUTOEXIT[] = "C:\\4EXIT";

// "gszCmdline" is the input buffer for command line & batch file entry
// also used as temporary stack by server when a TSR terminates in an
//   unusual way, and by some of the internal commands
char gszCmdline[CMDBUFSIZ+16];
char *pszCmdLineOpts;
char gszOsVersion[6];           // holds DOS / OS2 version as string
char gszMyVersion[6];           // major and minor 4xxx version string
char *gpNthptr;                 // pointer to nth arg in line
char *gpInternalName;           // name of internal command being executed
char *gpBatchName;              // name of batch file to be executed
char gszSortSequence[16];       // directory sort order flags (DIR & SELECT)
char gszFindDesc[128];          // File description to search for
char gchInclusiveMode;          // flags for /A:[-]xxxx directory searches
char gchExclusiveMode;
char gchBlock = 'Û';            // chars for DOS/V / non-ASCII support
char gchRightArrow = 0x10;      // -> arrow for SELECT and DIR
char gchDimBlock = '°';
char gchVerticalBar = '³';
char gchUpArrow = 24;
char gchDownArrow = 25;

PCH glpEnvironment;             // pointer to environment (paragraph aligned)
PCH glpMasterEnvironment;       // pointer to master environment
PCH glpAliasList;               // pointer to alias list (paragraph aligned)
PCH glpHistoryList;             // pointer to base of history list
PCH glpHptr = 0L;               // pointer into history list
PCH glpDirHistory;              // pointer to directory history list

CRITICAL_VARS cv;               // vars that must be saved when doing an INT 2E

BATCHFRAME bframe[MAXBATCH];    // batch file control structure

COUNTRYINFO gaCountryInfo;      // international formatting info

TIMERS gaTimers[3];

INIFILE gaInifile;              // initialization data structure
INIFILE *gpIniptr = (INIFILE *)&gaInifile;      // pointer to inifile
char gaIniStrings[INI_STRMAX];

UINT gaIniKeys[(2 * INI_KEYMAX)] = {
    CTL_BS,                 // ^Bksp -> ^R
    TAB,                    // TAB -> F9
    SHIFT_TAB,              // Shift-Tab -> F8
    CTL_TAB,                // ^Tab -> F7
    18 + (MAP_GEN << 8),    // ^Bksp -> ^R, general keys
    F9 + (MAP_EDIT << 8),   // TAB -> F9, editing keys
    F8 + (MAP_EDIT << 8),   // Shift-Tab -> F8, editing keys
    F7 + (MAP_EDIT << 8),   // ^Tab -> F7, editing keys
};

int gnOsVersion;                // combined major & minor version
int gnTransient = 0;            // transient processor (/C) flag
int gnGFH = 0;                  // global file handle (for ^C trapping)
int gnCurrentDisk;              // current disk drive (A=1, B=2, etc.)
int gnErrorLevel = 0;           // return code for external commands
int gnInternalErrorLevel = 0;   // return code for internal commands
int gnSysError = 0;             // return code for last DOS/OS2/NT error
unsigned int gnPageLength;      // number of rows per page
unsigned long glDirFlags;       // DIR flags (VSORT, RECURSE, JUSTIFY, etc.)
int gnDirTimeField;             // creation, last access, or last write sort


char gszSessionTitle[128];      // window title
char *gpRexxCmdline;            // pointer to REXX arguments
char gchSysBootDrive;           // system boot drive

HWND ghwndWindowHandle = (HWND)NULL;    // Windowed frame handle for 4OS2 session
char gszQueueName[32];          // termination queue for DosStartSession

int gnOS2_Revision;             // OS/2 revision letter

HQUEUE ghQueueHandle;           // handle for termination queue
PLINFOSEG gpLIS;                // pointer to local info area

COUNTRYCODE gaCountryCode;

unsigned long gnChildProcess = 0;       // current child process ID
unsigned long gnChildPipeProcess = 0;   // current child pipe process ID
PCH _pgmptr;

WSTAI pfnWSTAI = 0L;            // WinSetTitleAndIcon
SKEYS *pfnSendKeys = 0L;        // SendKeys
QUITSKEYS *pfnQuitSendKeys = 0L; // QuitSendKeys
    #if _PM == 0
WINIT *pfnWINIT = 0L;           // WinInitialize
WTERM *pfnWTERM = 0L;           // WinTerminate
WCMQ *pfnWCMQ = 0L;             // WinCreateMsgQueue
WDMQ *pfnWDMQ = 0L;             // WinDestroyMsgQueue
WCS *pfnWCS = 0L;               // WinCancelShutdown
WSAW *pfnWSAW = 0L;             // WinSetActiveWindow
WGLE *pfnWGLE = 0L;             // WinGetLastError
WQSL *pfnWQSL = 0L;             // WinQuerySwitchList
WQSE *pfnWQSE = 0L;             // WinQuerySwitchEntry
WQSH *pfnWQSH = 0L;             // WinQuerySwitchHandle
WCSE *pfnWCSE = 0L;             // WinChangeSwitchEntry
WSA *pfnWSA = 0L;               // WinStartApp
WQWP *pfnWQWP = 0L;             // WinQueryWindowPos
WSWP *pfnWSWP = 0L;             // WinSetWindowPos
WQSV *pfnWQSV = 0L;             // WinQuerySysValue
WPM *pfnWPM = 0L;               // WinPostMessage
WSWT *pfnWSWT = 0L;             // WinSetWindowText
WSKST *pfnWSKST = 0L;           // WinSetKeyboardStateTable

WOC *pfnWOC = 0L;               // WinOpenClipbrd
WEC *pfnWEC = 0L;               // WinEmptyClipbrd
WSCD *pfnWSCD = 0L;             // WinSetClipbrdData
WCC *pfnWCC = 0L;               // WinCloseClipbrd
WQCFI *pfnWQCFI = 0L;           // WinQueryClipbrdFmtInfo
WQCD *pfnWQCD = 0L;             // WinQueryClipbrdData

POP *pfnPOP = 0L;               // PrfOpenProfile
PQPD *pfnPQPD = 0L;             // PrfQueryProfileData
PWPD *pfnPWPD = 0L;             // PrfWriteProfileData
PCP *pfnPCP = 0L;               // PrfCloseProfile
    #endif

// points to exception structure in "main()"
PEXCEPTIONREGISTRATIONRECORD pExceptionStruct;


#else           // DEFINE_GLOBALS

// external definitions

extern char AUTOSTART[];
extern char AUTOEXIT[];
extern char gszCmdline[];
extern char *pszCmdLineOpts;
extern char gszOsVersion[];
extern char *gpNthptr;
extern char *gpInternalName;
extern char *gpBatchName;
extern char gszSortSequence[];
extern char gszFindDesc[];
extern char gszMyVersion[];
extern char gchInclusiveMode;
extern char gchExclusiveMode;
extern char gchBlock;           // chars for DOS/V support
extern char gchDimBlock;
extern char gchRightArrow;
extern char gchVerticalBar;
extern char gchUpArrow;
extern char gchDownArrow;

extern PCH _pgmptr;
extern PCH glpEnvironment;
extern PCH glpMasterEnvironment;
extern PCH glpAliasList;
extern PCH glpHistoryList;
extern PCH glpHptr;
extern PCH glpDirHistory;

extern CRITICAL_VARS cv;

extern BATCHFRAME bframe[];

extern COUNTRYINFO gaCountryInfo;

extern BUILTIN commands[];

extern TIMERS gaTimers[];

extern char szHelpFileName[];

extern INIFILE gaInifile;
extern INIFILE *gpIniptr;
extern char gaIniStrings[];
extern UINT gaIniKeys[];

extern int gnOsVersion;
extern int gnTransient;
extern int gnGFH;
extern int gnCurrentDisk;
extern int gnErrorLevel;
extern int gnInternalErrorLevel;
extern int gnSysError;
extern int gnDirTimeField;
extern unsigned int gnPageLength;
extern unsigned long glDirFlags;

extern char gchSysBootDrive;
extern char gszSessionTitle[];
extern char *gpRexxCmdline;


extern HAB ghHAB;
extern HMQ ghHMQ;
extern HWND ghwndWindowHandle;
extern char gszQueueName[];

extern int gnOS2_Revision;

extern HQUEUE ghQueueHandle;
extern PLINFOSEG gpLIS;

extern COUNTRYCODE gaCountryCode;

extern unsigned long gnChildProcess;
extern unsigned long gnChildPipeProcess;

extern WSTAI pfnWSTAI;          // WinSetTitleAndIcon
extern SKEYS *pfnSendKeys;      // SendKeys
extern QUITSKEYS *pfnQuitSendKeys; // QuitSendKeys
extern WINIT *pfnWINIT;         // WinInitialize
extern WTERM *pfnWTERM;         // WinTerminate
extern WCMQ *pfnWCMQ;           // WinCreateMsgQueue
extern WDMQ *pfnWDMQ;           // WinDestroyMsgQueue
extern WCS *pfnWCS;             // WinCancelShutdown
extern WSAW *pfnWSAW;           // WinSetActiveWindow
extern WGLE *pfnWGLE;           // WinGetLastError
extern WQSL *pfnWQSL;           // WinQuerySwitchList
extern WQSE *pfnWQSE;           // WinQuerySwitchEntry
extern WQSH *pfnWQSH;           // WinQuerySwitchHandle
extern WCSE *pfnWCSE;           // WinChangeSwitchEntry
extern WSA *pfnWSA;             // WinStartApp
extern WQWP *pfnWQWP;           // WinQueryWindowPos
extern WSWP *pfnWSWP;           // WinSetWindowPos
extern WQSV *pfnWQSV;           // WinQuerySysValue
extern WPM *pfnWPM;             // WinPostMessage
extern WSKST *pfnWSKST;         // WinSetKeyboardStateTable
extern WSWT *pfnWSWT;           // WinSetWindowText

extern WOC *pfnWOC;             // WinOpenClipbrd
extern WEC *pfnWEC;             // WinEmptyClipbrd
extern WSCD *pfnWSCD;           // WinSetClipbrdData
extern WCC *pfnWCC;             // WinCloseClipbrd
extern WQCFI *pfnWQCFI;         // WinQueryClipbrdFmtInfo
extern WQCD *pfnWQCD;           // WinQueryClipbrdData

extern POP *pfnPOP;             // PrfOpenProfile
extern PQPD *pfnPQPD;           // PrfQueryProfileData
extern PWPD *pfnPWPD;           // PrfQueryProfileData
extern PCP *pfnPCP;             // PrfCloseProfile

extern PEXCEPTIONREGISTRATIONRECORD pExceptionStruct;
extern PEXCEPTIONREGISTRATIONRECORD pRexxExceptionStruct;

#endif          // DEFINE GLOBALS
