/* F_platform.h */
/* Common platform abstraction for FreePM: PMGPI client + mini server.
 * Derived from gpi_platform.h; only the missing server-side bits
 * (bundles, masks, extra Dos* emulation, extra Win32 API imports)
 * are added. Everything else kept verbatim. */

#ifndef F_PLATFORM_H
#define F_PLATFORM_H

#if defined(__OS2__) || defined(__OS2V2__)

  #define INCL_GPI
  #define INCL_DEV
  #define INCL_DOS
  #define INCL_DOSMODULEMGR
  #include <os2.h>
  #define EXTERN extern "C"

#elif defined(__WIN32__) || defined(_WIN32)

#include <string.h>

#ifdef __cplusplus
      extern "C" {
#endif

  /* ---------- Базовые типы ---------- */
  typedef unsigned long DWORD;
  typedef unsigned int  UINT;
  typedef unsigned long ULONG;
  typedef long          LONG;
  typedef unsigned short USHORT;
  typedef short         SHORT;
  typedef USHORT       *PUSHORT;  
  typedef SHORT        *PSHORT;   
  typedef unsigned char BYTE;
  typedef unsigned char UCHAR;
  typedef char          CHAR;
  typedef void         *PVOID;
  typedef char         *PCH;
  typedef unsigned char *PBYTE;
  typedef LONG         *PLONG;
  typedef ULONG        *PULONG;
  typedef int           BOOL;
  typedef BOOL         *PBOOL;
  typedef LONG          FIXED;
  typedef FIXED        *PFIXED;
  typedef char         *PSTR8;
  typedef unsigned int  SIZE_T;
  typedef unsigned long APIRET;
  typedef int           INT;
  typedef unsigned long ULONG32;
  typedef long          LONG32;
  typedef unsigned long MPARAM;
  typedef unsigned long MRESULT;
  typedef ULONG         COLOR;

  /* OS/2-compatible pointer to a generic procedure (for DosQueryProcAddr). */
  typedef void (*PFN)(void);

  #ifndef TRUE
  #define TRUE 1
  #endif
  #ifndef FALSE
  #define FALSE 0
  #endif
  #ifndef NULL
  #define NULL ((void*)0)
  #endif

  /* ---------- Хэндлы ---------- */
  typedef void *HAB;
  typedef void *HDC;
  typedef void *HPS;
  typedef void *HWND;
  typedef void *HBITMAP;
  typedef void *HRGN;
  typedef void *HPAL;
  typedef void *HMF;
  typedef void *HMODULE;
  typedef void *HANDLE;
  typedef void *HMQ;
  typedef HANDLE HMTX;
  typedef void *HINSTANCE;
  typedef void *LPVOID;

  /* ---------- Строковые и указательные типы ---------- */
  typedef const char   *PCSZ;
  typedef char         *PSZ;
  typedef void         *PBUNDLE;
  typedef HRGN         *PHRGN;

  /* ---------- Конвенции вызова ---------- */
  /* Custom calling convention: __stdcall stack cleanup, no name
     decoration. Prevents _GpiSetColor@8 / _PmcPostCommand@8, so the
     DLL exports match the .def entries and the client resolves the
     PMCLIENT.LIB imports. Same pragma as F_platform.hpp. */
    #pragma aux __estdcall "*" \
               parm routine [] \
               value struct struct caller [] \
               modify [eax ecx edx]

    #undef EXPENTRY
    #define EXPENTRY __declspec(__pragma("__estdcall"))

  #ifndef APIENTRY
    #define APIENTRY   __declspec(__pragma("__estdcall"))
  #endif
  #ifndef APIENTRY16
    #define APIENTRY16 far pascal
  #endif

  #define EXTERN extern "C" 

  #ifndef WINAPI
    #define WINAPI __stdcall
  #endif

  /* ---------- Коды ошибок GPI ---------- */
  #define GPI_ERROR 0
  #define GPI_OK 1
  #define GPI_ALTERROR (-1)

  /* ---------- PS_FORMAT / PS_UNITS / PS_NORESET ---------- */
    #define PS_UNITS_MASK     0x0000000FL
    #define PU_ARBITRARY      0x00000000L
    #define PU_PELS           0x00000001L
    #define PU_LOMETRIC       0x00000002L
    #define PU_HIMETRIC       0x00000003L
    #define PU_LOENGLISH      0x00000004L
    #define PU_HIENGLISH      0x00000005L
    #define PU_TWIPS          0x00000006L

  #define PS_FORMAT_MASK    0x0000FF00L
  #define GPIF_DEFAULT      0x00000000L
  #define GPIF_LONG         0x00000100L
  #define GPIF_SHORT        0x00000200L
  #define PS_NORESET        0x80000000L

  /* ---------- Константы DosAllocMem ---------- */
  #define PAG_COMMIT     0x00010000
  #define PAG_READ       0x00000001
  #define PAG_WRITE      0x00000004

  /* ---------- Константы BITMAPINFOHEADER2 ---------- */
    #define BCA_UNCOMP          0L
    #define BRU_METRIC          0L
    #define BRA_BOTTOMUP        0L
    #define BRH_NOTHALFTONED    0L
    #define BCE_RGB             0L

  /* ---------- Константы Win32 API ---------- */
    #define MEM_COMMIT     0x1000
    #define MEM_RELEASE    0x8000
    #define PAGE_READWRITE 0x04
    #define SYNCHRONIZE    0x00100000L
    #define MUTEX_MODIFY_STATE      0x0001L
    #define SEMAPHORE_MODIFY_STATE  0x0002L

  /* ---------- OS/2 GPI constants (Win32 fallback) ---------- */
#define CAPS_WIDTH   8L
#define CAPS_HEIGHT  9L

  /* ---------- Paths (Win32 fallback) ---------- */
  #define MPATH_STROKE          6L
  #define FPATH_ALTERNATE       0L
  #define FPATH_WINDING         2L
  #define FPATH_INCL            0L
  #define FPATH_EXCL            8L
  #define SCP_ALTERNATE         0L
  #define SCP_WINDING           2L
  #define SCP_AND               4L
  #define SCP_RESET             0L
  #define SCP_INCL              0L
  #define SCP_EXCL              8L

   #define DM_ERROR                        0L
   #define DM_DRAW                         1L
   #define DM_RETAIN                       2L
   #define DM_DRAWANDRETAIN                3L

  #define ATTR_DETECTABLE      1L
  #define ATTR_VISIBLE         2L
  #define ATTR_CHAINED         6L
  #define ATTR_DYNAMIC         8L
  #define ATTR_FASTCHAIN       9L
  #define ATTR_PROP_DETECTABLE 10L
  #define ATTR_PROP_VISIBLE    11L
  #define ATTR_OFF             0L
  #define ATTR_ON              1L

  #define DCTL_ERASE           1L
  #define DCTL_DISPLAY         2L
  #define DCTL_BOUNDARY        3L
  #define DCTL_DYNAMIC         4L
  #define DCTL_CORRELATE       5L
  #define DCTL_OFF             0L
  #define DCTL_ON              1L

  #define SDW_OFF              0L
  #define SDW_ON               1L

/* ---------- DevOpenDC output device types (Win32 fallback) ---------- */

  #define OD_QUEUED            0L
  #define OD_DIRECT            1L
  #define OD_INFO              2L
  #define OD_MEMORY            3L
  #define OD_METAFILE          4L
  #define OD_METAFILE_NOQUERY  5L

  /* ---------- Segment priority (Win32 fallback) ---------- */
  #define LOWER_PRI            (-1L)
  #define HIGHER_PRI              1L

  /* ---------- Foreground/background mix (Win32 fallback) ---------- */
  #define FM_XOR               4L    /* PMGPI.H */
  #define BM_LEAVEALONE        5L    /* PMGPI.H */

  /* ---------- MAKELONG (Win32) ---------- */
  #ifndef MAKELONG
    #define MAKELONG(a, b) \
        ((LONG)(((USHORT)(a)) | ((ULONG)((USHORT)(b)) << 16)))
  #endif

  /* ---------- Win32 API ---------- */
  __declspec(dllimport) PVOID __stdcall VirtualAlloc(PVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
  __declspec(dllimport) DWORD __stdcall VirtualFree(PVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
  __declspec(dllimport) HMODULE __stdcall GetModuleHandleA(const char *lpModuleName);
  __declspec(dllimport) PVOID __stdcall GetProcAddress(HMODULE hModule, const char *lpProcName);
  __declspec(dllimport) void __stdcall OutputDebugStringA(const char *lpOutputString);
  __declspec(dllimport) HMODULE __stdcall LoadLibraryA(const char *lpLibFileName);
  __declspec(dllimport) BOOL __stdcall FreeLibrary(HMODULE hLibModule);
  __declspec(dllimport) HANDLE __stdcall CreateMutexA(void *lpMutexAttributes, BOOL bInitialOwner, const char *lpName);
  __declspec(dllimport) HANDLE __stdcall OpenMutexA(DWORD dwDesiredAccess, BOOL bInheritHandle, const char *lpName);
  __declspec(dllimport) BOOL __stdcall CloseHandle(HANDLE hObject);
  __declspec(dllimport) DWORD __stdcall GetEnvironmentVariableA(const char *lpName, char *lpBuffer, DWORD nSize);
  __declspec(dllimport) void __stdcall Sleep(DWORD dwMilliseconds);
  __declspec(dllimport) DWORD __stdcall GetLastError(void);

  /* Named pipes. */
  __declspec(dllimport) HANDLE __stdcall CreateNamedPipeA(const char *lpName, DWORD dwOpenMode, DWORD dwPipeMode, DWORD nMaxInstances, DWORD nOutBufferSize, DWORD nInBufferSize, DWORD nDefaultTimeOut, void *lpSecurityAttributes);
  __declspec(dllimport) BOOL   __stdcall ConnectNamedPipe(HANDLE hNamedPipe, void *lpOverlapped);
  __declspec(dllimport) BOOL   __stdcall DisconnectNamedPipe(HANDLE hNamedPipe);
  __declspec(dllimport) HANDLE __stdcall CreateFileA(const char *lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, void *lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
  __declspec(dllimport) BOOL   __stdcall ReadFile(HANDLE hFile, void *lpBuffer, DWORD nNumberOfBytesToRead, DWORD *lpNumberOfBytesRead, void *lpOverlapped);
  __declspec(dllimport) BOOL   __stdcall WriteFile(HANDLE hFile, const void *lpBuffer, DWORD nNumberOfBytesToWrite, DWORD *lpNumberOfBytesWritten, void *lpOverlapped);
  __declspec(dllimport) BOOL   __stdcall WaitNamedPipeA(const char *lpNamedPipeName, DWORD nTimeOut);

  /* Shared memory and semaphores (queues). */
  __declspec(dllimport) HANDLE __stdcall CreateFileMappingA(HANDLE hFile, void *lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, const char *lpName);
  __declspec(dllimport) HANDLE __stdcall OpenFileMappingA(DWORD dwDesiredAccess, BOOL bInheritHandle, const char *lpName);
  __declspec(dllimport) PVOID  __stdcall MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, DWORD dwNumberOfBytesToMap);
  __declspec(dllimport) BOOL   __stdcall UnmapViewOfFile(PVOID lpBaseAddress);
  __declspec(dllimport) HANDLE __stdcall CreateSemaphoreA(void *lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, const char *lpName);
  __declspec(dllimport) HANDLE __stdcall OpenSemaphoreA(DWORD dwDesiredAccess, BOOL bInheritHandle, const char *lpName);
  __declspec(dllimport) BOOL   __stdcall ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LONG *lpPreviousCount);
  __declspec(dllimport) DWORD  __stdcall GetCurrentProcessId(void);
  __declspec(dllimport) DWORD  __stdcall GetCurrentThreadId(void);
  __declspec(dllimport) LONG  __stdcall InterlockedCompareExchange(
                                 LONG volatile *dest, LONG exch, LONG cmp);

  #define FILE_MAP_ALL_ACCESS 0xF001F

  /* ---------- Error codes used by the client/server ---------- */
  #define ERROR_OPEN_FAILED   110L
  #define ERROR_BROKEN_PIPE   109L
  #define ERROR_ALREADY_EXISTS 183L

  #define ERROR_QUE_PROC_NOT_OWNED      330L
  #define ERROR_QUE_DUPLICATE       332L
  #define ERROR_QUE_ELEMENT_NOT_EXIST   333L
  #define ERROR_QUE_NO_MEMORY       334L
  #define ERROR_QUE_INVALID_HANDLE  337L
  #define ERROR_QUE_EMPTY           342L
  #define ERROR_QUE_NAME_NOT_EXIST      343L

  #define DC_SEM_SHARED        0x00000001UL   /* ignored on Win32 */

  /* ---------- DosExecPgm constants & RESULTCODES ---------- */
  #ifndef EXEC_ASYNC
    #define EXEC_ASYNC          0L
    #define EXEC_ASYNCB         1L
    #define EXEC_SYNC           2L
    #define EXEC_SYNCB          3L
    #define EXEC_LOAD           4L
    #define EXEC_ASYNCRESULT    5L
  #endif
  /* Legacy name used by F_platform.hpp. */
  #ifndef EXEC_BACKGROUND
    #define EXEC_BACKGROUND     EXEC_ASYNCRESULT
  #endif

  typedef struct _RESULTCODES {
      ULONG codeTerminate;
      ULONG codeResult;
      ULONG pidProcess;
  } RESULTCODES, *PRESULTCODES;

  /* ---------- Minimal STARTUPINFOA/PROCESS_INFORMATION for DosExecPgm ----------
     Layout matches the Win32 ANSI ABI (cb = 68 / 16 bytes on Win32).
     Declared locally so this header does not need <windows.h>, which
     would clash with the OS/2-compatible typedefs above. */
  typedef struct _STARTUPINFOA_LITE {
      DWORD   cb;
      char   *lpReserved;
      char   *lpDesktop;
      char   *lpTitle;
      DWORD   dwX, dwY, dwXSize, dwYSize;
      DWORD   dwXCountChars, dwYCountChars, dwFillAttribute, dwFlags;
      USHORT  wShowWindow, cbReserved2;
      BYTE   *lpReserved2;
      HANDLE  hStdInput, hStdOutput, hStdError;
  } STARTUPINFOA_LITE;

  typedef struct _PROCESS_INFORMATION_LITE {
      HANDLE hProcess, hThread;
      DWORD  dwProcessId, dwThreadId;
  } PROCESS_INFORMATION_LITE;

  #ifndef DETACHED_PROCESS
    #define DETACHED_PROCESS    0x00000008L
  #endif

  #ifndef DLL_PROCESS_ATTACH
    #define DLL_PROCESS_ATTACH  1
  #endif
  #ifndef DLL_PROCESS_DETACH
    #define DLL_PROCESS_DETACH  0
  #endif

  __declspec(dllimport) void __stdcall ExitProcess(UINT uExitCode);

  __declspec(dllimport) BOOL __stdcall CreateProcessA(
      const char *lpApplicationName,
      char       *lpCommandLine,
      void       *lpProcessAttributes,
      void       *lpThreadAttributes,
      BOOL        bInheritHandles,
      DWORD       dwCreationFlags,
      void       *lpEnvironment,
      const char *lpCurrentDirectory,
      STARTUPINFOA_LITE        *lpStartupInfo,
      PROCESS_INFORMATION_LITE *lpProcessInformation);

  __declspec(dllimport) DWORD __stdcall WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
  __declspec(dllimport) BOOL  __stdcall ReleaseMutex(HANDLE hMutex);
  __declspec(dllimport) HANDLE __stdcall CreateThread(void *lpThreadAttributes,
                                                      SIZE_T dwStackSize,
                                                      unsigned long (__stdcall *lpStartAddress)(void *),
                                                      void *lpParameter,
                                                      DWORD dwCreationFlags,
                                                      DWORD *lpThreadId);


  #ifndef CREATE_NEW_CONSOLE
    #define CREATE_NEW_CONSOLE  0x00000010L
  #endif

  __declspec(dllimport) BOOL __stdcall Beep(DWORD dwFreq, DWORD dwDuration);

  __declspec(dllimport) BOOL __stdcall GetExitCodeProcess(HANDLE hProcess, DWORD *lpExitCode);
  /* ---------- Эмуляция OS/2 API ---------- */

  #define CCHMAXPATH   260

    #define SEM_INDEFINITE_WAIT  0xFFFFFFFFUL
    #define INFINITE             0xFFFFFFFFUL
    #define WAIT_OBJECT_0        0x00000000UL
    #define WAIT_ABANDONED       0x00000080UL


  static inline APIRET DosBeep(ULONG freq, ULONG duration)
  {
      Beep(freq, duration);
      return 0;
  }

static inline APIRET DosExecPgm(char *pszFailName, LONG cbFailName,
                                ULONG execFlags,
                                const char *pszArg,
                                const char *pszEnv,
                                PRESULTCODES pResCodes,
                                const char *pszExe)
{
    STARTUPINFOA_LITE        si;
    PROCESS_INFORMATION_LITE pi;
    char szCmd[1024];
    char *p;
    int   n;
    DWORD flags = 0;
    BOOL  wait  = FALSE;

    (void)pszFailName; (void)cbFailName; (void)pszEnv;

    p = (char *)&si;
    for (n = 0; n < (int)sizeof(si); n++) p[n] = 0;
    p = (char *)&pi;
    for (n = 0; n < (int)sizeof(pi); n++) p[n] = 0;
    si.cb = sizeof(si);

    /* Build "exe arg" (pszArg may be NULL or empty). */
    n = 0;
    while (pszExe[n] && n < 1023) { szCmd[n] = pszExe[n]; n++; }
    if (pszArg && *pszArg) {
        szCmd[n++] = ' ';
        while (*pszArg && n < 1023) { szCmd[n++] = *pszArg++; }
    }
    szCmd[n] = 0;

    /* EXEC_ASYNC* -> don't wait, own console (caller wants it visible).
       EXEC_SYNC*  -> wait for completion, inherit parent's console. */
    switch (execFlags) {
    case EXEC_ASYNC:
    case EXEC_ASYNCB:
    case EXEC_ASYNCRESULT:
        flags = CREATE_NEW_CONSOLE;
        wait  = FALSE;
        break;
    case EXEC_SYNC:
    case EXEC_SYNCB:
    case EXEC_LOAD:
    default:
        flags = 0;
        wait  = TRUE;
        break;
    }

    if (!CreateProcessA((const char *)NULL, szCmd, NULL, NULL, FALSE,
                        flags, NULL, (const char *)NULL,
                        &si, &pi))
        return 1;

    if (wait) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        if (pResCodes) {
            DWORD code = 0;
            GetExitCodeProcess(pi.hProcess, &code);
            pResCodes->codeTerminate = code;
            pResCodes->codeResult    = 0;
            pResCodes->pidProcess    = pi.dwProcessId;
        }
    } else if (pResCodes) {
        pResCodes->codeTerminate = 0;
        pResCodes->codeResult    = 0;
        pResCodes->pidProcess    = pi.dwProcessId;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

/* ======================================================================
   Shared-memory registry (Win32 only)
   ======================================================================
   On Windows the file-mapping NAME is removed from the object namespace
   as soon as the last HANDLE to the object is closed — even if mapped
   views still exist. The object lives until the last view is unmapped,
   but its name is gone, so peer processes calling OpenFileMappingA get
   ERROR_FILE_NOT_FOUND (rc=2).

   The OS/2-compatible DosAllocSharedMem / DosGetNamedSharedMem wrappers
   below MUST keep the file-mapping handle open for the entire lifetime
   of the mapped view. We track {view -> handle} in a small registry so
   that DosFreeMem can close the handle when the view is unmapped.
   ====================================================================== */

#define F_SHMREG_MAX 64
static PVOID  g_shmRegView[F_SHMREG_MAX];
static HANDLE g_shmRegMap [F_SHMREG_MAX];
static int    g_shmRegCount   = 0;
static HANDLE g_shmRegMtx     = NULL;
static LONG   g_shmRegMtxInit = 0;

static void F_ShmRegLock(void)
{
    if (InterlockedCompareExchange(&g_shmRegMtxInit, 1, 0) == 0) {
        g_shmRegMtx = CreateMutexA(NULL, FALSE, NULL);
        g_shmRegMtxInit = 2;
    } else {
        while (g_shmRegMtxInit != 2) Sleep(0);
    }
    if (g_shmRegMtx) WaitForSingleObject(g_shmRegMtx, INFINITE);
}

static void F_ShmRegUnlock(void)
{
    if (g_shmRegMtx) ReleaseMutex(g_shmRegMtx);
}

/*! @brief Register a {view,handle} pair. Returns 0 on success, -1 on
    overflow (registry full). */
static int F_ShmRegAdd(PVOID view, HANDLE hMap)
{
    int rc = -1;
    F_ShmRegLock();
    if (g_shmRegCount < F_SHMREG_MAX) {
        g_shmRegView[g_shmRegCount] = view;
        g_shmRegMap [g_shmRegCount] = hMap;
        g_shmRegCount++;
        rc = 0;
    }
    F_ShmRegUnlock();
    return rc;
}

/*! @brief Remove the {view,handle} pair; return handle (caller closes),
    or NULL if not found (view belongs to VirtualAlloc, not a mapping). */
static HANDLE F_ShmRegTake(PVOID view)
{
    int    i;
    HANDLE h = NULL;
    F_ShmRegLock();
    for (i = 0; i < g_shmRegCount; i++) {
        if (g_shmRegView[i] == view) {
            h = g_shmRegMap[i];
            g_shmRegView[i] = g_shmRegView[g_shmRegCount - 1];
            g_shmRegMap [i] = g_shmRegMap [g_shmRegCount - 1];
            g_shmRegCount--;
            break;
        }
    }
    F_ShmRegUnlock();
    return h;
}

  static inline APIRET DosAllocMem(PVOID *ppv, ULONG cb, ULONG flags) {
      (void)flags;
      *ppv = VirtualAlloc(NULL, cb, MEM_COMMIT, PAGE_READWRITE);
      return (*ppv != NULL) ? 0 : 1;
  }

static inline APIRET DosFreeMem(PVOID pv)
{
    HANDLE h;
    if (!pv) return 1;
    h = F_ShmRegTake(pv);
    if (h) {
        /* pv is a named file-mapping view: unmap + release our handle,
           which destroys the name once both are gone. */
        UnmapViewOfFile(pv);
        CloseHandle(h);
        return 0;
    }
    /* pv was allocated by DosAllocMem -> VirtualAlloc. */
    if (UnmapViewOfFile(pv)) return 0;
    if (VirtualFree(pv, 0, MEM_RELEASE)) return 0;
    return 1;
}

  /*! @brief Translate OS/2 "\SHAREMEM\name" to Win32 "Local\name"
      by replacing backslashes with underscores. */
static inline void dos_shm_name(const char *pszOS2, char *out, int cbMax)
{
    const char *p = pszOS2 ? pszOS2 : "";
    char *o = out;
    int i;
    if (strncmp(p, "\\SHAREMEM\\", 10) == 0) p += 10;
    else if (strncmp(p, "\\sharemem\\", 10) == 0) p += 10;
    else if (*p == '\\') p++;
    memcpy(o, "Local\\", 6);
    o += 6;
    for (i = 0; p[i] && (int)(o - out) < cbMax - 1; i++) {
        *o++ = (p[i] == '\\') ? '_' : p[i];
    }
    *o = 0;
}

static inline APIRET DosAllocSharedMem(PVOID *ppv, const char *pszName,
                                       ULONG cb, ULONG flAttr)
{
    HANDLE hMap;
    PVOID  pView;
    DWORD  err;
    char   localName[300];

    (void)flAttr;
    if (pszName) dos_shm_name(pszName, localName, sizeof(localName));
    hMap = CreateFileMappingA((HANDLE)(long)-1, NULL, PAGE_READWRITE,
                              0, (DWORD)cb,
                              pszName ? localName : NULL);
    err = GetLastError();
    if (!hMap) return (APIRET)err;
    if (pszName && err == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMap);
        *ppv = NULL;
        return ERROR_ALREADY_EXISTS;
    }
    pView = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!pView) {
        CloseHandle(hMap);
        return (APIRET)GetLastError();
    }
    if (F_ShmRegAdd(pView, hMap) != 0) {
        /* Registry full — fall back to old (buggy) behaviour: name will
           vanish. Should not happen with F_SHMREG_MAX = 64. */
        CloseHandle(hMap);
    }
    *ppv = pView;
    return 0;
}

static inline APIRET DosGetNamedSharedMem(PVOID *ppv, const char *pszName,
                                          ULONG flAttr)
{
    HANDLE hMap;
    PVOID  pView;
    char   localName[300];

    (void)flAttr;
    dos_shm_name(pszName, localName, sizeof(localName));
    hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, localName);
    if (!hMap) return (APIRET)GetLastError();
    pView = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!pView) {
        CloseHandle(hMap);
        return (APIRET)GetLastError();
    }
    if (F_ShmRegAdd(pView, hMap) != 0) {
        CloseHandle(hMap);
    }
    *ppv = pView;
    return 0;
}

  static inline APIRET DosQueryModuleHandle(const char *pszName, HMODULE *phmod) {
      *phmod = GetModuleHandleA(pszName);
      return (*phmod != NULL) ? 0 : 1;
  }
  static inline APIRET DosQueryProcAddr(HMODULE hmod, ULONG ord, const char *pszName, PFN *ppfn) {
      (void)pszName;
      *ppfn = (PFN)GetProcAddress(hmod, (const char *)(ULONG)ord);
      return (*ppfn != 0) ? 0 : 1;
  }
  static inline APIRET DosLoadModule(const char *pszFailName, ULONG cbFailName,
                                     const char *pszModName, HMODULE *phmod) {
      (void)pszFailName; (void)cbFailName;
      *phmod = LoadLibraryA(pszModName);
      return (*phmod != NULL) ? 0 : 1;
  }
  static inline APIRET DosFreeModule(HMODULE hmod) {
      return FreeLibrary(hmod) ? 0 : 1;
  }
  static inline APIRET DosScanEnv(const char *pszName, char **ppszValue) {
      static char szBuf[1024];
      DWORD dwLen = GetEnvironmentVariableA(pszName, szBuf, sizeof(szBuf));
      if (dwLen == 0 || dwLen >= sizeof(szBuf)) {
          *ppszValue = 0;
          return 1;
      }
      *ppszValue = szBuf;
      return 0;
  }
  static inline APIRET DosSleep(ULONG msec) {
      Sleep(msec);
      return 0;
  }

  /* ---------- Thread handle emulation for DosWaitThread ----------
     _beginthread() on Win32 Watcom returns a thread HANDLE cast to
     int; on OS/2 it returns a TID. Callers treat the two opaquely
     and only pass them to DosWaitThread(). */
  typedef ULONG TID;
  typedef TID  *PTID;

  #define DCWW_WAIT     0
  #define DCWW_NOWAIT   1

  static inline APIRET DosWaitThread(PTID ptid, ULONG option) {
      (void)option;
      WaitForSingleObject((HANDLE)(*ptid), INFINITE);
      return 0;
  }

  static inline APIRET DosCreateMutexSem(const char *pszName, HMTX *phmtx,
                                         ULONG flAttr, BOOL fState) {
      HANDLE h;
      DWORD  err;

      (void)flAttr;
      h   = CreateMutexA(NULL, fState ? TRUE : FALSE, pszName);
      err = GetLastError();
      if (!h) return (APIRET)err;
      if (pszName && err == ERROR_ALREADY_EXISTS) {
          CloseHandle(h);
          return ERROR_ALREADY_EXISTS;
      }
      *phmtx = (HMTX)h;
      return 0;
  }
  static inline APIRET DosOpenMutexSem(const char *pszName, HMTX *phmtx) {
      *phmtx = OpenMutexA(SYNCHRONIZE, FALSE, pszName);
      return (*phmtx != NULL) ? 0 : 1;
  }
  static inline APIRET DosCloseMutexSem(HMTX hmtx) {
      return CloseHandle(hmtx) ? 0 : 1;
  }

  /* ---------- Semaphore helpers for Fs_ClientList ---------- */


  static inline APIRET DosRequestMutexSem(HMTX hmtx, ULONG ulTimeout) {
      DWORD dwTimeout = (ulTimeout == SEM_INDEFINITE_WAIT) ? INFINITE : (DWORD)ulTimeout;
      DWORD dwWait = WaitForSingleObject((HANDLE)hmtx, dwTimeout);
      return (dwWait == WAIT_OBJECT_0 || dwWait == WAIT_ABANDONED) ? 0 : 1;
  }
  static inline APIRET DosReleaseMutexSem(HMTX hmtx) {
      return ReleaseMutex((HANDLE)hmtx) ? 0 : 1;
  }

  /* ---------- Thread-local storage (Win32) ---------- */
   #define TLS_OUT_OF_INDEXES  ((DWORD)0xFFFFFFFF)
  __declspec(dllimport) DWORD __stdcall TlsAlloc(void);
  __declspec(dllimport) PVOID __stdcall TlsGetValue(DWORD dwTlsIndex);
  __declspec(dllimport) BOOL  __stdcall TlsSetValue(DWORD dwTlsIndex, PVOID lpTlsValue);
  __declspec(dllimport) BOOL  __stdcall TlsFree(DWORD dwTlsIndex);
	
  /* ---------- TIB/TIB2/PIB + DosGetInfoBlocks (Win32, TLS-backed) ---------- */

  typedef struct _TIB2 { ULONG tib2_ultid; } TIB2, *PTIB2;
  typedef struct _TIB  { PTIB2 tib_ptib2; }  TIB,  *PTIB;
  typedef struct _PIB  { ULONG pib_ulpid; }  PIB,  *PPIB;

  static inline APIRET DosGetInfoBlocks(PTIB *pptib, PPIB *pppib)
  {
      static DWORD s_tls = TLS_OUT_OF_INDEXES;
      TIB *pTib;
      TIB2 *pTib2;
      PIB *pPib;
      DWORD idx;

      if (s_tls == TLS_OUT_OF_INDEXES) {
          DWORD ni = TlsAlloc();
          if (ni == TLS_OUT_OF_INDEXES) return 1;
          s_tls = ni;
      }
      idx = s_tls;

      pTib = (TIB *)TlsGetValue(idx);
      if (!pTib) {
          BYTE *b = (BYTE *)VirtualAlloc(NULL,
                          sizeof(TIB) + sizeof(TIB2) + sizeof(PIB),
                          MEM_COMMIT, PAGE_READWRITE);
          if (!b) return 1;
          pTib  = (TIB *)b;
          pTib2 = (TIB2 *)(b + sizeof(TIB));
          pPib  = (PIB  *)(b + sizeof(TIB) + sizeof(TIB2));
          pTib->tib_ptib2 = pTib2;
          TlsSetValue(idx, pTib);
      } else {
          pTib2 = pTib->tib_ptib2;
          pPib  = (PIB *)((BYTE *)pTib + sizeof(TIB) + sizeof(TIB2));
      }

      pTib2->tib2_ultid = (ULONG)GetCurrentThreadId();
      pPib->pib_ulpid   = (ULONG)GetCurrentProcessId();

      if (pptib) *pptib = pTib;
      if (pppib) *pppib = pPib;
      return 0;
  }

  /* ---------- Структуры ---------- */
  typedef struct _POINTL { LONG x; LONG y; } POINTL, *PPOINTL;
  typedef struct _POINTS { SHORT x; SHORT y; } POINTS, *PPOINTS;
  typedef struct _SIZEL { LONG cx; LONG cy; } SIZEL, *PSIZEL;
  typedef struct _SIZEF { FIXED cx; FIXED cy; } SIZEF, *PSIZEF;
  typedef struct _RECTL { LONG xLeft; LONG yBottom; LONG xRight; LONG yTop; } RECTL, *PRECTL;
  typedef struct _MATRIXLF { FIXED fxM11; FIXED fxM12; LONG lM13; FIXED fxM21; FIXED fxM22; LONG lM23; LONG lM31; LONG lM32; LONG lM33; } MATRIXLF, *PMATRIXLF;
  typedef struct _ARCPARAMS { LONG lP; LONG lQ; LONG lR; LONG lS; } ARCPARAMS, *PARCPARAMS;
  typedef struct _GRADIENTL { LONG x; LONG y; } GRADIENTL, *PGRADIENTL;

  typedef struct _RGB { BYTE bBlue; BYTE bGreen; BYTE bRed; BYTE fcOptions; } RGB, *PRGB;
  typedef RGB RGB2, *PRGB2;

  typedef struct _BITMAPINFOHEADER {
      ULONG cbFix, cx, cy; USHORT cPlanes, cBitCount;
  } BITMAPINFOHEADER, *PBITMAPINFOHEADER;

  typedef struct _BITMAPINFOHEADER2 {
      ULONG cbFix, cx, cy; USHORT cPlanes, cBitCount;
      ULONG ulCompression, cbImage, cxResolution, cyResolution, cclrUsed, cclrImportant;
      USHORT usUnits, usReserved, usRecording, usRendering;
      ULONG cSize1, cSize2, ulColorEncoding, ulIdentifier;
  } BITMAPINFOHEADER2, *PBITMAPINFOHEADER2;

  /* Плоские: поля заголовка доступны напрямую (pInfo->cbFix) */
  typedef struct _BITMAPINFO {
      ULONG cbFix, cx, cy; USHORT cPlanes, cBitCount; RGB argbColor[1];
  } BITMAPINFO, *PBITMAPINFO;

  typedef struct _BITMAPINFO2 {
      ULONG cbFix, cx, cy; USHORT cPlanes, cBitCount;
      ULONG ulCompression, cbImage, cxResolution, cyResolution, cclrUsed, cclrImportant;
      USHORT usUnits, usReserved, usRecording, usRendering;
      ULONG cSize1, cSize2, ulColorEncoding, ulIdentifier;
      RGB2 argbColor[1];
  } BITMAPINFO2, *PBITMAPINFO2;

  typedef struct _FATTRS { USHORT usRecordLength, fsSelection; LONG lMatch; CHAR szFacename[32]; USHORT idRegistry, usCodePage; LONG lMaxBaselineExt, lAveCharWidth; } FATTRS, *PFATTRS;
  typedef struct _FONTMETRICS { CHAR szFamilyname[32], szFacename[32]; USHORT usCodepage, usWeight, usWidth; } FONTMETRICS, *PFONTMETRICS;
  typedef struct _KERNINGPAIRS { SHORT sFirstChar, sSecondChar; LONG lKerningAmount; } KERNINGPAIRS, *PKERNINGPAIRS;
  typedef struct _FACENAMEDESC { USHORT usSize, usWeightClass, usWidthClass, usReserved; ULONG flOptions; } FACENAMEDESC, *PFACENAMEDESC;
  typedef struct _RASTERIZERCAPS { ULONG cb, ulAvailFlags, ulEnabledFlags; } RASTERIZERCAPS, *PRASTERIZERCAPS;
  typedef struct _POLYGON { ULONG ulPoints; PPOINTL aPointl; } POLYGON, *PPOLYGON;
  typedef struct _POLYSET { ULONG ulPolys; POLYGON aPolygon[1]; } POLYSET, *PPOLYSET;
  typedef struct _RGNRECT { ULONG ircStart, crc, crcReturned, ulDirection; } RGNRECT, *PRGNRECT;
  typedef CHAR FFDESCS[2][32];
  typedef FFDESCS *PFFDESCS;
  typedef struct _QMSG { HWND hwnd; ULONG msg; MPARAM mp1, mp2; ULONG time; POINTL ptl; } QMSG, *PQMSG;

  typedef PSZ *PDEVOPENDATA;                          /* user-provided */

  typedef struct _DRIVDATA {
      LONG cb;
      LONG lVersion;
      CHAR szDeviceName[32];
      CHAR abGeneralData[1];
  } DRIVDATA, *PDRIVDATA;

  typedef struct _HCINFO {
      CHAR szFormname[32];
      LONG cx;
      LONG cy;
      LONG xLeftClip;
      LONG yBottomClip;
      LONG xRightClip;
      LONG yTopClip;
      LONG xPels;
      LONG yPels;
      LONG flAttributes;
  } HCINFO, *PHCINFO;

  typedef CHAR STR16[16], *PSTR16;
  typedef CHAR STR32[32], *PSTR32;
  typedef CHAR STR64[64], *PSTR64;

  /* ---------- GPI foreground bundles ---------- */
  typedef struct _LINEBUNDLE {
      LONG   lColor;
      LONG   lBackColor;
      USHORT usMixMode;
      USHORT usBackMixMode;
      FIXED  fxWidth;
      LONG   lGeomWidth;
      USHORT usType;
      USHORT usEnd;
      USHORT usJoin;
      USHORT usReserved;
  } LINEBUNDLE, *PLINEBUNDLE;

  typedef struct _CHARBUNDLE {
      LONG   lColor;
      LONG   lBackColor;
      USHORT usMixMode;
      USHORT usBackMixMode;
      USHORT usSet;
      USHORT usPrecision;
      SIZEF  sizfxCell;
      POINTL ptlAngle;
      POINTL ptlShear;
      USHORT usDirection;
      USHORT usTextAlign;
      FIXED  fxExtra;
      FIXED  fxBreakExtra;
  } CHARBUNDLE, *PCHARBUNDLE;

  typedef struct _MARKERBUNDLE {
      LONG   lColor;
      LONG   lBackColor;
      USHORT usMixMode;
      USHORT usBackMixMode;
      USHORT usSet;
      USHORT usSymbol;
      SIZEF  sizfxCell;
  } MARKERBUNDLE, *PMARKERBUNDLE;

  typedef struct _AREABUNDLE {
      LONG   lColor;
      LONG   lBackColor;
      USHORT usMixMode;
      USHORT usBackMixMode;
      USHORT usSet;
      USHORT usSymbol;
      POINTL ptlRefPoint;
  } AREABUNDLE, *PAREABUNDLE;

  /* ---------- Primitive selectors, masks, flags ---------- */
  #define PRIM_LINE      1L
  #define PRIM_CHAR      2L
  #define PRIM_MARKER    3L
  #define PRIM_AREA      4L
  #define PRIM_IMAGE     5L

  #define LBB_COLOR          0x0001L
  #define LBB_BACK_COLOR     0x0002L
  #define LBB_MIX_MODE       0x0004L
  #define LBB_BACK_MIX_MODE  0x0008L
  #define LBB_WIDTH          0x0010L
  #define LBB_GEOM_WIDTH     0x0020L
  #define LBB_TYPE           0x0040L
  #define LBB_END            0x0080L
  #define LBB_JOIN           0x0100L

  #define CBB_COLOR          0x0001L
  #define CBB_BACK_COLOR     0x0002L
  #define CBB_MIX_MODE       0x0004L
  #define CBB_BACK_MIX_MODE  0x0008L
  #define CBB_SET            0x0010L
  #define CBB_MODE           0x0020L
  #define CBB_BOX            0x0040L
  #define CBB_ANGLE          0x0080L
  #define CBB_SHEAR          0x0100L
  #define CBB_DIRECTION      0x0200L
  #define CBB_TEXT_ALIGN     0x0400L
  #define CBB_EXTRA          0x0800L
  #define CBB_BREAK_EXTRA    0x1000L

  #define MBB_COLOR          0x0001L
  #define MBB_BACK_COLOR     0x0002L
  #define MBB_MIX_MODE       0x0004L
  #define MBB_BACK_MIX_MODE  0x0008L
  #define MBB_SET            0x0010L
  #define MBB_SYMBOL         0x0020L
  #define MBB_BOX            0x0040L

  #define ABB_COLOR          0x0001L
  #define ABB_BACK_COLOR     0x0002L
  #define ABB_MIX_MODE       0x0004L
  #define ABB_BACK_MIX_MODE  0x0008L
  #define ABB_SET            0x0010L
  #define ABB_SYMBOL         0x0020L
  #define ABB_REF_POINT      0x0040L

  #define BA_NOBOUNDARY      0L
  #define BA_BOUNDARY        1L
  #define BA_ALTERNATE       0L
  #define BA_WINDING         2L

  #define CHS_CLIP           0x0010L

  /* ==================================================================
     Прототипы Gpi* (для gpi16*.c). Пропущены GpiBeginElement /
     GpiQueryElementType / GpiElement: они объявлены в pmgpi16.c с
     PSZ (non-const), дубликат с PCSZ вызвал бы конфликт типов.
     ================================================================== */

  /* GPICONTROL */
  HPS   APIENTRY GpiCreatePS(HAB hab, HDC hdc, PSIZEL psizlSize, ULONG flOptions);
  BOOL  APIENTRY GpiDestroyPS(HPS hps);
  BOOL  APIENTRY GpiAssociate(HPS hps, HDC hdc);
  BOOL  APIENTRY GpiRestorePS(HPS hps, LONG lPSid);
  LONG  APIENTRY GpiSavePS(HPS hps);
  BOOL  APIENTRY GpiErase(HPS hps);
  HDC   APIENTRY GpiQueryDevice(HPS hps);
  BOOL  APIENTRY GpiResetPS(HPS hps, ULONG flOptions);
  BOOL  APIENTRY GpiSetPS(HPS hps, PSIZEL psizlsize, ULONG flOptions);
  ULONG APIENTRY GpiQueryPS(HPS hps, PSIZEL psizlSize);
  LONG  APIENTRY GpiErrorSegmentData(HPS hps, PLONG plSegment, PLONG plContext);
  LONG  APIENTRY GpiQueryDrawControl(HPS hps, LONG lControl);
  BOOL  APIENTRY GpiSetDrawControl(HPS hps, LONG lControl, LONG lValue);
  LONG  APIENTRY GpiQueryDrawingMode(HPS hps);
  BOOL  APIENTRY GpiSetDrawingMode(HPS hps, LONG lMode);
  LONG  APIENTRY GpiQueryStopDraw(HPS hps);
  BOOL  APIENTRY GpiSetStopDraw(HPS hps, LONG lValue);

  /* GPICORRELATION */
  LONG  APIENTRY GpiCorrelateChain(HPS hps, LONG lType, PPOINTL pptlPick, LONG lMaxHits, LONG lMaxDepth, PLONG pl2);
  BOOL  APIENTRY GpiQueryTag(HPS hps, PLONG plTag);
  BOOL  APIENTRY GpiSetTag(HPS hps, LONG lTag);
  BOOL  APIENTRY GpiQueryPickApertureSize(HPS hps, PSIZEL psizlSize);
  BOOL  APIENTRY GpiSetPickApertureSize(HPS hps, LONG lOptions, PSIZEL psizlSize);
  BOOL  APIENTRY GpiQueryPickAperturePosition(HPS hps, PPOINTL pptlPoint);
  BOOL  APIENTRY GpiSetPickAperturePosition(HPS hps, PPOINTL pptlPick);
  BOOL  APIENTRY GpiQueryBoundaryData(HPS hps, PRECTL prclBoundary);
  BOOL  APIENTRY GpiResetBoundaryData(HPS hps);
  LONG  APIENTRY GpiCorrelateFrom(HPS hps, LONG lFirstSegment, LONG lLastSegment, LONG lType, PPOINTL pptlPick, LONG lMaxHits, LONG lMaxDepth, PLONG plSegTag);
  LONG  APIENTRY GpiCorrelateSegment(HPS hps, LONG lSegment, LONG lType, PPOINTL pptlPick, LONG lMaxHits, LONG lMaxDepth, PLONG alSegTag);

  /* GPISEGMENTS */
  BOOL  APIENTRY GpiOpenSegment(HPS hps, LONG lSegment);
  BOOL  APIENTRY GpiCloseSegment(HPS hps);
  BOOL  APIENTRY GpiDeleteSegment(HPS hps, LONG lSegid);
  LONG  APIENTRY GpiQueryInitialSegmentAttrs(HPS hps, LONG lAttribute);
  BOOL  APIENTRY GpiSetInitialSegmentAttrs(HPS hps, LONG lAttribute, LONG lValue);
  LONG  APIENTRY GpiQuerySegmentAttrs(HPS hps, LONG lSegid, LONG lAttribute);
  BOOL  APIENTRY GpiSetSegmentAttrs(HPS hps, LONG lSegid, LONG lAttribute, LONG lValue);
  LONG  APIENTRY GpiQuerySegmentPriority(HPS hps, LONG lRefSegid, LONG lOrder);
  BOOL  APIENTRY GpiSetSegmentPriority(HPS hps, LONG lSegid, LONG lRefSegid, LONG lOrder);
  BOOL  APIENTRY GpiDeleteSegments(HPS hps, LONG lFirstSegment, LONG lLastSegment);
  LONG  APIENTRY GpiQuerySegmentNames(HPS hps, LONG lFirstSegid, LONG lLastSegid, LONG lMax, PLONG alSegids);
  LONG  APIENTRY GpiGetData(HPS hps, LONG lSegid, PLONG plOffset, LONG lFormat, LONG lLength, PBYTE pbData);
  LONG  APIENTRY GpiPutData(HPS hps, LONG lFormat, PLONG plCount, PBYTE pbData);
  BOOL  APIENTRY GpiDrawChain(HPS hps);
  BOOL  APIENTRY GpiDrawFrom(HPS hps, LONG lFirstSegment, LONG lLastSegment);
  BOOL  APIENTRY GpiDrawSegment(HPS hps, LONG lSegment);
  BOOL  APIENTRY GpiDrawDynamics(HPS hps);
  BOOL  APIENTRY GpiRemoveDynamics(HPS hps, LONG lFirstSegid, LONG lLastSegid);

  /* GPISEGEDITING */
  LONG  APIENTRY GpiQueryEditMode(HPS hps);
  BOOL  APIENTRY GpiSetEditMode(HPS hps, LONG lMode);
  LONG  APIENTRY GpiQueryElementPointer(HPS hps);
  BOOL  APIENTRY GpiSetElementPointer(HPS hps, LONG lElement);
  BOOL  APIENTRY GpiOffsetElementPointer(HPS hps, LONG loffset);
  BOOL  APIENTRY GpiDeleteElement(HPS hps);
  BOOL  APIENTRY GpiDeleteElementRange(HPS hps, LONG lFirstElement, LONG lLastElement);
  BOOL  APIENTRY GpiLabel(HPS hps, LONG lLabel);
  BOOL  APIENTRY GpiDeleteElementsBetweenLabels(HPS hps, LONG lFirstLabel, LONG lLastLabel);
  BOOL  APIENTRY GpiSetElementPointerAtLabel(HPS hps, LONG lLabel);
  LONG  APIENTRY GpiQueryElement(HPS hps, LONG lOff, LONG lMaxLength, PBYTE pbData);
  BOOL  APIENTRY GpiEndElement(HPS hps);

  /* GPITRANSFORMS */
  BOOL  APIENTRY GpiQuerySegmentTransformMatrix(HPS hps, LONG lSegid, LONG lCount, PMATRIXLF pmatlfArray);
  BOOL  APIENTRY GpiSetSegmentTransformMatrix(HPS hps, LONG lSegid, LONG lCount, PMATRIXLF pmatlfarray, LONG lOptions);
  BOOL  APIENTRY GpiConvert(HPS hps, LONG lSrc, LONG lTarg, LONG lCount, PPOINTL aptlPoints);
  BOOL  APIENTRY GpiConvertWithMatrix(HPS hps, LONG lCountp, PPOINTL aptlPoints, LONG lCount, PMATRIXLF pmatlfArray);
  BOOL  APIENTRY GpiQueryModelTransformMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray);
  BOOL  APIENTRY GpiSetModelTransformMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray, LONG lOptions);
  LONG  APIENTRY GpiCallSegmentMatrix(HPS hps, LONG lSegment, LONG lCount, PMATRIXLF pmatlfArray, LONG lOptions);
  BOOL  APIENTRY GpiQueryDefaultViewMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray);
  BOOL  APIENTRY GpiSetDefaultViewMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfarray, LONG lOptions);
  BOOL  APIENTRY GpiQueryPageViewport(HPS hps, PRECTL prclViewport);
  BOOL  APIENTRY GpiSetPageViewport(HPS hps, PRECTL prclViewport);
  BOOL  APIENTRY GpiQueryViewingTransformMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray);
  BOOL  APIENTRY GpiSetViewingTransformMatrix(HPS hps, LONG lCount, PMATRIXLF pmatlfArray, LONG lOptions);
  BOOL  APIENTRY GpiTranslate(HPS hps, PMATRIXLF pmatlf, LONG lOptions, PPOINTL pptl);
  BOOL  APIENTRY GpiScale(HPS hps, PMATRIXLF pmatlf, LONG lOptions, PFIXED afx, PPOINTL pptl);
  BOOL  APIENTRY GpiRotate(HPS hps, PMATRIXLF pmatlf, LONG lOptions, FIXED fxAngle, PPOINTL pptl);
  BOOL  APIENTRY GpiSetGraphicsField(HPS hps, PRECTL prclField);
  BOOL  APIENTRY GpiQueryGraphicsField(HPS hps, PRECTL prclField);
  BOOL  APIENTRY GpiSetViewingLimits(HPS hps, PRECTL prclLimits);
  BOOL  APIENTRY GpiQueryViewingLimits(HPS hps, PRECTL prclLimits);

  /* GPIPATHS */
  BOOL  APIENTRY GpiBeginPath(HPS hps, LONG lPath);
  BOOL  APIENTRY GpiEndPath(HPS hps);
  BOOL  APIENTRY GpiCloseFigure(HPS hps);
  BOOL  APIENTRY GpiModifyPath(HPS hps, LONG lPath, LONG lMode);
  LONG  APIENTRY GpiFillPath(HPS hps, LONG lPath, LONG lOptions);
  BOOL  APIENTRY GpiSetClipPath(HPS hps, LONG lPath, LONG lOptions);
  LONG  APIENTRY GpiOutlinePath(HPS hps, LONG lPath, LONG lOptions);
  HRGN  APIENTRY GpiPathToRegion(HPS hps, LONG lPath, LONG lOptions);
  LONG  APIENTRY GpiStrokePath(HPS hps, LONG lPath, ULONG flOptions);

  /* GPILOGCOLORTABLE */
  BOOL  APIENTRY GpiCreateLogColorTable(HPS hps, ULONG flOptions, LONG lFormat, LONG lStart, LONG lCount, PLONG alTable);
  BOOL  APIENTRY GpiQueryColorData(HPS hps, LONG lCount, PLONG alArray);
  LONG  APIENTRY GpiQueryLogColorTable(HPS hps, ULONG flOptions, LONG lStart, LONG lCount, PLONG alArray);
  LONG  APIENTRY GpiQueryRealColors(HPS hps, ULONG flOptions, LONG lStart, LONG lCount, PLONG alColors);
  LONG  APIENTRY GpiQueryNearestColor(HPS hps, ULONG flOptions, LONG lRgbIn);
  LONG  APIENTRY GpiQueryColorIndex(HPS hps, ULONG flOptions, LONG lRgbColor);
  LONG  APIENTRY GpiQueryRGBColor(HPS hps, ULONG flOptions, LONG lColorIndex);
  HPAL  APIENTRY GpiCreatePalette(HAB hab, ULONG flOptions, ULONG ulFormat, ULONG ulCount, PULONG aulTable);
  BOOL  APIENTRY GpiDeletePalette(HPAL hpal);
  HPAL  APIENTRY GpiSelectPalette(HPS hps, HPAL hpal);
  LONG  APIENTRY GpiAnimatePalette(HPAL hpal, ULONG ulFormat, ULONG ulStart, ULONG ulCount, PULONG aulTable);
  BOOL  APIENTRY GpiSetPaletteEntries(HPAL hpal, ULONG ulFormat, ULONG ulStart, ULONG ulCount, PULONG aulTable);
  HPAL  APIENTRY GpiQueryPalette(HPS hps);
  LONG  APIENTRY GpiQueryPaletteInfo(HPAL hpal, HPS hps, ULONG flOptions, ULONG ulStart, ULONG ulCount, PULONG aulArray);
  ULONG APIENTRY GpiResizePalette(HPAL hpal, ULONG newsize);
  LONG  APIENTRY GpiQueryNearestPaletteIndex(HPAL hpal, ULONG color);

  /* GPIPRIMITIVES */
  BOOL  APIENTRY GpiSetColor(HPS hps, LONG lColor);
  LONG  APIENTRY GpiQueryColor(HPS hps);
  LONG  APIENTRY GpiBox(HPS hps, LONG lControl, PPOINTL pptlPoint, LONG lHRound, LONG lVRound);
  BOOL  APIENTRY GpiMove(HPS hps, PPOINTL pptlPoint);
  LONG  APIENTRY GpiLine(HPS hps, PPOINTL pptlEndPoint);
  LONG  APIENTRY GpiPolyLine(HPS hps, LONG lCount, PPOINTL aptlPoints);
  LONG  APIENTRY GpiPolyLineDisjoint(HPS hps, LONG lCount, PPOINTL aptlPoints);
  BOOL  APIENTRY GpiSetPattern(HPS hps, LONG lPatternSymbol);
  LONG  APIENTRY GpiQueryPattern(HPS hps);
  BOOL  APIENTRY GpiBeginArea(HPS hps, ULONG flOptions);
  LONG  APIENTRY GpiEndArea(HPS hps);
  LONG  APIENTRY GpiCharString(HPS hps, LONG lCount, PCH pchString);
  LONG  APIENTRY GpiCharStringAt(HPS hps, PPOINTL pptlPoint, LONG lCount, PCH pchString);
  BOOL  APIENTRY GpiSetAttrMode(HPS hps, LONG lMode);
  LONG  APIENTRY GpiQueryAttrMode(HPS hps);
  BOOL  APIENTRY GpiSetAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask, ULONG flDefMask, PBUNDLE ppbunAttrs);
  LONG  APIENTRY GpiQueryAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask, PBUNDLE ppbunAttrs);
  BOOL  APIENTRY GpiSetBackColor(HPS hps, LONG lColor);
  LONG  APIENTRY GpiQueryBackColor(HPS hps);
  BOOL  APIENTRY GpiSetMix(HPS hps, LONG lMixMode);
  LONG  APIENTRY GpiQueryMix(HPS hps);
  BOOL  APIENTRY GpiSetBackMix(HPS hps, LONG lMixMode);
  LONG  APIENTRY GpiQueryBackMix(HPS hps);
  BOOL  APIENTRY GpiSetLineType(HPS hps, LONG lLineType);
  LONG  APIENTRY GpiQueryLineType(HPS hps);
  BOOL  APIENTRY GpiSetLineWidth(HPS hps, FIXED fxLineWidth);
  FIXED APIENTRY GpiQueryLineWidth(HPS hps);
  BOOL  APIENTRY GpiSetLineWidthGeom(HPS hps, LONG lLineWidth);
  LONG  APIENTRY GpiQueryLineWidthGeom(HPS hps);
  BOOL  APIENTRY GpiSetLineEnd(HPS hps, LONG lLineEnd);
  LONG  APIENTRY GpiQueryLineEnd(HPS hps);
  BOOL  APIENTRY GpiSetLineJoin(HPS hps, LONG lLineJoin);
  LONG  APIENTRY GpiQueryLineJoin(HPS hps);
  BOOL  APIENTRY GpiSetCurrentPosition(HPS hps, PPOINTL pptlPoint);
  BOOL  APIENTRY GpiQueryCurrentPosition(HPS hps, PPOINTL pptlPoint);
  BOOL  APIENTRY GpiSetArcParams(HPS hps, PARCPARAMS parcpArcParams);
  BOOL  APIENTRY GpiQueryArcParams(HPS hps, PARCPARAMS parcpArcParams);
  LONG  APIENTRY GpiPointArc(HPS hps, PPOINTL pptl2);
  LONG  APIENTRY GpiFullArc(HPS hps, LONG lControl, FIXED fxMultiplier);
  LONG  APIENTRY GpiPartialArc(HPS hps, PPOINTL pptlCenter, FIXED fxMultiplier, FIXED fxStartAngle, FIXED fxSweepAngle);
  LONG  APIENTRY GpiOval(HPS hps, PPOINTL aptlPoints, FIXED fxStartAngle, FIXED fxSweepAngle, ULONG ulOptions);
  LONG  APIENTRY GpiPolyFillet(HPS hps, LONG lCount, PPOINTL aptlPoints);
  LONG  APIENTRY GpiPolySpline(HPS hps, LONG lCount, PPOINTL aptlPoints);
  LONG  APIENTRY GpiPolyFilletSharp(HPS hps, LONG lCount, PPOINTL aptlPoints, PFIXED afxPoints);
  BOOL  APIENTRY GpiSetPatternSet(HPS hps, LONG lSet);
  LONG  APIENTRY GpiQueryPatternSet(HPS hps);
  BOOL  APIENTRY GpiSetPatternRefPoint(HPS hps, PPOINTL pptlRefPoint);
  BOOL  APIENTRY GpiQueryPatternRefPoint(HPS hps, PPOINTL pptlRefPoint);
  BOOL  APIENTRY GpiQueryCharStringPos(HPS hps, ULONG flOptions, LONG lCount, PCH pchString, PLONG alXincrements, PPOINTL aptlPositions);
  BOOL  APIENTRY GpiQueryCharStringPosAt(HPS hps, PPOINTL pptlStart, ULONG flOptions, LONG lCount, PCH pchString, PLONG alXincrements, PPOINTL aptlPositions);
  LONG  APIENTRY GpiQueryTabbedTextExtent(HPS hps, LONG lCount, PCH pchString, LONG lTabCount, PULONG puTabStops);
  BOOL  APIENTRY GpiQueryTextBox(HPS hps, LONG lCount1, PCH pchString, LONG lCount2, PPOINTL aptlPoints);
  BOOL  APIENTRY GpiQueryDefCharBox(HPS hps, PSIZEL psizlSize);
  BOOL  APIENTRY GpiSetCharSet(HPS hps, LONG llcid);
  LONG  APIENTRY GpiQueryCharSet(HPS hps);
  BOOL  APIENTRY GpiSetCharBox(HPS hps, PSIZEF psizfxBox);
  BOOL  APIENTRY GpiQueryCharBox(HPS hps, PSIZEF psizfxSize);
  BOOL  APIENTRY GpiSetCharAngle(HPS hps, PGRADIENTL pgradlAngle);
  BOOL  APIENTRY GpiQueryCharAngle(HPS hps, PGRADIENTL pgradlAngle);
  BOOL  APIENTRY GpiSetCharShear(HPS hps, PPOINTL pptlAngle);
  BOOL  APIENTRY GpiQueryCharShear(HPS hps, PPOINTL pptlShear);
  BOOL  APIENTRY GpiSetCharDirection(HPS hps, LONG lDirection);
  LONG  APIENTRY GpiQueryCharDirection(HPS hps);
  BOOL  APIENTRY GpiSetCharMode(HPS hps, LONG lMode);
  LONG  APIENTRY GpiQueryCharMode(HPS hps);
  BOOL  APIENTRY GpiSetTextAlignment(HPS hps, LONG lHoriz, LONG lVert);
  BOOL  APIENTRY GpiQueryTextAlignment(HPS hps, PLONG plHoriz, PLONG plVert);
  LONG  APIENTRY GpiCharStringPos(HPS hps, PRECTL prclRect, ULONG flOptions, LONG lCount, PCH pchString, PLONG alAdx);
  LONG  APIENTRY GpiTabbedCharStringAt(HPS hps, PPOINTL pPtStart, PRECTL prclRect, ULONG flOptions, LONG lCount, PCH pchString, LONG lTabCount, PULONG puTabStops, LONG lTabOrigin);
  LONG  APIENTRY GpiCharStringPosAt(HPS hps, PPOINTL pptlStart, PRECTL prclRect, ULONG flOptions, LONG lCount, PCH pchString, PLONG alAdx);
  BOOL  APIENTRY GpiSetCharExtra(HPS hps, FIXED Extra);
  BOOL  APIENTRY GpiSetCharBreakExtra(HPS hps, FIXED BreakExtra);
  BOOL  APIENTRY GpiQueryCharExtra(HPS hps, PFIXED Extra);
  BOOL  APIENTRY GpiQueryCharBreakExtra(HPS hps, PFIXED BreakExtra);
  LONG  APIENTRY GpiMarker(HPS hps, PPOINTL pptlPoint);
  LONG  APIENTRY GpiPolyMarker(HPS hps, LONG lCount, PPOINTL aptlPoints);
  BOOL  APIENTRY GpiSetMarker(HPS hps, LONG lSymbol);
  BOOL  APIENTRY GpiSetMarkerBox(HPS hps, PSIZEF psizfxSize);
  BOOL  APIENTRY GpiSetMarkerSet(HPS hps, LONG lSet);
  LONG  APIENTRY GpiQueryMarker(HPS hps);
  BOOL  APIENTRY GpiQueryMarkerBox(HPS hps, PSIZEF psizfxSize);
  LONG  APIENTRY GpiQueryMarkerSet(HPS hps);
  LONG  APIENTRY GpiImage(HPS hps, LONG lFormat, PSIZEL psizlImageSize, LONG lLength, PBYTE pbData);
  BOOL  APIENTRY GpiPop(HPS hps, LONG lCount);
  LONG  APIENTRY GpiPtVisible(HPS hps, PPOINTL pptlPoint);
  LONG  APIENTRY GpiRectVisible(HPS hps, PRECTL prclRectangle);
  BOOL  APIENTRY GpiComment(HPS hps, LONG lLength, PBYTE pbData);

  /* GPILCIDS */
  BOOL  APIENTRY GpiQueryRasterizerCaps(PRASTERIZERCAPS pCaps);
  LONG  APIENTRY GpiCreateLogFont(HPS hps, PSTR8 pName, LONG lLcid, PFATTRS pfatAttrs);
  BOOL  APIENTRY GpiDeleteSetId(HPS hps, LONG lLcid);
  BOOL  APIENTRY GpiLoadFonts(HAB hab, PCSZ pszFilename);
  BOOL  APIENTRY GpiUnloadFonts(HAB hab, PCSZ pszFilename);
  LONG  APIENTRY GpiQueryFonts(HPS hps, ULONG flOptions, PCSZ pszFacename, PLONG plReqFonts, LONG lMetricsLength, PFONTMETRICS afmMetrics);
  BOOL  APIENTRY GpiQueryFontMetrics(HPS hps, LONG lMetricsLength, PFONTMETRICS pfmMetrics);
  LONG  APIENTRY GpiQueryKerningPairs(HPS hps, LONG lCount, PKERNINGPAIRS akrnprData);
  BOOL  APIENTRY GpiQueryWidthTable(HPS hps, LONG lFirstChar, LONG lCount, PLONG alData);
  LONG  APIENTRY GpiQueryNumberSetIds(HPS hps);
  BOOL  APIENTRY GpiQuerySetIds(HPS hps, LONG lCount, PLONG alTypes, PSTR8 aNames, PLONG allcids);
  ULONG APIENTRY GpiQueryFaceString(HPS hps, PCSZ FamilyName, PFACENAMEDESC attrs, LONG length, PCSZ CompoundFaceName);
  BOOL  APIENTRY GpiQueryLogicalFont(HPS hps, LONG lcid, PSTR8 name, PFATTRS attrs, LONG length);
  ULONG APIENTRY GpiQueryFontAction(HAB anchor, ULONG options);
  BOOL  APIENTRY GpiLoadPublicFonts(HAB hab, PCSZ pszFilename);
  BOOL  APIENTRY GpiUnloadPublicFonts(HAB hab, PCSZ pszFilename);
  BOOL  APIENTRY GpiSetCp(HPS hps, ULONG ulCodePage);
  ULONG APIENTRY GpiQueryCp(HPS hps);
  LONG  APIENTRY GpiQueryFullFontFileDescs(HAB hab, PCSZ pszFilename, PLONG plCount, PVOID pNames, PLONG plNamesBuffLength);
  ULONG APIENTRY GpiSetFontMappingFlags(HPS hps, ULONG flags);
  ULONG APIENTRY GpiQueryFontMappingFlags(HPS hps);
  LONG  APIENTRY GpiQueryFontFileDescriptions(HAB hab, PCSZ pszFilename, PLONG plCount, PFFDESCS affdescsNames);

  /* GPIBITMAPS */
  LONG    APIENTRY GpiBitBlt(HPS hpsTarget, HPS hpsSource, LONG lCount, PPOINTL aptlPoints, LONG lRop, ULONG flOptions);
  BOOL    APIENTRY GpiDeleteBitmap(HBITMAP hbm);
  HBITMAP APIENTRY GpiLoadBitmap(HPS hps, HMODULE Resource, ULONG idBitmap, LONG lWidth, LONG lHeight);
  HBITMAP APIENTRY GpiSetBitmap(HPS hps, HBITMAP hbm);
  LONG    APIENTRY GpiWCBitBlt(HPS hpsTarget, HBITMAP hbmSource, LONG lCount, PPOINTL aptlPoints, LONG lRop, ULONG flOptions);
  HBITMAP APIENTRY GpiCreateBitmap(HPS hps, PBITMAPINFOHEADER2 pbmpNew, ULONG flOptions, PBYTE pbInitData, PBITMAPINFO2 pbmiInfoTable);
  LONG    APIENTRY GpiSetBitmapBits(HPS hps, LONG lScanStart, LONG lScans, PBYTE pbBuffer, PBITMAPINFO2 pbmiInfoTable);
  BOOL    APIENTRY GpiSetBitmapDimension(HBITMAP hbm, PSIZEL psizlBitmapDimension);
  BOOL    APIENTRY GpiSetBitmapId(HPS hps, HBITMAP hbm, LONG lLcid);
  LONG    APIENTRY GpiQueryBitmapBits(HPS hps, LONG lScanStart, LONG lScans, PBYTE pbBuffer, PBITMAPINFO2 pbmiInfoTable);
  BOOL    APIENTRY GpiQueryBitmapDimension(HBITMAP hbm, PSIZEL psizlBitmapDimension);
  HBITMAP APIENTRY GpiQueryBitmapHandle(HPS hps, LONG lLcid);
  BOOL    APIENTRY GpiQueryBitmapParameters(HBITMAP hbm, PBITMAPINFOHEADER pbmpData);
  BOOL    APIENTRY GpiQueryBitmapInfoHeader(HBITMAP hbm, PBITMAPINFOHEADER2 pbmpData);
  BOOL    APIENTRY GpiQueryDeviceBitmapFormats(HPS hps, LONG lCount, PLONG alArray);
  LONG    APIENTRY GpiSetPel(HPS hps, PPOINTL pptlPoint);
  LONG    APIENTRY GpiQueryPel(HPS hps, PPOINTL pptlPoint);
  LONG    APIENTRY GpiFloodFill(HPS hps, LONG lOptions, LONG lColor);
  LONG    APIENTRY GpiDrawBits(HPS hps, PVOID pBits, PBITMAPINFO2 pbmiInfoTable, LONG lCount, PPOINTL aptlPoints, LONG lRop, ULONG flOptions);

  /* GPIREGIONS */
  LONG  APIENTRY GpiCombineRegion(HPS hps, HRGN hrgnDest, HRGN hrgnSrc1, HRGN hrgnSrc2, LONG lMode);
  HRGN  APIENTRY GpiCreateRegion(HPS hps, LONG lCount, PRECTL arclRectangles);
  BOOL  APIENTRY GpiDestroyRegion(HPS hps, HRGN hrgn);
  LONG  APIENTRY GpiEqualRegion(HPS hps, HRGN hrgnSrc1, HRGN hrgnSrc2);
  BOOL  APIENTRY GpiOffsetRegion(HPS hps, HRGN hrgn, PPOINTL pptlOffset);
  LONG  APIENTRY GpiPaintRegion(HPS hps, HRGN hrgn);
  LONG  APIENTRY GpiFrameRegion(HPS hps, HRGN hrgn, PSIZEL thickness);
  LONG  APIENTRY GpiPtInRegion(HPS hps, HRGN hrgn, PPOINTL pptlPoint);
  LONG  APIENTRY GpiQueryRegionBox(HPS hps, HRGN hrgn, PRECTL prclBound);
  BOOL  APIENTRY GpiQueryRegionRects(HPS hps, HRGN hrgn, PRECTL prclBound, PRGNRECT prgnrcControl, PRECTL prclRect);
  LONG  APIENTRY GpiRectInRegion(HPS hps, HRGN hrgn, PRECTL prclRect);
  BOOL  APIENTRY GpiSetRegion(HPS hps, HRGN hrgn, LONG lcount, PRECTL arclRectangles);
  LONG  APIENTRY GpiSetClipRegion(HPS hps, HRGN hrgn, PHRGN phrgnOld);
  HRGN  APIENTRY GpiQueryClipRegion(HPS hps);
  LONG  APIENTRY GpiQueryClipBox(HPS hps, PRECTL prclBound);
  LONG  APIENTRY GpiExcludeClipRectangle(HPS hps, PRECTL prclRectangle);
  LONG  APIENTRY GpiIntersectClipRectangle(HPS hps, PRECTL prclRectangle);
  LONG  APIENTRY GpiOffsetClipRegion(HPS hps, PPOINTL pptlPoint);
  HRGN  APIENTRY GpiCreateRoundRectRegion(HPS hps, PPOINTL pptlPoint, LONG lHround, LONG lVRound);
  HRGN  APIENTRY GpiCreateEllipticRegion(HPS hps, PRECTL prclRect);

  /* GPIMETAFILES */
  BOOL  APIENTRY GpiResumePlay(HPS hps);
  BOOL  APIENTRY GpiSuspendPlay(HPS hps);
  HMF   APIENTRY GpiCopyMetaFile(HMF hmf);
  BOOL  APIENTRY GpiDeleteMetaFile(HMF hmf);
  HMF   APIENTRY GpiLoadMetaFile(HAB hab, PCSZ pszFilename);
  LONG  APIENTRY GpiPlayMetaFile(HPS hps, HMF hmf, LONG lCount1, PLONG alOptarray, PLONG plSegCount, LONG lCount2, PCSZ pszDesc);
  BOOL  APIENTRY GpiQueryMetaFileBits(HMF hmf, LONG lOffset, LONG lLength, PBYTE pbData);
  LONG  APIENTRY GpiQueryMetaFileLength(HMF hmf);
  BOOL  APIENTRY GpiSaveMetaFile(HMF hmf, PCSZ pszFilename);
  BOOL  APIENTRY GpiSetMetaFileBits(HMF hmf, LONG lOffset, LONG lLength, PBYTE pbBuffer);

  /* GPIDEFAULTS */
  BOOL  APIENTRY GpiQueryDefArcParams(HPS hps, PARCPARAMS parcpArcParams);
  BOOL  APIENTRY GpiQueryDefAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask, PBUNDLE ppbunAttrs);
  BOOL  APIENTRY GpiQueryDefTag(HPS hps, PLONG plTag);
  BOOL  APIENTRY GpiQueryDefViewingLimits(HPS hps, PRECTL prclLimits);
  BOOL  APIENTRY GpiSetDefArcParams(HPS hps, PARCPARAMS parcpArcParams);
  BOOL  APIENTRY GpiSetDefAttrs(HPS hps, LONG lPrimType, ULONG flAttrMask, PBUNDLE ppbunAttrs);
  BOOL  APIENTRY GpiSetDefTag(HPS hps, LONG lTag);
  BOOL  APIENTRY GpiSetDefViewingLimits(HPS hps, PRECTL prclLimits);

  /* GPIPOLYGON */
  LONG  APIENTRY GpiPolygons(HPS hps, ULONG ulCount, PPOLYGON paplgn, ULONG flOptions, ULONG flModel);
  HRGN  APIENTRY GpiCreatePolygonRegion(HPS hps, ULONG ulCount, PPOLYGON paplgn, ULONG flOptions);

  /* GPIINK */
  BOOL  APIENTRY GpiBeginInkPath(HPS hps, LONG lPath, ULONG flOptions);
  BOOL  APIENTRY GpiEndInkPath(HPS hps, ULONG flOptions);
  LONG  APIENTRY GpiStrokeInkPath(HPS hps, LONG lPath, LONG lCount, PPOINTL aptlPoints, ULONG flOptions);



  /* Server-side placeholder entry points from pmgpi.c
     ("fix prototype !!!"). Real prototypes are unknown yet;
     each is a no-op stub in gpi_manual.cpp. Declaring them here
     gives them extern "C" linkage so .def EXPORTS matches. */
