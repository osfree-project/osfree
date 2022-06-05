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

/* Concept of implementation:

 * In this description, the stack is described as growing upwards

 * The stack has two pointers to the ends of a double-linked list,
 * which contains entries consisting of just a pointer to the text (in
 * a struct streng) in addition to pointers to next and previous
 * entry.  Last box' next, and first box' prev is both NIL. The text
 * pointer points to the text of that line in the stack.

 * If the text pointer is NIL, then that box in the stack-structure is
 * just symbolizing a stack buffer mark. The integer variable
 * 'buffers' contains the current number of buffermarks in the stack.
 * A buffer is the lines between two buffermarks. The first buffer mark
 * (for buffer 0) is not marked, but is implicit the bottom of the
 * stack. The last buffer is the lines between the uppermost
 * buffermark and the top of the stack.

 * Initialy all lines are put in buffer 0. When a new buffer is
 * created, lines pushed lifo is put on top of that buffer, and lines
 * queued fifo are put in the bottom of that buffer (i.e. not in the
 * bottom of buffer 0.) Lines pushed on the stack are 'eaten' and
 * there must not be any referances to them in other parts of the
 * program after they have been pushed.

 * When reading lines from the stack, and all lines in the current
 * buffer has been read, the buffer-mark will be removed, and lines
 * are read from the buffer underneath. Lines read from the stack must
 * be free'ed by the routine that popped them from the stack.

 * When the whole stack is empty, lines are read from standard input.

 * Buffer 0 contains the lines between the bottom of the stack, and
 * the first buffermark on the stack, the first buffer, is the lines
 * between buffermark 1 and buffer mark 2.

 * When creating a buffer, the value returned will be the number of
 * buffermarks in the stack after the new buffermark is created.
 * When destroying buffers, the parameter given will equal the number
 * of the lowest buffermark to be destroyed (i.e dropbuf 4) will leave
 * 3 buffermarks on the stack).

 * A special variable buf is now part of a stack element. It points to
 * the last buffer' begin or is NULL for buffer 0.
 * In short:
 *
 * Overview of internal or temporary queues. Even temporary queues may
 * contain buffer marks if the content was moved from an internal queue to
 * a temporary queue.
 * The firstbox is equivalent to firstline and lastbox to lastline.
 * The ends of the double linked list are NULL values. p is short for prev,
 * n for next. b is short for buf. buf is either NULL or points to the element
 * which is the next buffer mark.
 * A buffer mark is a StackLine (SL) with a contents-field of NULL.
 *
 *    -->---------->---------->---------->---------->-+
 *    ^          ^          ^          ^          ^   |    NULL       NULL
 *    b          b          b          b          b  /       b          b
 *    |          |          |          |          | /        |          |
 *  p-SL-n <-> p-SL-n <-> p-SL-n <-> p-SL-n <-> p-SL-n <-> p-SL-n <-> p-SL-n
 *    ||                                          ||                    ||
 * firstbox                                     Buffer                lastbox
 *                                               Mark
 *
 *  READER'S END                                                  FIFO'S END
 *  LIFO'S END
 *

 * Possible extentions:
 *  o A new 'hard-buffer', that is not removed when a line is popped
 *    from it while it is empty
 */

#include "rexx.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_PROCESS_H
# include <process.h>
#endif

#include "extstack.h"

#define NUMBER_QUEUES 100
/*
 * Queue 0 is SESSION and cannot be added/deleted
 */

#define ENSURE_BUFFER(iq) {                                               \
   /* Ensures that at least one buffer exists for an internal queue.      \
    */                                                                    \
   if ( (iq)->u.i.top == NULL )                                           \
   {                                                                      \
      (iq)->u.i.top = (iq)->u.i.bottom = (Buffer *)MallocTSD( sizeof( Buffer ) ) ;  \
      memset( (iq)->u.i.top, 0, sizeof( Buffer ) ) ;                      \
      assert( (iq)->u.i.elements == 0 ) ;                                 \
      assert( (iq)->u.i.buffers == 0 ) ;                                  \
      (iq)->u.i.elements = 0 ;                                            \
      (iq)->u.i.buffers = 1 ;                                             \
   }                                                                      \
}

typedef struct { /* stk_tsd: static variables of this module (thread-safe) */
#if !defined(NO_EXTERNAL_QUEUES)
   int initialized ;
#endif
   /*
    * The pointer of the current queue points to one of the elements of queue.
    */
   Queue *current_queue;
   Queue queue[ NUMBER_QUEUES ] ;
   /*
    * The current queue's name is buffered for a faster access.
    */
   streng *current_queue_name;
   /*
    * runner is needed to create fresh ids.
    */
   int runner ;
} stk_tsd_t; /* thread-specific but only needed by this module. see
              * init_stacks
              */

#if !defined(NO_EXTERNAL_QUEUES)
static int get_socket_details_and_connect( tsd_t *TSD, Queue *target );
static int save_parse_queue( tsd_t *TSD, streng *queue, Queue *q,
                                                              int ignore_name);
static Queue *open_external( tsd_t *TSD, const streng *queue, Queue *q,
                                 int *rc, int ignore_name, streng **basename );
#endif

/* init_stacks initializes the module.
 * Currently, we set up the thread specific data.
 * The function returns 1 on success, 0 if memory is short.
 */
int init_stacks( tsd_t *TSD )
{
   stk_tsd_t *st;

   if (TSD->stk_tsd != NULL)
      return(1);

   if ( ( TSD->stk_tsd = MallocTSD( sizeof(stk_tsd_t) ) ) == NULL )
      return(0);
   st = (stk_tsd_t *)TSD->stk_tsd;
   memset( st, 0, sizeof(stk_tsd_t) );  /* correct for all values */
   /*
    * Create the default internal SESSION queue on demand. purge_stacks()
    * deletes the queuename very often.
    */
   st->queue[0].type = QisSESSION ;
   st->queue[0].u.i.isReal = 1 ;
   st->current_queue = &st->queue[0] ;
   return(1);
}

/* delete_buffer_content deletes the content of a buffer and resets the
 * buffer to an empty buffer.
 */
static void delete_buffer_content( const tsd_t *TSD, stk_tsd_t *st, Buffer *b )
{
   StackLine *p, *h ;

   /* prepare the deletion and become reentrant in case of exceptions */
   p = b->top ;
   b->top = b->bottom = NULL ;
   b->elements = 0 ;

   while ( p != NULL ) {
      h = p ;
      p = p->lower ;
      if ( h->contents != NULL )
         Free_stringTSD( h->contents ) ;
      FreeTSD( h ) ;
   }
}

/* delete_an_internal_queue deletes the content of an internal queue (including
 * SESSION) resets the queue to "not used" or "empty" in case of "SESSION".
 */
static void delete_an_internal_queue( const tsd_t *TSD, stk_tsd_t *st, Queue *q )
{
   Buffer *p, *h ;
   streng *n ;
   int type = q->type ;

   if ( type == QisUnused )
      return ;
   assert( ( type == QisInternal ) || ( type == QisSESSION ) ) ;

   /* prepare the deletion and become reentrant in case of exceptions */
   p = q->u.i.top ;
   n = q->u.i.name ;
   memset( q, 0, sizeof( Queue ) ) ;
   q->type = ( type == QisSESSION ) ? QisSESSION : QisUnused ;

   /* delete the name and all content buffers */
   if ( n != NULL )
      Free_stringTSD( n ) ;
   while ( p != NULL ) {
      h = p ;
      p = p->lower ;
      delete_buffer_content( TSD, st, h ) ;
      FreeTSD( h ) ;
   }
}

/* delete_a_temp_queue cleans up one of the internal stacks by freeing its
 * content. The queue is reset to "not used".
 */
