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
 * This file contains the code for preparing a function call. The call itself
 * is done in gci_call.c.
 */

#include "gci.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/*
 * For some recursive iterations we need several informations. We put them
 * together for convenience und to keep the stack's usage small.
 * Note that the elements will be shared over recursive calls. It will be
 * a good idea to be careful when changing.
 * The elements are:
 * hidden:  As usual, for the interpreter's usage.
 * base:    The name or the current variable. Will not change after an error.
 * nodes:   All nodes we can use or address.
 * basebuf: Where to put stuff. It has a size of max while filling.
 * tmp:     256 byte buffer for number reading.
 * strbuf:  A hollow string used with GCI_strfromascii for strings.
 * dshift:  Current shift which must be added while addressing direct targets.
 *          This value will be modified while accessing arrays and
 *          indirections.
 * ishift:  Current shift which must be added while addressing indirect
 *          targets. This value will be modified while accessing arrays and
 *          indirections.
 * max:     Number of bytes in basebuf while filling basebuf.
 * novalue: Flag if the loaded value was a default while filling basebuf.
 * count:   Value for checking indirect array and container VALUE parameters.
 * helper:  buffer for the textual iterator.
 * prefix:  prefix character for names.
 */
typedef struct {
   void               *hidden;
   GCI_str            *base;
   const GCI_nodeinfo *nodes;
   char               *basebuf;
   GCI_str            *tmp;
   GCI_str             strbuf;
   unsigned            dshift;
   unsigned            ishift;
   unsigned            max;
   int                 novalue;
   unsigned            count;
   char                helper[80];
   const char         *prefix;
} reader;

/*
 * We need another structure for writing operations.
 * Note that the elements will be shared over recursive calls. It will be
 * a good idea to be careful when changing.
 * The elements are:
 * hidden:  As usual, for the interpreter's usage.
 * base:    The name or the current variable. Will not change after an error.
 * nodes:   All nodes we can use or address.
 * tmp:     256 byte buffer for number reading.
 * size:    used during a conversion.
 * strbuf:  A hollow string used with GCI_strfromascii for strings.
 * drop:    NULL-string for dropping a variable.
 * helper:  buffer for the textual iterator.
 * prefix:  prefix character for names.
 */
typedef struct {
   void               *hidden;
   GCI_str            *base;
   const GCI_nodeinfo *nodes;
   GCI_str            *tmp;
   int                 size;
   GCI_str             strbuf;
   GCI_str             drop;
   char                helper[80];
   const char         *prefix;
} writer;

/*
 * validArgs return 1 if and only if the GCI_REXX_ARGS arguments in args (which
 * are counted as argc arguments) fit the requirements of the function call
 * which are described in ci/ti. 0 is returned otherwise.
 */
static int validArgs( int argc,
                      const GCI_str *args,
                      const GCI_callinfo *ci,
                      const GCI_treeinfo *ti )
{
   int i, lastarg = 0;

   if ( ci->with_parameters )
   {
      /*
       * We allow missing arguments exactly for values which may be indirect.
       * We don't allow more arguments even if they are virtually missing as
       * in func(,,,,,).
       */
      if ( argc > GCI_REXX_ARGS )
         return 0;
      for ( i = 0; i < GCI_REXX_ARGS; i++ )
      {
         if ( ti->args[i] == -1 )
         {
            if ( GCI_ccontent( &args[i] ) != NULL )
               return 0;
            break;
         }
         else
         {
            if ( ( GCI_ccontent( &args[i] ) == NULL ) &&
                 !ti->nodes[ti->args[i]].type.indirect )
               return 0;
            lastarg = i + 1;
         }
      }
      if ( argc > lastarg )
         return 0;
   }
   else
   {
      if ( ( argc != 1 ) || ( GCI_ccontent( &args[0] ) == NULL ) )
         return 0;
   }

   return 1;
}

/*
 * assignArgumentsFromParameters assigns all arguments of the calling function
 * to the buffer which shall hold the binary values.
 *
 * args contains n arguments which may have a content of NULL. n is the number
 * of needed arguments.
 *
 * ti contains all informations we need to parse and assign the arguments.
 *
 * basebuf is the buffer we need for all arguments to pass to GCI_call.
 *
 * error_disposition must be a string big enough to hold 8 characters.
 * It will reflect the error position in case of an error.
 *
 * Return values:
 * GCI_OK:              Everything is fine.
 *
 *                      In case of an error error_disposition will contain the
 *                      arguments's name where the problem raises first.
 *
 * GCI_UnsupportedType: Wrong type of input, e.g. FLOAT31 or the empty string
 *                      in a type description string. Another reason is an
 *                      internal error since the default sizes for "unsigned"
 *                      and "integer" are not supported.
 * GCI_WrongInput:      Strange characters occur in the input string as a
 *                      number.
 * GCI_NumberRange:     Number to small or big to fit into the desired type
 *                      with the desired destbyte-size. This applies to the
 *                      different number type.
 * GCI_StringRange:     String too big for the defined buffer.
 * GCI_InternalError:   Unexpected type for the base type.
 */
