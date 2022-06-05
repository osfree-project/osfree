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

#define NO_CTYPE_REPLACEMENT
#include "rexx.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# include "configur.h"
#endif

#if defined(OS2) && defined(__WATCOMC__)
#include <os2/types.h>
#endif

#ifdef WIN32
# if defined(_MSC_VER)
#  if _MSC_VER >= 1100
/* Stupid MSC can't compile own headers without warning at least in VC 5.0 */
#   pragma warning(disable: 4115 4201 4214 4514)
#  endif
#  include <windows.h>
#  if _MSC_VER >= 1100
#   pragma warning(default: 4115 4201 4214)
#  endif
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
# ifdef HAVE_SYS_SELECT_H
#  include <sys/select.h>
# endif
# ifdef HAVE_NETDB_H
#  include <netdb.h>
# endif
# ifdef HAVE_ARPA_INET_H
#  include <arpa/inet.h>
# endif
# define closesocket(x) close(x)
#endif

#ifdef __LCC__
# include <winsock2.h>
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

#include "extstack.h"
#ifndef HAVE_GETOPT_LONG
# include "mygetopt.h"
#endif

#define SUCCESS(a) ((a)&&(a)->value[0] == '0')

/*
 * debugging is turned off. You can turn it on by the command line option "-D".
 */
static int debug = 0 ;
#define DEBUGDUMP(x) { if ( debug ) \
                          {x;}      \
                     }

static int fromtext = 0;

char *buff=NULL;
unsigned int bufflen=0;

static streng *session = NULL;

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

int Str_cmp( const streng *first, const streng *second )
{
   int lim;

   lim = Str_len( first );
   if ( lim != Str_len( second ) )
      return 1;

   return memcmp( first->value, second->value, lim );
}

int send_all( int sock, char *action )
{
   /*
    * Send everything in stdin to the socket. Each LF encountered
    * results in a send.
    * This code borrowed from rxstack.c in REXX/imc
    */
   int c,rc;
   unsigned int len;
   streng *result;

   for ( ; ; )
   {
      if ( fromtext )
      {
         len = strlen( buff );
      }
      else
      {
         len = 0;
         while ( ( c = getchar() ) != EOF )
         {
            if ( c == REGINA_EOL )
            {
   #if defined(DOS) || defined(OS2) || defined(WIN32)
               if (len && ( buff[len-1] == REGINA_CR ) )
                  len--;
   #endif
               break;
            }
            if ( len >= bufflen
            && (( buff = (char *)realloc( buff, bufflen <<= 1 ) ) == NULL ) )
            {
              showerror( ERR_STORAGE_EXHAUSTED, 0, ERR_STORAGE_EXHAUSTED_TMPL );
              exit( ERR_STORAGE_EXHAUSTED );
            }
            buff[len++] = (char)c;
         }
         if ( (c == EOF || feof(stdin) )
         &&  !len)
            break;
      }
      DEBUGDUMP(printf("--- Queue %s from %s ---\n", action[0] == RXSTACK_QUEUE_FIFO ? "FIFO" : "LIFO", fromtext ? "text" : "stdin"););
      rc = send_command_to_rxstack( NULL, sock, action, buff, len );
      if ( rc != -1 )
      {
         result = read_result_from_rxstack( NULL, sock, RXSTACK_HEADER_SIZE );
         DROPSTRENG( result );
      }
      if ( fromtext )
         break;
   }
   return 0;
}

char *get_unspecified_queue( void )
{
   char *rxq = getenv( "RXQUEUE" );

   if ( rxq == NULL )
      rxq = "SESSION";
   return rxq;
}

char *force_remote( char *rxq )
{
   char *h ;

   if ( strchr(rxq, '@' ) == NULL )
   {
      if ( ( h = (char *)malloc( strlen( rxq ) + 2 ) ) != NULL )
      {
         strcpy( h, rxq ) ;
         strcat( h, "@" ) ;
         return h ;
      }
   }
   return rxq ;
}

