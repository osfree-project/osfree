/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 2001-2004  Florian Groﬂe-Coosmann
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
 *
 ******************************************************************************
 *
 * Asynchroneous thread multiplexer with a parallel use of the REXXSAA API.
 *
 * This example works with Win32 as with OS/2 or Posix threads.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <time.h>

#ifdef POSIX_THREADS
# include <sys/time.h>
# include <unistd.h>
# include <pthread.h>
# define USE_SEMAPHORES 0
# ifdef _POSIX_SEMAPHORES
#  include <semaphore.h>
#  include <signal.h>
#  undef USE_SEMAPHORES
#  define USE_SEMAPHORES 1
# endif
#endif

#ifdef OS2_THREADS
# include <io.h>
# include <stddef.h>
# include <process.h>
# define INCL_DOSSEMAPHORES
# define INCL_ERRORS
# include <os2.h>
# define CHAR_TYPEDEFED
# define SHORT_TYPEDEFED
# define LONG_TYPEDEFED
# ifndef _OS2EMX_H
#  define _OS2EMX_H       /* prevents PFN from defining (Watcom) */
# endif
#endif

#ifdef _MSC_VER
/* This picky compiler claims about unused formal parameters.
 * This is correct but hides (for human eyes) other errors since they
 * are many and we can't reduce them all.
 * Error 4100 is "unused formal parameter".
 */
# pragma warning(disable:4100 4115 4201 4214 4514)
#endif

#ifdef WIN32_THREADS
# define WIN32_LEAN_AND_MEAN
# include <process.h>
# include <windows.h>
# include <io.h>
#endif

#ifdef _MSC_VER
# pragma warning(default:4100 4115 4201 4214)
#endif

#define INCL_RXSHV
#define INCL_RXFUNC
#define INCL_RXSYSEXIT
#define INCL_RXSUBCOM

#ifdef USE_OREXX
# include "rexx.h"
#else
# include "rexxsaa.h"
#endif

/*
 * MAX_THREADS is the number of parallel starting threads. 20 is a good
 * maximum.
 */
#define MAX_THREADS 20

/*
 * MAX_RUN defines the number of loops each thread has to perform.
 * Don't modify.
 */
#define MAX_RUN ( sizeof( unsigned ) * CHAR_BIT )

/*
 * TOTAL_THREADS is the number of threads which shall be created. 2000 should
 * be sufficient to detect memory leaks, etc.
 * Can be overwritten with -t command line switch
 */
#define TOTAL_THREADS 1500
int total_threads = TOTAL_THREADS;

/*
 * timeout_seconds is the number of seconds a thread is allowed to live before
 * stopping the program. Overwrite with -s command line switch
 */
int timeout_seconds = 3;

#ifdef POSIX_THREADS
/*
 * See below at WIN32_THREADS for a description.
 */
#define ThreadIndexType pthread_t
#define my_threadidx() pthread_self()
#define my_threadid() pthread_self()
#define THREAD_RETURN void *
#define THREAD_CONVENTION
static pthread_t thread[MAX_THREADS];
#endif

#ifdef OS2_THREADS
/*
 * See below at WIN32_THREADS for a description.
 */
#define ThreadIndexType int
#define my_threadidx() *_threadid
#define my_threadid() *_threadid
#define THREAD_RETURN void
#define THREAD_RETURN_VOID 1
#define THREAD_CONVENTION
static int thread[MAX_THREADS];
#endif

#ifdef WIN32_THREADS
/*
 * ThreadIndexType is the type of my_threadidx which shall be the thread
 * identifier.
 */
#define ThreadIndexType DWORD

/*
 * my_threadidx() has to return the thread identifier of the current thread.
 */
#define my_threadidx() GetCurrentThreadId()

/*
 * my_threadid() has to return the thread's handle of the current thread if
 * such a thing exists.
 */
#define my_threadid() GetCurrentThread()

/*
 * THREAD_RETURN defines the return value type of the thread creation function.
 */
#define THREAD_RETURN unsigned

/*
 * THREAD_CONVENTION defines the calling convention of the thread creation
 * function. It may be the empty string for cdecl.
 */
#define THREAD_CONVENTION __stdcall

/*
 * thread will hold the handle of each thread.
 */
static HANDLE thread[MAX_THREADS];
#endif


/*
 * threadx will hold the identifier of each thread (in opposite to "thread").
 */
