#define YY_SSLParser_h_included

/*  A Bison++ parser, made from rtl/sslparser.y  */

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
#line 85 "rtl/sslparser.cc"
#line 78 "rtl/sslparser.y"

#include "global.h"
#include "rttelem.h"
#include "hrtl.h"
#define IntType 1
#define VectorType 2
#ifdef WIN32
#include <malloc.h>
#endif
class SSLScanner;

#line 95 "rtl/sslparser.y"
typedef union {
    SSElem*         ss;
    char*           str;
    int             num;
    double          dbl;
    RTTElem*        regtransfer;
    
    Table*          tab;
    InsNameElem*    insel;
    list<int>*      parmlist;
    list<string>*   strlist;
    deque<SSElem*>* exprlist;
    deque<string>*  namelist;
    list<SSElem*>*  sslist;
    RTTListElem*    rtlist;
    list<SSElem*>*  spreglist;      // Was list of SpecialReg*
} yy_SSLParser_stype;
#define YY_SSLParser_STYPE yy_SSLParser_stype
#line 114 "rtl/sslparser.y"

#include "sslscanner.h"
#define YY_SSLParser_DEBUG  1 
#define YY_SSLParser_PARSE_PARAM  \
    RTLInstDict& Dict
#define YY_SSLParser_CONSTRUCTOR_PARAM  \
    const string& sslFile, \
    bool trace
#define YY_SSLParser_CONSTRUCTOR_INIT  : \
   sslFile(sslFile), bFloat(false)
#define YY_SSLParser_CONSTRUCTOR_CODE  \
    FILE* inFile = fopen(sslFile.c_str(),"r"); \
    if (inFile == NULL) { \
        ostrstream ost; \
        ost << "can't open `" << sslFile << "' for reading"; \
        error(str(ost)); \
    } \
    theScanner = new SSLScanner(inFile, trace); \
    if (trace) yydebug = 1;
#define YY_SSLParser_MEMBERS  \
public: \
        virtual ~SSLParser(); \
protected: \
\
    /** \
     * The scanner. \
     */ \
    SSLScanner* theScanner; \
\
    /** \
     * The file from which the SSL spec is read. \
     */ \
    string sslFile; \
\
    /** \
     * Maps SSL constants to their values. \
     */ \
    map<string,int,StrCmp> ConstTable; \
\
    /** \
     * Map from expands to index (e.g. COND). \
     */ \
    map<string, int, StrCmp> ExpandMap; \
\
    /** \
     * Map temp reg names to index. \
     */ \
    map<string, int, StrCmp> TempMap; \
\
    /** \
     * maps index names to instruction name-elements \
     */ \
    map<string, InsNameElem*, StrCmp> indexrefmap; \
\
    /** \
     * Maps table names to Table's.\
     */ \
    map<string, Table*, StrCmp> TableDict; \
\
    /* \
     * True when FLOAT keyword seen; false when INTEGER keyword seen \
     * (in @REGISTER section) \
     */ \
    bool bFloat;

#line 73 "/usr/local/lib/bison.cc"
/* %{ and %header{ and %union, during decl */
#define YY_SSLParser_BISON 1
#ifndef YY_SSLParser_COMPATIBILITY
#ifndef YY_USE_CLASS
#define  YY_SSLParser_COMPATIBILITY 1
#else
#define  YY_SSLParser_COMPATIBILITY 0
#endif
#endif

#if YY_SSLParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YYLTYPE
#ifndef YY_SSLParser_LTYPE
#define YY_SSLParser_LTYPE YYLTYPE
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_SSLParser_STYPE 
#define YY_SSLParser_STYPE YYSTYPE
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_SSLParser_DEBUG
#define  YY_SSLParser_DEBUG YYDEBUG
#endif
#endif
#ifdef YY_SSLParser_STYPE
#ifndef yystype
#define yystype YY_SSLParser_STYPE
#endif
#endif
/* use goto to be compatible */
#ifndef YY_SSLParser_USE_GOTO
#define YY_SSLParser_USE_GOTO 1
#endif
#endif

/* use no goto to be clean in C++ */
#ifndef YY_SSLParser_USE_GOTO
#define YY_SSLParser_USE_GOTO 0
#endif

#ifndef YY_SSLParser_PURE

/* #line 117 "/usr/local/lib/bison.cc" */
#line 230 "rtl/sslparser.cc"

#line 117 "/usr/local/lib/bison.cc"
/*  YY_SSLParser_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 121 "/usr/local/lib/bison.cc" */
#line 239 "rtl/sslparser.cc"

#line 121 "/usr/local/lib/bison.cc"
/* prefix */
#ifndef YY_SSLParser_DEBUG

/* #line 123 "/usr/local/lib/bison.cc" */
#line 246 "rtl/sslparser.cc"

#line 123 "/usr/local/lib/bison.cc"
/* YY_SSLParser_DEBUG */
#endif


#ifndef YY_SSLParser_LSP_NEEDED

/* #line 128 "/usr/local/lib/bison.cc" */
#line 256 "rtl/sslparser.cc"

#line 128 "/usr/local/lib/bison.cc"
 /* YY_SSLParser_LSP_NEEDED*/
#endif



/* DEFAULT LTYPE*/
#ifdef YY_SSLParser_LSP_NEEDED
#ifndef YY_SSLParser_LTYPE
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

#define YY_SSLParser_LTYPE yyltype
#endif
#endif
/* DEFAULT STYPE*/
      /* We used to use `unsigned long' as YY_SSLParser_STYPE on MSDOS,
	 but it seems better to be consistent.
	 Most programs should declare their own type anyway.  */

#ifndef YY_SSLParser_STYPE
#define YY_SSLParser_STYPE int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_SSLParser_PARSE
#define YY_SSLParser_PARSE yyparse
#endif
#ifndef YY_SSLParser_LEX
#define YY_SSLParser_LEX yylex
#endif
#ifndef YY_SSLParser_LVAL
#define YY_SSLParser_LVAL yylval
#endif
#ifndef YY_SSLParser_LLOC
#define YY_SSLParser_LLOC yylloc
#endif
#ifndef YY_SSLParser_CHAR
#define YY_SSLParser_CHAR yychar
#endif
#ifndef YY_SSLParser_NERRS
#define YY_SSLParser_NERRS yynerrs
#endif
#ifndef YY_SSLParser_DEBUG_FLAG
#define YY_SSLParser_DEBUG_FLAG yydebug
#endif
#ifndef YY_SSLParser_ERROR
#define YY_SSLParser_ERROR yyerror
#endif
#ifndef YY_SSLParser_PARSE_PARAM
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
#define YY_SSLParser_PARSE_PARAM
#ifndef YY_SSLParser_PARSE_PARAM_DEF
#define YY_SSLParser_PARSE_PARAM_DEF
#endif
#endif
#endif
#endif
#ifndef YY_SSLParser_PARSE_PARAM
#define YY_SSLParser_PARSE_PARAM void
#endif
#endif
#if YY_SSLParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YY_SSLParser_LTYPE
#ifndef YYLTYPE
#define YYLTYPE YY_SSLParser_LTYPE
#else
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
#endif
#endif
#ifndef YYSTYPE
#define YYSTYPE YY_SSLParser_STYPE
#else
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
#endif
#ifdef YY_SSLParser_PURE
#ifndef YYPURE
#define YYPURE YY_SSLParser_PURE
#endif
#endif
#ifdef YY_SSLParser_DEBUG
#ifndef YYDEBUG
#define YYDEBUG YY_SSLParser_DEBUG 
#endif
#endif
#ifndef YY_SSLParser_ERROR_VERBOSE
#ifdef YYERROR_VERBOSE
#define YY_SSLParser_ERROR_VERBOSE YYERROR_VERBOSE
#endif
#endif
#ifndef YY_SSLParser_LSP_NEEDED
#ifdef YYLSP_NEEDED
#define YY_SSLParser_LSP_NEEDED YYLSP_NEEDED
#endif
#endif
#endif
#ifndef YY_USE_CLASS
/* TOKEN C */

/* #line 236 "/usr/local/lib/bison.cc" */
#line 369 "rtl/sslparser.cc"
#define	ASSIGNSIZE	258
#define	COND_OP	259
#define	BIT_OP	260
#define	ARITH_OP	261
#define	LOG_OP	262
#define	NAME	263
#define	REG_ID	264
#define	COND_TNAME	265
#define	DECOR	266
#define	FARITH_OP	267
#define	FPUSH	268
#define	FPOP	269
#define	TEMP	270
#define	SHARES	271
#define	CONV_FUNC	272
#define	TRANSCEND	273
#define	BIG	274
#define	LITTLE	275
#define	NAME_CALL	276
#define	NAME_LOOKUP	277
#define	INFINITESTACK	278
#define	ENDIANNESS	279
#define	COVERS	280
#define	INDEX	281
#define	NOT	282
#define	THEN	283
#define	LOOKUP_RDC	284
#define	BOGUS	285
#define	ASSIGN	286
#define	TO	287
#define	COLON	288
#define	S_E	289
#define	AT	290
#define	ADDR	291
#define	REG_IDX	292
#define	EQUATE	293
#define	MEM_IDX	294
#define	INTEGER	295
#define	FLOAT	296
#define	FAST	297
#define	OPERAND	298
#define	FETCHEXEC	299
#define	CAST_OP	300
#define	FLAGMACRO	301
#define	NUM	302
#define	FLOATNUM	303


#line 236 "/usr/local/lib/bison.cc"
 /* #defines tokens */
#else
/* CLASS */
#ifndef YY_SSLParser_CLASS
#define YY_SSLParser_CLASS SSLParser
#endif
#ifndef YY_SSLParser_INHERIT
#define YY_SSLParser_INHERIT
#endif
#ifndef YY_SSLParser_MEMBERS
#define YY_SSLParser_MEMBERS 
#endif
#ifndef YY_SSLParser_LEX_BODY
#define YY_SSLParser_LEX_BODY  
#endif
#ifndef YY_SSLParser_ERROR_BODY
#define YY_SSLParser_ERROR_BODY  
#endif
#ifndef YY_SSLParser_CONSTRUCTOR_PARAM
#define YY_SSLParser_CONSTRUCTOR_PARAM
#endif
#ifndef YY_SSLParser_CONSTRUCTOR_CODE
#define YY_SSLParser_CONSTRUCTOR_CODE
#endif
#ifndef YY_SSLParser_CONSTRUCTOR_INIT
#define YY_SSLParser_CONSTRUCTOR_INIT
#endif
/* choose between enum and const */
#ifndef YY_SSLParser_USE_CONST_TOKEN
#define YY_SSLParser_USE_CONST_TOKEN 0
/* yes enum is more compatible with flex,  */
/* so by default we use it */ 
#endif
#if YY_SSLParser_USE_CONST_TOKEN != 0
#ifndef YY_SSLParser_ENUM_TOKEN
#define YY_SSLParser_ENUM_TOKEN yy_SSLParser_enum_token
#endif
#endif

