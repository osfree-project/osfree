/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*=============================================================================
 * FILE:       codegenemu.h
 * OVERVIEW:   The main emulator generator class (CodeGenEmu)
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 *===========================================================================*/

/* $Revision: 1.15 $
 * 23 Apr 01 - Nathan: Initial version
 * 21 May 01 - David: CodeGenEmu::sprintBodyCode added to allow addition
 *              generation of instrumented code.
 * 21 May 01 - David: InstrmParser *instrm_parser now part of CodeGenEmu
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. 
 */    

#ifndef codegenemu_H
#define codegenemu_H

#include <map>
#include <vector>
#include <list>
#include "codegen.h"
#include "sledtree.h"
#include "hrtl.h"
#include "instrm_parser.h"
    
class RegisterPart {
  public:
    RegisterPart(int n,int b,int w) { num = n; offset = b; width = w; }
    int num;
    int offset;
    int width;
};

typedef vector<set<RegisterPart> > RegTree;
typedef set<RegisterPart>::iterator SRP_IT;
typedef map<string,Register,less<string> >::iterator sregmap_it;
typedef map<int,Register,less<int> >::iterator regmap_it;
typedef vector<Type>::const_iterator type_it;

class CodeGenEmu : public CodeGenApp {
  protected:
    // Used to allow generating multiple disassembleInstruction
    // methods if one gets too big for java to handle
    list<InstIdent *>::iterator disIt;
    int disIteration;
    list<InstIdent *> disInsts;

  public:
    /* Construct a new CodeGenEmu using the given language and
     * skeleton search path
     */
    CodeGenEmu( CodeGenLanguage *l, const string &s ) {
        disIt = NULL;
        disIteration = 0;
        
        lang = l;
        setSkeletonSearchPath( s );
        instrm_parser = 0;
    }

    /*
     * Initialize the CodeGenEmu. Loads the specification files given, and
     * does the pre-generation computations. Returns false if anything fails,
     * and emits an error message to stderr. This function must be called
     * before trying to do anything else with this class.
     */
    bool init( int argc, char **argv, bool disasmOnly );
    
    bool doDisasm, doEmu; /* Activity flags to selectively generate sections */
    string sledFilename, sslFilename;

    // instrumentation parser
    InstrmParser *instrm_parser;
    
  protected:
    /*
     * Determines the structures needed to hold the registers in the machine
     * we will be emulating. Sets registerStruct, getreg, and setreg.
     * Normally called from init.
     */
    virtual void computeRegisterStructure( );
    /*
     * If the machine contains any complex parameters to it's operations
     * (ie almost every machine), generate the code needed to compute them
     * at runtime. Sets decodeParamFuncs and decodeParamExpr. Normally
     * called from init.
     */
    virtual void computeDecodeParam( );

    /*
     * If we have any synthesized instructions, try to compute rtls
     * for them (if they don't already have some
     */
    virtual void computeSynths( );
    SemStr getSynthParamExpr( const string &name, InvokeExpr *invoke, int i,
                              list<int> &sslparams );
    
    /* Called by init to check the sanity of whoever wrote the specs */
    bool sanityCheck();
    
    /* Mandatory implementation for CodeGenApp Interface */
    virtual string getSubstitutionFor( const string & );
    virtual bool isActive( const string & );
    virtual string getSkelBasename( ) { return "emuskel"; }
    virtual string getHeader( const string &, bool );

    /* mangle SSL names such that they become legal C identifiers */
 public:
    string mangleName( const string &name, const char *prefix );
    string mangleReg( const string &name ) { return mangleName( name, "r" ); }
    string mangleParam( const string &name ) { return mangleName( name, "op" ); }
    string getSSLName( const string &sledname );

    /* Emulator generation functions */
    virtual string sprintFlagCalls( );
    virtual string sprintInstructions( );
    virtual string sprintExecuteFunc( ); 
    virtual string sprintExecuteOneFunc(  );

    /* Disassembler generation functions */
    virtual string sprintDisassembleParams( );
    virtual string sprintDisassembleFunc( );

    /* Register dumper generation */
    virtual string sprintDumpRegisters( );

