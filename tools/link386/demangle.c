/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  C++ name demangling.
*
****************************************************************************/

/*!
 * @file demangle.c
 * @brief C++ name demangling for Watcom-mangled symbols.
 *
 * Implements the parser and output engine used by the link clone to
 * turn Watcom C++ mangled names back into human-readable form.  The
 * parser walks the mangled string, invoking the output callback
 * (@c outfunPtr) with DM_* events that describe the structure of the
 * name.  Multiple drivers (simple text demangler, callback-based
 * structure extractor, and self-tests) reuse the same parser.
 */

/*!
 * @def __DIP__
 * @brief Enables the callback-driven parser mode in TEST builds.
 */
#ifdef TEST
#define __DIP__
#endif
/*!
 * @def __NO_STACK_CHECKING__
 * @brief Disables recursion-depth probes in DIP builds.
 */
#ifdef __DIP__
#define __NO_STACK_CHECKING__
#endif

#if 0 || defined(TEST) || defined(DUMP)
#include <stdio.h>
#endif
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "walloca.h"
#include "watcom.h"
#include "demangle.h"

/*!
 * @def _WCI86FAR
 * @brief Fallback for the Watcom far-pointer qualifier.
 */
#ifndef _WCI86FAR
#define _WCI86FAR
#endif

/*!
 * @def IMPORT_PREFIX_STR
 * @brief Prefix added by `__declspec(dllimport)` to imported symbols.
 */
#define IMPORT_PREFIX_STR       "__imp_"
/*!
 * @def IMPORT_PREFIX_LEN
 * @brief Length of @ref IMPORT_PREFIX_STR.
 */
#define IMPORT_PREFIX_LEN       ( sizeof( IMPORT_PREFIX_STR ) - 1 )

#undef  TRUE
#undef  FALSE
/*!
 * @def TRUE
 * @brief Boolean true used throughout the demangler.
 */
#define TRUE                    (1)
/*!
 * @def FALSE
 * @brief Boolean false used throughout the demangler.
 */
#define FALSE                   (0)
/*!
 * @def RECURSE_CHECK
 * @brief Stack bytes consumed per recursion check in @c check_recurse.
 */
#define RECURSE_CHECK           (100*sizeof(int))
/*!
 * @def AUTO_BUFFER_SIZE
 * @brief Size of the on-stack buffer used for recursive sub-names.
 */
#define AUTO_BUFFER_SIZE        80
/*!
 * @def SUPPRESS_LIMIT
 * @brief Baseline value of @c suppress_output; deeper values suppress output.
 */
#define SUPPRESS_LIMIT          2
/*!
 * @def NULL_CHAR
 * @brief C string terminator.
 */
#define NULL_CHAR               '\0'

/*!
 * @def TRUNCATED_HASH_LEN
 * @brief Number of hash characters in a truncated (compressed) mangled name.
 */
#define TRUNCATED_HASH_LEN      4

/*!
 * @def TRUNCATED_PREFIX1
 * @brief First character of a truncated (compressed) mangled name.
 */
#define TRUNCATED_PREFIX1       'T'
/*!
 * @def MANGLE_PREFIX1
 * @brief First character of a normal mangled name.
 */
#define MANGLE_PREFIX1          'W'
/*!
 * @def MANGLE_PREFIX2
 * @brief Second character of a normal mangled name.
 */
#define MANGLE_PREFIX2          '?'
/*!
 * @def PREFIX_EMBEDDED
 * @brief Introduces an embedded (recursive) mangled name.
 */
#define PREFIX_EMBEDDED         '?'
/*!
 * @def MEMBER_POINTER
 * @brief Marks a pointer-to-member encoding.
 */
#define MEMBER_POINTER          'M'
/*!
 * @def UNDERSCORE
 * @brief Literal '_' character.
 */
#define UNDERSCORE              '_'
/*!
 * @def PERIOD
 * @brief Literal '.' character.
 */
#define PERIOD                  '.'
/*!
 * @def SCOPE_PREFIX
 * @brief Introduces a scope-qualified name component.
 */
#define SCOPE_PREFIX            ':'
/*!
 * @def TEMPLATE_PREFIX
 * @brief Introduces a template-argument list.
 */
#define TEMPLATE_PREFIX         ':'
/*!
 * @def TEMPLATE_INT
 * @brief Template argument is an integer.
 */
#define TEMPLATE_INT            '0'
/*!
 * @def TEMPLATE_TYPE
 * @brief Template argument is a type.
 */
#define TEMPLATE_TYPE           '1'
/*!
 * @def POSITIVE_INT
 * @brief Marks a non-negative template integer.
 */
#define POSITIVE_INT            'Z'
/*!
 * @def NEGATIVE_INT
 * @brief Marks a negative template integer.
 */
#define NEGATIVE_INT            'Y'
/*!
 * @def OPNAME_PREFIX
 * @brief Introduces an operator-name encoding.
 */
#define OPNAME_PREFIX           '$'
/*!
 * @def SYMBOL_SUFFIX
 * @brief Terminates an identifier.
 */
#define SYMBOL_SUFFIX           '$'
/*!
 * @def MANGLE_RECURSE_SUFFIX
 * @brief Terminates a recursive mangled-name clause.
 */
#define MANGLE_RECURSE_SUFFIX   '$'
/*!
 * @def TYPE_NAME_PREFIX
 * @brief Introduces a type name.
 */
#define TYPE_NAME_PREFIX        '$'
/*!
 * @def TYPE_NAME_SUFFIX
 * @brief Terminates a type name.
 */
#define TYPE_NAME_SUFFIX        '$'
/*!
 * @def THIS_QUAL_PREFIX
 * @brief Introduces a this-qualified function type.
 */
#define THIS_QUAL_PREFIX        '.'
/*!
 * @def ARRAY_PREFIX
 * @brief Introduces an array dimension list.
 */
#define ARRAY_PREFIX            '['
/*!
 * @def ARRAY_SUFFIX
 * @brief Terminates an array dimension.
 */
#define ARRAY_SUFFIX            ']'
/*!
 * @def FUNCTION_PREFIX
 * @brief Introduces a function parameter list.
 */
#define FUNCTION_PREFIX         '('
/*!
 * @def FUNCTION_SUFFIX
 * @brief Terminates a function parameter list.
 */
#define FUNCTION_SUFFIX         ')'
/*!
 * @def OP_FUN_PREFIX
 * @brief Introduces a plain operator function.
 */
#define OP_FUN_PREFIX           'o'
/*!
 * @def REL_FUN_PREFIX
 * @brief Introduces a relational operator function.
 */
#define REL_FUN_PREFIX          'r'
/*!
 * @def SPEC_TYPE_PREFIX
 * @brief Introduces a special type (e.g. anonymous enum).
 */
#define SPEC_TYPE_PREFIX        't'
/*!
 * @def WAT_FUN_PREFIX
 * @brief Introduces a Watcom-specific internal object.
 */
#define WAT_FUN_PREFIX          'w'
/*!
 * @def ASGN_FUN_PREFIX
 * @brief Introduces an assignment operator function.
 */
#define ASGN_FUN_PREFIX         'a'

/*!
 * @typedef realloc_fn_t
 * @brief Reallocation callback used to grow the output buffer.
 */
typedef char *(*realloc_fn_t)( char *, size_t );

/*!
 * @def MAX_REPLICATE
 * @brief Number of distinct identifiers remembered for back-reference.
 */
#define MAX_REPLICATE   10
/*!
 * @struct replicate_desc
 * @brief One remembered identifier for back-referencing (digit references).
 */
typedef struct replicate_desc {
    char const      *ptr;   /*!< Identifier text. */
    size_t          len;    /*!< Identifier length in bytes. */
} replicate_desc;

// static R/W data
/*!
 * @brief User-supplied output-buffer reallocator, or NULL.
 */
static realloc_fn_t user_realloc;
/*!
 * @brief Ring buffer of identifiers remembered for back-reference.
 */
static replicate_desc replicate[MAX_REPLICATE];
/*!
 * @brief Next slot in @ref replicate to be filled.
 */
static int            next_replicate;

/*!
 * @struct output_desc
 * @brief Parser/output state shared across the whole demangle run.
 */
typedef struct output_desc {
    outfunPtr       outfun;         /*!< Output callback. */
    void            *cookie;        /*!< Opaque cookie passed to @c outfun. */
    char const      *input;         /*!< Current position in mangled input. */
    char const      *end;           /*!< End of mangled input. */
    char const      *scope_ptr;     /*!< Remembered scope-name pointer. */
    char            *output;        /*!< Output buffer, or NULL in count mode. */
    size_t          size;           /*!< Size of @c output. */
    size_t          count;          /*!< Characters emitted so far. */
    size_t          index;          /*!< Current insertion index in @c output. */
    size_t          pending_loc;    /*!< Saved index for ctor/dtor name. */
    size_t          scope_len;      /*!< Length of the remembered scope name. */
    int             suppress_output;/*!< Non-zero while output is suppressed. */
    int             scope_index;    /*!< Current scope level being extracted. */
    unsigned        ctdt_pending : 1;   /*!< Ctor/dtor name pending. */
    unsigned        cv_pending : 1;     /*!< const/volatile suffix pending. */
    unsigned        scope_name : 1;     /*!< Collecting scope names. */
    unsigned        base_name : 1;      /*!< Emitting base name only. */
    unsigned        dllimport : 1;      /*!< Name had __imp_ prefix. */
} output_desc;

// the simple demangler uses these to output & count chars in the output buffer
// the parser callback uses these to extract the structure of the name
/*!
 * @def _output1
 * @brief Invoke the output callback with a single argument.
 * @param[in] _a  DM_* event code.
 */
#define _output1( _a         ) (data->outfun)( &(data->cookie), _a,  0,  0 )
/*!
 * @def _output2
 * @brief Invoke the output callback with an event code and an integer argument.
 * @param[in] _a  DM_* event code.
 * @param[in] _b  Integer argument.
 */
#define _output2( _a, _b     ) (data->outfun)( &(data->cookie), _a, _b,  0 )
/*!
 * @def _output3
 * @brief Invoke the output callback with an event code, integer and pointer.
 * @param[in] _a  DM_* event code.
 * @param[in] _b  Integer argument.
 * @param[in] _c  Pointer argument.
 */
#define _output3( _a, _b, _c ) (data->outfun)( &(data->cookie), _a, _b, _c )

/*!
 * @struct state_desc
 * @brief Parser state passed down between type-encoding productions.
 */
typedef struct state_desc {
    size_t          prefix; /*!< Insertion index for prefixes. */
    size_t          suffix; /*!< Reset distance from the output end. */
    char            right : 1;  /*!< Right-side parens must be inserted. */
} state_desc;

/*!
 * @brief Literal '~' used to start a destructor name.
 */
static char const _WCI86FAR dtorChar = '~';
/*!
 * @brief Literal '('.
 */
static char const _WCI86FAR openParen = '(';
/*!
 * @brief Literal ')'.
 */
static char const _WCI86FAR closeParen = ')';
/*!
 * @brief Text emitted for a `signed` character type.
 */
static char const _WCI86FAR signedPrefix[] = "signed ";
/*!
 * @brief Text emitted before an operator name.
 */
static char const _WCI86FAR operatorName[] = "operator ";
/*!
 * @brief Name of the scalar `delete` operator.
 */
static char const _WCI86FAR deleteFunction[] = "delete";
/*!
 * @brief Name of the array `delete[]` operator.
 */
static char const _WCI86FAR deleteArrayFunction[] = "delete []";
/*!
 * @brief Text emitted for an anonymous enum type.
 */
static char const _WCI86FAR anonymousEnum[] = "__anonymous_enum ";
/*!
 * @brief Name of the scalar `new` operator.
 */
static char const _WCI86FAR newFunction[] = "new";
/*!
 * @brief Name of the array `new[]` operator.
 */
static char const _WCI86FAR newArrayFunction[] = "new []";
/*!
 * @brief Scope separator text.
 */
static char const _WCI86FAR scopeSeparator[] = "::";
/*!
 * @brief Template argument list opener.
 */
static char const _WCI86FAR templatePrefix[] = "<";
/*!
 * @brief Template argument list closer.
 */
static char const _WCI86FAR templateSuffix[] = ">";
/*!
 * @brief Template argument separator.
 */
static char const _WCI86FAR templateSeparator[] = ",";
/*!
 * @brief Array dimension opener.
 */
static char const _WCI86FAR arrayPrefix[] = "[";
/*!
 * @brief Array dimension closer.
 */
static char const _WCI86FAR arraySuffix[] = "]";
/*!
 * @brief Function parameter list opener.
 */
static char const _WCI86FAR functionPrefix[] = "( ";
/*!
 * @brief Function parameter list closer.
 */
static char const _WCI86FAR functionSuffix[] = ")";
/*!
 * @brief Function parameter separator.
 */
static char const _WCI86FAR functionSeparator[] = ", ";
/*!
 * @brief Based-pointer closing suffix.
 */
static char const _WCI86FAR basedSuffix[] = ") ";
/*!
 * @brief Based-string opening quote.
 */
