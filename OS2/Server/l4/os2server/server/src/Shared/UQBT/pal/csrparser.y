/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2000, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:          csrparser.y
 * OVERVIEW:      Defines a parser for a Call Signature Recovery (CSR)
 *                specification; what is now called the Procedural 
 *                Abstraction Language (PAL) specification.
 *
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 * Copyright (C) 2000, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.9 $
 * 08 Feb 00 - Mike: Added optional global abstraction
 * 14 Mar 00 - Cristina: Added AGP support (abstract global pointer)
 * 22 Feb 01 - Mike: Added GLOBALOFFSET support
 * 26 Apr 01 - Mike: Use typeToSemStrMap to make the ReturnLocation type to
 *              SemStr map sign insensitive
 * 01 May 01 - Mike: If can't open PAL file, exit (else hangs in parser)
 * 23 Jul 01 - Mike: Added "STACK IS NORMAL|INVERTED" logic
 * 01 Aug 01 - Mike: GLOBAL ABSTRACTION has new format, just a list of register
 *              substitutions (no INIT, no logue associations)
 * 17 Aug 01 - Mike: Parse extra specs for floating point register parameters
 */


%name CSRParser

%define DEBUG 1
%define DEBUG_INIT 1

// These macros can only be defined if DEBUG != 0
%define ERROR_VERBOSE

// stuff to go in csrparser.h
%header{
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
%}

/*==============================================================================
 * Declaration of semantic value type. Must be first thing in csrparser.cc.
 *============================================================================*/
%union {
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
}
// Other stuff to go in csrparser.cc
%{
#include "csrscanner.h"
%} 

/*==============================================================================
 * Since this parser is designed to build the dictionaries and other structures
 * required for CSR analysis, most of its members are simply references to these
 * structures in a CSR object and as such, it must be initialised with a CSR
 * object so that these aliases are initialised properly.
 *============================================================================*/
%define CONSTRUCTOR_PARAM \
    const string& csrFile, \
    CSR& csr, \
    RTLInstDict& rtlDict, \
    bool trace /*= false*/

%define CONSTRUCTOR_INIT : \
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

%define CONSTRUCTOR_CODE \
    FILE* inFile = fopen(csrFile.c_str(),"r"); \
    if (inFile == NULL) { \
        ostrstream ost; \
        ost << "can't open `" << csrFile << "' for reading"; \
        error(str(ost)); \
        exit(1); \
    } \
    theScanner = new CSRScanner(inFile, trace); \
    if (trace) yydebug = 1;

/*==============================================================================
 * Declaration of class members.
 *============================================================================*/
%define MEMBERS \
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


/*==============================================================================
 * Declaration of token types, associativity and precedence
 *============================================================================*/

%token <str> MEM_IDX REG_ID IDENT ARITH_OP
%token <num> NUM GLOBALOFFSET
%token       INTEGER ADDRESS FLOAT AGGREGATE PARAMETERS RETURNS IN BASE
%token       OFFSET DEFINED_BY REGISTERS STACK FRAME GLOBAL
%token       ABSTRACTION INIT PARENT TO STEP LOCALS IS INVERTED NORMAL
%token       DOUBLE ALIGNMENT BYTES

%type <List_string>     ident_list
%type <ss>              exp register_id aggregate_decl
%type <Map_ss_ss>       substitutions agpSubstitutions
%type <List_ss>         int_registers float_registers double_registers
%type <Pair_ss_int>     stack
%type <param>           parameters_clauses
%type <Pair_Type_ss>    return_decl
%type <Map_Type_ss>     return_decls
%type <parentStack>     range
%type <PType>           align_type
%type <Pair_Type_int>   align
%type <Map_Type_int>    alignment

%left  ARITH_OP
%%

/*==============================================================================
 * This is the start symbol.
 *============================================================================*/

CSR_Spec:
        stack_declaration
        frame_abstraction
        global_abstraction
        locals
        parameters
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
        }
        returns
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
        }
        parent_stack
    ;


