/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         __reginaparse
#define yylex           __reginalex
#define yyerror         __reginaerror
#define yydebug         __reginadebug
#define yynerrs         __reginanerrs

#define yylval          __reginalval
#define yychar          __reginachar

/* Copy the first part of user declarations.  */
#line 1 "../yaccsrc.y" /* yacc.c:339  */


#ifndef lint
static char *RCSid = "$Id: yaccsrc.c,v 1.47 2019/04/06 10:11:45 mark Exp $";
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

#include "rexx.h"
#include <time.h>

#if defined(HAVE_MALLOC_H)
# include <malloc.h>
#endif

#if defined(HAVE_ALLOCA_H)
# include <alloca.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#if defined(_MSC_VER) || defined(MAC)
# define __STDC__ 1 /* Hack to allow const since it is not defined */
#endif

#define YYSTYPE nodeptr

/* locals, they are protected by regina_parser (see lexsrc.l) */
static int tmplno,           /* lineno of current instruction             */
           tmpchr,           /* character position of current instruction */
           level,            /* nested do/if/select depth                 */
           start_parendepth; /* see below at parendepth                   */

/*
 * parendepth regulates the action which happens detecting a comma or an
 * empty expression. A negative values indicates an error; both a comma
 * and an empty expression raise an error.
 * We regulate the enumeration of arguments with this semantical flag.
 * Look at "call subroutine args" and "function args". Function itself
 * contains a parentheses pair, so starting with a depth of just allows
 * the enumeration. subroutine starts with either 0 or 1. The latter one
 * is allowed for the support request of "call subroutine(a,b,c)" which
 * isn't allowed by ANSI but can be enabled for backward compatibility.
 */
static int parendepth;

static nodeptr current, with = NULL;

static char *nullptr = NULL; /* for C++ compilation */

typedef enum { IS_UNKNOWN,
               IS_A_NUMBER,
               IS_NO_NUMBER,
               IS_SIM_SYMBOL,
               IS_COMP_SYMBOL } node_type;

typedef enum { REDUCE_CALL,
               REDUCE_EXPR,
               REDUCE_RIGHT,
               REDUCE_SUBEXPR } reduce_mode;

static node_type gettypeof( nodeptr thisptr ) ;
static void checkconst( nodeptr thisptr ) ;
static nodeptr reduce_expr_list( nodeptr thisptr, reduce_mode mode );
static void transform( nodeptr thisptr ) ;
static nodeptr create_head( const char *name ) ;
static nodeptr makenode( int type, int numb, ... ) ;
static void checkdosyntax( cnodeptr thisptr ) ;
void newlabel( const tsd_t *TSD, internal_parser_type *ipt, nodeptr thisptr ) ;
static nodeptr optgluelast( nodeptr p1, nodeptr p2 );
static void move_labels( nodeptr front, nodeptr end, int level );

#define IS_EXPRLIST(x) ( ( (x) != NULL )                   \
                      && ( ( (x)->type == X_CEXPRLIST )    \
                        || ( (x)->type == X_EXPRLIST ) ) )

#define IS_FUNCTION(x) ( ( (x) != NULL )                  \
                      && ( ( (x)->type == X_EX_FUNC )     \
                        || ( (x)->type == X_IN_FUNC ) ) )

#define AUTO_REDUCE(x,y) { if ( parendepth == 1 )                             \
                           {                                                  \
                             x = reduce_expr_list( x, REDUCE_EXPR );          \
                             /* detect "call s (a,b)<op>" and      */         \
                             /* "call s ()<op>"                    */         \
                             if ( IS_EXPRLIST( x ) )                          \
                             {                                                \
                                if ( (y) != NULL )                            \
                                   exiterror( ERR_INVALID_EXPRESSION, 1, y ); \
                                else if ( (x)->p[0] == NULL )                 \
                                   exiterror( ERR_UNEXPECTED_PARAN, 0 );      \
                                else                                          \
                                   exiterror( ERR_UNEXPECTED_PARAN, 1 );      \
                             }                                                \
                           }                                                  \
                         }
#line 150 "../yaccsrc.y" /* yacc.c:339  */

#ifdef NDEBUG
# define YYDEBUG 0
#else
# define YYDEBUG 1
#endif

#line 199 "../yaccsrc.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "yaccsrc.h".  */
#ifndef YY__REGINA_YACCSRC_H_INCLUDED
# define YY__REGINA_YACCSRC_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int __reginadebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ADDRESS = 258,
    ARG = 259,
    CALL = 260,
    DO = 261,
    TO = 262,
    BY = 263,
    FOR = 264,
    WHILE = 265,
    UNTIL = 266,
    EXIT = 267,
    IF = 268,
    THEN = 269,
    ELSE = 270,
    ITERATE = 271,
    INTERPRET = 272,
    LEAVE = 273,
    NOP = 274,
    NUMERIC = 275,
    PARSE = 276,
    EXTERNAL = 277,
    SOURCE = 278,
    VAR = 279,
    VALUE = 280,
    WITH = 281,
    PROCEDURE = 282,
    EXPOSE = 283,
    PULL = 284,
    PUSH = 285,
    QUEUE = 286,
    SAY = 287,
    RETURN = 288,
    SELECT = 289,
    WHEN = 290,
    DROP = 291,
    OTHERWISE = 292,
    SIGNAL = 293,
    ON = 294,
    OFF = 295,
    SYNTAX = 296,
    HALT = 297,
    NOVALUE = 298,
    TRACE = 299,
    END = 300,
    UPPER = 301,
    ASSIGNMENTVARIABLE = 302,
    STATSEP = 303,
    FOREVER = 304,
    DIGITS = 305,
    FORM = 306,
    FUZZ = 307,
    SCIENTIFIC = 308,
    ENGINEERING = 309,
    NOT = 310,
    CONCATENATE = 311,
    MODULUS = 312,
    GTE = 313,
    GT = 314,
    LTE = 315,
    LT = 316,
    DIFFERENT = 317,
    EQUALEQUAL = 318,
    NOTEQUALEQUAL = 319,
    OFFSET = 320,
    SPACE = 321,
    EXP = 322,
    XOR = 323,
    PLACEHOLDER = 324,
    NOTREADY = 325,
    CONSYMBOL = 326,
    SIMSYMBOL = 327,
    EXFUNCNAME = 328,
    INFUNCNAME = 329,
    LABEL = 330,
    DOVARIABLE = 331,
    HEXSTRING = 332,
    STRING = 333,
    VERSION = 334,
    LINEIN = 335,
    WHATEVER = 336,
    NAME = 337,
    FAILURE = 338,
    BINSTRING = 339,
    OPTIONS = 340,
    ENVIRONMENT = 341,
    LOSTDIGITS = 342,
    GTGT = 343,
    LTLT = 344,
    NOTGTGT = 345,
    NOTLTLT = 346,
    GTGTE = 347,
    LTLTE = 348,
    INPUT = 349,
    OUTPUT = 350,
    ERROR = 351,
    NORMAL = 352,
    APPEND = 353,
    REPLACE = 354,
    STREAM = 355,
    STEM = 356,
    LIFO = 357,
    FIFO = 358,
    NOEOL = 359,
    LOWER = 360,
    CASELESS = 361,
    PLUSASSIGNMENTVARIABLE = 362,
    MINUSASSIGNMENTVARIABLE = 363,
    MULTASSIGNMENTVARIABLE = 364,
    DIVASSIGNMENTVARIABLE = 365,
    MODULUSASSIGNMENTVARIABLE = 366,
    INTDIVASSIGNMENTVARIABLE = 367,
    ORASSIGNMENTVARIABLE = 368,
    XORASSIGNMENTVARIABLE = 369,
    ANDASSIGNMENTVARIABLE = 370,
    CONCATASSIGNMENTVARIABLE = 371,
    CCAT = 372,
    UMINUS = 373,
    UPLUS = 374
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE __reginalval;

int __reginaparse (void);

#endif /* !YY__REGINA_YACCSRC_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 370 "../yaccsrc.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2307

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  131
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  196
/* YYNRULES -- Number of rules.  */
#define YYNRULES  479
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  654

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   374

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   125,   118,     2,
     128,   129,   123,   121,   130,   122,     2,   124,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   119,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,   117,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   120,   126,   127
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   160,   160,   160,   170,   173,   179,   181,   184,   186,
     189,   190,   193,   198,   199,   202,   203,   206,   207,   210,
     211,   214,   215,   218,   222,   225,   226,   227,   228,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   268,   272,   277,   281,   286,   290,
     294,   298,   302,   306,   310,   314,   318,   322,   326,   330,
     334,   338,   342,   346,   350,   354,   359,   363,   367,   371,
     375,   379,   379,   383,   386,   387,   388,   388,   390,   394,
     400,   406,   406,   411,   411,   417,   418,   419,   420,   421,
     427,   432,   433,   439,   440,   443,   451,   454,   460,   466,
     468,   469,   472,   493,   497,   501,   505,   508,   509,   510,
     512,   515,   516,   517,   520,   520,   526,   529,   530,   531,
     534,   535,   536,   537,   540,   544,   548,   552,   553,   554,
     555,   556,   559,   560,   561,   562,   563,   566,   567,   568,
     569,   570,   573,   574,   577,   578,   581,   582,   585,   586,
     588,   589,   594,   595,   597,   598,   599,   600,   605,   613,
     614,   627,   628,   632,   633,   637,   640,   640,   644,   644,
     648,   649,   652,   653,   656,   664,   672,   675,   676,   677,
     680,   681,   682,   685,   686,   690,   691,   695,   699,   707,
     717,   719,   721,   722,   725,   728,   731,   736,   738,   739,
     742,   750,   754,   756,   757,   760,   761,   764,   767,   768,
     769,   772,   774,   776,   777,   778,   781,   784,   785,   788,
     791,   797,   803,   804,   807,   809,   811,   813,   815,   817,
     819,   823,   825,   828,   829,   830,   831,   832,   833,   834,
     836,   837,   840,   841,   842,   843,   844,   848,   852,   856,
     860,   864,   868,   869,   870,   873,   878,   879,   881,   882,
     883,   887,   888,   889,   892,   898,   900,   901,   911,   912,
     915,   919,   925,   928,   929,   931,   932,   933,   934,   935,
     941,   945,   946,   952,   953,   954,   957,   958,   959,   962,
     963,   966,   969,   972,   973,   974,   975,   978,   979,   980,
     981,   984,   986,   988,   989,   993,   997,  1013,  1019,  1024,
    1030,  1035,  1041,  1046,  1052,  1057,  1063,  1068,  1074,  1079,
    1085,  1090,  1096,  1101,  1107,  1112,  1118,  1123,  1130,  1130,
    1163,  1163,  1166,  1166,  1169,  1169,  1172,  1173,  1173,  1176,
    1177,  1177,  1180,  1181,  1181,  1184,  1185,  1185,  1188,  1189,
    1189,  1192,  1193,  1193,  1196,  1197,  1197,  1200,  1201,  1201,
    1204,  1205,  1205,  1208,  1209,  1211,  1212,  1212,  1216,  1217,
    1217,  1221,  1222,  1222,  1226,  1227,  1227,  1231,  1232,  1232,
    1236,  1237,  1237,  1241,  1242,  1242,  1245,  1246,  1246,  1249,
    1250,  1250,  1253,  1254,  1254,  1257,  1258,  1258,  1261,  1262,
    1262,  1265,  1266,  1266,  1269,  1270,  1270,  1273,  1274,  1275,
    1277,  1281,  1285,  1287,  1288,  1290,  1292,  1293,  1296,  1297,
    1298,  1301,  1305,  1307,  1311,  1311,  1318,  1321,  1324,  1325,
    1326,  1329,  1331,  1333,  1335,  1337,  1339,  1341,  1343,  1345,
    1349,  1350,  1359,  1360,  1364,  1370,  1371,  1372,  1382,  1382,
    1462,  1506,  1507,  1510,  1512,  1514,  1516,  1520,  1523,  1524
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ADDRESS", "ARG", "CALL", "DO", "TO",
  "BY", "FOR", "WHILE", "UNTIL", "EXIT", "IF", "THEN", "ELSE", "ITERATE",
  "INTERPRET", "LEAVE", "NOP", "NUMERIC", "PARSE", "EXTERNAL", "SOURCE",
  "VAR", "VALUE", "WITH", "PROCEDURE", "EXPOSE", "PULL", "PUSH", "QUEUE",
  "SAY", "RETURN", "SELECT", "WHEN", "DROP", "OTHERWISE", "SIGNAL", "ON",
  "OFF", "SYNTAX", "HALT", "NOVALUE", "TRACE", "END", "UPPER",
  "ASSIGNMENTVARIABLE", "STATSEP", "FOREVER", "DIGITS", "FORM", "FUZZ",
  "SCIENTIFIC", "ENGINEERING", "NOT", "CONCATENATE", "MODULUS", "GTE",
  "GT", "LTE", "LT", "DIFFERENT", "EQUALEQUAL", "NOTEQUALEQUAL", "OFFSET",
  "SPACE", "EXP", "XOR", "PLACEHOLDER", "NOTREADY", "CONSYMBOL",
  "SIMSYMBOL", "EXFUNCNAME", "INFUNCNAME", "LABEL", "DOVARIABLE",
  "HEXSTRING", "STRING", "VERSION", "LINEIN", "WHATEVER", "NAME",
  "FAILURE", "BINSTRING", "OPTIONS", "ENVIRONMENT", "LOSTDIGITS", "GTGT",
  "LTLT", "NOTGTGT", "NOTLTLT", "GTGTE", "LTLTE", "INPUT", "OUTPUT",
  "ERROR", "NORMAL", "APPEND", "REPLACE", "STREAM", "STEM", "LIFO", "FIFO",
  "NOEOL", "LOWER", "CASELESS", "PLUSASSIGNMENTVARIABLE",
  "MINUSASSIGNMENTVARIABLE", "MULTASSIGNMENTVARIABLE",
  "DIVASSIGNMENTVARIABLE", "MODULUSASSIGNMENTVARIABLE",
  "INTDIVASSIGNMENTVARIABLE", "ORASSIGNMENTVARIABLE",
  "XORASSIGNMENTVARIABLE", "ANDASSIGNMENTVARIABLE",
  "CONCATASSIGNMENTVARIABLE", "'|'", "'&'", "'='", "CCAT", "'+'", "'-'",
  "'*'", "'/'", "'%'", "UMINUS", "UPLUS", "'('", "')'", "','", "$accept",
  "start", "$@1", "prog", "stats", "xstats", "ystatement", "lonely_end",
  "nxstats", "ncl", "nlncl", "optLabels", "statement", "gruff",
  "mttstatement", "mtstatement", "nclstatement", "call", "do", "exit",
  "if", "iterate", "leave", "say", "return", "address", "arg", "drop",
  "interpret", "label", "nop", "numeric", "options", "parse", "proc",
  "pull", "push", "queue", "select", "signal", "when", "otherwise",
  "trace", "upper", "address_stat", "@2", "address_stat2", "$@3",
  "arg_stat", "call_stat", "$@4", "$@5", "call_name", "call_args",
  "expr_stat", "end_stat", "end", "do_stat", "repetitor", "nvir",
  "naddr_with", "@6", "addr_with", "connection", "inputstmts",
  "outputstmts", "errorstmts", "adeo", "adei", "adio", "inputstmt",
  "outputstmt", "errorstmt", "resourcei", "resourceo", "resources",
  "nsimsymb", "$@7", "nnvir", "$@8", "nspace", "addrAll", "addrSim",
  "addrString", "dovar", "tobyfor", "conditional", "drop_stat",
  "upper_stat", "exit_stat", "if_stat", "unexp_then", "unexp_else",
  "ipret_stat", "iterate_stat", "label_stat", "labelname", "leave_stat",
  "nop_stat", "numeric_stat", "form_expr", "options_stat", "parse_stat",
  "parse_flags", "templs", "parse_param", "proc_stat", "pull_stat",
  "push_stat", "queue_stat", "say_stat", "return_stat", "sel_end",
  "select_stat", "when_stats", "when_stat", "ex_when_stat",
  "otherwise_stat", "signal_stat", "signal_name", "namespec", "asymbol",
  "on", "off", "c_action", "s_action", "trace_stat", "whatever",
  "assignment", "ass_part", "plusassignment", "plus_ass_part",
  "minusassignment", "minus_ass_part", "multassignment", "mult_ass_part",
  "divassignment", "div_ass_part", "intdivassignment", "intdiv_ass_part",
  "modulusassignment", "modulus_ass_part", "andassignment", "and_ass_part",
  "xorassignment", "xor_ass_part", "orassignment", "or_ass_part",
  "concatassignment", "concat_ass_part", "expr", "$@9", "$@10", "$@11",
  "$@12", "$@13", "$@14", "$@15", "$@16", "$@17", "$@18", "$@19", "$@20",
  "$@21", "$@22", "$@23", "$@24", "$@25", "$@26", "$@27", "$@28", "$@29",
  "$@30", "$@31", "$@32", "$@33", "$@34", "$@35", "exprs_sub", "symbtree",
  "function", "func_args", "@36", "intfunc", "extfunc", "template",
  "solid", "offset", "string", "pv", "exprs", "$@37", "nexpr", "anyvars",
  "xsimsymb", "simsymb", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   124,    38,    61,
     372,    43,    45,    42,    47,    37,   373,   374,    40,    41,
      44
};
# endif

