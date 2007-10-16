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

#ifndef _LOADOBJLX_H_
#define _LOADOBJLX_H_
//#define FALSE 1
#include <modlx.h>
#include <processlx.h>
#include "gcc_os2def.h"
#include <exe386.h>


#ifdef __cplusplus
extern "C" {
#endif

int load_code_data_obj_lx(struct LX_module * lx_exe_mod, struct t_processlx * proc);

int load_dll_code_obj_lx(struct LX_module * lx_exe_mod, struct t_processlx * proc);

unsigned int vm_alloc_obj_lx(struct LX_module * lx_exe_mod, struct o32_obj * lx_obj);

int load_obj_lx(struct LX_module * lx_exe_mod, struct t_processlx * proc, 
				struct o32_obj * lx_obj, void *vm_ptr_obj);
				
int do_fixup_code_data_lx(struct LX_module * lx_exe_mod, struct t_processlx * proc);

int do_fixup_obj_lx(struct LX_module * lx_exe_mod, struct t_processlx * proc, 
				struct o32_obj * lx_obj);

int apply_import_fixup(struct LX_module * this_module, struct LX_module * found_module, 
						struct t_processlx * proc, struct o32_obj * lx_obj,
						int mod_nr, int import_ord, int addit, int srcoff_cnt1,
						struct r32_rlc * min_rlc);
						
const int ENTRYNATIVE = 5;

#ifdef __cplusplus
};
#endif

#endif