/*==============================================================================
 * This part defines constructs used by the different parts of a CSR spec.
 *============================================================================*/

ident_list:
        IDENT
        {
            $$ = new list<string>;
            $$->push_back($1);
        }

    |   ident_list IDENT
        {
            $1->push_back($2);
            $$ = $1;
        }
    ;

register_id:
        REG_ID
        {
            map<string,int,less<string> >::const_iterator it = 
                rtlDict.RegMap.find($1);

            // Special case for the AFP
            if (string($1) == "%afp") {
                $$ = new SemStr();
                $$->push(idAFP);
            }
            // Special case for the AGP register
            else if (string($1) == "%agp") {
                $$ = new SemStr();
                $$->push(idAGP); 
            }
            else if (it == rtlDict.RegMap.end()) {
                ostrstream os;
                os << "`" << $1 << "' is not declared";
                yyerror(str(os));
            } else {
                if ((*it).second == -1) {

                    // A special register, e.g. %pc or %npc
                    ostrstream ost;
                    ost << "`" << $1 << "' is a special register ";
                    ost << "which can't be used in a CSR specification";
                    yyerror(str(ost));

                } else {

                    // An immediate register
                    // Create it with a type appropriate for the register
                    const Register& reg = rtlDict.DetRegMap[it->second];
                    $$ = new SemStr(reg.g_type());
                    $$->push(idRegOf);
                    $$->push(idIntConst);
                    $$->push((*it).second);
                }
            }
        }
    ;

exp:
        exp ARITH_OP exp
        {
            $$ = $1;
            $$->prep(theSemTable.findItem($2));
            $$->pushSS($3);
            delete $3;
        }

    |   register_id
        {
            $$ = $1;
        }

    |   MEM_IDX exp ']'
        {
            $$ = $2;
            $$->prep(idMemOf);
        }

    |   NUM
        {
            $$ = new SemStr();
            $$->push(idIntConst);
            $$->push($1);
        }

        // This should only be seen in a GLOBAL ABSTRACTION section
        // It represents the difference in bytes from %agp to the machine's
        // global data pointer register
    |   GLOBALOFFSET
        {
            // The loader knows about the address of %agp (first), and a
            // constant (second) that represents GLOBALOFFSET
            pair<unsigned, unsigned> pr = prog.pBF->GetGlobalPointerInfo();
            $$ = new SemStr();
            $$->push(idIntConst);
            $$->push(pr.second);
        }

    |   IDENT
        {

            // Any identifier used in an expression must be in the
            // current scope
            checkScope($1);

            int varIdx = theSemTable.findItem($1,false);
            $$ = new SemStr();
            $$->push(idParam);
            $$->push(varIdx);
        }
    |   '(' exp ')'
        {
            $$ = $2;
        }
    ;

/*==============================================================================
 * This part of the grammar is for specifying the frame abstraction mechanism.
 *============================================================================*/

substitutions:
        register_id DEFINED_BY exp
        {
            if ($1->getFirstIdx() == idAFP)
                yyerror("can't use %afp on the left hand side of a substitution");
            $$ = new map<SemStr,SemStr>;
            (*$$)[*$1] = *$3;
            delete $1;
            delete $3;
        }
    |   substitutions register_id DEFINED_BY exp
        {
            $$ = $1;
            (*$$)[*$2] = *$4;
            delete $2;
            delete $4;
        }
    |   // Can have no substitutions. For example, a simple callee prologue
        // which does not have a "link" style instruction, and does not set up
        // a frame register, would have nothing here
        {
            $$ = new map<SemStr, SemStr>;
        }
    ;

afp_substitutions:
        ident_list '{'
        {
            setScope(*$1,Logue::CALLEE_PROLOGUE);
        }

        substitutions '}'
        {
            for (list<string>::iterator it = $1->begin();
                it != $1->end(); it++) {

                CalleePrologue* logue = (CalleePrologue*)getLogue(*it);
                logue->setSubstitutions(*$4);
            }
            delete $4;
            delete $1;
        }
    ;

