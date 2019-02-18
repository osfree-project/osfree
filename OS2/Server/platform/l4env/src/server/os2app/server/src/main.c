/*  OS personality app l4env wrapper
 *  (c) osFree project, 2011
 *
 *  This file is distributed under GNU general 
 *  public license, version 2, see http://www.gnu.org
 *  for details.
 */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal includes */
#include <os3/io.h>
#include <os3/kal.h>

/* l4env includes */
//#include <l4/util/rdtsc.h>
#include <l4/dm_phys/dm_phys.h>
#include <l4/l4rm/l4rm.h>
#include <l4/names/libnames.h>
#include <l4/events/events.h>

/* servers RPC call includes */
#include <l4/os2srv/os2server-client.h>
#include "os2app-server.h"

/* libc includes */
#include <stdlib.h>
#include <getopt.h>

extern char LOG_tag[9];

/* l4rm heap address (thus, moved from 0xa000 higher) */
const l4_addr_t l4rm_heap_start_addr = 0xb9000000;
/* l4thread stack map start address */
const l4_addr_t l4thread_stack_area_addr = 0xbc000000;
/* l4thread default stack size */
const l4_size_t l4thread_stack_size = 0x20000;
/* l4thread max stack size */
const l4_size_t l4thread_max_stack = 0x200000;
/* l4thread TCB table map address */
const l4_addr_t l4thread_tcb_table_addr = 0xbe000000;

/* private memory arena settings */
extern ULONG   private_memory_base;
extern ULONG   private_memory_size;
extern ULONGLONG private_memory_area;

/* shared memory arena settings */
extern ULONG   shared_memory_base;
extern ULONG   shared_memory_size;
extern ULONGLONG shared_memory_area;

/* OS/2 server id        */
l4_os3_thread_t os2srv;
/* FS server id        */
l4_threadid_t fs;
/* dataspace manager id  */
l4_threadid_t dsm;
/* l4env infopage        */
l4env_infopage_t infopg;
extern l4env_infopage_t *l4env_infopage;
/* file provider name    */
char fprov[20] = "fprov_proxy_fs";
/* file provider id      */
l4_os3_thread_t fprov_id;

extern l4_os3_thread_t thread;

void usage(void);
void server_loop(void);

VOID CDECL Exit(ULONG action, ULONG result);

void event_thread(void);

struct options
{
  char  use_events;
  char  *progname;
};

int init(struct options *opts);
void done(void);
void reserve_regions(void);
void parse_options(int argc, char *argv[], struct options *opts);

void usage(void)
{
  io_log("os2app usage:\n");
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
    Exit(1, 1);
  }

  if ((rc = l4events_register(L4EVENTS_EXIT_CHANNEL, 15)) != 0)
  {
    io_log("l4events_register failed\n");
    Exit(1, 1);
  }

  while(1)
  {
    /* wait for event */
    if ((rc = l4events_give_ack_and_receive(&event_ch, &event, &event_nr,
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
      Exit(1, rc);
  }
}

void server_loop(void)
{
  CORBA_Server_Environment env = dice_default_server_environment;
  // server loop
  env.malloc = (dice_malloc_func)malloc;
  env.free = (dice_free_func)free;
  os2app_server_loop(&env);
}

void reserve_regions(void)
{
  int rc;

  // reserve the lower 64 Mb for OS/2 app
  rc = l4rm_area_reserve_region(private_memory_base, private_memory_size, 0, (l4_uint32_t *)&private_memory_area);
  if (rc < 0)
  {
    io_log("Panic: cannot reserve memory for private arena!\n");
    Exit(1, 1);
  }

  // reserve the upper 1 Gb for shared memory arena
  rc = l4rm_area_reserve_region(shared_memory_base, shared_memory_size, 0, (l4_uint32_t *)&shared_memory_area);
  if (rc < 0)
  {
    io_log("Panic: cannot reserve memory for shared arena!\n");
    Exit(1, 1);
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

  // Parse command line arguments
  for (;;)
  {
    opt = getopt_long(argc, argv, "e", long_options, &optionid);
    if (opt == -1) break;
    switch (opt)
    {
      case 'e':
        io_log("using events server\n");
        opts->use_events = 1;
        break;

      default:
        io_log("Error: Unknown option %c\n", opt);
        usage();
        Exit(1, 2);
    }
  }

  opts->progname = argv[argc - 1];
}

int main (int argc, char *argv[])
{
  struct options opts = {0};
  int rc;

  parse_options(argc, argv, &opts);

  if (! names_waitfor_name(fprov, &fprov_id.thread, 30000))
  {
    io_log("Can't find %s on names, exiting...\n", fprov);
    Exit(1, 1);
  }

  /* query default dataspace manager id */
  dsm = l4env_get_default_dsm();

  if (l4_is_invalid_id(dsm))
  {
    io_log("No dataspace manager found\n");
    Exit(1, 1);
  }

  io_log("dsm=%u.%u\n", dsm.id.task, dsm.id.lthread);
  io_log("frov_id=%u.%u\n", fprov_id.thread.id.task, fprov_id.thread.id.lthread);

  l4env_infopage = &infopg;
  l4env_infopage->fprov_id = fprov_id.thread;
  l4env_infopage->memserv_id = dsm;

  // start server loop
  thread = ThreadCreate((void *)server_loop, 0, THREAD_ASYNC);

  if (opts.use_events)
  {
    // start events thread
    ThreadCreate((void *)event_thread, 0, THREAD_ASYNC);
    io_log("event thread started\n");
  }

  /* start platform-independent init */
  rc = init(&opts);

  /* destruct */
  done();

  return rc;
}
