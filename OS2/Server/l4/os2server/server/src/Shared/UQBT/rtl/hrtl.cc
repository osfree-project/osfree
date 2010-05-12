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
 * FILE:       hrtl.cc
 * OVERVIEW:   Implementation of the classes that describe a UQBT HRTL, a
 *             high-level register transfer list. A HRTL is either:
 *                1) a higher-level RTL (instance of class HLJump, HLCall, etc.)
 *                   that represents information about a control transfer
 *                   instruction (CTI) in the source program, or
 *                2) a low-level RTL (instance of class RTL) that is the result
 *                   of decoding a non-CTI source machine instruction.
 *
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.14 $
 * 
 * Mod 20/2/98 MVE: Added GetOutAddr() and GetProcAddr()
 * 24 Feb 98 by David - added GetAddress, UpdateAddress method to RTlist
 * 25 Feb 98 - Cristina
 *  GetControlTransfer() returns true/false depending on the RTlist
 *      instruction, and the kind of the instruction for control transfer ones.
 *      It also checks for empty RTlist's.
 * 26 Feb 98 - Mike:     Changed several static casts to dynamic
 * 3 Mar 98 - Cristina:  Fixed RTlist::print()
 * 4 Mar 98 - Mike:      Fixed GetControlTransfer() for calls and rets 
 * 13 Mar 98 - Mike:     GetNumRT() now const
 * 11 Mar 98 - Cristina
 *  replaced BOOL type for bool (defined in C++), similar for TRUE and FALSE.
 * 3 Apr 98 - Mike: changes to cope with SpecialRegs inside Registers now
 * 26 Jun 98 - Mike: changes for ostream printing
 * 11 Aug 98 - Mike: split GetControlTransfer() etc from this file
 * 20 Aug 98 - Mike: split GetOutAddr() as well
 * 03 Dec 98 - Mike: deepCopy() updated for RTCall class
 * 10 Dec 98 - Mike: Added writeRtl()
 * 06 Jan 99 - Mike: Added deepCopy() /to RTlist copy constructor
 * 07 Jan 99 - Mike: Fixed deepCopy() to cope with classes like OpTableExpr
                derived from SemStr
 * 20 Jan 99 - Mike: Added kind member variable to RTlist; various classes
                derived from RTlist such as HLJump
 * 29 Jan 99 - Mike: Added HL version of GetProcAddr()
 * 04 Feb 99 - Mike: HLCall etc have print() now
 * 08 Feb 99 - Doug: Commented out unused (I think) constructors for HLCall and
                     modified remaining one to set new field (size of return
                     type)
                     Added HLRet class.
 * 11 Feb 99 - Mike: Added HLComp, changed HLJcond to hold iKind and bFloat
 * 12 Feb 99 - Mike: Added writeRtl() for high level RTlist classes
 * 22 Feb 99 - Mike: deepCopyList() copies to a reference parameter now; fixed
 *              deepCopyList() for RTCond's
 * 11 Mar 99 - Mike: Fixed RTlist::print()'s indent; added RTlist::DeleteRT()
 * 19 Mar 99 - Mike: fixed RTlist::expand() for copying "actuals" of RTFlagCalls
 * 30 Mar 99 - Doug: Constructors for HL RTlist classes moved into ctiXXX.cc
 *                   files as they now include analysis that replaces the
 *                   GetControlTransfer functionality
 * 31 Mar 99 - Mike: delete pCond in ~HLJcond()
 * 01 Apr 99 - Mike: Added constructor to HLJump for jump to fixed address;
 *                    removed printing of some low level RTLs
 * 07 Apr 99 - Doug: Added SetCondType()
 * 07 Apr 99 - Mike: Tidied up printing of HL RTLs; re-added MakeSigned();
 *                    updated comments; HEADERS
 * 15 Apr 99 - Mike: initialise preceedsRestore to false in HLCall constructor
 * 22 Apr 99 - Mike: don't output RTLs for high level jumps
 * 27 Apr 99 - Doug: Added AddDefinedLocations
 * 27 Apr 99 - Doug: Added AddUsedLocations
 * 30 Apr 99 - Mike: Initialise pDest in HLJump::HLJump
 * 19 May 99 - Mike: SetIsComputed() (etc) moved from HLCall to HLJump
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 18 Nov 99 - Mike: Added RTlist::appendRTlist(RTlist& rtl); use indent param
 *              in RTlist::print() and derived classes
 * 19 Nov 99 - Mike: Added [set is]ReturnAfterCall() to replace the mess that
 *              was isFollowedByRestore() and friends
 * 23 Nov 99 - Mike: HLCall::getUseDefLocations() calls its RTlist counterpart
 *              first, since HLCalls can have semantics now.
 * 15 Mar 00 - Cristina: RTList::setAFP transformed into setAXP. 
 * 30 Mar 00 - Mike: All searchAndReplace type functions take an optional
 *                      typeSens bool now
 * 04 May 00 - Mike: Fixed HLReturn::clone() (was trying to clone pDest)
 * 29 May 00 - Mike: getFixedDest works OK on register jumps (pDest == 0)
 * 14 Jul 00 - Mike: Added functions for post call semantics in HLCall, and to
 *                      force the name of the destination. Also, substitute the
 *                      post call semantics. Also print all these
 * 20 Jul 00 - Mike: When looking for uses, check m[exp] on the LHS as well
 *              (added searchExprForUses to do this)
 * 25 Aug 00 - Mike: In HLCall::print(), cope with findProc() returning -1
 * 07 Sep 00 - Mike: getUseDefLocations takes a Proc* now, so we can register
 *              the use of the return location
 * 07 Sep 00 - Mike: Restored code to add the return location to the def set
 * 17 Sep 00 - Mike: Added non const version of HLCall::getParams()
 * 20 Sep 00 - Mike: Removed a "<< endl" that was making -r ugly
 * 29 Nov 00 - Mike: Removed the "pc := dest" semantics of a JUMP_HRTL
 * 11 Feb 01 - Nathan: Initial version, separated from BinaryFile.cc to deal
 *              with some dependency problems.
 * 05 Mar 01 - Simon: Removed expand, eOPTABLE, etc
 * 06 Mar 01 - Mike: Modified HLJcond::print() to use operator<<, so that the
 *              type of the condition can be seen
 * 06 Mar 01 - Mike: Added RTlist::getSize(); converted insertAssign() to a
 *              member function of class RTlist
 * 19 Mar 01 - Mike: Added RTlist::insertAfterTemps() to solve the problem
 *              of inserting assignments that depend on temporaries
 * 20 Mar 01 - Mike: Finally made all RTlist and RT copies "deep"
 * 21 Mar 01 - Mike: Added a test to RTlist::print() to limit the indenting
 * 31 Mar 01 - Mike: getFixedDest() returns NO_ADDRESS if not a fixed address
 *              (286 programs typically put main at the start of the image,
 *              which before relocation is 0:0)
 * 11 Apr 01 - Mike: Added HLJump::getUseDefLocations; fixes 130.li's cequal()
 *              where the only use of a parameter is in a register jump
 * 31 Jul 01 - Brian: Renamed file from hrtlist.cc to hrtl.cc. Moved the code
 *              from rtlist.cc to here (rtlist.cc has been deleted), so this
 *              file now implements class HRTL and all its derived classes.
 *              Renamed class RTlist to HRTL. Added new derived class RTL.
 *              Removed unused operator== (equality test) from HRTL.
 * 06 Aug 01 - Mike: HLJcond::print displays semantics (if any)
 * 08 Aug 01 - Mike: Added forwardSubs()
 * 08 Aug 01 - Mike: Fixed a GPF associated with temps.erase(mm)
 * 11 Aug 01 - Mike: Don't forward substitute a guarded assignment to a temp
 * 13 Aug 01 - Bernard: Added support for type analysis
 * 14 Aug 01 - Mike: gcc v3 doesn't support semstr != semstr any more
 * 21 Aug 01 - Bernard: Moved all storeUseDefineStruct to HRTLUDstruct.cc
 * 30 Aug 01 - Mike: HLCall's parameters changed from vector to list
 * 26 Oct 01 - Mike: searchExprForUses is top down now, to get the correct type;
 *              also locMap is back to just sign-insensitive
 */

#include "global.h"
#include "hrtl.h"
#include "rtl.h"
#include "proc.h"
#include "prog.h"
#include "bitset.h"             // Some functions return class BitSet

/******************************************************************************
 * HRTL methods.
 * Class HRTL represents high-level register transfer lists. 
 *****************************************************************************/

/*==============================================================================
 * FUNCTION:        HRTL::HRTL
 * OVERVIEW:        Constructor.
 * PARAMETERS:      <none>
 * RETURNS:         N/a
 *============================================================================*/
HRTL::HRTL()
    : kind(LOW_LEVEL_HRTL), nativeAddr(0), numNativeBytes(0), isCommented(false)
{}

/*==============================================================================
 * FUNCTION:        HRTL::HRTL
 * OVERVIEW:        Constructor.
 * PARAMETERS:      instNativeAddr - the native address of the instruction
 *                  listRT - existing list of RTs
 * RETURNS:         N/a
 *============================================================================*/
HRTL::HRTL(ADDRESS instNativeAddr, list<RT*>* listRT /*= NULL*/)
    : kind(LOW_LEVEL_HRTL), nativeAddr(instNativeAddr), numNativeBytes(0),
      isCommented(false)
{
    if (listRT != NULL) {
        rtlist = *listRT;
    }
}

/*==============================================================================
 * FUNCTION:        HRTL::HRTL
 * OVERVIEW:        Copy constructor. A deep clone is made of the given object
 *                  so that the lists of RTs do not share memory.
 * PARAMETERS:      other: HRTL to copy from
 * RETURNS:         N/a
 *============================================================================*/
