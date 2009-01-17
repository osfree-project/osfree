/*
 *  Generic Call Interface for Rexx
 *  Copyright © 2003-2004, Florian Groﬂe-Coosmann
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
 *
 * ----------------------------------------------------------------------------
 *
 * This file converts numerical string values to binary values and vice versa.
 *
 * Supported datatypes must be configured from outside in the file
 * gci_convert.h. We need functions for parsing and printing floating points
 * and integers.
 *
 * We support integers with a size of a power of 2 only, typically
 * 1, 2, 4, 8 and probably 16; every type with a smaller value will be
 * accepted automatically.
 *
 * This is expected by the include file to be defined:
 * GCI_I_x  signed integer data type with x bytes.
 * GCI_I_xm minimum value for GCI_I_x.
 * GCI_I_xM maximum value for GCI_I_x.
 * GCI_Ir   function for converting GCI_I_x, x largest of all x, which
 *          converts a string into a binary representation. The function
 *          must behave as strtol with the fitting data type.
 * GCI_Iw   function for converting GCI_I_x, x largest of all x, which
 *          converts a binary representation of a number into a zero
 *          terminated string. The prototype must fit "GCI_Iw(string,GCI_I)".
 *          No error throwing is allowed. The maximum number of characters to
 *          write is 127. The function must return the length of string
 *          without the terminating '\0'.
 *
 *
 * GCI_U_x
 * GCI_U_xm
 * GCI_U_xM
 * GCI_Ur
 * GCI_Uw   same as above but for unsigned datatypes.
 *
 * GCI_F_x
 * GCI_F_xm
 * GCI_F_xM
 * GCI_Fr
 * GCI_Fw   same as above but for floating point data types. GCI_Fw may
 *          write a maximum of 127 characters and may use "NaN", etc to
 *          present anomal numbers.
 *
 *
 * One can take advantage of strtobigl and strtobigul for GCI_Ir and GCI_Ur
 * is NEED_STRTOBIGL or NEED_STRTOBIGUL are defined. They are slow but work.
 *
 * strtobigf can be used for GCI_Fr if NEED_STRTOBIGF is defined. In this case,
 * F_SCAN must be defined and should be the scanf() compatible mask for parsing
 * a GCI_F_x value. Most systems need a '#define F_SCAN "%Lf"' in this case.
 */

#include "gci_convert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>

#undef GCI_I
#undef GCI_Imax
#undef GCI_U
#undef GCI_Umax
#undef GCI_F
#undef GCI_Fmax
#undef GCI_max

/*
 * Determine the maximum size for each family.
 */
#if   defined(GCI_I_16)
# define GCI_I    GCI_I_16
# define GCI_Imax 16
# define GCI_Im   GCI_I_16m
# define GCI_IM   GCI_I_16M
#elif defined(GCI_I_8)
# define GCI_I    GCI_I_8
# define GCI_Imax 8
# define GCI_Im   GCI_I_8m
# define GCI_IM   GCI_I_8M
#else
# define GCI_I    GCI_I_4
# define GCI_Imax 4
# define GCI_Im   GCI_I_4m
# define GCI_IM   GCI_I_4M
#endif

#if   defined(GCI_U_16)
# define GCI_U    GCI_U_16
# define GCI_Umax 16
# define GCI_UM   GCI_U_16M
#elif defined(GCI_U_8)
# define GCI_U    GCI_U_8
# define GCI_Umax 8
# define GCI_UM   GCI_U_8M
#else
# define GCI_U    GCI_U_4
# define GCI_Umax 4
# define GCI_UM   GCI_U_4M
#endif

#if   defined(GCI_F_16)
# define GCI_F    GCI_F_16
# define GCI_Fmax 16
#elif defined(GCI_F_12)
# define GCI_F    GCI_F_12
# define GCI_Fmax 12
#elif defined(GCI_F_10)
# define GCI_F    GCI_F_10
# define GCI_Fmax 10
#elif defined(GCI_F_8)
# define GCI_F    GCI_F_8
# define GCI_Fmax 8
#elif defined(GCI_F_4)
# define GCI_F    GCI_F_4
# define GCI_Fmax 4
#else
# define GCI_Fmax 0
#endif

