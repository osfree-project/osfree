/*
 *  API implementations
 *  parts (on the client side)
 *  These functions are those which are
 *  exported from the KAL.DLL virtual library.
 */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/dataspace.h>
#include <os3/thread.h>
#include <os3/err.h>
#include <os3/rm.h>
#include <os3/ipc.h>
#include <os3/io.h>
#include <os3/handlemgr.h>
#include <os3/stacksw.h>
#include <os3/kal.h>
#include <os3/exec.h>
#include <os3/cpi.h>
#include <os3/fs.h>
#include <os3/app.h>

/* L4 includes */
//#include <l4/generic_ts/generic_ts.h> // l4ts_exit
#include <l4/semaphore/semaphore.h>
#include <l4/dm_phys/dm_phys.h>
#include <l4/lock/lock.h>
#include <l4/sys/syscalls.h>
#include <l4/sys/segment.h>
#include <l4/sys/types.h>

/* libc includes*/
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>

/* servers RPC includes */
#include <l4/os2srv/os2server-client.h>
#include <l4/os2fs/os2fs-client.h>
#include <l4/os2exec/os2exec-client.h>

char LOG_tag[9];

/* Last used thread id */
static ULONG ulThread = 1;

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

/* FS server cap        */
extern l4_threadid_t fs;
/* OS/2 server cap      */
extern l4_threadid_t os2srv;
/* Exec server cap      */
extern l4_threadid_t execsrv;

/* private memory arena settings */
extern l4_addr_t     private_memory_base;
extern l4_size_t     private_memory_size;
extern unsigned long long private_memory_area;

/* shared memory arena settings */
extern l4_addr_t     shared_memory_base;
extern l4_size_t     shared_memory_size;
extern unsigned long long shared_memory_area;

/* server loop thread of os2app   */
extern l4_uint32_t   service_lthread;

/* old FS selector value          */
unsigned short old_sel;
/* TIB new FS selector value      */
unsigned short tib_sel;
/* Stack pointer value            */
extern unsigned long __stack;
/* Max number of file descriptors */
static ULONG CurMaxFH = 20;
/* Thread Info Block pointer      */
extern PTIB ptib[MAX_TID];
/* Process Info Block pointer     */
extern PPIB ppib;

vmdata_t *areas_list = NULL;

struct start_data;

vmdata_t *get_area(void *addr);
vmdata_t *get_mem_by_name(char *pszName);

int commit_pages(PVOID pb,
                 ULONG cb,
                 l4_uint32_t rights);

int decommit_pages(PVOID pb,
                   ULONG cb,
                   l4_uint32_t rights);

void exit_func(l4thread_t tid, void *data);
static void thread_func(void *data);

/* The following two routines are needed because of
   collision between Fiasco.OC and OS/2: Fiasco.OC
   stores the UTCB selector in fs, and OS/2 stores
   TIB selector there. So, a workaround: save/restore
   these selectors when entering/exiting to/from
   L4 world / OS/2 world. */

void KalEnter(void)
{
  STKIN

  /* Transition from OS/2 world to L4 world:
     save TIB selector to tib_sel and restore
     host kernel FS selector from old_sel */
  asm ("movw %[old_sel], %%dx \n"
       "movw %%dx, %%fs \n"
       :
       :[old_sel]  "m"  (old_sel));
}

void KalQuit(void)
{
  /* Transition from L4 world to OS/2 world:
     save an old FS selector to old_sel and restore
     TIB selector in FS from tib_sel     */
  asm ("movw %[tib_sel], %%dx \n"
       "movw %%dx, %%fs \n"
       :
       :[tib_sel]  "m"  (tib_sel));

  STKOUT
}

APIRET CDECL
KalOpenL (PSZ pszFileName,
          HFILE *phFile,
          ULONG *pulAction,
          LONGLONG cbFile,
          ULONG ulAttribute,
          ULONG fsOpenFlags,
          ULONG fsOpenMode,
          PEAOP2 peaop2)
{
  //CORBA_Environment env = dice_default_environment;
  EAOP2 eaop2;
  APIRET  rc;

  KalEnter();

  memset(&eaop2, 0, sizeof(EAOP2));

  if (peaop2 == NULL)
    peaop2 = &eaop2;

    /* Support for current/working directory for filenames.
    No support yet for devicenames, COM1 etc. That needs to be done
    inside os2server and the added current directory in this function needs to
    be separated from the filename in pszFileName
    ( add one more argument to os2fs_dos_OpenL_call for current dir). */
  unsigned char drive = '\0';
  char *path = "";

  int BUFLEN = 99;
  char *filbuf = (char *)calloc(1, BUFLEN + 1);

  ULONG dsknum=0, plog=0, dirbuf_len=100, dirbuf_len2=100, dirbuf_len_out=100;
  PBYTE dir_buf=calloc(1,dirbuf_len),
         dir_buf2=calloc(1,dirbuf_len2), dir_buf_out=calloc(1,dirbuf_len_out);
  BYTE /*drive=0,*/ drive2=0;

  rc = KalQueryCurrentDisk(&dsknum, &plog); /*For c:\os2 it becomes 3 (drive letter)*/
                     /*  3     "os2" */
  if (rc)
  {
    KalQuit();
    return rc;
  }

  rc = KalQueryCurrentDir(dsknum, (PBYTE)dir_buf, &dirbuf_len);

  if (rc)
  {
    KalQuit();
    return rc;
  }

  /*char *pszFileName = fil12; */


  drive = parse_drv(pszFileName);  if(drive==0)  drive2 = disknum_to_char(dsknum);
  path = parse_path(pszFileName, filbuf, BUFLEN);

  BYTE drv = 0;
  if(drive==0) {  /* Is actual device specified? */
      drv = char_to_disknum(drive2);
  }else {
      drv = char_to_disknum(drive);
  }

  /* Working directory is put into dir_buf2. */
  rc = KalQueryCurrentDir(drv, (PBYTE)dir_buf2, &dirbuf_len2);

  if (rc)
  {
    KalQuit();
    return rc;
  }

  char s[10] = "";  /*Create device letter with colon. */
  s[0] = disknum_to_char(drv);
  s[1] = 0;
  strncat((char*)dir_buf_out, s, 1);

  /*  .\config.sys               ':' */
  if(isRelativePath(path) ) {/* Add working directory. */

      strncat((char*)dir_buf_out, ":\\", 2);
      strncat((char*)dir_buf_out, (char*)dir_buf2, dirbuf_len2);
      if((strcmp((char*)dir_buf2,"")!=0) && isRelativePath((char*)dir_buf2))
          strncat((char*)dir_buf_out, "\\", 1);
      strncat((char*)dir_buf_out, path, strlen(path));
      /* Complete path for filename is now inside dir_buf_out.*/
  }

  /*  \config.sys               'c:' */
  if((! isRelativePath(path)) ) {/* Add working directory from specified disk*/

      strncat((char*)dir_buf_out, ":", 2);
      /* Is string in working directory? dir_buf2*/
      /*Is working directory going to be added?*/
      if( (isRelativePath((char*)path)) ) { 
          if((strcmp((char*)dir_buf2,"")!=0) && isRelativePath((char*)dir_buf2)) {
              strncat((char*)dir_buf_out, "\\", 1);
              strncat((char*)dir_buf_out, 
                          (char*)dir_buf2, dirbuf_len2);/*No working directory!*/
              if(isRelativePath((char*)path))
                  strncat((char*)dir_buf_out, "\\", 1);
          }
      }
      strncat((char*)dir_buf_out, path, strlen(path));
      /* Complete path for filename is now inside dir_buf_out.*/
  }

  /********************************************/
                                /* pszFileName */
  //rc = os2fs_dos_OpenL_call (&fs, dir_buf_out, phFile,
    //                  pulAction, cbFile, ulAttribute,
    //                  fsOpenFlags, fsOpenMode, peaop2, &env);
  rc = FSClientOpenL(dir_buf_out, phFile,
                     pulAction, cbFile, ulAttribute,
                     fsOpenFlags, fsOpenMode, peaop2);
  KalQuit();
  return rc;
}

APIRET CDECL
KalFSCtl (PVOID pData,
          ULONG cbData,
          PULONG pcbData,
          PVOID pParms,
          ULONG cbParms,
          PULONG pcbParms,
          ULONG function,
          PSZ pszRoute,
          HFILE hFile,
          ULONG method)
{
  APIRET  rc = NO_ERROR;
  KalEnter();
  // ...
  KalQuit();
  return rc;
}


APIRET CDECL
KalRead (HFILE hFile, PVOID pBuffer,
            ULONG cbRead, PULONG pcbActual)
{
  //CORBA_Environment env = dice_default_environment;
  int nread = 0;
  APIRET rc = NO_ERROR;

  KalEnter();

  if (! cbRead)
  {
    KalQuit();
    return NO_ERROR;
  }

  nread = read(hFile, pBuffer, cbRead);
  if (nread == -1)
  {
    io_log("read() error, errno=%d\n", errno);
    switch (errno)
    {
      // @todo: more accurate error handling
      default:
        return ERROR_NO_DATA;
    }
  }
  *pcbActual = nread;

  // strange, if I remove this, the command line refuses to work!
  //io_log("test\n");

  KalQuit();

  return rc;
}


