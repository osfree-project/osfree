/*  Win_Kernel.h	1.36
    Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

*/

#ifndef Win_Kernel__h
#define Win_Kernel__h

#ifndef RC_INVOKED
#include "setjmp.h"
#endif

/* Application's entry procedure */
#ifdef STRICT
int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
#endif


/* System Information **********************************************/
#ifndef NOAPIPROTO
DWORD   WINAPI GetVersion(void);
BOOL	WINAPI GetVersionEx(LPOSVERSIONINFO);

DWORD   WINAPI GetFreeSpace(UINT);
UINT    WINAPI GetCurrentPDB(void);

UINT    WINAPI GetWindowsDirectory(LPSTR, UINT);
UINT    WINAPI GetSystemDirectory(LPSTR, UINT);

UINT    WINAPI GetFreeSystemResources(UINT);

DWORD   WINAPI GetWinFlags(void);

LPSTR   WINAPI GetDOSEnvironment(void);

DWORD   WINAPI GetCurrentTime(void);
DWORD   WINAPI GetTickCount(void);
DWORD   WINAPI GetTimerResolution(void);

#endif      /* NOAPIPROTO */

/* GetFreeSystemResources flags */
#define GFSR_SYSTEMRESOURCES   0x0000
#define GFSR_GDIRESOURCES      0x0001
#define GFSR_USERRESOURCES     0x0002

/* Window Flags (GetWinFlags())*/
#define WF_PMODE    0x0001
#define WF_CPU286   0x0002
#define WF_CPU386   0x0004
#define WF_CPU486   0x0008
#define WF_STANDARD 0x0010
#define WF_WIN286   0x0010
#define WF_ENHANCED 0x0020
#define WF_WIN386   0x0020
#define WF_CPU086   0x0040
#define WF_CPU186   0x0080
#define WF_LARGEFRAME   0x0100
#define WF_SMALLFRAME   0x0200
#define WF_80x87    0x0400
#define WF_PAGING   0x0800
#define WF_WLO          0x8000


/* Error logging, debugging ****************************************/
#ifndef NOLOGERROR

/* Debugging support (DEBUG SYSTEM ONLY) */
typedef struct tagWINDEBUGINFO
{
    UINT    flags;
    DWORD   dwOptions;
    DWORD   dwFilter;
    char    achAllocModule[8];
    DWORD   dwAllocBreak;
    DWORD   dwAllocCount;
} WINDEBUGINFO;

#ifndef NOAPIPROTO
void    WINAPI LogError(UINT err, void FAR* lpInfo);
void    WINAPI LogParamError(UINT err, FARPROC lpfn, void FAR* param);

BOOL    WINAPI GetWinDebugInfo(WINDEBUGINFO FAR* lpwdi, UINT flags);
BOOL    WINAPI SetWinDebugInfo(const WINDEBUGINFO FAR* lpwdi);

void    FAR _cdecl DebugOutput(UINT flags, LPCSTR lpsz, ...);
#endif      /* NOAPIPROTO */


/* Error modifier bits from LogError() and LogParamError()*/
#define ERR_WARNING     0x8000
#define ERR_PARAM       0x4000

#define ERR_SIZE_MASK       0x3000
#define ERR_BYTE                0x1000
#define ERR_WORD                0x2000
#define ERR_DWORD               0x3000

/* Generic parameter values, from LogParamError() */
#define ERR_BAD_VALUE           0x6001
#define ERR_BAD_FLAGS           0x6002
#define ERR_BAD_INDEX           0x6003
#define ERR_BAD_DVALUE      0x7004
#define ERR_BAD_DFLAGS      0x7005
#define ERR_BAD_DINDEX      0x7006
#define ERR_BAD_PTR     0x7007
#define ERR_BAD_FUNC_PTR    0x7008
#define ERR_BAD_SELECTOR        0x6009
#define ERR_BAD_STRING_PTR  0x700a
#define ERR_BAD_HANDLE          0x600b

/* KERNEL parameter errors, from LogParamError() */
#define ERR_BAD_HINSTANCE       0x6020
#define ERR_BAD_HMODULE         0x6021
#define ERR_BAD_GLOBAL_HANDLE   0x6022
#define ERR_BAD_LOCAL_HANDLE    0x6023
#define ERR_BAD_ATOM            0x6024
#define ERR_BAD_HFILE           0x6025

/* USER parameter errors, from LogParamError() */
#define ERR_BAD_HWND            0x6040
#define ERR_BAD_HMENU           0x6041
#define ERR_BAD_HCURSOR         0x6042
#define ERR_BAD_HICON           0x6043
#define ERR_BAD_HDWP            0x6044
#define ERR_BAD_CID             0x6045
#define ERR_BAD_HDRVR           0x6046

/* GDI parameter errors, from LogParamError() */
#define ERR_BAD_COORDS      0x7060
#define ERR_BAD_GDI_OBJECT      0x6061
#define ERR_BAD_HDC             0x6062
#define ERR_BAD_HPEN            0x6063
#define ERR_BAD_HFONT           0x6064
#define ERR_BAD_HBRUSH          0x6065
#define ERR_BAD_HBITMAP         0x6066
#define ERR_BAD_HRGN            0x6067
#define ERR_BAD_HPALETTE        0x6068
#define ERR_BAD_HMETAFILE       0x6069


/* KERNEL errors, from LogError() */
#define ERR_GALLOC              0x0001
#define ERR_GREALLOC            0x0002
#define ERR_GLOCK               0x0003
#define ERR_LALLOC              0x0004
#define ERR_LREALLOC            0x0005
#define ERR_LLOCK               0x0006
#define ERR_ALLOCRES            0x0007
#define ERR_LOCKRES             0x0008
#define ERR_LOADMODULE          0x0009

/* USER errors, from LogError() */
#define ERR_CREATEDLG           0x0040
#define ERR_CREATEDLG2          0x0041
#define ERR_REGISTERCLASS       0x0042
#define ERR_DCBUSY              0x0043
#define ERR_CREATEWND           0x0044
#define ERR_STRUCEXTRA          0x0045
#define ERR_LOADSTR             0x0046
#define ERR_LOADMENU            0x0047
#define ERR_NESTEDBEGINPAINT    0x0048
#define ERR_BADINDEX            0x0049
#define ERR_CREATEMENU          0x004a

/* GDI errors, from LogError() */
#define ERR_CREATEDC            0x0080
#define ERR_CREATEMETA          0x0081
#define ERR_DELOBJSELECTED      0x0082
#define ERR_SELBITMAP           0x0083


/* WINDEBUGINFO flags values */
#define WDI_OPTIONS         0x0001
#define WDI_FILTER          0x0002
#define WDI_ALLOCBREAK      0x0004

/* dwOptions values */
#define DBO_CHECKHEAP       0x0001
#define DBO_BUFFERFILL      0x0004
#define DBO_DISABLEGPTRAPPING 0x0010
#define DBO_CHECKFREE       0x0020

#define DBO_SILENT          0x8000

#define DBO_TRACEBREAK      0x2000
#define DBO_WARNINGBREAK    0x1000
#define DBO_NOERRORBREAK    0x0800
#define DBO_NOFATALBREAK    0x0400
#define DBO_INT3BREAK       0x0100

/* DebugOutput flags values */
#define DBF_TRACE           0x0000
#define DBF_WARNING         0x4000
#define DBF_ERROR           0x8000
#define DBF_FATAL           0xc000

/* dwFilter values */
#define DBF_KERNEL          0x1000
#define DBF_KRN_MEMMAN      0x0001
#define DBF_KRN_LOADMODULE  0x0002
#define DBF_KRN_SEGMENTLOAD 0x0004
#define DBF_USER            0x0800
#define DBF_GDI             0x0400
#define DBF_MMSYSTEM        0x0040
#define DBF_PENWIN          0x0020
#define DBF_APPLICATION     0x0008
#define DBF_DRIVER          0x0010

#endif      /* NOLOGERROR */

BOOL    WINAPI ExitWindows(DWORD dwReturnCode, UINT wReserved);

/* Error handling **************************************************/
#ifndef NOAPIPROTO

void    WINAPI FatalExit(int);
void    WINAPI FatalAppExit(UINT, LPCSTR);

BOOL    WINAPI ExitWindowsExec(LPCSTR, LPCSTR);

void    WINAPI DebugBreak(void);
void    WINAPI OutputDebugString(LPCSTR);

UINT    WINAPI SetErrorMode(UINT);

#endif      /* NOAPIPROTO */


#define EW_RESTARTWINDOWS 0x42
#define EW_REBOOTSYSTEM   0x43

/* Flags for SetErrorMode() */
#define SEM_FAILCRITICALERRORS  0x0001
#define SEM_NOGPFAULTERRORBOX   0x0002
#define SEM_NOOPENFILEERRORBOX  0x8000


/* Pointer validation **********************************************/
#ifndef NOAPIPROTO

BOOL    WINAPI IsBadReadPtr(const void FAR* lp, UINT cb);
BOOL    WINAPI IsBadWritePtr(void FAR* lp, UINT cb);
BOOL    WINAPI IsBadHugeReadPtr(const void _huge* lp, DWORD cb);
BOOL    WINAPI IsBadHugeWritePtr(void _huge* lp, DWORD cb);
BOOL    WINAPI IsBadCodePtr(FARPROC lpfn);
BOOL    WINAPI IsBadStringPtr(const void FAR* lpsz, UINT cchMax);

#endif      /* NOAPIPROTO */


/* Profiling support ***********************************************/
#ifndef NOPROFILER
#ifndef NOAPIPROTO

