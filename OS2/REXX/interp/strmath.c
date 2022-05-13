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
#include <limits.h>
#include <assert.h>
#include <string.h>
#include "regina64.h"


#define log_xor(a,b)    (( (a)&&(!(b)) ) || ( (!(a)) && (b) ))
#if !defined(MAX)
# define MAX(a,b) (((a)>(b))?(a):(b))
#endif
#if !defined(MIN)
# define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#define IS_AT_LEAST(ptr,now,min) \
   if (now<min) { if (ptr) FreeTSD(ptr); ptr=(char *)MallocTSD(now=min) ; } ;


#define stringize(x) #x
#define stringize_value(x) stringize(x)

typedef struct { /* mat_tsd: static variables of this module (thread-safe) */
#ifdef TRACEMEM
   void *    outptr1;
   void *    outptr2;
   void *    outptr3;
   void *    outptr4;
   void *    outptr5;
#endif

   num_descr edescr;
   num_descr fdescr;
   num_descr rdescr;
   num_descr sdescr;


   int       add_outsize;   /* This values MAY all become one. CHECK THIS! */
   char *    add_out;
   int       norm_outsize;
   char *    norm_out;
   int       div_outsize;
   char *    div_out;
   int       mul_outsize;
   char *    mul_out;
   int       max_exponent_len;
} mat_tsd_t; /* thread-specific but only needed by this module. see
              * init_math
              */

/* init_math initializes the module.
 * Currently, we set up the thread specific data and check for environment
 * variables to change debugging behaviour.
 * The function returns 1 on success, 0 if memory is short.
 */
int init_math( tsd_t *TSD )
{
   mat_tsd_t *mt;

   if (TSD->mat_tsd != NULL)
      return(1);

   if ( ( TSD->mat_tsd = MallocTSD( sizeof(mat_tsd_t) ) ) == NULL )
      return(0);
   mt = (mat_tsd_t *)TSD->mat_tsd;
   memset( mt, 0, sizeof(mat_tsd_t) );

   mt->max_exponent_len = strlen(stringize_value(MAX_EXPONENT));
   return(1);
}

#ifdef TRACEMEM
void mark_descrs( const tsd_t *TSD )
{
   mat_tsd_t *mt;

   mt = (mat_tsd_t *)TSD->mat_tsd;
   if (mt->rdescr.num) markmemory( mt->rdescr.num, TRC_MATH ) ;
   if (mt->sdescr.num) markmemory( mt->sdescr.num, TRC_MATH ) ;
   if (mt->fdescr.num) markmemory( mt->fdescr.num, TRC_MATH ) ;
   if (mt->edescr.num) markmemory( mt->edescr.num, TRC_MATH ) ;

   if (mt->outptr1) markmemory( mt->outptr1, TRC_MATH ) ;
   if (mt->outptr2) markmemory( mt->outptr2, TRC_MATH ) ;
   if (mt->outptr3) markmemory( mt->outptr3, TRC_MATH ) ;
   if (mt->outptr4) markmemory( mt->outptr4, TRC_MATH ) ;
   if (mt->outptr5) markmemory( mt->outptr5, TRC_MATH ) ;
}
#endif /* TRACEMEM */

static streng *name_of_node( const tsd_t *TSD, cnodeptr node,
                             const num_descr *val )
{
   streng *retval;
   cnodeptr run;
   num_descr num;
   int len;

   if (node)
   {
      switch ( node->type )
      {
         case X_STRING:
         case X_CON_SYMBOL:
         case X_SIM_SYMBOL:
         case X_STEM_SYMBOL:
            return Str_dupTSD( node->name );

         case X_HEAD_SYMBOL:
            /*
             * Build the complete name of the variable.
             */
            len = Str_len( node->name );
            for ( run = node->p[0]; run; run = run->p[0] )
            {
               len += Str_len( run->name ) + 1;
            }
            retval = Str_makeTSD( len );
            Str_catTSD( retval, node->name );
            for ( run = node->p[0]; run; run = run->p[0] )
            {
               Str_catTSD( retval, run->name );
               if ( run->p[0] )
                  retval->value[retval->len++] = '.';
            }
            return retval;
      }
   }

   /*
    * reformat the number with all possible digits to show the user the
    * true value..
    */
   num = *val;
   num.used_digits = ( num.size ) ? num.size : 1;
   retval = str_norm( TSD, &num, NULL );

   return retval;
}

#define LOSTDIGITS_CHECK(val,maxdigits,node) {              \
   const char *_ptr = (const char *) ((val)->num);          \
   int _size = (val)->size;                                 \
   int _digits = maxdigits;                                 \
   while (_size && *_ptr == '0')                            \
   {                                                        \
      _ptr++;                                               \
      _size--;                                              \
   }                                                        \
   if (_size > _digits)                                     \
   {                                                        \
      _size -= _digits;                                     \
      _ptr += _digits;                                      \
      while (_size)                                         \
      {                                                     \
         if (*_ptr != '0')                                  \
         {                                                  \
            condition_hook( TSD,                            \
                            SIGNAL_LOSTDIGITS,              \
                            0,                              \
                            0,                              \
                            -1,                             \
                            name_of_node( TSD, node, val ), \
                            NULL );                         \
            break;                                          \
         }                                                  \
         _ptr++;                                            \
         _size--;                                           \
      }                                                     \
   }                                                        \
}

/*
 * ANSI chapter 7, beginning: "...matches that syntax and also has a value
 * that is 'whole', that is has no non-zero fractional part." The syntax
 * is that of a plain number.
 * Thus, 1E1 or 1.00 are allowed.
 * returns 0 on error, 1 on success. *value is set to the value on success.
 */
static int whole_number( const num_descr *input, int *value )
{
   /* number must be integer, and must be small enough */
   int result,i,digit;

   if ( input->size > input->exp )
   {
      /*
       * Check for non-zeros in the fractional part of the number.
       */
      i = MAX( 0, input->exp );
      for ( ; i < input->size; i++ )
      {
         if ( input->num[i] != '0' )
            return 0;
      }
   }

   /*
    * The number is valid but may be too large. Keep care.
    */
   for ( i = 0, result = 0; i < input->exp; i++ )
   {
      if ( result > INT_MAX / 10 )
         return 0;
      result *= 10;
      if ( i < input->size )
      {
         digit = input->num[i] - '0';
         if ( result > INT_MAX - digit )
            return 0;
         result += digit;
      }
   }
   if (input->negative)
      result = -result;

   *value = result;
   return 1;
}

/*
 * ANSI chapter 7, beginning: "...matches that syntax and also has a value
 * that is 'whole', that is has no non-zero fractional part." The syntax
 * is that of a plain number.
 * Thus, 1E1 or 1.00 are allowed.
 * returns 0 on error, 1 on success. *value is set to the value on success.
 */
static int whole_rx64_number( const num_descr *input, rx_64 *value )
{
   /* number must be integer, and must be small enough */
   rx_64 result;
   int i,digit;

   if ( input->size > input->exp )
   {
      /*
       * Check for non-zeros in the fractional part of the number.
       */
      i = MAX( 0, input->exp );
      for ( ; i < input->size; i++ )
      {
         if ( input->num[i] != '0' )
            return 0;
      }
   }

   /*
    * The number is valid but may be too large. Keep care.
    */
   for ( i = 0, result = 0; i < input->exp; i++ )
   {
      if ( result > RX_64MAX / 10 )
         return 0;
      result *= 10;
      if ( i < input->size )
      {
         digit = input->num[i] - '0';
         if ( result > RX_64MAX - digit )
            return 0;
         result += digit;
      }
   }
   if (input->negative)
      result = -result;

   *value = result;
   return 1;
}

int descr_to_int( const tsd_t *TSD, const num_descr *input, int errnum, int suberrnum, const char *bif, int argno )
{
   int result = 0;

   if ( !whole_number( input, &result ) )
   {
      volatile char *fs;
      streng *h;

      h = name_of_node( TSD, NULL, input );
      fs = tmpstr_of( (tsd_t *)TSD, h );
      Free_stringTSD( h );
      switch( errnum )
      {
         case ERR_INCORRECT_CALL:
            exiterror( errnum, suberrnum, bif, argno, fs );
            break;
         case ERR_INVALID_INTEGER:
            exiterror( errnum, suberrnum, fs );
            break;
      }
      /* should NOT get here */
      exiterror( ERR_INVALID_INTEGER, 0 );
   }

   return result;
}

/*
 * strip leading zeros and translate 0e? into a plain 0.
 */
void str_strip( num_descr *num )
{
   int i=0, j=0;

   if (num->size==1)
   {
      if (num->num[0] == '0')
      {
         num->negative = 0;
         num->exp = 1;
      }
      return;
   }

   for ( i = 0; ( i < num->size - 1 ) && ( num->num[i] == '0'); i++ )
      /* Keep at least one character */;
   if ( i )
   {
      for ( j = 0; j < num->size - i; j++ )
      {
         num->num[j] = num->num[j + i];
      }

      num->exp -= i;
      num->size -= i;
      assert( num->size > 0 );
   }

   if ( ( num->size == 1 ) && ( num->num[0] == '0' ) )
   {
      num->negative = 0;
      num->exp = 1;
   }
}


