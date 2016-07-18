/* API implementations
 * (on the server side)
 */

// libc includes
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

// l4 includes
#include <l4/semaphore/semaphore.h>
#include <l4/lock/lock.h>
#include <l4/thread/thread.h>

// osFree internal includes
#include <l4/os3/types.h>
#include <l4/os3/dataspace.h>
#include <l4/os3/rm.h>
#include <l4/os3/gcc_os2def.h>
#include <l4/os3/processmgr.h>
#include <l4/os3/handlemgr.h>
#include <l4/os3/globals.h>
#include <l4/os3/io.h>

// IPC includes
#include <l4/os3/ipc.h>

// os2fs RPC includes
#include <l4/os2fs/os2fs-client.h>

// os2srv RPC includes
#include <os2server-client.h>
#include <os2server-server.h>

extern l4os3_cap_idx_t fs;
extern l4os3_cap_idx_t os2srv;
extern l4os3_cap_idx_t sysinit_id;
extern struct t_os2process *proc_root;

#define SEMTYPE_EVENT    0
#define SEMTYPE_MUTEX    1
#define SEMTYPR_MUXWAIT  2

/* Semaphore handle table pointer */
HANDLE_TABLE *htSem;
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
    l4semaphore_t evt;
    l4lock_t mtx;
    struct _SEM *mux;
  }           uSem;
} SEM, *PSEM;

int cdir(char **dir, char *component);
int strlstcpy(char *s1, char *s2);
int strlstlen(char *p);

struct DosExecPgm_params {
  struct t_os2process *proc;
  l4os3_cap_idx_t thread;
  char *pObjname;
  long cbObjname;
  unsigned long execFlag;
  char *pArg;
  char *pEnv;
  struct _RESULTCODES *pRes;
  char *pName;
};

void os2server_dos_ExecPgm_worker(struct DosExecPgm_params *parm);

void CV
os2server_dos_Exit_component(CORBA_Object obj,
                             ULONG action, ULONG result,
                             CORBA_srv_env *_srv_env)
{
  unsigned long ppid;
  struct t_os2process *proc, *parentproc;  
  //int t;

  // get caller t_os2process structure
  proc = PrcGetProcL4(*obj);

  // kill calling thread; @todo: implement real thread termination!
  //if ((t = l4ts_kill_task(*obj, L4TS_KILL_SYNC)))
  //    io_log("Error %d killing task\n", t);
  //else
  //    io_log("task killed\n");

  // get parent pid
  ppid = proc->lx_pib->pib_ulppid;
  // get parent proc
  parentproc = PrcGetProc(ppid);
  if (!parentproc)
  {
    io_log("parent proc is 0\n");
    return;
  }
  // set termination code
  parentproc->term_code = result;
  // unblock parent thread
  if ( parentproc->exec_sync &&
       (ppid || l4_thread_equal(parentproc->task, sysinit_id)) )
    l4semaphore_up(&parentproc->term_sem);
  io_log("semaphore unblock\n");
  // destroy calling thread's proc
  PrcDestroy(proc);
  io_log("proc destroy\n");

  return;
}


/* DosExecPgm worker thread */
void
os2server_dos_ExecPgm_worker(struct DosExecPgm_params *parm)
{
  CORBA_Environment env = default_env;
  struct t_os2process *proc;
  APIRET rc;
  char *p;
  int  i, l;

  io_log("worker start\n");
  /* get caller t_os2process structure */
  proc = parm->proc;
 
  l = strlstlen(parm->pArg);
  io_log("pArg len=%ld\n", (ULONG)l);
  io_log("pEnv len=%ld\n", (ULONG)strlstlen(parm->pEnv));

  io_log("pArg=%lx\n", (ULONG)parm->pArg);
  
  for (i = 0, p = parm->pArg; i < l; i++)
    if (p[i])
      io_log("%c\n", p[i]);
    else
      io_log("\\0\n");

  io_log("pEnv=%lx\n", (ULONG)parm->pEnv);
  
  for (i = 0, p = parm->pEnv; i < l; i++)
    if (p[i])
      io_log("%c\n", p[i]);
    else
      io_log("\\0\n");

  io_log("begin exec\n");
  /* try executing the new task */
  rc =  PrcExecuteModule(parm->pObjname, parm->cbObjname, parm->execFlag,
                         parm->pArg, parm->pEnv, parm->pRes, parm->pName, proc->pid);
  io_log("end exec\n");

  /* if child execution is synchronous
     and it is started successfully, 
     block until it terminates */
  io_log("term wait\n");
  if (!rc && parm->execFlag == EXEC_SYNC)
  {
    // do a sync wait
    proc->exec_sync = 1;
    l4semaphore_down(&proc->term_sem);
  }
  // sync wait done
  proc->exec_sync = 0;
  io_log("done waiting\n");
  /* set termination code */
  parm->pRes->codeTerminate = 0; /* TC_EXIT, @todo add real termination cause */
  parm->pRes->codeResult    = proc->term_code;
  /* notify the server loop to return API result */
  io_log("pRes=%lx\n", (ULONG)parm->pRes);
  io_log("pObjname=%lx\n",  (ULONG)parm->pObjname);
  io_log("cbObjname=%lx\n", (ULONG)parm->cbObjname);
  os2server_dos_ExecPgm_notify_call(&os2srv, &parm->thread, parm->pObjname, 
                                    parm->cbObjname, parm->pRes, rc, &env);
  /* free our parameters structure */
  free(parm->pArg);
  free(parm->pEnv);
  free(parm);
  io_log("worker terminate\n");
  /* terminate the worker thread */
  l4thread_exit();
}

