/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:          patternparser.y
 * OVERVIEW:      Defines a parser for a Call Signature Recovery (CSR)
 *                specification; what is now known as the Procedural 
 *                Abstraction Language (PAL) specification.
 *
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.10 $
 *
 * 04 Aug 00 - Mike: match statements are now match [nextPC] inst of [numBytes]
 * 07 Aug 00 - Mike: Added header with dollar Revision to the generated .m file
 * 29 Mar 01 - Mike: Added return statement around the range statement gener-
 *              ated by buildIterExpr (builds iterhlp%d function)
 * 02 Apr 01 - Mike: If a pattern matches trivially, this is not longer an
 *              assert, instead the pattern merely fails. Need this for example
 *              for the Pentium frameless_pro callee prologue; untabbed
 * 06 May 01 - Simon: Many changes to accomodate constructor nesting more than
 *              1 level deep (needed for HP ra-risc, at least)
 * 07 May 01 - Simon: Some changes to get BV() working; was duplicate formal
 *              parameter _a, and an extra comma emitted
 * 10 May 01 - Mike: Don't emit the Revision for this file to the generated
 * 11 May 01 - Nathan: Don't emit __loop when it's not used
 */

%name PatternParser

%define DEBUG 1
%define DEBUG_INIT 1

// These macros can only be defined if DEBUG != 0
%define ERROR_VERBOSE

// stuff to go in patternparser.h
%header{
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

%}

/*==============================================================================
 * Declaration of semantic value type
 *============================================================================*/
%union{
    char*           str;
    int*            numPtr;
    string*         String;
    int             num;
    list<string>*   List_string;
    arg_pair*        arg;
    list<InnerConstructor*>* argList;
}

// stuff to go in patternparser.cc AFTER the declaration of the
// semantic value type (i.e. the %union declaration)
%{
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

%}

/*==============================================================================
 * Declaration of PatternParser class members.
 *============================================================================*/
%define MEMBERS \
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

/*==============================================================================
 * Declaration of PatternParser constructor.
 *============================================================================*/
%define CONSTRUCTOR_PARAM \
    const string& fileName, \
    const set<string>& types, \
    bool trace = false
%define CONSTRUCTOR_INIT : \
    patFile(fileName), typeNames(types), emit_sig(true)\

%define CONSTRUCTOR_CODE \
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

/*==============================================================================
 * Declaration of token types, associativity and precedence
 *============================================================================*/

%token <str> IDENT
%token <num> NUM 
%token       WILD IS NAMES UPTO PATTERNS TYPES NONE

%type <numPtr>  default_val
%type <str>     macro_use opcode
%type <arg>     arguments inner_constructor_application argument 
%type <String>  constructor_application simple_pattern grouped_pattern leaf_argument
%type <String>  optional_pattern multi_match_pattern pattern
%type <String>  range_var range_constraint pattern_decl pattern_decls

%type <List_string> operands

%left '|'
%left ';'
%left '*'

%%
/*==============================================================================
 * Productions and actions
 *============================================================================*/

pattern_spec:
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

        }
        NAMES macro_decls
        {
            header_os << "public:\n";
        }
        PATTERNS pattern_decls
        {
            header_os << "private:\n";
            for (map<string,Constructor*>::iterator it =
                functionTable.begin(); it != functionTable.end(); it++) {
                it->second->genHeader(header_os);
                it->second->genCode(code_os);
            }
            header_os << "};\n#endif\n";

            code_os << *$6;
            code_os << str(dict_os) << "}\n";
        }
    ;

macro_decl:
        IDENT '=' NUM
        {
            symTab->declareMacro($1);
            code_os << "int InstructionPatterns::" << $1;
            code_os << " = " << $3 << ";\n";
            header_os << "\tstatic int " << $1 << ";\n";
        }
    ;

macro_decls:
        macro_decl
    |   macro_decls macro_decl
    |
    ;

macro_use:
        '$' IDENT
        {
            symTab->useMacro($2);
            $$ = $2;
        }
    ;

opcode:
        IDENT
        {
            $$ = $1;
        }
    ; 

default_val:
        '=' NUM
        {
            $$ = new int;
            *$$ = $2;
        }
    |
        {
            $$ = NULL;
        }
    ;

