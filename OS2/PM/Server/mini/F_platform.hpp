/* F_platform.hpp */
/* Platform abstraction for PMCLIENT.
   Same PMCLIENT sources build for OS/2 and Win32.
   Only the macros and helpers below are platform-specific. */

#ifndef PMC_PLATFORM_HPP
#define PMC_PLATFORM_HPP

#if defined(__OS2__) || defined(__OS2V2__)

  #define INCL_DOS
  #define INCL_DOSERRORS
  #include <os2.h>

  typedef HMODULE  PMC_HMODULE;
  typedef HMTX     PMC_HMTX;

  #define PMC_ERR_OPEN_FAILED   ERROR_OPEN_FAILED
  #define PMC_ERR_BROKEN_PIPE   ERROR_BROKEN_PIPE

  /* Load a DLL by name. Returns 0 on success. */
  static inline APIRET PMC_LoadModule(const char *pszName, PMC_HMODULE *phmod)
  {
      return DosLoadModule(NULL, 0, (PSZ)pszName, phmod);
  }

  /* Resolve an ordinal in a loaded DLL. Returns 0 on success. */
  static inline APIRET PMC_QueryOrdinal(PMC_HMODULE hmod, ULONG ulOrdinal, void **ppfn)
  {
      PFN    pfn = 0;
      APIRET rc  = DosQueryProcAddr(hmod, ulOrdinal, 0, &pfn);
      *ppfn = (void *)pfn;
      return rc;
  }

  #define PMC_FreeModule(hmod)   DosFreeModule(hmod)

  /* Read an environment variable. On success returns 0 and sets
     *ppszValue to a pointer valid for the process lifetime. */
  static inline APIRET PMC_ScanEnv(const char *pszName, char **ppszValue)
  {
      return DosScanEnv((PSZ)pszName, (PSZ *)ppszValue);
  }

  #define PMC_Sleep(ms)          DosSleep(ms)

  /* Try to open a named system mutex. Returns 0 on success. */
  static inline APIRET PMC_MutexOpen(const char *pszName, PMC_HMTX *phmtx)
  {
      return DosOpenMutexSem((PSZ)pszName, phmtx);
  }

  #define PMC_MutexClose(h)      DosCloseMutexSem(h)

  /* Start PMSERVER.EXE detached. Returns 0 on success. */
  static inline APIRET PMC_StartProcess(const char *pszExe)
  {
      RESULTCODES rc;
      char        szFail[CCHMAXPATH];
      return DosExecPgm(szFail, sizeof(szFail), EXEC_BACKGROUND,
                        (PSZ)pszExe, NULL, &rc, (PSZ)pszExe);
  }

  #define PMC_HAS_MUTEX_CHECK  1

#elif defined(__WIN32__) || defined(_WIN32)

  #include <windows.h>

  /* OS/2-compatible types. */
  #ifndef APIRET
    typedef unsigned long APIRET;
  #endif
  #ifndef ULONG
    typedef unsigned long ULONG;
  #endif
  #ifndef PULONG
    typedef ULONG *PULONG;
  #endif
  #ifndef LONG
    typedef long LONG;
  #endif
  #ifndef PLONG
    typedef LONG *PLONG;
  #endif
  #ifndef PVOID
    typedef void *PVOID;
  #endif
  #ifndef PCSZ
    typedef const char *PCSZ;
  #endif
  #ifndef PSZ
    typedef char *PSZ;
  #endif
  #ifdef APIENTRY
    #undef APIENTRY
  #endif
  #ifndef APIENTRY
    #define APIENTRY __stdcall
     #pragma aux __estdcall "*" \
                parm routine [] \
                value struct struct caller [] \
                modify [eax ecx edx]
    #undef EXPENTRY
    #define EXPENTRY __declspec(__pragma("__estdcall"))
  #endif

  typedef HMODULE  PMC_HMODULE;
  typedef HANDLE   PMC_HMTX;

  #define PMC_ERR_OPEN_FAILED   (-1L)
  #define PMC_ERR_BROKEN_PIPE   (-2L)

  static inline APIRET PMC_LoadModule(const char *pszName, PMC_HMODULE *phmod)
  {
      *phmod = LoadLibraryA(pszName);
      return (*phmod != NULL) ? 0 : -1;
  }

  static inline APIRET PMC_QueryOrdinal(PMC_HMODULE hmod, ULONG ulOrdinal, void **ppfn)
  {
      FARPROC pfn = GetProcAddress(hmod, MAKEINTRESOURCEA(ulOrdinal));
      *ppfn = (void *)pfn;
      return (pfn != NULL) ? 0 : -1;
  }

  #define PMC_FreeModule(hmod)   FreeLibrary(hmod)

  static inline APIRET PMC_ScanEnv(const char *pszName, char **ppszValue)
  {
      static char szBuf[1024];
      DWORD dwLen = GetEnvironmentVariableA(pszName, szBuf, sizeof(szBuf));
      if (dwLen == 0 || dwLen >= sizeof(szBuf)) {
          *ppszValue = NULL;
          return -1;
      }
      *ppszValue = szBuf;
      return 0;
  }

  #define PMC_Sleep(ms)          Sleep(ms)

  static inline APIRET PMC_MutexOpen(const char *pszName, PMC_HMTX *phmtx)
  {
      *phmtx = OpenMutexA(SYNCHRONIZE, FALSE, pszName);
      return (*phmtx != NULL) ? 0 : -1;
  }

  #define PMC_MutexClose(h)      CloseHandle(h)

  static inline APIRET PMC_StartProcess(const char *pszExe)
  {
      STARTUPINFOA        si;
      PROCESS_INFORMATION pi;
      char                szCmd[1024];

      ZeroMemory(&si, sizeof(si));
      si.cb = sizeof(si);
      ZeroMemory(&pi, sizeof(pi));

      lstrcpynA(szCmd, pszExe, sizeof(szCmd));
      if (!CreateProcessA(NULL, szCmd, NULL, NULL, FALSE,
                          DETACHED_PROCESS, NULL, NULL, &si, &pi))
          return -1;

      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      return 0;
  }

  #define PMC_HAS_MUTEX_CHECK  1

#else
  #error Unsupported platform for PMCLIENT
#endif

#endif /* PMC_PLATFORM_HPP */
