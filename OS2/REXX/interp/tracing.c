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
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif
#include <string.h>


typedef struct { /* tra_tsd: static variables of this module (thread-safe) */
   int  traceflag;   /* boolean: 1 indicates that trace output is NOT displayed */
   int  lasttracedline;
   int  intercount;  /* number of times to execute trace without interaction */
   int  quiet;       /* boolean: run quietly in interaction trace */
   int  notnow;
   char tracestr[BUFFERSIZE+100];
   char buf0[32];
   int  bufptr0;
   char tracefmt[50];
   int  initialhtmltracing; /* set to 1 after first line of HTML tracing set */
} tra_tsd_t; /* thread-specific but only needed by this module. see
              * init_tracing
              */

PROTECTION_VAR( trace_setting )

/* init_tracing initializes the module.
 * Currently, we set up the thread specific data.
 * The function returns 1 on success, 0 if memory is short.
 */
int init_tracing( tsd_t *TSD )
{
   tra_tsd_t *tt;

   if ( TSD->tra_tsd != NULL )
      return 1;

   if ( ( TSD->tra_tsd = MallocTSD( sizeof( tra_tsd_t ) ) ) == NULL )
      return 0;
   tt = (tra_tsd_t *)TSD->tra_tsd;
   memset( tt, 0, sizeof( tra_tsd_t ) );
   tt->lasttracedline = -1;
   return 1;
}

int purge_tracing( tsd_t *TSD )
{
   if ( TSD->tra_tsd )
   {
      FreeTSD( TSD->tra_tsd );
      TSD->tra_tsd = NULL;
   }
   return 0;
}

int pushcallstack( const tsd_t *TSD, treenode *thisptr )
{
   nodeptr *tmpptr;
   tra_tsd_t *tt;

   tt = (tra_tsd_t *)TSD->tra_tsd;
   if ( TSD->systeminfo->cstackcnt >= TSD->systeminfo->cstackmax )
   {
      assert( TSD->systeminfo->cstackcnt == TSD->systeminfo->cstackmax );
      tmpptr = (nodeptr *)MallocTSD( ( TSD->systeminfo->cstackmax * 2 + 10 ) *
                                                           sizeof( nodeptr ) );
      if ( TSD->systeminfo->callstack )
      {
         memcpy( tmpptr, TSD->systeminfo->callstack,
                              TSD->systeminfo->cstackcnt * sizeof( nodeptr ) );
         FreeTSD( TSD->systeminfo->callstack );
      }
      TSD->systeminfo->callstack = tmpptr;
      TSD->systeminfo->cstackmax *= 2;
      TSD->systeminfo->cstackmax += 10;
   }

   TSD->systeminfo->callstack[TSD->systeminfo->cstackcnt++] = thisptr;
   return TSD->systeminfo->cstackcnt;
}

void popcallstack( const tsd_t *TSD, int value )
{
   tra_tsd_t *tt;

   tt = (tra_tsd_t *)TSD->tra_tsd;
   if ( value >= 0 )
   {
      assert( TSD->systeminfo->cstackcnt >= value );
      TSD->systeminfo->cstackcnt = value;
   }

   --TSD->systeminfo->cstackcnt;

   assert( TSD->systeminfo->cstackcnt >= 0 );
}

static void printout( tsd_t *TSD, const streng *message )
{
   int rc;
   FILE *fp=stderr;
   FILE *myfp=NULL;
   char *rxtrace=NULL;
   tra_tsd_t *tt;

   tt = (tra_tsd_t *)TSD->tra_tsd;

   rc = HOOK_GO_ON;
   if ( TSD->systeminfo->hooks & HOOK_MASK( HOOK_STDERR ) )
      rc = hookup_output( TSD, HOOK_STDERR, message );

   if ( rc == HOOK_GO_ON )
   {
      if ( get_options_flag( TSD->currlevel, EXT_STDOUT_FOR_STDERR ) )
         fp = stdout;
      rxtrace = getenv( "RXTRACE" );
      if ( rxtrace != NULL )
      {
         myfp = fopen( rxtrace, "a" );
         if ( myfp != NULL )
            fp = myfp;
      }
      if ( get_options_flag( TSD->currlevel, EXT_TRACE_HTML ) )
      {
         if ( tt->initialhtmltracing == 0 )
         {
            tt->initialhtmltracing = 1;
            fwrite( "Content-Type: text/html\n\n", 25, 1, fp );
         }
         fwrite( "<FONT COLOR=#669933><PRE>", 25, 1, fp );
      }
      fwrite( message->value, message->len, 1, fp ) ;
      if ( get_options_flag( TSD->currlevel, EXT_TRACE_HTML ) )
         fwrite( "</PRE></FONT>", 13, 1, fp );
#if defined(DOS) || defined(OS2) || defined(WIN32)
      /*
       * stdout is open in binary mode, so we need to add the
       * extra CR to the end of the line.
       */
      fputc( REGINA_CR, fp );
#endif
      fputc( REGINA_EOL, fp );
      fflush( fp );
      if ( myfp )
         fclose( fp );
   }
}