static void delete_a_temp_queue( const tsd_t *TSD, stk_tsd_t *st, Queue *q )
{
   Buffer b ;

   if ( q->type == QisUnused ) /* happens very often */
      return ;
   assert( q->type == QisTemp ) ;

   /* prepare the deletion and become reentrant in case of exceptions */
   b = q->u.t ;
   memset( q, 0, sizeof( Queue ) ) ;
   q->type = QisUnused ;

   delete_buffer_content( TSD, st, &b ) ;
}

#if !defined(NO_EXTERNAL_QUEUES)
/* delete_an_external_queue cleans up one of the external stacks by
 * disconnecting from the server.
 */
static void delete_an_external_queue( const tsd_t *TSD, stk_tsd_t *st, Queue *q )
{
   Queue b ;

   if ( q->type == QisUnused )
      return ;
   assert( q->type == QisExternal ) ;

   /* prepare the deletion and become reentrant in case of exceptions */
   b = *q ;
   memset( q, 0, sizeof( Queue ) ) ;
   q->type = QisUnused ;

   disconnect_from_rxstack( TSD, &b ) ;
}
#endif

/* delete_a_queue cleans up any queue by purging or disconnecting. */
static void delete_a_queue( const tsd_t *TSD, stk_tsd_t *st, Queue *q )
{
   switch ( q->type )
   {
      case QisTemp:
         delete_a_temp_queue( TSD, st, q ) ;
         break;

      case QisInternal:
      case QisSESSION:
         delete_an_internal_queue( TSD, st, q ) ;
         break;

#if !defined(NO_EXTERNAL_QUEUES)
      case QisExternal:
         delete_an_external_queue( TSD, st, q ) ;
         break;
#endif

      default:
         assert( q->type == QisUnused ) ;
   }
}

/*
 * This routine clears all the internal queues we have made
 * and if we have a connection to rxstack, disconnects from it
 */
void purge_stacks( const tsd_t *TSD )
{
   stk_tsd_t *st;
   int i;

   st = (stk_tsd_t *)TSD->stk_tsd;
   for ( i = 0; i < NUMBER_QUEUES; i++ )
      delete_a_queue( TSD, st, &st->queue[i] );
   st->current_queue = &st->queue[0];
   if ( st->current_queue_name != NULL )
      Free_stringTSD( st->current_queue_name );
   st->current_queue_name = NULL;
}

static void SetSessionName( const tsd_t *TSD, stk_tsd_t *st )
{
   Queue *q = &st->queue[0];

   assert( q->type == QisSESSION ) ;
   if ( q->u.i.name == NULL )
   {
      q->u.i.name = Str_creTSD( "SESSION" ) ;
      q->u.i.isReal = 1 ;
      st->current_queue_name = Str_dupTSD( q->u.i.name );
   }
}

/*
 * Find the named internal queue - case insensitive
 */
static Queue *find_queue( const tsd_t *TSD, stk_tsd_t *st, const streng *queue_name )
{
   int i;

   if ( st->queue[0].u.i.name == NULL )
      SetSessionName( TSD, st ) ;
   for ( i = 0; i < NUMBER_QUEUES; i++ ) /* inefficient !! */
   {
      if ( ( st->queue[i].type != QisInternal )
      &&   ( st->queue[i].type != QisSESSION ) )
         continue ;

      if ( Str_ccmp( st->queue[i].u.i.name, queue_name ) == 0 )
         return &st->queue[i] ;
   }
   return NULL ;
}

/* This function creates a new queue.
 * The returned queue's type must be overwritten.
 */
Queue *find_free_slot( const tsd_t *TSD )
{
   int i;
   stk_tsd_t *st = (stk_tsd_t *)TSD->stk_tsd ;

   for ( i = 1; i < NUMBER_QUEUES; i++ ) /* never ever select SESSION */
   {
      if ( st->queue[i].type == QisUnused )
         return &st->queue[i] ;
   }
   if ( !TSD->called_from_saa )
      exiterror( ERR_STORAGE_EXHAUSTED, 0 ); /* message not very helpful */
   return NULL ;
}

/*
 * Glues the content of the temporary stack src to the top buffer of
 * dst which must not be temporary. is_fifo controls the position of
 * the glue. is_fifo should reflect the type of insertion in the
 * temporary buffer.
 * src is destroyed at the end of the operation.
 */
void flush_stack( const tsd_t *TSD, Queue *src, Queue *dst, int is_fifo )
{
#if !defined( NO_EXTERNAL_QUEUES )
   StackLine *ptr, *h ;
#endif
   stk_tsd_t *st;

   if ( src == NULL ) /* no temporary stack? may happen */
      return ;
   st = (stk_tsd_t *)TSD->stk_tsd;
   /*
    * I think this is never used. Let's verify it. Florian, 06.01.2005
    */
   fprintf( stderr, "Regina internal error detected in %s, line %u.\n"
                    "Please, send an email to M.Hessling@qut.edu.au.\n",
                    __FILE__, __LINE__ );

   assert( src->type == QisTemp ) ;
   assert( dst->type == QisSESSION
        || dst->type == QisInternal
        || dst->type == QisExternal ) ;
   if ( src->u.t.top == NULL )
   {
      /* nothing in temporary stack to flush */
      assert( src->u.t.bottom == NULL ) ;
      return ;
   }

#if !defined(NO_EXTERNAL_QUEUES)
   if ( dst->type == QisExternal )
   {
      for ( ptr = src->u.t.top; ptr != NULL; )
      {
         if (is_fifo)
         {
            queue_line_fifo_to_rxstack( TSD, dst->u.e.socket, ptr->contents ) ;
         }
         else
         {
            queue_line_lifo_to_rxstack( TSD, dst->u.e.socket, ptr->contents ) ;
         }

         h = ptr ;
         ptr = ptr->lower ;
         Free_stringTSD( h->contents ) ;
         FreeTSD( h ) ;
      }
      src->u.t.top = src->u.t.bottom = NULL ; /* allow safe cleanup */
      delete_a_temp_queue( TSD, st, src ) ;
      return ;
   }
#endif

   /* dst->type is QisSESSION or QisInternal, stack it either fifo or lifo */
   ENSURE_BUFFER( dst ) ;

   dst->u.i.elements += src->u.t.elements ;
   if ( is_fifo )
   {
      GLUE_BUFFER2( &src->u.t, dst->u.i.top ); /* begin of newest buffer */
   }
   else
   {
      GLUE_BUFFER1( dst->u.i.top, &src->u.t );
   }
   delete_a_temp_queue( TSD, st, src ) ;
}

/*
 * stack_to_line converts the content of the temporary stack to one blank
 * delimited line.
 * An empty string is returned if nothing is in the queue.
 */
streng *stack_to_line( const tsd_t *TSD, Queue *q )
{
   StackLine *ptr, *h ;
   unsigned size = 0;
   char *dest;
   stk_tsd_t *st;
   streng *retval;

   st = (stk_tsd_t *)TSD->stk_tsd;

   if (q->type == QisUnused )
      return(nullstringptr());

   assert( q->type == QisTemp ) ;

   if ( q->u.t.top == NULL )
   {
      /* nothing in temporary stack to flush */
      assert( q->u.t.bottom == NULL ) ;
      delete_a_temp_queue( TSD, st, q ) ;
      return(nullstringptr());
   }

   /* first, count the needed string length. */
   for ( ptr = q->u.t.top; ptr != NULL; ptr=ptr->lower )
   {
      size += Str_len(ptr->contents) + 1 ; /* blank is delimiter */
   }

   retval = Str_makeTSD( size ) ;
   dest = retval->value ;

   for ( ptr = q->u.t.top; ptr != NULL; )
   {
      memcpy( dest, ptr->contents->value, Str_len( ptr->contents ) ) ;
      dest += Str_len( ptr->contents ) ;
      *dest++ = ' ';

      h = ptr ;
      ptr = ptr->lower ;
      Free_stringTSD( h->contents ) ;
      FreeTSD( h ) ;
   }

   /* strip any blanks at the end of the string. */
   while ((dest != retval->value) && (dest[-1] == ' '))
      dest--;

   /* finally, terminate the string */
   *dest = '\0';                          /* DON'T increment dest */
   Str_len( retval ) = (int)( dest - retval->value ) ;

   /* reset the pointers, to signal that temporary stack is empty */
   q->u.t.top = q->u.t.bottom = NULL ;
   delete_a_temp_queue( TSD, st, q ) ;
   return(retval);
}

