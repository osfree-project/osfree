/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       hrtl.h
 * OVERVIEW:   Definition of the classes that describe a UQBT HRTL, a high-level
 *             register transfer list. A HRTL is either:
 *                1) a higher-level RTL (instance of class HLJump, HLCall, etc.)
 *                   that represents information about a control transfer
 *                   instruction (CTI) in the source program, or
 *                2) a low-level RTL (instance of class RTL) that is the result
 *                   of decoding a non-CTI source machine instruction.
 *             UQBT's analysis code adds information to existing higher-level
 *             RTLs and sometimes creates new higher-level RTLs (e.g., for
 *             switch statements). 
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2001, The University of Queensland, BT group
 *============================================================================*/

/*
 * $Revision: 1.9 $
 *        01 - Brian: Created based on earlier rtl header files
 * 26 Jul 01 - Brian: Added new HRTL class. Moved RTlist, HLJump, and other HL*
 *              classes from rtl.h to this header file. Edited and reformatted
 *              the latter classes. Changed some argument names for clarity
 *              (not yet complete). Removed unused writeRT() methods.
 * 30 Jul 01 - Brian: Reorganized class structure to reflect reality in today's
 *              code: instructions are initially decoded into HRTLs, not
 *              low-level RTLs, and analysis mostly adds information to existing
 *              higher-level RTLs. Base class is HRTL, which contains most of
 *              the old "RTlist" class functionality. Added new derived class,
 *              "RTL", to represent a low-level RTL; its name reflects current
 *              usage in comments and discussion.
 * 31 Jul 01 - Brian: Removed unused operator== (equality test). Also removed
 *              unused writeRtl() methods. Moved RTLInstDict, TableEntry, and
 *              ParamEntry classes here from rtl.h.
 * 08 Aug 01 - Mike: Added forwardSubs()
 * 13 Aug 01 - Bernard: Added support for type analysis
 * 14 Aug 01 - Mike: decideType(SemStr*) takes int assignSize now
 * 30 Aug 01 - Mike: Changed HLCall's parameters from vector to list
 */

#ifndef __HRTL_H__
#define __HRTL_H__

#include "rtl.h"
#include "ss.h"
#include "bitset.h"			// For BitSet (= BITSET)

/*==============================================================================
 * Class HRTL: describes UQBT's HRTLs, or high-level register transfer lists.
 * A HRTL is either:
 *    1) a higher-level RTL (instance of class HLJump, HLCall, etc.) that
 *       represents information about a control transfer instruction (CTI)
 *       in the source program, or
 *    2) a low-level RTL (instance of class RTL) that is the result of
 *       decoding a non-CTI source machine instruction.
 *============================================================================*/
class HRTL {
public:
    HRTL();
    HRTL(ADDRESS instNativeAddr, list<RT*>* listRT = NULL);
    HRTL(const HRTL& other);           // Makes deep copy of "other"
    virtual ~HRTL();

    // Return a deep copy, including a deep copy of the list of RTs,
    virtual HRTL* clone() const;

    // Assignment copy: set this HRTL to a deep copy of "other".
    const HRTL& operator=(const HRTL &other);

    // Common enquiry methods
    HRTL_KIND getKind() const { return kind; };
    ADDRESS getAddress() const;        // Return HRTL's native address
    bool getCommented() const;         // Return whether to comment in HL C.
    int getSize() const;               // Return size in bits of first RTAssgn.
    unsigned getNumBytes() const;      // Return number of bytes in instruction.
    ADDRESS getOutAddr(int idx) const; // Return the specified out edge addr
    ADDRESS getFallthrough() const;    // Return the fall through address

    // RT list enquiry methods
    int getNumRT() const;              // Return the number of RTs in HRTL.
    RT* elementAt(unsigned i) const;   // Return the i'th element in HRTL.
    
    // RT list editing methods
    void appendRT(RT *rt);             // Add rt to end of HRTL.
    void prependRT(RT *rt);            // Add rt to start of HRTL.
    void insertRT(RT *rt, unsigned i); // Insert rt before RT at position i
    void updateRT(RT *rt, unsigned i); // Change RT at position i.
    void deleteRT(unsigned int);       // Delete RT at position i.
    void clear();                      // Remove all RTs from this HRTL.
    void appendRTlist(const list<RT*>& rtl); // Append list of RTs to end.
    void appendRTlist(const HRTL& rtl); // Append RTs from other HRTL to end

