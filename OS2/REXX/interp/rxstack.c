#ifndef lint
static char *RCSid = "$Id: rxstack.c,v 1.31 2004/04/18 02:54:34 florian Exp $";
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
/*
 * This process runs as a daemon (or NT service). It maintains multiple,
 * named Rexx queues.
 * All communication is done via TCP/IP sockets.
 * This process waits on a known port; 5656 by default for connections
 * from clients. A client is any process that respects the Interface
 * defined below. The "normal" clients are regina and rxqueue.
 * Details about each client is kept, like current queue name.
 *
 * Structure
 * ---------
 * startup
 * - set signal handler for SIGTERM
 * initialise socket interface
 * - socket()
 * - bind()
 * - listen()
 * loop until killed
 * - setup read FDs
 * - select()
 * - if listen socket, add new client
 * - otherwise read command
 * cleanup
 * - disconnect all clients
 * - free up resources
 *
 * Interface.
 * ---------
 * Once a client connects, it sends commands
 * Commands are single character, followed by optional 6 hex character
 * length and optional data.
 *   F - queue data onto client's current queue (FIFO)
 *       in -> FFFFFFFxxx--data--xxx
 *       out-> 0000000 (if successful)
 *       out-> 2xxxxxx (if error, eg queue deleted)
 *       out-> 3000000 (memory allocation error)
 *       regina QUEUE, rxqueue /fifo
 *   L - push data onto client's current queue (LIFO)
 *       in->  LFFFFFFxxx--data--xxx
 *       out-> 0000000 (if successful)
 *       out-> 2xxxxxx (if error, eg queue deleted)
 *       out-> 3000000 (memory allocation error)
 *       regina PUSH, rxqueue /lifo
 *   C - create queue
 *       in->  CFFFFFFxxx--queue name--xxx (if length 0, create name)
 *       out-> 0FFFFFFxxx--queue name--xxx (if queue name created)
 *       out-> 1FFFFFFxxx--queue name--xxx (if queue name existed)
 *       out-> 2xxxxxx (if error)
 *       regina RXQUEUE('C'), rxqueue N/A
 *   D - delete queue
 *       in->  DFFFFFFxxx--queue name--xxx
 *       out-> 0000000 (if queue name deleted)
 *       out-> 5xxxxxx (trying to delete 'SESSION' queue)
 *       out-> 6000000 (queue name not passed)
 *       out-> 9xxxxxx (if error, eg queue already deleted)
 *       regina RXQUEUE('D'), rxqueue N/A
 *   E - empty data from specified queue
 *       in->  KFFFFFFxxx--queue name--xxx
 *       out-> 0000000 (if queue emptied)
 *       out-> 2xxxxxx (if error, eg queue deleted)
 *       out-> 3000000 (memory allocation error)
 *       regina N/A, rxqueue /clear
 *   P - pop item off client's default queue
 *       in->  P000000
 *       out-> 0FFFFFFxxx--data--xxx (if queue name existed)
 *       out-> 1000000 (if queue empty)
 *       out-> 2xxxxxx (if queue name deleted - length ignored)
 *       out-> 4xxxxxx (if timeout on queue exceeded - length ignored)
 *       regina PULL, rxqueue N/A
 *   p - fetch item off client's default queue
 *       in->  p000000
 *       out-> 0FFFFFFxxx--data--xxx (if queue name existed)
 *       out-> 1000000 (if queue empty)
 *       out-> 2xxxxxx (if queue name deleted - length ignored)
 *       regina PULL without timeout, rxqueue N/A
 *x  S - set default queue name (allow false queues)
 *       in->  SFFFFFFxxx--queue name--xxx
 *       out-> 0000000 (if successful)
 *       out-> 3000000 (memory allocation error)
 *       out-> 6000000 (queue name not passed)
 *       regina RXQUEUE('S'), rxqueue N/A
 *   G - get default queue name
 *       in->  G000000
 *       out-> 0FFFFFFxxx--queue name--xxx
 *       regina RXQUEUE('G'), rxqueue N/A
 *   N - return number of lines on stack
 *       in->  N000000
 *       out-> 0FFFFFF (if queue exists)
 *       out-> 2xxxxxx (if error or queue doesn't exist - length ignored)
 *       regina QUEUED(), rxqueue N/A
 *   T - set timeout on queue pull
 *       in->  DFFFFFFTTTTTT
 *       out-> 0000000 (if queue timeout set)
 *       out-> 2xxxxxx (if error, eg invalid argument)
 *       out-> 6000000 (queue name not passed)
 *       regina RXQUEUE('T'), rxqueue N/A
 *   X - client disconnect
 *       in->  X000000
 *       out->
 *   Z - client requests shutdown - should only be called by ourselves!!
 *       in->  Z000000
 *       out->
 */

#define NO_CTYPE_REPLACEMENT
#include "rexx.h"

#if defined(WIN32) || defined(__LCC__)
# if defined(_MSC_VER)
#  if _MSC_VER >= 1100
/* Stupid MSC can't compile own headers without warning at least in VC 5.0 */
#   pragma warning(disable: 4115 4201 4214 4514)
#  endif
#  include <windows.h>
#  if _MSC_VER >= 1100
#   pragma warning(default: 4115 4201 4214)
#  endif
# elif defined(__LCC__)
#  include <windows.h>
#  include <winsvc.h>
#  include <winsock.h>
# else
#  include <windows.h>
# endif
# include <io.h>
#else
# ifdef HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
# endif
# ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
#  endif
# if defined(HAVE_POLL_H)
#  include <poll.h>
# elif defined(HAVE_SYS_SELECT_H)
#  include <sys/select.h>
# endif
# ifdef HAVE_NETDB_H
#  include <netdb.h>
# endif
# ifdef HAVE_ARPA_INET_H
#  include <arpa/inet.h>
# endif
#endif
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#ifdef HAVE_PROCESS_H
# include <process.h>
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

#include <assert.h>

#define HAVE_FORK
#if defined(__WATCOMC__) || defined(_MSC_VER) || (defined(__IBMC__) && defined(WIN32)) || defined(__SASC) || defined(__MINGW32__) || defined(__BORLANDC__) || defined(DOS) || defined(__LCC__)
# undef HAVE_FORK
#endif
#if defined(__WATCOMC__) && defined(__QNX__)
# define HAVE_FORK
#endif

#include "extstack.h"

#ifdef BUILD_NT_SERVICE
# include "service.h"
/*
 * this event is signalled when the
 * service should end
 */
HANDLE  hServerStopEvent = NULL;
#endif

#ifdef WIN32
# define os_errno ((int)WSAGetLastError())
# define errno_str(code) Win32ErrorString(code)
# undef EINTR
# define EINTR WSAEINTR
# undef ECONNRESET
# define ECONNRESET WSAECONNRESET
#else
# define os_errno errno
# define errno_str(code) strerror(code)
#endif


/*
 * debugging is turned off. You can turn it on by the command line option "-D".
 */
static int debug = 0 ;
#define DEBUGDUMP(x) { if ( debug ) \
                          {x;}      \
                     }

/*
 * DEFAULT_WAKEUP is the time in ms after which the process shall wakeup.
 * The time has a maximum of 49 days and shall be around one day. It may
 * be set much shorter for debugging purpose.
 */
#define DEFAULT_WAKEUP 86400000

/*
 * QUEUE_TIMEOUT is the time in ms after which an unused queue will be
 * removed. A queue is unused if no client is connected to it.
 * be set much shorter for debugging purpose.
 * The time has a maximum of 49 days and may be set to one week. It may
 * be set much shorter for debugging purpose.
 */
#define QUEUE_TIMEOUT (86400000*7)

/*
 * RxTime defines a structure holding the time in milliseconds resolution.
 * I know, most systems have at least one sort of high precision
 * time structure, but the ugly "#ifdef" are unreadable if we use
 * them all over here.
 * Defining a structure on our own is much more helpful.
 * Of course, we can't use a single 32 bit value for milliseconds. This
 * will break the server after 49 days. unix will live a little bit longer
 * and there shall Windows machines exist which doesn't have reboot since
 * longer periods ;-)
 */