static char const _WCI86FAR basedStrPrefix[] = "\"";
/*!
 * @brief Based-string closing quote.
 */
static char const _WCI86FAR basedStrSuffix[] = "\"";
/*!
 * @brief Text emitted for a `__self` based pointer.
 */
static char const _WCI86FAR basedSelf[] = "__self";
/*!
 * @brief Text emitted for a `__based(void)` pointer.
 */
static char const _WCI86FAR basedVoid[] = "void";

// mangled character translations
/*!
 * @struct table_t
 * @brief One entry of a mangled-character translation table.
 */
typedef struct table_t {
    char const _WCI86FAR        *string;    /*!< Translated string. */
    char                        grouping;   /*!< Flavour of character. */
} table_t;

/*!
 * @def LOWER_TABLE_LIMIT
 * @brief First character code covered by @ref translate_type_encoding.
 */
#define LOWER_TABLE_LIMIT           'A'

/*!
 * @brief Translation of one mangled character code in a type encoding.
 */
static table_t const _WCI86FAR translate_type_encoding[] = {
/*!
 * @def CHAR_UNUSED
 * @brief Character code has no type meaning.
 */
#define CHAR_UNUSED         0
/*!
 * @def CHAR_BASIC_TYPE
 * @brief Character code denotes a basic type.
 */
#define CHAR_BASIC_TYPE     1
/*!
 * @def CHAR_POINTER
 * @brief Character code denotes a pointer-like type.
 */
#define CHAR_POINTER        2
/*!
 * @def CHAR_MODIFIER
 * @brief Character code denotes a type modifier.
 */
#define CHAR_MODIFIER       3
    /* 'A' */   { "char ",      CHAR_BASIC_TYPE },
    /* 'B' */   { "float ",     CHAR_BASIC_TYPE },
    /* 'C' */   { "char ",      CHAR_BASIC_TYPE },
    /* 'D' */   { "double ",    CHAR_BASIC_TYPE },
    /* 'E' */   { "... ",       CHAR_BASIC_TYPE },
    /* 'F' */   { "far ",       CHAR_MODIFIER },
    /* 'G' */   { "far16 ",     CHAR_MODIFIER },
    /* 'H' */   { "huge ",      CHAR_MODIFIER },
    /* 'I' */   { "int ",       CHAR_BASIC_TYPE },
    /* 'J' */   { "__based(",   CHAR_MODIFIER },
    /* 'K' */   { NULL,         CHAR_UNUSED },
    /* 'L' */   { "long ",      CHAR_BASIC_TYPE },
    /* 'M' */   { "::* ",       CHAR_POINTER },
    /* 'N' */   { "near ",      CHAR_MODIFIER },
    /* 'O' */   { NULL,         CHAR_UNUSED },
    /* 'P' */   { "* ",         CHAR_POINTER },
    /* 'Q' */   { "bool ",      CHAR_BASIC_TYPE },
    /* 'R' */   { "& ",         CHAR_POINTER },
    /* 'S' */   { "short ",     CHAR_BASIC_TYPE },
    /* 'T' */   { "long double ",CHAR_BASIC_TYPE },
    /* 'U' */   { "unsigned ",  CHAR_MODIFIER },
    /* 'V' */   { "void ",      CHAR_BASIC_TYPE },
    /* 'W' */   { "wchar_t ",   CHAR_BASIC_TYPE },
    /* 'X' */   { "const ",     CHAR_MODIFIER },
    /* 'Y' */   { "volatile ",  CHAR_MODIFIER },
    /* 'Z' */   { "__int64 ",   CHAR_BASIC_TYPE },
    /* '[' */   { NULL,         CHAR_UNUSED },
    /* '\' */   { NULL,         CHAR_UNUSED },
    /* ']' */   { NULL,         CHAR_UNUSED },
    /* '^' */   { NULL,         CHAR_UNUSED },
    /* '_' */   { "",           CHAR_BASIC_TYPE } // for ctor/dtor return type
};

/*!
 * @brief Translation of one-argument operator function letters.
 */
static char _WCI86FAR * const _WCI86FAR operatorFunction[] = {
    /* A */ ">>",
    /* B */ "<<",
    /* C */ "!",
    /* D */ "[]",
    /* E */ "->",
    /* F */ "*",
    /* G */ "++",
    /* H */ "--",
    /* I */ "-",
    /* J */ "+",
    /* K */ "&",
    /* L */ "->*",
    /* M */ "/",
    /* N */ "%",
    /* O */ ",",
    /* P */ "()",
    /* Q */ "~",
    /* R */ "^",
    /* S */ "|",
    /* T */ "&&",
    /* U */ "||"
};
/*!
 * @brief Translation of one-argument relational operator function letters.
 */
static char const _WCI86FAR * const _WCI86FAR relationalFunction[] = {
    /* A */ "==",
    /* B */ "!=",
    /* C */ "<",
    /* D */ "<=",
    /* E */ ">",
    /* F */ ">="
};
/*!
 * @brief Translation of one-argument assignment operator function letters.
 */
static char const _WCI86FAR * const _WCI86FAR assignmentFunction[] = {
    /* A */ "=",
    /* B */ "*=",
    /* C */ "+=",
    /* D */ "-=",
    /* E */ "/=",
    /* F */ "%=",
    /* G */ ">>=",
    /* H */ "<<=",
    /* I */ "&=",
    /* J */ "|=",
    /* K */ "^="
};
/*!
 * @def num_elements
 * @brief Number of elements in a static array.
 * @param[in] __a  Array expression.
 */
#define num_elements( __a ) (sizeof(__a) / sizeof(__a[0]))

/*!
 * @union key_desc
 * @brief Two-character lookup key used for Watcom-object matching.
 */
typedef union key_desc {
    char        str[2];     /*!< Key bytes. */
    short       val;        /*!< Key value for fast comparison. */
} key_desc;
/*!
 * @struct assoc_desc
 * @brief One Watcom-object name entry.
 */
typedef struct assoc_desc {
    key_desc            u;      /*!< Lookup key. */
    char _WCI86FAR      *name;  /*!< Emitted name. */
} assoc_desc;

/*!
 * @brief Mapping of two-character keys to Watcom-object names.
 */
static assoc_desc const _WCI86FAR watcomObject[] = {
    { {"A*"},  "__internal" },
    { {"BI"},  "__onceonly" },
    { {"DA"},  "__arrdtorblk" },
    { {"DF"},  "__defarg" },
    { {"DI"},  "__debuginfo" },
    { {"DO"},  "__dtorobjblk" },
    { {"MP"},  "__mbrptrthunk" },
    { {"SI"},  "__staticinit" },
    { {"TH"},  "__throwblk" },
    { {"TI"},  "__typeid" },
    { {"TS"},  "__typesig" },
    { {"VA"},  "__rtti" },
    { {"VB"},  "__vbtbl" },
    { {"VF"},  "__vftbl" },
    { {"VM"},  "__vmtbl" },
    { {"VT"},  "__vfthunk" },
    { {"ST"},  "__typstattab" },
    { {"CM"},  "__stattabcmd" },
    { {"UN"},  "<unique>" },
};
/*!
 * @def MAX_WATCOM_OBJECT
 * @brief Number of entries in @ref watcomObject.
 */
#define MAX_WATCOM_OBJECT       num_elements( watcomObject )

#if 0 || defined(TEST) || defined(DUMP)
int no_errors;
unsigned errors;
#endif

/*!
 * @brief Parse a scope-qualified name.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int scoped_name( output_desc *data, state_desc *state );
/*!
 * @brief Parse a type encoding.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int type_encoding( output_desc *data, state_desc *state );
/*!
 * @brief Parse an embedded (recursive) mangled name.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int recursive_mangled_name( output_desc *data, state_desc *state );
/*!
 * @brief Terminate the output buffer (append NUL, trim trailing space).
 * @param[in,out] data  Parser/output state. Not NULL.
 * @return Length of the terminated output, excluding the NUL.
 */
static size_t terminateOutput( output_desc *data );

/*!
 * @brief Remove a single trailing space from the output buffer.
 * @param[in,out] data  Parser/output state. Not NULL.
 */
static void zapSpace( output_desc *data )
{
    if( data->output == NULL ) {
        /* count the characters mode */
        return;
    }
    if( data->index > data->size ) {
        return;
    }
    if( data->output[data->index-1] != ' ' ) {
        return;
    }
    data->count--;
    if( data->index != (data->count+1) ) {
        size_t last = min( data->size, data->count );
        if( last >= data->index ) {
            memmove( &data->output[data->index-1],
                     &data->output[data->index],
                     (last - data->index) + 1 );
        }
    }
    data->index--;
}

/*!
 * @def START_ADJUST
 * @brief Initial amount by which the output buffer is grown on overflow.
 */
#define START_ADJUST    ( 1 << 8 )

/*!
 * @brief Append one character to the output buffer.
 *
 * Grows the buffer through @ref user_realloc if needed.  Honours
 * output suppression.
 *
 * @param[in,out] data  Parser/output state. Not NULL.
 * @param[in]     c     Character to append.
 */
static void emitChar( output_desc *data, char c )
{
    size_t adjust_size;
    size_t test_size;
    char *test_realloc;

    if( data->suppress_output > SUPPRESS_LIMIT ) {
        return;
    }
    data->count++;
    if( data->output == NULL ) {
        /* count the characters mode */
        return;
    }
    if( data->count == (data->size-1) ) {
        if( user_realloc != NULL ) {
            adjust_size = START_ADJUST;
            for(;;) {
                if( adjust_size == 0 ) {
                    return;
                }
                test_size = data->size + adjust_size;
                test_realloc = user_realloc( data->output, test_size );
                if( test_realloc != NULL ) break;
                adjust_size >>= 1;
            }
            data->output = test_realloc;
            data->size = test_size;
        }
    }
    if( data->index < data->size ) {
        if( data->index < (data->count-1) ) {
            size_t last = min( data->size, data->count );
            memmove( &data->output[data->index+1],
                     &data->output[data->index],
                     (last - data->index) - 1);
        }
        data->output[data->index] = c;
        data->index++;
    }
}

/*!
 * @brief Append a NUL-terminated string to the output buffer.
 * @param[in,out] data  Parser/output state. Not NULL.
 * @param[in]     p     String to append. May be NULL.
 */
static void emitStr( output_desc *data, const char _WCI86FAR *p )
{
    if( p ) {
        while( *p ) {
            emitChar( data, *p );
            ++p;
        }
    }
}

/*!
 * @def setEmitIndex
 * @brief Set the current insertion index of the output buffer.
 * @param[in] data  Parser/output state.
 * @param[in] idx   New insertion index.
 */
#define setEmitIndex( data, idx ) ((data)->index = (idx))

/*!
 * @brief Reset the insertion index relative to the current output end.
 * @param[in,out] data              Parser/output state. Not NULL.
 * @param[in]     offset_from_end   Distance back from the current end.
 */
static void resetEmitIndex( output_desc *data, size_t offset_from_end )
{
    size_t index;

    index = data->count - offset_from_end;
    if( index < data->size ) {
        data->index = index;
    } else {
        data->index = data->size-1;
    }
}

/*!
 * @brief Enter one level of output suppression.
 * @param[in,out] data  Parser/output state. Not NULL.
 */
static void setSuppression( output_desc *data )
{
    if( data->suppress_output ) {
        data->suppress_output++;
    }
}

/*!
 * @brief Leave one level of output suppression.
 * @param[in,out] data  Parser/output state. Not NULL.
 */
static void resetSuppression( output_desc *data )
{
    if( data->suppress_output ) {
        --data->suppress_output;
    }
}

/*!
 * @brief Force output suppression above the base limit.
 * @param[in,out] data  Parser/output state. Not NULL.
 */
static void forceSuppression( output_desc *data )
{
    if( data->suppress_output <= SUPPRESS_LIMIT ) {
        data->suppress_output += SUPPRESS_LIMIT+1;
    }
}

/*!
 * @brief Undo a previous forceSuppression() call.
 * @param[in,out] data  Parser/output state. Not NULL.
 */
static void unforceSuppression( output_desc *data )
{
    if( data->suppress_output > SUPPRESS_LIMIT ) {
        data->suppress_output -= SUPPRESS_LIMIT+1;
    }
}

/*!
 * @brief Output callback invoked by the parser for each DM_* event.
 *
 * Dispatches on the event code and updates the output buffer (or the
 * character count, in count-only mode).
 *
 * @param[in,out] cookie  Pointer to the current @c output_desc.
 * @param[in]     dp      DM_* event code.
 * @param[in]     value   Integer argument (event-specific).
 * @param[in]     ptr     Pointer argument (event-specific, may be NULL).
 */