     // Print HRTL to a stream.
    virtual void print(ostream& os = cout, int indent = 0) const;

    // Given a map from registers to expressions, replace each use of a reg
    // with the corresponding expression in the map. Then for every
    // definition of such a reg, update the map with the new RHS resulting
    // from the first substitution.
    void subAXP(map<SemStr,SemStr>& subMap);

    // Return set of the locations a) used, b) defined and c) used before
    // definition by this HRTL.
    virtual void getUseDefLocations(LocationMap& locMap, LocationFilter* filter,
                                    BITSET& defSet, BITSET& useSet,
                                    BITSET& useUndefSet, Proc* proc) const;


    // Set the RTL's source address
    void updateAddress(ADDRESS addr);

    // Set whether to emit low level C for this HRTL as a comment.
    // E.g., in a switch, where a register is loaded from the switch table.
    void setCommented(bool state);

    // Set the number of bytes in the instruction.
    void updateNumBytes(unsigned uNumBytes);

    // Is this HRTL a compare instruction? If so, the passed register and
    // compared value (a semantic string) are set.
    bool isCompare(int& iReg, SemStr*& pTerm);

    // Return true if HRTL loads the high half of an immediate constant into
    // anything. If so, loads the already shifted high value into the parameter.
    bool isHiImmedLoad(ADDRESS& uHiHalf);

    // As above for low half. Extra parameters are required for SPARC, where
    // bits are potentially transferred from one register to another.
    bool isLoImmedLoad(ADDRESS& uLoHalf, bool& bTrans, int& iSrc);

    // Do a machine dependent, and a standard simplification of the HRTL.
    void allSimplify();

    // Perform forward substitutions of temps, if possible. Called from the
    // above
    void forwardSubs();

    // Insert an assignment into this HRTL
    //   ssLhs: ptr to Semstr to place on LHS
    //   ssRhs: ptr to SemStr to place on RHS
    //   prep:  true if prepend (else append)
    //   size:  size of the transfer, or -1 to be the same as the
    //          assign this HRTL
    void insertAssign(SemStr* ssLhs, SemStr* ssRhs, bool prep, int size = -1);

    // Insert an assignment into this HRTL, after temps have been defined
    //   ssLhs: ptr to Semstr to place on LHS
    //   ssRhs: ptr to SemStr to place on RHS
    //   size:  size of the transfer, or -1 to be the same as the
    //          first assign this HRTL
    void insertAfterTemps(SemStr* ssLhs, SemStr* ssRhs, int size = -1);

    // Replace all instances of "search" with "replace".
    virtual void searchAndReplace(SemStr& search, const SemStr& replace,
                                  bool typeSens = false);
    
    // Searches for all instances of "search" and adds them to "result" in
    // reverse nesting order. The search is optionally type sensitive.
    virtual bool searchAll(SemStr &search, list<SemStr*> &result,
                           bool typeSens = false);

    // Make a deep copy of the list of RTs in this HRTL including any SemStrs.
    void deepCopyList(list<RT*>& dest) const; 

#if SRC==SPARC
    bool isAnullBit() const;    // Return true if this is an anulled branch
    HRTL& getNonCti() const;    // Return the non CTI effects
    bool isCCAffected() const;  // Return true if CC affected by this RTL
    bool isDDCall() const;      // Return true if this is a dynamic delayed
                                // instruction and is a call
#endif

protected:
    HRTL_KIND kind;
    ADDRESS nativeAddr;         // HRTL's source program instruction address
    unsigned numNativeBytes;    // Number of source code bytes from which this
                                // HRTL was constructed. Used in coverage
                                // analysis.
    list<RT*> rtlist;           // The list of RTs in HRTL.
    bool isCommented;           // If true, RTL should be emitted as a comment.

#ifdef TRENTS_DATAFLOW_ANALYSIS
    // The following declarations are for Trent's dataflow analysis,
    // which is currently unimplemented and unused.
    //
    // All data flow analysis is done with bitsets. HRTLs must have this
    // information stored in them too as high level things like calls define
    // and use definitions. Note that not all RTLs will have this filled in.
    // I might change this later. - Trent.
    int index;                  // Index of this definition in bitsets. Note:
                                // Note: assignments are "definitions" in
                                // dataflow analysis.
    BITSET defusers;            // Users of this definition
    BITSET defsused;            // Definitions used by this definition
    BITSET livein;              // Available definitions at this definition
    BITSET kills;               // Definitions killed by this definition
    BITSET liveout;             // Available definitions after this definition
#endif /*TRENTS_DATAFLOW_ANALYSIS*/

public:
    // Used for type analysis. Stores type information that
    // can be gathered from the HRTL instruction inside a
    // data structure within BBBlock inBlock
    virtual void storeUseDefineStruct(BBBlock& inBlock);
};


