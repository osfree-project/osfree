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

#include "rexx.h"

#if defined(WIN32)
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
#  include <winsock.h>
# endif
# include <io.h>
#else
# ifdef HAVE_TYPES_H
#  include <types.h>
# endif
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "extstack.h"

#ifdef EXTERNAL_TO_REGINA
# include <assert.h>
/* tmpstr_of must be defined before the inclusion of extstack.h since it
 * redefines tsd_t (grrr) */
volatile char *tmpstr_of( dummy_tsd_t *TSD, const streng *input )
{
   /* even exiterror isn't permitted at this point. */
   assert(0); /* hint while debugging */
   return((volatile char *) input->value);
}
#endif

static int debug = -1 ;
#define DEBUGDUMP(x) {                                                       \
                        if ( debug == -1 )                                   \
                        {                                                    \
                           debug = ( getenv( "RXDEBUG" ) == NULL ) ? 0 : 1 ; \
                        }                                                    \
                        if ( debug )                                         \
                        {                                                    \
                           x ;                                               \
                        }                                                    \
                     }

/* "localhost" maps to "127.0.0.1" on most systems but it may use DNS on
 * badly configured systems. Use "127.0.0.1" to bypass any errors.
 */
static const char ReginaLocalHost[] = "127.0.0.1";

void showerror( int err, int suberr, char *tmpl, ...)
{
   va_list argptr;
   if ( suberr )
      fprintf( stderr, "Error:%d.%d - ", err, suberr );
   else
      fprintf( stderr, "Error:%d - ", err );
   va_start( argptr, tmpl );
   vfprintf( stderr, tmpl, argptr );
   va_end( argptr );
   fprintf( stderr, "\n" );
}

int init_external_queue( const tsd_t *TSD )
{
   int rc=0;
#ifdef WIN32
   WORD wsver = (WORD)MAKEWORD(1,1);
   WSADATA wsaData;
   if ( WSAStartup( wsver, &wsaData ) != 0 )
   {
      /* TSD will be NULL when called from rxqueue or rxstack */
      if ( TSD == NULL )
         showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_NO_WINSOCK, ERR_RXSTACK_NO_WINSOCK_TMPL, WSAGetLastError() );
      else if ( !TSD->called_from_saa )
         exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_NO_WINSOCK, WSAGetLastError() );
      rc = 1;
   }
#endif
   return rc;
}

void term_external_queue( void )
{
#ifdef WIN32
   WSACleanup();
#endif
}

int get_length_from_header( const tsd_t *TSD, const streng *header )
{
   int length=0,error=1;
   streng *result=NULL;

   result = MAKESTRENG( RXSTACK_HEADER_SIZE - 1 );
   if ( result )
   {
      result->len = RXSTACK_HEADER_SIZE - 1;
      memcpy( result->value, header->value+1, RXSTACK_HEADER_SIZE-1 );
      DEBUGDUMP(printf("Hex value: %.*s\n", PSTRENGLEN(result),PSTRENGVAL(result)););
      length = REXX_X2D( result, &error );
      DROPSTRENG( result );
   }
   return ( error ) ? 0 : length;
}

#if !defined(NO_EXTERNAL_QUEUES)
int default_port_number( void )
{
   int portno;
   char dummy;
   char *port = getenv("RXSTACK"); /* FIXME: May be overwritten by VALUE-BIF */

   if ( port != NULL )
   {
      if ( sscanf( port, "%d %c", &portno, &dummy ) == 1 )
         if ( (portno > 1) && (portno < 0xFFFF) )
            return portno;
   }
   return RXSOCKET;
}

int default_external_address( void )
{
   return inet_addr( ReginaLocalHost );
}

streng *default_external_name( const tsd_t *TSD )
{
   int len;
   streng *result;

   len = sizeof(ReginaLocalHost); /* includes the term. \0 */
   result = MAKESTRENG( len );
   result->len = len - 1;
   memcpy( result->value, ReginaLocalHost, len ) ;
   return result ;
}

