#ifndef YY_CSRParser_h_included
#define YY_CSRParser_h_included

#line 1 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
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

/* #line 14 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 21 "include/csrparser.h"
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


#line 14 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
 /* %{ and %header{ and %union, during decl */
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
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_CSRParser_STYPE 
#define YY_CSRParser_STYPE YYSTYPE
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
/* use %define STYPE */
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_CSRParser_DEBUG
#define  YY_CSRParser_DEBUG YYDEBUG
/* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
/* use %define DEBUG */
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

/* #line 63 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 222 "include/csrparser.h"

#line 63 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
/* YY_CSRParser_PURE */
#endif

/* #line 65 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 229 "include/csrparser.h"

#line 65 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
/* prefix */
#ifndef YY_CSRParser_DEBUG

/* #line 67 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 236 "include/csrparser.h"

#line 67 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
/* YY_CSRParser_DEBUG */
#endif
#ifndef YY_CSRParser_LSP_NEEDED

/* #line 70 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 244 "include/csrparser.h"

#line 70 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
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

/* TOKEN C */
#ifndef YY_USE_CLASS

#ifndef YY_CSRParser_PURE
extern YY_CSRParser_STYPE YY_CSRParser_LVAL;
#endif


/* #line 143 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 322 "include/csrparser.h"
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


#line 143 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
 /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
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

/* #line 182 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 399 "include/csrparser.h"
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


#line 182 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
 /* decl const */
#else
enum YY_CSRParser_ENUM_TOKEN { YY_CSRParser_NULL_TOKEN=0

/* #line 185 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 440 "include/csrparser.h"
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


#line 185 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_CSRParser_PARSE(YY_CSRParser_PARSE_PARAM);
 virtual void YY_CSRParser_ERROR(char *msg) YY_CSRParser_ERROR_BODY;
#ifdef YY_CSRParser_PURE
#ifdef YY_CSRParser_LSP_NEEDED
 virtual int  YY_CSRParser_LEX(YY_CSRParser_STYPE *YY_CSRParser_LVAL,YY_CSRParser_LTYPE *YY_CSRParser_LLOC) YY_CSRParser_LEX_BODY;
#else
 virtual int  YY_CSRParser_LEX(YY_CSRParser_STYPE *YY_CSRParser_LVAL) YY_CSRParser_LEX_BODY;
#endif
#else
 virtual int YY_CSRParser_LEX() YY_CSRParser_LEX_BODY;
 YY_CSRParser_STYPE YY_CSRParser_LVAL;
#ifdef YY_CSRParser_LSP_NEEDED
 YY_CSRParser_LTYPE YY_CSRParser_LLOC;
#endif
 int YY_CSRParser_NERRS;
 int YY_CSRParser_CHAR;
#endif
#if YY_CSRParser_DEBUG != 0
public:
 int YY_CSRParser_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
#endif
public:
 YY_CSRParser_CLASS(YY_CSRParser_CONSTRUCTOR_PARAM);
public:
 YY_CSRParser_MEMBERS 
};
/* other declare folow */
#endif


#if YY_CSRParser_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE YY_CSRParser_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE YY_CSRParser_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_CSRParser_DEBUG 
#define YYDEBUG YY_CSRParser_DEBUG
#endif
#endif

#endif
/* END */

/* #line 236 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 529 "include/csrparser.h"
#endif
