#ifndef YY_SSLParser_h_included
#define YY_SSLParser_h_included

#line 1 "/usr/local/lib/bison.h"
/* before anything */
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
#endif
#include <stdio.h>

/* #line 14 "/usr/local/lib/bison.h" */
#line 21 "include/sslparser.h"
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

#line 14 "/usr/local/lib/bison.h"
 /* %{ and %header{ and %union, during decl */
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
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_SSLParser_STYPE 
#define YY_SSLParser_STYPE YYSTYPE
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
/* use %define STYPE */
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_SSLParser_DEBUG
#define  YY_SSLParser_DEBUG YYDEBUG
/* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
/* use %define DEBUG */
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

/* #line 63 "/usr/local/lib/bison.h" */
#line 168 "include/sslparser.h"

#line 63 "/usr/local/lib/bison.h"
/* YY_SSLParser_PURE */
#endif

/* #line 65 "/usr/local/lib/bison.h" */
#line 175 "include/sslparser.h"

#line 65 "/usr/local/lib/bison.h"
/* prefix */
#ifndef YY_SSLParser_DEBUG

/* #line 67 "/usr/local/lib/bison.h" */
#line 182 "include/sslparser.h"

#line 67 "/usr/local/lib/bison.h"
/* YY_SSLParser_DEBUG */
#endif
#ifndef YY_SSLParser_LSP_NEEDED

/* #line 70 "/usr/local/lib/bison.h" */
#line 190 "include/sslparser.h"

#line 70 "/usr/local/lib/bison.h"
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

/* TOKEN C */
#ifndef YY_USE_CLASS

#ifndef YY_SSLParser_PURE
extern YY_SSLParser_STYPE YY_SSLParser_LVAL;
#endif


/* #line 143 "/usr/local/lib/bison.h" */
#line 268 "include/sslparser.h"
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


#line 143 "/usr/local/lib/bison.h"
 /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
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

/* #line 182 "/usr/local/lib/bison.h" */
#line 359 "include/sslparser.h"
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


#line 182 "/usr/local/lib/bison.h"
 /* decl const */
#else
enum YY_SSLParser_ENUM_TOKEN { YY_SSLParser_NULL_TOKEN=0

/* #line 185 "/usr/local/lib/bison.h" */
#line 414 "include/sslparser.h"
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


#line 185 "/usr/local/lib/bison.h"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_SSLParser_PARSE(YY_SSLParser_PARSE_PARAM);
 virtual void YY_SSLParser_ERROR(char *msg) YY_SSLParser_ERROR_BODY;
#ifdef YY_SSLParser_PURE
#ifdef YY_SSLParser_LSP_NEEDED
 virtual int  YY_SSLParser_LEX(YY_SSLParser_STYPE *YY_SSLParser_LVAL,YY_SSLParser_LTYPE *YY_SSLParser_LLOC) YY_SSLParser_LEX_BODY;
#else
 virtual int  YY_SSLParser_LEX(YY_SSLParser_STYPE *YY_SSLParser_LVAL) YY_SSLParser_LEX_BODY;
#endif
#else
 virtual int YY_SSLParser_LEX() YY_SSLParser_LEX_BODY;
 YY_SSLParser_STYPE YY_SSLParser_LVAL;
#ifdef YY_SSLParser_LSP_NEEDED
 YY_SSLParser_LTYPE YY_SSLParser_LLOC;
#endif
 int YY_SSLParser_NERRS;
 int YY_SSLParser_CHAR;
#endif
#if YY_SSLParser_DEBUG != 0
public:
 int YY_SSLParser_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
#endif
public:
 YY_SSLParser_CLASS(YY_SSLParser_CONSTRUCTOR_PARAM);
public:
 YY_SSLParser_MEMBERS 
};
/* other declare folow */
#endif


#if YY_SSLParser_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE YY_SSLParser_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE YY_SSLParser_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_SSLParser_DEBUG 
#define YYDEBUG YY_SSLParser_DEBUG
#endif
#endif

#endif
/* END */

/* #line 236 "/usr/local/lib/bison.h" */
#line 517 "include/sslparser.h"
#endif
