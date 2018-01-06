/*  Filesystem/IFS server
 *  for osFree OS/2 personality
 *
 */

/* osFree internal */
#include <os3/MountReg.h>
#include <os3/globals.h>
#include <os3/io.h>

/* l4env includes */
#include <l4/names/libnames.h>
#include <l4/sys/types.h>
#include <l4/thread/thread.h>
//#include <l4/events/events.h>

/* libc includes */
#include <stdlib.h>
#include <getopt.h>

/* dice includes */
#include <dice/dice.h>

/* os2fs server includes */
#include <os2fs-server.h>

//l4_threadid_t os2srv;
l4_threadid_t fprov_id;

// use events server flag
char use_events = 0;

void usage(void);
void event_thread(void);

int FSR_INIT(void);

void usage(void)
{
  io_log("os2fs usage:\n");
  io_log("-e:  Use events server");
} 

#if 0
void event_thread(void)
{
  l4events_ch_t event_ch = L4EVENTS_EXIT_CHANNEL;
  l4events_nr_t event_nr = L4EVENTS_NO_NR;
  l4events_event_t event;
  l4_threadid_t tid;
  int rc;

  if (! l4events_init())
  {
    io_log("l4events_init() failed\n");
    return;
  }

  if ((rc = l4events_register(L4EVENTS_EXIT_CHANNEL, 15)) != 0)
  {
    io_log("l4events_register failed\n");
    return;
  }

  while(1)
  {
    /* wait for event */
    if ((rc = l4events_give_ack_and_receive(&event_ch, &event, &event_nr,
					    L4_IPC_NEVER,
					    L4EVENTS_RECV_ACK))<0)
    {
      io_log("l4events_give_ack_and_receive()\n");
      continue;
    }
    tid = *(l4_threadid_t *)event.str;
    io_log("Got exit event for %x.%x\n", tid.id.task, tid.id.lthread);

    /* exit myself */
    if (l4_task_equal(tid, os2srv))
      exit(rc);
  }
}
#endif
 
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

  if (! names_register("os2fs"))
  {
    io_log("Can't register on the name server!\n");
    return 1;
  }

  io_log("registered at the name server\n");

  /* if (! names_waitfor_name("os2srv", &os2srv, 30000))
  {
    io_log("Can't find os2srv on names, exiting...\n");
    return 1;
  } */

  //io_log("got os2srv tid from the name server\n");
  io_log("argc=%d\n", argc);

  // Parse command line arguments
  for (;;)
  {
    opt = getopt_long(argc, argv, "e", long_options, &optionid);
    io_log("opt=%d\n", opt);
    if (opt == -1) break;
    switch (opt)
    {
      case 'e':
        io_log("using events server\n");
        use_events = 1;
        break;

      default:
        io_log("Error: Unknown option %c\n", opt);
        usage();
        break;
    }
  }

  // start events thread
  /* if (use_events)
  {
    // start events thread
    l4thread_create((void *)event_thread, 0, L4THREAD_CREATE_ASYNC);
    io_log("event thread started\n");
  } */

  // get our thread ID
  fprov_id = l4_myself();

  // server loop
  io_log("going to the server loop\n");
  env.malloc = (dice_malloc_func)malloc;
  env.free = (dice_free_func)free;
  os2fs_server_loop(&env);
  return 0;
}
