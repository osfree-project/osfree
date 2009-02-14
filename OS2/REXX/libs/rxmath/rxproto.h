/* definitions and prototypes for Rexx interface libraries
 *
 *  Copyright 1998 Patrick TJ McPhee
 *                 ptjm@interlog.com
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * $Header: C:/ptjm/rexx/math/tmp/RCS/rxproto.h 1.13 2001/01/30 20:20:48 pmcphee Rel $
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* this is to define alloca appropriately. I rather wish ansi had defined
 * this useful function */

#if defined(_WIN32)	/* microsoft C */
# include <malloc.h>
# define strcasecmp _stricmp
#elif defined (AIX)
# pragma alloca
#elif !defined(__GNUC__)
# include <alloca.h>
#endif

#define INCL_REXXSAA
#ifdef __EMX__  	/* emx under OS/2 */
# include <os2.h>
# define strcasecmp stricmp
#elif defined(__WATCOM__)
# include <os2.h>
# include <rexxsaa.h>
#else
# include "rexxsaa.h"
#endif

/* a rexx api function has type RexxFunctionHandler, which takes the name of
 * the function, the number of arguments, an array of arguments, a pointer
 * to something else, and a pointer to the result string.
 * It returns 0 for success, and some other value for failure */

#define rxfunc(x) APIRET APIENTRY x(PUCHAR fname, ULONG argc, PRXSTRING argv, PSZ pSomething, PRXSTRING result)

/* return codes. 22 is rc for invalid call */
#define NOMEMORY 5
#define BADARGS 22
#define BADGENERAL 40

/* ensure there are enough args, and not too many */
#define checkparam(f,t) if (argc < (f) || argc > (t)) return BADARGS

/* make a null-terminated string from a rexx string */
#define rxstrdup(y, x) do { y = (char *)alloca(RXSTRLEN(x)+1); memcpy(y,RXSTRPTR(x),RXSTRLEN(x)); y[RXSTRLEN(x)] = 0; } while (0)

/* number of elements in a fixed-size array */
#define DIM(x) (sizeof(x)/sizeof(*x))

/* allocate and free the way the source engine does (only do this if there's
 * not enough room in strptr). For Regina, use malloc to allocate, but don't
 * free, since result->strptr points to static data. */
#ifndef REXXALLOCATEMEMORY
# ifdef __EMX__
#  define REXXALLOCATEMEMORY DosAllocMem
#  define REXXFREEMEMORY DosFreeMem
# elif defined(OBJECTREXXW32)
#  define REXXALLOCATEMEMORY GlobalAlloc
#  define REXXFREEMEMORY GlobalFree
# else
#  define REXXALLOCATEMEMORY malloc
#  define REXXFREEMEMORY free
# endif
#endif

#ifdef RXAUTOBUFLEN
# define DEFAULTSTRINGSIZE RXAUTOBUFLEN
#else
# define DEFAULTSTRINGSIZE 255
#endif

/* resize the result variable. The Rexx engine is responsible for
 * freeing the memory that was passed in. */
#define rxresize(x, y) do { (x)->strlength = y;\
	if ((x)->strlength > DEFAULTSTRINGSIZE) {\
	   (x)->strptr = (char *)REXXALLOCATEMEMORY((x)->strlength);\
	}\
        if (!(x)->strptr) return NOMEMORY; } while (0)

/* sometimes we want to return just a 0 or 1 to indicate success or
 * failure */
#define result_zero() result->strlength = 1, *result->strptr = '0'
#define result_one() result->strlength = 1, *result->strptr = '1'

/* datatype: re-sizeable array of characters */
typedef struct {
    int count;
    int ptr_alloc, char_alloc, char_used;
    PRXSTRING array;
    char *chardata;
} chararray;


/* set the special variable rc. We usually set it to either 0 or 1 */
void set_rc(const char * const value, const int len);
void rc_one();
void rc_zero();

/* set an arbitrary variable */
void setavar(PRXSTRING varname, const char * const value, const int len);

/* map an array to a stem variable */
int setastem(PRXSTRING varname, const chararray * const values);
int getastem(PRXSTRING varname, chararray * const values);

/* retrieve portions of a numeric stem */
int getstemtail(PRXSTRING varname, const int ind, chararray * const values);
int setstemtail(PRXSTRING varname, const int ind, const chararray * const values);
int getstemsize(PRXSTRING varname, int * count);


/* add a string to a character array */
int addstr(chararray *ca, const char * str, int len);
/* add a string to a character array without copying the data */
int adddummy(chararray *ca, const char * str, int len);

chararray * new_chararray(void);
void delete_chararray(chararray * );

/* how many micro-seconds in a string */
int rxuint(PRXSTRING ptime);
int rxint(PRXSTRING ptime);
#ifdef _WIN32
int sleep(unsigned usecs);
int usleep(unsigned usecs);
#else
char * strupr(char * s);
char * strlwr(char * s);
#endif

#ifndef min
# define min(x,y) ((x) < (y) ? (x) : (y))
#endif