/* notifier for main DosExecPgm component */
void CV
os2server_dos_ExecPgm_notify_component (CORBA_Object obj,
                            const l4os3_cap_idx_t *job /* in */,
                            const char* pObjname /* in */,
                            int cbObjname /* in */,
                            const struct _RESULTCODES *pRes /* in */,
                            int result /* in */,
                            CORBA_srv_env *_srv_env)
{
  io_log("pRes=%lx\n", (ULONG)pRes);
  io_log("pObjname=%lx\n", (ULONG)pObjname);
  io_log("cbObjname=%lx\n", (ULONG)cbObjname);
  io_log("pRes->codeTerminate=%lx\n", (ULONG)pRes->codeTerminate);
  io_log("pRes->codeResult=%lx\n", (ULONG)pRes->codeResult);
  os2server_dos_ExecPgm_reply ((l4os3_cap_idx_t *)job, result, (char **)&pObjname, 
			       (long *)&cbObjname, (struct _RESULTCODES *)pRes, _srv_env);
}

APIRET CV
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
                                 CORBA_srv_env *_srv_env)
{
  struct t_os2process *proc;
  struct DosExecPgm_params *parm;
  l4os3_cap_idx_t thread;
  //APIRET rc;
  //int    ret;
  char *arg, *env;

  /* caller thread id */
  thread = *obj;
  /* caller t_os2process structure */
  proc = PrcGetProcL4(thread);
  /* allocate parameters structure for worker thread */
  parm = (struct DosExecPgm_params *)malloc(sizeof(struct DosExecPgm_params));

  if (!parm)
    return 8; /* ERROR_NOT_ENOUGH_MEMORY */

  io_log("pRes=%lx\n", (ULONG)pRes);
  io_log("pObjname=%lx\n",  (ULONG)*pObjname);
  io_log("cbObjname=%lx\n", (ULONG)*cbObjname);

  io_log("len of pArg=%ld\n", (ULONG)strlstlen((char *)pArg));

  arg = malloc(arglen);
  env = malloc(envlen);
  
  strlstcpy(arg, (char *)pArg);
  strlstcpy(env, (char *)pEnv);

  /* fill in the params structure */
  parm->proc = proc;
  parm->thread = thread;
  parm->pObjname = *pObjname;
  parm->cbObjname = *cbObjname;
  parm->execFlag = execFlag;
  parm->pArg = arg;
  parm->pEnv = env;
  parm->pRes = pRes;
  parm->pName = (char *)pName;
  
  /* start DosExecPgm worker thread */
  l4thread_create((void *)os2server_dos_ExecPgm_worker, (void *)parm, L4THREAD_CREATE_ASYNC);
  *dice_reply = DICE_NO_REPLY; // do not reply to the client until notification

  return 0;
}

int
strlstlen(char *p);

/* copy string lists */
int
strlstcpy(char *s1, char *s2)
{
  char *p, *q;
  int  l, len = 0;
  
  p = s1; q = s2;
  while (*q)
  {
    strcpy(p, q);
    l = strlen(q) + 1;
    p   += l;
    q   += l;
    len += l;
  }
  *p = '\0';
  len++;

  return len;
}

