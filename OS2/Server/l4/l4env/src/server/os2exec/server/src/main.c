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
#include <getopt.h>
#include <ctype.h>
/* l4env includes */
#include <l4/env/env.h>
#include <l4/env/errno.h>
#include <l4/dm_generic/consts.h>
#include <l4/names/libnames.h>
#include <l4/generic_ts/generic_ts.h>
//#include <l4/events/events.h>
#include <l4/util/l4_macros.h>
/* dice includes   */
#include <dice/dice.h>
/* osFree includes */
#include <l4/os3/gcc_os2def.h>
#include <l4/os3/cfgparser.h>
#include <l4/os3/ixfmgr.h>
#include <l4/os3/modmgr.h>
#include <l4/os3/memmgr.h>
#include <l4/os3/globals.h>
#include <l4/os3/dataspace.h>
#include <l4/os3/thread.h>
#include <l4/os3/types.h>
#include <l4/os3/io.h>

/* OS/2 Server includes */
#include <l4/loader/loader-client.h>
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

l4os3_cap_idx_t os2srv;
l4os3_cap_idx_t fs;
l4os3_cap_idx_t execsrv;
l4os3_cap_idx_t loader;
l4os3_cap_idx_t fprov_id;

l4env_infopage_t *infopage;

// use events server flag
char use_events = 0;
/* Root for module list.    */
extern struct module_rec module_root;
/* Root mem area for memmgr */
struct t_mem_area root_area;

void app_pager(void *unused);

extern IXFHandler *IXFHandlers;

#define OPENFLAG_EXEC       1
#define ERROR_MOD_NOT_FOUND 126

int share_sections (unsigned long hmod, l4os3_cap_idx_t client);
void *getsym (void *hdl, char *fmtname, char *fn);
void *get_ixf_handler(char *fmtname, IXFHandler **handler);
int load_ixfs (void);
void usage(void);


long
getimp(unsigned long hmod,
       int *index,
       unsigned long *imp_hmod);

long DICE_CV
os2exec_open_component (CORBA_Object _dice_corba_obj,
                        const char* fname /* in */,
                        const l4dm_dataspace_t *img_ds /* in */,
                        unsigned long flags /* in */,
                        char **chLoadError /* in, out */,
                        unsigned long *cbLoadError /* out */,
                        unsigned long *hmod /* out */,
                        CORBA_Server_Environment *_dice_corba_env)
{
  char exeflag = flags & OPENFLAG_EXEC;
  /* Error info from ModLoadModule */
  //char chLoadError[CCHMAXPATH];

  return OpenModule(*chLoadError, *cbLoadError, fname, exeflag, hmod);
}


long DICE_CV
os2exec_load_component (CORBA_Object _dice_corba_obj,
                        unsigned long hmod /* in */,
                        char **chLoadError /* in, out */,
                        unsigned long *cbLoadError /* out */,
                        os2exec_module_t *s /* out */,
                        CORBA_Server_Environment *_dice_corba_env)
{
  unsigned long rc;
  //char chLoadError[CCHMAXPATH];
  IXFModule *ixf;
  IXFSYSDEP *sysdep;

  rc = LoadModule(*chLoadError, *cbLoadError, &hmod);

  ixf = (IXFModule *)hmod;
  s->ip = (ULONG)ixf->EntryPoint;

  sysdep = (IXFSYSDEP *)(ixf->hdlSysDep);
  s->sp = sysdep->stack_high;
  s->sp_limit = sysdep->stack_low;
  s->hmod = hmod;

  io_log("load_component exited\n");  

  return rc;
}


/*  share dataspaces with executable file sections
 *  with a client task
 */