/*
 * The input for a command may be overwritten if both input and output or
 * input and error are STEM values. A temporary queue is created in this
 * case to prevent the destruction of an unread value.
 * fill_input_queue_stem fills up a new queue with a copy of a stems
 * content. stemname must end with a period and stem0 is the count of lines
 * in stem.
 * OPTIMIZING HINT: The function can be rewritten to access just only the
 * stem leaves.
 */
Queue *fill_input_queue_stem(tsd_t *TSD, streng *stemname, int stem0)
{
   int i,stemlen = Str_len( stemname ) ;
   streng *stem, *newstr ;
   StackLine *line ;
   Queue *q = find_free_slot( TSD ) ;

   q->type = QisTemp ;

   stem = Str_makeTSD(stemlen + 1 + 3*sizeof(int));
   memcpy(stem->value, stemname->value, stemlen);

   for (i = 1; i <= stem0; i++)
   {                                    /* Fetch the value:                  */
      stem->len = stemlen + sprintf(stem->value + stemlen, "%d", i);
      newstr = Str_dupTSD(get_it_anyway_compound(TSD, stem));

      line = (StackLine *) MallocTSD( sizeof( StackLine ) ) ;
      line->contents = newstr ;
      FIFO_LINE( &q->u.t, line ) ;
   }

   Free_stringTSD(stem);
   return q ;
}

/*
 * The input for a command may be overwritten if both input and output or
 * input and error are STREAM values. A temporary queue is created in this
 * case to prevent the destruction of an unread value.
 * fill_input_queue_stream fills up a new queue with a copy of a streams
 * content. fileptr must be an opened file stream pointer gotten by
 * addr_reopen_file() for an input stream.
 */
Queue *fill_input_queue_stream( tsd_t *TSD, void *fileptr )
{
   StackLine *line;
   streng *c;
   Queue *q = find_free_slot( TSD );

   q->type = QisTemp;

   for ( ; ; )
   {                                    /* Fetch the value:                  */
      c = addr_io_file( TSD, fileptr, NULL );
      if ( c == NULL )
         break;
      if ( c->len == 0 )
      {
         Free_stringTSD( c );
         break;
      }

      line = (StackLine *) MallocTSD( sizeof( StackLine ) );
      line->contents = c;
      FIFO_LINE( &q->u.t, line );
   }

   return q;
}

/*
 * use_external checks whether or not an external queue must be processed.
 * The different reasons why not to do it are checked.
 * use_external returns either 0, if queue_name is a local queue name or
 * the current queue is a local queue, or it returns 1 for an external queue
 * name or if the current queue is external.
 */
static int use_external( const tsd_t *TSD, const streng *queue_name )
{
   stk_tsd_t *st = (stk_tsd_t *)TSD->stk_tsd;

   /* A little bit off topic, but purge_stacks() may kill "SESSION" at the
    * end of RexxStart() and we need a working "SESSION" always. Not deleting
    * it will produce "errors" with the tracemem feature.
    * This function is called at top of each relevant function.
    */
   if ( st->queue[0].u.i.name == NULL )
      SetSessionName( TSD, st );

#if defined(NO_EXTERNAL_QUEUES)
   (queue_name);
   return 0;        /* trivial */
#else

   if ( !st->initialized )
   {
      /*
       * Attention, set the current queue type to default. This may be
       * either internal or external.
       */
      st->initialized = 1;
      init_external_queue( TSD );
   }

   if ( get_options_flag( TSD->currlevel, EXT_INTERNAL_QUEUES ) )
      return 0;     /* user forces a local queue in every case */
   if ( ( queue_name == NULL ) || ( PSTRENGLEN( queue_name ) == 0 ) )
      return st->current_queue->type == QisExternal;

   if ( get_options_flag( TSD->currlevel, EXT_QUEUES_301 ) == 0 )
   {
      /*
       * A name exists, check it.
       */
      if  ( memchr( queue_name->value, '@', Str_len( queue_name ) ) == NULL )
         return 0;
   }

   return 1;
#endif
}

/*
 * Returns 1 if we have started using external queues, 0 if we haven't
 * started using external queues
 */
int external_queues_used( const tsd_t *TSD )
{
   return use_external( TSD, NULL ) ;
}

/*
 * Pushes 'line' onto the REXX stack, lifo, and sets 'lastline' to
 *    point to the new line. The line is put on top of the current
 *    buffer.
 *
 * A queue_name is supplied by external reference, only. See IfcAddQueue.
 * Reuses the current connection if possible.
 */
int stack_lifo( tsd_t *TSD, streng *line, const streng *queue_name )
{
   StackLine *newbox ;
   Buffer *b ;
   stk_tsd_t *st ;
   Queue *q ;
   int rc=0;

   st = (stk_tsd_t *)TSD->stk_tsd;

   assert( line != NULL ) ;

   if ( !use_external( TSD, queue_name ) )
   {
      if ( queue_name )
      {
         if ( ( q = find_queue( TSD, st, queue_name ) ) == NULL )
         {
            return 9;
         }
      }
      else
         q = st->current_queue;
      assert( ( q->type == QisSESSION ) || ( q->type == QisInternal ) ) ;

      newbox = (StackLine *) MallocTSD( sizeof( StackLine ) ) ;
      newbox->contents = line ;
      ENSURE_BUFFER( q ) ;
      b = q->u.i.top ;
      LIFO_LINE( b, newbox ) ;
      q->u.i.elements++;
   }
#if !defined(NO_EXTERNAL_QUEUES)
   else
   {
      Queue q1,*work;

      if ( ( work = open_external( TSD, queue_name, &q1, &rc, 0, NULL ) ) == NULL )
         return rc;

      if ( ( rc = queue_line_lifo_to_rxstack( TSD, work->u.e.socket, line ) ) == -1 )
         rc = 100;
      disconnect_from_rxstack( TSD, &q1 ) ;
   }
#endif
   return rc;
}


/*
 * Puts 'line' on the REXX stack, fifo. This routine is similar to
 *    stack_lifo but the differences are big enough to have a separate
 *    routine. The line is put in the bottom of the current buffer,
 *    that is just above the uppermost buffer mark, or at the bottom
 *    of the stack, if there are no buffer marks.
 *
 * A queue_name is supplied by external reference, only. See IfcAddQueue.
 * Reuses the current connection if possible.
 */
