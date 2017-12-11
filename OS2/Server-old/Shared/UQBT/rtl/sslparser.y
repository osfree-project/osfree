/*
 * Copyright (C) 1997, Shane Sendall
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 1998, David Ung
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       sslparser.y
 * OVERVIEW:   Defines a parser class that reads an SSL specification and
 *             returns the list of SSL instruction and table definitions.
 * 
 * (C) 1997, Shane Sendall
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 * (C) 1998, David Ung
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/* $Revision: 1.35 $
 * Updates:
 * Shane Sendall (original C version) Dec 1997
 * Doug Simon (C++ version) Jan 1998
 *  2 Mar, 98 - David : added code for parsing ConstantExpr - r[rd+1]
 * 17 Apr, 98 - David : added code for constructing new class ModRM.
 * 28 Apr, 98 - David : added Flag RTLS to parser.
 * 23 Jun, 98 - Mike  : Semantic String version
 * 12 Jul, 98 - Mike  : rparam nonterminal for r[param rd] problem
 * 15 Oct, 98 - Mike  : pushSS takes SemStr* now
 * 04 Nov, 98 - Mike  : implement ADDR(exp)
 * 12 Dec, 98 - Shawn : completed additions of float operators.
 * 07 Jan, 98 - Mike  : Commented out code to do with NewRTs (no longer used)
 * 13 Jan, 99 - Ian   : added new parameter to yyparse for interpreter,
 *                      map<int, Register, less<int> >
 *                      and included headers needed. (reg.h)
 * 14 Jan, 99 - Ian   : added syntactic structures for handling detailed 
 *                      registers.
 * 04 Mar 99 - Mike: Added CONV_FUNC (fsize, itof, ftoi); FPUSH, FPOP
 * 05 Mar 99 - Mike: Moved AT to var_op (so exp@[] can appear on the LHS of
 *                      an expression)
 * 09 Mar 99 - Mike: Added TRANSCEND(), fround(), floating point constants
 * 10 Mar 99 - Mike: Removed FUNARY_OP (was only SQRTs etc)
 * 17 Mar 99 - Doug: Removed NewRTs related code
 * 30 Mar, 99 - David : added deflist rules to the parser.
 *                      changed the parameters to yyparse to take in a
 *                      RTLDict as the 3rd argument
 * 15 Apr, 99 - Doug: Removed the creation of new SemStr's for internal nodes of
 *                    an expression tree, just built ontop of one of the
 *                    subexpressions instead. All other subexpressions apart
 *                    from the one being built onto are now deleted once the
 *                    complete expression is built.
 * 15 May, 99 - Doug: Fixed parsing of floating point conversion functions (I
 *                    think). Was putting the conversion sizes in front of the
 *                    operator in the SemStr.
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 09 Jul 99 - Doug: Modified to remove need for *_base files
 * 11 Jan 00 - Mike: CONV_FUNC had the arguments around the wrong way
 * 19 Jan 00 - Mike: Added support for the INTEGER and FLOAT keywords in the
 *                    @REGISTER section; old FLOAT -> FLOATNUM
 * 22 Nov 00 - Mike: Added ability to have "-1" in `constants' initialisation;
 *                    separated Quick and standard versions of MULT/DIV (using
 *                    the @FAST@ section)
 * 30 Nov 00 - Mike: trunc -> truncu, truncs
 * 05 Dec 00 - Mike: Allow bit extraction (@ operator) on expressions
 * 31 Jan 00 - Mike: Removed idDef and idIndex; not used except
 *                    for UQDBT on another CVS tree
 * 11 Feb 01 - Nathan: special registers now take (and require) sizes
 * 11 Feb 01 - Nathan: added operand section for interpreter
 * 11 Feb 01 - Nathan: added list form for register definitions
 * 15 Feb 01 - Nathan: Added support for explicit fetch-execute cycle
 * 07 Mar 01 - Simon: New table expansion code; all SSLs are expanded here in
 *              the SSL parser rather than later in expand functions
 * 12 Mar 01 - Mike: Quelched a signed/unsigned warning
 * 22 Mar 01 - Simon: added '\'' NAME '\'' code -> expands to { "", "NAME" }
 * 26 Mar 01 - Mike: Trivial - fixed an error message
 * 24 Apr 01 - Nathan: Fixed some old merging errors
 * 03 Jun 01 - Nathan: Changed logical ops (and/or) to have lower
 *              precedence than conditionals (as per C)
 * 26 Jun 01 - Nathan: Added syntax & semantics for function-like params
 * 10 Jul 01 - Nathan: Added syntax for conditional assignment
 * 30 Jul 01 - Mike: SHARES registers were always integer
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. 
 * 14 Aug 01 - Mike: Removed toUpper (would not compile under gcc v3)
 */

%name SSLParser     // the parser class name will be SSLParser

