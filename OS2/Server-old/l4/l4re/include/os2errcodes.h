#ifndef _OS2ERRCODES_H_
#define _OS2ERRCODES_H_

/* Includes OS/2 error codes for other build targets.
   The include might fail if it has already been included without
   INCL_ERRORS set.
*/

#if defined(__LINUX__)
#define INCL_ERRORS
#include <os2/bseerr.h>
#endif

#ifdef L4API_l4v2
#define INCL_ERRORS
//#include </mnt/c/program/WATCOM/h/os2/bseerr.h>
#endif

#endif
