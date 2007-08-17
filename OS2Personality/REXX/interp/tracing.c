#ifndef lint
static char *RCSid = "$Id: tracing.c,v 1.2 2003/12/11 04:43:24 prokushev Exp $";
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
   int  ctrlcounter;
   int  intercount;  /* number of times to execute trace without interaction */
   int  quiet;       /* boolean: run quietly in interaction trace */
   int  notnow;
   char tracestr[LINELENGTH+1] ;
   char buf0[32] ;
   int  bufptr0 ;
   char tracefmt[20] ;
} tra_tsd_t; /* thread-specific but only needed by this module. see
              * init_tracing
              */


/* init_tracing initializes the module.
 * Currently, we set up the thread specific data.
 * The function returns 1 on success, 0 if memory is short.
 */
int init_tracing( tsd_t *TSD )
{
   tra_tsd_t *tt;

   if (TSD->tra_tsd != NULL)
      return(1);

   if ((tt = TSD->tra_tsd = MallocTSD(sizeof(tra_tsd_t))) == NULL)
      return(0);
   memset(tt,0,sizeof(tra_tsd_t));
   tt->lasttracedline = -1 ;
   return(1);
}

int pushcallstack( const tsd_t *TSD, treenode *this )
{
   nodeptr *tmpptr=NULL ;
   tra_tsd_t *tt;

   tt = TSD->tra_tsd;
   if (TSD->systeminfo->cstackcnt >= TSD->systeminfo->cstackmax)
   {
      assert( TSD->systeminfo->cstackcnt == TSD->systeminfo->cstackmax ) ;
      tmpptr = MallocTSD( (TSD->systeminfo->cstackmax*2+10)*sizeof(nodeptr)) ;
      if (TSD->systeminfo->callstack)
      {
         memcpy( tmpptr, TSD->systeminfo->callstack,
                                 TSD->systeminfo->cstackcnt*sizeof(nodeptr) ) ;
         FreeTSD( TSD->systeminfo->callstack ) ;
      }
      TSD->systeminfo->callstack = tmpptr ;
      TSD->systeminfo->cstackmax *= 2 ;
      TSD->systeminfo->cstackmax += 10 ;
   }

   tt->ctrlcounter++ ;
   TSD->systeminfo->callstack[TSD->systeminfo->cstackcnt++] = this ;
   return TSD->systeminfo->cstackcnt ;
}

void popcallstack( const tsd_t *TSD, int value )
{
   tra_tsd_t *tt;

   tt = TSD->tra_tsd;
   if (value>=0)
   {
      assert( TSD->systeminfo->cstackcnt >= value ) ;
      tt->ctrlcounter -= TSD->systeminfo->cstackcnt - value ;
      TSD->systeminfo->cstackcnt = value ;
   }

   tt->ctrlcounter-- ;
   --(TSD->systeminfo->cstackcnt) ;

   assert( TSD->systeminfo->cstackcnt >= 0 ) ;
   assert( tt->ctrlcounter >= 0 ) ;
}

static void printout( tsd_t *TSD, const streng *message )
{
   int rc=0 ;
   FILE *fp=stderr;

   rc = HOOK_GO_ON ;
   if (TSD->systeminfo->hooks & HOOK_MASK(HOOK_STDERR))
      rc = hookup_output( TSD, HOOK_STDERR, message ) ;

   if (rc==HOOK_GO_ON)
   {
      if ( get_options_flag( TSD->currlevel, EXT_STDOUT_FOR_STDERR ) )
         fp = stdout;
      if ( get_options_flag( TSD->currlevel, EXT_TRACE_HTML ) )
         fwrite( "<FONT COLOR=#669933><PRE>", 25, 1, fp );
      fwrite( message->value, message->len, 1, fp ) ;
      if ( get_options_flag( TSD->currlevel, EXT_TRACE_HTML ) )
         fwrite( "</PRE></FONT>", 13, 1, fp );
#if defined(DOS) || defined(OS2) || defined(WIN32)
      /*
       * stdout is open in binary mode, so we need to add the
       * extra CR to the end of the line.
       */
      fputc( REGINA_CR, fp ) ;
#endif
      fputc( REGINA_EOL, fp ) ;
      fflush( fp ) ;
   }
}

