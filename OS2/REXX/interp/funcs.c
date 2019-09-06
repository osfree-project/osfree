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
#include "rxiface.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

static streng *conflict_close( tsd_t *TSD, cparamboxptr parms );
static streng *conflict_eof( tsd_t *TSD, cparamboxptr parms );
static streng *conflict_open( tsd_t *TSD, cparamboxptr parms );

struct function_type
{
   int compat ;
   streng *(*function)(tsd_t *,cparamboxptr) ;
   const char *funcname ;
} ;

/*
 * A 0 in the first column of this table indicates that this BIF is ANSI.
 * Any other value
 */
static const struct function_type functions[] = {
  { 0,              std_abbrev,            "ABBREV" },
  { 0,              std_abs,               "ABS" },
  { 0,              std_address,           "ADDRESS" },
#ifdef TRACEMEM
  { EXT_REGINA_BIFS,dbg_allocated,         "ALLOCATED" },
#endif
  { 0,              std_arg,               "ARG" },
  { EXT_AREXX_BIFS, arexx_b2c,             "B2C" },
  { 0,              std_b2x,               "B2X" },
  { EXT_REGINA_BIFS,os2_beep,              "BEEP" },
  { 0,              std_bitand,            "BITAND" },
  { EXT_AREXX_BIFS, arexx_bitchg,          "BITCHG" },
  { EXT_AREXX_BIFS, arexx_bitclr,          "BITCLR" },
  { EXT_AREXX_BIFS, arexx_bitcomp,         "BITCOMP" },
  { 0,              std_bitor,             "BITOR" },
  { EXT_AREXX_BIFS, arexx_bitset,          "BITSET" },
  { EXT_AREXX_BIFS, arexx_bittst,          "BITTST" },
  { 0,              std_bitxor,            "BITXOR" },
  { EXT_BUFTYPE_BIF,cms_buftype,           "BUFTYPE" },
  { EXT_AREXX_BIFS, arexx_c2b,             "C2B" },
  { 0,              std_c2d,               "C2D" },
  { 0,              std_c2x,               "C2X" },
  { EXT_REGINA_BIFS,unx_chdir,             "CD" },
  { 0,              std_center,            "CENTER" },
  { 0,              std_center,            "CENTRE" },
  { 0,              std_changestr,         "CHANGESTR" }, /* ANSI Std 1996 - MH 10-06-96 */
  { 0,              std_charin,            "CHARIN" },
  { 0,              std_charout,           "CHAROUT" },
  { 0,              std_chars,             "CHARS" },
  { EXT_REGINA_BIFS,unx_chdir,             "CHDIR" },
  { EXT_REGINA_BIFS,conflict_close,        "CLOSE" },
  { 0,              std_compare,           "COMPARE" },
  { EXT_AREXX_BIFS, arexx_compress,        "COMPRESS" },
  { 0,              std_condition,         "CONDITION" },
  { 0,              std_copies,            "COPIES" },
  { 0,              std_countstr,          "COUNTSTR" },   /* ANSI Std 1996 - MH 10-06-96 */
  { EXT_REGINA_BIFS,unx_crypt,             "CRYPT" },
  { 0,              std_d2c,               "D2C" },
  { 0,              std_d2x,               "D2X" },
  { 0,              std_datatype,          "DATATYPE" },
  { 0,              std_date,              "DATE" },
  { 0,              std_delstr,            "DELSTR" },
  { 0,              std_delword,           "DELWORD" },
  { EXT_DESBUF_BIF, cms_desbuf,            "DESBUF" },
  { 0,              std_digits,            "DIGITS" },
  { EXT_REGINA_BIFS,os2_directory,         "DIRECTORY" },
  { EXT_DROPBUF_BIF,cms_dropbuf,           "DROPBUF" },
#ifndef NDEBUG
  { EXT_REGINA_BIFS,dbg_dumpfiles,         "DUMPFILES" },
  { EXT_REGINA_BIFS,dbg_dumptree,          "DUMPTREE" },
  { EXT_REGINA_BIFS,dbg_dumpvars,          "DUMPVARS" },
#endif
  { EXT_REGINA_BIFS,conflict_eof,          "EOF" },
  { 0,              std_errortext,         "ERRORTEXT" },
  { EXT_AREXX_BIFS, arexx_exists,          "EXISTS" },
  { EXT_AREXX_BIFS, arexx_export,          "EXPORT" },
#ifdef VMS
  { EXT_REGINA_BIFS,vms_f_cvsi,            "F$CVSI" },
  { EXT_REGINA_BIFS,vms_f_cvtime,          "F$CVTIME" },
  { EXT_REGINA_BIFS,vms_f_cvui,            "F$CVUI" },
  { EXT_REGINA_BIFS,vms_f_directory,       "F$DIRECTORY" },
  { EXT_REGINA_BIFS,vms_f_element,         "F$ELEMENT" },
  { EXT_REGINA_BIFS,vms_f_extract,         "F$EXTRACT" },
  { EXT_REGINA_BIFS,vms_f_fao,             "F$FAO" },
  { EXT_REGINA_BIFS,vms_f_file_attributes, "F$FILE_ATTRIBUTES" },
  { EXT_REGINA_BIFS,vms_f_getdvi,          "F$GETDVI" },
  { EXT_REGINA_BIFS,vms_f_getjpi,          "F$GETJPI" },
  { EXT_REGINA_BIFS,vms_f_getqui,          "F$GETQUI" },
  { EXT_REGINA_BIFS,vms_f_getsyi,          "F$GETSYI" },
  { EXT_REGINA_BIFS,vms_f_identifier,      "F$IDENTIFIER" },
  { EXT_REGINA_BIFS,vms_f_integer,         "F$INTEGER" },
  { EXT_REGINA_BIFS,vms_f_length,          "F$LENGTH" },
  { EXT_REGINA_BIFS,vms_f_locate,          "F$LOCATE" },
  { EXT_REGINA_BIFS,vms_f_logical,         "F$LOGICAL" },
  { EXT_REGINA_BIFS,vms_f_message,         "F$MESSAGE" },
  { EXT_REGINA_BIFS,vms_f_mode,            "F$MODE" },
  { EXT_REGINA_BIFS,vms_f_parse,           "F$PARSE" },
  { EXT_REGINA_BIFS,vms_f_pid,             "F$PID" },
  { EXT_REGINA_BIFS,vms_f_privilege,       "F$PRIVILEGE" },
  { EXT_REGINA_BIFS,vms_f_process,         "F$PROCESS" },
  { EXT_REGINA_BIFS,vms_f_search,          "F$SEARCH" },
  { EXT_REGINA_BIFS,vms_f_setprv,          "F$SETPRV" },
  { EXT_REGINA_BIFS,vms_f_string,          "F$STRING" },
  { EXT_REGINA_BIFS,vms_f_time,            "F$TIME" },
  { EXT_REGINA_BIFS,vms_f_trnlnm,          "F$TRNLNM" },
  { EXT_REGINA_BIFS,vms_f_type,            "F$TYPE" },
  { EXT_REGINA_BIFS,vms_f_user,            "F$USER" },
/*{ EXT_REGINA_BIFS,vms_f_verify,          "F$VERIFY" }, */
#endif
  { EXT_REGINA_BIFS,os2_filespec,          "FILESPEC" },
  { EXT_REGINA_BIFS,cms_find,              "FIND" },
#ifdef OLD_REGINA_FEATURES
  { EXT_REGINA_BIFS,unx_close,             "FINIS" },
#endif /* OLD_REGINA_FEATURES */
  { EXT_REGINA_BIFS,unx_fork,              "FORK" },
  { 0,              std_form,              "FORM" },
  { 0,              std_format,            "FORMAT" },
#if defined(REGINA_DEBUG_MEMORY)
  { EXT_REGINA_BIFS,dbg_freelists,         "FREELISTS" },
#endif
  { EXT_AREXX_BIFS, arexx_freespace,       "FREESPACE" },
  { 0,              std_fuzz,              "FUZZ" },
#ifdef HAVE_GCI
  { EXT_REGINA_BIFS,rex_gciprefixchar,     "GCIPREFIXCHAR" },
#endif
  { EXT_REGINA_BIFS,rex_getcaller,         "GETCALLER" },
  { EXT_REGINA_BIFS,rex_getcallstack,      "GETCALLSTACK" },
  { EXT_REGINA_BIFS,unx_getenv,            "GETENV" },
  { EXT_REGINA_BIFS,unx_getpath,           "GETPATH" },
  { EXT_REGINA_BIFS,unx_getpid,            "GETPID" },
  { EXT_AREXX_BIFS, arexx_getspace,        "GETSPACE" },
  { EXT_REGINA_BIFS,unx_gettid,            "GETTID" },
  { EXT_AREXX_BIFS, arexx_hash,            "HASH" },
  { EXT_AREXX_BIFS, arexx_import,          "IMPORT" },
  { EXT_REGINA_BIFS,cms_index,             "INDEX" },
  { 0,              std_insert,            "INSERT" },
  { EXT_REGINA_BIFS,cms_justify,           "JUSTIFY" },
  { 0,              std_lastpos,           "LASTPOS" },
  { 0,              std_left,              "LEFT" },
  { 0,              std_length,            "LENGTH" },
  { 0,              std_linein,            "LINEIN" },
  { 0,              std_lineout,           "LINEOUT" },
  { 0,              std_lines,             "LINES" },
#ifdef TRACEMEM
  { EXT_REGINA_BIFS,dbg_listleaked,        "LISTLEAKED" },
#endif
  { EXT_REGINA_BIFS,rex_lower,             "LOWER" },
  { EXT_MAKEBUF_BIF,cms_makebuf,           "MAKEBUF" },
  { 0,              std_max,               "MAX" },
#ifdef TRACEMEM
  { EXT_REGINA_BIFS,dbg_memorystats,       "MEMORYSTATS" },
#endif
  { 0,              std_min,               "MIN" },
  { EXT_REGINA_BIFS,conflict_open,         "OPEN" },
  { 0,              std_overlay,           "OVERLAY" },
  { EXT_REGINA_BIFS,rex_poolid,            "POOLID" },
  { EXT_REGINA_BIFS,unx_popen,             "POPEN" },
  { 0,              std_pos,               "POS" },
  { EXT_REGINA_BIFS,unx_putenv,            "PUTENV" },
  { 0,              std_qualify,           "QUALIFY" },
  { 0,              std_queued,            "QUEUED" },
  { 0,              std_random,            "RANDOM" },
  { EXT_AREXX_BIFS, arexx_randu,           "RANDU" },
  { EXT_AREXX_BIFS, arexx_readch,          "READCH" },
  { EXT_AREXX_BIFS, arexx_readln,          "READLN" },
  { 0,              std_reverse,           "REVERSE" },
  { 0,              std_right,             "RIGHT" },

  { 0,              rex_rxfuncadd,         "RXFUNCADD" },
#ifdef HAVE_GCI
  { EXT_REGINA_BIFS,rex_rxfuncdefine,      "RXFUNCDEFINE" },
#endif
  { 0,              rex_rxfuncdrop,        "RXFUNCDROP" },
  { EXT_REGINA_BIFS,rex_rxfuncerrmsg,      "RXFUNCERRMSG" },
  { 0,              rex_rxfuncquery,       "RXFUNCQUERY" },
  { 0,              rex_rxqueue,           "RXQUEUE" },

  { EXT_AREXX_BIFS, arexx_seek,            "SEEK" },
  { EXT_AREXX_BIFS, arexx_show,            "SHOW" },
  { 0,              std_sign,              "SIGN" },
  { EXT_REGINA_BIFS,cms_sleep,             "SLEEP" },
  { 0,              std_sourceline,        "SOURCELINE" },
  { 0,              std_space,             "SPACE" },
  { EXT_REGINA_BIFS,cms_state,             "STATE" },
  { EXT_AREXX_BIFS, arexx_storage,         "STORAGE" },
  { 0,              std_stream,            "STREAM" },
  { 0,              std_strip,             "STRIP" },
  { 0,              std_substr,            "SUBSTR" },
  { 0,              std_subword,           "SUBWORD" },
  { 0,              std_symbol,            "SYMBOL" },
  { 0,              std_time,              "TIME" },
  { 0,              std_trace,             "TRACE" },

  { EXT_REGINA_BIFS,dbg_traceback,         "TRACEBACK" },

  { 0,              std_translate,         "TRANSLATE" },
  { EXT_AREXX_BIFS, arexx_trim,            "TRIM" },
  { 0,              std_trunc,             "TRUNC" },
  { EXT_REGINA_BIFS,unx_uname,             "UNAME" },
  { EXT_REGINA_BIFS,unx_unixerror,         "UNIXERROR" },
  { EXT_REGINA_BIFS,arexx_upper,           "UPPER" },
  { EXT_REGINA_BIFS,rex_userid,            "USERID" },
  { 0,              std_value,             "VALUE" },
  { 0,              std_verify,            "VERIFY" },
  { 0,              std_word,              "WORD" },
  { 0,              std_wordindex,         "WORDINDEX" },
  { 0,              std_wordlength,        "WORDLENGTH" },
  { 0,              std_wordpos,           "WORDPOS" },
  { 0,              std_words,             "WORDS" },
  { EXT_AREXX_BIFS, arexx_writech,         "WRITECH" },
  { EXT_AREXX_BIFS, arexx_writeln,         "WRITELN" },
  { 0,              std_x2b,               "X2B" },
  { 0,              std_x2c,               "X2C" },
  { 0,              std_x2d,               "X2D" },
  { 0,              std_xrange,            "XRANGE" },
  { 0,              NULL,                  NULL }
} ;

