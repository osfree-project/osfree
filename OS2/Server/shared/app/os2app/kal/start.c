/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal includes */
#include <os3/stacksw.h>
#include <os3/loader.h>
#include <os3/dataspace.h>
#include <os3/segment.h>
#include <os3/thread.h>
#include <os3/types.h>
#include <os3/cpi.h>
#include <os3/kal.h>
#include <os3/io.h>

/* l4env includes */
//#include <l4/names/libnames.h>
//#include <l4/sys/segment.h>

/* libc includes */
#include <stdio.h> // sprintf
#include <string.h>

extern l4_os3_thread_t me;

/* server loop thread of os2app   */
//extern l4_uint32_t   service_lthread;

extern unsigned long __stack;
/* Thread IDs array        */
l4_os3_thread_t ptid[MAX_TID];
/* application info blocks */
PTIB ptib[MAX_TID];
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

USHORT tramp(PCHAR argv, PCHAR envp, ULONG hmod, USHORT tib_sel, void *eip);

/* OS/2 app main thread */
int
trampoline(struct param *param)
{
  PCHAR argv = param->pib->pib_pchcmd;
  PCHAR envp = param->pib->pib_pchenv;
  ULONG hmod = param->pib->pib_hmte;

  unsigned long     base;
  struct desc       desc;

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
  //fiasco_gdt_set(&desc, sizeof(struct desc), 0, l4_myself());
  segment_gdt_set(&desc, sizeof(struct desc), 0, me);

  /* Get a selector */
  //tib_sel = (sizeof(struct desc)) * fiasco_gdt_get_entry_offset();
  tib_sel = (sizeof(struct desc)) * segment_gdt_get_entry_offset();
  tib_sel |= 3; // ring3 GDT descriptor
  io_log("sel=%x\n", tib_sel);

  /* save the previous stack to __stack
     and set current one to OS/2 app stack */
  STKINIT(s.sp - 0x10)

  /* We have changed the stack so it now points to our LX image. */
  //enter_kdebug("debug");
  old_sel = tramp(argv, envp, hmod, tib_sel, param->eip);

  STKOUT

  return 0;
}

char buf[1024];
l4_os3_thread_t thread;
struct param param;
APIRET rc;

APIRET CDECL KalStartApp(char *name, char *pszLoadError, ULONG cbLoadError)
{
  //CORBA_Environment env = dice_default_environment;
  //vmdata_t *ptr;
  /* Error info from LoadModule */
  //char uchLoadError[260];
  unsigned long hmod;
  ULONG curdisk, map;
  unsigned long ulActual;
  char *p = buf;
  int i;

  /* Load the LX executable */
  rc = KalPvtLoadModule(pszLoadError, &cbLoadError,
                        name, &s, &hmod);

  rcCode = rc;

  if (rc)
  {
    io_log("LX load error!\n");
    CPClientAppNotify2(&s, "os2app", &thread,
                       pszLoadError, cbLoadError, rcCode);
    KalExit(1, 1);
  }

  io_log("LX loaded successfully\n");

  param.eip = s.ip;
  param.esp = s.sp;

  strcpy(s.path, name);

  /* notify OS/2 server about parameters got from execsrv */
  //os2server_app_notify2_call (&os2srv, &s, &env);
  CPClientAppNotify2(&s, "os2app", &thread,
                     pszLoadError, cbLoadError, rcCode);

  STKINIT(__stack - 0x800)

  /* notify OS/2 server about parameters got from execsrv */
  //os2server_app_notify1_call (&os2srv, &env);
  CPClientAppNotify1();

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
  for (i = 1; i < MAX_TID; i++)
    ptib[i] = NULL;

  ptid[0] = KalNativeID();

  // initialize TIDs array
  for (i = 1; i < MAX_TID; i++)
    ptid[i] = INVALID_THREAD;

  param.pib = ppib;
  param.tib = ptib[0];

  io_log("ppib=%lx, ptib=%lx\n", ppib, ptib[0]);
  io_log("ppib->pib_pchcmd=%lx\n", ppib->pib_pchcmd);
  io_log("ppib->pib_pchenv=%lx\n", ppib->pib_pchenv);

#ifdef L4API_l4v2
  l4rm_show_region_list();
#endif

  // write PID to the screen
  sprintf(p, "The process id is %lx\n", ppib->pib_ulpid);
  KalWrite(1, p, strlen(p) + 1, &ulActual);

  io_log("Starting %s LX exe...\n", name);
  rc = trampoline (&param);
  io_log("... %s finished.\n", name);

  // unload exe module
  KalFreeModule(hmod);

  STKOUT

  /* wait for our termination */
  KalExit(1, 0); // successful return

  /* Free all the memory allocated by the process */
  //for (ptr = areas_list; ptr; ptr = ptr->next)
    //KalFreeMem(ptr->addr);

  return NO_ERROR;
}