int
share_sections (unsigned long hmod, l4os3_cap_idx_t client)
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
      io_log("error sharing dataspace %x with task %x.%x\n",
          section->ds, client.id.task, client.id.lthread);
      return rc;
    }
    io_log("dataspace %x shared\n", section->ds);
    s = s->next;
  }

  index = 0;
  while (!(rc = getimp (hmod, &index, &imp_hmod)))
  {
    if (!imp_hmod) continue; // DL, no sections to share
    rc = share_sections(imp_hmod, client);
    io_log("module %x sections shared\n", imp_hmod);
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

  /* skip fake DL module */
  if (!strcasecmp(ixf->name, "DL"))
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
  //int rc;
  IXFModule *ixf;
  IXFSYSDEP *sysdep;
  slist_t   *s, *r = NULL;
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
  return ModQueryModuleName(hmod, cbBuf, *pBuf);
}


typedef struct
{
  char name[256];        // name for named shared mem
  l4_uint32_t   rights;  // OS/2-style access flags
  l4_uint32_t   area;    // area id
} vmdata_t;

/*   get a free area of specified size and reserve it in shared arena
 */
long DICE_CV
os2exec_alloc_sharemem_component (CORBA_Object _dice_corba_obj,
                                    l4_uint32_t size /* in */,
				    const char *name /* in */,
                                    l4_addr_t *addr /* out */,
                                    CORBA_Server_Environment *_dice_corba_env)
{
  l4_uint32_t area;
  vmdata_t *ptr; 
  int rc;
  
  rc =  l4rm_area_reserve(size, 0, addr, &area);
  ptr = (vmdata_t *)malloc(sizeof(vmdata_t));
  if (!ptr) return 1;
  ptr->area = area;
  if (name) strcpy(ptr->name, name);
  l4rm_set_userptr(addr, ptr);
  
  return rc;
}

long DICE_CV
os2exec_get_sharemem_component (CORBA_Object _dice_corba_obj,
                                l4_addr_t pb /* in */,
                                l4_addr_t *addr /* out */,
                                l4_uint32_t *size /* out */,
                                CORBA_Server_Environment *_dice_corba_env)
{
  vmdata_t *ptr;
  l4_uint32_t rights = 0;
  l4_addr_t address;
  l4_size_t sz = 0;
  l4_offs_t offset;
  l4os3_cap_idx_t pager;
  l4dm_dataspace_t ds;
  int rc;

  rc = l4rm_lookup_region((void *)pb, &address, &sz, &ds,
                     &offset, &pager);
  switch (rc)
  {
    case L4RM_REGION_RESERVED:
    case L4RM_REGION_DATASPACE:
      if ( (ptr = l4rm_get_userptr(addr)) )
	rights = ptr->rights;
      break;
    default:
      return ERROR_INVALID_ADDRESS;
  }
  
  *addr = address;
  *size = sz;
  
  return NO_ERROR;
}


long DICE_CV
os2exec_get_namedsharemem_component (CORBA_Object _dice_corba_obj,
                                     const char* name /* in */,
                                     l4_addr_t *addr /* out */,
                                     l4_uint32_t *size /* out */,
                                     CORBA_Server_Environment *_dice_corba_env)
{
  vmdata_t *ptr;
  l4rm_region_desc_t *desc;
  //l4_uint32_t rights = 0;
  //l4_addr_t address;
  //l4_size_t sz = 0;
  //l4_offs_t offset;
  //l4os3_cap_idx_t pager;
  //l4dm_dataspace_t ds;
  int found = 0;

  /* get region list */
  desc = l4rm_get_region_list();

  while (desc)
  {
    ptr = (vmdata_t *)desc->userptr;
    
    if (ptr && !strcmp(name, ptr->name))
    {
      found = 1;
      break;
    }
    desc = desc->next;
  }  
  
  if (found)
  {
    *addr = desc->start;
    *size = desc->end - desc->start;
    return NO_ERROR;
  }

  return ERROR_FILE_NOT_FOUND;
}

/*  release the reserved sharemem area
 */
