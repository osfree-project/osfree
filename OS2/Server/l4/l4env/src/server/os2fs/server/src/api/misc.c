/*  OS/2 fs server API: misc functions
 *  osFree OS/2 personality
 *
 */

#include <l4/os3/gcc_os2def.h>
#include <l4/os3/MountReg.h>
#include <l4/os3/globals.h>
#include <l4/os3/io.h>

#include <ctype.h>

#include <dice/dice.h>

#include <os2fs-server.h>

static ULONG drivemap = -1;

void setdrivemap(ULONG *map);

void setdrivemap(ULONG *map)
{
  struct I_Fs_srv *fsrv;
  int  i, diskno;
  char *drive;
  char drv;

  *map = 0;

  for (i = 0; i < fsrouter.srv_num_; i++)
  {
    fsrv = fsrouter.fs_srv_arr_[i];
    drive = fsrv->drive;
    drv = tolower(*(drive));
    diskno = drv - 'a';
    *map |= (1 << diskno);    
  }

  io_log("map=%lu", *map);
}
 
int DICE_CV
os2fs_get_drivemap_component (CORBA_Object _dice_corba_obj,
                              ULONG *map /* out */,
                              CORBA_Server_Environment *_dice_corba_env)
{
  if (drivemap == -1)
    setdrivemap(&drivemap);

  *map = drivemap;
  return 0;
}
