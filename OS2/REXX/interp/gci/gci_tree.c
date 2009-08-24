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
 * This file parses an input tree for GCI usage. Note that the value's tree
 * isn't parsed. It is accessed directly after checking the generated
 * internal tree.
 */

#include "gci.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
 * We don't want to pass n+1 elements on each recursive call. We put all
 * needed things together and pass it in one chunk.
 * Most elements are copied from GCI_parsetree near the file's end. Look
 * there.
 */
typedef struct {
   void           *hidden;
   GCI_str        *buffer;     /* The name of the current variable. */
   int             depth;      /* starting with 0                   */
   GCI_result    (*callback)(int depth,
                             int itemnumber,
                             void *arg,
                             const GCI_parseinfo *info);
   void           *arg;
   GCI_str         tempbuf;  /* buffer for the content of each structure line*/
   char            helper[80]; /* buffer for the textual iterator   */
   int             recurCount; /* counts recursions in LIKE */
   const char     *prefixChar; /* must be used in front of stem names */
} callblock;

/*
 * checkname tries to interpret the first characters of a string and returns
 * the symbolic name of it.
 * The string is taken from *str with a length of *size.
 *
 * On success (return value not GCI_unknown) *str is set to the first
 * character after the recognized word and *size is decremented by the
 * length of the recognized word.
 *
 * This function doesn't jump over leading whitespaces and the function
 * doesn't check for any word delimiters. *str must have been uppercased.
 */
static GCI_basetype checkname( const char **str,
                               int *size )
{
   static const struct {
      GCI_basetype  type;
      int           length;
      const char   *name;
   } list[] = {
      { GCI_array,     5, "ARRAY"     },
      { GCI_char,      4, "CHAR"      },
      { GCI_container, 9, "CONTAINER" },
      { GCI_float,     5, "FLOAT"     },
      { GCI_indirect,  8, "INDIRECT"  },
      { GCI_like,      4, "LIKE"      },
      { GCI_integer,   7, "INTEGER"   },
      { GCI_raw,       3, "RAW",      },
      { GCI_string,    6, "STRING"    },
      { GCI_unsigned,  8, "UNSIGNED"  }
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
         return list[i].type;
      }
   }
   return GCI_unknown;
}

/*
 * decode decodes one line of the type structure named str into the
 * a parseinfo block called pi. depth is the current depth which may be 0.
 *
 * newName is set in case of ARRAY (to the empty string) or CONTAINER. In this
 * case is set to the LIKE parameter or to the empty string.
 *
 * Leading and trailing spaces are ignored completely, "INDIRECTFLOAT32" is
 * acceptable. Differences in case are ignored.
 *
 * THE GENERATED TYPE MAY HAVE AN ILLEGAL BIT SIZE. IT ISN'T CHECKED ALWAYS!
 *
 * Return values:
 * GCI_OK:              Line understood, *pi filled.
 * GCI_UnsupportedType: Wrong type of input, e.g. FLOAT31 or the empty string.
 * GCI_NoBaseType:      The type won't fit the requirements for basic types.
 */
