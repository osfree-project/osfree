/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C

      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         __reginaparse
#define yylex           __reginalex
#define yyerror         __reginaerror
#define yylval          __reginalval
#define yychar          __reginachar
#define yydebug         __reginadebug
#define yynerrs         __reginanerrs


/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 1 "./yaccsrc.y"


#ifndef lint
static char *RCSid = "$Id: yaccsrc.c,v 1.46 2015/04/02 06:20:01 mark Exp $";
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

/* Line 268 of yacc.c  */
#line 150 "./yaccsrc.y"

#ifdef NDEBUG
# define YYDEBUG 0
#else
# define YYDEBUG 1
#endif


/* Line 268 of yacc.c  */
#line 207 "./yaccsrc.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
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
     LOWER = 359,
     CASELESS = 360,
     PLUSASSIGNMENTVARIABLE = 361,
     MINUSASSIGNMENTVARIABLE = 362,
     MULTASSIGNMENTVARIABLE = 363,
     DIVASSIGNMENTVARIABLE = 364,
     MODULUSASSIGNMENTVARIABLE = 365,
     INTDIVASSIGNMENTVARIABLE = 366,
     ORASSIGNMENTVARIABLE = 367,
     XORASSIGNMENTVARIABLE = 368,
     ANDASSIGNMENTVARIABLE = 369,
     CONCATASSIGNMENTVARIABLE = 370,
     CCAT = 371,
     UPLUS = 372,
     UMINUS = 373
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 367 "./yaccsrc.c"

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
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
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
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
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
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)            \
    do                           \
      {                          \
   YYSIZE_T yynewbytes;                \
   YYCOPY (&yyptr->Stack_alloc, Stack, yysize);       \
   Stack = &yyptr->Stack_alloc;              \
   yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
   yyptr += yynewbytes / sizeof (*yyptr);          \
      }                          \
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)     \
      do             \
   {              \
     YYSIZE_T yyi;            \
     for (yyi = 0; yyi < (Count); yyi++)  \
       (To)[yyi] = (From)[yyi];     \
   }              \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2256

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  130
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  196
/* YYNRULES -- Number of rules.  */
#define YYNRULES  477
/* YYNRULES -- Number of states.  */
#define YYNSTATES  651

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   373