  protected:
    /* Support routines for generating code from RTLs */
    virtual string sprintRTLCode( const HRTL &rtl );
    virtual string sprintAssignRT( RTAssgn *rta );
    virtual string sprintAssignRT( const SemStr &lhs, const SemStr &rhs,
                                   type_it &type );
    virtual string sprintSemStr( const SemStr &ss );
    virtual string sprintSemStr( const SemStr &ss, int asnsize );
    virtual string sprintSemStr( const SemStr &ss, const vector<Type> &type );
    virtual string sprintSemStr( const SemStr &ss, type_it &type );
    virtual string sprintSemStr( SSCIT &it, SSCIT &end, type_it &type );
    virtual string sprintOperationCall( const Type &type, int id,
                                        const string &lhs, const string &rhs );
    virtual string sprintRegisterAssign( SSCIT &in, SSCIT &end,
                                         const SemStr &rhs, type_it &type );
    virtual string sprintRegisterRead( SSCIT &in, SSCIT &end, type_it &type );
    virtual string sprintMemoryAssign( SSCIT &in, SSCIT &end,
                                       const SemStr &rhs, type_it &type );
    virtual string sprintMemoryRead( SSCIT &in, SSCIT &end, type_it &type );

    /* Support routines for computeRegisterStructure */
    virtual string sprintRegisterType( int num );
    virtual void setRegisterParent( int child, int offset, int size, int parent,
                                    RegTree &treedn, vector<int> &treeup );
    
    virtual param_t consSimpleRegister( const Type &type, const string &sslname,
                                        const string &prefix ); 
    virtual params_t consRegisterTree( SRP_IT &it, const SRP_IT &end,
                                       const Type &type, const string &prefix,
                                       int offset, int border );

    /* Support routine for computeDecodeParam
     * Generates code to decode a single complex (ie typed) operand.
     */
    virtual string sprintDecodeTypedParam( const string &name,
                                           const list<int> &opts,
                                           bool lhsparam );
    virtual void findLHSParams( );
    
    /* Support routines for disassembly */
    virtual string sprintDisassembleInst( const InstIdent &inst, bool callName=0 );
    virtual string sprintDisassembleTypedParam( const string &name,
                                                const ListExpr &list );

    virtual string sprintDisassembleTypedParams( const string &type,
                                                 map<string,bool> &types );
    /* Support routines for generating NJMC matching statements */
    virtual string sprintMatchStmt( const string &body, const string &other,
                                    bool returnNextPC );

  public:
    /* The SLED & SSL dictionaries, after parsing */
    SledDict sled;
    RTLInstDict ssl;

    /* The typename of the register structure, by default "RegisterFile" */
    string registerStructTypeName;
    /* The name of the register structure, by default "regs" */
    string registerStructName;
    
    /* Mapping from register name (ssl version) to get/set expressions. Set
     * expression needs to have %s replaced with the real value to assign
     */
    map<string,string> getreg, setreg; /* register, expr */
    /* Mapping from register parameter name to get/set expressions. %r needs
     * to be replaced with the register parameter, and %s with the assignment
     * value (for setidxreg).
     */
    map<string,string> getidxreg, setidxreg; /* register, expr */
    /* Mapping from parameter ids to an expression which decodes it */
    map<int, string> decodeParamExpr;
    
    /* Mapping from parameter ids to its stringified type */
    map<int, string> decodeParamType;

    /* Mapping from ssl names to sled names (they can differ slightly) */
    map<string,string> slednamemap;

    /* Code for the register structure body - generated by
     * computeRegisterStructure */
    string registerStruct;
    /* Code for functions which are used to get/set registers - generated
     * by computeRegisterStructure */
    string registerAccessFuncs;
    /* Code for functions which are used in parameter decoding - generated
     * by computeDecodeParam() */
    string decodeParamFuncs;

    /* Default type for instruction parameters */
    Type paramType;

    /* Default type for addresses */
    Type addressType;

    /* Default type used for addressing registers */
    Type registerIndexType;
    
    /* True if the source & target architectures differ in endianess */
    bool needByteSwap;

    /* Convert substitution names to id numbers for quicker (and more legible)
     * substitutions (see getSubstitutionFor & prepareSubstitutions) */
    map<string,int> substHash;

    // allows instrumentation
    string CodeGenEmu::sprintBodyCode(string, const HRTL&);
    
    /* The instruction we're currently working on, if any. Only
     * has any meaning within the context of an sprintInstructions
     */
    const string *currentInst;
    bool needNextPC;
};


#endif  /* !codegenemu_H */
