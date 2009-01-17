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
 * This file implements the RxFuncDefine function. The direct useable
 * function is placed in gci_rexxbridge.c or similar.
 */

#include "gci.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
 * A small enum type describing the parsed additional arguments to "CALLTYPE".
 */
typedef enum {
   InfoUnknown = 0,
   InfoAs,
   InfoFunction,
   InfoWith,
   InfoParameters
} InfoType;

/*
 * checkname tries to interpret the first characters of a string and sets
 * the symbolic name of it.
 * The string is taken from *str with a length of *size.
 *
 * On success *str is set to the first character after the recognized word
 * and *size is decremented by the length of the recognized word. Additionally
 * *calltype and *infotype is set. Exactly one of them becomes "unknown".
 *
 * This function doesn't jump over leading whitespaces and the function
 * doesn't check for any word delimiters. *str must have been uppercased.
 */
static GCI_result checkname( const char **str,
                             int *size,
                             GCI_calltype *calltype,
                             InfoType *infotype )
{
   static const struct {
      GCI_calltype  ctype;
      InfoType      itype;
      int           length;
      const char   *name;
   } list[] = {
      { GCI_ctCdecl,   InfoUnknown,     5, "CDECL"      },
      { GCI_ctPascal,  InfoUnknown,     6, "PASCAL"     },
      { GCI_ctStdcall, InfoUnknown,     7, "STDCALL"    },
      { GCI_ctUnknown, InfoAs,          2, "AS"         },
      { GCI_ctUnknown, InfoFunction,    8, "FUNCTION"   },
      { GCI_ctUnknown, InfoWith,        4, "WITH"       },
      { GCI_ctUnknown, InfoParameters, 10, "PARAMETERS" }
   };
   const char *s = *str;
   int i, l, len = *size;

   for ( i = 0; i < (int) elements( list ); i++ )
   {
      l = list[i].length;
      if ( len < l )
         continue;
      if ( memcmp( s, list[i].name, l ) == 0 )
      {
         *str = s + l;
         *size -= l;
         *calltype = list[i].ctype;
         *infotype = list[i].itype;
         return GCI_OK;
      }
   }
   return GCI_UnsupportedType;
}

/*
 * parseCallTree parses the string str and puts all fetched informations into
 * *ci. The string str usually is the argument of stem.CALTYPE.
 *
 * Return values:
 * GCI_OK:              Everything is fine.
 * GCI_UnsupportedType: The base calltype isn't cdecl, stdcall, etc or the
 *                      modifiers (as function, etc) couldn't be recognized.
 */
static GCI_result parseCallType( void *hidden,
                                 const GCI_str *str,
                                 GCI_callinfo *ci )
{
   GCI_calltype ct;
   InfoType it, lastit;
   const char *ptr = GCI_ccontent( str );
   int size = GCI_strlen( str );

   (hidden = hidden);

   lastit = InfoUnknown;
   memset( ci, 0, sizeof( GCI_callinfo ) );

   /*
    * Allow any order as far as double word arguments like "as function"
    * appears together and in the right order.
    */
   for ( ; ; )
   {
      /*
       * Chop off spaces. We stop if we reach EOS here.
       */
      while ( ( size > 0 ) && GCI_isspace( *ptr ) )
      {
         ptr++;
         size--;
      }
      if ( size == 0 )
         break;

      if ( checkname( &ptr, &size, &ct, &it ) != GCI_OK )
         return GCI_UnsupportedType;
      if ( lastit == InfoAs )
      {
         if ( it != InfoFunction )
            return GCI_UnsupportedType;
         else
            {
               ci->as_function = 1;
               lastit = InfoUnknown;
               continue;
            }
      }
      if ( lastit == InfoWith )
      {
         if ( it != InfoParameters )
            return GCI_UnsupportedType;
         else
            {
               ci->with_parameters = 1;
               lastit = InfoUnknown;
               continue;
            }
      }

      if ( ci->type == GCI_ctUnknown )
      {
         if ( ct == GCI_ctUnknown )
            return GCI_UnsupportedType;
         else
            {
               ci->type = ct;
               continue;
            }
      }
      else
      {
         if ( ct != GCI_ctUnknown )
            return GCI_UnsupportedType;
      }

      switch ( it )
      {
         case InfoAs:
         case InfoWith:
            lastit = it;
            continue;

         default:
            break;
      }
      return GCI_UnsupportedType;
   }

   if ( ( ci->type == GCI_ctUnknown ) || ( lastit != InfoUnknown ) )
      return GCI_UnsupportedType;

   return GCI_OK;
}