#if   ( GCI_Imax >= GCI_Umax ) && ( GCI_Imax >= GCI_Fmax )
# define GCI_max GCI_Imax
#elif ( GCI_Umax >= GCI_Imax ) && ( GCI_Umax >= GCI_Fmax )
# define GCI_max GCI_Umax
#else
# define GCI_max GCI_Fmax
#endif

/*
 * Create a table for conversion functions. We know the maximum entry
 * count now.
 */
static struct {
   GCI_result (*s2ifunc)( void *hidden, const char *str, int strsize,
                          void *target );
   GCI_result (*s2ufunc)( void *hidden, const char *str, int strsize,
                          void *target );
   GCI_result (*s2ffunc)( void *hidden, const char *str, int strsize,
                          void *target );
   GCI_result (*i2sfunc)( void *hidden, const void *data, char *str,
                          int *strsize );
   GCI_result (*u2sfunc)( void *hidden, const void *data, char *str,
                          int *strsize );
   GCI_result (*f2sfunc)( void *hidden, const void *data, char *str,
                          int *strsize );
} functable[GCI_max+1] = {
   {NULL, },
};
static int functable_assigned = 0;

#ifdef NEED_STRTOBIGL
static GCI_I strtobigl( void *hidden, const char *nptr, char **end, int base )
{
   int neg = 0;
   char *run = (char *) nptr;
   GCI_I retval = 0;

   (hidden = hidden);

   assert( base == 10 );

   while ( GCI_isspace( *run ) )
      run++;

   if ( *run == '-' )
   {
      neg = 1;
      run++;
   }
   if ( !GCI_isdigit( *run ) )
   {
      *end = (char *) nptr;
      return 0;
   }

   while ( GCI_isdigit( *run ) )
   {
      if ( neg )
      {
         if ( retval <= GCI_Im / 10 )
         {
            if ( retval == GCI_Im / 10 )
            {
               if ( (char ) -( GCI_Im - 10 * retval ) < ( *run - '0' ) )
               {
                  retval = GCI_Im;
                  errno = ERANGE;
               }
               else
               {
                  retval *= 10;
                  retval -= ( GCI_I ) ( *run - '0' );
               }
            }
            else
            {
               retval = GCI_Im;
               errno = ERANGE;
            }
         }
         else
         {
            retval *= 10;
            retval -= ( GCI_I ) ( *run - '0' );
         }
      }
      else
      {
         if ( retval >= GCI_IM / 10 )
         {
            if ( retval == GCI_IM / 10 )
            {
               if ( (char ) ( GCI_IM - 10 * retval ) < ( *run - '0' ) )
               {
                  retval = GCI_IM;
                  errno = ERANGE;
               }
               else
               {
                  retval *= 10;
                  retval += ( GCI_I ) ( *run - '0' );
               }
            }
            else
            {
               retval = GCI_IM;
               errno = ERANGE;
            }
         }
         else
         {
            retval *= 10;
            retval += ( GCI_I ) ( *run - '0' );
         }
      }
      run++;
   }

   *end = run;

   return retval;
}
#endif

#ifdef NEED_STRTOBIGUL
static GCI_U strtobigul( void *hidden, const char *nptr, char **end, int base )
{
   char *run = (char *) nptr;
   GCI_U retval = 0;

   (hidden = hidden);

   assert( base == 10 );

   while ( GCI_isspace( *run ) )
      run++;

   if ( !GCI_isdigit( *run ) )
   {
      *end = (char *) nptr;
      return 0;
   }

   while ( GCI_isdigit( *run ) )
   {
      if ( retval >= GCI_UM / 10 )
      {
         if ( retval == GCI_UM / 10 )
         {
            if ( (char ) ( GCI_UM - 10 * retval ) < ( *run - '0' ) )
            {
               retval = GCI_UM;
               errno = ERANGE;
            }
            else
            {
               retval *= 10;
               retval += ( GCI_U ) ( *run - '0' );
            }
         }
         else
         {
            retval = GCI_UM;
            errno = ERANGE;
         }
      }
      else
      {
         retval *= 10;
         retval += ( GCI_U ) ( *run - '0' );
      }
      run++;
   }

   *end = run;

   return retval;
}
#endif

