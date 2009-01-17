#ifndef lint
static char *RCSid = "$Id: macros.c,v 1.24 2006/02/20 07:54:00 mark Exp $";
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

   if ( systm->script_exit )
   {
      FreeTSD( systm->script_exit ) ;
      systm->script_exit = NULL;
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
   sysinfobox *newsystem, *tmpsys;
   streng *ptr=NULL;
   jmp_buf *jbuf;
   unsigned InterpreterStatus[IPRT_BUFSIZE];
   tsd_t * volatile saved_TSD;
   int * volatile saved_RetCode;
   volatile proclevel oldlevel;
   volatile int doTermHook=0;

   if ( RetCode )
      *RetCode = 0;

   SaveInterpreterStatus( TSD, InterpreterStatus );
   jbuf = (jmp_buf *)MallocTSD( sizeof( jmp_buf ) );
   assert( !TSD->in_protected );

   saved_TSD = TSD;            /* vars used until here */
   saved_RetCode = RetCode;
   if ( setjmp( *jbuf ) )
   {
      TSD = saved_TSD;         /* prevents bugs like  592393 */
      RetCode = saved_RetCode;

      ptr = TSD->systeminfo->result;
      TSD->systeminfo->result = NULL;
      if ( !TSD->instore_is_errorfree )
      {
         /*
          * In case of an error we don't return the error number as the result.
          * Instead, use the current "result" as the error code. It is set
          * to a negative value already by errortext().
          *
          * ==> errortext() uses a static buffer, if you ever need the value
          *     you have to dup it after the restore of the old system or you
          *     risk an endless loop.
          */
         if ( RetCode )
            *RetCode = atoi( ptr->value );

         ptr = NULL;
      }
   }
   else
   {
      nodeptr savecurrentnode = TSD->currentnode; /* pgb fixes bug 595300 */

      TSD->currentnode = NULL;

      newsystem = creat_sysinfo( TSD, Str_dupTSD( envir ) );
      newsystem->previous = TSD->systeminfo;
      newsystem->hooks = hooks;
      newsystem->script_exit = jbuf;
      newsystem->invoked = ctype;
      newsystem->input_file = Str_dupstrTSD( name );
      newsystem->trace_override = newsystem->previous->trace_override;
      newsystem->ctrlcounter = newsystem->previous->ctrlcounter +
                                               newsystem->previous->cstackcnt;

      /*
       * see note in execute_external
       * Fixes bug 604219
       */
      oldlevel = TSD->currlevel;

      TSD->systeminfo = newsystem;
      TSD->systeminfo->currlevel0 = TSD->currlevel = newlevel( TSD, NULL );
      TSD->currlevel->pool = oldlevel->pool + 1;

      TSD->currlevel->args = args;

      if ( ipt )
         TSD->systeminfo->tree = *ipt;
      else if ( IsValidTin( (const external_parser_type *)instore, instore_length ) )
         TSD->systeminfo->tree = ExpandTinnedTree( TSD, (const external_parser_type *)instore, instore_length,
                                                   instore_source,
                                                   instore_source_length );
      else
      {
         memset( &TSD->systeminfo->tree, 0, sizeof( TSD->systeminfo->tree ) );
      }
      if ( TSD->systeminfo->hooks & HOOK_MASK( HOOK_INIT ) )
         hookup( TSD, HOOK_INIT );

      doTermHook = 1;

      if ( TSD->systeminfo->tree.root )
         ptr = interpret( TSD, TSD->systeminfo->tree.root );
      else
         ptr = NULL;
      TSD->currentnode = savecurrentnode; /* pgb */
   }

   if ( doTermHook )
   {
      /*
       * Be sure to try it only once.
       */
      doTermHook = 0;
      if ( TSD->systeminfo->hooks & HOOK_MASK( HOOK_TERMIN ) )
         hookup( TSD, HOOK_TERMIN );
   }

   tmpsys = TSD->systeminfo;
   TSD->systeminfo = TSD->systeminfo->previous;
   TSD->currlevel = oldlevel;
   TSD->trace_stat = TSD->currlevel->tracestat;

   tmpsys->currlevel0->args = NULL;

   /*
    * jbuf will be freed by killsystem.
    */
   killsystem( TSD, tmpsys );

   RestoreInterpreterStatus( TSD, InterpreterStatus );

   return ptr;
}