int     WINAPI ProfInsChk(void);
void    WINAPI ProfSetup(int,int);
void    WINAPI ProfSampRate(int,int);
void    WINAPI ProfStart(void);
void    WINAPI ProfStop(void);
void    WINAPI ProfClear(void);
void    WINAPI ProfFlush(void);
void    WINAPI ProfFinish(void);

#endif      /* NOAPIPROTO */
#endif      /* NOPROFILER */


/* Stack management ************************************************/
typedef int CATCHBUF[9];
typedef int FAR* LPCATCHBUF;

int     TWIN_Catch(LPBYTE, int);


#ifdef WINMALLOC_CHECK

LPVOID	WinMallocCheck(unsigned int, char *,int,int,int);
LPVOID	WinReallocCheck(LPVOID,unsigned int, char *,int,int,int);
void	WinFreeCheck(LPVOID,char *,int);

#define WinMalloc(s) 	WinMallocCheck(s,__FILE__,__LINE__,0,0)
#define WinFree(l)	WinFreeCheck(l,__FILE__,__LINE__)
#define WinRealloc(l,s)	WinReallocCheck(l,s,__FILE__,__LINE__,0,0)

#define WINMALLOC(s,t,h)    WinMallocCheck(s,__FILE__,__LINE__,t,h)
#define WINFREE(l) 	    WinFreeCheck(l,__FILE__,__LINE__)
#define WINREALLOC(l,s,t,h) WinReallocCheck(l,s,__FILE__,__LINE__,t,h)
#else

LPVOID WinMalloc(unsigned int);
void  WinFree(LPVOID);
LPVOID WinRealloc(LPVOID,unsigned int);

#define WINMALLOC(s,t,h)    WinMalloc(s)
#define WINFREE(l) 	    WinFree(l)
#define WINREALLOC(l,s,t,h) WinRealloc(l,s)
#endif

LPSTR WinStrdup(LPCSTR);

#if defined(Linux)
#define TWIN_jmp_buf __jmp_buf
#else
#define TWIN_jmp_buf jmp_buf
#endif
#define Catch(c) TWIN_Catch((LPBYTE)(c), \
			    setjmp((*(TWIN_jmp_buf *)((void *)(c = \
				   (int *)WinMalloc(sizeof(TWIN_jmp_buf)))))))

#ifndef NOAPIPROTO

void    WINAPI Throw(const int FAR*, int);
void    WINAPI SwitchStackBack(void);
void    WINAPI SwitchStackTo(UINT, UINT, UINT);

#endif      /* NOAPIPROTO */


/* Module management ***********************************************/
#define HINSTANCE_ERROR ((HINSTANCE)32)

typedef struct tagSEGINFO
{
    UINT offSegment;
    UINT cbSegment;
    UINT flags;
    UINT cbAlloc;
    HGLOBAL h;
    UINT alignShift;
    UINT reserved[2];
} SEGINFO;
typedef SEGINFO FAR* LPSEGINFO;


#ifndef NOAPIPROTO

HINSTANCE   WINAPI LoadModule(LPCSTR, LPVOID);
BOOL        WINAPI FreeModule(HINSTANCE);

HINSTANCE   WINAPI LoadLibrary(LPCSTR);
void        WINAPI FreeLibrary(HINSTANCE);

UINT        WINAPI WinExec(LPCSTR, UINT);

HMODULE     WINAPI GetModuleHandle(LPCSTR);

int         WINAPI GetModuleUsage(HINSTANCE);
#ifdef TWIN32
DWORD         WINAPI GetModuleFileName(HMODULE, LPTSTR, DWORD);
#else
int         WINAPI GetModuleFileName(HINSTANCE, LPSTR, int);
#endif

FARPROC     WINAPI GetProcAddress(HINSTANCE, LPCSTR);

int         WINAPI GetInstanceData(HINSTANCE, BYTE*, int);

HGLOBAL     WINAPI GetCodeHandle(FARPROC);

void    WINAPI GetCodeInfo(FARPROC lpProc, SEGINFO FAR* lpSegInfo);

FARPROC WINAPI MakeProcInstance(FARPROC, HINSTANCE);
void    WINAPI FreeProcInstance(FARPROC);

LONG    WINAPI SetSwapAreaSize(UINT);
void    WINAPI SwapRecording(UINT);
void    WINAPI ValidateCodeSegments(void);

#endif      /* NOAPIPROTO */

/* Windows Exit Procedure flag values */
#define WEP_SYSTEM_EXIT 1
#define WEP_FREE_DLL    0



/* Task management *************************************************/

#ifndef NOAPIPROTO

UINT    WINAPI GetNumTasks(void);
BOOL    WINAPI IsTask(HTASK);

HTASK   WINAPI GetCurrentTask(void);

void    WINAPI Yield(void);
void    WINAPI DirectedYield(HTASK);

#endif      /* NOAPIPROTO */

/* Thread management ***********************************************/

#ifndef NOAPIPROTO

HANDLE	WINAPI GetCurrentProcess(void);
HANDLE	WINAPI GetCurrentThread(void);

#endif      /* NOAPIPROTO */

/* Global memory management ****************************************/
#ifndef NOMEMMGR

#ifdef STRICT
typedef BOOL (CALLBACK* GNOTIFYPROC)(HGLOBAL);
#else
typedef FARPROC GNOTIFYPROC;
#endif

/* API Prototypes **************/
#ifndef NOAPIPROTO

#define GlobalDiscard(h)    GlobalReAlloc(h, 0L, GMEM_MOVEABLE)

HGLOBAL WINAPI GlobalAlloc(UINT, DWORD);
HGLOBAL WINAPI GlobalReAlloc(HGLOBAL, DWORD, UINT);
HGLOBAL WINAPI GlobalFree(HGLOBAL);

DWORD   WINAPI GlobalDosAlloc(DWORD);
UINT    WINAPI GlobalDosFree(UINT);

#ifdef STRICT
void FAR* WINAPI GlobalLock(HGLOBAL);
#else
char FAR* WINAPI GlobalLock(HGLOBAL);
#endif

BOOL    WINAPI GlobalUnlock(HGLOBAL);

DWORD   WINAPI GlobalSize(HGLOBAL);

HGLOBAL	WINAPI GlobalHandle(LPCVOID);
HGLOBAL GlobalHandle32(LPCVOID);

UINT    WINAPI GlobalFlags(HGLOBAL);

#ifdef STRICT
void FAR* WINAPI GlobalWire(HGLOBAL);
#else
char FAR* WINAPI GlobalWire(HGLOBAL);
#endif

BOOL    WINAPI GlobalUnWire(HGLOBAL);

UINT    WINAPI GlobalPageLock(HGLOBAL);
UINT    WINAPI GlobalPageUnlock(HGLOBAL);

void    WINAPI GlobalFix(HGLOBAL);
void    WINAPI GlobalUnfix(HGLOBAL);

HGLOBAL WINAPI GlobalLRUNewest(HGLOBAL);
HGLOBAL WINAPI GlobalLRUOldest(HGLOBAL);

DWORD   WINAPI GlobalCompact(DWORD);

void    WINAPI GlobalNotify(GNOTIFYPROC);

HGLOBAL WINAPI LockSegment(UINT);
void    WINAPI UnlockSegment(UINT);

#define LockData(dummy)     LockSegment((UINT)-1)
#define UnlockData(dummy)   UnlockSegment((UINT)-1)

UINT    WINAPI AllocSelector(UINT);
UINT    WINAPI FreeSelector(UINT);
UINT    WINAPI AllocDStoCSAlias(UINT);
UINT    WINAPI PrestoChangoSelector(UINT sourceSel, UINT destSel);
DWORD   WINAPI GetSelectorBase(UINT);
UINT    WINAPI SetSelectorBase(UINT, DWORD);
DWORD   WINAPI GetSelectorLimit(UINT);
UINT    WINAPI SetSelectorLimit(UINT, DWORD);

void    WINAPI LimitEmsPages(DWORD);

void    WINAPI ValidateFreeSpaces(void);

/* Local Memory Management */
#define LocalDiscard     GlobalDiscard

#define LocalLock    GlobalLock
#define LocalUnlock  GlobalUnlock
#define LocalFree    GlobalFree
#define LocalAlloc   GlobalAlloc
#define LocalReAlloc GlobalReAlloc
#define LocalSize    GlobalSize

#define LocalHandle  GlobalHandle
#define LocalFlags   GlobalFlags
#define LocalCompact GlobalCompact
#define LocalShrink  GlobalShrink


#endif      /* NOAPIPROTO */


/* Flags and definitions *******/

/* Global Memory Flags */

#define GMEM_FIXED          0x0000
#define GMEM_MOVEABLE       0x0002
#define GMEM_NOCOMPACT      0x0010
#define GMEM_NODISCARD      0x0020
#define GMEM_ZEROINIT       0x0040
#define GMEM_MODIFY         0x0080
#define GMEM_DISCARDABLE    0x0100
#define GMEM_NOT_BANKED     0x1000
#define GMEM_SHARE          0x2000
#define GMEM_DDESHARE       0x2000
#define GMEM_NOTIFY         0x4000
#define GMEM_LOWER          GMEM_NOT_BANKED
#define GMEM_VALID_FLAGS    0x7F72
#define GMEM_INVALID_HANDLE 0x8000

#define GHND            (GMEM_MOVEABLE | GMEM_ZEROINIT)
#define GPTR            (GMEM_FIXED | GMEM_ZEROINIT)

