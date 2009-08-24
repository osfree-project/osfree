#ifndef _BUILTIN_H_
#define _BUILTIN_H_


/*******************************************/
#include <stdio.h>
/* #define INCL_DOSPROCESS 
#include <os2.h> */
#include <process.h>
/* #define F_INCL_DOSPROCESS  */
/* #include <F_base.hpp>  */


/*
#define VOID void 
#define APIENTRY    _System
typedef unsigned long  APIRET;
APIRET APIENTRY DosEnterCritSec(VOID);
APIRET APIENTRY DosExitCritSec(VOID);
*/
 
/*  june 6 2007

Does not exist in Open Watcom. What should it do? 
Maybe it's "lock exchange"? In that case I assume it checks if a is 
set with one or zero. One stands for LOCKED and zero for UNLOCKED.

If a is unlocked and b is locked the set a to locked and return zero.
Or if a is locked and b is unlocked, set a to unlocked and return zero.
If none of the above return 1.

The return value indicates if a has been changed or not.
*/

inline int __lxchg(int volatile * a, int  b) {
	/*printf("FIXME: builtin.h:%s (%d, %d)\n", __func__, *a, b); */
	int ret = 1;
	if(*a == 0 && b == 1) {
		*a = b;
		ret = 0;
	}
	if(*a == 1 && b == 0) {
		*a = b;
		ret = 0;
	}
	
	return ret;
}

/***********************************/

#endif