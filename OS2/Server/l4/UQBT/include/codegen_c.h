/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*=============================================================================
 * FILE:       codegen_c.h
 * OVERVIEW:   C language support class
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 *===========================================================================*/

/* $Revision: 1.8 $
 * 23 Apr 01 - Nathan: Initial version
 */

#ifndef codegen_c_H
#define codegen_c_H

#include "codegen.h"

/*
 * See codegen.h for descriptions of these methods
 */
class CodeGenC : public CodeGenLanguage {   
  public:
    virtual string sprintVariableDecl (const string &type, const string &name);
    virtual string sprintVariableDefn (const string &type, const string &name);
    virtual string sprintVariableDefn (const string &type, const string &name,
                                       const string &init );
    virtual string sprintArrayVarDefn (const string &type, const string &name,
                                       unsigned elems);
    virtual string sprintArrayVarDefn (const string &type, const string &name,
                                       unsigned elems, const string &init);
    virtual string sprintFunctionDecl (const string &type, const string &name,
                                       cparams_t &params );
    virtual string sprintFunctionDefn (const string &type, const string &name,
                                       cparams_t &params, const string &body);
    virtual string sprintFunctionCall (const string &name, cparams_t &params);
    virtual string sprintClosureDefn (const string &type, const string &name,
                                      cparams_t &params, cparams_t &funcParams,
                                      const string &body);
    virtual string sprintClosureCreate (const string &name, cparams_t &params);
    virtual string sprintClosureCall (const string &name, cparams_t &params);
    virtual string sprintClosureType (const string &type, const string &name,
                                      cparams_t &funcParams);
    virtual string sprintMacroDefn (const string &name, cparams_t &params,
                                    const string &body);
    virtual string sprintMacroCall (const string &name, cparams_t &params);
    virtual string sprintStructDefn (const string &name, cparams_t &params);
    virtual string sprintUnionDefn (const string &name, cparams_t &params);
    virtual string sprintComment (const string &comment);
    virtual string sprintAssign (const string &lhs, const string &rhs);
    virtual string sprintSwitch (const string &on, const string &opts,
                                 const string &def);
    virtual string sprintSwitchBranch (const string &value, const string &expr,
                                       bool needBreak = true );
    virtual string sprintReturn (const string &expr);
    virtual string sprintAddressOf (const string &expr);
    virtual string sprintDereference (const string &expr);
    virtual string sprintGetBitslice( const Type &baseType, const string &expr,
                                      int lo, int hi );
    virtual string sprintSetBitslice( const Type &baseType, const string &lhs,
                                      const string &rhs, int lo, int hi );
    virtual string sprintFormattedPrint( const string &fmt,
                                         const list<string> &params,
                                         const string &tgt );
    virtual string sprintType (const Type &type);

    /* Normal type cast as in C (ie performs conversions where needed) */
    virtual string sprintTypeCast (const Type &fromType, const Type &toType,
                                   const string &expr);
    /* Type change with no conversion (ie preserves bit pattern) */
    virtual string sprintTypeChange (const Type &fromType, const Type &toType,
                                     const string &expr);

    virtual string sprintLiteral(int value);
    virtual string sprintLiteral(long long value);
    virtual string sprintLiteral(double value);
    virtual string sprintLiteral(const string &value);
    virtual string sprintLiteral(const list<int> &arr);
    virtual string sprintLiteral(const list<string> &arr);
    virtual string sprintNames(const list<string> &arr);
    virtual string sprintConstStringList( const string &name,
                                          const list<string> &arr );
      
    virtual bool hasMacros() { return true; }
    virtual bool hasInterface() { return true; }
    virtual bool hasObjects() { return false; }
    virtual bool hasUnions() { return true; }
    virtual bool hasPointers() { return true; }
    virtual bool hasStrings() { return false; }
    virtual bool hasExactType( const Type &type );
    virtual Type getNearestType( const Type &type );

    virtual Type getMaxIntType() { return SIGNED(64); }
    virtual Type getMaxFloatType() { return FLOAT(128); }

    virtual string getImplExtension() { return ".c"; }
    virtual string getIfaceExtension() { return ".h"; }
    virtual string getLanguageName() { return "C"; }
    virtual string getStreamType() { return "FILE *"; }
    virtual codegenlang_t getLanguageID() { return LANG_C; }

  protected:
    virtual string sprintParamDeclList (cparams_t &params);
    virtual string sprintParamCallList (cparams_t &params);
    virtual string sprintRecordDefn (const string &form, const string &name,
                                     cparams_t &params);
};



#endif /* !codegen_c_H */