static const int num_funcs = sizeof(functions) / (sizeof(functions[0])) - 1 ;

static const int MonthDays[] = {31,28,31,30,31,30,31,31,30,31,30,31};
static const int DaysInYear[] = {0,31,59,90,120,151,181,212,243,273,304,334};

static int leapyear(long year) ;
static void base2date(long basedate,void *conv_tmdata) ;

#ifdef TRACEMEM
void mark_listleaked_params( const tsd_t *TSD )
{
   paramboxptr pptr=NULL ;

   for (pptr=TSD->listleaked_params; pptr; pptr=pptr->next)
   {
      markmemory( pptr, TRC_PROCARG ) ;
      if (pptr->value)
         markmemory( pptr->value, TRC_PROCARG ) ;
   }
}
#endif

streng *buildtinfunc( tsd_t *TSD, nodeptr thisptr )
{
   int low=0, topp=0, mid=0, end=1, up=num_funcs-1, i=0 ;
   streng *ptr;
   struct entry_point *vptr;
   streng *(*func)(tsd_t *,cparamboxptr)=NULL ;
   const char *BIFname = NULL; /* set to non-NULL only in case of a BIF */
   void *BIFfunc = NULL; /* set to non-NULL only in case of a BIF */

   /*
    * Look for a function registered in a DLL
    */
   vptr = loaded_lib_func( TSD, thisptr->name ) ;
   if ( vptr )
      func = std_center ; /* e.g. */

   /*
    * If no function registered in a DLL or EXE, look for a builtin
    */
   if (!func)
   {
      topp = Str_len( thisptr->name ) ;

      if (thisptr->u.func)
         func = thisptr->u.func ;
      else
      {
         mid = 0 ;  /* to keep the compiler happy */
         while ((end)&&(up>=low))
         {
            mid = (up+low)/2 ;
            for (i=0; i<topp; i++ )
               if (thisptr->name->value[i] != functions[mid].funcname[i])
                  break ;

            if (i==topp)
               end = (functions[mid].funcname[i]!=0x00) ;
            else
               end = ( functions[mid].funcname[i] - thisptr->name->value[i] ) ;

            if (end>0)
               up = mid-1 ;
            else
               low = mid+1 ;
         }
         if (!end)
         {
            /*
             * Check if the function is an extension. If it is and it matches
             * an extension specified with the OPTIONS keyword, then allow it.
             * If the OPTION; STRICT_ANSI is in effect however, this overrides
             * the extension.
             */
            BIFname = functions[mid].funcname;
            if (functions[mid].compat)
            {
               if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
                  exiterror( ERR_NON_ANSI_FEATURE, 1, functions[mid].funcname );
               if ( ! get_options_flag( TSD->currlevel, functions[mid].compat ) )
                  func = NULL ;
               else
               {
                  func = functions[mid].function ;
                  if ( get_options_flag( TSD->currlevel, EXT_CACHEEXT ) )
                     thisptr->u.func = func ;
               }
            }
            else
               thisptr->u.func = func = functions[mid].function ;
         }
      }
      BIFfunc = (void *) func;
   }

   if (func)
   {
      if (TSD->bif_first)
         deallocplink( TSD, TSD->bif_first ) ;
      TSD->bif_first = NULL; /* NEVER delete this! initplist
                     * may setjmp to the line above
                     * which results to a twice-called
                     * deallocplink. FGC
                     */

      TSD->bif_first = initplist( TSD, thisptr ) ;
      TSD->BIFname = BIFname;
      TSD->BIFfunc = (void *) BIFfunc;
      if (vptr)
         ptr = call_known_external( TSD, vptr, TSD->bif_first, (char) thisptr->o.called ) ;
      else
         ptr = (*func)(TSD, TSD->bif_first /* ->next */ ) ;
      TSD->BIFname = NULL;
      TSD->BIFfunc = NULL;

      deallocplink( TSD, TSD->bif_first ) ;
      TSD->bif_first = NULL ;
      return ptr ;
   }
   else
   {
      if (IfcHaveFunctionExit( TSD )) /* we have an exit handler */
      {
         if (TSD->bif_first)
            deallocplink( TSD, TSD->bif_first ) ;
         TSD->bif_first = NULL; /* NEVER delete this! initplist
                                                 * may setjmp to the line above
                                                 * which results to a twice-called
                                                 * deallocplink. FGC
                                                 */


         TSD->bif_first = initplist( TSD, thisptr ) ;
         ptr = call_unknown_external( TSD, thisptr->name, TSD->bif_first, (char) thisptr->o.called ) ;
         deallocplink( TSD, TSD->bif_first ) ;
         TSD->bif_first = NULL ;
      }
      else
         ptr = NOFUNC;
   }
   return ptr;
}