APIRET CDECL
KalWrite (HFILE hFile, PVOID pBuffer,
          ULONG cbWrite, PULONG pcbActual)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc = NO_ERROR;
  int nwritten = 0;

  KalEnter();

  io_log("000\n");

  if (! cbWrite)
  {
    KalQuit();
    return NO_ERROR;
  }

  io_log("001\n");
  nwritten = write(hFile, pBuffer, cbWrite);
  io_log("002\n");
  if (nwritten == -1)
  {
    io_log("write() error, errno=%d\n", errno);
    switch (errno)
    {
      // @todo: more accurate error handling
      default:
        return ERROR_NO_DATA;
    }
  }
  io_log("003\n");
  *pcbActual = nwritten;
  io_log("004\n");

  KalQuit();

  return rc;
}

APIRET CDECL
KalLogWrite (PSZ s)
{
  KalEnter();
  io_log("%s\n", s);
  KalQuit();
  return NO_ERROR;
}

VOID CDECL
KalExit(ULONG action, ULONG result)
{
  //CORBA_Environment env = dice_default_environment;
  PID pid;
  TID tid;
  KalEnter();

  // get thread pid
  KalGetPID(&pid);

  // get current thread id
  KalGetTID(&tid);

  switch (action)
  {
    case EXIT_PROCESS:
      // send OS/2 server a message that we want to terminate
      //os2server_dos_Exit_send(&os2srv, action, result, &env);
      CPClientExit(action, result);
      // tell L4 task server that we want to terminate
      TaskExit();
    default:
      if (tid == 1)
      {
        // last thread of this task: terminate task
        //os2server_dos_Exit_send(&os2srv, action, result, &env);
        CPClientExit(action, result);
        // tell L4 task server that we want to terminate
        TaskExit();
      }
      else
      {
        // free thread TIB
        KalDestroyTIB(pid, tid);
        // terminate thread
        ThreadExit();
      }
  }
  KalQuit();
}


APIRET CDECL
KalQueryCurrentDisk(PULONG pdisknum,
                        PULONG plogical)
{
  //CORBA_Environment env = dice_default_environment;
  int rc;
  KalEnter();
  //rc = os2fs_get_drivemap_call(&fs, plogical, &env);
  rc = FSClientGetDriveMap(plogical);
  io_log("KalQueryCurrentDisk: get_drivemap rc=%lu\n", rc);

  if (rc)
  {
    KalQuit();
    return rc;
  }

  //rc = os2server_dos_QueryCurrentDisk_call(&os2srv, pdisknum, &env);
  rc = CPClientQueryCurrentDisk(pdisknum);
  KalQuit();
  return rc;
}

APIRET CDECL
KalSetCurrentDir(PSZ pszDir)
{
  //CORBA_Environment env = dice_default_environment;
  int rc;
  KalEnter();
  //rc = os2server_dos_SetCurrentDir_call(&os2srv, pszDir, &env);
  rc = CPClientSetCurrentDir(pszDir);
  KalQuit();
  return rc;
}

APIRET CDECL
KalSetDefaultDisk(ULONG disknum)
{
  //CORBA_Environment env = dice_default_environment;
  int rc;
  ULONG map;
  KalEnter();
  //rc = os2fs_get_drivemap_call(&fs, &map, &env);
  rc = FSClientGetDriveMap(&map);
  io_log("KalSetDefaultDisk: get_drivemap rc=%lu\n", rc);

  if (rc)
  {
    KalQuit();
    return rc;
  }

  //rc = os2server_dos_SetDefaultDisk_call(&os2srv, disknum, map, &env);
  rc = CPClientSetDefaultDisk(disknum, map);
  KalQuit();
  return rc;
}

APIRET CDECL
KalQueryCurrentDir(ULONG disknum,
                       PBYTE pBuf,
                       PULONG pcbBuf)
{
  //CORBA_Environment env = dice_default_environment;
  long rc = 0;
  ULONG map;
  KalEnter();

  if (! pcbBuf || ! pBuf)
  {
    io_log("***bad***\n");
    KalQuit();
    return ERROR_INVALID_PARAMETER;
  }

  //rc = os2fs_get_drivemap_call(&fs, &map, &env);
  rc = FSClientGetDriveMap(&map);
  io_log("KalQueryCurrentDir: get_drivemap rc=%lu\n", rc);

  if (rc)
  {
    KalQuit();
    return rc;
  }

  //rc = os2server_dos_QueryCurrentDir_call(&os2srv, disknum, map, &pBuf, pcbBuf, &env);
  rc = CPClientQueryCurrentDir(disknum, map, &pBuf, pcbBuf);
  io_log("dos_QueryCurrentDir rc=%lu\n", rc);

  if (rc)
  {
    KalQuit();
    return rc;
  }

  io_log("KalQUeryCurrentDir: pcbBuf=%p\n", pcbBuf);

  if (pcbBuf)
    io_log("KalQueryCurrentDir: *pcbBuf=%p\n", *pcbBuf);

  pBuf[*pcbBuf - 1] = '\0';
  KalQuit();
  return NO_ERROR;
}


APIRET CDECL
KalQueryProcAddr(ULONG hmod,
                     ULONG ordinal,
                     const PSZ  pszName,
                     void  **ppfn)
{
  //CORBA_Environment env = dice_default_environment;
  int rc;
  KalEnter();
  //rc = os2exec_query_procaddr_call(&execsrv, hmod, ordinal,
    //                               pszName, (l4_addr_t *)ppfn, &env);
  rc = ExcClientQueryProcAddr(hmod, ordinal,
                              pszName, ppfn);
  KalQuit();
  return rc;
}

APIRET CDECL
KalQueryModuleHandle(const char *pszModname,
                     unsigned long *phmod)
{
  //CORBA_Environment env = dice_default_environment;
  int rc;
  KalEnter();
  //rc = os2exec_query_modhandle_call(&execsrv, pszModname,
    //                                phmod, &env);
  rc = ExcClientQueryModuleHandle((char *)pszModname, phmod);
  KalQuit();
  return rc;
}

APIRET CDECL
KalQueryModuleName(unsigned long hmod, unsigned long cbBuf, char *pBuf)
{
  //CORBA_Environment env = dice_default_environment;
  int rc;
  KalEnter();
  //rc = os2exec_query_modname_call(&execsrv, hmod,
    //                              cbBuf, &pBuf, &env);
  rc = ExcClientQueryModuleName(hmod, cbBuf, pBuf);
  KalQuit();
  return rc;
}

#if 0

/** attach dataspace to our address space. (any free address) */
long attach_ds(l4_os3_dataspace_t ds, unsigned long flags, void **addr)
{
  int error;
  l4_size_t size;

  if ((error = l4dm_mem_size(ds, &size)))
    {
      io_log("Error %d (%s) getting size of dataspace\n",
             error, l4os3_errtostr(error));
      return error;
    }

  if ((error = l4rm_attach(ds, size, 0, flags, (void **)addr)))
    {
      io_log("Error %d (%s) attaching dataspace\n",
             error, l4os3_errtostr(error));
      return error;
    }
  return 0;
}

/** attach dataspace to our address space. (concrete address) */
long attach_ds_reg(l4_os3_dataspace_t ds, unsigned long flags, void *addr)
{
  int error;
  l4_size_t size;
  l4_addr_t a = addr;

  /* get dataspace size */
  if ((error = l4dm_mem_size(&ds, &size)))
    {
      io_log("Error %d (%s) getting size of dataspace\n",
             error, l4os3_errtostr(error));
      return error;
    }

  /* attach it to a given region */  
  if ((error = l4rm_attach_to_region(&ds, (void *)a, size, 0, flags)))
    {
      io_log("Error %d (%s) attaching dataspace\n",
             error, l4os3_errtostr(error));
      return error;
    }

  return 0;
}

/** attach dataspace to our address space. (concrete address) */
long attach_ds_area(l4_os3_dataspace_t ds, unsigned long area, unsigned long flags, void *addr)
{
  int error;
  l4_size_t size;
  l4_addr_t a = addr;

  /* get dataspace size */
  if ((error = l4dm_mem_size(&ds, &size)))
    {
      io_log("Error %d (%s) getting size of dataspace\n",
             error, l4os3_errtostr(error));
      return error;
    }

  /* attach it to a given region */
  if ( (error = l4rm_area_attach_to_region(&ds, area,
                       (void *)a, size, 0, flags)) )
    {
      io_log("Error %d (%s) attaching dataspace\n",
             error, l4os3_errtostr(error));
      return error;
    }

  return 0;
}

#endif

/*  Attaches all sections
 *  for a given module
 */
long attach_module (ULONG hmod, unsigned long long area)
{
  //CORBA_Environment env = dice_default_environment;
  l4_os3_section_t sect;
  l4_os3_dataspace_t ds;
  l4_os3_dataspace_t area_ds;
  ULONG flags;
  void        *addr;
  void        *map_addr;
  ULONG       map_size;
  ULONG       offset;
  //l4_threadid_t pager;
  unsigned type;
  unsigned long index;
  ULONG rc;

  index = 0; rc = 0;
  //while (! os2exec_getsect_call (&execsrv, hmod, &index, &sect, &env) && !rc)
  while (! ExcClientGetSect (hmod, &index, &sect) && ! rc)
  {
    ds    = sect.ds;
    addr  = sect.addr;
    type  = sect.type;
    flags = 0;

    if (type & SECTYPE_READ)
      flags |= L4DM_READ;

    if (type & SECTYPE_WRITE)
      flags |= L4DM_WRITE;

    //if ( (rc = l4rm_lookup(addr, &map_addr, &map_size,
    //                &area_ds, &offset, &pager)) != L4RM_REGION_DATASPACE)
    if (! (rc = RegLookupRegion(addr, &map_addr, &map_size, &offset,
                    &area_ds)) )
    {
      rc = attach_ds_area (ds, area, flags, addr);
      if (! rc)
        io_log("attached\n");
      else if (rc != -L4_EUSED)
      {
        io_log("attach_ds_area returned %d\n", rc);
        break;
      }
    }
    else
    {
      //io_log("map_addr=%x, map_size=%u, area_ds=%x\n",
        //  map_addr, map_size, area_ds);
      break;
    }
  }

  return 0;
}


