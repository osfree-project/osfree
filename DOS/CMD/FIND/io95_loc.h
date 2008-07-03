/* $Id: io95_loc.h 1.1 2000/01/11 09:34:51 ska Exp ska $
   $Locker: ska $	$Name:  $	$State: Exp $

	Local functions of the IO95 library

   $Log: io95_loc.h $
   Revision 1.1  2000/01/11 09:34:51  ska
   Initial revision

*/

#ifndef __IO95_LOC_H
#define __IO95_LOC_H

int callWin95(int fct, struct REGPACK * const rp);
/*	Invoke Win95/DOS function fct. It assumes that both APIs are the
	same except for the function number itself.

	*rp will hold the values of the successful call (either Win95
	or DOS).

	Return:
		0: success
		else: failure
*/


#endif
