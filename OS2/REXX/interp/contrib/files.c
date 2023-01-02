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
 * This module is a real pain, since file I/O is one of the features
 * that varies most between different platforms. And what makes it
 * even more of a pain, is the fact that it must be coordinated with
 * the handling of the condition NOTREADY. Anyway, here are the
 * decisions set up before (well ... during) the implementation that
 * guide how this this thing is supposed to work.
 *
 * There are four kind of routines, structured in four levels:
 *
 * (1)---------+       (2)--------+
 * | builtin   | ----> | general  |   B   C library
 * | functions |   A   | routines | ---->  Routines
 * +-----------+       +----------+
 *       |                  |
 *       |                  |
 *       |                  V     (3)--------+
 *       +----------------->+---> | Error    |
 *                                | routines |
 *                                +----------+
 *
 * 1) Builtin functions, these has the "std_" prefix which is standard
 *    for all buildin functions. The task for these functions are to
 *    process parameters, call (2) which specializes on operations (like
 *    read, write, position etc), and return a decent answer back to its
 *    caller. There is one routine in this level for each of the
 *    functions in the library of built-in functions. Most of them are
 *    std_* functions, but there are a few others too.
 *
 * 2) These are general operations for reading, writing, positioning,
 *    etc.  They may call the C library routines directly, or
 *    indirectly, through calls to (3). The interface (A) between (1)
 *    and (2) is based on the local structure fileboxptr and strengs.
 *    There are one function in this level for each of the basic
 *    operations needed to be performed on a file. Opening, closing,
 *    reading a line, writing a line, line positioning, reading chars,
 *    writing chars, positioning chars, counting lines, counting
 *    chars, etc. The interface (B) to the C library routines uses
 *    FILE* and char* for its operations.
 *
 * 3) General routines to perform 'trivial' tasks. In this level,
 *    things like retriving Rexx's file table entries are implemented,
 *    and all the errorhandling. These are called from both the two
 *    previous levels.
 *
 * There are three standard files, called "<stdin>", "<stdout>" and
 * "<stderr>" (note that the "<" and ">" are part of the filename.)
 * These are handles for the equivalent Unix standard files. This
 * might cause problems if you actually do want a file calls that, or
 * if one of these files is closed, and the more information is
 * written to it (I can easily visulize Users trying to delete such a
 * file :-)) So the standard files -- having set flag SURVIVOR -- will
 * never be closed or reopened.
 *
 * Error_file is called by that routine which actually discovers the
 * problem. If it is an CALL ON condition, it will set the FLAG_FAKE
 * flag, which all other routines will check for.
 */

/*
 * Bug in LCC complier wchar.h that incorrectly says it defines stat struct
 * but doesn't
 */
#if defined(__LCC__)
# include <sys/stat.h>
#endif

#include "rexx.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif
#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif
#include <time.h>
#if defined(VMS)
# include <stat.h>
#elif defined(OS2)
# include <sys/stat.h>
# ifdef HAVE_UNISTD_H
#  include <unistd.h>
# endif
#elif (defined(__WATCOMC__) && !defined(__QNX__)) || defined(_MSC_VER) || defined(__LCC__)
# include <sys/stat.h>                                  /* MH 10-06-96 */
# include <fcntl.h>                                     /* MH 10-06-96 */
# ifdef HAVE_UNISTD_H
#  include <unistd.h>                                   /* MH 10-06-96 */
# endif
# if defined(_MSC_VER) && !defined(__WINS__)
#  include <io.h>
# endif
#elif defined(WIN32) && defined(__IBMC__)               /* LM 26-02-99 */
# include <io.h>
# include <sys/stat.h>
# include <fcntl.h>
#elif defined(MAC)
# include "mac.h"
#else
# include <sys/stat.h>
# ifdef HAVE_PWD_H
#  include <pwd.h>
#endif
# ifdef HAVE_GRP_H
#  include <grp.h>
# endif
# include <fcntl.h>
# ifdef HAVE_UNISTD_H
#  include <unistd.h>
# endif
#endif

#ifdef HAVE_DIRECT_H
# include <direct.h>
#endif

#ifdef __EMX__
# include <io.h>
#endif

#if defined(WIN32)
# ifdef _MSC_VER
#  if _MSC_VER >= 1100
/* Stupid MSC can't compile own headers without warning at least in VC 5.0 */
#   pragma warning(disable: 4115 4201 4214 4514)
#  endif
# endif
# include <windows.h>
# ifdef _MSC_VER
#  if _MSC_VER >= 1100
#   pragma warning(default: 4115 4201 4214)
#  endif
# endif
# if defined(__WATCOMC__) || defined(__BORLANDC__)
#  include <io.h>
# endif
#endif

#ifndef min
# define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

/*
 * Stuff for FSTAT
 */
#if defined(S_IFDIR) && !defined(S_ISDIR)
# define S_ISDIR(mode) (((mode) & S_IFDIR)==S_IFDIR)
#endif
#if defined(S_IFREG) && !defined(S_ISREG)
# define S_ISREG(mode) (((mode) & S_IFREG)==S_IFREG)
#endif
#if defined(S_IFCHR) && !defined(S_ISCHR)
# define S_ISCHR(mode) (((mode) & S_IFCHR)==S_IFCHR)
#endif
#if defined(S_IFBLK) && !defined(S_ISBLK)
# define S_ISBLK(mode) (((mode) & S_IFBLK)==S_IFBLK)
#endif
#if defined(S_IFLNK) && !defined(S_ISLNK)
# define S_ISLNK(mode) (((mode) & S_IFLNK)==S_IFLNK)
#endif
#if defined(S_IFFIFO) && !defined(S_ISFIFO)
# define S_ISFIFO(mode) (((mode) & S_IFFIFO)==S_IFFIFO)
#endif
#if defined(S_IFSOCK) && !defined(S_ISSOCK)
# define S_ISSOCK(mode) (((mode) & S_IFSOCK)==S_IFSOCK)
#endif
#if defined(S_IFNAM) && !defined(S_ISNAM)
# define S_ISNAM(mode) (((mode) & S_IFNAM)==S_IFNAM)
#endif

#if !defined(ACCESSPERMS)
# if defined(S_IRWXU) && defined(S_IRWXG) && defined(S_IRWXO)
#  define ACCESSPERMS (S_IRWXU|S_IRWXG|S_IRWXO)
# else
#  if defined(S_IREAD) && defined(S_IWRITE) && defined(S_IEXEC)
#   define ACCESSPERMS (S_IREAD|S_IWRITE|S_IEXEC)
#  else
#   define ACCESSPERMS (0xfff)
#  endif
# endif
#endif
/*
 * The macrodefinition below defines the various modes in which a
 * file may be opened. These modes are:
 *
 *  READ      - Open for readonly access. The current read position
 *              is set to the the start of the file. If the file does
 *              not exist, report an error.
 *
 *  WRITE     - Open for read and write. The current read position is
 *              set to the start of the file, while the current write
 *              position is set to EOF. If file does not exist, create
 *              it. If file does exist, use existing data.
 *
 *  UPDATE    - The combined operation of READ and WRITE, but if file
 *              does not exist, issue an error.
 *
 *  APPEND    - Open in APPEND mode, i.e. open for writeonly, position
 *              at the End-Of-File, and (if possible) open in a mode
 *              that disallows positioning. The file will be a transient
 *              file. If the file does not exist, create it.
 *
 *  CREATE    - Open for write, but if the file does exist, truncate
 *              it at the beginning after opening it.
 */
#define ACCESS_NONE           0
#define ACCESS_READ           1
#define ACCESS_WRITE          2
#define ACCESS_UPDATE         3
#define ACCESS_APPEND         4
#define ACCESS_CREATE         5
#define ACCESS_STREAM_APPEND  6
#define ACCESS_STREAM_REPLACE 7

/*
 * These macros is used to set the value of the 'oper' in the filebox
 * data structure. If last operation on a file was a read or a write,
 * set 'oper' to OPER_READ or OPER_WRITE, respectively. If last
 * operation was repositioning or flushing, use OPER_NONE. See
 * description of 'oper' field in definition of 'filebox'.
 */
#define OPER_NONE          0
#define OPER_READ          1
#define OPER_WRITE         2

/*
 * Flags, carrying information about files. The 'flag' field in the
 * 'filebox' structure is set to values matching these defintions. The
 * meaning of each of these flags is:
 *
 * PERSIST   - Set if file is persistent, if unset, file is treated
 *             as a transient file.
 * EOF       - Currently not in use
 * READ      - File has been opened for read access.
 * WRITE     - File has been opened for write access.
 * CREATE    - Currently not in use
 * ERROR     - Set if the file is in error state. If operations are
 *             attempted performed on files in state error, the
 *             NOTREADY condition will in general be raised, and the
 *             operation will fail.
 * SURVIVOR  - Set for certain special files; the default streams, which
 *             is not really to be closed or reopened.
 * FAKE      - Meaningful only if ERROR is set. If FAKE is set, and
 *             an operation on the file is attempted, the operation is
 *             'faked' (NOTREADY is not triggered, and the result returned
 *             for write operations does not report that the output was
 *             not written.
 * WREOF     - Current write position is at EOF. If line output is
 *             performed, there is no need to truncate the file.
 * RDEOF     - Current read position is at EOF. Reading EOF raises the
 *             NOTREADY condition, but does not put the file into error
 *             state.
 * AFTER_RDEOF - Bit of a hack here. This flag is set after an attempt
 * (Added by MH) is made to read a stream once the RDEOF flag is set.
 *               The reason for this is that all the "read" stream
 *               functions; LINEIN, LINES, CHARIN, etc set the RDEOF
 *               flag at the point that they determine a RDEOF has
 *               occurred. This is usually at the end of the function.
 *               Therefore a LINEIN that reads EOF sets RDEOF and a
 *               subsequent call to STREAM(stream,'S') will return
 *               NOTREADY.  This to me is logical, but the behaviour
 *               of other interpreters is that the first call to
 *               STREAM(stream,'S') after reaching EOF should still return
 *               READY. Only when ANOTHER "read" stream function is
 *               called does STREAM(stream,'S') return NOTREADY.
 * SWAPPED   - This flag is set if the file is currently swapped out, that
 *             is, the file is closed in order to let another file use
 *             the system's file table sloth freed when the file was
 *             temporarily closed.
 */
#define FLAG_PERSIST     0x0001
#define FLAG_EOF         0x0002
#define FLAG_READ        0x0004
#define FLAG_WRITE       0x0008
#define FLAG_CREATE      0x0010
#define FLAG_ERROR       0x0020
#define FLAG_SURVIVOR    0x0040
#define FLAG_FAKE        0x0080
#define FLAG_WREOF       0x0100
#define FLAG_RDEOF       0x0200
#define FLAG_SWAPPED     0x0400
#define FLAG_AFTER_RDEOF 0x0800

/*
 * So, what is the big difference between FAKE and ERROR. Well, when a
 * file gets it ERROR flag set, it signalizes that the file is in
 * error state, and that no fileoperations should be performed on it.
 * The FAKE flag is only meaningful when the ERROR flag is set. If set
 * the FAKE flag tells that file operations should be faked in order to
 * give the user the impression that everything is OK, while if FAKE is
 * not set, errors are returned.
 *
 * The clue is that if a statement contains several operations on one
 * file, and the first operation bombs, CALL ON NOTREADY will not take
 * effect before the next statement boundary at the same procedural
 * level So, for the rest of the file operations until that statement
 * has finished, the FAKE flag is set, and signalizes that OK result
 * should be returned whenever positioning or write is performed, and
 * that NOTREADY should not be raised again.
 *
 * The reason for the RDEOF flag is that reading beyond EOF is not really
 * a capital crime, and a lot of programmers are likely to do that, and
 * expect things to be OK after repositioning current read position to
 * another part of the file. If a file is put into ERROR state, it has
 * to be explicitly reset in order to do any useful to it afterwards.
 * Therefore, if EOF is seen on input, RDEOF is set, and NOTREADY is
 * raised, but the file is not put into ERROR state.
 */

/*
 * The following macros defines symbolic names to the commands available
 * in the Rexx built-in function STREAM(). The meaning of each of these
 * commands are:
 *
 *  READ       - Opens the file with the corresponding mode. For a deeper
 *  WRITE        description of each of these modes, see the defininition
 *  APPEND       of the ACCESS_* macros. STREAM() is used to explicitly
 *  UPDATE       open a file, while Rexx is totally happy with the
 *  CREATE       traditional implicit opening, i.e. that the file is
 *               opened for the needed access at the time when it is
 *               first used. If the file to be opened is already open,
 *               it will first be closed, and then opened in the
 *               specified mode.
 *
 *  CLOSE      - Closes a file, works for any type of access. But if
 *               the file is a default stream, it will not be closed.
 *               Default streams should not be closed.
 *
 *  FLUSH      - Performs flushing on the file. Actually, I'm not so
 *               sure whether that is very interesting, since flushing
 *               is always performed after a write, anyway. Though, it
 *               might become an important function if the automatic
 *               flushing after write is removed (e.g. to improve speed).
 *
 *  STATUS     - Returns status information associated with the file as
 *               a human readable string. The information returned is the
 *               internal information that Rexx stores in the Rexx file
 *               table entry for that file. Use FSTAT to get information
 *               about the file from the operating system. See the
 *               function 'getrexxstatus()' for more information about
 *               the layout of the returned string.
 *
 *  FSTAT      - Returns status information associated with the file as
 *               a human readable string. The information returned is the
 *               information normally returned by the stat() system call
 *               under Unix (i.e. size, dates, access modes, etc). Use
 *               STATUS to get Rexx's information about the file. See
 *               the function 'getstatus()' for more information about
 *               the layout of the string returned.
 *
 * RESET       - Resets the file after an error. Of course, this will
 *               only work for files which are 'resettable'. If the error
 *               is too serious, resetting will help little to fix the
 *               problem. E.g. writing beyond end-of-file can easily be
 *               fixed by RESET, trying to use a file which is named
 *               by an invalid syntax can not be correctly reset.
 *
 * READABLE    - Checks that the file in question is available in the
 * WRITABLE      mode given, for the user that is executing the script.
 * EXECUTABLE    I.e. READABLE will return '1' for a file, if the file
 *               is readable for the user, else '0' is returned. Note
 *               that FSTAT returns the information about the accessmodes
 *               for a file, these returns the 'accessmode' which is
 *               relevant for a particular user. Also note that if your
 *               machine are using suid-bit (i.e. Unix), this function
 *               will check for the real uid, not the effective uid.
 *               Consequently, it may not give the wanted result for
 *               suid rexx scripts, see the Unix access() function. (And
 *               anyway, suid scripts are a _very_ bad idea under Unix,
 *               so this is probably not a problem ... :-)
 */
#define COMMAND_NONE                       0
#define COMMAND_READ                       1
#define COMMAND_WRITE                      2
#define COMMAND_APPEND                     3
#define COMMAND_UPDATE                     4
#define COMMAND_CREATE                     5
#define COMMAND_CLOSE                      6
#define COMMAND_FLUSH                      7
#define COMMAND_STATUS                     8
#define COMMAND_FSTAT                      9
#define COMMAND_RESET                     10
#define COMMAND_READABLE                  11
#define COMMAND_WRITEABLE                 12
#define COMMAND_EXECUTABLE                13
#define COMMAND_LIST                      14
#define COMMAND_QUERY_DATETIME            15
#define COMMAND_QUERY_EXISTS              16
#define COMMAND_QUERY_HANDLE              17
#define COMMAND_QUERY_SEEK                18
#define COMMAND_QUERY_SIZE                19
#define COMMAND_QUERY_STREAMTYPE          20
#define COMMAND_QUERY_TIMESTAMP           21
#define COMMAND_QUERY_CREATETIME          22
#define COMMAND_QUERY_MODIFYTIME          23
#define COMMAND_QUERY_ACCESSTIME          24
#define COMMAND_QUERY_POSITION            25
#define COMMAND_QUERY                     26
#define COMMAND_QUERY_POSITION_READ       27
#define COMMAND_QUERY_POSITION_WRITE      28
#define COMMAND_QUERY_POSITION_SYS        29
#define COMMAND_QUERY_POSITION_READ_CHAR  30
#define COMMAND_QUERY_POSITION_READ_LINE  31
#define COMMAND_QUERY_POSITION_WRITE_CHAR 32
#define COMMAND_QUERY_POSITION_WRITE_LINE 33
#define COMMAND_OPEN                      34
#define COMMAND_OPEN_READ                 35
#define COMMAND_OPEN_WRITE                36
#define COMMAND_OPEN_BOTH                 37
#define COMMAND_OPEN_BOTH_APPEND          38
#define COMMAND_OPEN_BOTH_REPLACE         39
#define COMMAND_OPEN_WRITE_APPEND         40
#define COMMAND_OPEN_WRITE_REPLACE        41
#define COMMAND_SEEK                      42
#define COMMAND_POSITION                  43

#define STREAMTYPE_UNKNOWN     0
#define STREAMTYPE_PERSISTENT  1
#define STREAMTYPE_TRANSIENT   2
/*
 * Define TRUE_TRL_IO, if you want the I/O system to be even more like
 * TRL. It will try to mimic the behaviour in TRL exactly. Note that if
 * you _do_ define this, you might experience a degrade in runtime
 * performance.
 */
#define TRUE_TRL_IO

/*
 * There are two ways to report an error for file I/O operations. Either
 * as an "error" or as a "warning". Both will raise the NOTREADY
 * condition, but only ERROR will actually put the file into ERROR mode.
 * Warnings are used for e.g. EOF while reading. Both are implemented
 * by the same routine.
 */
#define file_error(a,b,c)   handle_file_error(TSD,a,b,c,1)
#define file_warning(a,b,c) handle_file_error(TSD,a,b,c,0)

/*
 * CASE_SENSITIVE_FILENAMES is used to determine if internal file
 * pointers respect the case of files and treat "ABC" as a different
 * file to "abc".
 */
#ifdef UNIX
# define CASE_SENSITIVE_FILENAMES
#endif
/*
 * Regina truncates a file when repositioning by the use of a line
 * count. That is, if the file has ten lines, and you use the BIF
 * lineout(file,,4), it will be truncated after the fourth line.
 * Truncating is not performed for character repositioning.
 *
 * If you don't want truncating after line repositioning, undefine
 * the macro HAVE_FTRUNCATE in config.h. Also, if your system doesn't
 * have ftruncate(), undefine HAVE_FTRUNCATE, and survive without the
 * truncating.
 *
 * The function ftruncate() is a BSDism; if you have trouble finding
 * it, try linking with -lbsd or -lucb or something like that. Since
 * it is not a standard POSIX feature, some machines may generate
 * warnings during compilation. Let's help these machines ...
 */
#if defined(FIX_PROTOS) && defined(HAVE_FTRUNCATE)
# if defined(ultrix)
   int ftruncate( int fd, int length ) ;
# endif
#endif

/*
 * Since development of Ultrix has ceased, and they never managed to
 * fix a few things, we want to define a few things, just in order
 * to kill a few warnings ...
 */
#if defined(FIX_PROTOS) && defined(FIX_ALL_PROTOS) && defined(ultrix)
   int fstat( int fd, struct stat *buf ) ;
   int stat( char *path, struct stat *buf ) ;
#endif

#include "regina64.h"

#if defined(WIN32)
/*
 * Work around for bug in WIN32 stat() function; can't have trailing slash
 */
# if defined(HAVE__STATI64)
static int rx_w32_stat( const char *path, struct _stati64 *buffer )
{
   int lastpos;
   int rc;
   char tmpstring[REXX_PATH_MAX ];

   rc = _stati64( path, buffer );
   if ( rc != 0 && errno == ENOENT )
   {
      if ( path )
      {
         lastpos = strlen( path ) -  1;
         if ( path[lastpos] == '\\' || path[lastpos] == '/')
         {
            memcpy( tmpstring, path, lastpos ) ;
            tmpstring[lastpos] = '\0';
            rc = _stati64( tmpstring, buffer );
         }
      }
   }
   return rc;
}
# else
static int rx_w32_stat( const char *path, struct _stat *buffer )
{
   int lastpos;
   int rc;
   char tmpstring[REXX_PATH_MAX ];

   rc = stat( path, buffer );
   if ( rc != 0 && errno == ENOENT )
   {
      if ( path )
      {
         lastpos = strlen( path ) -  1;
         if ( path[lastpos] == '\\'|| path[lastpos] == '/')
         {
            memcpy( tmpstring, path, lastpos ) ;
            tmpstring[lastpos+1] = '\0';
            rc = stat( tmpstring, buffer );
         }
      }
   }
   return rc;
}
# endif
#endif

/*
 * Here comes another 'sunshine-story' ...  Since SunOS don't have
 * a decent set of include-files in the standard version of the OS,
 * their <stdio.h> don't define these macros. Instead, Sun seems to
 * survive with the old custom of using the numberic values of these
 * macros directly. If compiled with "SunKlugdes" defined, try to
 * fix this.
 *
 * If you are using gcc on a Sun, you may want to run the program
 * fixincludes that comes with gcc. It will fix this more permanently.
 * At least one recent version of GCC for VMS doesn't have this

 */
#if defined(SunKludges) || (defined(__GNUC__) && defined(VMS))
# define SEEK_SET  0
# define SEEK_CUR  1
# define SEEK_END  2
#endif

/*
 * Some machines don't defined these ... they should!
 */
#if defined(VMS) || defined(_MSC_VER) || (defined(WIN32) && defined(__IBMC__)) || (defined(WIN32) && defined(__BORLANDC__)) || defined(__LCC__)
# define F_OK 0
# define X_OK 1
# define W_OK 2
# define R_OK 4
#endif

/*
 * Here is the datastructure in which to store the information about
 * open files. The storage format of the file table is discussed
 * elsewhere. The fields used to handle storing are 'next' and 'prev'
 * which is used to implement a double linked list of files having
 * the same hashfunc; and 'newer' and 'older' which are used to maintain
 * a large double linked list of all files in order of the most
 * recently used file.
 *
 * The other fields are:
 *
 *  fileptr    - Pointer to the filehandle use by the system when
 *               accessing the file through the normal I/O calls.
 *               If this pointer is NULL, it means that the file is
 *               not currently open.
 *  oper       - Holds the value that tells whether the most recent
 *               operation on this file was a read or a write. This has
 *               importance for flushing, since a read can't imediately
 *               follow a write (or vice versa) without a flush (or
 *               a repositioning) inbetween. Takes the values OPER_READ,
 *               OPER_WRITE or OPER_NONE (signalizes that most recent
 *               operation can be followed by either read or write).
 *  flag       - Bitfield that holds information about the file. The
 *               significance of the various fields are described by
 *               the FLAG_* macros.
 *  error      - Most recently 'errno' code for this file. It could have
 *               been stored into 'errmsg' instead, but that would require
 *               copying of data which might not be used. If undefined,
 *               it will have the value 0.
 *  readpos    - The current read position in the file, as a character
 *               position. Note that this is in 'C-syntax', i.e. the
 *               first character in the file is numbered "0". A value of
 *               -1 means that the value is unknown or undefined.
 *  readline   - The line number of the current read position, which must
 *               be positive if define. A value of zero means that the
 *               line number is undefined or unknown. If current read
 *               position is at an EOL, 'readline' refers to the line
 *               preceding the EOL.
 *  writepos   - Similar to 'readpos' but for current write position.
 *  writeline  - Similar to 'readline' but for current write position.
 *  filename0  - Pointer to string containing the filename assiciated
 *               with this file. This string is garanteed to have an
 *               ASCII NUL following the last character, so it can be
 *               used directly in file operations. This field *must*
 *               be defined.
 *  errmsg     - Error message associated with the file. Some errors are
 *               not trapped during call to system routines, and these
 *               does not have an error message defined by the opsys.
 *               E.g. when positioning current read position after EOF.
 *               This field stores errormessages for these situations.
 *               If undefined, it will be a NULL pointer.
 *
 * Both errmsg and error can not be defined simultaneously.
 */

typedef struct fileboxtype *fileboxptr ;
typedef const struct fileboxtype *cfileboxptr ;
typedef struct fileboxtype {
   FILE *fileptr ;
   unsigned char oper ;
//   size_t readpos, writepos, thispos ;
   rx_64 readpos, writepos, thispos ;
   rx_64 readline, writeline, linesleft ;
   int flag, error;
   fileboxptr prev, next ;    /* within a filehash entry */
   fileboxptr newer, older ;
   streng *filename0 ;
   streng *errmsg ;
} filebox ;

/* POSIX denies read and write operations on streams without intermediate
 * fflush, fseek, fsetpos or rewind (list from EMX). We use the following
 * macros to switch directly before an I/O operation. "Useful" fseeks should
 * be error checked. This is not necessary here since the following operation
 * will fault in case of an error.
 */

#define SWITCH_OPER_READ(fptr)  {if (fptr->oper==OPER_WRITE)          \
                                    rx_fseek(fptr->fileptr,0l,SEEK_CUR); \
                                 fptr->oper=OPER_READ;}
#define SWITCH_OPER_WRITE(fptr) {if (fptr->oper==OPER_READ)           \
                                    rx_fseek(fptr->fileptr,0l,SEEK_CUR); \
                                 fptr->oper=OPER_WRITE;}

typedef struct
{  /* fil_tsd: static variables of this module (thread-safe) */
   /*
    * The following two pointers are pointers into the doble linked list
    * of all files in the file table. They points to the most recently
    * used file, and the least recently used open file. Note that the latter
    * of these are _not_ the same as the last file in the list. If the
    * Rexx' file table contains more files than the system's file table
    * can contain, 'lrufile' will point to the last open file in the double
    * linked list. Files further out in the list are 'swapped' out.
    */
   fileboxptr mrufile;

   fileboxptr stdio_ptr[6];
   void *     rdarea;
   fileboxptr filehash[131];
   int        rol_size ;       /* readoneline() */
   char *     rol_string ;     /* readoneline() */
   int        got_eof ;        /* readkbdline() */
} fil_tsd_t; /* thread-specific but only needed by this module. see
              * init_filetable
              */
/*
 * Structure to define stream types; names and whether persistent,transient or unknown
 */
typedef struct
{
   int streamtype;
   char *streamname;
} stream_type_t;

#define STREAMTYPE_DIRECTORY          1
#define STREAMTYPE_CHARACTERSPECIAL   2
#define STREAMTYPE_BLOCKSPECIAL       3
#define STREAMTYPE_REGULARFILE        4
#define STREAMTYPE_FIFO               5
#define STREAMTYPE_SYMBOLICLINK       6
#define STREAMTYPE_SOCKET             7
#define STREAMTYPE_SPECIALNAME        8

static const stream_type_t stream_types[] =
{
   { STREAMTYPE_UNKNOWN  , ""                  },
   { STREAMTYPE_UNKNOWN  , " Directory"        },
   { STREAMTYPE_PERSISTENT," CharacterSpecial" },
   { STREAMTYPE_PERSISTENT," BlockSpecial"     },
   { STREAMTYPE_PERSISTENT," RegularFile"      },
   { STREAMTYPE_UNKNOWN  , " FIFO"             },
   { STREAMTYPE_UNKNOWN  , " SymbolicLink"     },
   { STREAMTYPE_UNKNOWN  , " Socket"           },
   { STREAMTYPE_UNKNOWN  , " SpecialName"      },
};

static rx_64 positioncharfile( tsd_t *TSD, const char *bif, int argno, fileboxptr fileptr, int oper, rx_64 where, int from );
static rx_64 positionfile( tsd_t *TSD, const char *bif, int argno, fileboxptr ptr, int oper, rx_64 lineno, int from );
static void handle_file_error( tsd_t *TSD, fileboxptr ptr, int rc, const char *errmsg, int level) ;
static int flush_output( tsd_t *TSD, fileboxptr ptr );
static const char *translateTilde( tsd_t *TSD, const char *src );
int my_rx_stat(  tsd_t *TSD, const char *src, struct stat *statbuf );