static void demangleEmit( void **cookie, dm_pts dp, int value, char const *ptr )
{
    output_desc *data = *((output_desc **)cookie);

    switch( dp ) {
    case DM_BASIC_TYPE:
    case DM_POINTER:
    case DM_ARRAY:
    case DM_FUNCTION:
    case DM_THIS_FUNCTION:
    case DM_UNMODIFIED_TYPE:
    case DM_BASED_ENCODING:
    case DM_MODIFIER_LIST:
    case DM_TEMPLATE_ARG:
    case DM_RECURSE_BEGIN:
    case DM_RECURSE_END:
    case DM_TEMPLATE_NAME:
    case DM_DESTRUCTOR:
    case DM_CONSTRUCTOR:
    case DM_OPERATOR_CONVERT:
    case DM_CTOR_DTOR_NAME:
    case DM_SCOPED_NAME:
    case DM_NAME:
    case DM_SCOPE:
    case DM_MANGLED_NAME:
    case DM_TRUNCATED_NAME:
        // ignore these for demangling
        break;
    case DM_TYPE_ENCODING:
        if( data->base_name ) {
            forceSuppression( data );
        }
        break;
    case DM_SET_INDEX:
        setEmitIndex( data, value );
        break;
    case DM_RESET_INDEX:
        resetEmitIndex( data, value );
        break;
    case DM_ZAP_SPACE:
        zapSpace( data );
        break;
    case DM_EMIT_SPACE:
        emitChar( data, ' ' );
        break;
    case DM_IDENTIFIER:
    case DM_COPY_STRING:
        if( data->base_name && data->ctdt_pending ) {
            unforceSuppression( data );
        }
        if( value == 0 ) {
            emitStr( data, ptr );
        } else {
            while( value-- ) {
                emitChar( data, *ptr++ );
            }
        }
        if( data->base_name && data->ctdt_pending ) {
            forceSuppression( data );
        }
        break;
    case DM_INTEGER:
        {
            char buff[12];
            itoa( value, buff, 10 );
            emitStr( data, buff );
        }
        break;
    case DM_ARRAY_SIZE:
        if( value != 0 ) {
            char buff[12];
            itoa( value, buff, 10 );
            emitStr( data, buff );
        }
        break;
    case DM_OPEN_PAREN:
        emitChar( data, openParen );
        break;
    case DM_CLOSE_PAREN:
        emitChar( data, closeParen );
        break;
    case DM_CHAR_ENCODING_SIGNED:
        emitStr( data, signedPrefix );
        break;
    case DM_ARRAY_PREFIX:
        emitStr( data, arrayPrefix );
        break;
    case DM_ARRAY_SUFFIX:
        emitStr( data, arraySuffix );
        break;
    case DM_FUNCTION_PREFIX:
        emitStr( data, functionPrefix );
        break;
    case DM_FUNCTION_SUFFIX:
        emitStr( data, functionSuffix );
        if( data->cv_pending ) {
            unforceSuppression( data );
        }
        break;
    case DM_FUNCTION_ARG_SEPARATOR:
        emitStr( data, functionSeparator );
        break;
    case DM_BASED_SUFFIX:
        emitStr( data, basedSuffix );
        break;
    case DM_BASED_STRING_PREFIX:
        emitStr( data, basedStrPrefix );
        break;
    case DM_BASED_SELF:
        emitStr( data, basedSelf );
        break;
    case DM_BASED_VOID:
        emitStr( data, basedVoid );
        break;
    case DM_BASED_STRING_SUFFIX:
        emitStr( data, basedStrSuffix );
        break;
    case DM_TEMPLATE_PREFIX:
        emitStr( data, templatePrefix );
        break;
    case DM_TEMPLATE_SUFFIX:
        emitStr( data, templateSuffix );
        break;
    case DM_TEMPLATE_ARG_SEPARATOR:
        emitStr( data, templateSeparator );
        break;
    case DM_ANONYMOUS_ENUM:
        emitStr( data, anonymousEnum );
        break;
    case DM_OPERATOR_PREFIX:
        emitStr( data, operatorName );
        break;
    case DM_OPERATOR_NEW:
        emitStr( data, newFunction );
        break;
    case DM_OPERATOR_NEW_ARRAY:
        emitStr( data, newArrayFunction );
        break;
    case DM_OPERATOR_DELETE:
        emitStr( data, deleteFunction );
        break;
    case DM_OPERATOR_DELETE_ARRAY:
        emitStr( data, deleteArrayFunction );
        break;
    case DM_DESTRUCTOR_CHAR:
        emitChar( data, dtorChar  );
        break;
    case DM_SCOPE_SEPARATOR:
        if( data->base_name ) {
            forceSuppression( data );
        }
        emitStr( data, scopeSeparator );
        break;
    case DM_CHAR_ENCODING:
        emitStr( data, translate_type_encoding[ value ].string );
        break;
    case DM_WATCOM_OBJECT:
        emitStr( data, watcomObject[ value ].name );
        break;
    case DM_OPERATOR_FUNCTION:
        emitStr( data, operatorFunction[ value ] );
        break;
    case DM_RELATIONAL_FUNCTION:
        emitStr( data, relationalFunction[ value ] );
        break;
    case DM_ASSIGNMENT_FUNCTION:
        emitStr( data, assignmentFunction[ value ] );
        break;
    case DM_DECLSPEC_IMPORT:
        emitStr( data, "__declspec(dllimport) " );
        break;
    default:
        #if 0 || defined(TEST)
            printf( "ERROR: demangleEmit unknown dm_pts(%d)\n", dp );
            ++errors;
        #endif
        break;
    }
}

// ==========================================================================
// everything before this point deals with output to the demangled name
// ==========================================================================
/*!
 * @brief Test whether a mangled character code belongs to a group.
 * @param[in] c         Character code (already uppercased, adjusted later).
 * @param[in] grouping  CHAR_* group to test against.
 * @retval 1  Character belongs to @p grouping.
 * @retval 0  Character does not belong, or the code is out of range.
 * @return 1 if the character belongs to the group.
 */
static int typeChar( int c, int grouping )
{
    c -= LOWER_TABLE_LIMIT;
    if( c < num_elements( translate_type_encoding ) ) {
        return( translate_type_encoding[c].grouping == grouping );
    }
    return( FALSE );
}

/*!
 * @brief Probe the stack to guard against runaway recursion.
 *
 * In `__NO_STACK_CHECKING__` builds the probe is a no-op returning 1.
 *
 * @retval 1  There is still room to recurse.
 * @return 1 if recursion may continue.
 */
static int check_recurse( void ) {
#ifdef __NO_STACK_CHECKING__
    return( 1 );
#else
    return( (int) alloca( RECURSE_CHECK ) );
#endif
}

/*!
 * @brief Read the previous input character, uppercased.
 * @param[in] data  Parser/output state. Not NULL.
 * @return The uppercased character before @c data->input.
 */
static char prevChar( output_desc *data )
{
    char c;

    c = *(data->input-1);
    return( toupper( c ) );
}

/*!
 * @brief Read the current input character, uppercased.
 *
 * Returns `NULL_CHAR` at end of input.
 *
 * @param[in] data  Parser/output state. Not NULL.
 * @return The uppercased current character, or `NULL_CHAR`.
 */
static char nextChar( output_desc *data )
{
    char c;

    c = *data->input;
    if( data->input == data->end ) {
        c = NULL_CHAR;
    } else {
        c = toupper( c );
    }
    return( c );
}

/*!
 * @brief Advance the input pointer if more data is available.
 * @param[in,out] data  Parser/output state. Not NULL.
 */
static void advanceChar( output_desc *data )
{
    if( data->input != data->end && *data->input ) {
        data->input++;
    }
}

/*!
 * @brief Case-insensitively match a literal string at the input.
 *
 * On success, advances @c data->input past the matched text.
 *
 * @param[in,out] data  Parser/output state. Not NULL.
 * @param[in]     str   Literal string to match. Not NULL.
 * @param[in]     len   Length of @p str in bytes.
 * @retval 1  String matched and consumed.
 * @retval 0  String did not match, or input was too short.
 * @return 1 on match.
 */
#ifndef __LIB__
static int strRecog( output_desc *data, char *str, unsigned len )
{
    if(( data->end - data->input ) < len ) {
        return( 0 );
    }
    if( memicmp( data->input, str, len ) != 0 ) {
        return( 0 );
    }
    data->input += len;
    return( 1 );
}
#endif

/*!
 * @brief Test whether a character can appear in a mangled identifier.
 * @param[in] c  Character to test.
 * @retval 1  Character is an identifier character.
 * @retval 0  Character is not.
 * @return 1 if @p c is alphanumeric, '_' or '.'.
 */
static int is_identifier( char c )
{
    if( isalnum( c ) || c == UNDERSCORE || c == PERIOD ) {
        return( TRUE );
    } else {
        return( FALSE );
    }
}

/*!
 * @brief Convert a single character to its base-36 digit value.
 * @param[in] c  Character to convert.
 * @return Digit value 0..35, or (unsigned)-1 for non-digits.
 */
static unsigned char_to_digit( char c )
{
    if( c >= '0' && c <= '9' ) return( c - '0' );
    if( c >= 'A' && c <= 'Z' ) return( c - 'A' + 10 );
    if( c >= 'a' && c <= 'z' ) return( c - 'a' + 10 );
    return( -1 );
}

/*!
 * @brief Convert a single character to its base-62 digit value.
 * @param[in] c  Character to convert.
 * @return Digit value 0..61, or (unsigned)-1 for non-digits.
 */
static unsigned char_to_digit_62( char c )
{
    if( c >= '0' && c <= '9' ) return( c - '0' );
    if( c >= 'A' && c <= 'Z' ) return( c - 'A' + 10 );
    if( c >= 'a' && c <= 'z' ) return( c - 'a' + 36 );
    return( -1 );
}

/*!
 * @brief Parse a base-32 number from the input.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[out]    value  Receiver for the parsed value. Not NULL.
 * @retval 1  Always returns 1.
 * @return 1.
 */
static int base_32_num( output_desc *data, int *value )
{
    unsigned    dig;
    int         v;

    v = 0;
    for( ;; ) {
        dig = char_to_digit( nextChar( data ) );
        if( dig >= 32 ) break;
        v = v * 32 + dig;
        advanceChar( data );
    }
    *value = v;
    return( TRUE );
}

/*!
 * @brief Parse a base-10 number from the input.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[out]    value  Receiver for the parsed value. Not NULL.
 * @retval 1  Always returns 1.
 * @return 1.
 */
static int base_10_num( output_desc *data, int *value )
{
    unsigned    dig;
    int         v;

    v = 0;
    for( ;; ) {
        dig = char_to_digit( nextChar( data ) );
        if( dig >= 10 ) break;
        v = v * 10 + dig;
        advanceChar( data );
    }
    *value = v;
    return( TRUE );
}

#if 0
static int base_36_2digit( output_desc *data, size_t *value )
{
    unsigned    first;
    unsigned    second;

    first = char_to_digit( nextChar( data ) );
    if( first < 36 ) {
        advanceChar( data );
        second = char_to_digit( nextChar( data ) );
        if( second < 36 ) {
            advanceChar( data );
            *value = (first * 36) + second;
            return( TRUE );
        }
    }
    return( FALSE );
}
#endif

/*!
 * @brief Parse a two-digit base-62 number from the input.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[out]    value  Receiver for the parsed value. Not NULL.
 * @retval 1  Number parsed successfully.
 * @retval 0  Number could not be parsed (input was not base-62 digits).
 * @return 1 on success.
 */
static int base_62_2digit( output_desc *data, size_t *value )
{
    unsigned    first;
    unsigned    second;

    first = char_to_digit_62( nextChar( data ) );
    if( first < 62 ) {
        advanceChar( data );
        second = char_to_digit_62( nextChar( data ) );
        if( second < 62 ) {
            advanceChar( data );
            *value = (first * 62) + second;
            return( TRUE );
        }
    }
    return( FALSE );
}