/*
 * Only called if return code from command to environment is NOT zero
 */
void traceerror( tsd_t *TSD, const treenode *thisptr, int RC )
{
   streng *message;
   /* Fix for Bug #434; ANSI 8.3.5 */
   if ( ( TSD->trace_stat == 'E' ) || ( RC < 0 && ( TSD->trace_stat == 'F' || TSD->trace_stat == 'N' ) ) )
   {
      traceline( TSD, thisptr, 'C', 0 );
      message = Str_makeTSD( 20 + sizeof( int ) * 3 ) ;
      message->len = sprintf( message->value, "       +++ RC=%d +++", RC );

      printout( TSD, message );
      Free_stringTSD( message );
   }
}

void tracecompound( tsd_t *TSD, const streng *stem, int length,
                    const streng *index, char trch )
{
   streng *message;
   tra_tsd_t *tt;
   int indent;

   tt = (tra_tsd_t *)TSD->tra_tsd;
   if ( tt->traceflag || ( TSD->trace_stat != 'I' ) || tt->quiet )
      return;

   indent = TSD->systeminfo->cstackcnt + TSD->systeminfo->ctrlcounter;
   message = Str_makeTSD( stem->len + index->len + 30 + indent );

   sprintf( tt->tracestr, "       >%c> %%%ds  \"%%.%ds.%%.%ds\"",
                          trch, indent, length, index->len );
   message->len = sprintf( message->value, tt->tracestr,
                                           "", stem->value, index->value );

   printout( TSD, message );
   Free_stringTSD( message );
}

void starttrace( const tsd_t *TSD )
{
   tra_tsd_t *tt;

   tt = (tra_tsd_t *)TSD->tra_tsd;
   tt->traceflag = 0;
   tt->notnow = 1;
}

static void tracemsg( tsd_t *TSD )
{
   streng *message;
   const streng *msg;

   msg = errortext( TSD, 0, 3, 0, 0 );
   message = Str_makeTSD( 12 + Str_len( msg ) );
   Str_catstrTSD( message, "       +++ " );
   Str_catTSD( message, msg );
   printout( TSD, message );
   Free_stringTSD( message );
}

int intertrace( tsd_t *TSD )
{
   streng *str=NULL;
   int retvalue1,rc;
   tra_tsd_t *tt;

   tt = (tra_tsd_t *)TSD->tra_tsd;

   if ( tt->intercount )
   {
      tt->intercount -= 1;
      if ( tt->intercount == 0 )
      {
         tt->quiet = 0;
         tt->traceflag = 0;
      }
      else
         return 0;
   }

   if ( tt->traceflag )
      return 0;

   if ( tt->notnow == 1 )
   {
      tt->notnow = 2;
      return 0;
   }
   else if ( tt->notnow == 2 )
   {
      tt->notnow = 0;
      tracemsg( TSD );
   }
   tt->traceflag = 1;
   retvalue1 = -1;

   for ( ; retvalue1 < 0; )
   {
      rc = HOOK_GO_ON;
      if ( TSD->systeminfo->hooks & HOOK_MASK( HOOK_TRCIN ) )
         rc = hookup_input( TSD, HOOK_TRCIN, &str );

      if ( rc == HOOK_GO_ON )
         str = readkbdline( TSD );

      if ( str->len == 0 )
      {
         tt->traceflag = 0;
         retvalue1 = 0;
      }

      if ( ( Str_len( str ) == 1 ) && (str->value[0] == '=' ) )
      {
         tt->traceflag = 0;
         retvalue1 = 1;
      }
      else if ( str->len )
      {
         dointerpret( TSD, str );
         if ( !TSD->systeminfo->interactive )
         {
            tt->intercount = tt->quiet = 0;
            return 0;
         }
         if ( tt->intercount )
         {
            if ( tt->quiet )
               tt->traceflag = 1;
            else
               tt->traceflag = 0;
            return 0;
         }
      }
   }

   return retvalue1;
}

