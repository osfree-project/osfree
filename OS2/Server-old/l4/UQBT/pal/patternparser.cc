#define YY_PatternParser_h_included

/*  A Bison++ parser, made from pal/patternparser.y  */

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
#line 89 "pal/patternparser.cc"
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
#line 85 "pal/patternparser.y"

#include "patternscanner.h"

/*==============================================================================
 * These are the helper classes that support the code generation
 * performed by this parser. Given that will all have local scope,
 * their methods have defined inlined.
 *============================================================================*/

/*==============================================================================
 * The SymbolTable class is used to provide a basic symbol table that
 * records declarations and check uses of variables within a pattern
 * spec.
 *============================================================================*/
class SymbolTable {
public:
    SymbolTable();

    /**
     * Declare a parameter.
     */
    void declareParam(const string& sym);

    /**
     * Declare a macro name.
     */
    void declareMacro(const string& sym);

    /**
     * Set the default value for a symbol. This will be guaranteed to
     * be a parameter by the allowable grammar.
     */
    void setParamDefault(const string& name, int val);

    /**
     * Return the <param,default> map and clear the internal copy.
     */
    map<string,int> getAndClearDefaults();

    /**
     * Check the usage of a local or parameter with respect to a declaration.
     * If this symbol is not declared then it is implicitly declared as a
     * local. This is the only way a local is declared.
     */
    void useParamOrLocal(const string& sym);

    /**
     * Check that the given macro has been declared.
     */
    void useMacro(const string& sym);

    /**
     * Clear all entries of the given type.
     */
    void clear();

    /**
     * These methods are used to allocate anonymous locals that will
     * be used as fixed or wild operands to constructor applications.
     */
    string nextAnonLocal();
    void newAnonLocals();
    void resetAnonLocals();
    int numAnonLocals();

private:
    set<string> localSyms;
    set<string> paramSyms;
    set<string> macroSyms;

    int anonLocals;
    int maxAnonLocals;

    map<string,int> defaults;
};

class InnerConstructor;

class ArgWrap {
public:
    ArgWrap(void) : next(NULL) {}
    virtual ~ArgWrap(void) { if (next != NULL) delete next; }
    
    list<InnerConstructor*>* eval_v(void);

    int count(void) { return arg_size() + n_left(); }
    string eval_s(int start) { return as_string(start) + ((n_left() > 0) ? (", " + next->eval_s(start + arg_size())) : ""); }

    // return value as a Constructor
    virtual InnerConstructor* as_constr (void) = 0;
    // return value as a string (effectively, all of the parameters contained
    // within this argument
    //  STW(a, b, addr(base, off))
    // in this, the 'a' -> 'a', while the 'addr(..)' goes to 'base, off'
    virtual string as_string(int start) = 0;
    virtual int arg_size(void) = 0;
    
    int n_left(void) { return (next != NULL) ? next->count() : 0; }
    void push_back(ArgWrap *a) { if (next != NULL) next->push_back(a); else next = a; }

private:
    ArgWrap* next;

};

/*==============================================================================
 * This abstract class is the base for classes that are used to store
 * information about NJMC constructors. These classes are used to
 * generate both declarations and code for the functions that will
 * implement the decoding equivalent to these constructors.
 *============================================================================*/
class Constructor {
public:
    Constructor(const string& name, const string& cons) :
        name(name), constructor(cons) {}
    virtual ~Constructor() {}

    /**
     * Generate the code for a constructor.
     */
    virtual void genCode(ostream& os) const = 0;

    /**
     * Generate the declaration for a constructor.
     */
    virtual void genHeader(ostream& os) const = 0;

    /**
     * Return the number of parameters a constuctor takes.
     */
    virtual int  getNumParams() const = 0;

    /**
     * Build the declaration of a constructor function's signature. For example:
     *
     *   buildParamsDecl(2) ==
     *      "(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR)"
     */
    string getParams(int numArgs) const;

protected:
    string name;
    string constructor;
};

class ArgWrap;

/*==============================================================================
 * This class represents the application of an inner constructor (i.e.
 * a 'typed' constructor in SLED speak e.g. Disp8(4,ebp)
 *============================================================================*/
class InnerConstructor : public Constructor {
public:
    InnerConstructor(const string& name, const string& cons, ArgWrap* params) :
        Constructor(name, cons), arguments(params)  { if (params == NULL) numParams = 0; else numParams = params->count();}
    ~InnerConstructor() { delete arguments; }

    // Override Constructors methods of the same name
    virtual void genCode(ostream& os) const;
    virtual void genHeader(ostream& os) const;
    string getMatchCall(int matchoffset, int offset) const;
    virtual int getNumParams() const;

    ArgWrap* arguments;
private:
    int numParams;
};

/*==============================================================================
 * This class represents the application of an outer constructor (i.e.
 * an 'untyped' constructor in SLED speak e.g. POPod(esp)
 *============================================================================*/
class OuterConstructor : public InnerConstructor {
public:
    OuterConstructor(const string& name, const string& cons, ArgWrap *params) :
        InnerConstructor(name, cons, params) {}
    ~OuterConstructor() {}

    // Override Constructors methods of the same name
    virtual void genCode(ostream& os) const;
    virtual void genHeader(ostream& os) const;

private:
};



class ValueWrap : public ArgWrap {
public:
    ValueWrap(void) : ArgWrap() {}
    
    virtual InnerConstructor* as_constr (void) { return NULL; }
    virtual string as_string(int start) { char a[2] = "a"; a[0] = 'a' + start; return string(a); }
    virtual int arg_size(void) { return 1; }

private:
};

class ConstrWrap : public ArgWrap {
public:
    ConstrWrap(InnerConstructor* c) : ArgWrap(), val(c) {}
    virtual ~ConstrWrap(void) {}

    virtual InnerConstructor* as_constr(void) { return val; }
    virtual string as_string(int start) { return (val->arguments != NULL) ? val->arguments->as_string(start) : ""; }
    virtual int arg_size(void) { return (val->arguments != NULL) ? val->arguments->count() : 0; }

private:
    InnerConstructor* val;
};

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

#line 77 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
/* %{ and %header{ and %union, during decl */
#define YY_PatternParser_BISON 1
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
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_PatternParser_STYPE 
#define YY_PatternParser_STYPE YYSTYPE
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_PatternParser_DEBUG
#define  YY_PatternParser_DEBUG YYDEBUG
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

/* #line 121 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 487 "pal/patternparser.cc"

#line 121 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
/*  YY_PatternParser_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 125 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 496 "pal/patternparser.cc"

#line 125 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
/* prefix */
#ifndef YY_PatternParser_DEBUG

