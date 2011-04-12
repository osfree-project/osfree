/* API implementations
 * (on the server side)
 */
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_DOSMISC
#include <l4/os3/gcc_os2def.h>
#include <l4/os3/processmgr.h>
#include <l4/os3/globals.h>
#include <l4/os3/io.h>

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include <l4/sys/types.h>
#include <l4/sys/syscalls.h>
#include <l4/log/l4log.h>
#include <l4/util/stack.h>
#include <l4/semaphore/semaphore.h>
#include <l4/thread/thread.h>
#include <l4/generic_ts/generic_ts.h>
#include <l4/dm_mem/dm_mem.h>
#include <l4/l4rm/l4rm.h>
#include <l4/sys/kdebug.h>>

#include <dice/dice.h>

extern l4_threadid_t os2srv;
extern struct t_os2process *proc_root;

int
attach_ds(l4dm_dataspace_t *ds, l4_uint32_t flags, l4_addr_t *addr);


APIRET
os2server_dos_Read_component(CORBA_Object _dice_corba_obj,
                             HFILE hFile, void **pBuffer,
                             ULONG *pcbRead,
                             short *dice_reply,
                             CORBA_Server_Environment *_dice_corba_env)
{
  int  c;
  int  nread = 0;
  int  total = 1;

  nread = read(hFile, (char *)*pBuffer, *pcbRead);
  if (nread == -1)
  {
    LOG("read() error, errno=%d", errno);
    switch (errno)
    {
      // @todo: more accurate error handling
      default:
        return 232; //ERROR_NO_DATA
    }
  }
  *pcbRead = nread;


  return 0; // NO_ERROR
}

APIRET
os2server_dos_Write_component(CORBA_Object _dice_corba_obj,
                              HFILE hFile, PVOID pBuffer,
                              ULONG *pcbWrite,
                              short *dice_reply,
	 		      CORBA_Server_Environment *_dice_corba_env)
{
  char *s;
  int  nwritten;

  LOG("entered");
  nwritten = write(hFile, pBuffer, *pcbWrite);
  LOG("in the middle");
  if (nwritten == -1)
  {
    LOG("write() error, errno=%d", errno);
    switch (errno)
    {
      // @todo: more accurate error handling
      default:
        return 232; //ERROR_NO_DATA
    }
  }

  LOG("nwritten=%u", nwritten);
  *pcbWrite = nwritten;
  LOG("exited");
  return 0/*NO_ERROR*/;
}

APIRET
os2server_dos_Exit_component(CORBA_Object _dice_corba_obj,
                             ULONG action, ULONG result,
                             CORBA_Server_Environment *_dice_corba_env)
{
  int t, ret;
  unsigned long ppid;
  struct t_os2process *proc, *parentproc;  
  
  // get caller t_os2process structure
  proc = PrcGetProcL4(*_dice_corba_obj);

  // kill calling thread
  if ((t = l4ts_kill_task(*_dice_corba_obj, L4TS_KILL_SYNC)))
    LOG("Error %d killing task\n", t);
  else
    LOG("task killed");

  LOG("0");
  // get parent pid
  ppid = proc->lx_pib->pib_ulppid;
  
  LOG("1");
  if (ppid) // if not protshell (which has no parent)
  {
    // get parent proc
    LOG("2");
    parentproc = PrcGetProc(ppid);

    // unblock parent thread
    LOG("3");
    l4semaphore_up(&parentproc->term_sem);
    LOG("4");
  }
  
  // destroy calling thread's proc
  LOG("5");
  PrcDestroy(proc);
  LOG("6");

  return 0;
}


APIRET DICE_CV
os2server_dos_QueryCurrentDisk_component (CORBA_Object _dice_corba_obj,
                                          ULONG *pdisknum /* out */,
                                          ULONG *plogical /* out */,
                                          CORBA_Server_Environment *_dice_corba_env)
{
  ULONG n;
  struct t_os2process *proc;
  LOG("1");

  proc = PrcGetProcL4(*_dice_corba_obj);
  
  LOG("2");
  n = proc->curdisk;
  *pdisknum = n;
  *plogical = 1 << (n - 1);
  LOG("3");

  return 0; /* NO_ERROR */
}


