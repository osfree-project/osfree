#ifndef lint
static char *RCSid = "$Id: doscmd.c,v 1.58 2004/03/12 12:20:17 mark Exp $";
#endif

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

/* FGC: From now it is intended to put additional (!!) things here for
 * shell.c which are related to the different OS.
 */

#if defined(OS2) || defined(__EMX__)
# define INCL_BASE
# include <os2.h>
# define DONT_TYPEDEF_PFN
#endif

#include "rexx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(DOS)                                        /* MH 10-06-96 */
# ifdef _POSIX_SOURCE
#  undef _POSIX_SOURCE
# endif
# include <dos.h>
#endif                                                  /* MH 10-06-96 */

#include <errno.h>

#if defined(HAVE_ASSERT_H)
# include <assert.h>
#endif

#if defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#if defined(HAVE_SYS_STAT_H)
# include <sys/stat.h>
#endif

#if defined(HAVE_SYS_WAIT_H)
# include <sys/wait.h>
#endif

#if defined(HAVE_FCNTL_H)
# include <fcntl.h>
#endif

#if defined(HAVE_SYS_FCNTL_H)
# include <sys/fcntl.h>
#endif

#if defined(WIN32)
# include <share.h>
# if defined(__BORLANDC__) || defined(__LCC__)
#  include <time.h>
#  include <process.h>
# endif
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
#endif

#if defined(MAC) || (defined(__WATCOMC__) && !defined(__QNX__)) || defined(_MSC_VER) || defined(__SASC) || defined(__MINGW32__) || defined(__BORLANDC__) || defined(__EPOC32__) || defined(__WINS__) || defined(__LCC__) || defined(SKYOS)
# include "utsname.h"                                   /* MH 10-06-96 */
# define NEED_UNAME
# if !defined(__WINS__) && !defined(__EPOC32__) && !defined(SKYOS)
#  define MAXPATHLEN  _MAX_PATH                          /* MH 10-06-96 */
# endif
#else                                                   /* MH 10-06-96 */
# if defined(WIN32) && defined(__IBMC__)                /* LM 26-02-99 */
#  include "utsname.h"
#  define NEED_UNAME
#  define MAXPATHLEN (8192)
#  include <io.h>
# else
#  ifndef VMS
#   include <sys/param.h>                                 /* MH 10-06-96 */
#  endif
#  include <sys/utsname.h>                               /* MH 10-06-96 */
#  include <sys/wait.h>
# endif
#endif

#if defined(MAC) || defined(GO32) || defined (__EMX__) || (defined(__WATCOMC__) && !defined(__QNX__)) || defined(_MSC_VER) || defined(DJGPP) || defined(__CYGWIN32__) || defined(__BORLANDC__) || defined(__MINGW32__) || defined(__WINS__) || defined(__EPOC32__)
# define HAVE_BROKEN_TMPNAM
# define PATH_DELIMS ":\\/"
# if defined(__EMX__) || defined(__CYGWIN32__)
#  define ISTR_SLASH "/"  /* This is not a must, \\ works, too */
#  define I_SLASH '/'  /* This is not a must, \\ works, too */
# elif defined(MAC)
#  define ISTR_SLASH ":"
#  define I_SLASH ':'
# else
#  define ISTR_SLASH "\\" /* This is not a must, / works at least for MSC, too */
#  define I_SLASH '\\'    /* This is not a must, / works at least for MSC, too */
# endif
# if !defined(MAC) && !defined(__WINS__) && !defined(__EPOC32__) && !defined(__CYGWIN__)
#  ifndef HAVE_UNISTD_H
#   include <io.h> /* access() */
#  endif
#  include <process.h>
#  include <share.h>
# endif
# include <time.h>
#endif

#if defined(__LCC__)
# if !defined(HasOverlappedIoCompleted)
#  define HasOverlappedIoCompleted(lpOverlapped) ((lpOverlapped)->Internal != STATUS_PENDING)
# endif
#endif

static char **makeargs(const char *string, char escape);
static char **makesimpleargs(const char *string);
static char *splitoffarg(const char *string, const char **trailer, char escape);
static void destroyargs(char **args);
static int local_mkstemp(const tsd_t *TSD, char *base);

#if defined(WIN32)
/*
 * The following; WIN9X_VER is used to determine if we are running under
 * a DOS-based Win32 platform; ie 95/98/Me
 * It can be changed to ( 1 ) for example to force the code through the
 * Win9X code if running on a different Win32 platform like NT.
#define WIN9X_VER ( 1 )
 */
#define WIN9X_VER ( _osver & 0x8000 )
/*****************************************************************************
 *****************************************************************************
 ** Win32 ********************************************************************
 *****************************************************************************
 *****************************************************************************/
typedef struct {
   const tsd_t   *TSD;
   struct {
      HANDLE      hdl;
      OVERLAPPED  ol;
      char       *buf;
      unsigned    maxbuf;
      unsigned    rused;
      unsigned    rusedbegin;
      unsigned    reading;
      unsigned    wused;
      int         is_reader;
   } h[3];
} AsyncInfo;

int open_subprocess_connection_dos(const tsd_t *TSD, environpart *ep);
void unblock_handle_dos(int *handle, void *async_info);
void restart_file_dos(int hdl);
int __regina_close_dos(int handle, void *async_info);
int __regina_read_dos(int handle, void *buf, unsigned size, void *async_info);
int __regina_write_dos(int handle, const void *buf, unsigned size,
                                                             void *async_info);
void *create_async_info_dos(const tsd_t *TSD);
void delete_async_info_dos(void *async_info);
void reset_async_info_dos(void *async_info);
void add_async_waiter_dos(void *async_info, int handle, int add_as_read_handle);
void wait_async_info_dos(void *async_info);

static BOOL MyCancelIo(HANDLE handle)
{
   static BOOL (WINAPI *DoCancelIo)(HANDLE handle) = NULL;
   static BOOL first = TRUE;
   HMODULE mod;

   if ( first )
   {
      /*
       * The kernel is always mapped, a LoadLibrary is useless
       */
      if ( ( mod = GetModuleHandle( "kernel32" ) ) != NULL )
      {
         DoCancelIo = (BOOL (WINAPI*)(HANDLE)) GetProcAddress( mod, "CancelIo" );
      }

      /*
       * It's safe now to set first. Never do it before the main work,
       * otherwise we're not reentrant.
       */
      first = FALSE;
   }

   if ( DoCancelIo == NULL )
      return FALSE;

   return DoCancelIo( handle );
}

int my_win32_setenv( const char *name, const char *value )
{
   return (SetEnvironmentVariable( name, value ) );
}

/* fork_exec spawns a new process with the given commandline.
 * it returns -1 on error (errno set), 0 on process start error (rcode set),
 * a process descriptor otherwise.
 * Basically this is a child process and we run in the child's environment
 * after the first few lines. The setup hasn't been done and the command needs
 * to be started.
 * Redirection takes place if one of the handles env->input.hdls[0],
 * env->output.hdls[1] or env->error.hdls[1] is defined. Other handles (except
 * standard handles) are closed. env->subtype must be a SUBENVIR_... constant.
 * cmdline is the whole command line.
 * Never use TSD after the fork() since this is not only a different thread,
 * it's a different process!
 */
int fork_exec(tsd_t *TSD, environment *env, const char *cmdline, int *rcode)
{
   static const char *interpreter[] = { "regina.exe", /* preferable even if */
                                                      /* not dynamic        */
                                        "rexx.exe" };
   PROCESS_INFORMATION pinfo;
   STARTUPINFO         sinfo;
   DWORD               done;
   char               *execname = NULL;
   const char         *commandline = NULL;
   char               *argline = NULL;
   BOOL                rc;
   int                broken_address_command = get_options_flag( TSD->currlevel, EXT_BROKEN_ADDRESS_COMMAND );
   int                subtype;

   if (env->subtype == SUBENVIR_REXX) /*special situation caused by recursion*/
   {
      environment e = *env;
      char *new_cmdline;
      int i, rc;
      unsigned len;

      if (argv0 == NULL)
         len = 11; /* max("rexx.exe", "regina.exe") */
      else
      {
         len = strlen(argv0) + 2;
         if (len < 11)
            len = 11; /* max("rexx.exe", "regina.exe") */
      }
      len += strlen(cmdline) + 2; /* Blank + term ASCII0 */

      if ((new_cmdline = malloc(len)) == NULL)
         return(-1); /* ENOMEM is set */

      if (argv0 != NULL) /* always the best choice */
      {
         strcpy(new_cmdline, "\"");
         strcat(new_cmdline, argv0);
         strcat(new_cmdline, "\" ");
         strcat(new_cmdline, cmdline);
         e.subtype = SUBENVIR_COMMAND;
         rc = fork_exec(TSD, &e, new_cmdline, &rc);
         if ( ( rc != 0 ) && ( rc != -1 ) )
         {
            free(new_cmdline);
            return(rc);
         }
      }

      /* load an interpreter by name from the path */
      for (i = 0; i < sizeof(interpreter) / sizeof(interpreter[0]);i++)
      {
         strcpy(new_cmdline, interpreter[i]);
         strcat(new_cmdline, " ");
         strcat(new_cmdline, cmdline);
         e.subtype = SUBENVIR_COMMAND;
         rc = fork_exec(TSD, &e, new_cmdline, &rc);
         if ( ( rc != 0 ) && ( rc != -1 ) )
         {
            free(new_cmdline);
            return(rc);
         }
      }

      *rcode = -errno; /* assume a load error */
      free(new_cmdline);
      return(0);
   }

   memset(&sinfo, 0, sizeof(sinfo));
   sinfo.cb = sizeof(sinfo);

   sinfo.dwFlags = STARTF_USESTDHANDLES;

   /* The following three handles have been created inheritable */
   if (env->input.hdls[0] != -1)
   {
      if ( WIN9X_VER )
      {
         sinfo.hStdInput  = (HANDLE) _get_osfhandle(env->input.hdls[0]);
         /*
          * fixes Bug 587687
          * We must ensure the called process already *uses* the handles;
          * they are closed by the OS in the other case. One chance is to
          * delay the execution until this happens, the other chance is to
          * wait with the closing until the called process works or
          * terminates. We use the latter one. For more info see MSDN.
          * Topics: Q190351, *Q150956*
          * I think, Q150956 is a workaround for the bug they didn't solve.
          * M$ just close the handles too early.
          */
         DuplicateHandle( GetCurrentProcess(),
                          sinfo.hStdInput,
                          GetCurrentProcess(),
                          &sinfo.hStdInput,
                          0,
                          TRUE,
                          DUPLICATE_SAME_ACCESS );
         /*
          * fixes bug 700405
          */
         env->input.hdls[2] = (int) sinfo.hStdInput;
      }
      else
         sinfo.hStdInput  = (HANDLE) env->input.hdls[0];
   }
   else
      sinfo.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
   if (env->output.hdls[1] != -1)
   {
      if ( WIN9X_VER )
      {
         sinfo.hStdOutput = (HANDLE) _get_osfhandle(env->output.hdls[1]);
         DuplicateHandle( GetCurrentProcess(),
                          sinfo.hStdOutput,
                          GetCurrentProcess(),
                          &sinfo.hStdOutput,
                          0,
                          TRUE,
                          DUPLICATE_SAME_ACCESS );
         /*
          * fixes bug 700405
          */
         env->output.hdls[2] = (int) sinfo.hStdOutput;
      }
      else
         sinfo.hStdOutput = (HANDLE) env->output.hdls[1];
   }
   else
      sinfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
   if (env->error.SameAsOutput)
      sinfo.hStdError  = (HANDLE) sinfo.hStdOutput;
   else if (env->error.hdls[1] != -1)
   {
      if ( WIN9X_VER )
      {
         sinfo.hStdError  = (HANDLE) _get_osfhandle(env->error.hdls[1]);
         DuplicateHandle( GetCurrentProcess(),
                          sinfo.hStdError,
                          GetCurrentProcess(),
                          &sinfo.hStdError,
                          0,
                          TRUE,
                          DUPLICATE_SAME_ACCESS );
         /*
          * fixes bug 700405
          */
         env->error.hdls[2] = (int) sinfo.hStdError;
      }
      else
         sinfo.hStdError  = (HANDLE) env->error.hdls[1];
   }
   else
      sinfo.hStdError  = GetStdHandle(STD_ERROR_HANDLE);

   /*
    * If the BROKEN_ADDRESS_COMMAND OPTION is in place,
    * and our environment is COMMAND, change it to SYSTEM
    */
   if ( env->subtype == SUBENVIR_PATH /* was SUBENVIR_COMMAND */
   &&   broken_address_command )
      subtype = SUBENVIR_SYSTEM;
   else
      subtype = env->subtype;

   switch ( subtype )
   {
      case SUBENVIR_PATH:
         execname = NULL;
         commandline = cmdline;
         break;

      case SUBENVIR_COMMAND:
         execname = NULL;
         commandline = cmdline;
#define NEED_SPLITOFFARG
         execname = splitoffarg(cmdline, NULL, '^');
         commandline = cmdline;
         break;

      case SUBENVIR_SYSTEM:
      /* insert "%COMSPEC% /c " or "%SHELL% -c " in front */
         if ((done = GetEnvironmentVariable("COMSPEC","",0)) != 0)
         {
            argline = MallocTSD(done + strlen(cmdline) + 5);
            GetEnvironmentVariable("COMSPEC",argline,done + 5);
            strcat(argline," /c ");
         }
         else if ((done = GetEnvironmentVariable("SHELL","",0)) != 0)
         {
            argline = MallocTSD(done + strlen(cmdline) + 5);
            GetEnvironmentVariable("SHELL",argline,done + 5);
            strcat(argline," -c ");
         }
         else
         {
            argline = MallocTSD(11 + strlen(cmdline) + 5);
            if (GetVersion() & 0x80000000) /* not NT ? */
               strcpy(argline,"COMMAND.COM /c ");
            else
               strcpy(argline,"CMD.EXE /c ");
         }
         strcat(argline, cmdline);
         execname = NULL;
         commandline = argline;
         break;

      case SUBENVIR_REXX:
         /* fall through */

      default: /* illegal subtype */
         errno = EINVAL;
         return -1;
   }

   /*
    * FGC: I checked different configurations.
    * 1) Never use DETACHED_PROCESS, the communication gets lost in NT
    *    kernels to a text program invoked using shells.
    * 2) Never use STARTF_USESHOWWINDOW/SW_HIDE in Win9x, this gives problems
    *    under Win9x or you have to know which combination fits to COMSPEC and
    *    your called program's type.
    * --> We can fiddle with CREATE_NEW_CONSOLE, CREATE_NEW_PROCESS_GROUP,
    *     CREATE_DEFAULT_ERROR_MODE. Select your choice!
    */
   if ( !WIN9X_VER )
   {
      sinfo.dwFlags |= STARTF_USESHOWWINDOW;
      sinfo.wShowWindow = SW_HIDE;
   }
   rc = CreateProcess(execname,
                      (char *) commandline,
                      NULL,        /* pointer to process security attributes */
                      NULL,        /* pointer to thread security attributes  */
                      TRUE,        /* no inheritance except sinfo.hStd...    */
                      CREATE_NEW_PROCESS_GROUP | CREATE_DEFAULT_ERROR_MODE,
                      NULL,        /* pointer to new environment block       */
                      NULL,        /* pointer to current directory name      */
                      &sinfo,
                      &pinfo);
   *rcode = (int) GetLastError();
   if (argline)
      FreeTSD(argline);
   if (execname)
      free(execname);
   if ( !rc )
      return 0;

   CloseHandle(pinfo.hThread); /* We don't need it */
   return (int) pinfo.hProcess;

   /* NT and W9x share the same functionality but we want to compile the */
   /* DOS-part below. Don't set this code to top */
# undef fork_exec
# define fork_exec fork_exec_dos
}

