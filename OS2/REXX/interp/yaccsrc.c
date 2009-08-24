/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse __reginaparse
#define yylex   __reginalex
#define yyerror __reginaerror
#define yylval  __reginalval
#define yychar  __reginachar
#define yydebug __reginadebug
#define yynerrs __reginanerrs


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
     ERROR = 296,
     SYNTAX = 297,
     HALT = 298,
     NOVALUE = 299,
     TRACE = 300,
     END = 301,
     UPPER = 302,
     ASSIGNMENTVARIABLE = 303,
     STATSEP = 304,
     FOREVER = 305,
     DIGITS = 306,
     FORM = 307,
     FUZZ = 308,
     SCIENTIFIC = 309,
     ENGINEERING = 310,
     NOT = 311,
     CONCATENATE = 312,
     MODULUS = 313,
     GTE = 314,
     GT = 315,
     LTE = 316,
     LT = 317,
     DIFFERENT = 318,
     EQUALEQUAL = 319,
     NOTEQUALEQUAL = 320,
     OFFSET = 321,
     SPACE = 322,
     EXP = 323,
     XOR = 324,
     PLACEHOLDER = 325,
     NOTREADY = 326,
     CONSYMBOL = 327,
     SIMSYMBOL = 328,
     EXFUNCNAME = 329,
     INFUNCNAME = 330,
     LABEL = 331,
     DOVARIABLE = 332,
     HEXSTRING = 333,
     STRING = 334,
     VERSION = 335,
     LINEIN = 336,
     WHATEVER = 337,
     NAME = 338,
     FAILURE = 339,
     BINSTRING = 340,
     OPTIONS = 341,
     ENVIRONMENT = 342,
     LOSTDIGITS = 343,
     GTGT = 344,
     LTLT = 345,
     NOTGTGT = 346,
     NOTLTLT = 347,
     GTGTE = 348,
     LTLTE = 349,
     INPUT = 350,
     OUTPUT = 351,
     NORMAL = 352,
     APPEND = 353,
     REPLACE = 354,
     STREAM = 355,
     STEM = 356,
     LIFO = 357,
     FIFO = 358,
     LOWER = 359,
     CASELESS = 360,
     CCAT = 361,
     UPLUS = 362,
     UMINUS = 363
   };
#endif
#define ADDRESS 258
#define ARG 259
#define CALL 260
#define DO 261
#define TO 262
#define BY 263
#define FOR 264
#define WHILE 265
#define UNTIL 266
#define EXIT 267
#define IF 268
#define THEN 269
#define ELSE 270
#define ITERATE 271
#define INTERPRET 272
#define LEAVE 273
#define NOP 274
#define NUMERIC 275
#define PARSE 276
#define EXTERNAL 277
#define SOURCE 278
#define VAR 279
#define VALUE 280
#define WITH 281
#define PROCEDURE 282
#define EXPOSE 283
#define PULL 284
#define PUSH 285
#define QUEUE 286
#define SAY 287
#define RETURN 288
#define SELECT 289
#define WHEN 290
#define DROP 291
#define OTHERWISE 292
#define SIGNAL 293
#define ON 294
#define OFF 295
#define ERROR 296
#define SYNTAX 297
#define HALT 298
#define NOVALUE 299
#define TRACE 300
#define END 301
#define UPPER 302
#define ASSIGNMENTVARIABLE 303
#define STATSEP 304
#define FOREVER 305
#define DIGITS 306
#define FORM 307
#define FUZZ 308
#define SCIENTIFIC 309
#define ENGINEERING 310
#define NOT 311
#define CONCATENATE 312
#define MODULUS 313
#define GTE 314
#define GT 315
#define LTE 316
#define LT 317
#define DIFFERENT 318
#define EQUALEQUAL 319
#define NOTEQUALEQUAL 320
#define OFFSET 321
#define SPACE 322
#define EXP 323
#define XOR 324
#define PLACEHOLDER 325
#define NOTREADY 326
#define CONSYMBOL 327
#define SIMSYMBOL 328
#define EXFUNCNAME 329
#define INFUNCNAME 330
#define LABEL 331
#define DOVARIABLE 332
#define HEXSTRING 333
#define STRING 334
#define VERSION 335
#define LINEIN 336
#define WHATEVER 337
#define NAME 338
#define FAILURE 339
#define BINSTRING 340
#define OPTIONS 341
#define ENVIRONMENT 342
#define LOSTDIGITS 343
#define GTGT 344
#define LTLT 345
#define NOTGTGT 346
#define NOTLTLT 347
#define GTGTE 348
#define LTLTE 349
#define INPUT 350
#define OUTPUT 351
#define NORMAL 352
#define APPEND 353
#define REPLACE 354
#define STREAM 355
#define STEM 356
#define LIFO 357
#define FIFO 358
#define LOWER 359
#define CASELESS 360
#define CCAT 361
#define UPLUS 362
#define UMINUS 363




/* Copy the first part of user declarations.  */
#line 1 "./yaccsrc.y"


#ifndef lint
static char *RCSid = "$Id: yaccsrc.c,v 1.40 2006/09/15 05:41:00 mark Exp $";
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
#line 149 "./yaccsrc.y"