static GCI_result assignArgumentsFromParameters( void *hidden,
                                                 const GCI_str *args,
                                                 const GCI_treeinfo *ti,
                                                 char *basebuf,
                                                 GCI_str *error_disposition )
{
   GCI_result rc;
   int i, start;
   unsigned pos;
   GCI_nodeinfo *n;
   GCI_parseinfo *info;
   char tmp[20];


   for ( i = 0; i < GCI_REXX_ARGS; i++ )
   {
      if ( ( start = ti->args[i] ) == -1 )
         break;
      GCI_strsetlen( error_disposition, 0 );
      sprintf( tmp, "arg(%d)", i + 1 );
      GCI_strcats( error_disposition, tmp );

      n = ti->nodes + start;
      info = &n->type;
      if ( GCI_ccontent( &args[i] ) == NULL )
      {
         if ( !info->indirect )
            return GCI_InternalError; /* should have been      */
                                      /* detected by validArgs */
         /* pointer already set to NULL */
         continue;
      }

      if ( info->indirect )
      {
         *((void **) (basebuf + n->direct_pos)) = basebuf + n->indirect_pos;
         pos = n->indirect_pos;
      }
      else
         pos = n->direct_pos;

      switch ( info->type )
      {
         case GCI_integer:
         case GCI_unsigned:
         case GCI_float:
         case GCI_char:
            if ( ( rc = GCI_string2bin( hidden,
                                        GCI_ccontent( &args[i] ),
                                        GCI_strlen( &args[i] ),
                                        basebuf + pos,
                                        info->size,
                                        info->type ) ) != GCI_OK )
               return rc;
            break;

         case GCI_string:
            if ( (unsigned) GCI_strlen( &args[i] ) > info->size )
               return GCI_StringRange;
            memcpy( basebuf + pos,
                    GCI_ccontent( &args[i] ),
                    GCI_strlen( &args[i] ) );
            basebuf[pos + GCI_strlen( &args[i] )] = '\0';
            break;

         case GCI_raw:
            if ( (unsigned) GCI_strlen( &args[i] ) > info->size )
               return GCI_StringRange;
            memcpy( basebuf + pos,
                    GCI_ccontent( &args[i] ),
                    GCI_strlen( &args[i] ) );
            memset( basebuf + pos + GCI_strlen( &args[i] ),
                    0,
                    info->size - GCI_strlen( &args[i] ) );
            break;

         default:
            return GCI_InternalError;
      }
   }
   return GCI_OK;
}

/*
 * readString reads in one string which is described in *info into the
 * rdr->basebuf at a given address which base address is basepos.
 * If novalueBreakout is set, the buffer is resetted to 0 if the variable
 * which name is taken from rdr->base is not registered by the interpreter.
 * GCI_OK is returned in this case, this state can be determined by a setted
 * rdr->novalue.
 * If novalueBreakout is not set, even a unregistered variable are accepted and
 * the default value is assigned.
 * If notAString is set, the buffer is treated as a buffer, not as a string.
 *
 * GCI_OK:              Everything is fine.
 *
 *                      In case of an error base will contain the
 *                      stem's name where the problem raises first.
 *
 * GCI_WrongInput:      Strange characters occur in the input string as a
 *                      number.
 * GCI_StringRange:     String too big for the defined buffer.
 * GCI_InternalError:   Unexpected type for the base type.
 * GCI_RexxError:       Strange return value by the interpreter.
 */
static GCI_result readString( reader *rdr,
                              const GCI_parseinfo *info,
                              unsigned basepos,
                              int novalueBreakout,
                              int notAString )
{
   GCI_result rc;
   int size = (notAString) ? info->size : info->size + 1;

   /*
    * We have to support a terminating zero in the buffer of the
    * interpreter. Some bloody fools decided to add a hidden terminating
    * zero at the end and now we have to allow the interpreter to
    * return a correct value in this case. Of course we have to check
    * for an overused buffer afterwards.
    */
   GCI_strfromascii( &rdr->strbuf, rdr->basebuf + basepos, size );
   assert( basepos + size <= rdr->max );
   rdr->novalue = 0;
   rc = GCI_readRexx( rdr->hidden,
                      rdr->base,
                      &rdr->strbuf,
                      0,
                      !novalueBreakout,
                      &rdr->novalue );
   if ( rdr->novalue && novalueBreakout )
   {
      memset( rdr->basebuf + basepos, 0, size );
      return GCI_OK;
   }
   switch ( rc )
   {
      case GCI_OK:
         break;

      case GCI_BufferTooSmall:
         return GCI_StringRange;

      default:
         return rc;
   }
   if ( (unsigned) GCI_strlen( &rdr->strbuf ) > info->size )
      return GCI_StringRange;
   if ( notAString )
   {
      memset( rdr->basebuf + basepos + GCI_strlen( &rdr->strbuf ),
              0,
              size - GCI_strlen( &rdr->strbuf ) );
   }
   else
   {
      rdr->basebuf[basepos + GCI_strlen( &rdr->strbuf )] = '\0';
   }

   return GCI_OK;
}

/*
 * readValue reads in one string which is described in *info, converts it, and
 * puts it into the rdr->basebuf at a given address which base address is
 * basepos.
 * If novalueBreakout is set, it indicates a bot registered variable by the
 * interpreter. GCI_OK is returned in this case, this state can be determined
 * by a setted rdr->novalue.
 * If novalueBreakout is not set, even a unregistered variable are accepted and
 * the default value is used for conversion and assignment.
 *
 * GCI_OK:              Everything is fine.
 *
 *                      In case of an error base will contain the
 *                      stem's name where the problem raises first.
 *
 * GCI_WrongInput:      Strange characters occur in the input string as a
 *                      number.
 * GCI_NumberRange:     Number to small or big to fit into the desired type
 *                      with the desired destbyte-size. This applies to the
 *                      different number type.
 * GCI_InternalError:   Unexpected type for the base type.
 * GCI_RexxError:       Strange return value by the interpreter.
 */