/* __regina_wait waits for a process started by fork_exec.
 * In general, this is called after the complete IO to the called process but
 * it isn't guaranteed. Never call if you don't expect a sudden death of the
 * subprocess.
 * Returns the exit status of the subprocess under normal circumstances. If
 * the subprocess has died by a signal, the return value is -signalnumber.
 */
int __regina_wait(int process)
{
   DWORD code = (DWORD) -1; /* in case something goes wrong */

   /* NT and W9x share the same functionality but we want to compile*/
# undef __regina_wait
# define __regina_wait __regina_wait_dos

   WaitForSingleObject((HANDLE) process, INFINITE);
   GetExitCodeProcess((HANDLE) process, &code);
   CloseHandle((HANDLE) process);

   if ((code & 0xC0000000) == 0xC0000000) /* assume signal */
      return -(int)(code & ~0xC0000000);
   return (int) code;
}

/* open_subprocess_connection acts like the unix-known function pipe and sets
 * ep->RedirectedFile if necessary. Just in the latter case ep->data
 * is set to the filename.
 * Close the handles with __regina_close later.
 * Do IO by using __regina_read() and __regina_write().
 */
int open_subprocess_connection(const tsd_t *TSD, environpart *ep)
{
#define MAGIC_MAX 2000000 /* Much beyond maximum, see below */
   static volatile unsigned BaseIndex = MAGIC_MAX;
   char buf[40];
   unsigned i;
   unsigned start,run;
   DWORD openmode, err;
   HANDLE in, out;
   OVERLAPPED ol;
   SECURITY_ATTRIBUTES sa;

# define NEED_STUPID_DOSCMD
# undef open_subprocess_connection
# define open_subprocess_connection open_subprocess_connection_dos
   if ( WIN9X_VER )
      return(open_subprocess_connection(TSD, ep));

   in = INVALID_HANDLE_VALUE;

   /* Anonymous pipes can't be run in overlapped mode. Therefore we use
    * named pipes (and files for W9x).
    */
   sa.nLength = sizeof(sa);
   sa.lpSecurityDescriptor = NULL;
   sa.bInheritHandle = TRUE;
   openmode = (ep->flags.isinput) ? PIPE_ACCESS_OUTBOUND : PIPE_ACCESS_INBOUND;
   openmode |= FILE_FLAG_OVERLAPPED;

   /* algorithm:
    * select a random number, e.g. a mixture of pid, tid and time.
    * increment this number by a fixed amount until we reach the starting
    * value again. Do a wrap around and an increment which is a unique prime.
    * The number 1000000 has the primes 2 and 5. We may use all primes
    * except 2 and 5; 9901 (which is prime) will give a good distribution.
    *
    * We want to be able to create several temporary files at once without
    * more OS calls than needed. Thus, we have a program wide runner to
    * ensure a simple distribution without strength.
    */
   if (BaseIndex == MAGIC_MAX)
   {
      /*
       * We have to create (nearly) reentrant code.
       */
      i = (unsigned) getpid() * (unsigned) time(NULL);
      i %= 1000000;
      if (!i)
         i = 1;
      BaseIndex = i;
   }
   if (++BaseIndex >= 1000000)
      BaseIndex = 1;

   start = TSD->thread_id;
   if (start == 0)
      start = 999999;
   start *= (unsigned) (clock() + 1);
   start *= BaseIndex;
   start %= 1000000;

   run = start;
   for (i = 0;i <= 1000000;i++)
   {
      sprintf(buf,"%s%06u._rx", "\\\\.\\pipe\\tmp\\", run );
      in = CreateNamedPipe(buf,
                           openmode,
                           PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                           1, /* just this instance! */
                           0, /* OutBufferSize: use system minimum, e.g.4K */
                           0, /* InBufferSize: use system minimum, e.g.4K */
                           0, /* nDefaultTimeout */
                           NULL); /* This handle won't be shared */
      if (in != INVALID_HANDLE_VALUE)
         break;

      err = GetLastError();
      if (err != ERROR_ACCESS_DENIED)
      {
         errno = EPIPE;
         return(-1);
      }

      /* Check the next possible candidate */
      run += 9901;
      run %= 1000000;
      if (run == start) /* paranoia check. i <= 1000000 should hit exactly */
         break;         /* here */
   }

   if (in == INVALID_HANDLE_VALUE)
   {
      errno = EPIPE;
      return(-1);
   }

   /* Prepare this pipe and then try to connect it to ourself. */
   if ((ol.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL)) == NULL)
   {
      CloseHandle(in);
      errno = EPIPE;
      return(-1);
   }
   ConnectNamedPipe(in, &ol); /* ignore the return */

   out = CreateFile(buf,
                    (ep->flags.isinput) ? GENERIC_READ : GENERIC_WRITE,
                    0, /* dwShareMode */
                    &sa,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, /* not OVERLAPPED! */
                    NULL); /* hTemplateFile */
   if (out == INVALID_HANDLE_VALUE)
   {
      MyCancelIo(in);
      CloseHandle(ol.hEvent);
      CloseHandle(in);
      errno = EPIPE; /* guess */
      return(-1);
   }

   /* Now do the final checking, the server end must be connected */
   if (!GetOverlappedResult(in, &ol, &openmode /* dummy */, FALSE))
   {
      CloseHandle(out);
      MyCancelIo(in);
      CloseHandle(ol.hEvent);
      CloseHandle(in);
      errno = EPIPE; /* guess */
      return(-1);
   }
   CloseHandle(ol.hEvent);

   /* We always want to have the server's end of the named pipe: */
   if (ep->flags.isinput)
   {
      ep->hdls[0] = (int) out;
      ep->hdls[1] = (int) in;
   }
   else
   {
      ep->hdls[0] = (int) in;
      ep->hdls[1] = (int) out;
   }
   return(0);
#undef MAGIC_MAX
}

/* sets the given handle to the non-blocking mode. The value may change.
 * async_info CAN be used to add the handle to the internal list of watched
 * handles.
 */
void unblock_handle( int *handle, void *async_info )
{
   AsyncInfo *ai;
   HANDLE hdl;
   unsigned i;

# define NEED_STUPID_DOSCMD
# undef unblock_handle
# define unblock_handle unblock_handle_dos
   if ( WIN9X_VER )
   {
      unblock_handle(handle, async_info);
      return;
   }

   ai = async_info;
   hdl = (HANDLE) *handle;

   assert(async_info != NULL);
   for (i = 0; i < 3;i++)
      if (ai->h[i].hdl == INVALID_HANDLE_VALUE)
         break;
   assert(i < 3);
   ai->h[i].hdl = hdl;
   ai->h[i].ol.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
}

/* sets the file pointer of the given handle to the beginning.
 */
void restart_file(int hdl)
{
# define NEED_STUPID_DOSCMD
# undef restart_file
# define restart_file restart_file_dos
   if ( WIN9X_VER )
   {
      restart_file(hdl);
      return;
   }

   assert((HANDLE) hdl == INVALID_HANDLE_VALUE);
}

/* __regina_close acts like close() but closes a handle returned by
 * open_subprocess_connection.
 * async_info MAY be used to delete the handle from the internal list of
 * watched handles.
 */
int __regina_close(int handle, void *async_info)
{
   AsyncInfo *ai;
   HANDLE hdl;
   unsigned i;

# define NEED_STUPID_DOSCMD
# define __regina_close __regina_close_dos
   if ( WIN9X_VER )
      return(__regina_close_dos(handle, async_info));

   ai = async_info;
   hdl = (HANDLE) handle;

   if (hdl == INVALID_HANDLE_VALUE)
   {
      errno = EINVAL;
      return(-1);
   }

   if (ai == NULL) /* unblocked handle? */
   {
      if (CloseHandle(hdl))
         return(0);
      errno = ENOSPC; /* guess */
      return(-1);
   }

   for (i = 0; i < 3;i++)
      if (ai->h[i].hdl == hdl)
         break;

   if (i == 3)
   {
      CloseHandle(hdl);
      errno = EINVAL;
      return(-1);
   }

   MyCancelIo(hdl);
   CloseHandle(ai->h[i].ol.hEvent);
   ai->h[i].hdl = INVALID_HANDLE_VALUE;
   if (ai->h[i].buf)
      Free_TSD(ai->TSD, ai->h[i].buf);
   if (CloseHandle(hdl))
      return(0);
   errno = ENOSPC; /* guess */
   return(-1);
}

/*
 * __regina_close_special acts like close() but closes any OS specific handle.
 * The close happens if the handle is not -1. A specific operation may be
 * associated with this. Have a look for occurances of "hdls[2]".
 */
void __regina_close_special( int handle )
{
   /*
    * DOS part not needed but we will get it. Rename the function.
    */
#define __regina_close_special __regina_close_special_dos
   if ( handle )
      CloseHandle( (HANDLE) handle );
}

/* __regina_read acts like read() but returns either an error (return code
 * == -errno) or the number of bytes read. EINTR and friends leads to a
 * re-read.
 * async_info is both a structure and a flag. If set, asynchronous IO shall
 * be used, otherwise blocked IO has to be used.
 */
