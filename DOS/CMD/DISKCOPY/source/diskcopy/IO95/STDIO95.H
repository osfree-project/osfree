/* $Id: STDIO95.H 1.1 2000/01/11 09:39:37 ska Exp ska $
   $Locker: ska $	$Name:  $	$State: Exp $

	stdio functions supporting Win95's LFN as well as DOS 4+.

   $Log: STDIO95.H $
   Revision 1.1  2000/01/11 09:39:37  ska
   Initial revision

*/

#ifndef __STDIO95_H
#define __STDIO95_H

/******
	compiler - specific settings
		*******/

			/* Name of fdopen() if available */
#define HAVE_FDOPEN	fdopen



#include <stdio.h>
#include "io95.h"


FILE *fopen95(const char fnam[], const char mode[]);


/**********		Define USE_IO95 to always use io95-functions	********/
#ifdef USE_IO95
#undef fopen
#define fopen fopen95
#endif

#endif