/*=============================================================================
 * Class RTL, which represents a low-level RTL. These are the result of
 * decoding a non-CTI source machine instruction.
 *===========================================================================*/
class RTL: public HRTL {
public:
    RTL();
    RTL(ADDRESS instNativeAddr, list<RT*>* listRT = NULL);
    virtual ~RTL();

    // Make a deep copy, and make the copy a derived object if needed.
    HRTL* clone() const;

    void print(ostream& os = cout, int indent = 0) const;
};


/*=============================================================================
 * HLJump has just one member variable, a semantic string representing the
 * jump's destination (an integer constant for direct jumps; an expression
 * for register jumps). An instance of this class will never represent a
 * return or computed call as these are distinguised by the decoder and are
 * instantiated as HLCalls and HLReturns respecitvely. This class also
 * represents unconditional jumps with a fixed offset (e.g BN, Ba on SPARC).
 *===========================================================================*/
class HLJump: public HRTL {
public:
    HLJump(ADDRESS instNativeAddr, list<RT*>* listRT = NULL);
    HLJump(ADDRESS instNativeAddr, ADDRESS jumpDest);
    virtual ~HLJump();

    // Make a deep copy, and make the copy a derived object if needed.
    virtual HRTL* clone() const;

    // Set and return the destination of the jump. The destination is either
    // a SemStr, or an ADDRESS that is converted to a SemStr.
    void setDest(SemStr* pd);
    void setDest(ADDRESS addr);
    virtual SemStr* getDest() const;

    // Return the fixed destination of this CTI. For dynamic CTIs, returns -1.
    ADDRESS getFixedDest() const;

    // Adjust the fixed destination by a given amount. Invalid for dynamic CTIs.
    void adjustFixedDest(int delta);
    
    // Set and return whether the destination of this CTI is computed.
    // NOTE: These should really be removed, once HLNwayJump and HLNwayCall
    // are implemented properly.
    void setIsComputed();
    bool isComputed() const;

    // Collect locations used by this HLJump
    void getUseDefLocations(LocationMap& locMap, LocationFilter* filter,
                            BITSET& defSet, BITSET& useSet,
                            BITSET& useUndefSet, Proc* proc) const;

    virtual void print(ostream& os = cout, int indent = 0) const;

    // Replace all instances of "search" with "replace".
    void searchAndReplace(SemStr& search, const SemStr& replace,
                          bool typeSens = false);
    
    // Searches for all instances of a given subexpression within this
    // expression and adds them to a given list in reverse nesting order.
    // If typeSens is set, then type is considered during the match
    virtual bool searchAll(SemStr &search, list<SemStr*> &result,
                           bool typeSens = false);

    // Used for type analysis. Stores type information that
    // can be gathered from the HRTL instruction inside a
    // data structure within BBBlock inBlock
    virtual void storeUseDefineStruct(BBBlock& inBlock);			   

protected:
    SemStr* pDest;              // Destination of a jump or call. This is the
                                // absolute destination for both static and
                                // dynamic CTIs.
    bool m_isComputed;          // True if this is a CTI with a computed
                                // destination address. NOTE: This should be
                                // removed, once HLNwayJump and HLNwayCall
                                // are implemented properly.
};


/*==============================================================================
 * HLJcond has a condition SemStr in addition to the destination of the jump.
 *============================================================================*/
class HLJcond: public HLJump {
public:
    HLJcond(ADDRESS instNativeAddr, list<RT*>* listRT = NULL);
    virtual ~HLJcond();