static ThreadIndexType threadx[MAX_THREADS];

/*
 * State manages the state of each possible parallel thread in the arrays
 * "thread" and "threadx".
 */
static enum {
   Ready = 0,  /* The thread's slot may be used */
   Running,    /* Thread has been started */
   Stopped     /* This value is used by a thread to signal its death. */
} State[MAX_THREADS];

/*
 * found contains the number of properly detected lines by the thread.
 * Lines matching a pattern are expected. The values are used bitwise.
 */
static unsigned found[MAX_THREADS];

/*
 * GlobalError contains the global error code (and return value).
 * This variables type shall be threadsafe.
 */
static int GlobalError = 0;

/*
 * stdout_is_tty is a flag which is set when we may drop additional garbage
 * to the output.
 */
static int stdout_is_tty = 0;

/*
 * UseInstore manages the state of the instore-processing. Instore macros
 * are precompiled macros and shall run faster than normal ones.
 */
static enum {
   UnUsed = 0, /* Don't use instore macros at all. */
   FirstRun,   /* This is the first run when compiling the macros */
   DoInstore   /* We have to use the instore macro */
} UseInstore = UnUsed;

/*
 * InstoreBuf will hold the compiled macro.
 */
static void *InstoreBuf = NULL;

/*
 * InstoreLen will hold the compiled macro's length if InstoreBuf is non-NULL.
 */
static unsigned InstoreLen;

static void ThreadHasStopped(unsigned position);

static char *program_name = NULL;

/*
 * We redirect Rexx' output. This is the redirection handler.
 * We expect the string "Loop <x> in thread <y>" where x is a running
 * counter and y is the thread identifier returned by Regina.
 */
LONG APIENTRY instore_exit( LONG ExNum, LONG Subfun, PEXIT PBlock )
{
   RXSIOSAY_PARM *psiosay;
   char buf[256];           /* enough to hold the data */
   RXSTRING rx;
   unsigned len,tid,loop;
   int rc,idx;

   if ( ( ExNum != RXSIO ) || ( Subfun != RXSIOSAY ) ) /* unexpected? */
      return RXEXIT_NOT_HANDLED;

   if ( GlobalError )
      return RXEXIT_HANDLED;

   /*
    * We have to check the data after fetching it from the parameter block.
    */
   psiosay = ( RXSIOSAY_PARM * ) PBlock;
   rx = psiosay->rxsio_string;
   if ( !RXVALIDSTRING( rx ) )
   {
      fprintf( stderr, "\n"
                      "Thread %lu gives an invalid string for a SAY output.\n",
                       ( unsigned long ) my_threadidx() );
      GlobalError = 1;
      return RXEXIT_HANDLED;
   }

   len = RXSTRLEN( rx );
   if ( len >= sizeof(buf) )
      len = sizeof(buf) - 1;  /* This shall NOT happen, but it's irrelevant */
   memcpy( buf, RXSTRPTR( rx ), len );
   buf[len] = '\0'; /* We have a sscanf-able string */
   rc = sscanf( buf, "Loop %u in thread %u", &loop, &tid );
   if ( rc != 2 )
   {
      fprintf( stderr, "\n"
                       "Thread %lu gives an unexpected SAY output \"%s\".\n",
                       ( unsigned long ) my_threadidx(), buf );
      GlobalError = 1;
      return RXEXIT_HANDLED;
   }

   /*
    * Is the wrong thread addressed? This may be a common error.
    */
   if ( ( ThreadIndexType ) tid != my_threadidx() )
   {
      fprintf( stderr, "\n"
                     "Thread %lu claims an incorrect thread identifier %lu.\n",
                       ( unsigned long ) my_threadidx(),
                       ( unsigned long ) tid );
      GlobalError = 1;
      return RXEXIT_HANDLED;
   }

   /*
    * Is this a known thread?
    */
   for ( idx = 0; idx < MAX_THREADS; idx++ )
   {
      if ( threadx[idx] == ( ThreadIndexType ) tid )
         break;
   }

   if ( idx >= MAX_THREADS )
   {
      fprintf( stderr, "\n"
                       "Thread %lu can't be found in the thread table.\n",
                       ( unsigned long ) my_threadidx() );
      GlobalError = 1;
      return RXEXIT_HANDLED;
   }

   /*
    * Check the loop number. We may have lost or duplicated data.
    */
   rc = 0; /* OK */
   if ( ( loop < 1 ) || ( loop > MAX_RUN ) )
      rc = 1;
   else
   {
      loop = 1 << ( loop - 1 ); /* Bitmask for the loop */
      if ( found[idx] & loop )
         rc = 1;                /* already found */
      found[idx] |= loop;
   }

   if ( rc )
   {
      fprintf( stderr, "\n"
                       "Thread %lu's loop doesn't run continuously.\n",
                       ( unsigned long ) my_threadidx() );
         GlobalError = 1;
   }

   return RXEXIT_HANDLED;
}

