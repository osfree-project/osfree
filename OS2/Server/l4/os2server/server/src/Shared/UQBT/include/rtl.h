/*
 * Copyright (C) 1997, Shane Sendall
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       rtl.h
 * OVERVIEW:   Provides the implementation for the RT class and its
 *             subclasses. Additional classes that are required
 *             for parsing the spec file but are not to be made available to
 *             the end user are #included.
 *
 * Copyright (C) 1997, Shane Sendall
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 *============================================================================*/

/* $Revision: 1.66 $
 * Dec 1997 - Shane: Created
 * Jun 1998 - Mike: Total reorganisation
 * Sep 1998 - Mike: Added some sparc specific functions (implemented in
 * ctisparc.cc)
 * 27 Oct 98 - Mike: ReadSSLFile() takes a bPrint now
 * 02 Nov 98 - Mike: Added I_COMPCALL for computed calls (register call instr's)
 * 03 Dec 98 - Mike: Added RTCall class
 * 06 Jan 99 - Mike: RTL() returns a reference now
 * 07 Jan 99 - Mike: Added RTlist::DeepCopyList() for memory problems
 * 13 Jan 99 - Ian: added map<int, Register> to RTLInst class
 * 20 Jan 99 - Mike: Removed class RTCall; added classes such as HLCall
 * 27 Jan 99 - Mike: Use COMPJUMP and COMPCALL now
 * 29 Jan 99 - Mike: Added GetFallthrough()
 * 04 Feb 99 - Mike: HLCall etc have print() now
 * 08 Feb 99 - Doug: Commented out unused (I think) constructors for HLCall and
 *                   modified remaining one to set new field (size of return
 *                   type)
 *                   Added HLRet class.
 * 11 Feb 99 - Mike: Added HLComp, changed HLJcond to hold iKind and bFloat
 * 22 Feb 99 - Mike: DeepCopyList() copies to a reference parameter now
 * 11 Mar 99 - Mike: Added RTlist::DeleteRT()
 * 15 Mar 99 - Doug: Merged in intersection of machine dependent subclasses
 *                   (RTlistSparc and RTlist386)
 * 19 Mar 99 - Mike: Added RTFlagCall copy constructor
 * 24 Mar 99 - Mike: Added HLJCond::SetFloat() and MakeSigned()
 * 30 Mar 99 - David: added DefMap and AliasMap to RTLInstDict
 * 12 Apr 99 - Doug: Removed RTlist::IsRestore and replaced with
 *                   HLCall::IsFollowedByRestore
 * 27 Apr 99 - Doug: Removed declarations for GetControlTransfer, IsComputedCall
 *                   Added AddDefinedLocations
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 24 Jun 99 - Mike: Added HLScond class
 * 15 Mar 00 - Cristina: RT::setAFP, RTassign::setAFP, RTFlagCalls::setAFP 
 *                     and RTList::setAFP transformed to setAXP
 * 30 Mar 00 - Mike: All searchAndReplace type functions take an optional
 *                     typeSens bool now
 * 07 Sep 00 - Mike: getUseDefLocations takes a Proc* now, so we can register
 *              the use of the return location
 * 12 Sep 00 - Mike: Added RTAssgn::updateType()
 * 14 Sep 00 - Mike: Added non const HLCall::getParams(), so we can modify
 *              the parameters in CSR::matchCallParams()
 * 27 Nov 00 - Mike: Added fastMap to RTLInstDict for the @FAST@ instructions
 * 10 Feb 01 - Nathan: Added SpecialRegMap to RTLInstDict to record details for
 *              the "special" (non-addressable) registers
 * 10 Feb 01 - Nathan: Added ParamMap & DetParamMap to RTLInstDict to keep
 *              track of the operands used in the SSL
 * 16 Feb 01 - Nathan: Added fetchExecuteCycle to RTLInstDict
 * 05 Mar 01 - Simon: Removed class RTCond, removed expand functions; moved
 *              class TableEntry here
 * 06 Mar 01 - Mike: Moved insertAssign as member function here; added getSize
 *              to get a guess for the size of this RTL
 * 20 Mar 01 - Mike: Added clone() methods to RT derived classes; also
 *              added insertAfterTemps()
 * 11 Apr 01 - Mike: added HLJump::getUseDefLocations()
 * 23 Apr 01 - Nathan: added prototypes for decideType and partialType
 * 24 Apr 01 - Nathan: added ParamEntry for DetParamMap in place of TableEntry
 * 01 May 01 - Nathan: split RTLInstDict::instantiateRTL into two pieces
 * 27 Jun 01 - Nathan: added searchAll to all RT classes
 * 27 Jun 01 - Nathan: modifications for lambda support - minor adjustments
 *              to ParamEntry and added fixupParams & friend to RTLInstDict
 * 26 Jul 01 - Mike: Added getGuard(), addGuard() to class RTAssgn
 * 30 Jul 01 - Brian: Moved RTlist (renamed to HRTL) and derived classes to
 *              new header file hrtl.h. Slight reformatting. Removed unused
 *              writeRT() methods.
 * 31 Jul 01 - Brian: Moved RTLInstDict, TableEntry, and ParamEntry classes to
 *              hrtl.h.
 * 08 Aug 01 - Mike: Added simplifyGuard();
 * 13 Aug 01 - Bernard: Added support for type analysis
 */

