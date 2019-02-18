/* API implementations
 * (on the server side)
 */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal includes */
#include <os3/types.h>
#include <os3/dataspace.h>
#include <os3/io.h>
#include <os3/cpi.h>

/* IPC includes (dice) */
#include <dice/dice.h>

/* os2srv RPC includes */
#include <os2server-client.h>
#include <os2server-server.h>

/* libc includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

/* local includes */
#include "api.h"

extern struct t_os2process *proc_root;

APIRET DICE_CV
os2server_cfg_getenv_component (CORBA_Object _obj,
                                const char* name /* in */,
                                char* *value /* out */,
                                CORBA_Server_Environment *_env)
{
  return CPCfgGetenv(name, value);
}


APIRET DICE_CV
os2server_cfg_getopt_component (CORBA_Object _obj,
                                const char* name,
                                int *is_int,
                                int *value_int,
                                char* *value_str,
                                CORBA_Server_Environment *_env)
{
  return CPCfgGetopt(name, is_int, value_int, value_str);
}

void DICE_CV
os2server_test_component(CORBA_Object _dice_corba_obj,
                         CORBA_Server_Environment *_dice_corba_env)
{
  CPTest();
}

/* is called by os2app after its successful startup */
void DICE_CV
os2server_app_notify1_component(CORBA_Object _dice_corba_obj,
                                CORBA_Server_Environment *_dice_corba_env)
{
  l4_os3_thread_t thread;

  thread.thread = *_dice_corba_obj;
  CPAppNotify1(thread);
}

/* is called by os2app, and notifies os2srv
   about some module parameters got from execsrv */
void DICE_CV
os2server_app_notify2_component(CORBA_Object _dice_corba_obj,
                                l4_uint32_t lthread,
                                const os2exec_module_t *s,
                                const char *pszName,
                                const void *szLoadError,
                                ULONG cbLoadError,
                                ULONG ret,
                                CORBA_Server_Environment *_dice_corba_env)
{
  l4_os3_thread_t task;

  task.thread = *_dice_corba_obj;
  task.thread.id.lthread = lthread;

  CPAppNotify2(task, s, pszName,
               szLoadError, cbLoadError, ret);
}

APIRET DICE_CV
os2server_app_send_component(CORBA_Object _dice_corba_obj,
                             const app_data_t *data,
                             CORBA_Server_Environment *_dice_corba_env)
{
  return CPAppAddData(data);
}

APIRET DICE_CV
os2server_app_get_component(CORBA_Object obj,
                            app_data_t *data,
                            CORBA_Server_Environment *_dice_corba_env)
{
  struct t_os2process *proc;
  l4_os3_thread_t thread;
  PID pid;

  thread.thread = *obj;
  proc = PrcGetProcNative(thread);

  if (! proc)
  {
    return ERROR_PROC_NOT_FOUND;
  }

  pid = proc->pid;
  return CPAppGetData(pid, data);
}

void DICE_CV
os2server_dos_Exit_component(CORBA_Object obj,
                             ULONG action, ULONG result,
                             CORBA_Server_Environment *_srv_env)
{
  l4_os3_thread_t thread;

  thread.thread = *obj;
  CPExit(thread, action, result);
}

/* notifier for main DosExecPgm component */
void DICE_CV
os2server_dos_ExecPgm_notify_component (CORBA_Object obj,
                            const l4_threadid_t *job /* in */,
                            const char* pObjname /* in */,
                            int cbObjname /* in */,
                            const struct _RESULTCODES *pRes /* in */,
                            int result /* in */,
                            CORBA_Server_Environment *_srv_env)
{
  io_log("pRes=%lx\n", (ULONG)pRes);
  io_log("pObjname=%lx\n", (ULONG)pObjname);
  io_log("cbObjname=%lx\n", (ULONG)cbObjname);
  io_log("pRes->codeTerminate=%lx\n", (ULONG)pRes->codeTerminate);
  io_log("pRes->codeResult=%lx\n", (ULONG)pRes->codeResult);

  os2server_dos_ExecPgm_reply ((l4_threadid_t *)job, result, (char **)&pObjname,
                               (long *)&cbObjname, (struct _RESULTCODES *)pRes, _srv_env);
}

APIRET DICE_CV
os2server_dos_ExecPgm_component (CORBA_Object obj,
                                 char **pObjname /* in, out */,
                                 long *cbObjname /* in, out */,
                                 unsigned long execFlag /* in */,
                                 const char* pArg /* in */,
                                 l4_uint32_t arglen,
                                 const char* pEnv /* in */,
                                 l4_uint32_t envlen,
                                 struct _RESULTCODES *pRes /* in, out */,
                                 const char* pName /* in */,
                                 short *dice_reply,
                                 CORBA_Server_Environment *_srv_env)
{
  l4_os3_thread_t thread;
  APIRET rc;

  thread.thread = *obj;
  rc = CPExecPgm(thread, pObjname, cbObjname, execFlag,
                 pArg, arglen, pEnv, envlen, pRes,
                 pName);

  *dice_reply = DICE_NO_REPLY; // do not reply to the client until notification
  return rc;
}

APIRET DICE_CV
os2server_dos_GetPIB_component (CORBA_Object obj,
                                PID pid, /* in */
                                l4dm_dataspace_t *ds /* out */,
                                CORBA_Server_Environment *_srv_env)
{
  l4_os3_thread_t thread;
  l4_os3_dataspace_t tmp_ds;
  APIRET rc;

  thread.thread = *obj;

  rc = CPGetPIB(pid, thread, &tmp_ds);

  *ds = tmp_ds.ds;
  return rc;
}