class YY_SSLParser_CLASS YY_SSLParser_INHERIT
{
public: 
#if YY_SSLParser_USE_CONST_TOKEN != 0
/* static const int token ... */

/* #line 280 "/usr/local/lib/bison.cc" */
#line 465 "rtl/sslparser.cc"
static const int ASSIGNSIZE;
static const int COND_OP;
static const int BIT_OP;
static const int ARITH_OP;
static const int LOG_OP;
static const int NAME;
static const int REG_ID;
static const int COND_TNAME;
static const int DECOR;
static const int FARITH_OP;
static const int FPUSH;
static const int FPOP;
static const int TEMP;
static const int SHARES;
static const int CONV_FUNC;
static const int TRANSCEND;
static const int BIG;
static const int LITTLE;
static const int NAME_CALL;
static const int NAME_LOOKUP;
static const int INFINITESTACK;
static const int ENDIANNESS;
static const int COVERS;
static const int INDEX;
static const int NOT;
static const int THEN;
static const int LOOKUP_RDC;
static const int BOGUS;
static const int ASSIGN;
static const int TO;
static const int COLON;
static const int S_E;
static const int AT;
static const int ADDR;
static const int REG_IDX;
static const int EQUATE;
static const int MEM_IDX;
static const int INTEGER;
static const int FLOAT;
static const int FAST;
static const int OPERAND;
static const int FETCHEXEC;
static const int CAST_OP;
static const int FLAGMACRO;
static const int NUM;
static const int FLOATNUM;


#line 280 "/usr/local/lib/bison.cc"
 /* decl const */
#else
enum YY_SSLParser_ENUM_TOKEN { YY_SSLParser_NULL_TOKEN=0

/* #line 283 "/usr/local/lib/bison.cc" */
#line 520 "rtl/sslparser.cc"
	,ASSIGNSIZE=258
	,COND_OP=259
	,BIT_OP=260
	,ARITH_OP=261
	,LOG_OP=262
	,NAME=263
	,REG_ID=264
	,COND_TNAME=265
	,DECOR=266
	,FARITH_OP=267
	,FPUSH=268
	,FPOP=269
	,TEMP=270
	,SHARES=271
	,CONV_FUNC=272
	,TRANSCEND=273
	,BIG=274
	,LITTLE=275
	,NAME_CALL=276
	,NAME_LOOKUP=277
	,INFINITESTACK=278
	,ENDIANNESS=279
	,COVERS=280
	,INDEX=281
	,NOT=282
	,THEN=283
	,LOOKUP_RDC=284
	,BOGUS=285
	,ASSIGN=286
	,TO=287
	,COLON=288
	,S_E=289
	,AT=290
	,ADDR=291
	,REG_IDX=292
	,EQUATE=293
	,MEM_IDX=294
	,INTEGER=295
	,FLOAT=296
	,FAST=297
	,OPERAND=298
	,FETCHEXEC=299
	,CAST_OP=300
	,FLAGMACRO=301
	,NUM=302
	,FLOATNUM=303


#line 283 "/usr/local/lib/bison.cc"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_SSLParser_PARSE (YY_SSLParser_PARSE_PARAM);
 virtual void YY_SSLParser_ERROR(char *msg) YY_SSLParser_ERROR_BODY;
#ifdef YY_SSLParser_PURE
#ifdef YY_SSLParser_LSP_NEEDED
 virtual int  YY_SSLParser_LEX (YY_SSLParser_STYPE *YY_SSLParser_LVAL,YY_SSLParser_LTYPE *YY_SSLParser_LLOC) YY_SSLParser_LEX_BODY;
#else
 virtual int  YY_SSLParser_LEX (YY_SSLParser_STYPE *YY_SSLParser_LVAL) YY_SSLParser_LEX_BODY;
#endif
#else
 virtual int YY_SSLParser_LEX() YY_SSLParser_LEX_BODY;
 YY_SSLParser_STYPE YY_SSLParser_LVAL;
#ifdef YY_SSLParser_LSP_NEEDED
 YY_SSLParser_LTYPE YY_SSLParser_LLOC;
#endif
 int   YY_SSLParser_NERRS;
 int    YY_SSLParser_CHAR;
#endif
#if YY_SSLParser_DEBUG != 0
 int YY_SSLParser_DEBUG_FLAG;   /*  nonzero means print parse trace     */
#endif
public:
 YY_SSLParser_CLASS(YY_SSLParser_CONSTRUCTOR_PARAM);
public:
 YY_SSLParser_MEMBERS 
};
/* other declare folow */
#if YY_SSLParser_USE_CONST_TOKEN != 0

/* #line 314 "/usr/local/lib/bison.cc" */
#line 603 "rtl/sslparser.cc"
const int YY_SSLParser_CLASS::ASSIGNSIZE=258;
const int YY_SSLParser_CLASS::COND_OP=259;
const int YY_SSLParser_CLASS::BIT_OP=260;
const int YY_SSLParser_CLASS::ARITH_OP=261;
const int YY_SSLParser_CLASS::LOG_OP=262;
const int YY_SSLParser_CLASS::NAME=263;
const int YY_SSLParser_CLASS::REG_ID=264;
const int YY_SSLParser_CLASS::COND_TNAME=265;
const int YY_SSLParser_CLASS::DECOR=266;
const int YY_SSLParser_CLASS::FARITH_OP=267;
const int YY_SSLParser_CLASS::FPUSH=268;
const int YY_SSLParser_CLASS::FPOP=269;
const int YY_SSLParser_CLASS::TEMP=270;
const int YY_SSLParser_CLASS::SHARES=271;
const int YY_SSLParser_CLASS::CONV_FUNC=272;
const int YY_SSLParser_CLASS::TRANSCEND=273;
const int YY_SSLParser_CLASS::BIG=274;
const int YY_SSLParser_CLASS::LITTLE=275;
const int YY_SSLParser_CLASS::NAME_CALL=276;
const int YY_SSLParser_CLASS::NAME_LOOKUP=277;
const int YY_SSLParser_CLASS::INFINITESTACK=278;
const int YY_SSLParser_CLASS::ENDIANNESS=279;
const int YY_SSLParser_CLASS::COVERS=280;
const int YY_SSLParser_CLASS::INDEX=281;
const int YY_SSLParser_CLASS::NOT=282;
const int YY_SSLParser_CLASS::THEN=283;
const int YY_SSLParser_CLASS::LOOKUP_RDC=284;
const int YY_SSLParser_CLASS::BOGUS=285;
const int YY_SSLParser_CLASS::ASSIGN=286;
const int YY_SSLParser_CLASS::TO=287;
const int YY_SSLParser_CLASS::COLON=288;
const int YY_SSLParser_CLASS::S_E=289;
const int YY_SSLParser_CLASS::AT=290;
const int YY_SSLParser_CLASS::ADDR=291;
const int YY_SSLParser_CLASS::REG_IDX=292;
const int YY_SSLParser_CLASS::EQUATE=293;
const int YY_SSLParser_CLASS::MEM_IDX=294;
const int YY_SSLParser_CLASS::INTEGER=295;
const int YY_SSLParser_CLASS::FLOAT=296;
const int YY_SSLParser_CLASS::FAST=297;
const int YY_SSLParser_CLASS::OPERAND=298;
const int YY_SSLParser_CLASS::FETCHEXEC=299;
const int YY_SSLParser_CLASS::CAST_OP=300;
const int YY_SSLParser_CLASS::FLAGMACRO=301;
const int YY_SSLParser_CLASS::NUM=302;
const int YY_SSLParser_CLASS::FLOATNUM=303;


#line 314 "/usr/local/lib/bison.cc"
 /* const YY_SSLParser_CLASS::token */
#endif
/*apres const  */
YY_SSLParser_CLASS::YY_SSLParser_CLASS(YY_SSLParser_CONSTRUCTOR_PARAM) YY_SSLParser_CONSTRUCTOR_INIT
{
#if YY_SSLParser_DEBUG != 0
YY_SSLParser_DEBUG_FLAG=0;
#endif
YY_SSLParser_CONSTRUCTOR_CODE;
};
#endif

/* #line 325 "/usr/local/lib/bison.cc" */
#line 666 "rtl/sslparser.cc"


#define	YYFINAL		285
#define	YYFLAG		-32768
#define	YYNTBASE	62

#define YYTRANSLATE(x) ((unsigned)(x) <= 303 ? yytranslate[x] : 110)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    56,     2,    58,     2,     2,    57,    60,
    55,     2,     2,    50,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    49,     2,
     2,     2,    61,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    53,     2,    54,     2,    59,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    51,     2,    52,     2,     2,     2,     2,     2,
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
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
    46,    47,    48
};

#if YY_SSLParser_DEBUG != 0
static const short yyprhs[] = {     0,
     0,     4,     7,     9,    12,    14,    16,    18,    20,    22,
    24,    26,    29,    33,    35,    41,    47,    51,    52,    55,
    59,    61,    63,    67,    68,    72,    73,    77,    81,    83,
    87,    94,   105,   120,   131,   140,   144,   146,   153,   157,
   163,   167,   169,   171,   173,   176,   178,   182,   187,   189,
   193,   195,   199,   203,   206,   208,   210,   212,   214,   218,
   224,   228,   232,   238,   242,   243,   248,   250,   253,   255,
   257,   260,   264,   268,   272,   277,   282,   286,   289,   291,
   293,   297,   301,   304,   306,   310,   312,   316,   318,   319,
   321,   325,   327,   328,   335,   340,   342,   344,   347,   349,
   351,   353,   357,   359,   367,   376,   380,   388,   390,   392,
   396,   400,   404,   407,   410,   413,   417,   421,   425,   429,
   433,   439,   441,   443,   447,   451,   453,   461,   464,   468,
   471,   473,   475,   478,   482,   484
};

static const short yyrhs[] = {    62,
    63,    49,     0,    63,    49,     0,    89,     0,    44,    94,
     0,    77,     0,    78,     0,   105,     0,   107,     0,    70,
     0,    67,     0,    76,     0,    43,    64,     0,    64,    50,
    65,     0,    65,     0,    98,    38,    51,    97,    52,     0,
    98,    97,    66,     3,   102,     0,    53,    97,    54,     0,
     0,    68,     8,     0,     8,    31,    47,     0,    69,     0,
    23,     0,     9,    28,   102,     0,     0,    40,    71,    73,
     0,     0,    41,    72,    73,     0,    73,    50,    74,     0,
    74,     0,     9,    26,    47,     0,     9,    53,    47,    54,
    26,    47,     0,     9,    53,    47,    54,    26,    47,    25,
     9,    32,     9,     0,     9,    53,    47,    54,    26,    47,
    16,     9,    35,    53,    47,    32,    47,    54,     0,    53,
    75,    54,    53,    47,    54,    26,    47,    32,    47,     0,
    53,    75,    54,    53,    47,    54,    26,    47,     0,    75,
    50,     9,     0,     9,     0,    21,    97,    55,    51,    94,
    52,     0,     8,    38,    47,     0,     8,    38,    47,     6,
    47,     0,     8,    38,    79,     0,    80,     0,    85,     0,
    87,     0,    80,    82,     0,    82,     0,    81,    50,    80,
     0,    81,    50,    56,    56,     0,    80,     0,    51,    81,
    52,     0,    83,     0,    57,     8,    57,     0,    56,     8,
    56,     0,    58,     8,     0,     8,     0,     5,     0,     6,
     0,    12,     0,    51,    86,    52,     0,    86,    50,    56,
    84,    56,     0,    56,    84,    56,     0,    51,    88,    52,
     0,    88,    50,    56,   102,    56,     0,    56,   102,    56,
     0,     0,    91,    90,    97,    94,     0,    92,     0,    91,
    11,     0,     8,     0,    93,     0,    92,    93,     0,    57,
     8,    57,     0,    22,    47,    54,     0,    22,     8,    54,
     0,    58,    22,    47,    54,     0,    58,    22,     8,    54,
     0,    56,     8,    56,     0,    94,    95,     0,    95,     0,
   100,     0,    21,    99,    55,     0,    46,    96,    55,     0,
    46,    55,     0,    59,     0,    96,    50,     9,     0,     9,
     0,    97,    50,    98,     0,    98,     0,     0,     8,     0,
    99,    50,   102,     0,   102,     0,     0,     3,   102,    28,
   103,    38,   102,     0,     3,   103,    38,   102,     0,    13,
     0,    14,     0,     3,   102,     0,    47,     0,    48,     0,
    15,     0,    60,   102,    55,     0,   103,     0,    53,   102,
    61,   102,    33,   102,    54,     0,    53,   102,    61,   102,
    33,   102,    54,   104,     0,    36,   102,    55,     0,    17,
    47,    50,    47,    50,   102,    55,     0,    13,     0,    14,
     0,    18,   102,    55,     0,    22,     8,    54,     0,    21,
    99,    55,     0,   102,    34,     0,   102,   104,     0,    27,
   102,     0,   102,    12,   102,     0,   102,     6,   102,     0,
   102,     5,   102,     0,   102,     4,   102,     0,   102,     7,
   102,     0,   102,    22,     8,    54,   101,     0,   101,     0,
     9,     0,    37,   102,    54,     0,    39,   102,    54,     0,
     8,     0,   102,    35,    53,   102,    33,   102,    54,     0,
   103,    57,     0,    51,    47,    52,     0,    24,   106,     0,
    19,     0,    20,     0,    42,   108,     0,   108,    50,   109,
     0,   109,     0,     8,    26,     8,     0
};

