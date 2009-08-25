#ifndef YY_PatternParser_h_included
#define YY_PatternParser_h_included

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
#line 21 "include/patternparser.h"
#define YY_PatternParser_DEBUG  1
#define YY_PatternParser_DEBUG_INIT  1
#define YY_PatternParser_ERROR_VERBOSE 
#line 36 "pal/patternparser.y"

#include <string>
#include <map>
#include <set>
#include <list>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <strstream>

using namespace std;

// Return the null terminated string of an ostrstream object
char* str(ostrstream& os);

// Define operator+ to add integers to a string
string operator+(const string& s, int i);

class PatternScanner;
class SymbolTable;
class Constructor;
class ArgWrap;
class OuterConstructor;
class InnerConstructor;
struct arg_pair_ {
    ArgWrap* arg;
    string inst;
};

typedef arg_pair_ arg_pair;


#line 73 "pal/patternparser.y"
typedef union{
    char*           str;
    int*            numPtr;
    string*         String;
    int             num;
    list<string>*   List_string;
    arg_pair*        arg;
    list<InnerConstructor*>* argList;
} yy_PatternParser_stype;
#define YY_PatternParser_STYPE yy_PatternParser_stype
#define YY_PatternParser_MEMBERS  \
public: \
    virtual ~PatternParser(); \
\
private: \
\
    /** \
     * Scanner used to get tokens. \
     */ \
    PatternScanner* theScanner; \
\
    /** \
     * The name of the .pat file to be read. \
     */ \
    string patFile; \
\
    /** \
     * The streams to use for the generated code. \
     */ \
    ofstream code_os; \
    ofstream header_os; \
    int iterhlps; \
\
    /** \
     * The set of valid type names. \
     */ \
    set<string> typeNames; \
\
    /** \
     * Symbol table. \
     */ \
    SymbolTable* symTab;\
\
    /** \
     * The code of LogueDict constructor. \
     */ \
    ostrstream dict_os; \
\
    /** \
     * The name of the function that will be generated from the \
     * current constructor application being parsed. \
     */ \
    string functionCall; \
\
    /** \
     * The inner constructors used to decode the parameters of a \
     * constructor application. \
     */ \
    list<InnerConstructor*> innerCons; \
\
    /** \
     * Keeps track of all the functions that have to be generated. \
     */ \
    map<string,Constructor*> functionTable; \
\
    /** \
     * Set to false when inside a pattern application within a \
     * pattern definition (i.e. inside a `@' construct). \
     */ \
    bool emit_sig; \
\
    /** \
     * Set to true when a pattern requires the __loop variable; \
     */ \
    bool needLoop; \
\
    /** \
     * Build the code generated for an iteration match e.g.: \
     *   POPod($EDI) *<0..3> \
     */ \
    string* buildIterExpr(string* pattn, string* var, string* range);
#define YY_PatternParser_CONSTRUCTOR_PARAM  \
    const string& fileName, \
    const set<string>& types, \
    bool trace = false
#define YY_PatternParser_CONSTRUCTOR_INIT  : \
    patFile(fileName), typeNames(types), emit_sig(true)\

#define YY_PatternParser_CONSTRUCTOR_CODE  \
    needLoop = false; \
    FILE* inFile = fopen(patFile.c_str(),"r"); \
    theScanner = new PatternScanner(inFile,trace); \
    symTab = new SymbolTable(); \
    if (trace) yydebug = 1; \
\
    code_os.open((patFile + ".m").c_str()); \
    iterhlps = 0; \
    header_os.open((patFile + ".h").c_str()); \
    patFile.replace(0,patFile.find_last_of('/')+1,"");

#line 14 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
 /* %{ and %header{ and %union, during decl */
#ifndef YY_PatternParser_COMPATIBILITY
#ifndef YY_USE_CLASS
#define  YY_PatternParser_COMPATIBILITY 1
#else
#define  YY_PatternParser_COMPATIBILITY 0
#endif
#endif

#if YY_PatternParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YYLTYPE
#ifndef YY_PatternParser_LTYPE
#define YY_PatternParser_LTYPE YYLTYPE
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_PatternParser_STYPE 
#define YY_PatternParser_STYPE YYSTYPE
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
/* use %define STYPE */
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_PatternParser_DEBUG
#define  YY_PatternParser_DEBUG YYDEBUG
/* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
/* use %define DEBUG */
#endif
#endif
#ifdef YY_PatternParser_STYPE
#ifndef yystype
#define yystype YY_PatternParser_STYPE
#endif
#endif
/* use goto to be compatible */
#ifndef YY_PatternParser_USE_GOTO
#define YY_PatternParser_USE_GOTO 1
#endif
#endif

/* use no goto to be clean in C++ */
#ifndef YY_PatternParser_USE_GOTO
#define YY_PatternParser_USE_GOTO 0
#endif

#ifndef YY_PatternParser_PURE

/* #line 63 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 212 "include/patternparser.h"

#line 63 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
/* YY_PatternParser_PURE */
#endif

/* #line 65 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 219 "include/patternparser.h"

#line 65 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
/* prefix */
#ifndef YY_PatternParser_DEBUG

/* #line 67 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 226 "include/patternparser.h"

#line 67 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
/* YY_PatternParser_DEBUG */
#endif
#ifndef YY_PatternParser_LSP_NEEDED

/* #line 70 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 234 "include/patternparser.h"

#line 70 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
 /* YY_PatternParser_LSP_NEEDED*/
#endif
/* DEFAULT LTYPE*/
#ifdef YY_PatternParser_LSP_NEEDED
#ifndef YY_PatternParser_LTYPE
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