#define YYPACT_NINF -532

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-532)))

#define YYTABLE_NINF -473

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -532,    38,  -532,  -532,  -532,    60,    91,    86,  -532,  -532,
    -532,  1240,  -532,  -532,  -532,  -532,    -4,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,   462,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,   966,  1040,
    -532,  -532,  -532,  -532,   207,  -532,    92,   267,  1868,  1985,
    1713,    25,    84,  1985,  1985,  -532,    83,   -38,  1985,    70,
     328,  1985,   195,    28,   109,  1985,  1985,   136,   420,  -532,
    -532,  1566,   -38,  -532,  -532,  -532,  -532,   247,    92,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  1985,  -532,  1985,  -532,  1985,  -532,  1985,
    -532,  1985,  -532,  1985,  -532,  1985,  -532,  1985,  -532,  1985,
    -532,  1985,  -532,  1985,  2068,  -532,  -532,  -532,  -532,    90,
    -532,  -532,  -532,  -532,  -532,  -532,  1791,  -532,  -532,    -4,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
      94,   105,  -532,   376,  -532,   379,    49,  2106,  2068,  -532,
      -4,  2068,  -532,  -532,   194,   206,  -532,  -532,   137,  -532,
     -11,    29,   -11,  -532,  1595,  -532,   101,   213,   -38,  2068,
    -532,  -532,  1985,   209,  1985,  -532,  -532,  -532,  -532,  -532,
      32,   892,  -532,    77,  -532,  -532,   110,   -24,   471,    83,
    -532,    30,  -532,  -532,  -532,  -532,  1034,  -532,  1985,  -532,
     225,  -532,   285,   436,  1985,  -532,   228,  -532,   231,   270,
      -4,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  1791,
    -532,    90,  -532,  -532,    24,    46,  1985,  -532,  -532,  -532,
    -532,  -532,    26,  -532,   321,  1985,  -532,  -532,  -532,  1985,
    1985,    92,  1985,   474,  -532,    41,  -532,  -532,  -532,  1985,
      48,  -532,  -532,  -532,  1985,  -532,  1985,  -532,  -532,    83,
    -532,  -532,  -532,   -41,   -30,    -3,   120,    83,  -532,  -532,
      98,  -532,  -532,  2068,  -532,  1985,  -532,  -532,   323,  2068,
    -532,  -532,   214,  -532,  -532,  -532,  -532,  -532,    83,    29,
    -532,   340,  -532,  -532,   251,  1114,   259,    40,  -532,  2068,
    -532,  -532,  -532,  -532,  -532,  -532,    44,  -532,   391,  2068,
    -532,  -532,  -532,  1985,  1985,  1985,  1985,  1985,  1985,  1985,
    1985,  1985,  1985,  1985,  1985,  1985,  1985,  1985,  1985,  1985,
    1985,  1985,  1985,  1985,  1985,  1985,  1985,  1985,  1985,  -532,
    -532,  -532,  -532,  -532,   135,  -532,    56,   442,  -532,  -532,
    2106,  2106,    -4,  2106,  -532,  -532,   219,  1914,  -532,   454,
    -532,  -532,  -532,   266,   266,   266,   -29,  2026,   219,  -532,
     120,  -532,   120,  -532,   120,  -532,   221,  -532,   -38,  2068,
    -532,  -532,    29,  -532,  -532,  -532,  1641,  -532,   566,  -532,
     280,  -532,   461,  -532,   440,   286,   939,   939,   939,   939,
     939,   939,   939,   440,  -532,  2144,   939,   939,   939,   939,
     939,   939,  2144,  2182,   939,   390,   390,   286,   286,   286,
    1985,  -532,  -532,  -532,  -532,  -532,  -532,  -532,   318,   320,
    -532,  1366,   496,   692,  -532,   341,  -532,  -532,  -532,   266,
     266,   140,  -532,   266,   266,   104,  -532,   266,   266,   288,
     181,   190,   211,   266,  -532,   241,   245,   256,  -532,  -532,
     406,  -532,  -532,   268,  -532,  -532,  -532,  -532,  1492,  -532,
    -532,  1985,  1985,  1985,   496,  -532,   378,  -532,   266,   345,
     266,   353,   266,   345,   266,   359,   266,   353,   266,   359,
    -532,   257,  -532,   428,  -532,   428,   219,  -532,  -532,  -532,
    -532,  -532,   472,  2106,  2106,  2106,   496,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,   265,    22,   372,   374,    69,
    -532,  -532,  -532,   196,   202,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,   818,  -532,  -532,   266,  -532,
    -532,   266,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,   450,  -532,  -532,   166,   101,  -532,  -532,  -532,
    -532,  -532,  -532,  -532
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,    20,     1,     3,    23,    18,    23,     7,    11,
      10,    23,    21,    22,    20,    76,    17,   221,    19,   220,
       6,    72,    73,    64,    65,    66,    67,    68,    75,    69,
      77,    78,    80,    81,    82,    83,    84,    70,    71,    85,
      87,    74,    88,    86,    89,   118,    90,   327,   385,   383,
     362,   391,   397,   394,   400,   403,   406,   409,   380,   377,
     374,   429,   441,   447,   446,   430,   432,   431,    79,   412,
     415,   418,   421,   424,   427,   329,   331,   333,   335,   339,
     337,   345,   343,   341,   347,   368,   371,   388,     0,     0,
     356,   359,   365,   348,     0,    24,     0,     0,   130,   472,
       0,     0,     0,   472,   472,    91,     0,     0,     0,     0,
       0,   472,     0,     0,     0,   472,   472,     0,     0,   288,
     289,   472,     0,    29,    31,    32,    30,     0,     0,    33,
      34,    52,    35,    26,    27,    28,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    48,    47,    49,
      50,    51,    63,   472,    53,   472,    54,   472,    55,   472,
      56,   472,    57,   472,    58,   472,    61,   472,    59,   472,
      60,   472,    62,   472,   117,   428,   433,   444,   444,    16,
      20,   384,   436,   434,   437,   435,   472,   214,   215,    25,
     114,   311,   312,   309,   310,   463,   462,   464,   101,   113,
       0,     0,   103,     0,   196,   202,     0,   191,   471,   207,
     212,    20,   479,   478,     0,     0,   270,   271,     0,   450,
     467,   100,   467,   252,   449,   477,     0,     0,   474,   216,
     226,   236,   228,     0,   234,   239,   243,   253,   255,   258,
       0,     0,   257,   244,   256,   254,   247,   250,     0,     0,
     263,     0,   267,   268,   269,   278,     0,   305,     0,   304,
       0,   303,     0,     0,     0,   325,     0,   322,     0,     0,
      12,   326,   328,   330,   332,   334,   336,   338,   340,   342,
     344,   346,   381,   360,   389,   395,   392,   398,   401,   404,
     407,   378,   375,   372,   410,   413,   416,   419,   422,   425,
     366,   369,   386,   350,   352,   354,   357,   363,   443,   472,
     442,    15,   440,   349,     0,   470,   472,   105,   314,   315,
     316,   313,     0,   106,     0,   472,   128,   190,   127,     0,
       0,     0,     0,   378,   129,     0,   218,   223,    96,     0,
       0,   131,   132,   133,     0,    92,   472,    94,   465,     0,
     466,   460,   461,     0,     0,     0,     0,     0,   453,   459,
       0,   203,   473,   227,   233,     0,   237,   238,     0,   235,
     259,   261,     0,   245,   248,   246,   249,   242,     0,   241,
     264,     0,   283,   279,     0,     0,     0,   291,   282,   292,
     293,   295,   319,   318,   320,   317,     0,   296,     0,   321,
     323,   205,   121,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   445,
     439,   438,   468,   102,   115,   107,     0,     0,   111,   104,
     191,   191,    23,   191,   213,    20,   134,   134,   143,     0,
     140,   141,   142,   191,   191,   191,   139,     0,   134,   251,
       0,   454,     0,   452,     0,   451,     0,   448,   476,   232,
     229,   260,   240,   265,   276,   287,    20,   277,     0,   281,
       0,   297,     0,   301,   382,   361,   390,   396,   393,   399,
     402,   405,   408,   379,   376,   373,   411,   414,   417,   420,
     423,   426,   367,   370,   387,   351,   353,   355,   358,   364,
     472,   116,   308,   306,   307,   108,   200,   201,    23,     0,
      23,    23,   126,     0,    97,     0,    93,   138,   144,   191,
     191,   151,   145,   191,   191,   156,   146,   191,   191,   161,
       0,     0,     0,   191,    98,     0,     0,     0,   455,   475,
       0,   280,    23,     0,   275,   298,   469,     8,    23,   122,
       9,     0,     0,     0,   125,   211,   208,   135,   191,   147,
     191,   149,   191,   152,   191,   154,   191,   157,   191,   159,
     163,     0,   165,     0,   167,     0,   134,   458,   457,   456,
      20,   290,     0,   191,   191,   191,   124,    20,   148,   150,
     153,   155,   158,   160,   171,     0,     0,     0,     0,     0,
     162,   168,   177,     0,     0,   164,   172,   166,    99,    23,
     273,   197,   199,   198,   123,     0,   179,   178,   191,   181,
     180,   191,   183,   182,   185,   184,   170,   169,   174,   173,
     176,   175,   284,   210,   209,     0,     0,   285,   189,   192,
     193,   195,   194,   187
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -532,  -532,  -532,  -532,  -532,  -532,    -7,  -532,   -86,   -85,
    -206,    -1,  -425,   -10,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -242,  -237,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,   143,  -532,  -532,   -49,  -532,  -532,  -532,
    -440,  -532,   -51,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -418,  -407,  -400,  -532,  -109,  -531,  -532,  -532,   -70,  -532,
    -107,  -532,  -166,  -532,  -532,  -524,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -229,   253,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,   102,  -532,  -532,  -532,  -532,
     103,   400,   423,   427,   343,   284,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
      -9,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,
    -532,  -532,  -532,  -532,  -532,  -532,  -532,  -532,   246,   -98,
    -532,   381,  -532,  -532,  -532,  -104,  -532,   203,  -220,    95,
    -304,  -532,   -52,  -116,  -217,   -99
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,     7,   518,     8,     9,   519,    16,
       5,     6,    10,    11,    12,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,    17,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   218,   345,   446,   124,   125,
     316,   325,   198,   433,   126,   127,   128,   129,   205,   346,
     524,   525,   347,   449,   450,   451,   452,   528,   532,   536,
     453,   454,   455,   610,   615,   616,   630,   631,   627,   628,
     328,   648,   649,   650,   206,   564,   331,   130,   131,   132,
     133,   134,   135,   136,   137,    18,    19,   138,   139,   140,
     368,   141,   142,   248,   221,   249,   143,   144,   145,   146,
     147,   148,   554,   149,   387,   388,    13,   480,   150,   260,
     437,   199,   200,   201,   395,   396,   151,   266,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     208,   186,   424,   425,   426,   427,   404,   428,   421,   422,
     414,   413,   412,   403,   423,   405,   407,   406,   408,   409,
     410,   411,   415,   416,   417,   418,   419,   420,   313,   175,
     176,   308,   309,   177,   178,   223,   357,   358,   202,   224,
     314,   510,   315,   227,   228,   214
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      20,    94,   174,   215,   359,   335,   268,   526,   222,   360,
     252,   189,   434,   179,   385,   210,   222,   520,   544,   386,
     379,   434,   375,   629,   351,   430,   212,   435,   269,   250,
     352,   380,   256,   212,   225,   351,   533,   537,     3,   181,
     596,   352,   444,   270,   180,   481,   529,   209,   538,   448,
     611,   216,   217,   530,   534,   445,   251,   512,   220,   235,
      -5,    62,   351,   253,   254,   540,   541,   542,   352,   267,
     636,   230,   624,  -219,  -110,    40,  -262,    42,   637,   183,
     185,   376,   639,   641,   219,   212,    -4,   460,  -186,   207,
     226,   211,  -300,   557,  -186,   317,  -191,   213,   462,   229,
     334,   271,   225,   272,   213,   273,   323,   274,   436,   275,
     219,   276,   362,   277,   327,   278,   574,   279,  -225,   280,
     578,   281,   222,   570,   222,   464,   436,   520,   513,   568,
     576,  -467,  -224,   572,   514,   381,   318,   255,   338,    14,
      14,   370,  -191,  -191,  -191,   385,   618,   318,  -467,   472,
     478,   222,   220,   431,  -467,    62,   213,  -467,   226,   349,
    -467,  -467,   339,   340,   319,    15,    15,  -467,   332,   605,
     606,   607,   608,   225,  -467,   319,   432,   320,   220,   311,
    -467,    62,   580,   373,    14,   -95,  -467,  -467,   320,   372,
     321,   582,    62,  -467,   642,   336,   236,   638,   540,   237,
     542,   321,  -467,   640,  -467,  -467,   556,   337,   341,   342,
     364,  -467,   584,  -467,   361,   343,   374,   238,   239,   240,
     241,   187,   188,   363,   242,   369,   390,   468,  -467,   400,
    -467,  -467,   401,   456,   365,   541,   542,  -467,   225,   523,
     471,   243,  -217,   195,   196,   459,   442,   327,   212,   389,
     197,   222,   474,   467,  -222,   399,   327,  -231,   466,   222,
     477,  -204,   366,   367,   511,   344,   626,  -136,   190,   212,
     550,   402,   552,  -294,   244,   245,  -324,   327,  -191,  -206,
     222,  -191,  -191,  -191,  -191,  -191,   391,  -191,  -191,  -191,
    -191,  -191,  -191,  -191,   458,  -120,   605,   606,   607,   608,
     246,   247,   605,   606,   607,   608,   191,   192,  -191,  -191,
    -191,  -191,  -191,  -191,  -191,   348,  -274,   350,  -119,   213,
     440,   441,   438,   443,   470,   553,   392,   318,   393,   231,
     447,  -188,   327,   516,   517,   457,   522,  -188,   193,   194,
     213,   473,  -188,  -188,   195,   196,   531,   535,   539,  -188,
     548,   197,   549,   292,   604,   319,   469,   605,   606,   607,
     608,   609,   545,   -13,   546,    45,   547,   340,   320,  -112,
     587,  -230,   394,   632,   588,   634,   476,   326,   232,   233,
     234,   321,   540,   541,   619,   589,  -191,  -191,  -266,   329,
     330,   625,   483,   597,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,   502,   503,   504,   505,   506,   507,   508,   509,
     590,   257,   569,   571,  -191,   651,   573,   575,   652,   652,
     577,   579,   521,   581,   583,   585,   586,   397,  -188,  -302,
    -188,   542,   327,   515,  -188,   258,  -188,   283,   541,  -188,
    -188,  -188,  -188,   540,   592,   527,  -188,   292,  -188,   191,
     192,   598,   555,   599,   647,   600,   591,   601,   439,   602,
     559,   603,   377,   620,   567,   237,   617,   392,   318,   393,
     653,  -190,  -190,  -190,  -190,  -190,   621,   622,   623,   479,
    -109,   193,   194,   238,   239,   240,   241,   283,   259,   482,
     242,   378,  -137,   561,   562,   563,   319,   292,   558,  -299,
     560,    94,   174,   305,   306,   307,   566,    48,   261,   320,
    -272,   645,  -190,   394,   646,   612,   613,   614,   605,   606,
     607,   608,   321,    61,    62,    63,    64,   633,   635,    65,
      66,   262,   521,   322,   324,   263,    67,   398,    94,   174,
     244,   245,   593,   594,   595,   429,   461,   463,   465,   310,
       0,   303,   304,   305,   306,   307,     0,   551,     0,   -20,
     -20,   -20,   -20,     0,     0,     0,     0,     0,   -20,   -20,
     -20,   -20,   -20,   -20,   -20,   -20,   -20,   -20,     0,     0,
      93,     0,     0,   -20,     0,   -20,   -20,   -20,   -20,   -20,
     -20,   -20,   -20,   -20,   -20,     0,     0,     0,     0,   558,
     -20,   -20,   -20,   -20,   -20,     0,     0,     0,   644,     0,
       0,   -20,   -20,   -20,   -20,   -20,   -20,   -20,   -20,   -20,
     -20,     0,   -20,   -20,   -20,     0,     0,   -20,   -20,   -20,
     -20,   -20,     0,   -20,   -20,     0,     0,     0,     0,     0,
     -20,   -20,     0,     0,   -20,   -20,   -20,   -20,   -20,   -20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   -20,   -20,   -20,   -20,   -20,   -20,   -20,
     -20,   -20,   -20,   -20,   -20,   -20,     0,   -20,   -20,   -20,
     -20,   -20,     0,   565,   -20,   -23,   -23,   -23,   -23,     0,
       0,     0,     0,     0,   -23,   -23,   -23,   -23,   -23,   -23,
     -23,   -23,   -23,   -23,     0,     0,     0,     0,     0,   -23,
       0,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,
     -23,     0,     0,     0,     0,     0,   -23,   -23,   -23,   -23,
       0,     0,     0,     0,     0,     0,     0,   -23,   -23,   -23,
     -23,   -23,   -23,   -23,   -23,   -23,   -23,     0,   -23,   -23,
     -23,     0,     0,   -23,   -23,   -23,   -23,     0,     0,   -23,
     -23,     0,     0,     0,     0,     0,   -23,   -23,     0,     0,
     -23,   -23,   -23,   -23,   -23,   -23,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   -23,
     -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,
     -23,   -23,     0,   -23,   -23,   -23,   -23,   -23,     0,   643,
     -23,   -23,   -23,   -23,   -23,     0,     0,     0,     0,     0,
     -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,
       0,     0,     0,     0,     0,   -23,     0,   -23,   -23,   -23,
     -23,   -23,   -23,   -23,   -23,   -23,   -23,     0,     0,     0,
       0,     0,   -23,   -23,   -23,   -23,     0,     0,     0,     0,
       0,     0,     0,   -23,   -23,   -23,   -23,   -23,   -23,   -23,
     -23,   -23,   -23,     0,   -23,   -23,   -23,     0,     0,   -23,
     -23,   -23,   -23,   371,     0,   -23,   -23,     0,     0,     0,
       0,     0,   -23,   -23,     0,     0,   -23,   -23,   -23,   -23,
     -23,   -23,     0,     0,     0,     0,     0,     0,  -472,     0,
       0,     0,     0,     0,     0,   -23,   -23,   -23,   -23,   -23,
     -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,     0,   -23,
     -23,   -23,   -23,   -23,     0,     0,   -23,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,     0,    58,    59,
      60,     0,     0,    61,    62,    63,    64,   182,     0,    65,
      66,     0,     0,     0,     0,     0,    67,     0,     0,     0,
      69,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   282,   283,     0,     0,     0,
       0,     0,     0,     0,     0,   291,   292,     0,     0,    85,
      86,    87,     0,    88,    89,    90,    91,    92,     0,     0,
      93,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,     0,    58,    59,    60,   382,     0,    61,    62,    63,
      64,   184,     0,    65,    66,     0,     0,     0,   383,     0,
      67,     0,     0,     0,    69,    70,    71,    72,    73,    74,
     303,   304,   305,   306,   307,     0,     0,     0,     0,    40,
       0,    42,     0,     0,     0,     0,     0,     0,     0,   384,
       0,     0,   180,    85,    86,    87,     0,    88,    89,    90,
      91,    92,     0,     0,    93,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,     0,    58,    59,    60,     0,
       0,    61,    62,    63,    64,   475,     0,    65,    66,     0,
       0,     0,     0,     0,    67,     0,     0,     0,    69,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    85,    86,    87,
       0,    88,    89,    90,    91,    92,     0,     0,    93,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,     0,
      58,    59,    60,     0,     0,    61,    62,    63,    64,     0,
       0,    65,    66,     0,     0,     0,     0,     0,    67,     0,
       0,     0,    69,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    85,    86,    87,     0,    88,    89,    90,    91,    92,
       0,     0,    93,    21,    22,    23,    24,     0,     0,     0,
       0,     0,    25,    26,     0,     0,    27,    28,    29,    30,
      31,    32,     0,     0,     0,     0,     0,    33,     0,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,     0,
       0,     0,     0,     0,    44,    45,    46,    47,     0,     0,
       0,     0,     0,     0,     0,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,     0,    58,    59,    60,     0,
       0,    61,    62,    63,    64,     0,     0,    65,    66,     0,
       0,     0,     0,     0,    67,    68,     0,     0,    69,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
       0,    88,    89,    90,    91,    92,     0,     0,    93,    21,
      22,    23,    24,     0,     0,     0,     0,     0,    25,    26,
       0,     0,    27,    28,    29,    30,    31,    32,     0,     0,
       0,     0,     0,    33,     0,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,     0,     0,     0,     0,     0,
      44,   -14,    46,    47,     0,     0,     0,     0,     0,     0,
       0,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,     0,    58,    59,    60,     0,     0,    61,    62,    63,
      64,     0,     0,    65,    66,     0,     0,     0,     0,     0,
      67,    68,     0,     0,    69,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,     0,    88,    89,    90,
      91,    92,     0,     0,    93,    21,    22,    23,    24,     0,
       0,     0,     0,     0,    25,    26,     0,     0,    27,    28,
      29,    30,    31,    32,     0,     0,     0,     0,     0,    33,
       0,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,     0,     0,     0,     0,     0,    44,     0,    46,    47,
       0,     0,     0,     0,     0,     0,     0,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,     0,    58,    59,
      60,     0,     0,    61,    62,    63,    64,     0,     0,    65,
      66,     0,     0,     0,     0,     0,    67,    68,     0,     0,
      69,    70,    71,    72,    73,    74,     0,     0,     0,     0,
       0,   264,     0,     0,     0,     0,     0,     0,     0,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,     0,    88,    89,    90,    91,    92,     0,     0,
      93,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,     0,    58,    59,    60,     0,     0,    61,    62,    63,
      64,     0,     0,    65,    66,     0,     0,   265,     0,     0,
      67,     0,     0,     0,    69,    70,    71,    72,    73,    74,
     351,     0,     0,     0,     0,     0,   352,     0,     0,     0,
       0,     0,   195,   196,     0,     0,  -286,     0,  -286,   197,
       0,     0,     0,    85,    86,    87,  -286,    88,    89,    90,
      91,    92,     0,     0,    93,     0,     0,   282,   283,   284,
     285,   286,   287,   288,   289,   290,     0,   291,   292,   293,
       0,     0,     0,     0,   353,     0,   354,   355,     0,     0,
       0,     0,     0,   356,     0,     0,     0,     0,     0,   294,
     295,   296,   297,   298,   299,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   300,   301,
     302,    14,   303,   304,   305,   306,   307,     0,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,     0,    58,
      59,    60,     0,     0,    61,    62,    63,    64,     0,     0,
      65,    66,     0,     0,     0,     0,     0,    67,     0,     0,
       0,    69,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      85,    86,    87,     0,    88,    89,    90,    91,    92,   312,
       0,    93,     0,     0,     0,     0,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,     0,    58,    59,    60,
       0,     0,    61,    62,    63,    64,     0,     0,    65,    66,
       0,     0,     0,     0,     0,    67,     0,     0,     0,    69,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    85,    86,
      87,     0,    88,    89,    90,    91,    92,   203,     0,    93,
       0,     0,     0,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,     0,    58,    59,    60,     0,     0,    61,
      62,    63,    64,     0,   204,    65,    66,     0,     0,     0,
       0,     0,    67,     0,     0,     0,    69,    70,    71,    72,
      73,    74,  -136,     0,     0,     0,     0,     0,     0,     0,
     282,   283,   284,   285,   286,   287,   288,   289,   290,     0,
     291,   292,   293,     0,     0,    85,    86,    87,     0,    88,
      89,    90,    91,    92,     0,     0,    93,     0,     0,     0,
       0,     0,   294,   295,   296,   297,   298,   299,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   300,   301,   302,     0,   303,   304,   305,   306,   307,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
       0,    58,    59,    60,     0,     0,    61,    62,    63,    64,
       0,     0,    65,    66,     0,     0,     0,     0,     0,    67,
       0,     0,     0,    69,    70,    71,    72,    73,    74,     0,
       0,     0,   282,   283,   284,   285,   286,   287,   288,   289,
     290,     0,   291,   292,   293,     0,     0,     0,     0,     0,
       0,     0,    85,    86,    87,     0,    88,    89,    90,    91,
      92,     0,     0,    93,   294,   295,   296,   297,   298,   299,
       0,     0,     0,     0,   282,   283,   284,   285,   286,   287,
     288,   289,   290,     0,   291,   292,   293,     0,     0,     0,
       0,     0,     0,   300,   301,   302,     0,   303,   304,   305,
     306,   307,     0,     0,     0,   543,   294,   295,   296,   297,
     298,   299,   282,   283,   284,   285,   286,   287,   288,   289,
     290,     0,   333,   292,   293,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   300,   301,   302,     0,   303,
     304,   305,   306,   307,   294,   295,   296,   297,   298,   299,
     282,   283,   284,   285,   286,   287,   288,   289,   290,     0,
     291,   292,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   300,   301,   302,     0,   303,   304,   305,
     306,   307,   294,   295,   296,   297,   298,   299,   282,   283,
     284,   285,   286,   287,   288,   289,   290,     0,   291,   292,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   301,   302,     0,   303,   304,   305,   306,   307,
     294,   295,   296,   297,   298,   299,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   302,     0,   303,   304,   305,   306,   307
};

