/*
 *
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/* libc includes */
#include <stdlib.h>
#include <dlfcn.h>
/* l4env includes */
#include <l4/env/env.h>
#include <l4/env/errno.h>
#include <l4/thread/thread.h>
#include <l4/dm_mem/dm_mem.h>
#include <l4/dm_generic/consts.h>
#include <l4/names/libnames.h>
#include <l4/log/l4log.h>
/* dice includes   */
#include <dice/dice.h>
/* osFree includes */
#include <l4/os3/gcc_os2def.h>
#include <l4/os3/cfgparser.h>
#include <l4/os3/ixfmgr.h>
#include <l4/os3/modmgr.h>
#include <l4/os3/memmgr.h>
#include <l4/os3/globals.h>
#include "os2exec-server.h"

/* execsrv link address */
const l4_addr_t execsrv_start_addr = 0xba000000;
/* l4rm heap address (thus, moved from 0xa000 higher) */
const l4_addr_t l4rm_heap_start_addr = 0xb9000000;
/* l4thread stack map start address */
const l4_addr_t l4thread_stack_area_addr = 0xbc000000;
/* l4thread TCB table map address */
const l4_addr_t l4thread_tcb_table_addr = 0xbe000000;

cfg_opts options;
//struct t_mem_area os2server_root_mem_area;

l4_threadid_t os2srv;
l4_threadid_t fs;
l4_threadid_t execsrv;
l4_threadid_t loader;

l4env_infopage_t *infopage;

/* Root for module list.    */
extern struct module_rec module_root;
/* Root mem area for memmgr */
struct t_mem_area root_area;

void app_pager(void *unused);

extern IXFHandler *IXFHandlers;

#define OPENFLAG_EXEC       1
#define ERROR_MOD_NOT_FOUND 126

long
getimp(unsigned long hmod,
       int *index,
       unsigned long *imp_hmod);

long DICE_CV
os2exec_open_component (CORBA_Object _dice_corba_obj,
                        const char* fname /* in */,
                        const l4dm_dataspace_t *img_ds /* in */,
                        unsigned long flags /* in */,
                        unsigned long *hmod /* out */,
                        CORBA_Server_Environment *_dice_corba_env)
{
  char exeflag = flags & OPENFLAG_EXEC;
  /* Error info from ModLoadModule */
  char chLoadError[CCHMAXPATH];
  
  return OpenModule(chLoadError, sizeof(chLoadError), fname, exeflag, hmod);
}


long DICE_CV
os2exec_load_component (CORBA_Object _dice_corba_obj,
                        unsigned long hmod /* in */,
			os2exec_module_t *s /* out */,
                        CORBA_Server_Environment *_dice_corba_env)
{
  unsigned long rc;
  char chLoadError[CCHMAXPATH];
  IXFModule *ixf;
  IXFSYSDEP *sysdep;

  rc = LoadModule(chLoadError, sizeof(chLoadError), &hmod);

  ixf = (IXFModule *)hmod;
  s->ip = ixf->EntryPoint;
  
  sysdep = (IXFSYSDEP *)(ixf->hdlSysDep);
  s->sp = sysdep->stack_high;
  s->sp_limit = sysdep->stack_low;
  s->hmod = hmod;

  LOG("load_component exited");  
  
  return rc;
}


/*  share dataspaces with executable file sections
 *  with a client task
 */
