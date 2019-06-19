/* $RCSfile: WIN95API.C $
   $Locker: ska $ $Name:  $   $State: Exp $

   int invokeWin95(int fct, struct REGPACK *rp)

   Invoke Win95/DOS function fct. It assumes that both APIs are the
   same except for the function number itself.

   *rp will hold the values of the successful call (either Win95
   or DOS).

   Return:
      0: success
      else: failure

   $Log: WIN95API.C $
   Revision 1.2  2000/01/11 09:34:42  ska
   add: support Turbo C v2.01

   Revision 1.1  2000/01/11 09:10:09  ska
   Auto Check-in

*/

#include <assert.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>

#include "io95.h"
#include "io95_loc.h"

#ifndef lint
static char const rcsid[] = 
   "$Id: WIN95API.C 1.2 2000/01/11 09:34:42 ska Exp ska $";
#endif

int callWin95(int fct, struct REGPACK * const rp)
{  struct REGPACK r;       /* temporary stack for the registers */

   assert(rp);
   rp->r_flags = 1;           /* asure that the carry is set */
   memcpy(&r, rp, sizeof(r));
   rp->r_ax = 0x7100 | fct;
   intr(0x21, rp);            /* call Win95 API */
   if((rp->r_flags & 1) && (rp->r_ax == 1 || rp->r_ax == 0x7100)) {
      /* try DOS API */
      r.r_ax = fct << 8;
      memcpy(rp, &r, sizeof(r));
      intr(0x21, rp);      /* call DOS API */
   }
   if(rp->r_flags & 1)
      return errno = rp->r_ax;      /* keep error code */
   return 0;      /* success */
}
