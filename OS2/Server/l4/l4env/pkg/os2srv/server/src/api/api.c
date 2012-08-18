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

APIRET CV
os2server_dos_QueryCurrentDisk_component (CORBA_Object obj,
                                          ULONG *pdisknum /* out */,
                                          ULONG *plogical /* out */,
                                          CORBA_srv_env *_srv_env)
{
  CORBA_Environment env = default_env;
  ULONG n;
  struct t_os2process *proc;
  io_printf("1");

  proc = PrcGetProcL4(*obj);
  
  io_printf("2");
  n = proc->curdisk;
  *pdisknum = n;

  // get drive map from fs server  
  io_printf("os2fs tid: %x.%x", fs.id.task, fs.id.lthread);
  os2fs_get_drivemap_call(&fs, plogical, &env);
  //*plogical = 1 << (n - 1);
  io_printf("3");

  return 0; /* NO_ERROR */
}


APIRET CV
os2server_dos_QueryCurrentDir_component (CORBA_Object obj,
                                         ULONG disknum /* in */,
                                         char **pBuf /* out */,
                                         ULONG *pcbBuf /* out */,
                                         CORBA_srv_env *_srv_env)
{
  //CORBA_Environment env = default_env;
  ULONG disk, map;
  struct t_os2process *proc;
  //struct I_Fs_srv *fs_srv;
  char buf[0x100];
  char *curdir = buf;
  //char drv;
  //int i;

  io_printf("1");
  proc = PrcGetProcL4(*obj);
  curdir = proc->curdir;

  io_printf("2");
  if (!disknum)
    os2server_dos_QueryCurrentDisk_component(obj, &disk, &map, _srv_env);
  else
    disk = disknum;

  io_printf("3");

  if (!((1 << (disk - 1)) & map))
    return 15; /* ERROR_INVALID_DRIVE */

  io_printf("5");
  if (!*pcbBuf)
  {
    io_printf("6");
    *pcbBuf = strlen(curdir) + 1;
    return 0; /* NO_ERROR */
  }
  else
  {
    io_printf("7");
    strncpy(*pBuf, curdir, *pcbBuf);
    return 0; /* NO_ERROR */
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
  //CORBA_Environment env = default_env;
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
                                        CORBA_srv_env *_srv_env)
{
  CORBA_Environment env = default_env;
  struct t_os2process *proc;
  ULONG  map;

  io_printf("0");
  proc = PrcGetProcL4(*obj);

  // get drive map from fs server  
  os2fs_get_drivemap_call(&fs, &map, &env);

  io_printf("1");
  if (!((1 << (disknum - 1)) & map))
    return ERROR_INVALID_DRIVE;

  io_printf("map=%lx", map);
  io_printf("2");
  proc->curdisk = disknum;


  return NO_ERROR;
}

void CV
os2server_dos_Exit_component(CORBA_Object obj,
                             ULONG action, ULONG result,
                             CORBA_srv_env *_srv_env)
{
  unsigned long ppid;
  struct t_os2process *proc, *parentproc;  

  // get caller t_os2process structure
  proc = PrcGetProcL4(*obj);

  // kill calling thread; @todo: implement real thread termination!
  //if ((t = l4ts_kill_task(*obj, L4TS_KILL_SYNC)))
  //    io_printf("Error %d killing task\n", t);
  //else
  //    io_printf("task killed");

  io_printf("0");
  // get parent pid
  ppid = proc->lx_pib->pib_ulppid;
  io_printf("1");
  // get parent proc
  parentproc = PrcGetProc(ppid);
  if (!parentproc)
  {
    io_printf("parent proc is 0");
    return;
  }
  // unblock parent thread
  if (ppid || l4_thread_equal(parentproc->task, sysinit_id))
    l4semaphore_up(&parentproc->term_sem);
  io_printf("4");
  // destroy calling thread's proc
  PrcDestroy(proc);
  io_printf("6");

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

  io_printf("worker start");
  /* get caller t_os2process structure */
  proc = parm->proc;
 
  l = strlstlen(parm->pArg);
  io_printf("pArg len=%ld", (ULONG)l);
  io_printf("pEnv len=%ld", (ULONG)strlstlen(parm->pEnv));

  io_printf("pArg=%lx", (ULONG)parm->pArg);
  
  for (i = 0, p = parm->pArg; i < l; i++)
    if (p[i])
      io_printf("%c", p[i]);
    else
      io_printf("\\0");
 
  io_printf("begin exec");
  /* try executing the new task */
  rc =  PrcExecuteModule(parm->pObjname, parm->cbObjname, parm->execFlag,
                         parm->pArg, parm->pEnv, parm->pRes, parm->pName, proc->pid);
  io_printf("end exec");

  /* if child execution is synchronous
     and it is started successfully, 
     block until it terminates */
  io_printf("term wait");
  if (!rc && parm->execFlag == EXEC_SYNC)
    l4semaphore_down(&proc->term_sem);
  io_printf("done waiting");    
  /* notify the server loop to return API result */
  io_printf("0");
  io_printf("pRes=%lx", (ULONG)parm->pRes);
  io_printf("pObjname=%lx",  (ULONG)parm->pObjname);
  io_printf("cbObjname=%lx", (ULONG)parm->cbObjname);
  os2server_dos_ExecPgm_notify_call(&os2srv, &parm->thread, parm->pObjname, 
                                    parm->cbObjname, parm->pRes, rc, &env);
  io_printf("1");
  /* free our parameters structure */
  free(parm->pArg);
  free(parm->pEnv);
  free(parm);
  io_printf("worker terminate");
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
  io_printf("111");
  io_printf("pRes=%lx", (ULONG)pRes);
  io_printf("pObjname=%lx", (ULONG)pObjname);
  io_printf("cbObjname=%lx", (ULONG)cbObjname);
  io_printf("pRes->codeTerminate=%lx", (ULONG)pRes->codeTerminate);
  io_printf("pRes->codeResult=%lx", (ULONG)pRes->codeResult);
  os2server_dos_ExecPgm_reply ((l4_cap_idx_t *)job, result, (char **)&pObjname, 
			       (long *)&cbObjname, (struct _RESULTCODES *)pRes, _srv_env);
  io_printf("b");
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
os2server_dos_ScanEnv_component (CORBA_Object obj,
                                 PCSZ pszName /* in */,
                                 PSZ *ppszValue /* out */,
                                 CORBA_srv_env *_srv_env)
{
  struct t_os2process *proc;
  char varname[256];
  char *env;
  char *p, *q;
  int  i;

  /* Get the caller process structure   */
  proc = PrcGetProcL4(*obj);
  /* get application environment */
  env  = proc->lx_pib->pib_pchenv;  
  
  /* search for needed env variable */
  for (p = env; *p; p += strlen(p) + 1)
  {
    // move until '=' sign is encountered
    for (i = 0, q = p; *q && *q != '=' && i < 256; q++, i++) ;
    
    /* copy to name buffer  */
    strncpy(varname, p, i);
    /* add ending zero byte */
    varname[i] = '\0';

    if (!strcasecmp(varname, pszName))
    {
      /* variable found */
      strcpy(*ppszValue, q + 1);
      return 0; /* NO_ERROR */
    }
  }
  
  return 203; /* ERROR_ENVVAR_NOT_FOUND */
}