int getdescr( const tsd_t *TSD, const streng *num, num_descr *descr )
/* converts num into a descr and returns 0 if successfully.
 * returns 9 or 11 in case of an error. descr contains nonsense in this case.
 * 9 is returned if the exponent is too big, 11 if num is no number.
 * The newly generated descr is as short as possible: leading and
 * trailing zeros (after a period) will be cut, rounding occurs.
 * We don't use registers and hope the compiler does it better than outselves
 * in the optimization stage, else try in this order: c, inlen, in, out, exp.
 */
{
   const char *in;      /* num->value */
   int   inlen;         /* chars left in "in" */
   char *out;           /* descr->num */
   int   outpos;        /* position where to write */
   int   outmax;        /* descr->max */
   char  c,             /* tmp var */
         lastdigit = 0; /* last digit seen for mantissa, init: error */
   int   pointseen,     /* point in mantissa seen? */
         exp,           /* exp from mantissa */
         exp2,          /* exp from "1E1" */
         expsign;       /* sign of the exp in "1E-1", 1 or -1 */

   /*
    * The maximum size of the mantissa is the worst case of a plain number,
    * e.g. 123456789
    */
   outmax = Str_len(num);

   IS_AT_LEAST( descr->num, descr->max, outmax );

   /*
    * A new number shall always be printed with the current DIGITS value.
    */
   descr->used_digits = TSD->currlevel->currnumsize;

   in = num->value;
   inlen = Str_len(num);
   /* skip leading spaces */
   while (inlen && rx_isspace(*in))
   {
      in++;
      inlen--;
   }

   if (!inlen)
      return 11 ;

   c = *in;

   /* check sign */
   if ((c == '-') || (c == '+'))
   {
      descr->negative = (c == '-') ;
      in++; /* c eaten */
      inlen--;
      while (inlen && rx_isspace(*in)) /* skip leading spaces */
      {
         in++;
         inlen--;
      }

      if (!inlen)
         return 11 ;
   }
   else
      descr->negative = 0 ;

   /* cut ending blanks first, a non blank exists (in[0]) at this point */
   while (rx_isspace(in[inlen-1]))
      inlen--;

   while (inlen && (*in == '0')) /* skip leading zeros */
   {
      in++;
      inlen--;
      lastdigit = '0';
   }
   if (!inlen)
   {  /* Fast breakout in case of a plain "0" or an error */

      descr->num[0] = lastdigit;
      descr->exp = 1;
      descr->size = 1;
      if (lastdigit == '0')
      {
         descr->negative = 0;
         return 0 ;
      }
      return 11 ;
   }

   /* Transfer digits and check for points */
   pointseen = 0; /* never seen */
   exp = 0;
   out = descr->num;
   outpos = 0;
   while (inlen)
   {
      if ((c = *in) == '.')
      {
         if (pointseen)
            return 11 ;
         pointseen = 1;
         in++;
         inlen--;
         continue;
      }
      if (!rx_isdigit(c))
         break;
      if (outpos < outmax)
      {
         lastdigit = c;
         if ((c=='0') && (outpos==0)) /* skip zeros in "0.0001" */
            exp--;       /* We must be after a point, see zero parsing above */
         else
         {
            out[outpos++] = c;
            if (!pointseen)
               exp++;
         }
      }
      else
      {
         lastdigit = '0';
         if (!pointseen)
            exp++;
      }
      in++;
      inlen--;
   }
   /* the mantissa is correct now, check for ugly "0.0000" later */
   if (inlen)
   {
      /* c is *in at this point, see above */
      expsign = 1;
      if ((c != 'e') && (c != 'E'))
         return 11 ;
      if (--inlen == 0) /* at least one digit must follow */
         return 11 ;
      in++;

      c = *in;
      if ((c == '+') || (c == '-'))
      {
         if (c == '-')
            expsign = -1;
         if (--inlen == 0) /* at least one digit must follow */
            return 11 ;
         in++;
      }
      exp2 = 0;
      while (inlen--)
      {
         c = *in++;
         if (!rx_isdigit(c))
            return 11 ;
         /* a rough test first, assume a mantissa with length < MAX_EXPONENT */
         if ( exp2 > MAX_EXPONENT / 10 )
            return 9;
         exp2 = exp2*10 + (c - '0');
         if ( expsign * (exp + expsign * exp2) - 1 > MAX_EXPONENT )
            return 9;
      }
      if (expsign < 0)
         exp -= exp2;
        else
         exp += exp2;
   }
   if (outpos == 0) /* no digit or 0.000 with or without exp */
   {
      if (!lastdigit)
         return 11 ;
      out[outpos++] = '0';
      exp = 1;
      descr->negative = 0;
   }
   descr->exp = exp;
   descr->size = outpos;
   assert(descr->size <= outmax);
   return(0);
}


/*
 * Rounds descr to size digits. If stop_on_cut is set, a LOSTDIGITS condition
 * is fired if anything other than zeros are truncated.
 */
static void descr_round( num_descr *descr, int size, tsd_t *stop_on_cut )
{
   int i;

   /*
    * We don't touch descr->used_digits here. If the caller really needs it,
    * it must be done at that level. Rounding itself isn't an operation
    * creating a number in the terms of Rexx in opposite to TRUNC() or the
    * normal mathematical operations.
    */

   /*
    * Can't do illegal operations.
    */
   assert( size > 0 );

   /*
    * Increment size by the number of leading zeros existing.
    */
   for ( i = 0; i < descr->size; i++ )
   {
      if ( descr->num[i] == '0' )
         size++;
      else
         break;
   }
   size += i;

   /*
    * Do we have to round?
    */
   if ( descr->size <= size )
      return;

   if ( stop_on_cut )
   {
      for ( i = size; i < descr->size; i++ )
      {
         if ( descr->num[i] != '0' )
         {
            condition_hook( stop_on_cut,
                            SIGNAL_LOSTDIGITS,
                            0,
                            0,
                            -1,
                            name_of_node( stop_on_cut, NULL, descr ),
                            NULL );
            break;
         }
      }
      if ( i >= descr->size )
      {
         descr->size = size;
         return;
      }
   }

   descr->size = size;
   /*
    * Is it possibly just a truncation?
    */
   if ( descr->num[size] < '5' )
   {
      return;
   }

   /*
    * increment next digit, and loop if that was a '9'
    */
   for ( i = size - 1; ; )
   {
      if ( descr->num[i] != '9' )
      {
         descr->num[i]++;
         break;
      }

      descr->num[i--] = '0';

      if ( i == -1 )
      {
         /*
          * "Carry", we have to increment the exponent. The complete mantissa
          * consists of zeros. We have to set it to "1000...".
          */
#ifndef NDEBUG
         /*
          * Just check a few things ... I don't like surprises
          */
         for ( i = 0; i < size; i++ )
            assert( descr->num[i] == '0' );
#endif
         descr->exp++;
         descr->num[0] = '1';
         break;
      }
   }
   return;
}


void str_round( num_descr *descr, int size )
{
   descr_round( descr, size, NULL );
}


void str_round_lostdigits( tsd_t *TSD, num_descr *descr, int size )
{
   descr_round( descr, size, TSD );
}


void descr_copy( const tsd_t *TSD, const num_descr *f, num_descr *s )
{
   /*
    * Check for the special case that these are identical, then we don't
    * have to do any copying, so just return.
    */
   if (f==s)
      return ;

   s->negative = f->negative ;
   s->exp = f->exp ;
   s->size = f->size ;
   s->used_digits = f->used_digits;

   IS_AT_LEAST( s->num, s->max, f->size ) ;
   memcpy( s->num, f->num, f->size ) ;
}

/*
 * string_add2 computes
 *
 * r=f+s
 *
 * with the current digits() setting of ccns (e.g. TSD->currlevel->currnumsize)
 * Keep in mind that f or s may be identical to r.
 *
 * Function rewritten completely on 03.07.2005 by FGC. The former one was
 * incompatible with the standard. This approach follows the ANSI standard's
 * code example.
 */
static void string_add2( tsd_t *TSD, const num_descr *f, const num_descr *s,
                         num_descr *r, int ccns )
{
   mat_tsd_t *mt;       /* mt->add_out is used      */
   int neg;             /* negate the result if set, and also: f WAS negative*/
   int sneg_factor;     /* -1: s is negative, 1: s is positive */
   int carry;           /* carry flag */
   int loan;            /* loan flag */
   const char *fnum;    /* mantissa start of f */
   const char *snum;    /* mantissa start of s */
   char *fpoint;        /* least significant mantissa position of f */
   const char *spoint;  /* least significant mantissa position of s */
                        /* There is no more digits available if point < num */
   int h, h2, h3;       /* helper */
   const num_descr *swp;/* helper */
   int c;               /* helper */
   static const char none[2] ="";  /* just to keep pointers valid */

   /*
    * In opposite to ANSI we don't have to consider NUMERIC FUZZ. The
    * comparisons are done using string_test.
    */

   /*
    * ANSI results f if s==0 (strict comparison!) and s if f==0 with respect
    * to the operator which is "+" in string_add2.
    */
   if ((s->size == 1) && (s->num[0] == '0'))
   {
      descr_copy( TSD, f, r );
      return;
   }
   if ((f->size == 1) && (f->num[0] == '0'))
   {
      descr_copy( TSD, s, r );
      return;
   }

   /*
    * The other shortcut isn't mentioned in ANSI where the exponents differ
    * so significantly that one operand isn't used at all. So just continue
    * to try computing 1+1e1000.
    */

   mt = (mat_tsd_t *)TSD->mat_tsd;
   /*
    * We use a temporary buffer for the result. We don't know the magnitude of
    * the result in advance (99.99 may be rounded to 100), so we need 2 more
    * digits: one for the digits after ccns which is calculated to determine
    * a first rounding and one for the overflow.
    * Thus, the most significant number which is a template of the result
    * has a virtual mantissa position of 1 because position 0 is reserved for
    * the overflow.
    */
   IS_AT_LEAST( mt->add_out, mt->add_outsize, ccns+2 );
#ifdef TRACEMEM
   mt->outptr5 = mt->add_out;
#endif

   if (f->exp < s->exp)
   {
      /*
       * The number with the most significant exponent needs to be the
       * first one to make things simpler.
       */
      swp = f;
      f = s;
      s = swp;
   }

   /*
    * It is much easier to reduce the various situations by making the first
    * number positive:
    * -x op y     is equivalent to       -(x op -y)
    */
   if (f->negative)
   {
      /* always use a positive number as the left operator */
      neg = 1;
      sneg_factor = s->negative ? 1 : -1;
   }
   else
   {
      neg = 0;
      sneg_factor = s->negative ? -1 : 1;
   }

   fnum = f->num;
   snum = s->num;
   /*
    * The most significant number is the base of the result. The other number
    * is aligned to the most significant. Example where the second is the
    * most significant number:
    *
    *       fffff.ff
    *          ss.ssssss
    */

   h = f->exp - s->exp;
   r->exp = f->exp;     /* save now even if s==r */
   /*
    * s may need an adjustement.
    */
   h2 = MIN( ccns + 1, f->size );     /* h2 = used mantissa length of f */
   h3 = MIN( ccns + 1 - h, s->size ); /* h3 = used mantissa length of s */
   if ( h3 < 0 )
   {
      /*
       * spoint = snum + h3 - 1 is NOT allowed. h3 may be so small that
       * spoint becomes really invalid and may cause a segment violation.
       */
      spoint = none;
      snum = spoint + 1;
   }
   else
   {
      spoint = snum + h3 - 1;
   }
   r->size = MAX( h2, h + h3 );

   /*
    * We conpute "r = f; r += s;" This is much easier to handle than everything
    * else.
    */

   mt->add_out[0] = '0';
   memcpy( mt->add_out + 1, fnum, h2 );
   memset( mt->add_out + h2 + 1, '0', r->size - h2 );

   /*
    * r += s;
    *
    * Get the fpoint to that position that needs to be added by spoint.
    * This is a valid position because we filled up with 0 already.
    */
   fnum = mt->add_out;              /* NOT +1, keep it on the starting 0 */
   fpoint = (char *) fnum + h + h3; /* NOT -1, because fnum is decremented */
   carry = loan = 0;

   while ( spoint >= snum )
   {
      c = *fpoint + sneg_factor * (*spoint - '0') + carry - loan;
      if ((loan = (c < '0')) != 0)
      {
         c += 10;
      }
      if ((carry = (c > '9')) != 0)
      {
         c -= 10;
      }
      spoint--;
      *fpoint-- = (char)c;
   }
   while ( fpoint > fnum )
   {
      c = *fpoint + carry - loan;
      if ((loan = (c < '0')) != 0)
      {
         c += 10;
      }
      if ((carry = (c > '9')) != 0)
      {
         c -= 10;
      }
      *fpoint-- = (char)c;
   }
   if ( !loan )
   {
      if ( carry )
      {
         *fpoint = '1';
         r->exp++;
         r->size++;
      }
      else
      {
         fpoint++;
      }
   }
   else
   {
      fpoint++;
      /*
       * Having a loan means we have a negative result. Reverse the result with
       * r = -r;
       */
      neg = !neg;
      for ( h = r->size - 1, carry = 10; h >= 0; h-- )
      {
         if ( ( fpoint[h] = (char) (carry - (fpoint[h] - '0') + '0') ) > '9' )
         {
            fpoint[h] = '0';
            carry = 10;
         }
         else
           carry = 9;
      }
   }

   /*
    * Added so far. Now accomplish the rounding following ANSI rules.
    *
    * We can increase the result's accurracy by jumping over one leading
    * zero if available first, but this breaks ANSI.
    */

   if ( r->size > ccns )
   {
      r->size = ccns;
      if ( fpoint[ccns] > '4' )
      {
         /*
          * Increment mantissa regardless of the true sign.
          */
         fnum = fpoint;
         fpoint += ccns - 1;
         carry = 1;
         while ( fpoint >= fnum )
         {
            if ( ++(*fpoint) <= '9' )
            {
               carry = 0;
               fpoint = (char *) fnum - 1;
               break;
            }
            *fpoint = '0';
            fpoint--;
         }

         if ( carry )
         {
            r->exp++;
            *fpoint = '1';
         }
         else
         {
            fpoint++;
         }
      }
   }
   IS_AT_LEAST( r->num, r->max, r->size ) ;
   memcpy( r->num, fpoint, r->size );
   r->negative = neg;
   str_strip( r ) ;
}