/*!
 * @brief Parse a basic type.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int basic_type( output_desc *data, state_desc *state )
{
    char  c;

    _output1( DM_BASIC_TYPE );
    c = nextChar( data );
    if( typeChar( c, CHAR_BASIC_TYPE ) ) {
        // put a signed before signed char
        if( c == 'C' && prevChar( data ) != 'U' ) {
            _output2( DM_SET_INDEX, state->prefix );
            _output1( DM_CHAR_ENCODING_SIGNED );
        }
        advanceChar( data );
        _output2( DM_SET_INDEX, state->prefix );
        _output2( DM_CHAR_ENCODING, (int)c - LOWER_TABLE_LIMIT );
        return( TRUE );
    }
    return( FALSE );
}

/*!
 * @brief Parse a pointer (or pointer-to-member) type.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int pointer( output_desc *data, state_desc *state )
{
    char  c;

    _output1( DM_POINTER );
    c = nextChar( data );
    if( typeChar( c, CHAR_POINTER ) ) {
        advanceChar( data );
        _output2( DM_SET_INDEX, state->prefix );
        _output2( DM_CHAR_ENCODING, (int)c - LOWER_TABLE_LIMIT );
        if( c == MEMBER_POINTER ) {
            c = nextChar( data );
            if( c == TYPE_NAME_PREFIX ) {
                advanceChar( data );
                if( !scoped_name( data, state ) ) {
                    return( FALSE );
                }
                c = nextChar( data );
                if( c != TYPE_NAME_PREFIX ) {
                    return( FALSE );
                }
                advanceChar( data );
                _output1( DM_ZAP_SPACE );
            }
        }
        return( TRUE );
    }
    return( FALSE );
}

/*!
 * @brief Parse a single array dimension.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int dimension( output_desc *data, state_desc *state )
{
    char c;
    int value;

    _output2( DM_RESET_INDEX, state->suffix );
    _output1( DM_ARRAY_PREFIX );
    if( base_10_num( data, &value ) ) {
        _output2( DM_ARRAY_SIZE, value );
        c = nextChar( data );
        if( c == ARRAY_SUFFIX ) {
            advanceChar( data );
            _output1( DM_ARRAY_SUFFIX );
            return( TRUE );
        }
    }
    return( FALSE );
}

/*!
 * @brief Parse an array type.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int array( output_desc *data, state_desc *state )
{
    char c;

    _output1( DM_ARRAY );
    _output2( DM_RESET_INDEX, state->suffix );
    _output1( DM_ZAP_SPACE );
    if( state->right ) {
        _output2( DM_SET_INDEX, state->prefix );
        _output1( DM_OPEN_PAREN );
        _output2( DM_RESET_INDEX, state->suffix );
        _output1( DM_CLOSE_PAREN );
        state->right = FALSE;
    }
    c = nextChar( data );
    while( c == ARRAY_PREFIX ) {
        advanceChar( data );
        if( !dimension( data, state ) ) {
            return( FALSE );
        }
        c = nextChar( data );
    }
    return( type_encoding( data, state ) );
}

/*!
 * @brief Parse a function type.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int function( output_desc *data, state_desc *state )
{
    auto state_desc new_state;
    size_t          conv_offset = 0;
    int             first_arg = 0;
    char            c;

    _output1( DM_FUNCTION );
    _output2( DM_RESET_INDEX, state->suffix );
    if( data->cv_pending ) {
        _output1( DM_EMIT_SPACE );
        conv_offset = data->index;
    } else {
        _output1( DM_ZAP_SPACE );
    }
    if( state->right ) {
        _output2( DM_SET_INDEX, state->prefix );
        _output1( DM_OPEN_PAREN );
        _output2( DM_RESET_INDEX, state->suffix );
        _output1( DM_CLOSE_PAREN );
        state->right = FALSE;
    }
    new_state = *state;
    _output1( DM_FUNCTION_PREFIX );
    setSuppression( data );
    c = nextChar( data );
    while( c != FUNCTION_SUFFIX ) {
        if( first_arg ) {
            _output2( DM_RESET_INDEX, state->suffix );
            _output1( DM_ZAP_SPACE );
            _output1( DM_FUNCTION_ARG_SEPARATOR );
        }
        new_state.prefix = data->index;
        if( !type_encoding( data, &new_state ) ) {
            return( FALSE );
        }
        first_arg = 1;
        c = nextChar( data );
    }
    advanceChar( data );
    _output2( DM_RESET_INDEX, state->suffix );
    if( !first_arg ) {
        _output1( DM_ZAP_SPACE );
    }
    resetSuppression( data );
    _output1( DM_FUNCTION_SUFFIX );
    if( data->cv_pending ) {
        _output2( DM_SET_INDEX, conv_offset );
    } else {
        _output2( DM_SET_INDEX, state->prefix );
    }
    new_state.prefix = data->index;
    if( type_encoding( data, &new_state ) ) {
        data->cv_pending = FALSE;
        return( TRUE );
    } else {
        return( FALSE );
    }
}

/*!
 * @brief Parse a this-qualified function type.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int tq_function( output_desc *data, state_desc *state )
{
    char c;
    char const *tq_ptr;
    size_t tq_len;

    _output1( DM_THIS_FUNCTION );
    tq_ptr = data->input;
    tq_len = 0;
    c = nextChar( data );
    while( typeChar( c, CHAR_MODIFIER ) ) {
        // note: doesn't handle based modifiers
        tq_len++;
        advanceChar( data );
        c = nextChar( data );
    }
    if( c == FUNCTION_PREFIX ) {
        advanceChar( data );
        if( function( data, state ) ) {
            _output2( DM_RESET_INDEX, state->suffix );
            if( tq_len > 0 ) {
                _output1( DM_EMIT_SPACE );
                while( tq_len-- ) {
                    _output2( DM_CHAR_ENCODING, (int)toupper(*tq_ptr++) - LOWER_TABLE_LIMIT );
                }
                _output1( DM_ZAP_SPACE );
            }
            return( TRUE );
        }
    }
    return( FALSE );
}

/*!
 * @brief Parse an unmodified type (no leading modifiers).
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int unmodified_type( output_desc *data, state_desc *state )
{
    char c;

    _output1( DM_UNMODIFIED_TYPE );
    c = nextChar( data );
    if( c == TYPE_NAME_PREFIX ) {
        advanceChar( data );
        if( scoped_name( data, state ) ) {
            c = nextChar( data );
            if( c == TYPE_NAME_SUFFIX ) {
                advanceChar( data );
                return( TRUE );
            }
        }
    } else if( c == FUNCTION_PREFIX ) {
        advanceChar( data );
        return( function( data, state ) );
    } else if( c == THIS_QUAL_PREFIX ) {
        advanceChar( data );
        return( tq_function( data, state ) );
    } else if( c == ARRAY_PREFIX ) {
        return( array( data, state ) );
    } else if( typeChar( c, CHAR_POINTER ) ) {
        if( pointer( data, state ) ) {
            state->right = TRUE;
            return( type_encoding( data, state ) );
        }
    } else {
        return( basic_type( data, state ) );
    }
    return( FALSE );
}

/*!
 * @brief Parse a `__based(...)` encoding.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int based_encoding( output_desc *data, state_desc *state )
{
    char   c;
    size_t len;

    _output1( DM_BASED_ENCODING );
    c = nextChar( data );
    switch( c ) {
    case 'S':
        advanceChar( data );
        _output1( DM_BASED_SELF );
        _output1( DM_BASED_SUFFIX );
        return( TRUE );
        break;
    case 'V':
        advanceChar( data );
        _output1( DM_BASED_VOID );
        _output1( DM_BASED_SUFFIX );
        return( TRUE );
        break;
    case 'L':
        advanceChar( data );
        if( base_62_2digit( data, &len ) ) {
            _output1( DM_BASED_STRING_PREFIX );
            _output3( DM_IDENTIFIER, len, data->input );
            while( len-- ) {
                advanceChar( data );
            }
            _output1( DM_BASED_STRING_SUFFIX );
            _output1( DM_BASED_SUFFIX );
            return( TRUE );
        }
        break;
    case 'A':
    case 'F':
        advanceChar( data );
        c = nextChar( data );
        if( c == TYPE_NAME_PREFIX ) {
            auto state_desc new_state;
            advanceChar( data );
            new_state = *state;
            new_state.prefix = data->index;
            new_state.suffix = data->count - data->index;
            if( scoped_name( data, &new_state ) ) {
                _output2( DM_RESET_INDEX, new_state.suffix );
                _output1( DM_ZAP_SPACE );
                _output1( DM_BASED_SUFFIX );
                return( TRUE );
            }
        }
        break;
    }
    return( FALSE );
}

/*!
 * @brief Parse a list of type modifiers.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int modifier_list( output_desc *data, state_desc *state )
{
    char c;

    _output1( DM_MODIFIER_LIST );
    c = nextChar( data );
    while( typeChar( c, CHAR_MODIFIER ) ) {
        advanceChar( data );
        if( c == 'U' && nextChar( data ) == 'A' ) {
            // don't emit unsigned before default char
        } else {
            _output2( DM_SET_INDEX, state->prefix );
            _output2( DM_CHAR_ENCODING, (int)c - LOWER_TABLE_LIMIT );
        }
        if( c == 'J' ) {
            if( !based_encoding( data, state ) ) {
                return( FALSE );
            }
        }
        c = nextChar( data );
    }
    return( TRUE );
}

/*!
 * @brief Parse a complete type encoding.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int type_encoding( output_desc *data, state_desc *state )
{
    int ret;
    char c;

    ret = FALSE;
    _output1( DM_TYPE_ENCODING );
    c = nextChar( data );
    if( typeChar( c, CHAR_MODIFIER ) ) {
        if( modifier_list( data, state ) ) {
            ret = unmodified_type( data, state );
        }
    } else {
        ret = unmodified_type( data, state );
    }
#if 0   // afs; this isn't the right place for this but we should output this
    if( ret && data->dllimport ) {
        _output2( DM_SET_INDEX, state->prefix );
        _output1( DM_DECLSPEC_IMPORT );
        data->dllimport = 0;
    }
#endif
    return( ret );
}

/*!
 * @brief Parse one template argument.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int template_arg( output_desc *data, state_desc *state )
{
    char c;

    _output1( DM_TEMPLATE_ARG );
    c = nextChar( data );
    if( c == TEMPLATE_INT ) {
        int value;
        advanceChar( data );
        if( base_32_num( data, &value ) ) {
            c = nextChar( data );
            if( c == POSITIVE_INT ) {
                advanceChar( data );
                _output2( DM_SET_INDEX, state->prefix );
                _output2( DM_INTEGER, value );
                return( TRUE );
            } else if( c == NEGATIVE_INT ) {
                advanceChar( data );
                _output2( DM_SET_INDEX, state->prefix );
                _output2( DM_INTEGER, -value );
                return( TRUE );
            }
        }
    } else if( c == TEMPLATE_TYPE ) {
        advanceChar( data );
        return( type_encoding( data, state ) );
    } else if( c == PREFIX_EMBEDDED ) {
        advanceChar( data );
        if( check_recurse() ) {
            return( recursive_mangled_name( data, state ) );
        }
    }
    return( FALSE );
}

/*!
 * @brief Parse a template argument list.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int template_name( output_desc *data, state_desc *state )
{
    auto state_desc new_state;
    int             first_arg = 0;
    char            c;

    _output1( DM_TEMPLATE_NAME );
    new_state = *state;
    _output2( DM_RESET_INDEX, state->suffix );
    _output1( DM_TEMPLATE_PREFIX );
    c = nextChar( data );
    while( c == TEMPLATE_INT || c == TEMPLATE_TYPE || c == PREFIX_EMBEDDED ) {
        if( first_arg ) {
            _output2( DM_RESET_INDEX, state->suffix );
            _output1( DM_ZAP_SPACE );
            _output1( DM_TEMPLATE_ARG_SEPARATOR );
        }
        new_state.prefix = data->index;
        if( !template_arg( data, &new_state ) ) {
            return( FALSE );
        }
        first_arg = 1;
        c = nextChar( data );
    }
    _output2( DM_RESET_INDEX, state->suffix );
    if( !first_arg ) {
        _output1( DM_ZAP_SPACE );
    }
    _output1( DM_TEMPLATE_SUFFIX );
    return( TRUE );
}

/*!
 * @brief Parse a Watcom-specific internal object name.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int watcom_object( output_desc *data, state_desc *state )
{
    key_desc srch;
    int      i;
    size_t   len;
    char     c;

    c = nextChar( data );
    if( c == 'A' ) {
        advanceChar( data );
        advanceChar( data );
        _output2( DM_SET_INDEX, state->prefix );
        _output2( DM_WATCOM_OBJECT, 0 );
    } else {
        srch.str[0] = c;
        advanceChar( data );
        c = nextChar( data );
        srch.str[1] = c;
        advanceChar( data );
        for( i = 1 ; i < MAX_WATCOM_OBJECT ; i++ ) {
            if( srch.val == watcomObject[i].u.val ) {
                _output2( DM_SET_INDEX, state->prefix );
                _output2( DM_WATCOM_OBJECT, i );
                break;
            }
        }
        // check to make sure we found something
        if( i == MAX_WATCOM_OBJECT ) {
            _output2( DM_SET_INDEX, state->prefix );
            _output2( DM_WATCOM_OBJECT, 0 );
        }
    }
    if( base_62_2digit( data, &len ) ) {
        while( len-- ) {
            advanceChar( data );
        }
        c = nextChar( data );
        if( c == TYPE_NAME_SUFFIX ) {
            advanceChar( data );
            return( TRUE );
        }
    }
    return( FALSE );
}

/*!
 * @brief Parse a special type name (currently only anonymous enums).
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int special_type_names( output_desc *data, state_desc *state )
{
    char c;

    c = nextChar( data );
    if( c == 'E' ) {
        advanceChar( data );
        _output2( DM_SET_INDEX, state->prefix );
        _output1( DM_ANONYMOUS_ENUM );
        for(;;) {
            c = nextChar( data );
            if( c == NULL_CHAR ) {
                return( FALSE );
            }
            advanceChar( data );
            if( c == TYPE_NAME_SUFFIX ) {
                return( TRUE );
            }
        }
    }
    return( FALSE );
}

/*!
 * @brief Parse a plain operator function name.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int operator_function( output_desc *data, state_desc *state )
{
    char c;

    c = nextChar( data ) - LOWER_TABLE_LIMIT;
    if( c < num_elements( operatorFunction ) ) {
        advanceChar( data );
        _output2( DM_SET_INDEX, state->prefix );
        _output1( DM_OPERATOR_PREFIX );
        _output2( DM_OPERATOR_FUNCTION, (int)c );
        return( TRUE );
    }
    return( FALSE );
}

/*!
 * @brief Parse a relational operator function name.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int relational_function( output_desc *data, state_desc *state )
{
    char c;

    c = nextChar( data ) - LOWER_TABLE_LIMIT;
    if( c < num_elements( relationalFunction ) ) {
        advanceChar( data );
        _output2( DM_SET_INDEX, state->prefix );
        _output1( DM_OPERATOR_PREFIX );
        _output2( DM_RELATIONAL_FUNCTION, (int)c );
        return( TRUE );
    }
    return( FALSE );
}

/*!
 * @brief Parse an assignment operator function name.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int assignment_function( output_desc *data, state_desc *state )
{
    char c;

    c = nextChar( data ) - LOWER_TABLE_LIMIT;
    if( c < num_elements( assignmentFunction ) ) {
        advanceChar( data );
        _output2( DM_SET_INDEX, state->prefix );
        _output1( DM_OPERATOR_PREFIX );
        _output2( DM_ASSIGNMENT_FUNCTION, (int)c );
        return( TRUE );
    }
    return( FALSE );
}

/*!
 * @brief Parse an `operator new` / `operator new[]` name.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int op_new( output_desc *data, state_desc *state )
{
    if( nextChar( data ) == 'W' ) {
        advanceChar( data );
        _output2( DM_SET_INDEX, state->prefix );
        _output1( DM_OPERATOR_PREFIX );
        _output1( DM_OPERATOR_NEW );
        return( TRUE );
    }
    if( nextChar( data ) == 'A' ) {
        advanceChar( data );
        _output2( DM_SET_INDEX, state->prefix );
        _output1( DM_OPERATOR_PREFIX );
        _output1( DM_OPERATOR_NEW_ARRAY );
        return( TRUE );
    }
    return( FALSE );
}

/*!
 * @brief Parse a destructor or `operator delete` name.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int dtor_or_delete( output_desc *data, state_desc *state )
{
    char c;

    c = nextChar( data );
    switch( c ) {
    case 'T':
        advanceChar( data );
        _output2( DM_SET_INDEX, state->prefix );
        _output1( DM_DESTRUCTOR_CHAR );
        _output1( DM_DESTRUCTOR );
        data->pending_loc = data->count - data->index;
        data->ctdt_pending = TRUE;
        return( TRUE );
    case 'L':
        advanceChar( data );
        _output2( DM_SET_INDEX, state->prefix );
        _output1( DM_OPERATOR_PREFIX );
        _output1( DM_OPERATOR_DELETE );
        return( TRUE );
    case 'A':
        advanceChar( data );
        _output2( DM_SET_INDEX, state->prefix );
        _output1( DM_OPERATOR_PREFIX );
        _output1( DM_OPERATOR_DELETE_ARRAY );
        return( TRUE );
    }
    return( FALSE );
}

/*!
 * @brief Parse a constructor or conversion-operator name.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int ctor_or_convert( output_desc *data, state_desc *state )
{
    char c;

    c = nextChar( data );
    if( c == 'T' ) {
        advanceChar( data );
        _output1( DM_CONSTRUCTOR );
        data->pending_loc = data->count - data->index;
        data->ctdt_pending = TRUE;
        return( TRUE );
    } else if( c == 'V' ) {
        advanceChar( data );
        _output2( DM_SET_INDEX, state->prefix );
        _output1( DM_OPERATOR_PREFIX );
        _output1( DM_OPERATOR_CONVERT );
        data->cv_pending = TRUE;
        return( TRUE );
    }
    return( FALSE );
}

/*!
 * @brief Parse an operator name (dispatch on the operator kind letter).
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int op_name( output_desc *data, state_desc *state )
{
    char c;

    c = tolower( nextChar( data ) );
    switch( c ) {
    case 'c':
        advanceChar( data );
        return( ctor_or_convert( data, state ) );
        break;
    case 'd':
        advanceChar( data );
        return( dtor_or_delete( data, state ) );
        break;
    case 'n':
        advanceChar( data );
        return( op_new( data, state ) );
        break;
    case ASGN_FUN_PREFIX:
        advanceChar( data );
        return( assignment_function( data, state ) );
        break;
    case REL_FUN_PREFIX:
        advanceChar( data );
        return( relational_function( data, state ) );
        break;
    case OP_FUN_PREFIX:
        advanceChar( data );
        return( operator_function( data, state ) );
        break;
    case SPEC_TYPE_PREFIX:
        advanceChar( data );
        return( special_type_names( data, state ) );
        break;
    case WAT_FUN_PREFIX:
        advanceChar( data );
        return( watcom_object( data, state ) );
        break;
    }
    return( FALSE );
}

/*!
 * @brief Parse a plain identifier.
 *
 * Identifiers come in two flavours:
 * - A digit `0`..`9`, referring back to a previously remembered
 *   identifier (up to @ref MAX_REPLICATE distinct ones).
 * - A run of identifier characters terminated by @ref SYMBOL_SUFFIX.
 *   The identifier is remembered for later digit references.
 *
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int name( output_desc *data, state_desc *state )
{
    int c;

    c = nextChar( data );
    if( isdigit( c ) ) {
        size_t len;
        char const *id;

        c = c - '0';
        // in case C++ compiler and demangler don't match; don't crash
        if( c >= next_replicate ) {
            id = "@";
            len = 1;
        } else {
            id = replicate[c].ptr;
            len = replicate[c].len;
        }
        if( data->ctdt_pending ) {
            _output2( DM_RESET_INDEX, data->pending_loc );
            _output1( DM_CTOR_DTOR_NAME );
            _output3( DM_IDENTIFIER, len, id );
            data->ctdt_pending = FALSE;
        }
        if( data->scope_name && data->scope_index == 0 ) {
            data->scope_name = FALSE;
            data->scope_ptr = id;
            data->scope_len = len;
        }
        _output2( DM_SET_INDEX, state->prefix );
        _output3( DM_IDENTIFIER, len, id );
        _output1( DM_EMIT_SPACE );
        advanceChar( data );
        return( TRUE );
    } else {
        char const *ptr = data->input;
        size_t  len = 0;
        while( is_identifier( c ) ) {
            len++;
            advanceChar( data );
            c = nextChar( data );
        }
        if( data->scope_name && data->scope_index == 0 ) {
            data->scope_name = FALSE;
            data->scope_ptr = ptr;
            data->scope_len = len;
        }
        if( c == SYMBOL_SUFFIX ) {
            advanceChar( data );
            if( data->ctdt_pending ) {
                _output2( DM_RESET_INDEX, data->pending_loc );
                _output1( DM_CTOR_DTOR_NAME );
                _output3( DM_IDENTIFIER, len, ptr );
                data->ctdt_pending = FALSE;
            }
            _output2( DM_SET_INDEX, state->prefix );
            _output3( DM_IDENTIFIER, len, ptr );
            _output1( DM_EMIT_SPACE );
            if( next_replicate < MAX_REPLICATE ) {
                replicate[next_replicate].ptr = ptr;
                replicate[next_replicate].len = len;
                next_replicate++;
            }
            return( TRUE );
        }
    }
    return( FALSE );
}
/*!
 * @brief Parse a symbol name (plain name or operator name).
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int sym_name( output_desc *data, state_desc *state )
{
    _output1( DM_NAME );
    if( nextChar( data ) == OPNAME_PREFIX ) {
        advanceChar( data );
        return( op_name( data, state ) );
    } else {
        return( name( data, state ) ) ;
    }
}

/*!
 * @brief Parse one component of a scope-qualified name.
 *
 * Handles embedded recursive names, template names, and ordinary
 * names.  @p symbol_length is updated to reflect the length of the
 * emitted symbol name.
 *
 * @param[in,out] data           Parser/output state. Not NULL.
 * @param[in,out] state          Type-encoding state. Not NULL.
 * @param[in,out] symbol_length  Current symbol name length. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int scope( output_desc *data, state_desc *state, size_t *symbol_length )
{
    char c;

    _output1( DM_SCOPE );
    if( data->scope_name && data->scope_index > 0 ) {
        data->scope_index--;
    }
    c = nextChar( data );
    if( c == PREFIX_EMBEDDED ) {
        advanceChar( data );
        if( check_recurse() ) {
            return( recursive_mangled_name( data, state ) );
        }
    } else if( c == TEMPLATE_PREFIX ) {
        auto state_desc new_state;
        advanceChar( data );
        new_state = *state;
        new_state.suffix = data->count - state->prefix - *symbol_length;
        return( template_name( data, &new_state ) );
    } else {
        int rc = sym_name( data, state );
        *symbol_length = data->index - state->prefix - 1;
        return( rc );
    }
    return( FALSE );
}

/*!
 * @brief Parse a scope-qualified name (`a::b::c`, possibly templated).
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int scoped_name( output_desc *data, state_desc *state )
{
    size_t symbol_length;

    _output1( DM_SCOPED_NAME );
    if( sym_name( data, state ) ) {
        symbol_length = data->index - state->prefix - 1;
        while( nextChar( data ) == SCOPE_PREFIX ) {
            _output2( DM_SET_INDEX, state->prefix );
            if( *(data->input + 1) != TEMPLATE_PREFIX ) {
                _output1( DM_SCOPE_SEPARATOR );
            }
            advanceChar( data );
            if( !scope( data, state, &symbol_length ) ) {
                return( FALSE );
            }
            _output1( DM_ZAP_SPACE );
        }
        return( TRUE );
    }
    return( FALSE );
}

/*!
 * @brief Parse a fully-mangled name: scoped name followed by type encoding.
 * @param[in,out] data  Parser/output state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure.
 * @return 1 on success.
 */