// stuff to go in sslparser.h
%header{
#include "global.h"
#include "rttelem.h"
#include "hrtl.h"
#define IntType 1
#define VectorType 2
#ifdef WIN32
#include <malloc.h>
#endif
class SSLScanner;
%}

//*============================================================================
//* Declaration of semantic value type. Must be first thing in
//* sslparser.cc (or at least before including sslscanner.h which needs
//* to know about this type).
//*============================================================================
%union {
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
}

// Other stuff to go in sslparser.cc
%{
#include "sslscanner.h"
%}

%define DEBUG 1 

%define PARSE_PARAM \
    RTLInstDict& Dict

%define CONSTRUCTOR_PARAM \
    const string& sslFile, \
    bool trace

%define CONSTRUCTOR_INIT : \
   sslFile(sslFile), bFloat(false)

%define CONSTRUCTOR_CODE \
    FILE* inFile = fopen(sslFile.c_str(),"r"); \
    if (inFile == NULL) { \
        ostrstream ost; \
        ost << "can't open `" << sslFile << "' for reading"; \
        error(str(ost)); \
    } \
    theScanner = new SSLScanner(inFile, trace); \
    if (trace) yydebug = 1;

%define MEMBERS \
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

/*==============================================================================
 * Declaration of token types, associativity and precedence
 *============================================================================*/

%token <str> ASSIGNSIZE COND_OP BIT_OP ARITH_OP LOG_OP
%token <str> NAME 
%token <str> REG_ID COND_TNAME DECOR
%token <str> FARITH_OP FPUSH FPOP
%token <str> TEMP SHARES CONV_FUNC TRANSCEND
%token <str> BIG LITTLE
%token <str> NAME_CALL NAME_LOOKUP

%token       INFINITESTACK ENDIANNESS COVERS INDEX
%token       SHARES NOT THEN LOOKUP_RDC BOGUS
%token       ASSIGN TO COLON S_E AT ADDR REG_IDX EQUATE
%token       MEM_IDX INTEGER FLOAT FAST OPERAND FETCHEXEC CAST_OP FLAGMACRO

%token <num> NUM  
%token <dbl> FLOATNUM      // I'd prefer type double here!

%token 

%left LOG_OP
%right COND_OP
%left BIT_OP
%left ARITH_OP
%left FARITH_OP
%right NOT
%left CAST_OP
%left LOOKUP_RDC
%left S_E
%nonassoc AT

%type <ss> exp var_op exp_term
%type <str> bin_oper
%type <regtransfer> rt assign_rt
%type <num> cast internal_type param
%type <tab> table_expr
%type <insel> name_contract instr_name instr_elem
%type <strlist> reg_table
%type <parmlist> list_parameter func_parameter
%type <namelist> str_term str_expr str_array name_expand opstr_expr opstr_array
%type <spreglist> flag_list
%type <exprlist> exprstr_expr exprstr_array
%type <sslist> list_actualparameter
%type <rtlist> rt_list
%type <str> esize

%%

specification:
        specification parts ';'
    |   parts ';'
    ;

parts:
        instr 
        
    |   FETCHEXEC rt_list {
            map<string, InsNameElem*, StrCmp> m;
            Dict.fetchExecCycle = $2->eval(TableDict, m);
            delete $2;
        }
    
    |   constants

    |   table_assign

    |   endianness

    |   fastlist

    |   reglist

    |   definition

    |   flag_fnc

    |   OPERAND operandlist { Dict.fixupParams(); }

    ;

operandlist:
        operandlist ',' operand
    |   operand
    ;

operand:
        param EQUATE '{' list_parameter '}' {
            Dict.DetParamMap[$1].params = *$4;
            Dict.DetParamMap[$1].kind = PARAM_VARIANT;
            delete $4;
        }
    |   param list_parameter func_parameter ASSIGNSIZE exp {
            map<string, InsNameElem*, StrCmp> m;
            ParamEntry &param = Dict.DetParamMap[$1];
            RTTAssignElem* rt = new RTTAssignElem(new SSListElem(), $5, $4);
            param.params = *$2;
            param.funcParams = *$3;
            param.rta = (RTAssgn *)(rt->eval(TableDict, m));
            param.kind = PARAM_EXPR;
            
            if( param.funcParams.size() != 0 ) {
                param.kind = PARAM_LAMBDA;
                /* Change the SemTable entry into a "function call" */
                theSemTable.setItem($1, cFUNCTION,0,0,param.funcParams.size(),
                                    theSemTable[$1].sName.c_str());
                
            }
            delete $2;
            delete $3;
            delete rt;
        }
    ;

func_parameter: '[' list_parameter ']' { $$ = $2; }
        |   { $$ = new list<int>(); }
        ;

