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

/*
 * $Id: configur.h,v 1.2 2003/12/11 04:43:03 prokushev Exp $
 */

/*
 * For platforms that don't use autoconf, put these here...
 */
#define HAVE_ASSERT_H        1
#define HAVE_ERRNO_H         1
#define HAVE_CTYPE_H         1
#define HAVE_STDARG_H        1
#define HAVE_SYS_STAT_H      1

#if defined(DOS) && defined(EMX)
# define HAVE_ALLOCA_H        1
# define HAVE_FTIME           1
# define HAVE_MATH_H          1
# define HAVE_RANDOM          1
#undef HAVE_RANDOM /* needs bsd support and linking with -lbsd */
# define HAVE_SIGACTION       1
# define HAVE_STDIO_H         1
# define HAVE_SYS_TIME_H      1
# define HAVE_SYSCONF         1
# define HAVE_TERMIOS_H       1
# define HAVE_TIME_H          1
# define HAVE_FCNTL_H         1
# define HAVE_GRP_H           1
# define HAVE_PWD_H           1
# define HAVE_LIMITS_H        1
# define HAVE_MALLOC_H        1
# define HAVE_SETJMP_H        1
# define HAVE_SIGNAL_H        1
# define HAVE_STDLIB_H        1
# define HAVE_STRING_H        1
# define HAVE_UNISTD_H        1
# define HAVE_SYS_SOCKET_H    1
# define HAVE_NETINET_IN_H    1
# define HAVE_SYS_SELECT_H    1
# define HAVE_NETDB_H         1
# define HAVE_ARPA_INET_H     1
# define HAVE__FULLPATH       1
# define HAVE_PUTENV          1
# define HAVE_GETTIMEOFDAY    1
# define HAVE_FTRUNCATE       1
# define HAVE_MEMCPY          1
# define HAVE_MEMMOVE         1
# define HAVE_STRERROR        1
# define HAVE_VSPRINTF        1
# define HAVE_GETPID          1
# define TIME_WITH_SYS_TIME   1
#endif

#if defined(OS2) && defined(EMX)
# define HAVE_ALLOCA_H        1
# define HAVE_FTIME           1
# define HAVE_MATH_H          1
# define HAVE_RANDOM          1
#undef HAVE_RANDOM /* needs bsd support and linking with -lbsd */
# define HAVE_SIGACTION       1
# define HAVE_STDIO_H         1
# define HAVE_SYS_TIME_H      1
# define HAVE_SYSCONF         1
# define HAVE_TERMIOS_H       1
# define HAVE_TIME_H          1
# define HAVE_FCNTL_H         1
# define HAVE_GRP_H           1
# define HAVE_PWD_H           1
# define HAVE_LIMITS_H        1
# define HAVE_MALLOC_H        1
# define HAVE_SETJMP_H        1
# define HAVE_SIGNAL_H        1
# define HAVE_STDLIB_H        1
# define HAVE_STRING_H        1
# define HAVE_UNISTD_H        1
# define HAVE_SYS_SOCKET_H    1
# define HAVE_NETINET_IN_H    1
# define HAVE_SYS_SELECT_H    1
# define HAVE_NETDB_H         1
# define HAVE_ARPA_INET_H     1
# define HAVE__FULLPATH       1
# define HAVE_PUTENV          1
# define HAVE_GETTIMEOFDAY    1
# define HAVE_FTRUNCATE       1
# define HAVE_MEMCPY          1
# define HAVE_MEMMOVE         1
# define HAVE_STRERROR        1
# define HAVE_VSPRINTF        1
# define HAVE_GETPID          1
# define TIME_WITH_SYS_TIME   1
# if defined(DYNAMIC)
#  define DYNAMIC_OS2
# endif
#endif

#if defined(OS2) && defined(__IBMC__)
# define HAVE_SETJMP_H
# define HAVE__FULLPATH
# define HAVE_PUTENV
# define HAVE_GETTIMEOFDAY
# define HAVE_FTRUNCATE
# define HAVE_MEMCPY
# define HAVE_MEMMOVE
# define HAVE_STRERROR
# define HAVE_VSPRINTF
# define HAVE_GETPID
# define TIME_WITH_SYS_TIME
#endif

