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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_PROCESS_H
#include <process.h>
#endif

#ifdef SunKludges
double pow( double, double ) ;
#endif

#if defined(HAVE_PUTENV) && defined(FIX_PROTOS) && defined(ultrix)
void putenv( char* );
#endif

#if defined(__OS2__) && defined(__WATCOMC__)
time_t mktime2(struct tm * tmp);
#endif

#define UPPERLETTER(a) ((((a)&0xdf)>='A')&&(((a)&0xdf)<='Z'))
#define NUMERIC(a) (((a)>='0')&&((a)<='9'))

static const char *WeekDays[] = { "Sunday", "Monday", "Tuesday", "Wednesday",
                                  "Thursday", "Friday", "Saturday" } ;
const char *months[] = { "January", "February", "March", "April", "May",
                         "June", "July", "August", "September", "October",
                         "November", "December" } ;

struct envirlist {
   struct envirlist *next ;
   streng *ptr ;
};

typedef struct { /* bui_tsd: static variables of this module (thread-safe) */
   struct envirlist * first_envirvar;
   lineboxptr         srcline_ptr;      /* std_sourceline() */
   lineboxptr         srcline_first;    /* std_sourceline() */
   int                srcline_lineno;   /* std_sourceline() */
   int                seed;
} bui_tsd_t; /* thread-specific but only needed by this module. see
              * init_builtin
              */

/* init_builtin initializes the module.
 * Currently, we set up the thread specific data.
 * The function returns 1 on success, 0 if memory is short.
 */
int init_builtin( tsd_t *TSD )
{
   bui_tsd_t *bt;

   if (TSD->bui_tsd != NULL)
      return(1);

   if ( ( TSD->bui_tsd = MallocTSD( sizeof(bui_tsd_t) ) ) == NULL )
      return(0);
   bt = (bui_tsd_t *)TSD->bui_tsd;
   memset( bt, 0, sizeof(bui_tsd_t) );  /* correct for all values */

#if defined(HAVE_RANDOM)
   srandom((int) (time((time_t *)0)+getpid())%(3600*24)) ;
#else
   srand((unsigned) (time((time_t *)0)+getpid())%(3600*24)) ;
#endif
   return(1);
}

static int contained_in( const char *first, const char *fend, const char *second, const char *send )
/*
 * Determines if one string exists in another string. Search is done
 * based on words.
 */
{
   /*
    * Skip over any leading spaces in the search string
    */
   for (; (first<fend)&&(rx_isspace(*first)); first++)
   {
      ;
   }
   /*
    * Trim any trailing spaces in the search string
    */
   for (; (first<fend)&&(rx_isspace(*(fend-1))); fend--)
   {
      ;
   }
   /*
    * Skip over any leading spaces in the searched string
    */
   for (; (second<send)&&(rx_isspace(*second)); second++)
   {
      ;
   }
   /*
    * Trim any trailing spaces in the searched string
    */
   for (; (second<send)&&(rx_isspace(*(send-1))); send--)
   {
      ;
   }
   /*
    * If the length of the search string is less than the string to
    * search we won't find a match
    */
   if (fend-first > send-second)
      return 0;

   for (; (first<fend); )
   {
      for (; (first<fend)&&(!rx_isspace(*first)); first++, second++)
      {
         if ((*first)!=(*second))
            return 0 ;
      }

      if ((second<send)&&(!rx_isspace(*second)))
         return 0 ;

      if (first==fend)
         return 1 ;

      for (; (first<fend)&&(rx_isspace(*first)); first++)
      {
         ;
      }
      for (; (second<send)&&(rx_isspace(*second)); second++)
      {
         ;
      }
   }

   return 1 ;
}


streng *std_wordpos( tsd_t *TSD, cparamboxptr parms )
{
   streng *seek=NULL, *target=NULL ;
   char *sptr=NULL, *tptr=NULL, *end=NULL, *send=NULL ;
   int start=1, res=0 ;

   checkparam(  parms,  2,  3 , "WORDPOS" ) ;
   seek = parms->value ;
   target = parms->next->value ;
   if ((parms->next->next)&&(parms->next->next->value))
      start = atopos( TSD, parms->next->next->value, "WORDPOS", 3 ) ;

   end = target->value + Str_len(target) ;
   /* Then lets position right in the target */
   for (tptr=target->value; (tptr<end) && rx_isspace(*tptr) ; tptr++)  /* FGC: ordered */
   {
      ;
   }
   for (res=1; (res<start); res++)
   {
      for (; (tptr<end)&&(!rx_isspace(*tptr)); tptr++ )
      {
         ;
      }
      for (; (tptr<end) && rx_isspace(*tptr); tptr++ )
      {
         ;
      }
   }

   send = seek->value + Str_len(seek) ;
   for (sptr=seek->value; (sptr<send) && rx_isspace(*sptr); sptr++)
   {
      ;
   }
   if (sptr<send)
   {
      for ( ; (sptr<send)&&(tptr<end); )
      {
         if (contained_in( sptr, send, tptr, end ))
            break ;

         for (; (tptr<end)&&(!rx_isspace(*tptr)); tptr++)
         {
            ;
         }
         for (; (tptr<end)&&(rx_isspace(*tptr)); tptr++)
         {
            ;
         }
         res++ ;
      }
   }
   if ((sptr>=send)||((sptr<send)&&(tptr>=end)))
      res = 0 ;

   return int_to_streng( TSD, res ) ;
}


streng *std_wordlength( tsd_t *TSD, cparamboxptr parms )
{
   int i=0, number=0 ;
   streng *string=NULL ;
   char *ptr=NULL, *end=NULL ;

   checkparam(  parms,  2,  2 , "WORDLENGTH" ) ;
   string = parms->value ;
   number = atopos( TSD, parms->next->value, "WORDLENGTH", 2 ) ;

   end = (ptr=string->value) + Str_len(string) ;
   for (; (ptr<end) && rx_isspace(*ptr); ptr++)
   {
      ;
   }
   for (i=0; i<number-1; i++)
   {
      for (; (ptr<end)&&(!rx_isspace(*ptr)); ptr++)
      {
         ;
      }
      for (; (ptr<end)&&(rx_isspace(*ptr)); ptr++ )
      {
         ;
      }
   }

   for (i=0; (((ptr+i)<end)&&(!rx_isspace(*(ptr+i)))); i++)
   {
      ;
   }
   return (int_to_streng( TSD,i)) ;
}



streng *std_wordindex( tsd_t *TSD, cparamboxptr parms )
{
   int i=0, number=0 ;
   streng *string=NULL ;
   char *ptr=NULL, *end=NULL ;

   checkparam(  parms,  2,  2 , "WORDINDEX" ) ;
   string = parms->value ;
   number = atopos( TSD, parms->next->value, "WORDINDEX", 2 ) ;

   end = (ptr=string->value) + Str_len(string) ;
   for (; (ptr<end) && rx_isspace(*ptr); ptr++)
   {
      ;
   }
   for (i=0; i<number-1; i++)
   {
      for (; (ptr<end)&&(!rx_isspace(*ptr)); ptr++)
      {
         ;
      }
      for (; (ptr<end)&&(rx_isspace(*ptr)); ptr++)
      {
         ;
      }
   }

   return ( int_to_streng( TSD, (ptr<end) ? (ptr - string->value + 1 ) : 0) ) ;
}


streng *std_delword( tsd_t *TSD, cparamboxptr parms )
{
   char *rptr=NULL, *cptr=NULL, *end=NULL ;
   streng *string=NULL ;
   int length=(-1), start=0, i=0 ;

   checkparam(  parms,  2,  3 , "DELWORD" ) ;
   string = Str_dupTSD(parms->value) ;
   start = atopos( TSD, parms->next->value, "DELWORD", 2 ) ;
   if ((parms->next->next)&&(parms->next->next->value))
      length = atozpos( TSD, parms->next->next->value, "DELWORD", 3 ) ;

   end = (cptr=string->value) + Str_len(string) ;
   for (; (cptr<end) && rx_isspace(*cptr); cptr++ )
   {
      ;
   }
   for (i=0; i<(start-1); i++)
   {
      for (; (cptr<end)&&(!rx_isspace(*cptr)); cptr++)
      {
         ;
      }
      for (; (cptr<end) && rx_isspace(*cptr); cptr++)
      {
         ;
      }
   }

   rptr = cptr ;
   for (i=0; (i<(length))||((length==(-1))&&(cptr<end)); i++)
   {
      for (; (cptr<end)&&(!rx_isspace(*cptr)); cptr++ )
      {
         ;
      }
      for (; (cptr<end) && rx_isspace(*cptr); cptr++ )
      {
         ;
      }
   }

   for (; (cptr<end);)
   {
      for (; (cptr<end)&&(!rx_isspace(*cptr)); *(rptr++) = *(cptr++))
      {
         ;
      }
      for (; (cptr<end) && rx_isspace(*cptr); *(rptr++) = *(cptr++))
      {
         ;
      }
   }

   string->len = (rptr - string->value) ;
   return string ;
}


streng *std_xrange( tsd_t *TSD, cparamboxptr parms )
{
   int start=0, stop=0xff, i=0, length=0 ;
   streng *result=NULL ;

   checkparam(  parms,  0,  2 , "XRANGE" ) ;
   if ( parms->value )
      start = (unsigned char) getonechar( TSD, parms->value, "XRANGE", 1 ) ;

   if ( ( parms->next )
   && ( parms->next->value ) )
      stop = (unsigned char) getonechar( TSD, parms->next->value, "XRANGE", 2 ) ;

   length = stop - start + 1 ;
   if (length<1)
      length = 256 + length ;

   result = Str_makeTSD( length ) ;
   for (i=0; (i<length); i++)
   {
      if (start==256)
        start = 0 ;
      result->value[i] = (char) start++ ;
   }
/*    result->value[i] = (char) stop ; */
   result->len = i ;

   return result ;
}


streng *std_lastpos( tsd_t *TSD, cparamboxptr parms )
{
   int res=0, start=0, i=0, j=0, nomore=0 ;
   streng *needle=NULL, *heystack=NULL ;

   checkparam(  parms,  2,  3 , "LASTPOS" ) ;
   needle = parms->value ;
   heystack = parms->next->value ;
   if ((parms->next->next)&&(parms->next->next->value))
      start = atopos( TSD, parms->next->next->value, "LASTPOS", 3 ) ;
   else
      start = Str_len( heystack ) ;

   nomore = Str_len( needle ) ;
   if (start>Str_len(heystack))
      start = Str_len( heystack ) ;

   if (nomore>start
   ||  nomore==0)
      res = 0 ;
   else
   {
      for (i=start-nomore ; i>=0; i-- )
      {
         /*
          * FGC: following loop was "<=nomore"
          */
         for (j=0; (j<nomore)&&(needle->value[j]==heystack->value[i+j]);j++) ;
         if (j>=nomore)
         {
            res = i + 1 ;
            break ;
         }
      }
   }
   return (int_to_streng( TSD,res)) ;
}



