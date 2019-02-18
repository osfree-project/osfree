/*!
  @file initdone.c

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
  @author Valery Sedletski <_valerius@mail.ru>

*/

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/cfgparser.h>
#include <os3/processmgr.h>
#include <os3/io.h>
#include <os3/thread.h>

int sysinit (cfg_opts *options);

l4_os3_thread_t sysinit_id;

extern cfg_opts options;

struct options
{
  char use_events;
  char *configfile;
  char *bootdrive;
  char fprov[20];
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
int init(struct options *opts)
{
  void     *addr;           // Pointer to CONFIG.SYS in memory
  unsigned long size;       // Size of CONFIG.SYS in memory
  int rc;                   // Return code

  io_log("osFree OS/2 personality server\n");

  // Initialize initial values from CONFIG.SYS
  rc = CfgInitOptions();

  if (rc != NO_ERROR)
  {
    io_log("Can't initialize CONFIG.SYS parser\n");
    return rc;
  }

  // default config.sys path
  options.configfile = "/file/system/config.sys";

  if (opts->configfile)
  {
    options.configfile = opts->configfile;
  }

  io_log("options.configfile=%s\n", options.configfile);

  // Load CONFIG.SYS into memory
  rc = io_load_file(options.configfile, &addr, &size);

  if (rc != NO_ERROR)
  {
    io_log("Can't load CONFIG.SYS\n");
    return rc;
  }

  // Parse CONFIG.SYS in memory
  rc = CfgParseConfig((char *)addr, size);

  if (rc != NO_ERROR)
  {
    io_log("Error parse CONFIG.SYS\n");
    return rc;
  }

  // Remove CONFIG.SYS from memory
  io_close_file(addr);

  /* init process manager */
  PrcInit();

  // Perform the system initialization
  ThreadCreate((void *)sysinit, (void *)&options, THREAD_ASYNC);

  return rc;
}

void done()
{
  /* destruct process manager */
  PrcDone();
}
