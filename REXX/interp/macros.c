#ifndef lint
static char *RCSid = "$Id: macros.c,v 1.2 2003/12/11 04:43:12 prokushev Exp $";
#endif

/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1993-1994  Anders Christensen <anders@pvv.unit.no>
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
#include "rxiface.h"

#include <stdio.h>
#include <setjmp.h>
#include <string.h>
#include <assert.h>


void killsystem( tsd_t *TSD, sysinfo systm )
{
   if ( systm->environment )
   {
      Free_stringTSD( systm->environment ) ;
      systm->environment = NULL;
   }

   if ( systm->called_as )
   {
      Free_stringTSD( systm->called_as ) ;
      systm->called_as = NULL;
   }

   if (systm->input_file )
   {
      Free_stringTSD( systm->input_file ) ;
      systm->input_file = NULL;
   }

   if (systm->input_fp )
   {
      fclose( systm->input_fp ) ;
      systm->input_fp = NULL;
   }

   DestroyInternalParsingTree( TSD, &systm->tree ) ;

   if (systm->currlevel0)
     {
      removelevel( TSD, systm->currlevel0 ) ;
      systm->currlevel0 = NULL;
     }

   if (systm->panic)
   {
      FreeTSD( systm->panic ) ;
      systm->panic = NULL;
   }

   if ( systm->result)
   {
      Free_stringTSD( systm->result ) ;
      systm->result = NULL;
   }

   if (systm->callstack)
   {
      FreeTSD( systm->callstack ) ;
      systm->callstack = NULL;
   }

   FreeTSD( systm ) ;
}

streng *do_instore( tsd_t * volatile TSD, const streng *name, paramboxptr args,
                    const streng *envir, int * volatile RetCode, int hooks,
                    const void *instore, unsigned long instore_length,
                    const char *instore_source,
                    unsigned long instore_source_length,
                    const internal_parser_type *ipt,
                    int ctype )
{
   sysinfobox *newsystem=NULL, *tmpsys=NULL ;
   streng *ptr=NULL ;
   jmp_buf *jbuf=NULL ;
   unsigned InterpreterStatus[IPRT_BUFSIZE];
   tsd_t * volatile saved_TSD;
   int * volatile saved_RetCode;
   volatile proclevel oldlevel;

   if (RetCode)
      *RetCode = 0 ;

   SaveInterpreterStatus(TSD,InterpreterStatus);
   TSD->instore_is_errorfree = 0 ;
   jbuf = MallocTSD( sizeof(jmp_buf) ) ;
   assert(!TSD->in_protected);

   saved_TSD = TSD; /* vars used until here */
   saved_RetCode = RetCode;
   if (setjmp(*jbuf))
   {
      TSD = saved_TSD; /* prevents bugs like  592393 */
      RetCode = saved_RetCode;

      ptr = TSD->systeminfo->result ;
      TSD->systeminfo->result = NULL ;
      if (!TSD->instore_is_errorfree && RetCode)
         *RetCode = -1 ;
   }
   else
   {
      nodeptr savecurrentnode = TSD->currentnode; /* pgb fixes bug 595300 */

      TSD->currentnode = NULL ;

      newsystem = creat_sysinfo( TSD, Str_dupTSD(envir)) ;
      newsystem->previous = TSD->systeminfo ;
      /*
       * see note in execute_external
       * Fixes bug 604219
       */
      oldlevel = TSD->currlevel;

      TSD->currlevel = NULL ;
      TSD->systeminfo = newsystem ;
      TSD->systeminfo->hooks = hooks ;
      TSD->systeminfo->panic = jbuf ;
      TSD->systeminfo->invoked = ctype ;
      TSD->systeminfo->called_as = Str_dupTSD( name ) ;
      TSD->systeminfo->input_file = Str_dupstrTSD( name ) ;
      TSD->systeminfo->currlevel0 = TSD->currlevel = newlevel( TSD, NULL ) ;

      TSD->systeminfo->trace_override = newsystem->previous->trace_override;

      TSD->currlevel->args = args ;

      if (ipt)
         TSD->systeminfo->tree = *ipt;
      else if (IsValidTin(instore, instore_length))
         TSD->systeminfo->tree = ExpandTinnedTree(TSD, instore, instore_length,
                                                  instore_source,
                                                  instore_source_length);
      else
      {
         memset(&TSD->systeminfo->tree, 0, sizeof(TSD->systeminfo->tree));
      }
      if (TSD->systeminfo->hooks & HOOK_MASK(HOOK_INIT))
         hookup( TSD, HOOK_INIT ) ;

      if (TSD->systeminfo->tree.root)
         ptr = interpret( TSD, TSD->systeminfo->tree.root ) ;
      else
         ptr = NULL ;
      TSD->currentnode = savecurrentnode; /* pgb */

   }
   if (TSD->systeminfo->hooks & HOOK_MASK(HOOK_TERMIN))
      hookup( TSD, HOOK_TERMIN ) ;

   tmpsys = TSD->systeminfo ;
   TSD->systeminfo = TSD->systeminfo->previous ;
   TSD->currlevel = oldlevel;
   TSD->trace_stat = TSD->currlevel->tracestat ;

   tmpsys->currlevel0->args = NULL ;
   killsystem( TSD, tmpsys ) ;

   RestoreInterpreterStatus(TSD,InterpreterStatus);
   /* Oops, we really ought to handle function-did-not-return-data */
   return (ptr) ? ptr : nullstringptr()  ;
}