EXTERN ULONG APIENTRY GpiSetDCOwner(void);
EXTERN ULONG APIENTRY GpiSetBitmapOwner(void);
EXTERN ULONG APIENTRY GpiSetRegionOwner(void);
EXTERN ULONG APIENTRY GpiInstallIFIFont(void);
EXTERN ULONG APIENTRY GpiOpenMetafile(void);
EXTERN ULONG APIENTRY GpiCloseMetafile(void);
EXTERN ULONG APIENTRY GpiConvPSH(void);
EXTERN ULONG APIENTRY GpiMtAssociate(void);
EXTERN ULONG APIENTRY GpiMtDisassociate(void);
EXTERN ULONG APIENTRY GpiEnableYInversion(void);
EXTERN ULONG APIENTRY GpiQueryYInversion(void);
EXTERN ULONG APIENTRY GpiSaveMetaFile2(void);
EXTERN ULONG APIENTRY GpiAllocateDCData(void);
EXTERN ULONG APIENTRY GpiQueryDCData(void);

  /* 32-bit mixed-case Dev* entry points (uppercase DEV* are 16-bit
     thunks in pmgpi16.cpp; these are the plain 32-bit stubs). */
  HMF    APIENTRY DevCloseDC(HDC);
  HDC    APIENTRY DevOpenDC(HAB, LONG, PCSZ, LONG, PDEVOPENDATA, HDC);
  LONG   APIENTRY DevPostDeviceModes(HAB, PDRIVDATA, PCSZ, PCSZ, PCSZ, ULONG);
  BOOL   APIENTRY DevQueryCaps(HDC, LONG, LONG, PLONG);
  LONG   APIENTRY DevEscape(HDC, LONG, LONG, PBYTE, PLONG, PBYTE);
  BOOL   APIENTRY DevQueryDeviceNames(HAB, PCSZ, PLONG, PSTR32, PSTR64, PLONG, PSTR16);
  LONG   APIENTRY DevQueryHardcopyCaps(HDC, LONG, LONG, PHCINFO);

  /* ---------- Winsock (без <windows.h>) ---------- */

  #ifndef INADDR_ANY
    #define INADDR_ANY       ((ULONG)0x00000000)
  #endif

  #ifndef SOCKET
    typedef UINT SOCKET;
  #endif
  #ifndef INVALID_SOCKET
    #define INVALID_SOCKET   ((SOCKET)(~0))
  #endif
  #ifndef SOCKET_ERROR
    #define SOCKET_ERROR     (-1)
  #endif

  #ifndef AF_INET
    #define AF_INET          2
  #endif
  #ifndef SOCK_STREAM
    #define SOCK_STREAM      1
  #endif
  #ifndef SOL_SOCKET
    #define SOL_SOCKET       0xFFFF
  #endif
  #ifndef SO_REUSEADDR
    #define SO_REUSEADDR     0x0004
  #endif
  #ifndef SHUT_WR
    #define SHUT_WR          1
  #endif
  #ifndef SD_SEND
    #define SD_SEND          1
  #endif

  struct in_addr {
      ULONG s_addr;
  };
  struct sockaddr {
      USHORT sa_family;
      CHAR   sa_data[14];
  };
  struct sockaddr_in {
      SHORT  sin_family;
      USHORT sin_port;
      struct in_addr sin_addr;
      CHAR   sin_zero[8];
  };

  typedef struct WSAData {
      USHORT wVersion;
      USHORT wHighVersion;
      char   szDescription[257];
      char   szSystemStatus[129];
      USHORT iMaxSockets;
      USHORT iMaxUdpDg;
      char  *lpVendorInfo;
  } WSADATA, *LPWSADATA;

  #ifndef MAKEWORD
    #define MAKEWORD(a, b) ((USHORT)(((BYTE)(a)) | ((BYTE)(b)) << 8))
  #endif

  #define WSAEINTR          10004
  #define WSAECONNREFUSED   10061
  #define WSAECONNABORTED   10053
  #define WSAECONNRESET     10054
  #define WSANOTINITIALISED 10093

  __declspec(dllimport) int __stdcall WSAStartup(USHORT wVersionRequested,
                                                 LPWSADATA lpWSAData);
  __declspec(dllimport) int __stdcall WSACleanup(void);
  __declspec(dllimport) int __stdcall WSAGetLastError(void);


  #define MAXGETHOSTSTRUCT 1024

  struct hostent {
      char  *h_name;
      char **h_aliases;
      short  h_addrtype;
      short  h_length;
      char **h_addr_list;
  };
  #define h_addr h_addr_list[0]

  __declspec(dllimport) struct hostent * __stdcall gethostbyname(const char *name);

  __declspec(dllimport) SOCKET __stdcall socket(int af, int type, int protocol);
  __declspec(dllimport) int    __stdcall connect(SOCKET s,
                                                 const struct sockaddr *name,
                                                 int namelen);
  __declspec(dllimport) int    __stdcall bind(SOCKET s,
                                              const struct sockaddr *name,
                                              int namelen);
  __declspec(dllimport) int    __stdcall listen(SOCKET s, int backlog);
  __declspec(dllimport) SOCKET __stdcall accept(SOCKET s,
                                                struct sockaddr *addr,
                                                int *addrlen);
  __declspec(dllimport) int    __stdcall send(SOCKET s,
                                              const char *buf, int len, int flags);
  __declspec(dllimport) int    __stdcall recv(SOCKET s,
                                              char *buf, int len, int flags);
  __declspec(dllimport) int    __stdcall closesocket(SOCKET s);
  __declspec(dllimport) int    __stdcall shutdown(SOCKET s, int how);
  __declspec(dllimport) int    __stdcall setsockopt(SOCKET s, int level,
                                                    int optname,
                                                    const char *optval,
                                                    int optlen);
  __declspec(dllimport) USHORT __stdcall htons(USHORT hostshort);
  __declspec(dllimport) ULONG  __stdcall htonl(ULONG hostlong);
  __declspec(dllimport) USHORT __stdcall ntohs(USHORT netshort);
  __declspec(dllimport) ULONG  __stdcall ntohl(ULONG netlong);
  __declspec(dllimport) ULONG  __stdcall inet_addr(const char *cp);

