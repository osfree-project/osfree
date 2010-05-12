/*==============================================================================
 * FILE:       csr.cc
 * OVERVIEW:   This file defines classes used to perform Call Signature
 *             Recovery (CSR) analysis; what is now called Procedural 
 *             Abstraction recovery.
 *
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.23 $
 * 07 May 99 - Mike: In CSRAnalysis::ReadCSRFile(), heed translator's path
 *                      so can run translate from any directory
 * 15 Jun 99 - Mike: Fixed debugging of CSR parser
 * 21 Dec 99 - Mike: Force main to return an integer (as far as the csr analysis
 *              is concerned)
 * 24 Feb 00 - Mike: Removed code which assumed that all parameters are the
 *               same size (in Parameters::filterLocations())
 * 13 Mar 00 - Mike: Initialise CalleePrologue::localsSize to "int 0" now.
 *               Thus, simple prologues which don't have any locals don't have
 *               to fake a zero size
 * 15 Mar 00 - Cristina: CSR::subAFP transformed into subAXP. 
 * 16 Mar 00 - Cristina: added CalleePrologue::setAGPSubstitutions.
 * 12 Apr 00 - Mike: ReturnLocations::filterLocations preserves type now
 * 18 Apr 00 - Mike: Added CSR::setReturnType()
 * 20 Apr 00 - Mike: missingParamCheck() takes an extra parameter to enable it
 *              to co-erce the size of parameters (needed for Pentium input)
 * 03 Jul 00 - Mike: cope with register calls being analysed now
 * 11 Jul 00 - Mike: Pass parameters to buildLiveOutSets now, so that the entry
 *              BB has its parameters live. Hopefully will fix the parameter
 *              pass through problem
 * 18 Jul 00 - Mike: Effectively reversed the above: can't find example where
 *              it's needed, and has nasty side effects for Pentium front end
 * 25 Aug 00 - Mike: Added valid() to easily test the returned pointer from
 *              findProc(). (If -1, was causing some segfaults)
 * 07 Sep 00 - Mike: getUseDefLocations takes a Proc* now, so we can register
 *              the use of the return location
 * 08 Sep 00 - Mike: Swapped the return value analysis and actual parameter
 *              analysis in analyseCaller(). This is because in some cases
 *              (e.g. returnparam), the return value analysis will turn up
 *              an additional parameter.
 * 14 Sep 00 - Mike: Big mods to parameter analysis code: analyse() calls
 *              analyseCallee and analyseCaller in a loop until no change
 * 18 Sep 00 - Mike: Cleaned up Parameters::firstParams() to handle register
 *              AND stack locations properly
 *              Also fixed analysis loop; setReturnType returns bool now
 * 19 Sep 00 - Mike: Fixed some code in Parameters::filterLocations that
 *              seg faulted when there are memory locations, but none pass
 *              through the filter. (Was a remnant of an invalid assertion)
 * 27 Sep 00 - Mike: Parameters::matches() no longer asserts if SemStr is empty
 * 06 Dec 00 - Mike: Fixed analyseCallee() situation like def_atk_nbr() in
 *              SPEC 099.go where use before defines are o0, o1, o2, o4,
 *              m[afp+k]; was getting 5 inst of 7 parameters, and the memory
 *              parameter wasn't being turned into a var
 * 18 Dec 00 - Mike: Added call to replaceParentStackAddresses()
 * 19 Dec 00 - Mike: Call checkReturnPass() to solve the problem of "passing on"
 *              the use of a return location (e.g. the returncallee test)
 * 21 Dec 00 - Mike: Removed the call to replaceParentStackAddress(); now
 *              done inside propagateSymbolics()
 * 11 Feb 01 - Nathan: Moved LocationMap implementation to util/locationmap.cc
 * 23 Feb 01 - Mike: Fixed a loop in missingParamCheck
 * 02 Mar 01 - Mike: Added Parameters::getIntSize()
 * 26 Mar 01 - Mike: If a proc has no epilogue, this is a warning, not an error
 *              now (analyseCallee()); also check for a "none" callee prologue
 * 31 Mar 01 - Mike: getFixedDest() returns NO_ADDRESS for non fixed addresses
 * 26 Apr 01 - Mike: setReturnType consults epilogue now
 * 15 May 01 - Mike: quelched a warning (with -O4) from missingParamCheck()
 * 18 Jul 01 - Mike: Changed addToSubstitutions() to allow overwriting (not
 *              used after all); missingParamCheck() copes with negative stack
 *              offsets
 * 01 Aug 01 - Mike: Changed GLOBAL SUBSTITUTIONS to be a list of substitutions
 *              (no longer tied to specific logues)
 * 01 Aug 01 - Mike: Parameters::missingParamCheck() OK with no register params
 * 13 Aug 01 - Mike: In Parameters::filterLocations(), Only emit parameters in
 *              reverse order if "STACK IS INVERTED"
 * 28 Aug 01 - Mike: missingParamCheck() and filterLocations are "slot based"
 * 30 Aug 01 - Mike: All procedure parameters changed from vector to list;
 *              added getParamSlot
 * 05 Sep 01 - Mike: missingParamCheck copes with aggregate pointer now
 * 12 Oct 01 - Mike: Added postAnalyseCaller to replace uninitialised actual
 *              params with const 0 (not currently active)
 * 15 Oct 01 - Mike: missingParamCheck is type insensitive now
 * 16 Oct 01 - Mike: Parameters::filterLocations is type insensitive now
 * 18 Oct 01 - Mike: getParamSlot returns correct size (1) for small types now
 * 22 Oct 01 - Mike: Only convert outgoing parameters to vars if a true stack
 *              based machine. Prevents code that depends on the order of locals
 * 26 Oct 01 - Mike: LocationMap is back to sign insensitive again
 */

//#define DEBUG_CSR             // Uncomment to debug this code

#include "global.h"
#include "csr.h"
#include "ss.h"
#include "cfg.h"
#include "proc.h"
#include "prog.h"               // For GetProgPath()
#include "csrparser.h"
#include "frontend.h"           // for decodeProc
#include "options.h"            // for progOptions