definition:
            internal_type NAME {
                // add this def to the DefMap and create the object
                pair<int, void*> *temp = new pair<int, void*>;              
                temp->first = $1;
                temp->second = new vector<int>;
                Dict.DefMap[$2] = temp;
            }
        |   NAME ASSIGN NUM {
                // add the def to the DefMap and create the object
                int *temp_int = new int;
                *temp_int = $3;
                pair<int,void*> *pair_temp = new pair<int,void*>;
                pair_temp->first = IntType;
                pair_temp->second = temp_int;
                Dict.DefMap[$1] = pair_temp;
            }
        |   aliases
        ;   


internal_type:
            INFINITESTACK {
                $$ = VectorType;
            }
        ;

        
aliases:        
            REG_ID THEN exp {
            
                int temp_reg;   
                map<string, int, StrCmp>::const_iterator it =
                    Dict.RegMap.find($1);
                if (it == Dict.RegMap.end()) {
                    ostrstream ost;
                    ost << "register `" << $1 << "' is undefined";
                    yyerror(str(ost));
                } else {
                    if ((*it).second == -1) {
                        // A special register, 
                        ostrstream ost;
                        ost << "`" << $1 << "' is a special register ";
                        ost << " and as such can't be used as an alias";
                        yyerror(str(ost));
                    } else {
                        // An immediate register
                        temp_reg = (*it).second;
                    }
                }
                map<string,InsNameElem*,StrCmp> m; 
                Dict.AliasMap[temp_reg] = $3->eval(TableDict, m);
                delete $3;
           } 
    ; 

            
reglist:
                INTEGER {
                    bFloat = false;
                } a_reglists
            |   FLOAT {
                    bFloat = true;
                } a_reglists
            ;

a_reglists:     a_reglists ',' a_reglist
            |   a_reglist
            ;

a_reglist:
            REG_ID INDEX NUM {
                if (Dict.RegMap.find($1) != Dict.RegMap.end())
                    yyerror("Name reglist decared twice\n");
                Dict.RegMap[$1] = $3;
            }
        |   REG_ID '[' NUM ']' INDEX NUM {
                if (Dict.RegMap.find($1) != Dict.RegMap.end())
                    yyerror("Name reglist declared twice\n");
                Dict.addRegister( $1, $6, $3, bFloat);
            }
        |   REG_ID '[' NUM ']' INDEX NUM COVERS REG_ID TO REG_ID {
                if (Dict.RegMap.find($1) != Dict.RegMap.end())
                    yyerror("Name reglist declared twice\n");
                Dict.RegMap[$1] = $6;
                // Now for detailed Reg information
                if (Dict.DetRegMap.find($6) != Dict.DetRegMap.end())
                    yyerror("Index used for more than one register\n");
                Dict.DetRegMap[$6].s_name($1);
                Dict.DetRegMap[$6].s_size($3);
                Dict.DetRegMap[$6].s_address(NULL);
                // check range is legitimate for size. 8,10
                if ((Dict.RegMap.find($8) == Dict.RegMap.end())
                    || (Dict.RegMap.find($10) == Dict.RegMap.end()))
                   yyerror("Undefined range\n");
                else {
                    int bitsize = Dict.DetRegMap[Dict.RegMap[$10]].g_size();
                    for (int i = Dict.RegMap[$8]; i != Dict.RegMap[$10]; i++) {
                        if (Dict.DetRegMap.find(i) == Dict.DetRegMap.end()) {
                            yyerror("Not all regesters in range defined\n");
                            break;
                        }
                        bitsize += Dict.DetRegMap[i].g_size();
                        if (bitsize > $3) {
                            yyerror("Range exceeds size of register\n");
                            break;
                        }
                    }
                if (bitsize < $3) 
                    yyerror("Register size is exceeds regesters in range\n");
                    // copy information
                }
                Dict.DetRegMap[$6].s_mappedIndex(Dict.RegMap[$8]);
                Dict.DetRegMap[$6].s_mappedOffset(0);
                Dict.DetRegMap[$6].s_float(bFloat);
            }
        |   REG_ID '[' NUM ']' INDEX NUM SHARES REG_ID AT '[' NUM TO NUM ']' {
                if (Dict.RegMap.find($1) != Dict.RegMap.end())
                    yyerror("Name reglist declared twice\n");
                Dict.RegMap[$1] = $6;
                // Now for detailed Reg information
                if (Dict.DetRegMap.find($6) != Dict.DetRegMap.end())
                    yyerror("Index used for more than one register\n");
                Dict.DetRegMap[$6].s_name($1);
                Dict.DetRegMap[$6].s_size($3);
                Dict.DetRegMap[$6].s_address(NULL);
                // Do checks
                if ($3 != ($13 - $11) + 1) 
                    error("Size does not equal range\n");
                    if (Dict.RegMap.find($8) != Dict.RegMap.end()) {
                        if ($13 >= Dict.DetRegMap[Dict.RegMap[$8]].g_size())
                            yyerror("Range extends over target register\n");
                    } else 
                        yyerror("Shared index not yet defined\n");
                Dict.DetRegMap[$6].s_mappedIndex(Dict.RegMap[$8]);
                Dict.DetRegMap[$6].s_mappedOffset($11);
                Dict.DetRegMap[$6].s_float(bFloat);
            }
        |   '[' reg_table ']' '[' NUM ']' INDEX NUM TO NUM {
                if ((int)$2->size() != ($10 - $8 + 1)) {
                    cerr << "size of register array does not match mapping to r["
                         << $8 << ".." << $10 << "]\n";
                    exit(1);
                } else {
                    list<string>::iterator loc = $2->begin();
                    for (int x = $8; x <= $10; x++, loc++) {
                        if (Dict.RegMap.find(*loc) != Dict.RegMap.end())
                            yyerror("Name reglist declared twice\n");
                        Dict.addRegister( loc->c_str(), x, $5, bFloat);
                    }
                    delete $2;
                }
            }
        |   '[' reg_table ']' '[' NUM ']' INDEX NUM {
                list<string>::iterator loc = $2->begin();
                for (; loc != $2->end(); loc++) {
                    if (Dict.RegMap.find(*loc) != Dict.RegMap.end())
                        yyerror("Name reglist declared twice\n");
		    Dict.addRegister(loc->c_str(), $8, $5, bFloat);
                }
                delete $2;
            }
        ;