void tracenumber( tsd_t *TSD, const num_descr *num, char type )
{
   char tmpch;
   streng *message,*tmpstr;
   tra_tsd_t *tt;
   num_descr nd;
   int indent;

   tmpch = TSD->currlevel->tracestat;
   if ( ( tmpch != 'I' ) && ( tmpch != 'R' ) )
      return;

   tt = (tra_tsd_t *)TSD->tra_tsd;
   if ( tt->traceflag || tt->quiet )
      return;

   memset( &nd, 0, sizeof( num_descr ) );
   descr_copy( TSD, num, &nd );
   tmpstr = str_norm( TSD, &nd, NULL );
   indent = TSD->systeminfo->cstackcnt + TSD->systeminfo->ctrlcounter;
   message = Str_makeTSD( 30 + indent + tmpstr->len );
   sprintf( tt->tracestr, "       >%%c> %%%ds  \"%%.%ds\"",
                          indent, tmpstr->len );
   message->len = sprintf( message->value, tt->tracestr,
                                           type, "", tmpstr->value );
   printout( TSD, message );
   if ( nd.num != NULL )
      FreeTSD( nd.num );
   Free_stringTSD( message );
   Free_stringTSD( tmpstr );
}

void tracebool( tsd_t *TSD, int value, char type )
{
   char tmpch;
   streng *message;
   tra_tsd_t *tt;
   int indent;

   tmpch = TSD->currlevel->tracestat;
   if ( ( tmpch != 'I' ) && ( tmpch != 'R' ) )
      return;

   tt = (tra_tsd_t *)TSD->tra_tsd;
   if ( tt->traceflag || tt->quiet )
      return;

   indent = TSD->systeminfo->cstackcnt + TSD->systeminfo->ctrlcounter;
   message = Str_makeTSD( 35 + indent );
   sprintf( tt->tracestr, "       >%%c> %%%ds  \"%%d\"",
                          indent );
   message->len = sprintf( message->value, tt->tracestr,
                                           type, "", value );
   printout( TSD, message );
   Free_stringTSD( message );
}

void tracevalue( tsd_t *TSD, const streng *str, char type )
{
   char tmpch;
   streng *message;
   tra_tsd_t *tt;
   int indent,i;

   /*
    * ANSI 8.3.17 requires placeholders in PARSE to be traced with TRACE R
    */
   tmpch = TSD->currlevel->tracestat;
   if ( ( tmpch != 'I' ) && ( tmpch != 'R' ) )
      return;

   tt = (tra_tsd_t *)TSD->tra_tsd;
   if ( tt->traceflag || tt->quiet )
      return;

   indent = TSD->systeminfo->cstackcnt + TSD->systeminfo->ctrlcounter;
   message = Str_makeTSD( str->len + 30 + indent );
   sprintf( tt->tracestr, "       >%c> %%%ds  \"", type, indent );
   message->len = sprintf( message->value, tt->tracestr, "" );
   /* bug #279 if we encounter a nul value in the value to be displayed, show a space instead */
   for (i=0;i<str->len;i++)
   {
      if ( str->value[i] == '\0' )
         message->value[message->len++] = ' ';
      else
         message->value[message->len++] = str->value[i];
   }
   message->value[message->len++] = '"';

   printout( TSD, message );
   Free_stringTSD( message );
}

void traceline( tsd_t *TSD, const treenode *thisptr, char tch, int offset )
{
   streng *srcstr;
   streng *message;
   tra_tsd_t *tt;
   int indent;

   tt = (tra_tsd_t *)TSD->tra_tsd;
   if ( tt->traceflag || tt->quiet )
      return;

   if ( ( thisptr->charnr < 0 ) || ( thisptr->lineno < 0 ) )
      return;

   switch ( tch )
   {
      case 'I':
      case 'R':
      case 'A':
         break; /* Oh yes, break the IRA ;-) */

      case 'L':
         if ( thisptr->type == X_LABEL )
            break;
         return;

      case 'C':
         if ( ( thisptr->type == X_COMMAND )
           || ( ( thisptr->type == X_ADDR_N ) && thisptr->p[0] ) )
            break;
         return;

      default:
         return;
   }

   srcstr = getsourceline( TSD, thisptr->lineno, thisptr->charnr,
                                                     &TSD->systeminfo->tree );

   indent = TSD->systeminfo->cstackcnt + TSD->systeminfo->ctrlcounter;
   message = Str_makeTSD( indent + 20 + srcstr->len + offset );
   if ( thisptr->lineno == tt->lasttracedline )
   {
      sprintf( tt->tracestr, "       *-* %%%ds%%.%ds",
                             indent + offset, srcstr->len );
      message->len = sprintf( message->value, tt->tracestr,
                                              "", srcstr->value );
   }
   else
   {
      sprintf( tt->tracestr, "%%6d *-* %%%ds%%.%ds",
                             indent + offset, srcstr->len );
      message->len = sprintf( message->value, tt->tracestr,
                                              thisptr->lineno, "", srcstr->value );
   }

   printout( TSD, message );
   tt->lasttracedline = thisptr->lineno;
   Free_stringTSD( message );
   Free_stringTSD( srcstr );
}