HRTL::HRTL(const HRTL& other)
    : kind(other.kind), nativeAddr(other.nativeAddr),
      numNativeBytes(other.numNativeBytes), isCommented(other.isCommented)
{
    other.deepCopyList(rtlist);
}

/*==============================================================================
 * FUNCTION:        HRTL::~HRTL
 * OVERVIEW:        Destructor.
 * PARAMETERS:      <none>
 * RETURNS:         N/a
 *============================================================================*/
HRTL::~HRTL() {
    list<RT*>::iterator it;
    for (it = rtlist.begin(); it != rtlist.end(); it++) {
        if (*it != NULL) {
            delete *it;
        }
    }
}

/*==============================================================================
 * FUNCTION:        HRTL::operator=
 * OVERVIEW:        Assignment copy.
 * NOTE:            This used to be a "shallow copy"; now it's a "deep copy"
 * PARAMETERS:      other - HRTL to copy
 * RETURNS:         a reference to this object
 *============================================================================*/
const HRTL& HRTL::operator=(const HRTL& other) {
    if (this != &other) {
        // Do a deep copy always
        rtlist.empty();                     // Delete any existing RT list
        other.deepCopyList(rtlist);         // Duplicate the list
        
        kind = other.kind;
        nativeAddr = other.nativeAddr;
        numNativeBytes = other.numNativeBytes;
        isCommented = other.isCommented;
    }
    return *this;
}

/*==============================================================================
 * FUNCTION:        HRTL::deepCopyList
 * OVERVIEW:        Given a list of RTs, appends a copy of this HRTL's list of
 *                  RTs. This is required as opposed
 *                  to returning a pointer to a clone of the list as it is used
 *                  in the copy constructor of HRTL to build the rtlist member
 *                  which has its memory automatically allocated when the object
 *                  is constructed.
 * PARAMETERS:      dest - the list in which to build the copy
 * RETURNS:         <nothing>
 *============================================================================*/
void HRTL::deepCopyList(list<RT*>& dest) const {
    for (RT_CIT p = rtlist.begin();  p != rtlist.end();  p++) {
        // Copy the RT; note that *p is a pointer to an RT, and that
        // RT is an abstract type
        RT* pNew = (*p)->clone();
        dest.push_back(pNew);
    }
}

/*==============================================================================
 * FUNCTION:        HRTL::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new HRTL that is a clone of this one
 *============================================================================*/
HRTL* HRTL::clone() const {
    list<RT*> rlist;
    deepCopyList(rlist);
    
    HRTL* ret = new HRTL(nativeAddr, &rlist);
    ret->kind = kind;
    ret->numNativeBytes = numNativeBytes;
    ret->isCommented = isCommented;
    return ret;
}

/*==============================================================================
 * FUNCTION:        HRTL::appendRT
 * OVERVIEW:        Append the given RT at the end of this RTL
 * NOTE:            Leaves any flag call at the end (so may push rt to second
 *                  last position, instead of last)
 * NOTE:            A copy of *rt is inserted
 * PARAMETERS:      rt: pointer to RT to append
 * RETURNS:         Nothing
 *============================================================================*/
void HRTL::appendRT(RT* rt) {
    if (rtlist.size() && (rtlist.back()->getKind() == RTFLAGCALL)) {
        RT_IT it = rtlist.end();
        rtlist.insert(--it, rt->clone());
    } else {
        rtlist.push_back(rt->clone());
    }
}

/*==============================================================================
 * FUNCTION:        HRTL::prependRT
 * OVERVIEW:        Prepend the given RT at the start of this RTL
 * NOTE:            A copy of rt is prepended
 * PARAMETERS:      rt: pointer to RT to prepend
 * RETURNS:         Nothing
 *============================================================================*/
void HRTL::prependRT(RT* rt) {
    rtlist.push_front(rt->clone());
}

/*==============================================================================
 * FUNCTION:        HRTL::appendRTlist
 * OVERVIEW:        Append a given list of RT*s to this RTL
 * NOTE:            A copy of the RTs in lrt are appended
 * PARAMETERS:      rtl: list of RT*s to insert
 * RETURNS:         Nothing
 *============================================================================*/
void HRTL::appendRTlist(const list<RT*>& lrt) {
    list<RT*>::const_iterator it;
    for (it = lrt.begin();  it != lrt.end();  it++) {
        rtlist.insert(rtlist.end(), (*it)->clone());
    }
}

/*==============================================================================
 * FUNCTION:        HRTL::appendRTlist
 * OVERVIEW:        Append the RTs of another RTlist to this RTL
 * NOTE:            A copy of the RTs in rtl are appended
 * PARAMETERS:      rtl: HRTL whose RTs we are to insert
 * RETURNS:         Nothing
 *============================================================================*/
void HRTL::appendRTlist(const HRTL& rtl) {
    appendRTlist(rtl.rtlist);
}

/*==============================================================================
 * FUNCTION:        HRTL::insertRT
 * OVERVIEW:        Insert the given RT before index i
 * NOTE:            A copy of *rt is inserted
 * PARAMETERS:      rt: pointer to the RT to insert
 *                  i: position to insert before (0 = first)
 * RETURNS:         Nothing
 *============================================================================*/
// insert register transfer at position i (or the head of the list)
void HRTL::insertRT(RT *rt, unsigned i) {
    // check that position i is not out of bounds
    assert (i < rtlist.size() || rtlist.size() == 0);

    // find the position
    RT_IT p = rtlist.begin();
    for (; i > 0; i--, p++);

    // do the insertion
    rtlist.insert(p,rt);
}

/*==============================================================================
 * FUNCTION:        HRTL::updateRT
 * OVERVIEW:        Replace the ith RT with the given RT
 * PARAMETERS:      rt: pointer to the new RT
 *                  i: index of RT position (0 = first)
 * RETURNS:         Nothing
 *============================================================================*/
void HRTL::updateRT(RT *rt, unsigned i) {
    // check that position i is not out of bounds
    assert (i < rtlist.size());

    // find the position
    RT_IT p = rtlist.begin();
    for (; i > 0; i--, p++);    

    // do the update
    *p = rt;
}

void HRTL::deleteRT(unsigned i) {
    // check that position i is not out of bounds
    assert (i < rtlist.size());

    // find the position
    RT_IT p = rtlist.begin();
    for (; i > 0; i--, p++);    

    // do the delete
    rtlist.erase(p);
}
    
/*==============================================================================
 * FUNCTION:        HRTL::getNumRT
 * OVERVIEW:        Get the number of RTs in this HRTL
 * PARAMETERS:      None
 * RETURNS:         Integer number of RTs
 *============================================================================*/
int HRTL::getNumRT() const {
    return rtlist.size();
}

/*==============================================================================
 * FUNCTION:        HRTL::at
 * OVERVIEW:        Provides indexing on a list. Changed from operator[] so that
 *                  we keep in mind it is linear in its execution time.
 * PARAMETERS:      i - the index of the element we want
 * RETURNS:         the element at the given index or NULL if the index is out
 *                  of bounds
 *============================================================================*/
RT* HRTL::elementAt(unsigned i) const {
    RT_CIT it;
    for (it = rtlist.begin();  i > 0 && it != rtlist.end();  i--, it++);
    if (it == rtlist.end()) {
        return NULL;
    }
    return *it;
}

/*==============================================================================
 * FUNCTION:        HRTL::print
 * OVERVIEW:        Prints this object to a stream in text form.
 * PARAMETERS:      os - stream to output to (often cout or cerr)
 *                  indent - Number of characters to move across
 * RETURNS:         <nothing>
 *============================================================================*/
void HRTL::print(ostream& os /*= cout*/, int indent /*= 0*/) const {
    if (indent > 80) indent = 80;       // Helps with debugging; sometimes
                                        // you get millions

    // print out the instruction address of this RTL
    os << hex << setfill('0') << setw(8) << nativeAddr;
    os << dec << setfill(' ');      // Ugh - why is this needed?

    // Print the register transfers
    // First line has 8 extra chars as above
    bool bFirst = true;
    if (indent == 0) indent = 1;        // Will print min of 1 char anyway
    for (RT_CIT p = rtlist.begin(); p != rtlist.end(); p++)
    {
        if (bFirst) os << setw(indent) << " ";
        else        os << setw(indent+8) << " ";
        (*p)->print(os);
        os << "\n";
        bFirst = false;
    }
    if (rtlist.empty()) os << endl;     // New line for NOP
}


/*==============================================================================
 * FUNCTION:        HRTL::getAddress
 * OVERVIEW:        Return the native address of this RTL
 * PARAMETERS:      None
 * RETURNS:         Native address
 *============================================================================*/
ADDRESS HRTL::getAddress() const {
    return nativeAddr;
}


/*==============================================================================
 * FUNCTION:        HRTL::updateAddress
 * OVERVIEW:        Set the nativeAddr field
 * PARAMETERS:      Native address
 * RETURNS:         Nothing
 *============================================================================*/
void HRTL::updateAddress(ADDRESS addr) {
    nativeAddr = addr;
}

/*==============================================================================
 * FUNCTION:        HRTL::setCommented
 * OVERVIEW:        Set the isCommented flag (so this RTL will be emitted as a
 *                    comment)
 * PARAMETERS:      state: whether to set or reset the flag
 * RETURNS:         Nothing
 *============================================================================*/
void HRTL::setCommented(bool state) {
    isCommented = state;
}

/*==============================================================================
 * FUNCTION:        HRTL::getCommented
 * OVERVIEW:        Get the isCommented flag
 * PARAMETERS:      state: whether to set or reset the flag
 * RETURNS:         Nothing
 *============================================================================*/
