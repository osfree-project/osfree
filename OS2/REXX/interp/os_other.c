/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 2005 Florian Grosse-Coosmann
 *
 *  This file contains OS specific code not matching known systems or
 *  for "trivial" systems. Even Win32 systems use this code in order
 *  to get this graphical DOS extension called Win9x running.
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


#if defined(__EMX__)
# define INCL_BASE
# include <os2.h>
# define DONT_TYPEDEF_PFN
#endif

#include "rexx.h"
#include "utsname.h"

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
#include <assert.h>

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

#if defined(MAC) || (defined(__WATCOMC__) && !defined(__QNX__)) || defined(_MSC_VER) || defined(__SASC) || defined(__MINGW32__) || defined(__BORLANDC__) || defined(__EPOC32__) || defined(__WINS__) || defined(__LCC__) || defined(SKYOS)
  /*
   * MAC OSX hopefully uses the posix branch.
   */
# define NEED_UNAME
#else                                                   /* MH 10-06-96 */
# if defined(WIN32) && defined(__IBMC__)                /* LM 26-02-99 */
#  include "utsname.h"
#  define NEED_UNAME
#  include <io.h>
# else
#  ifndef VMS
#   include <sys/param.h>                                 /* MH 10-06-96 */
#  endif
#  include <sys/utsname.h>                               /* MH 10-06-96 */
# endif
#endif

#if defined(MAC) || defined(GO32) || defined (__EMX__) || (defined(__WATCOMC__) && !defined(__QNX__)) || defined(_MSC_VER) || defined(DJGPP) || defined(__CYGWIN32__) || defined(__BORLANDC__) || defined(__MINGW32__) || defined(__WINS__) || defined(__EPOC32__) ||defined(__LCC__)
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

/*
 * This flavour is the default style. It doesn't use pipes. It uses temporary
 * files instead.
 * Slow, but shall work with all machines.
 */

static int Oth_setenv( const char *name, const char *value )
{
   char *cmd;

   if (value == NULL)
      value = "";
   cmd = malloc( strlen(name) + strlen(value) + 2);
   sprintf( cmd, "%s=%s", name, value );
   putenv( cmd );
   free( cmd );
   return 1;
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
static int Oth_fork_exec(tsd_t *TSD, environment *env, const char *cmdline, int *rcode)
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
         rc = Oth_fork_exec(TSD, &e, new_cmdline, &rc);
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
         rc = Oth_fork_exec(TSD, &e, new_cmdline, &rc);
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
# else
         args = makeargs(cmdline, '\\');
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
         if (args != NULL)
            destroyargs(args);
         errno = EINVAL;
         rc = -1;
         break;
   }

   eno = errno;
   STD_RESTORE(saved_in, 0);
   STD_RESTORE(saved_out, 1);
   STD_RESTORE(saved_err, 2);
   if (args != NULL)
      destroyargs(args);
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

/* wait waits for a process started by fork_exec.
 * In general, this is called after the complete IO to the called process but
 * it isn't guaranteed. Never call if you don't expect a sudden death of the
 * subprocess.
 * Returns the exit status of the subprocess under normal circumstances. If
 * the subprocess has died by a signal, the return value is -signalnumber.
 */
static int Oth_wait(int process)
{
   return(process + 0x4000);
}

/*
 * local_mkstemp() works mostly like the commands
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
#if defined(_MSC_VER) && ( !defined(__EPOC32__) && !defined(__WINS__) ) /* currently not used but what's about CE ? */
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

/* open_subprocess_connection acts like the unix-known function pipe and sets
 * ep->RedirectedFile if necessary. Just in the latter case ep->data
 * is set to the filename.
 * Close the handles with __regina_close later.
 * Do IO by using __regina_read() and __regina_write().
 */