/*
 * Based on the st_mode filed returned from stat(), determine the Regina "stream type".
 * The returned value is an index into stream_types array, which allows the caller to
 * determine if the stream is persistent, transient or unknown, and also allows the
 * user to look up the stream type name.
 * Added to resolve 802114.
 */
static int determine_stream_type( int mode )
{
#ifdef S_ISDIR
   if ( S_ISDIR(mode) )
      return STREAMTYPE_DIRECTORY;
#endif
#ifdef S_ISCHR
   if ( S_ISCHR(mode) )
      return STREAMTYPE_CHARACTERSPECIAL;
#endif
#ifdef S_ISBLK
   if ( S_ISBLK(mode) )
      return STREAMTYPE_BLOCKSPECIAL;
#endif
#ifdef S_ISREG
   if ( S_ISREG(mode) )
      return STREAMTYPE_REGULARFILE;
#endif
#ifdef S_ISFIFO
   if ( S_ISFIFO(mode) )
      return STREAMTYPE_FIFO;
#endif
#ifdef S_ISLNK
   if ( S_ISLNK(mode) )
      return STREAMTYPE_SYMBOLICLINK;
#endif
#ifdef S_ISSOCK
   if ( S_ISSOCK(mode) )
      return STREAMTYPE_SOCKET;
#endif
#ifdef S_ISNAM
   if ( S_ISNAM(mode) )
      return STREAMTYPE_SPECIALNAME;
#endif
   return STREAMTYPE_UNKNOWN;
}

/*
 * Marks all entries in the filetable. Used only by the memory
 * management. Does not really change anything, so you can in general
 * forget this one. This routine is called from memory.c in order to
 * mark all statically defined data in this file.
 */
#ifdef TRACEMEM
void mark_filetable( const tsd_t *TSD )
{
   fileboxptr ptr=NULL ;
   fil_tsd_t *ft;

   ft = (fil_tsd_t *)TSD->fil_tsd;
   for (ptr=ft->mrufile; ptr; ptr=ptr->older)
   {
      markmemory( ptr, TRC_FILEPTR ) ;
      markmemory( ptr->filename0, TRC_FILEPTR ) ;
      if (ptr->errmsg)
         markmemory( ptr->errmsg, TRC_FILEPTR ) ;
   }

   if (ft->rdarea)
      markmemory( ft->rdarea, TRC_FILEPTR ) ;

}
#endif /* TRACEMEM */

#if defined(WIN32) && defined(_MSC_VER)
/*
 * This is a replacement fo the BSD ftruncate() function.
 * The code in this function was written by Les Moull.
 */

int ftruncate( int fd, long pos )
{
   HANDLE h = (HANDLE)_get_osfhandle( fd ) ;

   if (SetFilePointer( h, pos, NULL, FILE_BEGIN) == 0xFFFFFFFF)
      return -1;

   if ( !SetEndOfFile( h ) )
      return -1;

   return 0;
}
#endif

#if defined(__WATCOMC__) && defined(__QNX__)
# define ftruncate( fd, pos ) ltrunc( fd, pos, SEEK_SET )
#endif

/*
 * This command maps the string 'cmd' into a number which is to be
 * interpreted according to the settings of the COMMAND_ macros.
 * The input strings must be one of the valid command, or else the
 * COMMAND_NONE value is returned.
 *
 * Well, this routine should really have been implemented differently,
 * since sequential searching through a list of strings is not very
 * efficient. But still, it is not so many entries in the list, and
 * this function is not going to be called often, so I suppose it
 * doesn't matter too much. Ideallistic, it should be rewritten to
 * a binary search.
 */

static char get_command( streng *cmd )
{
   Str_upper(cmd);

   if (cmd->len>=5 && !memcmp(cmd->value,  "QUERY", 5))
      return COMMAND_QUERY ;
   if (cmd->len>=4 && !memcmp(cmd->value,  "OPEN", 4))
      return COMMAND_OPEN ;
   if (cmd->len==5 && !memcmp(cmd->value, "CLOSE", 5))
      return COMMAND_CLOSE ;
   if (cmd->len==5 && !memcmp(cmd->value, "FLUSH", 5))
      return COMMAND_FLUSH ;
   if (cmd->len==6 && !memcmp(cmd->value, "STATUS", 6))
      return COMMAND_STATUS ;
   if (cmd->len==5 && !memcmp(cmd->value, "FSTAT", 5))
      return COMMAND_FSTAT ;
   if (cmd->len==5 && !memcmp(cmd->value, "RESET", 5))
      return COMMAND_RESET ;
   if (cmd->len==8 && !memcmp(cmd->value, "READABLE", 8))
      return COMMAND_READABLE ;
   if (cmd->len==8 && !memcmp(cmd->value, "WRITABLE", 8))
      return COMMAND_WRITEABLE ;
   if (cmd->len==8 && !memcmp(cmd->value, "WRITEABLE", 9))
      return COMMAND_WRITEABLE ;
   if (cmd->len==10 && !memcmp(cmd->value, "EXECUTABLE", 10))
      return COMMAND_EXECUTABLE ;
   if (cmd->len>=4 && !memcmp(cmd->value, "SEEK", 4))
      return COMMAND_SEEK ;
   if (cmd->len>=8 && !memcmp(cmd->value, "POSITION", 8))
      return COMMAND_POSITION ;
   if (cmd->len==4 && !memcmp(cmd->value, "READ", 4))
      return COMMAND_READ ;
   if (cmd->len==5 && !memcmp(cmd->value, "WRITE", 5))
      return COMMAND_WRITE ;
   if (cmd->len==6 && !memcmp(cmd->value, "APPEND", 6))
      return COMMAND_APPEND ;
   if (cmd->len==6 && !memcmp(cmd->value, "UPDATE", 6))
      return COMMAND_UPDATE ;
   if (cmd->len==6 && !memcmp(cmd->value, "CREATE", 6))
      return COMMAND_CREATE ;
   if (cmd->len==4 && !memcmp(cmd->value, "LIST", 4)) /* not implemented */
      return COMMAND_LIST ;
   return COMMAND_NONE ;
}

static char get_querycommand( const streng *cmd )
{
   if (cmd->len==6 && !memcmp(cmd->value,  "EXISTS", 6))
      return COMMAND_QUERY_EXISTS ;
   if (cmd->len==4  && !memcmp(cmd->value, "SIZE", 4))
      return COMMAND_QUERY_SIZE ;
   if (cmd->len==8 && !memcmp(cmd->value,  "DATETIME", 8))
      return COMMAND_QUERY_DATETIME ;
   if (cmd->len>=4 && !memcmp(cmd->value,  "SEEK", 4))
      return COMMAND_QUERY_SEEK ;
   if (cmd->len>=8 && !memcmp(cmd->value,  "POSITION", 8))
      return COMMAND_QUERY_POSITION ;
   if (cmd->len==10 && !memcmp(cmd->value, "STREAMTYPE", 10))
      return COMMAND_QUERY_STREAMTYPE ;
   if (cmd->len==9  && !memcmp(cmd->value, "TIMESTAMP", 9))
      return COMMAND_QUERY_TIMESTAMP ;
   if (cmd->len==10  && !memcmp(cmd->value, "CREATETIME", 10))
      return COMMAND_QUERY_CREATETIME;
   if (cmd->len==10  && !memcmp(cmd->value, "MODIFYTIME", 10))
      return COMMAND_QUERY_MODIFYTIME;
   if (cmd->len==10  && !memcmp(cmd->value, "ACCESSTIME", 10))
      return COMMAND_QUERY_ACCESSTIME;
   if (cmd->len==6 && !memcmp(cmd->value,  "HANDLE", 6))
      return COMMAND_QUERY_HANDLE ;
   return COMMAND_NONE ;
}

static char get_querypositioncommand( const streng *cmd )
{
   if (cmd->len>=4 && !memcmp(cmd->value,  "READ", 4))
      return COMMAND_QUERY_POSITION_READ ;
   if (cmd->len>=5 && !memcmp(cmd->value,  "WRITE", 5))
      return COMMAND_QUERY_POSITION_WRITE ;
   if (cmd->len==3 && !memcmp(cmd->value,  "SYS", 3))
      return COMMAND_QUERY_POSITION_SYS ;
   return COMMAND_NONE ;
}

static char get_querypositionreadcommand( const streng *cmd )
{
   if (cmd->len==4 && !memcmp(cmd->value,  "CHAR", 4))
      return COMMAND_QUERY_POSITION_READ_CHAR ;
   if (cmd->len==4 && !memcmp(cmd->value,  "LINE", 4))
      return COMMAND_QUERY_POSITION_READ_LINE ;
   if (cmd->len==0)
      return COMMAND_QUERY_POSITION_READ_CHAR ;
   return COMMAND_NONE ;
}

static char get_querypositionwritecommand( const streng *cmd )
{
   if (cmd->len==4 && !memcmp(cmd->value,  "CHAR", 4))
      return COMMAND_QUERY_POSITION_WRITE_CHAR ;
   if (cmd->len==4 && !memcmp(cmd->value,  "LINE", 4))
      return COMMAND_QUERY_POSITION_WRITE_LINE ;
   if (cmd->len==0)
      return COMMAND_QUERY_POSITION_WRITE_CHAR ;
   return COMMAND_NONE ;
}

static char get_opencommand( const streng *cmd )
{
   if (cmd->len>=4 && !memcmp(cmd->value,  "BOTH", 4))
      return COMMAND_OPEN_BOTH ;
   if (cmd->len==4 && !memcmp(cmd->value,  "READ", 4))
      return COMMAND_OPEN_READ ;
   if (cmd->len>=5 && !memcmp(cmd->value,  "WRITE", 5))
      return COMMAND_OPEN_WRITE ;
   if (cmd->len==0)
      return COMMAND_OPEN_BOTH ;
   return COMMAND_NONE ;
}

static char get_opencommandboth( const streng *cmd )
{
   if (cmd->len==6 && !memcmp(cmd->value,  "APPEND", 6))
      return COMMAND_OPEN_BOTH_APPEND ;
   if (cmd->len==7 && !memcmp(cmd->value,  "REPLACE", 7))
      return COMMAND_OPEN_BOTH_REPLACE ;
   if (cmd->len==0)
      return COMMAND_OPEN_BOTH ;
   return COMMAND_NONE ;
}

static char get_opencommandwrite( const streng *cmd )
{
   if (cmd->len==6 && !memcmp(cmd->value,  "APPEND", 6))
      return COMMAND_OPEN_WRITE_APPEND ;
   if (cmd->len==7 && !memcmp(cmd->value,  "REPLACE", 7))
      return COMMAND_OPEN_WRITE_REPLACE ;
   if (cmd->len==0)
      return COMMAND_OPEN_WRITE ;
   return COMMAND_NONE ;
}


/* ==================================================================== */
/*                       level 3 routines                               */

/*
 * Returns the fileboxptr of a file, if is has already been opened.
 * If it does not exist in Rexx's file table, a NULL pointer is
 * returned in stead. It is easy to change the datastruction in
 * which the file table is stored.
 *
 * If using VMS, or another opsys that has a caseinsensitive file
 * system, maybe it should disregard the case of the filename. In
 * general, maybe it should 'normalize' the file name before storing
 * it in the file table (do we sence an upcoming namei() :-)
 */

#define FILEHASH_SIZE (sizeof(((fil_tsd_t*)0)->filehash) / \
                       sizeof(((fil_tsd_t*)0)->filehash[0]))

#ifdef CASE_SENSITIVE_FILENAMES
#define filehashvalue(strng) (hashvalue(strng->value, strng->len) % FILEHASH_SIZE)
#else
#define filehashvalue(strng) (hashvalue_ic(strng->value, strng->len) % FILEHASH_SIZE)
#endif

static void removefileptr( const tsd_t *TSD, cfileboxptr ptr )
{
   fil_tsd_t *ft;

   ft = (fil_tsd_t *)TSD->fil_tsd;

   if (ft->mrufile==ptr)
      ft->mrufile = ptr->older ;

   if (ptr->older)
      ptr->older->newer = ptr->newer ;

   if (ptr->newer)
      ptr->newer->older = ptr->older ;

   if (ptr->next)
      ptr->next->prev = ptr->prev ;

   if (ptr->prev)
      ptr->prev->next = ptr->next ;
   else
      ft->filehash[filehashvalue(ptr->filename0)] = ptr->next ;
}

/* enterfileptr initializes a fileboxptr. It must be allocated and the
 * following fields must already been set:
 * errmsg, error, fileptr, flag, filename0
 */
static void enterfileptr( const tsd_t *TSD, fileboxptr ptr )
{
   int hashval=0 ;
   fil_tsd_t *ft;

   ft = (fil_tsd_t *)TSD->fil_tsd;

   /*
    * First, get the magic number for this file. Note that when we're
    * doing hashing like this, we *may* get trouble on machines that
    * don't differ between upper and lower case letters in filenames.
    */
   hashval = filehashvalue(ptr->filename0) ;
   /*
    * Then, link it into the list of values having the same hashvalue
    */
   ptr->next = ft->filehash[hashval] ;
   if (ptr->next)
      ptr->next->prev = ptr ;
   ft->filehash[hashval] = ptr ;
   ptr->prev = NULL ;

   /*
    * Then, link it into the 'global' list of files, sorted by how
    * recently they have been used.
    */
   ptr->older = ft->mrufile ;
   if (ptr->older)
      ptr->older->newer = ptr ;
   ptr->newer = NULL ;
   ft->mrufile = ptr ;

   ptr->readline = 0 ;
   ptr->linesleft = 0 ;
   ptr->writeline = 0 ;
   ptr->thispos = (size_t) EOF ;
   ptr->readpos = (size_t) EOF ;
   ptr->writepos = (size_t) EOF ;
   ptr->oper = OPER_NONE;
}

/* swapout_file swaps out one closeable file. The state persists. Use
 * swapin_file to reuse it.
 * The given fileboxptr MUST NOT be swapped out. It indicates a file which
 * should be swapped in after this operation. dont_swap may be NULL.
 */
static void swapout_file( tsd_t *TSD, fileboxptr dont_swap )
{
   fil_tsd_t *ft;
   fileboxptr start, run, found;

   ft = (fil_tsd_t *)TSD->fil_tsd;
   /*
    * Too many open files simultaneously, we have to close one down
    * in order to free one file descriptor, but only if there actually
    * are some files that can be closed down.
    */
   found = NULL;

   if ( ( start = dont_swap ) == NULL ) /* any start point is better than */
      start = ft->mrufile ;             /* mru head. We need the opposite */

   /* first try finding an older file */
   for ( run = start ; run ; run = run->older )
   {
      if ( ( (run->flag & ( FLAG_SURVIVOR | FLAG_SWAPPED ) ) == 0 ) &&
           ( run->fileptr != NULL ) &&
           ( run != dont_swap ) )
         found = run ; /* continue looking for an older file */
   }

   /* if !found, try finding a more recent swapable file */
   if ( found == NULL )
   {
      for ( run = start ; run ; run = run->newer )
      {
         if ( ( (run->flag & ( FLAG_SURVIVOR | FLAG_SWAPPED ) ) == 0 ) &&
              ( run->fileptr != NULL ) &&
              ( run != dont_swap ) )
         {
            found = run ; /* least newer swapable file */
            break;
         }
      }
   }
   if ( found == NULL )
      exiterror( ERR_SYSTEM_FAILURE, 0 )  ;

   flush_output( TSD, found );
}


/* swapout_all swaps out all closeable files. The states persist. Use
 * swapin_file to reuse them.
 * This function is useful when exiting the external's interface.
 */
static void swapout_all( tsd_t *TSD )
{
   fil_tsd_t *ft;
   fileboxptr run;

   ft = (fil_tsd_t *)TSD->fil_tsd;

   for ( run = ft->mrufile ; run ; run = run->older )
   {
      flush_output( TSD, run );
   }
}

#ifdef VMS
static const char *acc_mode[] = { "r",  "r+",  "a"  } ;
#else
static const char *acc_mode[] = { "rb", "r+b", "ab" } ;
#endif

#define ACCMODE_READ  0
#define ACCMODE_RDWRT 1
#define ACCMODE_WRITE 2
#define ACCMODE_NONE  3

static void swapin_file( tsd_t *TSD, fileboxptr ptr )
{
   int faccess=0, itmp=0 ;

   /*
    * First, just try to reopen the file, we _might_ have a vacant
    * entry in the system file table, so, use that.
    */
   itmp = (ptr->flag & (FLAG_READ | FLAG_WRITE)) ;
   if (itmp==(FLAG_READ | FLAG_WRITE))
      faccess = ACCMODE_RDWRT ;
   else if (itmp==(FLAG_READ))
      faccess = ACCMODE_READ ;
   else if (itmp==(FLAG_WRITE))
      faccess = ACCMODE_WRITE ;
   else
     faccess = ACCMODE_NONE ;

   if (faccess == ACCMODE_NONE)
      exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;

tryagain:
#ifdef SGIkludges
   errno = EMFILE ;
#else
   errno = 0 ;
#endif
   ptr->fileptr = fopen( ptr->filename0->value, acc_mode[faccess] ) ;
   if ((!ptr->fileptr) && (errno == EMFILE))
   {
      swapout_file( TSD, ptr ) ;
      goto tryagain ;
   }

   ptr->flag &= ~(FLAG_SWAPPED) ;
   if (ptr->fileptr==NULL)
      file_error( ptr, errno, NULL ) ;
   else
   {
      if ( ptr->thispos == (size_t) EOF )
         rx_fseek( ptr->fileptr, 0, SEEK_SET ) ;
      else
         rx_fseek( ptr->fileptr, ptr->thispos, SEEK_SET ) ;
      /*
       * If the swapped-in file was already after EOF; ie
       * ptr->flag contained FLAG_RDEOF, then force eof()
       * to return true.
       */
      if ( ptr->flag & FLAG_RDEOF )
      {
         rx_fseek( ptr->fileptr, 0, SEEK_END );
         fgetc( ptr->fileptr );
      }
   }
}

/*
 * Compares two filesnames, either with ignoring or respecting the letter case.
 */
int filename_cmp( const streng *name1, const streng *name2 )
{
#ifdef CASE_SENSITIVE_FILENAMES
   return Str_cmp( name1, name2 );
#else
   return Str_ccmp( name1, name2 );
#endif
}

static fileboxptr getfileptr( tsd_t *TSD, const streng *name )
{
   int hashval ;
   fileboxptr ptr ;
   fil_tsd_t *ft;

   ft = (fil_tsd_t *)TSD->fil_tsd;

   hashval = filehashvalue( name ) ;
   /*
    * First, try to find the correct file in this slot of the
    * hash table. If one is found, ptr points to it, else, ptr is set
    * to NULL
    */
   for (ptr=ft->filehash[hashval];ptr;ptr=ptr->next)
   {
      if ( !filename_cmp( name, ptr->filename0 ) )
         break ;
   }
   /*
    * In order not to create any problems later, just return with NULL
    * (signifying that no file was found) if that is the case. Then we may
    * able to assume that ptr _is_ set later.
    */
   if (!ptr)
      return NULL ;

   /*
    * Now, put the file in front of the list of files stored by how
    * recently they were used. We assume that any access to a file is
    * equivalent to the file being used.
    */
   if ( ptr != ft->mrufile )
   {
      if ( ptr->newer )
      {
         ptr->newer->older = ptr->older ;
      }
      if ( ptr->older )
      {
         ptr->older->newer = ptr->newer ;
      }

      ptr->older = ft->mrufile ;
      ptr->newer = NULL ;
      ft->mrufile->newer = ptr ;
      ft->mrufile = ptr ;
   }
   if ( ptr != ft->filehash[ hashval ] )
   {
      /* Why the hell do we use ft->mrufile? Is it useful for anything?
       * The following code speeds up getfileptr much more.
       * If the current file pointer (ptr) is not the first file pointer
       * in the list for this hash value; reposition it so that it is
       * at the front of the list, and move the current file pointer
       * that is at the front of the list after ptr.
       * Fixes bug 594553
       */
      if ( ptr->next )
         ptr->next->prev = ptr->prev ;
      if ( ptr->prev )
         ptr->prev->next = ptr->next ;

      ptr->prev = NULL;
      ptr->next = ft->filehash[ hashval ];
      ft->filehash[ hashval ]->prev = ptr;
      ft->filehash[ hashval ] = ptr ;
   }

   /*
    * If this file has been swapped out, we have to reopen it, so we can
    * continue to access it.
    */
   if (ptr->flag & FLAG_SWAPPED)
      swapin_file( TSD, ptr ) ;

   return ptr ;
}


static void flush_input( cfileboxptr dummy )
{
   dummy = dummy; /* keep compiler happy */
   return ;
}


/*
 * flush_output does close the file. The purpose of this function is to free
 * up space for opening another file while maintaining all state information about
 * this file if/when it is needed again.
 * Returns -1 in case of an error, 0 on success.
 */
static int flush_output( tsd_t *TSD, fileboxptr ptr )
{
   int h;

   errno = 0;

   if ( ptr->fileptr == NULL || ptr->flag & FLAG_SWAPPED )
      return 0;

   if ( ptr->flag & FLAG_SURVIVOR )
   {
      if ( ptr->flag & FLAG_WRITE )
      {
         if ( fflush( ptr->fileptr ) != 0 )
         {
            file_error( ptr, errno, NULL );
            return -1;
         }
      }
      return 0;
   }

   if ( fflush( ptr->fileptr ) != 0 )
   {
      h = errno;
      fclose( ptr->fileptr );
      ptr->fileptr = NULL;
      ptr->flag |= FLAG_SWAPPED;
      file_error( ptr, h, NULL );
      return -1;
   }
   if ( fclose( ptr->fileptr ) == EOF )
   {
      h = errno;
      ptr->fileptr = NULL;
      ptr->flag |= FLAG_SWAPPED;
      file_error( ptr, h, NULL );
      return -1;
   }

   ptr->fileptr = NULL;
   ptr->flag |= FLAG_SWAPPED;
   return 0;
}

/*
 * Sets up the internal filetable for REXX, and initializes it with
 * the three standard files under Unix, stderr, stdout og and stdin.
 * Should only be called once, from the main routine. We should also
 * add code to register the routine for marking memory from this
 * routine.
 *
 * As a shortcut to access these three default files, there is a
 * variable 'stdio_ptr' which contains pointers to them. This allows
 * for quick access to the default streams.
 * The function returns 1 on success, 0 if memory is short.
 *
 * IMPORTANT!
 * The entry for stdin must be the same as the following #define for
 * DEFAULT_STDIN_INDEX below. Never changes it.
 * This assumption is used in readkbdline().
 */
#define DEFAULT_STDIN_INDEX 0
#define DEFAULT_STDOUT_INDEX 1
#define DEFAULT_STDERR_INDEX 2
int init_filetable( tsd_t *TSD )
{
   int i=0 ;
   fil_tsd_t *ft;

   if (TSD->fil_tsd != NULL)
      return(1);

   if ( ( TSD->fil_tsd = MallocTSD( sizeof(fil_tsd_t) ) ) == NULL )
      return(0);
   ft = (fil_tsd_t *)TSD->fil_tsd;
   memset( ft, 0, sizeof(fil_tsd_t) );

   for ( i = 0; i < 6; i++ )
   {
      ft->stdio_ptr[i] = (fileboxptr)MallocTSD( sizeof( filebox )) ;
      ft->stdio_ptr[i]->errmsg = NULL ;
      ft->stdio_ptr[i]->error = 0 ;
   }

   ft->stdio_ptr[0]->fileptr = ft->stdio_ptr[3]->fileptr = stdin ;
   ft->stdio_ptr[1]->fileptr = ft->stdio_ptr[4]->fileptr = stdout ;
   ft->stdio_ptr[2]->fileptr = ft->stdio_ptr[5]->fileptr = stderr ;

   ft->stdio_ptr[0]->flag = ft->stdio_ptr[3]->flag = ( FLAG_SURVIVOR + FLAG_READ ) ;
   ft->stdio_ptr[1]->flag = ft->stdio_ptr[4]->flag = ( FLAG_SURVIVOR + FLAG_WRITE ) ;
   ft->stdio_ptr[2]->flag = ft->stdio_ptr[5]->flag = ( FLAG_SURVIVOR + FLAG_WRITE ) ;

   ft->stdio_ptr[0]->filename0 = Str_crestrTSD( "<stdin>" ) ;
   ft->stdio_ptr[1]->filename0 = Str_crestrTSD( "<stdout>" ) ;
   ft->stdio_ptr[2]->filename0 = Str_crestrTSD( "<stderr>" ) ;
   ft->stdio_ptr[3]->filename0 = Str_crestrTSD( "stdin" ) ;
   ft->stdio_ptr[4]->filename0 = Str_crestrTSD( "stdout" ) ;
   ft->stdio_ptr[5]->filename0 = Str_crestrTSD( "stderr" ) ;

   for (i=0; i<6; i++)
      enterfileptr( TSD, ft->stdio_ptr[i] ) ;

   return(1);
}

void purge_filetable( tsd_t *TSD )
{
   fileboxptr ptr1, ptr2, save_ptr1, save_ptr2 ;
   int i;
   fil_tsd_t *ft;

   ft = (fil_tsd_t *)TSD->fil_tsd;
   /* Naming this the "removal loop".   */
   for ( ptr1=ft->mrufile; ptr1; )
   {
      save_ptr1 = ptr1->older ;
      for ( ptr2=ptr1; ptr2; )
      {
         save_ptr2 = ptr2->next ;      /* this was moved from third parm of loop
                                          so that it did not address the free'd
                                          memory.  See if statement below.   */
         /*
          * If this is one of the default streams, don't let it be closed.
          * These file shall stay open, whatever happens.
          */
         /*
          * JH 19991105 if was modified to include the next 5 statements.  Originally,
          * the file was not closed, but all other references to it were deleted.  In
          * situations where one *.exe invokes Rexx mutiple times, subsequent calls to
          * the standard streams caused an error.  (getfileptr() failed, the file name
          * for stdio_ptr[?] comes up blank.)
          */
         if (!(ptr2->flag & FLAG_SURVIVOR)
         &&  ptr2->fileptr)
         {
            fclose( ptr2->fileptr ) ;

            removefileptr( TSD, ptr2 ) ;

            if (ptr2->errmsg)
               Free_stringTSD( ptr2->errmsg ) ;

            Free_stringTSD( ptr2->filename0 ) ;
            FreeTSD( ptr2 ) ;
         }
         ptr2 = save_ptr2 ;
      }
      ptr1 = save_ptr1 ;
   }

   ft->mrufile = NULL;

   /*
    * Now lets be absolutely paranoid, and remove all entries from the
    * filehash table...
    */
   memset( ft->filehash, 0, sizeof(ft->filehash) );
   /*
    * JH 19991105 The following loop was added to re-instate the std streams into the
    * hash table.  It seems easier to do this then to muck around with reseting the pointers
    * as the fileboxptr's are deleted.  Cannot modify the loop above to look at filenames
    * before removing from filehas table, it might be pointing to a fileboxptr that got removed
    * by the "removal loop".
    */
   for (i=0; i<6; i++)
   {
      enterfileptr( TSD, ft->stdio_ptr[i] ) ;
   }

#if 0
// can't free this as the next call to RexxStart() expects this memory to be here
   if ( TSD->fil_tsd )
   {
      FreeTSD( TSD->fil_tsd );
      TSD->fil_tsd = NULL;
   }
#endif
}

/*
 * checkProperStreamName raises 40.27 if errno describes an error leading
 * to the assumption that the filename was malformed according to ANSI 9.2.1.
 */
static void checkProperStreamName( tsd_t *TSD, streng *kill, const char *fn,
                                   int eno )
{
   static const int bad[] = {
#if defined(ENAMETOOLONG)
      ENAMETOOLONG,
#endif
      0
   };

   int i;

   for ( i = 0; bad[i] != 0; i++ )
   {
      if ( eno == bad[i] )
      {
         /*
          * ANSI 9.2.1 wants us to raise 40.27 if stream is malformed.
          * Feel free to provide more errno values describing this situation.
          */
         if ( kill )
           Free_stringTSD( kill ) ;
         exiterror( ERR_INCORRECT_CALL, 27, BIFname( TSD ), fn );
      }
   }
}