typedef struct {
   /* seconds are typical time_t values. */
   time_t seconds ;

   /*
    * milli's values are between 0 and 999. The special value -1 indicates
    * a not-used condition.
    */
   int milli ;
} RxTime ;

/*
 * now holds the current time. It isn't updated after every operation and
 * may be out of date by some milliseconds some times.
 */
RxTime now ;

/*
 * This value is now plus 7 days.
 */
RxTime queue_deadline ;

struct _Client ;
typedef struct _RxQueue {
   /*
    * linked list maintainance elements
    */
   struct _RxQueue *prev, *next ;
   /* name is the uppercased name of the queue.
    */
   streng *name ;
   /*
    * Indicates if the queue is a "real" queue
    * or a false queue as a result of a rxqueue('set', 'qname')
    * on a queue that doesn't exist. This is crap behaviour!
    * but that's how Object Rexx works :-(
    */
   int isReal ;
   /*
    * Content: single buffered stack in opposite to the multi buffered
    * internal stacks of Regina.
    */
   Buffer buf ;
   /*
    * deadline is the time the queue was last used plus a timeout.
    * The queue is removed if the queue isn't used for one week.
    * Thus, the value is the time the queue was used last plus one week.
    */
   RxTime deadline ;
   /*
    * Several clients may want to wait for incoming data. They are queued
    * in the following structure and automatically reponsed by the
    * data acceptor of the queue in a FIFO manner, which is a fair-queue
    * algorithm.
    * The clients will get a notice of am error if the queue is destroyed
    * or emptied by an explicite call.
    * structure (n = newer, o = older)
    *         oldest                 newest
    *           ||                     ||
    * NULL<-o--client<-o--client<-o--client--n->NULL
    *               |     ^    |     ^
    *               |     |    |     |
    *               +--n--+    +--n--+
    */
   struct _Client *oldest, *newest;
} RxQueue;

/*
 * queues we work on.
 * Format, p=prev, n=next:
 *           queues
 *             ||
 *  NULL<--p-queue-n---->queue-n---->queue-n-->NULL
 *               ^       v   ^       v
 *               |       |   |       |
 *               +---p---+   +---p---+
 */
RxQueue *queues ;

/* SESSION is the special queue which can't be deleted and to which clients
 * drop when their current queue is deleted.
 */
RxQueue *SESSION ;

/*
 * Structure for multiple clients
 */
typedef struct _Client
{
   /*
    * linked list maintainance elements
    */
   struct _Client *prev, *next ;

   /*
    * socket contains the socket's handle
    */
   int socket;

   /*
    * each client has a default queue associated. It must be valid all
    * the times after initialization.
    */
   RxQueue *default_queue;

   /*
    * if queue_timeout is set, the client expects an error code after
    * this time instead of waiting until world's end.
    * The value is in milliseconds.
    */
   long queue_timeout;

   /*
    * We manage a deadline. A PULL operation is in an error state after
    * this timestamp. The value is set at a PULL operation to
    * now+queue_timeout.
    */
   RxTime deadline ;

   /*
    * linked list maintainance elements for waiters.
    */
   struct _Client *older, *newer ;
} Client;

/*
 * clients we work on.
 * Format, p=prev, n=next:
 *           clients
 *             ||
 *  NULL<--p-client-n--->client-n--->client-n-->NULL
 *               ^       v   ^       v
 *               |       |   |       |
 *               +---p---+   +---p---+
 */
Client *clients;

int running = 1;
int allclean = 0;
time_t base_secs; /* the time the process started */

void empty_queue( RxQueue *q ) ;

#if !defined(HAVE_STRERROR)
/*
 * Sigh! This must probably be done this way, although it's incredibly
 * backwards. Some versions of gcc comes with a complete set of ANSI C
 * include files, which contains the definition of strerror(). However,
 * that function does not exist in the default libraries of SunOS.
 * To circumvent that problem, strerror() is #define'd to get_sys_errlist()
 * in config.h, and here follows the definition of that function.
 * Originally, strerror() was #defined to sys_errlist[x], but that does
 * not work if string.h contains a declaration of the (non-existing)
 * function strerror().
 *
 * So, this is a mismatch between the include files and the library, and
 * it should not create problems for Regina. However, the _user_ will not
 * encounter any problems until he compiles Regina, so we'll have to
 * clean up after a buggy installation of the C compiler!
 */
const char *get_sys_errlist( int num )
{
   extern char *sys_errlist[] ;
   return sys_errlist[num] ;
}
#endif

#ifdef WIN32
const volatile char *Win32ErrorString(int code)
{
   static char buffer[512];
   size_t len;
   const CHAR *array[10];
   static HINSTANCE tcpip = NULL;
   DWORD rc;

   for (rc = 0;rc < sizeof(array) / sizeof(array[0]);rc++)
      array[rc] = "?";

   sprintf(buffer,"code %d: ",code);
   len = strlen(buffer);

   if (tcpip == NULL)
      tcpip = GetModuleHandle("wsock32");
   rc = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_FROM_HMODULE |
                      FORMAT_MESSAGE_ARGUMENT_ARRAY |
                      FORMAT_MESSAGE_MAX_WIDTH_MASK,
                      tcpip,                       /* lpSource               */
                      code,
                      GetUserDefaultLangID(),
                      buffer + len,
                      sizeof(buffer) - len - 1,    /* w/o term. 0            */
                      (va_list *) &array);
   if ((rc == 0) && (len >= 2))
      buffer[len - 2] = '\0';                      /* cut off ": " at the end*/
   return(buffer);
}
#endif

/*
 * get_now returns the current time.
 */
RxTime get_now( void )
{
   RxTime retval ;
#if defined(HAVE_GETTIMEOFDAY)
   struct timeval times ;

   gettimeofday(&times, NULL) ;
   retval.seconds = times.tv_sec ;
   retval.milli = times.tv_usec / 1000 ;

#elif defined(MAXLONGLONG)
   static enum { T_first, T_OK, T_illegal } state = T_first ;
   static LARGE_INTEGER freq;
   LARGE_INTEGER curr;

   retval.seconds = 0; /* Keep compiler happy */
   if ( state == T_first )
   {
      if ( !QueryPerformanceFrequency( &freq ) )
         state = T_illegal ;
      else
         state = T_OK ;
   }
   if ( state == T_OK )
   {
      if ( !QueryPerformanceCounter( &curr ) )
         state = T_illegal ;
      else
      {
         ULONGLONG h ;
         /*
          * if we don't native support for the 64 bit arithmetic, use
          * doubles. Everything else is a pain.
          * I hope we never get a compiler for Windows which can't compile
          * this directly.
          */
         retval.seconds = (time_t) ( curr.QuadPart / freq.QuadPart ) ;
         h = curr.QuadPart % freq.QuadPart ;
         h *= 1000 ;
         retval.milli = (int) ( h / freq.QuadPart ) ;
      }
   }
   if ( state == T_illegal )
   {
      /* Windows systems have ftime in the C library */
      struct timeb timebuffer;

      ftime(&timebuffer);
      retval.seconds = timebuffer.time ;
      retval.milli = timebuffer.millitm ;
   }

#elif defined(HAVE_FTIME)
   struct timeb timebuffer;

   ftime(&timebuffer);
   retval.seconds = timebuffer.time ;
   retval.milli = timebuffer.millitm ;

#else
   clock_t c ;

   if ( ( c = clock() ) == (clock_t) -1 )
   {
      /*
       * clock() values are not adjusted to 1.1.1970 and CLOCKS_PER_SEC
       * may be a float or double
       */
      retval.seconds = (time_t) (c / CLOCKS_PER_SEC) ;
      retval.milli = (int) ( ( c * 1000.0 ) / CLOCKS_PER_SEC) % 1000 ;
   }
   else
   {
      retval.seconds = time( NULL ) ;
      retval.milli = 0 ;
   }
#endif
   return retval ;
}

/*
 * time_add increments an amount of milliseconds to time. the amount is
 * usually a Client->queue_timeout.
 */