#if defined(OS2) && defined(__WATCOMC__)
# define HAVE_MALLOC_H
# define HAVE_PROCESS_H
# define HAVE_SETJMP_H
# define HAVE_STRING_H
# define HAVE_UNISTD_H
# define HAVE_STRING_H
# define HAVE_DIRECT_H
# define HAVE_PROCESS_H    /* at least OpenWatcom */
# define HAVE_SYS_SOCKET_H /* at least OpenWatcom */
# define HAVE_NETINET_IN_H /* at least OpenWatcom */
# define HAVE_SYS_SELECT_H /* at least OpenWatcom */
# define HAVE_NETDB_H      /* at least OpenWatcom */
# define HAVE_ARPA_INET_H  /* at least OpenWatcom */
# define HAVE__FULLPATH
# define HAVE_PUTENV
# define HAVE_DIRECT_H
# define HAVE_TYPES_H
# define HAVE_SYS_SOCKET_H
# define HAVE_NETINET_IN_H
# define HAVE_SYS_SELECT_H
# define HAVE_NETDB_H
# define HAVE_ARPA_INET_H
# define HAVE_MEMCPY
# define HAVE_MEMMOVE
# define HAVE_STRERROR
# define HAVE_VSPRINTF
# define HAVE_GETPID
# if defined(DYNAMIC)
#  define DYNAMIC_OS2
# endif
#endif

#if defined(WIN32) && defined(__WATCOMC__)
# define HAVE_MALLOC_H
# define HAVE_SETJMP_H
# define HAVE_SIGNAL_H
# define HAVE_UNISTD_H
# define HAVE_FTIME
# define HAVE_FCNTL_H
# define HAVE__FULLPATH
# define HAVE__SPLITPATH2
/* # define HAVE_FTRUNCATE */
# define HAVE_MEMCPY
# define HAVE_MEMMOVE
# define HAVE_SETENV
# define HAVE_STRERROR
# define HAVE_VSPRINTF
# define HAVE_GETPID
# if defined(DYNAMIC)
#  define DYNAMIC_WIN32
# endif
#endif

#if defined(WIN32) && defined(_MSC_VER)
# define HAVE_FCNTL_H
# define HAVE_LIMITS_H
# define HAVE_MALLOC_H
# define HAVE_PROCESS_H
# define HAVE_SETJMP_H
# define HAVE_SIGNAL_H
# define HAVE_STDLIB_H
# define HAVE_STRING_H
# define HAVE__FULLPATH
# define HAVE__SPLITPATH
# define HAVE_FTRUNCATE
# define HAVE_MY_WIN32_SETENV
# define HAVE_FTIME
# define HAVE_MEMCPY
# define HAVE_MEMMOVE
# define HAVE_STRERROR
# define HAVE_VSPRINTF
# define HAVE_GETPID
# if defined(DYNAMIC)
#  define DYNAMIC_WIN32
# endif
#endif

#if defined(WIN32) && defined(__BORLANDC__)
# define HAVE_LIMITS_H
# define HAVE_MALLOC_H
# define HAVE_PROCESS_H
# define HAVE_SETJMP_H
# define HAVE_SIGNAL_H
# define HAVE_STDLIB_H
# define HAVE_STRING_H
# define HAVE__FULLPATH
# define HAVE__SPLITPATH
# define HAVE_MY_WIN32_SETENV
# define HAVE_FTIME
# define HAVE_MEMCPY
# define HAVE_MEMMOVE
# define HAVE_STRERROR
# define HAVE_VSPRINTF
# if defined(DYNAMIC)
#  define DYNAMIC_WIN32
# endif
#endif

#if defined(WIN32) && defined(__CYGWIN32__)
# define HAVE_GRP_H           1
# define HAVE_PWD_H           1
# define HAVE_LIMITS_H        1
# define HAVE_MALLOC_H        1
# define HAVE_PROCESS_H       1
# define HAVE_SETJMP_H        1
# define HAVE_SIGNAL_H        1
# define HAVE_STDLIB_H        1
# define HAVE_STRING_H        1
# define HAVE_UNISTD_H        1
/* # define HAVE__FULLPATH        1 */
# define HAVE_PUTENV          1
# define HAVE_GETTIMEOFDAY    1
# define HAVE_FTRUNCATE       1
# define HAVE_MEMCPY          1
# define HAVE_MEMMOVE         1
# define HAVE_STRERROR        1
# define HAVE_VSPRINTF        1
# define HAVE_GETPID
# define TIME_WITH_SYS_TIME   1
# if defined(DYNAMIC)
#  define DYNAMIC_WIN32
# endif
#endif

