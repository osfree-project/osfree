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
 * This file contains the code for calling an external function.
 * This file must be compiled using cdecl calling conventions.
 * Finally we assume to have to fill up any argument smaller than a
 * GCI_STACK_ELEMENT up to the size of a GCI_STACK_ELEMENT.
 *
 * DON'T FORGET TO DISABLE ANY OPTIMIZER FOR THSI FILE!
 */

#include "gci_convert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <assert.h>
#include <setjmp.h>

static void *returnValue;

GCI_JUMPVAR(safetyRope)

/*
 * doCall is the core routine for calling an external routine. It doesn't
 * return by the obvious ways, it does a longjmp instead. This bypasses every
 * problem resulting from a wrong calling convention. We simply have to do the
 * call and assign the return value.
 *
 * func is the function to call, info is the return value's type and buf the
 * buffer of arguments for the function chopped in stack element pieces.
 * n may be NULL, we don't have a return value in this case.
 */
static void doCall( void (*func)(),
                    const GCI_parseinfo *info,
                    const GCI_STACK_ELEMENT *buf )
{
#ifdef __cplusplus
# define DOCALL(type) *((type *) returnValue) =                               \
                               ((type (*)(...))func)( GCI_PASSARGS( buf ) ); \
                     GCI_JUMP( GCI_JUMP_GETVAR( safetyRope ), 1 );           \
                     break;
#else
# define DOCALL(type) *((type *) returnValue) =                               \
                               ((type (*)())func)( GCI_PASSARGS( buf ) ); \
                     GCI_JUMP( GCI_JUMP_GETVAR( safetyRope ), 1 );           \
                     break;
#endif

   GCI_parseinfo ptr;

   if ( info == NULL )
   {
#ifdef __cplusplus
      ((void (*)(...))func)( GCI_PASSARGS( buf ) );
#else
      ((void (*)())func)( GCI_PASSARGS( buf ) );
#endif
      GCI_JUMP( GCI_JUMP_GETVAR( safetyRope ), 1 );
   }
   else
   {
      if ( info->indirect )
      {
         ptr.type = GCI_unsigned;
         ptr.size = sizeof(void *);
         info = &ptr;
      }
      switch ( info->type )
      {
         case GCI_integer:
            switch ( info->size )
            {
#ifdef GCI_I_1
               case 1:  DOCALL( GCI_I_1 );
#endif
#ifdef GCI_I_2
               case 2:  DOCALL( GCI_I_2 );
#endif
#ifdef GCI_I_4
               case 4:  DOCALL( GCI_I_4 );
#endif
#ifdef GCI_I_8
               case 8:  DOCALL( GCI_I_8 );
#endif
#ifdef GCI_I_16
               case 16: DOCALL( GCI_I_16 );
#endif
            }
            break;

         case GCI_unsigned:
            switch ( info->size )
            {
#ifdef GCI_U_1
               case 1:  DOCALL( GCI_U_1 );
#endif
#ifdef GCI_U_2
               case 2:  DOCALL( GCI_U_2 );
#endif
#ifdef GCI_U_4
               case 4:  DOCALL( GCI_U_4 );
#endif
#ifdef GCI_U_8
               case 8:  DOCALL( GCI_U_8 );
#endif
#ifdef GCI_U_16
               case 16: DOCALL( GCI_U_16 );
#endif
            }
            break;

         case GCI_float:
            switch ( info->size )
            {
#ifdef GCI_F_4
               case 4:  DOCALL( GCI_F_4 );
#endif
#ifdef GCI_F_8
               case 8:  DOCALL( GCI_F_8 );
#endif
#ifdef GCI_F_10
               case 10: DOCALL( GCI_F_10 );
#endif
#ifdef GCI_F_12
               case 12: DOCALL( GCI_F_12 );
#endif
#ifdef GCI_F_16
               case 16: DOCALL( GCI_F_16 );
#endif
            }
            break;

         case GCI_char:
            switch ( info->size )
            {
               case 1: DOCALL( char );
            }
            break;

         default:
            break;
      }
   }
}

