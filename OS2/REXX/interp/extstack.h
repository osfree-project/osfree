/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 2000  Mark Hessling <M.Hessling@qut.edu.au>
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
#include "rexxbif.h"            /* C functions that mimic REXX BIFs */

#define FIFO_LINE(buf,line) {                                            \
      /* This macro adds a line to a buffer in FIFO manner.              \
       * The line's higher/lower elements will be overwritten both.      \
       */                                                                \
      (line)->lower = NULL ;                                             \
      (line)->higher = (buf)->bottom ;                                   \
      (buf)->bottom = line ;                                             \
      if ( (line)->higher != NULL )                                      \
         (line)->higher->lower = (line) ;                                \
      else                                                               \
      {                                                                  \
         (buf)->top = (line) ;         /* buffer was empty previously */ \
         assert( (buf)->elements == 0 ) ;                                \
      }                                                                  \
      (buf)->elements++ ;                                                \
   }

#define LIFO_LINE(buf,line) {                                            \
      /* This macro adds a line to a buffer in LIFO manner.              \
       * The line's higher/lower elements will be overwritten both.      \
       */                                                                \
      (line)->higher = NULL ;                                            \
      (line)->lower = (buf)->top ;                                       \
      (buf)->top = line ;                                                \
      if ( (line)->lower != NULL )                                       \
         (line)->lower->higher = (line) ;                                \
      else                                                               \
      {                                                                  \
         (buf)->bottom = (line) ;      /* buffer was empty previously */ \
         assert( (buf)->elements == 0 ) ;                                \
      }                                                                  \
      (buf)->elements++ ;                                                \
   }

#define POP_LINE(buf,line) {                                             \
      /* This macro removes a line from a buffer. The argument "line"    \
       * will be overwritten. The higher/older elements of that value    \
       * won't be overwritten.                                           \
       */                                                                \
      if ( ( (line) = (buf)->top ) != NULL )                             \
      {                                                                  \
         (buf)->elements-- ;                                             \
         if ( ( (buf)->top = (line)->lower ) == NULL )                   \
         {                                                               \
            (buf)->bottom = NULL ;                                       \
            assert( (buf)->elements == 0 ) ;                             \
            (buf)->elements = 0 ;         /* if assert is defined out */ \
         }                                                               \
         else                                                            \
         {                                                               \
            (buf)->top->higher = NULL ;                                  \
         }                                                               \
      }                                                                  \
   }

#define GLUE_BUFFER1(buf1,buf2) {                                              \
      /* This macro glues buf2 at the top of buf1 and writes the result        \
       * to buf1. buf2's top/bottom/elements fields are reset.                 \
       */                                                                      \
      if ( (buf2)->bottom != NULL )                                            \
      {                                                                        \
         if ( (buf1)->top == NULL )                                            \
         {                                                                     \
            assert( ( (buf1)->bottom == NULL ) && ( (buf1)->elements == 0 ) ) ;\
            (buf1)->top = (buf2)->top ;                                        \
            (buf1)->bottom = (buf2)->bottom ;                                  \
            (buf1)->elements = (buf2)->elements ;                              \
         }                                                                     \
         else                                                                  \
         {                                                                     \
            (buf1)->top->higher = (buf2)->bottom ;                             \
            (buf2)->bottom->lower = (buf1)->top ;                              \
            (buf1)->top = (buf2)->top ;                                        \
            (buf1)->elements += (buf2)->elements ;                             \
         }                                                                     \
      }                                                                        \
      else                                                                     \
      {                                                                        \
         assert( ( (buf2)->top == NULL ) && ( (buf2)->elements == 0 ) ) ;      \
      }                                                                        \
      (buf2)->top = NULL ;                                                       \
      (buf2)->bottom = NULL ;                                                    \
      (buf2)->elements = 0 ;                                                     \
   }

#define GLUE_BUFFER2(buf1,buf2) {                                              \
      /* This macro glues buf2 at the top of buf1 and writes the result        \
       * to buf2. buf1's top/bottom/elements fields are reset.                 \
       */                                                                      \
      if ( (buf1)->bottom != NULL )                                            \
      {                                                                        \
         if ( (buf2)->top == NULL )                                            \
         {                                                                     \
            assert( ( (buf2)->bottom == NULL ) && ( (buf2)->elements == 0 ) ) ;\
            (buf2)->top = (buf1)->top ;                                        \
            (buf2)->bottom = (buf1)->bottom ;                                  \
            (buf2)->elements = (buf1)->elements ;                              \
         }                                                                     \
         else                                                                  \
         {                                                                     \
            (buf1)->top->higher = (buf2)->bottom ;                             \
            (buf2)->bottom->lower = (buf1)->top ;                              \
            (buf2)->bottom = (buf1)->bottom ;                                  \
            (buf2)->elements += (buf2)->elements ;                             \
         }                                                                     \
      }                                                                        \
      else                                                                     \
      {                                                                        \
         assert( ( (buf1)->top == NULL ) && ( (buf1)->elements == 0 ) ) ;      \
      }                                                                        \
      (buf1)->top = NULL ;                                                       \
      (buf1)->bottom = NULL ;                                                    \
      (buf1)->elements = 0 ;                                                     \
   }