/*  Attaches recursively a module and
 *  all its dependencies
 */
long attach_all (ULONG hmod, unsigned long long area)
{
  //CORBA_Environment env = dice_default_environment;
  ULONG imp_hmod, rc = 0;
  unsigned long index = 0;

  rc = attach_module(hmod, area);

  if (rc)
    return rc;

  //while (! os2exec_getimp_call (&execsrv, hmod, &index, &imp_hmod, &env) && !rc)
  while (! ExcClientGetImp(hmod, &index, &imp_hmod) && ! rc)
  {
    if (! imp_hmod) // KAL fake module: no need to attach sections
      continue;

    rc = attach_all(imp_hmod, shared_memory_area);
  }

  return rc;
}

unsigned long
KalPvtLoadModule(char *pszName,
                 unsigned long *pcbName,
                 char const *pszModname,
                 os2exec_module_t *s,
                 PHMODULE phmod)
{
  HMODULE hmod;
  ULONGLONG area;
  APIRET rc;

  rc = ExcClientOpen((char *)pszModname, 1, pszName, pcbName, &hmod);

  if (rc)
    return rc;

  *phmod = hmod;

  rc = ExcClientLoad(hmod, pszName, pcbName, s);

  if (rc)
    return rc;

  if (s->exeflag) // store .exe sections in default (private) area
    area = private_memory_area;
  else // and .dll sections in shared area
    area = shared_memory_area;

  rc = ExcClientShare(hmod);

  if (rc)
    return rc;

  rc = attach_all(hmod, area);

  if (rc)
    return rc;

  return 0;
}


APIRET CDECL
KalLoadModule(PSZ pszName,
                  ULONG cbName,
                  char const *pszModname,
                  PULONG phmod)
{
  os2exec_module_t s;
  int rc;
  KalEnter();
  rc = KalPvtLoadModule(pszName, &cbName, pszModname,
                       &s, phmod);
  KalQuit();
  return rc;
}

#define PT_16BIT 0
#define PT_32BIT 1

APIRET CDECL
KalQueryProcType(HMODULE hmod,
                 ULONG ordinal,
                 PSZ pszName,
                 PULONG pulProcType)
{
  void *pfn;
  APIRET rc;

  KalEnter();
  rc = KalQueryProcAddr(hmod, ordinal, pszName, &pfn);
  if (rc) return rc;
  if (pfn)
    *pulProcType = PT_32BIT;
  KalQuit();
  return NO_ERROR;
}


APIRET CDECL
KalQueryAppType(PSZ pszName,
                PULONG pFlags)
{
  APIRET rc = NO_ERROR;

  KalEnter();
  // ...
  KalQuit();
  return rc;
}


APIRET CDECL
KalExecPgm(char *pObjname,
           long cbObjname,
           unsigned long execFlag,
           char *pArg,
           char *pEnv,
           struct _RESULTCODES *pRes,
           char *pName)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;
  ULONG disk, map;
  char buf[260];
  char str[260];
  //char str2[260];
  //char *p;
  char drv;
  int i, j, l, len;

  KalEnter();

  if (execFlag > 6)
    return ERROR_INVALID_FUNCTION;

  /* if no path specified, add the current dir */
  if (pName[1] != ':')
  {
    /* query current disk */
    rc = KalQueryCurrentDisk(&disk, &map);

    if (rc)
    {
      KalQuit();
      return rc;
    }

    drv = disk - 1 + 'A';

    len = 0; buf[0] = '\0';
    if (pName[0] != '\\')
    {
      /* query current dir  */
      rc = KalQueryCurrentDir(0, buf, (PULONG)&len);

      if (rc)
      {
        KalQuit();
        return rc;
      }

      rc = KalQueryCurrentDir(0, buf, (PULONG)&len);

      if (rc)
      {
        KalQuit();
        return rc;
      }
    }

    if (len + strlen(pName) + 3 > 256)
      return ERROR_FILENAME_EXCED_RANGE;

    i = 0;
    str[i++] = drv;
    str[i++] = ':';
    str[i] = '\0';

    if (pName[0] != '\\') 
    {
      str[i++] = '\\';
      str[i] = '\0';
      strcat(str, buf);

      if (str[len + i - 2] != '\\') 
      {
        str[len + i - 1] = '\\';
        len++;
      }
      str[len + i - 1] = '\0';
    }

    pName = strcat(str, pName);

    if (! strstr(pName, ".exe"))
      strcat(pName, ".exe");
  }

  if (pArg == NULL)
  {
    pArg = "\0\0";
    i = 2;
  }
  else
    i = strlstlen(pArg);

  if (pEnv == NULL)
    pEnv = ppib->pib_pchenv;

  j = strlstlen(pEnv);
  l = strlstlen(pArg);

  //rc =  os2server_dos_ExecPgm_call (&os2srv, &pObjname,
    //                    &cbObjname, execFlag, pArg, i,
    //                    pEnv, j,
    //                    pRes, pName, &env);
  rc =  CPClientExecPgm(&pObjname,
                        &cbObjname, execFlag, pArg, i,
                        pEnv, j,
                        pRes, pName);

  io_log("=== rc=%lu\n", rc);
  KalQuit();
  return rc;
}

APIRET CDECL
KalError(ULONG error)
{
  //CORBA_Environment env = dice_default_environment;
  int rc;
  KalEnter();
  //rc = os2server_dos_Error_call (&os2srv, error, &env);
  rc = CPClientError(error);
  KalQuit();
  return rc;
}

vmdata_t *get_area(void *addr)
{
  vmdata_t *ptr;

  for (ptr = areas_list; ptr; ptr = ptr->next)
  {
    if (ptr->addr <= addr && addr <= ptr->addr + ptr->size)
      break;
  }

  return ptr;
}

vmdata_t *get_mem_by_name(char *pszName)
{
  vmdata_t *ptr;

  for (ptr = areas_list; ptr; ptr = ptr->next)
  {
    if (ptr->name[0] && ! strcmp(ptr->name, pszName))
      break;
  }

  return ptr;
}

APIRET CDECL
KalAllocMem(PVOID *ppb,
            ULONG cb,
            ULONG flags)
{
  ULONG rights = 0;
  ULONGLONG area;
  l4_os3_dataspace_t ds;
  void *addr;
  vmdata_t  *ptr;
  int rc;

  KalEnter();

  if (flags & PAG_READ)
    rights |= L4DM_READ;

  if (flags & PAG_WRITE)
    rights |= L4DM_WRITE;

  if (flags & PAG_EXECUTE)
    rights |= L4DM_READ;

  rc = l4rm_area_reserve(cb, 0, (l4_addr_t *)&addr, (l4_uint32_t *)&area);

  if (rc < 0)
  {
    KalQuit();

    switch (-rc)
    {
      case L4_ENOMEM:
      case L4_ENOTFOUND:
        return ERROR_NOT_ENOUGH_MEMORY;
      default:
        return ERROR_ACCESS_DENIED;
    }
  }

  ptr = (vmdata_t *)malloc(sizeof(vmdata_t));

  ptr->is_shared = 0;
  ptr->owner.thread  = l4_myself();
  ptr->rights = (l4_uint32_t)flags;
  ptr->area   = area;
  ptr->name[0] = '\0';
  ptr->addr   = addr;
  ptr->size   = cb;
  if (areas_list) areas_list->prev = ptr;
  ptr->next   = areas_list;
  ptr->prev   = 0;
  areas_list  = ptr;

  if (flags & PAG_COMMIT)
  {
    /* Create a dataspace of a given size */
    //rc = l4dm_mem_open(L4DM_DEFAULT_DSM, cb,
    //           4096, rights, "DosAllocMem dataspace", &ds);
    rc = DataspaceAlloc(&ds, rights, DEFAULT_DSM, cb);

    if (rc < 0)
    {
      KalQuit();
      return 8; /* ERROR_NOT_ENOUGH_MEMORY */
    }

    /* attach the created dataspace to our address space */
    rc = attach_ds_area(ds, area, rights, addr);

    if (rc)
    {
      KalQuit();
      return 8; /* What to return? */
    }
  }

  *ppb = (void *)addr;

  KalQuit();
  return 0; /* NO_ERROR */
}

