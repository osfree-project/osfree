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

/* L4 includes */
#include <l4/loader/loader-client.h>

/* osFree includes */
#include <l4/os3/err.h>
#include <l4/os3/types.h>
#include <l4/os3/dataspace.h>
#include <l4/os3/rm.h>
#include <l4/os3/modmgr.h>
#include <l4/os3/ixfmgr.h>
#include <l4/os3/execlx.h>
#include <l4/os3/fixuplx.h>
#include <l4/os3/io.h>

/* other includes */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

// servers thread ids
extern l4os3_cap_idx_t os2srv;
extern l4os3_cap_idx_t loader_id;
extern l4os3_cap_idx_t fprov_id;
extern l4os3_cap_idx_t dsm_id;

int l4os3_exec(char *cmd, char *params, l4_taskid_t *taskid);
int l4os3_os2_exec(char *pName, struct t_os2process *proc);

// use events server
extern char use_events;

int
l4os3_exec(char *cmd, char *params, l4_taskid_t *taskid)
{
  #define MAX_TASK_ID 16
  CORBA_Environment env = dice_default_environment;
  //char name[] = "os2app.cfg";
  char parm[1024] = "";
  char cmd_buf[0x20];
  l4_taskid_t task_ids[MAX_TASK_ID];
  char error_msg[1024];
  char *ptr = error_msg;
  l4os3_ds_t ds = L4DM_INVALID_DATASPACE;
  l4_addr_t addr;
  int error;

  // If we use events server, pass this option
  // to other servers/apps too
  if (use_events)
  {
    io_log("using events\n");
    strcat(parm, " --events ");
  }

  strcat(parm, params);
  
  io_log("cmd=%s\n", cmd);
  io_log("parm=\"%s\"\n", parm);

  /* RPC call to DM_PHYS (create a dataspace) */
  //if (if_l4dm_mem_open_call(&dsm_id, 1024, 0, 0,
  //                            name, &ds, &env))
  if (l4os3_ds_allocate(&ds, 0, 1024))
    {
      io_log("Can't allocate a dataspace!\n");
      while (1) { l4_sleep(0.1); }
    }
  io_log("dataspace created\n");
      
  /* attach the dataspace to our own address space */
  attach_ds(&ds, L4DM_RW, &addr);
  io_log("dataspace attached\n");
      
  /* create a loader script */
  strcpy((char *)addr, "modpath \"/file/system\"\n\ntask \"");
  strcat((char *)addr, cmd);
  strcat((char *)addr, "\"  \"--stdin /dev/vc0 --stdout /dev/vc0 --stderr /dev/vc0 ");
  strcat((char *)addr, parm);
  strcat((char *)addr, "\"");
  strcat((char *)addr, "\n\n  priority 0xA0");

  /* detach dataspace */
  l4rm_detach((void *)addr);

  /* transfer dataspace to loader */
  l4dm_transfer(&ds,              // dataspace
                loader_id);       // taskid
  
  /* RPC to L4 loader to start OS/2 app L4 startup */
  if ((error = l4loader_app_open_call(&loader_id, &ds, cmd_buf,
                                        &fprov_id, 0, task_ids,
                                        &ptr, &env)) < 0)
    {
      switch (-error)
      {
        case L4_ENOTFOUND:
          io_log("file %s not found!\n", cmd_buf);
          return ERROR_FILE_NOT_FOUND;
        case L4_ENOMEM:
          io_log("Not enough memory!\n");
          return ERROR_NOT_ENOUGH_MEMORY;
        case L4_EINVAL:
          io_log("Invalid data!\n");
          return ERROR_INVALID_DATA;
        default:
          io_log("General failure loading the ELF file!\n");
          return ERROR_GEN_FAILURE;
      }
    }

  *taskid = task_ids[0];
  return NO_ERROR;
}


int l4os3_os2_exec(char *pName, struct t_os2process *proc)
{
    l4os3_cap_idx_t    taskid;
    int rc;

    rc = l4os3_exec("os2app", pName, &taskid);
    proc->task = taskid;
    return rc;
}

