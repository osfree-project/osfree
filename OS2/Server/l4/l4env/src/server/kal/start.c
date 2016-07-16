/* L4 includes */
#include <l4/names/libnames.h>
/* OS/2 server internal includes */
#include <l4/os3/gcc_os2def.h>
#include <l4/os3/ixfmgr.h>
#include <l4/os3/processmgr.h>
#include <l4/os3/stacksw.h>
#include <l4/os3/loader.h>
#include <l4/os3/dataspace.h>
//#include <l4/os3/apistub.h>
#include <l4/os3/types.h>
#include <l4/os3/kal.h>
#include <l4/os3/dl.h>
#include <l4/os3/io.h>
/* OS/2 server RPC call includes */
#include <l4/os2srv/os2server-client.h>
/* exec server RPC call includes */
#include <l4/os2exec/os2exec-client.h>
/* DICE includes                 */
#include <dice/dice.h>
/* libc includes */
#include <stdio.h> // sprintf

/* fs server thread id   */
extern l4os3_cap_idx_t fs;
/* OS/2 server thread id */
extern l4os3_cap_idx_t os2srv;
/* exec server thread id */
extern l4os3_cap_idx_t execsrv;

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

  l4os3_cap_idx_t task;

  task = l4_myself();

  /* TIB base */
  base = (unsigned long)param->tib;

  /* Prepare TIB GDT descriptor */
  desc.limit_lo = 0x30; desc.limit_hi = 0;
  desc.acc_lo   = 0xF3; desc.acc_hi   = 0;
  desc.base_lo1 = base & 0xffff;
  desc.base_lo2 = (base >> 16) & 0xff;
  desc.base_hi  = base >> 24;

  /* Allocate a GDT descriptor */
  __fiasco_gdt_set(&desc, sizeof(struct desc), 0, task);

  /* Get a selector */
  tib_sel = (sizeof(struct desc)) * __fiasco_gdt_get_entry_offset();
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

APIRET CDECL kalStartApp(char *name)
{
  CORBA_Environment env = dice_default_environment;
  struct param param;
  APIRET rc;
  /* Error info from LoadModule */
  char uchLoadError[260];
  unsigned long hmod;
  ULONG curdisk, map;
  unsigned long ulActual;
  char buf[1024];
  char *p = buf;
  int i;

  if (!names_waitfor_name("os2exec", &execsrv, 30000))
    {
      io_log("Can't find os2exec on names, exiting...\n");
      kalExit(1, 1);
    }

  if (!names_waitfor_name("os2fs", &fs, 30000))
    {
      io_log("Can't find os2fs on names, exiting...\n");
      kalExit(1, 1);
    }

  if (!names_waitfor_name("os2srv", &os2srv, 30000))
    {
      io_log("Can't find os2srv on names, exiting...\n");
      kalExit(1, 1);
    }

  /* Load the LX executable */
  rc = kalPvtLoadModule(uchLoadError, sizeof(uchLoadError), 
                       name, &s, &hmod);

  if (rc)
  {
    io_log("LX load error!\n");
    kalExit(1, 1);
  }

  io_log("LX loaded successfully\n");

  param.eip = s.ip;
  param.esp = s.sp;

  strcpy(s.path, name);

  /* notify OS/2 server about parameters got from execsrv */
  os2server_app_notify_call (&os2srv, &s, &env);

  STKINIT(__stack - 0x800)

  rc = kalQueryCurrentDisk(&curdisk, &map);

  if (rc)
    io_log("Cannot get the current disk!\n");

  param.curdisk = curdisk;

  /* get the info blocks (needed by C startup code) */
  rc = kalMapInfoBlocks(&ptib[0], &ppib);

  // initialize TIB pointers array
  for (i = 1; i < 128; i++)
    ptib[i] = NULL;

  param.pib = ppib;
  param.tib = ptib[0];

  l4rm_show_region_list();

  // write PID to the screen
  sprintf(p, "The process id is %lx\n", ppib->pib_ulpid);
  kalWrite(1, p, strlen(p) + 1, &ulActual);

  io_log("Starting %s LX exe...\n", name);
  rc = trampoline (&param);
  io_log("... %s finished.\n", name);

  STKOUT

  /* wait for our termination */
  kalExit(1, 0); // successful return
  return NO_ERROR;
}