static const yytype_int16 yycheck[] =
{
       7,    11,    11,   102,   224,   211,   122,   447,   106,   226,
     114,    96,   316,    14,   256,   100,   114,   442,   458,   256,
     249,   325,    46,     1,    65,     1,     1,     1,   127,     1,
      71,     1,   117,     1,    72,    65,   454,   455,     0,    48,
     564,    71,     1,   128,    48,     1,   453,    99,   455,     1,
     581,   103,   104,   453,   454,    14,    28,     1,    69,   111,
       0,    72,    65,   115,   116,    94,    95,    96,    71,   121,
       1,     1,   596,    48,    48,    35,    48,    37,   609,    88,
      89,   105,   613,   614,     1,     1,     0,   128,    66,    98,
     128,   100,    48,   518,    72,     1,    48,    72,   128,   108,
     207,   153,    72,   155,    72,   157,     1,   159,    82,   161,
       1,   163,   228,   165,    66,   167,   534,   169,    48,   171,
     538,   173,   220,   530,   222,   128,    82,   552,    72,   529,
     537,    48,    48,   533,    78,   251,    42,     1,     1,    48,
      48,   240,    94,    95,    96,   387,   586,    42,    65,   378,
     387,   249,    69,   129,    71,    72,    72,    48,   128,   130,
      77,    78,    25,    26,    70,    75,    75,    84,   119,   100,
     101,   102,   103,    72,    65,    70,   130,    83,    69,   180,
      71,    72,     1,   106,    48,    48,    77,    78,    83,   241,
      96,     1,    72,    84,   619,     1,     1,     1,    94,     4,
      96,    96,   119,     1,   121,   122,   510,     1,    71,    72,
       1,   128,     1,   130,     1,    78,   106,    22,    23,    24,
      25,    14,    15,   232,    29,   234,     1,   129,   119,     1,
     121,   122,     1,   340,    25,    95,    96,   128,    72,   445,
      26,    46,    48,    77,    78,   349,   331,    66,     1,   258,
      84,   349,     1,   357,    48,   264,    66,    48,   356,   357,
       1,    48,    53,    54,   129,   128,     1,    48,     1,     1,
     476,     1,   478,    48,    79,    80,    48,    66,    97,    48,
     378,   100,   101,   102,   103,   104,     1,    97,    98,    99,
     100,   101,   102,   103,   346,    48,   100,   101,   102,   103,
     105,   106,   100,   101,   102,   103,    39,    40,    97,    98,
      99,   100,   101,   102,   103,   220,    48,   222,    48,    72,
     329,   330,     1,   332,     1,    45,    41,    42,    43,     1,
     339,    66,    66,   440,   441,   344,   443,    72,    71,    72,
      72,     1,    77,    78,    77,    78,   453,   454,   455,    84,
     129,    84,   468,    67,    97,    70,   365,   100,   101,   102,
     103,   104,   460,    45,   462,    45,   464,    26,    83,    48,
     129,    48,    87,     1,   129,     1,   385,     1,    50,    51,
      52,    96,    94,    95,   590,   129,    10,    11,    48,    10,
      11,   597,     1,    15,   403,   404,   405,   406,   407,   408,
     409,   410,   411,   412,   413,   414,   415,   416,   417,   418,
     419,   420,   421,   422,   423,   424,   425,   426,   427,   428,
      14,     1,   529,   530,    48,   645,   533,   534,   645,   646,
     537,   538,   442,   540,   541,   542,   543,     1,    66,    48,
      66,    96,    66,     1,    72,    25,    72,    57,    95,    77,
      78,    77,    78,    94,   553,     1,    84,    67,    84,    39,
      40,   568,     1,   570,    14,   572,   552,   574,   325,   576,
     519,   578,     1,     1,   525,     4,   585,    41,    42,    43,
     646,     7,     8,     9,    10,    11,   593,   594,   595,   387,
      48,    71,    72,    22,    23,    24,    25,    57,    78,   396,
      29,   248,    48,     7,     8,     9,    70,    67,   518,    48,
     520,   521,   521,   123,   124,   125,   523,    55,   118,    83,
      48,   628,    48,    87,   631,    97,    98,    99,   100,   101,
     102,   103,    96,    71,    72,    73,    74,   607,   608,    77,
      78,   118,   552,   200,   201,   118,    84,   263,   558,   558,
      79,    80,   561,   562,   563,   309,   353,   354,   355,   178,
      -1,   121,   122,   123,   124,   125,    -1,     1,    -1,     3,
       4,     5,     6,    -1,    -1,    -1,    -1,    -1,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    -1,    -1,
     128,    -1,    -1,    27,    -1,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,   619,
      44,    45,    46,    47,    48,    -1,    -1,    -1,   625,    -1,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    -1,    -1,    71,    72,    73,
      74,    75,    -1,    77,    78,    -1,    -1,    -1,    -1,    -1,
      84,    85,    -1,    -1,    88,    89,    90,    91,    92,    93,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,    -1,   121,   122,   123,
     124,   125,    -1,     1,   128,     3,     4,     5,     6,    -1,
      -1,    -1,    -1,    -1,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    -1,    -1,    -1,    -1,    44,    45,    46,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    -1,    -1,    71,    72,    73,    74,    -1,    -1,    77,
      78,    -1,    -1,    -1,    -1,    -1,    84,    85,    -1,    -1,
      88,    89,    90,    91,    92,    93,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,    -1,   121,   122,   123,   124,   125,    -1,     1,
     128,     3,     4,     5,     6,    -1,    -1,    -1,    -1,    -1,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      -1,    -1,    44,    45,    46,    47,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    -1,    -1,    71,
      72,    73,    74,     1,    -1,    77,    78,    -1,    -1,    -1,
      -1,    -1,    84,    85,    -1,    -1,    88,    89,    90,    91,
      92,    93,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,
      -1,    -1,    -1,    -1,    -1,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,    -1,   121,
     122,   123,   124,   125,    -1,    -1,   128,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    -1,    -1,    71,    72,    73,    74,     1,    -1,    77,
      78,    -1,    -1,    -1,    -1,    -1,    84,    -1,    -1,    -1,
      88,    89,    90,    91,    92,    93,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    56,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    67,    -1,    -1,   117,
     118,   119,    -1,   121,   122,   123,   124,   125,    -1,    -1,
     128,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,     1,    -1,    71,    72,    73,
      74,     1,    -1,    77,    78,    -1,    -1,    -1,    14,    -1,
      84,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
     121,   122,   123,   124,   125,    -1,    -1,    -1,    -1,    35,
      -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    48,   117,   118,   119,    -1,   121,   122,   123,
     124,   125,    -1,    -1,   128,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    -1,
      -1,    71,    72,    73,    74,     1,    -1,    77,    78,    -1,
      -1,    -1,    -1,    -1,    84,    -1,    -1,    -1,    88,    89,
      90,    91,    92,    93,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   117,   118,   119,
      -1,   121,   122,   123,   124,   125,    -1,    -1,   128,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    -1,    -1,    71,    72,    73,    74,    -1,
      -1,    77,    78,    -1,    -1,    -1,    -1,    -1,    84,    -1,
      -1,    -1,    88,    89,    90,    91,    92,    93,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   117,   118,   119,    -1,   121,   122,   123,   124,   125,
      -1,    -1,   128,     3,     4,     5,     6,    -1,    -1,    -1,
      -1,    -1,    12,    13,    -1,    -1,    16,    17,    18,    19,
      20,    21,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    -1,    -1,    -1,    44,    45,    46,    47,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    -1,
      -1,    71,    72,    73,    74,    -1,    -1,    77,    78,    -1,
      -1,    -1,    -1,    -1,    84,    85,    -1,    -1,    88,    89,
      90,    91,    92,    93,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
      -1,   121,   122,   123,   124,   125,    -1,    -1,   128,     3,
       4,     5,     6,    -1,    -1,    -1,    -1,    -1,    12,    13,
      -1,    -1,    16,    17,    18,    19,    20,    21,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,
      44,    45,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    -1,    -1,    71,    72,    73,
      74,    -1,    -1,    77,    78,    -1,    -1,    -1,    -1,    -1,
      84,    85,    -1,    -1,    88,    89,    90,    91,    92,    93,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,    -1,   121,   122,   123,
     124,   125,    -1,    -1,   128,     3,     4,     5,     6,    -1,
      -1,    -1,    -1,    -1,    12,    13,    -1,    -1,    16,    17,
      18,    19,    20,    21,    -1,    -1,    -1,    -1,    -1,    27,
      -1,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    -1,    -1,    -1,    -1,    44,    -1,    46,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      68,    -1,    -1,    71,    72,    73,    74,    -1,    -1,    77,
      78,    -1,    -1,    -1,    -1,    -1,    84,    85,    -1,    -1,
      88,    89,    90,    91,    92,    93,    -1,    -1,    -1,    -1,
      -1,    25,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,    -1,   121,   122,   123,   124,   125,    -1,    -1,
     128,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    -1,    -1,    71,    72,    73,
      74,    -1,    -1,    77,    78,    -1,    -1,    81,    -1,    -1,
      84,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      65,    -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,    -1,
      -1,    -1,    77,    78,    -1,    -1,    35,    -1,    37,    84,
      -1,    -1,    -1,   117,   118,   119,    45,   121,   122,   123,
     124,   125,    -1,    -1,   128,    -1,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      -1,    -1,    -1,    -1,   119,    -1,   121,   122,    -1,    -1,
      -1,    -1,    -1,   128,    -1,    -1,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   117,   118,
     119,    48,   121,   122,   123,   124,   125,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    -1,    -1,    71,    72,    73,    74,    -1,    -1,
      77,    78,    -1,    -1,    -1,    -1,    -1,    84,    -1,    -1,
      -1,    88,    89,    90,    91,    92,    93,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     117,   118,   119,    -1,   121,   122,   123,   124,   125,    48,
      -1,   128,    -1,    -1,    -1,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      -1,    -1,    71,    72,    73,    74,    -1,    -1,    77,    78,
      -1,    -1,    -1,    -1,    -1,    84,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   117,   118,
     119,    -1,   121,   122,   123,   124,   125,    49,    -1,   128,
      -1,    -1,    -1,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    -1,    -1,    71,
      72,    73,    74,    -1,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    84,    -1,    -1,    -1,    88,    89,    90,    91,
      92,    93,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    -1,    -1,   117,   118,   119,    -1,   121,
     122,   123,   124,   125,    -1,    -1,   128,    -1,    -1,    -1,
      -1,    -1,    88,    89,    90,    91,    92,    93,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   117,   118,   119,    -1,   121,   122,   123,   124,   125,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    -1,    -1,    71,    72,    73,    74,
      -1,    -1,    77,    78,    -1,    -1,    -1,    -1,    -1,    84,
      -1,    -1,    -1,    88,    89,    90,    91,    92,    93,    -1,
      -1,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   117,   118,   119,    -1,   121,   122,   123,   124,
     125,    -1,    -1,   128,    88,    89,    90,    91,    92,    93,
      -1,    -1,    -1,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    67,    68,    -1,    -1,    -1,
      -1,    -1,    -1,   117,   118,   119,    -1,   121,   122,   123,
     124,   125,    -1,    -1,    -1,   129,    88,    89,    90,    91,
      92,    93,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,   118,   119,    -1,   121,
     122,   123,   124,   125,    88,    89,    90,    91,    92,    93,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   117,   118,   119,    -1,   121,   122,   123,
     124,   125,    88,    89,    90,    91,    92,    93,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   118,   119,    -1,   121,   122,   123,   124,   125,
      88,    89,    90,    91,    92,    93,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   119,    -1,   121,   122,   123,   124,   125
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   132,   133,     0,   134,   141,   142,   135,   137,   138,
     143,   144,   145,   247,    48,    75,   140,   160,   226,   227,
     137,     3,     4,     5,     6,    12,    13,    16,    17,    18,
      19,    20,    21,    27,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    44,    45,    46,    47,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    66,    67,
      68,    71,    72,    73,    74,    77,    78,    84,    85,    88,
      89,    90,    91,    92,    93,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   121,   122,
     123,   124,   125,   128,   144,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   179,   180,   185,   186,   187,   188,
     218,   219,   220,   221,   222,   223,   224,   225,   228,   229,
     230,   232,   233,   237,   238,   239,   240,   241,   242,   244,
     249,   257,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   310,   311,   314,   315,   142,
      48,   281,     1,   281,     1,   281,   282,    14,    15,   140,
       1,    39,    40,    71,    72,    77,    78,    84,   183,   252,
     253,   254,   319,    49,    76,   189,   215,   281,   281,   323,
     140,   281,     1,    72,   326,   326,   323,   323,   176,     1,
      69,   235,   310,   316,   320,    72,   128,   324,   325,   281,
       1,     1,    50,    51,    52,   323,     1,     4,    22,    23,
      24,    25,    29,    46,    79,    80,   105,   106,   234,   236,
       1,    28,   316,   323,   323,     1,   140,     1,    25,    78,
     250,   252,   253,   254,    25,    81,   258,   323,   324,   326,
     140,   323,   323,   323,   323,   323,   323,   323,   323,   323,
     323,   323,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    66,    67,    68,    88,    89,    90,    91,    92,    93,
     117,   118,   119,   121,   122,   123,   124,   125,   312,   313,
     312,   142,    48,   309,   321,   323,   181,     1,    42,    70,
      83,    96,   255,     1,   255,   182,     1,    66,   211,    10,
      11,   217,   119,    66,   211,   141,     1,     1,     1,    25,
      26,    71,    72,    78,   128,   177,   190,   193,   320,   130,
     320,    65,    71,   119,   121,   122,   128,   317,   318,   319,
     325,     1,   324,   281,     1,    25,    53,    54,   231,   281,
     326,     1,   323,   106,   106,    46,   105,     1,   236,   235,
       1,   324,     1,    14,    45,   171,   172,   245,   246,   281,
       1,     1,    41,    43,    87,   255,   256,     1,   256,   281,
       1,     1,     1,   294,   287,   296,   298,   297,   299,   300,
     301,   302,   293,   292,   291,   303,   304,   305,   306,   307,
     308,   289,   290,   295,   283,   284,   285,   286,   288,   309,
       1,   129,   130,   184,   321,     1,    82,   251,     1,   184,
     281,   281,   140,   281,     1,    14,   178,   281,     1,   194,
     195,   196,   197,   201,   202,   203,   211,   281,   323,   316,
     128,   318,   128,   318,   128,   318,   310,   316,   129,   281,
       1,    26,   235,     1,     1,     1,   281,     1,   172,   246,
     248,     1,   251,     1,   281,   281,   281,   281,   281,   281,
     281,   281,   281,   281,   281,   281,   281,   281,   281,   281,
     281,   281,   281,   281,   281,   281,   281,   281,   281,   281,
     322,   129,     1,    72,    78,     1,   211,   211,   136,   139,
     143,   144,   211,   141,   191,   192,   191,     1,   198,   202,
     203,   211,   199,   201,   203,   211,   200,   201,   202,   211,
      94,    95,    96,   129,   191,   310,   310,   310,   129,   324,
     141,     1,   141,    45,   243,     1,   321,   143,   144,   187,
     144,     7,     8,     9,   216,     1,   137,   193,   203,   211,
     202,   211,   203,   211,   201,   211,   202,   211,   201,   211,
       1,   211,     1,   211,     1,   211,   211,   129,   129,   129,
      14,   139,   326,   281,   281,   281,   216,    15,   211,   211,
     211,   211,   211,   211,    97,   100,   101,   102,   103,   104,
     204,   206,    97,    98,    99,   205,   206,   205,   191,   141,
       1,   211,   211,   211,   216,   141,     1,   209,   210,     1,
     207,   208,     1,   209,     1,   209,     1,   206,     1,   206,
       1,   206,   143,     1,   137,   211,   211,    14,   212,   213,
     214,   319,   325,   213
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   131,   133,   132,   134,   134,   135,   135,   136,   136,
     137,   137,   138,   139,   139,   140,   140,   141,   141,   142,
     142,   143,   143,   144,   145,   146,   146,   146,   146,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   176,   175,   177,   177,   177,   178,   177,   177,   177,
     179,   181,   180,   182,   180,   180,   180,   180,   180,   180,
     180,   180,   180,   183,   183,   184,   184,   185,   186,   187,
     187,   187,   188,   189,   189,   189,   189,   189,   189,   189,
     189,   190,   190,   190,   192,   191,   191,   193,   193,   193,
     194,   194,   194,   194,   195,   196,   197,   198,   198,   198,
     198,   198,   199,   199,   199,   199,   199,   200,   200,   200,
     200,   200,   201,   201,   202,   202,   203,   203,   204,   204,
     204,   204,   205,   205,   205,   205,   205,   205,   206,   206,
     206,   206,   206,   206,   206,   206,   208,   207,   210,   209,
     211,   211,   212,   212,   213,   214,   215,   216,   216,   216,
     217,   217,   217,   218,   218,   219,   219,   220,   221,   221,
     221,   221,   221,   221,   222,   223,   224,   225,   225,   225,
     226,   227,   228,   228,   228,   229,   229,   230,   230,   230,
     230,   230,   230,   230,   230,   230,   230,   231,   231,   232,
     233,   233,   233,   233,   234,   234,   234,   234,   234,   234,
     234,   235,   235,   236,   236,   236,   236,   236,   236,   236,
     236,   236,   237,   237,   237,   237,   237,   238,   239,   240,
     241,   242,   243,   243,   243,   244,   244,   244,   244,   244,
     244,   245,   245,   245,   246,   246,   246,   246,   247,   247,
     248,   248,   249,   249,   249,   249,   249,   249,   249,   249,
     249,   249,   249,   250,   250,   250,   251,   251,   251,   252,
     252,   253,   254,   255,   255,   255,   255,   256,   256,   256,
     256,   257,   257,   257,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   282,   281,
     283,   281,   284,   281,   285,   281,   281,   286,   281,   281,
     287,   281,   281,   288,   281,   281,   289,   281,   281,   290,
     281,   281,   291,   281,   281,   292,   281,   281,   293,   281,
     281,   294,   281,   281,   281,   281,   295,   281,   281,   296,
     281,   281,   297,   281,   281,   298,   281,   281,   299,   281,
     281,   300,   281,   281,   301,   281,   281,   302,   281,   281,
     303,   281,   281,   304,   281,   281,   305,   281,   281,   306,
     281,   281,   307,   281,   281,   308,   281,   281,   281,   281,
     281,   281,   281,   281,   281,   281,   281,   281,   309,   309,
     309,   310,   311,   311,   313,   312,   314,   315,   316,   316,
     316,   317,   317,   317,   317,   317,   317,   317,   317,   317,
     318,   318,   319,   319,   319,   320,   320,   320,   322,   321,
     321,   323,   323,   324,   324,   324,   324,   325,   326,   326
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     1,     2,     1,     2,     2,
       1,     1,     3,     1,     1,     3,     2,     2,     1,     2,
       0,     1,     1,     0,     2,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     3,     3,     1,     0,     0,     3,     3,     5,
       2,     0,     4,     0,     4,     3,     3,     4,     5,     4,
       3,     4,     3,     1,     1,     1,     2,     1,     1,     2,
       1,     3,     6,     7,     6,     5,     4,     2,     2,     2,
       0,     1,     1,     1,     0,     2,     0,     2,     3,     2,
       1,     1,     1,     1,     2,     2,     2,     2,     3,     2,
       3,     1,     2,     3,     2,     3,     1,     2,     3,     2,
       3,     1,     4,     3,     4,     3,     4,     3,     1,     2,
       2,     1,     1,     2,     2,     2,     2,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     0,     3,     0,     3,
       1,     0,     1,     1,     1,     1,     1,     3,     3,     3,
       3,     3,     0,     3,     2,     3,     2,     2,     6,     9,
       9,     6,     2,     4,     2,     2,     2,     2,     3,     1,
       1,     1,     2,     3,     1,     1,     2,     3,     2,     4,
       3,     2,     4,     3,     2,     3,     2,     1,     1,     2,
       4,     3,     3,     2,     1,     2,     2,     1,     2,     2,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     2,
       3,     2,     1,     2,     3,     4,     3,     2,     2,     2,
       2,     2,     2,     3,     1,     5,     4,     4,     2,     3,
       5,     2,     1,     1,     6,     7,     2,     2,     2,     2,
       3,     0,     3,     3,     2,     3,     3,     4,     5,     4,
       3,     4,     3,     1,     1,     1,     2,     2,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     2,     3,     2,     1,     2,     1,     2,     1,
       2,     1,     2,     1,     2,     1,     2,     1,     2,     1,
       2,     1,     2,     1,     2,     1,     2,     1,     0,     3,
       0,     4,     0,     4,     0,     4,     1,     0,     4,     1,
       0,     4,     1,     0,     4,     1,     0,     4,     1,     0,
       4,     1,     0,     4,     1,     0,     4,     1,     0,     4,
       1,     0,     4,     1,     2,     1,     0,     4,     1,     0,
       4,     1,     0,     4,     1,     0,     4,     1,     0,     4,
       1,     0,     4,     1,     0,     4,     1,     0,     4,     1,
       0,     4,     1,     0,     4,     1,     0,     4,     1,     0,
       4,     1,     0,     4,     1,     0,     4,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     2,     2,     2,     2,
       1,     1,     2,     2,     0,     2,     1,     1,     3,     1,
       1,     2,     2,     1,     2,     3,     4,     4,     4,     1,
       1,     1,     1,     1,     1,     2,     2,     0,     0,     4,
       1,     1,     0,     2,     1,     4,     3,     1,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 160 "../yaccsrc.y" /* yacc.c:1646  */
    { level = 0;
                                         if ( get_options_flag( parser_data.TSD->currlevel, EXT_CALLS_AS_FUNCS )
                                           && !get_options_flag( parser_data.TSD->currlevel, EXT_STRICT_ANSI ) )
                                            start_parendepth = 1;
                                         else
                                            start_parendepth = 0;
                                         parendepth = 0; }
#line 2354 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 4:
#line 170 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = optgluelast( (yyvsp[-1]), (yyvsp[0]) );
                                         (yyval)->o.last = NULL;
                                         EndProg( (yyval) ) ; }