void string_add( tsd_t *TSD, const num_descr *f, const num_descr *s,
                 num_descr *r, cnodeptr left, cnodeptr right )
{
   int ccns = TSD->currlevel->currnumsize ;

   LOSTDIGITS_CHECK( f, ccns, left );
   LOSTDIGITS_CHECK( s, ccns, right );

   string_add2( TSD, f, s, r, ccns );

   r->used_digits = ccns;
}

streng *str_format(tsd_t *TSD, const streng *input, int Before,
                                                 int After, int Expp, int Expt)
/* According to ANSI X3J18-199X, 9.4.2, this function performs the BIF "format"
 * with extensions made by Brian.
 * I rewrote the complete function to allow comparing of this function code
 * to that one made in Rexx originally.
 * input is the first arg to "format" and may not be a number.
 * Before, After, Expp and Expt are the other args to this function and are
 * -1 if they are missing value.
 * FGC
 */
{
#define Enlarge(Num,atleast) if (Num.size + (atleast) > Num.max) {            \
                                char *newnum = (char *)MallocTSD(Num.size + (atleast) + 5); \
                                Num.max = Num.size + (atleast) + 5;           \
                                memcpy( newnum, Num.num, Num.size );                 \
                                FreeTSD(Num.num);                             \
                                Num.num = newnum;                                \
                             }
   char *buf;
   size_t bufsize;
   size_t bufpos;
   int ShowExp,Exponent,ExponentLen = 0,Afters,Sign,Point,OrigExpp,h;
   streng *retval;
   char Expart[80]; /* enough even on a 256-bit-machine for an int */
   mat_tsd_t *mt;
   int StrictAnsi;

   mt = (mat_tsd_t *)TSD->mat_tsd;

   /*
    * Convert the input to a number and check if it is a number at all.
    */
   if ( ( h = getdescr( TSD, input, &mt->fdescr ) ) != 0 )
   {
      if ( h == 9 )
         exiterror( ERR_INCORRECT_CALL, h, "FORMAT", 1, mt->max_exponent_len, tmpstr_of( TSD, input ) );
      else
         exiterror( ERR_INCORRECT_CALL, h, "FORMAT", 1, tmpstr_of( TSD, input ) );
   }

   StrictAnsi = get_options_flag( TSD->currlevel, EXT_STRICT_ANSI );
   /*
    * Round the number according to NUMERIC DIGITS. This is rule 9.2.1.
    * It is mentioned at several places in 9.4.1 (FORMAT).
    * FGC: This is bullshit if you want to have format() formatting numbers
    *      with a higher precision than DIGITS. I've put it into STRICT mode.
    *      Regina's normal mode allows any numbers to be formatted. The
    *      default formatting rounds to DIGITS, though.
    */
   if (StrictAnsi)
   {
      str_round_lostdigits( TSD, &mt->fdescr, TSD->currlevel->currnumsize );
   }

   /*
    * We have done the "call CheckArgs" of the ANSI function.
    */

   /*
    * In the simplest case the first is the only argument.
    */
   if ( ( Before == -1 ) && ( After == -1 ) && ( Expp == -1 ) && ( Expt == -1 ) )
      return str_norm( TSD, &mt->fdescr, NULL );

   if (Expt == -1)
      Expt = TSD->currlevel->currnumsize;

   /*
    * The number is already set up but check twice that we don't have leading
    * zeros:
    */
   str_strip( &mt->fdescr );

   /*
    * Trailing zeros are confusing, too:
    */
   while ( ( mt->fdescr.size > 1 )
      && ( mt->fdescr.num[mt->fdescr.size - 1] == '0' ) )
      mt->fdescr.size--;

   Sign = ( mt->fdescr.negative ) ? 1 : 0;

   /*
    * Now compute the Exponent str_norm would use to format the number.
    * Don't keep care for ENGINEERING. Note that this equals to the result
    * We can determine the value of ShowExp en passent. This shortens our
    * approach to ANSI's algorithm significantly.
    */
   ShowExp = 0;
   Exponent = 0;
   if ( ( ( Expp != 0 ) &&
          ( ( mt->fdescr.exp < -5 ) || ( mt->fdescr.exp > Expt ) ) ) ||
        ( ( Expt == 0 ) && !StrictAnsi /* fixes bug 562668 */ ) )
   {
      ShowExp = 1;
      Exponent = mt->fdescr.exp - 1;
   }

   /* The number is normalized, now.
    * Usage of the variables:
    * mt->fdescr.num: Mantissa in the ANSI-standard and defined as usual, zeros
    *                 may be padded at the end but never at the start because:
    * mt->fdescr.exp: true exponent for the mantissa. The point is just before
    *                 the mantissa.
    * Exponent:       Used Exponent for the mantissa, e.g.:
    *                 mt->fdescr.num=1,mt->fdescr.exp=2 = 0.1E2 = 10E0
    *                 In this case Exponent may be 0 to reflect the exponent we
    *                 should display.
    * Point:          Defined in the standard but not used. It is obviously
    *                 equal to (mt->fdescr.exp-Exponent) where Point must
    *                 be inserted before.
    * examples with both mt->fdescr.num=Mantissa="101" and mt->fdescr.exp=-2:
    *    Exponent=0:  output may be "0.00101"
    *    Exponent=-3: output may be "1.01E-3"
    */

   /*
    * The fourth and fifth arguments allow for exponential notation.
    *
    * Decide whether exponential form to be used, setting ShowExp.
    * (Done above).
    *
    * These tests have to be on the number before any rounding since
    * decision on whether to have exponent affects what digits surround
    * the decimal point.
    *
    * Sign, Mantissa(mt->fdescr.num) and Exponent now reflect the Number.
    * Keep in mind that the Mantissa of a num_descr is always normalized to
    * a value smaller than 1. Thus, mt->fdescr(num=1,exp=1) means 0.1E1=1)
    *
    * ShowExp now indicates whether to show an exponent.
    */
   if ( ShowExp )
   {
      h = Exponent % 3;
      if ( ( TSD->currlevel->numform == NUM_FORM_ENG ) && h )
      {
         /*
          * Integer division may return values < 0
          */
         if ( h < 0 )
            h += 3;
         Exponent -= h;
         if ( StrictAnsi )
         {
            /*
             * As a side effect, ANSI adds zeros automatically. This must be
             * honoured if after isn't given.
             */
            Enlarge( mt->fdescr, h );
            memset( mt->fdescr.num + mt->fdescr.size, '0', h );
            mt->fdescr.size += h;
         }
      }
   }

   /*
    * Deal with right of decimal point first since that can affect the
    * left. Ensure the requested number of digits there.
    * Afters = length(Mantissa) - Point, thus;
    */
   Afters = mt->fdescr.size - ( mt->fdescr.exp - Exponent );
   if ( After == -1 )
      After = Afters;  /* Note default. */

   /*
    * The following happens due to our excessive trimming of zeros.
    */
   if ( After < 0 )
      After = 0;

   /* Make Afters match the requested After */
   if ( Afters < After )
   {
      /*
       * We have to add (After - Afters) zeros. This can be done more
       * efficiently later.
       */
   }
   else if ( Afters > After )
   {
      /*
       * Don't forget the most needed thing. We need it later to determine
       * the number of zeros being added as 0.
       */
      Afters = After;

      /*
       * Round by adding 5 at the right place.
       * Regina uses a different algorithm.
       */

      h = mt->fdescr.exp - Exponent + After; /* aka Point + After */

      mt->fdescr.size = h;

      if ( ( h < 0 ) || ( ( h == 0 ) && ( mt->fdescr.num[0] < '5' ) ) )
      {
         /*
          * Round to zero. We may not have any usable characters in the
          * mantissa, so create one.
          */
         mt->fdescr.num[0] = '0';
         mt->fdescr.size = 1;
         Sign = 0;
      }
      else if ( mt->fdescr.num[h] >= '5' )
      {
         for ( h--; h >= 0; h-- )
         {
            if ( ++mt->fdescr.num[h] <= '9' )
               break;
            mt->fdescr.num[h] = '0';
         }

         /*
          * We have a carry one in front if h < 0.
          * In this case we have to re-adjust the Exponent which is pretty
          * difficult in ENGINEERING notation.
          */
         if ( h < 0 )
         {
            Enlarge( mt->fdescr, 1 );
            memmove( mt->fdescr.num + 1, mt->fdescr.num, mt->fdescr.size );
            mt->fdescr.size++;
            mt->fdescr.num[0] = '1';
            mt->fdescr.exp++;

            /* The hard part follows */
            if ( mt->fdescr.exp - Exponent > Expt )
            {
               if ( StrictAnsi )
               {
                  ShowExp = 1;
               }
               else
               {
                  if ( Expp != 0 )
                     ShowExp = 1;
               }
            }
            if (ShowExp)
            {
               Exponent = mt->fdescr.exp - 1;
               h = Exponent % 3;
               if ( ( TSD->currlevel->numform == NUM_FORM_ENG ) && h )
               {
                  /*
                   * Integer division may return values < 0
                   */
                  if ( h < 0 )
                     h += 3;
                  Exponent -= h;
               }
            }
         }
      }
      else
      {
         /*
          * This can leave the result zero. The remaining zero-characters
          * shall persist, but the sign may change.
          */
         for ( h--; h >= 0; h-- )
         {
            if ( mt->fdescr.num[h] != '0' )
               break ;
         }
         if ( h < 0 )
         {
            Sign = 0;
         }
      }
   }
   /*
    * Rounded
    * That's all for now with the right part
    */

   /*
    * Now deal with the part of the result before the decimal point.
    * Point doesn't change never more.
    */
   Point = mt->fdescr.exp - Exponent;
   h = Point;

   /*
    * missing front of the number?
    * assume 1 char for "0" of "0.xxx"
    */
   if ( h <= 0 )
      h = 1;
   if ( Before == -1 )
      Before = h + Sign;
   /*
    * Make Point match Before
    */
   if ( h > Before - Sign )
   {
      exiterror( ERR_INCORRECT_CALL, 38, "FORMAT", 2, tmpstr_of( TSD, input ) );
   }
   /*
    * We don't fill up leading zeros as documented in the standard. Useless!
    */

   /*
    * We check the length of the exponent field, first. This allows to
    * allocate a sufficient string for the complete number.
    */
   OrigExpp = Expp;
   if ( ShowExp )
   {
      /*
       * Format the exponent.
       */
      sprintf( Expart, "%+d", Exponent );
      ExponentLen = strlen( Expart ) - 1;
      if ( Expp == -1 )
         Expp = ExponentLen;

      if ( ExponentLen > Expp )
      {
         exiterror( ERR_INCORRECT_CALL, 38, "FORMAT", 4, tmpstr_of( TSD, input ) );
      }
   }
   else
   {
      /*
       * no exponent
       */
      Expp = 0;
   }

   bufsize = Before + After + Expp + 4; /* Point, "E+", term. zero */
   buf = (char *)MallocTSD(bufsize);

   /*
    * Now do the formatting, it's a little bit complicated, since the parts
    * of the number may not exist (partially).
    *
    * Format the part before the point
    */
   if ( Point <= 0 )
   {
      /*
       * denormalized number
       */
      assert( Before >= 1 + Sign );
      memset( buf, ' ', Before - 1 );
      buf[Before - 1] = '0';
      if ( Sign )
         buf[Before - 2] = '-';
   }
   else
   {
      memset( buf, ' ', Before - Point );
      if ( ( h = Point ) > mt->fdescr.size )
         h = mt->fdescr.size;
      memcpy( buf + Before - Point, mt->fdescr.num, h );
      memset( buf + Before - Point + h, '0', Point - h );
      if ( Sign )
         buf[Before - Point - 1] = '-';
   }
   bufpos = Before;

   /*
    * Process the part after the decimal point
    */
   if ( After > 0 )
   {
      buf[bufpos++] = '.';
      if (Point < 0)
      {
         /*
          * Denormalized mantissa, we must fill up with zeros
          */
         h = -Point;
         if ( h > After )
            h = After; /* beware of an overrun */
         memset( buf + bufpos, '0', h );
         if ( After - h <= mt->fdescr.size )
         {
            memcpy( buf + bufpos + h, mt->fdescr.num, After - h );
         }
         else
         {
            memcpy( buf + bufpos + h, mt->fdescr.num, mt->fdescr.size );
            memset( buf + bufpos + h + mt->fdescr.size,
                    '0',
                    After - h - mt->fdescr.size );
         }
      }
      else
      {
         if ( After + Point <= mt->fdescr.size )
         {
            memcpy( buf + bufpos, mt->fdescr.num + Point, After );
         }
         else
         {
            /*
             * number of After characters in the mantissa?
             */
            if ( ( h = mt->fdescr.size - Point ) < 0 )
               h = 0;

            memcpy( buf + bufpos, mt->fdescr.num + Point, h );
            memset( buf + bufpos + h, '0', After - h );
         }
      }
      bufpos += After;
   }

   /* Finally process the exponent. ExponentBuffer contents the exponent
    * without the sign.
    */
   if ( ShowExp )
   {
      if ( Exponent == 0 )
      {
         if ( OrigExpp != -1 )
         {
            memset( buf + bufpos, ' ', Expp + 2 );
            bufpos += Expp + 2;
         }
      }
      else
      {
         buf[bufpos++] = 'E';
         buf[bufpos++] = Expart[0];
         memset( buf + bufpos, '0', Expp - ExponentLen );
         memcpy( buf + bufpos + Expp - ExponentLen, Expart + 1, ExponentLen );
         bufpos += Expp;
      }
   }

   assert( bufpos < bufsize );
   buf[bufpos] = '\0';

   retval = Str_creTSD( buf );
   FreeTSD( buf );
   return retval;