int
share_sections (unsigned long hmod, l4_threadid_t client)
{
  IXFModule *ixf;
  IXFSYSDEP *sysdep;
  l4exec_section_t *section;
  l4_uint32_t rights;
  slist_t *s;
  unsigned long imp_hmod;
  int index;
  int rc;

  ixf = (IXFModule *)hmod;
  sysdep = (IXFSYSDEP *)(ixf->hdlSysDep);
  s = sysdep->seclist;
  
  while (s)
  {
    section = s->section;
    rights = 0;

    if (section->info.type & L4_DSTYPE_READ)
      rights |= L4DM_READ;
      
    if (section->info.type & L4_DSTYPE_WRITE)
      rights |= L4DM_WRITE;
    
    rc = l4dm_share(&section->ds, client, rights);
    
    if (rc)
    {
      LOG("error sharing dataspace %x with task %x.%x",
          section->ds, client.id.task, client.id.lthread);
      return rc;
    }
    LOG("dataspace %x shared", section->ds);
    s = s->next;
  } 

  index = 0;
  while (!(rc = getimp (hmod, &index, &imp_hmod)))
  {
    if (!imp_hmod) continue; // KAL, no sections to share
    rc = share_sections(imp_hmod, client);
    LOG("module %x sections shared", imp_hmod);
    if (rc) break;
  }
  
  if (rc == ERROR_MOD_NOT_FOUND)
    return 0;


  return rc;
}


long DICE_CV
os2exec_share_component (CORBA_Object _dice_corba_obj,
                         unsigned long hmod /* in */,
                         CORBA_Server_Environment *_dice_corba_env)
{
  return share_sections(hmod, *_dice_corba_obj);
}


long
getimp(unsigned long hmod,
       int *index,
       unsigned long *imp_hmod)
{
  int rc;
  int ind = 0;
  IXFModule *ixf;
  char **mod;
  /* Error info from ModLoadModule */
  char chLoadError[CCHMAXPATH];
  unsigned long hmod2;
  
  if (!hmod)
    return ERROR_INVALID_PARAMETER;

  ixf = (IXFModule *)hmod;

  if (ixf->cbModules <= *index)
    return ERROR_MOD_NOT_FOUND; /* 126 */

  for (ind = 0, mod = ixf->Modules; ind < ixf->cbModules; ind++, mod++)
    if (ind == *index)
      break;

  /* open a module to get its handle. If it is already
     opened, it just returned module handle, not opens
     it for the next time */
  rc = OpenModule(chLoadError, sizeof(chLoadError), *mod, 0, &hmod2);

  if (rc)
    return ERROR_MOD_NOT_FOUND; /* 126 */
#if 0
  ixf = (IXFModule *)hmod2;

  /* skip fake KAL module */
  if (!strcasecmp(ixf->name, "KAL"))
    hmod2 = 0;
#endif
  *imp_hmod = hmod2;
  ++ *index;  

  return rc;  
}

long DICE_CV
os2exec_getimp_component (CORBA_Object _dice_corba_obj,
                          unsigned long hmod /* in */,
                          int *index /* in, out */,
                          unsigned long *imp_hmod /* out */,
                          CORBA_Server_Environment *_dice_corba_env)
{
  return getimp(hmod, index, imp_hmod);
}


long DICE_CV
os2exec_getsect_component (CORBA_Object _dice_corba_obj,
                           unsigned long hmod /* in */,
                           int *index /* in, out */,
                           l4exec_section_t *sect /* out */,
                           CORBA_Server_Environment *_dice_corba_env)
{
  int rc;
  IXFModule *ixf;
  IXFSYSDEP *sysdep;
  slist_t   *s, *r;
  int i;
  
  ixf = (IXFModule *)hmod;
  sysdep = (IXFSYSDEP *)(ixf->hdlSysDep);
  s = sysdep->seclist;

  if (!s) // section list is empty (i.e., forwarder)
  {
    sect = 0;
    return 1;
  }

  for (i = 0; s && i < *index + 1; i++, s = s->next) r = s;

  if (i < *index)
  {
    sect = 0;
    return 1; // set more real error
  }

  memmove((char *)sect, (char *)r->section, sizeof(l4exec_section_t));
  ++ *index;
  
  return 0;
}


long DICE_CV
os2exec_query_procaddr_component (CORBA_Object _dice_corba_obj,
                                  unsigned long hmod /* in */,
                                  unsigned long ordinal /* in */,
                                  const char* modname /* in */,
                                  l4_addr_t *addr /* out */,
                                  CORBA_Server_Environment *_dice_corba_env)
{
  return ModQueryProcAddr(hmod, ordinal, modname, (void **)addr);
}


