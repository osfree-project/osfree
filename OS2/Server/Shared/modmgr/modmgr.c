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

//#include <modlx.h>
#include "io.h"
//#include "execlx.h"
//#include "loadobjlx.h"
#include "modmgr.h"
#include <ixfmgr.h>
#include "cfgparser.h"
#include "native_dynlink.h"

unsigned int find_module_path(char * name, char * full_path_name);

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
unsigned long ModLoadModule(const char *    pszName,
                            unsigned long   cbName,
                            const char *    pszModname,
                            unsigned long * phmod)
{
  struct module_rec * new_module_el;
  #define buf_size 4096
  char buf[buf_size+1];
  char *p_buf = (char *) &buf;
  void * addr;
  unsigned long size;
  unsigned long rc;
  IXFModule *ixfModule;
  void *ptr_mod;
  struct module_rec * prev;

  // Check input arguments
  if ((phmod==NULL)||(pszModname==NULL)) return ERROR_INVALID_PARAMETER;

  // @todo extract filename only because can be fullname with path

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
      *phmod=prev->module_struct;
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
  if (rc) return rc;

  ixfModule = (IXFModule *) malloc(sizeof(IXFModule));

  rc=IXFIdentifyModule(addr, size, ixfModule);
  if (rc)
  {
    free(ixfModule);
    return rc;
  }

  // Load module
  rc=IXFLoadModule(addr, size, ixfModule);
  if (rc)
  {
    free(ixfModule);
    return rc;
  }

  // Register in module list
  // @todo extract filename only because can be fullname with path
  new_module_el = register_module(pszModname, ixfModule);
  new_module_el->load_status = LOADING;

  // Fixup module
  rc=IXFFixupModule(ixfModule);
  if (rc)
  {
    free(ixfModule);
    return rc;
  }

  new_module_el->load_status = DONE_LOADING;

  //@todo use handle table
  *phmod=(unsigned long)ixfModule;

  return NO_ERROR;
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
register_module(char * name, void * mod_struct)
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


struct DFN_Search_Request {
        const char *basename;                   /* search for <<path>>\basename.* */
        const char *extensions;
        char *foundMatch;
        int flags;
        char delim;
};


struct STR_SAVED_TOKENS_ {
        char *str_next_saved_tokens;
        char str_ch_saved_tokens;
};

typedef struct STR_SAVED_TOKENS_ STR_SAVED_TOKENS;


static char *nxtToken = 0;              /* pointer to previous scanned string */
static char ch;                                         /* previous token delimiter */

char *StrTokenize(char *str, const char * const token)
{

        if(!str) {
                if((str = nxtToken) == 0                /* nothing to do */
                 || (*str++ = ch) == 0)               /* end of string reached */
                        return( nxtToken = 0);
        }

        if(!token || !*token)                   /* assume all the string */
                nxtToken = 0;
        else {
                nxtToken = str - 1;
                while(!strchr(token, *++nxtToken));
                ch = *nxtToken;
                *nxtToken = 0;
        }

        return( str);
}

void StrTokSave(STR_SAVED_TOKENS *st)
{

        if(st) {
                st->str_next_saved_tokens = nxtToken;
                st->str_ch_saved_tokens = ch;
        }

}

void StrTokRestore(STR_SAVED_TOKENS *st)
{

        if(st) {
                nxtToken = st->str_next_saved_tokens;
                ch = st->str_ch_saved_tokens;
        }

}


#define StrTokStop() (void)StrTokenize(0, 0)


unsigned int find_module_path(char * name, char * full_path_name)
{
        FILE *f;
        char *p = options.libpath - 1;
        STR_SAVED_TOKENS st;
        char * p_buf = full_path_name;


                        StrTokSave(&st);
                        if((p = StrTokenize((char*)options.libpath, ";")) != 0) do if(*p)
                              {
                                p_buf = full_path_name;
                                p_buf[0] = 0;
                                strcat(p_buf, p);
                                strcat(p_buf, "\\");
                                strcat(p_buf, name);
                                strcat(p_buf, ".dll");
                                f = fopen(p_buf, "rb"); /* Tries to open the file, if it works f is a valid pointer.*/
                                if(f)
                                {
                                  StrTokStop();
                                  return NO_ERROR;
                                }
                        } while((p = StrTokenize(0, ";")) != 0);
                        StrTokRestore(&st);

  return ERROR_FILE_NOT_FOUND;
}

/* Goes through every loaded module and prints out all it's objects. */
void print_detailed_module_table() {
     int num_objects;
     int i;
        struct module_rec * el = &module_root;
        io_printf("--- Detailed Loaded Module Table ---\n");
        while((el = el->next)) {
                io_printf("module = %s, module_struct = %p, load_status = %d\n",
                                el->mod_name, el->module_struct, el->load_status);
                num_objects = get_obj_num(el->module_struct);
                i=0;
                for(i=1; i<=num_objects; i++) {
                        struct o32_obj * an_obj = get_obj(el->module_struct, i);
                        print_o32_obj_info(*an_obj, el->mod_name);
                }
        }
}

void print_module_table() {
        struct module_rec * el = &module_root;
        io_printf("--- Loaded Module Table ---\n");
        while((el = el->next)) {
                io_printf("module = %s, module_struct = %p, load_status = %d\n",
                                el->mod_name, el->module_struct, el->load_status);
        }
}
