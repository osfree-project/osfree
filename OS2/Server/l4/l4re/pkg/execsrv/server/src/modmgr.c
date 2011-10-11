/*
    LXLoader - Loads LX exe files or DLLs for execution or to extract information from.
    Copyright (C) 2007  Sven Rosén (aka Viking)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
    Or see <http://www.gnu.org/licenses/>
*/

#define INCL_DOSERRORS
#define INCL_MISC
#define INCL_ERRORS
//#include <os2.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <l4/env/env.h>
#include <l4/log/l4log.h>
#include <l4/dm_mem/dm_mem.h>
#include <l4/dm_generic/consts.h>

#include <l4/os3/token.h>
#include <l4/os3/io.h>
#include <l4/os3/modmgr.h>
#include <l4/os3/ixfmgr.h>
#include <l4/os3/cfgparser.h>
#include <l4/os3/os2errcodes.h>
#include <l4/os3/apistub.h>
#include <l4/os3/MountReg.h>


unsigned int  find_module_path(const char *name, char *full_path_name);
unsigned int  find_path(const char *name, char *full_path_name);

struct module_rec module_root; /* Root for module list.*/

extern PVOID entry_Table[];

extern _prog_img_start;

void ModLinkModule (IXFModule *ixfModule, unsigned long *phmod);
/* Some general ideas here:
     We use static array of modules structures here. We limit now to 1024 module handles.
     Such approach is much protected in comparation of allowing direct access to module structure.

@todo: Add option MAXLOADEDMODULES=xxxx to config.sys for more flexibility?
*/

struct module_rec modulehandles[1024];

/*! @brief Searches for the module name which the process needs.
    It first sees if it's already loaded and then just returns the
    found module handle. If it can't be found then loads module
    using LIBPATH, BEGINLIBPATH, ENDLIBPATH variables/options and
    applies fixups.

    @param pszName The address of a buffer into which the name of an object
                   that contributed to the failure of ModLoadModule is to
                   be placed. The name of the object is usually the name
                   of a dynamic link or shared library that either could
                   not be found or could not be loaded.

    @param cbName  The length, in bytes, of the buffer described by pszName.
    @param pszModname The address of an ASCIIZ name string that contains the
                      dynamic link module name. The file-name extension used
                      for dynamic link libraries is ".DLL". For shared
                      libraries "lib" prefix and ".so" suffix are added.
                      When a request is made to load a module and a
                      fully-qualified path is specified, the system loads
                      that library, if it exists. If a fully-qualified path
                      is not specified, the system checks if the library
                      is already loaded. If it is loaded, that library is
                      the one that is used; otherwise, the system searches
                      the paths in the LIBPATH string in the "CONFIG.SYS"
                      file and uses the first instance of the specified
                      library it finds. If the current directory is not
                      specified in the LIBPATH, the system does not check
                      that directory to see if a different version exists.
                      Consequently, if two processes started from different
                      directories use the same library, but different
                      versions of that library exist in both directories,
                      the version of the library loaded by the first process
                      is the one used by both processes. Extended control
                      of search path is provided by BEGINLIBPATH and
                      ENDLIBPATH environment variables.

    @param phmod      Pointer to an module handle in which the handle for
                      the library is returned.

    @return
      NO_ERROR                  module loaded successfully
      ERROR_INVALID_PARAMETER   invalid function parameted passed
      ERROR_FILE_NOT_FOUND      file not found in searched path
      ERROR_PATH_NOT_FOUND
      ERROR_TOO_MANY_OPEN_FILES
      ERROR_ACCESS_DENIED
      ERROR_NOT_ENOUGH_MEMORY   not enough memory
      ERROR_BAD_FORMAT
      ERROR_NOT_DOS_DISK
      ERROR_SHARING_VIOLATION
      ERROR_LOCK_VIOLATION
      ERROR_SHARING_BUFFER_EXCEEDED
      ERROR_INTERRUPT
      ERROR_DRIVE_LOCKED
      ERROR_INVALID_NAME
      ERROR_PROC_NOT_FOUND
      ERROR_INVALID_SEGMENT_NUMBER
      ERROR_INVALID_ORDINAL
      ERROR_INVALID_MODULETYPE
      ERROR_INVALID_EXE_SIGNATURE
      ERROR_EXE_MARKED_INVALID
      ERROR_ITERATED_DATA_EXCEEDS_64K
      ERROR_INVALID_MINALLOCSIZE
      ERROR_DYNLINK_FROM_INVALID_RING
      ERROR_INVALID_SEGDPL
      ERROR_AUTODATASEG_EXCEEDS_64K
      ERROR_RELOCSRC_CHAIN_EXCEEDS_SEGLIMIT
      ERROR_FILENAME_EXCED_RANGE
      ERROR_INIT_ROUTINE_FAILED
*/

int
getrec(char *mname, struct module_rec **p)
{
  struct module_rec *prev;
  
  // First search in the module list
  prev = (struct module_rec *) module_root.next;

  while(prev)
  {
    if(strcasecmp(mname, prev->mod_name)==0)
    {
      if(prev->load_status == LOADING)
        return -1;
	
      *p = prev;
      return 0;
    }
    prev = (struct module_rec *) prev->next;
  }
  
  return 1;
}

int
getrec2(unsigned long hmod, struct module_rec **p)
{
  struct module_rec *prev;
  
  // First search in the module list
  prev = (struct module_rec *) module_root.next;

  while(prev)
  {
    if(hmod == (unsigned long)prev)
    {
      if(prev->load_status == LOADING)
        return -1;
	
      *p = prev;
      return 0;
    }
    prev = (struct module_rec *) prev->next;
  }
  
  return 1;
}