/*
 * Sets the proper error conditions for the file, including providing a
 * a hook into the CALL/SIGNAL ON system. Now, we also like to set some
 * other information, like the status of the file (taken from rc).
 *
 * First parameter is the file to operate on, the second and third
 * parameters are the error message to set (they can't both be defined),
 * and the last parameter is the level of 'severity'. If set, the file
 * is thrown into error state.
 */
static void handle_file_error( tsd_t *TSD, fileboxptr ptr, int rc, const char *errmsg, int level)
{
   trap *traps=NULL ;

   assert( !(rc && errmsg) ) ;

   if ((ptr->flag & FLAG_ERROR) && (ptr->flag & FLAG_FAKE))
   {
      /*
       * If we are faking for this file already, don't bother to do anything
       * more. In particular, we do not want to set a new error, since that
       * will in general only overwrite the old (and probably more relevant)
       * error message. However, faking are _only_ done when NOTREADY is
       * being trapped.
       */
      return ;
   }
   else
   {
      /*
       * If the file is not already in error, set the ERROR flag, and record
       * the error message. Also, clear the FAKE flag. This flag is only
       * defined when the ERROR flag is set, and we don't want any old
       * values laying around (it will be set later if needed).
       */
      if (level)
      {
         ptr->flag &= ~FLAG_FAKE ;
         ptr->flag |= FLAG_ERROR ;
      }
      else if (ptr->flag & FLAG_RDEOF)
      {
         /*
          * If the file was in RDEOF state; ie EOF was read on the file
          * set the AFTER_RDEOF flag to ensure STREAM(stream,'S') works
          * like other interpreters.
          */
         ptr->flag |= FLAG_AFTER_RDEOF;
      }

      checkProperStreamName( TSD, ptr->errmsg, Str_val( ptr->filename0 ), rc );

      /*
       * Set the error message, but only if one was given. This routine
       * can be called _without_ any errormessage, and if so, keep the
       * old one (if any)
       */
      if (rc || errmsg)
      {
         if (ptr->errmsg)
           Free_stringTSD( ptr->errmsg ) ;

         ptr->error = rc ;
         if ( errmsg )
            ptr->errmsg = Str_creTSD( errmsg ) ;
         else
#ifdef WIN32
         {
            /*
             * For Win32 always get the last error, and store that;
             * it seems to be more meaningful than strerror( errno )
             * Address bug ???? FIXME
             */
            CHAR LastError[256];
            ULONG last_error = GetLastError();
            if ( last_error )
            {
               FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT), LastError, 256, NULL ) ;
               ptr->errmsg = Str_creTSD( LastError ) ;
            }
            else
               ptr->errmsg = NULL;
         }
#else
         ptr->errmsg = NULL;
#endif
      }

      /*
       * OK, the file has been put into ERROR state, now we must check
       * to see if we should raise the NOTREADY condition. If NOTREADY
       * is not currently enabled, don't bother to try to raise it.
       */
      traps = gettraps( TSD, TSD->currlevel ) ;
      if (traps[SIGNAL_NOTREADY].on_off)
      {
         /*
          * The NOTREADY condition is being trapped; set the FAKE flag
          * so that we don't create more errors for this file. But _only_
          * set the FAKE flag if NOTREADY is trapped by method CALL.
          * Then raise the condition ...
          */
         if (!traps[SIGNAL_NOTREADY].invoked)
            ptr->flag |= FLAG_FAKE ;

         condition_hook(TSD,SIGNAL_NOTREADY,rc+100,0,-1,Str_dupTSD(ptr->filename0),NULL);
      }
   }
}



/*
 * This routine is supposed to be called when the condition is triggered
 * by method CALL. From the time the condition is raised until the CALL is
 * is triggered, I/O to the file is faked. But before the condition handler
 * is called, we try to tidy things up a bit.
 *
 * At least, we have to clear the FAKE flag. Other 'nice' things to do
 * is to clear error indicator in the file pointer, and to reset the
 * file in general. The ERROR state is not cleared, _unless_ the file
 * is one of the default streams.
 */

void fixup_file( tsd_t *TSD, const streng *filename )
{
   fileboxptr ptr=NULL ;

   if ( filename )
   {
      /*
       * filename will be NULL when condition_hook() called with a NULL description
       * argument. This happens when NOTREADY occurs when pulling from an external
       * queue with a timeout and the timeout expires.
       */
      ptr = getfileptr( TSD, filename ) ;
      if (ptr)
      {
         /*
          * If the file is open, try to clear it, first clear the error
          * indicator, and then try to fseek() to a 'safe' point. If the
          * seeking didn't work out, don't bother, it was worth a try.
          */
         if (ptr->fileptr)
         {
            clearerr( ptr->fileptr ) ;
            if ( ptr->flag & FLAG_PERSIST )
               rx_fseek( ptr->fileptr, 0, SEEK_SET ) ;
            ptr->thispos = 0 ;
            ptr->oper = OPER_NONE ;
         }

         if (ptr->flag & FLAG_SURVIVOR)
         {
            ptr->flag &= ~(FLAG_ERROR) ;
            /*
             * MHES Added following 4 flag resets - 30-11-2004
             */
            ptr->flag &= ~(FLAG_RDEOF) ;
            ptr->flag &= ~(FLAG_WREOF) ;
            ptr->flag &= ~(FLAG_AFTER_RDEOF) ;
         }

         ptr->flag &= ~(FLAG_FAKE) ;
      }
   }
}




/*
 * This is stupid ... if the file exists, but is in error mode, we
 * shall not close it, but leave it open, so that the rest of the
 * operations on this file in this statement don't trip. Same happens
 * if we are not able to close it properly. Oh well ...
 *
 * On second thoughts ... Faking only applies for input and output.
 * So closing doesn't have to be faked. Remove the file, whatever
 * happens.
 */
void closefile( tsd_t *TSD, const streng *name )
{
   fileboxptr ptr=NULL ;

   /* If it isn't open, don't try to close it ... */
   ptr = getfileptr( TSD, name ) ;
   if (ptr)
   {
      /*
       * If this is one of the default streams, don't let it be closed.
       * These file shall stay open, whatever happens.
       */
      if (ptr->flag & FLAG_SURVIVOR)
         return ;

      /*
       * If the fileptr seems to point to something ... close it. We
       * really don't want to leak file table slots. Actually, we should
       * check that the close was ok, and not let the fileptr go unless
       * we know that it was really closed (and released for new use).
       * Previously, it only closed when file was not in error. I don't
       * know what is the correct action, but this seems to be the most
       * sensible ...
       */
      if (ptr->fileptr)
         fclose( ptr->fileptr ) ;

      removefileptr( TSD, ptr ) ;

      if (ptr->errmsg)
         Free_stringTSD( ptr->errmsg ) ;

      Free_stringTSD( ptr->filename0 ) ;
      FreeTSD( ptr ) ;
   }
}




/*
 * This function is called when we need some kind of access to a file
 * but don't (yet) have it. It will only be called when we want to
 * open a file implicitly, e.g. it is open for reading, and it has then
 * been named in a output function.
 *
 * This is rather primitive ... but this function can only be called
 * when the file is open for read, and we want to open it for write;
 * or if the file i open for write, and we want to open it for read.
 * So I think this will suffice. It ignores the 'access' parameter
 * And just assumes that the file must be opened in both read and
 * write.
 *
 * To improve on this function, we ought to do a lot more checking,
 * e.g. that the 'access' wanted are required, and that the file is
 * already open in some kind of mode. If this don't hold, we probably
 * have an error condition.
 *
 * We should also check another thing, that the new file which is opened
 * is in fact the same file that we closed. Perferably, we should open
 * the new file, then check the device and inode of both the old and
 * new file to see whether they are the same (using stat()). If they
 * are not the same, the reopening should fail. As it is implemented
 * now, the Unix method for temporary files (open it, remove it,
 * use it, and then close it) will fail; and we loose access to the
 * original file too.
 */
static void reopen_file( tsd_t *TSD, fileboxptr ptr )
{
   if (!ptr)
      exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;

   /*
    * We can not reopen the default streams, that makes no sence. If
    * tried, report an error.
    */
   if (ptr->flag & FLAG_SURVIVOR)
   {
      file_error( ptr, 0, "Invalid operation on default stream" ) ;
      return ;
   }

   /*
    * Close the old file, and try to reopen the new file. There is the
    * same problem here as in closefile(); if closing didn't work (for
    * some mysterious reason), the system's file table should become
    * full. Better checking might be required.
    */
   errno = 0 ;
   fclose( ptr->fileptr ) ;
#ifdef VMS
   ptr->fileptr = fopen( ptr->filename0->value, "r+" ) ;
#else
   ptr->fileptr = fopen( ptr->filename0->value, "r+b" ) ;
#endif
   if (ptr->fileptr==NULL)
   {
      file_error( ptr, errno, NULL ) ;
      return ;
   }
   ptr->oper = OPER_NONE ;

   /*
    * We definitively want to set the close-on-exec flag. Suppose
    * an output file has not been flushed, and we execute a command.
    * This might perform an exec() and then a system(), which _will_
    * flush all files (close them). The result is that the file might
    * be flushed twice ... not good.
    *
    * This don't work on VMS ... but the file system on VMS is so
    * different anyway, so it will probably not create any problems.
    * Besides, we don't do exec() and system() on VMS.
    */
#if !defined(VMS) && !defined(MAC) && !defined(OS2) && !defined(DOS) && !defined(__WATCOMC__) && !defined(_MSC_VER) && !(defined(WIN32) && defined(__IBMC__)) && !defined(__MINGW32__) && !defined(__BORLANDC__) && !defined(__EPOC32__) && !defined(__LCC__) && !defined(SKYOS)
   if (ptr && ptr->fileptr)
   {
      int flags, fno ;
      fno = fileno( ptr->fileptr ) ;
      assert( fno >= -1) ;
      flags = fcntl( fno, F_GETFD ) ;
      flags |= FD_CLOEXEC ;
      if (fcntl( fno, F_SETFD, flags)== -1)
         exiterror( ERR_SYSTEM_FAILURE, 1, strerror(errno) )  ;
   }
#endif

   /*
    * If readposition is EOF (=illegal), then we "probably" needed to
    * open it in read mode. Set the current read position to the start
    * of the file.
    */
   if (ptr->readpos == (size_t) EOF)
   {
      ptr->readline = 1 ;
      ptr->linesleft = 0 ;
      ptr->readpos = 0 ;
      ptr->thispos = 0 ;
      if ( ptr->flag & FLAG_PERSIST )
         rx_fseek( ptr->fileptr, 0, SEEK_SET ) ;
   }

   /*
    * Then do the same thing for write access. We always set this to the
    * end-of-file -- the default -- even though there are other write
    * modes available. If the file is implicitly open in write mode,
    * then the current write position should be set to the default
    * value.
    */
   if (ptr->writepos == (size_t) EOF)
   {
      ptr->writeline = 0 ;
      if ( ptr->flag & FLAG_PERSIST )
         rx_fseek( ptr->fileptr, 0, SEEK_END ) ;
      ptr->writepos = rx_ftell( ptr->fileptr ) ;
      ptr->thispos = ptr->writepos ;
   }

   /*
    * Then, at last, do some simple bookkeeping, set both read and
    * write access, and clear any previous problem.
    */
   ptr->flag = FLAG_READ | FLAG_WRITE | FLAG_PERSIST ;
   ptr->error = 0 ;
   if (ptr->errmsg)
      Free_stringTSD(ptr->errmsg) ;

   ptr->errmsg = NULL ;
}



/*
 * This function explicitly opens a file. It will be called if the user
 * has called the built-in function STREAM() in order to open a file
 * in a particular mode. It will also be called if the file is not
 * previously open, and is used in a read or write operation.
 *
 * It takes two parameters, the name of the file to open, and the
 * mode in which it is to be opened. The mode has a value which is
 * matched by the ACCESS_ macros defined earlier.
 *
 * If the file is actually open in advance, then we close it before we
 * do any other operations. If the user is interested in the file in
 * one particular mode, he is probably not interested in any previous
 * modes.
 */
static fileboxptr openfile( tsd_t *TSD, const streng *name, int faccess )
{
   fileboxptr ptr=NULL ;
   long lpos=0L ;

   /*
    * First check wether this file is already open, and use that open
    * file if possible. However, that may not be possible, since we
    * may want to use the file for another operation now. So, if the
    * file _is_ open, check to see if access is right.
    */
   ptr = getfileptr( TSD, name ) ;
   if (ptr)
   {
      if (ptr->flag & FLAG_SURVIVOR)
      {
         file_error( ptr, 0, "Can't open a default stream" ) ;
         return ptr ;
      }
      closefile( TSD, name ) ;
   }

   /*
    * Now, get a new file table entry, and fill in the various
    * field with appropriate (i.e. default) values.
    */
   ptr = (fileboxptr)MallocTSD( sizeof(filebox) ) ;
   ptr->filename0 = Str_dupstrTSD( name ) ;
   ptr->flag = 0 ;
   ptr->error = 0 ;
   ptr->errmsg = NULL ;
   ptr->readline = 0 ;
   ptr->linesleft = 0 ;
   ptr->writeline = 0 ;
   ptr->thispos = (size_t) EOF ;
   ptr->readpos = (size_t) EOF ;
   ptr->writepos = (size_t) EOF ;
   ptr->oper = OPER_NONE;

   /*
    * suppose we tried to open, but didn't manage, well, stuff it into
    * the file table, we might want to retrieve information about it
    * later on. _And_ we need to know about the problem if the file
    * I/O is to be faked later on.
    */
   enterfileptr( TSD, ptr ) ;
   name = ptr->filename0 ;
   goto try_to_open ;

kill_one_file:
   swapout_file( TSD, ptr ) ;

try_to_open:
   /*
    * In most of these, we have to check that the file opened is really
    * a persistent file. We should not take that for granted.
    */
   errno = 0 ;
   if (faccess==ACCESS_READ)
   {
#ifdef VMS
      if ((ptr->fileptr = fopen( name->value, "r" )) != NULL)
#else
      if ((ptr->fileptr = fopen( name->value, "rb" )) != NULL)
#endif
      {
         ptr->flag = FLAG_READ | FLAG_PERSIST ;
         ptr->readline = 1 ;
         ptr->linesleft = 0 ;
         ptr->thispos = ptr->readpos = 0 ;
      }
      else if (errno==EMFILE)
         goto kill_one_file ;
      else
      {
         file_error( ptr, errno, NULL ) ;
      }
   }
   else if (faccess==ACCESS_WRITE)
   {
      /*
       * This is really a problem. If opened in mode "w", it will
       * truncate the file if it did exist. If opened int mode "r+",
       * it will fail if the file did not exist. So we try to
       * combine the two.
       */
      ptr->flag = FLAG_READ ;
#ifdef VMS
      ptr->fileptr = fopen( name->value, "r+" ) ;
#else
      ptr->fileptr = fopen( name->value, "r+b" ) ;
#endif
      errno = 0 ;
      if (!ptr->fileptr)
#ifdef VMS
         ptr->fileptr = fopen( name->value, "w+" ) ;
#else
         ptr->fileptr = fopen( name->value, "w+b" ) ;
#endif

      errno = 0 ;
      if (!ptr->fileptr)
      {
#ifdef SGIkludges
         errno = EMFILE ;
#else
         errno = 0 ;
#endif
#ifdef VMS
         ptr->fileptr = fopen( name->value, "w" ) ;
#else
         ptr->fileptr = fopen( name->value, "wb" ) ;
#endif
         ptr->flag &= 0 ;
      }

      /*
       * Then set the current read and write positions to the start and
       * the end of the file, respectively. When we first open the file
       * we can quickly determine readpos, writepos and readline, but
       * writeline is expensive to determine, because we have to read
       * the whole file to determine this. So we don't do this when we
       * open the file, because we may never use it. Instead we set the
       * value of writeline to 0 to indicate that the actual position
       * is unknown.  When we do want to use writeline, we have to
       * determine it then.
       */
      if (ptr->fileptr)
      {
         ptr->flag |= FLAG_WRITE | FLAG_PERSIST ;
         rx_fseek( ptr->fileptr, 0, SEEK_END ) ;
         lpos = rx_ftell( ptr->fileptr ) ;
         ptr->thispos = ptr->writepos = lpos ;
         ptr->writeline = 0 ;
         ptr->readpos = 0 ;
         ptr->readline = 1 ;
         ptr->linesleft = 0 ;
      }
      else if (errno==EMFILE)
         goto kill_one_file ;
      else
         file_error( ptr, errno, NULL ) ;
   }
   else if (faccess==ACCESS_APPEND)
   {
      /*
       * In append mode, the file is opened as a transient file, all
       * writing must be done at the end of the file. It is not
       * possible to perform reading on the file. Useful for files
       * to which you have write, but not read access (e.g. logfiles).
       */
#ifdef VMS
      if ((ptr->fileptr = fopen( name->value, "a" )) != NULL)
#else
      if ((ptr->fileptr = fopen( name->value, "ab" )) != NULL)
#endif
      {
         ptr->flag = FLAG_WRITE | FLAG_WREOF ;
      }
      else if (errno==EMFILE)
         goto kill_one_file ;
      else
         file_error( ptr, errno, NULL ) ;
   }
   else if (faccess==ACCESS_STREAM_APPEND)
   {
      /*
       * In "stream" append mode, the file is opened as a persistent file, all
       * writing must be done at the end of the file. It is not
       * possible to perform reading on the file. Useful for files
       * to which you have write, but not read access (e.g. logfiles).
       */
#ifdef VMS
      if ((ptr->fileptr = fopen( name->value, "a" )) != NULL)
#else
      if ((ptr->fileptr = fopen( name->value, "ab" )) != NULL)
#endif
      {
         ptr->flag = FLAG_WRITE | FLAG_WREOF | FLAG_PERSIST;
         if ( ptr->flag & FLAG_PERSIST )
            rx_fseek( ptr->fileptr, 0, SEEK_END ) ;
         lpos = rx_ftell( ptr->fileptr ) ;
         ptr->thispos = ptr->writepos = lpos ;
         ptr->writeline = 0 ; /* unknown position */
         ptr->readpos = 0 ;
         ptr->readline = 1 ;
         ptr->linesleft = 0 ;
      }
      else if (errno==EMFILE)
         goto kill_one_file ;
      else
         file_error( ptr, errno, NULL ) ;
   }
   else if (faccess==ACCESS_STREAM_REPLACE)
   {
      /*
       * The file is created if it didn't exist, and if it did exist
       * it is truncated and the file pointers set to the start of file.
       */
#ifdef VMS
      if ((ptr->fileptr = fopen( name->value, "w+" )) != NULL)
#else
      if ((ptr->fileptr = fopen( name->value, "w+b" )) != NULL)
#endif
      {
         ptr->flag = FLAG_WRITE | FLAG_READ | FLAG_WREOF | FLAG_RDEOF |
                     FLAG_PERSIST ;
         ptr->writeline = ptr->readline = 1 ;
         ptr->linesleft = 0 ;
         ptr->readpos = ptr->writepos = ptr->thispos = 0 ;
      }
      else if (errno==EMFILE)
         goto kill_one_file ;
      else
         file_error( ptr, errno, NULL ) ;
   }
   else if (faccess==ACCESS_UPDATE)
   {
      /*
       * Like read access, but it will not create the file if it didn't
       * already exist. Instead, an error is reported.
       */
#ifdef VMS
      if ((ptr->fileptr = fopen( name->value, "r+" )) != NULL)
#else
      if ((ptr->fileptr = fopen( name->value, "r+b" )) != NULL)
#endif
      {
         ptr->flag = FLAG_WRITE | FLAG_READ | FLAG_PERSIST ;
         ptr->readline = 0 ;
         ptr->linesleft = 0 ;
         ptr->writeline = 0 ; /* unknown */
      }
      else if (errno==EMFILE)
         goto kill_one_file ;
      else
         file_error( ptr, errno, NULL ) ;
   }
   else if (faccess==ACCESS_CREATE)
   {
      /*
       * The file is created if it didn't exist, and if it did exist
       * it is truncated.
       */
#ifdef VMS
      if ((ptr->fileptr = fopen( name->value, "w+" )) != NULL)
#else
      if ((ptr->fileptr = fopen( name->value, "w+b" )) != NULL)
#endif
      {
         ptr->flag = FLAG_WRITE | FLAG_READ | FLAG_WREOF | FLAG_RDEOF |
                     FLAG_PERSIST ;
         ptr->writeline = ptr->readline = 1 ;
         ptr->linesleft = 0 ;
         ptr->readpos = ptr->writepos = ptr->thispos = 0 ;
      }
      else if (errno==EMFILE)
         goto kill_one_file ;
      else
         file_error( ptr, errno, NULL ) ;
   }
   else
      exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;

#if !defined(VMS) && !defined(MAC) && !defined(OS2) && !defined(DOS) && !defined(__WATCOMC__) && !defined(_MSC_VER) && !defined(__MINGW32__) && !defined(__BORLANDC__) && !defined(__EPOC32__) && !defined(__LCC__) && !defined(SKYOS)
   /*
    * Then we check to see if this is a transient or persistent file.
    * We can remove a 'persistent' setting, but add one, since we
    * sometimes want to access a persistent file as transient (append).
    */
   if (ptr->fileptr)
   {
      int fno, rc ;
      struct rx_stat_buf statbuf ;
      errno = 0 ;
      fno = fileno(ptr->fileptr) ;
      rc = rx_fstat( fno, &statbuf ) ;
      if (rc==0 && !S_ISREG(statbuf.st_mode))
         ptr->flag &= ~(FLAG_PERSIST) ;
      else if (rc!=0)
         file_error( ptr, errno, NULL ) ;
   }

   /*
    * As with reopen_file(), we want to set the close-on-exec flag,
    * see reopen_file for more information.
    */
   if (ptr->fileptr)
   {
      int flags, fno ;
      fno = fileno(ptr->fileptr) ;
      assert( fno >= -1) ;
      flags = fcntl( fno, F_GETFD ) ;
      flags |= FD_CLOEXEC ;
      if (fcntl( fno, F_SETFD, flags)== -1)
         exiterror( ERR_SYSTEM_FAILURE, 1, strerror(errno) )  ;
   }
#endif

   return (ptr) ;
}




/* ------------------------------------------------------------------- */
/*                     High level utility routines                     */




/*
 * This function is really just an interface to the function getfileptr().
 * It takes a (possible) filename, and retrieves the corresponding
 * Rexx file table entry. If the file does not exist, it is opened in
 * the mode indicated by 'open_mode'. If it does exist, this routine
 * verifies that it it has been opened in a mode corresponding to
 * 'access' (OPER_READ or OPER_WRITE).
 *
 * If the file does not exist, it is opened in either normal read
 * or normal write. This correcspinds to an "implicit" file open
 * in Rexx.
 */
static fileboxptr get_file_ptr( tsd_t *TSD, const streng *name, int faccess, int open_mode )
{
   fileboxptr ptr=NULL ;

   ptr = getfileptr( TSD, name ) ;
   if (ptr==NULL)
      return openfile( TSD, name, open_mode ) ;

   if (ptr->flag & FLAG_ERROR)
      return ptr ;

   if (faccess==OPER_READ && (!(ptr->flag & FLAG_READ)))
      reopen_file( TSD, ptr ) ;
   else if (faccess==OPER_WRITE && (!(ptr->flag & FLAG_WRITE)))
      reopen_file( TSD, ptr ) ;

   return ptr ;
}



/*
 * This routine reads one complete line from the file indicated by
 * the file table entry 'ptr'. Or rather, it read from the current
 * read position, until and including the first EOL mark, and returns
 * that. If the EOL mark is implemented as certain characters, they are
 * not returned. It closely corresponds to the LINEIN() built-in
 * function.
 *
 * What is the upper limit for the size that we might read in? It's
 * best not to have any limit, so the method is the following: A
 * temporary area is used for storing the data read from the file.
 * We never know the size needed until the EOL mark is found. So
 * just read the data into the temporary area. If the EOL is found,
 * then we know the size, and we can transfer the data into a 'streng'
 * of suitable size. If the temporary area is too small, allocate
 * an area twice the size, and copy the data over. Afterwards, keep the
 * new area as the temporary area.
 *
 * This way, we normally use little memory, but we are still able to
 * read as large lines as the memory allows, if it is needed.
 *
 * No error condition is raised if noerrors is set. Instead, NULL is returned.
 */