int connect_to_rxstack( tsd_t *TSD, Queue *q )
{
   struct sockaddr_in server;
   int eno;

   /*
    * Connect to external rxstack process/daemon/service
    */
   assert( q->type == QisExternal ) ;
   DEBUGDUMP(printf("In connect_to_rxstack: q = {name=%.*s, address=%08X, portno=%d}\n", q->u.e.name->len, q->u.e.name->value, q->u.e.address, q->u.e.portno););
   memset( &server, 0, sizeof(server) );
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = q->u.e.address;
   server.sin_port = htons((unsigned short) q->u.e.portno);

   q->u.e.socket = socket( AF_INET, SOCK_STREAM, 0 );
   if ( q->u.e.socket >= 0 )
   {

      if ( connect( q->u.e.socket, (struct sockaddr *)&server, sizeof(server) ) >= 0 )
      {
         DEBUGDUMP(printf("In connect_to_rxstack: socket=%d\n", q->u.e.socket););
         return(q->u.e.socket);
      }
      eno = errno;
      closesocket(q->u.e.socket);
      q->u.e.socket = -1;
      errno = eno;
   }
   /* TSD will be NULL when called from rxqueue or rxstack */
   if ( TSD == NULL )
      showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_CANT_CONNECT, ERR_RXSTACK_CANT_CONNECT_TMPL, q->u.e.name->value, q->u.e.portno, strerror ( errno ) );
   else if ( !TSD->called_from_saa )
      exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_CANT_CONNECT, tmpstr_of( TSD, q->u.e.name ), q->u.e.portno, strerror ( errno ) );

   return -1;
}

int disconnect_from_rxstack( const tsd_t *TSD, Queue *q )
{
   int rc;

   assert( q->type == QisExternal ) ;
   if (q->u.e.socket != -1)
   {
      DEBUGDUMP(printf("Disconnecting from socket %d\n", q->u.e.socket ););
      rc = send_command_to_rxstack( TSD, q->u.e.socket, RXSTACK_EXIT_STR, NULL, 0 );
      closesocket( q->u.e.socket );
   }
   else
      rc = 0; /* success: 0 bytes transfered */
   if (q->u.e.name != NULL)
      DROPSTRENG( q->u.e.name );
   memset( q, 0, sizeof(Queue) ) ;
   q->type = QisUnused ;
   return rc;
}

int send_command_to_rxstack( const tsd_t *TSD, int sock, const char *action, const char *str, int len )
{
   streng *qlen, *header;
   int rc=-1;

   DEBUGDUMP(printf("\n--> Sending to %d Action: %s <%.*s> Len:%d\n", sock, action, len, (str) ? str : "", len););
   qlen = REXX_D2X( len );
   if ( qlen )
   {
      header = REXX_RIGHT( qlen, RXSTACK_HEADER_SIZE, '0');
      DROPSTRENG( qlen );
      if ( header )
      {
         header->value[0] = action[0];
         rc = send( sock, PSTRENGVAL(header), PSTRENGLEN(header), 0 );
         DEBUGDUMP(printf("Send length: %.*s(%d) rc %d\n", PSTRENGLEN(header),PSTRENGVAL(header),PSTRENGLEN(header),rc););
         if ( str && rc != -1 )
         {
            rc = send( sock, str, len, 0 );
            DEBUGDUMP(printf("Send str length: %d\n", rc););
         }
         DROPSTRENG( header );
      }
   }
   return rc;
}

static int inject_result_from_rxstack( int sock, streng *str, int size )
{
  /*
   * Reads size bytes from sock and adds them to str. No overflow checking
   * is performed.
   * returns the result from recv.
   */
   int rc;

   rc = recv( sock, PSTRENGVAL(str) + PSTRENGLEN(str), size, 0 );
   str->len += size;
   DEBUGDUMP(printf("<-- Recv result: %.*s(%d) rc %d\n", size, PSTRENGVAL(str) + PSTRENGLEN(str), PSTRENGLEN(str), rc ); );
   return rc;
}