static GCI_result readValue( reader *rdr,
                             const GCI_parseinfo *info,
                             unsigned basepos,
                             int novalueBreakout )
{
   GCI_result rc;

   rdr->novalue = 0;
   rc = GCI_readRexx( rdr->hidden,
                      rdr->base,
                      rdr->tmp,
                      0,
                      !novalueBreakout,
                      &rdr->novalue );
   if ( rdr->novalue && novalueBreakout )
      return GCI_OK;

   if ( rc != GCI_OK )
      return rc;

   switch ( info->type )
   {
      case GCI_integer:
      case GCI_unsigned:
      case GCI_float:
      case GCI_char:
         assert( basepos + info->size <= rdr->max );
         return GCI_string2bin( rdr->hidden,
                                GCI_ccontent( rdr->tmp ),
                                GCI_strlen( rdr->tmp ),
                                rdr->basebuf + basepos,
                                info->size,
                                info->type );

      default:
         break;
   }
   return GCI_InternalError;
}

/*
 * readTree assigns all node values of a given tree recursively to the buffer
 * which shall hold the binary values.
 *
 * reader contains all variables useful for all or several leafs.
 *
 * n is the starting node within reader->nodes.
 *
 * Return values:
 * GCI_OK:              Everything is fine.
 *
 *                      In case of an error base will contain the
 *                      stem's name where the problem raises first.
 *
 * GCI_UnsupportedType: Wrong type of input, e.g. FLOAT31 or the empty string
 *                      in a type description string. Another reason is an
 *                      internal error since the default sizes for "unsigned"
 *                      and "integer" are not supported.
 * GCI_WrongInput:      Strange characters occur in the input string as a
 *                      number.
 * GCI_NumberRange:     Number to small or big to fit into the desired type
 *                      with the desired destbyte-size. This applies to the
 *                      different number type.
 *                      Another reason is a wrong value for indirected arrays
 *                      or containers.
 * GCI_StringRange:     String too big for the defined buffer.
 * GCI_InternalError:   Unexpected type for the base type.
 * GCI_RexxError:       Strange return value by the interpreter.
 */