#ifdef NDEBUG
# define YYDEBUG 0
#else
# define YYDEBUG 1
#endif


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

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 213 of yacc.c.  */
#line 436 "./yaccsrc.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2209

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  120
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  177
/* YYNRULES -- Number of rules. */
#define YYNRULES  447
/* YYNRULES -- Number of states. */
#define YYNSTATES  611

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   363

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   114,   107,     2,
     117,   118,   112,   110,   119,   111,     2,   113,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   108,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,   106,     2,     2,     2,     2,     2,
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
     105,   109,   115,   116
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short int yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    12,    15,    17,    20,
      23,    25,    27,    31,    33,    35,    39,    42,    45,    47,
      50,    51,    53,    55,    56,    59,    62,    64,    66,    68,
      70,    72,    74,    76,    78,    80,    82,    84,    86,    88,
      90,    92,    94,    96,    98,   100,   102,   104,   106,   108,
     110,   112,   114,   116,   118,   120,   122,   124,   126,   128,
     130,   132,   134,   136,   138,   140,   142,   144,   146,   148,
     150,   152,   154,   156,   158,   160,   162,   164,   166,   168,
     170,   172,   173,   177,   181,   183,   184,   185,   189,   193,
     199,   202,   203,   208,   209,   214,   218,   222,   227,   233,
     238,   242,   247,   251,   253,   255,   257,   260,   262,   264,
     267,   269,   273,   280,   288,   295,   301,   306,   309,   312,
     315,   316,   318,   320,   322,   323,   326,   327,   330,   334,
     337,   339,   341,   343,   345,   348,   351,   354,   357,   361,
     364,   368,   370,   373,   377,   380,   384,   386,   389,   393,
     396,   400,   402,   407,   411,   416,   420,   425,   429,   431,
     433,   435,   438,   441,   444,   447,   449,   452,   455,   458,
     461,   464,   467,   470,   473,   474,   478,   479,   483,   485,
     486,   488,   490,   492,   494,   496,   500,   504,   508,   512,
     516,   517,   521,   524,   528,   531,   534,   541,   551,   561,
     568,   571,   576,   579,   582,   585,   588,   592,   594,   596,
     598,   601,   605,   607,   609,   612,   616,   619,   624,   628,
     631,   636,   640,   643,   647,   650,   652,   654,   657,   662,
     666,   670,   673,   675,   678,   681,   683,   686,   689,   691,
     695,   697,   699,   701,   703,   705,   707,   709,   712,   716,
     719,   721,   724,   728,   733,   737,   740,   743,   746,   749,
     752,   755,   759,   761,   767,   772,   777,   780,   784,   790,
     793,   795,   797,   804,   812,   815,   818,   820,   822,   825,
     829,   830,   834,   838,   841,   845,   849,   854,   860,   865,
     869,   874,   878,   880,   882,   884,   887,   890,   892,   894,
     896,   898,   900,   902,   904,   906,   908,   910,   912,   914,
     918,   921,   925,   928,   930,   933,   935,   936,   940,   941,
     946,   947,   952,   953,   958,   960,   961,   966,   968,   969,
     974,   976,   977,   982,   984,   985,   990,   992,   993,   998,
    1000,  1001,  1006,  1008,  1009,  1014,  1016,  1017,  1022,  1024,
    1025,  1030,  1032,  1035,  1037,  1038,  1043,  1045,  1046,  1051,
    1053,  1054,  1059,  1061,  1062,  1067,  1069,  1070,  1075,  1077,
    1078,  1083,  1085,  1086,  1091,  1093,  1094,  1099,  1101,  1102,
    1107,  1109,  1110,  1115,  1117,  1118,  1123,  1125,  1126,  1131,
    1133,  1134,  1139,  1141,  1142,  1147,  1149,  1151,  1153,  1155,
    1157,  1159,  1161,  1164,  1167,  1170,  1173,  1176,  1179,  1181,
    1183,  1186,  1189,  1190,  1193,  1195,  1197,  1201,  1203,  1205,
    1208,  1211,  1213,  1216,  1220,  1225,  1230,  1235,  1237,  1239,
    1241,  1243,  1245,  1247,  1250,  1253,  1254,  1255,  1260,  1262,
    1264,  1265,  1268,  1270,  1275,  1279,  1281,  1283
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short int yyrhs[] =
{
     121,     0,    -1,    -1,   122,   123,    -1,   130,   124,    -1,
     130,    -1,   124,   126,    -1,   126,    -1,   125,   132,    -1,
     132,   133,    -1,   132,    -1,   127,    -1,   133,   176,   129,
      -1,   125,    -1,   133,    -1,   129,    49,   131,    -1,    49,
     131,    -1,   131,   129,    -1,   131,    -1,   131,   215,    -1,
      -1,   134,    -1,   237,    -1,    -1,   133,   135,    -1,   136,
     129,    -1,   210,    -1,   211,    -1,   212,    -1,   164,    -1,
     174,    -1,   168,    -1,   169,    -1,   177,    -1,   207,    -1,
     209,    -1,   213,    -1,   214,    -1,   217,    -1,   218,    -1,
     219,    -1,   221,    -1,   222,    -1,   226,    -1,   227,    -1,
     228,    -1,   229,    -1,   231,    -1,   230,    -1,   233,    -1,
     239,    -1,   247,    -1,   208,    -1,   249,    -1,     5,    -1,
       6,    -1,    12,    -1,    13,    -1,    16,    -1,    18,    -1,
      32,    -1,    33,    -1,     3,    -1,     4,    -1,    36,    -1,
      17,    -1,    76,    -1,    19,    -1,    20,    -1,    86,    -1,
      21,    -1,    27,    -1,    29,    -1,    30,    -1,    31,    -1,
      34,    -1,    38,    -1,    35,    -1,    37,    -1,    45,    -1,
      47,    -1,    -1,   145,   165,   166,    -1,    25,   251,   180,
      -1,   182,    -1,    -1,    -1,     1,   167,   180,    -1,   179,
     293,   180,    -1,   117,   251,   118,   200,   180,    -1,   146,
     224,    -1,    -1,   137,   172,   170,   173,    -1,    -1,   137,
     289,   171,   173,    -1,   137,   243,     1,    -1,   137,   244,
       1,    -1,   137,   243,   245,     1,    -1,   137,   243,   245,
     241,     1,    -1,   137,   243,   245,   241,    -1,   137,   243,
     245,    -1,   137,   244,   245,     1,    -1,   137,   244,   245,
      -1,   242,    -1,     1,    -1,   291,    -1,   291,   118,    -1,
     251,    -1,    46,    -1,   175,   296,    -1,   175,    -1,   175,
     296,     1,    -1,   138,   178,   206,   129,   128,   176,    -1,
     204,   108,   251,   200,   205,   205,   205,    -1,   204,   108,
     251,   200,   205,   205,    -1,   204,   108,   251,   200,   205,
      -1,   204,   108,   251,   200,    -1,    50,   200,    -1,    50,
       1,    -1,   251,   200,    -1,    -1,    72,    -1,    73,    -1,
      79,    -1,    -1,   181,   182,    -1,    -1,    26,   183,    -1,
      26,   183,     1,    -1,    26,   200,    -1,   184,    -1,   185,
      -1,   186,    -1,     1,    -1,   190,   187,    -1,   191,   188,
      -1,   192,   189,    -1,   191,   200,    -1,   191,   192,   200,
      -1,   192,   200,    -1,   192,   191,   200,    -1,   200,    -1,
     190,   200,    -1,   190,   192,   200,    -1,   192,   200,    -1,
     192,   190,   200,    -1,   200,    -1,   190,   200,    -1,   190,
     191,   200,    -1,   191,   200,    -1,   191,   190,   200,    -1,
     200,    -1,   200,    95,   200,   193,    -1,   200,    95,     1,
      -1,   200,    96,   200,   194,    -1,   200,    96,     1,    -1,
     200,    41,   200,   194,    -1,   200,    41,     1,    -1,   195,
      -1,    97,    -1,   195,    -1,    98,   195,    -1,    98,     1,
      -1,    99,   195,    -1,    99,     1,    -1,    97,    -1,   100,
     198,    -1,   100,     1,    -1,   101,   196,    -1,   101,     1,
      -1,   102,   198,    -1,   102,     1,    -1,   103,   198,    -1,
     103,     1,    -1,    -1,   197,   200,   202,    -1,    -1,   199,
     200,   201,    -1,    67,    -1,    -1,   202,    -1,   203,    -1,
     295,    -1,   289,    -1,    77,    -1,     7,   251,   200,    -1,
       9,   251,   200,    -1,     8,   251,   200,    -1,    10,   251,
     200,    -1,    11,   251,   200,    -1,    -1,   147,   294,     1,
      -1,   147,   294,    -1,   163,   294,     1,    -1,   163,   294,
      -1,   139,   293,    -1,   140,   251,   130,    14,   130,   126,
      -1,   140,   251,   130,    14,   130,   126,    15,   130,   126,
      -1,   140,   251,   130,    14,   130,   126,    15,   130,     1,
      -1,   140,   251,   130,    14,   130,     1,    -1,   140,   129,
      -1,   140,   251,   130,     1,    -1,   133,    14,    -1,   133,
      15,    -1,   148,   251,    -1,   141,   296,    -1,   141,   296,
       1,    -1,   141,    -1,   216,    -1,   149,    -1,   142,   296,
      -1,   142,   296,     1,    -1,   142,    -1,   150,    -1,   150,
       1,    -1,   151,    51,   251,    -1,   151,    51,    -1,   151,
      52,   220,     1,    -1,   151,    52,   220,    -1,   151,    52,
      -1,   151,    52,    25,   251,    -1,   151,    52,     1,    -1,
     151,    53,    -1,   151,    53,   251,    -1,   151,     1,    -1,
      54,    -1,    55,    -1,   152,   293,    -1,   153,   223,   225,
     224,    -1,   153,   225,   224,    -1,   153,   223,     1,    -1,
     153,     1,    -1,    47,    -1,    47,   105,    -1,   105,    47,
      -1,   104,    -1,   104,   105,    -1,   105,   104,    -1,   105,
      -1,   224,   119,   286,    -1,   286,    -1,     4,    -1,    81,
      -1,    22,    -1,    80,    -1,    29,    -1,    23,    -1,    24,
     296,    -1,    25,   293,    26,    -1,    25,     1,    -1,   154,
      -1,   154,     1,    -1,   154,    28,     1,    -1,   154,    28,
     294,     1,    -1,   154,    28,   294,    -1,   155,   286,    -1,
     156,   293,    -1,   157,   293,    -1,   143,   293,    -1,   144,
     293,    -1,    46,   296,    -1,    46,   296,     1,    -1,    46,
      -1,   158,   129,   234,   238,   232,    -1,   158,   129,    46,
       1,    -1,   158,   129,   161,     1,    -1,   158,     1,    -1,
     158,   129,    14,    -1,   158,   129,   234,   161,     1,    -1,
     234,   235,    -1,   235,    -1,     1,    -1,   160,   251,   130,
      14,   130,   132,    -1,   160,   251,   130,    14,   130,   132,
      14,    -1,   160,   251,    -1,   160,     1,    -1,   160,    -1,
     161,    -1,   133,   236,    -1,   161,   130,   128,    -1,    -1,
     159,    25,   251,    -1,   159,   240,     1,    -1,   159,   240,
      -1,   159,   243,     1,    -1,   159,   244,     1,    -1,   159,
     243,   246,     1,    -1,   159,   243,   246,   241,     1,    -1,
     159,   243,   246,   241,    -1,   159,   243,   246,    -1,   159,
     244,   246,     1,    -1,   159,   244,   246,    -1,   242,    -1,
      79,    -1,     1,    -1,    83,    73,    -1,    83,     1,    -1,
      72,    -1,    73,    -1,    39,    -1,    40,    -1,    41,    -1,
      43,    -1,    71,    -1,    84,    -1,   245,    -1,    44,    -1,
      42,    -1,    88,    -1,   162,    25,   251,    -1,   162,   251,
      -1,   162,   248,     1,    -1,   162,   248,    -1,    82,    -1,
     250,   293,    -1,    48,    -1,    -1,   117,   252,   279,    -1,
      -1,   251,   110,   253,   251,    -1,    -1,   251,   111,   254,
     251,    -1,    -1,   251,   112,   255,   251,    -1,   112,    -1,
      -1,   251,   113,   256,   251,    -1,   113,    -1,    -1,   251,
      58,   257,   251,    -1,    58,    -1,    -1,   251,   114,   258,
     251,    -1,   114,    -1,    -1,   251,   106,   259,   251,    -1,
     106,    -1,    -1,   251,   107,   260,   251,    -1,   107,    -1,
      -1,   251,    69,   261,   251,    -1,    69,    -1,    -1,   251,
      68,   262,   251,    -1,    68,    -1,    -1,   251,    67,   263,
     251,    -1,    67,    -1,    -1,   251,    57,   264,   251,    -1,
      57,    -1,    56,   251,    -1,    56,    -1,    -1,   251,   108,
     265,   251,    -1,   108,    -1,    -1,   251,    59,   266,   251,
      -1,    59,    -1,    -1,   251,    61,   267,   251,    -1,    61,
      -1,    -1,   251,    60,   268,   251,    -1,    60,    -1,    -1,
     251,    62,   269,   251,    -1,    62,    -1,    -1,   251,    63,
     270,   251,    -1,    63,    -1,    -1,   251,    64,   271,   251,
      -1,    64,    -1,    -1,   251,    65,   272,   251,    -1,    65,
      -1,    -1,   251,    89,   273,   251,    -1,    89,    -1,    -1,
     251,    90,   274,   251,    -1,    90,    -1,    -1,   251,    91,
     275,   251,    -1,    91,    -1,    -1,   251,    92,   276,   251,
      -1,    92,    -1,    -1,   251,    93,   277,   251,    -1,    93,
      -1,    -1,   251,    94,   278,   251,    -1,    94,    -1,   280,
      -1,    72,    -1,    78,    -1,    85,    -1,    79,    -1,   281,
      -1,   110,   251,    -1,   111,   251,    -1,   110,     1,    -1,
     111,     1,    -1,   291,   118,    -1,   291,     1,    -1,    49,
      -1,    73,    -1,   285,   282,    -1,   284,   282,    -1,    -1,
     283,   279,    -1,    75,    -1,    74,    -1,   290,   287,   286,
      -1,   290,    -1,     1,    -1,   111,   288,    -1,   110,   288,
      -1,   288,    -1,   108,   288,    -1,   117,   280,   118,    -1,
     111,   117,   280,   118,    -1,   110,   117,   280,   118,    -1,
     108,   117,   280,   118,    -1,   289,    -1,    66,    -1,    72,
      -1,    79,    -1,    78,    -1,    85,    -1,    70,   290,    -1,
     280,   290,    -1,    -1,    -1,   293,   119,   292,   291,    -1,
     293,    -1,   251,    -1,    -1,   295,   294,    -1,   295,    -1,
     117,   295,   118,   294,    -1,   117,   295,   118,    -1,    73,
      -1,    73,    -1,     1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   159,   159,   159,   169,   172,   178,   180,   183,   185,
     188,   189,   192,   197,   198,   201,   202,   205,   206,   209,
     210,   213,   214,   217,   221,   224,   225,   226,   227,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   257,   261,   266,   270,   275,   279,
     283,   287,   291,   295,   299,   303,   307,   311,   315,   319,
     323,   327,   331,   335,   339,   343,   348,   352,   356,   360,
     364,   368,   368,   372,   375,   376,   377,   377,   379,   383,
     389,   395,   395,   400,   400,   406,   407,   408,   409,   410,
     416,   421,   422,   428,   429,   432,   440,   443,   449,   455,
     457,   458,   461,   482,   486,   490,   494,   497,   498,   499,
     501,   504,   505,   506,   509,   509,   515,   518,   519,   520,
     523,   524,   525,   526,   529,   533,   537,   541,   542,   543,
     544,   545,   548,   549,   550,   551,   552,   555,   556,   557,
     558,   559,   562,   563,   566,   567,   570,   571,   574,   575,
     580,   581,   583,   584,   585,   586,   591,   599,   600,   613,
     614,   618,   619,   623,   626,   626,   630,   630,   634,   635,
     638,   639,   642,   650,   658,   661,   662,   663,   666,   667,
     668,   671,   672,   676,   677,   681,   685,   693,   703,   705,
     707,   708,   711,   714,   717,   722,   724,   725,   728,   736,
     740,   742,   743,   746,   747,   750,   753,   754,   755,   758,
     760,   762,   763,   764,   767,   770,   771,   774,   777,   783,
     789,   790,   793,   795,   797,   799,   801,   803,   805,   809,
     811,   814,   815,   816,   817,   818,   819,   820,   822,   823,
     826,   827,   828,   829,   830,   834,   838,   842,   846,   850,
     854,   855,   856,   859,   864,   865,   867,   868,   869,   873,
     874,   875,   878,   884,   886,   887,   890,   891,   894,   897,
     901,   907,   910,   911,   913,   914,   915,   916,   917,   923,
     927,   928,   934,   935,   936,   939,   940,   943,   944,   947,
     950,   953,   954,   955,   956,   959,   960,   961,   962,   965,
     967,   969,   970,   974,   978,   994,  1001,  1001,  1034,  1034,
    1037,  1037,  1040,  1040,  1043,  1044,  1044,  1047,  1048,  1048,
    1051,  1052,  1052,  1055,  1056,  1056,  1059,  1060,  1060,  1063,
    1064,  1064,  1067,  1068,  1068,  1071,  1072,  1072,  1075,  1076,
    1076,  1079,  1080,  1082,  1083,  1083,  1087,  1088,  1088,  1092,
    1093,  1093,  1097,  1098,  1098,  1102,  1103,  1103,  1107,  1108,
    1108,  1112,  1113,  1113,  1116,  1117,  1117,  1120,  1121,  1121,
    1124,  1125,  1125,  1128,  1129,  1129,  1132,  1133,  1133,  1136,
    1137,  1137,  1140,  1141,  1141,  1144,  1145,  1146,  1148,  1152,
    1156,  1158,  1159,  1161,  1163,  1164,  1167,  1168,  1169,  1172,
    1176,  1178,  1182,  1182,  1189,  1192,  1195,  1196,  1197,  1200,
    1202,  1204,  1206,  1208,  1210,  1212,  1214,  1216,  1220,  1221,
    1230,  1231,  1235,  1241,  1242,  1243,  1253,  1253,  1333,  1377,
    1378,  1381,  1383,  1385,  1387,  1391,  1394,  1395
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ADDRESS", "ARG", "CALL", "DO", "TO",
  "BY", "FOR", "WHILE", "UNTIL", "EXIT", "IF", "THEN", "ELSE", "ITERATE",
  "INTERPRET", "LEAVE", "NOP", "NUMERIC", "PARSE", "EXTERNAL", "SOURCE",
  "VAR", "VALUE", "WITH", "PROCEDURE", "EXPOSE", "PULL", "PUSH", "QUEUE",
  "SAY", "RETURN", "SELECT", "WHEN", "DROP", "OTHERWISE", "SIGNAL", "ON",
  "OFF", "ERROR", "SYNTAX", "HALT", "NOVALUE", "TRACE", "END", "UPPER",
  "ASSIGNMENTVARIABLE", "STATSEP", "FOREVER", "DIGITS", "FORM", "FUZZ",
  "SCIENTIFIC", "ENGINEERING", "NOT", "CONCATENATE", "MODULUS", "GTE",
  "GT", "LTE", "LT", "DIFFERENT", "EQUALEQUAL", "NOTEQUALEQUAL", "OFFSET",
  "SPACE", "EXP", "XOR", "PLACEHOLDER", "NOTREADY", "CONSYMBOL",
  "SIMSYMBOL", "EXFUNCNAME", "INFUNCNAME", "LABEL", "DOVARIABLE",
  "HEXSTRING", "STRING", "VERSION", "LINEIN", "WHATEVER", "NAME",
  "FAILURE", "BINSTRING", "OPTIONS", "ENVIRONMENT", "LOSTDIGITS", "GTGT",
  "LTLT", "NOTGTGT", "NOTLTLT", "GTGTE", "LTLTE", "INPUT", "OUTPUT",
  "NORMAL", "APPEND", "REPLACE", "STREAM", "STEM", "LIFO", "FIFO", "LOWER",
  "CASELESS", "'|'", "'&'", "'='", "CCAT", "'+'", "'-'", "'*'", "'/'",
  "'%'", "UPLUS", "UMINUS", "'('", "')'", "','", "$accept", "start", "@1",
  "prog", "stats", "xstats", "ystatement", "lonely_end", "nxstats", "ncl",
  "nlncl", "optLabels", "statement", "gruff", "mttstatement",
  "mtstatement", "nclstatement", "call", "do", "exit", "if", "iterate",
  "leave", "say", "return", "address", "arg", "drop", "interpret", "label",
  "nop", "numeric", "options", "parse", "proc", "pull", "push", "queue",
  "select", "signal", "when", "otherwise", "trace", "upper",
  "address_stat", "@2", "address_stat2", "@3", "arg_stat", "call_stat",
  "@4", "@5", "call_name", "call_args", "expr_stat", "end_stat", "end",
  "do_stat", "repetitor", "nvir", "naddr_with", "@6", "addr_with",
  "connection", "inputstmts", "outputstmts", "errorstmts", "adeo", "adei",
  "adio", "inputstmt", "outputstmt", "errorstmt", "resourcei", "resourceo",
  "resources", "nsimsymb", "@7", "nnvir", "@8", "nspace", "addrAll",
  "addrSim", "addrString", "dovar", "tobyfor", "conditional", "drop_stat",
  "upper_stat", "exit_stat", "if_stat", "unexp_then", "unexp_else",
  "ipret_stat", "iterate_stat", "label_stat", "labelname", "leave_stat",
  "nop_stat", "numeric_stat", "form_expr", "options_stat", "parse_stat",
  "parse_flags", "templs", "parse_param", "proc_stat", "pull_stat",
  "push_stat", "queue_stat", "say_stat", "return_stat", "sel_end",
  "select_stat", "when_stats", "when_stat", "when_or_other",
  "ex_when_stat", "otherwise_stat", "signal_stat", "signal_name",
  "namespec", "asymbol", "on", "off", "c_action", "s_action", "trace_stat",
  "whatever", "assignment", "ass_part", "expr", "@9", "@10", "@11", "@12",
  "@13", "@14", "@15", "@16", "@17", "@18", "@19", "@20", "@21", "@22",
  "@23", "@24", "@25", "@26", "@27", "@28", "@29", "@30", "@31", "@32",
  "@33", "@34", "@35", "exprs_sub", "symbtree", "function", "func_args",
  "@36", "intfunc", "extfunc", "template", "solid", "offset", "string",
  "pv", "exprs", "@37", "nexpr", "anyvars", "xsimsymb", "simsymb", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
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
     355,   356,   357,   358,   359,   360,   124,    38,    61,   361,
      43,    45,    42,    47,    37,   362,   363,    40,    41,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short int yyr1[] =
{
       0,   120,   122,   121,   123,   123,   124,   124,   125,   125,
     126,   126,   127,   128,   128,   129,   129,   130,   130,   131,
     131,   132,   132,   133,   134,   135,   135,   135,   135,   136,
     136,   136,   136,   136,   136,   136,   136,   136,   136,   136,
     136,   136,   136,   136,   136,   136,   136,   136,   136,   136,
     136,   136,   136,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   165,   164,   166,   166,   166,   167,   166,   166,   166,
     168,   170,   169,   171,   169,   169,   169,   169,   169,   169,
     169,   169,   169,   172,   172,   173,   173,   174,   175,   176,
     176,   176,   177,   178,   178,   178,   178,   178,   178,   178,
     178,   179,   179,   179,   181,   180,   180,   182,   182,   182,
     183,   183,   183,   183,   184,   185,   186,   187,   187,   187,
     187,   187,   188,   188,   188,   188,   188,   189,   189,   189,
     189,   189,   190,   190,   191,   191,   192,   192,   193,   193,
     194,   194,   194,   194,   194,   194,   195,   195,   195,   195,
     195,   195,   195,   195,   197,   196,   199,   198,   200,   200,
     201,   201,   202,   203,   204,   205,   205,   205,   206,   206,
     206,   207,   207,   208,   208,   209,   210,   210,   210,   210,
     210,   210,   211,   212,   213,   214,   214,   214,   215,   216,
     217,   217,   217,   218,   218,   219,   219,   219,   219,   219,
     219,   219,   219,   219,   219,   220,   220,   221,   222,   222,
     222,   222,   223,   223,   223,   223,   223,   223,   223,   224,
     224,   225,   225,   225,   225,   225,   225,   225,   225,   225,
     226,   226,   226,   226,   226,   227,   228,   229,   230,   231,
     232,   232,   232,   233,   233,   233,   233,   233,   233,   234,
     234,   234,   235,   235,   235,   235,   236,   236,   237,   238,
     238,   239,   239,   239,   239,   239,   239,   239,   239,   239,
     239,   239,   240,   240,   240,   241,   241,   242,   242,   243,
     244,   245,   245,   245,   245,   246,   246,   246,   246,   247,
     247,   247,   247,   248,   249,   250,   252,   251,   253,   251,
     254,   251,   255,   251,   251,   256,   251,   251,   257,   251,
     251,   258,   251,   251,   259,   251,   251,   260,   251,   251,
     261,   251,   251,   262,   251,   251,   263,   251,   251,   264,
     251,   251,   251,   251,   265,   251,   251,   266,   251,   251,
     267,   251,   251,   268,   251,   251,   269,   251,   251,   270,
     251,   251,   271,   251,   251,   272,   251,   251,   273,   251,
     251,   274,   251,   251,   275,   251,   251,   276,   251,   251,
     277,   251,   251,   278,   251,   251,   251,   251,   251,   251,
     251,   251,   251,   251,   251,   251,   279,   279,   279,   280,
     281,   281,   283,   282,   284,   285,   286,   286,   286,   287,
     287,   287,   287,   287,   287,   287,   287,   287,   288,   288,
     289,   289,   289,   290,   290,   290,   292,   291,   291,   293,
     293,   294,   294,   294,   294,   295,   296,   296
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     2,     2,     1,     2,     1,     2,     2,
       1,     1,     3,     1,     1,     3,     2,     2,     1,     2,
       0,     1,     1,     0,     2,     2,     1,     1,     1,     1,
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
       1,     1,     6,     7,     2,     2,     1,     1,     2,     3,
       0,     3,     3,     2,     3,     3,     4,     5,     4,     3,
       4,     3,     1,     1,     1,     2,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       2,     3,     2,     1,     2,     1,     0,     3,     0,     4,
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

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short int yydefact[] =
{
       2,     0,    20,     1,     3,    23,    18,    23,     7,    11,
      10,    23,    21,    22,    20,    66,    17,   209,    19,   208,
       6,    62,    63,    54,    55,    56,    57,    58,    65,    59,
      67,    68,    70,    71,    72,    73,    74,    60,    61,    75,
      77,    64,    78,    76,    79,   108,    80,   315,   353,   351,
     330,   359,   365,   362,   368,   371,   374,   377,   348,   345,
     342,   397,   409,   415,   414,   398,   400,   399,    69,   380,
     383,   386,   389,   392,   395,   336,   339,   356,     0,     0,
     324,   327,   333,   316,     0,    24,     0,     0,   120,   440,
       0,     0,     0,   440,   440,    81,     0,     0,     0,     0,
       0,   440,     0,     0,     0,   440,   440,     0,     0,   276,
     277,     0,     0,    29,    31,    32,    30,     0,     0,    33,
      34,    52,    35,    26,    27,    28,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    48,    47,    49,
     278,    50,    51,    53,   440,   107,   396,   401,   412,   412,
      16,    20,   352,   404,   402,   405,   403,   440,   202,   203,
      25,   104,   299,   300,   297,   298,   431,   430,   432,    91,
     103,     0,     0,    93,     0,   184,   190,     0,   179,   439,
     195,   200,    20,   447,   446,     0,     0,   258,   259,     0,
     418,   435,    90,   435,   240,   417,   445,     0,     0,   442,
     204,   214,   224,   216,     0,   222,   227,   231,   241,   243,
     246,     0,     0,   245,   232,   244,   242,   235,   238,     0,
       0,   251,     0,   255,   256,   257,   266,     0,   294,     0,
     293,     0,   292,     0,     0,     0,   313,     0,   310,     0,
       0,    12,   314,   349,   328,   357,   363,   360,   366,   369,
     372,   375,   346,   343,   340,   378,   381,   384,   387,   390,
     393,   334,   337,   354,   318,   320,   322,   325,   331,   411,
     440,   410,    15,   408,   317,     0,   438,   440,    95,   301,
     302,   303,   304,     0,    96,     0,   440,   118,   178,   117,
       0,     0,     0,     0,   346,   119,     0,   206,   211,    86,
       0,     0,   121,   122,   123,     0,    82,   440,    84,   433,
       0,   434,   428,   429,     0,     0,     0,     0,     0,   421,
     427,     0,   191,   441,   215,   221,     0,   225,   226,     0,
     223,   247,   249,     0,   233,   236,   234,   237,   230,     0,
     229,   252,     0,   271,   267,     0,     0,     0,   280,   270,
     281,   282,   284,   307,   306,   308,   305,     0,   285,     0,
     309,   311,   193,   111,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     413,   407,   406,   436,    92,   105,    97,     0,     0,   101,
      94,   179,   179,    23,   179,   201,    20,   124,   124,   133,
       0,   130,   131,   132,   179,   179,   179,   129,     0,   124,
     239,     0,   422,     0,   420,     0,   419,     0,   416,   444,
     220,   217,   248,   228,   253,   264,   275,    20,   265,     0,
     269,     0,   286,     0,   290,   350,   329,   358,   364,   361,
     367,   370,   373,   376,   347,   344,   341,   379,   382,   385,
     388,   391,   394,   335,   338,   355,   319,   321,   323,   326,
     332,   440,   106,   296,   295,    98,   188,   189,    23,     0,
      23,    23,   116,     0,    87,     0,    83,   128,   134,   179,
     179,   141,   135,   179,   179,   146,   136,   179,   179,   151,
       0,     0,     0,   179,    88,     0,     0,     0,   423,   443,
       0,   268,    23,     0,   263,   287,   437,     8,    23,   112,
       9,     0,     0,     0,   115,   199,   196,   125,   179,   137,
     179,   139,   179,   142,   179,   144,   179,   147,   179,   149,
     157,     0,   153,     0,   155,     0,   124,   426,   425,   424,
      20,   279,     0,   179,   179,   179,   114,    20,   138,   140,
     143,   145,   148,   150,   165,     0,     0,     0,     0,     0,
       0,   156,   160,   159,   152,   158,   154,    89,    23,   261,
     185,   187,   186,   113,     0,   162,   161,   164,   163,   167,
     166,   179,   169,   168,   179,   171,   170,   173,   172,   272,
     198,   197,     0,     0,   273,   177,   180,   181,   183,   182,
     175
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short int yydefgoto[] =
{
      -1,     1,     2,     4,     7,   478,     8,     9,   479,    16,
       5,     6,    10,    11,    12,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    17,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   189,   306,   407,   114,   115,
     277,   286,   169,   394,   116,   117,   118,   119,   176,   307,
     484,   485,   308,   410,   411,   412,   413,   488,   492,   496,
     414,   415,   416,   574,   571,   572,   593,   594,   590,   591,
     289,   605,   606,   607,   177,   524,   292,   120,   121,   122,
     123,   124,   125,   126,   127,    18,    19,   128,   129,   130,
     329,   131,   132,   219,   192,   220,   133,   134,   135,   136,
     137,   138,   514,   139,   348,   349,   140,    13,   441,   141,
     231,   398,   170,   171,   172,   356,   357,   142,   237,   143,
     144,   179,   157,   385,   386,   387,   388,   365,   389,   382,
     383,   375,   374,   373,   364,   384,   366,   368,   367,   369,
     370,   371,   372,   376,   377,   378,   379,   380,   381,   274,
     146,   147,   269,   270,   148,   149,   194,   318,   319,   173,
     195,   275,   471,   276,   198,   199,   185
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -493
static const short int yypact[] =
{
    -493,    59,  -493,  -493,  -493,    63,    -8,    90,  -493,  -493,
    -493,  1133,  -493,  -493,  -493,  -493,    -4,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,   405,  -493,
    -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,   888,   953,
    -493,  -493,  -493,  -493,   247,  -493,     3,   268,  1679,  1804,
    1548,    28,    37,  1804,  1804,  -493,    39,   177,  1804,    45,
     252,  1804,    33,   167,    82,  1804,  1804,   168,   270,  -493,
    -493,  1425,   177,  -493,  -493,  -493,  -493,   214,     3,  -493,
    -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,  1804,  1927,  -493,  -493,  -493,  -493,
      70,  -493,  -493,  -493,  -493,  -493,  -493,  1614,  -493,  -493,
      -4,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    -493,    52,   391,  -493,   334,  -493,    56,    83,  1985,  1927,
    -493,    -4,  1927,  -493,  -493,   257,   265,  -493,  -493,    47,
    -493,   -19,    84,   -19,  -493,  2092,  -493,   135,   299,   177,
    1927,  -493,  -493,  1804,   456,  1804,  -493,  -493,  -493,  -493,
    -493,    34,   823,  -493,   128,  -493,  -493,   149,    -5,   414,
      39,  -493,    48,  -493,  -493,  -493,  -493,  1526,  -493,  1804,
    -493,   381,  -493,   205,   231,  1804,  -493,   404,  1927,   409,
     416,    -4,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    1614,  -493,    70,  -493,  -493,    26,   140,  1804,  -493,  -493,
    -493,  -493,  -493,    42,  -493,   418,  1804,  -493,  -493,  -493,
    1804,  1804,     3,  1804,   556,  -493,    46,  -493,  -493,  -493,
    1804,   161,  -493,  -493,  -493,  1804,  -493,  1804,  -493,  -493,
      39,  -493,  -493,  -493,    -1,    10,   211,   194,    39,  -493,
    -493,   147,  -493,  -493,  1927,  -493,  1804,  -493,  -493,   420,
    1927,  -493,  -493,   262,  -493,  -493,  -493,  -493,  -493,    39,
      84,  -493,   440,  -493,  -493,   289,  1018,   291,   192,  -493,
    1927,  -493,  -493,  -493,  -493,  -493,  -493,    49,  -493,   447,
    1927,  -493,  -493,  -493,  1804,  1804,  1804,  1804,  1804,  1804,
    1804,  1804,  1804,  1804,  1804,  1804,  1804,  1804,  1804,  1804,
    1804,  1804,  1804,  1804,  1804,  1804,  1804,  1804,  1804,  1804,
    -493,  -493,  -493,  -493,  -493,   179,  -493,    35,   455,  -493,
    -493,  1985,  1985,    -4,  1985,  -493,  -493,   267,  1745,  -493,
     458,  -493,  -493,  -493,   251,   251,   251,   184,  1865,   267,
    -493,   194,  -493,   194,  -493,   194,  -493,   202,  -493,   177,
    1927,  -493,  -493,    84,  -493,  -493,  -493,  1488,  -493,   524,
    -493,   285,  -493,   472,  -493,   406,   286,   387,   387,   387,
     387,   387,   387,   387,   406,  -493,  2043,   387,   387,   387,
     387,   387,   387,  2043,  2055,   387,   435,   435,   286,   286,
     286,  1804,  -493,  -493,  -493,  -493,  -493,  -493,   343,   360,
    -493,  1248,   316,   639,  -493,   383,  -493,  -493,  -493,   251,
     251,   125,  -493,   251,   251,   156,  -493,   251,   251,   190,
      73,   138,    86,   251,  -493,   298,   302,   304,  -493,  -493,
     412,  -493,  -493,   277,  -493,  -493,  -493,  -493,  1363,  -493,
    -493,  1804,  1804,  1804,   316,  -493,   408,  -493,   251,   399,
     251,   332,   251,   399,   251,   347,   251,   332,   251,   347,
    -493,   591,  -493,   522,  -493,   591,   267,  -493,  -493,  -493,
    -493,  -493,   484,  1985,  1985,  1985,   316,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,  -493,    77,   142,   134,    30,   254,
     329,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,   754,  -493,  -493,  -493,  -493,  -493,
    -493,   251,  -493,  -493,   251,  -493,  -493,  -493,  -493,   432,
    -493,  -493,   548,   135,  -493,  -493,  -493,  -493,  -493,  -493,
    -493
};

/* YYPGOTO[NTERM-NUM].  */
static const short int yypgoto[] =
{
    -493,  -493,  -493,  -493,  -493,  -493,    -7,  -493,   -65,   -74,
    -169,     0,  -378,   -10,  -493,  -493,  -493,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    -207,  -206,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    -493,  -493,  -493,   163,  -493,  -493,   -16,  -493,  -493,  -493,
    -389,  -493,   -25,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    -234,  -286,  -330,  -493,   -79,  -214,  -493,  -493,  -185,  -493,
    -103,  -493,  -121,  -493,  -493,  -492,  -493,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,  -194,   273,  -493,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,  -493,   139,  -493,  -493,  -493,  -493,
    -493,   129,   398,   407,   415,   232,   290,  -493,  -493,  -493,
    -493,    -9,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,
    -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,  -493,   256,
     -87,  -493,   382,  -493,  -493,  -493,   -96,  -493,   260,  -191,
      43,  -262,  -493,   -83,  -107,  -190,   -89
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -441
static const short int yytable[] =
{
      20,    84,   145,   186,   320,   239,   180,   321,   223,   193,
     187,   188,   160,   296,   150,   395,   181,   193,   206,   486,
     346,   347,   224,   225,   395,   480,   340,   391,   240,   183,
     504,   592,   556,   227,   207,   183,   473,   208,   183,   152,
     190,    14,   336,   396,   241,   151,   201,   405,   299,   341,
     442,   191,    14,   278,    62,   209,   210,   211,   212,     3,
     406,   242,   213,    -5,   583,   312,   290,   291,    15,   154,
     156,   313,   300,   301,   540,   295,   312,  -207,   585,   178,
     214,   182,   313,   190,   490,   494,  -212,   544,  -435,   200,
      -4,  -100,   323,   279,  -213,   280,   -85,  -174,  -289,   337,
     517,   184,   238,  -174,   193,  -435,   193,   184,   474,   191,
     184,  -435,    62,   215,   216,   342,   421,  -435,  -435,   302,
     303,   196,   331,   281,  -435,   397,   304,   423,   489,   333,
     498,  -435,   397,   193,   480,   589,   282,   217,   218,   542,
     288,   346,   439,   587,   392,   433,    15,  -435,  -435,  -435,
    -435,   272,   191,   288,  -435,    62,  -435,   577,  -435,   528,
    -435,  -435,   409,   532,   305,   197,   500,  -435,   221,   226,
    -179,  -179,  -179,  -179,  -179,  -179,  -179,   567,   568,   569,
     570,   493,   497,  -179,  -179,  -179,  -179,  -179,  -179,  -179,
    -435,   293,  -435,  -435,   324,   222,   330,   500,   417,  -435,
     599,  -176,  -179,   310,   530,   288,   352,  -176,   196,   516,
    -179,   536,  -176,  -176,   420,   183,  -250,    14,   403,  -176,
     350,   502,   428,   193,   419,   500,   360,    40,   288,    42,
     427,   193,   358,   334,   309,  -179,   311,   483,  -179,  -179,
    -179,  -179,   567,   568,   569,   570,   279,   353,   280,   354,
     196,   501,   193,   202,   335,   595,  -179,  -179,   297,   393,
     534,   158,   159,  -110,   538,   429,   298,    62,   510,   161,
     512,   228,   279,   353,   280,   354,   281,   312,   183,   501,
     502,   401,   402,   313,   404,   501,   502,   184,   432,   282,
     435,   408,   438,   355,   197,   229,   418,   472,   476,   477,
     322,   482,   281,   203,   204,   205,  -205,   162,   163,   162,
     163,   491,   495,   499,  -210,   282,  -126,   430,   288,   355,
     508,  -176,   509,   521,   522,   523,  -262,  -176,   425,   575,
     597,   513,  -176,  -176,   505,   287,   506,   437,   507,  -176,
     164,   165,   164,   165,  -179,  -179,   166,   167,  -192,   230,
     184,   586,   588,   168,   253,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   578,   351,  -179,   596,   598,   529,   531,   584,   -13,
     533,   535,   284,   481,   537,   539,  -176,   541,   543,   545,
     546,   288,  -176,   283,   285,   361,    45,  -176,  -176,   301,
     362,   608,   609,   609,  -176,   338,   547,   363,   208,   399,
     548,   431,   549,   557,   552,   558,   550,   559,   502,   560,
    -283,   561,   279,   562,   280,   563,   209,   210,   211,   212,
     500,   434,   501,   213,   243,   244,   604,   551,   444,   400,
     580,   581,   582,  -312,   252,   253,   475,   325,  -194,   487,
     527,    48,   281,   519,   244,  -109,   576,  -102,   518,  -218,
     520,    84,   145,   515,   253,   282,   526,    61,    62,    63,
      64,   326,   610,    65,    66,   579,   443,   440,   602,  -254,
      67,   603,   339,   244,   215,   216,  -291,   264,   265,   266,
     267,   268,   481,   253,   -99,  -219,   232,  -127,    84,   145,
     327,   328,   553,   554,   555,   233,   264,   265,   266,   267,
     268,  -288,    83,   234,   359,   511,   390,   -20,   -20,   -20,
     -20,   271,     0,  -260,     0,     0,   -20,   -20,   -20,   -20,
     -20,   -20,   -20,   -20,   -20,   -20,     0,   266,   267,   268,
       0,   -20,     0,   -20,   -20,   -20,   -20,   -20,   -20,   -20,
     -20,   -20,   -20,  -178,  -178,  -178,  -178,  -178,   518,   -20,
     -20,   -20,   -20,   -20,   422,   424,   426,   601,     0,     0,
     -20,   -20,   -20,   -20,   -20,   -20,   -20,   -20,   -20,   -20,
       0,   -20,   -20,   -20,     0,     0,   -20,   -20,   -20,   -20,
     -20,     0,   -20,   -20,     0,  -178,     0,     0,     0,   -20,
     -20,     0,     0,   -20,   -20,   -20,   -20,   -20,   -20,   573,
       0,   196,   567,   568,   569,   570,   166,   167,     0,     0,
     -20,   -20,   -20,   168,   -20,   -20,   -20,   -20,   -20,     0,
     525,   -20,   -23,   -23,   -23,   -23,     0,     0,     0,     0,
       0,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,
     -23,     0,     0,     0,     0,     0,   -23,     0,   -23,   -23,
     -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,     0,     0,
       0,     0,     0,     0,   -23,   -23,   -23,   -23,   564,   565,
     566,   567,   568,   569,   570,   -23,   -23,   -23,   -23,   -23,
     -23,   -23,   -23,   -23,   -23,     0,   -23,   -23,   -23,     0,
       0,   -23,   -23,   -23,   -23,     0,     0,   -23,   -23,     0,
       0,     0,     0,     0,   -23,   -23,     0,     0,   -23,   -23,
     -23,   -23,   -23,   -23,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   -23,   -23,   -23,     0,   -23,
     -23,   -23,   -23,   -23,     0,   600,   -23,   -23,   -23,   -23,
     -23,     0,     0,     0,     0,     0,   -23,   -23,   -23,   -23,
     -23,   -23,   -23,   -23,   -23,   -23,     0,     0,     0,     0,
       0,   -23,     0,   -23,   -23,   -23,   -23,   -23,   -23,   -23,
     -23,   -23,   -23,     0,     0,     0,     0,     0,     0,   -23,
     -23,   -23,   -23,     0,     0,     0,     0,     0,     0,     0,
     -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,   -23,
       0,   -23,   -23,   -23,   332,     0,   -23,   -23,   -23,   -23,
       0,     0,   -23,   -23,     0,     0,     0,     0,     0,   -23,
     -23,     0,     0,   -23,   -23,   -23,   -23,   -23,   -23,  -440,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     -23,   -23,   -23,     0,   -23,   -23,   -23,   -23,   -23,     0,
       0,   -23,     0,     0,     0,     0,     0,     0,     0,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,   153,
      58,    59,    60,     0,     0,    61,    62,    63,    64,     0,
       0,    65,    66,     0,     0,     0,     0,     0,    67,     0,
       0,     0,    69,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
      76,    77,     0,    78,    79,    80,    81,    82,     0,     0,
      83,     0,     0,     0,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,   155,    58,    59,    60,     0,     0,
      61,    62,    63,    64,     0,     0,    65,    66,     0,     0,
       0,     0,     0,    67,     0,     0,     0,    69,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,    76,    77,     0,    78,    79,
      80,    81,    82,     0,     0,    83,     0,     0,     0,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,   436,
      58,    59,    60,     0,     0,    61,    62,    63,    64,     0,
       0,    65,    66,     0,     0,     0,     0,     0,    67,     0,
       0,     0,    69,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
      76,    77,     0,    78,    79,    80,    81,    82,     0,     0,
      83,     0,     0,     0,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,     0,    58,    59,    60,     0,     0,
      61,    62,    63,    64,     0,     0,    65,    66,     0,     0,
       0,     0,     0,    67,     0,     0,     0,    69,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,    76,    77,     0,    78,    79,
      80,    81,    82,     0,     0,    83,    21,    22,    23,    24,
       0,     0,     0,     0,     0,    25,    26,     0,     0,    27,
      28,    29,    30,    31,    32,     0,     0,     0,     0,     0,
      33,     0,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,     0,     0,     0,     0,     0,     0,    44,    45,
      46,    47,     0,     0,     0,     0,     0,     0,     0,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,     0,
      58,    59,    60,     0,     0,    61,    62,    63,    64,     0,
       0,    65,    66,     0,     0,     0,     0,     0,    67,    68,
       0,     0,    69,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
      76,    77,     0,    78,    79,    80,    81,    82,     0,     0,
      83,    21,    22,    23,    24,     0,     0,     0,     0,     0,
      25,    26,     0,     0,    27,    28,    29,    30,    31,    32,
       0,     0,     0,     0,     0,    33,     0,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,     0,     0,     0,
       0,     0,     0,    44,   -14,    46,    47,     0,     0,     0,
       0,     0,     0,     0,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,     0,    58,    59,    60,     0,     0,
      61,    62,    63,    64,     0,     0,    65,    66,     0,     0,
       0,     0,     0,    67,    68,     0,     0,    69,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,    76,    77,     0,    78,    79,
      80,    81,    82,     0,     0,    83,    21,    22,    23,    24,
       0,     0,     0,     0,     0,    25,    26,     0,     0,    27,
      28,    29,    30,    31,    32,     0,     0,     0,     0,     0,
      33,     0,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,     0,     0,     0,     0,     0,     0,    44,     0,
      46,    47,     0,     0,     0,     0,     0,     0,     0,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,     0,
      58,    59,    60,     0,     0,    61,    62,    63,    64,     0,
       0,    65,    66,     0,     0,     0,     0,     0,    67,    68,
     235,     0,    69,    70,    71,    72,    73,    74,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    75,
      76,    77,     0,    78,    79,    80,    81,    82,     0,     0,
      83,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,     0,    58,    59,    60,     0,     0,    61,    62,    63,
      64,     0,     0,    65,    66,     0,     0,   236,     0,     0,
      67,     0,     0,     0,    69,    70,    71,    72,    73,    74,
       0,     0,     0,  -274,     0,  -274,     0,   343,     0,     0,
       0,    75,    76,    77,  -274,    78,    79,    80,    81,    82,
     344,     0,    83,     0,     0,   243,   244,   245,   246,   247,
     248,   249,   250,   251,     0,   252,   253,   254,     0,     0,
       0,    40,     0,    42,     0,     0,     0,     0,     0,     0,
       0,     0,   345,     0,     0,   151,     0,   255,   256,   257,
     258,   259,   260,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   261,   262,   263,    14,   264,   265,
     266,   267,   268,     0,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,     0,    58,    59,    60,     0,     0,
      61,    62,    63,    64,     0,     0,    65,    66,     0,     0,
       0,     0,     0,    67,     0,     0,     0,    69,    70,    71,
      72,    73,    74,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    75,    76,    77,     0,    78,    79,
      80,    81,    82,   273,     0,    83,     0,     0,     0,     0,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
       0,    58,    59,    60,     0,     0,    61,    62,    63,    64,
       0,     0,    65,    66,     0,     0,     0,     0,     0,    67,
       0,     0,     0,    69,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,    76,    77,     0,    78,    79,    80,    81,    82,   174,
       0,    83,     0,     0,     0,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,     0,    58,    59,    60,     0,
       0,    61,    62,    63,    64,     0,   175,    65,    66,     0,
       0,     0,     0,     0,    67,     0,     0,     0,    69,    70,
      71,    72,    73,    74,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    75,    76,    77,     0,    78,
      79,    80,    81,    82,  -126,     0,    83,     0,     0,     0,
       0,     0,   243,   244,   245,   246,   247,   248,   249,   250,
     251,     0,   252,   253,   254,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   255,   256,   257,   258,   259,   260,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   261,   262,   263,     0,   264,   265,   266,   267,   268,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
       0,    58,    59,    60,     0,     0,    61,    62,    63,    64,
       0,     0,    65,    66,     0,     0,     0,     0,     0,    67,
       0,     0,     0,    69,    70,    71,    72,    73,    74,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      75,    76,    77,     0,    78,    79,    80,    81,    82,     0,
       0,    83,   243,   244,   245,   246,   247,   248,   249,   250,
     251,     0,   252,   253,   254,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   255,   256,   257,   258,   259,   260,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   261,   262,   263,     0,   264,   265,   266,   267,   268,
       0,     0,     0,   503,   243,   244,   245,   246,   247,   248,
     249,   250,   251,     0,   252,   253,   254,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   255,   256,   257,   258,
     259,   260,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   261,   262,   263,     0,   264,   265,   266,
     267,   268,   243,   244,   245,   246,   247,   248,   249,   250,
     251,     0,   294,   253,   254,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   255,   256,   257,   258,   259,   260,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   261,   262,   263,     0,   264,   265,   266,   267,   268,
     243,   244,   245,   246,   247,   248,   249,   250,   251,     0,
     252,   253,   243,   244,   245,   246,   247,   248,   249,   250,
     251,     0,   252,   253,     0,     0,     0,     0,     0,     0,
       0,     0,   255,   256,   257,   258,   259,   260,     0,     0,
       0,     0,     0,     0,   255,   256,   257,   258,   259,   260,
     262,   263,     0,   264,   265,   266,   267,   268,   312,     0,
       0,     0,     0,   263,   313,   264,   265,   266,   267,   268,
     166,   167,     0,     0,     0,     0,     0,   168,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     314,     0,   315,   316,     0,     0,     0,     0,     0,   317
};

static const short int yycheck[] =
{
       7,    11,    11,    92,   195,   112,    89,   197,   104,    96,
      93,    94,    86,   182,    14,   277,    90,   104,   101,   408,
     227,   227,   105,   106,   286,   403,   220,     1,   117,     1,
     419,     1,   524,   107,     1,     1,     1,     4,     1,    48,
       1,    49,    47,     1,   118,    49,     1,     1,     1,     1,
       1,    70,    49,     1,    73,    22,    23,    24,    25,     0,
      14,   144,    29,     0,   556,    66,    10,    11,    76,    78,
      79,    72,    25,    26,     1,   178,    66,    49,     1,    88,
      47,    90,    72,     1,   414,   415,    49,     1,    49,    98,
       0,    49,   199,    41,    49,    43,    49,    67,    49,   104,
     478,    73,   111,    73,   191,    66,   193,    73,    73,    70,
      73,    72,    73,    80,    81,   222,   117,    78,    79,    72,
      73,    73,   211,    71,    85,    83,    79,   117,   414,   212,
     416,    49,    83,   220,   512,     1,    84,   104,   105,     1,
      67,   348,   348,     1,   118,   339,    76,   108,    66,   110,
     111,   151,    70,    67,    72,    73,   117,   546,   119,   489,
      78,    79,     1,   493,   117,   117,    41,    85,     1,     1,
      97,    98,    99,   100,   101,   102,   103,   100,   101,   102,
     103,   415,   416,    97,    98,    99,   100,   101,   102,   103,
     108,   108,   110,   111,   203,    28,   205,    41,   301,   117,
     578,    67,    41,   119,   490,    67,     1,    73,    73,   471,
      49,   497,    78,    79,   310,     1,    49,    49,   292,    85,
     229,    96,   318,   310,   307,    41,   235,    35,    67,    37,
     317,   318,     1,   105,   191,    97,   193,   406,   100,   101,
     102,   103,   100,   101,   102,   103,    41,    42,    43,    44,
      73,    95,   339,     1,   105,     1,    95,    96,     1,   119,
     494,    14,    15,    49,   498,   118,     1,    73,   437,     1,
     439,     1,    41,    42,    43,    44,    71,    66,     1,    95,
      96,   290,   291,    72,   293,    95,    96,    73,    26,    84,
       1,   300,     1,    88,   117,    25,   305,   118,   401,   402,
       1,   404,    71,    51,    52,    53,    49,    39,    40,    39,
      40,   414,   415,   416,    49,    84,    49,   326,    67,    88,
     118,    67,   429,     7,     8,     9,    49,    73,   117,   543,
       1,    46,    78,    79,   421,     1,   423,   346,   425,    85,
      72,    73,    72,    73,    10,    11,    78,    79,    49,    79,
      73,   565,   566,    85,    68,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
     389,   550,     1,    49,   569,   570,   489,   490,   557,    46,
     493,   494,     1,   403,   497,   498,    67,   500,   501,   502,
     503,    67,    73,   171,   172,     1,    46,    78,    79,    26,
       1,   602,   602,   603,    85,     1,   118,     1,     4,     1,
     118,     1,   118,    15,   513,   528,    14,   530,    96,   532,
      49,   534,    41,   536,    43,   538,    22,    23,    24,    25,
      41,     1,    95,    29,    57,    58,    14,   512,     1,   286,
     553,   554,   555,    49,    67,    68,     1,     1,    49,     1,
     485,    56,    71,   479,    58,    49,   545,    49,   478,    49,
     480,   481,   481,     1,    68,    84,   483,    72,    73,    74,
      75,    25,   603,    78,    79,     1,   357,   348,   591,    49,
      85,   594,   219,    58,    80,    81,    49,   110,   111,   112,
     113,   114,   512,    68,    49,    49,   108,    49,   518,   518,
      54,    55,   521,   522,   523,   108,   110,   111,   112,   113,
     114,    49,   117,   108,   234,     1,   270,     3,     4,     5,
       6,   149,    -1,    49,    -1,    -1,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    -1,   112,   113,   114,
      -1,    27,    -1,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,     7,     8,     9,    10,    11,   578,    45,
      46,    47,    48,    49,   314,   315,   316,   584,    -1,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    67,    68,    69,    -1,    -1,    72,    73,    74,    75,
      76,    -1,    78,    79,    -1,    49,    -1,    -1,    -1,    85,
      86,    -1,    -1,    89,    90,    91,    92,    93,    94,    97,
      -1,    73,   100,   101,   102,   103,    78,    79,    -1,    -1,
     106,   107,   108,    85,   110,   111,   112,   113,   114,    -1,
       1,   117,     3,     4,     5,     6,    -1,    -1,    -1,    -1,
      -1,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    46,    47,    48,    97,    98,
      99,   100,   101,   102,   103,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    67,    68,    69,    -1,
      -1,    72,    73,    74,    75,    -1,    -1,    78,    79,    -1,
      -1,    -1,    -1,    -1,    85,    86,    -1,    -1,    89,    90,
      91,    92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,   107,   108,    -1,   110,
     111,   112,   113,   114,    -1,     1,   117,     3,     4,     5,
       6,    -1,    -1,    -1,    -1,    -1,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      46,    47,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    67,    68,    69,     1,    -1,    72,    73,    74,    75,
      -1,    -1,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,
      86,    -1,    -1,    89,    90,    91,    92,    93,    94,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,   107,   108,    -1,   110,   111,   112,   113,   114,    -1,
      -1,   117,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,     1,
      67,    68,    69,    -1,    -1,    72,    73,    74,    75,    -1,
      -1,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,    -1,   110,   111,   112,   113,   114,    -1,    -1,
     117,    -1,    -1,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,     1,    67,    68,    69,    -1,    -1,
      72,    73,    74,    75,    -1,    -1,    78,    79,    -1,    -1,
      -1,    -1,    -1,    85,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,   107,   108,    -1,   110,   111,
     112,   113,   114,    -1,    -1,   117,    -1,    -1,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,     1,
      67,    68,    69,    -1,    -1,    72,    73,    74,    75,    -1,
      -1,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,    -1,   110,   111,   112,   113,   114,    -1,    -1,
     117,    -1,    -1,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    67,    68,    69,    -1,    -1,
      72,    73,    74,    75,    -1,    -1,    78,    79,    -1,    -1,
      -1,    -1,    -1,    85,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,   107,   108,    -1,   110,   111,
     112,   113,   114,    -1,    -1,   117,     3,     4,     5,     6,
      -1,    -1,    -1,    -1,    -1,    12,    13,    -1,    -1,    16,
      17,    18,    19,    20,    21,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    46,
      47,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      67,    68,    69,    -1,    -1,    72,    73,    74,    75,    -1,
      -1,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,    86,
      -1,    -1,    89,    90,    91,    92,    93,    94,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,    -1,   110,   111,   112,   113,   114,    -1,    -1,
     117,     3,     4,     5,     6,    -1,    -1,    -1,    -1,    -1,
      12,    13,    -1,    -1,    16,    17,    18,    19,    20,    21,
      -1,    -1,    -1,    -1,    -1,    27,    -1,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    67,    68,    69,    -1,    -1,
      72,    73,    74,    75,    -1,    -1,    78,    79,    -1,    -1,
      -1,    -1,    -1,    85,    86,    -1,    -1,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,   107,   108,    -1,   110,   111,
     112,   113,   114,    -1,    -1,   117,     3,     4,     5,     6,
      -1,    -1,    -1,    -1,    -1,    12,    13,    -1,    -1,    16,
      17,    18,    19,    20,    21,    -1,    -1,    -1,    -1,    -1,
      27,    -1,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,
      47,    48,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      67,    68,    69,    -1,    -1,    72,    73,    74,    75,    -1,
      -1,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,    86,
      25,    -1,    89,    90,    91,    92,    93,    94,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,    -1,   110,   111,   112,   113,   114,    -1,    -1,
     117,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    68,    69,    -1,    -1,    72,    73,    74,
      75,    -1,    -1,    78,    79,    -1,    -1,    82,    -1,    -1,
      85,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    35,    -1,    37,    -1,     1,    -1,    -1,
      -1,   106,   107,   108,    46,   110,   111,   112,   113,   114,
      14,    -1,   117,    -1,    -1,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    67,    68,    69,    -1,    -1,
      -1,    35,    -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    -1,    -1,    49,    -1,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,   107,   108,    49,   110,   111,
     112,   113,   114,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    67,    68,    69,    -1,    -1,
      72,    73,    74,    75,    -1,    -1,    78,    79,    -1,    -1,
      -1,    -1,    -1,    85,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,   107,   108,    -1,   110,   111,
     112,   113,   114,    49,    -1,   117,    -1,    -1,    -1,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    67,    68,    69,    -1,    -1,    72,    73,    74,    75,
      -1,    -1,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,   107,   108,    -1,   110,   111,   112,   113,   114,    50,
      -1,   117,    -1,    -1,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    67,    68,    69,    -1,
      -1,    72,    73,    74,    75,    -1,    77,    78,    79,    -1,
      -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,    89,    90,
      91,    92,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,   107,   108,    -1,   110,
     111,   112,   113,   114,    49,    -1,   117,    -1,    -1,    -1,
      -1,    -1,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   107,   108,    -1,   110,   111,   112,   113,   114,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    67,    68,    69,    -1,    -1,    72,    73,    74,    75,
      -1,    -1,    78,    79,    -1,    -1,    -1,    -1,    -1,    85,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     106,   107,   108,    -1,   110,   111,   112,   113,   114,    -1,
      -1,   117,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   107,   108,    -1,   110,   111,   112,   113,   114,
      -1,    -1,    -1,   118,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    69,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,   108,    -1,   110,   111,   112,
     113,   114,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    68,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   107,   108,    -1,   110,   111,   112,   113,   114,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      67,    68,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    89,    90,    91,    92,    93,    94,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
     107,   108,    -1,   110,   111,   112,   113,   114,    66,    -1,
      -1,    -1,    -1,   108,    72,   110,   111,   112,   113,   114,
      78,    79,    -1,    -1,    -1,    -1,    -1,    85,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,    -1,   110,   111,    -1,    -1,    -1,    -1,    -1,   117
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short int yystos[] =
{
       0,   121,   122,     0,   123,   130,   131,   124,   126,   127,
     132,   133,   134,   237,    49,    76,   129,   149,   215,   216,
     126,     3,     4,     5,     6,    12,    13,    16,    17,    18,
      19,    20,    21,    27,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    45,    46,    47,    48,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    67,    68,
      69,    72,    73,    74,    75,    78,    79,    85,    86,    89,
      90,    91,    92,    93,    94,   106,   107,   108,   110,   111,
     112,   113,   114,   117,   133,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   168,   169,   174,   175,   176,   177,
     207,   208,   209,   210,   211,   212,   213,   214,   217,   218,
     219,   221,   222,   226,   227,   228,   229,   230,   231,   233,
     236,   239,   247,   249,   250,   251,   280,   281,   284,   285,
     131,    49,   251,     1,   251,     1,   251,   252,    14,    15,
     129,     1,    39,    40,    72,    73,    78,    79,    85,   172,
     242,   243,   244,   289,    50,    77,   178,   204,   251,   251,
     293,   129,   251,     1,    73,   296,   296,   293,   293,   165,
       1,    70,   224,   280,   286,   290,    73,   117,   294,   295,
     251,     1,     1,    51,    52,    53,   293,     1,     4,    22,
      23,    24,    25,    29,    47,    80,    81,   104,   105,   223,
     225,     1,    28,   286,   293,   293,     1,   129,     1,    25,
      79,   240,   242,   243,   244,    25,    82,   248,   251,   294,
     296,   129,   293,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    67,    68,    69,    89,    90,    91,    92,    93,
      94,   106,   107,   108,   110,   111,   112,   113,   114,   282,
     283,   282,   131,    49,   279,   291,   293,   170,     1,    41,
      43,    71,    84,   245,     1,   245,   171,     1,    67,   200,
      10,    11,   206,   108,    67,   200,   130,     1,     1,     1,
      25,    26,    72,    73,    79,   117,   166,   179,   182,   290,
     119,   290,    66,    72,   108,   110,   111,   117,   287,   288,
     289,   295,     1,   294,   251,     1,    25,    54,    55,   220,
     251,   296,     1,   293,   105,   105,    47,   104,     1,   225,
     224,     1,   294,     1,    14,    46,   160,   161,   234,   235,
     251,     1,     1,    42,    44,    88,   245,   246,     1,   246,
     251,     1,     1,     1,   264,   257,   266,   268,   267,   269,
     270,   271,   272,   263,   262,   261,   273,   274,   275,   276,
     277,   278,   259,   260,   265,   253,   254,   255,   256,   258,
     279,     1,   118,   119,   173,   291,     1,    83,   241,     1,
     173,   251,   251,   129,   251,     1,    14,   167,   251,     1,
     183,   184,   185,   186,   190,   191,   192,   200,   251,   293,
     286,   117,   288,   117,   288,   117,   288,   280,   286,   118,
     251,     1,    26,   224,     1,     1,     1,   251,     1,   161,
     235,   238,     1,   241,     1,   251,   251,   251,   251,   251,
     251,   251,   251,   251,   251,   251,   251,   251,   251,   251,
     251,   251,   251,   251,   251,   251,   251,   251,   251,   251,
     251,   292,   118,     1,    73,     1,   200,   200,   125,   128,
     132,   133,   200,   130,   180,   181,   180,     1,   187,   191,
     192,   200,   188,   190,   192,   200,   189,   190,   191,   200,
      41,    95,    96,   118,   180,   280,   280,   280,   118,   294,
     130,     1,   130,    46,   232,     1,   291,   132,   133,   176,
     133,     7,     8,     9,   205,     1,   126,   182,   192,   200,
     191,   200,   192,   200,   190,   200,   191,   200,   190,   200,
       1,   200,     1,   200,     1,   200,   200,   118,   118,   118,
      14,   128,   296,   251,   251,   251,   205,    15,   200,   200,
     200,   200,   200,   200,    97,    98,    99,   100,   101,   102,
     103,   194,   195,    97,   193,   195,   194,   180,   130,     1,
     200,   200,   200,   205,   130,     1,   195,     1,   195,     1,
     198,   199,     1,   196,   197,     1,   198,     1,   198,   132,
       1,   126,   200,   200,    14,   201,   202,   203,   289,   295,
     202
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (0)
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
              (Loc).first_line, (Loc).first_column,	\
              (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
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

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Type, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
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
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);


# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
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
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

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
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  register short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;


  yyvsp[0] = yylval;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

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

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to look-ahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
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
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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
#line 159 "./yaccsrc.y"
    { level = 0;
                                         if ( get_options_flag( parser_data.TSD->currlevel, EXT_CALLS_AS_FUNCS )
                                           && !get_options_flag( parser_data.TSD->currlevel, EXT_STRICT_ANSI ) )
                                            start_parendepth = 1;
                                         else
                                            start_parendepth = 0;
                                         parendepth = 0; ;}
    break;

  case 4:
#line 169 "./yaccsrc.y"
    { (yyval) = optgluelast( (yyvsp[-1]), (yyvsp[0]) );
                                         (yyval)->o.last = NULL;
                                         EndProg( (yyval) ) ; ;}
    break;

  case 5:
#line 172 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]);
                                         if ( (yyval) != NULL )
                                            (yyval)->o.last = NULL;
                                         EndProg( (yyval) ); ;}
    break;

  case 6:
#line 178 "./yaccsrc.y"
    { /* fixes bug 579711 */
                                         (yyval) = optgluelast( (yyvsp[-1]), (yyvsp[0]) ); ;}
    break;

  case 7:
#line 180 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]); ;}
    break;

  case 8:
#line 183 "./yaccsrc.y"
    { /* fixes bug 579711 */
                                         (yyval) = optgluelast( (yyvsp[-1]), (yyvsp[0]) ); ;}
    break;

  case 9:
#line 185 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]); ;}
    break;

  case 10:
#line 188 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 11:
#line 189 "./yaccsrc.y"
    { exiterror( ERR_UNMATCHED_END, 1 ); ;}
    break;

  case 13:
#line 197 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]); ;}
    break;

  case 14:
#line 198 "./yaccsrc.y"
    { (yyval) = NULL; ;}
    break;

  case 15:
#line 201 "./yaccsrc.y"
    { (yyval) = optgluelast( (yyvsp[-2]), (yyvsp[0]) ); ;}
    break;

  case 16:
#line 202 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]); ;}
    break;

  case 17:
#line 205 "./yaccsrc.y"
    { (yyval) = optgluelast( (yyvsp[-1]), (yyvsp[0]) ); ;}
    break;

  case 18:
#line 206 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]); ;}
    break;

  case 19:
#line 209 "./yaccsrc.y"
    { (yyval) = optgluelast( (yyvsp[-1]), (yyvsp[0]) ); ;}
    break;

  case 20:
#line 210 "./yaccsrc.y"
    { (yyval) = NULL; ;}
    break;

  case 23:
#line 217 "./yaccsrc.y"
    { tmpchr=parser_data.tstart;
                                         tmplno=parser_data.tline; ;}
    break;

  case 24:
#line 221 "./yaccsrc.y"
    { (yyval)=(yyvsp[0]); ;}
    break;

  case 25:
#line 224 "./yaccsrc.y"
    { (yyval) = optgluelast( (yyvsp[-1]), (yyvsp[0]) ); ;}
    break;

  case 54:
#line 257 "./yaccsrc.y"
    { (yyval) = makenode(X_CALL,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 55:
#line 261 "./yaccsrc.y"
    { (yyval) = makenode(X_DO,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         level++; ;}
    break;

  case 56:
#line 266 "./yaccsrc.y"
    { (yyval) = makenode(X_EXIT,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 57:
#line 270 "./yaccsrc.y"
    { (yyval) = makenode(X_IF,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         level++; ;}
    break;

  case 58:
#line 275 "./yaccsrc.y"
    { (yyval) = makenode(X_ITERATE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 59:
#line 279 "./yaccsrc.y"
    { (yyval) = makenode(X_LEAVE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 60:
#line 283 "./yaccsrc.y"
    { (yyval) = makenode(X_SAY,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 61:
#line 287 "./yaccsrc.y"
    { (yyval) = makenode(X_RETURN,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 62:
#line 291 "./yaccsrc.y"
    { (yyval) = makenode(X_ADDR_N,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 63:
#line 295 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_ARG,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 64:
#line 299 "./yaccsrc.y"
    { (yyval) = makenode(X_DROP,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 65:
#line 303 "./yaccsrc.y"
    { (yyval) = makenode(X_IPRET,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 66:
#line 307 "./yaccsrc.y"
    { (yyval) = makenode(X_LABEL,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 67:
#line 311 "./yaccsrc.y"
    { (yyval) = makenode(X_NULL,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 68:
#line 315 "./yaccsrc.y"
    { (yyval) = makenode(0,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 69:
#line 319 "./yaccsrc.y"
    { (yyval) = makenode(X_OPTIONS,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 70:
#line 323 "./yaccsrc.y"
    { (yyval) = makenode(0,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 71:
#line 327 "./yaccsrc.y"
    { (yyval) = makenode(X_PROC,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 72:
#line 331 "./yaccsrc.y"
    { (yyval) = makenode(X_PULL,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 73:
#line 335 "./yaccsrc.y"
    { (yyval) = makenode(X_PUSH,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 74:
#line 339 "./yaccsrc.y"
    { (yyval) = makenode(X_QUEUE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 75:
#line 343 "./yaccsrc.y"
    { (yyval) = makenode(X_SELECT,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         level++; ;}
    break;

  case 76:
#line 348 "./yaccsrc.y"
    { (yyval) = makenode(X_SIG_LAB,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 77:
#line 352 "./yaccsrc.y"
    { (yyval) = makenode(X_WHEN,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 78:
#line 356 "./yaccsrc.y"
    { (yyval) = makenode(X_OTHERWISE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 79:
#line 360 "./yaccsrc.y"
    { (yyval) = makenode(X_TRACE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 80:
#line 364 "./yaccsrc.y"
    { (yyval) = makenode(X_UPPER_VAR,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 81:
#line 368 "./yaccsrc.y"
    { (yyval) = current = (yyvsp[0]) ; ;}
    break;

  case 83:
#line 372 "./yaccsrc.y"
    { current->type = X_ADDR_V ;
                                         current->p[0] = (yyvsp[-1]) ;
                                         current->p[1] = (yyvsp[0]) ; ;}
    break;

  case 84:
#line 375 "./yaccsrc.y"
    { exiterror( ERR_STRING_EXPECTED, 1, __reginatext ) ;;}
    break;

  case 85:
#line 376 "./yaccsrc.y"
    { current->type = X_ADDR_S ; ;}
    break;

  case 86:
#line 377 "./yaccsrc.y"
    { exiterror( ERR_STRING_EXPECTED, 1, __reginatext ) ;;}
    break;

  case 88:
#line 379 "./yaccsrc.y"
    { current->name = (streng *)(yyvsp[-2]) ;
                                         current->type = X_ADDR_N ;
                                         current->p[0] = (yyvsp[-1]) ;
                                         current->p[1] = (yyvsp[0]) ; ;}
    break;

  case 89:
#line 383 "./yaccsrc.y"
    { current->type = X_ADDR_V ;
                                         current->p[0] = (yyvsp[-3]) ;
                                         current->p[1] = (yyvsp[0]) ;
                                         current->u.nonansi = 1 ; ;}
    break;

  case 90:
#line 389 "./yaccsrc.y"
    { (yyval) = makenode( X_PARSE, 2, (yyvsp[-1]), (yyvsp[0]) );
                                         (yyval)->u.parseflags = PARSE_UPPER;
                                         (yyval)->lineno = (yyvsp[-1])->lineno;
                                         (yyval)->charnr = (yyvsp[-1])->charnr; ;}
    break;

  case 91:
#line 395 "./yaccsrc.y"
    { parendepth = start_parendepth; ;}
    break;

  case 92:
#line 396 "./yaccsrc.y"
    { (yyval) = (yyvsp[-3]);
                                         (yyval)->p[0] = (yyvsp[0]);
                                         (yyval)->name = (streng *) (yyvsp[-2]);
                                         parendepth = 0; ;}
    break;

  case 93:
#line 400 "./yaccsrc.y"
    { parendepth = start_parendepth; ;}
    break;

  case 94:
#line 401 "./yaccsrc.y"
    { (yyval) = (yyvsp[-3]);
                                         (yyval)->type = X_EX_FUNC;
                                         (yyval)->p[0] = (yyvsp[0]);
                                         (yyval)->name = (streng *) (yyvsp[-2]);
                                         parendepth = 0; ;}
    break;

  case 95:
#line 406 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 1, "ERROR FAILURE HALT NOTREADY", __reginatext ) ;;}
    break;

  case 96:
#line 407 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 2, "ERROR FAILURE HALT NOTREADY", __reginatext ) ;;}
    break;

  case 97:
#line 408 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;;}
    break;

  case 98:
#line 409 "./yaccsrc.y"
    { exiterror( ERR_STRING_EXPECTED, 3, __reginatext ) ;;}
    break;

  case 99:
#line 411 "./yaccsrc.y"
    { (yyval) = (yyvsp[-3]) ;
                                         (yyval)->type = X_CALL_SET ;
                                         (yyval)->p[0] = (yyvsp[-2]) ;
                                         (yyval)->name = (streng *)(yyvsp[0]) ;
                                         (yyval)->p[1] = (yyvsp[-1]) ; ;}
    break;

  case 100:
#line 416 "./yaccsrc.y"
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_CALL_SET ;
                                         (yyval)->p[0] = (yyvsp[-1]) ;
                                         (yyval)->name = NULL ;
                                         (yyval)->p[1] = (yyvsp[0]) ; ;}
    break;

  case 101:
#line 421 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;;}
    break;

  case 102:
#line 422 "./yaccsrc.y"
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_CALL_SET ;
                                         (yyval)->p[0] = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ; ;}
    break;

  case 103:
#line 428 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]); ;}
    break;

  case 104:
#line 429 "./yaccsrc.y"
    { exiterror( ERR_STRING_EXPECTED, 2, __reginatext );;}
    break;

  case 105:
#line 432 "./yaccsrc.y"
    {
                                         /*
                                          * "call_args" accepted probably with
                                          * surrounding parentheses. Strip them.
                                          */
                                         (yyval) = reduce_expr_list( (yyvsp[0]),
                                                                REDUCE_CALL );
                                       ;}
    break;

  case 106:
#line 440 "./yaccsrc.y"
    { exiterror(ERR_UNEXPECTED_PARAN, 2); ;}
    break;

  case 107:
#line 443 "./yaccsrc.y"
    { (yyval) = makenode(X_COMMAND,0) ;
                                         (yyval)->charnr = tmpchr ;
                                         (yyval)->lineno = tmplno;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 108:
#line 449 "./yaccsrc.y"
    { (yyval) = makenode(X_END,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         level--; ;}
    break;

  case 109:
#line 455 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->name = (streng*)((yyvsp[0])) ; ;}
    break;

  case 110:
#line 457 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 111:
#line 458 "./yaccsrc.y"
    {  exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;;}
    break;

  case 112:
#line 462 "./yaccsrc.y"
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
                                       ;}
    break;

  case 113:
#line 483 "./yaccsrc.y"
    { (yyval) =makenode(X_REP,4,(yyvsp[-4]),(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0])) ;
                                         (yyval)->name = (streng *)(yyvsp[-6]) ;
                                         checkdosyntax((yyval)) ; ;}
    break;

  case 114:
#line 487 "./yaccsrc.y"
    { (yyval) =makenode(X_REP,3,(yyvsp[-3]),(yyvsp[-1]),(yyvsp[0])) ;
                                         (yyval)->name = (streng *)(yyvsp[-5]) ;
                                         checkdosyntax((yyval)) ; ;}
    break;

  case 115:
#line 491 "./yaccsrc.y"
    { (yyval) = makenode(X_REP,2,(yyvsp[-2]),(yyvsp[0])) ;
                                         (yyval)->name = (streng *)(yyvsp[-4]) ;
                                         checkdosyntax((yyval)) ; ;}
    break;

  case 116:
#line 494 "./yaccsrc.y"
    { (yyval) = makenode(X_REP,1,(yyvsp[-1])) ;
                                         (yyval)->name = (streng *)(yyvsp[-3]) ;
                                         checkdosyntax((yyval)) ; ;}
    break;

  case 117:
#line 497 "./yaccsrc.y"
    { (yyval) = makenode(X_REP_FOREVER,0) ; ;}
    break;

  case 118:
#line 498 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 16, "WHILE UNTIL", __reginatext ) ; ;}
    break;

  case 119:
#line 499 "./yaccsrc.y"
    { (yyvsp[-1]) = makenode(X_DO_EXPR,1,(yyvsp[-1])) ;
                                         (yyval) = makenode(X_REP,2,NULL,(yyvsp[-1])) ; ;}
    break;

  case 120:
#line 501 "./yaccsrc.y"
    { (yyval) = NULL ; ;}
    break;

  case 121:
#line 504 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; ;}
    break;

  case 122:
#line 505 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; ;}
    break;

  case 123:
#line 506 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; ;}
    break;

  case 124:
#line 509 "./yaccsrc.y"
    { SymbolDetect |= SD_ADDRWITH ;
                                         (yyval) = with = makenode(X_ADDR_WITH,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 125:
#line 513 "./yaccsrc.y"
    { with = NULL ;
                                         SymbolDetect &= ~SD_ADDRWITH ; ;}
    break;

  case 126:
#line 515 "./yaccsrc.y"
    { (yyval) = NULL ; ;}
    break;

  case 127:
#line 518 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]); ;}
    break;

  case 128:
#line 519 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 5, __reginatext ) ; ;}
    break;

  case 129:
#line 520 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 5, __reginatext ) ; ;}
    break;

  case 133:
#line 526 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 5, __reginatext ) ; ;}
    break;

  case 152:
#line 562 "./yaccsrc.y"
    { with->p[0] = (yyvsp[0]); ;}
    break;

  case 153:
#line 563 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 6, __reginatext ) ; ;}
    break;

  case 154:
#line 566 "./yaccsrc.y"
    { with->p[1] = (yyvsp[0]); ;}
    break;

  case 155:
#line 567 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 7, __reginatext ) ; ;}
    break;

  case 156:
#line 570 "./yaccsrc.y"
    { with->p[2] = (yyvsp[0]); ;}
    break;

  case 157:
#line 571 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 14, __reginatext ) ; ;}
    break;

  case 158:
#line 574 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 159:
#line 575 "./yaccsrc.y"
    { (yyval) = makenode(X_ADDR_WITH, 0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 160:
#line 580 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 161:
#line 581 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ;
                                         (yyval)->u.of.append = 1 ; ;}
    break;

  case 162:
#line 583 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 8, __reginatext ) ; ;}
    break;

  case 163:
#line 584 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 164:
#line 585 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 9, __reginatext ) ; ;}
    break;

  case 165:
#line 586 "./yaccsrc.y"
    { (yyval) = makenode(X_ADDR_WITH, 0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 166:
#line 591 "./yaccsrc.y"
    { /* ANSI extension: nsimsymb is
                                          * used by the standard, but I think
                                          * there are no reasons why using
                                          * it here as a must. FGC
                                          */
                                         (yyval) = (yyvsp[0]) ;
                                         (yyval)->u.of.awt = awtSTREAM;
                                         SymbolDetect |= SD_ADDRWITH ; ;}
    break;

  case 167:
#line 599 "./yaccsrc.y"
    { exiterror( ERR_INVALID_OPTION, 1, __reginatext ) ; ;}
    break;

  case 168:
#line 600 "./yaccsrc.y"
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
                                         SymbolDetect |= SD_ADDRWITH ; ;}
    break;

  case 169:
#line 613 "./yaccsrc.y"
    { exiterror( ERR_INVALID_OPTION, 2, __reginatext ) ; ;}
    break;

  case 170:
#line 614 "./yaccsrc.y"
    {
                                         (yyval) = (yyvsp[0]) ;
                                         (yyval)->u.of.awt = awtLIFO ;
                                         SymbolDetect |= SD_ADDRWITH ; ;}
    break;

  case 171:
#line 618 "./yaccsrc.y"
    { exiterror( ERR_INVALID_OPTION, 100, __reginatext ) ; ;}
    break;

  case 172:
#line 619 "./yaccsrc.y"
    {
                                         (yyval) = (yyvsp[0]) ;
                                         (yyval)->u.of.awt = awtFIFO ;
                                         SymbolDetect |= SD_ADDRWITH ; ;}
    break;

  case 173:
#line 623 "./yaccsrc.y"
    { exiterror( ERR_INVALID_OPTION, 101, __reginatext ) ; ;}
    break;

  case 174:
#line 626 "./yaccsrc.y"
    { SymbolDetect &= ~SD_ADDRWITH ; ;}
    break;

  case 175:
#line 627 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 176:
#line 630 "./yaccsrc.y"
    { SymbolDetect &= ~SD_ADDRWITH ; ;}
    break;

  case 177:
#line 631 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 180:
#line 638 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 181:
#line 639 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 182:
#line 642 "./yaccsrc.y"
    { (yyval) = makenode(X_ADDR_WITH, 0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->u.of.ant = antSIMSYMBOL;
                                       ;}
    break;

  case 183:
#line 650 "./yaccsrc.y"
    { (yyval) = makenode(X_ADDR_WITH, 0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->u.of.ant = antSTRING;
                                       ;}
    break;

  case 184:
#line 658 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; ;}
    break;

  case 185:
#line 661 "./yaccsrc.y"
    { (yyval) = makenode(X_DO_TO,1,(yyvsp[-1])) ; ;}
    break;

  case 186:
#line 662 "./yaccsrc.y"
    { (yyval) = makenode(X_DO_FOR,1,(yyvsp[-1])) ; ;}
    break;

  case 187:
#line 663 "./yaccsrc.y"
    { (yyval) = makenode(X_DO_BY,1,(yyvsp[-1])) ; ;}
    break;

  case 188:
#line 666 "./yaccsrc.y"
    { (yyval) = makenode(X_WHILE,1,(yyvsp[-1])) ; ;}
    break;

  case 189:
#line 667 "./yaccsrc.y"
    { (yyval) = makenode(X_UNTIL,1,(yyvsp[-1])) ; ;}
    break;

  case 190:
#line 668 "./yaccsrc.y"
    { (yyval) = NULL ; ;}
    break;

  case 191:
#line 671 "./yaccsrc.y"
    { exiterror( ERR_SYMBOL_EXPECTED, 1, __reginatext ) ;;}
    break;

  case 192:
#line 672 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 193:
#line 676 "./yaccsrc.y"
    { exiterror( ERR_SYMBOL_EXPECTED, 1, __reginatext ) ;;}
    break;

  case 194:
#line 677 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 195:
#line 681 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 196:
#line 686 "./yaccsrc.y"
    { move_labels( (yyvsp[-5]), (yyvsp[0]), level - 1 );
                                         (yyval) = (yyvsp[-5]);
                                         (yyval)->p[0] = optgluelast( (yyvsp[-4]), (yyvsp[-3]) );
                                         (yyval)->p[0]->o.last = NULL;
                                         (yyval)->p[1] = optgluelast( (yyvsp[-1]), (yyvsp[0]) );
                                         (yyval)->p[1]->o.last = NULL;
                                         level--; ;}
    break;

  case 197:
#line 694 "./yaccsrc.y"
    { move_labels( (yyvsp[-8]), (yyvsp[0]), level - 1 );
                                         (yyval) = (yyvsp[-8]);
                                         (yyval)->p[0] = optgluelast( (yyvsp[-7]), (yyvsp[-6]) );
                                         (yyval)->p[0]->o.last = NULL;
                                         (yyval)->p[1] = optgluelast( (yyvsp[-4]), (yyvsp[-3]) );
                                         (yyval)->p[1]->o.last = NULL;
                                         (yyval)->p[2] = optgluelast( (yyvsp[-1]), (yyvsp[0]) );
                                         (yyval)->p[2]->o.last = NULL;
                                         level--; ;}
    break;

  case 198:
#line 704 "./yaccsrc.y"
    {  exiterror( ERR_INCOMPLETE_STRUCT, 4 ) ;;}
    break;

  case 199:
#line 706 "./yaccsrc.y"
    {  exiterror( ERR_INCOMPLETE_STRUCT, 3 ) ;;}
    break;

  case 200:
#line 707 "./yaccsrc.y"
    {  exiterror( ERR_INCOMPLETE_STRUCT, 0 ) ;;}
    break;

  case 201:
#line 708 "./yaccsrc.y"
    {  exiterror( ERR_THEN_EXPECTED, 1, parser_data.if_linenr, __reginatext )  ; ;}
    break;

  case 202:
#line 711 "./yaccsrc.y"
    {  exiterror( ERR_THEN_UNEXPECTED, 1 )  ; ;}
    break;

  case 203:
#line 714 "./yaccsrc.y"
    {  exiterror( ERR_THEN_UNEXPECTED, 2 )  ; ;}
    break;

  case 204:
#line 717 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 205:
#line 722 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; ;}
    break;

  case 206:
#line 724 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;;}
    break;

  case 207:
#line 725 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 208:
#line 728 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ;
                                         (yyval)->u.trace_only =
                                                         (level == 0) ? 0 : 1;
                                         newlabel( (const tsd_t *)parser_data.TSD,
                                                   &parser_data,
                                                   (yyvsp[0]) ) ; ;}
    break;

  case 209:
#line 736 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ;
                                         (yyval)->name = Str_cre_TSD(parser_data.TSD,retvalue) ; ;}
    break;

  case 210:
#line 740 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; ;}
    break;

  case 211:
#line 742 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;;}
    break;

  case 212:
#line 743 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 213:
#line 746 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 214:
#line 747 "./yaccsrc.y"
    {  exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;;}
    break;

  case 215:
#line 750 "./yaccsrc.y"
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_NUM_D ;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 216:
#line 753 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]); (yyval)->type = X_NUM_DDEF ; ;}
    break;

  case 217:
#line 754 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;;}
    break;

  case 218:
#line 755 "./yaccsrc.y"
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_NUM_F ;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 219:
#line 758 "./yaccsrc.y"
    { /* NOTE! This clashes ANSI! */
                                         (yyval) = (yyvsp[-1]) ; (yyval)->type=X_NUM_FRMDEF ;;}
    break;

  case 220:
#line 760 "./yaccsrc.y"
    { (yyval) = (yyvsp[-3]) ; (yyval)->type=X_NUM_V ;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 221:
#line 762 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 11, "ENGINEERING SCIENTIFIC", __reginatext ) ;;}
    break;

  case 222:
#line 763 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]); (yyval)->type = X_NUM_FDEF ;;}
    break;

  case 223:
#line 764 "./yaccsrc.y"
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_NUM_FUZZ ;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 224:
#line 767 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 15, "DIGITS FORM FUZZ", __reginatext ) ;;}
    break;

  case 225:
#line 770 "./yaccsrc.y"
    { (yyval) = makenode(X_NUM_SCI,0) ; ;}
    break;

  case 226:
#line 771 "./yaccsrc.y"
    { (yyval) = makenode(X_NUM_ENG,0) ; ;}
    break;

  case 227:
#line 774 "./yaccsrc.y"
    { ((yyval)=(yyvsp[-1]))->p[0]=(yyvsp[0]) ; ;}
    break;

  case 228:
#line 778 "./yaccsrc.y"
    { (yyval) = (yyvsp[-3]) ;
                                         (yyval)->type = X_PARSE ;
                                         (yyval)->u.parseflags = (int) (yyvsp[-2]) ;
                                         (yyval)->p[0] = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ; ;}
    break;

  case 229:
#line 784 "./yaccsrc.y"
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_PARSE ;
                                         (yyval)->u.parseflags = 0;
                                         (yyval)->p[0] = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ; ;}
    break;

  case 230:
#line 789 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 12, "ARG EXTERNAL LINEIN PULL SOURCE VAR VALUE VERSION", __reginatext ) ;;}
    break;

  case 231:
#line 790 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 12, "ARG CASELESS EXTERNAL LINEIN LOWER PULL SOURCE UPPER VAR VALUE VERSION", __reginatext ) ;;}
    break;

  case 232:
#line 793 "./yaccsrc.y"
    { (yyval) = (nodeptr) (PARSE_UPPER  |
                                                         PARSE_NORMAL); ;}
    break;

  case 233:
#line 795 "./yaccsrc.y"
    { (yyval) = (nodeptr) (PARSE_UPPER  |
                                                         PARSE_CASELESS); ;}
    break;

  case 234:
#line 797 "./yaccsrc.y"
    { (yyval) = (nodeptr) (PARSE_UPPER  |
                                                         PARSE_CASELESS); ;}
    break;

  case 235:
#line 799 "./yaccsrc.y"
    { (yyval) = (nodeptr) (PARSE_LOWER  |
                                                         PARSE_NORMAL); ;}
    break;

  case 236:
#line 801 "./yaccsrc.y"
    { (yyval) = (nodeptr) (PARSE_LOWER  |
                                                         PARSE_CASELESS); ;}
    break;

  case 237:
#line 803 "./yaccsrc.y"
    { (yyval) = (nodeptr) (PARSE_LOWER  |
                                                         PARSE_CASELESS); ;}
    break;

  case 238:
#line 805 "./yaccsrc.y"
    { (yyval) = (nodeptr) (PARSE_NORMAL |
                                                         PARSE_CASELESS); ;}
    break;

  case 239:
#line 809 "./yaccsrc.y"
    { /* fixes bugs like bug 579711 */
                                         (yyval) = optgluelast( (yyvsp[-2]), (yyvsp[0]) ); ;}
    break;

  case 240:
#line 811 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 241:
#line 814 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_ARG,0) ; ;}
    break;

  case 242:
#line 815 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_EXT,0) ; ;}
    break;

  case 243:
#line 816 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_EXT,0) ; ;}
    break;

  case 244:
#line 817 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_VER,0) ; ;}
    break;

  case 245:
#line 818 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_PULL,0) ; ;}
    break;

  case 246:
#line 819 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_SRC,0) ; ;}
    break;

  case 247:
#line 820 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_VAR,0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; ;}
    break;

  case 248:
#line 822 "./yaccsrc.y"
    { (yyval) = makenode(X_PARSE_VAL,1,(yyvsp[-1])) ; ;}
    break;

  case 249:
#line 823 "./yaccsrc.y"
    { exiterror( ERR_INVALID_TEMPLATE, 3 ) ;;}
    break;

  case 250:
#line 826 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 251:
#line 827 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 17, __reginatext ) ;;}
    break;

  case 252:
#line 828 "./yaccsrc.y"
    { exiterror( ERR_SYMBOL_EXPECTED, 1, __reginatext ) ;;}
    break;

  case 253:
#line 829 "./yaccsrc.y"
    { exiterror( ERR_SYMBOL_EXPECTED, 1, __reginatext ) ;;}
    break;

  case 254:
#line 830 "./yaccsrc.y"
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 255:
#line 834 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 256:
#line 838 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 257:
#line 842 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 258:
#line 846 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 259:
#line 850 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 260:
#line 854 "./yaccsrc.y"
    { exiterror( ERR_UNMATCHED_END, 0 ) ;;}
    break;

  case 261:
#line 855 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;;}
    break;

  case 262:
#line 856 "./yaccsrc.y"
    { level--; ;}
    break;

  case 263:
#line 860 "./yaccsrc.y"
    { (yyval) = (yyvsp[-4]);
                                         (yyval)->p[0] = optgluelast( (yyvsp[-3]), (yyvsp[-2]) );
                                         (yyval)->p[0]->o.last = NULL;
                                         (yyval)->p[1] = (yyvsp[-1]); ;}
    break;

  case 264:
#line 864 "./yaccsrc.y"
    {  exiterror( ERR_WHEN_EXPECTED, 0 ) ;;}
    break;

  case 265:
#line 866 "./yaccsrc.y"
    {  exiterror( ERR_WHEN_EXPECTED, 0 ) ;;}
    break;

  case 266:
#line 867 "./yaccsrc.y"
    {  exiterror( ERR_EXTRA_DATA, 1, __reginatext )  ;;}
    break;

  case 267:
#line 868 "./yaccsrc.y"
    {  exiterror( ERR_THEN_UNEXPECTED, 0 ) ;;}
    break;

  case 268:
#line 870 "./yaccsrc.y"
    {  exiterror( ERR_INCOMPLETE_STRUCT, 0 ) ;;}
    break;

  case 269:
#line 873 "./yaccsrc.y"
    { (yyval) = optgluelast( (yyvsp[-1]), (yyvsp[0]) ); ;}
    break;

  case 270:
#line 874 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 271:
#line 875 "./yaccsrc.y"
    {  exiterror( ERR_WHEN_EXPECTED, 0 )  ;;}
    break;

  case 272:
#line 879 "./yaccsrc.y"
    { (yyval) = (yyvsp[-5]); /* fixes bugs like bug 579711 */
                                         (yyval)->p[0] = optgluelast( (yyvsp[-4]), (yyvsp[-3]) );
                                         (yyval)->p[0]->o.last = NULL;
                                         (yyval)->p[1] = optgluelast( (yyvsp[-1]), (yyvsp[0]) );
                                         (yyval)->p[1]->o.last = NULL; ;}
    break;

  case 273:
#line 885 "./yaccsrc.y"
    {  exiterror( ERR_THEN_UNEXPECTED, 0 ) ;;}
    break;

  case 274:
#line 886 "./yaccsrc.y"
    {  exiterror( ERR_THEN_EXPECTED, 2, parser_data.when_linenr, __reginatext )  ; ;}
    break;

  case 275:
#line 887 "./yaccsrc.y"
    {  exiterror( ERR_INVALID_EXPRESSION, 0 ) ;;}
    break;

  case 278:
#line 894 "./yaccsrc.y"
    { exiterror( ERR_WHEN_UNEXPECTED, 0 ); ;}
    break;

  case 279:
#line 897 "./yaccsrc.y"
    { (yyval) = (yyvsp[-2]);
                                         (yyval)->p[0] = optgluelast( (yyvsp[-1]), (yyvsp[0]) );
                                         if ( (yyval)->p[0] )
                                            (yyval)->p[0]->o.last = NULL; ;}
    break;

  case 280:
#line 901 "./yaccsrc.y"
    { (yyval) = makenode(X_NO_OTHERWISE,0) ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->charnr = parser_data.tstart ; ;}
    break;

  case 281:
#line 907 "./yaccsrc.y"
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_SIG_VAL ;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 282:
#line 910 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;;}
    break;

  case 283:
#line 911 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->name = (streng *)(yyvsp[0]) ; ;}
    break;

  case 284:
#line 913 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 3, "ERROR FAILURE HALT NOTREADY NOVALUE SYNTAX LOSTDIGITS", __reginatext ) ;;}
    break;

  case 285:
#line 914 "./yaccsrc.y"
    { exiterror( ERR_INV_SUBKEYWORD, 4, "ERROR FAILURE HALT NOTREADY NOVALUE SYNTAX LOSTDIGITS", __reginatext ) ;;}
    break;

  case 286:
#line 915 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;;}
    break;

  case 287:
#line 916 "./yaccsrc.y"
    { exiterror( ERR_STRING_EXPECTED, 3, __reginatext ) ;;}
    break;

  case 288:
#line 918 "./yaccsrc.y"
    { (yyval) = (yyvsp[-3]) ;
                                         (yyval)->type = X_SIG_SET ;
                                         (yyval)->p[0] = (yyvsp[-2]) ;
                                         (yyval)->name = (streng *)(yyvsp[0]) ;
                                         (yyval)->p[1] = (yyvsp[-1]) ; ;}
    break;

  case 289:
#line 923 "./yaccsrc.y"
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_SIG_SET ;
                                         (yyval)->p[0] = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ; ;}
    break;

  case 290:
#line 927 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;;}
    break;

  case 291:
#line 928 "./yaccsrc.y"
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->type = X_SIG_SET ;
                                         (yyval)->p[0] = (yyvsp[-1]) ;
                                         (yyval)->p[1] = (yyvsp[0]) ; ;}
    break;

  case 292:
#line 934 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]); ;}
    break;

  case 293:
#line 935 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; ;}
    break;

  case 294:
#line 936 "./yaccsrc.y"
    { exiterror( ERR_STRING_EXPECTED, 4, __reginatext );;}
    break;

  case 295:
#line 939 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue);;}
    break;

  case 296:
#line 940 "./yaccsrc.y"
    { exiterror( ERR_STRING_EXPECTED, 3, __reginatext ) ;;}
    break;

  case 297:
#line 943 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; ;}
    break;

  case 298:
#line 944 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; ;}
    break;

  case 299:
#line 947 "./yaccsrc.y"
    { (yyval) = makenode(X_ON,0) ; ;}
    break;

  case 300:
#line 950 "./yaccsrc.y"
    { (yyval) = makenode(X_OFF,0) ; ;}
    break;

  case 301:
#line 953 "./yaccsrc.y"
    { (yyval) = makenode(X_S_ERROR,0) ; ;}
    break;

  case 302:
#line 954 "./yaccsrc.y"
    { (yyval) = makenode(X_S_HALT,0) ; ;}
    break;

  case 303:
#line 955 "./yaccsrc.y"
    { (yyval) = makenode(X_S_NOTREADY,0) ; ;}
    break;

  case 304:
#line 956 "./yaccsrc.y"
    { (yyval) = makenode(X_S_FAILURE,0) ; ;}
    break;

  case 305:
#line 959 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 306:
#line 960 "./yaccsrc.y"
    { (yyval) = makenode(X_S_NOVALUE,0) ; ;}
    break;

  case 307:
#line 961 "./yaccsrc.y"
    { (yyval) = makenode(X_S_SYNTAX,0) ; ;}
    break;

  case 308:
#line 962 "./yaccsrc.y"
    { (yyval) = makenode(X_S_LOSTDIGITS,0) ; ;}
    break;

  case 309:
#line 965 "./yaccsrc.y"
    { (yyval) = (yyvsp[-2]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 310:
#line 967 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->p[0] = (yyvsp[0]) ; ;}
    break;

  case 311:
#line 969 "./yaccsrc.y"
    { exiterror( ERR_EXTRA_DATA, 1, __reginatext ) ;;}
    break;

  case 312:
#line 970 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; ;}
    break;

  case 313:
#line 974 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; ;}
    break;

  case 314:
#line 978 "./yaccsrc.y"
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
                                            (yyval)->type = X_NASSIGN ; ;}
    break;

  case 315:
#line 994 "./yaccsrc.y"
    { (yyval) = makenode(X_ASSIGN,0) ;
                                         (yyval)->charnr = parser_data.tstart ;
                                         (yyval)->lineno = parser_data.tline ;
                                         (yyval)->p[0] = (nodeptr)create_head( (const char *)retvalue ); ;}
    break;

  case 316:
#line 1001 "./yaccsrc.y"
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
                                         parendepth--; ;}
    break;

  case 317:
#line 1017 "./yaccsrc.y"
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
                                       ;}
    break;

  case 318:
#line 1034 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 319:
#line 1035 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "+" );
                                         (yyval) = makenode( X_PLUSS, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 320:
#line 1037 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 321:
#line 1038 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "-" );
                                         (yyval) = makenode( X_MINUS, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 322:
#line 1040 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 323:
#line 1041 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "*" );
                                         (yyval) = makenode( X_MULT, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 324:
#line 1043 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "*" ); ;}
    break;

  case 325:
#line 1044 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 326:
#line 1045 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "/" );
                                         (yyval) = makenode( X_DEVIDE, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 327:
#line 1047 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "/" ); ;}
    break;

  case 328:
#line 1048 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 329:
#line 1049 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "//" );
                                         (yyval) = makenode( X_MODULUS, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 330:
#line 1051 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "//" ); ;}
    break;

  case 331:
#line 1052 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 332:
#line 1053 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "%" );
                                         (yyval) = makenode( X_INTDIV, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 333:
#line 1055 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "%" ); ;}
    break;

  case 334:
#line 1056 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 335:
#line 1057 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "|" );
                                         (yyval) = makenode( X_LOG_OR, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 336:
#line 1059 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "|" ); ;}
    break;

  case 337:
#line 1060 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 338:
#line 1061 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "&" );
                                         (yyval) = makenode( X_LOG_AND, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 339:
#line 1063 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "&" ); ;}
    break;

  case 340:
#line 1064 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 341:
#line 1065 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "&&" );
                                         (yyval) = makenode( X_LOG_XOR, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 342:
#line 1067 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "&&" ); ;}
    break;

  case 343:
#line 1068 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 344:
#line 1069 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "**" );
                                         (yyval) = makenode( X_EXP, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 345:
#line 1071 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "**" ); ;}
    break;

  case 346:
#line 1072 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 347:
#line 1073 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), " " );
                                         (yyval) = makenode( X_SPACE, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 348:
#line 1075 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, " " ); ;}
    break;

  case 349:
#line 1076 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 350:
#line 1077 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "||" );
                                         (yyval) = makenode( X_CONCAT, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 351:
#line 1079 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "||" ); ;}
    break;

  case 352:
#line 1080 "./yaccsrc.y"
    { AUTO_REDUCE( (yyvsp[0]), "\\" );
                                         (yyval) = makenode( X_LOG_NOT, 1, (yyvsp[0]) ); ;}
    break;

  case 353:
#line 1082 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "\\" ); ;}
    break;

  case 354:
#line 1083 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 355:
#line 1084 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "=" );
                                         (yyval) = makenode( X_EQUAL, 2, (yyvsp[-3]), (yyvsp[0]) );
                                         transform( (yyval) ); ;}
    break;

  case 356:
#line 1087 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "=" ); ;}
    break;

  case 357:
#line 1088 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 358:
#line 1089 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), ">=" );
                                         (yyval) = makenode( X_GTE, 2, (yyvsp[-3]), (yyvsp[0]) );
                                         transform( (yyval) ) ; ;}
    break;

  case 359:
#line 1092 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, ">=" ); ;}
    break;

  case 360:
#line 1093 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 361:
#line 1094 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "<=" );
                                         (yyval) = makenode( X_LTE, 2, (yyvsp[-3]), (yyvsp[0]) );
                                         transform( (yyval) ) ; ;}
    break;

  case 362:
#line 1097 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "<=" ); ;}
    break;

  case 363:
#line 1098 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 364:
#line 1099 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), ">" );
                                         (yyval) = makenode( X_GT, 2, (yyvsp[-3]), (yyvsp[0]) );
                                         transform( (yyval) ) ; ;}
    break;

  case 365:
#line 1102 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, ">" ); ;}
    break;

  case 366:
#line 1103 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 367:
#line 1104 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "<" );
                                         (yyval) = makenode( X_LT, 2, (yyvsp[-3]), (yyvsp[0]) );
                                         transform( (yyval) ) ; ;}
    break;

  case 368:
#line 1107 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "<" ); ;}
    break;

  case 369:
#line 1108 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 370:
#line 1109 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "\\=" );
                                         (yyval) = makenode( X_DIFF, 2, (yyvsp[-3]), (yyvsp[0]) );
                                         transform( (yyval) ) ; ;}
    break;

  case 371:
#line 1112 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "\\=" ); ;}
    break;

  case 372:
#line 1113 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 373:
#line 1114 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "==" );
                                         (yyval) = makenode( X_S_EQUAL, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 374:
#line 1116 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "==" ); ;}
    break;

  case 375:
#line 1117 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 376:
#line 1118 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "\\==" );
                                         (yyval) = makenode( X_S_DIFF, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 377:
#line 1120 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "\\==" ); ;}
    break;

  case 378:
#line 1121 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 379:
#line 1122 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), ">>" );
                                         (yyval) = makenode( X_S_GT, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 380:
#line 1124 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, ">>" ); ;}
    break;

  case 381:
#line 1125 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 382:
#line 1126 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "<<" );
                                         (yyval) = makenode( X_S_LT, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 383:
#line 1128 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "<<" ); ;}
    break;

  case 384:
#line 1129 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 385:
#line 1130 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "\\>>" );
                                         (yyval) = makenode( X_S_NGT, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 386:
#line 1132 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "\\>>" ); ;}
    break;

  case 387:
#line 1133 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 388:
#line 1134 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "\\<<" );
                                         (yyval) = makenode( X_S_NLT, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 389:
#line 1136 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "\\<<" ); ;}
    break;

  case 390:
#line 1137 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 391:
#line 1138 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), ">>=" );
                                         (yyval) = makenode( X_S_GTE, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 392:
#line 1140 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, ">>=" ); ;}
    break;

  case 393:
#line 1141 "./yaccsrc.y"
    { parendepth--; ;}
    break;

  case 394:
#line 1142 "./yaccsrc.y"
    { parendepth++; AUTO_REDUCE( (yyvsp[-3]), "<<=" );
                                         (yyval) = makenode( X_S_LTE, 2, (yyvsp[-3]), (yyvsp[0]) ); ;}
    break;

  case 395:
#line 1144 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, "<<=" ); ;}
    break;

  case 396:
#line 1145 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 397:
#line 1146 "./yaccsrc.y"
    { (yyval) = makenode( X_STRING, 0 );
                                         (yyval)->name = Str_cre_TSD(parser_data.TSD,retvalue) ; ;}
    break;

  case 398:
#line 1148 "./yaccsrc.y"
    { (yyval) = makenode( X_STRING, 0 );
                                         (yyval)->name = Str_make_TSD(parser_data.TSD,retlength) ;
                                         memcpy((yyval)->name->value,retvalue,
                                                    (yyval)->name->len=retlength); ;}
    break;

  case 399:
#line 1152 "./yaccsrc.y"
    { (yyval) = makenode( X_STRING, 0 );
                                         (yyval)->name = Str_make_TSD(parser_data.TSD,retlength) ;
                                         memcpy((yyval)->name->value,retvalue,
                                                    (yyval)->name->len=retlength); ;}
    break;

  case 400:
#line 1156 "./yaccsrc.y"
    { (yyval) = makenode( X_STRING, 0 );
                                         (yyval)->name = Str_cre_TSD(parser_data.TSD,retvalue) ; ;}
    break;

  case 401:
#line 1158 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 402:
#line 1159 "./yaccsrc.y"
    { AUTO_REDUCE( (yyvsp[0]), nullptr );
                                         (yyval) = makenode( X_U_PLUSS, 1, (yyvsp[0]) ); ;}
    break;

  case 403:
#line 1161 "./yaccsrc.y"
    { AUTO_REDUCE( (yyvsp[0]), nullptr );
                                         (yyval) = makenode( X_U_MINUS, 1, (yyvsp[0]) ); ;}
    break;

  case 404:
#line 1163 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, __reginatext ); ;}
    break;

  case 405:
#line 1164 "./yaccsrc.y"
    { exiterror( ERR_INVALID_EXPRESSION, 1, __reginatext ); ;}
    break;

  case 406:
#line 1167 "./yaccsrc.y"
    { (yyval) = (yyvsp[-1]); ;}
    break;

  case 407:
#line 1168 "./yaccsrc.y"
    { exiterror( ERR_UNMATCHED_PARAN, 0 ); ;}
    break;

  case 408:
#line 1169 "./yaccsrc.y"
    { exiterror( ERR_UNMATCHED_PARAN, 0 ); ;}
    break;

  case 409:
#line 1172 "./yaccsrc.y"
    { (yyval) = (nodeptr)create_head( (const char *)retvalue ) ; ;}
    break;

  case 410:
#line 1176 "./yaccsrc.y"
    { (yyval) = makenode(X_EX_FUNC,1,(yyvsp[0])) ;
                                         (yyval)->name = (streng *)(yyvsp[-1]) ; ;}
    break;

  case 411:
#line 1178 "./yaccsrc.y"
    { (yyval) = makenode(X_IN_FUNC,1,(yyvsp[0])) ;
                                         (yyval)->name = (streng *)(yyvsp[-1]) ; ;}
    break;

  case 412:
#line 1182 "./yaccsrc.y"
    { /* ugly fake preservs parendepth */
                                         (yyval) = (YYSTYPE) parendepth;
                                         parendepth = 0; ;}
    break;

  case 413:
#line 1185 "./yaccsrc.y"
    { parendepth = (int) (yyval);
                                         (yyval) = (yyvsp[0]); ;}
    break;

  case 414:
#line 1189 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; ;}
    break;

  case 415:
#line 1192 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; ;}
    break;

  case 416:
#line 1195 "./yaccsrc.y"
    { (yyval) =makenode(X_TPL_SOLID,3,(yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));;}
    break;

  case 417:
#line 1196 "./yaccsrc.y"
    { (yyval) =makenode(X_TPL_SOLID,1,(yyvsp[0])) ; ;}
    break;

  case 418:
#line 1197 "./yaccsrc.y"
    { exiterror( ERR_INVALID_TEMPLATE, 1, __reginatext ) ;;}
    break;

  case 419:
#line 1200 "./yaccsrc.y"
    { (yyval) = makenode(X_NEG_OFFS,0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; ;}
    break;

  case 420:
#line 1202 "./yaccsrc.y"
    { (yyval) = makenode(X_POS_OFFS,0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; ;}
    break;

  case 421:
#line 1204 "./yaccsrc.y"
    { (yyval) = makenode(X_ABS_OFFS,0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; ;}
    break;

  case 422:
#line 1206 "./yaccsrc.y"
    { (yyval) = makenode(X_ABS_OFFS,0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; ;}
    break;

  case 423:
#line 1208 "./yaccsrc.y"
    { (yyval) = makenode(X_TPL_VAR,0) ;
                                         (yyval)->p[0] = (yyvsp[-1]) ; ;}
    break;

  case 424:
#line 1210 "./yaccsrc.y"
    { (yyval) = makenode(X_NEG_OFFS,0) ;
                                         (yyval)->p[0] = (yyvsp[-1]) ; ;}
    break;

  case 425:
#line 1212 "./yaccsrc.y"
    { (yyval) = makenode(X_POS_OFFS,0) ;
                                         (yyval)->p[0] = (yyvsp[-1]) ; ;}
    break;

  case 426:
#line 1214 "./yaccsrc.y"
    { (yyval) = makenode(X_ABS_OFFS,0) ;
                                         (yyval)->p[0] = (yyvsp[-1]) ; ;}
    break;

  case 427:
#line 1216 "./yaccsrc.y"
    { (yyval) = makenode(X_TPL_MVE,0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; ;}
    break;

  case 428:
#line 1220 "./yaccsrc.y"
    { (yyval) = (nodeptr)Str_cre_TSD(parser_data.TSD,retvalue) ; ;}
    break;

  case 429:
#line 1221 "./yaccsrc.y"
    { streng *sptr = Str_cre_TSD(parser_data.TSD,retvalue) ;
                                          if (myisnumber(parser_data.TSD, sptr))
                                          {
                                             exiterror( ERR_INVALID_INTEGER, 4, sptr->value ) ;
                                          }
                                          else
                                             exiterror( ERR_INVALID_TEMPLATE, 0 ) ;;}
    break;

  case 430:
#line 1230 "./yaccsrc.y"
    { (yyval) = (nodeptr) Str_cre_TSD(parser_data.TSD,retvalue) ; ;}
    break;

  case 431:
#line 1231 "./yaccsrc.y"
    { streng *sptr = Str_make_TSD(parser_data.TSD,retlength) ;
                                         memcpy(sptr->value,retvalue,
                                                 sptr->len=retlength) ;
                                         (yyval) = (nodeptr) sptr ; ;}
    break;

  case 432:
#line 1235 "./yaccsrc.y"
    { streng *sptr = Str_make_TSD(parser_data.TSD,retlength) ;
                                         memcpy(sptr->value,retvalue,
                                                 sptr->len=retlength) ;
                                         (yyval) = (nodeptr) sptr ; ;}
    break;

  case 433:
#line 1241 "./yaccsrc.y"
    { (yyval) = makenode(X_TPL_POINT,1,(yyvsp[0])) ; ;}
    break;

  case 434:
#line 1242 "./yaccsrc.y"
    { (yyval) = makenode(X_TPL_SYMBOL,2,(yyvsp[0]),(yyvsp[-1])) ; ;}
    break;

  case 435:
#line 1243 "./yaccsrc.y"
    { (yyval) = NULL ; ;}
    break;

  case 436:
#line 1253 "./yaccsrc.y"
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
                                       ;}
    break;

  case 437:
#line 1296 "./yaccsrc.y"
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
                                         checkconst( (yyval) ); ;}
    break;

  case 438:
#line 1333 "./yaccsrc.y"
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
                                        ;}
    break;

  case 439:
#line 1377 "./yaccsrc.y"
    { (yyval) = (yyvsp[0]) ; ;}
    break;

  case 440:
#line 1378 "./yaccsrc.y"
    { (yyval) = NULL ; ;}
    break;

  case 441:
#line 1381 "./yaccsrc.y"
    { (yyval) = makenode(X_SIM_SYMBOL,1,(yyvsp[0])) ;
                                         (yyval)->name = (streng *) (yyvsp[-1]) ; ;}
    break;

  case 442:
#line 1383 "./yaccsrc.y"
    { (yyval) = makenode(X_SIM_SYMBOL,0) ;
                                         (yyval)->name = (streng *) (yyvsp[0]) ; ;}
    break;

  case 443:
#line 1385 "./yaccsrc.y"
    { (yyval) = makenode(X_IND_SYMBOL,1,(yyvsp[0])) ;
                                         (yyval)->name = (streng *) (yyvsp[-2]) ; ;}
    break;

  case 444:
#line 1387 "./yaccsrc.y"
    { (yyval) = makenode(X_IND_SYMBOL,0) ;
                                         (yyval)->name = (streng *) (yyvsp[-1]) ; ;}
    break;

  case 445:
#line 1391 "./yaccsrc.y"
    { (yyval) = (treenode *) Str_cre_TSD(parser_data.TSD,retvalue);;}
    break;

  case 446:
#line 1394 "./yaccsrc.y"
    { (yyval) = (treenode *) Str_cre_TSD(parser_data.TSD,retvalue);;}
    break;

  case 447:
#line 1395 "./yaccsrc.y"
    { exiterror( ERR_SYMBOL_EXPECTED, 1, __reginatext ) ;;}
    break;


    }

/* Line 1037 of yacc.c.  */
#line 4798 "./yaccsrc.c"

  yyvsp -= yylen;
  yyssp -= yylen;


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
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {

		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 yydestruct ("Error: popping",
                             yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  yydestruct ("Error: discarding", yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
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


      yydestruct ("Error: popping", yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token. */
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
  yydestruct ("Error: discarding lookahead",
              yytoken, &yylval);
  yychar = YYEMPTY;
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 1398 "./yaccsrc.y"


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
                             "Please inform Mark Hessling or Florian Coosmann about the\n"
                             "circumstances and this script.\n"
                             "\n"
                             "Many thanks!\n"
                             "email addresses:\n"
                             "m.hessling@qut.edu.au\n"
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