/** attach dataspace to our address space. */
int
attach_ds(l4dm_dataspace_t *ds, l4_uint32_t flags, l4_addr_t *addr)
{
  int error;
  l4_size_t size;
 
  if ((error = l4dm_mem_size(ds, &size)))
    {
      printf("Error %d (%s) getting size of dataspace\n",
	  error, l4env_errstr(error));
      return error;
    }
  
  if ((error = l4rm_attach(ds, size, 0, flags, addr)))
    {
      printf("Error %d (%s) attaching dataspace\n",
	  error, l4env_errstr(error));
      return error;
    }

  return 0;
}

long DICE_CV
os2exec_query_modhandle_component (CORBA_Object _dice_corba_obj,
                                   const char* pszModname /* in */,
                                   unsigned long *phmod /* out */,
                                   CORBA_Server_Environment *_dice_corba_env)
{
  return ModQueryModuleHandle(pszModname, phmod);
}


long DICE_CV
os2exec_query_modname_component (CORBA_Object _dice_corba_obj,
                                 unsigned long hmod /* in */,
                                 unsigned long cbBuf /* in */,
                                 char* *pBuf /* out */,
                                 CORBA_Server_Environment *_dice_corba_env)
{
  return ModQueryModuleName(hmod, cbBuf, pBuf);
}

#if 0
int
loadso (void)
{
  void *hdl, *func;

  /* load libmemmgr.s.so */  
  LOG_printf("calling dl_open at %08lx\n", (l4_addr_t)dlopen);
  hdl = dlopen("libmemmgr.s.so", 2);
  LOG_printf("handle libmemmgr=%08lx\n", (l4_addr_t)hdl);

  init_memmgr = dlsym(hdl, "init_memmgr");
  LOG_printf("init_memmgr=%08lx\n", (l4_addr_t)init_memmgr);

  seek_free_mem = dlsym(hdl, "seek_free_mem");
  LOG_printf("seek_free_mem=%08lx\n", (l4_addr_t)seek_free_mem);

  is_mem_used = dlsym(hdl, "is_mem_used");
  LOG_printf("is_mem_used=%08lx\n", (l4_addr_t)is_mem_used);

  alloc_mem_area = dlsym(hdl, "alloc_mem_area");
  LOG_printf("alloc_mem_area=%08lx\n", (l4_addr_t)alloc_mem_area);

  dealloc_mem_area = dlsym(hdl, "dealloc_mem_area");
  LOG_printf("dealloc_mem_area=%08lx\n", (l4_addr_t)dealloc_mem_area);

  print_used_mem = dlsym(hdl, "print_used_mem");
  LOG_printf("print_used_mem=%08lx\n", (l4_addr_t)print_used_mem);


  return 0;
}
#endif

void *
getsym (void *hdl, char *fmtname, char *fn)
{
  char *func[0x40];
  char *p = fmtname, *q = func;
  void *sym;

  while (*p) *q++ = toupper(*p++);
  *q++ = '\0';
  strcat(func, fn);

  return dlsym(hdl, func);
}

void *
get_ixf_handler(char *fmtname, IXFHandler **handler)
{
  char lib[0x40];
  void *hdl;

  strcpy(lib, fmtname);
  strcat(lib, ".ixf");

  hdl = dlopen(lib, 2);
  if (!hdl) return 0;
  LOG_printf("handle %s=%08lx\n", lib, (l4_addr_t)hdl);
  
  (*handler) = (IXFHandler *)malloc(sizeof(IXFHandler)); // todo: release!

  (*handler)->Identify = getsym(hdl, fmtname, "Identify");
  (*handler)->Load     = getsym(hdl, fmtname, "Load");
  (*handler)->Fixup    = getsym(hdl, fmtname, "Fixup");
  (*handler)->next     = 0;
 
  return hdl;
}