unsigned long OpenModule(char *          pszName,
                         unsigned long   cbName,
                         char const *    pszModname,
			 char            exeflag,
                         unsigned long * phmod)
{
#ifdef L4API_l4v2
  l4_addr_t pageaddr, addr2;
#endif
  #define buf_size 4096
  char buf[buf_size+1];
  char *p_buf = (char *) &buf;
  //char *orig_name = NULL;
  struct module_rec *prev;
  IXFModule *ixfModule, *ixf;
  IXFSYSDEP *ixfSysDep, *sd;
  slist_t *s, *s0, *r;
  l4exec_section_t *section;
  void *addr;
  unsigned long size;
  int  rc, t, n, i;

  // Check input arguments
  if ((phmod==NULL)||
      (pszModname==NULL)||
      (pszName==NULL)) return 87 /*ERROR_INVALID_PARAMETER*/;

  // Initialize return vars
  *pszName=0;
  *phmod=0;

  // @todo extract filename only because can be fullname with path

  char *mname = get_fname(pszModname);

  
  // Specail case - EMXWRAP.DLL. Read more in docs\os2\sub32.txt
  if (!exeflag && !strcasecmp(mname, "EMXWRAP"))
  {
    if (options.debugmodmgr) LOG("ModLoadModule: EXMWRAP module replaced by SUB32 module.");
    //mname="SUB32";
    pszModname="SUB32.DLL";
  }

  if (options.debugmodmgr) LOG("ModLoadModule: Loading module %s...", mname);

  ////
  //if (!exeflag)
    if (!(t = getrec(mname, &prev)))
    {
      if (!exeflag)
      {
        // @todo use handles here
        *phmod=(unsigned long)prev->module_struct;
        LOG("already loaded");
        return 0/*NO_ERROR*/;
      }
    }
    else if (t == -1)
    {
      LOG("mod not loaded!");
      *phmod=NULL;
      if (cbName<=strlen(mname)) return 8 /*ERROR_NOT_ENOUGH_MEMORY*/;
      strcpy(pszName, mname);
      return 5 /*ERROR_ACCESS_DENIED*/; // @todo Need more accurate code
    }

  // Ok. No module found. Try to load file
  LOG("open: %s", pszModname);
  // Consider fully-qualified name specified.
  rc=io_load_file(pszModname, &addr, &size);
  if (rc)
  {
    // Searches for module name and returns the full path in the buffer p_buf.
    LOG("io_load_file1: rc=%u", rc);

    if (!exeflag)
      rc = find_module_path(pszModname, p_buf);
    else
      rc = find_path(pszModname, p_buf);

    LOG("find_module_path: rc=%u, p_buf=%s", rc, p_buf);
    if (!rc) rc=io_load_file(p_buf, &addr, &size);
  }
  else
    LOG("successful");
  
  if (rc)
  {
    LOG("io_load_file2: rc=%u", rc);
    strcpy(pszName, pszModname);
    *phmod=NULL;
    return rc;
  }

  ixfModule = (IXFModule *)malloc(sizeof(IXFModule));
#ifdef L4API_l4v2
  ixfSysDep = (IXFSYSDEP *)malloc(sizeof(IXFSYSDEP));

  if (!t && exeflag)
  {
    // 1st instance IXFModule structure
    ixf = (IXFModule *)(prev->module_struct);
    memmove(ixfModule, ixf, sizeof(IXFModule));

    sd = (IXFSYSDEP *)(ixf->hdlSysDep);
    memmove(ixfSysDep, sd, sizeof(IXFSYSDEP));
    ixfModule->hdlSysDep = (unsigned int)ixfSysDep;

    for (i = 0, r = 0, s0 = sd->seclist; 
         s0; i++, r = s, s0 = s0->next)
    {
      s = (slist_t *)malloc(sizeof(slist_t));
      if (i == 0) // 1st loop iteration
        ixfSysDep->seclist = s;
      else
      {
        r->next = s;
	s->next = 0;
      }
      section = (l4exec_section_t *)malloc(sizeof(l4exec_section_t));
      s->section = section;
      memmove(section, s0->section, sizeof(l4exec_section_t));
      // create Copy-On-Write copy of original dataspace
      rc = l4dm_copy(&s0->section->ds, L4DM_COW, 
                     "os2exec section", &section->ds);
      if (rc)
        LOG("dataspace copy rc=%d", rc);
    }
    // @todo use handles here
    *phmod=(unsigned long)ixfModule;
    ModRegister(mname, ixfModule, exeflag);
    LOG("copy created");
    return 0/*NO_ERROR*/;
  }
  else
  {
    ixfModule->hdlSysDep = (unsigned int)ixfSysDep;
    // initialize section number to zero
    ixfSysDep->secnum = 0;
    ixfSysDep->seclist = 0;
  }
#else
  // other hosts
  // ...
#endif

  rc=IXFIdentifyModule(addr, size, ixfModule);
  if (rc)
  {
    LOG("IXFIdentifyModule: rc=%u", rc);
    strcpy(pszName, pszModname);
    *phmod=NULL;
    free((void *)(ixfModule->hdlSysDep));
    free(ixfModule);
    return rc;
  }

//save addr and size into the structure
  ixfModule->addr = addr;
  ixfModule->size = size;
  ixfModule->name = (char *)malloc(strlen(pszModname) + 1);
  strcpy(ixfModule->name, pszModname);
  ixfModule->exec = exeflag;
  //@todo use handle table
  *phmod=(unsigned long)ixfModule;

  return rc;
}