static GCI_result decode( void *hidden,
                          const GCI_str *str,
                          GCI_parseinfo *pi,
                          int depth,
                          GCI_str *newName )
{
   const char *ptr = GCI_ccontent( str );
   int size = GCI_strlen( str );

   /*
    * Chop off leading and trailing spaces. We really need it.
    */
   while ( ( size > 0 ) && GCI_isspace( *ptr ) )
   {
      ptr++;
      size--;
   }
   while ( ( size > 0 ) && GCI_isspace( ptr[size - 1] ) )
      size--;

   memset( pi, 0, sizeof( GCI_parseinfo ) );

   if ( ( pi->type = checkname( &ptr, &size ) ) == GCI_unknown )
      return GCI_UnsupportedType;
   if ( pi->type == GCI_indirect )
   {
      while ( ( size > 0 ) && GCI_isspace( *ptr ) )
      {
         ptr++;
         size--;
      }
      pi->type = checkname( &ptr, &size );
      if ( ( pi->type == GCI_unknown ) || ( pi->type == GCI_indirect ) )
         return GCI_UnsupportedType;
      pi->indirect = 1;
   }
   else
      pi->indirect = 0;

   /*
    * Check for a size operand.
    */
   while ( ( size > 0 ) && GCI_isspace( *ptr ) )
   {
      ptr++;
      size--;
   }

   /*
    * We may have a size operand only if not an array or container is
    * processed!
    * This implementation shall support plain types like "INTEGER" without
    * any bit size.
    */
   switch ( pi->type )
   {
      case GCI_container:
         if ( size > 0 )
         {
            GCI_str tmp;

            if ( checkname( &ptr, &size ) != GCI_like )
               return GCI_UnsupportedType;
            while ( ( size > 0 ) && GCI_isspace( *ptr ) )
            {
               ptr++;
               size--;
            }
            if ( size == 0 )
            {
               /*
                * Single "like" after "container".
                */
               return GCI_UnsupportedType;
            }
            while ( GCI_isspace( ptr[size - 1] ) )
               size--;
            /*
             * Cut off a final dot, we append one later.
             */
            if ( ptr[size - 1] == '.' )
            {
               /*
                * Check for single "." as stem.
                */
               if ( --size == 0 )
                  return GCI_UnsupportedType;
            }
            if ( GCI_stralloc( hidden, newName, size + 256 ) != GCI_OK )
               return GCI_NoMemory;
            GCI_strfromascii( &tmp, (char *) ptr, size );
            GCI_strsetlen( &tmp, size );
            GCI_strcpy( newName, &tmp );
            size = 0;
         }
         /* fall through */

      case GCI_array:
         if ( size > 0 )
            return GCI_UnsupportedType;
         if ( ( depth == 0 ) && !pi->indirect )
         {
            if ( GCI_content( newName ) != NULL )
               GCI_strfree( hidden, newName );
            return GCI_NoBaseType;
         }
         pi->size = 0;
         return GCI_OK;

      case GCI_integer:
         if ( size == 0 )
            pi->size = 8 * sizeof( int );
         break;

      case GCI_unsigned:
         if ( size == 0 )
            pi->size = 8 * sizeof( unsigned );
         break;

      case GCI_float:
         if ( size == 0 )
            pi->size = 8 * sizeof( double ); /* surprised? */
         break;

      case GCI_char:
         if ( size == 0 )
            pi->size = 8 * 1; /* always, even in unicode or utf8 systems */
         break;

      case GCI_string:
      case GCI_raw:
         if ( size == 0 ) /* length must be supplied */
            return GCI_UnsupportedType;
         break;

      default:
         return GCI_UnsupportedType;
   }

   if ( size > 0 )
   {
      if ( GCI_string2bin( hidden,
                           ptr,
                           size,
                           &pi->size,
                           sizeof( pi->size ),
                           GCI_unsigned ) != GCI_OK )
         return GCI_UnsupportedType;
   }

   if ( ( ( pi->type == GCI_string ) || ( pi->type == GCI_raw ) ) &&
        ( pi->size > 0 ) )
      return GCI_OK;

   /*
    * A byte has 8 bit, always! We don't support PDP10.
    */
   if ( ( pi->type != GCI_string ) && ( pi->type != GCI_raw ) )
   {
      if ( pi->size % 8 )
         return GCI_UnsupportedType;
      pi->size /= 8;
   }

   return GCI_validate( pi->size, pi->type, depth || pi->indirect );
}