int __regina_read(int handle, void *buf, unsigned size, void *async_info)
{
   AsyncInfo *ai;
   HANDLE hdl;
   unsigned i;
   OVERLAPPED *ol;
   DWORD done;
   int retval;

# define NEED_STUPID_DOSCMD
# define __regina_read __regina_read_dos
   if ( WIN9X_VER )
      return(__regina_read(handle, buf, size, async_info));

   ai = async_info;
   hdl = (HANDLE) handle;
   retval = 0;

   if (ai == NULL)
   {
      if (!ReadFile( (HANDLE) hdl, buf, size, &done, NULL))
      {
         retval = (int) GetLastError();
         if (retval == ERROR_BROKEN_PIPE)
            return(0); /* "Normal" EOF */
         return(-EPIPE); /* guess */
      }
      retval = (int) done;
      return(retval);
   }

   /* Async IO */
   for (i = 0;i < 3;i++)
      if (ai->h[i].hdl == hdl)
         break;
   if (i == 3)
      return(-EINVAL);

   ol = &ai->h[i].ol;

   if (ai->h[i].reading) /* pending IO? */
   {
      if (!HasOverlappedIoCompleted(ol))
      {
         done = 0;
         retval = -EAGAIN;
      }
      else if (!GetOverlappedResult(hdl, ol, &done, FALSE))
      {
         done = 0;
         if (GetLastError() == ERROR_IO_PENDING)
         {
            retval = -EAGAIN;
         }
         else
         {
            ai->h[i].reading = 0;
            retval = -EPIPE;
         }
      }
      else
         ai->h[i].reading = 0;

      if (done)
         ai->h[i].rused += done;
   }

   if (ai->h[i].rused)
   {
      if (size > ai->h[i].rused)
         size = ai->h[i].rused;
      memcpy(buf, ai->h[i].buf + ai->h[i].rusedbegin, size);
      ai->h[i].rusedbegin += size;
      ai->h[i].rused -= size;
      retval = size;
   }
   else
      retval = -EAGAIN; /* still may change! */

   if (ai->h[i].maxbuf < 0x1000) /* Buffer not allocated? */
   {
      /* Never allocate too much, we want a fast response to do some
       * work!
       */
      if (ai->h[i].buf) /* THIS IS DEFINITELY A BUG! */
         Free_TSD(ai->TSD, ai->h[i].buf);
      ai->h[i].maxbuf = 0x1000;
      ai->h[i].rusedbegin = 0;
      ai->h[i].rused = 0;
      ai->h[i].reading = 0;
      ai->h[i].buf = Malloc_TSD(ai->TSD, ai->h[i].maxbuf);
   }

   if (ai->h[i].reading) /* Pending IO, we can't do more */
      return(retval);

   /* not reading or no longer reading, initiate a new reading */
   if (ai->h[i].rused && (ai->h[i].rusedbegin != 0))
      memmove(ai->h[i].buf, ai->h[i].buf + ai->h[i].rusedbegin, ai->h[i].rused);
   ai->h[i].rusedbegin = 0;

   if (ai->h[i].rused < ai->h[i].maxbuf)
   {
      ai->h[i].reading = ai->h[i].maxbuf - ai->h[i].rused;
      ResetEvent(ai->h[i].ol.hEvent);
      if (!ReadFile((HANDLE) hdl,
                    ai->h[i].buf + ai->h[i].rused,
                    ai->h[i].reading,
                    &done,
                    ol))
      {
         done = GetLastError();
         if (done == ERROR_IO_PENDING)
            return(retval);

         ai->h[i].reading = 0;
         if ((retval > 0) || (ai->h[i].rused != 0))
            return(retval);

         if (done == ERROR_BROKEN_PIPE)
            return(0); /* "Normal" EOF */
         return(-EPIPE); /* guess */
      }

      /* success */
      ai->h[i].reading = 0;
      ai->h[i].rused += done;
   }

   if ((retval < 0) && ai->h[i].rused) /* fresh data? return at once */
   {
      if (size > ai->h[i].rused)
         size = ai->h[i].rused;
      memcpy(buf, ai->h[i].buf + ai->h[i].rusedbegin, size);
      ai->h[i].rusedbegin += size;
      ai->h[i].rused -= size;
      retval = size;
   }
   return(retval);
}

/* __regina_write acts like write() but returns either an error (return code
 * == -errno) or the number of bytes written. EINTR and friends leads to a
 * re-write.
 * async_info is both a structure and a flag. If set, asynchronous IO shall
 * be used, otherwise blocked IO has to be used.
 * The file must be flushed if both buf and size are 0.
 */
int __regina_write(int handle, const void *buf, unsigned size, void *async_info)
{
   AsyncInfo *ai;
   HANDLE hdl;
   unsigned i;
   OVERLAPPED *ol;
   DWORD done;
   int retval;

# define NEED_STUPID_DOSCMD
# define __regina_write __regina_write_dos
   if ( WIN9X_VER )
      return(__regina_write(handle, buf, size, async_info));

   ai = async_info;
   hdl = (HANDLE) handle;
   retval = 0;

   if (!ai)
   {
      if (buf == NULL)
         return(0); /* flushing is useless here! */

      if (!WriteFile(hdl, buf, size, &done, NULL))
      {
         retval = (int) GetLastError();
         return(-EPIPE); /* guess */
      }
      retval = (int) done;
      return(retval);
   }

   /* Async IO */
   for (i = 0;i < 3;i++)
      if (ai->h[i].hdl == hdl)
         break;
   if (i == 3)
      return(-EINVAL);

   ol = &ai->h[i].ol;

   if (buf == NULL)
   {
      if (ai->h[i].wused) /* pending IO? */
      {
         if (!GetOverlappedResult(hdl, ol, &done, TRUE))
            return(-EPIPE); /* guess */
         if (done != ai->h[i].wused)
            return(-EPIPE); /* guess */
      }
      return(0); /* OK */
   }

   if (ai->h[i].wused) /* pending IO? */
   {
      if (!HasOverlappedIoCompleted(ol))
         return(-EAGAIN);
      if (!GetOverlappedResult(hdl, ol, &done, FALSE))
      {
         done = GetLastError();
         if (done == ERROR_IO_PENDING)
            return(-EAGAIN);
         return(-EPIPE); /* guess */
      }
      if (done < ai->h[i].wused)
      {
         memmove(ai->h[i].buf, ai->h[i].buf + done, ai->h[i].wused - done);
         ai->h[i].wused -= done;
      }
      else
         ai->h[i].wused = 0;
   }

   if (ai->h[i].wused < 0x10000) /* Never buffer too much at once! */
   {
      /* We have to add the argumented stuff to the local buffer */
      if (ai->h[i].wused + size >= ai->h[i].maxbuf)
      {
         char *new;

         ai->h[i].maxbuf = ai->h[i].wused + size + 0x400;
         if (ai->h[i].maxbuf < 0x2000) /* initial minimum value */
            ai->h[i].maxbuf = 0x2000;
         new = Malloc_TSD(ai->TSD, ai->h[i].maxbuf);
         if (ai->h[i].wused)
            memcpy(new, ai->h[i].buf, ai->h[i].wused);
         if (ai->h[i].buf)
            Free_TSD(ai->TSD, ai->h[i].buf);

         ai->h[i].buf = new;
      }
      memmove(ai->h[i].buf + ai->h[i].wused, buf, size);
      ai->h[i].wused += size;
      retval = size;
   }
   else
      retval = -EAGAIN;

   if (ai->h[i].wused == 0)
      return(retval);

   ResetEvent(ai->h[i].ol.hEvent);

   if (!WriteFile(hdl, ai->h[i].buf, ai->h[i].wused, &done, ol))
   {
      done = (int) GetLastError();
      if (done == ERROR_IO_PENDING)
         return(retval);
      return(-EPIPE); /* guess */
   }
   /* No errors, thus success. We don't want to redo all the stuff. We
    * simply set your wait-semaphore to prevent sleeping.
    */
   SetEvent(ai->h[i].ol.hEvent);

   return(retval);
}

/* create_async_info return an opaque structure to allow the process wait for
 * asyncronous IO. There are three IO slots (in, out, error) which can be
 * filled by add_waiter. The structure can be cleaned by reset_async_info.
 * The structure must be destroyed by delete_async_info.
 */
void *create_async_info(const tsd_t *TSD)
{
   AsyncInfo *retval;

# define NEED_STUPID_DOSCMD
# undef create_async_info
# define create_async_info create_async_info_dos
   if ( WIN9X_VER )
      return(create_async_info(TSD));

   retval = MallocTSD(sizeof(AsyncInfo));
   memset(retval, 0, sizeof(AsyncInfo));

   retval->TSD = TSD;
   retval->h[0].hdl = INVALID_HANDLE_VALUE;
   retval->h[0].ol.hEvent = INVALID_HANDLE_VALUE;
   retval->h[1] = retval->h[0];
   retval->h[2] = retval->h[0];
   return(retval);
}

/* delete_async_info deletes the structure created by create_async_info and
 * all of its components.
 */
void delete_async_info(void *async_info)
{
   AsyncInfo *ai;
   unsigned i;

# define NEED_STUPID_DOSCMD
# undef delete_async_info
# define delete_async_info delete_async_info_dos
   if ( WIN9X_VER )
   {
      delete_async_info(async_info);
      return;
   }

   ai = async_info;
   if (ai == NULL)
      return;

   for (i = 0; i < 3;i++)
      __regina_close((int) ai->h[i].hdl, ai);
   Free_TSD(ai->TSD, ai);
}

/* reset_async_info clear async_info in such a way that fresh add_waiter()
 * calls can be performed.
 */
void reset_async_info(void *async_info)
{
# define NEED_STUPID_DOSCMD
# undef reset_async_info
# define reset_async_info reset_async_info_dos
   if ( WIN9X_VER )
   {
      reset_async_info(async_info);
      return;
   }
}

/* add_async_waiter adds a further handle to the asyncronous IO structure.
 * add_as_read_handle must be != 0 if the next operation shall be a
 * __regina_read, else it must be 0 for __regina_write.
 * Call reset_async_info before a wait-cycle to different handles and use
 * wait_async_info to wait for at least one IO-able handle.
 */
void add_async_waiter(void *async_info, int handle, int add_as_read_handle)
{
   AsyncInfo *ai;
   HANDLE hdl;
   unsigned i;

# define NEED_STUPID_DOSCMD
# undef add_async_waiter
# define add_async_waiter add_async_waiter_dos
   if ( WIN9X_VER )
   {
      add_async_waiter(async_info, handle, add_as_read_handle);
      return;
   }

   ai = async_info;
   hdl = (HANDLE) handle;
   if (ai)
   {
      for (i = 0;i < 3;i++)
         if (ai->h[i].hdl == hdl)
         {
            if (add_as_read_handle)
               ai->h[i].is_reader = 1;
            else
               ai->h[i].is_reader = 0;
         }
   }
}

/* wait_async_info waits for some handles to become ready. This function
 * returns if at least one handle becomes ready.
 * A handle can be added with add_async_waiter to the bundle of handles to
 * wait for.
 * No special handling is implemented if an asyncronous interrupt occurs.
 * Thus, there is no guarantee to have handle which works.
 */
void wait_async_info(void *async_info)
{
   AsyncInfo *ai;
   unsigned i, used;
   HANDLE list[3];

# define NEED_STUPID_DOSCMD
# undef wait_async_info
# define wait_async_info wait_async_info_dos
   if ( WIN9X_VER )
   {
      wait_async_info(async_info);
      return;
   }

   ai = async_info;
   for (i = 0, used = 0; i < 3; i++)
   {
      if (ai->h[i].hdl == INVALID_HANDLE_VALUE)
         continue;
      if (ai->h[i].ol.hEvent == INVALID_HANDLE_VALUE)
         continue;
      if (ai->h[i].is_reader)
      {
         if (ai->h[i].rused != 0) /* Still data to read? this is a killer! */
            return;
         if (ai->h[i].reading == 0)
            continue;
      }
      else
      {
         if (ai->h[i].wused == 0)
            continue;
      }
      if (HasOverlappedIoCompleted(&ai->h[i].ol)) /* fresh meat arrived? */
         return;
      list[used++] = ai->h[i].ol.hEvent;
   }
   if (used)
      WaitForMultipleObjects(used, list, FALSE, INFINITE);
}
#elif defined(__EMX__) || defined(OS2) /* until here not WIN32 */
/*****************************************************************************
 *****************************************************************************
 ** EMX **********************************************************************
 *****************************************************************************
 *****************************************************************************/
#include <io.h>

  typedef struct {
   const tsd_t *TSD;
   HEV          sem;
   int          mustwait;
   int          hdl[3];
  } AsyncInfo;

/* fork_exec spawns a new process with the given commandline.
 * it returns -1 on error (errno set), 0 on process start error (rcode set),
 * a process descriptor otherwise.
 * Basically this is a child process and we run in the child's environment
 * after the first few lines. The setup hasn't been done and the command needs
 * to be started.
 * Redirection takes place if one of the handles env->input.hdls[0],
 * env->output.hdls[1] or env->error.hdls[1] is defined. Other handles (except
 * standard handles) are closed. env->subtype must be a SUBENVIR_... constant.
 * cmdline is the whole command line.
 * Never use TSD after the fork() since this is not only a different thread,
 * it's a different process!
 * Although mentioned in the documentation we have to use a backslash a
 * escape character nevertheless. It's a bug of EMX not to recognize a
 * circumflex as the default escape character and therefore we have to
 * use the "wrong" escape character. Note the difference between EMX and OS/2.
 * Maybe, I'm wrong. Drop me an email in this case. FGC
 */
