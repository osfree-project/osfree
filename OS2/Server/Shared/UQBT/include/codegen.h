/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*=============================================================================
 * FILE:       codegen.h
 * OVERVIEW:   Generic code generator classes
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 *===========================================================================*/

/* $Revision: 1.10 $
 * 23 Apr 01 - Nathan: Initial version
 */

#ifndef codegen_H
#define codegen_H

#include <iostream.h>
#include <string>
#include <list>
#include "type.h"
#include "global.h"

/*
 * class param_t
 *
 * A simple class to hold a parameter (or more generally any type/variable
 * pair). The parameters are stored are a string pair. An additional value,
 * count, is used to hold the count for array variables, and is 0 for non-array
 * variables.
 *
 * Note:
 *    It would probably be cleaner to use a type class of some description,
 *    but we already have UQBT's Type to deal with, and it seemed better to
 *    retain that and construct other types via string-generating functions.
 *
 * Note 2:
 *    This may have been a mistake.
 */
class param_t {
  public:
    param_t() { count = 0; }
    param_t( const string &a, const string &b, int arr = 0) { 
      type = a; name = b; count = arr;
    }
    
    string type;
    string name;
    int count; /* array count, 0 = non-array var */
};

/*
 * class params_t
 *
 * A simple list of param_t, this class really only exists as a convenience
 * for the most common invocations involving parameters
 */
class params_t : public list<param_t> {
  public:
    params_t() : list<param_t>() {}
    params_t( const string &a, const string &b ) : list<param_t>() {
        push_back(a,b);
    }
    params_t( const string &a1, const string &b1,
              const string &a2, const string &b2 ) : list<param_t>() {
        push_back(a1,b1);
        push_back(a2,b2);
    }
    void push_back( const string &a, const string &b ) {
        param_t tmp; /* ugh */
        tmp.type = a;
        tmp.name = b;
        list<param_t>::push_back( tmp );
    }
    void push_back( const param_t &p ){ list<param_t>::push_back( p );}
    void push_back( const params_t &p ) {
        for( const_iterator i = p.begin(); i != p.end(); i++ )
            push_back(*i);
    }
};
typedef const params_t cparams_t;
typedef params_t::const_iterator cparams_it;

#define VOID_TYPE Type(TVOID,0,false)
#define BOOL_TYPE Type(BOOLEAN,1,false)
#define UNKNOWN_TYPE Type(UNKNOWN,0,false)
#define UNSIGNED(x) Type(INTEGER,x,false)
#define SIGNED(x) Type(INTEGER,x,true)
#define FLOAT(x) Type(FLOATP,x,true)

typedef enum { LANG_C, LANG_CPP, LANG_JAVA } codegenlang_t;

#define DEFAULT_SKEL_SEARCH_PATH "/usr/share/genemu/:$BINDIR/runtime:."

/*
 * class CodeGenLanguage
 *
 * Pure abstract base class for programming-language specific behaviour.
 * Specifies functions for generating various basic language constructs - note
 * that we assume the target language is procedural, or can be mapped onto
 * procedural semantics.
 */