APIRET CDECL
KalFreeMem(PVOID pb)
{
  //CORBA_Environment env = dice_default_environment;
  vmdata_t *ptr;
  void *addr;
  l4_size_t size;
  l4_offs_t offset;
  unsigned long refcnt = 0;
  l4_os3_thread_t owner;
  l4_threadid_t pager;
  l4_os3_dataspace_t ds;
  int rc, ret;

  KalEnter();

  ptr = get_area(pb);

  if (! ptr)
    return ERROR_INVALID_ADDRESS;

  addr = ptr->addr;

  if (ptr->is_shared)
  {
    // release area at os2exec
    //os2exec_release_sharemem_call(&execsrv, ptr->addr, &refcnt, &env);
    ExcClientReleaseSharedMem(ptr->addr, &refcnt);
  }

  // detach and release all dataspaces in ptr->area
  while (ptr->addr <= addr && addr <= ptr->addr + ptr->size)
  {
    ret = l4rm_lookup_region(addr, (l4_addr_t *)&addr, &size, &ds.ds,
                             &offset, &pager);

    if (ret < 0)
      return ERROR_INVALID_ADDRESS;

    switch (ret)
    {
      case L4RM_REGION_RESERVED:
      case L4RM_REGION_FREE:
        break;
      case L4RM_REGION_DATASPACE:
        if (ptr->is_shared)
          // unmap dataspace from os2exec address space
          //os2exec_unmap_dataspace_call(&execsrv, addr, &ds, &env);
          ExcClientUnmapDataspace(addr, ds);
        // unmap from local address space
        l4rm_detach(addr);
        break;
      default:
        KalQuit();
        return ERROR_INVALID_ADDRESS;
    }

    if (ret == L4RM_REGION_DATASPACE)
    {
      if (ptr->is_shared)
      {
        if (! refcnt)
        {
          owner = ptr->owner;
          if (! l4_thread_equal(owner.thread, l4_myself()))
            // ask owner to delete dataspace
            //os2app_app_ReleaseDataspace_call(&owner, &ds, &env);
            AppClientDataspaceRelease(owner, ds);
          else
            l4dm_close(&ds.ds);
        }
      }
      else
        // delete myself
        l4dm_close(&ds.ds);

      rc = NO_ERROR;
    }

    addr += size;
  }

  if (! refcnt)
  {
    rc = l4rm_area_release_addr(ptr->addr);

    if (rc < 0)
    {
      KalQuit();
      return ERROR_ACCESS_DENIED;
    }

    if (ptr->prev)
      ptr->prev->next = ptr->next;

    if (ptr->next)
      ptr->next->prev = ptr->prev;

    free(ptr);
  }

  KalQuit();
  return 0; /* NO_ERROR */
}

/* commit cb bytes starting from pb address */
int commit_pages(PVOID pb,
                 ULONG cb,
                 l4_uint32_t rights)
{
  //CORBA_Environment env = dice_default_environment;
  l4_addr_t addr;
  l4_size_t size;
  l4_offs_t offset;
  l4_threadid_t pager;
  l4_os3_dataspace_t ds;
  l4dm_dataspace_t temp_ds;
  vmdata_t *ptr;
  int rc;

  ptr = get_area(pb);

  if (! ptr)
    return ERROR_INVALID_ADDRESS;

  for (;;)
  {
    rc = l4rm_lookup_region(pb, &addr, &size, &temp_ds,
                            &offset, &pager);

    if (rc < 0)
      return ERROR_INVALID_ADDRESS;

    switch (rc)
    {
      case L4RM_REGION_RESERVED:
        //rc = l4dm_mem_open(L4DM_DEFAULT_DSM, cb, 4096,
        //                   rights, "DosAllocMem dataspace", &ds);
        rc = DataspaceAlloc(&ds, rights, DEFAULT_DSM, cb);

        if (rc < 0)
          return ERROR_NOT_ENOUGH_MEMORY;

        rc = attach_ds_area(ds, ptr->area, rights, pb);

        if (rc < 0)
          return ERROR_NOT_ENOUGH_MEMORY;

        if (ptr->is_shared)
          //os2exec_map_dataspace_call(&execsrv, pb, rights, &ds, &env);
          ExcClientMapDataspace(pb, rights, ds);

        break;

      default:
        ;
    }

    if ((l4_addr_t)pb + cb <= addr + size)
      break;

    cb -= addr + size - (l4_addr_t)pb;
    pb += addr + size - (l4_addr_t)pb;
  }

  return NO_ERROR;
}

/* decommit cb bytes starting from pb address */
int decommit_pages(PVOID pb,
                   ULONG cb,
                   l4_uint32_t rights)
{
  //CORBA_Environment env = dice_default_environment;
  void *addr;
  l4_size_t size;
  l4_offs_t offset;
  l4_threadid_t pager;
  l4_os3_dataspace_t ds, ds1, ds2;
  vmdata_t *ptr;
  int rc;

  ptr = get_area(pb);

  if (! ptr)
    return ERROR_INVALID_ADDRESS;

  for (;;)
  {
    rc = l4rm_lookup_region(pb, (l4_addr_t *)&addr, &size, &ds.ds,
                            &offset, &pager);

    if (rc < 0)
      return ERROR_INVALID_ADDRESS;

    switch (rc)
    {
      case L4RM_REGION_DATASPACE:
        // detach dataspace first
        l4rm_detach(addr);
        if (ptr->is_shared)
          // unmap from os2exec address space
          //os2exec_unmap_dataspace_call(&execsrv, addr, &ds, &env);
          ExcClientUnmapDataspace(addr, ds);

        if (pb > addr)
        {
          // copy dataspace before hole
          rc = l4dm_memphys_copy(&ds.ds, 0, 0, (l4_addr_t)(pb - addr),
                                 L4DM_MEMPHYS_DEFAULT, L4DM_MEMPHYS_ANY_ADDR,
                                 (l4_addr_t)(pb - addr), 4096, 0,
                                 "DosAllocMem dataspace", &ds1.ds);

          if (rc < 0)
          {
            KalQuit();
            return 8; /* What to return? */
          }

          // attach dataspace part before hole
          rc = l4rm_area_attach_to_region(&ds1.ds, ptr->area, addr,
                                          (l4_addr_t)(pb - addr), 0, rights);

          if (rc < 0)
          {
            KalQuit();
            return 8; /* What to return? */
          }

          if (ptr->is_shared)
            //os2exec_map_dataspace_call(&execsrv, addr, rights, &ds1, &env);
            ExcClientMapDataspace(addr, rights, ds1);
        }

        if (pb + cb < addr + size)
        {
          // copy dataspace after hole
          rc = l4dm_memphys_copy(&ds.ds, (l4_addr_t)(pb + cb - addr), 0, (l4_addr_t)(addr + size - pb - cb),
                                 L4DM_MEMPHYS_DEFAULT, L4DM_MEMPHYS_ANY_ADDR,
                                 (l4_addr_t)(addr + size - pb - cb), 4096, 0,
                                 "DosAllocMem dataspace", &ds2.ds);

          if (rc < 0)
          {
            KalQuit();
            return 8; /* What to return? */
          }

          // attach dataspace part after hole
          rc = l4rm_area_attach_to_region(&ds2.ds, ptr->area, pb + cb,
                                          (l4_addr_t)(addr + size - pb - cb), 0, rights);

          if (rc < 0)
          {
            KalQuit();
            return 8; /* What to return? */
          }

          if (ptr->is_shared)
            //os2exec_map_dataspace_call(&execsrv, (l4_addr_t)pb + cb, rights, &ds2, &env);
            ExcClientMapDataspace(pb + cb, rights, ds2);
        }

        l4dm_close(&ds.ds);
        break;

      default:
        ;
    }

    if (pb + cb <= addr + size)
      break;

    cb -= addr + size - pb;
    pb += addr + size - pb;
  }

  return NO_ERROR;
}

APIRET CDECL
KalSetMem(PVOID pb,
          ULONG cb,
          ULONG flags)
{
  vmdata_t *ptr;
  l4_uint32_t rights = 0;
  int rc = 0;

  KalEnter();

  ptr = get_area(pb);

  if (! ptr)
    return ERROR_INVALID_ADDRESS;

  if (flags & PAG_DEFAULT)
    flags |= ptr->rights;

  if (flags & PAG_READ)
    rights |= L4DM_READ;

  if (flags & PAG_WRITE)
    rights |= L4DM_WRITE;

  if (flags & PAG_EXECUTE)
    rights |= L4DM_READ;

  if ( !(flags & PAG_DECOMMIT) && !(flags & PAG_DEFAULT) &&
       !(flags & PAG_READ) && !(flags & PAG_WRITE) && !(flags & PAG_EXECUTE) )
    return ERROR_INVALID_PARAMETER;

  if ( (flags & PAG_COMMIT) && (flags & PAG_DECOMMIT) )
    return ERROR_INVALID_PARAMETER;

  if (flags & PAG_COMMIT)
  {
    ptr->rights |= PAG_COMMIT;
    rc = commit_pages(pb, cb, rights);
  }
  else if (flags & PAG_DECOMMIT)
  {
    ptr->rights &= ~PAG_COMMIT;
    rc = decommit_pages(pb, cb, rights);
  }

  // @todo Implement PAG_GUARD (need exceptions implementation)

  KalQuit();
  return rc;
}