int fork_exec(tsd_t *TSD, environment *env, const char *cmdline, int *rcode)
{
   static const char *interpreter[] = { "regina.exe", /* preferable even if */
                                                      /* not dynamic        */
                                        "rexx.exe" };
   char **args = NULL;
   int saved_in = -1, saved_out = -1, saved_err = -1;
   int rc;
   const char *ipret;
   char *argline;
   int broken_address_command = get_options_flag( TSD->currlevel, EXT_BROKEN_ADDRESS_COMMAND );
   int subtype;

   if (env->subtype == SUBENVIR_REXX) /*special situation caused by recursion*/
   {
      environment e = *env;
      char *new_cmdline;
      int i, rc;
      unsigned len;

      if (argv0 == NULL)
         len = 11; /* max("rexx.exe", "regina.exe") */
      else
      {
         len = strlen(argv0);
         if (len < 11)
            len = 11; /* max("rexx.exe", "regina.exe") */
      }
      len += strlen(cmdline) + 2; /* Blank + term ASCII0 */

      if ((new_cmdline = malloc(len)) == NULL)
         return(-1); /* ENOMEM is set */

      if (argv0 != NULL) /* always the best choice */
      {
         strcpy(new_cmdline, argv0);
         strcat(new_cmdline, " ");
         strcat(new_cmdline, cmdline);
         e.subtype = SUBENVIR_COMMAND;
         rc = fork_exec(TSD, &e, new_cmdline, &rc);
         if ( ( rc != 0 ) && ( rc != -1 ) )
         {
            free(new_cmdline);
            return(rc);
         }
      }

      /* load an interpreter by name from the path */
      for (i = 0; i < sizeof(interpreter) / sizeof(interpreter[0]);i++)
      {
         strcpy(new_cmdline, interpreter[i]);
         strcat(new_cmdline, " ");
         strcat(new_cmdline, cmdline);
         e.subtype = SUBENVIR_COMMAND;
         rc = fork_exec(TSD, &e, new_cmdline, &rc);
         if ( ( rc != 0 ) && ( rc != -1 ) )
         {
            free(new_cmdline);
            return(rc);
         }
      }

#ifndef __EMX__
      *rcode = -errno; /* assume a load error */
      free( new_cmdline );
      return 0;
#else
      if ( ( rc = fork() ) != 0 ) /* EMX is fork-capable */
         return rc;
#endif
   }
#ifdef __EMX__ /* redirect this call to the non-OS/2-code if DOS is running */
# define NEED_STUPID_DOSCMD
   /* SUBENVIR_REXX must(!!) fork if we are here! */
# undef fork_exec
# define fork_exec __regina_fork_exec_dos
   {
      int fork_exec(tsd_t *TSD, environment *env, const char *cmdline);
      if ((_osmode != OS2_MODE) && (env->subtype != SUBENVIR_REXX))
      return(fork_exec(TSD, env, cmdline));
   }
#endif

#define STD_REDIR(hdl,dest,save) if ((hdl != -1) && (hdl != dest)) \
                                    { save = dup(dest); dup2(hdl, dest); }
#define STD_RESTORE(saved,dest) if (saved != -1) \
                                    { close(dest); dup2(saved,dest); \
                                      close(saved); }
#define SET_MAXHDL(hdl) if (hdl > max_hdls) max_hdls = hdl
#define SET_MAXHDLS(ep) SET_MAXHDL(ep.hdls[0]); SET_MAXHDL(ep.hdls[1])

                                        /* Force the standard redirections:  */
   STD_REDIR(env->input.hdls[0],    0, saved_in);
   STD_REDIR(env->output.hdls[1],   1, saved_out);
   if (env->error.SameAsOutput)
   {
      saved_err = dup(2);
      dup2(1, 2);
   }
   else
   {
      STD_REDIR(env->error.hdls[1], 2, saved_err);
   }

   /*
    * If the BROKEN_ADDRESS_COMMAND OPTION is in place,
    * and our environment is COMMAND, change it to SYSTEM
    */
   if ( env->subtype == SUBENVIR_PATH /* was SUBENVIR_COMMAND */
   &&   broken_address_command )
      subtype = SUBENVIR_SYSTEM;
   else
      subtype = env->subtype;

   rc = -1;
   switch ( subtype )
   {
      case SUBENVIR_PATH:
         args = makeargs(cmdline, '^');
#define NEED_MAKEARGS
         rc = spawnvp(P_NOWAIT, *args, args);
         break;

      case SUBENVIR_COMMAND:
         args = makeargs(cmdline, '^');
         rc = spawnv(P_NOWAIT, *args, args);
         break;

      case SUBENVIR_SYSTEM:
         /* insert "%COMSPEC% /c " or "%SHELL% -c " in front */
         if ((ipret = getenv("COMSPEC")) != NULL)
         {
            argline = MallocTSD(strlen(ipret) + strlen(cmdline) + 5);
            strcpy(argline,ipret);
            strcat(argline," /c ");
         }
         else if ((ipret = getenv("SHELL")) != NULL)
         {
            argline = MallocTSD(strlen(ipret) + strlen(cmdline) + 5);
            strcpy(argline,ipret);
            strcat(argline," -c ");
         }
         else
         {
            ipret = "CMD.EXE";
            argline = MallocTSD(strlen(ipret) + strlen(cmdline) + 5);
            strcpy(argline,ipret);
            strcat(argline," /c ");
         }
         strcat(argline, cmdline);
         args = makeargs(argline, '^');
         rc = spawnvp(P_NOWAIT, *args, args);
         break;

      case SUBENVIR_REXX:
         {
            /* we are forked and we are the child!!!! */
            /* last chance, worst choice, use the re-entering code: */
            char *new_cmdline = malloc(strlen(cmdline) + 4);
            char **run;
            int i;

            strcpy(new_cmdline, "\"\" ");
            strcat(new_cmdline, cmdline);
            args = makeargs(new_cmdline, '^');

            for (i = 0, run = args; *run; run++)
                  i++;
            exit(__regina_reexecute_main(i, args));
         }

      default: /* illegal subtype */
         STD_RESTORE(saved_in, 0);
         STD_RESTORE(saved_out, 1);
         STD_RESTORE(saved_err, 2);
         errno = EINVAL;
         return -1;
   }

   *rcode = errno;
   STD_RESTORE(saved_in, 0);
   STD_RESTORE(saved_out, 1);
   STD_RESTORE(saved_err, 2);
   if (args != NULL)
      destroyargs(args);
#define NEED_DESTROYARGS

   return ( rc == -1 ) ? 0 : rc;
#undef SET_MAXHDLS
#undef SET_MAXHDL
#undef STD_RESTORE
#undef STD_REDIR
}

/* __regina_wait waits for a process started by fork_exec.
 * In general, this is called after the complete IO to the called process but
 * it isn't guaranteed. Never call if you don't expect a sudden death of the
 * subprocess.
 * Returns the exit status of the subprocess under normal circumstances. If
 * the subprocess has died by a signal, the return value is -signalnumber.
 */
int __regina_wait(int process)
{
   int rc, retval, status;

#ifdef __EMX__ /* redirect this call to the non-OS/2-code if DOS is running */
# define NEED_STUPID_DOSCMD
# define __regina_wait __regina_wait_dos
   int __regina_wait(int process);
   if (_osmode != OS2_MODE)
      return(__regina_wait(process));
#endif

#ifdef __WATCOMC__
   /*
    * Watcom is strange. EINTR isn't an indicator for a retry of the call.
    */
   status = -1;
   rc = cwait(&status, process, WAIT_CHILD);
   if (rc == -1)
   {
      if ((status != -1) && (errno == EINTR))
         retval = -status; /* Exception reason in lower byte */
      else
         retval = -errno;  /* I don't have a better idea */
   }
   else
   {
      if (status & 0xFF)
         retval = -status; /* Exception reason in lower byte */
      else
         retval = status >> 8;
   }
#else
   do {
      rc = waitpid(process, &status, 0);
   } while ((rc == -1) && (errno == EINTR));

   if (WIFEXITED(status))
   {
      retval = (int) WEXITSTATUS(status);
      if ( retval < 0 )
         retval = -retval;
   }
   else if (WIFSIGNALED(status))
   {
      retval = -WTERMSIG(status);
      if ( retval > 0 )
         retval = -retval;
      else if ( retval == 0 )
         retval = -1;
   }
   else
   {
      retval = -WSTOPSIG(status);
      if ( retval > 0 )
         retval = -retval;
      else if ( retval == 0 )
         retval = -1;
   }
#endif

   return(retval);
}

/* open_subprocess_connection acts like the unix-known function pipe and sets
 * ep->RedirectedFile if necessary. Just in the latter case ep->data
 * is set to the filename.
 * Close the handles with __regina_close later.
 * Do IO by using __regina_read() and __regina_write().
 */
int open_subprocess_connection(const tsd_t *TSD, environpart *ep)
{
#define MAGIC_MAX 2000000 /* Much beyond maximum, see below */
   static volatile unsigned BaseIndex = MAGIC_MAX;
   char buf[40];
   unsigned i;
   unsigned start,run;
   ULONG rc, openmode, dummy;
   HPIPE in = (HPIPE) -1, out;

#ifdef __EMX__ /* redirect this call to the non-OS/2-code if DOS is running */
# define NEED_STUPID_DOSCMD
# undef open_subprocess_connection
# define open_subprocess_connection __regina_open_subprocess_connection_dos
   int open_subprocess_connection(const tsd_t *TSD, environpart *ep);
   if (_osmode != OS2_MODE)
      return(open_subprocess_connection(TSD, ep));
#endif

   /* We have to use named pipes for various reasons. */
   openmode = (ep->flags.isinput) ? NP_ACCESS_OUTBOUND : NP_ACCESS_INBOUND;
   openmode |= NP_NOINHERIT | NP_WRITEBEHIND;

   /* algorithm:
    * select a random number, e.g. a mixture of pid, tid and time.
    * increment this number by a fixed amount until we reach the starting
    * value again. Do a wrap around and an increment which is a unique prime.
    * The number 1000000 has the primes 2 and 5. We may use all primes
    * except 2 and 5; 9901 (which is prime) will give a good distribution.
    *
    * We want to be able to create several temporary files at once without
    * more OS calls than needed. Thus, we have a program wide runner to
    * ensure a simple distribution without strength.
    */
   if (BaseIndex == MAGIC_MAX)
   {
      /*
       * We have to create (nearly) reentrant code.
       */
      i = (unsigned) getpid() * (unsigned) time(NULL);
      i %= 1000000;
      if (!i)
         i = 1;
      BaseIndex = i;
   }
   if (++BaseIndex >= 1000000)
      BaseIndex = 1;

   start = TSD->thread_id;
   if (start == 0)
      start = 999999;
   start *= (unsigned) (clock() + 1);
   start *= BaseIndex;
   start %= 1000000;

   run = start;
   for (i = 0;i <= 1000000;i++)
   {
      sprintf(buf,"%s%06u._rx", "\\pipe\\tmp\\", run );
      rc = DosCreateNPipe(buf,
                          &in,
                          openmode,
                          NP_TYPE_BYTE | NP_READMODE_BYTE | NP_NOWAIT | 1,
                          4096,
                          4096,
                          0); /* msec timeout */
      if (rc == NO_ERROR)
         break;

      if (rc != ERROR_PIPE_BUSY)
      {
         errno = EPIPE;
         return(-1);
      }

      /* Check the next possible candidate */
      run += 9901;
      run %= 1000000;
      if (run == start) /* paranoia check. i <= 1000000 should hit exactly */
         break;         /* here */
   }

   if (in == (HPIPE) -1)
   {
      errno = EPIPE;
      return(-1);
   }

   DosConnectNPipe(in); /* ignore the return */

   openmode = (ep->flags.isinput) ? OPEN_ACCESS_READONLY :
                                    OPEN_ACCESS_WRITEONLY;
   openmode |= OPEN_FLAGS_SEQUENTIAL | OPEN_SHARE_DENYREADWRITE;
   rc = DosOpen(buf,
                &out,
                &dummy, /* action */
                0ul, /* initial size */
                FILE_NORMAL,
                OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                openmode,
                NULL); /* peaop2 */
   if (rc != NO_ERROR)
   {
      DosClose(in);
      errno = EPIPE; /* guess */
      return(-1);
   }

   /* Now do the final checking, the server end must be connected */
   if (DosConnectNPipe(in) != NO_ERROR)
   {
      DosClose(out);
      DosClose(in);
      errno = EPIPE; /* guess */
      return(-1);
   }

   /* We always want to have the server's end of the named pipe: */
   if (ep->flags.isinput)
   {
      ep->hdls[0] = (int) out;
      ep->hdls[1] = (int) in;
   }
   else
   {
      ep->hdls[0] = (int) in;
      ep->hdls[1] = (int) out;
   }
   return(0);
#undef MAGIC_MAX
}

/* sets the given handle to the non-blocking mode. The value may change.
 * async_info CAN be used to add the handle to the internal list of watched
 * handles.
 */
void unblock_handle( int *handle, void *async_info )
{
   AsyncInfo *ai = async_info;
   int i;
   ULONG rc;

#ifdef __EMX__ /* redirect this call to the non-OS/2-code if DOS is running */
# define NEED_STUPID_DOSCMD
# undef unblock_handle
# define unblock_handle __regina_unblock_handle_dos
   void unblock_handle( int *handle, void *async_info );
   if (_osmode != OS2_MODE)
   {
      unblock_handle(handle, async_info);
      return;
   }
#endif
   if (*handle == -1)
      return ;

   for (i = 0;i < 3;i++)
   {
      if ((ai->hdl[i] != *handle) && (ai->hdl[i] == -1))
      {
         ai->hdl[i] = *handle;
         rc = DosSetNPipeSem((HFILE) *handle, (HSEM) ai->sem, 0);

         if (rc != NO_ERROR)
            ai->hdl[i] = *handle = -2;
            /* This shall produce an error. -1 isn't a good idea: special
             * meaning
             */
         return;
      }
   }
}

/* restart_file sets the file pointer of the given handle to the beginning.
 */
void restart_file(int hdl)
{
#ifdef __EMX__ /* redirect this call to the non-OS/2-code if DOS is running */
# define NEED_STUPID_DOSCMD
# undef restart_file
# define restart_file __regina_restart_file_dos
   void restart_file(int hdl);
   if (_osmode != OS2_MODE)
   {
      restart_file(hdl);
      return;
   }
#endif

   lseek(hdl, 0l, SEEK_SET); /* unused! */
}

/* __regina_close acts like close() but closes a handle returned by
 * open_subprocess_connection.
 * async_info MAY be used to delete the handle from the internal list of
 * watched handles.
 */
int __regina_close(int handle, void *async_info)
{
   AsyncInfo *ai = async_info;
   int i;

#ifdef __EMX__ /* redirect this call to the non-OS/2-code if DOS is running */
# define NEED_STUPID_DOSCMD
# define __regina_close __regina_close_dos
   int __regina_close(int handle, void *async_info);
   if (_osmode != OS2_MODE)
      return(__regina_close(handle, async_info));
#endif

   if ((handle != -1) && (ai != NULL))
      for (i = 0;i < 3;i++)
         if ((int) ai->hdl[i] == handle)
            DosSetNPipeSem((HPIPE) handle, NULLHANDLE, 0);
   return(DosClose((HFILE) handle));
}