streng *std_pos( tsd_t *TSD, cparamboxptr parms )
{
   int start=1, res=0 ;
   streng *needle=NULL, *heystack=NULL ;
   checkparam(  parms,  2,  3 , "POS" ) ;

   needle = parms->value ;
   heystack = parms->next->value ;
   if ((parms->next->next)&&(parms->next->next->value))
      start = atopos( TSD, parms->next->next->value, "POS", 3 ) ;

   if ((!needle->len)
   ||  (!heystack->len)
   ||  (start>heystack->len))
      res = 0 ;
   else
   {
      res = bmstrstr(heystack, start-1, needle, 0) + 1 ;
   }

   return (int_to_streng( TSD, res ) ) ;
}



streng *std_subword( tsd_t *TSD, cparamboxptr parms )
{
   int i=0, length=0, start=0 ;
   char *cptr=NULL, *eptr=NULL, *cend=NULL ;
   streng *string=NULL, *result=NULL ;

   checkparam(  parms,  2,  3 , "SUBWORD" ) ;
   string = parms->value ;
   start = atopos( TSD, parms->next->value, "SUBWORD", 2 ) ;
   if ((parms->next->next)&&(parms->next->next->value))
      length = atozpos( TSD, parms->next->next->value, "SUBWORD", 3 ) ;
   else
      length = -1 ;

   cptr = string->value ;
   cend = cptr + Str_len(string) ;
   for (i=1; i<start; i++)
   {
      for ( ; (cptr<cend)&&(rx_isspace(*cptr)); cptr++)
      {
         ;
      }
      for ( ; (cptr<cend)&&(!rx_isspace(*cptr)); cptr++)
      {
         ;
      }
   }
   for ( ; (cptr<cend)&&(rx_isspace(*cptr)); cptr++)
   {
      ;
   }

   eptr = cptr ;
   if (length>=0)
   {
      for( i=0; (i<length); i++ )
      {
         for (;(eptr<cend)&&(rx_isspace(*eptr)); eptr++) /* wount hit 1st time */
         {
            ;
         }
         for (;(eptr<cend)&&(!rx_isspace(*eptr)); eptr++)
         {
            ;
         }
      }
   }
   else
      eptr = cend;

   /* fixes bug 1113373 */
   while ((eptr > cptr) && rx_isspace(*(eptr-1)))
   {
      eptr--;
   }

   result = Str_makeTSD( eptr-cptr ) ;
   memcpy( result->value, cptr, (eptr-cptr) ) ;
   result->len = (eptr-cptr) ;

   return result ;
}



streng *std_symbol( tsd_t *TSD, cparamboxptr parms )
{
   int type=0 ;

   checkparam(  parms,  1,  1 , "SYMBOL" ) ;

   type = valid_var_symbol( parms->value ) ;
   if (type==SYMBOL_BAD)
      return Str_creTSD("BAD") ;

   if ( ( type != SYMBOL_CONSTANT ) && ( type != SYMBOL_NUMBER ) )
   {
      assert(type==SYMBOL_STEM||type==SYMBOL_SIMPLE||type==SYMBOL_COMPOUND);
      if (isvariable(TSD, parms->value))
         return Str_creTSD("VAR") ;
   }

   return Str_creTSD("LIT") ;
}


#if defined(TRACEMEM)
static void mark_envirvars( const tsd_t *TSD )
{
   struct envirlist *ptr=NULL ;
   bui_tsd_t *bt;

   bt = (bui_tsd_t *) TSD->bui_tsd;
   for (ptr=bt->first_envirvar; ptr; ptr=ptr->next)
   {
      markmemory( ptr, TRC_STATIC ) ;
      markmemory( ptr->ptr, TRC_STATIC ) ;
   }
}

static void add_new_env( const tsd_t *TSD, streng *ptr )
{
   struct envirlist *newElem=NULL ;
   bui_tsd_t *bt;

   bt = (bui_tsd_t *) TSD->bui_tsd;
   newElem = (struct envirlist *) MallocTSD( sizeof( struct envirlist )) ;
   newElem->next = bt->first_envirvar ;
   newElem->ptr = ptr ;

   if (!bt->first_envirvar)
      regmarker( TSD, mark_envirvars ) ;

   bt->first_envirvar = newElem ;
}
#endif

/*
 * ext_pool_value processes the request of the BIF value() and putenv() for the external
 * variable pool known as the "environment" in terms of the C library.
 *
 * name has to be a '\0'-terminated streng, value is either NULL or the
 * new content of the variable called name.
 */
streng *ext_pool_value( tsd_t *TSD, streng *name, streng *value,
                               streng *env )
{
   streng *retval=NULL;
   int ok=HOOK_GO_ON;

   (env = env); /* Make the compiler happy */

   /*
    * Get the current value from the exit if we have one, or from the
    * environment directly if not...
    */
   if ( TSD->systeminfo->hooks & HOOK_MASK( HOOK_GETENV ) )
      ok = hookup_input_output( TSD, HOOK_GETENV, name, &retval );

#ifdef VMS
   if ( ok == HOOK_GO_ON )
   {
      /*
       * Either there was no exit handler, or the exit handler didn't
       * handle the GETENV. Get the environment variable directly from
       * the system.
       */
      retval = vms_resolv_symbol( TSD, name, value, env );
   }
   else if ( value )
      exiterror( ERR_SYSTEM_FAILURE, 1, "No support for setting an environment variable" );
   /*
    * FIXME: What happens if value is set and HOOK_GO_ON isn't set?
    *        What happens with the different Pools SYMBOL, SYSTEM, LOGICAL?
    */
   return retval;
#else
   if ( ok == HOOK_GO_ON )
   {
      char *val = mygetenv( TSD, name->value, NULL, 0 );
      if ( val )
      {
         retval = Str_creTSD( val );
         FreeTSD( val );
      }
   }

   /*
    * retval is prepared. Check for setting a new value.
    */
   if ( value )
   {
      /*
       * We are setting a value in the external environment
       */

      if ( TSD->restricted )
         exiterror( ERR_RESTRICTED, 2, "VALUE", 2 );

      if ( TSD->systeminfo->hooks & HOOK_MASK( HOOK_SETENV ) )
         ok = hookup_output2( TSD, HOOK_SETENV, name, value );

      if ( ok == HOOK_GO_ON )
      {
# if defined(HAVE_MY_WIN32_SETENV)
         streng *strvalue = Str_dupstrTSD( value );

         TSD->OS->setenv(name->value, strvalue->value );
         Free_stringTSD( strvalue );
# elif defined(HAVE_SETENV)
         streng *strvalue = Str_dupstrTSD( value );

         setenv(name->value, strvalue->value, 1 );
         Free_stringTSD( strvalue );
# elif defined(HAVE_PUTENV)
         /*
          * Note: we don't release the allocated memory, because the runtime
          * system might use the pointer itself, not the content.
          * (See glibc's documentation)
          */
         streng *newstr = Str_makeTSD( Str_len( name ) + Str_len( value ) + 2 );

         Str_catTSD( newstr, name );
         Str_catstrTSD( newstr, "=" );
         Str_catTSD( newstr, value );
         newstr->value[Str_len(newstr)] = '\0';

         putenv( newstr->value );
#  ifdef TRACEMEM
         add_new_env( TSD, newstr );
#  endif
# else
         exiterror( ERR_SYSTEM_FAILURE, 1, "No support for setting an environment variable" );
# endif /* HAVE_PUTENV */
      }
   }

   return retval;
#endif /* !VMS */
}

/*
 * FGC, 07.04.2005
 * FIXME: We are not throwing 40.36, but I'm not sure we should at all.
 */
streng *std_value( tsd_t *TSD, cparamboxptr parms )
{
   streng *name,*retval;
   streng *value=NULL,*env=NULL;
   int i,err,pool=-1;

   checkparam(  parms, 1, 3 , "VALUE" );
   name = Str_dupstrTSD( parms->value );

   if ( parms->next )
   {
      value = parms->next->value;
      if ( parms->next->next )
         env = parms->next->next->value;
   }

   if ( env )
   {
      i = Str_len( env );
      if ( ( ( i == 6  ) && ( memcmp( env->value, "SYSTEM", 6 ) == 0 ) )
      ||   ( ( i == 14 ) && ( memcmp( env->value, "OS2ENVIRONMENT", 14 ) == 0 ) )
      ||   ( ( i == 11 ) && ( memcmp( env->value, "ENVIRONMENT", 11 ) == 0 ) ) )
      {
         retval = ext_pool_value( TSD, name, value, env );
         Free_stringTSD( name );
         if ( retval == NULL )
            retval = nullstringptr();

         return retval;
      }

      pool = streng_to_int( TSD, env, &err );

      /*
       * Accept a builtin pool if it is a number >= 0.
       */
      if ( pool < 0 )
         err = 1;
      if ( pool > TSD->currlevel->pool )
         err = 1;
      if ( err )
         exiterror( ERR_INCORRECT_CALL, 37, "VALUE", tmpstr_of( TSD, env ) );
   }

   /*
    * Internal variable pool; ie Rexx variables. According to ANSI standard
    * need to uppercase the variable name first.
    */
   if ( !valid_var_symbol( name ) )
   {
      Free_stringTSD( name );
      exiterror( ERR_INCORRECT_CALL, 26, "VALUE", tmpstr_of( TSD, parms->value ) );
   }

   Str_upper( name );
   retval = Str_dupTSD( get_it_anyway( TSD, name, pool ) );
   if ( value )
      setvalue( TSD, name, Str_dupTSD( value ), pool );
   Free_stringTSD( name );

   return retval;
}


streng *std_abs( tsd_t *TSD, cparamboxptr parms )
{
   checkparam(  parms, 1, 1 , "ABS" ) ;
   return str_abs( TSD, parms->value ) ;
}