#if defined(WIN32) && defined(__MINGW32__)
# define HAVE_LIMITS_H        1
# define HAVE_MALLOC_H        1
# define HAVE_PROCESS_H       1
# define HAVE_SETJMP_H        1
# define HAVE_SIGNAL_H        1
# define HAVE_STDLIB_H        1
# define HAVE_STRING_H        1
# define HAVE_UNISTD_H        1
/* # define HAVE__FULLPATH        1 */
# define HAVE_PUTENV          1
/* # define HAVE_GETTIMEOFDAY    1 */
/* # define HAVE_FTRUNCATE       1 */
# define HAVE_MEMCPY          1
# define HAVE_MEMMOVE         1
# define HAVE_STRERROR        1
# define HAVE_VSPRINTF        1
# define HAVE_GETPID
# define TIME_WITH_SYS_TIME   1
# if defined(DYNAMIC)
#  define DYNAMIC_WIN32
# endif
#endif

#if defined(WIN32) && defined(__LCC__)
# define HAVE_FCNTL_H         1
# define HAVE_DIRECT_H        1
# define HAVE_LIMITS_H        1
# define HAVE_MALLOC_H        1
# define HAVE_PROCESS_H       1
# define HAVE_SETJMP_H        1
# define HAVE_SIGNAL_H        1
# define HAVE_STDLIB_H        1
# define HAVE_STRING_H        1
# define HAVE_UNISTD_H        1
/* # define HAVE__FULLPATH        1 */
# define HAVE_PUTENV          1
/* # define HAVE_GETTIMEOFDAY    1 */
/* # define HAVE_FTRUNCATE       1 */
# define HAVE_MEMCPY          1
# define HAVE_MEMMOVE         1
# define HAVE_STRERROR        1
# define HAVE_VSPRINTF        1
# define HAVE_GETPID
/* # define TIME_WITH_SYS_TIME   1 */
# if defined(DYNAMIC)
#  define DYNAMIC_WIN32
# endif
#endif

#if defined(DOS) && defined(DJGPP)
# define HAVE_FCNTL_H
# define HAVE_GRP_H
# define HAVE_PWD_H
# define HAVE_LIMITS_H
# define TIME_WITH_SYS_TIME
# define HAVE_SETJMP_H
# define HAVE_UNISTD_H
# define HAVE_SYS_SOCKET_H
# define HAVE_NETINET_IN_H
/* # define HAVE_SYS_SELECT_H */
# define HAVE_NETDB_H
# define HAVE_ARPA_INET_H
# define HAVE_PUTENV
# define HAVE_DRAND48
# define HAVE_GETTIMEOFDAY
# define HAVE_FTRUNCATE
# define HAVE_RANDOM
# define HAVE_DRAND48
# define HAVE_USLEEP
# define HAVE_MEMCPY
# define HAVE_MEMMOVE
# define HAVE_STRERROR
# define HAVE__TRUENAME
# define HAVE_VSPRINTF
# define HAVE_GETPID
# ifndef _MAX_PATH
#  define _MAX_PATH  PATH_MAX
# endif
#endif

#if defined(_AMIGA) && defined(__SASC)
# define HAVE_SETJMP_H
/* # define HAVE_UNISTD_H */
# define HAVE_PUTENV
# define HAVE_MEMCPY
# define HAVE_MEMMOVE
# define HAVE_STRERROR
# define HAVE_VSPRINTF
# define UNIX
#endif

#if defined(AMIGA) && defined(GCC)
# define HAVE_GRP_H           1
# define HAVE_PWD_H           1
# define HAVE_LIMITS_H        1
# define HAVE_MALLOC_H        1
# define HAVE_SETJMP_H        1
# define HAVE_SIGNAL_H        1
# define HAVE_STDLIB_H        1
# define HAVE_STRING_H        1
# define HAVE_UNISTD_H        1
# define HAVE_SYS_SOCKET_H    1
# define HAVE_NETINET_IN_H    1
# define HAVE_NETDB_H         1
# define HAVE_ARPA_INET_H     1
/* # define HAVE__FULLPATH        1 */
# define HAVE_PUTENV          1
# define HAVE_FTIME           1
# define HAVE_FTRUNCATE       1
# define HAVE_MEMCPY          1
# define HAVE_MEMMOVE         1
# define HAVE_STRERROR        1
# define HAVE_VSPRINTF        1
# define HAVE_GETPID
# define TIME_WITH_SYS_TIME   1
#endif