#ifndef __RTL_H__
#define __RTL_H__

#include "reg.h"
#include "ss.h"
#include "bitset.h"            // For BitSet (= BITSET)

// HACK: The below should be read from a spec file
#define STD_SIZE 32            // 32 bits is a standard size

// For type analysis
#include "typeLex.h"

//**************************************************************
// The following describes the RT class and its derived classes
//**************************************************************

/*==============================================================================
 * RT is the abstract base class from which various types of RT's are derived.
 *============================================================================*/
class RT {
public:
    RT(RT_KIND t);
    virtual ~RT();

    RT_KIND getKind() const;    // return the kind of the RT

    // Given a map from registers to expressions, replace each use of a reg
    // with the corresponding expression in the map. Then for every
    // definition of such a reg, update the map with the new RHS resulting
    // from the first substitution.
    virtual bool subAXP(map<SemStr,SemStr>& subMap) { return false; }

    // Replace instances of "search" with "replace". Optionally type sensitive.
    virtual void searchAndReplace(SemStr& search, const SemStr& replace,
                                  bool typeSens = false) {}

    // Searches for all instances of a given subexpression within this
    // expression and adds them to a given list in reverse nesting order.
    // If typeSens is set, then type is considered during the match
    virtual bool searchAll(SemStr &search, list<SemStr*> &result,
                           bool typeSens = false) { return false; }
        
    virtual void print(ostream& os = cout) const = 0;

    // Make a clone of this RT object. Each new RT derived class has to
    // implement this function, which makes a new copy of itself.
    // The copy can be deleted without affecting the original object
    virtual RT* clone() = 0;
    
    
    // This function store type information which we gather from the
    // individual RTs (through the use of the typeLex and typeAnalysis
    // classes) into the BBBlock inBlock.
    virtual void storeUseDefineStruct(BBBlock& inBlock, 
                                      ADDRESS memAddress, 
				      int counter) = 0; 


private:
    RT_KIND kind;        // the kind of the RT
};


/*==============================================================================
 * RTAssgn is an assignment register transfer (i.e. location := expression).
 *============================================================================*/
class RTAssgn: public RT {
public:
    RTAssgn();
    RTAssgn(SemStr* l, SemStr* r, SemStr *c, Byte s); // with guard condition.
    RTAssgn(SemStr* l, SemStr* r, Byte s);

    virtual ~RTAssgn();

    RT* clone();

    // Comparison operator
    bool operator==(const RTAssgn &other) const;

    void updateLHS(SemStr* l);
    void updateRHS(SemStr* r);

    SemStr* getLHS() const;
    SemStr* getRHS() const;

    // Add a guard. Makes a copy of the given SemStr. ANDs the condition
    // with any existing condition, if needed.
    void addGuard(const SemStr* cond);

    // Return the assignment's guard, or NULL if none.
    const SemStr* getGuard() { return pCOND; }

    // Simplify the guard expression. It returns true if the whole RT can be
    // deleted
    bool simplifyGuard();

    // Change size of assignment in bits
    void updateSize(Byte s);

