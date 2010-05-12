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

#include <l4/log/l4log.h>

#include <token.h>
#include <io.h>
#include <modmgr.h>
#include <ixfmgr.h>
#include <cfgparser.h>
#include <os2errcodes.h>
#include "apistub.h"
#include <MountReg.h>


unsigned int find_module_path(const char * name, char * full_path_name);

struct module_rec module_root; /* Root for module list.*/

extern PVOID entry_Table[];

/* Some general ideas here:
     We use static array of modules structures here. We limit now to 1024 module handles.
     Such approach is much protected in comparation of allowing direct access to module structure.

@todo: Add option MAXLOADEDMODULES=xxxx to config.sys for more flexability?
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

unsigned long ModLoadModule(char *          pszName,
                            unsigned long   cbName,
                            char const *    pszModname,
                            unsigned long * phmod)
{
  struct module_rec * new_module_el;
  #define buf_size 4096
  char buf[buf_size+1];
  UCHAR chLoadError[CCHMAXPATH]; /* Error info from ModLoadModule */
  char *p_buf = (char *) &buf;
  void * addr;
  unsigned long size;
  unsigned long rc=NO_ERROR;
  IXFModule *ixfModule;
  void *ptr_mod;
  struct module_rec * prev;
  unsigned long module_counter;
  unsigned long imports_counter;
  unsigned long hmod;
  int relative_jmp;


  // Check input arguments
  if ((phmod==NULL)||
      (pszModname==NULL)||
      (pszName==NULL)) return 87 /*ERROR_INVALID_PARAMETER*/;

  // Initialize return vars
  *pszName=0;
  *phmod=0;

  // @todo extract filename only because can be fullname with path
  
  char * mname=get_fname(pszModname);

  // Specail case - EMXWRAP.DLL. Read more in docs\os2\sub32.txt
  if (!strcasecmp(mname, "EMXWRAP"))
  {
    if (options.debugmodmgr) LOG("ModLoadModule: EXMWRAP module replaced by SUB32 module.\n");
    mname="SUB32";
    pszModname="SUB32.DLL";
  }

  if (options.debugmodmgr) LOG("ModLoadModule: Loading module %s...\n", mname);


  // First search in the module list
  prev = (struct module_rec *) module_root.next;
  while(prev)
  {
    if(strcasecmp(mname, prev->mod_name)==0)
    {
      if(prev->load_status == LOADING)
      {
        *phmod=NULL;
        if (cbName<=strlen(mname)) return 8 /*ERROR_NOT_ENOUGH_MEMORY*/;
        strcpy(pszName, mname);
        return 5 /*ERROR_ACCESS_DENIED*/; // @todo Need more accurate code
      }
      // @todo use handles here
      *phmod=(unsigned long)prev->module_struct;
      LOG("loaded.");
      return 0/*NO_ERROR*/;
    }
    prev = (struct module_rec *) prev->next;
  }

  // Ok. No module found. Try to load file
  // Consider fully-qualified name specified.
  rc=io_load_file(pszModname, &addr, &size);
  if (rc)
  {
    // Searches for module name and returns the full path in the buffer p_buf.
    rc=find_module_path(pszModname, p_buf);
    if (!rc) rc=io_load_file(p_buf, &addr, &size);
  }
  if (rc)
  {
    strcpy(pszName, pszModname);
    *phmod=NULL;
    return rc;
  }

  ixfModule = (IXFModule *) malloc(sizeof(IXFModule));

  rc=IXFIdentifyModule(addr, size, ixfModule);
  if (rc)
  {
    strcpy(pszName, pszModname);
    *phmod=NULL;
    free(ixfModule);
    return rc;
  }

  // Load module
  rc=IXFLoadModule(addr, size, ixfModule);
  if (rc)
  {
    strcpy(pszName, pszModname);
    *phmod=NULL;
    free(ixfModule);
    return rc;
  }

  // Display module entry table
  if (options.debugixfmgr)
  {
    unsigned long entries_counter;
    io_printf("Module entry table\n");
    io_printf("------------------\n");
    for (entries_counter=1;
         entries_counter<ixfModule->cbEntries+1;
         entries_counter++)
    {
      if (ixfModule->Entries[entries_counter-1].ModuleName)
      {
        io_printf("%s.%d\n", ixfModule->Entries[entries_counter-1].ModuleName, ixfModule->Entries[entries_counter-1].Ordinal);
      } else {
        io_printf("%x\n", ixfModule->Entries[entries_counter-1].Address);
      }
    }

  }

  // Register in module list
  // @todo extract filename only because can be fullname with path
  new_module_el = ModRegister(mname, ixfModule);
  new_module_el->load_status = LOADING;

  // Load modules which not loaded yet
  for (module_counter=1;
       module_counter<ixfModule->cbModules+1;
       module_counter++)
  {
    char  name[1024];//=malloc(strlen(ixfModule->Modules[module_counter-1]+5));
    strcpy(name, ixfModule->Modules[module_counter-1]);
    strcat(name, ".dll");
    rc=ModLoadModule(chLoadError, sizeof(chLoadError),
                              name, (unsigned long *)&hmod);
                              LOG("%s loaded",name);
  }

  // Fixup module
  rc=IXFFixupModule(ixfModule);
  if (rc)
  {
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


  // Link module (import table resolving)
  for (imports_counter=1;
       imports_counter<ixfModule->cbFixups+1;
       imports_counter++)
  {
    LOG("Import entrie %d of %d",imports_counter, ixfModule->cbFixups);
    LOG("Module=%s", ixfModule->Fixups[imports_counter-1].ImportEntry.ModuleName);
    
    prev = (struct module_rec *) module_root.next;
    while(prev)
    {
      if(strcasecmp(ixfModule->Fixups[imports_counter-1].ImportEntry.ModuleName, prev->mod_name)==0)
      {
        if(prev->load_status == LOADING)
        {
          *phmod=NULL;
          return 5/*ERROR_ACCESS_DENIED*/; // @todo Need more accurate code
        }
        // @todo use handles here
        *phmod=(unsigned long)prev->module_struct;
        break;
      }
      prev = (struct module_rec *) prev->next;
    }

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
    {
      relative_jmp = (int)(ixfModule->Fixups[imports_counter-1].ImportEntry.Address) - (int)(ixfModule->Fixups[imports_counter-1].SrcAddress)-4;
    } else {
      relative_jmp = 0xffffffff-((int)(ixfModule->Fixups[imports_counter-1].SrcAddress) - (int)(ixfModule->Fixups[imports_counter-1].ImportEntry.Address))-3;
    }
    LOG("jmp=%x=%x", (int)(ixfModule->Fixups[imports_counter-1].SrcAddress), relative_jmp);
    *((int *) ixfModule->Fixups[imports_counter-1].SrcAddress) = relative_jmp;

  }
  //@todo use handle table
  *phmod=(unsigned long)ixfModule;


  new_module_el->load_status = DONE_LOADING;


  return rc; /*NO_ERROR;*/
}

