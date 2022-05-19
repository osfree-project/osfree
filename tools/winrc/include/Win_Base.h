/*  Win_Base.h	1.19
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

#ifndef Win_Base__h
#define Win_Base__h

/*
** Define API decoration for direct importing of DLL references.
*/

#if !defined(_ADVAPI32_)
#define WINADVAPI DECLSPEC_IMPORT
#else
#define WINADVAPI
#endif

#if !defined(_KERNEL32_)
#define WINBASEAPI DECLSPEC_IMPORT
#else
#define WINBASEAPI
#endif

#define STD_INPUT_HANDLE    (DWORD)-10
#define STD_OUTPUT_HANDLE   (DWORD)-11
#define STD_ERROR_HANDLE      (DWORD)-12

typedef struct _OVERLAPPED {
    DWORD   Internal;
    DWORD   InternalHigh;
    DWORD   Offset;
    DWORD   OffsetHigh;
    HANDLE  hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef VOID (WINAPI *LPOVERLAPPED_COMPLETION_ROUTINE)(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransfered,
    LPOVERLAPPED lpOverlapped
    );

#ifndef _FILETIME_
#define _FILETIME_
typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;
#endif /* _FILETIME_ */

#ifndef _SYSTEMTIME_
#define _SYSTEMTIME_
typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;
#endif /* _SYSTEMTIME_ */

typedef struct _WIN32_FIND_DATA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    CHAR   cFileName[ _MAX_PATH ];
    CHAR   cAlternateFileName[ 14 ];
} WIN32_FIND_DATA, *PWIN32_FIND_DATA, *LPWIN32_FIND_DATA;

/*
 *  Internal (TWIN-only) structure used for passing file information
 *  between the lowest driver levels, up through the MFS layer, and
 *  finally to the library itself, as well in the reverse direction.
 *
 *  These fields are mainly driven by the contents/needs for the
 *  Win32 structures WIN32_FIND_INFO and BY_HANDLE_FILE_INFORMATION,
 *  as well as any other structures that need stat-type information.
 *  Feel free to add fields as necessary here, but make sure that
 *  the lower Driver levels are changed to support them.
 */
typedef struct tagTWIN_FILEINFO
{
    DWORD       dwFileAttributes;
    FILETIME    ftCreationTime;
    FILETIME    ftLastAccessTime;
    FILETIME    ftLastWriteTime;
    DWORD       dwVolumeSerialNumber;
    DWORD       nFileSizeHigh;
    DWORD       nFileSizeLow;
    DWORD       nNumberOfLinks;
    DWORD       nFileIndexHigh;
    DWORD       nFileIndexLow;
} TWIN_FILEINFO, *LPTWIN_FILEINFO;

typedef struct _PROCESS_HEAP_ENTRY {
    PVOID lpData;
    DWORD cbData;
    BYTE cbOverhead;
    BYTE iRegionIndex;
    WORD wFlags;
    union {
        struct {
            HANDLE hMem;
            DWORD dwReserved[ 3 ];
        } Block;
        struct {
            DWORD dwCommittedSize;
            DWORD dwUnCommittedSize;
            LPVOID lpFirstBlock;
            LPVOID lpLastBlock;
        } Region;
    } BR;
} PROCESS_HEAP_ENTRY, *LPPROCESS_HEAP_ENTRY, *PPROCESS_HEAP_ENTRY;

typedef struct _TIME_ZONE_INFORMATION {
    LONG Bias;
    WCHAR StandardName[ 32 ];
    SYSTEMTIME StandardDate;
    LONG StandardBias;
    WCHAR DaylightName[ 32 ];
    SYSTEMTIME DaylightDate;
    LONG DaylightBias;
} TIME_ZONE_INFORMATION, *PTIME_ZONE_INFORMATION, *LPTIME_ZONE_INFORMATION;

#ifdef TWIN32
UINT WINAPI GetDriveType( LPTSTR );
UINT WINAPI GetTempFileName( LPCTSTR, LPCTSTR, UINT, LPTSTR );
#endif /* TWIN32 */

#define INVALID_HANDLE_VALUE (HANDLE)-1

#define FILE_BEGIN          0
#define FILE_CURRENT        1
#define FILE_END            2

#define CREATE_NEW          1
#define CREATE_ALWAYS       2
#define OPEN_EXISTING       3
#define OPEN_ALWAYS         4
#define TRUNCATE_EXISTING   5

#define FILE_TYPE_UNKNOWN   0x0000
#define FILE_TYPE_DISK      0x0001
#define FILE_TYPE_CHAR      0x0002
#define FILE_TYPE_PIPE      0x0003
#define FILE_TYPE_REMOTE    0x0004

