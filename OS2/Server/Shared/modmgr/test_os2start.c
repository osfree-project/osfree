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
#include <stdio.h>
#include <execlx.h>
#include <fixuplx.h>
#include <loadobjlx.h>
#include <dynlink.h>
#include <native_dynlink.h>
#include <memmgr.h>


/* http://www.moses.uklinux.net/patches/lki-2.html#ss2.11
   http://tldp.org/LDP/khg/HyperNews/get/syscall/syscall86.html
*/

#define  size_t unsigned long int
#define off_t unsigned long int

//_WCRTLINK extern
void *mmap( void *__addr, size_t __len, int __prot, int __flags, int __fd, off_t __offset ){return 0;}

//_WCRTLINK extern
int munmap( void *__addr, size_t __len ){return 0;}

/* Driver program for LXLoader. */

void print_detailed_module_table();

int main(int argc, char ** argv) {
int do_start;
void * lx_buf;
int pos;
FILE *f;
char* libpath[] = {
                ".",
                "/pub/L4_Fiasco/tudos2/tudos/l4/pkg/loader/server/src/test_os2start/mini_msg_dll",
                        "/mnt/rei3/OS2/os2_program/prog_iso/Mplayer_os2/mplayer",
                        "/mnt/rei3/OS2/os2_program/prog_iso/Mplayer_os2/libc-0.6.1-csd1",
                        };
        char* native_libpath[] = {
                "c:\\os2\\dll"
                        };

        /* The structure for loaded LX exe. */
        struct LX_module lx_exe_mod ;



        printf("Starter for OS/2 programs (32 bit).\n");

        /* Initializes the memory registry. Keeps info about which virtual memory blocks
           is used (for dlls). Used for only a single process. Moved to processlx_create() */
        /* struct t_mem_area root_mem_area;
        init_memmgr(&root_mem_area); */

        /* Initializes the module list. Keeps info about which dlls an process have loaded and
           has linked to it (Only support for LX dlls so far). The head of the linked list is
           declared as a global inside dynlink.c */
        init_dynlink();


        set_libpath(libpath, 3); /* Zero-based number in libpath.*/

        set_native_libpath(native_libpath, 0); /* Zero-based number i native_libpath.*/


        if(argc > 0) {
                int i = 1;
                printf("arg %d: %s \n", i, argv[i]);
                //int f = open(argv[i], O_RDONLY);
                f = fopen(argv[i], "rb");
                if(f) {

                        /* Reads the exe file header and loads the header, if it's a valid LX exe.
                           The two functions, load_lx and load_lx_stream behaves as constructors. */

                        /* if(load_lx(f, &lx_exe_mod)) { */  /* A normal disk file.*/

                        /* Testing the buffer version of lxloader. */
                                fseek(f, 0, SEEK_END);
                                pos = ftell(f);  /* Extract the size of the file and reads it into a buffer.*/
                                rewind(f);
                                lx_buf = malloc(pos+1);
                                fread(lx_buf, pos, 1, f);
                                fclose(f);
                        if(load_lx_stream((char*)lx_buf, pos, &lx_exe_mod)) { /* A file from a buffer.*/

                            /* Creates an simple process(keeps info about it, does not start to execute). */
                                struct t_processlx * tiny_process = processlx_create(&lx_exe_mod);


                                /* Reads the actual objects from the file, code- and dataobjects.*/
                                /* load_code_data_obj_lx(&lx_exe_mod, tiny_process); */
                                load_dll_code_obj_lx(&lx_exe_mod, tiny_process); /* Use same routine as the one is loading dlls.*/

                                /* Register the exe with the module table. With the complete path. */
                                register_module(argv[i], (void *) &lx_exe_mod);

                                do_start = 0; /* A check to make sure the loading of the file succeeded. */

                                /* Applies fixups to the loaded objects. */
                                if(do_fixup_code_data_lx(&lx_exe_mod, tiny_process))
                                        do_start = 1;

                                /* Print info about used memory loaded modules. */
                                print_used_mem(&tiny_process->root_mem_area);
                                print_detailed_module_table();

                                /* Starts to execute the process. */
                                if(do_start)
                                        exec_lx(&lx_exe_mod, tiny_process);
                                processlx_destroy(tiny_process); /* Removes the process.
                                                                                                        Maybe use garbage collection here? Based on reference counter?
                                                                                                        And when the counter reeches zero, release process. */
                        } else {
                                printf("load_lx returns an error !!! \n");
                        }
                        printf("Exiting test_os2start with: %s \n", argv[i]);
                }
        } else printf("Unable to load file! \n");


        return 0;
}


/* Goes through every loaded module and prints out all it's objects. */
void print_detailed_module_table() {
     int num_objects;
     int i;
        struct module_rec * el = get_root();
        printf("--- Detailed Loaded Module Table ---\n");
        while((el = get_next(el))) {
                printf("module = %s, module_struct = %p, load_status = %d\n",
                                get_name(el), get_module(el), el->load_status);
                num_objects = get_obj_num(get_module(el));
                i=0;
                for(i=1; i<=num_objects; i++) {
                        struct o32_obj * an_obj = get_obj(get_module(el), i);
                        print_o32_obj_info(an_obj, get_name(el));
                }
        }
}