void traceerror( tsd_t *TSD, const treenode *this, int RC )
{
   streng *message=NULL ;

   if ( TSD->trace_stat == 'N' || TSD->trace_stat == 'F' )
      traceline(TSD, this, 'C', 0) ;

   if (TSD->trace_stat!='O')
   {
      message = Str_makeTSD( 20+sizeof(int)*3 ) ;
      sprintf(message->value, "       +++ RC=%d +++",RC) ;
      message->len = strlen( message->value ) ;

      printout( TSD, message ) ;
      Free_stringTSD( message ) ;
   }
}

void tracecompound( tsd_t *TSD, const streng *stem, int length, const streng *index, char trch )
{
   streng *message=NULL ;
   tra_tsd_t *tt;

   tt = TSD->tra_tsd;
   if ( (tt->traceflag) || (TSD->trace_stat!='I') || tt->quiet )
      return ;

   message = Str_makeTSD( stem->len + index->len + 30 + tt->ctrlcounter ) ;

   sprintf(tt->tracestr,"       >%c> %%%ds  \"%%.%ds.%%.%ds\"",trch,
                                        tt->ctrlcounter,length, index->len) ;
   sprintf(message->value, tt->tracestr,"",stem->value,index->value ) ;

   message->len = strlen( message->value ) ;
   printout( TSD, message ) ;

   Free_stringTSD( message ) ;
}

void starttrace( const tsd_t *TSD )
{
   tra_tsd_t *tt;

   tt = TSD->tra_tsd;
   tt->traceflag = 0;
   tt->notnow = 1 ;
}

static void tracemsg( tsd_t *TSD )
{
   streng *message;
   const streng *msg;

   msg = errortext( TSD, 0, 3, 0, 0 );
   message = Str_makeTSD( 12 + Str_len( msg ) );
   Str_catstrTSD( message, "       +++ " );
   Str_catTSD( message, msg );
   printout( TSD, message ) ;
   Free_stringTSD( message ) ;
}

int intertrace( tsd_t *TSD )
{
   streng *str=NULL ;
   int retvalue1=0, rc=0 ;
   tra_tsd_t *tt;

   tt = TSD->tra_tsd;

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
      return 0 ;

   if (tt->notnow==1)
   {
      tt->notnow = 2 ;
      return 0 ;
   }
   else if (tt->notnow==2)
   {
      tt->notnow = 0 ;
      tracemsg(TSD) ;
   }
   tt->traceflag = 1 ;
   retvalue1 = (-1) ;

   for (; retvalue1<0; )
   {
      rc = HOOK_GO_ON ;
      if (TSD->systeminfo->hooks & HOOK_MASK(HOOK_TRCIN))
         rc = hookup_input( TSD, HOOK_TRCIN, &str ) ;

      if (rc==HOOK_GO_ON)
         str = readkbdline( TSD ) ;

      if (str->len==0)
      {
         tt->traceflag = 0 ;
         retvalue1 = 0 ;
      }

      if ((Str_len(str)==1)&&(str->value[0]=='='))
      {
         tt->traceflag = 0 ;
         retvalue1 = 1 ;
      }
      else if (str->len)
      {
         dointerpret( TSD, str ) ;
         if (!TSD->systeminfo->interactive)
         {
            tt->intercount = tt->quiet = 0;
            return 0 ;
         }
         if ( tt->intercount )
         {
            if ( tt->quiet )
               tt->traceflag = 1 ;
            else
               tt->traceflag = 0 ;
            return 0;
         }
      }
   }

   return retvalue1 ;
}

void tracenumber( tsd_t *TSD, const num_descr *num, char type )
{
   char tmpch=' ' ;
   streng *message=NULL, *tmpstr=NULL ;
   tra_tsd_t *tt;
   num_descr nd;

   tt = TSD->tra_tsd;
   if ( tt->traceflag || tt->quiet )
      return ;

   tmpch = TSD->currlevel->tracestat ;
   if ((tmpch=='I')||((tmpch=='R')&&(type!='.')))
   {
      memset(&nd,0,sizeof(num_descr));
      descr_copy(TSD, num, &nd);
      tmpstr = str_norm( TSD, &nd, NULL ) ;
      message = Str_makeTSD( 30 + tt->ctrlcounter + tmpstr->len ) ;
      sprintf(tt->tracestr,"       >%%c> %%%ds  \"%%.%ds\"",tt->ctrlcounter,tmpstr->len) ; /* bja */
      sprintf(message->value,tt->tracestr,type,"", tmpstr->value) ; /* bja */
      message->len = strlen( message->value ) ;
      printout( TSD, message ) ;
      if (nd.num != NULL)
         FreeTSD(nd.num);
      Free_stringTSD( message ) ;
      Free_stringTSD( tmpstr ) ;
   }
}