    // Make a deep copy, and make the copy a derived object if needed.
    virtual HRTL* clone() const;

    // Set and return the JCOND_TYPE of this jcond as well as whether the
    // floating point condition codes are used.
    void setCondType(JCOND_TYPE cond, bool usesFloat = false);
    JCOND_TYPE getCond() const { return jtCond; }
    bool isFloat() const       { return bFloat; }
    void setFloat(bool b)      { bFloat = b; }

    // Set and return the SemStr representing the HL condition
    SemStr* getCondExpr() const;
    void setCondExpr(SemStr* pss);
    
    // Probably only used in front386.cc: convert this from an unsigned to a
    // signed conditional branch
    void makeSigned();

    virtual void print(ostream& os = cout, int indent = 0) const;

    // Replace all instances of "search" with "replace".
    void searchAndReplace(SemStr& search, const SemStr& replace,
                          bool typeSens = false);
    
    // Searches for all instances of a given subexpression within this
    // expression and adds them to a given list in reverse nesting order.
    // If typeSens is set, then type is considered during the match
    virtual bool searchAll(SemStr &search, list<SemStr*> &result,
                           bool typeSens = false);

    // Used for type analysis. Stores type information that
    // can be gathered from the HRTL instruction inside a
    // data structure within BBBlock inBlock
    void storeUseDefineStruct(BBBlock& inBlock);   

private:
    JCOND_TYPE jtCond;          // The condition for jumping
    SemStr* pCond;              // The SemStr representation of the high level
                                // condition: e.g., r[8] == 5
    bool bFloat;                // True if uses floating point CC

};

/*==============================================================================
 * HLNwayJump is derived from HLJump. In addition to the destination of the
 * jump, it has a switch variable SemStr.
 *============================================================================*/
typedef struct {
    SemStr* pSwitchVar;         // Ptr to SemStr repres switch var, e.g. v[7]
    char    chForm;             // Switch form: 'A', 'O', 'R', or 'H'
    int     iLower;             // Lower bound of the switch variable
    int     iUpper;             // Upper bound for the switch variable
    ADDRESS uTable;             // Native address of the table
    int     iNumTable;          // Number of entries in the table (form H only)
    int     iOffset;            // Distance from jump to table (form R only)
    int     delta;              // Host address - Native address
} SWITCH_INFO;

class HLNwayJump: public HLJump {
public:
    HLNwayJump(ADDRESS instNativeAddr, list<RT*>* listRT = NULL);
    virtual ~HLNwayJump();

    // Make a deep copy, and make the copy a derived object if needed.
    virtual HRTL* clone() const;

    // Set and return the SemStr representing the switch variable
    SWITCH_INFO* getSwitchInfo() const; 
    void setSwitchInfo(SWITCH_INFO* pss);
    
    virtual void print(ostream& os = cout, int indent = 0) const;

    // Replace all instances of "search" with "replace".
    void searchAndReplace(SemStr& search, const SemStr& replace,
                          bool typeSens = false);
    
    // Searches for all instances of a given subexpression within this
    // expression and adds them to a given list in reverse nesting order.
    // If typeSens is set, then type is considered during the match
    virtual bool searchAll(SemStr &search, list<SemStr*> &result,
                           bool typeSens = false);
    
    // Used for type analysis. Stores type information that
    // can be gathered from the HRTL instruction inside a
    // data structure within BBBlock inBlock
    void storeUseDefineStruct(BBBlock& inBlock);   
        
private:    
    SWITCH_INFO* pSwitchInfo;   // SemStr representation of the switch variable:
                                // e.g., r[8]
};

/*==============================================================================
 * HLCall: represents a high level call. Information about parameters and
 * the like are stored here.
 *============================================================================*/
class HLCall: public HLJump {
public:
    HLCall(ADDRESS instNativeAddr, int returnTypeSize = 0,
           list<RT*>* listRT = NULL);
    HLCall::~HLCall();

    // Make a deep copy, and make the copy a derived object if needed.
    virtual HRTL* clone() const;

    // Return true if the called function returns an aggregate: i.e., a
    // struct, union or quad floating point value.
    bool returnsStruct();

    void setBB(PBB BB);                // Set link from call to its enclosing BB
    const PBB getBB();                 // Return link from call to enclosing BB

