/*  Filesystem/IFS server
 *  for osFree OS/2 personality
 *
 */

/* l4env includes */
#include <l4/names/libnames.h>
#include <l4/env/errno.h>
#include <l4/log/l4log.h>
#include <l4/sys/types.h>
#include <l4/thread/thread.h>
#include <l4/generic_ts/generic_ts.h>
#include <l4/events/events.h>
#include <l4/util/l4_macros.h>
/* OS/2 personality includes */
#include <l4/os3/MountReg.h>
#include <l4/os3/globals.h>
/* libc includes */
#include <stdlib.h>
#include <getopt.h>
/* dice includes */
#include <dice/dice.h>
/* os2fs server includes */
#include <os2fs-server.h>

l4_threadid_t fs;
l4_threadid_t os2srv;

// use events server flag
char use_events = 0;

void usage(void);
void event_thread(void);

int FSR_INIT(void);

void usage(void)
{
  LOG("os2fs usage:\n");
  LOG("-e:  Use events server");
} 

void event_thread(void)
{
  l4events_ch_t event_ch = L4EVENTS_EXIT_CHANNEL;
  l4events_nr_t event_nr = L4EVENTS_NO_NR;
  l4events_event_t event;
  l4_threadid_t tid;
  int rc;

  if (!l4events_init())
  {
    LOG_Error("l4events_init() failed");
    return;
  }

  if ((rc = l4events_register(L4EVENTS_EXIT_CHANNEL, 15)) != 0)
  {
    LOG_Error("l4events_register failed");
    return;
  }

  while(1)
  {
    /* wait for event */
    if ((rc = l4events_give_ack_and_receive(&event_ch, &event, &event_nr,
					    L4_IPC_NEVER,
					    L4EVENTS_RECV_ACK))<0)
    {
      l4env_perror("l4events_give_ack_and_receive()", -rc);
      continue;
    }
    tid = *(l4_threadid_t *)event.str;
    LOG("Got exit event for "l4util_idfmt, l4util_idstr(tid));

    /* exit myself */
    if (l4_task_equal(tid, os2srv))
      exit(rc);
  }
}
 
int main(int argc, char **argv)
{
  CORBA_Environment env = dice_default_environment;
  int optionid;
  int opt = 0;
  const struct option long_options[] =
                {
                { "events",      no_argument, NULL, 'e'},
		{ 0, 0, 0, 0}
                };
  //int  rc;

  init_globals();
  //FSR_INIT();

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

  LOG("argc=%d", argc);
  // Parse command line arguments
  for (;;)
  {
    opt = getopt_long(argc, argv, "e", long_options, &optionid);
    LOG("opt=%d", opt);
    if (opt == -1) break;
    switch (opt)
    {
      case 'e':
        LOG("using events server");
	use_events = 1;
	break;
      
      default:
        LOG("Error: Unknown option %c", opt);
        usage();
        break;
    }
  }

  // start events thread
  if (use_events)
  {
    // start events thread
    l4thread_create((void *)event_thread, 0, L4THREAD_CREATE_ASYNC);
    LOG("event thread started");
  }

  // my id
  fs = l4_myself();

  // server loop
  LOG("going to the server loop");
  env.malloc = (dice_malloc_func)malloc;
  env.free = (dice_free_func)free;
  os2fs_server_loop(&env);

  return 0;
}
