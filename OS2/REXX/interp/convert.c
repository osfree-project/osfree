#ifndef lint
static char *RCSid = "$Id: convert.c,v 1.12 2006/09/13 07:52:46 mark Exp $";
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
 * The functions in this file converts between the different
 * 'datatypes' that REXX uses, such as decimal number, character
 * string and hex string. Some of the functions are meant for the
 * standard library, like c2x, x2c, d2x, x2d, c2d and d2c, and also
 * the new functions in language level 4.00: b2x and x2b. The routines
 * in the file might also be used elsewhere for conversion between
 * char, decimal and hex.
 *
 * In general, there are two levels, the interface for the functions
 * in the builtin library (std_c2x, std_c2d, etc) and the low-level
 * routines that do the actual job. The interface routines 'only' call
 * the low-level routines in the right order, to preprocess parameters
 * and to postprocess the result. However, this implementation is not
 * 100% clean in this respect, since std_b2x() and std_x2b() are
 * selfcontained.
 *
 * When converting to or from decimal numbers, the decimal number will
 * trigger an overflow condition if more bits are pushed into the
 * number than the is allowed by the current setting of NUMERIC
 * DIGITS.
 *
 * The low-level routines are:
 *
 *    pack_hex()      ---  packs a hex string into a char string
 *    unpack_hex()    ---  unpacks a char string into a hex string.
 *    numberize()     ---  converts a char string to a whole number
 *
 * In addition, low level routine that operates on whole numbers are
 * placed in strmath.c. The relevant routines there which are called
 * from this file are (numberize() is only a frontend to the function
 * str_digitize()):
 *
 *    str_binerize()  ---  converts a whole number into a char string
 *    str_digitize()  ---  converts a char string into a whole number
 *
 * The high-level routines in this file are:
 *
 *    std_b2x()       ---  converts bin string to hex string
 *    std_c2d()       ---  converts char string to whole number
 *    std_c2x()       ---  converts char string to hex string
 *    std_d2c()       ---  converts whole number to char string
 *    std_d2x()       ---  converts whole number to hex string
 *    std_x2b()       ---  converts hex string to bin string
 *    std_x2c()       ---  converts hex string to char string
 *    std_x2d()       ---  converts hex string to whole number
 *
 *
 * There are four 'generic' dataformats used in this implementation:
 * bin string, hex string, char string and whole number.
 *
 * * CHAR STRING consists of chars (any of the 256, including 0). It
 *   can have any length (including 0). This is the 'normal' strings
 *   in Rexx, but note that in the context of this module, char strings
 *   will often contain unprintable characters, while in normal use
 *   char strings will often just contain printable characters.
 *   To emphsize the difference, it will sometimes be refered to as
 *   packed char string.
 *
 * * BIN STRING consists of the binary digits 0 and 1, and will be
 *   implicitly padded out to a nibble (4 bit) boundary. They may have
 *   any length, including 0. Optional spaces might be added between
 *   nibble boundaries, but not at the start or end of the string. The
 *   first group of bin digits may have any number of digits, and will
 *   be padded with leading zeros at the left to make it a multiple
 *   of four digits.
 *
 * * HEX STRING consists of the hex digits 0-9 and A-F (in upper or
 *   lower case). It may have any length (including 0).  Optional
 *   spaces might be added between pairs of digits to form groups of
 *   hex digits, but not at the start or end. If the first group does
 *   not contain an even number of hex digits, it is padded to the
 *   left with an zero.
 *
 * * WHOLE NUMBER must be a string which is a valid Rexx whole number,
 *   which is a subset of Rexx numbers. If it has a decimal part, that
 *   must be zero (i.e. 13.0 is a whole number). In addition, it must
 *   have a precition which is sufficient high to identify the number
 *   as a particular integer (i.e. 13E1 is not a whole number, since
 *   it really is 130 plus/minus 5).
 *
 * All these, except whole numbers, can have zero length, in which
 * case they are considered the nullstring.  When generating these
 * datatype for output, they will be normalized, which means:
 *
 * * For whole numbers, bin and hex string, no additional space is
 *   added; neither internal, leading nor trailing.
 *
 * * Whole numbers will not be in exponential form, and the decimal
 *   part of the number (if any) is truncated.
 *
 * * Hex string will use upper case alphanumberic characters in stead
 *   of lower case. Bin string will be padded with zeros at the left
 *   to a multiple of four.
 */