APIRET DICE_CV
os2server_dos_GetTIB_component (CORBA_Object obj,
                                PID pid, /* in */
                                TID tid, /* in */
                                l4dm_dataspace_t *ds /* out */,
                                CORBA_Server_Environment *_srv_env)
{
  l4_os3_thread_t thread;
  l4_os3_dataspace_t tmp_ds;
  APIRET rc;

  thread.thread = *obj;

  rc = CPGetTIB(pid, tid, thread, &tmp_ds);

  *ds = tmp_ds.ds;
  return rc;
}

APIRET DICE_CV
os2server_dos_Error_component (CORBA_Object obj,
                               ULONG error /* in */,
                               CORBA_Server_Environment *_srv_env)
{
  return CPError(error);
}

APIRET DICE_CV
os2server_dos_QueryDBCSEnv_component (CORBA_Object obj,
                                      ULONG *cb /* in, out */,
                                      const COUNTRYCODE *pcc /* out */,
                                      char **pBuf /* in */,
                                      CORBA_Server_Environment *_srv_env)
{
  return CPQueryDBCSEnv(cb, pcc, pBuf);
}


APIRET DICE_CV
os2server_dos_QueryCp_component (CORBA_Object obj,
                                 ULONG *cb /* in, out */,
                                 char  **arCP /* out */,
                                 CORBA_Server_Environment *_srv_env)
{
  return CPQueryCp(cb, arCP);
}

APIRET DICE_CV
os2server_dos_QueryCurrentDisk_component (CORBA_Object obj,
                                          ULONG *pdisknum /* out */,
                                          CORBA_Server_Environment *_srv_env)
{
  l4_os3_thread_t thread;

  thread.thread = *obj;
  return CPQueryCurrentDisk(thread, pdisknum);
}

APIRET DICE_CV
os2server_dos_QueryCurrentDir_component (CORBA_Object obj,
                                         ULONG disknum /* in */,
                                         ULONG logical /* in */,
                                         char **pBuf /* out */,
                                         ULONG *pcbBuf /* out */,
                                         CORBA_Server_Environment *_srv_env)
{
  l4_os3_thread_t thread;

  thread.thread = *obj;
  return CPQueryCurrentDir(thread, disknum, logical,
                           pBuf, pcbBuf);
}

APIRET DICE_CV
os2server_dos_SetCurrentDir_component (CORBA_Object obj,
                                       PCSZ pszDir /* in */,
                                       CORBA_Server_Environment *_srv_env)
{
  l4_os3_thread_t thread;

  thread.thread = *obj;
  return CPSetCurrentDir(thread, pszDir);
}

APIRET DICE_CV
os2server_dos_SetDefaultDisk_component (CORBA_Object obj,
                                        ULONG disknum /* in */,
                                        ULONG logical /* in */,
                                        CORBA_Server_Environment *_srv_env)
{
  l4_os3_thread_t thread;

  thread.thread = *obj;
  return CPSetDefaultDisk(thread, disknum, logical);
}

APIRET DICE_CV
os2server_dos_CreateEventSem_component (CORBA_Object obj,
                                        const char* pszName /* in */,
                                        HEV *phev /* out */,
                                        ULONG flAttr /* in */,
                                        BOOL32 fState /* in */,
                                        CORBA_Server_Environment *_srv_env)
{
  l4_os3_thread_t thread;

  thread.thread = *obj;
  return CPCreateEventSem(thread, pszName, phev,
                          flAttr, fState);
}

APIRET DICE_CV
os2server_dos_OpenEventSem_component (CORBA_Object obj,
                                      const char* pszName /* in */,
                                      HEV *phev /* in, out */,
                                      CORBA_Server_Environment *_srv_env)
{
  l4_os3_thread_t thread;

  thread.thread = *obj;
  return CPOpenEventSem(thread, pszName, phev);
}

APIRET DICE_CV
os2server_dos_CloseEventSem_component (CORBA_Object obj,
                                       HEV hev /* in */,
                                       CORBA_Server_Environment *_srv_env)
{
  l4_os3_thread_t thread;

  thread.thread = *obj;
  return CPCloseEventSem(thread, hev);
}

APIRET DICE_CV
os2server_dos_GetPID_component (CORBA_Object obj,
                                PID *ppid /* out */,
                                CORBA_Server_Environment *_srv_env)
{
  l4_os3_thread_t thread;
  thread.thread = *obj;

  return CPGetPID(thread, ppid);
}

APIRET DICE_CV
os2server_dos_GetNativeID_component (CORBA_Object obj,
                                     PID pid /* in */,
                                     TID tid /* in */,
                                     l4_os3_thread_t *id /* out */,
                                     CORBA_Server_Environment *_srv_env)
{
  return CPGetNativeID(pid, tid, id);
}

APIRET DICE_CV
os2server_dos_NewTIB_component (CORBA_Object obj,
                                PID pid /* in */,
                                TID tid /* in */,
                                const l4_os3_thread_t *id /* in */,
                                CORBA_Server_Environment *_srv_env)
{
  return CPNewTIB(pid, tid, id);
}

APIRET DICE_CV
os2server_dos_DestroyTIB_component (CORBA_Object obj,
                                    PID pid /* in */,
                                    TID tid /* in */,
                                    CORBA_Server_Environment *_srv_env)
{
  return CPDestroyTIB(pid, tid);
}