streng *std_condition( tsd_t *TSD, cparamboxptr parms )
{
   char opt='I' ;
   streng *result=NULL ;
   sigtype *sig=NULL ;
   trap *traps=NULL ;
   char buf[20];

   checkparam(  parms,  0,  1 , "CONDITION" ) ;

   if (parms&&parms->value)
      opt = getoptionchar( TSD, parms->value, "CONDITION", 1, "CEIDS", "" ) ;

   result = NULL ;
   sig = getsigs(TSD->currlevel) ;
   if (sig)
      switch (opt)
      {
         case 'C':
            result = Str_creTSD( signalnames[sig->type] ) ;
            break ;

         case 'I':
            result = Str_creTSD( (sig->invoke) ? "SIGNAL" : "CALL" ) ;
            break ;

         case 'D':
            if (sig->descr)
               result = Str_dupTSD( sig->descr ) ;
            break ;

         case 'E':
            if (sig->subrc)
               sprintf(buf, "%d.%d", sig->rc, sig->subrc );
            else
               sprintf(buf, "%d", sig->rc );
            result = Str_creTSD( buf ) ;
            break ;

         case 'S':
            traps = gettraps( TSD, TSD->currlevel ) ;
            if (traps[sig->type].delayed)
               result = Str_creTSD( "DELAY" ) ;
            else
               result = Str_creTSD( (traps[sig->type].on_off) ? "ON" : "OFF" ) ;
            break ;

         default:
            /* should not get here */
            break;
      }

   if (!result)
       result = nullstringptr() ;

   return result ;
}


streng *std_format( tsd_t *TSD, cparamboxptr parms )
{
   streng *number=NULL ;
   int before=(-1), after=(-1) ;
   int esize=(-1), trigger=(-1) ;
   cparamboxptr ptr ;

   checkparam( parms, 1, 5, "FORMAT" ) ;
   number = (ptr=parms)->value ;

   if ((ptr) && ((ptr=ptr->next)!=NULL) && (ptr->value))
      before = atozpos( TSD, ptr->value, "FORMAT", 2 ) ;

   if ((ptr) && ((ptr=ptr->next)!=NULL) && (ptr->value))
      after = atozpos( TSD, ptr->value, "FORMAT", 3 ) ;

   if ((ptr) && ((ptr=ptr->next)!=NULL) && (ptr->value))
      esize = atozpos( TSD, ptr->value, "FORMAT", 4 ) ;

   if ((ptr) && ((ptr=ptr->next)!=NULL) && (ptr->value))
      trigger = atozpos( TSD, ptr->value, "FORMAT", 5 ) ;

   return str_format( TSD, number, before, after, esize, trigger ) ;
}



streng *std_overlay( tsd_t *TSD, cparamboxptr parms )
{
   streng *newstr=NULL, *oldstr=NULL, *retval=NULL ;
   char padch=' ' ;
   int length=0, spot=0, oldlen=0, i=0, j=0, k=0 ;
   paramboxptr tmpptr=NULL ;

   checkparam( parms, 2, 5, "OVERLAY" ) ;
   newstr = parms->value ;
   oldstr = parms->next->value ;
   length = Str_len(newstr) ;
   oldlen = Str_len(oldstr) ;
   if (parms->next->next)
   {
      tmpptr = parms->next->next ;
      if (parms->next->next->value)
         spot = atopos( TSD, tmpptr->value, "OVERLAY", 3 ) ;

      if (tmpptr->next)
      {
         tmpptr = tmpptr->next ;
         if (tmpptr->value)
            length = atozpos( TSD, tmpptr->value, "OVERLAY", 4 ) ;
         if ((tmpptr->next)&&(tmpptr->next->value))
            padch = getonechar( TSD, tmpptr->next->value, "OVERLAY", 5 ) ;
      }
   }

   retval = Str_makeTSD(((spot+length-1>oldlen)?spot+length-1:oldlen)) ;
   for (j=i=0;(i<spot-1)&&(i<oldlen);retval->value[j++]=oldstr->value[i++]) ;
   for (;j<spot-1;retval->value[j++]=padch) ;
   for (k=0;(k<length)&&(Str_in(newstr,k));retval->value[j++]=newstr->value[k++])
      if (i<oldlen) i++ ;

   for (;k++<length;retval->value[j++]=padch) if (oldlen>i) i++ ;
   for (;oldlen>i;retval->value[j++]=oldstr->value[i++]) ;

   retval->len = j ;
   return retval ;
}

streng *std_insert( tsd_t *TSD, cparamboxptr parms )
{
   streng *newstr=NULL, *oldstr=NULL, *retval=NULL ;
   char padch=' ' ;
   int length=0, spot=0, oldlen=0, i=0, j=0, k=0 ;
   paramboxptr tmpptr=NULL ;

   checkparam( parms, 2, 5, "INSERT" ) ;
   newstr = parms->value ;
   oldstr = parms->next->value ;
   length = Str_len(newstr) ;
   oldlen = Str_len(oldstr) ;
   if (parms->next->next)
   {
      tmpptr = parms->next->next ;
      if (parms->next->next->value)
         spot = atozpos( TSD, tmpptr->value, "INSERT", 3 ) ;

      if (tmpptr->next)
      {
         tmpptr = tmpptr->next ;
         if (tmpptr->value)
            length = atozpos( TSD, tmpptr->value, "INSERT", 4 ) ;
         if ((tmpptr->next)&&(tmpptr->next->value))
            padch = getonechar( TSD, tmpptr->next->value, "INSERT", 5) ;
      }
   }

   retval = Str_makeTSD(length+((spot>oldlen)?spot:oldlen)) ;
   for (j=i=0;(i<spot)&&(oldlen>i);retval->value[j++]=oldstr->value[i++]) ;
   for (;j<spot;retval->value[j++]=padch) ;
   for (k=0;(k<length)&&(Str_in(newstr,k));retval->value[j++]=newstr->value[k++]) ;
   for (;k++<length;retval->value[j++]=padch) ;
   for (;oldlen>i;retval->value[j++]=oldstr->value[i++]) ;
   retval->len = j ;
   return retval ;
}



streng *std_time( tsd_t *TSD, cparamboxptr parms )
{
   int hour=0 ;
   time_t unow=0, now=0, rnow=0 ;
   long usec=0L, sec=0L, timediff=0L ;
   char *ampm=NULL ;
   char format='N' ;
#ifdef __CHECKER__
   /* Fix a bug by checker: */
   streng *answer=Str_makeTSD( 64 ) ;
#else
   streng *answer=Str_makeTSD( 50 ) ;
#endif
   streng *supptime=NULL;
   streng *str_suppformat=NULL;
   char suppformat = 'N' ;
   paramboxptr tmpptr=NULL;
   struct tm tmdata, *tmptr ;

   checkparam(  parms,  0,  3 , "TIME" ) ;
   if ((parms)&&(parms->value))
      format = getoptionchar( TSD, parms->value, "TIME", 1, "CEHLMNORS", "JT" ) ;

   if (parms->next)
   {
      tmpptr = parms->next ;
      if (parms->next->value)
         supptime = tmpptr->value ;

      if (tmpptr->next)
      {
         tmpptr = tmpptr->next ;
         if (tmpptr->value)
         {
            str_suppformat = tmpptr->value;
            suppformat = getoptionchar( TSD, tmpptr->value, "TIME", 3, "CHLMNS", "T" ) ;
         }
      }
      else
      {
         suppformat = 'N';
      }
   }

   if (TSD->currentnode->now)
   {
      now = TSD->currentnode->now->sec ;
      unow = TSD->currentnode->now->usec ;
   }
   else
   {
      getsecs(&now, &unow) ;
      TSD->currentnode->now = (rexx_time *)MallocTSD( sizeof( rexx_time ) ) ;
      TSD->currentnode->now->sec = now ;
      TSD->currentnode->now->usec = unow ;
   }

   rnow = now ;

   if (unow>=(500*1000)
   &&  format != 'L')
      now ++ ;


   if ((tmptr = localtime(&now)) != NULL)
      tmdata = *tmptr;
   else
      memset(&tmdata,0,sizeof(tmdata)); /* what shall we do in this case? */

   if (supptime) /* time conversion required */
   {
      if (convert_time(TSD,supptime,suppformat,&tmdata,&unow))
      {
         char *p1, *p2;
         if (supptime && supptime->value)
            p1 = (char *) tmpstr_of( TSD, supptime ) ;
         else
            p1 = "";
         if (str_suppformat && str_suppformat->value)
            p2 = (char *) tmpstr_of( TSD, str_suppformat ) ;
         else
            p2 = "N";
         exiterror( ERR_INCORRECT_CALL, 19, "TIME", p1, p2 )  ;
      }
   }

   switch (format)
   {
      case 'C':
         hour = tmdata.tm_hour ;
         ampm = (char *)( ( hour > 11 ) ? "pm" : "am" ) ;
         if ((hour=hour%12)==0)
            hour = 12 ;
         answer->len = sprintf( answer->value, "%d:%02d%s", hour, tmdata.tm_min, ampm );
         break ;

      case 'E':
      case 'R':
         sec = (long)((TSD->currlevel->rx_time.sec) ? rnow-TSD->currlevel->rx_time.sec : 0) ;
         usec = (long)((TSD->currlevel->rx_time.sec) ? unow-TSD->currlevel->rx_time.usec : 0) ;

         if (usec<0)
         {
            usec += 1000000 ;
            sec-- ;
         }

/*         assert( usec>=0 && sec>=0 ) ; */
         if (!TSD->currlevel->rx_time.sec || format=='R')
         {
            TSD->currlevel->rx_time.sec = rnow ;
            TSD->currlevel->rx_time.usec = unow ;
         }

         /*
          * We have to cast these since time_t can be 'any' type, and
          * the format specifier can not be set to correspond with time_t,
          * then be have to convert it. Besides, we use unsigned format
          * in order not to generate any illegal numbers
          */
         if (sec)
            answer->len = sprintf( answer->value,"%ld.%06lu", (long)sec, (unsigned long)usec );
         else
            answer->len = sprintf( answer->value,".%06lu", (unsigned long)usec );
         break ;

      case 'H':
         answer->len = sprintf( answer->value, "%d", tmdata.tm_hour );
         break ;

      case 'J':
         answer->len = sprintf( answer->value, "%.06f", cpu_time() );
         break ;

      case 'L':
         answer->len = sprintf(answer->value, "%02d:%02d:%02d.%06ld", tmdata.tm_hour, tmdata.tm_min, tmdata.tm_sec, unow );
         break ;

      case 'M':
         answer->len = sprintf(answer->value, "%d", tmdata.tm_hour*60 + tmdata.tm_min);
         break ;

      case 'N':
         answer->len = sprintf(answer->value, "%02d:%02d:%02d", tmdata.tm_hour, tmdata.tm_min, tmdata.tm_sec );
         break ;

      case 'O':
#ifdef VMS
         timediff = mktime(localtime(&now));
#else
//#if defined(__OS2__) && defined(__WATCOMC__)
//         timediff = (long)(mktime2(localtime(&now))-mktime2(gmtime(&now)));
//#else
         timediff = (long)(mktime(localtime(&now))-mktime(gmtime(&now)));
//#endif
         tmptr = localtime(&now);
         if ( tmptr->tm_isdst )
            timediff += 3600;
#endif
         answer->len = sprintf( answer->value, "%ld%s", timediff,(timediff)?"000000":"" );
         break ;

      case 'S':
         answer->len = sprintf(answer->value, "%d", (((tmdata.tm_hour*60)+tmdata.tm_min) * 60) + tmdata.tm_sec);
         break ;

      case 'T':
//#if defined(__OS2__) && defined(__WATCOMC__)
//         rnow = mktime2( &tmdata );
//#else
         rnow = mktime( &tmdata );
//#endif
         answer->len = sprintf(answer->value, "%ld", rnow );
         break ;

      default:
         /* should not get here */
         break;
   }
   return answer ;
}