paramboxptr initplist( tsd_t *TSD, cnodeptr thisptr )
{
   paramboxptr first,newptr,currnt;

   first = currnt = NULL ;
   for (thisptr=thisptr->p[0]; thisptr; thisptr=thisptr->p[1])
   {
      if (TSD->par_stack)
      {
         newptr = TSD->par_stack ;
         TSD->par_stack = newptr->next ;
      }
      else
         newptr = (paramboxptr)MallocTSD( sizeof( parambox )) ;

      if (!first)
         first = currnt = newptr ;
      else
      {
         currnt->next = newptr ;
         currnt = newptr ;
      }

      if (thisptr->type==X_CEXPRLIST && TSD->trace_stat!='I')
      {
         if (thisptr->u.strng)
            currnt->value = thisptr->u.strng ;
         else
            currnt->value  = NULL ;

         currnt->dealloc = 0 ;
      }
      else if ( !thisptr->p[0] )
      {
         currnt->dealloc = 1;
         currnt->value = NULL;
      }
      else
      {
         /*
          * This fixes bug 590589 and others.
          * Always force a fresh new return value of evaluate.
          * Imagine this code, it will produce a crash otherwise:
          * call func x
          * return
          * func:
          *    x = "new" || "value"
          *    say arg(1)
          */
         currnt->dealloc = 1;
         currnt->value = evaluate( TSD, thisptr->p[0], NULL );
      }
   }
#ifdef TRACEMEM
   TSD->listleaked_params = first ;
#endif
   if ( currnt )
      currnt->next = NULL ;
   return first ;
}