#define YY_PatternParser_LTYPE yyltype
#endif
#endif
/* DEFAULT STYPE*/
#ifndef YY_PatternParser_STYPE
#define YY_PatternParser_STYPE int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_PatternParser_PARSE
#define YY_PatternParser_PARSE yyparse
#endif
#ifndef YY_PatternParser_LEX
#define YY_PatternParser_LEX yylex
#endif
#ifndef YY_PatternParser_LVAL
#define YY_PatternParser_LVAL yylval
#endif
#ifndef YY_PatternParser_LLOC
#define YY_PatternParser_LLOC yylloc
#endif
#ifndef YY_PatternParser_CHAR
#define YY_PatternParser_CHAR yychar
#endif
#ifndef YY_PatternParser_NERRS
#define YY_PatternParser_NERRS yynerrs
#endif
#ifndef YY_PatternParser_DEBUG_FLAG
#define YY_PatternParser_DEBUG_FLAG yydebug
#endif
#ifndef YY_PatternParser_ERROR
#define YY_PatternParser_ERROR yyerror
#endif

#ifndef YY_PatternParser_PARSE_PARAM
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
#define YY_PatternParser_PARSE_PARAM
#ifndef YY_PatternParser_PARSE_PARAM_DEF
#define YY_PatternParser_PARSE_PARAM_DEF
#endif
#endif
#endif
#endif
#ifndef YY_PatternParser_PARSE_PARAM
#define YY_PatternParser_PARSE_PARAM void
#endif
#endif

/* TOKEN C */
#ifndef YY_USE_CLASS

#ifndef YY_PatternParser_PURE
extern YY_PatternParser_STYPE YY_PatternParser_LVAL;
#endif


/* #line 143 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 312 "include/patternparser.h"
#define	IDENT	258
#define	NUM	259
#define	WILD	260
#define	IS	261
#define	NAMES	262
#define	UPTO	263
#define	PATTERNS	264
#define	TYPES	265
#define	NONE	266


#line 143 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
 /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
#ifndef YY_PatternParser_CLASS
#define YY_PatternParser_CLASS PatternParser
#endif

#ifndef YY_PatternParser_INHERIT
#define YY_PatternParser_INHERIT
#endif
#ifndef YY_PatternParser_MEMBERS
#define YY_PatternParser_MEMBERS 
#endif
#ifndef YY_PatternParser_LEX_BODY
#define YY_PatternParser_LEX_BODY  
#endif
#ifndef YY_PatternParser_ERROR_BODY
#define YY_PatternParser_ERROR_BODY  
#endif
#ifndef YY_PatternParser_CONSTRUCTOR_PARAM
#define YY_PatternParser_CONSTRUCTOR_PARAM
#endif
/* choose between enum and const */
#ifndef YY_PatternParser_USE_CONST_TOKEN
#define YY_PatternParser_USE_CONST_TOKEN 0
/* yes enum is more compatible with flex,  */
/* so by default we use it */
#endif
#if YY_PatternParser_USE_CONST_TOKEN != 0
#ifndef YY_PatternParser_ENUM_TOKEN
#define YY_PatternParser_ENUM_TOKEN yy_PatternParser_enum_token
#endif
#endif

class YY_PatternParser_CLASS YY_PatternParser_INHERIT
{
public:
#if YY_PatternParser_USE_CONST_TOKEN != 0
/* static const int token ... */

/* #line 182 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 366 "include/patternparser.h"
static const int IDENT;
static const int NUM;
static const int WILD;
static const int IS;
static const int NAMES;
static const int UPTO;
static const int PATTERNS;
static const int TYPES;
static const int NONE;


#line 182 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
 /* decl const */
#else
enum YY_PatternParser_ENUM_TOKEN { YY_PatternParser_NULL_TOKEN=0

/* #line 185 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 384 "include/patternparser.h"
	,IDENT=258
	,NUM=259
	,WILD=260
	,IS=261
	,NAMES=262
	,UPTO=263
	,PATTERNS=264
	,TYPES=265
	,NONE=266


#line 185 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_PatternParser_PARSE(YY_PatternParser_PARSE_PARAM);
 virtual void YY_PatternParser_ERROR(char *msg) YY_PatternParser_ERROR_BODY;
#ifdef YY_PatternParser_PURE
#ifdef YY_PatternParser_LSP_NEEDED
 virtual int  YY_PatternParser_LEX(YY_PatternParser_STYPE *YY_PatternParser_LVAL,YY_PatternParser_LTYPE *YY_PatternParser_LLOC) YY_PatternParser_LEX_BODY;
#else
 virtual int  YY_PatternParser_LEX(YY_PatternParser_STYPE *YY_PatternParser_LVAL) YY_PatternParser_LEX_BODY;
#endif
#else
 virtual int YY_PatternParser_LEX() YY_PatternParser_LEX_BODY;
 YY_PatternParser_STYPE YY_PatternParser_LVAL;
#ifdef YY_PatternParser_LSP_NEEDED
 YY_PatternParser_LTYPE YY_PatternParser_LLOC;
#endif
 int YY_PatternParser_NERRS;
 int YY_PatternParser_CHAR;
#endif
#if YY_PatternParser_DEBUG != 0
public:
 int YY_PatternParser_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
#endif
public:
 YY_PatternParser_CLASS(YY_PatternParser_CONSTRUCTOR_PARAM);
public:
 YY_PatternParser_MEMBERS 
};
/* other declare folow */
#endif


#if YY_PatternParser_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE YY_PatternParser_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE YY_PatternParser_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_PatternParser_DEBUG 
#define YYDEBUG YY_PatternParser_DEBUG
#endif
#endif

#endif
/* END */

/* #line 236 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 450 "include/patternparser.h"
#endif