#ifdef WIN32
namespace NameSpaceCSR {
#endif


/*==============================================================================
 * FUNCTION:        valid
 * OVERVIEW:        Return true if the parameter is a valid proc pointer
 * NOTE:            Needed because Prog::findProc() can return -1 for procedures
 *                    that used to exist but are now deleted
 * PARAMETERS:      proc: pointer to proc, as returned from findProc()
 * RETURNS:         true if valid
 *============================================================================*/
bool valid(Proc* proc)
{
    if (proc == 0) return false;
    if (proc == (Proc*)-1) return false;
    return true;
}

/**********************************
 * Parameters methods
 **********************************/

/*==============================================================================
 * FUNCTION:        Parameters::Parameter
 * OVERVIEW:        Constructor. Given that a parameter spec may not
 *                  include registers or a stack part, either of the
 *                  parameters to this constructor may be NULL.
 * PARAMETERS:      aggregate - pointer to a SemStr representing the parameter
 *                    location for aggregates
 *                  int_regs - the registers where the integer parameters may
 *                    be found
 *                  float_regs - the registers where the float parameters may
 *                    be found (if different from int_regs)
 *                  double_regs - as above for doubles
 *                  stack1 - the base and offset of stack locations
 *                    included in this parameter spec
 *                  alignment - pointer to a map from LOC_TYPE to integer,
 *                    representing the alignment information for that LOC_TYPE
 * RETURNS:         <nothing>
 *============================================================================*/
Parameters::Parameters(SemStr* aggregate, const list<SemStr>* int_regs,
  const list<SemStr>* float_regs, const list<SemStr>* double_regs,
  const pair<SemStr,int>* stack1, const map<Type, int>* alignment)
{
    if (aggregate != NULL)
        this->aggregate = *aggregate;
    if (int_regs != NULL)
        int_registers = *int_regs;
    if (float_regs != NULL)
        float_registers = *float_regs;
    if (double_regs != NULL)
        double_registers = *double_regs;
    if (stack1 != NULL) {
        stackBase = stack1->first;
        offset = stack1->second;
    }
    if (alignment != NULL)
        this->alignment = *alignment;
}

/*==============================================================================
 * FUNCTION:        Parameters::instantiate
 * OVERVIEW:        Instantiate the variables in this spec with the
 *                  given mapping from variable names to values.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void Parameters::instantiate(vector<pair<string,int> >& nameValMap)
{
    for (unsigned i = 0; i < nameValMap.size(); i++) {
    
        // Create an expression that represents the current variable
        SemStr search;
        search.push(idParam);
        search.push(theSemTable.findItem(nameValMap[i].first));

        // Create an expression that represents the value of the
        // current variable
        SemStr replace;
        replace.push(idIntConst);
        replace.push(nameValMap[i].second);

        // Replace any uses of the current variable in the
        // aggregate spec
        aggregate.searchReplaceAll(search,replace);
        
        // Replace any uses of the current variable in the
        // stack base spec
        stackBase.searchReplaceAll(search,replace);
    }
    stackBase.simplify();
    aggregate.simplify();
}

/*==============================================================================
 * FUNCTION:        Parameters::~Parameters
 * OVERVIEW:        Destructor
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
Parameters::~Parameters()
{}

/*==============================================================================
 * FUNCTION:        Parameters::filterLocations
 * OVERVIEW:        Given a set locations, remove those that aren't valid
 *                  parameter locations.
 * PARAMETERS:      locs - a set of locations to be filtered
 *                  result - the ordered set of locations from locs that are
 *                    valid parameter locations in the order they would be used
 *                  locMap - a map from locations to integers (for BitSets)
 *                  stackInverted - true if the source machine has a stack that
 *                    grows away from zero
 *                  uniqueSlots - if true, the slot numbers for the given
 *                    parameters must be unique (else error)
 *                    For example, if this is true, then passing pa-risc
 *                    parameters r26 and fr4 (both slot 0) would be an error
 * RETURNS:         True if the aggregate location is found
 *============================================================================*/
bool Parameters::filterLocations(BITSET& locs, list<SemStr>& result,
    LocationMap& locMap, bool stackInverted, bool uniqueSlots) const
{
    // Set the aggregate pointer to be the first location in the
    // result if it is in locs
    bool aggregateUsed = false;
    if (aggregate.len() != 0 && locs.test(locMap.toBit(aggregate))) {

        aggregateUsed = true;
        result.push_back(aggregate);
        locs.set(locMap.toBit(aggregate),0);
    }

    // Partition the given locs into register locs and mem locs
    // These are all "sign insensitive" containers, so sign is not considered
    setSgiSemStr allLocs;
    locMap.toLocations(locs, allLocs);
    multimap<int, SemStr, lessSGI> regLocs;
    setSgiSemStr memLocs;
    set<int> usedSlots;                 // Keep track of the slot numbers
//  const Parameters& outgoing = prog.csrSrc.getOutgoingParamSpec();
    bool first = true;
    for (setSgiSemStr::iterator it1 = allLocs.begin(); it1 != allLocs.end();
      it1++) {
        if (it1->getFirstIdx() == idRegOf) {
            int slots, firstOff;
            int slotNum = getParamSlot(*it1, slots, first, firstOff);
            first = false;
            if (slotNum == -1)
                // Didn't even find it in the spec (can happen e.g. Sparc)
                continue;
            // Can't use [] for multimap... not even on LHS!
            regLocs.insert(pair<int, SemStr>(slotNum, *it1));
            if (uniqueSlots) {
                if (usedSlots.find(slotNum) != usedSlots.end()) {
                    ostrstream ost;
                    ost << "Filtering location " << *it1 << " overlaps another "
                        "parameter with slot " << dec << slotNum;
                    error(str(ost));
                }
            }
            usedSlots.insert(slotNum);
        }
        else if (it1->getFirstIdx() == idMemOf)
            memLocs.insert(*it1);
    }

    // For each register parameter, look for it in the appropriate list.
    // The fact that regLocs is a multimap from slot number to register
    // will guarantee the correct ordering
    multimap<int, SemStr, lessSGI>::const_iterator rit;
    for (rit = regLocs.begin(); rit != regLocs.end(); rit++) {
        const SemStr& ss = rit->second;
        LOC_TYPE lt = ss.getType().getType();
        if (lt == INTEGER) {
            list<SemStr>::const_iterator rr;
            for (rr = int_registers.begin(); rr != int_registers.end(); rr++) {
                if (*rr %= ss) {        // Compare, ignoring type
                    result.push_back(ss);
                    break;
                }
            }
        } else {
            int size = ss.getType().getSize();
            if (size == 32) {
                if (find(float_registers.begin(), float_registers.end(), ss)
                  != float_registers.end())
                    result.push_back(ss);
            } else if (size == 64) {
                if (find(double_registers.begin(), double_registers.end(), ss)
                  != double_registers.end())
                    result.push_back(ss);
            }
        }
    }
    if (memLocs.size() == 0)
        return aggregateUsed;

    // Only continue if there are "real" memory locations live (not the aggreg-
    // ate pointer, if it's a memof)
    if (aggregateUsed && (aggregate.getFirstIdx() == idMemOf) &&
      memLocs.size() == 1)
        return aggregateUsed;

    // We have real memory locations; make sure all the registers are in
    // the result
    // Example: Sparc parameters passed in o0, o1, o2, o3, o4, m[afp+k]. Then
    // result.size() == 5, and registers.size() == 6. But there are still
    // memory locations to check, and we need another parameter for the
    // unused parameter register o5
    unsigned numRegs = int_registers.size();
    unsigned numDone = usedSlots.size();
    list<SemStr>::const_reverse_iterator rrit = int_registers.rbegin();
    unsigned u;
    for (u=numDone; u < numRegs; u++)
        rrit++;
    for (u=numDone; u < numRegs; u++)
        result.push_back(*--rrit);

    // If all of the register locations were used, then now find the stack
    // locations that are used. For X86 etc, there are no register locations,
    // hence no unused register locations.
    // Create a map from each memory location (which
    // should also be a stack location) to its offset from the
    // base. Then iterate through this map starting at the lowest
    // offset adding the locations to the result until a
    // difference in offsets not equal to the OFFSET= amount
    // is found.
    map<int,const SemStr*> offset_locs;
    for (setSgiSemStr::iterator it4 = memLocs.begin(); it4 != memLocs.end();
      it4++) {

        int loc_offset;
        if (getOffset(*it4,loc_offset)) {
            offset_locs[loc_offset] = &(*it4);
        }
        else {
            // This message isn't valid at least for sparc. Whenever
            // there are incoming and outgoing memory parameters, this
            // message comes up needlessly. (For example, see the "parms"
            // test program). For now, it is commented out
            // error("mem loc must be a valid stack parameter loc");
        }
    }
    // Note that we want to output the memory locations in order that parameters
    // are passed, because of the way they are specified in a .pal file.
    // Usually, this will be in increasing order if the stack grows towards
    // zero (the normal case), but e.g. pa/risc has [%afp -52] and offset of -4,
    // so we want -52, -56, etc for the first, second, etc parameters
    // Note: X86 has offset < 0 (for callers), and yet the stack is not inverted
    if (!stackInverted) {
        map<int,const SemStr*>::iterator it5  = offset_locs.begin();
        while(it5 != offset_locs.end()) {
            result.push_back(*(it5->second));
            it5++;
        }
    } else {
        // STACK IS INVERTED case
        map<int,const SemStr*>::reverse_iterator it5 = offset_locs.rbegin();
        while (it5 != offset_locs.rend()) {
            result.push_back(*(it5->second));
            it5++;
        }
    }
    return aggregateUsed;
}

/*==============================================================================
 * FUNCTION:        Parameters::getOffset
 * OVERVIEW:        Given a location, return true if it is a memory
 *                  location that can be reduced to being at a
 *                  POSITIVE multiple of my offset from my stack incoming
 *                  parameter base. Return the offset in the given reference
 *                  parameter.
 * PARAMETERS:      loc - a location to test
 *                  loc_offset - offset from the start of stack incoming params
 * RETURNS:         true if loc is a memory location at a multiple of
 *                  this.offset from this.base
 *============================================================================*/
bool Parameters::getOffset(const SemStr& loc, int& loc_offset) const
{
    if (stackBase.len() == 0)
        return false;

    // Subtract the current memory location for the base and simplify it. If
    // it simplifies to a constant and that constant is a multiple of the
    // offset, then the memory location is a possible stack parameter
    // location
    SemStr offsetExpr(loc);

    // Strip the enclosing memOf expression
    offsetExpr.getSubExpr(0,offsetExpr);
    offsetExpr.prep(idMinus);
    offsetExpr.pushSS(&stackBase);
    offsetExpr.simplify();

    if (offsetExpr.getFirstIdx() == idIntConst) {
        loc_offset = offsetExpr.getSecondIdx();
        if (loc_offset % offset == 0 && loc_offset / offset >= 0)
            return true;
    }
    return false;
}

/*==============================================================================
 * FUNCTION:        Parameters::matches
 * OVERVIEW:        Given a single location, return true if it *could* be a
 *                  parameter location. Ignores order in which locations must be
 *                  used to be valid parameter locations.
 * PARAMETERS:      loc - a location
 * RETURNS:         loc is a valid parameter location
 *============================================================================*/
bool Parameters::matches(const SemStr& loc) const
{
    if (loc.len() == 0)
        // Can happen e.g. Pentium POP psedo instruction has no LHS
        return false;

    if (aggregate.len() != 0) {
        SemStr copy_loc(loc);
        if (copy_loc == aggregate)
            return true;
    }
                
    switch (loc.getFirstIdx()) {

        // If it is a register location, test for it in the list of register
        // locations (depending on type)
        case idRegOf:
        {
            LOC_TYPE basic = loc.getType().getType();
            if (basic == INTEGER) {
                for (registers_CIT it = int_registers.begin();
                  it != int_registers.end(); it++) {
                    // Sign insensitive compare
                    if ((*it) -= loc)
                        return true;
                }
            } else if (basic == FLOATP) {
                int size = loc.getType().getSize();
                if (size == 32) {
                    for (registers_CIT it = float_registers.begin();
                      it != float_registers.end(); it++) {
                        // Sign insensitive compare
                        if ((*it) -= loc)
                            return true;
                    }
                } else if (size == 64) {
                    for (registers_CIT it = double_registers.begin();
                      it != double_registers.end(); it++) {
                        // Sign insensitive compare
                        if ((*it) -= loc)
                            return true;
                    }
                }
            }
            return false;
        }

        // If it is a memory location, see if it equals the aggregate
        // pointer or the stack base plus some *positive* multiple of
        // the offset
        case idMemOf:
        {
            int loc_offset;
            return (getOffset(loc,loc_offset));
        }
        default:
            return false;
    }
}

/*==============================================================================
 * FUNCTION:        Parameters::addParams
 * OVERVIEW:        Add n more parameters to a given list of parameters.
 *                  NOTE: Only adds register parameters as there
 *                  should never really be a need to add more than one
 *                  parameter (a returned result) to correct programs.
 * PARAMETERS:      n - the number of parameters to add
 *                  params - the list of parameters to extend
 * RETURNS:         <nothing>
 *============================================================================*/
void Parameters::addParams(int n, list<SemStr>& params) const
{
    if (params.size() + n > int_registers.size()) {
        ostrstream ost;
        ost << "adding parameters requires adding stack parameters ";
        ost << "which can't be done reliably for some architectures";
        error(str(ost));
        return;
    }

    registers_CIT first_reg = int_registers.begin();
    advance(first_reg,params.size());
    for (registers_CIT it = first_reg; n != 0; it++, n--)
        params.push_back(*it);
}

/*==============================================================================
 * FUNCTION:        Parameters::firstParams
 * OVERVIEW:        Return a list of semantic strings representing the first
 *                  n parameters
 * NOTE:            The returned vector should be deleted after use
 * PARAMETERS:      n - the number of parameters to put in the list
 * RETURNS:         Pointer to a new list
 *============================================================================*/
list<SemStr>* Parameters::firstParams(int n) const
{
    list<SemStr>* lst = new list<SemStr>;
    int i=0;                // i is the number already added to lst
    if (int_registers.size()) {
        // This architecture passes (at least some) parameters in registers
        registers_CIT it;
        for (it = int_registers.begin(); (i < n) && (it != int_registers.end());
          i++, it++)
            lst->push_back(*it);
    }
    // Use stack locations
    for ( ; i < n; i++) {
        SemStr loc(stackBase);
        loc.prep(idMemOf);
        // m[ + %afp int Const]
        // 0  1  2    3    4
        // Want to add offset*i to const
        int Const = loc.getIndex(4);
        loc.substIndex(4, Const+offset*i);
            lst->push_back(loc);
    }
    return lst;
}

/*==============================================================================
 * FUNCTION:        Parameters::conformParamSize
 * OVERVIEW:        Increase the size of the parameter if necessary to conform
 *                    to the minimum size as given in the .pal file (PARAMETERS
 *                    OFFSET)
 * PARAMETERS:      size - current size of a parameter; may be altered
 * RETURNS:         <nothing>
 *============================================================================*/
void Parameters::conformParamSize(int& size) const
{
    if (size < (abs(offset))) {
        size = abs(offset);
    }
}

/*==============================================================================
 * FUNCTION:        Parameters::getParamSlot
 * OVERVIEW:        Get the "slot index" for this parameter location.
 *                    A parameter slot holds one parameter of size <= one
 *                    machine word. If the parameter would use more than one
 *                    slot, the lower slot index is returned
 * NOTE:            Slot numbers are not the same as parameter numbers, because
 *                    of (a) parameters larger than one slot, and (b) gaps due
 *                    to alignment constraints
 * NOTE:            Don't call this function with a var expression (idVar)
 * PARAMETERS:      loc: Reference to the SemStr representing the parameter
 *                    location
 *                  slots: Reference to an int which will receive the size
 *                    (in slots) of the parameter
 *                  first: set to true if this is the first offset (see below)
 *                  firstOffset: an integer that must be allocated by the
 *                    caller; no need to initialise it
 * RETURNS:         An int representing the first slot index, or -1 if this is
 *                    not a valid parameter location
 *============================================================================*/
int Parameters::getParamSlot(const SemStr& loc, int& slots, bool first,
    int& firstOff) const
{
    int idx = loc.getFirstIdx();
    if (idx == idMemOf) {
        int loc_offset, thisOff;
        if (getOffset(loc, loc_offset)) {
            // Compare the sign of offset with the direction of stack growth
            if ((offset < 0) != prog.csrSrc.isStackInverted()) {
                // The slot numbers will be wrong, unless great care is taken.
                // This is essentially because the parameters are pushed in
                // reverse order to their use.
                // We ensure that parameters are processed in the order of
                // increasing slot number.
                // Example: pentium actual params [%afp-12], [%afp-8], [%afp-4]
                // First call has first set to true, we extract -12 from the
                // loc, save this in firstOff, and return 0.
                // Next call, first is false and we return (-12 - -8)/(-4) = 1.
                // Last call, we return (-12 - -4)/(-4) = 2.
                // Assume m[ +/- %afp int k
                //         0  1    2   3  4
                assert(loc.getIndex(3) == idIntConst);
                if (loc.getSecondIdx() == idPlus)
                    thisOff = loc.getIndex(4);
                else if (loc.getSecondIdx() == idMinus)
                    thisOff = - loc.getIndex(4);
                else assert(0);
                if (first) {
                    firstOff = thisOff;
                    loc_offset = 0;
                } else
                    loc_offset = firstOff - thisOff;
            }
            slots = loc.getType().getSize() / 8 / abs(offset);
            if (slots == 0)
                // Small types still occupy one slot
                slots = 1;
            // loc_offset has the distance from the start of stack parameters
            return (loc_offset / offset) + int_registers.size();
        } else
            // An invalid memory location
            return -1;
    } else if (idx == idRegOf) {
        const Register& reg = prog.RTLDict.DetRegMap[loc.getThirdIdx()];
        // Want size to be in terms of words. Word size is abs(offset) bytes
        slots = reg.g_size() / 8 / abs(offset);
        // Get the type of the register. Assume it has been properly typed
        int i=0;
        registers_CIT it;
        switch (loc.getType().getType()) {
            case INTEGER:
                for (it = int_registers.begin(); it != int_registers.end();
                  it++, i++) {
                    if (loc %= *it)         // Compare, disregarding type
                        return i;
                }
                return -1;
            case FLOATP: {
                int sz = loc.getType().getSize();
                if (sz == 32) {
                    for (it = float_registers.begin();
                      it != float_registers.end(); it++, i++) {
                        if (loc %= *it)
                            return i;
                    }
                    return -1;
                } else if (sz == 64) {
                    for (it = double_registers.begin();
                      it != double_registers.end(); it++, i++) {
                        if (loc %= *it)
                            return i;
                    }
                    return -1;
                } else return -1;
            }
            default:
                return -1;
        }
    }
    // Not a register or memory; could be var
    return -1;
}

/*==============================================================================
 * FUNCTION:        Parameters::getActParamLoc
 * OVERVIEW:        Get a location for the actual parameter location, with the
 *                    given type and slot number
 * NOTE:            Caller must delete result when done with it
 * PARAMETERS:      ty - Type for the parameter
 *                  slot - slot number (0=first)
 * RETURNS:         Pointer to a new SemStr with the location
 *============================================================================*/
const SemStr* Parameters::getActParamLoc(const Type& ty, int slot) const
{
    SemStr* ret = new SemStr(ty);
    const list<SemStr>* pRegs = NULL;
    int slotsPerReg = 0;
    if (ty.getType() == FLOATP) {
        if (ty.getSize() == 32) {
            pRegs = &float_registers;
            slotsPerReg = 4 / offset;   // Assume float is 4 bytes
            if (slotsPerReg == 0) slotsPerReg = 1;  // In case offset = 64 bits
        }
        else {
            pRegs = &double_registers;
            slotsPerReg = 8 / offset;
        }
    } else {
        pRegs = &int_registers;
        slotsPerReg = 1;
    }

    // Find number of slots used up by registers
    int numRegSlots = pRegs->size() * slotsPerReg;
    if (slot < numRegSlots) {
        // Then the indicated slot number is a register
        list<SemStr>::const_iterator it = pRegs->begin();
        for (int i=0; i < slot; i += slotsPerReg)
            it++;
        return new SemStr(*it);
    } else {
        // We are into the stack locations
        // Generate m[ + %afp int k
        // where k is a constant
        // Assume stackBase is of the form + %afp int b
        //                                 0   1   2  3
        int base = stackBase.getIndex(3);
        // Or possibly - %afp int b
        //             0   1   2  3
        if (stackBase.getFirstIdx() == idMinus)
            base = -base;
        int k = base + (slot - numRegSlots) * offset;
        *ret << idMemOf << idPlus << idAFP << idIntConst << k;
    }
    return ret;
}

/*==============================================================================
 * FUNCTION:        operator<<
 * OVERVIEW:        Outputs operator for a Parameters object.
 * PARAMETERS:      os - the stream to write to
 *                  cp - the Parameters object to write
 * RETURNS:         a reference to os
 *============================================================================*/
ostream& operator<<(ostream& os, const Parameters& cp)
{
    // Print the aggregate pointer location
    if (cp.aggregate.len() != 0)
        os << "\taggregate: " << cp.aggregate << endl;

    // Print the register locations
    if (cp.int_registers.size() != 0) {
        os << "\tinteger registers: ";
        for (list<SemStr>::const_iterator it = cp.int_registers.begin();
            it != cp.int_registers.end(); it++)
            os << *it << " ";
        os << endl;
    }

    if (cp.float_registers.size() != 0) {
        os << "\tfloat registers: ";
        for (list<SemStr>::const_iterator it = cp.float_registers.begin();
            it != cp.float_registers.end(); it++)
            os << *it << " ";
        os << endl;
    }

    if (cp.double_registers.size() != 0) {
        os << "\tdouble registers: ";
        for (list<SemStr>::const_iterator it = cp.double_registers.begin();
            it != cp.double_registers.end(); it++)
            os << *it << " ";
        os << endl;
    }

    // Print the stack base and offset
    if (cp.stackBase.len() != 0) {
        os << "\tstack: ";
        os << "base =  " << cp.stackBase;
        os << dec << ", offset = " << cp.offset;
        os << endl;
    }

    return os;
}

/*==============================================================================
 * FUNCTION:        alignSlotNumber
 * OVERVIEW:        Change the given slot number if necessary for alignment
 *                    purposes
 *                  For example: pa-risc, slot=1, ty=double: slot will be
 *                    changed to 2 to doubleword align it
 * PARAMETERS:      slot: Ref to a slot number of a parameter (0=first)
 *                  ty: reference to the Type for the parameter
 * RETURNS:         Nothing, but reference parameter slot may be changed
 *============================================================================*/
void Parameters::alignSlotNumber(int& slot, const Type& ty) const
{
    map<Type, int>::const_iterator mm;
    mm = alignment.find(ty);
    if (mm != alignment.end()) {
        int a = mm->second;
        slot +=  (a-1);         // Align to a-slot boundary
        slot &= ~(a-1);         // E.g. a=4, add 3 and AND with F...FC
    }
}

/*==============================================================================
 * FUNCTION:        Parameters::missingParamCheck
 * OVERVIEW:        Check the vector of parameters for gaps. If so, insert
 *                    appropriate extra formal parameter(s) to fill the gaps
 * NOTE:            This procedure is "slot based" now (complete rewrite)
 * NOTE:            This procedure works on formal parameters only
 * PARAMETERS:      params - vector of SemStr to check
 *                  proc - Ptr to UserProc object for this procedure
 *                  outgoingParams - ref to a Parameters object that encaps-
 *                    ulates the outgoing parameter calling conventions
 *                  aggUsed - true if the aggregate return location is used
 * RETURNS:         <nothing>, but params may be changed as a side effect
 *                    May also change proc (allocate a new local)
 *============================================================================*/
void Parameters::missingParamCheck(list<SemStr>& params, UserProc* proc,
    const Parameters& outgoingParams, bool aggUsed) const
{
    int currSlot = -1;              // Current parameter slot number
    int currSize = 1;               // Size of current parameter, in slots
    list<SemStr>::iterator it = params.begin();
    if (aggUsed) {
        // This means that the first parameter is the aggregate pointer, and is
        // "inserted" before what was the first parameter (according to normal
        // ABI rules) for the source architecture
        // Just assume that the first parameter is the aggregate pointer
        if (it != params.end()) it++;
    }
    bool first = true; int firstOff;
    for (; it != params.end(); it++) {
        currSlot += currSize;
        // MVE: Again, outgoingParams.getParamSlot... was there a good reason
        // for this? outgoingParams is now an unused parameter
        // int slot = outgoingParams.getParamSlot(*it, currSize);
        int slot = getParamSlot(*it, currSize, first, firstOff);
        first = false;
        // Perform alignment, if needed
        Type& ty = it->getType();
        alignSlotNumber(currSlot, ty);
        if (slot != currSlot) {
            // Insert a var as a "spacer" formal parameter
            // Use a var, because duplicating a formal parameter won't compile
            int wantSize = slot - currSlot;
            assert(wantSize > 0);
            // Make a type of the required size
            LOC_TYPE lt = INTEGER;
            if (wantSize > 2) lt = FLOATP;
            Type ty(lt, wantSize*8*abs(offset), true);
            SemStr* dummy = proc->newLocal(ty);
            ostrstream ost;
            ost << "Adding extra formal parameter " << *dummy;
            ost << " to procedure " << proc->getName();
            warning(str(ost));
            params.insert(it, *dummy);
            delete dummy;
            currSlot += wantSize;
        }
    }
}

/*==============================================================================
 * FUNCTION:        Parameters::getIntSize
 * OVERVIEW:        Get the absolute value of the OFFSET = parameter from the
 *                    PAL spec. This is essentually sizeof(int) for the given
 *                    architecture
 * PARAMETERS:      None
 * RETURNS:         The size in bytes (e.g. 2 for mc68328)
 *============================================================================*/
int Parameters::getIntSize() const
{
    return abs(offset);
}

/**********************************
 * ReturnLocation methods
 **********************************/

/*==============================================================================
 * FUNCTION:        ReturnLocations::ReturnLocations
 * OVERVIEW:        Constructor that set the mapping of types to locations.
 * PARAMETERS:      mapping - a map from types to locations
 * RETURNS:         <nothing>
 *============================================================================*/
ReturnLocations::ReturnLocations(const typeToSemStrMap& mapping)
{
    type_locs = mapping;
    for (type_locs_CIT it = type_locs.begin(); it != type_locs.end();it++)
        locs.insert(it->second);
}

/*==============================================================================
 * FUNCTION:        ReturnLocations::filterLocations
 * OVERVIEW:        Given a set of locations, remove those that aren't
 *                  valid return value locations.
 *                  locMap - a map from locations to integers (for BitSets)
 * PARAMETERS:      locs - a set of locations
 *                  result: a list of SemStr to store the results in
 *                  locMap: a map needed to make sense of the BITSET
 * RETURNS:         <nothing>
 *============================================================================*/
void ReturnLocations::filterLocations(BITSET& locs, list<SemStr>& result,
    LocationMap& locMap) const
{
    setSgiSemStr set_locs;
    locMap.toLocations(locs, set_locs);
    for (locs_CIT it = this->locs.begin(); it != this->locs.end(); it++) {
        setSgiSemStr::const_iterator itf = set_locs.find(*it);
        if (itf != set_locs.end())
            result.push_back(*itf);
    }
}

/*==============================================================================
 * FUNCTION:        ReturnLocations::locationFor
 * OVERVIEW:        Return the location used for a given type or NULL if t is an
 *                  invalid return type.
 * PARAMETERS:      t - a location type
 * RETURNS:         the location used for t or NULL is t is an invalid return
 *                  type
 *============================================================================*/
const SemStr* ReturnLocations::locationFor(const Type& t) const
{
    type_locs_CIT it = type_locs.find(t);
    if (it != type_locs.end()) {
        return &(it->second);
    }
    return NULL;
}

/*==============================================================================
 * FUNCTION:        ReturnLocations::typeFor
 * OVERVIEW:        Return the type that uses the given location. The
 *                  type returned is the first type in the map (which
 *                  is ordered by type) which corresponds to the given
 *                  location.
 * PARAMETERS:      loc - a location
 * RETURNS:         the type that uses the given location
 *============================================================================*/
const Type& ReturnLocations::typeFor(const SemStr& loc) const
{
    for (type_locs_CIT it = type_locs.begin(); it != type_locs.end(); it++)
        // Note: we want a sign insensitive comparison, since we don't specify
        // sizes in the PAL spec. So for mc68k, r0 is for Int and Short
        if (it->second -= loc)      // Compare, not "minus equals"
            return it->first;
    return *new Type(UNKNOWN);
}

/*==============================================================================
 * FUNCTION:        ReturnLocations::toBitset
 * OVERVIEW:        Return the set of return locations as a bitset.
 * PARAMETERS:      locMap - a map from locations to integers (for BitSets)
 * RETURNS:         the sets of locations represented by this object
 *============================================================================*/
BITSET ReturnLocations::toBitset(LocationMap& locMap) const
{
    return locMap.toBitset(locs);
}

/*==============================================================================
 * FUNCTION:        ReturnLocations::matches
 * OVERVIEW:        Return true if the given location is within the
 *                  return location spec.
 * PARAMETERS:      loc - location to match with
 * RETURNS:         True if given location matches the return location spec
 *============================================================================*/
bool ReturnLocations::matches(const SemStr& loc) const
{
    return (locs.find(loc) != locs.end());
}

/*==============================================================================
 * FUNCTION:        operator<<
 * OVERVIEW:        Outputs operator for a ReturnLocations object.
 * PARAMETERS:      os - the stream to write to
 *                  cp - the ReturnLocations object to write
 * RETURNS:         a reference to os
 *============================================================================*/
ostream& operator<<(ostream& os, const ReturnLocations& rl)
{
    for (map<Type,SemStr>::const_iterator it = rl.type_locs.begin();
      it != rl.type_locs.end(); it++)
        os << "\t" << it->first.getCtype() << " in " << it->second << endl;
    return os;
}


/**********************************
 * ParentStack methods
 **********************************/

/*==============================================================================
 * FUNCTION:        ParentStack::ParentStack
 * OVERVIEW:        Default constructor.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
ParentStack::ParentStack() :
    range(0), step(0)
{
}

/*==============================================================================
 * FUNCTION:        ParentStack::ParentStack
 * OVERVIEW:        Constructor.
 * PARAMETERS:      low - the low end of the range in the parents's
 *                   that is writeable to.
 *                  high - the high end of the range in the parent's
 *                   that is writeable to.
 *                  step - the minimum size in bytes of quantities
 *                    which may be accessed in the range
 * RETURNS:         <nothing>
 *============================================================================*/
ParentStack::ParentStack(const SemStr& low, const SemStr& high, int step) :
    low(low), step(step)
{
    // Take low away from high to get the size of the range
    SemStr diff(high);
    diff.pushSS(&low);
    diff.prep(idMinus);
    diff.simplify();
    assert(diff.len() == 2 && diff.getFirstIdx() == idIntConst);
    range = diff.getSecondIdx();
    assert (range > 0 && range % step == 0);
}

/*==============================================================================
 * FUNCTION:        ParentStack::instantiate
 * OVERVIEW:        Instantiate the variables in this spec with the
 *                  given mapping from variable names to values.
 * PARAMETERS:      nameValuMap - a map from name to values
 * RETURNS:         <nothing>
 *============================================================================*/
void ParentStack::instantiate(vector<pair<string,int> >& nameValMap)
{
    for (unsigned i = 0; i < nameValMap.size(); i++) {
    
        // Create an expression that represents the current variable
        SemStr search;
        search.push(idParam);
        search.push(theSemTable.findItem(nameValMap[i].first));

        // Create an expression that represents the value of the
        // current variable
        SemStr replace;
        replace.push(idIntConst);
        replace.push(nameValMap[i].second);

        // Replace any uses of the current variable in low
        low.searchReplaceAll(search,replace);
    }
    low.simplify();
}

/*==============================================================================
 * FUNCTION:        ParentStack::getRange
 * OVERVIEW:        Returns the range of locations defined by this spec.
 * PARAMETERS:      locs - the vector to return the range in
 * RETURNS:         <nothing>
 *============================================================================*/
void ParentStack::getRange(vector<SemStr>& locs)
{
    assert(locs.size() == 0);

    if (step == 0 || range == 0)
        return;
 
    // Now go through the range and insert the locations within it
    for (int i = 0; i <= range; i += step) {

        locs.push_back(low);

        // Add the current position in the range to the newly inserted
        // location and convert the final result to a memory index
        // expression
        SemStr& loc = locs.back();
        loc.push(idIntConst);
        loc.push(i);
        loc.prep(idPlus);
        loc.simplify();
        loc.prep(idMemOf);
    }
}

/*==============================================================================
 * FUNCTION:        operator<<
 * OVERVIEW:        Output operator for a ParentStack object.
 * PARAMETERS:      os -
 *                  cs -
 * RETURNS:         
 *============================================================================*/
ostream& operator<<(ostream& os, const ParentStack& cs)
{
    if (cs.low.len() == 0)
        return os;

    SemStr high(cs.low);
    high.prep(idPlus);
    high.push(idIntConst);
    high.push(cs.range);
    high.simplify();


    return os << cs.low << " .. " << high << " STEP " << cs.step << endl;
}

/**********************************
 * Logue methods
 **********************************/

int Logue::nextOrderPos = 1;
const string Logue::TYPE_NAMES[4] = { "CALLER_PROLOGUE", "CALLEE_PROLOGUE",
    "CALLEE_EPILOGUE", "CALLER_EPILOGUE"};

/*==============================================================================
 * FUNCTION:        Logue::Logue
 * OVERVIEW:        Constructor.
 * PARAMETERS:      name - the name of the logue
 *                  params - the parameters of the logue
 * RETURNS:         <nothing>
 *============================================================================*/
Logue::Logue(const string& name, Logue::TYPE type,
    const list<string>& params) :
        name(name), type(type), params(params.size()),
        instantiated(false)
{
    assert(params.size() == this->params.size());

    // Copy the parameters into the vector of pairs
    list<string>::const_iterator it = params.begin();
    params_IT my_it = this->params.begin();
    for (;it != params.end(); it++, my_it++)
        my_it->first = *it;

    orderPos = nextOrderPos++;
}

/*==============================================================================
 * FUNCTION:        Logue::getName
 * OVERVIEW:        Get the name of this logue.
 * PARAMETERS:      <none>
 * RETURNS:         the name of this logue
 *============================================================================*/
string Logue::getName() const
{
    return name;
}

/*==============================================================================
 * FUNCTION:        Logue::getType
 * OVERVIEW:        Get the type of this logue.
 * PARAMETERS:      <none>
 * RETURNS:         The type of this logue, e.g. CALLEE_PROLOGUE
 *============================================================================*/
Logue::TYPE Logue::getType() const
{
    return type;
}

/*==============================================================================
 * FUNCTION:        Logue::getFormals
 * OVERVIEW:        Return the names of the formal parameters of this
 *                  logue in the given set.
 * PARAMETERS:      formals - the set to return the names in
 * RETURNS:         <nothing>
 *============================================================================*/
void Logue::getFormals(set<string>& formals) const
{
    for (params_CIT it = params.begin();
        it != params.end(); it++)
        formals.insert(it->first);
}

/*==============================================================================
 * FUNCTION:        Logue::getOrder
 * OVERVIEW:        The position of this logue in a partial ordering between all
 *                  logues.
 * PARAMETERS:      <none>
 * RETURNS:         the order of this logue
 *============================================================================*/
int Logue::getOrder() const
{
    return orderPos;
}

/*==============================================================================
 * FUNCTION:        Logue::instantiate
 * OVERVIEW:        Make a copy of this logue and complete its actuals with
 *                  the given values. This methods assumes that the number of
 *                  given values equals the number of actuals.
 * PARAMETERS:      values - the values to instantiate with
 * RETURNS:         an instantiated logue
 *============================================================================*/
void Logue::instantiate(vector<int>& values)
{
    instantiated = true;

    if (params.size() != values.size()) {

        ostrstream ost;
        ost << "too " << (params.size() > values.size() ? "few" :"many");
        ost << "arguments to logue `" << name << "'";
        error(str(ost));
    }

//cerr << "instantiation for `" << name << "':" << endl;
    // Instantiate the parameter values
    for (unsigned i = 0; i < params.size(); i++) {
        params[i].second = values[i];

//cerr << "  " << params[i].first << " = ";
//cerr << values[i] << endl;
    }
}

/*==============================================================================
 * FUNCTION:        Logue::put
 * OVERVIEW:        Output a logue to an ostream.
 * PARAMETERS:      os - output stream to use.
 * RETURNS:         the given output stream
 *============================================================================*/
ostream& Logue::put(ostream& os) const
{

    // Display the parameter info
    os << TYPE_NAMES[type] << " " << name << "(";
    for (params_CIT it = params.begin(); it != params.end(); it++) {

        // put the separator in if this isn't the first parameter
        if (it != params.begin())
            os << ", ";

        // Show the name of the param
        os << it->first;

        // Show the value of the param if this is an instantiated
        // logue
        if (instantiated)
            os << "=" << it->second;
    }
    os << ")" << endl;

    os << "order = " << orderPos << endl;

    return os;
}

/*==============================================================================
 * FUNCTION:        operator<<
 * OVERVIEW:        Output operator for a Logue.
 * PARAMETERS:      os - output stream to use.
 * RETURNS:         the given output stream
 *============================================================================*/
ostream& operator<<(ostream& os, const Logue& l)
{
    l.put(os);
    return os;
}

/**********************************
 * CalleePrologue methods
 **********************************/

/*==============================================================================
 * FUNCTION:        CalleePrologue::CalleePrologue
 * OVERVIEW:        Constructor.
 * PARAMETERS:      name - the name of the prologue
 *                  params - the parameters of the prologue
 * RETURNS:         <nothing>
 *============================================================================*/
CalleePrologue::CalleePrologue(const string& name,
    const list<string>& params) :
    Logue(name,CALLEE_PROLOGUE,params)
{
    // Initialise the locals size to zero, in case there is no LOCALS section
    // for this prologue (some prologues don't imply local variables at all)
    // This can be overridden by setLocalsSize()
    localsSize.push(idIntConst);
    localsSize.push(0);
}

/*==============================================================================
 * FUNCTION:        CalleePrologue::setParamSpec
 * OVERVIEW:        Record the info about where incoming parameter
 *                  will be found.
 * PARAMETERS:      paramSpec - a location filter that specifies where
 *                    the incoming parameters of a procedure with this
 *                    prologue can be found
 * RETURNS:         <nothing>
 *============================================================================*/
void CalleePrologue::setParamSpec(const Parameters& paramSpec)
{
    incomingParams = paramSpec;
}

/*==============================================================================
 * FUNCTION:        CalleePrologue::getParamSpec
 * OVERVIEW:        Get a pointer to the incoming parameters' spec.
 * PARAMETERS:      <none>
 * RETURNS:         pointer to the incoming parameters' spec
 *============================================================================*/
Parameters* CalleePrologue::getParamSpec()
{
    return &incomingParams;
}

/*==============================================================================
 * FUNCTION:        CalleePrologue::setSubstitutions
 * OVERVIEW:        Set the substitutions that must be effected after
 *                  execution of this prologue.
 * PARAMETERS:      subs - the substitutions
 * RETURNS:         <nothing>
 *============================================================================*/
void CalleePrologue::setSubstitutions(const map<SemStr,SemStr>& subs)
{
    substitutions = subs;
}

/*==============================================================================
 * FUNCTION:        CalleePrologue::addToSubstitutions
 * OVERVIEW:        Add the substitutions of this prologue to a given map.
 * PARAMETERS:      subs - the map to add to
 * RETURNS:         <nothing>
 *============================================================================*/
void CalleePrologue::addToSubstitutions(map<SemStr,SemStr>& subs)
{
    /* Note: used to use the two operator insert() function, but this does not
       allow overwriting of existing elements of the map (it seems).
        Example: (from early version of pa-risc hppa.pal)
        FRAME ABSTRACTION
          INIT = %sp
          logue
          {
              %r3 -> %afp
              %sp -> %afp + locals
          }
        Here, %sp changes from being just %afp (e.g. no logue), to being
        %afp+locals with the given logue
    */

    for (map<SemStr,SemStr>::iterator it=substitutions.begin(); 
      it!=substitutions.end(); it++)
        subs[it->first] = it->second;
}

/*==============================================================================
 * FUNCTION:        CalleePrologue::setLocalsSize
 * OVERVIEW:        Sets the expression that represents the amount of space
 *                  allocated for local variables.
 * PARAMETERS:      localsSize -
 * RETURNS:         <nothing>
 *============================================================================*/
void CalleePrologue::setLocalsSize(const SemStr& localsSize)
{
    this->localsSize = localsSize;
}

/*==============================================================================
 * FUNCTION:        CalleePrologue::getLocalsSize
 * OVERVIEW:        Get the amount of space allocated for local variables.
 * PARAMETERS:      <none>
 * RETURNS:         the amount of space in bytes
 *============================================================================*/
int CalleePrologue::getLocalsSize()
{
    assert(localsSize.len() == 2 && localsSize.getFirstIdx() == idIntConst);
    return abs(localsSize.getSecondIdx());
}

/*==============================================================================
 * FUNCTION:        CalleePrologue::setParentStackSpec
 * OVERVIEW:        Record the info about which part of a calller's
 *                  stack may be written to.
 * PARAMETERS:      parentStack -
 * RETURNS:         <nothing>
 *============================================================================*/
void CalleePrologue::setParentStackSpec(const ParentStack& parentStack)
{
    this->parentStack = parentStack;
}

/*==============================================================================
 * FUNCTION:        CalleePrologue::getParentStackSpec
 * OVERVIEW:        Get a pointer to the parent stack spec.
 * PARAMETERS:      <none>
 * RETURNS:         
 *============================================================================*/
ParentStack* CalleePrologue::getParentStackSpec()
{
    return &parentStack;
}

/*==============================================================================
 * FUNCTION:        CalleePrologue::instantiate
 * OVERVIEW:        Extend the instantiation of these types of logues
 *                  to give values to the variables in the incoming
 *                  parameters spec and the substitutions spec.
 * PARAMETERS:      values - the values to instantiate with
 * RETURNS:         an instantiated logue
 *============================================================================*/
void CalleePrologue::instantiate(vector<int>& values)
{
    // Use Logue's instantiate method to instantiate the parameters
    Logue::instantiate(values);

    // To instantiate the substitutions, we simply need to replace any
    // idParams in the RHS of a substitution.
    for (params_IT p_it = params.begin(); p_it != params.end(); p_it++) {


        // Create an expression that represents the current variable
        SemStr search;
        search.push(idParam);
        search.push(theSemTable.findItem(p_it->first));

        // Create an expression that represents the value of the
        // current variable
        SemStr replace;
        replace.push(idIntConst);
        replace.push(p_it->second);

        // Now go through each of the subs RHS's to find uses of
        // the current parameter and replace it
        for (substitutions_IT it = substitutions.begin();
            it != substitutions.end(); it++) {

            SemStr& RHS = it->second;
            RHS.searchReplaceAll(search,replace);
        }

        // Instantiate the localsSize expression
        if (localsSize.searchReplaceAll(search,replace))
            localsSize.simplify();

    }

    // Now instantiate the incoming parameters spec
    incomingParams.instantiate(params);

    // Now instantiate the parent stack spec
    parentStack.instantiate(params);
}

/*==============================================================================
 * FUNCTION:        CalleePrologue::put
 * OVERVIEW:        Output this logue to an ostream.
 * PARAMETERS:      os - output stream to use.
 * RETURNS:         the given output stream
 *============================================================================*/
ostream& CalleePrologue::put(ostream& os) const
{
    Logue::put(os);

    os << "AFP substitutions:" << endl;
    for (substitutions_CIT it = substitutions.begin();
        it != substitutions.end(); it++)
        os << "\t" << it->first << " -> " << it->second << endl;

    os << "locals size in byte:" << localsSize << endl;
    os << "incoming parameters:" << incomingParams << endl;
    os << "accessible parent stack: " << parentStack << endl;
    return os;
}

/**********************************
 * CalleeEpilogue methods
 **********************************/

/*==============================================================================
 * FUNCTION:        CalleeEpilogue::CalleeEpilogue
 * OVERVIEW:        Constructor.
 * PARAMETERS:      name - the name of the prologue
 *                  params - the parameters of the prologue
 * RETURNS:         <nothing>
 *============================================================================*/
CalleeEpilogue::CalleeEpilogue(const string& name,
    const list<string>& params) :
    Logue(name,CALLEE_EPILOGUE,params)
{}

/*==============================================================================
 * FUNCTION:        CalleeEpilogue::setRetSpec
 * OVERVIEW:        Set the info about where returned values are found.
 * PARAMETERS:      retSpec -
 * RETURNS:         <nothing>
 *============================================================================*/
void CalleeEpilogue::setRetSpec(const ReturnLocations& retSpec)
{
    retLocs = retSpec;
}

/*==============================================================================
 * FUNCTION:        CalleeEpilogue::getRetSpec
 * OVERVIEW:        Return a pointer to the return locations spec.
 * PARAMETERS:      <none>
 * RETURNS:         pointer to the return locations spec
 *============================================================================*/
ReturnLocations* CalleeEpilogue::getRetSpec()
{
    return &retLocs;
}

/*==============================================================================
 * FUNCTION:        CalleeEpilogue::put
 * OVERVIEW:        Output this logue to an ostream.
 * PARAMETERS:      os - output stream to use.
 * RETURNS:         the given output stream
 *============================================================================*/
ostream& CalleeEpilogue::put(ostream& os) const
{
    Logue::put(os);
    return os << "returns:" << endl << retLocs << endl;
}

/**********************************
 * LogueDict methods
 **********************************/

/*==============================================================================
 * FUNCTION:        LogueDict::newLogue
 * OVERVIEW:        Constructs and inserts a new logue into the
 *                  dictionary.
 * PARAMETERS:      name - the name of the logue to add
 *                  type - the type of the logue in string form. This
 *                    is used to decide exactly which type of Logue
 *                    (or subclass) to construct.
 *                  params - the parameters of the logue
 * RETURNS:         <nothing>
 *============================================================================*/
void LogueDict::newLogue(const string& name, const string& type,
    const list<string>& params)
{
    if (type == "CALLER_PROLOGUE")
        dict[name] = new Logue(name, Logue::CALLER_PROLOGUE, params);
    else if (type == "CALLEE_PROLOGUE")
        dict[name] = new CalleePrologue(name, params);
    else if (type == "CALLEE_EPILOGUE")
        dict[name] = new CalleeEpilogue(name, params);
    else if (type == "CALLER_EPILOGUE")
        dict[name] = new Logue(name, Logue::CALLER_EPILOGUE, params);
}

/*==============================================================================
 * FUNCTION:        LogueDict::exists()
 * OVERVIEW:        Return true if a logue exists with the given name
 * PARAMETERS:      name - the name of the logue to look for
 * RETURNS:         true if exists
 *============================================================================*/
bool LogueDict::exists(const string& name) const
{
    dict_CIT it = dict.find(name);
    return (it != dict.end());
}

/*==============================================================================
 * FUNCTION:        LogueDict::operator[]
 * OVERVIEW:        Return a reference to the template logue of the
 *                  given name.
 * PARAMETERS:      name - the name of the logue to return
 * RETURNS:         a reference to an Logue or NULL if no such Logue
 *                  exists with the given name.
 *============================================================================*/
Logue* LogueDict::operator[](const string& name)
{
    dict_IT it = dict.find(name);
    if (it == dict.end()) {
        ostrstream ost;
        ost << "`" << name << "' is undeclared";
        error(str(ost));
        return NULL;
    }
    else
        return it->second;
}

/*==============================================================================
 * FUNCTION:        LogueDict::instantiate
 * OVERVIEW:        Returns a Logue object instantiated from the gievn
 *                  parameters. As with RTLInstDict::InstantiateRTL, this method
 *                  assumes the order of the actual values given is the same as
 *                  the order in which the formals parameters were specified. In
 *                  addition, the number of parameters given is also assumed to
 *                  match.
 * PARAMETERS:      name - name of the logue
 *                  params - the actual values of the positional parameters
 *                    of the logue as declared in the SLED & CSR specs.
 * RETURNS:         a Logue instance or NULL if there isn't one
 *                  corresponding to the given name
 *============================================================================*/
Logue* LogueDict::instantiate(const string& name, vector<int>& params)
{
    dict_IT it = dict.find(name);
    if (it == dict.end()) {
        ostrstream os;
        os << "can't instantiate undefined logue `" << name << "'";
        error(str(os));
        return NULL;
    }

    // Having assured the given logue exists, make a copy of it and
    // instantiate this copy. Have to do the copy here so that a logue
    // of the right type is returned.
    Logue* logue;
    switch (it->second->getType()) {
        case Logue::CALLEE_PROLOGUE:
            logue = new CalleePrologue(*((CalleePrologue*)it->second));
            logue->instantiate(params);
            return logue;
        case Logue::CALLEE_EPILOGUE:
            logue = new CalleeEpilogue(*((CalleeEpilogue*)it->second));
            logue->instantiate(params);
            return logue;
        default:
            logue = new Logue(*(it->second));
            logue->instantiate(params);
            return logue;
    }
}

/*==============================================================================
 * FUNCTION:        operator<<
 * OVERVIEW:        Outputs operator for a LogueDict object.
 * PARAMETERS:      os - the stream to write to
 *                  dict - the LogueDict object to write
 * RETURNS:         a reference to os
 *============================================================================*/
ostream& operator<<(ostream& os, const LogueDict& dict)
{
    for (map<string,Logue*>::const_iterator it = dict.dict.begin();
        it != dict.dict.end(); it++) {

        os << *(it->second) << endl;
    }
    os << endl;
    return os;
}

/**********************************
 * LocationFilter methods
 **********************************/

/*==============================================================================
 * FUNCTION:        LocationFilter::~LocationFilter
 * OVERVIEW:        Destructor.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
LocationFilter::~LocationFilter()
{}

/*==============================================================================
 * FUNCTION:        LocationFilter::matches
 * OVERVIEW:        Returns true.
 * PARAMETERS:      loc - the location to test
 * RETURNS:         true
 *============================================================================*/
bool LocationFilter::matches(const SemStr& loc)
{
    return true;
}


/**********************************
 * CSRLocationFilter methods
 * This is a simple class that has a pointer to two other classes, one for
 * parameters and one for return locations, which do all the work
 * If either of these objects' match returns true, this class matches
 **********************************/

/*==============================================================================
 * FUNCTION:        CSRLocationFilter::CSRLocationFilter
 * OVERVIEW:        Constructor.
 * PARAMETERS:      params - a ptr to a Parameters object which has a matching
 *                    function for parameter locations
 *                  retLocs - a ptr to a ReturnLocations object which has a
 *                    matching function for return locations
 * RETURNS:         <nothing>
 *============================================================================*/
CSRLocationFilter::CSRLocationFilter(Parameters* params,
    ReturnLocations* retLocs) : LocationFilter(), 
    params(params), retLocs(retLocs)
{}
/*==============================================================================
 * FUNCTION:        CSRLocationFilter::matches
 * OVERVIEW:        Test the given location to see if it is a valid parameter
 *                  location or a return location as specified in a CSR spec.
 * PARAMETERS:      loc - the location to test
 * RETURNS:         the given location matches
 *============================================================================*/
bool CSRLocationFilter::matches(const SemStr& loc)
{
    return (retLocs != NULL && retLocs->matches(loc) ||
        (params != NULL && params->matches(loc)));
}

/**********************************
 * CSR methods
 **********************************/

/*==============================================================================
 * FUNCTION:        CSR::CSR
 * OVERVIEW:        Constructor.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
CSR::CSR()
{}

/*==============================================================================
 * FUNCTION:        CSR::~CSR
 * OVERVIEW:        Destructor.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
CSR::~CSR()
{}

/*==============================================================================
 * FUNCTION:        CSR::instantiateLogue
 * OVERVIEW:        Given an logue name and a list of ordered values, return an
 *                  instantiated Logue. Delegates the instantiation to the logue
 *                  dictionary contained in this CSR object.
 * PARAMETERS:      name - name of the logue
 *                  params - the actual values of the positional parameters
 *                    of the logue as declared in the SLED & CSR specs.
 * RETURNS:         a Logue instance or NULL if there isn't one
 *                  corresponding to the given name
 *============================================================================*/
Logue* CSR::instantiateLogue(const string name, vector<int>& params)
{
    return logueDict.instantiate(name,params);
}

/*==============================================================================
 * FUNCTION:        CSR::readCSRFile
 * OVERVIEW:        Initialise the CSR analysis by parsing in a CSR
 *                  specification.
 * PARAMETERS:      fileName - the file containing the CSR spec
 *                  debug - print out the CSR spec as it is parsed
 * RETURNS:         parse was successful
 *============================================================================*/
bool CSR::readCSRFile(const char* fileName, bool debug /*= false*/)
{
    // Attempt to open the CSR file
    CSRParser parser(prog.getProgPath()+fileName,*this,prog.RTLDict,debug);

    parser.yyparse();

    if (debug) {
        cout << "\n================ CSR analysis object============" << endl;
        cout << *this;
        cout << "\n================================================" << endl;
    }
    
    return true;
}

/*==============================================================================
 * FUNCTION:        CSR::subAXP
 * OVERVIEW:        Do the substitution of registers to equivalent
 *                  %afp/%agp relative locations as specified in the FRAME/
 *                  GLOBAL ABSTRACTION section of a PAL spec.
 * PARAMETERS:      proc - a procedure to be analysed
 * RETURNS:         <nothing>
 *============================================================================*/
void CSR::subAXP(UserProc* proc)
{
    map<SemStr,SemStr> subMap;

    // add initial AFP substitution 
    subMap[afpInit].push(idAFP);

    // Copy AFP substitutions to be made to a local map
    CalleePrologue* prologue = (CalleePrologue*)proc->getPrologue();
    if (prologue != NULL) 
        prologue->addToSubstitutions(subMap);
        
    // Also copy AGP substitutions to this local map
    for (map<SemStr,SemStr>::iterator it=agpMap.begin(); 
      it!=agpMap.end(); it++)
        subMap[it->first] = it->second;

#ifdef DEBUG_CSR
    cout << "\nDoing AXP substitutions with initial state:\n";
    for (map<SemStr,SemStr>::iterator it = subMap.begin();
        it != subMap.end(); it++)
        cout << "\t" << it->first << " -> " << it->second << endl;
#endif

    proc->subAXP(subMap);
}

/*==============================================================================
 * FUNCTION:        CSR::replaceParentStackReferences
 * OVERVIEW:        Replace any references to locations in the given
 *                  procedure's parents's stack frame with variables.
 * PARAMETERS:      proc - a procedure to be analysed
 * RETURNS:         <nothing>
 *============================================================================*/
void CSR::replaceParentStackReferences(UserProc* proc)
{
    // Get a reference to a parent stack specification
    ParentStack* cStack;
    if (proc->getPrologue() == NULL) {
        // Check if there is a "none" (special name) prologue entry
        string none("none");
        if (logueDict.exists(none)) {
            vector<int> noParams(0);
            CalleePrologue* noneLogue = (CalleePrologue*)
                instantiateLogue(none, noParams);
            cStack = noneLogue->getParentStackSpec();
            // Fall through and do the replacement below
        }
        else
            return;
    }
    else if ((cStack = proc->getPrologue()->getParentStackSpec()) == NULL)
        return;

    // Extract the range from the specification and convert each
    // location in the range to a symbolic (i.e. a variable)
    vector<SemStr> range;
    cStack->getRange(range);
    for (vector<SemStr>::iterator it = range.begin(); it != range.end(); it++)
        proc->toSymbolic(*it,*it);
}

/*==============================================================================
 * FUNCTION:        CSR::analyseCallee
 * OVERVIEW:        Analyse a given procedure as a callee to recover
 *                  its signature (apart from the return type). That
 *                  is, determine which locations are live into this
 *                  procedure (i.e. used before definition).
 *                  This finds the formal parameters.
 * PARAMETERS:      proc - a procedure to be analysed as a callee
 * RETURNS:         true if the number of formal parameters changed
 *============================================================================*/
bool CSR::analyseCallee(UserProc* proc)
{
    int oldNumArgs = proc->getNumArgs();

    // We want to restrict the locations considered during the
    // following DFA to those locations that could possibly be parameters or
    // return values. As such, we use a filter based upon the locations
    // specified as parameter and return locations in a CSR spec.

    CalleePrologue* prologue = proc->getPrologue();
    CalleeEpilogue* epilogue = proc->getEpilogue();

    if (epilogue == NULL) {

        ostrstream ost;
        ost << "procedure `" << proc->getName() << "' has no epilogue";
        warning(str(ost));
    }

    // Create a location filter out of the spec attached to the logues
    // of this procedure or else out of the caller context specs for
    // those logues that don't exist.
    Parameters* paramSpec;
    if (prologue == NULL) {
        // First check to see if there is a "none" callee prologue
        // For some architectures (e.g. Pentium), defaulting to the CALLER
        // spec is just wrong
        string none("none");
        if (logueDict.exists(string(none))) {
            Logue* log = logueDict[none];
            if (log->getType() == Logue::CALLEE_PROLOGUE)
                prologue = (CalleePrologue*)log;
        }
    }
    if (prologue != NULL)
        paramSpec = prologue->getParamSpec();
    else
        paramSpec = &outgoingParams;
    ReturnLocations* retSpec;
    if (epilogue != NULL)
        retSpec = epilogue->getRetSpec();
    else
        retSpec = &retLocations;
        
    // Main is a special case; we want to force two parameters (if envp is
    // used, we can't handle that as yet, especially with the Java backend).
    if (strcmp(proc->getName(), "main") == 0) {
        if (prologue) {
            Parameters* par = prologue->getParamSpec();
            list<SemStr>* lst = par->firstParams(2);
            proc->setParams(*lst);
            delete lst;
            return proc->getNumArgs() != oldNumArgs;
        }
    }

#ifdef DEBUG_CSR
    cout << "\nAnalysing `" << proc->getName();
    cout << "' as a callee using the following filter:"<<endl;
    cout << *paramSpec << endl << *retSpec << endl;
#endif

    CSRLocationFilter csrFilter(paramSpec,retSpec);
    LocationFilter* filter = &csrFilter;

    Cfg* cfg = proc->getCFG();
    assert(cfg != NULL);

    // Clear the DFA sets built when analysing this procedure as a caller.
    cfg->resetDFASets();

    // Build the set of locations that are used and defined in each BB
    cfg->buildUseDefSets(locMap,filter,proc);

    // Propogate this information across all BBs, keeping in mind that
    // calls define return locations.
    BITSET retLocs = retLocations.toBitset(locMap);
    cfg->buildLiveInSets(retLocs);

#ifdef DEBUG_CSR
    cfg->printDFAInfo(cout,locMap);
#endif

    // Find the locations that are live into this procedure.
    BITSET ins;
    cfg->getLiveIns(ins);

    // Use the call parameters spec to establish the parameters of this proc.
    list<SemStr> params;
    // The locations should not have overlapping parameter slot numbers,
    // hence the last argument is true
    bool aggUsed;               // True if the aggregate location is used
    aggUsed = paramSpec->filterLocations(ins,params,locMap,stackInverted, true);
    // Check for missing formal parameters
    paramSpec->missingParamCheck(params, proc, outgoingParams, aggUsed);
    proc->setParams(params, aggUsed);

#ifdef DEBUG_CSR
    cout << "\nCallee analysis summary for procedure `";
    cout << proc->getName() << "':" << endl;
    cout << "  parameters: ";
    ostream_iterator<SemStr> iter1(cout," ");
    copy(params.begin(),params.end(),iter1);
    cout << endl << endl;
#endif

    return proc->getNumArgs() != oldNumArgs;
}

/*==============================================================================
 * FUNCTION:        CSR::analyseCaller
 * OVERVIEW:        Analyse a given procedure as a caller. For each given call
 *                  in this procedure, determine the locations that are live at
 *                  the call (i.e. have been defined on all paths to the call)
 *                  and the locations that are used but not defined after the
 *                  call. Save this information into each call.
 *                  This finds the actual parameters, and return type
 * PARAMETERS:      proc - a procedure to be analysed
 * RETURNS:         true if return location changes at any call, or different
 *                  number of actual parameters at any call
 *============================================================================*/
bool CSR::analyseCaller(UserProc* proc)
{
    bool change = false;

    // We want to restrict the locations considered during the
    // following DFA to those locations that could possibly be parameters or
    // return values. As such, we use a filter based upon the locations
    // specified as parameter and return locations in a CSR spec.
    CSRLocationFilter csrFilter(&outgoingParams,&retLocations);
    LocationFilter* filter = &csrFilter;

    Cfg* cfg = proc->getCFG();
    assert(cfg != NULL);

    // Clear the DFA sets built when analysing this procedure as a callee.
    cfg->resetDFASets();

#ifdef DEBUG_CSR
    cout << "\nAnalysing `" << proc->getName();
    cout << "' as a caller using the following filter:"<<endl;
    cout << outgoingParams << endl << retLocations << endl;
#endif

    // Build the set of locations that are used and defined in each BB
    cfg->buildUseDefSets(locMap,filter,proc);

    // Propagate this information across all BBs
    // NOTE MVE: I was passing the parameters of this procedure to
    // buildLiveOutSets, so that parameters would be live. I can't seem
    // to recreate the example where it was needed (it was a register call
    // Sparc example, where if a certain predicate was false, it would return
    // directly, and this path caused a void return.
    // There is a nasty error message side effect with Pentium front ends, so
    // for now, this is commented out
    // NOTE MVE: I can't find this error now, but it causes 1 silly problem
    // with the sparcpatt test.
#if 0
    setSgiSemStr& paramSet = proc->getParamSet();
    BitSet paramBS = locMap.toBitset(paramSet);
#else
    BitSet paramBS;
#endif
    cfg->buildLiveOutSets(paramBS);
    cfg->buildRecursiveUseUndefSets();
    SET_CALLS& calls = cfg->getCalls();

    // Analyse each call site for return values. Any location used
    // after a call without first being defined is determined to be a
    // returned value. There *must* be at most one of these locations
    // for each call.
    for (SET_CALLS::iterator it2 = calls.begin(); it2 != calls.end(); it2++) {
        
        HLCall* call = *it2;
        ADDRESS dest = call->getFixedDest();
        Proc* callee = 0;
        if (dest != NO_ADDRESS) callee = prog.findProc(dest);
        // Get the BB of this call which contains the required liveness sets.
        const PBB callBB = call->getBB();

        BITSET usedUndef;
        const vector<PBB>& outEdges = callBB->getOutEdges();
        for (vector<PBB>::const_iterator it3 = outEdges.begin();
          it3 != outEdges.end(); it3++)
            usedUndef |= (*it3)->getRecursiveUseUndefSet();

        // Refine the return locations with the return locations spec
        list<SemStr> retLocs;
        retLocations.filterLocations(usedUndef,retLocs,locMap);
        if (retLocs.size() > 1) {
            ostrstream ost;
            ost << "code after call at 0x" << hex << call->getAddress() << dec;
            ost << " in `" << proc->getName() << "' to `";
            if (valid(callee))
                ost << callee->getName();
            else
                ost << "register call";
            ost << "' appears to need more than one return value:\n";
            list<SemStr>::const_iterator rl;
            for (rl = retLocs.begin(); rl != retLocs.end(); rl++) {
                rl->printt(ost);
                ost << " ";
            }
            ost << endl;
            ost << "usedUndef is ";
            locMap.printBitset(ost, usedUndef);
            warning(str(ost));
        }

//cout << "ACaller: callee " << callee->getName() << " has " << dec << retLocs.size() << " ret locs\n";   // HACK!
        if (retLocs.size() >= 1) {

            // Inform the callee of the return type it expects (if it expects
            // one at all). This will set the return type of the callee if
            // it doesn't already have one.
            // Note: if more than one, just pick the first one
            SemStr& returnLoc = *retLocs.begin();

            if (valid(callee)) {
                change |= callee->setReturnType(returnLoc, retLocations);
//cout << "ACaller: callee " << callee->getName() << " has ret loc " << returnLoc << "; change is " << change << endl;    // HACK!
                // Check if this use of a return location is "passed on" to
                // any callees. For example in the returncallee test, main
                // uses the return value from add4, and this use is "passed on"
                // to add2, since add4 doesn't define the return location
                // after the call to add2
                if (!callee->isLib()) {
                    ((UserProc*)callee)->checkReturnPass(locMap.toBit(
                        returnLoc), returnLoc, retLocations);
                }
            }

            // Convert the return location (if any) to a variable, if needed.
            // This will be rare, since return locations are usually registers
            SemStr dummy;
            proc->toSymbolic(returnLoc,dummy);
            call->setReturnLoc(returnLoc);
            callBB->setReturnLoc(locMap, returnLoc); // Set the bitmap in the BB
        }
    
#ifdef DEBUG_CSR
        cout << endl << "call at: "<< hex << call->getAddress() << dec << " `";
        if (valid(callee))
            cout << callee->getName();
        else
            cout << "register call";
        cout << "'\n  params: ";
        list<SemStr> params = call->getParams();
        unsigned n = 0;
        for (list<SemStr>::iterator it4 = params.begin();
          it4 != params.end(); it4++) {
            if (it4->getFirstIdx() == idVar) {
                cout << "<dummy> ";
                n++;
            }
            else
                cout << *it4 << " ";
        }
        cout << endl;
        if (n && (n == params.size()))
            cout << "  Warning: call has all dummy actual parameters\n";

        if (retLocs.size() == 1) {
            cout << "  returned value location: ";
            cout << call->getReturnLoc() << endl;
        }
#endif
    }


    // Analyse each call site. The live outs at each call are
    // determined to be the parameters of the call. These parameters
    // may be refined later when they are matched against the
    // recovered parameters declared for the corresponding callees.
    for (SET_CALLS::iterator it = calls.begin(); it != calls.end(); it++) {
        
        HLCall* call = *it;
        // If the name is forced, then the parameters must be forced also
        // (so we don't want to override those forced parameters here)
        if (call->getDestName())
            continue;
        ADDRESS dest = call->getFixedDest();
        Proc* callee = NULL;
        if (dest != NO_ADDRESS) {
            callee = prog.findProc(dest);
            if (callee == (Proc*)-1)
                // Deleted proc: just ignore it
                continue;
            assert(callee != NULL);
            // Save the callees while we have them (add to set of callees)
            proc->setCallee(callee);
        }

        // Get the BB of this call which contains the required liveness sets.
        const PBB callBB = call->getBB();

        // Initialise the parameters for this call by filtering the live-outs
        // from its enclosing basic block through the call parameters spec.
        list<SemStr> params;
        // There could be multiple potential actual parameters with the same
        // parameter slot number (e.g. pa-risc r26 and fr4 both live)
        outgoingParams.filterLocations(callBB->getLiveOuts(),params,locMap,
          stackInverted, false);

        // Now match the potential parameters against the signature of
        // the called procedure. Note that this may increase, decrease
        // or leave unchanged, the number of actual parameters (see
        // Proc::matchParams for an explanation as to why).
        if (callee)
            callee->matchParams(params, *proc, outgoingParams);

        // Only convert outgoing parameters to vars if a true stack based
        // machine, e.g. pentium. The way to tell is that offset= is
        // negative and no "STACK IS INVERTED", or offset= is positive
        // with an inverted stack
        if ((outgoingParams.getOffsetEquals() < 0) !=
          prog.csrSrc.isStackInverted()) {
            // Reserve vars for the outgoing parameters. Note that we don't
            // actually save the symbolic names (e.g. v9) anywhere as yet,
            // but we call toSymbolic to "reserve" symbolics in the symbolMap
            // for proc (the // caller).
            // We use the original locations (e.g. r[8], m[%afp-10]) to
            // pass to HLCall::setParams (needed in this form below where
            // uses of parameters are propagated into the DFA)
            // They get converted to symbolics in Proc::propagateSymbolics()
            for (list<SemStr>::iterator it1 = params.begin();
              it1 != params.end(); it1++) {
                SemStr dummy;
                // Make it a local only if it's not an incoming parameter
                proc->toSymbolic(*it1, dummy, !proc->isParam(*it1));
            }
        }

        // Save the parameters in the call
        change |= call->getParams().size() != params.size();
        call->setParams(params);
    }

    // Now have to redo the DFA to account for the new uses by a call
    // of its parameter locations, and definitions of return locations.
    cfg->resetDFASets();
    cfg->buildUseDefSets(locMap,filter,proc);
    cfg->buildLiveOutSets(paramBS);

#ifdef DEBUG_CSR
    cfg->printDFAInfo(cout,locMap);
#endif

    return change;
}

/*==============================================================================
 * FUNCTION:        CSR::postAnalyseCaller
 * OVERVIEW:        Analyse a given procedure as a caller. For each given call
 *                  in this procedure, check the parameters against the set of
 *                  locations that are used but not defined before the call.
 *                  These could be parameters added as a result of matchParams()
 *                  which aren't part of a callchain or paramchain (see tests
 *                  with these names); these often happen in Sparc programs with
 *                  certain idiomatic patterns like call/restore. Java back
 *                  ends will not allow these cases, and it doesn't make sense
 *                  to pass something that is not always initialised, so we
 *                  change these parameters to constant 0.
 * NOTE:            Assumes that live sets are set by a previous call to
 *                    analyseCaller()
 * PARAMETERS:      proc - a procedure to be analysed
 * RETURNS:         Nothing
 *============================================================================*/
void CSR::postAnalyseCaller(UserProc* proc)
{
    SemStr zero;
    zero << idIntConst << 0;

    Cfg* cfg = proc->getCFG();
    SET_CALLS& calls = cfg->getCalls();
    for (SET_CALLS::iterator it2 = calls.begin(); it2 != calls.end(); it2++) {
        
        HLCall* call = *it2;
        ADDRESS dest = call->getFixedDest();
        Proc* callee = 0;
        if (dest != NO_ADDRESS) callee = prog.findProc(dest);
        // Get the BB of this call
        const PBB callBB = call->getBB();

        BITSET liveOut = callBB->getLiveOuts();
        list<SemStr> actuals(call->getParams());
        bool changed = false;
        list<SemStr>::iterator it4;
        for (it4 = actuals.begin(); it4 != actuals.end(); it4++) {
            if (!locMap.test(*it4, liveOut)) {
                changed = true;
                ostrstream ost;
                ost << "In call to " << callee->getName() << " from proc "
                  << proc->getName() << ", replacing parameter "
                  << *it4 << " with zero";
                warning(str(ost));
                it4 = actuals.erase(it4);
                it4 = actuals.insert(it4, zero);
            }
        }
        if (changed)
            call->setParams(actuals);
    }
}

/*==============================================================================
 * FUNCTION:        CSR::analyse
 * OVERVIEW:        Recover parameters and return types for the given
 *                  procedure and all procedures underneath it in the
 *                  call graph that haven't already been analysed.
 * PARAMETERS:      proc - the procedure to be analysed
 *                  fe - reference to the FrontEndSrc object
 *                  spec - if true, this is a speculative analysis
 * RETURNS:         <nothing>
 *============================================================================*/
void CSR::analyse(UserProc* proc, FrontEndSrc& fe, bool spec)
{

    // Do the substitution of the abstract frame pointer (and abstract data
    // pointer if applicable) as specified by the first section in the
    // relevant CSR file
    subAXP(proc);

    // Repeat the analyse as callee and analyse as caller code, until there is
    // no change. This is needed for architectures like Sparc where some return
    // locations use the same registers as parameters do; thus, finding a return
    // value may require previous analysis to be redone. This can take several
    // iterations if the calls are chained
    bool change;
    do {
        // Recover the signature of this procedure (apart from the return type)
        change = analyseCallee(proc);

        // Don't recurse to callees if the "decode single procedure" flag is
        // used
        if (!progOptions.singleProc) {
            // Iterate through the calls in the given procedure and decode the
            // corresponding callees if they are user procedures.
            SET_CALLS& callSites = proc->getCFG()->getCalls();
            for (SET_CALLS::iterator it= callSites.begin();
              it!= callSites.end(); it++){
                HLCall* call = (HLCall*)*it;
                ADDRESS dest = call->getFixedDest();
                if(dest != NO_ADDRESS) {
                    // Recurse on the callee
                    decodeProc(dest, fe, true, spec);
                }
            }
        }

        // Recover actual parameters and return value for each call in
        // this procedure. Also recovers the return type for user
        // procedures that are called by at least one of these call sites.
        change |= analyseCaller(proc);
    } while (change);

    // This causes a few problems, and other priorities await. So for now,
    // the following call, which replaces non-live actual parameters with
    // constant 0 parameters, is commented out.
//  postAnalyseCaller(proc);

    // Special analysis for main/PilotMain: return type is always integer
    if ((strcmp(proc->getName(), "main") == 0) ||
        (strcmp(proc->getName(), "PilotMain") == 0)) {
        proc->setReturnType(*retLocations.locationFor(Type(INTEGER)),
            retLocations);
    }

    // Special treatment for procs that are being speculatively analysed.
    // We don't have callers for these, so we need to guess our own return type.
    // Note that the data flow information is still set up from the last call
    // to analyseCaller, with the filter set to parameter and return locations
    if (spec) {
        Type ty = guessReturnType(proc);
        if (ty.getType() != TVOID)
            setReturnType(proc, ty);
    }

    // Detect any references to locations with a parent's stack from a
    // callee and replace these with variables
    replaceParentStackReferences(proc);

    // Replace all locations with their symbolic representation for
    // those that now have one
    proc->propagateSymbolics();

}

/*==============================================================================
 * FUNCTION:        setReturnType
 * OVERVIEW:        Set the return type for the given proc
 * PARAMETERS:      proc - pointer to the UserProc to change the type of
 *                  type - a reference to the return type
 * RETURNS:         <nothing>
 *============================================================================*/
void CSR::setReturnType(UserProc* proc, const Type& type)
{
    SemStr* loc;
    // In general, the return location depends on the epilogue
    CalleeEpilogue* epilogue = proc->getEpilogue();
    if (epilogue) {
        loc = new SemStr(*epilogue->getRetSpec()->locationFor(type));
    } else {
        ostrstream ost;
        ost << "Can't set return type for " << proc->getName() <<
            " since it has no epilogue";
        warning(str(ost));
        // Use the CALLER return locations; often correct
        loc = new SemStr(*retLocations.locationFor(type));
    }
    // Make sure it's the size we want
    loc->getType().setSize(type.getSize());
    proc->setReturnType(*loc, retLocations);
    delete loc;
}

/*==============================================================================
 * FUNCTION:        guessReturnType
 * OVERVIEW:        Guess the return type for the given proc
 * NOTE:            This is only used for speculatively decoded procs, where
 *                  we don't have access to the callers
 * PARAMETERS:      proc - pointer to the proc to guess the type of
 * RETURNS:         Guessed type as a Type object
 *============================================================================*/
Type CSR::guessReturnType(UserProc* proc)
{
    // Consider the return BBs. Make a set of liveOuts, and liveOuts that are
    // not used
    BitSet liveOut, liveOutUnused;
    // Set both to the universal set
    liveOut.set(); liveOutUnused.set();
    list<PBB>::iterator it;
    Cfg* cfg = proc->getCFG();
    for (PBB pBB = cfg->getFirstBB(it); pBB; pBB = cfg->getNextBB(it)) {
        if (pBB->getType() == RET) {
            liveOut &= pBB->getLiveOuts();
            liveOutUnused &= pBB->getLiveOutUnuseds();
        }
    }
    // Filter the sets through the return locations filter
    list<SemStr> retLocs;
    retLocations.filterLocations(liveOutUnused, retLocs, locMap);

    // Are there any live out that are not used? If so, these are the best
    // return location candidates
    int n = retLocs.size();
    if (n > 1) {
        // We have more than one return type to choose!
        ostrstream ost;
        ost << dec << n << " return locations live without being used. "
            "Cannot guess return type for proc " << proc->getName() << endl;
        warning(str(ost));
    }
    if (n > 0) {
        return retLocs.front().getType();
    }

    // All live outs are used. But they may be valid return locations!
    retLocations.filterLocations(liveOut, retLocs, locMap);
    n = retLocs.size();
    if (n > 1) {
        // We have more than one return type to choose!
        ostrstream ost;
        ost << dec << n << " return locations live. "
            "Cannot guess return type for proc " << proc->getName() << endl;
        warning(str(ost));
    }
    if (n > 0) {
        return retLocs.front().getType();
    }
    return Type(TVOID);
}

/*==============================================================================
 * FUNCTION:        getIntSize
 * OVERVIEW:        Get the size of the OFFSET = parameter from the PAL spec
 *                    This is essentually sizeof(int) for the given architecture
 * PARAMETERS:      None
 * RETURNS:         The size in bytes, e.g. 4 for Sparc
 *============================================================================*/
int CSR::getIntSize()
{
    return outgoingParams.getIntSize();
}

/*==============================================================================
 * FUNCTION:        operator<<
 * OVERVIEW:        Output operator for a CSR object.
 * PARAMETERS:      os -
 *                  csr -
 * RETURNS:         <nothing>
 *============================================================================*/
ostream& operator<<(ostream& os, const CSR& csr)
{
    os << "logue dictionary:\n" << csr.logueDict << endl;
    os << "outgoing parameters:\n" << csr.outgoingParams << endl;
    os << "returns:\n" << csr.retLocations << endl;
    os << "AFP is initialised to: " << csr.afpInit << endl;
    substitutions_CIT it = csr.agpMap.begin();
    while (it != csr.agpMap.end()) {
        os << "AGP map entry: " << (*it).first << " to " << (*it).second <<
          endl;
        it++;
    }
    return os;
}

/*==============================================================================
 * FUNCTION:        CSR::existsNoneLogue
 * OVERVIEW:        Return true if the logue "none" exists
 * PARAMETERS:      <none>
 * RETURNS:         As above
 *============================================================================*/
bool CSR::existsNoneLogue()
{
    string none("none");
    return logueDict.exists(none);
}


#ifdef WIN32
}
#endif