paramboxptr initargs( tsd_t *TSD, int argc, const int *lengths,
                      const char **strings )
{
   paramboxptr first,newptr,currnt;
   int i;

   first = currnt = NULL;
   for ( i = 0; i < argc; i++ )
   {
      if ( TSD->par_stack )
      {
         newptr = TSD->par_stack;
         TSD->par_stack = newptr->next;
      }
      else
         newptr = (paramboxptr)MallocTSD( sizeof( parambox ) );

      if ( !first )
         first = currnt = newptr;
      else
      {
         currnt->next = newptr;
         currnt = newptr;
      }

      if ( lengths[i] == RX_NO_STRING )
      {
         currnt->dealloc = 1;
         currnt->value = NULL;
      }
      else
      {
         currnt->value = Str_ncreTSD( strings[i], lengths[i] );
         currnt->dealloc = 1;
      }
   }

#ifdef TRACEMEM
   TSD->listleaked_params = first;
#endif

   if ( currnt )
      currnt->next = NULL;
   return first;
}


void deallocplink( tsd_t *TSD, paramboxptr first )
{
   paramboxptr thisptr;

   for (;first;)
   {
      thisptr = first ;
      first = first->next ;
      if (thisptr->dealloc && thisptr->value)
      {
         Free_stringTSD( thisptr->value ) ;
         thisptr->value = NULL;
      }

#if defined(CHECK_MEMORY)
      FreeTSD(thisptr);
#else
      /* Back to the freed-parbox stack: */
      thisptr->next = TSD->par_stack ;
      TSD->par_stack = thisptr ;
#endif
   }
}


#ifdef TRACEMEM
void mark_param_cache( const tsd_t *TSD )
{
   paramboxptr ptr=NULL ;

   ptr = TSD->par_stack ;
   for (; ptr; ptr=ptr->next )
      markmemory( ptr, TRC_P_CACHE ) ;
}
#endif



int myatol( const tsd_t *TSD, const streng *text )
{
   int num, error ;

   num = streng_to_int( TSD, text, &error ) ;
   if (error)
       exiterror( ERR_INVALID_INTEGER, 0 )  ;

   return num ;
}

static int myintatol( tsd_t *TSD, const streng *text, int suberr, const char *bif, int argnum )
{
   int num, error ;

   num = streng_to_int( TSD, text, &error ) ;
   if ( error )
      exiterror( ERR_INCORRECT_CALL, suberr, bif, argnum, tmpstr_of( TSD, text ) ) ;

   return num ;
}