/* #line 127 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 503 "pal/patternparser.cc"

#line 127 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
/* YY_PatternParser_DEBUG */
#endif


#ifndef YY_PatternParser_LSP_NEEDED

/* #line 132 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 513 "pal/patternparser.cc"

#line 132 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
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
      /* We used to use `unsigned long' as YY_PatternParser_STYPE on MSDOS,
	 but it seems better to be consistent.
	 Most programs should declare their own type anyway.  */

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
#if YY_PatternParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YY_PatternParser_LTYPE
#ifndef YYLTYPE
#define YYLTYPE YY_PatternParser_LTYPE
#else
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
#endif
#endif
#ifndef YYSTYPE
#define YYSTYPE YY_PatternParser_STYPE
#else
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
#endif
#ifdef YY_PatternParser_PURE
#ifndef YYPURE
#define YYPURE YY_PatternParser_PURE
#endif
#endif
#ifdef YY_PatternParser_DEBUG
#ifndef YYDEBUG
#define YYDEBUG YY_PatternParser_DEBUG 
#endif
#endif
#ifndef YY_PatternParser_ERROR_VERBOSE
#ifdef YYERROR_VERBOSE
#define YY_PatternParser_ERROR_VERBOSE YYERROR_VERBOSE
#endif
#endif
#ifndef YY_PatternParser_LSP_NEEDED
#ifdef YYLSP_NEEDED
#define YY_PatternParser_LSP_NEEDED YYLSP_NEEDED
#endif
#endif
#endif
#ifndef YY_USE_CLASS
/* TOKEN C */

/* #line 240 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 626 "pal/patternparser.cc"
#define	IDENT	258
#define	NUM	259
#define	WILD	260
#define	IS	261
#define	NAMES	262
#define	UPTO	263
#define	PATTERNS	264
#define	TYPES	265
#define	NONE	266


#line 240 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
 /* #defines tokens */
#else
/* CLASS */
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
#ifndef YY_PatternParser_CONSTRUCTOR_CODE
#define YY_PatternParser_CONSTRUCTOR_CODE
#endif
#ifndef YY_PatternParser_CONSTRUCTOR_INIT
#define YY_PatternParser_CONSTRUCTOR_INIT
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

/* #line 284 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 685 "pal/patternparser.cc"
static const int IDENT;
static const int NUM;
static const int WILD;
static const int IS;
static const int NAMES;
static const int UPTO;
static const int PATTERNS;
static const int TYPES;
static const int NONE;


#line 284 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
 /* decl const */
#else
enum YY_PatternParser_ENUM_TOKEN { YY_PatternParser_NULL_TOKEN=0

/* #line 287 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 703 "pal/patternparser.cc"
	,IDENT=258
	,NUM=259
	,WILD=260
	,IS=261
	,NAMES=262
	,UPTO=263
	,PATTERNS=264
	,TYPES=265
	,NONE=266


#line 287 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_PatternParser_PARSE (YY_PatternParser_PARSE_PARAM);
 virtual void YY_PatternParser_ERROR(char *msg) YY_PatternParser_ERROR_BODY;
#ifdef YY_PatternParser_PURE
#ifdef YY_PatternParser_LSP_NEEDED
 virtual int  YY_PatternParser_LEX (YY_PatternParser_STYPE *YY_PatternParser_LVAL,YY_PatternParser_LTYPE *YY_PatternParser_LLOC) YY_PatternParser_LEX_BODY;
#else
 virtual int  YY_PatternParser_LEX (YY_PatternParser_STYPE *YY_PatternParser_LVAL) YY_PatternParser_LEX_BODY;
#endif
#else
 virtual int YY_PatternParser_LEX() YY_PatternParser_LEX_BODY;
 YY_PatternParser_STYPE YY_PatternParser_LVAL;
#ifdef YY_PatternParser_LSP_NEEDED
 YY_PatternParser_LTYPE YY_PatternParser_LLOC;
#endif
 int   YY_PatternParser_NERRS;
 int    YY_PatternParser_CHAR;
#endif
#if YY_PatternParser_DEBUG != 0
 int YY_PatternParser_DEBUG_FLAG;   /*  nonzero means print parse trace     */
#endif
public:
 YY_PatternParser_CLASS(YY_PatternParser_CONSTRUCTOR_PARAM);
public:
 YY_PatternParser_MEMBERS 
};
/* other declare folow */
#if YY_PatternParser_USE_CONST_TOKEN != 0

/* #line 318 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 749 "pal/patternparser.cc"
const int YY_PatternParser_CLASS::IDENT=258;
const int YY_PatternParser_CLASS::NUM=259;
const int YY_PatternParser_CLASS::WILD=260;
const int YY_PatternParser_CLASS::IS=261;
const int YY_PatternParser_CLASS::NAMES=262;
const int YY_PatternParser_CLASS::UPTO=263;
const int YY_PatternParser_CLASS::PATTERNS=264;
const int YY_PatternParser_CLASS::TYPES=265;
const int YY_PatternParser_CLASS::NONE=266;


#line 318 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
 /* const YY_PatternParser_CLASS::token */
#endif
/*apres const  */
YY_PatternParser_CLASS::YY_PatternParser_CLASS(YY_PatternParser_CONSTRUCTOR_PARAM) YY_PatternParser_CONSTRUCTOR_INIT
{
#if YY_PatternParser_DEBUG != 0
YY_PatternParser_DEBUG_FLAG=0;
#endif
YY_PatternParser_CONSTRUCTOR_CODE;
};
#endif

