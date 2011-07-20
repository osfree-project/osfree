/*  OS/2 app l4env wrapper
 *  (c) osFree project, 2011
 *  
 *  This file is distributed under GNU general 
 *  public license, version 2, see http://www.gnu.org
 *  for details.
 */

/* standard C includes */
//#include <stdio.h>
#include <getopt.h>
/* L4 includes */
#include <l4/sys/types.h>
#include <l4/log/l4log.h>
#include <l4/util/stack.h>
#include <l4/util/l4_macros.h>
#include <l4/sys/kdebug.h>
#include <l4/sys/segment.h>
#include <l4/dm_mem/dm_mem.h>
#include <l4/dm_generic/consts.h>
#include <l4/l4rm/l4rm.h>
#include <l4/events/events.h>
#include <l4/generic_ts/generic_ts.h>
#include <l4/env/env.h>
#include <l4/env/errno.h>
#include <l4/log/l4log.h>
#include <l4/util/rdtsc.h>
/* OS/2 server internal includes */
#include <l4/os3/gcc_os2def.h>
#include <l4/os3/ixfmgr.h>
#include <l4/os3/processmgr.h>
/* OS/2 server RPC call includes */
#include <l4/os2srv/os2server-client.h>
/* exec server RPC call includes */
#include <l4/execsrv/os2exec-client.h>
/* DICE includes                 */
#include <dice/dice.h>
/* local includes*/
#include "stacksw.h"

/* l4rm heap address (thus, moved from 0xa000 higher) */
const l4_addr_t l4rm_heap_start_addr = 0xb9000000;
/* l4thread stack map start address */
const l4_addr_t l4thread_stack_area_addr = 0xbc000000;
/* l4thread TCB table map address */
const l4_addr_t l4thread_tcb_table_addr = 0xbe000000;
/* previous stack (when switching between 
   task and os2app stacks)        */
unsigned long __stack;
// use events server flag
char use_events = 0;

/* fs server thread id */
l4_threadid_t fs;
/* OS/2 server thread id */
l4_threadid_t os2srv;
/* exec server thread id */
l4_threadid_t execsrv;
/* entry point, stack and 
   other module parameters */
os2exec_module_t s;

//void term_wait (void);

unsigned long
PvtLoadModule(char *pszName,
              unsigned long cbName,
              char const *pszModname,
              os2exec_module_t *s,
              unsigned long *phmod);

APIRET KalGetInfoBlocks(PTIB *pptib, PPIB *pppib);
APIRET KalQueryCurrentDisk(PULONG pdisknum, PULONG plogical);
VOID KalExit(ULONG action, ULONG result);

/* trampoline() params    */
struct param
{
  /* a system-dependent structure*/
  //IXFSYSDEP *sysdep;

  /* OS/2-specific params */
  unsigned long  eip;
  unsigned long  esp;
  //unsigned short sel;
  PTIB           tib;
  PPIB           pib;
  BYTE		 curdisk;
};

/* GDT/LDT descriptor structure */
struct desc
{
  short limit_lo :16;
  short base_lo1 :16;
  short base_lo2 :8;
  short acc_lo   :8;
  short limit_hi :4;
  short acc_hi   :4;
  short base_hi  :8;
};