#include "rexx.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>


/*
 * The following string is used to convert a nibble value
 * into its hexadecimal character set representation. Perhaps this
 * should be placed somewhere else, e.g. in misc.c
 */
static const char hexnum[] = "0123456789ABCDEF" ;



/* ---------------------------------------------------------------
 * Takes a bin string as input, and returns a normalized hex
 * string representing the same data. The output will have an even
 * number of digits.
 *
 * Since we are reading byte values in the range 0-255, we must take
 * care to use unsigned chars, else we might get some surprises.
 * Therefore there are some casting in this routine.
 */
static streng *unpack_hex( const tsd_t *TSD, const streng *string )
{
   streng *result=NULL ;          /* the output string */
   char *res_ptr=NULL ;           /* ptr to current char in output string */
   unsigned char *ptr=NULL ;      /* ptr to input string */
   unsigned char *end_ptr=NULL ;  /* ptr to end+1 of input string */

   /*
    * Allocate the needed space, which is very easy to calculate
    */
   result =  Str_makeTSD( Str_len( string )*2 ) ;
   res_ptr = result->value ;

   /*
    * Initialize loop ptr and the end ptr, and loop through each
    * character in the input string. Write two output hex digits
    * for each char in the input string
    */
   end_ptr = (unsigned char *)(Str_end(string)) ;
   ptr = (unsigned char *)(string->value) ;
   for (; ptr<end_ptr; ptr++ )
   {
      *res_ptr++ = hexnum[ (*ptr>>4) ] ;
      *res_ptr++ = hexnum[ (*ptr & 0x0f) ] ;
   }

   /*
    * That's it, set the length and return to caller
    */
   result->len = ( res_ptr - result->value ) ;
   assert( result->len <= result->max ) ;
   return result ;
}



/* -----------------------------------------------------------------
 * Input is a hex string, which is converted to a char string
 * representing the same information and returned.
 *
 * We have to concider the optional grouping of hex digits by spaces at
 * byte boundaries, and the possibility of having to pad first group
 * with a zero.
 *
 * There is one performance problem with this. If the hex string is
 * normalized and long, we have to loop through the string twice, while
 * once would suffice. To determine whether or not to pad first group
 * with a zero, all of first group must be scanned, which is identical
 * to the whole string if it is normalized.
 */