/* Flags returned from GlobalFlags() */
/* (in addition to GMEM_DISCARDABLE) */
#define GMEM_DISCARDED      0x4000
#define GMEM_LOCKCOUNT      0x00FF

/* Low system memory notification message */
#define WM_COMPACTING       0x0041


/* Local memory management flags */
#define LMEM_FIXED          0x0000
#define LMEM_MOVEABLE       0x0002
#define LMEM_NOCOMPACT      0x0010
#define LMEM_NODISCARD      0x0020
#define LMEM_ZEROINIT       0x0040
#define LMEM_MODIFY         0x0080
#define LMEM_DISCARDABLE    0x0F00

#define LHND            (LMEM_MOVEABLE | LMEM_ZEROINIT)
#define LPTR            (LMEM_FIXED | LMEM_ZEROINIT)

#define NONZEROLHND     (LMEM_MOVEABLE)
#define NONZEROLPTR     (LMEM_FIXED)

/* Flags returned from LocalFlags() */
/* (in addition to LMEM_DISCARDABLE) */
#define LMEM_DISCARDED      0x4000
#define LMEM_LOCKCOUNT      0x00FF

#endif      /* NOMEMMGR */


/* File I/O Support ************************************************/

#ifndef NOAPIPROTO

#ifdef TWIN32
UINT    TWINAPI GetDriveType16(int);
int	TWINAPI GetTempFileName16(BYTE, LPCSTR, UINT, LPSTR);
#else
UINT    WINAPI GetDriveType(int);
int     WINAPI GetTempFileName(BYTE, LPCSTR, UINT, LPSTR);
BYTE    WINAPI GetTempDrive(char);
#endif

UINT    WINAPI SetHandleCount(UINT);

#endif      /*NOAPIPROTO */

/* GetTempFileName() Flags */
#define TF_FORCEDRIVE       (BYTE)0x80

/* GetDriveType return values */
#define DRIVE_UNKNOWN   0
#define DRIVE_NO_ROOT_DIR 1
#define DRIVE_REMOVABLE 2
#define DRIVE_FIXED     3
#define DRIVE_REMOTE    4
#define DRIVE_CDROM     5
#define DRIVE_RAMDISK   6


#ifndef NOLFILEIO

typedef int HFILE;      /* Polymorphic with C runtime file handle type */
#define HFILE_ERROR ((HFILE)-1)


#ifndef NOAPIPROTO

HFILE   WINAPI _lopen(LPCSTR, int);
HFILE   WINAPI _lcreat(LPCSTR, int);
int	TWINAPI _ldelete(LPCSTR);

HFILE   WINAPI _lclose(HFILE);

LONG    WINAPI _llseek(HFILE, LONG, int);

UINT    WINAPI _lread(HFILE, void _huge*, UINT);
UINT    WINAPI _lwrite(HFILE, const void _huge*, UINT);

long    WINAPI _hread(HFILE, void _huge*, long);
long    WINAPI _hwrite(HFILE, const void _huge*, long);

#endif      /* NOAPIPROTO */


/* _lopen() flags */
#define READ        0
#define WRITE       1
#define READ_WRITE  2

/* _llseek origin values */
#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif


/* File Input ******************************************************/
#ifndef NOOPENFILE

/* OpenFile() Structure */
typedef struct tagOFSTRUCT
{
    BYTE cBytes;
    BYTE fFixedDisk;
    UINT nErrCode;
    BYTE reserved[4];
    char szPathName[128];
} OFSTRUCT;
typedef OFSTRUCT*       POFSTRUCT;
typedef OFSTRUCT NEAR* NPOFSTRUCT;
typedef OFSTRUCT FAR*  LPOFSTRUCT;


/* API Prototypes **************/
#ifndef NOAPIPROTO

HFILE   WINAPI OpenFile(LPCSTR, OFSTRUCT FAR*, UINT);

#endif      /* NOAPIPROTO */


/* Flags and definitions *******/

/* Flags for OpenFile() */
#define OF_READ         0x0000
#define OF_WRITE        0x0001
#define OF_READWRITE        0x0002
#define OF_SHARE_COMPAT     0x0000
#define OF_SHARE_EXCLUSIVE  0x0010
#define OF_SHARE_DENY_WRITE 0x0020
#define OF_SHARE_DENY_READ  0x0030
#define OF_SHARE_DENY_NONE  0x0040
#define OF_PARSE        0x0100
#define OF_DELETE       0x0200
#define OF_VERIFY       0x0400      /* Used with OF_REOPEN */
#define OF_SEARCH       0x0400      /* Used without OF_REOPEN */
#define OF_CANCEL       0x0800
#define OF_CREATE       0x1000
#define OF_PROMPT       0x2000
#define OF_EXIST        0x4000
#define OF_REOPEN       0x8000


#endif  /* NOOPENFILE */

#endif  /* NOLFILEIO */


/* Network support *************************************************/
#ifndef NOAPIPROTO

UINT WINAPI WNetAddConnection(LPSTR, LPSTR, LPSTR);
UINT WINAPI WNetGetConnection(LPSTR, LPSTR, UINT FAR*);
UINT WINAPI WNetCancelConnection(LPSTR, BOOL);

#endif      /* NOAPIPROTO */

/* Errors */
#define WN_SUCCESS              0x0000
#define WN_NOT_SUPPORTED        0x0001
#define WN_NET_ERROR            0x0002
#define WN_MORE_DATA            0x0003
#define WN_BAD_POINTER          0x0004
#define WN_BAD_VALUE            0x0005
#define WN_BAD_PASSWORD         0x0006
#define WN_ACCESS_DENIED        0x0007
#define WN_FUNCTION_BUSY        0x0008
#define WN_WINDOWS_ERROR        0x0009
#define WN_BAD_USER             0x000A
#define WN_OUT_OF_MEMORY        0x000B
#define WN_CANCEL               0x000C
#define WN_CONTINUE             0x000D

/* Connection errors */
#define WN_NOT_CONNECTED        0x0030
#define WN_OPEN_FILES           0x0031
#define WN_BAD_NETNAME          0x0032
#define WN_BAD_LOCALNAME        0x0033
#define WN_ALREADY_CONNECTED    0x0034
#define WN_DEVICE_ERROR         0x0035
#define WN_CONNECTION_CLOSED    0x0036


/* Resource management *********************************************/
#define MAKEINTRESOURCE(i)  ((LPCSTR)MAKELP(0, (i)))

#ifdef STRICT
typedef HGLOBAL (CALLBACK* RSRCHDLRPROC)(HGLOBAL, HINSTANCE, HRSRC);
#else
typedef FARPROC RSRCHDLRPROC;
#endif

#ifndef NOAPIPROTO

HRSRC   WINAPI FindResource(HINSTANCE, LPCSTR, LPCSTR);
HGLOBAL WINAPI LoadResource(HINSTANCE, HRSRC);
BOOL    WINAPI FreeResource(HGLOBAL);

HANDLE  WINAPI LoadImage(HINSTANCE, LPCSTR, UINT, int, int, UINT);

#ifdef STRICT
void FAR* WINAPI LockResource(HGLOBAL);
#else
char FAR* WINAPI LockResource(HGLOBAL);
#endif

#define     UnlockResource(h)       GlobalUnlock(h)

DWORD   WINAPI SizeofResource(HINSTANCE, HRSRC);

int     WINAPI AccessResource(HINSTANCE, HRSRC);

HGLOBAL WINAPI AllocResource(HINSTANCE, HRSRC, DWORD);

RSRCHDLRPROC WINAPI SetResourceHandler(HINSTANCE, LPCSTR, RSRCHDLRPROC);

#endif      /* NOAPIPROTO */


#ifndef NORESOURCE

/* Predefined Resource Types */
#define RT_CURSOR       MAKEINTRESOURCE(1)
#define RT_BITMAP       MAKEINTRESOURCE(2)
#define RT_ICON         MAKEINTRESOURCE(3)
#define RT_MENU         MAKEINTRESOURCE(4)
#define RT_DIALOG       MAKEINTRESOURCE(5)
#define RT_STRING       MAKEINTRESOURCE(6)
#define RT_FONTDIR      MAKEINTRESOURCE(7)
#define RT_FONT         MAKEINTRESOURCE(8)
#define RT_ACCELERATOR  MAKEINTRESOURCE(9)
#define RT_RCDATA       MAKEINTRESOURCE(10)

#define RT_GROUP_CURSOR MAKEINTRESOURCE(12)
#define RT_GROUP_ICON   MAKEINTRESOURCE(14)


#endif  /* NORESOURCE */


#ifdef OEMRESOURCE

/* OEM Resource Ordinal Numbers */
#define OBM_CLOSE           32754
#define OBM_UPARROW         32753
#define OBM_DNARROW         32752
#define OBM_RGARROW         32751
#define OBM_LFARROW         32750
#define OBM_REDUCE          32749
#define OBM_ZOOM            32748
#define OBM_RESTORE         32747
#define OBM_REDUCED         32746
#define OBM_ZOOMD           32745
#define OBM_RESTORED        32744
#define OBM_UPARROWD        32743
#define OBM_DNARROWD        32742
#define OBM_RGARROWD        32741
#define OBM_LFARROWD        32740
#define OBM_MNARROW         32739
#define OBM_COMBO           32738

#define OBM_UPARROWI        32737
#define OBM_DNARROWI        32736
#define OBM_RGARROWI        32735
#define OBM_LFARROWI        32734

