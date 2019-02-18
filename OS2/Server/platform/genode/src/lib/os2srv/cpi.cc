/* os2srv client-side RPC API (Genode platform) */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* libc includes */
#include <string.h>

/* osFree internal */
#include <os3/thread.h>
#include <os3/io.h>

/* Genode includes */
#include <base/allocator.h>
#include <base/capability.h>
#include <dataspace/client.h>
#include <cpi_session/connection.h>
#include <base/attached_dataspace.h>

/* local includes */
#include "genode_env.h"

using namespace OS2::Cpi;

static Connection *cpi;

static Sysio *_sysio = NULL;

extern "C"
APIRET CPClientInit(l4_os3_thread_t *thread)
{
    Genode::Allocator &alloc = genode_alloc();
    Genode::Env &env = genode_env();
    Genode::Dataspace_capability _ds;

    try
    {
        cpi = new (alloc) OS2::Cpi::Connection(env, "main");
    }
    catch (...)
    {
        return ERROR_FILE_NOT_FOUND;
    }

    _ds = cpi->sysio_dataspace();

    if (!_ds.valid())
    {
        return ERROR_INVALID_DATASPACE;
    }

    try
    {
        static Genode::Attached_dataspace _sysio_ds(env.rm(), _ds);
        _sysio = _sysio_ds.local_addr<Sysio>();
    }
    catch (Genode::Region_map::Invalid_dataspace)
    {
        return ERROR_INVALID_DATASPACE;
    }
    catch (Genode::Region_map::Region_conflict)
    {
        return ERROR_NO_MAPPING;
    }

    *thread = cpi;

    return NO_ERROR;
}

extern "C"
APIRET CPClientDone(void)
{
    Genode::Allocator &alloc = genode_alloc();

    destroy(alloc, cpi);
    return NO_ERROR;
}

extern "C"
void CPClientTest(void)
{
    cpi->syscall(Session::SYSCALL_MAIN_TEST);
}

extern "C"
APIRET CPClientCfgGetenv(const char *name,
                       char **value)
{
    APIRET rc;
    strcpy((char *)_sysio->cfggetenv.in.name, name);
    cpi->syscall(Session::SYSCALL_MAIN_CFGGETENV);
    strcpy(*value, (char *)_sysio->cfggetenv.out.value);
    rc = _sysio->cfggetenv.out.rc;
    return rc;
}

extern "C"
APIRET CPClientCfgGetopt(const char *name,
                         int *is_int,
                         int *value_int,
                         char **value_str)
{
    APIRET rc;

    strcpy((char *)_sysio->cfggetopt.in.name, name);

    cpi->syscall(Session::SYSCALL_MAIN_CFGGETOPT);

    rc = _sysio->cfggetopt.out.rc;
    *is_int = _sysio->cfggetopt.out.is_int;
    *value_int = _sysio->cfggetopt.out.value_int;
    strcpy(*value_str, (char *)_sysio->cfggetopt.out.value_str);
    return rc;
}

extern "C"
APIRET CPClientAppNotify1(void)
{
    cpi->syscall(Session::SYSCALL_MAIN_APPNOTIFY1);
    return NO_ERROR;
}

extern "C"
APIRET CPClientAppNotify2(os2exec_module_t *s,
                          const char *pszName,
                          l4_os3_thread_t *thread,
                          const char *szLoadError,
                          ULONG cbLoadError,
                          ULONG ret)
{
    memcpy(&_sysio->appnotify2.in.s, s, sizeof(os2exec_module_t));

    if (pszName)
    {
        strcpy((char *)_sysio->appnotify2.in.pszName, pszName);
    }
    else
    {
        *_sysio->appnotify2.in.pszName = '\0';
    }

    _sysio->appnotify2.in.thread = *thread;
    strcpy((char *)_sysio->appnotify2.in.szLoadError, (char *)szLoadError);
    _sysio->appnotify2.in.cbLoadError = cbLoadError;
    _sysio->appnotify2.in.ret = ret;
    cpi->syscall(Session::SYSCALL_MAIN_APPNOTIFY2);
    return NO_ERROR;
}

extern "C"
APIRET CPClientAppAddData(const app_data_t *data)
{
    APIRET rc;
    memcpy(&_sysio->appsend.in.data, data, sizeof(app_data_t));
    cpi->syscall(Session::SYSCALL_MAIN_APPSEND);
    rc = _sysio->appsend.out.rc;
    return rc;
}

extern "C"
APIRET CPClientAppGetData(app_data_t *data)
{
    APIRET rc;
    cpi->syscall(Session::SYSCALL_MAIN_APPGET);
    memcpy(data, &_sysio->appget.out.data, sizeof(app_data_t));
    rc = _sysio->appget.out.rc;
    return rc;
}