#undef Enlarge
}

/*
 * str_norm does the "PostOp" operation of the ANSI standard. It throws
 * away leading zeros and does some rounding with DIGITS of the time the
 * number was generated. try (if non-NULL) is used to print the number and is
 * returned. Never use try again after the call with the exception of
 * "x = str_norm(?,?,x)".
 *
 * The return value is the printable number.
 *
 * The value "in" may be rounded and reformatted.
 */
streng *str_norm( const tsd_t *TSD, num_descr *in, streng *trystr )
{
   streng *result;
   int i;
   int size,exp,ccns,Point;
   mat_tsd_t *mt;

   mt = (mat_tsd_t *)TSD->mat_tsd;

   ccns = in->used_digits;
   /*
    * We use ccns for the allocation of the string's content. Chop this value
    * is case of number which doesn't need billions of digits.
    */
   if ( ( exp = in->exp ) < 0 )
      exp = -exp;
   if ( ccns > in->size + exp + 10 )
      ccns = in->size + exp + 10 ;
   /*
    * The longest number produced from a num_descr is (with DIGITS=i)
    * -1.2...iE-MAX_EXPONENT
    * and its length is DIGITS + length(MAX_EXPONENT) + strlen(-.E-\0)
    */
   IS_AT_LEAST( mt->norm_out, mt->norm_outsize, ccns + mt->max_exponent_len + 6 );
#ifdef TRACEMEM
   mt->outptr2 = mt->norm_out;
#endif

   /*
    * remove effect of leading zeros in the descriptor
    */
   for ( i = 0; i < in->size; i++ )
   {
      if ( in->num[i] != '0' )
         break;
   }
   if ( i )
   {
      memmove( in->num, in->num + i, in->size - i );
      in->exp -= i;
      in->size -= i;
   }

   /*
    * We may have a number without mantissa. Even a rounding with DIGITS==1
    * will always produce a non-zero number. We can therefore do the test
    * before every other and return "0" in case of a mantissa with zeros.
    */
   if ( in->size == 0 )
   {
      in->size = 1;
      in->exp = 1;
      in->negative = 0;
      in->num[0] = '0';
      if ( trystr )
      {
         if ( trystr->max )
         {
            trystr->value[0] = '0';
            trystr->len = 1;
         }
         else
         {
            Free_stringTSD( trystr );
            trystr = Str_creTSD( "0" );
         }
      }
      else
         trystr = Str_creTSD( "0" );
#if NOT_YET_TESTED
      /* free memory to fix memory leak; Bug #455 */
      if (mt->norm_out)
      {
         FreeTSD(mt->norm_out);
         mt->norm_out = NULL;
         mt->norm_outsize = 0;
      }
#endif

      return trystr;
   }

   /*
    * Do the rounding needed for DIGITS. It may be to late here for doing this.
    * The user may have changed DIGITS between the operation and this function.
    */

   i = ccns;
   if ( in->size > i )
   {
      in->size = i;
      if ( in->num[i] >= '5' )
      {
         for ( i--; i >= 0; i-- )
         {
            if ( ++in->num[i] <= '9' )
            {
               break;
            }
            in->num[i] = '0';
         }
         if ( i < 0 )
         {
            /*
             * "Carry"
             */
            memmove( in->num + 1, in->num, in->size - 1 );
            in->num[0] = '1';
            in->exp++;
         }
      }
      /*
       * This may have produced leading zeros.
       */
   }

   /*
    * Truncation of trailing zeros must be done by the operations themself.
    * We are not allowed to cut them away, even after a decimal point.
    */

   exp = in->exp - 1;

   /*
    * Compute the exponent used to display. exp==0 -> don't show an exponent.
    * Respect the ENGINEERING format.
    * TRL states that:
    * We use Exponential format to display a number if the number of places
    * before the decimal is greater than DIGITS or if the number of places
    * after the decimal is greater than 2 * DIGITS (TRL2 page 136)
    * if ( ( exp >= ccns ) || ( abs(exp) > (2*ccns) ) )
    *
    * ANSI states that:
    * We use Exponential format to display a number if the number of places
    * before the decimal is greater than DIGITS or if the number of places
    * after the decimal is greater than 5 (ANSI page 52)
    * if ( ( exp >= ccns ) || ( abs(exp) > (2*ccns) ) )
    */
   if ( ( exp < -6 ) || ( exp >= ccns ) ) /* ANSI rules for displaying exponent */
   {
      i = exp % 3;
      if ( ( TSD->currlevel->numform == NUM_FORM_ENG ) && i )
      {
         /*
          * Integer division may return values < 0.
          */
         if ( i < 0 )
            i += 3;
         exp -= i;
      }
      if ( ( MAX_EXPONENT < exp ) || ( -MAX_EXPONENT > exp ) )
      {
         exiterror( ERR_ARITH_OVERFLOW, 0 )  ;
         return NULL ;
      }
   }
   else
   {
      exp = 0;
   }

   /*
    * Point points to the first char in the mantissa which is right of the
    * decimal point.
    */
   Point = in->exp - exp;

   size = 0;
   if ( in->negative )
      mt->norm_out[size++] = '-';

   /*
    * Process the part BEFORE the point.
    */
   if ( Point <= 0 )
   {
      /*
       * Something like "0.1". We have to provide an integer part.
       */
      mt->norm_out[size++] = '0';
   }
   else if ( Point <= in->size )
   {
      /*
       * Integer part exists and lays in the matissa completely.
       */
      memcpy( mt->norm_out + size, in->num, Point );
      size += Point;
   }
   else
   {
      /*
       * Integer part exists but is partially represented only, something
       * like "1e3" without trailing zeros.
       */
      memcpy( mt->norm_out + size, in->num, in->size );
      size += in->size;
      memset( mt->norm_out + size, '0', Point - in->size );
      size += Point - in->size;
   }

   /*
    * Process the part AFTER the point.
    */
   if ( Point < in->size )
   {
      /*
       * We have to show something as a fractional part.
       */
      mt->norm_out[size++] = '.';

      if ( Point < 0 )
      {
         /*
          * Something like 1E-3, leading zeros are missing.
          */
         memset( mt->norm_out + size, '0', -Point );
         size += -Point;
         memcpy( mt->norm_out + size, in->num, in->size );
         size += in->size;
      }
      else
      {
         /*
          * Something of the fractional part is there as induced by the
          * outer "if".
          */
         memcpy( mt->norm_out + size, in->num + Point, in->size - Point );
         size += in->size - Point;
      }
   }

   /*
    * We can add the exponent and that's it.
    */
   if ( exp != 0 )
   {
      size += sprintf( mt->norm_out + size, "E%+d", exp );
      /*
       * implicitely adds a \0 at the end.
       */
   }
   assert( size + 1 <= mt->norm_outsize );

   if ( trystr )
   {
      if ( trystr->max < size )
      {
          Free_stringTSD( trystr );
          trystr = NULL;
      }
   }
   if ( trystr )
      result = trystr;
   else
      result = Str_makeTSD( size );
   result->len = size;
   memcpy( result->value, mt->norm_out, size );
#if NOT_YET_TESTED
   /* free memory to fix memory leak; Bug #455 */
   if (mt->norm_out)
   {
      FreeTSD(mt->norm_out);
      mt->norm_out = NULL;
      mt->norm_outsize = 0;
   }
#endif
   return result ;
}