static int mangled_name( output_desc *data )
{
    auto state_desc new_state;

    _output1( DM_MANGLED_NAME );
    new_state.prefix = 0;
    new_state.suffix = 0;
    new_state.right = FALSE;
    if( scoped_name( data, &new_state ) ) {
        _output2( DM_RESET_INDEX, 0 );
        _output1( DM_ZAP_SPACE );
        return( type_encoding( data, &new_state ) );
    }
    return( FALSE );
}

/*!
 * @brief Parse a full mangled name, including the `W?`/`T?` prefix.
 *
 * On success, @c data->input points just past the mangled name and
 * @c data->dllimport is set if the name had an `__imp_` prefix.
 *
 * @param[in,out] data  Parser/output state. Not NULL.
 * @retval 1  Parsed successfully.
 * @retval 0  Parse failure (not a mangled name).
 * @return 1 on success.
 */
#ifndef __LIB__
static int full_mangled_name( output_desc *data )
{
    unsigned advances;

    advances = 1;
    if( strRecog( data, IMPORT_PREFIX_STR, IMPORT_PREFIX_LEN ) ) {
        data->dllimport = 1;
    }
    switch( nextChar( data ) ) {
    case TRUNCATED_PREFIX1:
        advances += TRUNCATED_HASH_LEN;
        _output1( DM_TRUNCATED_NAME );
        /* fall through */
    case MANGLE_PREFIX1:
        advanceChar( data );
        if( nextChar( data ) == MANGLE_PREFIX2 ) {
            for( ; advances != 0; --advances ) {
                advanceChar( data );
                if( nextChar( data ) == NULL_CHAR ) {
                    return( FALSE );
                }
            }
            return( mangled_name( data ) );
        }
        break;
    }
    return( FALSE );
}

/*!
 * @brief Demangle the name described by @c data->input into @c data->output.
 * @param[in,out] data  Parser/output state. Not NULL.
 */
static void do_demangle( output_desc *data )
{
    #if 0 || defined(TEST)
    char const *input = data->input;
    #endif

    if( !full_mangled_name( data ) ) {
        #if 0 || defined(TEST)
        if( ! no_errors ) {
            ++errors;
            printf( "ERROR: full_mangled_name failed\n" );
            printf( "in:-->%s<--\n", input );
            printf( "remaining:-->%s<--\n", data->input );
            terminateOutput( data );
            printf( "output:-->%s<--\n", data->output );
        }
        #endif
    } else {
        #if 0 || defined(TEST)
            if( nextChar( data ) != NULL_CHAR ) {
                ++errors;
                printf( "ERROR: full_mangled_name failed to consume all\n" );
                printf( "-->%s<--\n", input );
            }
        #endif
    }
}

/*!
 * @brief Copy an unmangled name verbatim into the output buffer.
 * @param[in,out] data  Parser/output state. Not NULL.
 */
static void do_copy( output_desc *data )
{
    char const *ptr;
    int len;
    char c;

    ptr = data->input;
    len = 0;
    c = nextChar( data );
    advanceChar( data );
    while( c ) {
        len++;
        c = nextChar( data );
        advanceChar( data );
    }
    _output3( DM_COPY_STRING, len, ptr );
}
#endif

