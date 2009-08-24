// 4OS2.H - Include file for 4OS2/32
//   Copyright (c) 1990-1997  Rex C. Conn   All rights reserved

#include "resource.h"

#define INCL_REXXSAA
#ifdef _lint
#define _Packed
#endif
#include <rexxsaa.h>            // REXX call header

#define _sopen sopen
#define _lseek lseek
#define _read read
#define _write write
#define _close close
#define _isatty isatty
#define _dup dup
#define _dup2 dup2
#define _kbhit kbhit
#define _setmode setmode
#define _strdup strdup
#define _strlwr strlwr
#define _strupr strupr
#define _stricmp stricmp
#define _strnicmp strnicmp
#define _chsize chsize
#define _chdir chdir
#define _mkdir mkdir
#define _rmdir rmdir

#define ReadCellStr(a,b,c,d)    VioReadCellStr(a,&b,c,d,0)
#define WriteCellStr(a,b,c,d)   VioWrtCellStr(a,b,c,d,0)

#define ARGMAX          255
#define MAXARGSIZ       511
#define MAXFILENAME     260
#define MAXPATH         255
#define MAXREDIR        10
#define MAXLINESIZ      1024            // maximum size of input line
#define CMDBUFSIZ       2048            // size of command input buffer

#define TABSIZE         gpIniptr->Tabs

#define MIN_STACK       3072

#define NOTSPECIFIED 0                  // START session type flags
#define NOTWINDOWCOMPAT 1
#define WINDOWCOMPAT 2
#define WINDOWAPI 3
#define DOSVDM_FS 4
#define DOSVDM_WIN 7

#define INVALID_HANDLE_VALUE -1

// disable intrinsic functions
#undef memchr
#undef memcmp
#undef memcpy
#undef memmove
#undef memset
#undef strcat
#undef strchr
#undef strcmp
#undef strcpy
#undef strlen
#undef strncat
#undef strncmp
#undef strncpy
#undef strrchr


#define ENVIRONMENT_SIZE 4096
#define ALIAS_SIZE ENVIRONMENT_SIZE

// Local Info Seg
typedef struct _LINFOSEG
{
        USHORT  pidCurrent;     // current process id
        USHORT  pidParent;      // process id of parent
        USHORT  prtyCurrent;    // priority of current thread
        USHORT  tidCurrent;     // thread ID of current thread
        USHORT  sgCurrent;      // session
        UCHAR   rfProcStatus;   // process status
        UCHAR   dummy1;
        USHORT  fForeground;    // current process has keyboard focus
        UCHAR   typeProcess;    // process type
        UCHAR   dummy2;
        SEL     selEnvironment; // environment selector
        USHORT  offCmdLine;     // command line offset
        USHORT  cbDataSegment;  // length of data segment
        USHORT  cbStack;        // stack size
        USHORT  cbHeap;         // heap size
        HMODULE hmod;           // module handle of the application
        SEL     selDS;          // data segment handle of the application
} LINFOSEG;

typedef LINFOSEG FAR *PLINFOSEG;


// Process Type codes (local info seg typeProcess field)
#define PT_FULLSCREEN       0       /* Full screen app.          */
#define PT_REALMODE         1       /* Real mode process         */
#define PT_WINDOWABLEVIO    2       /* VIO windowable app.       */
#define PT_PM               3       /* Presentation Manager app. */
#define PT_DETACHED         4       /* Detached app.             */

// Process Status Flag definitions (local info seg rfProcStatus field)
#define PS_EXITLIST         1       /* Thread is in exitlist routine */


APIRET16 APIENTRY16 DosGetInfoSeg(PSEL, PSEL);

typedef APIRET16 (APIENTRY16 * WSTAI)(PSZ,PSZ);         // WinSetTitleAndIcon

typedef HAB (APIENTRY WINIT)(ULONG);                    // WinInitialize
typedef BOOL (APIENTRY WTERM)(HAB);
typedef BOOL (APIENTRY WSAW)(HWND, HWND);
typedef ERRORID (APIENTRY  WGLE)(HAB);

