#ifndef YY_typeAnalysis_h_included
#define YY_typeAnalysis_h_included

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
#line 21 "include/typeAnalysis.h"
#line 4 "typeAnalysis/typeAnalysis.y"

#include "global.h"
// #include "analyzeBlocks.h"
// #include "typeLex.h"
class typeLex;
class tokenType;

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

#line 14 "/usr/local/lib/bison.h"
 /* %{ and %header{ and %union, during decl */
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
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_typeAnalysis_STYPE 
#define YY_typeAnalysis_STYPE YYSTYPE
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
/* use %define STYPE */
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_typeAnalysis_DEBUG
#define  YY_typeAnalysis_DEBUG YYDEBUG
/* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
/* use %define DEBUG */
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

/* #line 63 "/usr/local/lib/bison.h" */
#line 129 "include/typeAnalysis.h"

#line 63 "/usr/local/lib/bison.h"
/* YY_typeAnalysis_PURE */
#endif

/* #line 65 "/usr/local/lib/bison.h" */
#line 136 "include/typeAnalysis.h"

#line 65 "/usr/local/lib/bison.h"
/* prefix */
#ifndef YY_typeAnalysis_DEBUG

/* #line 67 "/usr/local/lib/bison.h" */
#line 143 "include/typeAnalysis.h"

#line 67 "/usr/local/lib/bison.h"
/* YY_typeAnalysis_DEBUG */
#endif
#ifndef YY_typeAnalysis_LSP_NEEDED

/* #line 70 "/usr/local/lib/bison.h" */
#line 151 "include/typeAnalysis.h"

#line 70 "/usr/local/lib/bison.h"
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

/* TOKEN C */
#ifndef YY_USE_CLASS

#ifndef YY_typeAnalysis_PURE
extern YY_typeAnalysis_STYPE YY_typeAnalysis_LVAL;
#endif


/* #line 143 "/usr/local/lib/bison.h" */
#line 229 "include/typeAnalysis.h"
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


#line 143 "/usr/local/lib/bison.h"
 /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
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

/* #line 182 "/usr/local/lib/bison.h" */
#line 312 "include/typeAnalysis.h"
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


#line 182 "/usr/local/lib/bison.h"
 /* decl const */
#else
enum YY_typeAnalysis_ENUM_TOKEN { YY_typeAnalysis_NULL_TOKEN=0

/* #line 185 "/usr/local/lib/bison.h" */
#line 359 "include/typeAnalysis.h"
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


#line 185 "/usr/local/lib/bison.h"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_typeAnalysis_PARSE(YY_typeAnalysis_PARSE_PARAM);
 virtual void YY_typeAnalysis_ERROR(char *msg) YY_typeAnalysis_ERROR_BODY;
#ifdef YY_typeAnalysis_PURE
#ifdef YY_typeAnalysis_LSP_NEEDED
 virtual int  YY_typeAnalysis_LEX(YY_typeAnalysis_STYPE *YY_typeAnalysis_LVAL,YY_typeAnalysis_LTYPE *YY_typeAnalysis_LLOC) YY_typeAnalysis_LEX_BODY;
#else
 virtual int  YY_typeAnalysis_LEX(YY_typeAnalysis_STYPE *YY_typeAnalysis_LVAL) YY_typeAnalysis_LEX_BODY;
#endif
#else
 virtual int YY_typeAnalysis_LEX() YY_typeAnalysis_LEX_BODY;
 YY_typeAnalysis_STYPE YY_typeAnalysis_LVAL;
#ifdef YY_typeAnalysis_LSP_NEEDED
 YY_typeAnalysis_LTYPE YY_typeAnalysis_LLOC;
#endif
 int YY_typeAnalysis_NERRS;
 int YY_typeAnalysis_CHAR;
#endif
#if YY_typeAnalysis_DEBUG != 0
public:
 int YY_typeAnalysis_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
#endif
public:
 YY_typeAnalysis_CLASS(YY_typeAnalysis_CONSTRUCTOR_PARAM);
public:
 YY_typeAnalysis_MEMBERS 
};
/* other declare folow */
#endif


#if YY_typeAnalysis_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE YY_typeAnalysis_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE YY_typeAnalysis_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_typeAnalysis_DEBUG 
#define YYDEBUG YY_typeAnalysis_DEBUG
#endif
#endif

#endif
/* END */

/* #line 236 "/usr/local/lib/bison.h" */
#line 454 "include/typeAnalysis.h"
#endif