/*!
 * @brief Reset the module-level demangler state before a new run.
 * @param[in] reallocator  Output-buffer reallocator, or NULL.
 */
static void init_globals( realloc_fn_t reallocator )
{
    next_replicate = 0;
    user_realloc = reallocator;
}

/*!
 * @brief Initialise an @c output_desc for one demangle run.
 * @param[out] data    Structure to initialise. Not NULL.
 * @param[in]  ofn     Output callback. Not NULL.
 * @param[in]  cookie  Opaque cookie passed to @p ofn.
 * @param[in]  input   Mangled input. Not NULL.
 * @param[in]  len     Input length, or 0 to run until NUL.
 * @param[in]  output  Output buffer, or NULL for count-only mode.
 * @param[in]  size    Size of @p output.
 */
static void init_descriptor( output_desc *data,
                             outfunPtr ofn, void *cookie,
                             char const *input, size_t len,
                             char *output, size_t size )
{
    data->outfun = ofn;
    data->cookie = cookie;
    data->end = NULL;
    if( len != 0 ) {
        /* length of mangled name is known */
        data->end = input + len;
    }
    data->input = input;
    data->output = output;
    data->scope_ptr = NULL;
    data->size = size;
    data->count = 0;
    data->index = 0;
    data->pending_loc = 0;
    data->scope_len = 0;
    data->suppress_output = 1;
    data->scope_index = 0;
    data->ctdt_pending = 0;
    data->cv_pending = 0;
    data->scope_name = 0;
    data->base_name = 0;
    data->dllimport = 0;
}

/*!
 * @brief Terminate the output buffer.
 *
 * Trims a trailing space and writes a NUL terminator.  In count-only
 * mode, just returns the accumulated count.
 *
 * @param[in,out] data  Parser/output state. Not NULL.
 * @return The final output length, excluding the NUL.
 */
static size_t terminateOutput( output_desc *data )
{
    size_t outlen;

    outlen = data->count;
    if( data->output != NULL ) {
        /* name may have been truncated */
        if( outlen >= data->size ) {
            outlen = data->size-1;
        }
        if( outlen > 0 ) {
            if( data->output[outlen-1] == ' ' ) {
                --outlen;
            }
        }
        data->output[outlen] = '\0';
    }
    /* size does not include '\0' */
    return( outlen );
}

/*!
 * @brief Demangle an embedded mangled name into a caller-supplied buffer.
 *
 * Used for recursive names inside template arguments and pointer-to-
 * member types.  Uses a NULL user reallocator, so the buffer must be
 * large enough.
 *
 * @param[in]  input   Mangled sub-name. Not NULL.
 * @param[out] output  Destination buffer. Not NULL.
 * @param[in]  size    Size of @p output.
 * @return Number of input bytes consumed.
 */
static size_t demangle_recursive( char const *input, char *output, size_t size )
{
    realloc_fn_t save_reallocator;
    auto output_desc data;

    save_reallocator = user_realloc;
    user_realloc = NULL;
    init_descriptor( &data, &demangleEmit, &data, input, 0, output, size );
    if( !mangled_name( &data ) ) {
        #if 0 || defined(TEST)
            ++errors;
            printf( "ERROR: mangled_name failed\n" );
            printf( "-->%s<--\n", input );
        #endif
    }
    terminateOutput( &data );
    user_realloc = save_reallocator;
    return( data.input - input );
}

/*!
 * @brief Demangle an embedded recursive mangled name into a temp buffer.
 * @param[in,out] data   Parser/output state. Not NULL.
 * @param[in,out] state  Type-encoding state. Not NULL.
 * @retval 1  Always returns 1.
 * @return 1.
 */
static int recursive_mangled_name( output_desc *data, state_desc *state )
{
    auto char buff[AUTO_BUFFER_SIZE];

    _output1( DM_RECURSE_BEGIN );
    data->input += demangle_recursive( data->input, buff, sizeof( buff ) );
    _output1( DM_RECURSE_END );
    _output2( DM_SET_INDEX, state->prefix );
    _output3( DM_COPY_STRING, 0, buff );
    return( TRUE );
}

/*!
 * @brief Demangle a C++ type encoding.
 * @param[in]  input   Mangled C++ type. Not NULL.
 * @param[in]  len     Length of @p input, or 0 to run until NUL.
 * @param[out] output  Destination buffer. Not NULL.
 * @param[in]  size    Size of @p output.
 * @return Length of the terminated output, excluding the NUL.
 */
#ifdef __LIB__
_WCRTLINK
#endif
size_t __demangle_t(                            // DEMANGLE A C++ TYPE
    char const *input,                          // - mangled C++ type
    size_t len,                                 // - length of mangled type
    char *output,                               // - for demangled C++ type
    size_t size )                               // - size of output buffer
{
    size_t outlen;
    auto output_desc data;
    auto state_desc new_state;

    init_globals( NULL );
    init_descriptor( &data, &demangleEmit, &data, input, len, output, size );
    data.suppress_output = 0;
    new_state.prefix = 0;
    new_state.suffix = 0;
    new_state.right = FALSE;
    type_encoding( &data, &new_state );
    outlen = terminateOutput( &data );
    /* size does not include '\0' */
    return( outlen );
}

/*!
 * @brief Demangle a C++ mangled name into a fixed-size buffer.
 * @param[in]  input   Mangled C++ name. Not NULL.
 * @param[in]  len     Length of @p input, or 0 to run until NUL.
 * @param[out] output  Destination buffer. Not NULL.
 * @param[in]  size    Size of @p output.
 * @return Length of the terminated output, excluding the NUL.
 */
#ifndef __LIB__
size_t __demangle_l(                            // DEMANGLE A C++ NAME
    char const *input,                          // - mangled C++ name
    size_t len,                                 // - length of mangled name
    char *output,                               // - for demangled C++ name
    size_t size )                               // - size of output buffer
{
    int                 mangled;
    size_t              outlen;
    auto output_desc    data;

    init_globals( NULL );
    mangled = __is_mangled( input, len );
    init_descriptor( &data, &demangleEmit, &data, input, len, output, size );
    if( mangled ) {
        do_demangle( &data );
    } else {
        do_copy( &data );
    }
    outlen = terminateOutput( &data );
    /* size does not include '\0' */
    return( outlen );
}

/*!
 * @brief Demangle a C++ mangled name into a reallocatable buffer.
 * @param[in]     input    Mangled C++ name. Not NULL.
 * @param[in]     len      Length of @p input, or 0 to run until NUL.
 * @param[in,out] output   Buffer pointer. May be NULL on entry.
 * @param[in]     size     Current size of @p *output.
 * @param[in]     realloc  Buffer reallocator. Not NULL.
 * @return Length of the terminated output, excluding the NUL.
 */
size_t __demangle_r(                            // DEMANGLE A C++ NAME
    char const *input,                          // - mangled C++ name
    size_t len,                                 // - length of mangled name
    char **output,                              // - for demangled C++ name
    size_t size,                                // - size of output buffer
    char * (*realloc)( char *, size_t ) )       // - size adjuster for output
{
    char                *output_buff;
    int                 mangled;
    size_t              outlen;
    auto output_desc    data;

    init_globals( realloc );
    mangled = __is_mangled( input, len );
    output_buff = NULL;
    if( output != NULL ) {
        output_buff = *output;
    }
    init_descriptor( &data, &demangleEmit, &data, input, len, output_buff, size );
    data.suppress_output = 0;
    if( mangled ) {
        do_demangle( &data );
    } else {
        do_copy( &data );
    }
    outlen = terminateOutput( &data );
    if( output != NULL ) {
        *output = data.output;
    }
    /* size does not include '\0' */
    return( outlen );
}

/*!
 * @brief Extract one component of a C++ scope-qualified name.
 * @param[in]  input  Mangled C++ name. Not NULL.
 * @param[in]  len    Length of @p input, or 0 to run until NUL.
 * @param[in]  index  Scope component index (0 = innermost name).
 * @param[out] scope  Receiver for the scope component pointer. Not NULL.
 * @param[out] size   Receiver for the scope component length. Not NULL.
 * @retval 1  Scope component found.
 * @retval 0  Name is not mangled, or index out of range.
 * @return 1 on success.
 */
int __scope_name(                               // EXTRACT A C++ SCOPE
    char const *input,                          // - mangled C++ name
    size_t len,                                 // - length of mangled name
    unsigned index,                             // - scope wanted
    char const **scope,                         // - location of name
    size_t *size )                              // - size of output buffer
{                                               // returns TRUE on success
    int                 mangled;
    auto output_desc    data;

    init_globals( NULL );
    *scope = NULL;
    *size = 0;
    mangled = __is_mangled( input, len );
    if( !mangled ) {
        return( FALSE );
    }
    init_descriptor( &data, &demangleEmit, &data, input, len, NULL, 0 );
    data.scope_name = TRUE;
    data.scope_index = index;
    do_demangle( &data );
    if( data.scope_len != 0 ) {
        *scope = data.scope_ptr;
        *size = data.scope_len;
        return( TRUE );
    }
    return( FALSE );
}

/*!
 * @brief Return the number of bytes to skip to reach the mangled body.
 *
 * Recognises the `W?`, `T?` and `__imp_` prefixes.  Returns 0 if the
 * name is not mangled.
 *
 * @param[in] name  Symbol name. Not NULL.
 * @param[in] len   Symbol name length (currently ignored).
 * @return Offset of the mangled body, or 0 if not mangled.
 */
int __is_mangled( char const *name, size_t len )
{
    int offset;
    len = len;

    offset = 2;
    if( name[0] == '_' && memicmp( name, IMPORT_PREFIX_STR, IMPORT_PREFIX_LEN ) == 0 ) {
        name += IMPORT_PREFIX_LEN;
        offset += IMPORT_PREFIX_LEN;
    }
    switch( name[0] ) {
    case TRUNCATED_PREFIX1:
        offset += TRUNCATED_HASH_LEN;
        /* fall through */
    case MANGLE_PREFIX1:
        if( name[1] == MANGLE_PREFIX2 ) {
            return( offset );
        }
        break;
    }
    return( 0 );
}

/*!
 * @brief Classify a mangled symbol body as internal, ctor, dtor or plain.
 * @param[in] n  Mangled body (after the `W?`/`T?` prefix). Not NULL.
 * @return One of @c __MANGLED_INTERNAL, @c __MANGLED_CTOR,
 *         @c __MANGLED_DTOR or @c __MANGLED.
 */
static int checkInternal( char const *n )
{
    if( n[0] == '.' || (n[0] == OPNAME_PREFIX && tolower(n[1]) == WAT_FUN_PREFIX)) {
        return( __MANGLED_INTERNAL );
    }
    if( n[0] == OPNAME_PREFIX && tolower(n[1]) == 'c' && tolower(n[2]) == 't' ) {
        return( __MANGLED_CTOR );
    }
    if( n[0] == OPNAME_PREFIX && tolower(n[1]) == 'd' && tolower(n[2]) == 't' ) {
        return( __MANGLED_DTOR );
    }
    return( __MANGLED );
}

/*!
 * @brief Classify a mangled symbol body.
 *
 * Same as __is_mangled_internal() but returns a classification code
 * instead of an offset.
 *
 * @param[in] name  Symbol name. Not NULL.
 * @param[in] len   Symbol name length (currently ignored).
 * @return One of the @c __MANGLED_* / @c __NOT_MANGLED constants.
 */
int __is_mangled_internal( char const *name, size_t len )
{
    unsigned offset;
    len = len;

    offset = 2;
    switch( name[0] ) {
    case TRUNCATED_PREFIX1:
        offset += TRUNCATED_HASH_LEN;
        /* fall through */
    case MANGLE_PREFIX1:
        if( name[1] == MANGLE_PREFIX2 ) {
            return( checkInternal( name + offset ) );
        }
        break;
    }
    return( __NOT_MANGLED );
}

/*!
 * @brief Locate the unmangled base name within a mangled symbol name.
 *
 * @param[in]  name  Symbol name. Not NULL.
 * @param[in]  len   Symbol name length, or 0 to run until NUL.
 * @param[out] base  Receiver for the base-name pointer. Not NULL.
 * @param[out] size  Receiver for the base-name length. Not NULL.
 * @retval 1  Name is mangled; @p base and @p size updated.
 * @retval 0  Name is not mangled.
 * @return 1 if the name is mangled.
 */
int __unmangled_name(                           // FIND UNMANGLED BASE NAME
    char const *name,                           // - mangled C++ name
    size_t len,                                 // - length of mangled name
    char const **base,                          // - location of base name
    size_t *size )                              // - size of base name
{                                               // return TRUE if name mangled
    char const *end;
    int   mangled;

    *size = 0;
    *base = name;
    if( len == 0 ) {
        len = strlen( name );
    }
    end = name + len;
    mangled = __is_mangled( name, len );
    if( mangled ) {
        name += mangled; // skip magic cookie
        *base = name;
        if( *name == OPNAME_PREFIX ) {
            (*size) += 3;
            name++;
            if( tolower(*name) == WAT_FUN_PREFIX ) {
                (*size) += 1;
            }
        } else {
            for( ; name != end ; ++name ) {
                if( *name == SYMBOL_SUFFIX ) break;
                (*size)++;
            }
        }
        return( TRUE );
    }
    *size = len;
    return( FALSE );
}



