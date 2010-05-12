#define YY_CSRParser_h_included

/*  A Bison++ parser, made from pal/csrparser.y  */

 /* with Bison++ version bison++ Version 1.21-7, adapted from GNU bison by coetmeur@icdc.fr
  */


#line 1 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
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
#ifndef _MSDOS
#ifdef MSDOS
#define _MSDOS
#endif
#endif
/* turboc */
#ifdef __MSDOS__
#ifndef _MSDOS
#define _MSDOS
#endif
#endif

#ifndef alloca
#if defined( __GNUC__)
#define alloca __builtin_alloca

#elif (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc)  || defined (__sgi)
#include <alloca.h>

#elif defined (_MSDOS)
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

/* #line 77 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 89 "pal/csrparser.cc"
#define YY_CSRParser_DEBUG  1
#define YY_CSRParser_DEBUG_INIT  1
#define YY_CSRParser_ERROR_VERBOSE 
#line 33 "pal/csrparser.y"

#include "global.h"
#include "ss.h"
#include "csr.h"
#include "rtl.h"
#include "prog.h"           // For global Prog object
#include "BinaryFile.h"     // For GetGlobalPointerInfo()

#ifdef WIN32
#include <malloc.h>
#endif

class CSRScanner;

#line 51 "pal/csrparser.y"
typedef union {
    char*                   str;
    int                     num;
    SemStr*                 ss;
    Parameters*             param;
    ParentStack*            parentStack;
    map<SemStr,SemStr>*     Map_ss_ss;
    typeToSemStrMap*        Map_Type_ss;
    list<string>*           List_string;
    list<SemStr>*           List_ss;
    pair<SemStr,int>*       Pair_ss_int;
    pair<Type,SemStr>*      Pair_Type_ss;
    pair<Type, int>*        Pair_Type_int;
    map<Type, int>*         Map_Type_int;
    Type*                   PType;
} yy_CSRParser_stype;
#define YY_CSRParser_STYPE yy_CSRParser_stype
#line 68 "pal/csrparser.y"

#include "csrscanner.h"
#define YY_CSRParser_CONSTRUCTOR_PARAM  \
    const string& csrFile, \
    CSR& csr, \
    RTLInstDict& rtlDict, \
    bool trace /*= false*/
#define YY_CSRParser_CONSTRUCTOR_INIT  : \
    csrFile(csrFile), \
    logueDict(csr.logueDict), \
    outgoingParams(csr.outgoingParams), \
    retLocations(csr.retLocations), \
    afpInit(csr.afpInit), \
    agpMap(csr.agpMap), \
    rtlDict(rtlDict), \
    callerSpecified(false), \
    receiverSpecified(false), \
    stackInverted(csr.stackInverted)
#define YY_CSRParser_CONSTRUCTOR_CODE  \
    FILE* inFile = fopen(csrFile.c_str(),"r"); \
    if (inFile == NULL) { \
        ostrstream ost; \
        ost << "can't open `" << csrFile << "' for reading"; \
        error(str(ost)); \
        exit(1); \
    } \
    theScanner = new CSRScanner(inFile, trace); \
    if (trace) yydebug = 1;
#define YY_CSRParser_MEMBERS  \
public: \
    virtual ~CSRParser(); \
private: \
\
    /** \
     * The name of the .csr file to be read. \
     */ \
    string csrFile; \
\
    /** \
     * These are aliases to members of the CSR object that this parser is used \
     * to construct. \
     */ \
    LogueDict& logueDict; \
    Parameters& outgoingParams; \
    ReturnLocations& retLocations; \
    SemStr& afpInit; \
    map<SemStr, SemStr>& agpMap; \
\
    /** \
     * The RTL template dictionary which contains the register maps required by\
     * this parser. \
     */ \
    RTLInstDict& rtlDict; \
    \
    /** \
     * The scanner. \
     */ \
    CSRScanner* theScanner; \
\
    /** \
     * This is the set of parameter names that are common to all the \
     * logues currently in scope. Any identifier in an expression \
     * must be in this set. \
     */ \
    set<string> scope; \
\
    /** \
     * Set the above parameter name scope as well as checking that each \
     * logue from which these names are extracted is of the expected \
     * logue type. \
     */ \
    void setScope(list<string>& logues, Logue::TYPE type, \
        const char* exception = NULL); \
\
    /** \
     * Check whether or not the given name is in the current scope. \
     */ \
    void checkScope(const string& name); \
\
    /** \
     * Clear the current scope. \
     */ \
    void clearScope(); \
\
    /** \
     * A wrapper around direct indexing into logueDict to \
     * issue a warning and stop if the result is NULL. \
     */ \
    Logue* getLogue(const string& name); \
\
    /** \
     * Given a list of names and a spec of a given type, \
     * set the spec to the logues corresponding to \
     * the names. \
     */ \
    void setParameters(const list<string>& names, \
        const Parameters& spec); \
    void setReturnLocations(const list<string>& names, \
        const ReturnLocations& spec); \
    void setParentStackSpecs(const list<string>& names, \
        const ParentStack& spec); \
\
    /** \
     * Flags to ensure certain constraints have been met in the \
     * PARAMETERS, RETURNS and PARENT STACK sections. \
     */ \
    bool callerSpecified; \
    bool receiverSpecified; \
    /* \
     * True if the stack is inverted \
     */ \
    bool& stackInverted; \


#line 77 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
/* %{ and %header{ and %union, during decl */
#define YY_CSRParser_BISON 1
#ifndef YY_CSRParser_COMPATIBILITY
#ifndef YY_USE_CLASS
#define  YY_CSRParser_COMPATIBILITY 1
#else
#define  YY_CSRParser_COMPATIBILITY 0
#endif
#endif

#if YY_CSRParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YYLTYPE
#ifndef YY_CSRParser_LTYPE
#define YY_CSRParser_LTYPE YYLTYPE
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_CSRParser_STYPE 
#define YY_CSRParser_STYPE YYSTYPE
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_CSRParser_DEBUG
#define  YY_CSRParser_DEBUG YYDEBUG
#endif
#endif
#ifdef YY_CSRParser_STYPE
#ifndef yystype
#define yystype YY_CSRParser_STYPE
#endif
#endif
/* use goto to be compatible */
#ifndef YY_CSRParser_USE_GOTO
#define YY_CSRParser_USE_GOTO 1
#endif
#endif

/* use no goto to be clean in C++ */
#ifndef YY_CSRParser_USE_GOTO
#define YY_CSRParser_USE_GOTO 0
#endif

#ifndef YY_CSRParser_PURE

/* #line 121 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 288 "pal/csrparser.cc"

#line 121 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
/*  YY_CSRParser_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 125 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 297 "pal/csrparser.cc"

#line 125 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
/* prefix */
#ifndef YY_CSRParser_DEBUG

/* #line 127 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 304 "pal/csrparser.cc"

#line 127 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
/* YY_CSRParser_DEBUG */
#endif


#ifndef YY_CSRParser_LSP_NEEDED

/* #line 132 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 314 "pal/csrparser.cc"

#line 132 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
 /* YY_CSRParser_LSP_NEEDED*/
#endif



/* DEFAULT LTYPE*/
#ifdef YY_CSRParser_LSP_NEEDED
#ifndef YY_CSRParser_LTYPE
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

#define YY_CSRParser_LTYPE yyltype
#endif
#endif
/* DEFAULT STYPE*/
      /* We used to use `unsigned long' as YY_CSRParser_STYPE on MSDOS,
	 but it seems better to be consistent.
	 Most programs should declare their own type anyway.  */