#endif

#if YY_SSLParser_DEBUG != 0
static const short yyrline[] = { 0,
   236,   238,   241,   244,   250,   252,   254,   256,   258,   260,
   262,   264,   268,   270,   273,   279,   301,   302,   305,   313,
   322,   326,   333,   362,   366,   366,   369,   371,   372,   375,
   381,   386,   421,   443,   458,   469,   473,   479,   490,   497,
   510,   516,   521,   525,   531,   542,   547,   556,   559,   564,
   568,   573,   579,   582,   597,   615,   620,   624,   629,   635,
   640,   646,   652,   657,   663,   666,   688,   692,   715,   719,
   722,   728,   732,   743,   752,   763,   772,   777,   787,   794,
   798,   809,   812,   815,   820,   829,   840,   847,   851,   856,
   864,   869,   874,   879,   883,   890,   893,   896,   901,   909,
   920,   929,   933,   937,   946,   960,   969,   991,   994,   999,
  1019,  1041,  1072,  1080,  1090,  1097,  1105,  1113,  1121,  1129,
  1137,  1161,  1166,  1192,  1199,  1206,  1226,  1234,  1242,  1248,
  1253,  1257,  1264,  1268,  1271,  1275
};

static const char * const yytname[] = {   "$","error","$illegal.","ASSIGNSIZE",
"COND_OP","BIT_OP","ARITH_OP","LOG_OP","NAME","REG_ID","COND_TNAME","DECOR",
"FARITH_OP","FPUSH","FPOP","TEMP","SHARES","CONV_FUNC","TRANSCEND","BIG","LITTLE",
"NAME_CALL","NAME_LOOKUP","INFINITESTACK","ENDIANNESS","COVERS","INDEX","NOT",
"THEN","LOOKUP_RDC","BOGUS","ASSIGN","TO","COLON","S_E","AT","ADDR","REG_IDX",
"EQUATE","MEM_IDX","INTEGER","FLOAT","FAST","OPERAND","FETCHEXEC","CAST_OP",
"FLAGMACRO","NUM","FLOATNUM","';'","','","'{'","'}'","'['","']'","')'","'\"'",
"'\\''","'$'","'_'","'('","'?'","specification","parts","operandlist","operand",
"func_parameter","definition","internal_type","aliases","reglist","@1","@2",
"a_reglists","a_reglist","reg_table","flag_fnc","constants","table_assign","table_expr",
"str_expr","str_array","str_term","name_expand","bin_oper","opstr_expr","opstr_array",
"exprstr_expr","exprstr_array","instr","@3","instr_name","instr_elem","name_contract",
"rt_list","rt","flag_list","list_parameter","param","list_actualparameter","assign_rt",
"exp_term","exp","var_op","cast","endianness","esize","fastlist","fastentries",
"fastentry",""
};
#endif

static const short yyr1[] = {     0,
    62,    62,    63,    63,    63,    63,    63,    63,    63,    63,
    63,    63,    64,    64,    65,    65,    66,    66,    67,    67,
    67,    68,    69,    71,    70,    72,    70,    73,    73,    74,
    74,    74,    74,    74,    74,    75,    75,    76,    77,    77,
    78,    79,    79,    79,    80,    80,    81,    81,    81,    82,
    82,    83,    83,    83,    83,    84,    84,    84,    85,    86,
    86,    87,    88,    88,    90,    89,    91,    91,    92,    92,
    92,    93,    93,    93,    93,    93,    93,    94,    94,    95,
    95,    95,    95,    95,    96,    96,    97,    97,    97,    98,
    99,    99,    99,   100,   100,   100,   100,   100,   101,   101,
   101,   101,   101,   101,   101,   101,   101,   101,   101,   101,
   101,   101,   102,   102,   102,   102,   102,   102,   102,   102,
   102,   102,   103,   103,   103,   103,   103,   103,   104,   105,
   106,   106,   107,   108,   108,   109
};

static const short yyr2[] = {     0,
     3,     2,     1,     2,     1,     1,     1,     1,     1,     1,
     1,     2,     3,     1,     5,     5,     3,     0,     2,     3,
     1,     1,     3,     0,     3,     0,     3,     3,     1,     3,
     6,    10,    14,    10,     8,     3,     1,     6,     3,     5,
     3,     1,     1,     1,     2,     1,     3,     4,     1,     3,
     1,     3,     3,     2,     1,     1,     1,     1,     3,     5,
     3,     3,     5,     3,     0,     4,     1,     2,     1,     1,
     2,     3,     3,     3,     4,     4,     3,     2,     1,     1,
     3,     3,     2,     1,     3,     1,     3,     1,     0,     1,
     3,     1,     0,     6,     4,     1,     1,     2,     1,     1,
     1,     3,     1,     7,     8,     3,     7,     1,     1,     3,
     3,     3,     2,     2,     2,     3,     3,     3,     3,     3,
     5,     1,     1,     3,     3,     1,     7,     2,     3,     2,
     1,     1,     2,     3,     1,     3
};

static const short yydefact[] = {     0,
    69,     0,    89,     0,    22,     0,    24,    26,     0,     0,
     0,     0,     0,     0,     0,     0,    10,     0,    21,     9,
    11,     5,     6,     3,    65,    67,    70,     7,     8,     0,
     0,     0,    90,     0,    88,     0,     0,   131,   132,   130,
     0,     0,     0,   133,   135,    12,    14,    89,     0,    96,
    97,    93,     0,    84,     4,    79,    80,     0,     0,     0,
     0,     2,    19,    68,    89,    71,    20,    55,    39,     0,
     0,     0,     0,    41,    42,    46,    51,    43,    44,   126,
   123,   108,   109,   101,     0,     0,    93,     0,     0,     0,
     0,     0,    99,   100,     0,     0,   122,    23,   103,     0,
     0,    74,    73,     0,     0,    25,    29,    27,     0,     0,
     0,     0,    18,    98,   103,     0,    92,    86,    83,     0,
    78,    77,    72,     0,     0,     1,     0,     0,     0,     0,
    49,     0,     0,     0,     0,     0,    54,    45,     0,     0,
     0,     0,   115,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,   113,     0,     0,   114,   128,    87,
     0,     0,     0,    37,     0,     0,   136,   134,    13,    89,
    89,     0,     0,     0,     0,    81,     0,    82,    76,    75,
    66,    40,    56,    57,   126,    58,     0,     0,     0,    50,
     0,    59,     0,    62,    53,    52,     0,   110,   112,   111,
   106,   124,   125,     0,   102,   119,   118,   117,   120,   116,
     0,     0,     0,     0,    30,     0,     0,     0,    28,     0,
     0,     0,     0,   103,    95,    91,    85,    61,    64,     0,
    47,     0,     0,     0,     0,     0,     0,   129,    38,     0,
    36,     0,    15,    17,    16,     0,    48,     0,     0,     0,
     0,   121,     0,     0,     0,    94,    60,    63,     0,     0,
     0,    31,     0,   107,   104,   127,     0,     0,     0,   105,
     0,     0,    35,     0,     0,     0,     0,    32,    34,     0,
     0,     0,    33,     0,     0
};

static const short yydefgoto[] = {    15,
    16,    46,    47,   172,    17,    18,    19,    20,    41,    42,
   106,   107,   165,    21,    22,    23,    74,   131,   132,    76,
    77,   187,    78,   133,    79,   134,    24,    65,    25,    26,
    27,    55,    56,   120,    34,    35,   116,    57,    97,   117,
    99,   158,    28,    40,    29,    44,    45
};

static const short yypact[] = {   335,
    -8,     7,    12,     5,-32768,    94,-32768,-32768,    35,    12,
    15,    43,    67,    64,    25,    74,-32768,   117,-32768,-32768,
-32768,-32768,-32768,-32768,   135,   184,-32768,-32768,-32768,   123,
   136,   254,-32768,    -5,-32768,   110,   137,-32768,-32768,-32768,
     3,     3,   102,   129,-32768,   145,-32768,     6,   254,-32768,
-32768,   254,    18,-32768,    15,-32768,-32768,   143,   152,    23,
   163,-32768,-32768,-32768,    12,-32768,-32768,-32768,   194,    33,
   200,   203,   212,-32768,    80,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   177,   254,   254,   219,   254,   254,
   254,   254,-32768,-32768,   254,   254,-32768,   478,   171,    12,
   182,-32768,-32768,   -11,   240,   201,-32768,   201,   242,    35,
    12,   204,   -31,     4,   -17,    46,   478,-32768,-32768,    54,
-32768,-32768,-32768,   198,   205,-32768,    89,   210,    80,   217,
    80,    97,   125,   195,   202,   209,-32768,-32768,   211,   282,
    95,   206,    65,   291,   382,   403,    73,   316,   254,   254,
   254,   254,   254,   265,-32768,   221,   232,-32768,-32768,-32768,
    15,   233,   235,-32768,    22,     3,-32768,-32768,-32768,    12,
    12,   275,   254,   254,   254,-32768,   274,-32768,-32768,-32768,
    15,-32768,-32768,-32768,   202,-32768,   228,   147,   159,-32768,
   229,-32768,   236,-32768,-32768,-32768,   252,-32768,-32768,-32768,
-32768,-32768,-32768,   254,-32768,   497,   162,   154,   497,    65,
   246,   254,   253,   119,-32768,   255,   297,   257,-32768,   196,
    61,   254,   478,    55,   478,   478,-32768,-32768,-32768,    -3,
    80,   231,   254,   258,   444,   254,   464,-32768,-32768,   285,
-32768,   268,-32768,-32768,   478,   254,-32768,   256,   151,   254,
   254,-32768,   254,   271,   270,   478,-32768,-32768,   348,   407,
   440,    81,   293,-32768,   276,-32768,   320,   321,   284,-32768,
   299,   300,   303,   283,   330,   294,   298,-32768,-32768,   308,
   301,   295,-32768,   347,-32768
};

static const short yypgoto[] = {-32768,
   346,-32768,   251,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   322,   197,-32768,-32768,-32768,-32768,-32768,   -28,-32768,   -69,
-32768,   133,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   340,   -90,   -54,-32768,   -41,    -6,   281,-32768,   138,   -32,
   -47,   104,-32768,-32768,-32768,-32768,   262
};


#define	YYLAST		548


