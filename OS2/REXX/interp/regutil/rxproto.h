/* definitions and prototypes for Rexx interface libraries
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is regutil.
 *
 * The Initial Developer of the Original Code is Patrick TJ McPhee.
 * Portions created by Patrick McPhee are Copyright © 1998, 2001
 * Patrick TJ McPhee. All Rights Reserved.
 *
 * Contributors:
 *
 * $Header: /opt/cvs/Regina/regutil/rxproto.h,v 1.7 2010/07/02 22:02:07 mark Exp $
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "configur.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* this is to define alloca appropriately. I rather wish ansi had defined
 * this useful function */

#if defined(_WIN32)     /* microsoft C */
# include <malloc.h>
# define strcasecmp _stricmp
#elif defined (AIX)
# pragma alloca
#elif !defined(__GNUC__) && !defined(QNX4)
# include <alloca.h>
#endif

#define INCL_REXXSAA
#ifdef __EMX__  /* emx under OS/2 */
# include <os2.h>
# define strcasecmp stricmp
#elif defined(REXXTRANS)
# include "rexxtrans.h"
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
#define checkparam(f,t) if (argc < (f) || (t != -1 && argc > (t))) return BADARGS

/* make a null-terminated string from a rexx string */
#define rxstrdup(y, x) do { y = (char *)alloca(RXSTRLEN(x)+1); memcpy(y,RXSTRPTR(x),RXSTRLEN(x)); y[RXSTRLEN(x)] = 0; } while (0)

/* number of elements in a fixed-size array */
#define DIM(x) (sizeof(x)/sizeof(*x))

/* allocate and free the way the source engine does (only do this if there's
 * not enough room in strptr). For Regina, use malloc to allocate, but don't
 * free, since result->strptr points to static data. */
#ifndef REXXALLOCATEMEMORY
# ifdef __EMX__
static void * ptjp;
#  define REXXALLOCATEMEMORY(b) DosAllocMem(&ptjp, b, PAG_READ|PAG_WRITE), ptjp
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
#define result_minus_one() result->strlength = 2, result->strptr[0] = '-', result->strptr[1] = '1'

/* datatype: re-sizeable array of characters */

struct caalloc {
   struct caalloc * N;
   int alloc, used;
   char data[sizeof(int)];
};

typedef struct {
    int count;
    int ptr_alloc;
    PRXSTRING array;
    struct caalloc * chars;
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
int setstemsize(PRXSTRING varname, const int count);

/* add a string to a character array */
int cha_addstr(chararray *ca, const char * str, int len);
/* add a string to a character array without copying the data */
int cha_adddummy(chararray *ca, const char * str, int len);

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

#ifdef HAVE_NCURSES_H
# define USE_TERMCAP_DB 1
#elif defined(HAVE_TERMCAP_H)
# define USE_TERMCAP_DB 1
#elif defined(HAVE_CURSES_H)
# define USE_TERMCAP_DB 1
#elif defined(HAVE_TERM_H)
# define USE_TERMCAP_DB 1
#endif
