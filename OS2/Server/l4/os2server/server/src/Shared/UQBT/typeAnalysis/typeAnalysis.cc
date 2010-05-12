#define YY_typeAnalysis_h_included

/*  A Bison++ parser, made from typeAnalysis/typeAnalysis.y  */

 /* with Bison++ version bison++ Version 1.21-8, adapted from GNU bison by coetmeur@icdc.fr
  */


#line 1 "/usr/local/lib/bison.cc"
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Bob Corbett and Richard Stallman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* HEADER SECTION */
#if defined( _MSDOS ) || defined(MSDOS) || defined(__MSDOS__) 
#define __MSDOS_AND_ALIKE
#endif
#if defined(_WINDOWS) && defined(_MSC_VER)
#define __HAVE_NO_ALLOCA
#define __MSDOS_AND_ALIKE
#endif

#ifndef alloca
#if defined( __GNUC__)
#define alloca __builtin_alloca

#elif (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc)  || defined (__sgi)
#include <alloca.h>

#elif defined (__MSDOS_AND_ALIKE)
#include <malloc.h>
#ifndef __TURBOC__
/* MS C runtime lib */
#define alloca _alloca
#endif

#elif defined(_AIX)
#include <malloc.h>
#pragma alloca

#elif defined(__hpux)
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */

#endif /* not _AIX  not MSDOS, or __TURBOC__ or _AIX, not sparc.  */
#endif /* alloca not defined.  */
#ifdef c_plusplus
#ifndef __cplusplus
#define __cplusplus
#endif
#endif
#ifdef __cplusplus
#ifndef YY_USE_CLASS
#define YY_USE_CLASS
#endif
#else
#ifndef __STDC__
#define const
#endif
#endif
#include <stdio.h>
#define YYBISON 1  

/* #line 73 "/usr/local/lib/bison.cc" */
#line 85 "typeAnalysis/typeAnalysis.cc"
#line 4 "typeAnalysis/typeAnalysis.y"

#include "global.h"
// #include "analyzeBlocks.h"
// #include "typeLex.h"
class typeLex;
class tokenType;
#line 12 "typeAnalysis/typeAnalysis.y"

#include "typeLex.h"

#line 19 "typeAnalysis/typeAnalysis.y"
typedef union {
tokenType *term;     /* For returning terminals                  */
varType nonterm;   /* For returning symbol-table pointers      */
} yy_typeAnalysis_stype;
#define YY_typeAnalysis_STYPE yy_typeAnalysis_stype
#define YY_typeAnalysis_DEBUG  1 
#define YY_typeAnalysis_CONSTRUCTOR_PARAM  \
    typeLex * paramLex,  \
    BBBlock * paramBlock, \
    varType inVarType    
#define YY_typeAnalysis_CONSTRUCTOR_CODE  \
    inLex = paramLex; \
    inBlock = paramBlock; \
    globalUse = DEFINE_TYPE; \
    globalType = INT_TYPE; \
    binOpType = NULL; \
    aliasBoolean = false; \
    inTypeHint = inVarType; \
    inTypeHintOn = false;
#define YY_typeAnalysis_MEMBERS  \
public: \
    virtual ~typeAnalysis();            \
private: \
    /* Note that globalType values have only valid		\
       value to the immediate next token grouping		\
   								\
       After the next immediate token grouping, the 		\
       value of these global variables will be unknown		\
       until they are explicitly assigned again			\
    */   							\
    defUse globalUse;     \
    varType globalType;   \
    varType prevType;     \
    Byte prevRegNum;      \
    tokenType* binOpType; \
    bool aliasBoolean;    \
    Byte aliasReg;        \
    varType inTypeHint;   \
    bool inTypeHintOn;    \
    void savePrevType(Byte inRegNum, varType inType); \
    void rollbackToPrevType(varType inType);          \
protected: \
\
    typeLex * inLex; \
    BBBlock * inBlock;

#line 73 "/usr/local/lib/bison.cc"
/* %{ and %header{ and %union, during decl */
#define YY_typeAnalysis_BISON 1
#ifndef YY_typeAnalysis_COMPATIBILITY
#ifndef YY_USE_CLASS
#define  YY_typeAnalysis_COMPATIBILITY 1
#else
#define  YY_typeAnalysis_COMPATIBILITY 0
#endif
#endif

#if YY_typeAnalysis_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YYLTYPE
#ifndef YY_typeAnalysis_LTYPE
#define YY_typeAnalysis_LTYPE YYLTYPE
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_typeAnalysis_STYPE 
#define YY_typeAnalysis_STYPE YYSTYPE
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_typeAnalysis_DEBUG
#define  YY_typeAnalysis_DEBUG YYDEBUG
#endif
#endif
#ifdef YY_typeAnalysis_STYPE
#ifndef yystype
#define yystype YY_typeAnalysis_STYPE
#endif
#endif
/* use goto to be compatible */
#ifndef YY_typeAnalysis_USE_GOTO
#define YY_typeAnalysis_USE_GOTO 1
#endif
#endif

/* use no goto to be clean in C++ */
#ifndef YY_typeAnalysis_USE_GOTO
#define YY_typeAnalysis_USE_GOTO 0
#endif

#ifndef YY_typeAnalysis_PURE

/* #line 117 "/usr/local/lib/bison.cc" */
#line 191 "typeAnalysis/typeAnalysis.cc"

#line 117 "/usr/local/lib/bison.cc"
/*  YY_typeAnalysis_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 121 "/usr/local/lib/bison.cc" */
#line 200 "typeAnalysis/typeAnalysis.cc"

#line 121 "/usr/local/lib/bison.cc"
/* prefix */
#ifndef YY_typeAnalysis_DEBUG

/* #line 123 "/usr/local/lib/bison.cc" */
#line 207 "typeAnalysis/typeAnalysis.cc"

#line 123 "/usr/local/lib/bison.cc"
/* YY_typeAnalysis_DEBUG */
#endif


#ifndef YY_typeAnalysis_LSP_NEEDED

/* #line 128 "/usr/local/lib/bison.cc" */
#line 217 "typeAnalysis/typeAnalysis.cc"

#line 128 "/usr/local/lib/bison.cc"
 /* YY_typeAnalysis_LSP_NEEDED*/
#endif



/* DEFAULT LTYPE*/
#ifdef YY_typeAnalysis_LSP_NEEDED
#ifndef YY_typeAnalysis_LTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YY_typeAnalysis_LTYPE yyltype
#endif
#endif
/* DEFAULT STYPE*/
      /* We used to use `unsigned long' as YY_typeAnalysis_STYPE on MSDOS,
	 but it seems better to be consistent.
	 Most programs should declare their own type anyway.  */