/* GetBinaryType() return values */
#define SCS_32BIT_BINARY    0
#define SCS_DOS_BINARY      1
#define SCS_WOW_BINARY      2
#define SCS_PIF_BINARY      3
#define SCS_POSIX_BINARY    4
#define SCS_OS216_BINARY    5

#define MOVEFILE_REPLACE_EXISTING   0x00000001
#define MOVEFILE_COPY_ALLOWED       0x00000002
#define MOVEFILE_DELAY_UNTIL_REBOOT 0x00000004

#define FS_CASE_IS_PRESERVED            FILE_CASE_PRESERVED_NAMES
#define FS_CASE_SENSITIVE               FILE_CASE_SENSITIVE_SEARCH
#define FS_UNICODE_STORED_ON_DISK       FILE_UNICODE_ON_DISK
#define FS_PERSISTENT_ACLS              FILE_PERSISTENT_ACLS
#define FS_VOL_IS_COMPRESSED            FILE_VOLUME_IS_COMPRESSED
#define FS_FILE_COMPRESSION             FILE_FILE_COMPRESSION

/* File32 function prototypes */
HANDLE 
CreateFile(LPCTSTR lpszName, DWORD fdwAccess, DWORD fdwShare,
           LPSECURITY_ATTRIBUTES lpsa, DWORD fdwCreate, 
           DWORD fdwAttr, HANDLE hTemplate);

BOOL 
DeleteFile(LPTSTR lpszName);

BOOL 
CopyFile(LPTSTR lpExisting, LPTSTR lpNew, BOOL fFail);

BOOL
MoveFile(LPCTSTR lpExisting, LPCTSTR lpNew);

BOOL
MoveFileEx(LPCTSTR lpExisting, LPCTSTR lpNew, DWORD dwFlags);

BOOL
CloseHandle(HANDLE hObject);

BOOL
ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nBytesToRead,
         LPDWORD lpBytesRead, LPOVERLAPPED lpOverlapped);

BOOL
ReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nBytesToRead,
           LPOVERLAPPED lpOverlapped, 
           LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

BOOL
WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nBytesToWrite,
          LPDWORD lpBytesWritten, LPOVERLAPPED lpOverlapped);

BOOL
WriteFileEx(HANDLE hFile, LPCVOID lpBuffer, DWORD nBytesToWrite,
            LPOVERLAPPED lpOverlapped,
            LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

DWORD
SetFilePointer(HANDLE hFile, LONG lDistance, PLONG lpDistance,
               DWORD dwMoveMethod);

BOOL
SetEndOfFile(HANDLE hFile);

BOOL
FlushFileBuffers(HANDLE hFile);

BOOL
LockFile(HANDLE hFile, DWORD dwOffsetLow, DWORD dwOffsetHigh,
         DWORD nBytesToLockLow, DWORD nBytesToLockHigh);

BOOL
UnlockFile(HANDLE hFile, DWORD dwOffsetLow, DWORD dwOffsetHigh,
           DWORD nBytesToUnlockLow, DWORD nBytesToUnlockHigh);

BOOL
UnlockFileEx(HANDLE hFile, DWORD dwReserved,
             DWORD nBytesToUnlockLow, DWORD nBytesToUnlockHigh,
             LPOVERLAPPED lpOverlapped);

HANDLE
FindFirstFile(LPCTSTR lpszSearchFile, LPWIN32_FIND_DATA lpffd);

BOOL
FindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATA lpffd);

BOOL
FindClose(HANDLE hFindFile);

#ifdef TWIN32
UINT
GetTempFileName(LPCTSTR lpszPath, LPCTSTR lpszPrefix, 
                UINT uUnique, LPTSTR lpszTempFile);
#endif /* TWIN32 */

DWORD
GetTempPath(DWORD cchBuffer, LPTSTR lpszTempFile);

DWORD
GetFileAttributes(LPCTSTR lpszFile);

BOOL
SetFileAttributes(LPCTSTR lpFileName, DWORD dwAttributes);

DWORD
GetFileType(HANDLE hFile);

DWORD
GetFileSize(HANDLE hFile, LPDWORD lpdwFileSizeHigh);

DWORD
GetFullPathName(LPCTSTR lpszFile, DWORD cchPath, 
                LPTSTR lpszPath, LPTSTR *ppszFilePart);

BOOL
CreateDirectory(LPTSTR lpszPath, LPSECURITY_ATTRIBUTES lpsa);

