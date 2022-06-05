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
 * This file contains types, constants and prototypes which are global to the
 * GCI system and may be used to invoke the GCI.
 */

#ifndef incl_LINK_FUNCS_ONLY
#include "embedded.h"

/*
 * GCI_result is either GCI_OK for success or a more or less understandable
 * error code.
 */
typedef enum {
   GCI_OK = 0,
   GCI_NoMemory,          /* out of memory while processing                  */
   GCI_WrongInput,        /* unexpected input, either unknown type (e.g. FIXED)
                             or illegal data (e.g. 0xFF as an integer).      */
   GCI_NumberRange,       /* Number doesn't fit in the allowed range..       */
   GCI_StringRange,       /* String too big for the defined buffer.          */
   GCI_UnsupportedType,   /* Illegal combination of type/size, e.g. INTEGER3 */
   GCI_UnsupportedNumber, /* nan, inf, etc                                   */
   GCI_BufferTooSmall,    /* Internal error: insufficent buffer size. The    */
                          /* structure is too complex.                       */
   GCI_MissingName,       /* A name in the structure is missing.             */
   GCI_MissingValue,      /* A value in the structure is missing.            */
   GCI_IllegalName,       /* The name or part of a name is illegal for the   */
                          /* interpreter                                     */
   GCI_RexxError,         /* Problems communicating with the interpreter.    */
                          /* NEVER FORGET TO EXTEND gci_helper.c:GCI_describe*/
   GCI_NoBaseType,        /* The type won't fit the requirements for basic   */
                          /* types.                                          */
   GCI_InternalError,     /* Internal, unknown problems                      */
   GCI_FunctionAlreadyKnown, /* New name already registered to Rexx          */
   GCI_LibraryNotFound,   /* The given external library can't be loaded      */
   GCI_NoLibraryFunction, /* The given external function can't be found in   */
                          /* the library                                     */
   GCI_FunctionNotFound,  /* The name was not registered in the Rexx core    */
   GCI_SyntaxError,       /* The number of arguments is wrong                */
   GCI_CoreConfused,      /* The core of GCI can't determine how to invoke   */
                          /* generic functions                               */
   GCI_ArgStackOverflow,  /* GCI's internal stack for arguments got an       */
                          /* overflow                                        */
   GCI_NestingOverflow,   /* GCI counted too many nested LIKE containers     */
   GCI_LastResult         /* currently not used.                             */
} GCI_result;

/*
 * GCI understands some basic types of which it builds more complex type
 * structures. The basic types are listed here, although we need some extra
 * types while parsing the name; these have negative values.
 */
typedef enum {
   GCI_unknown = -3,
   GCI_like = -2,
   GCI_indirect = -1,
   GCI_integer = 0,
   GCI_unsigned,
   GCI_float,
   GCI_char,
   GCI_string,
   GCI_raw,
   GCI_container,
   GCI_array
} GCI_basetype;

/*
 * GCI enumerates all basic types while parsing the type tree. Each leaf or
 * branch has an info node for the type.
 */
typedef struct {
   GCI_basetype type;
   unsigned     indirect;  /* flag "indirect" */
   unsigned     size;      /* bytes or # of elements (array, container) */
   unsigned     generated; /* generated indirection for arrays */
} GCI_parseinfo;

/*
 * GCI knows several basic call types.
 * cdecl: arguments pushed in order right to left, stack cleanup by caller
 * stdcall: arguments pushed in order right to left, stack cleanup by callee
 * pascal: arguments pushed in order left to right, stack cleanup by callee
 */
typedef enum {
      GCI_ctUnknown = 0,
      GCI_ctCdecl,
      GCI_ctStdcall,
      GCI_ctPascal
} GCI_calltype;

/*
 * GCI has a structure for the call type.
 */
typedef struct {
   GCI_calltype type;
   unsigned     as_function;     /* flag "as function" */
   unsigned     with_parameters; /* flag "as parameters" */
} GCI_callinfo;

/*
 * GCI_nodeinfo provides all informations needed for one branch or leaf in the
 * type information tree.
 */
typedef struct {
   GCI_parseinfo type;
   int           parent;         /* index of the parent's node or -1 */
   int           child;          /* index of the first child or -1 */
   int           sibling;        /* next if same depth with same parent */
   unsigned      direct_size;    /* size in bytes of this and all children */
   unsigned      indirect_size;  /* as direct_size, but for INDIRECT children */
   unsigned      direct_pos;     /* pos in buffer of this node */
   unsigned      indirect_pos;   /* as direct_pos, but for INDIRECT children */
} GCI_nodeinfo;

/*
 * GCI_treeinfo provides all informations needed for all arguments and the
 * return value for one call.
 */
typedef struct {
   GCI_callinfo  callinfo;
   GCI_nodeinfo *nodes;
   int           used;                /* used elements of nodes */
   int           max;                 /* current maximum of elements of nodes */
   int           args[GCI_REXX_ARGS]; /* ??? arguments' starting index or -1 */
   int           retval;              /* return value's starting index or -1 */
   unsigned      size;                /* size in bytes of all args */
} GCI_treeinfo;

