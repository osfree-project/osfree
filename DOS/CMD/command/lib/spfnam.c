/* $Id: spfnam.c 984 2004-06-29 14:14:57Z skaus $

	skip filename token

	return the pointer of the next character that is no is_fnchar()

*/

#include "../config.h"

#include <assert.h>

#include "../include/misc.h"

char *skipfnam(const char * const fnam)
{	const char *p;

	assert(fnam);
	p = fnam - 1;
	while(is_fnchar(*++p));
	return (char*)p;
}