operands:
        IDENT default_val
        {
            // Add this operand to the current local scope
            symTab->declareParam($1);

            // Add operand to the list of operands
            $$ = new list<string>;
            $$->push_back($1);

            // Record the default value if there is one
            if ($2 != NULL) {
                symTab->setParamDefault($1,*$2);
                delete $2;
            }

            // Generate code segment for constructing this parameter
            dict_os << "\t\tparams.push_back(\"" << $1 << "\");\n";

            // Add an entry for this param to the semantic table as it
            // will be required later on
            dict_os << "\t\ttheSemTable.addItem(\"" << $1 << "\");\n";
        }
        
    |   operands ',' IDENT default_val
        {
            // Add this operand to the current local scope
            symTab->declareParam($3);

            // Add operand to the list of operands
            $$ = $1;
            $$->push_back($3);

            // Record the default value if there is one
            if ($4 != NULL) {
                symTab->setParamDefault($3,*$4);
                delete $4;
            }

            // Generate code segment for constructing this parameter
            // Generate code segment for constructing this parameter
            dict_os << "\t\tparams.push_back(\"" << $3 << "\");\n";

            // Add an entry for this param to the semantic table as it
            // will be required later on
            dict_os << "\t\ttheSemTable.addItem(\"" << $3 << "\");\n";
        }
    |
        {
            $$ = NULL;
        }
    ;

range_constraint:
        '<' NUM UPTO NUM '>'
        {
            if ($2 < 1)
                yyerror("cardinality of repetitive match must be >= 1");
            if ($4 <= $2)
                yyerror("upper limit must be greater than lower limit");

            $$ = new string(" >= ");
            (*$$)  = (*$$) + $2;
            (*$$) += " && ";
            (*$$)  = (*$$) + $4;
            (*$$) += " >= ";
        }
    |   '<' NUM '>'
        {
            if ($2 < 1)
                yyerror("cardinality of repetitive match must be >= 1");

            $$ = new string(" == ");
            (*$$) = (*$$) + $2;
        }
    ;