/*
 * STEMDEPTH is the amount of space which will be allocated additionally to
 * a stem's string length for iterative access. We shall support roughly
 * a depth of 100 elements, most of a short depth. Don't forgetting the
 * separating dot we have about 250 chars.
 */
#define STEMDEPTH 250

#define elements(array) ( sizeof( array ) / sizeof( (array)[0] ) )

/*
 * gci_convert.c
 */
GCI_result GCI_string2bin( void *hidden,
                           const char *str,
                           int size,
                           void *dest,
                           int destbyte,
                           GCI_basetype type );
GCI_result GCI_bin2string( void *hidden,
                           const void *base,
                           int size,
                           char *str,
                           int *strsize,
                           GCI_basetype type );
GCI_result GCI_validate( int size,
                         GCI_basetype type,
                         int basetype );

/*
 * gci_helper.c
 */
int GCI_strlen( const GCI_str *str );
int GCI_strmax( const GCI_str *str );
char * GCI_content( GCI_str *str );
const char * GCI_ccontent( const GCI_str *str );
int GCI_streq( const GCI_str *s1,
               const GCI_str *s2 );
GCI_result GCI_strcat( GCI_str *first,
                       const GCI_str *second );
GCI_result GCI_strcats( GCI_str *first,
                        const char *second );
void GCI_strsetlen( GCI_str *str,
                    int max );
GCI_result GCI_strcpy( GCI_str *first,
                       const GCI_str *second );
GCI_str *GCI_strfromascii( GCI_str *str,
                           char *ptr,
                           int max );
const GCI_str *GCI_migratefromascii( GCI_str *str,
                                     const char *ptr );
GCI_result GCI_stralloc( void *hidden,
                         GCI_str *str,
                         unsigned size );
void GCI_strfree( void *hidden,
                  GCI_str *str );
GCI_result GCI_strdup( void *hidden,
                       GCI_str *first,
                       const GCI_str *second );
char *GCI_strtoascii( void *hidden,
                      const GCI_str *str );
void GCI_uppercase( void *hidden,
                    GCI_str *str );
void GCI_describe( GCI_str *description,
                   GCI_result rc );
void GCI_strswap( GCI_str *first,
                  GCI_str *second );

/*
 * gci_prepare.c
 */
GCI_result GCI_parsenodes( void *hidden,
                           GCI_str *base,
                           GCI_treeinfo *ti,
                           unsigned argc,
                           unsigned return_valid,
                           const char *prefixChar );

/*
 * gci_rexxbridge.c
 */
GCI_result GCI_readRexx( void *hidden,
                         const GCI_str *name,
                         GCI_str *target,
                         int symbolicAccess,
                         int signalOnNoValue,
                         int *novalue );
GCI_result GCI_readNewRexx( void *hidden,
                            const GCI_str *name,
                            GCI_str *target,
                            int symbolicAccess,
                            int signalOnNoValue,
                            int *novalue );
GCI_result GCI_writeRexx( void *hidden,
                          const GCI_str *name,
                          const GCI_str *value,
                          int symbolicAccess );
GCI_result GCI_RegisterDefinedFunction( void *hidden,
                                        const GCI_str *internal,
                                        const GCI_str *library,
                                        const GCI_str *external,
                                        const GCI_treeinfo *ti );
void GCI_remove_structure( void *hidden,
                           GCI_treeinfo *gci_info );

/*
 * gci_rxfuncdefine.c
 */
GCI_result GCI_ParseTree( void *hidden,
                          const GCI_str *stem,
                          GCI_treeinfo *gci_info,
                          GCI_str *error_disposition,
                          const char *prefixChar );
GCI_result GCI_RxFuncDefine( void *hidden,
                             const GCI_str *internal,
                             const GCI_str *library,
                             const GCI_str *external,
                             const GCI_str *stem,
                             GCI_str *error_disposition,
                             const char *prefixChar );

/*
 * gci_tree.c
 */
GCI_result GCI_parsetree( void *hidden,
                          GCI_str *base,
                          GCI_result (*callback)(int depth,
                                                 int itemnumber,
                                                 void *arg,
                                                 const GCI_parseinfo *info),
                          void *arg,
                          const char *prefixChar );

/*
 * gci_execute.c
 */
GCI_result GCI_execute( void *hidden,
                        void (*func)(),
                        const GCI_treeinfo *ti,
                        int argc,
                        const GCI_str *args,
                        GCI_str *error_disposition,
                        GCI_str *retval,
                        const char *prefixChar );

/*
 * gci_call.c
 */
GCI_result GCI_call( void *hidden,
                     void (*func)(),
                     const GCI_treeinfo *ti,
                     char *basebuf );

#endif /* incl_LINK_FUNCS_ONLY */

/*
 * gci_oslink.c (not used for a GCI which is embedded in a true interpreter)
 */
void *GCI_getLibrary( const char *libname, int libnamelen, char *buf );
void GCI_freeLibrary( void *handle );
void ( *GCI_getEntryPoint( void *handle, const char *function ) )();