/* ---------- Winsock additions: select, non-blocking connect ---------- */

#ifndef INADDR_NONE
    #define INADDR_NONE      ((ULONG)0xFFFFFFFF)
#endif

#ifndef FD_SETSIZE
    #define FD_SETSIZE 64
#endif

typedef struct fd_set {
    unsigned int fd_count;
    SOCKET       fd_array[FD_SETSIZE];
} fd_set;

__declspec(dllimport) int __stdcall __WSAFDIsSet(SOCKET, fd_set *);

#define FD_ZERO(set) ((set)->fd_count = 0)
#define FD_SET(s, set) do { \
    if ((set)->fd_count < FD_SETSIZE) \
        (set)->fd_array[(set)->fd_count++] = (s); \
} while (0)
#define FD_ISSET(s, set) (__WSAFDIsSet((s), (set)))

struct timeval {
    long tv_sec;
    long tv_usec;
};

__declspec(dllimport) int __stdcall select(int nfds, fd_set *rfds, fd_set *wfds,
                                           fd_set *efds, struct timeval *timeout);
__declspec(dllimport) int __stdcall ioctlsocket(SOCKET s, long cmd,
                                                 unsigned long *argp);
__declspec(dllimport) int __stdcall getsockopt(SOCKET s, int level, int optname,
                                               char *optval, int *optlen);

