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

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/cfgparser.h>
#include <os3/io.h>
#include <os3/thread.h>
#include <os3/cpi.h>

/* l4env includes */
#include <l4/sys/types.h>
#include <l4/names/libnames.h>
#include <l4/events/events.h>

/* Servers RPC includes */
#include "os2server-server.h"

/* libc includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

//struct t_mem_area os2server_root_mem_area;
char fprov[20] = "BMODFS";
// use events server
char use_events = 0;

l4_threadid_t fs;
l4_threadid_t os2srv;
l4_threadid_t fprov_id;
l4_os3_thread_t dsm_id;
l4_os3_thread_t loader_id;
l4_os3_thread_t sysinit_id;

extern cfg_opts options;

int sysinit (cfg_opts *options);
void usage(void);

l4_os3_thread_t CPNativeID(void)
{
  l4_os3_thread_t thread;

  thread.thread = l4_myself();

  return thread;
}

void exit_notify(void)
{
  l4events_ch_t event_ch = L4EVENTS_EXIT_CHANNEL;
  l4events_nr_t event_nr = L4EVENTS_NO_NR;
  l4events_event_t event;

  if (use_events) // use events server
  {
    // terminate by sending an exit event
    event.len = sizeof(l4_threadid_t);
    *(l4_threadid_t*)event.str = l4_myself();
    // send exit event
    l4events_send(event_ch, &event, &event_nr, L4EVENTS_SEND_ACK);
    // get acknowledge
    l4events_get_ack(&event_nr, L4_IPC_NEVER);
  }
}

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
  CORBA_Server_Environment env = dice_default_server_environment;
  void     *addr;           // Pointer to CONFIG.SYS in memory
  unsigned long size;       // Size of CONFIG.SYS in memory
  int rc;                   // Return code
  int opt = 0;
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

  io_log("osFree OS/2 Personality Server\n");
  io_log("argc=%d\n", argc);

  if (! names_register("os2srv") )
  {
    io_log("Error registering on the name server\n");
    return 1;
  }

  // Initialize initial values from CONFIG.SYS
  rc = CfgInitOptions();

  if (rc != NO_ERROR)
  {
    io_log("Can't initialize CONFIG.SYS parser\n");
    return rc;
  }

  // default config.sys path
  options.configfile = "/file/system/config.sys";

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
            io_log("fprov is %s\n", optarg);
            strcpy(fprov, optarg);
            break;

        case 'e':
            io_log("using events server\n");
            use_events = 1;
            break;

        case -1:
            // prevents showing fstab_info if no more option exists
            break;

        default:
            io_log("Error: Unknown option \"%c\"\n", opt);
            usage();
            return 1;

        }
    }

  io_log("options.configfile=%s\n", options.configfile);

  /* Wait for servers to be started */
  if (! names_waitfor_name(fprov, &fprov_id, 10000))
  {
    io_log("Server \"%s\" not found\n", fprov);
    return 1;
  }

  io_log("waiting for LOADER server to be started...\n");

  if (! names_waitfor_name("LOADER", &loader_id.thread, 30000))
    {
      io_log("Dynamic loader LOADER not found -- terminating\n");
      while (1) l4_sleep(0.1);
    }

  io_log("loader id: %x.%x", loader_id.thread.id.task, loader_id.thread.id.lthread);

  // query OS/2 server task id
  //os2srv = l4_myself();
  CPClientInit();
  io_log("OS/2 server uid=%x.%x\n", os2srv.id.task, os2srv.id.lthread);

  /* query default dataspace manager id */
  dsm_id.thread = l4env_get_default_dsm();

  if (l4_is_invalid_id(dsm_id.thread))
  {
    printf("No dataspace manager found\n");
    return 2;
  }

  // Load CONFIG.SYS into memory
  rc = io_load_file(options.configfile, &addr, &size);

  if (rc != NO_ERROR)
  {
    io_log("Can't load CONFIG.SYS\n");
    return rc;
  }

  io_log("%s\n", (char *)addr);

  // Parse CONFIG.SYS in memory
  rc = CfgParseConfig((char *)addr, size);

  if (rc != NO_ERROR)
  {
    io_log("Error parse CONFIG.SYS\n");
    return rc;
  }

  // Remove CONFIG.SYS from memory
  io_close_file(addr);

  // Perform the System initialization
  ThreadCreate((void *)sysinit, (void *)&options, THREAD_ASYNC);

  // server loop
  env.malloc = (dice_malloc_func)malloc;
  env.free = (dice_free_func)free;
  os2server_server_loop(&env);

  CPClientDone();
  return rc;
}
