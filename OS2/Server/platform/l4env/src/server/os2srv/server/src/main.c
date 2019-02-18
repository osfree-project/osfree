/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/io.h>
#include <os3/thread.h>

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

l4_threadid_t fs;
l4_threadid_t os2srv;
l4_os3_thread_t fprov_id;
l4_os3_thread_t dsm_id;
l4_os3_thread_t loader_id;

struct options
{
  char use_events;
  char *configfile;
  char *bootdrive;
  char fprov[20];
};

/* whether to use event server */
char use_events = 0;

l4_os3_thread_t CPNativeID(void);

void parse_options(int argc, char **argv, struct options *opts);
void exit_notify(void);
int init(struct options *opts);
void done(void);
void usage(void);

l4_os3_thread_t CPNativeID(void)
{
  l4_os3_thread_t thread;

  thread.thread = l4_myself();

  return thread;
}

void exit_notify()
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

void parse_options(int argc, char **argv, struct options *opts)
{
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

  strcpy(opts->fprov, "BMODFS");

  // Parse command line arguments
  for (;;)
    {
      opt = getopt_long(argc, (char **)argv, "s:c:b:f:e", long_options, &optionid);

      if (opt == -1)
        break;

      switch (opt)
        {
        case 'c':
            opts->configfile = malloc(strlen(optarg)+1);
            strcpy(opts->configfile, optarg);
            break;

        case 'b':
            opts->bootdrive = malloc(strlen(optarg)+1);
            strcpy(opts->bootdrive, optarg);
            break;

        case 'f':
            io_log("fprov is %s\n", optarg);
            strcpy(opts->fprov, optarg);
            break;

        case 'e':
            io_log("using events server\n");
            opts->use_events = 1;
            use_events = 1;
            break;

        case -1:
            // prevents showing fstab_info if no more option exists
            break;

        default:
            io_log("Error: Unknown option \"%c\"\n", opt);
            usage();
            return;

        }
    }
}

int main(int argc, char **argv)
{
  CORBA_Server_Environment env = dice_default_server_environment;
  struct options opts = {0};
  int rc; // Return code

  /* parse options */
  parse_options(argc, argv, &opts);

  if (! names_register("os2srv") )
  {
    io_log("Error registering on the name server\n");
    return 1;
  }

  /* Wait for servers to be started */
  if (! names_waitfor_name(opts.fprov, &fprov_id.thread, 10000))
  {
    io_log("Server \"%s\" not found\n", opts.fprov);
    return 1;
  }

  io_log("waiting for LOADER server to be started...\n");

  if (! names_waitfor_name("LOADER", &loader_id.thread, 30000))
    {
      io_log("Dynamic loader LOADER not found -- terminating\n");
      while (1) l4_sleep(0.1);
    }

  io_log("loader id: %x.%x", loader_id.thread.id.task, loader_id.thread.id.lthread);

  /* query default dataspace manager id */
  dsm_id.thread = l4env_get_default_dsm();

  if (l4_is_invalid_id(dsm_id.thread))
  {
    printf("No dataspace manager found\n");
    return 2;
  }

  CPClientInit(&os2srv);

  /* call platform-independent init */
  if ( (rc = init(&opts)) )
  {
    CPClientDone();
    return rc;
  }

  /* server loop */
  env.malloc = (dice_malloc_func)malloc;
  env.free = (dice_free_func)free;
  os2server_server_loop(&env);

  CPClientDone();

  /* destruct */
  done();

  return NO_ERROR;
}