long DICE_CV
os2exec_release_sharemem_component (CORBA_Object _dice_corba_obj,
                                    l4_addr_t addr /* in */,
                                    CORBA_Server_Environment *_dice_corba_env)
{
  l4_uint32_t area;
  vmdata_t *ptr;

  ptr = l4rm_get_userptr((void *)addr);
  if (!ptr) return 1;
  area = ptr->area;
  free(ptr);
  return l4rm_area_release(area);
}


#if 0
int
loadso (void)
{
  void *hdl, *func;

  /* load libmemmgr.s.so */  
  io_log("calling dl_open at %08lx\n", (l4_addr_t)dlopen);
  hdl = dlopen("libmemmgr.s.so", 2);
  io_log("handle libmemmgr=%08lx\n", (l4_addr_t)hdl);

  init_memmgr = dlsym(hdl, "init_memmgr");
  io_log("init_memmgr=%08lx\n", (l4_addr_t)init_memmgr);

  seek_free_mem = dlsym(hdl, "seek_free_mem");
  io_log("seek_free_mem=%08lx\n", (l4_addr_t)seek_free_mem);

  is_mem_used = dlsym(hdl, "is_mem_used");
  io_log("is_mem_used=%08lx\n", (l4_addr_t)is_mem_used);

  alloc_mem_area = dlsym(hdl, "alloc_mem_area");
  io_log("alloc_mem_area=%08lx\n", (l4_addr_t)alloc_mem_area);

  dealloc_mem_area = dlsym(hdl, "dealloc_mem_area");
  io_log("dealloc_mem_area=%08lx\n", (l4_addr_t)dealloc_mem_area);

  print_used_mem = dlsym(hdl, "print_used_mem");
  io_log("print_used_mem=%08lx\n", (l4_addr_t)print_used_mem);


  return 0;
}
#endif

void *
getsym (void *hdl, char *fmtname, char *fn)
{
  char func[0x40];
  char *p = fmtname, *q = func;
  //void *sym;

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
  io_log("handle %s=%08lx\n", lib, (l4_addr_t)hdl);

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
  IXFHandler *handler;
  void *rc;

  rc = get_ixf_handler("lx", &handler);
  if (!rc) return 1;

  io_log("Identify=%x, Load=%x, Fixup=%x\n",
             handler->Identify, handler->Load, handler->Fixup);  

  IXFHandlers = handler;

  rc = get_ixf_handler("ne", &handler->next);
  if (!rc) return 1;

  io_log("Identify=%x, Load=%x, Fixup=%x\n",
             handler->next->Identify, handler->next->Load, handler->next->Fixup);  

  return 0;
}

void usage(void)
{
  io_log("execsrv usage:\n");
  io_log("-e:  Use events server");
} 

#if 0
void event_thread(void)
{
  l4events_ch_t event_ch = L4EVENTS_EXIT_CHANNEL;
  l4events_nr_t event_nr = L4EVENTS_NO_NR;
  l4events_event_t event;
  l4os3_cap_idx_t tid;
  int rc;

  if (!l4events_init())
  {
    io_log("l4events_init() failed\n");
    return;
  }

  if ((rc = l4events_register(L4EVENTS_EXIT_CHANNEL, 15)) != 0)
  {
    io_log("l4events_register failed\n");
    return;
  }

  while(1)
  {
    /* wait for event */
    if ((rc = l4events_give_ack_and_receive(&event_ch, &event, &event_nr,
					    L4_IPC_NEVER,
					    L4EVENTS_RECV_ACK))<0)
    {
      io_log("l4events_give_ack_and_receive()\n");
      continue;
    }
    tid = *(l4os3_cap_idx_t *)event.str;
    io_log("Got exit event for %x.%x\n", tid.id.task, tid.id.lthread);

    /* exit myself */
    if (l4_task_equal(tid, os2srv))
      exit(rc);
  }
}
#endif


