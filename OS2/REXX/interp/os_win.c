/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 2005 Florian Grosse-Coosmann
 *
 *  This file contains Windows specific code.
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

#include "rexx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

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

#if (defined(__WATCOMC__) && !defined(__QNX__)) || defined(_MSC_VER) || defined(__MINGW32__) || defined(__BORLANDC__) || defined(__EPOC32__) || defined(__WINS__) || defined(__LCC__)
# include "utsname.h"                                   /* MH 10-06-96 */
# define NEED_UNAME
# if !defined(__WINS__) && !defined(__EPOC32__)
#  define MAXPATHLEN  _MAX_PATH                          /* MH 10-06-96 */
# endif
#else                                                   /* MH 10-06-96 */
# if defined(WIN32) && defined(__IBMC__)                /* LM 26-02-99 */
#  include "utsname.h"
#  define NEED_UNAME
#  define MAXPATHLEN (8192)
#  include <io.h>
# else
#  include <sys/param.h>                                 /* MH 10-06-96 */
#  include <sys/utsname.h>                               /* MH 10-06-96 */
#  include <sys/wait.h>
# endif
#endif

#if (defined(__WATCOMC__) && !defined(__QNX__)) || defined(_MSC_VER) || defined(DJGPP) || defined(__CYGWIN32__) || defined(__BORLANDC__) || defined(__MINGW32__) || defined(__WINS__) || defined(__EPOC32__)
# define HAVE_BROKEN_TMPNAM
# define PATH_DELIMS ":\\/"
# if defined(__CYGWIN32__)
#  define ISTR_SLASH "/"  /* This is not a must, \\ works, too */
#  define I_SLASH '/'  /* This is not a must, \\ works, too */
# else
#  define ISTR_SLASH "\\" /* This is not a must, / works at least for MSC, too */
#  define I_SLASH '\\'    /* This is not a must, / works at least for MSC, too */
# endif
# if !defined(__WINS__) && !defined(__EPOC32__) && !defined(__CYGWIN__)
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

static int IsWin9X(void)
{
   static int retval = -1;
   OSVERSIONINFO osinfo;

   if ( retval >= 0 )
      return retval;

#ifdef DEBUG
   if (getenv("REGINA_ACT_AS_WIN9X") != NULL)
   {
      retval = 1;
      return retval;
   }
#endif

   osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   GetVersionEx( &osinfo );

   switch( osinfo.dwPlatformId ) {
      case VER_PLATFORM_WIN32s:        /* Very stupid system */
      case VER_PLATFORM_WIN32_WINDOWS: /* Stupid systems     */
      default:
         retval = 1;
         break;
    case VER_PLATFORM_WIN32_NT:        /* Reasonable systems ... just my 2 ct, FGC */
         retval = 0;
         break;
   }
   return retval;
}

typedef struct {
   HANDLE      hdl;
   OVERLAPPED  ol;
   char       *buf;
   unsigned    maxbuf;
   unsigned    rused;
   unsigned    rusedbegin;
   unsigned    reading;
   unsigned    wused;
   int         is_reader;
} Win32IO;

typedef struct {
   const tsd_t   *TSD;
   Win32IO        h[3];
} AsyncInfo;

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

static int Win_setenv( const char *name, const char *value )
{
   return SetEnvironmentVariable( name, value );
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
static int Win_fork_exec(tsd_t *TSD, environment *env, const char *cmdline, int *rcode)
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
         rc = Win_fork_exec(TSD, &e, new_cmdline, &rc);
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
         rc = Win_fork_exec(TSD, &e, new_cmdline, &rc);
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
      if ( IsWin9X() )
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
      if ( IsWin9X() )
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
      if ( IsWin9X() )
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
   if ( !IsWin9X() )
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
}

/* __regina_wait waits for a process started by fork_exec.
 * In general, this is called after the complete IO to the called process but
 * it isn't guaranteed. Never call if you don't expect a sudden death of the
 * subprocess.
 * Returns the exit status of the subprocess under normal circumstances. If
 * the subprocess has died by a signal, the return value is -signalnumber.
 */
