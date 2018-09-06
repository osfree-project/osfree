/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 2005 Florian Grosse-Coosmann
 *
 *  System interfacing functions for unix and equivalent systems
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
#include "utsname.h"

#include <stdlib.h>
#include <string.h>

#if defined(HAVE_STRING_H)
# include <stdio.h>
#endif

#if defined(DOS)
# ifdef _POSIX_SOURCE   /* emulation system? */
#  undef _POSIX_SOURCE
# endif
# include <dos.h>
#endif

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

#if defined(HAVE_SHARE_H)
# include <share.h>
#endif

#if defined(HAVE_TIME_H)
# include <time.h>
#endif

#if defined(HAVE_ERRNO_H)
# include <errno.h>
#endif

#if defined(HAVE_SIGNAL_H)
# include <signal.h>
#endif

#if defined(HAVE_ASSERT_H)
# include <assert.h>
#endif

#if defined(HAVE_SYS_UTSNAME_H)
# include <sys/utsname.h>
#endif

#if defined(HAVE_POLL) && (defined(HAVE_POLL_H) || defined(HAVE_SYS_POLL_H))
# if defined(HAVE_POLL_H)
#  include <poll.h>
# else
#  include <sys/poll.h>
# endif
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

static int Unx_setenv( const char *name, const char *value )
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

