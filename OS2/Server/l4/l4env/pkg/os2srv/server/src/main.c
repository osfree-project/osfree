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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include <l4/os3/types.h>
#include <l4/os3/cfgparser.h>
#include <l4/os3/io.h>
#include <l4/os3/native_dynlink.h>
#include <l4/os3/modmgr.h>
#include <l4/os3/memmgr.h>
#include <l4/os3/processmgr.h>
#include <l4/os3/modlx.h>
#include <l4/os3/gcc_os2def.h>
#include <l4/os3/globals.h>
#include <l4/os3/thread.h>
#include <l4/os3/namespace.h>

// IPC includes
#include <l4/os3/ipc.h>
#include "os2server-server.h"

struct t_mem_area os2server_root_mem_area;
char fprov[20] = "BMODFS";
// use events server
char use_events = 0;

l4_cap_idx_t fs;
l4_cap_idx_t os2srv;
l4_cap_idx_t fprov_id;
l4_cap_idx_t sysinit_id;
l4_cap_idx_t dsm_id;
l4_cap_idx_t loader_id;

extern cfg_opts options;

int sysinit (cfg_opts *options);
void usage(void);

void usage(void)
{
};

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
  CORBA_srv_env env = default_srv_env;
  //l4dm_dataspace_t ds;
  l4_addr_t addr;           // Pointer to CONFIG.SYS in memory
  l4_size_t size;           // Size of CONFIG.SYS in memory
  //l4_cap_idx_t tid;
  int rc;                   // Return code
  //int step = 0;
  int opt = 0;
  //char *drive=0;
  int optionid;
  const struct option long_options[] =
                {
                { "server",      1, NULL, 's'},
                { "drive",       1, NULL, 'd'},
                { "mountpoint",  1, NULL, 'm'},
                { "config",      1, NULL, 'c'},
                { "bootdrive",   1, NULL, 'b'},
                { "fprov",       1, NULL, 'f'},
                { "events",      no_argument, NULL, 'e'},
		{ 0, 0, 0, 0}
                };

  io_printf("osFree OS/2 Personality Server");
  io_printf("argc=%d", argc);

  if (!names_register("os2srv"))
  {
    io_printf("Error registering on the name server");
    return 1;
  }

  // Parse command line arguments
  for (;;)
    {
      opt = getopt_long(argc, (char **)argv, "s:c:b:f:e", long_options, &optionid);
      if (opt == -1) break;
      switch (opt)
        {
        case 'c':
            options.configfile=malloc(strlen(optarg)+1);
            strcpy(options.configfile,optarg);
            break;

        case 'b':
            options.bootdrive=malloc(strlen(optarg)+1);
            strcpy(options.bootdrive,optarg);
            break;

        case 'f':
            io_printf("fprov is %s", optarg);
	    strcpy(fprov, optarg);
	    break;

        case 'e':
            io_printf("using events server");
	    use_events = 1;
	    break;

        case -1:
            // prevents showing fstab_info if no more option exists
            break;
	    
        default:
            io_printf("Error: Unknown option \"%c\"", opt);
            usage();
            return 1;

        }
    }

  /* Wait for servers to be started */
  if (!names_waitfor_name(fprov, &fprov_id, 10000))
  {
    io_printf("Server \"%s\" not found\n", fprov);
    return 1;
  }

  io_printf("waiting for LOADER server to be started...");
  if (!names_waitfor_name("LOADER", &loader_id, 30000))
    {
      io_printf("Dynamic loader LOADER not found -- terminating");
      while (1) l4_sleep(0.1);
    }
  io_printf("loader id: %x.%x", loader_id.id.task, loader_id.id.lthread);
  
  // query OS/2 server task id
  os2srv = l4_myself();
  io_printf("OS/2 server uid=%x.%x", os2srv.id.task, os2srv.id.lthread);

  /* query default dataspace manager id */
  dsm_id = l4env_get_default_dsm();
  if (l4_is_invalid_id(dsm_id))
  {
    printf("No dataspace manager found\n");
    return 2;
  }

  // Initialize initial values from CONFIG.SYS
  rc = CfgInitOptions();
  if (rc != NO_ERROR)
  {
    io_printf("Can't initialize CONFIG.SYS parser");
    return rc;
  }
  io_printf("options.configfile=%s", options.configfile);

  // Load CONFIG.SYS into memory
  rc = io_load_file(options.configfile, &addr, &size);
  if (rc != NO_ERROR)
  {
    io_printf("Can't load CONFIG.SYS");
    return rc;
  }

  io_printf("%s\n", (char *)addr);
  // Parse CONFIG.SYS in memory
  rc = CfgParseConfig((char *)addr, size);
  if (rc != NO_ERROR)
  {
    io_printf("Error parse CONFIG.SYS");
    return rc;
  }

  // Remove CONFIG.SYS from memory
  io_close_file(addr);

  // Perform the System initialization
  os3_thread_create((void *)sysinit, (void *)&options, L4THREAD_CREATE_ASYNC);

  // server loop
  env.malloc = (dice_malloc_func)malloc;
  env.free = (dice_free_func)free;
  os2server_server_loop(&env);

  return rc;
}