#ifndef YY_typeAnalysis_STYPE
#define YY_typeAnalysis_STYPE int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_typeAnalysis_PARSE
#define YY_typeAnalysis_PARSE yyparse
#endif
#ifndef YY_typeAnalysis_LEX
#define YY_typeAnalysis_LEX yylex
#endif
#ifndef YY_typeAnalysis_LVAL
#define YY_typeAnalysis_LVAL yylval
#endif
#ifndef YY_typeAnalysis_LLOC
#define YY_typeAnalysis_LLOC yylloc
#endif
#ifndef YY_typeAnalysis_CHAR
#define YY_typeAnalysis_CHAR yychar
#endif
#ifndef YY_typeAnalysis_NERRS
#define YY_typeAnalysis_NERRS yynerrs
#endif
#ifndef YY_typeAnalysis_DEBUG_FLAG
#define YY_typeAnalysis_DEBUG_FLAG yydebug
#endif
#ifndef YY_typeAnalysis_ERROR
#define YY_typeAnalysis_ERROR yyerror
#endif
#ifndef YY_typeAnalysis_PARSE_PARAM
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
#define YY_typeAnalysis_PARSE_PARAM
#ifndef YY_typeAnalysis_PARSE_PARAM_DEF
#define YY_typeAnalysis_PARSE_PARAM_DEF
#endif
#endif
#endif
#endif
#ifndef YY_typeAnalysis_PARSE_PARAM
#define YY_typeAnalysis_PARSE_PARAM void
#endif
#endif
#if YY_typeAnalysis_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YY_typeAnalysis_LTYPE
#ifndef YYLTYPE
#define YYLTYPE YY_typeAnalysis_LTYPE
#else
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
#endif
#endif
#ifndef YYSTYPE
#define YYSTYPE YY_typeAnalysis_STYPE
#else
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
#endif
#ifdef YY_typeAnalysis_PURE
#ifndef YYPURE
#define YYPURE YY_typeAnalysis_PURE
#endif
#endif
#ifdef YY_typeAnalysis_DEBUG
#ifndef YYDEBUG
#define YYDEBUG YY_typeAnalysis_DEBUG 
#endif
#endif
#ifndef YY_typeAnalysis_ERROR_VERBOSE
#ifdef YYERROR_VERBOSE
#define YY_typeAnalysis_ERROR_VERBOSE YYERROR_VERBOSE
#endif
#endif
#ifndef YY_typeAnalysis_LSP_NEEDED
#ifdef YYLSP_NEEDED
#define YY_typeAnalysis_LSP_NEEDED YYLSP_NEEDED
#endif
#endif
#endif
#ifndef YY_USE_CLASS
/* TOKEN C */

/* #line 236 "/usr/local/lib/bison.cc" */
#line 330 "typeAnalysis/typeAnalysis.cc"
#define	EOL	258
#define	ASSIGN	259
#define	MEM_OF	260
#define	BIN_OP	261
#define	REG_OF	262
#define	ADDR_OF	263
#define	VAR_OF	264
#define	THREE_OP	265
#define	TMP	266
#define	INT_CONSTANT	267
#define	FLOAT_CONSTANT	268
#define	ADDR_CONSTANT	269
#define	SIZE	270
#define	CAST_INT_STAR	271
#define	SINGLE_OP	272
#define	ZERO_OP	273
#define	FIXED_OP	274
#define	TRUNCU	275
#define	TRUNCS	276
#define	ZFILL	277
#define	SGNEX	278
#define	FSIZE	279
#define	ITOF	280
#define	FTOI	281
#define	FROUND	282
#define	TOINT	283
#define	TOFLT	284
#define	SIN	285
#define	COS	286
#define	TAN	287
#define	ARCTAN	288
#define	LOG2	289
#define	LOG10	290
#define	LOGE	291
#define	SQRT	292
#define	EXECUTE	293
#define	HLCTI	294
#define	DEFINE	295


#line 236 "/usr/local/lib/bison.cc"
 /* #defines tokens */
#else
/* CLASS */
#ifndef YY_typeAnalysis_CLASS
#define YY_typeAnalysis_CLASS typeAnalysis
#endif
#ifndef YY_typeAnalysis_INHERIT
#define YY_typeAnalysis_INHERIT
#endif
#ifndef YY_typeAnalysis_MEMBERS
#define YY_typeAnalysis_MEMBERS 
#endif
#ifndef YY_typeAnalysis_LEX_BODY
#define YY_typeAnalysis_LEX_BODY  
#endif
#ifndef YY_typeAnalysis_ERROR_BODY
#define YY_typeAnalysis_ERROR_BODY  
#endif
#ifndef YY_typeAnalysis_CONSTRUCTOR_PARAM
#define YY_typeAnalysis_CONSTRUCTOR_PARAM
#endif
#ifndef YY_typeAnalysis_CONSTRUCTOR_CODE
#define YY_typeAnalysis_CONSTRUCTOR_CODE
#endif
#ifndef YY_typeAnalysis_CONSTRUCTOR_INIT
#define YY_typeAnalysis_CONSTRUCTOR_INIT
#endif
/* choose between enum and const */
#ifndef YY_typeAnalysis_USE_CONST_TOKEN
#define YY_typeAnalysis_USE_CONST_TOKEN 0
/* yes enum is more compatible with flex,  */
/* so by default we use it */ 
#endif
#if YY_typeAnalysis_USE_CONST_TOKEN != 0
#ifndef YY_typeAnalysis_ENUM_TOKEN
#define YY_typeAnalysis_ENUM_TOKEN yy_typeAnalysis_enum_token
#endif
#endif