bool HRTL::getCommented() const {
    return isCommented;
}

/*==============================================================================
 * FUNCTION:        HRTL::subAXP
 * OVERVIEW:        (See comment for Proc::subAXP)
 * PARAMETERS:      subMap -
 * RETURNS:         Nothing
 *============================================================================*/
void HRTL::subAXP(map<SemStr,SemStr>& subMap) {
    list<RT*>::iterator it = rtlist.begin();
    while (it != rtlist.end()) {
// if (*it) {cerr << "\nAXP substitutution on `";
// (*it)->print(cerr);cerr<<"'\n";}
        if ((*it)->subAXP(subMap)) {
            // This RT was a definition of a substituted register and
            // so we must delete it here
            delete *it;
            it = rtlist.erase(it);
        } else {
            it++;
        }
    }
}

/*==============================================================================
 * FUNCTION:        HRTL::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 *                  typeSens - if true, the search is sensitive to type
 * NOTES:           It is difficult to make search a const parameter, though
 *                    it should be
 * RETURNS:         <nothing>
 *============================================================================*/
void HRTL::searchAndReplace(SemStr& search, const SemStr& replace,
                            bool typeSens)
{
    for (list<RT*>::iterator it = rtlist.begin(); it != rtlist.end(); it++) {
        (*it)->searchAndReplace(search, replace, typeSens);
    }
}

/*==============================================================================
 * FUNCTION:        RTList::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 *                  typeSens - if true, consider type when matching
 * NOTES:           search can't easily be made const
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool HRTL::searchAll(SemStr& search, list<SemStr *> &result,
                     bool typeSens)
{
    bool found = false;
    for (list<RT*>::iterator it = rtlist.begin(); it != rtlist.end(); it++) {
        if ((*it)->searchAll(search, result, typeSens)) {
            found = true;
        }
    }
    return true;
}

/*==============================================================================
 * FUNCTION:        HRTL::clear
 * OVERVIEW:        Clear the list of RTs
 * PARAMETERS:      None
 * RETURNS:         Nothing
 *============================================================================*/
void HRTL::clear() {
    rtlist.clear();
}

/*==============================================================================
 * FUNCTION:        searchExprForUses
 * OVERVIEW:        Search the expression for uses according to a given filter
 * PARAMETERS:      exp: expression to search
 *                  locMap - a map between locations and integer bit numbers
 *                  filter - a filter to restrict which locations are
 *                    considered
 *                  useSet - has added to it those locations used this BB
 *                  defSet - has added to it those locations defined this BB
 *                  useUndefSet - has added those locations used before defined
 * RETURNS:         Nothing, but reference sets added to
 *============================================================================*/
void searchExprForUses(SemStr* exp, LocationMap& locMap, LocationFilter* filter,
                       BITSET& defSet, BITSET& useSet, BITSET& useUndefSet)
{

    int idx = exp->getFirstIdx();
    int numVar = theSemTable[idx].iNumVarArgs;
    // Only interested in r[] or m[]
    if ((idx == idRegOf) || (idx == idMemOf)) {
        // This is the "bottom of the tree"; filter the whole expression
        if (filter->matches(*exp)) {
            int bit = locMap.toBit(*exp);

            // Record the use
            useSet.set(bit);

            // Add this to the use-before-definition set if necessary
            if (!defSet.test(bit)) {
                useUndefSet.set(bit);
            }
        }
    }

    // We have to recurse even into memofs, because they may contain some
    // register of expressions are used
    for (int i=0; i < numVar; i++) {
        // Recurse into the ith subexpression
        SemStr* sub = exp->getSubExpr(i);
        searchExprForUses(sub, locMap, filter, defSet, useSet, useUndefSet);
        delete sub;
    }
}

/*==============================================================================
 * FUNCTION:        HRTL::getUseDefLocations
 * OVERVIEW:        Get the set of the locations a) used, b) defined and
 *                  c) used before definition by this RTL.
 * PARAMETERS:      locMap - a map between locations and integer bit numbers
 *                  filter - a filter to restrict which locations are
 *                    considered
 *                  useSet - has added to it those locations used this BB
 *                  defSet - has added to it those locations defined this BB
 *                  useUndefSet - has added those locations used before defined
 *                  proc - pointer to the Proc object containing this HRTL
 * RETURNS:         <nothing>
 *============================================================================*/
void HRTL::getUseDefLocations(LocationMap& locMap, LocationFilter* filter,
                              BITSET& defSet, BITSET& useSet,
                              BITSET& useUndefSet, Proc* proc) const
{
    // Process each assignment RT in this RTL
    for (list<RT*>::const_iterator it = rtlist.begin();
         it != rtlist.end(); it++) {
        if ((*it)->getKind() == RTASSGN) {
            RTAssgn* assignRT = static_cast<RTAssgn*>(*it);
            // Search RHS for usage of any locations in filter and add
            // them to locs
            SemStr* RHS = assignRT->getRHS();
            searchExprForUses(RHS, locMap, filter, defSet, useSet, useUndefSet);
            // If the LHS is m[exp], filter exp and add any found locations
            // to the useage sets
            SemStr* LHS = assignRT->getLHS();
            if (LHS->getFirstIdx() == idMemOf) {
                SemStr* exp = LHS->getSubExpr(0);
                searchExprForUses(exp, locMap, filter, defSet, useSet,
                    useUndefSet);
            }
            
            // Add the LHS to the definition set if is it matched
            if (filter->matches(*LHS)) {
                defSet.set(locMap.toBit(*LHS));
            }
        }
    }
}

/*==============================================================================
 * FUNCTION:        HRTL::insertAssign
 * OVERVIEW:        Prepends or appends an assignment to the front or back of
 *                    this RTL
 * ASSUMES:         Assumes that ssLhs and ssRhs are "new" SemStr's that are
 *                  not part of other RTs. (Otherwise, there will be problems
 *                  when deleting this RT)
 *                  If size == -1, assumes there is already at least one assign-
 *                    ment in this HRTL
 * PARAMETERS:      ssLhs: ptr to SemStr to place on LHS
 *                  ssRhs: ptr to SemStr to place on the RHS
 *                  prep: true if prepend (else append)
 *                  size: size of the transfer, or -1 to be the same as the
 *                    first assign this RTL
 * RETURNS:         <nothing>
 *============================================================================*/
void HRTL::insertAssign(SemStr* ssLhs, SemStr* ssRhs, bool prep,
                        int size /*= -1*/) {
    RTAssgn* pRT;
    if (size == -1)
        size = getSize();

    // First create the left hand side
    // Generate the assign RT
    pRT = new RTAssgn(ssLhs, ssRhs, size);
    if (prep)
        prependRT(pRT);
    else
        appendRT(pRT);
    // Mark this as an added RT
    pRT->added = true;
}

/*==============================================================================
 * FUNCTION:        HRTL::insertAfterTemps
 * OVERVIEW:        Inserts an assignment at or near the top of this RTL, after
 *                    any assignments to temporaries. If the last assignment
 *                    is to a temp, the insertion is done before that last
 *                    assignment
 * ASSUMES:         Assumes that ssLhs and ssRhs are "new" SemStr's that are
 *                  not part of other RTs. (Otherwise, there will be problems
 *                  when deleting this RT)
 *                  If size == -1, assumes there is already at least one assign-
 *                    ment in this HRTL
 * NOTE:            Hopefully this is only a temporary measure
 * PARAMETERS:      ssLhs: ptr to SemStr to place on LHS
 *                  ssRhs: ptr to SemStr to place on the RHS
 *                  size: size of the transfer, or -1 to be the same as the
 *                    first assign this RTL
 * RETURNS:         <nothing>
 *============================================================================*/
void HRTL::insertAfterTemps(SemStr* ssLhs, SemStr* ssRhs, int size /* = -1 */) {
    RT_IT it;
    // First skip all RTAssgns with temps on LHS
    for (it = rtlist.begin(); it != rtlist.end(); it++) {
        RTAssgn* pRT = (RTAssgn*)*it;
        if (pRT->getKind() != RTASSGN)
            break;
        SemStr* LHS = pRT->getLHS();
        if (LHS->getSecondIdx() != idTemp)
            break;
    }

    // Now check if the next RT is an RTAssgn
    if ((it == rtlist.end()) || (((RTAssgn*)*it)->getKind() != RTASSGN)) {
        // There isn't an RTAssgn following. Use the previous RT to insert
        // before
        if (it != rtlist.begin())
            it--;
    }

    if (size == -1)
        size = getSize();

    // Generate the assign RT
    RTAssgn* pRT = new RTAssgn(ssLhs, ssRhs, size);

    // Insert before "it"
    rtlist.insert(it, pRT);
}

/*==============================================================================
 * FUNCTION:        HRTL::getSize
 * OVERVIEW:        Get the "size" for this HRTL. Just gets the size in bits of
 *                    the first RTAssgn
 * NOTE:            The size of the first assign may not be the size that you
 *                    want!
 * PARAMETERS:      None
 * RETURNS:         The size
 *============================================================================*/
int HRTL::getSize() const {
    RT_CIT it;
    for (it = rtlist.begin(); it != rtlist.end(); it++) {
        RTAssgn* pRT = (RTAssgn*)*it;
        if (pRT->getKind() == RTASSGN)
            return pRT->getSize();
    }
    return 32;              // Default to 32 bits if no assignments
}