#ifndef YY_CSRParser_STYPE
#define YY_CSRParser_STYPE int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_CSRParser_PARSE
#define YY_CSRParser_PARSE yyparse
#endif
#ifndef YY_CSRParser_LEX
#define YY_CSRParser_LEX yylex
#endif
#ifndef YY_CSRParser_LVAL
#define YY_CSRParser_LVAL yylval
#endif
#ifndef YY_CSRParser_LLOC
#define YY_CSRParser_LLOC yylloc
#endif
#ifndef YY_CSRParser_CHAR
#define YY_CSRParser_CHAR yychar
#endif
#ifndef YY_CSRParser_NERRS
#define YY_CSRParser_NERRS yynerrs
#endif
#ifndef YY_CSRParser_DEBUG_FLAG
#define YY_CSRParser_DEBUG_FLAG yydebug
#endif
#ifndef YY_CSRParser_ERROR
#define YY_CSRParser_ERROR yyerror
#endif
#ifndef YY_CSRParser_PARSE_PARAM
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
#define YY_CSRParser_PARSE_PARAM
#ifndef YY_CSRParser_PARSE_PARAM_DEF
#define YY_CSRParser_PARSE_PARAM_DEF
#endif
#endif
#endif
#endif
#ifndef YY_CSRParser_PARSE_PARAM
#define YY_CSRParser_PARSE_PARAM void
#endif
#endif
#if YY_CSRParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YY_CSRParser_LTYPE
#ifndef YYLTYPE
#define YYLTYPE YY_CSRParser_LTYPE
#else
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
#endif
#endif
#ifndef YYSTYPE
#define YYSTYPE YY_CSRParser_STYPE
#else
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
#endif
#ifdef YY_CSRParser_PURE
#ifndef YYPURE
#define YYPURE YY_CSRParser_PURE
#endif
#endif
#ifdef YY_CSRParser_DEBUG
#ifndef YYDEBUG
#define YYDEBUG YY_CSRParser_DEBUG 
#endif
#endif
#ifndef YY_CSRParser_ERROR_VERBOSE
#ifdef YYERROR_VERBOSE
#define YY_CSRParser_ERROR_VERBOSE YYERROR_VERBOSE
#endif
#endif
#ifndef YY_CSRParser_LSP_NEEDED
#ifdef YYLSP_NEEDED
#define YY_CSRParser_LSP_NEEDED YYLSP_NEEDED
#endif
#endif
#endif
#ifndef YY_USE_CLASS
/* TOKEN C */

/* #line 240 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 427 "pal/csrparser.cc"
#define	MEM_IDX	258
#define	REG_ID	259
#define	IDENT	260
#define	ARITH_OP	261
#define	NUM	262
#define	GLOBALOFFSET	263
#define	INTEGER	264
#define	ADDRESS	265
#define	FLOAT	266
#define	AGGREGATE	267
#define	PARAMETERS	268
#define	RETURNS	269
#define	IN	270
#define	BASE	271
#define	OFFSET	272
#define	DEFINED_BY	273
#define	REGISTERS	274
#define	STACK	275
#define	FRAME	276
#define	GLOBAL	277
#define	ABSTRACTION	278
#define	INIT	279
#define	PARENT	280
#define	TO	281
#define	STEP	282
#define	LOCALS	283
#define	IS	284
#define	INVERTED	285
#define	NORMAL	286
#define	DOUBLE	287
#define	ALIGNMENT	288
#define	BYTES	289


#line 240 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
 /* #defines tokens */
#else
/* CLASS */
#ifndef YY_CSRParser_CLASS
#define YY_CSRParser_CLASS CSRParser
#endif
#ifndef YY_CSRParser_INHERIT
#define YY_CSRParser_INHERIT
#endif
#ifndef YY_CSRParser_MEMBERS
#define YY_CSRParser_MEMBERS 
#endif
#ifndef YY_CSRParser_LEX_BODY
#define YY_CSRParser_LEX_BODY  
#endif
#ifndef YY_CSRParser_ERROR_BODY
#define YY_CSRParser_ERROR_BODY  
#endif
#ifndef YY_CSRParser_CONSTRUCTOR_PARAM
#define YY_CSRParser_CONSTRUCTOR_PARAM
#endif
#ifndef YY_CSRParser_CONSTRUCTOR_CODE
#define YY_CSRParser_CONSTRUCTOR_CODE
#endif
#ifndef YY_CSRParser_CONSTRUCTOR_INIT
#define YY_CSRParser_CONSTRUCTOR_INIT
#endif
/* choose between enum and const */
#ifndef YY_CSRParser_USE_CONST_TOKEN
#define YY_CSRParser_USE_CONST_TOKEN 0
/* yes enum is more compatible with flex,  */
/* so by default we use it */
#endif
#if YY_CSRParser_USE_CONST_TOKEN != 0
#ifndef YY_CSRParser_ENUM_TOKEN
#define YY_CSRParser_ENUM_TOKEN yy_CSRParser_enum_token
#endif
#endif