streng *read_result_from_rxstack( const tsd_t *TSD, int sock, int result_size )
{
  /*
   * Creates a streng of length 'result_size' and if non-zero,
   * reads this many characters from the socket.
   * The caller MUST DROPSTRENG this streng.
   */
   streng *result;
   int rc;

   result = MAKESTRENG( result_size );
   if ( result )
   {
      if ( result_size )
      {
         result->len = 0 ; /* MAKESTRENG sets ->len to nonzero sometimes */
         rc = inject_result_from_rxstack( sock, result, result_size );
      }
   }
   return result;
}

/* parse_queue validates the queue name. The format is [queue][@host[:port]]
 * *eq is filled and may contain default values.
 * queue may be empty to designate the current/default queue.
 * host may be empty to designate the local host. It has to be either a
 * IPv4 hostname or a IPv4 IP address.
 * The [@host[:port]] part is chopped off and the pure queue name remains.
 * return values:
 * -code: error detected (exiterror() is thrown normally!), code is one
 *        or RXQUEUE_...
 *     0: queue is NULL
 *     1: queue is parsed successfully, either with or without host part.
 */
int parse_queue( tsd_t *TSD, streng *queue, Queue *q )
{
   int len, AtPos;
   char *h, dummy;
   struct hostent *he;

   q->type = QisExternal ;
   q->u.e.portno = 0;  /* Good practise for initialisation */
   q->u.e.socket = -1;
   q->u.e.address = 0;
   q->u.e.name = NULL;

   if ( queue == NULL )
      return 0 ;

   len = PSTRENGLEN( queue );
   if (( h = (char *)memchr( PSTRENGVAL( queue ), '@', len ) ) == NULL )
      return 1 ;

   AtPos = (int) ( h - PSTRENGVAL( queue ) ) ;
   h++ ;
   len -= AtPos + 1 ;

   /* h is at host, len contains its length. We now do a trick. We copy the
    * complete host part into q->u.e.name, but a '\0' appended. We can simply
    * check out the portno part and we can apply sscanf() on it. Working with
    * a temporary buffer for the portno has no benefit, we can save 5 byte
    * per connection with a maximum of 1 permanent connection ==> 5 byte
    */
   q->u.e.name = MAKESTRENG( len + 1 ) ;
   if (q->u.e.name == NULL )
   {
      /* TSD will be NULL when called from rxqueue or rxstack */
      if ( TSD == NULL )
         showerror( ERR_STORAGE_EXHAUSTED, 0, ERR_STORAGE_EXHAUSTED_TMPL );
      else if ( !TSD->called_from_saa )
         exiterror( ERR_STORAGE_EXHAUSTED, 0 );
      return -4;  /* RXQUEUE_NOEMEM */
   }
   memcpy( PSTRENGVAL( q->u.e.name ), h, len ) ;
   PSTRENGVAL( q->u.e.name )[len] = '\0' ;
   q->u.e.name->len = len ;

   if (( h = (char *)memchr( PSTRENGVAL( q->u.e.name ), ':', len ) ) == NULL )
      q->u.e.portno = default_port_number();
   else
   {
      q->u.e.name->len = (int) ( h - PSTRENGVAL( q->u.e.name ) ) ;
      *h++ = '\0' ; /* chop off the portno part and jump over ':' */
      if ( sscanf( h, "%d %c", &q->u.e.portno, &dummy ) != 1 )
         q->u.e.portno = 0;
      if ( ( q->u.e.portno < 1 ) || ( q->u.e.portno > 0xFFFF ) )
      {
         if ( TSD == NULL )
            showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INVALID_QUEUE, ERR_RXSTACK_INVALID_QUEUE_TMPL, PSTRENGVAL( queue ) );
         else if ( !TSD->called_from_saa )
            exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INVALID_QUEUE, tmpstr_of( TSD, queue ) );
         DROPSTRENG( q->u.e.name ) ;
         q->u.e.name = NULL ;
         return -5; /* RXQUEUE_BADQNAME */
      }
   }

   h = PSTRENGVAL( q->u.e.name ) ;
   if ( *h == '\0' )
   {
      q->u.e.address = default_external_address();
      DROPSTRENG( q->u.e.name );
      q->u.e.name = default_external_name( TSD );
   }
   else
   {
      /* h is either a dotted name or a host name, always try the dotted one
       * first. It's MUCH faster.
       */
      q->u.e.address = inet_addr( h );
      if ( ( q->u.e.address == 0 ) || ( q->u.e.address == -1 ) ) /* various errors */
      {
         he = gethostbyname( h ) ;
         if ( ( he != NULL )
           && ( he->h_addr != NULL )
           && ( he->h_addrtype == AF_INET ) )
            q->u.e.address = ( ( struct in_addr * ) he->h_addr )->s_addr;
         if ( ( q->u.e.address == 0 ) || ( q->u.e.address == -1 ) )
         {
            /* TSD will be NULL when called from rxqueue or rxstack */
            if ( TSD == NULL )
               showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_NO_IP, ERR_RXSTACK_NO_IP_TMPL, PSTRENGVAL( q->u.e.name ) );
            else if ( !TSD->called_from_saa )
               exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_NO_IP, tmpstr_of( TSD, q->u.e.name ) );
            DROPSTRENG( q->u.e.name ) ;
            q->u.e.name = NULL ;
            return -5; /* RXQUEUE_BADQNAME */
         }
      }
   }

   queue->len = AtPos ; /* chop off the host part */
   return 1 ;
}