/*==============================================================================
 * FUNCTION:        HRTL::forwardSubs
 * OVERVIEW:        Perform forward substitutions of temporaries (but not
 *                    tempNul) if possible. Useful where conditional assignments
 *                    are heavily used, and the simplification from forward
 *                    substitution is needed for analysis to work (e.g. pa-risc)
 * ASSUMPTION:      It is assumed that temporaries (other than tmpNul, which is
 *                    a sort of global) are only used within the current RTL
 * PARAMETERS:      None
 * RETURNS:         Nothing
 *============================================================================*/
void HRTL::forwardSubs()
{
    map<SemStr, SemStr> temps;  // A map from left hand sides to right hand
                                // sides, suitable for substition
    map<SemStr, SemStr>::iterator mm;
    SemStr replace;             // Dummy that gets the replacement
    list<SemStr*> result;       // Another dummy for results
    SemStr srch;                // Needed to work around silly const issues

    // Find the temporaries on the LHS, and make substitutions on
    // the right where they appear
    // Be careful with changes to the temps, or to components thereof
    list<RT*>::iterator it;
    for (it = rtlist.begin(); it != rtlist.end(); it++) {
        if ((*it)->getKind() != RTASSGN) continue;
        RTAssgn* pRT = (RTAssgn*) *it;
        SemStr* lhs = pRT->getLHS();
        SemStr* rhs = pRT->getRHS();
        // Substitute the RHS, and LHS if in m[] etc
        for (mm = temps.begin(); mm != temps.end(); mm++) {
            if (mm->second.len() == 0)
                // This temp assignment has been disabled by a clear() (below)
                continue;
            srch = mm->first;
            rhs->searchReplaceAll(srch, mm->second);
            if (!(*lhs == srch))
                lhs->searchReplaceAll(srch, mm->second);
        }
        if (
          (pRT->getGuard() == 0) &&     // Must not be guarded!
          (lhs->getFirstIdx() == idRegOf) &&
          (lhs->getSecondIdx() == idTemp) &&
          (lhs->getThirdIdx() != idTmpNul)) {
            // We have a temp. Add it to the map. (If it already exists,
            // then the mapping is updated rather than added)
            // The map has to be of SemStr, not SemStr*, for this to work.
            temps[*lhs] = *rhs;
        } else {
            // This is not assigning to a temp. Must check whether any temps
            // are now invalidated, for the purpose of substiution, by this
            // assignment
            for (mm = temps.begin(); mm != temps.end(); mm++) {
                if (mm->second.search(*lhs, replace)) {
                    // This temp is no longer usable for forward substitutions
                    // Rather than deleting it, we "set a flag" by clearing
                    // the map value (mm.second). This makes it ineligible for
                    // substitutions, but still available for deleting the
                    // assignment of. If we don't do this, then with
                    // tmp1 = r19 + r20;
                    // r19 = r19 + tmp1;
                    // we get the substitution, but the assignment to tmp1
                    // remains
                    mm->second.clear();
                }
            }
        }
    }

    // Now see if the assignments to the temps can go. Delete any entries from
    // the map for those that can't go
    for (mm = temps.begin(); mm != temps.end(); mm++) {
        for (it = rtlist.begin(); it != rtlist.end(); it++) {
            if ((*it)->getKind() == RTASSGN) {
                RTAssgn* pRT = (RTAssgn*)(*it);
                SemStr* rhs = pRT->getRHS();
                if (rhs->search(mm->first, replace)) {
                    // Note: map::erase seems to return a void! So can't do the
                    // usual and safe mm = temps.erase(mm)
                    temps.erase(mm);
                    break;
                }
                // Temps can also appear on the LHS, e.g. as m[r[tmp]], but
                // ignore direct assignment to it (i.e. *lhs == *mm->first)
                SemStr* lhs = pRT->getRHS();
                if ((!(*lhs == mm->first)) &&
                  (lhs->search(mm->first, replace))) {
                    temps.erase(mm);
                    break;
                }
            } else if ((*it)->getKind() == RTFLAGCALL) {
                // If used in a flag call, still used
                RTFlagCall* pRT = (RTFlagCall*)(*it);
                srch = mm->first;
                if (pRT->searchAll(srch, result)) {
                    temps.erase(mm);
                    break;
                }
            }
        }
    }

    // Any entries left in the map can have their assignments deleted
    for (mm = temps.begin(); mm != temps.end(); mm++) {
        for (it = rtlist.begin(); it != rtlist.end(); ) {
            if ((*it)->getKind() != RTASSGN) {it++; continue;}
            RTAssgn* pRT = (RTAssgn*) *it;
            SemStr* lhs = pRT->getLHS();
            if (*lhs == mm->first) {
                // Delete the assignment
                it = rtlist.erase(it);
                continue;
            }
            it++;
        }
    }

}

/******************************************************************************
 * RTL methods.
 * Class RTL represents a low-level RTL. These are the result of decoding
 * non-CTI source machine instructions.
 *****************************************************************************/

/*==============================================================================
 * FUNCTION:        RTL::RTL
 * OVERVIEW:        Constructor.
 * PARAMETERS:      none.
 * RETURNS:         N/a
 *============================================================================*/
RTL::RTL()
    : HRTL()
{
    kind = LOW_LEVEL_HRTL;
}

/*==============================================================================
 * FUNCTION:        RTL::RTL
 * OVERVIEW:        Constructor.
 * PARAMETERS:      instNativeAddr: native address of the RTL
 *                  listRT: a list of RTs to serve as the initial list of RTs
 * RETURNS:         N/a
 *============================================================================*/
RTL::RTL(ADDRESS instNativeAddr, list<RT*>* listRT /*= NULL*/)
    : HRTL(instNativeAddr, listRT)
{
    kind = LOW_LEVEL_HRTL;
}

/*==============================================================================
 * FUNCTION:        RTL::~RTL
 * OVERVIEW:        Destructor
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
RTL::~RTL()
{ }

/*==============================================================================
 * FUNCTION:        RTL::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new HRTL that is a clone of this one
 *============================================================================*/
HRTL* RTL::clone() const {
    return HRTL::clone();
}

/*==============================================================================
 * FUNCTION:        RTL::print
 * OVERVIEW:        Prints this object to a stream in text form.
 * PARAMETERS:      os - stream to output to (often cout or cerr)
 *                  indent - Number of characters to move across
 * RETURNS:         <nothing>
 *============================================================================*/
void RTL::print(ostream& os /*= cout*/, int indent /*= 0*/) const {
    HRTL::print(os, indent);
}


/******************************************************************************
 * HLJump methods
 *****************************************************************************/

/*==============================================================================
 * FUNCTION:        HLJump::HLJump
 * OVERVIEW:        Constructor.
 * PARAMETERS:      instNativeAddr: native address of the RTL
 *                  listRT: a list of RTs (not the same as an RTL) to serve
 *                      as the initial list of RTs
 * RETURNS:         N/a
 *============================================================================*/
HLJump::HLJump(ADDRESS instNativeAddr, list<RT*>* listRT /*= NULL*/)
    : HRTL(instNativeAddr, listRT), pDest(0), m_isComputed(false)
{
    kind = JUMP_HRTL;
}

/*==============================================================================
 * FUNCTION:        HLJump::HLJump
 * OVERVIEW:        Construct a jump to a fixed address
 * PARAMETERS:      instNativeAddr: native address of the jump RTL
 *                  uDest: native address of destination
 * RETURNS:         N/a
 *============================================================================*/
HLJump::HLJump(ADDRESS instNativeAddr, ADDRESS uDest) :
    HRTL(instNativeAddr), m_isComputed(false)
{
    kind = JUMP_HRTL;
    // Note: we used to generate an assignment (pc := <dest>), but it gets
    // ignored anyway, and it causes us to declare pc as a variable in the back
    // end. So now the semantics of a HLJUMP are purely implicit
    pDest = new SemStr;
    pDest->push(idIntConst);
    pDest->push(uDest);
}

/*==============================================================================
 * FUNCTION:        HLJump::~HLJump
 * OVERVIEW:        Destructor
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
HLJump::~HLJump()
{
    if (pDest) delete pDest;
}

/*==============================================================================
 * FUNCTION:        HLJump::getFixedDest
 * OVERVIEW:        Get the fixed destination of this CTI. Assumes destination
 *                  simplication has already been done so that a fixed dest will
 *                  be of the SemStr form:
 *                     idIntConst dest
 * PARAMETERS:      <none>
 * RETURNS:         Fixed dest or -1 if there isn't one
 *============================================================================*/
ADDRESS HLJump::getFixedDest() const
{
    if (pDest && pDest->len() == 2 && pDest->getFirstIdx() == idIntConst)
        return pDest->getSecondIdx();
    else
        return NO_ADDRESS;
}


/*==============================================================================
 * FUNCTION:        HLJump::setDest
 * OVERVIEW:        Set the destination of this CTI to be a given address.
 * PARAMETERS:      addr - the new fixed address
 * RETURNS:         Nothing
 *============================================================================*/
void HLJump::setDest(SemStr* pd)
{
    if (pDest != NULL)
        delete pDest;
    pDest = pd;
}

/*==============================================================================
 * FUNCTION:        HLJump::setDest
 * OVERVIEW:        Set the destination of this CTI to be a given fixed address.
 * PARAMETERS:      addr - the new fixed address
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJump::setDest(ADDRESS addr)
{
    // Delete the old destination if there is one
    if (pDest != NULL)
        delete pDest;

    pDest = new SemStr();
    pDest->push(idIntConst);
    pDest->push(addr);
}

/*==============================================================================
 * FUNCTION:        HLJump::getDest
 * OVERVIEW:        Returns the destination of this CTI.
 * PARAMETERS:      None
 * RETURNS:         Pointer to the SS representing the dest of this jump
 *============================================================================*/
SemStr* HLJump::getDest() const
{
    return pDest;
}

