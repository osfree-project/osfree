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
 * FILE:       csr.h
 * OVERVIEW:   This file declares classes used to perform Call Signature
 *             Recovery (CSR) analysis.
 *
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.52 $
 * 15 Mar 00 - Cristina: CSR::setAFP transformed to setAXP and idAGP 
 *               introduced. 
 * 16 Mar 00 - Cristina: added addMoreSubstitutions to support agp 
 *               substitutions.
 * 25 Aug 00 - Mike: LocationMap's map is type insensitive now
 * 08 Sep 00 - Mike: analyseCallee() is public now (called from proc.cc)
 * 14 Sep 00 - Mike: analyseCallee and analyseCaller return a bool now
 * 02 Mar 01 - Mike: LocationMap's maps are now sign insensitive, but otherwise
 *              type sensitive. A typo left lessTI the same as lessSGI
 *              Also added getIntSize()
 * 26 Apr 01 - Mike: ReturnLocation's type_locs is sign insensitive now;
 *              CSR::setReturnType takes a UserProc now
 * 08 May 01 - Nathan: Fix includes
 * 23 Jul 01 - Mike: Added stackInverted boolean
 * 01 Aug 01 - Mike: Changed agpInit to agpMap, to accomodate changes to .pal
 * 13 Aug 01 - Mike: Parameters::filterLocations() has an extra parameter now
 * 17 Aug 01 - Mike: class Parameters has more members for float register params
 * 30 Aug 01 - Mike: Procedure parameters changed from vector to list
 * 12 Oct 01 - Mike: Changed LocationMap to be type insensitive (was just sign
 *              insensitive); added Parameters::getActParamLoc()
 * 22 Oct 01 - Mike: Added Parameters::getOffsetEquals member function
 * 26 Oct 01 - Mike: LocationMap is back to sign insensitive again
 */

#ifndef __CSR__
#define __CSR__

#include "decoder.h"
#include "ss.h"                 // For class SemStr
#include "bitset.h"             // For class BitSet
#include "type.h"               // For class typeLessSI

#ifdef WIN32
namespace NameSpaceCSR {
#endif

typedef map<SemStr,SemStr>::iterator substitutions_IT;
typedef map<SemStr,SemStr>::const_iterator substitutions_CIT;

/*==============================================================================
 * A Parameters object represents a specification of the locations
 * that can be used for parameter passing. Such a specification can be
 * used to represent both stack and register locations.
 *============================================================================*/
class Parameters {
public:

    /**
     * Default constructor.
     */
    Parameters() {}

    /**
     * Constructor. Given that a parameter spec may not
     * include registers (of various types) or a stack part, any of the
     * parameters to this constructor may be NULL.
     */
    Parameters(SemStr* aggregate,
        const list<SemStr>* int_regs,
        const list<SemStr>* float_regs,
        const list<SemStr>* double_regs,
        const pair<SemStr,int>* stack,
        const map<Type, int>* alignment);

    /**
     * Destructor.
     */
    ~Parameters();

    /**
     * Instantiate the variables in this spec with the given mapping
     * from variable names to values.
     */
    void instantiate(vector<pair<string,int> >& nameValMap);

    /**
     * Given a set of locations, remove those that aren't valid parameter
     * locations. Used to create the list of parameters to a call
     */
    bool filterLocations(BITSET& locs, list<SemStr>& result,
        LocationMap& locMap, bool stackInverted, bool uniqueSlots) const;

    /**
     * Given a single location, return true if it *could* be a parameter
     * location. Ignores order in which locations must be used to be valid
     * parameter locations.
     */
    bool matches(const SemStr& loc) const;

    /**
     * Add n more parameters to a given list of parameters.
     * NOTE: Only adds register parameters as there
     * should never really be a need to add more than one
     * parameter (a returned result) to correct programs.
     */
    void addParams(int n, list<SemStr>& params) const;

    /*
     * Return a list of locations for the first n parameters
     */
    list<SemStr>* firstParams(int n) const;

    /*
     * Adjust the parameter size to conform to the minimum possible
     * For most architectures, this is 4 bytes; for Palm, it is 2
     * Use the PARAMETERS OFFSET value from the .pal file
     */
    void    conformParamSize(int& size) const;