int delete_queue_from_rxstack( const tsd_t *TSD, int sock, const streng *queue_name )
{
   int rc;
   streng *result;

   rc = send_command_to_rxstack( TSD, sock, RXSTACK_DELETE_QUEUE_STR, PSTRENGVAL( queue_name ), PSTRENGLEN( queue_name ) );
   if ( rc != -1 )
   {
      result = read_result_from_rxstack( TSD, sock, RXSTACK_HEADER_SIZE );
      if ( result )
      {
         rc = result->value[0]-'0';
         DROPSTRENG( result );
      }
   }
   return rc;
}

int timeout_queue_on_rxstack( const tsd_t *TSD, int sock, long timeout )
{
   int rc=0;
   streng *result,*qtimeout, *hex_timeout;

   qtimeout = REXX_D2X( timeout );
   if ( qtimeout )
   {
      hex_timeout = REXX_RIGHT( qtimeout, RXSTACK_TIMEOUT_SIZE, '0');
      DROPSTRENG( qtimeout );
      if ( hex_timeout )
      {
         DEBUGDUMP(printf("Send timeout: %.*s(%d) rc %d\n", PSTRENGLEN(hex_timeout),PSTRENGVAL(hex_timeout),PSTRENGLEN(hex_timeout),rc););
         rc = send_command_to_rxstack( TSD, sock, RXSTACK_TIMEOUT_QUEUE_STR, PSTRENGVAL(hex_timeout), PSTRENGLEN(hex_timeout) );
         DROPSTRENG( hex_timeout );
         if ( rc != -1 )
         {
            result = read_result_from_rxstack( TSD, sock, RXSTACK_HEADER_SIZE );
            if ( result )
            {
               rc = result->value[0]-'0';
               DROPSTRENG( result );
            }
         }
      }
   }
   return rc;
}

int get_number_in_queue_from_rxstack( const tsd_t *TSD, int sock, int *errcode )
{
   int rc,length=0;
   streng *header;

   rc = send_command_to_rxstack( TSD, sock, RXSTACK_NUMBER_IN_QUEUE_STR, NULL, 0 );
   if ( rc != -1 )
   {
      header = read_result_from_rxstack( TSD, sock, RXSTACK_HEADER_SIZE );
      if ( header )
      {
         rc = header->value[0]-'0';
         if ( rc == 0 )
         {
            /*
             * now get the length from the header
             */
            DEBUGDUMP(printf("before get_length_from_header: %.*s\n", header->len, header->value););
            length = get_length_from_header( TSD, header );
         }
         else
         {
            /* TSD will be NULL when called from rxqueue or rxstack */
            if ( TSD == NULL )
               showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Getting number in queue" );
            else if ( !TSD->called_from_saa )
               exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, rc, "Getting number in queue"  );
            rc = 9; /* RXQUEUE_NOTREG */
         }
         DROPSTRENG( header );
      }
   }
   if ( errcode )
      *errcode = rc;
   return length;
}