static streng *pack_hex( tsd_t *TSD, const char *bif, const streng *string )
{
   streng *result=NULL ;      /* output char string */
   const char *ptr=NULL ;     /* current digit in input hex string */
   const char *end_ptr=NULL ; /* ptr to end+1 in input hex string */
   char *res_ptr=NULL ;       /* ptr to current char in output string */
   int byte_boundary=0 ;      /* boolean, are we at at byte bounary? */
   int count;                 /* used to count positions */
   int last_blank=0;          /* used to report last byte errors */

   /*
    * Allow one extra char for padding, ignore that allocated string
    * might be too long if there is spacing at byte boundaries.
    */
   result = Str_makeTSD( (Str_len(string)+1)/2 +1 ) ;
   res_ptr = result->value ;

   /*
    * Initiate pointers to current char in intput string, and to
    * end+1 in input string.
    */
   ptr = string->value ;
   end_ptr = Str_end(string) ;

   /*
    * Explicitly check for space at start or end. Illegal space within
    * the hex string is checked for during the loop.
    */
   if ((ptr<end_ptr) && ((rx_isspace(*ptr)) || (rx_isspace(*(end_ptr-1)))))
   {
      goto invalid;
   }

   /*
    * Find the number of hex digits in the first group of hex digits.
    * Let the variable 'byte_boundary' be a boolean, indicating if
    * current char might be a byte boundary. I.e if byte_boundary is
    * set, spaces are legal.
    *
    * Also, set the first byte in the output string. That is not
    * necessary if the first group of hex digits has an even number of
    * digits, but it is cheaper to do it always that check for it.
    */
   for (; (ptr<end_ptr) && (rx_isxdigit(*ptr)); ptr++ ) ;
   byte_boundary = !((ptr-string->value)%2) ;

/* Does this statement do anything useful? (Proabably, things crash if
   I remove it ...
 */
   *res_ptr = 0x00 ;

   /*
    * Loop through the elements of the input string. Skip over spaces.
    * Stuff hex digits into the output string, and report error
    * for any other type of data.
    */
   for (count=1,ptr=string->value; ptr<end_ptr; ptr++, count++)
   {
      if (rx_isspace(*ptr))
      {
         /*
          * Just make sure that this space occurs at a byte boundary,
          * except from that, ignore it.
          */
         last_blank = count;
         if (!byte_boundary)
         {
            goto invalid;
         }
      }
      else if (rx_isxdigit(*ptr))
      {
         /*
          * Stuff it into the output array, either as upper or lower
          * part of a byte, depending on the value of 'byte_boundary'.
          * Then toggle the value of 'byte_boundary'.
          */
         if (byte_boundary)
            *res_ptr = (char)( HEXVAL(*ptr) << 4 ) ;
         else
            {
               /* Damn'ed MSVC: */
               *res_ptr = (char) (*res_ptr + (char) (HEXVAL(*ptr))) ;
               res_ptr++;
            }

         byte_boundary = !byte_boundary ;
      }
      else
      {
         goto invalid;
      }
   }

   /*
    * Set the length and do 'redundant' check for problems. In
    * particular, check 'byte_boundary' to verify that the last group
    * of hex digits ended at a byte boundary; report error if not.
    */
   if (!byte_boundary)
   {
      goto invalid;
   }

   result->len = res_ptr - result->value ;
   assert( result->len <= result->max ) ;

   return result ;

invalid:
   Free_stringTSD( result );
   exiterror( ERR_INCORRECT_CALL, 25, bif, tmpstr_of( TSD, string ) );
   return NULL; /* not reached */
}



/* ------------------------------------------------------------------
 * Takes a char string input and concerts it into a whole number of
 * base 10, which can be signed. Actually, the real work is done in
 * str_digitize().
 *
 * If length is -1 (i.e. unspecified) it will be interpreted as an
 * unsigned integer.
 *
 * If length is specified, the input string will be interpreted as an
 * two's complement number having that length. If parameter 'length'
 * is bigger than the length of parameter string, 'string' is
 * logically extended with '0' at the left. If parameter length is
 * smaller than the length of parameter string, only the rightmost
 * characters of string is significant.
 *
 * The output string will not contain any leading zeros (unless the
 * value of the number is zero, in which case '0' is returned). A '0'
 * will also be returned if 'length' is zero, or if 'string' is the
 * nullstring.
 */
static streng *numerize( tsd_t *TSD, streng *string, int length,
                         const char *bif, int removeStringOnError )
{
   int start=0 ;       /* character to start reading at */
   int sign=0 ;        /* is this to be interpreted as signed? */

   /* The trivial case, either the nullstring, or length=0 */
   if ((length==0) || (Str_len(string)==0))
      return int_to_streng( TSD, 0 ) ;

   /*
    * Set the variable 'start' to the most significant byte in 'string'.
    * That is the first byte if 'length' is either unspecified or
    * bigger than (or equal to) the length of 'string'.
    *
    * If 'length' is specified and is less than the length of 'string',
    * then set 'start' to the the 'length'th byte, counted backward.
    */
   if ((length==-1) || (length>Str_len(string)))
      start = sign = 0 ;
   else
   {
      assert((length>0) && (length<=Str_len(string))) ;
      start = Str_len(string) - length ;
      sign = 1 ;
      assert((start>=0) && (start<Str_len(string))) ;
   }

   /*
    * Call the correct routine in the string module. The number will
    * always be signed if length is specified.
    */
   return str_digitize( TSD, string, start, sign, bif, removeStringOnError ) ;
}



