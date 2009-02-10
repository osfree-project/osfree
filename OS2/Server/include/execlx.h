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

#ifndef EXECLX_H
#define EXECLX_H

#include "processmgr.h"
#include "modlx.h"


#ifdef __cplusplus
extern "C" {
#endif

void exec_lx(struct LX_module * lx_exe_mod, struct t_os2process * proc);
void l4_exec_lx(struct LX_module * lx_exe_mod, struct t_os2process * proc);

int test_elfexecf(const char* path);

#ifdef __cplusplus
};
#endif


#endif