    /*
     * Check for missing parameters, and insert appropriate locations to
     * fill any gaps found
     */
    void missingParamCheck(list<SemStr>& params, UserProc* proc,
        const Parameters& outgoingParams, bool aggUsed) const;

    /*
     * Align a slot number, given also the type for the parameter
     * This makes use of the private member alignment
     */
    void alignSlotNumber(int& slot, const Type& ty) const;

    /*
     * Get the size of the OFFSET = parameter from the PAL spec
     * This is essentually sizeof(int) for the given architecture
     */
    int getIntSize() const;

    /*
     * Get the "slot index" for the given parameter location.
     * A parameter slot holds one parameter of size <= one machine
     * word. If the parameter would use more than one slot, the
     * lower slot index is returned
     */
    int getParamSlot(const SemStr& loc, int& slots, bool first, int& firstOff)
        const;

    /*
     * Get a location for the actual parameter location, with the
     * given type and slot number
     */
    const SemStr* getActParamLoc(const Type& ty, int slot) const;

    /**
     * Output operator for a Parameters object.
     */
    friend ostream& operator<<(ostream& os, const Parameters& cp);

    /*
     * Return the offset= value
     */
    int getOffsetEquals() const {return offset;}

private:

    /**
     * Given a location, return true if it is a memory
     * location that can be reduced to being at a
     * multiple of my offset from my base. Return the
     * offset in the given reference parameter.
     */
    bool getOffset(const SemStr& loc, int& loc_offset) const;

private:

    /**
     * The list of register locations.
     */
    list<SemStr> int_registers;
    list<SemStr> float_registers;
    list<SemStr> double_registers;
    typedef list<SemStr>::iterator registers_IT;
    typedef list<SemStr>::const_iterator registers_CIT;

    /**
     * The stack location base.
     */
    SemStr stackBase;

    /**
     * The size of the offset increments from the stack base that stack
     * parameters can be found at.
     */
    int offset;

    /**
     * The locations used for passing pointers to returned structs.
     * Only used by some architectures (e.g. SPARC).
     */
    SemStr aggregate;

    /*
     * A map from LOC_TYPE to integer, representing the alignment information
     * for that LOC_TYPE
     */
    map<Type, int> alignment;
};

/*==============================================================================
 * This class stores a mapping between types and the locations in
 * which data of these types is returned from a function.
 * Note there is one ReturnLocations object in the CSR object which stores the
 * locations from the CALLER's perspective (i.e. where they are received).
 * Each callee prologue has a ReturnLocations object for callees using that
 * particular epilogue.
 *============================================================================*/
typedef map<Type, SemStr, typeLessSI> typeToSemStrMap;
class ReturnLocations {
public:

    /**
     * Default constructor.
     */
    ReturnLocations() {}

    /*
     * Constructor that sets the mapping of types to locations.
     */
    ReturnLocations(const typeToSemStrMap& mapping);

    /**
     * Given a set locations, remove those that aren't valid return
     * value locations.
     */
    void filterLocations(BITSET& locs, list<SemStr>& result,
        LocationMap& locMap) const;

    /**
     * Return the location used for a given type or NULL if t is an invalid
     * return type.
     */
    const SemStr* locationFor(const Type& t) const;

    /*
     * Return the type that uses the given location.
     */
    const Type& typeFor(const SemStr& loc) const;

    /*
     * Return the set of return locations as a bitset.
     */
    BITSET toBitset(LocationMap& locMap) const;

    /*
     * Return true if the given location is within the
     * return location spec.
     */
    bool matches(const SemStr& loc) const;

    /**
     * Output operator for a ReturnLocations object.
     */
    friend ostream& operator<<(ostream& os, const ReturnLocations& rl);

private:

    /*
     * The set of return locations.
     * Note that we don't want to distnguish between SemStr's of different signs
     * (e.g. we want to consider int32 and unsigned int32 to be the same loc'n
     */
    setSgiSemStr locs;
    typedef setSgiSemStr::const_iterator locs_CIT;

