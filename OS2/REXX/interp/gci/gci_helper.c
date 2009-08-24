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
 * This file contains little helper routines.
 */

#include "gci.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*****************************************************************************
 *****************************************************************************
 ** GLOBAL FUNCTIONS *********************************************************
 *****************************************************************************
 *****************************************************************************/

/*
 * GCI_strlen returns the current length of the string.
 */
int GCI_strlen( const GCI_str *str )
{
   return str->used;
}

/*
 * GCI_strmax returns the maximum length of the string including any
 * terminators.
 */
int GCI_strmax( const GCI_str *str )
{
   return str->max;
}

/*
 * GCI_content returns a pointer to the content of str. The content may not be
 * 0-terminated.
 */
char * GCI_content( GCI_str *str )
{
   return str->val;
}

/*
 * GCI_content returns a pointer to the content of str. The content may not be
 * 0-terminated.
 */
const char * GCI_ccontent( const GCI_str *str )
{
   return str->val;
}

/*
 * GCI_streq returns 1 if two string have an equal content, 0 otherwise.
 */
int GCI_streq( const GCI_str *s1,
               const GCI_str *s2 )
{
   int l = s1->used;

   if ( l != s2->used )
      return 0;
   return ( memcmp( s1->val, s2->val, l ) == 0 ) ? 1 : 0;
}

/*
 * concat appends the content of second to first. second_len contains the
 * length of second. second may not be 0-terminated.
 * The return value is either GCI_OK or GCI_BufferTooSmall.
 */
static GCI_result concat( GCI_str *first,
                          const char *second,
                          int size )
{
   if ( first->used + size > first->max )
      return GCI_BufferTooSmall;

   memcpy( first->val + first->used, second, size );
   first->used += size;
   return GCI_OK;
}

/*
 * GCI_strcat appends the content of second to first.
 * The return value is either GCI_OK or GCI_BufferTooSmall.
 */
GCI_result GCI_strcat( GCI_str *first,
                       const GCI_str *second )
{
   return concat( first, second->val, second->used );
}

/*
 * GCI_strcats appends the content of the 0-terminated string second to first.
 * The return value is either GCI_OK or GCI_BufferTooSmall.
 */
GCI_result GCI_strcats( GCI_str *first,
                        const char *second )
{
   return concat( first, second, strlen( second ) );
}

/*
 * GCI_strsetlen sets the length of str to max. If max if bigger than the
 * string's internal maximum it is set to the string's internal maximum.
 */
void GCI_strsetlen( GCI_str *str,
                    int max )
{
   if ( str->max < max )
      str->used = str->max;
   else
      str->used = max;
}

/*
 * GCI_strcpy sets the content of first to the content of second.
 * The return value is either GCI_OK or GCI_BufferTooSmall.
 */
GCI_result GCI_strcpy( GCI_str *first,
                       const GCI_str *second )
{
   if ( first->max < second->used )
      return GCI_BufferTooSmall;

   memcpy( first->val, second->val, second->used );
   first->used = second->used;
   return GCI_OK;
}

/*
 * GCI_strfromascii creates a GCI_str from a character pointer. The content's
 * maximum is set to max, the content itself is taken from ptr.
 * The used length is set to 0.
 * THE FILLED GCI_str MUST NEVER BE PASSED TO GCI_strfree!
 * The passed string is returned.
 */
GCI_str *GCI_strfromascii( GCI_str *str,
                           char *ptr,
                           int max )
{
   str->val = ptr;
   str->used = 0;
   str->max = max;

   return str;
}

/*
 * GCI_migratefromascii creates a GCI_str from a character pointer.
 * The content's maximum and its length is set to length of the zero-terminated
 * string in ptr, the content itself is taken from ptr.
 * THE FILLED GCI_str MUST NEVER BE PASSED TO GCI_strfree!
 * The passed string is returned.
 */
const GCI_str *GCI_migratefromascii( GCI_str *str,
                                     const char *ptr )
{
   str->val = (char *) ptr;
   str->used = strlen( ptr );
   str->max = str->used;

   return str;
}

/*
 * GCI_stralloc allocates the content of a string. The content will have
 * size bytes. The former content won't be freed if there is something
 * allocated.
 * The return value is either GCI_OK or GCI_NoMemory.
 */
GCI_result GCI_stralloc( void *hidden,
                         GCI_str *str,
                         unsigned size )
{
   (hidden = hidden);

   if ( size == 0 )
      size = 1;
   if ( ( str->val = (char *) GCI_malloc( hidden, size ) ) == NULL )
      return GCI_NoMemory;

   str->used = 0;
   str->max = size;
   return GCI_OK;
}

/*
 * GCI_strfree deallocates the content of a string. The string itself isn't
 * freed. The content is set to NULL.
 */
void GCI_strfree( void *hidden,
                  GCI_str *str )
{
   (hidden = hidden);

   if ( str->val != NULL )
      GCI_free( hidden, str->val );

   str->val = NULL;
   str->max = str->used = 0;
}