static void time_add( RxTime *t, long incr )
{
   time_t s = (time_t) ( incr / 1000 ) ;
   int m = ( incr % 1000 ) ;

   assert( t->milli != -1 ) ;
   /* wrapping may occur, be careful */
   s += t->seconds ;
   if ( ( t->milli += m ) >= 1000 )
   {
      s++ ;
      t->milli -= 1000 ;
   }
   if ( t->seconds > s )
   {
      /* wrapping! */
      s = (time_t) -1 ; /* maximum 1 */
      if ( t->seconds > s )
      {
         /* time_t is a signed value, most representations have
          * MinX = -MaxX - 1
          */
         s++;
         s = -s;
      }
      assert( t->seconds <= s ) ;
   }
   t->seconds = s ;
}

/*
 * time_diff returns t1 - t2 in milliseconds. -1 is returned on overflow;
 * -1 is never returned else. -2 is used is the returned time will be
 * negative.
 */
static int time_diff( RxTime t1, RxTime t2 )
{
   int retval, h ;

   assert( t1.milli != -1 ) ;
   assert( t2.milli != -1 ) ;
   if ( t1.seconds < t2.seconds )
      retval = -2 ;
   else
   {
      retval = (int) ( ( t1.seconds - t2.seconds ) * 1000 ) ;

      if ( ( t1.milli < t2.milli ) && ( retval == 0 ) )
         retval = -2 ;
      else
      {
         retval += t1.milli - t2.milli ;

         /* final check for overflow */
         h = (int) ( t1.seconds - t2.seconds ) ;
         if ( ( ( retval / 1000 ) < h - 1 )
           || ( ( retval / 1000 ) > h + 1 )
           || ( retval < 0 ) )
            return -1 ;
      }
   }
   return retval ;
}

streng *Str_upper( streng *str )
{
   int i;

   for ( i = 0; i < PSTRENGLEN( str ); i++ )
   {
      if ( islower( str->value[i] ) )
         str->value[i] = (char)toupper( str->value[i] );
   }
   return str;
}

/* compares 2 strengs and returns 0 if they are equal, 1 if not.
 * The second one is converted to uppercase while comparing, the first
 * one must be uppercase.
 */
int Str_ccmp( const streng *first, const streng *second )
{
   int tmp ;

   if ( PSTRENGLEN( first ) != PSTRENGLEN( second ) )
      return 1 ;

   for (tmp=0; tmp < PSTRENGLEN( first ); tmp++ )
      if ( first->value[tmp] != toupper( second->value[tmp] ) )
         return 1 ;

   return 0 ;
}

/* Str_cre_or_exit create a streng or exits after a message about
 * missing memory.
 */
streng *Str_cre_or_exit( const char *str, unsigned length )
{
   streng *retval ;

   if ( ( retval = MAKESTRENG( length ) ) == NULL )
   {
      showerror( ERR_STORAGE_EXHAUSTED, 0, ERR_STORAGE_EXHAUSTED_TMPL );
      exit( ERR_STORAGE_EXHAUSTED );
   }

   memcpy( PSTRENGVAL( retval ), str, length ) ;
   retval->len = length ;
   return retval ;
}

/*
 * Str_buf create a streng from a buffer. It may return NULL on error.
 */
streng *Str_buf( const char *str, unsigned length )
{
   streng *retval ;

   if ( ( retval = MAKESTRENG( length ) ) == NULL )
   {
      showerror( ERR_STORAGE_EXHAUSTED, 0, ERR_STORAGE_EXHAUSTED_TMPL );
      return NULL;
   }

   memcpy( PSTRENGVAL( retval ), str, length );
   retval->len = length;
   return retval;
}

/*
 * Str_dup duplicates a streng. It may return NULL on error.
 */
streng *Str_dup( const streng *str )
{
   return Str_buf( PSTRENGVAL( str ), PSTRENGLEN( str ) ) ;
}

/*
 * delete_a_queue deletes the queue's content and unlinks it from the list
 * of existing queues if q isn't SESSION.
 */
void delete_a_queue( RxQueue *q )
{
   Client *c ;

   if ( q->name )
   {
      DEBUGDUMP(printf("Deleting queue <%.*s>\n", PSTRENGLEN( q->name), PSTRENGVAL( q->name)););
   }
   else
   {
      DEBUGDUMP(printf("Deleting natal queue\n"););
   }
   empty_queue( q ) ;

   if ( q == SESSION )
      return ;

   if ( q->name != NULL )
      DROPSTRENG( q->name );

   /*
    * dequeue from the linked list and free space
    */
   if (q->prev != NULL)
      q->prev->next = q->next ;
   else
      queues = q->next ;
   if (q->next != NULL)
      q->next->prev = q->prev ;
   free( q ) ;

   /*
    * Let all clients connected to this queue fall back to "SESSION" as
    * the default queue.
    * FIXME: It may be better to leave a queue to a non-isReal state.
    *        This kind of work destroys the data integrity on SESSION.
    *        We get many more connections working on SESSION than expected.
    */
   for( c = clients; c != NULL; c = c->next )
   {
      if ( c->default_queue == q )
         c->default_queue = SESSION ;
   }
}

/*
 * delete_a_client deletes the clients' content and unlinks it from the list
 * of existing clients.
 */
void delete_a_client( Client *c )
{
   close( c->socket ) ;

   /*
    * dequeue from the linked list and free space
    */
   if (c->prev != NULL)
      c->prev->next = c->next ;
   else
      clients = c->next ;
   if (c->next != NULL)
      c->next->prev = c->prev ;
   free( c ) ;
}

void delete_all_queues( void )
{
   RxQueue *q, *h ;

   /*
    * SESSION won't be deleted. Be careful to initiate one delete per
    * queue.
    */
   for ( q = queues; q != NULL; )
   {
      h = q ;
      q = q->next ;
      delete_a_queue( h );
   }
}

char *get_unspecified_queue( void )
{
   char *rxq = getenv( "RXQUEUE" );

   if ( rxq == NULL )
      rxq = "SESSION";

   if ( strchr(rxq, '@' ) == NULL )
   {
      char *h ;

      if ( ( h = malloc( strlen( rxq ) + 2 ) ) != NULL )
      {
         strcpy( h, rxq ) ;
         strcat( h, "@" ) ;
         rxq = h ;
      }
   }
   return rxq;
}

int suicide( void )
{
   int sock;
   streng *queue;
   char *in_queue=get_unspecified_queue();
   Queue q;

   if ( init_external_queue( NULL ) )
      return 1;

   queue = Str_cre_or_exit( in_queue, strlen( in_queue ) ) ;

   if ( parse_queue( NULL, queue, &q ) == 1 )
   {
      sock = connect_to_rxstack( NULL, &q );
      if ( sock < 0 )
      {
         /* error already shown by the function */
         return(ERR_RXSTACK_CANT_CONNECT);
      }
      send_command_to_rxstack( NULL, sock, RXSTACK_KILL_STR, NULL, 0 );
      read_result_from_rxstack( NULL, sock, RXSTACK_HEADER_SIZE );
      close(sock);
   }
   term_external_queue( ) ;
   return 0;
}

int rxstack_cleanup( void )
{
   if ( !allclean )
   {
      DEBUGDUMP(printf("Cleaning up\n"););
      /*
       * Disconnect all clients
       * Delete all clients
       */
      delete_all_queues();
      DEBUGDUMP(printf("Finished Cleaning up\n"););
      term_external_queue( ) ;
      allclean = 1;
   }
   return 0;
}

#ifdef BUILD_NT_SERVICE
BOOL report_service_start( void )
{
   /*
    * report the status to the service control manager.
    */
   return (ReportStatusToSCMgr(
           SERVICE_RUNNING,       /* service state */
           NO_ERROR,              /* exit code */
           0));                   /* wait hint */
}

BOOL report_service_pending_start( void )
{
   /*
    * report the status to the service control manager.
    */
   return (ReportStatusToSCMgr(
           SERVICE_START_PENDING, /* service state */
           NO_ERROR,              /* exit code */
           3000));                /* wait hint */
}

