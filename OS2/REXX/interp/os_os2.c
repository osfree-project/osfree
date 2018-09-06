/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 2005 Florian Grosse-Coosmann
 *
 *  System interfacing functions for OS/2 and EMX
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

#define INCL_BASE
#include <os2.h>
#define DONT_TYPEDEF_PFN

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

#if defined(HAVE_PROCESS_H)
# include <process.h>
#endif

#include "utsname.h"                                   /* MH 10-06-96 */

#if defined (__EMX__) || defined(__WATCOMC__)
# if defined(__EMX__)
#  include <sys/wait.h>
# endif
#endif

#include <io.h>

static int Os2_setenv( const char *name, const char *value )
{
#if defined(HAVE_SETENV)
   setenv( name, value, 1 );
   return 1;
#else
   char *cmd;

   if (value == NULL)
      value = "";
   cmd = malloc( strlen(name) + strlen(value) + 2);
   sprintf( cmd, "%s=%s", name, value );
   putenv( cmd );
   free( cmd );
   return 1;
#  endif
}

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
static int Os2_fork_exec(tsd_t *TSD, environment *env, const char *cmdline, int *rcode)
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
         rc = Os2_fork_exec(TSD, &e, new_cmdline, &rc);
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
         rc = Os2_fork_exec(TSD, &e, new_cmdline, &rc);
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
   /* SUBENVIR_REXX must(!!) fork if we are here! */
   {
      extern OS_Dep_funcs __regina_OS_Other;

      if ((_osmode != OS2_MODE) && (env->subtype != SUBENVIR_REXX))
         return(__regina_OS_Other.fork_exec(TSD, env, cmdline, rcode));
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

   return ( rc == -1 ) ? 0 : rc;
#undef SET_MAXHDLS
#undef SET_MAXHDL
#undef STD_RESTORE
#undef STD_REDIR
}

/* wait waits for a process started by fork_exec.
 * In general, this is called after the complete IO to the called process but
 * it isn't guaranteed. Never call if you don't expect a sudden death of the
 * subprocess.
 * Returns the exit status of the subprocess under normal circumstances. If
 * the subprocess has died by a signal, the return value is -signalnumber.
 */
