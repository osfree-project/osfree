/* os2srv client-side RPC API (Genode platform) */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

#include <string.h>

#include <os3/io.h>

/* Genode includes */
#include <base/allocator.h>
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
    //Genode::Allocator &alloc = genode_alloc();
    Genode::Env &env = genode_env();
    Genode::Dataspace_capability _ds;

    try
    {
        static OS2::Cpi::Connection conn(env);
        cpi = &conn;
        //cpi = new (alloc) OS2::Cpi::Connection(env);
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
    //Genode::Allocator &alloc = genode_alloc();

    //destroy(alloc, cpi);
    return NO_ERROR;
}

extern "C"
void CPClientTest(void)
{
    cpi->syscall(Session::SYSCALL_TEST);
}

extern "C"
void CPClientCfgGetenv(const char *name,
                       char **value)
{
    strcpy((char *)_sysio->cfggetenv_in.name, name);
    cpi->syscall(Session::SYSCALL_CFGGETENV);
    strcpy(*value, (char *)_sysio->cfggetenv_out.value);
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
void CPClientAppNotify1(void)
{
    cpi->syscall(Session::SYSCALL_APPNOTIFY1);
}

extern "C"
void CPClientAppNotify2(const os2exec_module_t *s,
                        const char *pszName,
                        PID pid,
                        l4_os3_thread_t *thread,
                        const void *szLoadError,
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

    _sysio->appnotify2_in.pid = pid;
    _sysio->appnotify2_in.thread = *thread;
    strcpy((char *)_sysio->appnotify2_in.szLoadError, (char *)szLoadError);
    _sysio->appnotify2_in.cbLoadError = cbLoadError;
    _sysio->appnotify2_in.ret = ret;
    cpi->syscall(Session::SYSCALL_APPNOTIFY2);
}

extern "C"
void CPClientExit(ULONG action, ULONG result)
{
    action = _sysio->exit_in.action;
    _sysio->exit_in.action = action;
    _sysio->exit_in.result = result;
    cpi->syscall(Session::SYSCALL_EXIT);
    action = action;
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