pattern_decl:
        IDENT opcode
        {
            if (typeNames.find($1) == typeNames.end()) {
                ostrstream ost;
                ost << "`" << $1 << "' is not a valid pattern type name";
                yyerror(str(ost));
            }

            // Clear the local scope
            symTab->clear();
            symTab->newAnonLocals();

            // Start the code segment for constructing an entry for
            // this logue in the logueDict
            dict_os << "\t{\n\t\tlist<string> params;\n";
        }

        operands IS pattern
        {
            // Generate the signature from the opcode and operands as
            // well as the code to build the params for instantiation
            string sig($2);
            sig += string("(CSR& csr, ADDRESS& lc");

            string params("vector<int> params(");
            params = params + ($4 == NULL ? 0 : $4->size());
            params += "); ";

            if ($4 != NULL) {
                int idx = 0;
                for (list<string>::iterator it = $4->begin(); it !=$4->end();
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
            dict_os << "\t\tthis->newLogue(\"" << $2 << "\",\"";
            dict_os << $1 << "\",params);\n\t}\n";

            // Generate the function prologue including the signature
            $$ = new string(sig);
            $$->insert(0,string("Logue* InstructionPatterns::"));
            (*$$) += "\n{\n\tADDRESS __save = lc;\n";
            if( needLoop ) {
                (*$$) += "\tint __loop;\n";
                needLoop = false;
            }

            // Generate the anonymous locals declarations
            symTab->resetAnonLocals();
            for (int i = 0; i < symTab->numAnonLocals(); i++)
                (*$$) += string("\tint ") + symTab->nextAnonLocal() + ";\n";

            // Generate the assignment of default values
            map<string,int> defaults = symTab->getAndClearDefaults();
            for (map<string,int>::iterator it = defaults.begin();
                it != defaults.end(); it++) {

                (*$$) += string("\t") + it->first + " = ";
                (*$$)  = (*$$) + it->second;
                (*$$) += ";\n";
            }

            (*$$) += "\tif (\n";
            (*$$) += *$6 + ") {\n\t\t";
            (*$$) += params + "\t\treturn csr.instantiateLogue(\"" + $2;
            (*$$) += "\",params);\n\t} else {\n";
            (*$$) += "\t\tlc = __save;\n\t\treturn NULL;\n\t}\n}\n";

            delete($4);
            delete($6);
        }
    ;

pattern_decls:
        pattern_decls pattern_decl
        {
            $$ = $1;
            (*$$) += *$2;
            delete($2);
        }
    |   pattern_decl
        {
                $$ = $1;
        }
    ;

constructor_application:
        opcode '('
        {
            symTab->resetAnonLocals();

            functionCall.erase(0,functionCall.size());
            innerCons.clear();
        }
        arguments ')'
        {
            $$ = new string(functionCall);
            $$->insert(0,$1);

            replace($$->begin(),$$->end(),'.','$');

            if (emit_sig) {
                Constructor*& fn =
                    functionTable[*$$];

                if (fn == NULL) {
                    fn = new OuterConstructor(*$$,$1,$4->arg);
                }
            }

            (*$$) += (emit_sig ? "(lc" : "(csr, lc");
            if ($4->arg != NULL && $4->arg->count() > 0) {
                (*$$) += ", ";
                (*$$) += ($4->inst);
            }
            (*$$) += ")";

            delete($4);

        }
    ;

inner_constructor_application:
        opcode '(' arguments ')'
        {
            InnerConstructor* ffn = NULL;
            functionCall.insert(0,$1);
            
            Constructor* &fn = functionTable[functionCall];
            ffn = (InnerConstructor*)fn;

            if (fn == NULL) {
                ffn = new InnerConstructor(functionCall, $1, $3->arg);
                fn = ffn;
            }
            $$ = $3;
			//cout << $1 << " (" << $3->inst << ") " << endl << flush;
            
            $$->arg = new ConstrWrap(ffn);
        
            functionCall.insert(0,"$");
        }
    ;

argument:
        leaf_argument
        {
            $$ = new arg_pair;
            $$->inst = *$1;
            delete $1;
            $$->arg = new ValueWrap();
        }
    |   inner_constructor_application
        {
            $$ = $1;
        }
    ;

leaf_argument:
        macro_use
        {
            string result;
            symTab->useMacro($1);
            
            $$ = new string($1);

            if (emit_sig)
                (*$$) += (", VAL");
        }
    |   NUM
        {
            $$ = new string(symTab->nextAnonLocal());

            (*$$) += " = ";
            (*$$)  = (*$$) + $1;

            if (emit_sig)
                (*$$) += ", VAL";
            
        }
    |   IDENT
        {
            symTab->useParamOrLocal($1);

            $$ = new string($1);

            if (emit_sig)
                (*$$) += ", VAR";
        }
    |   WILD
        {
            $$ = new string(symTab->nextAnonLocal());

            if (emit_sig)
                (*$$) += ", VAR";
        }
    ;

arguments:
        {
            $$ = new arg_pair;
            $$->arg = NULL;
            $$->inst = "";
        }
    |   argument
        {
            $$ = $1;
        }
    |   arguments ',' argument
        {
            $$ = $1;
			if (($$->arg != NULL) && ($$->arg->count() > 0)
			 && ($3->arg != NULL) && ($3->arg->count() > 0))
				$$->inst += ", ";
			$$->inst += $3->inst; 
            if ($$->arg != NULL) $$->arg->push_back($3->arg);
			else ($$->arg = $3->arg);
            delete($3);
        }
    ;

simple_pattern:
        constructor_application
        {
            $$ = $1;
            $$->insert(0,"\t");
        }
    |   '@'
        {
            emit_sig = false;
        }
        constructor_application
        {
            $$ = $3;
            $$->insert(0,"\t");
            emit_sig = true;
        }
    ;

grouped_pattern:
        '[' pattern ']'
        {
            $$ = $2;
            $$->insert($$->find_first_not_of('\t'),"(");
            (*$$) += ")";
        }
    ;

optional_pattern:
        '{' pattern '}'
        {
            $$ = $2;

            $$->insert($$->find_first_not_of('\t'),"((");
            (*$$) += ") || true)";
        }
    ;

range_var:
        IDENT
        {
            symTab->useParamOrLocal($1);

            $$ = new string($1);
        }
    |
        {
            $$ = new string("__loop");
            needLoop = true;
        }
    ;


multi_match_pattern:
        simple_pattern '*' range_var range_constraint
        {
            $$ = buildIterExpr($1,$3,$4);
            delete($3);
            delete($4);
        }
    |   grouped_pattern '*' range_var range_constraint
        {
            $$ = buildIterExpr($1,$3,$4);
            delete($3);
            delete($4);
        }
    ;

pattern:
        '<' NUM '>'
        {
            if ($2 < 0)
                yyerror("the size must be positive");

            $$ = new string("\t(lc += ");
            (*$$) = (*$$) + $2;
            (*$$) += ", true)";
        }
    |   simple_pattern
        {
            $$ = $1;
        }
    |   optional_pattern
        {
            $$ = $1;
        }
    |   grouped_pattern
        {
            $$ = $1;
        }
    |   multi_match_pattern
        {
            $$ = $1;
        }
    |   pattern ';' pattern
        {
            $$ = $1;
            (*$$) += " && \n";
            (*$$) += (*$3);
            delete ($3);
        }
    |   pattern '|' pattern
        {
            $$ = $1;
            (*$$) += " || \n";
            (*$$) += (*$3);
            delete ($3);
        }
    |   NONE
        {
            // End up with if(false) {instantiate this Logur} else {return NULL}
            $$ = new string("false");
        }
    ;

%%

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