/* ------------------------------------------------------------------
 * Converts a hex string to a decimal number using two's complement.
 * This is a high level routine, which just calls the above low level
 * routines to do the job. First the hex string is converted into
 * a char string, and then the char string is converted into a whole
 * number.
 */
streng *std_x2d( tsd_t *TSD, cparamboxptr parms )
{
   int length=0 ;         /* the length of the input hex string */
   streng *result=NULL ;  /* the output string */
   streng *packed=NULL ;  /* tmp variable holding the char string */

   /*
    * First read the parameters, and set length to -1 (meaning
    * unspecified) if the second parameter was not specified.
    */
   checkparam(  parms,  1,  2 , "X2D" ) ;
   if ((parms->next)&&(parms->next->value))
      length = atozpos( TSD, parms->next->value, "X2D", 2 ) ;
   else
      length = (-1) ;

   /*
    * Convert the hex string into a whole number in two steps
    *
    * Note that the 'length' variable is the length in hex digits, and
    * that numerize receives its input as a char string, so we have to
    * convert the 'length' a bit. Also, that means that we have to
    * sign extend the number at the left to a byte boundary.
    */
   packed = pack_hex( TSD, "X2D", parms->value ) ;
   if ((length>0) && (length%2))
   {
      /*
       * The char string was padded with an extra zero nibble in pack_hex()
       * so we must signextend that nibble. 'msb' is a tmp variable
       * that points to the most significant byte in packed. Hmmm this
       * is a kludge ...
       */
      int msb = Str_len(packed)-(length/2)-1 ;
      if (msb >= 0)   /* only if length <= hexchars supplied */
      {
         if (packed->value[msb] & 0x08)
            packed->value[msb] |= 0xf0 ;
         else
            packed->value[msb] &= 0x0f ;
      }
   }
   result = numerize( TSD,
                      packed,
                      ((length!=-1) ? ((length+1)/2) : -1),
                      "X2D",
                      1 ) ;

   /*
    * Clean up and return to caller
    */
   Free_stringTSD( packed ) ;

   return result ;
}



/* ------------------------------------------------------------------
 * Converts a char string to a hex string. This a just a box around
 * one of the low level routines, that processes the parameters.
 */
streng *std_x2c( tsd_t *TSD, cparamboxptr parms )
{
   checkparam(  parms,  1,  1 , "X2C" ) ;
   return pack_hex( TSD, "X2C", parms->value ) ;
}



/* ------------------------------------------------------------------
 * Builtin function that converts a binary string to a normalized
 * hexstring. The hexstring will be padded to *nibble* boundary,
 * (note: not byte boundary).
 *
 * This function should have been implemented as a convertion from
 * bin string to packed binary string, and then a call to unpack
 * the binary string to a hex string. Unfortunately, accuracy would
 * be lost, since packed binary strings are padded to byte boundary.
 *
 * Therefore, this function converts directly into a hex string. If
 * more functions are added to Rexx (like b2d() and b2c()) in the
 * future, it might be more effective to create a nibble format (or
 * change the packed binary string to nibble array), in order to
 * decrease the number of lines of code.
 *
 * The code for this is mostly taken from lexsrc.l, these two pieces
 * should perhaps be tuned to use the same function.
 */

