/*  Filesystem/IFS server
 *  for osFree OS/2 personality
 *
 */

#include <l4/names/libnames.h>
#include <l4/log/l4log.h>
#include <l4/sys/types.h>

#include <l4/os3/MountReg.h>
#include <l4/os3/globals.h>

#include <stdlib.h>

#include <dice/dice.h>

l4_threadid_t fs;
l4_threadid_t os2srv;
 
int main(int argc, char **argv)
{
  CORBA_Environment env = dice_default_environment;
  int  rc;

  if (!names_register("os2fs"))
  {
    LOG("Can't register on the name server!");
    return 1;
  }
  LOG("registered at the name server");

  if (!names_waitfor_name("os2srv", &os2srv, 30000))
  {
    LOG("Can't find os2srv on names, exiting...");
    return 1;
  }
  LOG("got os2srv tid from the name server");

  init_globals();

  // my id
  fs = l4_myself();

  // server loop
  LOG("going to the server loop");
  env.malloc = (dice_malloc_func)malloc;
  env.free = (dice_free_func)free;
  os2fs_server_loop(&env);

  return 0;
}