int stack_fifo( tsd_t *TSD, streng *line, const streng *queue_name )
{
   StackLine *newbox ;
   stk_tsd_t *st;
   Queue *q;
   Buffer *b ;
   int rc=0;

   st = (stk_tsd_t *)TSD->stk_tsd;

   assert( line != NULL ) ;

   if ( !use_external( TSD, queue_name ) )
   {
      if ( queue_name )
      {
         if ( ( q = find_queue( TSD, st, queue_name ) ) == NULL )
         {
            return 9;
         }
      }
      else
         q = st->current_queue;
      assert( ( q->type == QisSESSION ) || ( q->type == QisInternal ) ) ;

      newbox = (StackLine *) MallocTSD( sizeof( StackLine ) ) ;
      newbox->contents = line ;
      ENSURE_BUFFER( q ) ;
      b = q->u.i.top ;
      FIFO_LINE( b, newbox ) ;
      q->u.i.elements++;
   }
#if !defined(NO_EXTERNAL_QUEUES)
   else
   {
      Queue q1, *work;

      if ( ( work = open_external( TSD, queue_name, &q1, &rc, 0, NULL ) ) == NULL )
         return rc;

      if ( ( rc = queue_line_fifo_to_rxstack( TSD, work->u.e.socket, line ) ) == -1 )
         rc = 100;
      disconnect_from_rxstack( TSD, &q1 ) ;
   }
#endif
   return rc;
}


/*
 * Removes one (or several) buffers from the stack. Number must be a
 *    non-negative number giving the number of first buffer from the bottom
 *    which has to be removed. You can pass a number of buffers to destroy
 *    from top of the stack if you supply the negative number of buffers.
 *
 * Example: Queue has buffers 0, 1, 2, 3, 4 and the number is 3. Then
 *    the buffers 3 and 4 are removed and 2 becomes the current buffer.
 *    If the number is -2 then the top two buffers are destroyed and
 *    2 becomes the current buffer.
 *
 * Return values: -2: specified buffer doesn't exist
 *                -1: external queues don't support buffers
 *            others: remaining buffers after success
 */

int drop_buffer( const tsd_t *TSD, int number )
{
   Buffer *b, *h ;
   stk_tsd_t *st;
   Queue *curr ;

   st = (stk_tsd_t *)TSD->stk_tsd;
   curr = st->current_queue ;
   if ( curr->type == QisExternal )
      exiterror( ERR_EXTERNAL_QUEUE, 110, "DROPBUF" ) ;
   assert( ( curr->type == QisInternal ) || ( curr->type == QisSESSION ) ) ;

   ENSURE_BUFFER( curr ) ; /* makes things easier */
   if ( number < 0 )
      number = curr->u.i.buffers + number ;

   if ( number >= (int) curr->u.i.buffers )
      return -2 ;

   for ( b = curr->u.i.bottom; ( number > 0 ) && ( b != NULL ); number-- )
      b = b->higher ;

   assert( b ) ; /* must exist */
   /* disconnect the buffer sequence marked for deletion */
   if ( b == curr->u.i.bottom )
   {
      /* preserve en empty buffer */
      curr->u.i.elements -= b->elements ;
      delete_buffer_content( TSD, st, b ) ;
      b = b->higher ;
      curr->u.i.top = curr->u.i.bottom ;
      curr->u.i.top->higher = NULL ;
   }
   else
   {
      curr->u.i.top = b->lower ;
      curr->u.i.top->higher = NULL ;
   }

   /* delete each buffer and its content */
   while ( b != NULL ) {
      curr->u.i.elements -= b->elements ;
      delete_buffer_content( TSD, st, b ) ;

      h = b ;
      b = b->higher ;
      FreeTSD( h ) ;
      curr->u.i.buffers-- ;
   }

   return curr->u.i.buffers - 1 ;
}


/*
 * Fetches a line from the top of the stack. If the current buffer
 *    does not contain any lines, it is removed and the second current
 *    buffer is search for a line etc. If there isn't any lines in the
 *    stack, a line is read from the standard input.
 * Reuses the current connection if possible.
 */
streng *popline( tsd_t *TSD, const streng *queue_name, int *result, unsigned long waitflag )
{
   streng *contents=NULL ;
   StackLine *line ;
   stk_tsd_t *st ;
   int need_line_from_stdin=0;
   int rc=0;
   Queue *q ;
   Buffer *b ;

   st = (stk_tsd_t *)TSD->stk_tsd;

   if ( !use_external( TSD, queue_name ) )
   {
      if ( queue_name )
      {
         if ( ( q = find_queue( TSD, st, queue_name ) ) == NULL )
         {
            if ( result )
               *result = 9;
            if ( !TSD->called_from_saa )
               contents = nullstringptr();
            return contents;
         }
      }
      else
         q = st->current_queue;
      assert( ( q->type == QisSESSION ) || ( q->type == QisInternal ) ) ;

      line = NULL ;
      while ( ( b = q->u.i.top ) != NULL )
      {
         POP_LINE( b, line ) ;
         if ( line != NULL )
         {
            q->u.i.elements-- ;
            break ;
         }

         /* buffer is empty, fetch the next one and drop the empty one
          */
         q->u.i.top = b->lower ;
         if ( q->u.i.top )
            q->u.i.top->higher = NULL ; /* fixes bug 1068204 */
         q->u.i.buffers-- ;
         FreeTSD( b ) ;
      }
      if ( line != NULL )
      {
         contents = line->contents ;
         FreeTSD( line ) ;
      }
      else
      {
         /* q->u.i.top == NULL, too; but bottom's not set */
         q->u.i.bottom = NULL ;
         assert( q->u.i.elements == 0 ) ;
         assert( q->u.i.buffers == 0 ) ;
         need_line_from_stdin = 1;
      }
   }
#if !defined(NO_EXTERNAL_QUEUES)
   else
   {
      Queue q1, *work;

      if ( ( work = open_external( TSD, queue_name, &q1, &rc, 0, NULL ) ) == NULL )
      {
         if ( result )
            *result = rc;
         return NULL;
      }
      /*
       * waitflag is 0 if called from a Rexx program as [PARSE] PULL, but can be
       * 1 when called from the Rexx API. We override the waitflag when the current
       * queue is external and a timeout has been set for the queue
       */
      if ( work->u.e.timeoutSet )
         waitflag = 1;
      rc = get_line_from_rxstack( TSD, work->u.e.socket, &contents, ( waitflag == 0 ) );
      switch ( rc )
      {
         case -1:                rc = 100;  break;
         case RXSTACK_ERROR:     rc = 9;   break;  /* map generic error to RXQUEUE_NOTREG */
         default:    ;
      }
      disconnect_from_rxstack( TSD, &q1 ) ;
      if ( rc == RXSTACK_TIMEOUT )
      {
         condition_hook( TSD, SIGNAL_NOTREADY, 94, 1, -1, NULL, Str_cre_TSD( TSD, "Timeout on external queue" ) );
      }
      if ( ( rc == RXSTACK_EMPTY ) || ( rc == RXSTACK_TIMEOUT ) ) /* empty or timeout */
         need_line_from_stdin = 1;
   }
#endif

   if ( need_line_from_stdin )
   {
      if ( TSD->called_from_saa )
      {
         rc = 8; /* RXQUEUE_EMPTY */
      }
      else if ( rc == RXSTACK_TIMEOUT )
      {
         rc = 8; /* RXQUEUE_EMPTY */
      }
      else
      {
         int rc2 = HOOK_GO_ON ;
         if ( TSD->systeminfo->hooks & HOOK_MASK( HOOK_PULL ) )
            rc2 = hookup_input( TSD, HOOK_PULL, &contents );

         if ( rc2 == HOOK_GO_ON )
            contents = readkbdline( TSD );

         assert( contents );
      }
      rc = 0;
   }
   if ( result )
      *result = rc;
   else if ( ( contents == NULL ) && !TSD->called_from_saa )
      contents = nullstringptr();

   return contents;
}


/*
 * Counts the lines in the stack.
 * Reuses the current connection if possible.
 */