#define FIONBIO  0x8004667E

#ifndef WSAEWOULDBLOCK
    #define WSAEWOULDBLOCK 10035
#endif
#ifndef WSAEINPROGRESS
    #define WSAEINPROGRESS 10036
#endif
#ifndef WSAETIMEDOUT
    #define WSAETIMEDOUT   10060
#endif

#ifndef SO_ERROR
    #define SO_ERROR       0x1007
#endif
#ifndef SO_KEEPALIVE
    #define SO_KEEPALIVE   0x0008
#endif
#ifndef TCP_NODELAY
    #define TCP_NODELAY    0x0001
#endif
#ifndef IPPROTO_TCP
    #define IPPROTO_TCP    6
#endif

/* ---------- Named pipes (Win32 emulation of OS/2 Dos*NPipe) ---------- */

typedef void *HPIPE;
#define NULLHANDLE ((HPIPE)0)
#define INVALID_HANDLE_VALUE ((void *)(long)-1)

/* OS/2 pipe-mode constants. */
#define NP_ACCESS_DUPLEX        0x00000003UL
#define NP_NOINHERIT            0x00000080UL
#define NP_WMESG                0x00000400UL
#define NP_RMESG                0x00000100UL
#define NP_TYPE_BYTE            0x00000000UL
#define NP_TYPE_MESSAGE         0x00000400UL