/*
 * isempty returns 1 exactly if the passed string is either empty or consists
 * of whitespaces only. 0 is returned otherwise.
 */
static int isempty( void *hidden,
                    const GCI_str *str )
{
   const char *s = GCI_ccontent( str );
   int len = GCI_strlen( str );

   (hidden = hidden);

   while ( len > 0 ) {
      if ( !GCI_isspace( *s ) )
         return 0;
      len--;
      s++;
   }

   return 1;
}

/*
 * parseTree parses a complete argument stem of RxFuncDefine.
 * base must be the uppercased and otherwise prepared stem's name suitable
 * for non-symbolic access. base must have some space left, e.g. 200 byte, for
 * expanding all arguments. base's content isn't resetted on error which
 * provides a simple way to determine the errorneous variable.
 * *ti will be filled. *ti should be initialized. The caller must free the
 * nodes element if it isn't NULL on return and the return value signals an
 * error.
 *
 * prefixChar is the prefix that must be used in front of stem names.
 *
 * Return values:
 * GCI_OK:  Everything is fine.
 * other:   There are so many different errors, it doesn't make any sense
 *          to list them. Use the errorneous stem's name in base and the
 *          GCI_describe'd string of the return value.
 */
static GCI_result parseTree( void *hidden,
                             GCI_str *base,
                             GCI_treeinfo *ti,
                             const char *prefixChar )
{
   GCI_result rc;
   unsigned argc, return_valid = 0;
   int origlen = GCI_strlen( base );
   /*
    * All simple arguments must fit into a static buffer.
    */
   char tmp[256];
   GCI_strOfCharBuffer( tmp );

   GCI_strcats( base, "." );
   GCI_strcats( base, prefixChar );
   GCI_strcats( base, "CALLTYPE" );
   if ( ( rc = GCI_readRexx( hidden, base, &str_tmp, 0, 1, NULL ) ) != GCI_OK )
   {
      if ( rc == GCI_MissingValue )
         rc = GCI_MissingName;
      return rc;
   }
   GCI_uppercase( hidden, &str_tmp );

   if ( ( rc = parseCallType( hidden, &str_tmp, &ti->callinfo ) ) != GCI_OK )
      return rc;

   GCI_strsetlen( base, origlen );
   GCI_strcats( base, ".0" );
   if ( ( rc = GCI_readRexx( hidden, base, &str_tmp, 0, 1, NULL ) ) != GCI_OK )
   {
      if ( rc == GCI_MissingValue )
         rc = GCI_MissingName;
      return rc;
   }

   if ( GCI_string2bin( hidden,
                        GCI_ccontent( &str_tmp ),
                        GCI_strlen( &str_tmp ),
                        &argc,
                        sizeof( argc ),
                        GCI_unsigned ) != GCI_OK )
      return GCI_UnsupportedType;

   if ( argc > GCI_REXX_ARGS )
      return GCI_NumberRange;

   /*
    * We still need to know whether to provide a return value.
    */
   GCI_strsetlen( base, origlen );
   GCI_strcats( base, "." );
   GCI_strcats( base, prefixChar );
   GCI_strcats( base, "RETURN" );
   GCI_strcats( base, "." );
   GCI_strcats( base, prefixChar );
   GCI_strcats( base, "TYPE" );
   if ( ( rc = GCI_readRexx( hidden, base, &str_tmp, 0, 1, NULL ) ) != GCI_OK )
   {
      if ( rc == GCI_MissingValue )
         return GCI_MissingName;
   }
   else
   {
      if ( !isempty( hidden, &str_tmp ) )
      {
         /*
          * If we have to respect a return value, we have to pass back a
          * return values in a stem, but this cannot be done if we have
          * the with_parameters option in effect.
          * Auto-enable as_function as mentioned in the proposal.
          */
         if ( ti->callinfo.with_parameters )
            ti->callinfo.as_function = 1;
         return_valid = 1;
      }
      else if ( ti->callinfo.as_function ) /* senseless */
         return GCI_NoBaseType;
   }
   GCI_strsetlen( base, origlen );

   if ( rc != GCI_OK )
      return rc;

   return GCI_parsenodes( hidden, base, ti, argc, return_valid, prefixChar );
}