unsigned long LoadModule(char *          pszName,
                         unsigned long   cbName,
                         unsigned long * phmod)
{
  struct module_rec * new_module_el;
  UCHAR chLoadError[CCHMAXPATH]; /* Error info from ModLoadModule */
  unsigned long rc=NO_ERROR;
  IXFModule *ixfModule = *phmod;
  void *addr = ixfModule->addr;
  unsigned long size = ixfModule->size;
  void *ptr_mod;
  unsigned long module_counter;
  unsigned long imports_counter;
  unsigned long hmod;
  int relative_jmp;
  char *pszModname;
  char exeflag;
  struct module_rec *prev;

  pszModname  = ixfModule->name;
  exeflag    = ixfModule->exec;
  char *mname = get_fname(pszModname);
  int t;

  t = getrec(mname, &prev);

  if (!strcasecmp(mname, "SUB32"))
    t = getrec("EMXWRAP", &prev);

  LOG("%s", mname);

  if (!t)
  {
    if (!*phmod)
      return 6; /* ERROR_INVALID_HANDLE */
  
    if (!exeflag)
    {
      LOG("already loaded, dll");
      *phmod = (unsigned long)prev->module_struct;
      return 0;
    }
    else
    {
      /* get the last instance */
      while (prev->up)
        prev = prev->up;
	
      LOG("already loaded, exe");
      *phmod = (unsigned long)prev->module_struct;
      prev->load_status = DONE_LOADING;
      return 0;
    }
  }
 
  if ((t == 1) || (!t && prev->load_status != DONE_LOADING));
  {
    // not loaded
    // Load module
    rc=IXFLoadModule(addr, size, ixfModule);
    if (rc)
    {
      strcpy(pszName, pszModname);
      *phmod=NULL;
      free(ixfModule);
      LOG("0");
      return rc;
    }
  }

  // Display module entry table
  if (options.debugixfmgr)
  {
    unsigned long entries_counter;
    LOG("Module entry table");
    LOG("------------------");
    for (entries_counter=1;
         entries_counter<ixfModule->cbEntries+1;
         entries_counter++)
    {
      if (ixfModule->Entries[entries_counter-1].ModuleName)
      {
        LOG("%s.%d", ixfModule->Entries[entries_counter-1].ModuleName, ixfModule->Entries[entries_counter-1].Ordinal);
      } else {
        LOG("%x", ixfModule->Entries[entries_counter-1].Address);
      }
    }

  }

  if (!strcasecmp(mname, "SUB32"))
    mname = "EMXWRAP";
  LOG("mname=%s", mname);
  LOG("exeflag=%d", exeflag);
  // Register in module list
  // @todo extract filename only because can be fullname with path
  new_module_el = ModRegister(mname, ixfModule, exeflag);
  new_module_el->load_status = LOADING;

  // Load modules which not loaded yet
  for (module_counter=1;
       module_counter<ixfModule->cbModules+1;
       module_counter++)
  {
    char  name[1024];//=malloc(strlen(ixfModule->Modules[module_counter-1]+5));
    strcpy(name, ixfModule->Modules[module_counter-1]);
    strcat(name, ".dll");

    /* if module is not loaded, then load it */
    if (t = getrec(ixfModule->Modules[module_counter-1], &prev))
    {
      LOG("opening %s", name);
      rc = OpenModule(chLoadError, sizeof(chLoadError), name,
                      0, (unsigned long *)&hmod);
      if (!rc) LOG("open successful");
      if (rc)
      {
        LOG("Error opening module: %s", name);
        return rc;
      }

      LOG("loading %s", name);
      rc = LoadModule(chLoadError, sizeof(chLoadError), (unsigned long *)&hmod);
      if (!rc) LOG("load successful");
    
      if (rc)
      {
        LOG("Error loading module: %s", name);
        return rc;
      }
    }
    // Fixup module
    if (!exeflag)
    {
#if 1
      rc=IXFFixupModule(ixfModule);
      if (rc)
      {
        LOG("IXFFixupModule: rc=%u", rc);
        strcpy(pszName, pszModname);
        *phmod=NULL;
        free(ixfModule);
        return rc;
      }
      else
      { /*
        if (!strcmp(pszModname, "KAL"))
        {
          IXFMODULEENTRY *e;
          char *name;
          int  ord;
          APIRET APIENTRY (*func) ();

          e = ixfModule->Entries;
          ord  = e->Ordinal;
          name = e->FunctionName;
          if (!strcmp(name, "KalInit") && ord == 1)
          {
            func = e->Address;
            func(entry_Table);
            io_printf("KalInit() found and called @ 0x%x\n", func);
          }
          else
          {
            io_printf("No KalInit.1 function in KAL.dll, fatal!");
            rc = ERROR_INVALID_FUNCTION;
          }
        } */
      }
#endif
      //if (t)
      //{
        //LOG("000");
        //ModLinkModule(&hmod, phmod);
      //}
    }
  }


  // Fixup module

  LOG("111");
  if (exeflag)
  {
    LOG("222");
    rc = IXFFixupModule(ixfModule);
    if (rc!=0/*NO_ERROR*/)
    {
      LOG("LoadModule: Error %s module fixup", pszModname);
      LOG("3");
      return rc;
    }
  }

  LOG("333");
  ModLinkModule(ixfModule, phmod);
  //@todo use handle table
  *phmod=(unsigned long)ixfModule;
  new_module_el->load_status = DONE_LOADING;

  LOG("4");
  
  return rc; /*NO_ERROR;*/
}


unsigned long ModLoadModule(char *          pszName,
                            unsigned long   cbName,
                            char const *    pszModname,
                            unsigned long * phmod)
{
  int rc = 0;
  
  rc = OpenModule(pszName, cbName, pszModname, 0, phmod);

  if (rc) 
    return rc;
  
  rc = LoadModule(pszName, cbName, phmod);
  return rc;
}

unsigned long ModLoadExeModule(char *          pszName,
                               unsigned long   cbName,
                               char const *    pszModname,
                               unsigned long * phmod)
{
  int rc = 0;
  
  rc = OpenModule(pszName, cbName, pszModname, 1, phmod);

  if (rc) 
    return rc;
  
  rc = LoadModule(pszName, cbName, phmod);
  return rc;
}