/*
 * parse is the local implementation of GCI_parsetree below. Most parameters
 * are in *cb. Have a look at callblock at top of file or at GCI_parsestring
 * below.
 * itemnumber is the iterator of the container item or array item, the later
 * always has number 1.
 *
 * The function loops over a type structure tree, the current node name is
 * placed in cb->buffer. We do a depth-first iteration.
 *
 * Indirect array[x] are replaced by a combination of
 * indirect container[1], array[x]. This allows a better addressing later.
 * The indirect container is flagged as "generated" in this case.
 *
 * THE GENERATED TYPES MAY HAVE ILLEGAL BIT SIZES. IT ISN'T CHECKED ALWAYS!
 *
 * Return values:
 * GCI_OK:              Everything is fine.
 *
 *                      In case of an error cb->buffer will contain the
 *                      variable's name where the problem raises first.
 *
 * GCI_MissingName:     A variable's name isn't set. This is the equivalence
 *                      for GCI_MissingValue in the type parsing step. The
 *                      system may or may not raise a NOVALUE condition instead
 *                      depending on the implementation.
 * GCI_BufferTooSmall:  The variable's name buffer cb->buffer can't hold the
 *                      complete variable's name or the type string exceeds
 *                      256 byte.
 * GCI_IllegalName:     The variables name in cb->buffer is illegal in terms of
 *                      Rexx. In general, the basename of GCI_paretree is
 *                      wrong.
 * GCI_RexxError:       An unexpected error is returned by the interpreter
 *                      while trying to access Rexx variables.
 * GCI_UnsupportedType: Wrong type of input, e.g. FLOAT31 or the empty string
 *                      in a type description string. Another reason is an
 *                      internal error since the default sizes for "unsigned"
 *                      and "integer" are not supported.
 * GCI_WrongInput:      Strange characters occur in the input string as the
 *                      bit size of the type.
 * GCI_NumberRange:     Number to small or big to fit into the desired type
 *                      with the desired destbyte-size. This applies to the
 *                      element count of an "ARRAY" or "CONTAINER" type size
 *                      or the bit size of the plain type.
 * GCI_NoBaseType:      The type won't fit the requirements for basic types.
 *
 * And there are numerous other possible errors returned by cb->callback.
 */