static streng *readoneline( tsd_t *TSD, fileboxptr ptr )
{
   int i=0, j=0, eolf=0, eolchars=1 ;
/*#if !defined(UNIX)*/
   int k=0;
/*#endif*/
   streng *ret=NULL ;
   fil_tsd_t *ft;

   ft = (fil_tsd_t *)TSD->fil_tsd;

   /*
    * First verify that we actually have a file that is not in an
    * ERROR state. If so, don't perform any operations.
    */
   if ( ptr->flag & FLAG_ERROR )
   {
      if (!(ptr->flag & FLAG_FAKE))
         file_error( ptr, 0, NULL ) ;

      return nullstringptr() ;
   }

   /*
    * If we have an EOF from the last linein() then cause the NOTREADY
    * condition now.
    */
   if ( ptr->flag & FLAG_RDEOF )
   {
      file_warning( ptr, 0, "EOF on line input" ) ;
   }

   /*
    * If the string is not yet allocated, allocate it, and use an
    * initial size of 512 bytes. This can be increased during runtime.
    * Using higher initial sizes will waste allocation time on modern systems
    * with page sizes around 4KB. 512 fits most cases at its best.

    * MH 4 Sep 02 We should read into the ft->rol_string 512 bytes at a
    * time and search through the buffer for the EOL. getc() for every
    * character is innefficient, however (at least on Linux) getc() is
    * implemented as a read( 4096 ), and then returns each character
    * from some internal buffer.
    */
   if (!ft->rol_string)
   {
      ft->rol_string = (char *)MallocTSD( (ft->rol_size=512) ) ;
#ifdef TRACEMEM
      ft->rdarea = ft->rol_string ;
#endif
   }

 /*
   if (ptr->fileptr==stdin)
      fcntl( stdin, F_SETFL, O_NONBLOCK | fcntl(stdin,F_GETFL)) ;
  */
   errno = 0 ;
   SWITCH_OPER_READ(ptr);
   /*
    * Switch to the current readpos setting in case the current position
    * is based on the write position.
    */
   ptr->thispos = ptr->readpos;
   if ( ptr->flag & FLAG_PERSIST )
      rx_fseek(ptr->fileptr,ptr->thispos,SEEK_SET);
   for (i=0; ; i++)
   {
      j = getc(ptr->fileptr);
      if (j == REGINA_EOL)
      {
         eolf = REGINA_EOL;
         break;
      }
/*#if !defined(UNIX) && !defined(MAC)*/
      /*
       * MH 25042003 - all platforms can read lines with
       * CRLF, LF, or CR - consistent with ObjectRexx
       */
      if (j == REGINA_CR)
      {
         k = getc(ptr->fileptr);
         if (k == REGINA_EOL)
         {
            eolf = REGINA_EOL;
            eolchars = 2;
            break;
         }
         else
         {
            ungetc(k,ptr->fileptr);
            eolf = REGINA_EOL;
            break;
         }
      }
/*#endif*/
      /*
       * If we hit end-of-file, handle it carefully, and terminate the
       * reading. Note that this means that there we have read an
       * incomplete last line, so return what we've got, and report
       * an NOTREADY condition. (Partly, I disagree, but this is how
       * TRL defines it ... Case I: Programmer using Emacs forgets to
       * add a EOL after the last line; Rexx triggers NOTREADY when
       * reading that last, incomplete line.

       * MH 4-Sep-2002 - change in semantics happened a while ago.
       * We treat incomplete line as a "normal" line, and NOTREADY
       * is NOT raised when reading an incomplete line.
       */
      if (j==EOF)
      {
         ptr->flag |= FLAG_RDEOF ;
/*            file_warning( ptr, 0, "EOF on line input" ) ; */
         break ;
      }

      /*
       * We are trying to avoid any limits other than memory-imposed
       * limits. So if the buffer size that we currently have are too
       * small, double it, and hide the operation from the rest of the
       * interpreter.
       */
      if (i>=ft->rol_size)
      {
         char *tmpstring ;

         assert( i == ft->rol_size ) ;
         tmpstring = (char *)MallocTSD( 2*ft->rol_size+10 ) ;
         memcpy( tmpstring, ft->rol_string, ft->rol_size ) ;
         FreeTSD( ft->rol_string ) ;
         ft->rol_string = tmpstring ;
         ft->rol_size *= 2 ;
#ifdef TRACEMEM
         ft->rdarea = ft->rol_string ;
#endif
      }

      /*
       * Just an ordinary character ... append it to the buffer
       */
      ft->rol_string[i] = (char) j ;
   }
   /*
    * Attempt to set the read pointer and the current file
    * pointer based on the length of the line we just read.
    */
#if 1 /* really MH */
   if ( ptr->thispos == ptr->readpos )
   {
      if ( ptr->thispos == (size_t) EOF )
      {
         errno = 0 ;
         ptr->thispos = ptr->readpos = rx_ftell( ptr->fileptr ) ;
      }
      else
      {
         ptr->thispos += (i - (j==EOF)) + eolchars ;
         ptr->readpos = ptr->thispos ;
      }
   }
   else
   {
      errno = 0 ;
      ptr->thispos = ptr->readpos = rx_ftell( ptr->fileptr ) ;
   }
#else
   if (ptr->thispos != (size_t) EOF)
      ptr->thispos += (i - (j==EOF)) + eolchars ;

   if (ptr->readpos != (size_t) EOF)
      ptr->readpos = ptr->thispos ;
#endif
   /*
    * If we did read a complete line, we have to increment the line
    * count for the current read pointer of this file. This part of
    * the code is a bit Unix-ish. It will have to be reworked for
    * other types of operating systems.
    */
   if ((eolf==REGINA_EOL) && (ptr->readline > 0))
   {
#if 1 /* really MH */
      ptr->readline += 1 ;  /* only if we actually saw the "\n" !!! */
#else
      ptr->readline += eolchars ;  /* only if we actually saw the "\n" !!! */
#endif
      if (ptr->linesleft)
         ptr->linesleft-- ;
   }
   /*
    * A bit of a hack here.  Because countlines() determines if any lines
    * are left in the stream by using the feof() function, we have to
    * attempt to read the EOF after each line, and set the file's state
    * to EOF.  If the character read is not EOF, then put it back on
    * the stream to be read later.
    * Only do this for persistent streams!!
    */
   if ( ptr->flag & FLAG_PERSIST
   &&   !feof( ptr->fileptr ) )
   {
      int ch0;
      ch0 = getc(ptr->fileptr);
      if (feof(ptr->fileptr))
      {
         ptr->flag |= FLAG_RDEOF ;
/*         file_warning( ptr, 0, "EOF on line input" ) ; */
      }
      else
      {
         ungetc(ch0,ptr->fileptr);
      }
   }

   /*
    * Wrap up the data that was read, and return it as a 'streng'.
    *
    */
/*   if (i>1000) i = 1000 ; */
   ret = Str_makeTSD( i ) ;
   memcpy( ret->value, ft->rol_string, ret->len=i ) ;
   return ret ;
}


static rx_64 positionfile_SEEK_SET( tsd_t *TSD, const char *bif, int argno, fileboxptr ptr, int oper, rx_64 lineno )
{
   int ch=0x00;
   rx_64 from_line=0, tmp=0 ;
   rx_64 from_char=0L ;
   rx_64 ret;

   /*
    * We know the line number of at most three positions in the file:
    * the start of the file, the write position and the read position.
    * If the file is open only for reading or writing, we know at most
    * two positions. And in addition, the read and/or the write
    * position may be be invalid (i.e. previous operation was
    * character oriented). But at least, we know the line number of
    * one position, the start of the file, which is the first line.
    *
    * The best method seems to be: First start with the start of file
    * and then see if using the read or the write position instead is
    * a better deal. There is one drawback ... we assume that all lines
    * are equally long. That assumption is probably not too bad for text
    * files, but it may create unnecessary overhead for 'peculiar' files
    */
   from_line = 1 ;
   from_char = 0 ;

   if ( oper & OPER_READ
   &&   ptr->flag & FLAG_PERSIST )
   {
      if ( rx_fseek( ptr->fileptr, ptr->readpos, SEEK_SET ) )
      {
         file_error( ptr, errno, NULL ) ;
         return 0;
      }
      ptr->thispos = ptr->readpos ;
   }
   if ( oper & OPER_WRITE
   &&   ptr->flag & FLAG_PERSIST )
   {
      if ( rx_fseek( ptr->fileptr, ptr->writepos, SEEK_SET ) )
      {
         file_error( ptr, errno, NULL ) ;
         return 0;
      }
      ptr->thispos = ptr->writepos ;
   }

   /*
    * First, let's check to see if we gain anything from using the
    * read position instead. If the distance from the current read
    * position to the wanted line (counted in number of lines) is smaller
    * than the number of lines from the first line to the wanted line,
    * use the current read position in stead. But only if the current
    * read position is defined.
    */
   if ((ptr->flag & FLAG_READ) && (ptr->readline > 0))
   {
      assert( ptr->readpos != (size_t) EOF) ;
      tmp = ptr->readline - lineno ;
      if (tmp<0)
         tmp = (-tmp) ;

      if (tmp < (lineno - from_line))
      {
         from_line = ptr->readline ;
         from_char = ptr->readpos ;
      }
   }

   /*
    * Then, we check to see whether we can gain even more if we use
    * the current write position of the file instead.
    */
   if ((ptr->flag & FLAG_WRITE) && (ptr->writeline > 0))
   {
      assert( ptr->writepos != (size_t) EOF ) ;
      tmp = ptr->writeline - lineno ;
      if (tmp<0)
         tmp = (-tmp) ;

      if (tmp < (lineno - from_line))
      {
         from_line = ptr->writeline ;
         from_char = ptr->writepos ;
      }
   }

   /*
    * By now, the variables from_line, and from_char should contain
    * the optimal starting point from where a seek for the 'lineno'
    * line in the file can start, so first, move there. An in addition,
    * it should be the known position which is closest to the wanted
    * line.
    */
   if (from_char != (long) ptr->thispos)
   {
      errno = 0 ;
      if ( ptr->flag & FLAG_PERSIST
      &&  rx_fseek( ptr->fileptr, from_char, SEEK_SET ))
      {
         file_error( ptr, errno, NULL ) ;
         return 0;
      }
      ptr->oper = OPER_NONE;
      ptr->thispos = from_char ;
   }
   assert( from_char == rx_ftell(ptr->fileptr) ) ;

   /*
    * Now we are positioned at the right spot, so seek forwards or
    * backwards until we reach the correct line. Actually, the method
    * we are going to use may seem a bit strange at first. First we
    * seek forward until we pass the line, and then we seek backwards
    * until we reach the line and at the end we back up to the first
    * preceding end-of-line marker. This may seem awkward, but it is
    * fairly simple. And in addition, it will always position us at
    * the _start_ of the wanted line.
    */
   once_more:
   while ((lineno>from_line))   /* seek forward */
   {
      SWITCH_OPER_READ(ptr);
      for (;((ch=getc(ptr->fileptr))!=EOF)&&(ch!=REGINA_EOL);from_char++) ;
      if (ch==REGINA_EOL)
         from_line++ ;
      else
         break ;
   }

   /*
    * Then we seek backwards until we reach the line. The backwards
    * movement is _really_ awkward, so perhaps we should read in 512
    * bytes, and analyse the data in it instead? Indeed, another
    * algoritm should be chosen. Maybe later ...
    */
   while (lineno<=from_line && from_char>0)
   {
      errno = 0 ;
      if ( ptr->flag & FLAG_PERSIST
      &&  rx_fseek(ptr->fileptr, -1, SEEK_CUR))
      {
         /*
          * Should this happen? Only if someone overwrites EOF chars in
          * the file, but that _may_ happend ...   Report error for
          * any errors from the fseek and ftell. If we hit the start of
          * the file, reset from_line check whether we are _below_ lineno
          * If so, jump back and seek from the start (then we *must*
          * start at line 1, since the data we've got are illegal).
          *
          * It will also happen if we are seeking backwards for the
          * first line.
          */
         errno = 0 ;
         if (rx_fseek(ptr->fileptr,0,SEEK_SET))
         {
            file_error( ptr, errno, NULL ) ;
            return 0;
         }
         ptr->oper = OPER_NONE;

         from_line = 1 ;
         ptr->thispos = 0 ;
         if (from_line<lineno)
         {
            ptr->readline = (-1);
            ptr->writeline = 0; /* unknown */
            goto once_more ;
         }

         break ;  /* we were looking for the first line ... how lucky :-) */
      }

      /*
       * After seeking one character backwards, we must read the character
       * that we just skipped over. Do that, and test whether it is
       * a end-of-line character.
       */
      SWITCH_OPER_READ(ptr);
      ch = getc(ptr->fileptr) ;
      if (ch==REGINA_EOL)
      {
         if (lineno==from_line)
            break ;

         from_line-- ;
      }

      /*
       * Then we move backwards once more, in order to compensate for
       * reading the character. Sigh, we are really doing a lot of
       * forward and backward reading, arn't we?
       */
      errno = 0 ;
      if ( ptr->flag & FLAG_PERSIST
      &&  rx_fseek(ptr->fileptr, -1, SEEK_CUR))
      {
         file_error( ptr, errno, NULL ) ;
         return 0;
      }
      ptr->oper = OPER_NONE;
   }

   /*
    * Now we are almost finished. We just have to set the correct
    * information in the Rexx file table entry.
    */
   ptr->thispos = rx_ftell( ptr->fileptr ) ;
   if (oper & OPER_READ)
   {
      ptr->readline = from_line ; /* was lineno */
      ptr->readpos = ptr->thispos ;
      ptr->flag &= ~(FLAG_RDEOF) ;
      ptr->flag &= ~(FLAG_AFTER_RDEOF) ;
   }
   if (oper & OPER_WRITE)
   {
      ptr->writeline = from_line ; /* was lineno */
      ptr->writepos = ptr->thispos ;
      ptr->flag &= ~(FLAG_WREOF) ;
   }

   if (oper & OPER_READ)
      ret = ptr->readline ;
   else
      ret = ptr->writeline ;
   return ret;
}

static rx_64 positionfile_SEEK_CUR( tsd_t *TSD, const char *bif, int argno, fileboxptr ptr, int oper, rx_64 lineno, rx_64 from_line, rx_64 from_char )
{
   rx_64 tmp,ret;

   /*
    * Do simple checks first.
    * If we are seeking back before the first line, then set:
    * READ:
    * ptr->readline = 1
    * ptr->readpos = 1
    * return 1
    * WRITE:
    * ptr->writeline = 1
    * ptr->writepos = 0
    * return 0
    */
   tmp = from_line + lineno;

   if ( tmp < 1 )
   {
      /*
       * We have positioned to before the first line
       *
       */
      rx_fseek( ptr->fileptr, 0L, SEEK_SET );
      ptr->thispos = rx_ftell( ptr->fileptr );
      if ( oper == OPER_READ )
      {
         ptr->readline = 1;
         ptr->readpos = 1;
         ptr->oper = OPER_READ;
         return 1;
      }
      else
      {
         ptr->writeline = 1;
         ptr->writepos = 0;
         ptr->oper = OPER_WRITE;
         return 0;
      }
   }
   /*
    * We now have an absolute line number from the +ve relative line
    * number, so use the absolute positioning code.
    */
   ret = positionfile_SEEK_SET( TSD, bif, argno, ptr, oper, tmp );
   return ret;
}

static rx_64 positionfile_SEEK_END( tsd_t *TSD, const char *bif, int argno, fileboxptr ptr, int oper, rx_64 lineno )
{
   /*
    * This function does file positioning on a line basis from the
    * end of the file.
    * There is not a lot of optimisation we can do reading backwards.
    * We first need to determine the last line; does it end in an EOL,
    * or is it incomplete; ie does not end in EOL. We treat an incomplete
    * last line as a line.
    * Our initial attempt at this will position the file at the
    * end, and read backwards; ie getc(ptr->fileptr), seek() back 2 chars,
    * until the start of file, or we have the specified number of lines.
    */
   rx_64 here, next, save_pos, i, ret, this_lineno, num_lines;
   size_t bret;
   char buf[512];
   int found = 0;

   SWITCH_OPER_READ(ptr);
   /*
    * First, get the size of the file. We can only do positioning on
    * persistant files...
    */
   if ( ! (ptr->flag & FLAG_PERSIST) )
   {
      file_error( ptr, 0, "Cannot position on transient stream" );
      return 0;
   }
   if ( rx_fseek( ptr->fileptr, 0L, SEEK_END ) )
   {
      file_error( ptr, errno, NULL ) ;
      return 0;
   }

   here = rx_ftell( ptr->fileptr );
   /*
    * Seek backwards one character and read the character. If the last
    * character is REGINA_EOL, then DON'T treat this as a new line.
    */
   if ( rx_fseek( ptr->fileptr, -1L, SEEK_CUR ) )
   {
      file_error( ptr, errno, NULL ) ;
      return 0;
   }
   buf[0] = (char)getc( ptr->fileptr );
   if ( buf[0] == REGINA_EOL )
      num_lines = 0;
   else
      num_lines = 1;
   /*
    * Move the file pointer back to after the last character
    */
   if ( rx_fseek( ptr->fileptr, 0L, SEEK_END ) )
   {
      file_error( ptr, errno, NULL ) ;
      return 0;
   }

   /*
    * We have to read backwards in the file until we reach a known point.
    * The only point which we can guarantee is known is the start of the
    * file. We can't use ptr->(read/write)line as we really don't know
    * how many lines are in the file (we may have appended several earlier).
    * An incomplete last line of a file (ie no CRLF/LF) is treated as a complete
    * line.
    */
   for ( ; ; )
   {
      /*
       * Determine where we want to move the file pointer backwards
       * into the file, and then move the file pointer there ready for
       * reading a buffer.
       */
      next = min( 512, here );
      if ( rx_fseek( ptr->fileptr, -next, SEEK_CUR ) )
      {
         file_error( ptr, errno, NULL ) ;
         return 0;
      }
      /*
       * Save our current position; start of the buffer being read
       */
      save_pos = rx_ftell( ptr->fileptr );
      /*
       * Read a buffer, this moves the file pointer forward to the
       * end of the buffer
       */

      bret = fread( buf, sizeof(char), next, ptr->fileptr );
      if ( bret != next
      &&   bret != EOF )
      {
         file_error( ptr, errno, NULL ) ;
         return 0;
      }
      /*
       * Count the number of lines from the end of the buffer
       */
      for ( i = next-1; i >= 0; i-- )
      {
         if ( buf[i] == REGINA_EOL )
         {
            num_lines++;
            if ( num_lines > lineno
            &&   !found )
            {
               /*
                * Calculate the actual char file position of the EOL
                */
               /*
                * the +1 is to point at the character AFTER the EOL; the
                * first character of the next line.
                */
               ptr->thispos = save_pos + i + 1;
               found = 1;
            }
         }
      }
      /*
       * move the file pointer back to the start of the buffer just
       * read, so the next fseek() backwards is from the START of the
       * buffer.
       */
      if ( rx_fseek( ptr->fileptr, save_pos, SEEK_SET ) )
      {
         file_error( ptr, errno, NULL ) ;
         return 0;
      }
      /*
       * Calculate our own file position
       */
      here -= next;
      if ( here == 0 )
         break;
   }
   /*
    * We are at the start of the file. If we haven't found our lineno
    * (because it was greater than the number of lines in the file),
    * that's where we stay.
    */
   if ( found )
   {
      /* ptr->thispos already set */
      this_lineno = 1 + (num_lines - lineno);
   }
   else
   {
      ptr->thispos = 0;
      this_lineno = 1;
   }

   /*
    * Now we are almost finished. We just have to set the correct
    * information in the Rexx file table entry.
    */
   if ( rx_fseek( ptr->fileptr, ptr->thispos, SEEK_SET ) )
   {
      file_error( ptr, errno, NULL ) ;
      return 0;
   }
   if ( oper & OPER_READ )
   {
      ptr->readline = this_lineno;
      ptr->readpos = ptr->thispos;
      ptr->flag &= ~(FLAG_RDEOF);
      ptr->flag &= ~(FLAG_AFTER_RDEOF);
   }
   if ( oper & OPER_WRITE )
   {
      ptr->writeline = this_lineno;
      ptr->writepos = ptr->thispos ;
      ptr->flag &= ~(FLAG_WREOF) ;
   }
   /*
    * We just counted the number of lines between the end of the file and
    * our current position, so we know how many lines are left.
    * The number of lines counted is one more than actually left in the file.
    */
   ptr->linesleft = num_lines-1;
   if ( oper & OPER_READ )
      ret = ptr->readline ;
   else
      ret = ptr->writeline ;
   return ret;
}

/*
 * This routine will position the current read or write position
 * of a file, to the start of a particular line. The file to be
 * operated on is 'ptr', the pointer to manipulate is indicated
 * by 'oper' (either OPER_READ or OPER_WRITE or both), and the linenumber
 * to position at is 'lineno'.
 * 'from' specifies if the positioning is done as an absolute position SEEK_SET,
 * a relative position from the current position SEEK_CUR, or relative to the
 * end of file: SEEK_END.
 *
 * There are (at least) two ways to do the backup of the current
 * position in the file. First to backup to the start of the file
 * and then to seek forward, or to seek backwards from the current
 * position of the file.
 *
 * Perhaps the first is best for the standard case, and the second
 * should be activated when the line-parameter is negative ... ?
 */

static rx_64 positionfile( tsd_t *TSD, const char *bif, int argno, fileboxptr ptr, int oper, rx_64 lineno, int from )
{
   rx_64 from_line=0;
   rx_64 from_char=0;
   rx_64 ret=0;

   /*
    * If file is in ERROR state, don't touch it.
    */
   if (ptr->flag & FLAG_ERROR)
   {
      if (!(ptr->flag & FLAG_FAKE))
         file_error( ptr, 0, NULL ) ;
      return 0;
   }

   /*
    * If this isn't a persistent file, then report an error. We can only
    * perform repositioning in persistent files.
    */

   if (!(ptr->flag & FLAG_PERSIST ))
      exiterror( ERR_INCORRECT_CALL, 42, bif, tmpstr_of( TSD, ptr->filename0 ) ) ;

   /*
    * If the operation is READ, but the file is not open for READ,
    * return an error.
    */
   if ((oper&OPER_READ) && !(ptr->flag & FLAG_READ))
      exiterror( ERR_INCORRECT_CALL, 921, bif, argno, "READ" ) ;
   /*
    * If the operation is WRITE, but the file is not open for WRITE,
    * return an error.
    */
   if ( (oper&OPER_WRITE) && !(ptr->flag & FLAG_WRITE) )
      exiterror( ERR_INCORRECT_CALL, 921, bif, argno, "WRITE" ) ;

   /*
    * If we do any repositioning, then make the old estimate of lines
    * left to read invalid. This is not really needed in all cases, but
    * it is a good start. And you _may_ even want to recalculate the
    * number of lines left!
    */
   if ( ptr->linesleft > 0 )
      ptr->linesleft = 0 ;

   if ( ptr->thispos == (size_t) EOF )
   {
      errno = 0 ;
      ptr->thispos = rx_ftell( ptr->fileptr ) ;
   }

   /*
    * So, what we are going to do depends partly on whether we are moving
    * the read or the write position of the file. We may even be as
    * lucky as not to have to move anything ... :-) First we can clear
    * the EOF flag, if set. Repositioning will clean up any EOF state.
    */
   if (oper & OPER_READ)
   {
      ptr->flag &= ~(FLAG_RDEOF) ;
      ptr->flag &= ~(FLAG_AFTER_RDEOF) ;
   }
   if (oper & OPER_WRITE)
      ptr->flag &= ~(FLAG_WREOF) ;

   /*
    * Positioning by line in a forwards direction is always going to be more efficient
    * starting at the current line position and reading forward.
    * Positioning by line in a backwards direction may be more efficient to start at the
    * beginning of the file and read forwards, rather than reading backwards; it
    * depends on how far back we are positioning.
    */
   switch( from )
   {
      case SEEK_CUR: /* position relative to current position */
         if ( oper & OPER_READ )
         {
            if ( ptr->readline > 0 )
            {
               from_line = ptr->readline ;
               from_char = ptr->readpos ;
               ret = positionfile_SEEK_CUR( TSD, bif, argno, ptr, OPER_READ, lineno, from_line, from_char );
            }
            else
            {
               /*
                * If the readpos is set, then we can (inefficiently) determine the line
                * position by starting at the beginning of the file.
                */
               if ( ptr->readpos != (size_t) -1 )
               {
                  /*
                   * FIXME: We need a mechanism to convert to readpos into a readline;
                   * positionfile_SEEK_SET() doesn't do it for us.
                   *
                   */
                  errno = 2;
                  ret = -1;
                  break;
               }
               else
               {
                  /*
                   * ERROR:2
                   * Can't seek lines relatively if there is no current read char position
                   */
                  errno = 2;
                  ret = -1;
                  break;
               }
            }
         }
         if ( oper & OPER_WRITE )
         {
            if ( ptr->writeline > 0 )
            {
               from_line = ptr->writeline ;
               from_char = ptr->writepos ;
               ret = positionfile_SEEK_CUR( TSD, bif, argno, ptr, OPER_WRITE, lineno, from_line, from_char );
            }
            else
            {
               /*
                * If the writepos is set, then we can (inefficiently) determine the line
                * position by starting at the beginning of the file.
                */
               if ( ptr->writepos != (size_t) -1 )
               {
                  /*
                   * FIXME: We need a mechanism to convert to writepos into a writeline;
                   * positionfile_SEEK_SET() doesn't do it for us.
                   *
                   */
                  errno = 2;
                  ret = -1;
                  break;
               }
               else
               {
                  /*
                   * ERROR:2
                   * Can't seek lines relatively if there is no current write char position
                   */
                  errno = 2;
                  ret = -1;
                  break;
               }
            }
         }
         /*
          * Now we are almost finished. We just have to set the correct
          * information in the Rexx file table entry.
          */
         if ( (oper & OPER_READ) && (oper & OPER_WRITE) )
         {
            ptr->oper = OPER_NONE;
         }
         if ( oper & OPER_READ )
         {
            ptr->flag &= ~(FLAG_RDEOF) ;
            ptr->flag &= ~(FLAG_AFTER_RDEOF) ;
         }
         if ( oper & OPER_WRITE )
         {
            ptr->flag &= ~(FLAG_WREOF) ;
         }
         break;
      case SEEK_END: /* position relative to end of file */
         ret = positionfile_SEEK_END( TSD, bif, argno, ptr, oper, lineno );
         break;
      case SEEK_SET: /* position absolute */
         ret = positionfile_SEEK_SET( TSD, bif, argno, ptr, oper, lineno );
      default: /* should not get here */
         break;
   }
   return ret;
}



/*
 * I wish every function would be as easy as this! Basically, it
 * only contain simple error checking, and a direct positioning.
 * it is called by the built-in function CHARIN() and CHAROUT()
 * in order to position the current read or write position at the
 * correct place in the file.
 */
static rx_64 positioncharfile( tsd_t *TSD, const char *bif, int argno, fileboxptr fileptr, int oper, rx_64 where, int from )
{
   rx_64 where_read=0,where_write=0;
   /*
    * If the file is in state ERROR, don't touch it! Since we are not
    * to return any data, don't bother about the state of FAKE.
    */
   if (fileptr->flag & FLAG_ERROR)
   {
      if (!(fileptr->flag & FLAG_FAKE))
         file_error( fileptr, 0, NULL ) ;
      return 0;
   }

   /*
    * If the file is not persistent, then positioning is not allowed.
    * Give the appropriate error for this.
    */
   if (!(fileptr->flag & FLAG_PERSIST))
      exiterror( ERR_INCORRECT_CALL, 42, bif, tmpstr_of( TSD, fileptr->filename0 ) ) ;
   /*
    * If the operation is READ, but the file is not open for READ,
    * return an error.
    */
   if ((oper&OPER_READ) && !(fileptr->flag & FLAG_READ))
      exiterror( ERR_INCORRECT_CALL, 921, bif, argno, "READ" ) ;
   /*
    * If the operation is WRITE, but the file is not open for WRITE,
    * return an error.
    */
   if ((oper&OPER_WRITE) && !(fileptr->flag & FLAG_WRITE))
      exiterror( ERR_INCORRECT_CALL, 921, bif, argno, "WRITE" ) ;

#ifdef TRUE_TRL_IO
   /*
    * TRL says that positioning the read position to after the last
    * character in the file, is an error. Unix allows it, and gives
    * an EOF at the next reading. So, we have to handle this as a
    * special case ...  Check that the new position is valid.
    *
    * Should we give "Incorrect call to routine" when the character
    * position is greater than the size of the file? Perhaps we should
    * raise the NOTREADY condition instead?
    */
   {
      rx_64 oldp, endp ;

      oldp = rx_ftell( fileptr->fileptr ) ;
      rx_fseek(fileptr->fileptr, 0, SEEK_END) ;
      endp = rx_ftell( fileptr->fileptr ) ;
      rx_fseek( fileptr->fileptr, oldp, SEEK_SET ) ;
      fileptr->oper = OPER_NONE;

      /*
       * Determine the value of "where" depending on the starting
       * location determined by "from". "where" is passed in in an
       * external format; ie 1 based, internally it must be 0 based
       */
      switch(from)
      {
         case SEEK_CUR:
            if ( oper & OPER_READ )
               where_read = 1 + where + fileptr->readpos;
            if ( oper & OPER_WRITE )
               where_write = 1 + where + fileptr->writepos;
            break;
         case SEEK_END:
            if ( oper & OPER_READ )
               where_read = endp - where;
#if SEEK_TO_EOF_FOR_WRITE_IS_AT_EOF
            if ( oper & OPER_WRITE )
               where_write = endp - where;
#else
            if ( oper & OPER_WRITE )
               where_write = 1 + endp - where;
#endif
            break;
         default:  /* SEEK_SET */
            if ( oper & OPER_READ )
               where_read = where;
            if ( oper & OPER_WRITE )
               where_write = where;
            break;
      }
      if ( oper & OPER_READ )
      {
         if ( where_read < 1 )
         {
            file_error( fileptr, 0, "Repositioning before start of file" ) ;
            return 0;
         }
         if ( endp < where_read )
         {
            file_error( fileptr, 0, "Repositioning at or after EOF" ) ;
            return 0;
         }
      }
      if ( oper & OPER_WRITE )
      {
         if ( where_write < 1 )
         {
            file_error( fileptr, 0, "Repositioning before start of file" ) ;
            return 0;
         }
         if ( (endp+1) < where_write )
         {
            file_error( fileptr, 0, "Repositioning after EOF" ) ;
            return 0;
         }
      }
   }
#endif

   /*
    * Then do the actual positioning. Remember to clear errno first.
    * Previously, this code tested afterwards to see if ftell()
    * returned the same position that fseek() tried to set. Surely, that
    * must be unnecessary?
    * We need to reposition using both the read and write postions (if
    * required).
    */
   errno = 0 ;
   /*
    * Position the real file pointer to the write or read pointers
    * calculated. The "thispos" member is set to the last seek
    * executed. READ is done last as this is probably the most
    * likely use of character positioning, hence it may be slightly
    * more efficient.
    */
   if ( oper & OPER_WRITE )
   {
      if ( rx_fseek(fileptr->fileptr,(where_write-1),SEEK_SET ) )
      {
         file_error( fileptr, errno, NULL ) ;
         return 0;
      }
      fileptr->thispos = where_write ; /* this was where-1; is that correct ?*/
   }
   if ( oper & OPER_READ )
   {
      if ( rx_fseek(fileptr->fileptr,(where_read-1),SEEK_SET) )
      {
         file_error( fileptr, errno, NULL ) ;
         return 0;
      }
      fileptr->thispos = where_read ; /* this was where-1; is that correct ?*/
   }
   fileptr->oper = OPER_NONE;

   /*
    * Then we have to update the file pointers in the entry in our
    * file table.
    *
    * Clear the end-of-file flag. Even if we *did* position to the
    * end of file, we don't want to discover that until we actually
    * _read_ data that is _off_ the end-of-file.
    */

   if (oper & OPER_READ)
   {
      fileptr->readpos = where_read-1 ;
      fileptr->flag &= ~(FLAG_RDEOF) ;
      fileptr->flag &= ~(FLAG_AFTER_RDEOF) ;
   }
   if (oper & OPER_WRITE)
   {
      fileptr->writepos = where_write-1 ;
      fileptr->flag &= ~(FLAG_WREOF) ;
   }
   if (oper == OPER_NONE)
      file_error( fileptr, 0, NULL ) ;

   /*
    * We have moved the file pointer by a number of characters which
    * may have spanned any number of lines. So we have no idea which
    * line we are in the file, so we need to invalidate the
    * read or write line position.
    */
   if (oper & OPER_READ)
      fileptr->readline = 0 ;
   if (oper & OPER_WRITE)
      fileptr->writeline = 0 ;

   /*
    * Return the new position of the file pointer.  If both file
    * pointers were set, then readpos and writepos are the same, so
    * the following test is valid.
    */
   if (oper & OPER_READ)
      return fileptr->readpos + 1; /* external representation */
   else
      return fileptr->writepos + 1; /* external representation */
}