/*==============================================================================
 * FUNCTION:        HLJump::adjustFixedDest
 * OVERVIEW:        Adjust the destination of this CTI by a given amount. Causes
 *                  an error is this destination is not a fixed destination
 *                  (i.e. a constant offset).
 * PARAMETERS:      delta - the amount to add to the destination (can be
 *                  negative)
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJump::adjustFixedDest(int delta)
{
    // Ensure that the destination is fixed.
    if (pDest == NULL || pDest->getIndex(pDest->len() - 2) != idIntConst)
        error("Can't adjust destination of non-static CTI");

    int dest = pDest->pop();
    pDest->push(dest+delta);
}

/*==============================================================================
 * FUNCTION:        HLJump::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 *                  typeSens - if true, the search is sensitive to type
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJump::searchAndReplace(SemStr& search, const SemStr& replace,
    bool typeSens)
{
    HRTL::searchAndReplace(search, replace, typeSens);
    if (pDest)
        pDest->searchReplaceAll(search, replace, typeSens);
}

/*==============================================================================
 * FUNCTION:        HLJump::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 *                  typeSens - if true, consider type when matching
 * NOTES:           search can't easily be made const
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool HLJump::searchAll(SemStr& search, list<SemStr *> &result,
                       bool typeSens)
{
    return HRTL::searchAll(search, result, typeSens) ||
        ( pDest && pDest->searchAll(search, result, typeSens) );
}

/*==============================================================================
 * FUNCTION:        HLJump::print
 * OVERVIEW:        Display a text reprentation of this RTL to the given stream
 * PARAMETERS:      os: stream to write to
 *                  indent: number of columns to skip
 * RETURNS:         Nothing
 *============================================================================*/
void HLJump::print(ostream& os /*= cout*/, int indent /*= 0*/) const
{
    // Returns can all have semantics (e.g. ret/restore)
    if (rtlist.size() != 0)
        HRTL::print(os, indent);

    os << hex << setfill('0') << setw(8) << nativeAddr;
    os << dec << setfill(' ');      // Ugh - why is this needed?
    if (indent == 0) indent = 1;
    os << setw(indent) << " ";
    if (getKind() == RET_HRTL)
    {
        os << " RET\n";             // RET is a special case of a JUMP_HRTL
        return;
    }

    os << " JUMP ";
    if (pDest == NULL)
        os << "*no dest*";
    else if (pDest->getFirstIdx() != idIntConst)
         pDest->print(os);
    else
        os << hex << getFixedDest();
    os << endl;
}

/*==============================================================================
 * FUNCTION:      HLJump::setIsComputed
 * OVERVIEW:      Sets the fact that this call is computed.
 * NOTE:          This should really be removed, once HLNwayJump and HLNwayCall
 *                  are implemented properly
 * PARAMETERS:    <none>
 * RETURNS:       <nothing>
 *============================================================================*/
void HLJump::setIsComputed()
{
    m_isComputed = true;
}

/*==============================================================================
 * FUNCTION:      HLJump::isComputed
 * OVERVIEW:      Returns whether or not this call is computed.
 * NOTE:          This should really be removed, once HLNwayJump and HLNwayCall
 *                  are implemented properly
 * PARAMETERS:    <none>
 * RETURNS:       this call is computed
 *============================================================================*/
bool HLJump::isComputed() const
{
    return m_isComputed;
}

/*==============================================================================
 * FUNCTION:        HLJump::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new HRTL that is a clone of this one
 *============================================================================*/
HRTL* HLJump::clone() const
{
    list<RT*> rlist;
    deepCopyList(rlist);
    HLJump* ret = new HLJump(nativeAddr, &rlist);
    ret->pDest = new SemStr(*pDest);
    ret->m_isComputed = m_isComputed;
    ret->numNativeBytes = numNativeBytes;
    return ret;
}

/*==============================================================================
 * FUNCTION:        HLJump::getUseDefLocations
 * OVERVIEW:        The DFA analysis of a jump RTL extends that of a
 *                    standard RTL in that it *uses* registers etc in its
 *                    destination expression. E.g. jump to r[25], r[25] is used
 * PARAMETERS:      locMap - a map between locations and integers
 *                  filter - a filter to restrict which locations are
 *                    considered
 *                  useSet - has added to it those locations used this BB
 *                  defSet - has added to it those locations defined this BB
 *                  useUndefSet - has added those locations used before defined
 *                  proc - pointer to the Proc object containing this HRTL
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJump::getUseDefLocations(LocationMap& locMap, LocationFilter* filter,
    BITSET& defSet, BITSET& useSet, BITSET& useUndefSet, Proc* proc) const
{
    // If jumps ever have semantics, then this call would be needed
    // HRTL::getUseDefLocations(locMap, filter, defSet, useSet, useUndefSet,
        // proc);

    if (pDest)
        searchExprForUses(pDest, locMap, filter, defSet, useSet, useUndefSet);
}

/**********************************
 * HLJcond methods
 **********************************/

/*==============================================================================
 * FUNCTION:        HLJcond::HLJcond
 * OVERVIEW:        Constructor.
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
HLJcond::HLJcond(ADDRESS instNativeAddr, list<RT*>* listRT /*= NULL*/) :
    HLJump(instNativeAddr, listRT), jtCond((JCOND_TYPE)0), pCond(NULL),
    bFloat(false)
{
    kind = JCOND_HRTL;
}

/*==============================================================================
 * FUNCTION:        HLJcond::~HLJcond
 * OVERVIEW:        Destructor
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
HLJcond::~HLJcond()
{
    if (pCond)
        delete pCond;
}

/*==============================================================================
 * FUNCTION:        HLJcond::setCondType
 * OVERVIEW:        Sets the JCOND_TYPE of this jcond as well as the flag
 *                  indicating whether or not the floating point condition codes
 *                  are used.
 * PARAMETERS:      cond - the JCOND_TYPE
 *                  usesFloat - this condional jump checks the floating point
 *                    condition codes
 * RETURNS:         a semantic string
 *============================================================================*/
void HLJcond::setCondType(JCOND_TYPE cond, bool usesFloat /*= false*/)
{
    jtCond = cond;
    bFloat = usesFloat;
}

/*==============================================================================
 * FUNCTION:        HLJcond::makeSigned
 * OVERVIEW:        Change this from an unsigned to a signed branch
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJcond::makeSigned()
{
    // Make this into a signed branch
    switch (jtCond)
    {
        case HLJCOND_JUL : jtCond = HLJCOND_JSL;  break;
        case HLJCOND_JULE: jtCond = HLJCOND_JSLE; break;
        case HLJCOND_JUGE: jtCond = HLJCOND_JSGE; break;
        case HLJCOND_JUG : jtCond = HLJCOND_JSG;  break;
        default:
            // Do nothing for other cases
            break;
    }
}

/*==============================================================================
 * FUNCTION:        HLJcond::getCondExpr
 * OVERVIEW:        Return the SemStr expression containing the HL condition.
 * PARAMETERS:      <none>
 * RETURNS:         a semantic string
 *============================================================================*/
SemStr* HLJcond::getCondExpr() const
{
    return pCond;
}

/*==============================================================================
 * FUNCTION:        HLJcond::setCondExpr
 * OVERVIEW:        Set the SemStr expression containing the HL condition.
 * PARAMETERS:      Pointer to semantic string to set
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJcond::setCondExpr(SemStr* pss)
{
    pCond = pss;
}

/*==============================================================================
 * FUNCTION:        HLJcond::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 *                  typeSens - if true, the search is sensitive to type
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJcond::searchAndReplace(SemStr& search, const SemStr& replace,
    bool typeSens)
{
    HLJump::searchAndReplace(search, replace, typeSens);
    if (pCond)
        pCond->searchReplaceAll(search, replace, typeSens);
}

/*==============================================================================
 * FUNCTION:        HLJCond::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 *                  typeSens - if true, consider type when matching
 * NOTES:           search can't easily be made const
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool HLJcond::searchAll(SemStr& search, list<SemStr *> &result,
                       bool typeSens)
{
    return HRTL::searchAll(search, result, typeSens) ||
        (pCond && pCond->searchAll(search, result, typeSens) );
}


/*==============================================================================
 * FUNCTION:        HLJcond::print
 * OVERVIEW:        Write a text representation to the given stream
 * PARAMETERS:      os: stream
 *                  indent: number of columns to skip
 * RETURNS:         Nothing
 *============================================================================*/
void HLJcond::print(ostream& os /*= cout*/, int indent /*= 0*/) const
{
    // These can have semantics (e.g. pa-risc add and (conditionally) branch)
    if (rtlist.size() != 0)
        HRTL::print(os, indent);
    os << hex << setfill('0') << setw(8) << nativeAddr;
    os << dec << setfill(' ');      // Ugh - why is this needed?
    if (indent == 0) indent = 1;
    os << setw(indent) << " ";
    os << " JCOND ";
    if (pDest == NULL)
        os << "*no dest*";
    else if (pDest->getIndex(pDest->len() - 2) != idIntConst)
        os << *pDest;
    else {
        // Really we'd like to display the destination label here...
        os << hex << getFixedDest();
    }
    os << ", condition ";
    switch (jtCond)
    {
        case HLJCOND_JE:    os << "equals"; break;
        case HLJCOND_JNE:   os << "not equals"; break;
        case HLJCOND_JSL:   os << "signed less"; break;
        case HLJCOND_JSLE:  os << "signed less or equals"; break;
        case HLJCOND_JSGE:  os << "signed greater or equals"; break;
        case HLJCOND_JSG:   os << "signed greater"; break;
        case HLJCOND_JUL:   os << "unsigned less"; break;
        case HLJCOND_JULE:  os << "unsigned less or equals"; break;
        case HLJCOND_JUGE:  os << "unsigned greater or equals"; break;
        case HLJCOND_JUG:   os << "unsigned greater"; break;
        case HLJCOND_JMI:   os << "minus"; break;
        case HLJCOND_JPOS:  os << "plus"; break;
        case HLJCOND_JOF:   os << "overflow"; break;
        case HLJCOND_JNOF:  os << "no overflow"; break;
        case HLJCOND_JPAR:  os << "parity"; break;
    }
    if (bFloat) os << " float";
    os << endl;
    if (pCond) {
        os << "High level: " << *pCond << endl;
    }
}