streng *std_b2x( tsd_t *TSD, cparamboxptr parms )
{
   char *ptr=NULL ;        /* loop variable */
   char *endptr=NULL ;     /* pointer to end+1 of input string */
   char *res_ptr=NULL ;    /* pointer to result string */
   streng *result=NULL ;   /* result string */
   streng *string=NULL ;   /* tmp variable to avoid pointer chasing */
   int first_group=0 ;     /* number of bin digits in first bin group */
   int cur_bit=0 ;         /* current bit in input string */
   int nibble=0 ;          /* collects bin digits to nibbles */

   /*
    * Have we been called correctly?
    */
   checkparam(  parms,  1,  1 , "B2X" ) ;
   string = parms->value ;

   /*
    * Since a bin string can have any number of digits in its first
    * group, we have to find the number of digits, in order to be able
    * to pad that group with leading zeros. The number can also be
    * zero (i.e. the bin string is empty.) The number first_group
    * contains the number of binary digits in the first group.
    */
   endptr = Str_end(string) ;
   ptr = string->value ;
   for (; (ptr<endptr) && (((*ptr)=='0') || ((*ptr)=='1')); ptr++ ) ;
   first_group = ptr - string->value ;

   /*
    * If the first group contained zero binary digits, then either does
    * it contain leading space, or it is the nullstring. The former is
    * an error, so report it if that is the case.
    */
   if (Str_len(string) && ((first_group==0) || (rx_isspace(*(endptr-1)))))
   {
      /* fixes 1107969 */
      exiterror( ERR_INCORRECT_CALL, 24, "B2X", tmpstr_of( TSD, string ) );
   }

   /*
    * If the string is a proper bin string, we need one hex digit for
    * each fourth bin digit (after having taken into account that the
    * first group might have a maximum of three implied zeros.) If it
    * is not a proper bin string, the error will be caught later.
    *
    * Actually, this might be more than we need, since there might be
    * spaces embedded within the bin string, but don't bother about
    * that since it just takes CPU time to figure out exactly how many
    * bytes we need. Just allocate enough.
    */
   result = Str_makeTSD( (Str_len(string)+3)/4 ) ;
   res_ptr = result->value ;

   /*
    * Initialize some variables, 'cur_bit' is the current bit within
    * the current nibble. It must be initialized to the number of
    * implied leading zeros in the bin string. The following
    * transformation from 'frist_group%4' to 'cur_bit' has the following
    * mapping {0,1,2,3}->{0,3,2,1}.
    */
   cur_bit = (4 - first_group%4)%4 ;
   nibble = 0 ;

   /*
    * Then, loop for each character in the input string, and perform
    * some action, based on whether it is a space or a binary digit.
    * If it is neither, report an error.
    */
   for (ptr=string->value; ptr<endptr; ptr++)
   {
      if (rx_isspace(*ptr))
      {
         /*
          * The variable 'cur_bit' is a number containing the relative
          * position of the current bit within the current group of
          * binary digits. After reading a complete nibble, it reaches
          * 4, and is reset to 0. So if this is a nibble boundary, it
          * better be 0, or else there is space within a nibble.
          */
         if (cur_bit!=0)
         {
            Free_stringTSD( result );
            exiterror( ERR_INCORRECT_CALL, 24, "B2X", tmpstr_of( TSD, string ) );
         }
      }

      else if (((*ptr)=='0')||((*ptr)=='1'))
      {
         /*
          * If it is a binary digit, shift 'nibble' and add the digit.
          * If 'cur_bit' (after being incremented) shows 4 (i.e we have
          * completed processing a nibble), reset it to 0, and and
          * flush 'nibble' to the result string and reset 'nibble' too.
          */
         nibble = nibble * 2 + ((*ptr)-'0') ;
         if ((++cur_bit)==4)
         {
            *(res_ptr++) = hexnum[nibble] ;
            nibble = 0 ;
            cur_bit = 0 ;
         }
      }
      else
      {
         exiterror( ERR_INCORRECT_CALL, 24, "B2X", tmpstr_of( TSD, string ) );
      }
   }

   /*
    * Wow, we're finished, we just have to set the length of 'result'
    */
   result->len = (res_ptr - result->value ) ;
   assert( result->len <= result->max ) ;
   return( result ) ;
}




/* ------------------------------------------------------------------
 * Function that converts a hex string to a binary string. The hex
 * string may have spaces at byte boundaries as usual, but the first
 * group of hex digits may have an odd number of digits, in which
 * case it is *not* padded with an zero hex digit.
 *
 * Just like std_b2x(), this function should really use a common data
 * format, but it does not matter before more rexx gets more functions
 * that converts to/from bin strings.
 */