#define OBM_OLD_CLOSE       32767
#define OBM_SIZE            32766
#define OBM_OLD_UPARROW     32765
#define OBM_OLD_DNARROW     32764
#define OBM_OLD_RGARROW     32763
#define OBM_OLD_LFARROW     32762
#define OBM_BTSIZE          32761
#define OBM_CHECK           32760
#define OBM_CHECKBOXES      32759
#define OBM_BTNCORNERS      32758
#define OBM_OLD_REDUCE      32757
#define OBM_OLD_ZOOM        32756
#define OBM_OLD_RESTORE     32755

#define OCR_NORMAL          32512
#define OCR_IBEAM           32513
#define OCR_WAIT            32514
#define OCR_CROSS           32515
#define OCR_UP              32516
#define OCR_SIZE            32640
#define OCR_ICON            32641
#define OCR_SIZENWSE        32642
#define OCR_SIZENESW        32643
#define OCR_SIZEWE          32644
#define OCR_SIZENS          32645
#define OCR_SIZEALL         32646
#define OCR_ICOCUR          32647

#define OIC_SAMPLE          32512
#define OIC_HAND            32513
#define OIC_QUES            32514
#define OIC_BANG            32515
#define OIC_NOTE            32516

#endif  /* OEMRESOURCE */



/* Atom management *************************************************/
#define MAKEINTATOM(i)      ((LPCSTR)MAKELP(0, (i)))

#ifndef NOATOM
#ifndef NOAPIPROTO

BOOL    WINAPI InitAtomTable(int);
ATOM    WINAPI AddAtom(LPCSTR);
ATOM    WINAPI DeleteAtom(ATOM);
ATOM    WINAPI FindAtom(LPCSTR);
UINT    WINAPI GetAtomName(ATOM, LPSTR, int);
ATOM    WINAPI GlobalAddAtom(LPCSTR);
ATOM    WINAPI GlobalDeleteAtom(ATOM);
ATOM    WINAPI GlobalFindAtom(LPCSTR);
UINT    WINAPI GlobalGetAtomName(ATOM, LPSTR, int);
HLOCAL  WINAPI GetAtomHandle(ATOM);

#endif      /* NOAPIPROTO */
#endif      /* NOATOM */



/* win.ini support *************************************************/
#ifndef NOAPIPROTO

/* User Profile Routines */
UINT    WINAPI GetProfileInt(LPCSTR, LPCSTR, int);
int     WINAPI GetProfileString(LPCSTR, LPCSTR, LPCSTR, LPSTR, int);
BOOL    WINAPI WriteProfileString(LPCSTR, LPCSTR, LPCSTR);

UINT    WINAPI GetPrivateProfileInt(LPCSTR, LPCSTR, int, LPCSTR);
int     WINAPI GetPrivateProfileString(LPCSTR, LPCSTR, LPCSTR, LPSTR, int, LPCSTR);
BOOL    WINAPI WritePrivateProfileString(LPCSTR, LPCSTR, LPCSTR, LPCSTR);

#endif      /* NOAPIPROTO */

#define WM_WININICHANGE     0x001A



/* International and character translation support *****************/

#ifndef NOAPIPROTO

void    WINAPI AnsiToOem(const char _huge*, char _huge*);
void    WINAPI OemToAnsi(const char _huge*, char _huge*);

void    WINAPI AnsiToOemBuff(LPCSTR, LPSTR, UINT);
void    WINAPI OemToAnsiBuff(LPCSTR, LPSTR, UINT);

LPSTR   WINAPI AnsiNext(LPCSTR);
LPSTR   WINAPI AnsiPrev(LPCSTR, LPCSTR);

LPSTR   WINAPI AnsiUpper(LPSTR);
LPSTR   WINAPI AnsiLower(LPSTR);

UINT    WINAPI AnsiUpperBuff(LPSTR, UINT);
UINT    WINAPI AnsiLowerBuff(LPSTR, UINT);

#ifndef  NOLANGUAGE
BOOL    WINAPI IsCharAlpha(char);
BOOL    WINAPI IsCharAlphaNumeric(char);
BOOL    WINAPI IsCharUpper(char);
BOOL    WINAPI IsCharLower(char);
#endif

#ifndef NOLSTRING
int     WINAPI lstrcmp(LPCSTR, LPCSTR);
int     WINAPI lstrcmpi(LPCSTR, LPCSTR);
LPSTR   WINAPI lstrcpy(LPSTR, LPCSTR);
LPSTR   WINAPI lstrcat(LPSTR, LPCSTR);
int     WINAPI lstrlen(LPCSTR);

LPSTR   WINAPI lstrcpyn(LPSTR, LPCSTR, int);
void    WINAPI hmemcpy(void _huge*, const void _huge*, long);

#endif  /* NOLSTRING */

#ifndef NODBCS
BOOL    WINAPI IsDBCSLeadByte(BYTE);
#endif  /* NODBCS */

int     WINAPI LoadString(HINSTANCE, UINT, LPSTR, int);

#endif      /* NOAPIPROTO */



/* Keyboard driver support *****************************************/
#ifndef NOKEYBOARDINFO
#ifndef NOAPIPROTO

DWORD   WINAPI OemKeyScan(UINT);
UINT    WINAPI VkKeyScan(UINT);
int     WINAPI GetKeyboardType(int);
UINT    WINAPI MapVirtualKey(UINT, UINT);
int     WINAPI GetKBCodePage(void);
int     WINAPI GetKeyNameText(LONG, LPSTR, int);
int     WINAPI ToAscii(UINT wVirtKey, UINT wScanCode, BYTE FAR* lpKeyState, DWORD FAR* lpChar, UINT wFlags);

#endif      /* NOAPIPROTO */
#endif      /* NOKEYBOARDINFO */

/* (WIN32) Profile support ************************************************* */

#ifndef NOAPIPROTO

DWORD	WINAPI GetPrivateProfileSectionNames(LPTSTR, DWORD, LPCTSTR);
DWORD	WINAPI GetPrivateProfileSection(LPCSTR, LPSTR, DWORD, LPCSTR);
BOOL	WINAPI WritePrivateProfileSection(LPCTSTR, LPCTSTR, LPCTSTR);
BOOL	WINAPI WriteProfileSection(LPCTSTR, LPCTSTR);

BOOL	WINAPI GetPrivateProfileStruct(LPCTSTR, LPCTSTR, LPVOID, UINT,
	LPCTSTR);
BOOL	WINAPI WritePrivateProfileStruct(LPCTSTR, LPCTSTR, LPVOID, UINT,
	LPCTSTR);

#endif  /* NOAPIPROTO */

/* (WIN32) Kernel Error **************************************************** */

/*
 *  NOTE: Due to the way the TWIN library is currently built, we need
 *  these Win32 message numbers visible even for a TWIN16 build.
 *  However, some of these conflict with messages under Win16, so we
 *  individually comment out each affected message.  It's not very pretty,
 *  and not really perfect, but it should work for now.
 */

#define NO_ERROR			0