static int Oth_open_subprocess_connection(const tsd_t *TSD, environpart *ep)
{
   char *name;
   int eno;

   name = MallocTSD(REXX_PATH_MAX + 1);
   /* Remember to create two handles to be "pipe()"-conform. */
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
static void Oth_unblock_handle( int *handle, void *async_info )
{
   (handle = handle);
   (async_info = async_info);
}

/* restart_file sets the file pointer of the given handle to the beginning.
 */
static void Oth_restart_file(int hdl)
{
   lseek(hdl, 0l, SEEK_SET);
}

/* close acts like close() but closes a handle returned by
 * open_subprocess_connection.
 * async_info MAY be used to delete the handle from the internal list of
 * watched handles.
 */
static int Oth_close(int handle, void *async_info)
{
   (async_info = async_info);
   return(close(handle));
}

/*
 * close_special acts like close() but closes any OS specific handle.
 * The close happens if the handle is not -1. A specific operation may be
 * associated with this. Have a look for occurances of "hdls[2]".
 */
static void Oth_close_special( int handle )
{
   assert( handle == -1 );
}

/* read acts like read() but returns either an error (return code
 * == -errno) or the number of bytes read. EINTR and friends leads to a
 * re-read.
 * use_blocked_io is a flag. If set, the handle is set to blocked IO and
 * we shall use blocked IO here.
 */
static int Oth_read(int hdl, void *buf, unsigned size, void *async_info)
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

/* write acts like write() but returns either an error (return code
 * == -errno) or the number of bytes written. EINTR and friends leads to a
 * re-write.
 * use_blocked_io is a flag. If set, the handle is set to blocked IO and
 * we shall use blocked IO here.
 * The file must be flushed if buf or size are 0.
 */
static int Oth_write(int hdl, const void *buf, unsigned size, void *async_info)
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
static void *Oth_create_async_info(const tsd_t *TSD)
{
   (TSD = TSD);
   return(NULL);
}

/* delete_async_info deletes the structure created by create_async_info and
 * all of its components.
 */
static void Oth_delete_async_info(void *async_info)
{
   (async_info = async_info);
}

/* reset_async_info clear async_info in such a way that fresh add_waiter()
 * calls can be performed.
 */
static void Oth_reset_async_info(void *async_info)
{
   (async_info = async_info);
}

/* add_async_waiter adds a further handle to the asyncronous IO structure.
 * add_as_read_handle must be != 0 if the next operation shall be a
 * __regina_read, else it must be 0 for __regina_write.
 * Call reset_async_info before a wait-cycle to different handles and use
 * wait_async_info to wait for at least one IO-able handle.
 */
static void Oth_add_async_waiter(void *async_info, int handle, int add_as_read_handle)
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
static void Oth_wait_async_info(void *async_info)
{
   (async_info = async_info);
}

int Oth_uname(struct regina_utsname *name)
{
   /*
    * Set up values for utsname structure...
    */
#if defined(_AMIGA)
   strcpy( name->sysname,  "AMIGA" );
   strcpy( name->version,  "UNKNOWN" );
   strcpy( name->release,  "UNKNOWN" );
   strcpy( name->nodename, "standalone" );
   strcpy( name->machine,  "m68k" );
#elif defined(MAC)
   strcpy( name->sysname,  "MAC" );
   strcpy( name->version,  "UNKNOWN" );
   strcpy( name->release,  "UNKNOWN" );
   strcpy( name->nodename, "standalone" );
   strcpy( name->machine,  "m68k" );
#else
   strcpy( name->sysname,  "UNKNOWN" );
   strcpy( name->version,  "UNKNOWN" );
   strcpy( name->release,  "UNKNOWN" );
   strcpy( name->nodename, "UNKNOWN" );
   strcpy( name->machine,  "UNKNOWN" );
#endif

   return 0;
}

static void Oth_init(void)
{
}

OS_Dep_funcs __regina_OS_Other =
{
   Oth_init,                           /* init                       */
   Oth_setenv,                         /* setenv                     */
   Oth_fork_exec,                      /* fork_exec                  */
   Oth_wait,                           /* wait                       */
   Oth_open_subprocess_connection,     /* open_subprocess_connection */
   Oth_unblock_handle,                 /* unblock_handle             */
   Oth_restart_file,                   /* restart_file               */
   Oth_close,                          /* close                      */
   Oth_close_special,                  /* close_special              */
   Oth_read,                           /* read                       */
   Oth_write,                          /* write                      */
   Oth_create_async_info,              /* create_async_info          */
   Oth_delete_async_info,              /* delete_async_info          */
   Oth_reset_async_info,               /* reset_async_info           */
   Oth_add_async_waiter,               /* add_async_waiter           */
   Oth_wait_async_info,                /* wait_async_info            */
   Oth_uname                           /* uname                      */
};