    void setParams(const list<SemStr>& params); // Set call's parameters
    const list<SemStr>& getParams() const;      // Return call's parameters
    list<SemStr>& getParams();                  // Non-const version of above

    void setReturnLoc(const SemStr& loc); // Set location used for return value
    SemStr getReturnLoc() const;       // Return location used for return value

    void setPrologue(Logue* prologue); // Set call's prologue
    Logue* getPrologue() const;        // Return call's prologue (if any)

    void setEpilogue(Logue* epilogue); // Set call's epilogue
    Logue* getEpilogue() const;        // Return call's epilogue (if any)

    // The DFA analysis of a call RTL differs from a standard RTL in that it
    // doesn't have any RTs to analyse. A call *uses* its parameters and
    // *defines* its return location. These uses and definitions won't be
    // recorded by this method if they have been recorded yet.
    void getUseDefLocations(LocationMap& locMap, LocationFilter* filter,
                            BITSET& defSet, BITSET& useSet,
                            BITSET& useUndefSet, Proc* proc) const;

    virtual void print(ostream& os = cout, int indent = 0) const;

    // Replace all instances of "search" with "replace".
    void searchAndReplace(SemStr& search, const SemStr& replace,
                          bool typeSens = false);
    
    // Searches for all instances of a given subexpression within this
    // expression and adds them to a given list in reverse nesting order.
    // If typeSens is set, then type is considered during the match
    virtual bool searchAll(SemStr &search, list<SemStr*> &result,
                           bool typeSens = false);

   /*==========================================================================
    * These methods are only used (so far) if the source is a SPARC object file
    *========================================================================*/

    // Set and return whether the call is effectively followed by a return.
    // E.g. on Sparc, whether there is a restore in the delay slot.
    void setReturnAfterCall(bool b);
    bool isReturnAfterCall() const;

    // Set and return the list of RTs that occur *after* the call (the
    // list of RTs in the HRTL occur before the call). Useful for odd patterns.
    void setPostCallRtlist(const list<RT*>* lrt);
    const list<RT*>* getPostCallRtlist() const;

    // Set and return the destination name. Useful for patterns.
    void setDestName(const char* pName);
    const char* getDestName() const;
    
    // Used for type analysis. Stores type information that
    // can be gathered from the HRTL instruction inside a
    // data structure within BBBlock inBlock
    void storeUseDefineStruct(BBBlock& inBlock);       

private:
    int returnTypeSize;         // Size in bytes of the struct, union or quad FP
                                // value returned by the called function.
    Logue* prologue;            // The prologue (if any) of this call.
    Logue* epilogue;            // The epilogue (if any) of this call.
    PBB basicBlock;             // The call's enclosing basic block.
    SemStr returnLoc;           // Location used to hold value returned by call.
    bool returnAfterCall;       // True if call is effectively followed by
                                // a return.
    
    // The list of locations that are live at this call. This list may be
    // refined at a later stage to match the number of parameters declared
    // for the called procedure.
    list<SemStr> params;

    // List of reg transfers that occur *after* the call.
    const list<RT*>* postCallRtlist;

    // The name of the destination. (Note: this is usually found by looking
    // up the destination using the BinaryFile object, but in this case the
    // name is totally synthetic.) When used, getFixedDest() returns 0
    const char* pDestName;
};


/*==============================================================================
 * HLReturn: represents a high level return.
 *============================================================================*/
class HLReturn: public HLJump {
public:
    HLReturn(ADDRESS instNativeAddr, list<RT*>* listRT = NULL);
    ~HLReturn();

    // Make a deep copy, and make the copy a derived object if needed.
    virtual HRTL* clone() const;

    // Return the set of the locations a) used, b) defined and c) used
    // before definition by this RTL.
    virtual void getUseDefLocations(LocationMap& locMap, LocationFilter* filter,
                                    BITSET& defSet, BITSET& useSet,
                                    BITSET& useUndefSet, Proc* proc) const;
				    
    // Used for type analysis. Stores type information that
    // can be gathered from the HRTL instruction inside a
    // data structure within BBBlock inBlock
    void storeUseDefineStruct(BBBlock& inBlock);   				    
};