/* OS/2 DosOpen constants (subset needed by FreePM transport). */
#define OPEN_ACTION_OPEN_IF_EXISTS   2L
#define OPEN_FLAGS_WRITE_THROUGH     0x00000002UL
#define OPEN_FLAGS_FAIL_ON_ERROR     0x00000004UL
#define OPEN_FLAGS_RANDOM            0x00000008UL
#define OPEN_SHARE_DENYNONE          0x00000040UL
#define OPEN_ACCESS_READWRITE        0x00000002UL

  #define ERROR_PIPE_BUSY            231L
  #define ERROR_PIPE_CONNECTED       535L
  #define PIPE_ACCESS_DUPLEX         0x00000003UL
  #define PIPE_TYPE_BYTE             0x00000000UL
  #define PIPE_READMODE_BYTE         0x00000000UL
  #define PIPE_WAIT                  0x00000000UL
  #define PIPE_UNLIMITED_INSTANCES   255
  #define GENERIC_READ               0x80000000UL
  #define GENERIC_WRITE              0x40000000UL
  #define OPEN_EXISTING              3


static inline APIRET DosCreateNPipe(const char *pszName, HPIPE *phpipe,
                                    ULONG flOpenMode, ULONG flPipeMode,
                                    ULONG cbOutBuf, ULONG cbInBuf,
                                    ULONG ulTimeout)
{
    DWORD nInst = (flPipeMode & 0xFF);
    if (nInst == 0) nInst = PIPE_UNLIMITED_INSTANCES;
    (void)flOpenMode;   /* DUPLEX only */
    (void)flPipeMode;   /* byte mode only */
    *phpipe = (HPIPE)CreateNamedPipeA(pszName, PIPE_ACCESS_DUPLEX,
                                      PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                                      nInst, cbOutBuf, cbInBuf, ulTimeout, NULL);
    return (*phpipe != INVALID_HANDLE_VALUE) ? 0 : 1;
}

