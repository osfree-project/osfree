/*  Filesystem/IFS server
 *  for osFree OS/2 personality
 *
 */

/* osFree internal */
#include <os3/MountReg.h>
#include <os3/globals.h>
#include <os3/cpi.h>
#include <os3/io.h>

struct options
{
  char use_events;
};

int init(struct options *opts);
void done(void);

l4_os3_thread_t fs;
l4_os3_thread_t os2srv;
l4_os3_thread_t mythread;

os2exec_module_t s = {0};
char szLoadError[260];

int FSR_INIT(void);

int init(struct options *opts)
{
  io_log("osFree FS server\n");

  init_globals();
  //FSR_INIT();

  if ( CPClientInit(&os2srv) )
  {
    io_log("Can't find os2srv on names, exiting...\n");
    return 1;
  }

  // notify os2srv about successful startup
  CPClientAppNotify2(&s, "os2fs", &mythread,
                     szLoadError, sizeof(szLoadError), 0);

  return 0;
}

void done(void)
{
  CPClientDone();
}