static GCI_result parse( callblock *cb,
                         int itemnumber )
{
   GCI_parseinfo pi;
   GCI_str newName;
   static const GCI_parseinfo indirectArray = { GCI_container, 1, 1, 1 };
   GCI_result rc;
   unsigned i;
   int origlen = GCI_strlen( cb->buffer );

   GCI_strfromascii( &newName, NULL, 0 );
   GCI_strcats( cb->buffer, "." );
   GCI_strcats( cb->buffer, cb->prefixChar );
   if ( ( rc = GCI_strcats( cb->buffer, "TYPE" ) ) != GCI_OK )
      return rc;
   if ( ( rc = GCI_readRexx( cb->hidden,
                             cb->buffer,
                             &cb->tempbuf,
                             0,
                             1,
                             NULL ) ) != GCI_OK )
   {
      if ( rc == GCI_MissingValue )
         rc = GCI_MissingName;
      return rc;
   }
   GCI_uppercase( cb->hidden, &cb->tempbuf );

   if ( ( rc = decode( cb->hidden, &cb->tempbuf, &pi, cb->depth, &newName ) )
                                                                    != GCI_OK )
      return rc;
   GCI_strsetlen( cb->buffer, origlen );

   if ( GCI_content( &newName ) != NULL ) {
      if (cb->recurCount++ >= 100) {
         GCI_strfree( cb->hidden, &newName );
         return GCI_NestingOverflow;
      }
      GCI_strswap( &newName, cb->buffer );
      origlen = GCI_strlen( cb->buffer );
   }

   /*
    * Alright, we have it, but we have to fetch the number of elements
    * if we parse a container or an array.
    */
   if ( ( pi.type == GCI_container ) || ( pi.type == GCI_array ) )
   {
      if ( ( rc = GCI_strcats( cb->buffer, ".0" ) ) != GCI_OK )
      {
         /*
          * The tmp buffer persists for error displaying, kill the other.
          */
         if ( GCI_content( &newName ) != NULL )
            GCI_strfree( cb->hidden, &newName );
         return rc;
      }
      if ( ( rc = GCI_readRexx( cb->hidden,
                                cb->buffer,
                                &cb->tempbuf,
                                0,
                                1,
                                NULL ) ) != GCI_OK )
      {
         if ( rc == GCI_MissingValue )
            rc = GCI_MissingName;
         /*
          * The tmp buffer persists for error displaying, kill the other.
          */
         if ( GCI_content( &newName ) != NULL )
            GCI_strfree( cb->hidden, &newName );
         return rc;
      }

      /*
       * The result shall be a whole, positive number. Lets see...
       */
      if ( ( rc = GCI_string2bin( cb->hidden,
                                  GCI_content( &cb->tempbuf ),
                                  GCI_strlen( &cb->tempbuf ),
                                  &pi.size,
                                  sizeof( pi.size ),
                                  GCI_unsigned ) ) != GCI_OK )
      {
         /*
          * The tmp buffer persists for error displaying, kill the other.
          */
         if ( GCI_content( &newName ) != NULL )
            GCI_strfree( cb->hidden, &newName );
         return rc;
      }
      if ( pi.size == 0 )
      {
         /*
          * The tmp buffer persists for error displaying, kill the other.
          */
         if ( GCI_content( &newName ) != NULL )
            GCI_strfree( cb->hidden, &newName );
         return GCI_NumberRange;
      }
      GCI_strsetlen( cb->buffer, origlen );
   }

   if ( pi.indirect && ( pi.type == GCI_array ) )
   {
      if ( ( rc = cb->callback( cb->depth,
                                itemnumber,
                                cb->arg,
                                &indirectArray) ) != GCI_OK )
         return rc;
      pi.indirect = 0;
      if ( ( rc = cb->callback( cb->depth,
                                itemnumber,
                                cb->arg,
                                &pi) ) != GCI_OK )
         return rc;
      pi.indirect = 1;
   }
   else
   {
      if ( ( rc = cb->callback( cb->depth,
                                itemnumber,
                                cb->arg,
                                &pi) ) != GCI_OK )
      {
         /*
          * The tmp buffer persists for error displaying, kill the other.
          */
         if ( GCI_content( &newName ) != NULL )
            GCI_strfree( cb->hidden, &newName );
         return rc;
      }
   }

   if ( ( pi.type != GCI_container ) && ( pi.type != GCI_array ) )
      return GCI_OK;

   cb->depth++;
   for ( i = 0; i < pi.size; i++ )
   {
      sprintf( cb->helper, ".%u", i + 1 );

      if ( ( rc = GCI_strcats( cb->buffer, cb->helper ) ) != GCI_OK )
      {
         /*
          * The tmp buffer persists for error displaying, kill the other.
          */
         if ( GCI_content( &newName ) != NULL )
            GCI_strfree( cb->hidden, &newName );
         return rc;
      }
      if ( ( rc = parse( cb, i ) ) != GCI_OK )
      {
         /*
          * The tmp buffer persists for error displaying, kill the other.
          */
         if ( GCI_content( &newName ) != NULL )
            GCI_strfree( cb->hidden, &newName );
         return rc;
      }
      GCI_strsetlen( cb->buffer, origlen );

      if ( pi.type == GCI_array )
         break;
   }
   cb->depth--;
   cb->recurCount--;
   if ( GCI_content( &newName ) != NULL )
   {
      GCI_strswap( &newName, cb->buffer );
      GCI_strfree( cb->hidden, &newName );
   }

   if ( pi.indirect && ( pi.type == GCI_array ) )
   {
      pi.indirect = 0;
      if ( ( rc = cb->callback( cb->depth, -1, cb->arg, &pi) ) != GCI_OK )
         return rc;
      return cb->callback( cb->depth, -1, cb->arg, &indirectArray );
   }
   return cb->callback( cb->depth, -1, cb->arg, &pi );
}

/*****************************************************************************
 *****************************************************************************
 ** GLOBAL FUNCTIONS *********************************************************
 *****************************************************************************
 *****************************************************************************/