static inline APIRET DosConnectNPipe(HPIPE hpipe)
{
    if (ConnectNamedPipe((HANDLE)hpipe, NULL)) return 0;
    if (GetLastError() == ERROR_PIPE_CONNECTED) return 0;  /* already connected */
    return 1;
}

static inline APIRET DosDisConnectNPipe(HPIPE hpipe)
{
    return DisconnectNamedPipe((HANDLE)hpipe) ? 0 : 1;
}

static inline APIRET DosClose(HPIPE hpipe)
{
    return CloseHandle((HANDLE)hpipe) ? 0 : 1;
}

static inline APIRET DosOpen(const char *pszName, HPIPE *phpipe,
                             ULONG *pulAction, ULONG cbFile, ULONG ulAttribute,
                             ULONG flOpenFlag, ULONG flOpenMode, ULONG ulTimeout)
{
    (void)cbFile; (void)ulAttribute; (void)flOpenFlag; (void)flOpenMode;
    (void)ulTimeout;
    *phpipe = (HPIPE)CreateFileA(pszName, GENERIC_READ | GENERIC_WRITE,
                                 0, NULL, OPEN_EXISTING, 0, NULL);
    if (*phpipe == (HPIPE)INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (err == ERROR_PIPE_BUSY) return ERROR_PIPE_BUSY;
        return (APIRET)err;
    }
    if (pulAction) *pulAction = 0;
    return 0;
}