extern "C"
APIRET CPClientExit(ULONG action, ULONG result)
{
    APIRET rc;
    action = _sysio->exit.in.action;
    _sysio->exit.in.action = action;
    _sysio->exit.in.result = result;
    cpi->syscall(Session::SYSCALL_MAIN_EXIT);
    rc = _sysio->exit.out.rc;
    return rc;
}

extern "C"
APIRET CPClientExecPgm(char **pObjname,
                       long *cbObjname,
                       unsigned long execFlag,
                       const char* pArg,
                       ULONG arglen,
                       const char* pEnv,
                       ULONG envlen,
                       struct _RESULTCODES *pRes,
                       const char* pName)
{
    APIRET rc;

    _sysio->execpgm.in.cbObjname = *cbObjname;
    _sysio->execpgm.in.execFlag = execFlag;
    memcpy(_sysio->execpgm.in.pArgs, pArg, arglen);
    _sysio->execpgm.in.arglen = arglen;
    memcpy(_sysio->execpgm.in.pEnv, pEnv, envlen);
    _sysio->execpgm.in.envlen = envlen;
    strcpy(_sysio->execpgm.in.pName, pName);

    cpi->syscall(Session::SYSCALL_MAIN_EXECPGM);

    rc = _sysio->execpgm.out.rc;
    strcpy(*pObjname, _sysio->execpgm.out.pObjname);
    memcpy(pRes, &_sysio->execpgm.out.pRes, sizeof(RESULTCODES));

    return rc;
}

extern "C"
APIRET CPClientExecPgmNotify(l4_os3_thread_t job,
                             const char *pObjname,
                             int cbObjname,
                             const struct _RESULTCODES *pRes,
                             int result)
{
    job = job;
    pObjname = pObjname;
    cbObjname = cbObjname;
    pRes = pRes;
    result = result;
    return NO_ERROR;
}

extern "C"
APIRET CPClientGetPIB(PID pid, l4_os3_dataspace_t *ds)
{
    APIRET rc;
    Genode::Untyped_capability cap;
    Genode::Dataspace_capability _ds;
    _sysio->getpib.in.pid = pid;
    cpi->syscall(Session::SYSCALL_MAIN_GETPIB);
    rc = _sysio->getpib.out.rc;
    cap = cpi->get_cap(0);
    _ds = Genode::reinterpret_cap_cast<Genode::Dataspace>(cap);
    *ds = (l4_os3_dataspace_t)&_ds;
    return rc;
}

extern "C"
APIRET CPClientGetTIB(PID pid, TID tid, l4_os3_dataspace_t *ds)
{
    APIRET rc;
    Genode::Untyped_capability cap;
    Genode::Dataspace_capability _ds;
    _sysio->gettib.in.pid = pid;
    _sysio->gettib.in.tid = tid;
    cpi->syscall(Session::SYSCALL_MAIN_GETTIB);
    rc = _sysio->gettib.out.rc;
    cap = cpi->get_cap(0);
    _ds = Genode::reinterpret_cap_cast<Genode::Dataspace>(cap);
    *ds = (l4_os3_dataspace_t)&_ds;
    return rc;
}

extern "C"
APIRET CPClientError(ULONG error)
{
    APIRET rc;
    _sysio->error.in.error = error;
    cpi->syscall(Session::SYSCALL_MAIN_ERROR);
    rc = _sysio->error.out.rc;
    return rc;
}

extern "C"
APIRET CPClientQueryDBCSEnv(ULONG *cb,
                            const COUNTRYCODE *pcc,
                            char **pBuf)
{
    APIRET rc;
    _sysio->querydbcsenv.in.cb = *cb;
    memcpy(&_sysio->querydbcsenv.in.cc, pcc, sizeof(COUNTRYCODE));
    cpi->syscall(Session::SYSCALL_MAIN_QUERYDBCSENV);
    *cb = _sysio->querydbcsenv.out.cb;
    memcpy(*pBuf, &_sysio->querydbcsenv.out.pBuf, *cb);
    rc = _sysio->querydbcsenv.out.rc;
    return rc;
}

extern "C"
APIRET CPClientQueryCp(ULONG *cb,
                       char **arCP)
{
    APIRET rc;
    _sysio->querycp.in.cb = *cb;
    cpi->syscall(Session::SYSCALL_MAIN_QUERYCP);
    *cb = _sysio->querycp.out.cb;
    memcpy(*arCP, &_sysio->querycp.out.arCP, *cb);
    rc = _sysio->querycp.out.rc;
    return rc;
}