/*==============================================================================
 * HLScond represents "setCC" type instructions, where some destination is
 * set (to 1 or 0) depending on the condition codes. It has a condition
 * SemStr, similar to the HLJcond class.
 * *==========================================================================*/
class HLScond: public HRTL {
public:
    HLScond(ADDRESS instNativeAddr, list<RT*>* listRT = NULL);
    virtual ~HLScond();

    // Make a deep copy, and make the copy a derived object if needed.
    virtual HRTL* clone() const;

    // Set and return the JCOND_TYPE of this scond as well as whether the
    // floating point condition codes are used.
    void setCondType(JCOND_TYPE cond, bool usesFloat = false);
    JCOND_TYPE getCond() const {return jtCond;}
    bool isFloat() const  {return bFloat;}
    void setFloat(bool b) { bFloat = b; }

    // Set and return the SemStr representing the HL condition
    SemStr* getCondExpr() const;
    void setCondExpr(SemStr* pss);

    SemStr* getDest() const;       // Return the destination of the set
    int getSize() const;           // Return the size of the assignment

    void makeSigned();             // Is this ever used?

    virtual void print(ostream& os = cout, int indent = 0) const;

    // Used for type analysis. Stores type information that
    // can be gathered from the HRTL instruction inside a
    // data structure within BBBlock inBlock
    void storeUseDefineStruct(BBBlock& inBlock);       

private:
    JCOND_TYPE jtCond;             // the condition for jumping
    SemStr* pCond;                 // SemStr representation of the high level
                                   // condition: e.g. r[8] == 5
    bool bFloat;                   // True if condition uses floating point CC
};


/*==============================================================================
 * The TableEntry class represents a single instruction - a string/HRTL pair.
 *
 * This class plus ParamEntry and RTLInstDict should be moved to a separate
 * header file...
 *============================================================================*/

class TableEntry {
public:
    TableEntry();
    TableEntry(const list<int>& p, const HRTL& rtl);

    const TableEntry& operator=(const TableEntry& other);

    void setParam(const list<int>& p);
    void setHRTL(const HRTL& hrtl);
    
    // non-zero return indicates failure
    int appendHRTL(list<int>& p, HRTL& hrtl);
    
public:
    list<int> params;
    HRTL hrtl;

#define TEF_NEXTPC 1        
    int flags;                  // aka required capabilities. Init. to 0 
};


/*==============================================================================
 * The ParamEntry class represents the details of a single parameter.
 *============================================================================*/
typedef enum {PARAM_SIMPLE, PARAM_EXPR, PARAM_LAMBDA, PARAM_VARIANT} ParamKind;

class ParamEntry {
 public:
    ParamEntry() {
        rta = NULL;
        kind = PARAM_SIMPLE;
        type = Type();
        regType = Type(TVOID,0,false);
        lhs = false;
        mark = 0;
    }
    
    list<int> params;           /* PARAM_VARIANT & PARAM_EXPR only */
    list<int> funcParams;       /* PARAM_LAMBDA - late bound params */
    RTAssgn *rta;               /* PARAM_EXPR only */
    bool lhs;                   /* True if this param ever appears on the LHS
                                 * of an expression */
    ParamKind kind;
    Type type;
    Type regType;               /* Type of r[this], if any (void otherwise) */
    set<int> regIdx;            /* Values this param can take as an r[param] */
    int mark;                   /* Traversal mark. (free temporary use,
                                 * basically) */
};

class PartialType;


/*==============================================================================
 * The RTLInstDict represents a dictionary that maps instruction names to the
 * parameters they take and a template for the RT list describing their
 * semantics. It handles both the parsing of the SSL file that fills in
 * the dictionary entries as well as instantiation of an RT list for a given
 * instruction name and list of actual parameters. 
 *============================================================================*/

class RTLInstDict {
public:
    // Parse a file containing a list of instructions definitions in SSL format
    // and build the contents of this dictionary.
    bool readSSLFile(const string& SSLFileName, bool bPrint = false);

    // Return the signature of the given instruction.
    pair<string,unsigned> getSignature(const char* name);

    // Appends an RTL to an idict entry, or Adds it to idict if an
    // entry does not already exist. A non-zero return indicates failure.
    int appendToDict(string &n, list<int>& p, HRTL& rtl);
    