static int Os2_wait(int process)
{
   int rc, retval, status;

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
 * Do IO by using Os2_read() and Os2_write().
 */
static int Os2_open_subprocess_connection(const tsd_t *TSD, environpart *ep)
{
#define MAGIC_MAX 2000000 /* Much beyond maximum, see below */
   static volatile unsigned BaseIndex = MAGIC_MAX;
   char buf[40];
   unsigned i;
   unsigned start,run;
   ULONG rc, openmode, dummy;
   HPIPE in = (HPIPE) -1, out;

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
static void Os2_unblock_handle( int *handle, void *async_info )
{
   AsyncInfo *ai = async_info;
   int i;
   ULONG rc;

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
static void Os2_restart_file(int hdl)
{
   lseek(hdl, 0l, SEEK_SET); /* unused! */
}

/* close acts like close() but closes a handle returned by
 * open_subprocess_connection.
 * async_info MAY be used to delete the handle from the internal list of
 * watched handles.
 */
static int Os2_close(int handle, void *async_info)
{
   AsyncInfo *ai = async_info;
   int i;

   if ((handle != -1) && (ai != NULL))
      for (i = 0;i < 3;i++)
         if ((int) ai->hdl[i] == handle)
            DosSetNPipeSem((HPIPE) handle, NULLHANDLE, 0);
   return(DosClose((HFILE) handle));
}

/*
 * close_special acts like close() but closes any OS specific handle.
 * The close happens if the handle is not -1. A specific operation may be
 * associated with this. Have a look for occurances of "hdls[2]".
 */
static void Os2_close_special( int handle )
{
   assert( handle == -1 );
}

/* read acts like read() but returns either an error (return code
 * == -errno) or the number of bytes read. EINTR and friends leads to a
 * re-read.
 * use_blocked_io is a flag. If set, the handle is set to blocked IO and
 * we shall use blocked IO here.
 */
static int Os2_read(int hdl, void *buf, unsigned size, void *async_info)
{
   ULONG rc;
   ULONG done;

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

/* write acts like write() but returns either an error (return code
 * == -errno) or the number of bytes written. EINTR and friends leads to a
 * re-write.
 * use_blocked_io is a flag. If set, the handle is set to blocked IO and
 * we shall use blocked IO here.
 * The file must be flushed if buf or size are 0.
 */
static int Os2_write(int hdl, const void *buf, unsigned size, void *async_info)
{
   ULONG rc;
   ULONG done;

   if ((buf == NULL) || (size == 0)) /* nothing to do for flushing buffers */
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
static void *Os2_create_async_info(const tsd_t *TSD)
{
   AsyncInfo *ai;

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
static void Os2_delete_async_info(void *async_info)
{
   AsyncInfo *ai = async_info;

   if (ai == NULL)
      return;
   DosCloseEventSem(ai->sem);
   Free_TSD(ai->TSD, ai);
}

/* reset_async_info clear async_info in such a way that fresh add_waiter()
 * calls can be performed.
 */
static void Os2_reset_async_info(void *async_info)
{
   AsyncInfo *ai = async_info;
   ULONG ul;

   DosResetEventSem(ai->sem, &ul);
   ai->mustwait = 0;
}

/* add_async_waiter adds a further handle to the asyncronous IO structure.
 * add_as_read_handle must be != 0 if the next operation shall be a
 * Os2_read, else it must be 0 for Os2_write.
 * Call reset_async_info before a wait-cycle to different handles and use
 * wait_async_info to wait for at least one IO-able handle.
 */
static void Os2_add_async_waiter(void *async_info, int handle, int add_as_read_handle)
{
   AsyncInfo *ai = async_info;
   int i;

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
static void Os2_wait_async_info(void *async_info)
{
   AsyncInfo *ai = async_info;

   if (ai->mustwait)
      DosWaitEventSem(ai->sem, SEM_INDEFINITE_WAIT);
}

static int Os2_uname(struct regina_utsname *name)
{
   strcpy( name->sysname, "OS2" );
   sprintf( name->version, "%d" ,_osmajor );
   sprintf( name->release, "%d" ,_osminor );
   strcpy( name->nodename, "standalone" );
   strcpy( name->machine, "i386" );

   return 0;
}

static void Os2_init(void);

OS_Dep_funcs __regina_OS_Os2 =
{
   Os2_init,                           /* init                       */
   Os2_setenv,                         /* setenv                     */
   Os2_fork_exec,                      /* fork_exec                  */
   Os2_wait,                           /* wait                       */
   Os2_open_subprocess_connection,     /* open_subprocess_connection */
   Os2_unblock_handle,                 /* unblock_handle             */
   Os2_restart_file,                   /* restart_file               */
   Os2_close,                          /* close                      */
   Os2_close_special,                  /* close_special              */
   Os2_read,                           /* read                       */
   Os2_write,                          /* write                      */
   Os2_create_async_info,              /* create_async_info          */
   Os2_delete_async_info,              /* delete_async_info          */
   Os2_reset_async_info,               /* reset_async_info           */
   Os2_add_async_waiter,               /* add_async_waiter           */
   Os2_wait_async_info,                /* wait_async_info            */
   Os2_uname                           /* uname                      */
};


static void Os2_init(void)
{
#undef OS2
#define OS2 __regina_OS_Os2
#define DOS __regina_OS_Other

#ifdef __EMX__ /* redirect this call to the non-OS/2-code if DOS is running */
   if (_osmode != OS2_MODE)
   {
      extern OS_Dep_funcs __regina_OS_Other;

      OS2.fork_exec                  = DOS.fork_exec;
      OS2.wait                       = DOS.wait;
      OS2.open_subprocess_connection = DOS.open_subprocess_connection;
      OS2.unblock_handle             = DOS.unblock_handle;
      OS2.restart_file               = DOS.restart_file;
      OS2.close                      = DOS.close;
      OS2.read                       = DOS.read;
      OS2.write                      = DOS.write;
      OS2.create_async_info          = DOS.create_async_info;
      OS2.delete_async_info          = DOS.delete_async_info;
      OS2.reset_async_info           = DOS.reset_async_info;
      OS2.add_async_waiter           = DOS.add_async_waiter;
      OS2.wait_async_info            = DOS.wait_async_info;
   }
#endif
}