#line 2362 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 5:
#line 173 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]);
                                         if ( (yyval) != NULL )
                                            (yyval)->o.last = NULL;
                                         EndProg( (yyval) ); }
#line 2371 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 6:
#line 179 "../yaccsrc.y" /* yacc.c:1646  */
    { /* fixes bug 579711 */
                                         (yyval) = optgluelast( (yyvsp[-1]), (yyvsp[0]) ); }
#line 2378 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 7:
#line 181 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2384 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 8:
#line 184 "../yaccsrc.y" /* yacc.c:1646  */
    { /* fixes bug 579711 */
                                         (yyval) = optgluelast( (yyvsp[-1]), (yyvsp[0]) ); }
#line 2391 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 9:
#line 186 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 2397 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 10:
#line 189 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 2403 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 11:
#line 190 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_UNMATCHED_END, 1 ); }
#line 2409 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 13:
#line 198 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2415 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 14:
#line 199 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = NULL; }
#line 2421 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 15:
#line 202 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = optgluelast( (yyvsp[-2]), (yyvsp[0]) ); }
#line 2427 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 16:
#line 203 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2433 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 17:
#line 206 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = optgluelast( (yyvsp[-1]), (yyvsp[0]) ); }
#line 2439 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 18:
#line 207 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2445 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 19:
#line 210 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = optgluelast( (yyvsp[-1]), (yyvsp[0]) ); }
#line 2451 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 20:
#line 211 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = NULL; }
#line 2457 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 23:
#line 218 "../yaccsrc.y" /* yacc.c:1646  */
    { tmpchr=parser_data.tstart;
                                         tmplno=parser_data.tline; }
