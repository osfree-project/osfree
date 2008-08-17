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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <modlx.h>
#include "io.h"
#include "execlx.h"
#include "loadobjlx.h"
#include "modmgr.h"
#include "cfgparser.h"
#include "dynlink.h"
#include "native_dynlink.h"

struct module_rec module_root; /* Root for module list.*/


#if 0
typedef
  struct
  {
    void * FormatStruct;
  } IXFModule;

unsigned long IXFLoadModule(void * addr, unsigned long size, IXFModule * ifxModule)
{
  struct LX_module *lx_exe_mod;

  lx_exe_mod = (struct LX_module *) malloc(sizeof(struct LX_module));

  ixfModule->FormatStruct=(void *)lx_exe_mod;

  if(load_lx_stream((char*)addr, size, lx_exe_mod)) /* A file from a buffer.*/
  {
    load_dll_code_obj_lx(lx_exe_mod); /* Load all objects in dll.*/
  }
}

unsigned long IXFFixupModule(IXFModule module)
{
  do_fixup_code_data_lx((struct LX_module *)ixfModule->FormatStruct);
  /* Apply fixups. */
}

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

  find_module_path(name, p_buf); /* Searches for module name and returns the full path in
                           the buffer p_buf. */

  rc=io_load_file(p_buf, &addr, &size);

  // Load module
  rc=IXFLoadModule(addr, size, &ixfModule);

  // Register in module list
  new_module_el = register_module(name, ixfModule);
  new_module_el->load_status = LOADING;

  // Fixup module
  rc=IXFFixupModule(ixfModule);

  new_module_el->load_status = DONE_LOADING;
  return lx_exe_mod;
}

#endif

        /* Loads a module name which proc needs. */
void * load_module(char * name) {
        struct LX_module *lx_exe_mod;
        FILE *f;
        struct module_rec * new_module_el;
        #define buf_size 4096
        char buf[buf_size+1];
        char *p_buf = (char *) &buf;

        find_module_path(name, p_buf); /* Searches for module name and returns the full path in
                                                                        the buffer p_buf. */

        lx_exe_mod = (struct LX_module *) malloc(sizeof(struct LX_module));
        io_printf("load_module: '%s' \n", p_buf);
        f = fopen(p_buf, "rb");  /* Open file in read only binary mode, in case this code
                                          will be compiled on OS/2 or on windows. */

        /* Load LX file from buffer. */
        /* if(load_lx_stream((char*)lx_buf, pos, &lx_exe_mod)) { */


        /* Load LX file from ordinary disk file. */
        if(f && load_lx(f, lx_exe_mod)) {     /* Load LX header.*/
                load_dll_code_obj_lx(lx_exe_mod); /* Load all objects in dll.*/
                new_module_el = register_module(name, lx_exe_mod);
                new_module_el->load_status = LOADING;
                        /* A risk for cycles here. Need to check if a dll is already loading,
                           indirect recursion.*/
                do_fixup_code_data_lx(lx_exe_mod); /* Apply fixups. */
                new_module_el->load_status = DONE_LOADING;
                return lx_exe_mod;
        }
        free(lx_exe_mod);
        if(f)
                fclose(f);
        io_printf("load_module: Load error!!! of %s in %s\n", name, p_buf);
        return 0;
}


  /* Initializes the root node in the linked list, which itself is not used.
  Only to make sure the list at least always has one element allocated. */
void init_dynlink(void) {
        module_root.mod_name = "root";
        module_root.module_struct = 0;
        module_root.next = 0;
}

#ifdef SDIOS
#include <ctype.h>
int strcasecmp(const char* dest, const char* src)
{
        while(*dest != 0 && toupper(*src) == toupper(*dest)) {
                dest++;
                src++;
        }

        return *dest - *src;
}
#endif

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

        /* Searches for the module name which the process proc needs.
           It first sees if it's already loaded and then just returns the found module.
           If it can't be found load_module() searches the mini_libpath inside find_module_path(). */
void * find_module(char * name) {
     void *ptr_mod;
        struct module_rec * prev = (struct module_rec *) module_root.next;

        while(prev) {
                io_printf("find_module: %s == %s, mod=%p \n", name, prev->mod_name, prev->module_struct);
                if(strcmp(name, prev->mod_name)==0) {
                        io_printf("ret find_module: %p\n", prev->module_struct);

                        if(prev->load_status == LOADING) {
                                io_printf("find_module: ERROR, Cycle in loading of %s\n",name);
                                return 0;
                        }
                        return prev->module_struct;
                }
                prev = (struct module_rec *) prev->next;
        }

        ptr_mod = load_module(name);
        if(ptr_mod != 0) { /* If the module has been loaded, register it. */
                /* register_module(name, ptr_mod); */
                return ptr_mod;
        }

        return 0;
}



struct module_rec * get_root() {
        return &module_root;
}

struct module_rec * get_next(struct module_rec * el) {
        if(el != 0)
                return (struct module_rec *) el->next;
        else
                return 0;
}

char * get_name(struct module_rec * el) {
        if(el != 0)
                return el->mod_name;
        else
                return 0;
}

struct LX_module * get_module(struct module_rec * el) {
        if(el != 0)
                return (struct LX_module *) el->module_struct;
        else
                return 0;
}

void print_module_table() {
        struct module_rec * el = get_root();
        io_printf("--- Loaded Module Table ---\n");
        while((el = get_next(el))) {
                io_printf("module = %s, module_struct = %p, load_status = %d\n",
                                el->mod_name, el->module_struct, el->load_status);
        }
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


void find_module_path(char * name, char * full_path_name)
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
                                  break;
                                }
                        } while((p = StrTokenize(0, ";")) != 0);
                        StrTokRestore(&st);

}