#if defined(MAC)
/* # define HAVE_GRP_H           1 */
/* # define HAVE_PWD_H           1 */
# define HAVE_LIMITS_H        1
/* # define HAVE_MALLOC_H        1 */
# define HAVE_SETJMP_H        1
# define HAVE_SIGNAL_H        1
# define HAVE_STDLIB_H        1
# define HAVE_STRING_H        1
/* # define HAVE_UNISTD_H        1 */
/*
# define HAVE_SYS_SOCKET_H    1
# define HAVE_NETINET_IN_H    1
# define HAVE_NETDB_H         1
# define HAVE_ARPA_INET_H     1
*/
/* # define HAVE__FULLPATH        1 */
/*# define HAVE_SETENV          1 */
/* # define HAVE_FTIME           1 */
/* # define HAVE_FTRUNCATE       1 */
# define HAVE_MEMCPY          1
# define HAVE_MEMMOVE         1
# define HAVE_STRERROR        1
# define HAVE_VSPRINTF        1
# define HAVE_GETPID
/* # define TIME_WITH_SYS_TIME   1 */
#endif

/*
 * The following are for Epoc32 platform
 */
#if defined(__VC32__) && defined(__WINS__)
# define HAVE_SYS_FCNTL_H
# define HAVE_LIMITS_H
# define HAVE_PROCESS_H
# define HAVE_SETJMP_H
# define HAVE_SIGNAL_H
# define HAVE_STDLIB_H
# define HAVE_STRING_H
/* # define HAVE__FULLPATH */
/* # define HAVE__SPLITPATH */
/* # define HAVE_FTRUNCATE */
/* # define HAVE_MY_WIN32_SETENV */
# define HAVE_MEMCPY
# define HAVE_MEMMOVE
# define HAVE_STRERROR
# define HAVE_VSPRINTF
# define HAVE_GETPID
# define HAVE_SYS_SOCKET_H
# define HAVE_NETINET_IN_H
# define HAVE_NETDB_H
# define HAVE_ARPA_INET_H
# define HAVE_UNISTD_H
# if defined(DYNAMIC)
#  define DYNAMIC_WIN32
# endif
#endif

#if defined(__GCC32__) && defined(__EPOC32__)
# define HAVE_SYS_FCNTL_H     1
/* # define HAVE_GRP_H           1 */
/* # define HAVE_PWD_H           1 */
# define HAVE_LIMITS_H        1
/*# define HAVE_MALLOC_H        1 */
# define HAVE_PROCESS_H       1
# define HAVE_SETJMP_H        1
# define HAVE_SIGNAL_H        1
# define HAVE_STDLIB_H        1
# define HAVE_STRING_H        1
# define HAVE_UNISTD_H        1
/* # define HAVE__FULLPATH        1 */
/* # define HAVE_PUTENV          1 */
# define HAVE_GETTIMEOFDAY    1
/*# define HAVE_FTRUNCATE       1 */
# define HAVE_MEMCPY          1
# define HAVE_MEMMOVE         1
# define HAVE_STRERROR        1
# define HAVE_VSPRINTF        1
# define HAVE_GETPID          1
# define TIME_WITH_SYS_TIME   1
# define HAVE_SYS_SOCKET_H    1
# define HAVE_NETINET_IN_H    1
# define HAVE_NETDB_H         1
# define HAVE_ARPA_INET_H     1
# if defined(DYNAMIC)
#  define DYNAMIC_WIN32
# endif
#endif

#if defined(VMS)
# include "vms.h"
#endif

/* Configurable values */


/* The macro MAXLEVELS is the maximum number of levels that the
 *   interpreter shall handle. One level is used for functioncalls and
 *   the commands CALL, IF, DO and SELECT
 */
#define MAXLEVELS        1024

/* The macro HASHTABLENGTH is the number of sloths in the hash table
 *   containing the values of the rexx variables, must be a power of 2.
 */
#define HASHTABLENGTH     256

/* The macro MAX_INDEX_LENGTH defines the maximum length that the name
 *   of an compund variable might expand to.
 */
#define MAX_INDEX_LENGTH  256