streng *std_date( tsd_t *TSD, cparamboxptr parms )
{
   static const char *fmt = "%02d%c%02d%c%02d" ;
   static const char *fmt1 = "%02d%02d%02d" ;
   static const char *sdate = "%04d%c%02d%c%02d" ;
   static const char *sdate1 = "%04d%02d%02d" ;
   static const char *iso = "%04d%c%02d%c%02d" ;
   static const char *iso1 = "%04d%02d%02d" ;
   static const char *ndate = "%d%c%c%c%c%c%4d" ;
   static const char *ndate1 = "%d%c%c%c%4d" ;
   char format = 'N' ;
   char suppformat = 'N' ;
   int length=0,rcode=0 ;
   const char *chptr=NULL ;
   streng *answer=Str_makeTSD( 50 ) ;
   paramboxptr tmpptr=NULL;
   streng *suppdate=NULL;
   streng *str_suppformat=NULL;
   struct tm tmdata, *tmptr ;
   time_t now=0, unow=0, rnow=0 ;
   char osep = '?', isep = '?';
   streng *str_isep=NULL;
   streng *str_osep=NULL;

   if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
      checkparam(  parms,  0,  3 , "DATE" ) ;
   else
      checkparam(  parms,  0,  5 , "DATE" ) ;
   if ((parms)&&(parms->value))
      format = getoptionchar( TSD, parms->value, "DATE", 1, "BDEMNOSUW", "CIT" ) ;

   tmpptr = parms->next ;
   if (tmpptr)
   {
      if (tmpptr->value)
         suppdate = tmpptr->value ;
      tmpptr = tmpptr->next ;
      if (tmpptr)
      {
         if (tmpptr->value)
         {
            str_suppformat = tmpptr->value;
            suppformat = getoptionchar( TSD, tmpptr->value, "DATE", 3, "BDENOSU", "IT" ) ;
         }
         tmpptr = tmpptr->next ;
         if (tmpptr)
         {
            if (tmpptr->value)
            {
               str_osep = tmpptr->value;
               if ( Str_len( tmpptr->value ) == 0 )
                  osep = '\0';
               else
                  osep = getonespecialchar( TSD, tmpptr->value, "DATE", 4 ) ;
               if ( !(format == 'E' || format == 'N' || format == 'O' || format == 'S' || format == 'U' || format == 'I') )
                  exiterror( ERR_INCORRECT_CALL, 44, "DATE", 2, tmpstr_of( TSD, suppdate ), 4 ) ;
            }
            tmpptr = tmpptr->next ;
            if (tmpptr)
            {
               if (tmpptr->value)
               {
                  str_isep = tmpptr->value;
                  if ( Str_len( tmpptr->value ) == 0 )
                     isep = '\0';
                  else
                     isep = getonespecialchar( TSD, tmpptr->value, "DATE", 5 ) ;
                  if ( !(suppformat == 'E' || suppformat == 'N' || suppformat == 'O' || suppformat == 'S' || suppformat == 'U' || suppformat == 'I') )
                     exiterror( ERR_INCORRECT_CALL, 44, "DATE", 2, tmpstr_of( TSD, suppdate ), 5 ) ;
               }
            }
         }
      }
   }
   if ( isep == '?' )
   {
      /* set default input separator */
      switch( suppformat )
      {
         case 'E':
         case 'O':
         case 'U':
            isep = '/';
            break;
         case 'S':
            isep = '\0';
            break;
         case 'I':
            isep = '-';
            break;
         case 'N':
            isep = ' ';
            break;
      }
   }
   if ( osep == '?' )
   {
       /* set default output separator */
       switch( format )
       {
          case 'E':
          case 'O':
          case 'U':
             osep = '/';
             break;
          case 'S':
             osep = '\0';
             break;
         case 'I':
            osep = '-';
            break;
          case 'N':
             osep = ' ';
             break;
       }
   }

   if (TSD->currentnode->now)
   {
      now = TSD->currentnode->now->sec ;
      unow = TSD->currentnode->now->usec ;
   }
   else
   {
      getsecs(&now, &unow) ;
      TSD->currentnode->now = (rexx_time *)MallocTSD( sizeof( rexx_time ) ) ;
      TSD->currentnode->now->sec = now ;
      TSD->currentnode->now->usec = unow ;
   }

   /*
    * MH - 3/3/2000
    * This should not be rounded up for dates. If this were
    * run at 11:59:59.500001 on 10 Jun, DATE would report back
    * 11 Jun!
   if (unow>=(500*1000))
      now ++ ;
   */

   if ( ( tmptr = localtime( &now ) ) != NULL )
      tmdata = *tmptr;
   else
      memset( &tmdata, 0, sizeof( tmdata ) ); /* what shall we do in this case? */
   tmdata.tm_year += 1900;

   if ( suppdate )
   {
      /* date conversion required */
      if ( ( rcode = convert_date( TSD, suppdate, suppformat, &tmdata, isep ) ) )
      {
         char *p1, *p2;
         if (suppdate && suppdate->value)
            p1 = (char *) tmpstr_of( TSD, suppdate ) ;
         else
            p1 = "";
         if (str_suppformat && str_suppformat->value)
            p2 = (char *) tmpstr_of( TSD, str_suppformat ) ;
         else
            p2 = "N";
         if ( rcode == 1 )
            exiterror( ERR_INCORRECT_CALL, 19, "DATE", p1, p2 )  ;
         else
            exiterror( ERR_INCORRECT_CALL, 44, "DATE", 2, p1, 5 )  ;
      }
      /*
       * Check for crazy years...
       */
      if ( tmdata.tm_year < 0 || tmdata.tm_year > 9999 )
         exiterror( ERR_INCORRECT_CALL, 18, "DATE" )  ;
   }

   switch (format)
   {
      case 'B':
         answer->len = sprintf( answer->value, "%d", tmdata.tm_yday + basedays( tmdata.tm_year ) );
         break ;

      case 'C':
         length = tmdata.tm_yday + basedays(tmdata.tm_year); /* was +1 */
         answer->len = sprintf( answer->value, "%d", length-basedays( (tmdata.tm_year/100)*100)+1 ); /* bja */
         break ;
      case 'D':
         answer->len = sprintf( answer->value, "%d", tmdata.tm_yday + 1 );
         break ;

      case 'E':
         if ( osep == '\0' )
            answer->len = sprintf( answer->value, fmt1, tmdata.tm_mday, tmdata.tm_mon+1, tmdata.tm_year%100 );
         else
            answer->len = sprintf( answer->value, fmt, tmdata.tm_mday, osep, tmdata.tm_mon+1, osep, tmdata.tm_year%100 );
         break ;

      case 'I':
         if ( osep == '\0' )
            answer->len = sprintf( answer->value, iso1, tmdata.tm_year, tmdata.tm_mon+1, tmdata.tm_mday );
         else
            answer->len = sprintf( answer->value, iso, tmdata.tm_year, osep, tmdata.tm_mon+1, osep, tmdata.tm_mday );
         break ;

      case 'M':
         chptr = months[tmdata.tm_mon] ;
         answer->len = strlen( chptr );
         memcpy( answer->value, chptr, answer->len ) ;
         break ;

      case 'N':
         chptr = months[tmdata.tm_mon] ;
         if ( osep == '\0' )
            answer->len = sprintf( answer->value, ndate1, tmdata.tm_mday, chptr[0], chptr[1], chptr[2], tmdata.tm_year );
         else
            answer->len = sprintf( answer->value, ndate, tmdata.tm_mday, osep, chptr[0], chptr[1], chptr[2], osep, tmdata.tm_year );
         break ;

      case 'O':
         if ( osep == '\0' )
            answer->len = sprintf( answer->value, fmt1, tmdata.tm_year%100, tmdata.tm_mon+1, tmdata.tm_mday );
         else
            answer->len = sprintf( answer->value, fmt, tmdata.tm_year%100, osep, tmdata.tm_mon+1, osep, tmdata.tm_mday );
         break ;

      case 'S':
         if ( osep == '\0' )
            answer->len = sprintf(answer->value, sdate1, tmdata.tm_year, tmdata.tm_mon+1, tmdata.tm_mday );
         else
            answer->len = sprintf(answer->value, sdate, tmdata.tm_year, osep, tmdata.tm_mon+1, osep, tmdata.tm_mday );
         break ;

      case 'T':
         tmdata.tm_year -= 1900;
//#if defined(__OS2__) && defined(__WATCOMC__)
//         rnow = mktime2( &tmdata );
//#else
         rnow = mktime( &tmdata );
//#endif
         answer->len = sprintf(answer->value, "%ld", rnow );
         break ;

      case 'U':
         if ( osep == '\0' )
            answer->len = sprintf( answer->value, fmt1, tmdata.tm_mon+1, tmdata.tm_mday, tmdata.tm_year%100 );
         else
            answer->len = sprintf( answer->value, fmt, tmdata.tm_mon+1, osep, tmdata.tm_mday, osep, tmdata.tm_year%100 );
         break ;

      case 'W':
         chptr = WeekDays[tmdata.tm_wday] ;
         answer->len = strlen(chptr);
         memcpy(answer->value, chptr, answer->len) ;
         break ;

      default:
         /* should not get here */
         break;
   }

   return ( answer );
}


streng *std_words( tsd_t *TSD, cparamboxptr parms )
{
   int space=0, i=0, j=0 ;
   streng *string=NULL ;
   int send=0 ;

   checkparam(  parms,  1,  1 , "WORDS" ) ;
   string = parms->value ;

   send = Str_len(string) ;
   space = 1 ;
   for (i=j=0;send>i;i++) {
      if ((!space)&&(rx_isspace(string->value[i]))) j++ ;
      space = (rx_isspace(string->value[i])) ; }

   if ((!space)&&(i>0)) j++ ;
   return( int_to_streng( TSD, j ) ) ;
}