#define YYTRANSLATE(YYX)                  \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   124,   117,     2,
     127,   128,   122,   120,   129,   121,     2,   123,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   118,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,   116,     2,     2,     2,     2,     2,
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
     115,   119,   125,   126
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    12,    15,    17,    20,
      23,    25,    27,    31,    33,    35,    39,    42,    45,    47,
      50,    51,    53,    55,    56,    59,    62,    64,    66,    68,
      70,    72,    74,    76,    78,    80,    82,    84,    86,    88,
      90,    92,    94,    96,    98,   100,   102,   104,   106,   108,
     110,   112,   114,   116,   118,   120,   122,   124,   126,   128,
     130,   132,   134,   136,   138,   140,   142,   144,   146,   148,
     150,   152,   154,   156,   158,   160,   162,   164,   166,   168,
     170,   172,   174,   176,   178,   180,   182,   184,   186,   188,
     190,   192,   193,   197,   201,   203,   204,   205,   209,   213,
     219,   222,   223,   228,   229,   234,   238,   242,   247,   253,
     258,   262,   267,   271,   273,   275,   277,   280,   282,   284,
     287,   289,   293,   300,   308,   315,   321,   326,   329,   332,
     335,   336,   338,   340,   342,   343,   346,   347,   350,   354,
     357,   359,   361,   363,   365,   368,   371,   374,   377,   381,
     384,   388,   390,   393,   397,   400,   404,   406,   409,   413,
     416,   420,   422,   427,   431,   436,   440,   445,   449,   451,
     453,   455,   458,   461,   464,   467,   469,   472,   475,   478,
     481,   484,   487,   490,   493,   494,   498,   499,   503,   505,
     506,   508,   510,   512,   514,   516,   520,   524,   528,   532,
     536,   537,   541,   544,   548,   551,   554,   561,   571,   581,
     588,   591,   596,   599,   602,   605,   608,   612,   614,   616,
     618,   621,   625,   627,   629,   632,   636,   639,   644,   648,
     651,   656,   660,   663,   667,   670,   672,   674,   677,   682,
     686,   690,   693,   695,   698,   701,   703,   706,   709,   711,
     715,   717,   719,   721,   723,   725,   727,   729,   732,   736,
     739,   741,   744,   748,   753,   757,   760,   763,   766,   769,
     772,   775,   779,   781,   787,   792,   797,   800,   804,   810,
     813,   815,   817,   824,   832,   835,   838,   841,   844,   848,
     849,   853,   857,   860,   864,   868,   873,   879,   884,   888,
     893,   897,   899,   901,   903,   906,   909,   912,   914,   916,
     918,   920,   922,   924,   926,   928,   930,   932,   934,   936,
     940,   943,   947,   950,   952,   955,   957,   960,   962,   965,
     967,   970,   972,   975,   977,   980,   982,   985,   987,   990,
     992,   995,   997,  1000,  1002,  1005,  1007,  1008,  1012,  1013,
    1018,  1019,  1024,  1025,  1030,  1032,  1033,  1038,  1040,  1041,
    1046,  1048,  1049,  1054,  1056,  1057,  1062,  1064,  1065,  1070,
    1072,  1073,  1078,  1080,  1081,  1086,  1088,  1089,  1094,  1096,
    1097,  1102,  1104,  1107,  1109,  1110,  1115,  1117,  1118,  1123,
    1125,  1126,  1131,  1133,  1134,  1139,  1141,  1142,  1147,  1149,
    1150,  1155,  1157,  1158,  1163,  1165,  1166,  1171,  1173,  1174,
    1179,  1181,  1182,  1187,  1189,  1190,  1195,  1197,  1198,  1203,
    1205,  1206,  1211,  1213,  1214,  1219,  1221,  1223,  1225,  1227,
    1229,  1231,  1233,  1236,  1239,  1242,  1245,  1248,  1251,  1253,
    1255,  1258,  1261,  1262,  1265,  1267,  1269,  1273,  1275,  1277,
    1280,  1283,  1285,  1288,  1292,  1297,  1302,  1307,  1309,  1311,
    1313,  1315,  1317,  1319,  1322,  1325,  1326,  1327,  1332,  1334,
    1336,  1337,  1340,  1342,  1347,  1351,  1353,  1355
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     131,     0,    -1,    -1,   132,   133,    -1,   140,   134,    -1,
     140,    -1,   134,   136,    -1,   136,    -1,   135,   142,    -1,
     142,   143,    -1,   142,    -1,   137,    -1,   143,   186,   139,
      -1,   135,    -1,   143,    -1,   139,    48,   141,    -1,    48,
     141,    -1,   141,   139,    -1,   141,    -1,   141,   225,    -1,
      -1,   144,    -1,   246,    -1,    -1,   143,   145,    -1,   146,
     139,    -1,   220,    -1,   221,    -1,   222,    -1,   174,    -1,
     184,    -1,   178,    -1,   179,    -1,   187,    -1,   217,    -1,
     219,    -1,   223,    -1,   224,    -1,   227,    -1,   228,    -1,
     229,    -1,   231,    -1,   232,    -1,   236,    -1,   237,    -1,
     238,    -1,   239,    -1,   241,    -1,   240,    -1,   243,    -1,
     248,    -1,   256,    -1,   218,    -1,   260,    -1,   262,    -1,
     264,    -1,   266,    -1,   268,    -1,   270,    -1,   274,    -1,
     276,    -1,   272,    -1,   278,    -1,   258,    -1,     5,    -1,
       6,    -1,    12,    -1,    13,    -1,    16,    -1,    18,    -1,
      32,    -1,    33,    -1,     3,    -1,     4,    -1,    36,    -1,
      17,    -1,    75,    -1,    19,    -1,    20,    -1,    85,    -1,
      21,    -1,    27,    -1,    29,    -1,    30,    -1,    31,    -1,
      34,    -1,    38,    -1,    35,    -1,    37,    -1,    44,    -1,
      46,    -1,    -1,   155,   175,   176,    -1,    25,   280,   190,
      -1,   192,    -1,    -1,    -1,     1,   177,   190,    -1,   189,
     322,   190,    -1,   127,   280,   128,   210,   190,    -1,   156,
     234,    -1,    -1,   147,   182,   180,   183,    -1,    -1,   147,
     318,   181,   183,    -1,   147,   252,     1,    -1,   147,   253,
       1,    -1,   147,   252,   254,     1,    -1,   147,   252,   254,
     250,     1,    -1,   147,   252,   254,   250,    -1,   147,   252,
     254,    -1,   147,   253,   254,     1,    -1,   147,   253,   254,
      -1,   251,    -1,     1,    -1,   320,    -1,   320,   128,    -1,
     280,    -1,    45,    -1,   185,   325,    -1,   185,    -1,   185,
     325,     1,    -1,   148,   188,   216,   139,   138,   186,    -1,
     214,   118,   280,   210,   215,   215,   215,    -1,   214,   118,
     280,   210,   215,   215,    -1,   214,   118,   280,   210,   215,
      -1,   214,   118,   280,   210,    -1,    49,   210,    -1,    49,
       1,    -1,   280,   210,    -1,    -1,    71,    -1,    72,    -1,
      78,    -1,    -1,   191,   192,    -1,    -1,    26,   193,    -1,
      26,   193,     1,    -1,    26,   210,    -1,   194,    -1,   195,
      -1,   196,    -1,     1,    -1,   200,   197,    -1,   201,   198,
      -1,   202,   199,    -1,   201,   210,    -1,   201,   202,   210,
      -1,   202,   210,    -1,   202,   201,   210,    -1,   210,    -1,
     200,   210,    -1,   200,   202,   210,    -1,   202,   210,    -1,
     202,   200,   210,    -1,   210,    -1,   200,   210,    -1,   200,
     201,   210,    -1,   201,   210,    -1,   201,   200,   210,    -1,
     210,    -1,   210,    94,   210,   203,    -1,   210,    94,     1,
      -1,   210,    95,   210,   204,    -1,   210,    95,     1,    -1,
     210,    96,   210,   204,    -1,   210,    96,     1,    -1,   205,
      -1,    97,    -1,   205,    -1,    98,   205,    -1,    98,     1,
      -1,    99,   205,    -1,    99,     1,    -1,    97,    -1,   100,
     208,    -1,   100,     1,    -1,   101,   206,    -1,   101,     1,
      -1,   102,   208,    -1,   102,     1,    -1,   103,   208,    -1,
     103,     1,    -1,    -1,   207,   210,   212,    -1,    -1,   209,
     210,   211,    -1,    66,    -1,    -1,   212,    -1,   213,    -1,
     324,    -1,   318,    -1,    76,    -1,     7,   280,   210,    -1,
       9,   280,   210,    -1,     8,   280,   210,    -1,    10,   280,
     210,    -1,    11,   280,   210,    -1,    -1,   157,   323,     1,
      -1,   157,   323,    -1,   173,   323,     1,    -1,   173,   323,
      -1,   149,   322,    -1,   150,   280,   140,    14,   140,   136,
      -1,   150,   280,   140,    14,   140,   136,    15,   140,   136,
      -1,   150,   280,   140,    14,   140,   136,    15,   140,     1,
      -1,   150,   280,   140,    14,   140,     1,    -1,   150,   139,
      -1,   150,   280,   140,     1,    -1,   143,    14,    -1,   143,
      15,    -1,   158,   280,    -1,   151,   325,    -1,   151,   325,
       1,    -1,   151,    -1,   226,    -1,   159,    -1,   152,   325,
      -1,   152,   325,     1,    -1,   152,    -1,   160,    -1,   160,
       1,    -1,   161,    50,   280,    -1,   161,    50,    -1,   161,
      51,   230,     1,    -1,   161,    51,   230,    -1,   161,    51,
      -1,   161,    51,    25,   280,    -1,   161,    51,     1,    -1,
     161,    52,    -1,   161,    52,   280,    -1,   161,     1,    -1,
      53,    -1,    54,    -1,   162,   322,    -1,   163,   233,   235,
     234,    -1,   163,   235,   234,    -1,   163,   233,     1,    -1,
     163,     1,    -1,    46,    -1,    46,   105,    -1,   105,    46,
      -1,   104,    -1,   104,   105,    -1,   105,   104,    -1,   105,
      -1,   234,   129,   315,    -1,   315,    -1,     4,    -1,    80,
      -1,    22,    -1,    79,    -1,    29,    -1,    23,    -1,    24,
     325,    -1,    25,   322,    26,    -1,    25,     1,    -1,   164,
      -1,   164,     1,    -1,   164,    28,     1,    -1,   164,    28,
     323,     1,    -1,   164,    28,   323,    -1,   165,   315,    -1,
     166,   322,    -1,   167,   322,    -1,   153,   322,    -1,   154,
     322,    -1,    45,   325,    -1,    45,   325,     1,    -1,    45,
      -1,   168,   139,   244,   247,   242,    -1,   168,   139,    45,
       1,    -1,   168,   139,   171,     1,    -1,   168,     1,    -1,
     168,   139,    14,    -1,   168,   139,   244,   171,     1,    -1,
     244,   245,    -1,   245,    -1,     1,    -1,   170,   280,   140,
      14,   140,   142,    -1,   170,   280,   140,    14,   140,   142,
      14,    -1,   170,   280,    -1,   170,     1,    -1,   143,   170,
      -1,   143,   171,    -1,   171,   140,   138,    -1,    -1,   169,
      25,   280,    -1,   169,   249,     1,    -1,   169,   249,    -1,
     169,   252,     1,    -1,   169,   253,     1,    -1,   169,   252,
     255,     1,    -1,   169,   252,   255,   250,     1,    -1,   169,
     252,   255,   250,    -1,   169,   252,   255,    -1,   169,   253,
     255,     1,    -1,   169,   253,   255,    -1,   251,    -1,    78,
      -1,     1,    -1,    82,    72,    -1,    82,    78,    -1,    82,
       1,    -1,    71,    -1,    72,    -1,    39,    -1,    40,    -1,
      96,    -1,    42,    -1,    70,    -1,    83,    -1,   254,    -1,
      43,    -1,    41,    -1,    87,    -1,   172,    25,   280,    -1,
     172,   322,    -1,   172,   257,     1,    -1,   172,   257,    -1,
      81,    -1,   259,   322,    -1,    47,    -1,   261,   322,    -1,
     106,    -1,   263,   322,    -1,   107,    -1,   265,   322,    -1,
     108,    -1,   267,   322,    -1,   109,    -1,   269,   322,    -1,
     111,    -1,   271,   322,    -1,   110,    -1,   273,   322,    -1,
     114,    -1,   275,   322,    -1,   113,    -1,   277,   322,    -1,
     112,    -1,   279,   322,    -1,   115,    -1,    -1,   127,   281,
     308,    -1,    -1,   280,   120,   282,   280,    -1,    -1,   280,
     121,   283,   280,    -1,    -1,   280,   122,   284,   280,    -1,
     122,    -1,    -1,   280,   123,   285,   280,    -1,   123,    -1,
      -1,   280,    57,   286,   280,    -1,    57,    -1,    -1,   280,
     124,   287,   280,    -1,   124,    -1,    -1,   280,   116,   288,
     280,    -1,   116,    -1,    -1,   280,   117,   289,   280,    -1,
     117,    -1,    -1,   280,    68,   290,   280,    -1,    68,    -1,
      -1,   280,    67,   291,   280,    -1,    67,    -1,    -1,   280,
      66,   292,   280,    -1,    66,    -1,    -1,   280,    56,   293,
     280,    -1,    56,    -1,    55,   280,    -1,    55,    -1,    -1,
     280,   118,   294,   280,    -1,   118,    -1,    -1,   280,    58,
     295,   280,    -1,    58,    -1,    -1,   280,    60,   296,   280,
      -1,    60,    -1,    -1,   280,    59,   297,   280,    -1,    59,
      -1,    -1,   280,    61,   298,   280,    -1,    61,    -1,    -1,
     280,    62,   299,   280,    -1,    62,    -1,    -1,   280,    63,
     300,   280,    -1,    63,    -1,    -1,   280,    64,   301,   280,
      -1,    64,    -1,    -1,   280,    88,   302,   280,    -1,    88,
      -1,    -1,   280,    89,   303,   280,    -1,    89,    -1,    -1,
     280,    90,   304,   280,    -1,    90,    -1,    -1,   280,    91,
     305,   280,    -1,    91,    -1,    -1,   280,    92,   306,   280,
      -1,    92,    -1,    -1,   280,    93,   307,   280,    -1,    93,
      -1,   309,    -1,    71,    -1,    77,    -1,    84,    -1,    78,
      -1,   310,    -1,   120,   280,    -1,   121,   280,    -1,   120,
       1,    -1,   121,     1,    -1,   320,   128,    -1,   320,     1,
      -1,    48,    -1,    72,    -1,   314,   311,    -1,   313,   311,
      -1,    -1,   312,   308,    -1,    74,    -1,    73,    -1,   319,
     316,   315,    -1,   319,    -1,     1,    -1,   121,   317,    -1,
     120,   317,    -1,   317,    -1,   118,   317,    -1,   127,   309,
     128,    -1,   121,   127,   309,   128,    -1,   120,   127,   309,
     128,    -1,   118,   127,   309,   128,    -1,   318,    -1,    65,
      -1,    71,    -1,    78,    -1,    77,    -1,    84,    -1,    69,
     319,    -1,   309,   319,    -1,    -1,    -1,   322,   129,   321,
     320,    -1,   322,    -1,   280,    -1,    -1,   324,   323,    -1,
     324,    -1,   127,   324,   128,   323,    -1,   127,   324,   128,
      -1,    72,    -1,    72,    -1,     1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
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
     591,   592,   594,   595,   596,   597,   602,   610,   611,   624,
     625,   629,   630,   634,   637,   637,   641,   641,   645,   646,
     649,   650,   653,   661,   669,   672,   673,   674,   677,   678,
     679,   682,   683,   687,   688,   692,   696,   704,   714,   716,
     718,   719,   722,   725,   728,   733,   735,   736,   739,   747,
     751,   753,   754,   757,   758,   761,   764,   765,   766,   769,
     771,   773,   774,   775,   778,   781,   782,   785,   788,   794,
     800,   801,   804,   806,   808,   810,   812,   814,   816,   820,
     822,   825,   826,   827,   828,   829,   830,   831,   833,   834,
     837,   838,   839,   840,   841,   845,   849,   853,   857,   861,
     865,   866,   867,   870,   875,   876,   878,   879,   880,   884,
     885,   886,   889,   895,   897,   898,   908,   909,   912,   916,
     922,   925,   926,   928,   929,   930,   931,   932,   938,   942,
     943,   949,   950,   951,   954,   955,   956,   959,   960,   963,
     966,   969,   970,   971,   972,   975,   976,   977,   978,   981,
     983,   985,   986,   990,   994,  1010,  1016,  1021,  1027,  1032,
    1038,  1043,  1049,  1054,  1060,  1065,  1071,  1076,  1082,  1087,
    1093,  1098,  1104,  1109,  1115,  1120,  1127,  1127,  1160,  1160,
    1163,  1163,  1166,  1166,  1169,  1170,  1170,  1173,  1174,  1174,
    1177,  1178,  1178,  1181,  1182,  1182,  1185,  1186,  1186,  1189,
    1190,  1190,  1193,  1194,  1194,  1197,  1198,  1198,  1201,  1202,
    1202,  1205,  1206,  1208,  1209,  1209,  1213,  1214,  1214,  1218,
    1219,  1219,  1223,  1224,  1224,  1228,  1229,  1229,  1233,  1234,
    1234,  1238,  1239,  1239,  1242,  1243,  1243,  1246,  1247,  1247,
    1250,  1251,  1251,  1254,  1255,  1255,  1258,  1259,  1259,  1262,
    1263,  1263,  1266,  1267,  1267,  1270,  1271,  1272,  1274,  1278,
    1282,  1284,  1285,  1287,  1289,  1290,  1293,  1294,  1295,  1298,
    1302,  1304,  1308,  1308,  1315,  1318,  1321,  1322,  1323,  1326,
    1328,  1330,  1332,  1334,  1336,  1338,  1340,  1342,  1346,  1347,
    1356,  1357,  1361,  1367,  1368,  1369,  1379,  1379,  1459,  1503,
    1504,  1507,  1509,  1511,  1513,  1517,  1520,  1521
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
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
  "LOWER", "CASELESS", "PLUSASSIGNMENTVARIABLE", "MINUSASSIGNMENTVARIABLE",
  "MULTASSIGNMENTVARIABLE", "DIVASSIGNMENTVARIABLE",
  "MODULUSASSIGNMENTVARIABLE", "INTDIVASSIGNMENTVARIABLE",
  "ORASSIGNMENTVARIABLE", "XORASSIGNMENTVARIABLE", "ANDASSIGNMENTVARIABLE",
  "CONCATASSIGNMENTVARIABLE", "'|'", "'&'", "'='", "CCAT", "'+'", "'-'",
  "'*'", "'/'", "'%'", "UPLUS", "UMINUS", "'('", "')'", "','", "$accept",
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
  "xsimsymb", "simsymb", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
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
     365,   366,   367,   368,   369,   370,   124,    38,    61,   371,
      43,    45,    42,    47,    37,   372,   373,    40,    41,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   130,   132,   131,   133,   133,   134,   134,   135,   135,
     136,   136,   137,   138,   138,   139,   139,   140,   140,   141,
     141,   142,   142,   143,   144,   145,   145,   145,   145,   146,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   175,   174,   176,   176,   176,   177,   176,   176,   176,
     178,   180,   179,   181,   179,   179,   179,   179,   179,   179,
     179,   179,   179,   182,   182,   183,   183,   184,   185,   186,
     186,   186,   187,   188,   188,   188,   188,   188,   188,   188,
     188,   189,   189,   189,   191,   190,   190,   192,   192,   192,
     193,   193,   193,   193,   194,   195,   196,   197,   197,   197,
     197,   197,   198,   198,   198,   198,   198,   199,   199,   199,
     199,   199,   200,   200,   201,   201,   202,   202,   203,   203,
     204,   204,   204,   204,   204,   204,   205,   205,   205,   205,
     205,   205,   205,   205,   207,   206,   209,   208,   210,   210,
     211,   211,   212,   213,   214,   215,   215,   215,   216,   216,
     216,   217,   217,   218,   218,   219,   220,   220,   220,   220,
     220,   220,   221,   222,   223,   224,   224,   224,   225,   226,
     227,   227,   227,   228,   228,   229,   229,   229,   229,   229,
     229,   229,   229,   229,   229,   230,   230,   231,   232,   232,
     232,   232,   233,   233,   233,   233,   233,   233,   233,   234,
     234,   235,   235,   235,   235,   235,   235,   235,   235,   235,
     236,   236,   236,   236,   236,   237,   238,   239,   240,   241,
     242,   242,   242,   243,   243,   243,   243,   243,   243,   244,
     244,   244,   245,   245,   245,   245,   246,   246,   247,   247,
     248,   248,   248,   248,   248,   248,   248,   248,   248,   248,
     248,   249,   249,   249,   250,   250,   250,   251,   251,   252,
     253,   254,   254,   254,   254,   255,   255,   255,   255,   256,
     256,   256,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   281,   280,   282,   280,
     283,   280,   284,   280,   280,   285,   280,   280,   286,   280,
     280,   287,   280,   280,   288,   280,   280,   289,   280,   280,
     290,   280,   280,   291,   280,   280,   292,   280,   280,   293,
     280,   280,   280,   280,   294,   280,   280,   295,   280,   280,
     296,   280,   280,   297,   280,   280,   298,   280,   280,   299,
     280,   280,   300,   280,   280,   301,   280,   280,   302,   280,
     280,   303,   280,   280,   304,   280,   280,   305,   280,   280,
     306,   280,   280,   307,   280,   280,   280,   280,   280,   280,
     280,   280,   280,   280,   280,   280,   308,   308,   308,   309,
     310,   310,   312,   311,   313,   314,   315,   315,   315,   316,
     316,   316,   316,   316,   316,   316,   316,   316,   317,   317,
     318,   318,   318,   319,   319,   319,   321,   320,   320,   322,
     322,   323,   323,   323,   323,   324,   325,   325
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
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
       3,     1,     4,     3,     4,     3,     4,     3,     1,     1,
       1,     2,     2,     2,     2,     1,     2,     2,     2,     2,
       2,     2,     2,     2,     0,     3,     0,     3,     1,     0,
       1,     1,     1,     1,     1,     3,     3,     3,     3,     3,
       0,     3,     2,     3,     2,     2,     6,     9,     9,     6,
       2,     4,     2,     2,     2,     2,     3,     1,     1,     1,
       2,     3,     1,     1,     2,     3,     2,     4,     3,     2,
       4,     3,     2,     3,     2,     1,     1,     2,     4,     3,
       3,     2,     1,     2,     2,     1,     2,     2,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     2,     3,     2,
       1,     2,     3,     4,     3,     2,     2,     2,     2,     2,
       2,     3,     1,     5,     4,     4,     2,     3,     5,     2,
       1,     1,     6,     7,     2,     2,     2,     2,     3,     0,
       3,     3,     2,     3,     3,     4,     5,     4,     3,     4,
       3,     1,     1,     1,     2,     2,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       2,     3,     2,     1,     2,     1,     2,     1,     2,     1,
       2,     1,     2,     1,     2,     1,     2,     1,     2,     1,
       2,     1,     2,     1,     2,     1,     0,     3,     0,     4,
       0,     4,     0,     4,     1,     0,     4,     1,     0,     4,
       1,     0,     4,     1,     0,     4,     1,     0,     4,     1,
       0,     4,     1,     0,     4,     1,     0,     4,     1,     0,
       4,     1,     2,     1,     0,     4,     1,     0,     4,     1,
       0,     4,     1,     0,     4,     1,     0,     4,     1,     0,
       4,     1,     0,     4,     1,     0,     4,     1,     0,     4,
       1,     0,     4,     1,     0,     4,     1,     0,     4,     1,
       0,     4,     1,     0,     4,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     2,     2,     2,     2,     1,     1,
       2,     2,     0,     2,     1,     1,     3,     1,     1,     2,
       2,     1,     2,     3,     4,     4,     4,     1,     1,     1,
       1,     1,     1,     2,     2,     0,     0,     4,     1,     1,
       0,     2,     1,     4,     3,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,    20,     1,     3,    23,    18,    23,     7,    11,
      10,    23,    21,    22,    20,    76,    17,   219,    19,   218,
       6,    72,    73,    64,    65,    66,    67,    68,    75,    69,
      77,    78,    80,    81,    82,    83,    84,    70,    71,    85,
      87,    74,    88,    86,    89,   118,    90,   325,   383,   381,
     360,   389,   395,   392,   398,   401,   404,   407,   378,   375,
     372,   427,   439,   445,   444,   428,   430,   429,    79,   410,
     413,   416,   419,   422,   425,   327,   329,   331,   333,   337,
     335,   343,   341,   339,   345,   366,   369,   386,     0,     0,
     354,   357,   363,   346,     0,    24,     0,     0,   130,   470,
       0,     0,     0,   470,   470,    91,     0,     0,     0,     0,
       0,   470,     0,     0,     0,   470,   470,     0,     0,   286,
     287,   470,     0,    29,    31,    32,    30,     0,     0,    33,
      34,    52,    35,    26,    27,    28,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    48,    47,    49,
      50,    51,    63,   470,    53,   470,    54,   470,    55,   470,
      56,   470,    57,   470,    58,   470,    61,   470,    59,   470,
      60,   470,    62,   470,   117,   426,   431,   442,   442,    16,
      20,   382,   434,   432,   435,   433,   470,   212,   213,    25,
     114,   309,   310,   307,   308,   461,   460,   462,   101,   113,
       0,     0,   103,     0,   194,   200,     0,   189,   469,   205,
     210,    20,   477,   476,     0,     0,   268,   269,     0,   448,
     465,   100,   465,   250,   447,   475,     0,     0,   472,   214,
     224,   234,   226,     0,   232,   237,   241,   251,   253,   256,
       0,     0,   255,   242,   254,   252,   245,   248,     0,     0,
     261,     0,   265,   266,   267,   276,     0,   303,     0,   302,
       0,   301,     0,     0,     0,   323,     0,   320,     0,     0,
      12,   324,   326,   328,   330,   332,   334,   336,   338,   340,
     342,   344,   379,   358,   387,   393,   390,   396,   399,   402,
     405,   376,   373,   370,   408,   411,   414,   417,   420,   423,
     364,   367,   384,   348,   350,   352,   355,   361,   441,   470,
     440,    15,   438,   347,     0,   468,   470,   105,   312,   313,
     314,   311,     0,   106,     0,   470,   128,   188,   127,     0,
       0,     0,     0,   376,   129,     0,   216,   221,    96,     0,
       0,   131,   132,   133,     0,    92,   470,    94,   463,     0,
     464,   458,   459,     0,     0,     0,     0,     0,   451,   457,
       0,   201,   471,   225,   231,     0,   235,   236,     0,   233,
     257,   259,     0,   243,   246,   244,   247,   240,     0,   239,
     262,     0,   281,   277,     0,     0,     0,   289,   280,   290,
     291,   293,   317,   316,   318,   315,     0,   294,     0,   319,
     321,   203,   121,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   443,
     437,   436,   466,   102,   115,   107,     0,     0,   111,   104,
     189,   189,    23,   189,   211,    20,   134,   134,   143,     0,
     140,   141,   142,   189,   189,   189,   139,     0,   134,   249,
       0,   452,     0,   450,     0,   449,     0,   446,   474,   230,
     227,   258,   238,   263,   274,   285,    20,   275,     0,   279,
       0,   295,     0,   299,   380,   359,   388,   394,   391,   397,
     400,   403,   406,   377,   374,   371,   409,   412,   415,   418,
     421,   424,   365,   368,   385,   349,   351,   353,   356,   362,
     470,   116,   306,   304,   305,   108,   198,   199,    23,     0,
      23,    23,   126,     0,    97,     0,    93,   138,   144,   189,
     189,   151,   145,   189,   189,   156,   146,   189,   189,   161,
       0,     0,     0,   189,    98,     0,     0,     0,   453,   473,
       0,   278,    23,     0,   273,   296,   467,     8,    23,   122,
       9,     0,     0,     0,   125,   209,   206,   135,   189,   147,
     189,   149,   189,   152,   189,   154,   189,   157,   189,   159,
     163,     0,   165,     0,   167,     0,   134,   456,   455,   454,
      20,   288,     0,   189,   189,   189,   124,    20,   148,   150,
     153,   155,   158,   160,   169,     0,     0,     0,     0,   162,
     168,   175,     0,     0,   164,   170,   166,    99,    23,   271,
     195,   197,   196,   123,     0,   177,   176,   189,   179,   178,
     189,   181,   180,   183,   182,   172,   171,   174,   173,   282,
     208,   207,     0,     0,   283,   187,   190,   191,   193,   192,
     185
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
     453,   454,   455,   609,   614,   615,   629,   630,   626,   627,
     328,   645,   646,   647,   206,   564,   331,   130,   131,   132,
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

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -511
static const yytype_int16 yypact[] =
{
    -511,    35,  -511,  -511,  -511,    45,   -29,    50,  -511,  -511,
    -511,  1236,  -511,  -511,  -511,  -511,    15,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,   466,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,   963,  1037,
    -511,  -511,  -511,  -511,   148,  -511,    23,   919,  1789,  1905,
    1636,    39,    92,  1905,  1905,  -511,    89,   -39,  1905,    37,
     467,  1905,   197,    47,   111,  1905,  1905,    66,   287,  -511,
    -511,  1559,   -39,  -511,  -511,  -511,  -511,   156,    23,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  1905,  -511,  1905,  -511,  1905,  -511,  1905,
    -511,  1905,  -511,  1905,  -511,  1905,  -511,  1905,  -511,  1905,
    -511,  1905,  -511,  1905,  1986,  -511,  -511,  -511,  -511,    27,
    -511,  -511,  -511,  -511,  -511,  -511,  1713,  -511,  -511,    15,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
      22,   236,  -511,   604,  -511,    46,    61,  2024,  1986,  -511,
      15,  1986,  -511,  -511,   154,   226,  -511,  -511,    26,  -511,
     -28,    18,   -28,  -511,  2129,  -511,   120,   256,   -39,  1986,
    -511,  -511,  1905,    28,  1905,  -511,  -511,  -511,  -511,  -511,
      31,   889,  -511,    94,  -511,  -511,   106,   -20,   456,    89,
    -511,    29,  -511,  -511,  -511,  -511,  2164,  -511,  1905,  -511,
     304,  -511,   255,   274,  1905,  -511,   323,  -511,   327,   342,
      15,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  1713,
    -511,    27,  -511,  -511,    21,    88,  1905,  -511,  -511,  -511,
    -511,  -511,    24,  -511,   344,  1905,  -511,  -511,  -511,  1905,
    1905,    23,  1905,   652,  -511,    33,  -511,  -511,  -511,  1905,
     245,  -511,  -511,  -511,  1905,  -511,  1905,  -511,  -511,    89,
    -511,  -511,  -511,   141,   242,   387,   152,    89,  -511,  -511,
     181,  -511,  -511,  1986,  -511,  1905,  -511,  -511,   380,  1986,
    -511,  -511,   243,  -511,  -511,  -511,  -511,  -511,    89,    18,
    -511,   424,  -511,  -511,   309,  1111,   313,    25,  -511,  1986,
    -511,  -511,  -511,  -511,  -511,  -511,    48,  -511,   461,  1986,
    -511,  -511,  -511,  1905,  1905,  1905,  1905,  1905,  1905,  1905,
    1905,  1905,  1905,  1905,  1905,  1905,  1905,  1905,  1905,  1905,
    1905,  1905,  1905,  1905,  1905,  1905,  1905,  1905,  1905,  -511,
    -511,  -511,  -511,  -511,   194,  -511,    54,   465,  -511,  -511,
    2024,  2024,    15,  2024,  -511,  -511,   281,  1835,  -511,   474,
    -511,  -511,  -511,   265,   265,   265,   119,  1945,   281,  -511,
     152,  -511,   152,  -511,   152,  -511,   209,  -511,   -39,  1986,
    -511,  -511,    18,  -511,  -511,  -511,   439,  -511,   565,  -511,
     310,  -511,   476,  -511,   326,   293,   321,   321,   321,   321,
     321,   321,   321,   326,  -511,  2062,   321,   321,   321,   321,
     321,   321,  2062,  2100,   321,   315,   315,   293,   293,   293,
    1905,  -511,  -511,  -511,  -511,  -511,  -511,  -511,   318,   322,
    -511,  1361,   227,   690,  -511,   328,  -511,  -511,  -511,   265,
     265,   101,  -511,   265,   265,   -26,  -511,   265,   265,   146,
      42,   164,   184,   265,  -511,   257,   261,   301,  -511,  -511,
     359,  -511,  -511,   199,  -511,  -511,  -511,  -511,  1486,  -511,
    -511,  1905,  1905,  1905,   227,  -511,   425,  -511,   265,   355,
     265,   358,   265,   355,   265,   361,   265,   358,   265,   361,
    -511,   544,  -511,   641,  -511,   641,   281,  -511,  -511,  -511,
    -511,  -511,   493,  2024,  2024,  2024,   227,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,   109,   207,   176,   302,  -511,
    -511,  -511,    68,   189,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,   815,  -511,  -511,   265,  -511,  -511,
     265,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,   442,
    -511,  -511,   688,   120,  -511,  -511,  -511,  -511,  -511,  -511,
    -511
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -511,  -511,  -511,  -511,  -511,  -511,    -7,  -511,   -93,   -86,
    -206,    -2,  -424,   -10,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -241,  -239,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,   139,  -511,  -511,   -49,  -511,  -511,  -511,
    -434,  -511,   -52,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -418,  -346,  -395,  -511,  -103,  -263,  -511,  -511,  -313,  -511,
    -107,  -511,  -160,  -511,  -511,  -510,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -228,   244,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,   117,  -511,  -511,  -511,  -511,
      97,   397,   407,   408,    99,   270,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
      -9,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,
    -511,  -511,  -511,  -511,  -511,  -511,  -511,  -511,   225,   -98,
    -511,   367,  -511,  -511,  -511,  -105,  -511,   136,  -218,   -48,
    -305,  -511,   -38,  -115,  -222,   -99
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -471
static const yytype_int16 yytable[] =
{
      20,    94,   174,   215,   360,   335,   359,   268,   222,   252,
     189,   434,   179,   526,   210,   385,   222,   386,   520,    14,
     434,   379,   430,   317,   544,   435,   375,   338,   269,   364,
     380,   256,   212,   225,   444,     3,   533,   537,   230,   181,
     212,   220,   270,   580,    62,    -5,    15,   445,   250,   481,
      -4,   339,   340,   365,   596,   512,   329,   330,   530,   534,
      40,   209,    42,   180,   318,   216,   217,   255,   540,   635,
     542,    14,  -110,   235,   -95,   251,  -229,   253,   254,   183,
     185,   366,   367,   267,   376,  -223,   623,  -217,   226,   207,
     219,   211,   319,   212,   557,  -260,  -298,   341,   342,   229,
     334,   225,    15,   213,   343,   320,   436,   529,   327,   538,
     625,   213,   219,   362,    14,   271,   574,   272,   321,   273,
     578,   274,   222,   275,   222,   276,   513,   277,   520,   278,
     436,   279,   514,   280,   568,   281,   381,  -465,   572,  -189,
    -222,   370,  -189,  -189,  -189,  -189,   385,   349,   478,   431,
     472,   222,   617,   344,  -465,   336,   226,   212,   220,  -465,
    -465,    62,   187,   188,   213,   582,  -465,  -465,   605,   606,
     607,   608,   348,  -465,   350,  -186,  -465,   631,   311,   332,
     220,  -186,  -465,    62,   570,   584,  -186,  -186,  -465,  -465,
     637,   576,   225,  -186,   639,  -465,   541,   542,   236,   373,
     212,   237,  -215,   372,  -120,   556,   351,  -465,   628,  -465,
    -465,   374,   352,   540,   541,   542,  -465,   432,  -465,   238,
     239,   240,   241,   363,    62,   369,   242,   337,   213,  -465,
     327,  -465,  -465,   456,   561,   562,   563,   323,  -465,   523,
     540,   541,  -186,   243,   459,   442,   448,  -272,  -186,   389,
     327,   222,   467,  -186,  -186,   399,   391,   361,   466,   222,
    -186,  -189,  -189,  -189,  -189,  -189,  -189,  -189,   460,   471,
     550,   213,   552,  -184,  -220,   397,   244,   245,   318,  -184,
     222,  -189,  -189,  -189,  -189,  -189,  -189,  -189,   257,   605,
     606,   607,   608,  -189,   632,   634,   392,   318,   393,   322,
     324,   246,   247,   633,  -202,   390,   319,   351,   458,   468,
     474,   327,   258,   352,   477,   392,   318,   393,   610,   320,
     440,   441,   511,   443,   400,   319,   191,   192,   401,  -136,
     447,   327,   321,   516,   517,   457,   522,   548,   320,  -189,
    -189,  -189,   394,   402,   319,   438,   531,   535,   539,   636,
     638,   321,  -292,   549,   340,   553,   469,   320,   193,   194,
     292,   394,   545,   -13,   546,   259,   547,    45,  -186,   462,
     321,  -322,   283,   590,  -186,  -204,   476,   282,   283,  -186,
    -186,   470,   292,   283,   618,   587,  -186,   291,   292,   588,
    -119,   624,  -112,   292,   484,   485,   486,   487,   488,   489,
     490,   491,   492,   493,   494,   495,   496,   497,   498,   499,
     500,   501,   502,   503,   504,   505,   506,   507,   508,   509,
     649,   649,   569,   571,   648,   473,   573,   575,  -228,   589,
     577,   579,   521,   581,   583,   585,   586,   305,   306,   307,
     597,   303,   304,   305,   306,   307,   303,   304,   305,   306,
     307,   542,   351,   541,   592,   540,   644,   377,   352,   591,
     237,   598,   483,   599,   439,   600,   515,   601,   231,   602,
     559,   603,  -264,   567,  -284,   527,  -284,   555,   238,   239,
     240,   241,   616,   650,  -284,   242,   620,   621,   622,   461,
     463,   465,   378,   482,   619,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   479,   291,   292,   293,   558,  -300,
     560,    94,   174,  -109,   464,   261,   566,   232,   233,   234,
     642,    48,  -137,   643,  -297,   262,   263,   294,   295,   296,
     297,   298,   299,   398,   429,   244,   245,    61,    62,    63,
      64,  -270,   521,    65,    66,   310,     0,     0,    94,   174,
      67,     0,   593,   594,   595,   300,   301,   302,     0,   303,
     304,   305,   306,   307,     0,     0,   551,     0,   -20,   -20,
     -20,   -20,     0,     0,     0,     0,     0,   -20,   -20,   -20,
     -20,   -20,   -20,   -20,   -20,   -20,   -20,     0,     0,     0,
       0,     0,   -20,    93,   -20,   -20,   -20,   -20,   -20,   -20,
     -20,   -20,   -20,   -20,     0,   326,     0,     0,   558,   -20,
     -20,   -20,   -20,   -20,  -189,  -189,     0,   641,     0,     0,
     -20,   -20,   -20,   -20,   -20,   -20,   -20,   -20,   -20,   -20,
       0,   -20,   -20,   -20,     0,     0,   -20,   -20,   -20,   -20,
     -20,   604,   -20,   -20,   605,   606,   607,   608,     0,   -20,
     -20,     0,  -189,   -20,   -20,   -20,   -20,   -20,   -20,  -188,
    -188,  -188,  -188,  -188,     0,     0,     0,     0,     0,     0,
     327,   -20,   -20,   -20,   -20,   -20,   -20,   -20,   -20,   -20,
     -20,   -20,   -20,   -20,     0,   -20,   -20,   -20,   -20,   -20,
       0,   565,   -20,   -23,   -23,   -23,   -23,     0,     0,     0,
    -188,     0,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,
     -23,   -23,     0,     0,     0,     0,     0,   -23,     0,   -23,
     -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,     0,
       0,     0,     0,     0,   -23,   -23,   -23,   -23,   611,   612,
     613,   605,   606,   607,   608,   -23,   -23,   -23,   -23,   -23,
     -23,   -23,   -23,   -23,   -23,     0,   -23,   -23,   -23,     0,
     225,   -23,   -23,   -23,   -23,   195,   196,   -23,   -23,     0,
       0,     0,   197,     0,   -23,   -23,     0,     0,   -23,   -23,
     -23,   -23,   -23,   -23,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   -23,   -23,   -23,   -23,
     -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,     0,
     -23,   -23,   -23,   -23,   -23,     0,   640,   -23,   -23,   -23,
     -23,   -23,     0,     0,     0,     0,     0,   -23,   -23,   -23,
     -23,   -23,   -23,   -23,   -23,   -23,   -23,     0,     0,     0,
       0,     0,   -23,     0,   -23,   -23,   -23,   -23,   -23,   -23,
     -23,   -23,   -23,   -23,     0,     0,     0,     0,     0,   -23,
     -23,   -23,   -23,     0,     0,     0,     0,     0,     0,     0,
     -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,
       0,   -23,   -23,   -23,     0,     0,   -23,   -23,   -23,   -23,
     371,     0,   -23,   -23,     0,     0,     0,     0,     0,   -23,
     -23,     0,     0,   -23,   -23,   -23,   -23,   -23,   -23,     0,
       0,     0,     0,     0,     0,  -470,     0,     0,     0,     0,
     190,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,
     -23,   -23,   -23,   -23,     0,   -23,   -23,   -23,   -23,   -23,
       0,     0,   -23,     0,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,     0,    58,    59,    60,   191,   192,
      61,    62,    63,    64,   182,     0,    65,    66,     0,     0,
       0,     0,     0,    67,     0,     0,     0,    69,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,     0,
     193,   194,     0,     0,     0,     0,   195,   196,     0,     0,
       0,     0,     0,   197,     0,    85,    86,    87,     0,    88,
      89,    90,    91,    92,     0,     0,    93,     0,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,     0,    58,
      59,    60,     0,     0,    61,    62,    63,    64,   184,     0,
      65,    66,     0,     0,     0,     0,     0,    67,     0,     0,
       0,    69,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    85,
      86,    87,     0,    88,    89,    90,    91,    92,     0,     0,
      93,     0,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,     0,    58,    59,    60,     0,     0,    61,    62,
      63,    64,   475,     0,    65,    66,     0,     0,     0,     0,
       0,    67,     0,     0,     0,    69,    70,    71,    72,    73,
      74,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    85,    86,    87,     0,    88,    89,    90,
      91,    92,     0,     0,    93,     0,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,     0,    58,    59,    60,
       0,     0,    61,    62,    63,    64,     0,     0,    65,    66,
       0,     0,     0,     0,     0,    67,     0,     0,     0,    69,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    85,    86,    87,
       0,    88,    89,    90,    91,    92,     0,     0,    93,    21,
      22,    23,    24,     0,     0,     0,     0,     0,    25,    26,
       0,     0,    27,    28,    29,    30,    31,    32,     0,     0,
       0,     0,     0,    33,     0,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,     0,     0,     0,     0,     0,
      44,    45,    46,    47,     0,     0,     0,     0,     0,     0,
       0,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,     0,    58,    59,    60,     0,     0,    61,    62,    63,
      64,     0,     0,    65,    66,     0,     0,     0,     0,     0,
      67,    68,     0,     0,    69,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,     0,    88,    89,    90,    91,
      92,     0,     0,    93,    21,    22,    23,    24,     0,     0,
       0,     0,     0,    25,    26,     0,     0,    27,    28,    29,
      30,    31,    32,     0,     0,     0,     0,     0,    33,     0,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
       0,     0,     0,     0,     0,    44,   -14,    46,    47,     0,
       0,     0,     0,     0,     0,     0,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,     0,    58,    59,    60,
       0,     0,    61,    62,    63,    64,     0,     0,    65,    66,
       0,     0,     0,     0,     0,    67,    68,     0,     0,    69,
      70,    71,    72,    73,    74,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
       0,    88,    89,    90,    91,    92,     0,     0,    93,    21,
      22,    23,    24,     0,     0,     0,     0,     0,    25,    26,
       0,     0,    27,    28,    29,    30,    31,    32,     0,     0,
       0,     0,     0,    33,     0,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,     0,     0,     0,     0,     0,
      44,     0,    46,    47,     0,     0,     0,     0,     0,     0,
       0,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,     0,    58,    59,    60,     0,     0,    61,    62,    63,
      64,     0,     0,    65,    66,     0,     0,     0,     0,     0,
      67,    68,     0,     0,    69,    70,    71,    72,    73,    74,
       0,     0,     0,     0,   264,     0,     0,     0,     0,     0,
       0,     0,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,     0,    88,    89,    90,    91,
      92,     0,     0,    93,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,     0,    58,    59,    60,     0,     0,
      61,    62,    63,    64,     0,     0,    65,    66,     0,     0,
     265,     0,     0,    67,     0,     0,     0,    69,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    85,    86,    87,     0,    88,
      89,    90,    91,    92,    14,     0,    93,     0,     0,     0,
       0,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,     0,    58,    59,    60,     0,     0,    61,    62,    63,
      64,     0,     0,    65,    66,     0,     0,     0,     0,     0,
      67,     0,     0,     0,    69,    70,    71,    72,    73,    74,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    85,    86,    87,     0,    88,    89,    90,    91,
      92,   312,     0,    93,     0,     0,     0,     0,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,     0,    58,
      59,    60,     0,     0,    61,    62,    63,    64,     0,     0,
      65,    66,     0,     0,     0,     0,     0,    67,     0,     0,
       0,    69,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    85,
      86,    87,     0,    88,    89,    90,    91,    92,   203,     0,
      93,     0,     0,     0,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,     0,    58,    59,    60,     0,     0,
      61,    62,    63,    64,     0,   204,    65,    66,     0,     0,
       0,     0,     0,    67,     0,     0,     0,    69,    70,    71,
      72,    73,    74,  -136,     0,     0,     0,     0,     0,     0,
       0,   282,   283,   284,   285,   286,   287,   288,   289,   290,
       0,   291,   292,   293,     0,    85,    86,    87,     0,    88,
      89,    90,    91,    92,     0,     0,    93,     0,     0,     0,
       0,     0,     0,   294,   295,   296,   297,   298,   299,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   300,   301,   302,     0,   303,   304,   305,   306,   307,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
       0,    58,    59,    60,     0,     0,    61,    62,    63,    64,
       0,     0,    65,    66,     0,     0,     0,     0,     0,    67,
       0,     0,     0,    69,    70,    71,    72,    73,    74,     0,
       0,   282,   283,   284,   285,   286,   287,   288,   289,   290,
       0,   291,   292,   293,     0,     0,     0,     0,     0,     0,
       0,    85,    86,    87,     0,    88,    89,    90,    91,    92,
       0,     0,    93,   294,   295,   296,   297,   298,   299,     0,
       0,     0,   282,   283,   284,   285,   286,   287,   288,   289,
     290,     0,   291,   292,   293,     0,     0,     0,     0,     0,
       0,   300,   301,   302,     0,   303,   304,   305,   306,   307,
       0,     0,     0,   543,   294,   295,   296,   297,   298,   299,
     282,   283,   284,   285,   286,   287,   288,   289,   290,     0,
     333,   292,   293,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   300,   301,   302,     0,   303,   304,   305,   306,
     307,     0,   294,   295,   296,   297,   298,   299,   282,   283,
     284,   285,   286,   287,   288,   289,   290,     0,   291,   292,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     300,   301,   302,     0,   303,   304,   305,   306,   307,     0,
     294,   295,   296,   297,   298,   299,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   382,   291,   292,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   383,   301,
     302,     0,   303,   304,   305,   306,   307,     0,   294,   295,
     296,   297,   298,   299,   351,     0,     0,     0,     0,    40,
     352,    42,     0,     0,     0,     0,   195,   196,     0,   384,
       0,     0,   180,   197,     0,     0,     0,     0,   302,     0,
     303,   304,   305,   306,   307,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   353,     0,   354,
     355,     0,     0,     0,     0,     0,   356
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-511))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       7,    11,    11,   102,   226,   211,   224,   122,   106,   114,
      96,   316,    14,   447,   100,   256,   114,   256,   442,    48,
     325,   249,     1,     1,   458,     1,    46,     1,   127,     1,
       1,   117,     1,    72,     1,     0,   454,   455,     1,    48,
       1,    69,   128,     1,    72,     0,    75,    14,     1,     1,
       0,    25,    26,    25,   564,     1,    10,    11,   453,   454,
      35,    99,    37,    48,    42,   103,   104,     1,    94,     1,
      96,    48,    48,   111,    48,    28,    48,   115,   116,    88,
      89,    53,    54,   121,   104,    48,   596,    48,   127,    98,
       1,   100,    70,     1,   518,    48,    48,    71,    72,   108,
     207,    72,    75,    72,    78,    83,    82,   453,    66,   455,
       1,    72,     1,   228,    48,   153,   534,   155,    96,   157,
     538,   159,   220,   161,   222,   163,    72,   165,   552,   167,
      82,   169,    78,   171,   529,   173,   251,    48,   533,    97,
      48,   240,   100,   101,   102,   103,   387,   129,   387,   128,
     378,   249,   586,   127,    65,     1,   127,     1,    69,    48,
      71,    72,    14,    15,    72,     1,    77,    78,   100,   101,
     102,   103,   220,    84,   222,    66,    65,     1,   180,   118,
      69,    72,    71,    72,   530,     1,    77,    78,    77,    78,
       1,   537,    72,    84,   618,    84,    95,    96,     1,   105,
       1,     4,    48,   241,    48,   510,    65,   118,     1,   120,
     121,   105,    71,    94,    95,    96,   127,   129,   129,    22,
      23,    24,    25,   232,    72,   234,    29,     1,    72,   118,
      66,   120,   121,   340,     7,     8,     9,     1,   127,   445,
      94,    95,    66,    46,   349,   331,     1,    48,    72,   258,
      66,   349,   357,    77,    78,   264,     1,     1,   356,   357,
      84,    97,    98,    99,   100,   101,   102,   103,   127,    26,
     476,    72,   478,    66,    48,     1,    79,    80,    42,    72,
     378,    97,    98,    99,   100,   101,   102,   103,     1,   100,
     101,   102,   103,    48,   607,   608,    41,    42,    43,   200,
     201,   104,   105,     1,    48,     1,    70,    65,   346,   128,
       1,    66,    25,    71,     1,    41,    42,    43,   581,    83,
     329,   330,   128,   332,     1,    70,    39,    40,     1,    48,
     339,    66,    96,   440,   441,   344,   443,   128,    83,    94,
      95,    96,    87,     1,    70,     1,   453,   454,   455,   612,
     613,    96,    48,   468,    26,    45,   365,    83,    71,    72,
      67,    87,   460,    45,   462,    78,   464,    45,    66,   127,
      96,    48,    57,    14,    72,    48,   385,    56,    57,    77,
      78,     1,    67,    57,   590,   128,    84,    66,    67,   128,
      48,   597,    48,    67,   403,   404,   405,   406,   407,   408,
     409,   410,   411,   412,   413,   414,   415,   416,   417,   418,
     419,   420,   421,   422,   423,   424,   425,   426,   427,   428,
     642,   643,   529,   530,   642,     1,   533,   534,    48,   128,
     537,   538,   442,   540,   541,   542,   543,   122,   123,   124,
      15,   120,   121,   122,   123,   124,   120,   121,   122,   123,
     124,    96,    65,    95,   553,    94,    14,     1,    71,   552,
       4,   568,     1,   570,   325,   572,     1,   574,     1,   576,
     519,   578,    48,   525,    35,     1,    37,     1,    22,    23,
      24,    25,   585,   643,    45,    29,   593,   594,   595,   353,
     354,   355,   248,   396,     1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,   387,    66,    67,    68,   518,    48,
     520,   521,   521,    48,   127,   118,   523,    50,    51,    52,
     627,    55,    48,   630,    48,   118,   118,    88,    89,    90,
      91,    92,    93,   263,   309,    79,    80,    71,    72,    73,
      74,    48,   552,    77,    78,   178,    -1,    -1,   558,   558,
      84,    -1,   561,   562,   563,   116,   117,   118,    -1,   120,
     121,   122,   123,   124,    -1,    -1,     1,    -1,     3,     4,
       5,     6,    -1,    -1,    -1,    -1,    -1,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    -1,    -1,    -1,
      -1,    -1,    27,   127,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,     1,    -1,    -1,   618,    44,
      45,    46,    47,    48,    10,    11,    -1,   624,    -1,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    -1,    -1,    71,    72,    73,    74,
      75,    97,    77,    78,   100,   101,   102,   103,    -1,    84,
      85,    -1,    48,    88,    89,    90,    91,    92,    93,     7,
       8,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      66,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,    -1,   120,   121,   122,   123,   124,
      -1,     1,   127,     3,     4,     5,     6,    -1,    -1,    -1,
      48,    -1,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    -1,    -1,    -1,    44,    45,    46,    47,    97,    98,
      99,   100,   101,   102,   103,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    66,    67,    68,    -1,
      72,    71,    72,    73,    74,    77,    78,    77,    78,    -1,
      -1,    -1,    84,    -1,    84,    85,    -1,    -1,    88,    89,
      90,    91,    92,    93,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,    -1,
     120,   121,   122,   123,   124,    -1,     1,   127,     3,     4,
       5,     6,    -1,    -1,    -1,    -1,    -1,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    -1,    -1,    -1,
      -1,    -1,    27,    -1,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,    44,
      45,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    -1,    -1,    71,    72,    73,    74,
       1,    -1,    77,    78,    -1,    -1,    -1,    -1,    -1,    84,
      85,    -1,    -1,    88,    89,    90,    91,    92,    93,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,
       1,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,    -1,   120,   121,   122,   123,   124,
      -1,    -1,   127,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    39,    40,
      71,    72,    73,    74,     1,    -1,    77,    78,    -1,    -1,
      -1,    -1,    -1,    84,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    -1,    -1,    -1,    -1,    77,    78,    -1,    -1,
      -1,    -1,    -1,    84,    -1,   116,   117,   118,    -1,   120,
     121,   122,   123,   124,    -1,    -1,   127,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    -1,    -1,    71,    72,    73,    74,     1,    -1,
      77,    78,    -1,    -1,    -1,    -1,    -1,    84,    -1,    -1,
      -1,    88,    89,    90,    91,    92,    93,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,
     117,   118,    -1,   120,   121,   122,   123,   124,    -1,    -1,
     127,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    66,    67,    68,    -1,    -1,    71,    72,
      73,    74,     1,    -1,    77,    78,    -1,    -1,    -1,    -1,
      -1,    84,    -1,    -1,    -1,    88,    89,    90,    91,    92,
      93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   116,   117,   118,    -1,   120,   121,   122,
     123,   124,    -1,    -1,   127,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      -1,    -1,    71,    72,    73,    74,    -1,    -1,    77,    78,
      -1,    -1,    -1,    -1,    -1,    84,    -1,    -1,    -1,    88,
      89,    90,    91,    92,    93,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,   117,   118,
      -1,   120,   121,   122,   123,   124,    -1,    -1,   127,     3,
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
      -1,    -1,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,    -1,   120,   121,   122,   123,
     124,    -1,    -1,   127,     3,     4,     5,     6,    -1,    -1,
      -1,    -1,    -1,    12,    13,    -1,    -1,    16,    17,    18,
      19,    20,    21,    -1,    -1,    -1,    -1,    -1,    27,    -1,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    -1,    -1,    -1,    -1,    44,    45,    46,    47,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    66,    67,    68,
      -1,    -1,    71,    72,    73,    74,    -1,    -1,    77,    78,
      -1,    -1,    -1,    -1,    -1,    84,    85,    -1,    -1,    88,
      89,    90,    91,    92,    93,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
      -1,   120,   121,   122,   123,   124,    -1,    -1,   127,     3,
       4,     5,     6,    -1,    -1,    -1,    -1,    -1,    12,    13,
      -1,    -1,    16,    17,    18,    19,    20,    21,    -1,    -1,
      -1,    -1,    -1,    27,    -1,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,
      44,    -1,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    -1,    -1,    71,    72,    73,
      74,    -1,    -1,    77,    78,    -1,    -1,    -1,    -1,    -1,
      84,    85,    -1,    -1,    88,    89,    90,    91,    92,    93,
      -1,    -1,    -1,    -1,    25,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,    -1,   120,   121,   122,   123,
     124,    -1,    -1,   127,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    -1,    -1,
      71,    72,    73,    74,    -1,    -1,    77,    78,    -1,    -1,
      81,    -1,    -1,    84,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   116,   117,   118,    -1,   120,
     121,   122,   123,   124,    48,    -1,   127,    -1,    -1,    -1,
      -1,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    -1,    -1,    71,    72,    73,
      74,    -1,    -1,    77,    78,    -1,    -1,    -1,    -1,    -1,
      84,    -1,    -1,    -1,    88,    89,    90,    91,    92,    93,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   116,   117,   118,    -1,   120,   121,   122,   123,
     124,    48,    -1,   127,    -1,    -1,    -1,    -1,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    66,
      67,    68,    -1,    -1,    71,    72,    73,    74,    -1,    -1,
      77,    78,    -1,    -1,    -1,    -1,    -1,    84,    -1,    -1,
      -1,    88,    89,    90,    91,    92,    93,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   116,
     117,   118,    -1,   120,   121,   122,   123,   124,    49,    -1,
     127,    -1,    -1,    -1,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    66,    67,    68,    -1,    -1,
      71,    72,    73,    74,    -1,    76,    77,    78,    -1,    -1,
      -1,    -1,    -1,    84,    -1,    -1,    -1,    88,    89,    90,
      91,    92,    93,    48,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    -1,   116,   117,   118,    -1,   120,
     121,   122,   123,   124,    -1,    -1,   127,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    89,    90,    91,    92,    93,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   116,   117,   118,    -1,   120,   121,   122,   123,   124,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    -1,    -1,    71,    72,    73,    74,
      -1,    -1,    77,    78,    -1,    -1,    -1,    -1,    -1,    84,
      -1,    -1,    -1,    88,    89,    90,    91,    92,    93,    -1,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    66,    67,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   116,   117,   118,    -1,   120,   121,   122,   123,   124,
      -1,    -1,   127,    88,    89,    90,    91,    92,    93,    -1,
      -1,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    67,    68,    -1,    -1,    -1,    -1,    -1,
      -1,   116,   117,   118,    -1,   120,   121,   122,   123,   124,
      -1,    -1,    -1,   128,    88,    89,    90,    91,    92,    93,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      66,    67,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   116,   117,   118,    -1,   120,   121,   122,   123,
     124,    -1,    88,    89,    90,    91,    92,    93,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     116,   117,   118,    -1,   120,   121,   122,   123,   124,    -1,
      88,    89,    90,    91,    92,    93,    56,    57,    58,    59,
      60,    61,    62,    63,    64,     1,    66,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    14,   117,
     118,    -1,   120,   121,   122,   123,   124,    -1,    88,    89,
      90,    91,    92,    93,    65,    -1,    -1,    -1,    -1,    35,
      71,    37,    -1,    -1,    -1,    -1,    77,    78,    -1,    45,
      -1,    -1,    48,    84,    -1,    -1,    -1,    -1,   118,    -1,
     120,   121,   122,   123,   124,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,   120,
     121,    -1,    -1,    -1,    -1,    -1,   127
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   131,   132,     0,   133,   140,   141,   134,   136,   137,
     142,   143,   144,   246,    48,    75,   139,   159,   225,   226,
     136,     3,     4,     5,     6,    12,    13,    16,    17,    18,
      19,    20,    21,    27,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    44,    45,    46,    47,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    66,    67,
      68,    71,    72,    73,    74,    77,    78,    84,    85,    88,
      89,    90,    91,    92,    93,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   120,   121,
     122,   123,   124,   127,   143,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   178,   179,   184,   185,   186,   187,
     217,   218,   219,   220,   221,   222,   223,   224,   227,   228,
     229,   231,   232,   236,   237,   238,   239,   240,   241,   243,
     248,   256,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   309,   310,   313,   314,   141,
      48,   280,     1,   280,     1,   280,   281,    14,    15,   139,
       1,    39,    40,    71,    72,    77,    78,    84,   182,   251,
     252,   253,   318,    49,    76,   188,   214,   280,   280,   322,
     139,   280,     1,    72,   325,   325,   322,   322,   175,     1,
      69,   234,   309,   315,   319,    72,   127,   323,   324,   280,
       1,     1,    50,    51,    52,   322,     1,     4,    22,    23,
      24,    25,    29,    46,    79,    80,   104,   105,   233,   235,
       1,    28,   315,   322,   322,     1,   139,     1,    25,    78,
     249,   251,   252,   253,    25,    81,   257,   322,   323,   325,
     139,   322,   322,   322,   322,   322,   322,   322,   322,   322,
     322,   322,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    66,    67,    68,    88,    89,    90,    91,    92,    93,
     116,   117,   118,   120,   121,   122,   123,   124,   311,   312,
     311,   141,    48,   308,   320,   322,   180,     1,    42,    70,
      83,    96,   254,     1,   254,   181,     1,    66,   210,    10,
      11,   216,   118,    66,   210,   140,     1,     1,     1,    25,
      26,    71,    72,    78,   127,   176,   189,   192,   319,   129,
     319,    65,    71,   118,   120,   121,   127,   316,   317,   318,
     324,     1,   323,   280,     1,    25,    53,    54,   230,   280,
     325,     1,   322,   105,   105,    46,   104,     1,   235,   234,
       1,   323,     1,    14,    45,   170,   171,   244,   245,   280,
       1,     1,    41,    43,    87,   254,   255,     1,   255,   280,
       1,     1,     1,   293,   286,   295,   297,   296,   298,   299,
     300,   301,   292,   291,   290,   302,   303,   304,   305,   306,
     307,   288,   289,   294,   282,   283,   284,   285,   287,   308,
       1,   128,   129,   183,   320,     1,    82,   250,     1,   183,
     280,   280,   139,   280,     1,    14,   177,   280,     1,   193,
     194,   195,   196,   200,   201,   202,   210,   280,   322,   315,
     127,   317,   127,   317,   127,   317,   309,   315,   128,   280,
       1,    26,   234,     1,     1,     1,   280,     1,   171,   245,
     247,     1,   250,     1,   280,   280,   280,   280,   280,   280,
     280,   280,   280,   280,   280,   280,   280,   280,   280,   280,
     280,   280,   280,   280,   280,   280,   280,   280,   280,   280,
     321,   128,     1,    72,    78,     1,   210,   210,   135,   138,
     142,   143,   210,   140,   190,   191,   190,     1,   197,   201,
     202,   210,   198,   200,   202,   210,   199,   200,   201,   210,
      94,    95,    96,   128,   190,   309,   309,   309,   128,   323,
     140,     1,   140,    45,   242,     1,   320,   142,   143,   186,
     143,     7,     8,     9,   215,     1,   136,   192,   202,   210,
     201,   210,   202,   210,   200,   210,   201,   210,   200,   210,
       1,   210,     1,   210,     1,   210,   210,   128,   128,   128,
      14,   138,   325,   280,   280,   280,   215,    15,   210,   210,
     210,   210,   210,   210,    97,   100,   101,   102,   103,   203,
     205,    97,    98,    99,   204,   205,   204,   190,   140,     1,
     210,   210,   210,   215,   140,     1,   208,   209,     1,   206,
     207,     1,   208,     1,   208,     1,   205,     1,   205,   142,
       1,   136,   210,   210,    14,   211,   212,   213,   318,   324,
     212
};