BOOL
CreateDirectoryEx(LPCTSTR lpTemplate, LPCTSTR lpNew, LPSECURITY_ATTRIBUTES lpsa);

BOOL
RemoveDirectory(LPCTSTR lpPath);

DWORD
GetCurrentDirectory(DWORD cchCurDir, LPTSTR lpszCurDir);

BOOL
SetCurrentDirectory(LPCTSTR lpPath);

#ifdef LATER
BOOL
GetBinaryType(LPCTSTR lpAppName, LPDWORD lpBinType);

HANDLE
FindFirstChangeNotification(LPTSTR lpszPath, BOOL fWatchSubTree, DWORD fdwFilter);

BOOL
FindNextChangeNotification(HANDLE hChange);

BOOL
FindCloseChangeNotification(HANDLE hChange);
#endif /* LATER */

BOOL
GetVolumeInformation(LPCTSTR lpRootPath, LPTSTR lpVolumeName,
                     DWORD nVolumeNameSize, LPDWORD lpSerialNo,
                     LPDWORD lpMaxLength, LPDWORD lpFlags,
                     LPTSTR lpFileSysName, DWORD nFileSysNameSize);

BOOL
GetDiskFreeSpace(LPCTSTR lpszRootPath, LPDWORD lpSectorsPerCluster,
                 LPDWORD lpBytesPerSector, LPDWORD lpFreeClusters,
                 LPDWORD lpClusters);

#ifdef TWIN32
UINT
GetDriveType(LPTSTR lpszRootPath);
#endif /* TWIN32 */

DWORD
GetLogicalDrives(void);

DWORD
GetLogicalDriveStrings(DWORD cchBuffer, LPTSTR lpszBuffer);

BOOL
SetFileTime(HANDLE hFile, LPFILETIME lpftCreation, LPFILETIME lpftAccess,
	    LPFILETIME lpftWrite);

BOOL	WINAPI
GetFileTime(HANDLE hFile, LPFILETIME lpftCreation, LPFILETIME lpftAccess,
	    LPFILETIME lpftWrite);

DWORD	WINAPI
SearchPath(LPCTSTR, LPCTSTR, LPCTSTR, DWORD, LPTSTR, LPTSTR *);

/* Win32 time functions */
BOOL FileTimeToSystemTime(LPFILETIME lpft, LPSYSTEMTIME lpst);
BOOL SystemTimeToFileTime(LPSYSTEMTIME lpst, LPFILETIME lpft);
BOOL FileTimeToLocalFileTime(LPFILETIME lpft, LPFILETIME lpftLocal);
BOOL LocalFileTimeToFileTime(LPFILETIME lpftLocal, LPFILETIME lpft);
void GetSystemTime(LPSYSTEMTIME lpst);
void GetLocalTime(LPSYSTEMTIME lpst);
BOOL SetSystemTime(LPSYSTEMTIME lpst);
BOOL SetLocalTime(LPSYSTEMTIME lpst);
BOOL SetTimeZoneInformation(LPTIME_ZONE_INFORMATION lptzi);
BOOL GetTimeZoneInformation(LPTIME_ZONE_INFORMATION lptzi);
BOOL DosDateTimeToFileTime(WORD wDOSDate, WORD wDOSTime, LPFILETIME lpft);
BOOL FileTimeToDosDateTime(LPFILETIME lpft, LPWORD lpwDOSDate, LPWORD lpwDOSTime);
LONG CompareFileTime(LPFILETIME lpft1, LPFILETIME lpft2);
BOOL SetSystemTimeAdjustment(DWORD dwTimeAdjustment, BOOL bTimeAdjustmentDisabled);
BOOL GetSystemTimeAdjustment(PDWORD lpTimeAdjustment, PDWORD lpTimeIncrement, PBOOL lpTimeAdjustmentDisabled);
void GetSystemTimeAsFileTime(LPFILETIME lpft);
BOOL SystemTimeToTzSpecificLocalTime(LPTIME_ZONE_INFORMATION lpTimeZoneInformation,
				     LPSYSTEMTIME lpUCT, LPSYSTEMTIME lpst);

/* PROCESS_INFORMATION */

typedef struct _PROCESS_INFORMATION {
	HANDLE	hProcess;
	HANDLE	hThread;
	DWORD	dwProcessId;
	DWORD	dwThreadId;
} PROCESS_INFORMATION;
typedef PROCESS_INFORMATION		*PPROCESS_INFORMATION;
typedef PROCESS_INFORMATION NEAR	*NPPROCESS_INFORMATION;
typedef PROCESS_INFORMATION FAR		*LPPROCESS_INFORMATION;