#line 2464 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 24:
#line 222 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 2470 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 25:
#line 225 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = optgluelast( (yyvsp[-1]), (yyvsp[0]) ); }
#line 2476 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 64:
#line 268 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_CALL,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2484 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 65:
#line 272 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_DO,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         level++; }
#line 2493 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 66:
#line 277 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_EXIT,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2501 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 67:
#line 281 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_IF,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         level++; }
#line 2510 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 68:
#line 286 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_ITERATE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2518 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 69:
#line 290 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_LEAVE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2526 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 70:
#line 294 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_SAY,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2534 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 71:
#line 298 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_RETURN,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2542 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 72:
#line 302 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_ADDR_N,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2550 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 73:
#line 306 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_PARSE_ARG,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2558 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 74:
#line 310 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_DROP,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2566 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 75:
#line 314 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_IPRET,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2574 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 76:
#line 318 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_LABEL,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2582 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 77:
#line 322 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_NULL,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2590 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 78:
#line 326 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(0,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2598 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 79:
#line 330 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_OPTIONS,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2606 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 80:
#line 334 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(0,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2614 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 81:
#line 338 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_PROC,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2622 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 82:
#line 342 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_PULL,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2630 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 83:
#line 346 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_PUSH,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2638 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 84:
#line 350 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_QUEUE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2646 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 85:
#line 354 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_SELECT,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         level++; }
#line 2655 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 86:
#line 359 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_SIG_LAB,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2663 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 87:
#line 363 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_WHEN,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2671 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 88:
#line 367 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_OTHERWISE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2679 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 89:
#line 371 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_TRACE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2687 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 90:
#line 375 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_UPPER_VAR,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 2695 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 91:
#line 379 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = current = (yyvsp[0]) ; }
#line 2701 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 93:
#line 383 "../yaccsrc.y" /* yacc.c:1646  */
    { current->type = X_ADDR_V ;
                                         current->p[0] = (yyvsp[-1]) ;
                                         current->p[1] = (yyvsp[0]) ; }
#line 2709 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 94:
#line 386 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_STRING_EXPECTED, 1, __reginatext ) ;}
#line 2715 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 95:
#line 387 "../yaccsrc.y" /* yacc.c:1646  */
    { current->type = X_ADDR_S ; }
#line 2721 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 96:
#line 388 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_STRING_EXPECTED, 1, __reginatext ) ;}
#line 2727 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 98:
#line 390 "../yaccsrc.y" /* yacc.c:1646  */
    { current->name = (streng *)(yyvsp[-2]) ;
                                         current->type = X_ADDR_N ;
                                         current->p[0] = (yyvsp[-1]) ;
                                         current->p[1] = (yyvsp[0]) ; }
#line 2736 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 99:
#line 394 "../yaccsrc.y" /* yacc.c:1646  */
    { current->type = X_ADDR_V ;
                                         current->p[0] = (yyvsp[-3]) ;
                                         current->p[1] = (yyvsp[0]) ;
                                         current->u.nonansi = 1 ; }
#line 2745 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 100:
#line 400 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode( X_PARSE, 2, (yyvsp[-1]), (yyvsp[0]) );
                                         (yyval)->u.parseflags = PARSE_UPPER;
                                         (yyval)->lineno = (yyvsp[-1])->lineno;
                                         (yyval)->charnr = (yyvsp[-1])->charnr; }
#line 2754 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 101:
#line 406 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth = start_parendepth; }
#line 2760 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 102:
#line 407 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]);
                                         (yyval)->p[0] = (yyvsp[0]);
                                         (yyval)->name = (streng *) (yyvsp[-2]);
                                         parendepth = 0; }
#line 2769 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 103:
#line 411 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth = start_parendepth; }
#line 2775 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 104:
#line 412 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]);
                                         (yyval)->type = X_EX_FUNC;
                                         (yyval)->p[0] = (yyvsp[0]);
                                         (yyval)->name = (streng *) (yyvsp[-2]);
                                         parendepth = 0; }
#line 2785 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 105:
#line 417 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 1, "ERROR FAILURE HALT NOTREADY", __reginatext ) ;}
#line 2791 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 106:
#line 418 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 2, "ERROR FAILURE HALT NOTREADY", __reginatext ) ;}
#line 2797 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 107:
#line 419 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
#line 2803 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 108:
#line 420 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_STRING_EXPECTED, 3, __reginatext ) ;}
#line 2809 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 109:
#line 422 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]) ;
                                         (yyval)->type = X_CALL_SET ;
                                         (yyval)->p[0] = (yyvsp[-2]) ;
                                         (yyval)->name = (streng *)(yyvsp[0]) ;
                                         (yyval)->p[1] = (yyvsp[-1]) ; }
#line 2819 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 110:
#line 427 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_CALL_SET ;
                                         (yyval)->p[0] = (yyvsp[-1]) ;
                                         (yyval)->name = NULL ;
                                         (yyval)->p[1] = (yyvsp[0]) ; }
#line 2829 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 111:
#line 432 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
#line 2835 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 112:
#line 433 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_CALL_SET ;
                                         (yyval)->p[0] = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ; }
#line 2844 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 113:
#line 439 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 2850 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 114:
#line 440 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_STRING_EXPECTED, 2, __reginatext );}
#line 2856 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 115:
#line 443 "../yaccsrc.y" /* yacc.c:1646  */
    {
                                         /*
                                          * "call_args" accepted probably with
                                          * surrounding parentheses. Strip them.
                                          */
                                         (yyval) = reduce_expr_list( (yyvsp[0]),
                                                                REDUCE_CALL );
                                       }
#line 2869 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 116:
#line 451 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror(ERR_UNEXPECTED_PARAN, 2); }
#line 2875 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 117:
#line 454 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_COMMAND,0) ;
                                         (yyval)->charnr = tmpchr ;
                                         (yyval)->lineno = tmplno;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 2884 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 118:
#line 460 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_END,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         level--; }
#line 2893 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 119:
#line 466 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->name = (streng*)((yyvsp[0])) ; }
#line 2900 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 120:
#line 468 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 2906 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 121:
#line 469 "../yaccsrc.y" /* yacc.c:1646  */
    {  exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
#line 2912 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 122:
#line 473 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-5]);
                                         (yyval)->p[0] = (yyvsp[-4]);
                                         (yyval)->p[1] = (yyvsp[-3]);
                                         (yyval)->p[2] = optgluelast( (yyvsp[-2]), (yyvsp[-1]) );
                                         if ( (yyval)->p[2] )
                                            (yyval)->p[2]->o.last = NULL;
                                         (yyval)->p[3] = (yyvsp[0]);
                          if (((yyval)->p[0]==NULL || (yyval)->p[0]->name==NULL)
                              && (yyval)->p[3]->name)
                                            exiterror( ERR_UNMATCHED_END, 0 );
                          if (((yyval)->p[0])&&((yyval)->p[0]->name)&&
                              ((yyval)->p[3]->name)&&
                              (((yyval)->p[3]->name->len != (yyval)->p[0]->name->len)||
                               (strncmp((yyval)->p[3]->name->value,
                                        (yyval)->p[0]->name->value,
                                        (yyval)->p[0]->name->len))))
                                            exiterror( ERR_UNMATCHED_END, 0 );
                                       }
#line 2935 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 123:
#line 494 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) =makenode(X_REP,4,(yyvsp[-4]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0])) ;
                                         (yyval)->name = (streng *)(yyvsp[-6]) ;
                                         checkdosyntax((yyval)) ; }
#line 2943 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 124:
#line 498 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) =makenode(X_REP,3,(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0])) ;
                                         (yyval)->name = (streng *)(yyvsp[-5]) ;
                                         checkdosyntax((yyval)) ; }
#line 2951 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 125:
#line 502 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_REP,2,(yyvsp[-2]),(yyvsp[0])) ;
                                         (yyval)->name = (streng *)(yyvsp[-4]) ;
                                         checkdosyntax((yyval)) ; }
#line 2959 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 126:
#line 505 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_REP,1,(yyvsp[-1])) ;
                                         (yyval)->name = (streng *)(yyvsp[-3]) ;
                                         checkdosyntax((yyval)) ; }
#line 2967 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 127:
#line 508 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_REP_FOREVER,0) ; }
#line 2973 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 128:
#line 509 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 16, "WHILE UNTIL", __reginatext ) ; }
#line 2979 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 129:
#line 510 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyvsp[-1]) = makenode(X_DO_EXPR,1,(yyvsp[-1])) ;
                                         (yyval) = makenode(X_REP,2,NULL,(yyvsp[-1])) ; }
#line 2986 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 130:
#line 512 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = NULL ; }
#line 2992 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 131:
#line 515 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
#line 2998 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 132:
#line 516 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
#line 3004 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 133:
#line 517 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
#line 3010 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 134:
#line 520 "../yaccsrc.y" /* yacc.c:1646  */
    { SymbolDetect |= SD_ADDRWITH ;
                                         (yyval) = with = makenode(X_ADDR_WITH,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 3019 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 135:
#line 524 "../yaccsrc.y" /* yacc.c:1646  */
    { with = NULL ;
                                         SymbolDetect &= ~SD_ADDRWITH ; }
#line 3026 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 136:
#line 526 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = NULL ; }
#line 3032 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 137:
#line 529 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3038 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 138:
#line 530 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 5, __reginatext ) ; }
#line 3044 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 139:
#line 531 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 5, __reginatext ) ; }
#line 3050 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 143:
#line 537 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 5, __reginatext ) ; }
#line 3056 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 162:
#line 573 "../yaccsrc.y" /* yacc.c:1646  */
    { with->p[0] = (yyvsp[0]); }
#line 3062 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 163:
#line 574 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 6, __reginatext ) ; }
#line 3068 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 164:
#line 577 "../yaccsrc.y" /* yacc.c:1646  */
    { with->p[1] = (yyvsp[0]); }
#line 3074 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 165:
#line 578 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 7, __reginatext ) ; }
#line 3080 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 166:
#line 581 "../yaccsrc.y" /* yacc.c:1646  */
    { with->p[2] = (yyvsp[0]); }
#line 3086 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 167:
#line 582 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 14, __reginatext ) ; }
#line 3092 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 168:
#line 585 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 3098 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 169:
#line 586 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ;
                                         (yyval)->u.of.noeol = 1 ; }
#line 3105 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 170:
#line 588 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 8, __reginatext ) ; }
#line 3111 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 171:
#line 589 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_ADDR_WITH, 0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 3119 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 172:
#line 594 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 3125 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 173:
#line 595 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ;
                                         (yyval)->u.of.append = 1 ; }
#line 3132 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 174:
#line 597 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 8, __reginatext ) ; }
#line 3138 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 175:
#line 598 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 3144 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 176:
#line 599 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 9, __reginatext ) ; }
#line 3150 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 177:
#line 600 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_ADDR_WITH, 0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 3158 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 178:
#line 605 "../yaccsrc.y" /* yacc.c:1646  */
    { /* ANSI extension: nsimsymb is
                                          * used by the standard, but I think
                                          * there are no reasons why using
                                          * it here as a must. FGC
                                          */
                                         (yyval) = (yyvsp[0]) ;
                                         (yyval)->u.of.awt = awtSTREAM;
                                         SymbolDetect |= SD_ADDRWITH ; }
#line 3171 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 179:
#line 613 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_OPTION, 1, __reginatext ) ; }
#line 3177 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 180:
#line 614 "../yaccsrc.y" /* yacc.c:1646  */
    {
                                         streng *tmp = (yyvsp[0])->name;
                                         char *p;

                                         /*
                                          * expect a single dot as the last character
                                          */
                                         p = (char *)memchr( tmp->value, '.', tmp->len );
                                         if ( p != tmp->value + tmp->len - 1 )
                                            exiterror( ERR_INVALID_OPTION, 3, __reginatext );
                                         (yyval) = (yyvsp[0]) ;
                                         (yyval)->u.of.awt = awtSTEM ;
                                         SymbolDetect |= SD_ADDRWITH ; }
#line 3195 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 181:
#line 627 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_OPTION, 2, __reginatext ) ; }
#line 3201 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 182:
#line 628 "../yaccsrc.y" /* yacc.c:1646  */
    {
                                         (yyval) = (yyvsp[0]) ;
                                         (yyval)->u.of.awt = awtLIFO ;
                                         SymbolDetect |= SD_ADDRWITH ; }
#line 3210 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 183:
#line 632 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_OPTION, 100, __reginatext ) ; }
#line 3216 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 184:
#line 633 "../yaccsrc.y" /* yacc.c:1646  */
    {
                                         (yyval) = (yyvsp[0]) ;
                                         (yyval)->u.of.awt = awtFIFO ;
                                         SymbolDetect |= SD_ADDRWITH ; }
#line 3225 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 185:
#line 637 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_OPTION, 101, __reginatext ) ; }
#line 3231 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 186:
#line 640 "../yaccsrc.y" /* yacc.c:1646  */
    { SymbolDetect &= ~SD_ADDRWITH ; }
#line 3237 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 187:
#line 641 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 3243 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 188:
#line 644 "../yaccsrc.y" /* yacc.c:1646  */
    { SymbolDetect &= ~SD_ADDRWITH ; }
#line 3249 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 189:
#line 645 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 3255 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 192:
#line 652 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 3261 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 193:
#line 653 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 3267 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 194:
#line 656 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_ADDR_WITH, 0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->u.of.ant = antSIMSYMBOL;
                                       }
#line 3278 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 195:
#line 664 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_ADDR_WITH, 0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->u.of.ant = antSTRING;
                                       }
#line 3289 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 196:
#line 672 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
#line 3295 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 197:
#line 675 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_DO_TO,1,(yyvsp[-1])) ; }
#line 3301 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 198:
#line 676 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_DO_FOR,1,(yyvsp[-1])) ; }
#line 3307 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 199:
#line 677 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_DO_BY,1,(yyvsp[-1])) ; }
#line 3313 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 200:
#line 680 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_WHILE,1,(yyvsp[-1])) ; }
#line 3319 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 201:
#line 681 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_UNTIL,1,(yyvsp[-1])) ; }
#line 3325 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 202:
#line 682 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = NULL ; }
#line 3331 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 203:
#line 685 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_SYMBOL_EXPECTED, 1, __reginatext ) ;}
#line 3337 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 204:
#line 686 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 3344 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 205:
#line 690 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_SYMBOL_EXPECTED, 1, __reginatext ) ;}
#line 3350 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 206:
#line 691 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 3357 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 207:
#line 695 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 3364 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 208:
#line 700 "../yaccsrc.y" /* yacc.c:1646  */
    { move_labels( (yyvsp[-5]), (yyvsp[0]), level - 1 );
                                         (yyval) = (yyvsp[-5]);
                                         (yyval)->p[0] = optgluelast( (yyvsp[-4]), (yyvsp[-3]) );
                                         (yyval)->p[0]->o.last = NULL;
                                         (yyval)->p[1] = optgluelast( (yyvsp[-1]), (yyvsp[0]) );
                                         (yyval)->p[1]->o.last = NULL;
                                         level--; }
#line 3376 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 209:
#line 708 "../yaccsrc.y" /* yacc.c:1646  */
    { move_labels( (yyvsp[-8]), (yyvsp[0]), level - 1 );
                                         (yyval) = (yyvsp[-8]);
                                         (yyval)->p[0] = optgluelast( (yyvsp[-7]), (yyvsp[-6]) );
                                         (yyval)->p[0]->o.last = NULL;
                                         (yyval)->p[1] = optgluelast( (yyvsp[-4]), (yyvsp[-3]) );
                                         (yyval)->p[1]->o.last = NULL;
                                         (yyval)->p[2] = optgluelast( (yyvsp[-1]), (yyvsp[0]) );
                                         (yyval)->p[2]->o.last = NULL;
                                         level--; }
#line 3390 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 210:
#line 718 "../yaccsrc.y" /* yacc.c:1646  */
    {  exiterror( ERR_INCOMPLETE_STRUCT, 4 ) ;}
#line 3396 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 211:
#line 720 "../yaccsrc.y" /* yacc.c:1646  */
    {  exiterror( ERR_INCOMPLETE_STRUCT, 3 ) ;}
#line 3402 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 212:
#line 721 "../yaccsrc.y" /* yacc.c:1646  */
    {  exiterror( ERR_INCOMPLETE_STRUCT, 0 ) ;}
#line 3408 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 213:
#line 722 "../yaccsrc.y" /* yacc.c:1646  */
    {  exiterror( ERR_THEN_EXPECTED, 1, parser_data.if_linenr, __reginatext )  ; }