static const short yytable[] = {    98,
   121,   115,    75,    48,   135,   138,   113,   149,   150,   151,
   152,   104,    36,    33,   162,   153,   114,    49,   100,    33,
   174,   171,    30,   127,   284,   154,   118,    50,    51,    31,
   124,   173,     1,     2,    32,    52,   181,   155,   156,   159,
    68,   163,    43,   112,   100,     3,     4,     5,     6,   101,
    58,    37,   247,   140,   157,   105,   143,   144,   145,   146,
    53,   138,   147,   148,     7,     8,     9,    10,    11,   125,
   214,   217,   119,    54,    59,   218,   149,   150,   151,   152,
    12,    13,    14,   129,   153,    60,   154,    68,   130,    72,
    73,    49,   246,   160,   154,   175,   267,   188,   155,   156,
   176,    50,    51,   177,    48,   268,   155,   156,   178,    52,
   100,   159,    38,    39,   244,   157,   206,   207,   208,   209,
   210,    49,    62,   157,    63,   224,   121,   109,   220,   221,
   129,    50,    51,   204,    53,    71,    72,    73,   100,    52,
   223,   225,   226,    68,   175,    64,   189,    54,   190,   199,
   149,   150,   151,   152,   149,   150,   151,   152,   153,   121,
   231,   138,   153,   102,    53,   153,    68,   151,   154,    67,
   239,   235,   154,   153,   191,   154,   192,    54,   110,   237,
   155,   156,    69,   154,   155,   156,    70,   155,   156,   245,
   103,    71,    72,    73,   111,   155,   156,   157,   122,   128,
   249,   157,   229,   223,   157,     4,   258,   135,   123,   129,
   136,   126,   157,   256,   230,    72,    73,   259,   260,   137,
   261,   183,   184,   139,   185,    81,   142,   159,   186,    82,
    83,    84,   161,    85,    86,   183,   184,    87,    88,    12,
    13,    14,   186,    89,   193,   100,   194,   243,   164,   167,
   166,   179,    90,    91,   170,    92,   182,   195,   180,   200,
   197,    80,    81,    93,    94,   196,    82,    83,    84,    95,
    85,    86,   211,   212,    87,    88,    96,   222,   213,   215,
    89,   216,   227,   228,   232,   149,   150,   151,   152,    90,
    91,   233,    92,   153,   149,   150,   151,   152,   234,   236,
    93,    94,   153,   154,   238,   241,    95,   250,   240,   242,
   254,   257,   154,    96,   255,   155,   156,   262,   269,   149,
   150,   151,   152,   263,   155,   156,   157,   153,   271,   272,
   273,   275,   157,   274,   276,   277,   198,   154,   278,   281,
   279,   157,     1,     2,   280,   201,   285,   282,   283,   155,
   156,   149,   150,   151,   152,     3,     4,     5,     6,   153,
    61,   169,   219,   108,   248,    66,   157,   141,   270,   154,
   205,   168,     0,   252,     7,     8,     9,    10,    11,     0,
     0,   155,   156,     0,     0,   149,   150,   151,   152,     0,
    12,    13,    14,   153,     0,     0,     0,     0,   157,     0,
     0,     0,   264,   154,     0,     0,   149,   150,   151,   152,
   149,   150,   151,   152,   153,   155,   156,     0,   153,     0,
     0,     0,     0,     0,   154,     0,     0,     0,   154,     0,
     0,     0,   157,     0,     0,   202,   155,   156,     0,     0,
   155,   156,     0,   149,   150,   151,   152,   149,   150,   151,
   152,   153,     0,   157,     0,   153,   203,   157,     0,     0,
   265,   154,     0,     0,     0,   154,     0,   149,   150,   151,
   152,     0,     0,   155,   156,   153,   251,   155,   156,     0,
     0,   149,   150,   151,   152,   154,     0,     0,     0,   153,
   157,     0,     0,   266,   157,     0,   253,   155,   156,   154,
   149,   150,   151,     0,     0,     0,     0,     0,   153,     0,
     0,   155,   156,     0,   157,     0,     0,     0,   154,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   157,     0,
   155,   156,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   157
};

static const short yycheck[] = {    32,
    55,    49,    31,    10,     8,    75,    48,     4,     5,     6,
     7,     9,     8,     8,    26,    12,    49,     3,    50,     8,
    38,    53,    31,    65,     0,    22,     9,    13,    14,    38,
     8,    28,     8,     9,    28,    21,   127,    34,    35,    57,
     8,    53,     8,    38,    50,    21,    22,    23,    24,    55,
     8,    47,    56,    86,    51,    53,    89,    90,    91,    92,
    46,   131,    95,    96,    40,    41,    42,    43,    44,    47,
   161,    50,    55,    59,     8,    54,     4,     5,     6,     7,
    56,    57,    58,    51,    12,    22,    22,     8,    56,    57,
    58,     3,    38,   100,    22,    50,    16,   130,    34,    35,
    55,    13,    14,    50,   111,    25,    34,    35,    55,    21,
    50,    57,    19,    20,    54,    51,   149,   150,   151,   152,
   153,     3,    49,    51,     8,   173,   181,    26,   170,   171,
    51,    13,    14,    61,    46,    56,    57,    58,    50,    21,
   173,   174,   175,     8,    50,    11,    50,    59,    52,    55,
     4,     5,     6,     7,     4,     5,     6,     7,    12,   214,
   189,   231,    12,    54,    46,    12,     8,     6,    22,    47,
    52,   204,    22,    12,    50,    22,    52,    59,    50,   212,
    34,    35,    47,    22,    34,    35,    51,    34,    35,   222,
    54,    56,    57,    58,    50,    34,    35,    51,    56,     6,
   233,    51,    56,   236,    51,    22,    56,     8,    57,    51,
     8,    49,    51,   246,    56,    57,    58,   250,   251,     8,
   253,     5,     6,    47,     8,     9,     8,    57,    12,    13,
    14,    15,    51,    17,    18,     5,     6,    21,    22,    56,
    57,    58,    12,    27,    50,    50,    52,    52,     9,     8,
    50,    54,    36,    37,    51,    39,    47,    56,    54,    54,
    50,     8,     9,    47,    48,    57,    13,    14,    15,    53,
    17,    18,     8,    53,    21,    22,    60,     3,    47,    47,
    27,    47,     9,    56,    56,     4,     5,     6,     7,    36,
    37,    56,    39,    12,     4,     5,     6,     7,    47,    54,
    47,    48,    12,    22,    52,     9,    53,    50,    54,    53,
    26,    56,    22,    60,    47,    34,    35,    47,    26,     4,
     5,     6,     7,    54,    34,    35,    51,    12,     9,     9,
    47,    32,    51,    35,    32,    53,    55,    22,     9,    32,
    47,    51,     8,     9,    47,    55,     0,    47,    54,    34,
    35,     4,     5,     6,     7,    21,    22,    23,    24,    12,
    15,   111,   166,    42,   232,    26,    51,    87,   265,    22,
    55,   110,    -1,   236,    40,    41,    42,    43,    44,    -1,
    -1,    34,    35,    -1,    -1,     4,     5,     6,     7,    -1,
    56,    57,    58,    12,    -1,    -1,    -1,    -1,    51,    -1,
    -1,    -1,    55,    22,    -1,    -1,     4,     5,     6,     7,
     4,     5,     6,     7,    12,    34,    35,    -1,    12,    -1,
    -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,    22,    -1,
    -1,    -1,    51,    -1,    -1,    54,    34,    35,    -1,    -1,
    34,    35,    -1,     4,     5,     6,     7,     4,     5,     6,
     7,    12,    -1,    51,    -1,    12,    54,    51,    -1,    -1,
    54,    22,    -1,    -1,    -1,    22,    -1,     4,     5,     6,
     7,    -1,    -1,    34,    35,    12,    33,    34,    35,    -1,
    -1,     4,     5,     6,     7,    22,    -1,    -1,    -1,    12,
    51,    -1,    -1,    54,    51,    -1,    33,    34,    35,    22,
     4,     5,     6,    -1,    -1,    -1,    -1,    -1,    12,    -1,
    -1,    34,    35,    -1,    51,    -1,    -1,    -1,    22,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    -1,
    34,    35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51
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

#if YY_SSLParser_USE_GOTO != 0
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

#ifdef YY_SSLParser_LSP_NEEDED
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
#define yyclearin       (YY_SSLParser_CHAR = YYEMPTY)
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
  if (YY_SSLParser_CHAR == YYEMPTY && yylen == 1)                               \
    { YY_SSLParser_CHAR = (token), YY_SSLParser_LVAL = (value);                 \
      yychar1 = YYTRANSLATE (YY_SSLParser_CHAR);                                \
      YYPOPSTACK;                                               \
      YYGOTO(yybackup);                                            \
    }                                                           \
  else                                                          \
    { YY_SSLParser_ERROR ("syntax error: cannot back up"); YYERROR; }   \
while (0)

#define YYTERROR        1
#define YYERRCODE       256

#ifndef YY_SSLParser_PURE
/* UNPURE */
#define YYLEX           YY_SSLParser_LEX()
#ifndef YY_USE_CLASS
/* If nonreentrant, and not class , generate the variables here */
int     YY_SSLParser_CHAR;                      /*  the lookahead symbol        */
YY_SSLParser_STYPE      YY_SSLParser_LVAL;              /*  the semantic value of the */
				/*  lookahead symbol    */
int YY_SSLParser_NERRS;                 /*  number of parse errors so far */
#ifdef YY_SSLParser_LSP_NEEDED
YY_SSLParser_LTYPE YY_SSLParser_LLOC;   /*  location data for the lookahead     */
			/*  symbol                              */
#endif
#endif


#else
/* PURE */
#ifdef YY_SSLParser_LSP_NEEDED
#define YYLEX           YY_SSLParser_LEX(&YY_SSLParser_LVAL, &YY_SSLParser_LLOC)
#else
#define YYLEX           YY_SSLParser_LEX(&YY_SSLParser_LVAL)
#endif
#endif
#ifndef YY_USE_CLASS
#if YY_SSLParser_DEBUG != 0
int YY_SSLParser_DEBUG_FLAG;                    /*  nonzero means print parse trace     */
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
 YY_SSLParser_CLASS::
#endif
     YY_SSLParser_PARSE(YY_SSLParser_PARSE_PARAM)
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
/* parameter definition without protypes */
YY_SSLParser_PARSE_PARAM_DEF
#endif
#endif
#endif
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YY_SSLParser_STYPE *yyvsp;
  int yyerrstatus;      /*  number of tokens to shift before error messages enabled */
  int yychar1=0;          /*  lookahead token as an internal (translated) token number */

  short yyssa[YYINITDEPTH];     /*  the state stack                     */
  YY_SSLParser_STYPE yyvsa[YYINITDEPTH];        /*  the semantic value stack            */

  short *yyss = yyssa;          /*  refer to the stacks thru separate pointers */
  YY_SSLParser_STYPE *yyvs = yyvsa;     /*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YY_SSLParser_LSP_NEEDED
  YY_SSLParser_LTYPE yylsa[YYINITDEPTH];        /*  the location stack                  */
  YY_SSLParser_LTYPE *yyls = yylsa;
  YY_SSLParser_LTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YY_SSLParser_PURE
  int YY_SSLParser_CHAR;
  YY_SSLParser_STYPE YY_SSLParser_LVAL;
  int YY_SSLParser_NERRS;
#ifdef YY_SSLParser_LSP_NEEDED
  YY_SSLParser_LTYPE YY_SSLParser_LLOC;
#endif
#endif

  YY_SSLParser_STYPE yyval;             /*  the variable used to return         */
				/*  semantic values from the action     */
				/*  routines                            */

  int yylen;
/* start loop, in which YYGOTO may be used. */
YYBEGINGOTO

#if YY_SSLParser_DEBUG != 0
  if (YY_SSLParser_DEBUG_FLAG)
    fprintf(stderr, "Starting parse\n");
#endif
  yystate = 0;
  yyerrstatus = 0;
  YY_SSLParser_NERRS = 0;
  YY_SSLParser_CHAR = YYEMPTY;          /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YY_SSLParser_LSP_NEEDED
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
      YY_SSLParser_STYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YY_SSLParser_LSP_NEEDED
      YY_SSLParser_LTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YY_SSLParser_LSP_NEEDED
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
#ifdef YY_SSLParser_LSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  YY_SSLParser_ERROR("parser stack overflow");
	  __ALLOCA_return(2);
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) __ALLOCA_alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      __ALLOCA_free(yyss1,yyssa);
      yyvs = (YY_SSLParser_STYPE *) __ALLOCA_alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
      __ALLOCA_free(yyvs1,yyvsa);