/*!
 * @brief Demangle only the base name of a C++ mangled name.
 * @param[in]  input   Mangled C++ name. Not NULL.
 * @param[in]  len     Length of @p input, or 0 to run until NUL.
 * @param[out] output  Destination buffer. Not NULL.
 * @param[in]  size    Size of @p output.
 * @return Length of the terminated output, excluding the NUL.
 */
size_t __demangled_basename(                    // CREATE DEMANGLED BASE NAME
    char const *input,                          // - mangled C++ name
    size_t len,                                 // - length of mangled name
    char *output,                               // - for demangled C++ name
    size_t size )                               // - size of output buffer
{                                               // return len of output
    int                 mangled;
    size_t              outlen;
    auto output_desc    data;

    init_globals( NULL );
    mangled = __is_mangled( input, len );
    init_descriptor( &data, &demangleEmit, &data, input, len, output, size );
    data.suppress_output = 0;
    data.base_name = 1;
    if( mangled ) {
        do_demangle( &data );
    } else {
        do_copy( &data );
    }
    outlen = terminateOutput( &data );
    /* size does not include '\0' */
    return( outlen );
}

#ifdef __DIP__

/*!
 * @brief Parse a mangled name, invoking a callback for each DM_* event.
 *
 * Used by clients that want the structural events of the name rather
 * than a rendered string.
 *
 * @param[in] input   Mangled C++ name. Not NULL.
 * @param[in] len     Length of @p input, or 0 to run until NUL.
 * @param[in] cookie  Opaque cookie passed to @p ofn.
 * @param[in] ofn     Output callback. Not NULL.
 */
void __parse_mangled_name(                      // PARSE MANGLED NAME
    char const *input,                          // - mangled C++ name
    size_t len,                                 // - length of mangled name
    void *cookie,                               // - data to carry around
    outfunPtr ofn )                             // - function to invoke
{
    int                 mangled;
    auto output_desc    data;

    init_globals( NULL );
    mangled = __is_mangled( input, len );
    init_descriptor( &data, ofn, cookie, input, len, NULL, 0 );
    data.suppress_output = 0;
    if( mangled ) {
        do_demangle( &data );
    } else {
        do_copy( &data );
    }
}


/*!
 * @brief Mangle an operator token back into the encoded form.
 *
 * Recognises the plain, relational and assignment operator
 * vocabularies.  The result is `$o<c>`, `$r<c>` or `$a<c>` where `<c>`
 * is the lowercase operator index.
 *
 * @param[in]  op      Operator token. Not NULL.
 * @param[in]  len     Length of @p op, or 0 to run until NUL.
 * @param[out] result  Destination buffer for 3 characters. Not NULL.
 * @return The number of bytes written (0 or 3).
 */
size_t __mangle_operator(                       // MANGLE OPERATOR NAME
    char const *op,                             // - operator token
    size_t len,                                 // - length of operator token
    char *result )                              // - operator name
                                                // return len of operator name
{
    int i;
    if( len == 0 ) {
        len = strlen( op );
    }

    for( i = 0 ; i < num_elements( operatorFunction ) ; i++ ) {
        if( strncmp( op, operatorFunction[i], len ) == 0 ) {
            if( operatorFunction[i][len] == NULL_CHAR ) {
                *result++ = OPNAME_PREFIX;
                *result++ = OP_FUN_PREFIX;
                *result++ = i + 'a';
                return( 3 );
            }
        }
    }
    for( i = 0 ; i < num_elements( relationalFunction ) ; i++ ) {
        if( strncmp( op, relationalFunction[i], len ) == 0 ) {
            if( relationalFunction[i][len] == NULL_CHAR ) {
                *result++ = OPNAME_PREFIX;
                *result++ = REL_FUN_PREFIX;
                *result++ = i + 'a';
                return( 3 );
            }
        }
    }
    for( i = 0 ; i < num_elements( assignmentFunction ) ; i++ ) {
        if( strncmp( op, assignmentFunction[i], len ) == 0 ) {
            if( assignmentFunction[i][len] == NULL_CHAR ) {
                *result++ = OPNAME_PREFIX;
                *result++ = ASGN_FUN_PREFIX;
                *result++ = i + 'a';
                return( 3 );
            }
        }
    }
    return( 0 );
}
#endif //__DIP__

#endif //!__LIB__

#if 0 || defined(TEST) || defined(DUMP)
void dump( output_desc *data )
{
    int i;

    printf( "input:   [%x]>%s<\n", data->input, data->input );
    printf( "end:     [%x]\n", data->end );
    printf( "output:  [%x]>%s<\n", data->output, data->output );
    printf( "realloc: [%x]\n", user_realloc );
    printf( "size: %d, count: %d, index: %d, suppress_output: %d\n",
            data->size,
            data->count,
            data->index,
            data->suppress_output );
    for( i = 0 ; i < next_replicate ; i++ ) {
        printf( "replicate#%d: ptr: [%x]>%s< len: %d\n",
                i,
                replicate[i].ptr,
                replicate[i].ptr,
                replicate[i].len );
    }
    printf( "pending_loc: %d, ctdt_pending: %d, cv_pending: %d, base_name: %d\n",
            data->pending_loc,
            data->ctdt_pending,
            data->cv_pending,
            data->base_name );
    printf( "scope_name: %d, scope_index: %d, scope_ptr: %x, scope_len %d\n",
            data->scope_name,
            data->scope_index,
            data->scope_ptr,
            data->scope_len );
}

#endif

#if 0 || defined(TEST)
/*!
 * @def TRUNC_BUFFER
 * @brief Size of the truncation-test buffer.
 */
#define TRUNC_BUFFER    256
/*!
 * @def GUARD_CHAR
 * @brief Guard byte used to detect buffer overruns.
 */
#define GUARD_CHAR      '@'
/*!
 * @def ALLOC_SIZE
 * @brief Initial size of the reallocatable test buffer.
 */
#define ALLOC_SIZE      2
/*!
 * @def BUF_SIZE
 * @brief Size of the small test buffer.
 */
#define BUF_SIZE        256

typedef struct test_stream {
    char *mangle;               // mangled name
    char *full_demangle;        // complete demangled name
    char *unmangled;            // unmangled name
    char *basename;             // demangled basename
} test_stream;

test_stream testVector[] = {
#if 1
    "W?foo$n(pn(uauaua)v)v",
        "void near foo( void (near *)( char, char, char ))",
        "foo",
        "foo",
    "W?a$ni",
        "int near a",
        "a",
        "a",
    "W?$CT:S$n()_",
        "near S::S()",
        "$CT",
        "S",
    "W?$DT:S$n()_",
        "near S::~S()",
        "$DT",
        "~S",
    "W?$DLn(pv)v",
        "void near operator delete( void * )",
        "$DL",
        "operator delete",
    "W?foo$n(iuipua)v",
        "void near foo( int, int unsigned, char * )",
        "foo",
        "foo",
    "W?$CV:opCONV$n()i",
        "near opCONV::operator int ()",
        "$CV",
        "operator int",
    "W?foo$n(pn$opCONV$$)pn$opCONV$$",
        "opCONV near * near foo( opCONV near * )",
        "foo",
        "foo",
    "W?foo$n(pn$opCONV$$)pn$1$",
        "opCONV near * near foo( opCONV near * )",
        "foo",
        "foo",
    "W?$NWn(ui)pnv",
        "void near * near operator new( int unsigned )",
        "$NW",
        "operator new",
    "W?bar$n($opnew$$)$1$",
        "opnew near bar( opnew )",
        "bar",
        "bar",
    "W?foo$n(pn[2][3][4]ipn[6]ua)v",
        "void near foo( int (near *)[2][3][4], char (near *)[6])",
        "foo",
        "foo",
    "W?bar$n(pn(uab)ipn(pn(d)it)v)v",
        "void near bar( int (near *)( char, float ), void (near *)( int (near *)( double ), long double ))",
        "bar",
        "bar",
    "W?pff$npn(db)ua",
        "char (near * near pff)( double, float )",
        "pff",
        "pff",
    "W?goo$nm$Base$$ni",
        "int near Base::* near goo",
        "goo",
        "goo",
    "W?goo$n(nm$Base$$ni)v",
        "void near goo( int near Base::* near )",
        "goo",
        "goo",
    "W?$Wvm07:H$$:E$$nx[]ui",
        "int unsigned const near __vmtbl[]",
        "$Wvm",
        "__vmtbl",
    "W?$Wvt0qset_v1:A$c$:A$set_v1$n(i)v$:V1$n(i)v",
        "void near V1::__vfthunk( int )",
        "$Wvt",
        "__vfthunk",
    "W?foo$:S$n(ua)i",
        "int near S::foo( char )",
        "foo",
        "foo",
    "W?foo$:S$n.x(ua)i",
        "int near S::foo( char ) const",
        "foo",
        "foo",
    "W?$CV:S$n()i",
        "near S::operator int ()",
        "$CV",
        "operator int",
    "W?$CV:S$n.x()i",
        "near S::operator int () const",
        "$CV",
        "operator int",
    "W?$WAA19ios:$opDTOR$$istream:c$$istream:$$opDTOR$?n()$n()pnv",
        "void near * near __internal()",
        "$WAA",
        "__internal",
    "W?_trmem_open$n(pn(ui)pnvpn(pnv)vpn(pnvui)pnvpn(pnvui)pnvpnvpn(pnvpnxuaui)vui)pn$_trmem_internal$$",
        "_trmem_internal near * near _trmem_open( void near * (near *)( int unsigned ), void (near *)( void near * ), void near * (near *)( void near "
        "*, int unsigned ), void near * (near *)( void near *, int unsigned ), void near *, void (near *)( void near *, char const near *, int unsigned ), int unsigned )",
        "_trmem_open",
        "_trmem_open",
    "W?s$n$Stack$::1ni0az?ok$n()v$",
        "Stack<int near,10,void near ok()> near s",
        "s",
        "s",
    "W?dummy$:Stack$::1ni0az?ok$n()vn()v",
        "void near Stack<int near,10,void near ok()>::dummy()",
        "dummy",
        "dummy",
    "W?$CT:Stack$::1ni0ay?ok$n()vn()_",
        "near Stack<int near,-10,void near ok()>::Stack()",
        "$CT",
        "Stack",
    "W?s$n$Inner$:Stack$::1ni0az?ok$n()v:xyz$$",
        "xyz::Stack<int near,10,void near ok()>::Inner near s",
        "s",
        "s",
    "W?s$n$Inner$:Stack$::1ni0az1n$xyz$::1ni$:abc$$",
        "abc::Stack<int near,10,xyz<int near > near >::Inner near s",
        "s",
        "s",
    "W?dummy$:Stack$::1ni0az?ok$n()v:xyz$n()v",
        "void near xyz::Stack<int near,10,void near ok()>::dummy()",
        "dummy",
        "dummy",
    "W?dummy$:Stack$::1ni0az?ok$n()v:xyz$::1nin()v",
        "void near xyz<int near >::Stack<int near,10,void near ok()>::dummy()",
        "dummy",
        "dummy",
    "W?a$:.1$:?foo$n()vn[]i",
        "int near void near foo()::.1::a[]",
        "a",
        "a",
    "W?cout$n$ostream$$",
        "ostream near cout",
        "cout",
        "cout",
    "W?$OB:ostream$n(pnxua)rn$ostream$$",
        "ostream near & near ostream::operator <<( char const near * )",
        "$OB",
        "operator <<",
    "W?endl$n(rn$ostream$$)rn$ostream$$",
        "ostream near & near endl( ostream near & )",
        "endl",
        "endl",
    "W?$OB:ostream$n(pn(rn$ostream$$)rn$ostream$$)rn$ostream$$",
        "ostream near & near ostream::operator <<( ostream near & (near *)( ostream near & ))",
        "$OB",
        "operator <<",
    "W?m$n()pjvv",
        "void __based(void) * near m()",
        "m",
        "m",
    "W?pv$npjvua",
        "char __based(void) * near pv",
        "pv",
        "pv",
    "W?ps$npjsua",
        "char __based(__self) * near ps",
        "ps",
        "ps",
    "W?pn$npjl03FOOuc",
        "char unsigned __based(\"FOO\") * near pn",
        "pn",
        "pn",
    "W?pa$npjf$s$uc",
        "char unsigned __based(s) * near pa",
        "pa",
        "pa",
    "W?pp$npja$b$ua",
        "char __based(b) * near pp",
        "pp",
        "pp",
    "W?bar$n(uaua)v",
        "void near bar( char, char )",
        "bar",
        "bar",
    "W?bar$n(cc)v",
        "void near bar( char signed, char signed )",
        "bar",
        "bar",
    "W?bar$n(ucuc)v",
        "void near bar( char unsigned, char unsigned )",
        "bar",
        "bar",
    "W?mpa$nm$S$$nxi",
        "int const near S::* near mpa",
        "mpa",
        "mpa",
    "W?mpb$nm$S$$nyi",
        "int volatile near S::* near mpb",
        "mpb",
        "mpb",
    "W?$Wmp05a$nxi$:S$n()pnxi",
        "int const near * near S::__mbrptrthunk()",
        "$Wmp",
        "__mbrptrthunk",
    "W?$Wmp05b$nyi$:S$n()pnyi",
        "int volatile near * near S::__mbrptrthunk()",
        "$Wmp",
        "__mbrptrthunk",
    "W?y$n$E$$",
        "E near y",
        "y",
        "y",
    "W?y$n$$TE$$",
        "__anonymous_enum near y",
        "y",
        "y",
    "W?setSymType$:KeySymbol$f($$TE$$)v",
        "void far KeySymbol::setSymType( __anonymous_enum )",
        "setSymType",
        "setSymType",
    "W?y$n$$TEextratext$$",
        "__anonymous_enum near y",
        "y",
        "y",
    "W?setSymType$:KeySymbol$f($$TEextra_stuff$$)v",
        "void far KeySymbol::setSymType( __anonymous_enum )",
        "setSymType",
        "setSymType",
    "W?foo$n(pn$_123456789012345678901234567890_1$$pn$_123456789012345678901234567890_2$$pn$_123456789012345678901234567890_3$$pn$_1234567890123456789"
    "01234567890_4$$pn$_123456789012345678901234567890_5$$pn$_123456789012345678901234567890_6$$pn$_123456789012345678901234567890_7$$)v",
        "void near foo( _123456789012345678901234567890_1 near *, _123456789012345678901234567890_2 near *, _123456789012345678901234567890_3 near *, "
        "_123456789012345678901234567890_4 near *, _123456789012345678901234567890_5 near *, _123456789012345678901234567890_6 near *, _123456789012345678901234567890_7 near * )",
        "foo",
        "foo",
    "W?$nan(ui)pnv",
        "void near * near operator new []( int unsigned )",
        "$na",
        "operator new []",
    "W?$dan(pnv)v",
        "void near operator delete []( void near * )",
        "$da",
        "operator delete []",
    "W?$Wcm012$nx[]uc",
        "char unsigned const near __stattabcmd[]",
        "$Wcm",
        "__stattabcmd",
    "W?$Wqq012$i",
        "int __internal",
        "$Wqq",
        "__internal",
    "W?$Wdi0vnql0f8@f__dlang_h_win_windows_h$na",
        "char near __debuginfo",
        "$Wdi",
        "__debuginfo",
    "W?e$nuz",
        "__int64 unsigned near e",
        "e",
        "e",
    "__imp_W?foo$n(iiapn(aas)i)i",
        "int near foo( int, int, char, int (near *)( char, char, short ))",
        "foo",
        "foo",
    "W?x$:$Wun0f1_dr061n1l8koys$:A$ni",
        "int near A::<unique>::x",
        "x",
        "x",
#endif
    "W?exprNodeAnalysis$:$Wun0e2o_eo9stv6l5hj$n(pn$ExprNode$$pnv)pn$1$",
        "ExprNode near * near <unique>::exprNodeAnalysis( ExprNode near *, void near * )",
        "exprNodeAnalysis",
        "exprNodeAnalysis",
#if 0 || defined(__INCLUDE_TRUNCATED_NAME)
    "T?hhhhfoo$n(pn$_123456789012345678901234567890_1$$pn$_123456789012345678901234567890_2$$pn$_123456789012345678901234567890_3$$pn$_123456789012345"
    "678901234567890_4$$pn$_123456789012345678901234567890_5$$pn$_123456789012345678901234567890_6$$pn$_12345678901",
        "near foo( _123456789012345678901234567890_1 near *, _123456789012345678901234567890_2 near *, _123456789012345678901234567890_3 near *, _1234"
        "56789012345678901234567890_4 near *, _123456789012345678901234567890_5 near *, _123456789012345678901234567890_6 near *, near * ",
        "foo",
        "foo",
    "T?hhhhxx$n$S$::?foo$n(pn$_123456789012345678901234567890_1$$pn$_123456789012345678901234567890_2$$pn$_123456789012345678901234567890_3$$pn$_12345"
    "6789012345678901234567890_4$$pn$_123456789012345678901234567890_5$$pn$_123456789012345678901234567890_6$$pn$_1",
        "near foo( _123456789012345678901234567890_1 near *, _12345678901234567890123456>::S near xx",
        "xx",
        "xx",
#endif
    NULL,
        NULL,
        NULL
};

