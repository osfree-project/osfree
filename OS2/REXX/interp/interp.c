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

static void set_line_nos( treenode *ptr, int lineno, int charno )
{
   unsigned i=0 ;

   if (!ptr)
      return ;

   if (ptr->lineno >= 0)
   {
      ptr->lineno = lineno ;
      ptr->charnr = charno ;
   }

   for (i=0; i<sizeof(ptr->p)/sizeof(ptr->p[0]); i++)
      if (ptr->p[i])
         set_line_nos( ptr->p[i], lineno, charno ) ;

   if (ptr->next)
      set_line_nos( ptr->next, lineno, charno ) ;
}

/* Will do the "INTERPRET" work on a given streng. This function returns
 * either NULL on error or the result string of the interpretation.
 * The streng will be deleted.
 */
streng *dointerpret( tsd_t *TSD, streng *string )
{
   treenode *newtree ;
   nodeptr savecurrentnode ;
   streng *ptr;
   internal_parser_type parsing;
   int errpos;

   fetch_string( TSD, string, &parsing );

   if (parsing.result != 0)
   {
      Free_stringTSD(string) ;
      /*
       * get position from parent script if available and add it.
       * fixes bug 658542.
       */
      errpos = TSD->currentnode->lineno;
      errpos = ( errpos > 0 ) ? errpos - 1 : 0 ;
      exiterror( ERR_YACC_SYNTAX, 1, parsing.tline + errpos ) ;
      return NULL ;
   }

   newtree = parsing.root ;
   parsing.kill = string ;
   if (TSD->currentnode)
      set_line_nos( newtree, TSD->currentnode->lineno, TSD->currentnode->charnr ) ;

   /* Save and restore currentnode around interpret. It is set within
    * interpret and may result to illegal memory accesses in case of
    * errors if it is not restored, FGC
    */
   savecurrentnode = TSD->currentnode;
   ptr = interpret( TSD, newtree ) ;
   TSD->currentnode = savecurrentnode;
#if 0
   /*
    * FIXME: Always clean up the structure. newtree may be NULL in case
    *        of an empty command where we still have to remove the
    *        single empty line. I can't remember why we should destroy
    *        it just in that case we have a newtree. What is a typical
    *        other reason of not having newtree?
    *        In either case, delete this comment and the "#if 0"-block
    *        one year after now. Florian, 04.03.2004
    */
   if (newtree)
#endif
      DestroyInternalParsingTree( TSD, &parsing ) ;

   return ptr ;
}
