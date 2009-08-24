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
 * This file contains the code to use Regina's internal structures.
 * use it instead.
 */

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gci.h"
#include "../rxiface.h"

typedef struct {
   void *tsd;
   void *treeinfo;
} regina;

/*
 * streng_of returns a new streng with the same content as str has.
 * NULL is allowed for str and will result in a NULL return value.
 * The non-existing string is allowed for for str and will result in an empty
 * string.
 */
static streng *streng_of( const tsd_t *TSD, const GCI_str *str )
{
   if ( str == NULL )
      return NULL;
   if ( GCI_ccontent( str ) == NULL )
      return nullstringptr();
   return Str_ncreTSD( GCI_ccontent( str ), GCI_strlen( str ) );
}

/*
 * Returns the translated function code from GCI_result to the code that
 * Regina shall return to the caller.
 * This function set the textual representation of an error code to that value
 * that will be accessed by RxFuncErrMsg() and sets the variable GCI_RC to
 * that value, too.
 *
 * dispo is either NULL (or the content is NULL) or contains the position of
 * the error within the structure. dispo's content will be deallocated.
 */
static int GCIcode2ReginaFuncCode( tsd_t *TSD,
                                   GCI_result rc,
                                   GCI_str *dispo,
                                   int forceError )
{
   GCI_str description, fullinfo, *fi = NULL, *out;
   volatile char *tmpDispo, *tmpFull = NULL, *tmpBest;
   streng *h;
   char GCI_RC[7];
   GCI_strOfCharBuffer(GCI_RC);

   GCI_strcats( &str_GCI_RC, "GCI_RC" );
   GCI_describe( &description, rc );

   if ( ( dispo != NULL ) && ( GCI_content( dispo ) == NULL ) )
      dispo = NULL;

   if ( ( dispo != NULL ) && ( rc != GCI_OK ) )
   {
      if ( GCI_stralloc( TSD, &fullinfo, GCI_strlen( dispo ) +
                                         GCI_strlen( &description ) +
                                         3 ) == GCI_OK )
      {
         fi = &fullinfo;
         GCI_strcpy( fi, &description );
         GCI_strcats( fi, ": " );
         GCI_strcat( fi, dispo );
      }
   }

   out = ( fi != NULL ) ? fi : &description;
   GCI_writeRexx( TSD, &str_GCI_RC, out, 0 );

   if ( ( rc == GCI_OK ) && !forceError )
   {
      if ( dispo != NULL )
         GCI_strfree( TSD, dispo );
      if ( fi != NULL )
         GCI_strfree( TSD, fi );
      return 0;
   }

   h = streng_of( TSD, &description );
   tmpDispo = tmpstr_of( TSD, h );
   Free_stringTSD( h );

   if ( fi != NULL )
   {
      h = streng_of( TSD, fi );
      tmpFull = tmpstr_of( TSD, h );
      Free_stringTSD( h );
   }

   if ( dispo != NULL )
      GCI_strfree( TSD, dispo );
   if ( fi != NULL )
      GCI_strfree( TSD, fi );

   /*
    * We have two temporary strings describing the error condition.
    * All stuff we have to deallocate is deallocated. Let's go.
    */
   tmpBest = ( tmpFull != NULL ) ? tmpFull : tmpDispo;
   set_err_message( TSD, (char *) tmpBest, "" );

   switch ( rc )
   {
      case GCI_NoMemory:
         exiterror( ERR_STORAGE_EXHAUSTED, 0 );

      case GCI_WrongInput:
         exiterror( ERR_INCORRECT_CALL, 980, ( tmpDispo ) ? ": " : "", ( tmpDispo ) ? tmpDispo : "" );

      case GCI_NumberRange:
         exiterror( ERR_INCORRECT_CALL, 981, ( tmpDispo ) ? ": " : "", ( tmpDispo ) ? tmpDispo : "" );

      case GCI_StringRange:
         exiterror( ERR_INCORRECT_CALL, 982, ( tmpDispo ) ? ": " : "", ( tmpDispo ) ? tmpDispo : "" );

      case GCI_UnsupportedType:
         if ( !forceError )
            return 71; /* RXFUNC_BADTYPE + 1 */
         exiterror( ERR_INCORRECT_CALL, 983, ( tmpDispo ) ? ": " : "", ( tmpDispo ) ? tmpDispo : "" );

      case GCI_UnsupportedNumber:
         exiterror( ERR_INCORRECT_CALL, 984, ( tmpDispo ) ? ": " : "", ( tmpDispo ) ? tmpDispo : "" );

      case GCI_BufferTooSmall:
         exiterror( ERR_INCORRECT_CALL, 985, ( tmpDispo ) ? ": " : "", ( tmpDispo ) ? tmpDispo : "" );

      case GCI_MissingName:
         exiterror( ERR_INCORRECT_CALL, 986, ( tmpDispo ) ? ": " : "", ( tmpDispo ) ? tmpDispo : "" );

      case GCI_MissingValue:
         exiterror( ERR_INCORRECT_CALL, 987, ( tmpDispo ) ? ": " : "", ( tmpDispo ) ? tmpDispo : "" );

      case GCI_IllegalName:
         exiterror( ERR_INCORRECT_CALL, 988, ( tmpDispo ) ? ": " : "", ( tmpDispo ) ? tmpDispo : "" );

      case GCI_RexxError:
         exiterror( ERR_INCORRECT_CALL, 989, ( tmpDispo ) ? ": " : "", ( tmpDispo ) ? tmpDispo : "" );

      case GCI_NoBaseType:
         exiterror( ERR_INCORRECT_CALL, 990, ( tmpDispo ) ? ": " : "", ( tmpDispo ) ? tmpDispo : "" );

      case GCI_SyntaxError:
         exiterror( ERR_INCORRECT_CALL, 991, ( tmpDispo ) ? ": " : "", ( tmpDispo ) ? tmpDispo : "" );

      case GCI_ArgStackOverflow:
         exiterror( ERR_INCORRECT_CALL, 992, ( tmpDispo ) ? ": " : "", ( tmpDispo ) ? tmpDispo : "" );

      case GCI_NestingOverflow:
         exiterror( ERR_INCORRECT_CALL, 993, ( tmpDispo ) ? ": " : "", ( tmpDispo ) ? tmpDispo : "" );

      default:
         break;
   }
   exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, tmpBest );
   return 0; /* Keep the compiler happy */
}