int get_queues_from_rxstack( const tsd_t *TSD, int sock, int *errcode , streng **result )
{
   int rc,length=0;
   streng *header;

   DEBUGDUMP(printf("before send_command_to_rxstack:\n"););
   rc = send_command_to_rxstack( TSD, sock, RXSTACK_SHOW_QUEUES_STR, NULL, 0 );
   if ( rc != -1 )
   {
      header = read_result_from_rxstack( TSD, sock, RXSTACK_HEADER_SIZE );
      if ( header )
      {
         rc = header->value[0]-'0';
         if ( rc == 0 )
         {
            /*
             * now get the length from the header
             */
            DEBUGDUMP(printf("before get_length_from_header: %.*s\n", header->len, header->value););
            length = get_length_from_header( TSD, header );
            *result = read_result_from_rxstack( TSD, sock, length );
         }
         else
         {
            /* TSD will be NULL when called from rxqueue or rxstack */
            if ( TSD == NULL )
               showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Getting queues" );
            else if ( !TSD->called_from_saa )
               exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, rc, "Getting queues"  );
            rc = 9; /* RXQUEUE_NOTREG */
         }
         DROPSTRENG( header );
      }
   }
   if ( errcode )
      *errcode = rc;
   return length;
}

int clear_queue_on_rxstack( const tsd_t *TSD, int sock )
{
   int rc ;
   streng *result ;

   rc = send_command_to_rxstack( TSD, sock, RXSTACK_EMPTY_QUEUE_STR, NULL, 0 );
   if ( rc != -1 )
   {
      result = read_result_from_rxstack( TSD, sock, RXSTACK_HEADER_SIZE );
      if ( result )
      {
         rc = result->value[0]-'0';
         DROPSTRENG( result );
      }
   }
   return rc ;
}

static streng *init_connect_string( const tsd_t *TSD, const Queue *q, int addlength )
{
   /* Determines the length of the connect string of eq and sums up
    * addlength. A streng of that size is created and returned.
    */
   streng *retval ;
   int len = 0 ;

   assert( q->type == QisExternal ) ;
   if ( q->u.e.name != 0 )
      len = PSTRENGLEN( q->u.e.name ) ;
   if ( len == 0 )
      len = 15 ; /* enough for "xxx.xxx.xxx.xxx" */
   len += 8 ;    /* enough for "@" and ":65535" and a terminating 0 */

   retval = MAKESTRENG( len + addlength ) ;
   if ( retval == NULL )
   {
      /* TSD will be NULL when called from rxqueue or rxstack */
      if ( TSD == NULL )
         showerror( ERR_STORAGE_EXHAUSTED, 0, ERR_STORAGE_EXHAUSTED_TMPL );
      else if ( !TSD->called_from_saa )
         exiterror( ERR_STORAGE_EXHAUSTED, 0 );
   }
   else
      retval->len = 0 ; /* MAKESTRENG sets ->len to nonzero sometimes */
   return retval ;
}

static void add_connect_string( const Queue *q, streng *str )
{
   /* Adds the connect string of eq to str.
    * This may be "@localhost:5757"
    * We always add the port. It may be redefined in between by a call to
    * the VALUE BIF.
    */
   int sum = 0, len ;
   char *ptr = PSTRENGVAL( str ) ;
   char *addr ;
   struct in_addr ia ;

   assert( q->type == QisExternal ) ;
   ptr += PSTRENGLEN( str ) ;
   *ptr++ = '@' ;
   sum++ ;
   if ( q->u.e.name != 0 )
   {
      len = PSTRENGLEN( q->u.e.name ) ;
      memcpy( ptr, PSTRENGVAL( q->u.e.name ), len ) ;
      ptr += len ;
      sum += len ;
   }
   if ( sum == 1 ) /* no q->u.e.name? use the dotted IP address */
   {
      ia.s_addr = q->u.e.address ;
      addr = inet_ntoa( ia ) ;
      len = strlen( addr ) ;
      memcpy( ptr, addr, len ) ;
      ptr += len ;
      sum += len ;
   }
   /* finally add the port */
   sum += sprintf( ptr, ":%u", (unsigned) q->u.e.portno ) ;
   str->len += sum ;
}