/*
 * The macro DEFAULT_FLUSH_STACK can be defined to either 1 or 0,
 * depending on whether you want the stack to be flushed during a
 * command reading input from or writing output to the stack. If
 * defined, a line written will be physically in the stack at once,
 * while if not defined, all output lines will be temporarily stacked
 * until the command has terminated, then they are all simultaneously
 * flushed to the stack. The default default value is 0, it can be
 * changed through OPTIONS.
 */
#ifndef DEFAULT_FLUSHSTACK
# define DEFAULT_FLUSHSTACK 0
#endif

/*
 * The macro DEFAULT_LINEOUTTRUNC can be set to 1 or 0, whether you want
 * the default behavior of lineout() to truncate the file immediately
 * after the end of each new file written out. When set to 1, the file
 * will be truncated. Also note, that if HAVE_FTRUNCATE isn't set later
 * in this script, the DEFAULT_LINEOUTTRUNC will be forced to 0, just
 * for consistency (then, setting it to 1 will have no practical effect.
 */
#ifndef DEFAULT_LINEOUTTRUNC
# define DEFAULT_LINEOUTTRUNC 1
#endif

#define DEFAULT_MAKEBUF_BIF 1
#define DEFAULT_DROPBUF_BIF 1
#define DEFAULT_BUFTYPE_BIF 1
#define DEFAULT_DESBUF_BIF 1

#define DEFAULT_CACHEEXT 0
#define DEFAULT_PRUNE_TRACE 1

/*
 * Indicates that Regina treats external commands as functions by default
 */
#define DEFAULT_EXT_COMMANDS_AS_FUNCS  1
/*
 * Indicates that Regina does not send output that normally goes to stderr, to stdout
 */
#define DEFAULT_STDOUT_FOR_STDERR      0
/*
 * Indicates that Regina allows external queues (and internal queues) by default
 */
#define DEFAULT_INTERNAL_QUEUES        0
/*
 * Indicates that Regina does not wrap trace output in HTML code by default
 */
#define DEFAULT_TRACE_HTML             0
/*
 * Indicates that Regina returns 1 or 0 for when calling LINES BIF without optional 2nd argument by default
 */
#define DEFAULT_FAST_LINES_BIF_DEFAULT 1
/*
 * Indicates that Regina does not observe strict ANSI functionality by default
 */
#define DEFAULT_STRICT_ANSI            0
/*
 * Indicates that Regina-specific BUFs are available by default
 */
#define DEFAULT_REGINA_BIFS            1
/*
 * Indicates that Regina does not do strict white space comparison by default
 */
#define DEFAULT_STRICT_WHITE_SPACE_COMPARISONS 0
/*
 * Indicates if the ARexx BIFs (OPEN, CLOSE, EOF) use AREXX semantics or not
 * On Amiga and AROS, they are on by default.
 */
#if defined(_AMIGA) || defined(__AROS__)
# define DEFAULT_AREXX_SEMANTICS        1
#else
# define DEFAULT_AREXX_SEMANTICS        0
#endif
/*
 * Indicates that AREXX BIFs are available by default on Amiga and AROS, but
 * OFF on other platforms
 */
#if defined(_AMIGA) || defined(__AROS__)
# define DEFAULT_AREXX_BIFS             1
#else
# define DEFAULT_AREXX_BIFS             0
#endif
/*
 * Indicates if the user want the "broken" semantics of ADDRESS COMMAND
 */
#define DEFAULT_BROKEN_ADDRESS_COMMAND 0

/*
 * The FILE_SEPARATOR is one character, and defines the char that separates
 * directories, and filename from directories in a file's path.
 * The PATH_SEPARATOR is one character, and defines the char that separates
 * directories, from one another in a PATH environment variable.
 */

#if defined(MSDOS) || ( defined(__WATCOMC__) && !defined(__QNX__) ) || defined(_MSC_VER) || defined(DOS) || defined(OS2) || defined(__WINS__) || defined(__EPOC32__)
# define FILE_SEPARATOR      '\\'
# define FILE_SEPARATOR_STR  "\\"
# define PATH_SEPARATOR      ';'
# define PATH_SEPARATOR_STR  ";"
#elif defined(VMS)
# define FILE_SEPARATOR     ']'
# define FILE_SEPARATOR_STR "]"
# define PATH_SEPARATOR     '?'
# define PATH_SEPARATOR_STR "?"
#elif defined(MAC)
# define FILE_SEPARATOR     ']'
# define FILE_SEPARATOR_STR "]"
# define PATH_SEPARATOR     '?'
# define PATH_SEPARATOR_STR "?"
#else
# define FILE_SEPARATOR     '/'
# define FILE_SEPARATOR_STR "/"
# define PATH_SEPARATOR     ':'
# define PATH_SEPARATOR_STR ":"
#endif


