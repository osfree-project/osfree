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

#define INCL_DOS
#define INCL_BASE
#define INCL_BSEDOS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cfgparser.h>
#include <io.h>
#include <native_dynlink.h>
#include <modmgr.h>
#include <memmgr.h>
#include <processmgr.h>

#include <modlx.h>

#if defined(L4API_l4v2)
 #include <gcc_os2def.h>
 #include "globals.h"
#endif

struct t_mem_area os2server_root_mem_area;

#define  size_t unsigned long int
/* munmap is not available with Open Watcom.*/
#if defined(__LINUX__)
int munmap( void *__addr, size_t __len ){ printf("munmap not implemented\n"); return 0;}
#endif

/*! @brief This is the main function of the osFree OS/2 Personality Server.
           Loads config.sys, executes all CALL and RUN stataments, creates
           initial environment using SET stataments, starts main shell
           pointed by PROTSHELL statament.

    @param argc   Number of arguments
    @param argv   Array of arguments

    @return
      NO_ERROR                  Server finished successfully
      ERROR_INVALID_PARAMETER   Invalid argument or CONFIG.SYS settings

      See also other error codes

*/
int main(int argc, const char **argv)
{
  int rc;               // Return code
  void *addr;           // Pointer to CONFIG.SYS in memory
  unsigned long size;   // Size of CONFIG.SYS in memory

  io_printf("osFree OS/2 Personality Server\n");

  #if defined(L4API_l4v2)
  init_globals();
  l4_test_mem_alloc();
  #endif

  // Initialize initial values from CONFIG.SYS
  rc=CfgInitOptions();

  /* Initializes the module list. Keeps info about which dlls an process have loaded and
     has linked to it (Only support for LX dlls so far). The head of the linked list is
     declared as a global inside dynlink.c */
  rc=ModInitialize();

  // Load CONFIG.SYS into memory
  rc=io_load_file("config.sys", &addr, &size);
  if (rc)
  {
    io_printf("Can't load CONFIG.SYS\n");
    return rc;
  }

  // Parse CONFIG.SYS in memory
  rc=CfgParseConfig(addr, size);

  // Remove CONFIG.SYS from memory
  //    rc=memmgr_free(addr, size);

  // Check PROTSHELL statament value
  if (!options.protshell||(strlen(options.protshell)==0))
  {
    io_printf("No PROTSHELL statement in CONFIG.SYS\n");
    return ERROR_INVALID_PARAMETER; /*ERROR_INVALID_PARAMETER 87; Not defined for Windows*/
  } else {
    /*ModLoadModule(char *          pszName,
                            unsigned long   cbName,
                            char const *    pszModname,
                            unsigned long * phmod) */
//     char errbuf[256];
//     char *p_errbuf = (char *) &errbuf;
//     unsigned long m=0;
//     struct LX_module *found_module=0;
//     ModLoadModule(p_errbuf, 255, "KAL", &m); /* Seems to compare module names case sensitive.*/
//     ModLoadModule(p_errbuf, 255, "DOSCALLS", (int*)&found_module);
//     ModLoadModule(p_errbuf, 255, "MSG", &m);
//     if (found_module) {
//          found_module=(struct LX_module *)(((IXFModule *)found_module)->FormatStruct);
//     }
//     int ret_flags, ret_offset, ret_obj, ret_modord, ret_type;
//     int *fn_ptr=0;
//     fn_ptr = get_entry((struct LX_module *)found_module, 387,
//                      &ret_flags, &ret_offset, &ret_obj, &ret_modord, &ret_type);
//     io_printf("fn_ptr: %p, ret_type:%d\n", fn_ptr, ret_type);

    // Load and execute shell
    rc=PrcExecuteModule(NULL, 0, 0, NULL, NULL, NULL, options.protshell);
    if(rc) io_printf("execute error: %d ('%s')\n", rc, options.protshell);
  }

  // Clean up config data
  rc=CfgCleanup();

  io_printf("OS/2 Server ended\n");

  return rc;
}