int nt_service_start( void )
{
   /*
    * code copied from sample NT Service code. The goto's are
    * not mine!!
    * report the status to the service control manager.
    */
   if ( !report_service_pending_start() )
      goto cleanupper;

   /*
    * create the event object. The control handler function signals
    * this event when it receives the "stop" control code.
    */
   hServerStopEvent = CreateEvent(
        NULL,    /* no security attributes */
        TRUE,    /* manual reset event */
        FALSE,   /* not-signalled */
        NULL);   /* no name */

   if ( hServerStopEvent == NULL)
      goto cleanupper;

   /*
    * report the status to the service control manager.
    */
   if ( !report_service_pending_start() )
      goto cleanupper;

    return 0;
cleanupper:
    return 1;
}

VOID ServiceStop()
{
   DEBUGDUMP(printf("In ServiceStop()\n"););
   suicide();
/*
   running = 0;
*/
}
#endif

void rxstack_signal_handler( int sig )
{
   running = 0;
}

/* Creates a new client and appends it in front of the current clients.
 * Don't forget to set a default_queue and the socket at once.
 */
Client *get_new_client( )
{
   Client *retval = malloc( sizeof( Client ) ) ;

   if ( retval == NULL )
      return NULL ;
   memset( retval, 0, sizeof( Client ) ) ;
   retval->socket = -1 ;
   retval->deadline.milli = -1 ; /* deadline not used --> infinite timeout */

   retval->next = clients ;
   if ( clients != NULL )
      clients->prev = retval ;
   clients = retval ;
   return retval ;
}

/*
 * Find the named queue - case insensitive
 * returns the queue or NULL if no queue with this name exists.
 */
RxQueue *find_queue( const streng *queue_name )
{
   RxQueue *q ;

   for ( q = queues; q != NULL; q = q->next )
   {
      /* This is inefficient, FIXME: Introduce a hash value */
      if ( Str_ccmp( q->name, queue_name ) == 0 )
         return q;
   }
   return NULL ;
}

/* Creates a new queue and appends it in front of the current queues.
 * Don't forget to set a name at once.
 */
RxQueue *get_new_queue( void )
{
   RxQueue *retval = malloc( sizeof( RxQueue ) ) ;

   if ( retval == NULL )
      return NULL ;
   memset( retval, 0, sizeof( RxQueue ) ) ;
   retval->deadline = queue_deadline ;

   retval->next = queues ;
   if ( queues != NULL )
      queues->prev = retval ;
   queues = retval ;
   return retval ;
}

int rxstack_delete_queue( Client *client, streng *queue_name )
{
   RxQueue *q ;
   int rc ;

   if ( ( q = find_queue( queue_name ) ) == NULL )
   {
      rc = 9;
   }
   else
   {
      if ( q == SESSION )
         rc = 5;
      else
      {
         if ( !q->isReal )
         {
            /*
             * If we found a false queue, return 9
             * but delete it.
             */
            delete_a_queue( q );
            rc = 9;
         }
         else
         {
            /*
             * Delete the contents of the queue
             * and mark it as gone.
             */
            delete_a_queue( q );
            rc = 0;
         }
      }
   }
   return rc ;
}

int rxstack_create_client( int socket )
{
   Client *c ;

   if ( ( c = get_new_client( ) ) == NULL )
   {
      close( socket ) ;
      /* This may have been the connectioon telling us to go down ;-) */
      showerror( ERR_STORAGE_EXHAUSTED, 0, ERR_STORAGE_EXHAUSTED_TMPL );
      exit( ERR_STORAGE_EXHAUSTED );
   }

   c->socket = socket;
   c->default_queue = SESSION ;
   return 0;
}

/* rxstack_send_return writes back to the client the action return code
 * and optionally a string of length len.
 * The functions returns 0 on success, -1 on error
 */
int rxstack_send_return( int sock, char *action, char *str, int len )
{
   streng *qlen, *header;
   int rc, retval = 0 ;

   DEBUGDUMP(printf("Sending to %d Result: %c <%.*s>\n", sock, *action, len, (str) ? str : ""););
   qlen = REXX_D2X( len );
   if ( qlen )
   {
      header = REXX_RIGHT( qlen, RXSTACK_HEADER_SIZE, '0');
      DROPSTRENG( qlen );
      if ( header )
      {
         header->value[0] = action[0];
         rc = send( sock, PSTRENGVAL(header), PSTRENGLEN(header), 0 );
         if ( rc != PSTRENGLEN(header) )
         {
            DEBUGDUMP(printf("Send failed: errno = %d\n", os_errno ););
            retval = -1 ;
         }
         else if ( str )
         {
            rc = send( sock, str, len, 0 );
            if ( rc != PSTRENGLEN(header) )
            {
               DEBUGDUMP(printf("Send failed: errno = %d\n", os_errno ););
               retval = -1 ;
            }
         }
         DROPSTRENG( header );
      }
   }
   return retval ;
}

int rxstack_delete_client( Client *client )
{
   delete_a_client( client ) ;
   return 0;
}

/* rxstack_set_default_queue sets the client's (new) queue name.
 * A false queue is created if the queue isn't found.
 * The new queue is returned or NULL if we are out of memory.
 */
RxQueue *rxstack_set_default_queue( Client *client, streng *data )
{
   RxQueue *q, *prev;
   streng *newq;

   prev = client->default_queue;
   if ( ( q = find_queue( data ) ) == NULL )
   {
      /*
       * We didn't find a real or a false queue, so create
       * a false queue
       */
      q = get_new_queue( );
      if ( q != NULL )
      {
         newq = Str_dup( data );
         if ( newq == NULL )
         {
            delete_a_queue( q );
            return NULL;
         }
         q->name = Str_upper( newq ) ;
         DEBUGDUMP(printf("Creating the false queue <%.*s>", PSTRENGLEN( q->name ), PSTRENGVAL( q->name ) ););
         /* q->isReal set to 0 by get_new_queue --> false queue */
         client->default_queue = q;
      }
   }
   else
   {
      client->default_queue = q;
   }

   if ( q == NULL )
   {
      DEBUGDUMP(printf("No FREE MEMORY when setting default queue for client: <%.*s>\n", PSTRENGLEN(data), PSTRENGVAL(data) ););
   }
   else
   {
      DEBUGDUMP(printf("Setting default queue for client: <%.*s> Prev: %p <%.*s>\n", PSTRENGLEN(q->name), PSTRENGVAL(q->name), prev, PSTRENGLEN(prev->name), PSTRENGVAL(prev->name) ););
      /* SET or CREATE resets a timeout to 0 */
      client->queue_timeout = 0 ;
   }
   return q;
}

int rxstack_timeout_queue( Client *client, const streng *data )
{
   int val,error;

   /*
    * Convert the timeout
    * If the supplied timeout is 0 (infinite wait), set the client->queue_timeout
    * to -1.
    */
   val = REXX_X2D( data, &error );
   if ( error )
      return 2;

   client->queue_timeout = val;
   DEBUGDUMP(printf("Timeout on queue: %ld\n", client->queue_timeout ););

   return 0;
}

/* unique_name creates a unique name for a queue.
 * The function may exit after a message about missing memory.
 */
static streng *unique_name( void )
{
   static int first = 1 ;
   static char buf[ 80 ] ;
   static char *ptr ;
   static unsigned runner = 0;

   if ( first )
   {
      first = 0 ;
      sprintf( buf, "S%d%ld", (int) getpid(), (long) time( NULL ) ) ;
      ptr = buf + strlen( buf ) ;
   }
   sprintf( ptr, "%u", runner++ ) ;
   return Str_buf( buf, strlen( buf ) ) ;
}