APIRET DICE_CV
os2server_dos_QueryCurrentDir_component (CORBA_Object _dice_corba_obj,
                                         ULONG disknum /* in */,
                                         char **pBuf /* out */,
                                         ULONG *pcbBuf /* out */,
                                         CORBA_Server_Environment *_dice_corba_env)
{
  ULONG disk, map;
  struct t_os2process *proc;
  struct I_Fs_srv *fs_srv;
  char *curdir;
  char drv;
  int i;

  LOG("1");
  proc = PrcGetProcL4(*_dice_corba_obj);
  curdir = proc->curdir;
  
  LOG("2");
  if (!disknum)
    os2server_dos_QueryCurrentDisk_component(_dice_corba_obj, &disk, &map, _dice_corba_env);
  else
    disk = disknum;

  LOG("3");
  for (i = 0; i < fsrouter.srv_num_; i++)
  {
    fs_srv = fsrouter.fs_srv_arr_[i];
    drv = tolower(*(fs_srv->drive));
    if (disk == drv - 'a' + 1)
      break;
  }

  LOG("4");
  if (i == fsrouter.srv_num_)
     return 15; /* ERROR_INVALID_DRIVE */

  LOG("5");
  if (!*pcbBuf)
  {
    LOG("6");
    *pcbBuf = strlen(curdir) + 1;
    return 0; /* NO_ERROR */
  }
  else
  {
    LOG("7");
    strncpy(*pBuf, curdir, *pcbBuf);
    return 0; /* NO_ERROR */
  }

  return 0; /* NO_ERROR */
}

/* changes the current directory in '*dir'
   by one path component in 'component' */
void cdir(char **dir, char *component)
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
}

