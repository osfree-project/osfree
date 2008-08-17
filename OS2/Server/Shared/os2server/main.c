/*!
  @file main.c

  @brief OS/2 Server

    OS/2 Server - Starts osFree OS/2 personality
    Copyright (C) 2007  Sven Rosén (aka Viking)
    Copyright 2007 by Sascha Schmidt and the osFree Project
    Copyright 2008 Yuri Prokushev

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

  @author Sascha Schmidt <sascha.schmidt@asamnet.de>
  @author Sven Rosén (aka Viking)
  @author Yuri Prokushev

*/

#include <cfgparser.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <io.h>
#include <execlx.h>
#include <processlx.h>
#include <fixuplx.h>
#include <loadobjlx.h>
#include <native_dynlink.h>
#include <memmgr.h>
#include <modmgr.h>

struct t_mem_area os2server_root_mem_area;

#define  size_t unsigned long int

int munmap( void *__addr, size_t __len ){return 0;}

void print_detailed_module_table();

 /*********************************************************************
 * Here everything starts. This is the main function of the           *
 * OS/2 Server.                                                       *
 **********************************************************************/
int main(int argc, const char **argv)
{
  int rc;               // Return code
  void *addr;           // Pointer to CONFIG.SYS in memory
  int size;             // Size of CONFIG.SYS in memory

  io_printf("OS/2 Server started\n");

    // Initialize initial values from CONFIG.SYS
  rc=cfg_init_options();

    /* Initializes the module list. Keeps info about which dlls an process have loaded and
           has linked to it (Only support for LX dlls so far). The head of the linked list is
           declared as a global inside dynlink.c */
  init_dynlink();


  // Load CONFIG.SYS into memory
  rc=io_load_file("config.sys", &addr, &size);

  // Parse CONFIG.SYS in memory
  rc=cfg_parse_config(addr, size);

  // Remove CONFIG.SYS from memory
  //    rc=memmgr_free(addr, size);

  // Check PROTSHELL statament value
  if (!options.protshell||(strlen(options.protshell)==0))
  {
    io_printf("No PROTSHELL statament in CONFIG.SYS\n");
  } else {
    // Load and execute shell
    rc=modmgr_execute_module(options.protshell);
  }

  // Clean up config data
  rc=cfg_cleanup();

  io_printf("OS/2 Server ended\n");

  return rc;
}



/* Goes through every loaded module and prints out all it's objects. */
void print_detailed_module_table() {
     int num_objects;
     int i;
        struct module_rec * el = get_root();
        io_printf("--- Detailed Loaded Module Table ---\n");
        while((el = get_next(el))) {
                io_printf("module = %s, module_struct = %p, load_status = %d\n",
                                get_name(el), get_module(el), el->load_status);
                num_objects = get_obj_num(get_module(el));
                i=0;
                for(i=1; i<=num_objects; i++) {
                        struct o32_obj * an_obj = get_obj(get_module(el), i);
                        print_o32_obj_info(an_obj, get_name(el));
                }
        }
}