APIRET CV
os2server_dos_GetPIB_component (CORBA_Object obj,
                                l4dm_dataspace_t *ds /* out */,
                                CORBA_srv_env *_srv_env)
{
  APIRET rc;
  l4_addr_t addr;
  l4_size_t size;
  l4_offs_t offset;
  l4os3_cap_idx_t pager;
  
  unsigned base;
  struct t_os2process *proc;
  PPIB ppib;

  // process PTDA  
  proc = PrcGetProcL4(*obj);
  ppib = proc->lx_pib;

  // fixup the PIB fields, so all
  // addresses are based from the dataspace start
  base = (unsigned)ppib;
  ppib->pib_pchcmd -= base;
  ppib->pib_pchenv -= base;
  
  rc = l4rm_lookup_region(ppib, &addr, &size, ds,
                          &offset, &pager);

  if (rc < 0)
    return ERROR_INVALID_ADDRESS;

  // share the dataspace with an application    
  rc = l4dm_share(ds, *obj, L4DM_RW);
  
  if (rc < 0)
    return ERROR_INVALID_ACCESS;

  return NO_ERROR;
}

APIRET CV
os2server_dos_GetTIB_component (CORBA_Object obj,
                                l4dm_dataspace_t *ds /* out */,
                                CORBA_srv_env *_srv_env)
{
  APIRET rc;
  unsigned base;
  struct t_os2process *proc;
  PTIB ptib;
  TID  tid;
  l4_addr_t addr;
  l4_size_t size;
  l4_offs_t offset;
  l4os3_cap_idx_t pager;

  // process PTDA
  proc = PrcGetProcL4(*obj);
  tid  = PrcGetTIDL4(*obj);
  ptib = proc->tib_array[tid - 1];

  // fixup the PIB fields, so all
  // addresses are based from the dataspace start
  base = (unsigned)ptib;
  ptib->tib_ptib2 = (PTIB2)((char*)ptib->tib_ptib2 - base);

  rc = l4rm_lookup_region(ptib, &addr, &size, ds,
                          &offset, &pager);

  if (rc < 0)
    return ERROR_INVALID_ADDRESS;

  // share the dataspace with an application
  rc = l4dm_share(ds, *obj, L4DM_RW);

  if (rc < 0)
    return ERROR_INVALID_ACCESS;

  return NO_ERROR;
}

APIRET CV
os2server_dos_Error_component (CORBA_Object obj,
                               ULONG error /* in */,
                               CORBA_srv_env *_srv_env)
{
  return 0; /* NO_ERROR */
}

APIRET CV
os2server_dos_QueryDBCSEnv_component (CORBA_Object obj,
                                      ULONG *cb /* in, out */,
                                      const COUNTRYCODE *pcc /* out */,
                                      char **pBuf /* in */,
                                      CORBA_srv_env *_srv_env)
{
  if (cb && *cb)
    memset(*pBuf, 0, *cb); // empty
    
  return 0; /* NO_ERROR */
}


APIRET CV
os2server_dos_QueryCp_component (CORBA_Object obj,
                                 ULONG *cb /* in, out */,
                                 char  **arCP /* out */,
                                 CORBA_srv_env *_srv_env)
{
  if (*cb < 3 * sizeof(ULONG))
    return 473; /* ERROR_CPLIST_TOO_SMALL */

  io_log("cb=%ld\n", (ULONG)*cb);
  io_log("arCP=%lx\n", (ULONG)*arCP);

  (*(ULONG **)arCP)[0] = 437; /* current codepage   */
  (*(ULONG **)arCP)[1] = 437; /* primary codepage   */
  (*(ULONG **)arCP)[2] = 850; /* secondary codepage */
  *cb = 3 * sizeof(ULONG);
  
  return 0; /* NO_ERROR */
}

APIRET CV
os2server_dos_QueryCurrentDisk_component (CORBA_Object obj,
                                          ULONG *pdisknum /* out */,
                                          //ULONG logical /* in */,
                                          CORBA_srv_env *_srv_env)
{
  //CORBA_Environment env = dice_default_environment;
  ULONG n;
  struct t_os2process *proc;

  proc = PrcGetProcL4(*obj);
  
  io_log("proc=%lx\n", (ULONG)proc);
  n = proc->curdisk;
  io_log("n=%lx\n", n);
  *pdisknum = n;

  // get drive map from fs server  
  io_log("os2fs tid: %x.%x\n", fs.id.task, fs.id.lthread);
  //os2fs_get_drivemap_call(&fs, &logical, &env);
  //*plogical = 1 << (n - 1);

  return 0; /* NO_ERROR */
}