int rxstack_create_queue( Client *client, streng *data, streng **result )
{
   RxQueue *q ;
   streng *new_queue = NULL;
   int rc = 0;

   if ( data )
   {
      DEBUGDUMP(printf("Creating new user-specified queue: <%.*s>\n", PSTRENGLEN(data), PSTRENGVAL(data) ););
      if ( ( q = find_queue( data ) ) == NULL )
      {
         /*
          * No queue of that name, so use a duplicate of it.
          */
         DEBUGDUMP(printf("Couldn't find <%.*s>; so creating it\n", PSTRENGLEN(data), PSTRENGVAL(data) ););
         new_queue = data;
      }
      else
      {
         /*
          * If the queue we found is a false queue, we can still
          * use the supplied name and the slot
          */
         DROPSTRENG( data );
         if ( !q->isReal )
         {
            DEBUGDUMP(printf("Found false queue\n" ););
            q->isReal = 1;
            /* SET or CREATE resets a timeout to 0 */
            client->queue_timeout = 0;
            *result = q->name;
            return 0; /* Pass back the name. May be different due to
                       * different locales or codepages, but it IS the selected
                       * name.
                       */
         }
         new_queue = unique_name( );
         if ( new_queue == NULL )
            return 3;
         DEBUGDUMP(printf("Having to create unique queue <%.*s>\n", PSTRENGLEN( new_queue ), PSTRENGVAL( new_queue ) ););
         rc = 1;
      }
   }
   else
   {
      DEBUGDUMP(printf("Creating system generated queue.\n"););
      new_queue = unique_name( );
      if ( new_queue == NULL )
         return 3;
   }

   if ( ( q = get_new_queue( ) ) == NULL )
   {
      DROPSTRENG( new_queue );
      showerror( ERR_STORAGE_EXHAUSTED, 0, ERR_STORAGE_EXHAUSTED_TMPL );
      return 3;
   }

   /*
    * Uppercase the queue name
    */
   q->name = Str_upper( new_queue );
   q->isReal = 1;
   /* SET or CREATE resets a timeout to 0 */
   client->queue_timeout = 0 ;
   *result = q->name;
   return rc; /* Both code 0 and code 1 return the name to the caller.
               * May be different due to codepages, etc.
               */
}

/*
 * Pushes 'line' onto the REXX stack in LIFO manner.
 *    point to the new line. The line is put on top of the current
 *    buffer.
 */
StackLine *rxstack_stack_lifo( RxQueue *current_queue, streng *line )
{
   StackLine *newbox ;

   if ( ( newbox = (StackLine *) malloc( sizeof(StackLine) ) ) != NULL )
   {
      newbox->contents = line ;
      LIFO_LINE( &current_queue->buf, newbox ) ;
   }

   return newbox;
}


/*
 * Pushes 'line' onto the REXX stack in FIFO manner.
 *    point to the new line. The line is put on top of the current
 *    buffer.
 */
StackLine *rxstack_stack_fifo( RxQueue *current_queue, streng *line )
{
   StackLine *newbox ;

   if ( ( newbox = (StackLine *) malloc( sizeof(StackLine) ) ) != NULL )
   {
      newbox->contents = line ;
      FIFO_LINE( &current_queue->buf, newbox ) ;
   }

   return newbox;
}

/*
 * dequeue_waiter dequeues the Client c from the waiter's list of the queue q.
 * The client must be linked in the waiter's list of the queue.
 * The client's variables for this purpose are cleaned, too.
 */
static void dequeue_waiter( RxQueue *q, Client *c )
{
#if defined(DEBUG)
   Client *run ;
   for ( run = q->oldest; run != NULL; run = run->newer )
      if ( run == c )
         break ;
   assert( run != NULL ) ; /* This is the test if c is a waiter of q */
#endif

   if ( c->older != NULL )
   {
      c->older->newer = c->newer ;
   }
   else
   {
      q->oldest = c->newer ;
      if ( q->oldest != NULL )
         q->oldest->older = NULL ;
   }

   if ( c->newer != NULL )
   {
      c->newer->older = c->older ;
   }
   else
   {
      q->newest = c->older ;
      if ( q->newest != NULL )
         q->newest->newer = NULL ;
   }
   assert( ( ( q->newest != NULL ) && ( q->oldest != NULL ) ) ||
           ( ( q->newest == NULL ) && ( q->oldest == NULL ) ) ) ;
   c->older = c->newer = NULL ;
   c->deadline.milli = -1 ;
}

/* redir sends data as the answer of a pull operation back to
 * the oldest waiting client and dequeues this client.
 * data is dropped after the operation.
 */
void redir( RxQueue *q, streng *data )
{
   Client *c ;

   c = q->oldest ;
   DEBUGDUMP(printf("Redirecting <%.*s> to waiting client %d\n", PSTRENGLEN(data), (PSTRENGVAL(data)) ? PSTRENGVAL(data) : "", c->socket ););

   dequeue_waiter( q, c ) ;

   rxstack_send_return( c->socket, "0", PSTRENGVAL( data ), PSTRENGLEN( data ) ) ;
   DROPSTRENG( data ) ;
}

/* bad_news_for_waiter informs a waiter about an error while waiting for
 * data for a pull request. The client is dequeued from its queue.
 */
void bad_news_for_waiter( RxQueue *q, Client *c )
{
   dequeue_waiter( q, c ) ;
   DEBUGDUMP(printf("Sending negative response to waiting client %d\n", c->socket ););

   rxstack_send_return( c->socket, "0", NULL, 0 ) ;
}

int rxstack_queue_data( Client *client, streng *data, char order )
{
   int rc = 0;

   if ( client->default_queue->oldest != NULL )
   {
      redir( client->default_queue, data ) ;
      return 0 ;
   }
   DEBUGDUMP(printf("Queueing: <%.*s> Order: %c\n", PSTRENGLEN(data), (PSTRENGVAL(data)) ? PSTRENGVAL(data) : "", order ););
   if ( order == RXSTACK_QUEUE_FIFO )
   {
      if ( rxstack_stack_fifo( client->default_queue, data ) == NULL )
      {
         DROPSTRENG( data );
         rc = 3;
      }
   }
   else
   {
      if ( rxstack_stack_lifo( client->default_queue, data ) == NULL )
      {
         DROPSTRENG( data );
         rc = 3;
      }
   }
   return rc;
}

/* Clears the content of the queue. All waiters are informed by code
 * 2 of a cleaned queue and removed the the waiter's list.
 */
void empty_queue( RxQueue *q )
{
   StackLine *tmp, *line;
   streng *contents;
   Buffer *b ;

   b = &q->buf ;
   for ( line = b->top; line != NULL; )
   {
      contents = line->contents;
      DROPSTRENG( contents );
      tmp = line;
      line = line->lower;
      free( tmp );
   }
   memset( &q->buf, 0, sizeof( Buffer ) ) ;

   /* acknowledge waiters for data not ready and dequeue them */

   while ( q->oldest != NULL ) {
      bad_news_for_waiter( q, q->oldest ) ;
   }
}

/* Clears the content of the queue named data. In opposite to the previous
 * version, the client's current queue isn't set to the named queue any
 * longer. The is the default behaviour in Regina.
 * returns 0 on success, 2 if the queue doesn't exist.
 */
int rxstack_empty_queue( Client *client, streng *data )
{
   RxQueue *q ;

   DEBUGDUMP(printf("Emptying queue: <%.*s>\n", PSTRENGLEN(data), (PSTRENGVAL(data)) ? PSTRENGVAL(data) : "" ););
   if ( ( q =  find_queue( data ) ) == NULL )
      return 2;

   empty_queue( q ) ;

   return 0;
}

int rxstack_number_in_queue( Client *client )
{
   int lines = (int) client->default_queue->buf.elements;

   DEBUGDUMP(printf("Querying number in queue: %d\n", lines ););
   return lines ;
}

/*
 * Pulls a line off the queue and dequeues it.
 *
 * If nowait isn't set and no data is available and the client's queue_timeout
 * is set, the client is set to the newest end of the client's default_queue.
 *
 * It will be awaken by either ariving data on this pipe, or deleting/emptying
 * the pipe, or by a timeout.
 * Returns:
 * 0 if line OK
 * 1 if queue empty
 * 3 if waiting
 */
int rxstack_pull_line_off_queue( Client *client, streng **result, int nowait )
{
   int rc;
   Buffer *b;
   StackLine *line;
   RxQueue *q;

   b = &client->default_queue->buf;
   POP_LINE( b, line );
   if ( line != NULL )
   {
      *result = line->contents;
      free( line );
      rc = 0;
   }
   else
   {
      *result = NULL;
      if ( nowait )
         rc = 1; /* queue empty */
      else
      {
         if ( client->queue_timeout == 0 )
            rc = 1 ; /* queue empty */
         else
         {
            assert( client->deadline.milli == -1 );
            assert ( client->newer == NULL );
            now = get_now( );
            client->deadline = now;
            time_add( &client->deadline, client->queue_timeout );

            q = client->default_queue;
            client->newer = NULL;
            client->older = q->newest;
            if ( client->older != NULL )
               client->older->newer = client;
            q->newest = client;
            if ( q->oldest == NULL )
               q->oldest = client;
            rc = 3; /* waiting */
         }
      }
   }
   DEBUGDUMP(printf("Pulling line off queue; rc %d\n", rc ););
   return rc;
}

