/*
    LXLoader - Loads LX exe files or DLLs for execution or to extract information from.
    Copyright (C) 2007  Sven Rosén (aka Viking)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <processlx.h>

#ifndef _NATIVE_DYNLINK_H_
#define _NATIVE_DYNLINK_H_



#ifdef __cplusplus
extern "C" {
#endif

 

struct native_module_rec {
	char * file_name_path; 
	char * file_name;
	char * mod_name;      /* Name of module. */
	void * module_struct; /* Pointer to LX_module. */
	void *next;	          /* Next element or NULL at end. */
	int load_status;      /* Status variable to check for recursion in loading state. 
	                         A one means it is being loaded and zero it is done. */
};

void init_native_dynlink(void);
struct native_module_rec * native_register_module(char * name, char * filepath, void * mod_struct);
void * native_find_module(char * name, struct t_processlx *proc);
void native_find_module_path(char * name, char * full_path_name);
void native_load_dyn_link(char * name);
void * native_get_func_ptr_ord_modname(int ord, char * modname);
void * native_get_func_ptr_str_modname(char * funcname, char * modname);
void * native_get_func_ptr_handle_modname(char * funcname, void * native_mod_handle);
void * native_load_module(char * name, struct t_processlx *proc);
struct native_module_rec * native_get_root();
struct native_module_rec * native_get_next(struct native_module_rec * el);
char * native_get_name(struct native_module_rec * el);
struct LX_module * native_get_module(struct native_module_rec * el);
void native_print_module_table(void);
void set_native_libpath(char ** path, int nr);
char ** get_native_libpath();

/*
DosFreeModule(hk_module);       // Decrement the DLL usage count
     if (!hk_module) {
             if (DosLoadModule(NULL,0,"X11pmhk.dll",&hk_module))
                     DosQueryProcAddr(hk_module, 0, "Xlib_NewGrab", (PFN *)&Xlib_NewGrab);


*/

#ifdef __cplusplus
};
#endif

#endif