extern "C"
APIRET CPClientQueryCurrentDisk(ULONG *pdisknum)
{
    APIRET rc;
    cpi->syscall(Session::SYSCALL_MAIN_QUERYCURRENTDISK);
    *pdisknum = _sysio->querycurrentdisk.out.disknum;
    rc = _sysio->querycurrentdisk.out.rc;
    return rc;
}

extern "C"
APIRET CPClientQueryCurrentDir(ULONG disknum, ULONG logical,
                               char **pBuf, ULONG *pcbBuf)
{
    APIRET rc;
    _sysio->querycurrentdir.in.disknum = disknum;
    _sysio->querycurrentdir.in.logical = logical;
    cpi->syscall(Session::SYSCALL_MAIN_QUERYCURRENTDIR);
    *pcbBuf = _sysio->querycurrentdir.out.cbBuf;
    memcpy(*pBuf, _sysio->querycurrentdir.out.pBuf, *pcbBuf);
    rc = _sysio->querycurrentdir.out.rc;
    return rc;
}

extern "C"
APIRET CPClientSetCurrentDir(const char *pszDir)
{
    APIRET rc;
    strcpy((char *)_sysio->setcurrentdir.in.pszDir, pszDir);
    cpi->syscall(Session::SYSCALL_MAIN_SETCURRENTDIR);
    rc = _sysio->setcurrentdir.out.rc;
    return rc;
}

extern "C"
APIRET CPClientSetDefaultDisk(ULONG disknum, ULONG logical)
{
    APIRET rc;
    _sysio->setdefaultdisk.in.disknum = disknum;
    _sysio->setdefaultdisk.in.logical = logical;
    cpi->syscall(Session::SYSCALL_MAIN_SETDEFAULTDISK);
    rc = _sysio->setdefaultdisk.out.rc;
    return rc;
}

extern "C"
APIRET CPClientCreateEventSem(PCSZ pszName, HEV *phev,
                              ULONG flAttr, BOOL32 fState)
{
    APIRET rc;
    strcpy(_sysio->createeventsem.in.pszName, pszName);
    _sysio->createeventsem.in.flAttr = flAttr;
    _sysio->createeventsem.in.fState = fState;
    cpi->syscall(Session::SYSCALL_MAIN_CREATEEVENTSEM);
    *phev = _sysio->createeventsem.out.hev;
    rc = _sysio->createeventsem.out.rc;
    return rc;
}

extern "C"
APIRET CPClientOpenEventSem(PCSZ pszName, HEV *phev)
{
    APIRET rc;
    strcpy(_sysio->openeventsem.in.pszName, pszName);
    cpi->syscall(Session::SYSCALL_MAIN_OPENEVENTSEM);
    *phev = _sysio->openeventsem.out.hev;
    rc = _sysio->openeventsem.out.rc;
    return rc;
}

extern "C"
APIRET CPClientCloseEventSem(HEV hev)
{
    APIRET rc;
    _sysio->closeeventsem.in.hev = hev;
    cpi->syscall(Session::SYSCALL_MAIN_CLOSEEVENTSEM);
    rc = _sysio->closeeventsem.out.rc;
    return rc;
}

extern "C"
APIRET CPClientGetPID(PID *pid)
{
    APIRET rc;
    cpi->syscall(Session::SYSCALL_MAIN_GETPID);
    *pid = _sysio->getpid.out.pid;
    rc = _sysio->getpid.out.rc;
    return rc;
}

extern "C"
APIRET CPClientGetNativeID(PID pid, TID tid,
                           l4_os3_thread_t *id)
{
    APIRET rc;
    _sysio->getnativeid.in.pid = pid;
    _sysio->getnativeid.in.tid = tid;
    cpi->syscall(Session::SYSCALL_MAIN_GETNATIVEID);
    *id = _sysio->getnativeid.out.id;
    rc = _sysio->getnativeid.out.rc;
    return rc;
}

extern "C"
APIRET CPClientNewTIB(PID pid, TID tid,
                      const l4_os3_thread_t *id)
{
    APIRET rc;
    _sysio->newtib.in.pid = pid;
    _sysio->newtib.in.tid = tid;
    _sysio->newtib.in.id = *id;
    cpi->syscall(Session::SYSCALL_MAIN_NEWTIB);
    rc = _sysio->newtib.out.rc;
    return rc;
}

extern "C"
APIRET CPClientDestroyTIB(PID pid, TID tid)
{
    APIRET rc;
    _sysio->destroytib.in.pid = pid;
    _sysio->destroytib.in.tid = tid;
    cpi->syscall(Session::SYSCALL_MAIN_DESTROYTIB);
    rc = _sysio->destroytib.out.rc;
    return rc;
}