#if defined(TWIN32)
#define ERROR_SUCCESS                    0L
#endif /* TWIN32 */
#define ERROR_INVALID_FUNCTION           1L
#define ERROR_FILE_NOT_FOUND             2L
#define ERROR_PATH_NOT_FOUND             3L
#define ERROR_TOO_MANY_OPEN_FILES        4L
#if defined(TWIN32)
#define ERROR_ACCESS_DENIED              5L
#endif /* TWIN32 */
#define ERROR_INVALID_HANDLE             6L
#define ERROR_ARENA_TRASHED              7L
#define ERROR_NOT_ENOUGH_MEMORY          8L
#define ERROR_INVALID_BLOCK              9L
#define ERROR_BAD_ENVIRONMENT            10L
#define ERROR_BAD_FORMAT                 11L
#define ERROR_INVALID_ACCESS             12L
#define ERROR_INVALID_DATA               13L
#if defined(TWIN32)
#define ERROR_OUTOFMEMORY                14L
#endif /* TWIN32 */
#define ERROR_INVALID_DRIVE              15L
#define ERROR_CURRENT_DIRECTORY          16L
#define ERROR_NOT_SAME_DEVICE            17L
#define ERROR_NO_MORE_FILES              18L
#define ERROR_WRITE_PROTECT              19L
#define ERROR_BAD_UNIT                   20L
#define ERROR_NOT_READY                  21L
#define ERROR_BAD_COMMAND                22L
#define ERROR_CRC                        23L
#define ERROR_BAD_LENGTH                 24L
#define ERROR_SEEK                       25L
#define ERROR_NOT_DOS_DISK               26L
#define ERROR_SECTOR_NOT_FOUND           27L
#define ERROR_OUT_OF_PAPER               28L
#define ERROR_WRITE_FAULT                29L
#define ERROR_READ_FAULT                 30L
#define ERROR_GEN_FAILURE                31L
#define ERROR_SHARING_VIOLATION          32L
#define ERROR_LOCK_VIOLATION             33L
#define ERROR_WRONG_DISK                 34L
#define ERROR_SHARING_BUFFER_EXCEEDED    36L
#define ERROR_HANDLE_EOF                 38L
#define ERROR_HANDLE_DISK_FULL           39L
#define ERROR_NOT_SUPPORTED              50L
#define ERROR_REM_NOT_LIST               51L
#define ERROR_DUP_NAME                   52L
#define ERROR_BAD_NETPATH                53L
#define ERROR_NETWORK_BUSY               54L
#define ERROR_DEV_NOT_EXIST              55L
#define ERROR_TOO_MANY_CMDS              56L
#define ERROR_ADAP_HDW_ERR               57L
#define ERROR_BAD_NET_RESP               58L
#define ERROR_UNEXP_NET_ERR              59L
#define ERROR_BAD_REM_ADAP               60L
#define ERROR_PRINTQ_FULL                61L
#define ERROR_NO_SPOOL_SPACE             62L
#define ERROR_PRINT_CANCELLED            63L
#define ERROR_NETNAME_DELETED            64L
#define ERROR_NETWORK_ACCESS_DENIED      65L
#define ERROR_BAD_DEV_TYPE               66L
#define ERROR_BAD_NET_NAME               67L
#define ERROR_TOO_MANY_NAMES             68L
#define ERROR_TOO_MANY_SESS              69L
#define ERROR_SHARING_PAUSED             70L
#define ERROR_REQ_NOT_ACCEP              71L
#define ERROR_REDIR_PAUSED               72L
#define ERROR_FILE_EXISTS                80L
#define ERROR_CANNOT_MAKE                82L
#define ERROR_FAIL_I24                   83L
#define ERROR_OUT_OF_STRUCTURES          84L
#define ERROR_ALREADY_ASSIGNED           85L
#define ERROR_INVALID_PASSWORD           86L
#if defined(TWIN32)
#define ERROR_INVALID_PARAMETER          87L
#endif /* TWIN32 */
#define ERROR_NET_WRITE_FAULT            88L
#define ERROR_NO_PROC_SLOTS              89L
#define ERROR_TOO_MANY_SEMAPHORES        100L
#define ERROR_EXCL_SEM_ALREADY_OWNED     101L
#define ERROR_SEM_IS_SET                 102L
#define ERROR_TOO_MANY_SEM_REQUESTS      103L
#define ERROR_INVALID_AT_INTERRUPT_TIME  104L
#define ERROR_SEM_OWNER_DIED             105L
#define ERROR_SEM_USER_LIMIT             106L
#define ERROR_DISK_CHANGE                107L
#define ERROR_DRIVE_LOCKED               108L
#define ERROR_BROKEN_PIPE                109L
#define ERROR_OPEN_FAILED                110L
#define ERROR_BUFFER_OVERFLOW            111L
#define ERROR_DISK_FULL                  112L
#define ERROR_NO_MORE_SEARCH_HANDLES     113L
#define ERROR_INVALID_TARGET_HANDLE      114L
#define ERROR_INVALID_CATEGORY           117L
#define ERROR_INVALID_VERIFY_SWITCH      118L
#define ERROR_BAD_DRIVER_LEVEL           119L
#define ERROR_CALL_NOT_IMPLEMENTED       120L
#define ERROR_SEM_TIMEOUT                121L
#define ERROR_INSUFFICIENT_BUFFER        122L
#define ERROR_INVALID_NAME               123L
#define ERROR_INVALID_LEVEL              124L
#define ERROR_NO_VOLUME_LABEL            125L
#define ERROR_MOD_NOT_FOUND              126L
#define ERROR_PROC_NOT_FOUND             127L
#define ERROR_WAIT_NO_CHILDREN           128L
#define ERROR_CHILD_NOT_COMPLETE         129L
#define ERROR_DIRECT_ACCESS_HANDLE       130L
#define ERROR_NEGATIVE_SEEK              131L
#define ERROR_SEEK_ON_DEVICE             132L
#define ERROR_IS_JOIN_TARGET             133L
#define ERROR_IS_JOINED                  134L
#define ERROR_IS_SUBSTED                 135L
#define ERROR_NOT_JOINED                 136L
#define ERROR_NOT_SUBSTED                137L
#define ERROR_JOIN_TO_JOIN               138L
#define ERROR_SUBST_TO_SUBST             139L
#define ERROR_JOIN_TO_SUBST              140L
#define ERROR_SUBST_TO_JOIN              141L
#define ERROR_BUSY_DRIVE                 142L
#define ERROR_SAME_DRIVE                 143L
#define ERROR_DIR_NOT_ROOT               144L
#define ERROR_DIR_NOT_EMPTY              145L
#define ERROR_IS_SUBST_PATH              146L
#define ERROR_IS_JOIN_PATH               147L
#define ERROR_PATH_BUSY                  148L
#define ERROR_IS_SUBST_TARGET            149L
#define ERROR_SYSTEM_TRACE               150L
#define ERROR_INVALID_EVENT_COUNT        151L
#define ERROR_TOO_MANY_MUXWAITERS        152L
#define ERROR_INVALID_LIST_FORMAT        153L
#define ERROR_LABEL_TOO_LONG             154L
#define ERROR_TOO_MANY_TCBS              155L
#define ERROR_SIGNAL_REFUSED             156L
#define ERROR_DISCARDED                  157L
#define ERROR_NOT_LOCKED                 158L
#define ERROR_BAD_THREADID_ADDR          159L
#define ERROR_BAD_ARGUMENTS              160L
#define ERROR_BAD_PATHNAME               161L
#define ERROR_SIGNAL_PENDING             162L
#define ERROR_MAX_THRDS_REACHED          164L
#define ERROR_LOCK_FAILED                167L
#define ERROR_BUSY                       170L
#define ERROR_CANCEL_VIOLATION           173L
#define ERROR_ATOMIC_LOCKS_NOT_SUPPORTED 174L
#define ERROR_INVALID_SEGMENT_NUMBER     180L
#define ERROR_INVALID_ORDINAL            182L
#define ERROR_ALREADY_EXISTS             183L
#define ERROR_INVALID_FLAG_NUMBER        186L
#define ERROR_SEM_NOT_FOUND              187L
#define ERROR_INVALID_STARTING_CODESEG   188L
#define ERROR_INVALID_STACKSEG           189L
#define ERROR_INVALID_MODULETYPE         190L
#define ERROR_INVALID_EXE_SIGNATURE      191L
#define ERROR_EXE_MARKED_INVALID         192L
#define ERROR_BAD_EXE_FORMAT             193L
#define ERROR_ITERATED_DATA_EXCEEDS_64k  194L
#define ERROR_INVALID_MINALLOCSIZE       195L
#define ERROR_DYNLINK_FROM_INVALID_RING  196L
#define ERROR_IOPL_NOT_ENABLED           197L
#define ERROR_INVALID_SEGDPL             198L
#define ERROR_AUTODATASEG_EXCEEDS_64k    199L
#define ERROR_RING2SEG_MUST_BE_MOVABLE   200L
#define ERROR_RELOC_CHAIN_XEEDS_SEGLIM   201L
#define ERROR_INFLOOP_IN_RELOC_CHAIN     202L
#define ERROR_ENVVAR_NOT_FOUND           203L
#define ERROR_NO_SIGNAL_SENT             205L
#define ERROR_FILENAME_EXCED_RANGE       206L
#define ERROR_RING2_STACK_IN_USE         207L
#define ERROR_META_EXPANSION_TOO_LONG    208L
#define ERROR_INVALID_SIGNAL_NUMBER      209L
#define ERROR_THREAD_1_INACTIVE          210L
#define ERROR_LOCKED                     212L
#define ERROR_TOO_MANY_MODULES           214L
#define ERROR_NESTING_NOT_ALLOWED        215L
#define ERROR_BAD_PIPE                   230L
#define ERROR_PIPE_BUSY                  231L
#define ERROR_NO_DATA                    232L
#define ERROR_PIPE_NOT_CONNECTED         233L
#define ERROR_MORE_DATA                  234L
#define ERROR_VC_DISCONNECTED            240L
#define ERROR_INVALID_EA_NAME            254L
#define ERROR_EA_LIST_INCONSISTENT       255L
#define ERROR_NO_MORE_ITEMS              259L
#define ERROR_CANNOT_COPY                266L
#define ERROR_DIRECTORY                  267L
#define ERROR_EAS_DIDNT_FIT              275L
#define ERROR_EA_FILE_CORRUPT            276L
#define ERROR_EA_TABLE_FULL              277L
#define ERROR_INVALID_EA_HANDLE          278L
#define ERROR_EAS_NOT_SUPPORTED          282L
#define ERROR_NOT_OWNER                  288L
#define ERROR_TOO_MANY_POSTS             298L
#define ERROR_PARTIAL_COPY               299L
#define ERROR_MR_MID_NOT_FOUND           317L
#define ERROR_INVALID_ADDRESS            487L
#define ERROR_ARITHMETIC_OVERFLOW        534L
#define ERROR_PIPE_CONNECTED             535L
#define ERROR_PIPE_LISTENING             536L
#define ERROR_EA_ACCESS_DENIED           994L
#define ERROR_OPERATION_ABORTED          995L
#define ERROR_IO_INCOMPLETE              996L
#define ERROR_IO_PENDING                 997L
#define ERROR_NOACCESS                   998L
#define ERROR_SWAPERROR                  999L
#define ERROR_STACK_OVERFLOW             1001L
#define ERROR_INVALID_MESSAGE            1002L
#define ERROR_CAN_NOT_COMPLETE           1003L
#define ERROR_INVALID_FLAGS              1004L
#define ERROR_UNRECOGNIZED_VOLUME        1005L
#define ERROR_FILE_INVALID               1006L
#define ERROR_FULLSCREEN_MODE            1007L
#define ERROR_NO_TOKEN                   1008L
#if defined(TWIN32)
#define ERROR_BADDB                      1009L
#define ERROR_BADKEY                     1010L
#define ERROR_CANTOPEN                   1011L
#define ERROR_CANTREAD                   1012L
#define ERROR_CANTWRITE                  1013L
#endif /* TWIN32 */
#define ERROR_REGISTRY_RECOVERED         1014L
#define ERROR_REGISTRY_CORRUPT           1015L
#define ERROR_REGISTRY_IO_FAILED         1016L
#define ERROR_NOT_REGISTRY_FILE          1017L
#define ERROR_KEY_DELETED                1018L
#define ERROR_NO_LOG_SPACE               1019L
#define ERROR_KEY_HAS_CHILDREN           1020L
#define ERROR_CHILD_MUST_BE_VOLATILE     1021L
#define ERROR_NOTIFY_ENUM_DIR            1022L
#define ERROR_DEPENDENT_SERVICES_RUNNING 1051L
#define ERROR_INVALID_SERVICE_CONTROL    1052L
#define ERROR_SERVICE_REQUEST_TIMEOUT    1053L
#define ERROR_SERVICE_NO_THREAD          1054L
#define ERROR_SERVICE_DATABASE_LOCKED    1055L
#define ERROR_SERVICE_ALREADY_RUNNING    1056L
#define ERROR_INVALID_SERVICE_ACCOUNT    1057L
#define ERROR_SERVICE_DISABLED           1058L
#define ERROR_CIRCULAR_DEPENDENCY        1059L
#define ERROR_SERVICE_DOES_NOT_EXIST     1060L
#define ERROR_SERVICE_CANNOT_ACCEPT_CTRL 1061L
#define ERROR_SERVICE_NOT_ACTIVE         1062L
#define ERROR_FAILED_SERVICE_CONTROLLER_CONNECT 1063L
#define ERROR_EXCEPTION_IN_SERVICE       1064L
#define ERROR_DATABASE_DOES_NOT_EXIST    1065L
#define ERROR_SERVICE_SPECIFIC_ERROR     1066L
#define ERROR_PROCESS_ABORTED            1067L
#define ERROR_SERVICE_DEPENDENCY_FAIL    1068L
#define ERROR_SERVICE_LOGON_FAILED       1069L
#define ERROR_SERVICE_START_HANG         1070L
#define ERROR_INVALID_SERVICE_LOCK       1071L
#define ERROR_SERVICE_MARKED_FOR_DELETE  1072L
#define ERROR_SERVICE_EXISTS             1073L
#define ERROR_ALREADY_RUNNING_LKG        1074L
#define ERROR_SERVICE_DEPENDENCY_DELETED 1075L
#define ERROR_BOOT_ALREADY_ACCEPTED      1076L
#define ERROR_SERVICE_NEVER_STARTED      1077L
#define ERROR_DUPLICATE_SERVICE_NAME     1078L
#define ERROR_END_OF_MEDIA               1100L
#define ERROR_FILEMARK_DETECTED          1101L
#define ERROR_BEGINNING_OF_MEDIA         1102L
#define ERROR_SETMARK_DETECTED           1103L
#define ERROR_NO_DATA_DETECTED           1104L
#define ERROR_PARTITION_FAILURE          1105L
#define ERROR_INVALID_BLOCK_LENGTH       1106L
#define ERROR_DEVICE_NOT_PARTITIONED     1107L
#define ERROR_UNABLE_TO_LOCK_MEDIA       1108L
#define ERROR_UNABLE_TO_UNLOAD_MEDIA     1109L
#define ERROR_MEDIA_CHANGED              1110L
#define ERROR_BUS_RESET                  1111L
#define ERROR_NO_MEDIA_IN_DRIVE          1112L
#define ERROR_NO_UNICODE_TRANSLATION     1113L
#define ERROR_DLL_INIT_FAILED            1114L
#define ERROR_SHUTDOWN_IN_PROGRESS       1115L
#define ERROR_NO_SHUTDOWN_IN_PROGRESS    1116L
#define ERROR_IO_DEVICE                  1117L
#define ERROR_SERIAL_NO_DEVICE           1118L
#define ERROR_IRQ_BUSY                   1119L
#define ERROR_MORE_WRITES                1120L
#define ERROR_COUNTER_TIMEOUT            1121L
#define ERROR_FLOPPY_ID_MARK_NOT_FOUND   1122L
#define ERROR_FLOPPY_WRONG_CYLINDER      1123L
#define ERROR_FLOPPY_UNKNOWN_ERROR       1124L
#define ERROR_FLOPPY_BAD_REGISTERS       1125L
#define ERROR_DISK_RECALIBRATE_FAILED    1126L
#define ERROR_DISK_OPERATION_FAILED      1127L
#define ERROR_DISK_RESET_FAILED          1128L
#define ERROR_EOM_OVERFLOW               1129L
#define ERROR_NOT_ENOUGH_SERVER_MEMORY   1130L
#define ERROR_POSSIBLE_DEADLOCK          1131L
#define ERROR_MAPPED_ALIGNMENT           1132L
#define ERROR_SET_POWER_STATE_VETOED     1140L
#define ERROR_SET_POWER_STATE_FAILED     1141L
#define ERROR_OLD_WIN_VERSION            1150L
#define ERROR_APP_WRONG_OS               1151L
#define ERROR_SINGLE_INSTANCE_APP        1152L
#define ERROR_RMODE_APP                  1153L
#define ERROR_INVALID_DLL                1154L
#define ERROR_NO_ASSOCIATION             1155L
#define ERROR_DDE_FAIL                   1156L
#define ERROR_DLL_NOT_FOUND              1157L
#define ERROR_BAD_USERNAME               2202L
#define ERROR_NOT_CONNECTED              2250L
#define ERROR_OPEN_FILES                 2401L
#define ERROR_ACTIVE_CONNECTIONS         2402L
#define ERROR_DEVICE_IN_USE              2404L
#define ERROR_BAD_DEVICE                 1200L
#define ERROR_CONNECTION_UNAVAIL         1201L
#define ERROR_DEVICE_ALREADY_REMEMBERED  1202L
#define ERROR_NO_NET_OR_BAD_PATH         1203L
#define ERROR_BAD_PROVIDER               1204L
#define ERROR_CANNOT_OPEN_PROFILE        1205L
#define ERROR_BAD_PROFILE                1206L
#define ERROR_NOT_CONTAINER              1207L
#define ERROR_EXTENDED_ERROR             1208L
#define ERROR_INVALID_GROUPNAME          1209L
#define ERROR_INVALID_COMPUTERNAME       1210L
#define ERROR_INVALID_EVENTNAME          1211L
#define ERROR_INVALID_DOMAINNAME         1212L
#define ERROR_INVALID_SERVICENAME        1213L
#define ERROR_INVALID_NETNAME            1214L
#define ERROR_INVALID_SHARENAME          1215L
#define ERROR_INVALID_PASSWORDNAME       1216L
#define ERROR_INVALID_MESSAGENAME        1217L
#define ERROR_INVALID_MESSAGEDEST        1218L
#define ERROR_SESSION_CREDENTIAL_CONFLICT 1219L
#define ERROR_REMOTE_SESSION_LIMIT_EXCEEDED 1220L
#define ERROR_DUP_DOMAINNAME             1221L
#define ERROR_NO_NETWORK                 1222L
#define ERROR_CANCELLED                  1223L
#define ERROR_USER_MAPPED_FILE           1224L
#define ERROR_CONNECTION_REFUSED         1225L
#define ERROR_GRACEFUL_DISCONNECT        1226L
#define ERROR_ADDRESS_ALREADY_ASSOCIATED 1227L
#define ERROR_ADDRESS_NOT_ASSOCIATED     1228L
#define ERROR_CONNECTION_INVALID         1229L
#define ERROR_CONNECTION_ACTIVE          1230L
#define ERROR_NETWORK_UNREACHABLE        1231L
#define ERROR_HOST_UNREACHABLE           1232L
#define ERROR_PROTOCOL_UNREACHABLE       1233L
#define ERROR_PORT_UNREACHABLE           1234L
#define ERROR_REQUEST_ABORTED            1235L
#define ERROR_CONNECTION_ABORTED         1236L
#define ERROR_RETRY                      1237L
#define ERROR_CONNECTION_COUNT_LIMIT     1238L
#define ERROR_LOGIN_TIME_RESTRICTION     1239L
#define ERROR_LOGIN_WKSTA_RESTRICTION    1240L
#define ERROR_INCORRECT_ADDRESS          1241L
#define ERROR_ALREADY_REGISTERED         1242L
#define ERROR_SERVICE_NOT_FOUND          1243L
#define ERROR_NOT_AUTHENTICATED          1244L
#define ERROR_NOT_LOGGED_ON              1245L
#define ERROR_CONTINUE                   1246L
#define ERROR_ALREADY_INITIALIZED        1247L
#define ERROR_NO_MORE_DEVICES            1248L
#define ERROR_NOT_ALL_ASSIGNED           1300L
#define ERROR_SOME_NOT_MAPPED            1301L
#define ERROR_NO_QUOTAS_FOR_ACCOUNT      1302L
#define ERROR_LOCAL_USER_SESSION_KEY     1303L
#define ERROR_NULL_LM_PASSWORD           1304L
#define ERROR_UNKNOWN_REVISION           1305L
#define ERROR_REVISION_MISMATCH          1306L
#define ERROR_INVALID_OWNER              1307L
#define ERROR_INVALID_PRIMARY_GROUP      1308L
#define ERROR_NO_IMPERSONATION_TOKEN     1309L
#define ERROR_CANT_DISABLE_MANDATORY     1310L
#define ERROR_NO_LOGON_SERVERS           1311L
#define ERROR_NO_SUCH_LOGON_SESSION      1312L
#define ERROR_NO_SUCH_PRIVILEGE          1313L
#define ERROR_PRIVILEGE_NOT_HELD         1314L
#define ERROR_INVALID_ACCOUNT_NAME       1315L
#define ERROR_USER_EXISTS                1316L
#define ERROR_NO_SUCH_USER               1317L
#define ERROR_GROUP_EXISTS               1318L
#define ERROR_NO_SUCH_GROUP              1319L
#define ERROR_MEMBER_IN_GROUP            1320L
#define ERROR_MEMBER_NOT_IN_GROUP        1321L
#define ERROR_LAST_ADMIN                 1322L
#define ERROR_WRONG_PASSWORD             1323L
#define ERROR_ILL_FORMED_PASSWORD        1324L
#define ERROR_PASSWORD_RESTRICTION       1325L
#define ERROR_LOGON_FAILURE              1326L
#define ERROR_ACCOUNT_RESTRICTION        1327L
#define ERROR_INVALID_LOGON_HOURS        1328L
#define ERROR_INVALID_WORKSTATION        1329L
#define ERROR_PASSWORD_EXPIRED           1330L
#define ERROR_ACCOUNT_DISABLED           1331L
#define ERROR_NONE_MAPPED                1332L
#define ERROR_TOO_MANY_LUIDS_REQUESTED   1333L
#define ERROR_LUIDS_EXHAUSTED            1334L
#define ERROR_INVALID_SUB_AUTHORITY      1335L
#define ERROR_INVALID_ACL                1336L
#define ERROR_INVALID_SID                1337L
#define ERROR_INVALID_SECURITY_DESCR     1338L
#define ERROR_BAD_INHERITANCE_ACL        1340L
#define ERROR_SERVER_DISABLED            1341L
#define ERROR_SERVER_NOT_DISABLED        1342L
#define ERROR_INVALID_ID_AUTHORITY       1343L
#define ERROR_ALLOTTED_SPACE_EXCEEDED    1344L
#define ERROR_INVALID_GROUP_ATTRIBUTES   1345L
#define ERROR_BAD_IMPERSONATION_LEVEL    1346L
#define ERROR_CANT_OPEN_ANONYMOUS        1347L
#define ERROR_BAD_VALIDATION_CLASS       1348L
#define ERROR_BAD_TOKEN_TYPE             1349L
#define ERROR_NO_SECURITY_ON_OBJECT      1350L
#define ERROR_CANT_ACCESS_DOMAIN_INFO    1351L
#define ERROR_INVALID_SERVER_STATE       1352L
#define ERROR_INVALID_DOMAIN_STATE       1353L
#define ERROR_INVALID_DOMAIN_ROLE        1354L
#define ERROR_NO_SUCH_DOMAIN             1355L
#define ERROR_DOMAIN_EXISTS              1356L
#define ERROR_DOMAIN_LIMIT_EXCEEDED      1357L
#define ERROR_INTERNAL_DB_CORRUPTION     1358L
#define ERROR_INTERNAL_ERROR             1359L
#define ERROR_GENERIC_NOT_MAPPED         1360L
#define ERROR_BAD_DESCRIPTOR_FORMAT      1361L
#define ERROR_NOT_LOGON_PROCESS          1362L
#define ERROR_LOGON_SESSION_EXISTS       1363L
#define ERROR_NO_SUCH_PACKAGE            1364L
#define ERROR_BAD_LOGON_SESSION_STATE    1365L
#define ERROR_LOGON_SESSION_COLLISION    1366L
#define ERROR_INVALID_LOGON_TYPE         1367L
#define ERROR_CANNOT_IMPERSONATE         1368L
#define ERROR_RXACT_INVALID_STATE        1369L
#define ERROR_RXACT_COMMIT_FAILURE       1370L
#define ERROR_SPECIAL_ACCOUNT            1371L
#define ERROR_SPECIAL_GROUP              1372L
#define ERROR_SPECIAL_USER               1373L
#define ERROR_MEMBERS_PRIMARY_GROUP      1374L
#define ERROR_TOKEN_ALREADY_IN_USE       1375L
#define ERROR_NO_SUCH_ALIAS              1376L
#define ERROR_MEMBER_NOT_IN_ALIAS        1377L
#define ERROR_MEMBER_IN_ALIAS            1378L
#define ERROR_ALIAS_EXISTS               1379L
#define ERROR_LOGON_NOT_GRANTED          1380L
#define ERROR_TOO_MANY_SECRETS           1381L
#define ERROR_SECRET_TOO_LONG            1382L
#define ERROR_INTERNAL_DB_ERROR          1383L
#define ERROR_TOO_MANY_CONTEXT_IDS       1384L
#define ERROR_LOGON_TYPE_NOT_GRANTED     1385L
#define ERROR_NT_CROSS_ENCRYPTION_REQUIRED 1386L
#define ERROR_NO_SUCH_MEMBER             1387L
#define ERROR_INVALID_MEMBER             1388L
#define ERROR_TOO_MANY_SIDS              1389L
#define ERROR_LM_CROSS_ENCRYPTION_REQUIRED 1390L
#define ERROR_NO_INHERITANCE             1391L
#define ERROR_FILE_CORRUPT               1392L
#define ERROR_DISK_CORRUPT               1393L
#define ERROR_NO_USER_SESSION_KEY        1394L
#define ERROR_LICENSE_QUOTA_EXCEEDED     1395L
#define ERROR_INVALID_WINDOW_HANDLE      1400L
#define ERROR_INVALID_MENU_HANDLE        1401L
#define ERROR_INVALID_CURSOR_HANDLE      1402L
#define ERROR_INVALID_ACCEL_HANDLE       1403L
#define ERROR_INVALID_HOOK_HANDLE        1404L
#define ERROR_INVALID_DWP_HANDLE         1405L
#define ERROR_TLW_WITH_WSCHILD           1406L
#define ERROR_CANNOT_FIND_WND_CLASS      1407L
#define ERROR_WINDOW_OF_OTHER_THREAD     1408L
#define ERROR_HOTKEY_ALREADY_REGISTERED  1409L
#define ERROR_CLASS_ALREADY_EXISTS       1410L
#define ERROR_CLASS_DOES_NOT_EXIST       1411L
#define ERROR_CLASS_HAS_WINDOWS          1412L
#define ERROR_INVALID_INDEX              1413L
#define ERROR_INVALID_ICON_HANDLE        1414L
#define ERROR_PRIVATE_DIALOG_INDEX       1415L
#define ERROR_LISTBOX_ID_NOT_FOUND       1416L
#define ERROR_NO_WILDCARD_CHARACTERS     1417L
#define ERROR_CLIPBOARD_NOT_OPEN         1418L
#define ERROR_HOTKEY_NOT_REGISTERED      1419L
#define ERROR_WINDOW_NOT_DIALOG          1420L
#define ERROR_CONTROL_ID_NOT_FOUND       1421L
#define ERROR_INVALID_COMBOBOX_MESSAGE   1422L
#define ERROR_WINDOW_NOT_COMBOBOX        1423L
#define ERROR_INVALID_EDIT_HEIGHT        1424L
#define ERROR_DC_NOT_FOUND               1425L
#define ERROR_INVALID_HOOK_FILTER        1426L
#define ERROR_INVALID_FILTER_PROC        1427L
#define ERROR_HOOK_NEEDS_HMOD            1428L
#define ERROR_GLOBAL_ONLY_HOOK           1429L
#define ERROR_JOURNAL_HOOK_SET           1430L
#define ERROR_HOOK_NOT_INSTALLED         1431L
#define ERROR_INVALID_LB_MESSAGE         1432L
#define ERROR_SETCOUNT_ON_BAD_LB         1433L
#define ERROR_LB_WITHOUT_TABSTOPS        1434L
#define ERROR_DESTROY_OBJECT_OF_OTHER_THREAD 1435L
#define ERROR_CHILD_WINDOW_MENU          1436L
#define ERROR_NO_SYSTEM_MENU             1437L
#define ERROR_INVALID_MSGBOX_STYLE       1438L
#define ERROR_INVALID_SPI_VALUE          1439L
#define ERROR_SCREEN_ALREADY_LOCKED      1440L
#define ERROR_HWNDS_HAVE_DIFF_PARENT     1441L
#define ERROR_NOT_CHILD_WINDOW           1442L
#define ERROR_INVALID_GW_COMMAND         1443L
#define ERROR_INVALID_THREAD_ID          1444L
#define ERROR_NON_MDICHILD_WINDOW        1445L
#define ERROR_POPUP_ALREADY_ACTIVE       1446L
#define ERROR_NO_SCROLLBARS              1447L
#define ERROR_INVALID_SCROLLBAR_RANGE    1448L
#define ERROR_INVALID_SHOWWIN_COMMAND    1449L
#define ERROR_NO_SYSTEM_RESOURCES        1450L
#define ERROR_NONPAGED_SYSTEM_RESOURCES  1451L
#define ERROR_PAGED_SYSTEM_RESOURCES     1452L
#define ERROR_WORKING_SET_QUOTA          1453L
#define ERROR_PAGEFILE_QUOTA             1454L
#define ERROR_COMMITMENT_LIMIT           1455L
#define ERROR_MENU_ITEM_NOT_FOUND        1456L
#define ERROR_EVENTLOG_FILE_CORRUPT      1500L
#define ERROR_EVENTLOG_CANT_START        1501L
#define ERROR_LOG_FILE_FULL              1502L
#define ERROR_EVENTLOG_FILE_CHANGED      1503L
#define ERROR_INVALID_USER_BUFFER        1784L
#define ERROR_UNRECOGNIZED_MEDIA         1785L
#define ERROR_NO_TRUST_LSA_SECRET        1786L
#define ERROR_NO_TRUST_SAM_ACCOUNT       1787L
#define ERROR_TRUSTED_DOMAIN_FAILURE     1788L
#define ERROR_TRUSTED_RELATIONSHIP_FAILURE 1789L
#define ERROR_TRUST_FAILURE              1790L
#define ERROR_NETLOGON_NOT_STARTED       1792L
#define ERROR_ACCOUNT_EXPIRED            1793L
#define ERROR_REDIRECTOR_HAS_OPEN_HANDLES 1794L
#define ERROR_PRINTER_DRIVER_ALREADY_INSTALLED 1795L
#define ERROR_UNKNOWN_PORT               1796L
#define ERROR_UNKNOWN_PRINTER_DRIVER     1797L
#define ERROR_UNKNOWN_PRINTPROCESSOR     1798L
#define ERROR_INVALID_SEPARATOR_FILE     1799L
#define ERROR_INVALID_PRIORITY           1800L
#define ERROR_INVALID_PRINTER_NAME       1801L
#define ERROR_PRINTER_ALREADY_EXISTS     1802L
#define ERROR_INVALID_PRINTER_COMMAND    1803L
#define ERROR_INVALID_DATATYPE           1804L
#define ERROR_INVALID_ENVIRONMENT        1805L
#define ERROR_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT 1807L
#define ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT 1808L
#define ERROR_NOLOGON_SERVER_TRUST_ACCOUNT 1809L
#define ERROR_DOMAIN_TRUST_INCONSISTENT  1810L
#define ERROR_SERVER_HAS_OPEN_HANDLES    1811L
#define ERROR_RESOURCE_DATA_NOT_FOUND    1812L
#define ERROR_RESOURCE_TYPE_NOT_FOUND    1813L
#define ERROR_RESOURCE_NAME_NOT_FOUND    1814L
#define ERROR_RESOURCE_LANG_NOT_FOUND    1815L
#define ERROR_NOT_ENOUGH_QUOTA           1816L
#define ERROR_INVALID_TIME               1901L
#define ERROR_INVALID_FORM_NAME          1902L
#define ERROR_INVALID_FORM_SIZE          1903L
#define ERROR_ALREADY_WAITING            1904L
#define ERROR_PRINTER_DELETED            1905L
#define ERROR_INVALID_PRINTER_STATE      1906L
#define ERROR_PASSWORD_MUST_CHANGE       1907L
#define ERROR_DOMAIN_CONTROLLER_NOT_FOUND 1908L
#define ERROR_ACCOUNT_LOCKED_OUT         1909L
#define ERROR_NO_BROWSER_SERVERS_FOUND   6118L
#define ERROR_INVALID_PIXEL_FORMAT       2000L
#define ERROR_BAD_DRIVER                 2001L
#define ERROR_INVALID_WINDOW_STYLE       2002L
#define ERROR_METAFILE_NOT_SUPPORTED     2003L
#define ERROR_TRANSFORM_NOT_SUPPORTED    2004L
#define ERROR_CLIPPING_NOT_SUPPORTED     2005L
#define ERROR_UNKNOWN_PRINT_MONITOR      3000L
#define ERROR_PRINTER_DRIVER_IN_USE      3001L
#define ERROR_SPOOL_FILE_NOT_FOUND       3002L
#define ERROR_SPL_NO_STARTDOC            3003L
#define ERROR_SPL_NO_ADDJOB              3004L
#define ERROR_PRINT_PROCESSOR_ALREADY_INSTALLED 3005L
#define ERROR_PRINT_MONITOR_ALREADY_INSTALLED 3006L
#define ERROR_WINS_INTERNAL              4000L
#define ERROR_CAN_NOT_DEL_LOCAL_WINS     4001L
#define ERROR_STATIC_INIT                4002L
#define ERROR_INC_BACKUP                 4003L
#define ERROR_FULL_BACKUP                4004L
#define ERROR_REC_NON_EXISTENT           4005L
#define ERROR_RPL_NOT_ALLOWED            4006L