static GCI_result readTree( reader *rdr,
                            const GCI_nodeinfo *n )
{
   GCI_result rc;
   const GCI_parseinfo *info;
   unsigned i;
   unsigned shift = 0;
   int origlen = GCI_strlen( rdr->base );

   info = &n->type;

   GCI_strcats( rdr->base, "." );
   GCI_strcats( rdr->base, rdr->prefix );
   if ( ( rc = GCI_strcats( rdr->base, "VALUE" ) ) != GCI_OK )
      return rc;
   if ( info->indirect )
   {
      /*
       * This allows missing values. We have to try loading a value for arrays
       * and containers but should read strings directly to the target place to
       * prevent a slow double buffering. We assume to have values in most
       * cases.
       */
      switch ( info->type )
      {
         case GCI_integer:
         case GCI_unsigned:
         case GCI_float:
         case GCI_char:
            if ( ( rc = readValue( rdr,
                                   info,
                                   n->indirect_pos + rdr->ishift,
                                   1 ) ) != GCI_OK )
               return rc;
            if ( rdr->novalue )
            {
               GCI_strsetlen( rdr->base, origlen );
               return GCI_OK;
            }
            break;

         case GCI_string:
         case GCI_raw:
            if ( ( rc = readString( rdr,
                                    info,
                                    n->indirect_pos + rdr->ishift,
                                    1,
                                    ( info->type == GCI_raw ) ) ) != GCI_OK )
               return rc;
            if ( rdr->novalue )
            {
               memset( rdr->basebuf + n->indirect_pos + rdr->ishift,
                       0,
                       info->size + 1 );
               GCI_strsetlen( rdr->base, origlen );
               return GCI_OK;
            }
            break;

         case GCI_container:
            rdr->novalue = 0;
            rc = GCI_readRexx( rdr->hidden,
                               rdr->base,
                               rdr->tmp,
                               0,
                               0,
                               &rdr->novalue );
            if ( rdr->novalue )
            {
               GCI_strsetlen( rdr->base, origlen );
               return GCI_OK;
            }
            if ( rc != GCI_OK )
               return rc;
            if ( ( rc = GCI_string2bin( rdr->hidden,
                                        GCI_ccontent( rdr->tmp ),
                                        GCI_strlen( rdr->tmp ),
                                        &rdr->count,
                                        sizeof( unsigned ),
                                        GCI_unsigned ) ) != GCI_OK )
               return rc;
            if ( info->generated )
            {
               if ( rdr->count != rdr->nodes[n->child].type.size )
                  return GCI_NumberRange;
            }
            else
            {
               if ( rdr->count != info->size )
                  return GCI_NumberRange;
            }
            break;

         case GCI_array:
         default:
            return GCI_InternalError;
      }

      assert( n->direct_pos + rdr->dshift + sizeof( void* ) <= rdr->max );
      *((void **) (rdr->basebuf + n->direct_pos + rdr->dshift)) =
                                  rdr->basebuf + n->indirect_pos + rdr->ishift;
   }
   else
   {
      /*
       * The direct case, this is somewhat simpler, but we have do make special
       * considerations for GCI_string, too.
       */
      switch ( info->type )
      {
         case GCI_integer:
         case GCI_unsigned:
         case GCI_float:
         case GCI_char:
            if ( ( rc = readValue( rdr,
                                   info,
                                   n->direct_pos + rdr->dshift,
                                   0 ) ) != GCI_OK )
               return rc;
            break;

         case GCI_string:
         case GCI_raw:
            if ( ( rc = readString( rdr,
                                    info,
                                    n->direct_pos + rdr->dshift,
                                    0,
                                    ( info->type == GCI_raw ) ) ) != GCI_OK )
               return rc;
            break;

         case GCI_container:
         case GCI_array:
            break;

         default:
            return GCI_InternalError;
      }
   }

   GCI_strsetlen( rdr->base, origlen );

   switch ( info->type )
   {
      case GCI_integer:
      case GCI_unsigned:
      case GCI_float:
      case GCI_char:
      case GCI_string:
      case GCI_raw:
         return GCI_OK;

      case GCI_container:
         if ( info->indirect )
         {
            shift = rdr->dshift;
            rdr->dshift = rdr->ishift;
            rdr->ishift = 0;
         }
         n = rdr->nodes + n->child;
         for ( i = 0; i < info->size; i++ )
         {
            assert( n >= rdr->nodes );
            if ( info->generated )
            {
               assert( info->size == 1 );
               assert( n->type.type == GCI_array );
            }
            else
            {
               sprintf( rdr->helper, ".%u", i + 1 );
               if ( ( rc = GCI_strcats( rdr->base, rdr->helper ) ) != GCI_OK )
                  return rc;
            }

            if ( ( rc = readTree( rdr, n ) ) != GCI_OK )
               return rc;

            GCI_strsetlen( rdr->base, origlen );
            n = rdr->nodes + n->sibling;
         }
         if ( info->indirect )
         {
            rdr->ishift = rdr->dshift;
            rdr->dshift = shift;
         }
         return GCI_OK;

      case GCI_array:
         assert( info->indirect == 0 );
         assert( n->child >= 0 );
         assert( ( n->direct_size % info->size ) == 0 );
         assert( ( n->indirect_size % info->size ) == 0);
         for ( i = 0; i < info->size; i++ )
         {
            sprintf( rdr->helper, ".%u", i + 1 );
            if ( ( rc = GCI_strcats( rdr->base, rdr->helper ) ) != GCI_OK )
               return rc;

            if ( ( rc = readTree( rdr, rdr->nodes + n->child ) ) != GCI_OK )
               return rc;

            GCI_strsetlen( rdr->base, origlen );
            rdr->dshift += n->direct_size / info->size;
            rdr->ishift += n->indirect_size / info->size;
         }
         rdr->dshift -= n->direct_size;
         rdr->ishift -= n->indirect_size;
         return GCI_OK;

      default:
         break;
   }
   return GCI_InternalError;
}

/*
 * loadStem loads the name of a stem and prepares its usage.
 *
 * stem is the stem's base name passed to the dispatcher.
 *
 * base will be set to the stem's value will an additional space of STEMDEPTH
 * on success. This value will be uppercased and without a leading dot.
 *
 * Return values:
 * GCI_OK:           Everything is fine.
 *
 *                   In case of an error base may contain the variable's name.
 * GCI_NoMemory:     The system is out of memory.
 * GCI_IllegalName:  "name" is illegal in terms of Rexx.
 * GCI_RexxError:    An unexpected other error is returned by the
 *                   interpreter.
 */
static GCI_result loadStem( void *hidden,
                            const GCI_str *stem,
                            GCI_str *base )
{
   GCI_result rc;
   GCI_str stemval;
   int l;

   /*
    * Allow novalue.
    */
   if ( ( rc = GCI_readNewRexx( hidden,
                                stem,
                                &stemval,
                                1,
                                0,
                                NULL ) ) != GCI_OK )
   {
      /*
       * Let the system show a proper error location.
       */
      if ( GCI_stralloc( hidden,
                         base,
                         GCI_strlen( stem ) ) == GCI_OK )
         GCI_strcpy( base, stem );
      return rc;
   }

   /*
    * An extra buffer for roughly 100 indentions, that's enough.
    */
   if ( ( rc = GCI_stralloc( hidden,
                             base,
                             GCI_strlen( &stemval ) + STEMDEPTH ) ) != GCI_OK )
   {
      GCI_strfree( hidden, &stemval );
      return rc;
   }
   /*
    * We do a non-symbolic access later. Make sure we don't loose!
    */
   GCI_uppercase( hidden, &stemval );
   l = GCI_strlen( &stemval );
   if ( ( l > 0 ) & ( GCI_content( &stemval )[l - 1] == '.' ) )
      GCI_strsetlen( &stemval, l - 1 );
   GCI_strcpy( base, &stemval );
   GCI_strfree( hidden, &stemval );

   return GCI_OK;
}

