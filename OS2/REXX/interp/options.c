#ifndef lint
static char *RCSid = "$Id: options.c,v 1.14 2004/02/22 09:25:29 florian Exp $";
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

#include "rexx.h"

#include <assert.h>
#include <string.h>


#define OPTION(opt) { #opt, EXT_##opt, NULL }
#define METAOP(name,value) { #name, -1, value }


static const struct __regina_option all_options[] = {  /* Must be alphabetically sorted! */
   OPTION( AREXX_BIFS ),
   OPTION( AREXX_SEMANTICS ),
   OPTION( BROKEN_ADDRESS_COMMAND ),
   METAOP( BUFFERS, "BUFTYPE_BIF DESBUF_BIF DROPBUF_BIF MAKEBUF_BIF" ),
   OPTION( BUFTYPE_BIF ),
   OPTION( CACHEEXT ),
   OPTION( CALLS_AS_FUNCS ),
   OPTION( DESBUF_BIF ),
   OPTION( DROPBUF_BIF ),
   OPTION( EXT_COMMANDS_AS_FUNCS ),
   OPTION( FAST_LINES_BIF_DEFAULT ),
   OPTION( FLUSHSTACK ),
   OPTION( INTERNAL_QUEUES ),
   OPTION( LINEOUTTRUNC ),
   OPTION( MAKEBUF_BIF ),
   OPTION( PRUNE_TRACE ),
   OPTION( QUEUES_301 ),
   OPTION( REGINA_BIFS ),
   OPTION( STDOUT_FOR_STDERR ),
   OPTION( STRICT_ANSI ),
   OPTION( STRICT_WHITE_SPACE_COMPARISONS ),
   OPTION( TRACE_HTML ),
   { NULL, 0 }
} ;


void do_options( const tsd_t *TSD, proclevel pl, streng *options, int toggle )
{
   char *cptr,*eptr,*start;
   int length,inverse=0,tmp;
   const struct __regina_option *lower,*upper,*middle=NULL;

   cptr = options->value;
   eptr = cptr + options->len;

   while ( cptr < eptr )
   {
      for ( ; cptr < eptr && rx_isspace( *cptr ); cptr++ )
         ;
      for ( start = cptr; cptr < eptr && !rx_isspace( *cptr ); cptr++ )
         *cptr = (char) rx_toupper( *cptr );

      if ( cptr > start+2 )
      {
         if ( ( inverse = ( *start == 'N' && *( start + 1) == 'O') ) != 0 )
            start += 2;
      }
      length = cptr - start;

      lower = all_options;
      upper = lower + sizeof( all_options ) / sizeof( all_options[0] ) - 2;

      while ( upper >= lower )
      {
         middle = lower + ( upper - lower ) / 2;
         tmp = strncmp( middle->name, start, length );
         if ( tmp == 0 && middle->name[length] == '\0' )
            break;

         if ( tmp > 0 )
            upper = middle - 1;
         else
            lower = middle + 1;
      }

      /* If option is unknown, don't care ... */
      if ( upper >= lower )
      {
         assert ( middle->name );
         if ( middle->offset == -1 )
         {
            do_options( TSD, pl, Str_creTSD( middle->contains ),
                        toggle ^ inverse );
         }
         else
         {
            if (inverse ^ toggle)
               set_options_flag( pl, middle->offset, 0 );
            else
               set_options_flag( pl, middle->offset, 1 );
         }
      }
   }
   Free_stringTSD( options );
}

int get_options_flag( cproclevel pl, int offset )
{
   return pl->options & ( 1ul << offset );
}

void set_options_flag( proclevel pl, int offset, int status )
{
   if ( status )
      pl->options |= ( 1ul << offset );
   else
      pl->options &= ~( 1ul << offset );
}