int string_test( const tsd_t *TSD, const num_descr *first,
                 const num_descr *second )
{
   int i=0, top=0, fnul=0, snul=0 ;
   char fchar=' ', schar=' ' ;
   int ccns = TSD->currlevel->currnumsize;

   if ( first->negative != second->negative )  /* have different signs */
      return ( first->negative ? -1 : 1 ) ;

   fnul = ( first->size==1 && first->exp==1 && first->num[0]=='0') ;
   snul = ( second->size==1 && second->exp==1 && second->num[0]=='0') ;
   if (fnul || snul)
   {
      if (fnul && snul) return 0 ;
      if (fnul) return (second->negative ? 1 : -1 ) ;
      else      return (first->negative ? -1 : 1 ) ;
   }

   if ( first->exp != second->exp ) /* have different order */
      return (log_xor( first->negative, first->exp>second->exp ) ? 1 : -1 ) ;

   /* same order and sign, have to compare ccns - TSD->currlevel->numfuzz first */
   top = MIN( ccns - TSD->currlevel->numfuzz, MAX( first->size, second->size )) ;
   for ( i=0; i<top; i++ )
   {
      fchar = (char) ((first->size > i) ? first->num[i] : '0') ;
      schar = (char) ((second->size > i) ? second->num[i] : '0') ;
      if ( fchar != schar )
         return log_xor( first->negative, fchar>schar ) ? 1 : -1 ;
   }

   /* hmmm, last resort: can the numbers be rounded to make a difference */
   fchar = (char) ((first->size > i) ? first->num[i] : '0') ;
   schar = (char) ((second->size > i) ? second->num[i] : '0') ;
   if (((fchar>'4') && (schar>'4')) || ((fchar<'5') && (schar<'5')))
      return 0 ;  /* equality! */

   /* now, one is rounded upwards, the other downwards */
   return log_xor( first->negative, fchar>'5' ) ? 1 : -1 ;
}



num_descr *string_incr( tsd_t *TSD, num_descr *input, cnodeptr node )
{
   int last,ccns=TSD->currlevel->currnumsize;
   char *cptr=NULL ;

   assert( input->size > 0 ) ;

   if (input->size != input->exp || input->exp >= ccns)
   {
      static const num_descr one = { "1", 0, 1, 1, 1, -1 } ;

      string_add( TSD, input, (num_descr *) &one, input, node, NULL ) ;
      str_round(input,ccns) ;
      return input ;
   }

   cptr = input->num ;
   last = input->size;


   LOSTDIGITS_CHECK( input, ccns, node );
   /*
    * No LOSTDIGITS check for "1". If this fails, everything fails...
    */
   last--;

   for (;;)
   {
      if (input->negative)
      {
         if (cptr[last] > '1')
         {
            cptr[last]-- ;
            input->used_digits = ccns;
            return input ;
         }
         else if (cptr[last]=='1')
         {
            cptr[last]-- ;
            if (last==0)
               str_strip( input ) ;
            input->used_digits = ccns;
            return input ;
         }
         else
         {
            assert( cptr[last] == '0' ) ;
            assert( last ) ;
            cptr[last--] = '9' ;
         }
      }
      else
      {
         if (cptr[last] < '9')
         {
            cptr[last]++ ;
            input->used_digits = ccns;
            return input ;
         }
         else
         {
            assert( cptr[last] == '9' ) ;
            cptr[last--] = '0' ;
         }
      }

      if (last<0)
      {
         if (input->size >= input->max)
         {
            char *newnum ;

            assert( input->size == input->max ) ;
            newnum = (char *)MallocTSD( input->max * 2 + 2 ) ;
            memcpy( newnum+1, input->num, input->size ) ;
            newnum[0] = '0' ;
            input->size++ ;
            input->exp++ ;
            input->max = input->max*2 + 2 ;
            FreeTSD( input->num ) ;
            cptr = input->num = newnum ;
         }
         else
         {
            memmove( input->num+1, input->num, input->size ) ;
            input->size++ ;
            input->exp++ ;
            input->num[0] = '0' ;
         }
         last++ ;
      }
   }
}


/*
 * tests for an ANSI compatible whole number. Look at myiswnumber()
 * for a description.
 */
static int test_whole( const tsd_t *TSD, const num_descr *input,
                       int noDigitsLimit )
{
   int i;

   if ( input->size > input->exp )
   {
      /*
       * Check for non-zeros in the fractional part of the number.
       */
      i = MAX( 0, input->exp );
      for ( ; i < input->size; i++ )
      {
         if ( input->num[i] != '0' )
            return 0;
      }
   }

   if ( !noDigitsLimit )
   {
      for (i = 0; i < input->size; i++)
      {
         if (input->num[i] != '0')
            break;
      }
      if (i < input->size)
      {
         /* not a 0 */
         if (input->exp - i > TSD->currlevel->currnumsize)
            return 0;
      }
   }
   return 1;
}


/*
 * Division in the typical manner we learn in school hopefully.
 *
 * type is DIVTYPE_NORMAL for floating point division, DIVTYPE_INTEGER for
 * division without remainer, DIVTYPE_REMAINER if the remainer is interested in
 * and DIVTYPE_BOTH if both the integer part and the remainer shall be
 * returned.
 *
 * We compute f/s with a NUMERIC DIGITS value of ccns.
 *
 * The return value is put into *r, *r2 holds the remainer if DIVTYPE_BOTH
 * is set.
 *
 * We throw an error on non-floating point division if the COMPLETE integer
 * part of the division can't be represented without rounding.
 */
static void string_div2( tsd_t *TSD, const num_descr *f, const num_descr *s,
                         num_descr *r, num_descr *r2, int type, int ccns )
{
   int ssize,tstart,tcnt,finished=0,tend;
   int i,cont,outp,test,loan;
   int origneg,origexp;
   mat_tsd_t *mt;

   mt = (mat_tsd_t *)TSD->mat_tsd;

   IS_AT_LEAST( mt->div_out, mt->div_outsize, (ccns+1) * 2 + 1 );
   IS_AT_LEAST( r->num, r->max, ccns+1 );
#ifdef TRACEMEM
   mt->outptr3 = mt->div_out;
#endif

   /*
    * We don't want to strip leading zeros here!
    */
   assert( ( ( f->size > 1 ) && ( f->num[0] != '0' ) ) || ( f->size == 1 ) );
   assert( ( s->size != 0 ) && ( s->num[0] != '0' ) );

   /*
    * ssize is the count of the used digits from s's mantissa.
    */
   ssize = MIN( s->size, ccns+1 );

   /*
    * Compute the trivial parts of the result.
    * Imagine xxxxx : yy = zzzz, probably with zeros.
    */
   r->exp = 1 + f->exp - s->exp;
   r->negative = log_xor( f->negative, s->negative );

   /*
    * Initialize the pointers.
    * tstart, tend, tcnt
    */
   tstart = 0;
   tend = tstart + MIN( f->size, ccns+1 );

   /*
    * First, fill div_out with f as the residual. Fill up with zeros.
    */
   for ( tcnt = tstart; tcnt < ssize; tcnt++ )
      mt->div_out[tcnt] = (char) ( ( tcnt < tend ) ? f->num[tcnt] : '0' );

   /*
    * Imagine xxxxx : yy again. If the first length(yy) digits of xxxxx
    * are smaller than yy, we have to set the first digit of z to 0. For
    * entering the main algorithm, we do the step here decrementing the
    * result's exponent, which if mathematically the same.
    * e.g. 12345 : 23 = 0zzz
    */

   for ( i = 0; i < ssize; i++ )
   {
      if ( mt->div_out[i] > s->num[i] )
         break;
      else if ( mt->div_out[i] < s->num[i] )
      {
         /*
          * Fetch next digit of f for the next iteration, remember the school.
          */
         mt->div_out[tcnt] = (char) ( ( tcnt < tend) ? f->num[tcnt] : '0' );
         tcnt++;
         r->exp--;
         break;
      }
   }

   /*
    * Situation: s->num[0..ssize-1] contains the divisor, and the array
    * div_out[tstart==0..tcnt-1] hold the (first part of the) dividend. The
    * array f->num[tcnt..tend-1] (which may be empty) holds the last
    * part of the dividend.
    *
    * We compute (the first part of) div_out : s
    *
    * Iterate through each digit of div_out, fetching the next digit from
    * f if available.
    */
   for ( outp = 0; outp < ccns+1 && !finished; outp++ )
   {
      /*
       * Assume 0 as the result for the next digit. We may increment it below
       * some times.
       */
      r->num[outp] = '0';
      if ( ( tcnt - tstart > ssize ) && ( mt->div_out[tstart] == '0' ) )
         tstart++;

      /*
       * Stop the iteration if this is integer division, and we have hit the
       * decimal point.
       */
      if ( ( type != DIVTYPE_NORMAL ) && ( outp >= r->exp ) )
      {
         finished = 1 ;
         continue ;
      }

      /*
       * Try to subtract as many times as possible, that is, compute the
       * next digit of the result. Our example in the second step:
       * 12 345  : 23 = 0 zzz (before iteration)
       * 123 45  : 23 = 05 zz (first iteration)
       * 00 84 5 : 23 = 053 z (84 contains 3 times 23)
       */
      for ( cont = 1; cont; )
      {
         /*
          * If the current operation works on equal sized numbers (e.g.
          * second iteration), we have to compare if we can do the next
          * subtraction. This isn't necessary if (tcnt-tstart) > ssize, which
          * means the partial dividend (123 in first iteration) is longer
          * than the divisor (23, only two chars). xx always is smaller than
          * yyy if they don't start with 0.
          */
         if ( tcnt - tstart == ssize )
         {
            for ( i = 0; i < ssize; i++ )
            {
               test = mt->div_out[tstart + i] - s->num[i];
               if ( test < 0 )
                  cont = 0;
               if ( test != 0 )
                  break;
            }
         }

         /*
          * If we can continue, subtract it.
          */
         loan = 0;
         if ( cont )
         {
            r->num[outp]++;
            for ( i = 0; i < ssize; i++ )
            {
               char h = (char) ( s->num[ssize-1-i] - '0' + loan );
               mt->div_out[tcnt-1-i] = (char) ( mt->div_out[tcnt-1-i] - h );
               if ( ( loan = (mt->div_out[tcnt-1-i] < '0' ) ) != 0 )
                  mt->div_out[tcnt-1-i] += 10;
            }
            if ( loan )
            {
               /*
                * decrement it and check for '0'
                */
               mt->div_out[tstart] -= 1;
               if ( ( tcnt - tstart > ssize ) &&
                                               ( mt->div_out[tstart] == '0' ) )
                  tstart++;
            }

         }
      } /* for each possible subtraction */

      if ( ( tcnt - tstart > ssize ) && ( mt->div_out[tstart] == '0' ) )
         tstart++;

      /*
       * Do we have anything left of the dividend? This is only meaningful if
       * all digits in the original divident have been processed, it is
       * also safe to assume that divident and divisor have equal sizes.
       */

      assert( tcnt-tstart == ssize );
      mt->div_out[tcnt] = (char) ( ( tcnt < tend ) ? f->num[tcnt] : '0' );
      if ( ++tcnt > tend )
      {
         finished = 1;
         for ( i = tstart; i < tcnt; i++ )
         {
            if ( mt->div_out[i] != '0' )
            {
               finished = 0;
               break;
            }
         }
      }

   } /* for each digit wanted in the result */

   if ( type != DIVTYPE_NORMAL )
   {
      /*
       * fixes bug 687399
       *
       * Perform a validity check. We may got a remainder bigger than
       * the residual. It indicates a rounded integer part value.
       * The residual in div_out[tstart..tcnt-1] counted from div_out[0] is
       * f->exp based.
       * Find the first non-zero in the residiual and continue then.
       */
      finished = 1;
      test = MIN( MAX( tend, tcnt ) - tstart, ccns + 1 );
      for ( i = 0; i < test; i++ )
      {
         char h;
         h = (char) ( ( i < tcnt - tstart ) ? mt->div_out[tstart+i] :
                                              f->num[tstart+i] );
         if ( h != '0' )
            break;
      }

      /*
       * s begins withs a non-zero as the digit at tstart+i. Only compare the
       * numbers if the residual may be greater than s.
       */
      if ( ( f->exp - tstart - i >= s->exp ) && ( i < test ) )
      {
         if ( f->exp - tstart - i > s->exp )
         {
            /*
             * The residual has a higher exponent. We have definitely an error.
             */
            finished = 0;
         }
         else
         {
            /*
             * This fits many situations. The exponent is the same, we have
             * to compare the digits of the number.
             */
            int j;

            test = MIN( test - i, ssize );
            for ( j = 0; j < test; j++, i++ )
            {
               int h;
               h = ( i < tcnt - tstart ) ? mt->div_out[tstart+i] :
                                           f->num[tstart+i];
               h -= s->num[j];
               if ( h > 0 )
                  finished = 0;
               if ( h != 0 )
                  break;
            }
            /*
             * We still can have an error. Imagine a residual of 22 and a
             * divisor of 2e1.
             */
            if ( ( j >= test ) && ( ssize > test ) && ( test > 0 ) )
               finished = 0;
         }
      }

      /*
       * We perform the operation with DIGITS+1 precision for a later
       * rounding and to prevent math errors. We have to check if rounding
       * would occur later.
       */
      if ( finished )
      {
         if ( ( outp > ccns ) && ( r->num[ccns] != '0' ) )
            finished = 0;
         /*
          * FGC: I'm not sure whether the following test supersedes the
          * the testing of the mantissa above. It should, but who can
          * prove?
          */
         r->size = outp;
         if ( !test_whole( TSD, r, 0 ) )
            finished = 0;
      }
      if ( !finished )
      {
         volatile char *fs, *ss;
         streng *h;

         h = name_of_node( TSD, NULL, f );
         fs = tmpstr_of( TSD, h );
         Free_stringTSD( h );
         h = name_of_node( TSD, NULL, s );
         ss = tmpstr_of( TSD, h );
         Free_stringTSD( h );
         exiterror( ERR_INVALID_INTEGER,
                    ( type == DIVTYPE_REMAINDER ) ? 12 : 11,
                    fs, ss, ccns);
      }
   }

   origexp = f->exp;
   origneg = f->negative;

   if ( type == DIVTYPE_BOTH )
   {
      /*
       * Return both answers
       */
      IS_AT_LEAST( r2->num, r2->max, outp );

      memcpy( r2->num, r->num, outp );
      r2->negative = r->negative;
      r2->size = r->size;
      r2->exp = r->exp;

      for ( r2->size = outp; ( r2->size > r2->exp ) && ( r2->size > 1 );
                                                                   r2->size-- )
      {
         if ( r2->num[r2->size-1] != '0' )
            break;
      }
   }

   if ( ( type == DIVTYPE_REMAINDER ) || ( type == DIVTYPE_BOTH ) )
   {
      /*
       * We are really interested in the remainder, so swap things
       */
      for ( i = 0; i < MIN( MAX( tend, tcnt ) - tstart, ccns + 1 ); i++ )
         r->num[i] = (char) ( ( i < tcnt - tstart ) ? mt->div_out[tstart+i] :
                                                      f->num[tstart+i] );

      r->size = outp = i;
      r->exp = origexp - tstart;
      r->negative = origneg;
   }

   /*
    * Then, at the end, we have to strip of trailing zeros that come
    * after the decimal point, first do we have any decimals?
    */
   for ( r->size = outp; ( r->size > r->exp ) && ( r->size > 1 ); r->size-- )
   {
      if ( r->num[r->size - 1] != '0' )
         break;
   }
   /* free memory to fix memory leak; Bug #455 */
   if (mt->div_out)
   {
      FreeTSD(mt->div_out);
      mt->div_out = NULL;
      mt->div_outsize = 0;
   }
}

