#ifndef YY_InstrmParser_h_included
#define YY_InstrmParser_h_included

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
#line 21 "include/instrm_parser.h"
#line 16 "dynamic/tools/instrm/instrmparser.y"

// #include "global.h"

#include <iostream>
#include <strstream>
#include <string>
#include <set>
#include <list>
#include <map>

class InstrmScanner;

#define T_CODE 1
#define T_SSL 2
#define T_SEMANTICS 3
#define T_PARAM 4

class table_code {	// instrument code for all the instructions within the table
public:
	// table_code() { tableset = new set<string>; }
	set<string>					tableset;			// all instructions belowing to the table
	list< pair<int, void*> > 	instrm_code;		// the instrumented code for this table
	// string 					before, after;
	list<string> 				*param_list;
};


#line 50 "dynamic/tools/instrm/instrmparser.y"
typedef union {
    char*			str;
    int             num;
	list<string>*	lstring;
} yy_InstrmParser_stype;
#define YY_InstrmParser_STYPE yy_InstrmParser_stype
#define YY_InstrmParser_DEBUG  1 
#define YY_InstrmParser_PARSE_PARAM  \

#define YY_InstrmParser_CONSTRUCTOR_PARAM  \
    const string& instrmFile, \
    bool trace
#define YY_InstrmParser_CONSTRUCTOR_CODE  \
    FILE* inFile = fopen(instrmFile.c_str(),"r"); \
    if (inFile == NULL) { \
        ostrstream ost; \
        ost << "can't open `" << instrmFile << "' for reading"; \
        abort(); \
    } \
    Scanner = new InstrmScanner(inFile, trace); \
    if (trace) yydebug = 1;
#define YY_InstrmParser_MEMBERS  \
public: \
        virtual ~InstrmParser(); \
		map<string, table_code*> table2icode; \
		map<string, table_code*> inst2icode;  \
		string support_code; \
protected: \
\
    /** \
     * The scanner. \
     */ \
    InstrmScanner* Scanner; \
	table_code				 *current_table;	\
	// use to hold current instructions in the table

#line 14 "/usr/local/lib/bison.h"
 /* %{ and %header{ and %union, during decl */
#ifndef YY_InstrmParser_COMPATIBILITY
#ifndef YY_USE_CLASS
#define  YY_InstrmParser_COMPATIBILITY 1
#else
#define  YY_InstrmParser_COMPATIBILITY 0
#endif
#endif

#if YY_InstrmParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YYLTYPE
#ifndef YY_InstrmParser_LTYPE
#define YY_InstrmParser_LTYPE YYLTYPE
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_InstrmParser_STYPE 
#define YY_InstrmParser_STYPE YYSTYPE
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
/* use %define STYPE */
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_InstrmParser_DEBUG
#define  YY_InstrmParser_DEBUG YYDEBUG
/* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
/* use %define DEBUG */
#endif
#endif
#ifdef YY_InstrmParser_STYPE
#ifndef yystype
#define yystype YY_InstrmParser_STYPE
#endif
#endif
/* use goto to be compatible */
#ifndef YY_InstrmParser_USE_GOTO
#define YY_InstrmParser_USE_GOTO 1
#endif
#endif

/* use no goto to be clean in C++ */
#ifndef YY_InstrmParser_USE_GOTO
#define YY_InstrmParser_USE_GOTO 0
#endif

#ifndef YY_InstrmParser_PURE

/* #line 63 "/usr/local/lib/bison.h" */
#line 139 "include/instrm_parser.h"

#line 63 "/usr/local/lib/bison.h"
/* YY_InstrmParser_PURE */
#endif

/* #line 65 "/usr/local/lib/bison.h" */
#line 146 "include/instrm_parser.h"

#line 65 "/usr/local/lib/bison.h"
/* prefix */
#ifndef YY_InstrmParser_DEBUG

/* #line 67 "/usr/local/lib/bison.h" */
#line 153 "include/instrm_parser.h"

#line 67 "/usr/local/lib/bison.h"
/* YY_InstrmParser_DEBUG */
#endif
#ifndef YY_InstrmParser_LSP_NEEDED

/* #line 70 "/usr/local/lib/bison.h" */
#line 161 "include/instrm_parser.h"

#line 70 "/usr/local/lib/bison.h"
 /* YY_InstrmParser_LSP_NEEDED*/
#endif
/* DEFAULT LTYPE*/
#ifdef YY_InstrmParser_LSP_NEEDED
#ifndef YY_InstrmParser_LTYPE
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

#define YY_InstrmParser_LTYPE yyltype
#endif
#endif
/* DEFAULT STYPE*/
#ifndef YY_InstrmParser_STYPE
#define YY_InstrmParser_STYPE int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_InstrmParser_PARSE
#define YY_InstrmParser_PARSE yyparse
#endif
#ifndef YY_InstrmParser_LEX
#define YY_InstrmParser_LEX yylex
#endif
#ifndef YY_InstrmParser_LVAL
#define YY_InstrmParser_LVAL yylval
#endif
#ifndef YY_InstrmParser_LLOC
#define YY_InstrmParser_LLOC yylloc
#endif
#ifndef YY_InstrmParser_CHAR
#define YY_InstrmParser_CHAR yychar
#endif
#ifndef YY_InstrmParser_NERRS
#define YY_InstrmParser_NERRS yynerrs
#endif
#ifndef YY_InstrmParser_DEBUG_FLAG
#define YY_InstrmParser_DEBUG_FLAG yydebug
#endif
#ifndef YY_InstrmParser_ERROR
#define YY_InstrmParser_ERROR yyerror
#endif

