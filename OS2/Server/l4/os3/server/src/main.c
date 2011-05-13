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

#include <l4/os3/cfgparser.h>
#include <l4/os3/io.h>
#include <l4/os3/native_dynlink.h>
#include <l4/os3/modmgr.h>
#include <l4/os3/memmgr.h>
#include <l4/os3/processmgr.h>

#include <l4/os3/modlx.h>

#if defined(L4API_l4v2)
 #include <l4/os3/gcc_os2def.h>
 #include <l4/os3/globals.h>
 #include <l4/names/libnames.h>
 //#include <l4/l4con/l4con.h>
 //#include <l4/l4con/l4con-client.h>
 #include <l4/env/errno.h>
 //#include <l4/thread/thread.h>
 #include <l4/log/l4log.h>
 #include <l4/l4rm/l4rm.h>
 //#include <l4/generic_ts/generic_ts.h>
 #include <l4/generic_fprov/generic_fprov-client.h>
#endif

struct t_mem_area os2server_root_mem_area;
char fprov[20] = "BMODFS";
// use events server
char use_events = 0;

l4_threadid_t fs;
l4_threadid_t os2srv;
l4_threadid_t sysinit_id;
l4_threadid_t dsm_id;
l4_threadid_t fprov_id;
l4_threadid_t loader_id;

extern cfg_opts options;

int sysinit (cfg_opts *options);

#define  size_t unsigned long int
/* munmap is not available with Open Watcom.*/
#if defined(__LINUX__)
int munmap( void *__addr, size_t __len ){ printf("munmap not implemented\n"); return 0;}
#endif

void usage(void)
{
};

int
fprov_attach_file(char *file, l4dm_dataspace_t *ds, l4_addr_t *addr, l4_size_t *size)
{
  CORBA_Environment env = dice_default_environment;
  int rc;

  if ((rc = l4fprov_file_open_call(&fprov_id,
				   file,
				   &dsm_id,
				   0, ds, size, &env)))
  {
    printf("Error opening file \"%s\" at %s: %s\n",
           file, fprov, l4env_errstr(rc));
    return rc;
  }

  if ((rc = l4rm_attach(ds, *size, 0,
                	L4DM_RO | L4RM_MAP, addr)))
  {
    printf("Error attaching dataspace: %s\n", l4env_errstr(rc));
    l4dm_close(ds);
    return rc;
  }

  return 0; /* NO_ERROR */  
}

void
fprov_detach_file(l4dm_dataspace_t ds, int addr)
{
  l4rm_detach(addr);
  l4dm_close(&ds);
}

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
  CORBA_Environment env = dice_default_environment;
  l4dm_dataspace_t ds;
  l4_addr_t addr;           // Pointer to CONFIG.SYS in memory
  l4_size_t size;           // Size of CONFIG.SYS in memory
  l4_threadid_t tid;
  int rc;                   // Return code
  int step = 0;
  int opt = 0;
  char *drive=0;
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

  LOG("osFree OS/2 Personality Server");
  LOG("argc=%d", argc);

  //int task_status = l4ts_allocate_task2(0, &taskid);
                
  //#if defined(L4API_l4v2)
  //init_globals();
  //#endif

  if (!names_register("os2srv"))
  {
    LOG("Error registering on the name server");
    return 1;
  }

  // Parse command line arguments
  for (;;)
    {
      opt = getopt_long(argc, argv, "s:c:b:f:e", long_options, &optionid);
      if (opt == -1) break;
      switch (opt)
        {
#if 0
        case 'd':
            if (step == 0)
            {
                drive   = optarg;
                step = 1;
            }
            else
            {
                usage();
                return 1;
            }
            break;

        case 'm':
            if (step == 1)
            {
               char * mp=malloc(strlen(optarg)+1);
               strcpy(mp, optarg);
               struct I_Fs_srv  *fs_srv = malloc(sizeof(struct I_Fs_srv));
               I_Fs_srv_init2(fs_srv, mp, drive);
               FSRouter_add(&fsrouter, fs_srv);
               LOG("%s => %s", fs_srv->drive, fs_srv->mountpoint);
               step = 0;
            }
            else
            {
                usage();
                return 1;
            }
            break;
#endif
        case 'c':
            options.configfile=malloc(strlen(optarg)+1);
            strcpy(options.configfile,optarg);
            break;

        case 'b':
            options.bootdrive=malloc(strlen(optarg)+1);
            strcpy(options.bootdrive,optarg);
            break;

        case 'f':
            LOG("fprov is %s", optarg);
	    strcpy(fprov, optarg);
	    break;

        case 'e':
            LOG("using events server");
	    use_events = 1;
	    break;

        case -1:
            // prevents showing fstab_info if no more option exists
            break;
	    
        default:
            LOG("Error: Unknown option \"%c\"", opt);
            usage();
            return 1;

        }
    }

  /* Wait for servers to be started */
  if (!names_waitfor_name(fprov, &fprov_id, 10000))
  {
    LOG("Server \"%s\" not found\n", fprov);
    return 1;
  }

  LOG("waiting for LOADER server to be started...");
  if (!names_waitfor_name("LOADER", &loader_id, 30000))
    {
      LOG("Dynamic loader LOADER not found -- terminating");
      while (1) l4_sleep(0.1);
    }
  LOG("loader id: %x.%x", loader_id.id.task, loader_id.id.lthread);
  
  // query OS/2 server task id
  os2srv = l4_myself();
  LOG("OS/2 server uid=%x.%x", os2srv.id.task, os2srv.id.lthread);

  /* query default dataspace manager id */
  dsm_id = l4env_get_default_dsm();
  if (l4_is_invalid_id(dsm_id))
  {
    printf("No dataspace manager found\n");
    return 2;
  }
  
  //contxt_init(65535, 11);

  // Initialize initial values from CONFIG.SYS
  rc=CfgInitOptions();
  if (rc!=NO_ERROR)
  {
    LOG("Can't initialize CONFIG.SYS parser");
    return rc;
  }
  LOG("options.configfile=%s", options.configfile);

  // Load CONFIG.SYS into memory
  LOG("options.configfile=%s", options.configfile);
  //rc=io_load_file(options.configfile, &addr, &size);
  rc = fprov_attach_file(options.configfile, &ds, &addr, &size);
  if (rc!=NO_ERROR)
  {
    io_printf("Can't load CONFIG.SYS");
    return rc;
  }

  LOG("options.configfile=%s", options.configfile);
  LOG_printf("%s\n", (char *)addr);
  // Parse CONFIG.SYS in memory
  rc=CfgParseConfig(addr, size);
  if (rc!=NO_ERROR)
  {
    io_printf("Error parse CONFIG.SYS");
    return rc;
  }

  // Remove CONFIG.SYS from memory
  //    rc=memmgr_free(addr, size);
  fprov_detach_file(ds, addr);

  // Perform the System initialization
  l4thread_create(sysinit, (void *)&options, L4THREAD_CREATE_ASYNC);

  // server loop
  env.malloc = (dice_malloc_func)malloc;
  env.free = (dice_free_func)free;
  os2server_server_loop(&env);
    
  return rc;
}