/*
 * GCI_migrateStreng converts a streng into a GCI_str. No further memory
 * allocation is done and it is STRONGLY forbidden to use GCI_strfree.
 * The return value shall be used for further operations.
 */
static const GCI_str *GCI_migrateStreng( GCI_str *str,
                                         const streng *string )
{
   if ( Str_len( string ) == RX_NO_STRING )
   {
      str->used = str->max = 0;
      str->val = NULL;
   }
   else if ( ( str->val = (char *) Str_val( string ) ) == NULL )
      str->used = str->max = 0;
   else
      str->used = str->max = (int) Str_len( string );

   return str;
}

/*
 * GCI_migrateRxString converts a RXSTRING into a GCI_str. No further memory
 * allocation is done and it is STRONGLY forbidden to use GCI_strfree.
 * The return value shall be used for further operations.
 */
static const GCI_str *GCI_migrateRxString( GCI_str *str,
                                           const RXSTRING *string )
{
   if ( !RXVALIDSTRING( *string ) && !RXZEROLENSTRING( *string ) )
   {
      str->val = NULL;
      str->used = str->max = 0;
   }
   else
   {
      str->val = (char *) RXSTRPTR( *string );
      str->used = str->max = (int) RXSTRLEN( *string );
   }
   return str;
}

/*
 * assignedRxString builds a RXSTRING of a GCI_str. The RXSTRING is set
 * in the usual manner and a terminating zero is appended without notification
 * of the target. NULL-strings are converted to empty strings.
 * Returns 1 on error, 0 on success.
 */