reg_table:
            reg_table ',' REG_ID {
                $1->push_back($3);
            }
        |   REG_ID {
                $$ = new list<string>;
                $$->push_back($1);
            }
        ;

flag_fnc:
            NAME_CALL list_parameter ')' '{' rt_list '}' {
                    // passing an empty variable map
                    map<string, InsNameElem*, StrCmp> m;
                    RTTFDefElem frt = RTTFDefElem($1, *$2, $5);
                    Dict.FlagFuncs[$1] = (RTFlagDef*)(frt.eval(TableDict, m));
                    delete $2;
                    // frt will delete $6
            }
        ;

constants: 
            NAME EQUATE NUM {
                if (ConstTable.find($1) != ConstTable.end())
                    yyerror("Constant declared twice");
                ConstTable[string($1)] = $3;
            }

        |   NAME EQUATE NUM ARITH_OP NUM {
                if (ConstTable.find($1) != ConstTable.end())
                    yyerror("Constant declared twice");
                else if ($4 == string("-"))
                    ConstTable[string($1)] = $3 - $5;
                else if ($4 == string("+"))
                    ConstTable[string($1)] = $3 + $5;
                else
                    yyerror("Constant expression must be NUM + NUM or NUM - NUM");
            }
        ;


table_assign:
        NAME EQUATE table_expr {
            TableDict[$1] = $3;
        }
    ;

table_expr:
        str_expr {
            $$ = new Table(*$1);
            delete $1;
        }
    |   opstr_expr {
            $$ = new OpTable(*$1);
            delete $1;
        }
    |   exprstr_expr {
            $$ = new ExprTable(*$1);
            delete $1;
        }
    ;

str_expr:
        str_expr str_term {
            // cross-product of two str_expr's
            deque<string>::iterator i, j;
            $$ = new deque<string>;
            for (i = $1->begin(); i != $1->end(); i++)
                for (j = $2->begin(); j != $2->end(); j++)
                    $$->push_back((*i) + (*j));
            delete $1;
            delete $2;
        }
    |   str_term {
            $$ = $1;
        }
    ;

str_array:
        str_array ',' str_expr {
            // want to append $3 to $1
            // The following causes a massive warning message about mixing
            // signed and unsigned
            $1->insert($1->end(), $3->begin(), $3->end());
            delete $3;
            $$ = $1;
        }
    |   str_array ',' '"' '"' {
            $1->push_back("");
        }
    |   str_expr {
            $$ = $1;
        }
    ;

str_term:
        '{' str_array '}' {
            $$ = $2;
        }
    |   name_expand {
            $$ = $1;
        }
    ;

name_expand:
        '\'' NAME '\'' {
            $$ = new deque<string>;
            $$->push_back("");
            $$->push_back($2);
        }
    |   '"' NAME '"' {
            $$ = new deque<string>(1, $2);
        }
    |   '$' NAME {
            ostrstream o;
            // expand $2 from table of names
            if (TableDict.find($2) != TableDict.end())
                if (TableDict[$2]->getType() == NAMETABLE)
                    $$ = new deque<string>(TableDict[$2]->records);
                else {
                    o << "name " << $2 << " is not a NAMETABLE.\n";
                    yyerror(str(o));
                }
            else {
                o << "could not dereference name " << $2 << "\n";
                yyerror(str(o));
            }
        }
    |   NAME {
            // try and expand $2 from table of names
            // if fail, expand using '"' NAME '"' rule
            if (TableDict.find($1) != TableDict.end())
                if (TableDict[$1]->getType() == NAMETABLE)
                    $$ = new deque<string>(TableDict[$1]->records);
                else {
                    ostrstream o;
                    o << "name " << $1 << " is not a NAMETABLE.\n";
                    yyerror(str(o));
                }
            else {
                $$ = new deque<string>;
                $$->push_back($1);
            }
        }
    ;

