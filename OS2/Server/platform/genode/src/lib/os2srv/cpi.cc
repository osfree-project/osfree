/* os2srv client-side RPC API (Genode platform) */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* libc includes */
#include <string.h>

/* osFree internal */
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

Connection *cpi;

Sysio *_sysio = NULL;

extern "C"
APIRET CPClientInit(void)
{
    Genode::Allocator &alloc = genode_alloc();
    Genode::Env &env = genode_env();
    Genode::Dataspace_capability _ds;

    try
    {
        cpi = new (alloc) OS2::Cpi::Connection(env);
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
    cpi->syscall(Session::SYSCALL_TEST);
}

extern "C"
APIRET CPClientCfgGetenv(const char *name,
                       char **value)
{
    APIRET rc;
    strcpy((char *)_sysio->cfggetenv_in.name, name);
    cpi->syscall(Session::SYSCALL_CFGGETENV);
    strcpy(*value, (char *)_sysio->cfggetenv_out.value);
    rc = _sysio->cfggetenv_out.rc;
    return rc;
}

extern "C"
APIRET CPClientCfgGetopt(const char *name,
                         int *is_int,
                         int *value_int,
                         char **value_str)
{
    APIRET rc;

    strcpy((char *)_sysio->cfggetopt_in.name, name);

    cpi->syscall(Session::SYSCALL_CFGGETOPT);

    rc = _sysio->cfggetopt_out.rc;
    *is_int = _sysio->cfggetopt_out.is_int;
    *value_int = _sysio->cfggetopt_out.value_int;
    strcpy(*value_str, (char *)_sysio->cfggetopt_out.value_str);
    return rc;
}

extern "C"
APIRET CPClientAppNotify1(void)
{
    cpi->syscall(Session::SYSCALL_APPNOTIFY1);
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
    memcpy(&_sysio->appnotify2_in.s, s, sizeof(os2exec_module_t));

    if (pszName)
    {
        strcpy((char *)_sysio->appnotify2_in.pszName, pszName);
    }
    else
    {
        *_sysio->appnotify2_in.pszName = '\0';
    }

    _sysio->appnotify2_in.thread = *thread;
    strcpy((char *)_sysio->appnotify2_in.szLoadError, (char *)szLoadError);
    _sysio->appnotify2_in.cbLoadError = cbLoadError;
    _sysio->appnotify2_in.ret = ret;
    cpi->syscall(Session::SYSCALL_APPNOTIFY2);
    return NO_ERROR;
}

extern "C"
APIRET CPClientAppAddData(const app_data_t *data)
{
    APIRET rc;
    memcpy(&_sysio->appsend_in.data, data, sizeof(app_data_t));
    cpi->syscall(Session::SYSCALL_APPSEND);
    rc = _sysio->appsend_out.rc;
    return rc;
}

extern "C"
APIRET CPClientAppGetData(app_data_t *data)
{
    APIRET rc;
    cpi->syscall(Session::SYSCALL_APPGET);
    memcpy(data, &_sysio->appget_out.data, sizeof(app_data_t));
    rc = _sysio->appget_out.rc;
    return rc;
}

extern "C"
APIRET CPClientExit(ULONG action, ULONG result)
{
    APIRET rc;
    action = _sysio->exit_in.action;
    _sysio->exit_in.action = action;
    _sysio->exit_in.result = result;
    cpi->syscall(Session::SYSCALL_EXIT);
    rc = _sysio->exit_out.rc;
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

    _sysio->execpgm_in.cbObjname = *cbObjname;
    _sysio->execpgm_in.execFlag = execFlag;
    memcpy(_sysio->execpgm_in.pArgs, pArg, arglen);
    _sysio->execpgm_in.arglen = arglen;
    memcpy(_sysio->execpgm_in.pEnv, pEnv, envlen);
    _sysio->execpgm_in.envlen = envlen;
    strcpy(_sysio->execpgm_in.pName, pName);

    cpi->syscall(Session::SYSCALL_EXECPGM);

    rc = _sysio->execpgm_out.rc;
    strcpy(*pObjname, _sysio->execpgm_out.pObjname);
    memcpy(pRes, &_sysio->execpgm_out.pRes, sizeof(RESULTCODES));

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
    _sysio->getpib_in.pid = pid;
    cpi->syscall(Session::SYSCALL_GETPIB);
    rc = _sysio->getpib_out.rc;
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
    _sysio->gettib_in.pid = pid;
    _sysio->gettib_in.tid = tid;
    cpi->syscall(Session::SYSCALL_GETTIB);
    rc = _sysio->gettib_out.rc;
    cap = cpi->get_cap(0);
    _ds = Genode::reinterpret_cap_cast<Genode::Dataspace>(cap);
    *ds = (l4_os3_dataspace_t)&_ds;
    return rc;
}

extern "C"
APIRET CPClientError(ULONG error)
{
    APIRET rc;
    _sysio->error_in.error = error;
    cpi->syscall(Session::SYSCALL_ERROR);
    rc = _sysio->error_out.rc;
    return rc;
}

extern "C"
APIRET CPClientQueryDBCSEnv(ULONG *cb,
                            const COUNTRYCODE *pcc,
                            char **pBuf)
{
    APIRET rc;
    _sysio->querydbcsenv_in.cb = *cb;
    memcpy(&_sysio->querydbcsenv_in.cc, pcc, sizeof(COUNTRYCODE));
    cpi->syscall(Session::SYSCALL_QUERYDBCSENV);
    *cb = _sysio->querydbcsenv_out.cb;
    memcpy(*pBuf, &_sysio->querydbcsenv_out.pBuf, *cb);
    rc = _sysio->querydbcsenv_out.rc;
    return rc;
}

extern "C"
APIRET CPClientQueryCp(ULONG *cb,
                       char **arCP)
{
    APIRET rc;
    _sysio->querycp_in.cb = *cb;
    cpi->syscall(Session::SYSCALL_QUERYCP);
    *cb = _sysio->querycp_out.cb;
    memcpy(*arCP, &_sysio->querycp_out.arCP, *cb);
    rc = _sysio->querycp_out.rc;
    return rc;
}

extern "C"
APIRET CPClientQueryCurrentDisk(ULONG *pdisknum)
{
    APIRET rc;
    cpi->syscall(Session::SYSCALL_QUERYCURRENTDISK);
    *pdisknum = _sysio->querycurrentdisk_out.disknum;
    rc = _sysio->querycurrentdisk_out.rc;
    return rc;
}

extern "C"
APIRET CPClientQueryCurrentDir(ULONG disknum, ULONG logical,
                               char **pBuf, ULONG *pcbBuf)
{
    APIRET rc;
    _sysio->querycurrentdir_in.disknum = disknum;
    _sysio->querycurrentdir_in.logical = logical;
    cpi->syscall(Session::SYSCALL_QUERYCURRENTDIR);
    *pcbBuf = _sysio->querycurrentdir_out.cbBuf;
    memcpy(*pBuf, _sysio->querycurrentdir_out.pBuf, *pcbBuf);
    rc = _sysio->querycurrentdir_out.rc;
    return rc;
}

extern "C"
APIRET CPClientSetCurrentDir(const char *pszDir)
{
    APIRET rc;
    strcpy((char *)_sysio->setcurrentdir_in.pszDir, pszDir);
    cpi->syscall(Session::SYSCALL_SETCURRENTDIR);
    rc = _sysio->setcurrentdir_out.rc;
    return rc;
}

extern "C"
APIRET CPClientSetDefaultDisk(ULONG disknum, ULONG logical)
{
    APIRET rc;
    _sysio->setdefaultdisk_in.disknum = disknum;
    _sysio->setdefaultdisk_in.logical = logical;
    cpi->syscall(Session::SYSCALL_SETDEFAULTDISK);
    rc = _sysio->setdefaultdisk_out.rc;
    return rc;
}

extern "C"
APIRET CPClientCreateEventSem(PCSZ pszName, HEV *phev,
                              ULONG flAttr, BOOL32 fState)
{
    APIRET rc;
    strcpy(_sysio->createeventsem_in.pszName, pszName);
    _sysio->createeventsem_in.flAttr = flAttr;
    _sysio->createeventsem_in.fState = fState;
    cpi->syscall(Session::SYSCALL_CREATEEVENTSEM);
    *phev = _sysio->createeventsem_out.hev;
    rc = _sysio->createeventsem_out.rc;
    return rc;
}

extern "C"
APIRET CPClientOpenEventSem(PCSZ pszName, HEV *phev)
{
    APIRET rc;
    strcpy(_sysio->openeventsem_in.pszName, pszName);
    cpi->syscall(Session::SYSCALL_OPENEVENTSEM);
    *phev = _sysio->openeventsem_out.hev;
    rc = _sysio->openeventsem_out.rc;
    return rc;
}

extern "C"
APIRET CPClientCloseEventSem(HEV hev)
{
    APIRET rc;
    _sysio->closeeventsem_in.hev = hev;
    cpi->syscall(Session::SYSCALL_CLOSEEVENTSEM);
    rc = _sysio->closeeventsem_out.rc;
    return rc;
}

extern "C"
APIRET CPClientGetPID(PID *pid)
{
    APIRET rc;
    cpi->syscall(Session::SYSCALL_GETPID);
    *pid = _sysio->getpid_out.pid;
    rc = _sysio->getpid_out.rc;
    return rc;
}

extern "C"
APIRET CPClientGetNativeID(PID pid, TID tid,
                           l4_os3_thread_t *id)
{
    APIRET rc;
    _sysio->getnativeid_in.pid = pid;
    _sysio->getnativeid_in.tid = tid;
    cpi->syscall(Session::SYSCALL_GETNATIVEID);
    *id = _sysio->getnativeid_out.id;
    rc = _sysio->getnativeid_out.rc;
    return rc;
}

extern "C"
APIRET CPClientNewTIB(PID pid, TID tid,
                      const l4_os3_thread_t *id)
{
    APIRET rc;
    _sysio->newtib_in.pid = pid;
    _sysio->newtib_in.tid = tid;
    _sysio->newtib_in.id = *id;
    cpi->syscall(Session::SYSCALL_NEWTIB);
    rc = _sysio->newtib_out.rc;
    return rc;
}

extern "C"
APIRET CPClientDestroyTIB(PID pid, TID tid)
{
    APIRET rc;
    _sysio->destroytib_in.pid = pid;
    _sysio->destroytib_in.tid = tid;
    cpi->syscall(Session::SYSCALL_DESTROYTIB);
    rc = _sysio->destroytib_out.rc;
    return rc;
}