class YY_typeAnalysis_CLASS YY_typeAnalysis_INHERIT
{
public: 
#if YY_typeAnalysis_USE_CONST_TOKEN != 0
/* static const int token ... */

/* #line 280 "/usr/local/lib/bison.cc" */
#line 418 "typeAnalysis/typeAnalysis.cc"
static const int EOL;
static const int ASSIGN;
static const int MEM_OF;
static const int BIN_OP;
static const int REG_OF;
static const int ADDR_OF;
static const int VAR_OF;
static const int THREE_OP;
static const int TMP;
static const int INT_CONSTANT;
static const int FLOAT_CONSTANT;
static const int ADDR_CONSTANT;
static const int SIZE;
static const int CAST_INT_STAR;
static const int SINGLE_OP;
static const int ZERO_OP;
static const int FIXED_OP;
static const int TRUNCU;
static const int TRUNCS;
static const int ZFILL;
static const int SGNEX;
static const int FSIZE;
static const int ITOF;
static const int FTOI;
static const int FROUND;
static const int TOINT;
static const int TOFLT;
static const int SIN;
static const int COS;
static const int TAN;
static const int ARCTAN;
static const int LOG2;
static const int LOG10;
static const int LOGE;
static const int SQRT;
static const int EXECUTE;
static const int HLCTI;
static const int DEFINE;


#line 280 "/usr/local/lib/bison.cc"
 /* decl const */
#else
enum YY_typeAnalysis_ENUM_TOKEN { YY_typeAnalysis_NULL_TOKEN=0

/* #line 283 "/usr/local/lib/bison.cc" */
#line 465 "typeAnalysis/typeAnalysis.cc"
	,EOL=258
	,ASSIGN=259
	,MEM_OF=260
	,BIN_OP=261
	,REG_OF=262
	,ADDR_OF=263
	,VAR_OF=264
	,THREE_OP=265
	,TMP=266
	,INT_CONSTANT=267
	,FLOAT_CONSTANT=268
	,ADDR_CONSTANT=269
	,SIZE=270
	,CAST_INT_STAR=271
	,SINGLE_OP=272
	,ZERO_OP=273
	,FIXED_OP=274
	,TRUNCU=275
	,TRUNCS=276
	,ZFILL=277
	,SGNEX=278
	,FSIZE=279
	,ITOF=280
	,FTOI=281
	,FROUND=282
	,TOINT=283
	,TOFLT=284
	,SIN=285
	,COS=286
	,TAN=287
	,ARCTAN=288
	,LOG2=289
	,LOG10=290
	,LOGE=291
	,SQRT=292
	,EXECUTE=293
	,HLCTI=294
	,DEFINE=295


#line 283 "/usr/local/lib/bison.cc"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_typeAnalysis_PARSE (YY_typeAnalysis_PARSE_PARAM);
 virtual void YY_typeAnalysis_ERROR(char *msg) YY_typeAnalysis_ERROR_BODY;
#ifdef YY_typeAnalysis_PURE
#ifdef YY_typeAnalysis_LSP_NEEDED
 virtual int  YY_typeAnalysis_LEX (YY_typeAnalysis_STYPE *YY_typeAnalysis_LVAL,YY_typeAnalysis_LTYPE *YY_typeAnalysis_LLOC) YY_typeAnalysis_LEX_BODY;
#else
 virtual int  YY_typeAnalysis_LEX (YY_typeAnalysis_STYPE *YY_typeAnalysis_LVAL) YY_typeAnalysis_LEX_BODY;
#endif
#else
 virtual int YY_typeAnalysis_LEX() YY_typeAnalysis_LEX_BODY;
 YY_typeAnalysis_STYPE YY_typeAnalysis_LVAL;
#ifdef YY_typeAnalysis_LSP_NEEDED
 YY_typeAnalysis_LTYPE YY_typeAnalysis_LLOC;
#endif
 int   YY_typeAnalysis_NERRS;
 int    YY_typeAnalysis_CHAR;
#endif
#if YY_typeAnalysis_DEBUG != 0
 int YY_typeAnalysis_DEBUG_FLAG;   /*  nonzero means print parse trace     */
#endif
public:
 YY_typeAnalysis_CLASS(YY_typeAnalysis_CONSTRUCTOR_PARAM);
public:
 YY_typeAnalysis_MEMBERS 
};
/* other declare folow */
#if YY_typeAnalysis_USE_CONST_TOKEN != 0

/* #line 314 "/usr/local/lib/bison.cc" */
#line 540 "typeAnalysis/typeAnalysis.cc"
const int YY_typeAnalysis_CLASS::EOL=258;
const int YY_typeAnalysis_CLASS::ASSIGN=259;
const int YY_typeAnalysis_CLASS::MEM_OF=260;
const int YY_typeAnalysis_CLASS::BIN_OP=261;
const int YY_typeAnalysis_CLASS::REG_OF=262;
const int YY_typeAnalysis_CLASS::ADDR_OF=263;
const int YY_typeAnalysis_CLASS::VAR_OF=264;
const int YY_typeAnalysis_CLASS::THREE_OP=265;
const int YY_typeAnalysis_CLASS::TMP=266;
const int YY_typeAnalysis_CLASS::INT_CONSTANT=267;
const int YY_typeAnalysis_CLASS::FLOAT_CONSTANT=268;
const int YY_typeAnalysis_CLASS::ADDR_CONSTANT=269;
const int YY_typeAnalysis_CLASS::SIZE=270;
const int YY_typeAnalysis_CLASS::CAST_INT_STAR=271;
const int YY_typeAnalysis_CLASS::SINGLE_OP=272;
const int YY_typeAnalysis_CLASS::ZERO_OP=273;
const int YY_typeAnalysis_CLASS::FIXED_OP=274;
const int YY_typeAnalysis_CLASS::TRUNCU=275;
const int YY_typeAnalysis_CLASS::TRUNCS=276;
const int YY_typeAnalysis_CLASS::ZFILL=277;
const int YY_typeAnalysis_CLASS::SGNEX=278;
const int YY_typeAnalysis_CLASS::FSIZE=279;
const int YY_typeAnalysis_CLASS::ITOF=280;
const int YY_typeAnalysis_CLASS::FTOI=281;
const int YY_typeAnalysis_CLASS::FROUND=282;
const int YY_typeAnalysis_CLASS::TOINT=283;
const int YY_typeAnalysis_CLASS::TOFLT=284;
const int YY_typeAnalysis_CLASS::SIN=285;
const int YY_typeAnalysis_CLASS::COS=286;
const int YY_typeAnalysis_CLASS::TAN=287;
const int YY_typeAnalysis_CLASS::ARCTAN=288;
const int YY_typeAnalysis_CLASS::LOG2=289;
const int YY_typeAnalysis_CLASS::LOG10=290;
const int YY_typeAnalysis_CLASS::LOGE=291;
const int YY_typeAnalysis_CLASS::SQRT=292;
const int YY_typeAnalysis_CLASS::EXECUTE=293;
const int YY_typeAnalysis_CLASS::HLCTI=294;
const int YY_typeAnalysis_CLASS::DEFINE=295;


#line 314 "/usr/local/lib/bison.cc"
 /* const YY_typeAnalysis_CLASS::token */
#endif
/*apres const  */
YY_typeAnalysis_CLASS::YY_typeAnalysis_CLASS(YY_typeAnalysis_CONSTRUCTOR_PARAM) YY_typeAnalysis_CONSTRUCTOR_INIT
{
#if YY_typeAnalysis_DEBUG != 0
YY_typeAnalysis_DEBUG_FLAG=0;
#endif
YY_typeAnalysis_CONSTRUCTOR_CODE;
};
#endif

/* #line 325 "/usr/local/lib/bison.cc" */
#line 595 "typeAnalysis/typeAnalysis.cc"


#define	YYFINAL		126
#define	YYFLAG		-32768
#define	YYNTBASE	41

#define YYTRANSLATE(x) ((unsigned)(x) <= 295 ? yytranslate[x] : 75)

static const char yytranslate[] = {     0,
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
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40
};

#if YY_typeAnalysis_DEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     5,     6,    11,    14,    15,    19,    23,    26,
    27,    32,    33,    34,    40,    41,    42,    48,    49,    53,
    54,    58,    61,    64,    65,    69,    71,    73,    75,    77,
    78,    81,    82,    86,    87,    88,    93,    95,   100,   102,
   104,   106,   109,   110,   114,   115,   119,   121,   123,   124,
   128,   131,   134,   137,   140,   141,   145,   146,   150,   151,
   155,   156,   160,   163,   166,   169,   172,   175,   178,   181,
   184,   187,   190,   191,   195,   197,   199
};

static const short yyrhs[] = {    -1,
    42,    64,     3,     0,     0,    40,    43,    64,     3,     0,
     4,    46,     0,     0,    44,    52,     3,     0,     7,    12,
     3,     0,    64,     3,     0,     0,     7,    12,    47,    45,
     0,     0,     0,    48,    64,    49,    64,     3,     0,     0,
     0,    50,    52,    51,    64,     3,     0,     0,    39,    53,
    64,     0,     0,     5,    55,    64,     0,     7,    12,     0,
     7,    11,     0,     0,     8,    56,    64,     0,     9,     0,
    74,     0,    54,     0,    74,     0,     0,    59,    54,     0,
     0,    74,    61,    57,     0,     0,     0,    62,    54,    63,
    58,     0,    54,     0,    10,    64,    64,    64,     0,    11,
     0,    74,     0,    68,     0,    15,    64,     0,     0,    16,
    65,    64,     0,     0,    17,    66,    64,     0,    18,     0,
    19,     0,     0,     6,    67,    60,     0,    20,    64,     0,
    21,    64,     0,    22,    64,     0,    23,    64,     0,     0,
    24,    69,    64,     0,     0,    25,    70,    64,     0,     0,
    26,    71,    64,     0,     0,    27,    72,    64,     0,    28,
    64,     0,    29,    64,     0,    30,    64,     0,    31,    64,
     0,    32,    64,     0,    33,    64,     0,    34,    64,     0,
    35,    64,     0,    36,    64,     0,    37,    64,     0,     0,
    38,    73,    64,     0,    12,     0,    13,     0,    14,     0
};