class YY_CSRParser_CLASS YY_CSRParser_INHERIT
{
public:
#if YY_CSRParser_USE_CONST_TOKEN != 0
/* static const int token ... */

/* #line 284 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 509 "pal/csrparser.cc"
static const int MEM_IDX;
static const int REG_ID;
static const int IDENT;
static const int ARITH_OP;
static const int NUM;
static const int GLOBALOFFSET;
static const int INTEGER;
static const int ADDRESS;
static const int FLOAT;
static const int AGGREGATE;
static const int PARAMETERS;
static const int RETURNS;
static const int IN;
static const int BASE;
static const int OFFSET;
static const int DEFINED_BY;
static const int REGISTERS;
static const int STACK;
static const int FRAME;
static const int GLOBAL;
static const int ABSTRACTION;
static const int INIT;
static const int PARENT;
static const int TO;
static const int STEP;
static const int LOCALS;
static const int IS;
static const int INVERTED;
static const int NORMAL;
static const int DOUBLE;
static const int ALIGNMENT;
static const int BYTES;


#line 284 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
 /* decl const */
#else
enum YY_CSRParser_ENUM_TOKEN { YY_CSRParser_NULL_TOKEN=0

/* #line 287 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 550 "pal/csrparser.cc"
	,MEM_IDX=258
	,REG_ID=259
	,IDENT=260
	,ARITH_OP=261
	,NUM=262
	,GLOBALOFFSET=263
	,INTEGER=264
	,ADDRESS=265
	,FLOAT=266
	,AGGREGATE=267
	,PARAMETERS=268
	,RETURNS=269
	,IN=270
	,BASE=271
	,OFFSET=272
	,DEFINED_BY=273
	,REGISTERS=274
	,STACK=275
	,FRAME=276
	,GLOBAL=277
	,ABSTRACTION=278
	,INIT=279
	,PARENT=280
	,TO=281
	,STEP=282
	,LOCALS=283
	,IS=284
	,INVERTED=285
	,NORMAL=286
	,DOUBLE=287
	,ALIGNMENT=288
	,BYTES=289


#line 287 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_CSRParser_PARSE (YY_CSRParser_PARSE_PARAM);
 virtual void YY_CSRParser_ERROR(char *msg) YY_CSRParser_ERROR_BODY;
#ifdef YY_CSRParser_PURE
#ifdef YY_CSRParser_LSP_NEEDED
 virtual int  YY_CSRParser_LEX (YY_CSRParser_STYPE *YY_CSRParser_LVAL,YY_CSRParser_LTYPE *YY_CSRParser_LLOC) YY_CSRParser_LEX_BODY;
#else
 virtual int  YY_CSRParser_LEX (YY_CSRParser_STYPE *YY_CSRParser_LVAL) YY_CSRParser_LEX_BODY;
#endif
#else
 virtual int YY_CSRParser_LEX() YY_CSRParser_LEX_BODY;
 YY_CSRParser_STYPE YY_CSRParser_LVAL;
#ifdef YY_CSRParser_LSP_NEEDED
 YY_CSRParser_LTYPE YY_CSRParser_LLOC;
#endif
 int   YY_CSRParser_NERRS;
 int    YY_CSRParser_CHAR;
#endif
#if YY_CSRParser_DEBUG != 0
 int YY_CSRParser_DEBUG_FLAG;   /*  nonzero means print parse trace     */
#endif
public:
 YY_CSRParser_CLASS(YY_CSRParser_CONSTRUCTOR_PARAM);
public:
 YY_CSRParser_MEMBERS 
};
/* other declare folow */
#if YY_CSRParser_USE_CONST_TOKEN != 0

/* #line 318 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 619 "pal/csrparser.cc"
const int YY_CSRParser_CLASS::MEM_IDX=258;
const int YY_CSRParser_CLASS::REG_ID=259;
const int YY_CSRParser_CLASS::IDENT=260;
const int YY_CSRParser_CLASS::ARITH_OP=261;
const int YY_CSRParser_CLASS::NUM=262;
const int YY_CSRParser_CLASS::GLOBALOFFSET=263;
const int YY_CSRParser_CLASS::INTEGER=264;
const int YY_CSRParser_CLASS::ADDRESS=265;
const int YY_CSRParser_CLASS::FLOAT=266;
const int YY_CSRParser_CLASS::AGGREGATE=267;
const int YY_CSRParser_CLASS::PARAMETERS=268;
const int YY_CSRParser_CLASS::RETURNS=269;
const int YY_CSRParser_CLASS::IN=270;
const int YY_CSRParser_CLASS::BASE=271;
const int YY_CSRParser_CLASS::OFFSET=272;
const int YY_CSRParser_CLASS::DEFINED_BY=273;
const int YY_CSRParser_CLASS::REGISTERS=274;
const int YY_CSRParser_CLASS::STACK=275;
const int YY_CSRParser_CLASS::FRAME=276;
const int YY_CSRParser_CLASS::GLOBAL=277;
const int YY_CSRParser_CLASS::ABSTRACTION=278;
const int YY_CSRParser_CLASS::INIT=279;
const int YY_CSRParser_CLASS::PARENT=280;
const int YY_CSRParser_CLASS::TO=281;
const int YY_CSRParser_CLASS::STEP=282;
const int YY_CSRParser_CLASS::LOCALS=283;
const int YY_CSRParser_CLASS::IS=284;
const int YY_CSRParser_CLASS::INVERTED=285;
const int YY_CSRParser_CLASS::NORMAL=286;
const int YY_CSRParser_CLASS::DOUBLE=287;
const int YY_CSRParser_CLASS::ALIGNMENT=288;
const int YY_CSRParser_CLASS::BYTES=289;


#line 318 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
 /* const YY_CSRParser_CLASS::token */
#endif
/*apres const  */
YY_CSRParser_CLASS::YY_CSRParser_CLASS(YY_CSRParser_CONSTRUCTOR_PARAM) YY_CSRParser_CONSTRUCTOR_INIT
{
#if YY_CSRParser_DEBUG != 0
YY_CSRParser_DEBUG_FLAG=0;
#endif
YY_CSRParser_CONSTRUCTOR_CODE;
};
#endif

/* #line 329 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 668 "pal/csrparser.cc"


#define	YYFINAL		159
#define	YYFLAG		-32768
#define	YYNTBASE	43

#define YYTRANSLATE(x) ((unsigned)(x) <= 289 ? yytranslate[x] : 82)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    36,
    37,     2,     2,     2,     2,    42,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    40,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    41,     2,    35,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    38,     2,    39,     2,     2,     2,     2,     2,
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
    26,    27,    28,    29,    30,    31,    32,    33,    34
};

#if YY_CSRParser_DEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     2,    12,    14,    17,    19,    23,    25,    29,
    31,    33,    35,    39,    43,    48,    49,    50,    56,    58,
    61,    62,    66,    70,    71,    72,    80,    84,    89,    93,
    94,    95,   101,   104,   107,   111,   112,   116,   121,   124,
   125,   130,   133,   134,   139,   142,   143,   154,   155,   157,
   159,   161,   166,   168,   171,   172,   179,   180,   186,   189,
   192,   198,   204,   208,   210,   213,   214,   220,   223,   226,
   232,   233,   239,   243,   246
};

static const short yyrhs[] = {    -1,
     0,    53,    54,    57,    60,    72,    44,    77,    45,    81,
     0,     5,     0,    46,     5,     0,     4,     0,    48,     6,
    48,     0,    47,     0,     3,    48,    35,     0,     7,     0,
     8,     0,     5,     0,    36,    48,    37,     0,    47,    18,
    48,     0,    49,    47,    18,    48,     0,     0,     0,    46,
    38,    51,    49,    39,     0,    50,     0,    52,    50,     0,
     0,    20,    29,    31,     0,    20,    29,    30,     0,     0,
     0,    21,    23,    55,    24,    40,    47,    52,     0,    47,
    18,    48,     0,    56,    47,    18,    48,     0,    22,    23,
    56,     0,     0,     0,    46,    38,    59,    48,    39,     0,
    28,    58,     0,    60,    58,     0,    12,    18,    48,     0,
     0,    19,    18,    47,     0,     9,    19,    18,    47,     0,
    62,    47,     0,     0,    11,    19,    18,    47,     0,    63,
    47,     0,     0,    32,    19,    18,    47,     0,    64,    47,
     0,     0,    20,    18,    16,    40,    41,    48,    35,    17,
    40,     7,     0,     0,     9,     0,    11,     0,    32,     0,
    66,    33,     7,    34,     0,    67,     0,    68,    67,     0,
     0,    61,    62,    63,    64,    65,    68,     0,     0,    46,
    38,    71,    69,    39,     0,    13,    70,     0,    72,    70,
     0,     9,    42,     7,    15,    47,     0,    11,    42,     7,
    15,    47,     0,    10,    15,    47,     0,    73,     0,    74,
    73,     0,     0,    46,    38,    76,    74,    39,     0,    14,
    75,     0,    77,    75,     0,    48,    26,    48,    27,     7,
     0,     0,    46,    38,    80,    78,    39,     0,    25,    20,
    79,     0,    81,    79,     0,     0
};

#endif

#if YY_CSRParser_DEBUG != 0
static const short yyrline[] = { 0,
   227,   245,   257,   264,   271,   278,   321,   330,   335,   341,
   351,   361,   373,   383,   393,   400,   408,   414,   427,   429,
   430,   433,   438,   442,   448,   453,   470,   481,   490,   497,
   506,   512,   525,   527,   534,   539,   545,   552,   560,   566,
   572,   581,   587,   593,   602,   608,   614,   620,   626,   629,
   632,   637,   643,   650,   656,   659,   677,   683,   691,   693,
   701,   709,   714,   720,   727,   735,   741,   749,   751,   759,
   768,   773,   780,   782,   783
};

static const char * const yytname[] = {   "$","error","$illegal.","MEM_IDX",
"REG_ID","IDENT","ARITH_OP","NUM","GLOBALOFFSET","INTEGER","ADDRESS","FLOAT",
"AGGREGATE","PARAMETERS","RETURNS","IN","BASE","OFFSET","DEFINED_BY","REGISTERS",
"STACK","FRAME","GLOBAL","ABSTRACTION","INIT","PARENT","TO","STEP","LOCALS",
"IS","INVERTED","NORMAL","DOUBLE","ALIGNMENT","BYTES","']'","'('","')'","'{'",
"'}'","'='","'['","'.'","CSR_Spec","@1","@2","ident_list","register_id","exp",
"substitutions","afp_substitutions","@3","afps","stack_declaration","frame_abstraction",
"@4","agpSubstitutions","global_abstraction","locals_decl","@5","locals","aggregate_decl",
"int_registers","float_registers","double_registers","stack","align_type","align",
"alignment","parameters_clauses","parameters_decl","@6","parameters","return_decl",
"return_decls","returns_","@7","returns","range","parent_stack_decl","@8","parent_stack",
""
};
#endif

static const short yyr1[] = {     0,
    44,    45,    43,    46,    46,    47,    48,    48,    48,    48,
    48,    48,    48,    49,    49,    49,    51,    50,    52,    52,
    52,    53,    53,    53,    55,    54,    56,    56,    57,    57,
    59,    58,    60,    60,    61,    61,    62,    62,    62,    62,
    63,    63,    63,    64,    64,    64,    65,    65,    66,    66,
    66,    67,    68,    68,    68,    69,    71,    70,    72,    72,
    73,    73,    73,    74,    74,    76,    75,    77,    77,    78,
    80,    79,    81,    81,    81
};

static const short yyr2[] = {     0,
     0,     0,     9,     1,     2,     1,     3,     1,     3,     1,
     1,     1,     3,     3,     4,     0,     0,     5,     1,     2,
     0,     3,     3,     0,     0,     7,     3,     4,     3,     0,
     0,     5,     2,     2,     3,     0,     3,     4,     2,     0,
     4,     2,     0,     4,     2,     0,    10,     0,     1,     1,
     1,     4,     1,     2,     0,     6,     0,     5,     2,     2,
     5,     5,     3,     1,     2,     0,     5,     2,     2,     5,
     0,     5,     3,     2,     0
};

static const short yydefact[] = {    24,
     0,     0,     0,     0,    30,    23,    22,    25,     0,     0,
     0,     0,     0,     0,     0,     6,     0,    29,     4,     0,
    33,     0,    34,     1,     0,     0,     0,     5,    31,     0,
    59,     0,    60,    21,     0,    12,    10,    11,     0,     8,
    27,     0,     0,    57,     0,     2,     0,    19,    26,     0,
     0,     0,    28,     0,    36,     0,    68,    75,    69,    17,
    20,     9,    13,     7,    32,     0,    40,     0,    66,     0,
     3,    16,     0,     0,     0,    43,    58,     0,     0,     0,
    74,     0,     0,    35,     0,     0,     0,    39,    46,     0,
     0,     0,    64,     0,    73,    71,     0,    18,     0,     0,
    37,     0,     0,    42,    48,     0,     0,     0,    67,    65,
     0,    14,     0,    38,     0,     0,     0,    45,    55,     0,
    63,     0,     0,     0,    15,    41,     0,     0,    49,    50,
    51,     0,    53,    56,     0,     0,     0,    72,    44,     0,
     0,    54,    61,    62,     0,     0,     0,     0,     0,    52,
    70,     0,     0,     0,     0,    47,     0,     0,     0
};

static const short yydefgoto[] = {   157,
    32,    58,    20,    40,    41,    83,    48,    72,    49,     2,
     5,    11,    18,    10,    21,    43,    14,    67,    76,    89,
   105,   119,   132,   133,   134,    68,    31,    55,    24,    93,
    94,    57,    78,    46,   124,    81,   111,    71
};

static const short yypact[] = {     8,
    24,    51,    27,    47,    53,-32768,-32768,-32768,    50,    52,
    54,    81,    82,    49,    46,-32768,    71,    81,-32768,     0,
-32768,    82,-32768,    82,    81,    14,    72,-32768,-32768,     4,
-32768,    77,-32768,    82,    14,-32768,-32768,-32768,    14,-32768,
    86,    14,    14,-32768,    82,    82,     7,-32768,    82,    28,
     2,    14,    86,     9,    84,    11,-32768,    69,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,    79,    21,    59,-32768,    80,
    82,    81,    14,    83,    88,    65,-32768,    73,    82,    18,
-32768,    89,    -3,    86,    90,    81,    85,-32768,    20,    57,
    94,    68,-32768,    22,-32768,-32768,    14,-32768,    93,    81,
-32768,    95,    97,-32768,    23,   105,    81,   107,-32768,-32768,
    14,    86,    14,-32768,    81,   100,   101,-32768,    35,   106,
-32768,   110,    29,    62,    86,-32768,    81,   104,-32768,-32768,
-32768,    96,-32768,    35,    81,    81,    14,-32768,-32768,    87,
   115,-32768,-32768,-32768,    41,    91,    92,   121,    14,-32768,
-32768,    31,   113,    98,   124,-32768,   133,   134,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,   -20,   -12,   -32,-32768,    99,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   122,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,     1,-32768,-32768,   116,-32768,-32768,    43,
-32768,   103,-32768,-32768,-32768,    60,-32768,-32768
};


#define	YYLAST		149


static const short yytable[] = {    17,
    16,    30,    50,    30,    28,    27,    51,    52,    28,    53,
    54,    28,    34,    47,    52,    28,    35,    16,    36,    64,
    37,    38,    28,    16,    56,    56,    16,     1,    47,    74,
    90,    91,    92,    52,    52,    98,    52,    29,    63,    75,
    84,    44,   117,   129,    60,   130,    52,    65,    69,    39,
    80,   103,     3,    19,   137,    96,     6,     7,    80,    82,
   109,    22,    62,    88,   112,   153,   131,   148,    16,     8,
    99,     4,    12,   101,     9,    87,   104,    15,   123,    13,
   125,    90,    91,    92,    16,    25,    19,   114,    26,    42,
    45,    52,   118,    70,   121,    66,    73,    77,   106,    79,
   138,    85,   126,   102,   145,    86,    97,   100,   107,   108,
   113,   120,   115,   122,   139,   116,   152,   127,   128,   140,
   135,   147,   143,   144,   136,   150,   146,   151,   141,   154,
   156,   149,   158,   159,   142,    23,   110,   155,    95,    33,
     0,     0,     0,     0,     0,     0,     0,    61,    59
};

static const short yycheck[] = {    12,
     4,    22,    35,    24,     5,    18,    39,     6,     5,    42,
    43,     5,    25,    34,     6,     5,     3,     4,     5,    52,
     7,     8,     5,     4,    45,    46,     4,    20,    49,     9,
     9,    10,    11,     6,     6,    39,     6,    38,    37,    19,
    73,    38,    20,     9,    38,    11,     6,    39,    38,    36,
    71,    32,    29,     5,    26,    38,    30,    31,    79,    72,
    39,    13,    35,    76,    97,    35,    32,    27,     4,    23,
    83,    21,    23,    86,    22,    11,    89,    24,   111,    28,
   113,     9,    10,    11,     4,    40,     5,   100,    18,    18,
    14,     6,   105,    25,   107,    12,    18,    39,    42,    20,
    39,    19,   115,    19,   137,    18,    18,    18,    15,    42,
    18,     7,    18,     7,   127,    19,   149,    18,    18,    16,
    15,     7,   135,   136,    15,    34,    40,     7,    33,    17,
     7,    41,     0,     0,   134,    14,    94,    40,    79,    24,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    46
};

#line 329 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
 /* fattrs + tables */

/* parser code folow  */


/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: dollar marks section change
   the next  is replaced by the list of actions, each action
   as one case of the switch.  */ 

#if YY_CSRParser_USE_GOTO != 0
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

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (YY_CSRParser_CHAR = YYEMPTY)
#define YYEMPTY         -2
#define YYEOF           0
#define YYACCEPT        return(0)
#define YYABORT         return(1)
#define YYERROR         YYGOTO(yyerrlab1)
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL          YYGOTO(yyerrlab)
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do                                                              \
  if (YY_CSRParser_CHAR == YYEMPTY && yylen == 1)                               \
    { YY_CSRParser_CHAR = (token), YY_CSRParser_LVAL = (value);                 \
      yychar1 = YYTRANSLATE (YY_CSRParser_CHAR);                                \
      YYPOPSTACK;                                               \
      YYGOTO(yybackup);                                            \
    }                                                           \
  else                                                          \
    { YY_CSRParser_ERROR ("syntax error: cannot back up"); YYERROR; }   \
while (0)

#define YYTERROR        1
#define YYERRCODE       256

#ifndef YY_CSRParser_PURE
/* UNPURE */
#define YYLEX           YY_CSRParser_LEX()
#ifndef YY_USE_CLASS
/* If nonreentrant, and not class , generate the variables here */
int     YY_CSRParser_CHAR;                      /*  the lookahead symbol        */
YY_CSRParser_STYPE      YY_CSRParser_LVAL;              /*  the semantic value of the */
				/*  lookahead symbol    */
int YY_CSRParser_NERRS;                 /*  number of parse errors so far */
#ifdef YY_CSRParser_LSP_NEEDED
YY_CSRParser_LTYPE YY_CSRParser_LLOC;   /*  location data for the lookahead     */
			/*  symbol                              */
#endif
#endif


#else
/* PURE */
#ifdef YY_CSRParser_LSP_NEEDED
#define YYLEX           YY_CSRParser_LEX(&YY_CSRParser_LVAL, &YY_CSRParser_LLOC)
#else
#define YYLEX           YY_CSRParser_LEX(&YY_CSRParser_LVAL)
#endif
#endif
#ifndef YY_USE_CLASS
#if YY_CSRParser_DEBUG != 0
int YY_CSRParser_DEBUG_FLAG;                    /*  nonzero means print parse trace     */
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
 YY_CSRParser_CLASS::
#endif
     YY_CSRParser_PARSE(YY_CSRParser_PARSE_PARAM)
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
/* parameter definition without protypes */
YY_CSRParser_PARSE_PARAM_DEF
#endif
#endif
#endif
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YY_CSRParser_STYPE *yyvsp;
  int yyerrstatus;      /*  number of tokens to shift before error messages enabled */
  int yychar1=0;          /*  lookahead token as an internal (translated) token number */

