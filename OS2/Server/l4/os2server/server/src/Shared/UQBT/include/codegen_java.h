/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*=============================================================================
 * FILE:       codegen_java.h
 * OVERVIEW:   Java language support class
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 *===========================================================================*/

/* $Revision: 1.7 $
 * 23 Apr 01 - Nathan: Initial version
 */

#ifndef codegen_java_H
#define codegen_java_H

#include "codegen.h"
#include "codegen_c.h"

class CodeGenJava : public CodeGenC {   
  public:
    virtual string sprintVariableDefn (const string &type, const string &name);
    virtual string sprintArrayVarDefn (const string &type, const string &name,
                                       unsigned elems);
    virtual string sprintFunctionDefn (const string &type, const string &name,
                                       cparams_t &params, const string &body);
    virtual string sprintStructDefn (const string &name, cparams_t &params);
    virtual string sprintClosureDefn (const string &type, const string &name,
                                      cparams_t &params, cparams_t &funcParams,
                                      const string &body);
    virtual string sprintClosureCreate (const string &name, cparams_t &params);
    virtual string sprintClosureCall (const string &name, cparams_t &params);
    virtual string sprintClosureType (const string &type, const string &name,
                                      cparams_t &funcParams);
    virtual string sprintType (const Type &type);
    virtual string sprintTypeCast (const Type &fromType, const Type &toType,
                                   const string &expr);
    virtual string sprintTypeChange (const Type &fromType, const Type &toType,
                                     const string &expr);
    virtual string sprintGetBitslice( const Type &baseType, const string &expr,
                                      int lo, int hi );
    virtual string sprintSetBitslice( const Type &baseType, const string &lhs,
                                      const string &rhs, int lo, int hi );
    virtual string sprintConstStringList( const string &name,
                                          const list<string> &arr );
    virtual string sprintFormattedPrint( const string &fmt,
                                         const list<string> &params,
                                         const string &tgt );
    
    virtual bool hasMacros() { return false; }
    virtual bool hasInterface() { return false; }
    virtual bool hasObjects() { return true; }
    virtual bool hasUnions() { return false; }
    virtual bool hasPointers() { return false; }
    virtual bool hasStrings() { return true; }
    virtual bool hasExactType( const Type &type );
    virtual Type getNearestType( const Type &type );

    virtual Type getMaxIntType() { return SIGNED(64); }
    virtual Type getMaxFloatType() { return FLOAT(64); }

    virtual string getImplExtension() { return ".java"; }
    virtual string getIfaceExtension() { return ""; }
    virtual string getLanguageName() { return "Java"; }
    virtual string getStreamType() { return "PrintWriter"; }
    virtual codegenlang_t getLanguageID() { return LANG_JAVA; }

  protected:
    set<string> closureTypes;
};



#endif /* !codegen_java_H */
