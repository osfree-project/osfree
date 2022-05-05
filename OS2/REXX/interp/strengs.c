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
#include <string.h>
#include <stdio.h>
#include <assert.h>

#if !defined(MIN)
# define MIN(a,b) (((a)>(b))?(b):(a))
#endif

void __regina_Str_first( void )
{
   /*
    * Together with __regina_Str_last it defines a region of function
    * which should not be mentioned in the list of allocator addresses.
    * See memory.c:mymallocTSD()
    */
}

streng *Str_ndup( const streng *name, int length )
{
   return Str_ndup_TSD(__regina_get_tsd(), name, length);
}

streng *Str_ndup_TSD( const tsd_t *TSD, const streng *name, int length )
{
   streng *ptr;

   assert( Str_len( name ) >= length );
   ptr = Str_makeTSD( length );
   ptr->len = length;

   memcpy( ptr->value, name->value, length );
   return ptr;
}

streng *Str_nodup( const streng *name, int offset, int length )
{
   return Str_nodup_TSD( __regina_get_tsd(), name, offset, length );
}

streng *Str_nodup_TSD( const tsd_t *TSD, const streng *name, int offset, int length )
{
   streng *ptr;

   assert( Str_len( name ) >= ( offset + length ) );
   ptr = Str_makeTSD( length );
   memcpy( ptr->value, name->value + offset, length );
   ptr->len = length;
   return ptr;
}

streng *Str_dup( const streng *name )
{
   return Str_dup_TSD( __regina_get_tsd(), name );
}

streng *Str_dup_TSD( const tsd_t *TSD, const streng *input )
{
   streng *output;

   output = Str_makeTSD( input->len );
   output->len = input->len;
   memcpy( output->value, input->value, input->len );

   return output;
}

streng *Str_dupstr( const streng *name )
{
   return Str_dupstr_TSD( __regina_get_tsd(), name );
}

/* Str_dupstr works like Str_dup but appends a hidden '\0' at the end. The
 * terminator will be deleted by further Str_... operations.
 */
streng *Str_dupstr_TSD( const tsd_t *TSD, const streng *input )
{
   streng *output;

   output = Str_makeTSD( input->len + 1 );
   output->len = input->len;
   memcpy( output->value, input->value, input->len );
   output->value[input->len] = '\0';

   return output;
}

streng *Str_ncpy( streng *to, const streng *from, int length )
{
   return Str_ncpy_TSD( __regina_get_tsd(), to, from, length );
}

streng *Str_ncpy_TSD( const tsd_t *TSD, streng *to, const streng *from, int length )
{
   streng *newstr;

   assert( Str_len(from) >= length );
   newstr = to;
   if ( to->max < to->len + length )
   {
      newstr = Str_makeTSD( to->len + length );
      memcpy( newstr->value, to->value, to->len );
      newstr->len = to->len;
      /*
       * Freeing the string is a bad idea if the string is bound to
       * something yet.
       * FreeTSD( to );
       */
   }

   if ( length > Str_len( from ) )
      length = Str_len( from );
   memcpy( newstr->value, from->value, length );
   newstr->len += length;

   return newstr;
}

streng *Str_ncre( const char *from, int length )
{
   return Str_ncre_TSD( __regina_get_tsd(), from, length );
}

streng *Str_ncre_TSD( const tsd_t *TSD, const char *from, int length )
{
   streng *result;

   assert( length >= 0 );

   result = Str_makeTSD( length );
   memcpy( result->value, from, length );

   result->len = length;
   return result;
}


int Str_ncmp( const streng *first, const streng *second, int limit )
{
   int i,rlim;

   rlim = MIN( limit, MIN( first->len, second->len ) );
   for ( i = 0; i < rlim; i++)
      if (first->value[i] != second->value[i])
         return (int) ( (unsigned char) first->value[i] -
                        (unsigned char) second->value[i] );

   if ( rlim < limit )
      return (first->len != second->len );

   return 0;
}


streng *Str_cre( const char *input )
{
   return Str_cre_TSD( __regina_get_tsd(), input );
}

streng *Str_cre_TSD( const tsd_t *TSD, const char *input )
{
   streng *result;
   int len;

   len = strlen( input );

   result = Str_makeTSD( len );
   result->len = len;
   memcpy( result->value, input, len );
   return result;
}

streng *Str_crestr( const char *input )
{
   return Str_crestr_TSD( __regina_get_tsd(), input );
}