/* STARTUP_INFO */

#ifndef UNICODE_ONLY
typedef struct _STARTUPINFOA {
	DWORD	cb;
	LPSTR	lpReserved;
	LPSTR	lpDesktop;
	LPSTR	lpTitle;
	DWORD	dwX;
	DWORD	dwY;
	DWORD	dwXSize;
	DWORD	dwYSize;
	DWORD	dwXCountChars;
	DWORD	dwYCountChars;
	DWORD	dwFillAttribute;
	DWORD	dwFlags;
	WORD	wShowWindow;
	WORD	cbReserved2;
	LPBYTE	lpReserved2;
	HANDLE	hStdInput;
	HANDLE	hStdOutput;
	HANDLE	hStdError;
} STARTUPINFOA;
typedef STARTUPINFOA		*PSTARTUPINFOA;
typedef STARTUPINFOA NEAR	*NPSTARTUPINFOA;
typedef STARTUPINFOA FAR	*LPSTARTUPINFOA;
#endif	/* UNICODE_ONLY */

#ifndef ANSI_ONLY
typedef struct _STARTUPINFOW {
	DWORD	cb;
	LPWSTR	lpReserved;
	LPWSTR	lpDesktop;
	LPWSTR	lpTitle;
	DWORD	dwX;
	DWORD	dwY;
	DWORD	dwXSize;
	DWORD	dwYSize;
	DWORD	dwXCountChars;
	DWORD	dwYCountChars;
	DWORD	dwFillAttribute;
	DWORD	dwFlags;
	WORD	wShowWindow;
	WORD	cbReserved2;
	LPBYTE	lpReserved2;
	HANDLE	hStdInput;
	HANDLE	hStdOutput;
	HANDLE	hStdError;
} STARTUPINFOW;
typedef STARTUPINFOW		*PSTARTUPINFOW;
typedef STARTUPINFOW NEAR	*NPSTARTUPINFOW;
typedef STARTUPINFOW FAR	*LPSTARTUPINFOW;
#endif	/* ANSI_ONLY */

#ifdef UNICODE
typedef STARTUPINFOW		STARTUPINFO;
typedef NPSTARTUPINFOW		NPSTARTUPINFO;
typedef LPSTARTUPINFOW		LPSTARTUPINFO;
#else
typedef STARTUPINFOA		STARTUPINFO;
typedef NPSTARTUPINFOA		NPSTARTUPINFO;
typedef LPSTARTUPINFOA		LPSTARTUPINFO;
#endif	/* UNICODE */

/* OSVERSIONINFO */

#ifndef UNICODE_ONLY
typedef struct _OSVERSIONINFOA {
	DWORD	dwOSVersionInfoSize;
	DWORD	dwMajorVersion;
	DWORD	dwMinorVersion;
	DWORD	dwBuildNumber;
	DWORD	dwPlatformId;
	CHAR	szCSDVersion[128];
} OSVERSIONINFOA;
typedef OSVERSIONINFOA		*POSVERSIONINFOA;
typedef OSVERSIONINFOA NEAR	*NPOSVERSIONINFOA;
typedef OSVERSIONINFOA FAR	*LPOSVERSIONINFOA;
#endif	/* UNICODE_ONLY */

#ifndef ANSI_ONLY
typedef struct _OSVERSIONINFOW {
	DWORD	dwOSVersionInfoSize;
	DWORD	dwMajorVersion;
	DWORD	dwMinorVersion;
	DWORD	dwBuildNumber;
	DWORD	dwPlatformId;
	WCHAR	szCSDVersion[128];
} OSVERSIONINFOW;
typedef OSVERSIONINFOW		*POSVERSIONINFOW;
typedef OSVERSIONINFOW NEAR	*NPOSVERSIONINFOW;
typedef OSVERSIONINFOW FAR	*LPOSVERSIONINFOW;
#endif	/* ANSI_ONLY */

#ifdef UNICODE
typedef OSVERSIONINFOW		OSVERSIONINFO;
typedef POSVERSIONINFOW		POSVERSIONINFO;
typedef NPOSVERSIONINFOW 	NPOSVERSIONINFO;
typedef LPOSVERSIONINFOW	LPOSVERSIONINFO;
#else
typedef OSVERSIONINFOA		OSVERSIONINFO;
typedef POSVERSIONINFOA		POSVERSIONINFO;
typedef NPOSVERSIONINFOA	NPOSVERSIONINFO;
typedef LPOSVERSIONINFOA	LPOSVERSIONINFO;
#endif	/* UNICODE */