static int assignRxString( void *hidden,
                           PRXSTRING dest,
                           const GCI_str *src )
{
   char *h;

   if ( RXNULLSTRING( *dest ) || ( RXSTRLEN( *dest ) < (ULONG) src->used+1 ) )
   {
      if ( ( h = (char *) IfcAllocateMemory( (ULONG) (src->used+1) ) ) == NULL )
         return 1;
   }
   else
      h = RXSTRPTR( *dest );

   memcpy( h, src->val, src->used );
   h[(int) (src->used)] = '\0';
   MAKERXSTRING( *dest, h, (ULONG) src->used );
   return 0;
}

/*
 * readRexx works as a merged version of the function GCI_readRexx and
 * GCI_readNewRexx below. The difference is the flag allocate. If this is
 * set, the function works aas GCI_readNewRexx, otherwise is works like
 * GCI_readRexx.
 */
static GCI_result readRexx( void *hidden,
                            const GCI_str *name,
                            GCI_str *target,
                            int symbolicAccess,
                            int signalOnNovalue,
                            int allocate,
                            int *novalue )
{
   tsd_t *TSD = (tsd_t *) hidden;
   int retval;
   GCI_result rc;
   int Lengths[2];
   char *Strings[2];
   int allocated;

   Lengths[0] = GCI_strlen( name );
   Strings[0] = (char *) GCI_ccontent( name );

   if ( !signalOnNovalue )
      set_ignore_novalue( (const tsd_t *) hidden );
   retval = IfcVarPool( (tsd_t *) hidden,
                        ( symbolicAccess ) ? RX_GETSVAR : RX_GETVAR,
                        Lengths,
                        Strings,
                        &allocated );
   if ( !signalOnNovalue )
      clear_ignore_novalue( (const tsd_t *) hidden );

   switch ( retval )
   {
      case RX_CODE_OK:
         if ( novalue )
            *novalue = 0;
         rc = GCI_OK;
         break;

      case RX_CODE_NOVALUE:
         if ( novalue )
            *novalue = 1;
         rc = GCI_OK;
         break;

      case RX_CODE_INVNAME:
         return GCI_IllegalName;

      default:
         if ( allocated )
            FreeTSD( Strings[1] );
         return GCI_RexxError;
   }
   if ( Lengths[1] == RX_NO_STRING )
      return GCI_RexxError;

   /*
    * We must copy the value's content (Strings[1]/Lengths[1]) and we must
    * destroy the value if "allocated" is set.
    */
   if ( allocate )
   {
      if ( ( rc = GCI_stralloc( hidden, target, Lengths[1] ) ) != GCI_OK )
      {
         if ( allocated )
            FreeTSD( Strings[1] );
         return rc;
      }
   }
   else if ( Lengths[1] > GCI_strmax( target ) )
   {
      if ( allocated )
         FreeTSD( Strings[1] );

      return GCI_BufferTooSmall;
   }

   memcpy( GCI_content( target ), Strings[1], Lengths[1] );
   if ( allocated )
      FreeTSD( Strings[1] );
   GCI_strsetlen( target, Lengths[1] );
   return GCI_OK;
}

/*****************************************************************************
 *****************************************************************************
 ** GLOBAL FUNCTIONS *********************************************************
 *****************************************************************************
 *****************************************************************************/

/*
 * GCI_readRexx reads the content of one variable of name "name" into the
 * "target". The size or the content-holding string of target isn't changed,
 * the caller must provide a sufficient space.
 *
 * symbolicAccess shall be set if normal access is expected. If this variable
 * is 0, the variable's name is treated as "tail-expanded" and any further
 * interpretation of the name isn't done by the interpreter.
 *
 * signalOnNovalue shall be set if this function shall throw a NOVALUE
 * condition if the variable isn't set. This function may or may not be
 * able to do so. If not, the return value is set to GCI_MissingValue.
 *
 * *novalue is set either to 1 for a return of a variable's default value or
 * to 0 if the variable has an assigned value. novalue may be NULL.
 *
 * Return values:
 * GCI_OK:             Everything is fine.
 * GCI_MissingValue:   signalOnNovalue is set and this function doesn't
 *                     support to fire a NOVALUE condition.
 * GCI_BufferTooSmall: The "target" buffer is too small to hold the result.
 * GCI_IllegalName:    "name" is illegal in terms of Rexx. Especially on
 *                     non-"symbolicAccess" the caller must provide uppercased
 *                     stem names if a stem is used.
 * GCI_RexxError:      An unexpected other error is returned by the
 *                     interpreter.
 */
