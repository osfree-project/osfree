#ifndef lint
static char *RCSid = "$Id: options.c,v 1.2 2003/12/11 04:43:14 prokushev Exp $";
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
   OPTION( DESBUF_BIF ),
   OPTION( DROPBUF_BIF ),
   OPTION( EXT_COMMANDS_AS_FUNCS ),
   OPTION( FAST_LINES_BIF_DEFAULT ),
   OPTION( FLUSHSTACK ),
   OPTION( INTERNAL_QUEUES ),
   OPTION( LINEOUTTRUNC ),
   OPTION( MAKEBUF_BIF ),
   OPTION( PRUNE_TRACE ),
   OPTION( REGINA_BIFS ),
   OPTION( STDOUT_FOR_STDERR ),
   OPTION( STRICT_ANSI ),
   OPTION( STRICT_WHITE_SPACE_COMPARISONS ),
   OPTION( TRACE_HTML ),
   { NULL, 0 }
} ;


void do_options( const tsd_t *TSD, streng *options, int toggle )
{
   char *cptr=NULL, *eptr=NULL, *start=NULL ;
   int length=0, inverse=0, tmp=0 ;
   const struct __regina_option *lower=NULL, *upper=NULL, *middle=NULL ;

   cptr = options->value ;
   eptr = cptr + options->len ;

   while (cptr<eptr)
   {
      for (;cptr<eptr && isspace(*cptr); cptr++) ;
      for (start=cptr; cptr<eptr && !isspace(*cptr); cptr++ )
         *cptr = (char) toupper( *cptr ) ;

      if (((inverse=(*start=='N' && *(start+1)=='O'))!=0) && cptr>start+2)
         start += 2 ;

      length = cptr - start ;

      lower = all_options ;
      upper = lower + (sizeof(all_options)/sizeof(struct __regina_option)) - 2 ;

      while( upper >= lower )
      {
         middle = lower + (upper-lower)/2 ;
         tmp = strncmp(middle->name,start,length) ;
         if (tmp==0 && middle->name[length]==0x00)
            break ;

         if (tmp>0)
            upper = middle - 1 ;
         else
            lower = middle + 1 ;
      }

      /* If option is unknown, don't care ... */
      if ( upper >= lower )
      {
         assert ( middle->name ) ;
         if (middle->offset == -1)
         {
            do_options( TSD, Str_creTSD(middle->contains), toggle^inverse ) ;
         }
         else
         {

            if (inverse^toggle)
               set_options_flag( TSD->currlevel, middle->offset, 0 ) ;
            else
               set_options_flag( TSD->currlevel, middle->offset, 1 ) ;
         }
      }
   }
   Free_stringTSD( options ) ;
}

int get_options_flag( cproclevel pl, int offset )
{
   register int obyte = offset / ( sizeof( unsigned char ) * 8 ) ;
   register int obit  = offset % ( sizeof( unsigned char ) * 8 ) ;

   return ( pl->u.flags[obyte] & ( 1 << ( 7 - obit ) ) ) ;
}

void set_options_flag( proclevel pl, int offset, int status )
{
   register int obyte = offset / ( sizeof( unsigned char ) * 8 ) ;
   register int obit  = offset % ( sizeof( unsigned char ) * 8 ) ;

   if ( status )
      pl->u.flags[obyte] |= (unsigned char)(1<<(7-obit)) ;
   else
      pl->u.flags[obyte] &= (unsigned char)(~(1<<(7-obit))) ;
}