typedef ULONG (APIENTRY WQSL)(HAB, PSWBLOCK, ULONG);    // WinQuerySwitchEntry
typedef ULONG (APIENTRY WQSE)(HSWITCH, PSWCNTRL);       // WinQuerySwitchEntry
typedef HSWITCH (APIENTRY WQSH)(HWND, PID);             // WinQuerySwitchHandle
typedef ULONG (APIENTRY WCSE)(HSWITCH, PSWCNTRL);       // WinChangeSwitchEntry
// WinStartApp
typedef HAPP (APIENTRY WSA)(HWND, PPROGDETAILS, PSZ, PVOID, ULONG);
typedef ULONG (APIENTRY WQWP)(HWND, PSWP);              // WinQueryWindowPos
// WinSetWindowPos
typedef ULONG (APIENTRY WSWP)(HWND, HWND, LONG, LONG, LONG, LONG, ULONG);
typedef ULONG (APIENTRY WQSV)(HWND, LONG);              // WinQuerySysValue
typedef ULONG (APIENTRY WPM)(HWND, ULONG, MPARAM, MPARAM);      // WinPostMsg
typedef ULONG (APIENTRY WSWT)(HWND, PSZ);               // WinSetWindowText
typedef ULONG (APIENTRY WSKST)(HWND, PBYTE, BOOL);      // WinSetKeyboardStateTable

typedef BOOL  (APIENTRY WSCD)(HAB, ULONG, ULONG, ULONG );
typedef ULONG (APIENTRY WQCD)(HAB, ULONG);
typedef BOOL  (APIENTRY WQCFI)(HAB, ULONG, PULONG );
typedef BOOL  (APIENTRY WEC)(HAB);
typedef BOOL  (APIENTRY WOC)(HAB);
typedef BOOL  (APIENTRY WCC)(HAB);

// Profile calls
typedef HINI    (APIENTRY POP)(HAB, PCSZ);
typedef BOOL    (APIENTRY PQPD)(HINI, PCSZ, PCSZ, PVOID, PULONG);
typedef BOOL    (APIENTRY PWPD)(HINI, PCSZ, PCSZ, PVOID, ULONG);
typedef BOOL    (APIENTRY PCP)(HINI);


// SendKeys (in JPOS2DLL.C)
typedef INT (APIENTRY SKEYS)( PSZ );
typedef VOID (APIENTRY QUITSKEYS)( VOID );

// Helper macros used to convert selector to LINFOSEG
#define MAKEPLINFOSEG(sel)  ((PLINFOSEG)MAKEP(sel, 0))


// Info to pass to child pipe process
typedef struct {
        int  fInitialized;
        char CriticalVars[sizeof(CRITICAL_VARS)];
        char BatchFrame[sizeof(BATCHFRAME) * MAXBATCH];
        char CommandLine[MAXLINESIZ+MAXFILENAME+2];
} PIPE_INHERIT;


// redirection flags for STDIN, STDOUT, STDERR, and active pipes
typedef struct
{
        int std[MAXREDIR];
        int fClip[MAXREDIR];
        int pipe_open;
        int nChildPipe;         // process handle to child
        int fFocus;
        unsigned long lPreviousPipe;
        PIPE_INHERIT *pPipeSource;
} REDIR_IO;

/*
typedef double LONGLONG;

// Disk info (free space, total space, and cluster size)
typedef struct
{
        LONGLONG BytesFree;
        LONGLONG BytesTotal;
        long ClusterSize;
} QDISKINFO;
*/

// OS/2 doesn't have unique handles built into the DIR structure, so we make
//  one of our own
typedef struct
{
        ULONG   oNextEntryOffset;
        union {
                USHORT  fdCreation;
                FDATE   fdateCreation;
        } fdC;
        union {
                USHORT  ftCreation;
                FTIME   ftimeCreation;
        } ftC;
        union {
                USHORT  fdLAccess;
                FDATE   fdateLastAccess;
        } fdLA;
        union {
                USHORT  ftLAccess;
                FTIME   ftimeLastAccess;
        } ftLA;
        union {
                USHORT  fdLWrite;
                FDATE   fdateLastWrite;
        } fdLW;
        union {
                USHORT  ftLWrite;
                FTIME   ftimeLastWrite;
        } ftLW;
        ULONG   size;
        ULONG   cbFileAlloc;
        ULONG   attrib;
        ULONG   cbList;
        UCHAR   cchName;
        CHAR    name[CCHMAXPATHCOMP];
        char    dummy[3];       // kludge for IFS overwrites of "hdir"
        HDIR    hdir;           // search handle
        USHORT  fHPFS;          // HPFS or NTFS drive flag
        RANGES  aRanges;        // date / time / size ranges
} FILESEARCH;


