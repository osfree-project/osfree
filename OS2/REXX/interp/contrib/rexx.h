/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992-1994  Anders Christensen <anders@pvv.unit.no>
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
 */
#ifndef __REXX_H_INCLUDED
#define __REXX_H_INCLUDED

#include "wrappers.h"

#if defined(__APPLE__) && defined(__MACH__)
# undef REGINA_BITS
# if defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__)
#  define REGINA_BITS 64
# else
#  define REGINA_BITS 32
# endif
#endif

/* Things you might want to change .... (at your own risk!!!) */

/*
 * define NOFLISTS if you DON'T want to use the internal memory
 * allocation functions.  By not using FLISTS, execution speed is reduced
 * by some 50%.  The use of NOFLISTS is useful for bounds checking
 * features of some compilers, but should be avoided for general use.
 *
 * It is available under Unix by using the --disable-flists switch
 * in autoconf script.
 * NOFLISTS can be be set on other ports by using FLISTS=N on the
 * 'make' command.
 */
#ifndef NOFLISTS
# define FLISTS
#endif

/*
 * define TRACEMEM to use tracing of dynamically allocated memory. This
 * is only useful when debugging memory management of the interpreter.
 * Some conditions must be fullfilled when using this (see memory.c).
 *
 * TRACEMEM is a subset of the debugging functionality. It is
 * available under Unix by using the --enable-tracemem switch
 * in autoconf script.
 * TRACEMEM can be be set on other ports by using TRACMEM=Y on the
 * 'make' command (you MUST also specify DEBUG=Y).
 */
#ifdef NDEBUG
# ifdef TRACEMEM
#  undef TRACEMEM
# endif
#endif


#ifdef CHECK_MEMORY                     /* Don't allow manual setting!       */
#  undef CHECK_MEMORY
#endif
#if defined(__GNUC__) && (defined(__BOUNDS_CHECKING_ON) || defined(__CHECKER__))
/*
 * Automatically define CHECK_MEMORY in case of GNU's bound checking
 * methods. Undef other memory checker settings since they are useless
 * in this case.
 */
#  define CHECK_MEMORY
#  ifdef FLISTS
#     undef FLISTS
#  endif
#  ifdef TRACEMEM
#     undef TRACEMEM
#  endif
#endif


/*
 * define PATTERN_MEMORY to initiate newly allocated dynamic memory to
 * a particular value, and freed memory to be set to another value
 * before it is freed. Useful for debugging only.
 */
#define PATTERN_MEMORY

#if defined(__SASC)
# define REG_FAR __far
#else
# define REG_FAR
#endif

/*
 * define MAX_ARGS_TO_REXXSTART to specify the maximum number of arguments that
 * can be passed to the RexxStart() and RexxCallBack() API functions.
 */
#define MAX_ARGS_TO_REXXSTART  32

/*
 * define MAX_CONCURRENT_REGINA_THREADS to change how many concurrent Regina
 * instances (threads) can be running.
 */
#define MAX_CONCURRENT_REGINA_THREADS 1000

#include "defs.h"
#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif
#include "configur.h"

#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif

#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

#define MAXNUMERIC 64           /* Max setting for NUMERIC DIGITS       */
/* #define REXXDEBUG */         /* You probably don't want this ... :-) */

/* #define HAVE_STRMATH */      /* Do we have *real* REXX string math ? */
/* #define HAVE_MPMATH */       /* Do we have C ``mp'' math             */
#define HAVE_CMATH              /* Do we have std. C math calls ?       */
#define MATH_TYPES 1            /* How many different types available   */

/*#define LINELENGTH 1024 not used */         /* max linelength of source code        */
/* #define STACKSIZE 400 not used ? */           /* Was 256, then 512 (too much)         */
/*
 * If you increase BUFFERSIZE you MUST ensure that YY_BUF_SIZE in lexsrc.c
 * is made larger than BUFFERSIZE.
 * Increasing this value will slow down parsing a bit.
 */
#define BUFFERSIZE 100000 /* Was 512 */ /* Size of input buffer, longest line   */
#define LOOKAHEAD 256           /* Size of input lookahead              */

/*
 * Buffer size for reading/writing with ADDRESS SYSTEM ... WITH
 * REGINA_MAX_BUFFER_SIZE must be larger than REGINA_BUFFER_SIZE.
 * Keep it on a page size for best performance in unix systems.
 */
#if defined(__WINS__) || defined(__EPOC32__)
# define REGINA_BUFFER_SIZE     128
#else
# define REGINA_BUFFER_SIZE  0x1000
#endif
#define REGINA_MAX_BUFFER_SIZE REGINA_BUFFER_SIZE * 2