#endif

#if YY_typeAnalysis_DEBUG != 0
static const short yyrline[] = { 0,
    98,    99,   100,   100,   104,   106,   106,   109,   129,   133,
   156,   162,   162,   163,   166,   166,   167,   172,   173,   186,
   187,   190,   244,   249,   249,   252,   259,   263,   267,   271,
   271,   276,   278,   281,   282,   282,   287,   291,   295,   299,
   302,   305,   309,   309,   312,   312,   318,   321,   324,   324,
   329,   331,   332,   333,   334,   334,   335,   335,   336,   336,
   337,   337,   338,   339,   340,   341,   342,   343,   344,   345,
   346,   347,   348,   348,   351,   358,   362
};

static const char * const yytname[] = {   "$","error","$illegal.","EOL","ASSIGN",
"MEM_OF","BIN_OP","REG_OF","ADDR_OF","VAR_OF","THREE_OP","TMP","INT_CONSTANT",
"FLOAT_CONSTANT","ADDR_CONSTANT","SIZE","CAST_INT_STAR","SINGLE_OP","ZERO_OP",
"FIXED_OP","TRUNCU","TRUNCS","ZFILL","SGNEX","FSIZE","ITOF","FTOI","FROUND",
"TOINT","TOFLT","SIN","COS","TAN","ARCTAN","LOG2","LOG10","LOGE","SQRT","EXECUTE",
"HLCTI","DEFINE","assignment","@1","@2","@3","assignleftsidealias","assignrightside",
"@4","@5","@6","@7","@8","highleveljump","@9","variable","@10","@11","binsecondparamconstant",
"binsecondparamvariable","@12","binfirstparam","@13","@14","@15","operation",
"@16","@17","@18","function","@19","@20","@21","@22","@23","constants",""
};
#endif

static const short yyr1[] = {     0,
    42,    41,    43,    41,    41,    44,    41,    45,    45,    47,
    46,    48,    49,    46,    50,    51,    46,    53,    52,    55,
    54,    54,    54,    56,    54,    54,    57,    57,    58,    59,
    58,    61,    60,    62,    63,    60,    64,    64,    64,    64,
    64,    64,    65,    64,    66,    64,    64,    64,    67,    64,
    68,    68,    68,    68,    69,    68,    70,    68,    71,    68,
    72,    68,    68,    68,    68,    68,    68,    68,    68,    68,
    68,    68,    73,    68,    74,    74,    74
};

static const short yyr2[] = {     0,
     0,     3,     0,     4,     2,     0,     3,     3,     2,     0,
     4,     0,     0,     5,     0,     0,     5,     0,     3,     0,
     3,     2,     2,     0,     3,     1,     1,     1,     1,     0,
     2,     0,     3,     0,     0,     4,     1,     4,     1,     1,
     1,     2,     0,     3,     0,     3,     1,     1,     0,     3,
     2,     2,     2,     2,     0,     3,     0,     3,     0,     3,
     0,     3,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     0,     3,     1,     1,     1
};

static const short yydefact[] = {     1,
    12,     3,     0,     0,     0,     5,     0,     0,     0,    20,
    49,     0,    24,    26,     0,    39,    75,    76,    77,     0,
    43,    45,    47,    48,     0,     0,     0,     0,    55,    57,
    59,    61,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    73,    37,     0,    41,    40,    18,     0,    10,
    13,    16,     0,     0,    34,    23,    22,     0,     0,    42,
     0,     0,    51,    52,    53,    54,     0,     0,     0,     0,
    63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
     0,     2,     0,     7,     0,     0,     0,     4,    21,    50,
     0,    32,    25,     0,    44,    46,    56,    58,    60,    62,
    74,    19,     0,    11,     0,     0,     0,    35,     0,    38,
     0,     9,    14,    17,    30,    28,    33,    27,     8,    36,
     0,    29,    31,     0,     0,     0
};

static const short yydefgoto[] = {   124,
     3,     9,     4,   104,     6,    85,     7,    86,     8,    87,
    49,    83,    44,    54,    58,   117,   120,   121,    90,   109,
    91,   115,    45,    61,    62,    55,    46,    67,    68,    69,
    70,    81,    47
};

static const short yypact[] = {    -3,
    -1,-32768,    83,   -36,     0,-32768,    83,   -36,    83,-32768,
-32768,    13,-32768,-32768,    83,-32768,-32768,-32768,-32768,    83,
-32768,-32768,-32768,-32768,    83,    83,    83,    83,-32768,-32768,
-32768,-32768,    83,    83,    83,    83,    83,    83,    83,    83,
    83,    83,-32768,-32768,    14,-32768,-32768,-32768,    19,-32768,
-32768,-32768,    53,    83,    36,-32768,-32768,    83,    83,-32768,
    83,    83,-32768,-32768,-32768,-32768,    83,    83,    83,    83,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
    83,-32768,    83,-32768,   117,    83,    83,-32768,-32768,-32768,
    35,-32768,-32768,    83,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,    34,-32768,    54,    56,    62,-32768,     2,-32768,
    63,-32768,-32768,-32768,    36,-32768,-32768,-32768,-32768,-32768,
    35,-32768,-32768,     5,    39,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
    59,-32768,   -68,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,    -7,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   -51
};


#define	YYLAST		155


static const short yytable[] = {    51,
     1,    53,    48,    92,   125,     5,    10,    59,    12,    13,
    14,    50,    60,    17,    18,    19,    82,    63,    64,    65,
    66,    84,   108,    56,    57,    71,    72,    73,    74,    75,
    76,    77,    78,    79,    80,    -6,     2,   -15,   126,    10,
   116,    12,    13,    14,    56,   111,    89,    17,    18,    19,
    93,    94,   123,    95,    96,    88,   112,   118,   113,    97,
    98,    99,   100,   122,   114,   119,    52,     0,     0,     0,
     0,     0,     0,   101,     0,   102,     0,   105,   106,   107,
     0,     0,     0,     0,     0,     0,   110,    10,    11,    12,
    13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
    23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
    33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
    43,    10,    11,   103,    13,    14,    15,    16,    17,    18,
    19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
    29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
    39,    40,    41,    42,    43
};

static const short yycheck[] = {     7,
     4,     9,    39,    55,     0,     7,     5,    15,     7,     8,
     9,    12,    20,    12,    13,    14,     3,    25,    26,    27,
    28,     3,    91,    11,    12,    33,    34,    35,    36,    37,
    38,    39,    40,    41,    42,    39,    40,    39,     0,     5,
   109,     7,     8,     9,    11,    12,    54,    12,    13,    14,
    58,    59,   121,    61,    62,     3,     3,   109,     3,    67,
    68,    69,    70,   115,     3,     3,     8,    -1,    -1,    -1,
    -1,    -1,    -1,    81,    -1,    83,    -1,    85,    86,    87,
    -1,    -1,    -1,    -1,    -1,    -1,    94,     5,     6,     7,
     8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
    28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
    38,     5,     6,     7,     8,     9,    10,    11,    12,    13,
    14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
    34,    35,    36,    37,    38
};

#line 325 "/usr/local/lib/bison.cc"
 /* fattrs + tables */

/* parser code folow  */


/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: dollar marks section change
   the next  is replaced by the list of actions, each action
   as one case of the switch.  */ 

