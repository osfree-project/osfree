/*  OS personality app l4env wrapper
 *  (c) osFree project, 2011
 *  
 *  This file is distributed under GNU general 
 *  public license, version 2, see http://www.gnu.org
 *  for details.
 */

/* standard C includes */
#include <getopt.h>
/* L4 includes */
#include <l4/util/rdtsc.h>
#include <l4/dm_phys/dm_phys.h>
#include <l4/l4rm/l4rm.h>
#include <l4/names/libnames.h>
#include <l4/generic_ts/generic_ts.h>
/* OS/2 server internal includes */
#include <l4/os3/gcc_os2def.h>
#include <l4/os3/ixfmgr.h>
#include <l4/os3/processmgr.h>
#include <l4/os3/segment.h>
#include <l4/os3/types.h>
#include <l4/os3/ipc.h>
#include <l4/os3/kal.h>
//#include <l4/os3/dl.h>
#include <l4/os3/io.h>
/* local includes*/
#include <stacksw.h>
/* OS/2 server call includes */
#include <l4/os2srv/os2server-client.h>

#include "os2app-server.h"

extern char LOG_tag[9];

/* l4rm heap address (thus, moved from 0xa000 higher) */
const l4_addr_t l4rm_heap_start_addr = 0xb9000000;
/* l4thread stack map start address */
const l4_addr_t l4thread_stack_area_addr = 0xbc000000;
/* l4thread TCB table map address */
const l4_addr_t l4thread_tcb_table_addr = 0xbe000000;

/* private memory arena settings */
l4_addr_t   private_memory_base = 0x10000;
l4_size_t   private_memory_size = 512*1024*1024;
l4_uint32_t private_memory_area;

/* shared memory arena settings */
l4_addr_t   shared_memory_base = 0x60000000;
l4_size_t   shared_memory_size = 1024*1024*1024;
l4_uint32_t shared_memory_area;

vmdata_t **pareas_list = NULL;

// use events server flag
char use_events = 0;
/* previous stack (when switching between 
   task and os2app stacks)        */
unsigned long __stack;

/* OS/2 server id        */
l4os3_cap_idx_t os2srv;
/* FS server id        */
l4os3_cap_idx_t fs;
/* exec server id        */
l4os3_cap_idx_t execsrv;
/* dataspace manager id  */
l4os3_cap_idx_t dsm;
/* l4env infopage        */
l4env_infopage_t infopg;
extern l4env_infopage_t *l4env_infopage;
/* file provider name    */
char fprov[20] = "fprov_proxy_fs";
/* file provider id      */
l4os3_cap_idx_t fprov_id;

l4_uint32_t service_lthread;

char pszLoadError[260];
ULONG rcCode;

void usage(void);
VOID CDECL __exit(ULONG action, ULONG result);

void event_thread(void);

void __fiasco_gdt_set(void *desc, unsigned int size,
                      unsigned int entry_number_start, l4os3_cap_idx_t tid);

unsigned __fiasco_gdt_get_entry_offset(void);

ULONG kalHandle;

struct kal_init_struct initstr;

void usage(void)
{
  io_log("os2app usage:\n");
  io_log("-e:  Use events server");
} 

VOID CDECL
__exit(ULONG action, ULONG result)
{
  CORBA_Environment env = dice_default_environment;
  STKIN
  // send OS/2 server a message that we want to terminate
  io_log("action=%lu\n", action);
  io_log("result=%lu\n", result);
  os2server_dos_Exit_send(&os2srv, action, result, &env);
  // tell L4 task server that we want to terminate
  //l4_ipc_sleep(L4_IPC_NEVER);
  l4ts_exit();
  STKOUT
}

#if 0
void event_thread(void)
{
  l4events_ch_t event_ch = L4EVENTS_EXIT_CHANNEL;
  l4events_nr_t event_nr = L4EVENTS_NO_NR;
  l4events_event_t event;
  l4_threadid_t tid;
  int rc;

  if (!l4events_init())
  {
    io_log("l4events_init() failed\n");
    __exit(1, 1);
  }

  if ((rc = l4events_register(L4EVENTS_EXIT_CHANNEL, 15)) != 0)
  {
    io_log("l4events_register failed\n");
    __exit(1, 1);
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
      __exit(1, rc);
  }
}
#endif

void __fiasco_gdt_set(void *desc, unsigned int size,
                      unsigned int entry_number_start, l4_threadid_t tid)
{
  l4os3_gdt_set(desc, size, entry_number_start, tid);
}

unsigned __fiasco_gdt_get_entry_offset(void)
{
  return l4os3_gdt_get_entry_offset();
}

void server_loop(void)
{
  CORBA_srv_env env = default_srv_env;
  // server loop
  env.malloc = (dice_malloc_func)malloc;
  env.free = (dice_free_func)free;
  os2app_server_loop(&env);
}