/*==============================================================================
 * FUNCTION:        HLJcond::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new HRTL that is a clone of this HLJcond
 *============================================================================*/
HRTL* HLJcond::clone() const
{
    list<RT*> rlist;
    deepCopyList(rlist);
    HLJcond* ret = new HLJcond(nativeAddr, &rlist);
    ret->pDest = new SemStr(*pDest);
    ret->m_isComputed = m_isComputed;
    ret->jtCond = jtCond;
    if (pCond) ret->pCond = new SemStr(*pCond);
    else ret->pCond = NULL;
    ret->m_isComputed = m_isComputed;
    ret->bFloat = bFloat;
    ret->numNativeBytes = numNativeBytes;
    return ret;
}

/**********************************
 * HLNwayJump methods
 **********************************/
/*==============================================================================
 * FUNCTION:        HLNwayJump::HLNwayJump
 * OVERVIEW:        Constructor.
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
HLNwayJump::HLNwayJump(ADDRESS instNativeAddr, list<RT*>* listRT /*= NULL*/) :
    HLJump(instNativeAddr, listRT), pSwitchInfo(NULL)
{
    kind = NWAYJUMP_HRTL;
}

/*==============================================================================
 * FUNCTION:        HLNwayJump::~HLNwayJump
 * OVERVIEW:        Destructor
 * NOTE:            Don't delete the pSwitchVar; it's always a copy of something
 *                  else (so don't delete twice)
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
HLNwayJump::~HLNwayJump()
{
    if (pSwitchInfo)
        delete pSwitchInfo;
}

/*==============================================================================
 * FUNCTION:        HLNwayJump::getSwitchInfo
 * OVERVIEW:        Return a pointer to a struct with switch information in it
 * PARAMETERS:      <none>
 * RETURNS:         a semantic string
 *============================================================================*/
SWITCH_INFO* HLNwayJump::getSwitchInfo() const
{
    return pSwitchInfo;
}

/*==============================================================================
 * FUNCTION:        HLNwayJump::setSwitchInfo
 * OVERVIEW:        Set a pointer to a SWITCH_INFO struct
 * PARAMETERS:      Pointer to SWITCH_INFO struct
 * RETURNS:         <nothing>
 *============================================================================*/
void HLNwayJump::setSwitchInfo(SWITCH_INFO* psi)
{
    pSwitchInfo = psi;
}

/*==============================================================================
 * FUNCTION:        HLNwayJump::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 *                  typeSens - if true, the search is sensitive to type
 * RETURNS:         <nothing>
 *============================================================================*/
void HLNwayJump::searchAndReplace(SemStr& search, const SemStr& replace,
    bool typeSens)
{
    HLJump::searchAndReplace(search, replace, typeSens);
    if (pSwitchInfo && pSwitchInfo->pSwitchVar)
        pSwitchInfo->pSwitchVar->searchReplaceAll(search, replace, typeSens);
}

/*==============================================================================
 * FUNCTION:        HLNwayJump::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 *                  typeSens - if true, consider type when matching
 * NOTES:           search can't easily be made const
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool HLNwayJump::searchAll(SemStr& search, list<SemStr *> &result,
                       bool typeSens)
{
    return HLJump::searchAll(search, result, typeSens) ||
        ( pSwitchInfo && pSwitchInfo->pSwitchVar &&
          pSwitchInfo->pSwitchVar->searchAll(search, result, typeSens) );
}

/*==============================================================================
 * FUNCTION:        HLNwayJump::print
 * OVERVIEW:        Write a text representation to the given stream
 * PARAMETERS:      os: stream
 *                  indent: number of columns to skip
 * RETURNS:         Nothing
 *============================================================================*/
void HLNwayJump::print(ostream& os /*= cout*/, int indent /*= 0*/) const
{
    os << hex << setfill('0') << setw(8) << nativeAddr;
    os << dec << setfill(' ');      // Ugh - why is this needed?
    if (indent == 0) indent = 1;
    os << setw(indent) << " ";
    os << " NWAY_JUMP [";
    if (pDest == NULL)
        os << "*no dest*";
    else os << *pDest;
    os << "] ";
    if (pSwitchInfo) {
        os << "Switch variable: ";
        pSwitchInfo->pSwitchVar->print(os);
        os << endl;
    }
}


/*==============================================================================
 * FUNCTION:        HLNwayJump::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new HRTL that is a clone of this one
 *============================================================================*/
HRTL* HLNwayJump::clone() const
{
    list<RT*> rlist;
    deepCopyList(rlist);
    HLNwayJump* ret = new HLNwayJump(nativeAddr, &rlist);
    ret->pDest = new SemStr(*pDest);
    ret->m_isComputed = m_isComputed;
    ret->numNativeBytes = numNativeBytes;
    ret->pSwitchInfo = new SWITCH_INFO;
    *ret->pSwitchInfo = *pSwitchInfo;
    ret->pSwitchInfo->pSwitchVar = new SemStr(*pSwitchInfo->pSwitchVar);
    return ret;
}


/**********************************
 * HLCall methods
 **********************************/

/*==============================================================================
 * FUNCTION:         HLCall::HLCall
 * OVERVIEW:         Constructor for a call that we have extra information
 *                   for and is part of a prologue.
 * PARAMETERS:       instNativeAddr - the address of the call instruction
 *                   returnTypeSize - the size of a return union, struct or quad
 *                     floating point value
 * RETURNS:          <nothing>
 *============================================================================*/
HLCall::HLCall(ADDRESS instNativeAddr, int returnTypeSize /*= 0*/, list<RT*>* listRT /*= NULL*/):
    HLJump(instNativeAddr, listRT),returnTypeSize(returnTypeSize), returnAfterCall(false)
{
    kind = CALL_HRTL;
    prologue = NULL;
    epilogue = NULL;
    basicBlock = NULL;
    postCallRtlist = NULL;
    pDestName = NULL;
}

/*==============================================================================
 * FUNCTION:      HLCall::~HLCall
 * OVERVIEW:      Sets the link from this call to its enclosing BB.
 * PARAMETERS:    BB - the enclosing basic block of this call
 * RETURNS:       <nothing>
 *============================================================================*/
HLCall::~HLCall()
{
    if (prologue != NULL)
        delete prologue;
    if (epilogue != NULL)
        delete epilogue;
}

/*==============================================================================
 * FUNCTION:      HLCall::setBB
 * OVERVIEW:      Sets the link from this call to its enclosing BB.
 * PARAMETERS:    BB - the enclosing basic block of this call
 * RETURNS:       <nothing>
 *============================================================================*/
void HLCall::setBB(PBB BB)
{
    basicBlock = BB;
}

/*==============================================================================
 * FUNCTION:      HLCall::getBB
 * OVERVIEW:      Get the enclosing BB of this call.
 * PARAMETERS:    <none>
 * RETURNS:       the enclosing basic block of this call
 *============================================================================*/
const PBB HLCall::getBB()
{
    return basicBlock;
}

/*==============================================================================
 * FUNCTION:      HLCall::getParams
 * OVERVIEW:      Return a copy of the locations that have been determined
 *                as the actual parameters for this call.
 * PARAMETERS:    <none>
 * RETURNS:       A const reference to the parameters
 *============================================================================*/
const list<SemStr>& HLCall::getParams() const
{
    return params;
}

/*==============================================================================
 * FUNCTION:      HLCall::getParams
 * OVERVIEW:      Return a copy of the locations that have been determined
 *                as the actual parameters for this call.
 * NOTE:          Non const, so the parameters can be manipilated in
 *                  CSR::matchCallParams()
 * PARAMETERS:    <none>
 * RETURNS:       A non-const reference to the parameters
 *============================================================================*/
list<SemStr>& HLCall::getParams()
{
    return params;
}

/*==============================================================================
 * FUNCTION:      HLCall::setParams
 * OVERVIEW:      Set the parameters of this call.
 * PARAMETERS:    params - the list of locations live at this call
 * RETURNS:       <nothing>
 *============================================================================*/
void HLCall::setParams(const list<SemStr>& params)
{
    this->params = params;
}

/*==============================================================================
 * FUNCTION:      HLCall::setReturnLoc
 * OVERVIEW:      Set the location that will be used to hold
 *                the value returned by this call.
 * PARAMETERS:    loc -
 * RETURNS:       <nothing>
 *============================================================================*/
void HLCall::setReturnLoc(const SemStr& loc)
{
    returnLoc = loc;
}

/*==============================================================================
 * FUNCTION:      HLCall::getReturnLoc
 * OVERVIEW:      Return a copy of the location that will be used to hold
 *                the value returned by this call.
 * PARAMETERS:    <none>
 * RETURNS:       the location that will be used to hold the return value
 *============================================================================*/
SemStr HLCall::getReturnLoc() const
{
    return returnLoc;
}