int main (int argc, char *argv[])
{
  CORBA_Server_Environment env = dice_default_server_environment;
  CORBA_Environment e = dice_default_environment;
  l4dm_dataspace_t ds;
  //l4os3_cap_idx_t pager;
  l4_addr_t addr;
  l4_size_t size;
  int  rc;
  //int  is_int = 0;
  //int  value_int = 0;
  char buf[0x1000];
  char *p = buf;
  int optionid;
  int opt = 0;
  const struct option long_options[] =
                {
                { "events",      no_argument, NULL, 'e'},
		{ 0, 0, 0, 0}
                };

  //init_globals();

  if (!names_register("os2exec"))
    {
      io_log("Error registering on the name server!\n");
      return -1;
    }

  if (!names_waitfor_name("os2srv", &os2srv, 30000))
    {
      io_log("os2srv not found on name server!\n");
      return -1;
    }

  if (!names_waitfor_name("os2fs", &fs, 30000))
    {
      io_log("os2fs not found on name server!\n");
      return -1;
    }
  fprov_id = fs;
  io_log("os2fs tid:%x.%x\n", fs.id.task, fs.id.lthread);

  if (!names_waitfor_name("LOADER", &loader, 30000))
    {
      io_log("LOADER not found on name server!\n");
      return -1;
    }

  execsrv = l4_myself();

  // Parse command line arguments
  for (;;)
  {
    opt = getopt_long(argc, argv, "e", long_options, &optionid);
    if (opt == -1) break;
    switch (opt)
    {
      case 'e':
        io_log("using events server\n");
	use_events = 1;
	break;
      
      default:
        io_log("Error: Unknown option %c\n", opt);
        usage();
        return -1;
    }
  }

  // start events thread
  /* if (use_events)
  {
    // start events thread
    l4thread_create(event_thread, 0, L4THREAD_CREATE_ASYNC);
    io_log("event thread started");
  } */

  /* reserve the area below 64 Mb for application private code
     (not for use by libraries, loaded by execsrv) */
  addr = 0x2f000; size = 0x04000000 - addr;
  if ((rc  = l4rm_direct_area_setup_region(addr,
					   size,
					   L4RM_DEFAULT_REGION_AREA,
					   L4RM_REGION_BLOCKED, 0,
					   L4_INVALID_ID)) < 0)
    {
      io_log("main(): setup region %x-%x failed (%d)!\n",
	  addr, addr + size, rc);
      l4rm_show_region_list();
      enter_kdebug("PANIC");
    }
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

  io_log("debugmodmgr=%d\n", options.debugmodmgr);
  io_log("debugixfmgr=%d\n", options.debugixfmgr);
  io_log("libpath=%s\n", options.libpath);
#endif
  options.libpath = (char *)malloc(4);
  strcpy(options.libpath, "c:\\");

  /* get our l4env infopage as a dataspace */
  rc = l4loader_app_info_call(&loader, l4_myself().id.task,
                              0, &p, &ds, &e);
  /* attach it */  
  attach_ds(&ds, L4DM_RO, (l4_addr_t *)&infopage);

#if 0
  /* load shared libs */
  rc = loadso();
  if (rc)
  {
    io_log("Error loading shared libraries\n");
    return -1;
  }
#endif

  init_memmgr(&root_area);

  /* load IXF's */
  rc = load_ixfs();
  if (rc)
  {
    io_log("Error loading IXF driver\n");
    return -1;
  }

  /* Initializes the module list. Keeps info about which dlls an process have loaded and
     has linked to it (Only support for LX dlls so far). The head of the linked list is
     declared as a global inside dynlink.c */
  rc = ModInitialize();
  if (rc)
  {
    io_log("Can't initialize module manager\n");
    return -1;
  }

  // server loop
  io_log("execsrv started.\n");
  env.malloc = (dice_malloc_func)malloc;
  env.free = (dice_free_func)free;
  os2exec_server_loop(&env);
  return 0;
}

#ifdef __cplusplus
}
#endif