unsigned long ModQueryModuleHandle(const char *pszModname, unsigned long *phmod)
{
  struct module_rec *prev;
  char *mname;
  
  mname = get_fname(pszModname);
  if (getrec(mname, &prev))
  {
    LOG("module not found");
    *phmod = 0;
    return 123; /* ERROR_INVALID_NAME */
  }
  *phmod = prev->module_struct;
  return 0; /* NO_ERROR */
}

unsigned long ModQueryModuleName(unsigned long hmod, unsigned long cbName, char *pchName)
{
  struct module_rec *prev;
  
  if (getrec2(hmod, &prev))
  {
    LOG("module not found");
    *pchName = '\0';
    return 6; /* ERROR_INVALID_HANDLE */
  }

  if (strlen(prev->mod_name) + 1 > cbName)
    return 24; /* ERROR_BAD_LENGTH */
    
  strcpy(pchName, prev->mod_name);
  return 0; /* NO_ERROR */
}


slist_t *
next_slist (slist_t *s)
{
  slist_t *p;
  l4exec_section_t *section;    
  
  p = (slist_t *)malloc(sizeof(slist_t));
  section = (l4exec_section_t *)malloc(sizeof(l4exec_section_t));

  if (s)  s->next = p;

  p->next = 0;
  p->section = section;

  return p;
}

void
getline (char **from, char *to)
{
  char *p, *q = to;
  
  for (p = *from; *p && *p != '\r' && *p != '\n'; p++, q++) *q = *p;
  *q++ = '\0';
  while (*p == '\r' || *p == '\n') p++;
  *from = p;
}

int
dl_get_funcs (int *numentries, IXFMODULEENTRY **entries)
{
  unsigned long addr, size;
  char buf[0x100];
  char *line = buf;
  char str[24];
  char *s = str;
  char *p, *funcname;
  int funcaddr;
  int i, n, rc;

  memset(line, 0, 0x100);  
  rc = io_load_file("c:\\dl.map", &addr, &size);
  
  if (rc)
    return rc;
    
  p = (char *)addr;    

  getline(&p, line);

  n = atol(line);
  
  *numentries = n;
  *entries = (IXFMODULEENTRY *)malloc(n * sizeof(IXFMODULEENTRY));
  if (!*entries) return 1;

  for (i = 0; i < n; i++)
  {
    getline(&p, line);
    sscanf(line, "%x %s", &funcaddr, s);
    funcname = (char *)malloc(strlen(s) + 1);
    if (!funcname) return 1;
    strcpy(funcname, s);
    LOG("function %s", funcname);
    LOG("address %x",  funcaddr);
    (*entries)[i].FunctionName = funcname;
    (*entries)[i].Address = funcaddr;
    (*entries)[i].Ordinal  = i + 1;
    (*entries)[i].ModuleName = NULL;
  }
  
  return 0;
}


/* @brief Initializes the root node in the linked list, which itself
   is not used. Only to make sure the list at least always has one
   element allocated. Registers fake DL.DLL which contains host
   specific functions. */
unsigned long ModInitialize(void)
{
  IXFModule *ixf;
  IXFSYSDEP *sysdep;
  l4exec_section_t *section;
  slist_t *r;
  l4_uint32_t rights;
  l4dm_dataspace_t ds;
  l4_addr_t addr;
  l4_addr_t size;
  l4_offs_t offset;
  l4_threadid_t pager;
  unsigned i;
  int rc;
  int (*dl_init)(CORBA_Environment *e);
  void *execsym, *hdl;
  struct module_rec * new_module_el;
  static IXFMODULEENTRY *entries;

  // Create root node

  module_root.mod_name = "root";
  module_root.module_struct = 0;
  module_root.next = 0;

  // Register DL.DLL

  ixf=malloc(sizeof(IXFModule));

  ixf->name  = (char *)malloc(4);
  strcpy(ixf->name, "DL");
  
  ixf->Load  = NULL;
  ixf->Fixup = NULL;
  ixf->FormatStruct = NULL;

  if (rc = dl_get_funcs (&ixf->cbEntries, &ixf->Entries))
    return rc;

  ixf->cbModules=0;
  ixf->Modules=NULL;
  ixf->cbFixups=0;
  ixf->Stack=NULL;
  ixf->EntryPoint=NULL;
  ixf->PIC=0;

  /* add execsrv sections from l4env infopage
     as DL.DLL sections list */
  sysdep = (IXFSYSDEP *)malloc(sizeof(IXFSYSDEP));
  memset(sysdep, 0, sizeof(IXFSYSDEP));
  ixf->hdlSysDep  = sysdep;
  sysdep->secnum  = 0;
  sysdep->seclist = 0;
#if 0
  r = next_slist(0);
  sysdep->secnum  = 1;
  sysdep->seclist = r;

  section = r->section;

  rc = l4rm_lookup_region(&_prog_img_start, &addr, &size, &ds,
                     &offset, &pager);

  if (rc < 0)
    return rc;

  section->addr = addr;
  section->size = size;
  section->info.type = L4_DSTYPE_READ | L4_DSTYPE_WRITE | L4_DSTYPE_EXECUTE;
  section->info.id = 0;

  rights = L4DM_READ | L4DM_WRITE;

  rc = l4dm_mem_open(L4DM_DEFAULT_DSM, section->size,
           4096, rights, "", &section->ds);

  if (rc)
  {
    LOG("error allocating dataspace!");
    return rc;
  }

  rc = l4rm_attach(&section->ds, section->size,
         0, rights, &addr);

  if (rc)
  {
    LOG("error attaching dataspace %x", section->ds);
    return rc;
  }

  memmove(addr, section->addr, section->size);

  l4rm_detach(addr);

  //// ....

  region = l4rm_get_region_list();
  for (i = 0, r = 0; region; i++, region = region->next)
  {
    LOG("addr=%x",  region->start);
    LOG("end=%x",   region->end);
    LOG("flags=%x", region->flags);

    if (!(region->flags & REGION_DATASPACE))
      continue;

    LOG("ds=%x",   region->data.ds.ds.id);

    if ((void *)region->start <= dl_init &&
        dl_init <= (void *)region->end)
    {
      LOG("is code section");
      s = r;
      r = next_slist(r);
      if (!s) sysdep->seclist = r;
      section = r->section;
      section->addr = region->start;
           section->size = region->end - region->start;
      section->ds   = region->data.ds.ds;
      section->info.type = L4_DSTYPE_READ | L4_DSTYPE_EXECUTE;
      section->info.id = 0;
    }
    else
      if ((void *)region->start <= execsym &&
          execsym <= (void *)region->end)
    {
      LOG("is data section");
      s = r;
      r = next_slist(r);
      if (!s) sysdep->seclist = r;
      section = r->section;
      section->addr = region->start;
      section->size = region->end - region->start;
      section->ds   = region->data.ds.ds;
      section->info.type = L4_DSTYPE_READ | L4_DSTYPE_WRITE;
      section->info.id = 1;
    }
    else
      continue;

    rights = L4DM_READ | L4DM_WRITE;

    rc = l4dm_mem_open(L4DM_DEFAULT_DSM, section->size,
           4096, rights, "", &section->ds);

    if (rc)
    {
      LOG("error allocating dataspace!");
      return rc;
    }

    rc = l4rm_attach(&section->ds, section->size,
           0, rights, &addr);

    if (rc)
    {
      LOG("error attaching dataspace %x", section->ds);
      return rc;
    }

    memmove(addr, section->addr, section->size);

    l4rm_detach(addr);
  }
#endif  

  new_module_el = ModRegister("DL", ixf, 0);
  //new_module_el->load_status = DONE_LOADING;

  return NO_ERROR;
}