bin_oper:
        BIT_OP {
            $$ = $1;
        }

    |   ARITH_OP {
            $$ = $1;
        }

    |   FARITH_OP {
            $$ = $1;
        }
    ;

opstr_expr:
        '{' opstr_array '}' {
            $$ = $2;
        }
    ;

opstr_array:
        opstr_array ',' '"' bin_oper '"' {
            $$ = $1;
            $$->push_back($4);
        }
    |   '"' bin_oper '"' {
            $$ = new deque<string>;
            $$->push_back($2);
        }
    ;

exprstr_expr:
        '{' exprstr_array '}' {
            $$ = $2;
        }
    ;

exprstr_array:
        exprstr_array ',' '"' exp '"' {
            $$ = $1;
            $$->push_back($4);
        }
    |   '"' exp '"' {
            $$ = new deque<SSElem*>;
            $$->push_back($2);
        }
    ;

instr:
        instr_name {
            $1->getrefmap(indexrefmap);
        } list_parameter rt_list {
            int i, m;
            string n;
            ostrstream o;
            HRTL* r;
            m = $1->ninstructions();
            for (i = 0, $1->reset(); i < m; i++, $1->increment()) {
                n = $1->getinstruction();
                r = $4->eval(TableDict, indexrefmap);
                if (Dict.appendToDict(n, *$3, *r) != 0) {
                    o << "Pattern " << $1->getinspattern()
                        << " conflicts with an earlier declaration of " << n << ".\n";
                    yyerror(str(o));
                }
            }
            delete $1;
            delete $3;
            delete $4;
            indexrefmap.erase(indexrefmap.begin(), indexrefmap.end());
        }
    ;

instr_name:
        instr_elem {
            $$ = $1;
        }
    |   instr_name DECOR {
            unsigned i;
            InsNameElem *temp;
            string nm = $2;
            
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
            $$ = $1;
            $$->append(temp);
        }
    ;

instr_elem:
        NAME {
            $$ = new InsNameElem($1);
        }
    |   name_contract {
            $$ = $1;
        }
    |   instr_elem name_contract {
            $$ = $1;
            $$->append($2);
        }
    ;

name_contract:
        '\'' NAME '\'' {
            $$ = new InsOptionElem($2);
        }
    |   NAME_LOOKUP NUM ']' {
            ostrstream o;
            if (TableDict.find($1) == TableDict.end()) {
                o << "Table " << $1 << " has not been declared.\n";
                yyerror(str(o));
            } else if (($2 < 0) || ($2 >= (int)TableDict[$1]->records.size())) {
                o << "Can't get element " << $2 << " of table " << $1 << ".\n";
                yyerror(str(o));
            } else
                $$ = new InsNameElem(TableDict[$1]->records[$2]);
        }
    |   NAME_LOOKUP NAME ']' {
            ostrstream o;
            if (TableDict.find($1) == TableDict.end()) {
                o << "Table " << $1 << " has not been declared.\n";
                yyerror(str(o));
            } else
                $$ = new InsListElem($1, TableDict[$1], $2);
        }
        
    |   '$' NAME_LOOKUP NUM ']' {
            ostrstream o;
            if (TableDict.find($2) == TableDict.end()) {
                o << "Table " << $2 << " has not been declared.\n";
                yyerror(str(o));
            } else if (($3 < 0) || ($3 >= (int)TableDict[$2]->records.size())) {
                o << "Can't get element " << $3 << " of table " << $2 << ".\n";
                yyerror(str(o));
            } else
                $$ = new InsNameElem(TableDict[$2]->records[$3]);
        }
    |   '$' NAME_LOOKUP NAME ']' {
            ostrstream o;
            if (TableDict.find($2) == TableDict.end()) {
                o << "Table " << $2 << " has not been declared.\n";
                yyerror(str(o));
            } else
                $$ = new InsListElem($2, TableDict[$2], $3);
        }
    
    |   '"' NAME '"' {
            $$ = new InsNameElem($2);
        }
    ;

rt_list:
        rt_list rt {
            // append any automatically generated RT's and clear the
            // list they were stored in. Do nothing for a NOP (i.e. $2 = 0)
            if ($2 != NULL) {
                $1->append($2);
            }
            $$ = $1;
        }

    |   rt {
            $$ = new RTTListElem();
            if ($1 != NULL)
                $$->append($1);
        }
    ;

