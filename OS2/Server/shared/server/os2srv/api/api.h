/* os2srv server-side RPC API */

#ifndef __OS2SRV_API_H__
#define __OS2SRV_API_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/thread.h>
#include <os3/dataspace.h>
#include <os3/semaphore.h>
#include <os3/loader.h>

#define SEMTYPE_EVENT    0
#define SEMTYPE_MUTEX    1
#define SEMTYPR_MUXWAIT  2

/* Handle table element           */
typedef struct _SEM
{
  struct _SEM *pNext;
  char        szName[CCHMAXPATH];
  char        cShared;
  char        cType;
  ULONG       ulRefCnt; 
  union
  {
    l4_os3_semaphore_t evt;
    l4_os3_lock_t mtx;
    struct _SEM *mux;
  }           uSem;
} SEM, *PSEM;

struct DosExecPgm_params
{
  struct t_os2process *proc;
  l4_os3_thread_t thread;
  char *pObjname;
  long cbObjname;
  unsigned long execFlag;
  char *pArg;
  char *pEnv;
  struct _RESULTCODES *pRes;
  char *pName;
};

int cdir(char **dir, char *component);
int strlstcpy(char *s1, char *s2);
int strlstlen(char *p);

void CPAppNotify1(l4_os3_thread_t thread);

void CPAppNotify2(l4_os3_thread_t thread,
                  const os2exec_module_t *s);

APIRET CPCfgGetenv(PCSZ name, char **value);

APIRET CPCfgGetopt(PCSZ name, int *is_int,
                   int *value_int, char **value_str);

APIRET CPExit(l4_os3_thread_t thread,
              ULONG action, ULONG result);

/* DosExecPgm worker thread */
void CPExecPgmWorker(struct DosExecPgm_params *parm);

APIRET CPExecPgm(l4_os3_thread_t thread,
                 char **pObjname,
                 long *cbObjname,
                 unsigned long execFlag,
                 const char* pArg,
                 unsigned long arglen,
                 const char* pEnv,
                 unsigned long envlen,
                 struct _RESULTCODES *pRes,
                 const char* pName);

APIRET CPGetPIB(l4_os3_thread_t thread,
                l4_os3_dataspace_t *ds);

APIRET CPGetTIB(l4_os3_thread_t thread,
                l4_os3_dataspace_t *ds);

APIRET CPError(ULONG error);

APIRET CPQueryDBCSEnv(ULONG *cb,
                      const COUNTRYCODE *pcc,
                      char **pBuf);

APIRET CPQueryCp(ULONG *cb,
                 char **arCP);

APIRET CPQueryCurrentDisk(l4_os3_thread_t thread,
                          ULONG *pdisknum);

APIRET CPQueryCurrentDir(l4_os3_thread_t thread,
                         ULONG disknum,
                         ULONG logical,
                         char **pBuf,
                         ULONG *pcbBuf);

APIRET CPSetCurrentDir(l4_os3_thread_t thread,
                       PCSZ pszDir);

APIRET CPSetDefaultDisk(l4_os3_thread_t thread,
                        ULONG disknum, ULONG logical);

APIRET CPCreateEventSem(l4_os3_thread_t thread,
                        PCSZ pszName, HEV *phev,
                        ULONG flAttr, BOOL32 fState);

APIRET CPOpenEventSem(l4_os3_thread_t thread,
                      PCSZ pszName, HEV *phev);

APIRET CPCloseEventSem(l4_os3_thread_t thread,
                       HEV hev);

APIRET CPGetTID(l4_os3_thread_t thread,
                TID *ptid);

APIRET CPGetPID(l4_os3_thread_t thread,
                PID *ppid);

APIRET CPGetNativeID(PID pid, TID tid,
                     l4_os3_thread_t *thread);

APIRET CPGetTIDNative(const l4_os3_thread_t *thread,
                      TID *ptid);

APIRET CPNewTIB(PID pid, TID tid,
                const l4_os3_thread_t *id);

APIRET CPDestroyTIB(PID pid, TID tid);

#ifdef __cplusplus
  }
#endif

#endif /* __OS2SRV_API_H__ */
