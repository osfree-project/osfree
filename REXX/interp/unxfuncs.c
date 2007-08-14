#ifndef lint
static char *RCSid = "$Id: unxfuncs.c,v 1.2 2003/12/11 04:43:24 prokushev Exp $";
#endif

/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992  Anders Christensen <anders@pvv.unit.no>
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

/*
 * _XOPEN_SOURCE required for crypt()
 * but it stuffs up SCO and QNX-RTO :-(
 */
#if !defined(_SCO_ELF) && !defined(_SCO_COFF) && !defined(__QNXNTO__)
# ifndef _XOPEN_SOURCE
#  define _XOPEN_SOURCE
#  define _GNU_SOURCE
# endif
#endif

#ifndef _GNU_SOURCE
/* At least Linux needs _GNU_SOURCE and _XOPEN_SOURCE together to allow
 * a warning free compile.
 */
# define _GNU_SOURCE
#endif

#define HAVE_FORK
#if defined(__WATCOMC__) || defined(_MSC_VER)  || (defined(__IBMC__) && defined(WIN32)) || defined(__SASC) || defined(__MINGW32__) || defined(__BORLANDC__) || defined(DOS) || defined(__WINS__) || defined(__EPOC32__) ||defined(__LCC__)
# undef HAVE_FORK
#endif
#if defined(__WATCOMC__) && defined(__QNX__)
# define HAVE_FORK
#endif

#include "rexx.h"
#include <stdio.h>
#include <string.h>

#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif

#include <errno.h>

#if defined(MAC)
# include <types.h>
typedef struct utsname
{
   char *sysname ;
   char *nodename ;
   char *release ;
   char *version ;
   char *machine ;
} _utsname ;
int uname (struct utsname *name);
#else
# include <sys/types.h>
# ifdef HAVE_UNISTD_H
#  include <unistd.h>
# endif
# if defined(WIN32) && defined(__IBMC__)
#   include <process.h>
#   include <direct.h>
typedef struct utsname
{
   char *sysname ;
   char *nodename ;
   char *release ;
   char *version ;
   char *machine ;
} _utsname ;
# elif defined(__WATCOMC__) || defined(_MSC_VER) || defined(__SASC) || defined(__MINGW32__) || defined(__BORLANDC__) || defined(__WINS__) || defined(__EPOC32__) || defined(__LCC__)
#  include "utsname.h"
#  if !defined(__SASC) && !defined(__QNX__) && !defined(__WINS__) && !defined(__EPOC32__)
#   include <process.h>
#   include <direct.h>
#  endif
# else
#  ifndef VMS
#  include <sys/utsname.h>
#  endif
# endif
#endif

streng *unx_getpath( tsd_t *TSD, cparamboxptr dummy )
{
   TSD = TSD; /* keep compiler happy */
   dummy = dummy; /* keep compiler happy */
   return nullstringptr() ;
}


