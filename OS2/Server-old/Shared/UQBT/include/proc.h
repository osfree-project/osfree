/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       proc.h
 * OVERVIEW:   Interface for the procedure classes, which are used to
 *             store information about variables in the procedure such
 *             as parameters and locals.
 *
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 *============================================================================*/

/* $Revision: 1.59 $
 * 11 Mar 98 - Cristina  
 *  replaced BOOL for bool type (C++'s), same for TRUE and FALSE.
 * 18 Mar 98 - Cristina 
 *  added procedure iterator typedef (PROC_IT), and print() function.
 *  added GetCFG() function and changed the member Cfg to be a non pointer.
 * 24 Jul 98 - Mike
 *  Directly #include ../CFG/cfg.h now
 * 15 Dec 98 - Mike: Coverage functions
 * 16 Dec 98 - Mike: GetCFG() returns a pointer now
 * 08 Jan 99 - Mike: cfg is a pointer now, so can delete the whole CFG
 * 27 Jan 99 - Mike: Added m_bDecoded (in place of m_bIncomplete)
 * 19 Jan 99 - Mike: Removed path to "cfg.h"
 * 07 Apr 99 - Mike: Mods for HEADERS
 * 27 Apr 99 - Mike: Mods to some parameter stuff; added SetLibParams()
 * 29 Apr 99 - Mike: Added m_bDecoded and m_bDecoding, etc
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 15 Mar 00 - Cristina: UserProc::setAFP transformed to setAXP
 * 29 Mar 00 - Mike: Removed Type class
 * 14 Jun 00 - Mike: Changes for new Coverage system
 * 21 Jun 00 - Mike: Removed setTailCaller
 * 30 Aug 00 - Sameer/Brian: Added UserProc::getFirstLocalIndex() and
 *              UserProc::getLastLocalIndex() to return the index of the first
 *              and last symbolic local of each procedure.
 * 15 Sep 00 - Mike: matchParams takes a ReturnLocations parameter now
 * 18 Sep 00 - Mike: Removed the last parametrer to matchParams; outgoing is
 *              a reference now (was pointer)
 *              setReturnType() functions now return a bool
 * 20 Nov 00 - Mike: Added getVarType()
 * 19 Dec 00 - Mike: Added checkReturnPass() and checkReturnPassBB()
 * 21 Dec 00 - Mike: Removed replaceParentStackAddresses()
 * 23 Feb 01 - Mike: Added checkMemSizes and findVarEntry for "ninths" test.
 *              Also, symbolMap is totally type insensitive now
 * 13 Aug 01 - Bernard: Added support for type analysis
 * 30 Aug 01 - Mike: Proc's parameters changed from vector to list
 *              Also, findVarEntry returns SemStr* now.
 * 20 Sep 01 - Brian: Added getSymbolicLocals() to return the list of symbolic
 *              locals for a procedure.
*/

#ifndef _PROC_H_
#define _PROC_H_

#include "bitset.h"             // For BitSet (= BITSET)
#include "coverage.h"           // For Coverage class
#include "ss.h"

// For Type Analysis
class BBBlock;

/*==============================================================================
 * Procedure class.
 *============================================================================*/
class Proc {
public:

    /**
     * Constructor with name, native address and optional bLib.
     */
    Proc(const string& name, ADDRESS uNative);

    virtual ~Proc();

    /**
     * Gets name of this procedure.
     */
    const char* getName() const;

    /**
     * Get the native address.
     */
    ADDRESS getNativeAddress() const;

    /**
     * Returns the number of parameters this procedure takes.
     */
    int getNumArgs();

    /**
     * Prints this procedure to an output stream.
     */
    virtual ostream& put(ostream& os) const = 0;

    /*
     * Return the coverage of this procedure in bytes.
     */
    virtual unsigned getCoverage() const = 0;

    /**
     * Return the n'th parameter.
     */
    const SemStr* getParameter(unsigned int n) const;

    /*
     * Return true if given location is a parameter
     */
    bool isParam(const SemStr& loc) const;

    /**
     * Recover the return type for a user procedure or just ensure
     * that a library  procedure returns the expected type.
     */
    virtual bool setReturnType(const SemStr& retLoc,
        const ReturnLocations& retSpec) = 0;

