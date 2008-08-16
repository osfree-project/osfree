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
#include <loadobjlx.h>

#include "dynlink.h"
#include "io.h"

struct module_rec module_root; /* Root for module list.*/

char **mini_libpath;
int sz_mini_libpath; /* Number of elements in mini_libpath (zero-based).*/

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

        io_printf("register_module: %s, %p \n", name, mod_struct);
        new_mod = (struct module_rec *) malloc(sizeof(struct module_rec));
        new_mod->mod_name = (char *)malloc(strlen(name)+1);
        strcpy(new_mod->mod_name, name);
        prev = &module_root;

        while(prev->next) /* Find free node at end. */
                prev = (struct module_rec *) prev->next;

        prev->next = new_mod;  /*struct module_rec module_struct*/
        new_mod->module_struct = mod_struct; /* A pointer to struct LX_module. */
        return new_mod;
}

        /* Searches for the module name which the process proc needs.
           It first sees if it's already loaded and then just returns the found module.
           If it can't be found load_module() searches the mini_libpath inside find_module_path(). */
void * find_module(char * name, struct t_processlx *proc) {
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

        ptr_mod = load_module(name, proc);
        if(ptr_mod != 0) { /* If the module has been loaded, register it. */
                /* register_module(name, ptr_mod); */
                return ptr_mod;
        }

        return 0;
}


        /* Searches a module in the mini_libpath. */
void find_module_path(char * name, char * full_path_name) {

        char * lcase_name;
        int i;
        FILE *f;
        const char *dll_suf = ".dll";  /* DLL extension. */
        char *sep = "/";               /* Dir separator for GNU/Linux,  */

        char * p_buf = full_path_name;

        char buf_lcase_name[300];

        strcpy((char*)&buf_lcase_name, name);
        lcase((char*)&buf_lcase_name, name);
        lcase_name = (char*)&buf_lcase_name;
        i =0;
        f=0;
        do {
                p_buf = full_path_name;
                p_buf[0] = 0;
                strcat(p_buf, mini_libpath[i]);
                strcat(p_buf, sep);
                strcat(p_buf, name);
                strcat(p_buf, dll_suf);
                f = fopen(p_buf, "rb"); /* Tries to open the file, if it works f is a valid pointer.*/
                if(!f) {
                        p_buf[0] = 0;
                        strcat(p_buf, mini_libpath[i]);
                        strcat(p_buf, sep);
                        strcat(p_buf, lcase_name);
                        strcat(p_buf, dll_suf);
                        f = fopen(p_buf, "rb"); /* Tries to open the file, if it works f is a valid pointer.*/
                }
                ++i;
        }while(!f && (i <= sz_mini_libpath));
        if(f)
                fclose(f);
        else
                p_buf[0] = 0;
}

        /* Loads a module name which proc needs. */
void * load_module(char * name, struct t_processlx *proc) {
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
                load_dll_code_obj_lx(lx_exe_mod, proc); /* Load all objects in dll.*/
                new_module_el = register_module(name, lx_exe_mod);
                new_module_el->load_status = LOADING;
                        /* A risk for cycles here. Need to check if a dll is already loading,
                           indirect recursion.*/
                do_fixup_code_data_lx(lx_exe_mod, proc); /* Apply fixups. */
                new_module_el->load_status = DONE_LOADING;
                return lx_exe_mod;
        }
        free(lx_exe_mod);
        if(f)
                fclose(f);
        io_printf("load_module: Load error!!! of %s in %s\n", name, p_buf);
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

/*void load_dyn_link(char * name) {

        io_printf("Laddar dll: ");

        //DosPutMessage(1, name[0], name);
        io_printf("\n");
}
*/

/* MSG.DLL med DosPutMessage med ordinal: 5*/
/* DOSCALLS.234  Dos32Exit */
/* DOSCALLS.348   Dos32QuerySysInfo */
/* strcasecmp  strcmp*/

/*void * get_func_ptr_ord_modname(int ord, char * modname) {
        if(strcasecmp (modname, "MSG")==0 && ord == 5)
                return 0 ;//&DosPutMessage;
        if(strcasecmp(modname, "DOSCALLS")==0 && ord == 234)
                return 0 ;//&DosExit;
        if(strcasecmp(modname, "DOSCALLS")==0 && ord == 348)
                return 0 ;//&DosQuerySysInfo;

        io_printf("DL: Can't find ordinal for function! ord:%d mod:%s \n",ord,modname);
        return 0;
}
*/

void set_libpath(char ** path, int nr) {
  mini_libpath = path;
  sz_mini_libpath = nr;
}

char ** get_libpath() {
  return mini_libpath;
}