void string_div( tsd_t *TSD, const num_descr *f, const num_descr *s,
                 num_descr *r, num_descr *r2, int type, cnodeptr left,
                 cnodeptr right )
{
   int ccns = TSD->currlevel->currnumsize;

   LOSTDIGITS_CHECK( f, ccns, left );
   LOSTDIGITS_CHECK( s, ccns, right );

   string_div2( TSD, f, s, r, r2, type, ccns );

   if ( r != NULL )
      r->used_digits = ccns;
   if ( r2 != NULL )
      r2->used_digits = ccns;
}

/* The multiplication table for two single-digits numbers */
static const char mult[10][10][3] = {
   { "00", "00", "00", "00", "00", "00", "00", "00", "00", "00" },
   { "00", "01", "02", "03", "04", "05", "06", "07", "08", "09" },
   { "00", "02", "04", "06", "08", "10", "12", "14", "16", "18" },
   { "00", "03", "06", "09", "12", "15", "18", "21", "24", "27" },
   { "00", "04", "08", "12", "16", "20", "24", "28", "32", "36" },
   { "00", "05", "10", "15", "20", "25", "30", "35", "40", "45" },
   { "00", "06", "12", "18", "24", "30", "36", "42", "48", "54" },
   { "00", "07", "14", "21", "28", "35", "42", "49", "56", "63" },
   { "00", "08", "16", "24", "32", "40", "48", "56", "64", "72" },
   { "00", "09", "18", "27", "36", "45", "54", "63", "72", "81" },
} ;


static void string_mul2( tsd_t *TSD, const num_descr *f, const num_descr *s,
                         num_descr *r, int ccns )
{
   char *outp;
   const char *answer;
   int i,sskip,fskip,sstart,fstart,base,offset,carry,j;
   mat_tsd_t *mt;

   mt = (mat_tsd_t *)TSD->mat_tsd;

   IS_AT_LEAST( mt->mul_out, mt->mul_outsize, 2*(ccns+1) ) ;
#ifdef TRACEMEM
   mt->outptr4 = mt->mul_out ;
#endif

   for (i=0; i<2*(ccns+1); mt->mul_out[i++]='0') ;
   outp = &mt->mul_out[2*(ccns+1)-1] ;

   for (sskip=0; (sskip<s->size) && (s->num[sskip]=='0'); sskip++ ) ;
   sstart = MIN( sskip+ccns, s->size-1 ) ;

   for (fskip=0; (fskip<f->size) && (f->num[fskip]=='0'); fskip++ ) ;
   fstart = MIN( fskip+ccns, f->size-1 ) ;

   base = 2*(ccns+1)-1 ;
   offset = carry = 0 ;
   /*
    * Use a maximum of DIGITS+1 significant digits on input for each operand.
    */
   for ( i=sstart; i>=sskip; i-- )
   {
      offset = carry = 0 ;
      assert( base >= 0 ) ;
      for ( j=fstart; j>=fskip; j-- )
      {
         answer = mult[f->num[j]-'0'][s->num[i]-'0'] ;
         assert( base-offset >= 0 ) ;
         /* Stupid MSVC likes this only: */
         mt->mul_out[base-offset] = (char) (mt->mul_out[base-offset] +
                                            answer[1] - '0' + carry) ;
         carry = answer[0] - '0' ;
         while ( mt->mul_out[base-offset] > '9' )
         {
            mt->mul_out[base-offset] -= 10 ;
            carry++ ;
         }
         offset++ ;
      }
      if (base-offset >= 0)
         mt->mul_out[base-offset++] = (char) (carry + '0') ;
      else
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;

      base-- ;
   }

   IS_AT_LEAST( r->num, r->max, /*2*(ccns+1)*/
                              outp - mt->mul_out-base+offset  ) ;
   j = 0 ;
   for (i=base-offset+2; (i<=outp - mt->mul_out); i++ )
      r->num[j++] = mt->mul_out[i] ;

   if (j==0)
   {
      r->num[j++] = '0' ;
      r->exp = 1 ;
   }
   else
      r->exp = s->exp + f->exp ;

   r->negative = log_xor( f->negative, s->negative ) ;
   r->size = j ;
   str_round( r, ccns ) ;
   /* free memory to fix memory leak; Bug #455 */
   if (mt->mul_out)
   {
      FreeTSD(mt->mul_out);
      mt->mul_out = NULL;
      mt->mul_outsize = 0;
   }
}

void string_mul( tsd_t *TSD, const num_descr *f, const num_descr *s,
                 num_descr *r, cnodeptr left, cnodeptr right )
{
   int ccns = TSD->currlevel->currnumsize;

   LOSTDIGITS_CHECK( f, ccns, left );
   LOSTDIGITS_CHECK( s, ccns, right );

   string_mul2( TSD, f, s, r, ccns );

   r->used_digits = ccns;
}

static void descr_strip( const tsd_t *TSD, const num_descr *from, num_descr *to )
{
   int i=0, j=0 ;

   IS_AT_LEAST( to->num, to->max, TSD->currlevel->currnumsize+1 ) ;

   to->negative = from->negative ;
   for (i=0; (i<from->size) && (from->num[i]=='0'); i++ ) ;
   to->exp = from->exp - i ;
   for (j=0; j+i<from->size; j++ )
      to->num[j] = from->num[i+j] ;

   if ((to->exp-1 > MAX_EXPONENT) || ( -MAX_EXPONENT > to->exp+1))
      exiterror( ERR_ARITH_OVERFLOW, 0 )  ;

   to->size = j ;
   to->used_digits = from->used_digits;
}



void string_pow( tsd_t *TSD, const num_descr *num, num_descr *acc,
                 num_descr *res, cnodeptr lname, cnodeptr rname )
{
   static const num_descr one = { "1", 0, 1, 1, 2, -1 } ;
   int ineg=0, pow, cnt,power ;
   int ccns = TSD->currlevel->currnumsize;

   IS_AT_LEAST( res->num, res->max, ccns+1 ) ;

   LOSTDIGITS_CHECK( num, ccns, lname );
   LOSTDIGITS_CHECK( acc, ccns, rname );
   power = descr_to_int( TSD, acc, ERR_INVALID_INTEGER, 8, NULL, 0 ) ;

   IS_AT_LEAST( acc->num, acc->max, ccns+1 ) ;
   acc->exp = 1 ;
   acc->size = 1 ;
   acc->negative = 0 ;
   acc->num[0] = '1' ;

   if (power < 0)
   {
      power = -power ;
      ineg  = 1 ;
   }
   pow = power;

   for (cnt=0; pow; cnt++ )
      pow = pow>>1 ;

   for ( ;cnt ; )
   {
      if (power & (1<<(cnt-1)))
      {
         /* multiply acc with *f, and put answer into acc */
         string_mul2( TSD, acc, num, res, ccns ) ;
         assert( acc->size <= acc->max && res->size <= res->max ) ;
         descr_strip( TSD, res, acc ) ;
         assert( acc->size <= acc->max && res->size <= res->max ) ;
      }

      if ((--cnt)==0)
         break ;   /* horrible example of dataflow */

      /* then, square the contents of acc */
      string_mul2( TSD, acc, acc, res, ccns ) ;
      assert( acc->size <= acc->max && res->size <= res->max ) ;
      descr_strip( TSD, res, acc ) ;
      assert( acc->size <= acc->max && res->size <= res->max ) ;
   }

   if (ineg)
      /* may hang if acc==zero ? */
      string_div2( TSD, &one, acc, res, NULL, DIVTYPE_NORMAL, ccns ) ;
   else
      descr_strip( TSD, acc, res ) ;
   assert( acc->size <= acc->max && res->size <= res->max ) ;
   acc->used_digits = ccns;
}


