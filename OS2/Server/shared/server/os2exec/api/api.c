/* osFree OS/2 personality internal */
#include <os3/io.h>
#include <os3/kal.h>
#include <os3/rm.h>
#include <os3/modmgr.h>

/* libc includes */
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <ctype.h>

/* local includes */
#include "api.h"

int load_ixfs (void);
void *get_ixf_handler(char *fmtname, IXFHandler **handler);
void *getsym (void *hdl, char *fmtname, char *fn);
vmdata_t *get_mem_by_name(const char *pszName);
vmdata_t *get_area(void *addr);

long excShare(mod_list_t **list, unsigned long hmod, l4_os3_thread_t client_id);

extern IXFHandler *IXFHandlers;

/* shared memory arena settings */
void          *shared_memory_base = (void *)0x60000000;
unsigned long  shared_memory_size = 1024 * 1024 * 1024;
unsigned long long shared_memory_area;

vmdata_t *areas_list = NULL;

long ExcOpen(char *szLoadError,
             unsigned long cbLoadError,
             const char *pszModname,
             unsigned long flags,
             unsigned long *phmod)
{
    char exeflag = flags & OPENFLAG_EXEC;
    return OpenModule(szLoadError, cbLoadError, pszModname, exeflag, phmod);
}

long ExcLoad(unsigned long *phmod,
             char *szLoadError,
             unsigned long cbLoadError,
             os2exec_module_t *s)
{
    unsigned long rc;
    IXFModule *ixf;
    IXFSYSDEP *sysdep;

    rc = LoadModule(szLoadError, cbLoadError, phmod);

    ixf = (IXFModule *)*phmod;
    s->ip = ixf->EntryPoint;

    if (ixf->area == 0)
        s->exeflag = 1;
    else
        s->exeflag = 0;

    sysdep = (IXFSYSDEP *)(unsigned long)(ixf->hdlSysDep);
    s->sp = sysdep->stack_high;
    s->sp_limit = sysdep->stack_low;
    s->hmod = *phmod;

    io_log("load_component exited\n");

    return rc;
}

long ExcFree(unsigned long hmod)
{
    return ModFreeModule(hmod);
}

long ExcShare(unsigned long hmod, l4_os3_thread_t client_id)
{
    mod_list_t *list = NULL;
    return excShare(&list, hmod, client_id);
}

long ExcGetImp(unsigned long hmod,
               unsigned long *index,
               unsigned long *imp_hmod)
{
    /* Error info from ModLoadModule */
    char chLoadError[CCHMAXPATH];
    unsigned long ind = 0;
    unsigned long hmod2;
    IXFModule *ixf;
    char **mod;
    int rc;

    if (! hmod || ! index || ! imp_hmod)
      return ERROR_INVALID_PARAMETER;

    ixf = (IXFModule *)hmod;

    if (ixf->cbModules <= *index)
      return ERROR_MOD_NOT_FOUND;

    for (ind = 0, mod = ixf->Modules; ind < ixf->cbModules; ind++, mod++)
    {
      if (ind == *index)
        break;
    }

    /* open a module to get its handle. If it is already
       opened, it just returned module handle, not opens
       it for the next time */
    rc = OpenModule(chLoadError, sizeof(chLoadError), *mod, 0, &hmod2);

    if (rc)
      return ERROR_MOD_NOT_FOUND;
#if 0
    ixf = (IXFModule *)hmod2;

    /* skip fake KAL module */
    if (! strcasecmp(ixf->name, "KAL") )
      hmod2 = 0;
#endif
    *imp_hmod = hmod2;
    ++ *index;

    return rc;
}

long ExcGetSect(unsigned long hmod,
                unsigned long *index,
                l4_os3_section_t *sect)
{
    IXFModule *ixf;
    IXFSYSDEP *sysdep;
    slist_t   *s, *r = NULL;
    unsigned long i;

    ixf = (IXFModule *)hmod;
    sysdep = (IXFSYSDEP *)(unsigned long)(ixf->hdlSysDep);
    s = sysdep->seclist;

    if (! s)
    {
      // section list is empty (i.e., forwarder)
      sect = NULL;
      return 1;
    }

    for (i = 0; s && i < *index + 1; i++, s = s->next)
    {
      r = s;
    }

    if (i < *index)
    {
      sect = NULL;
      return 1; // set more real error
    }

    io_log("r->section->type=%x\n", r->section->type);
    memcpy((char *)sect, (char *)r->section, sizeof(l4_os3_section_t));
    io_log("sect->type=%x\n", sect->type);
    ++ *index;

    return 0;
}