static rx_64 myintatoll( tsd_t *TSD, const streng *text, int suberr, const char *bif, int argnum )
{
   rx_64 num;
   int error ;

   num = streng_to_rx64( TSD, text, &error ) ;
   if ( error )
      exiterror( ERR_INCORRECT_CALL, suberr, bif, argnum, tmpstr_of( TSD, text ) ) ;

   return num ;
}


int atozpos( tsd_t *TSD, const streng *text, const char *bif, int argnum )
{
   int result=0 ;

   /* fixes bug 1108868 */
   if ( ( result = myintatol( TSD, text, 12, bif, argnum ) ) < 0 )
      exiterror( ERR_INCORRECT_CALL, 13, bif, argnum, tmpstr_of( TSD, text ) )  ;

   return result ;
}

rx_64 atozposrx64( tsd_t *TSD, const streng *text, const char *bif, int argnum )
{
   rx_64 result=0 ;

   if ( ( result = myintatoll( TSD, text, 12, bif, argnum ) ) < 0 )
      exiterror( ERR_INCORRECT_CALL, 13, bif, argnum, tmpstr_of( TSD, text ) )  ;

   return result ;
}


char getoptionchar( tsd_t *TSD, const streng *text, const char* bif, int argnum, const char *ansi_choices, const char *regina_choices )
{
   char ch=0 ;
   const char *ptr = NULL;
   char tmp[50];

   if (text->len == 0)
      exiterror( ERR_INCORRECT_CALL, 21, bif, argnum )  ;

   ch = (char) rx_toupper( text->value[0] ) ;
   /*
    * If the option supplied is ANSI, then return when we find it.
    */
   for ( ptr = ansi_choices; *ptr; ptr++ )
   {
      if ( *ptr == ch )
         return ch ;
   }
   /*
    * If the option supplied is a Regina extension, and we are NOT running in
    * ANSI mode, then return when we find it.
    */
   for ( ptr = regina_choices; *ptr; ptr++ )
   {
      if ( *ptr == ch )
      {
         if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
            exiterror( ERR_NON_ANSI_FEATURE, 3, bif, argnum, ansi_choices, tmpstr_of( TSD, text ) );
         else
            return ch ;
      }
   }

   if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
      exiterror( ERR_INCORRECT_CALL, 28, bif, argnum, ansi_choices, tmpstr_of( TSD, text ) );
   else
   {
      strcpy( tmp, ansi_choices );
      strcat( tmp, regina_choices );
      exiterror( ERR_INCORRECT_CALL, 28, bif, argnum, tmp, tmpstr_of( TSD, text ) );
   }
   return 0 ;
}


char getonechar( tsd_t *TSD, const streng *text, const char *bif, int argnum )
{
   if ( !text )
      exiterror( ERR_INCORRECT_CALL, 23, bif, argnum, "" )  ;
   if ( Str_len( text ) != 1 )
      exiterror( ERR_INCORRECT_CALL, 23, bif, argnum, tmpstr_of( TSD, text ) ) ;

   return text->value[0] ;
}

char getonespecialchar( tsd_t *TSD, const streng *text, const char *bif, int argnum )
{
   if ( !text )
      exiterror( ERR_INCORRECT_CALL, 43, bif, argnum, "" )  ;
   if ( Str_len( text ) != 1 )
      exiterror( ERR_INCORRECT_CALL, 43, bif, argnum, tmpstr_of( TSD, text ) ) ;
   if ( rx_isalnum( text->value[0] ) )
      exiterror( ERR_INCORRECT_CALL, 43, bif, argnum, tmpstr_of( TSD, text ) ) ;

   return text->value[0] ;
}

int atopos( tsd_t *TSD, const streng *text, const char *bif, int argnum )
{
   int result=0 ;

   /* fixes bug 1108868 */
   if ( ( result = myintatol( TSD, text, 12, bif, argnum ) ) <= 0 )
      exiterror( ERR_INCORRECT_CALL, 14, bif, argnum, tmpstr_of( TSD, text ) ) ;

   return result ;
}

rx_64 atoposrx64( tsd_t *TSD, const streng *text, const char *bif, int argnum )
{
   rx_64 result=0 ;

   if ( ( result = myintatoll( TSD, text, 12, bif, argnum ) ) <= 0 )
      exiterror( ERR_INCORRECT_CALL, 14, bif, argnum, tmpstr_of( TSD, text ) ) ;

   return result ;
}

int atoposorzero( tsd_t *TSD, const streng *text, const char *bif, int argnum )
{
   int result=0 ;

   if ( ( result = myintatol( TSD, text, 11, bif, argnum ) ) < 0 )
      exiterror( ERR_INCORRECT_CALL, 13, bif, argnum, tmpstr_of( TSD, text ) ) ;

   return result ;
}



streng *int_to_streng( const tsd_t *TSD, int input )
{
   streng *output=NULL ;
   char *cptr=NULL, *start=NULL, *top=NULL ;

   output = Str_makeTSD( sizeof(int)*3 + 2 ) ;
   start = output->value ;
   cptr = start + sizeof(int)*3 + 2 ;
   if (input)
   {
      if (input<0)
      {
         input = - input ;
         *(start++) = '-' ;
      }

      for (top=cptr;input;)
      {
         *(--cptr) = (char) (input % 10 + '0') ;
         input = input / 10 ;
      }

      memmove( start, cptr, top-cptr ) ;
      output->len = top-cptr + start-output->value ;
   }
   else
   {
      *start = '0' ;
      output->len = 1 ;
   }

   return output ;
}

