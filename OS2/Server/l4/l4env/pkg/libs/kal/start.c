/* L4 includes */
//#include <l4/sys/types.h>
//#include <l4/util/stack.h>
//#include <l4/util/l4_macros.h>
//#include <l4/sys/kdebug.h>
#include <l4/dm_mem/dm_mem.h>
#include <l4/dm_generic/consts.h>
#include <l4/events/events.h>
#include <l4/generic_ts/generic_ts.h>
#include <l4/env/env.h>
#include <l4/env/errno.h>
#include <l4/log/l4log.h>
#include <l4/util/rdtsc.h>
#include <l4/l4rm/l4rm.h>
/* OS/2 server internal includes */
#include <l4/os3/gcc_os2def.h>
#include <l4/os3/ixfmgr.h>
#include <l4/os3/processmgr.h>
#include <l4/os3/stacksw.h>
#include <l4/os3/loader.h>
#include <l4/os3/apistub.h>
#include <l4/os3/dl.h>
/* OS/2 server RPC call includes */
#include <l4/os2srv/os2server-client.h>
/* exec server RPC call includes */
#include <l4/execsrv/os2exec-client.h>
/* DICE includes                 */
#include <dice/dice.h>

/* fs server thread id   */
extern l4_threadid_t fs;
/* OS/2 server thread id */
extern l4_threadid_t os2srv;
/* exec server thread id */
extern l4_threadid_t execsrv;

extern unsigned long __stack;
/* application info blocks */
PTIB ptib;
PPIB ppib;
/* entry point, stack and 
   other module parameters */
os2exec_module_t s;

unsigned long
kalPvtLoadModule(char *pszName,
              unsigned long cbName,
              char const *pszModname,
              os2exec_module_t *s,
              unsigned long *phmod);

void 
__fiasco_gdt_set(void *desc, unsigned int size,
                 unsigned int entry_number_start, l4_threadid_t tid);

unsigned 
__fiasco_gdt_get_entry_offset(void);

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

  //struct desc *dsc;

  //int  stop, start;
  //int  i, k;
  //char *p, *str;
  //char buf[0x100];

  l4_threadid_t task;

  task = l4_myself();

#if 0

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

#if 0
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
  __fiasco_gdt_set(&desc, sizeof(struct desc), 0, task);

  /* Get a selector */
  sel = (sizeof(struct desc)) * __fiasco_gdt_get_entry_offset();
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

  LOG("0");

  if (!names_waitfor_name("os2exec", &execsrv, 30000))
    {
      LOG("Can't find os2exec on names, exiting...");
      kalExit(1, 1);
    }

  if (!names_waitfor_name("os2fs", &fs, 30000))
    {
      LOG("Can't find os2fs on names, exiting...");
      kalExit(1, 1);
    }

  if (!names_waitfor_name("os2srv", &os2srv, 30000))
    {
      LOG("Can't find os2srv on names, exiting...");
      kalExit(1, 1);
    }

  LOG("1");

  /* Load the LX executable */
  rc = kalPvtLoadModule(uchLoadError, sizeof(uchLoadError), 
                       name, &s, &hmod);

  if (rc)
  {
    LOG("LX load error!");
    kalExit(1, 1);
  }

  LOG("LX loaded successfully");

  param.eip = s.ip;
  param.esp = s.sp;

  strcpy(s.path, name);

  LOG("00");
  /* notify OS/2 server about parameters got from execsrv */
  os2server_app_notify_call (&os2srv, &s, &env);
  LOG("11");
  STKINIT(__stack - 0x800)
  LOG("22");
  rc = kalQueryCurrentDisk(&curdisk, &map);
  LOG("33");
  if (rc)
    LOG("Cannot get the current disk!");
  LOG("44");
  param.curdisk = curdisk;

  /* get the info blocks (needed by C startup code) */
  rc = kalMapInfoBlocks(&ptib, &ppib);

  param.pib = ppib;
  param.tib = ptib;

  l4rm_show_region_list();

  // write PID to the screen
  sprintf(p, "The process id is %x\n", ppib->pib_ulpid);
  kalWrite(1, p, strlen(p) + 1, &ulActual);

  LOG("Starting %s LX exe...", name);
  rc = trampoline (&param);
  LOG("... %s finished.", name);

  STKOUT

  /* wait for our termination */
  kalExit(1, 0); // successful return
  return NO_ERROR;
}