/*
 * instore is a separate thread and invokes a Rexx script.
 * It runs a loop (inside Rexx) and checks for errors.
 * The return value is 0 if a return value is used at all.
 * The argument data is the index of the thread within threadx.
 */
THREAD_RETURN THREAD_CONVENTION instore( void *data )
{
   RXSTRING Instore[2];
   RXSYSEXIT Exits[2];
   char instore_buf[256];
   int rc;

   /*
    * signal that we are alive.
    */
   threadx[(unsigned) data] = my_threadidx();

   /*
    * Register an exit handler which shall check Regina's output of the thread.
    */
   RexxRegisterExitExe( "ExitHandler",       /* name of the handler        */
#ifdef RX_WEAKTYPING
                        (PFN) instore_exit,  /* entry point of the handler */
#else
                        instore_exit,        /* entry point of the handler */
#endif
                        NULL );              /* user area of the handler   */

   /*
    * Build up the structure which informs Regina to use the exit handler.
    */
   Exits[0].sysexit_name = "ExitHandler";
   Exits[0].sysexit_code = RXSIO;
   Exits[1].sysexit_code = RXENDLST;

   sprintf( instore_buf, "Do i = 1 To %u;"
                         "say 'Loop' i 'in thread' gettid();"
                         "End;"
                         "Return 0",
                         (unsigned) MAX_RUN );
   Instore[0].strptr = instore_buf;
   Instore[0].strlength = strlen( Instore[0].strptr );
   if ( UseInstore == DoInstore )
   {
      /*
       * We don't need a script any longer. Instead we pass a compiled script
       * for the interpreter. You may or may not pass the original script,
       * it is ignored.
       */
      Instore[1].strptr = (char *)InstoreBuf;
      Instore[1].strlength = InstoreLen;
   }
   else
   {
      Instore[1].strptr = NULL;
   }
   rc = RexxStart( 0,         /* ArgCount                  */
                   NULL,      /* ArgList                   */
                   "Testing", /* ProgramName               */
                   Instore,   /* Instore (source/compiled) */
                   "Foo",     /* EnvironmentName           */
                   RXCOMMAND, /* CallType                  */
                   Exits,     /* ExitHandlerList           */
                   NULL,      /* ReturnCode (ignored)      */
                   NULL );    /* ReturnValue (ignored)     */
   switch ( UseInstore )
   {
      case UnUsed:
         /*
          * We will get an instore macro even if not desired. Delete it.
          */
         if ( Instore[1].strptr )
            RexxFreeMemory( Instore[1].strptr );
         else
         {
            GlobalError = 1;
            fprintf( stderr, "\n"
                             "Didn't got the instore macro.\n" );
         }
         break;

      case FirstRun:
         /*
          * On the first run save the instore macro for later use.
          */
         if ( Instore[1].strptr )
         {
            InstoreBuf = Instore[1].strptr;
            InstoreLen = Instore[1].strlength;
         }
         else
         {
            GlobalError = 1;
            fprintf( stderr, "\n"
                             "Didn't got the instore macro.\n" );
         }
         break;

      default:
         /*
          * I don't know if the standard allows a success and a return value
          * of NULL in Instore[1]. Ignore it. It will be detected later.
          * True application should check the return code of RexxStart().
          */
         break;
   }
   RexxDeregisterExit( "ExitHandler", /* name of the handler */
                       NULL );        /* module name, NULL=executable */

   /*
    * Finally inform the invoker that we have stopped gracefully.
    */
   ThreadHasStopped( ( unsigned ) data );
#ifdef REGINAVERSION
   ReginaCleanup();
#endif
#ifndef THREAD_RETURN_VOID
   return ( THREAD_RETURN ) 0;
#endif
}

/*
 * external is a separate thread and invokes a Rexx program from disk.
 * It runs a loop (inside Rexx) and checks for errors.
 * The return value is 0 if a return value is used at all.
 * The argument data is the index of the thread within threadx.
 * The Rexx program filename is in a global variable.
 */
