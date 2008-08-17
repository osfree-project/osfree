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
#include "cfgparser.h"

struct module_rec module_root; /* Root for module list.*/


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


#if 0
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

#endif