APIRET CV
os2server_dos_QueryCurrentDir_component (CORBA_Object obj,
                                         ULONG disknum /* in */,
                                         ULONG logical /* in */,
                                         char **pBuf /* out */,
                                         ULONG *pcbBuf /* out */,
                                         CORBA_srv_env *_srv_env)
{
  //CORBA_Environment env = dice_default_environment;
  ULONG disk;
  struct t_os2process *proc;
  //struct I_Fs_srv *fs_srv;
  char buf[0x100];
  char *curdir = buf;
  //char drv;
  //int i;

  proc = PrcGetProcL4(*obj);
  curdir = proc->curdir;

  if (!disknum)
  {
    disk = proc->curdisk;
    //os2fs_get_drivemap_call(&fs, &map, &env);
  }
  else
    disk = disknum;

  io_log("disk=%lx\n", disk);

  if (!((1 << (disk - 1)) & logical))
    return 15; /* ERROR_INVALID_DRIVE */

  if (*pcbBuf < strlen(curdir) + 1)
    return ERROR_BUFFER_OVERFLOW;
  else
  {
    *pcbBuf = strlen(curdir) + 1;
    strcpy(*pBuf, curdir);
    return NO_ERROR;
  }

  return 0; /* NO_ERROR */
}

/* changes the current directory in '*dir'
   by one path component in 'component' */
int cdir(char **dir, char *component)
{
  char *p;

  if (!strcmp(component, ".."))
  {
    if (**dir)
    {
      p = *dir + strlen(*dir);

      // find last backslash position
      while (p >= *dir && *p != '\\') p--;
      if (p < *dir) p++;
      *p = '\0';
    }
    return 0; /* NO_ERROR */  
  }

  if (!strcmp(component, "."))
    return 0;

  if (*component != '\\')
  {
    if (**dir)
      strcat(*dir, "\\");

    strcat(*dir, component);
  }
  else
    strcpy(*dir, component);

  return 0;
}

APIRET CV
os2server_dos_SetCurrentDir_component (CORBA_Object obj,
                                       PCSZ pszDir /* in */,
                                       CORBA_srv_env *_srv_env)
{
  //CORBA_Environment env = dice_default_environment;
  struct t_os2process *proc;
  //ULONG disknum;
  char str[0x100];
  char buf[0x100];
  char *s = buf;
  char *p, *q, *r;

  proc = PrcGetProcL4(*obj);

  if (proc == NULL)
      return ERROR_INVALID_PROCID;

  s = proc->curdir;

  if (pszDir == NULL)
  {
    *s = '\0';
    return NO_ERROR;
  }

  p = q = (char *)pszDir;

  for (r = p; *r; r++)
    if (*r == '/')
      *r = '\\';

  if (!strcmp(pszDir, "\\"))
  {
    *s = '\0';
    return NO_ERROR;
  }

  do
  {
    p = strstr(p, "\\");

    if (p)
    {
      strncpy(str, q, p - q);
      str[p - q] = '\0';
      p++;
    }
    else
      strcpy(str, q);

    cdir(&s, str);
    q = p;
  }
  while (p);

  return NO_ERROR;
}

APIRET CV
os2server_dos_SetDefaultDisk_component (CORBA_Object obj,
                                        ULONG disknum /* in */,
                                        ULONG logical /* in */,
                                        CORBA_srv_env *_srv_env)
{
  //CORBA_Environment env = dice_default_environment;
  struct t_os2process *proc;
  //ULONG  map;

  proc = PrcGetProcL4(*obj);

  // get drive map from fs server
  //os2fs_get_drivemap_call(&fs, &map, &env);

  if (!((1 << (disknum - 1)) & logical))
    return ERROR_INVALID_DRIVE;

  io_log("map=%lx", logical);
  proc->curdisk = disknum;

  return NO_ERROR;
}

APIRET DICE_CV
os2server_dos_CreateEventSem_component (CORBA_Object obj,
                                        const char* pszName /* in */,
                                        HEV *phev /* out */,
                                        ULONG flAttr /* in */,
                                        BOOL32 fState /* in */,
                                        CORBA_srv_env *_srv_env)
{
  CORBA_Environment env = default_env;
  APIRET rc;
  HEV    hev;
  SEM    *sem;

  if (fState > 1 || flAttr > DC_SEM_SHARED)
    return ERROR_INVALID_PARAMETER;

  if (pszName && *pszName && strstr(pszName, "\\SEM32\\") != pszName)
    return ERROR_INVALID_NAME;

  if (! os2server_dos_OpenEventSem_call(&os2srv, pszName, &hev, &env) )
  {
    os2server_dos_CloseEventSem_call(&os2srv, hev, &env);
    return ERROR_DUPLICATE_NAME;
  }

  /* allocate a hev for a new event semaphore */
  if ( (rc = HndAllocateHandle(htSem, &hev, (HANDLE **)&sem)) )
    return rc;

  if (!pszName || !*pszName)
  {
    // shared semaphore
    sem->cShared = flAttr;
  }
  else
  {
    // set name  
    strncpy(sem->szName, pszName, CCHMAXPATH);
    sem->szName[CCHMAXPATH - 1] = '\0';
    sem->cShared = TRUE;
  }

  sem->cType = SEMTYPE_EVENT;

  // set initial state
  sem->uSem.evt  = L4SEMAPHORE_INIT(fState);
  sem->ulRefCnt = 1;

  // return handle
  *phev = hev;
  return NO_ERROR;
}

