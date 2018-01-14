/* os2srv client-side RPC API */

#ifndef __OS3_CPI_H__
#define __OS3_CPI_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/thread.h>
#include <os3/loader.h>
#include <os3/processmgr.h>

APIRET CPClientInit(void);

APIRET CPClientDone(void);

APIRET CPClientAppNotify1(void);

APIRET CPClientAppNotify2(os2exec_module_t *s);

APIRET CPClientCfgGetenv(PCSZ name, char **value);

APIRET CPClientCfgGetopt(PCSZ name, int *is_int,
                         int *value_int, char **value_str);

APIRET CPClientExit(ULONG action, ULONG result);

APIRET CPClientExecPgmNotify(l4_os3_thread_t job,
                             const char* pObjname,
                             int cbObjname,
                             const struct _RESULTCODES *pRes,
                             int result);

APIRET CPClientExecPgm(char **pObjname,
                       long *cbObjname,
                       unsigned long execFlag,
                       const char* pArg,
                       ULONG arglen,
                       const char* pEnv,
                       ULONG envlen,
                       struct _RESULTCODES *pRes,
                       const char* pName);

APIRET CPClientGetPIB(l4_os3_dataspace_t *ds);

APIRET CPClientGetTIB(l4_os3_dataspace_t *ds);

APIRET CPClientError(ULONG error);

APIRET CPClientQueryDBCSEnv(ULONG *cb,
                            const COUNTRYCODE *pcc,
                            char **pBuf);

APIRET CPClientQueryCp(ULONG *cb,
                       char **arCP);

APIRET CPClientQueryCurrentDisk(ULONG *pdisknum);

APIRET CPClientQueryCurrentDir(ULONG disknum,
                               ULONG logical,
                               char **pBuf,
                               ULONG *pcbBuf);

APIRET CPClientSetCurrentDir(PCSZ pszDir);

APIRET CPClientSetDefaultDisk(ULONG disknum, ULONG logical);

APIRET CPClientCreateEventSem(PCSZ pszName, HEV *phev,
                              ULONG flAttr, BOOL32 fState);

APIRET CPClientOpenEventSem(PCSZ pszName, HEV *phev);

APIRET CPClientCloseEventSem(HEV hev);

APIRET CPClientGetTID(TID *ptid);

APIRET CPClientGetPID(PID *ppid);

APIRET CPClientGetNativeID(PID pid, TID tid,
                           l4_os3_thread_t *thread);

APIRET CPClientGetTIDNative(const l4_os3_thread_t *thread,
                            TID *ptid);

APIRET CPClientNewTIB(PID pid, TID tid,
                      const l4_os3_thread_t *id);

APIRET CPClientDestroyTIB(PID pid, TID tid);

#ifdef __cplusplus
  }
#endif

#endif /* __OS3_CPI_H__ */