void tracebool( tsd_t *TSD, int value, char type )
{
   char tmpch=' ' ;
   streng *message=NULL ;
   tra_tsd_t *tt;

   tt = TSD->tra_tsd;
   if ( tt->traceflag || tt->quiet )
      return ;

   tmpch = TSD->currlevel->tracestat ;
   if ((tmpch=='I') || ((tmpch=='R') && (type!='.')))
   {
      message = Str_makeTSD( 35 + tt->ctrlcounter ) ;
      sprintf(tt->tracestr,"       >%%c> %%%ds  \"%%d\"",tt->ctrlcounter ) ;
      sprintf(message->value,tt->tracestr,type,"",value) ;
      message->len = strlen( message->value ) ;
      printout( TSD, message ) ;
      Free_stringTSD( message ) ;
   }
}

void tracevalue( tsd_t *TSD, const streng *str, char type )
{
   char tmpch=' ' ;
   streng *message=NULL ;
   tra_tsd_t *tt;

   tt = TSD->tra_tsd;
   if ( tt->traceflag || tt->quiet )
      return ;

   tmpch = TSD->currlevel->tracestat ;
   /*
    * ANSI 8.3.17 requires placeholders in PARSE to be traced with TRACE R
    */
   if ( ( tmpch == 'I' ) || ( tmpch == 'R' ) )
   {
      message = Str_makeTSD( str->len + 30 + tt->ctrlcounter ) ;
      sprintf(tt->tracestr,"       >%%c> %%%ds  \"%%.%ds\"",tt->ctrlcounter, str->len) ; /* bja */
      sprintf(message->value,tt->tracestr,type,"", str->value) ; /* bja */
      message->len = strlen( message->value ) ;
      printout( TSD, message ) ;
      Free_stringTSD( message ) ;
   }
}

void traceline( tsd_t *TSD, const treenode *this, char tch, int offset )
{
   streng *srcstr=NULL ;
   streng *message ;
   tra_tsd_t *tt;

   tt = TSD->tra_tsd;
   if ( (tch=='O') || (tt->traceflag) || tt->quiet )
      return ;

   if ((this->charnr<0)||(this->lineno<0))
      return ;

   if ((tch=='I')||(tch=='R')||(tch=='A')) goto trace ;
   if ((tch=='L')&&(this->type==X_LABEL)) goto trace ;
   if (tch=='C')
      if ((this->type==X_COMMAND) ||
          ((this->type==X_ADDR_N) && (this->p[0])))
         goto trace ;

   return ;

trace:
   srcstr = getsourceline(TSD,this->lineno,this->charnr,
                          &TSD->systeminfo->tree) ;

   message = Str_makeTSD( tt->ctrlcounter+20+srcstr->len+offset ) ;
   if (this->lineno==tt->lasttracedline) {
      sprintf(tt->tracestr,"       *-* %%%ds%%.%ds",tt->ctrlcounter+offset,srcstr->len) ; /* bja */
      sprintf(message->value,tt->tracestr,"", srcstr->value) ; } /* bja */
   else {
      sprintf(tt->tracestr,"%%6d *-* %%%ds%%.%ds",tt->ctrlcounter+offset, srcstr->len) ; /* bja */
      sprintf(message->value,tt->tracestr,this->lineno,"", srcstr->value) ; } /* bja */

   message->len = strlen( message->value ) ;
   printout( TSD, message ) ;
   tt->lasttracedline = this->lineno ;
   Free_stringTSD( message ) ;
   Free_stringTSD( srcstr ) ;
}