streng *std_word( tsd_t *TSD, cparamboxptr parms )
{
   streng *string=NULL, *result=NULL ;
   int i=0, j=0, finished=0, start=0, stop=0, number=0, space=0, slen=0 ;

   checkparam(  parms,  2,  2 , "WORD" ) ;
   string = parms->value ;
   number = atopos( TSD, parms->next->value, "WORD", 2 ) ;

   start = 0 ;
   stop = 0 ;
   finished = 0 ;
   space = 1 ;
   slen = Str_len(string) ;
   for (i=j=0;(slen>i)&&(!finished);i++)
   {
      if ((space)&&(!rx_isspace(string->value[i])))
         start = i ;
      if ((!space)&&(rx_isspace(string->value[i])))
      {
         stop = i ;
         finished = (++j==number) ;
      }
      space = (rx_isspace(string->value[i])) ;
   }

   if ((!finished)&&(((number==j+1)&&(!space)) || ((number==j)&&(space))))
   {
      stop = i ;
      finished = 1 ;
   }

   if (finished)
   {
      result = Str_makeTSD(stop-start) ; /* problems with length */
      result = Str_nocatTSD( result, string, stop-start, start) ;
      result->len = stop-start ;
   }
   else
      result = nullstringptr() ;

   return result ;
}





streng *std_address( tsd_t *TSD, cparamboxptr parms )
{
   char opt = 'N';

   checkparam(  parms,  0,  1 , "ADDRESS" ) ;

   if ( parms && parms->value )
      opt = getoptionchar( TSD, parms->value, "ADDRESS", 1, "EINO", "" ) ;

   update_envirs( TSD, TSD->currlevel ) ;
   if ( opt == 'N' )
      return Str_dupTSD( TSD->currlevel->environment ) ;
   else
   {
      return get_envir_details( TSD, opt, TSD->currlevel->environment );
   }
}


streng *std_digits( tsd_t *TSD, cparamboxptr parms )
{
   checkparam(  parms,  0,  0 , "DIGITS" ) ;
   return int_to_streng( TSD, TSD->currlevel->currnumsize ) ;
}


streng *std_form( tsd_t *TSD, cparamboxptr parms )
{
   checkparam(  parms,  0,  0 , "FORM" ) ;
   return Str_creTSD( numeric_forms[TSD->currlevel->numform] ) ;
}


streng *std_fuzz( tsd_t *TSD, cparamboxptr parms )
{
   checkparam(  parms,  0,  0 , "FUZZ" ) ;
   return int_to_streng( TSD, TSD->currlevel->numfuzz ) ;
}


streng *std_abbrev( tsd_t *TSD, cparamboxptr parms )
{
   int length=0, answer=0, i=0 ;
   streng *longstr=NULL, *shortstr=NULL ;

   checkparam(  parms,  2,  3 , "ABBREV" ) ;
   longstr = parms->value ;
   shortstr = parms->next->value ;

   if ((parms->next->next)&&(parms->next->next->value))
      length = atozpos( TSD, parms->next->next->value, "ABBREV", 3 ) ;
   else
      length = Str_len(shortstr) ;

   answer = (Str_ncmp(shortstr,longstr,length)) ? 0 : 1 ;

   if ((length>Str_len(shortstr))||(Str_len(shortstr)>Str_len(longstr)))
      answer = 0 ;
   else
   {
      for (i=length; i<Str_len(shortstr); i++)
         if (shortstr->value[i] != longstr->value[i])
            answer = 0 ;
   }

   return int_to_streng( TSD, answer ) ;
}


streng *std_qualify( tsd_t *TSD, cparamboxptr parms )
{
   streng *ret=NULL;

   checkparam(  parms,  1,  1 , "QUALIFY" ) ;
   ret = ConfigStreamQualified( TSD, parms->value );
   /*
    * Returned streng is always MAX_PATH long, so it should be safe
    * to Nul terminate the ret->value
    */
   ret->value[ret->len] = '\0';
   return (ret) ;
}

streng *std_queued( tsd_t *TSD, cparamboxptr parms )
{
   int rc;

   checkparam(  parms,  0,  0 , "QUEUED" );
   rc = lines_in_stack( TSD, NULL);
   return int_to_streng( TSD, ( rc < 0 ) ? 0 : rc );
}



streng *std_strip( tsd_t *TSD, cparamboxptr parms )
{
   char option='B', padch=' ' ;
   streng *input=NULL ;
   int leading=0, trailing=0, start=0, stop=0 ;

   checkparam(  parms,  1,  3 , "STRIP" ) ;
   if ( ( parms->next )
   && ( parms->next->value ) )
      option = getoptionchar( TSD, parms->next->value, "STRIP", 2, "LTB", "" );

   if ( ( parms->next )
   && ( parms->next->next )
   && ( parms->next->next->value ) )
      padch = getonechar( TSD, parms->next->next->value, "STRIP", 3 ) ;

   input = parms->value ;
   leading = ((option=='B')||(option=='L')) ;
   trailing = ((option=='B')||(option=='T')) ;

   for (start=0;(start<Str_len(input))&&(input->value[start]==padch)&&(leading);start++) ;
   for (stop=Str_len(input)-1;(stop >=start)&&(input->value[stop]==padch)&&(trailing);stop--) ;
   if (stop<start)
      stop = start - 1 ; /* FGC: If this happens, it will crash */

   return Str_nocatTSD(Str_makeTSD(stop-start+2),input,stop-start+1, start) ;
}



streng *std_space( tsd_t *TSD, cparamboxptr parms )
{
   streng *retval=NULL, *string=NULL ;
   char padch=' ' ;
   int i=0, j=0, k=0, l=0, space=1, length=1, hole=0 ;

   checkparam(  parms,  1,  3 , "SPACE" ) ;
   if ( ( parms->next )
   && ( parms->next->value ) )
      length = atozpos( TSD, parms->next->value, "SPACE", 2 ) ;

   if ( ( parms->next )
   && ( parms->next->next )
   && ( parms->next->next->value ) )
      padch = getonechar( TSD, parms->next->next->value, "SPACE", 3 ) ;

   string = parms->value ;
   for ( i = 0; Str_in( string, i ); i++ )
   {
      if ((space)&&(string->value[i]!=' ')) hole++ ;
      space = (string->value[i]==' ') ;
   }

   space = 1 ;
   retval = Str_makeTSD(i + hole*length ) ;
   for (j=l=i=0;Str_in(string,i);i++)
   {
      if (!((space)&&(string->value[i]==' ')))
      {
         if ((space=(string->value[i]==' '))!=0)
            for (l=j,k=0;k<length;k++)
               retval->value[j++] = padch ;
         else
            retval->value[j++] = string->value[i] ;
      }
   }

   retval->len = j ;
   if ((space)&&(j))
      retval->len -= length ;

   return retval ;
}


streng *std_arg( tsd_t *TSD, cparamboxptr parms )
{
   rx_64 number=0, retval=0, tmpval=0 ;
   char flag='N' ;
   streng *value=NULL ;
   paramboxptr ptr=NULL ;

   checkparam(  parms,  0,  2 , "ARG" ) ;
   if ( ( parms )
   && ( parms->value ) )
   {
      number = atoposrx64( TSD, parms->value, "ARG", 1 ) ;
      if ( parms->next )
         flag = getoptionchar( TSD, parms->next->value, "ARG", 2, "ENO", "" ) ;
   }

   ptr = TSD->currlevel->args ;
   if (number==0)
   {
      for (retval=0,tmpval=1; ptr; ptr=ptr->next, tmpval++)
         if (ptr->value)
            retval = tmpval ;

      value = rx64_to_streng( TSD, retval ) ;
   }

   else
   {
      for (retval=1;(retval<number)&&(ptr)&&((ptr=ptr->next)!=NULL);retval++) ;
      switch (flag)
      {
         case 'E':
            retval = ((ptr)&&(ptr->value)) ;
            value = rx64_to_streng( TSD, retval ? 1 : 0 ) ;
            break;
         case 'O':
            retval = ((ptr)&&(ptr->value)) ;
            value = rx64_to_streng( TSD, retval ? 0 : 1 ) ;
            break;
         case 'N':
            if ((ptr)&&(ptr->value))
               value = Str_dupTSD(ptr->value) ;
            else
               value = nullstringptr() ;
            break;
      }
   }

   return value ;
}


#define LOGIC_AND 0
#define LOGIC_OR  1
#define LOGIC_XOR 2


static char logic( char first, char second, int ltype )
{
   switch (ltype)
   {
      case ( LOGIC_AND ) : return (char)( first & second ) ;
      case ( LOGIC_OR  ) : return (char)( first | second ) ;
      case ( LOGIC_XOR ) : return (char)( first ^ second ) ;
      default :
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
   }
   /* not reached, next line only to satisfy compiler */
   return 'X' ;
}


static streng *misc_logic( tsd_t *TSD, int ltype, cparamboxptr parms, const char *bif, int argnum )
{
   int length1=0, length2=0, i=0 ;
   char padch=' ' ;
   streng *kill=NULL ;
   streng *pad=NULL, *outstr=NULL, *str1=NULL, *str2=NULL ;

   checkparam(  parms,  1,  3 , bif ) ;
   str1 = parms->value ;

   str2 = (parms->next) ? (parms->next->value) : NULL ;
   if (str2 == NULL)
      kill = str2 = nullstringptr() ;
   else
      kill = NULL ;

   if ((parms->next)&&(parms->next->next))
      pad = parms->next->next->value ;
   else
      pad = NULL ;

   if (pad)
      padch = getonechar( TSD, pad, bif, argnum ) ;
#ifdef lint
   else
      padch = ' ' ;
#endif

   length1 = Str_len(str1) ;
   length2 = Str_len(str2) ;
   if (length2 > length1 )
   {
      streng *tmp ;
      tmp = str2 ;
      str2 = str1 ;
      str1 = tmp ;
   }

   outstr = Str_makeTSD( Str_len(str1) ) ;

   for (i=0; Str_in(str2,i); i++)
      outstr->value[i] = logic( str1->value[i], str2->value[i], ltype ) ;

   if (pad)
      for (; Str_in(str1,i); i++)
         outstr->value[i] = logic( str1->value[i], padch, ltype ) ;
   else
      for (; Str_in(str1,i); i++)
         outstr->value[i] = str1->value[i] ;

   if (kill)
      Free_stringTSD( kill ) ;
   outstr->len = i ;
   return outstr ;
}