/* #line 329 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 775 "pal/patternparser.cc"


#define	YYFINAL		85
#define	YYFLAG		-32768
#define	YYNTBASE	27

#define YYTRANSLATE(x) ((unsigned)(x) <= 266 ? yytranslate[x] : 53)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,    16,     2,     2,     2,    20,
    21,    14,     2,    17,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    13,    18,
    15,    19,     2,    22,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    23,     2,    24,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    25,    12,    26,     2,     2,     2,     2,     2,
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
     6,     7,     8,     9,    10,    11
};

#if YY_PatternParser_DEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     2,     9,    13,    15,    18,    19,    22,    24,
    27,    28,    31,    36,    37,    43,    47,    48,    55,    58,
    60,    61,    67,    72,    74,    76,    78,    80,    82,    84,
    85,    87,    91,    93,    94,    98,   102,   106,   108,   109,
   114,   119,   123,   125,   127,   129,   131,   135,   139
};

static const short yyrhs[] = {    -1,
     0,    28,     7,    31,    29,     9,    39,     0,     3,    15,
     4,     0,    30,     0,    31,    30,     0,     0,    16,     3,
     0,     3,     0,    15,     4,     0,     0,     3,    34,     0,
    35,    17,     3,    34,     0,     0,    18,     4,     8,     4,
    19,     0,    18,     4,    19,     0,     0,     3,    33,    38,
    35,     6,    52,     0,    39,    37,     0,    37,     0,     0,
    33,    20,    41,    45,    21,     0,    33,    20,    45,    21,
     0,    44,     0,    42,     0,    32,     0,     4,     0,     3,
     0,     5,     0,     0,    43,     0,    45,    17,    43,     0,
    40,     0,     0,    22,    47,    40,     0,    23,    52,    24,
     0,    25,    52,    26,     0,     3,     0,     0,    46,    14,
    50,    36,     0,    48,    14,    50,    36,     0,    18,     4,
    19,     0,    46,     0,    49,     0,    48,     0,    51,     0,
    52,    13,    52,     0,    52,    12,    52,     0,    11,     0
};

#endif

#if YY_PatternParser_DEBUG != 0
static const short yyrline[] = { 0,
   421,   457,   461,   476,   486,   488,   489,   492,   500,   507,
   513,   519,   543,   566,   572,   586,   596,   614,   694,   701,
   707,   715,   743,   765,   773,   779,   790,   801,   810,   819,
   825,   829,   842,   848,   853,   860,   869,   879,   886,   894,
   901,   909,   919,   923,   927,   931,   935,   942,   949
};

static const char * const yytname[] = {   "$","error","$illegal.","IDENT","NUM",
"WILD","IS","NAMES","UPTO","PATTERNS","TYPES","NONE","'|'","';'","'*'","'='",
"'$'","','","'<'","'>'","'('","')'","'@'","'['","']'","'{'","'}'","pattern_spec",
"@1","@2","macro_decl","macro_decls","macro_use","opcode","default_val","operands",
"range_constraint","pattern_decl","@3","pattern_decls","constructor_application",
"@4","inner_constructor_application","argument","leaf_argument","arguments",
"simple_pattern","@5","grouped_pattern","optional_pattern","range_var","multi_match_pattern",
"pattern",""
};
#endif

static const short yyr1[] = {     0,
    28,    29,    27,    30,    31,    31,    31,    32,    33,    34,
    34,    35,    35,    35,    36,    36,    38,    37,    39,    39,
    41,    40,    42,    43,    43,    44,    44,    44,    44,    45,
    45,    45,    46,    47,    46,    48,    49,    50,    50,    51,
    51,    52,    52,    52,    52,    52,    52,    52,    52
};

static const short yyr2[] = {     0,
     0,     0,     6,     3,     1,     2,     0,     2,     1,     2,
     0,     2,     4,     0,     5,     3,     0,     6,     2,     1,
     0,     5,     4,     1,     1,     1,     1,     1,     1,     0,
     1,     3,     1,     0,     3,     3,     3,     1,     0,     4,
     4,     3,     1,     1,     1,     1,     3,     3,     1
};

static const short yydefact[] = {     1,
     0,     7,     0,     5,     2,     0,     0,     6,     4,     0,
     0,    20,     3,     9,    17,    19,    14,    11,     0,     0,
    12,     0,     0,    10,    49,     0,    34,     0,     0,     0,
    33,    43,    45,    44,    46,    18,    11,     0,     0,     0,
     0,    21,    39,    39,     0,     0,    13,    42,    35,    36,
    37,    30,    38,     0,     0,    48,    47,    28,    27,    29,
     0,    26,     0,    25,    31,    24,     0,     0,    40,    41,
     8,    30,     0,    22,     0,     0,    32,     0,    16,    23,
     0,    15,     0,     0,     0
};

static const short yydefgoto[] = {    83,
     1,     7,     4,     5,    62,    30,    21,    19,    69,    12,
    17,    13,    31,    52,    64,    65,    66,    67,    32,    39,
    33,    34,    54,    35,    36
};

static const short yypact[] = {-32768,
     3,     2,    -3,-32768,     2,    25,    22,-32768,-32768,    32,
    33,-32768,    32,-32768,-32768,-32768,    34,    24,     5,    36,
-32768,    -2,    39,-32768,-32768,    40,-32768,    -2,    -2,    23,
-32768,    31,    35,-32768,-32768,    21,    24,    27,    33,     6,
    -9,-32768,    44,    44,    -2,    -2,-32768,-32768,-32768,-32768,
-32768,    10,-32768,    30,    30,    37,-32768,    38,-32768,-32768,
    48,-32768,    43,-32768,-32768,-32768,   -15,    49,-32768,-32768,
-32768,    10,    10,-32768,    19,    11,-32768,    50,-32768,-32768,
    41,-32768,    52,    55,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,    51,-32768,-32768,   -11,    20,-32768,     4,    53,
-32768,-32768,    26,-32768,-32768,    -6,-32768,    -8,-32768,-32768,
-32768,-32768,    28,-32768,   -21
};


#define	YYLAST		72


static const short yytable[] = {    15,
    14,    73,    45,    46,     3,    74,    40,    41,    25,     2,
    22,     6,    58,    59,    60,    26,    51,    45,    46,    27,
    28,    23,    29,    56,    57,    61,    78,    73,     9,    50,
    10,    80,    45,    46,    11,    14,    18,    79,    20,    24,
    63,    37,    42,    38,    43,    48,    53,    68,    44,    46,
    71,    84,    75,    81,    85,     8,    47,    -9,    70,    82,
    63,    63,    72,    76,    49,    16,    77,     0,     0,     0,
     0,    55
};

static const short yycheck[] = {    11,
     3,    17,    12,    13,     3,    21,    28,    29,    11,     7,
     6,    15,     3,     4,     5,    18,    26,    12,    13,    22,
    23,    17,    25,    45,    46,    16,     8,    17,     4,    24,
     9,    21,    12,    13,     3,     3,     3,    19,    15,     4,
    52,     3,    20,     4,    14,    19,     3,    18,    14,    13,
     3,     0,     4,     4,     0,     5,    37,    20,    55,    19,
    72,    73,    20,    72,    39,    13,    73,    -1,    -1,    -1,
    -1,    44
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

#if YY_PatternParser_USE_GOTO != 0
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
#define yyclearin       (YY_PatternParser_CHAR = YYEMPTY)
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
  if (YY_PatternParser_CHAR == YYEMPTY && yylen == 1)                               \
    { YY_PatternParser_CHAR = (token), YY_PatternParser_LVAL = (value);                 \
      yychar1 = YYTRANSLATE (YY_PatternParser_CHAR);                                \
      YYPOPSTACK;                                               \
      YYGOTO(yybackup);                                            \
    }                                                           \
  else                                                          \
    { YY_PatternParser_ERROR ("syntax error: cannot back up"); YYERROR; }   \
while (0)

#define YYTERROR        1
#define YYERRCODE       256

#ifndef YY_PatternParser_PURE
/* UNPURE */
#define YYLEX           YY_PatternParser_LEX()
#ifndef YY_USE_CLASS
/* If nonreentrant, and not class , generate the variables here */
int     YY_PatternParser_CHAR;                      /*  the lookahead symbol        */
YY_PatternParser_STYPE      YY_PatternParser_LVAL;              /*  the semantic value of the */
				/*  lookahead symbol    */