int get_queue_from_rxstack( const tsd_t *TSD, const Queue *q, streng **result )
{
   int rc,length;
   streng *header;

   assert( q->type == QisExternal ) ;
   rc = send_command_to_rxstack( TSD, q->u.e.socket, RXSTACK_GET_QUEUE_STR, NULL, 0 );
   if ( rc != -1 )
   {
      header = read_result_from_rxstack( TSD, q->u.e.socket, RXSTACK_HEADER_SIZE );
      if ( header )
      {
         rc = header->value[0]-'0';
         if ( rc == 0 )
         {
            /*
             * now get the length from the header and get that many characters...
             */
            length = get_length_from_header( TSD, header );
            if ( ( *result = init_connect_string( TSD, q, length ) ) != NULL )
            {
               inject_result_from_rxstack( q->u.e.socket, *result, length ) ;
               add_connect_string( q, *result );
            }
            else
               rc = 4; /* RXQUEUE_NOEMEM, not really used */
         }
         else
         {
            /* TSD will be NULL when called from rxqueue or rxstack */
            if ( TSD == NULL )
               showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Getting queue from stack" );
            else if ( !TSD->called_from_saa )
               exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, rc, "Getting queue from stack" );
         }
         DROPSTRENG( header );
      }
   }
   return rc;
}

int get_line_from_rxstack( const tsd_t *TSD, int sock, streng **result, int nowait )
{
   int rc,length;
   streng *header;

   rc = send_command_to_rxstack( TSD, sock, (nowait) ? RXSTACK_FETCH_STR : RXSTACK_PULL_STR, NULL, 0 );
   if ( rc != -1 )
   {
      header = read_result_from_rxstack( TSD, sock, RXSTACK_HEADER_SIZE );
      if ( header )
      {
         rc = header->value[0]-'0';
         DEBUGDUMP(printf("rc from read_result_from_rxstack=%d\n", rc););
         switch( rc )
         {
            case RXSTACK_OK:
               /*
                * now get the length from the header and get that many characters...
                */
               length = get_length_from_header( TSD, header );
               *result = read_result_from_rxstack( TSD, sock, length );
               break;
            case RXSTACK_TIMEOUT:
            case RXSTACK_EMPTY:
               /*
                * queue is empty - return a NULL
                */
               *result = NULL;
               break;
            default:
               /* TSD will be NULL when called from rxqueue or rxstack */
               if ( TSD == NULL )
                  showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Getting line from queue" );
               else if ( !TSD->called_from_saa )
                  exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, rc, "Getting line from queue" );
            break;
         }
         DROPSTRENG( header );
      }
   }
   return rc;
}

int create_queue_on_rxstack( const tsd_t *TSD, const Queue *q, const streng *queue, streng **result )
{
   int rc,length;
   streng *header;

   assert( q->type == QisExternal ) ;
   rc = send_command_to_rxstack( TSD, q->u.e.socket, RXSTACK_CREATE_QUEUE_STR, (queue) ? PSTRENGVAL( queue ) : NULL, (queue) ? PSTRENGLEN( queue ) : 0 );
   if ( rc != -1 )
   {
      header = read_result_from_rxstack( TSD, q->u.e.socket, RXSTACK_HEADER_SIZE );
      if ( header )
      {
         rc = header->value[0]-'0';
         if ( ( rc == 0 ) || ( rc == 1 ) )
         {
            /*
             * now get the length from the header and get that many characters...
             */
            length = get_length_from_header( TSD, header );
            if ( ( *result = init_connect_string( TSD, q, length ) ) != NULL )
            {
               inject_result_from_rxstack( q->u.e.socket, *result, length ) ;
               add_connect_string( q, *result );
            }
            else
               rc = 4; /* RXQUEUE_NOEMEM */
         }
         else
         {
            /* TSD will be NULL when called from rxqueue or rxstack */
            if ( TSD == NULL )
               showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Creating queue" );
            else if ( !TSD->called_from_saa )
               exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, rc, "Creating queue" );
            switch ( rc )
            {
               case 2: rc = 5; break; /* RXQUEUE_BADQNAME */
               case 3: rc = 4; break; /* RXQUEUE_NOEMEM */
               case 6: rc = 1; break; /* RXQUEUE_STORAGE */
               default:;
            }
         }
         DROPSTRENG( header );
      }
   }
   return rc;
}