#if YY_typeAnalysis_USE_GOTO != 0
/* 
 SUPRESSION OF GOTO : on some C++ compiler (sun c++)
  the goto is strictly forbidden if any constructor/destructor
  is used in the whole function (very stupid isn't it ?)
 so goto are to be replaced with a 'while/switch/case construct'
 here are the macro to keep some apparent compatibility
*/
#define YYGOTO(lb) {yy_gotostate=lb;continue;}
#define YYBEGINGOTO  enum yy_labels yy_gotostate=yygotostart; \
                     for(;;) switch(yy_gotostate) { case yygotostart: {
#define YYLABEL(lb) } case lb: {
#define YYENDGOTO } } 
#define YYBEGINDECLARELABEL enum yy_labels {yygotostart
#define YYDECLARELABEL(lb) ,lb
#define YYENDDECLARELABEL  };
#else
/* macro to keep goto */
#define YYGOTO(lb) goto lb
#define YYBEGINGOTO 
#define YYLABEL(lb) lb:
#define YYENDGOTO
#define YYBEGINDECLARELABEL 
#define YYDECLARELABEL(lb)
#define YYENDDECLARELABEL 
#endif
/* LABEL DECLARATION */
YYBEGINDECLARELABEL
  YYDECLARELABEL(yynewstate)
  YYDECLARELABEL(yybackup)
/* YYDECLARELABEL(yyresume) */
  YYDECLARELABEL(yydefault)
  YYDECLARELABEL(yyreduce)
  YYDECLARELABEL(yyerrlab)   /* here on detecting error */
  YYDECLARELABEL(yyerrlab1)   /* here on error raised explicitly by an action */
  YYDECLARELABEL(yyerrdefault)  /* current state does not do anything special for the error token. */
  YYDECLARELABEL(yyerrpop)   /* pop the current state because it cannot handle the error token */
  YYDECLARELABEL(yyerrhandle)  
YYENDDECLARELABEL
/* ALLOCA SIMULATION */
/* __HAVE_NO_ALLOCA */
#ifdef __HAVE_NO_ALLOCA
int __alloca_free_ptr(char *ptr,char *ref)
{if(ptr!=ref) free(ptr);
 return 0;}

#define __ALLOCA_alloca(size) malloc(size)
#define __ALLOCA_free(ptr,ref) __alloca_free_ptr((char *)ptr,(char *)ref)

#ifdef YY_typeAnalysis_LSP_NEEDED
#define __ALLOCA_return(num) \
            return( __ALLOCA_free(yyss,yyssa)+\
		    __ALLOCA_free(yyvs,yyvsa)+\
		    __ALLOCA_free(yyls,yylsa)+\
		   (num))
#else
#define __ALLOCA_return(num) \
            return( __ALLOCA_free(yyss,yyssa)+\
		    __ALLOCA_free(yyvs,yyvsa)+\
		   (num))
#endif
#else
#define __ALLOCA_return(num) return(num)
#define __ALLOCA_alloca(size) alloca(size)
#define __ALLOCA_free(ptr,ref) 
#endif

/* ENDALLOCA SIMULATION */

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (YY_typeAnalysis_CHAR = YYEMPTY)
#define YYEMPTY         -2
#define YYEOF           0
#define YYACCEPT        __ALLOCA_return(0)
#define YYABORT         __ALLOCA_return(1)
#define YYERROR         YYGOTO(yyerrlab1)
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL          YYGOTO(yyerrlab)
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do                                                              \
  if (YY_typeAnalysis_CHAR == YYEMPTY && yylen == 1)                               \
    { YY_typeAnalysis_CHAR = (token), YY_typeAnalysis_LVAL = (value);                 \
      yychar1 = YYTRANSLATE (YY_typeAnalysis_CHAR);                                \
      YYPOPSTACK;                                               \
      YYGOTO(yybackup);                                            \
    }                                                           \
  else                                                          \
    { YY_typeAnalysis_ERROR ("syntax error: cannot back up"); YYERROR; }   \
while (0)

#define YYTERROR        1
#define YYERRCODE       256

#ifndef YY_typeAnalysis_PURE
/* UNPURE */
#define YYLEX           YY_typeAnalysis_LEX()
#ifndef YY_USE_CLASS
/* If nonreentrant, and not class , generate the variables here */
int     YY_typeAnalysis_CHAR;                      /*  the lookahead symbol        */
YY_typeAnalysis_STYPE      YY_typeAnalysis_LVAL;              /*  the semantic value of the */
				/*  lookahead symbol    */
int YY_typeAnalysis_NERRS;                 /*  number of parse errors so far */
#ifdef YY_typeAnalysis_LSP_NEEDED
YY_typeAnalysis_LTYPE YY_typeAnalysis_LLOC;   /*  location data for the lookahead     */
			/*  symbol                              */
#endif
#endif


#else
/* PURE */
#ifdef YY_typeAnalysis_LSP_NEEDED
#define YYLEX           YY_typeAnalysis_LEX(&YY_typeAnalysis_LVAL, &YY_typeAnalysis_LLOC)
#else
#define YYLEX           YY_typeAnalysis_LEX(&YY_typeAnalysis_LVAL)
#endif
#endif
#ifndef YY_USE_CLASS
#if YY_typeAnalysis_DEBUG != 0
int YY_typeAnalysis_DEBUG_FLAG;                    /*  nonzero means print parse trace     */
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif
#endif



/*  YYINITDEPTH indicates the initial size of the parser's stacks       */

#ifndef YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif


#if __GNUC__ > 1                /* GNU C and GNU C++ define this.  */
#define __yy_bcopy(FROM,TO,COUNT)       __builtin_memcpy(TO,FROM,COUNT)
#else                           /* not GNU C or C++ */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */

#ifdef __cplusplus
static void __yy_bcopy (char *from, char *to, int count)
#else
#ifdef __STDC__
static void __yy_bcopy (char *from, char *to, int count)
#else
static void __yy_bcopy (from, to, count)
     char *from;
     char *to;
     int count;
#endif
#endif
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}
#endif

int
#ifdef YY_USE_CLASS
 YY_typeAnalysis_CLASS::
#endif
     YY_typeAnalysis_PARSE(YY_typeAnalysis_PARSE_PARAM)
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
/* parameter definition without protypes */
YY_typeAnalysis_PARSE_PARAM_DEF
#endif
#endif
#endif
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YY_typeAnalysis_STYPE *yyvsp;
  int yyerrstatus;      /*  number of tokens to shift before error messages enabled */
  int yychar1=0;          /*  lookahead token as an internal (translated) token number */

  short yyssa[YYINITDEPTH];     /*  the state stack                     */
  YY_typeAnalysis_STYPE yyvsa[YYINITDEPTH];        /*  the semantic value stack            */

  short *yyss = yyssa;          /*  refer to the stacks thru separate pointers */
  YY_typeAnalysis_STYPE *yyvs = yyvsa;     /*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YY_typeAnalysis_LSP_NEEDED
  YY_typeAnalysis_LTYPE yylsa[YYINITDEPTH];        /*  the location stack                  */
  YY_typeAnalysis_LTYPE *yyls = yylsa;
  YY_typeAnalysis_LTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YY_typeAnalysis_PURE
  int YY_typeAnalysis_CHAR;
  YY_typeAnalysis_STYPE YY_typeAnalysis_LVAL;
  int YY_typeAnalysis_NERRS;
#ifdef YY_typeAnalysis_LSP_NEEDED
  YY_typeAnalysis_LTYPE YY_typeAnalysis_LLOC;
#endif
#endif

  YY_typeAnalysis_STYPE yyval;             /*  the variable used to return         */
				/*  semantic values from the action     */
				/*  routines                            */

  int yylen;
/* start loop, in which YYGOTO may be used. */
YYBEGINGOTO

#if YY_typeAnalysis_DEBUG != 0
  if (YY_typeAnalysis_DEBUG_FLAG)
    fprintf(stderr, "Starting parse\n");
#endif
  yystate = 0;
  yyerrstatus = 0;
  YY_typeAnalysis_NERRS = 0;
  YY_typeAnalysis_CHAR = YYEMPTY;          /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YY_typeAnalysis_LSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
YYLABEL(yynewstate)

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YY_typeAnalysis_STYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YY_typeAnalysis_LSP_NEEDED
      YY_typeAnalysis_LTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YY_typeAnalysis_LSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YY_typeAnalysis_LSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  YY_typeAnalysis_ERROR("parser stack overflow");
	  __ALLOCA_return(2);
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) __ALLOCA_alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      __ALLOCA_free(yyss1,yyssa);
      yyvs = (YY_typeAnalysis_STYPE *) __ALLOCA_alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
      __ALLOCA_free(yyvs1,yyvsa);