#ifndef NOAPIPROTO

DWORD	WINAPI GetLastError(VOID);
VOID	WINAPI SetLastError(DWORD);
VOID	WINAPI SetLastErrorEx(DWORD, DWORD);

#endif	/* NOAPIPROTO */

/* Critical Section ******************************************************** */

#ifndef NOAPIPROTO

VOID	WINAPI InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
VOID	WINAPI DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
VOID	WINAPI EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
VOID	WINAPI LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

#endif	/* NOAPIPROTO */

/* Thread Local Storage **************************************************** */

#ifndef NOAPIPROTO

DWORD	WINAPI TlsAlloc(VOID);
BOOL	WINAPI TlsFree(DWORD dwTlsIndex);
LPVOID	WINAPI TlsGetValue(DWORD dwTlsIndex);
BOOL	WINAPI TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue);

#endif	/* NOAPIPROTO */

/* Interlocked Variable **************************************************** */

#ifndef NOAPIPROTO

LONG	WINAPI InterlockedDecrement(LPLONG);
LONG	WINAPI InterlockedIncrement(LPLONG);

#endif	/* NOAPIPROTO */

/* (WIN32) Event *********************************************************** */

/* function prototypes */

#ifndef NOAPIPROTO

HANDLE	WINAPI
CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes,
	BOOL bManualReset,
	BOOL bInitialState,
	LPCSTR lpName);