streng *std_bitand( tsd_t *TSD, cparamboxptr parms )
{
   return misc_logic( TSD, LOGIC_AND, parms, "BITAND", 3 ) ;
}

streng *std_bitor( tsd_t *TSD, cparamboxptr parms )
{
   return misc_logic( TSD, LOGIC_OR, parms, "BITOR", 3 ) ;
}

streng *std_bitxor( tsd_t *TSD, cparamboxptr parms )
{
   return misc_logic( TSD, LOGIC_XOR, parms, "BITXOR", 3 ) ;
}


streng *std_center( tsd_t *TSD, cparamboxptr parms )
{
   int length=0, i=0, j=0, start=0, stop=0, chars=0 ;
   char padch=' ' ;
   streng *pad=NULL, *str=NULL, *ptr=NULL ;

   checkparam(  parms,  2,  3 , "CENTER" ) ;
   length = atozpos( TSD, parms->next->value, "CENTER", 2 ) ;
   str = parms->value ;
   if (parms->next->next!=NULL)
      pad = parms->next->next->value ;
   else
      pad = NULL ;

   chars = Str_len(str) ;
   if (pad==NULL)
      padch = ' ' ;
   else
      padch = getonechar( TSD, pad, "CENTER", 3 ) ;

   start = (chars>length) ? ((chars-length)/2) : 0 ;
   stop = (chars>length) ? (chars-(chars-length+1)/2) : chars ;

   ptr = Str_makeTSD( length ) ;
   for (j=0;j<((length-chars)/2);ptr->value[j++]=padch) ;
   for (i=start;i<stop;ptr->value[j++]=str->value[i++]) ;
   for (;j<length;ptr->value[j++]=padch) ;

   ptr->len = j ;
   assert((ptr->len<=ptr->max) && (j==length));

   return ptr ;
}

static unsigned num_sourcelines(const internal_parser_type *ipt)
{
   const otree *otp;

   if (ipt->first_source_line != NULL)
      return ipt->last_source_line->lineno ;

   /* must be incore_source but that value may be NULL because of a failed
    * instore[0] of RexxStart!
    */
   if ((otp = ipt->srclines) == NULL)
      return 0; /* May happen if the user doesn't provides the true
                 * source. If you set it to 1 you must return anything
                 * below for that line.
                 */
   while (otp->next)
      otp = otp->next;
   return otp->sum + otp->num;
}

streng *std_sourceline( tsd_t *TSD, cparamboxptr parms )
{
   int line, i ;
   bui_tsd_t *bt;
   const internal_parser_type *ipt = &TSD->systeminfo->tree ;
   const otree *otp;
   streng *retval;

   bt = (bui_tsd_t *)TSD->bui_tsd;
   checkparam(  parms,  0,  1 , "SOURCELINE" ) ;
   if (!parms->value)
      return int_to_streng( TSD, num_sourcelines( ipt ) ) ;

   line = atopos( TSD, parms->value, "SOURCELINE", 1 ) ;

   if (ipt->first_source_line == NULL)
   { /* must be incore_source but that value may be NULL because of a failed
      * instore[0] of RexxStart!
      */
      otp = ipt->srclines; /* NULL if incore_source==NULL */
      if (line > 0)
      {
         while (otp && ((int) otp->num < line))
         {
            line -= otp->num;
            otp = otp->next;
         }
      }
      if ((otp == NULL) || /* line not found or error */
          (line < 1))
      {
         exiterror( ERR_INCORRECT_CALL, 34, "SOURCELINE", 1, line, num_sourcelines( ipt ) )  ;
      }

      line--;
      i = otp->elems[line].length ;
      retval = Str_makeTSD( i ) ;
      retval->len = i ;
      memcpy( retval->value, ipt->incore_source + otp->elems[line].offset, i ) ;
      return(retval);
   }
   if (bt->srcline_first != ipt->first_source_line)
   {
      bt->srcline_lineno = 1 ;
      bt->srcline_first =
      bt->srcline_ptr =
      ipt->first_source_line ;
   }
   for (;(bt->srcline_lineno<line);)
   {
      if ((bt->srcline_ptr=bt->srcline_ptr->next)==NULL)
      {
         exiterror( ERR_INCORRECT_CALL, 34, "SOURCELINE", 1, line, num_sourcelines( ipt ) )  ;
      }
      bt->srcline_lineno = bt->srcline_ptr->lineno ;
   }
   for (;(bt->srcline_lineno>line);)
   {
      if ((bt->srcline_ptr=bt->srcline_ptr->prev)==NULL)
         exiterror( ERR_INCORRECT_CALL, 0 )  ;
      bt->srcline_lineno = bt->srcline_ptr->lineno ;
   }

   return Str_dupTSD(bt->srcline_ptr->line) ;
}


streng *std_compare( tsd_t *TSD, cparamboxptr parms )
{
   char padch=' ' ;
   streng *pad=NULL, *str1=NULL, *str2=NULL ;
   int i=0, j=0, value=0 ;

   checkparam(  parms,  2,  3 , "COMPARE" ) ;
   str1 = parms->value ;
   str2 = parms->next->value ;
   if (parms->next->next)
      pad = parms->next->next->value ;
   else
      pad = NULL ;

   if (!pad)
      padch = ' ' ;
   else
      padch = getonechar( TSD, pad, "COMPARE", 3) ;

   value=i=j=0 ;
   while ((Str_in(str1,i))||(Str_in(str2,j))) {
      if (((Str_in(str1,i))?(str1->value[i]):(padch))!=
          ((Str_in(str2,j))?(str2->value[j]):(padch))) {
         value = (i>j) ? i : j ;
         break ; }
      if (Str_in(str1,i)) i++ ;
      if (Str_in(str2,j)) j++ ; }

   if ((!Str_in(str1,i))&&(!Str_in(str2,j)))
      value = 0 ;
   else
      value++ ;

   return int_to_streng( TSD, value ) ;
}


streng *std_errortext( tsd_t *TSD, cparamboxptr parms )
{
   char opt = 'N';
   streng *tmp,*tmp1,*tmp2,*retstr;
   int numdec=0, errnum, suberrnum, pos=0, i;
#if 0
   const char *err=NULL;
#endif

   checkparam(  parms,  1,  2 , "ERRORTEXT" ) ;

   if (parms&&parms->next&&parms->next->value)
      opt = getoptionchar( TSD, parms->next->value, "ERRORTEXT", 2, "NS", "" ) ;
   tmp = Str_dupTSD( parms->value );
   for (i=0; i<Str_len( tmp); i++ )
   {
      if ( *( tmp->value+i ) == '.' )
      {
         numdec++;
         *( tmp->value+i) = '\0';
         pos = i;
      }
   }
   if ( numdec > 1 )
      exiterror( ERR_INCORRECT_CALL, 11, 1, tmpstr_of( TSD, parms->value ) )  ;

   if ( numdec == 1 )
   {
      tmp1 = Str_ncreTSD( tmp->value, pos );
      tmp2 = Str_ncreTSD( tmp->value+pos+1, Str_len( tmp ) - pos - 1 );
      errnum = atoposorzero( TSD, tmp1, "ERRORTEXT", 1  );
      suberrnum = atoposorzero( TSD, tmp2, "ERRORTEXT", 1 );
      Free_stringTSD( tmp1 ) ;
      Free_stringTSD( tmp2 ) ;
   }
   else
   {
      errnum = atoposorzero( TSD, tmp, "ERRORTEXT", 1  );
      suberrnum = 0;
   }
   /*
    * Only restrict the error number passed if STRICT_ANSI is in effect.
    */
   if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI )
   &&   ( errnum > 90 || suberrnum > 900 ) )
      exiterror( ERR_INCORRECT_CALL, 17, "ERRORTEXT", tmpstr_of( TSD, parms->value ) )  ;

   Free_stringTSD( tmp ) ;

   retstr = Str_dupTSD( errortext( TSD, errnum, suberrnum, (opt=='S')?1:0, 1 ) ) ;
   clear_errortext_buffers( TSD ); /* fix bug 449 */
   return retstr;
}


streng *std_length( tsd_t *TSD, cparamboxptr parms )
{
   checkparam(  parms,  1,  1 , "LENGTH" ) ;
   return int_to_streng( TSD, Str_len( parms->value )) ;
}


streng *std_left( tsd_t *TSD, cparamboxptr parms )
{
   int length=0, i=0 ;
   char padch=' ' ;
   streng *pad=NULL, *str=NULL, *ptr=NULL ;

   checkparam(  parms,  2,  3 , "LEFT" ) ;
   length = atozpos( TSD, parms->next->value, "LEFT", 2 ) ;
   str = parms->value ;
   if (parms->next->next!=NULL)
      pad = parms->next->next->value ;
   else
      pad = NULL ;

   if (pad==NULL)
      padch = ' ' ;
   else
      padch = getonechar( TSD, pad, "LEFT", 3) ;

   ptr = Str_makeTSD( length ) ;
   for (i=0;(i<length)&&(Str_in(str,i));i++)
      ptr->value[i] = str->value[i] ;

   for (;i<length;ptr->value[i++]=padch) ;
   ptr->len = length ;

   return ptr ;
}

streng *std_right( tsd_t *TSD, cparamboxptr parms )
{
   int length=0, i=0, j=0 ;
   char padch=' ' ;
   streng *pad=NULL, *str=NULL, *ptr=NULL ;

   checkparam(  parms,  2,  3 , "RIGHT" ) ;
   length = atozpos( TSD, parms->next->value, "RIGHT", 2 ) ;
   str = parms->value ;
   if (parms->next->next!=NULL)
      pad = parms->next->next->value ;
   else
      pad = NULL ;

   if (pad==NULL)
      padch = ' ' ;
   else
      padch = getonechar( TSD, pad, "RIGHT", 3 ) ;

   ptr = Str_makeTSD( length ) ;
   for (j=0;Str_in(str,j);j++) ;
   for (i=length-1,j--;(i>=0)&&(j>=0);ptr->value[i--]=str->value[j--]) ;

   for (;i>=0;ptr->value[i--]=padch) ;
   ptr->len = length ;

   return ptr ;
}