int lcase(char* dest, const char* src)
{
        while(*dest != 0) {
            *dest = tolower(*src);
                dest++;
                src++;
        }

        return *dest - *src;
}

/* Register a module with the name. 
 * If module with a given name doesn't exist,
 * add it to the end of list (through ->next field)
 * otherwise, if it exists and is an EXE module,
 * then add it to the end of (->up) list
 */
struct module_rec * ModRegister(const char * name, 
                                void * mod_struct, 
				unsigned long exeflag)
{
  struct module_rec * new_mod;
  struct module_rec * prev;
  int    t, n = 0;
  
  new_mod = (struct module_rec *) malloc(sizeof(struct module_rec));

  if (!(t = getrec(name, &prev)) && exeflag)
  {
    // get the last record in the instances list
    while (prev->up)
      prev = prev->up;

    n = prev->instance + 1;

    prev->up = new_mod;
  }
  else
  {
    prev = &module_root;

    while(prev->next) /* Find free node at end. */
      prev = prev->next;

    prev->next = new_mod;  /*struct module_rec module_struct*/
  }

  new_mod->mod_name = (char *)malloc(strlen(name)+1);
  strcpy(new_mod->mod_name, name);
  new_mod->module_struct = mod_struct; /* A pointer to struct LX_module. */
  new_mod->next = NULL;
  new_mod->up   = NULL;
  new_mod->instance = n;
  new_mod->load_status=DONE_LOADING;      /* Status variable to check for recursion in loading state. */

  return new_mod;
}

#ifdef __LINUX__
#include <dirent.h>
#else
#ifdef L4API_l4v2
 /* #include <dirent.h> */
#else
#include <direct.h>
#endif
#endif

/* On L4/Fiasco there is problems with opendir/open/close and such (not implemented) */
#ifndef L4API_l4v2
typedef struct dirent tdirentry;

/*
   Find a file in directory (case insensitive) and copy the real file name
   to a buffer and return true if success or false if not found.
        char * file_to_find=argv[1];
        int B_LEN = 250;
        char buf[251];
        char *str_buf=(char*) &buf;
        if(find_case_file(file_to_find, argv[2], str_buf, B_LEN)) {
            printf("Found file: %s (%s) in %s\n", file_to_find, str_buf, argv[2]);

*/
int find_case_file(char *file_to_find, char *path, char *buffer_of_found_file, int buf_len) {
    DIR *dir = opendir(path);
    tdirentry *diren = (tdirentry *)1;

    while(diren) {
        diren = (tdirentry *)readdir(dir);
        if(!diren)
            break;

        if(strcasecmp(diren->d_name, file_to_find)==0) {
          /*  printf("Hittade fil: %s (%s)\n", diren->d_name, file_to_find);
            printf("(%p), errno:%d \n", diren, errno);
            printf("                 %s, %d, %d, %d \n",
              diren->d_name,  diren->d_reclen,  diren->d_off,  diren->d_ino); */

            strncpy(buffer_of_found_file, diren->d_name, buf_len);
            closedir(dir);
            return 1;
        }
    }
    closedir(dir);
    return 0;
}
#endif


void print_module_table()
{
  struct module_rec * el = &module_root;
  io_printf("--- Loaded Module Table ---\n");
  while((el = el->next))
  {
    io_printf("module = %s, module_struct = %p, load_status = %d\n",
                  el->mod_name, el->module_struct, el->load_status);
  }
}