    // Given an instruction name and list of actual parameters, return an
    // instantiated RTL for the corresponding instruction entry.
    list<RT*>* instantiateRTL(const string& name,
                              const vector<SemStr*>& actuals);
    // As above, but takes an HRTL & param list directly rather than doing
    // a table lookup by name.
    list<RT*>* instantiateRTL(const HRTL& rtls, const list<int> &params,
                              const vector<SemStr*>& actuals);

    // Transform the given list into another list which doesn't have
    // post-variables, by either adding temporaries or just removing them
    // where possible. Modifies the list passed, and also returns a pointer
    // to it. Second parameter indicates whether the routine should attempt
    // to optimize the resulting output, ie to minimize the number of
    // temporaries. This is recommended for fully expanded expressions (ie
    // within uqbt), but unsafe otherwise.
    list<RT*>* transformPostVars(list <RT*> *rts, bool optimize);

    void print(ostream& os = cout) const;
    
    // Add a new register to the machine
    void addRegister(const char *name, int id, int size, bool flt);

    // If the top level operator of the given expression indicates any kind
    // of type, update the expression's type to match.
    bool partialType(SemStr *exp);

    // Scan the expression / assignment given, and attempt to determine the
    // correct overall type. If the type is decideable, updates the parameter
    // appropriately and returns true. Other the functions return false.
    bool decideType(SemStr *exp, int assignSize);
    bool decideType(RTAssgn *rta);

    // Type checking and evaluation functions
    set<int> evaluateSemStr(const SemStr &ss) const;
    set<int> evaluateBitsliceRange(const SemStr &lo, const SemStr &hi) const;
    int matchRegIdx(const SemStr &ss) const;
    vector<Type> computeRHSTypes(SemStr &ss, int sz) const;
    vector<Type> computeLHSTypes(SemStr &ss) const;
    vector<Type> computeRTAssgnTypes(RTAssgn &rta) const;
    
    // Go through the params and fixup any lambda functions
    void fixupParams();

public:
    // A map from the symbolic representation of a register (e.g. "%g0")
    // to its index within an array of registers.
    map<string, int, less<string> > RegMap;

    // Similar to r_map but stores more information about a register such as its
    // size, its addresss etc (see reg.h).
    map<int, Register, less<int> > DetRegMap;

    // A map from symbolic representation of a special (non-addressable)
    // register to a Register object
    map<string, Register, less<string> > SpecialRegMap;

    // A map from symbolic representation of a parameter to a semtable index
    map<string, int, less<string> > ParamMap;

    // Parameter details (where given)
    map<int, ParamEntry, less<int> > DetParamMap;

    // ???
    map<string, RTFlagDef*, less<string> > FlagFuncs;
    map<string, pair<int, void*>*, less<string> > DefMap;
    map<int, SemStr*, less<int> > AliasMap;

    // Map from ordinary instruction to fast pseudo instruction, for use with
    // -f (fast but not as exact) switch
    map<string, string> fastMap;

    bool bigEndian;                // True if this source is big endian

    // The actual dictionary.
    map<string, TableEntry, less<string> > idict;

    // An HRTL describing the machine's basic fetch-execute cycle
    HRTL *fetchExecCycle;

protected:
    int evaluateSemStr(const SemStr &ss, map<int,int> &params,
                       map<int,int> &regs) const;
    int evaluateSemStr(SSCIT &it, SSCIT &end, map<int,int> &params,
                       map<int,int> &regs) const;
    PartialType computeTypes(SemStr &ss, const PartialType &in,
                             vector<PartialType> &types) const;
    PartialType computeTypes(SemStr &ss, SSIT &it, SSIT &end,
                             const PartialType &in,
                             vector<PartialType> &types) const;
    void distributeTypes(const SemStr &ss, const PartialType &in,
                         vector<PartialType> &types, vector<Type> &out) const;
    void distributeTypes(const SemStr &ss, SSCIT &it, SSCIT &end,
                         const PartialType &in,
                         vector<PartialType>::iterator &type,
                         vector<Type> &out) const;
    set<int> evaluateSemStr(const list<SemStr> &lss,
                            int (*callback)(list<int> &)) const;
    
    void fixupParamsSub(int n, list<int> &funcParams, bool &haveCount,
                        int mark);
};

#endif /*__HRTL_H__*/