#ifdef NEED_STRTOBIGF
static GCI_F strtobigf( const char *nptr, char **end )
{
   char c;
   GCI_F f;
   int rc;

   errno = 0;
   rc = sscanf( nptr, F_SCAN " %c", &f, &c );
   if ( rc <= 0 )
   {
      *end = (char *) nptr;
      if ( !errno )
         errno = ERANGE;
      return f; /* pro forma */
   }

   if ( rc == 2 )
   {
      if ( ( *end = strrchr( nptr, c ) ) == NULL )
         *end = (char *) nptr;
   }
   else
      *end = (char *) nptr + strlen( nptr );

   return f;
}
#endif

/*
 * preparenum chops off leading and trailing whitespaces from *str with the
 * initial length of *size. Leading zeros will be cut off, too.
 * *str and *size represent the final string after the stripping.
 * A resulting length of 0 may occur only if the complete string contain
 * whitespaces, only.
 */
static void preparenum( void *hidden,
                        const char **str,
                        int *size )
{
   const char *s = *str;
   int len = *size;

   (hidden = hidden);

   /*
    * We prepare the number by hand and strip away blanks and useless zeros.
    */
   while ( len && GCI_isspace( *s ) )
   {
      s++;
      len--;
   }
   if ( !len )
   {
      *size = 0;
      return;
   }

   /*
    * We know from the previous test that at least one non-space exists.
    */
   while ( GCI_isspace( s[len - 1] ) )
   {
      len--;
   }

   /*
    * At least one non-space exist. Strip leading zeros; if no character
    * remains, we have cut off one or more zeros. Undo one step then.
    */
   while ( len && ( *s == '0' ) )
   {
      s++;
      len--;
   }
   if ( !len )
   {
      s--;
      len = 1;
   }

   *str = s;
   *size = len;
}

/*
 * iswhole returns 1 exactly if all characters in str are decimal digits,
 * 0 otherwise.
 */
static int iswhole( void *hidden,
                    const char *str,
                    int size )
{
   (hidden = hidden);

   while ( size && GCI_isdigit( *str ) )
   {
      str++;
      size--;
   }
   return !size;
}

/*
 * string2int is the basic routine which implements GCI_Ir. size characters
 * of str (not strongly 0-terminated) are converted and the number is placed
 * in *retval.
 *
 * The string may contain garbage whitespace.
 *
 * The return code may be GCI_OK, GCI_WrongInput, GCI_NumberRange.
 */
static GCI_result string2int( void *hidden,
                              const char *str,
                              int size,
                              GCI_I *retval )
{
   char buf[80]; /* enough even for 256 bit numbers */
   char *p;
   int rc;

   preparenum( hidden, &str, &size );
   if ( !size || ( size > (int) sizeof( buf ) - 1 ) )
      return GCI_WrongInput;

   if ( ( *str == '-' ) || ( *str == '+' ) )
      rc = iswhole( hidden, str + 1, size - 1 );
   else
      rc = iswhole( hidden, str, size );
   if ( !rc )
      return GCI_WrongInput;

   memcpy( buf, str, size );
   buf[size] = '\0';

   /*
    * Carefully try to detect an overflow. We have to set errno for that
    * cases where MAX_??? is given in str.
    */
   errno = 0;
   *retval = GCI_Ir( buf, &p, 10 );
   if ( ( *p != '\0' ) || errno )
      return GCI_NumberRange;

   return GCI_OK;
}

/*
 * int2string is the basic routine which implements GCI_Iw. bin is converted
 * into a string which is placed into str. strsize must hold the buffer width
 * of str and must be greater than 79.
 *
 * The return code will be GCI_OK.
 *
 * *strsize is set to the resulting length without the terminator.
 */
static GCI_result int2string( void *hidden,
                              GCI_I bin,
                              char *str,
                              int *strsize )
{
   (hidden = hidden);
   assert( *strsize >= 80 );

   *strsize = GCI_Iw( str, bin );
   return GCI_OK;
}