THREAD_RETURN THREAD_CONVENTION external( void *data )
{
   int rc;

   /*
    * signal that we are alive.
    */
   threadx[(unsigned) data] = my_threadidx();

   rc = RexxStart( 0,            /* ArgCount                  */
                   NULL,         /* ArgList                   */
                   program_name, /* ProgramName               */
                   NULL,         /* Instore (source/compiled) */
                   "Foo",        /* EnvironmentName           */
                   RXCOMMAND,    /* CallType                  */
                   NULL,         /* ExitHandlerList           */
                   NULL,         /* ReturnCode (ignored)      */
                   NULL );       /* ReturnValue (ignored)     */

   /*
    * Finally inform the invoker that we have stopped gracefully.
    */
   ThreadHasStopped( ( unsigned ) data );
#ifdef REGINAVERSION
   ReginaCleanup();
#endif
#ifndef THREAD_RETURN_VOID
   return ( THREAD_RETURN ) 0;
#endif
}

/******************************************************************************
 ******************************************************************************
 * thread management **********************************************************
 ******************************************************************************
 *****************************************************************************/

/*
 * reap checks the thread's result buffer to see if it has seen all the lines
 * the interpreter has emitted.
 * The global error is set in case of an error.
 * The result buffer is reset on success.
 * Only called when using "instore" macro.
 */
void reap( unsigned position )
{
   if (found[position] != ~((unsigned) 0))
   {
      fprintf(stderr,"\n"
                     "Thread %lu has stopped without completing its loop.\n",
                     (unsigned long) threadx[position]);
      GlobalError = 1;
   }
   found[position] = 0;
}

#ifdef WIN32_THREADS
/*
 * init_threads initializes the usage of our thread management system.
 * Returns 1 on success, 0 otherwise.
 */
int init_threads( void )
{
   return 1;
}

/*
 * start_a_thread starts a thread and sets some state informations which are
 * set back in case of an error.
 * The return code is 1 on success, 0 otherwise.
 */
int start_a_thread( unsigned position )
{
   int rc;
   unsigned threadID;

   State[position] = Running;
   /* Avoid some race conditions. I don't know if this is a problem of the
    * runtime system or the kernel. If the systems runs into severe swapping
    * the threads seems to run before the thread id is known which is used
    * in instore_exit. We suspend the thread until all details of the new
    * thread are known before we continue. This gives a little bit worse
    * performance.
    */
   thread[position] = ( HANDLE ) _beginthreadex( NULL,
                                                 0,
                                                 (program_name) ? external : instore,
                                                 ( void * ) position,
                                                 CREATE_SUSPENDED,
                                                 &threadID );
   rc = ( long ) thread[position] != 0l;
   if ( !rc )
   {
      fprintf( stderr, "\n"
                       "Error starting thread, error code is %ld\n",
                       GetLastError() );
      GlobalError = 1;
      ThreadHasStopped( position );
   }
   ResumeThread( thread[position] );
   return rc;
}

/*
 * Thread has stopped sets the global state information of the thread with the
 * index "position" to "Stopped".
 */
static void ThreadHasStopped( unsigned position )
{
   State[position] = Stopped;
}

/*
 * wait_for_threads restarts new threads until the requested count of
 * TOTAL_THREADS has been reached. GlobalError is set if any error occurs.
 *
 * We expect to have MAX_THREADS already running.
 */