    /*
     * A map from type to the locations used for those types.
     */
    typeToSemStrMap type_locs;
    typedef typeToSemStrMap::const_iterator type_locs_CIT;
};


/*==============================================================================
 * On certain architectures, a callee may write to a certain portion
 * of a parent's stack. Typically this will be to save the value of a
 * register parameter when that register needs to be used for
 * something else before the value is used. It will also happen for
 * procedures taking variable arguments. This class is used to record
 * information about that portion and replace any write to locations
 * within it with simply saves to a local variable.
 *============================================================================*/
class ParentStack {
public:

    /**
     * Default constructor.
     */
    ParentStack();

    /**
     * Constructor.
     */
    ParentStack(const SemStr& low, const SemStr& high, int step);

    /**
     * Instantiate the variables in this spec with the given mapping
     * from variable names to values.
     */
    void instantiate(vector<pair<string,int> >& nameValMap);

    /**
     * Returns the range of locations defined by this spec.
     */
    void getRange(vector<SemStr>& locs);

    /**
     * Output operator.
     */
    friend ostream& operator<<(ostream& os, const ParentStack& cs);

private:

    /**
     * This is the low end of the range.
     */
    SemStr low;

    /**
     * This is the size of the range in bytes;
     */
    int range;

    /**
     * This is the minimum size in bytes of quantities which may be
     * accessed in the range.
     */
    int step;
};

/*==============================================================================
 * A Logue object represents a caller/callee prologue or epilogue. It may either
 * be a template Logue or an instantiated log. The first results from
 * parsing a CSR spec and the latter from instantiating a template Logue object
 * during actual decoding.
 *============================================================================*/
class Logue {
public:

    /**
     * The types of logues.
     */
    enum TYPE {
        CALLER_PROLOGUE,
        CALLEE_PROLOGUE,
        CALLEE_EPILOGUE,
        CALLER_EPILOGUE
    };

    /**
     * The type names.
     */
    static const string TYPE_NAMES[4];

    /**
     * Constructor.
     */
    Logue(const string& name, TYPE type, const list<string>& params);

    /**
     * Destructor.
     */
    virtual ~Logue() {}

    /**
     * Get the name of this logue.
     */
    string getName() const;

    /**
     * Get the type of this logue.
     */
    TYPE getType() const;

    /**
     * Return a set containing the names of the formal parameters
     * of this logue.
     */
    void getFormals(set<string>& formals) const;

    /**
     * The position of this logue in a partial ordering between all logues.
     */
    int getOrder() const;

    /**
     * Instantiate this newly constructed logue.
     */
    virtual void instantiate(vector<int>& values);

    /**
     * Output operator.
     */
    friend ostream& operator<<(ostream& os, const Logue& logue);

    /**
     * Performs the real work of output.
     */
    virtual ostream& put(ostream& os) const;

protected:

    /**
     * The name of the logue.
     */
    string name;

    /**
     * The type of the logue.
     */
    TYPE type;

    /**
     * The parameters of this logue. Implemented as a vector of pairs
     * instead of a plain map so that the order of the parameters is
     * preserved (required for instantiation).
     */
    vector<pair<string,int> > params;
    typedef vector<pair<string,int> >::iterator params_IT;
    typedef vector<pair<string,int> >::const_iterator params_CIT;

    /**
     * The position of this logue in a total ordering of all logues.
     */
    int orderPos;

    /**
     * Indicate whether or not this is an instantiated logue.
     */
    bool instantiated;

    /**
     * Class field used to assign orderings to default instantiated logues.
     */
    static int nextOrderPos;
};

/*==============================================================================
 * A CalleePrologue extends a standard Logue to include information
 * about substitutions that must be effected after a prologue in a
 * callee, where incoming parameters may be found, the amount of space
 * allocated for local variables and what range of a
 * parent's stack may be written to.
 *============================================================================*/
class CalleePrologue : public Logue {
public:

    /**
     * Constructor.
     */
    CalleePrologue(const string& name, const list<string>& params);

    /**
     * Record the info about where incoming parameters will be found.
     */
    void setParamSpec(const Parameters& paramSpec);