streng *std_verify( tsd_t *TSD, cparamboxptr parms )
{
   char tab[256], ch=' ' ;
   streng *str=NULL, *ref=NULL ;
   int inv=0, start=0, res=0, i=0 ;

   checkparam(  parms, 2, 4 , "VERIFY" ) ;

   str = parms->value ;
   ref = parms->next->value ;
   if ( parms->next->next )
   {
      if ( parms->next->next->value )
      {
         ch = getoptionchar( TSD, parms->next->next->value, "VERIFY", 3, "MN", "" ) ;
         if ( ch == 'M' )
            inv = 1 ;
      }
      if (parms->next->next->next)
         start = atopos( TSD, parms->next->next->next->value, "VERIFY", 4 ) - 1 ;
   }

   for (i=0;i<256;tab[i++]=0) ;
   for (i=0;Str_in(ref,i);tab[(unsigned char)(ref->value[i++])]=1) ;
   for (i=start;(Str_in(str,i))&&(!res);i++)
   {
      if (inv==(tab[(unsigned char)(str->value[i])]))
         res = i+1 ;
   }

   return int_to_streng( TSD, res ) ;
}



streng *std_substr( tsd_t *TSD, cparamboxptr parms )
{
   int rlength=0, length=0, start=0, i=0 ;
   int available, copycount;
   char padch=' ' ;
   streng *pad=NULL, *str=NULL, *ptr=NULL ;
   paramboxptr bptr=NULL ;

   checkparam(  parms,  2,  4 , "SUBSTR" ) ;
   str = parms->value ;
   rlength = Str_len( str ) ;
   start = atopos( TSD, parms->next->value, "SUBSTR", 2 ) ;
   if ( ( (bptr = parms->next->next) != NULL )
   && ( parms->next->next->value ) )
      length = atozpos( TSD, parms->next->next->value, "SUBSTR", 3 ) ;
   else
      length = ( rlength >= start ) ? rlength - start + 1 : 0;

   if ( (bptr )
   && ( bptr->next )
   && ( bptr->next->value ) )
      pad = parms->next->next->next->value ;

   if ( pad == NULL )
      padch = ' ' ;
   else
      padch = getonechar( TSD, pad, "SUBSTR", 4) ;

   ptr = Str_makeTSD( length ) ;
   i = ((rlength>=start)?start-1:rlength) ;
   /*
    * New algorithm by Julian Onions speeds up substr() by 50%
    */
   available = Str_len(str) - i;
   copycount = length > available ? available : length;
   memcpy(ptr->value, &str->value[i], copycount);
   if (copycount < length)
      memset(&ptr->value[copycount], padch, length - copycount);
   ptr->len = length;
   return ptr ;
}


static streng *minmax( tsd_t *TSD, cparamboxptr parms, const char *name,
                       int sign )
{
   /*
    * fixes bug 677645
    */
   streng *retval;
   num_descr *m,*test;
   int ccns,fuzz,StrictAnsi,result,required,argno;

   StrictAnsi = get_options_flag( TSD->currlevel, EXT_STRICT_ANSI );
   /*
    * Round the number according to NUMERIC DIGITS. This is rule 9.2.1.
    * Don't set DIGITS or FUZZ where it's possible to raise a condition.
    * We don't have a chance to set it back to the original value.
    */
   ccns = TSD->currlevel->currnumsize;
   fuzz = TSD->currlevel->numfuzz;

   required = count_params(parms, PARAM_TYPE_HARD);
   if ( !parms->value )
      exiterror( ERR_INCORRECT_CALL, 3, name, required );
   m = get_a_descr( TSD, name, 1, parms->value );
   if ( StrictAnsi )
   {
      str_round_lostdigits( TSD, m, ccns );
   }

   parms = parms->next;
   argno = 1;
   while ( parms )
   {
      argno++;
      if ( !parms->value )
         exiterror( ERR_INCORRECT_CALL, 3, name, required ); /* fixes bug 1109296 */

      test = get_a_descr( TSD, name, argno, parms->value );
      if ( StrictAnsi )
      {
         str_round_lostdigits( TSD, test, ccns );
      }

      if ( ( TSD->currlevel->currnumsize = test->size ) < m->size )
         TSD->currlevel->currnumsize = m->size;
      TSD->currlevel->numfuzz = 0;
      result = string_test( TSD, test, m ) * sign;
      TSD->currlevel->currnumsize = ccns;
      TSD->currlevel->numfuzz = fuzz;

      if ( result <= 0 )
      {
         free_a_descr( TSD, test );
      }
      else
      {
         free_a_descr( TSD, m );
         m = test;
      }
      parms = parms->next;
   }

   m->used_digits = m->size;
   retval = str_norm( TSD, m, NULL );
   free_a_descr( TSD, m );
   return retval;

}
streng *std_max( tsd_t *TSD, cparamboxptr parms )
{
   return minmax( TSD, parms, "MAX", 1 );
}



streng *std_min( tsd_t *TSD, cparamboxptr parms )
{
   return minmax( TSD, parms, "MIN", -1 );
}



streng *std_reverse( tsd_t *TSD, cparamboxptr parms )
{
   streng *ptr=NULL ;
   int i=0, j=0 ;

   checkparam(  parms,  1,  1 , "REVERSE" ) ;

   ptr = Str_makeTSD(j=Str_len(parms->value)) ;
   ptr->len = j--  ;
   for (i=0;j>=0;ptr->value[i++]=parms->value->value[j--]) ;

   return ptr ;
}

streng *std_random( tsd_t *TSD, cparamboxptr parms )
{
   int min=0, max=999, result=0 ;
#if defined(HAVE_RANDOM)
   int seed;
#else
   unsigned seed;
#endif

   checkparam(  parms,  0,  3 , "RANDOM" ) ;
   if (parms!=NULL)
   {
      if (parms->value)
      {
         if (parms->next)
            min = atozpos( TSD, parms->value, "RANDOM", 1 ) ;
         else
         {
            max = atozpos( TSD, parms->value, "RANDOM", 1 ) ;
            if ( max > 100000 )
               exiterror( ERR_INCORRECT_CALL, 31, "RANDOM", max )  ;
         }
      }
      if (parms->next!=NULL)
      {
         if (parms->next->value!=NULL)
            max = atozpos( TSD, parms->next->value, "RANDOM", 2 ) ;

         if (parms->next->next!=NULL&&parms->next->next->value!=NULL)
         {
            seed = atozpos( TSD, parms->next->next->value, "RANDOM", 3 ) ;
#if defined(HAVE_RANDOM)
            srandom( seed ) ;
#else
            srand( seed ) ;
#endif
         }
      }
   }

   if (min>max)
      exiterror( ERR_INCORRECT_CALL, 33, "RANDOM", min, max )  ;
   if (max-min > 100000)
      exiterror( ERR_INCORRECT_CALL, 32, "RANDOM", min, max )  ;

#if defined(HAVE_RANDOM)
   result = (random() % (max-min+1)) + min ;
#else
# if RAND_MAX < 100000
/*   result = (((rand() * 100) + (clock() % 100)) % (max-min+1)) + min ; */
   result = (((rand() * RAND_MAX) + rand() ) % (max-min+1)) + min ; /* pgb */
# else
   result = (rand() % (max-min+1)) + min ;
# endif
#endif
   return int_to_streng( TSD, result ) ;
}


streng *std_copies( tsd_t *TSD, cparamboxptr parms )
{
   streng *ptr=NULL ;
   int copies=0, i=0, length=0 ;

   checkparam(  parms,  2,  2 , "COPIES" ) ;

   length = Str_len(parms->value) ;
   copies = atozpos( TSD, parms->next->value, "COPIES", 2 ) * length ;
   ptr = Str_makeTSD( copies ) ;
   for (i=0;i<copies;i+=length)
      memcpy(ptr->value+i,parms->value->value,length) ;

   ptr->len = i ;
   return ptr ;
}


streng *std_sign( tsd_t *TSD, cparamboxptr parms )
{
   checkparam(  parms,  1,  1 , "SIGN" );

   return str_sign( TSD, parms->value );
}


streng *std_trunc( tsd_t *TSD, cparamboxptr parms )
{
   int decimals=0;

   checkparam(  parms,  1,  2 , "TRUNC" );
   if ( parms->next && parms->next->value )
      decimals = atozpos( TSD, parms->next->value, "TRUNC", 2 );

   return str_trunc( TSD, parms->value, decimals );
}


streng *std_translate( tsd_t *TSD, cparamboxptr parms )
{
   streng *iptr=NULL, *optr=NULL ;
   char padch=' ' ;
   streng *string=NULL, *result=NULL ;
   paramboxptr ptr=NULL ;
   int olength=0, i=0, ii=0 ;

   checkparam(  parms,  1,  4 , "TRANSLATE" ) ;

   string = parms->value ;
   if ( ( (ptr = parms->next) != NULL )
   && ( parms->next->value ) )
   {
      optr = parms->next->value ;
      olength = Str_len( optr ) ;
   }

   if ( ( ptr )
   && ( (ptr = ptr->next) != NULL )
   && ( ptr->value ) )
   {
      iptr = ptr->value ;
   }

   if ( ( ptr )
   && ( (ptr = ptr->next) != NULL )
   && ( ptr->value ) )
      padch = getonechar( TSD, ptr->value, "TRANSLATE", 4 ) ;

   result = Str_makeTSD( Str_len(string) ) ;
   for (i=0; Str_in(string,i); i++)
   {
      if ((!iptr)&&(!optr))
         result->value[i] = (char) rx_toupper(string->value[i]) ;
      else
      {
         if (iptr)
         {
            for (ii=0; Str_in(iptr,ii); ii++)
               if (iptr->value[ii]==string->value[i])
                  break ;

            if (ii==Str_len(iptr))
            {
               result->value[i] = string->value[i] ;
               continue ;
            }
         }
         else
            ii = ((unsigned char*)string->value)[i] ;

         if ((optr)&&(ii<olength))
            result->value[i] = optr->value[ii] ;
         else
            result->value[i] = padch ;
      }
   }

   result->len = i ;
   return result ;
}


streng *std_delstr( tsd_t *TSD, cparamboxptr parms )
{
   int i=0, j=0, length=0, sleng=0, start=0 ;
   streng *string=NULL, *result=NULL ;

   checkparam(  parms,  2,  3 , "DELSTR" ) ;

   sleng = Str_len((string = parms->value)) ;
   /*
    * found while fixing bug 1108868, but fast-finding Walter will create
    * a new bug item before releasing the fix I suppose ;-)   (was atozpos)
    */
   start = atopos( TSD, parms->next->value, "DELSTR", 2 ) ;

   if ((parms->next->next)&&(parms->next->next->value))
      length = atozpos( TSD, parms->next->next->value, "DELSTR", 3 ) ;
   else
      length = Str_len( string ) - start + 1 ;

   if (length<0)
      length = 0 ;

   result = Str_makeTSD( (start+length>sleng) ? start : sleng-length ) ;

   for (i=j=0; (Str_in(string,i))&&(i<start-1); result->value[i++] = string->value[j++]) ;
   j += length ;
   for (; (j<=sleng)&&(Str_in(string,j)); result->value[i++] = string->value[j++] ) ;

   result->len = i ;
   return result ;
}





