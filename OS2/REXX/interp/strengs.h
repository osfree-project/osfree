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
#ifndef _STRINGS_ALREADY_DEFINED_
#define _STRINGS_ALREADY_DEFINED_

typedef struct strengtype {
   int len, max ;
#ifdef CHECK_MEMORY                     /* FGC: Test                         */
   char *value;
#else
   char value[4] ;
#endif
} streng ;

/*
 * Some strange define's to allow constant strengs to be defined. They will
 * be accessible as pointers.
 */
#ifdef CHECK_MEMORY
#  define conststreng(name,value) const streng __regina__##name = { sizeof( value ) - 1, \
                                                        sizeof( value ) - 1,             \
                                                        value };                         \
                                  const streng *name = (const streng *) &__regina__##name
#  define staticstreng(name,value) static streng x_##name = { sizeof( value ) - 1,       \
                                                                sizeof( value ) - 1,     \
                                                                value };                 \
                                   const static streng *name = (const streng *) &x_##name
#else
#  define conststreng(name,value) const struct {                               \
                                     int len, max;                             \
                                     char content[sizeof( value )];            \
                                  } x__regina__##name = { sizeof( value ) - 1, \
                                                sizeof( value ) - 1,           \
                                                value };                       \
                                  const streng *name = (streng *) &x__regina__##name
#  define staticstreng(name,value) static struct {                     \
                                      int len, max;                    \
                                      char content[sizeof( value )];   \
                                   } x_##name = { sizeof( value ) - 1, \
                                                 sizeof( value ) - 1,  \
                                                 value };              \
                                   static const streng *name = (const streng *) &x_##name
#endif
#define STRENG_TYPEDEFED 1

#define Str_len(a) ((a)->len)
#define Str_max(a) ((a)->max)
#define Str_val(a) ((a)->value)
#define Str_in(a,b) (Str_len(a)>(b))
#define Str_end(a) ((a)->value+Str_len(a))
#define Str_zero(a) ((Str_len(a)<Str_max(a)) && ((a)->value[(a)->len]==0x00))


#define STRHEAD (1+(sizeof(int)<<1))


typedef struct num_descr_type
{
   char *num ;      /* pointer to matissa of precision + 1 */
   int negative ;   /* boolean, true if negative number */
   int exp ;        /* value of exponent */
   int size ;       /* how much of num is actually used */
   int max ;        /* how much can num actually take */

   /*
    * The number has an absolute value of
    *   *********************
    *   * "0."<num>"E"<exp> *
    *   *********************
    * Only size byte of num are used.
    *
    * The number of used digits depends on its usage. In general, it's a good
    * idea to use the standard value. used_digits shall be reset after each
    * computation and may or may not be respected. It shall be respected, and
    * this is the intention, by str_norm and all other function which make
    * a string from the number accidently. Functions like string_add may
    * or may not use this value.
    * fixes bug 675395
    */
   int used_digits;
} num_descr ;


#endif /* _STRINGS_ALREADY_INCLUDED_ */