    /**
     * Get a pointer to the incoming parameters' spec.
     */
    Parameters* getParamSpec();

    /**
     * Set the substitutions that must be effected after execution of
     * this prologue.
     */
    void setSubstitutions(const map<SemStr,SemStr>& subs);

    /**
     * Add the AXP substitutions of this prologue to a given map.
     */
    void addToSubstitutions(map<SemStr,SemStr>& subs);

    /**
     * Set the expression that represents the amount of space
     * allocated for local variables.
     */
    void setLocalsSize(const SemStr& localsSize);

    /**
     * Get the amount of space allocated for local variables.
     */
    int getLocalsSize();

    /**
     * Record the info about which part of a calller's stack may be
     * written to.
     */
    void setParentStackSpec(const ParentStack& parentStack);

    /**
     * Get a pointer to the parent stack spec.
     */
    ParentStack* getParentStackSpec();

    /**
     * Extend the instantiation of these types of logues to give
     * values to the variables in the incoming parameters spec and the
     * substitutions spec.
     */
    void instantiate(vector<int>& values);
    
    /**
     * Extend the output operator.
     */
    ostream& put(ostream& os) const;

private:
    /**
     * These are the locations that must be converted to AFP 
     * relative locations after this prologue is effected. It's a map
     * from register expression to arithmetic expressions involving
     * the abstract frame pointer. An example of an entry is:
     *   %fp <-> %afp - simm13
     * Any non-constant on the right hand side must be either the AFP
     * or a parameter of this prologue.
     * As of 1/Aug/2001, AGP substitutions are global (not specific to a
     * logue)
     */
    map<SemStr,SemStr> substitutions;

    /**
     * This is where incoming parameters can be found.
     */
    Parameters incomingParams;

    /*
     * The expression that represents the amount of space allocated for
     *  local variables. It needs to be a SemStr since it will get instantiated
     *  using searchReplaceAll()
     */
    SemStr localsSize;
    
    /**
     * This is the part of a parent's stack that may be written to.
     */
    ParentStack parentStack;
};

/*==============================================================================
 * A CalleePrologue extends a standard Logue to include information
 * about where returned values can be found.
 *============================================================================*/
class CalleeEpilogue : public Logue {
public

    /**
     * Constructor.
     */:
    CalleeEpilogue(const string& name, const list<string>& params);

    /**
     * Set the info about where returned values are found.
     */
    void setRetSpec(const ReturnLocations& retSpec);
    
    /**
     * Return a pointer to the return locations spec.
     */
    ReturnLocations* getRetSpec();
    
    /**
     * Extend the output operator.
     */
    ostream& put(ostream& os) const;

private:
    /**
     * This is where return values are put. That is, if one of these
     * locations is defined upon exit from a procedure then it
     * potentially holds a returned value.
     */
    ReturnLocations retLocs;
};

/*==============================================================================
 * A LogueDict stores Logue templates.
 *============================================================================*/
class LogueDict {
public:

    /**
     * Initialise the dictionary by inserting the logues declared in a
     * pattern file. The code for this method is automatically
     * generated from a .pat file.
     */
    LogueDict();

    /**
     * Constructs and inserts a new logue into the dictionary.
     */
    void newLogue(const string& name, const string& type,
        const list<string>& params);

    /*
     * Return true if a logue exists with the given name
     */
    bool exists(const string& name) const;

    /**
     * Return a reference to the logue template of the given name.
     * This causes an error if the given name doesn't refer to an
     * existing logue.
     */
    Logue* operator[](const string& name);

    /**
     * Given a logue name and a list of ordered values, return an
     * instantiated Logue.
     */
    Logue* instantiate(const string& name, vector<int>& params);

    /**
     * Output operator for a LogueDict object.
     */
    friend ostream& operator<<(ostream& os, const LogueDict& dict);

private:
    /**
     * An logue dictionary is implemented as a map from strings
     * to Logues.
     */
    map<string,Logue*> dict;
    typedef map<string,Logue*>::iterator dict_IT;
    typedef map<string,Logue*>::const_iterator dict_CIT;
};
/*==============================================================================
 * To enable efficient analysis, we want to be able to represent locations as
 * integers so that sets of locations can be represented using bitsets. The
 * LocationMap provides a convenient way to provide a mapping between locations
 * and integers.
 *============================================================================*/
class LocationMap {
public:

    ~LocationMap();
    /**
     * Return a reference to the location that has the given ID. Assumes that
     * the location is already in the map.
     */
    const SemStr& toLocation(int locID) const;

    /*
     * Convert a given bitset into a (sign insensitive) set of locations.
     * Note: this has been a type insensitive map in the past, but memory
     * locations in particular get re-used with different sizes
     */
    void toLocations(const BITSET& locs, setSgiSemStr& result) const;

    /**
     * Return the bit position corresponding to the given location.
     * Will add the location to the map if it isn't already in it.
     */
    int toBit(const SemStr& loc);
    
    /*
     * Return the bitset corresponding to the given set of locations.
     * Will add any locations to the map if they aren't already in it.
     */
    BITSET toBitset(const setSgiSemStr& locs);

    /*
     * Test if the given location exists in the given bitset
     */
    bool test(const SemStr& loc, const BITSET& bs) const;
    
    /**
     * Return the number of locations currently in the map.
     */
    int size() const;

    /**
     * Print the expressions represented a given bitset whose bit
     * representation was set by this filter.
     */
    ostream& printBitset(ostream& os, const BITSET& exprs);

    /**
     * Print the expressions represented a given list of integers.
     */
    ostream& printLocs(ostream& os, const vector<int>& exprs);

private:

    /*
     * The unique IDs are generated using a map from locations (SemStr*) to
     * integers. The map is type insensitive; uses and definitions don't care
     * about type. For example, if m[1000] is defined as an integer and used as
     * a float, these should be considered the same location. Otherwise,
     * m[1000]<32f> is "use before defined", which is wrong.
     */
    map<SemStr, int, lessSGI> locationIDs;
    typedef map<SemStr,int, lessSGI>::const_iterator locationIDs_CIT;

    /*
     * For fast conversion back to a location from an ID, use a vector of
     * iterators to the contents of locationIDs. Using a map of iterators means
     * that only one copy of any given SemStr is in this map.
     */
    vector<map<SemStr, int, lessSGI>::iterator> IDlocations;
    typedef vector<map<SemStr,int, lessSGI>::iterator>::const_iterator
        IDlocations_CIT;
};

/*==============================================================================
 * When doing CSR analysis, we would like to be able to parameterise a generic
 * data flow analysis (DFA) algorithm that collects a particular type of
 * data-flow information (such as available expressions, live variables etc)
 * with a filter constraining which facts to consider. For example,
 * we may want a filter that ensures we only collect information
 * about certain locations (i.e. parameter and return locations).
 * The LocationFilter is a base class for defining such filters. It is a
 * trivial filter that will match anything.
 *============================================================================*/
class LocationFilter {
public:

    /**
     * Destructor (only for preventing compiler warnings).
     */
    virtual ~LocationFilter();

    /**
     * Returns true.
     */
    virtual bool matches(const SemStr& loc);

};

/*==============================================================================
 * The CSRLocationFilter matches only locations specified as valid
 * parameter or return locations in a CSR spec.
 *============================================================================*/
class CSRLocationFilter : public LocationFilter {
public:

    /**
     * A CSRLocationFilter must be instantiated with the two objects it
     * delegates the actual matching to.
     */
    CSRLocationFilter(Parameters* params, ReturnLocations* retLocs);

    /**
     * Returns true if the given location is a valid parameter location or a
     * return location as specified in a CSR spec.
     */
    bool matches(const SemStr& loc);

private:

    /**
     * The parameter spec has its own `match' function that we can
     * delegate to.
     */
    const Parameters* params;

    /**
     * The return locations define a mapping between types and
     * locations. 
     */
    const ReturnLocations* retLocs;

};

/*==============================================================================
 * All of the above classes are used to store information that will be used when
 * doing the analysis required to recover call signatures. The actual analysis
 * itself will be done by methods in the CSR class.
 *============================================================================*/
class CSR {