static int Win_wait(int process)
{
   DWORD code = (DWORD) -1; /* in case something goes wrong */

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
static int Win_open_subprocess_connection(const tsd_t *TSD, environpart *ep)
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
static void Win_unblock_handle( int *handle, void *async_info )
{
   AsyncInfo *ai;
   Win32IO *w;
   HANDLE hdl;
   unsigned i;

   ai = async_info;
   hdl = (HANDLE) *handle;

   assert(async_info != NULL);
   for (i = 0, w = ai->h; i < 3; i++, w++)
      if (w->hdl == INVALID_HANDLE_VALUE)
         break;
   assert(i < 3);
   w->hdl = hdl;
   w->ol.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
}

/* sets the file pointer of the given handle to the beginning.
 */
static void Win_restart_file(int hdl)
{
   assert((HANDLE) hdl == INVALID_HANDLE_VALUE);
}

/* __regina_close acts like close() but closes a handle returned by
 * open_subprocess_connection.
 * async_info MAY be used to delete the handle from the internal list of
 * watched handles.
 */
static int Win_close(int handle, void *async_info)
{
   AsyncInfo *ai;
   Win32IO *w;
   HANDLE hdl;
   unsigned i;

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

   for (i = 0, w = ai->h; i < 3; i++, w++)
      if (w->hdl == hdl)
         break;

   if (i == 3)
   {
      CloseHandle(hdl);
      errno = EINVAL;
      return(-1);
   }

   MyCancelIo(hdl);
   CloseHandle(w->ol.hEvent);
   w->hdl = INVALID_HANDLE_VALUE;
   if (w->buf)
      Free_TSD(ai->TSD, w->buf);
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
static void Win_close_special( int handle )
{
   if ( handle )
      CloseHandle( (HANDLE) handle );
}

/* __regina_read acts like read() but returns either an error (return code
 * == -errno) or the number of bytes read. EINTR and friends leads to a
 * re-read.
 * async_info is both a structure and a flag. If set, asynchronous IO shall
 * be used, otherwise blocked IO has to be used.
 */
static int Win_read(int handle, void *buf, unsigned size, void *async_info)
{
   AsyncInfo *ai;
   Win32IO *w;
   HANDLE hdl;
   unsigned i;
   OVERLAPPED *ol;
   DWORD done;
   int retval;

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
   for (i = 0, w = ai->h; i < 3; i++, w++)
   {
      if (w->hdl == hdl)
         break;
   }
   if (i == 3)
      return(-EINVAL);
   ol = &w->ol;

   if (w->reading) /* pending IO? */
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
            w->reading = 0;
            retval = -EPIPE;
         }
      }
      else
         w->reading = 0;

      if (done)
         w->rused += done;
   }

   if (w->rused)
   {
      if (size > w->rused)
         size = w->rused;
      memcpy(buf, w->buf + w->rusedbegin, size);
      w->rusedbegin += size;
      w->rused -= size;
      retval = size;
   }
   else
      retval = -EAGAIN; /* still may change! */

   if (w->maxbuf < REGINA_BUFFER_SIZE) /* Buffer not allocated? */
   {
      /* Never allocate too much, we want a fast response to do some
       * work!
       */
      if (w->buf) /* THIS IS DEFINITELY A BUG! */
         Free_TSD(ai->TSD, w->buf);
      w->maxbuf = REGINA_BUFFER_SIZE;
      w->rusedbegin = 0;
      w->rused = 0;
      w->reading = 0;
      w->buf = Malloc_TSD(ai->TSD, w->maxbuf);
   }

   if (w->reading) /* Pending IO, we can't do more */
      return(retval);

   /* not reading or no longer reading, initiate a new reading */
   if (w->rused && (w->rusedbegin != 0))
      memmove(w->buf, w->buf + w->rusedbegin, w->rused);
   w->rusedbegin = 0;

   if (w->rused < w->maxbuf)
   {
      w->reading = w->maxbuf - w->rused;
      ResetEvent(w->ol.hEvent);
      if (!ReadFile((HANDLE) hdl,
                    w->buf + w->rused,
                    w->reading,
                    &done,
                    ol))
      {
         done = GetLastError();
         if (done == ERROR_IO_PENDING)
            return(retval);

         w->reading = 0;
         if ((retval > 0) || (w->rused != 0))
            return(retval);

         if (done == ERROR_BROKEN_PIPE)
            return(0); /* "Normal" EOF */
         return(-EPIPE); /* guess */
      }

      /* success */
      w->reading = 0;
      w->rused += done;
   }

   if ((retval < 0) && w->rused) /* fresh data? return at once */
   {
      if (size > w->rused)
         size = w->rused;
      memcpy(buf, w->buf + w->rusedbegin, size);
      w->rusedbegin += size;
      w->rused -= size;
      retval = size;
   }
   return(retval);
}