streng *execute_external( tsd_t * volatile TSD, const streng *command,
                          paramboxptr args, const streng *envir,
                          int * volatile RetCode, int hooks, int ctype )
{
   sysinfobox *newsystem=NULL, *tmpsys=NULL ;
   char name[1024] ;
   streng *ptr=NULL ;
   const char *cptr=NULL, *eptr=NULL, *start=NULL, *stop=NULL ;
   char path[1024] ;
   FILE *fptr ;
   jmp_buf *jbuf=NULL ;
   internal_parser_type parsing;
   volatile proclevel oldlevel;
   unsigned InterpreterStatus[IPRT_BUFSIZE];
   nodeptr savecurrentnode = TSD->currentnode;
   tsd_t * volatile saved_TSD;
   int * volatile saved_RetCode;

   if (RetCode)
      *RetCode = 0 ;

   SaveInterpreterStatus(TSD,InterpreterStatus);
   jbuf = MallocTSD( sizeof(jmp_buf) ) ;
   TSD->instore_is_errorfree = 0 ;
   assert(!TSD->in_protected);

   saved_TSD = TSD; /* vars used until here */
   saved_RetCode = RetCode;

   if (setjmp(*jbuf))
   {
      TSD = saved_TSD; /* prevents bugs like  592393 */
      RetCode = saved_RetCode;

      ptr = TSD->systeminfo->result ;
      TSD->systeminfo->result = NULL ;
      if (!TSD->instore_is_errorfree && RetCode)
         *RetCode = -1 ;
   }
   else
   {
      fptr = NULL ;
      /* FGC: Check length first to avoid  */
      /*      access of invalid buffer     */
      if ( ( command->len == 7 )
      && ( memcmp("<stdin>",command->value,command->len) == 0 ) )
      {
         fptr = stdin;
         strcpy(name,command->value);
      }
      else
      {
         cptr = command->value ;
         eptr = cptr + command->len ;

         for (start=cptr; start<eptr && isspace(*start); start++) ;
         for (stop=eptr-1;stop>start && isspace(*stop);  stop--) ;

         memcpy( path, start, (stop-start)+1 ) ;
         *(path+(stop-start)+1) = 0x00 ;
         get_external_routine( TSD, "REGINA_MACROS", path, &fptr, name, 1 );
         if (!fptr)
         {
            get_external_routine( TSD, "PATH", path, &fptr, name, 1 );
            if (!fptr)
            {
               FreeTSD( jbuf ) ;
               if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
               {
                  /*
                   * If we can't find the external routine, we should exiterror()
                   * with 43.1 - as per ANSI
                   */
                  exiterror( ERR_ROUTINE_NOT_FOUND, 1, path );
               }
               else
               {
                  /*
                   * The only time this function is called with a non-NULL
                   * RetCode is from client.c, when an external routine
                   * is being executed via the SAA interface. In this case
                   * the error returned is the only thing we should do;
                   * we certainly should NOT be writing anything to stdout
                   * or stderr!
                   */
                  if (RetCode)
                  {
                     *RetCode = -ERR_PROG_UNREADABLE;
                  }
                  return NULL ;
               }
            }
         }
      }

      newsystem = creat_sysinfo( TSD, Str_dupTSD(envir)) ;
      newsystem->previous = TSD->systeminfo ;
      /* FGC: NOTE: I found that currlevel has changed outside between calls
       *            to this function. I really don't know, if this should
       *            happen. A typical change of currlevel is done in interpret.
       *            Maybe, in interpret is an error caused by an illegal
       *            "postrecursed" re-interpret. Somebody with a higher view of
       *            the code as mine should check the code there.
       *            I detected the error in THE using regina while calling
       *            macros in macros called by THE in a macro.
       *            (confusing, hmm? :-(  )
       * FIXME!
       */
      oldlevel = TSD->currlevel;

      TSD->currlevel = NULL ;
      TSD->systeminfo = newsystem ;
      TSD->systeminfo->hooks = hooks ;
      TSD->systeminfo->invoked = ctype ;
      TSD->systeminfo->panic = jbuf ;
      TSD->systeminfo->called_as = Str_dupTSD( command ) ;
      TSD->systeminfo->input_file = Str_crestrTSD( name ) ;
      TSD->systeminfo->currlevel0 = TSD->currlevel = newlevel( TSD, NULL ) ;

      savecurrentnode = TSD->currentnode; /* pgb fixes bug 595300 */
      TSD->systeminfo->trace_override = newsystem->previous->trace_override;

      TSD->currlevel->args = args ;
      TSD->currentnode = NULL ;

      fetch_file( TSD, fptr, &parsing );
      if (fptr != stdin)
         fclose(fptr) ;
      if (parsing.result == 0)
      {
         TSD->systeminfo->tree = parsing ;
         treadit( TSD->systeminfo->tree.root ) ;
         /*
          * Execute any RXINI system exit
          */
         if (TSD->systeminfo->hooks & HOOK_MASK(HOOK_INIT))
            hookup( TSD, HOOK_INIT ) ;

         ptr = interpret( TSD, TSD->systeminfo->tree.root ) ;
         TSD->currentnode = savecurrentnode; /* pgb */
      }
      else
      {
         TSD->currentnode = savecurrentnode; /* pgb */
         ptr = NULL ;
         exiterror( ERR_YACC_SYNTAX, 1, parsing.tline ) ;
      }
   }
   /*
    * Execute any RXTER system exit
    */
   if (TSD->systeminfo->hooks & HOOK_MASK(HOOK_TERMIN))
      hookup( TSD, HOOK_TERMIN ) ;

/*
   if (must_pop)
      popcallstack( -1 ) ;
*/
   tmpsys = TSD->systeminfo ;
   TSD->systeminfo = TSD->systeminfo->previous ;
   TSD->currlevel = oldlevel;
   TSD->trace_stat = TSD->currlevel->tracestat ;

   tmpsys->currlevel0->args = NULL ;
   killsystem( TSD, tmpsys ) ;

   RestoreInterpreterStatus(TSD,InterpreterStatus);
   /* Oops, we really ought to handle function-did-not-return-data */
   return (ptr) ? ptr : nullstringptr()  ;
}


