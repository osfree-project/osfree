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

#ifndef _DYNLINK_H_
#define _DYNLINK_H_

#include <processlx.h>

#ifdef __cplusplus
extern "C" {
#endif

const int LOADING = 1;
const int DONE_LOADING = 0; 

struct module_rec {
	char * mod_name;      /* Name of module. */
	void * module_struct; /* Pointer to LX_module. */
	void *next;	          /* Next element or NULL at end. */
	int load_status;      /* Status variable to check for recursion in loading state. 
	                         A one means it is being loaded and zero it is done. */
};

void init_dynlink(void);
struct module_rec * register_module(char * name, void * mod_struct);
void * find_module(char * name, struct t_processlx *proc);
void load_dyn_link(char * name);
void * get_func_ptr_ord_modname(int ord, char * modname);
void * load_module(char * name, struct t_processlx *proc);
struct module_rec * get_root();
struct module_rec * get_next(struct module_rec * el);
char * get_name(struct module_rec * el);
struct LX_module * get_module(struct module_rec * el);
void print_module_table();
void set_libpath(char ** path, int nr);
char ** get_libpath();

#ifdef __cplusplus
};
#endif

#endif