BOOL	WINAPI
ResetEvent(HANDLE hEvent);

BOOL	WINAPI
SetEvent(HANDLE hEvent);

BOOL	WINAPI
PulseEvent(HANDLE hEvent);

#endif	/* NOAPIPROTO */

/* (WIN32) Process Creation ************************************************ */

/* dwCreationFlag values */

#define DEBUG_PROCESS			0x00000001
#define DEBUG_ONLY_THIS_PROCESS		0x00000002
#define CREATE_SUSPENDED		0x00000004
#define DETACHED_PROCESS		0x00000008
#define CREATE_NEW_CONSOLE		0x00000010
#define NORMAL_PRIORITY_CLASS		0x00000020
#define IDLE_PRIORITY_CLASS		0x00000040
#define HIGH_PRIORITY_CLASS		0x00000080
#define REALTIME_PRIORITY_CLASS		0x00000100
#define CREATE_NEW_PROCESS_GROUP	0x00000200
#define CREATE_UNICODE_ENVIRONMENT	0x00000400
#define CREATE_SEPARATE_WOW_VDM		0x00000800
#define CREATE_SHARED_WOW_VDM		0x00001000
#define CREATE_DEFAULT_ERROR_MODE	0x04000000
#define CREATE_NO_WINDOW		0x08000000
#define PROFILE_USER			0x10000000
#define PROFILE_KERNEL			0x20000000
#define PROFILE_SERVER			0x40000000