void wait_for_threads( void )
{
   unsigned i,j,done,running;
   DWORD rc;
   HANDLE compressed[MAX_THREADS];
   unsigned BackSort[MAX_THREADS];

   running = done = MAX_THREADS;

   if ( stdout_is_tty )
      printf( "%u\r", MAX_THREADS );
   if ( GlobalError )
      return;

   for ( ; ; )
   {
      /*
       * We have to pass an array of thread handles to the OS' waiter function.
       * But some threads may not be running at the last steps. Therefore we
       * have to resort the handles to be consecutive in a temporary array.
       */
      for ( i = 0, j = 0; i < MAX_THREADS; i++ )
      {
         if ( State[i] != Ready )
         {
            compressed[j] = thread[i];
            BackSort[j] = i;
            j++;
         }
      }
      rc = WaitForMultipleObjects( running, compressed, FALSE, timeout_seconds*1000 );
      if ( rc == 0xFFFFFFFF )
      {
         /*
          * Failure or dead thread, look for a stopped one
          */
         for ( i = 0; i < running; i++ )
         {
            if ( State[BackSort[i]] == Stopped )
               rc = WAIT_OBJECT_0 + i;
         }
      }
      if ( ( rc < WAIT_OBJECT_0 ) || ( rc >= running + WAIT_OBJECT_0 ) )
      {
         fprintf( stderr, "\n"
                          "At least one thread won't finish normally within 3 seconds (rc=%u, error=%lu).\n",
                          rc, GetLastError() );
         GlobalError = 1;
      }
      if ( GlobalError )
         break;

      /*
       * A thread has died. Find it and check the reason.
       */
      i = BackSort[rc - WAIT_OBJECT_0];
      if ( State[i] != Stopped )
      {
         fprintf( stderr, "\n"
                          "Thread %u hasn't finished normally.\n",i);
         GlobalError = 1;
         break;
      }

      /*
       * Destroy associated buffers, check values and
       * restart a new instance if we still have to do so.
       */
      CloseHandle( thread[i] );
      State[i] = Ready;
      running--;
      /*
       * Only reap our threads if running the instore test code
       */
      if ( program_name == NULL )
         reap( i );

      if ( done < total_threads )
      {
         if ( !start_a_thread( i ) )
            break;
         done++;
         running++;
      }
      if ( stdout_is_tty )
         printf( "%u(%u)\r", done, running );
      if ( GlobalError || !running )
         break;
   }
}
#endif

#ifdef OS2_THREADS
/*
 * init_threads initializes the usage of our thread management system.
 * Returns 1 on success, 0 otherwise.
 */
HMUX hmux;
SEMRECORD thread_sems[MAX_THREADS];

int init_threads( void )
{
   int i;
   LONG rc;

   for ( i = 0; i < MAX_THREADS; i++ )
   {
      thread_sems[i].ulUser = i;
      rc = DosCreateEventSem( NULL,
                              (PHEV) &thread_sems[i].hsemCur,
                              0,
                              0 );
      if ( rc != 0 )
      {
         fprintf( stderr, "\n"
                          "Error creating an EventSem, error code is %lu\n",
                          rc );
         return 0;
      }
   }

   rc = DosCreateMuxWaitSem( NULL,
                             &hmux,
                             MAX_THREADS,
                             thread_sems,
                             DCMW_WAIT_ANY);
   if ( rc != 0 )
   {
      fprintf( stderr, "\n"
                       "Error creating a MuxWaitSem, error code is %lu\n",
                       rc );
      return 0;
   }
   return 1;
}

/*
 * start_a_thread starts a thread and sets some state informations which are
 * set back in case of an error.
 * The return code is 1 on success, 0 otherwise.
 */
int start_a_thread( unsigned position )
{
   ULONG rc, post;

   rc = DosResetEventSem( (HEV) thread_sems[position].hsemCur, &post );
   if ( ( rc != 0 ) && ( rc != ERROR_ALREADY_RESET ) )
   {
      fprintf( stderr, "\n"
                       "Error resetting an EventSem, error code is %lu\n",
                       rc );
      GlobalError = 1;
      return 0;
   }
   State[position] = Running;
   thread[position] = _beginthread( (program_name) ? external : instore,
                                    NULL,
                                    0x8000,
                                    ( void * ) position );
   if ( thread[position] == -1 )
   {
      fprintf( stderr, "\n"
                       "Error starting thread, error code is %d\n",
                       errno );
      GlobalError = 1;
      State[position] = Stopped;
      DosPostEventSem( (HEV) thread_sems[position].hsemCur );
      return 0;
   }
   return 1;
}

/*
 * Thread has stopped sets the global state information of the thread with the
 * index "position" to "Stopped".
 */
static void ThreadHasStopped( unsigned position )
{
   ULONG rc;

   State[position] = Stopped;
   if ( ( rc = DosPostEventSem( (HEV) thread_sems[position].hsemCur ) ) != 0 )
   {
      fprintf( stderr, "\n"
                       "Error posting an EventSem, error code is %lu\n",
                       rc );
      GlobalError = 1;
   }
}

/*
 * wait_for_threads restarts new threads until the requested count of
 * TOTAL_THREADS has been reached. GlobalError is set if any error occurs.
 *
 * We expect to have MAX_THREADS already running.
 */