    /*
     * Modify actuals so that it is now the list of locations that must
     * be passed to this procedure. The modification will be to either add
     * dummp locations to actuals, delete from actuals, or leave it
     * unchanged.
     * Add "dummy" params: this will be required when there are
     *   less live outs at a call site than the number of parameters
     *   expected by the procedure called. This will be a result of
     *   one of two things:
     *   i) a value returned by a preceeding call is used as a
     *      parameter and as such is not detected as defined by the
     *      procedure. E.g.:
     *
     *         foo(bar(x));
     *
     *      Here, the result of bar(x) is used as the first and only
     *      parameter to foo. On some architectures (such as SPARC),
     *      the location used as the first parameter (e.g. %o0) is
     *      also the location in which a value is returned. So, the
     *      call to bar defines this location implicitly as shown in
     *      the following SPARC assembly that may be generated by from
     *      the above code:
     *
     *          mov   x, %o0
     *          call  bar
     *          nop
     *          call  foo
     *
     *     As can be seen, there is no definition of %o0 after the
     *     call to bar and before the call to foo. Adding the integer
     *     return location is therefore a good guess for the dummy
     *     location to add (but may occasionally be wrong).
     *
     *  ii) uninitialised variables are used as parameters to a call
     *
     *  Note that both of these situations can only occur on
     *  architectures such as SPARC that use registers for parameter
     *  passing. Stack parameters must always be pushed so that the
     *  callee doesn't access the caller's non-parameter portion of
     *  stack.
     *
     * This used to be a virtual function, implemented differenty for
     * LibProcs and for UserProcs. But in fact, both need the exact same
     * treatment; the only difference is how the local member "parameters"
     * is set (from common.hs in the case of LibProc objects, or from analysis
     * in the case of UserProcs).
     */
    void matchParams(list<SemStr>& actuals, UserProc& caller,
        const Parameters& outgoing) const;

    /*
     * Get a list of types to cast a given list of actual parameters to
     */
    list<Type>* Proc::getParamTypeList(const list<SemStr>& actuals);

    /*
     * Get the formal parameters of this procedure
     */
    const list<SemStr>& getParams() const;

    /* 
     * Used by type analysis to store params in 
     * a separate list data structure (regParams)
     */
    void storeParams(BBBlock& inBlock);

    /**
     * Print the parameters of this procedure, as "v[1]" etc
     */
    void printParams(ostream& os) const;

    /**
     * Print the parameters of this procedure, as "int v1" etc.
     */
    void printParamsAsC(ostream& os) const;

    /*
     * Print the return type of this procedure, as "int" etc.
     */
    void printReturnTypeAsC(ostream& os) const;

    /**
     * Get the return type of this procedure, as a Type object
     */
    const Type& getReturnType() const;

    /*
     * Get the return location of this procedure, as a SemStr*
     */
    const SemStr* getReturnLoc() const;

    /*
     * Return true if this is a library proc
     */
    virtual bool isLib() const {return false;}

    /*
     * Return true if the aggregate pointer is used.
     * It is assumed that this is false for library procs
     */
    virtual bool isAggregateUsed() const {return false;}

    /**
     * Output operator for a Proc object.
     */
    friend ostream& operator<<(ostream& os, const Proc& proc);
    
    /*
     * Stores a list of basic blocks that end with a RET
     */
    list<BBBlock*> basicBlocksEndingWithRet;
    
    /*
     * Stores a list of reg to the procedure used for 
     * type propagation
     */
    list<int> regParams;

protected:

    /**
     * Procedure's name.
     */
    string name;

    /**
     * Procedure's address.
     */
    unsigned address;

    /*
     * The formal parameters of this procedure. This information is determined
     * either by the common.hs file (for a library function) or by CSR analysis.
     */
    list<SemStr> parameters;

    /**
     * The return location for this procedure, e.g. v2 or r[8]. Empty by default
     */
    SemStr returnLocn;

}; 

/*==============================================================================
 * LibProc class.
 *============================================================================*/
class LibProc : public Proc {
public:

    LibProc(const string& name, ADDRESS address);

    /*
     * Return the coverage of this procedure in bytes.
     */
    unsigned getCoverage() const { return 0; }

#if 0
    /**
     * See comment for Proc::matchParams.
     */
    void matchParams(list<SemStr>& actuals, UserProc& caller,
        const Parameters* outgoing, const SemStr* intRetLoc) const;
#endif

    /**
     * Ensure that this procedure returns the expected type.
     */
    bool setReturnType(const SemStr& retLoc, const ReturnLocations& retSpec);

    /*
     * Return true, since is a library proc
     */
    bool isLib() const {return true;}

    /*
     * Return true if the aggregate pointer is used.
     * It is assumed that this is false for library procs
     */
    virtual bool isAggregateUsed() const {return false;}