/*
 * addCallList adds an argument to the call list.
 * info is the information about the current argument. ptr is the pointer
 * within the current argument list, src is the pointer to the value of the
 * argument.
 */
static void addCallList( const GCI_parseinfo *info,
                         char **ptr,
                         const char *src )
{
   char *p = *ptr;
   unsigned rest;
   int size = info->size;

   if ( info->indirect )
      size = sizeof(void *);

   /*
    * We don't support packed values.
    */
   rest = (unsigned) (unsigned long) ptr % sizeof( GCI_STACK_ELEMENT );
   assert( rest == 0 );
   if ( rest )
      p += sizeof( GCI_STACK_ELEMENT ) - rest;

   /*
    * Now compute the padding bytes.
    */
   if ( size >= (int) sizeof( GCI_STACK_ELEMENT ) )
   {
      /*
       * If you have a better idea and know on which boundary to place
       * big objects feel free to change this!
       */
      assert( ( size % sizeof( GCI_STACK_ELEMENT ) ) == 0 );

      memcpy( p, src, size );
      p += size;
   }
   else
   {
      /*
       * We have to pad depending on the endian.
       */
#if GCI_LITTLE_ENDIAN
      memcpy( p, src, size );
#else
      rest = sizeof( GCI_STACK_ELEMENT ) - size;
      memcpy( p + rest, src, size );
#endif
      p += sizeof( GCI_STACK_ELEMENT );
   }
   *ptr = p;
}


/*****************************************************************************
 *****************************************************************************
 ** GLOBAL FUNCTIONS *********************************************************
 *****************************************************************************
 *****************************************************************************/

/*
 * GCI_call calls the external function, builds the arguments for the call and
 * assigns the return value.
 *
 * func is the function we have to call, ti contains all informations how to
 * call the function and what to pass to the function and basebuf really
 * contains the data. Use ti->nodes to figure out, what to extract from
 * basebuf.
 *
 * Return codes:
 * GCI_OK:             Everything is perfect.
 * GCI_InternalError:  Unexpected type for the base type or calling convention.
 * GCI_CoreConfused:   The call didn't happen although everything has been
 *                     prepared to do so. In multithreading systems we have
 *                     a locked semaphore. Bye, bye cruel world...
 */
GCI_result GCI_call( void *hidden,
                     void (*func)(),
                     const GCI_treeinfo *ti,
                     char *basebuf )
{
   const GCI_nodeinfo *n;
   const GCI_parseinfo *info;
   GCI_STACK_ELEMENT buf[GCI_ARGS + 8];
   char *dest = (char *) buf;
   int start, incr, end;

   (hidden = hidden);

   memset( buf, 0, sizeof( buf ) );

   switch ( ti->callinfo.type )
   {
      case GCI_ctCdecl:
      case GCI_ctStdcall:
         start = 0;
         incr = 1;
         end = GCI_REXX_ARGS;
         break;

      case GCI_ctPascal:
         start = GCI_REXX_ARGS - 1;
         incr = -1;
         end = -1;
         break;

      default:
         return GCI_InternalError;
   }
   /*
    * It isn't very funny to produce code for different systems without
    * knowledge about the system and without assembler. This generic approach
    * won't work for systems where pascal's ordering on the stack isn't that
    * one of cdecl or stdcall in reversed order.
    * Feel free to provide better code!
    */

   for ( ; start != end; start = start + incr )
   {
      if ( ti->args[start] == -1 )
         continue;
      n = ti->nodes + ti->args[start];
      addCallList( &n->type,  &dest, basebuf + n->direct_pos );
      if ( ( dest - (char *) buf ) > (ptrdiff_t) (GCI_ARGS * sizeof(unsigned)) )
         return GCI_ArgStackOverflow;
   }

   if ( GCI_JUMP_SETVAR( hidden, safetyRope ) == 0 )
   {
      if ( ti->retval == -1 )
      {
         returnValue = NULL;
         info = NULL;
      }
      else
      {
         n = ti->nodes + ti->retval;
         returnValue = basebuf + n->direct_pos;
         info = &n->type;
      }
      doCall( func, info, buf );
      return GCI_CoreConfused;
   }

   return GCI_OK;
}
