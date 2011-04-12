/*  OS/2 app l4env wrapper
 *  (c) osFree project, 2011
 *  
 *  This file is distributed under GNU general 
 *  public license, version 2, see http://www.gnu.org
 *  for details.
 */

/* standard C includes */
//#include <stdio.h>
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
#include <l4/env/env.h>
#include <l4/env/errno.h>
#include <l4/log/l4log.h>
/* OS/2 server internal includes */
#include <l4/os3/gcc_os2def.h>
#include <l4/os3/ixfmgr.h>
#include <l4/os3/processmgr.h>
/* OS/2 server RPC call includes */
#include <l4/os3/os2server-client.h>
/* exec server RPC call includes */
#include <l4/execsrv/os2exec-client.h>
/* DICE includes                 */
#include <dice/dice.h>

/* l4rm heap address (thus, moved from 0xa000 higher) */
const l4_addr_t l4rm_heap_start_addr = 0xb9000000;
/* l4thread stack map start address */
const l4_addr_t l4thread_stack_area_addr = 0xbc000000;
/* l4thread TCB table map address */
const l4_addr_t l4thread_tcb_table_addr = 0xbe000000;

/* OS/2 server thread id */
l4_threadid_t os2srv;
/* exec server thread id */
l4_threadid_t execsrv;
/* entry point, stack and 
   other module parameters */
os2exec_module_t s;

void term_wait (void);
int  trampoline(struct param *param);

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

  unsigned short    sel;
  unsigned long     base;
  struct desc       desc;

  int  i;
  char *p;

  LOG("call exe: eip=%x, esp=%x, tib=%x", param->eip, param->esp, param->tib);

  LOG("sp: %x", param->tib->tib_pstack);
  LOG("sp_limit: %x", param->tib->tib_pstacklimit);
  LOG("tid: %x", param->tib->tib_ptib2->tib2_ultid);

  LOG("hmod: %x", hmod);

  i = 0; p = argv;
  while (*p)	
  {
    LOG("argv[%u]=%s", i, p);
    p += strlen(p) + 1;
    i++;
  }

  i = 0; p = envp;
  while (*p)
  {
    LOG("envp[%u]=%s", i, p);
    p += strlen(p) + 1;
    i++;
  }
  
  /* TIB base */
  base = param->tib;	
              
  /* Prepare TIB GDT descriptor */
  desc.limit_lo = 0x30; desc.limit_hi = 0;
  desc.acc_lo   = 0xF3; desc.acc_hi   = 0;
  desc.base_lo1 = base & 0xffff;
  desc.base_lo2 = (base >> 16) & 0xff;
  desc.base_hi  = base >> 24;
        
  /* Allocate a GDT descriptor */
  fiasco_gdt_set(&desc, sizeof(struct desc), 0, l4_myself());

  /* Get a selector */
  sel = (sizeof(struct desc)) * fiasco_gdt_get_entry_offset();
  LOG("sel=%x", sel);

  //enter_kdebug("debug");
  asm(
      "movl  %[sel], %%edx \n"
      "movw  %%dx, %%fs \n"              /* TIB selector */
      "movl  %[eip_data], %%ecx \n"
      "movl  %[esp_data], %%edx \n"      /* Put old esp in eax */
      "pushl %%ebp \n"                   /* save ebp on the old stack      */
      "movl  %%esp, %%eax \n"            /* Copy esp to eax. Stack pointer */
      "movl  %%edx, %%esp \n"            /* Copy edx to esp. Stack pointer */
      "pushl %%eax \n"                   /* old esp on the new stack       */
      /* We have changed the stack so it now points to our LX image. */
      "movl  %[argv], %%edx \n"
      "pushl %%edx \n"                   /* argv  */
      "movl  %[envp], %%edx \n"
      "pushl %%edx \n"                   /* envp  */
      "movl  $0, %%edx \n"
      "pushl %%edx \n"                   /* sizec */
      "movl  %[hmod], %%edx \n"
      "pushl %%edx \n"                   /* hmod  */
      "movl  %%esp, %%ebp \n"
      "call  *%%ecx \n"                  /* Call the startup code of an OS/2 executable */
      "addl  $0x10, %%esp \n"            /* clear stack            */
      "popl  %%esp \n"                   /* restored the esp from the old stack */
      "popl  %%ebp \n"                   /* restored the old ebp   */
      :
      :[sel]      "m" (sel),
       [argv]     "m" (argv),
       [envp]     "m" (envp),
       [hmod]     "m" (hmod),
       [esp_data] "m" (param->esp),       /* esp+ data_mmap+8+ */
       [eip_data] "m" (param->eip));

      return 0;
}
 
void main (int argc, char *argv[])
{
  CORBA_Environment env = dice_default_environment;
  struct param param;
  /* Error info from LoadModule */
  char uchLoadError[260];
  unsigned long hmod;
  ULONG curdisk = 2, map = 1 << 2;
  PPIB ppib;
  PTIB ptib;
  char buf[1024];
  char *p = buf;
  int i, rc = 0;

  if (!names_waitfor_name("os2exec", &execsrv, 30000))
    {
      LOG("Can't find os2exec on names, exiting...");
      return;
    }

  if (!names_waitfor_name("os2srv", &os2srv, 30000))
    {
      LOG("Can't find os2srv on names, exiting...");
      return;
    }

  /* Load the LX executable */
  rc = PvtLoadModule(uchLoadError, sizeof(uchLoadError), 
                     argv[1], &s, &hmod);
  
  if (rc)
  {
    LOG("LX load error!");
    KalExit(1, 1);
  }

  LOG("LX loaded successfully");

  param.eip = s.ip;
  param.esp = s.sp;

  /* notify OS/2 server about parameters got from execsrv */
  os2server_app_notify_call (&os2srv, &s, &env);

  rc = KalQueryCurrentDisk(&curdisk, &map);

  if (rc)
    LOG("Cannot get the current disk!");

  param.curdisk = curdisk;

  /* get the info blocks (needed by C startup code) */
  rc = KalGetInfoBlocks(&ptib, &ppib);

  param.pib = ppib;
  param.tib = ptib;
   
  l4rm_show_region_list();

  LOG("Starting %s LX exe...", argv[1]);
  rc = trampoline (&param);
  LOG("... %s finished.", argv[1]);

  /* wait for our termination */
  KalExit(1, 0); // successful return
}