rt:
        assign_rt {
            $$ = $1;
        }
    |   NAME_CALL list_actualparameter ')' {
            ostrstream o;
            if (Dict.FlagFuncs.find($1) != Dict.FlagFuncs.end()) {
                RTTFCallElem* pfl;
                pfl = new RTTFCallElem(Dict.FlagFuncs[$1]->func_name, *$2);
                $$ = pfl;
            } else {
                o << $1 << " is not a valid rtl.\n";
                yyerror(str(o));
            }
        }
    |   FLAGMACRO flag_list ')' {
            $$ = 0;
        }
    |   FLAGMACRO ')' {
            $$ = 0;
        }
    |  '_' {
        $$ = NULL;
    }
    ;

flag_list:
        flag_list ',' REG_ID {
/*          SSListElem* pFlag = new SSListElem();
            pFlag->app(idRegOf);
            pFlag->app(Dict.RegMap[$3]);
            $1->push_back(pFlag);
            $$ = $1;
*/          $$ = 0;
        }
    |   REG_ID {
/*          list<SSElem*>* tmp = new list<SSElem*>;
            SSListElem* pFlag = new SSListElem();
            pFlag->app(idRegOf);
            pFlag->app(Dict.RegMap[$1]);
            tmp->push_back(pFlag);
            $$ = tmp;
*/          $$ = 0;
        }
    ;
    
list_parameter:
        list_parameter ',' param {
            assert($3 != 0);
            $1->push_back($3);
            $$ = $1;
        }

    |   param {
            $$ = new list<int>;
            $$->push_back($1);
        }
    |   {
            $$ = new list<int>;
        }
    ;

param:  NAME {
            if (Dict.ParamMap.find($1) == Dict.ParamMap.end()) {
                Dict.ParamMap[$1] = ($$ = theSemTable.addItem($1));
            } else {
                $$ = Dict.ParamMap[$1];
            }
        }

list_actualparameter:
        list_actualparameter ',' exp {
            $$->push_back($3);
        }

    |   exp {
            $$ = new list<SSElem*>;
            $$->push_back($1);
        }

    |   {
            $$ = new list<SSElem*>;
        }
    ;

assign_rt:
        ASSIGNSIZE exp THEN var_op EQUATE exp {
            $$ = new RTTAssignElem($4,$6,$1, $2);
        }
    |   ASSIGNSIZE var_op EQUATE exp {
            // update the size of any generated RT's
            $$ = new RTTAssignElem($2, $4, $1);
        }

        // FPUSH and FPOP are special RTAssgn's with an empty left hand side,
        // and a right hand side with just a single id
    |   FPUSH {
            $$ = new RTTAssignElem(new SSListElem(), new SSElem(idFpush), 0);
        }
    |   FPOP {
            $$ = new RTTAssignElem(new SSListElem(), new SSElem(idFpop), 0);
        }
    |   ASSIGNSIZE exp {
            $$ = new RTTAssignElem(new SSListElem(), $2, $1);
        }
    ;