/*
 * This routine reads a string of data from a file indicated by
 * the Rexx file table entry 'ptr'. The read starts at the current
 * read position, and the length will be 'length' characters.
 *
 * Then, what if the data to be read are more than what is possible
 * to store in one string; let's say length=100,000, and the size of
 * length in a string is 16 bit. Well, That should return an error
 * in Str_makeTSD(), but maybe we should handle it more elegantly?
 *
 * No file_error() is thrown if noerrors is set.
 */
static streng *readbytes( tsd_t *TSD, fileboxptr fileptr, size_t length, int noerrors )
{
   size_t didread=0 ;
   streng *retvalue=NULL ;

   /*
    * If state is ERROR, then refuse to handle the file further.
    * If the state was 'only' EOF, then don't bother, the length of
    * the file might have increased since last attempt to read.
    */
   if (fileptr->flag & FLAG_ERROR)
   {
      if (!noerrors && !(fileptr->flag & FLAG_FAKE))
         file_error( fileptr, 0, NULL ) ;
      return nullstringptr() ;
   }

   assert( fileptr->flag & FLAG_READ ) ;

   /*
    * If we are not at the current read position, we have to
    * seek to the correct position, but first we have to the validity
    * of these positions.
    */
   if (fileptr->flag & FLAG_PERSIST)
   {
      if (fileptr->thispos != fileptr->readpos)
      {
         errno = 0 ;
         if ( fileptr->flag & FLAG_PERSIST
         &&  rx_fseek(fileptr->fileptr, fileptr->readpos, SEEK_SET ))
         {
            if (!noerrors)
               file_error( fileptr, errno, NULL ) ;
            return nullstringptr() ;
         }
         fileptr->thispos = fileptr->readpos ;
         fileptr->oper = OPER_NONE ;
      }
   }

   /*
    * The joy of POSIX ...  If a file is open for input and output, it
    * must be flushed when changing between the two. Therefore, check
    * the type of the last operation. Actually, this are not very likely
    * since that situation would in general have been handled above.
    */
   if (fileptr->oper==OPER_WRITE)
   {
      errno = 0 ;
      if ( fileptr->flag & FLAG_PERSIST
      &&  rx_fseek( fileptr->fileptr, 0L, SEEK_CUR ))
      {
         /* Hey, how could this have happened?!?! NFS down? */
         if (!noerrors)
            file_error( fileptr, errno, NULL ) ;
         return nullstringptr() ;
      }
      fileptr->oper = OPER_NONE ;
   }

   /*
    * Lets get ready for the big event. First allocate enough space to
    * hold the data we are hoping to be able to read. Then read it
    * directly into the string.
    */
   retvalue = Str_makeTSD(length+1) ;
   errno = 0 ;
   didread = fread( retvalue->value, 1, length, fileptr->fileptr ) ;
   fileptr->oper = OPER_READ;

   /*
    * Variable 'read' contains the number of items (=bytes) read, or
    * it contains EOF if an error occurred. Handle the error the
    * normal way; i.e. trigger file_error and return nothing.
    */
   if (didread==EOF)
   {
      if (!noerrors)
         file_error( fileptr, errno, NULL ) ;
      return nullstringptr() ;
   }

   /*
    * What if we didn't manage to read all the data? Well, return what
    * we got, but still trigger an error, since EOF should be
    * considered a NOTREADY condition. However, we try to handle EOF
    * a bit more elegantly than other errors, since lots of programmers
    * are probably not bothering about EOF; an EOF condition should be
    * able to be reset using a file positioning.
    */
   assert( 0<=didread && didread<=length ) ;   /* It'd better be! */
   retvalue->len = didread ;
   if (didread<length)
   {
      if (!noerrors)
         file_warning( fileptr, 0, "EOF on char input" ) ;
      fileptr->flag |= FLAG_RDEOF ;
   }
   else
   {
      fileptr->flag &= ~FLAG_RDEOF ;
      fileptr->flag &= ~FLAG_AFTER_RDEOF ;
   }

   /*
    * Then, at the end, we have to set the pointers and counter to
    * the correct values
    */
   fileptr->thispos += didread ;
   fileptr->readpos += didread ;
   fileptr->readline = (-1) ;
   fileptr->linesleft = 0 ;

   return retvalue ;
}



/*
 * This routines write a string to a file pointed to by the Rexx file
 * table entry 'fileptr'. The string to be written is 'string', and the
 * length of the write is implicitly given as the length of 'string'
 *
 * This routine is called from the Rexx built-in function CHAROUT().
 * It is a fairly straight forward implementation.
 *
 * No file_error() is thrown if noerrors is set.
 */
static size_t writebytes( tsd_t *TSD, fileboxptr fileptr, const streng *string, int noerrors )
{
#ifdef WIN32
   int rc=0;
#endif
   long todo, done, written=0 ;
   const char *buf ;

   /*
    * First, if this file is in state ERROR, don't touch it, what to
    * return depends on whether the file is in state FAKE.
    */
   if ( fileptr->flag & FLAG_ERROR )
   {
      if ( fileptr->flag & FLAG_FAKE )
         return string->len ;
      else
      {
         if (!noerrors)
            file_error( fileptr, 0, NULL ) ;
         if (fileptr->flag & FLAG_FAKE)
            return string->len ;

         return 0 ;
      }
   }
   /*
    * If we are not at the current write position, we have to
    * seek to the correct position
    */
   if (fileptr->thispos != fileptr->writepos)
   {
      errno = 0 ;
      if ( fileptr->flag & FLAG_PERSIST
      &&  rx_fseek(fileptr->fileptr, fileptr->writepos, SEEK_SET ))
      {
         if (!noerrors)
            file_error( fileptr, errno, NULL ) ;
         return 0 ;
      }
      fileptr->thispos = fileptr->writepos ;
      fileptr->oper = OPER_NONE ;
   }

   /*
    * If previous operation on this file was a read, we have to flush
    * the file before we can perform any write operations. This will
    * seldom happen, since it is in general handled above.
    */
   if (fileptr->oper == OPER_READ)
   {
      errno = 0 ;
      if ( fileptr->flag & FLAG_PERSIST
      &&  rx_fseek(fileptr->fileptr, 0, SEEK_CUR))
      {
         if (!noerrors)
            file_error( fileptr, errno, NULL ) ;
         return (fileptr->flag & FLAG_FAKE) ? string->len : 0 ;
      }
      fileptr->oper = OPER_NONE ;
   }

   /*
    * Here comes the actual writing. This also works when the length
    * of string is zero.
    */
   errno = 0 ;
   buf = string->value ;
   todo = string->len ;
   fileptr->oper = OPER_WRITE ;
   do
   {
#ifdef WIN32
      done = fwrite( buf, 1, todo, fileptr->fileptr ) ;
#else
      done = fwrite( buf, 1, todo, fileptr->fileptr ) ;
#endif
      /*
       * Win32 has a bug with fwrite and disk full. If the size of the
       * chunk to write is < 4096 and the disk fills up, then you don't get
       * an error indication. So flush the stream if the size of data is
       * < 4096 and test the result of fflush(). Bug 731664
       */
#ifdef WIN32
      if (string->len < 4096 )
         rc = fflush( fileptr->fileptr );
      if (done < 0 || rc != 0 )
#else
      if (done < 0)
#endif
      {
         written = -1 ;
         break ;
      } else if (done == 0)
         break;
      else
         written += done ;
      buf += done ;
      todo -= done ;
   } while ( ( todo > 0 ) && noerrors ) ;

   /*
    * Here comes the error checking. Note that this function will
    * return the number of elements written, it will never return
    * EOF as fread can, since the problems surrounding EOF can not
    * occur in this operation. Therefore, report a fullfleged error
    * whenever rc is less than the length of string.
    */
   assert( 0<=written && written<=string->len ) ;
   if (written < string->len )
   {
      if (!noerrors)
         file_error( fileptr, errno, NULL ) ;
   }
   else
   {
      /*
       * If the operation was successful, then we set misc status
       * information about the file, and the counters and pointers.
       */
      fileptr->writeline = 0 ;
      fileptr->flag &= ~FLAG_RDEOF ;
      fileptr->flag &= ~FLAG_AFTER_RDEOF ;
      fileptr->thispos += written ;
      fileptr->writepos += written ;

      fflush( fileptr->fileptr ) ;
      fileptr->oper = OPER_NONE;
   }

   return written ;
}


/*
 * This routine calculates the number of bytes remaining in the file,
 * i.e the number of bytes from the current read position until the
 * end-of-file.  It is, of course, called from the Rexx built-in
 * function CHARS()
 */

static rx_64 calc_chars_left( tsd_t *TSD, fileboxptr ptr )
{
   rx_64 left=0 ;
   rx_64 oldpoint=0L, newpoint=0L ;

   if (!(ptr->flag & FLAG_READ))
      return 0 ;

   /*
    * First, determine whether this file is in ERROR state. If so, we
    * don't want to touch it. Whether or not the file is in FAKE state
    * is fairly irrelevant in this situation
    */
   if ( ptr->flag & FLAG_ERROR )
   {
      if (!(ptr->flag & FLAG_FAKE))
         file_error( ptr, 0, NULL ) ;
      return 0 ;
   }

   /*
    * If this is not a persistent file, then we have no means of finding
    * out how much of the file is available. Then, return 1 if we are not
    * at the end-of-file, and 0 otherwise.
    */
   if (!(ptr->flag & FLAG_PERSIST))
   {
#if 0
      left = ( !(ptr->flag & FLAG_RDEOF)) ;
#else
      struct rx_stat_buf finfo;
      int fno;

      fno = fileno( ptr->fileptr ) ;
      rx_fstat( fno, &finfo );
      left = finfo.st_size;
#endif
   }
   else
   {
      /*
       * This is a persistent file, which is not in error state. OK, then
       * we must record the current point, fseek to the end-of-file,
       * ftell to get that position, and fseek back to where we started.
       * And we have to check for errors everywhere ... sigh.
       *
       * First, record the current position in the file.
       */
      errno = 0 ;
      oldpoint = rx_ftell( ptr->fileptr ) ;
      if (oldpoint==EOF)
      {
         file_error( ptr, errno, NULL ) ;
         return 0 ;
      }

      /*
       * Then, move the current position to the end-of-file
       */
      errno = 0 ;
      if (rx_fseek(ptr->fileptr, 0L, SEEK_END))
      {
         file_error( ptr, errno, NULL ) ;
         return 0 ;
      }
      ptr->oper = OPER_NONE;

      /*
       * And record the position of the end-of-file
       */
      errno = 0 ;
      newpoint = rx_ftell( ptr->fileptr ) ;
      if (newpoint==EOF)
      {
         file_error( ptr, errno, NULL ) ;
         return 0 ;
      }

      /*
       * And, at last, position back to the place where we started.
       * Actually, this may not be necessary, since we _can_ leave the
       * current position at the end-of-file. After all, the next read
       * or write _will_ position back correctly. However, let's be
       * nice ...
       */
      errno = 0 ;
      if (rx_fseek(ptr->fileptr, oldpoint, SEEK_SET))
      {
         file_error( ptr, errno, NULL ) ;
         return 0 ;
      }

      /*
       * Then we have some accounting to do; calculate the size of the
       * last part of the file. And also set oper to NONE, we _have_
       * done a repositioning ... actually, several :-)
       */
      left = newpoint - ptr->readpos ;
/*      left = newpoint - oldpoint ; */ /* YURI - wrong */
      ptr->oper = OPER_NONE ;
   }

   return left ;
}


/*
 * This routine counts the complete lines remaining in the file
 * pointed to by the Rexx file table entry 'ptr'. The count starts
 * at the current read or write position, and the current line will be counted
 * even if the current read position points to the middle of a line.
 * The last line will only be counted if it was actually terminated
 * by a EOL marker. If the current line is the last line, but it was
 * not explicitly terminated by a EOL marker, zero is returned.
 */
static rx_64 countlines( tsd_t *TSD, fileboxptr ptr, int actual, int oper )
{
   long oldpoint=0L ;
   int left=0, ch=0;
   int prevch=-1 ;

   /*
    * If this file is in ERROR state, we really don't want to try to
    * operate on it. Just report an error, and return 0.
    */
   if ( ptr->flag & FLAG_ERROR )
   {
      if (!(ptr->flag & FLAG_FAKE))
         file_error( ptr, 0, NULL ) ;
      return 0 ;
   }

   /*
    * Counting lines requires us to reposition in the file. However,
    * we can not reposition in transient files. If this is not a
    * persistent file, don't do any repositioning, just return one
    * for any situation where we are not sure whether there are more
    * data or not (i.e. unless we are sure that there are no more data,
    * return "1"
    */
   if (!(ptr->flag & FLAG_PERSIST))
      return (!feof(ptr->fileptr)) ;
   else if (!actual)
   {
      /*
       * We just want to know of there are any lines left for a persistent
       * file.  First check if we have reached EOF. Return 0 if so.
       * Next check if the file size is 0; if so then return 0, otherwise
       * there is at least 1 line left; return 1
       */
      if ( feof(ptr->fileptr ) )
         return 0;
      else
      {
         struct rx_stat_buf buffer ;
         int fno;
         memset( &buffer, 0, sizeof(buffer) );
         fno = fileno( ptr->fileptr ) ;
         rx_fstat( fno, &buffer ) ;
         return ((buffer.st_size==0) ? 0 : 1);
      }
   }
   else
   {
      /*
       * Take advantage of the cached value of the lines left in the
       * file
       */
      if (ptr->linesleft)
         return ptr->linesleft ;

      /*
       * If, however, this is a persistent file, we have to read from
       * the current read position to the end-of-file, and count all
       * the lines. First, make sure that we position at the current
       * read position.
       */
      errno = 0 ;
      oldpoint = rx_ftell( ptr->fileptr ) ;
      if (oldpoint==EOF)
      {
         file_error( ptr, errno, NULL ) ;
         return 0 ;
      }

      /*
       * Then read the rest of the file, and keep a count of all the files
       * read in the process.
       */
      SWITCH_OPER_READ(ptr);
      /*
       * Switch to the current read or write pos setting
       */
      if ( oper == OPER_READ )
         ptr->thispos = ptr->readpos;
      else
         ptr->thispos = ptr->writepos;
      rx_fseek(ptr->fileptr,ptr->thispos,SEEK_SET);
#if defined(UNIX) || defined(MAC)
      for(left=0;((ch=getc(ptr->fileptr))!=EOF);)
      {
         if (ch==REGINA_EOL)
            left++ ;
         prevch = ch;
      }
      if (prevch != REGINA_EOL
      &&  prevch != -1)
         left++;
#else
      for(left=0;;)
      {
         ch = getc(ptr->fileptr);
         if (ch == EOF)
            break;
         if ( ch == REGINA_CR)
            left++ ;
         else
         {
            if ( ch == REGINA_EOL && prevch != REGINA_CR)
               left++ ;
         }
         prevch = ch;
      }
      if (prevch != REGINA_EOL
      &&  prevch != REGINA_CR
      &&  prevch != -1)
         left++;
#endif

      /*
       * At the end, try to reposition back to the old current read
       * position, and report an error if that attempt failed.
       */
      errno = 0 ;
      if ( ptr->flag & FLAG_PERSIST
      &&  rx_fseek(ptr->fileptr, oldpoint, SEEK_SET))
      {
         file_error( ptr, errno, NULL ) ;
         return 0 ;
      }
      ptr->oper = OPER_NONE;
      ptr->linesleft = left ;
   }
   return left ;
}



/*
 * This routine writes a line to the file indicated by 'ptr'. The line
 * to be written is 'data', and it will be terminated by an extra
 * EOL marker after the charactrers in 'data'.
 */
static int writeoneline( tsd_t *TSD, fileboxptr ptr, const streng *data )
{
   const char *i=NULL ;
   int num_eol_chars=0;

   /*
    * First, make sure that the file is not in ERROR state. If it is
    * report an error, and return a result depending on whether this
    * file is to be faked.
    */
   if (ptr->flag & FLAG_ERROR)
   {
      if (ptr->flag & FLAG_FAKE)
         return 0 ;
      else
      {
         file_error( ptr, 0, NULL ) ;
         if (ptr->flag & FLAG_FAKE)
            return 0 ;
         return 1 ;
      }
   }

   /*
    * If we are to write a new line, we ought to truncate the file after
    * that line. Or rather, we truncate the file at the start of the
    * new line, before we write it out. But only if we have the non-POSIX
    * function ftruncate() available. And not if we are already there.
    */
#if defined(HAVE_FTRUNCATE)
   if ( get_options_flag( TSD->currlevel, EXT_LINEOUTTRUNC ) )
   {
      if (ptr->oper != OPER_WRITE && !(ptr->flag & (FLAG_WREOF)) &&
                                     (ptr->flag & FLAG_PERSIST))
      {
        int fno ;
        errno = 0 ;
        SWITCH_OPER_WRITE(ptr); /* Maybe, ftruncate is a write operation in
                                 * the meaning of POSIX. This shouldn't do
                                 * any harm in other systems.
                                 */

        fno = fileno( ptr->fileptr ) ;
        if (ftruncate( fno, ptr->writepos) == -1)
        {
           file_error( ptr, errno, NULL ) ;
           return !(ptr->flag & FLAG_FAKE) ;
        }
        if ( ptr->flag & FLAG_PERSIST )
           rx_fseek( ptr->fileptr, 0, SEEK_END ) ;
        ptr->oper = OPER_NONE;
        ptr->thispos = ptr->writepos = rx_ftell( ptr->fileptr ) ;
        if (ptr->readpos>ptr->thispos && ptr->readpos!= (size_t) EOF)
        {
           ptr->readpos = ptr->thispos ;
           ptr->readline = 0 ;
           ptr->linesleft = 0 ;
        }
      }
   }
#endif

   /*
    * Then, output the characters in 'data', and sense any problem.
    * If there is a problem, report an error
    */
   errno = 0 ;
   SWITCH_OPER_WRITE(ptr);
   for (i=data->value; i<Str_end(data); i++)
   {
      if (putc( *i, ptr->fileptr)==EOF)
      {
         file_error( ptr, errno, NULL ) ;
         return 1 ;
      }
   }

   /*
    * After all the data has been written out, we have to explicitly
    * terminate the file with an end-of-line marker. Under Unix this
    * is the single character EOL. Under Macintosh this is the single
    * character CR, and all others it is CR and EOL.
    */
#if !defined(UNIX)
   SWITCH_OPER_WRITE(ptr);
   if (putc( REGINA_CR, ptr->fileptr)==EOF)
   {
      file_error( ptr, errno, NULL ) ;
      return 1 ;
   }
   num_eol_chars++;
#endif
#if !defined(MAC)
   SWITCH_OPER_WRITE(ptr);
   if (putc( REGINA_EOL, ptr->fileptr)==EOF)
   {
      file_error( ptr, errno, NULL ) ;
      return 1 ;
   }
   num_eol_chars++;
#endif

   /*
    * Then we have to update the counters and pointers in the Rexx
    * file table entry. We must do that in order to be able to keep
    * track of where we are.
    */
   ptr->thispos += data->len + num_eol_chars ; /* fix 736578 */
   ptr->writepos = ptr->thispos ;
   ptr->oper = OPER_WRITE ;

   /*
    * FIXME - under what circumstances will writeline be 0 ?
    * If it hasn't been determined by what calls this function, the
    * a) the calling function(s) should determine the line number, or
    * b) the current line number should be determined here.
    */
   if (ptr->writeline)
      ptr->writeline++ ;

   ptr->flag |= FLAG_WREOF ;

   /*
    * At the end, we flush the data. We do this in order to avoid
    * surprises later. Maybe we shouldn't do that, since it may force
    * a systemcall, which might give away the timeslice and decrease
    * system time. So you might want to remove this call ... at your
    * own risk :-)
    */
   errno = 0 ;
   if (fflush( ptr->fileptr ))
   {
      file_error( ptr, errno, NULL ) ;
      return 1 ;
   }

   return 0 ;
}

/*
 * This routine is a way of retrieving the information returned by the
 * standard Unix call stat(). It takes the name of a file as parameter,
 * and return information about that file. This is not standard Rexx,
 * but quite useful. It is accessed through the built-in function
 * STREAM(), command 'FSTAT'
 * This is now also used for the "standard" STREAM() options.
 * *Persistent will be set to 1 if the stream's type is a file. The setting
 * happens on success and if Persistent isn't NULL.
 */