GCI_result GCI_readRexx( void *hidden,
                         const GCI_str *name,
                         GCI_str *target,
                         int symbolicAccess,
                         int signalOnNovalue,
                         int *novalue )
{
   return readRexx( hidden,
                    name,
                    target,
                    symbolicAccess,
                    signalOnNovalue,
                    0,
                    novalue );
}

/*
 * GCI_readNewRexx reads the content of one variable of name "name" into the
 * "target". The content of the target is overwritten regardless of its
 * current content.
 *
 * symbolicAccess shall be set if normal access is expected. If this variable
 * is 0, the variable's name is treated as "tail-expanded" and any further
 * interpretation of the name isn't done by the interpreter.
 *
 * signalOnNovalue shall be set if this function shall throw a NOVALUE
 * condition if the variable isn't set. This function may or may not be
 * able to do so. If not, the return value is set to GCI_MissingValue.
 *
 * *novalue is set either to 1 for a return of a variable's default value or
 * to 0 if the variable has an assigned value. novalue may be NULL.
 *
 * Return values:
 * GCI_OK:             Everything is fine.
 * GCI_NoMemory:       Can't allocate enough memory for the return value.
 * GCI_MissingValue:   signalOnNovalue is set and this function doesn't
 *                     support to fire a NOVALUE condition.
 * GCI_IllegalName:    "name" is illegal in terms of Rexx. Especially on
 *                     non-"symbolicAccess" the caller must provide uppercased
 *                     stem names if a stem is used.
 * GCI_RexxError:      An unexpected other error is returned by the
 *                     interpreter.
 */
GCI_result GCI_readNewRexx( void *hidden,
                            const GCI_str *name,
                            GCI_str *target,
                            int symbolicAccess,
                            int signalOnNovalue,
                            int *novalue )
{
   return readRexx( hidden,
                    name,
                    target,
                    symbolicAccess,
                    signalOnNovalue,
                    1,
                    novalue );
}

/*
 * GCI_writeRexx sets the content of one variable of name "name" to the content
 * of "value".
 *
 * symbolicAccess shall be set if normal access is expected. If this variable
 * is 0, the variable's name is treated as "tail-expanded" and any further
 * interpretation of the name isn't done by the interpreter.
 *
 * Return values:
 * GCI_OK:             Everything is fine.
 * GCI_NoMemory:       Can't allocate enough memory for the return value.
 * GCI_MissingValue:   signalOnNovalue is set and this function doesn't
 *                     support to fire a NOVALUE condition.
 * GCI_IllegalName:    "name" is illegal in terms of Rexx. Especially on
 *                     non-"symbolicAccess" the caller must provide uppercased
 *                     stem names if a stem is used.
 * GCI_RexxError:      An unexpected other error is returned by the
 *                     interpreter.
 */
GCI_result GCI_writeRexx( void *hidden,
                          const GCI_str *name,
                          const GCI_str *value,
                          int symbolicAccess )
{
   int retval;
   int Lengths[2];
   char *Strings[2];
   int allocated;

   Lengths[0] = GCI_strlen( name );
   Strings[0] = (char *) GCI_ccontent( name );
   if ( GCI_ccontent( value ) == NULL )
   {
      Lengths[1] = RX_NO_STRING;
      Strings[1] = NULL;
   }
   else
   {
      Lengths[1] = GCI_strlen( value );
      Strings[1] = (char *) GCI_ccontent( value );
   }

   retval = IfcVarPool( (tsd_t *) hidden,
                        ( symbolicAccess ) ? RX_SETSVAR : RX_SETVAR,
                        Lengths,
                        Strings,
                        &allocated );

   switch ( retval )
   {
      case RX_CODE_OK:
      case RX_CODE_NOVALUE:
         break;

      case RX_CODE_INVNAME:
         return GCI_IllegalName;

      default:
         return GCI_RexxError;
   }

   return GCI_OK;
}

