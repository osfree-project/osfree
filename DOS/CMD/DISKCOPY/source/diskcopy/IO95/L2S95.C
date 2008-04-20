/* $RCSfile: L2S95.C $
   $Locker: ska $	$Name:  $	$State: Exp $

	int lfn2sfn95(const char * const fnam, char * const buf);

	fully-qualify fnam and convert all long filename portions into
	their short form.

	Return:
		0: success
		2: invalid component in path or drive letter
		3: malformed path or buffer too short


   $Log: L2S95.C $
   Revision 1.1  2000/01/11 09:10:09  ska
   Auto Check-in

*/

#include <assert.h>
#include <dos.h>
#include <string.h>
#include "io95.h"

#ifndef lint
static char const rcsid[] = 
	"$Id: L2S95.C 1.1 2000/01/11 09:10:09 ska Exp ska $";
#endif

static int conv95(int fct, const char * const fnam, char * const buf)
{	struct REGPACK r;

	assert(fnam);
	assert(buf);
	r.r_ax = 0x7160;
	r.r_cx = 0x8000 | fct;
	r.r_ds = FP_SEG(fnam);
	r.r_si = FP_OFF(fnam);
	r.r_es = FP_SEG(buf);
	r.r_di = FP_OFF(buf);
	r.r_flags = 0;
	*buf = '\0';
	intr(0x21, &r);
	if(r.r_flags & 1) {
		if(*buf && r.r_ax == 3 && strlen(buf) > 255)
			/* assume the bug */
			return 0;
		return r.r_ax;
	}
	return 0;
}

int truename95(const char * const fnam, char * const buf)
{	return conv95(2, fnam, buf);		}
int lfn2sfn95(const char * const fnam, char * const buf)
{	return conv95(1, fnam, buf);		}
