#ifndef __CPI_SESSION_SYSIO_H__
#define __CPI_SESSION_SYSIO_H__

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

#include <os3/thread.h>
#include <os3/cpi.h>

#define SYSIO_DECL(syscall_name, args, results) \
  struct \
  { \
    struct args in; \
    struct results out; \
  } syscall_name;

namespace OS2::Cpi {
    struct Sysio;
}

struct OS2::Cpi::Sysio
{
    typedef char Path[CCHMAXPATHCOMP];

    typedef char Objname[260];

    enum { ARGS_MAX_LEN = 5 * 1024 };
    typedef char Args[ARGS_MAX_LEN];

    enum { ENV_MAX_LEN = 0x10000 };
    typedef char Env[ENV_MAX_LEN];

    union
    {
        SYSIO_DECL(test, { }, { });

        SYSIO_DECL(cfggetenv, { Path name; }, { APIRET rc; Env value; });

        SYSIO_DECL(cfggetopt, { Path name; }, { APIRET rc; int is_int; int value_int;
                   Path value_str; });

        SYSIO_DECL(appnotify1, { }, { });

        SYSIO_DECL(appnotify2, { os2exec_module_t s; Path pszName;
                   l4_os3_thread_t thread; Objname szLoadError; ULONG cbLoadError;
                   ULONG ret; }, { });

        SYSIO_DECL(appsend, { app_data_t data; }, { APIRET rc; });

        SYSIO_DECL(appget, { }, { APIRET rc; app_data_t data; });

        SYSIO_DECL(exit, { ULONG action; ULONG result; }, { APIRET rc; });

        SYSIO_DECL(execpgm, { long cbObjname; ULONG execFlag;
                   Args pArgs; ULONG arglen; Env pEnv; ULONG envlen; Path pName; },
                   { APIRET rc; Objname pObjname; RESULTCODES pRes; });

        SYSIO_DECL(getpib, { PID pid; }, { APIRET rc; });

        SYSIO_DECL(gettib, { PID pid; TID tid; }, { APIRET rc; });

        SYSIO_DECL(error, { ULONG error; }, { APIRET rc; });

        SYSIO_DECL(querydbcsenv, { ULONG cb; COUNTRYCODE cc; }, { APIRET rc;
                   ULONG cb; Path pBuf; });

        SYSIO_DECL(querycp, { ULONG cb; }, { APIRET rc; ULONG cb; Path arCP; });

        SYSIO_DECL(querycurrentdisk, { }, { APIRET rc; ULONG disknum; });

        SYSIO_DECL(querycurrentdir, { ULONG disknum; ULONG logical; }, { APIRET rc;
                   Path pBuf; ULONG cbBuf; });

        SYSIO_DECL(setcurrentdir, { Path pszDir; }, { APIRET rc; });

        SYSIO_DECL(setdefaultdisk, { ULONG disknum; ULONG logical; }, { APIRET rc; });

        SYSIO_DECL(createeventsem, { Path pszName; ULONG flAttr; ULONG fState; },
                   { APIRET rc; HEV hev; });

        SYSIO_DECL(openeventsem, { Path pszName; }, { APIRET rc; HEV hev; });

        SYSIO_DECL(closeeventsem, { HEV hev; }, { APIRET rc; });

        SYSIO_DECL(getpid, { }, { APIRET rc; PID pid; });

        SYSIO_DECL(getnativeid, { PID pid; TID tid; }, { APIRET rc; l4_os3_thread_t id; });

        SYSIO_DECL(newtib, { PID pid; TID tid; l4_os3_thread_t id; }, { APIRET rc; });

        SYSIO_DECL(destroytib, { PID pid; TID tid; }, { APIRET rc; });

        SYSIO_DECL(execopen, { Path pszName; ULONG flags; ULONG cbLoadError; }, { APIRET rc; ULONG hmod; });

        SYSIO_DECL(execload, { ULONG hmod; ULONG cbLoadError; }, { APIRET rc; });

        SYSIO_DECL(execfree, { ULONG hmod; }, { APIRET rc; });

        SYSIO_DECL(execshare, { ULONG hmod; }, { APIRET rc; });

        SYSIO_DECL(execgetimp, { ULONG hmod; }, { APIRET rc; ULONG index; ULONG imp_hmod; });

        SYSIO_DECL(execgetsect, { ULONG hmod; }, { APIRET rc; ULONG index; });

        SYSIO_DECL(execqueryprocaddr, { ULONG hmod; ULONG ordinal; Path mName; },
                   { APIRET rc; ULONG addr; });