/*
 * __regina_close_special acts like close() but closes any OS specific handle.
 * The close happens if the handle is not -1. A specific operation may be
 * associated with this. Have a look for occurances of "hdls[2]".
 */
void __regina_close_special( int handle )
{
#define __regina_close_special __regina_close_special_dos
   assert( handle == -1 );
}

/* __regina_read acts like read() but returns either an error (return code
 * == -errno) or the number of bytes read. EINTR and friends leads to a
 * re-read.
 * use_blocked_io is a flag. If set, the handle is set to blocked IO and
 * we shall use blocked IO here.
 */
int __regina_read(int hdl, void *buf, unsigned size, void *async_info)
{
   ULONG rc;
   ULONG done;

#ifdef __EMX__ /* redirect this call to the non-OS/2-code if DOS is running */
# define NEED_STUPID_DOSCMD
# define __regina_read __regina_read_dos
   int __regina_read(int hdl, void *buf, unsigned size, void *async_info);
   if (_osmode != OS2_MODE)
      return(__regina_read(hdl, buf, size, async_info));
#endif

   do {
      rc = DosRead((HFILE) hdl, buf, size, &done);
   } while (rc == ERROR_INTERRUPT);

   if (rc != 0)
   {
      if (rc == ERROR_NO_DATA)
         return(-EAGAIN);
      if (rc == ERROR_BROKEN_PIPE)
         return(-EPIPE);
      return(-EINVAL);   /* good assumption */
   }

   return(done);
}

/* __regina_write acts like write() but returns either an error (return code
 * == -errno) or the number of bytes written. EINTR and friends leads to a
 * re-write.
 * use_blocked_io is a flag. If set, the handle is set to blocked IO and
 * we shall use blocked IO here.
 * The file must be flushed, if both buf and size are 0.
 */
int __regina_write(int hdl, const void *buf, unsigned size, void *async_info)
{
   ULONG rc;
   ULONG done;

#ifdef __EMX__ /* redirect this call to the non-OS/2-code if DOS is running */
# define NEED_STUPID_DOSCMD
# define __regina_write __regina_write_dos
   int __regina_write(int hdl, const void *buf, unsigned size, void *async_info);
   if (_osmode != OS2_MODE)
      return(__regina_write(hdl, buf, size, async_info));
#endif

   if ((buf == NULL) || (size == 0)) /* nothing to to for flushing buffers */
      return(0);

   do {
      rc = DosWrite((HFILE) hdl, buf, size, &done);
   } while (rc == ERROR_INTERRUPT);

   if (rc != 0)
   {
      if (rc == ERROR_NO_DATA)
         return(-EAGAIN);
      if ((rc == ERROR_BROKEN_PIPE) || (rc == ERROR_DISCARDED))
         return(-EPIPE);
      if (rc == ERROR_INVALID_HANDLE)
         return(-EINVAL);
      return(-ENOSPC);   /* good assumption */
   } else if (done == 0)
      return(-EAGAIN);

   return((int) done);
}

/* create_async_info return an opaque structure to allow the process wait for
 * asyncronous IO. There are three IO slots (in, out, error) which can be
 * filled by add_waiter. The structure can be cleaned by reset_async_info.
 * The structure must be destroyed by delete_async_info.
 */
void *create_async_info(const tsd_t *TSD)
{
   AsyncInfo *ai;

#ifdef __EMX__ /* redirect this call to the non-OS/2-code if DOS is running */
# define NEED_STUPID_DOSCMD
# undef create_async_info
# define create_async_info __regina_create_async_info_dos
   void *create_async_info(const tsd_t *TSD);
   if (_osmode != OS2_MODE)
      return(create_async_info(TSD));
#endif

   ai = MallocTSD(sizeof(AsyncInfo));
   ai->TSD = TSD;
   ai->sem = NULLHANDLE;
   ai->hdl[0] = ai->hdl[1] = ai->hdl[2] = (HFILE) -1;
   ai->mustwait = 0;
   DosCreateEventSem(NULL, &ai->sem, DC_SEM_SHARED, FALSE);
   return(ai);
}

/* delete_async_info deletes the structure created by create_async_info and
 * all of its components.
 */
void delete_async_info(void *async_info)
{
   AsyncInfo *ai = async_info;

#ifdef __EMX__ /* redirect this call to the non-OS/2-code if DOS is running */
# define NEED_STUPID_DOSCMD
# undef delete_async_info
# define delete_async_info __regina_delete_async_info_dos
   void delete_async_info(void *async_info);
   if (_osmode != OS2_MODE)
   {
      delete_async_info(async_info);
      return;
   }
#endif

   if (ai == NULL)
      return;
   DosCloseEventSem(ai->sem);
   Free_TSD(ai->TSD, ai);
}

/* reset_async_info clear async_info in such a way that fresh add_waiter()
 * calls can be performed.
 */
void reset_async_info(void *async_info)
{
   AsyncInfo *ai = async_info;
   ULONG ul;

#ifdef __EMX__ /* redirect this call to the non-OS/2-code if DOS is running */
# define NEED_STUPID_DOSCMD
# undef reset_async_info
# define reset_async_info __regina_reset_async_info_dos
   void reset_async_info(void *async_info);
   if (_osmode != OS2_MODE)
   {
      reset_async_info(async_info);
      return;
   }
#endif

   DosResetEventSem(ai->sem, &ul);
   ai->mustwait = 0;
}

/* add_async_waiter adds a further handle to the asyncronous IO structure.
 * add_as_read_handle must be != 0 if the next operation shall be a
 * __regina_read, else it must be 0 for __regina_write.
 * Call reset_async_info before a wait-cycle to different handles and use
 * wait_async_info to wait for at least one IO-able handle.
 */
void add_async_waiter(void *async_info, int handle, int add_as_read_handle)
{
   AsyncInfo *ai = async_info;
   int i;

#ifdef __EMX__ /* redirect this call to the non-OS/2-code if DOS is running */
# define NEED_STUPID_DOSCMD
# undef add_async_waiter
# define add_async_waiter __regina_add_async_waiter_dos
   void add_async_waiter(void *async_info, int handle, int add_as_read_handle);
   if (_osmode != OS2_MODE)
   {
      add_async_waiter(async_info, handle, add_as_read_handle);
      return;
   }
#endif

   if (handle != -1)
      for (i = 0;i < 3;i++)
         if ((int) ai->hdl[i] == handle)
            ai->mustwait = 1;
}

/* wait_async_info waits for some handles to become ready. This function
 * returns if at least one handle becomes ready.
 * A handle can be added with add_async_waiter to the bundle of handles to
 * wait for.
 * No special handling is implemented if an asyncronous interrupt occurs.
 * Thus, there is no guarantee to have handle which works.
 */
void wait_async_info(void *async_info)
{
   AsyncInfo *ai = async_info;

#ifdef __EMX__ /* redirect this call to the non-OS/2-code if DOS is running */
# define NEED_STUPID_DOSCMD
# undef wait_async_info
# define wait_async_info __regina_wait_async_info_dos
   void wait_async_info(void *async_info);
   if (_osmode != OS2_MODE)
   {
      wait_async_info(async_info);
      return;
   }
#endif

   if (ai->mustwait)
      DosWaitEventSem(ai->sem, SEM_INDEFINITE_WAIT);
}
/* end of elif define(__EMX__) */
#elif defined(MAC) || defined(DOS) || defined(__WINS__) || defined(__EPOC32__) || defined(_AMIGA) || defined(SKYOS)
#define NEED_STUPID_DOSCMD
#else /* !(MAC || DOS || WIN32 || OS2 || _AMIGA || __WINS__ || __EPOC32__) */
/*****************************************************************************
 *****************************************************************************
 ** unix and others **********************************************************
 *****************************************************************************
 *****************************************************************************/

#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#if defined(HAVE_ASSERT_H)
# include <assert.h>
#endif

#if (defined(__WATCOMC__) && !defined(__QNX__)) || defined(_MSC_VER) || defined(__MINGW32__) || defined(__BORLANDC__)
# include <process.h>
#else
# include <sys/wait.h>
# include <sys/time.h>
#endif
#if defined(HAVE_POLL_H)
# include <poll.h>
  /* implement a simple(!) wait mechanism for a max. of 3 handles */
  typedef struct {
   const tsd_t *TSD;
   struct pollfd _p[3] ;
   int _p_cnt ;
  } AsyncInfo;
#elif defined(HAVE_SYS_SELECT_H) || defined(SELECT_IN_TIME_H) || defined(HAVE_SYS_SOCKET_H)
# if defined(HAVE_SYS_SELECT_H)
#   include <sys/select.h>
# elif defined(HAVE_SYS_SOCKET_H)
#   include <sys/socket.h>
# else
#   include <time.h>
# endif
  /* implement a simple(!) wait mechanism for a max. of 3 handles */
  typedef struct {
   const tsd_t *TSD;
   fd_set _p_in;
   fd_set _p_out;
   int _p_max ;
  } AsyncInfo;
#endif
#if defined(HAVE_SYS_RESOURCE_H)
# include <sys/resource.h>
#endif
#define I_SLASH '/'

/* MaxFiles returns the maximum number of files which can be addressed by a
 * single process. We guess the result if we can't determine it.
 */
#if defined(__QNX__) && !defined(__QNXNTO__)
#include <sys/osinfo.h>
static int MaxFiles(void)
/*
 * returns the maximum number of files which can be addressed by a single
 * process. We guess the result if we can't determine it.
 */
{
   struct _osinfo osdata;
   if ( qnx_osinfo( 0, &osdata ) != -1 )
   {
      return osdata.num_fds[1];
   }
   else
   {
      return 256;
   }
}
#else
static int MaxFiles(void)
{
   int rlmax = INT_MAX; /* resource's limit */
   int scmax = INT_MAX; /* sysconf limit */

#ifdef _SC_OPEN_MAX
   scmax = sysconf(_SC_OPEN_MAX); /* systemwide maximum */
#endif

#if defined(HAVE_SYS_RESOURCE_H) && defined(RLIMIT_OFILE)
   /* user's limit might be decreased by himself: */
   {
      struct rlimit rl;

      if (getrlimit(RLIMIT_OFILE,&rl) == 0)
         if ((unsigned) rl.rlim_cur < (unsigned) INT_MAX)
            rlmax = (int) rl.rlim_cur;
   }
#endif

   if (rlmax < scmax) /* map rlmax to scmax */
      scmax = rlmax;
   if (scmax != INT_MAX) /* either getrlimit or sysconf valid? */
      return(scmax);

#ifdef POSIX_OPEN_MAX
   /* maybe, we have a hardcoded limit */
   if (POSIX_OPEN_MAX != INT_MAX) /* shall work in most cases */
        return(POSIX_OPEN_MAX);
#endif

   return(256); /* just a guess */
}
#endif

/* fork_exec spawns a new process with the given commandline.
 * it returns -1 on error (errno set), 0 on process start error (rcode set),
 * a process descriptor otherwise.
 * Basically this is a child process and we run in the child's environment
 * after the first few lines. The setup hasn't been done and the command needs
 * to be started.
 * Redirection takes place if one of the handles env->input.hdls[0],
 * env->output.hdls[1] or env->error.hdls[1] is defined. Other handles (except
 * standard handles) are closed. env->subtype must be a SUBENVIR_... constant.
 * cmdline is the whole command line.
 * Never use TSD after the fork() since this is not only a different thread,
 * it's a different process!
 */