/*
 * assignArgumentsFromStem assigns all values of the first argument of the
 * calling function to the buffer which shall hold the binary values.
 *
 * base must be the uppercased and otherwise prepared stem's name suitable
 * for non-symbolic access. base must have some space left, e.g. 200 byte, for
 * expanding all arguments. base's content isn't resetted on error which
 * provides a simple way to determine the errorneous variable.
 *
 * ti contains all informations we need to parse and assign the arguments.
 *
 * basebuf is the buffer we need for all arguments to pass to GCI_call.
 *
 * prefixChar is the prefix that must be used in front of stem names.
 *
 * Return values:
 * GCI_OK:              Everything is fine.
 *
 *                      In case of an error base will contain the
 *                      stem's name where the problem raises first.
 *
 * GCI_UnsupportedType: Wrong type of input, e.g. FLOAT31 or the empty string
 *                      in a type description string. Another reason is an
 *                      internal error since the default sizes for "unsigned"
 *                      and "integer" are not supported.
 * GCI_WrongInput:      Strange characters occur in the input string as a
 *                      number.
 * GCI_NumberRange:     Number to small or big to fit into the desired type
 *                      with the desired destbyte-size. This applies to the
 *                      different number type.
 * GCI_StringRange:     String too big for the defined buffer.
 * GCI_InternalError:   Unexpected type for the base type.
 * GCI_RexxError:       An unexpected other error is returned by the
 *                      interpreter.
 */
static GCI_result assignArgumentsFromStem( void *hidden,
                                           GCI_str *base,
                                           const GCI_treeinfo *ti,
                                           char *basebuf,
                                           const char *prefixChar )
{
   GCI_result rc;
   int i, start;
   int origlen = GCI_strlen( base );
   char iter[3];
   reader rdr;
   /*
    * All simple arguments must fit into a static buffer.
    */
   char tmp[256];
   GCI_strOfCharBuffer( tmp );

   rdr.hidden =  hidden;
   rdr.base =    base;
   rdr.nodes =   ti->nodes;
   rdr.basebuf = basebuf;
   rdr.tmp =     &str_tmp;
   rdr.dshift =  0;
   rdr.ishift =  0;
   rdr.max =     ti->size;
   rdr.prefix =  prefixChar;

   for ( i = 0; i < GCI_REXX_ARGS; i++ )
   {
      GCI_strsetlen( base, origlen );
      if ( ( start = ti->args[i] ) == -1 )
         break;
      sprintf( iter, ".%d", i+1 );
      GCI_strcats( base, iter );
      if ( ( rc = readTree( &rdr, ti->nodes + start ) ) != GCI_OK )
         return rc;
   }

   GCI_strsetlen( base, origlen );
   return GCI_OK;
}

#ifdef DEBUG
/*
 * dump dumps the complete buffer of the arguments to stdout if the
 * environment variable "_GCI_DUMP" is set.
 */
static void dump( void *hidden,
                  const void *buf,
                  unsigned size )
{
   const unsigned char *b = (const unsigned char *) buf;
   unsigned i, j;

   (hidden = hidden);

   if ( getenv("_GCI_DUMP") == NULL )
      return;

   printf( "Buffer with all arguments, base buffer address = %p\n", b );
   for ( i = 0; i < size; i += 16 )
   {
      printf( "%8X: ", i );
      for ( j = i; ( j < i + 16 ) && ( j < size ); j++ )
         printf( " %02X", b[j] );
      printf( "%*s", ( i + 16 - j ) * 3 + 2, "");
      for ( j = i; ( j < i + 16 ) && ( j < size ); j++ )
         printf( "%c", GCI_isprint( b[j] ) ? b[j] : '.' );
      printf( "\n" );
   }
}
#else
# define dump(hidden,x,size)
#endif

/*
 * dropStem kills every variable of a stem's branch.
 * We kill descendants too.
 *
 * Return values:
 * GCI_OK:             Everything is fine.
 *
 *                     In case of an error wrt->base will contain the
 *                     stem's name where the problem raises first.
 *
 * GCI_NoMemory:       Can't allocate enough memory for the return value.
 * GCI_IllegalName:    "name" is illegal in terms of Rexx. Especially on
 *                     non-"symbolicAccess" the caller must provide uppercased
 *                     stem names if a stem is used.
 * GCI_RexxError:      An unexpected other error is returned by the
 *                     interpreter.
 * GCI_InternalError:  Attempt to use an invalid type.
 */
static GCI_result dropStem( writer *wrt,
                            const GCI_nodeinfo *n )
{
   GCI_result rc;
   int origlen = GCI_strlen( wrt->base );
   unsigned i;
   const GCI_parseinfo *info = &n->type;

   switch ( info->type )
   {
      case GCI_container:
         if ( info->generated )
         {
            assert( n->child != -1 );
            n = wrt->nodes + n->child;
            assert( n->sibling == -1 );
            return dropStem( wrt, n );
         }
         /*
          * fall through
          */

      case GCI_array:
         assert( n->child != -1 );
         n = wrt->nodes + n->child;
         for ( i = 0; i < info->size; i++ )
         {
            sprintf( wrt->helper, ".%d", i+1 );
            if ( ( rc = GCI_strcats( wrt->base, wrt->helper ) ) != GCI_OK )
               return rc;

            if ( ( rc = dropStem( wrt, n ) ) != GCI_OK )
               return rc;
            GCI_strsetlen( wrt->base, origlen );

            if ( info->type == GCI_container )
               n = wrt->nodes + n->sibling;
         }
         /*
          * fall through
          */


      case GCI_integer:
      case GCI_unsigned:
      case GCI_float:
      case GCI_char:
      case GCI_string:
      case GCI_raw:
         GCI_strcats( wrt->base, "." );
         GCI_strcats( wrt->base, wrt->prefix );
         if ( ( rc = GCI_strcats( wrt->base, "VALUE" ) ) != GCI_OK )
            return rc;
         if ( ( rc = GCI_writeRexx( wrt->hidden,
                                    wrt->base,
                                    &wrt->drop,
                                    0 ) ) != GCI_OK )
            return rc;
         GCI_strsetlen( wrt->base, origlen );
         return GCI_OK;

      default:
         return GCI_InternalError;
   }

   return GCI_OK;
}