streng *std_x2b( tsd_t *TSD, cparamboxptr parms )
{
   int space_stat=0 ;      /* state machine: nibble or byte boundary? */
   char *ptr=NULL ;        /* loop control variable */
   char *end_ptr=NULL ;    /* points to end+1 of input, endcondition in loop */
   streng *result=NULL ;   /* the output streng */
   char *res_ptr=NULL ;    /* ptr to contents of 'result' */
   int nibble=0 ;          /* holds a nibble while extracting bin digits */
   int count=0 ;           /* loop control variable */
   int pos=0 ;             /* position in string for error reporting */

   /*
    * Check that we got the parameters that we needed. Then initialize
    * some of the variables.
    */
   checkparam(  parms,  1,  1 ,"X2B" ) ;
   ptr = parms->value->value ;
   end_ptr = Str_end( parms->value ) ;
   space_stat = 0 ;

   /*
    * Let us allocate enough space, we could tune some space here
    * if we actually checked how many hex digits that the input
    * string contiained.
    */
   result = Str_makeTSD( (end_ptr-ptr) * 4 ) ;
   res_ptr = result->value ;

   /*
    * Check for leading or trailing space in the hex string.
    */
   if (end_ptr>ptr)
   {
      if (rx_isspace(*ptr) || rx_isspace(*(end_ptr-1)))
      {
         goto invalid;
      }
   }

   /*
    * The main loop. For each hex digit, output four bin digits to
    * the output string, and check for illegal spaces within bytes.
    * If anything other than spaces or hex digits are found, report
    * an error.
    */
   for (pos=1; ptr<end_ptr; ptr++,pos++)
   {
      if (rx_isspace(*ptr))
      {
         /*
          * We have found space in the hex string, eat it up, and keep
          * the statemachine 'space_state' going. If state is 0 (end
          * of first group) go to state 2 (at byte boundary). If state
          * is 1 (inside a byte) report an error, since space may not
          * occur there.
          */
         if (space_stat==0)
         {
            space_stat = 2 ;
         }
         else if (space_stat==1)
         {
            goto invalid;
         }
      }
      else if (rx_isxdigit(*ptr))
      {
         /*
          * We have found a hex digit, chop it into four parts, and
          * stuff them into 'result'. Requires that the character set
          * value of '1' is one higher than that of '0'.
          */
         nibble = HEXVAL( *ptr ) ;
         for (count=0; count<4; count++)
         {
            *(res_ptr++) = (char)(( (nibble & 0x08) != 0 ) + '0') ;
            nibble <<= 1 ;
         }

         /*
          * Remember to toggle the statemachine between states 1 and 2
          * so we can keep track of byte and nibble boundaries. If in
          * state 0 (whitin first group), stay there.
          */
         if (space_stat)
           space_stat = ((space_stat==1) ? 2 : 1) ;
      }
      else
      {
         goto invalid;
      }
   }

   /*
    * Set the length, and get out of here.
    */
   result->len = res_ptr - result->value ;
   return result ;

invalid:
   Free_stringTSD( result );
   exiterror( ERR_INCORRECT_CALL, 25, "X2B", tmpstr_of( TSD, parms->value ) );
   return NULL; /* not reached */
}




/* --------------------------------------------------------------------
 * Converts a char string to a decimal string. Really just a box around
 * numerize, that only preprocesses the parameters.
 */
streng *std_c2d( tsd_t *TSD, cparamboxptr parms )
{
   int length ;   /* The length of the input char string */

   checkparam(  parms,  1,  2 , "C2D" ) ;
   if ((parms->next)&&(parms->next->value))
      length = atozpos( TSD, parms->next->value, "C2D", 2 ) ;
   else
      length = -1 ;

   return numerize( TSD, parms->value, length, "C2D", 0 ) ;
}




/* ---------------------------------------------------------------------
 * Converts a packed binary string to a hexadecimal string
 */
streng *std_c2x( tsd_t *TSD, cparamboxptr parms )
{
   checkparam(  parms,  1,  1 , "C2X" ) ;
   return unpack_hex( TSD, parms->value ) ;
}

static void check_wholenum( tsd_t *TSD, const char *bif, const streng *arg,
                            num_descr **num )
{
   if ( !myiswnumber( TSD, arg, num,
                      !get_options_flag( TSD->currlevel, EXT_STRICT_ANSI ) ) )
      exiterror( ERR_INCORRECT_CALL, 12, bif, 1, tmpstr_of( TSD, arg ) );
}