        SYSIO_DECL(execquerymodulehandle, { Path mName; }, { APIRET rc; ULONG hmod; });

        SYSIO_DECL(execquerymodulename, { ULONG hmod; ULONG cbName; }, { APIRET rc; Path pbName; });

        SYSIO_DECL(execallocsharedmem, { ULONG cbSize; Path pszName; ULONG rights; },
                   { APIRET rc; void *addr; ULONG area; });

        SYSIO_DECL(execmapdataspace, { void *addr; ULONG rights; }, { APIRET rc; });

        SYSIO_DECL(execunmapdataspace, { void *addr; }, { APIRET rc; });

        SYSIO_DECL(execgetdataspace, { }, { APIRET rc; void *addr; ULONG size; });

        SYSIO_DECL(execgetsharedmem, { void *pb; }, { APIRET rc; void *addr;
                   ULONG size; PID owner; });

        SYSIO_DECL(execgetnamedsharedmem, { Path pszName; }, { APIRET rc; void *addr;
                   ULONG size; PID owner; });

        SYSIO_DECL(execincrementsharedmemrefcnt, { void *addr; }, { APIRET rc; });

        SYSIO_DECL(execreleasesharedmem, { void *addr; }, { APIRET rc; ULONG count; });

        SYSIO_DECL(fsgetdrivemap, { }, { APIRET rc; ULONG map; });

        SYSIO_DECL(fsread, { HFILE hFile; ULONG cbRead; }, { APIRET rc; ULONG cbActual; });

        SYSIO_DECL(fswrite, { HFILE hFile; ULONG cbWrite; }, { APIRET rc; ULONG cbActual; });

        SYSIO_DECL(fsresetbuffer, { HFILE hFile; }, { APIRET rc; });

        SYSIO_DECL(fssetfileptrl, { HFILE hFile; LONGLONG ib; ULONG method; },
                   { APIRET rc; ULONGLONG ibActual; });

        SYSIO_DECL(fsclose, { HFILE hFile; }, { APIRET rc; });

        SYSIO_DECL(fsqueryhtype, { HFILE hFile; }, { APIRET rc; ULONG type; ULONG attr; });

        SYSIO_DECL(fsopenl, { Path pszFilename; ULONG cbFile; ULONG ulAttribute; ULONG fsOpenFlags;
                   ULONG fsOpenMode; }, { APIRET rc; HFILE hFile; ULONG ulAction; EAOP2 eaop2; });

        SYSIO_DECL(fsduphandle, { HFILE hFile; }, { APIRET rc; HFILE hFile2; });

        SYSIO_DECL(fsdelete, { Path pszFilename; }, { APIRET rc; });

        SYSIO_DECL(fsforcedelete, { Path pszFilename; }, { APIRET rc; });

        SYSIO_DECL(fsdeletedir, { Path pszDirname; }, { APIRET rc; });

        SYSIO_DECL(fscreatedir, { Path pszDirname; EAOP2 eaop2; }, { APIRET rc; });

        SYSIO_DECL(fsfindfirst, { Path pszFilespec; ULONG ulAttribute; ULONG ulInfoLevel; },
                   { APIRET rc; HDIR hDir; ULONG cbBuf; ULONG cFileNames; });

        SYSIO_DECL(fsfindnext, { HDIR hDir; },
                   { APIRET rc; ULONG cbBuf; ULONG cFileNames; });

        SYSIO_DECL(fsfindclose, { HDIR hDir; }, { APIRET rc; });

        SYSIO_DECL(fsqueryfhstate, { HFILE hFile; }, { APIRET rc; ULONG ulMode; });

        SYSIO_DECL(fssetfhstate, { HFILE hFile; ULONG ulMode; }, { APIRET rc; });

        SYSIO_DECL(fsqueryfileinfo, { HFILE hFile; ULONG ulInfoLevel; }, { APIRET rc; ULONG cbInfoBuf; });

        SYSIO_DECL(fsquerypathinfo, { Path pszPathName; ULONG ulInfoLevel; }, { APIRET rc; ULONG cbInfoBuf; });

        SYSIO_DECL(fssetfilesizel, { HFILE hFile; LONGLONG cbSize; }, { APIRET rc; });

        SYSIO_DECL(fssetfileinfo, { HFILE hFile; ULONG ulInfoLevel; }, { APIRET rc; ULONG cbInfoBuf; });

        SYSIO_DECL(fssetpathinfo, { Path pszPathName; ULONG ulInfoLevel; ULONG flOptions; },
                   { APIRET rc; ULONG cbInfoBuf; });
    };
};

#endif /* __CPI_SESSION_SYSIO_H__ */