APIRET DICE_CV
os2server_dos_SetCurrentDir_component (CORBA_Object _dice_corba_obj,
                                       PSZ pszDir /* in */,
                                       CORBA_Server_Environment *_dice_corba_env)
{
  struct t_os2process *proc;
  char str[0x100];
  char *s;
  char *p, *q, *r;

  proc = PrcGetProcL4(*_dice_corba_obj);
  
  if (proc == NULL)
    return 303; /* ERROR_INVALID_PROCID */
  
  s = proc->curdir;

  if (pszDir == NULL)
  {
    *s = '\0';
    return 0; /* NO_ERROR */
  }

  p = q = pszDir;
  
  for (r = p; *r; r++)
    if (*r == '/')
      *r = '\\';

  if (!strcmp(pszDir, "\\"))
  {
    *s = '\0';
    return 0; /* NO_ERROR */
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
  
  return 0; /* NO_ERROR */
}

struct DosExecPgm_params {
  struct t_os2process *proc;
  l4_threadid_t thread;
  char **pObjname;
  long *cbObjname;
  unsigned long execFlag;
  char *pArg;
  char *pEnv;
  struct _RESULTCODES *pRes;
  char *pName;
};

#define EXEC_SYNC           0
#define EXEC_ASYNC          1
#define EXEC_ASYNCRESULT    2
#define EXEC_TRACE          3
#define EXEC_BACKGROUND     4
#define EXEC_LOAD           5
#define EXEC_ASYNCRESULTDB  6

/* DosExecPgm worker thread */
void
os2server_dos_ExecPgm_worker(struct DosExecPgm_params *parm)
{
  CORBA_Environment env = dice_default_environment;
  struct t_os2process *proc;
  APIRET rc;

  LOG("worker start");
  /* get caller t_os2process structure */
  proc = parm->proc;
 
  LOG("begin exec");
  /* try executing the new task */
  rc =  PrcExecuteModule(*(parm->pObjname), *(parm->cbObjname), parm->execFlag,
                         parm->pArg, parm->pEnv, parm->pRes, parm->pName, proc->pid);
  LOG("end exec");

  /* if child execution is synchronous
     and it is started successfully, 
     block until it terminates */
  LOG("term wait");
  if (!rc && parm->execFlag == EXEC_SYNC)
    l4semaphore_down(&proc->term_sem);
  LOG("done waiting");    
  /* notify the server loop to return API result */
  LOG("0");
  LOG("pRes=%x", parm->pRes);
  LOG("pObjname=%x",  *(parm->pObjname));
  LOG("cbObjname=%x", *(parm->cbObjname));
  os2server_dos_ExecPgm_notify_call(&os2srv, &parm->thread, *(parm->pObjname), 
                                    *(parm->cbObjname), parm->pRes, rc, &env);
  LOG("1");
  /* free our parameters structure */
  free(parm);
  LOG("worker terminate");
  /* terminate the worker thread */
  l4thread_exit();
}

/* notifier for main DosExecPgm component */
void DICE_CV
os2server_dos_ExecPgm_notify_component (CORBA_Object _dice_corba_obj,
                            const l4_threadid_t *job /* in */,
                            const char* pObjname /* in */,
                            int cbObjname /* in */,
                            const struct _RESULTCODES *pRes /* in */,
                            int result /* in */,
                            CORBA_Server_Environment *_dice_corba_env)
{
  LOG("111");
  LOG("pRes=%x", pRes);
  LOG("pObjname=%x", pObjname);
  LOG("cbObjname=%x", cbObjname);
  LOG("pRes->codeTerminate=%x", pRes->codeTerminate);
  LOG("pRes->codeResult=%x", pRes->codeResult);
  os2server_dos_ExecPgm_reply (job, result, &pObjname, &cbObjname, pRes, _dice_corba_env);
  LOG("b");
}

APIRET DICE_CV
os2server_dos_ExecPgm_component (CORBA_Object _dice_corba_obj,
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
                                 CORBA_Server_Environment *_dice_corba_env)
{
  struct t_os2process *proc;
  struct DosExecPgm_params *parm;
  l4_threadid_t thread;
  APIRET rc;
  int    ret;

  /* caller thread id */
  thread = *_dice_corba_obj;
  /* caller t_os2process structure */
  proc = PrcGetProcL4(thread);
  /* allocate parameters structure for worker thread */
  parm = (struct DosExecPgm_params *)malloc(sizeof(struct DosExecPgm_params));

  if (!parm)
    return 8; /* ERROR_NOT_ENOUGH_MEMORY */

  LOG("pRes=%x", pRes);
  LOG("pObjname=%x",  *pObjname);
  LOG("cbObjname=%x", *cbObjname);

  LOG("len of pArg=%d", strlstlen(pArg));

  /* fill in the params structure */
  parm->proc = proc;
  parm->thread = thread;
  parm->pObjname = pObjname;
  parm->cbObjname = cbObjname;
  parm->execFlag = execFlag;
  parm->pArg = pArg;
  parm->pEnv = pEnv;
  parm->pRes = pRes;
  parm->pName = pName;
  
  /* start DosExecPgm worker thread */
  l4thread_create(os2server_dos_ExecPgm_worker, (void *)parm, L4THREAD_CREATE_ASYNC);
  *dice_reply = DICE_NO_REPLY; // do not reply to the client until notification

  return 0;
}

APIRET DICE_CV
os2server_dos_Error_component (CORBA_Object _dice_corba_obj,
                               ULONG error /* in */,
                               CORBA_Server_Environment *_dice_corba_env)
{
  return 0; /* NO_ERROR */
}


APIRET DICE_CV
os2server_dos_SetDefaultDisk_component (CORBA_Object _dice_corba_obj,
                                        ULONG disknum /* in */,
                                        CORBA_Server_Environment *_dice_corba_env)
{
  struct t_os2process *proc;
  struct I_Fs_srv *fs_srv;
  char   drv;
  int    i;

  proc = PrcGetProcL4(*_dice_corba_obj);

  for (i = 0; i < fsrouter.srv_num_; i++)
  {
    fs_srv = fsrouter.fs_srv_arr_[i];
    drv = tolower(*(fs_srv->drive));
    if (disknum == drv - 'a' + 1)
      break;
  }

  if (i == fsrouter.srv_num_)
     return 15; /* ERROR_INVALID_DRIVE */

  proc->curdisk = disknum;

  return 0; /* NO_ERROR */
}

APIRET DICE_CV
os2server_dos_ResetBuffer_component (CORBA_Object _dice_corba_obj,
                                     HFILE handle /* in */,
                                     CORBA_Server_Environment *_dice_corba_env)
{
  return 0; /* NO_ERROR */
}

#define FILE_BEGIN   0
#define FILE_CURRENT 1
#define FILE_END     2

APIRET DICE_CV
os2server_dos_SetFilePtr_component (CORBA_Object _dice_corba_obj,
                                    HFILE handle /* in */,
                                    long ib /* in */,
                                    ULONG method /* in */,
                                    ULONG *ibActual /* out */,
                                    CORBA_Server_Environment *_dice_corba_env)
{
  long pos, ret;
  unsigned long len;
  struct stat stat;
  
  ret = fstat(handle, &stat);

  if (ret == -1)
    return 6; /* ERROR_INVALID_HANDLE */

  /* get length */
  len = stat.st_size;
  /* get current position */
  pos = lseek(handle, 0L, SEEK_CUR);

  switch (method)
  {
  case FILE_BEGIN:
    pos = ib;
    break;
  case FILE_CURRENT:
    pos += ib;
    break;
  case FILE_END:
    pos = len + ib;
    break;
  default:
    pos = ib;
  }

  if (pos < 0)
    return 131; /* ERROR_NEGATIVE_SEEK */

  *ibActual = pos;
  ret = lseek(handle, pos, SEEK_SET);

  if (ret == -1)
    return 132; /* ERROR_SEEK_ON_DEVICE */

  return 0; /* NO_ERROR */
}

APIRET DICE_CV
os2server_dos_Close_component (CORBA_Object _dice_corba_obj,
                               HFILE handle /* in */,
                               CORBA_Server_Environment *_dice_corba_env)
{
  int ret;
  
  ret = close(handle);
  
  if (ret == - 1)
  {
    if (errno == EBADF)
      return 6; /* ERROR_INVALID_HANDLE */
      
    return 5; /* ERROR_ACCESS_DENIED */
  }
  
  return 0; /* NO_ERROR */
}


APIRET DICE_CV
os2server_dos_QueryHType_component (CORBA_Object _dice_corba_obj,
                                    HFILE handle /* in */,
                                    ULONG *pType /* out */,
                                    ULONG *pAttr /* out */,
                                    CORBA_Server_Environment *_dice_corba_env)
{
  int ret;
  unsigned short m;
  struct stat stat;
  
  ret = fstat(handle, &stat);

  if (ret == -1)
    return 6; /* ERROR_INVALID_HANDLE */

  m = stat.st_mode;

  if (S_ISREG(m))
  {
    *pType = 0; // disk file
    *pAttr = 0;
    return 0;
  }

  if (S_ISCHR(m))
  {
    *pType = 1; // character device
    *pAttr = 0;
    return 0;
  }

  if (S_ISFIFO(m))
  {
    *pType = 2; // pipe
    *pAttr = 0;
    return 0;
  }
  
  return 0; /* NO_ERROR */
}


APIRET DICE_CV
os2server_dos_QueryDBCSEnv_component (CORBA_Object _dice_corba_obj,
                                      ULONG *cb /* in, out */,
                                      COUNTRYCODE *pcc /* out */,
                                      const char **pBuf /* in */,
                                      CORBA_Server_Environment *_dice_corba_env)
{
  if (cb)
    memset(*pBuf, 0, *cb); // empty
    
  return 0; /* NO_ERROR */
}


APIRET DICE_CV
os2server_dos_QueryCp_component (CORBA_Object _dice_corba_obj,
                                 ULONG cb /* in */,
                                 ULONG *arCP /* out */,
                                 ULONG *pcCP /* out */,
                                 CORBA_Server_Environment *_dice_corba_env)
{
  if (cb < 3 * sizeof(ULONG))
    return 473; /* ERROR_CPLIST_TOO_SMALL */

  arCP[0] = 437; /* current codepage   */
  arCP[1] = 437; /* primary codepage   */
  arCP[2] = 850; /* secondary codepage */
  *pcCP = 3 * sizeof(ULONG);
  
  return 0; /* NO_ERROR */
}

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

APIRET DICE_CV
os2server_dos_GetInfoBlocks_component (CORBA_Object _dice_corba_obj,
                                       l4dm_dataspace_t *ds /* out */,
                                       l4_offs_t *tib_offs /* out */,
                                       l4_offs_t *pib_offs /* out */,
                                       CORBA_Server_Environment *_dice_corba_env)
{
  struct t_os2process *proc;
  l4_addr_t addr;
  l4_size_t size;

  PTIB  ptib, pt;
  PTIB2 ptib2, pt2;
  PPIB  ppib, pp;
  char *s1, *s2;

  int   len, rc;
  
  /* get the caller proc structure */
  proc  = PrcGetProcL4(*_dice_corba_obj);

  /* info blocks */
  ppib  = proc->lx_pib;
  ptib  = proc->main_tib; // @todo: fix for non-main threads!
  ptib2 = proc->main_tib->tib_ptib2;
  
  /* total size of all info */
  size = sizeof(PIB) + sizeof(TIB) + sizeof(TIB2) + 
         strlen(ppib->pib_pchcmd) + 1 + strlen(ppib->pib_pchenv) + 1; 

  /* allocate a dataspace of a given size */	 
  rc = l4dm_mem_open(L4DM_DEFAULT_DSM, size, L4_PAGESIZE, 0, "OS/2 app info blocks", ds);

  if (rc)
    return 8; /* ERROR_NOT_ENOUGH_MEMORY */
    
  /* attach it to our address space */
  rc = attach_ds(ds, L4DM_RW, &addr);
  
  if (rc)
  {
    LOG("cannot attach ds");
    return 5; /* ERROR_ACCESS_DENIED */  
  }
  
  pt = (PTIB)(addr);
  memmove(pt, ptib, sizeof(TIB));
  pt2 = (PTIB2)((char *)addr + sizeof(TIB));
  memmove(pt2, ptib2, sizeof(TIB2));
  pp = (PPIB)((char *)pt2 + sizeof(TIB2));
  memmove(pp, ppib, sizeof(PIB));

  /* copy argv and envp */
  s1 = (char *)pp + sizeof(PIB);
  len = strlstcpy(s1, ppib->pib_pchcmd);
  s2 = s1 + len;
  len = strlstcpy(s2, ppib->pib_pchenv);

  /* fixup addresses -- make them addr-based */
  s1 -= addr;
  s2 -= addr;
  pp->pib_pchcmd = s1;
  pp->pib_pchenv = s2;
  pt2 = (char *)pt2 - addr;
  pt->tib_ptib2 = pt2;
  pt  = (char *)pt - addr;
  pp  = (char *)pp - addr; 

  /* share the dataspace with client */
  rc = l4dm_share(ds, *_dice_corba_obj, L4DM_RW);

  if (rc)
  {
    LOG("cannot share ds");
    return 5; /* ERROR_ACCESS_DENIED */
  }
  
  /* set offsets */
  *tib_offs = 0;
  *pib_offs = pp;

  /* detach the dataspace */
  l4rm_detach(addr);
  
  return 0; /* NO_ERROR */
}
