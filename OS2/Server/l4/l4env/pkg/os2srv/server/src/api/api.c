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
//#include <l4/sys/types.h>
#include <l4/semaphore/semaphore.h>
#include <l4/thread/thread.h>
#include <l4/generic_ts/generic_ts.h>
//#include <l4/dm_mem/dm_mem.h>
//#include <l4/l4rm/l4rm.h>

// osFree internal includes
#include <l4/os3/types.h>
#include <l4/os3/dataspace.h>
#include <l4/os3/rm.h>
#include <l4/os3/gcc_os2def.h>
#include <l4/os3/processmgr.h>
#include <l4/os3/globals.h>
#include <l4/os3/io.h>

// IPC includes
#include <l4/os3/ipc.h>

// os2fs RPC includes
#include <l4/os2fs/os2fs-client.h>

// os2srv RPC includes
#include <os2server-client.h>
#include <os2server-server.h>

extern l4_cap_idx_t fs;
extern l4_cap_idx_t os2srv;
extern l4_cap_idx_t sysinit_id;
extern struct t_os2process *proc_root;

int cdir(char **dir, char *component);
int attach_ds(l4re_ds_t *ds, l4_uint32_t flags, l4_addr_t *addr);
int strlstcpy(char *s1, char *s2);
int strlstlen(char *p);

struct DosExecPgm_params {
  struct t_os2process *proc;
  l4_cap_idx_t thread;
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
  //    io_printf("Error %d killing task\n", t);
  //else
  //    io_printf("task killed");

  io_printf("0\n");
  // get parent pid
  ppid = proc->lx_pib->pib_ulppid;
  io_printf("1\n");
  // get parent proc
  parentproc = PrcGetProc(ppid);
  if (!parentproc)
  {
    io_printf("parent proc is 0\n");
    return;
  }
  // unblock parent thread
  if (ppid || l4_thread_equal(parentproc->task, sysinit_id))
    l4semaphore_up(&parentproc->term_sem);
  io_printf("semaphore unblock\n");
  // destroy calling thread's proc
  PrcDestroy(proc);
  io_printf("proc destroy\n");

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

  io_printf("worker start\n");
  /* get caller t_os2process structure */
  proc = parm->proc;
 
  l = strlstlen(parm->pArg);
  io_printf("pArg len=%ld\n", (ULONG)l);
  io_printf("pEnv len=%ld\n", (ULONG)strlstlen(parm->pEnv));

  io_printf("pArg=%lx\n", (ULONG)parm->pArg);
  
  for (i = 0, p = parm->pArg; i < l; i++)
    if (p[i])
      io_printf("%c\n", p[i]);
    else
      io_printf("\\0\n");

  io_printf("pEnv=%lx\n", (ULONG)parm->pEnv);
  
  for (i = 0, p = parm->pEnv; i < l; i++)
    if (p[i])
      io_printf("%c\n", p[i]);
    else
      io_printf("\\0\n");

  io_printf("begin exec\n");
  /* try executing the new task */
  rc =  PrcExecuteModule(parm->pObjname, parm->cbObjname, parm->execFlag,
                         parm->pArg, parm->pEnv, parm->pRes, parm->pName, proc->pid);
  io_printf("end exec\n");

  /* if child execution is synchronous
     and it is started successfully, 
     block until it terminates */
  io_printf("term wait\n");
  if (!rc && parm->execFlag == EXEC_SYNC)
    l4semaphore_down(&proc->term_sem);
  io_printf("done waiting\n");    
  /* notify the server loop to return API result */
  io_printf("0\n");
  io_printf("pRes=%lx\n", (ULONG)parm->pRes);
  io_printf("pObjname=%lx\n",  (ULONG)parm->pObjname);
  io_printf("cbObjname=%lx\n", (ULONG)parm->cbObjname);
  os2server_dos_ExecPgm_notify_call(&os2srv, &parm->thread, parm->pObjname, 
                                    parm->cbObjname, parm->pRes, rc, &env);
  io_printf("1\n");
  /* free our parameters structure */
  free(parm->pArg);
  free(parm->pEnv);
  free(parm);
  io_printf("worker terminate\n");
  /* terminate the worker thread */
  l4thread_exit();
}