APIRET CDECL
KalQueryMem(PVOID  pb,
            PULONG pcb,
            PULONG pflags)
{
  vmdata_t *ptr;
  l4_uint32_t rights = 0;
  void *addr;
  l4_size_t size = 0;
  l4_offs_t offset;
  l4_threadid_t pager;
  l4dm_dataspace_t ds;
  int rc = 0, ret;
  void *base;
  ULONG totsize = 0;

  KalEnter();

  if (! pcb || ! pflags)
    return ERROR_INVALID_PARAMETER;

  ptr = get_area(pb);

  if (! ptr)
    return ERROR_INVALID_ADDRESS;

  base = pb;

  do
  {
    ret = l4rm_lookup_region(base, (l4_addr_t *)&addr, &size, &ds,
                             &offset, &pager);

    switch (ret)
    {
      case L4RM_REGION_DATASPACE:
        rights  = ptr->rights;
        if (addr + size <= pb + *pcb)
          totsize += size;
        break;
      case L4RM_REGION_RESERVED:
        break;
      case L4RM_REGION_FREE:
        rights = PAG_FREE;
        break;
      default:
        KalQuit();
        return ERROR_INVALID_ADDRESS;
    }

    base = addr + size;
  } while (base < pb + *pcb);

  if (pb - ptr->addr <= L4_PAGESIZE)
    rights |= PAG_BASE;

  *pcb = totsize;
  *pflags = rights;

  KalQuit();
  return rc;
}

APIRET CDECL
KalAllocSharedMem(PPVOID ppb,
                  PSZ    pszName,
                  ULONG  cb,
                  ULONG  flags)
{
  //CORBA_Environment env = dice_default_environment;
  l4_uint32_t rights = 0;
  ULONGLONG area = shared_memory_area;
  l4_os3_dataspace_t ds;
  void *addr;
  vmdata_t  *ptr;
  char *p;
  char name[] = "";
  int rc = 0, ret;

  KalEnter();

  if (! ppb)
    return ERROR_INVALID_PARAMETER;

  if (pszName)
  {
    // uppercase pszName
    for (p = pszName; *p; p++)
      *p = toupper(*p);

    if (strstr(pszName, "\\SHAREMEM\\") != pszName)
      return ERROR_INVALID_NAME;

    if (get_mem_by_name(pszName))
      return ERROR_ALREADY_EXISTS;
  }
  else
    pszName = name;

  if (flags & PAG_READ)
    rights |= L4DM_READ;

  if (flags & PAG_WRITE)
    rights |= L4DM_WRITE;

  if (flags & PAG_EXECUTE)
    rights |= L4DM_READ;

  // reserve area on os2exec and attach data to it (user pointer)
  //rc = os2exec_alloc_sharemem_call (&execsrv, cb, pszName, flags, &addr, &area, &env);
  rc = ExcClientAllocSharedMem (cb, pszName, flags, &addr, &area);

  if (rc)
  {
    KalQuit();
    return ERROR_NOT_ENOUGH_MEMORY;
  }

  // reserve the same area in local region mapper
  area = shared_memory_area;
  rc = l4rm_area_reserve_region_in_area((l4_addr_t)addr, cb, 0, (l4_uint32_t *)&area);

  if (rc)
  {
    KalQuit();
    return ERROR_NOT_ENOUGH_MEMORY;
  }

  ptr = (vmdata_t *)malloc(sizeof(vmdata_t));

  if (! ptr)
  {
    KalQuit();
    return ERROR_NOT_ENOUGH_MEMORY;
  }

  ptr->is_shared = 1;
  ptr->owner.thread = l4_myself();
  ptr->area = area;
  ptr->rights = flags;
  ptr->addr = addr;
  ptr->size = cb;
  if (pszName) strcpy(ptr->name, pszName);
  if (areas_list) areas_list->prev = ptr;
  ptr->next = areas_list;
  ptr->prev = 0;
  areas_list = ptr;

  if (flags & PAG_COMMIT)
  {
    /* Create a dataspace of a given size */
    //rc = l4dm_mem_open(L4DM_DEFAULT_DSM, cb,
    //           4096, rights, "DosAllocSharedMem dataspace", &ds);
    rc = DataspaceAlloc(&ds, rights, DEFAULT_DSM, cb);

    if (rc < 0)
    {
      KalQuit();
      return 8; /* ERROR_NOT_ENOUGH_MEMORY */
    }

    /* attach the created dataspace to our address space */
    rc = attach_ds_area(ds, area, rights, addr);

    if (rc)
    {
      KalQuit();
      return 8; /* What to return? */
    }

    // map dataspace to os2exec address space
    if ( (ret = l4dm_share(&ds.ds, execsrv, rights)) < 0)
    {
      switch (-ret)
      {
        case L4_EINVAL: return ERROR_FILE_NOT_FOUND;
        case L4_EPERM:  return ERROR_ACCESS_DENIED;
        default:        return ERROR_INVALID_PARAMETER;
      }
    }

    //rc = os2exec_map_dataspace_call(&execsrv, addr, rights, &ds, &env);
    rc = ExcClientMapDataspace(addr, rights, ds);
  }

  if (! rc)
  {
    *ppb = addr;
  }

  KalQuit();
  return rc;
}

APIRET CDECL
KalGetSharedMem(PVOID pb,
                ULONG flag)
{
  //CORBA_Environment env = dice_default_environment;
  l4_os3_dataspace_t ds;
  void *addr, *a;
  unsigned long size, sz;
  ULONGLONG area = shared_memory_area;
  l4_uint32_t rights = 0;
  l4_os3_cap_idx_t owner;
  vmdata_t *ptr;
  int ret, rc = 0;

  KalEnter();

  if (flag & PAG_READ)
    rights |= L4DM_READ;

  if (flag & PAG_WRITE)
    rights |= L4DM_WRITE;

  if (flag & PAG_EXECUTE)
    rights |= L4DM_READ;

  //rc = os2exec_get_sharemem_call (&execsrv, pb, &addr, &size, &owner, &env);
  rc = ExcClientGetSharedMem (pb, &addr, &size, &owner);

  if (rc)
  {
    KalQuit();
    return ERROR_FILE_NOT_FOUND;
  }

  if ( (ptr = get_area(addr)) )
    ptr->rights |= flag;
  else
  {
    // reserve the same area in local region mapper
    rc = l4rm_area_reserve_in_area(size, 0, (l4_addr_t *)&addr, (l4_uint32_t *)&area);

    if (rc)
    {
      KalQuit();
      return ERROR_NOT_ENOUGH_MEMORY;
    }

    ptr = (vmdata_t *)malloc(sizeof(vmdata_t));

    if (! ptr)
    {
      KalQuit();
      return ERROR_NOT_ENOUGH_MEMORY;
    }

    ptr->area = area;
    ptr->is_shared = 1;
    ptr->owner = owner;
    ptr->rights = flag;
    ptr->addr = addr;
    ptr->size = size;
    ptr->name[0] = '\0';
    if (areas_list) areas_list->prev = ptr;
    ptr->next = areas_list;
    ptr->prev = 0;
    areas_list = ptr;
  }

  // get all dataspaces attached between addr and addr + size
  // from os2exec and attech them to the same regions of local address space
  a = addr;
  while (addr <= a && a <= addr + size)
  {
    // get dataspace from os2exec
    //if ( !(ret = os2exec_get_dataspace_call(&execsrv, &a, &sz, ds, &env)) )
    if (! (ret = ExcClientGetDataspace(&a, &sz, &ds)) )
      // attach it to our address space
      ret = attach_ds_area(ds, area, rights, a);

    a = a + sz;
  }

  KalQuit();
  return rc;
}

APIRET CDECL
KalGetNamedSharedMem(PPVOID ppb,
                     PSZ pszName,
                     ULONG flag)
{
  //CORBA_Environment env = dice_default_environment;
  l4_os3_dataspace_t ds;
  void *addr, *a;
  unsigned long size, sz;
  ULONGLONG area = shared_memory_area;
  l4_uint32_t rights = 0;
  l4_os3_cap_idx_t owner;
  vmdata_t *ptr;
  char *p;
  int ret, rc = 0;
  int ds_cnt  = 0;

  KalEnter();

  if (! ppb || !pszName)
    return ERROR_INVALID_PARAMETER;

  // uppercase pszName
  for (p = pszName; *p; p++)
    *p = toupper(*p);

  if (strstr(pszName, "\\SHAREMEM\\") != pszName)
    return ERROR_INVALID_NAME;

  if (flag & PAG_READ)
    rights |= L4DM_READ;

  if (flag & PAG_WRITE)
    rights |= L4DM_WRITE;

  if (flag & PAG_EXECUTE)
    rights |= L4DM_READ;

  //rc = os2exec_get_namedsharemem_call (&execsrv, pszName, &addr, &size, &owner, &env);
  rc = ExcClientGetNamedSharedMem (pszName, &addr, &size, &owner);

  if (rc)
  {
    KalQuit();
    return ERROR_FILE_NOT_FOUND;
  }

  if ( (ptr = get_area(addr)) )
    ptr->rights |= flag;
  else
  {
    // reserve the same area in local region mapper
    rc = l4rm_area_reserve_region_in_area((l4_addr_t)addr, size, 0, (l4_uint32_t *)&area);

    if (rc)
    {
      KalQuit();
      return ERROR_NOT_ENOUGH_MEMORY;
    }

    ptr = (vmdata_t *)malloc(sizeof(vmdata_t));

    if (! ptr)
    {
      KalQuit();
      return ERROR_NOT_ENOUGH_MEMORY;
    }

    ptr->area = area;
    ptr->is_shared = 1;
    ptr->owner = owner;
    ptr->rights = flag;
    ptr->addr = addr;
    ptr->size = size;
    if (pszName) strcpy(ptr->name, pszName);
    if (areas_list) areas_list->prev = ptr;
    ptr->next = areas_list;
    ptr->prev = 0;
    areas_list = ptr;
  }

  // get all dataspaces attached between addr and addr + size
  // from os2exec and attech them to the same regions of local address space
  a = addr;
  while (addr <= a && a <= addr + size)
  {
    // get dataspace from os2exec
    //if ( !(ret = os2exec_get_dataspace_call(&execsrv, &a, &sz, ds, &env)) )
    if (! (ret = ExcClientGetDataspace(&a, &sz, &ds)) )
    {
      // attach it to our address space
      if ( !(ret = attach_ds_area(ds, area, rights, a)) )
        ds_cnt++;
    }
    else
      // no dataspace
      break;

    a = a + sz;
  }

  if (ds_cnt)
    *ppb = addr;
  else
    rc = ERROR_FILE_NOT_FOUND;

  KalQuit();
  return rc;
}