internal_parser_type enter_macro( tsd_t *TSD, const streng *source,
                                  streng *name, void **ept,
                                  unsigned long *extlength)
{
   internal_parser_type parsing;

   fetch_string( TSD, source, &parsing );
   if (parsing.result != 0)
   {
      exiterror( ERR_YACC_SYNTAX, 1, parsing.tline ) ;
      return(parsing);
   }
   if (ept && extlength)
      *ept = TinTree( TSD, &parsing, extlength ) ;
   name = name; /* keep compiler happy */

   return( parsing );
}

/*
 * Takes as input a pointer to a parameter structure, and counts the
 * number of parameters in it, and return that value. The counting
 * can be performed in two different manners, either soft or hard.
 * Soft means that "trailing" ommitted parameters are ignored, hard
 * means that all parameters are counted. (When counting hard, all
 * routines stared from Rexx has at least one parameter: the one that
 * was ommitted. However, when started from C by SAA API, functions
 * can be started with zero parameters.)
 */
int count_params( cparamboxptr ptr, int soft )
{
   int scnt=0, hcnt=0 ;

   assert( PARAM_TYPE_SOFT && !PARAM_TYPE_HARD ) ;

   for (hcnt=scnt=0; ptr; ptr=ptr->next, hcnt++)
      if (soft && ptr->value)
         scnt = hcnt ;

   return ((soft) ? scnt : hcnt ) ;

}


/*
 * Takes a pointer to an argument structure as input, together with an
 * integer. Returns the parameter numbered by the number, or NULL if
 * either that parameter is omitted or otherwise non-existing.
 */
streng *get_parameter( paramboxptr ptr, int number )
{
   assert( number>0 ) ;
   for (; ptr && (--number!=0); ptr=ptr->next) ;

   return ((ptr && ptr->value) ? ptr->value : NULL ) ;
}