int fork_exec(tsd_t *TSD, environment *env, const char *cmdline, int *rcode)
{
   static const char *interpreter[] = { "regina", /* preferable even if not */
                                                  /* dynamic                */
                                        "rexx" };
   char **args ;
   int i, rc, max_hdls = MaxFiles() ;
   int broken_address_command = get_options_flag( TSD->currlevel, EXT_BROKEN_ADDRESS_COMMAND );
   int subtype;

   if ( ( rc = fork() ) != 0 )
      return( rc );

   /* Now we are the child */

#define STD_REDIR(hdl,dest) if ((hdl != -1) && (hdl != dest)) dup2(hdl, dest)
#define SET_MAXHDL(hdl) if (hdl > max_hdls) max_hdls = hdl
#define SET_MAXHDLS(ep) SET_MAXHDL(ep.hdls[0]); SET_MAXHDL(ep.hdls[1])

                                        /* Force the standard redirections:  */
   STD_REDIR(env->input.hdls[0],    0);
   STD_REDIR(env->output.hdls[1],   1);
   if (env->error.SameAsOutput)
   {
      STD_REDIR(1,                  2);
   }
   else
   {
      STD_REDIR(env->error.hdls[1], 2);
   }

                                    /* any handle greater than the default ? */
   SET_MAXHDLS(env->input);
   SET_MAXHDLS(env->output);
   if (!env->error.SameAsOutput)
      SET_MAXHDLS(env->error);

   for (i=3; i <= max_hdls; i++)
      close( i ) ;

   /*
    * If the BROKEN_ADDRESS_COMMAND OPTION is in place,
    * and our environment is COMMAND, change it to SYSTEM
    */
   if ( env->subtype == SUBENVIR_PATH /* was SUBENVIR_COMMAND */
   &&   broken_address_command )
      subtype = SUBENVIR_SYSTEM;
   else
      subtype = env->subtype;

   switch ( subtype )
   {
      case SUBENVIR_PATH:
         args = makeargs(cmdline, '\\');
#define NEED_MAKEARGS
         execvp(*args, args);
         break;

      case SUBENVIR_COMMAND:
         args = makeargs(cmdline, '\\');
         execv(*args, args);
         break;

      case SUBENVIR_SYSTEM:
#if defined(HAVE_WIN32GUI)
         rc = mysystem( cmdline ) ;
#else
         rc = system( cmdline ) ;
#endif
#ifdef VMS
         exit (rc); /* This is a separate process, exit() is allowed */
#else
         if ( WIFEXITED( rc ) )
         {
            fflush( stdout );
            _exit( (int) WEXITSTATUS(rc) ); /* This is a separate process, exit() is allowed */
         }
         else if ( WIFSIGNALED( rc ) )
            raise( WTERMSIG( rc ) ); /* This is a separate process, raise() is allowed */
         else
            raise( WSTOPSIG( rc ) ); /* This is a separate process, raise() is allowed */
#endif
         break;
      case SUBENVIR_REXX:
         {
            char *new_cmdline;
            char **run;
            int i;
            unsigned len;

            if (argv0 == NULL)
               len = 7; /* max("rexx", "regina") */
            else
               {
                  len = strlen(argv0);
                  if (len < 7)
                     len = 7; /* max("rexx", "regina") */
               }
            len += strlen(cmdline) + 2; /* Blank + term ASCII0 */

            if ((new_cmdline = malloc(len)) == NULL)
               raise( SIGKILL ); /* This is a separate process, raise() is allowed */

            if (argv0 != NULL) /* always the best choice */
            {
               strcpy(new_cmdline, argv0);
               strcat(new_cmdline, " ");
               strcat(new_cmdline, cmdline);
               args = makeargs(new_cmdline, '\\');
               execv(*args, args);
#define NEED_DESTROYARGS
               destroyargs(args);
            }

            /* load an interpreter by name from the path */
            for (i = 0; i < sizeof(interpreter) / sizeof(interpreter[0]);i++)
            {
               strcpy(new_cmdline, interpreter[i]);
               strcat(new_cmdline, " ");
               strcat(new_cmdline, cmdline);
               args = makeargs(new_cmdline, '\\');
               execvp(*args, args);
#define NEED_DESTROYARGS
               destroyargs(args);
            }

            /* last chance, worst choice, use the re-entering code: */
            strcpy(new_cmdline, "\"\" ");
            strcat(new_cmdline, cmdline);
            args = makeargs(new_cmdline, '\\');

            for (i = 0, run = args; *run; run++)
               i++;
            fflush( stdout );
            _exit(__regina_reexecute_main(i, args));
   }

      default: /* illegal subtype */
         raise( SIGKILL ) ; /* This is a separate process, raise() is allowed */
   }

   /* exec() failed */
   raise( SIGKILL ); /* This is a separate process, raise() is allowed */
#undef SET_MAXHDLS
#undef SET_MAXHDL
#undef STD_REDIR
   return -1; /* keep the compiler happy */
}

/* __regina_wait waits for a process started by fork_exec.
 * In general, this is called after the complete IO to the called process but
 * it isn't guaranteed. Never call if you don't expect a sudden death of the
 * subprocess.
 * Returns the exit status of the subprocess under normal circumstances. If
 * the subprocess has died by a signal, the return value is -(100+signalnumber)
 */
int __regina_wait(int process)
{
   int rc, retval, status;
#ifdef VMS
   for ( ; ; )
   {
      rc = wait( &status ) ;
      if (rc != -1)
         break;
      rc = errno;
      if (rc == EINTR)
         continue;
      break;
   }
   retval = status & 0xff ;
#else
   for ( ; ; )
   {
# ifdef NEXT
      /*
       * According to Paul F. Kunz, NeXTSTEP 3.1 Prerelease doesn't have
       * the waitpid() function, so wait() must be used instead. The
       * following klugde will remain until NeXTSTEP gets waitpid().
       */
      wait( &status ) ;
# else /* ndef NEXT */
#  ifdef DOS
      rc = wait( &status ) ;
#  else /* ndef DOS */
      rc = waitpid( process, &status, 0 ) ;
#  endif /* def DOS */
# endif /* def NEXT */
      if (rc != -1)
         break;
      rc = errno;
      if (rc == EINTR)
         continue;
      break;
   }
   /* still ndef VMS */
   if (WIFEXITED(status))
   {
      retval = (int) WEXITSTATUS(status);
      if ( retval < 0 )
         retval = -retval;
   }
   else if (WIFSIGNALED(status))
   {
      retval = -WTERMSIG(status);
      if ( retval > 0 )
         retval = -retval;
      else if ( retval == 0 )
         retval = -1;
   }
   else
   {
      retval = -WSTOPSIG(status);
      if ( retval > 0 )
         retval = -retval;
      else if ( retval == 0 )
         retval = -1;
   }
#endif /* def VMS */
   return(retval);
}

/* open_subprocess_connection acts like the unix-known function pipe and sets
 * ep->RedirectedFile if necessary. Just in the latter case ep->data
 * is set to the filename.
 */
int open_subprocess_connection(const tsd_t *TSD, environpart *ep)
{
   return(pipe(ep->hdls));
}

/* sets the given handle to the non-blocking mode. The value may change.
 * async_info CAN be used to add the handle to the internal list of watched
 * handles.
 */
void unblock_handle( int *handle, void *async_info )
{
   int fl ;

   if (*handle == -1)
      return ;

   fl = fcntl( *handle, F_GETFL ) ;

   if ( fl == -1 ) /* We can either abort or try to continue, try to */
      return ;     /* continue for now.                              */

   fcntl( *handle, F_SETFL, fl | O_NONBLOCK ) ;
}

/* restart_file sets the file pointer of the given handle to the beginning.
 */
void restart_file(int hdl)
{
   lseek(hdl, 0l, SEEK_SET);
}

/* __regina_close acts like close() but closes a handle returned by
 * open_subprocess_connection.
 * async_info MAY be used to delete the handle from the internal list of
 * watched handles.
 */
int __regina_close(int handle, void *async_info)
{
   return(close(handle));
}

/*
 * __regina_close_special acts like close() but closes any OS specific handle.
 * The close happens if the handle is not -1. A specific operation may be
 * associated with this. Have a look for occurances of "hdls[2]".
 */
void __regina_close_special( int handle )
{
   assert( handle == -1 );
}

/* __regina_read acts like read() but returns either an error (return code
 * == -errno) or the number of bytes read. EINTR and friends leads to a
 * re-read.
 * use_blocked_io is a flag. If set, the handle is set to blocked IO and
 * we shall use blocked IO here.
 */
int __regina_read(int hdl, void *buf, unsigned size, void *async_info)
{
   int done ;

   do {
      done = read( hdl, buf, size ) ;
   } while ((done == -1) && (errno == EINTR));

   if (done < 0)
   {
      done = errno;
      if (done == 0)      /* no error set? */
         done = EPIPE ;   /* good assumption */
#if defined(EWOULDBLOCK) && defined(EAGAIN) && (EAGAIN != EWOULDBLOCK)
      /* BSD knows this value with the same meaning as EAGAIN */
      if (done == EWOULDBLOCK)
         done = EAGAIN ;
#endif
      return( -done ) ; /* error */
   }

   return(done);
}

/* __regina_write acts like write() but returns either an error (return code
 * == -errno) or the number of bytes written. EINTR and friends leads to a
 * re-write.
 * use_blocked_io is a flag. If set, the handle is set to blocked IO and
 * we shall use blocked IO here.
 * The file must be flushed, if both buf and size are 0.
 */
int __regina_write(int hdl, const void *buf, unsigned size, void *async_info)
{
   int done ;

   if ((buf == NULL) || (size == 0)) /* nothing to to for flushing buffers */
      return(0);

   do {
      done = write( hdl, buf, size ) ;
   } while ((done == -1) && (errno == EINTR));

   if (done < 0)
   {
      done = errno;
      if (done == 0)       /* no error set? */
         done = ENOSPC ;   /* good assumption */
#if defined(EWOULDBLOCK) && defined(EAGAIN) && (EAGAIN != EWOULDBLOCK)
      /* BSD knows this value with the same meaning as EAGAIN */
      if (done == EWOULDBLOCK)
         done = EAGAIN ;
#endif
      return( -done ) ; /* error */
   }

   return(done);
}

/* create_async_info return an opaque structure to allow the process wait for
 * asyncronous IO. There are three IO slots (in, out, error) which can be
 * filled by add_waiter. The structure can be cleaned by reset_async_info.
 * The structure must be destroyed by delete_async_info.
 */
void *create_async_info(const tsd_t *TSD)
{
   AsyncInfo *ai = MallocTSD(sizeof(AsyncInfo));

   ai->TSD = TSD;
   return(ai);
}
/* delete_async_info deletes the structure created by create_async_info and
 * all of its components.
 */
void delete_async_info(void *async_info)
{
   AsyncInfo *ai = async_info;

   if (ai == NULL)
      return;
   Free_TSD(ai->TSD, ai);
}

#ifdef POLLIN /* we have poll() */

/* reset_async_info clear async_info in such a way that fresh add_waiter()
 * calls can be performed.
 */
void reset_async_info(void *async_info)
{
   AsyncInfo *ai = async_info;

   ai->_p_cnt = 0;
}

/* add_async_waiter adds a further handle to the asyncronous IO structure.
 * add_as_read_handle must be != 0 if the next operation shall be a
 * __regina_read, else it must be 0 for __regina_write.
 * Call reset_async_info before a wait-cycle to different handles and use
 * wait_async_info to wait for at least one IO-able handle.
 */
void add_async_waiter(void *async_info, int handle, int add_as_read_handle)
{
   AsyncInfo *ai = async_info;

   assert(ai->_p_cnt < 3);
   ai->_p[ai->_p_cnt].fd = handle;
   ai->_p[ai->_p_cnt++].events = (add_as_read_handle) ? POLLIN : POLLOUT;
}

/* wait_async_info waits for some handles to become ready. This function
 * returns if at least one handle becomes ready.
 * A handle can be added with add_async_waiter to the bundle of handles to
 * wait for.
 * No special handling is implemented if an asyncronous interrupt occurs.
 * Thus, there is no guarantee to have handle which works.
 */
void wait_async_info(void *async_info)
{
   AsyncInfo *ai = async_info;

   if (ai->_p_cnt)
      poll(ai->_p, ai->_p_cnt, -1);
}

#else /* end of POLLIN, must be select */

/* reset_async_info clear async_info in such a way that fresh add_waiter()
 * calls can be performed.
 */
void reset_async_info(void *async_info)
{
   AsyncInfo *ai = async_info;

   FD_ZERO( &ai->_p_in );
   FD_ZERO( &ai->_p_out );
   ai->_p_max = -1 ;
}

/* add_async_waiter adds a further handle to the asyncronous IO structure.
 * add_as_read_handle must be != 0 if the next operation shall be a
 * __regina_read, else it must be 0 for __regina_write.
 * Call reset_async_info before a wait-cycle to different handles and use
 * wait_async_info to wait for at least one IO-able handle.
 */
void add_async_waiter(void *async_info, int handle, int add_as_read_handle)
{
   AsyncInfo *ai = async_info;

   FD_SET(handle,(add_as_read_handle) ? &ai->_p_in : &ai->_p_out ) ;
   if (handle > ai->_p_max)
      ai->_p_max = handle ;
}

/* wait_async_info waits for some handles to become ready. This function
 * returns if at least one handle becomes ready.
 * A handle can be added with add_async_waiter to the bundle of handles to
 * wait for.
 * No special handling is implemented if an asyncronous interrupt occurs.
 * Thus, there is no guarantee to have handle which works.
 */
void wait_async_info(void *async_info)
{
   AsyncInfo *ai = async_info;

   if (ai->_p_max >= 0)
      select( ai->_p_max+1, &ai->_p_in, &ai->_p_out, NULL, NULL);
}

#endif /* POLLIN or select */
#endif /* WIN32 */

/*****************************************************************************
 *****************************************************************************
 ** DOS and unknown or trivial systems ***************************************
 *****************************************************************************
 *****************************************************************************/
#ifdef NEED_STUPID_DOSCMD
#include <errno.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#else
# include <io.h>
#endif

#if defined(HAVE_SYS_STAT_H)
# include <sys/stat.h>
#endif

#if defined(HAVE_FCNTL_H)
# include <fcntl.h>
#endif

#if defined(HAVE_SYS_FCNTL_H)
# include <sys/fcntl.h>
#endif

#ifndef I_SLASH
# define I_SLASH '/'
#endif

#ifndef ISTR_SLASH
# define ISTR_SLASH "/"
#endif

/* This flavour is the default style. It doesn't use pipes. It uses temporary
 * files instead.
 * Slow, but shall work with all machines.
 */

/* fork_exec spawns a new process with the given commandline.
 * it returns -1 on error (errno set), 0 on process start error (rcode set),
 * a process descriptor otherwise.
 * Basically this is a child process and we run in the child's environment
 * after the first few lines. The setup hasn't been done and the command needs
 * to be started.
 * Redirection takes place if one of the handles env->input.hdls[0],
 * env->output.hdls[1] or env->error.hdls[1] is defined. Other handles (except
 * standard handles) are closed. env->subtype must be a SUBENVIR_... constant.
 * cmdline is the whole command line.
 * Never use TSD after the fork() since this is not only a different thread,
 * it's a different process!
 */