/* OS/2 app main thread */
int
trampoline(struct param *param)
{
  
  PCHAR argv = param->pib->pib_pchcmd;
  PCHAR envp = param->pib->pib_pchenv;
  ULONG hmod = param->pib->pib_hmte;

  unsigned long     stacksize;
  unsigned short    sel;
  unsigned long     base;
  struct desc       desc;

  struct desc *dsc;

  int  stop, start;
  int  i, k;
  char *p, *str;
  char buf[0x100];

  l4_threadid_t task;

  l4_calibrate_tsc();

  LOG("call exe: eip=%x, esp=%x, tib=%x", param->eip, param->esp, param->tib);

  LOG("sp: %x", param->tib->tib_pstack);
  LOG("sp_limit: %x", param->tib->tib_pstacklimit);
  LOG("tid: %x", param->tib->tib_ptib2->tib2_ultid);
  LOG("hmod: %x", hmod);

  //stacksize = param->tib->tib_pstack - param->tib->tib_pstacklimit;
  /* clear stack area */
  //memset((void *)param->tib->tib_pstacklimit, 0, stacksize);

  p = argv;
  p -= 2;
  while (*p) p--;
  p++;
  LOG("argv[0]=%s", p);
  
  i = 0;
  p = argv;
  while (*p) p++;
  p++;
  while (*p)
  {
    str = buf;
    while (*p != ' ' && *p) *str++ = *p++;
    while (*p == ' ') if (*p) *str++ = *p++;
    *str = '\0';
    str++;
    p++;
    i++;
    LOG("argv[%u]=%s", i, buf);
  }

  k = p - envp;

  i = 0; p = envp;
  while (*p)
  {
      LOG("envp[%u]=%s", i, p);
      p += strlen(p) + 1;
      i++;
  }

  for (i = 0; i < k; i++)
  if (envp[i])
    LOG("%c", envp[i]);
  else
    LOG("\\0");    

  task = l4_myself();

#if 1
  dsc = malloc(0x1000);

  for (i = 0; i < 512; i++)
  {
    base = i * 0x10000;

    dsc[i].limit_lo = 0xffff; dsc[i].limit_hi = 0;
    dsc[i].acc_lo   = 0xFE;   dsc[i].acc_hi = 0;
    dsc[i].base_lo1 = base & 0xffff;
    dsc[i].base_lo2 = (base >> 16) & 0xff;
    dsc[i].base_hi  = base >> 24;
  }
  LOG("---");
  start = l4_rdtsc();
  fiasco_ldt_set(dsc, 0x1000, 0, task.id.task);
  stop  = l4_rdtsc();
  LOG("+++");
  free(dsc);
  LOG("===");
  LOG("LDT switch time=%u ns", l4_tsc_to_ns(stop - start));
#endif  

  /* TIB base */
  base = param->tib;	
              
  /* Prepare TIB GDT descriptor */
  desc.limit_lo = 0x30; desc.limit_hi = 0;
  desc.acc_lo   = 0xF3; desc.acc_hi   = 0;
  desc.base_lo1 = base & 0xffff;
  desc.base_lo2 = (base >> 16) & 0xff;
  desc.base_hi  = base >> 24;
        
  /* Allocate a GDT descriptor */
  fiasco_gdt_set(&desc, sizeof(struct desc), 0, task);

  /* Get a selector */
  sel = (sizeof(struct desc)) * fiasco_gdt_get_entry_offset();
  LOG("sel=%x", sel);

  /* save the previous stack to __stack
     and set current one to OS/2 app stack */
  STKINIT(s.sp - 0x10)
  
  /* We have changed the stack so it now points to our LX image. */
  //enter_kdebug("debug");
  asm(
      "movl  %[sel], %%edx \n"
      "movw  %%dx, %%fs \n"              /* TIB selector */
      "pushl %%ebp \n"                   /* save ebp on the old stack      */
      "movl  %[argv], %%edx \n"
      "pushl %%edx \n"                   /* argv  */
      "movl  %[envp], %%edx \n"
      "pushl %%edx \n"                   /* envp  */
      "movl  $0, %%edx \n"
      "pushl %%edx \n"                   /* sizec */
      "movl  %[hmod], %%edx \n"
      "pushl %%edx \n"                   /* hmod  */
      "movl  %[eip_data], %%ecx \n"
      "call  *%%ecx \n"                  /* Call the startup code of an OS/2 executable */
      "addl  $0x10, %%esp \n"            /* clear stack            */
      "popl  %%ebp \n"                   /* restored the old ebp   */
      :
      :[argv]     "m"  (argv),
       [envp]     "m"  (envp),
       [hmod]     "m"  (hmod),
       [sel]      "m"  (sel),
       [eip_data] "m"  (param->eip));

  STKOUT

  return 0;
}

void usage(void)
{
  LOG("os2app usage:\n");
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
    KalExit(1, 1);
  }

  if ((rc = l4events_register(L4EVENTS_EXIT_CHANNEL, 15)) != 0)
  {
    LOG_Error("l4events_register failed");
    KalExit(1, 1);
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

void main (int argc, char *argv[])
{
  CORBA_Environment env = dice_default_environment;
  struct param param;
  /* Error info from LoadModule */
  char uchLoadError[260];
  unsigned long hmod;
  ULONG curdisk, map;
  PPIB ppib;
  PTIB ptib;
  unsigned long ulActual;
  char buf[1024];
  char *p = buf;
  int i, rc = 0;
  int optionid;
  int opt = 0;
  const struct option long_options[] =
                {
                { "events",      no_argument, NULL, 'e'},
		{ 0, 0, 0, 0}
                };

  if (!names_waitfor_name("os2exec", &execsrv, 30000))
    {
      LOG("Can't find os2exec on names, exiting...");
      KalExit(1, 1);
    }

  if (!names_waitfor_name("os2fs", &fs, 30000))
    {
      LOG("Can't find os2fs on names, exiting...");
      KalExit(1, 1);
    }

  if (!names_waitfor_name("os2srv", &os2srv, 30000))
    {
      LOG("Can't find os2srv on names, exiting...");
      KalExit(1, 1);
    }

  // Parse command line arguments
  for (;;)
  {
    opt = getopt_long(argc, argv, "e", long_options, &optionid);
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
        KalExit(1, 2);
    }
  }

  // start events thread
  if (use_events)
  {
    // start events thread
    l4thread_create(event_thread, 0, L4THREAD_CREATE_ASYNC);
    LOG("event thread started");
  }

  /* Load the LX executable */
  rc = PvtLoadModule(uchLoadError, sizeof(uchLoadError), 
                     argv[argc - 1], &s, &hmod);
  
  if (rc)
  {
    LOG("LX load error!");
    KalExit(1, 1);
  }

  LOG("LX loaded successfully");

  param.eip = s.ip;
  param.esp = s.sp;

  strcpy(s.path, argv[argc - 1]);

  /* notify OS/2 server about parameters got from execsrv */
  os2server_app_notify_call (&os2srv, &s, &env);

  STKINIT(__stack - 0x800)

  rc = KalQueryCurrentDisk(&curdisk, &map);

  if (rc)
    LOG("Cannot get the current disk!");

  param.curdisk = curdisk;

  /* get the info blocks (needed by C startup code) */
  rc = KalGetInfoBlocks(&ptib, &ppib);

  param.pib = ppib;
  param.tib = ptib;

  l4rm_show_region_list();

  // write PID to the screen
  sprintf(p, "The process id is %x\n", ppib->pib_ulpid);
  KalWrite(1, p, strlen(p) + 1, &ulActual);

  LOG("Starting %s LX exe...", argv[argc - 1]);
  rc = trampoline (&param);
  LOG("... %s finished.", argv[argc - 1]);

  STKOUT

  /* wait for our termination */
  KalExit(1, 0); // successful return
}