#line 3414 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 214:
#line 725 "../yaccsrc.y" /* yacc.c:1646  */
    {  exiterror( ERR_THEN_UNEXPECTED, 1 )  ; }
#line 3420 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 215:
#line 728 "../yaccsrc.y" /* yacc.c:1646  */
    {  exiterror( ERR_THEN_UNEXPECTED, 2 )  ; }
#line 3426 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 216:
#line 731 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 3433 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 217:
#line 736 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; }
#line 3440 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 218:
#line 738 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
#line 3446 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 219:
#line 739 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 3452 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 220:
#line 742 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ;
                                         (yyval)->u.trace_only =
                                                         (level == 0) ? 0 : 1;
                                         newlabel( (const tsd_t *)parser_data.TSD,
                                                   &parser_data,
                                                   (yyvsp[0]) ) ; }
#line 3463 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 221:
#line 750 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ;
                                         (yyval)->name = Str_cre_TSD(parser_data.TSD,retvalue) ; }
#line 3470 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 222:
#line 754 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; }
#line 3477 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 223:
#line 756 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
#line 3483 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 224:
#line 757 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 3489 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 225:
#line 760 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 3495 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 226:
#line 761 "../yaccsrc.y" /* yacc.c:1646  */
    {  exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
#line 3501 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 227:
#line 764 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_NUM_D ;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 3509 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 228:
#line 767 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval)->type = X_NUM_DDEF ; }
#line 3515 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 229:
#line 768 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
#line 3521 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 230:
#line 769 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_NUM_F ;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 3529 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 231:
#line 772 "../yaccsrc.y" /* yacc.c:1646  */
    { /* NOTE! This clashes ANSI! */
                                         (yyval) = (yyvsp[-1]) ; (yyval)->type=X_NUM_FRMDEF ;}
#line 3536 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 232:
#line 774 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]) ; (yyval)->type=X_NUM_V ;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 3543 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 233:
#line 776 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 11, "ENGINEERING SCIENTIFIC", __reginatext ) ;}
#line 3549 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 234:
#line 777 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); (yyval)->type = X_NUM_FDEF ;}
#line 3555 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 235:
#line 778 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_NUM_FUZZ ;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 3563 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 236:
#line 781 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 15, "DIGITS FORM FUZZ", __reginatext ) ;}
#line 3569 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 237:
#line 784 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_NUM_SCI,0) ; }
#line 3575 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 238:
#line 785 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_NUM_ENG,0) ; }
#line 3581 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 239:
#line 788 "../yaccsrc.y" /* yacc.c:1646  */
    { ((yyval)=(yyvsp[-1]))->p[0]=(yyvsp[0]) ; }
#line 3587 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 240:
#line 792 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]) ;
                                         (yyval)->type = X_PARSE ;
                                         (yyval)->u.parseflags = (int) (yyvsp[-2]) ;
                                         (yyval)->p[0] = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ; }
#line 3597 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 241:
#line 798 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_PARSE ;
                                         (yyval)->u.parseflags = 0;
                                         (yyval)->p[0] = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ; }
#line 3607 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 242:
#line 803 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 12, "ARG EXTERNAL LINEIN PULL SOURCE VAR VALUE VERSION", __reginatext ) ;}
#line 3613 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 243:
#line 804 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 12, "ARG CASELESS EXTERNAL LINEIN LOWER PULL SOURCE UPPER VAR VALUE VERSION", __reginatext ) ;}
#line 3619 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 244:
#line 807 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr) (PARSE_UPPER  |
                                                         PARSE_NORMAL); }
#line 3626 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 245:
#line 809 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr) (PARSE_UPPER  |
                                                         PARSE_CASELESS); }
#line 3633 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 246:
#line 811 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr) (PARSE_UPPER  |
                                                         PARSE_CASELESS); }
#line 3640 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 247:
#line 813 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr) (PARSE_LOWER  |
                                                         PARSE_NORMAL); }
#line 3647 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 248:
#line 815 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr) (PARSE_LOWER  |
                                                         PARSE_CASELESS); }
#line 3654 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 249:
#line 817 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr) (PARSE_LOWER  |
                                                         PARSE_CASELESS); }
#line 3661 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 250:
#line 819 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr) (PARSE_NORMAL |
                                                         PARSE_CASELESS); }
#line 3668 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 251:
#line 823 "../yaccsrc.y" /* yacc.c:1646  */
    { /* fixes bugs like bug 579711 */
                                         (yyval) = optgluelast( (yyvsp[-2]), (yyvsp[0]) ); }
#line 3675 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 252:
#line 825 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 3681 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 253:
#line 828 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_PARSE_ARG,0) ; }
#line 3687 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 254:
#line 829 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_PARSE_EXT,0) ; }
#line 3693 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 255:
#line 830 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_PARSE_EXT,0) ; }
#line 3699 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 256:
#line 831 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_PARSE_VER,0) ; }
#line 3705 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 257:
#line 832 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_PARSE_PULL,0) ; }
#line 3711 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 258:
#line 833 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_PARSE_SRC,0) ; }
#line 3717 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 259:
#line 834 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_PARSE_VAR,0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; }
#line 3724 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 260:
#line 836 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_PARSE_VAL,1,(yyvsp[-1])) ; }
#line 3730 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 261:
#line 837 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_TEMPLATE, 3 ) ;}
#line 3736 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 262:
#line 840 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 3742 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 263:
#line 841 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 17, __reginatext ) ;}
#line 3748 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 264:
#line 842 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_SYMBOL_EXPECTED, 1, __reginatext ) ;}
#line 3754 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 265:
#line 843 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_SYMBOL_EXPECTED, 1, __reginatext ) ;}
#line 3760 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 266:
#line 844 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 3767 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 267:
#line 848 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 3774 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 268:
#line 852 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 3781 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 269:
#line 856 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 3788 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 270:
#line 860 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 3795 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 271:
#line 864 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 3802 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 272:
#line 868 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_UNMATCHED_END, 0 ) ;}
#line 3808 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 273:
#line 869 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
#line 3814 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 274:
#line 870 "../yaccsrc.y" /* yacc.c:1646  */
    { level--; }
#line 3820 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 275:
#line 874 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-4]);
                                         (yyval)->p[0] = optgluelast( (yyvsp[-3]), (yyvsp[-2]) );
                                         (yyval)->p[0]->o.last = NULL;
                                         (yyval)->p[1] = (yyvsp[-1]); }
#line 3829 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 276:
#line 878 "../yaccsrc.y" /* yacc.c:1646  */
    {  exiterror( ERR_WHEN_EXPECTED, 1, parser_data.select_linenr, __reginatext ) ;}
#line 3835 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 277:
#line 880 "../yaccsrc.y" /* yacc.c:1646  */
    {  exiterror( ERR_WHEN_EXPECTED, 2, parser_data.select_linenr, __reginatext ) ;}
#line 3841 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 278:
#line 881 "../yaccsrc.y" /* yacc.c:1646  */
    {  exiterror( ERR_EXTRA_DATA, 1, __reginatext )  ;}
#line 3847 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 279:
#line 882 "../yaccsrc.y" /* yacc.c:1646  */
    {  exiterror( ERR_THEN_UNEXPECTED, 0 ) ;}
#line 3853 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 280:
#line 884 "../yaccsrc.y" /* yacc.c:1646  */
    {  exiterror( ERR_INCOMPLETE_STRUCT, 0 ) ;}
#line 3859 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 281:
#line 887 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = optgluelast( (yyvsp[-1]), (yyvsp[0]) ); }
#line 3865 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 282:
#line 888 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 3871 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 283:
#line 889 "../yaccsrc.y" /* yacc.c:1646  */
    {  exiterror( ERR_WHEN_EXPECTED, 2, parser_data.select_linenr, __reginatext )  ;}
#line 3877 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 284:
#line 893 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-5]); /* fixes bugs like bug 579711 */
                                         (yyval)->p[0] = optgluelast( (yyvsp[-4]), (yyvsp[-3]) );
                                         (yyval)->p[0]->o.last = NULL;
                                         (yyval)->p[1] = optgluelast( (yyvsp[-1]), (yyvsp[0]) );
                                         (yyval)->p[1]->o.last = NULL; }
#line 3887 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 285:
#line 899 "../yaccsrc.y" /* yacc.c:1646  */
    {  exiterror( ERR_THEN_UNEXPECTED, 0 ) ;}
#line 3893 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 286:
#line 900 "../yaccsrc.y" /* yacc.c:1646  */
    {  exiterror( ERR_THEN_EXPECTED, 2, parser_data.select_linenr, __reginatext )  ; }
#line 3899 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 287:
#line 901 "../yaccsrc.y" /* yacc.c:1646  */
    {  exiterror( ERR_INVALID_EXPRESSION, 0 ) ;}
#line 3905 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 288:
#line 911 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_WHEN_UNEXPECTED, 1 ); }
#line 3911 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 289:
#line 912 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_WHEN_UNEXPECTED, 2 ); }
#line 3917 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 290:
#line 915 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]);
                                         (yyval)->p[0] = optgluelast( (yyvsp[-1]), (yyvsp[0]) );
                                         if ( (yyval)->p[0] )
                                            (yyval)->p[0]->o.last = NULL; }
#line 3926 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 291:
#line 919 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_NO_OTHERWISE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
#line 3934 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 292:
#line 925 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_SIG_VAL ;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 3942 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 293:
#line 928 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
#line 3948 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 294:
#line 929 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->name = (streng *)(yyvsp[0]) ; }
#line 3955 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 295:
#line 931 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 3, "ERROR FAILURE HALT NOTREADY NOVALUE SYNTAX LOSTDIGITS", __reginatext ) ;}
#line 3961 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 296:
#line 932 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INV_SUBKEYWORD, 4, "ERROR FAILURE HALT NOTREADY NOVALUE SYNTAX LOSTDIGITS", __reginatext ) ;}
#line 3967 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 297:
#line 933 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
#line 3973 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 298:
#line 934 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_STRING_EXPECTED, 3, __reginatext ) ;}
#line 3979 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 299:
#line 936 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-3]) ;
                                         (yyval)->type = X_SIG_SET ;
                                         (yyval)->p[0] = (yyvsp[-2]) ;
                                         (yyval)->name = (streng *)(yyvsp[0]) ;
                                         (yyval)->p[1] = (yyvsp[-1]) ; }
#line 3989 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 300:
#line 941 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_SIG_SET ;
                                         (yyval)->p[0] = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ; }
#line 3998 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 301:
#line 945 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
#line 4004 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 302:
#line 946 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_SIG_SET ;
                                         (yyval)->p[0] = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ; }
#line 4013 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 303:
#line 952 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 4019 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 304:
#line 953 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
#line 4025 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 305:
#line 954 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_STRING_EXPECTED, 4, __reginatext );}
#line 4031 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 306:
#line 957 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue);}
#line 4037 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 307:
#line 958 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
#line 4043 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 308:
#line 959 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_STRING_EXPECTED, 3, __reginatext ) ;}
#line 4049 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 309:
#line 962 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
#line 4055 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 310:
#line 963 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
#line 4061 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 311:
#line 966 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_ON,0) ; }
#line 4067 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 312:
#line 969 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_OFF,0) ; }
#line 4073 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 313:
#line 972 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_S_ERROR,0) ; }
#line 4079 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 314:
#line 973 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_S_HALT,0) ; }
#line 4085 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 315:
#line 974 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_S_NOTREADY,0) ; }
#line 4091 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 316:
#line 975 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_S_FAILURE,0) ; }
#line 4097 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 317:
#line 978 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 4103 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 318:
#line 979 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_S_NOVALUE,0) ; }
#line 4109 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 319:
#line 980 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_S_SYNTAX,0) ; }
#line 4115 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 320:
#line 981 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_S_LOSTDIGITS,0) ; }
#line 4121 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 321:
#line 984 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 4128 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 322:
#line 986 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; }
#line 4135 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 323:
#line 988 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
#line 4141 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 324:
#line 989 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; }
#line 4148 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 325:
#line 993 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
#line 4154 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 326:
#line 997 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ;
                                         /*
                                          * An assignment is a numerical
                                          * assignment if and only if we have
                                          * to do a numerical operation, which
                                          * is equivalent to the existence of
                                          * one more argument to $2.
                                          * This fixes bug 720166.
                                          */
                                         if ((yyvsp[0]) &&
                                             (yyvsp[0])->p[0] &&
                                             gettypeof((yyvsp[0])) == IS_A_NUMBER)
                                            (yyval)->type = X_NASSIGN ; }
#line 4173 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 327:
#line 1013 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_ASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
#line 4182 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 328:
#line 1019 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ;
                                       }
#line 4190 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 329:
#line 1024 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_PLUSASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
#line 4199 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 330:
#line 1030 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ;
                                       }
#line 4207 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 331:
#line 1035 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_MINUSASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
#line 4216 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 332:
#line 1041 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ;
                                       }
#line 4224 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 333:
#line 1046 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_MULTASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
#line 4233 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 334:
#line 1052 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ;
                                       }
#line 4241 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 335:
#line 1057 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_DIVASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
#line 4250 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 336:
#line 1063 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ;
                                       }
#line 4258 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 337:
#line 1068 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_INTDIVASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
#line 4267 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 338:
#line 1074 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ;
                                       }
#line 4275 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 339:
#line 1079 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_MODULUSASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
#line 4284 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 340:
#line 1085 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ;
                                       }
#line 4292 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 341:
#line 1090 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_ANDASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
#line 4301 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 342:
#line 1096 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ;
                                       }
#line 4309 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 343:
#line 1101 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_XORASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
#line 4318 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 344:
#line 1107 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ;
                                       }
#line 4326 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 345:
#line 1112 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_ORASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
#line 4335 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 346:
#line 1118 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ;
                                       }
#line 4343 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 347:
#line 1123 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_CONCATASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
#line 4352 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 348:
#line 1130 "../yaccsrc.y" /* yacc.c:1646  */
    { /* We have to accept exprs here even
                                          * if we just want to accept
                                          * '(' expr ')' only. We do this
                                          * because we appect
                                          * "call '(' exprs ')'" too.
                                          * This kann happen only if the
                                          * related control flag parendepth is
                                          * set. But since the parentheses are
                                          * voided just for the outer ones, we
                                          * can reduce the allowness level.
                                          * We don't have to set it back,
                                          * because the outer parentheses
                                          * either is THE one we look for or
                                          * none. This allows a faster error
                                          * detection and that's a good goal.*/
                                         parendepth--; }
#line 4373 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 349:
#line 1146 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++;
                                         if ( parendepth == 1 )
                                         {
                                            /* exprs on as-is basis */
                                            (yyval) = (yyvsp[0]);
                                         }
                                         else
                                         {
                                            /* Must already be a plain expr.
                                             * The nexpr part of exprs detects
                                             * mistakes. */
                                            (yyval) = reduce_expr_list( (yyvsp[0]),
                                                                REDUCE_EXPR );
                                            if ( (yyval) == (yyvsp[0]) )
                                               exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "Reduction of `exprs' not happened." );
                                         }
                                       }