/* rxstack_process_command reads a new command from the client and processes
 * it.
 * returns 0 if the client has been terminated, 1 if the client persists.
 */
int rxstack_process_command( Client *client )
{
   RxQueue *q ;
   char cheader[RXSTACK_HEADER_SIZE];
   streng *header;
   streng *buffer = NULL ;
   streng *result=NULL;
   int rc,length;
   char rcode[2];

   if ( client->deadline.milli != -1 )
   {
      /*
       * interrupted wait, assume the client don't want to wait for data any
       * longer
       */
      bad_news_for_waiter( client->default_queue, client ) ;
   }
   rcode[1] = '\0';
   memset( cheader, 0, sizeof(cheader) );
   DEBUGDUMP(printf("reading from socket %d\n", client->socket););
   rc = recv( client->socket, cheader, RXSTACK_HEADER_SIZE, 0 );
   if ( rc < 0 )
   {
      if ( os_errno != ECONNRESET )
      {
         showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_READING_SOCKET, ERR_RXSTACK_READING_SOCKET_TMPL, errno_str( os_errno ) );
      }
      /*
       * Assume client has been lost
       */
      rxstack_delete_client( client );
      return 0 ;
   }
   if ( rc == 0 )
   {
      DEBUGDUMP(printf("read empty header\n"););
      /*
       * Assume client has been lost
       */
      rxstack_delete_client( client );
      return 0 ;
   }
   else if ( rc != RXSTACK_HEADER_SIZE )
   {
      DEBUGDUMP(printf("read corrupted header\n"););
      /*
       * Assume client has been lost
       */
      rxstack_delete_client( client );
      return 0 ;
   }

   header = MakeStreng( RXSTACK_HEADER_SIZE - 1 );
   if ( header == NULL )
   {
      showerror( ERR_STORAGE_EXHAUSTED, 0, ERR_STORAGE_EXHAUSTED_TMPL );
      exit( ERR_STORAGE_EXHAUSTED );
   }
   memcpy( PSTRENGVAL(header), cheader+1, RXSTACK_HEADER_SIZE-1 );
   header->len = RXSTACK_HEADER_SIZE-1 ;
   buffer = NULL;
   /*
    * Convert the data length
    */
   length = REXX_X2D( header, &rc );
   if ( rc )
   {
      /*
       * Errorneous number. Kill the client.
       */
      DEBUGDUMP(printf("Invalid header: <%.*s>, client killed\n", header->len, header->value););
      rxstack_send_return( client->socket, "9", NULL, 0 );
      rxstack_delete_client( client );
      return 1;
   }

   DEBUGDUMP(printf("Header: <%.*s> length: %d\n", header->len, header->value, length););
   DROPSTRENG( header );
   if ( length > 0 )
   {
      /*
       * Allocate a streng big enough for the expected data
       * string, based on the length just read; even if the length
       * is zero
       */
      buffer = MAKESTRENG ( length );
      if ( buffer == NULL )
      {
         showerror( ERR_STORAGE_EXHAUSTED, 0, ERR_STORAGE_EXHAUSTED_TMPL );
         DEBUGDUMP(printf("can't buffer input of client\n"););
         rxstack_delete_client( client );
         return 0;
      }
      rc = recv( client->socket, PSTRENGVAL(buffer), length, 0 );
      if ( rc < 0 )
      {
         showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_READING_SOCKET, ERR_RXSTACK_READING_SOCKET_TMPL, errno_str( os_errno ) );
      }
      else if ( rc == 0 )
      {
         /*
          * All we can assume here is that the client has been lost
          */
         DEBUGDUMP(printf("read empty header\n"););
         rxstack_delete_client( client );
         DROPSTRENG( buffer ) ;
         return 0 ;
      }
      else
         buffer->len = length ;
   }

   switch( cheader[0] )
   {
      case RXSTACK_QUEUE_FIFO:
      case RXSTACK_QUEUE_LIFO:
         DEBUGDUMP(printf("--- Queue %s ---\n", cheader[0] == RXSTACK_QUEUE_FIFO ? "FIFO" : "LIFO"););
         /*
          * fixes bug 700539
          */
         if ( buffer == NULL )
            buffer = Str_buf( "", 0 );
         if ( buffer == NULL )
            rc = 3;
         else
            rc = rxstack_queue_data( client, buffer, cheader[0] );
         rcode[0] = (char)(rc+'0');
         rxstack_send_return( client->socket, rcode, NULL, 0 );
         buffer = NULL ; /* consumed by rxstack_queue_data */
         break;
      case RXSTACK_EXIT:
         DEBUGDUMP(printf("--- Exit ---\n"););
         /*
          * Client has requested disconnect, so remove all
          * references to the client
          */
         rxstack_send_return( client->socket, "0", NULL, 0 );
         rxstack_delete_client( client );
         if ( buffer != NULL )
         {
            DROPSTRENG( buffer );
            buffer = NULL;
         }
         break;
      case RXSTACK_KILL:
         DEBUGDUMP(printf("--- Kill ---\n"););
         /*
          * Client has requested server to stop
          */
         rxstack_send_return( client->socket, "0", NULL, 0 );
         rxstack_delete_client( client );
         running = 0;
         return 0;
      case RXSTACK_SET_QUEUE:
         DEBUGDUMP(printf("--- Set Queue ---\n"););
         /*
          * Set the default queue for the client
          */
         if ( buffer == NULL )
            rxstack_send_return( client->socket, "6", NULL, 0 );
         else
         {
            q = rxstack_set_default_queue( client, buffer );
            if ( q == NULL )
               rxstack_send_return( client->socket, "3", NULL, 0 );
            else
               rxstack_send_return( client->socket, "0", q->name->value, q->name->len );
            DROPSTRENG( buffer );
            buffer = NULL;
         }
         break;
      case RXSTACK_EMPTY_QUEUE:
         DEBUGDUMP(printf("--- Empty Queue ---\n"););
         /*
          * Use the current queue as the default queue.
          */
         if ( buffer == NULL )
            buffer = client->default_queue->name;
         rc = rxstack_empty_queue( client, buffer );
         rcode[0] = (char)(rc+'0');
         rxstack_send_return( client->socket, rcode, NULL, 0 );
         if ( buffer != client->default_queue->name )
            DROPSTRENG( buffer );
         buffer = NULL ;
         break;
      case RXSTACK_NUMBER_IN_QUEUE:
         DEBUGDUMP(printf("--- Number in Queue ---\n"););
         length = rxstack_number_in_queue( client );
         rxstack_send_return( client->socket, "0", NULL, length );
         if ( buffer != NULL )
         {
            DROPSTRENG( buffer );
            buffer = NULL;
         }
         break;
      case RXSTACK_PULL:
      case RXSTACK_FETCH:
         DEBUGDUMP(printf("--- Pull ---\n"););
         rc = rxstack_pull_line_off_queue( client, &result, cheader[0] == RXSTACK_FETCH );
         switch( rc )
         {
            case 0: /* all OK */
               rxstack_send_return( client->socket, "0", PSTRENGVAL( result ), PSTRENGLEN( result ) );
               DROPSTRENG( result );
               break;
            default: /* empty/error */
               rcode[0] = (char)(rc+'0');
               rxstack_send_return( client->socket, rcode, NULL, 0 );
               break;
            case 3: /* still waiting; don't return */
               break;
         }
         if ( buffer != NULL )
         {
            DROPSTRENG( buffer );
            buffer = NULL;
         }
         break;
      case RXSTACK_GET_QUEUE:
         DEBUGDUMP(printf("--- Get Queue ---\n"););
         rxstack_send_return( client->socket, "0", PSTRENGVAL(client->default_queue->name), PSTRENGLEN(client->default_queue->name) ) ;
         if ( buffer != NULL )
         {
            DROPSTRENG( buffer );
            buffer = NULL;
         }
         break;
      case RXSTACK_CREATE_QUEUE:
         DEBUGDUMP(printf("--- Create Queue ---\n"););
         /*
          * Create a new queue
          */
         rc = rxstack_create_queue( client, buffer, &result );
         rcode[0] = (char)(rc+'0');
         if ( ( rc != 1 ) && ( rc != 0 ) )
            rxstack_send_return( client->socket, rcode, NULL, 0 );
         else
            rxstack_send_return( client->socket, rcode, PSTRENGVAL(result), PSTRENGLEN(result) );
         buffer = NULL;  /* consumed by rxstack_create_queue */
         break;
      case RXSTACK_DELETE_QUEUE:
         DEBUGDUMP(printf("--- Delete Queue ---\n"););
         /*
          * Delete the queue
          */
         if ( buffer == NULL )
            rc = 6;
         else
            rc = rxstack_delete_queue( client, buffer );
         rcode[0] = (char)(rc+'0');
         rxstack_send_return( client->socket, rcode, NULL, 0 );
         if ( buffer != NULL )
         {
            DROPSTRENG( buffer );
            buffer = NULL;
         }
         break;
      case RXSTACK_TIMEOUT_QUEUE:
         DEBUGDUMP(printf("--- Timeout Queue ---\n"););
         /*
          * Set timeout for pull from queue
          */
         if ( buffer == NULL )
            rc = 6;
         else
            rc = rxstack_timeout_queue( client, buffer );
         rcode[0] = (char)(rc+'0');
         rxstack_send_return( client->socket, rcode, NULL, 0 );
         if ( buffer != NULL )
            DROPSTRENG( buffer );
         buffer = NULL;
         break;
      case RXSTACK_UNKNOWN:
         /* do nothing */
         break;
      default:
         rxstack_send_return( client->socket, "9", NULL, 0 );
         break;
   }
   assert( buffer == NULL ) ;
   if ( buffer != NULL )
      DROPSTRENG( buffer ) ;
   return 1;
}