exp_term:
        NUM {
            SSListElem* s = new SSListElem();
            s->app(idIntConst);
            s->app($1);
            $$ = s;
        }

    |   FLOATNUM {
            SSListElem* s = new SSListElem();
            s->app(idFltConst);
            // Note: code assumes the double takes the size of two integers!
            union {struct {int i1; int i2;} i; double d;} u;
            u.d = $1;
            s->app(u.i.i1);
            s->app(u.i.i2);
            $$ = s;
        }

    |   TEMP {
            SSListElem* s = new SSListElem();
            s->app(idTemp);
            if (TempMap.find($1) == TempMap.end())
                TempMap[$1] = theSemTable.addItem($1);
            s->app(TempMap[$1]);
            $$ = s;
        }
    
    |   '(' exp ')' {
            $$ = $2;
        }

    |   var_op {
            $$ = $1;
        }
    
    |   '[' exp '?' exp COLON exp ']' {
            SSListElem* s = new SSListElem();
            s->app(idTern);
            s->app($2);
            s->app($4);
            s->app($6);
            $$ = s;
        }

    |   '[' exp '?' exp COLON exp ']' cast {
            SSListElem* s = new SSListElem();
            if ($8 != STD_SIZE) {
                s->app(idSize);
                s->app($8);       // The size
            }
            s->app(idTern);
            s->app($2);
            s->app($4);
            s->app($6);
            $$ = s;
        }

    // Address-of, for LEA type instructions
    |   ADDR exp ')' {
            SSListElem* s = new SSListElem();
            s->app(idAddrOf);
            s->app($2);
            $$ = s;
        }

    // Conversion functions, e.g. fsize(32, 80, modrm)
    // Args are FROMsize, TOsize, EXPression
    |   CONV_FUNC NUM ',' NUM ',' exp ')' {
            ostrstream o;
            SSListElem* s = new SSListElem();
            s->app($2);
            s->app($4);
            s->app($6);
            if ($1 == string("fsize"))       s->prep(idFsize);
            else if ($1 == string("itof"))   s->prep(idItof);
            else if ($1 == string("ftoi"))   s->prep(idFtoi);
            else if ($1 == string("fround")) s->prep(idFround);
            else if ($1 == string("truncu")) s->prep(idTruncu);
            else if ($1 == string("truncs")) s->prep(idTruncs);
            else if ($1 == string("zfill"))  s->prep(idZfill);
            else if ($1 == string("sgnex"))  s->prep(idSgnEx);
            else {
                o << "Unknown CONV_FUNC " << $1;
                yyerror(str(o));
            }
            $$ = s;
        }

    // FPUSH and FPOP
    |   FPUSH {
            $$ = new SSElem(idFpush);
        }
    |   FPOP {
            $$ = new SSElem(idFpop);
        }

    // Transcendental functions
    |   TRANSCEND exp ')' {
            ostrstream o;
            SSListElem* s = new SSListElem();
            s->app($2);
            if ($1 == string("sin"))         s->prep(idSin);
            else if ($1 == string("cos"))    s->prep(idCos);
            else if ($1 == string("tan"))    s->prep(idTan);
            else if ($1 == string("arctan")) s->prep(idArcTan);
            else if ($1 == string("log2"))   s->prep(idLog2);
            else if ($1 == string("log10"))  s->prep(idLog10);
            else if ($1 == string("loge"))   s->prep(idLoge);
            else if ($1 == string("sqrt"))   s->prep(idSqrt);
            else if ($1 == string("execute"))s->prep(idExecute);
            else {
                o << "Unknown TRANSCEND " << $1;
                yyerror(str(o));
            }
            $$ = s;
        }

    |   NAME_LOOKUP NAME ']' {
            ostrstream o;
            if (indexrefmap.find($2) == indexrefmap.end()) {
                o << "index " << $2 << " not declared for use.\n";
                yyerror(str(o));
            } else if (TableDict.find($1) == TableDict.end()) {
                o << "table " << $1 << " not declared for use.\n";
                yyerror(str(o));
            } else if (TableDict[$1]->getType() != EXPRTABLE) {
                o << "table " << $1 << " is not an expression table but "
                  "appears to be used as one.\n";
                yyerror(str(o));
            } else if ((int)((ExprTable*)TableDict[$1])->expressions.size() <
              indexrefmap[$2]->ntokens()) {
                o << "table " << $1 << " (" <<
                  ((ExprTable*)TableDict[$1])->expressions.size() <<
                  ") is too small to use " << $2 << " (" <<
                  indexrefmap[$2]->ntokens() << ") as an index.\n";
                yyerror(str(o));
            }
            $$ = new SSTableElem($1, $2);
        }
    |   NAME_CALL list_actualparameter ')' {
        ostrstream o;
        if( Dict.ParamMap.find($1) != Dict.ParamMap.end() ) {
            if (Dict.DetParamMap.find(Dict.ParamMap[$1]) != Dict.DetParamMap.end()) {
                ParamEntry &param = Dict.DetParamMap[Dict.ParamMap[$1]];
                if( $2->size() != param.funcParams.size() ) {
                    o << $1 << " requires " << param.funcParams.size()
                      << " parameters, but received " << $2->size() << ".\n";
                    yyerror(str(o));
                } else {
                    /* Everything checks out. *phew* */
                    SSListElem *s = new SSListElem();
                    s->app(Dict.ParamMap[$1]);
                    for( list<SSElem *>::iterator it = $2->begin();
                         it != $2->end(); it++ ) {
                        s->app(*it);
                    }
                    delete $2;
                    $$ = s;
                }
            } else {
                o << $1 << " is not a function, even if you try to call it.\n";
                yyerror(str(o));
            }
        } else {
            o << $1 << ": Unrecognized name in call.\n";
            yyerror(str(o));
        }
    }
    ;