long ExcQueryProcAddr(unsigned long hmod,
                      unsigned long ordinal,
                      const char *pszName,
                      void **addr)
{
    return ModQueryProcAddr(hmod, ordinal, pszName, addr);
}

long ExcQueryModuleHandle(const char *pszModname,
                       unsigned long *hmod)
{
    return ModQueryModuleHandle(pszModname, hmod);
}

long ExcQueryModuleName(unsigned long hmod,
                     unsigned long cbBuf,
                     char *pszBuf)
{
    return ModQueryModuleName(hmod, cbBuf, pszBuf);
}

long ExcAllocSharedMem(unsigned long size,
                       const char *name,
                       unsigned long rights,
                       void **addr,
                       unsigned long long *area)
{
  unsigned long long area2 = shared_memory_area;
  vmdata_t *ptr;
  int rc = 0, ret;

  //ret =  l4rm_area_reserve_in_area(size, 0, addr, &area2);
  ret = RegAreaReserveInArea(size, 0, addr, &area2);

  if (ret < 0)
    return ERROR_NOT_ENOUGH_MEMORY;

  *area = area2;

  ptr = (vmdata_t *)malloc(sizeof(vmdata_t));

  if (! ptr) 
    return ERROR_NOT_ENOUGH_MEMORY;

  ptr->refcnt = 1;
  ptr->area = area2;
  ptr->addr = *addr;
  ptr->size = size;

  if (name)
    strcpy(ptr->name, name);

  ptr->rights = rights;

  if (areas_list)
    areas_list->prev = ptr;

  ptr->next = areas_list;
  ptr->prev = 0;
  areas_list = ptr;

  return rc;
}

long ExcMapDataspace(void               *addr,
                     unsigned long      rights,
                     l4_os3_dataspace_t ds)
{
  vmdata_t *ptr;
  int ret;

  io_log("areas_list=%x\n", areas_list);

  if (! (ptr = get_area(addr)) )
    return ERROR_INVALID_ADDRESS;

  //if ( (ret = attach_ds_area(*ds, ptr->area, rights, addr)) < 0)
  if ( (ret = RegAttachDataspaceToArea(ds, ptr->area, rights, addr)) < 0)
    return ERROR_FILE_NOT_FOUND;

  return 0;
}

long ExcUnmapDataspace(void               *addr,
                       l4_os3_dataspace_t ds)
{
  return RegDetach(addr);
}

long ExcGetDataspace(void               **addr,
                     unsigned long      *size,
                     l4_os3_dataspace_t *ds,
                     l4_os3_thread_t    client_id)
{
  //l4_threadid_t pager;
  ULONG offset;
  //l4dm_dataspace_t temp_ds;
  int ret, rc = NO_ERROR;

  if (! addr || ! size || ! ds)
    return ERROR_INVALID_PARAMETER;

  //ret = l4rm_lookup_region((void *)*addr, addr, size, &temp_ds, &offset, &pager);
  ret = RegLookupRegion(*addr, addr, size, &offset, ds);

  //if ( (ret == L4RM_REGION_DATASPACE) )
  if ( (ret == REG_DATASPACE) )
  {
    // transfer dataspace to client
    io_log("*** %x\n", (char *)*addr);
    //*ds = &temp_ds;
    //if ( (ret = l4dm_share(*ds, *(l4_threadid_t *)client_id, L4DM_RW)) < 0)
    //{
      //switch (-ret)
      //{
        //case L4_EINVAL: rc = ERROR_FILE_NOT_FOUND; break;
        //case L4_EPERM:  rc = ERROR_ACCESS_DENIED; break;
        //default:        rc = ERROR_INVALID_PARAMETER;
      //}
    //}
    rc = DataspaceShare(*ds, client_id, 0); // L4DM_RW !!!
  }
  else
    rc = ERROR_FILE_NOT_FOUND;

  return rc;
}