#ifndef YY_InstrmParser_PARSE_PARAM
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
#define YY_InstrmParser_PARSE_PARAM
#ifndef YY_InstrmParser_PARSE_PARAM_DEF
#define YY_InstrmParser_PARSE_PARAM_DEF
#endif
#endif
#endif
#endif
#ifndef YY_InstrmParser_PARSE_PARAM
#define YY_InstrmParser_PARSE_PARAM void
#endif
#endif

/* TOKEN C */
#ifndef YY_USE_CLASS

#ifndef YY_InstrmParser_PURE
extern YY_InstrmParser_STYPE YY_InstrmParser_LVAL;
#endif


/* #line 143 "/usr/local/lib/bison.h" */
#line 239 "include/instrm_parser.h"
#define	INSTRUMENTATION_ROUTINES	258
#define	FETCHEXECUTE	259
#define	DEFINITION	260
#define	NAME	261
#define	SSL	262
#define	PARAM	263
#define	C_CODE	264
#define	INSTRM_END	265
#define	SSL_INST_SEMANTICS	266


#line 143 "/usr/local/lib/bison.h"
 /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
#ifndef YY_InstrmParser_CLASS
#define YY_InstrmParser_CLASS InstrmParser
#endif

#ifndef YY_InstrmParser_INHERIT
#define YY_InstrmParser_INHERIT
#endif
#ifndef YY_InstrmParser_MEMBERS
#define YY_InstrmParser_MEMBERS 
#endif
#ifndef YY_InstrmParser_LEX_BODY
#define YY_InstrmParser_LEX_BODY  
#endif
#ifndef YY_InstrmParser_ERROR_BODY
#define YY_InstrmParser_ERROR_BODY  
#endif
#ifndef YY_InstrmParser_CONSTRUCTOR_PARAM
#define YY_InstrmParser_CONSTRUCTOR_PARAM
#endif
/* choose between enum and const */
#ifndef YY_InstrmParser_USE_CONST_TOKEN
#define YY_InstrmParser_USE_CONST_TOKEN 0
/* yes enum is more compatible with flex,  */
/* so by default we use it */ 
#endif
#if YY_InstrmParser_USE_CONST_TOKEN != 0
#ifndef YY_InstrmParser_ENUM_TOKEN
#define YY_InstrmParser_ENUM_TOKEN yy_InstrmParser_enum_token
#endif
#endif

class YY_InstrmParser_CLASS YY_InstrmParser_INHERIT
{
public: 
#if YY_InstrmParser_USE_CONST_TOKEN != 0
/* static const int token ... */

/* #line 182 "/usr/local/lib/bison.h" */
#line 293 "include/instrm_parser.h"
static const int INSTRUMENTATION_ROUTINES;
static const int FETCHEXECUTE;
static const int DEFINITION;
static const int NAME;
static const int SSL;
static const int PARAM;
static const int C_CODE;
static const int INSTRM_END;
static const int SSL_INST_SEMANTICS;


#line 182 "/usr/local/lib/bison.h"
 /* decl const */
#else
enum YY_InstrmParser_ENUM_TOKEN { YY_InstrmParser_NULL_TOKEN=0

/* #line 185 "/usr/local/lib/bison.h" */
#line 311 "include/instrm_parser.h"
	,INSTRUMENTATION_ROUTINES=258
	,FETCHEXECUTE=259
	,DEFINITION=260
	,NAME=261
	,SSL=262
	,PARAM=263
	,C_CODE=264
	,INSTRM_END=265
	,SSL_INST_SEMANTICS=266


#line 185 "/usr/local/lib/bison.h"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_InstrmParser_PARSE(YY_InstrmParser_PARSE_PARAM);
 virtual void YY_InstrmParser_ERROR(char *msg) YY_InstrmParser_ERROR_BODY;
#ifdef YY_InstrmParser_PURE
#ifdef YY_InstrmParser_LSP_NEEDED
 virtual int  YY_InstrmParser_LEX(YY_InstrmParser_STYPE *YY_InstrmParser_LVAL,YY_InstrmParser_LTYPE *YY_InstrmParser_LLOC) YY_InstrmParser_LEX_BODY;
#else
 virtual int  YY_InstrmParser_LEX(YY_InstrmParser_STYPE *YY_InstrmParser_LVAL) YY_InstrmParser_LEX_BODY;
#endif
#else
 virtual int YY_InstrmParser_LEX() YY_InstrmParser_LEX_BODY;
 YY_InstrmParser_STYPE YY_InstrmParser_LVAL;
#ifdef YY_InstrmParser_LSP_NEEDED
 YY_InstrmParser_LTYPE YY_InstrmParser_LLOC;
#endif
 int YY_InstrmParser_NERRS;
 int YY_InstrmParser_CHAR;
#endif
#if YY_InstrmParser_DEBUG != 0
public:
 int YY_InstrmParser_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
#endif
public:
 YY_InstrmParser_CLASS(YY_InstrmParser_CONSTRUCTOR_PARAM);
public:
 YY_InstrmParser_MEMBERS 
};
/* other declare folow */
#endif


#if YY_InstrmParser_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE YY_InstrmParser_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE YY_InstrmParser_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_InstrmParser_DEBUG 
#define YYDEBUG YY_InstrmParser_DEBUG
#endif
#endif

#endif
/* END */

/* #line 236 "/usr/local/lib/bison.h" */
#line 377 "include/instrm_parser.h"
#endif