void wait_for_threads( void )
{
   unsigned done,running;
   ULONG rc, post, user;

   running = done = MAX_THREADS;

   if ( stdout_is_tty )
      printf( "%u\r", MAX_THREADS );
   if ( GlobalError )
      return;

   for ( ; ; )
   {
      rc = DosWaitMuxWaitSem( hmux, timeout_seconds*1000, &user );
      if ( rc != 0 )
      {
         fprintf( stderr, "\n"
                          "At least one thread won't finish normally within 3 seconds (error=%lu).\n",
                          rc );
         GlobalError = 1;
      }
      if ( user >= MAX_THREADS )
      {
         fprintf( stderr, "\n"
                          "Strange behaviour after wating for MuxWaitSem, released thread index is %lu.\n",
                          user );
         GlobalError = 1;
      }
      if ( GlobalError )
         break;

      /*
       * A thread has died. Check the reason.
       */
      if ( State[user] != Stopped )
      {
         fprintf( stderr, "\n"
                          "Thread %lu hasn't finished normally.\n", user );
         GlobalError = 1;
         break;
      }

      /*
       * Check values and restart a new instance if we still have to do so.
       */
      State[user] = Ready;
      running--;
      /*
       * Only reap our threads if running the instore test code
       */
      if ( program_name == NULL )
         reap( (int) user );

      if ( done < total_threads )
      {
         if ( !start_a_thread( (int) user ) )
            break;
         done++;
         running++;
      }
      else
      {
         rc = DosResetEventSem( (HEV) thread_sems[user].hsemCur, &post );
         if ( ( rc != 0 ) && ( rc != ERROR_ALREADY_RESET ) )
         {
            fprintf( stderr, "\n"
                             "Error resetting an EventSem, error code is %lu\n",
                             rc );
            GlobalError = 1;
            break;
         }
      }
      if ( stdout_is_tty )
         printf( "%u(%u)\r", done, running );
      if ( GlobalError || !running )
         break;
   }
}
#endif

#ifdef POSIX_THREADS
/*
 * The number of processed runs needs to be global for error analysis.
 */
static unsigned done = 0;

#if USE_SEMAPHORES
static sem_t something_stopped;

/*
 * timer_alarm will end the program if nothing happens within 3 seconds.
 */
void timer_alarm( int sig )
{
   static unsigned lastvalue = ( unsigned ) -1;
   static int call_count = 0;

   if ( ( lastvalue != done ) || ( (int) done == total_threads ) )
   {
      lastvalue = done;
      signal( SIGALRM, timer_alarm );
      return;
   }

   GlobalError = 1;
   signal( SIGALRM, timer_alarm );
   if ( call_count++ == 0 )
   {
      fprintf( stderr, "\n"
                       "At least one thread won't finish within 3 seconds.\n" );
      sem_post( &something_stopped );
   }

   if ( call_count > 2 )
      exit( 1 );
}
#else
static pthread_mutex_t thread_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t something_stopped = PTHREAD_COND_INITIALIZER;
#endif

/*
 * init_threads initializes the usage of our thread management system.
 * Returns 1 on success, 0 otherwise.
 */
int init_threads( void )
{
#if USE_SEMAPHORES
   if ( sem_init( &something_stopped, 0, 0 ) != 0 )
      return 0;
#endif
   return 1;
}

/*
 * start_a_thread starts a thread and sets some state informations which are
 * set back in case of an error.
 * The return code is 1 on success, 0 otherwise.
 */
int start_a_thread( unsigned position )
{
   int rc;

   State[position] = Running;
   rc = pthread_create( &thread[position], NULL, (program_name) ? external : instore, (void *) position );
   if ( rc )
   {
      fprintf( stderr, "\n"
                       "Error starting thread, error code is %d\n",
                       rc );
      GlobalError = 1;
      ThreadHasStopped( position );
   }
   return(!rc);
}


/*
 * Thread has stopped sets the global state information of the thread with the
 * index "position" to "Stopped".
 * The master semaphore is set to signal the main thread about the thread's
 * death.
 */
static void ThreadHasStopped( unsigned position )
{
#if USE_SEMAPHORES
   State[position] = Stopped;
   sem_post( &something_stopped );
#else
   /*
    * The use of the mutex lock semaphores is forced by Posix.
    */
   pthread_mutex_lock( &thread_lock );
   State[position] = Stopped;
   pthread_cond_signal( &something_stopped );
   pthread_mutex_unlock( &thread_lock );
#endif
}