int lines_in_stack( tsd_t *TSD, const streng *queue_name )
{
   stk_tsd_t *st ;
   int lines ;
   Queue *q ;

   st = (stk_tsd_t *)TSD->stk_tsd;
   if ( !use_external( TSD, queue_name ) )
   {
      if ( queue_name )
      {
         if ( ( q = find_queue( TSD, st, queue_name ) ) == NULL )
         {
            return -9;
         }
      }
      else
         q = st->current_queue;
      assert( ( q->type == QisSESSION ) || ( q->type == QisInternal ) ) ;

      lines = q->u.i.elements ;
   }
#if !defined(NO_EXTERNAL_QUEUES)
   else
   {
      Queue q1, *work;
      int rc;

      if ( ( work = open_external( TSD, queue_name, &q1, &rc, 0, NULL ) ) == NULL )
         return -rc;

      lines = get_number_in_queue_from_rxstack( TSD, work->u.e.socket, &rc );
      disconnect_from_rxstack( TSD, &q1 ) ;
      if ( rc != 0 )
         lines = -rc;
   }
#endif
   return lines ;
}

#ifdef TRACEMEM
/*
 * Marks all chunks of dynamic allocated memory that are allocated
 *   to the stack subsystem.
 */
static void mark_buffer( const Buffer *b )
{
   StackLine *ptr ;

   for ( ptr = b->top; ptr != NULL; ptr = ptr->lower )
   {
      markmemory( ptr, TRC_STACKBOX ) ;
      markmemory( ptr->contents, TRC_STACKLINE ) ;
   }
}

void mark_stack( const tsd_t *TSD )
{
   Buffer *b ;
   Queue *q ;
   stk_tsd_t *st;
   int i;

   st = (stk_tsd_t *)TSD->stk_tsd;
   q = st->current_queue ;
   for ( i = 0; i < NUMBER_QUEUES; i++ )
   {
      switch ( st->queue[i].type )
      {
         default:
            assert( st->queue[i].type ) ;
         case QisUnused:
            break;

         case QisSESSION:
         case QisInternal:
            if ( st->queue[i].u.i.name )
            {
               markmemory( st->queue[i].u.i.name, TRC_STACKBOX ) ;
            }

            for ( b = st->queue[i].u.i.top; b != NULL; b = b->lower )
            {
               markmemory( b, TRC_STACKBOX ) ;
               mark_buffer( b ) ;
            }
            break ;

         case QisTemp:
            mark_buffer( &st->queue[i].u.t ) ;
            break ;

         case QisExternal:
            markmemory( st->queue[i].u.e.name, TRC_STACKBOX ) ;
            break ;
      }
   }
}
#endif


/*
 * Creates a new buffer and returns the number of buffers in the stack
 * excluding the zeroth buffer.
 */
int make_buffer( tsd_t *TSD )
{
   stk_tsd_t *st;
   Buffer *b ;
   Queue *q ;

   st = (stk_tsd_t *)TSD->stk_tsd;
   q = st->current_queue ;
   if ( q->type == QisExternal )
      exiterror( ERR_EXTERNAL_QUEUE, 110, "MAKEBUF" ) ;
   assert( ( q->type == QisSESSION )
        || ( q->type == QisInternal ) ) ;

   ENSURE_BUFFER( q ) ; /* Possibly create the zeroth buffer */

   /* Make a *new* buffer */
   b = (Buffer *)MallocTSD( sizeof( Buffer ) ) ;
   memset( b, 0, sizeof( Buffer ) ) ;

   b->lower = q->u.i.top ; /* The zeroth buffer exists */
   b->lower->higher = b ;
   q->u.i.top = b ;
   q->u.i.buffers++ ;

   return q->u.i.buffers - 1 ;
}


/*
 * Dumps the contents of the stack to standard error. Buffer marks are
 *    indicated in the printout.
 */
void type_buffer( tsd_t *TSD )
{
   Buffer *b ;
   StackLine *ptr ;
   char *cptr, *stop ;
   int counter ;
   streng *name ;
   stk_tsd_t *st;
   Queue *q ;

   if (TSD->stddump == NULL)
      return;
   st = (stk_tsd_t *)TSD->stk_tsd;
   q = st->current_queue ;
   name = get_queue( TSD ) ;
   fprintf(TSD->stddump,"==> Name: %.*s\n", Str_len(name), name->value ) ;
   fprintf(TSD->stddump,"==> Lines: %d\n", lines_in_stack( TSD, NULL )) ;
   if ( q->type == QisExternal )
      return ;

   assert( ( q->type == QisSESSION ) || ( q->type == QisInternal ) ) ;
   ENSURE_BUFFER( q ) ; /* Possibly create the zeroth buffer */
   for ( counter = q->u.i.buffers, b = q->u.i.top; b != NULL; b = b->lower )
   {
      fprintf(TSD->stddump,"==> Buffer: %d\n", --counter ) ;
      for ( ptr = b->top; ptr != NULL; ptr = ptr->lower )
      {
         putc( '"', TSD->stddump ) ;
         stop = Str_end( ptr->contents ) ;
         for ( cptr = ptr->contents->value ; cptr < stop; cptr++ )
            putc( ( rx_isprint( *cptr ) ? ( *cptr ) : '?' ), TSD->stddump ) ;

         putc( '"', TSD->stddump ) ;
#if defined(DOS) || defined(OS2) || defined(WIN32)
         putc( REGINA_CR, TSD->stddump ) ;
#endif
         putc( REGINA_EOL, TSD->stddump ) ;
      }
   }

   fprintf(TSD->stddump,"==> End of Stack\n") ;
   fflush(TSD->stddump) ;
}

#if !defined(NO_EXTERNAL_QUEUES)
/*
 * The following functions allow interfacing to the external
 * queue process; rxstack
 * The connection to eq is opened if and only if it isn't opened yet.
 */
static int get_socket_details_and_connect( tsd_t *TSD, Queue *q )
{
   assert( q->type == QisExternal ) ;

   /* Fill in default values. Missing values are rarely used but it may
    * happen.
    */
   if ( q->u.e.name == NULL )
      q->u.e.name = default_external_name( TSD ) ;
   if ( q->u.e.portno == 0 )
      q->u.e.portno = default_port_number() ;
   if ( q->u.e.address == 0 )
      q->u.e.address = default_external_address() ;

   if ( q->u.e.socket == -1 )
   {
      if ( connect_to_rxstack( TSD, q ) == -1 )
         return 100; /* RXQUEUE_NETERROR */
   }
   q->u.e.timeoutSet = 0;
   return 0;
}

/*
 * save_parse_queue wraps parse_queue and exiterrors in case of an error.
 * Returns 1 if queue is (probably) different from the current queue, 0 else.
 * *q is filled even with a return value of 0.
 * A negative return code indicates a RXQUEUE_??? error code which must be
 * passed back to the SAA interface immediately.
 * Queues are treated as equal if ignore_name is set but host address and port
 * numbers are the same.
 */
static int save_parse_queue( tsd_t *TSD, streng *queue, Queue *q,
                                                               int ignore_name)
{
   stk_tsd_t *st = (stk_tsd_t *)TSD->stk_tsd;
   Queue *curr ;
   int rc;

   if ( ( rc = parse_queue( TSD, queue, q ) ) <= 0 )
      return rc;

   if ( !ignore_name && ( PSTRENGLEN( queue ) != 0 ) )
      return 1 ;
   curr = st->current_queue ;
   if ( curr->type != QisExternal )
      return 1 ;
   if ( q->u.e.address != curr->u.e.address )
      return 1 ;
   if ( q->u.e.portno != curr->u.e.portno )
      return 1 ;
   return 0 ;
}

/*
 * open_external opens the connection to the external queue will a full
 * error checking. NULL is returned on error and *rc describes the
 * RXQUEUE_??? error.
 * On success the opened queue is returned and *q is set to the old or
 * temporary queue. queue is either NULL or describes the new temporary
 * queue's name.
 * Queues are treated as equal if ignore_name is set but host address and port
 * numbers are the same. No set_queue_in_rxstack() is called if ignore_name
 * is set.
 * *basename is set to the queue's name without the network extension if
 * basename != NULL. It must be freed later. *basename may become NULL.
 */