// structure of new OS/2 (& Windows) .EXE header
typedef struct
{
        USHORT magic;           // magic number NE
        UCHAR ver;              // version number
        UCHAR rev;              // revision number
        USHORT enttab;          // offset of Entry Table
        USHORT cbenttab;        // number of bytes in Entry Table
        ULONG crc;              // CRC checksum of file
        USHORT flags;           // flag word
        USHORT autodata;        // automatic data segment number
        USHORT heap;            // initial heap allocation
        USHORT stack;           // initial stack allocation
        ULONG csip;             // initial CS:IP
        ULONG sssp;             // initial SS:SP
        USHORT cseg;            // count of file segments
        USHORT cmod;            // entries in Module Reference Table
        USHORT cbnrestab;       // size of non-resident name table
        USHORT segtab;          // offset of Segment Table
        USHORT rsrctab;         // offset of Resource Table
        USHORT restab;          // offset of resident name table
        USHORT modtab;          // offset of Module Reference Table
        USHORT imptab;          // offset of Imported Names Table
        ULONG nrestab;          // offset of Non-resident Names Table
        USHORT cmovent;         // count of movable entries
        USHORT align;           // segment alignment shift count
        USHORT cres;            // count of resource entries
        UCHAR exetype;          // target operating system
        UCHAR reserved[9];      // pad it to 64 bytes
} NEW_EXE;


// APM info
typedef struct
{
        USHORT ParmLength;
        USHORT PowerFlags;
        UCHAR  ACStatus;
        UCHAR  BatteryStatus;
        UCHAR  BatteryLife;
} PARMPACK;


// Notebook paramaters and data structures
#define WM_OPEN_NOTEBOOK WM_USER + 0x101
#define WM_CLOSE_NOTEBOOK WM_USER + 0x102
#define MP0 MPFROMSHORT(0)
#define TAB_WIDTH_MARGIN 10   // Padding for the width of a notebook tab
#define TAB_HEIGHT_MARGIN 6    // Padding for the height of a notebook tab
#define DEFAULT_NB_TAB_HEIGHT 16   // Default if Gpi calls fail

typedef struct _NBPAGE                          // VARIABLES USED FOR A NOTEBOOK PAGE
{
        PFNWP           pfnwpDlg;                               // Window procedure address for the dialog
        PSZ             pszStatusLineText;      // Text to go on status line
        PSZ             pszTabText;                             // Text to go on major tab
        ULONG           idDlg;                                  // ID of the dialog box for this page
        HWND            hwndPageDlg;                    // Page dialog handle
        ULONG           idFirst;                                        // ID of the first control on this page*
        ULONG           idLast;                                 // ID of the last control on this page*
        ULONG           idFocus;                                        // ID of the control to get the focus first
        BOOL            fParent;                                        // Is this a Parent page with minor pages
        USHORT  usTabType;                              // BKA_MAJOR or BKA_MINOR
        PSZ             pszHelpName;                    // Name of related help topic (with quotes)

        // *These items only required if used by dialog procedure (*pfnwpDlg)

} NBPAGE, *PNBPAGE;

typedef struct _NBHDR                           // VARIABLES USED FOR A NOTEBOOK
{
        ULONG           ulNotebookID;                   // ID of notebook control in upper-level dialog
        int             nPages;                                 // Page count
        PNBPAGE PageArray;                              // Pointer to first entry in NBPAGE array
        HWND            hwndNB;                                 // Handle of notebook control
        HWND            hwndNBDlg;                              // Handle of upper-level dialog

} NBHDR, *PNBHDR;