streng *unx_popen( tsd_t *TSD, cparamboxptr parms )
{
   streng *string=NULL, *result=NULL ;
   streng *cptr=NULL ;
   int length=0, lines=0, hl ;
   int save_internal_queues_option;

   if ( TSD->restricted )
      exiterror( ERR_RESTRICTED, 1, "POPEN" )  ;

   checkparam(  parms,  1,  2 , "POPEN" ) ;
   string = (parms->value) ;

   /*
    * Because this sort of redirection is only valid with internal
    * queues, we need to make regina think we are using internal
    * queues for this function.
    */
   save_internal_queues_option = get_options_flag( TSD->currlevel, EXT_INTERNAL_QUEUES ) ;
   set_options_flag( TSD->currlevel, EXT_INTERNAL_QUEUES, 1 ) ;

   cptr = Str_makeTSD( length=Str_len(string) + 6 ) ;
   cptr = Str_catTSD( cptr, string ) ;
   cptr = Str_catstrTSD( cptr, ">LIFO" ) ;

   if (parms->next && parms->next->value)
   {
      lines = lines_in_stack( TSD, NULL );
      if ( lines < 0 )
         lines = 0;
   }

   result = perform( TSD, cptr, TSD->currlevel->environment, TSD->currentnode, NULL ) ;
   Free_stringTSD( cptr ) ;

   if (parms->next && parms->next->value)
   {
      streng *varname=NULL, *varstem=NULL ;
      int stemlen=0 ;
      char *eptr=NULL ;
      streng *tmpptr=NULL ;

      varstem = parms->next->value ;
      varname = Str_makeTSD( (stemlen=varstem->len) + 8 ) ;

      memcpy( varname->value, varstem->value, stemlen ) ;
      mem_upper( varname->value, stemlen );
      eptr = varname->value + stemlen ;

      if (*(eptr-1)!='.')
      {
         *((eptr++)-1) = '.' ;
         stemlen++ ;
      }

      hl = lines_in_stack( TSD, NULL );
      lines = ( ( hl < 0 ) ? 0 : hl ) - lines ;
      *eptr = '0' ;
      varname->len = stemlen+1 ;
      tmpptr = int_to_streng( TSD, lines ) ;
      setvalue( TSD, varname, tmpptr ) ;
      for (; lines>0; lines--)
      {
         tmpptr = popline( TSD, NULL, NULL, 0 ) ;
         sprintf(eptr, "%d", lines ) ;
         varname->len = strlen( varname->value ) ;
         setvalue( TSD, varname, tmpptr ) ;
      }
      Free_stringTSD( varname ); /* bja */
   }
   set_options_flag( TSD->currlevel, EXT_INTERNAL_QUEUES, save_internal_queues_option ) ;

   return result ;

/*   Free_stringTSD( cptr ) ;
   sprintf( (result=Str_makeTSD(SMALLSTR))->value, "%d", rcode ) ;
   result->len = Str_len(result) ;
   return result ; */
}


streng *unx_getpid( tsd_t *TSD, cparamboxptr parms )
{
   checkparam(  parms,  0,  0 , "GETPID" ) ;
   return int_to_streng( TSD, getpid() ) ;
}

streng *unx_gettid( tsd_t *TSD, cparamboxptr parms )
{
   checkparam(  parms,  0,  0 , "GETTID" ) ;
   return int_to_streng( TSD, TSD->thread_id ) ;
}


streng *unx_eof( tsd_t *TSD, cparamboxptr parms )
{
   checkparam(  parms,  0,  0 , "EOF" ) ;
/*    sprintf(ptr=MallocTSD(SMALLSTR),"%d",eof_on_input()) ; */
   return( nullstringptr() ) ;
}


streng *unx_uname( tsd_t *TSD, cparamboxptr parms )
{
   char option=' ' ;
   char *cptr=NULL ;
   int length=0 ;
   streng *result=NULL ;
   struct utsname utsbox ;

   checkparam(  parms,  0,  1 , "UNAME" ) ;
   if (parms->value)
      option = getoptionchar( TSD, parms->value, "UNAME", 1, "ASMNRV", "" ) ;
   else
      option = 'A' ;

   if (uname( &utsbox ) <0)
       exiterror( ERR_SYSTEM_FAILURE, 1, strerror( errno ) )  ;

   switch( option )
   {
      case 'A':
         result = Str_makeTSD( 1+sizeof(struct utsname)) ;
         sprintf( result->value, "%s %s %s %s %s", utsbox.sysname,
                  utsbox.nodename, utsbox.release, utsbox.version,
                  utsbox.machine ) ;
         result->len = strlen( result->value ) ;
         assert( result->len <= result->max ) ;
         return result ;

      case 'S': cptr = utsbox.sysname ; break ;
      case 'N': cptr = utsbox.nodename ; break ;
      case 'R': cptr = utsbox.release ; break ;
      case 'V': cptr = utsbox.version ; break ;
      case 'M': cptr = utsbox.machine ; break ;
      default:
         assert( 0 ) ;
   }

   length = strlen(cptr);
   result = Str_makeTSD( length+1 ) ;
   memcpy( result->value, cptr, length ) ;
   result->len = length ;
   assert( result->len <= result->max ) ;
   return result ;
}


streng *unx_fork( tsd_t *TSD, cparamboxptr parms )
{
   int i=1 ;

   checkparam(  parms,  0,  0 , "FORK" ) ;
#if defined(HAVE_FORK)
   i = fork() ;
#endif                                                  /* MH 10-06-96 */
   return int_to_streng( TSD, i ) ;
}