#ifdef YY_SSLParser_LSP_NEEDED
      yyls = (YY_SSLParser_LTYPE *) __ALLOCA_alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
      __ALLOCA_free(yyls1,yylsa);
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YY_SSLParser_LSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YY_SSLParser_DEBUG != 0
      if (YY_SSLParser_DEBUG_FLAG)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YY_SSLParser_DEBUG != 0
  if (YY_SSLParser_DEBUG_FLAG)
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

  if (YY_SSLParser_CHAR == YYEMPTY)
    {
#if YY_SSLParser_DEBUG != 0
      if (YY_SSLParser_DEBUG_FLAG)
	fprintf(stderr, "Reading a token: ");
#endif
      YY_SSLParser_CHAR = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (YY_SSLParser_CHAR <= 0)           /* This means end of input. */
    {
      yychar1 = 0;
      YY_SSLParser_CHAR = YYEOF;                /* Don't call YYLEX any more */

#if YY_SSLParser_DEBUG != 0
      if (YY_SSLParser_DEBUG_FLAG)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(YY_SSLParser_CHAR);

#if YY_SSLParser_DEBUG != 0
      if (YY_SSLParser_DEBUG_FLAG)
	{
	  fprintf (stderr, "Next token is %d (%s", YY_SSLParser_CHAR, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, YY_SSLParser_CHAR, YY_SSLParser_LVAL);
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

#if YY_SSLParser_DEBUG != 0
  if (YY_SSLParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting token %d (%s), ", YY_SSLParser_CHAR, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (YY_SSLParser_CHAR != YYEOF)
    YY_SSLParser_CHAR = YYEMPTY;

  *++yyvsp = YY_SSLParser_LVAL;
#ifdef YY_SSLParser_LSP_NEEDED
  *++yylsp = YY_SSLParser_LLOC;
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

#if YY_SSLParser_DEBUG != 0
  if (YY_SSLParser_DEBUG_FLAG)
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
#line 1540 "rtl/sslparser.cc"

  switch (yyn) {

case 4:
#line 244 "rtl/sslparser.y"
{
            map<string, InsNameElem*, StrCmp> m;
            Dict.fetchExecCycle = yyvsp[0].rtlist->eval(TableDict, m);
            delete yyvsp[0].rtlist;
        ;
    break;}
case 12:
#line 264 "rtl/sslparser.y"
{ Dict.fixupParams(); ;
    break;}
case 15:
#line 274 "rtl/sslparser.y"
{
            Dict.DetParamMap[yyvsp[-4].num].params = *yyvsp[-1].parmlist;
            Dict.DetParamMap[yyvsp[-4].num].kind = PARAM_VARIANT;
            delete yyvsp[-1].parmlist;
        ;
    break;}
case 16:
#line 279 "rtl/sslparser.y"
{
            map<string, InsNameElem*, StrCmp> m;
            ParamEntry &param = Dict.DetParamMap[yyvsp[-4].num];
            RTTAssignElem* rt = new RTTAssignElem(new SSListElem(), yyvsp[0].ss, yyvsp[-1].str);
            param.params = *yyvsp[-3].parmlist;
            param.funcParams = *yyvsp[-2].parmlist;
            param.rta = (RTAssgn *)(rt->eval(TableDict, m));
            param.kind = PARAM_EXPR;
            
            if( param.funcParams.size() != 0 ) {
                param.kind = PARAM_LAMBDA;
                /* Change the SemTable entry into a "function call" */
                theSemTable.setItem(yyvsp[-4].num, cFUNCTION,0,0,param.funcParams.size(),
                                    theSemTable[yyvsp[-4].num].sName.c_str());
                
            }
            delete yyvsp[-3].parmlist;
            delete yyvsp[-2].parmlist;
            delete rt;
        ;
    break;}
case 17:
#line 301 "rtl/sslparser.y"
{ yyval.parmlist = yyvsp[-1].parmlist; ;
    break;}
case 18:
#line 302 "rtl/sslparser.y"
{ yyval.parmlist = new list<int>(); ;
    break;}
case 19:
#line 306 "rtl/sslparser.y"
{
                // add this def to the DefMap and create the object
                pair<int, void*> *temp = new pair<int, void*>;              
                temp->first = yyvsp[-1].num;
                temp->second = new vector<int>;
                Dict.DefMap[yyvsp[0].str] = temp;
            ;
    break;}
case 20:
#line 313 "rtl/sslparser.y"
{
                // add the def to the DefMap and create the object
                int *temp_int = new int;
                *temp_int = yyvsp[0].num;
                pair<int,void*> *pair_temp = new pair<int,void*>;
                pair_temp->first = IntType;
                pair_temp->second = temp_int;
                Dict.DefMap[yyvsp[-2].str] = pair_temp;
            ;
    break;}
case 22:
#line 327 "rtl/sslparser.y"
{
                yyval.num = VectorType;
            ;
    break;}
case 23:
#line 334 "rtl/sslparser.y"
{
            
                int temp_reg;   
                map<string, int, StrCmp>::const_iterator it =
                    Dict.RegMap.find(yyvsp[-2].str);
                if (it == Dict.RegMap.end()) {
                    ostrstream ost;
                    ost << "register `" << yyvsp[-2].str << "' is undefined";
                    yyerror(str(ost));
                } else {
                    if ((*it).second == -1) {
                        // A special register, 
                        ostrstream ost;
                        ost << "`" << yyvsp[-2].str << "' is a special register ";
                        ost << " and as such can't be used as an alias";
                        yyerror(str(ost));
                    } else {
                        // An immediate register
                        temp_reg = (*it).second;
                    }
                }
                map<string,InsNameElem*,StrCmp> m; 
                Dict.AliasMap[temp_reg] = yyvsp[0].ss->eval(TableDict, m);
                delete yyvsp[0].ss;
           ;
    break;}
case 24:
#line 363 "rtl/sslparser.y"
{
                    bFloat = false;
                ;
    break;}
case 26:
#line 366 "rtl/sslparser.y"
{
                    bFloat = true;
                ;
    break;}
case 30:
#line 376 "rtl/sslparser.y"
{
                if (Dict.RegMap.find(yyvsp[-2].str) != Dict.RegMap.end())
                    yyerror("Name reglist decared twice\n");
                Dict.RegMap[yyvsp[-2].str] = yyvsp[0].num;
            ;
    break;}
case 31:
#line 381 "rtl/sslparser.y"
{
                if (Dict.RegMap.find(yyvsp[-5].str) != Dict.RegMap.end())
                    yyerror("Name reglist declared twice\n");
                Dict.addRegister( yyvsp[-5].str, yyvsp[0].num, yyvsp[-3].num, bFloat);
            ;
    break;}
case 32:
#line 386 "rtl/sslparser.y"
{
                if (Dict.RegMap.find(yyvsp[-9].str) != Dict.RegMap.end())
                    yyerror("Name reglist declared twice\n");
                Dict.RegMap[yyvsp[-9].str] = yyvsp[-4].num;
                // Now for detailed Reg information
                if (Dict.DetRegMap.find(yyvsp[-4].num) != Dict.DetRegMap.end())
                    yyerror("Index used for more than one register\n");
                Dict.DetRegMap[yyvsp[-4].num].s_name(yyvsp[-9].str);
                Dict.DetRegMap[yyvsp[-4].num].s_size(yyvsp[-7].num);
                Dict.DetRegMap[yyvsp[-4].num].s_address(NULL);
                // check range is legitimate for size. 8,10
                if ((Dict.RegMap.find(yyvsp[-2].str) == Dict.RegMap.end())
                    || (Dict.RegMap.find(yyvsp[0].str) == Dict.RegMap.end()))
                   yyerror("Undefined range\n");
                else {
                    int bitsize = Dict.DetRegMap[Dict.RegMap[yyvsp[0].str]].g_size();
                    for (int i = Dict.RegMap[yyvsp[-2].str]; i != Dict.RegMap[yyvsp[0].str]; i++) {
                        if (Dict.DetRegMap.find(i) == Dict.DetRegMap.end()) {
                            yyerror("Not all regesters in range defined\n");
                            break;
                        }
                        bitsize += Dict.DetRegMap[i].g_size();
                        if (bitsize > yyvsp[-7].num) {
                            yyerror("Range exceeds size of register\n");
                            break;
                        }
                    }
                if (bitsize < yyvsp[-7].num) 
                    yyerror("Register size is exceeds regesters in range\n");
                    // copy information
                }
                Dict.DetRegMap[yyvsp[-4].num].s_mappedIndex(Dict.RegMap[yyvsp[-2].str]);
                Dict.DetRegMap[yyvsp[-4].num].s_mappedOffset(0);
                Dict.DetRegMap[yyvsp[-4].num].s_float(bFloat);
            ;
    break;}
case 33:
#line 421 "rtl/sslparser.y"
{
                if (Dict.RegMap.find(yyvsp[-13].str) != Dict.RegMap.end())
                    yyerror("Name reglist declared twice\n");
                Dict.RegMap[yyvsp[-13].str] = yyvsp[-8].num;
                // Now for detailed Reg information
                if (Dict.DetRegMap.find(yyvsp[-8].num) != Dict.DetRegMap.end())
                    yyerror("Index used for more than one register\n");
                Dict.DetRegMap[yyvsp[-8].num].s_name(yyvsp[-13].str);
                Dict.DetRegMap[yyvsp[-8].num].s_size(yyvsp[-11].num);
                Dict.DetRegMap[yyvsp[-8].num].s_address(NULL);
                // Do checks
                if (yyvsp[-11].num != (yyvsp[-1].num - yyvsp[-3].num) + 1) 
                    error("Size does not equal range\n");
                    if (Dict.RegMap.find(yyvsp[-6].str) != Dict.RegMap.end()) {
                        if (yyvsp[-1].num >= Dict.DetRegMap[Dict.RegMap[yyvsp[-6].str]].g_size())
                            yyerror("Range extends over target register\n");
                    } else 
                        yyerror("Shared index not yet defined\n");
                Dict.DetRegMap[yyvsp[-8].num].s_mappedIndex(Dict.RegMap[yyvsp[-6].str]);
                Dict.DetRegMap[yyvsp[-8].num].s_mappedOffset(yyvsp[-3].num);
                Dict.DetRegMap[yyvsp[-8].num].s_float(bFloat);
            ;
    break;}
case 34:
#line 443 "rtl/sslparser.y"
{
                if ((int)yyvsp[-8].strlist->size() != (yyvsp[0].num - yyvsp[-2].num + 1)) {
                    cerr << "size of register array does not match mapping to r["
                         << yyvsp[-2].num << ".." << yyvsp[0].num << "]\n";
                    exit(1);
                } else {
                    list<string>::iterator loc = yyvsp[-8].strlist->begin();
                    for (int x = yyvsp[-2].num; x <= yyvsp[0].num; x++, loc++) {
                        if (Dict.RegMap.find(*loc) != Dict.RegMap.end())
                            yyerror("Name reglist declared twice\n");
                        Dict.addRegister( loc->c_str(), x, yyvsp[-5].num, bFloat);
                    }
                    delete yyvsp[-8].strlist;
                }
            ;
    break;}
case 35:
#line 458 "rtl/sslparser.y"
{
                list<string>::iterator loc = yyvsp[-6].strlist->begin();
                for (; loc != yyvsp[-6].strlist->end(); loc++) {
                    if (Dict.RegMap.find(*loc) != Dict.RegMap.end())
                        yyerror("Name reglist declared twice\n");
		    Dict.addRegister(loc->c_str(), yyvsp[0].num, yyvsp[-3].num, bFloat);
                }
                delete yyvsp[-6].strlist;
            ;
    break;}
case 36:
#line 470 "rtl/sslparser.y"
{
                yyvsp[-2].strlist->push_back(yyvsp[0].str);
            ;
    break;}
case 37:
#line 473 "rtl/sslparser.y"
{
                yyval.strlist = new list<string>;
                yyval.strlist->push_back(yyvsp[0].str);
            ;
    break;}
case 38:
#line 480 "rtl/sslparser.y"
{
                    // passing an empty variable map
                    map<string, InsNameElem*, StrCmp> m;
                    RTTFDefElem frt = RTTFDefElem(yyvsp[-5].str, *yyvsp[-4].parmlist, yyvsp[-1].rtlist);
                    Dict.FlagFuncs[yyvsp[-5].str] = (RTFlagDef*)(frt.eval(TableDict, m));
                    delete yyvsp[-4].parmlist;
                    // frt will delete $6
            ;
    break;}
case 39:
#line 491 "rtl/sslparser.y"
{
                if (ConstTable.find(yyvsp[-2].str) != ConstTable.end())
                    yyerror("Constant declared twice");
                ConstTable[string(yyvsp[-2].str)] = yyvsp[0].num;
            ;
    break;}
case 40:
#line 497 "rtl/sslparser.y"
{
                if (ConstTable.find(yyvsp[-4].str) != ConstTable.end())
                    yyerror("Constant declared twice");
                else if (yyvsp[-1].str == string("-"))
                    ConstTable[string(yyvsp[-4].str)] = yyvsp[-2].num - yyvsp[0].num;
                else if (yyvsp[-1].str == string("+"))
                    ConstTable[string(yyvsp[-4].str)] = yyvsp[-2].num + yyvsp[0].num;
                else
                    yyerror("Constant expression must be NUM + NUM or NUM - NUM");
            ;
    break;}
case 41:
#line 511 "rtl/sslparser.y"
{
            TableDict[yyvsp[-2].str] = yyvsp[0].tab;
        ;
    break;}
case 42:
#line 517 "rtl/sslparser.y"
{
            yyval.tab = new Table(*yyvsp[0].namelist);
            delete yyvsp[0].namelist;
        ;
    break;}
case 43:
#line 521 "rtl/sslparser.y"
{
            yyval.tab = new OpTable(*yyvsp[0].namelist);
            delete yyvsp[0].namelist;
        ;
    break;}
case 44:
#line 525 "rtl/sslparser.y"
{
            yyval.tab = new ExprTable(*yyvsp[0].exprlist);
            delete yyvsp[0].exprlist;
        ;
    break;}
case 45:
#line 532 "rtl/sslparser.y"
{
            // cross-product of two str_expr's
            deque<string>::iterator i, j;
            yyval.namelist = new deque<string>;
            for (i = yyvsp[-1].namelist->begin(); i != yyvsp[-1].namelist->end(); i++)
                for (j = yyvsp[0].namelist->begin(); j != yyvsp[0].namelist->end(); j++)
                    yyval.namelist->push_back((*i) + (*j));
            delete yyvsp[-1].namelist;
            delete yyvsp[0].namelist;
        ;
    break;}
case 46:
#line 542 "rtl/sslparser.y"
{
            yyval.namelist = yyvsp[0].namelist;
        ;
    break;}
case 47:
#line 548 "rtl/sslparser.y"
{
            // want to append $3 to $1
            // The following causes a massive warning message about mixing
            // signed and unsigned
            yyvsp[-2].namelist->insert(yyvsp[-2].namelist->end(), yyvsp[0].namelist->begin(), yyvsp[0].namelist->end());
            delete yyvsp[0].namelist;
            yyval.namelist = yyvsp[-2].namelist;
        ;
    break;}
case 48:
#line 556 "rtl/sslparser.y"
{
            yyvsp[-3].namelist->push_back("");
        ;
    break;}
case 49:
#line 559 "rtl/sslparser.y"
{
            yyval.namelist = yyvsp[0].namelist;
        ;
    break;}
case 50:
#line 565 "rtl/sslparser.y"
{
            yyval.namelist = yyvsp[-1].namelist;
        ;
    break;}
case 51:
#line 568 "rtl/sslparser.y"
{
            yyval.namelist = yyvsp[0].namelist;
        ;
    break;}
case 52:
#line 574 "rtl/sslparser.y"
{
            yyval.namelist = new deque<string>;
            yyval.namelist->push_back("");
            yyval.namelist->push_back(yyvsp[-1].str);
        ;
    break;}
case 53:
#line 579 "rtl/sslparser.y"
{
            yyval.namelist = new deque<string>(1, yyvsp[-1].str);
        ;
    break;}
case 54:
#line 582 "rtl/sslparser.y"
{
            ostrstream o;
            // expand $2 from table of names
            if (TableDict.find(yyvsp[0].str) != TableDict.end())
                if (TableDict[yyvsp[0].str]->getType() == NAMETABLE)
                    yyval.namelist = new deque<string>(TableDict[yyvsp[0].str]->records);
                else {
                    o << "name " << yyvsp[0].str << " is not a NAMETABLE.\n";
                    yyerror(str(o));
                }
            else {
                o << "could not dereference name " << yyvsp[0].str << "\n";
                yyerror(str(o));
            }
        ;
    break;}
case 55:
#line 597 "rtl/sslparser.y"
{
            // try and expand $2 from table of names
            // if fail, expand using '"' NAME '"' rule
            if (TableDict.find(yyvsp[0].str) != TableDict.end())
                if (TableDict[yyvsp[0].str]->getType() == NAMETABLE)
                    yyval.namelist = new deque<string>(TableDict[yyvsp[0].str]->records);
                else {
                    ostrstream o;
                    o << "name " << yyvsp[0].str << " is not a NAMETABLE.\n";
                    yyerror(str(o));
                }
            else {
                yyval.namelist = new deque<string>;
                yyval.namelist->push_back(yyvsp[0].str);
            }
        ;
    break;}
case 56:
#line 616 "rtl/sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 57:
#line 620 "rtl/sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 58:
#line 624 "rtl/sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 59:
#line 630 "rtl/sslparser.y"
{
            yyval.namelist = yyvsp[-1].namelist;
        ;
    break;}
case 60:
#line 636 "rtl/sslparser.y"
{
            yyval.namelist = yyvsp[-4].namelist;
            yyval.namelist->push_back(yyvsp[-1].str);
        ;
    break;}
case 61:
#line 640 "rtl/sslparser.y"
{
            yyval.namelist = new deque<string>;
            yyval.namelist->push_back(yyvsp[-1].str);
        ;
    break;}
case 62:
#line 647 "rtl/sslparser.y"
{
            yyval.exprlist = yyvsp[-1].exprlist;
        ;
    break;}
case 63:
#line 653 "rtl/sslparser.y"
{
            yyval.exprlist = yyvsp[-4].exprlist;
            yyval.exprlist->push_back(yyvsp[-1].ss);
        ;
    break;}
case 64:
#line 657 "rtl/sslparser.y"
{
            yyval.exprlist = new deque<SSElem*>;
            yyval.exprlist->push_back(yyvsp[-1].ss);
        ;
    break;}
case 65:
#line 664 "rtl/sslparser.y"
{
            yyvsp[0].insel->getrefmap(indexrefmap);
        ;
    break;}
case 66:
#line 666 "rtl/sslparser.y"
{
            int i, m;
            string n;
            ostrstream o;
            HRTL* r;
            m = yyvsp[-3].insel->ninstructions();
            for (i = 0, yyvsp[-3].insel->reset(); i < m; i++, yyvsp[-3].insel->increment()) {
                n = yyvsp[-3].insel->getinstruction();
                r = yyvsp[0].rtlist->eval(TableDict, indexrefmap);
                if (Dict.appendToDict(n, *yyvsp[-1].parmlist, *r) != 0) {
                    o << "Pattern " << yyvsp[-3].insel->getinspattern()
                        << " conflicts with an earlier declaration of " << n << ".\n";
                    yyerror(str(o));
                }
            }
            delete yyvsp[-3].insel;
            delete yyvsp[-1].parmlist;
            delete yyvsp[0].rtlist;
            indexrefmap.erase(indexrefmap.begin(), indexrefmap.end());
        ;
    break;}
case 67:
#line 689 "rtl/sslparser.y"
{
            yyval.insel = yyvsp[0].insel;
        ;
    break;}
case 68:
#line 692 "rtl/sslparser.y"
{
            unsigned i;
            InsNameElem *temp;
            string nm = yyvsp[0].str;
            
            if (nm[0] == '^')
	            nm.replace(0, 1, "");

            // remove all " and _, from the decoration
            while ((i = nm.find("\"")) != nm.npos)
                nm.replace(i,1,"");
            // replace all '.' with '_'s from the decoration
            while ((i = nm.find(".")) != nm.npos)
	            nm.replace(i,1,"_");
            while ((i = nm.find("_")) != nm.npos)
	            nm.replace(i,1,"");
 
            temp = new InsNameElem(nm);
            yyval.insel = yyvsp[-1].insel;
            yyval.insel->append(temp);
        ;
    break;}
case 69:
#line 716 "rtl/sslparser.y"
{
            yyval.insel = new InsNameElem(yyvsp[0].str);
        ;
    break;}
case 70:
#line 719 "rtl/sslparser.y"
{
            yyval.insel = yyvsp[0].insel;
        ;
    break;}
case 71:
#line 722 "rtl/sslparser.y"
{
            yyval.insel = yyvsp[-1].insel;
            yyval.insel->append(yyvsp[0].insel);
        ;
    break;}
case 72:
#line 729 "rtl/sslparser.y"
{
            yyval.insel = new InsOptionElem(yyvsp[-1].str);
        ;
    break;}
case 73:
#line 732 "rtl/sslparser.y"
{
            ostrstream o;
            if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
                o << "Table " << yyvsp[-2].str << " has not been declared.\n";
                yyerror(str(o));
            } else if ((yyvsp[-1].num < 0) || (yyvsp[-1].num >= (int)TableDict[yyvsp[-2].str]->records.size())) {
                o << "Can't get element " << yyvsp[-1].num << " of table " << yyvsp[-2].str << ".\n";
                yyerror(str(o));
            } else
                yyval.insel = new InsNameElem(TableDict[yyvsp[-2].str]->records[yyvsp[-1].num]);
        ;
    break;}
case 74:
#line 743 "rtl/sslparser.y"
{
            ostrstream o;
            if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
                o << "Table " << yyvsp[-2].str << " has not been declared.\n";
                yyerror(str(o));
            } else
                yyval.insel = new InsListElem(yyvsp[-2].str, TableDict[yyvsp[-2].str], yyvsp[-1].str);
        ;
    break;}
case 75:
#line 752 "rtl/sslparser.y"
{
            ostrstream o;
            if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
                o << "Table " << yyvsp[-2].str << " has not been declared.\n";
                yyerror(str(o));
            } else if ((yyvsp[-1].num < 0) || (yyvsp[-1].num >= (int)TableDict[yyvsp[-2].str]->records.size())) {
                o << "Can't get element " << yyvsp[-1].num << " of table " << yyvsp[-2].str << ".\n";
                yyerror(str(o));
            } else
                yyval.insel = new InsNameElem(TableDict[yyvsp[-2].str]->records[yyvsp[-1].num]);
        ;
    break;}
case 76:
#line 763 "rtl/sslparser.y"
{
            ostrstream o;
            if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
                o << "Table " << yyvsp[-2].str << " has not been declared.\n";
                yyerror(str(o));
            } else
                yyval.insel = new InsListElem(yyvsp[-2].str, TableDict[yyvsp[-2].str], yyvsp[-1].str);
        ;
    break;}
case 77:
#line 772 "rtl/sslparser.y"
{
            yyval.insel = new InsNameElem(yyvsp[-1].str);
        ;
    break;}
case 78:
#line 778 "rtl/sslparser.y"
{
            // append any automatically generated RT's and clear the
            // list they were stored in. Do nothing for a NOP (i.e. $2 = 0)
            if (yyvsp[0].regtransfer != NULL) {
                yyvsp[-1].rtlist->append(yyvsp[0].regtransfer);
            }
            yyval.rtlist = yyvsp[-1].rtlist;
        ;
    break;}
case 79:
#line 787 "rtl/sslparser.y"
{
            yyval.rtlist = new RTTListElem();
            if (yyvsp[0].regtransfer != NULL)
                yyval.rtlist->append(yyvsp[0].regtransfer);
        ;
    break;}
case 80:
#line 795 "rtl/sslparser.y"
{
            yyval.regtransfer = yyvsp[0].regtransfer;
        ;
    break;}
case 81:
#line 798 "rtl/sslparser.y"
{
            ostrstream o;
            if (Dict.FlagFuncs.find(yyvsp[-2].str) != Dict.FlagFuncs.end()) {
                RTTFCallElem* pfl;
                pfl = new RTTFCallElem(Dict.FlagFuncs[yyvsp[-2].str]->func_name, *yyvsp[-1].sslist);
                yyval.regtransfer = pfl;
            } else {
                o << yyvsp[-2].str << " is not a valid rtl.\n";
                yyerror(str(o));
            }
        ;
    break;}
case 82:
#line 809 "rtl/sslparser.y"
{
            yyval.regtransfer = 0;
        ;
    break;}
case 83:
#line 812 "rtl/sslparser.y"
{
            yyval.regtransfer = 0;
        ;
    break;}
case 84:
#line 815 "rtl/sslparser.y"
{
        yyval.regtransfer = NULL;
    ;
    break;}
case 85:
#line 821 "rtl/sslparser.y"
{
/*          SSListElem* pFlag = new SSListElem();
            pFlag->app(idRegOf);
            pFlag->app(Dict.RegMap[$3]);
            $1->push_back(pFlag);
            $$ = $1;
*/          yyval.spreglist = 0;
        ;
    break;}
case 86:
#line 829 "rtl/sslparser.y"
{
/*          list<SSElem*>* tmp = new list<SSElem*>;
            SSListElem* pFlag = new SSListElem();
            pFlag->app(idRegOf);
            pFlag->app(Dict.RegMap[$1]);
            tmp->push_back(pFlag);
            $$ = tmp;
*/          yyval.spreglist = 0;
        ;
    break;}
case 87:
#line 841 "rtl/sslparser.y"
{
            assert(yyvsp[0].num != 0);
            yyvsp[-2].parmlist->push_back(yyvsp[0].num);
            yyval.parmlist = yyvsp[-2].parmlist;
        ;
    break;}
case 88:
#line 847 "rtl/sslparser.y"
{
            yyval.parmlist = new list<int>;
            yyval.parmlist->push_back(yyvsp[0].num);
        ;
    break;}
case 89:
#line 851 "rtl/sslparser.y"
{
            yyval.parmlist = new list<int>;
        ;
    break;}
case 90:
#line 856 "rtl/sslparser.y"
{
            if (Dict.ParamMap.find(yyvsp[0].str) == Dict.ParamMap.end()) {
                Dict.ParamMap[yyvsp[0].str] = (yyval.num = theSemTable.addItem(yyvsp[0].str));
            } else {
                yyval.num = Dict.ParamMap[yyvsp[0].str];
            }
        ;
    break;}
case 91:
#line 865 "rtl/sslparser.y"
{
            yyval.sslist->push_back(yyvsp[0].ss);
        ;
    break;}
case 92:
#line 869 "rtl/sslparser.y"
{
            yyval.sslist = new list<SSElem*>;
            yyval.sslist->push_back(yyvsp[0].ss);
        ;
    break;}
case 93:
#line 874 "rtl/sslparser.y"
{
            yyval.sslist = new list<SSElem*>;
        ;
    break;}
case 94:
#line 880 "rtl/sslparser.y"
{
            yyval.regtransfer = new RTTAssignElem(yyvsp[-2].ss,yyvsp[0].ss,yyvsp[-5].str, yyvsp[-4].ss);
        ;
    break;}
case 95:
#line 883 "rtl/sslparser.y"
{
            // update the size of any generated RT's
            yyval.regtransfer = new RTTAssignElem(yyvsp[-2].ss, yyvsp[0].ss, yyvsp[-3].str);
        ;
    break;}
case 96:
#line 890 "rtl/sslparser.y"
{
            yyval.regtransfer = new RTTAssignElem(new SSListElem(), new SSElem(idFpush), 0);
        ;
    break;}
case 97:
#line 893 "rtl/sslparser.y"
{
            yyval.regtransfer = new RTTAssignElem(new SSListElem(), new SSElem(idFpop), 0);
        ;
    break;}
case 98:
#line 896 "rtl/sslparser.y"
{
            yyval.regtransfer = new RTTAssignElem(new SSListElem(), yyvsp[0].ss, yyvsp[-1].str);
        ;
    break;}
case 99:
#line 902 "rtl/sslparser.y"
{
            SSListElem* s = new SSListElem();
            s->app(idIntConst);
            s->app(yyvsp[0].num);
            yyval.ss = s;
        ;
    break;}
case 100:
#line 909 "rtl/sslparser.y"
{
            SSListElem* s = new SSListElem();
            s->app(idFltConst);
            // Note: code assumes the double takes the size of two integers!
            union {struct {int i1; int i2;} i; double d;} u;
            u.d = yyvsp[0].dbl;
            s->app(u.i.i1);
            s->app(u.i.i2);
            yyval.ss = s;
        ;
    break;}
case 101:
#line 920 "rtl/sslparser.y"
{
            SSListElem* s = new SSListElem();
            s->app(idTemp);
            if (TempMap.find(yyvsp[0].str) == TempMap.end())
                TempMap[yyvsp[0].str] = theSemTable.addItem(yyvsp[0].str);
            s->app(TempMap[yyvsp[0].str]);
            yyval.ss = s;
        ;
    break;}
case 102:
#line 929 "rtl/sslparser.y"
{
            yyval.ss = yyvsp[-1].ss;
        ;
    break;}
case 103:
#line 933 "rtl/sslparser.y"
{
            yyval.ss = yyvsp[0].ss;
        ;
    break;}
case 104:
#line 937 "rtl/sslparser.y"
{
            SSListElem* s = new SSListElem();
            s->app(idTern);
            s->app(yyvsp[-5].ss);
            s->app(yyvsp[-3].ss);
            s->app(yyvsp[-1].ss);
            yyval.ss = s;
        ;
    break;}
case 105:
#line 946 "rtl/sslparser.y"
{
            SSListElem* s = new SSListElem();
            if (yyvsp[0].num != STD_SIZE) {
                s->app(idSize);
                s->app(yyvsp[0].num);       // The size
            }
            s->app(idTern);
            s->app(yyvsp[-6].ss);
            s->app(yyvsp[-4].ss);
            s->app(yyvsp[-2].ss);
            yyval.ss = s;
        ;
    break;}
case 106:
#line 960 "rtl/sslparser.y"
{
            SSListElem* s = new SSListElem();
            s->app(idAddrOf);
            s->app(yyvsp[-1].ss);
            yyval.ss = s;
        ;
    break;}
case 107:
#line 969 "rtl/sslparser.y"
{
            ostrstream o;
            SSListElem* s = new SSListElem();
            s->app(yyvsp[-5].num);
            s->app(yyvsp[-3].num);
            s->app(yyvsp[-1].ss);
            if (yyvsp[-6].str == string("fsize"))       s->prep(idFsize);
            else if (yyvsp[-6].str == string("itof"))   s->prep(idItof);
            else if (yyvsp[-6].str == string("ftoi"))   s->prep(idFtoi);
            else if (yyvsp[-6].str == string("fround")) s->prep(idFround);
            else if (yyvsp[-6].str == string("truncu")) s->prep(idTruncu);
            else if (yyvsp[-6].str == string("truncs")) s->prep(idTruncs);
            else if (yyvsp[-6].str == string("zfill"))  s->prep(idZfill);
            else if (yyvsp[-6].str == string("sgnex"))  s->prep(idSgnEx);
            else {
                o << "Unknown CONV_FUNC " << yyvsp[-6].str;
                yyerror(str(o));
            }
            yyval.ss = s;
        ;
    break;}
case 108:
#line 991 "rtl/sslparser.y"
{
            yyval.ss = new SSElem(idFpush);
        ;
    break;}
case 109:
#line 994 "rtl/sslparser.y"
{
            yyval.ss = new SSElem(idFpop);
        ;
    break;}
case 110:
#line 999 "rtl/sslparser.y"
{
            ostrstream o;
            SSListElem* s = new SSListElem();
            s->app(yyvsp[-1].ss);
            if (yyvsp[-2].str == string("sin"))         s->prep(idSin);
            else if (yyvsp[-2].str == string("cos"))    s->prep(idCos);
            else if (yyvsp[-2].str == string("tan"))    s->prep(idTan);
            else if (yyvsp[-2].str == string("arctan")) s->prep(idArcTan);
            else if (yyvsp[-2].str == string("log2"))   s->prep(idLog2);
            else if (yyvsp[-2].str == string("log10"))  s->prep(idLog10);
            else if (yyvsp[-2].str == string("loge"))   s->prep(idLoge);
            else if (yyvsp[-2].str == string("sqrt"))   s->prep(idSqrt);
            else if (yyvsp[-2].str == string("execute"))s->prep(idExecute);
            else {
                o << "Unknown TRANSCEND " << yyvsp[-2].str;
                yyerror(str(o));
            }
            yyval.ss = s;
        ;
    break;}
case 111:
#line 1019 "rtl/sslparser.y"
{
            ostrstream o;
            if (indexrefmap.find(yyvsp[-1].str) == indexrefmap.end()) {
                o << "index " << yyvsp[-1].str << " not declared for use.\n";
                yyerror(str(o));
            } else if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
                o << "table " << yyvsp[-2].str << " not declared for use.\n";
                yyerror(str(o));
            } else if (TableDict[yyvsp[-2].str]->getType() != EXPRTABLE) {
                o << "table " << yyvsp[-2].str << " is not an expression table but "
                  "appears to be used as one.\n";
                yyerror(str(o));
            } else if ((int)((ExprTable*)TableDict[yyvsp[-2].str])->expressions.size() <
              indexrefmap[yyvsp[-1].str]->ntokens()) {
                o << "table " << yyvsp[-2].str << " (" <<
                  ((ExprTable*)TableDict[yyvsp[-2].str])->expressions.size() <<
                  ") is too small to use " << yyvsp[-1].str << " (" <<
                  indexrefmap[yyvsp[-1].str]->ntokens() << ") as an index.\n";
                yyerror(str(o));
            }
            yyval.ss = new SSTableElem(yyvsp[-2].str, yyvsp[-1].str);
        ;
    break;}