/*
 * fillStem sets a stem's branch to the values taken from the buffer.
 * We follow every pointer, even if they don't point to the buffer any longer.
 *
 * Each value is converted or copied and the resulting string is placed into
 * the stem within the interpreter.
 *
 * Return values:
 * GCI_OK:                Everything is fine.
 *
 *                        In case of an error wrt->base will contain the
 *                        stem's name where the problem raises first.
 *
 * GCI_UnsupportedNumber: The floating point value in base is not a number
 *                        which can be displayed by digits (e.g. NaN).
 * GCI_BufferTooSmall:    The target buffer size strsize is less than 128.
 * GCI_UnsupportedType:   The combination of size/type is unknown or not
 *                        supported.
 * GCI_InternalError:     Attempt to use an invalid type.
 */
static GCI_result fillStem( writer *wrt,
                            const GCI_nodeinfo *n,
                            const char *direct )
{
   GCI_result rc;
   const GCI_parseinfo *info = &n->type;
   int origlen = GCI_strlen( wrt->base );
   unsigned i;

   assert( direct != NULL );

   if ( info->indirect )
   {
      direct = *((const char **) direct);
      if ( direct == NULL )
         return dropStem( wrt, n );
   }

   switch ( info->type )
   {
      case GCI_container:
         if ( info->generated )
         {
            assert( n->child != -1 );
            n = wrt->nodes + n->child;
            assert( n->sibling == -1 );
            assert( n->type.type == GCI_array );
            return fillStem( wrt, n, direct );
         }
         break;

      case GCI_array:
         break;

      case GCI_integer:
      case GCI_unsigned:
      case GCI_float:
      case GCI_char:
         wrt->size = GCI_strmax( wrt->tmp );
         if ( ( rc = GCI_bin2string( wrt->hidden,
                                     direct,
                                     info->size,
                                     GCI_content( wrt->tmp ),
                                     &wrt->size,
                                     info->type ) ) != GCI_OK )
            return rc;
         GCI_strsetlen( wrt->tmp, wrt->size );
         GCI_strcats( wrt->base, "." );
         GCI_strcats( wrt->base, wrt->prefix );
         if ( ( rc = GCI_strcats( wrt->base, "VALUE" ) ) != GCI_OK )
            return rc;
         if ( ( rc = GCI_writeRexx( wrt->hidden,
                                    wrt->base,
                                    wrt->tmp,
                                    0 ) ) != GCI_OK )
            return rc;
         GCI_strsetlen( wrt->base, origlen );
         return GCI_OK;

      case GCI_string:
      case GCI_raw:
         GCI_strcats( wrt->base, "." );
         GCI_strcats( wrt->base, wrt->prefix );
         if ( ( rc = GCI_strcats( wrt->base, "VALUE" ) ) != GCI_OK )
            return rc;
         if ( info->type == GCI_string )
         {
            GCI_migratefromascii( &wrt->strbuf, direct );
         }
         else
         {
            GCI_strfromascii( &wrt->strbuf, (char *) direct, info->size );
            GCI_strsetlen( &wrt->strbuf, info->size );
         }
         if ( ( rc = GCI_writeRexx( wrt->hidden,
                                    wrt->base,
                                    &wrt->strbuf,
                                    0 ) ) != GCI_OK )
            return rc;
         GCI_strsetlen( wrt->base, origlen );
         return GCI_OK;

      default:
         return GCI_InternalError;
   }

   /*
    * We have to set ".VALUE" for arrays and containers.
    */
   GCI_strsetlen( wrt->tmp,
                  sprintf( GCI_content( wrt->tmp), "%d", info->size ) );
   GCI_strcats( wrt->base, "." );
   GCI_strcats( wrt->base, wrt->prefix );
   if ( ( rc = GCI_strcats( wrt->base, "VALUE" ) ) != GCI_OK )
      return rc;
   if ( ( rc = GCI_writeRexx( wrt->hidden,
                              wrt->base,
                              wrt->tmp,
                              0 ) ) != GCI_OK )
      return rc;
   GCI_strsetlen( wrt->base, origlen );

   /*
    * Now iterate over every element of the container or array.
    */
   assert( n->child != -1 );
   n = wrt->nodes + n->child;
   for ( i = 0; i < info->size; i++ )
   {
      sprintf( wrt->helper, ".%d", i+1 );
      if ( ( rc = GCI_strcats( wrt->base, wrt->helper ) ) != GCI_OK )
         return rc;

      if ( ( rc = fillStem( wrt, n, direct ) ) != GCI_OK )
         return rc;
      GCI_strsetlen( wrt->base, origlen );

      direct += n->direct_size;
      if ( info->type == GCI_container )
         n = wrt->nodes + n->sibling;
   }

   return GCI_OK;
}

/*
 * setArgumentToStem is a wrapper for fillstem. Look there.
 */