streng *Str_crestr_TSD( const tsd_t *TSD, const char *input )
{
   streng *result;
   int len;

   len = strlen( input ) + 1;

   result = Str_makeTSD( len );
   result->len = len - 1;
   memcpy( result->value, input, len );
   return result;
}


#if !defined(FLISTS) || defined(TRACEMEM)
streng *__regina_Str_make( int size )
{
   return Str_make_TSD( __regina_get_tsd(), size );
}

# ifdef CHECK_MEMORY
streng *__regina_Str_make_TSD( const tsd_t *TSD, int size )
{
   streng *retval;

   retval = MallocTSD( sizeof( streng ) );
   if ( retval != NULL )
   {
      retval->len = 0;
      retval->max = size;
      if (size == 0)
         size = 1; /* Don't allow malloc(0), Checker doesn't like it */
      if ( ( retval->value = MallocTSD( size ) ) == NULL )
      {
         FreeTSD( retval );
         retval = NULL;
         exiterror( ERR_STORAGE_EXHAUSTED, 0 );
      }
   }
   else
      exiterror( ERR_STORAGE_EXHAUSTED, 0 );
   return retval;
}
# else
streng *__regina_Str_make_TSD( const tsd_t *TSD, int size )
{
   streng *result;

   result = (streng *) MallocTSD( size + STRHEAD );
   result->max = size;
   result->len = 0;

   return result;
}
# endif /* CHECK_MEMORY */
#endif /* !defined(FLISTS) || defined(TRACEMEM) */


static streng *assure( const tsd_t *TSD, const streng *str, int length )
{
   streng *ptr;

   if ( length <= str->max )
      return (streng *) str;

   ptr = Str_makeTSD( length );
   memcpy( ptr->value, str->value, str->len );
   ptr->len = str->len;

   /* We don't free str. */
   return ptr;
}


streng *Str_catstr( streng *base, const char *append )
{
   return Str_catstr_TSD( __regina_get_tsd(), base, append );
}

streng *Str_catstr_TSD( const tsd_t *TSD, streng *base, const char *append )
{
   streng *ptr;
   int tmp;

   tmp = strlen( append );
   ptr = assure( TSD, base, tmp + base->len );
   memcpy( ptr->value + ptr->len, append, tmp );
   ptr->len += tmp;
   return ptr;
}


streng *Str_nocat( streng *first, const streng *second, int length, int offset )
{
   return Str_nocat_TSD( __regina_get_tsd(), first, second, length, offset );
}

streng *Str_nocat_TSD( const tsd_t *TSD, streng *first, const streng *second, int length, int offset )
{
   streng *ptr;
   int tmp;

   assert( second->len + 1 >= offset + length );

   tmp = second->len - offset;
   if ( ( tmp < 0 ) || ( tmp > length ) )
      tmp = length;

   ptr = assure( TSD, first, first->len + tmp );
   memcpy( &ptr->value[first->len], &second->value[offset], tmp );

   ptr->len += tmp;
   return ptr;
}


streng *Str_cat( streng *first, const streng *second )
{
   return Str_cat_TSD( __regina_get_tsd(), first, second );
}

streng *Str_cat_TSD( const tsd_t *TSD, streng *first, const streng *second )
{
   streng *ptr;
   int tmp;

   tmp = Str_len( first ) + Str_len( second );
   ptr = assure( TSD, first, tmp );
   memcpy( ptr->value + Str_len( first ), second->value, Str_len( second ) );
   ptr->len = tmp;

   return ptr;
}


streng *Str_ncat( streng *first, const streng *second, int length )
{
   return Str_ncat_TSD( __regina_get_tsd(), first, second, length );
}

streng *Str_ncat_TSD( const tsd_t *TSD, streng *first, const streng *second, int length )
{
   streng *ptr;
   int tmp;

   tmp = MIN( length, Str_len( second ) );
   ptr = assure( TSD, first, Str_len( first ) + tmp );
   memcpy( ptr->value + Str_len( first ), second->value, tmp );

   ptr->len += tmp;
   return ptr;
}



int Str_cmp( const streng *first, const streng *second )
{
   int lim;

   lim = Str_len( first );
   if ( lim != Str_len( second ) )
      return 1;

   return memcmp( first->value, second->value, lim );
}



int Str_ccmp( const streng *first, const streng *second )
{
   int lim;

   lim = Str_len( first );
   if ( lim != Str_len( second ) )
      return 1;

   return ( mem_cmpic( first->value, second->value, lim ) ) ? 1 : 0;
}