/*
 * earlier returns the time stamp which is more early.
 */
static RxTime earlier( RxTime t1, RxTime t2 )
{
   if ( time_diff( t1, t2 ) == -2 )
      return t1 ;
   return t2 ;
}

/* check_for_waiting checks a client for a pending IO request.
 * We currently only support PULL requests.
 * The function returns immediately if the client doesn't wait.
 * In the other case it checks whether the maximum wait time has been
 * expired.
 * If the client is still waiting and the deadline isn't reached, it
 * checks if the deadline is more early then next_timeout and sets this
 * value is necessary.
 *
 * The client's queue's deadline is set to the default deadline of queues in
 * all cases.
 */
void check_for_waiting( Client *client, RxTime *next_timeout )
{
   int diff ;

   client->default_queue->deadline = queue_deadline ;
   /* Do we are a waiter? */
   if ( client->deadline.milli == -1 )
      return ;

   /*
    * Check if there is anything in the queue...
    */
   diff = time_diff( client->deadline, now ) ;
   if ( ( diff != -2 ) && ( diff != 0 ) )
   {
      DEBUGDUMP(
         if ( diff == -1 )
            printf("Still waiting infinitely for %d\n", client->socket );
         else
            printf("Still waiting %d ms at max for %d\n", diff, client->socket );
      );
      *next_timeout = earlier( *next_timeout, client->deadline ) ;
   }
   else
   {
      bad_news_for_waiter( client->default_queue, client ) ;
   }
}

/* check_queue checks a queue has reached its deadline and if no clients
 * have this queue as the default queue. The queue is deleted in this
 * case.
 * If the queuet is still valid and the deadline isn't reached, it
 * checks if the deadline is more early then next_timeout and sets this
 * value is necessary.
 */
void check_queue( RxQueue *q, RxTime *next_timeout )
{
   int diff ;
   Client *c ;

   diff = time_diff( q->deadline, now ) ;
   if ( ( diff != -2 ) && ( diff != 0 ) )
   {
      *next_timeout = earlier( *next_timeout, q->deadline ) ;
      return ;
   }

   for ( c = clients; c != NULL; c = c->next )
   {
      if ( c->default_queue == q )
      {
         q->deadline = queue_deadline ;
         *next_timeout = earlier( *next_timeout, q->deadline ) ;
         return ;
      }
   }

   DEBUGDUMP( printf( "Purging unused queue <%.*s>\n", PSTRENGLEN( q->name ), PSTRENGVAL( q->name ) ) );
   q->deadline = queue_deadline ; /* needed at least for SESSION */
   *next_timeout = earlier( *next_timeout, q->deadline ) ;
   delete_a_queue( q ) ;
}

int rxstack_doit( )
{
   RxTime timeout ;
   int listen_sock,msgsock;
   struct sockaddr_in server,client;
   int client_size ;
   int portno,rc;
   Client *c, *ch ;
   RxQueue *q, *qh ;
#ifdef HAVE_POLL_H
# define POLL_INCR 16
   struct pollfd *pd = NULL ;
   unsigned poll_max = 0 ;
   unsigned poll_cnt = 0 ;
#else
   int max_sock ;
   fd_set ready ;
   struct timeval to ;
#endif
#ifdef BUILD_NT_SERVICE
   char buf[30];
#endif
#if defined(SO_REUSEADDR) && defined(SOL_SOCKET)
   int on = 1;
#endif

   client_size = sizeof( struct sockaddr );
#ifdef WIN32
   if ( init_external_queue( NULL ) )
      return 1;
#endif

#ifdef BUILD_NT_SERVICE
   if ( IsItNT()
   &&   !report_service_pending_start() )
      goto notrunning;
#endif
   /*
    * Set up signal handler
    */
#ifdef SIGTERM
   signal( SIGTERM, rxstack_signal_handler );
#endif
#ifdef SIGINT
   signal( SIGINT, rxstack_signal_handler );
#endif
#ifdef SIGPIPE
   signal( SIGPIPE, SIG_IGN );
#endif
   clients = NULL ;
   queues = NULL ;

   /*
    * Initialise default "SESSION" queue
    */
   if ( ( SESSION = get_new_queue( ) ) == NULL )
   {
      showerror( ERR_STORAGE_EXHAUSTED, 0, ERR_STORAGE_EXHAUSTED_TMPL ) ;
      return ERR_STORAGE_EXHAUSTED ;
   }
   SESSION->name = Str_cre_or_exit( "SESSION", 7 ) ;
   SESSION->isReal = 1;

#ifdef HAVE_POLL_H
   pd = malloc( ( poll_max = POLL_INCR ) * sizeof( struct pollfd ) );
   if ( pd == NULL )
   {
      showerror( ERR_STORAGE_EXHAUSTED, 0, ERR_STORAGE_EXHAUSTED_TMPL );
      exit( ERR_STORAGE_EXHAUSTED );
   }
#endif

#ifdef BUILD_NT_SERVICE
   if ( IsItNT()
   &&   !report_service_pending_start() )
      goto notrunning;
#endif
   /*
    * Create listener socket
    */
   listen_sock = socket(AF_INET, SOCK_STREAM, 0);
   if (listen_sock < 0)
   {
      showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_GENERAL, ERR_RXSTACK_GENERAL_TMPL, "Listening on socket", errno_str( os_errno ) );
      rxstack_cleanup();
      exit(ERR_RXSTACK_GENERAL);
   }
   memset( &server, 0, sizeof(server) );
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = htonl(INADDR_ANY);
   portno = default_port_number();
   server.sin_port = htons((unsigned short) portno);

#ifdef BUILD_NT_SERVICE
   if ( IsItNT()
   &&   !report_service_pending_start() )
      goto notrunning;
#endif

#if defined(SO_REUSEADDR) && defined(SOL_SOCKET)
   setsockopt( listen_sock, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof( on ) );
#endif
   if ( bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) < 0)
   {
      showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_GENERAL, ERR_RXSTACK_GENERAL_TMPL, "Error binding socket", errno_str( os_errno ) );
      rxstack_cleanup();
      exit( ERR_RXSTACK_GENERAL );
   }