int main (int argc, char *argv[])
{
  CORBA_srv_env env = default_srv_env;
  l4thread_t thread;
  l4_threadid_t tid;
  //l4_uint32_t area;
  //struct desc *dsc;
  //int i, stop, start;
  //unsigned long base;
  //l4_threadid_t task;
  int rc = 0;
  int optionid;
  int opt = 0;

  const struct option long_options[] =
                {
                { "events",      no_argument, NULL, 'e'},
		{ 0, 0, 0, 0}
                };

  if (!names_waitfor_name("os2srv", &os2srv, 30000))
    {
      io_log("Can't find os2srv on names, exiting...\n");
      __exit(1, 1);
    }

  if (!names_waitfor_name("os2fs", &fs, 30000))
    {
      io_log("Can't find os2fs on names, exiting...\n");
      __exit(1, 1);
    }

  if (!names_waitfor_name("os2exec", &execsrv, 30000))
    {
      io_log("Can't find os2exec on names, exiting...\n");
      __exit(1, 1);
    }

  if (!names_waitfor_name(fprov, &fprov_id, 30000))
    {
      io_log("Can't find %s on names, exiting...\n", fprov);
      __exit(1, 1);
    }

  // reserve the lower 64 Mb for OS/2 app
  rc = l4rm_area_reserve_region(private_memory_base, private_memory_size, 0, &private_memory_area);
  if (rc < 0)
  {
    io_log("Panic: cannot reserve memory for private arena!\n");
    __exit(1, 1);
  }

  // reserve the upper 1 Gb for shared memory arena
  rc = l4rm_area_reserve_region(shared_memory_base, shared_memory_size, 0, &shared_memory_area);
  if (rc < 0)
  {
    io_log("Panic: cannot reserve memory for shared arena!\n");
    __exit(1, 1);
  }

  /* query default dataspace manager id */
  dsm = l4env_get_default_dsm();
  if (l4_is_invalid_id(dsm))
  {
    io_log("No dataspace manager found\n");
    __exit(1, 1);
  }
  io_log("dsm=%u.%u\n", dsm.id.task, dsm.id.lthread);
  io_log("frov_id=%u.%u\n", fprov_id.id.task, fprov_id.id.lthread);

  l4env_infopage = &infopg;
  l4env_infopage->fprov_id = fprov_id;
  l4env_infopage->memserv_id = dsm;

  //if ((rc = DlOpen("/file/system/libkal.s.so", &kalHandle)))
  //{
    //io_log("Can't load libkal.s.so!\n");
    //__exit(1, 1);
  //}

  //io_log("kalHandle=%lu\n", kalHandle);

  // start server loop
  thread = l4thread_create((void *)server_loop, 0, L4THREAD_CREATE_ASYNC);
  tid = l4thread_l4_id(thread);
  service_lthread = tid.id.lthread;

  //fill in the parameter structure for KalInit
  /* initstr.stack   = __stack;
  initstr.shared_memory_base = shared_memory_base;
  initstr.shared_memory_size = shared_memory_size;
  initstr.shared_memory_area = shared_memory_area;
  initstr.service_lthread    = service_lthread;
  initstr.l4rm_do_attach = l4rm_do_attach;
  initstr.l4rm_detach = l4rm_detach;
  initstr.l4rm_lookup        = l4rm_lookup;
  initstr.l4rm_lookup_region = l4rm_lookup_region;
  initstr.l4rm_do_reserve  = l4rm_do_reserve;
  initstr.l4rm_do_reserve_in_area  = l4rm_do_reserve_in_area;
  initstr.l4rm_set_userptr   = l4rm_set_userptr; 
  initstr.l4rm_get_userptr   = l4rm_get_userptr;
  initstr.l4rm_area_release  = l4rm_area_release;
  initstr.l4rm_area_release_addr = l4rm_area_release_addr;
  initstr.l4rm_show_region_list = l4rm_show_region_list;
  initstr.l4dm_memphys_copy = l4dm_memphys_copy;
  initstr.l4env_get_default_dsm  = l4env_get_default_dsm;
  initstr.l4thread_exit = l4thread_exit;
  initstr.l4thread_on_exit = l4thread_on_exit;
  initstr.l4thread_create_long = l4thread_create_long;
  initstr.l4thread_shutdown = l4thread_shutdown;
  initstr.l4thread_l4_id = l4thread_l4_id;
  initstr.l4thread_get_parent = l4thread_get_parent;
  initstr.fiasco_gdt_set         = __fiasco_gdt_set;
  initstr.fiasco_gdt_get_entry_offset = __fiasco_gdt_get_entry_offset;
  initstr.logtag = LOG_tag;

  // init kal.dll
  DlRoute(0, "KalInit", &initstr); */

  // Parse command line arguments
  for (;;)
  {
    opt = getopt_long(argc, argv, "e", long_options, &optionid);
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
        __exit(1, 2);
    }
  }

  /* task = l4_myself();

  dsc = malloc(0x8);

  for (i = 0; i < 512; i++)
  {
    base = i * 0x10000;

    dsc[0].limit_lo = 0xffff; dsc[0].limit_hi = 0;
    dsc[0].acc_lo   = 0xFE;   dsc[0].acc_hi = 0;
    dsc[0].base_lo1 = base & 0xffff;
    dsc[0].base_lo2 = (base >> 16) & 0xff;
    dsc[0].base_hi  = base >> 24;
    io_log("--- %d\n", i);
    fiasco_ldt_set(dsc, 0x8, i, task.id.task);
  }
  io_log("---\n");
  start = l4_rdtsc();
  //fiasco_ldt_set(dsc, 0x10000, 0, task.id.task);
  stop  = l4_rdtsc();
  io_log("+++\n");
  free(dsc);
  io_log("===\n");
  io_log("LDT switch time=%u ns\n", l4_tsc_to_ns(stop - start)); */

  // start events thread
  /* if (use_events)
  {
    // start events thread
    l4thread_create((void *)event_thread, 0, L4THREAD_CREATE_ASYNC);
    io_log("event thread started\n");
  } */

  //DlSym(kalHandle, "areas_list", &pareas_list);

  // release the reserved area for application
  //rc = l4rm_area_release(holdarea);

  io_log("calling KalStartApp...\n");
  //rcCode = DlRoute(0, "KalStartApp", argv[argc - 1], pszLoadError, sizeof(pszLoadError));
  KalStartApp(argv[argc - 1], pszLoadError, sizeof(pszLoadError));

  // server loop
  //env.malloc = (dice_malloc_func)malloc;
  //env.free = (dice_free_func)free;
  //os2app_server_loop(&env);

  return 0;
}
