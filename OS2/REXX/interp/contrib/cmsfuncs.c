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

/*
 * Bug in LCC complier wchar.h that incorrectly says it defines stat struct
 * but doesn't
 */
#if defined(__LCC__)
# include <sys/stat.h>
#endif

#include "rexx.h"

#if defined(MAC)
# include "mac.h"
#else
# if defined(VMS)
#  include <stat.h>
# else
#  include <sys/stat.h>
#  ifdef HAVE_UNISTD_H
#   include <unistd.h>
#  endif
# endif
#endif

#include <stdio.h>
#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif

#if defined(__WATCOMC__) && !defined(__QNX__)
# include <dos.h>
#endif

#if defined(WIN32)
# ifdef _MSC_VER
#  if _MSC_VER >= 1100
/* Stupid MSC can't compile own headers without warning at least in VC 5.0 */
#   pragma warning(disable: 4115 4201 4214 4514)
#  endif
# endif
# include <windows.h>
# ifdef _MSC_VER
#  if _MSC_VER >= 1100
#   pragma warning(default: 4115 4201 4214)
#  endif
# endif
#endif

/*
 * Since development of Ultrix has ceased, and they never managed to
 * fix a few things, we want to define a few things, just in order
 * to kill a few warnings ...
 */
#if defined(FIX_PROTOS) && defined(FIX_ALL_PROTOS) && defined(ultrix)
   int fstat( int fd, struct stat *buf ) ;
   int stat( char *path, struct stat *buf ) ;
#endif

streng *cms_sleep( tsd_t *TSD, cparamboxptr parms )
{
   checkparam(  parms,  1,  1, "SLEEP" ) ;
#if defined(WIN32) && (defined(_MSC_VER) || defined(__IBMC__) || defined(__BORLANDC__) || defined(__MINGW32__))
   Sleep( (int)((myatof(TSD,parms->value))*1000) ) ;
#else
#if defined(HAVE_USLEEP)
   usleep( (int)((myatof(TSD,parms->value))*1000*1000) ) ;
#else
   sleep( atozpos( TSD, parms->value, "SLEEP", 1 ) ) ;
#endif
#endif
   return nullstringptr() ;
}


streng *cms_makebuf( tsd_t *TSD, cparamboxptr parms )
{
   checkparam(  parms,  0,  0 , "MAKEBUF" ) ;
   return int_to_streng( TSD,make_buffer( TSD )) ;
}



streng *cms_justify( tsd_t *TSD, cparamboxptr parms )
{
   int inspace=0, i=0, count=0, between=0, extra=0, initial=0;
   int spaces=0, chars=0, length=0 ;
   char *cend=NULL, *cp=NULL, *cptr=NULL, *out=NULL, *oend=NULL ;
   char pad=' ' ;
   streng *result=NULL ;

   checkparam(  parms,  2,  3 , "JUSTIFY" ) ;

   cptr = parms->value->value ;
   cend = cptr + parms->value->len ;

   length = atozpos( TSD, parms->next->value, "JUSTIFY", 2 ) ;
   if (parms->next->next && parms->next->next->value)
      pad = getonechar( TSD, parms->next->next->value, "JUSTIFY", 3 ) ;
   else
      pad = ' ' ;

   inspace = 1 ;
   spaces = 0 ;
   chars = 0 ;
   for (cp=cptr; cp<cend; cp++)
   {
      if (inspace)
      {
         if (!rx_isspace(*cp))
         {
            chars++ ;
            inspace = 0 ;
         }
      }
      else
      {
         if (!rx_isspace(*cp))
            chars++ ;
         else
         {
            spaces++ ;
            inspace = 1 ;
         }
      }
   }

   if (inspace && spaces)
      spaces-- ;

   result = Str_makeTSD( length ) ;
   if (chars+spaces>length || spaces==0)
   {
      between = 1 ;
      extra = 0 ;
      initial = 0 ;
   }
   else
   {
      extra = (length - chars) % spaces ;
      between = (length - chars) / spaces ;
      initial = (spaces - extra) / 2 ;
   }

   count = 0 ;
   out = result->value ;
   oend = out + length ;
   cp = cptr ;
   for (; cp<cend && rx_isspace(*cp); cp++) ;
   for (; cp<cend && out<oend; cp++)
   {
      if (rx_isspace(*cp))
      {
         for (;cp<cend && rx_isspace(*cp); cp++) ;
         for (i=0; i<between && out<oend; i++)
            *(out++) = pad ;
         if (count<initial)
            count++ ;
         else if (extra && out<oend)
         {
            extra-- ;
            *(out++) = pad ;
         }
         if (out<oend)
            *(out++) = *cp ;
      }
      else
         *(out++) = *cp ;
   }

   for (; out<oend; out++)
      *out = pad ;

   assert( out - result->value == length ) ;
   result->len = length ;

   return result ;
}