/*==============================================================================
 * FUNCTION:        HLCall::getUseDefLocations
 * OVERVIEW:        The DFA analysis of a call RTL extends that of a
 *                    standard RTL in that it *uses* its parameters.
 * PARAMETERS:      locMap - a map between locations and integers
 *                  filter - a filter to restrict which locations are
 *                    considered
 *                  useSet - has added to it those locations used this BB
 *                  defSet - has added to it those locations defined this BB
 *                  useUndefSet - has added those locations used before defined
 *                  proc - pointer to the Proc object containing this HRTL
 * RETURNS:         <nothing>
 *============================================================================*/
void HLCall::getUseDefLocations(LocationMap& locMap,
    LocationFilter* filter, BITSET& defSet, BITSET& useSet,
    BITSET& useUndefSet, Proc* proc) const
{
    // Note: calls can have semantics now (mainly from restore instructions
    // in their delay slots).
    // So process the semantics (assignments) for this HLCall
    HRTL::getUseDefLocations(locMap, filter, defSet, useSet, useUndefSet,
        proc);

    // Calls are also jumps; the destination expression may use some locations
    HLJump::getUseDefLocations(locMap, filter, defSet, useSet, useUndefSet,
        proc);

    // Get the set of locations that are parameters for the call
    // Use a type insensitive set
    setSgiSemStr params_set;
    for (list<SemStr>::const_iterator it = params.begin();
      it != params.end(); it++)
        // We should not have vars here at this stage. These uses will be needed
        // for things like return location analysis, and the ReturnLocation
        // object will have strings like r[8], not v0
        if (it->getFirstIdx() != idVar)
            params_set.insert(*it);
    BITSET paramSet = locMap.toBitset(params_set);

    // Add each parameter to the use and if applicable the uneUndef set
    useSet |= paramSet;
    useUndefSet |= (paramSet & ~defSet);

    // Add the return location to the def set
    if (returnLoc.len() != 0)
        defSet.set(locMap.toBit(returnLoc));
}

/*==============================================================================
 * FUNCTION:         HLCall::returnsStruct
 * OVERVIEW:         Returns true if the function called by this call site
 *                   returns an aggregate value (i.e a struct, union or quad
 *                   floating point value).
 * PARAMETERS:       <none>
 * RETURNS:          the called function returns an aggregate value
 *============================================================================*/
bool HLCall::returnsStruct()
{
    return (returnTypeSize != 0);
}

/*==============================================================================
 * FUNCTION:      HLCall::setPrologue
 * OVERVIEW:      Set the prologue for this call.
 * PARAMETERS:    prologue - a caller prologue
 * RETURNS:       <nothing>
 *============================================================================*/
void HLCall::setPrologue(Logue* prologue)
{
    assert(this->prologue == NULL);
    this->prologue = prologue;
}

/*==============================================================================
 * FUNCTION:      HLCall::setEpilogue
 * OVERVIEW:      Set the epilogue for this call.
 * PARAMETERS:    epilogue - a caller epilogue
 * RETURNS:       <nothing>
 *============================================================================*/
void HLCall::setEpilogue(Logue* epilogue)
{
    assert(this->epilogue == NULL);
    this->epilogue = epilogue;
}

/*==============================================================================
 * FUNCTION:      HLCall::getPrologue
 * OVERVIEW:      Get the prologue (if any) for this call.
 * PARAMETERS:    <none>
 * RETURNS:       a caller prologue
 *============================================================================*/
Logue* HLCall::getPrologue() const
{
    return this->prologue;
}

/*==============================================================================
 * FUNCTION:      HLCall::getEpilogue
 * OVERVIEW:      Get the epilogue (if any) for this call.
 * PARAMETERS:    <none>
 * RETURNS:       a caller epilogue
 *============================================================================*/
Logue* HLCall::getEpilogue() const
{
    return this->epilogue;
}

/*==============================================================================
 * FUNCTION:        HLCall::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 *                  typeSens - if true, the search is sensitive to type
 * RETURNS:         <nothing>
 *============================================================================*/
void HLCall::searchAndReplace(SemStr& search, const SemStr& replace,
    bool typeSens)
{
    HLJump::searchAndReplace(search, replace, typeSens);
    if (returnLoc.len() != 0)
        returnLoc.searchReplaceAll(search, replace, typeSens);
    for (list<SemStr>::iterator it = params.begin();
      it != params.end(); it++)
        it->searchReplaceAll(search, replace, typeSens);
    // Also replace the postCall rtls, if any
    if (postCallRtlist) {
        for (RT_CIT it = postCallRtlist->begin(); it != postCallRtlist->end();
          it++)
            (*it)->searchAndReplace(search, replace, typeSens);
    }
}

/*==============================================================================
 * FUNCTION:        HLCall::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 *                  typeSens - if true, consider type when matching
 * NOTES:           search can't easily be made const
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool HLCall::searchAll(SemStr& search, list<SemStr *> &result,
                       bool typeSens)
{
    bool found = false;
    if( HLJump::searchAll(search, result, typeSens) ||
        (returnLoc.len() != 0 && returnLoc.searchAll(search, result, typeSens)))
        found = true;
    for (list<SemStr>::iterator it = params.begin();
      it != params.end(); it++)
        if( it->searchAll(search, result, typeSens) )
            found = true;
    // Also replace the postCall rtls, if any
    if (postCallRtlist) {
        for (RT_CIT it = postCallRtlist->begin(); it != postCallRtlist->end();
          it++)
            if( (*it)->searchAll(search, result, typeSens) )
                found = true;
    }
    return found;
}

/*==============================================================================
 * FUNCTION:        HLCall::print
 * OVERVIEW:        Write a text representation of this RTL to the given stream
 * PARAMETERS:      os: stream to write to
 *                  indent: number of columns to skip
 * RETURNS:         Nothing
 *============================================================================*/
void HLCall::print(ostream& os /*= cout*/, int indent /*= 0*/) const
{
    // Calls can all have semantics (e.g. call/restore)
    if (rtlist.size() != 0)
        HRTL::print(os, indent);

    os << hex << setfill('0') << setw(8) << nativeAddr;
    os << dec << setfill(' ');      // Ugh - why is this needed?
    if (indent == 0) indent = 1;
    os << setw(indent) << " ";

    // Print the return location if there is one
    if (returnLoc.len() != 0)
        os << " " << returnLoc << " :=";
 
    os << " CALL ";
    if (pDest == NULL) {
        if (pDestName)
            os << pDestName;
        else
            os << "*no dest*";
    }
    else if (pDest->getFirstIdx() != idIntConst) {
        // Not just an integer constant. Print as a semantic string
        os << *pDest << endl;
        return;
    }
    else {
        Proc* pProc = prog.findProc(getFixedDest());
        if ((pProc == NULL) || (pProc == (Proc*)-1))
            os << "0x"<< hex << getFixedDest();
        else
            os << pProc->getName();
    }

    // Print the actual parameters of the call
    os << "(";
    for (list<SemStr>::const_iterator it = params.begin();
      it != params.end(); it++) {
        if (it != params.begin())
            os << ", ";
        os << *it;
    }
    os << ")" << endl;

    // Print the post call RTLs, if any
    if (postCallRtlist) {
        for (RT_CIT it = postCallRtlist->begin(); it != postCallRtlist->end();
          it++) {
            os << setw(indent+8) << " ";
            (*it)->print(os);
            os << "\n";
        }
    }
}

/*==============================================================================
 * FUNCTION:         HLCall::setReturnAfterCall
 * OVERVIEW:         Sets a bit that says that this call is effectively followed
 *                      by a return. This happens e.g. on Sparc when there is a
 *                      restore in the delay slot of the call
 * PARAMETERS:       b: true if this is to be set; false to clear the bit
 * RETURNS:          <nothing>
 *============================================================================*/
void HLCall::setReturnAfterCall(bool b)
{
    returnAfterCall = b;
}

/*==============================================================================
 * FUNCTION:         HLCall::isReturnAfterCall
 * OVERVIEW:         Tests a bit that says that this call is effectively
 *                      followed by a return. This happens e.g. on Sparc when
 *                      there is a restore in the delay slot of the call
 * PARAMETERS:       none
 * RETURNS:          True if this call is effectively followed by a return
 *============================================================================*/
bool HLCall::isReturnAfterCall() const
{
    return returnAfterCall;
}

/*==============================================================================
 * FUNCTION:         HLCall::setPostCallRtlist
 * OVERVIEW:         Sets the list of RTs to be emitted after the call
 * PARAMETERS:       Pointer to the list of RTs to be saved
 * RETURNS:          <nothing>
 *============================================================================*/
void HLCall::setPostCallRtlist(const list<RT*>* lrt)
{
    postCallRtlist = lrt;
}

/*==============================================================================
 * FUNCTION:         HLCall::getPostCallRtlist
 * OVERVIEW:         Gets the list of RTs to be emitted after the call
 * PARAMETERS:       <None>
 * RETURNS:          List of RTs to be saved
 *============================================================================*/
const list<RT*>* HLCall::getPostCallRtlist() const
{
    return postCallRtlist;
}

/*==============================================================================
 * FUNCTION:         HLCall::setDestName
 * OVERVIEW:         Sets the name of the destination of the call
 * NOTE:             This is not for ordinary calls; their names are found by
 *                    looking up the native address of the destination in the
 *                    BinaryFile object
 * PARAMETERS:       Pointer to the name of the destination
 * RETURNS:          <nothing>
 *============================================================================*/
void HLCall::setDestName(const char* pName)
{
    pDestName = pName;
}

/*==============================================================================
 * FUNCTION:         HLCall::getDestNane
 * OVERVIEW:         Gets the name of the destination of the call
 * NOTE:             See note above
 * PARAMETERS:       <None>
 * RETURNS:          Pointer to the name
 *============================================================================*/
const char* HLCall::getDestName() const
{
    return pDestName;
}