/*
 * string2uint is the basic routine which implements GCI_Ur. size characters
 * of str (not strongly 0-terminated) are converted and the number is placed
 * in *retval.
 *
 * The string may contain garbage whitespace.
 *
 * The return code may be GCI_OK, GCI_WrongInput, GCI_NumberRange.
 */
static GCI_result string2uint( void *hidden,
                               const char *str,
                               int size,
                               GCI_U *retval )
{
   char buf[80]; /* enough even for 256 bit numbers */
   char *p;

   preparenum( hidden, &str, &size );
   if ( !size ||
        ( size > (int) sizeof( buf ) - 1 ) ||
        !iswhole( hidden, str, size ) )
      return GCI_WrongInput;

   memcpy( buf, str, size );
   buf[size] = '\0';

   /*
    * Carefully try to detect an overflow. We have to set errno for that
    * cases where MAX_??? is given in str.
    */
   errno = 0;
   *retval = GCI_Ur( buf, &p, 10 );
   if ( ( *p != '\0' ) || errno )
      return GCI_NumberRange;

   return GCI_OK;
}

/*
 * uint2string is the basic routine which implements GCI_Uw. bin is converted
 * into a string which is placed into str. strsize must hold the buffer width
 * of str and must be greater than 79.
 *
 * The return code will be GCI_OK.
 *
 * *strsize is set to the resulting length without the terminator.
 */
static GCI_result uint2string( void *hidden,
                               GCI_U bin,
                               char *str,
                               int *strsize )
{
   (hidden = hidden);
   assert( *strsize >= 80 );

   *strsize = GCI_Uw( str, bin );
   return GCI_OK;
}

#ifdef GCI_F
/*
 * string2float is the basic routine which implements GCI_Fr. size characters
 * of str (not strongly 0-terminated) are converted and the number is placed
 * in *retval.
 *
 * The string may contain garbage whitespace.
 *
 * The return code may be GCI_OK, GCI_NoMemory, GCI_WrongInput,
 * GCI_NumberRange.
 */
static GCI_result string2float( void *hidden,
                                const char *str,
                                int size,
                                GCI_F *retval )
{
   char *buf;
   char *p;

   (hidden = hidden);
   preparenum( hidden, &str, &size );
   if ( !size )
      return GCI_WrongInput;

   if ( ( buf = (char *) GCI_malloc( hidden, size + 1 ) ) == NULL )
      return GCI_NoMemory;

   memcpy( buf, str, size );
   buf[size] = '\0';

   /*
    * Carefully try to detect an overflow. We have to set errno for that
    * cases where MAX_??? is given in str.
    */
   errno = 0;
   *retval = GCI_Fr( buf, &p );
   while ( GCI_isspace( *p ) )
      p++;

   if ( *p != '\0' )
   {
      GCI_free( hidden, buf );
      return GCI_WrongInput;
   }

   if ( errno )
   {
      GCI_free( hidden, buf );
      return GCI_NumberRange;
   }

   GCI_free( hidden, buf );
   return GCI_OK;
}

/*
 * float2string is the basic routine which implements GCI_Uw. bin is converted
 * into a string which is placed into str. *strsize must hold the buffer width
 * of str and must be greater than 127.
 *
 * The return code will be GCI_OK or GCI_UnsupportedNumber if the generated
 * number cannot be represented by digits (e.g. NaN).
 *
 * *strsize is set to the resulting length without the terminator.
 */
static GCI_result float2string( void *hidden,
                                GCI_F bin,
                                char *str,
                                int *strsize )
{
   (hidden = hidden);
   assert( *strsize >= 128 );

   *strsize = GCI_Fw( str, bin );
   if ( ( *str == '-' ) || ( *str == '+' ) )
      str++;
   if ( !GCI_isdigit( *str ) )
      return GCI_UnsupportedNumber;
   return GCI_OK;
}
#endif /* ifdef GCI_F */

/*
 * We define a set of functions which bases on string2int and int2string.
 * Each function will either convert to a smaller sized number or display a
 * smaller sized number.
 *
 * usage: AUTOIFUNC(x) create the two functions s2ifuncx and i2sfuncx.
 */
