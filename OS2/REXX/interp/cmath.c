#ifndef lint
static char *RCSid = "$Id: cmath.c,v 1.5 2004/02/10 10:43:47 mark Exp $";
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

/*
 * More attention should be paid to overflow and underflow conditions.
 * This *is* done in some of the routines for the GCC compiler, but should
 * be done in an ANSI-compatible manner.
 */


#include "rexx.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#ifdef SunKludges
double strtod( const char *, char ** ) ;
#endif

double myatof( const tsd_t *TSD, const streng *string )
{
   char *str,*ptr ;
   double answer ;

   str = str_ofTSD(string) ;
   answer = strtod(str,&ptr) ;

   /* remove leading spaces */
   for (;(*ptr)&&(rx_isspace(*ptr));ptr++) ;
#ifdef SUNOS_STRTOD_BUG
   for (;*ptr=='0';ptr++) ;
#endif /* SUNOS_STRTOD_BUG */
   if (*ptr)
       exiterror( ERR_BAD_ARITHMETIC, 0 )  ;
   FreeTSD( str ) ;

   return answer ;
}


int myisnumber( const streng *string )
{
   const register char *ptr=NULL, *eptr=NULL ;
   int num=0 ;

   if (!string->len)
      return 0 ;

   ptr = string->value ;
   eptr = Str_end( string ) ;

   for (; (ptr<eptr) && (rx_isspace(*ptr)); ptr++) ;
   if ((ptr<eptr) && ((*ptr=='-') || (*ptr=='+')))
      for (ptr++; (ptr<eptr) && (rx_isspace(*ptr)); ptr++) ;

   for (; (ptr<eptr) && rx_isdigit(*ptr); ptr++, num++) ;
   if ((ptr<eptr) && *ptr=='.')
      for (ptr++;(ptr<eptr) && rx_isdigit(*ptr); ptr++, num++) ;

   if (!num)
      return 0 ;

   if ((ptr<eptr) && ((*ptr=='e') || (*ptr=='E')))
   {
      ptr++ ;
      num = 0 ;
      if ((ptr<eptr) && ((*ptr=='-') || (*ptr=='+')))
         ptr++ ;

      for (; (ptr<eptr) && rx_isdigit(*ptr); ptr++, num++ ) ;
      if (!num)
         return 0 ;
   }

   for (; (ptr<eptr) && (rx_isspace(*ptr)); ptr++) ;
   return (ptr==eptr) ;
}


/*
 * Takes 'string' as parameter, analyze whether it is an integer, and
 * return a boolean variable which is true iff 'string' is a legal
 * integer. The format of a legal integer is (the regexpr):
 *        [ ]*([-+][ ]*)?[0-9]+[ ]*
 */
int myisinteger( const streng *string )
{
   const char *cptr=NULL, *eptr=NULL ;

   cptr = string->value ;
   eptr = cptr + string->len ;

   for (;cptr<eptr && rx_isspace(*cptr); cptr++) ;
   if (cptr<eptr && (*cptr=='-' || *cptr=='+'))
      for (cptr++; cptr<eptr && rx_isspace(*cptr); cptr++) ;

   if (cptr>=eptr)
      return 0 ;

   for (;cptr<eptr && rx_isdigit(*cptr); cptr++) ;
   for (;cptr<eptr && rx_isspace(*cptr); cptr++) ;
   return (cptr==eptr) ;
}