    // Return size of assignment transfer. Deprecated; use getType()
    Byte getSize() const;

    // Update type of the assignment (updates the type of both left and
    // right hand sides)
    void updateType(const Type& t);

    // Return the type of the assignment (= type of the expression for the LHS)
    const Type& getType() const;

    // Check if special reg was defined.
    //bool isSpRegDefined(int idReg) const;    

    // Check if numbered reg was def'd.
    bool isNumRegDefined(int idReg) const;

    // Check if special register was used.
    //bool isSpRegUsed(int idReg) const;

    // Check if numbered register was used.
    bool isNumRegUsed(int idReg) const;

    // Given a map from registers to expressions, replace each use of a reg
    // with the corresponding expression in the map. Then for every
    // definition of such a reg, update the map with the new RHS resulting
    // from the first substitution.
    bool subAXP(map<SemStr,SemStr>& subMap);

    // Replace instances of "search" with "replace". Optionally type sensitive.
    void searchAndReplace(SemStr& search, const SemStr& replace,
                          bool typesens = false);

    // Searches for all instances of a given subexpression within this
    // expression and adds them to a given list in reverse nesting order.
    // If typeSens is set, then type is considered during the match
    virtual bool searchAll(SemStr &search, list<SemStr*> &result,
                           bool typeSens = false);
        
    void print(ostream& os = cout) const;

    // Refer to the same function in class RT for info	    
    void storeUseDefineStruct(BBBlock& inBlock, ADDRESS memAddress, int counter); 

public:    
    Byte added;                 // True if this assignment is added, not part
                                // of the original code.
private:    
    SemStr* pLHS;
    SemStr* pRHS;
    SemStr* pCOND;              // Conditional part of a conditional transfer
                                // (guard). NULL if none.
    Byte size;                  // Size of register transfer in bits. Redundant
                                // now; one day will be the size of the LHS.
};


/*==============================================================================
 * An RTFlagDef is an RT that defines a flag function (cf RTFlagCall which
 * calls a flag function).
 *============================================================================*/
class RTFlagDef: public RT {
public:
    RTFlagDef(const string& fname);
    virtual ~RTFlagDef();

    RT* clone();

    void print(ostream& os = cout) const;
    
    // Refer to the same function in class RT for info
    void storeUseDefineStruct(BBBlock& inBlock, ADDRESS memAddress, int counter);     

public:
    list<int> params;           // List of parameters
    HRTL* flag_rtl;
    string func_name;

private:
    // Determines whether this statement defines or undefines the given
    // condition codes.
//    FLAGOP_TYPE ftype;

    // The condition codes that are un/defined.
//    list<SpecialReg*> conds;
};


/*==============================================================================
 * An RTFlagCall is an RT that calls a flag function. It mainly has the
 * name of the function, and the parameters (as a list of SemStrs,
 * representing expressions for the actual parameters).
 *============================================================================*/
class RTFlagCall: public RT {
public:

    RTFlagCall(const string& fname);
    RTFlagCall(const RTFlagCall& other);
    virtual ~RTFlagCall();

    RT* clone();

    void print(ostream& os = cout) const;
    
    // Replace the actual parameters with a symbolic equivalent, e.g.
    // all r[8] -> v[0]
    void searchAndReplace(SemStr& search, const SemStr& replace,
                          bool typeSens = false);

    // Searches for all instances of a given subexpression within this
    // expression and adds them to a given list in reverse nesting order.
    // If typeSens is set, then type is considered during the match
    virtual bool searchAll(SemStr &search, list<SemStr*> &result,
                           bool typeSens = false);

    // Given a map from registers to expressions, replace each use of a reg
    // with the corresponding expression in the map. Then for every
    // definition of such a reg, update the map with the new RHS resulting
    // from the first substitution.
    bool subAXP(map<SemStr,SemStr>& subMap);
    
    // Refer to the same function in class RT for info
    void storeUseDefineStruct(BBBlock& inBlock, ADDRESS memAddress, int counter);            

public:
    list<SemStr*> actuals;      // List of actual parameters.
    string func_name;           // Name of the function (e.g. SUBFLAGS32)
};

#endif // __RTL_H__