/* ========= interface routines to the arithmetic routines ========== */

int descr_sign( const void *descr )
{
   return( ((num_descr*)descr)->negative ? -1 : 1 ) ;
}


void free_a_descr( const tsd_t *TSD, num_descr *in )
{
   assert( in->size <= in->max ) ;

   if ( in->num )
      FreeTSD( in->num ) ;

   FreeTSD( in ) ;
}


num_descr *get_a_descr( tsd_t *TSD, const char *bif, int argno,
                        const streng *num )
{
   mat_tsd_t *mt;
   num_descr *descr=NULL ;
   int i;
   descr = (num_descr *)MallocTSD( sizeof(num_descr) ) ;
   descr->max = 0 ;
   descr->num = NULL ;

   if ( ( i = getdescr( TSD, num, descr ) ) != 0 )
   {
      mt = (mat_tsd_t *)TSD->mat_tsd;

      FreeTSD( descr );
      if ( bif == NULL )
         exiterror( ERR_BAD_ARITHMETIC, 0 );

      if ( i == 9 )
         exiterror( ERR_INCORRECT_CALL, i, bif, argno, mt->max_exponent_len, tmpstr_of( TSD, num ) );
      else
         exiterror( ERR_INCORRECT_CALL, i, bif, argno, tmpstr_of( TSD, num ) );
   }

   return descr ;
}


int str_true( const tsd_t *TSD, const streng *input )
{
   if (input->len != 1)
       exiterror( ERR_UNLOGICAL_VALUE, 0 )  ;

   switch (input->value[0])
   {
      case '1':
         return 1 ;
      case '0':
         return 0 ;
      default:
          exiterror( ERR_UNLOGICAL_VALUE, 0 )  ;
   }

   /* Too keep the compiler happy */
   return 1 ;
}


streng *str_abs( tsd_t *TSD, const streng *input )
{
   mat_tsd_t *mt;
   streng *retval;
   int i;

   mt = (mat_tsd_t *)TSD->mat_tsd;
   if ( ( i = getdescr( TSD, input, &mt->fdescr ) ) != 0 )
   {
      if ( i == 9 )
         exiterror( ERR_INCORRECT_CALL, i, "ABS", 1, mt->max_exponent_len, tmpstr_of( TSD, input ) );
      else
         exiterror( ERR_INCORRECT_CALL, i, "ABS", 1, tmpstr_of( TSD, input ) );
   }

   if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
   {
      str_round_lostdigits( TSD, &mt->fdescr, TSD->currlevel->currnumsize );
      mt->fdescr.negative = 0;
      return str_norm( TSD, &mt->fdescr, NULL );
   }

   mt->fdescr.negative = 0;
   mt->fdescr.used_digits = mt->fdescr.size;
   retval = str_norm( TSD, &mt->fdescr, NULL );
   return retval;
}


streng *str_sign( tsd_t *TSD, const streng *input )
{
   mat_tsd_t *mt;
   char *mant;
   int i;

   mt = (mat_tsd_t *)TSD->mat_tsd;
   if ( ( i = getdescr( TSD, input, &mt->fdescr ) ) != 0 )
   {
      if ( i == 9 )
         exiterror( ERR_INCORRECT_CALL, i, "SIGN", 1, mt->max_exponent_len, tmpstr_of( TSD, input ) );
      else
         exiterror( ERR_INCORRECT_CALL, i, "SIGN", 1, tmpstr_of( TSD, input ) );
   }

   if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
   {
      str_round_lostdigits( TSD, &mt->fdescr, TSD->currlevel->currnumsize );
   }

   mant = mt->fdescr.num;
   for ( i = 0; i < mt->fdescr.size; i++ )
   {
      if ( mant[i] != '0' )
      {
         if ( mt->fdescr.negative )
         {
            return Str_creTSD( "-1" );
         }
         else
         {
            return Str_creTSD( "1" );
         }
      }
   }
   return Str_creTSD( "0" );
}


streng *str_trunc( tsd_t *TSD, const streng *number, int deci )
{
   int i=0, j=0, k=0, size=0, top=0 ;
   streng *result=NULL ;
   mat_tsd_t *mt;

   mt = (mat_tsd_t *)TSD->mat_tsd;

   /* first, convert number to internal representation */
   if ( ( i = getdescr( TSD, number, &mt->fdescr ) ) != 0 )
   {
      if ( i == 9 )
         exiterror( ERR_INCORRECT_CALL, i, "TRUNC", 1, mt->max_exponent_len, tmpstr_of( TSD, number ) );
      else
         exiterror( ERR_INCORRECT_CALL, i, "TRUNC", 1, tmpstr_of( TSD, number ) );
   }

   /* get rid of possible excessive precision */
   if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
   {
      str_round_lostdigits( TSD, &mt->fdescr, TSD->currlevel->currnumsize );
   }

   /* who big must the result string be? */
   if ((i=mt->fdescr.exp) > 0 )
      size = mt->fdescr.exp + deci ;
   else
      size = deci ;

   /*
    * Adrian Sutherland <adrian@dealernet.co.uk>
    * Changed the following line from '+ 2' to '+ 3',
    * because I was getting core dumps ... I think that we need this
    * because negative numbers BIGGER THAN -1 need a sign, a zero and
    * a decimal point ... A.
    */
   result = Str_makeTSD( size + 3 ) ;  /* allow for sign and decimal point */
   j = 0 ;

   if (mt->fdescr.negative)
      result->value[j++] = '-' ;

   /* first fill in the known numerals of the integer part */
   top = MIN( mt->fdescr.exp, mt->fdescr.size ) ;
   for (i=0; i<top; i++)
      result->value[j++] = mt->fdescr.num[i] ;

   /* pad out with '0' in the integer part, if necessary */
   for (k=i; k<mt->fdescr.exp; k++)
      result->value[j++] = '0' ;

   if (k==0)
      result->value[j++] = '0' ;

   k = 0 ;
   if (deci>0)
   {
      result->value[j++] = '.' ;

      /* pad with zeros between decimal point and number */
      for (k=0; k>mt->fdescr.exp; k--)
         result->value[j++] = '0' ;
   }

   /* fill in with the decimals, if any */
   top = MIN( mt->fdescr.size-mt->fdescr.exp, deci ) + i + k ;
   for (; i<top; i++ )
      result->value[j++] = mt->fdescr.num[i] ;

   /* pad with zeros if necessary */
   for (; i<deci+MIN(mt->fdescr.exp,mt->fdescr.size); i++ )
      result->value[j++] = '0' ;

   result->len = j ;
   assert( (result->len <= result->max) && (result->len<=size+2) ) ;

   return( result ) ;
}



/* ------------------------------------------------------------------
 * This function converts a packed binary string to a decimal integer.
 * It is equivalent of interpreting the binary string as a number of
 * base 256, and converting it to base 10 (the actual algorithm uses
 * a number of base 2, padded to a multiple of 8 digits). Negative
 * numbers are interpreted as two's complement.
 *
 * First parameter is the packed binary string; second parameter is
 * the number of initial characters to skip (i.e. the position of the
 * most significant byte in 'input'; the third parameter is a boolean
 * telling if this number is signed or not.
 *
 * The significance of the 'too_large' variable: If the number has
 * leading zeros, that is not an error, so the 'fdescr' might be set
 * to values larger than it can hold. However, the error occurs only
 * if that value is used. Therefore, if 'fdescr' becomes bigger than
 * the max whole number, 'too_large' is set. If attempts are made to
 * use 'fdescr' while 'too_large' is set, an error occurs.
 *
 * Note that this algoritm requires that string_mul and string_add
 * does not change anything in their first two parameters.
 *
 * The 'input' variable is assumed to have at least one digit, so don't
 * call this function with a null string. Maybe the compiler could
 * optimize this function better if [esf]descr were locals?
 *
 * In case of errors we throw SYNTAX(40,35).
 */

streng *str_digitize( tsd_t *TSD, streng *input, int start, int sign,
                      const char *bif, int removeStringOnError )
{
   int cur_byte=0 ;     /* current byte in 'input' */
   int cur_bit=0 ;      /* current bit in 'input' */
   int too_large=0 ;    /* error flag (see above) */
   int i, ccns;
   int user_ccns = TSD->currlevel->currnumsize;
   mat_tsd_t *mt;

   mt = (mat_tsd_t *)TSD->mat_tsd;

   /* do we have anything to work on? */
   assert( start < Str_len(input) );

   ccns = 3 * Str_len(input);

   /* ensure that temporary number descriptors has enough space */
   IS_AT_LEAST( mt->fdescr.num, mt->fdescr.max, ccns+2 ) ;
   IS_AT_LEAST( mt->edescr.num, mt->edescr.max, ccns+2 ) ;
   IS_AT_LEAST( mt->sdescr.num, mt->sdescr.max, ccns+2 ) ;

   /*
    * Initialize the temporary number descriptors: 'fdescr', 'sdescr'
    * and 'edescr'. They will be initialized to 0, 1 and 2 respectively.
    * They are used for:
    *
    *   fdescr: contains the value of the current bit of the current
    *           byte, e.g the third last bit in the last byte will
    *           have the value '0100'b (=4). This value is multiplied
    *           with two at each iteration of the inner loop. Is
    *           initialized to the value '1', and will have the same
    *           sign as 'input'.
    *
    *   sdescr: contains '2', to make doubling of 'fdescr' easy
    *
    *   edescr: contains the answer, initially set to '0' if 'input'
    *           is positive, or '-1' if 'input' is negative. The
    *           descriptor 'fdescr' is added to (or implicitly
    *           subtracted from) this number.
    */
   mt->fdescr.size     = mt->sdescr.size     = mt->edescr.size     = 1 ;
   mt->fdescr.negative = mt->sdescr.negative = mt->edescr.negative = 0 ;
   mt->fdescr.exp      = mt->sdescr.exp      = mt->edescr.exp      = 1 ;

   mt->edescr.num[0] = '0' ;   /* the resulting number */
   mt->fdescr.num[0] = '1' ;   /* the value of each binary digit */
   mt->sdescr.num[0] = '2' ;   /* the number to multiply 'fdescr' in */

   /*
    * If 'input' is signed, but positive, treat as if it was unsigned.
    * 'sign' is then effectively a boolean stating whether 'input' is
    * a negative number. In that case, 'edescr' should be set to '-1'.
    * Also, 'fdescr' is set to negative, so that it is subtracted from
    * 'edescr' when given to string_add().
    */
   if (sign)
   {
      if (input->value[start] & 0x80)
      {
         mt->edescr.num[0] = '1' ;
         mt->edescr.negative = 1 ;
         mt->fdescr.negative = 1 ;
      }
      else
         sign = 0 ;
   }

   /*
    * Each iteration of the outer loop will process a byte in 'input',
    * starting with the last (least significant) byte. Each iteration
    * of the inner loop will process one bit in the byte currently
    * processed by the outer loop.
    */
   for (cur_byte=Str_len(input)-1; cur_byte>=start; cur_byte--)
   {
      for (cur_bit=0; cur_bit<8; cur_bit++)
      {
         /*
          * does the precision hold? if not, set flag
          * The error can be considered to be a severe error. We should
          * always have "enough" precision. See ccns above.
          */
         if (mt->fdescr.size > ccns)
            too_large = 1 ;

         /*
          * If the current bit (the j'th bit in the i'th byte) is set
          * and input is positive; or if current bit is not set and
          * input is negative, then increase the value of the result.
          * This is not really a bitwise xor, but a logical xor, but
          * the values are always 1 or 0, so it doesn't matter.
          */
         if ((sign) ^ ((input->value[cur_byte] >> cur_bit) & 1))
         {
            if (too_large)
                exiterror( ERR_INVALID_INTEGER, 0 )  ;

            string_add2( TSD, &mt->edescr, &mt->fdescr, &mt->edescr, ccns );
         }

         /*
          * Str_ip away any leading zeros. If this is not done, the
          * accuracy of the operation will deter, since string_add()
          * return answer with leading zero, and the accumulative
          * effect of this would make 'edescr' zero after a few
          * iterations of the loop.
          */
         str_strip( &mt->edescr ) ;

         /*
          * Increase the value of 'fdescr', so that it corresponds with
          * the significance of the current bit in 'input'. But don't
          * do this if 'fdescr' isn't capable of holding that number.
          */
         if (!too_large)
         {
            string_mul2( TSD, &mt->fdescr, &mt->sdescr, &mt->fdescr, ccns );
            str_strip( &mt->fdescr ) ;
         }
      }
   }

   /*
    * normalize answer and return to caller. Always show all digits if we
    * don't have to support STRICT_ANSI. In ANSI we have to throw a SYNTAX
    * in case of number overflow.
    */
   if ( get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) )
   {
      for (i = 0; i < mt->edescr.size; i++)
      {
         if (mt->edescr.num[i] != '0')
            break;
      }
      if (i < mt->edescr.size)
      {
         /* not a 0 */
         if (mt->edescr.exp - i > user_ccns)
         {
            volatile char *msg;

            if ( removeStringOnError )
               Free_stringTSD( input );
            mt->edescr.used_digits = mt->edescr.size;
            input = str_norm( TSD, &mt->edescr, NULL );
            msg = tmpstr_of( TSD, input );
            Free_stringTSD( input );
            /* fixes bug 1112956 */
            exiterror( ERR_INCORRECT_CALL, 35, bif, msg );
         }
      }
      mt->edescr.used_digits = user_ccns;
   }
   else
   {
      int s,e;

      for ( s = 0; s < mt->edescr.size; s++ )
      {
         if ( mt->edescr.num[s] != '0' )
            break;
      }
      for ( e = mt->edescr.size - 1; e > s; e-- )
      {
         if ( mt->edescr.num[e] != '0' )
            break;
      }
      e -= s - 1;
      if ( e < 1 )
         e = 1;
      mt->edescr.used_digits = ( e < user_ccns ) ? user_ccns : e;
   }
   return str_norm( TSD, &mt->edescr, NULL );
}