#define SMALLSTR 5              /* For holding small integers           */
#define NULL_STR_LENGTH 1
#define BOOL_STR_LENGTH 2

#define DEFAULT_TRACING         'N'
#define DEFAULT_INT_TRACING     0
#define DEFAULT_NUMFORM         NUM_FORM_SCI
#define DEFAULT_ENVIRONMENT     ENV_SYSTEM
#define NESTEDCOMMENTS          /* The Standard wants them ....         */

#if defined(__WINS__) || defined(__EPOC32__)
# include "epoc32.h"
# define REXX_PATH_MAX MAXPATHLEN
#else
# ifndef REXX_PATH_MAX
#  ifndef PATH_MAX
#   ifndef _POSIX_PATH_MAX
#    ifndef _MAX_PATH
#     define REXX_PATH_MAX 1024
#    else
#     define REXX_PATH_MAX _MAX_PATH
#    endif
#   else
#    define REXX_PATH_MAX _POSIX_PATH_MAX
#   endif
#  else
#   define REXX_PATH_MAX PATH_MAX
#  endif
# endif
#endif

#include "regina_c.h"

#define HEXVAL( c ) ( rx_isdigit(c) ? ( ( c ) - '0' ) : ( rx_tolower(c) - 'a' + 10 ) )

/*
 * Which character is used to delimit lines in text files? Actually,
 * this should have been a bit more general, since the use of LF as
 * EOL-marker is a bit Unix-ish. We have to change this before porting
 * to other platforms.
 */
#define REGINA_CR (0x0d)
#if defined(MAC)
# define REGINA_EOL REGINA_CR
#else
# define REGINA_EOL (0x0a)
#endif

#define TRUE 1
#define FALSE 0

/* Things you probably don't want to change ....                        */

#include <stdlib.h>
#include <stdio.h>
#if defined(VMS) || defined(MAC)
# include <types.h>
#else
# include <sys/types.h>
#endif

#ifdef HAVE_SETJMP_H
# include <setjmp.h>
#endif

#if defined(TIME_WITH_SYS_TIME)
# include <sys/time.h>
# include <time.h>
#else
# if defined(HAVE_SYS_TIME_H)
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if defined(HAVE_FTIME)
# include <sys/timeb.h>
#endif

#if defined(HAVE_STRING_H)
# include <string.h>
#endif

#if defined(HAVE_SIGNAL_H)
# include <signal.h>
#endif

#if !defined(HAVE_RAISE)
# define raise(_s) kill(getpid(), (_s))
#endif

#if defined(WIN32) && defined(__BORLANDC__)
# include <mem.h>
#endif

#include "strengs.h"            /* definitions of REXX strings */

#if defined(_MSC_VER) && !defined(__WINS__)
# include "contrib/time64.h"
#endif

#include "regina_t.h"           /* various Regina types */

#include "mt.h"                 /* multi-threading support */
#include "extern.h"             /* function prototypes */

#define FREE_IF_DEFINED(a,b) { if (b) Free_TSD(a,b); b=NULL ; }

#ifdef VMS  /* F*ck DEC */
# ifdef EXIT_SUCCESS
#  undef EXIT_SUCCESS
# endif
# define EXIT_SUCCESS 1
#endif

#ifdef HAVE_GETOPT_LONG
# include <getopt.h>
# define my_getopt_long getopt_long
# define my_getopt_option option
#endif

/* For some mysterious reason, this macro is very difficult for some vendors */
#ifndef EXIT_SUCCESS
# define EXIT_SUCCESS 0
#endif

#define STRIP_TRAILING   1
#define STRIP_LEADING    2
#define STRIP_BOTH       (STRIP_TRAILING|STRIP_LEADING)

/* NOFUNC must be different from NULL and illegal, too */
#define NOFUNC ((streng *) (void *) -1l)

#if defined(MULTI_THREADED)
# ifdef __MINGW32__
#  define REGINA_VERSION_THREAD "(MT MINGW)"
# else
#  define REGINA_VERSION_THREAD "(MT)"
# endif
#else
# define REGINA_VERSION_THREAD ""
#endif

/*
#define REGINA_VERSION_MAJOR "3"
#define REGINA_VERSION_MINOR "4"
#define REGINA_VERSION_SUPP  "beta1"
*/

#define PARSE_VERSION_STRING    "REXX-Regina_" REGINA_VERSION_MAJOR "." \
                                REGINA_VERSION_MINOR "." REGINA_VERSION_RELEASE REGINA_VERSION_SUPP \
                                REGINA_VERSION_THREAD \
                                " 5.00 " REGINA_VERSION_DATE

#define INSTORE_VERSION 10 /* Must be incremented each time the parser/lexer
                           * or data structure changes something.
                           */

#endif