APIRET CDECL
KalGiveSharedMem(PVOID pb,
                 PID pid,
                 ULONG flag)
{
  //CORBA_Environment env = dice_default_environment;
  int rc, ret;
  //l4thread_t id;
  l4_os3_thread_t tid;
  l4_uint32_t rights = 0;
  void *addr;
  l4_size_t size;
  l4_offs_t offset;
  l4_os3_dataspace_t ds;
  l4_threadid_t pager;
  vmdata_t *ptr;

  KalEnter();
  if (! pb || ! pid)
  {
    KalQuit();
    return ERROR_INVALID_PARAMETER;
  }

  KalGetNativeID(pid, 1, &tid);
  tid.thread.id.lthread = service_lthread;

  if (l4_thread_equal(tid.thread, L4_INVALID_ID))
  {
    KalQuit();
    return ERROR_INVALID_PROCID;
  }

  if (flag & PAG_READ)
    rights |= L4DM_READ;

  if (flag & PAG_WRITE)
    rights |= L4DM_WRITE;

  if (flag & PAG_EXECUTE)
    rights |= L4DM_READ;

  if ( !(ptr = get_area(pb)) )
  {
    KalQuit();
    return ERROR_INVALID_ADDRESS;
  }

  ptr->rights |= flag;

  //rc = os2app_app_AddArea_call(&tid, ptr->addr, ptr->size, flag, &env);
  rc = AppClientAreaAdd(tid, ptr->addr, ptr->size, (ULONG)flag);

  if (rc)
    return rc;

  addr = ptr->addr;
  while (ptr->addr <= addr && addr <= ptr->addr + ptr->size)
  {
    ret = l4rm_lookup_region(addr, (l4_addr_t *)&addr, &size, &ds.ds,
                            &offset, &pager);

    if (ret == L4RM_REGION_DATASPACE)
    {
      // transfer dataspace to a given process
      l4dm_share(&ds.ds, tid.thread, rights);
      //os2exec_increment_sharemem_refcnt_call(&execsrv, addr, &env);
      ExcClientIncrementSharedMemRefcnt(addr);
      // say that process to map dataspace to a given address
      //rc = os2app_app_AttachDataspace_call(&tid, addr, &ds, rights, &env);
      rc = AppClientDataspaceAttach(tid, addr, ds, (ULONG)rights);
    }

    addr += size;
  }

  KalQuit();
  return rc;
}

APIRET CDECL
KalResetBuffer(HFILE handle)
{
  //CORBA_Environment env = dice_default_environment;
  int rc;
  KalEnter();
  //rc = os2fs_dos_ResetBuffer_call (&fs, handle, &env);
  rc = FSClientResetBuffer(handle);
  KalQuit();
  return rc;
}

APIRET CDECL
KalSetFilePtrL(HFILE handle,
               LONGLONG ib,
               ULONG method,
               PULONGLONG ibActual)
{
  //CORBA_Environment env = dice_default_environment;
  int rc;
  KalEnter();
  //rc = os2fs_dos_SetFilePtrL_call (&fs, handle, ib,
    //                              method, ibActual, &env);
  rc = FSClientSetFilePtrL(handle, ib,
                           method, ibActual);
  KalQuit();
  return rc;
}

APIRET CDECL
KalClose(HFILE handle)
{
  //CORBA_Environment env = dice_default_environment;
  int rc;
  KalEnter();
  //rc = os2fs_dos_Close_call (&fs, handle, &env);
  rc = FSClientClose(handle);
  KalQuit();
  return rc;
}

APIRET CDECL
KalQueryHType(HFILE handle,
              PULONG pType,
              PULONG pAttr)
{
  //CORBA_Environment env = dice_default_environment;
  int rc;
  KalEnter();
  //rc = os2fs_dos_QueryHType_call(&fs, handle, pType, pAttr, &env);
  rc = FSClientQueryHType(handle, pType, pAttr);
  KalQuit();
  return rc;
}

APIRET CDECL
KalQueryDBCSEnv(ULONG cb,
                COUNTRYCODE *pcc,
                PBYTE pBuf)
{
  //CORBA_Environment env = dice_default_environment;
  int rc;
  KalEnter();
  //rc = os2server_dos_QueryDBCSEnv_call (&os2srv, &cb, pcc, &pBuf, &env);
  rc = CPClientQueryDBCSEnv(&cb, pcc, &pBuf);
  KalQuit();
  return rc;
}

APIRET CDECL
KalQueryCp(ULONG cb,
           PULONG arCP,
           PULONG pcCP)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;
  KalEnter();
  //rc = os2server_dos_QueryCp_call (&os2srv, &cb, (char **)&arCP, &env);
  rc = CPClientQueryCp(&cb, (char **)&arCP);
  *pcCP = cb;
  KalQuit();
  return rc;
}

APIRET CDECL
KalSetMaxFH(ULONG cFH)
{
  CurMaxFH = cFH;
  return NO_ERROR;
}

APIRET CDECL
KalSetRelMaxFH(PLONG pcbReqCount, PULONG pcbCurMaxFH)
{
  CurMaxFH += *pcbReqCount;
  *pcbCurMaxFH = CurMaxFH;
  return NO_ERROR;
}

APIRET CDECL
KalSleep(ULONG ms)
{
  KalEnter();
  l4_sleep(ms);
  KalQuit();

  return NO_ERROR;
}

APIRET CDECL
KalDupHandle(HFILE hFile, HFILE *phFile2)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2fs_dos_DupHandle_call(&fs, hFile, phFile2, &env);
  rc = FSClientDupHandle(hFile, phFile2);
  KalQuit();
  return rc; /* NO_ERROR */
}

APIRET CDECL
KalDelete(PSZ pszFileName)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2fs_dos_Delete_call (&fs, pszFileName, &env);
  rc = FSClientDelete(pszFileName);
  KalQuit();
  return rc; /* NO_ERROR */
}

APIRET CDECL
KalForceDelete(PSZ pszFileName)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2fs_dos_ForceDelete_call (&fs, pszFileName, &env);
  rc = FSClientForceDelete(pszFileName);
  KalQuit();
  return rc; /* NO_ERROR */
}

APIRET CDECL
KalDeleteDir(PSZ pszDirName)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2fs_dos_DeleteDir_call (&fs, pszDirName, &env);
  rc = FSClientDeleteDir(pszDirName);
  KalQuit();
  return rc; /* NO_ERROR */
}

APIRET CDECL
KalCreateDir(PSZ pszDirName, PEAOP2 peaop2)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2fs_dos_CreateDir_call (&fs, pszDirName, peaop2, &env);
  rc = FSClientCreateDir(pszDirName, peaop2);
  KalQuit();
  return rc; /* NO_ERROR */
}


APIRET CDECL
KalFindFirst(char  *pszFileSpec,
             HDIR  *phDir,
             ULONG flAttribute,
             PVOID pFindBuf,
             ULONG cbBuf,
             ULONG *pcFileNames,
             ULONG ulInfolevel)
{
  //CORBA_Environment env = dice_default_environment;
  char  buf[256];
  char  str[256];
  ULONG disk, map;
  char  drv;
  int   len = 0, i;
  char  *s;
  APIRET rc;

  KalEnter();

  /* if no path specified, add the current dir */
  if (pszFileSpec[1] != ':')
  {
    /* query current disk */
    rc = KalQueryCurrentDisk(&disk, &map);
    drv = disk - 1 + 'A';

    len = 0; buf[0] = '\0';
    if (pszFileSpec[0] != '\\')
    {
      /* query current dir  */
      rc = KalQueryCurrentDir(0, buf, (PULONG)&len);

      if (rc)
      {
        KalQuit();
        return rc;
      }

      rc = KalQueryCurrentDir(0, buf, (PULONG)&len);

      if (rc)
      {
        KalQuit();
        return rc;
      }
    }

    if (len + strlen(pszFileSpec) + 3 > 256)
      return ERROR_FILENAME_EXCED_RANGE;

    i = 0;
    str[i++] = drv;
    str[i++] = ':';
    str[i] = '\0';

    if (pszFileSpec[0] != '\\') 
    {
      str[i++] = '\\';
      str[i] = '\0';
      strcat(str, buf);

      if (str[len + i - 2] != '\\') 
      {
        str[len + i - 1] = '\\';
        len++;
      }
      str[len + i - 1] = '\0';
    }

    s = strcat(str, pszFileSpec);
  }
  else
    s = pszFileSpec;

  //rc = os2fs_dos_FindFirst_call(&fs, s, phDir,
    //                            flAttribute, (char **)&pFindBuf, &cbBuf,
    //                            pcFileNames, ulInfolevel, &env);
  rc = FSClientFindFirst(s, phDir,
                         flAttribute, pFindBuf, &cbBuf,
                         pcFileNames, ulInfolevel);
  KalQuit();
  return rc;
}


