/* API implementations
 * (on the server side)
 */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal includes */
#include <os3/types.h>
#include <os3/dataspace.h>
#include <os3/rm.h>
#include <os3/processmgr.h>
#include <os3/thread.h>
#include <os3/semaphore.h>
#include <os3/handlemgr.h>
#include <os3/globals.h>
#include <os3/cpi.h>
#include <os3/io.h>

/* libc includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

/* local includes */
#include "api.h"

extern l4_os3_thread_t sysinit_id;

extern struct t_os2process *proc_root;

/* Semaphore handle table pointer */
HANDLE_TABLE *htSem;

void CPTest(void)
{
  io_log("Hello OS/2!\n");
}

APIRET CPExit(l4_os3_thread_t thread,
              ULONG action,
              ULONG result)
{
  unsigned long ppid;
  struct t_os2process *proc, *parentproc;

  // get caller t_os2process structure
  proc = PrcGetProcNative(thread);

  if (! proc)
  {
    return ERROR_PROC_NOT_FOUND;
  }

  // get parent pid
  ppid = proc->lx_pib->pib_ulppid;

  // get parent proc
  parentproc = PrcGetProc(ppid);

  if (! parentproc)
  {
    io_log("parent proc is NULL!\n");
    return ERROR_PROC_NOT_FOUND;
  }

  // set termination code
  parentproc->term_code = result;

  io_log("parentproc->exec_sync=%x\n", parentproc->exec_sync);
  io_log("ppid=%x\n", ppid);
  //io_log("parentproc->task=%lx:%lx, sysinit_id=%lx:%lx\n", 
    //parentproc->task.thread.id.task, parentproc->task.thread.id.lthread, 
    //sysinit_id.thread.id.task, sysinit_id.thread.id.lthread);

  // unblock parent thread
  if ( parentproc->exec_sync &&
       (ppid || TaskEqual(parentproc->task, sysinit_id)) )
  {
    SemaphoreUp(&parentproc->term_sem);
    io_log("semaphore unblock\n");
  }

  // destroy calling thread's proc
  io_log("proc destroy\n");
  PrcDestroy(proc);

  return NO_ERROR;
}

/* DosExecPgm worker thread */
void CPExecPgmWorker(struct DosExecPgm_params *parm)
{
  //CORBA_Environment env = dice_default_environment;
  struct t_os2process *proc;
  APIRET rc;
  char *p;
  int  i, l;

  /* get caller t_os2process structure */
  io_log("worker start\n");
  proc = parm->proc;

  l = strlstlen(parm->pArg);

  io_log("pArg len=%ld\n", (ULONG)l);
  io_log("pEnv len=%ld\n", (ULONG)strlstlen(parm->pEnv));

  io_log("pArg=%lx\n", (ULONG)parm->pArg);

  for (i = 0, p = parm->pArg; i < l; i++)
  {
    if (p[i])
      io_log("%c\n", p[i]);
    else
      io_log("\\0\n");
  }

  io_log("pEnv=%lx\n", (ULONG)parm->pEnv);

  for (i = 0, p = parm->pEnv; i < l; i++)
  {
    if (p[i])
      io_log("%c\n", p[i]);
    else
      io_log("\\0\n");
  }

  io_log("begin exec\n");
  /* try executing the new task */
  rc =  PrcExecuteModule(parm->pObjname, parm->cbObjname, parm->execFlag,
                         parm->pArg, parm->pEnv, parm->pRes, parm->pName, proc->pid);
  io_log("end exec\n");

  if (! rc)
  {
    // wait for successful startup
    SemaphoreDown(&proc->startup_sem);
  }

  /* if child execution is synchronous
     and it is started successfully,
     block until it terminates */
  io_log("term wait\n");
  if (parm->execFlag == EXEC_SYNC)
  {
    // do a sync wait
    proc->exec_sync = 1;

    if (! rc)
      SemaphoreDown(&proc->term_sem);
  }

  // sync wait done
  proc->exec_sync = 0;

  io_log("done waiting\n");

  /* set termination code */
  parm->pRes->codeResult = proc->term_code;

  /* notify the server loop to return API result */
  io_log("pRes=%lx\n", (ULONG)parm->pRes);
  io_log("pObjname=%lx\n",  (ULONG)parm->pObjname);
  io_log("cbObjname=%lx\n", (ULONG)parm->cbObjname);

#ifdef L4API_l4v2
  CPClientExecPgmNotify(parm->thread, parm->pObjname,
                        parm->cbObjname,
                        parm->pRes, rc);
#endif

  /* free our parameters structure */
  free(parm->pArg);
  free(parm->pEnv);
  free(parm);
  io_log("worker terminate\n");

  /* terminate the worker thread */
  ThreadExit();
}