static streng *getstatus( tsd_t *TSD, const streng *filename , int subcommand )
{
   fileboxptr ptr=NULL ;
   int rc=0 ;
   int fno=0 ;
   rx_64 pos_read = -2L, pos_write = -2L, line_read = -2L, line_write = -2;
   int streamtype = STREAMTYPE_UNKNOWN;
   streng *result=NULL ;
   struct rx_stat_buf buffer ;
   struct tm tmdata, *tmptr ;
   char *fn=NULL;
#if 0
   static const char *fmt = "%02d-%02d-%02d %02d:%02d:%02d" ;
   static const char *iso = "%04d-%02d-%02d %02d:%02d:%02d" ;
#endif
   static const char *streamdesc[] = { "UNKNOWN", "PERSISTENT", "TRANSIENT" };
   char tmppwd[50];
   char tmpgrp[50];
   char *ptmppwd=tmppwd,*ptmpgrp=tmpgrp;
#if !(defined(VMS) || defined(MAC) || defined(OS2) || defined(DOS) || (defined (__WATCOMC__) && !defined(__QNX__)) || defined(_MSC_VER) || (defined(WIN32) && defined(__IBMC__)) || defined(__MINGW32__) || defined(__BORLANDC__) || defined(__EPOC32__) || defined(__LCC__))
   struct passwd *ppwd;
   struct group *pgrp;
#endif

   memset( &buffer, 0, sizeof(buffer) );
   /*
    * Nul terminate the input filename string, as stat() will barf if
    * it isn't and other functions stuff up!
    */
   fn = str_ofTSD(filename);
   /*
    * First get the Rexx file table entry associated with the file,
    * and then call stat() for that file. If the file is already open,
    * then call fstat, since that will in general be a 'safer' way
    * to be sure that it is _really_ the file that is open in Rexx.
    */
   ptr = getfileptr( TSD, filename ) ;
   if (ptr && ptr->fileptr)
   {
      fno = fileno( ptr->fileptr ) ;
      rc = rx_fstat( fno, &buffer ) ;
      if (ptr->flag & FLAG_PERSIST)
         streamtype = STREAMTYPE_PERSISTENT;
      else
         streamtype = STREAMTYPE_TRANSIENT;
      pos_read = ptr->readpos;
      pos_write = ptr->writepos;
      line_read = ptr->readline;
      line_write = ptr->writeline;
   }
   else
   {
      /*
       * To be consistent with other functions when determining persistence,
       * we need to check for a "regular" file. Everything other than a
       * "regular" file in transient.
       * If we don't have S_ISREG macro, then revert to a simple check; if the
       * stream is a directory it is transent; ugly!
       */
      rc = my_rx_stat( TSD, fn, &buffer ) ;
      if ( rc != 0 )
         streamtype = STREAMTYPE_UNKNOWN;
      else
      {
         /*
          * Resolves 802114.
          */
         streamtype = stream_types[determine_stream_type( buffer.st_mode )].streamtype;
      }
   }

   /*
    * If we were able to retrieve any useful information, store it
    * in a string of suitable length, and return that string.
    * If the filename does not exist, always return an empty string.
    */
   if ( rc == -1 )
   {
      if ( fn )
         FreeTSD( fn );
      checkProperStreamName( TSD, NULL, (const char *) tmpstr_of( TSD, filename ), errno );
      return nullstringptr();
   }
   switch ( subcommand )
   {
      case COMMAND_FSTAT: /* return information about symbolic link if a symbolic link */
#ifdef HAVE_LSTAT
         /*
          * If we have lstat(), use it to gather details, this is the only
          * way to determine if the file is a symlink.
          */
         lstat(fn, &buffer) ;
#endif
#if defined(VMS) || defined(MAC) || defined(OS2) || defined(DOS) || (defined (__WATCOMC__) && !defined(__QNX__)) || defined(_MSC_VER) || (defined(WIN32) && defined(__IBMC__)) || defined(__MINGW32__) || defined(__BORLANDC__) || defined(__EPOC32__) || defined(__LCC__)
         ptmppwd = "USER";
         ptmpgrp = "GROUP";
#else
         ppwd = getpwuid( buffer.st_uid );
         if ( ppwd )
            ptmppwd = ppwd->pw_name;
         else
            sprintf( tmppwd, "%d", buffer.st_uid );

         pgrp = getgrgid( buffer.st_gid );
         if ( pgrp )
            ptmpgrp = pgrp->gr_name;
         else
            sprintf( tmpgrp, "%d", buffer.st_gid );
#endif
         result = Str_makeTSD( 128 ) ;
#if defined(HAVE_I64U)
         sprintf( result->value,
            "%ld %ld %03o %d %s %s %I64u",
            (long)(buffer.st_dev), (long)(buffer.st_ino),
            buffer.st_mode & ACCESSPERMS, buffer.st_nlink,
            ptmppwd, ptmpgrp,
            buffer.st_size ) ;
#else
         if ( sizeof(rx_64) > 4 )
            sprintf( result->value,
               "%ld %ld %03o %d %s %s %lld",
               (long)(buffer.st_dev), (long)(buffer.st_ino),
               buffer.st_mode & ACCESSPERMS, buffer.st_nlink,
               ptmppwd, ptmpgrp,
               (rx_64)(buffer.st_size) ) ;
         else
            sprintf( result->value,
               "%ld %ld %03o %d %s %s %ld",
               (long)(buffer.st_dev), (long)(buffer.st_ino),
               buffer.st_mode & ACCESSPERMS, buffer.st_nlink,
               ptmppwd, ptmpgrp,
               (long)(buffer.st_size) ) ;
#endif
         /*
          * Append the stream type name...
          */
         strcat( result->value, stream_types[determine_stream_type( buffer.st_mode )].streamname );
         break;
      case COMMAND_QUERY_EXISTS:
         if ( streamtype == STREAMTYPE_TRANSIENT )
         {
            result = nullstringptr();
         }
         else
         {
            result = Str_makeTSD( REXX_PATH_MAX );
            my_fullpath( TSD, result->value, fn );
            result->len = strlen( result->value );
         }
         break;
      case COMMAND_QUERY_SIZE:
         if ( streamtype == STREAMTYPE_TRANSIENT )
         {
            result = nullstringptr() ;
         }
         else
         {
            result = Str_makeTSD( 50 ) ;
#if defined(HAVE_I64U)
            sprintf( result->value, "%I64u", buffer.st_size ) ;
#else
            if ( sizeof(rx_64) > 4 )
               sprintf( result->value, "%lld", (rx_64)(buffer.st_size) ) ;
            else
               sprintf( result->value, "%ld", (long)(buffer.st_size) ) ;
#endif
         }
         break;
      case COMMAND_QUERY_HANDLE:
         if (fno)
         {
            result = Str_makeTSD( 10 ) ;
            sprintf( result->value, "%d", fno ) ;
         }
         else
            result = nullstringptr() ;
         break;
      case COMMAND_QUERY_STREAMTYPE:
         result = Str_makeTSD( 12 ) ;
         sprintf( result->value, "%s", streamdesc[streamtype] ) ;
         break;
      case COMMAND_QUERY_DATETIME:
         if ( streamtype == STREAMTYPE_TRANSIENT )
         {
            result = nullstringptr() ;
         }
         else
         {
            time_t num64;
            num64 = buffer.st_mtime;
            if ( ( tmptr = localtime( &num64 ) ) != NULL )
               tmdata = *tmptr;
            else
               memset(&tmdata,0,sizeof(tmdata)); /* what shall we do in this case? */
            result = Str_makeTSD( 20 ) ;
#if 0
            sprintf( result->value, fmt, tmdata.tm_mon+1, tmdata.tm_mday,
                  (tmdata.tm_year % 100), tmdata.tm_hour, tmdata.tm_min,
                  tmdata.tm_sec ) ;
#else
            strftime( result->value, 20, "%m-%d-%y %H:%M:%S", &tmdata );
#endif
         }
         break;
      case COMMAND_QUERY_TIMESTAMP:
         if ( streamtype == STREAMTYPE_TRANSIENT )
         {
            result = nullstringptr() ;
         }
         else
         {
            time_t num64;
            num64 = buffer.st_mtime;
            if ( ( tmptr = localtime( &num64 ) ) != NULL )
               tmdata = *tmptr;
            else
               memset( &tmdata, 0, sizeof(tmdata) ); /* what shall we do in this case? */
            result = Str_makeTSD( 20 ) ;
#if 0
            sprintf( result->value, iso, tmdata.tm_year+1900, tmdata.tm_mon+1,
                  tmdata.tm_mday,
                  tmdata.tm_hour, tmdata.tm_min,
                  tmdata.tm_sec ) ;
#else
            strftime( result->value, 20, "%Y-%m-%d %H:%M:%S", &tmdata );
#endif
         }
         break;
      case COMMAND_QUERY_CREATETIME:
         if ( streamtype == STREAMTYPE_TRANSIENT )
         {
            result = nullstringptr() ;
         }
         else
         {
            result = Str_makeTSD( 20 ) ;
#if defined(HAVE_I64U)
            sprintf( result->value, "%I64u", buffer.st_ctime );
#else
            if ( sizeof(off_t) > 4 )
               sprintf( result->value, "%lld", buffer.st_ctime );
            else
               sprintf( result->value, "%ld", buffer.st_ctime);
#endif
         }
         break;
      case COMMAND_QUERY_MODIFYTIME:
         if ( streamtype == STREAMTYPE_TRANSIENT )
         {
            result = nullstringptr() ;
         }
         else
         {
            result = Str_makeTSD( 20 ) ;
#if defined(HAVE_I64U)
            sprintf( result->value, "%I64u", buffer.st_mtime );
#else
            if ( sizeof(off_t) > 4 )
               sprintf( result->value, "%lld", buffer.st_mtime );
            else
               sprintf( result->value, "%ld", buffer.st_mtime);
#endif
         }
         break;
      case COMMAND_QUERY_ACCESSTIME:
         if ( streamtype == STREAMTYPE_TRANSIENT )
         {
            result = nullstringptr() ;
         }
         else
         {
            result = Str_makeTSD( 20 ) ;
#if defined(HAVE_I64U)
            sprintf( result->value, "%I64u", buffer.st_atime );
#else
            if ( sizeof(off_t) > 4 )
               sprintf( result->value, "%lld", buffer.st_atime );
            else
               sprintf( result->value, "%ld", buffer.st_atime);
#endif
         }
         break;
      case COMMAND_QUERY_POSITION_READ_CHAR:
      case COMMAND_QUERY_POSITION_SYS:
         if (pos_read != (-2))
         {
            result = Str_makeTSD( 50 ) ;
#if defined(HAVE_I64U)
            sprintf( result->value, "%I64u", pos_read + 1 );
#else
            if ( sizeof(off_t) > 4 )
               sprintf( result->value, "%lld", pos_read + 1 );
            else
               sprintf( result->value, "%ld", (long)(pos_read + 1) );
#endif
         }
         else
            result = nullstringptr() ;
         break;
      case COMMAND_QUERY_POSITION_WRITE_CHAR:
         if (pos_write != (-2))
         {
            result = Str_makeTSD( 50 ) ;
#if defined(HAVE_I64U)
            sprintf( result->value, "%I64u", pos_write + 1 );
#else
            if ( sizeof(off_t) > 4 )
               sprintf( result->value, "%lld", pos_write + 1 );
            else
               sprintf( result->value, "%ld", (long)(pos_write + 1) );
#endif
         }
         else
            result = nullstringptr() ;
         break;
      case COMMAND_QUERY_POSITION_READ_LINE:
         if (line_read != (-2))
         {
            result = Str_makeTSD( 50 ) ;
#if defined(HAVE_I64U)
            sprintf( result->value, "%I64u", line_read );
#else
            if ( sizeof(off_t) > 4 )
               sprintf( result->value, "%lld", line_read );
            else
               sprintf( result->value, "%ld", (long)line_read );
#endif
         }
         else
            result = nullstringptr() ;
         break;
      case COMMAND_QUERY_POSITION_WRITE_LINE:
         if ( line_write == 0 )
         {
            rx_64 here;
            rx_64 char_count;
            int ch;
            /*
             * When a file is first opened for both read and
             * write (default for implicit open), it is inexpensive
             * to determine pos_read, pos_write and line_read, but
             * is very expensive to determine line_write, so we
             * don't do it. It is set to 0 indicating that we don't
             * know the current write position. So to reduce the
             * cost when we may never use it, we have to pay the
             * price the first time we need the value; this is
             * one of the times we pay the price!
             */
            result = Str_makeTSD( 50 ) ;
            /*
             * We can't use countlines(), so do our our counting
             * of lines form the beginning of the file to the current
             * write pos...
             */
            here = rx_ftell( ptr->fileptr );
            rx_fseek( ptr->fileptr, 0L, SEEK_SET );
            SWITCH_OPER_READ(ptr);
            for( char_count = 0, line_write = 0; char_count < (long) ptr->writepos; char_count++ )
            {
               ch = getc( ptr->fileptr );
               if ( ch == EOF )
                  break;
               if ( ch == REGINA_EOL )
                  line_write++;
            }
#if defined(HAVE_I64U)
            sprintf( result->value, "%I64u", line_write + 1 );
#else
            if ( sizeof(off_t) > 4 )
               sprintf( result->value, "%lld", line_write + 1 ) ;
            else
               sprintf( result->value, "%ld", (long)(line_write + 1) ) ;
#endif
            rx_fseek( ptr->fileptr, here, SEEK_SET );
         }
         else if (line_write != (-2))
         {
            result = Str_makeTSD( 50 ) ;
#if defined(HAVE_I64U)
            sprintf( result->value, "%I64u", line_write );
#else
            if ( sizeof(off_t) > 4 )
               sprintf( result->value, "%lld", line_write ) ;
            else
               sprintf( result->value, "%ld", (long)line_write ) ;
#endif
         }
         else
            result = nullstringptr() ;
         break;
   }
   result->len = strlen( result->value ) ;

   if ( fn )
      FreeTSD( fn );
   return result ;
}


/*
 * This little sweet routine returns information stored in the Rexx
 * file table entry about the named file 'filename'. It is perhaps more
 * of a debugging function than a Rexx function. It is accessed by the
 * Rexx built-in function STREAM(), command 'STATUS'. One of the nice
 * pieces of information this function returns is whether a file is
 * transient or persistent.
 *
 * This is really a simple function, just retrieve the Rexx file
 * table entry, and store the information in that entry into a string
 * and return that string.
 *
 * The difference between getrexxstatus() and getstatus() is that
 * that former returns information stored in Rexx's datastructures,
 * while the latter return information about the file stored in and
 * managed by the operating system
 */
static streng *getrexxstatus( const tsd_t *TSD, cfileboxptr ptr )
{
   streng *result=NULL ;

   if (ptr==NULL)
      return nullstringptr() ;

   result = Str_makeTSD(64) ;  /* Ought to be enough */
   result->value[0] = 0x00 ;

   if ((ptr->flag & FLAG_READ) && (ptr->flag & FLAG_WRITE))
      strcat( result->value, "READ/WRITE" ) ;
   else if (ptr->flag & FLAG_READ)
      strcat( result->value, "READ" ) ;
   else if (ptr->flag & FLAG_WRITE)
      strcat( result->value, "WRITE" ) ;
   else
      strcat( result->value, "NONE" ) ;

#if defined(HAVE_I64U)
   sprintf( result->value + strlen(result->value),
            " READ: char=%I64u line=%I64u WRITE: char=%I64u line=%I64u %s",
            (ptr->readpos+1), ptr->readline,
            (ptr->writepos+1), ptr->writeline,
            (ptr->flag & FLAG_PERSIST) ? "PERSISTENT" : "TRANSIENT" ) ;
#else
   if ( sizeof(rx_64) > 4 )
      sprintf( result->value + strlen(result->value),
            " READ: char=%lld line=%lld WRITE: char=%lld line=%lld %s",
            (ptr->readpos+1), ptr->readline,
            (ptr->writepos+1), ptr->writeline,
            (ptr->flag & FLAG_PERSIST) ? "PERSISTENT" : "TRANSIENT" ) ;
   else
      sprintf( result->value + strlen(result->value),
            " READ: char=%ld line=%ld WRITE: char=%ld line=%ld %s",
            (long)(ptr->readpos+1), (long)ptr->readline,
            (long)(ptr->writepos+1), (long)ptr->writeline,
            (ptr->flag & FLAG_PERSIST) ? "PERSISTENT" : "TRANSIENT" ) ;
#endif

   result->len = strlen(result->value) ;
   return result ;
}


/*
 * This routine parses the remainder of the parameters passed to the
 * Stream(,'C','QUERY...') function.
 */
static streng *getquery( tsd_t *TSD, const streng *filename , const streng *subcommand)
{
   streng *result=NULL, *psub=NULL, *psubsub=NULL ;
   char oper = 0;
   char seek_oper = 0;

   /*
    * Get the subcommand to QUERY
    */
   oper = get_querycommand( subcommand );
   switch ( oper )
   {
      case COMMAND_QUERY_DATETIME :
      case COMMAND_QUERY_TIMESTAMP :
      case COMMAND_QUERY_CREATETIME :
      case COMMAND_QUERY_MODIFYTIME :
      case COMMAND_QUERY_ACCESSTIME :
      case COMMAND_QUERY_EXISTS :
      case COMMAND_QUERY_HANDLE :
      case COMMAND_QUERY_SIZE :
      case COMMAND_QUERY_STREAMTYPE :
         result = getstatus( TSD, filename, oper );
         break;
      case COMMAND_QUERY_SEEK :
      case COMMAND_QUERY_POSITION :
         if ( oper == COMMAND_QUERY_SEEK )
         {
            psub = Str_nodupTSD( subcommand, 4, subcommand->len - 4 );
            seek_oper = 1;
         }
         else
         {
            psub = Str_nodupTSD( subcommand, 8, subcommand->len - 8 );
            seek_oper = 0;
         }
         psub = Str_strp( psub, ' ', STRIP_LEADING);
         oper = get_querypositioncommand( psub );
         switch ( oper )
         {
            case COMMAND_QUERY_POSITION_SYS :
               result = getstatus(TSD, filename, oper );
               break;
            case COMMAND_QUERY_POSITION_READ :
               psubsub = Str_nodupTSD( psub, 4, psub->len - 4 );
               psubsub = Str_strp( psubsub, ' ', STRIP_LEADING);
               oper = get_querypositionreadcommand( psubsub );
               switch( oper )
               {
                  case COMMAND_QUERY_POSITION_READ_CHAR:
                  case COMMAND_QUERY_POSITION_READ_LINE:
                     result = getstatus( TSD, filename, oper );
                     break;
                  default:
                     exiterror( ERR_STREAM_COMMAND, 1, (seek_oper)?"QUERY SEEK READ":"QUERY POSITION READ", "CHAR LINE ''", tmpstr_of( TSD, psubsub ) )  ;
                     break;
               }
               break;
            case COMMAND_QUERY_POSITION_WRITE :
               psubsub = Str_nodupTSD( psub, 5, psub->len - 5 );
               psubsub = Str_strp( psubsub, ' ', STRIP_LEADING);
               oper = get_querypositionwritecommand( psubsub );
               switch( oper )
               {
                  case COMMAND_QUERY_POSITION_WRITE_CHAR:
                  case COMMAND_QUERY_POSITION_WRITE_LINE:
                     result = getstatus( TSD, filename, oper );
                     break;
                  default:
                     exiterror( ERR_STREAM_COMMAND, 1, (seek_oper)?"QUERY SEEK WRITE":"QUERY POSITION WRITE", "CHAR LINE ''", tmpstr_of( TSD, psubsub ) )  ;
                     break;
               }
               break;
            default:
               exiterror( ERR_STREAM_COMMAND, 1, (seek_oper)?"QUERY SEEK":"QUERY POSITION", "READ WRITE SYS", tmpstr_of( TSD, psub ) )  ;
               break;
         }
         Free_stringTSD(psub);
         break;
      default:
         exiterror( ERR_STREAM_COMMAND, 1, "QUERY", "DATETIME TIMESTAMP CREATETIME MODIFYTIME ACCESSTIME EXISTS HANDLE SIZE STREAMTYPE SEEK POSITION", tmpstr_of( TSD, subcommand ) )  ;
         break;
   }

   return result ;
}

/*
 * This routine parses the remainder of the parameters passed to the
 * Stream(,'C','OPEN...') function.
 */
static streng *getopen( tsd_t *TSD, const streng *filename , const streng *subcommand)
{
   fileboxptr ptr=NULL ;
   streng *result=NULL, *psub=NULL ;
   char oper = 0;
   char buf[20];

   /*
    * Get the subcommand to OPEN
    */
   oper = get_opencommand( subcommand );
   switch ( oper )
   {
      case COMMAND_OPEN_BOTH :
         if ( subcommand->len >= 4
         &&   memcmp(subcommand->value, "BOTH", 4) == 0 )
            psub = Str_nodupTSD( subcommand, 4, subcommand->len - 4 );
         else
            psub = Str_dupTSD( subcommand );
         psub = Str_strp( psub, ' ', STRIP_LEADING);
         oper = get_opencommandboth( psub );
         if ( TSD->restricted )
            exiterror( ERR_RESTRICTED, 4 )  ;
         switch ( oper )
         {
            case COMMAND_OPEN_BOTH :
               closefile( TSD, filename ) ;
               ptr = openfile( TSD, filename, ACCESS_WRITE ) ;
               break;
            case COMMAND_OPEN_BOTH_APPEND :
               closefile( TSD, filename ) ;
               ptr = openfile( TSD, filename, ACCESS_STREAM_APPEND ) ;
               break;
            case COMMAND_OPEN_BOTH_REPLACE :
               closefile( TSD, filename ) ;
               ptr = openfile( TSD, filename, ACCESS_STREAM_REPLACE ) ;
               break;
            default:
               exiterror( ERR_STREAM_COMMAND, 1, "OPEN BOTH", "APPEND REPLACE ''", tmpstr_of( TSD, psub ) )  ;
               break;
         }
         Free_stringTSD(psub);
         if (ptr->fileptr)
            result = Str_creTSD( "READY:" ) ;
         else
         {
            sprintf(buf,"ERROR:%d",errno);
            result = Str_creTSD( buf ) ;
         }
         break;
      case COMMAND_OPEN_READ :
         closefile( TSD, filename ) ;
         ptr = openfile( TSD, filename, ACCESS_READ ) ;
         if (ptr->fileptr)
            result = Str_creTSD( "READY:" ) ;
         else
         {
            sprintf(buf,"ERROR:%d",errno);
            result = Str_creTSD( buf ) ;
         }
         break;
      case COMMAND_OPEN_WRITE :
         if ( TSD->restricted )
            exiterror( ERR_RESTRICTED, 4 )  ;
         psub = Str_nodupTSD( subcommand, 5, subcommand->len - 5 );
         psub = Str_strp( psub, ' ', STRIP_LEADING);
         oper = get_opencommandwrite( psub );

         switch ( oper )
         {
            case COMMAND_OPEN_WRITE :
               closefile( TSD, filename ) ;
               ptr = openfile( TSD, filename, ACCESS_WRITE ) ;
               break;
            case COMMAND_OPEN_WRITE_APPEND :
               closefile( TSD, filename ) ;
               ptr = openfile( TSD, filename, ACCESS_STREAM_APPEND ) ;
               break;
            case COMMAND_OPEN_WRITE_REPLACE :
               closefile( TSD, filename ) ;
               ptr = openfile( TSD, filename, ACCESS_STREAM_REPLACE ) ;
               break;
            default:
               exiterror( ERR_STREAM_COMMAND, 1, "OPEN WRITE", "APPEND REPLACE ''", tmpstr_of( TSD, psub ) )  ;
               break;
         }
         Free_stringTSD(psub);
         if (ptr->fileptr)
            result = Str_creTSD( "READY:" ) ;
         else
         {
            sprintf(buf,"ERROR:%d",errno);
            result = Str_creTSD( buf ) ;
         }
         break;
      default:
         exiterror( ERR_STREAM_COMMAND, 1, "OPEN", "BOTH READ WRITE ''", tmpstr_of( TSD, subcommand ) )  ;
         break;
   }

   return result ;
}


static streng *getseek( tsd_t *TSD, const streng *filename, const streng *cmd )
{
#define STATE_START 0
#define STATE_WORD  1
#define STATE_DELIM 2
   char *word[5] = {NULL,NULL,NULL,NULL};
   char *str;
   char *offset=NULL;
   int i,j=0;
   int state=STATE_START;
   int seek_by_line=0;
   int seek_type=0;
   int seek_sign=0;
   rx_64 seek_offset=0,pos=0;
   int pos_type=OPER_NONE,num_params=0;
   int str_start=0,str_end=(-1),words;
   fileboxptr ptr;
   streng *result=NULL;
   char buf[20];

   str = str_ofTSD(cmd);
   words = 4;
   for (i=0;i<Str_len(cmd);i++)
   {
      switch(state)
      {
         case STATE_START:
            if (*(str+i) == ' ')
            {
               state = STATE_DELIM;
               break;
            }
            if ( j < 3 )
               word[j] = str+str_start;
            j++;
            if (str_end != (-1))
            {
               *(str+str_end) = '\0';
            }
            state = STATE_WORD;
            break;
         case STATE_WORD:
            if (*(str+i) == ' ')
            {
               state = STATE_DELIM;
               str_end = i;
               str_start = str_end + 1;
               break;
            }
            break;
         case STATE_DELIM:
            state = STATE_WORD;
            if (*(str+i) == ' ')
            {
               state = STATE_DELIM;
            }
            if (state == STATE_WORD)
            {
               if ( j < 3 )
                  word[j] = str+str_start;
               j++;
               if (str_end != (-1))
               {
                  *(str+str_end) = '\0';
               }
            }
            break;
      }
   }
   num_params = j;
   if (num_params < 1)
      exiterror( ERR_INCORRECT_CALL, 922, "STREAM", 3, 2, num_params+1 );
   if (num_params > 3)
      exiterror( ERR_INCORRECT_CALL, 923, "STREAM", 3, 4, num_params+1 );

   switch( num_params )
   {
      case 3:
         if (strcmp(word[2],"CHAR") == 0)
            seek_by_line = 0;
         else
         {
            if (strcmp(word[2],"LINE") == 0)
               seek_by_line = 1;
            else
               exiterror( ERR_INCORRECT_CALL, 924, "STREAM", 3, "CHAR LINE", word[2] );
         }
      /* meant to fall through */
      case 2:
         /*
          * 2 params(to SEEK), last one (word[1]) could be READ/WRITE or CHAR/LINE
          */
         if (strcmp(word[1],"READ") == 0)
            pos_type = OPER_READ;
         else if (strcmp(word[1],"WRITE") == 0)
            pos_type = OPER_WRITE;
         else if (strcmp(word[1],"CHAR") == 0)
            seek_by_line = 0;
         else if (strcmp(word[1],"LINE") == 0)
            seek_by_line = 1;
         else
            exiterror( ERR_INCORRECT_CALL, 924, "STREAM", 3, "READ WRITE CHAR LINE", word[1] );
   }
   /*
    * Determine the position type if not supplied prior
    */
   if ( pos_type == OPER_NONE )
   {
      ptr = getfileptr( TSD, filename ) ;
      if ( ptr != NULL )
      {
         if ( ptr->flag & FLAG_READ )
            pos_type |= OPER_READ;
         if ( ptr->flag & FLAG_WRITE )
            pos_type |= OPER_WRITE;
      }
   }
   offset = word[0];
   switch(*offset)
   {
      case '=':
         seek_type = SEEK_SET;
         offset++;
         break;
      case '-':
         seek_type = SEEK_CUR;
         seek_sign = 1;
         offset++;
         break;
      case '+':
         seek_type = SEEK_CUR;
         seek_sign = 0;
         offset++;
         break;
      case '<':
         seek_type = SEEK_END;
         offset++;
         break;
      default:
         seek_type = SEEK_SET;
         break;
   }
   for (i=0;i<(int)strlen(offset);i++)
   {
      if (!rx_isdigit(*(offset+i)))
         exiterror( ERR_INCORRECT_CALL, 924, "STREAM", 3, "n, +n, -n, =n or <n", word[0] );
   }
#if defined(HAVE_ATOLL)
   seek_offset = atoll(offset);
#elif defined(HAVE__ATOI64)
   seek_offset = _atoi64(offset);
#else
   seek_offset = atol(offset);
#endif
   if (seek_sign) /* negative */
      seek_offset *= -1;
   ptr = get_file_ptr( TSD, filename, pos_type, (pos_type&OPER_WRITE) ? ACCESS_WRITE : ACCESS_READ ) ;
   if (!ptr)
   {
      sprintf(buf,"ERROR:%d",errno);
      result = Str_creTSD( buf ) ;
   }
   if (seek_by_line) /* position by line */
      pos = positionfile( TSD, "STREAM", 3, ptr, pos_type, seek_offset, seek_type ) ;
   else
      pos = positioncharfile( TSD, "STREAM", 3, ptr, pos_type, seek_offset, seek_type ) ;
   if ( pos >= 0 )
   {
      result = Str_makeTSD( 20 ) ; /* should be enough digits */
#if defined(HAVE_I64U)
      sprintf(result->value, "%I64u", pos );
#else
      if ( sizeof(rx_64) > 4 )
         sprintf(result->value, "%lld", pos );
      else
         sprintf(result->value, "%ld", (long)pos );
#endif
      Str_len( result ) = strlen( result->value );
   }
   else
   {
      sprintf(buf,"ERROR:%d",errno);
      result = Str_creTSD( buf ) ;
   }
   FreeTSD(str);
   return result ;
}



/* ------------------------------------------------------------------- */
/*                   Rexx builtin functions (level 3)                  */
/*
 * This part consists of one function for each of the Rexx builtin
 * functions that operates on filesystem I/O
 */


/*
 * This routine implements the Rexx built-in function CHARS(). It is
 * really quite simple, little more than a wrap-around to the
 * function calc_chars_left.
 */
streng *std_chars( tsd_t *TSD, cparamboxptr parms )
{
   char opt = 'N';
   streng *string=NULL ;
   fileboxptr ptr=NULL ;
   int was_closed=0;
   rx_64 result=0 ;
   fil_tsd_t *ft;

   ft = (fil_tsd_t *)TSD->fil_tsd;

   /* Syntax: chars([filename]) */
   checkparam(  parms,  0,  2 , "CHARS" ) ;

   if (parms&&parms->next&&parms->next->value)
      opt = getoptionchar( TSD, parms->next->value, "CHARS", 2, "CN", "" ) ;

   string = (parms->value && parms->value->len) ? parms->value : ft->stdio_ptr[0]->filename0 ;
   /*
    * Get a pointer to the Rexx file table entry of the file, and
    * calculate the number of characters left.
    */
   ptr = getfileptr( TSD, string ) ;
   was_closed = (ptr==NULL) ;
   if (!ptr)
      ptr = get_file_ptr( TSD, string, OPER_READ, ACCESS_READ ) ;

   result = calc_chars_left( TSD, ptr ) ;
   if (was_closed)
      closefile( TSD, string ) ;

   return rx64_to_streng( TSD, result ) ;
}



/*
 * Implements the Rexx builtin function charin(). This function takes
 * three parameters, and they are treated pretty straight forward
 * according to TRL. If called with no start position, and a length of
 * zero, it may be used to do some fancy work (flushing I/O?), although
 * that is probably more needed for output :-)  Note that the file in
 * entered into the file table in this case, so it might be used to
 * explicitly open a file for reading. However, consider using stream()
 * to do this, it's a much cleaner approach!
 */
streng *std_charin( tsd_t *TSD, cparamboxptr parms )
{
   streng *filename=NULL, *result=NULL ;
   fileboxptr ptr=NULL ;
   size_t length=0 ;
   rx_64 start=0 ;
   fil_tsd_t *ft;

   ft = (fil_tsd_t *)TSD->fil_tsd;

   /* Syntax: charin([filename][,[start][,length]]) */
   checkparam(  parms,  0,  3 , "CHARIN" ) ;

   /*
    * First, let's get the information about the file from the
    * file table, and open it in the correct mode if is not already
    * availble as such.
    */
   filename = (parms->value && parms->value->len) ? (parms->value) : ft->stdio_ptr[0]->filename0 ;
   ptr = get_file_ptr( TSD, filename, OPER_READ, ACCESS_READ ) ;

   /*
    * Then, get the starting point, or set it to zero.
    */
   parms = parms->next ;
   if ((parms)&&(parms->value))
      start = atoposrx64( TSD, parms->value, "CHARIN", 2 ) ;
   else
      start = 0 ;

   /*
    * At last, get the length, or use the default value one.
    */
   if (parms)
      parms = parms->next ;

   if ((parms)&&(parms->value))
      length = atozpos( TSD, parms->value, "CHARIN", 3 ) ;
   else
      length = 1 ;

   /*
    * Position current position in file if necessary
    */
   if (start)
      positioncharfile( TSD, "CHARIN", 2, ptr, OPER_READ, start, SEEK_SET ) ;

   if (length)
      result = readbytes( TSD, ptr, length, 0 ) ;
   else
   {
      if (!start)
         flush_input( ptr )  ;  /* Whatever happens ... */
      result = nullstringptr() ;
   }

   return result ;
}



/*
 * This function implements the Rexx built-in function CHAROUT(). It
 * is basically a wrap-around for the two functions that perform
 * character repositioning in a file; and writes out characters.
 */