APIRET CDECL
KalFindNext(HDIR  hDir,
            PVOID pFindBuf,
            ULONG cbBuf,
            ULONG *pcFileNames)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2fs_dos_FindNext_call(&fs, hDir, (char **)&pFindBuf,
    //                           &cbBuf, pcFileNames, &env);
  rc = FSClientFindNext(hDir, pFindBuf,
                        &cbBuf, pcFileNames);
  KalQuit();
  return rc;
}


APIRET CDECL
KalFindClose(HDIR hDir)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2fs_dos_FindClose_call(&fs, hDir, &env);
  rc = FSClientFindClose(hDir);
  KalQuit();
  return rc;
}


APIRET CDECL
KalQueryFHState(HFILE hFile,
                PULONG pMode)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2fs_dos_QueryFHState_call(&fs, hFile, pMode, &env);
  rc = FSClientQueryFHState(hFile, pMode);
  KalQuit();
  return rc;
}


APIRET CDECL
KalSetFHState(HFILE hFile,
              ULONG pMode)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2fs_dos_SetFHState_call(&fs, hFile, pMode, &env);
  rc = FSClientSetFHState(hFile, pMode);
  KalQuit();
  return rc;
}

APIRET CDECL
KalQueryFileInfo(HFILE hf,
                 ULONG ulInfoLevel,
                 char *pInfo,
                 ULONG cbInfoBuf)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2fs_dos_QueryFileInfo_call(&fs, hf, ulInfoLevel,
    //                                &pInfo, &cbInfoBuf, &env);
  rc = FSClientQueryFileInfo(hf, ulInfoLevel,
                             pInfo, &cbInfoBuf);
  KalQuit();
  return rc;
}


APIRET CDECL
KalQueryPathInfo(PSZ pszPathName,
                 ULONG ulInfoLevel,
                 PVOID pInfo,
                 ULONG cbInfoBuf)
{
  //CORBA_Environment env = dice_default_environment;
  char   buf[CCHMAXPATH];
  char   str[CCHMAXPATH];
  ULONG  disk, map;
  ULONG  len;
  char   drv;
  int    i;
  APIRET rc;

  KalEnter();

  if (pszPathName[1] != ':')
  {
    /* query current disk */
    rc = KalQueryCurrentDisk(&disk, &map);
    drv = disk - 1 + 'A';

    len = 0; buf[0] = '\0';
    if (pszPathName[0] != '\\')
    {
      /* query current dir  */
      rc = KalQueryCurrentDir(0, buf, (PULONG)&len);

      if (rc)
      {
        KalQuit();
        return rc;
      }

      rc = KalQueryCurrentDir(0, buf, (PULONG)&len);

      if (rc)
      {
        KalQuit();
        return rc;
      }
    }

    if (len + strlen(pszPathName) + 3 > 256)
      return ERROR_FILENAME_EXCED_RANGE;

    i = 0;
    str[i++] = drv;
    str[i++] = ':';
    str[i] = '\0';

    if (pszPathName[0] != '\\')
    {
      str[i++] = '\\';
      str[i] = '\0';
      strcat(str, buf);

      if (str[len + i - 2] != '\\') 
      {
        str[len + i - 1] = '\\';
        len++;
      }
      str[len + i - 1] = '\0';
    }

    pszPathName = strcat(str, pszPathName);
  }

  //rc = os2fs_dos_QueryPathInfo_call(&fs, pszPathName, ulInfoLevel,
    //                                (char **)&pInfo, &cbInfoBuf, &env);
  rc = FSClientQueryPathInfo(pszPathName, ulInfoLevel,
                             pInfo, &cbInfoBuf);
  KalQuit();
  return rc;
}


APIRET CDECL
KalSetFileSizeL(HFILE hFile,
                long long cbSize)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2fs_dos_SetFileSizeL_call(&fs, hFile, cbSize, &env);
  rc = FSClientSetFileSizeL(hFile, cbSize);
  KalQuit();
  return rc;
}

APIRET CDECL
KalMove(PSZ pszOld, PSZ pszNew)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  // return an error while it is unimplemented
  rc = ERROR_INVALID_PARAMETER;
  KalQuit();
  return rc;
}

APIRET CDECL
KalOpenEventSem(PSZ pszName,
                PHEV phev)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2server_dos_OpenEventSem_call(&os2srv, pszName, phev, &env);
  rc = CPClientOpenEventSem(pszName, phev);
  KalQuit();
  return rc;
}

APIRET CDECL
KalCloseEventSem(HEV hev)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2server_dos_CloseEventSem_call(&os2srv, hev, &env);
  rc = CPClientCloseEventSem(hev);
  KalQuit();
  return rc;
}

APIRET CDECL
KalCreateEventSem(PSZ pszName,
                  PHEV phev,
                  ULONG flags,
                  BOOL32 fState)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2server_dos_CreateEventSem_call(&os2srv, pszName, phev, flags, fState, &env);
  rc = CPClientCreateEventSem(pszName, phev, flags, fState);
  KalQuit();
  return rc;
}

void exit_func(l4thread_t tid, void *data)
{
  l4_threadid_t t = l4thread_l4_id(l4thread_get_parent());
  l4_msgdope_t dope;

  // notify parent about our termination
  l4_ipc_send(t, (void *)(L4_IPC_SHORT_MSG | L4_IPC_DECEIT_MASK),
              tid, 0, L4_IPC_SEND_TIMEOUT_0, &dope);
}
L4THREAD_EXIT_FN_STATIC(fn, exit_func);

struct start_data
{
  PFNTHREAD pfn;
  ULONG param;
};

static void thread_func(void *data)
{
  struct start_data *start_data = (struct start_data *)data;
  PFNTHREAD pfn = start_data->pfn;
  ULONG param   = start_data->param;
  //l4thread_t        id;
  l4_os3_thread_t   thread;
  unsigned long     base;
  unsigned short    sel;
  struct desc       desc;
  PID pid;
  TID tid;

  //register exit func
  if ( l4thread_on_exit(&fn, NULL) < 0 )
    io_log("error setting the exit function!\n");
  else
    io_log("exit function set successfully\n");

  // get current process id
  KalGetPID(&pid);
  // get current thread id
  KalGetTID(&tid);
  // get l4thread thread id
  KalGetNativeID(pid, tid, &thread);

  /* TIB base */
  base = (unsigned long)ptib[tid - 1];

  /* Prepare TIB GDT descriptor */
  desc.limit_lo = 0x30; desc.limit_hi = 0;
  desc.acc_lo   = 0xF3; desc.acc_hi   = 0;
  desc.base_lo1 = base & 0xffff;
  desc.base_lo2 = (base >> 16) & 0xff;
  desc.base_hi  = base >> 24;

  /* Allocate a GDT descriptor */
  fiasco_gdt_set(&desc, sizeof(struct desc), 0, thread.thread);

  /* Get a selector */
  sel = (sizeof(struct desc)) * fiasco_gdt_get_entry_offset();

  // set fs register to TIB selector
  //enter_kdebug("debug");
  asm(
      "movw  %[sel], %%dx \n"
      "movw  %%dx, %%fs \n"
      :
      :[sel]  "m"  (sel));

  // execute OS/2 thread function
  (*pfn)(param);
}

static void wait_func(void)
{
  l4_threadid_t src;
  l4_umword_t dw1, dw2;
  l4_msgdope_t dope;

  for (;;)
  {
    if ( l4_ipc_wait(&src, L4_IPC_SHORT_MSG,
                      &dw1, &dw2, L4_IPC_NEVER,
                      &dope) < 0 )
    {
      io_log("IPC error\n");
    }
  }
}

APIRET CDECL
KalCreateThread(PTID ptid,
                PFNTHREAD pfn,
                ULONG param,
                ULONG flag,
                ULONG cbStack)
{
  l4_uint32_t flags = L4THREAD_CREATE_ASYNC;
  l4thread_t rc;
  l4_os3_thread_t thread;
  struct start_data data;
  PTIB tib;
  PID pid;

  KalEnter();

  if (flag & STACK_COMMITED)
    flags |= L4THREAD_CREATE_MAP;

  data.pfn = pfn;
  data.param = param;

  if ( (rc = l4thread_create_long(L4THREAD_INVALID_ID,
                       thread_func, "OS/2 thread",
                       L4THREAD_INVALID_SP, cbStack, L4THREAD_DEFAULT_PRIO,
                       &data, flags)) > 0)
  {
    // @todo watch the thread ids to be in [1..128] range
    ulThread++;
    *ptid = ulThread;

    // get pid
    KalGetPID(&pid);
    // crsate TIB, update PTDA
    thread.thread = l4thread_l4_id(rc);
    KalNewTIB(pid, ulThread, thread);
    // get new TIB
    KalGetTIB(&ptib[ulThread - 1]);
    tib = ptib[ulThread - 1];
    tib->tib_eip_saved = 0;
    tib->tib_esp_saved = 0;

    // suspend thread if needed
    if (flag & CREATE_SUSPENDED)
      KalSuspendThread(ulThread);

    rc = NO_ERROR;
  }
  else
  {
    io_log("Thread creation error: %d\n", rc);

    switch (-rc)
    {
      case L4_EINVAL:     rc = ERROR_INVALID_PARAMETER; break;
      case L4_ENOTHREAD:  rc = ERROR_MAX_THRDS_REACHED; break;
      case L4_ENOMAP:
      case L4_ENOMEM:     rc = ERROR_NOT_ENOUGH_MEMORY; break;
      default:            rc = ERROR_INVALID_PARAMETER; // ???
    }
  }

  KalQuit();
  return rc;
}