static Queue *open_external( tsd_t *TSD, const streng *queue, Queue *q,
                                  int *rc, int ignore_name, streng **basename )
{
   stk_tsd_t *st = (stk_tsd_t *)TSD->stk_tsd;
   streng *qn;
   int h;
   Queue *retval;

   qn = ( queue == NULL ) ? NULL : Str_dupTSD( queue );

   if ( ( h = save_parse_queue( TSD, qn, q, ignore_name ) ) < 0 )
   {
      if ( qn != NULL )
         Free_stringTSD( qn );
      *rc = -h;
      return NULL;
   }
   if ( h == 1 )
   {
      if ( ( h = get_socket_details_and_connect( TSD, q ) ) != 0 )
      {
         if ( qn != NULL )
            Free_stringTSD( qn );
         disconnect_from_rxstack( TSD, q );
         *rc = h;
         return NULL;
      }
      if ( !ignore_name )
      {
         if ( ( h = set_queue_in_rxstack( TSD, q->u.e.socket, qn ) ) != 0 )
         {
            if ( qn != NULL )
               Free_stringTSD( qn );
            disconnect_from_rxstack( TSD, q );
            *rc = h;
            return NULL;
         }
      }
      retval = q ;
   }
   else
   {
      retval = st->current_queue ;
   }

   if ( basename != NULL )
      *basename = qn;
   else if ( qn != NULL )
      Free_stringTSD( qn );

   *rc = 0;
   return retval;
}
#endif

/* SetCurrentQueue set the current queue and makes all the necessary cleanup
 * of the "old" current queue.
 * new_name is assigned to the current queue's name, the former value is
 * returned.
 */
static streng *SetCurrentQueue( const tsd_t *TSD, stk_tsd_t *st, Queue *q,
                                                             streng *new_name )
{
   streng *retval;

#if !defined( NO_EXTERNAL_QUEUES )
   if ( ( st->current_queue->type == QisExternal )
     && ( st->current_queue != q ) )
      delete_an_external_queue( TSD, st, st->current_queue );
#else
   (TSD);
#endif
   st->current_queue = q;

   retval = st->current_queue_name;
   st->current_queue_name = new_name;
   return retval;
}

/*
 * Create a new queue
 * RXQUEUE( 'Create' )
 * Reuses the current connection if possible. The newly created current
 * queue becomes the current queue.
 */
int create_queue( tsd_t *TSD, const streng *queue_name, streng **result )
{
   streng *new_queue = NULL ;
   stk_tsd_t *st;
   char buf[50];
   Queue *q = NULL ;
   int rc = 0 ;

   st = (stk_tsd_t *)TSD->stk_tsd;
   if ( !use_external( TSD, queue_name ) )
   {
      if ( queue_name == NULL )
      {
         /*
          * Create a unique queue name
          */
         sprintf(buf,"S%d-%ld-%d", getpid(), clock(), st->runner++ );
         new_queue = Str_cre_TSD( TSD, buf ) ;
      }
      else
      {
         if ( ( q = find_queue( TSD, st, queue_name ) ) == NULL )
         {
            /*
             * No queue of that name, so use it.
             */
            new_queue = Str_dupTSD( queue_name ) ;
         }
         else
         {
            if ( q->type == QisSESSION )
            {
               if ( !TSD->called_from_saa )
                  exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, rc, "Getting queue from stack" );

               return 5 ;
            }
            /*
             * If the queue we found is a false queue, we can still
             * use the supplied name and the slot
             */
            if ( q->u.i.isReal )
            {
               /*
                * Create a unique queue name
                */
               sprintf(buf,"S%d-%ld-%d", getpid(), clock(), st->runner++ );
               new_queue = Str_cre_TSD( TSD, buf ) ;
               rc = 1;
            }
         }
      }

      if ( new_queue != NULL ) /* new name for a new slot */
      {
         q = find_free_slot( TSD );
         if ( q == NULL )
         {
            if ( new_queue != NULL )
               Free_stringTSD( new_queue );
            return 12; /* RXQUEUE_MEMFAIL */
         }
         q->type = QisInternal ;
         if ( new_queue == queue_name ) /* need a fresh one */
            new_queue = Str_dupTSD( new_queue ) ;
         q->u.i.name = Str_upper( new_queue ) ;
      }
      assert( q->type == QisInternal ) ;
      q->u.i.isReal = 1;
      /*
       * result created here; up to caller to free it
       */
      *result = Str_dupTSD( q->u.i.name );
   }
#if !defined(NO_EXTERNAL_QUEUES)
   else
   {
      Queue q1, *work;
      streng *base;

      if ( ( work = open_external( TSD, queue_name, &q1, &rc, 1, &base ) ) == NULL )
         return rc;

      if ( ( rc = create_queue_on_rxstack( TSD, work, base, result ) ) == -1 )
         rc = 100;

      if ( base != NULL )
         Free_stringTSD( base );
      disconnect_from_rxstack( TSD, &q1 ) ;
   }
#endif
   return rc;
}

/*
 * Delete a queue
 * RXQUEUE( 'Delete' )
 * Reuses the current connection if possible. SESSION becomes the current
 * queue always.
 */
int delete_queue( tsd_t *TSD, const streng *queue_name )
{
   int rc = 0;
   stk_tsd_t *st;
   Queue *q;
   streng *h;

   st = (stk_tsd_t *)TSD->stk_tsd;
   if ( !use_external( TSD, queue_name ) )
   {
      if ( ( queue_name == NULL ) || ( PSTRENGLEN( queue_name ) == 0 ) )
         return 9 ;
      if ( ( q = find_queue( TSD, st, queue_name ) ) == NULL )
         return 9 ;
      if ( q->type == QisSESSION )
         return 5 ; /* SESSION can't be deleted */
      assert( q->type == QisInternal ) ;
      /*
       * If we found a false queue, return 9
       */
      if ( !q->u.i.isReal )
         rc = 9;
      /*
       * Delete the contents of the queue
       * and mark it as gone.
       */
      delete_an_internal_queue( TSD, st, q );
      h = SetCurrentQueue( TSD, st, &st->queue[0], Str_creTSD( "SESSION" ) ) ;
      Free_stringTSD( h );
   }
#if !defined(NO_EXTERNAL_QUEUES)
   else
   {
      Queue q1, *work;
      streng *base;

      if ( ( work = open_external( TSD, queue_name, &q1, &rc, 1, &base ) ) == NULL )
         return rc;

      if ( ( base == NULL ) || ( PSTRENGLEN( base ) == 0 ) )
      {
         if ( base != NULL )
            Free_stringTSD( base );
         if ( TSD->called_from_saa )
            return 9;
         exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INVALID_QUEUE, tmpstr_of(TSD, queue_name ) ) ;
      }

      if ( ( rc = delete_queue_from_rxstack( TSD, work->u.e.socket, base ) ) == 0 )
      {
         /* This will disconnect the current external queue if needed */
         h = SetCurrentQueue( TSD, st, &st->queue[0], Str_creTSD( "SESSION" ) );
         Free_stringTSD( h );
      }

      if ( base != NULL )
         Free_stringTSD( base );
      disconnect_from_rxstack( TSD, &q1 ) ;

      if ( rc == -1 )
         rc = 100;
   }
#endif
   return rc;
}

/*
 * Set timeout for queue
 * RXQUEUE( 'Timeout' )
 * Do not reuse the current connection. A different external queue name
 * leads to an expensive NOP.
 * Timeout only valid for external queues.
 */