int Str_cncmp( const streng *first, const streng *second, int length )
{
   int top, shorter;

   shorter = MIN( Str_len( first ), Str_len( second ) );
   if ( ( shorter < length ) && ( Str_len( first ) != Str_len( second ) ) )
      return 1;

   top = MIN( shorter, length );
   return ( mem_cmpic( first->value, second->value, top ) ) ? 1 : 0;
}


int Str_cnocmp( const streng *first, const streng *second, int length, int offset )
{
   int top, shorter;

   shorter = MIN( Str_len( first ), Str_len( second ) - offset );
   if ( ( shorter < length ) &&
      ( Str_len( first ) != Str_len( second ) - offset ) )
      return 1;

   top = MIN( shorter, length );
   return ( mem_cmpic( first->value, second->value + offset, top ) ) ? 1 : 0;
}


/*
 * str_of() returns a 0-terminated copy of the string-value of input. Free it
 * with function Free.
 */
char *str_of( const tsd_t *TSD, const streng *input )
{
   char *retval = (char *)MallocTSD( Str_len( input ) + 1 );

   memcpy( retval, input->value, Str_len( input ) );
   retval[Str_len( input )] = '\0';
   return retval;
}

/*
 * tmpstr_of() returns a temporarily allocated 0-terminated copy of the
 * string-value of input. Never free it explicitely. There is storage for 7
 * parallel living strings. The oldest will be deleted first. The main purpose
 * of this function is to get a 0-terminated string of a streng for a very
 * short time, e.g. exiterror. Since exiterror longjmp's back to another place,
 * you don't have any chance to free up the allocated string. This is done now
 * automatically.
 * Call this function with NULL as the argument to free all temporary strings.
 * WARNING: This function insn't thread-safe and won't ever be! Move the
 *          variable strs to the thread-specific memory to be thread-safe.
 */
volatile char *tmpstr_of( tsd_t *TSD, const streng *input )
{
   unsigned i;

   if ( input == NULL )
   {
      for ( i = 0; i < sizeof( TSD->tmp_strs ) / sizeof( TSD->tmp_strs[0] ); i++ )
      {
         if ( TSD->tmp_strs[i] != NULL )
         {
            FreeTSD( (char *) TSD->tmp_strs[i] );
            TSD->tmp_strs[i] = NULL;
         }
      }
      TSD->next_tmp_str = 0;
      return NULL;
   }

   /* allocate a new one */
   if ( TSD->tmp_strs[TSD->next_tmp_str] != NULL )
      FreeTSD( (char *) TSD->tmp_strs[TSD->next_tmp_str] );
   /* keep exiterror within Malloc in mind */
   TSD->tmp_strs[TSD->next_tmp_str] = NULL;
   TSD->tmp_strs[TSD->next_tmp_str] = str_of( TSD, input );
   i = TSD->next_tmp_str;

   if ( ++TSD->next_tmp_str >= (int) (sizeof( TSD->tmp_strs ) / sizeof( TSD->tmp_strs[0]) ) )
      TSD->next_tmp_str = 0;

   return TSD->tmp_strs[i];
}

streng *Str_ncatstr( streng *base, const char *input, int length )
{
   return Str_ncatstr_TSD( __regina_get_tsd(), base, input, length );
}

streng *Str_ncatstr_TSD( const tsd_t *TSD, streng *base, const char *input, int length )
{
   streng *ptr;
   int top;

   top = MIN( (int) strlen( input ), length );
   ptr = assure( TSD, base, Str_len( base ) + top );
   memcpy( ptr->value + Str_len( ptr ), input, top );
   ptr->len += top;
   return ptr;
}


streng *Str_upper( streng *input )
{
   mem_upper( input->value, Str_len( input ) );
   return input;
}

streng *Str_lower( streng *input )
{
   mem_lower( input->value, Str_len( input ) );
   return input;
}

streng *Str_strp( streng *input , char chr, char opt )
{
   int i,j;

   if ( input->len == 0 )
      return input;

   if ( opt & STRIP_TRAILING )
   {
      for ( i = input->len - 1; i > -1; i-- )
      {
         if ( input->value[i] != chr )
            break;
      }
      input->len = i + 1;
   }
   if ( opt & STRIP_LEADING )
   {
      for ( j = 0; j < input->len; j++ )
         if ( input->value[j] != chr )
            break;
      for ( i = 0; j < input->len; i++, j++ )
         input->value[i] = input->value[j];
      input->len = i;
   }
   return input;
}

void __regina_Str_last( void )
{
   /*
    * Together with __regina_Str_first it defines a region of function
    * which should not be mentioned in the list of allocator addresses.
    * See memory.c:mymallocTSD()
    */
}