void traceback( tsd_t *TSD )
{
   sysinfo ss ;
   streng *message=NULL ;
   streng *srcline=NULL;
   nodeptr ptr=NULL ;
   int i=0, j=0 ;
   tra_tsd_t *tt;

   tt = TSD->tra_tsd;
   /* Allocate enough space for one line and control stuff. Count below
    * characters for the needed size. Beware of the computed format
    * entry.
    */
   message = Str_makeTSD( 512 + tt->ctrlcounter*3 + 20 ) ;
   if (TSD->currentnode)
   {
/*    sprintf(tt->tracefmt,"%%6d +++ %%%ds%%s", tt->ctrlcounter*3 ) ; */
      srcline = getsourceline(TSD,
                              TSD->currentnode->lineno,
                              TSD->currentnode->charnr,
                              &TSD->systeminfo->tree) ;
      sprintf(tt->tracefmt,"%%6d +++ %%%ds%%s", tt->ctrlcounter*3 ) ;
      sprintf(message->value,
              tt->tracefmt,
              TSD->currentnode->lineno,
              "",
              srcline->value );
      message->len = strlen( message->value ) ;
      printout( TSD, message ) ;
      Free_stringTSD( srcline ) ;
   }

   j = tt->ctrlcounter ;
   for (ss=TSD->systeminfo; ss; ss=ss->previous )
   {
      for (i=ss->cstackcnt-1;i>=0;i--)
      {
         ptr = ss->callstack[i] ;
         if (!ptr)
            continue ;
         if ( -- j > 12
         && get_options_flag( TSD->currlevel, EXT_PRUNE_TRACE ) )
            sprintf(tt->tracefmt,"%%6d +++ [...] %%%ds%%s", 30 ) ;
         else
            sprintf(tt->tracefmt,"%%6d +++ %%%ds%%s", (j)*3 ) ;
         srcline = getsourceline(TSD,
                                 ptr->lineno,
                                 ptr->charnr,
                                 &ss->tree) ;
         sprintf(message->value,
                 tt->tracefmt,ptr->lineno,
                 "",
                 srcline->value );

         message->len = strlen( message->value ) ;
         printout( TSD, message ) ;
         Free_stringTSD( srcline ) ;
      }
   }
   Free_stringTSD( message ) ;
}

void queue_trace_char( const tsd_t *TSD, char ch2 )
{
   tra_tsd_t *tt;

   tt = TSD->tra_tsd;
   if (tt->bufptr0<32)
      tt->buf0[tt->bufptr0++]=ch2 ;
   else
      exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
}

void flush_trace_chars( tsd_t *TSD )
{
   int cnt=0 ;
   tra_tsd_t *tt;

   tt = TSD->tra_tsd;
   for (cnt=0; cnt<tt->bufptr0; cnt++)
      set_trace_char( TSD, tt->buf0[cnt] ) ;

   tt->bufptr0 = 0 ;
}

void set_trace_char( tsd_t *TSD, char ch2 )
{
   switch (ch2=(char)toupper(ch2))
   {
      case '?':
         ch2 = (char) (TSD->systeminfo->interactive = !TSD->systeminfo->interactive) ;
         TSD->currlevel->traceint = (char) TSD->systeminfo->interactive ; /* MDW 30012002 */
         if (ch2)
            starttrace(TSD) ;
         break ;

      case 'F':
      case 'A':
      case 'C':
      case 'E':
      case 'I':
      case 'L':
      case 'N':
      case 'O':
      case 'R':
         TSD->currlevel->tracestat=ch2 ;
         break ;

      default:
         exiterror( ERR_INVALID_TRACE, 1, "ACEFILNOR", ch2 )  ;
   }

   if ( ch2 == 'O' )
      TSD->systeminfo->interactive = TSD->currlevel->traceint = 0 ; /* MDW 30012002 */
      /* MDW 30012002 - above line replaces this one TSD->systeminfo->interactive = 0 ; */
   TSD->trace_stat = TSD->currlevel->tracestat ;
}

void set_trace( tsd_t *TSD, const streng *setting )
{
   int cptr,error;
   tra_tsd_t *tt;

   if (myisnumber(setting))
   {
      cptr = streng_to_int( TSD, setting, &error );
      if (error)
         exiterror( ERR_INVALID_INTEGER, 7, tmpstr_of( TSD, setting ) );

      /*
       * If the number is positive, interactive tracing continues
       * for the supplied number of clauses, but no pausing is done.
       * If the number is negative, no trace output is inhibited
       * (as is the pauses) for the supplied number of clauses.
       * If the number is zero, this is the same as TRACE OFF
       */
      tt = TSD->tra_tsd;
      if ( cptr == 0 )
      {
         TSD->systeminfo->interactive = TSD->currlevel->traceint = 0 ; /* MDW 30012002 */
         /* MDW 30012002 - above line replaces this one TSD->currlevel->tracestat = 'O' ; */
         TSD->systeminfo->interactive = 0 ;
         TSD->trace_stat = TSD->currlevel->tracestat ;
      }
      else if ( cptr > 0 )
      {
         tt->quiet = 0;
         tt->intercount = cptr+1;
      }
      else
      {
         tt->quiet = 1;
         tt->intercount = (-cptr)+1;
      }
      return ;  /* need to do something else ... */
   }
   for (cptr=0; cptr<Str_len(setting); cptr++)
   {
      set_trace_char( TSD, setting->value[cptr] ) ;
      if (isalpha( setting->value[cptr]))
         return ;
   }
}