int set_queue_in_rxstack( const tsd_t *TSD, int sock, const streng *queue_name )
{
   int rc,length;
   streng *header, *dummy;

   rc = send_command_to_rxstack( TSD, sock, RXSTACK_SET_QUEUE_STR, PSTRENGVAL( queue_name ), PSTRENGLEN( queue_name ) );
   if ( rc != -1 )
   {
      header = read_result_from_rxstack( TSD, sock, RXSTACK_HEADER_SIZE );
      if ( header )
      {
         rc = header->value[0]-'0';
         if ( rc == 0 )
         {
            /*
             * now get the length from the header and get that many characters...
             */
            length = get_length_from_header( TSD, header );
            dummy = read_result_from_rxstack( TSD, sock, length );
            /* dummy is no longer used */
            DROPSTRENG( dummy ) ;
         }
         else
         {
            /* TSD will be NULL when called from rxqueue or rxstack */
            if ( TSD == NULL )
               showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Setting queue" );
            else if ( !TSD->called_from_saa )
               exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, rc, "Setting queue" );
            switch ( rc )
            {
               case 3: rc = 4; break; /* RXQUEUE_NOEMEM */
               case 6: rc = 1; break; /* RXQUEUE_STORAGE */
               default:;
            }
         }
         DROPSTRENG( header );
      }
   }
   else
      rc = 100; /* RXQUEUE_NETERROR */
   return rc;
}

int queue_line_fifo_to_rxstack( const tsd_t *TSD, int sock, const streng *line )
{
   int rc;
   streng *header;

   rc = send_command_to_rxstack( TSD, sock, RXSTACK_QUEUE_FIFO_STR, PSTRENGVAL( line ), PSTRENGLEN( line ) );
   if ( rc != -1 )
   {
      header = read_result_from_rxstack( TSD, sock, RXSTACK_HEADER_SIZE );
      if ( header )
      {
         rc = header->value[0]-'0';
         if ( rc != 0 )
         {
            /* TSD will be NULL when called from rxqueue or rxstack */
            if ( TSD == NULL )
               showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Queueing line" );
            else if ( !TSD->called_from_saa )
               exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, rc, "Queueing line" );
            switch ( rc )
            {
               case 2: rc = 5; break; /* RXQUEUE_BADQNAME */
               case 3: rc = 4; break; /* RXQUEUE_NOEMEM */
               case 6: rc = 1; break; /* RXQUEUE_STORAGE */
               default:;
            }
         }
         DROPSTRENG( header );
      }
   }
   return rc;
}

int queue_line_lifo_to_rxstack( const tsd_t *TSD, int sock, const streng *line )
{
   int rc;
   streng *header;

   rc = send_command_to_rxstack( TSD, sock, RXSTACK_QUEUE_LIFO_STR, PSTRENGVAL( line ), PSTRENGLEN( line ) );
   if ( rc != -1 )
   {
      header = read_result_from_rxstack( TSD, sock, RXSTACK_HEADER_SIZE );
      if ( header )
      {
         rc = header->value[0]-'0';
         if ( rc != 0 )
         {
            /* TSD will be NULL when called from rxqueue or rxstack */
            if ( TSD == NULL )
               showerror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, ERR_RXSTACK_INTERNAL_TMPL, rc, "Queueing line" );
            else if ( !TSD->called_from_saa )
               exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, rc, "Queueing line" );
            switch ( rc )
            {
               case 2: rc = 5; break; /* RXQUEUE_BADQNAME */
               case 3: rc = 4; break; /* RXQUEUE_NOEMEM */
               case 6: rc = 1; break; /* RXQUEUE_STORAGE */
               default:;
            }
         }
         DROPSTRENG( header );
      }
   }
   return rc;
}
#endif