/*
 * GCI_ParseTree parses the stem of a function definition and checks its
 * validity.
 *
 * hidden is the usual parameter of the system; stem is the parameter of the
 * (GCI_)RxFuncDefine call.
 *
 * The validated tree is returned in gci_info.
 *
 * error_disposition is a non-allocated string on entry. It will contain the
 * error position in the stem in case of an error. It may be unset again to
 * indicate a non specific error location.
 *
 * prefixChar is the prefix that must be used in front of stem names.
 *
 * Return values:
 * GCI_OK:  Everything is fine.
 * other:   There are so many different errors, it doesn't make any sense
 *          to list them. Use the errorneous stem's name in error_disposition
 *          (if available) and the GCI_describe'd string of the return value.
 */
GCI_result GCI_ParseTree( void *hidden,
                          const GCI_str *stem,
                          GCI_treeinfo *gci_info,
                          GCI_str *error_disposition,
                          const char *prefixChar )
{
   GCI_result rc;
   GCI_str stemval;
   GCI_treeinfo tree;
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
                         error_disposition,
                         GCI_strlen( stem ) ) == GCI_OK )
         GCI_strcpy( error_disposition, stem );
      return rc;
   }

   /*
    * An extra buffer for roughly 100 indentions, that's enough.
    */
   if ( ( rc = GCI_stralloc( hidden,
                             error_disposition,
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
   GCI_strcpy( error_disposition, &stemval );
   GCI_strfree( hidden, &stemval );

   memset( &tree, 0, sizeof( tree ) );
   for ( l = 0; l < (int) elements( tree.args ); l++ )
      tree.args[l] = -1;
   tree.retval = -1;

   if ( ( rc = parseTree( hidden,
                          error_disposition,
                          &tree,
                          prefixChar ) ) != GCI_OK )
   {
      if ( tree.nodes != NULL )
         GCI_free( hidden, tree.nodes );
      return rc;
   }

   /*
    * We don't need the error disposition any longer.
    */
   GCI_strfree( hidden, error_disposition );

   *gci_info = tree;

   return GCI_OK;
}

/*
 * GCI_RxFuncDefine is the main executor or the RxFuncDefine function.
 * It checks the stem for validity. If successful it tries to register a new
 * function to the generic call executor. That function will effectively call
 * the desired function.
 *
 * hidden is the usual parameter of the system; internal, library, external
 * and stem are the parameters of the RxFuncDefine call.
 *
 * error_disposition is a non-allocated string on entry. It will contain the
 * error position in the stem in case of an error. It may be unset again to
 * indicate a non specific error location.
 *
 * prefixChar is the prefix that must be used in front of stem names.
 *
 * Return values:
 * GCI_OK:  Everything is fine.
 * other:   There are so many different errors, it doesn't make any sense
 *          to list them. Use the errorneous stem's name in error_disposition
 *          (if available) and the GCI_describe'd string of the return value.
 */
GCI_result GCI_RxFuncDefine( void *hidden,
                             const GCI_str *internal,
                             const GCI_str *library,
                             const GCI_str *external,
                             const GCI_str *stem,
                             GCI_str *error_disposition,
                             const char *prefixChar )
{
   GCI_result rc;
   GCI_treeinfo tree;

   if ( ( rc = GCI_ParseTree( hidden,
                              stem,
                              &tree,
                              error_disposition,
                              prefixChar ) ) != GCI_OK )
      return rc;

   if ( ( rc = GCI_RegisterDefinedFunction( hidden,
                                            internal,
                                            library,
                                            external,
                                            &tree ) ) != GCI_OK )
   {
      if ( tree.nodes != NULL )
         GCI_free( hidden, tree.nodes );
   }

   return rc;
}
