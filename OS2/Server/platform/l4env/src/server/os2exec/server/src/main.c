/*
 *
 *
 */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/dataspace.h>
#include <os3/thread.h>
#include <os3/types.h>
#include <os3/io.h>

/* l4env includes */
#include <l4/env/env.h>
#include <l4/events/events.h>
#include <l4/names/libnames.h>

/* os2exec RPC includes */
#include <os2exec-server.h>

/* libc includes */
#include <stdlib.h>
#include <getopt.h>

/* execsrv link address */
const l4_addr_t execsrv_start_addr = 0xba000000;
/* l4rm heap address (thus, moved from 0xa000 higher) */
const l4_addr_t l4rm_heap_start_addr = 0xb9000000;
/* l4thread stack map start address */
const l4_addr_t l4thread_stack_area_addr = 0xbc000000;
/* l4thread TCB table map address */
const l4_addr_t l4thread_tcb_table_addr = 0xbe000000;

/* shared memory arena settings */
extern void           *shared_memory_base;
extern unsigned long  shared_memory_size;
extern unsigned long long shared_memory_area;

extern l4_os3_thread_t thread;

extern l4_os3_thread_t os2srv;
//extern l4_os3_thread_t fs;
l4_threadid_t loader;

l4env_infopage_t *infopage;

// use events server flag
char use_events = 0;

struct options
{
  char use_events;
  char *configfile;
  char *kal_map;
};

void usage(void);
void event_thread(void);
void reserve_regions(void);
void parse_options(int argc, char *argv[], struct options *opts);
int init(struct options *opts);
void done(void);

long l4loader_app_info_call(CORBA_Object obj,
                            unsigned long task,
                            unsigned long flags,
                            char **fname,
                            l4dm_dataspace_t *ds,
                            CORBA_Environment *env);

void usage(void)
{
  io_log("execsrv usage:\n");
  io_log("-e:  Use events server");
}

void event_thread(void)
{
  l4events_ch_t event_ch = L4EVENTS_EXIT_CHANNEL;
  l4events_nr_t event_nr = L4EVENTS_NO_NR;
  l4events_event_t event;
  l4_threadid_t tid;
  int rc;

  if (! l4events_init() )
  {
    io_log("l4events_init() failed\n");
    return;
  }

  if ( (rc = l4events_register(L4EVENTS_EXIT_CHANNEL, 15)) != 0)
  {
    io_log("l4events_register failed\n");
    return;
  }

  while(1)
  {
    /* wait for event */
    if ( (rc = l4events_give_ack_and_receive(&event_ch, &event, &event_nr,
                                            L4_IPC_NEVER,
                                            L4EVENTS_RECV_ACK)) < 0)
    {
      io_log("l4events_give_ack_and_receive()\n");
      continue;
    }
    tid = *(l4_threadid_t *)event.str;
    io_log("Got exit event for %x.%x\n", tid.id.task, tid.id.lthread);

    /* exit myself */
    if (l4_task_equal(tid, os2srv.thread))
    {
      exit(rc);
    }
  }
}

void reserve_regions(void)
{
  void *addr;
  unsigned long size;
  int rc;

  /* reserve the area below 64 Mb for application private code
     (not for use by libraries, loaded by execsrv) */
  addr = (void *)0x2f000; size = 0x04000000 - (unsigned long)addr;
  if ( (rc  = l4rm_direct_area_setup_region((l4_addr_t)addr,
                                           size,
                                           L4RM_DEFAULT_REGION_AREA,
                                           L4RM_REGION_BLOCKED, 0,
                                           L4_INVALID_ID)) < 0 )
  {
    io_log("main(): setup region %x-%x failed (%d)!\n",
           addr, addr + size, rc);
    l4rm_show_region_list();
    enter_kdebug("PANIC");
  }

  // reserve the upper 1 Gb for shared memory arena
  rc = l4rm_area_reserve_region((l4_addr_t)shared_memory_base,
                                shared_memory_size, 0,
                                (l4_uint32_t *)&shared_memory_area);
  if (rc < 0)
  {
    io_log("Panic: cannot reserve memory for shared arena!\n");
    return;
  }
}

void parse_options(int argc, char *argv[], struct options *opts)
{
  int optionid;
  int opt = 0;
  const struct option long_options[] =
                {
                { "events",      no_argument, NULL, 'e'},
                { 0, 0, 0, 0}
                };

  opts->configfile = "c:\\config.sys";
  opts->kal_map = "c:\\kal.map";

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
        return;
    }
  }
}

int main (int argc, char *argv[])
{
  CORBA_Server_Environment env = dice_default_server_environment;
  CORBA_Environment e = dice_default_environment;
  l4_os3_dataspace_t ds;
  struct options opts = {0};
  char buf[0x1000];
  char *p = buf;
  int  rc;

  parse_options(argc, argv, &opts);

  if (! names_register("os2exec"))
  {
    io_log("Error registering on the name server!\n");
    return -1;
  }

  if (! names_waitfor_name("LOADER", &loader, 30000))
  {
    io_log("LOADER not found on name server!\n");
    return -1;
  }

  if (opts.use_events)
  {
    // start events thread
    ThreadCreate((void *)event_thread, 0, THREAD_ASYNC);
    io_log("event thread started\n");
  }

  thread.thread = l4_myself();

  /* get our l4env infopage as a dataspace */
  rc = l4loader_app_info_call(&loader, thread.thread.id.task,
                              0, &p, &ds.ds, &e);
  /* attach it */
  attach_ds(ds, DATASPACE_RO, (void **)&infopage);

  /* call platform-independent init */
  if ( (rc = init(&opts)) )
  {
    return rc;
  }

  // server loop
  io_log("execsrv started.\n");
  env.malloc = (dice_malloc_func)malloc;
  env.free = (dice_free_func)free;
  os2exec_server_loop(&env);

  /* destruct */
  done();

  return 0;
}