exp:
        exp S_E {
            SSListElem* s = new SSListElem();
            s->app(idSignExt);
            s->app($1);
            $$ = s;
        }

    |   exp cast %prec CAST_OP {
            SSListElem* s = new SSListElem();
            s->app($1);
            if ($2 != STD_SIZE) {
                s->prep($2);       // The size
                s->prep(idSize);
            }
            $$ = s;
        }
    
    |   NOT exp {
            SSListElem* s = new SSListElem();
            s->app(idNot);
            s->app($2);
            $$ = s;
        }

    |   exp FARITH_OP exp {
            SSListElem* s = new SSListElem();
            s->app(theSemTable.findItem($2));
            s->app($1);
            s->app($3);
            $$ = s;
        }

    |   exp ARITH_OP exp {
            SSListElem* s = new SSListElem();
            s->app(theSemTable.findItem($2));
            s->app($1);
            s->app($3);
            $$ = s;
        }

    |   exp BIT_OP exp {
            SSListElem* s = new SSListElem();
            s->app(theSemTable.findItem($2));
            s->app($1);
            s->app($3);
            $$ = s;
        }

    |   exp COND_OP exp {
            SSListElem* s = new SSListElem();
            s->app(theSemTable.findItem($2));
            s->app($1);
            s->app($3);
            $$ = s;
        }
        
    |   exp LOG_OP exp {
            SSListElem* s = new SSListElem();
            s->app(theSemTable.findItem($2));
            s->app($1);
            s->app($3);
            $$ = s;
        }
        
    |   exp NAME_LOOKUP NAME ']' exp_term %prec LOOKUP_RDC {
            ostrstream o;
            SSListElem* s = new SSListElem();
            if (indexrefmap.find($3) == indexrefmap.end()) {
                o << "index " << $3 << " not declared for use.\n";
                yyerror(str(o));
            } else if (TableDict.find($2) == TableDict.end()) {
                o << "table " << $2 << " not declared for use.\n";
                yyerror(str(o));
            } else if (TableDict[$2]->getType() != OPTABLE) {
                o << "table " << $2 << " is not an operator table but appears to be used as one.\n";
                yyerror(str(o));
            } else if ((int)TableDict[$2]->records.size() <
              indexrefmap[$3]->ntokens()) {
                o << "table " << $2 << " is too small to use " << $3
                  << " as an index.\n";
                yyerror(str(o));
            }
            s->app(new SSOpTableElem($2, $3));
            s->app($1);
            s->app($5);
            $$ = s;
        }

    |   exp_term {
            $$ = $1;
        }
    ;

var_op:
        // This is for immediate registers. Often, these are special,
        // in the sense that the register mapping is -1. If so, the
        // equivalent of a special register is generated, i.e. a special
        // index representing a register.
        REG_ID {
            map<string, int, StrCmp>::const_iterator it = Dict.RegMap.find($1);
            if (it == Dict.RegMap.end()) {
                ostrstream ost;
                ost << "register `" << $1 << "' is undefined";
                yyerror(str(ost));
            } else if ((*it).second == -1)
                    // A special register, e.g. %npc or %CF
                    // If it already exists, the existing index is returned
                    $$ = new SSElem(theSemTable.addItem($1));
            else {
                // An immediate register, e.g. %g2. In this case, we
                // want to return r[int 2]
                SSListElem* s = new SSListElem();
                s->app(idRegOf);
                s->app(idIntConst);
                s->app((*it).second);
                $$ = s;
            }
        }

    |   REG_IDX exp ']' {
            SSListElem* s = new SSListElem();
            s->prep($2);
            s->prep(idRegOf);
            $$ = s;
        }

    |   MEM_IDX exp ']' {
            SSListElem* s = new SSListElem();
            s->app($2);
            s->prep(idMemOf);
            $$ = s;
        }

    |   NAME {
        // this is a mixture of the param: PARM {} match
        // and the value_op: NAME {} match
            SSListElem* s = new SSListElem();
            map<string, int, StrCmp>::const_iterator it = Dict.ParamMap.find($1);
            if (it != Dict.ParamMap.end()) {
                s->app(idParam);
                s->app((*it).second);
            } else if (ConstTable.find($1) != ConstTable.end()) {
                s->app(idIntConst);
                s->app(ConstTable[$1]);
            } else {
                ostrstream ost;
                ost << "`" << $1 << "' is not a constant, definition or a";
                ost << " parameter of this instruction\n";
                yyerror(str(ost));
            }
            $$ = s;
        }

    |      exp AT '[' exp COLON exp ']' {
            SSListElem* s = new SSListElem();
            s->app($1);
            s->prep(idAt);
            s->app($4);
            s->app($6);
            $$ = s;
        }
    |      var_op '\'' {
            SSListElem *s = new SSListElem();
            s->app(idPostVar);
            s->app($1);
            $$ = s;
        }
    ;

cast:
        '{' NUM '}' {
            $$ = $2;
        }
    ;

endianness:
        ENDIANNESS esize {
            Dict.bigEndian = (strcmp($2, "BIG") == 0);
        }

esize:
        BIG {
            $$ = $1;
        }
    |   LITTLE {
            $$ = $1;
        }
    ;

// Section for indicating which instructions to substitute when using -f (fast
// but not quite as exact instruction mapping)
fastlist:
        FAST fastentries
    ;

fastentries:
        fastentries ',' fastentry
    
    |   fastentry

    ;

fastentry:
        NAME INDEX NAME {
            Dict.fastMap[string($1)] = string($3);
        }
%%

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