#define FIX_PROTOS
#define FIX_ALL_PROTOS


#if defined(HPUX) || defined(__hpux)
# define YY_CHAR_TYPE unsigned char
#endif

#if !defined(HAVE_MEMCPY)
# define memcpy(a,b,c)  bcopy((b),(a),(c))
#endif

#if !defined(HAVE_MEMMOVE)
# define memmove(a,b,c) bcopy((b),(a),(c))
#endif

#if !defined(HAVE_STRERROR)
  const char *get_sys_errlist( int num );
# define strerror(a)    get_sys_errlist(a)
#endif


#if defined(sgi) || defined(__sgi)
# define SGI 1
#endif

#if defined(__osf__)
# define OSF 1
# define YY_CHAR_TYPE unsigned char
#endif

/*
 * Define HAVE_USLEEP if you have the usleep() function available. It will
 *    take the number of micro-seconds to sleep. If this is not defined,
 *    the 'normal' sleep() will be used.
 * Handled by autoconf
 */

/*
 * Define HAVE_RANDOM if your machine has the routines random() and
 *    srandom(). If it is not defined, rand() and srand() are used
 *    instead. There are several reason why you might want to use
 *    random() instead of rand()
 * Handled by autoconf
 */


#define DEFAULT_NUMERIC_FUZZ 0
#define DEFAULT_NUMERIC_SIZE 9
#define DEFAULT_MATH_TYPE 0


/*
 * If you use gcc on a machine that does not have proper ANSI C
 * libraries and the function div() is not defined, then you can
 * define the symbol HAVE_NO_DIV, which will try to kludge it.
 *
 * This is necessary on Suns, and some other machines.
 *
 * Not used anymore MH 10-06-97
 */


#ifndef YY_CHAR_TYPE
# ifdef _AIX
#  define YY_CHAR_TYPE unsigned char
# else
#  define YY_CHAR_TYPE char
# endif
#endif


/* Unfortunately, ftruncate() is not POSIX, so if you don't have it,
 * just ignore it, the effect is that lines written to the middle of a
 * file will not cause the rest of the file (after the linefeed
 * terminating the line written), also to truncate the file at that
 * point. If you wonder whether you have ftruncate(), the bsd portions
 * of you system (if it exist) is a good place to start looking. You
 * may have to link in a specific library too, maybe libbsd.a.
 */
/*#define HAVE_FTRUNCATE*/    /* handled in makefile - TBD in autoconf */



#ifdef VMS
# ifdef HAVE_GETTIMEOFDAY
#  undef HAVE_GETTIMEOFDAY
# endif
# ifdef HAVE_PUTENV
#  undef HAVE_PUTENV
# endif
# ifdef HAVE_FTRUNCATE
#  undef HAVE_FTRUNCATE
# endif
#endif


#if defined(__GNUC__) && defined(VMS)
# ifdef HAVE_GETTIMEOFODAY
#  undef HAVE_GETTIMEOFDAY
# endif
#endif

/*
 * DLOPEN   - dlopen()/dlsym() under SunOS and SysV4
 * HPSHLOAD - shl_load()/shl_findsym() under HP-UX
 * AIXLOAD  - load()/loadbind() under AIX - not supported
 * OS2      - DosLoadModule()/DosQueryProcAddr() under OS/2
 * WIN32    - LoadLibrary()/GetProcAddress() under Win32
 */

/*
 * QNX 4.2x does have a ftrunc() equivalent: ltrunc()
 */
#if defined(__WATCOMC__) && defined(__QNX__)
# define HAVE_FTRUNCATE
#endif
/*
 * If we don't have ftruncate() then we don't want the default setting
 * of LINEOUT() to be to truncate after each new line written out.
 */
#ifndef HAVE_FTRUNCATE
# undef DEFAULT_LINEOUTTRUNC                            /* MH 10-06-96 */
# define DEFAULT_LINEOUTTRUNC 0
#endif

#ifdef _MSC_VER
/* This picky compiler claims about unused formal parameters.
 * This is correct but hides (for human eyes) other errors since they
 * are many and we can't reduce them all.
 * Error 4100 is "unused formal parameter".
 */
# pragma warning(disable:4100)
#endif