char *get_action( char *parm )
{
   char *action=NULL;

   if ( strcmp( parm, "/fifo" ) == 0 )
      action = RXSTACK_QUEUE_FIFO_STR;
   else if ( strcmp( parm, "/lifo" ) == 0 )
      action = RXSTACK_QUEUE_LIFO_STR;
   else if ( strcmp( parm, "/clear" ) == 0 )
      action = RXSTACK_EMPTY_QUEUE_STR;
   else if ( strcmp( parm, "/pull" ) == 0 )
      action = RXSTACK_PULL_STR;
   else if ( strcmp( parm, "/queued" ) == 0 )
      action = RXSTACK_NUMBER_IN_QUEUE_STR;
   else if ( strcmp( parm, "/delete" ) == 0 )
      action = RXSTACK_DELETE_QUEUE_STR;
   else if ( strcmp( parm, "/list" ) == 0 )
      action = RXSTACK_SHOW_QUEUES_STR;
   else
   {
      showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INVALID_SWITCH, ERR_RXSTACK_INVALID_SWITCH_TMPL, "/fifo, /lifo, /clear, /queued, /pull, /list" );
      /* not setting action will exit */
   }
   return action;
}

void junk_return_from_rxstack( int sock, streng *header )
{
   int length;
   streng *result;

   length = get_length_from_header( NULL, header );
   if ( length )
   {
      result = read_result_from_rxstack( NULL, sock, length );
      if ( result )
         DROPSTRENG( result );

   }
}

/*
 * Gives a short usage description on stderr and returns 1
 */
static int usage( const char *argv0 )
{
   fprintf( stdout, "\n%s: %s (%d bit). All rights reserved.\n", argv0, PARSE_VERSION_STRING, REGINA_BITS );
   fprintf( stdout,"Regina is distributed under the terms of the GNU Library General Public License \n" );
   fprintf( stdout,"and comes with NO WARRANTY. See the file COPYING-LIB for details.\n" );
   fprintf( stdout,"\n%s [switches] [queue] [command]\n", argv0 );
   fprintf( stdout,"where switches are:\n\n" );
   fprintf( stdout,"  --help, -h               show this message\n" );
   fprintf( stdout,"  --version, -v            display Regina version and exit\n" );
   fprintf( stdout,"  --debug, -D              turn on debugging\n" );
   fprintf( stdout,"  --text, -t               text to add to queue (ignores input on stdin)\n" );
   fprintf( stdout,"\n[queue] is the optional queue name to be used\n");
   fprintf( stdout,"\nand command is one of:\n\n" );
   fprintf( stdout,"  /fifo                    add text into [queue] first-in-first-out\n" );
   fprintf( stdout,"  /lifo                    add text into [queue] last-in-first-out\n" );
   fprintf( stdout,"  /clear                   clears all lines in [queue]\n" );
   fprintf( stdout,"  /delete                  delete queue\n" );
   fprintf( stdout,"  /pull                    pulls all lines from [queue]\n" );
   fprintf( stdout,"  /queued                  displays number of lines in [queue]\n" );
   fprintf( stdout,"  /list                    displays list of queues held by rxstack\n" );
   fflush( stdout );
   return 1 ;
}

int cleanup( void )
{
#ifdef WIN32
   WSACleanup();
#endif
   if ( buff )
      free( buff );
   return 0;
}

int main( int argc, char *argv[])
{
   int sock,rc=0,num,c;
   char *action;
   streng *queue=NULL,*server_name=NULL;
   char *in_queue=NULL;
   streng *result;
   Queue q;
   const char *argv0 = argv[ 0 ] ;
   static struct my_getopt_option long_options[] =
   {
      {"help",    no_argument,       0,  'h' },
      {"debug",   no_argument,       0,  'D' },
      {"text",    required_argument, 0,  't' },
      {"version", no_argument,       0,  'v' },
      {0,         0,                 0,  0 }
   };
#ifdef WIN32
   WORD wsver = (WORD)MAKEWORD(1,1);
   WSADATA wsaData;
#endif

#ifdef WIN32
   if ( WSAStartup( wsver, &wsaData ) != 0 )
   {
      showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_NO_WINSOCK, ERR_RXSTACK_NO_WINSOCK_TMPL, WSAGetLastError() );
      exit(ERR_RXSTACK_NO_WINSOCK);
   }