streng *rx64_to_streng( const tsd_t *TSD, rx_64 input )
{
   streng *output=NULL ;
   char *cptr=NULL, *start=NULL, *top=NULL ;

   output = Str_makeTSD( sizeof(rx_64)*3 + 2 ) ;
   start = output->value ;
   cptr = start + sizeof(rx_64)*3 + 2 ;
   if (input)
   {
      if (input<0)
      {
         input = - input ;
         *(start++) = '-' ;
      }

      for (top=cptr;input;)
      {
         *(--cptr) = (char) (input % 10 + '0') ;
         input = input / 10 ;
      }

      memmove( start, cptr, top-cptr ) ;
      output->len = top-cptr + start-output->value ;
   }
   else
   {
      *start = '0' ;
      output->len = 1 ;
   }

   return output ;
}

streng *rx64u_to_streng( const tsd_t *TSD, rx_64u input )
{
   streng *output=NULL ;
   char *cptr=NULL, *start=NULL, *top=NULL ;

   output = Str_makeTSD( sizeof(rx_64u)*3 + 2 ) ;
   start = output->value ;
   cptr = start + sizeof(rx_64u)*3 + 2 ;
   if (input)
   {
      for (top=cptr;input;)
      {
         *(--cptr) = (char) (input % 10 + '0') ;
         input = input / 10 ;
      }

      memmove( start, cptr, top-cptr ) ;
      output->len = top-cptr + start-output->value ;
   }
   else
   {
      *start = '0' ;
      output->len = 1 ;
   }

   return output ;
}


void checkparam( cparamboxptr params, int min, int max, const char *name )
{
   int i=0 ;

   for (i=0;i<min;i++,params=(cparamboxptr) (params->next))
      if ((!params)||(!params->value))
         exiterror( ERR_INCORRECT_CALL, 3, name, min )  ;

   for (;(i<max)&&(params);i++,params=(cparamboxptr) (params->next)) ;
   if (((i==max)&&(params))&&((max)||(params->value)))
      exiterror( ERR_INCORRECT_CALL, 4, name, max )  ;
}


/*
 * These functions are rather ugly, but they works :-)
 */
/*
 * Converts a date supplied in an external format (specified by suppformat)
 * into a struct tm (individual values for year, month, day, year_days and
 * base days).
 */
int convert_date(tsd_t *TSD, const streng *suppdate, char suppformat, struct tm *indate, char isep)
{
   int rc,i=0,off=0,save_year=indate->tm_year,ilen;
   long num1=0,num2=0,num3=0;
   char buf[20];
   char *ptr=(char*)suppdate->value;
   struct tm *tmpTime;
   time_t num64;

   ilen = (isep == '\0' ? 0 : 1);
   indate->tm_sec = indate->tm_min = indate->tm_hour = 0;
   switch(suppformat)
   {
      case 'B': /* 99999... */
      case 'D': /* 99999... */
         if (suppdate->len > 19)
            return(1);
         memcpy(buf,ptr,suppdate->len);
         buf[suppdate->len] = '\0';
         if ((num1 = atol(buf)) == 0)
         {
            for (i=0;i<suppdate->len;i++)
            {
               if (buf[i] != '0')
                  return(1);
            }
         }
         if ( suppformat == 'B' )
            base2date(num1,indate);
         else
            base2date(num1+basedays(indate->tm_year)-1,indate);
         break;
#if 0
      case 'I': /* WHAT IS THIS? */
         if (suppdate->len > 19)
            return(1);
         memcpy(buf,ptr,suppdate->len);
         buf[suppdate->len] = '\0';
         if ((num1 = atol(buf)) == 0)
         {
            for (i=0;i<suppdate->len;i++)
            {
               if (buf[i] != '0')
                  return(1);
            }
         }
         base2date(num1+basedays(1978)-1,indate);
         break;
#endif
      case 'E': /* dd/mm/yy or ddmmyy (where '/' is isep) */
      case 'O': /* yy/mm/dd or yymmdd (where '/' is isep) */
      case 'U': /* mm/dd/yy or mmddyy (where '/' is isep) */
         if (suppdate->len != 6+(ilen*2))
            return(1);
         memcpy(buf,ptr,2);
         buf[2] = '\0';
         if ( !rx_isdigit( buf[0] ) || !rx_isdigit( buf[1] ) )
            return( 1 );
         num1 = atol( buf );
         memcpy(buf,(ptr+2+ilen),2);
         buf[2] = '\0';
         if ( !rx_isdigit( buf[0] ) || !rx_isdigit( buf[1] ) )
            return( 1 );
         num2 = atol( buf );
         memcpy(buf,(ptr+4+(ilen*2)),2);
         buf[2] = '\0';
         if ( !rx_isdigit( buf[0] ) || !rx_isdigit( buf[1] ) )
            return( 1 );
         /* if we have an input separator, validate that it matches */
         if ( ilen )
         {
            if (*(ptr+2) != isep && *(ptr+5) != isep)
               return( 2 );
         }
         num3 = atol( buf );
         switch(suppformat)
         {
            case 'E':
               if ( num1 == 0 || num2 == 0 )
                  return( 1 );
               indate->tm_mday = num1;
               indate->tm_mon = num2-1;
               indate->tm_year = num3;
               break;
            case 'O':
               if ( num3 == 0 || num2 == 0 )
                  return( 1 );
               indate->tm_mday = num3;
               indate->tm_mon = num2-1;
               indate->tm_year = num1;
               break;
            case 'U':
               if ( num2 == 0 || num1 == 0 )
                  return( 1 );
               indate->tm_mday = num2;
               indate->tm_mon = num1-1;
               indate->tm_year = num3;
               break;
         }
         /*
          * Work out the century based on a sliding year
          */
         if (indate->tm_year < 100)   /* do something with century ... */
            indate->tm_year += ( indate->tm_year <= (save_year - 2000 ) + 50 ) ? 2000 : 1900;
         break;
      case 'N': /* dd mmm yyyy or d mmm yyyy or ddmmmyyyy or dmmmyyyy */
         if (suppdate->len < 8 || suppdate->len > 11)
            return(1);
         if (suppdate->len == 10 || suppdate->len == 8)
            off = 1;
         memcpy(buf,ptr,2-off);
         buf[2-off] = '\0';
         if ((num1 = atol(buf)) == 0)
            return(1);
         memcpy(buf,(ptr+2-off+ilen),3);
         buf[3] = '\0';
         /* find month */
         num2 = (-1);
         for (i=0;i<12;i++)
         {
            if (strncmp(months[i],buf,3) == 0)
            {
               num2 = i;
               break;
            }
         }
         if (num2 == (-1))
            return(1);
         memcpy(buf,(ptr+5-off+(ilen*2)),4);
         buf[4] = '\0';
         if ((num3 = atol(buf)) == 0 && strcmp("0000",buf) != 0)
            return(1);
         /* if we have an input separator, validate that it matches */
         if ( ilen )
         {
            if (*(ptr+2-off) != isep && *(ptr+6-off) != isep)
               return( 2 );
         }
         indate->tm_mday = num1;
         indate->tm_mon = num2;
         indate->tm_year = num3;
         break;
      case 'S': /* yyyymmdd   (where '' is isep) */
      case 'I': /* yyyy-mm-dd (where '-' is isep */
         if ( suppdate->len != 8+(ilen*2) )
            return(1);
         memcpy( buf, ptr, 4 );
         buf[4] = '\0';
         if ( ( num1 = atol( buf ) ) == 0 )
            return(1);
         memcpy( buf, (ptr+4+(ilen)), 2 );
         buf[2] = '\0';
         if ( ( num2 = atol( buf ) ) == 0 )
            return(1);
         memcpy( buf, (ptr+6+(ilen*2)), 2 );
         buf[2] = '\0';
         if ( ( num3 = atol( buf ) ) == 0 )
            return(1);
         /* if we have an input separator, validate that it matches */
         if ( ilen )
         {
            if (*(ptr+4) != isep && *(ptr+7) != isep)
               return( 2 );
         }
         indate->tm_mday = num3;
         indate->tm_mon = num2-1;
         indate->tm_year = num1;
         break;
      case 'T': /* +|-999999... */
         num64 = streng_to_rx64( TSD, suppdate, &rc );
         if ( rc )
            return 1;
         tmpTime = localtime( (time_t *)&num64 );
         *indate = *tmpTime;
         indate->tm_year += 1900;
         /*
          * Reset time to 00:00:00
          */
         indate->tm_sec = indate->tm_hour = indate->tm_min = 0;
         break;
      default:
         /* should not get here */
         break;
   }
   if (indate->tm_mday > ( MonthDays[indate->tm_mon] + ( (indate->tm_mon == 1) ? leapyear(indate->tm_year) : 0 ) )
   ||  indate->tm_mday < 1
   ||  indate->tm_mon > 11
   ||  indate->tm_mon < 0
   ||  indate->tm_year < 0)
      return(1);

   indate->tm_yday = DaysInYear[indate->tm_mon]+
                     ((leapyear(indate->tm_year)&&indate->tm_mon>1)?1:0)+
                     indate->tm_mday - 1;
   indate->tm_wday = (((indate->tm_yday+basedays(indate->tm_year))+8) % 7);
   return(0);
}