#if 0
char *unx_unixerror( tsd_t *TSD, cparamboxptr parms )
{
   const char *errtxt=NULL ;
   char *result=NULL ;
   int errnum=0 ; /* change name from errno to not conflist with global errno */

   checkparam(  parms,  1,  1 , "UNIXERROR" ) ;
   errnum = atozpos( TSD, parms->value, "UNIXERROR", 1 ) ;
   errtxt = strerror(errnum) ;
/* unixerror returns char, not streng !                                          bja
   strcpy( result=MallocTSD(strlen(errtxt)+1+STRHEAD), errtxt ) ;
 */                                                                            /* bja */
   strcpy( result=MallocTSD(strlen(errtxt)+1), errtxt ) ;
   return result ;
}
#else
streng *unx_unixerror( tsd_t *TSD, cparamboxptr parms )
{
   const char *errtxt=NULL ;
   int errnum=0 ; /* change name from errno to not conflist with global errno */

   checkparam(  parms,  1,  1 , "UNIXERROR" ) ;
   errnum = atozpos( TSD, parms->value, "UNIXERROR", 1 ) ;
   errtxt = strerror(errnum) ;
   return Str_creTSD( errtxt ) ;
}
#endif



streng *unx_chdir( tsd_t *TSD, cparamboxptr parms )
{
   char *path;
   int rc=0;
   int ok=HOOK_GO_ON ;

   checkparam(  parms,  1,  1 , "CD" ) ;
   /*
    * Check if we have a registred system exit
    */
   if (TSD->systeminfo->hooks & HOOK_MASK(HOOK_SETCWD))
      ok = hookup_output( TSD, HOOK_SETCWD, parms->value ) ;

   if (ok==HOOK_GO_ON)
   {
      path = str_of( TSD, parms->value ) ;
      rc = chdir( path ) ;
      FreeTSD( path ) ;
   }
   return int_to_streng( TSD, rc!=0 ) ;
}


streng *unx_getenv( tsd_t *TSD, cparamboxptr parms )
{
   streng *retval=NULL ;
   char *output=NULL ;
   char *path ;
   int ok=HOOK_GO_ON ;

   checkparam(  parms,  1,  1 , "GETENV" ) ;

   if (TSD->systeminfo->hooks & HOOK_MASK(HOOK_GETENV))
      ok = hookup_input_output( TSD, HOOK_GETENV, parms->value, &retval ) ;

   if (ok==HOOK_GO_ON)
   {
      path = str_of( TSD, parms->value ) ;
      output = mygetenv( TSD, path, NULL, 0 ) ;
      FreeTSD( path ) ;
      if ( output )
      {
         retval = Str_creTSD( output ) ;
         FreeTSD( output );
      }
      else
         retval = nullstringptr() ;
   }
   return retval ;
}

streng *unx_crypt( tsd_t *TSD, cparamboxptr parms )
{
   streng *retval=NULL ;
#ifdef HAVE_CRYPT
   char *output=NULL ;
   char *key, *salt ;
#endif
   int ch2,i;

   checkparam(  parms,  2,  2 , "CRYPT" ) ;
   for ( i = 0; i < Str_len( parms->next->value ); i++ )
   {
      ch2 = (int)*(parms->next->value->value+i);
      if ((ch2 >= (int)'A' && ch2 <= (int)'Z')
      ||  (ch2 >= (int)'a' && ch2 <= (int)'z')
      ||  (ch2 >= (int)'0' && ch2 <= (int)'9')
      ||  (ch2 == (int)'.' || ch2 == (int)'/'))
      {
      }
      else
      {
         char tmp[2];
         tmp[0] = (char)ch2;
         tmp[1] = '\0';
         exiterror( ERR_INCORRECT_CALL, 914, "CRYPT", 2, "A-Za-z0-9./", tmp )  ;
      }
   }
#ifdef HAVE_CRYPT
   key = str_of( TSD, parms->value );
   salt = str_of( TSD, parms->next->value );
   output = crypt( key, salt ) ;
   FreeTSD( salt ) ;
   FreeTSD( key ) ;
   if (output)
      retval = Str_creTSD(output) ;
   else
      retval = nullstringptr() ;
#else
   retval = Str_dup( parms->value );
#endif
   TSD = TSD; /* keep compiler happy */
   return retval ;
}