void traceback( tsd_t *TSD )
{
   sysinfo ss;
   streng *message;
   streng *srcline;
   nodeptr ptr;
   int i,j,linesize=128,indent;
   tra_tsd_t *tt;

   tt = (tra_tsd_t *)TSD->tra_tsd;
   /*
    * Allocate enough space for one line and control stuff. Count below
    * characters for the needed size. Beware of the computed format
    * entry.
    */
   indent = TSD->systeminfo->cstackcnt + TSD->systeminfo->ctrlcounter;
   message = Str_makeTSD( linesize + indent * 3 + 20 );
   if ( TSD->currentnode )
   {
      srcline = getsourceline( TSD,
                               TSD->currentnode->lineno,
                               TSD->currentnode->charnr,
                               &TSD->systeminfo->tree );
      if ( srcline->len > linesize )
      {
         Free_stringTSD( message );
         linesize = srcline->len;
         message = Str_makeTSD( linesize + indent * 3 + 20 );
      }
      sprintf( tt->tracefmt, "%%6d +++ %%%ds%%.%ds",
                             indent * 3, srcline->len );
      message->len = sprintf( message->value, tt->tracefmt,
                                              TSD->currentnode->lineno, "",
                                                             srcline->value );
      printout( TSD, message );
      Free_stringTSD( srcline );
   }

   j = indent;
   for ( ss = TSD->systeminfo; ss; ss = ss->previous )
   {
      for ( i = ss->cstackcnt - 1; i >= 0; i-- )
      {
         ptr = ss->callstack[i];
         if ( !ptr )
            continue;

         srcline = getsourceline( TSD,
                                  ptr->lineno,
                                  ptr->charnr,
                                  &ss->tree );
         if ( srcline->len > linesize )
         {
            Free_stringTSD( message );
            linesize = srcline->len;
            message = Str_makeTSD( linesize + indent * 3 + 20 );
         }

         j--;
         if ( ( j > 12 ) && get_options_flag( TSD->currlevel, EXT_PRUNE_TRACE ) )
            sprintf( tt->tracefmt, "%%6d +++ [...] %%%ds%%.%ds",
                                   30, srcline->len );
         else
            sprintf( tt->tracefmt, "%%6d +++ %%%ds%%.%ds",
                                   j * 3, srcline->len );
         message->len = sprintf( message->value, tt->tracefmt,
                                                 ptr->lineno, "",
                                                             srcline->value );
         printout( TSD, message );
         Free_stringTSD( srcline );
      }
   }
   Free_stringTSD( message );
}

/*
 * Get or show call stack
 * If stem is NULL, display to stderr
 */
void getcallstack( tsd_t *TSD, streng *stem )
{
   sysinfo ss;
   nodeptr ptr;
   int i,j=0;
   streng *varname=NULL, *value=NULL, *lineno=NULL;
   int stemlen=0 ;
   char *eptr=NULL ;
   streng *tmpptr=NULL ;

   if ( stem )
   {
      varname = Str_makeTSD( (stemlen=stem->len) + 8 ) ;
      memcpy( varname->value, stem->value, stemlen ) ;
      mem_upper( varname->value, stemlen );
      eptr = varname->value + stemlen ;

      if (*(eptr-1)!='.')
      {
         *((eptr++)-1) = '.' ;
         stemlen++ ;
      }
   }
   for ( ss = TSD->systeminfo; ss; ss = ss->previous )
   {
      for ( i = ss->cstackcnt - 1; i >= 0; i-- )
      {
         ptr = ss->callstack[i];
         if ( !ptr )
            continue;
         if ( !ptr->name )
            continue;
         tmpptr = ptr->name;
         /* get the value; lineno name */
         lineno = int_to_streng( TSD, ptr->lineno );
         if ( stem )
         {
            value = Str_makeTSD( (tmpptr->len) + 10 ) ; /* should not be more than 999999999 levels in the call stack */
            memcpy( value->value, lineno->value, lineno->len ) ;
            value->len = lineno->len;
            Str_catstr_TSD( TSD, value, " " );
            Str_cat_TSD( TSD, value, tmpptr );
            Free_stringTSD( lineno );
            /* set the tail value */
            sprintf(eptr, "%d", ++j ) ;
            varname->len = strlen( varname->value ) ;
            setvalue( TSD, varname, value, -1 ) ;
         }
         else
         {
            fprintf( stderr, "%.*s %.*s\n", lineno->len, lineno->value, tmpptr->len, tmpptr->value );
         }
      }
   }
   if ( stem )
   {
      *eptr = '0' ;
      varname->len = stemlen+1 ;
      tmpptr = int_to_streng( TSD, j ) ;
      setvalue( TSD, varname, tmpptr, -1 ) ;
      Free_stringTSD( varname );
   }
}