  short yyssa[YYINITDEPTH];     /*  the state stack                     */
  YY_CSRParser_STYPE yyvsa[YYINITDEPTH];        /*  the semantic value stack            */

  short *yyss = yyssa;          /*  refer to the stacks thru separate pointers */
  YY_CSRParser_STYPE *yyvs = yyvsa;     /*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YY_CSRParser_LSP_NEEDED
  YY_CSRParser_LTYPE yylsa[YYINITDEPTH];        /*  the location stack                  */
  YY_CSRParser_LTYPE *yyls = yylsa;
  YY_CSRParser_LTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YY_CSRParser_PURE
  int YY_CSRParser_CHAR;
  YY_CSRParser_STYPE YY_CSRParser_LVAL;
  int YY_CSRParser_NERRS;
#ifdef YY_CSRParser_LSP_NEEDED
  YY_CSRParser_LTYPE YY_CSRParser_LLOC;
#endif
#endif

  YY_CSRParser_STYPE yyval;             /*  the variable used to return         */
				/*  semantic values from the action     */
				/*  routines                            */

  int yylen;
/* start loop, in which YYGOTO may be used. */
YYBEGINGOTO

#if YY_CSRParser_DEBUG != 0
  if (YY_CSRParser_DEBUG_FLAG)
    fprintf(stderr, "Starting parse\n");
#endif
  yystate = 0;
  yyerrstatus = 0;
  YY_CSRParser_NERRS = 0;
  YY_CSRParser_CHAR = YYEMPTY;          /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YY_CSRParser_LSP_NEEDED
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
      YY_CSRParser_STYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YY_CSRParser_LSP_NEEDED
      YY_CSRParser_LTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YY_CSRParser_LSP_NEEDED
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
#ifdef YY_CSRParser_LSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  YY_CSRParser_ERROR("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YY_CSRParser_STYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YY_CSRParser_LSP_NEEDED
      yyls = (YY_CSRParser_LTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YY_CSRParser_LSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YY_CSRParser_DEBUG != 0
      if (YY_CSRParser_DEBUG_FLAG)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YY_CSRParser_DEBUG != 0
  if (YY_CSRParser_DEBUG_FLAG)
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

  if (YY_CSRParser_CHAR == YYEMPTY)
    {
#if YY_CSRParser_DEBUG != 0
      if (YY_CSRParser_DEBUG_FLAG)
	fprintf(stderr, "Reading a token: ");
#endif
      YY_CSRParser_CHAR = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (YY_CSRParser_CHAR <= 0)           /* This means end of input. */
    {
      yychar1 = 0;
      YY_CSRParser_CHAR = YYEOF;                /* Don't call YYLEX any more */

#if YY_CSRParser_DEBUG != 0
      if (YY_CSRParser_DEBUG_FLAG)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(YY_CSRParser_CHAR);

#if YY_CSRParser_DEBUG != 0
      if (YY_CSRParser_DEBUG_FLAG)
	{
	  fprintf (stderr, "Next token is %d (%s", YY_CSRParser_CHAR, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, YY_CSRParser_CHAR, YY_CSRParser_LVAL);
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

#if YY_CSRParser_DEBUG != 0
  if (YY_CSRParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting token %d (%s), ", YY_CSRParser_CHAR, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (YY_CSRParser_CHAR != YYEOF)
    YY_CSRParser_CHAR = YYEMPTY;

  *++yyvsp = YY_CSRParser_LVAL;
#ifdef YY_CSRParser_LSP_NEEDED
  *++yylsp = YY_CSRParser_LLOC;
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

#if YY_CSRParser_DEBUG != 0
  if (YY_CSRParser_DEBUG_FLAG)
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


/* #line 783 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 1349 "pal/csrparser.cc"

  switch (yyn) {

case 1:
#line 233 "pal/csrparser.y"
{
            // The parameters spec for a caller must have been specified
            if (!callerSpecified) {
                ostrstream ost;
                ost << "the PARAMETERS section must include a specification ";
                ost << "for where callers will pass outgoing parameters";
                ost << "(i.e. need to use CALLER keyword for exactly ";
                ost << "one of specs in this section)";
                yyerror(str(ost));
            }
        ;
    break;}
case 2:
#line 245 "pal/csrparser.y"
{
            // The return spec for a caller must have been specified
            if (!receiverSpecified) {
                ostrstream ost;
                ost << "the RETURNS section must include a specification ";
                ost << "for where callers will receive a returned value ";
                ost << "(i.e. need to use CALLER keyword for exactly ";
                ost << "one of specs in this section)";
                yyerror(str(ost));
            }
        ;
    break;}
case 4:
#line 266 "pal/csrparser.y"
{
            yyval.List_string = new list<string>;
            yyval.List_string->push_back(yyvsp[0].str);
        ;
    break;}
case 5:
#line 272 "pal/csrparser.y"
{
            yyvsp[-1].List_string->push_back(yyvsp[0].str);
            yyval.List_string = yyvsp[-1].List_string;
        ;
    break;}
case 6:
#line 280 "pal/csrparser.y"
{
            map<string,int,less<string> >::const_iterator it = 
                rtlDict.RegMap.find(yyvsp[0].str);

            // Special case for the AFP
            if (string(yyvsp[0].str) == "%afp") {
                yyval.ss = new SemStr();
                yyval.ss->push(idAFP);
            }
            // Special case for the AGP register
            else if (string(yyvsp[0].str) == "%agp") {
                yyval.ss = new SemStr();
                yyval.ss->push(idAGP); 
            }
            else if (it == rtlDict.RegMap.end()) {
                ostrstream os;
                os << "`" << yyvsp[0].str << "' is not declared";
                yyerror(str(os));
            } else {
                if ((*it).second == -1) {

                    // A special register, e.g. %pc or %npc
                    ostrstream ost;
                    ost << "`" << yyvsp[0].str << "' is a special register ";
                    ost << "which can't be used in a CSR specification";
                    yyerror(str(ost));

                } else {

                    // An immediate register
                    // Create it with a type appropriate for the register
                    const Register& reg = rtlDict.DetRegMap[it->second];
                    yyval.ss = new SemStr(reg.g_type());
                    yyval.ss->push(idRegOf);
                    yyval.ss->push(idIntConst);
                    yyval.ss->push((*it).second);
                }
            }
        ;
    break;}
case 7:
#line 323 "pal/csrparser.y"
{
            yyval.ss = yyvsp[-2].ss;
            yyval.ss->prep(theSemTable.findItem(yyvsp[-1].str));
            yyval.ss->pushSS(yyvsp[0].ss);
            delete yyvsp[0].ss;
        ;
    break;}
case 8:
#line 331 "pal/csrparser.y"
{
            yyval.ss = yyvsp[0].ss;
        ;
    break;}
case 9:
#line 336 "pal/csrparser.y"
{
            yyval.ss = yyvsp[-1].ss;
            yyval.ss->prep(idMemOf);
        ;
    break;}
case 10:
#line 342 "pal/csrparser.y"
{
            yyval.ss = new SemStr();
            yyval.ss->push(idIntConst);
            yyval.ss->push(yyvsp[0].num);
        ;
    break;}
case 11:
#line 352 "pal/csrparser.y"
{
            // The loader knows about the address of %agp (first), and a
            // constant (second) that represents GLOBALOFFSET
            pair<unsigned, unsigned> pr = prog.pBF->GetGlobalPointerInfo();
            yyval.ss = new SemStr();
            yyval.ss->push(idIntConst);
            yyval.ss->push(pr.second);
        ;
    break;}
case 12:
#line 362 "pal/csrparser.y"
{

            // Any identifier used in an expression must be in the
            // current scope
            checkScope(yyvsp[0].str);

            int varIdx = theSemTable.findItem(yyvsp[0].str,false);
            yyval.ss = new SemStr();
            yyval.ss->push(idParam);
            yyval.ss->push(varIdx);
        ;
    break;}
case 13:
#line 374 "pal/csrparser.y"
{
            yyval.ss = yyvsp[-1].ss;
        ;
    break;}
case 14:
#line 385 "pal/csrparser.y"
{
            if (yyvsp[-2].ss->getFirstIdx() == idAFP)
                yyerror("can't use %afp on the left hand side of a substitution");
            yyval.Map_ss_ss = new map<SemStr,SemStr>;
            (*yyval.Map_ss_ss)[*yyvsp[-2].ss] = *yyvsp[0].ss;
            delete yyvsp[-2].ss;
            delete yyvsp[0].ss;
        ;
    break;}
case 15:
#line 394 "pal/csrparser.y"
{
            yyval.Map_ss_ss = yyvsp[-3].Map_ss_ss;
            (*yyval.Map_ss_ss)[*yyvsp[-2].ss] = *yyvsp[0].ss;
            delete yyvsp[-2].ss;
            delete yyvsp[0].ss;
        ;
    break;}
case 16:
#line 403 "pal/csrparser.y"
{
            yyval.Map_ss_ss = new map<SemStr, SemStr>;
        ;
    break;}
case 17:
#line 410 "pal/csrparser.y"
{
            setScope(*yyvsp[-1].List_string,Logue::CALLEE_PROLOGUE);
        ;
    break;}
case 18:
#line 415 "pal/csrparser.y"
{
            for (list<string>::iterator it = yyvsp[-4].List_string->begin();
                it != yyvsp[-4].List_string->end(); it++) {

                CalleePrologue* logue = (CalleePrologue*)getLogue(*it);
                logue->setSubstitutions(*yyvsp[-1].Map_ss_ss);
            }
            delete yyvsp[-1].Map_ss_ss;
            delete yyvsp[-4].List_string;
        ;
    break;}
case 22:
#line 435 "pal/csrparser.y"
{
            stackInverted = false;
        ;
    break;}
case 23:
#line 439 "pal/csrparser.y"
{
            stackInverted = true;
        ;
    break;}
case 24:
#line 443 "pal/csrparser.y"
{
            stackInverted = false;
        ;
    break;}
case 25:
#line 450 "pal/csrparser.y"
{
            clearScope();
        ;
    break;}
case 26:
#line 455 "pal/csrparser.y"
{
            // Save the AFP initial value
            afpInit = *yyvsp[-1].ss;
            delete yyvsp[-1].ss;
        ;
    break;}
case 27:
#line 472 "pal/csrparser.y"
{
            if (yyvsp[-2].ss->getFirstIdx() == idAGP)
                yyerror(
                  "can't use %agp on the left hand side of a substitution");
            yyval.Map_ss_ss = new map<SemStr,SemStr>;
            (*yyval.Map_ss_ss)[*yyvsp[-2].ss] = *yyvsp[0].ss;
            delete yyvsp[-2].ss;
            delete yyvsp[0].ss;
        ;
    break;}
case 28:
#line 482 "pal/csrparser.y"
{
            yyval.Map_ss_ss = yyvsp[-3].Map_ss_ss;
            (*yyval.Map_ss_ss)[*yyvsp[-2].ss] = *yyvsp[0].ss;
            delete yyvsp[-2].ss;
            delete yyvsp[0].ss;
        ;
    break;}
case 29:
#line 492 "pal/csrparser.y"
{
            // Save the AGP map
            agpMap = *yyvsp[0].Map_ss_ss;
            delete yyvsp[0].Map_ss_ss;
        ;
    break;}
case 31:
#line 508 "pal/csrparser.y"
{
            setScope(*yyvsp[-1].List_string,Logue::CALLEE_PROLOGUE);
        ;
    break;}
case 32:
#line 513 "pal/csrparser.y"
{
            for (list<string>::iterator it = yyvsp[-4].List_string->begin();
                it != yyvsp[-4].List_string->end(); it++) {

                CalleePrologue* logue = (CalleePrologue*)getLogue(*it);
                logue->setLocalsSize(*yyvsp[-1].ss);
            }
            delete yyvsp[-4].List_string;
            delete yyvsp[-1].ss;
        ;
    break;}
case 35:
#line 536 "pal/csrparser.y"
{
            yyval.ss = yyvsp[0].ss;
        ;
    break;}
case 36:
#line 540 "pal/csrparser.y"
{
            yyval.ss = NULL;
        ;
    break;}
case 37:
#line 547 "pal/csrparser.y"
{
            yyval.List_ss = new list<SemStr>;
            yyval.List_ss->push_back(*yyvsp[0].ss);
            delete yyvsp[0].ss;
        ;
    break;}
case 38:
#line 554 "pal/csrparser.y"
{
            yyval.List_ss = new list<SemStr>;
            yyval.List_ss->push_back(*yyvsp[0].ss);
            delete yyvsp[0].ss;
        ;
    break;}
case 39:
#line 561 "pal/csrparser.y"
{
            yyval.List_ss = yyvsp[-1].List_ss;
            yyval.List_ss->push_back(*yyvsp[0].ss);
            delete yyvsp[0].ss;
        ;
    break;}
case 40:
#line 567 "pal/csrparser.y"
{
            yyval.List_ss = NULL;
        ;
    break;}
case 41:
#line 575 "pal/csrparser.y"
{
            yyval.List_ss = new list<SemStr>;
            yyval.List_ss->push_back(*yyvsp[0].ss);
            delete yyvsp[0].ss;
        ;
    break;}
case 42:
#line 582 "pal/csrparser.y"
{
            yyval.List_ss = yyvsp[-1].List_ss;
            yyval.List_ss->push_back(*yyvsp[0].ss);
            delete yyvsp[0].ss;
        ;
    break;}
case 43:
#line 588 "pal/csrparser.y"
{
            yyval.List_ss = NULL;
        ;
    break;}
case 44:
#line 596 "pal/csrparser.y"
{
            yyval.List_ss = new list<SemStr>;
            yyval.List_ss->push_back(*yyvsp[0].ss);
            delete yyvsp[0].ss;
        ;
    break;}
case 45:
#line 603 "pal/csrparser.y"
{
            yyval.List_ss = yyvsp[-1].List_ss;
            yyval.List_ss->push_back(*yyvsp[0].ss);
            delete yyvsp[0].ss;
        ;
    break;}
case 46:
#line 609 "pal/csrparser.y"
{
            yyval.List_ss = NULL;
        ;
    break;}
case 47:
#line 617 "pal/csrparser.y"
{
            yyval.Pair_ss_int = new pair<SemStr,int>(*yyvsp[-4].ss,yyvsp[0].num);
        ;
    break;}
case 48:
#line 621 "pal/csrparser.y"
{
            yyval.Pair_ss_int = NULL;
        ;
    break;}
case 49:
#line 628 "pal/csrparser.y"
{ yyval.PType = new Type(::INTEGER); ;
    break;}
case 50:
#line 631 "pal/csrparser.y"
{ yyval.PType = new Type(FLOATP, 32); ;
    break;}
case 51:
#line 634 "pal/csrparser.y"
{ yyval.PType = new Type(FLOATP, 64); ;
    break;}
case 52:
#line 639 "pal/csrparser.y"
{
            yyval.Pair_Type_int = new pair<Type, int>(*yyvsp[-3].PType, yyvsp[-1].num);
        ;
    break;}
case 53:
#line 645 "pal/csrparser.y"
{
            yyval.Map_Type_int = new map<Type, int>;
            (*yyval.Map_Type_int)[yyvsp[0].Pair_Type_int->first] = yyvsp[0].Pair_Type_int->second;
            delete yyvsp[0].Pair_Type_int;
        ;
    break;}
case 54:
#line 652 "pal/csrparser.y"
{
            (*yyval.Map_Type_int)[yyvsp[0].Pair_Type_int->first] = yyvsp[0].Pair_Type_int->second;
            delete yyvsp[0].Pair_Type_int;
        ;
    break;}
case 55:
#line 657 "pal/csrparser.y"
{ yyval.Map_Type_int = NULL; ;
    break;}
case 56:
#line 666 "pal/csrparser.y"
{
            yyval.param = new Parameters(yyvsp[-5].ss,yyvsp[-4].List_ss,yyvsp[-3].List_ss,yyvsp[-2].List_ss,yyvsp[-1].Pair_ss_int,yyvsp[0].Map_Type_int);
            if (yyvsp[-5].ss != NULL) delete yyvsp[-5].ss;
            if (yyvsp[-4].List_ss != NULL) delete yyvsp[-4].List_ss;
            if (yyvsp[-3].List_ss != NULL) delete yyvsp[-3].List_ss;
            if (yyvsp[-2].List_ss != NULL) delete yyvsp[-2].List_ss;
            if (yyvsp[-1].Pair_ss_int != NULL) delete yyvsp[-1].Pair_ss_int;
            if (yyvsp[0].Map_Type_int != NULL) delete yyvsp[0].Map_Type_int;
        ;
    break;}
case 57:
#line 679 "pal/csrparser.y"
{
            setScope(*yyvsp[-1].List_string,Logue::CALLEE_PROLOGUE,"CALLER");
        ;
    break;}
case 58:
#line 684 "pal/csrparser.y"
{
            setParameters(*yyvsp[-4].List_string,*yyvsp[-1].param);
            delete yyvsp[-1].param;
            delete yyvsp[-4].List_string;
        ;
    break;}
case 61:
#line 703 "pal/csrparser.y"
{
            // The semantic string for the register will have a default size
            // (e.g. 32 for mc68k %d0). Change to the size given
            yyvsp[0].ss->getType().setSize(yyvsp[-2].num);
            yyval.Pair_Type_ss = new pair<Type, SemStr>(Type(::INTEGER, yyvsp[-2].num),*yyvsp[0].ss);
        ;
    break;}
case 62:
#line 710 "pal/csrparser.y"
{
            yyvsp[0].ss->getType().setSize(yyvsp[-2].num);
            yyval.Pair_Type_ss = new pair<Type, SemStr>(Type(::FLOATP, yyvsp[-2].num),*yyvsp[0].ss);
        ;
    break;}
case 63:
#line 715 "pal/csrparser.y"
{
            yyval.Pair_Type_ss = new pair<Type, SemStr>(Type(::DATA_ADDRESS),*yyvsp[0].ss);
        ;
    break;}
case 64:
#line 722 "pal/csrparser.y"
{
            yyval.Map_Type_ss = new typeToSemStrMap;
            yyval.Map_Type_ss->insert(*yyvsp[0].Pair_Type_ss);
            delete yyvsp[0].Pair_Type_ss;
        ;
    break;}
case 65:
#line 728 "pal/csrparser.y"
{
            yyval.Map_Type_ss = yyvsp[-1].Map_Type_ss;
            yyval.Map_Type_ss->insert(*yyvsp[0].Pair_Type_ss);
            delete yyvsp[0].Pair_Type_ss;
        ;
    break;}
case 66:
#line 737 "pal/csrparser.y"
{
            setScope(*yyvsp[-1].List_string,Logue::CALLEE_EPILOGUE,"CALLER");
        ;
    break;}
case 67:
#line 742 "pal/csrparser.y"
{
            setReturnLocations(*yyvsp[-4].List_string,ReturnLocations(*yyvsp[-1].Map_Type_ss));
            delete yyvsp[-4].List_string;
            delete yyvsp[-1].Map_Type_ss;
        ;
    break;}
case 70:
#line 761 "pal/csrparser.y"
{
            yyval.parentStack = new ParentStack(*yyvsp[-4].ss,*yyvsp[-2].ss,yyvsp[0].num);
            delete yyvsp[-4].ss;
            delete yyvsp[-2].ss;
        ;
    break;}
case 71:
#line 770 "pal/csrparser.y"
{
            setScope(*yyvsp[-1].List_string,Logue::CALLEE_PROLOGUE);
        ;
    break;}
case 72:
#line 774 "pal/csrparser.y"
{
            setParentStackSpecs(*yyvsp[-4].List_string,*yyvsp[-1].parentStack);
            delete yyvsp[-1].parentStack;
            delete yyvsp[-4].List_string;
        ;
    break;}
}

#line 783 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
   /* the action file gets copied in in place of this dollarsign  */
  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YY_CSRParser_LSP_NEEDED
  yylsp -= yylen;
#endif

#if YY_CSRParser_DEBUG != 0
  if (YY_CSRParser_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YY_CSRParser_LSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = YY_CSRParser_LLOC.first_line;
      yylsp->first_column = YY_CSRParser_LLOC.first_column;
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
      ++YY_CSRParser_NERRS;

#ifdef YY_CSRParser_ERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       (unsigned)x < (sizeof(yytname) / sizeof(char *)); x++)
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
		       (unsigned)x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      YY_CSRParser_ERROR(msg);
	      free(msg);
	    }
	  else
	    YY_CSRParser_ERROR ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YY_CSRParser_ERROR_VERBOSE */
	YY_CSRParser_ERROR("parse error");
    }

  YYGOTO(yyerrlab1);
YYLABEL(yyerrlab1)   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (YY_CSRParser_CHAR == YYEOF)
	YYABORT;

#if YY_CSRParser_DEBUG != 0
      if (YY_CSRParser_DEBUG_FLAG)
	fprintf(stderr, "Discarding token %d (%s).\n", YY_CSRParser_CHAR, yytname[yychar1]);
#endif

      YY_CSRParser_CHAR = YYEMPTY;
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
#ifdef YY_CSRParser_LSP_NEEDED
  yylsp--;
#endif

#if YY_CSRParser_DEBUG != 0
  if (YY_CSRParser_DEBUG_FLAG)
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

#if YY_CSRParser_DEBUG != 0
  if (YY_CSRParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = YY_CSRParser_LVAL;
#ifdef YY_CSRParser_LSP_NEEDED
  *++yylsp = YY_CSRParser_LLOC;
#endif

  yystate = yyn;
  YYGOTO(yynewstate);
/* end loop, in which YYGOTO may be used. */
  YYENDGOTO
}

/* END */

/* #line 982 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 2062 "pal/csrparser.cc"
#line 786 "pal/csrparser.y"


/*==============================================================================
 * FUNCTION:        CSRParser::~CSRParser
 * OVERVIEW:        Destructor.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
CSRParser::~CSRParser()
{
    if (theScanner != NULL)
        delete theScanner;
}

/*==============================================================================
 * FUNCTION:        CSRParser::yyerror
 * OVERVIEW:        Display an error message and exit.
 * PARAMETERS:      msg - an error message
 * RETURNS:         <nothing>
 *============================================================================*/
void CSRParser::yyerror(char* msg)
{
    cerr << csrFile << ":" << theScanner->theLine << ": " << msg << endl;
    exit(1);
}

/*==============================================================================
 * FUNCTION:        CSRParser::yylex
 * OVERVIEW:        The scanner driver than returns the next token.
 * PARAMETERS:      <none>
 * RETURNS:         the next token
 *============================================================================*/
int CSRParser::yylex()
{
    int token = theScanner->yylex(yylval);
    return token;
}

/*==============================================================================
 * FUNCTION:        CSRParser::getLogue
 * OVERVIEW:        A wrapper around direct indexing into logueDict to
 *                  issue a warning and stop if the result is NULL.
 * PARAMETERS:      name - the name of the logue to retrieve
 * RETURNS:         a pointer to the requested logue
 *============================================================================*/
Logue* CSRParser::getLogue(const string& name)
{
    Logue* logue = logueDict[name];
    if (logue == NULL) {
        ostrstream ost;
        ost << "`" << name << "' undeclared";
        yyerror(str(ost));
    }
    return logue;
}

/*==============================================================================
 * FUNCTION:        CSRParser::setScope
 * OVERVIEW:        Set the parameter name scope as well as checking
 *                  that each logue from which these names are
 *                  extracted is of the expected logue type
 * PARAMETERS:      logues - a list of logue names
 *                  type - a logue type
 * RETURNS:         <nothing>
 *============================================================================*/
void CSRParser::setScope(list<string>& logues, Logue::TYPE type,
    const char* exception /*= NULL*/)
{
    // Clear the current scope.
    clearScope();

    bool scopeInit = false;
    for (list<string>::iterator it = logues.begin();
        it != logues.end(); it++) {

        if (exception != NULL && *it == exception)
            continue;

        // Ensure that the logue has been declared and is of the
        // expected type
        Logue* logue = getLogue(*it);
        if (logue->getType() != type) {
            ostrstream ost;
            ost << "can't use a `" << Logue::TYPE_NAMES[logue->getType()];
            ost << "' where a `" << Logue::TYPE_NAMES[type] << "' is ";
            ost << "expected";
            yyerror(str(ost));
        }

        // If this is the first logue in the list, set its parameters
        // to be the current scope.
        // if (it == logues.begin() || !scopeInit) {
        if (!scopeInit) {
            logue->getFormals(scope);
            scopeInit = true;
        }

        // Otherwise remove any names from the scope that aren't in
        // the formals of the current logue
        else {
            set<string> formals;
            logue->getFormals(formals);

            // Really want to use STL set_intersection algorithm but
            // get unexpected const warnings if I do
            /*
            set<string> newScope;
            set_intersection(scope.begin(), scope.end(),
                formals.begin(), formals.end(), newScope.begin());
            scope.swap(newScope);
            */
            for (set<string>::iterator it = scope.begin();
                it != scope.end();it++)
                if (formals.find(*it) == formals.end())
                    scope.erase(it);
        }
    }
}

/*==============================================================================
 * FUNCTION:        CSRParser::checkScope
 * OVERVIEW:        Check that the given name is in the current scope.
 *                  Calls yyerror if it isn't.
 * PARAMETERS:      name - a name to check
 * RETURNS:         <nothing>
 *============================================================================*/
void CSRParser::checkScope(const string& name)
{
    if (scope.find(name) == scope.end()) {
        ostrstream ost;
        ost << "`" << name << "' not declared in the current scope";
        yyerror(str(ost));
    }
}

/*==============================================================================
 * FUNCTION:        CSRParser::clearScope
 * OVERVIEW:        Clear the current parameter name scope.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void CSRParser::clearScope()
{
    scope.clear();
}

/*==============================================================================
 * FUNCTION:        CSRParser::setParameters
 * OVERVIEW:        Given a list of names and a parameters spec,
 *                  set the spec to the callee epilogues corresponding to
 *                  the names.
 * PARAMETERS:      names - a list of names
 *                  spec - a parameters spec
 * RETURNS:         <nothing>
 *============================================================================*/
void CSRParser::setParameters(const list<string>& names,
    const Parameters& spec)
{
    for (list<string>::const_iterator it = names.begin();
        it != names.end(); it++) {

        // The CALLER name is special in that it says the
        // the parameters spec is to be used as the spec
        // for how parameters are passed by a caller
        if (*it == "CALLER") {
            callerSpecified = true;
            outgoingParams = spec;
        }
        else {
            CalleePrologue* logue = (CalleePrologue*)getLogue(*it);
            logue->setParamSpec(spec);
        }
    }
}

/*==============================================================================
 * FUNCTION:        CSRParser::setReturnLocations
 * OVERVIEW:        Given a list of Logue names and a return location spec,
 *                  set the spec to callee epilogues corresponding to
 *                  the names.
 * PARAMETERS:      names - a list of names
 *                  spec - a return locations spec
 * RETURNS:         <nothing>
 *============================================================================*/
void CSRParser::setReturnLocations(const list<string>& names,
    const ReturnLocations& spec)
{
    for (list<string>::const_iterator it = names.begin();
      it != names.end(); it++) {

        // The CALLER name is special in that it says the
        // the return spec is to be used as the location spec
        // for where returned parameters are found from a
        // caller's point of view (i.e. where they are
        // received)
        if (*it == "CALLER") {
            receiverSpecified = true;
            retLocations = spec;
        }
        else {
            CalleeEpilogue* logue = (CalleeEpilogue*)getLogue(*it);
            logue->setRetSpec(spec);
        }
    }
}

/*==============================================================================
 * FUNCTION:        CSRParser::setParentStackSpecs
 * OVERVIEW:        Given a list of names and a parent stack spec,
 *                  set the spec to callee prologues corresponding to
 *                  the names.
 * PARAMETERS:      names - a list of names
 *                  spec - a parent stack spec
 * RETURNS:         <nothing>
 *============================================================================*/
void CSRParser::setParentStackSpecs(const list<string>& names,
    const ParentStack& spec)
{
    for (list<string>::const_iterator it = names.begin();
        it != names.end(); it++) {

        CalleePrologue* logue = (CalleePrologue*)getLogue(*it);
        logue->setParentStackSpec(spec);
    }
}