/* ---------------------------------------------------------------------
 * Converts a whole number into char string. This is just a wrapper
 * around str_binerize(), which preprocesses the parameters.
 */
streng *std_d2c( tsd_t *TSD, cparamboxptr parms )
{
   int length;  /* the length of the output string */
   num_descr *num;

   checkparam( parms,  1,  2 , "D2C" );

   check_wholenum( TSD, "D2C", parms->value, &num );
   if ( parms->next && parms->next->value )
      length = atozpos( TSD, parms->next->value, "D2C", 2 );
   else
   {
      /*
       * The strange syntax forces a check for non-negative first arg if the
       * second doesn't exist.
       */
      if ( num->negative )
         exiterror( ERR_INCORRECT_CALL, 13, "D2C", 1,
                    tmpstr_of( TSD, parms->value ) );

      length = -1;
   }

   return str_binerize( TSD, num, length );
}


/* ------------------------------------------------------------------
 * Converts a decimal string into a hexadecimal string, using char
 * string as an intermediate format. Due to the use of intermediate
 * format, an extra hex digit might slip into the left end of the
 * answer, and code is added to remove it.
 */
streng *std_d2x( tsd_t *TSD, cparamboxptr parms )
{
   int length;         /* holds the requested langth of the result */
   streng *result;     /* the output streng */
   streng *packed;     /* tmp variable, holds the packed string */
   num_descr *num;

   /*
    * Check the parameters, and set 'length' to the specified length, or
    * to -1 if the second parameter was not specified.
    */
   checkparam(  parms,  1,  2 , "D2X" );

   check_wholenum( TSD, "D2X", parms->value, &num );
   if ( parms->next && parms->next->value )
      length = atozpos( TSD, parms->next->value, "D2X", 2 );
   else
   {
      /*
       * The strange syntax forces a check for non-negative first arg if the
       * second doesn't exist.
       */
      if ( num->negative )
         exiterror( ERR_INCORRECT_CALL, 13, "D2X", 1,
                    tmpstr_of( TSD, parms->value ) );

      length = -1;
   }

   /*
    * Convert the whole number into a hex string in a two step operation.
    * First it is converted into a char string, and then that char string
    * is converted into a hexstring.
    */
   packed = str_binerize( TSD, num, ( length == -1 ) ? -1 : ( length+1 ) / 2 );
   result = unpack_hex( TSD, packed );
   Free_stringTSD( packed );

   /*
    * Since we used char string as a temporary format, the hex string
    * will now be padded with one extra zero at the left. If we specified
    * length, and that length does not match the actual length, we must
    * strip away the first zero in 'result'.
    *
    * Here we check for length>0, since we want to catch it if length was
    * specified, but not if it was 0. In the latter case, the string will
    * be the nullstring, and we don't need to do anything anyway.
    */
   if ( ( length > 0 ) && ( Str_len( result ) != length ) && Str_len( result ) )
   {
      assert( Str_len( result ) == length + 1 );
      memmove( result->value, &result->value[1], --result->len );
   }

   /*
    * Just to be safe, check that we did get the nullstring if length
    * was specified to 0
    */
   assert( ( length != 0 ) || ( Str_len(result) == 0 ) );

   /*
    * If length was not specified, there might be leading zeros in the
    * answer that we might want to get rid of. However, the algoritm
    * that is used in str_binerize() should ensure that there will not
    * be more than one leading zero in this case. This part of the code
    * could easily be merged with the other call to memmove() above,
    * but have been placed here to improve readability.
    *
    * If there are more than one leading zero, this will not work. The
    * situation where the result only consists of one zero, should not
    * occur, but it will be handled, since the result should then be
    * the nullstring.
    */
   if ( ( length == -1 ) && ( result->value[0] == '0' ) )
   {
      assert( Str_len( result ) > 1 );
      assert( ( result->value[0] == '0') && ( ( result->value[1] != '0' ) || ( Str_len( result ) == 2 ) ) );

      memmove( result->value, &result->value[1], --result->len );
   }

   /*
    * That's it, now we just have to get out of here
    */
   return result;
}