int fork_exec(tsd_t *TSD, environment *env, const char *cmdline, int *rcode)
{
   static const char *interpreter[] = { "regina", /* preferable even if not */
                                                  /* dynamic                */
                                        "rexx" };
   char **args = NULL;
   int saved_in = -1, saved_out = -1, saved_err = -1;
   int rc, eno ;
   int broken_address_command = get_options_flag( TSD->currlevel, EXT_BROKEN_ADDRESS_COMMAND );
   int subtype;

   if (env->subtype == SUBENVIR_REXX) /*special situation caused by recursion*/
   {
      environment e = *env;
      char *new_cmdline;
      int i, rc;
      unsigned len;

      if (argv0 == NULL)
         len = 7; /* max("rexx", "regina") */
      else
         {
            len = strlen(argv0);
            if (len < 7)
               len = 7; /* max("rexx", "regina") */
         }
      len += strlen(cmdline) + 2; /* Blank + term ASCII0 */

      if ((new_cmdline = malloc(len)) == NULL)
         return -1; /* ENOMEM is set */

      if (argv0 != NULL) /* always the best choice */
      {
         strcpy(new_cmdline, argv0);
         strcat(new_cmdline, " ");
         strcat(new_cmdline, cmdline);
         e.subtype = SUBENVIR_COMMAND;
         rc = fork_exec(TSD, &e, new_cmdline, &rc);
         if ( ( rc != 0 ) && ( rc != -1 ) )
         {
            free(new_cmdline);
            return(rc);
         }
      }

      /* load an interpreter by name from the path */
      for (i = 0; i < sizeof(interpreter) / sizeof(interpreter[0]);i++)
      {
         strcpy(new_cmdline, interpreter[i]);
         strcat(new_cmdline, " ");
         strcat(new_cmdline, cmdline);
         e.subtype = SUBENVIR_SYSTEM;
         rc = fork_exec(TSD, &e, new_cmdline, &rc);
         if ( ( rc != 0 ) && ( rc != -1 ) )
         {
            free(new_cmdline);
            return(rc);
         }
      }

      *rcode = -errno; /* assume a load error */
      free(new_cmdline);
      return 0;
   }

#define STD_REDIR(hdl,dest,save) if ((hdl != -1) && (hdl != dest)) \
                                    { save = dup(dest); dup2(hdl, dest); }
#define STD_RESTORE(saved,dest) if (saved != -1) \
                                    { close(dest); dup2(saved,dest); \
                                      close(saved); }
#define SET_MAXHDL(hdl) if (hdl > max_hdls) max_hdls = hdl
#define SET_MAXHDLS(ep) SET_MAXHDL(ep.hdls[0]); SET_MAXHDL(ep.hdls[1])

                                        /* Force the standard redirections:  */
   STD_REDIR(env->input.hdls[0],    0, saved_in);
   STD_REDIR(env->output.hdls[1],   1, saved_out);
   if (env->error.SameAsOutput)
   {
      saved_err = dup(2);
      dup2(1, 2);
   }
   else
   {
      STD_REDIR(env->error.hdls[1], 2, saved_err);
   }

   /*
    * If the BROKEN_ADDRESS_COMMAND OPTION is in place,
    * and our environment is COMMAND, change it to SYSTEM
    */
   if ( env->subtype == SUBENVIR_PATH /* was SUBENVIR_COMMAND */
   &&   broken_address_command )
      subtype = SUBENVIR_SYSTEM;
   else
      subtype = env->subtype;
   rc = -1;

   switch (env->subtype)
   {
      case SUBENVIR_PATH:
#ifdef P_WAIT
# if defined(DOS) && !defined(__EMX__)
         args = makesimpleargs(cmdline);
# define NEED_MAKESIMPLEARGS
# else
         args = makeargs(cmdline, '\\');
# define NEED_MAKEARGS
# endif
         rc = spawnvp(P_WAIT, *args, args);
#else
         goto USE_SUBENVIR_SYSTEM;
#endif
         break;

      case SUBENVIR_COMMAND:
#ifdef P_WAIT
# if defined(DOS) && !defined(__EMX__)
         args = makesimpleargs(cmdline);
# else
         args = makeargs(cmdline, '\\');
# endif
         rc = spawnv(P_WAIT, *args, args);
#else
         goto USE_SUBENVIR_SYSTEM;
#endif
         break;

      case SUBENVIR_SYSTEM:
#ifndef P_WAIT
         USE_SUBENVIR_SYSTEM:
#endif
         rc = system(cmdline);
         break;

      case SUBENVIR_REXX:
         /* fall through */

      default: /* illegal subtype */
         STD_RESTORE(saved_in, 0);
         STD_RESTORE(saved_out, 1);
         STD_RESTORE(saved_err, 2);
#ifdef NEED_MAKEARGS
         if (args != NULL)
            destroyargs(args);
#define NEED_DESTROYARGS
#endif
         errno = EINVAL;
         rc = -1;
         break;
   }

   eno = errno;
   STD_RESTORE(saved_in, 0);
   STD_RESTORE(saved_out, 1);
   STD_RESTORE(saved_err, 2);
#ifdef NEED_MAKEARGS
   if (args != NULL)
      destroyargs(args);
#endif
   errno = eno;

   if ( rc == -1 )
      return(0);

   rc -= 0x4000; /* do a remap */
   if ( (rc == -1) || ( rc == 0 ) )
      rc = -0x4000 + 127;
   return rc;
#undef SET_MAXHDLS
#undef SET_MAXHDL
#undef STD_RESTORE
#undef STD_REDIR
}

/* __regina_wait waits for a process started by fork_exec.
 * In general, this is called after the complete IO to the called process but
 * it isn't guaranteed. Never call if you don't expect a sudden death of the
 * subprocess.
 * Returns the exit status of the subprocess under normal circumstances. If
 * the subprocess has died by a signal, the return value is -signalnumber.
 */
int __regina_wait(int process)
{
   return(process + 0x4000);
}

/* open_subprocess_connection acts like the unix-known function pipe and sets
 * ep->RedirectedFile if necessary. Just in the latter case ep->data
 * is set to the filename.
 * Close the handles with __regina_close later.
 * Do IO by using __regina_read() and __regina_write().
 */
int open_subprocess_connection(const tsd_t *TSD, environpart *ep)
{
   char *name;
   int eno;

   name = MallocTSD(REXX_PATH_MAX + 1);
   /* Remember to create two handles to be "pipe()"-conform. */
#define NEED_LOCAL_MKSTEMP
   if ((ep->hdls[0] = local_mkstemp(TSD, name)) == -1)
   {
      eno = errno;
      free( name );
      errno = eno;
      return(-1);
   }

   if ((ep->hdls[1] = dup(ep->hdls[0])) == -1)
   {
      eno = errno;
      close(ep->hdls[0]);
      ep->hdls[0] = -1;
      unlink(name);
      free(name);
      errno = eno;
      return(-1);
   }

   ep->FileRedirected = 1;
   ep->tempname = name;
   return(0);
}

/* sets the given handle to the non-blocking mode. The value may change.
 * async_info CAN be used to add the handle to the internal list of watched
 * handles.
 */
void unblock_handle( int *handle, void *async_info )
{
   (handle = handle);
   (async_info = async_info);
}

/* restart_file sets the file pointer of the given handle to the beginning.
 */
void restart_file(int hdl)
{
   lseek(hdl, 0l, SEEK_SET);
}

/* __regina_close acts like close() but closes a handle returned by
 * open_subprocess_connection.
 * async_info MAY be used to delete the handle from the internal list of
 * watched handles.
 */
int __regina_close(int handle, void *async_info)
{
   (async_info = async_info);
   return(close(handle));
}

/*
 * __regina_close_special acts like close() but closes any OS specific handle.
 * The close happens if the handle is not -1. A specific operation may be
 * associated with this. Have a look for occurances of "hdls[2]".
 */
void __regina_close_special( int handle )
{
   assert( handle == -1 );
}

/* __regina_read acts like read() but returns either an error (return code
 * == -errno) or the number of bytes read. EINTR and friends leads to a
 * re-read.
 * use_blocked_io is a flag. If set, the handle is set to blocked IO and
 * we shall use blocked IO here.
 */
int __regina_read(int hdl, void *buf, unsigned size, void *async_info)
{
   int done;

#ifdef EINTR
   do {
      done = read(hdl, buf, size);
   } while ((done == -1) && (errno == EINTR));
#else
   done = read(hdl, buf, size);
#endif

   if (done < 0)
   {
      done = errno;
      if (done == 0)      /* no error set? */
         done = EPIPE ;   /* good assumption */
#if defined(EWOULDBLOCK) && defined(EAGAIN) && (EAGAIN != EWOULDBLOCK)
      /* BSD knows this value with the same meaning as EAGAIN */
      if (done == EWOULDBLOCK)
         done = EAGAIN;
#endif
      return(-done); /* error */
   }

   return(done);
}

/* __regina_write acts like write() but returns either an error (return code
 * == -errno) or the number of bytes written. EINTR and friends leads to a
 * re-write.
 * use_blocked_io is a flag. If set, the handle is set to blocked IO and
 * we shall use blocked IO here.
 * The file must be flushed, if both buf and size are 0.
 */
int __regina_write(int hdl, const void *buf, unsigned size, void *async_info)
{
   int done;

   if ((buf == NULL) || (size == 0)) /* nothing to to for flushing buffers */
      return(0);

#ifdef EINTR
   do {
      done = write( hdl, buf, size ) ;
   } while ((done == -1) && (errno == EINTR));
#else
   done = write( hdl, buf, size ) ;
#endif

   if (done <= 0)
   {
      done = errno;
      if (done == 0)      /* no error set? */
         done = ENOSPC;   /* good assumption */
#if defined(EWOULDBLOCK) && defined(EAGAIN) && (EAGAIN != EWOULDBLOCK)
      /* BSD knows this value with the same meaning as EAGAIN */
      if (done == EWOULDBLOCK)
         done = EAGAIN;
#endif
      return(-done); /* error */
   }

   return(done);
}

/* create_async_info return an opaque structure to allow the process wait for
 * asyncronous IO. There are three IO slots (in, out, error) which can be
 * filled by add_waiter. The structure can be cleaned by reset_async_info.
 * The structure must be destroyed by delete_async_info.
 */
void *create_async_info(const tsd_t *TSD)
{
   (TSD = TSD);
   return(NULL);
}

/* delete_async_info deletes the structure created by create_async_info and
 * all of its components.
 */
void delete_async_info(void *async_info)
{
   (async_info = async_info);
}

/* reset_async_info clear async_info in such a way that fresh add_waiter()
 * calls can be performed.
 */
void reset_async_info(void *async_info)
{
   (async_info = async_info);
}

/* add_async_waiter adds a further handle to the asyncronous IO structure.
 * add_as_read_handle must be != 0 if the next operation shall be a
 * __regina_read, else it must be 0 for __regina_write.
 * Call reset_async_info before a wait-cycle to different handles and use
 * wait_async_info to wait for at least one IO-able handle.
 */
void add_async_waiter(void *async_info, int handle, int add_as_read_handle)
{
   (async_info = async_info);
   (handle = handle);
   (add_as_read_handle = add_as_read_handle);
}

/* wait_async_info waits for some handles to become ready. This function
 * returns if at least one handle becomes ready.
 * A handle can be added with add_async_waiter to the bundle of handles to
 * wait for.
 * No special handling is implemented if an asyncronous interrupt occurs.
 * Thus, there is no guarantee to have handle which works.
 */
void wait_async_info(void *async_info)
{
   (async_info = async_info);
}
#endif /* NEED_STUPID_DOSCMD */
/*****************************************************************************
 *****************************************************************************
 ** fork_exec and depending functions ends here ******************************
 *****************************************************************************
 *****************************************************************************/

#if defined(NEED_MAKEARGS) || defined(NEED_SPLITOFFARG)
/* nextarg parses source for the next argument in unix shell terms. If target
 * is given, it must consist of enough free characters to hold the result +
 * one byte for the terminator. If len != NULL it will become the length of
 * the string (which might not been return if target == NULL). The return value
 * is either NULL or a new start value for nextarg.
 * escape is the current escape value which should be used, must be set.
 */
static const char *nextarg(const char *source, unsigned *len, char *target,
                                                                   char escape)
{
   unsigned l;
   char c, term;

   if (len != NULL)
      *len = 0;
   if (target != NULL)
      *target = '\0';
   l = 0;  /* cached length */

   if (source == NULL)
      return(NULL);

   while (rx_isspace(*source)) /* jump over initial spaces */
      source++;
   if (*source == '\0')
      return(NULL);

   do {
      /* There's something to return. Check for delimiters */
      term = *source++;

      if ((term == '\'') || (term == '\"'))
      {
         while ((c = *source++) != term) {
            if (c == escape)
               c = *source++;
            if (c == '\0')  /* stray \ at EOS is equiv to normal EOS */
            {
               /* empty string is valid! */
               if (len != NULL)
                  *len = l;
               if (target != NULL)
                  *target = '\0';
               return(source - 1); /* next try returns NULL */
            }
            l++;
            if (target != NULL)
               *target++ = c;
         }
      }
      else /* whitespace delimiters */
      {
         c = term;
         while (!rx_isspace(c) && (c != '\'') && (c != '\"')) {
            if (c == escape)
               c = *source++;
            if (c == '\0')  /* stray \ at EOS is equiv to normal EOS */
            {
               /* at least a stray \ was found, empty string checked in
                * the very beginning.
                */
               if (len != NULL)
                  *len = l;
               if (target != NULL)
                  *target = '\0';
               return(source - 1); /* next try returns NULL */
            }
            l++;
            if (target != NULL)
               *target++ = c;
            c = *source++;
         }
         source--; /* undo the "wrong" character */
      }
   } while (!rx_isspace(*source));

   if (len != NULL)
      *len = l;
   if (target != NULL)
      *target = '\0';
   return(source);
}
#endif

