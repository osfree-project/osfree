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
  USHORT old_sel = 0;

  /* Add real implementation! */
  asm(
      "nop \n"
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
  /* Add real implementation! */
  asm (
      "nop \n"
       :
       :[old_sel]  "m"  (old_sel));
}

void KalQuit(void)
{
  /* Transition from L4 world to OS/2 world:
     save an old FS selector to old_sel and restore
     TIB selector in FS from tib_sel     */
  /* Add real implementation! */
  asm (
      "nop \n"
       :
       :[tib_sel]  "m"  (tib_sel));

  STKOUT
}