/* __regina_write acts like write() but returns either an error (return code
 * == -errno) or the number of bytes written. EINTR and friends leads to a
 * re-write.
 * async_info is both a structure and a flag. If set, asynchronous IO shall
 * be used, otherwise blocked IO has to be used.
 * The file must be flushed if buf or size are 0. Even in this case -EAGAIN
 * may be returned.
 */
static int Win_write(int handle, const void *buf, unsigned size, void *async_info)
{
   AsyncInfo *ai;
   Win32IO *w;
   HANDLE hdl;
   unsigned i;
   OVERLAPPED *ol;
   DWORD done;
   int retval;
   int pending = 0;

   ai = async_info;
   hdl = (HANDLE) handle;
   retval = 0;

   if (!ai)
   {
      if ((buf == NULL) | (size == 0))
         return(0); /* flushing is useless here! */

      retval = 0;
      do {
         if (!WriteFile(hdl, buf, size, &done, NULL))
         {
            return(-EPIPE); /* guess */
         }
         retval += (int) done;
         buf = (const void *) ((char *) buf + done);
         size -= done;
      } while (retval < (int) size);
      return retval;
   }

   /* Async IO */
   for (i = 0, w = ai->h; i < 3; i++, w++)
   {
      if (w->hdl == hdl)
         break;
   }
   if (i == 3)
      return(-EINVAL);
   ol = &w->ol;

   /*
    * Reap any pending overlapped IO first.
    */
   if (w->wused) /* pending IO? */
   {
      if (!HasOverlappedIoCompleted(ol))
      {
         if ((w->wused == w->maxbuf) || (buf == NULL) || (size == 0))
            return(-EAGAIN);
         pending = 1;
      }
      else
      {
         if (!GetOverlappedResult(hdl, ol, &done, FALSE))
         {
            done = GetLastError();
            if (done == ERROR_IO_PENDING)
               return(-EAGAIN);
            return(-EPIPE); /* guess */
         }
         if (done < w->wused)
         {
            memmove(w->buf, w->buf + done, w->wused - done);
            w->wused -= done;
         }
         else
            w->wused = 0;
      }
   }

   if (((buf == NULL) || (size == 0)) && !w->wused)
   {
      return 0;
   }

   if (w->buf == NULL)
   {
      w->maxbuf = REGINA_MAX_BUFFER_SIZE;
      w->buf = Malloc_TSD(ai->TSD, w->maxbuf);
   }

   /*
    * At this point we may have set pending, in which case it is forbidden
    * to reinitiate a write or a semaphore event. We just may ADD something to
    * a buffer.
    */

   if (w->wused < w->maxbuf)
   {
      retval = (int) (w->maxbuf - w->wused);
      if (retval > (int) size)
      {
         retval = (int) size;
      }
      memcpy(w->buf + w->wused, buf, retval);
      w->wused += (unsigned) retval;
      if (pending)
      {
         return retval; /* fixes bug 945218 */
      }
   }
   else
      return -EAGAIN; /* Nothing written/copied, must be pending */

   ResetEvent(ol->hEvent);

   if (!WriteFile(hdl, w->buf, w->wused, &done, ol))
//   if (!WriteFile(hdl, w->buf, w->wused, NULL, ol))
   {
      done = (int) GetLastError();
      if (done == ERROR_IO_PENDING)
         return((retval == 0) ? -EAGAIN : retval);
      return(-EPIPE); /* guess */
   }
   /* No errors, thus success. We don't want to redo all the stuff. We
    * simply set your wait-semaphore to prevent sleeping.
    */
   SetEvent(ol->hEvent);

   return(retval);
}

/* create_async_info return an opaque structure to allow the process wait for
 * asyncronous IO. There are three IO slots (in, out, error) which can be
 * filled by add_waiter. The structure can be cleaned by reset_async_info.
 * The structure must be destroyed by delete_async_info.
 */