APIRET CDECL
KalSuspendThread(TID tid)
{
  l4_threadid_t preempter = L4_INVALID_ID;
  l4_threadid_t pager     = L4_INVALID_ID;
  l4_os3_thread_t id;
  l4_umword_t eflags, eip, esp;
  PTIB tib;
  PID pid;

  KalEnter();

  // get pid
  KalGetPID(&pid);
  // get L4 native thread id
  KalGetNativeID(pid, tid, &id);

  if (l4_thread_equal(id.thread, L4_INVALID_ID))
  {
    KalQuit();
    return ERROR_INVALID_THREADID;
  }

  // suspend thread execution: set eip to -1
  l4_thread_ex_regs(id.thread, (l4_umword_t)wait_func, ~0,
                    &preempter, &pager,
                    &eflags, &eip, &esp);

  tib = ptib[tid - 1];
  tib->tib_eip_saved = eip;
  tib->tib_esp_saved = esp;
  KalQuit();
  return NO_ERROR;
}

APIRET CDECL
KalResumeThread(TID tid)
{
  l4_os3_thread_t id;
  l4_threadid_t preempter = L4_INVALID_ID;
  l4_threadid_t pager     = L4_INVALID_ID;
  l4_umword_t eflags, eip, esp, new_eip, new_esp;
  PTIB tib;
  PID pid;

  KalEnter();

  // get pid
  KalGetPID(&pid);
  // get L4 native thread id
  KalGetNativeID(pid, tid, &id);

  if (l4_thread_equal(id.thread, L4_INVALID_ID))
  {
    KalQuit();
    return ERROR_INVALID_THREADID;
  }

  tib = ptib[tid - 1];

  if (! tib->tib_eip_saved)
    return ERROR_NOT_FROZEN;

  new_eip = tib->tib_eip_saved;
  new_esp = tib->tib_esp_saved;

  // resume thread
  l4_thread_ex_regs(id.thread, new_eip, new_esp,
                    &preempter, &pager,
                    &eflags, &eip, &esp);

  tib->tib_eip_saved = 0;
  tib->tib_esp_saved = 0;
  KalQuit();
  return NO_ERROR;
}

APIRET CDECL
KalWaitThread(PTID ptid, ULONG option)
{
  l4_threadid_t me = l4_myself();
  l4_os3_thread_t id, src;
  l4_umword_t   dw1, dw2;
  l4_msgdope_t  dope;
  APIRET        rc = NO_ERROR;
  TID           tid = 0;
  PID           pid;

  KalEnter();

  // get pid
  KalGetPID(&pid);

  if (! ptid)
    ptid = &tid;

  // get native L4 id
  KalGetNativeID(pid, *ptid, &id);

  // wait until needed thread terminates
  switch (option)
  {
    case DCWW_WAIT:
      for (;;)
      {
        if (! l4_ipc_wait(&src.thread, L4_IPC_SHORT_MSG,
                          &dw1, &dw2, L4_IPC_NEVER,
                          &dope) &&
            l4_task_equal(src.thread, me) )
        {
          if (*ptid)
          {
            if (l4_thread_equal(id.thread, L4_INVALID_ID))
            {
              rc = ERROR_INVALID_THREADID;
              break;
            }

            if (l4_thread_equal(src.thread, id.thread))
              break;
          }
          else
          {
            KalGetTIDNative(src, ptid);
            break;
          }
        }
      }
      break;

    case DCWW_NOWAIT: // ???
      if (l4_thread_equal(id.thread, L4_INVALID_ID))
        rc = ERROR_INVALID_THREADID;
      else
        rc = ERROR_THREAD_NOT_TERMINATED;
      break;

    default:
      rc = ERROR_INVALID_PARAMETER;
  }

  KalQuit();
  return rc;
}

APIRET CDECL
KalKillThread(TID tid)
{
  l4_os3_thread_t id;
  PID pid;
  APIRET rc = NO_ERROR;

  KalEnter();

  // get current task pid
  KalGetPID(&pid);
  // get L4 native thread ID
  KalGetNativeID(pid, tid, &id);

  if (l4_thread_equal(id.thread, L4_INVALID_ID))
  {
    KalQuit();
    return ERROR_INVALID_THREADID;
  }

  if (! (rc = l4thread_shutdown(l4thread_id(id.thread))) )
    io_log("thread killed\n");
  else
    io_log("thread kill failed!\n");

  // free thread TIB
  KalDestroyTIB(pid, tid);

  KalQuit();
  return rc;
}

/* Get tid of current thread */
APIRET CDECL
KalGetTID(TID *ptid)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2server_dos_GetTID_call(&os2srv, ptid, &env);
  rc = CPClientGetTID(ptid);
  KalQuit();
  return rc;
}

/* Get pid of current process */
APIRET CDECL
KalGetPID(PID *ppid)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2server_dos_GetPID_call(&os2srv, ppid, &env);
  rc = CPClientGetPID(ppid);
  KalQuit();
  return rc;
}

APIRET CDECL
KalGetNativeID(PID pid, TID tid, l4_os3_thread_t *id)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2server_dos_GetNativeID_call(&os2srv, pid, tid, id, &env);
  rc = CPClientGetNativeID(pid, tid, id);
  KalQuit();
  return rc;
}

APIRET CDECL
KalGetTIDNative(l4_os3_thread_t id, TID *ptid)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2server_dos_GetTIDNative_call(&os2srv, &id, ptid,  &env);
  rc = CPClientGetTIDNative(&id, ptid);
  KalQuit();
  return rc;
}

APIRET CDECL
KalNewTIB(PID pid, TID tid, l4_os3_thread_t id)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2server_dos_NewTIB_call(&os2srv, pid, tid, &id, &env);
  rc = CPClientNewTIB(pid, tid, &id);
  KalQuit();
  return rc;
}

/* Destroy TIB of thread with given pid/tid */
APIRET CDECL
KalDestroyTIB(PID pid, TID tid)
{
  //CORBA_Environment env = dice_default_environment;
  APIRET rc;

  KalEnter();
  //rc = os2server_dos_DestroyTIB_call(&os2srv, pid, tid, &env);
  rc = CPClientDestroyTIB(pid, tid);
  KalQuit();
  return rc;
}

/* Get TIB of current thread */
APIRET CDECL
KalGetTIB(PTIB *ptib)
{
  //CORBA_Environment env = dice_default_environment;
  l4_os3_dataspace_t ds;
  void *addr;
  APIRET rc;

  KalEnter();
  //rc = os2server_dos_GetTIB_call(&os2srv, &ds.ds, &env);
  rc = CPClientGetTIB(&ds);

  if (rc)
  {
    KalQuit();
    return rc;
  }

  /* attach it */ 
  rc = attach_ds(ds, L4DM_RW, &addr);
  if (rc)
  {
    io_log("error attaching TIB!\n");
    KalQuit();
    return rc;
  }
  else
    io_log("TIB attached\n");

  *ptib = (PTIB)addr;

  (*ptib)->tib_ptib2 = (PTIB2)((char *)(*ptib)->tib_ptib2 + (unsigned)addr);

  KalQuit();
  return rc;
}

/* Get PPIB of current process */
APIRET CDECL
KalGetPIB(PPIB *ppib)
{
  //CORBA_Environment env = dice_default_environment;
  l4_os3_dataspace_t ds;
  void *addr;
  APIRET rc;

  KalEnter();
  //rc = os2server_dos_GetPIB_call (&os2srv, &ds.ds, &env);
  rc = CPClientGetPIB(&ds);

  if (rc)
  {
    KalQuit();
    return rc;
  }

  /* attach it */ 
  rc = attach_ds(ds, L4DM_RW, &addr);
  if (rc)
  {
    io_log("error attaching PIB!\n");
    KalQuit();
    return rc;
  }
  else
    io_log("PIB attached\n");

  *ppib = (PPIB)addr;

  /* fixup fields */
  (*ppib)->pib_pchcmd = (char *)((*ppib)->pib_pchcmd) + (unsigned)addr;
  (*ppib)->pib_pchenv = (char *)((*ppib)->pib_pchenv) + (unsigned)addr;

  KalQuit();
  return rc;
}

/* Map info blocks when starting up process */
APIRET CDECL
KalMapInfoBlocks(PTIB *ptib, PPIB *ppib)
{
  APIRET rc;

  KalEnter();

  /* get the dataspace with info blocks */
  rc = KalGetTIB(ptib);
  /* get the dataspace with info blocks */
  rc = KalGetPIB(ppib);

  KalQuit();
  return NO_ERROR;
}

/* Get PTIB and PPIB of current process/thread */
APIRET CDECL
KalGetInfoBlocks(PTIB *pptib, PPIB *pppib)
{
  TID tid;
  KalEnter();
  // get thread ID
  KalGetTID(&tid);
  // TIB
  *pptib = ptib[tid - 1];
  // PIB
  *pppib = ppib;
  KalQuit();
  return NO_ERROR;
}