#endif
   if ( getenv( "RXDEBUG" ) != NULL )
      debug = 1 ;
   while (1)
   {
      int option_index = 0;

      c = my_getopt_long( argc, argv, "+hDt:v", long_options, &option_index );
      if ( c == -1 )
         break;

      switch(c)
      {
         case 'h':
            return usage( argv0 );
            break;
         case 'D': /* debug */
            debug = 1;
            putenv( "RXDEBUG=1" ) ;
            break;
         case 't': /* input from command line */
            fromtext = 1;
            buff = strdup( optarg );
            break;
         case 'v':
            fprintf( stderr, "%s: %s (%d bit)\n", argv0, PARSE_VERSION_STRING, REGINA_BITS );
            return 0;
         default: /* unknown switch */
            return usage( argv0 );
            break;
      }
   }
   num = argc - optind;

   action = NULL;
   /*
    * Process the command line
    */
   if ( num == 0 )
   {
      /*
       * "rxqueue"
       */
      action = RXSTACK_QUEUE_FIFO_STR;
      in_queue = get_unspecified_queue();
   }
   else if ( num == 1 )
   {
      /*
       * "rxqueue queue"
       * or
       * "rxqueue /switch"
       */
      if ( argv[optind][0] == '/' )
      {
         /*
          * Only parameter is a switch
          */
         in_queue = get_unspecified_queue();
         action = get_action( argv[optind] );
      }
      else
      {
         /*
          * Only parameter is a queue name
          */
         in_queue = argv[optind];
         action = RXSTACK_QUEUE_FIFO_STR;
      }
   }
   else if ( num == 2 )
   {
      /*
       * "rxqueue queue /switch"
       */
      in_queue = argv[optind];
      if ( argv[optind+1][0] == '/' )
      {
         /*
          * Parameter is a switch
          */
         action = get_action( argv[optind+1] );
      }
   }
   else
   {
      /*
       * ERROR
       */
      fprintf(stderr, "Invalid number of parameters\n");
      return usage( argv0 );
   }
   /*
    * Validate actions with -t or --text switches
    */
   if ( fromtext )
   {
      if ( action[0] == RXSTACK_EMPTY_QUEUE )
      {
         fprintf(stderr, "-t or --text switch invalid with /clear action.\n");
         return usage( argv0 );
      }
      else if ( action[0] == RXSTACK_PULL )
      {
         fprintf(stderr, "-t or --text switch invalid with /pull action.\n");
         return usage( argv0 );
      }
      else if ( action[0] == RXSTACK_NUMBER_IN_QUEUE )
      {
         fprintf(stderr, "-t or --text switch invalid with /queued action.\n");
         return usage( argv0 );
      }
      else if ( action[0] == RXSTACK_DELETE_QUEUE )
      {
         fprintf(stderr, "-t or --text switch invalid with /delete action.\n");
         return usage( argv0 );
      }
      else if ( action[0] == RXSTACK_SHOW_QUEUES )
      {
         fprintf(stderr, "-t or --text switch invalid with /list action.\n");
         return usage( argv0 );
      }
   }
   in_queue = force_remote( in_queue ) ;
   if ( action )
   {
      queue = MAKESTRENG( strlen( in_queue ) );
      if ( queue == NULL )
      {
         showerror( ERR_STORAGE_EXHAUSTED, 0, ERR_STORAGE_EXHAUSTED_TMPL );
         exit(ERR_STORAGE_EXHAUSTED);
      }
      memcpy( PSTRENGVAL( queue ), in_queue, PSTRENGLEN( queue ) );
      queue->len = strlen( in_queue ) ;
      /*
       * Parse the queue to determine server address
       * and queue name
       */
      if ( parse_queue( NULL, queue, &q ) == 1 )
      {
         if ( PSTRENGLEN( queue ) == 0 )
         {
            DROPSTRENG( queue );
            queue = MAKESTRENG( 7 );
            if ( queue == NULL )
            {
               showerror( ERR_STORAGE_EXHAUSTED, 0, ERR_STORAGE_EXHAUSTED_TMPL );
               exit(ERR_STORAGE_EXHAUSTED);
            }
            memcpy( PSTRENGVAL( queue ), "SESSION", 7 );
            queue->len = 7 ;
         }
         DEBUGDUMP(printf( "queue: <%.*s> server: %.*s<%d> Port:%d\n", PSTRENGLEN( queue ), PSTRENGVAL( queue ), PSTRENGLEN( q.u.e.name ), PSTRENGVAL( q.u.e.name ), q.u.e.address, q.u.e.portno ););
         sock = connect_to_rxstack( NULL, &q );
         if ( sock < 0 )
         {
            cleanup();
            exit(ERR_RXSTACK_CANT_CONNECT);
         }
         /*
          * Now process the actual command
          */
         switch( action[0] )
         {
            case RXSTACK_QUEUE_FIFO:
            case RXSTACK_QUEUE_LIFO:
               DEBUGDUMP(printf("--- Queue %s ", action[0] == RXSTACK_QUEUE_FIFO ? "FIFO" : "LIFO"););
               /*
                * Allocate the initial buffer if input is from stdin
                */
               if ( fromtext == 0 )
               {
                  if (( buff = (char *)malloc( bufflen = 256 ) ) == NULL )
                  {
                     showerror( ERR_STORAGE_EXHAUSTED, 0, ERR_STORAGE_EXHAUSTED_TMPL );
                     rc = ERR_STORAGE_EXHAUSTED;
                     break;
                  }
               }
               /*
                * Set the current queue
                */
               DEBUGDUMP(printf("(Set Queue) ---\n"););
               rc = send_command_to_rxstack( NULL, sock, RXSTACK_SET_QUEUE_STR, PSTRENGVAL( queue) , PSTRENGLEN( queue ) );
               if ( rc == -1 )
               {
                  showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Setting default queue" );
                  rc = ERR_RXSTACK_INTERNAL;
                  break;
               }
               result = read_result_from_rxstack( NULL, sock, RXSTACK_HEADER_SIZE );
               if ( !SUCCESS( result ) )
               {
                  showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Setting default queue" );
                  DROPSTRENG( result );
                  rc = ERR_RXSTACK_INTERNAL;
                  break;
               }
               /*
                * Success. We need to throw away the returned queue name. We
                * don't need it.
                */
               junk_return_from_rxstack( sock, result );
               send_all( sock, action );
               DROPSTRENG( result );
               rc = 0;
               break;
            case RXSTACK_EMPTY_QUEUE:
               DEBUGDUMP(printf("--- Empty Queue ---\n"););
               rc = send_command_to_rxstack( NULL, sock, action, PSTRENGVAL( queue) , PSTRENGLEN( queue ) );
               if ( rc == -1 )
               {
                  showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Emptying queue" );
                  rc = ERR_RXSTACK_INTERNAL;
                  break;
               }
               result = read_result_from_rxstack( NULL, sock, RXSTACK_HEADER_SIZE );
               if ( !SUCCESS( result ) )
               {
                  showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Emptying queue" );
                  DROPSTRENG( result );
                  rc = ERR_RXSTACK_INTERNAL;
                  break;
               }
               DROPSTRENG( result );
               rc = 0;
               break;
            case RXSTACK_NUMBER_IN_QUEUE:
               /*
                * Set the current queue
                */
               DEBUGDUMP(printf("--- Set Queue ---\n"););
               rc = send_command_to_rxstack( NULL, sock, RXSTACK_SET_QUEUE_STR, PSTRENGVAL( queue ), PSTRENGLEN( queue ) );
               if ( rc == -1 )
               {
                  showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Setting default queue" );
                  rc = ERR_RXSTACK_INTERNAL;
                  break;
               }
               /*
                * Read the return response header
                */
               result = read_result_from_rxstack( NULL, sock, RXSTACK_HEADER_SIZE );
               if ( !SUCCESS( result ) )
               {
                  showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Setting default queue" );
                  DROPSTRENG( result );
                  rc = ERR_RXSTACK_INTERNAL;
                  break;
               }
               /*
                * Success. We need to throw away the returned queue name. We
                * don't need it.
                */
               junk_return_from_rxstack( sock, result );
               DROPSTRENG( result );
               DEBUGDUMP(printf("--- Number in Queue ---\n"););
               num = get_number_in_queue_from_rxstack( NULL, sock, &rc );
               if ( rc == 0 )
                  printf("%d\n", num );
               break;
            case RXSTACK_DELETE_QUEUE:
               /*
                * Delete the specified queue
                */
               DEBUGDUMP(printf("--- Delete Queue ---\n"););
               session = MAKESTRENG( 7 );
               memcpy( PSTRENGVAL( session ), "SESSION", 7 );
               session->len = 7;
               if ( Str_cmp( session, queue ) == 0 )
               {
                  showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_ACTION_SESSION, ERR_RXSTACK_ACTION_SESSION_TMPL, "delete" );
                  rc = ERR_RXSTACK_INTERNAL;
                  break;
               }
               rc = send_command_to_rxstack( NULL, sock, action, PSTRENGVAL( queue) , PSTRENGLEN( queue ) );
               if ( rc == -1 )
               {
                  showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Deleting queue" );
                  rc = ERR_RXSTACK_INTERNAL;
                  break;
               }
               result = read_result_from_rxstack( NULL, sock, RXSTACK_HEADER_SIZE );
               if ( !SUCCESS( result ) )
               {
                  showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Deleting queue" );
                  DROPSTRENG( result );
                  rc = ERR_RXSTACK_INTERNAL;
                  break;
               }
               DROPSTRENG( result );
               rc = 0;
               break;
            case RXSTACK_SHOW_QUEUES:
               DEBUGDUMP(printf("----- Show Queues -----\n"););
               num = get_queues_from_rxstack( NULL, sock, &rc, &result );
               if ( rc == 0 )
               {
                  printf( "%.*s\n", PSTRENGLEN( result ), PSTRENGVAL( result ) ) ;
               }
               DROPSTRENG( result );
               break;
            case RXSTACK_PULL:
               /*
                * Set the current queue
                */
               DEBUGDUMP(printf("--- Set Queue ---\n"););
               rc = send_command_to_rxstack( NULL, sock, RXSTACK_SET_QUEUE_STR, PSTRENGVAL( queue ), PSTRENGLEN( queue ) );
               if ( rc == -1 )
               {
                  showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Setting default queue" );
                  rc = ERR_RXSTACK_INTERNAL;
                  break;
               }
               result = read_result_from_rxstack( NULL, sock, RXSTACK_HEADER_SIZE );
               if ( !SUCCESS( result ) )
               {
                  showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Setting default queue" );
                  DROPSTRENG( result );
                  rc = ERR_RXSTACK_INTERNAL;
                  break;
               }
               /*
                * Success. We need to throw away the returned queue name. We
                * don't need it.
                */
               junk_return_from_rxstack( sock, result );
               DROPSTRENG( result );
               /*
                * Loop until the number of lines on the queue is zero
                */
               for ( ; ; )
               {
                  DEBUGDUMP(printf("--- Pull ---\n"););
                  rc = get_line_from_rxstack( NULL, sock, &result, 0 );
                  if ( rc == 0 )
                  {
                     printf( "%.*s\n", PSTRENGLEN( result ), PSTRENGVAL( result ) ) ;
                  }
                  else if ( ( rc == 1 ) || ( rc == 4 ) )
                  {
                     /*
                      * Queue empty.
                      */
                     rc = 0;
                     break;
                  }
                  else
                  {
                     showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Getting line from queue" );
                     rc = ERR_RXSTACK_INTERNAL;
                     break;
                  }
                  DROPSTRENG( result );
               }
               break;
         }
         /*
          * Tell the server we want to disconnect
          * Don't expect any return result.
          */
         DEBUGDUMP(printf("--- Exit ---\n"););
         send_command_to_rxstack( NULL, sock, RXSTACK_EXIT_STR, NULL, 0 );
         closesocket(sock);
      }
      else
      {
         DEBUGDUMP(printf( "queue: <%.*s>\n", PSTRENGLEN( queue ), PSTRENGVAL( queue ) ););
         rc = 1;
      }
   }
   DROPSTRENG( server_name );
   DROPSTRENG( queue );
   cleanup();
   return rc;
}