class CodeGenLanguage {   
  public:
    /* Given a numeric language id (from codegenlang_t), returns an appropriate
     * subclass instance, or NULL if there is no subclass available.
     */
    static CodeGenLanguage *getInstanceFor( codegenlang_t lang );
    
/* Generate various language constructs - hopefully these are self-explanatory.
 * Note that except for calls and expressions, each method is expected to 
 * return a complete (ie properly terminated) statement
 */
    virtual string sprintVariableDecl (const string &type, const string &name) = 0;
    virtual string sprintVariableDefn (const string &type, const string &name) = 0;
    virtual string sprintVariableDefn (const string &type, const string &name,
                                       const string &init ) = 0;
    virtual string sprintArrayVarDefn (const string &type, const string &name,
                                       unsigned elems) = 0;
    virtual string sprintArrayVarDefn (const string &type, const string &name,
                                       unsigned elems, const string &init) = 0;
    virtual string sprintFunctionDecl (const string &type, const string &name,
                                       cparams_t &params ) = 0;
    virtual string sprintFunctionDefn (const string &type, const string &name,
                                       cparams_t &params, const string &body) = 0;
    virtual string sprintFunctionCall (const string &name, cparams_t &params) = 0;
    virtual string sprintClosureDefn (const string &type, const string &name,
                                      cparams_t &params, cparams_t &funcParams,
                                      const string &body) = 0;
    virtual string sprintClosureCreate (const string &name, cparams_t &params) = 0;
    virtual string sprintClosureCall (const string &name, cparams_t &params) = 0;
    virtual string sprintClosureType (const string &type, const string &name,
                                      cparams_t &funcParams) = 0;
    virtual string sprintMacroDefn (const string &name, cparams_t &params,
                                    const string &body) = 0;
    virtual string sprintMacroCall (const string &name, cparams_t &params) = 0;
    virtual string sprintStructDefn (const string &name, cparams_t &params) = 0;
    virtual string sprintUnionDefn (const string &name, cparams_t &params) = 0;
    virtual string sprintComment (const string &comment) = 0;
    virtual string sprintAssign (const string &lhs, const string &rhs) = 0;
    virtual string sprintSwitch (const string &on, const string &opts,
                                 const string &def) = 0;
    virtual string sprintReturn (const string &expr) = 0;
    virtual string sprintSwitchBranch (const string &value, const string &expr,
                                       bool needBreak = true ) = 0;
    virtual string sprintAddressOf( const string &expr ) = 0;
    virtual string sprintDereference (const string &expr) = 0;
    virtual string sprintGetBitslice( const Type &baseType, const string &expr,
                                      int lo, int hi ) = 0;
    virtual string sprintSetBitslice( const Type &baseType, const string &lhs,
                                      const string &rhs, int lo, int hi ) = 0;
    virtual string sprintFormattedPrint( const string &fmt,
                                         const list<string> &params,
                                         const string &tgt ) = 0;
    
    virtual string sprintType (const Type &type) = 0;

    /* Normal type cast as in C (ie performs conversions where needed) */
    virtual string sprintTypeCast (const Type &fromType, const Type &toType,
                                   const string &expr) = 0;

    /* Type change with no conversion (ie preserves bit pattern) */
    virtual string sprintTypeChange (const Type &fromType, const Type &toType,
                                     const string &expr) = 0;

/* Convenience functions (as their namesakes above, except given a Type) */
    virtual string sprintVariableDecl (const Type &type, const string &name) {
        return sprintVariableDecl( sprintType(type), name );
    }
    virtual string sprintVariableDefn (const Type &type, const string &name) {
        return sprintVariableDefn( sprintType(type), name );
    }
    virtual string sprintVariableDefn (const Type &type, const string &name,
                                       const string &init) {
        return sprintVariableDefn( sprintType(type), name, init );
    }
    virtual string sprintFunctionDecl (const Type &type, const string &name,
                                       cparams_t &params ) {
        return sprintFunctionDecl( sprintType(type), name, params );
    }        
    virtual string sprintFunctionDefn (const Type &type, const string &name,
                                       cparams_t &params, const string &body) {
        return sprintFunctionDefn( sprintType(type), name, params, body );
    }
    virtual string sprintSwitchBranch (int value, const string &expr,
                                       bool needBreak = true ) {
        return sprintSwitchBranch( sprintLiteral(value), expr, needBreak );
    }

    /* generate a representation of a literal value, of whatever form */
    virtual string sprintLiteral(int value) = 0;
    virtual string sprintLiteral(long long value) = 0;
    virtual string sprintLiteral(double value) = 0;
    virtual string sprintLiteral(const string &value) = 0;
    virtual string sprintLiteral(const list<int> &arr) = 0;
    virtual string sprintLiteral(const list<string> &arr) = 0;

    /* generate a string list as-is, ie { first, second }, as opposed to
     * a list of strings as sprintLiteral would do. It's uncertain whether
     * this is useful in languages that aren't C (or lisp for that matter).
     */
    virtual string sprintNames(const list<string> &arr) = 0;

    /* generate a variable declaration for a const list/array of strings,
     * ie 'const char *name[2] = { "first", "second" };
     */
    virtual string sprintConstStringList( const string &name,
                                          const list<string> &arr ) = 0;

/* Capability functions, used by the caller to determine whether the
 * language supports particular features
 */
    virtual bool hasMacros() = 0;  /* Does the language (internally) support a macro processor */
    virtual bool hasInterface() = 0; /* Does the language require separate interface files */
    virtual bool hasObjects() = 0; /* Is the language object-oriented */
    virtual bool hasUnions() = 0;  /* Does the language support unions */
    virtual bool hasPointers() = 0; /* Does it have general pointers */
    virtual bool hasStrings() = 0; /* has first class strings */