#define AUTOIFUNC(size) static GCI_result s2ifunc##size( void *hidden,    \
                                                         const char *str, \
                                                         int strsize,     \
                                                         void *target )   \
{                                                                         \
   GCI_result rc;                                                         \
   GCI_I val;                                                             \
   if ( ( rc = string2int( hidden, str, strsize, &val ) ) != GCI_OK )     \
      return rc;                                                          \
                                                                          \
   if ( ( (GCI_I) val < GCI_I_##size##m ) ||                              \
        ( (GCI_I) val > GCI_I_##size##M ) )                               \
      return GCI_NumberRange;                                             \
                                                                          \
   *((GCI_I_##size *) target) = (GCI_I_##size) val;                       \
   return GCI_OK;                                                         \
}                                                                         \
static GCI_result i2sfunc##size( void *hidden,                            \
                                 const void *data,                        \
                                 char *str,                               \
                                 int *strsize)                            \
{                                                                         \
   GCI_I val;                                                             \
   val = (GCI_I) *( (GCI_I_##size *) data );                              \
   return int2string( hidden, val, str, strsize );                        \
}

#ifdef GCI_I_1
AUTOIFUNC(1)
#endif

#ifdef GCI_I_2
AUTOIFUNC(2)
#endif

#ifdef GCI_I_4
AUTOIFUNC(4)
#endif

#ifdef GCI_I_8
AUTOIFUNC(8)
#endif

#ifdef GCI_I_16
AUTOIFUNC(16)
#endif

/*
 * We define a set of functions which bases on string2uint and uint2string.
 * Each function will either convert to a smaller sized number or display a
 * smaller sized number.
 *
 * usage: AUTOUFUNC(x) create the two functions s2ufuncx and u2sfuncx.
 */
#define AUTOUFUNC(size) static GCI_result s2ufunc##size( void *hidden,    \
                                                         const char *str, \
                                                         int strsize,     \
                                                         void *target )   \
{                                                                         \
   GCI_result rc;                                                         \
   GCI_U val;                                                             \
   if ( ( rc = string2uint( hidden, str, strsize, &val ) ) != GCI_OK )    \
      return rc;                                                          \
                                                                          \
   if ( (GCI_U) val > GCI_U_##size##M )                                   \
      return GCI_NumberRange;                                             \
                                                                          \
   *((GCI_U_##size *) target) = (GCI_U_##size) val;                       \
   return GCI_OK;                                                         \
}                                                                         \
static GCI_result u2sfunc##size( void *hidden,                            \
                                 const void *data,                        \
                                 char *str,                               \
                                 int *strsize)                            \
{                                                                         \
   GCI_U val;                                                             \
   val = (GCI_U) *( (GCI_U_##size *) data );                              \
   return uint2string( hidden, val, str, strsize );                       \
}


#ifdef GCI_U_1
AUTOUFUNC(1)
#endif

#ifdef GCI_U_2
AUTOUFUNC(2)
#endif

#ifdef GCI_U_4
AUTOUFUNC(4)
#endif

#ifdef GCI_U_8
AUTOUFUNC(8)
#endif

#ifdef GCI_U_16
AUTOUFUNC(16)
#endif

#ifdef GCI_F
/*
 * We define a set of functions which bases on string2float and float2string.
 * Each function will either convert to a smaller sized number or display a
 * smaller sized number.
 *
 * usage: AUTOFFUNC(x) create the two functions s2ffuncx and f2sfuncx.
 */
#define AUTOFFUNC(size) static GCI_result s2ffunc##size( void *hidden,    \
                                                         const char *str, \
                                                         int strsize,     \
                                                         void *target )   \
{                                                                         \
   GCI_result rc;                                                         \
   GCI_F val;                                                             \
   if ( ( rc = string2float( hidden, str, strsize, &val ) ) != GCI_OK )   \
      return rc;                                                          \
                                                                          \
   if ( ( (GCI_F) val < GCI_F_##size##m ) ||                              \
        ( (GCI_F) val > GCI_F_##size##M ) )                               \
      return GCI_NumberRange;                                             \
                                                                          \
   *((GCI_F_##size *) target) = (GCI_F_##size) val;                       \
   return GCI_OK;                                                         \
}                                                                         \
static GCI_result f2sfunc##size( void *hidden,                            \
                                 const void *data,                        \
                                 char *str,                               \
                                 int *strsize)                            \
{                                                                         \
   GCI_F val;                                                             \
   val = (GCI_F) *( (GCI_F_##size *) data );                              \
   return float2string( hidden, val, str, strsize );                      \
}

#ifdef GCI_F_4
AUTOFFUNC(4)
#endif

#ifdef GCI_F_8
AUTOFFUNC(8)
#endif

#ifdef GCI_F_10
AUTOFFUNC(10)
#endif

#ifdef GCI_F_12
AUTOFFUNC(12)
#endif

#ifdef GCI_F_16
AUTOFFUNC(16)
#endif
#endif /* ifdef GCI_F */

/*
 * setup_functable initializes the functable table without setting illegal
 * values.
 * This allows simultaneous access without locking in multi-threading systems.
 */
static void setup_functable( void )
{
   int i;

   if ( functable_assigned )
      return;

   for ( i = 0; i <= GCI_max; i++)
   {
      switch ( i )
      {
         case 1:
#ifdef GCI_I_1
            functable[i].s2ifunc = s2ifunc1;
            functable[i].i2sfunc = i2sfunc1;
#else
            functable[i].s2ifunc = NULL;
            functable[i].i2sfunc = NULL;
#endif
#ifdef GCI_U_1
            functable[i].s2ufunc = s2ufunc1;
            functable[i].u2sfunc = u2sfunc1;
#else
            functable[i].s2ufunc = NULL;
            functable[i].u2sfunc = NULL;
#endif
            functable[i].s2ffunc = NULL;
            functable[i].f2sfunc = NULL;
            break;

         case 2:
#ifdef GCI_I_2
            functable[i].s2ifunc = s2ifunc2;
            functable[i].i2sfunc = i2sfunc2;
#else
            functable[i].s2ifunc = NULL;
            functable[i].i2sfunc = NULL;
#endif
#ifdef GCI_U_2
            functable[i].s2ufunc = s2ufunc2;
            functable[i].u2sfunc = u2sfunc2;
#else
            functable[i].s2ufunc = NULL;
            functable[i].u2sfunc = NULL;
#endif
            functable[i].s2ffunc = NULL;
            functable[i].f2sfunc = NULL;
            break;

         case 4:
#ifdef GCI_I_4
            functable[i].s2ifunc = s2ifunc4;
            functable[i].i2sfunc = i2sfunc4;
#else
            functable[i].s2ifunc = NULL;
            functable[i].i2sfunc = NULL;
#endif
#ifdef GCI_U_4
            functable[i].s2ufunc = s2ufunc4;
            functable[i].u2sfunc = u2sfunc4;
#else
            functable[i].s2ufunc = NULL;
            functable[i].u2sfunc = NULL;
#endif
#ifdef GCI_F_4
            functable[i].s2ffunc = s2ffunc4;
            functable[i].f2sfunc = f2sfunc4;
#else
            functable[i].s2ffunc = NULL;
            functable[i].f2sfunc = NULL;
#endif
            break;

         case 8:
#ifdef GCI_I_8
            functable[i].s2ifunc = s2ifunc8;
            functable[i].i2sfunc = i2sfunc8;
#else
            functable[i].s2ifunc = NULL;
            functable[i].i2sfunc = NULL;
#endif
#ifdef GCI_U_8
            functable[i].s2ufunc = s2ufunc8;
            functable[i].u2sfunc = u2sfunc8;
#else
            functable[i].s2ufunc = NULL;
            functable[i].u2sfunc = NULL;
#endif
#ifdef GCI_F_8
            functable[i].s2ffunc = s2ffunc8;
            functable[i].f2sfunc = f2sfunc8;
#else
            functable[i].s2ffunc = NULL;
            functable[i].f2sfunc = NULL;
#endif
            break;

         case 10:
            functable[i].s2ifunc = NULL;
            functable[i].i2sfunc = NULL;
            functable[i].s2ufunc = NULL;
            functable[i].u2sfunc = NULL;
#ifdef GCI_F_10
            functable[i].s2ffunc = s2ffunc10;
            functable[i].f2sfunc = f2sfunc10;
#else
            functable[i].s2ffunc = NULL;
            functable[i].f2sfunc = NULL;
#endif
            break;

         case 12:
            functable[i].s2ifunc = NULL;
            functable[i].i2sfunc = NULL;
            functable[i].s2ufunc = NULL;
            functable[i].u2sfunc = NULL;
#ifdef GCI_F_12
            functable[i].s2ffunc = s2ffunc12;
            functable[i].f2sfunc = f2sfunc12;
#else
            functable[i].s2ffunc = NULL;
            functable[i].f2sfunc = NULL;
#endif
            break;

         case 16:
#ifdef GCI_I_16
            functable[i].s2ifunc = s2ifunc16;
            functable[i].i2sfunc = i2sfunc16;
#else
            functable[i].s2ifunc = NULL;
            functable[i].i2sfunc = NULL;
#endif
#ifdef GCI_U_16
            functable[i].s2ufunc = s2ufunc16;
            functable[i].u2sfunc = u2sfunc16;
#else
            functable[i].s2ufunc = NULL;
            functable[i].u2sfunc = NULL;
#endif
#ifdef GCI_F_16
            functable[i].s2ffunc = s2ffunc16;
            functable[i].f2sfunc = f2sfunc16;
#else
            functable[i].s2ffunc = NULL;
            functable[i].f2sfunc = NULL;
#endif
            break;

         default:
            functable[i].s2ifunc = NULL;
            functable[i].i2sfunc = NULL;
            functable[i].s2ufunc = NULL;
            functable[i].u2sfunc = NULL;
            functable[i].s2ffunc = NULL;
            functable[i].f2sfunc = NULL;
            break;
      }
   }

   functable_assigned = 1;
}

/*****************************************************************************
 *****************************************************************************
 ** GLOBAL FUNCTIONS *********************************************************
 *****************************************************************************
 *****************************************************************************/

/*
 * GCI_string2bin converts size characters of the not strongly 0-terminated
 * string in str to a number.
 * The desired datatype is taken from type. It must be GCI_integer,
 * GCI_unsigned, GCI_float or GCI_char.
 * destbyte contains the number of bytes of the desired type.
 * Exactly destbyte bytes will be written to *dest on success. No checks for
 * alignment errors are done.
 *
 * The return code may be
 * GCI_OK              Everything is perfect.
 * GCI_NoMemory        Out of memory when converting a floating point number.
 * GCI_WrongInput      Strange characters occur in the input string.
 * GCI_NumberRange     Number to small or big to fit into the desired type
 *                     with the desired destbyte-size.
 * GCI_UnsupportedType The combination of destbyte/type is unknown or not
 *                     supported.
 */
GCI_result GCI_string2bin( void *hidden,
                           const char *str,
                           int size,
                           void *dest,
                           int destbyte,
                           GCI_basetype type )
{
   if ( !functable_assigned )
      setup_functable();

   if ( ( destbyte < 0 ) || ( destbyte >= (int) elements( functable ) ) )
      return GCI_UnsupportedType;

   switch ( type )
   {
      case GCI_integer:
         if ( functable[destbyte].s2ifunc == NULL )
            return GCI_UnsupportedType;
         return functable[destbyte].s2ifunc( hidden, str, size, dest );

      case GCI_unsigned:
         if ( functable[destbyte].s2ufunc == NULL )
            return GCI_UnsupportedType;
         return functable[destbyte].s2ufunc( hidden, str, size, dest );

      case GCI_float:
         if ( functable[destbyte].s2ffunc == NULL )
            return GCI_UnsupportedType;
         return functable[destbyte].s2ffunc( hidden, str, size, dest );

      case GCI_char:
         if ( destbyte != 1 )
            return GCI_UnsupportedType;
         if ( size != 1 )
            return GCI_WrongInput;
         *((char *) dest) = *str;
         return GCI_OK;

      default:
         break;
   }
   return GCI_UnsupportedType;
}

/*
 * GCI_bin2string converts size byte of base into a 0-terminated string with
 * a maximum target size of *strsize including the terminator.
 * The desired datatype is taken from type. It must be GCI_integer,
 * GCI_unsigned, GCI_float or GCI_char. GCI_string is not handled.
 * size contains the number of bytes of the desired type.
 * Exactly size bytes will be taken from *base. No checks for alignment errors
 * are done.
 * *strsize is set to the new size of the resulting string without the
 * terminator.
 *
 * The return code may be
 * GCI_OK                Everything is perfect.
 * GCI_UnsupportedNumber The floating point value in base is not a number
 *                       which can be displayed by digits (e.g. NaN).
 * GCI_BufferTooSmall    The target buffer size strsize is less than 128.
 * GCI_UnsupportedType   The combination of size/type is unknown or not
 *                       supported.
 */
GCI_result GCI_bin2string( void *hidden,
                           const void *base,
                           int size,
                           char *str,
                           int *strsize,
                           GCI_basetype type )
{
   if ( !functable_assigned )
      setup_functable();

   if ( ( size < 0 ) || ( size >= (int) elements( functable ) ) )
      return GCI_UnsupportedType;

   if ( *strsize < 128 )
      return GCI_BufferTooSmall;

   switch ( type )
   {
      case GCI_integer:
         if ( functable[size].i2sfunc == NULL )
            return GCI_UnsupportedType;
         return functable[size].i2sfunc( hidden, base, str, strsize );

      case GCI_unsigned:
         if ( functable[size].u2sfunc == NULL )
            return GCI_UnsupportedType;
         return functable[size].u2sfunc( hidden, base, str, strsize );

      case GCI_float:
         if ( functable[size].f2sfunc == NULL )
            return GCI_UnsupportedType;
         return functable[size].f2sfunc( hidden, base, str, strsize );

      case GCI_char:
         if ( size == 0 )
            return GCI_BufferTooSmall;
         if ( size > 1 )
            return GCI_UnsupportedType;
         str[0] = *((char *) base);
         str[1] = '\0';
         *strsize = 1;
         return GCI_OK;

      default:
         break;
   }
   return GCI_UnsupportedType;
}

/*
 * GCI_validate tests whether a specific type is supported by both conversion
 * routines. If basetype is set, an additional test is done for a base type.
 *
 * The return code is
 * GCI_OK                Everything is perfect.
 * GCI_UnsupportedType   The combination of size/type is unknown or not
 *                       supported.
 * GCI_NoBaseType        The type won't fit the requirements for basic types.
 */
GCI_result GCI_validate( int size,
                         GCI_basetype type,
                         int basetype )
{
   if ( !functable_assigned )
      setup_functable();

   if ( size < 0 )
      return GCI_UnsupportedType;

   if ( ( type == GCI_string ) ||
        ( type == GCI_raw ) ||
        ( type == GCI_container ) ||
        ( type == GCI_array ) )
      return ( basetype ) ? GCI_NoBaseType : GCI_OK;

   if ( size >= (int) elements( functable ) )
      return GCI_UnsupportedType;

   switch ( type )
   {
      case GCI_integer:
         if ( ( functable[size].i2sfunc == NULL ) ||
              ( functable[size].s2ifunc == NULL ) )
            return GCI_UnsupportedType;
         return GCI_OK;

      case GCI_unsigned:
         if ( ( functable[size].u2sfunc == NULL ) ||
              ( functable[size].s2ufunc == NULL ) )
            return GCI_UnsupportedType;
         return GCI_OK;

      case GCI_float:
         if ( ( functable[size].f2sfunc == NULL ) ||
              ( functable[size].s2ffunc == NULL ) )
            return GCI_UnsupportedType;
         return GCI_OK;

      case GCI_char:
         if ( size != 1 ) /* we don't support unicode or other MBCS */
            return GCI_UnsupportedType;
         return GCI_OK;

      default:
         break;
   }
   return GCI_UnsupportedType;
}