int
load_ixfs (void)
{
  void *hdl, *identify, *load, *fixup;
  IXFHandler *handler, *last;
  void *rc;

  rc = get_ixf_handler("lx", &handler);
  if (!rc) return 1;
  
  LOG_printf("Identify=%x, Load=%x, Fixup=%x\n",
             handler->Identify, handler->Load, handler->Fixup);  

  IXFHandlers = handler;
  
  rc = get_ixf_handler("ne", &handler->next);
  if (!rc) return 1;
  
  LOG_printf("Identify=%x, Load=%x, Fixup=%x\n",
             handler->next->Identify, handler->next->Load, handler->next->Fixup);  

  return 0;
}


void main (int argc, char *argv[])
{
  CORBA_Server_Environment env = dice_default_server_environment;
  l4dm_dataspace_t ds;
  l4_threadid_t pager;
  l4_addr_t addr;
  l4_size_t size;
  int  rc, t;
  int  is_int = 0;
  int  value_int = 0;
  char buf[0x1000];
  char *p = buf;

  //init_globals();

  if (!names_register("os2exec"))
    {
      LOG("Error registering on the name server!");
      return;
    }

  if (!names_waitfor_name("os2srv", &os2srv, 30000))
    {
      LOG("os2srv not found on name server!");
      return;
    }

  if (!names_waitfor_name("os2fs", &fs, 30000))
    {
      LOG("os2fs not found on name server!");
      return;
    }
  LOG("os2fs tid:%x.%x", fs.id.task, fs.id.lthread);

  if (!names_waitfor_name("LOADER", &loader, 30000))
    {
      LOG("LOADER not found on name server!");
      return;
    }

  execsrv = l4_myself();

#if 1
  /* reserve the area below 64 Mb for application private code
     (not for use by libraries, loaded by execsrv) */
  addr = 0x2f000; size = 0x04000000 - addr;
  if ((rc  = l4rm_direct_area_setup_region(addr,
					   size,
					   L4RM_DEFAULT_REGION_AREA,
					   L4RM_REGION_BLOCKED, 0,
					   L4_INVALID_ID)) < 0)
    {
      LOG_printf("main(): setup region %x-%x failed (%d)!\n",
	  addr, addr + size, rc);
      l4rm_show_region_list();
      enter_kdebug("PANIC");
    }
#endif
  memset (&options, 0, sizeof(options));

#if 0
  if (!CfgGetopt("debugmodmgr", &is_int, &value_int, (char **)&p))
    if (is_int)
      options.debugmodmgr = value_int;

  if (!CfgGetopt("debugixfmgr", &is_int, &value_int, (char **)&p))
    if (is_int)
      options.debugixfmgr = value_int;

  if (!CfgGetopt("libpath", &is_int, &value_int, (char **)&p))
    if (!is_int)
    {
      options.libpath = (char *)malloc(strlen(p) + 1);
      strcpy(options.libpath, p);
    }

  LOG("debugmodmgr=%d", options.debugmodmgr);
  LOG("debugixfmgr=%d", options.debugixfmgr);
  LOG("libpath=%s", options.libpath);
#endif
  options.libpath = (char *)malloc(4);
  strcpy(options.libpath, "c:\\");

  /* get our l4env infopage as a dataspace */
  rc = l4loader_app_info_call(&loader, l4_myself().id.task,
                              0, &p, &ds, &env);
  /* attach it */  
  attach_ds(&ds, L4DM_RO, &infopage);

#if 0
  /* load shared libs */
  rc = loadso();
  if (rc)
  {
    LOG("Error loading shared libraries");
    return;
  }
#endif

  init_memmgr(&root_area);

  /* load IXF's */
  rc = load_ixfs();
  if (rc)
  {
    LOG("Error loading IXF driver");
    return;
  }
			      
  /* Initializes the module list. Keeps info about which dlls an process have loaded and
     has linked to it (Only support for LX dlls so far). The head of the linked list is
     declared as a global inside dynlink.c */
  rc = ModInitialize();
  if (rc)
  {
    LOG("Can't initialize module manager");
    return;
  }

  // server loop
  LOG("execsrv started.");
  env.malloc = (dice_malloc_func)malloc;
  env.free = (dice_free_func)free;
  os2exec_server_loop(&env);
}

#ifdef __cplusplus
}
#endif