/* function prototypes */

#ifndef NOAPIPROTO

#endif	/* NOAPIPROTO */

/* (WIN32) Synchronization ************************************************* */

/* WaitForSingleObject*() return code */

#define WAIT_FAILED		((DWORD)0xFFFFFFFF)
#define WAIT_OBJECT_0		((STATUS_WAIT_0) + 0)
#define WAIT_ABANDONED		((STATUS_ABANDONED_WAIT_0) + 0)
#define WAIT_ABANDONED_0	((STATUS_ABANDONED_WAIT_0) + 0)
#define WAIT_TIMEOUT		STATUS_TIMEOUT
#define WAIT_IO_COMPLETION	STATUS_USER_APC

/* function prototypes */

#ifndef NOAPIPROTO

DWORD	WINAPI
WaitForSingleObject(HANDLE hObject, DWORD dwTimeOut);

DWORD	WINAPI
WaitForSingleObjectEx(HANDLE hObject, DWORD dwTimeOut, BOOL bAlertable);

DWORD	WINAPI
WaitForMultipleObjects(DWORD dwCount, CONST HANDLE *lpHandles,
	BOOL bWaitAll, DWORD dwMilliseconds);

DWORD	WINAPI
WaitForMultipleObjectsEx(DWORD dwCount, CONST HANDLE *lpHandles,
	BOOL bWaitAll, DWORD dwMilliseconds, BOOL bAlertable);

#endif	/* NOAPIPROTO */

/* (WIN32) File Security *************************************************** */

/* function prototypes */

#ifndef NOAPIPROTO

BOOL	WINAPI
GetFileSecurity(LPCSTR lpszFile,
	SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR psd,
	DWORD dw, LPDWORD lp);

BOOL	WINAPI
SetFileSecurity(LPCSTR lpszFile,
	SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR psd);

#endif	/* NOAPIPROTO */

/* (WIN32) Mutex *********************************************************** */

/* function prototypes */

#ifndef NOAPIPROTO

HANDLE	WINAPI
CreateMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,
	BOOL bInitialOwner,
	LPCSTR lpName);

HANDLE	WINAPI
OpenMutex(DWORD dwDesiredAccess,
	BOOL bInheritHandle,
	LPCSTR lpName);

BOOL	WINAPI
ReleaseMutex(HANDLE hMutex);

#endif	/* NOAPIPROTO */

/* (WIN32) Semaphore ******************************************************* */

/* function prototypes */

#ifndef NOAPIPROTO

HANDLE	WINAPI
CreateSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
	LONG lInitialCount,
	LONG lMaximumCount,
	LPCSTR lpName);

HANDLE	WINAPI
OpenSemaphore(DWORD dwDesiredAccess,
	BOOL bInheritHandle,
	LPCSTR lpName);

BOOL	WINAPI
ReleaseSemaphore(HANDLE hSemaphore,
	LONG lpReleaseCount, LPLONG lpPreviousCount);

#endif	/* NOAPIPROTO */

#endif      /* Win_Kernel__h */