#define yyerrok      (yyerrstatus = 0)
#define yyclearin (yychar = YYEMPTY)
#define YYEMPTY      (-2)
#define YYEOF     0

#define YYACCEPT  goto yyacceptlab
#define YYABORT      goto yyabortlab
#define YYERROR      goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL    goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)               \
do                      \
  if (yychar == YYEMPTY && yylen == 1)          \
    {                      \
      yychar = (Token);                \
      yylval = (Value);                \
      YYPOPSTACK (1);                  \
      goto yybackup;                \
    }                      \
  else                        \
    {                      \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                   \
    }                      \
while (YYID (0))


#define YYTERROR  1
#define YYERRCODE 256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)           \
    do                           \
      if (YYID (N))                                                    \
   {                       \
     (Current).first_line   = YYRHSLOC (Rhs, 1).first_line; \
     (Current).first_column = YYRHSLOC (Rhs, 1).first_column;  \
     (Current).last_line    = YYRHSLOC (Rhs, N).last_line;     \
     (Current).last_column  = YYRHSLOC (Rhs, N).last_column;   \
   }                       \
      else                       \
   {                       \
     (Current).first_line   = (Current).last_line   =    \
       YYRHSLOC (Rhs, 0).last_line;          \
     (Current).first_column = (Current).last_column =    \
       YYRHSLOC (Rhs, 0).last_column;           \
   }                       \
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)         \
do {                 \
  if (yydebug)             \
    YYFPRINTF Args;           \
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)           \
do {                            \
  if (yydebug)                        \
    {                           \
      YYFPRINTF (stderr, "%s ", Title);                 \
      yy_symbol_print (stderr,                    \
        Type, Value); \
      YYFPRINTF (stderr, "\n");                   \
    }                           \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
   break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)            \