void testEmit( void **cookie, dm_pts dp, int value, char const *ptr )
{
    unsigned **argc = cookie;
    static const char *names[] = {
        /*!
         * @def DM_DEF
         * @brief Emit one DM_* event name format string.
         * @param[in] id  DM_* identifier.
         */
        #define DM_DEF( id )    "\tDM_" #id ": %d\n",
        DM_DEFS
        #undef DM_DEF
    };

    ptr = ptr;
    if( ((unsigned)dp) < DM_INVALID ) {
        if( **argc > 1 ) {
            printf( names[dp], value );
        }
    } else {
        #if 0 || defined(TEST)
            ++errors;
            printf( "ERROR: testEmit unknown dm_pts(%d)\n", dp );
        #endif
    }
}

static char *typeTestVector[] = {
    "$D$$", "D",
    "pn$D$$", "D near *",
    NULL, NULL,
};

void main( int argc )
{
    static char buff[TRUNC_BUFFER+2];   // allow for two guard chars
    size_t len;
    size_t trunc_len;
    int i;
    char *p;
    char const *q;

    printf( "To see all the results, invoke demangle.exe with a parameter\n" );
    printf( "When executing with a parameter, no page fault is success.\n" );
    printf( "Starting test...\n" );
    assert((( sizeof( typeTestVector ) / sizeof( typeTestVector[0] ) ) & 1 ) == 0 );
    errors = 0;
    for( i = 0 ; testVector[i].mangle != NULL ; ++i ) {
        len = __demangle_l( testVector[i].mangle, 0, NULL, 0 );
        if( argc > 1 ) {
            printf( "%s -> (%u)\n", testVector[i].mangle, len );
        }
    }
    for( trunc_len = TRUNC_BUFFER ; trunc_len > 0 ; trunc_len-- ) {
        for( i = 0 ; testVector[i].mangle != NULL ; ++i ) {
            buff[0] = buff[trunc_len+1] = GUARD_CHAR;
            len = __demangle_l( testVector[i].mangle, 0, buff+1, trunc_len );
            if( argc > 1 ) {
                printf( "%s -->%s<-- (%u)\n", testVector[i].mangle, buff+1, len );
            }
            if( len > trunc_len ) {
                ++errors;
                printf( "ERROR: returned length exceeds truncate length\n" );
            }
            if( buff[0] != GUARD_CHAR ) {
                printf( "ERROR: ran off start of output buffer\n" );
                ++errors;
            }
            if( buff[trunc_len+1] != GUARD_CHAR ) {
                printf( "ERROR: ran off end of output buffer\n" );
                ++errors;
            }
        }
        for( i = 0; typeTestVector[i] != NULL; i += 2 ) {
            buff[0] = buff[trunc_len+1] = GUARD_CHAR;
            len = __demangle_t( typeTestVector[i], 0, buff+1, trunc_len );
            if( argc > 1 ) {
                printf( "%s -->%s<-- (%u)\n", typeTestVector[i], buff+1, len );
            }
            if( memcmp( buff+1, typeTestVector[i+1], len ) ) {
                printf( "ERROR:\n%s\ndemangle should yield -->%s<--\n", typeTestVector[i], typeTestVector[i+1] );
                printf(             "          but yielded -->%s<-- (%u)\n", buff+1, len );
                ++errors;
            }
            if( len > trunc_len ) {
                ++errors;
                printf( "ERROR: returned length exceeds truncate length\n" );
            }
            if( buff[0] != GUARD_CHAR ) {
                printf( "ERROR: ran off start of output buffer\n" );
                ++errors;
            }
            if( buff[trunc_len+1] != GUARD_CHAR ) {
                printf( "ERROR: ran off end of output buffer\n" );
                ++errors;
            }
        }
    }
    if( errors ) {
        printf( "test failed!\n" );
        exit( 1 );
    }
    for( i = 0 ; testVector[i].mangle != NULL ; ++i ) {
        p = malloc( ALLOC_SIZE );
        len = __demangle_r( testVector[i].mangle, 0, &p, ALLOC_SIZE, (char *(*)(char *, size_t))realloc );
        if( argc > 1 ) {
            printf( "%s -->%s<-- (%u)\n", testVector[i].mangle, p, len );
        }
        free( p );
    }
    for( i = 0 ; testVector[i].mangle != NULL ; ++i ) {
        if( __unmangled_name( testVector[i].mangle, 0, &q, &len ) ) {
            char buff[BUF_SIZE];
            memcpy( buff, q, len );
            buff[len] = 0;
            if( argc > 1 ) {
                printf( "%s -->%s<-- (%u) (unmangled name)\n", testVector[i].mangle, buff, len );
            }
        } else {
            if( argc > 1 ) {
                printf( "%s (not mangled)\n", testVector[i].mangle );
            }
        }
    }
    for( i = 0 ; testVector[i].mangle != NULL ; ++i ) {
        if( argc > 1 ) {
            printf( "parsing -->%s<--\n", testVector[i].mangle );
        }
        __parse_mangled_name( testVector[i].mangle, 0, &argc, &testEmit );
    }
    for( i = 0 ; testVector[i].mangle != NULL ; ++i ) {
        p = malloc( ALLOC_SIZE );
        len = __demangle_r( testVector[i].mangle, 0, &p, ALLOC_SIZE, (char *(*)(char *, size_t))realloc );
        if( strcmp( p, testVector[i].full_demangle ) ) {
            printf( "ERROR:\n%s\ndemangle should yield -->%s<--\n", testVector[i].mangle, testVector[i].full_demangle );
            printf(             "          but yielded -->%s<-- (%u)\n", p, len );
            ++errors;
        }
        free( p );
        __unmangled_name( testVector[i].mangle, 0, &q, &len );
        if( strncmp( q, testVector[i].unmangled, len ) ) {
            char buff[BUF_SIZE];
            memcpy( buff, q, len );
            buff[len] = 0;
            printf( "%s\nunmangle should yield -->%s<--\n", testVector[i].mangle, testVector[i].unmangled );
            printf(     "          but yielded -->%s<-- (%u)\n", buff, len );
            ++errors;
        }
        p = malloc( BUF_SIZE );
        len = __demangled_basename( testVector[i].mangle, 0, p, BUF_SIZE );
        if( strncmp( p, testVector[i].basename, len ) ) {
            printf( "%s\nbasename should yield -->%s<--\n", testVector[i].mangle, testVector[i].basename );
            printf(     "          but yielded -->%s<-- (%u)\n", p, len );
            ++errors;
        }
        free( p );
    }
    if( errors ) {
        printf( "test failed!\n" );
        exit( 1 );
    }
    for( trunc_len = TRUNC_BUFFER ; trunc_len > 0 ; trunc_len-- ) {
        for( i = 0 ; testVector[i].mangle != NULL ; ++i ) {
            buff[0] = buff[trunc_len+1] = GUARD_CHAR;
            len = __demangle_l( testVector[i].mangle, 0, buff+1, trunc_len );
            if( len > trunc_len ) {
                printf( "ERROR: returned length exceeds truncate length\n" );
                ++errors;
            }
            if( buff[0] != GUARD_CHAR ) {
                printf( "ERROR: ran off start of output buffer\n" );
                ++errors;
            }
            if( buff[trunc_len+1] != GUARD_CHAR ) {
                printf( "ERROR: ran off end of output buffer\n" );
                ++errors;
            }
            if( errors ) {
                printf( "test failed!\n" );
                exit( 1 );
            }
        }
    }
    if( argc > 1 ) {
        no_errors = 1;
        // test truncated names
        for( i = 0 ; testVector[i].mangle != NULL ; ++i ) {
            int j, max;
            max = strlen( testVector[i].mangle );
            for( j = max ; j > 0 ; j-- ) {
                testVector[i].mangle[j] = '\0';
                p = malloc( ALLOC_SIZE );
                __demangle_r( testVector[i].mangle, 0, &p, ALLOC_SIZE, (char *(*)(char *, size_t))realloc );
                printf( "truncated demangle yielded -->%s<-- (%u)\n", p, len );
                free( p );
            }
        }
    }
    printf( "...test completed.\n" );
    exit( 0 );
}
#endif

#if 0 || defined(UTIL)

/*!
 * @def BUF_SIZE
 * @brief Size of the command-line demangle buffer.
 */
#define BUF_SIZE 4096

void main( int argc, char **argv )
{
    char    buffer[BUF_SIZE];

    if( argc < 2) {
        printf( "Usage: demangle <mangled name>\n" );
        return;
    }
    __demangle_l( argv[1], strlen( argv[1] ), buffer, BUF_SIZE );
    puts( buffer );
}
#endif
