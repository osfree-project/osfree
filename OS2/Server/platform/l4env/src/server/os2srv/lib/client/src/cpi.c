/* os2srv client-side RPC API */

/* osFree internal */
#include <os3/io.h>
#include <os3/types.h>
#include <os3/loader.h>
#include <os3/dataspace.h>
#include <os3/cpi.h>

/* l4env includes */
#include <l4/names/libnames.h>

/* os2srv RPC includes */
#include <os2server-client.h>

extern l4_threadid_t os2srv;

APIRET CPClientInit(void)
{
    if (! names_waitfor_name("os2srv", &os2srv, 30000))
    {
        io_log("os2srv not found at the name server!\n");
        return ERROR_FILE_NOT_FOUND;
    }

    return NO_ERROR;
}

APIRET CPClientDone(void)
{
    return NO_ERROR;
}

APIRET CPClientAppNotify1(void)
{
    CORBA_Environment env = dice_default_environment;

    os2server_app_notify1_call(&os2srv, &env);
    return NO_ERROR;
}

APIRET CPClientAppNotify2(os2exec_module_t *s)
{
    CORBA_Environment env = dice_default_environment;

    os2server_app_notify2_call(&os2srv, s, &env);
    return NO_ERROR;
}

APIRET CPClientCfgGetenv(PCSZ name, char **value)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_cfg_getenv_call(&os2srv, name, value, &env);
}

APIRET CPClientCfgGetopt(PCSZ name, int *is_int,
                         int *value_int, char **value_str)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_cfg_getopt_call(&os2srv, name, is_int,
                                         value_int, value_str, &env);
}

APIRET CPClientExit(ULONG action, ULONG result)
{
    CORBA_Environment env = dice_default_environment;

    os2server_dos_Exit_send(&os2srv, action, result, &env);
    return NO_ERROR;
}

APIRET CPClientExecPgmNotify(l4_os3_thread_t job,
                             const char* pObjname,
                             int cbObjname,
                             const struct _RESULTCODES *pRes,
                             int result)
{
    CORBA_Environment env = dice_default_environment;

    os2server_dos_ExecPgm_notify_call(&os2srv, &job.thread,
                                      pObjname, cbObjname,
                                      pRes, result, &env);
    return NO_ERROR;
}

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
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_ExecPgm_call(&os2srv, pObjname, cbObjname,
        execFlag, pArg, arglen, pEnv, envlen,
        pRes, pName, &env);
}

APIRET CPClientGetPIB(l4_os3_dataspace_t *ds)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_GetPIB_call(&os2srv, &ds->ds, &env);
}

APIRET CPClientGetTIB(l4_os3_dataspace_t *ds)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_GetTIB_call(&os2srv, &ds->ds, &env);
}

APIRET CPClientError(ULONG error)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_Error_call(&os2srv, error, &env);
}

APIRET CPClientQueryDBCSEnv(ULONG *cb,
                            const COUNTRYCODE *pcc,
                            char **pBuf)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_QueryDBCSEnv_call(&os2srv, cb, pcc, pBuf, &env);
}

APIRET CPClientQueryCp(ULONG *cb,
                       char **arCP)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_QueryCp_call(&os2srv, cb, arCP, &env);
}

APIRET CPClientQueryCurrentDisk(ULONG *pdisknum)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_QueryCurrentDisk_call(&os2srv, pdisknum, &env);
}

APIRET CPClientQueryCurrentDir(ULONG disknum,
                               ULONG logical,
                               char **pBuf,
                               ULONG *pcbBuf)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_QueryCurrentDir_call(&os2srv, disknum, logical,
                                               pBuf, pcbBuf, &env);
}

APIRET CPClientSetCurrentDir(PCSZ pszDir)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_SetCurrentDir_call(&os2srv, pszDir, &env);
}

APIRET CPClientSetDefaultDisk(ULONG disknum, ULONG logical)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_SetDefaultDisk_call(&os2srv, disknum, logical, &env);
}

APIRET CPClientCreateEventSem(PCSZ pszName, HEV *phev,
                              ULONG flAttr, BOOL32 fState)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_CreateEventSem_call(&os2srv, pszName, phev,
                                             flAttr, fState, &env);
}

APIRET CPClientOpenEventSem(PCSZ pszName, HEV *phev)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_OpenEventSem_call(&os2srv, pszName, phev, &env);
}

APIRET CPClientCloseEventSem(HEV hev)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_CloseEventSem_call(&os2srv, hev, &env);
}

APIRET CPClientGetTID(TID *ptid)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_GetTID_call(&os2srv, ptid, &env);
}

APIRET CPClientGetPID(PID *ppid)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_GetPID_call(&os2srv, ppid, &env);
}

APIRET CPClientGetNativeID(PID pid, TID tid,
                           l4_os3_thread_t *thread)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_GetNativeID_call(&os2srv, pid, tid, thread, &env);
}

APIRET CPClientGetTIDNative(const l4_os3_thread_t *thread,
                            TID *ptid)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_GetTIDNative_call(&os2srv, thread, ptid, &env);
}

APIRET CPClientNewTIB(PID pid, TID tid,
                      const l4_os3_thread_t *thread)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_NewTIB_call(&os2srv, pid, tid, thread, &env);
}

APIRET CPClientDestroyTIB(PID pid, TID tid)
{
    CORBA_Environment env = dice_default_environment;

    return os2server_dos_DestroyTIB_call(&os2srv, pid, tid, &env);
}