/*==============================================================================
 * FUNCTION:        HLCall::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new HRTL that is a clone of this HLCall
 *============================================================================*/
HRTL* HLCall::clone() const
{
    list<RT*> rlist;
    deepCopyList(rlist);
    HLCall* ret = new HLCall(nativeAddr, returnTypeSize, &rlist);
    ret->pDest = new SemStr(*pDest);
    ret->m_isComputed = m_isComputed;
    ret->prologue = prologue;
    ret->epilogue = epilogue;
    ret->basicBlock = basicBlock;
    ret->params = params;
    ret->returnLoc = returnLoc;         // Copies whole SemStr
    ret->numNativeBytes = numNativeBytes;
    return ret;
}


/**********************************
 * HLReturn methods
 **********************************/

/*==============================================================================
 * FUNCTION:         HLReturn::HLReturn
 * OVERVIEW:         Constructor.
 * PARAMETERS:       instNativeAddr - the address of the return instruction
 *                   listRT - the RTs of the return
 * RETURNS:          <nothing>
 *============================================================================*/
HLReturn::HLReturn(ADDRESS instNativeAddr, list<RT*>* listRT /*= NULL*/):
    HLJump(instNativeAddr,listRT)
{
    kind = RET_HRTL;
}

/*==============================================================================
 * FUNCTION:         HLReturn::~HLReturn
 * OVERVIEW:         Destructor.
 * PARAMETERS:       <none>
 * RETURNS:          <nothing>
 *============================================================================*/HLReturn::~HLReturn()
{}

/*==============================================================================
 * FUNCTION:        HLReturn::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new HRTL that is a clone of this HLReturn
 *============================================================================*/
HRTL* HLReturn::clone() const
{
    list<RT*> rlist;
    deepCopyList(rlist);
    HLReturn* ret = new HLReturn(nativeAddr, &rlist);
    ret->pDest = NULL;                      // pDest should be null
    ret->m_isComputed = m_isComputed;
    ret->numNativeBytes = numNativeBytes;
    return ret;
}


/*==============================================================================
 * FUNCTION:        HLReturn::getUseDefLocations
 * OVERVIEW:        Get the set of locations used and defined in this BB
 * NOTE:            The return location is considered to be used, even if this
 *                    use is not explicit (e.g. in Sparc might return the first
 *                    parameter)
 * PARAMETERS:      locMap - a map between locations and integer bit numbers
 *                  filter - a filter to restrict which locations are
 *                    considered
 *                  useSet - has added to it those locations used this BB
 *                  defSet - has added to it those locations defined this BB
 *                  useUndefSet - has added those locations used before defined
 *                  proc - pointer to the Proc object containing this HRTL
 * RETURNS:         <nothing>
 *============================================================================*/
void HLReturn::getUseDefLocations(LocationMap& locMap,
    LocationFilter* filter, BITSET& defSet, BITSET& useSet,
    BITSET& useUndefSet, Proc* proc) const
{
    // It is possible that any RTL, including a HLReturn, has semantics
    // So process the semantics (assignments) for this HLCall
    HRTL::getUseDefLocations(locMap, filter, defSet, useSet, useUndefSet,
        proc);

    // Register a use for the return location. It may not be used anywhere
    // else; e.g. in the Sparc returnparam test, an empty procedure whose
    // integer return location is used must take a parameter
    const SemStr* retl = proc->getReturnLoc();
    if (retl->len()) {
        int bit = locMap.toBit(*retl);
        useSet.set(bit);
        // Add this to the use-before-definition set if necessary
        if (!defSet.test(bit))
            useUndefSet.set(bit);
    }
}

/**********************************
 * HLScond methods
 **********************************/

/*==============================================================================
 * FUNCTION:         HLScond::HLScond
 * OVERVIEW:         Constructor.
 * PARAMETERS:       instNativeAddr - the address of the set instruction
 *                   listRT - the RTs of the instr
 * RETURNS:          <N/a>
 *============================================================================*/
HLScond::HLScond(ADDRESS instNativeAddr, list<RT*>* listRT /*= NULL*/):
  HRTL(instNativeAddr, listRT), jtCond((JCOND_TYPE)0), pCond(NULL)
{
    kind = SCOND_HRTL;
}

/*==============================================================================
 * FUNCTION:        HLScond::~HLScond
 * OVERVIEW:        Destructor
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
HLScond::~HLScond()
{
    if (pCond)
        delete pCond;
}

/*==============================================================================
 * FUNCTION:        HLScond::setCondType
 * OVERVIEW:        Sets the JCOND_TYPE of this jcond as well as the flag
 *                  indicating whether or not the floating point condition codes
 *                  are used.
 * PARAMETERS:      cond - the JCOND_TYPE
 *                  usesFloat - this condional jump checks the floating point
 *                    condition codes
 * RETURNS:         a semantic string
 *============================================================================*/
void HLScond::setCondType(JCOND_TYPE cond, bool usesFloat /*= false*/)
{
    jtCond = cond;
    bFloat = usesFloat;
}

/*==============================================================================
 * FUNCTION:        HLScond::makeSigned
 * OVERVIEW:        Change this from an unsigned to a signed branch
 * NOTE:            Not sure if this is ever going to be used
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void HLScond::makeSigned()
{
    // Make this into a signed branch
    switch (jtCond)
    {
        case HLJCOND_JUL : jtCond = HLJCOND_JSL;  break;
        case HLJCOND_JULE: jtCond = HLJCOND_JSLE; break;
        case HLJCOND_JUGE: jtCond = HLJCOND_JSGE; break;
        case HLJCOND_JUG : jtCond = HLJCOND_JSG;  break;
        default:
            // Do nothing for other cases
            break;
    }
}

/*==============================================================================
 * FUNCTION:        HLScond::getCondExpr
 * OVERVIEW:        Return the SemStr expression containing the HL condition.
 * PARAMETERS:      <none>
 * RETURNS:         a semantic string
 *============================================================================*/
SemStr* HLScond::getCondExpr() const
{
    return pCond;
}

/*==============================================================================
 * FUNCTION:        HLScond::setCondExpr
 * OVERVIEW:        Set the SemStr expression containing the HL condition.
 * PARAMETERS:      Pointer to semantic string to set
 * RETURNS:         <nothing>
 *============================================================================*/
void HLScond::setCondExpr(SemStr* pss)
{
    pCond = pss;
}

/*==============================================================================
 * FUNCTION:        HLScond::print
 * OVERVIEW:        Write a text representation to the given stream
 * PARAMETERS:      os: stream
 *                  indent: number of columns to skip
 * RETURNS:         <Nothing>
 *============================================================================*/
void HLScond::print(ostream& os /*= cout*/, int indent /*= 0*/) const
{
    os << hex << setfill('0') << setw(8) << nativeAddr;
    os << dec << setfill(' ');      // Ugh - why is this needed?
    if (indent == 0) indent = 1;
    os << setw(indent) << " ";
    os << " SCOND ";
    getDest()->print(os);
    os << " := CC(";
    switch (jtCond)
    {
        case HLJCOND_JE:    os << "equals"; break;
        case HLJCOND_JNE:   os << "not equals"; break;
        case HLJCOND_JSL:   os << "signed less"; break;
        case HLJCOND_JSLE:  os << "signed less or equals"; break;
        case HLJCOND_JSGE:  os << "signed greater or equals"; break;
        case HLJCOND_JSG:   os << "signed greater"; break;
        case HLJCOND_JUL:   os << "unsigned less"; break;
        case HLJCOND_JULE:  os << "unsigned less or equals"; break;
        case HLJCOND_JUGE:  os << "unsigned greater or equals"; break;
        case HLJCOND_JUG:   os << "unsigned greater"; break;
        case HLJCOND_JMI:   os << "minus"; break;
        case HLJCOND_JPOS:  os << "plus"; break;
        case HLJCOND_JOF:   os << "overflow"; break;
        case HLJCOND_JNOF:  os << "no overflow"; break;
        case HLJCOND_JPAR:  os << "parity"; break;
    }
    os << ")";
    if (bFloat) os << ", float";
    os << endl;
    if (pCond) {
        os << "High level: ";
        pCond->print(os);
        os << endl;
    }
}

/*==============================================================================
 * FUNCTION:        HLScond::getDest
 * OVERVIEW:        Get the destination of the set. For now, we assume one
 *                  assignment RT, and we take the left hand side of that.
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to the semantic string
 *============================================================================*/
SemStr* HLScond::getDest() const
{
    assert(rtlist.size());
    RTAssgn* pRT = (RTAssgn*)rtlist.front();
    assert(pRT->getKind() == RTASSGN);
    return pRT->getLHS();
}

/*==============================================================================
 * FUNCTION:        HLScond::getSize
 * OVERVIEW:        Get the size of the set's assignment. For now, we assume
 *                  one assignment RT, and we take the size of that.
 * PARAMETERS:      <none>
 * RETURNS:         The size
 *============================================================================*/
int HLScond::getSize() const
{
    assert(rtlist.size());
    RTAssgn* pRT = (RTAssgn*)rtlist.front();
    assert(pRT->getKind() == RTASSGN);
    return (int)pRT->getSize();
}

/*==============================================================================
 * FUNCTION:        HLScond::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new HRTL that is a clone of this HLScond
 *============================================================================*/
HRTL* HLScond::clone() const
{
    list<RT*> rlist;
    deepCopyList(rlist);
    HLScond* ret = new HLScond(nativeAddr, &rlist);
    ret->jtCond = jtCond;
    if (pCond) ret->pCond = new SemStr(*pCond);
    else ret->pCond = NULL;
    ret->bFloat = bFloat;
    ret->numNativeBytes = numNativeBytes;
    return ret;
}