/*
 * GCI_parsetree tries to interpret the type information of the first
 * generation. That are the branches "branch.return", "branch.1",
 * "branch.2", ... The value of "base" is such an identifier.
 * base must be a string large enough to hold the longest type variable name
 * which occurs in the tree including the branch name. This argument will
 * contain the variable's name which forces the error in case of an error.
 * The buffer is always modified.
 *
 * callback is described later. arg is passed to callback without further
 * interpretation.
 *
 * prefixChar is the prefix that must be used in front of stem names.
 *
 * The function loops over a type structure tree, the current node name is
 * placed in cb->buffer. We do a depth-first iteration. The callback function
 * is called at least for each type declaration. The callback is described
 * below the error codes.
 *
 * THE COMPLETE PARSING IS STOPPED IF THE callback RETURNS ANOTHER VALUE THAN
 * GCI_OK.
 *
 * THE GENERATED TYPES MAY HAVE ILLEGAL BIT SIZES. IT ISN'T CHECKED ALWAYS!
 *
 * Return values:
 * GCI_OK:              Everything is fine.
 *
 *                      In case of an error cb->buffer will contain the
 *                      variable's name where the problem raises first.
 *
 * GCI_MissingName:     A variable's name isn't set. This is the equivalence
 *                      for GCI_MissingValue in the type parsing step. The
 *                      system may or may not raise a NOVALUE condition instead
 *                      depending on the implementation.
 * GCI_BufferTooSmall:  The variable's name buffer cb->buffer can't hold the
 *                      complete variable's name or the type string exceeds
 *                      256 byte.
 * GCI_IllegalName:     The variables name in cb->buffer is illegal in terms of
 *                      Rexx. In general, the basename of GCI_paretree is
 *                      wrong.
 * GCI_RexxError:       An unexpected error is returned by the interpreter
 *                      while trying to access Rexx variables.
 * GCI_UnsupportedType: Wrong type of input, e.g. FLOAT31 or the empty string
 *                      in a type description string. Another reason is an
 *                      internal error since the default sizes for "unsigned"
 *                      and "integer" are not supported.
 * GCI_WrongInput:      Strange characters occur in the input string as the
 *                      bit size of the type.
 * GCI_NumberRange:     Number to small or big to fit into the desired type
 *                      with the desired destbyte-size. This applies to the
 *                      element count of an "ARRAY" or "CONTAINER" type size
 *                      or the bit size of the plain type.
 * GCI_NoBaseType:      The type won't fit the requirements for basic types.
 *
 * And there are numerous other possible errors returned by cb->callback.
 *
 *****************************************************************************
 *
 * Description of the callback:
 * This function will be called when a type string has been parsed. The
 * GCI_parseinfo structure is filled to allow the caller allocating enough
 * memory.
 *
 * depth is the current depth within the structure starting with 0. itemnumber
 * is the current number of the item within a structure starting with 1.
 * Arrays will have an item of 1, not more, not less. The item describes all
 * identical values in the array.
 * itemnumber will be 0 for the base element (of depth 0).
 *
 * itemnumber may be -1! This happens on containers and arrays after
 * processing the last subitem. This allows the callback to close the
 * current container and to do some final processing.
 *
 * arg is the copy of the arg parameter of GCI_parsetree. A hidden parameter
 * must be passed in arg (too) if it is needed.
 *
 * The caller of GCI_parsetree is responsible to understand additional
 * return codes of the callback.
 */
GCI_result GCI_parsetree( void *hidden,
                          GCI_str *base,
                          GCI_result (*callback)(int depth,
                                                 int itemnumber,
                                                 void *arg,
                                                 const GCI_parseinfo *info),
                          void *arg,
                          const char *prefixChar )
{
   callblock cb;
   /*
    * All simple type values must fit into a static buffer.
    */
   char tmp[256];
   GCI_strOfCharBuffer( tmp );

   /*
    * We wrap the parameters in a block to prevent further parameters.
    * We use the given buffer for all further processing. We'll have the
    * errorneous string at once in that case.
    */
   cb.hidden = hidden;
   cb.buffer = base;
   cb.depth = 0;
   cb.callback = callback;
   cb.arg = arg;
   cb.tempbuf = str_tmp;
   cb.recurCount = 0;
   cb.prefixChar = prefixChar;

   return parse( &cb, 0 );
}