case 112:
#line 1041 "rtl/sslparser.y"
{
        ostrstream o;
        if( Dict.ParamMap.find(yyvsp[-2].str) != Dict.ParamMap.end() ) {
            if (Dict.DetParamMap.find(Dict.ParamMap[yyvsp[-2].str]) != Dict.DetParamMap.end()) {
                ParamEntry &param = Dict.DetParamMap[Dict.ParamMap[yyvsp[-2].str]];
                if( yyvsp[-1].sslist->size() != param.funcParams.size() ) {
                    o << yyvsp[-2].str << " requires " << param.funcParams.size()
                      << " parameters, but received " << yyvsp[-1].sslist->size() << ".\n";
                    yyerror(str(o));
                } else {
                    /* Everything checks out. *phew* */
                    SSListElem *s = new SSListElem();
                    s->app(Dict.ParamMap[yyvsp[-2].str]);
                    for( list<SSElem *>::iterator it = yyvsp[-1].sslist->begin();
                         it != yyvsp[-1].sslist->end(); it++ ) {
                        s->app(*it);
                    }
                    delete yyvsp[-1].sslist;
                    yyval.ss = s;
                }
            } else {
                o << yyvsp[-2].str << " is not a function, even if you try to call it.\n";
                yyerror(str(o));
            }
        } else {
            o << yyvsp[-2].str << ": Unrecognized name in call.\n";
            yyerror(str(o));
        }
    ;
    break;}