static GCI_result setArgumentToStem( void *hidden,
                                     GCI_str *base,
                                     const GCI_treeinfo *ti,
                                     int startingIndex,
                                     const char *basebuf,
                                     const char *prefixChar )
{
   writer wrt;
   char tmp[256];
   const GCI_nodeinfo *n;
   GCI_strOfCharBuffer( tmp );

   memset( &wrt, 0, sizeof( wrt ) );
   wrt.hidden = hidden;
   wrt.base = base;
   wrt.nodes = ti->nodes;
   wrt.tmp = &str_tmp;
   wrt.prefix = prefixChar;

   n = ti->nodes + startingIndex;

   return fillStem( &wrt, n, basebuf + n->direct_pos );
}

/*
 * setArgumentsFromStem will set all indirected arguments back to the stem
 * after execution of the call.
 *
 * See setArgumentToStem for a better explanation.
 */
static GCI_result setArgumentsToStem( void *hidden,
                                      GCI_str *base,
                                      const GCI_treeinfo *ti,
                                      const char *basebuf,
                                      const char *prefixChar )
{
   GCI_result rc;
   int i;
   int origlen = GCI_strlen( base );
   char iter[3];
   GCI_str iterstr;

   for ( i = 0; i < GCI_REXX_ARGS; i++ )
   {
      if ( ti->args[i] == -1 )
         break;
      if ( !ti->nodes[ti->args[i]].type.indirect )
         continue;

      sprintf( iter, ".%d", i+1 );
      if ( ( rc = GCI_strcats( base, iter ) ) != GCI_OK )
         return rc;
      if ( ( rc = setArgumentToStem( hidden,
                                     base,
                                     ti,
                                     ti->args[i],
                                     basebuf,
                                     prefixChar ) ) != GCI_OK )
         return rc;
      GCI_strsetlen( base, origlen );
   }

   /*
    * Finally set the number of parameters.
    */
   if ( ( rc = GCI_strcats( base, ".0" ) ) != GCI_OK )
      return rc;
   sprintf( iter, "%d", i );
   if ( ( rc = GCI_writeRexx( hidden,
                              base,
                              GCI_migratefromascii( &iterstr, iter ),
                              0 ) ) != GCI_OK )
      return rc;
   GCI_strsetlen( base, origlen );

   return GCI_OK;
}

/*
 * setFunctionReturn sets the return value of the called function. The value
 * is taken from the return value in the basebuf or the pointer to which
 * it is pointing to.
 *
 * Nothing is done if the target is an indirection and the pointer of the
 * indirection is NULL. Otherwise the value is converted or copied and
 * the resulting string is placed into the content of retval, which must be
 * an unallocated buffer.
 *
 * Return values:
 * GCI_OK:                Everything is fine.
 *
 *                        In case of an error error_disposition will contain
 *                        the value "RESULT".
 *
 * GCI_UnsupportedNumber: The floating point value in base is not a number
 *                        which can be displayed by digits (e.g. NaN).
 * GCI_BufferTooSmall:    The target buffer size strsize is less than 128.
 * GCI_UnsupportedType:   The combination of size/type is unknown or not
 *                        supported.
 * GCI_InternalError:     Attempt to use an invalid type.
 */
static GCI_result setFunctionReturn( void *hidden,
                                     GCI_str *error_disposition,
                                     const GCI_treeinfo *ti,
                                     const char *basebuf,
                                     GCI_str *retval )
{
   GCI_result rc;
   const GCI_nodeinfo *n;
   const char *p;
   int size;
   char tmp[256];

   GCI_strsetlen( error_disposition, 0 );
   GCI_strcats( error_disposition, "RESULT" );

   assert( ti->retval >= 0 );
   n = ti->nodes + ti->retval;
   assert( n->child == -1 );

   p = basebuf + n->direct_pos;
   if ( n->type.indirect )
   {
      p = *((const char **) p);
      if ( p == NULL )
      {
         GCI_strsetlen( error_disposition, 0 );
         return GCI_OK;
      }
   }
   switch ( n->type.type )
   {
      case GCI_integer:
      case GCI_unsigned:
      case GCI_float:
      case GCI_char:
         size = sizeof( tmp );
         if ( ( rc = GCI_bin2string( hidden,
                                     p,
                                     n->type.size,
                                     tmp,
                                     &size,
                                     n->type.type ) ) != GCI_OK )
            return rc;
         p = tmp;
         break;

      case GCI_string:
         size = strlen( p );
         break;

      case GCI_raw:
         size = n->type.size;
         break;

      default:
         return GCI_InternalError;
   }

   if ( ( rc = GCI_stralloc( hidden, retval, size ) ) != GCI_OK )
      return rc;

   memcpy( GCI_content( retval ), p, size );
   GCI_strsetlen( retval, size );

   GCI_strsetlen( error_disposition, 0 );
   return GCI_OK;
}

/*****************************************************************************
 *****************************************************************************
 ** GLOBAL FUNCTIONS *********************************************************
 *****************************************************************************
 *****************************************************************************/