#line 4395 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 350:
#line 1163 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4401 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 351:
#line 1164 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "+" );
                                         (yyval) = makenode( X_PLUSS, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4408 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 352:
#line 1166 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4414 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 353:
#line 1167 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "-" );
                                         (yyval) = makenode( X_MINUS, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4421 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 354:
#line 1169 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4427 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 355:
#line 1170 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "*" );
                                         (yyval) = makenode( X_MULT, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4434 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 356:
#line 1172 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "*" ); }
#line 4440 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 357:
#line 1173 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4446 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 358:
#line 1174 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "/" );
                                         (yyval) = makenode( X_DEVIDE, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4453 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 359:
#line 1176 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "/" ); }
#line 4459 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 360:
#line 1177 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4465 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 361:
#line 1178 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "//" );
                                         (yyval) = makenode( X_MODULUS, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4472 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 362:
#line 1180 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "//" ); }
#line 4478 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 363:
#line 1181 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4484 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 364:
#line 1182 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "%" );
                                         (yyval) = makenode( X_INTDIV, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4491 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 365:
#line 1184 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "%" ); }
#line 4497 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 366:
#line 1185 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4503 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 367:
#line 1186 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "|" );
                                         (yyval) = makenode( X_LOG_OR, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4510 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 368:
#line 1188 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "|" ); }
#line 4516 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 369:
#line 1189 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4522 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 370:
#line 1190 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "&" );
                                         (yyval) = makenode( X_LOG_AND, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4529 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 371:
#line 1192 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "&" ); }
#line 4535 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 372:
#line 1193 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4541 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 373:
#line 1194 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "&&" );
                                         (yyval) = makenode( X_LOG_XOR, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4548 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 374:
#line 1196 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "&&" ); }
#line 4554 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 375:
#line 1197 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4560 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 376:
#line 1198 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "**" );
                                         (yyval) = makenode( X_EXP, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4567 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 377:
#line 1200 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "**" ); }
#line 4573 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 378:
#line 1201 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4579 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 379:
#line 1202 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), " " );
                                         (yyval) = makenode( X_SPACE, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4586 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 380:
#line 1204 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, " " ); }
#line 4592 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 381:
#line 1205 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4598 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 382:
#line 1206 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "||" );
                                         (yyval) = makenode( X_CONCAT, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4605 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 383:
#line 1208 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "||" ); }
#line 4611 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 384:
#line 1209 "../yaccsrc.y" /* yacc.c:1646  */
    { AUTO_REDUCE( (yyvsp[0]), "\\" );
                                         (yyval) = makenode( X_LOG_NOT, 1, (yyvsp[0]) ); }
#line 4618 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 385:
#line 1211 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "\\" ); }
#line 4624 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 386:
#line 1212 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4630 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 387:
#line 1213 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "=" );
                                         (yyval) = makenode( X_EQUAL, 2, (yyvsp[-3]), (yyvsp[0]) );
                                         transform( (yyval) ); }
#line 4638 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 388:
#line 1216 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "=" ); }
#line 4644 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 389:
#line 1217 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4650 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 390:
#line 1218 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), ">=" );
                                         (yyval) = makenode( X_GTE, 2, (yyvsp[-3]), (yyvsp[0]) );
                                         transform( (yyval) ) ; }
#line 4658 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 391:
#line 1221 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, ">=" ); }
#line 4664 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 392:
#line 1222 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4670 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 393:
#line 1223 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "<=" );
                                         (yyval) = makenode( X_LTE, 2, (yyvsp[-3]), (yyvsp[0]) );
                                         transform( (yyval) ) ; }
#line 4678 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 394:
#line 1226 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "<=" ); }
#line 4684 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 395:
#line 1227 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4690 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 396:
#line 1228 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), ">" );
                                         (yyval) = makenode( X_GT, 2, (yyvsp[-3]), (yyvsp[0]) );
                                         transform( (yyval) ) ; }
#line 4698 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 397:
#line 1231 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, ">" ); }
#line 4704 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 398:
#line 1232 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4710 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 399:
#line 1233 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "<" );
                                         (yyval) = makenode( X_LT, 2, (yyvsp[-3]), (yyvsp[0]) );
                                         transform( (yyval) ) ; }
#line 4718 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 400:
#line 1236 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "<" ); }
#line 4724 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 401:
#line 1237 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4730 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 402:
#line 1238 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "\\=" );
                                         (yyval) = makenode( X_DIFF, 2, (yyvsp[-3]), (yyvsp[0]) );
                                         transform( (yyval) ) ; }
#line 4738 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 403:
#line 1241 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "\\=" ); }
#line 4744 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 404:
#line 1242 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4750 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 405:
#line 1243 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "==" );
                                         (yyval) = makenode( X_S_EQUAL, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4757 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 406:
#line 1245 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "==" ); }
#line 4763 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 407:
#line 1246 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4769 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 408:
#line 1247 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "\\==" );
                                         (yyval) = makenode( X_S_DIFF, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4776 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 409:
#line 1249 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "\\==" ); }
#line 4782 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 410:
#line 1250 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4788 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 411:
#line 1251 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), ">>" );
                                         (yyval) = makenode( X_S_GT, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4795 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 412:
#line 1253 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, ">>" ); }
#line 4801 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 413:
#line 1254 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4807 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 414:
#line 1255 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "<<" );
                                         (yyval) = makenode( X_S_LT, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4814 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 415:
#line 1257 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "<<" ); }
#line 4820 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 416:
#line 1258 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4826 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 417:
#line 1259 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "\\>>" );
                                         (yyval) = makenode( X_S_NGT, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4833 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 418:
#line 1261 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "\\>>" ); }
#line 4839 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 419:
#line 1262 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4845 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 420:
#line 1263 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "\\<<" );
                                         (yyval) = makenode( X_S_NLT, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4852 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 421:
#line 1265 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "\\<<" ); }
#line 4858 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 422:
#line 1266 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4864 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 423:
#line 1267 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), ">>=" );
                                         (yyval) = makenode( X_S_GTE, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4871 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 424:
#line 1269 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, ">>=" ); }
#line 4877 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 425:
#line 1270 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth--; }
#line 4883 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 426:
#line 1271 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "<<=" );
                                         (yyval) = makenode( X_S_LTE, 2, (yyvsp[-3]), (yyvsp[0]) ); }
#line 4890 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 427:
#line 1273 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, "<<=" ); }
#line 4896 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 428:
#line 1274 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 4902 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 429:
#line 1275 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode( X_STRING, 0 );
                                         (yyval)->name = Str_cre_TSD(parser_data.TSD,retvalue) ; }
#line 4909 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 430:
#line 1277 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode( X_STRING, 0 );
                                         (yyval)->name = Str_make_TSD(parser_data.TSD,retlength) ;
                                         memcpy((yyval)->name->value,retvalue,
                                                    (yyval)->name->len=retlength); }
#line 4918 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 431:
#line 1281 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode( X_STRING, 0 );
                                         (yyval)->name = Str_make_TSD(parser_data.TSD,retlength) ;
                                         memcpy((yyval)->name->value,retvalue,
                                                    (yyval)->name->len=retlength); }
#line 4927 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 432:
#line 1285 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode( X_STRING, 0 );
                                         (yyval)->name = Str_cre_TSD(parser_data.TSD,retvalue) ; }
#line 4934 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 433:
#line 1287 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 4940 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 434:
#line 1288 "../yaccsrc.y" /* yacc.c:1646  */
    { AUTO_REDUCE( (yyvsp[0]), nullptr );
                                         (yyval) = makenode( X_U_PLUSS, 1, (yyvsp[0]) ); }
#line 4947 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 435:
#line 1290 "../yaccsrc.y" /* yacc.c:1646  */
    { AUTO_REDUCE( (yyvsp[0]), nullptr );
                                         (yyval) = makenode( X_U_MINUS, 1, (yyvsp[0]) ); }
#line 4954 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 436:
#line 1292 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, __reginatext ); }
#line 4960 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 437:
#line 1293 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_EXPRESSION, 1, __reginatext ); }
#line 4966 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 438:
#line 1296 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]); }
#line 4972 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 439:
#line 1297 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_UNMATCHED_PARAN, 0 ); }
#line 4978 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 440:
#line 1298 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_UNMATCHED_PARAN, 0 ); }
#line 4984 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 441:
#line 1301 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr)create_head( (const char *)retvalue ) ; }
#line 4990 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 442:
#line 1305 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_EX_FUNC,1,(yyvsp[0])) ;
                                         (yyval)->name = (streng *)(yyvsp[-1]) ; }
#line 4997 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 443:
#line 1307 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_IN_FUNC,1,(yyvsp[0])) ;
                                         (yyval)->name = (streng *)(yyvsp[-1]) ; }
#line 5004 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 444:
#line 1311 "../yaccsrc.y" /* yacc.c:1646  */
    { /* ugly fake preservs parendepth */
                                         (yyval) = (YYSTYPE) parendepth;
                                         parendepth = 0; }
#line 5012 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 445:
#line 1314 "../yaccsrc.y" /* yacc.c:1646  */
    { parendepth = (int) (yyval);
                                         (yyval) = (yyvsp[0]); }
#line 5019 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 446:
#line 1318 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
#line 5025 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 447:
#line 1321 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
#line 5031 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 448:
#line 1324 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) =makenode(X_TPL_SOLID,3,(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 5037 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 449:
#line 1325 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) =makenode(X_TPL_SOLID,1,(yyvsp[0])) ; }
#line 5043 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 450:
#line 1326 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_INVALID_TEMPLATE, 1, __reginatext ) ;}
#line 5049 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 451:
#line 1329 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_NEG_OFFS,0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; }
#line 5056 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 452:
#line 1331 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_POS_OFFS,0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; }
#line 5063 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 453:
#line 1333 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_ABS_OFFS,0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; }
#line 5070 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 454:
#line 1335 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_ABS_OFFS,0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; }
#line 5077 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 455:
#line 1337 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_TPL_VAR,0) ;
                                         (yyval)->p[0] = (yyvsp[-1]) ; }
#line 5084 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 456:
#line 1339 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_NEG_OFFS,0) ;
                                         (yyval)->p[0] = (yyvsp[-1]) ; }
#line 5091 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 457:
#line 1341 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_POS_OFFS,0) ;
                                         (yyval)->p[0] = (yyvsp[-1]) ; }
#line 5098 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 458:
#line 1343 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_ABS_OFFS,0) ;
                                         (yyval)->p[0] = (yyvsp[-1]) ; }
#line 5105 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 459:
#line 1345 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_TPL_MVE,0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; }
#line 5112 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 460:
#line 1349 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
#line 5118 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 461:
#line 1350 "../yaccsrc.y" /* yacc.c:1646  */
    { streng *sptr = Str_cre_TSD(parser_data.TSD,retvalue) ;
                                          if (myisnumber(parser_data.TSD, sptr))
                                          {
                                             exiterror( ERR_INVALID_INTEGER, 4, sptr->value ) ;
                                          }
                                          else
                                             exiterror( ERR_INVALID_TEMPLATE, 0 ) ;}
#line 5130 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 462:
#line 1359 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (nodeptr) Str_cre_TSD(parser_data.TSD,retvalue) ; }
#line 5136 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 463:
#line 1360 "../yaccsrc.y" /* yacc.c:1646  */
    { streng *sptr = Str_make_TSD(parser_data.TSD,retlength) ;
                                         memcpy(sptr->value,retvalue,
                                                 sptr->len=retlength) ;
                                         (yyval) = (nodeptr) sptr ; }
#line 5145 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 464:
#line 1364 "../yaccsrc.y" /* yacc.c:1646  */
    { streng *sptr = Str_make_TSD(parser_data.TSD,retlength) ;
                                         memcpy(sptr->value,retvalue,
                                                 sptr->len=retlength) ;
                                         (yyval) = (nodeptr) sptr ; }
#line 5154 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 465:
#line 1370 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_TPL_POINT,1,(yyvsp[0])) ; }
#line 5160 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 466:
#line 1371 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_TPL_SYMBOL,2,(yyvsp[0]),(yyvsp[-1])) ; }
#line 5166 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 467:
#line 1372 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = NULL ; }
#line 5172 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 468:
#line 1382 "../yaccsrc.y" /* yacc.c:1646  */
    { /* detect
                                          * "x = ( a,. b )",
                                          * "x = ( ,. b )",
                                          * "x = a,. b",
                                          * "x = ,. b",
                                          * "x = f( ( x,. b ) )",
                                          * "x = f( ( ,. b ) )"      */
                                         if ( parendepth < 0 )
                                            exiterror( ERR_UNEXPECTED_PARAN, 1 );

                                         /* With call being the extended kind
                                          * of CALL we may have:
                                          * "x = f( (a),. b )",
                                          * "x = f( a,. b )",
                                          * "x = f( ,. b )",
                                          * "CALL s (a),. b",
                                          * "CALL s a,. b",
                                          * "CALL s ,. b",
                                          * "call s (a),. b",
                                          * "call s a,. b",
                                          * "call s ,. b",
                                          * "call s( (a),. b )",
                                          * "call s( a,. b )",
                                          * "call s( ,. b )",
                                          * "call s (a,a),. b",
                                          * "call s (a),. b",
                                          * "call s (),. b"
                                          *
                                          * detect "(a),." and transform it
                                          * to "a,."                         */
                                         (yyvsp[-1]) = reduce_expr_list( (yyvsp[-1]),
                                                                REDUCE_EXPR );

                                         /* detect "call s (a,b),. b" and
                                          * "call s (),. b", but every list on
                                          * the left side of "," is wrong, so
                                          * complain about every exprlist.   */
                                         if ( IS_EXPRLIST( (yyvsp[-1]) ) )
                                            exiterror( ERR_UNEXPECTED_PARAN, 1 );

                                         (yyvsp[-1]) = reduce_expr_list( (yyvsp[-1]),
                                                              REDUCE_SUBEXPR );
                                       }
#line 5220 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 469:
#line 1425 "../yaccsrc.y" /* yacc.c:1646  */
    { /*
                                          * Fixes bug 961301.
                                          */
                                         nodeptr curr;

                                         assert( IS_EXPRLIST( (yyvsp[0]) ) );

                                         /* detect ",()." */
                                         if ( IS_EXPRLIST( (yyvsp[0])->p[0] )
                                           && ( (yyvsp[0])->p[1] == NULL )
                                           && ( (yyvsp[0])->p[0]->p[0] == NULL ) )
                                            exiterror( ERR_UNEXPECTED_PARAN, 0 );

                                         /* detect ",(a,b)." */
                                         if ( IS_EXPRLIST( (yyvsp[0])->p[0] )
                                           && ( (yyvsp[0])->p[1] == NULL )
                                           && IS_EXPRLIST( (yyvsp[0])->p[0]->p[1] ) )
                                            exiterror( ERR_UNEXPECTED_PARAN, 1 );

                                         /* detect ",(a)." and transform it
                                          * to ",a."                         */
                                         (yyvsp[0]) = reduce_expr_list( (yyvsp[0]),
                                                                REDUCE_RIGHT );
                                         assert( IS_EXPRLIST( (yyvsp[0]) ) );

                                         /* Detect something like
                                          * "call s (a,b)+1"                 */
                                         curr = (yyvsp[0])->p[0];
                                         if ( ( curr != NULL )
                                           && !IS_EXPRLIST( curr )
                                           && !IS_FUNCTION( curr )
                                           && ( IS_EXPRLIST( curr->p[0] )
                                             || IS_EXPRLIST( curr->p[1] ) ) )
                                            exiterror( ERR_INVALID_EXPRESSION, 0 );

                                         (yyval) = makenode( X_EXPRLIST, 2, (yyvsp[-3]), (yyvsp[0]) );
                                         checkconst( (yyval) ); }
#line 5262 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 470:
#line 1462 "../yaccsrc.y" /* yacc.c:1646  */
    { /* detect
                                          * "x = ()."
                                          * "x = f(().)"
                                          * "call s (().)"
                                          * "CALL s ()."                     */
                                         if ( ( parendepth < 0 ) && ( (yyvsp[0]) == NULL ) )
                                            exiterror( ERR_UNEXPECTED_PARAN, 0 );

                                         /* With call being the extended kind
                                          * of CALL we may have:
                                          * "x = ( a. )",
                                          * "x = f( . )",
                                          * "x = f( ., )",
                                          * "x = f( a. )",
                                          * "x = f( a., )",
                                          * "x = f( a, . )",
                                          * "x = f( a, b. )",
                                          * "CALL s .",
                                          * "CALL s .,",
                                          * "CALL s a.,",
                                          * "CALL s a, .",
                                          * "CALL s a, b.",
                                          * "call s .",
                                          * "call s .,",
                                          * "call s a.,",
                                          * "call s a, .",
                                          * "call s a, b.",
                                          * "call s (a.)",
                                          * "call s (a)+1, .",
                                          * "call s (a), .",
                                          * "call s (a), a.",
                                          * "call s (a), (a).",
                                          * "call s ( ., )",
                                          * "call s ( a., )",
                                          * "call s ( a, . )",
                                          * "call s ( a, b. )"               */

                                         (yyvsp[0]) = reduce_expr_list( (yyvsp[0]),
                                                              REDUCE_SUBEXPR );
                                         (yyval) = makenode( X_EXPRLIST, 1, (yyvsp[0]) );
                                         checkconst( (yyval) );
                                        }
#line 5309 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 471:
#line 1506 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]) ; }
#line 5315 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 472:
#line 1507 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = NULL ; }
#line 5321 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 473:
#line 1510 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_SIM_SYMBOL,1,(yyvsp[0])) ;
                                         (yyval)->name = (streng *) (yyvsp[-1]) ; }
#line 5328 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 474:
#line 1512 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_SIM_SYMBOL,0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; }
#line 5335 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 475:
#line 1514 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_IND_SYMBOL,1,(yyvsp[0])) ;
                                         (yyval)->name = (streng *) (yyvsp[-2]) ; }
#line 5342 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 476:
#line 1516 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = makenode(X_IND_SYMBOL,0) ;
                                         (yyval)->name = (streng *) (yyvsp[-1]) ; }
#line 5349 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 477:
#line 1520 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (treenode *) Str_cre_TSD(parser_data.TSD,retvalue);}
#line 5355 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 478:
#line 1523 "../yaccsrc.y" /* yacc.c:1646  */
    { (yyval) = (treenode *) Str_cre_TSD(parser_data.TSD,retvalue);}