void queue_trace_char( const tsd_t *TSD, char ch2 )
{
   tra_tsd_t *tt;

   tt = (tra_tsd_t *)TSD->tra_tsd;
   if ( tt->bufptr0 < 32 )
      tt->buf0[tt->bufptr0++] = ch2;
   else
      exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__,
                                               "too many tracechars queued" );
}

void flush_trace_chars( tsd_t *TSD )
{
   int cnt;
   tra_tsd_t *tt;

   tt = (tra_tsd_t *)TSD->tra_tsd;
   for ( cnt = 0; cnt < tt->bufptr0; cnt++ )
      set_trace_char( TSD, tt->buf0[cnt] );

   tt->bufptr0 = 0;
}

void set_trace_char( tsd_t *TSD, char ch2 )
{
   ch2 = (char) rx_toupper( ch2 );
   switch ( ch2 )
   {
      case '?':
         TSD->systeminfo->interactive = !TSD->systeminfo->interactive;
         TSD->currlevel->traceint = (char) TSD->systeminfo->interactive;
         if ( TSD->systeminfo->interactive )
            starttrace( TSD );
         break ;

      case 'A':
      case 'C':
      case 'E':
      case 'F':
      case 'I':
      case 'L':
      case 'N':
      case 'O':
      case 'R':
         TSD->currlevel->tracestat = ch2;
         break;

      default:
         exiterror( ERR_INVALID_TRACE, 1, "ACEFILNOR", ch2 );
   }

   if ( ch2 == 'O' )
      TSD->systeminfo->interactive = TSD->currlevel->traceint = 0;
   TSD->trace_stat = TSD->currlevel->tracestat;
}

void set_trace( tsd_t *TSD, const streng *setting )
{
   int cptr,error;
   tra_tsd_t *tt;

   THREAD_PROTECT( trace_setting )

   if ( myisnumber( TSD, setting ) )
   {
      cptr = streng_to_int( TSD, setting, &error );
      if ( error )
         exiterror( ERR_INVALID_INTEGER, 7, tmpstr_of( TSD, setting ) );

      /*
       * If the number is positive, interactive tracing continues
       * for the supplied number of clauses, but no pausing is done.
       * If the number is negative, no trace output is inhibited
       * (as is the pauses) for the supplied number of clauses.
       * If the number is zero, this is the same as TRACE OFF
       */
      tt = (tra_tsd_t *)TSD->tra_tsd;
      if ( cptr == 0 )
      {
         TSD->currlevel->tracestat = 'O';
         TSD->systeminfo->interactive = 0;
         TSD->currlevel->traceint = 0;
         TSD->trace_stat = TSD->currlevel->tracestat;
      }
      else if ( cptr > 0 )
      {
         tt->quiet = 0;
         tt->intercount = cptr + 1;
      }
      else
      {
         tt->quiet = 1;
         tt->intercount = -cptr + 1;
      }
   }
   else
   {
      for ( cptr = 0; cptr < Str_len( setting ); cptr++ )
      {
         /*
          * Check each character in the string. Toggle interactive trace for
          * each ? until there is an alpha character in the string when we
          * stop processing the string.  So if you TRACE '???IGNORE'
          * and interactive tracing is currently off, this will turn interactive
          * tracing on (intermediates) and ignore the remaining string 'GNORE'
          */
         set_trace_char( TSD, setting->value[cptr] );
         if ( rx_isalpha( setting->value[cptr] ) )
            break;
      }
   }

   THREAD_UNPROTECT( trace_setting )

   return;
}