#if 0
/*  Used functions: get_entry, get_imp_mod_name, get_module_name_res_name_tbl_entry,
                    copy_pas_str, ModLoadModule, native_find_module, get_imp_proc_name,
                    native_get_func_ptr_ord_handle, native_get_func_ptr_handle_modname,
                    get_obj
*/
int apply_import_fixup(struct LX_module * this_module, struct LX_module * found_module,
                            struct o32_obj * lx_obj,
                            int mod_nr, int import_ord, int addit, int srcoff_cnt1,
                            struct r32_rlc * min_rlc, int *ret_rc)
{
  unsigned long rc;
  char * org_mod_name;
  void * native_module;
  unsigned int i_offs_to_fix_in_exe;
  unsigned long int i_dll_func_offs;
  unsigned long int obj_vm_base;
  int * ptr_fixup_code;
  struct LX_module *forward_found_module;
  struct LX_module * prev_mod;
  /* Name of module from forward entry. Chunk of variables for Forward Entrys.*/
  void *fn_ptr;
  char frw_buf_mod_name[255];
  char * frw_mod_name = (char *) &frw_buf_mod_name;
  void *frw_fn_ptr = 0; /* Returned from get_entry for forward entries, used as status check.*/
  int ret_flags, ret_obj, ret_modord, ret_type;
  unsigned long int ret_offset;
  char buf_mod_name[255];
  char * mod_name = (char*)&buf_mod_name;
  UCHAR uchLoadError[CCHMAXPATH] = {0}; /* Error info from DosExecPgm */

  /* Get name of imported module. */
  org_mod_name = get_imp_mod_name(this_module, mod_nr);

  *frw_mod_name = 0;
  copy_pas_str(mod_name, org_mod_name);

  /* Get the entry for an function from a module, assume only two types for now,
     an 32 bit entry or an forward entry.*/
  //io_printf("loadobjlx.c:%d, get_entry( %p, %d \n", __LINE__, found_module, import_ord);
  fn_ptr = get_entry((struct LX_module *)found_module, import_ord,
                     &ret_flags, (int*)&ret_offset, &ret_obj, &ret_modord, &ret_type);
  io_printf(" get_entry1: forward_found_module=0x%x, ordinal:%d, entry type:%d, entry:%d\n",
             found_module, ret_offset, ret_type, fn_ptr);
  /*  import ordinal -> forward import ordinal
      import_ord        ret_offset    */

  /* Forward Entry */
  /* A Forward Entry refers to another module and entry point which itself
     can refer to another module and entrypoint.
     So, to load the modules, find the chain of entrypoints as long it is a
     forward entry. Up to a maximum of 1024 forward entrys.

     There is some sort of bug with the loop below, it works as long the entries
     are forward entries but does not look up the last (which could be a 32-bit entry)
     with the correct function ordinal from the previous forward entry.

     An example:
      forward entry              32-bit entry
      MSG.5 (DosPutMessage)  ->  DOSCALLS.387
                                   ^
                                   Error it checks for DOSCALLS.5
      NOTE! This is possible when the function ordinals inside a dll are UNSORTED (only one
      unsorted item is enough to cause the error).
      Troublesome code is in ixfmgr_lx_modlx.c:576:get_entry()
  */

  if((ret_type & ENTRYFWD)==ENTRYFWD)
  {
    int frw_flags=ret_flags, frw_offset=ret_offset, frw_obj=ret_obj, frw_modord=ret_modord, frw_type=ret_type;
    int frw_ord;
    int forward_counter;
    //io_printf(" Forward Entry.\n");
    forward_found_module=found_module;

    org_mod_name = get_imp_mod_name(found_module, ret_modord);
    io_printf("get_entry, import_ord:%d, ret_offset:%d, ret_type:%d\n",
               import_ord, ret_offset, ret_type);
    frw_ord = ret_offset;
    /*****  ret_offset  ->  frw_ord      ***************/
    forward_counter=1;

    //frw_mod_name = (char *) &frw_buf_mod_name;
    //get_res_name_tbl_entry(struct LX_module * lx_mod, char *entry_name)
    //org_mod_name = get_imp_mod_name(forward_found_module, 0); /* Find out the module name.*/
    //copy_pas_str(frw_mod_name, org_mod_name);
    io_printf("Start module:%s, forward_found_module=0x%x\n",
                                get_module_name_res_name_tbl_entry(forward_found_module),
                                forward_found_module);

    do {
      frw_mod_name = (char *) &frw_buf_mod_name;
      org_mod_name = get_imp_mod_name(forward_found_module, ret_modord);
      copy_pas_str(frw_mod_name, org_mod_name);
      //io_printf("Forward imp mod: %s, ret_offset: %d\n",
      //                frw_mod_name, ret_offset);
      prev_mod = forward_found_module;
      rc=ModLoadModule(uchLoadError, sizeof(uchLoadError), frw_mod_name,
                        (unsigned long *)&forward_found_module);


      if (forward_found_module) {
        forward_found_module=(struct LX_module *)(((IXFModule *)forward_found_module)->FormatStruct);
         io_printf("Loading module: %s, forward_found_module=0x%x\n",
                     get_module_name_res_name_tbl_entry(forward_found_module),
                     forward_found_module);
      }
      else
      {
        io_printf("forward_found_module == 0, prev_mod = 0x%x", prev_mod);
      }
      *ret_rc = rc;
      if(!forward_found_module) { /* Unable to find and load module. */
        io_printf("Can't find forward module: '%s' \n", frw_mod_name);
#if 0
        native_module = native_find_module(frw_mod_name);
        if(native_module != 0) {
          /* Try to load a native library instead. */
          /* void * native_find_module(char * name, struct t_os2process *proc);
             void * native_get_func_ptr_handle_modname(char * funcname, void * native_mod_handle);
          */
          *ret_rc = 0; /*NO_ERROR 0;*/
          char buf_native_name[255];
          char * native_name = (char*)&buf_native_name;
          /* Get name of imported module. */
          char * org_native_name = get_imp_proc_name(prev_mod, frw_offset);
          copy_pas_str(native_name, org_native_name);

          if (strlen(native_name)==0)
          {
          #ifndef __LINUX__
            io_printf("Native ordinal: %d, module name: %s\n", ret_offset, frw_mod_name);
            frw_fn_ptr = native_get_func_ptr_ord_handle(ret_offset , native_module);
          #endif
          } else {
            io_printf("Native name: %s, module name: %s\n", native_name, frw_mod_name);
            frw_fn_ptr = native_get_func_ptr_handle_modname(native_name , native_module);
          }
          ret_offset = (unsigned long int)frw_fn_ptr;
          frw_type = ENTRYNATIVE;
          goto cont_native_entry;
        }
#endif
        return 0;
      }           /* ERROR, frw_ord has an ORDINAL! Not a module index!
                   frw_ord  index of module name in imported module table
                   mod_nr     */
      io_printf(" frw_type=%d, frw_modord=%d(0x%x), frw_ord=%d(0x%x)\n", frw_type,
                 frw_modord, frw_modord, frw_ord, frw_ord);
      /* If the next ordinal lookup returns no found ordinal, save previous info and return that
         instead. Or/and just call this function recursively instead of return?
         A forward entry is a traversal on modules and should probably be put in a separate
         function. */


      frw_fn_ptr = get_entry(forward_found_module, frw_ord,
                  &frw_flags, &frw_offset, &frw_obj, &frw_modord, &frw_type);
      io_printf(" get_entry: forward_found_module=0x%x, ordinal:%d, entry type:%d, entry:%d\n",
         forward_found_module, frw_ord, frw_type, frw_fn_ptr );
      io_printf("get_entry 2, frw_ord:%d, frw_offset:%d \n",  frw_ord, frw_offset);
      frw_ord = frw_offset;
      /***** var rename:  frw_offset  ->  frw_ord      ***************/
      forward_counter++;
      /*io_printf("## }while((frw_type & ... forward_counter=%d\n", forward_counter);*/
    }while((frw_type & ENTRYFWD)==ENTRYFWD && (forward_counter<1024));
    found_module = forward_found_module;

    cont_native_entry:; /* A label that is used from inside the do..while-loop. */
    ret_flags=frw_flags;   ret_offset=frw_offset; ret_obj=frw_obj;
    ret_modord=frw_modord; ret_type=frw_type;
    fn_ptr = frw_fn_ptr;

    io_printf(" frw_modord=%d, frw_ord=%d, frw_type:%d, ENTRYFWD:%d, ENTRY32:%d\n",
           frw_modord, frw_ord, frw_type, ENTRYFWD, ENTRY32);
           /*   ENTRYFWD:4, ENTRY32:3   */
    //io_printf(" Done with Forward Entry running. (%d) \n", forward_counter);
  }