static int valid_hex_const( const streng *str )
{
   const char *ptr=NULL, *end_ptr=NULL ;
   int space_stat=0 ;

   ptr = str->value ;
   end_ptr = ptr + str->len ;

   if ((end_ptr>ptr) && ((rx_isspace(*ptr)) || (rx_isspace(*(end_ptr-1)))))
   {
         return 0 ; /* leading or trailing space */
   }

   space_stat = 0 ;
   for (; ptr<end_ptr; ptr++)
   {
      if (rx_isspace(*ptr))
      {
         if (space_stat==0)
         {
            space_stat = 2 ;
         }
         else if (space_stat==1)
         {
            /* non-even number of hex digits in non-first group */
            return 0 ;
         }
      }
      else if (rx_isxdigit(*ptr))
      {
         if (space_stat)
           space_stat = ((space_stat==1) ? 2 : 1) ;
      }
      else
      {
         return 0 ; /* neither space nor hex digit */
      }
   }

   if (space_stat==1)
   {
      /* non-even number of digits in last grp, which not also first grp */
      return 0 ;
   }

   /* note: the nullstring is a valid hexstring */
   return 1 ;  /* a valid hex string */
}

static int valid_binary_const( const streng *str)
/* check for valid binary streng. returns 1 for TRUE, 0 for FALSE */
{
   char c;
   const char *ptr;
   int len,digits;

   ptr = str->value;
   if ((len = Str_len(str))==0)
      return(1); /* ANSI */
   len--; /* on last char */

   if (rx_isspace(ptr[0]) || rx_isspace(ptr[len]))
      return(0); /* leading or trailing space */
   /* ptr must consist of 0 1nd 1. After a blank follows a blank or a block
    * of four digits. Since the first block of binary digits may contain
    * less than four digits, we casn parse backwards and check only filled
    * block till we reach the start.  Thanks to ANSI testing program. */
   for (digits = 0; len >= 0; len--)
   {
      c = ptr[len];
      if (rx_isspace(c))
      {
         if ((digits % 4) != 0)
            return(0);
      }
      else if ((c != '0') && (c != '1'))
         return(0);
      digits++;
   }

   return(1);
}

streng *std_datatype( tsd_t *TSD, cparamboxptr parms )
{
   streng *string=NULL, *result=NULL ;
   char option=' ', *cptr=NULL ;
   int res;
   parambox parms_for_symbol;

   checkparam(  parms,  1,  2 , "DATATYPE" ) ;

   string = parms->value ;

   if ((parms->next)&&(parms->next->value))
   {
      option = getoptionchar( TSD, parms->next->value, "DATATYPE", 2, "ABLMNSUWX", "" ) ;
      res = 1 ;
      cptr = string->value ;
      if ((Str_len(string)==0)&&(option!='X')&&(option!='B'))
         res = 0 ;

      switch ( option )
      {
         case 'A':
            for (; cptr<Str_end(string); res = rx_isalnum(*cptr++) && res) ;
            res = ( res ) ? 1 : 0;
            break ;

         case 'B':
            res = valid_binary_const( string );
            break ;

         case 'L':
            for (; cptr<Str_end(string); res = rx_islower(*cptr++) && res ) ;
            res = ( res ) ? 1 : 0;
            break ;

         case 'M':
            for (; cptr<Str_end(string); res = rx_isalpha(*cptr++) && res ) ;
            res = ( res ) ? 1 : 0;
            break ;

         case 'N':
            res = myisnumber(TSD, string) ;
            break ;

         case 'S':
            /*
             * According to ANSI 9.3.8, this should return the result of:
             * Symbol( string ) \= 'BAD'
             * Fixes bug #737151
             */
            parms_for_symbol.next = NULL;
            parms_for_symbol.dealloc = 0;
            parms_for_symbol.value = string;
            result = std_symbol( TSD, &parms_for_symbol );
            if ( result->len == 3 && memcmp( result->value, "BAD", 3 ) == 0 )
               res = 0;
            else
               res = 1;
            Free_string_TSD( TSD,result );
            break ;

         case 'U':
            for (; cptr<Str_end(string); res = rx_isupper(*cptr++) && res ) ;
            res = ( res ) ? 1 : 0;
            break ;

         case 'W':
            res = myiswnumber( TSD, string, NULL, 0 );
            break ;

         case 'X':
            res = valid_hex_const( string ) ;
            break ;

         default:
            /* shouldn't get here */
            break;
      }
      result = int_to_streng( TSD, res ) ;
   }
   else
   {
      cptr = (char *)( ( ( string->len ) && ( myisnumber( TSD, string ) ) ) ? "NUM" : "CHAR" ) ;
      result = Str_creTSD( cptr ) ;
   }

   return result ;
}


streng *std_trace( tsd_t *TSD, cparamboxptr parms )
{
   streng *result=NULL, *string=NULL ;
   int i=0 ;
   char tc;

   checkparam(  parms,  0,  1 , "TRACE" ) ;

   result = Str_makeTSD( 3 ) ;
   if (TSD->systeminfo->interactive)
      result->value[i++] = '?' ;

   result->value[i++] = (char) TSD->trace_stat ;
   result->len = i ;

   if ( parms->value )
   {
      string = Str_dupTSD( parms->value );
      for (i = 0; i < string->len; i++ )
      {
         if ( string->value[ i ] == '?' )
            set_trace_char( TSD, '?' );
         else
            break;
      }
      /*
       * In opposite to ANSI this throws 40.21, too.
       * I assume this to be OK although "trace ?" throws 40.21.
       */
      tc = getoptionchar( TSD, Str_strp( string, '?', STRIP_LEADING ),
                                                 "TRACE",
                                                 1,
                                                 "ACEFILNOR", "" ) ;
      set_trace_char( TSD, tc );
      Free_stringTSD( string );
   }

   return result ;
}

streng *std_changestr( tsd_t *TSD, cparamboxptr parms )
{
   streng *needle=NULL, *heystack=NULL, *new_needle=NULL, *retval=NULL ;
   int neelen=0, heylen=0, newlen=0, newneelen=0, cnt=0, start=0, i=0, heypos=0, retpos=0 ;

   checkparam( parms, 3, 3, "CHANGESTR" ) ;
   needle = parms->value ;
   heystack = parms->next->value ;
   new_needle = parms->next->next->value ;

   neelen = Str_len(needle) ;
   heylen = Str_len(heystack) ;
   newneelen = Str_len(new_needle) ;

   /* find number of occurrences of needle in heystack */
   if ((!needle->len)||(!heystack->len)||(needle->len>heystack->len))
      cnt = 0 ;
   else
   {
      for(;;)
      {
        start = bmstrstr(heystack, start, needle, 0);
        if (start == (-1))
           break;
        cnt++;
        start += needle->len;
      }
   }
   newlen = 1 + heylen + ((newneelen-neelen) * cnt);
   retval = Str_makeTSD(newlen) ;

   if (!cnt)
      return (Str_ncpyTSD(retval,heystack,heylen));

   start=heypos=retpos=0;
   for(;;)
   {
     start = bmstrstr(heystack, start, needle, 0);
     if (start == (-1))
       {
        cnt = heylen-heypos;
        for(i=0;i<cnt;retval->value[retpos++]=heystack->value[heypos++],i++) ;
        break;
       }
     cnt = start-heypos;
     for(i=0;i<cnt;retval->value[retpos++]=heystack->value[heypos++],i++) ;
     for(i=0;i<neelen;heypos++,i++) ;
     for(i=0;i<newneelen;retval->value[retpos++]=new_needle->value[i++]) ;
     start = heypos;
   }

   retval->value[retpos] = '\0';
   retval->len=retpos;
   return retval ;
}

streng *std_countstr( tsd_t *TSD, cparamboxptr parms )
{
   int start=0, cnt=0 ;
   streng *needle=NULL, *heystack=NULL ;
   checkparam(  parms,  2,  2 , "COUNTSTR" ) ;

   needle = parms->value ;
   heystack = parms->next->value ;

   if ((!needle->len)||(!heystack->len))
      cnt = 0 ;
   else
   {
      for(;;)
      {
        start = bmstrstr(heystack, start, needle, 0);
        if (start == (-1))
           break;
        cnt++;
        start += needle->len;
      }
   }

   return (int_to_streng( TSD, cnt ) ) ;
}

streng *rex_poolid( tsd_t *TSD, cparamboxptr parms )
{
   checkparam(  parms,  0,  0 , "POOLID" );

   return ( int_to_streng( TSD, TSD->currlevel->pool ) );
}

streng *rex_lower( tsd_t *TSD, cparamboxptr parms )
{
   int rlength=0, length=0, start=1, i=0 ;
   int changecount;
   char padch=' ' ;
   streng *str=NULL, *ptr=NULL ;
   paramboxptr bptr=NULL ;

   /*
    * Check that we have between 1 and 4 args
    * ( str [,start[,length[,pad]]] )
    */
   checkparam(  parms,  1,  4 , "LOWER" ) ;
   str = parms->value ;
   rlength = Str_len( str ) ;
   /*
    * Get starting position, if supplied...
    */
   if ( parms->next != NULL
   &&   parms->next->value )
      start = atopos( TSD, parms->next->value, "LOWER", 2 ) ;
   /*
    * Get length, if supplied...
    */
   if ( parms->next != NULL
   && ( (bptr = parms->next->next) != NULL )
   && ( parms->next->next->value ) )
      length = atozpos( TSD, parms->next->next->value, "LOWER", 3 ) ;
   else
      length = ( rlength >= start ) ? rlength - start + 1 : 0;
   /*
    * Get pad character, if supplied...
    */
   if ( (bptr )
   && ( bptr->next )
   && ( bptr->next->value ) )
      padch = getonechar( TSD, parms->next->next->next->value, "LOWER", 4) ;
   /*
    * Create our new starting; duplicate of input string
    */
   ptr = Str_makeTSD( length );
   memcpy( Str_val( ptr ), Str_val( str ), Str_len( str ) );
   /*
    * Determine where to start changing case...
    */
   i = ((rlength>=start)?start-1:rlength) ;
   /*
    * Determine how many characters to change case...
    */
   changecount = length > rlength ? rlength : length;
   /*
    * Change them
    */
   mem_lower( &ptr->value[i], changecount );
   /*
    * Append pad characters if required...
    */
   if (changecount < length)
      memset(&ptr->value[changecount], padch, length - changecount);
   /*
    * Determine length of return string...
    */
   ptr->len = (length > rlength) ? length : rlength ;
   return ptr ;
}
