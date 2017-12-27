/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal includes */
#include <os3/ixfmgr.h>
#include <os3/processmgr.h>
#include <os3/stacksw.h>
#include <os3/loader.h>
#include <os3/dataspace.h>
//#include <os3/apistub.h>
#include <os3/types.h>
#include <os3/kal.h>
//#include <os3/dl.h>
#include <os3/io.h>

/* l4env includes */
#include <l4/names/libnames.h>
#include <l4/sys/segment.h>

/* OS/2 server RPC call includes */
#include <l4/os2srv/os2server-client.h>

/* exec server RPC call includes */
#include <l4/os2exec/os2exec-client.h>

/* DICE includes                 */
#include <dice/dice.h>

/* libc includes */
#include <stdio.h> // sprintf

/* fs server thread id   */
extern l4_os3_cap_idx_t fs;
/* OS/2 server thread id */
extern l4_os3_cap_idx_t os2srv;
/* exec server thread id */
extern l4_os3_cap_idx_t execsrv;

extern unsigned long __stack;
/* application info blocks */
PTIB ptib[128];
PPIB ppib;
/* entry point, stack and 
   other module parameters */
os2exec_module_t s;
/* old FS selector value   */
extern unsigned short old_sel;
/* new TIB FS selector     */
extern unsigned short tib_sel;

extern vmdata_t *areas_list;

extern ULONG rcCode;

/* OS/2 app main thread */
int
trampoline(struct param *param)
{
  PCHAR argv = param->pib->pib_pchcmd;
  PCHAR envp = param->pib->pib_pchenv;
  ULONG hmod = param->pib->pib_hmte;

  //unsigned long     stacksize;
  //unsigned short    sel;
  unsigned long     base;
  struct desc       desc;

  l4_os3_cap_idx_t task;

  task = l4_myself();

  /* TIB base */
  base = (unsigned long)param->tib;
  io_log("ptib[0]=%x\n", ptib[0]);

  /* Prepare TIB GDT descriptor */
  desc.limit_lo = 0x30; desc.limit_hi = 0;
  desc.acc_lo   = 0xF3; desc.acc_hi   = 0;
  desc.base_lo1 = base & 0xffff;
  desc.base_lo2 = (base >> 16) & 0xff;
  desc.base_hi  = base >> 24;

  /* Allocate a GDT descriptor */
  fiasco_gdt_set(&desc, sizeof(struct desc), 0, task);

  /* Get a selector */
  tib_sel = (sizeof(struct desc)) * fiasco_gdt_get_entry_offset();
  tib_sel |= 3; // ring3 GDT descriptor
  io_log("sel=%x\n", tib_sel);

  /* save the previous stack to __stack
     and set current one to OS/2 app stack */
  STKINIT(s.sp - 0x10)

  /* We have changed the stack so it now points to our LX image. */
  //enter_kdebug("debug");
  asm(
      "movw  %%fs, %%dx \n"
      "movw  %%dx, %[old_sel] \n"
      "movw  %[tib_sel], %%dx \n"
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
      :[old_sel]  "=r" (old_sel)
      :[argv]     "m"  (argv),
       [envp]     "m"  (envp),
       [hmod]     "m"  (hmod),
       [tib_sel]  "m"  (tib_sel),
       [eip_data] "m"  (param->eip));

  STKOUT

  return 0;
}

APIRET CDECL KalStartApp(char *name, char *pszLoadError, ULONG cbLoadError)
{
  CORBA_Environment env = dice_default_environment;
  vmdata_t *ptr;
  struct param param;
  APIRET rc;
  /* Error info from LoadModule */
  //char uchLoadError[260];
  unsigned long hmod;
  ULONG curdisk, map;
  unsigned long ulActual;
  char buf[1024];
  char *p = buf;
  int i;

  /* notify OS/2 server about parameters got from execsrv */
  os2server_app_notify1_call (&os2srv, &env);

  /* Load the LX executable */
  rc = KalPvtLoadModule(pszLoadError, &cbLoadError,
                        name, &s, &hmod);

  rcCode = rc;

  if (rc)
  {
    io_log("LX load error!\n");
    KalExit(1, 1);
  }

  io_log("LX loaded successfully\n");

  param.eip = s.ip;
  param.esp = s.sp;

  strcpy(s.path, name);

  /* notify OS/2 server about parameters got from execsrv */
  os2server_app_notify2_call (&os2srv, &s, &env);

  STKINIT(__stack - 0x800)

  rc = KalQueryCurrentDisk(&curdisk, &map);

  if (rc)
  {
    io_log("Cannot get the current disk!\n");
    return rc;
  }

  param.curdisk = curdisk;

  /* get the info blocks (needed by C startup code) */
  rc = KalMapInfoBlocks(&ptib[0], &ppib);

  // initialize TIB pointers array
  for (i = 1; i < 128; i++)
    ptib[i] = NULL;

  param.pib = ppib;
  param.tib = ptib[0];

  l4rm_show_region_list();

  // write PID to the screen
  sprintf(p, "The process id is %lx\n", ppib->pib_ulpid);
  KalWrite(1, p, strlen(p) + 1, &ulActual);

  io_log("Starting %s LX exe...\n", name);
  rc = trampoline (&param);
  io_log("... %s finished.\n", name);

  STKOUT

  /* wait for our termination */
  KalExit(1, 0); // successful return

  /* Free all the memory allocated by the process */
  //for (ptr = areas_list; ptr; ptr = ptr->next)
    //KalFreeMem(ptr->addr);

  return NO_ERROR;
}
