/* osFree internal */
#include <os3/io.h>
#include <os3/thread.h>

/* l4env includes */
#include <l4/names/libnames.h>
#include <l4/sys/types.h>
#include <l4/events/events.h>

/* libc includes */
#include <stdlib.h>
#include <getopt.h>

/* dice includes */
#include <dice/dice.h>

/* os2fs server includes */
#include <os2fs-server.h>

extern l4_threadid_t os2srv;
extern l4_os3_thread_t mythread;

l4_os3_thread_t fprov_id;

struct options
{
  char use_events;
};

void usage(void);
void event_thread(void);
void parse_options(int argc, char **argv, struct options *opts);
int init(struct options *opts);
void done(void);

void usage(void)
{
  io_log("os2fs usage:\n");
  io_log("-e:  Use events server");
}

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

    io_log("os2fs event thread: tid=%lx:%lx, os2srv=%lx:%lx\n",
           tid.id.task, tid.id.lthread,
           os2srv.id.task, os2srv.id.lthread);

    /* exit myself */
    if (l4_task_equal(tid, os2srv))
    {
      exit(rc);
    }
  }
}

void parse_options(int argc, char **argv, struct options *opts)
{
  int optionid;
  int opt = 0;
  const struct option long_options[] =
                {
                { "events",      no_argument, NULL, 'e'},
                { 0, 0, 0, 0}
                };

  // Parse command line arguments
  for (;;)
  {
    opt = getopt_long(argc, argv, "e", long_options, &optionid);

    if (opt == -1)
      break;

    switch (opt)
    {
      case 'e':
        io_log("using events server\n");
        opts->use_events = 1;
        break;

      default:
        io_log("Error: Unknown option %c\n", opt);
        usage();
        break;
    }
  }
}

l4_os3_thread_t thread;

int main(int argc, char **argv)
{
  CORBA_Environment env = dice_default_environment;
  struct options opts = {0};
  int rc;

  parse_options(argc, argv, &opts);

  if (! names_register("os2fs"))
  {
    io_log("Can't register on the name server!\n");
    return 1;
  }

  io_log("registered at the name server\n");

  // start events thread
  if (opts.use_events)
  {
    ThreadCreate((void *)event_thread, 0, THREAD_ASYNC);
    io_log("event thread started\n");
  }

  // get our thread ID
  fprov_id.thread = l4_myself();

  mythread = fprov_id;

  if ( (rc = init(&opts)) )
  {
    return rc;
  }

  // server loop
  env.malloc = (dice_malloc_func)malloc;
  env.free = (dice_free_func)free;
  os2fs_server_loop(&env);

  /* destruct */
  done();

  return 0;
}