#line 5361 "../yaccsrc.c" /* yacc.c:1646  */
    break;

  case 479:
#line 1524 "../yaccsrc.y" /* yacc.c:1646  */
    { exiterror( ERR_SYMBOL_EXPECTED, 1, __reginatext ) ;}
#line 5367 "../yaccsrc.c" /* yacc.c:1646  */
    break;


#line 5371 "../yaccsrc.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 1527 "../yaccsrc.y" /* yacc.c:1906  */


static nodeptr makenode( int type, int numb, ... )
{
   nodeptr thisleave ;
   va_list argptr ;
   int i ;

   assert(numb <= (int) (sizeof(thisleave->p)/sizeof(thisleave->p[0])));
#ifdef REXXDEBUG
   printf("makenode: making new node, type: %d\n",type) ;
#endif /* REXXDEBUG */

   thisleave = FreshNode( ) ;
   /* thisleave is initialized to zero except for nodeindex */
   va_start( argptr, numb ) ;
   thisleave->type = type ;
   thisleave->lineno = -1 ;
   thisleave->charnr = -1 ;
   for (i=0;i<numb;i++)
      thisleave->p[i]=va_arg(argptr, nodeptr) ;

   va_end( argptr ) ;

   return( thisleave ) ;
}

static char *getdokeyword( int type )
{
   char *ptr;
   switch( type )
   {
      case X_DO_TO: ptr="TO";break;
      case X_DO_BY: ptr="BY";break;
      case X_DO_FOR: ptr="FOR";break;
      default: ptr="";break;
   }
   return ptr;
}

static void checkdosyntax( cnodeptr thisptr )
{
   if ((thisptr->p[1]!=NULL)&&(thisptr->p[2]!=NULL))
   {
      if ((thisptr->p[1]->type)==(thisptr->p[2]->type))
      {
         exiterror( ERR_INVALID_DO_SYNTAX, 1, getdokeyword(thisptr->p[1]->type) )  ;
      }
   }
   if ((thisptr->p[2]!=NULL)&&(thisptr->p[3]!=NULL))
   {
      if ((thisptr->p[2]->type)==(thisptr->p[3]->type))
      {
         exiterror( ERR_INVALID_DO_SYNTAX, 1, getdokeyword(thisptr->p[2]->type) )  ;
      }
   }
   if ((thisptr->p[1]!=NULL)&&(thisptr->p[3]!=NULL))
   {
      if ((thisptr->p[1]->type)==(thisptr->p[3]->type))
      {
         exiterror( ERR_INVALID_DO_SYNTAX, 1, getdokeyword(thisptr->p[1]->type) )  ;
      }
   }
   return ;
}


void newlabel( const tsd_t *TSD, internal_parser_type *ipt, nodeptr thisptr )
{
   labelboxptr newptr ;

   assert( thisptr ) ;

   newptr = (labelboxptr)MallocTSD(sizeof(labelbox)) ;

   newptr->next = NULL ;
   newptr->entry = thisptr ;
   if (ipt->first_label == NULL)
   {
      ipt->first_label = newptr ;
      ipt->last_label = newptr ; /* must be NULL, too */
   }
   else
   {
      ipt->last_label->next = newptr ;
      ipt->last_label = newptr ;
   }
   ipt->numlabels++;
}

static nodeptr create_tail( const char *name )
{
   const char *cptr ;
   nodeptr node ;
   int constant ;
   streng *tname ;
   tsd_t *TSD = parser_data.TSD;

   if (!*name)
   {
      node = makenode( X_CTAIL_SYMBOL, 0 ) ;
      node->name = Str_make_TSD( parser_data.TSD, 0) ;
      return node ;
   }

   cptr = name ;
   constant = rx_isdigit(*cptr) || *cptr=='.' || (!*cptr) ;
   node = makenode( (constant) ? X_CTAIL_SYMBOL : X_VTAIL_SYMBOL, 0 ) ;

   for (;*cptr && *cptr!='.'; cptr++) ;
   node->name = Str_ncre_TSD( parser_data.TSD, name, cptr-name ) ;

   if (*cptr)
   {
      node->p[0] = create_tail( ++cptr ) ;
      if (constant && node->p[0]->type==X_CTAIL_SYMBOL)
      {
         streng *first, *second ;
         nodeptr tptr ;

         first = node->name ;
         second = node->p[0]->name ;
         node->name = NULL;
         node->p[0]->name = NULL;
         tname = Str_makeTSD( first->len + second->len + 1) ;
         memcpy( tname->value, first->value, first->len ) ;
         tname->value[first->len] = '.' ;
         memcpy( tname->value+first->len+1, second->value, second->len) ;
         tname->len = first->len + second->len + 1 ;

         Free_stringTSD( first ) ;
         Free_stringTSD( second ) ;
         node->name = tname ;
         tptr = node->p[0] ;
         node->p[0] = tptr->p[0] ;
         RejectNode(tptr);
      }
   }

   return node ;
}

static nodeptr create_head( const char *name )
{
   const char *cptr ;
   nodeptr node ;

   /* Bypass reserved variables */
   cptr = ( *name ) ? ( name + 1 ) : name;
   for (; *cptr && *cptr!='.'; cptr++) ;
   node = makenode( X_SIM_SYMBOL, 0 ) ;
   node->name = Str_ncre_TSD( parser_data.TSD, name, cptr-name+(*cptr=='.')) ;

   if (*cptr)
   {
      if (*(++cptr))
         node->p[0] = create_tail( cptr ) ;
      else
         node->p[0] = NULL ;

      node->type = (node->p[0]) ? X_HEAD_SYMBOL : X_STEM_SYMBOL ;
   }

   return node ;
}


static node_type gettypeof( nodeptr thisptr )
{
   tsd_t *TSD = parser_data.TSD;

   switch(thisptr->type)
   {
      case X_PLUSS:
      case X_MINUS:
      case X_MULT:
      case X_U_PLUSS:
      case X_U_MINUS:
      case X_DEVIDE:
      case X_INTDIV:
      case X_MODULUS:
      case X_EQUAL:
      case X_DIFF:
      case X_GTE:
      case X_GT:
      case X_LTE:
      case X_LT:
      case X_SEQUAL:
      case X_SDIFF:
      case X_SGTE:
      case X_SGT:
      case X_SLTE:
      case X_SLT:
      case X_NEQUAL:
      case X_NDIFF:
      case X_NGTE:
      case X_NGT:
      case X_NLTE:
      case X_NLT:
         return IS_A_NUMBER ;


      case X_SIM_SYMBOL:
         return IS_SIM_SYMBOL ;

      case X_HEAD_SYMBOL:
         return IS_COMP_SYMBOL ;

      case X_STRING:
      case X_CON_SYMBOL:
      {
         if (thisptr->u.number)
         {
            fprintf( stderr, "Found an internal spot of investigation of the Regina interpreter.\n"
                             "Please inform Mark Hessling or Florian Grosse-Coosmann about the\n"
                             "circumstances and this script.\n"
                             "\n"
                             "Many thanks!\n"
                             "email addresses:\n"
                             "mark@rexx.org\n"
                             "florian@grosse-coosmann.de\n");
            /* FIXME: When does this happen?
             * It doesn't happen if no feedback is send until end of 2005.
             */
            return IS_A_NUMBER ;
         }

         if ( ( thisptr->u.number = is_a_descr( TSD, thisptr->name ) ) != NULL )
            return IS_A_NUMBER;
         return IS_NO_NUMBER;
      }
   }
   return IS_UNKNOWN ;
}



static void transform( nodeptr thisptr )
{
   int type ;
   node_type left,rght;

   left = gettypeof( thisptr->p[0] ) ;
   rght = gettypeof( thisptr->p[1] ) ;
   type = thisptr->type ;

   if ( ( left == IS_A_NUMBER ) && ( rght == IS_A_NUMBER ) )
   {
      if (type==X_EQUAL)
         thisptr->type = X_NEQUAL ;
      else if (type==X_DIFF)
         thisptr->type = X_NDIFF ;
      else if (type==X_GTE)
         thisptr->type = X_NGTE ;
      else if (type==X_GT)
         thisptr->type = X_NGT ;
      else if (type==X_LTE)
         thisptr->type = X_NLTE ;
      else if (type==X_LT)
         thisptr->type = X_NLT ;
   }
   else if ( ( left == IS_NO_NUMBER ) || ( rght == IS_NO_NUMBER ) )
   {
      if (type==X_EQUAL)
         thisptr->type = X_SEQUAL ;
      else if (type==X_DIFF)
         thisptr->type = X_SDIFF ;
      else if (type==X_GTE)
         thisptr->type = X_SGTE ;
      else if (type==X_GT)
         thisptr->type = X_SGT ;
      else if (type==X_LTE)
         thisptr->type = X_SLTE ;
      else if (type==X_LT)
         thisptr->type = X_SLT ;
   }
   else
   {
      type = thisptr->p[0]->type ;
      if ( ( left == IS_A_NUMBER )
        && ( ( type == X_STRING ) || ( type == X_CON_SYMBOL ) ) )
         thisptr->u.flags.lnum = 1 ;
      else if ( left == IS_SIM_SYMBOL )
         thisptr->u.flags.lsvar = 1 ;
      else if ( left == IS_COMP_SYMBOL )
         thisptr->u.flags.lcvar = 1 ;

      type = thisptr->p[1]->type ;
      if ( ( rght == IS_A_NUMBER )
        && ( ( type == X_STRING ) || ( type == X_CON_SYMBOL ) ) )
         thisptr->u.flags.rnum = 1 ;
      else if ( rght == IS_SIM_SYMBOL )
         thisptr->u.flags.rsvar = 1 ;
      else if ( rght == IS_COMP_SYMBOL )
         thisptr->u.flags.rcvar = 1 ;
   }
}


static int is_const( cnodeptr thisptr )
{
   if (!thisptr)
      return 1 ;

   switch (thisptr->type)
   {
      case X_STRING:
      case X_CON_SYMBOL:
         return 1 ;

#if 0
      Pre-evaluation is not allowed. DIGITS and FUZZ may change within loops
      and the resulting value may or may not be the same. Concatenation with
      or without spaces is the sole allowed operation.

      NEVER ENABLE THIS AGAIN WITHOUT SOLVING THIS PROBLEMS!

      case X_U_PLUSS:
      case X_U_MINUS:
         return is_const( thisptr->p[0] ) ;

      case X_PLUSS:
      case X_MINUS:
      case X_MULT:
/*    case X_DEVIDE: Bug 20000807-41821 */
      case X_INTDIV:
      case X_MODULUS:
      case X_EQUAL:
      case X_DIFF:
      case X_GTE:
      case X_GT:
      case X_LTE:
      case X_LT:
      case X_SEQUAL:
      case X_SDIFF:
      case X_SGTE:
      case X_SGT:
      case X_SLTE:
      case X_SLT:
      case X_NEQUAL:
      case X_NDIFF:
      case X_NGTE:
      case X_NGT:
      case X_NLTE:
      case X_NLT:
#endif

      case X_SPACE:
      case X_CONCAT:
         return is_const( thisptr->p[0] ) && is_const( thisptr->p[1] ) ;
   }
   return 0 ;
}


static void checkconst( nodeptr thisptr )
{
   tsd_t *TSD = parser_data.TSD;

   assert( thisptr->type == X_EXPRLIST ) ;
   if (is_const(thisptr->p[0]))
   {
      if (thisptr->p[0])
         thisptr->u.strng = evaluate( TSD, thisptr->p[0], NULL ) ;
      else
         thisptr->u.strng = NULL ;

      thisptr->type = X_CEXPRLIST ;
   }
}

/*
 * reduce_expr_list will be invoked if the reduction of a list expression for
 * "call" arguments or a plain "(expr)" is needed. The reduction of the
 * outer parentheses of the extended CALL syntax is done with
 * mode==REDUCE_CALL, the reduction of a simple "(expr)" is done with
 * mode==REDUCE_EXPR. REDUCE_RIGHT is a synonym for REDUCE_CALL currently and
 * is intended to be used for reducing the right side of an expression list.
 *
 * REDUCE_SUBEXPR detect "call s (a)+1," and "call s 1+(a)," and reduces it.
 * Furthermore it detects "call s ()+1", "call s 1+()", "call s 1+(a,b)",
 * "call s (a,b)+1" and raises an error in this case.
 */
static nodeptr reduce_expr_list( nodeptr thisptr, reduce_mode mode )
{
   nodeptr h, retval = thisptr;

   if ( !thisptr )
      return retval;

   if ( mode == REDUCE_SUBEXPR )
   {
      if ( ( parendepth == 1 ) && !IS_FUNCTION( thisptr ) && !IS_EXPRLIST( thisptr ) )
      {
         if ( IS_EXPRLIST( thisptr->p[0] ) )
         {
            h = thisptr->p[0];
            if ( ( h->p[0] == NULL ) || ( h->p[1] != NULL ) )
               exiterror( ERR_INVALID_EXPRESSION, 0 );
            thisptr->p[0] = h->p[0];
            RejectNode( h );
         }
         if ( IS_EXPRLIST( thisptr->p[1] ) )
         {
            h = thisptr->p[1];
            if ( ( h->p[0] == NULL ) || ( h->p[1] != NULL ) )
               exiterror( ERR_INVALID_EXPRESSION, 0 );
            thisptr->p[1] = h->p[0];
            RejectNode( h );
         }
      }
      return retval;
   }

   if ( !IS_EXPRLIST( thisptr ) )
      return retval;

   if ( ( mode == REDUCE_CALL ) || ( mode == REDUCE_RIGHT ) )
   {
      if ( IS_EXPRLIST( thisptr->p[0] ) && ( thisptr->p[1] == NULL ) )
      {
         retval = thisptr->p[0];
         RejectNode( thisptr );
      }
   }
   else
   {
      /*
       * mode == REDUCE_EXPR
       */
      if ( ( thisptr->p[0] != NULL ) && ( thisptr->p[1] == NULL ) )
      {
         if ( !IS_EXPRLIST( thisptr->p[0] ) )
         {
            retval = thisptr->p[0];
            RejectNode( thisptr );
         }
      }
   }
   return retval;
}

/*
 * optgluelast connect p2 as the ->next element to p1. Every element may be
 * NULL.
 * If both are non-NULL, the connection is performed using the o.last elements.
 * Just the o.last element of p1 remains non-NULL.
 *
 * Returns: NULL if p1 and p2 are NULL.
 *          The non-NULL element if one argumet is NULL.
 *          p1 otherwise.
 */
static nodeptr optgluelast( nodeptr p1, nodeptr p2 )
{
   nodeptr p2last;

   if ( p1 == NULL )
      return p2;
   if ( p2 == NULL )
      return p1;

   /*
    * This is performed very often, so keep the code fast.
    *
    * p2last is the "o.last"-element of p2 or just p2 if p2 has no next
    * elements. We set p1's o.last further down, but we have to ensure that
    * p2->o.last is NULL first. Therefore every element in the ->next chain
    * of p1 will have NULL as its o.last field.
    */
   if ( ( p2last = p2->o.last ) == NULL )
      p2last = p2;
   else
      p2->o.last = NULL;

   if ( p1->o.last == NULL )
      p1->next = p2;
   else
      p1->o.last->next = p2;
   p1->o.last = p2last;

   return p1;
}

/*
 * justlabels returns 1, if n consists of a sequence of labels. The return
 * value is 0 otherwise.
 */
static int justlabels( nodeptr n )
{
   while ( n != NULL )
   {
      if ( n->type != X_LABEL )
         return 0;
      n = n->next;
   }

   return 1;
}

/*
 * move_labels move the labels from the end of "end" to the end of "front".
 * The labels are marked "read_only" if level is nonnull, the read-only flag
 * is removed if level is 0.
 * NOTE: At least one element of the "end" chain must contain a non-label
 * element.
 */
static void move_labels( nodeptr front, nodeptr end, int level )
{
   nodeptr oend = end;
   nodeptr labels;

   assert( front != NULL );
   assert( !justlabels( end ) );

   while ( !justlabels( end->next ) )
      end = end->next;

   if ( ( labels = end->next ) == NULL )
      return;

   /*
    * extract the labels.
    */
   labels->o.last = oend->o.last;
   end->next = NULL;
   if ( end == oend )
      oend->o.last = NULL;
   else
      oend->o.last = end;

   if ( labels->next == NULL )
      labels->o.last = NULL;

   /*
    * add the labels to the end of front and then re-mark the labels.
    */
   optgluelast( front, labels );
   while ( labels ) {
      labels->u.trace_only = ( level == 0 ) ? 0 : 1;
      labels = labels->next;
   }
}