    /**
     * The CSRParser class is declared as a friend as it is tightly coupled
     * with this class in that it is used to build the members of this class.
     */
    friend class CSRParser;

public:

    /**
     * Initialise the CSR analysis by parsing in a CSR specification.
     * The name used is to be consistent with prog.RTLInstDict
     */
    bool readCSRFile(const char* fileName, bool debug = false);

    /**
     * Constructor.
     */
    CSR();

    /**
     * Destructor.
     */
    ~CSR();

    /**
     * Given an logue name and a list of ordered values, return an
     * instantiated Logue.
     */
    Logue* instantiateLogue(const string name, vector<int>& params);

    /**
     * Recover parameters and return types for the given
     * procedure and all procedures underneath it in the
     * call graph that haven't already been analysed. Also determine
     * the outgoing paramaters and return values for each call
     * within the given procedure.
     * If spec is true, this is a speculative analysis
     */
    void analyse(UserProc* proc, FrontEndSrc& fe, bool spec);

    /*
     * Set the global substitutions
     */
    void setGlobalSubsts(map<SemStr, SemStr>& subs);

    /**
     * Output operator for a CSR object.
     */
    friend ostream& operator<<(ostream& os, const CSR& csr);

    /*
     * Set the return location for the given proc
     */
    void setReturnType(UserProc* proc, const Type& type);

    /*
     * Return true if the logue "none" exists
     */
    bool existsNoneLogue();

    /*
     * Get the outgoing parameter specification object
     */
    const Parameters& getOutgoingParamSpec() const { return outgoingParams;}

private: 

    /**
     * Do the substitution of registers to equivalent %afp/%agp relative
     * locations as specified in the FRAME/GLOBAL ABSTRACTION section of the
     * PAL spec.
     */
    void subAXP(UserProc* proc);

    /**
     * Analyse a given procedure as a caller. Finds actual parameters and
     * return locations
     * Return true if a change was made
     */
    bool analyseCaller(UserProc* proc);

    /*
     * More analysis as a caller, which replaces undefined actual parameters
     * with constant 0
     */
    void postAnalyseCaller(UserProc* proc);

public:
    /**
     * Analyse a given procedure as a callee. Using a set of locations that are
     * live at a call to this procedure, determine which locations are live into
     * this procedure (i.e. used before definition) and which locations are live
     * at the return from this procedure. This information is then saved in the
     * procedure object.
     * Return true if a change was made
     */
    bool analyseCallee(UserProc* proc);

    /*
     * Get the size of the OFFSET = parameter from the PAL spec
     * This is essentually sizeof(int) for the given architecture
     */
    int getIntSize();

    /*
     * Return true if the stack is inverted (grows away from 0) for this arch
     */
    bool isStackInverted() {return stackInverted;}

private:
    /**
     * Replace any references to locations in the given
     * procedure's parent's stack frame with variables.
     */
    void replaceParentStackReferences(UserProc* proc);

    /*
     * Guess the return type for a function (usually one that is speculatively
     * decoded, so we don't have access to the callers)
     */
    Type guessReturnType(UserProc* proc);

    /**
     * The logue templates parsed in from a CSR specification.
     */
    LogueDict logueDict;

    /**
     * The outgoing parameter specification (same for every procedure
     * when analysed as a caller).
     */
    Parameters outgoingParams;

    /**
     * The location in which returned values are found (same for every
     * procedure when analysed as a caller).
     */
    ReturnLocations retLocations;

    /*
     * The location to which the AFP is initialised.
     */
    SemStr afpInit;

    /*
     * The map from register(s) to expressions from the GLOBAL ABSTRACTION
     *  section. If this is not present in the .pal file, this will be empty
     */
    map<SemStr, SemStr> agpMap;

    /*
     * A mapping from locations to integers enabling
     * sets of locations to be represented and manipulated as bitsets.
     */
    LocationMap locMap;

    /*
     * True if the stack is inverted, i.e. newer stack frames are at larger
     * addresses (e.g. pa-risc)
     */
    bool stackInverted;
};

#ifdef WIN32
}
#endif
 
#endif