int YY_PatternParser_NERRS;                 /*  number of parse errors so far */
#ifdef YY_PatternParser_LSP_NEEDED
YY_PatternParser_LTYPE YY_PatternParser_LLOC;   /*  location data for the lookahead     */
			/*  symbol                              */
#endif
#endif


#else
/* PURE */
#ifdef YY_PatternParser_LSP_NEEDED
#define YYLEX           YY_PatternParser_LEX(&YY_PatternParser_LVAL, &YY_PatternParser_LLOC)
#else
#define YYLEX           YY_PatternParser_LEX(&YY_PatternParser_LVAL)
#endif
#endif
#ifndef YY_USE_CLASS
#if YY_PatternParser_DEBUG != 0
int YY_PatternParser_DEBUG_FLAG;                    /*  nonzero means print parse trace     */
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
 YY_PatternParser_CLASS::
#endif
     YY_PatternParser_PARSE(YY_PatternParser_PARSE_PARAM)
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
/* parameter definition without protypes */
YY_PatternParser_PARSE_PARAM_DEF
#endif
#endif
#endif
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YY_PatternParser_STYPE *yyvsp;
  int yyerrstatus;      /*  number of tokens to shift before error messages enabled */
  int yychar1=0;          /*  lookahead token as an internal (translated) token number */

  short yyssa[YYINITDEPTH];     /*  the state stack                     */
  YY_PatternParser_STYPE yyvsa[YYINITDEPTH];        /*  the semantic value stack            */

  short *yyss = yyssa;          /*  refer to the stacks thru separate pointers */
  YY_PatternParser_STYPE *yyvs = yyvsa;     /*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YY_PatternParser_LSP_NEEDED
  YY_PatternParser_LTYPE yylsa[YYINITDEPTH];        /*  the location stack                  */
  YY_PatternParser_LTYPE *yyls = yylsa;
  YY_PatternParser_LTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YY_PatternParser_PURE
  int YY_PatternParser_CHAR;
  YY_PatternParser_STYPE YY_PatternParser_LVAL;
  int YY_PatternParser_NERRS;
#ifdef YY_PatternParser_LSP_NEEDED
  YY_PatternParser_LTYPE YY_PatternParser_LLOC;
#endif
#endif

  YY_PatternParser_STYPE yyval;             /*  the variable used to return         */
				/*  semantic values from the action     */
				/*  routines                            */

  int yylen;
/* start loop, in which YYGOTO may be used. */
YYBEGINGOTO

#if YY_PatternParser_DEBUG != 0
  if (YY_PatternParser_DEBUG_FLAG)
    fprintf(stderr, "Starting parse\n");