do {                       \
  if (yydebug)                   \
    yy_stack_print ((Bottom), (Top));           \
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
        yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
             &(yyvsp[(yyi + 1) - (yynrhs)])
                         );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)      \
do {              \
  if (yydebug)          \
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

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
#ifndef  YYINITDEPTH
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
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
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
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
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
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

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

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

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
   break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
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
  int yytoken;
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

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

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
      yychar = YYLEX;
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
  *++yyvsp = yylval;

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
     `$$ = $1'.

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

/* Line 1806 of yacc.c  */
#line 160 "./yaccsrc.y"
    { level = 0;
                                         if ( get_options_flag( parser_data.TSD->currlevel, EXT_CALLS_AS_FUNCS )
                                           && !get_options_flag( parser_data.TSD->currlevel, EXT_STRICT_ANSI ) )
                                            start_parendepth = 1;
                                         else
                                            start_parendepth = 0;
                                         parendepth = 0; }
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 170 "./yaccsrc.y"
    { (yyval) = optgluelast( (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]) );
                                         (yyval)->o.last = NULL;
                                         EndProg( (yyval) ) ; }
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 173 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]);
                                         if ( (yyval) != NULL )
                                            (yyval)->o.last = NULL;
                                         EndProg( (yyval) ); }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 179 "./yaccsrc.y"
    { /* fixes bug 579711 */
                                         (yyval) = optgluelast( (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]) ); }
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 181 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 184 "./yaccsrc.y"
    { /* fixes bug 579711 */
                                         (yyval) = optgluelast( (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]) ); }
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 186 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 189 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ; }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 190 "./yaccsrc.y"
    { exiterror( ERR_UNMATCHED_END, 1 ); }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 198 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 199 "./yaccsrc.y"
    { (yyval) = NULL; }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 202 "./yaccsrc.y"
    { (yyval) = optgluelast( (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]) ); }
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 203 "./yaccsrc.y"
    { (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 206 "./yaccsrc.y"
    { (yyval) = optgluelast( (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]) ); }
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 207 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 210 "./yaccsrc.y"
    { (yyval) = optgluelast( (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]) ); }
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 211 "./yaccsrc.y"
    { (yyval) = NULL; }
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 218 "./yaccsrc.y"
    { tmpchr=parser_data.tstart;
                                         tmplno=parser_data.tline; }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 222 "./yaccsrc.y"
    { (yyval)=(yyvsp[(2) - (2)]); }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 225 "./yaccsrc.y"
    { (yyval) = optgluelast( (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]) ); }
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 268 "./yaccsrc.y"
    { (yyval) = makenode(X_CALL,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 272 "./yaccsrc.y"
    { (yyval) = makenode(X_DO,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         level++; }
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 277 "./yaccsrc.y"
    { (yyval) = makenode(X_EXIT,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 281 "./yaccsrc.y"
    { (yyval) = makenode(X_IF,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         level++; }
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 286 "./yaccsrc.y"
    { (yyval) = makenode(X_ITERATE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 290 "./yaccsrc.y"
    { (yyval) = makenode(X_LEAVE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 294 "./yaccsrc.y"
    { (yyval) = makenode(X_SAY,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 298 "./yaccsrc.y"
    { (yyval) = makenode(X_RETURN,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 302 "./yaccsrc.y"
    { (yyval) = makenode(X_ADDR_N,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 306 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_ARG,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 310 "./yaccsrc.y"
    { (yyval) = makenode(X_DROP,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 314 "./yaccsrc.y"
    { (yyval) = makenode(X_IPRET,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 318 "./yaccsrc.y"
    { (yyval) = makenode(X_LABEL,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 322 "./yaccsrc.y"
    { (yyval) = makenode(X_NULL,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 326 "./yaccsrc.y"
    { (yyval) = makenode(0,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 330 "./yaccsrc.y"
    { (yyval) = makenode(X_OPTIONS,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 334 "./yaccsrc.y"
    { (yyval) = makenode(0,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 338 "./yaccsrc.y"
    { (yyval) = makenode(X_PROC,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 342 "./yaccsrc.y"
    { (yyval) = makenode(X_PULL,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 346 "./yaccsrc.y"
    { (yyval) = makenode(X_PUSH,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 350 "./yaccsrc.y"
    { (yyval) = makenode(X_QUEUE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 354 "./yaccsrc.y"
    { (yyval) = makenode(X_SELECT,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         level++; }
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 359 "./yaccsrc.y"
    { (yyval) = makenode(X_SIG_LAB,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 363 "./yaccsrc.y"
    { (yyval) = makenode(X_WHEN,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 367 "./yaccsrc.y"
    { (yyval) = makenode(X_OTHERWISE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 371 "./yaccsrc.y"
    { (yyval) = makenode(X_TRACE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 375 "./yaccsrc.y"
    { (yyval) = makenode(X_UPPER_VAR,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 379 "./yaccsrc.y"
    { (yyval) = current = (yyvsp[(1) - (1)]) ; }
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 383 "./yaccsrc.y"
    { current->type = X_ADDR_V ;
                                         current->p[0] = (yyvsp[(2) - (3)]) ;
                                         current->p[1] = (yyvsp[(3) - (3)]) ; }
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 386 "./yaccsrc.y"
    { exiterror( ERR_STRING_EXPECTED, 1, __reginatext ) ;}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 387 "./yaccsrc.y"
    { current->type = X_ADDR_S ; }
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 388 "./yaccsrc.y"
    { exiterror( ERR_STRING_EXPECTED, 1, __reginatext ) ;}
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 390 "./yaccsrc.y"
    { current->name = (streng *)(yyvsp[(1) - (3)]) ;
                                         current->type = X_ADDR_N ;
                                         current->p[0] = (yyvsp[(2) - (3)]) ;
                                         current->p[1] = (yyvsp[(3) - (3)]) ; }
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 394 "./yaccsrc.y"
    { current->type = X_ADDR_V ;
                                         current->p[0] = (yyvsp[(2) - (5)]) ;
                                         current->p[1] = (yyvsp[(5) - (5)]) ;
                                         current->u.nonansi = 1 ; }
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 400 "./yaccsrc.y"
    { (yyval) = makenode( X_PARSE, 2, (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]) );
                                         (yyval)->u.parseflags = PARSE_UPPER;
                                         (yyval)->lineno = (yyvsp[(1) - (2)])->lineno;
                                         (yyval)->charnr = (yyvsp[(1) - (2)])->charnr; }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 406 "./yaccsrc.y"
    { parendepth = start_parendepth; }
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 407 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (4)]);
                                         (yyval)->p[0] = (yyvsp[(4) - (4)]);
                                         (yyval)->name = (streng *) (yyvsp[(2) - (4)]);
                                         parendepth = 0; }
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 411 "./yaccsrc.y"
    { parendepth = start_parendepth; }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 412 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (4)]);
                                         (yyval)->type = X_EX_FUNC;
                                         (yyval)->p[0] = (yyvsp[(4) - (4)]);
                                         (yyval)->name = (streng *) (yyvsp[(2) - (4)]);
                                         parendepth = 0; }
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 417 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 1, "ERROR FAILURE HALT NOTREADY", __reginatext ) ;}
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 418 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 2, "ERROR FAILURE HALT NOTREADY", __reginatext ) ;}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 419 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 420 "./yaccsrc.y"
    { exiterror( ERR_STRING_EXPECTED, 3, __reginatext ) ;}
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 422 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (4)]) ;
                                         (yyval)->type = X_CALL_SET ;
                                         (yyval)->p[0] = (yyvsp[(2) - (4)]) ;
                                         (yyval)->name = (streng *)(yyvsp[(4) - (4)]) ;
                                         (yyval)->p[1] = (yyvsp[(3) - (4)]) ; }
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 427 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (3)]) ;
                                         (yyval)->type = X_CALL_SET ;
                                         (yyval)->p[0] = (yyvsp[(2) - (3)]) ;
                                         (yyval)->name = NULL ;
                                         (yyval)->p[1] = (yyvsp[(3) - (3)]) ; }
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 432 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 433 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (3)]) ;
                                         (yyval)->type = X_CALL_SET ;
                                         (yyval)->p[0] = (yyvsp[(2) - (3)]) ;
                                         (yyval)->p[1] = (yyvsp[(3) - (3)]) ; }
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 439 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 440 "./yaccsrc.y"
    { exiterror( ERR_STRING_EXPECTED, 2, __reginatext );}
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 443 "./yaccsrc.y"
    {
                                         /*
                                          * "call_args" accepted probably with
                                          * surrounding parentheses. Strip them.
                                          */
                                         (yyval) = reduce_expr_list( (yyvsp[(1) - (1)]),
                                                                REDUCE_CALL );
                                       }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 451 "./yaccsrc.y"
    { exiterror(ERR_UNEXPECTED_PARAN, 2); }
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 454 "./yaccsrc.y"
    { (yyval) = makenode(X_COMMAND,0) ;
                                         (yyval)->charnr = tmpchr ;
                                         (yyval)->lineno = tmplno;
                                         (yyval)->p[0] = (yyvsp[(1) - (1)]) ; }
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 460 "./yaccsrc.y"
    { (yyval) = makenode(X_END,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         level--; }
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 466 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->name = (streng*)((yyvsp[(2) - (2)])) ; }
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 468 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ; }
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 469 "./yaccsrc.y"
    {  exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 473 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (6)]);
                                         (yyval)->p[0] = (yyvsp[(2) - (6)]);
                                         (yyval)->p[1] = (yyvsp[(3) - (6)]);
                                         (yyval)->p[2] = optgluelast( (yyvsp[(4) - (6)]), (yyvsp[(5) - (6)]) );
                                         if ( (yyval)->p[2] )
                                            (yyval)->p[2]->o.last = NULL;
                                         (yyval)->p[3] = (yyvsp[(6) - (6)]);
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
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 494 "./yaccsrc.y"
    { (yyval) =makenode(X_REP,4,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(6) - (7)]),(yyvsp[(7) - (7)])) ;
                                         (yyval)->name = (streng *)(yyvsp[(1) - (7)]) ;
                                         checkdosyntax((yyval)) ; }
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 498 "./yaccsrc.y"
    { (yyval) =makenode(X_REP,3,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)])) ;
                                         (yyval)->name = (streng *)(yyvsp[(1) - (6)]) ;
                                         checkdosyntax((yyval)) ; }
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 502 "./yaccsrc.y"
    { (yyval) = makenode(X_REP,2,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)])) ;
                                         (yyval)->name = (streng *)(yyvsp[(1) - (5)]) ;
                                         checkdosyntax((yyval)) ; }
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 505 "./yaccsrc.y"
    { (yyval) = makenode(X_REP,1,(yyvsp[(3) - (4)])) ;
                                         (yyval)->name = (streng *)(yyvsp[(1) - (4)]) ;
                                         checkdosyntax((yyval)) ; }
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 508 "./yaccsrc.y"
    { (yyval) = makenode(X_REP_FOREVER,0) ; }
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 509 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 16, "WHILE UNTIL", __reginatext ) ; }
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 510 "./yaccsrc.y"
    { (yyvsp[(1) - (2)]) = makenode(X_DO_EXPR,1,(yyvsp[(1) - (2)])) ;
                                         (yyval) = makenode(X_REP,2,NULL,(yyvsp[(1) - (2)])) ; }
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 512 "./yaccsrc.y"
    { (yyval) = NULL ; }
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 515 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 516 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 517 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 520 "./yaccsrc.y"
    { SymbolDetect |= SD_ADDRWITH ;
                                         (yyval) = with = makenode(X_ADDR_WITH,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 524 "./yaccsrc.y"
    { with = NULL ;
                                         SymbolDetect &= ~SD_ADDRWITH ; }
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 526 "./yaccsrc.y"
    { (yyval) = NULL ; }
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 529 "./yaccsrc.y"
    { (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 530 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 5, __reginatext ) ; }
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 531 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 5, __reginatext ) ; }
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 537 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 5, __reginatext ) ; }
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 573 "./yaccsrc.y"
    { with->p[0] = (yyvsp[(4) - (4)]); }
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 574 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 6, __reginatext ) ; }
    break;

  case 164:

/* Line 1806 of yacc.c  */
#line 577 "./yaccsrc.y"
    { with->p[1] = (yyvsp[(4) - (4)]); }
    break;

  case 165:

/* Line 1806 of yacc.c  */
#line 578 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 7, __reginatext ) ; }
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 581 "./yaccsrc.y"
    { with->p[2] = (yyvsp[(4) - (4)]); }
    break;

  case 167:

/* Line 1806 of yacc.c  */
#line 582 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 14, __reginatext ) ; }
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 585 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ; }
    break;

  case 169:

/* Line 1806 of yacc.c  */
#line 586 "./yaccsrc.y"
    { (yyval) = makenode(X_ADDR_WITH, 0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 591 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ; }
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 592 "./yaccsrc.y"
    { (yyval) = (yyvsp[(2) - (2)]) ;
                                         (yyval)->u.of.append = 1 ; }
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 594 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 8, __reginatext ) ; }
    break;

  case 173:

/* Line 1806 of yacc.c  */
#line 595 "./yaccsrc.y"
    { (yyval) = (yyvsp[(2) - (2)]) ; }
    break;

  case 174:

/* Line 1806 of yacc.c  */
#line 596 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 9, __reginatext ) ; }
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 597 "./yaccsrc.y"
    { (yyval) = makenode(X_ADDR_WITH, 0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 176:

/* Line 1806 of yacc.c  */
#line 602 "./yaccsrc.y"
    { /* ANSI extension: nsimsymb is
                                          * used by the standard, but I think
                                          * there are no reasons why using
                                          * it here as a must. FGC
                                          */
                                         (yyval) = (yyvsp[(2) - (2)]) ;
                                         (yyval)->u.of.awt = awtSTREAM;
                                         SymbolDetect |= SD_ADDRWITH ; }
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 610 "./yaccsrc.y"
    { exiterror( ERR_INVALID_OPTION, 1, __reginatext ) ; }
    break;

  case 178:

/* Line 1806 of yacc.c  */
#line 611 "./yaccsrc.y"
    {
                                         streng *tmp = (yyvsp[(2) - (2)])->name;
                                         char *p;

                                         /*
                                          * expect a single dot as the last character
                                          */
                                         p = (char *)memchr( tmp->value, '.', tmp->len );
                                         if ( p != tmp->value + tmp->len - 1 )
                                            exiterror( ERR_INVALID_OPTION, 3, __reginatext );
                                         (yyval) = (yyvsp[(2) - (2)]) ;
                                         (yyval)->u.of.awt = awtSTEM ;
                                         SymbolDetect |= SD_ADDRWITH ; }
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 624 "./yaccsrc.y"
    { exiterror( ERR_INVALID_OPTION, 2, __reginatext ) ; }
    break;

  case 180:

/* Line 1806 of yacc.c  */
#line 625 "./yaccsrc.y"
    {
                                         (yyval) = (yyvsp[(2) - (2)]) ;
                                         (yyval)->u.of.awt = awtLIFO ;
                                         SymbolDetect |= SD_ADDRWITH ; }
    break;

  case 181:

/* Line 1806 of yacc.c  */
#line 629 "./yaccsrc.y"
    { exiterror( ERR_INVALID_OPTION, 100, __reginatext ) ; }
    break;

  case 182:

/* Line 1806 of yacc.c  */
#line 630 "./yaccsrc.y"
    {
                                         (yyval) = (yyvsp[(2) - (2)]) ;
                                         (yyval)->u.of.awt = awtFIFO ;
                                         SymbolDetect |= SD_ADDRWITH ; }
    break;

  case 183:

/* Line 1806 of yacc.c  */
#line 634 "./yaccsrc.y"
    { exiterror( ERR_INVALID_OPTION, 101, __reginatext ) ; }
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 637 "./yaccsrc.y"
    { SymbolDetect &= ~SD_ADDRWITH ; }
    break;

  case 185:

/* Line 1806 of yacc.c  */
#line 638 "./yaccsrc.y"
    { (yyval) = (yyvsp[(3) - (3)]) ; }
    break;

  case 186:

/* Line 1806 of yacc.c  */
#line 641 "./yaccsrc.y"
    { SymbolDetect &= ~SD_ADDRWITH ; }
    break;

  case 187:

/* Line 1806 of yacc.c  */
#line 642 "./yaccsrc.y"
    { (yyval) = (yyvsp[(3) - (3)]) ; }
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 649 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ; }
    break;

  case 191:

/* Line 1806 of yacc.c  */
#line 650 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ; }
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 653 "./yaccsrc.y"
    { (yyval) = makenode(X_ADDR_WITH, 0) ;
                                         (yyval)->name = (streng *) (yyvsp[(1) - (1)]) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->u.of.ant = antSIMSYMBOL;
                                       }
    break;

  case 193:

/* Line 1806 of yacc.c  */
#line 661 "./yaccsrc.y"
    { (yyval) = makenode(X_ADDR_WITH, 0) ;
                                         (yyval)->name = (streng *) (yyvsp[(1) - (1)]) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->u.of.ant = antSTRING;
                                       }
    break;

  case 194:

/* Line 1806 of yacc.c  */
#line 669 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
    break;

  case 195:

/* Line 1806 of yacc.c  */
#line 672 "./yaccsrc.y"
    { (yyval) = makenode(X_DO_TO,1,(yyvsp[(2) - (3)])) ; }
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 673 "./yaccsrc.y"
    { (yyval) = makenode(X_DO_FOR,1,(yyvsp[(2) - (3)])) ; }
    break;

  case 197:

/* Line 1806 of yacc.c  */
#line 674 "./yaccsrc.y"
    { (yyval) = makenode(X_DO_BY,1,(yyvsp[(2) - (3)])) ; }
    break;

  case 198:

/* Line 1806 of yacc.c  */
#line 677 "./yaccsrc.y"
    { (yyval) = makenode(X_WHILE,1,(yyvsp[(2) - (3)])) ; }
    break;

  case 199:

/* Line 1806 of yacc.c  */
#line 678 "./yaccsrc.y"
    { (yyval) = makenode(X_UNTIL,1,(yyvsp[(2) - (3)])) ; }
    break;

  case 200:

/* Line 1806 of yacc.c  */
#line 679 "./yaccsrc.y"
    { (yyval) = NULL ; }
    break;

  case 201:

/* Line 1806 of yacc.c  */
#line 682 "./yaccsrc.y"
    { exiterror( ERR_SYMBOL_EXPECTED, 1, __reginatext ) ;}
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 683 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[0] = (yyvsp[(2) - (2)]) ; }
    break;

  case 203:

/* Line 1806 of yacc.c  */
#line 687 "./yaccsrc.y"
    { exiterror( ERR_SYMBOL_EXPECTED, 1, __reginatext ) ;}
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 688 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[0] = (yyvsp[(2) - (2)]) ; }
    break;

  case 205:

/* Line 1806 of yacc.c  */
#line 692 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[0] = (yyvsp[(2) - (2)]) ; }
    break;

  case 206:

/* Line 1806 of yacc.c  */
#line 697 "./yaccsrc.y"
    { move_labels( (yyvsp[(1) - (6)]), (yyvsp[(6) - (6)]), level - 1 );
                                         (yyval) = (yyvsp[(1) - (6)]);
                                         (yyval)->p[0] = optgluelast( (yyvsp[(2) - (6)]), (yyvsp[(3) - (6)]) );
                                         (yyval)->p[0]->o.last = NULL;
                                         (yyval)->p[1] = optgluelast( (yyvsp[(5) - (6)]), (yyvsp[(6) - (6)]) );
                                         (yyval)->p[1]->o.last = NULL;
                                         level--; }
    break;

  case 207:

/* Line 1806 of yacc.c  */
#line 705 "./yaccsrc.y"
    { move_labels( (yyvsp[(1) - (9)]), (yyvsp[(9) - (9)]), level - 1 );
                                         (yyval) = (yyvsp[(1) - (9)]);
                                         (yyval)->p[0] = optgluelast( (yyvsp[(2) - (9)]), (yyvsp[(3) - (9)]) );
                                         (yyval)->p[0]->o.last = NULL;
                                         (yyval)->p[1] = optgluelast( (yyvsp[(5) - (9)]), (yyvsp[(6) - (9)]) );
                                         (yyval)->p[1]->o.last = NULL;
                                         (yyval)->p[2] = optgluelast( (yyvsp[(8) - (9)]), (yyvsp[(9) - (9)]) );
                                         (yyval)->p[2]->o.last = NULL;
                                         level--; }
    break;

  case 208:

/* Line 1806 of yacc.c  */
#line 715 "./yaccsrc.y"
    {  exiterror( ERR_INCOMPLETE_STRUCT, 4 ) ;}
    break;

  case 209:

/* Line 1806 of yacc.c  */
#line 717 "./yaccsrc.y"
    {  exiterror( ERR_INCOMPLETE_STRUCT, 3 ) ;}
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 718 "./yaccsrc.y"
    {  exiterror( ERR_INCOMPLETE_STRUCT, 0 ) ;}
    break;

  case 211:

/* Line 1806 of yacc.c  */
#line 719 "./yaccsrc.y"
    {  exiterror( ERR_THEN_EXPECTED, 1, parser_data.if_linenr, __reginatext )  ; }
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 722 "./yaccsrc.y"
    {  exiterror( ERR_THEN_UNEXPECTED, 1 )  ; }
    break;

  case 213:

/* Line 1806 of yacc.c  */
#line 725 "./yaccsrc.y"
    {  exiterror( ERR_THEN_UNEXPECTED, 2 )  ; }
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 728 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[0] = (yyvsp[(2) - (2)]) ; }
    break;

  case 215:

/* Line 1806 of yacc.c  */
#line 733 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->name = (streng *) (yyvsp[(2) - (2)]) ; }
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 735 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
    break;

  case 217:

/* Line 1806 of yacc.c  */
#line 736 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ; }
    break;

  case 218:

/* Line 1806 of yacc.c  */
#line 739 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ;
                                         (yyval)->u.trace_only =
                                                         (level == 0) ? 0 : 1;
                                         newlabel( (const tsd_t *)parser_data.TSD,
                                                   &parser_data,
                                                   (yyvsp[(1) - (1)]) ) ; }
    break;

  case 219:

/* Line 1806 of yacc.c  */
#line 747 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ;
                                         (yyval)->name = Str_cre_TSD(parser_data.TSD,retvalue) ; }
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 751 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->name = (streng *) (yyvsp[(2) - (2)]) ; }
    break;

  case 221:

/* Line 1806 of yacc.c  */
#line 753 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 754 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ; }
    break;

  case 223:

/* Line 1806 of yacc.c  */
#line 757 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ; }
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 758 "./yaccsrc.y"
    {  exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
    break;

  case 225:

/* Line 1806 of yacc.c  */
#line 761 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (3)]) ;
                                         (yyval)->type = X_NUM_D ;
                                         (yyval)->p[0] = (yyvsp[(3) - (3)]) ; }
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 764 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]); (yyval)->type = X_NUM_DDEF ; }
    break;

  case 227:

/* Line 1806 of yacc.c  */
#line 765 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
    break;

  case 228:

/* Line 1806 of yacc.c  */
#line 766 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (3)]) ;
                                         (yyval)->type = X_NUM_F ;
                                         (yyval)->p[0] = (yyvsp[(3) - (3)]) ; }
    break;

  case 229:

/* Line 1806 of yacc.c  */
#line 769 "./yaccsrc.y"
    { /* NOTE! This clashes ANSI! */
                                         (yyval) = (yyvsp[(1) - (2)]) ; (yyval)->type=X_NUM_FRMDEF ;}
    break;

  case 230:

/* Line 1806 of yacc.c  */
#line 771 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (4)]) ; (yyval)->type=X_NUM_V ;
                                         (yyval)->p[0] = (yyvsp[(4) - (4)]) ; }
    break;

  case 231:

/* Line 1806 of yacc.c  */
#line 773 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 11, "ENGINEERING SCIENTIFIC", __reginatext ) ;}
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 774 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]); (yyval)->type = X_NUM_FDEF ;}
    break;

  case 233:

/* Line 1806 of yacc.c  */
#line 775 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (3)]) ;
                                         (yyval)->type = X_NUM_FUZZ ;
                                         (yyval)->p[0] = (yyvsp[(3) - (3)]) ; }
    break;

  case 234:

/* Line 1806 of yacc.c  */
#line 778 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 15, "DIGITS FORM FUZZ", __reginatext ) ;}
    break;

  case 235:

/* Line 1806 of yacc.c  */
#line 781 "./yaccsrc.y"
    { (yyval) = makenode(X_NUM_SCI,0) ; }
    break;

  case 236:

/* Line 1806 of yacc.c  */
#line 782 "./yaccsrc.y"
    { (yyval) = makenode(X_NUM_ENG,0) ; }
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 785 "./yaccsrc.y"
    { ((yyval)=(yyvsp[(1) - (2)]))->p[0]=(yyvsp[(2) - (2)]) ; }
    break;

  case 238:

/* Line 1806 of yacc.c  */
#line 789 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (4)]) ;
                                         (yyval)->type = X_PARSE ;
                                         (yyval)->u.parseflags = (int) (yyvsp[(2) - (4)]) ;
                                         (yyval)->p[0] = (yyvsp[(3) - (4)]) ;
                                         (yyval)->p[1] = (yyvsp[(4) - (4)]) ; }
    break;

  case 239:

/* Line 1806 of yacc.c  */
#line 795 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (3)]) ;
                                         (yyval)->type = X_PARSE ;
                                         (yyval)->u.parseflags = 0;
                                         (yyval)->p[0] = (yyvsp[(2) - (3)]) ;
                                         (yyval)->p[1] = (yyvsp[(3) - (3)]) ; }
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 800 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 12, "ARG EXTERNAL LINEIN PULL SOURCE VAR VALUE VERSION", __reginatext ) ;}
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 801 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 12, "ARG CASELESS EXTERNAL LINEIN LOWER PULL SOURCE UPPER VAR VALUE VERSION", __reginatext ) ;}
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 804 "./yaccsrc.y"
    { (yyval) = (nodeptr) (PARSE_UPPER  |
                                                         PARSE_NORMAL); }
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 806 "./yaccsrc.y"
    { (yyval) = (nodeptr) (PARSE_UPPER  |
                                                         PARSE_CASELESS); }
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 808 "./yaccsrc.y"
    { (yyval) = (nodeptr) (PARSE_UPPER  |
                                                         PARSE_CASELESS); }
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 810 "./yaccsrc.y"
    { (yyval) = (nodeptr) (PARSE_LOWER  |
                                                         PARSE_NORMAL); }
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 812 "./yaccsrc.y"
    { (yyval) = (nodeptr) (PARSE_LOWER  |
                                                         PARSE_CASELESS); }
    break;

  case 247:

/* Line 1806 of yacc.c  */
#line 814 "./yaccsrc.y"
    { (yyval) = (nodeptr) (PARSE_LOWER  |
                                                         PARSE_CASELESS); }
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 816 "./yaccsrc.y"
    { (yyval) = (nodeptr) (PARSE_NORMAL |
                                                         PARSE_CASELESS); }
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 820 "./yaccsrc.y"
    { /* fixes bugs like bug 579711 */
                                         (yyval) = optgluelast( (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]) ); }
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 822 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ; }
    break;

  case 251:

/* Line 1806 of yacc.c  */
#line 825 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_ARG,0) ; }
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 826 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_EXT,0) ; }
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 827 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_EXT,0) ; }
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 828 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_VER,0) ; }
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 829 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_PULL,0) ; }
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 830 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_SRC,0) ; }
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 831 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_VAR,0) ;
                                         (yyval)->name = (streng *) (yyvsp[(2) - (2)]) ; }
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 833 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_VAL,1,(yyvsp[(2) - (3)])) ; }
    break;

  case 259:

/* Line 1806 of yacc.c  */
#line 834 "./yaccsrc.y"
    { exiterror( ERR_INVALID_TEMPLATE, 3 ) ;}
    break;

  case 260:

/* Line 1806 of yacc.c  */
#line 837 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ; }
    break;

  case 261:

/* Line 1806 of yacc.c  */
#line 838 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 17, __reginatext ) ;}
    break;

  case 262:

/* Line 1806 of yacc.c  */
#line 839 "./yaccsrc.y"
    { exiterror( ERR_SYMBOL_EXPECTED, 1, __reginatext ) ;}
    break;

  case 263:

/* Line 1806 of yacc.c  */
#line 840 "./yaccsrc.y"
    { exiterror( ERR_SYMBOL_EXPECTED, 1, __reginatext ) ;}
    break;

  case 264:

/* Line 1806 of yacc.c  */
#line 841 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (3)]) ;
                                         (yyval)->p[0] = (yyvsp[(3) - (3)]) ; }
    break;

  case 265:

/* Line 1806 of yacc.c  */
#line 845 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[0] = (yyvsp[(2) - (2)]) ; }
    break;

  case 266:

/* Line 1806 of yacc.c  */
#line 849 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[0] = (yyvsp[(2) - (2)]) ; }
    break;

  case 267:

/* Line 1806 of yacc.c  */
#line 853 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[0] = (yyvsp[(2) - (2)]) ; }
    break;

  case 268:

/* Line 1806 of yacc.c  */
#line 857 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[0] = (yyvsp[(2) - (2)]) ; }
    break;

  case 269:

/* Line 1806 of yacc.c  */
#line 861 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[0] = (yyvsp[(2) - (2)]) ; }
    break;

  case 270:

/* Line 1806 of yacc.c  */
#line 865 "./yaccsrc.y"
    { exiterror( ERR_UNMATCHED_END, 0 ) ;}
    break;

  case 271:

/* Line 1806 of yacc.c  */
#line 866 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
    break;

  case 272:

/* Line 1806 of yacc.c  */
#line 867 "./yaccsrc.y"
    { level--; }
    break;

  case 273:

/* Line 1806 of yacc.c  */
#line 871 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (5)]);
                                         (yyval)->p[0] = optgluelast( (yyvsp[(2) - (5)]), (yyvsp[(3) - (5)]) );
                                         (yyval)->p[0]->o.last = NULL;
                                         (yyval)->p[1] = (yyvsp[(4) - (5)]); }
    break;

  case 274:

/* Line 1806 of yacc.c  */
#line 875 "./yaccsrc.y"
    {  exiterror( ERR_WHEN_EXPECTED, 1, parser_data.select_linenr, __reginatext ) ;}
    break;

  case 275:

/* Line 1806 of yacc.c  */
#line 877 "./yaccsrc.y"
    {  exiterror( ERR_WHEN_EXPECTED, 2, parser_data.select_linenr, __reginatext ) ;}
    break;

  case 276:

/* Line 1806 of yacc.c  */
#line 878 "./yaccsrc.y"
    {  exiterror( ERR_EXTRA_DATA, 1, __reginatext )  ;}
    break;

  case 277:

/* Line 1806 of yacc.c  */
#line 879 "./yaccsrc.y"
    {  exiterror( ERR_THEN_UNEXPECTED, 0 ) ;}
    break;

  case 278:

/* Line 1806 of yacc.c  */
#line 881 "./yaccsrc.y"
    {  exiterror( ERR_INCOMPLETE_STRUCT, 0 ) ;}
    break;

  case 279:

/* Line 1806 of yacc.c  */
#line 884 "./yaccsrc.y"
    { (yyval) = optgluelast( (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]) ); }
    break;

  case 280:

/* Line 1806 of yacc.c  */
#line 885 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ; }
    break;

  case 281:

/* Line 1806 of yacc.c  */
#line 886 "./yaccsrc.y"
    {  exiterror( ERR_WHEN_EXPECTED, 2, parser_data.select_linenr, __reginatext )  ;}
    break;

  case 282:

/* Line 1806 of yacc.c  */
#line 890 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (6)]); /* fixes bugs like bug 579711 */
                                         (yyval)->p[0] = optgluelast( (yyvsp[(2) - (6)]), (yyvsp[(3) - (6)]) );
                                         (yyval)->p[0]->o.last = NULL;
                                         (yyval)->p[1] = optgluelast( (yyvsp[(5) - (6)]), (yyvsp[(6) - (6)]) );
                                         (yyval)->p[1]->o.last = NULL; }
    break;

  case 283:

/* Line 1806 of yacc.c  */
#line 896 "./yaccsrc.y"
    {  exiterror( ERR_THEN_UNEXPECTED, 0 ) ;}
    break;

  case 284:

/* Line 1806 of yacc.c  */
#line 897 "./yaccsrc.y"
    {  exiterror( ERR_THEN_EXPECTED, 2, parser_data.select_linenr, __reginatext )  ; }
    break;

  case 285:

/* Line 1806 of yacc.c  */
#line 898 "./yaccsrc.y"
    {  exiterror( ERR_INVALID_EXPRESSION, 0 ) ;}
    break;

  case 286:

/* Line 1806 of yacc.c  */
#line 908 "./yaccsrc.y"
    { exiterror( ERR_WHEN_UNEXPECTED, 1 ); }
    break;

  case 287:

/* Line 1806 of yacc.c  */
#line 909 "./yaccsrc.y"
    { exiterror( ERR_WHEN_UNEXPECTED, 2 ); }
    break;

  case 288:

/* Line 1806 of yacc.c  */
#line 912 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (3)]);
                                         (yyval)->p[0] = optgluelast( (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]) );
                                         if ( (yyval)->p[0] )
                                            (yyval)->p[0]->o.last = NULL; }
    break;

  case 289:

/* Line 1806 of yacc.c  */
#line 916 "./yaccsrc.y"
    { (yyval) = makenode(X_NO_OTHERWISE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; }
    break;

  case 290:

/* Line 1806 of yacc.c  */
#line 922 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (3)]) ;
                                         (yyval)->type = X_SIG_VAL ;
                                         (yyval)->p[0] = (yyvsp[(3) - (3)]) ; }
    break;

  case 291:

/* Line 1806 of yacc.c  */
#line 925 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
    break;

  case 292:

/* Line 1806 of yacc.c  */
#line 926 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->name = (streng *)(yyvsp[(2) - (2)]) ; }
    break;

  case 293:

/* Line 1806 of yacc.c  */
#line 928 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 3, "ERROR FAILURE HALT NOTREADY NOVALUE SYNTAX LOSTDIGITS", __reginatext ) ;}
    break;

  case 294:

/* Line 1806 of yacc.c  */
#line 929 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 4, "ERROR FAILURE HALT NOTREADY NOVALUE SYNTAX LOSTDIGITS", __reginatext ) ;}
    break;

  case 295:

/* Line 1806 of yacc.c  */
#line 930 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
    break;

  case 296:

/* Line 1806 of yacc.c  */
#line 931 "./yaccsrc.y"
    { exiterror( ERR_STRING_EXPECTED, 3, __reginatext ) ;}
    break;

  case 297:

/* Line 1806 of yacc.c  */
#line 933 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (4)]) ;
                                         (yyval)->type = X_SIG_SET ;
                                         (yyval)->p[0] = (yyvsp[(2) - (4)]) ;
                                         (yyval)->name = (streng *)(yyvsp[(4) - (4)]) ;
                                         (yyval)->p[1] = (yyvsp[(3) - (4)]) ; }
    break;

  case 298:

/* Line 1806 of yacc.c  */
#line 938 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (3)]) ;
                                         (yyval)->type = X_SIG_SET ;
                                         (yyval)->p[0] = (yyvsp[(2) - (3)]) ;
                                         (yyval)->p[1] = (yyvsp[(3) - (3)]) ; }
    break;

  case 299:

/* Line 1806 of yacc.c  */
#line 942 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
    break;

  case 300:

/* Line 1806 of yacc.c  */
#line 943 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (3)]) ;
                                         (yyval)->type = X_SIG_SET ;
                                         (yyval)->p[0] = (yyvsp[(2) - (3)]) ;
                                         (yyval)->p[1] = (yyvsp[(3) - (3)]) ; }
    break;

  case 301:

/* Line 1806 of yacc.c  */
#line 949 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 302:

/* Line 1806 of yacc.c  */
#line 950 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
    break;

  case 303:

/* Line 1806 of yacc.c  */
#line 951 "./yaccsrc.y"
    { exiterror( ERR_STRING_EXPECTED, 4, __reginatext );}
    break;

  case 304:

/* Line 1806 of yacc.c  */
#line 954 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue);}
    break;

  case 305:

/* Line 1806 of yacc.c  */
#line 955 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
    break;

  case 306:

/* Line 1806 of yacc.c  */
#line 956 "./yaccsrc.y"
    { exiterror( ERR_STRING_EXPECTED, 3, __reginatext ) ;}
    break;

  case 307:

/* Line 1806 of yacc.c  */
#line 959 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
    break;

  case 308:

/* Line 1806 of yacc.c  */
#line 960 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
    break;

  case 309:

/* Line 1806 of yacc.c  */
#line 963 "./yaccsrc.y"
    { (yyval) = makenode(X_ON,0) ; }
    break;

  case 310:

/* Line 1806 of yacc.c  */
#line 966 "./yaccsrc.y"
    { (yyval) = makenode(X_OFF,0) ; }
    break;

  case 311:

/* Line 1806 of yacc.c  */
#line 969 "./yaccsrc.y"
    { (yyval) = makenode(X_S_ERROR,0) ; }
    break;

  case 312:

/* Line 1806 of yacc.c  */
#line 970 "./yaccsrc.y"
    { (yyval) = makenode(X_S_HALT,0) ; }
    break;

  case 313:

/* Line 1806 of yacc.c  */
#line 971 "./yaccsrc.y"
    { (yyval) = makenode(X_S_NOTREADY,0) ; }
    break;

  case 314:

/* Line 1806 of yacc.c  */
#line 972 "./yaccsrc.y"
    { (yyval) = makenode(X_S_FAILURE,0) ; }
    break;

  case 315:

/* Line 1806 of yacc.c  */
#line 975 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ; }
    break;

  case 316:

/* Line 1806 of yacc.c  */
#line 976 "./yaccsrc.y"
    { (yyval) = makenode(X_S_NOVALUE,0) ; }
    break;

  case 317:

/* Line 1806 of yacc.c  */
#line 977 "./yaccsrc.y"
    { (yyval) = makenode(X_S_SYNTAX,0) ; }
    break;

  case 318:

/* Line 1806 of yacc.c  */
#line 978 "./yaccsrc.y"
    { (yyval) = makenode(X_S_LOSTDIGITS,0) ; }
    break;

  case 319:

/* Line 1806 of yacc.c  */
#line 981 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (3)]) ;
                                         (yyval)->p[0] = (yyvsp[(3) - (3)]) ; }
    break;

  case 320:

/* Line 1806 of yacc.c  */
#line 983 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[0] = (yyvsp[(2) - (2)]) ; }
    break;

  case 321:

/* Line 1806 of yacc.c  */
#line 985 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;}
    break;

  case 322:

/* Line 1806 of yacc.c  */
#line 986 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->name = (streng *) (yyvsp[(2) - (2)]) ; }
    break;

  case 323:

/* Line 1806 of yacc.c  */
#line 990 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
    break;

  case 324:

/* Line 1806 of yacc.c  */
#line 994 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[1] = (yyvsp[(2) - (2)]) ;
                                         /*
                                          * An assignment is a numerical
                                          * assignment if and only if we have
                                          * to do a numerical operation, which
                                          * is equivalent to the existence of
                                          * one more argument to $2.
                                          * This fixes bug 720166.
                                          */
                                         if ((yyvsp[(2) - (2)]) &&
                                             (yyvsp[(2) - (2)])->p[0] &&
                                             gettypeof((yyvsp[(2) - (2)])) == IS_A_NUMBER)
                                            (yyval)->type = X_NASSIGN ; }
    break;

  case 325:

/* Line 1806 of yacc.c  */
#line 1010 "./yaccsrc.y"
    { (yyval) = makenode(X_ASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
    break;

  case 326:

/* Line 1806 of yacc.c  */
#line 1016 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[1] = (yyvsp[(2) - (2)]) ;
                                       }
    break;

  case 327:

/* Line 1806 of yacc.c  */
#line 1021 "./yaccsrc.y"
    { (yyval) = makenode(X_PLUSASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
    break;

  case 328:

/* Line 1806 of yacc.c  */
#line 1027 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[1] = (yyvsp[(2) - (2)]) ;
                                       }
    break;

  case 329:

/* Line 1806 of yacc.c  */
#line 1032 "./yaccsrc.y"
    { (yyval) = makenode(X_MINUSASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
    break;

  case 330:

/* Line 1806 of yacc.c  */
#line 1038 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[1] = (yyvsp[(2) - (2)]) ;
                                       }
    break;

  case 331:

/* Line 1806 of yacc.c  */
#line 1043 "./yaccsrc.y"
    { (yyval) = makenode(X_MULTASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
    break;

  case 332:

/* Line 1806 of yacc.c  */
#line 1049 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[1] = (yyvsp[(2) - (2)]) ;
                                       }
    break;

  case 333:

/* Line 1806 of yacc.c  */
#line 1054 "./yaccsrc.y"
    { (yyval) = makenode(X_DIVASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
    break;

  case 334:

/* Line 1806 of yacc.c  */
#line 1060 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[1] = (yyvsp[(2) - (2)]) ;
                                       }
    break;

  case 335:

/* Line 1806 of yacc.c  */
#line 1065 "./yaccsrc.y"
    { (yyval) = makenode(X_INTDIVASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
    break;

  case 336:

/* Line 1806 of yacc.c  */
#line 1071 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[1] = (yyvsp[(2) - (2)]) ;
                                       }
    break;

  case 337:

/* Line 1806 of yacc.c  */
#line 1076 "./yaccsrc.y"
    { (yyval) = makenode(X_MODULUSASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
    break;

  case 338:

/* Line 1806 of yacc.c  */
#line 1082 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[1] = (yyvsp[(2) - (2)]) ;
                                       }
    break;

  case 339:

/* Line 1806 of yacc.c  */
#line 1087 "./yaccsrc.y"
    { (yyval) = makenode(X_ANDASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
    break;

  case 340:

/* Line 1806 of yacc.c  */
#line 1093 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[1] = (yyvsp[(2) - (2)]) ;
                                       }
    break;

  case 341:

/* Line 1806 of yacc.c  */
#line 1098 "./yaccsrc.y"
    { (yyval) = makenode(X_XORASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
    break;

  case 342:

/* Line 1806 of yacc.c  */
#line 1104 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[1] = (yyvsp[(2) - (2)]) ;
                                       }
    break;

  case 343:

/* Line 1806 of yacc.c  */
#line 1109 "./yaccsrc.y"
    { (yyval) = makenode(X_ORASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
    break;

  case 344:

/* Line 1806 of yacc.c  */
#line 1115 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]) ;
                                         (yyval)->p[1] = (yyvsp[(2) - (2)]) ;
                                       }
    break;

  case 345:

/* Line 1806 of yacc.c  */
#line 1120 "./yaccsrc.y"
    { (yyval) = makenode(X_CONCATASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); }
    break;

  case 346:

/* Line 1806 of yacc.c  */
#line 1127 "./yaccsrc.y"
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
    break;

  case 347:

/* Line 1806 of yacc.c  */
#line 1143 "./yaccsrc.y"
    { parendepth++;
                                         if ( parendepth == 1 )
                                         {
                                            /* exprs on as-is basis */
                                            (yyval) = (yyvsp[(3) - (3)]);
                                         }
                                         else
                                         {
                                            /* Must already be a plain expr.
                                             * The nexpr part of exprs detects
                                             * mistakes. */
                                            (yyval) = reduce_expr_list( (yyvsp[(3) - (3)]),
                                                                REDUCE_EXPR );
                                            if ( (yyval) == (yyvsp[(3) - (3)]) )
                                               exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "Reduction of `exprs' not happened." );
                                         }
                                       }
    break;

  case 348:

/* Line 1806 of yacc.c  */
#line 1160 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 349:

/* Line 1806 of yacc.c  */
#line 1161 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "+" );
                                         (yyval) = makenode( X_PLUSS, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 350:

/* Line 1806 of yacc.c  */
#line 1163 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 351:

/* Line 1806 of yacc.c  */
#line 1164 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "-" );
                                         (yyval) = makenode( X_MINUS, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 352:

/* Line 1806 of yacc.c  */
#line 1166 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 353:

/* Line 1806 of yacc.c  */
#line 1167 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "*" );
                                         (yyval) = makenode( X_MULT, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 354:

/* Line 1806 of yacc.c  */
#line 1169 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "*" ); }
    break;

  case 355:

/* Line 1806 of yacc.c  */
#line 1170 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 356:

/* Line 1806 of yacc.c  */
#line 1171 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "/" );
                                         (yyval) = makenode( X_DEVIDE, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 357:

/* Line 1806 of yacc.c  */
#line 1173 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "/" ); }
    break;

  case 358:

/* Line 1806 of yacc.c  */
#line 1174 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 359:

/* Line 1806 of yacc.c  */
#line 1175 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "//" );
                                         (yyval) = makenode( X_MODULUS, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 360:

/* Line 1806 of yacc.c  */
#line 1177 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "//" ); }
    break;

  case 361:

/* Line 1806 of yacc.c  */
#line 1178 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 362:

/* Line 1806 of yacc.c  */
#line 1179 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "%" );
                                         (yyval) = makenode( X_INTDIV, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 363:

/* Line 1806 of yacc.c  */
#line 1181 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "%" ); }
    break;

  case 364:

/* Line 1806 of yacc.c  */
#line 1182 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 365:

/* Line 1806 of yacc.c  */
#line 1183 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "|" );
                                         (yyval) = makenode( X_LOG_OR, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 366:

/* Line 1806 of yacc.c  */
#line 1185 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "|" ); }
    break;

  case 367:

/* Line 1806 of yacc.c  */
#line 1186 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 368:

/* Line 1806 of yacc.c  */
#line 1187 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "&" );
                                         (yyval) = makenode( X_LOG_AND, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 369:

/* Line 1806 of yacc.c  */
#line 1189 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "&" ); }
    break;

  case 370:

/* Line 1806 of yacc.c  */
#line 1190 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 371:

/* Line 1806 of yacc.c  */
#line 1191 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "&&" );
                                         (yyval) = makenode( X_LOG_XOR, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 372:

/* Line 1806 of yacc.c  */
#line 1193 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "&&" ); }
    break;

  case 373:

/* Line 1806 of yacc.c  */
#line 1194 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 374:

/* Line 1806 of yacc.c  */
#line 1195 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "**" );
                                         (yyval) = makenode( X_EXP, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 375:

/* Line 1806 of yacc.c  */
#line 1197 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "**" ); }
    break;

  case 376:

/* Line 1806 of yacc.c  */
#line 1198 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 377:

/* Line 1806 of yacc.c  */
#line 1199 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), " " );
                                         (yyval) = makenode( X_SPACE, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 378:

/* Line 1806 of yacc.c  */
#line 1201 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, " " ); }
    break;

  case 379:

/* Line 1806 of yacc.c  */
#line 1202 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 380:

/* Line 1806 of yacc.c  */
#line 1203 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "||" );
                                         (yyval) = makenode( X_CONCAT, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 381:

/* Line 1806 of yacc.c  */
#line 1205 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "||" ); }
    break;

  case 382:

/* Line 1806 of yacc.c  */
#line 1206 "./yaccsrc.y"
    { AUTO_REDUCE( (yyvsp[(2) - (2)]), "\\" );
                                         (yyval) = makenode( X_LOG_NOT, 1, (yyvsp[(2) - (2)]) ); }
    break;

  case 383:

/* Line 1806 of yacc.c  */
#line 1208 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "\\" ); }
    break;

  case 384:

/* Line 1806 of yacc.c  */
#line 1209 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 385:

/* Line 1806 of yacc.c  */
#line 1210 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "=" );
                                         (yyval) = makenode( X_EQUAL, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) );
                                         transform( (yyval) ); }
    break;

  case 386:

/* Line 1806 of yacc.c  */
#line 1213 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "=" ); }
    break;

  case 387:

/* Line 1806 of yacc.c  */
#line 1214 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 388:

/* Line 1806 of yacc.c  */
#line 1215 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), ">=" );
                                         (yyval) = makenode( X_GTE, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) );
                                         transform( (yyval) ) ; }
    break;

  case 389:

/* Line 1806 of yacc.c  */
#line 1218 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, ">=" ); }
    break;

  case 390:

/* Line 1806 of yacc.c  */
#line 1219 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 391:

/* Line 1806 of yacc.c  */
#line 1220 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "<=" );
                                         (yyval) = makenode( X_LTE, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) );
                                         transform( (yyval) ) ; }
    break;

  case 392:

/* Line 1806 of yacc.c  */
#line 1223 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "<=" ); }
    break;

  case 393:

/* Line 1806 of yacc.c  */
#line 1224 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 394:

/* Line 1806 of yacc.c  */
#line 1225 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), ">" );
                                         (yyval) = makenode( X_GT, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) );
                                         transform( (yyval) ) ; }
    break;

  case 395:

/* Line 1806 of yacc.c  */
#line 1228 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, ">" ); }
    break;

  case 396:

/* Line 1806 of yacc.c  */
#line 1229 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 397:

/* Line 1806 of yacc.c  */
#line 1230 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "<" );
                                         (yyval) = makenode( X_LT, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) );
                                         transform( (yyval) ) ; }
    break;

  case 398:

/* Line 1806 of yacc.c  */
#line 1233 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "<" ); }
    break;

  case 399:

/* Line 1806 of yacc.c  */
#line 1234 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 400:

/* Line 1806 of yacc.c  */
#line 1235 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "\\=" );
                                         (yyval) = makenode( X_DIFF, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) );
                                         transform( (yyval) ) ; }
    break;

  case 401:

/* Line 1806 of yacc.c  */
#line 1238 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "\\=" ); }
    break;

  case 402:

/* Line 1806 of yacc.c  */
#line 1239 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 403:

/* Line 1806 of yacc.c  */
#line 1240 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "==" );
                                         (yyval) = makenode( X_S_EQUAL, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 404:

/* Line 1806 of yacc.c  */
#line 1242 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "==" ); }
    break;

  case 405:

/* Line 1806 of yacc.c  */
#line 1243 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 406:

/* Line 1806 of yacc.c  */
#line 1244 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "\\==" );
                                         (yyval) = makenode( X_S_DIFF, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 407:

/* Line 1806 of yacc.c  */
#line 1246 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "\\==" ); }
    break;

  case 408:

/* Line 1806 of yacc.c  */
#line 1247 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 409:

/* Line 1806 of yacc.c  */
#line 1248 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), ">>" );
                                         (yyval) = makenode( X_S_GT, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 410:

/* Line 1806 of yacc.c  */
#line 1250 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, ">>" ); }
    break;

  case 411:

/* Line 1806 of yacc.c  */
#line 1251 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 412:

/* Line 1806 of yacc.c  */
#line 1252 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "<<" );
                                         (yyval) = makenode( X_S_LT, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 413:

/* Line 1806 of yacc.c  */
#line 1254 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "<<" ); }
    break;

  case 414:

/* Line 1806 of yacc.c  */
#line 1255 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 415:

/* Line 1806 of yacc.c  */
#line 1256 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "\\>>" );
                                         (yyval) = makenode( X_S_NGT, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 416:

/* Line 1806 of yacc.c  */
#line 1258 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "\\>>" ); }
    break;

  case 417:

/* Line 1806 of yacc.c  */
#line 1259 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 418:

/* Line 1806 of yacc.c  */
#line 1260 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "\\<<" );
                                         (yyval) = makenode( X_S_NLT, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 419:

/* Line 1806 of yacc.c  */
#line 1262 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "\\<<" ); }
    break;

  case 420:

/* Line 1806 of yacc.c  */
#line 1263 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 421:

/* Line 1806 of yacc.c  */
#line 1264 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), ">>=" );
                                         (yyval) = makenode( X_S_GTE, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 422:

/* Line 1806 of yacc.c  */
#line 1266 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, ">>=" ); }
    break;

  case 423:

/* Line 1806 of yacc.c  */
#line 1267 "./yaccsrc.y"
    { parendepth--; }
    break;

  case 424:

/* Line 1806 of yacc.c  */
#line 1268 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[(1) - (4)]), "<<=" );
                                         (yyval) = makenode( X_S_LTE, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) ); }
    break;

  case 425:

/* Line 1806 of yacc.c  */
#line 1270 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "<<=" ); }
    break;

  case 426:

/* Line 1806 of yacc.c  */
#line 1271 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ; }
    break;

  case 427:

/* Line 1806 of yacc.c  */
#line 1272 "./yaccsrc.y"
    { (yyval) = makenode( X_STRING, 0 );
                                         (yyval)->name = Str_cre_TSD(parser_data.TSD,retvalue) ; }
    break;

  case 428:

/* Line 1806 of yacc.c  */
#line 1274 "./yaccsrc.y"
    { (yyval) = makenode( X_STRING, 0 );
                                         (yyval)->name = Str_make_TSD(parser_data.TSD,retlength) ;
                                         memcpy((yyval)->name->value,retvalue,
                                                    (yyval)->name->len=retlength); }
    break;

  case 429:

/* Line 1806 of yacc.c  */
#line 1278 "./yaccsrc.y"
    { (yyval) = makenode( X_STRING, 0 );
                                         (yyval)->name = Str_make_TSD(parser_data.TSD,retlength) ;
                                         memcpy((yyval)->name->value,retvalue,
                                                    (yyval)->name->len=retlength); }
    break;

  case 430:

/* Line 1806 of yacc.c  */
#line 1282 "./yaccsrc.y"
    { (yyval) = makenode( X_STRING, 0 );
                                         (yyval)->name = Str_cre_TSD(parser_data.TSD,retvalue) ; }
    break;

  case 431:

/* Line 1806 of yacc.c  */
#line 1284 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ; }
    break;

  case 432:

/* Line 1806 of yacc.c  */
#line 1285 "./yaccsrc.y"
    { AUTO_REDUCE( (yyvsp[(2) - (2)]), nullptr );
                                         (yyval) = makenode( X_U_PLUSS, 1, (yyvsp[(2) - (2)]) ); }
    break;

  case 433:

/* Line 1806 of yacc.c  */
#line 1287 "./yaccsrc.y"
    { AUTO_REDUCE( (yyvsp[(2) - (2)]), nullptr );
                                         (yyval) = makenode( X_U_MINUS, 1, (yyvsp[(2) - (2)]) ); }
    break;

  case 434:

/* Line 1806 of yacc.c  */
#line 1289 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, __reginatext ); }
    break;

  case 435:

/* Line 1806 of yacc.c  */
#line 1290 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, __reginatext ); }
    break;

  case 436:

/* Line 1806 of yacc.c  */
#line 1293 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 437:

/* Line 1806 of yacc.c  */
#line 1294 "./yaccsrc.y"
    { exiterror( ERR_UNMATCHED_PARAN, 0 ); }
    break;

  case 438:

/* Line 1806 of yacc.c  */
#line 1295 "./yaccsrc.y"
    { exiterror( ERR_UNMATCHED_PARAN, 0 ); }
    break;

  case 439:

/* Line 1806 of yacc.c  */
#line 1298 "./yaccsrc.y"
    { (yyval) = (nodeptr)create_head( (const char *)retvalue ) ; }
    break;

  case 440:

/* Line 1806 of yacc.c  */
#line 1302 "./yaccsrc.y"
    { (yyval) = makenode(X_EX_FUNC,1,(yyvsp[(2) - (2)])) ;
                                         (yyval)->name = (streng *)(yyvsp[(1) - (2)]) ; }
    break;

  case 441:

/* Line 1806 of yacc.c  */
#line 1304 "./yaccsrc.y"
    { (yyval) = makenode(X_IN_FUNC,1,(yyvsp[(2) - (2)])) ;
                                         (yyval)->name = (streng *)(yyvsp[(1) - (2)]) ; }
    break;

  case 442:

/* Line 1806 of yacc.c  */
#line 1308 "./yaccsrc.y"
    { /* ugly fake preservs parendepth */
                                         (yyval) = (YYSTYPE) parendepth;
                                         parendepth = 0; }
    break;

  case 443:

/* Line 1806 of yacc.c  */
#line 1311 "./yaccsrc.y"
    { parendepth = (int) (yyval);
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 444:

/* Line 1806 of yacc.c  */
#line 1315 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
    break;

  case 445:

/* Line 1806 of yacc.c  */
#line 1318 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
    break;

  case 446:

/* Line 1806 of yacc.c  */
#line 1321 "./yaccsrc.y"
    { (yyval) =makenode(X_TPL_SOLID,3,(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 447:

/* Line 1806 of yacc.c  */
#line 1322 "./yaccsrc.y"
    { (yyval) =makenode(X_TPL_SOLID,1,(yyvsp[(1) - (1)])) ; }
    break;

  case 448:

/* Line 1806 of yacc.c  */
#line 1323 "./yaccsrc.y"
    { exiterror( ERR_INVALID_TEMPLATE, 1, __reginatext ) ;}
    break;

  case 449:

/* Line 1806 of yacc.c  */
#line 1326 "./yaccsrc.y"
    { (yyval) = makenode(X_NEG_OFFS,0) ;
                                         (yyval)->name = (streng *) (yyvsp[(2) - (2)]) ; }
    break;

  case 450:

/* Line 1806 of yacc.c  */
#line 1328 "./yaccsrc.y"
    { (yyval) = makenode(X_POS_OFFS,0) ;
                                         (yyval)->name = (streng *) (yyvsp[(2) - (2)]) ; }
    break;

  case 451:

/* Line 1806 of yacc.c  */
#line 1330 "./yaccsrc.y"
    { (yyval) = makenode(X_ABS_OFFS,0) ;
                                         (yyval)->name = (streng *) (yyvsp[(1) - (1)]) ; }
    break;

  case 452:

/* Line 1806 of yacc.c  */
#line 1332 "./yaccsrc.y"
    { (yyval) = makenode(X_ABS_OFFS,0) ;
                                         (yyval)->name = (streng *) (yyvsp[(2) - (2)]) ; }
    break;

  case 453:

/* Line 1806 of yacc.c  */
#line 1334 "./yaccsrc.y"
    { (yyval) = makenode(X_TPL_VAR,0) ;
                                         (yyval)->p[0] = (yyvsp[(2) - (3)]) ; }
    break;

  case 454:

/* Line 1806 of yacc.c  */
#line 1336 "./yaccsrc.y"
    { (yyval) = makenode(X_NEG_OFFS,0) ;
                                         (yyval)->p[0] = (yyvsp[(3) - (4)]) ; }
    break;

  case 455:

/* Line 1806 of yacc.c  */
#line 1338 "./yaccsrc.y"
    { (yyval) = makenode(X_POS_OFFS,0) ;
                                         (yyval)->p[0] = (yyvsp[(3) - (4)]) ; }
    break;

  case 456:

/* Line 1806 of yacc.c  */
#line 1340 "./yaccsrc.y"
    { (yyval) = makenode(X_ABS_OFFS,0) ;
                                         (yyval)->p[0] = (yyvsp[(3) - (4)]) ; }
    break;

  case 457:

/* Line 1806 of yacc.c  */
#line 1342 "./yaccsrc.y"
    { (yyval) = makenode(X_TPL_MVE,0) ;
                                         (yyval)->name = (streng *) (yyvsp[(1) - (1)]) ; }
    break;

  case 458:

/* Line 1806 of yacc.c  */
#line 1346 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; }
    break;

  case 459:

/* Line 1806 of yacc.c  */
#line 1347 "./yaccsrc.y"
    { streng *sptr = Str_cre_TSD(parser_data.TSD,retvalue) ;
                                          if (myisnumber(parser_data.TSD, sptr))
                                          {
                                             exiterror( ERR_INVALID_INTEGER, 4, sptr->value ) ;
                                          }
                                          else
                                             exiterror( ERR_INVALID_TEMPLATE, 0 ) ;}
    break;

  case 460:

/* Line 1806 of yacc.c  */
#line 1356 "./yaccsrc.y"
    { (yyval) = (nodeptr) Str_cre_TSD(parser_data.TSD,retvalue) ; }
    break;

  case 461:

/* Line 1806 of yacc.c  */
#line 1357 "./yaccsrc.y"
    { streng *sptr = Str_make_TSD(parser_data.TSD,retlength) ;
                                         memcpy(sptr->value,retvalue,
                                                 sptr->len=retlength) ;
                                         (yyval) = (nodeptr) sptr ; }
    break;

  case 462:

/* Line 1806 of yacc.c  */
#line 1361 "./yaccsrc.y"
    { streng *sptr = Str_make_TSD(parser_data.TSD,retlength) ;
                                         memcpy(sptr->value,retvalue,
                                                 sptr->len=retlength) ;
                                         (yyval) = (nodeptr) sptr ; }
    break;

  case 463:

/* Line 1806 of yacc.c  */
#line 1367 "./yaccsrc.y"
    { (yyval) = makenode(X_TPL_POINT,1,(yyvsp[(2) - (2)])) ; }
    break;

  case 464:

/* Line 1806 of yacc.c  */
#line 1368 "./yaccsrc.y"
    { (yyval) = makenode(X_TPL_SYMBOL,2,(yyvsp[(2) - (2)]),(yyvsp[(1) - (2)])) ; }
    break;

  case 465:

/* Line 1806 of yacc.c  */
#line 1369 "./yaccsrc.y"
    { (yyval) = NULL ; }
    break;

  case 466:

/* Line 1806 of yacc.c  */
#line 1379 "./yaccsrc.y"
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
                                         (yyvsp[(1) - (2)]) = reduce_expr_list( (yyvsp[(1) - (2)]),
                                                                REDUCE_EXPR );

                                         /* detect "call s (a,b),. b" and
                                          * "call s (),. b", but every list on
                                          * the left side of "," is wrong, so
                                          * complain about every exprlist.   */
                                         if ( IS_EXPRLIST( (yyvsp[(1) - (2)]) ) )
                                            exiterror( ERR_UNEXPECTED_PARAN, 1 );

                                         (yyvsp[(1) - (2)]) = reduce_expr_list( (yyvsp[(1) - (2)]),
                                                              REDUCE_SUBEXPR );
                                       }
    break;

  case 467:

/* Line 1806 of yacc.c  */
#line 1422 "./yaccsrc.y"
    { /*
                                          * Fixes bug 961301.
                                          */
                                         nodeptr curr;

                                         assert( IS_EXPRLIST( (yyvsp[(4) - (4)]) ) );

                                         /* detect ",()." */
                                         if ( IS_EXPRLIST( (yyvsp[(4) - (4)])->p[0] )
                                           && ( (yyvsp[(4) - (4)])->p[1] == NULL )
                                           && ( (yyvsp[(4) - (4)])->p[0]->p[0] == NULL ) )
                                            exiterror( ERR_UNEXPECTED_PARAN, 0 );

                                         /* detect ",(a,b)." */
                                         if ( IS_EXPRLIST( (yyvsp[(4) - (4)])->p[0] )
                                           && ( (yyvsp[(4) - (4)])->p[1] == NULL )
                                           && IS_EXPRLIST( (yyvsp[(4) - (4)])->p[0]->p[1] ) )
                                            exiterror( ERR_UNEXPECTED_PARAN, 1 );

                                         /* detect ",(a)." and transform it
                                          * to ",a."                         */
                                         (yyvsp[(4) - (4)]) = reduce_expr_list( (yyvsp[(4) - (4)]),
                                                                REDUCE_RIGHT );
                                         assert( IS_EXPRLIST( (yyvsp[(4) - (4)]) ) );

                                         /* Detect something like
                                          * "call s (a,b)+1"                 */
                                         curr = (yyvsp[(4) - (4)])->p[0];
                                         if ( ( curr != NULL )
                                           && !IS_EXPRLIST( curr )
                                           && !IS_FUNCTION( curr )
                                           && ( IS_EXPRLIST( curr->p[0] )
                                             || IS_EXPRLIST( curr->p[1] ) ) )
                                            exiterror( ERR_INVALID_EXPRESSION, 0 );

                                         (yyval) = makenode( X_EXPRLIST, 2, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]) );
                                         checkconst( (yyval) ); }
    break;

  case 468:

/* Line 1806 of yacc.c  */
#line 1459 "./yaccsrc.y"
    { /* detect
                                          * "x = ()."
                                          * "x = f(().)"
                                          * "call s (().)"
                                          * "CALL s ()."                     */
                                         if ( ( parendepth < 0 ) && ( (yyvsp[(1) - (1)]) == NULL ) )
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

                                         (yyvsp[(1) - (1)]) = reduce_expr_list( (yyvsp[(1) - (1)]),
                                                              REDUCE_SUBEXPR );
                                         (yyval) = makenode( X_EXPRLIST, 1, (yyvsp[(1) - (1)]) );
                                         checkconst( (yyval) );
                                        }
    break;

  case 469:

/* Line 1806 of yacc.c  */
#line 1503 "./yaccsrc.y"
    { (yyval) = (yyvsp[(1) - (1)]) ; }
    break;

  case 470:

/* Line 1806 of yacc.c  */
#line 1504 "./yaccsrc.y"
    { (yyval) = NULL ; }
    break;

  case 471:

/* Line 1806 of yacc.c  */
#line 1507 "./yaccsrc.y"
    { (yyval) = makenode(X_SIM_SYMBOL,1,(yyvsp[(2) - (2)])) ;
                                         (yyval)->name = (streng *) (yyvsp[(1) - (2)]) ; }
    break;

  case 472:

/* Line 1806 of yacc.c  */
#line 1509 "./yaccsrc.y"
    { (yyval) = makenode(X_SIM_SYMBOL,0) ;
                                         (yyval)->name = (streng *) (yyvsp[(1) - (1)]) ; }
    break;

  case 473:

/* Line 1806 of yacc.c  */
#line 1511 "./yaccsrc.y"
    { (yyval) = makenode(X_IND_SYMBOL,1,(yyvsp[(4) - (4)])) ;
                                         (yyval)->name = (streng *) (yyvsp[(2) - (4)]) ; }
    break;

  case 474:

/* Line 1806 of yacc.c  */
#line 1513 "./yaccsrc.y"
    { (yyval) = makenode(X_IND_SYMBOL,0) ;
                                         (yyval)->name = (streng *) (yyvsp[(2) - (3)]) ; }
    break;

  case 475:

/* Line 1806 of yacc.c  */
#line 1517 "./yaccsrc.y"
    { (yyval) = (treenode *) Str_cre_TSD(parser_data.TSD,retvalue);}
    break;

  case 476:

/* Line 1806 of yacc.c  */
#line 1520 "./yaccsrc.y"
    { (yyval) = (treenode *) Str_cre_TSD(parser_data.TSD,retvalue);}
    break;

  case 477:

/* Line 1806 of yacc.c  */
#line 1521 "./yaccsrc.y"
    { exiterror( ERR_SYMBOL_EXPECTED, 1, __reginatext ) ;}
    break;



/* Line 1806 of yacc.c  */
#line 6080 "./yaccsrc.c"
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

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
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

  /* Do not reclaim the symbols of the rule which action triggered
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
  yyerrstatus = 3;   /* Each real token shifted decrements this.  */

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

  *++yyvsp = yylval;


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

#if !defined(yyoverflow) || YYERROR_VERBOSE
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
  /* Do not reclaim the symbols of the rule which action triggered
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
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 1524 "./yaccsrc.y"


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