    virtual Type getMaxIntType() = 0;
    virtual Type getMaxFloatType() = 0;
    virtual Type getNearestType( const Type &type ) = 0;
    virtual bool hasExactType( const Type &type ) = 0;
    virtual bool isTypeSupported( const Type &type );
    
    virtual string getImplExtension() = 0;
    virtual string getIfaceExtension() = 0;
    virtual string getLanguageName() = 0;
    virtual string getStreamType() = 0;
    virtual codegenlang_t getLanguageID() = 0;
};

/*
 * class CodeGenApp
 *
 * Base class for application specific behaviour. The model assumed is the
 * output of a pair of files (implementation and interface), derived from
 * a related pair of skeleton files into which the computed semantics are
 * substituted. If necessary a macro processor may be run on the generated
 * files.
 */
class CodeGenApp {
  public:
    virtual ~CodeGenApp() { }  /* Supplied to make gcc happy */
    
    /* filenames to generate, implementation and interface respectively */
    string implFilename, ifaceFilename;

    /* name prefix. The normal use of this is to prepend it directly to any
     * external names in the generated code.
     */
    string prefix;
    

    /* given a colon-separated list of paths, add each path to the list of
     * directories to search for skeleton files (searchPaths). The special
     * value $BINPATH will be replaced with the path of the current executable
     * file
     */
    void addSkeletonSearchPath( const string &path );

    /* as addSkeletonSearchPath, but sets searchPaths to exactly the paths 
     * given (ie it clears searchPaths first).
     */
    void setSkeletonSearchPath( const string &path );

    /* generate the implementation file. Returns true on sucess */
    virtual bool generateImplFile( );

    /* generate the interface file.
     * Note that if the current language doesn't actually _have_ interface
     * files, this function will silently do nothing whatsoever.
     * Returns true on success.
     */
    virtual bool generateIfaceFile( );
    
    /* ordered list of directories in which to search for skeleton files. */
    list<string> searchPaths;

    /* language in which to generate code */
    CodeGenLanguage *lang;

  protected:
/* SubclassResponsibility */
    /*
     * getSubstitutionFor will be called for each keyword in the skeleton file
     * of the form $KEYWORD or ${KEYWORD} with the keyword string (ie
     * "KEYWORD"), and is expected to return a string which will take the
     * place of the keyword. Note: this method should not assume that it will
     * necessarily be called more than once for the same keyword name.
     */
    virtual string getSubstitutionFor( const string &str ) = 0;
    
    /*
     * Called whenever the skeleton processor encounters a conditional block,
     * with the name of the section. If this method returns false, the section
     * will be skipped, otherwise processing proceeds as normal.
     */
    virtual bool isActive( const string &str ) = 0;
    
    /*
     * Return the base filename (sans extension) of the skeleton file(s). 
     * CodeGenApp will then look in the search path for basename + language
     * file extension.
     */
    virtual string getSkelBasename( ) = 0;
    /*
     * Return a comment to stick at the top of generated files, usually giving
     * generation date, source, etc. Parameters are the name of the output file,
     * and a boolean flag isImpl, which is true for an implementation file and
     * false for interface files.
     */
    virtual string getHeader( const string &filename, bool isImpl ) = 0;
    
/* Generic helper methods  */
    /* Locate a file with the given name by scanning through searchPaths, and
     * returning the first (readable) match. If the file cannot be found, emits
     * an error message and returns the empty string
     */
    string findSkeletonFile( const string &name );

    /* Given a pathname, return just the file portion */
    string getFilename( const string &pathname );

    /* Similar to above, but also strip off the extension if any */
    string getBasename( const string &pathname );

    /* Return a (possibly relative) path to this executable */
    string getBinaryPath( );

    /* read a file in from the given istream, perform substitutions, and write
     * to the ostream.
     */
    bool processSkeleton( ostream &out, istream &skel );
};


/* Miscellaneous routines */
class OperandIdent;
string sprintMatchLine( const string &name, const list<int> &params,
                        const string &body );
string sprintMatchLine( const string &name,
                        const vector<OperandIdent *> &params,
                        const string &body );


#endif /* !codegen_H */