static void *Win_create_async_info(const tsd_t *TSD)
{
   AsyncInfo *retval;

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
static void Win_delete_async_info(void *async_info)
{
   AsyncInfo *ai;
   unsigned i;

   ai = async_info;
   if (ai == NULL)
      return;

   for (i = 0; i < 3;i++)
      Win_close((int) ai->h[i].hdl, ai);
   Free_TSD(ai->TSD, ai);
}

/* reset_async_info clears async_info in such a way that fresh add_waiter()
 * calls can be performed.
 */
static void Win_reset_async_info(void *async_info)
{
}

/* add_async_waiter adds a further handle to the asyncronous IO structure.
 * add_as_read_handle must be != 0 if the next operation shall be a
 * __regina_read, else it must be 0 for __regina_write.
 * Call reset_async_info before a wait-cycle to different handles and use
 * wait_async_info to wait for at least one IO-able handle.
 */
static void Win_add_async_waiter(void *async_info, int handle, int add_as_read_handle)
{
   AsyncInfo *ai;
   HANDLE hdl;
   unsigned i;

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
static void Win_wait_async_info(void *async_info)
{
   AsyncInfo *ai;
   Win32IO *w;
   unsigned i, used;
   HANDLE list[3];

   ai = async_info;
   for (i = 0, used = 0, w = ai->h; i < 3; i++, w++)
   {
      if (w->hdl == INVALID_HANDLE_VALUE)
         continue;
      if (w->ol.hEvent == INVALID_HANDLE_VALUE)
         continue;
      if (w->is_reader)
      {
         if (w->rused != 0) /* Still data to read? this is a killer! */
            return;
         if (w->reading == 0)
            continue;
      }
      else
      {
         if (w->wused == 0)
            continue;
      }
      if (HasOverlappedIoCompleted(&w->ol)) /* fresh meat arrived? */
         return;
      list[used++] = w->ol.hEvent;
   }
   if (used)
      WaitForMultipleObjects(used, list, FALSE, INFINITE);
}


#if defined(__WINS__) || defined(__EPOC32__)
# define Win_uname epoc32_uname
#elif defined(NEED_UNAME)
int Win_uname(struct regina_utsname *name)              /* MH 10-06-96 */
{                                                       /* MH 10-06-96 */
   SYSTEM_INFO sysinfo;
   OSVERSIONINFOEX osinfo;
   char computername[MAX_COMPUTERNAME_LENGTH+1];
   DWORD namelen = sizeof(computername);
   char buf[128];

   ZeroMemory( &sysinfo, sizeof(SYSTEM_INFO) );
   ZeroMemory( &osinfo, sizeof(OSVERSIONINFOEX) );
   GetSystemInfo( &sysinfo );
   osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   GetVersionEx( &osinfo );

   sprintf( buf, "%u.%u", osinfo.dwMajorVersion, osinfo.dwMinorVersion );
   if (osinfo.wServicePackMajor)
      sprintf( buf + strlen(buf), "SP%u", ( unsigned ) osinfo.wServicePackMajor );
   strncpy( name->version, buf, sizeof(name->version) - 1 );
   sprintf( name->release, "%u", osinfo.dwBuildNumber );

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
            else if ( osinfo.dwMinorVersion == 2 )
            {
               if ( osinfo.wProductType == VER_NT_WORKSTATION
               &&  sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 )
                  strcpy( name->sysname, "WINXP64" );
               else
                  strcpy( name->sysname, "WIN2003" );
            }
            else
               strcpy( name->sysname, "WIN2K" );
         }
         else if ( osinfo.dwMajorVersion == 6 )
         {
            if ( osinfo.dwMinorVersion == 0 )
            {
               if ( osinfo.wProductType == VER_NT_WORKSTATION )
                  strcpy( name->sysname, "WINVISTA" );
               else
                  strcpy( name->sysname, "WIN2008" );
            }
            else if ( osinfo.dwMinorVersion == 1 )
            {
               if ( osinfo.wProductType == VER_NT_WORKSTATION
               ||   osinfo.wProductType == 0 )
                  strcpy( name->sysname, "WIN7" );
               else
                  strcpy( name->sysname, "WIN2008R2" );
            }
            else if ( osinfo.dwMinorVersion == 2 )
            {
               if ( osinfo.wProductType == VER_NT_WORKSTATION
               ||   osinfo.wProductType == 0 )
                  strcpy( name->sysname, "WIN8" );
               else
                  strcpy( name->sysname, "WIN2012" );
            }
            /*
             * The following tests for Windows 8.1 won't work as GetVersionEx() has been
             * deprecated in Windows 8.1 and unless you completley change the way that
             * version information is determined for Windows, then Windows 8.1 and above will
             * always return WIN8. I'm not going to waste my time to
             * change Regina because of the stupid morons who work for Microsoft!!
             */
            else if ( osinfo.dwMinorVersion == 3 )
            {
               if ( osinfo.wProductType == VER_NT_WORKSTATION
               ||   osinfo.wProductType == 0 )
                  strcpy( name->sysname, "WIN8.1" );
               else
                  strcpy( name->sysname, "WIN2012R2" );
            }
            else
               strcpy( name->sysname, "UNKNOWN" );
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
   if ( GetComputerName(computername, &namelen) )
      strcpy( name->nodename, computername );
   else
      strcpy( name->nodename, "UNKNOWN" );

   if ( sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL )
   {
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
#if defined(PROCESSOR_INTEL_MIPS_R4000)
         case PROCESSOR_INTEL_MIPS_R4000:
            strcpy( name->machine, "mipsR4000" );
            break;
#endif
         default:
            strcpy( name->machine, "UNKNOWN" );
            break;
      }
   }
   else if ( sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 )
   {
      strcpy( name->machine, "alpha21064" );
   }
   else if ( sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 )
   {
      strcpy( name->machine, "x86_64" );
   }
   else
   {
      strcpy( name->machine, "UNKNOWN" );
   }

   return 0;
}
#else
int Win_uname(struct regina_utsname *name)
{
   struct utsname osdata;

   /*
    * Don't know whether utsname uses pointer or char[].
    * So just copy data.
    */
   if ( uname( &osdata ) < 0 )
   {
      memset( name, 0, sizeof(struct regina_utsname) );
      return -1;
   }

   strcpy( name->sysname,  osdata.sysname  );
   strcpy( name->nodename, osdata.nodename );
   strcpy( name->release,  osdata.release  );
   strcpy( name->version,  osdata.version  );
   strcpy( name->machine,  osdata.machine  );
   return 0;
}
#endif

static void Win_init(void);

OS_Dep_funcs __regina_OS_Win =
{
   Win_init,                           /* init                       */
   Win_setenv,                         /* setenv                     */
   Win_fork_exec,                      /* fork_exec                  */
   Win_wait,                           /* wait                       */
   Win_open_subprocess_connection,     /* open_subprocess_connection */
   Win_unblock_handle,                 /* unblock_handle             */
   Win_restart_file,                   /* restart_file               */
   Win_close,                          /* close                      */
   Win_close_special,                  /* close_special              */
   Win_read,                           /* read                       */
   Win_write,                          /* write                      */
   Win_create_async_info,              /* create_async_info          */
   Win_delete_async_info,              /* delete_async_info          */
   Win_reset_async_info,               /* reset_async_info           */
   Win_add_async_waiter,               /* add_async_waiter           */
   Win_wait_async_info,                /* wait_async_info            */
   Win_uname                           /* uname                      */
};

extern OS_Dep_funcs __regina_OS_Other;

static void Win_init(void)
{
#define WIN __regina_OS_Win
#define DOS __regina_OS_Other
   if (IsWin9X())
   {
      WIN.fork_exec                  = DOS.fork_exec;
      WIN.open_subprocess_connection = DOS.open_subprocess_connection;
      WIN.unblock_handle             = DOS.unblock_handle;
      WIN.restart_file               = DOS.restart_file;
      WIN.close                      = DOS.close;
      WIN.read                       = DOS.read;
      WIN.write                      = DOS.write;
      WIN.create_async_info          = DOS.create_async_info;
      WIN.delete_async_info          = DOS.delete_async_info;
      WIN.reset_async_info           = DOS.reset_async_info;
      WIN.add_async_waiter           = DOS.add_async_waiter;
      WIN.wait_async_info            = DOS.wait_async_info;
      WIN.wait                       = DOS.wait;
   }
}