  if (fn_ptr == 0)
  {
    char entry_buf_mod_name[255];
    char * entry_mod_name = (char*)&entry_buf_mod_name;

    io_printf("Error, can't find entrypoint: %d, in module: %d (%s) \n",
                    import_ord, mod_nr, copy_pas_str(entry_mod_name,get_imp_mod_name(found_module, mod_nr)));
    if(frw_mod_name != 0)
      io_printf("frw_mod_name=%s", frw_mod_name);
    if(mod_name != 0)
      io_printf(" mod_name=%s", mod_name);
    io_printf("\n");
    return 0; /* Error can't find an entry point.*/
  }

  /* 32 bit Entry */
  if((ret_type & ENTRY32)==ENTRY32 || ((ret_type & ENTRYNATIVE)== ENTRYNATIVE))
  {
    int relative_jmp;

    if (options.debugixfmgr) io_printf(" Additive = %d\n", addit);

    obj_vm_base = 0;
    i_dll_func_offs = 0;

    if((ret_type & ENTRY32)==ENTRY32)
    {
      //obj_vm_base =  get_obj(found_module, ret_obj)->o32_base;
      obj_vm_base =  get_obj(found_module, ret_obj)->o32_reserved;
      i_dll_func_offs = obj_vm_base + ret_offset;
    } else {
      obj_vm_base     =  (unsigned long int)frw_fn_ptr;
      i_dll_func_offs =  (unsigned long int)frw_fn_ptr;
    }

    //i_offs_to_fix_in_exe = lx_obj->o32_base + srcoff_cnt1;
    i_offs_to_fix_in_exe = lx_obj->o32_reserved + srcoff_cnt1;

    /* Is the EXE module placed under the DLL module in memory? */
    if((i_offs_to_fix_in_exe) < (obj_vm_base+ ret_offset))
    {
      relative_jmp = i_dll_func_offs - i_offs_to_fix_in_exe;
    } else {
      relative_jmp = 0xffffffff-(i_offs_to_fix_in_exe - i_dll_func_offs)-3;
    }
    ptr_fixup_code = (int *) i_offs_to_fix_in_exe;
    *ptr_fixup_code = relative_jmp;
    //io_printf("Patched address: %p \n", ptr_fixup_code);
    //io_printf("Content of patched address: %d \n", *ptr_fixup_code);
  }

  return 1;
}
#endif


