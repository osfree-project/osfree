#define  INCL_BASE
#include <os2.h>

#include <os3/stacksw.h>

/* old FS selector value          */
extern unsigned short old_sel;
/* TIB new FS selector value      */
extern unsigned short tib_sel;

USHORT tramp(PCHAR argv, PCHAR envp, ULONG hmod,
             USHORT tib_sel, void *eip)
{
  USHORT old_sel;

  asm(
      "movw  %%fs, %%dx \n"
      "movw  %%dx, %[old_sel] \n"
      "movw  %[tib_sel], %%dx \n"
      "movw  %%dx, %%fs \n"              /* TIB selector */
      "pushq %%rbp \n"                   /* save ebp on the old stack      */
      "movq  %[argv], %%rdx \n"
      "pushq %%rdx \n"                   /* argv  */
      "movq  %[envp], %%rdx \n"
      "pushq %%rdx \n"                   /* envp  */
      "movq  $0, %%rdx \n"
      "pushq %%rdx \n"                   /* sizec */
      "movq  %[hmod], %%rdx \n"
      "pushq %%rdx \n"                   /* hmod  */
      "movq  %[eip_data], %%rcx \n"
      "call  *%%rcx \n"                  /* Call the startup code of an OS/2 executable */
      "addq  $0x10, %%rsp \n"            /* clear stack            */
      "popq  %%rbp \n"                   /* restored the old ebp   */
      :[old_sel]  "=r" (old_sel)
      :[argv]     "m"  (argv),
       [envp]     "m"  (envp),
       [hmod]     "m"  (hmod),
       [tib_sel]  "m"  (tib_sel),
       [eip_data] "m"  (eip));

  return old_sel;
}

/* The following two routines are needed because of
   collision between Fiasco.OC and OS/2: Fiasco.OC
   stores the UTCB selector in fs, and OS/2 stores
   TIB selector there. So, a workaround: save/restore
   these selectors when entering/exiting to/from
   L4 world / OS/2 world. */

void KalEnter(void)
{
  STKIN

  /* Transition from OS/2 world to L4 world:
     save TIB selector to tib_sel and restore
     host kernel FS selector from old_sel */
  asm ("pushq %%rax\n"
       "movw  %[old_sel], %%ax \n"
       "movw  %%ax, %%fs \n"
       "popq  %%rax \n"
       :
       :[old_sel]  "m"  (old_sel));
}

void KalQuit(void)
{
  /* Transition from L4 world to OS/2 world:
     save an old FS selector to old_sel and restore
     TIB selector in FS from tib_sel     */
  asm ("pushq %%rax \n"
       "movw  %[tib_sel], %%ax \n"
       "movw  %%ax, %%fs \n"
       "popq  %%rax \n"
       :
       :[tib_sel]  "m"  (tib_sel));

  STKOUT
}