#ifdef BUILD_NT_SERVICE
   sprintf(buf, "Listening on port: %d", portno );
   if ( IsItNT() )
   {
      if ( !report_service_start() )
         goto notrunning;
      AddToMessageLog(TEXT(buf));
   }
   else
   {
      printf( "%s\n", buf );
      fflush(stdout);
   }
#else
   printf( "rxstack listening on port: %d\n", portno );
   fflush(stdout);
#endif
   /*
    * Start accepting connections
    */
   listen(listen_sock, 5);
   timeout = get_now( ) ;
   time_add( &timeout, DEFAULT_WAKEUP ) ;
   queue_deadline = now ;
   time_add( &queue_deadline, QUEUE_TIMEOUT ) ;
   while ( running )
   {
#ifdef HAVE_POLL_H
      poll_cnt = 0 ;
      pd[ poll_cnt   ].events = POLLIN ;
      pd[ poll_cnt++ ].fd = listen_sock ;
      DEBUGDUMP(printf("****** poll((%d", listen_sock););
      for ( c = clients; c != NULL; c = c->next )
      {
         if ( c->socket == -1 )
         {
            continue ;
         }

         if ( poll_cnt == poll_max )
         {
            pd = realloc( pd, ( poll_max += POLL_INCR ) * sizeof( struct pollfd ) );
            if ( pd == NULL )
            {
               showerror( ERR_STORAGE_EXHAUSTED, 0, ERR_STORAGE_EXHAUSTED_TMPL );
               exit( ERR_STORAGE_EXHAUSTED );
            }
         }

         pd[ poll_cnt   ].events = POLLIN ;
         pd[ poll_cnt++ ].fd = c->socket ;
         DEBUGDUMP(printf(", %d", c->socket););
      }
#else
      FD_ZERO(&ready);
      FD_SET(listen_sock, &ready);
      DEBUGDUMP(printf("****** select((%d", listen_sock););
      max_sock = listen_sock;
      /*
       * For each connected client, allow its socket
       * to be triggered
       */
      for ( c = clients; c != NULL; c = c->next )
      {
         if ( c->socket != -1 )
         {
            DEBUGDUMP(printf(", %d", c->socket););
            FD_SET( c->socket, &ready );
            if ( c->socket > max_sock )
               max_sock = c->socket;
         }
      }
#endif
      now = get_now( ) ;
      rc = time_diff( timeout, now ) ;
      if ( rc == -2 )
         rc = 0 ; /* already timed out */
      if ( ( rc == -1 ) || ( rc > DEFAULT_WAKEUP ) )
         rc = DEFAULT_WAKEUP ;
      DEBUGDUMP(printf("), to=%d) ms at %ld,%03d\n", rc, now.seconds, now.milli ););
#ifdef HAVE_POLL_H
      rc = poll( pd, poll_cnt, rc ) ;
#else
      to.tv_usec = ( rc % 1000 ) * 1000 ; /* microseconds fraction */
      to.tv_sec = rc / 1000;
      rc = select( max_sock + 1, &ready, (fd_set *)0, (fd_set *)0, &to ) ;
#endif
      now = get_now( ) ;
      DEBUGDUMP(printf("****** after waiting(), rc=%d at %ld,%03d\n", rc, now.seconds, now.milli ););
      if ( rc < 0 )
      {
         if ( os_errno != EINTR ) /* Win32 doesn't know about it ? */
         {
            showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_GENERAL, ERR_RXSTACK_GENERAL_TMPL, "Calling select", errno_str( os_errno ) );
            exit( ERR_RXSTACK_GENERAL );
         }
         continue ;
      }
      if ( rc )
      {
#ifdef HAVE_POLL_H
         if ( pd[ 0 ].revents )
#else
         if ( FD_ISSET(listen_sock, &ready ) )
#endif
         {
            msgsock = accept(listen_sock, (struct  sockaddr *)&client, (int *)&client_size);
            if (msgsock == -1)
            {
               showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_GENERAL, ERR_RXSTACK_GENERAL_TMPL, "Calling listen", errno_str( os_errno ) );
               rxstack_cleanup();
               exit( ERR_RXSTACK_GENERAL );
            }
            else
            {
               /*
                * A client has connected, create a client entry
                * and set their default queue to SESSION
                */
               /*
                * Validate the client here...TBD
                * use details in client sockaddr struct
                */
               DEBUGDUMP(printf("Client connecting from %s has socket %d\n", inet_ntoa( client.sin_addr ), msgsock ););
               rxstack_create_client( msgsock );
            }
         }
         else
         {
#ifdef HAVE_POLL_H
            for ( c = clients, poll_cnt = 1; c != NULL; poll_cnt++ )
#else
            for ( c = clients; c != NULL; )
#endif
            {
               /* c might be deleted by the following calls.
                * Assure we have everything perfect to use the
                * next element. An access to c after rxstack_process_command
                * is forbidden.
                */
               ch = c ;
               c = c->next ;
#ifdef HAVE_POLL_H
               if ( pd[ poll_cnt ].revents )
#else
               if ( FD_ISSET( ch->socket, &ready ) )
#endif
               {
                  /*
                   * Process the client's command...
                   */
                  rxstack_process_command( ch ) ;
               }
            }
         }
      }
      /*
       * If select() timed out or received input, check all connected clients who
       * may be waiting for input on one of the queues.
       *
       * now contains the time between the start of select() call and now
       * in milliseconds
       */
      now = get_now();
      timeout = get_now( ) ;
      time_add( &timeout, DEFAULT_WAKEUP ) ;
      queue_deadline = now ;
      time_add( &queue_deadline, QUEUE_TIMEOUT ) ;
      for ( c = clients; c != NULL; c = c->next )
      {
         check_for_waiting( c, &timeout );
      }
      for ( q = queues; q != NULL; )
      {
         qh = q ;
         q = q->next ;
         check_queue( qh, &timeout );
      }
   }
#ifdef BUILD_NT_SERVICE
notrunning:
#endif
   return 0;
}

/*
 * Gives a short usage description on stderr and returns 1
 */
static int usage( const char *argv0 )
{
   fprintf( stderr, "Usage: %s [-D] "
#if defined(HAVE_FORK)
                                     "[-d|-k]"
#else
                                     "[-k]"
#endif
                                             "\n", argv0 ) ;
   return 1 ;
}

static void checkDebug(void)
{
   if ( getenv( "RXDEBUG" ) != NULL )
      debug = 1 ;
}

int runNormal( int argc, char **argv )
{
   int rc = 0 ;
   const char *argv0 = argv[ 0 ] ;

   argv++ ;
   argc-- ;

   checkDebug();

   if ( ( argc >= 1 ) && ( strcmp( *argv, "-D" ) == 0 ) )
   {
      debug = 1 ;
      putenv( "RXDEBUG=1" ) ;
      argc-- ;
      argv++ ;
   }
   if ( argc > 1 )
   {
      return usage( argv0 );
   }
   if ( argc == 1 )
   {
      if ( strcmp(*argv, "-k") == 0 )
      {
         return suicide();
      }
      if ( strcmp(*argv, "-d") == 0 )
      {
#if defined(HAVE_FORK)
         if ( ( rc = fork() ) != 0 )
            exit(rc < 0);
         rc = rxstack_doit();
#else
         fprintf( stderr, "Option \"-d\" option is invalid on this platform.\n" ) ;
         return usage( argv0 );
#endif
      }
      else
      {
         return usage( argv0 );
      }
   }
   else
   {
      rc = rxstack_doit();
   }
   rxstack_cleanup();
   printf( "%s terminated.\n", argv0 );
   fflush(stdout);
   return rc;
}

#ifdef BUILD_NT_SERVICE
VOID ServiceStart(DWORD argc, LPTSTR *argv)
#else
int main(int argc, char *argv[])
#endif
{
#ifdef BUILD_NT_SERVICE
   if ( IsItNT() )
   {
      if ( !nt_service_start() )
      {
         checkDebug();
         rxstack_doit();
      }
      rxstack_cleanup();
      return;
   }
   else
   {
      runNormal(argc, argv);
   }
#else
   return runNormal( argc, argv );
#endif
}