static inline APIRET DosWaitNPipe(const char *pszName, ULONG ulTimeout)
{
    if (WaitNamedPipeA(pszName, (DWORD)ulTimeout)) return 0;
    return (APIRET)GetLastError();
}

static inline APIRET DosRead(HPIPE hpipe, void *pv, ULONG cb, ULONG *pcbRead)
{
    DWORD dwRead = 0;
    if (!ReadFile((HANDLE)hpipe, pv, cb, &dwRead, NULL)) {
        *pcbRead = 0;
        return GetLastError();   /* ERROR_BROKEN_PIPE / ERROR_OPERATION_ABORTED */
    }
    *pcbRead = (ULONG)dwRead;
    return 0;
}

static inline APIRET DosWrite(HPIPE hpipe, const void *pv, ULONG cb,
                              ULONG *pcbWritten)
{
    DWORD dwWritten = 0;
    if (!WriteFile((HANDLE)hpipe, pv, cb, &dwWritten, NULL)) {
        *pcbWritten = 0;
        return GetLastError();
    }
    *pcbWritten = (ULONG)dwWritten;
    return 0;
}

/* ---------- Queues (Win32 emulation of OS/2 Dos*Queue) ---------- */


#ifndef PID
  typedef ULONG PID;
#endif

#ifndef HQUEUE
  typedef PVOID HQUEUE;
#endif

#ifndef QUE_FIFO
  #define QUE_FIFO      0
  #define QUE_LIFO      1
  #define QUE_PRIORITY  2
#endif
#ifndef DCWW_WAIT
  #define DCWW_WAIT     0
  #define DCWW_NOWAIT   1
#endif

typedef struct _REQUESTDATA {
    ULONG ulData;
    PID   pid;
} REQUESTDATA, *PREQUESTDATA;

#ifndef HEV
  typedef void *HEV;
#endif

typedef void *HEV;

#define WQ_MAGIC         0x57513031UL   /* 'WQ01' */
#define WQ_MAX_ELEMENTS  8              /* 1-2 in flight; spare capacity */
#define WQ_MAX_ELEMENT   16             /* 8B notification + 9B handshake */

#define WQ_MAP_SIZE      (sizeof(WQHeader) + \
                          WQ_MAX_ELEMENTS * sizeof(WQSlot) + \
                          WQ_MAX_ELEMENTS * WQ_MAX_ELEMENT)

typedef struct _WQHeader {
    ULONG magic;
    ULONG attrs;
    ULONG maxElements;
    ULONG elementSize;
    ULONG writeIdx;
    ULONG count;
    PID   ownerPid;
} WQHeader;

typedef struct _WQSlot {
    ULONG ulRequest;
    ULONG cbData;
    PID   pid;
} WQSlot;

typedef struct _WQQueue {
    HANDLE    hMap;
    HANDLE    hMtx;
    HANDLE    hSemFull;
    HANDLE    hSemFree;
    WQHeader *pHeader;
    BYTE     *pSlots;
    BYTE     *pData;
    int       isOwner;
} WQQueue;

static inline void wq_build_names(const char *pszName,
                                  char *mapName, char *mtxName,
                                  char *semFullName, char *semFreeName,
                                  int cbMax)
{
    const char *p = pszName ? pszName : "";
    const char *suf[4] = { "_MAP", "_MTX", "_FL", "_FR" };
    char *outs[4];
    int  k, i;

    outs[0] = mapName; outs[1] = mtxName;
    outs[2] = semFullName; outs[3] = semFreeName;

    for (k = 0; k < 4; k++) {
        char *o = outs[k];
        memcpy(o, "Local\\", 6);
        o += 6;
        for (i = 0; p[i] && (int)(o - outs[k]) < cbMax - 12; i++) {
            *o++ = (p[i] == '\\') ? '_' : p[i];
        }
        for (i = 0; suf[k][i]; i++) *o++ = suf[k][i];
        *o = 0;
    }
}

static inline APIRET DosCreateQueue(HQUEUE *phq, ULONG priority,
                                    const char *pszName)
{
    char mapName[300], mtxName[300], semFullName[300], semFreeName[300];
    WQQueue *q;
    HANDLE hMap;
    void *pView;

    wq_build_names(pszName, mapName, mtxName, semFullName, semFreeName,
                   sizeof(mapName));

    hMap = CreateFileMappingA((HANDLE)(long)-1, NULL, PAGE_READWRITE, 0,
                              (DWORD)WQ_MAP_SIZE, mapName);
    if (!hMap) return (APIRET)GetLastError();
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMap);
        return ERROR_QUE_DUPLICATE;
    }

    pView = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!pView) { CloseHandle(hMap); return (APIRET)GetLastError(); }

    q = (WQQueue *)VirtualAlloc(NULL, sizeof(WQQueue), MEM_COMMIT,
                                PAGE_READWRITE);
    if (!q) { UnmapViewOfFile(pView); CloseHandle(hMap); return ERROR_QUE_NO_MEMORY; }

    q->pHeader = (WQHeader *)pView;
    q->pSlots  = (BYTE *)pView + sizeof(WQHeader);
    q->pData   = q->pSlots + WQ_MAX_ELEMENTS * sizeof(WQSlot);
    q->hMap    = hMap;
    q->isOwner = 1;

    q->pHeader->magic       = WQ_MAGIC;
    q->pHeader->attrs       = priority & 0x07;
    q->pHeader->maxElements = WQ_MAX_ELEMENTS;
    q->pHeader->elementSize = WQ_MAX_ELEMENT;
    q->pHeader->writeIdx    = 0;
    q->pHeader->count       = 0;
    q->pHeader->ownerPid    = (PID)GetCurrentProcessId();

    q->hMtx     = CreateMutexA(NULL, FALSE, mtxName);
    q->hSemFull = CreateSemaphoreA(NULL, 0, WQ_MAX_ELEMENTS, semFullName);
    q->hSemFree = CreateSemaphoreA(NULL, WQ_MAX_ELEMENTS, WQ_MAX_ELEMENTS,
                                   semFreeName);

    if (!q->hMtx || !q->hSemFull || !q->hSemFree) goto fail;

    *phq = (HQUEUE)q;
    return 0;

fail:
    if (q->hSemFree) CloseHandle(q->hSemFree);
    if (q->hSemFull) CloseHandle(q->hSemFull);
    if (q->hMtx)     CloseHandle(q->hMtx);
    UnmapViewOfFile(pView);
    CloseHandle(hMap);
    VirtualFree(q, 0, MEM_RELEASE);
    return ERROR_QUE_NO_MEMORY;
}

static inline APIRET DosOpenQueue(PULONG ppid, HQUEUE *phq,
                                  const char *pszName)
{
    char mapName[300], mtxName[300], semFullName[300], semFreeName[300];
    WQQueue *q;
    HANDLE hMap;
    void *pView;
    DWORD mtxAccess, semAccess;

    if (ppid) *ppid = 0;

    wq_build_names(pszName, mapName, mtxName, semFullName, semFreeName,
                   sizeof(mapName));

    hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, mapName);
    if (!hMap) return ERROR_QUE_NAME_NOT_EXIST;

    pView = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!pView) { CloseHandle(hMap); return (APIRET)GetLastError(); }

    q = (WQQueue *)VirtualAlloc(NULL, sizeof(WQQueue), MEM_COMMIT,
                                PAGE_READWRITE);
    if (!q) { UnmapViewOfFile(pView); CloseHandle(hMap); return ERROR_QUE_NO_MEMORY; }

    q->pHeader = (WQHeader *)pView;
    q->pSlots  = (BYTE *)pView + sizeof(WQHeader);
    q->pData   = q->pSlots + WQ_MAX_ELEMENTS * sizeof(WQSlot);
    q->hMap    = hMap;
    q->isOwner = 0;

    if (q->pHeader->magic != WQ_MAGIC) goto fail;

    /* Caller will WaitForSingleObject, ReleaseMutex / ReleaseSemaphore on
       these handles. Need MODIFY_STATE in addition to SYNCHRONIZE. */
    mtxAccess = SYNCHRONIZE | MUTEX_MODIFY_STATE;
    semAccess = SYNCHRONIZE | SEMAPHORE_MODIFY_STATE;

    q->hMtx     = OpenMutexA(mtxAccess, FALSE, mtxName);
    q->hSemFull = OpenSemaphoreA(semAccess, FALSE, semFullName);
    q->hSemFree = OpenSemaphoreA(semAccess, FALSE, semFreeName);

    if (!q->hMtx || !q->hSemFull || !q->hSemFree) goto fail;

    if (ppid) *ppid = (ULONG)q->pHeader->ownerPid;

    *phq = (HQUEUE)q;
    return 0;

