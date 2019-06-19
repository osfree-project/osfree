/* $RCSfile: REN95.C $
   $Locker: ska $	$Name:  $	$State: Exp $

	Rename file

   $Log: REN95.C $
   Revision 1.2  2000/01/11 09:34:39  ska
   add: support Turbo C v2.01

   Revision 1.1  2000/01/11 09:10:09  ska
   Auto Check-in

*/

#include <assert.h>
#include <dos.h>
#include "io95.h"
#include "io95_loc.h"

#ifndef lint
static char const rcsid[] = 
	"$Id: REN95.C 1.2 2000/01/11 09:34:39 ska Exp ska $";
#endif

int rename95(const char * const oldnam, const char * const newnam)
{	struct REGPACK r;

	assert(oldnam);
	assert(newnam);
	r.r_ds = FP_SEG(oldnam);
	r.r_dx = FP_OFF(oldnam);
	r.r_es = FP_SEG(newnam);
	r.r_di = FP_OFF(newnam);
	r.r_si = 0;
	r.r_cx = 0;
	return callWin95(0x56, &r);
}
