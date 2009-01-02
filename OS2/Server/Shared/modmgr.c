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

#define INCL_ERRORS
#include <os2.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <token.h>
#include <io.h>
#include <modmgr.h>
#include <ixfmgr.h>
#include <cfgparser.h>
#if defined(__WIN32__) || defined(__LINUX__)
#include <os2/bseerr.h>
#endif

unsigned int find_module_path(const char * name, char * full_path_name);

struct module_rec module_root; /* Root for module list.*/

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
  char *p_buf = (char *) &buf;
  void * addr;
  unsigned long size;
  unsigned long rc=NO_ERROR;
  IXFModule *ixfModule;
  void *ptr_mod;
  struct module_rec * prev;

  // Check input arguments
  if ((phmod==NULL)||
      (pszModname==NULL)||
      (pszName==NULL)) return ERROR_INVALID_PARAMETER;

  // Initialize return vars
  *pszName=0;
  *phmod=0;

  // @todo extract filename only because can be fullname with path

  // Specail case - KAL.DLL
  #ifndef __LINUX__
  /* A test under Linux with a specially compiled kal.dll (LX format) but with 
     Open Watcoms C library for Linux. */

  if (!strcmp(pszModname, "KAL"))
  {
    return 1; // KAL module always has handle 1
  }
  #endif
  // First search in the module list
  prev = (struct module_rec *) module_root.next;
  while(prev)
  {
    if(strcmp(pszModname, prev->mod_name)==0)
    {
      if(prev->load_status == LOADING)
      {
        *phmod=NULL;
        if (cbName<=strlen(pszModname)) return ERROR_NOT_ENOUGH_MEMORY;
        strcpy(pszName, pszModname);
        return ERROR_ACCESS_DENIED; // @todo Need more accurate code
      }
      // @todo use handles here
      *phmod=(unsigned long)prev->module_struct;
      return NO_ERROR;
    }
    prev = (struct module_rec *) prev->next;
  }

  // Ok. No module found. Try to load file
  // (consider fully-qualified name specified)
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

  // Register in module list
  // @todo extract filename only because can be fullname with path
  new_module_el = register_module(pszModname, ixfModule);
  new_module_el->load_status = LOADING;

  // Load modules which not loaded yet


  // Fixup module
  rc=IXFFixupModule(ixfModule);
  if (rc)
  {
    strcpy(pszName, pszModname);
    *phmod=NULL;
    free(ixfModule);
    return rc;
  }

  new_module_el->load_status = DONE_LOADING;

  //@todo use handle table
  *phmod=(unsigned long)ixfModule;

  return rc; /*NO_ERROR;*/
}

/* Initializes the root node in the linked list, which itself is not used.
Only to make sure the list at least always has one element allocated. */
unsigned long ModInitialize(void)
{
  module_root.mod_name = "root";
  module_root.module_struct = 0;
  module_root.next = 0;
  return NO_ERROR;
}

int lcase(char* dest, const char* src)
{
        while(*dest != 0 /*&& toupper(*src) == toupper(*dest)*/) {
            *dest = tolower(*src);
                dest++;
                src++;
        }

        return *dest - *src;
}

        /* Register a module with the name. */
struct module_rec *
register_module(const char * name, void * mod_struct)
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
#include <direct.h>
#endif
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


unsigned int find_module_path(const char * name, char * full_path_name)
{
  FILE *f;
  char *p = options.libpath - 1;
  STR_SAVED_TOKENS st;
  char * p_buf = full_path_name;
  #ifndef __LINUX__
  char *sep="\\";
  char *psep=";";
  #else
  char *sep="/";
  char *psep=";";
  #endif

  StrTokSave(&st);
  if((p = StrTokenize((char*)options.libpath, psep)) != 0) do if(*p)
  {
    p_buf = full_path_name;
    p_buf[0] = '\0';
    strcat(p_buf, p);
    strcat(p_buf, sep);
    
    strcat(p_buf, name);
    strcat(p_buf, ".dll");
    printf("%s:find_module_path(), %s\n", __FILE__, p_buf);

    int B_LEN = 250;
    char buf[251];
    char *str_buf=(char*) &buf;
    char buf2[251];
    char * file_to_find=(char*) &buf2;
    buf[0] = '\0';
    buf2[0] = '\0';
    strcat(file_to_find, name);
    strcat(file_to_find, ".dll");
    /*printf("find_case_file(), %s, %s\n", file_to_find, p);*/
    if(find_case_file(file_to_find, p, str_buf, B_LEN)) {
        printf("Found file: %s (%s) in %s\n", file_to_find, str_buf, p);
        p_buf[0] = 0;
        strcat(p_buf, p);
        strcat(p_buf, sep);
        strcat(p_buf, str_buf); /* Case corrected for file, Needed on Linux. */
    }
    
    f = fopen(p_buf, "rb"); /* Tries to open the file, if it works f is a valid pointer.*/
    if(f)
    {
      StrTokStop();
      return NO_ERROR;
    }
  } while((p = StrTokenize(0, psep)) != 0);
  StrTokRestore(&st);

  return ERROR_FILE_NOT_FOUND;
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