/*
 * GCI_strdup duplicates the content of a string second and assigns it to
 * first. The content of first will be overwritten without any cleanup.
 * This function allocates the needed bytes only.
 * The return value is either GCI_OK or GCI_NoMemory.
 */
GCI_result GCI_strdup( void *hidden,
                       GCI_str *first,
                       const GCI_str *second )
{
   (hidden = hidden);

   if ( ( first->val = (char *) GCI_malloc( hidden, second->used ) ) == NULL )
      return GCI_NoMemory;

   memcpy( first->val, second->val, second->used );
   first->used = first->max = second->used;
   return GCI_OK;
}

/*
 * GCI_strtoascii duplicates the content of the string str to a newly created
 * ASCIIZ string which is returned. NULL is returned in case of an error or
 * if str describes a NULL string (in opposite to an empty string).
 */
char *GCI_strtoascii( void *hidden,
                      const GCI_str *str )
{
   char *retval;

   (hidden = hidden);

   if ( ( str == NULL ) || ( str->val == NULL ) )
      return NULL;
   if ( ( retval = (char *) GCI_malloc( hidden, str->used + 1 ) ) == NULL )
      return NULL;

   memcpy( retval, str->val, str->used );
   retval[str->used] = '\0';
   return retval;
}

/*
 * GCI_uppercase transforms the content of str to its uppercased value.
 * We may or may not convert foreign characters. It depends on the runtime
 * system and on the environment settings of codepage/language.
 */
void GCI_uppercase( void *hidden,
                    GCI_str *str )
{
   int i, len = str->used;
   char *ptr = str->val;

   (hidden = hidden);

   for ( i = 0; i < len; i++, ptr++ )
      *ptr = (char) GCI_toupper( *ptr );
}

/*
 * GCI_describe converts a GCI_result to a more or less human readable
 * string. The caller shall provide a string with at least 80 byte.
 * THIS FUNCTION IS NOT THREAD SAFE.
 */
void GCI_describe( GCI_str *description,
                   GCI_result rc )
{
   char *ptr = NULL;
   static char buf[40];

   switch ( rc )
   {  /* oversized lines! */                                                 /* here is the limit for the text: -->blahblah" */
      case GCI_OK:                   ptr = "GCI completed successfully";                                                      break;
      case GCI_NoMemory:             ptr = "Out of memory while processing a GCI request";                                    break;
      case GCI_WrongInput:           ptr = "Unexpected input, either unknown type or illegal data";                           break;
      case GCI_NumberRange:          ptr = "Number out of the allowed range";                                                 break;
      case GCI_StringRange:          ptr = "String too big for the defined buffer";                                           break;
      case GCI_UnsupportedType:      ptr = "Illegal combination of type/size";                                                break;
      case GCI_UnsupportedNumber:    ptr = "Unsupported number like NAN, +INF, -INF";                                         break;
      case GCI_BufferTooSmall:       ptr = "Structure too complex for static internal buffer";                                break;
      case GCI_MissingName:          ptr = "An element of the structure is missing";                                          break;
      case GCI_MissingValue:         ptr = "A value of the structure is missing";                                             break;
      case GCI_IllegalName:          ptr = "The name or part of the name is illegal for the interpreter";                     break;
      case GCI_RexxError:            ptr = "A problem raises when communicating with the interpreter";                        break;
      case GCI_NoBaseType:           ptr = "The type won't fit the requirements for basic types (arguments/return value)";    break;
      case GCI_InternalError:        ptr = "An unknown internal GCI error occured";                                           break;
      case GCI_FunctionAlreadyKnown: ptr = "New REXX function already registered to the interpreter";                         break;
      case GCI_LibraryNotFound:      ptr = "The external library was not found or can't be loaded";                           break;
      case GCI_NoLibraryFunction:    ptr = "The external function can't be found in the external library";                    break;
      case GCI_FunctionNotFound:     ptr = "The REXX function was not or no longer registered by the interpreter";            break;
      case GCI_SyntaxError:          ptr = "The number of arguments is wrong or an argument is missing";                      break;
      case GCI_CoreConfused:         ptr = "The core of GCI can't determine how to invoke generic functions";                 break;
      case GCI_ArgStackOverflow:     ptr = "GCI's internal stack for arguments got an overflow";                              break;
      case GCI_NestingOverflow:      ptr = "GCI counted too many nested LIKE containers";                                     break;
      default:
         break;
   }

   if ( ptr == NULL )
   {
      ptr = buf;
      sprintf( buf, "Unknown GCI error %d", rc );
   }
   description->val = ptr;
   description->used = strlen(ptr);
   description->max = description->used;
}

/*
 * GCI_strswap exchanges two strings' content completely.
 */
void GCI_strswap( GCI_str *first,
                  GCI_str *second )
{
   GCI_str h;

   h = *first;
   *first = *second;
   *second = h;
}