/*
 * wait_for_threads restarts new threads until the requested count of
 * TOTAL_THREADS has been reached. GlobalError is set if any error occurs.
 *
 * We expect to have MAX_THREADS already running.
 *
 * Note: You will get a better performance if you set the schedule policy to
 *       RR or FIFO, but you have to be root to do this. If USE_SEMAPHORES is
 *       active, changing the policy won't increase the performance.
 */
void wait_for_threads( void )
{
   unsigned i,running;
   int rc;
#if !USE_SEMAPHORES
   struct timeval now;
   struct timespec timeout;
#else
   struct itimerval ival;
#endif

   running = done = MAX_THREADS;
   if ( stdout_is_tty )
      printf( "%u\r", MAX_THREADS );

   if ( GlobalError )
      return;

#if !USE_SEMAPHORES
   /*
    * The lock is needed by pthread_cond_timewait.
    * The multithreading paradigma here is very unefficient.
    */
   pthread_mutex_lock( &thread_lock );
#else
   signal( SIGALRM, timer_alarm );
   ival.it_value.tv_sec = timeout_seconds;
   ival.it_value.tv_usec = 0;
   ival.it_interval = ival.it_value;
   setitimer( ITIMER_REAL, &ival, NULL );
#endif

   for ( ; ; )
   {
#if !USE_SEMAPHORES
      /*
       * Sleep a maximum of 3 seconds.
       */
      gettimeofday(&now,NULL);
      timeout.tv_sec = now.tv_sec + timeout_seconds;
      timeout.tv_nsec = now.tv_usec * 1000;
      /*
       * The following call will wait up to timeout time for a signalled
       * something_stopped condition semaphore. thread_lock is atomically
       * unlocked on function entry and locked on function exit.
       */
      rc = pthread_cond_timedwait( &something_stopped, &thread_lock, &timeout );
      if ( rc == ETIMEDOUT )
      {
         fprintf( stderr, "\n"
                          "At least one thread won't finish within 3 seconds.\n" );
         GlobalError = 1;
      }
#else
      if ( sem_wait( &something_stopped ) != 0 )
      {
         fprintf( stderr, "\n"
                          "Interrupted wait.\n" );
         GlobalError = 1;
      }
#endif
      if ( GlobalError )
         break;

      /*
       * Restart the threads is appropriate.
       */
      for ( i = 0; i < MAX_THREADS; i++ )
      {
         if ( State[i] != Stopped )
            continue;
         State[i] = Ready;
         running--;
         rc = pthread_join( thread[i], NULL );
         if ( rc != 0 )
         {
            fprintf( stderr, "\n"
                            "A thread can't be found in the internal table.\n" );
            GlobalError = 1;
            break;
         }

         /*
          * Has the thread done its work completely?
          */
         /*
          * Only reap our threads if running the instore test code
          */
         if ( program_name == NULL )
            reap(i);

         /*
          * Restart a new thread if we need some more runs.
          */
         if ( (int) done < total_threads )
         {
            if ( !start_a_thread( i ) )
               break;
            done++;
            running++;
         }
      }
      if ( stdout_is_tty )
         printf( "%u(%u)\r", done, running );
      if ( GlobalError || !running )
         break;
   }
#if !USE_SEMAPHORES
   pthread_mutex_unlock( &thread_lock );
#endif
}
#endif

/*
 * Usage shows this program's usage and stops the program.
 */
static void usage( void )
{
   printf( "usage: threader [-p] [-q] [-t total_threads] [-s timeout_seconds] [filename]\n"
           "\n"
           "Options:\n"
           "-p\tLoad the macro only once and then use the generated instore\n"
           "\tmacro. Default: Always load the macro new.\n"
           "\n"
           "-q\tRun quietly. Don't display running progress information.\n"
           "\n"
           "-t\ttotal_threads\tTotal number of threads to execute.\n"
           "\n"
           "-s\ttimeoutseconds\tNumber of seconds to timeout a thread. Default 3.\n"
           "\n"
           "filename\tThe Rexx program to execute rather than the instore test\n"
           "\tprogram.\n"
           "\nThe default instore macro generates lines with numbers which can\n"
           "be parsed to detect problems in the multi-threading implementation.\n"
           "A loop counter runs by default until %u. The test should run from a few\n"
           "seconds up to a few minutes. You should hit ^C to abort the program\n"
           "if you think your harddisk is working heavily.\n"
           "\n"
           "This program is for testing Regina's multithreading capabilities.\n"
           ,total_threads );
   exit( 1 );
}