/* @brief Initializes the root node in the linked list, which itself
   is not used. Only to make sure the list at least always has one
   element allocated. Registers fake KAL.DLL which contains host
   specific functions. */
unsigned long ModInitialize(void)
{
  IXFModule * ixf;
  struct module_rec * new_module_el;

  // Create root node

  module_root.mod_name = "root";
  module_root.module_struct = 0;
  module_root.next = 0;

  // Register KAL.DLL

  ixf=malloc(sizeof(IXFModule));

  ixf->Load=NULL;
  ixf->Fixup=NULL;
  ixf->FormatStruct=NULL;
  ixf->cbEntries=4;
  ixf->Entries=malloc(sizeof(IXFMODULEENTRY)*ixf->cbEntries);
  ixf->Entries[0].FunctionName="1KalWrite";
  ixf->Entries[0].Address=&api_DosWrite;
  ixf->Entries[0].ModuleName=NULL;
  ixf->Entries[0].Ordinal=0;
  ixf->Entries[1].FunctionName="2KalWrite";
  ixf->Entries[1].Address=&api_DosWrite;
  ixf->Entries[1].ModuleName=NULL;
  ixf->Entries[1].Ordinal=0;
  ixf->Entries[2].FunctionName="KalWrite";
  ixf->Entries[2].Address=&api_DosWrite;
  ixf->Entries[2].ModuleName=NULL;
  ixf->Entries[2].Ordinal=0;
  ixf->Entries[3].FunctionName="KalExit";
  ixf->Entries[3].Address=&api_DosExit;
  ixf->Entries[3].ModuleName=NULL;
  ixf->Entries[3].Ordinal=0;
  ixf->cbModules=0;
  ixf->Modules=NULL;
  ixf->cbFixups=0;
  ixf->Stack=NULL;
  ixf->EntryPoint=NULL;
  ixf->PIC=0;

  new_module_el = ModRegister("KAL", ixf);
  new_module_el->load_status = DONE_LOADING;

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

/* Register a module with the name. */
struct module_rec * ModRegister(const char * name, void * mod_struct)
{
  struct module_rec * new_mod;
  struct module_rec * prev;

  prev = &module_root;

  while(prev->next) /* Find free node at end. */
          prev = (struct module_rec *) prev->next;

  new_mod = (struct module_rec *) malloc(sizeof(struct module_rec));
  prev->next = new_mod;  /*struct module_rec module_struct*/

  new_mod->mod_name = (char *)malloc(strlen(name)+1);
  strcpy(new_mod->mod_name, name);
  new_mod->module_struct = mod_struct; /* A pointer to struct LX_module. */
  new_mod->next=NULL;
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

// This function searches LIBPATH option. Only OS/2 path formats are supported.

unsigned int find_module_path(const char * name, char * full_path_name)
{
  FILE *f;
  char *p = options.libpath - 1;
  STR_SAVED_TOKENS st;
  char * p_buf = full_path_name;
  char *sep="\\";
#ifdef __LINUX__
  char *hostsep="/";
#else
  char *hostsep="\\";
#endif
  char *psep=";";

  StrTokSave(&st);
  if((p = StrTokenize((char*)options.libpath, psep)) != 0)
  do if(*p)
  {
    int B_LEN = 250;
    char buf[251];
    char *str_buf=(char*) &buf;
    char buf2[251];
    char * file_to_find=(char*) &buf2;
    long i=0;

    p_buf = full_path_name;
    p_buf[0] = '\0';
    while (p[i]!='\0')
    {
      if (p[i]=='\\') p[i]=*hostsep; ++i;
    }

    strcat(p_buf, p);
    if (p[strlen(p)-1]!=*hostsep)
    {
      strcat(p_buf, hostsep);
    }
    
    strcat(p_buf, name);

    buf[0] = '\0';
    buf2[0] = '\0';
    strcat(file_to_find, name);
    #ifndef L4API_l4v2
      if(find_case_file(file_to_find, p, str_buf, B_LEN))
      {
        p_buf[0] = 0;
        strcat(p_buf, p);
        strcat(p_buf, hostsep);
        strcat(p_buf, str_buf); /* Case corrected for file, Needed on Linux. */
      }
    #endif

    LOG(p_buf);
    LOG(os2_fname_to_vfs_fname(p_buf));
    
    f = fopen(os2_fname_to_vfs_fname(p_buf), "rb"); /* Tries to open the file, if it works f is a valid pointer.*/
    if(f)
    {
      StrTokStop();
      return NO_ERROR;
    }
  } while((p = StrTokenize(0, psep)) != 0);
  StrTokRestore(&st);

  return 2 /*ERROR_FILE_NOT_FOUND*/;
}


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
      obj_vm_base =  get_obj(found_module, ret_obj)->o32_base;
      i_dll_func_offs = obj_vm_base + ret_offset;
    } else {
      obj_vm_base     =  (unsigned long int)frw_fn_ptr;
      i_dll_func_offs =  (unsigned long int)frw_fn_ptr;
    }

    i_offs_to_fix_in_exe = lx_obj->o32_base + srcoff_cnt1;

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


  if (ordinal!=0)
  { // Search by ordinal
    if (ixfModule->Entries[ordinal-1].ModuleName!=NULL)
    { // This is forward to another module. Call ourself...
      // First search in the module list
      prev = (struct module_rec *) module_root.next;
      while(prev)
      {
        if(strcasecmp(ixfModule->Entries[ordinal-1].ModuleName, prev->mod_name)==0)
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
                                  ixfModule->Entries[ordinal-1].Ordinal,
                                  ixfModule->Entries[ordinal-1].FunctionName,
                                  ppfn);
    }


    *ppfn=ixfModule->Entries[ordinal-1].Address;
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

      *ppfn=ixfModule->Entries[entries_counter-1].Address;
      return 0;
    }
  }
}