#ifdef YY_typeAnalysis_LSP_NEEDED
      yyls = (YY_typeAnalysis_LTYPE *) __ALLOCA_alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
      __ALLOCA_free(yyls1,yylsa);
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YY_typeAnalysis_LSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YY_typeAnalysis_DEBUG != 0
      if (YY_typeAnalysis_DEBUG_FLAG)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YY_typeAnalysis_DEBUG != 0
  if (YY_typeAnalysis_DEBUG_FLAG)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  YYGOTO(yybackup);
YYLABEL(yybackup)

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* YYLABEL(yyresume) */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    YYGOTO(yydefault);

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (YY_typeAnalysis_CHAR == YYEMPTY)
    {
#if YY_typeAnalysis_DEBUG != 0
      if (YY_typeAnalysis_DEBUG_FLAG)
	fprintf(stderr, "Reading a token: ");
#endif
      YY_typeAnalysis_CHAR = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (YY_typeAnalysis_CHAR <= 0)           /* This means end of input. */
    {
      yychar1 = 0;
      YY_typeAnalysis_CHAR = YYEOF;                /* Don't call YYLEX any more */

#if YY_typeAnalysis_DEBUG != 0
      if (YY_typeAnalysis_DEBUG_FLAG)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(YY_typeAnalysis_CHAR);

#if YY_typeAnalysis_DEBUG != 0
      if (YY_typeAnalysis_DEBUG_FLAG)
	{
	  fprintf (stderr, "Next token is %d (%s", YY_typeAnalysis_CHAR, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, YY_typeAnalysis_CHAR, YY_typeAnalysis_LVAL);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    YYGOTO(yydefault);

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	YYGOTO(yyerrlab);
      yyn = -yyn;
      YYGOTO(yyreduce);
    }
  else if (yyn == 0)
    YYGOTO(yyerrlab);

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YY_typeAnalysis_DEBUG != 0
  if (YY_typeAnalysis_DEBUG_FLAG)
    fprintf(stderr, "Shifting token %d (%s), ", YY_typeAnalysis_CHAR, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (YY_typeAnalysis_CHAR != YYEOF)
    YY_typeAnalysis_CHAR = YYEMPTY;

  *++yyvsp = YY_typeAnalysis_LVAL;
#ifdef YY_typeAnalysis_LSP_NEEDED
  *++yylsp = YY_typeAnalysis_LLOC;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  YYGOTO(yynewstate);

/* Do the default action for the current state.  */
YYLABEL(yydefault)

  yyn = yydefact[yystate];
  if (yyn == 0)
    YYGOTO(yyerrlab);

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
YYLABEL(yyreduce)
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YY_typeAnalysis_DEBUG != 0
  if (YY_typeAnalysis_DEBUG_FLAG)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


/* #line 811 "/usr/local/lib/bison.cc" */
#line 1297 "typeAnalysis/typeAnalysis.cc"

  switch (yyn) {

case 1:
#line 99 "typeAnalysis/typeAnalysis.y"
{globalUse = USE_TYPE;;
    break;}
case 3:
#line 100 "typeAnalysis/typeAnalysis.y"
{globalUse = DEFINE_TYPE; globalType = INT_TYPE;
    break;}
case 4:
#line 100 "typeAnalysis/typeAnalysis.y"
{
        /* Used for param defination. The operation is for the parameters
	   and it is always a define */ 
	;
    break;}
case 5:
#line 104 "typeAnalysis/typeAnalysis.y"
{
        ;
    break;}
case 6:
#line 106 "typeAnalysis/typeAnalysis.y"
{globalUse = USE_TYPE;;
    break;}
case 8:
#line 110 "typeAnalysis/typeAnalysis.y"
{
	    aliasBoolean = true;
	    aliasReg = yyvsp[-1].term->valueOne;
            inBlock->addUDElementReg(yyvsp[-1].term->valueOne, 
	                             yyvsp[-1].term->memAddress, 
		   	             yyvsp[-1].term->instructNumber, 
				     DEFINE_TYPE);	     
				      
            if (globalType > ((inBlock->returnRegChainListAt(yyvsp[-1].term->valueOne))->back())->chainType) {
	        inBlock->setTypeOfLastChainForReg(yyvsp[-1].term->valueOne, globalType);	     	     
            }		    
	    
	    if (inTypeHintOn){
	        if (inTypeHint > ((inBlock->returnRegChainListAt(yyvsp[-1].term->valueOne))->back())->chainType){
                    inBlock->setTypeOfLastChainForReg(yyvsp[-1].term->valueOne, inTypeHint);				
		}	
	    }
	
	;
    break;}
case 9:
#line 129 "typeAnalysis/typeAnalysis.y"
{
        ;
    break;}
case 10:
#line 134 "typeAnalysis/typeAnalysis.y"
{
	    aliasBoolean = false;
	    inBlock->addUDElementReg(yyvsp[0].term->valueOne, 
	                             yyvsp[0].term->memAddress, 
				     yyvsp[0].term->instructNumber, 
				     USE_TYPE);

	    /* Make sure there is some priority of assignment
	       i.e. FLOAT_TYPE > POINTER_I > POINTER_D > INT

	       We don't want to overwrite a higher priority
	       type over a lower prority type (because 
	       we are then losing information on the chain 
	    */
	    if (globalType > ((inBlock->returnRegChainListAt(yyvsp[0].term->valueOne))->back())->chainType) {
	       inBlock->setTypeOfLastChainForReg(yyvsp[0].term->valueOne, globalType);	     	     
	    }	           
	    else {
	       globalType = ((inBlock->returnRegChainListAt(yyvsp[0].term->valueOne))->back())->chainType;						  
            }
            globalUse = DEFINE_TYPE;
	    inTypeHintOn = true;	    	    
	;
    break;}
case 11:
#line 156 "typeAnalysis/typeAnalysis.y"
{
	    if (aliasBoolean){
	        inBlock->aliasRegLink((Byte)(yyvsp[-2].term->valueOne), aliasReg);
	    }
	    inTypeHintOn = false;
	;
    break;}
case 12:
#line 162 "typeAnalysis/typeAnalysis.y"
{globalUse = USE_TYPE;;
    break;}
case 13:
#line 162 "typeAnalysis/typeAnalysis.y"
{globalUse = DEFINE_TYPE; inTypeHintOn = true;
                                          globalType = yyvsp[0].nonterm; ;
    break;}
case 14:
#line 163 "typeAnalysis/typeAnalysis.y"
{
            inTypeHintOn = false;
	;
    break;}
case 15:
#line 166 "typeAnalysis/typeAnalysis.y"
{globalUse = USE_TYPE;;
    break;}
case 16:
#line 166 "typeAnalysis/typeAnalysis.y"
{globalUse = DEFINE_TYPE; inTypeHintOn = true;
                                              globalType = yyvsp[0].nonterm; ;
    break;}
case 17:
#line 167 "typeAnalysis/typeAnalysis.y"
{	
            inTypeHintOn = false;
        ;
    break;}
case 18:
#line 173 "typeAnalysis/typeAnalysis.y"
{globalType = POINTER_I;
    break;}
case 19:
#line 173 "typeAnalysis/typeAnalysis.y"
{
            /* LATER WE NEED TO GET TYPE INFO FROM HIGHLEVELJUMP */
	    /* First operation is for the address we are jumping to.*/
	    yyval.nonterm = INT_TYPE;
        ;
    break;}
case 20:
#line 187 "typeAnalysis/typeAnalysis.y"
{globalUse = USE_TYPE; globalType = POINTER_D;;
    break;}
case 21:
#line 187 "typeAnalysis/typeAnalysis.y"
{
	     yyval.nonterm = INT_TYPE;	    
	;
    break;}
case 22:
#line 190 "typeAnalysis/typeAnalysis.y"
{
        
	     list<UDChain*>* listChainPtr = inBlock->returnRegChainListAt(yyvsp[0].term->valueOne);
	     if (listChainPtr == NULL) {
	         if (inTypeHintOn){
	             savePrevType(yyvsp[0].term->valueOne, inTypeHint);		 
		 }
		 else {
	             savePrevType(yyvsp[0].term->valueOne, INT_TYPE);
		 }
             }
	     else {
	         UDChain* chainPtr = listChainPtr->back();
		 if (chainPtr == NULL){
	             if (inTypeHintOn){
	        	 savePrevType(yyvsp[0].term->valueOne, inTypeHint);
                     }		 
		     else {
	        	 savePrevType(yyvsp[0].term->valueOne, INT_TYPE);
		     }	
		 }
		 else{
    		     if (inTypeHintOn){
	        	 if (inTypeHint > chainPtr->chainType){
                	     inBlock->setTypeOfLastChainForReg(yyvsp[0].term->valueOne, inTypeHint);				
			 }	
		     } 		 
                     savePrevType(yyvsp[0].term->valueOne, chainPtr->chainType);
		 }	 
	     }    
     	     
    	     /* globalUse should hold the info that's needed to 
	        determine if the register is a Use or Define
	      */
    	     	     
             inBlock->addUDElementReg(yyvsp[0].term->valueOne, 
	                              yyvsp[0].term->memAddress, 
				      yyvsp[0].term->instructNumber, 
				      globalUse);
             
	     /* Make sure there is some priority of assignment
	        i.e. FLOAT_TYPE > POINTER_I > POINTER_D > INT
		
		We don't want to overwrite a higher priority
		type over a lower prority type (because 
		we are then losing information on the chain 
	     */
	     if (globalType > ((inBlock->returnRegChainListAt(yyvsp[0].term->valueOne))->back())->chainType) {
		inBlock->setTypeOfLastChainForReg(yyvsp[0].term->valueOne, globalType);	     	     
	     }	      
	     
	     yyval.nonterm = ((inBlock->returnRegChainListAt(yyvsp[0].term->valueOne))->back())->chainType;
	     
    	;
    break;}
case 23:
#line 244 "typeAnalysis/typeAnalysis.y"
{
    	    /* We don't currently handle this */
	    yyval.nonterm = globalType;	   
    	;
    break;}
case 24:
#line 249 "typeAnalysis/typeAnalysis.y"
{globalUse = USE_TYPE;
    break;}
case 25:
#line 249 "typeAnalysis/typeAnalysis.y"
{
	    yyval.nonterm = POINTER_D;
    	;
    break;}
case 26:
#line 252 "typeAnalysis/typeAnalysis.y"
{    
            /* Don't know what to do with this yet */ 
	    yyval.nonterm = globalType;
	;
    break;}
case 27:
#line 260 "typeAnalysis/typeAnalysis.y"
{
	    yyval.nonterm = INT_TYPE;  /* Just make it INT	*/
	;
    break;}
case 28:
#line 263 "typeAnalysis/typeAnalysis.y"
{
            yyval.nonterm = INT_TYPE;  /* Just make it INT */
        ;
    break;}
case 29:
#line 268 "typeAnalysis/typeAnalysis.y"
{
	    yyval.nonterm = INT_TYPE;  /* Just make it INT	*/
	;
    break;}
case 30:
#line 271 "typeAnalysis/typeAnalysis.y"
{rollbackToPrevType(binOpType->thisBinOpToken->left);
    break;}
case 31:
#line 271 "typeAnalysis/typeAnalysis.y"
{
            yyval.nonterm = INT_TYPE;  /* Just make it INT */
        ;
    break;}
case 32:
#line 277 "typeAnalysis/typeAnalysis.y"
{if (globalType < POINTER_D) globalType = binOpType->thisBinOpToken->right;;
    break;}
case 33:
#line 278 "typeAnalysis/typeAnalysis.y"
{
	    yyval.nonterm = INT_TYPE;  /* Just make it INT */
	;
    break;}
case 34:
#line 281 "typeAnalysis/typeAnalysis.y"
{if (globalType < POINTER_D) globalType = binOpType->thisBinOpToken->left;;
    break;}
case 35:
#line 282 "typeAnalysis/typeAnalysis.y"
{globalType = binOpType->thisBinOpToken->right;;
    break;}
case 36:
#line 282 "typeAnalysis/typeAnalysis.y"
{
	    yyval.nonterm = INT_TYPE;  /* Just make it INT	*/
        ;
    break;}
case 37:
#line 288 "typeAnalysis/typeAnalysis.y"
{
            yyval.nonterm = yyvsp[0].nonterm	
	;
    break;}
case 38:
#line 291 "typeAnalysis/typeAnalysis.y"
{
            /* Don't handle this yet */
	    yyval.nonterm = globalType;
	;
    break;}
case 39:
#line 295 "typeAnalysis/typeAnalysis.y"
{ 
            /* Don't handle this yet */
	    yyval.nonterm = globalType;
	;
    break;}
case 40:
#line 299 "typeAnalysis/typeAnalysis.y"
{
            yyval.nonterm = yyvsp[0].nonterm;
	;
    break;}
case 41:
#line 302 "typeAnalysis/typeAnalysis.y"
{
            yyval.nonterm = yyvsp[0].nonterm;
        ;
    break;}
case 42:
#line 305 "typeAnalysis/typeAnalysis.y"
{
    	    /* Not handling this 100% correct yet */
            yyval.nonterm = yyvsp[0].nonterm;
        ;
    break;}
case 43:
#line 309 "typeAnalysis/typeAnalysis.y"
{if (globalType != POINTER_I ) globalType = POINTER_D;;
    break;}
case 44:
#line 309 "typeAnalysis/typeAnalysis.y"
{            
            yyval.nonterm = yyvsp[0].nonterm;
	;
    break;}
case 45:
#line 312 "typeAnalysis/typeAnalysis.y"
{globalType = yyvsp[0].term->thisSingleOpToken->parameter;
    break;}
case 46:
#line 312 "typeAnalysis/typeAnalysis.y"
{
    	    if (yyvsp[0].nonterm > yyvsp[-2].term->thisSingleOpToken->entireToken)
	        yyval.nonterm = yyvsp[0].nonterm;
	    else 
	        yyval.nonterm = yyvsp[-2].term->thisSingleOpToken->entireToken;
        ;
    break;}
case 47:
#line 318 "typeAnalysis/typeAnalysis.y"
{
            yyval.nonterm = INT_TYPE;    
        ;
    break;}
case 48:
#line 321 "typeAnalysis/typeAnalysis.y"
{
            yyval.nonterm = INT_TYPE;
        ;
    break;}
case 49:
#line 324 "typeAnalysis/typeAnalysis.y"
{binOpType = yyvsp[0].term;
    break;}
case 50:
#line 324 "typeAnalysis/typeAnalysis.y"
{
            yyval.nonterm = yyvsp[-2].term->thisBinOpToken->entireToken
        ;
    break;}
case 51:
#line 330 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = INT_TYPE; ;
    break;}
case 52:
#line 331 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = INT_TYPE; ;
    break;}
case 53:
#line 332 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = INT_TYPE; ;
    break;}
case 54:
#line 333 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = INT_TYPE; ;
    break;}