/* notifier for main DosExecPgm component */
void CV
os2server_dos_ExecPgm_notify_component (CORBA_Object obj,
                            const l4_cap_idx_t *job /* in */,
                            const char* pObjname /* in */,
                            int cbObjname /* in */,
                            const struct _RESULTCODES *pRes /* in */,
                            int result /* in */,
                            CORBA_srv_env *_srv_env)
{
  io_printf("111\n");
  io_printf("pRes=%lx\n", (ULONG)pRes);
  io_printf("pObjname=%lx\n", (ULONG)pObjname);
  io_printf("cbObjname=%lx\n", (ULONG)cbObjname);
  io_printf("pRes->codeTerminate=%lx\n", (ULONG)pRes->codeTerminate);
  io_printf("pRes->codeResult=%lx\n", (ULONG)pRes->codeResult);
  os2server_dos_ExecPgm_reply ((l4_cap_idx_t *)job, result, (char **)&pObjname, 
			       (long *)&cbObjname, (struct _RESULTCODES *)pRes, _srv_env);
  io_printf("b\n");
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
  l4_cap_idx_t thread;
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

  io_printf("pRes=%lx", (ULONG)pRes);
  io_printf("pObjname=%lx",  (ULONG)*pObjname);
  io_printf("cbObjname=%lx", (ULONG)*cbObjname);

  io_printf("len of pArg=%ld", (ULONG)strlstlen((char *)pArg));

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
  l4_cap_idx_t pager;
  
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
  l4_addr_t addr;
  l4_size_t size;
  l4_offs_t offset;
  l4_cap_idx_t pager;

  // process PTDA  
  proc = PrcGetProcL4(*obj);
  ptib = proc->main_tib;

  // fixup the PIB fields, so all
  // addresses are based from the dataspace start
  base = (unsigned)ptib;
  ptib->tib_ptib2  -= base;
  
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

  io_printf("cb=%ld", (ULONG)*cb);
  io_printf("arCP=%lx", (ULONG)*arCP);

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
  io_printf("aaa1\n");

  proc = PrcGetProcL4(*obj);
  
  io_printf("aaa2: proc=%lx\n", (ULONG)proc);
  n = proc->curdisk;
  io_printf("aaa3: n=%lx\n", n);
  *pdisknum = n;
  io_printf("aaa4\n");

  // get drive map from fs server  
  io_printf("os2fs tid: %x.%x\n", fs.id.task, fs.id.lthread);
  //os2fs_get_drivemap_call(&fs, &logical, &env);
  //*plogical = 1 << (n - 1);
  io_printf("aaa5\n");

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

  io_printf("bbb1\n");
  proc = PrcGetProcL4(*obj);
  curdir = proc->curdir;

  io_printf("bbb2\n");
  if (!disknum)
  {
    disk = proc->curdisk;
    //os2fs_get_drivemap_call(&fs, &map, &env);
  }
  else
    disk = disknum;

  io_printf("bbb3: disk=%lx\n", disk);

  if (!((1 << (disk - 1)) & logical))
    return 15; /* ERROR_INVALID_DRIVE */

  io_printf("bbb5\n");
  if (*pcbBuf < strlen(curdir) + 1)
    return ERROR_BUFFER_OVERFLOW;
  else
  {
    io_printf("bbb6\n");
    *pcbBuf = strlen(curdir) + 1;
    io_printf("bbb7\n");
    strcpy(*pBuf, curdir);
    io_printf("bbb8\n");
    return NO_ERROR;
  }

  io_printf("bbb9\n");
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

  io_printf("0");
  proc = PrcGetProcL4(*obj);

  // get drive map from fs server
  //os2fs_get_drivemap_call(&fs, &map, &env);

  io_printf("1");
  if (!((1 << (disknum - 1)) & logical))
    return ERROR_INVALID_DRIVE;

  io_printf("map=%lx", logical);
  io_printf("2");
  proc->curdisk = disknum;


  return NO_ERROR;
}