int timeout_queue( tsd_t *TSD, const streng *timeout, const streng *queue_name )
{
   stk_tsd_t *st;
   int rc = 0 ;

   st = (stk_tsd_t *)TSD->stk_tsd;
   if ( !use_external( TSD, queue_name ) )
   {
      exiterror( ERR_EXTERNAL_QUEUE, 111, "TIMEOUT" ) ;
   }
#if !defined(NO_EXTERNAL_QUEUES)
   else
   {
      Queue q, *work;
      int val, err;

      if ( ( work = open_external( TSD, queue_name, &q, &rc, 1, NULL ) ) == NULL )
         return rc;
      /*
       * Convert incoming streng to positive (or zero) integer
       */
      val = streng_to_int( TSD, timeout, &err ) ;
      if ( ( val < 0 ) || err )
      {
         disconnect_from_rxstack( TSD, &q ) ;
         exiterror( ERR_INCORRECT_CALL, 930, 999999999, tmpstr_of( TSD, timeout ) );
      }
      /*
       * Indicate that this queue has a timeout
       */
      work->u.e.timeoutSet = 1;
      rc = timeout_queue_on_rxstack( TSD, work->u.e.socket, val );
      disconnect_from_rxstack( TSD, &q ) ;
   }
#endif
   return rc ;
}

/*
 * Return the name of the current queue. The value is silently nul terminated.
 * RXQUEUE( 'Get' )
 * Returns queue name even if it is a false queue
 */
streng *get_queue( tsd_t *TSD )
{
   streng *result;
   stk_tsd_t *st;
   int l;
   char *p;

   st = (stk_tsd_t *)TSD->stk_tsd;
   if ( st->queue[0].u.i.name == NULL )
      SetSessionName( TSD, st ) ;

   assert( st->current_queue_name != NULL );

   l = Str_len( st->current_queue_name );
   result = Str_makeTSD( l + 1 );
   p = Str_val( result );
   memcpy( p, Str_val( st->current_queue_name ), l );
   p[l] = '\0';
   Str_len( result ) = l;

   return result;
}

/*
 * Return the name and length of the current queue in argument pointers. No
 * copy is done.
 */
void fill_queue_name( const tsd_t *TSD, int *len, char **name )
{
   stk_tsd_t *st;

   st = (stk_tsd_t *)TSD->stk_tsd;
   if ( st->queue[0].u.i.name == NULL )
      SetSessionName( TSD, st ) ;

   assert( st->current_queue_name != NULL );

   *len = Str_len( st->current_queue_name );
   *name = Str_val( st->current_queue_name );
}

/*
 * Set the current queue
 * RXQUEUE( 'Set' )
 * Object Rexx allows the queue to be set to an unknown queue
 * This is stupid; the user should be told its not valid, and
 * return an empty string.
 * But we validly set the false queue :-(
 * Reuses the current connection if possible.
 */
streng *set_queue( tsd_t *TSD, const streng *queue_name )
{
   stk_tsd_t *st ;
   Queue *q ;

   st = (stk_tsd_t *)TSD->stk_tsd;
   if ( !use_external( TSD, queue_name ) )
   {
      if ( ( q = find_queue( TSD, st, queue_name ) ) == NULL )
      {
         /*
          * We didn't find a real or a false queue, so create
          * a false queue
          */
         q = find_free_slot( TSD );
         q->type = QisInternal ;
         q->u.i.name = Str_upper( Str_dupTSD( queue_name ) ) ;
         q->u.i.isReal = 0 ; /* false queue */
      }
      assert( ( q->type == QisSESSION ) || ( q->type == QisInternal ) ) ;
      return SetCurrentQueue( TSD, st, q, Str_dupTSD( q->u.i.name ) );
   }
#if !defined(NO_EXTERNAL_QUEUES)
   else
   {
      Queue q1, *work;
      streng *base, *result;
      int rc;

      if ( ( work = open_external( TSD, queue_name, &q1, &rc, 1, &base ) ) == NULL )
         exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, rc, "Setting queue from stack" );

      if ( ( base == NULL ) || ( PSTRENGLEN( base ) == 0 ) )
      {
         if ( base != NULL )
            Free_stringTSD( base );
         disconnect_from_rxstack( TSD, &q1 ) ;
         assert( !TSD->called_from_saa );
         exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INVALID_QUEUE, tmpstr_of(TSD, queue_name ) ) ;
      }
      if ( ( rc = set_queue_in_rxstack( TSD, work->u.e.socket, base ) ) == 0 )
      {
         Free_stringTSD( base );
         if ( ( rc = get_queue_from_rxstack( TSD, work, &result ) ) != 0 )
         {
            disconnect_from_rxstack( TSD, &q1 );
            assert( !TSD->called_from_saa );
            exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, rc, "Getting queue back from stack" );
         }
         if ( work == &q1 )
         {
            work = find_free_slot( TSD );
            *work = q1;
         }
         return SetCurrentQueue( TSD, st, work, result );
      }
      Free_stringTSD( base );
      disconnect_from_rxstack( TSD, &q1 ) ;
      assert( !TSD->called_from_saa );
      exiterror( ERR_EXTERNAL_QUEUE, ERR_RXSTACK_INTERNAL, rc, "Setting queue from stack" );
   }
#endif
   return NULL;
}

Queue *addr_reopen_queue( tsd_t *TSD, const streng *queuename, char code )
/* This is the open routine for the ADDRESS WITH-redirection. queuename is
 * the name of the queue. code is either 'r' for "READ",
 * 'A' for "WRITE APPEND", 'R' for "WRITE REPLACE".
 * The queue named queuename will be (re)opened in all cases.
 * In case of REPLACE the queue's content must be purged later by
 * addr_purge_queue.
 * A structure for queues is returned and should be used for calls
 * to addr_io_queue.
 * The return value *may* be NULL in case of an error. A NOTREADY condition
 * may have been raised in this case.
 */
{
   stk_tsd_t *st ;
   Queue *q ;
   streng *name ;

   st = (stk_tsd_t *)TSD->stk_tsd ;

   if ( ( queuename == NULL ) || ( PSTRENGLEN( queuename ) == 0 ) )
      return st->current_queue ;

   if ( !use_external( TSD, queuename ) )
   {
      q = find_queue( TSD, st, queuename ) ;
      if ( ( code == 'r' ) || ( q != NULL ) )
      {
         if ( q != NULL )
            q->u.i.isReal = 1 ; /* not sure if this's right */
         return q ;
      }
      /* create a new queue */
      q = find_free_slot( TSD ) ;
      q->type = QisInternal ;
      name = Str_dupTSD( queuename ) ;
      q->u.i.name = Str_upper( name ) ;
      q->u.i.isReal = 1 ;
      return q ;
   }
#if !defined( NO_EXTERNAL_QUEUES )
   /* always use a fresh connection for external queues. There are too many
    * circumstances to keep in mind to check cleanly, whether or not to use
    * the current connection or if it exists already. The caller will check
    * for same names later.
    */
   q = find_free_slot( TSD ) ;

   name = Str_dupTSD( queuename ) ;
   if ( save_parse_queue( TSD, name, q, 1 ) < 0 )
   {
      Free_stringTSD( name );
      return NULL;
   }
   if ( get_socket_details_and_connect( TSD, q ) != 0 )
   {
      disconnect_from_rxstack( TSD, q ) ;
      return NULL;
   }
   set_queue_in_rxstack( TSD, q->u.e.socket, name );

   return q ;
#else
   return NULL ;
#endif
}

/* returns 1 if the two queue pointers point to the same physical queue,
 * 0 else.
 */