streng *std_charout( tsd_t *TSD, cparamboxptr parms )
{
   streng *filename=NULL, *string=NULL ;
   int length=0 ;
   long pos=0 ;
   fileboxptr ptr=NULL ;
   fil_tsd_t *ft;

   ft = (fil_tsd_t *)TSD->fil_tsd;

   if ( TSD->restricted )
      exiterror( ERR_RESTRICTED, 1, "CHAROUT" )  ;

   /* Syntax: charout([filename][,[string][,start]]) */
   checkparam(  parms,  0,  3 , "CHAROUT" ) ;

   filename = (parms->value && parms->value->len) ? (parms->value) : ft->stdio_ptr[1]->filename0 ;

   /* Read the data to be written, if any */
   parms = parms->next ;
   if (parms && parms->value )
      string = parms->value ;
   else
      string = NULL ;

   /* Read the position to start writing, is any */
   if (parms)
      parms = parms->next ;

   if ( parms && parms->value )
      pos = atopos( TSD, parms->value, "CHAROUT", 3 ) ;
   else
      pos = 0 ;

   ptr = get_file_ptr( TSD, filename, OPER_WRITE, ACCESS_WRITE ) ;

   /*
    * If we are to position the write position somewhere, do that first.
    */
   if (pos)
      positioncharfile( TSD, "CHAROUT", 3, ptr, OPER_WRITE, pos, SEEK_SET ) ;

   /*
    * Then, write the actual data, or flush output if neither data nor
    * position was given.
    */
   if (string)
      length = string->len - writebytes( TSD, ptr, string, 0 ) ;
   else
   {
      length = 0;
      if ( !pos )
      {
         /*
          * flush_output() will swap out the file and close it, but leave ALL positions
          * intact
          * We need to set the write positions to end of file (NOT EOF)
          * See ANSI 9.7.2, 9.7.5, A.5.8.9
          * For efficiency sake, we will have to set writeline = 0 :-(
          * We do this BEFORE flush_output() otherwise we won't have a ptr->fileptr!
          */
         if ( ptr->flag & FLAG_PERSIST )
         {
            rx_fseek( ptr->fileptr, 0, SEEK_END ) ;
            ptr->writepos = rx_ftell( ptr->fileptr ) ;
         }
         else
            ptr->writepos = 0;
         ptr->writeline = 0;

         if ( flush_output( TSD, ptr ) == -1 )
            length = 1; /* simulate (at least) 1 byte not written */
      }
   }

   return int_to_streng( TSD, length ) ;
}



/*
 * Simple routine that implements the Rexx built-in function LINES().
 * Really just a wrap-around to the countlines() routine.
 */

streng *std_lines( tsd_t *TSD, cparamboxptr parms )
{
   char opt = 'N';
   fileboxptr ptr=NULL ;
   streng *filename=NULL ;
   int was_closed=0;
   rx_64 result=0 ;
   int actual;
   fil_tsd_t *ft;

   ft = (fil_tsd_t *)TSD->fil_tsd;

   /* Syntax: lines([filename][,C|N]) */
   checkparam(  parms,  0,  2 , "LINES" ) ;

   if (parms&&parms->next&&parms->next->value)
      opt = getoptionchar( TSD, parms->next->value, "LINES", 2, "CN", "" ) ;

   /*
    * Get the name of the file (use defaults if necessary), and get
    * a pointer to the entry of that file from the file table
    */
   if (parms->value
   &&  parms->value->len)
      filename = parms->value ;
   else
      filename = ft->stdio_ptr[0]->filename0 ;

   /*
    * Try to get the Rexx file table entry, if it doesn't work, then
    * try again ... and a bit harder
    */
   ptr = getfileptr( TSD, filename ) ;
   was_closed = (ptr==NULL) ;
   if (!ptr)
      ptr = get_file_ptr( TSD, filename, OPER_READ, ACCESS_READ ) ;

   /*
    * If 'C' is passed, we ALWAYS get the actual number of lines
    * If 'N' is passed (or unset), we get the actual number of lines
    * unless FAST_LINES_BIF is set (default is set)
    * Bug: 1000227
    */
   if ( opt == 'C' )
      actual = 1;
   else
   {
      if ( get_options_flag( TSD->currlevel, EXT_FAST_LINES_BIF_DEFAULT ) )
         actual = 0;
      else
         actual = 1;
   }
   result = countlines( TSD, ptr, actual, OPER_READ ) ;

   if (was_closed)
      closefile( TSD, filename ) ;


   return rx64_to_streng( TSD, result ) ;
}



/*
 * The Rexx built-in function LINEIN() reads a line from a file.
 * The actual reading is performed in 'readoneline', while this routine
 * takes care of range checking of parameters, and decides which
 * lower level routines to call.
 */

streng *std_linein( tsd_t *TSD, cparamboxptr parms )
{
   streng *filename=NULL, *res=NULL ;
   fileboxptr ptr=NULL ;
   int count=0, line=0 ;
   fil_tsd_t *ft;

   ft = (fil_tsd_t *)TSD->fil_tsd;

   /* Syntax: linein([filename][,[line][,count]]) */
   checkparam(  parms,  0,  3 , "LINEIN" ) ;

   /*
    * First get the name of the file, or use the appropriate default
    */
   if (parms->value
   &&  parms->value->len)
      filename = parms->value ;
   else
      filename = ft->stdio_ptr[0]->filename0 ;

   /*
    * Then get the line number at which the read it to start, or set
    * set it to zero if none was specified.
    */
   if (parms)
      parms = parms->next ;

   if (parms && parms->value)
      line = atopos( TSD, parms->value, "LINEIN", 2 ) ;
   else
      line = 0 ;  /* Illegal value */

   /*
    * And at last, read the count, which can be only 0 or 1, and which
    * is the number of lines to read.
    */
   if (parms)
      parms = parms->next ;

   if (parms && parms->value)
   {
      count = atozpos( TSD, parms->value, "LINEIN", 3 ) ;
      if (count!=0 && count!=1)
         exiterror( ERR_INCORRECT_CALL, 39, "LINEIN", tmpstr_of( TSD, parms->value ) )  ;
   }
   else
      count = 1 ;  /* The default */

   /*
    * Now, get the pointer to the entry in the file table that contains
    * information about this file, or make it automatically create
    * an entry if one didn't exist.
    */
   ptr = get_file_ptr( TSD, filename, OPER_READ, ACCESS_READ ) ;

   /*
    * If line was specified, we must reposition the current read
    * position of the file.
    */
   if (line)
      positionfile( TSD, "LINEIN", 2, ptr, OPER_READ, line, SEEK_SET ) ;

   /*
    * As the last thing, read in the data. If no data was wanted, skip it
    * but call flushing if line wasn't specified either.
    */
   if (count)
      res = readoneline( TSD, ptr ) ;
   else
   {
      if (!line)
         flush_input( ptr ) ;
      res = nullstringptr() ;
   }

   return res ;
}




/*
 * This function is a wrap-around for the Rexx built-in function
 * LINEOUT(). It performs parameter checking and decides which lower
 * level routines to call.
 */

streng *std_lineout( tsd_t *TSD, cparamboxptr parms )
{
   streng *string=NULL, *file=NULL ;
   int lineno=0, result=0 ;
   fileboxptr ptr=NULL ;
   fil_tsd_t *ft;

   ft = (fil_tsd_t *)TSD->fil_tsd;

   if ( TSD->restricted )
      exiterror( ERR_RESTRICTED, 1, "LINEOUT" )  ;

   /* Syntax: lineout([filename][,[string][,line]]) */
   checkparam(  parms,  0,  3 , "LINEOUT" ) ;

   /*
    * First get the pointer for the file to operate on. If omitted,
    * use the standard output stream
    */
   if (parms->value
   &&  parms->value->len)
      file = parms->value ;
   else
      file = ft->stdio_ptr[1]->filename0 ;
   /*
    * The file pointer is needed in ALL circumstances!
    */
   ptr = get_file_ptr( TSD, file, OPER_WRITE, ACCESS_WRITE ) ;

   /*
    * Then, get the data to be written, if any.
    */
   if (parms)
      parms = parms->next ;

   if (parms && parms->value)
      string = parms->value ;
   else
      string = NULL ;

   /*
    * At last, we must find the line number of the file to write. We
    * must position the file at this line before the write.
    */
   if (parms)
      parms = parms->next ;

   if (parms && parms->value)
      lineno = atopos( TSD, parms->value, "LINEOUT", 3 ) ;
   else
      lineno = 0 ;  /* illegal value */

   /*
    * First, let's reposition the file if necessary.
    */
   if (lineno)
      positionfile( TSD, "LINEOUT", 2, ptr, OPER_WRITE, lineno, SEEK_SET ) ;

   /*
    * And then, we write out the data. If there are not data, it may have
    * been just positioning. However, if there are neither data nor
    * a linenumber, something magic may happen.
    */
   if (string)
      result = writeoneline( TSD, ptr, string ) ;
   else
   {
      if ( !lineno )
      {
         /*
          * flush_output() will swap out the file and close it, but leave ALL positions
          * intact
          * We need to set the write positions to end of file (NOT EOF)
          * See ANSI 9.7.2, 9.7.5, A.5.8.9
          * For efficiency sake, we will have to set writeline = 0 :-(
          * We do this BEFORE flush_output() otherwise we won't have a ptr->fileptr!
          */
         if ( ptr->flag & FLAG_PERSIST )
         {
            /*
             * When using LINEOUT() to close a file and that file is NOT writeable, ptr->fileptr
             * gets set to NULL, so guard against this
             */
            if ( ptr->fileptr )
            {
               rx_fseek( ptr->fileptr, 0, SEEK_END ) ;
               ptr->writepos = rx_ftell( ptr->fileptr ) ;
            }
         }
         else
            ptr->writepos = 0;
         ptr->writeline = 0;
         /*
          * ANSI states that a file is not necessarily closed in this case.
          * Position of file pointers is explicitly stated in ANSI and if the
          * file is NOT closed they cause breakage.
          * Therefore implement ANSI in STRICT_ANSI mode and normal behaviour
          * (that does not cause breakage) in "regina" mode.
          * MH 22/06/2004 - after non-conclusive discussions on ANSI mailing list
          */
         if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
         {
            flush_output( TSD, ptr );
         }
         else
         {
            closefile( TSD, file ) ;
         }
      }
      result = 0;
   }

   return int_to_streng( TSD, result ) ;
}




/*
 * This function checks whether a particular file is accessable by
 * the user in a certain mode, which may be read, write or execute.
 * Unfortunately, this function differs a bit from the functionality
 * of several others. It explicitly checks a file, so that if the
 * file didn't exist in advance, it is _not_ opened. And even _if_
 * the file existed, the file in the file system is checked, not the
 * file opened by Regina. The two may differ slightly under certain
 * circumstances.
 */

static int is_accessable( const tsd_t *TSD, const streng *filename, int mode )
{
   int res=0 ;
   char *fn ;

   fn = str_ofTSD( filename ) ;
   /*
    * First, call access() with the 'correct' parameters, and store
    * the result in 'res'. If 'mode' had an "impossible" value, give
    * an error.
    */
#if defined(WIN32) && ( defined(__IBMC__) || defined(__LCC__) )
{
   DWORD Attrib;
   res=-1;
   Attrib=GetFileAttributes(fn);
   if (Attrib==(DWORD)-1)
      exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
   if ((Attrib&FILE_ATTRIBUTE_DIRECTORY)!=FILE_ATTRIBUTE_DIRECTORY)
   {
      if ((mode == COMMAND_READABLE) && ((Attrib&FILE_ATTRIBUTE_READONLY)==FILE_ATTRIBUTE_READONLY))
         res = 0 ;
      else if ((mode == COMMAND_WRITEABLE) || (mode == COMMAND_EXECUTABLE))
         res = 0 ;
   }
}
#else
   if (mode == COMMAND_READABLE)
      res = access( fn, R_OK ) ;
   else if (mode == COMMAND_WRITEABLE)
      res = access( fn, W_OK ) ;
   else if (mode == COMMAND_EXECUTABLE)
      res = access( fn, X_OK ) ;
   else
      exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
#endif

   /*
    * Perhaps we should analyze the output a bit before returning?
    * If res==EACCES, that is not really an error, while other errno
    * code _do_ signify an error. However ... since the return code
    * a boolean variable, just return it.
    */
   FreeTSD(fn) ;
   return (res==0) ;
}



/*
 * This little function implements the RESET command of the Rexx
 * built-in function STREAM(). Basically, most of the job is done in
 * the function 'fixup_file()'. Except from removing the ERROR flag.
 * The 'fixup_file()' function is intended for fixing the file at the
 * start of a condition handler for the NOTREADY condition.
 *
 * The value returned from this function is either "READY" or "UNKNOWN",
 * and reflects the philosophy that the file _is_ fixed, unless it
 * is impossible to open it. Of course, that may be a false READY,
 * since the actual _problem_ might not have been fixed, but at least
 * you have another try at the problem.
 */

static streng *reset_file( tsd_t *TSD, fileboxptr fileptr )
{
   if (!fileptr)
      return nullstringptr() ;

   fixup_file( TSD, fileptr->filename0 ) ;
   fileptr->flag &= ~(FLAG_ERROR | FLAG_FAKE) ;

   if (fileptr->fileptr)
      return Str_creTSD( "READY" ) ;   /* Per definition */
   else
      return Str_creTSD( "UNKNOWN" ) ;
}



/*
 * The built-in function STREAM() is new in TRL2. It is supposed to be
 * a sort of all-round function for just about anything having to do with
 * files. The details of its specification in TRL2 leaves a lot of room
 * for the implementors. Two of the options to this command -- the Status
 * and Description options are treated as defined by TRL, the Command
 * option takes several command, defined by the COMMAND_ macros.
 */
streng* std_stream( tsd_t *TSD, cparamboxptr parms )
{
   char oper=' ' ;
   streng *command=NULL, *result=NULL, *filename=NULL, *psub=NULL ;
   fileboxptr ptr=NULL ;

   /* Syntax: stream(filename[,[oper][,command ...]]) */
   if ((!parms)||(!parms->value))
      exiterror( ERR_INCORRECT_CALL, 5, "STREAM", 1 )  ;
   checkparam(  parms,  1,  3 , "STREAM" ) ;

   /*
    * Get the filepointer to Rexx's file table, but make sure that the
    * file is not in any way created if it didn't exist.
    */
   filename = Str_dupstrTSD( parms->value );
   ptr = getfileptr( TSD, filename ) ;
   /*
    * Read the 'operation'. This is really just an 'option'. The
    * default option is 'S'.
    */
   parms = parms->next ;
   if (parms && parms->value)
      oper = getoptionchar( TSD, parms->value, "STREAM", 2, "CSD", "" ) ;
   else
      oper = 'S' ;

   /*
    * If the operation was 'C', we _must_ have a third parameter, on the
    * other hand, if it was not 'C', we must never have a third parameter.
    * Make sure that these rules are followed.
    */
   command = NULL ;
   if (oper=='C')
   {
      parms = parms->next ;
      if (parms && parms->value)
         command = parms->value ;
      else
          exiterror( ERR_INCORRECT_CALL, 3, "STREAM", 3 )  ;
   }
   else
      if (parms && parms->next && parms->next->value)
          exiterror( ERR_INCORRECT_CALL, 4, "STREAM", 2 )  ;

   /*
    * Here comes the main loop.
    */
   result = NULL ;
   switch ( oper )
   {
      case 'C':
         /*
          * Read the command, and 'translate' it into an integer which
          * describes it, see the implementation of get_command(), and
          * the COMMAND_ macros. The first of these are rather simple,
          * in fact, they could probably be compressed to save some
          * space.
          */
         command = Str_strp( command, ' ', STRIP_BOTH );
         oper = get_command( command ) ;
         switch(oper)
         {
            case COMMAND_READ: /* Regina only - use OPEN READ */
               closefile( TSD, filename ) ;
               ptr = openfile( TSD, filename, ACCESS_READ ) ;
               break;
            case COMMAND_WRITE:  /* Regina only - use OPEN WRITE */
               closefile( TSD, filename ) ;
               ptr = openfile( TSD, filename, ACCESS_WRITE ) ;
               break;
            case COMMAND_APPEND: /* Regina only - use OPEN WRITE APPEND */
               closefile( TSD, filename ) ;
               ptr = openfile( TSD, filename, ACCESS_APPEND ) ;
               break;
            case COMMAND_UPDATE: /* Regina only - use OPEN BOTH */
               closefile( TSD, filename ) ;
               ptr = openfile( TSD, filename, ACCESS_UPDATE ) ;
               break;
            case COMMAND_CREATE: /* Regina only - use OPEN WRITE REPLACE */
               closefile( TSD, filename ) ;
               ptr = openfile( TSD, filename, ACCESS_CREATE ) ;
               break;
            case COMMAND_CLOSE:
               /*
                * The file is always unknown after is has been closed. Does
                * that sound convincing, or does it sound like I didn't feel
                * to implement the rest of this ... ?
                */
               closefile( TSD, filename ) ;
               result = Str_creTSD( "UNKNOWN" ) ;
               break ;
            case COMMAND_FLUSH:
               /*
                * Flush the file. Actually, this might not be needed, since
                * the functions that write out data may contain explicit
                * calls to fflush()
                */
               ptr = getfileptr( TSD, filename ) ;
               if (ptr && ptr->fileptr)
               {
                  errno = 0 ;
                  if (fflush( ptr->fileptr))
                  {
                     file_error( ptr, errno, NULL ) ;
                     result = Str_creTSD( "ERROR" ) ;
                  }
                  else
                     result = Str_creTSD( "READY" ) ;
               }
               else if (ptr)
                  result = Str_creTSD( "ERROR" ) ;
               else
                  result = Str_creTSD( "UNKNOWN" ) ;
               break ;
            case COMMAND_STATUS:
               ptr = getfileptr( TSD, filename ) ;
               result = getrexxstatus( TSD, ptr ) ;
               break;
            case COMMAND_FSTAT:
               result = getstatus( TSD, filename, COMMAND_FSTAT );
               break;
            case COMMAND_RESET:
               ptr = getfileptr( TSD, filename ) ;
               result = reset_file( TSD, ptr ) ;
               break;
            case COMMAND_READABLE:
            case COMMAND_WRITEABLE:
            case COMMAND_EXECUTABLE:
               result = int_to_streng( TSD, is_accessable( TSD, filename, oper )) ;
               break;
            case COMMAND_QUERY:
               /*
                * We have to further parse the remainder of the command
                * to determine what sub-command has been passed.
                */
               psub = Str_nodupTSD( command , 5, command->len - 5);
               psub = Str_strp( psub, ' ', STRIP_LEADING);
               result = getquery( TSD, filename, psub ) ;
               Free_stringTSD(psub);
               break;
            case COMMAND_OPEN:
               /*
                * We have to further parse the remainder of the command
                * to determine what sub-command has been passed.
                */
               psub = Str_nodupTSD( command , 4, command->len - 4);
               psub = Str_strp( psub, ' ', STRIP_LEADING);
               result = getopen( TSD, filename, psub ) ;
               Free_stringTSD(psub);
               break;
            case COMMAND_SEEK:
               psub = Str_nodupTSD( command , 4, command->len - 4);
               psub = Str_strp( psub, ' ', STRIP_LEADING);
               result = getseek( TSD, filename, psub ) ;
               Free_stringTSD(psub);
               break;
            case COMMAND_POSITION:
               psub = Str_nodupTSD( command , 8, command->len - 8);
               psub = Str_strp( psub, ' ', STRIP_LEADING);
               result = getseek( TSD, filename, psub ) ;
               Free_stringTSD(psub);
               break;
            default:
               exiterror( ERR_STREAM_COMMAND, 3, "CLOSE FLUSH OPEN POSITION QUERY RESET SEEK STATUS EXECUTABLE FSTAT READABLE WRITABLE", tmpstr_of( TSD, command ) )  ;
               break;
         }
         break ;

      case 'D':
         /*
          * Get a description of the most recent error for this file
          */
         if (ptr)
         {
            if (ptr->errmsg)
               result = Str_dupTSD(ptr->errmsg) ;
            else if (ptr->error)
               result = Str_creTSD( strerror(ptr->error) ) ;
         }
         break ;

      case 'S':
         /*
          * Get a simple status for the file in question. If the file
          * doesn't exist in Rexx's tables, UNKNOWN is returned. If the
          * file is in error state, return ERROR, else return READY,
          * unless current read position is at EOF, in which case
          * NOTREADY is return. Note that ERROR and NOTREADY are the
          * two states that will raise the NOTREADY condition.
          */
         if (ptr)
         {
            if (ptr->flag & FLAG_ERROR)
            {
               result = Str_creTSD( "ERROR" ) ;
            }
#if 1 /* really MH */
            else if (ptr->flag & FLAG_AFTER_RDEOF)
            {
               result = Str_creTSD( "NOTREADY" ) ;
            }
#else
            else if (ptr->flag & FLAG_RDEOF)
            {
               result = Str_creTSD( "NOTREADY" ) ;
            }
#endif
            else
            {
               result = Str_creTSD( "READY" ) ;
            }
         }
         else
            result = Str_creTSD( "UNKNOWN" ) ;

         break ;

      default:
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
   }

   if (result==NULL)
      result = nullstringptr() ;

   Free_stringTSD(filename);
   return result ;
}



/*
 * This routine will traverse the list of open files, and dump relevant
 * information about each of them. Really a debugging routine. It is
 * not available when Regina is compiled with optimalization.
 */
#ifndef NDEBUG
streng *dbg_dumpfiles( tsd_t *TSD, cparamboxptr parms )
{
   fileboxptr ptr ;
   int i=0, fno ;
   char string[11] ;
   fil_tsd_t *ft;
   char opt='N' ;
   int slot;

   ft = (fil_tsd_t *)TSD->fil_tsd;

   checkparam(  parms,  0,  2 , "DUMPFILES" ) ;

   if ( parms && parms->value )
      opt = getoptionchar( TSD, parms->value, "DUMPFILES", 1, "LN", "" ) ;

   if ( parms )
      parms = parms->next ;

   if ( parms && parms->value )
      slot = atopos( TSD, parms->value, "DUMPFILES", 2 ) ;
   else
      slot = -1 ;  /* dump all slots */

   if (TSD->stddump == NULL)
      return nullstringptr() ;

   if ( opt == 'N' )
   {
      fprintf(TSD->stddump,
        "                                                Read      Write\n" ) ;
      fprintf(TSD->stddump,
        "File Filename                       Flags    line char  line char\n");

      for ( ptr = ft->mrufile; ptr ; ptr = ptr->older )
      {
         fno = fileno( ptr->fileptr ) ;
         fprintf( TSD->stddump,"%4d %-30s", fno, ptr->filename0->value);
         i = 0 ;

         string[0] = (char) (( ptr->flag & FLAG_READ     ) ? 'r' : ' ') ;
         string[1] = (char) (( ptr->flag & FLAG_WRITE    ) ? 'w' : ' ') ;
         string[2] = (char) (( ptr->flag & FLAG_PERSIST  ) ? 'p' : 't') ;
         string[3] = (char) (( ptr->flag & FLAG_RDEOF    ) ? 'R' : ' ') ;
         string[4] = (char) (( ptr->flag & FLAG_AFTER_RDEOF    ) ? 'A' : ' ') ;
         string[5] = (char) (( ptr->flag & FLAG_WREOF    ) ? 'W' : ' ') ;
         string[6] = (char) (( ptr->flag & FLAG_SURVIVOR ) ? 'S' : ' ') ;
         string[7] = (char) (( ptr->flag & FLAG_ERROR    ) ? 'E' : ' ') ;
         string[8] = (char) (((ptr->flag & FLAG_FAKE) && (ptr->flag & FLAG_ERROR)   ) ? 'F' : ' ') ;
         string[9] = 0x00 ;

#if defined(HAVE_I64U)
         fprintf( TSD->stddump, " %8s %4I64u %4I64u  %4I64u %4I64u\n", string,
                ptr->readline, ptr->readpos,
                ptr->writeline,ptr->writepos ) ;
#else
            if ( sizeof(rx_64) > 4 )
               fprintf( TSD->stddump, " %8s %4lld %4lld  %4lld %4lld\n", string,
                      ptr->readline, ptr->readpos,
                      ptr->writeline,ptr->writepos ) ;
            else
               fprintf( TSD->stddump, " %8s %4ld %4ld  %4ld %4ld\n", string,
                      (long)(ptr->readline), (long)(ptr->readpos),
                      (long)(ptr->writeline),(long)(ptr->writepos) ) ;
#endif
         if (ptr->flag & FLAG_ERROR)
         {
            if (ptr->errmsg)
               fprintf(TSD->stddump, "     ==> %s\n", ptr->errmsg->value ) ;
            else if (ptr->error)
               fprintf(TSD->stddump, "     ==> %s\n", strerror( ptr->error )) ;
         }
      }
      fprintf( TSD->stddump,"  r=read, w=write, p=persistent, t=transient, e=eof\n");
      fprintf( TSD->stddump,"  R=read-eof, W=write-eof, S=special, E=error, F=fake\n");
   }
   else
   {
      int start, end;
      if ( slot == -1 )
      {
         start = 0;
         end = 131;
      }
      else
      {
         start = slot;
         end = start + 1;
      }
      for ( i = start; i < end; i++ )
      {
         ptr = ft->filehash[i];
         if ( ptr )
         {
            fno = 0;
            if ( ptr->fileptr )
               fno = fileno( ptr->fileptr ) ;
            fprintf( TSD->stddump,"Slot: %3d: %4d %8lx prev: %8lx next: %8lx %-30s\n",
               i, fno, (long)ptr, (long)ptr->prev, (long)ptr->next, ptr->filename0->value);
            for ( ptr = ptr->next; ptr; ptr = ptr->next )
            {
               fno = 0;
               if ( ptr->fileptr )
                  fno = fileno( ptr->fileptr ) ;
               fprintf( TSD->stddump,"           %4d %8lx prev: %8lx next: %8lx %-30s\n",
                  fno, (long)ptr, (long)ptr->prev, (long)ptr->next, ptr->filename0->value );
            }
         }
         else
         {
            fprintf( TSD->stddump,"Slot: %3d is empty\n", i );
         }
      }
   }

   return nullstringptr() ;
}
#endif




/*
 * Read from stdin using readoneline()
 */
streng *readkbdline( tsd_t *TSD )
{
   fil_tsd_t *ft;

   ft = (fil_tsd_t *)TSD->fil_tsd;
   return readoneline( TSD, ft->stdio_ptr[DEFAULT_STDIN_INDEX] );
}

void *addr_reopen_file( tsd_t *TSD, const streng *filename, char code,
                        int iserror )
/* This is the open routine for the ADDRESS WITH-redirection. filename is
 * the name of the file. code is either 'r' for "READ",
 * 'A' for "WRITE APPEND", 'R' for "WRITE REPLACE". In case of READ
 * already opened files will be reused. In case of APPEND or REPLACE the
 * files are (re-)opened. An internal structure for files is returned and
 * should be used for calls to addr_io_file.
 * An already opened file for write can't be used. See J18PUB.pdf, 5.5.1.
 * The return value may be NULL in case of an error. A NOTREADY condition
 * may have been raised in this case.
 * filename may be NULL for a default file.
 * iserror can be set or not. If set, stderr instead of stdout should be used.
 */
{
   fileboxptr ptr;
   fil_tsd_t *ft;

   ft = (fil_tsd_t *)TSD->fil_tsd;

   iserror = ( iserror ) ? 1 : 0;
   switch ( code )
   {
      case 'r':
         if ( ( filename == NULL ) || ( Str_len( filename ) == 0 ) )
            return ft->stdio_ptr[DEFAULT_STDIN_INDEX];
         ptr = get_file_ptr( TSD, filename, OPER_READ, ACCESS_READ );
         if ( ptr != NULL )
            ptr->readpos = 0;
         break;

      case 'A':
         if ( ( filename == NULL ) || ( Str_len( filename ) == 0 ) )
            return ft->stdio_ptr[DEFAULT_STDOUT_INDEX + iserror];
         if ( ( ptr = getfileptr( TSD, filename ) ) != NULL )
         {
            if ( ptr->flag & FLAG_SURVIVOR )
               return get_file_ptr( TSD, filename, OPER_WRITE, ACCESS_WRITE );
         }
         closefile( TSD, filename );
         ptr = openfile( TSD, filename, ACCESS_STREAM_APPEND );
         break;

      case 'R':
         if ( ( filename == NULL ) || ( Str_len( filename ) == 0 ) )
            return ft->stdio_ptr[DEFAULT_STDOUT_INDEX + iserror];
         if ( ( ptr = getfileptr( TSD, filename ) ) != NULL )
         {
            if ( ptr->flag & FLAG_SURVIVOR )
               return get_file_ptr( TSD, filename, OPER_WRITE, ACCESS_WRITE );
         }
         closefile( TSD, filename );
         ptr = openfile( TSD, filename, ACCESS_STREAM_REPLACE );
         break;

      default:
         ptr = NULL;
         break;
   }

   if ( ( ptr != NULL ) && ( ptr->fileptr == NULL ) )
      ptr = NULL;

   return ptr;
}