APIRET CPExecPgm(l4_os3_thread_t thread,
                 char **pObjname,
                 long *cbObjname,
                 unsigned long execFlag,
                 const char* pArg,
                 unsigned long arglen,
                 const char* pEnv,
                 unsigned long envlen,
                 struct _RESULTCODES *pRes,
                 const char* pName)
{
  struct t_os2process *proc;
  struct DosExecPgm_params *parm;
  char *arg, *env;

  /* caller t_os2process structure */
  proc = PrcGetProcNative(thread);

  if (! proc)
  {
    return ERROR_PROC_NOT_FOUND;
  }

  /* allocate parameters structure for worker thread */
  parm = (struct DosExecPgm_params *)malloc(sizeof(struct DosExecPgm_params));

  if (! parm)
    return ERROR_NOT_ENOUGH_MEMORY;

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
  //l4thread_create((void *)CPExecPgmWorker, (void *)parm, L4THREAD_CREATE_ASYNC);
  ThreadCreate((void *)CPExecPgmWorker, (void *)parm, THREAD_ASYNC);

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

APIRET CPGetPIB(PID pid, l4_os3_thread_t thread,
                l4_os3_dataspace_t *ds)
{
  struct t_os2process *proc;
  l4_os3_dataspace_t orig_ds;
  char *base;
  PPIB ppib;
  APIRET rc;
  void *addr_orig;
  void *addr;
  ULONG offset;
  ULONG size;

  // process PTDA
  proc = PrcGetProc(pid);

  if (! proc)
  {
    return ERROR_PROC_NOT_FOUND;
  }

  ppib = proc->lx_pib;
  io_log("ppib_orig=%lx\n", ppib);

  //rc = l4rm_lookup_region(ppib, &addr, &size, ds,
    //                      &offset, &pager);
  rc = RegLookupRegion(ppib, &addr_orig, &size, &offset, &orig_ds);
  io_log("addr_orig=%lx\n", addr_orig);

  //if (rc < 0)
  if (rc != REG_DATASPACE)
    return ERROR_INVALID_ADDRESS;

  rc = DataspaceAlloc(ds, DATASPACE_RW, DEFAULT_DSM, size);

  if (rc)
    return ERROR_NOT_ENOUGH_MEMORY;

  addr = addr_orig;

  rc = RegAttach(&addr, size, DATASPACE_RW, *ds, 0, 0);

  if (rc)
    return rc;

  memcpy(addr, addr_orig, size);

  // share the dataspace with an application
  //rc = l4dm_share(ds, *obj, L4DM_RW);
  rc = DataspaceShare(*ds, thread, DATASPACE_RW);

  //if (rc < 0)
  if (rc)
    return ERROR_INVALID_ACCESS;

  // fixup the PIB fields, so all
  // addresses are based from the dataspace start
  ppib = (PPIB)addr;
  base = (char *)addr;
  io_log("ppib=%lx\n", ppib);
  //ppib->pib_pchcmd -= base;
  //ppib->pib_pchenv -= base;
  io_log("base=%lx\n", base);
  io_log("0: pchcmd=%lx, pchenv=%lx\n", ppib->pib_pchcmd, ppib->pib_pchenv);
  ppib->pib_pchcmd = (char *)((char *)ppib->pib_pchcmd - (char *)addr_orig);
  ppib->pib_pchenv = (char *)((char *)ppib->pib_pchenv - (char *)addr_orig);
  io_log("1: pchcmd=%lx, pchenv=%lx\n", ppib->pib_pchcmd, ppib->pib_pchenv);

  return NO_ERROR;
}

APIRET CPGetTIB(PID pid, TID tid, l4_os3_thread_t thread,
                l4_os3_dataspace_t *ds)
{
  struct t_os2process *proc;
  l4_os3_dataspace_t orig_ds;
  //char *base;
  PTIB ptib;
  void *addr_orig;
  void *addr;
  ULONG offset;
  ULONG size;
  APIRET rc;

  // process PTDA
  proc = PrcGetProc(pid);

  if (! proc)
  {
    return ERROR_PROC_NOT_FOUND;
  }

  //tid  = PrcGetTIDNative(thread);
  ptib = proc->tib_array[tid - 1];

  //rc = l4rm_lookup_region(ptib, &addr, &size, ds,
    //                      &offset, &pager);
  rc = RegLookupRegion((void *)ptib, &addr_orig, &size, &offset, &orig_ds);

  //if (rc < 0)
  if (rc != REG_DATASPACE)
    return ERROR_INVALID_ADDRESS;

  rc = DataspaceAlloc(ds, DATASPACE_RW, DEFAULT_DSM, size);

  if (rc)
    return ERROR_NOT_ENOUGH_MEMORY;

  addr = addr_orig;

  rc = RegAttach(&addr, size, DATASPACE_RW, *ds, 0, 0);

  if (rc)
    return rc;

  memcpy(addr, addr_orig, size);

  // share the dataspace with an application
  //rc = l4dm_share(ds, *obj, L4DM_RW);
  rc = DataspaceShare(*ds, thread, DATASPACE_RW);
  io_log("rc=%lx\n", rc);

  //if (rc < 0)
  if (rc)
    return ERROR_INVALID_ACCESS;

  // fixup the PIB fields, so all
  // addresses are based from the dataspace start
  ptib = (PTIB)addr;
  //base = (char *)addr;
  io_log("tid=%lx\n", tid);
  io_log("ptib=%lx\n", ptib);
  io_log("ptib2=%lx\n", ptib->tib_ptib2);
  ptib->tib_ptib2 = (PTIB2)((char *)ptib->tib_ptib2 - (char *)addr_orig);
  io_log("ptib2=%lx\n", ptib->tib_ptib2);

  return NO_ERROR;
}

APIRET CPError(ULONG error)
{
  return NO_ERROR;
}

APIRET CPQueryDBCSEnv(ULONG *cb,
                      const COUNTRYCODE *pcc,
                      char **pBuf)
{
  if (cb && *cb)
    memset(*pBuf, 0, *cb); // empty

  return NO_ERROR;
}

APIRET CPQueryCp(ULONG *cb,
                 char  **arCP)
{
  if (*cb < 3 * sizeof(ULONG))
    return ERROR_CPLIST_TOO_SMALL;

  io_log("cb=%ld\n", (ULONG)*cb);
  io_log("arCP=%lx\n", (ULONG)*arCP);

  (*(ULONG **)arCP)[0] = 437; /* current codepage   */
  (*(ULONG **)arCP)[1] = 437; /* primary codepage   */
  (*(ULONG **)arCP)[2] = 850; /* secondary codepage */
  *cb = 3 * sizeof(ULONG);

  return NO_ERROR;
}

APIRET CPQueryCurrentDisk(l4_os3_thread_t thread,
                          ULONG *pdisknum)
{
  struct t_os2process *proc;
  ULONG n;

  proc = PrcGetProcNative(thread);

  if (! proc)
  {
    return ERROR_PROC_NOT_FOUND;
  }

  io_log("proc=%lx\n", (ULONG)proc);
  n = proc->curdisk;

  io_log("n=%lx\n", n);
  *pdisknum = n;

  // get drive map from fs server
  //io_log("os2fs tid: %x.%x\n", fs.id.task, fs.id.lthread);
  //os2fs_get_drivemap_call(&fs, &logical, &env);
  //*plogical = 1 << (n - 1);

  return NO_ERROR;
}

APIRET CPQueryCurrentDir(l4_os3_thread_t thread,
                         ULONG disknum,
                         ULONG logical,
                         char **pBuf,
                         ULONG *pcbBuf)
{
  ULONG disk;
  struct t_os2process *proc;
  char buf[0x100];
  char *curdir = buf;

  io_log("pcbBuf=%p\n", pcbBuf);
  io_log("pBuf=%p\n", pBuf);

  if (pBuf)
    io_log("*pBuf=%p\n", *pBuf);

  if (! pcbBuf || ! pBuf || ! *pBuf)
    return ERROR_INVALID_PARAMETER;

  proc = PrcGetProcNative(thread);

  if (! proc)
  {
    return ERROR_PROC_NOT_FOUND;
  }

  curdir = proc->curdir;

  if (! disknum)
  {
    disk = proc->curdisk;
    //os2fs_get_drivemap_call(&fs, &map, &env);
  }
  else
    disk = disknum;

  io_log("disk=%lx\n", disk);

  if (!((1 << (disk - 1)) & logical))
    return ERROR_INVALID_DRIVE;

  if (*pcbBuf && *pcbBuf < strlen(curdir) + 1)
    return ERROR_BUFFER_OVERFLOW;
  else
  {
    *pcbBuf = strlen(curdir) + 1;
    io_log("curdir=%s\n", curdir);
    strcpy(*pBuf, curdir);
    return NO_ERROR;
  }

  return NO_ERROR;
}

/* changes the current directory in '*dir'
   by one path component in 'component' */
int cdir(char **dir, char *component)
{
  char *p;

  if (! strcmp(component, "..") )
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

  if (! strcmp(component, ".") )
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

APIRET CPSetCurrentDir(l4_os3_thread_t thread,
                       PCSZ pszDir)
{
  struct t_os2process *proc;
  char str[0x100];
  char buf[0x100];
  char *s = buf;
  char *p, *q, *r;

  proc = PrcGetProcNative(thread);

  if (! proc)
  {
    return ERROR_PROC_NOT_FOUND;
  }

  s = proc->curdir;

  if (pszDir == NULL)
  {
    *s = '\0';
    return NO_ERROR;
  }

  p = q = (char *)pszDir;

  for (r = p; *r; r++)
  {
    if (*r == '/')
      *r = '\\';
  }

  if (! strcmp(pszDir, "\\"))
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

APIRET CPSetDefaultDisk(l4_os3_thread_t thread,
                        ULONG disknum,
                        ULONG logical)
{
  struct t_os2process *proc;

  proc = PrcGetProcNative(thread);

  if (! proc)
  {
    return ERROR_PROC_NOT_FOUND;
  }

  // get drive map from fs server
  //os2fs_get_drivemap_call(&fs, &map, &env);

  if (! ((1 << (disknum - 1)) & logical) )
    return ERROR_INVALID_DRIVE;

  io_log("map=%lx", logical);
  proc->curdisk = disknum;

  return NO_ERROR;
}

APIRET CPCreateEventSem(l4_os3_thread_t thread,
                        PCSZ pszName,
                        HEV *phev,
                        ULONG flAttr,
                        BOOL32 fState)
{
  APIRET rc;
  HEV    hev;
  SEM    *sem;

  if (fState > 1 || flAttr > DC_SEM_SHARED)
    return ERROR_INVALID_PARAMETER;

  if (pszName && *pszName && strstr(pszName, "\\SEM32\\") != pszName)
    return ERROR_INVALID_NAME;

  //if (! CPClientOpenEventSem(pszName, &hev) )
  if (! CPOpenEventSem(thread, pszName, &hev) )
  {
    //CPClientCloseEventSem(hev);
    CPCloseEventSem(thread, hev);
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
  sem->uSem.evt  = SEMAPHORE_INIT(fState);
  sem->ulRefCnt = 1;

  // return handle
  *phev = hev;
  return NO_ERROR;
}

APIRET CPOpenEventSem(l4_os3_thread_t thread,
                      PCSZ pszName,
                      HEV *phev)
{
  //APIRET rc;
  SEM    *sem;

  if (phev && (pszName || *pszName))
    return ERROR_INVALID_PARAMETER;

  if (*phev)
  {
    // open by handle
    if (! HndIsValidIndexHandle(htSem, *phev, (HANDLE **)&sem))
    {
      if (sem->cType == SEMTYPE_EVENT && sem->cShared)
      {
        if (! sem->ulRefCnt)
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
      if (! sem->ulRefCnt)
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

APIRET CPCloseEventSem(l4_os3_thread_t thread,
                       HEV hev)
{
  return NO_ERROR;
}

APIRET CPGetPID(l4_os3_thread_t thread,
                PID *ppid)
{
  struct t_os2process *proc = PrcGetProcNative(thread);

  if (! proc)
  {
    return ERROR_PROC_NOT_FOUND;
  }

  *ppid = proc->pid;
  return NO_ERROR;
}

APIRET CPGetNativeID(PID pid,
                     TID tid,
                     l4_os3_thread_t *id)
{
  *id = PrcGetNativeID(pid, tid);
  return NO_ERROR;
}

APIRET CPNewTIB(PID pid,
                TID tid,
                const l4_os3_thread_t *id)
{
  return PrcNewTIB(pid, tid, *id);
}

APIRET CPDestroyTIB(PID pid,
                    TID tid)
{
  return PrcDestroyTIB(pid, tid);
}