afps:
        afp_substitutions
    |   afps afp_substitutions
    |
    ;

stack_declaration:
        STACK IS NORMAL
        {
            stackInverted = false;
        }
    |   STACK IS INVERTED
        {
            stackInverted = true;
        }
    |
        {
            stackInverted = false;
        }
    ;

frame_abstraction:
        FRAME ABSTRACTION
        {
            clearScope();
        }
        INIT '=' register_id
        afps
        {
            // Save the AFP initial value
            afpInit = *$6;
            delete $6;
        }
    ;

/*==============================================================================
 * The global abstraction mechanism is for architectures like 68K (Macintosh 
 * or Palm Pilot) which reserve a register (e.g. a5) for pointing to globals. 
 * There can be more than one substitution (e.g. more than one data pointers),
 * but no association with logues (since global pointers are truly global).
 * This is an optional section in the PAL grammar. 
 *============================================================================*/

agpSubstitutions:
        register_id DEFINED_BY exp
        {
            if ($1->getFirstIdx() == idAGP)
                yyerror(
                  "can't use %agp on the left hand side of a substitution");
            $$ = new map<SemStr,SemStr>;
            (*$$)[*$1] = *$3;
            delete $1;
            delete $3;
        }
    |   agpSubstitutions register_id DEFINED_BY exp
        {
            $$ = $1;
            (*$$)[*$2] = *$4;
            delete $2;
            delete $4;
        }
    ;

global_abstraction:
        GLOBAL ABSTRACTION agpSubstitutions
        {
            // Save the AGP map
            agpMap = *$3;
            delete $3;
        }
    |
    ;

/*==============================================================================
 * This part of the grammar is for specifying the number of bytes
 * allocated on the stack for local variables by the various callee
 * prologues.
 *============================================================================*/

locals_decl:
        ident_list '{'
        {
            setScope(*$1,Logue::CALLEE_PROLOGUE);
        }

        exp '}'
        {
            for (list<string>::iterator it = $1->begin();
                it != $1->end(); it++) {

                CalleePrologue* logue = (CalleePrologue*)getLogue(*it);
                logue->setLocalsSize(*$4);
            }
            delete $1;
            delete $4;
        }
    ;

locals:
        LOCALS locals_decl
    |   locals locals_decl
    ;

/*==============================================================================
 * This part of the grammar is for specifying parameter locations.
 *============================================================================*/

aggregate_decl:
        AGGREGATE DEFINED_BY exp
        {
            $$ = $3;
        }
    |
        {
            $$ = NULL;
        }
    ;

int_registers:
        REGISTERS DEFINED_BY register_id
        {
            $$ = new list<SemStr>;
            $$->push_back(*$3);
            delete $3;
        }
    |
        INTEGER REGISTERS DEFINED_BY register_id
        {
            $$ = new list<SemStr>;
            $$->push_back(*$4);
            delete $4;
        }

    |   int_registers register_id
        {
            $$ = $1;
            $$->push_back(*$2);
            delete $2;
        }
    |
        {
            $$ = NULL;
        }
    ;

float_registers:

        FLOAT REGISTERS DEFINED_BY register_id
        {
            $$ = new list<SemStr>;
            $$->push_back(*$4);
            delete $4;
        }

    |   float_registers register_id
        {
            $$ = $1;
            $$->push_back(*$2);
            delete $2;
        }
    |
        {
            $$ = NULL;
        }
    ;

double_registers:

        DOUBLE REGISTERS DEFINED_BY register_id
        {
            $$ = new list<SemStr>;
            $$->push_back(*$4);
            delete $4;
        }

    |   double_registers register_id
        {
            $$ = $1;
            $$->push_back(*$2);
            delete $2;
        }
    |
        {
            $$ = NULL;
        }
    ;