streng *addr_io_file( tsd_t *TSD, void *fileptr, const streng *buffer )
/* This is the working routine for the ADDRESS WITH-redirection. fileptr is
 * the return value of addr_reopen_file. buffer must be NULL for a read
 * operation or a filled buffer.
 * The return value is NULL in case of a write operation or in case of EOF
 * while reading.
 * All IO is done by charin/charout.
 * A NOTREADY condition won't be raised.
 */
{
   streng *retval = NULL ;

   if ( fileptr == NULL ) /* fixes bug 806948 */
      return retval;

   if ( buffer == NULL )
      retval = readbytes( TSD, (fileboxptr)fileptr, 0x1000, 1 ) ;
   else
      writebytes( TSD, (fileboxptr)fileptr, buffer, 1 ) ;

   return( retval ) ;
}

void addr_reset_file( tsd_t *TSD, void *fileptr )
/* This is the "close" routine for the ADDRESS WITH-redirection. We don't close
 * the stream, we simply perform a reset, that's enough to restart reading
 * or writing.
 */
{
   fileboxptr ptr = (fileboxptr)fileptr;

   if ( fileptr == NULL ) /* fixes bug 806948 */
      return;

   if ( ptr->fileptr )
   {
      clearerr( ptr->fileptr );
      if ( ptr->flag & FLAG_PERSIST )
         rx_fseek( ptr->fileptr, 0, SEEK_SET );
      ptr->thispos = 0;
      ptr->oper = OPER_NONE;
   }

   if ( ptr->flag & FLAG_SURVIVOR )
      ptr->flag &= ~FLAG_ERROR;

   ptr->flag &= ~FLAG_FAKE;
}


streng *addr_file_info( tsd_t *TSD, const streng *source, int defchannel )
/*
 * addr_file_info is a helper for the ADDRESS WITH-redirection. source is the
 * name of the file and may be NULL for a default channel. The channel's number
 * is used then, 0 = stdin, 1 = stdout, 2 = stderr.
 * The return name is a fresh copy of the qualified name of the file.
 */
{
   fil_tsd_t *ft;
   streng *result;
   fileboxptr p;

   ft = (fil_tsd_t *)TSD->fil_tsd;

   /*
    * We don't know anything about the default channels. So just return our
    * internal name.
    */
   if ( source == NULL )
      return Str_dupstrTSD( ft->stdio_ptr[ defchannel ]->filename0 );

   /*
    * Check for well known devices, the SURVIVORs. Return a comparable name
    * which makes sure "<stdin>" is equal to "stdin".
    */
   if ( ( p = getfileptr( TSD, source ) ) != NULL )
   {
      if ( p->flag & FLAG_SURVIVOR )
      {
         if ( p->fileptr == stdin )
            defchannel = 0;
         else if ( p->fileptr == stdout )
            defchannel = 1;
         else
            defchannel = 2;

         return Str_dupstrTSD( ft->stdio_ptr[ defchannel ]->filename0 );
      }
   }

   /*
    * Even in case of character devices a name resolution makes sense, e.g.
    * "tty" may expand to "/dev/tty" or the fifo "local" may expand to
    * "/somethere/local".
    */
   result = Str_makeTSD( REXX_PATH_MAX );
   my_fullpathstreng( TSD, result->value, source );
   result->len = strlen( result->value );

   return result;
}


/*
 * This routine is not really interesting. You should use the STREAM()
 * built-in function for greater portability and functionality. It is
 * left in the code for portability reasons.
 */
streng *unx_open( tsd_t *TSD, cparamboxptr parms )
{
   fileboxptr ptr=NULL ;
   char ch=' ' ;
   int iaccess=ACCESS_NONE ;

   checkparam(  parms,  1,  2 , "OPEN" ) ;

   if ((parms->next)&&(parms->next->value))
   {
      ch = getoptionchar( TSD, parms->next->value, "OPEN", 2, "RW", "" ) ;
      if ( ch == 'R' ) /* bja */
         iaccess = ACCESS_READ ;
      else if ( ch == 'W' ) /* bja */
         iaccess = ACCESS_WRITE ;
      else
         assert( 0 ) ;
   }
   else
      iaccess = ACCESS_READ ;

   ptr = openfile( TSD, parms->value, iaccess ) ;

   return int_to_streng( TSD,( ptr && ptr->fileptr )) ;
}


/*
 * This routine is not really interesting. You should use the CLOSE
 * command of the STREAM() built-in function for greater portability
 * and compatibility. It is left in the code only for compatibility
 * reasons.
 */
streng *unx_close( tsd_t *TSD, cparamboxptr parms )
{
   fileboxptr ptr=NULL ;

   checkparam(  parms,  1,  1 , "CLOSE" ) ;
   ptr = getfileptr( TSD, parms->value ) ;
   closefile( TSD, parms->value ) ;

   return int_to_streng( TSD, ptr!=NULL ) ;
}


/*
 * a function called exists that checks if a file with a certain name
 * exists. This function was taken from the ARexx API.
 */
streng *arexx_exists( tsd_t *TSD, cparamboxptr parms )
{
   char *name;
   streng *retval;
   struct rx_stat_buf st;

   checkparam( parms, 1, 1, "EXISTS" ) ;

   name = str_of( TSD, parms->value ) ;
   retval = int_to_streng( TSD, my_rx_stat( TSD, name, &st ) != -1 ) ;
   Free_TSD( TSD, name ) ;

   return retval;
}

/*
 * get_external_routine_file opens a file in binary mode and returns the
 * fully qualified path name on success. NULL is returned otherwise.
 * The opened file pointer is returned in *fp.
 */
static streng *get_external_routine_file( const tsd_t *TSD,
                                          const char *inname, FILE **fp )
{
   char buf[3 * REXX_PATH_MAX + 1];

#ifdef VMS
   *fp = fopen( inname, "r" );
#else
   *fp = fopen( inname, "rb" );
#endif
   if ( *fp == NULL )
      return NULL;

   my_fullpath( TSD, buf, inname );

   return Str_crestrTSD( buf );
}

/*
 * See get_external_routine for comments. This function processes one path
 * element which is passed in path.
 * suffixes is either NULL or the list of extra suffixes which should be
 * tested. *fp must be NULL on entry.
 * path may be NULL if no further directory processing shall happen.
 */
static streng *get_external_routine_path( const tsd_t *TSD,
                                          const char *inname, FILE **fp,
                                          const char *path,
                                          const char *suffixes,
                                          int emptySuffixAllowed )
{
   char outname[REXX_PATH_MAX+1];
   int i,ilen,hlen;
   streng *retval;
   static const char *default_suffixes = "rexx,rex,cmd,rx";
   const char *suffixlist[2];
   const char *suffix;
   int suffixlen;

   ilen = strlen( inname );
   if ( !path )
      hlen = 0;
   else
      hlen = strlen( path );
   if ( !hlen )
   {
      if ( ilen > REXX_PATH_MAX )
         return NULL;
      strcpy( outname, inname );
   }
   else
   {
      if ( ( strchr( FILE_SEPARATORS, inname[0] ) == NULL ) &&
           ( strchr( FILE_SEPARATORS, path[hlen - 1] ) == NULL ) )
      {
         if ( ilen + hlen + 1 > REXX_PATH_MAX )
            return NULL;
         strcpy( outname, path );
         strcat( outname, FILE_SEPARATOR_STR );
         strcat( outname, inname );
      }
      else
      {
         if ( ilen + hlen > REXX_PATH_MAX )
            return NULL;
         strcpy( outname, path );
         strcat( outname, inname );
      }
   }

   /*
    * The filename is constructed. Try without fiddling with suffixes first.
    */
   if ( emptySuffixAllowed )
   {
      if ( ( retval = get_external_routine_file( TSD, outname, fp ) ) != NULL )
         return retval;
   }

   /*
    * Next try the supplied suffix list, then try the default list.
    * First check if a known extension exists, after every check do the
    * application.
    */

   suffixlist[0] = suffixes;
   suffixlist[1] = default_suffixes;
   ilen = strlen( outname );
#define IsDelim(c) ( ( (c) == ',' ) || ( (c) == '.' ) || \
                     ( (c) == PATH_SEPARATOR ) || rx_isspace(c) )
   for ( i = 0; i < 2; i++ )
   {
      suffixes = suffixlist[i];

      while ( suffixes )
      {
         while ( IsDelim(*suffixes) )
            suffixes++;
         if ( *suffixes == '\0' )
            break;

         for ( suffixlen = 1; !IsDelim(suffixes[suffixlen]); suffixlen++ )
            if ( suffixes[suffixlen] == '\0' )
               break;

         suffix = suffixes;
         suffixes += suffixlen;

         if ( suffixlen + 1 > ilen )
            continue;
         if ( outname[ ilen - suffixlen - 1 ] != '.' )
            continue;
#ifdef CASE_SENSITIVE_FILENAMES
         if ( memcmp( suffix, outname + ilen - suffixlen, suffixlen - 1 ) )
#else
         if ( mem_cmpic( suffix, outname + ilen - suffixlen, suffixlen - 1 ) )
#endif
            continue;

         /*
          * A matching suffix forces us to terminate every further seeking a
          * proper file.
          */
         if ( !emptySuffixAllowed )
            return get_external_routine_file( TSD, outname, fp );
         return NULL;
      }
   }

   /*
    * Try the extensions.
    */
   for ( i = 0; i < 2; i++ )
   {
      suffixes = suffixlist[i];

      while ( suffixes )
      {
         while ( IsDelim(*suffixes) )
            suffixes++;
         if ( *suffixes == '\0' )
            break;

         for ( suffixlen = 1; !IsDelim(suffixes[suffixlen]); suffixlen++ )
            if ( suffixes[suffixlen] == '\0' )
               break;

         suffix = suffixes;
         suffixes += suffixlen;

         if ( suffixlen + 1 + ilen > REXX_PATH_MAX )
            continue;
         outname[ ilen ] = '.';
         memcpy( outname + ilen + 1, suffix, suffixlen );
         outname[ilen + 1 + suffixlen] = '\0';
         if ( ( retval = get_external_routine_file( TSD, outname, fp ) ) !=
                                                                         NULL )
            return retval;
      }
   }

#undef Delim
   return NULL;
}

/*
 * See get_external_routine for comments. This function processes a list of
 * path elements delimited by the path separator which is passed in paths.
 * suffixes is either NULL or the list of extra suffixes which should be
 * tested. *fp must be NULL on entry.
 * paths will be destroyed.
 */
static streng *get_external_routine_paths( const tsd_t *TSD,
                                           const char *inname, FILE **fp,
                                           char *paths, const char *suffixes,
                                           int emptySuffixAllowed )
{
   char *path;
   streng *retval;

   if ( *paths == '\0' )
      return NULL;

   while ( paths )
   {
      path = paths;
      paths = strchr( paths, PATH_SEPARATOR );
      if ( paths != NULL )
         *paths++ = '\0';

      if ( *path == '\0')
      {
         /*
          * An empty string is counted as "." in unix systems and ignored in
          * all other systems.
          */
#ifdef UNIX
         path = ".";
#else
         continue;
#endif
      }

      retval = get_external_routine_path( TSD, inname, fp, path, suffixes,
                                          emptySuffixAllowed );

      if ( retval )
         return retval;
   }

   return NULL;
}

/*
 * get_external_routine searches for a script called inname. Some paths are
 * search if the file is not found and an extension may be added if no file is
 * found.
 *
 * On success *fp is set to the opened (binary) file and the return value is
 * the fully qualified file name. If no file was found the return value is
 * NULL and *fp will be NULL, too.
 * The returned file name is extended by a terminating '\0' without counting
 * is in the string's length.
 *
 * This is the search algorithm:
 *
 * First of all we process the environment variable REGINA_MACROS. If no file
 * is found we proceed with the current directory and then with the environment
 * variable PATH. The semantics of the use of REGINA_MACROS and PATH are the
 * same, and the search in the current directory is omitted for the superuser
 * in unix systems for security reasons. The current directory must be
 * specified explicitely by the superuser.
 * When processing an environment variable the content is split into the
 * different paths and each path is processed separately.
 * Note that the search algorithm to this point is ignored if the script name
 * contains a file path specification. eg. If "CALL .\MYPROG" is called, then
 * no searching of REGINA_MACROS or PATH is done; only the concatenation of
 * suffixes is carried out.
 *
 * For each file name and path element a concatenated file name is created. If
 * a known file extension is part of the file name only this file is searched,
 * otherwise the file name is extended by the extensions "<empty>", ".rexx",
 * ".rex", ".cmd", ".rx" in this order. The file name case is ignored on
 * systems that ignore the character case for normal file operations like DOS,
 * Windows, OS/2.
 *
 * The first matching file terminates the whole algorithm and the found file
 * is returned.
 *
 * The environment variable REGINA_SUFFIXES extends the list of known suffixes
 * as specified above, and is inserted after the "<empty"> extension in the
 * process. REGINA_SUFFIXES has to contain a space or comma separated list of
 * extensions, a dot in front of each entry is allowed, e.g.
 * ".macro,.mac,regina" or "macro mac regina"
 *
 * Note that it is planned to extend the list of known suffixes by ".rxc" in
 * version 3.4 to allow for seemless integration of precompiled macros.
 */
streng *get_external_routine( const tsd_t *TSD, const char *inname, FILE **fp )
{
   streng *retval=NULL;
   char *paths;
   char *suffixes;

   *fp = NULL;

   suffixes = mygetenv( TSD, "REGINA_SUFFIXES", NULL, 0 );

   if ( strpbrk( inname, FILE_SEPARATORS ) != NULL )
   {
      retval = get_external_routine_path( TSD, inname, fp, NULL, suffixes, 1 );
      if ( retval )
      {
         if ( suffixes )
            FreeTSD( suffixes );
         return retval;
      }
      return NULL;
   }

   if ( ( paths = mygetenv( TSD, "REGINA_MACROS", NULL, 0 ) ) != NULL )
   {
      retval = get_external_routine_paths( TSD, inname, fp, paths, suffixes, 1 );
      FreeTSD( paths );
      if ( retval )
      {
         if ( suffixes )
            FreeTSD( suffixes );
         return retval;
      }
   }

   paths = ".";
#ifdef UNIX
   if ( geteuid() == 0 )
      paths = NULL;
#endif
   if ( paths )
   {
      retval = get_external_routine_path( TSD, inname, fp, paths, suffixes, 1 );
      if ( retval )
      {
         if ( suffixes )
            FreeTSD( suffixes );
         return retval;
      }
   }

   if ( ( paths = mygetenv( TSD, "PATH", NULL, 0 ) ) != NULL )
   {
      retval = get_external_routine_paths( TSD, inname, fp, paths, suffixes, 0 );
      FreeTSD( paths );
   }

   if ( suffixes )
      FreeTSD( suffixes );
   return retval;
}

/*
 * find_shared_library is used for HP/UX purpose only.
 * It looks for the file inname in the content of the environment variable
 * inenv and puts the result into retname. retname has to have a size of
 * at least REXX_PATH_MAX+1.
 * retname becomes inname if no other file is found.
 */
void find_shared_library(const tsd_t *TSD, const char *inname, const char *inenv, char *retname)
{
   char *paths;
   char outname[REXX_PATH_MAX+1];
   char *env_path;

   strcpy( retname, inname );
   env_path = mygetenv( TSD, inenv, NULL, 0 ); /* fixes bug 595293 */
   if ( !env_path )
      return;
   paths = env_path;
   while ( paths && *paths )
   {
      int pathlen;
      char *sep;

      sep = strchr( paths, PATH_SEPARATOR );
      pathlen = sep ? sep-paths : strlen( paths );
      strncpy( outname, paths, pathlen );
      outname[pathlen] = 0;

      if ( ( pathlen > 0 ) && ( outname[pathlen-1] != FILE_SEPARATOR ) )
         strcat( outname, FILE_SEPARATOR_STR );
      strcat( outname, inname );
      paths = sep ? sep+1 : 0; /* set up for next pass */
      if ( access( outname,F_OK ) == 0)
      {
         strcpy( retname,outname );
         break;
      }
   }
   FreeTSD( env_path );
   return;
}

/* CloseOpenFiles closes all scripting input files and it closes all opened
 * STREAM files without destroying the associated informations.
 * Bug 982062: Added FilePtrDisposition to allow for purging fileptr
 * table if requested.
 */
void CloseOpenFiles( const tsd_t *TSD, FilePtrDisposition fpd )
{
   sysinfobox *ptr;

   if ( fpd == fpdRETAIN )
   {
      ptr = TSD->systeminfo;
      while (ptr)
      {
         if (ptr->input_fp)
         {
            fclose(ptr->input_fp);
            ptr->input_fp = NULL;
         }
         ptr = ptr->previous;
      }
      /*
       * Cheat about the const-state.
       */
      swapout_all( ( tsd_t *) TSD );
   }
   else
   {
      purge_filetable( ( tsd_t *) TSD );
   }
   return;
}

streng *ConfigStreamQualified( tsd_t *TSD, const streng *name )
{
   char *fn=NULL;
   streng *result=NULL ;

   /*
    * Nul terminate the input filename string, as stat() will barf if
    * it isn't and other functions stuff up!
    */
   fn = str_ofTSD(name);

   result = Str_makeTSD( REXX_PATH_MAX );
   if ( my_fullpath( TSD, result->value, fn ) == -1 )
   {
      /*
       * my_fullpath failed, so split the supplied file into filename
       * and directory. Then look for directory and append the filename
       * TODO -
       */
   }
   result->len = strlen( result->value );
   return( result );
}

#if defined(HAVE__FULLPATH) || defined(__EMX__)
/*
 * my_fullpath tries to get the fully qualified name of a file or directory
 * even if it doesn't exist. It tries to return a reasonable value even if
 * a path element is missing.
 * The return value is 0 on success, -1 in case of a severe error.
 */
int my_fullpath( tsd_t *TSD, char *dst, const char *src )
{
   char *copy=NULL;
   int rc = 0, len;
# if defined(__EMX__)
   int i;

   if ( _fullpath( dst, src, REXX_PATH_MAX ) == -1)
      strcpy( dst, src );
   /*
    * Convert / back to \.
    */
   len = strlen( dst );
   for ( i = 0; i < len; i++ )
   {
      if ( dst[i] == '/' )
         dst[i] = '\\';
   }
# else
   /* attempt to get the full path with the supplied source */
   if ( _fullpath( dst, src, REXX_PATH_MAX ) == NULL )
   {
#  if defined(WIN32)
      /* now try by removing trailing slash */
      /* this code may not actually be needed */
      len = strlen( src ) -  1;
      if ( src[len] == '\\' || src[len] == '/')
      {
         copy = (char *)MallocTSD( len+2 ) ;
         strcpy( copy, src );
         copy[len] = '\0';
         if ( _fullpath( dst, copy, REXX_PATH_MAX ) == NULL )
         {
            strcpy( dst, src );
            rc = -1;
         }
      }
      else
      {
         strcpy( dst, src );
         rc = -1;
      }
      if ( copy )
         FreeTSD( copy );
#  else
      strcpy( dst, src );
      rc = -1;
#  endif
   }
# endif
   return rc;
}
#elif defined(HAVE__TRUENAME)

int my_fullpath( tsd_t *TSD, char *dst, const char *src )
{
   _truename( src, dst );

   return 0;
}
#elif defined(HAVE_REALPATH)

int my_fullpath( tsd_t *TSD, char *dst, const char *src )
{
   const char *source=NULL;
   /* hack for leading ~/ */
   source = translateTilde( TSD, src );
   realpath( source, dst );
   if ( source != src )
   {
      FreeTSD( source );
   }

   return 0;
}
#elif defined(VMS)
#  include <ssdef.h>
#  include <rmsdef.h>
#  include <descrip.h>

int my_fullpath( tsd_t *TSD, char *dst, const char *src )
{
   char *s;
   int status, context = 0;
   struct dsc$descriptor_d result_dx = {0, DSC$K_DTYPE_T, DSC$K_CLASS_D, 0};
   struct dsc$descriptor_d finddesc_dx = {0, DSC$K_DTYPE_T, DSC$K_CLASS_D, 0};

   finddesc_dx.dsc$a_pointer = (char *)src; /* You may need to cast this */
   finddesc_dx.dsc$w_length = strlen(src);
   status = lib$find_file( &finddesc_dx, &result_dx, &context, 0, 0, 0, 0 );
   if ( status == RMS$_NORMAL )
   {
      memcpy(dst,result_dx.dsc$a_pointer,result_dx.dsc$w_length);
      *(dst+result_dx.dsc$w_length) = '\0';
   }
   else
      strcpy(dst,src);
   lib$find_file_end(&context);
   str$free1_dx(&result_dx);
   return(0);
}
#else /* neither _FULLPATH, _TRUENAME, REALNAME, VMS */

int my_fullpath( tsd_t *TSD, char *dst, const char *src )
{
   char tmp[REXX_PATH_MAX+1];
   char curr_path[REXX_PATH_MAX+1];
   char path[REXX_PATH_MAX+1];
   char fname[REXX_PATH_MAX+1];
   int i = 0, len = -1, retval;
   struct rx_stat_buf stat_buf;

   getcwd(curr_path,REXX_PATH_MAX);
   strcpy(tmp,src);
   /*
    * First determine if the supplied filename is a directory.
    */
# if defined(DJGPP) || defined(__WINS__) || defined(__EPOC32__)
   for ( i = 0; i < strlen( tmp ); i++ )
      if ( tmp[ i ] == '\\' )
         tmp[ i ] = '/';
# endif
   if ((rx_stat(tmp,&stat_buf) == 0)
   &&  (stat_buf.st_mode & S_IFMT) == S_IFDIR)
   {
      strcpy(path,tmp);
      strcpy(fname,"");
   }
   else          /* here if the file doesn't exist or is not a directory */
   {
      for (i=strlen(tmp),len=-1;i>-1;i--)
      {
         if (tmp[i] == '/')
         {
            len = i;
            break;
         }
      }
      switch(len)
      {
         case (-1):
            getcwd(path,REXX_PATH_MAX);
            strcpy(fname,tmp);
            break;
         case 0:
            strcpy(path,tmp);
            path[1] = '\0';
            strcpy(fname,tmp+1+len);
            break;
         default:
            strcpy(path,tmp);
            path[len] = '\0';
            strcpy(fname,tmp+1+len);
            break;
      }
   }
   /*
    * Change directory to the supplied path, if possible and store the
    * expanded path.
    * If an error, restore the current path.
    */
   if (chdir(path) != 0)
   {
      retval = -1;
   }
   else
   {
      getcwd(path,REXX_PATH_MAX);
      retval = 0;
   }
   chdir(curr_path);
   /*
    * Append the OS directory character to the path if it doesn't already
    * end in the character.
    */
   len = strlen(path);
   if (len > 0)
   {
# if defined(__WINS__) || defined(__EPOC32__)
      if ( path[ len - 1 ] != '\\'
# else
      if ( path[ len - 1 ] != '/'
# endif
      &&  strlen( fname ) != 0 )
      {
         strcat(path,"/");
         len++;
      }
# if defined(DJGPP) || defined(__WINS__) || defined(__EPOC32__)
      for ( i = 0; i < len; i++ )
         if ( path[ i ] == '/' )
            path[ i ] = '\\';
# endif
   }
   strcpy(dst,path);
   strcat(dst,fname);

   return retval;
}
#endif

int my_fullpathstreng( const tsd_t *TSD, char *dst, const streng *src )
{
   char *copy;
   int retval;

   copy = str_ofTSD( src );
   retval = my_fullpath( TSD, dst, copy );
   FreeTSD( copy );

   return retval;
}

#if !defined(HAVE__SPLITPATH2) && !defined(HAVE__SPLITPATH) && !defined(__EMX__) && !defined(DJGPP)
int my_splitpath2( const char *in, char *out, char **drive, char **dir, char **name, char **ext )
{
   int inlen = strlen(in);
   int last_slash_pos=-1,last_dot_pos=-1,last_pos=0,i=0;

   for (i=0;i<inlen;i++)
   {
      if ( *(in+i) == '/' || *(in+i) == '\\' )
         last_slash_pos = i;
      else if ( *(in+i) == '.' )
         last_dot_pos = i;
   }
   /*
    * drive is always empty !
    */
   out[0] = '\0';
   *drive = out;

   *ext = out+1;
   if (last_dot_pos > last_slash_pos)
   {
      strcpy(*ext,in+last_dot_pos);
      last_pos = 2 + (inlen - last_dot_pos);
      inlen = last_dot_pos;
   }
   else
   {
      **ext = '\0';
      last_pos = 2;
   }
   *dir = out+last_pos;
   /*
    * If there is a path component (last_slash_pos not -1), then copy
    * from the start of the in string to the last_slash_pos to out[1]
    */
   if (last_slash_pos != -1)
   {
      memcpy(*dir, in, last_slash_pos + 1);
      last_pos += last_slash_pos + 1;
      out[last_pos++] = '\0';
      *name = out+last_pos;
      memcpy(*name, in+last_slash_pos+1,(inlen - last_slash_pos - 1) );
      out[last_pos + (inlen - last_slash_pos - 1)] = '\0';
   }
   else
   {
      **dir = '\0';
      last_pos++;
      *name = out+last_pos;
      memcpy(*name, in, inlen);
      *(*name+inlen) = '\0';
   }
   return(0);
}
#endif

/*
 * Enable ~ to be translated to $HOME under linux
 * The caller is responsible for freeing the return value ONLY if
 * the return value is NOT the same as the input parameter
 */
#ifdef UNIX
static const char *translateTilde( tsd_t *TSD, const char *src )
{
   int len = strlen( src );
   char *copy = (char *)src;
   if ( ( len > 1 && strncmp( src, "~/", 2 ) == 0 ) || ( len == 1 && strncmp( src, "~", 1 ) == 0) )
   {
      char *env = getenv( "HOME" );
      if ( env != NULL )
      {
         int len2 = strlen( env );
         copy = (char *)MallocTSD( len+len2+2 ) ;
         strcpy( copy, env );
         strcat( copy, src+1 );
      }
      else
      {
         copy = (char *)src;
      }
   }
   else
   {
      copy = (char *)src;
   }
   return copy;
}
int my_rx_stat(  tsd_t *TSD, const char *src, struct stat *statbuf )
{
   int rc;
   char *source = (char *)translateTilde( TSD, src );

   rc = rx_stat( source, statbuf ) ;
   if ( source != src )
   {
      FreeTSD( source );
   }
   return rc;
}
#else
static const char *translateTilde( tsd_t *TSD, const char *src )
{
   return src;
}

int my_rx_stat(  tsd_t *TSD, const char *src, struct stat *statbuf )
{
   return rx_stat( src, statbuf ) ;
}
#endif