/* dwPlatformId member of OSVERSIONINFO */

#define VER_PLATFORM_WIN32s		0
#define VER_PLATFORM_WIN32_WINDOWS	1
#define VER_PLATFORM_WIN32_NT		2

/* SYSTEM_INFO */

typedef struct _SYSTEM_INFO {
    union {
        DWORD		dwOemId;	/* obsolete */
        struct {
            WORD	wProcessorArchitecture;
            WORD	wReserved;
        } used;
    } noname;
    DWORD		dwPageSize;
    LPVOID		lpMinimumApplicationAddress;
    LPVOID		lpMaximumApplicationAddress;
    DWORD		dwActiveProcessorMask;
    DWORD		dwNumberOfProcessors;
    DWORD		dwProcessorType;
    DWORD		dwAllocationGranularity;
    WORD		wProcessorLevel;
    WORD		wProcessorRevision;
} SYSTEM_INFO;
typedef SYSTEM_INFO		*PSYSTEM_INFO;
typedef SYSTEM_INFO NEAR	*NPSYSTEM_INFO;
typedef SYSTEM_INFO FAR		*LPSYSTEM_INFO;

BOOL
CreateProcess(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    );

/* THREAD_PRIORITY */

#define THREAD_PRIORITY_NORMAL		0
#define THREAD_PRIORITY_LOWEST		(THREAD_BASE_PRIORITY_MIN)
#define THREAD_PRIORITY_HIGHEST		(THREAD_BASE_PRIORITY_MAX)
#define THREAD_PRIORITY_BELOW_NORMAL	(THREAD_PRIORITY_LOWEST + 1)
#define THREAD_PRIORITY_ABOVE_NORMAL	(THREAD_PRIORITY_HIGHEST - 1)
#define THREAD_PRIORITY_ERROR_RETURN	(MAXLONG)

#define THREAD_PRIORITY_TIME_CRITICAL	(THREAD_BASE_PRIORITY_LOWRT)
#define THREAD_PRIORITY_IDLE		(THREAD_BASE_PRIORITY_IDLE)

#define THREAD_BASE_PRIORITY_LOWRT	15
#define THREAD_BASE_PRIORITY_MAX	2
#define THREAD_BASE_PRIORITY_MIN	-2
#define THREAD_BASE_PRIORITY_IDLE	-15

/* CRITICAL_SECTION */

typedef RTL_CRITICAL_SECTION		CRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION		PCRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION		LPCRITICAL_SECTION;

typedef RTL_CRITICAL_SECTION_DEBUG	CRITICAL_SECTION_DEBUG;
typedef PRTL_CRITICAL_SECTION_DEBUG	PCRITICAL_SECTION_DEBUG;
typedef PRTL_CRITICAL_SECTION_DEBUG	LPCRITICAL_SECTION_DEBUG;

/* FORMAT_MESSAGE */

#define FORMAT_MESSAGE_ALLOCATE_BUFFER	0x00000100
#define FORMAT_MESSAGE_IGNORE_INSERTS	0x00000200
#define FORMAT_MESSAGE_FROM_STRING	0x00000400
#define FORMAT_MESSAGE_FROM_HMODULE	0x00000800
#define FORMAT_MESSAGE_FROM_SYSTEM	0x00001000
#define FORMAT_MESSAGE_ARGUMENT_ARRAY	0x00002000
#define FORMAT_MESSAGE_MAX_WIDTH_MASK	0x000000FF

/* TIMEOUT */

#define INFINITE			0xFFFFFFFF

BOOL WINAPI Beep( DWORD dwFreq, DWORD dwDuration );

#define PAGE_NOACCESS           0x01
#define PAGE_READONLY           0x02
#define PAGE_READWRITE          0x04
#define PAGE_WRITECOPY          0x08
#define PAGE_EXECUTE            0x10
#define PAGE_EXECUTE_READ       0x20
#define PAGE_EXECUTE_READWRITE  0x40
#define PAGE_EXECUTE_WRITECOPY  0x80
#define PAGE_GUARD              0x100
#define PAGE_NOCACHE            0x200

#define MEM_COMMIT              0x00001000
#define MEM_RESERVE             0x00002000
#define MEM_DECOMMIT            0x00004000
#define MEM_RELEASE             0x00008000
#define MEM_FREE                0x00010000
#define MEM_PRIVATE             0x00020000
#define MEM_MAPPED              0x00040000
#define MEM_TOP_DOWN            0x00100000



#endif      /* Win_Base__h */