streng *str_binerize( tsd_t *TSD, num_descr *num, int length, int errnum, int suberrnum, const char *bif, int argno )
{
   int i,ccns;
   streng *result;

   /*
    * We are going to need two number in this algoritm, so we can
    * just as well make them right away. We could initialize these on
    * the first invocation of this routine, and thereby saving some
    * space, but that would 1) take CPU on every invocation; 2) it
    * would probably cost just as much space in the text segment.
    * (Would have to set NUMERIC DIGIT to at least 4 before calling
    * getdescr with these.)
    */
   static const num_descr minus_one = {   "1", 1, 1, 1, 2, -1 } ;
   static const num_descr byte      = { "256", 0, 3, 3, 4, -1 } ;

   mat_tsd_t *mt;

   mt = (mat_tsd_t *)TSD->mat_tsd;

   ccns = ( num->exp < 3 ) ? 3 : num->exp;

   assert( num == &mt->edescr );

   /*
    * If the length is zero, a special case applies, the return value
    * is a nullstring.
    */
   if ( length == 0 )
      result = nullstringptr();

   /*
    * Here comes the real work. To ease the implementation it is
    * divided into two parts based on whether or not length is
    * specified.
    */
   else if ( length == -1 )
   {
      /*
       * First, let's estimate the size of the output string that
       * we need. A crude (over)estimate is one char for every second
       * decimal digits. Also set length, just to chache the value.
       * (btw: isn't that MAX( ,0) unneeded? Since number don't have
       * a decimal part, and since it must have a integer part (else
       * it would be zero, and then trapped above.)
       */
      assert( num->exp > 0 );
      result = Str_makeTSD( ( length = ( MAX( num->exp, 0 ) ) / 2 ) + 1 );

      /*
       * Let's loop from the least significant part of edescr. For each
       * iteration we divide num by 256, stopping when edescr is
       * zero.
       */
      for ( i = length; ; i-- )
      {
         /*
          * Perform the integer divition, edescr gets the quotient,
          * while fdescr get the remainder. Afterwards, perform some
          * makeup on the numbers (that might not be needed?)
          */
         string_div2( TSD, num, &byte, &mt->fdescr, num, DIVTYPE_BOTH, ccns );
         str_strip( num );
         str_strip( &mt->fdescr );

         /*
          * Now, fdescr has the remainder, stuff it into the result string
          * before it escapes :-) (don't we have to cast lvalue here?)
          * Afterwards, check to see if there are more digits to extract.
          */
         result->value[i] = (char) descr_to_int( TSD, &mt->fdescr, errnum, suberrnum, bif, argno );
         if ( ( num->num[0] == '0' ) && ( num->size == 1 ) )
            break;
      }

      /*
       * That's it, now we just have to align the answer and set the
       * correct length. Have to use memmove() since strings may
       * overlap.
       */
      memmove( result->value, &result->value[i], length + 1 - i );
      result->len = length + 1 - i;
   }
   else
   {
      /*
       * We do have a specified length for the number. At least that
       * makes it easy to deside how large the result string should be.
       */
      result = Str_makeTSD( length );

      /*
       * In the loop, iterate once for each divition of 256, but stop
       * only when we have reached the start of the result string.
       * Below, edescr gets the quotient and fdescr gets the remainder.
       */
      for ( i = length - 1; i >= 0; i-- )
      {
         /* may hang if acc==zero ? */
         string_div2( TSD, num, &byte, &mt->fdescr, num, DIVTYPE_BOTH, ccns );
         str_strip( num );
         str_strip( &mt->fdescr );

         /*
          * If the remainder is negative (i.e. quotient is negative too)
          * then add 256 to the remainder, to bring it into the range of
          * an unsigned char. To compensate for that, subtract one from
          * the quotient. Store the remainder.
          */
         if ( mt->fdescr.negative )
         {
            /* the following two lines are not needed, but it does not
               work without them. */
            if ( ( num->size == 1 ) && ( num->num[0] == '0' ) )
               num->exp = 1;

            string_add2( TSD, num, &minus_one, num, ccns );
            str_strip( num );
            string_add2( TSD, &mt->fdescr, &byte, &mt->fdescr, ccns );
         }
         result->value[i] = (char) descr_to_int( TSD, &mt->fdescr, errnum, suberrnum, bif, argno );
      }
      /*
       * That's it, store the length
       */
      result->len = length;
   }

   /*
    * We're finished ... hope it works ...
    */
   return result;
}


streng *str_normalize( const tsd_t *TSD, const streng *number )
{
   mat_tsd_t *mt;
   int err;

   mt = (mat_tsd_t *)TSD->mat_tsd;
   if ( ( err = getdescr( TSD, number, &mt->fdescr ) ) != 0 )
      exiterror( ERR_BAD_ARITHMETIC, 0 )  ;

   return str_norm( TSD, &mt->fdescr, NULL ) ;
}



num_descr *is_a_descr( const tsd_t *TSD, const streng *number )
{
   num_descr *newnum=NULL ;
   mat_tsd_t *mt;

   mt = (mat_tsd_t *)TSD->mat_tsd;

   if ( getdescr( TSD, number, &mt->fdescr ) != 0 )
      return NULL ;

   newnum = (num_descr *)MallocTSD( sizeof( num_descr )) ;
   newnum->max = 0 ;
   newnum->num = NULL ;

   descr_copy( TSD, &mt->fdescr, newnum ) ;
   return newnum ;
}

/*
 * ANSI chapter 7, beginning: "...matches that syntax and also has a value
 * that is 'whole', that is has no non-zero fractional part." The syntax
 * is that of a plain number.
 * Thus, 1E1 or 1.00 are allowed.
 * This function returns 1 if number is a valid whole number, 0 else.
 *
 * The value is loaded into mat_tsd_t.edescr. A pointer to this is
 * returned in *num.
 *
 * Added 08.03.2005 (tt.mm.yyyy), FGC: Due to some misinterpretation by my own
 * this routine must not round, instead it must check for ANSI WHOLENUM.
 * This means that the number must be representable without loss in the
 * interval [-(10**digits()-1), 10**digits()-1].
 * noDigitsLimit raises digits() virtually to endless. Don't use it in
 * ANSI compatible environments.
 */
int myiswnumber( tsd_t *TSD, const streng *number, num_descr **num,
                 int noDigitsLimit )
{
   num_descr *input;
   mat_tsd_t *mt;

   mt = (mat_tsd_t *)TSD->mat_tsd;

   if ( getdescr( TSD, number, &mt->edescr ) )
      return 0;
   input = &mt->edescr;
   if ( num != NULL )
      *num = input;

   return test_whole( TSD, input, noDigitsLimit );
}


/*
 * Converts number to an integer. Sets *error to 1 on error (0 otherwise)
 *
 * ANSI chapter 7, beginning: "...matches that syntax and also has a value
 * that is 'whole', that is has no non-zero fractional part." The syntax
 * is that of a plain number.
 * Thus, 1E1 or 1.00 are allowed.
 */
int streng_to_int( const tsd_t *TSD, const streng *number, int *error )
{
   int result = 0;
   mat_tsd_t *mt;

   mt = (mat_tsd_t *)TSD->mat_tsd;

   if ( ( *error = getdescr( TSD, number, &mt->fdescr ) ) != 0 )
      return 0;

   if ( ( *error = !whole_number( &mt->fdescr, &result ) ) != 0 )
      return 0;

   return result;
}

/*
 * Converts number to a 64bit integer. Sets *error to 1 on error (0 otherwise)
 *
 * ANSI chapter 7, beginning: "...matches that syntax and also has a value
 * that is 'whole', that is has no non-zero fractional part." The syntax
 * is that of a plain number.
 * Thus, 1E1 or 1.00 are allowed.
 */
rx_64 streng_to_rx64( const tsd_t *TSD, const streng *number, int *error )
{
   rx_64 result = 0;
   mat_tsd_t *mt;

   mt = (mat_tsd_t *)TSD->mat_tsd;

   if ( ( *error = getdescr( TSD, number, &mt->fdescr ) ) != 0 )
      return 0;

   if ( ( *error = !whole_rx64_number( &mt->fdescr, &result ) ) != 0 )
      return 0;

   return result;
}

int myisnumber( const tsd_t *TSD, const streng *string )
{
   mat_tsd_t *mt;

   mt = (mat_tsd_t *)TSD->mat_tsd;

   return getdescr( TSD, string, &mt->edescr ) == 0;
}