APIRET DICE_CV
os2server_dos_OpenEventSem_component (CORBA_Object obj,
                                      const char* pszName /* in */,
                                      HEV *phev /* in, out */,
                                      CORBA_srv_env *_srv_env)
{
  //APIRET rc;
  SEM    *sem;

  if (phev && (pszName || *pszName))
    return ERROR_INVALID_PARAMETER;

  if (*phev)
  {
    // open by handle
    if (!HndIsValidIndexHandle(htSem, *phev, (HANDLE **)&sem))
    {
      if (sem->cType == SEMTYPE_EVENT && sem->cShared)
      {
        if (!sem->ulRefCnt)
        return ERROR_TOO_MANY_OPENS;

        // increment refcount
        sem->ulRefCnt++;

        return NO_ERROR;
      }
    }
    else
      return ERROR_INVALID_HANDLE;
  }
  else
  {
    // open by name
    if (strstr(pszName, "\\SEM32\\") != pszName)
      return ERROR_INVALID_NAME;

    for (sem = (SEM *)(htSem->pFirstHandle); sem; sem = sem->pNext)
    {
      if (sem->szName && !strcmp(sem->szName, pszName))
        break;
    }

    if (sem)
    {
      if (!sem->ulRefCnt)
        return ERROR_TOO_MANY_OPENS;

      // increment refcount
      sem->ulRefCnt++;

      *phev = (ULONG)(((PCHAR)sem - (PCHAR)htSem->pFirstHandle) / htSem->ulHandleSize);
      return NO_ERROR;
    }
    else
      return ERROR_SEM_NOT_FOUND;
  }

  return NO_ERROR;
}

APIRET DICE_CV
os2server_dos_CloseEventSem_component (CORBA_Object obj,
                                       HEV hev /* in */,
                                       CORBA_srv_env *_srv_env)
{
  return NO_ERROR;
}

APIRET DICE_CV
os2server_dos_GetTID_component (CORBA_Object obj,
                                TID *ptid /* in */,
                                CORBA_srv_env *_srv_env)
{
  *ptid = PrcGetTIDL4(*obj);
  return NO_ERROR;
}

APIRET DICE_CV
os2server_dos_GetPID_component (CORBA_Object obj,
                                PID *ppid /* in */,
                                CORBA_srv_env *_srv_env)
{
  struct t_os2process *proc = PrcGetProcL4(*obj);
  *ppid = proc->pid;
  return NO_ERROR;
}

APIRET DICE_CV
os2server_dos_GetL4ID_component (CORBA_Object obj,
                                 PID pid /* in */,
                                 TID tid /* in */,
                                 l4thread_t *id /* out */,
                                 CORBA_srv_env *_srv_env)
{
  *id = PrcGetL4ID(pid, tid);
  return NO_ERROR;
}

APIRET DICE_CV
os2server_dos_GetTIDL4_component (CORBA_Object obj,
                                  const l4_threadid_t *id /* in */,
                                  TID *ptid /* out */,
                                  CORBA_srv_env *_srv_env)
{
  *ptid = PrcGetTIDL4(*id);
  return NO_ERROR;
}

APIRET DICE_CV
os2server_dos_NewTIB_component (CORBA_Object obj,
                                PID pid /* in */,
                                TID tid /* in */,
                                l4thread_t id /* in */,
                                CORBA_srv_env *_srv_env)
{
  return PrcNewTIB(pid, tid, id);
}

APIRET DICE_CV
os2server_dos_DestroyTIB_component (CORBA_Object obj,
                                    PID pid /* in */,
                                    TID tid /* in */,
                                    CORBA_srv_env *_srv_env)
{
  return PrcDestroyTIB(pid, tid);
}