    /**
     * Prints this procedure to an output stream.
     */
    ostream& put(ostream& os) const;

};

/*==============================================================================
 * UserProc class.
 *============================================================================*/
class UserProc : public Proc {
public:

    UserProc(const string& name, ADDRESS address);

    /**
     * Records that this procedure has been decoded.
     */
    void setDecoded();

    /**
     * Returns a pointer to the CFG.
     */
    Cfg* getCFG() const;

    /**
     * Deletes the whole CFG and all the RTLs, RTs, and SemStrs associated with
     * it. Also nulls the internal cfg pointer (to prevent strange errors)
     */
    void deleteCFG();

    /**
     * Returns whether or not this procedure can be decoded (i.e. has
     * it already been decoded).
     */
    bool isDecoded() const;

    /**
     * Return the number of bytes allocated for locals on the stack.
     */
    int getLocalsSize() const;

    /*
     * Get the type of the given var
     */
    Type getVarType(int idx);

    /*
     * Set the size of the given var
     */
    void setVarSize(int idx, int size);

private:
    /*
     * Find a pointer to the SemStr representing the given var
     * Used by the above 2
     */
    SemStr* findVarEntry(int idx);

    /*
     * A special pass to check the sizes of memory that is about to be converted
     * into a var, ensuring that the largest size used in the proc is used for
     * all references (and it's declared that size)
     */
    void    checkMemSizes();

    /*
     * Implement the above for one given SemStr
     */
    void    checkMemSize(SemStr* ss);

public:

    /*
     * Return the coverage of this procedure in bytes.
     */
    unsigned getCoverage() const {return cover.totalCover();}

    /**
     * Set the prologue of this procedure.
     */
    void setPrologue(CalleePrologue* prologue);

    /**
     * Set the epilogue of this procedure.
     */
    void setEpilogue(CalleeEpilogue* epilogue);

    /**
     * Get the prologue (if any) of this procedure.
     */
    CalleePrologue* getPrologue();

    /**
     * Get the epilogue (if any) of this procedure.
     */
    CalleeEpilogue* getEpilogue();

    /**
     * Given a map from registers to expressions, follow the control
     * flow of the CFG replacing every use of a register in this map
     * with the corresponding expression. Then for every definition of
     * such a register, update its expression to be the RHS of the
     * definition after the first type of substitution has been
     * performed and remove that definition from the CFG. E.g.
     *
     *   map: { %esp -> %afp, %ebp -> %afp - 10 }
     *
     *  before subs: %esp    = %esp - 4
     *               m[%esp] = %ebx
     *               %esi    = m[%ebp + 8]
     *
     *  after subs:  m[%afp - 4] = %ebx
     *               %esi        = m[%afp + 2]
     */
    void subAXP(map<SemStr,SemStr>& subMap);

    /**
     * Sets the parameters that have been recovered for this procedure through
     * CSR analysis.
     */
    void setParams(const list<SemStr>& params, bool aggUsed = false);

    /*
     * Gets the parameters that have been recovered for this procedure through
     * CSR analysis to a set of non symbolic SemStrs
     */
    setTiSemStr& getParamSet() const;

#if 0
    /*
     * See comment for Proc::matchParams.
     */
    void matchParams(list<SemStr>& actuals, UserProc& caller,
        const Parameters* outgoing, const SemStr* intRetLoc) const;
#endif
    /**
     * Given the type of a location that has been
     * determined as holding a value returned by this
     * procedure, verify this against the type this
     * procedure currently thinks it returns. If this is
     * the first time this method has been called, then
     * the return type of this procedure is set to be the
     * given type and the location used for returning that
     * type is determined to be the return location of
     * this procedure. Otherwise, simply ensure that the
     * given type matches the already established return
     * type, emitting an error message if it isn't.
     */
    bool setReturnType(const SemStr& retLoc, const ReturnLocations& retSpec);

    /*
     * Set the return type for this procedure.
     */
    void doSetReturnType(const SemStr& retLoc, const ReturnLocations& retSpec);

    /*
     * Given a machine dependent location, return a generated symbolic
     * representation for it.
     */
    void toSymbolic(const SemStr& loc, SemStr& result, bool local = true);

    /*
     * Return the next available local variable; make it the given type
     */
    SemStr* newLocal(const Type& type);

    /**
     * Print the locals declaration in C style.
     */
    void printLocalsAsC(ostream& os) const;

    /**
     * Return the index of the first symbolic local for the procedure.
     */
    int getFirstLocalIndex();

