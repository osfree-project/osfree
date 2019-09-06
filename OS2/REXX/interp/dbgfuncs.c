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
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef REGINA_DEBUG_MEMORY
streng *dbg_freelists( tsd_t *TSD, cparamboxptr dummy )
{
   show_free_lists(TSD);
   return nullstringptr() ;
}
#endif

streng *dbg_traceback( tsd_t *TSD, cparamboxptr dummy )
{
   traceback(TSD) ;
   dummy = dummy; /* keep compiler happy */
   return nullstringptr() ;
}

#ifndef NDEBUG


streng *dbg_dumpvars( tsd_t *TSD, cparamboxptr dummy )
{
   dumpvars(TSD);
   dummy = dummy; /* keep compiler happy */
   return nullstringptr() ;
}


#ifdef TRACEMEM
streng *dbg_memorystats( tsd_t *TSD, cparamboxptr parms )
{
   memory_stats(TSD) ;
   return nullstringptr() ;
}


streng *dbg_allocated( tsd_t *TSD, cparamboxptr parms )
{
   char ch=' ' ;
   streng *ptr=NULL ;

   checkparam(  parms,  0,  1 , "ALLOCATED" ) ;
   if (!parms->value)
      ch = 'S' ;
   else
      ch = getonechar( TSD, parms->value, "ALLOCATED", 1 ) ;

   switch ( ch )
   {
      case 'A' :
         ptr = int_to_streng( TSD,have_allocated(TSD, MEM_ALLOC)) ;
         break ;

      case 'L' :
         ptr = int_to_streng( TSD,have_allocated(TSD, MEM_LEAKED)) ;
         break ;

      case 'C' :
         ptr = int_to_streng( TSD,have_allocated(TSD, MEM_CURRENT)) ;
         break ;

      case 'S' :
          ptr = Str_makeTSD( 132 ) ;
          sprintf( ptr->value,"Memory: Allocated=%d, Current=%d, Leaked=%d",
                 have_allocated(TSD, MEM_ALLOC),
                 have_allocated(TSD, MEM_CURRENT),
                 have_allocated(TSD, MEM_LEAKED)) ;

          ptr->len = strlen( ptr->value ) ;
          assert( ptr->len <= ptr->max ) ;
          break ;

      default:
          exiterror( ERR_INCORRECT_CALL, 28, "ALLOCATED", "ALCS", tmpstr_of( TSD, parms->value ) )  ;
   }

   return( ptr ) ;
}
#endif


streng *dbg_dumptree( tsd_t *TSD, cparamboxptr dummy )
{
   dumptree( TSD, TSD->systeminfo->tree.root, 1, 1 ) ;
   dummy = dummy; /* keep compiler happy */
   return nullstringptr() ;
}


#ifdef TRACEMEM


streng *dbg_listleaked( tsd_t *TSD, cparamboxptr parms )
{
   char ch=0 ;
   int i=0 ;

   checkparam(  parms,  0,  1 , "LISTLEAKED" ) ;
   if (parms->value)
      ch = getonechar( TSD, parms->value, "LISTLEAKED", 1 ) ;
   else
      ch = 'L' ;

   if (ch=='N')
      i = listleaked( TSD, MEMTRC_NONE ) ;
   else if (ch=='L')
      i = listleaked( TSD, MEMTRC_LEAKED ) ;
   else if (ch=='A')
      i = listleaked( TSD, MEMTRC_ALL ) ;
   else
      exiterror( ERR_INCORRECT_CALL, 28, "LISTLEAKED", "ALN", tmpstr_of( TSD, parms->value ) )  ;

   return int_to_streng( TSD, i ) ;
}
#endif

#endif /* !NDEBUG */