long ExcGetSharedMem(void             *pb,
                     void             **addr,
                     unsigned long    *size,
                     PID              *owner)
                     //l4_os3_thread_t  *owner)
{
  vmdata_t *ptr;

  if (! (ptr = get_area(pb)) )
    return ERROR_INVALID_ADDRESS;

  ptr->refcnt ++;

  *addr  = ptr->addr;
  *size  = ptr->size;
  *owner = ptr->owner;

  return NO_ERROR;
}

long ExcGetNamedSharedMem(const char        *name,
                          void              **addr,
                          unsigned long     *size,
                          PID               *owner)
                          //l4_os3_thread_t   *owner)
{
  vmdata_t *ptr;

  if (! (ptr = get_mem_by_name(name)) )
    return ERROR_INVALID_NAME;

  ptr->refcnt ++;

  *addr = ptr->addr;
  *size = ptr->size;
  *owner = ptr->owner;

  return NO_ERROR;
}

/*  increment the refcnt for a sharemem area
 */
long ExcIncrementSharedMemRefcnt(void *addr)
{
  vmdata_t *ptr;

  if (! (ptr = get_area(addr)) )
    return ERROR_INVALID_NAME;

  ptr->refcnt ++;

  return NO_ERROR;
}

/*  release the reserved sharemem area
 */
long ExcReleaseSharedMem(void          *addr,
                         unsigned long *count)
{
  unsigned long long area;
  vmdata_t *ptr;

  if (! (ptr = get_area(addr)) )
    return ERROR_INVALID_ADDRESS;

  ptr->refcnt --;
  *count = ptr->refcnt;

  if (ptr->refcnt)
    return NO_ERROR;

  area = ptr->area;

  if (ptr->prev)
    ptr->prev->next = ptr->next;

  if (ptr->next)
    ptr->next->prev = ptr->prev;

  free(ptr);
  //return l4rm_area_release(area);
  return RegAreaRelease(area);
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

void *getsym (void *hdl, char *fmtname, char *fn)
{
  char func[0x40];
  char *p = fmtname, *q = func;

  while (*p)
    *q++ = toupper(*p++);

  *q++ = '\0';
  strcat(func, fn);

  return dlsym(hdl, func);
}

void *get_ixf_handler(char *fmtname, IXFHandler **handler)
{
  char lib[0x40];
  void *hdl;

  strcpy(lib, fmtname);
  strcat(lib, ".ixf");

  hdl = dlopen(lib, 2);

  if (! hdl)
    return 0;

  io_log("handle %s=%08lx\n", lib, (void *)hdl);

  (*handler) = (IXFHandler *)malloc(sizeof(IXFHandler)); // todo: release!

  (*handler)->Identify = getsym(hdl, fmtname, "Identify");
  (*handler)->Load     = getsym(hdl, fmtname, "Load");
  (*handler)->Fixup    = getsym(hdl, fmtname, "Fixup");
  (*handler)->next     = 0;

  return hdl;
}

int load_ixfs (void)
{
  IXFHandler *handler;
  void *rc;

  rc = get_ixf_handler("lx", &handler);

  if (! rc)
    return 1;

  io_log("Identify=%x, Load=%x, Fixup=%x\n",
         handler->Identify, handler->Load, handler->Fixup);

  IXFHandlers = handler;

  rc = get_ixf_handler("ne", &handler->next);

  if (! rc)
    return 1;

  io_log("Identify=%x, Load=%x, Fixup=%x\n",
         handler->next->Identify, handler->next->Load, handler->next->Fixup);

  return 0;
}

vmdata_t *get_area(void *addr)
{
  vmdata_t *ptr;

  io_log("addr=%x\n", addr);

  for (ptr = areas_list; ptr; ptr = ptr->next)
  {
    io_log("%x\n", ptr->addr);

    if (ptr->addr <= addr && addr <= ptr->addr + ptr->size)
      break;
  }

  return ptr;
}

vmdata_t *get_mem_by_name(const char *pszName)
{
  vmdata_t *ptr;

  for (ptr = areas_list; ptr; ptr = ptr->next)
  {
    if (ptr->name[0] && ! strcmp(ptr->name, pszName))
      break;
  }

  return ptr;
}