case 55:
#line 334 "typeAnalysis/typeAnalysis.y"
{globalType = FLOAT_TYPE;;
    break;}
case 56:
#line 334 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = FLOAT_TYPE; ;
    break;}
case 57:
#line 335 "typeAnalysis/typeAnalysis.y"
{globalType = INT_TYPE;;
    break;}
case 58:
#line 335 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = FLOAT_TYPE; ;
    break;}
case 59:
#line 336 "typeAnalysis/typeAnalysis.y"
{globalType = FLOAT_TYPE;;
    break;}
case 60:
#line 336 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = INT_TYPE; ;
    break;}
case 61:
#line 337 "typeAnalysis/typeAnalysis.y"
{globalType = FLOAT_TYPE;;
    break;}
case 62:
#line 337 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = INT_TYPE; ;
    break;}
case 63:
#line 338 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = INT_TYPE; ;
    break;}
case 64:
#line 339 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = FLOAT_TYPE; ;
    break;}
case 65:
#line 340 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = FLOAT_TYPE; ;
    break;}
case 66:
#line 341 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = FLOAT_TYPE; ;
    break;}
case 67:
#line 342 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = FLOAT_TYPE; ;
    break;}
case 68:
#line 343 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = FLOAT_TYPE; ;
    break;}
case 69:
#line 344 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = FLOAT_TYPE; ;
    break;}