case 113:
#line 1073 "rtl/sslparser.y"
{
            SSListElem* s = new SSListElem();
            s->app(idSignExt);
            s->app(yyvsp[-1].ss);
            yyval.ss = s;
        ;
    break;}
case 114:
#line 1080 "rtl/sslparser.y"
{
            SSListElem* s = new SSListElem();
            s->app(yyvsp[-1].ss);
            if (yyvsp[0].num != STD_SIZE) {
                s->prep(yyvsp[0].num);       // The size
                s->prep(idSize);
            }
            yyval.ss = s;
        ;
    break;}
case 115:
#line 1090 "rtl/sslparser.y"
{
            SSListElem* s = new SSListElem();
            s->app(idNot);
            s->app(yyvsp[0].ss);
            yyval.ss = s;
        ;
    break;}
case 116:
#line 1097 "rtl/sslparser.y"
{
            SSListElem* s = new SSListElem();
            s->app(theSemTable.findItem(yyvsp[-1].str));
            s->app(yyvsp[-2].ss);
            s->app(yyvsp[0].ss);
            yyval.ss = s;
        ;
    break;}
case 117:
#line 1105 "rtl/sslparser.y"
{
            SSListElem* s = new SSListElem();
            s->app(theSemTable.findItem(yyvsp[-1].str));
            s->app(yyvsp[-2].ss);
            s->app(yyvsp[0].ss);
            yyval.ss = s;
        ;
    break;}