/*
 * GCI_checkDefinition parses and checks the content of a stem according to
 * the GCI definition stem's syntax.
 *
 * The arguments to GCI_checkDefinition are stem_name for the stem's name and
 * tree for the position where the parsed tree shall be copied to.
 *
 * The return will be that one as for RxFuncDefine. A SYNTAX error is thrown
 * in case of errors.
 * The value of RxFuncErrMsg() and of GCI_RC is set to a description in case
 * of an error.
 */
int GCI_checkDefinition( tsd_t *TSD,
                         const streng *stem_name,
                         void **tree )
{
   GCI_result rc;
   GCI_str stem, disposition;
   GCI_treeinfo t;

   *tree = NULL;
   memset( &disposition, 0, sizeof( disposition ) );
   memset( &t, 0, sizeof( t ) );
   if ( ( rc = GCI_ParseTree( TSD,
                              GCI_migrateStreng( &stem, stem_name ),
                              &t,
                              &disposition,
                              TSD->gci_prefix ) ) != GCI_OK )
      return GCIcode2ReginaFuncCode( TSD, rc, &disposition, 0 );

   *tree = MallocTSD( sizeof( GCI_treeinfo ) );
   *((GCI_treeinfo *) *tree) = t;
   return 0;
}

/*
 * GCI_RegisterDefinedFunction will return GCI_OK here. It is a stub.
 */
GCI_result GCI_RegisterDefinedFunction( void *hidden,
                                        const GCI_str *internal,
                                        const GCI_str *library,
                                        const GCI_str *external,
                                        const GCI_treeinfo *ti )
{
   (hidden = hidden);
   (internal = internal);
   (library = library);
   (external = external);
   (ti = ti);
   return GCI_OK;
}

/*
 * GCI_remove_structure deallocates the GCI_treeinfo structure and all
 * descendants.
 */
void GCI_remove_structure( void *hidden,
                           GCI_treeinfo *gci_info )
{
   GCI_treeinfo *ti = gci_info;

   if ( ti != NULL )
   {
      if ( ti->nodes != NULL )
         GCI_free( hidden, ti->nodes );
      GCI_free( hidden, ti );
   }
}

/*
 * GCI_Dispatcher is the entry point of all GCI registered functions by the
 * user.
 *
 * The function's arguments and return value depend on its usage from case
 * to case.
 */
int GCI_Dispatcher( tsd_t *TSD,
                    PFN func,
                    void *treeinfo,
                    int Params,
                    const PRXSTRING params,
                    PRXSTRING retstr )
{
   GCI_result rc;
   GCI_str disposition, direct_retval;
   GCI_str args[GCI_REXX_ARGS];
   int i, retval;

   /*
    * This trivial test should come first to be sure not to access nonexisting
    * memory. parseTree has fixed this number.
    */
   if ( Params > GCI_REXX_ARGS )
      GCIcode2ReginaFuncCode( TSD, GCI_InternalError, NULL, 1 );

   memset( args, 0, sizeof( args ) );
   for ( i = 0; i < Params; i++ )
      GCI_migrateRxString( &args[i], &params[i] );

   memset( &disposition, 0, sizeof( disposition ) );
   memset( &direct_retval, 0, sizeof( direct_retval ) );

   rc = GCI_execute( TSD,
                     (void (*)()) func,
                     (const GCI_treeinfo *) treeinfo,
                     Params,
                     args,
                     &disposition,
                     &direct_retval,
                     TSD->gci_prefix );

   if ( rc != GCI_OK )
   {
      GCI_strfree( TSD, &direct_retval ); /* not really needed hopefully */
      GCIcode2ReginaFuncCode( TSD, rc, &disposition, 1 );
   }

   retval = assignRxString( TSD, retstr, &direct_retval );
   GCI_strfree( TSD, &direct_retval );

   if ( retval )
      exiterror( ERR_STORAGE_EXHAUSTED, 0 );

   return 0;
}