int main( int argc, char *argv[] )
{
   RXSTRING version;
   ULONG versioncode;
   int i;
   time_t start;

   /*
    * In case of a connected tty we let run a counter to show the user some
    * "success".
    */
   if ( isatty( fileno( stdout ) ) )
      stdout_is_tty = 1;

   for ( i = 1; i < argc; i++ )
   {
      if ( argv[i][0] != '-' )
         break;

      if ( strcmp( argv[i], "-p" ) == 0 )
      {
         UseInstore = FirstRun;
      }
      else if ( strcmp( argv[i], "-t" ) == 0 )
      {
         i++;
         total_threads = atoi( argv[i] );
      }
      else if ( strcmp( argv[i], "-s" ) == 0 )
      {
         i++;
         timeout_seconds = atoi( argv[i] );
      }
      else if ( strcmp( argv[i], "-q" ) == 0 )
      {
         stdout_is_tty = 0;
      }
      else if ( strcmp( argv[i], "--" ) == 0 )
      {
         i++;
         break;
      }
      else
      {
         usage();
      }
   }

   if ( argc > i )
   {
      program_name = argv[i];
      i++;
   }
   if ( argc > i )
      usage();

   /*
    * Initialize some tables and tune the IO system to show every output
    * at once.
    */
   memset( found, 0,sizeof( found ) );
   memset( State, 0,sizeof( State ) );
   setvbuf( stdout,NULL, _IONBF, 0 );
   setvbuf( stderr,NULL, _IONBF, 0 );
   printf( "Regina Rexx Thread Tester\n" );
   printf( "-------------------------\n" );

   version.strlength = 0;
   version.strptr = NULL;
   /*
    * This will not work if we check another Rexx. You can safely comment out
    * the following code up to the 'printf( "\n" );'
    */
#ifdef REGINAVERSION
   versioncode = ReginaVersion( &version );
   printf( "Regina's version is %lu.%lu",
           versioncode >> 8,
           versioncode & 0xFF );
   if ( version.strptr )
   {
      printf( " (in complete \"%s\")", version.strptr );
      RexxFreeMemory( version.strptr );
   }
#endif
   printf( "\n" );

   if ( UseInstore && program_name )
   {
      printf( "Ignoring the \"-p\" flag for external REXX scripts\n" );
      UseInstore = UnUsed;
   }
   /*
    * In case of a processing with compiled macros we need something
    * compiled ;-) We let run one instance and let instore() save the compiled
    * macros for later use.
    */
   if ( UseInstore )
   {
      State[0] = Running;
      thread[0] = my_threadid();
      threadx[0] = my_threadidx();
      if ( program_name )
         external( NULL );
      else
         instore( NULL );
      State[0] = Stopped;
      found[0] = 0;
      UseInstore = DoInstore;
   }

   if ( stdout_is_tty )
   {
      printf( "\n"
              "You should see a loop counter which stops at %u.\n\n",
              total_threads );
   }

   /*
    * Start some threads and check for errors. Then restart threads up to the
    * maximum count.
    */
   if ( !init_threads() )
   {
      fprintf( stderr, "\n"
                       "Failure initializing the thread management system.\n\n" );
      return 1;
   }

   start = time( NULL );
   for ( i = 0; i < MAX_THREADS; i++ )
   {
      if ( !start_a_thread( i ) || GlobalError )
         break;
   }
   if ( !GlobalError )
      wait_for_threads();

   if ( GlobalError )
   {
      fprintf( stderr, "\n"
                       "An error encountered. Do you use the right shared "
                                                           "libs or DLLs?\n" );
      return 1;
   }

   printf( "\n"
           "Thread tester passed without an error.\n"
           "About %u seconds used for %u cyles, each creating a thread.\n",
           (unsigned) ( time( NULL ) - start ), total_threads );

   if ( !stdout_is_tty || !isatty( fileno( stdout ) ) )
      return 0;

   printf( "Press ENTER to continue and end the program. You may have a look\n"
          " at your preferred memory analyser like ps, pstat or tasklist...\n" );
   {
      char buf[128];
      fgets( buf, sizeof( buf ), stdin );
   }

   return 0;
}