/*
 * fork_exec spawns a new process with the given commandline.
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
int Unx_fork_exec(tsd_t *TSD, environment *env, const char *cmdline, int *rcode)
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

            if ((new_cmdline = (char *)malloc(len)) == NULL)
               raise( SIGKILL ); /* This is a separate process, raise() is allowed */

            if (argv0 != NULL) /* always the best choice */
            {
               strcpy(new_cmdline, argv0);
               strcat(new_cmdline, " ");
               strcat(new_cmdline, cmdline);
               args = makeargs(new_cmdline, '\\');
               execv(*args, args);
               destroyargs(args);
            }

            /* load an interpreter by name from the path */
            for (i = 0; i < (int) (sizeof(interpreter) / sizeof(interpreter[0]));i++)
            {
               strcpy(new_cmdline, interpreter[i]);
               strcat(new_cmdline, " ");
               strcat(new_cmdline, cmdline);
               args = makeargs(new_cmdline, '\\');
               execvp(*args, args);
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

/* wait waits for a process started by fork_exec.
 * In general, this is called after the complete IO to the called process but
 * it isn't guaranteed. Never call if you don't expect a sudden death of the
 * subprocess.
 * Returns the exit status of the subprocess under normal circumstances. If
 * the subprocess has died by a signal, the return value is -(100+signalnumber)
 */
static int Unx_wait(int process)
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
static int Unx_open_subprocess_connection(const tsd_t *TSD, environpart *ep)
{
   return(pipe(ep->hdls));
}

/* sets the given handle to the non-blocking mode. The value may change.
 * async_info CAN be used to add the handle to the internal list of watched
 * handles.
 */
static void Unx_unblock_handle( int *handle, void *async_info )
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
static void Unx_restart_file(int hdl)
{
   lseek(hdl, 0l, SEEK_SET);
}

/* close acts like close() but closes a handle returned by
 * open_subprocess_connection.
 * async_info MAY be used to delete the handle from the internal list of
 * watched handles.
 */
static int Unx_close(int handle, void *async_info)
{
   return(close(handle));
}

/*
 * close_special acts like close() but closes any OS specific handle.
 * The close happens if the handle is not -1. A specific operation may be
 * associated with this. Have a look for occurances of "hdls[2]".
 */
static void Unx_close_special( int handle )
{
   assert( handle == -1 );
}

/* read acts like read() but returns either an error (return code
 * == -errno) or the number of bytes read. EINTR and friends leads to a
 * re-read.
 * use_blocked_io is a flag. If set, the handle is set to blocked IO and
 * we shall use blocked IO here.
 */
static int Unx_read(int hdl, void *buf, unsigned size, void *async_info)
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

/* write acts like write() but returns either an error (return code
 * == -errno) or the number of bytes written. EINTR and friends leads to a
 * re-write.
 * use_blocked_io is a flag. If set, the handle is set to blocked IO and
 * we shall use blocked IO here.
 * The file must be flushed if buf or size are 0.
 */
static int Unx_write(int hdl, const void *buf, unsigned size, void *async_info)
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
static void *Unx_create_async_info(const tsd_t *TSD)
{
   AsyncInfo *ai = (AsyncInfo *)MallocTSD(sizeof(AsyncInfo));

   ai->TSD = TSD;
   return(ai);
}
/* delete_async_info deletes the structure created by create_async_info and
 * all of its components.
 */
static void Unx_delete_async_info(void *async_info)
{
   AsyncInfo *ai = (AsyncInfo *)async_info;

   if (ai == NULL)
      return;
   Free_TSD(ai->TSD, ai);
}

#if defined(POLLIN) && defined(HAVE_POLL) /* we have poll() */

/* reset_async_info clear async_info in such a way that fresh add_waiter()
 * calls can be performed.
 */
static void Unx_reset_async_info(void *async_info)
{
   AsyncInfo *ai = (AsyncInfo *)async_info;

   ai->_p_cnt = 0;
}

/* add_async_waiter adds a further handle to the asyncronous IO structure.
 * add_as_read_handle must be != 0 if the next operation shall be a
 * read, else it must be 0 for write.
 * Call reset_async_info before a wait-cycle to different handles and use
 * wait_async_info to wait for at least one IO-able handle.
 */
static void Unx_add_async_waiter(void *async_info, int handle, int add_as_read_handle)
{
   AsyncInfo *ai = (AsyncInfo *)async_info;

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
static void Unx_wait_async_info(void *async_info)
{
   AsyncInfo *ai = (AsyncInfo *)async_info;

   if (ai->_p_cnt)
      poll(ai->_p, ai->_p_cnt, -1);
}

#else /* end of POLLIN, must be select */

/* reset_async_info clear async_info in such a way that fresh add_waiter()
 * calls can be performed.
 */
static void Unx_reset_async_info(void *async_info)
{
   AsyncInfo *ai = async_info;

   FD_ZERO( &ai->_p_in );
   FD_ZERO( &ai->_p_out );
   ai->_p_max = -1 ;
}

/* add_async_waiter adds a further handle to the asyncronous IO structure.
 * add_as_read_handle must be != 0 if the next operation shall be a
 * read, else it must be 0 for write.
 * Call reset_async_info before a wait-cycle to different handles and use
 * wait_async_info to wait for at least one IO-able handle.
 */
static void Unx_add_async_waiter(void *async_info, int handle, int add_as_read_handle)
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
static void Unx_wait_async_info(void *async_info)
{
   AsyncInfo *ai = async_info;

   if (ai->_p_max >= 0)
      select( ai->_p_max+1, &ai->_p_in, &ai->_p_out, NULL, NULL);
}

#endif /* POLLIN or select */


static int Unx_uname(struct regina_utsname *name)
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

static void Unx_init(void)
{
}

OS_Dep_funcs __regina_OS_Unx =
{
   Unx_init,                           /* init                       */
   Unx_setenv,                         /* setenv                     */
   Unx_fork_exec,                      /* fork_exec                  */
   Unx_wait,                           /* wait                       */
   Unx_open_subprocess_connection,     /* open_subprocess_connection */
   Unx_unblock_handle,                 /* unblock_handle             */
   Unx_restart_file,                   /* restart_file               */
   Unx_close,                          /* close                      */
   Unx_close_special,                  /* close_special              */
   Unx_read,                           /* read                       */
   Unx_write,                          /* write                      */
   Unx_create_async_info,              /* create_async_info          */
   Unx_delete_async_info,              /* delete_async_info          */
   Unx_reset_async_info,               /* reset_async_info           */
   Unx_add_async_waiter,               /* add_async_waiter           */
   Unx_wait_async_info,                /* wait_async_info            */
   Unx_uname                           /* uname                      */
};