streng *execute_external( tsd_t * volatile TSD, const streng *command,
                          paramboxptr args, const streng *envir,
                          int * volatile RetCode, int hooks, int ctype )
{
   sysinfobox *newsystem, *tmpsys;
   streng *ptr=NULL;
   char *path;
   streng *name;
   int len;
   FILE *fptr;
   jmp_buf *jbuf;
   internal_parser_type parsing;
   volatile proclevel oldlevel;
   unsigned InterpreterStatus[IPRT_BUFSIZE];
   nodeptr savecurrentnode = TSD->currentnode;
   tsd_t * volatile saved_TSD;
   int * volatile saved_RetCode;
   volatile int doTermHook=0;
   int iserror=0;

   if ( RetCode )
      *RetCode = 0;

   SaveInterpreterStatus( TSD, InterpreterStatus );
   jbuf = (jmp_buf *)MallocTSD( sizeof( jmp_buf ) );
   assert( !TSD->in_protected );

   saved_TSD = TSD; /* vars used until here */
   saved_RetCode = RetCode;

   if ( setjmp( *jbuf ) )
   {
      TSD = saved_TSD;          /* prevents bugs like  592393 */
      RetCode = saved_RetCode;

      ptr = TSD->systeminfo->result;
      TSD->systeminfo->result = NULL;
      if ( !TSD->instore_is_errorfree )
      {
         /*
          * In case of an error we don't return the error number as the result.
          * Instead, use the current "result" as the error code. It is set
          * to a negative value already by errortext().
          *
          * ==> errortext() uses a static buffer, if you ever need the value
          *     you have to dup it after the restore of the old system or you
          *     risk an endless loop.
          */
         if ( RetCode )
            *RetCode = atoi( ptr->value );
         /*
          * Defer the exiting with 40.1 until the jmpbuf stuff is done
          */
         iserror = 1;
         ptr = NULL;
      }
   }
   else
   {
      fptr = NULL;
      /* FGC: Check length first to avoid  */
      /*      access of invalid buffer     */
      if ( ( command->len == 7 )
      && ( memcmp( "<stdin>", command->value, command->len ) == 0 ) )
      {
         fptr = stdin;
         name = Str_dupstrTSD( command );
      }
      else
      {
         path = (char *) tmpstr_of( TSD, command );
         while ( rx_isspace( *path ) )
            path++;
         len = strlen( path );
         while ( len > 0 )
            if ( !rx_isspace( path[len - 1] ) )
               break;
            else
               len--;
         path[len] = '\0';

         name = get_external_routine( TSD, path, &fptr );
         if ( !name )
         {
            FreeTSD( jbuf );
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
               if ( RetCode )
               {
                  *RetCode = -ERR_PROG_UNREADABLE;
               }
               return NULL;
            }
         }
      }

      newsystem = creat_sysinfo( TSD, Str_dupTSD( envir ) );
      newsystem->previous = TSD->systeminfo;
      newsystem->hooks = hooks;
      newsystem->invoked = ctype;
      newsystem->script_exit = jbuf;
      newsystem->input_file = name;
      newsystem->trace_override = newsystem->previous->trace_override;
      newsystem->ctrlcounter = newsystem->previous->ctrlcounter +
                                               newsystem->previous->cstackcnt;

      oldlevel = TSD->currlevel;

      TSD->systeminfo = newsystem;
      TSD->systeminfo->currlevel0 = TSD->currlevel = newlevel( TSD, NULL );
      TSD->currlevel->pool = oldlevel->pool + 1;

      savecurrentnode = TSD->currentnode; /* pgb fixes bug 595300 */

      TSD->currlevel->args = args;
      TSD->currentnode = NULL;

      fetch_file( TSD, fptr, &parsing );
      if ( fptr != stdin )
         fclose( fptr );
      if ( parsing.result == 0 )
      {
         TSD->systeminfo->tree = parsing;
         /*
          * Execute any RXINI system exit
          */
         if ( TSD->systeminfo->hooks & HOOK_MASK( HOOK_INIT ) )
            hookup( TSD, HOOK_INIT );

         doTermHook = 1;

         ptr = interpret( TSD, TSD->systeminfo->tree.root );
         TSD->currentnode = savecurrentnode; /* pgb */
      }
      else
      {
         TSD->currentnode = savecurrentnode; /* pgb */
         ptr = NULL;
         exiterror( ERR_YACC_SYNTAX, 1, parsing.tline );
      }
   }
   /*
    * Execute any RXTER system exit
    */
   if ( doTermHook )
   {
      /*
       * Be sure to try it only once.
       */
      doTermHook = 0;
      if ( TSD->systeminfo->hooks & HOOK_MASK( HOOK_TERMIN ) )
         hookup( TSD, HOOK_TERMIN );
   }
   if ( TSD->systeminfo->hooks & HOOK_MASK( HOOK_TERMIN ) )
      hookup( TSD, HOOK_TERMIN );

   tmpsys = TSD->systeminfo;
   TSD->systeminfo = TSD->systeminfo->previous;
   TSD->currlevel = oldlevel;
   TSD->trace_stat = TSD->currlevel->tracestat;

   tmpsys->currlevel0->args = NULL;

   /*
    * jbuf will be freed by killsystem
    */
   killsystem( TSD, tmpsys );

   /*
    * If the called routine failed exit with "external routine failed"
    * depending on OPTION HALT_ON_EXT_CALL_FAIL or with STRICT_ANSI
    */
   if ( iserror)
   {
      if ( get_options_flag( TSD->currlevel, EXT_HALT_ON_EXT_CALL_FAIL )
      ||   get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
      {
         char *cmd = (char *) tmpstr_of( TSD, command );
         exiterror( 40, 1, cmd);
      }
   }

   RestoreInterpreterStatus( TSD, InterpreterStatus );
   /*
    * Oops, we really ought to handle function-did-not-return-data
    */
   return ptr;
}


internal_parser_type enter_macro( tsd_t *TSD, const streng *source,
                                  void **ept, unsigned long *extlength)
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