stack:

        STACK DEFINED_BY BASE '=' '[' exp ']' OFFSET '=' NUM
        {
            $$ = new pair<SemStr,int>(*$6,$10);
        }
    |   
        {
            $$ = NULL;
        }
    ;

align_type:
        INTEGER
        { $$ = new Type(::INTEGER); }
    |
        FLOAT
        { $$ = new Type(FLOATP, 32); }
    |
        DOUBLE
        { $$ = new Type(FLOATP, 64); }
    ;

align:
        align_type ALIGNMENT NUM BYTES
        {
            $$ = new pair<Type, int>(*$1, $3);
        }

alignment:
        align
        {
            $$ = new map<Type, int>;
            (*$$)[$1->first] = $1->second;
            delete $1;
        }
    |
        alignment align
        {
            (*$$)[$2->first] = $2->second;
            delete $2;
        }
    |
        { $$ = NULL; }

parameters_clauses:
        aggregate_decl
        int_registers
        float_registers
        double_registers
        stack
        alignment
        {
            $$ = new Parameters($1,$2,$3,$4,$5,$6);
            if ($1 != NULL) delete $1;
            if ($2 != NULL) delete $2;
            if ($3 != NULL) delete $3;
            if ($4 != NULL) delete $4;
            if ($5 != NULL) delete $5;
            if ($6 != NULL) delete $6;
        }
    ;

parameters_decl:
        ident_list '{'
        {
            setScope(*$1,Logue::CALLEE_PROLOGUE,"CALLER");
        }

        parameters_clauses '}'
        {
            setParameters(*$1,*$4);
            delete $4;
            delete $1;
        }
    ;

parameters:
        PARAMETERS parameters_decl
    |   parameters parameters_decl
    ;

/*==============================================================================
 * This part of the grammar is for specifying the valid locations for
 * returning values from a call.
 *============================================================================*/

return_decl:
        INTEGER '.' NUM IN register_id
        {
            // The semantic string for the register will have a default size
            // (e.g. 32 for mc68k %d0). Change to the size given
            $5->getType().setSize($3);
            $$ = new pair<Type, SemStr>(Type(::INTEGER, $3),*$5);
        }
    |   FLOAT '.' NUM IN register_id
        {
            $5->getType().setSize($3);
            $$ = new pair<Type, SemStr>(Type(::FLOATP, $3),*$5);
        }
    |   ADDRESS IN register_id
        {
            $$ = new pair<Type, SemStr>(Type(::DATA_ADDRESS),*$3);
        }
    ;

return_decls:
        return_decl
        {
            $$ = new typeToSemStrMap;
            $$->insert(*$1);
            delete $1;
        }
    |   return_decls return_decl
        {
            $$ = $1;
            $$->insert(*$2);
            delete $2;
        }
    ;

returns_:
        ident_list '{'
        {
            setScope(*$1,Logue::CALLEE_EPILOGUE,"CALLER");
        }

        return_decls '}'
        {
            setReturnLocations(*$1,ReturnLocations(*$4));
            delete $1;
            delete $4;
        }
    ;

returns:
        RETURNS returns_
    |   returns returns_
    ;

/*==============================================================================
 * This part of the grammar is for specifying the part of a parent's
 * stack that can be accessed (read and/or written to/from) by a callee.
 *============================================================================*/

range:
        exp TO exp STEP NUM
        {
            $$ = new ParentStack(*$1,*$3,$5);
            delete $1;
            delete $3;
        }
    ;

parent_stack_decl:
        ident_list '{'
        {
            setScope(*$1,Logue::CALLEE_PROLOGUE);
        }
        range '}'
        {
            setParentStackSpecs(*$1,*$4);
            delete $4;
            delete $1;
        }
    ;
parent_stack:
        PARENT STACK parent_stack_decl
    |   parent_stack parent_stack_decl
    |
    ;

%%

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