/*
 * Converts a year (MUST have century) to a number of days
 * Base year is 0001 - hence the date 01 Jan 0001 is base day 1
 */
int basedays(int year)
{
   return((year-1)*365 + (year-1)/4 - (year-1)/100 + (year-1)/400);
}

/*
 * Determines if a year (MUST have a century) is a leap year
 */
static int leapyear(long year)
{
   if ((year%4 == 0 && year%100 != 0) || year%400 == 0)
      return(1);
   else
      return(0);
}

/*
 * Converts a number (representing the number of days since 01 Jan 0001)
 * to a struct tm value (individual fields for year, month, day and days in year)
 */
static void base2date(long basedate,void *conv_tmdata)
{
   struct tm *outdate=(struct tm *)conv_tmdata;
   int i=0;
   long day=0L,year=0L,month=0L,yeardays=0L,thismonth=0L;

   day = basedate + 1L;
   year = day / 366;
   day -= ((year*365) + (year/4) - (year/100) + (year/400));
   year++;
   while (day > (365 + leapyear(year)))
   {
      day -= (365 + leapyear(year));
      year++;
   }

   yeardays = day;
   for (i=0;i<11;i++)
   {
      thismonth = (MonthDays[i]) + ((i == 1) ? leapyear(year) : 0);
      if (day <= thismonth)
         break;
      day -= thismonth;
   }
   month = i;
   outdate->tm_year = year;
   outdate->tm_mon = month;
   outdate->tm_mday = day;
   outdate->tm_yday = yeardays;
   outdate->tm_wday = ((basedate+8) % 7);
   return;
}

/*
 * Converts a time supplied in an external format (specified by suppformat)
 * into a struct tm (individual values for hour, minute, second).
 */