#endif
  yystate = 0;
  yyerrstatus = 0;
  YY_PatternParser_NERRS = 0;
  YY_PatternParser_CHAR = YYEMPTY;          /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YY_PatternParser_LSP_NEEDED
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
      YY_PatternParser_STYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YY_PatternParser_LSP_NEEDED
      YY_PatternParser_LTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YY_PatternParser_LSP_NEEDED
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
#ifdef YY_PatternParser_LSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  YY_PatternParser_ERROR("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YY_PatternParser_STYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YY_PatternParser_LSP_NEEDED
      yyls = (YY_PatternParser_LTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YY_PatternParser_LSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YY_PatternParser_DEBUG != 0
      if (YY_PatternParser_DEBUG_FLAG)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YY_PatternParser_DEBUG != 0
  if (YY_PatternParser_DEBUG_FLAG)
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

  if (YY_PatternParser_CHAR == YYEMPTY)
    {
#if YY_PatternParser_DEBUG != 0
      if (YY_PatternParser_DEBUG_FLAG)
	fprintf(stderr, "Reading a token: ");
#endif
      YY_PatternParser_CHAR = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (YY_PatternParser_CHAR <= 0)           /* This means end of input. */
    {
      yychar1 = 0;
      YY_PatternParser_CHAR = YYEOF;                /* Don't call YYLEX any more */

#if YY_PatternParser_DEBUG != 0
      if (YY_PatternParser_DEBUG_FLAG)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(YY_PatternParser_CHAR);

#if YY_PatternParser_DEBUG != 0
      if (YY_PatternParser_DEBUG_FLAG)
	{
	  fprintf (stderr, "Next token is %d (%s", YY_PatternParser_CHAR, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, YY_PatternParser_CHAR, YY_PatternParser_LVAL);
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

#if YY_PatternParser_DEBUG != 0
  if (YY_PatternParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting token %d (%s), ", YY_PatternParser_CHAR, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (YY_PatternParser_CHAR != YYEOF)
    YY_PatternParser_CHAR = YYEMPTY;

  *++yyvsp = YY_PatternParser_LVAL;
#ifdef YY_PatternParser_LSP_NEEDED
  *++yylsp = YY_PatternParser_LLOC;
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

#if YY_PatternParser_DEBUG != 0
  if (YY_PatternParser_DEBUG_FLAG)
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
#line 1397 "pal/patternparser.cc"

  switch (yyn) {

case 1:
#line 422 "pal/patternparser.y"
{
            string headerName(patFile);
            replace(headerName.begin(),headerName.end(),'.','_');

            header_os << "#ifndef __" << headerName << "_H__\n";
            header_os << "#define __" << headerName << "_H__\n";
            header_os << "class InstructionPatterns {\n";
            header_os << "private:\n";


            code_os << "/*==============================================\n";
            code_os << " * FILE:      " << patFile << ".m\n";
            code_os << " * OVERVIEW:  Generated file; do not edit\n";
            code_os << " *\n";
            code_os <<
                " * (C) 1998-2000 The University of Queensland, BT group\n";
            // Don't want to emit the Revision for *this* source file.
            // But it turns out that it's almost as much of a nuisance emitting
            // a blank line.
//          code_os << " * VERSION:   $Revis""ion:$\n";
            code_os << " *==============================================*/\n\n";
            
            code_os << "#include \"global.h\"\n";
            code_os << "#include \"decoder.h\"\n";
            code_os << "#include \"" << patFile << ".h\"\n";
            code_os << "#include \"ss.h\"\n";
            code_os << "#include \"csr.h\"\n";
            code_os << "\n";

            code_os << "#define VAR true\n";
            code_os << "#define VAL false\n";

            dict_os << "LogueDict::LogueDict()\n{\n";

        ;
    break;}
case 2:
#line 458 "pal/patternparser.y"
{
            header_os << "public:\n";
        ;
    break;}
case 3:
#line 462 "pal/patternparser.y"
{
            header_os << "private:\n";
            for (map<string,Constructor*>::iterator it =
                functionTable.begin(); it != functionTable.end(); it++) {
                it->second->genHeader(header_os);
                it->second->genCode(code_os);
            }
            header_os << "};\n#endif\n";

            code_os << *yyvsp[0].String;
            code_os << str(dict_os) << "}\n";
        ;
    break;}
case 4:
#line 478 "pal/patternparser.y"
{
            symTab->declareMacro(yyvsp[-2].str);
            code_os << "int InstructionPatterns::" << yyvsp[-2].str;
            code_os << " = " << yyvsp[0].num << ";\n";
            header_os << "\tstatic int " << yyvsp[-2].str << ";\n";
        ;
    break;}
case 8:
#line 494 "pal/patternparser.y"
{
            symTab->useMacro(yyvsp[0].str);
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 9:
#line 502 "pal/patternparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 10:
#line 509 "pal/patternparser.y"
{
            yyval.numPtr = new int;
            *yyval.numPtr = yyvsp[0].num;
        ;
    break;}
case 11:
#line 514 "pal/patternparser.y"
{
            yyval.numPtr = NULL;
        ;
    break;}
case 12:
#line 521 "pal/patternparser.y"
{
            // Add this operand to the current local scope
            symTab->declareParam(yyvsp[-1].str);

            // Add operand to the list of operands
            yyval.List_string = new list<string>;
            yyval.List_string->push_back(yyvsp[-1].str);

            // Record the default value if there is one
            if (yyvsp[0].numPtr != NULL) {
                symTab->setParamDefault(yyvsp[-1].str,*yyvsp[0].numPtr);
                delete yyvsp[0].numPtr;
            }

            // Generate code segment for constructing this parameter
            dict_os << "\t\tparams.push_back(\"" << yyvsp[-1].str << "\");\n";

            // Add an entry for this param to the semantic table as it
            // will be required later on
            dict_os << "\t\ttheSemTable.addItem(\"" << yyvsp[-1].str << "\");\n";
        ;
    break;}
case 13:
#line 544 "pal/patternparser.y"
{
            // Add this operand to the current local scope
            symTab->declareParam(yyvsp[-1].str);

            // Add operand to the list of operands
            yyval.List_string = yyvsp[-3].List_string;
            yyval.List_string->push_back(yyvsp[-1].str);

            // Record the default value if there is one
            if (yyvsp[0].numPtr != NULL) {
                symTab->setParamDefault(yyvsp[-1].str,*yyvsp[0].numPtr);
                delete yyvsp[0].numPtr;
            }

            // Generate code segment for constructing this parameter
            // Generate code segment for constructing this parameter
            dict_os << "\t\tparams.push_back(\"" << yyvsp[-1].str << "\");\n";

            // Add an entry for this param to the semantic table as it
            // will be required later on
            dict_os << "\t\ttheSemTable.addItem(\"" << yyvsp[-1].str << "\");\n";
        ;
    break;}
case 14:
#line 567 "pal/patternparser.y"
{
            yyval.List_string = NULL;
        ;
    break;}
case 15:
#line 574 "pal/patternparser.y"
{
            if (yyvsp[-3].num < 1)
                yyerror("cardinality of repetitive match must be >= 1");
            if (yyvsp[-1].num <= yyvsp[-3].num)
                yyerror("upper limit must be greater than lower limit");

            yyval.String = new string(" >= ");
            (*yyval.String)  = (*yyval.String) + yyvsp[-3].num;
            (*yyval.String) += " && ";
            (*yyval.String)  = (*yyval.String) + yyvsp[-1].num;
            (*yyval.String) += " >= ";
        ;
    break;}
case 16:
#line 587 "pal/patternparser.y"
{
            if (yyvsp[-1].num < 1)
                yyerror("cardinality of repetitive match must be >= 1");

            yyval.String = new string(" == ");
            (*yyval.String) = (*yyval.String) + yyvsp[-1].num;
        ;
    break;}
case 17:
#line 598 "pal/patternparser.y"
{
            if (typeNames.find(yyvsp[-1].str) == typeNames.end()) {
                ostrstream ost;
                ost << "`" << yyvsp[-1].str << "' is not a valid pattern type name";
                yyerror(str(ost));
            }

            // Clear the local scope
            symTab->clear();
            symTab->newAnonLocals();

            // Start the code segment for constructing an entry for
            // this logue in the logueDict
            dict_os << "\t{\n\t\tlist<string> params;\n";
        ;
    break;}
case 18:
#line 615 "pal/patternparser.y"
{
            // Generate the signature from the opcode and operands as
            // well as the code to build the params for instantiation
            string sig(yyvsp[-4].str);
            sig += string("(CSR& csr, ADDRESS& lc");

            string params("vector<int> params(");
            params = params + (yyvsp[-2].List_string == NULL ? 0 : yyvsp[-2].List_string->size());
            params += "); ";

            if (yyvsp[-2].List_string != NULL) {
                int idx = 0;
                for (list<string>::iterator it = yyvsp[-2].List_string->begin(); it !=yyvsp[-2].List_string->end();
                    it++, idx++)
                {
                    sig += ", int& ";
                    sig += *it;

                    params += "params[";
                    params = params + idx;
                    params += "] = ";
                    params += *it + "; ";
                }
            }
            sig += ")";

#if 0   // Don't like Doug's assumption here; want to have patterns with
        // optional parts, and if they are all not present, then that's not
        // this pattern. Example: Pentium frameless_pro
            // Use an assert to match the pattern wasn't matched
            // trivially
            params += "\n\t\tassert(__save != lc);\n";
#else
            // If the pattern matched trivially, consider it a failure
            params += "\n\t\tif (__save == lc) return NULL;\n";
#endif

            header_os << "\tstatic Logue* " << sig << ";\n";

            // Complete the code segment for constructing an entry for
            // this logue in the logueDict
            dict_os << "\t\tthis->newLogue(\"" << yyvsp[-4].str << "\",\"";
            dict_os << yyvsp[-5].str << "\",params);\n\t}\n";

            // Generate the function prologue including the signature
            yyval.String = new string(sig);
            yyval.String->insert(0,string("Logue* InstructionPatterns::"));
            (*yyval.String) += "\n{\n\tADDRESS __save = lc;\n";
            if( needLoop ) {
                (*yyval.String) += "\tint __loop;\n";
                needLoop = false;
            }

            // Generate the anonymous locals declarations
            symTab->resetAnonLocals();
            for (int i = 0; i < symTab->numAnonLocals(); i++)
                (*yyval.String) += string("\tint ") + symTab->nextAnonLocal() + ";\n";

            // Generate the assignment of default values
            map<string,int> defaults = symTab->getAndClearDefaults();
            for (map<string,int>::iterator it = defaults.begin();
                it != defaults.end(); it++) {

                (*yyval.String) += string("\t") + it->first + " = ";
                (*yyval.String)  = (*yyval.String) + it->second;
                (*yyval.String) += ";\n";
            }

            (*yyval.String) += "\tif (\n";
            (*yyval.String) += *yyvsp[0].String + ") {\n\t\t";
            (*yyval.String) += params + "\t\treturn csr.instantiateLogue(\"" + yyvsp[-4].str;
            (*yyval.String) += "\",params);\n\t} else {\n";
            (*yyval.String) += "\t\tlc = __save;\n\t\treturn NULL;\n\t}\n}\n";

            delete(yyvsp[-2].List_string);
            delete(yyvsp[0].String);
        ;
    break;}
case 19:
#line 696 "pal/patternparser.y"
{
            yyval.String = yyvsp[-1].String;
            (*yyval.String) += *yyvsp[0].String;
            delete(yyvsp[0].String);
        ;
    break;}
case 20:
#line 702 "pal/patternparser.y"
{
                yyval.String = yyvsp[0].String;
        ;
    break;}
case 21:
#line 709 "pal/patternparser.y"
{
            symTab->resetAnonLocals();

            functionCall.erase(0,functionCall.size());
            innerCons.clear();
        ;
    break;}
case 22:
#line 716 "pal/patternparser.y"
{
            yyval.String = new string(functionCall);
            yyval.String->insert(0,yyvsp[-4].str);

            replace(yyval.String->begin(),yyval.String->end(),'.','$');

            if (emit_sig) {
                Constructor*& fn =
                    functionTable[*yyval.String];

                if (fn == NULL) {
                    fn = new OuterConstructor(*yyval.String,yyvsp[-4].str,yyvsp[-1].arg->arg);
                }
            }

            (*yyval.String) += (emit_sig ? "(lc" : "(csr, lc");
            if (yyvsp[-1].arg->arg != NULL && yyvsp[-1].arg->arg->count() > 0) {
                (*yyval.String) += ", ";
                (*yyval.String) += (yyvsp[-1].arg->inst);
            }
            (*yyval.String) += ")";

            delete(yyvsp[-1].arg);

        ;
    break;}
case 23:
#line 745 "pal/patternparser.y"
{
            InnerConstructor* ffn = NULL;
            functionCall.insert(0,yyvsp[-3].str);
            
            Constructor* &fn = functionTable[functionCall];
            ffn = (InnerConstructor*)fn;

            if (fn == NULL) {
                ffn = new InnerConstructor(functionCall, yyvsp[-3].str, yyvsp[-1].arg->arg);
                fn = ffn;
            }
            yyval.arg = yyvsp[-1].arg;
			//cout << $1 << " (" << $3->inst << ") " << endl << flush;
            
            yyval.arg->arg = new ConstrWrap(ffn);
        
            functionCall.insert(0,"$");
        ;
    break;}
case 24:
#line 767 "pal/patternparser.y"
{
            yyval.arg = new arg_pair;
            yyval.arg->inst = *yyvsp[0].String;
            delete yyvsp[0].String;
            yyval.arg->arg = new ValueWrap();
        ;
    break;}
case 25:
#line 774 "pal/patternparser.y"
{
            yyval.arg = yyvsp[0].arg;
        ;
    break;}
case 26:
#line 781 "pal/patternparser.y"
{
            string result;
            symTab->useMacro(yyvsp[0].str);
            
            yyval.String = new string(yyvsp[0].str);

            if (emit_sig)
                (*yyval.String) += (", VAL");
        ;
    break;}
case 27:
#line 791 "pal/patternparser.y"
{
            yyval.String = new string(symTab->nextAnonLocal());

            (*yyval.String) += " = ";
            (*yyval.String)  = (*yyval.String) + yyvsp[0].num;

            if (emit_sig)
                (*yyval.String) += ", VAL";
            
        ;
    break;}
case 28:
#line 802 "pal/patternparser.y"
{
            symTab->useParamOrLocal(yyvsp[0].str);

            yyval.String = new string(yyvsp[0].str);

            if (emit_sig)
                (*yyval.String) += ", VAR";
        ;
    break;}
case 29:
#line 811 "pal/patternparser.y"
{
            yyval.String = new string(symTab->nextAnonLocal());

            if (emit_sig)
                (*yyval.String) += ", VAR";
        ;
    break;}
case 30:
#line 820 "pal/patternparser.y"
{
            yyval.arg = new arg_pair;
            yyval.arg->arg = NULL;
            yyval.arg->inst = "";
        ;
    break;}
case 31:
#line 826 "pal/patternparser.y"
{
            yyval.arg = yyvsp[0].arg;
        ;
    break;}
case 32:
#line 830 "pal/patternparser.y"
{
            yyval.arg = yyvsp[-2].arg;
			if ((yyval.arg->arg != NULL) && (yyval.arg->arg->count() > 0)
			 && (yyvsp[0].arg->arg != NULL) && (yyvsp[0].arg->arg->count() > 0))
				yyval.arg->inst += ", ";
			yyval.arg->inst += yyvsp[0].arg->inst; 
            if (yyval.arg->arg != NULL) yyval.arg->arg->push_back(yyvsp[0].arg->arg);
			else (yyval.arg->arg = yyvsp[0].arg->arg);
            delete(yyvsp[0].arg);
        ;
    break;}
case 33:
#line 844 "pal/patternparser.y"
{
            yyval.String = yyvsp[0].String;
            yyval.String->insert(0,"\t");
        ;
    break;}
case 34:
#line 849 "pal/patternparser.y"
{
            emit_sig = false;
        ;
    break;}
case 35:
#line 853 "pal/patternparser.y"
{
            yyval.String = yyvsp[0].String;
            yyval.String->insert(0,"\t");
            emit_sig = true;
        ;
    break;}
case 36:
#line 862 "pal/patternparser.y"
{
            yyval.String = yyvsp[-1].String;
            yyval.String->insert(yyval.String->find_first_not_of('\t'),"(");
            (*yyval.String) += ")";
        ;
    break;}
case 37:
#line 871 "pal/patternparser.y"
{
            yyval.String = yyvsp[-1].String;

            yyval.String->insert(yyval.String->find_first_not_of('\t'),"((");
            (*yyval.String) += ") || true)";
        ;
    break;}
case 38:
#line 881 "pal/patternparser.y"
{
            symTab->useParamOrLocal(yyvsp[0].str);

            yyval.String = new string(yyvsp[0].str);
        ;
    break;}
case 39:
#line 887 "pal/patternparser.y"
{
            yyval.String = new string("__loop");
            needLoop = true;
        ;
    break;}
case 40:
#line 896 "pal/patternparser.y"
{
            yyval.String = buildIterExpr(yyvsp[-3].String,yyvsp[-1].String,yyvsp[0].String);
            delete(yyvsp[-1].String);
            delete(yyvsp[0].String);
        ;
    break;}
case 41:
#line 902 "pal/patternparser.y"
{
            yyval.String = buildIterExpr(yyvsp[-3].String,yyvsp[-1].String,yyvsp[0].String);
            delete(yyvsp[-1].String);
            delete(yyvsp[0].String);
        ;
    break;}
case 42:
#line 911 "pal/patternparser.y"
{
            if (yyvsp[-1].num < 0)
                yyerror("the size must be positive");

            yyval.String = new string("\t(lc += ");
            (*yyval.String) = (*yyval.String) + yyvsp[-1].num;
            (*yyval.String) += ", true)";
        ;
    break;}
case 43:
#line 920 "pal/patternparser.y"
{
            yyval.String = yyvsp[0].String;
        ;
    break;}
case 44:
#line 924 "pal/patternparser.y"
{
            yyval.String = yyvsp[0].String;
        ;
    break;}
case 45:
#line 928 "pal/patternparser.y"
{
            yyval.String = yyvsp[0].String;
        ;
    break;}
case 46:
#line 932 "pal/patternparser.y"
{
            yyval.String = yyvsp[0].String;
        ;
    break;}
case 47:
#line 936 "pal/patternparser.y"
{
            yyval.String = yyvsp[-2].String;
            (*yyval.String) += " && \n";
            (*yyval.String) += (*yyvsp[0].String);
            delete (yyvsp[0].String);
        ;
    break;}
case 48:
#line 943 "pal/patternparser.y"
{
            yyval.String = yyvsp[-2].String;
            (*yyval.String) += " || \n";
            (*yyval.String) += (*yyvsp[0].String);
            delete (yyvsp[0].String);
        ;
    break;}
case 49:
#line 950 "pal/patternparser.y"
{
            // End up with if(false) {instantiate this Logur} else {return NULL}
            yyval.String = new string("false");
        ;
    break;}
}

#line 783 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
   /* the action file gets copied in in place of this dollarsign  */
  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YY_PatternParser_LSP_NEEDED
  yylsp -= yylen;
#endif

#if YY_PatternParser_DEBUG != 0
  if (YY_PatternParser_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YY_PatternParser_LSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = YY_PatternParser_LLOC.first_line;
      yylsp->first_column = YY_PatternParser_LLOC.first_column;
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
      ++YY_PatternParser_NERRS;

#ifdef YY_PatternParser_ERROR_VERBOSE
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
	      YY_PatternParser_ERROR(msg);
	      free(msg);
	    }
	  else
	    YY_PatternParser_ERROR ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YY_PatternParser_ERROR_VERBOSE */
	YY_PatternParser_ERROR("parse error");
    }

  YYGOTO(yyerrlab1);
YYLABEL(yyerrlab1)   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (YY_PatternParser_CHAR == YYEOF)
	YYABORT;

#if YY_PatternParser_DEBUG != 0
      if (YY_PatternParser_DEBUG_FLAG)
	fprintf(stderr, "Discarding token %d (%s).\n", YY_PatternParser_CHAR, yytname[yychar1]);
#endif

      YY_PatternParser_CHAR = YYEMPTY;
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
#ifdef YY_PatternParser_LSP_NEEDED
  yylsp--;
#endif

#if YY_PatternParser_DEBUG != 0
  if (YY_PatternParser_DEBUG_FLAG)
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

#if YY_PatternParser_DEBUG != 0
  if (YY_PatternParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = YY_PatternParser_LVAL;
#ifdef YY_PatternParser_LSP_NEEDED
  *++yylsp = YY_PatternParser_LLOC;
#endif

  yystate = yyn;
  YYGOTO(yynewstate);
/* end loop, in which YYGOTO may be used. */
  YYENDGOTO
}

/* END */

/* #line 982 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 2165 "pal/patternparser.cc"
#line 956 "pal/patternparser.y"


int PatternParser::yylex()
{
    int token = theScanner->yylex(yylval);
    return token;
}

void PatternParser::yyerror(char* msg)
{
    cerr << patFile << ":" << theScanner->theLine << ": " << msg << endl;
    exit(1);
}

// Build an iteration expression. This will have the form:
//   for (var = 0; <matches>; var++);
//   return (var <range> var);
// MVE: Added the "return" above; seems needed!
string* PatternParser::buildIterExpr(string* pattn, string* var, string* range)
{
    assert(pattn != NULL);
    assert(var != NULL);
    assert(range != NULL);

    string pre = string("{ for(") + *var + string(" = 0; ");
    pattn->insert(pattn->find_first_not_of('\t'),pre);
    (*pattn) += string("; ") + *var + string("++); ");
    //(*pattn) += *var + *range + *var + string("; }");
    (*pattn) += "return (" + *var + *range + *var + string("); }");

    char tmp[100];
    sprintf(tmp,"iterhlp%i",iterhlps++);
    
    code_os << "int InstructionPatterns::" << tmp << "(ADDRESS& lc, int& " <<
        *var;
    for (int i=0; i<symTab->numAnonLocals(); i++)
        code_os << ", int& __loc" << i;
    code_os << ") {" << endl;
    code_os << *pattn << endl;
    code_os << "return 0;" << endl;
    code_os << "}" << endl << endl;

    header_os << "static int " << tmp << "(ADDRESS& lc, int& " << *var;
    for (int k=0; k<symTab->numAnonLocals(); k++)
          header_os << ", int& __loc" << k;
        header_os << ");" << endl;

    *pattn = string(tmp) + string("(lc,") + *var;
    for (int j=0; j<symTab->numAnonLocals(); j++) {
      char tmp1[20];
      sprintf(tmp1,",__loc%i",j);
          *pattn += string(tmp1);
        }
    *pattn += ")";

    return pattn;
}

PatternParser::~PatternParser()
{
    if (theScanner)
        delete(theScanner);
    if (symTab)
        delete(symTab);
}

SymbolTable::SymbolTable() : anonLocals(0), maxAnonLocals(0)
{}

void SymbolTable::declareParam(const string& name)
{
    if (paramSyms.find(name) != paramSyms.end()) {
        cerr << "redefinition of parameter `" << name << "'\n";
        exit(1);
    }
    paramSyms.insert(name);
}

void SymbolTable::declareMacro(const string& name)
{
    if (macroSyms.find(name) != macroSyms.end()) {
        cerr << "redefinition of macro `" << name << "'\n";
        exit(1);
    }
    macroSyms.insert(name);
}


void SymbolTable::setParamDefault(const string& name, int val)
{
    defaults[name] = val;
}

map<string,int> SymbolTable::getAndClearDefaults()
{
    map<string,int> ret(defaults);
    defaults.clear();
    return ret;
}

void SymbolTable::useMacro(const string& name)
{
    set<string>::iterator it = macroSyms.find(name);
    if (it == macroSyms.end()) {
        cerr << "macro `" << name << "' undeclared\n";
        exit(1);
    }
}

void SymbolTable::useParamOrLocal(const string& name)
{
    set<string>::iterator it = paramSyms.find(name);
    if (it != paramSyms.end()) {
        return;
    }
    else
        localSyms.insert(name);
}

void SymbolTable::clear()
{
    localSyms.clear();
    paramSyms.clear();
}

string SymbolTable::nextAnonLocal()
{
    string ret("__loc");
    anonLocals ++;
    maxAnonLocals = (anonLocals > maxAnonLocals ? anonLocals : maxAnonLocals);
    return ret + (anonLocals-1);
}

void SymbolTable::newAnonLocals()
{
    maxAnonLocals = 0;
}

void SymbolTable::resetAnonLocals()
{
    anonLocals = 0;
}

int SymbolTable::numAnonLocals()
{
    return maxAnonLocals;
}

list<InnerConstructor*>* ArgWrap::eval_v(void) {
    list<InnerConstructor*>* v;
    if (next != NULL) v = next->eval_v(); else v = new list<InnerConstructor*>;
    v->push_front(as_constr());
    return v;
}

string Constructor::getParams(int numArgs) const
{
    string ret("(ADDRESS& lc");
    for (int i = 0; i < numArgs; i++) {

        char arg = 'a' + i;
        ret += string(", int& ") + arg;
        ret += string(", bool ");
        ret += arg;
        ret += string("_isVAR");
    }
    ret += ")";
    return ret;
}

void InnerConstructor::genCode(ostream& os) const
{
    os << "bool InstructionPatterns::" << name << getParams(getNumParams());
    os << " {\n";

    os << "\tmatch lc to\n\t| " << constructor << "(";
    int matchIdx, paramIdx = 0;
    list<InnerConstructor*>* params;
    if (arguments != NULL) params = arguments->eval_v();
    else params = new list<InnerConstructor*>;
    for (list<InnerConstructor*>::const_iterator it = params->begin();
            it != params->end();
            it++) {
        os << (it != params->begin() ? ", " : "") << "_";
        os << (char)('a' + paramIdx);
        paramIdx++;
    }
    os << ") =>\n"; 
    
    paramIdx = 0;
	matchIdx = 0;
    for (list<InnerConstructor*>::const_iterator it = params->begin();
        it != params->end(); it++) {

        if (*it == NULL) {
            char param = 'a' + paramIdx;
			char mparam = 'a' + matchIdx;
            os << "\t\tif (!" << param << "_isVAR && (int)_" << mparam;
            os << " != " << param;
            os << ") return false; else " << param << " = _";
            os << mparam << ";\n";
            paramIdx++;
        }
        else {
            os << "\t\tif (!" << (*it)->getMatchCall(matchIdx, paramIdx);
            os << ") return false;\n";
			paramIdx += (*it)->getNumParams();
        }
		matchIdx++;
    }
    delete params;
    os << "\t\treturn true;\n\telse\n\t\treturn false;\n\tendmatch\n}\n";
 
}

void InnerConstructor::genHeader(ostream& os) const
{
    os << "\tstatic bool " << name << getParams(getNumParams()) << ";\n";
}

string InnerConstructor::getMatchCall(int matchoffset, int offset) const
{
    string ret(name);
    ret += string("(_") + (char)('a' + matchoffset);
    //list<InnerConstructor*>* params;
    //if (arguments != NULL) params = arguments->eval_v();
    //else params = new list<InnerConstructor*>;
    //list<InnerConstructor*>::const_iterator i;
    char param; int i;
    for (param = 'a' + offset, i = 0/*params->begin()*/; i != getNumParams()/*params->end()*/; param++, i++) {
        // for a param to be null it means it's not an inner_constructor_application
        //if (*i == NULL) {
            ret += string(", ") + param;
            ret += string(", ") + param;
            ret += string("_isVAR");
        //}
    }
    //delete params;
    return ret + ")";
}

int InnerConstructor::getNumParams() const
{
    return numParams;
}

void OuterConstructor::genCode(ostream& os) const
{ 
    os << "bool InstructionPatterns::" << name << getParams(getNumParams());
    os << " {\n\tADDRESS nextPC;\n";

    os << "\tmatch [nextPC] lc to\n\t| " << constructor << "(";
    int matchIdx, paramIdx = 0;
    list<InnerConstructor*>* params;
    if (arguments != NULL) params = arguments->eval_v();
    else params = new list<InnerConstructor*>;
    for (list<InnerConstructor*>::const_iterator it = params->begin();
            it != params->end();
            it++) {
        os << (it != params->begin() ? ", " : "") << "_";
        os << (char)('a' + paramIdx);
   		paramIdx++;
    }
    os << ") =>\n"; 
    
    paramIdx = 0;
	matchIdx = 0;
    for (list<InnerConstructor*>::const_iterator it = params->begin();
        it != params->end(); it++) {

        if (*it == NULL) {
            char param = 'a' + paramIdx;
			char mparam = 'a' + matchIdx;
            os << "\t\tif (!" << param << "_isVAR && (int)_" << mparam;
            os << " != " << param;
            os << ") return false; else " << param << " = _";
            os << mparam << ";\n";
            paramIdx++;
        }
        else {
            os << "\t\tif (!" << (*it)->getMatchCall(matchIdx, paramIdx);
            os << ") return false;\n";
            paramIdx += (*it)->getNumParams();
        }
		matchIdx++;
    }
    delete params;
    os << "\t\tlc = nextPC;\n";
    os << "\t\treturn true;\n\telse\n\t\treturn false;\n\tendmatch\n}\n";
}

void OuterConstructor::genHeader(ostream& os) const
{
    os << "\tstatic bool " << name << getParams(getNumParams()) << ";\n";
}