/*
 * GCI_execute parses arguments to the function according to the formerly
 * parsed tree information and reads in all relevant arguments values.
 * After this, the function is executed and all values which have to be written
 * back are written back.
 *
 *
 * func is the function pointer of the function, *ti contains all informations
 * the user had used to describe the arguments.
 *
 * argc contains the number of passed arguments in args. These contain the
 * arguments which content may be NULL. args must have GCI_REXX_ARGS entries,
 * the unused entries must contain NULL.
 *                                     *
 * error_disposition is a non-allocated string on entry. It will contain the
 * error position in the stem in case of an error. It may be unset again to
 * indicate a non specific error location.
 *
 * retval is a non-allocated string on entry. It will contain the value
 * in case of a "as function" call.
 *
 * prefixChar is the prefix that must be used in front of stem names.
 *
 * The string base may contain the error location within the stem in case of
 * an error.
 *
 * Return values:
 * GCI_OK:                Everything is fine.
 *
 *                        In case of an error error_disposition will contain
 *                        the variable's name where the problem raises first.
 *
 * GCI_MissingValue:      A variable's value isn't set and the expected value
 *                        was't an INDIRECT value. The system may or may not
 *                        raise a NOVALUE condition instead depending on the
 *                        implementation.
 * GCI_BufferTooSmall:    The variable's name buffer can't hold the complete
 *                        variable's name or the type string exceeds 256 byte
 *                        for non-string types.
 * GCI_IllegalName:       The variables name in cb->buffer is illegal in terms
 *                        of Rexx. In general, one of the strings in args is
 *                        wrong.
 * GCI_RexxError:         An unexpected error is returned by the interpreter
 *                        while trying to access Rexx variables.
 * GCI_UnsupportedType:   Wrong type of input, e.g. FLOAT31 or the empty string
 *                        in a type description string. Another reason is an
 *                        internal error since the default sizes for "unsigned"
 *                        and "integer" are not supported.
 * GCI_WrongInput:        Strange characters occur in the input string as a
 *                        number.
 * GCI_NumberRange:       Number to small or big to fit into the desired type
 *                        with the desired destbyte-size. This applies to the
 *                        different number type.
 * GCI_SyntaxError:       The number of arguments is wrong or the return value
 *                        became NULL.
 * GCI_InternalError:     Attempt to shorten the array so far that elements get
 *                        lost or illegal values used internally.
 * GCI_RexxError:         An unexpected other error is returned by the
 *                        interpreter.
 * GCI_UnsupportedNumber: The floating point value in base is not a number
 *                        which can be displayed by digits (e.g. NaN).
 */
GCI_result GCI_execute( void *hidden,
                        void (*func)(),
                        const GCI_treeinfo *ti,
                        int argc,
                        const GCI_str *args,
                        GCI_str *error_disposition,
                        GCI_str *retval,
                        const char *prefixChar )
{
   GCI_result rc;
   const GCI_callinfo *ci = &ti->callinfo;
   char *basebuf;

   /*
    * First check the argument number, the arguments and their content against
    * NULL.
    */
   if ( !validArgs( argc, args, ci, ti ) )
      return GCI_SyntaxError;

   /*
    * We're ready to read in all the values.
    */
   if ( ( basebuf = (char *) GCI_malloc( hidden, ti->size ) ) == NULL )
      return GCI_NoMemory;
   memset( basebuf, 0, ti->size );

   if ( ci->with_parameters )
   {
      /*
       * Since we either don't have to return anything or we are invoked
       * "as function", we don't need a big buffer.
       */
      if ( ( rc = GCI_stralloc( hidden, error_disposition, 20 ) ) == GCI_OK )
         rc = assignArgumentsFromParameters( hidden,
                                             args,
                                             ti,
                                             basebuf,
                                             error_disposition );
   }
   else
   {
      if ( ( rc = loadStem( hidden, args, error_disposition ) ) == GCI_OK )
         rc = assignArgumentsFromStem( hidden,
                                       error_disposition,
                                       ti,
                                       basebuf,
                                       prefixChar );
   }
   if ( rc != GCI_OK )
   {
      GCI_free( hidden, basebuf );
      return rc;
   }
   dump( hidden, basebuf, ti->size );

   rc = GCI_call( hidden, func, ti, basebuf );
   if ( rc != GCI_OK )
   {
      GCI_strfree( hidden, error_disposition );
      return rc;
   }
   dump( hidden, basebuf, ti->size );

   /*
    * Assign back all arguments and return values.
    */
   if ( !ci->with_parameters )
   {
      /*
       * Set values back exactly if INDIRECT is set.
       */
      if ( ( rc = setArgumentsToStem( hidden,
                                      error_disposition,
                                      ti,
                                      basebuf,
                                      prefixChar ) ) != GCI_OK )
         return rc;
   }

   if ( ci->as_function )
   {
      /*
       * Convert back a return value as a return value.
       */
      if ( ( rc = setFunctionReturn( hidden,
                                     error_disposition,
                                     ti,
                                     basebuf,
                                     retval ) ) != GCI_OK )
         return rc;
   }
   else
   {
      /*
       * We may have to set stem's .RETURN
       */
      if ( ti->retval != -1 )
      {
         assert( !ci->with_parameters );
         GCI_strcats( error_disposition, "." );
         GCI_strcats( error_disposition, prefixChar );
         if ( ( rc = GCI_strcats( error_disposition, "RETURN" ) ) != GCI_OK )
            return rc;

         if ( ( rc = setArgumentToStem( hidden,
                                        error_disposition,
                                        ti,
                                        ti->retval,
                                        basebuf,
                                        prefixChar ) ) != GCI_OK )
            return rc;
      }
   }

   GCI_strfree( hidden, error_disposition );
   return GCI_OK;
}