    /**
     * Return the index of the last symbolic local for the procedure.
     */
    int getLastLocalIndex();
    
    /*
     * Return the list of symbolic locals for the procedure.
     */
    const vector<SemStr>& getSymbolicLocals() const;
    
    /**
     * Replace each instance of a location in this procedure with its symbolic
     * representation if it has one.
     */
    void propagateSymbolics();

    /*
     * Get the BB that is the entry point (not always the first BB)
     */
    PBB getEntryBB() const;

    /**
     * Prints this procedure to an output stream.
     */
    ostream& put(ostream& os) const;

    /*
     * Set the entry BB for this procedure (constructor has the entry address)
     */
    void setEntryBB();

    /*
     * Get the callees
     */
    const set<const Proc*>& getCallees() const;

    /*
     * Add to the set of callees
     */
    void setCallee(const Proc* callee); 

    /*
     * Add (st, fi) to the set of ranges covered in this procedure
     */
    void addRange(ADDRESS st, ADDRESS fi) {cover.addRange(st, fi);}

    /*
     * Add all the ranges in other to the set of ranges covered this procedure
     */
    void addRanges(const Coverage& other) {cover.addRanges(other);}

    /*
     * Print the coverage for this procedure
     */
    void printCoverage(ostream& os = cout) {cover.print(os); }

    /*
     * Get the first gap (between ranges) for this Coverage object
     */
    bool    getFirstGap(ADDRESS& a1, ADDRESS& a2, COV_CIT& it) const
                {return cover.getFirstGap(a1, a2, it);}

    /*
     * Get the next gap (between ranges) for this Coverage object
     */
    bool    getNextGap(ADDRESS& a1, ADDRESS& a2, COV_CIT& it) const
                {return cover.getNextGap(a1, a2, it);}

    /*
     * Add this proc's range to the program's coverage
     */
    void    addProcCoverage() const;

    /*
     * Check if this return location is "passed through" this function to one
     * of its callees. For example in the returncallee test, main uses the
     * return value from add4, and this use is "passed on" to add2, since
     * add4 doesn't define the return location after the call to add4
     */
    void checkReturnPass(int returnLocBit, const SemStr& returnLoc,
        const ReturnLocations& retLocations);

    /*
     * Do the main work for the above
     */
    void checkReturnPassBB(const PBB pBB, HLCall* pCall, int returnLocBit,
        const SemStr& returnLoc, const ReturnLocations& retLocations,
        set<PBB>& seen);

    /*
     * Return true if this proc uses the special aggregate pointer as the
     * first parameter
     */
    virtual bool isAggregateUsed() const {return aggregateUsed;}
 

private:

    /**
     * The control flow graph.
     */
    Cfg* cfg;

    /**
     * True if this procedure has been decoded.
     */
    bool decoded;

    /**
     * Indicates whether or not a non-default return type has been
     * determined for this procedure.
     */
    bool returnIsSet;

    /*
     * Indicate that the procedure has had its variables converted to
     * symbolic form, e.g. r[8]->v2. This is only done once, by a call to
     * propagateSymbolics(). We need to know that this has happened if we
     * later determine a different return location, and it happens not to
     * have been converted to symbolic as yet 
     */
    bool isSymbolic;

    /**
     * Used to generate unique IDs for the parameters and locals to
     * calls that are recovered and given a symbolic name.
     */
    unsigned uniqueID;

    /*
     * Indicate that the aggregate location pointer "hidden" parameter is used,
     * and is thus explicit in this translation. Needed only by architectures
     * like Sparc where a special parent stack location is used to pass the
     * address of aggregates. Set with the setParams() member function
     */
    bool aggregateUsed;

    /**
     * This map records the allocation of local variables and their
     * types.
     */
    vector<SemStr> locals;

    /**
     * The prologue.
     */
    CalleePrologue* prologue;

    /**
     * The epilogue.
     */
    CalleeEpilogue* epilogue;

    /*
     * A map between machine dependent locations and their corresponding
     * symbolic, machine independent representations.
     */
    map<SemStr,SemStr, lessTI> symbolMap;

    /* 
     * An object that represents a set of ranges, which gives the coverage
     * of the source procedure
     */
    Coverage cover;

    /*
     * The return location as written to the .c file. Not valid unless the file
     * has been written (fileWritten true)
     */
    SemStr fileRetLocn;

    /*
     * Set of callees (Procedures that this procedure calls). Used for
     * call graph, among other things
     */
    set<const Proc*> calleeSet;


};      /* UserProc */
#endif