int convert_time( const tsd_t *TSD, const streng *supptime, char suppformat, struct tm *intime, time_t *unow)
{
   int rc,offset;
   long num1=0,num2=0,num3=0,num4=0;
   char buf[20];
   char *ptr=(char*)supptime->value;
   struct tm *tmpTime;
   time_t num64;

   switch(suppformat)
   {
      case 'C': /* hh:mmXX */
         /*
          * Format of time can be "3:45pm", or "11:45pm"; ie hour can be 1 or
          * two digits. Use of "offset" below fixes bug 742725
          */
         if (*(ptr+2) == ':')
            offset = 1;
         else if (*(ptr+1) == ':')
            offset = 0;
         else
            return(1);
         if (memcmp("am",ptr+4+offset,2) != 0 && memcmp("pm",ptr+4+offset,2) != 0)
            return(1);
         memcpy(buf,ptr,1+offset);
         buf[1+offset] = '\0';
         if ((num1 = atol(buf)) == 0 && strcmp("00",buf) != 0)
            return(1);
         if (num1 > 12)
            return(1);
         memcpy(buf,ptr+2+offset,2);
         buf[2] = '\0';
         if ((num2 = atol(buf)) == 0 && strcmp("00",buf) != 0)
            return(1);
         if (num2 > 59)
            return(1);
         intime->tm_sec = 0;
         if (memcmp("am",ptr+4+offset,2)==0)
         {
            if (num1 == 12)
               intime->tm_hour = 0;
            else
               intime->tm_hour = num1;
         }
         else
         {
            if (num1 == 12)
               intime->tm_hour = num1;
            else
               intime->tm_hour = num1+12;
         }
         intime->tm_min = num2;
         *unow = 0;
         break;
      case 'H': /* 99999... */
      case 'M': /* 99999... */
      case 'S': /* 99999... */
         /*
          * Convert supptime to whole number using streng_to_int()
          * rather than atoi(). Bug #20000922-78622
          */
         num1 = streng_to_int( TSD, supptime, &rc );
         if ( rc
         || num1 < 0 )
            return(1);
         switch(suppformat)
         {
            case 'H':
               intime->tm_hour = num1;
               intime->tm_min = intime->tm_sec = 0;
               break;
            case 'M':
               intime->tm_hour = num1 / 60;
               intime->tm_min = num1 % 60;
               intime->tm_sec = 0;
               break;
            case 'S':
               intime->tm_hour = num1 / 3600;
               intime->tm_min = (num1 % 3600) / 60;
               intime->tm_sec = (num1 % 3600) % 60;
               break;
         }
         if ( intime->tm_sec > 59 || intime->tm_hour > 23 || intime->tm_min > 59 )
            return(1);
         *unow = 0;
         break;
      case 'L': /* hh:mm:ss.mmmmmm */
      case 'N': /* hh:mm:ss */
         if (suppformat == 'N' && supptime->len != 8)
            return(1);
         if (suppformat == 'L' && supptime->len != 15)
            return(1);
         if (*(ptr+2) != ':' && *(ptr+5) != ':')
            return(1);
         memcpy(buf,ptr,2);
         buf[2] = '\0';
         if ((num1 = atol(buf)) == 0 && strcmp("00",buf) != 0)
            return(1);
         if (num1 < 0 || num1 > 23)
            return(1);

         memcpy(buf,ptr+3,2);
         buf[2] = '\0';
         if ((num2 = atol(buf)) == 0 && strcmp("00",buf) != 0)
            return(1);
         if (num2 < 0 || num2 > 59)
            return(1);

         memcpy(buf,ptr+6,2);
         buf[2] = '\0';
         if ((num3 = atol(buf)) == 0 && strcmp("00",buf) != 0)
            return(1);
         if (num3 < 0 || num3 > 59)
            return(1);
         intime->tm_sec = num3;
         intime->tm_hour = num1;
         intime->tm_min = num2;
         if (suppformat == 'N')
         {
            *unow = 0;
            break;
         }
         if (*(ptr+8) != '.')
            return(1);
         memcpy(buf,ptr+9,6);
         buf[6] = '\0';
         if ((num4 = atol(buf)) == 0 && strcmp("000000",buf) != 0)
            return(1);
         if (num4 < 0)
            return(1);
         *unow = num4;
         break;
      case 'T': /* +|-999999... */
         num64 = streng_to_int( TSD, supptime, &rc );
         if ( rc )
            return 1;
         tmpTime = localtime( (time_t *)&num64 );
         *intime = *tmpTime;
         *unow = 0;
         break;
      default:
         /* should not get here */
         break;
   }

   return(0);
}
/*
 * The following functions are wrappers for BIFs that are syntactically different
 * depending on the OPTONS used.
 */

static streng *conflict_close( tsd_t *TSD, cparamboxptr parms )
{
   if ( get_options_flag( TSD->currlevel, EXT_AREXX_SEMANTICS ) )
      return( arexx_close( TSD, parms ) );
   else
      return( unx_close( TSD, parms ) );
}

static streng *conflict_eof( tsd_t *TSD, cparamboxptr parms )
{
   if ( get_options_flag( TSD->currlevel, EXT_AREXX_SEMANTICS ) )
      return( arexx_eof( TSD, parms ) );
   else
      return( unx_eof( TSD, parms ) );
}

static streng *conflict_open( tsd_t *TSD, cparamboxptr parms )
{
   if ( get_options_flag( TSD->currlevel, EXT_AREXX_SEMANTICS ) )
      return( arexx_open( TSD, parms ) );
   else
      return( unx_open( TSD, parms ) );
}

/*
 * BIFname tries to guess the name of the BIF we currently evaluate.
 * We try to identify cached functions pointers.
 */
const char *BIFname( tsd_t *TSD )
{
   int i;
   void *func;

   assert( TSD->currentnode );

   if ( TSD->BIFname != NULL )
   {
      return TSD->BIFname;
   }

   if ( TSD->BIFfunc != NULL )
   {
      func = TSD->BIFfunc;
   }
   else
   {
      func = (void *) TSD->currentnode->u.func;

   }
   for ( i = 0; i < num_funcs; i++ )
   {
      if ( (void *) functions[i].function == func )
      {
         return functions[i].funcname;
      }
   }
   return "(internal)";
}