unsigned long ModQueryProcAddr(unsigned long hmod,
                               unsigned long ordinal,
                               const char *  pszName,
                               void **       ppfn)
{
  IXFModule * ixfModule;
  struct module_rec * prev;
  unsigned long entries_counter;
  unsigned long searched_hmod;

  // @todo Add input parameters check

  // Find module in module list
  prev = (struct module_rec *) module_root.next;
  while(prev)
  { // @todo use handles here
    if(hmod==(unsigned long)prev->module_struct)
    {
      // @todo use handles here
      ixfModule=(IXFModule *)(prev->module_struct);
      break;
    }
    prev = (struct module_rec *) prev->next;
  }


  if (ordinal > 0)
  { // Search by ordinal
    if (ixfModule->Entries[ordinal-1].ModuleName!=NULL)
    { // This is forward to another module. Call ourself...
      // First search in the module list
      LOG("ModuleName=%x", ixfModule->Entries[ordinal].ModuleName);
      prev = (struct module_rec *) module_root.next;
      while(prev)
      {
        LOG("1");
        if(strcasecmp(ixfModule->Entries[ordinal-1].ModuleName, prev->mod_name)==0)
        {
          LOG("2");
          if(prev->load_status == LOADING)
          {
            searched_hmod=NULL;
            return 5/*ERROR_ACCESS_DENIED*/; // @todo Need more accurate code
          }
          LOG("module: %d", prev->mod_name);
          // @todo use handles here
          searched_hmod=(unsigned long)prev->module_struct;
          break;
        }
        LOG("3");
        prev = (struct module_rec *) prev->next;
      }

      return ModQueryProcAddr(searched_hmod,
                                  ixfModule->Entries[ordinal-1].Ordinal,
                                  ixfModule->Entries[ordinal-1].FunctionName,
                                  ppfn);
    }

    LOG("lalala");
    *ppfn=ixfModule->Entries[ordinal-1].Address;
    LOG("ModQueryProcAddr(%u, %u, %s, %p)", hmod, ordinal, pszName, *ppfn);
    return 0;
  }



  // Search function in entry table
  for (entries_counter=1;
       entries_counter<ixfModule->cbEntries+1;
       entries_counter++)
  {
    // Search by name
    if (!strcmp(ixfModule->Entries[entries_counter-1].FunctionName, pszName))
    { // Found!
      if (ixfModule->Entries[entries_counter-1].ModuleName!=NULL)
      { // This is forward to another module. Call ourself...
        // First search in the module list
        prev = (struct module_rec *) module_root.next;
        while(prev)
        {
          if(strcasecmp(ixfModule->Entries[entries_counter-1].ModuleName, prev->mod_name)==0)
          {
            if(prev->load_status == LOADING)
            {
              searched_hmod=NULL;
              return 5/*ERROR_ACCESS_DENIED*/; // @todo Need more accurate code
            }
            // @todo use handles here
            searched_hmod=(unsigned long)prev->module_struct;
            break;
          }
          prev = (struct module_rec *) prev->next;
        }

        return ModQueryProcAddr(searched_hmod,
                                ixfModule->Entries[entries_counter-1].Ordinal,
                                ixfModule->Entries[entries_counter-1].FunctionName,
                                ppfn);
      }

      LOG("bebebe");
      *ppfn=ixfModule->Entries[entries_counter-1].Address;
      LOG("ModQueryProcAddr(%u, %u, %s, %p)", hmod, ordinal, pszName, *ppfn);
      return 0;
    }
  }
}

void ModLinkModule (IXFModule *ixfModule, unsigned long *phmod)
{
  int imports_counter;
  int relative_jmp;
  char name[256];
  struct module_rec *prev;

  // Link module (import table resolving)
  for (imports_counter=1;
       imports_counter<ixfModule->cbFixups+1;
       imports_counter++)
  {
    LOG("Import entry %d of %d",imports_counter, ixfModule->cbFixups);
    LOG("Module=%s", ixfModule->Fixups[imports_counter-1].ImportEntry.ModuleName);

    prev = (struct module_rec *) module_root.next;
    while(prev)
    {
      //strcpy(name, prev->mod_name);

      //if (!strcasecmp(name, "EMXWRAP"))
      //  strcpy(name, "SUB32");

      if (!strcasecmp(ixfModule->Fixups[imports_counter-1].ImportEntry.ModuleName, prev->mod_name))
      {
        if(prev->load_status == LOADING)
        {
          *phmod=NULL;
          return; // 5/*ERROR_ACCESS_DENIED*/; // @todo Need more accurate code
        }
        break;
      }
      prev = (struct module_rec *) prev->next;
    }
    // @todo use handles here
    *phmod=(unsigned long)prev->module_struct;
    LOG("%s.%d", ixfModule->Fixups[imports_counter-1].ImportEntry.FunctionName, ixfModule->Fixups[imports_counter-1].ImportEntry.Ordinal);
    ModQueryProcAddr(*phmod,
                     ixfModule->Fixups[imports_counter-1].ImportEntry.Ordinal,
                     ixfModule->Fixups[imports_counter-1].ImportEntry.FunctionName,
                     &(ixfModule->Fixups[imports_counter-1].ImportEntry.Address));

    if (options.debugmodmgr)
    {
      //LOG("src=%x, dst=%x\n",(ixfModule->Fixups[imports_counter-1].SrcAddress) , (ixfModule->Fixups[imports_counter-1].ImportEntry.Address));
    }

    /* Is the EXE module placed under the DLL module in memory? */
    if((ixfModule->Fixups[imports_counter-1].SrcAddress) < (ixfModule->Fixups[imports_counter-1].ImportEntry.Address))
      relative_jmp = (unsigned long)(ixfModule->Fixups[imports_counter-1].ImportEntry.Address) - (unsigned long)(ixfModule->Fixups[imports_counter-1].SrcAddress)-4;
    else
      relative_jmp = 0xffffffff-((unsigned long)(ixfModule->Fixups[imports_counter-1].SrcAddress) - (unsigned long)(ixfModule->Fixups[imports_counter-1].ImportEntry.Address))-3;

    LOG("jmp=%x=%x", (unsigned long)(ixfModule->Fixups[imports_counter-1].SrcAddress), relative_jmp);
    *((unsigned long *) ixfModule->Fixups[imports_counter-1].SrcVmAddress) = relative_jmp;
  }
}