case 70:
#line 345 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = FLOAT_TYPE; ;
    break;}
case 71:
#line 346 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = FLOAT_TYPE; ;
    break;}
case 72:
#line 347 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = FLOAT_TYPE; ;
    break;}
case 73:
#line 348 "typeAnalysis/typeAnalysis.y"
{globalType = POINTER_I;;
    break;}
case 74:
#line 348 "typeAnalysis/typeAnalysis.y"
{yyval.nonterm = INT_TYPE; ;
    break;}
case 75:
#line 352 "typeAnalysis/typeAnalysis.y"
{
	    /* We probably want to get my info from this
	       later when we want to do constant propagation
	     */	
            yyval.nonterm = INT_TYPE;
        ;
    break;}
case 76:
#line 358 "typeAnalysis/typeAnalysis.y"
{

	    yyval.nonterm = FLOAT_TYPE;
	;
    break;}
case 77:
#line 362 "typeAnalysis/typeAnalysis.y"
{
   	    yyval.nonterm = POINTER_D;
	;
    break;}
}

#line 811 "/usr/local/lib/bison.cc"
   /* the action file gets copied in in place of this dollarsign  */
  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YY_typeAnalysis_LSP_NEEDED
  yylsp -= yylen;
#endif

#if YY_typeAnalysis_DEBUG != 0
  if (YY_typeAnalysis_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YY_typeAnalysis_LSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = YY_typeAnalysis_LLOC.first_line;
      yylsp->first_column = YY_typeAnalysis_LLOC.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  YYGOTO(yynewstate);

YYLABEL(yyerrlab)   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++YY_typeAnalysis_NERRS;

#ifdef YY_typeAnalysis_ERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      YY_typeAnalysis_ERROR(msg);
	      free(msg);
	    }
	  else
	    YY_typeAnalysis_ERROR ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YY_typeAnalysis_ERROR_VERBOSE */
	YY_typeAnalysis_ERROR("parse error");
    }

  YYGOTO(yyerrlab1);
YYLABEL(yyerrlab1)   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (YY_typeAnalysis_CHAR == YYEOF)
	YYABORT;

#if YY_typeAnalysis_DEBUG != 0
      if (YY_typeAnalysis_DEBUG_FLAG)
	fprintf(stderr, "Discarding token %d (%s).\n", YY_typeAnalysis_CHAR, yytname[yychar1]);
#endif

      YY_typeAnalysis_CHAR = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;              /* Each real token shifted decrements this */

  YYGOTO(yyerrhandle);

YYLABEL(yyerrdefault)  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) YYGOTO(yydefault);
#endif

YYLABEL(yyerrpop)   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YY_typeAnalysis_LSP_NEEDED
  yylsp--;
#endif

#if YY_typeAnalysis_DEBUG != 0
  if (YY_typeAnalysis_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

YYLABEL(yyerrhandle)

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    YYGOTO(yyerrdefault);

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    YYGOTO(yyerrdefault);

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	YYGOTO(yyerrpop);
      yyn = -yyn;
      YYGOTO(yyreduce);
    }
  else if (yyn == 0)
    YYGOTO(yyerrpop);

  if (yyn == YYFINAL)
    YYACCEPT;

#if YY_typeAnalysis_DEBUG != 0
  if (YY_typeAnalysis_DEBUG_FLAG)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = YY_typeAnalysis_LVAL;
#ifdef YY_typeAnalysis_LSP_NEEDED
  *++yylsp = YY_typeAnalysis_LLOC;
#endif

  yystate = yyn;
  YYGOTO(yynewstate);
/* end loop, in which YYGOTO may be used. */
  YYENDGOTO
}

/* END */

/* #line 1010 "/usr/local/lib/bison.cc" */
#line 1978 "typeAnalysis/typeAnalysis.cc"
#line 367 "typeAnalysis/typeAnalysis.y"


/*==============================================================================
 * FUNCTION:        yylex
 * OVERVIEW:        The scanner driver than returns the next token.
 * PARAMETERS:      <none>
 * RETURNS:         the next token
 *============================================================================*/
int typeAnalysis::yylex()
{
    if (inLex->tokenIt == inLex->tokenList.end())
        return EOL;
 
    yylval.term = &(*(inLex->tokenIt)++);
    yylval.term->memAddress = inLex->memAddress;
    yylval.term->instructNumber = inLex->instructNumber;
    return yylval.term->tokenValue;
}

typeAnalysis::~typeAnalysis(){
}

void typeAnalysis::yyerror(char * inString){
}

void typeAnalysis::savePrevType(Byte inRegNum, varType inType){
    prevRegNum = inRegNum;
    prevType = inType;      
    // Note that prevType and prevRegNum are 
    // temporary. Only use within the same instruction
}

void typeAnalysis::rollbackToPrevType(varType inType){
    if (inType > prevType){
        inBlock->setTypeOfLastChainForReg(prevRegNum, inType);    
    }
    else{
        inBlock->setTypeOfLastChainForReg(prevRegNum, prevType);
    }
}