int addr_same_queue( const tsd_t *TSD, const Queue *q1, const Queue *q2 )
{
   int t1, t2, retval = 0 ;
#if !defined( NO_EXTERNAL_QUEUES )
   streng *n1, *n2 ;
#endif

   if ( q1 == q2 )
      return 1 ;
   if ( ( t1 = q1->type ) == QisSESSION )
      t1 = QisInternal ;
   if ( ( t2 = q2->type ) == QisSESSION )
      t2 = QisInternal ;
   if ( t1 != t2 )
      return 0 ;
   if (t1 == QisInternal)
      return !Str_cmp( q1->u.i.name, q2->u.i.name ) ;
   assert( q1->type == QisExternal ) ;
   if ( q1->u.e.address != q2->u.e.address )
      return 0 ;
   if ( q1->u.e.portno != q2->u.e.portno )
      return 0 ;
   /* we have to use the slow name comparison */
#if !defined( NO_EXTERNAL_QUEUES )
   if ( get_queue_from_rxstack( TSD, q1, &n1 ) != 0 )
      return 0;
   if ( get_queue_from_rxstack( TSD, q2, &n2 ) != 0 )
   {
      Free_stringTSD( n1 ) ;
      return 0;
   }
   retval = !Str_cmp( n1, n2 ) ;
   Free_stringTSD( n1 ) ;
   Free_stringTSD( n2 ) ;
#endif
   return retval ;
}

/* addr_redir_queue moves the content of the top buffer of an existing queue
 * to a new temporary queue.
 */
Queue *addr_redir_queue( const tsd_t *TSD, Queue *q )
{
   stk_tsd_t *st ;
   Queue *retval ;
   Buffer *b ;
#if !defined( NO_EXTERNAL_QUEUES )
   streng *contents ;
   StackLine *ptr ;
#endif

   st = (stk_tsd_t *)TSD->stk_tsd ;
   assert( ( q->type == QisSESSION )
        || ( q->type == QisInternal )
        || ( q->type == QisExternal ) ) ;
   retval = find_free_slot( TSD ) ;
   retval->type = QisTemp ;

   if ( ( q->type == QisSESSION ) || ( q->type == QisInternal ) )
   {
      /* trivial */
      if ( ( b = q->u.i.top ) != NULL )
      {
         /*
          * Fixes bug 777645
          */
         q->u.i.elements -= b->elements ;
         retval->u.t = *b ;
         retval->u.t.higher = retval->u.t.lower = NULL ;
         b->top = b->bottom = NULL ;
         b->elements = 0 ;
      }
   }
#if !defined( NO_EXTERNAL_QUEUES )
   else
   {
      /* loop until EOF. We can't use lines_in_stack, since the number may
       * change because of multiple accesses by different clients.
       */
      while ( get_line_from_rxstack( TSD, q->u.e.socket, &contents, 1 ) == 0 )
      {
         ptr = (StackLine *) MallocTSD( sizeof( StackLine ) ) ;
         ptr->contents = contents ;
         FIFO_LINE( &retval->u.t, ptr ) ;
      }
   }
#endif
   return retval ;
}

/* addr_purge_queue discards the contents of a queue. It is caused by a
 * ADDRESS WITH ??? REPLACE ??? or at the end of the use of an input
 * queue.
 */
void addr_purge_queue( const tsd_t *TSD, Queue *q )
{
   stk_tsd_t *st ;
   Buffer *b ;

   st = (stk_tsd_t *)TSD->stk_tsd ;
   assert( ( q->type == QisSESSION )
        || ( q->type == QisInternal )
        || ( q->type == QisExternal ) ) ;

   if ( ( q->type == QisSESSION ) || ( q->type == QisInternal ) )
   {
      ENSURE_BUFFER( q ) ;
      b = q->u.i.top ;
      q->u.i.elements -= b->elements ;
      delete_buffer_content( TSD, st, b ) ;
   }
#if !defined( NO_EXTERNAL_QUEUES )
   else
   {
      clear_queue_on_rxstack( TSD, q->u.e.socket ) ;
   }
#endif
}

streng *addr_io_queue( tsd_t *TSD, Queue *q, streng *buffer, int isFIFO )
/* This is the working routine for the ADDRESS WITH-redirection. q is
 * the return value of addr_reopen_queue. buffer must be NULL for a read
 * operation or a filled buffer.
 * The return value is NULL in case of a write operation or in case of EOF
 * while reading.
 * buffer is consumed if given and indicates a push operation instead of a
 * pull operation. isFIFO indicates the type of the push operation.
 */
{
   StackLine *ptr, *newbox ;
   Buffer *b ;
   streng *retval ;

   if ( buffer == NULL )
   {
      /* pull operation */
      switch ( q->type )
      {
         case QisSESSION:
         case QisInternal:
            ENSURE_BUFFER( q ) ;
            b = q->u.i.top ; /* Work on the top buffer only */
            POP_LINE( b, ptr ) ;
            if ( ptr == NULL )
               return NULL ;
            q->u.i.elements-- ;
            retval = ptr->contents ;
            FreeTSD( ptr ) ;
            return retval ;

         case QisTemp:
            b = &q->u.t ;
            POP_LINE( b, ptr ) ;
            if ( ptr == NULL )
               return NULL ;
            retval = ptr->contents ;
            FreeTSD( ptr ) ;
            return retval ;

#if !defined( NO_EXTERNAL_QUEUES )
         case QisExternal:
            if ( get_line_from_rxstack( TSD, q->u.e.socket, &retval, 1 ) == 0 )
               return retval ;
            return NULL ;
#endif

         default:
            break ;
      }
      /* default: (probably an unused queue) */
      return NULL ;
   }

   /* buffer's not NULL */
   assert( ( Str_max( buffer ) >= Str_len( buffer ) ) ) ;

#if !defined(NO_EXTERNAL_QUEUES)
   if ( q->type == QisExternal )
   {
      if ( isFIFO )
         queue_line_fifo_to_rxstack( TSD, q->u.e.socket, buffer ) ;
      else
         queue_line_lifo_to_rxstack( TSD, q->u.e.socket, buffer ) ;
      return NULL ;
   }
#endif

   newbox = (StackLine *) MallocTSD( sizeof( StackLine ) ) ;
   newbox->contents = buffer ;
   if ( isFIFO )
   {
      /* push FIFO operation */
      switch ( q->type )
      {
         case QisSESSION:
         case QisInternal:
            ENSURE_BUFFER( q ) ;
            b = q->u.i.top ;
            q->u.i.elements++ ;
            FIFO_LINE( b, newbox ) ;
            break ;

         case QisTemp:
            b = &q->u.t ;
            q->u.i.elements++ ;
            FIFO_LINE( b, newbox ) ;
            break ;

         default:
            break ;
      }
      return NULL ;
   }

   /* finally, we process push LIFO */
   switch ( q->type )
   {
      case QisSESSION:
      case QisInternal:
         ENSURE_BUFFER( q ) ;
         b = q->u.i.top ;
         q->u.i.elements++ ;
         LIFO_LINE( b, newbox ) ;
         break ;

      case QisTemp:
         b = &q->u.t ;
         q->u.i.elements++ ;
         LIFO_LINE( b, newbox ) ;
         break ;

      default:
         break ;
   }
   return NULL ;
}

/* addr_close_queue closes a queue opened by addr_reopen_queue.
 * Never use another routine to do this.
 * It is also capable to purge and close temporary stacks.
 */
void addr_close_queue( const tsd_t *TSD, Queue *q )
{
   stk_tsd_t *st ;

   st = (stk_tsd_t *)TSD->stk_tsd;
   if ( q->type == QisTemp )
   {
      delete_a_temp_queue( TSD, st, q ) ;
      return ;
   }
#if !defined(NO_EXTERNAL_QUEUES)
   if ( ( q != st->current_queue ) && ( q->type == QisExternal ) )
      delete_an_external_queue( TSD, st, q ) ;
#endif
}