#ifdef NEED_MAKEARGS
/* makeargs chops string into arguments and returns an array of x+1 strings if
 * string contains x args. The last argument is NULL. This function usually is
 * called from the subprocess if fork/exec is used.
 * Example: "xx y" -> { "xx", "y", NULL }
 * escape must be the escape character of the command line and is usually ^
 * or \
 */
static char **makeargs(const char *string, char escape)
{
   char **retval;
   const char *p;
   int i, argc = 0;
   unsigned size;

   p = string; /* count the number of strings */
   while ((p = nextarg(p, NULL, NULL, escape)) != NULL)
      argc++;
   if ((retval = malloc((argc + 1) * sizeof(char *))) == NULL)
      return(NULL);

   p = string; /* count each string length */
   for (i = 0; i < argc; i++)
   {
      p = nextarg(p, &size, NULL, escape);
      if ((retval[i] = malloc(size + 1)) == NULL)
      {
         i--;
         while (i >= 0)
            free(retval[i--]);
         free(retval);
         return(NULL);
      }
   }

   p = string; /* assign each string */
   for (i = 0; i < argc; i++)
      p = nextarg(p, NULL, retval[i], escape);
   retval[argc] = NULL;

   return(retval);
}
#endif

#ifdef NEED_SPLITOFFARG
/* splitoffarg chops string into two different pieces: The first argument and
 * all other (uninterpreted) arguments. The first argument is returned in a
 * freshly allocated string. The rest is a pointer somewhere within string
 * and returned in *trailer.
 * Example: "xx y" -> returns "xx", *trailer == "xx y"+2
 * escape must be the escape character of the command line and is usually ^
 * or \
 */
static char *splitoffarg(const char *string, const char **trailer, char escape)
{
   unsigned size;
   char *retval;
   const char *t;

   if (trailer != NULL)
      *trailer = ""; /* just a default */
   nextarg(string, &size, NULL, escape);
   if ((retval = malloc(size + 1)) == NULL)
      return(NULL);

   t = nextarg(string, NULL, retval, escape);
   if (trailer != NULL)
      *trailer = t;
   return(retval);
}
#endif

#ifdef NEED_MAKESIMPLEARGS
/* nextarg parses source for the next argument as a simple word. If target
 * is given, it must consist of enough free characters to hold the result +
 * one byte for the terminator. If len != NULL it will become the length of
 * the string (which might not been return if target == NULL). The return value
 * is either NULL or a new start value for nextarg.
 */
static const char *nextsimplearg(const char *source, unsigned *len,
                                                                  char *target)
{
   unsigned l;
   char c, term;

   if (len != NULL)
      *len = 0;
   if (target != NULL)
      *target = '\0';
   l = 0;  /* cached length */

   if (source == NULL)
      return(NULL);

   while (rx_isspace(*source)) /* jump over initial spaces */
      source++;
   if (*source == '\0')
      return(NULL);

   c = *source++;

   while (!rx_isspace(c))
   {
      if (c == '\0')  /* stray \ at EOS is equiv to normal EOS */
      {
         /* something's found, therefore we don't have to return NULL */
         if (len != NULL)
            *len = l;
         if (target != NULL)
            *target = '\0';
         return(source - 1); /* next try returns NULL */
      }
      l++;
      if (target != NULL)
         *target++ = c;
      c = *source++;
   }
   source--; /* undo the "wrong" character */

   if (len != NULL)
      *len = l;
   if (target != NULL)
      *target = '\0';
   return(source);
}

/* makesimpleargs chops string into arguments and returns an array of x+1
 * strings if string contains x args. The last argument is NULL. This function
 * usually is called from the subprocess if fork/exec is used.
 * Example: "xx y" -> { "xx", "y", NULL }
 */
static char **makesimpleargs(const char *string)
{
   char **retval;
   const char *p;
   int i, argc = 0;
   unsigned size;

   p = string; /* count the number of strings */
   while ((p = nextsimplearg(p, NULL, NULL)) != NULL)
      argc++;
   if ((retval = malloc((argc + 1) * sizeof(char *))) == NULL)
      return(NULL);

   p = string; /* count each string length */
   for (i = 0; i < argc; i++)
   {
      p = nextsimplearg(p, &size, NULL);
      if ((retval[i] = malloc(size + 1)) == NULL)
      {
         i--;
         while (i >= 0)
            free(retval[i--]);
         free(retval);
         return(NULL);
      }
   }

   p = string; /* assign each string */
   for (i = 0; i < argc; i++)
      p = nextsimplearg(p, NULL, retval[i]);

   return(retval);
}
#endif

#ifdef NEED_DESTROYARGS
/* destroyargs destroys the array created by makeargs */
static void destroyargs(char **args)
{
   char **run = args;

   while (*run) {
      free(*run);
      run++;
   }
   free(args);
}
#endif

#ifdef NEED_LOCAL_MKSTEMP
/* local_mkstemp() works mostly like the commands
 * strcpy(base, system_specific_temppath), mkstemp( TSD, base);
 * mkstemp opens a newly created file and returns its name in "base".
 * The handle of the file is the function return value. The function
 * returns -1 if an error occurs. Use the errno value in this case.
 *
 * The handle is at least suitable for fork_exec(), __regina_read(),
 * __regina_write() and __regina_close().
 * base should have REXX_PATH_MAX characters.
 * Be careful: Don't forget to delete the file afterwards.
 */
static int local_mkstemp(const tsd_t *TSD, char *base)
{
#ifndef S_IRWXU
# define S_IRWXU (_S_IREAD|_S_IWRITE)
#endif
#ifdef HAVE_MKSTEMP
   /* We are using a unix system. We either have mkstemp or you
    * should enable the above code.
    */
   strcpy(base, "/tmp/rxXXXXXX");
   return(mkstemp(base));
#else
#define MAGIC_MAX 2000000 /* Much beyond maximum, see below */
   static volatile unsigned BaseIndex = MAGIC_MAX;
   int retval;
   char *slash;
   char buf[REXX_PATH_MAX]; /* enough space for largest path name */
   unsigned i;
   unsigned start,run;

   if ( mygetenv( TSD, "TMP", buf, sizeof(buf) ) == NULL)
   {
      if ( mygetenv( TSD, "TEMP", buf, sizeof(buf) ) == NULL)
      {
         if ( mygetenv( TSD, "TMPDIR", buf, sizeof(buf) ) == NULL)
         {
#ifdef UNIX
            strcpy(buf,"/tmp");
#else
            strcpy(buf,"C:");
#endif
         }
      }
   }

   if (strlen(buf) > REXX_PATH_MAX - 14 /* 8.3 + "\0" + ISLASH */)
      buf[REXX_PATH_MAX - 14] = '\0';

   if ( buf[strlen(buf)-1] != I_SLASH )
      slash = ISTR_SLASH;
   else
      slash = "";

   /* algorithm:
    * select a random number, e.g. a mixture of pid, tid and time.
    * increment this number by a fixed amount until we reach the starting
    * value again. Do a wrap around and an increment which is a unique prime.
    * The number 1000000 has the primes 2 and 5. We may use all primes
    * except 2 and 5; 9901 (which is prime) will give a good distribution.
    *
    * We want to be able to create several temporary files at once without
    * more OS calls than needed. Thus, we have a program wide runner to
    * ensure a simple distribution without strength.
    */
   if (BaseIndex == MAGIC_MAX)
   {
      /*
       * We have to create (nearly) reentrant code.
       */
      i = (unsigned) getpid() * (unsigned) time(NULL);
      i %= 1000000;
      if (!i)
         i = 1;
      BaseIndex = i;
   }
   if (++BaseIndex >= 1000000)
      BaseIndex = 1;

   start = TSD->thread_id;
   if (start == 0)
      start = 999999;
   start *= (unsigned) (clock() + 1);
   start *= BaseIndex;
   start %= 1000000;

   strcat( buf, slash );
   slash = buf + strlen( buf );
   run = start;
   for (i = 0;i <= 1000000;i++)
   {
      /* form a name like "c:\temp\345302._rx" or "/tmp/345302._rx" */
      /*
       * fixes Bug 587687
       */
      sprintf( slash, "%06u._rx", run );
#if defined(_MSC_VER) /* currently not used but what's about CE ? */
      retval = _sopen(buf,
                      _O_RDWR|_O_CREAT|_O_BINARY|_O_SHORT_LIVED|_O_EXCL|
                                                                 _O_SEQUENTIAL,
                      SH_DENYRW,
                      S_IRWXU);
#else
      retval = open(buf,
                    O_RDWR|O_CREAT|O_EXCL
# if defined(O_NOCTTY)
                    |O_NOCTTY
# endif
                    ,
                    S_IRWXU);
#endif
      if (retval != -1) /* success */
      {
         strcpy(base,buf);
         return(retval);
      }
      /* Check for a true failure */
      if (errno != EEXIST)
         break;

      /* Check the next possible candidate */
      run += 9901;
      run %= 1000000;
      if (run == start) /* paranoia check. i <= 1000000 should hit exactly */
         break;         /* here */
   }
   return( -1 ); /* pro forma */
#undef MAGIC_MAX
#endif /* HAVE_MKSTEMP */
}
#endif /* LOCAL_MKSTEMP */

# ifdef NEED_UNAME
/********************************************************* MH 10-06-96 */
int uname(struct utsname *name)                         /* MH 10-06-96 */
/********************************************************* MH 10-06-96 */
{                                                       /* MH 10-06-96 */
#  if defined (WIN32)
 SYSTEM_INFO sysinfo;
 OSVERSIONINFO osinfo;
 char computername[MAX_COMPUTERNAME_LENGTH+1];
 char *pComputerName=computername;
 DWORD namelen=MAX_COMPUTERNAME_LENGTH+1;
#  endif
/*
 * Set up values for utsname structure...
 */
#  if defined(OS2)
 strcpy( name->sysname, "OS2" );
 sprintf( name->version, "%d" ,_osmajor );
 sprintf( name->release, "%d" ,_osminor );
 strcpy( name->nodename, "standalone" );
 strcpy( name->machine, "i386" );
#  elif defined(_AMIGA)
 strcpy( name->sysname, "AMIGA" );
 sprintf( name->version, "%d", 0 );
 sprintf( name->release, "%d", 0 );
 strcpy( name->nodename, "standalone" );
 strcpy( name->machine, "m68k" );
#  elif defined(MAC)
 strcpy( name->sysname, "MAC" );
 sprintf( name->version, "%d", 0 );
 sprintf( name->release, "%d", 0 );
 strcpy( name->nodename, "standalone" );
 strcpy( name->machine, "m68k" );
#  elif defined(__WINS__) || defined(__EPOC32__)
 epoc32_uname( name );
#  elif defined(WIN32)
 osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
 GetVersionEx( &osinfo );
 sprintf( name->version, "%d", osinfo.dwMajorVersion );
 sprintf( name->release, "%d", osinfo.dwMinorVersion );

 /* get specific OS version... */
 switch( osinfo.dwPlatformId )
 {
    case VER_PLATFORM_WIN32s:
       strcpy( name->sysname, "WIN32S" );
       break;
    case VER_PLATFORM_WIN32_WINDOWS:
       if ( osinfo.dwMinorVersion >= 90 )
          strcpy( name->sysname, "WINME" );
       else if ( osinfo.dwMinorVersion >= 10 )
          strcpy( name->sysname, "WIN98" );
       else
          strcpy( name->sysname, "WIN95" );
       break;
    case VER_PLATFORM_WIN32_NT:
       if ( osinfo.dwMajorVersion == 4 )
          strcpy( name->sysname, "WINNT" );
       else if ( osinfo.dwMajorVersion == 5 )
       {
          if ( osinfo.dwMinorVersion == 1 )
             strcpy( name->sysname, "WINXP" );
          else
             strcpy( name->sysname, "WIN2K" );
       }
       else
          strcpy( name->sysname, "UNKNOWN" );
       break;
    default:
       strcpy( name->sysname, "UNKNOWN" );
       break;
 }
 /*
  * get name of computer if possible.
  */
 if ( GetComputerName(pComputerName, &namelen) )
    strcpy( name->nodename, pComputerName );
 else
    strcpy( name->nodename, "UNKNOWN" );
 GetSystemInfo( &sysinfo );
 switch( sysinfo.dwProcessorType )
 {
    case PROCESSOR_INTEL_386:
       strcpy( name->machine, "i386" );
       break;
    case PROCESSOR_INTEL_486:
       strcpy( name->machine, "i486" );
       break;
    case PROCESSOR_INTEL_PENTIUM:
       strcpy( name->machine, "i586" );
       break;
#if 0
    case PROCESSOR_INTEL_MIPS_R4000:
       strcpy( name->machine, "mipsR4000" );
       break;
    case PROCESSOR_INTEL_ALPHA_21064:
       strcpy( name->machine, "alpha21064" );
       break;
#endif
 }
#  endif /* WIN32 */

 return(0);
}
# endif /* NEED_UNAME */