fail:
    if (q->hSemFree) CloseHandle(q->hSemFree);
    if (q->hSemFull) CloseHandle(q->hSemFull);
    if (q->hMtx)     CloseHandle(q->hMtx);
    UnmapViewOfFile(pView);
    CloseHandle(hMap);
    VirtualFree(q, 0, MEM_RELEASE);
    return ERROR_QUE_INVALID_HANDLE;
}

static inline APIRET DosWriteQueue(HQUEUE hq, ULONG ulRequest, ULONG cbData,
                                   PVOID pBuf, ULONG priority)
{
    WQQueue *q = (WQQueue *)hq;
    ULONG idx;
    WQSlot *slot;

    (void)priority;
    if (!q) return ERROR_QUE_INVALID_HANDLE;
    if (cbData > WQ_MAX_ELEMENT) return ERROR_QUE_NO_MEMORY;

    if (WaitForSingleObject(q->hSemFree, INFINITE) != WAIT_OBJECT_0)
        return ERROR_QUE_INVALID_HANDLE;
    if (WaitForSingleObject(q->hMtx, INFINITE) != WAIT_OBJECT_0) {
        ReleaseSemaphore(q->hSemFree, 1, NULL);
        return ERROR_QUE_INVALID_HANDLE;
    }

    idx  = q->pHeader->writeIdx;
    slot = (WQSlot *)(q->pSlots + idx * sizeof(WQSlot));
    slot->ulRequest = ulRequest;
    slot->cbData    = cbData;
    slot->pid       = (PID)GetCurrentProcessId();
    if (cbData && pBuf)
        memcpy(q->pData + (size_t)idx * WQ_MAX_ELEMENT, pBuf, cbData);

    q->pHeader->writeIdx = (idx + 1) % WQ_MAX_ELEMENTS;
    q->pHeader->count++;

    if (!ReleaseMutex(q->hMtx)) {
        return (APIRET)GetLastError();
    }
    if (!ReleaseSemaphore(q->hSemFull, 1, NULL)) {
        return (APIRET)GetLastError();
    }
    return 0;
}

static inline APIRET DosReadQueue(HQUEUE hq, PREQUESTDATA pReqData,
                                  PULONG pcbData, PVOID *ppElemHandle,
                                  ULONG element, ULONG wait,
                                  PBYTE pPriority, HEV hsem)
{
    WQQueue *q = (WQQueue *)hq;
    ULONG idx, cbMax, cbActual;
    DWORD waitMs;
    WQSlot *slot;
    PVOID pElem;

    (void)element;
    (void)hsem;
    if (!q || !pcbData || !ppElemHandle) return ERROR_QUE_INVALID_HANDLE;

    cbMax  = *pcbData;
    waitMs = (wait == DCWW_NOWAIT) ? 0 : INFINITE;

    if (WaitForSingleObject(q->hSemFull, waitMs) != WAIT_OBJECT_0) {
        if (wait == DCWW_NOWAIT) return ERROR_QUE_EMPTY;
        return ERROR_QUE_INVALID_HANDLE;
    }
    if (WaitForSingleObject(q->hMtx, INFINITE) != WAIT_OBJECT_0) {
        ReleaseSemaphore(q->hSemFull, 1, NULL);
        return ERROR_QUE_INVALID_HANDLE;
    }

    idx      = (q->pHeader->writeIdx - q->pHeader->count + WQ_MAX_ELEMENTS)
               % WQ_MAX_ELEMENTS;
    slot     = (WQSlot *)(q->pSlots + idx * sizeof(WQSlot));
    cbActual = slot->cbData;
    if (cbActual > cbMax) cbActual = cbMax;

    pElem = VirtualAlloc(NULL, cbActual ? cbActual : 1, MEM_COMMIT,
                         PAGE_READWRITE);
    if (!pElem) {
        ReleaseMutex(q->hMtx);
        ReleaseSemaphore(q->hSemFull, 1, NULL);
        return ERROR_QUE_NO_MEMORY;
    }
    if (cbActual)
        memcpy(pElem, q->pData + (size_t)idx * WQ_MAX_ELEMENT, cbActual);

    if (pReqData) {
        pReqData->ulData = slot->ulRequest;
        pReqData->pid    = slot->pid;
    }
    *pcbData      = cbActual;
    *ppElemHandle = pElem;
    if (pPriority) *pPriority = 0;

    q->pHeader->count--;

    if (!ReleaseMutex(q->hMtx)) {
        return (APIRET)GetLastError();
    }
    if (!ReleaseSemaphore(q->hSemFree, 1, NULL)) {
        return (APIRET)GetLastError();
    }
    return 0;
}

static inline APIRET DosCloseQueue(HQUEUE hq)
{
    WQQueue *q = (WQQueue *)hq;
    if (!q) return 0;
    if (q->hSemFree) CloseHandle(q->hSemFree);
    if (q->hSemFull) CloseHandle(q->hSemFull);
    if (q->hMtx)     CloseHandle(q->hMtx);
    UnmapViewOfFile(q->pHeader);
    CloseHandle(q->hMap);
    VirtualFree(q, 0, MEM_RELEASE);
    return 0;
}

static inline APIRET DosQueryQueue(HQUEUE hq, PULONG pcbEntries)
{
    WQQueue *q = (WQQueue *)hq;
    if (!q) return 1;
    if (WaitForSingleObject(q->hMtx, INFINITE) != WAIT_OBJECT_0) return 1;
    if (pcbEntries) *pcbEntries = q->pHeader->count;
    ReleaseMutex(q->hMtx);
    return 0;
}

/* ---------- Event semaphores (Win32 emulation of OS/2 Dos*EventSem) ----------
 *
 * OS/2 event semaphores carry a post count; DosPostEventSem increments
 * it, DosWaitEventSem waits for it to be non-zero, DosResetEventSem
 * clears it. The Win32 emulation uses a manual-reset event: the
 * producer's SetEvent is idempotent, and the consumer resets the
 * event under its own lock when the last queued item is drained, so
 * no wakeup is lost.
 */

__declspec(dllimport) HANDLE __stdcall CreateEventA(void *lpEventAttributes,
                                                     BOOL bManualReset,
                                                     BOOL bInitialState,
                                                     const char *lpName);
__declspec(dllimport) BOOL   __stdcall SetEvent(HANDLE hEvent);
__declspec(dllimport) BOOL   __stdcall ResetEvent(HANDLE hEvent);

static inline APIRET DosCreateEventSem(const char *pszName, HEV *phev,
                                       ULONG flAttr, BOOL fState)
{
    (void)flAttr;
    *phev = CreateEventA(NULL, TRUE, fState ? TRUE : FALSE, pszName);
    return (*phev != NULL) ? 0 : 1;
}

static inline APIRET DosPostEventSem(HEV hev)
{
    return SetEvent((HANDLE)hev) ? 0 : 1;
}

static inline APIRET DosResetEventSem(HEV hev, PULONG pulPostCt)
{
    BOOL ok = ResetEvent((HANDLE)hev);
    if (pulPostCt) *pulPostCt = 0;
    return ok ? 0 : 1;
}

static inline APIRET DosWaitEventSem(HEV hev, ULONG ulTimeout)
{
    DWORD dwTimeout = (ulTimeout == SEM_INDEFINITE_WAIT)
                      ? INFINITE : (DWORD)ulTimeout;
    DWORD dwWait = WaitForSingleObject((HANDLE)hev, dwTimeout);
    return (dwWait == WAIT_OBJECT_0) ? 0 : 1;
}

static inline APIRET DosCloseEventSem(HEV hev)
{
    return CloseHandle((HANDLE)hev) ? 0 : 1;
}

  #define WS_VISIBLE   0x80000000UL

  #define SWP_MOVE     0x0002UL
  #define SWP_SIZE     0x0004UL
  #define SWP_SHOW     0x0010UL
  #define SWP_HIDE     0x0020UL

  /* Window messages (subset used by the server-side default window
   * procedure; the full set comes from <os2.h> on OS/2). */
  #define WM_USER      0x0400
  #define WM_CLOSE     0x002BUL
  #define WM_QUIT      0x001AUL

  #define CLR_BLACK       (-1L)
  #define CLR_BACKGROUND     0L
  #define CLR_BLUE           1L
  #define CLR_RED            2L
  #define CLR_PINK           3L
  #define CLR_GREEN          4L
  #define CLR_CYAN           5L
  #define CLR_YELLOW         6L
  #define CLR_NEUTRAL        7L
  #define CLR_WHITE          0L

  #define HWND_DESKTOP     ((HWND)1)

  #define GPIA_ASSOC       0x0001L

  #define DRO_FILL         1L
  #define DRO_OUTLINE      2L
  #define DRO_OUTLINEFILL  3L

  #define PMERR_INVALID_HAB                   0x104A

#ifdef __cplusplus
  }
#endif

#else
  #error Unsupported platform for PMGPI
#endif


#ifdef __cplusplus
      extern "C" {
#endif

#define SEGEM_INSERT   1L
#define SEGEM_REPLACE  2L

#ifdef __cplusplus
BOOL APIENTRY GpiBeginElement(HPS, LONG, PCSZ);
#else
BOOL APIENTRY GpiBeginElement(HPS, LONG, PSZ);
#endif

BOOL APIENTRY GpiSetEditMode(HPS, LONG);
LONG APIENTRY GpiQueryEditMode(HPS);
BOOL APIENTRY GpiSetElementPointer(HPS, LONG);
LONG APIENTRY GpiQueryElementPointer(HPS);
BOOL APIENTRY GpiOffsetElementPointer(HPS, LONG);
BOOL APIENTRY GpiDeleteElement(HPS);
BOOL APIENTRY GpiDeleteElementRange(HPS, LONG, LONG);
BOOL APIENTRY GpiLabel(HPS, LONG);
BOOL APIENTRY GpiSetElementPointerAtLabel(HPS, LONG);
BOOL APIENTRY GpiDeleteElementsBetweenLabels(HPS, LONG, LONG);

#ifdef __cplusplus
LONG APIENTRY GpiQueryElementType(HPS, PLONG, LONG, PCSZ);
#else
LONG APIENTRY GpiQueryElementType(HPS, PLONG, LONG, PSZ);
#endif

LONG APIENTRY GpiQueryElement(HPS, LONG, LONG, PBYTE);

#ifdef __cplusplus
LONG APIENTRY GpiElement(HPS, LONG, PCSZ, LONG, PBYTE);
#else
LONG APIENTRY GpiElement(HPS, LONG, PSZ, LONG, PBYTE);
#endif

BOOL APIENTRY GpiEndElement(HPS);

BOOL APIENTRY GpiQueryArcParams(HPS, PARCPARAMS);

  /* ---------- PMWIN entry points ---------- */
  HAB   APIENTRY WinInitialize(ULONG flOptions);
  HMQ   APIENTRY WinCreateMsgQueue(HAB hab, LONG cMsg);
  BOOL  APIENTRY WinDestroyMsgQueue(HMQ hmq);
  BOOL  APIENTRY WinTerminate(HAB hab);
  HDC   APIENTRY WinOpenWindowDC(HWND hwnd);
  HPS   APIENTRY WinGetScreenPS(HWND hwndDesktop);
  BOOL  APIENTRY WinReleasePS(HPS hps);
  HWND  APIENTRY WinCreateWindow(HWND hwndParent, PCSZ pszClass,
                                 PCSZ pszName, ULONG flStyle,
                                 LONG x, LONG y, LONG cx, LONG cy,
                                 HWND hwndOwner, HWND hwndInsertBehind,
                                 ULONG id, PVOID pCtlData, PVOID pPresParams);
  BOOL  APIENTRY WinShowWindow(HWND hwnd, BOOL fShow);
  BOOL  APIENTRY WinSetWindowPos(HWND hwnd, HWND hwndInsertBehind,
                                 LONG x, LONG y, LONG cx, LONG cy, ULONG fl);
  BOOL  APIENTRY WinDestroyWindow(HWND hwnd);
  BOOL  APIENTRY WinPostMsg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
  BOOL  APIENTRY WinGetMsg(HAB hab, PQMSG pqmsg, HWND hwndFilter,
                           ULONG msgFirst, ULONG msgLast);
  MRESULT APIENTRY WinDispatchMsg(HAB hab, PQMSG pqmsg);


#ifdef __cplusplus
  }
#endif


#endif /* F_PLATFORM_H */