streng *cms_find( tsd_t *TSD, cparamboxptr parms )
{
   parambox newparms[3];

   checkparam(  parms,  2,  3 , "FIND" ) ;

   /* Rebuild parms but switch the first two parameters */

   memset(newparms, 0, sizeof(newparms) ) ; /* sets dealloc to 0, too */
   newparms[0].value = parms->next->value;
   newparms[0].next  = newparms + 1;
   newparms[1].value = parms->value;
   if (parms->next->next)
      {
         newparms[1].next  = newparms + 2;
         newparms[2].value  = parms->next->next->value;
      }

   return std_wordpos( TSD, newparms ) ;
}


streng *cms_index( tsd_t *TSD, cparamboxptr parms )
{
   parambox newparms[3];

   checkparam(  parms,  2,  3 , "INDEX" ) ;

   /* Rebuild parms but switch the first two parameters */

   memset(newparms, 0, sizeof(newparms) ) ; /* sets dealloc to 0, too */
   newparms[0].value = parms->next->value;
   newparms[0].next  = newparms + 1;
   newparms[1].value = parms->value;
   if (parms->next->next)
      {
         newparms[1].next  = newparms + 2;
         newparms[2].value  = parms->next->next->value;
      }

   return std_pos( TSD, newparms ) ;
}

streng *cms_desbuf( tsd_t *TSD, cparamboxptr parms )
{
   checkparam(  parms,  0,  0 , "DESBUF" ) ;
   return( int_to_streng( TSD,drop_buffer( TSD, 0))) ;
}


streng *cms_buftype( tsd_t *TSD, cparamboxptr parms )
{
   checkparam(  parms,  0,  0 , "BUFTYPE" ) ;
   type_buffer( TSD ) ;
   return (nullstringptr()) ;
}


streng *cms_dropbuf( tsd_t *TSD, cparamboxptr parms )
{
   int buffer=(-1) ;

   checkparam(  parms,  0,  1 , "DROPBUF" ) ;
   if (parms->value)
      buffer = myatol(TSD, parms->value) ;

   return( int_to_streng( TSD,drop_buffer(TSD, buffer))) ;
}


#ifdef HAS_SCANDIR
/* this part of the code is not used */

static int select_file( const struct direct *entry )
{
   return !(strcmp(entry->d_name,filename)) ;
}


streng *cms_state( tsd_t *TSD, cparamboxptr parms )
{
   struct direct *names=NULL ;
   int last=0, result=0 ;
   char *dir=NULL, *string=NULL, *retval=NULL ;

   checkparam(  parms,  1,  1 , "STATE" ) ;
   last = strlen(string=parms->value) ;
   for (;(string[last]!=FILE_SEPARATOR)&&(last>0);last--) ;
   if (last)
   {
      string[last] = '\000' ;
      dir = string ;
   }
   else
      dir = "." ;

   result = scandir(dir,&names,&select_file,NULL) ;
   if (last)
      string[last] = FILE_SEPARATOR ;

   /* Ought to open or stat the file to check if it is readable */

   return int_to_streng( TSD,result==1) ;
}
#else


streng *cms_state( tsd_t *TSD, cparamboxptr parms )
{
   /* this is a bit too easy ... but STREAM() function should handle it */
   streng *retval=NULL ;
   int rcode=0 ;
   struct stat buffer ;
   char *fn;

   checkparam(  parms,  1,  1 , "STATE" ) ;
   retval = Str_makeTSD( BOOL_STR_LENGTH ) ;

   /* will generate warning under Ultrix, don't care */
   fn = str_of(TSD,parms->value);
   rcode = stat( fn, &buffer ) ;
   FreeTSD(fn);
   return int_to_streng( TSD,rcode!=0) ;

}
#endif

