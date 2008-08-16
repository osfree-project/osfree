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

#include <stdlib.h>
#include <sys/mman.h>
#define INCL_DOSPROCESS
#include <processlx.h>
#include <modlx.h>
#include <io.h>


//#define INCL_DOSPROCESS
//#include <bsetib.h>


/*ULONG     pib_ulpid;      Process identifier.
  ULONG     pib_ulppid;     Parent process identifier.
  ULONG     pib_hmte;       Module handle of executable program.
  PCHAR     pib_pchcmd;     Command line pointer.
  PCHAR     pib_pchenv;     Environment pointer.
  ULONG     pib_flstatus;   Process' status bits.
  ULONG     pib_ultype;     Process' type code. */

/* Creates a process for an LX-module. */
struct t_processlx * processlx_create(struct LX_module * lx_m) {

        struct t_processlx * c = (struct t_processlx *) malloc(sizeof(struct t_processlx));
        c->lx_pib   = (struct _PIB*) malloc(sizeof(PIB));
        c->main_tid = (struct _TIB*) malloc(sizeof(TIB));

    if (c != NULL) {
                c->pid = 1;
                c->code_mmap = 0;
                c->stack_mmap = 0;
                c->lx_mod = lx_m;

                c->lx_pib->pib_ulpid = 1;
                c->lx_pib->pib_ulppid = 0;
                c->lx_pib->pib_hmte = (ULONG) lx_m;
                c->lx_pib->pib_pchcmd = "";
                c->lx_pib->pib_pchenv = "";

                init_memmgr(&c->root_mem_area); /* Initialize the memory registry. */
                /* Registrate base invalid area. */
                alloc_mem_area(&c->root_mem_area, (void*) 1, 0xfffd);
                /* Make sure the the lower 64 kb address space is marked as used. */
        }
    return(c);
}

void processlx_destroy(struct t_processlx * proc) {
        /* Avmappar filen. */
        struct o32_obj * kod_obj = (struct o32_obj *) get_code(proc->lx_mod);
        int ret_munmap = munmap((void*) kod_obj->o32_base,
                        kod_obj->o32_size);


        if(ret_munmap != 0)
                io_printf("Error at unmapping of fh: %p\n", proc->lx_mod->fh);
        else
                io_printf("\nUnmapping fh: %p\n\n", proc->lx_mod->fh);

        free(proc->lx_pib);
        free(proc->main_tid);
        free(proc);
}