case 118:
#line 1113 "rtl/sslparser.y"
{
            SSListElem* s = new SSListElem();
            s->app(theSemTable.findItem(yyvsp[-1].str));
            s->app(yyvsp[-2].ss);
            s->app(yyvsp[0].ss);
            yyval.ss = s;
        ;
    break;}
case 119:
#line 1121 "rtl/sslparser.y"
{
            SSListElem* s = new SSListElem();
            s->app(theSemTable.findItem(yyvsp[-1].str));
            s->app(yyvsp[-2].ss);
            s->app(yyvsp[0].ss);
            yyval.ss = s;
        ;
    break;}
case 120:
#line 1129 "rtl/sslparser.y"
{
            SSListElem* s = new SSListElem();
            s->app(theSemTable.findItem(yyvsp[-1].str));
            s->app(yyvsp[-2].ss);
            s->app(yyvsp[0].ss);
            yyval.ss = s;
        ;
    break;}
case 121:
#line 1137 "rtl/sslparser.y"
{
            ostrstream o;
            SSListElem* s = new SSListElem();
            if (indexrefmap.find(yyvsp[-2].str) == indexrefmap.end()) {
                o << "index " << yyvsp[-2].str << " not declared for use.\n";
                yyerror(str(o));
            } else if (TableDict.find(yyvsp[-3].str) == TableDict.end()) {
                o << "table " << yyvsp[-3].str << " not declared for use.\n";
                yyerror(str(o));
            } else if (TableDict[yyvsp[-3].str]->getType() != OPTABLE) {
                o << "table " << yyvsp[-3].str << " is not an operator table but appears to be used as one.\n";
                yyerror(str(o));
            } else if ((int)TableDict[yyvsp[-3].str]->records.size() <
              indexrefmap[yyvsp[-2].str]->ntokens()) {
                o << "table " << yyvsp[-3].str << " is too small to use " << yyvsp[-2].str
                  << " as an index.\n";
                yyerror(str(o));
            }
            s->app(new SSOpTableElem(yyvsp[-3].str, yyvsp[-2].str));
            s->app(yyvsp[-4].ss);
            s->app(yyvsp[0].ss);
            yyval.ss = s;
        ;
    break;}
case 122:
#line 1161 "rtl/sslparser.y"
{
            yyval.ss = yyvsp[0].ss;
        ;
    break;}
case 123:
#line 1171 "rtl/sslparser.y"
{
            map<string, int, StrCmp>::const_iterator it = Dict.RegMap.find(yyvsp[0].str);
            if (it == Dict.RegMap.end()) {
                ostrstream ost;
                ost << "register `" << yyvsp[0].str << "' is undefined";
                yyerror(str(ost));
            } else if ((*it).second == -1)
                    // A special register, e.g. %npc or %CF
                    // If it already exists, the existing index is returned
                    yyval.ss = new SSElem(theSemTable.addItem(yyvsp[0].str));
            else {
                // An immediate register, e.g. %g2. In this case, we
                // want to return r[int 2]
                SSListElem* s = new SSListElem();
                s->app(idRegOf);
                s->app(idIntConst);
                s->app((*it).second);
                yyval.ss = s;
            }
        ;
    break;}
case 124:
#line 1192 "rtl/sslparser.y"
{
            SSListElem* s = new SSListElem();
            s->prep(yyvsp[-1].ss);
            s->prep(idRegOf);
            yyval.ss = s;
        ;
    break;}
case 125:
#line 1199 "rtl/sslparser.y"
{
            SSListElem* s = new SSListElem();
            s->app(yyvsp[-1].ss);
            s->prep(idMemOf);
            yyval.ss = s;
        ;
    break;}
case 126:
#line 1206 "rtl/sslparser.y"
{
        // this is a mixture of the param: PARM {} match
        // and the value_op: NAME {} match
            SSListElem* s = new SSListElem();
            map<string, int, StrCmp>::const_iterator it = Dict.ParamMap.find(yyvsp[0].str);
            if (it != Dict.ParamMap.end()) {
                s->app(idParam);
                s->app((*it).second);
            } else if (ConstTable.find(yyvsp[0].str) != ConstTable.end()) {
                s->app(idIntConst);
                s->app(ConstTable[yyvsp[0].str]);
            } else {
                ostrstream ost;
                ost << "`" << yyvsp[0].str << "' is not a constant, definition or a";
                ost << " parameter of this instruction\n";
                yyerror(str(ost));
            }
            yyval.ss = s;
        ;
    break;}
case 127:
#line 1226 "rtl/sslparser.y"
{
            SSListElem* s = new SSListElem();
            s->app(yyvsp[-6].ss);
            s->prep(idAt);
            s->app(yyvsp[-3].ss);
            s->app(yyvsp[-1].ss);
            yyval.ss = s;
        ;
    break;}
case 128:
#line 1234 "rtl/sslparser.y"
{
            SSListElem *s = new SSListElem();
            s->app(idPostVar);
            s->app(yyvsp[-1].ss);
            yyval.ss = s;
        ;
    break;}
case 129:
#line 1243 "rtl/sslparser.y"
{
            yyval.num = yyvsp[-1].num;
        ;
    break;}
case 130:
#line 1249 "rtl/sslparser.y"
{
            Dict.bigEndian = (strcmp(yyvsp[0].str, "BIG") == 0);
        ;
    break;}
case 131:
#line 1254 "rtl/sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 132:
#line 1257 "rtl/sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 136:
#line 1276 "rtl/sslparser.y"
{
            Dict.fastMap[string(yyvsp[-2].str)] = string(yyvsp[0].str);
        ;
    break;}
}

#line 811 "/usr/local/lib/bison.cc"
   /* the action file gets copied in in place of this dollarsign  */
  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YY_SSLParser_LSP_NEEDED
  yylsp -= yylen;
#endif

#if YY_SSLParser_DEBUG != 0
  if (YY_SSLParser_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YY_SSLParser_LSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = YY_SSLParser_LLOC.first_line;
      yylsp->first_column = YY_SSLParser_LLOC.first_column;
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
      ++YY_SSLParser_NERRS;

#ifdef YY_SSLParser_ERROR_VERBOSE
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
	      YY_SSLParser_ERROR(msg);
	      free(msg);
	    }
	  else
	    YY_SSLParser_ERROR ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YY_SSLParser_ERROR_VERBOSE */
	YY_SSLParser_ERROR("parse error");
    }

  YYGOTO(yyerrlab1);
YYLABEL(yyerrlab1)   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (YY_SSLParser_CHAR == YYEOF)
	YYABORT;

#if YY_SSLParser_DEBUG != 0
      if (YY_SSLParser_DEBUG_FLAG)
	fprintf(stderr, "Discarding token %d (%s).\n", YY_SSLParser_CHAR, yytname[yychar1]);
#endif

      YY_SSLParser_CHAR = YYEMPTY;
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
#ifdef YY_SSLParser_LSP_NEEDED
  yylsp--;
#endif

#if YY_SSLParser_DEBUG != 0
  if (YY_SSLParser_DEBUG_FLAG)
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

#if YY_SSLParser_DEBUG != 0
  if (YY_SSLParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = YY_SSLParser_LVAL;
#ifdef YY_SSLParser_LSP_NEEDED
  *++yylsp = YY_SSLParser_LLOC;
#endif

  yystate = yyn;
  YYGOTO(yynewstate);
/* end loop, in which YYGOTO may be used. */
  YYENDGOTO
}

/* END */

/* #line 1010 "/usr/local/lib/bison.cc" */
#line 2931 "rtl/sslparser.cc"
#line 1279 "rtl/sslparser.y"


/*==============================================================================
 * FUNCTION:        SSLParser::~SSLParser
 * OVERVIEW:        Destructor.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
SSLParser::~SSLParser()
{
    map<string, Table*, StrCmp>::iterator loc;
    if (theScanner != NULL)
        delete theScanner;
    for(loc = TableDict.begin(); loc != TableDict.end(); loc++)
        delete loc->second;
}

/*==============================================================================
 * FUNCTION:        SSLParser::yyerror
 * OVERVIEW:        Display an error message and exit.
 * PARAMETERS:      msg - an error message
 * RETURNS:         <nothing>
 *============================================================================*/
void SSLParser::yyerror(char* msg)
{
    cerr << sslFile << ":" << theScanner->theLine << ": " << msg << endl;
    exit(1);
}

/*==============================================================================
 * FUNCTION:        SSLParser::yylex
 * OVERVIEW:        The scanner driver than returns the next token.
 * PARAMETERS:      <none>
 * RETURNS:         the next token
 *============================================================================*/
int SSLParser::yylex()
{
    int token = theScanner->yylex(yylval);
    return token;
}