void showerror( int err, int suberr, char *tmpl, ...);
int init_external_queue( const tsd_t *TSD );
void term_external_queue( void );
int default_port_number( void ) ;
int default_external_address( void ) ;
streng *default_external_name( const tsd_t *TSD ) ;
int connect_to_rxstack( tsd_t *TSD, Queue *q ) ;
int disconnect_from_rxstack( const tsd_t *TSD, Queue *q ) ;
int parse_queue( tsd_t *TSD, streng *queue, Queue *q ) ;
int send_command_to_rxstack( const tsd_t *TSD, int sock, const char *action, const char *str, int len );
streng *read_result_from_rxstack( const tsd_t *TSD, int sock, int result_size );
int delete_queue_from_rxstack( const tsd_t *TSD, int sock, const streng *queue_name );
int set_queue_in_rxstack( const tsd_t *TSD, int sock, const streng *queue_name );
int get_number_in_queue_from_rxstack( const tsd_t *TSD, int sock, int *errcode );
int clear_queue_on_rxstack( const tsd_t *TSD, int sock ) ;
int get_queue_from_rxstack( const tsd_t *TSD, const Queue *q, streng **result );
int create_queue_on_rxstack( const tsd_t *TSD, const Queue *q, const streng *queue, streng **result );
int timeout_queue_on_rxstack( const tsd_t *TSD, int sock, long timeout );
int get_line_from_rxstack( const tsd_t *TSD, int sock, streng **result, int nowait );
int queue_line_lifo_to_rxstack( const tsd_t *TSD, int sock, const streng *line );
int queue_line_fifo_to_rxstack( const tsd_t *TSD, int sock, const streng *line );
int get_length_from_header( const tsd_t *TSD, const streng *header );

#define RXSTACK_EXIT                'X'
#define RXSTACK_EXIT_STR            "X"
#define RXSTACK_KILL                'Z'
#define RXSTACK_KILL_STR            "Z"
#define RXSTACK_QUEUE_FIFO          'F'
#define RXSTACK_QUEUE_FIFO_STR      "F"
#define RXSTACK_QUEUE_LIFO          'L'
#define RXSTACK_QUEUE_LIFO_STR      "L"
#define RXSTACK_CREATE_QUEUE        'C'
#define RXSTACK_CREATE_QUEUE_STR    "C"
#define RXSTACK_DELETE_QUEUE        'D'
#define RXSTACK_DELETE_QUEUE_STR    "D"
#define RXSTACK_EMPTY_QUEUE         'E'
#define RXSTACK_EMPTY_QUEUE_STR     "E"
#define RXSTACK_PULL                'P'
#define RXSTACK_PULL_STR            "P"
#define RXSTACK_FETCH               'p'
#define RXSTACK_FETCH_STR           "p"
#define RXSTACK_SET_QUEUE           'S'
#define RXSTACK_SET_QUEUE_STR       "S"
#define RXSTACK_GET_QUEUE           'G'
#define RXSTACK_GET_QUEUE_STR       "G"
#define RXSTACK_NUMBER_IN_QUEUE     'N'
#define RXSTACK_NUMBER_IN_QUEUE_STR "N"
#define RXSTACK_TIMEOUT_QUEUE       'T'
#define RXSTACK_TIMEOUT_QUEUE_STR   "T"
#define RXSTACK_UNKNOWN             '?'
#define RXSTACK_UNKNOWN_STR         "?"
#define RXSTACK_HEADER_SIZE         7
#define RXSTACK_PEEK_HEADER_SIZE    2
#define RXSTACK_TIMEOUT_SIZE        6

#ifndef RXSOCKET
# define RXSOCKET 5757
#endif

#define ERR_RXSTACK_CANT_CONNECT       101
#define ERR_RXSTACK_CANT_CONNECT_TMPL    "Error connecting to %s on port %d: \"%s\""
#define ERR_RXSTACK_NO_IP              102
#define ERR_RXSTACK_NO_IP_TMPL           "Unable to obtain IP address for %s"
#define ERR_RXSTACK_INVALID_SERVER     103
#define ERR_RXSTACK_INVALID_SERVER_TMPL  "Invalid format for server in specified queue name: \"%s\""
#define ERR_RXSTACK_INVALID_QUEUE      104
#define ERR_RXSTACK_INVALID_QUEUE_TMPL   "Invalid format for queue name: \"%s\""
#define ERR_RXSTACK_NO_WINSOCK         105
#define ERR_RXSTACK_NO_WINSOCK_TMPL      "Unable to start Windows Socket interface: %s"
#define ERR_RXSTACK_TOO_MANY_QUEUES    106
#define ERR_RXSTACK_TOO_MANY_QUEUES_TMPL "Maximum number of external queues exceeded: %d"
#define ERR_RXSTACK_READING_SOCKET     107
#define ERR_RXSTACK_READING_SOCKET_TMPL  "Error occured reading socket: %s"
#define ERR_RXSTACK_INVALID_SWITCH     108
#define ERR_RXSTACK_INVALID_SWITCH_TMPL  "Invalid switch passed. Must be one of \"%s\""

#define ERR_RXSTACK_INTERNAL            99
#define ERR_RXSTACK_INTERNAL_TMPL        "Internal error with external queue interface: %d \"%s\""
#define ERR_RXSTACK_GENERAL            100
#define ERR_RXSTACK_GENERAL_TMPL         "General system error with external queue interface. %s. %s"

#define ERR_STORAGE_EXHAUSTED_TMPL       "System resources exhausted"

/*
 * Return codes from interacting with rxstack
 */
#define RXSTACK_OK      0
#define RXSTACK_EMPTY   1
#define RXSTACK_ERROR   2
#define RXSTACK_WAITING 3
#define RXSTACK_TIMEOUT 4
