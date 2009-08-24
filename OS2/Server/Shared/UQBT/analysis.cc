/*
 * Copyright (C) 1999-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       analysis.cc
 * OVERVIEW:   Perform higher level analysis on a procedure. The procedure has
 *              been decoded, and CSR analysis is complete, but a few source
 *              machine idioms may remain, especially flags. This module
 *              attempts to remove the flag uses
 *
 * Copyright (C) 1999-2001, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.48 $
 * 18 Jun 99 - Mike: created
 * 29 Jun 99 - Mike: dec treated like compare; use of temp vars to communicate
 *                      parameters of CC def's to CC uses
 * 30 Jun 99 - Mike: changes to handle HLSconds
 * 20 Jul 99 - Mike: Idioms cope with allSimplify() now; also duplicate the
 *              ss for HLJconds where needed
 * 05 Apr 00 - Mike: corrected the type of HLJcond HL expressions
 * 04 May 00 - Mike: Fixed code that replicates BBs using flags; used to assume
 *              that such BBs were all jconds
 * 08 May 00 - Mike: When duplicating a BB with a TWOWAY parent, don't assume
 *              that it's the true edge that leads to the current BB
 *              Also fixed duplicating of neutral BBs (neither use or define CC)
 *              Finally, FALL parent BBs are treated like ONEWAY BBs now
 * 19 May 00 - Mike: When copying BBs, check the copy as well as the original
 *              for uses of flags
 * 01 Jun 00 - Mike: Fixed "call BB leads to use of flags" message
 * 05 Sep 00 - Mike: Added checkBBconst to look for constants in text section
 * 26 Sep 00 - Mike: Added extra parameter flt to findDefs; keep searching
 *              until find right type (int/float) of CC definition
 * 14 Oct 00 - Mike: Fixed code reading FP constants for endianness
 * 20 Nov 00 - Mike: Treat SETFFLAGS as subtract-like; HLJCOND_JE and JNE
 *              treated in the sub-like way (so "cmp a,b be L" translates to
 *              if (a == b) goto L instead of if (a-b) == 0) goto L
 * 10 Jan 01 - Brian: Added another test (and parameter) to isFlagFloat() so
 *              that it returns true for a RTFlagCall where the 1st arg is
 *              a floating pt variable. Added more debugging support to
 *              findDefs().
 * 17 Jan 01 - Mike: Fixed a subtle problem where a duplicated BB can interfere
 *              with the fall through out edge of its original code; untabbed
 * 25 Jan 01 - Mike: Fixed rare case where instruction "subcc %g0, 3, %o0"
 *              sets %CF to a constant
 * 29 Jan 01 - Mike: Assignment to %CF is prepended to the subtract now (was
 *              appended). Makes a difference when the result of the subtract
 *              affects an operand (e.g. getterr() in 099.go, Sun cc 5.0)
 * 14 Feb 01 - Mike: Updated checkBBconst to cope with addresses where the
 *              offset happens to be zero (and hence is simplified away)
 * 19 Feb 01 - Mike: Major rewrite of checkBBconst to cope with situations such
 *                as those found in the ninths test where constants are
 *                propagated through expressions more complex than just SETHIs
 * 05 Mar 01 - Nathan: Several tweaks to make condition codes slightly less
 *              fragile. Types for JCond parameters now taken from the types of
 *              the flagcall actuals, also looks for "FLAGS" in the names of
 *              flag calls
 * 05 Mar 01 - Nathan: Fixed JCond construction to append the temp variable
 *              assignment to the flagcall, rather than prepending it.
 * 06 Mar 01 - Mike: insertAssign() is now a member of class RTlist;
 *              process[Add|Sub]Flags() sets the condition to correct size now
 * 16 Mar 01 - Mike: Changed checkBBconst() to be more general, so it correctly
 *              finds constants in Pentium programs, e.g. ninths
 *              Some changes were required to processConst() as well
 * 19 Mar 01 - Mike: processAddFlags sets the type of the condition now
 * 22 Mar 01 - Mike: Replaced swap4() with copySwap4(); loading of integers
 *              is swapped now if required
 * 31 Mar 01 - Mike: getFixedDest() returns NO_ADDRESS for non fixed addresses
 * 03 Apr 01 - Mike: in checkBBconst(), if there is an assignment of a non const
 *              expression to a register that was formerly const, delete the
 *              entry in constMap. Prevents some translation segfaults
 * 09 Apr 01 - Mike: Replaced all calls to insertAssign that had prep==true
 *              with insertAfterTemps; some compares were still wrong (099.go
 *              benchmark, fire_joseki)
 * 26 Apr 01 - Mike: retabbed; updated some debugging code
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 * 30 Aug 01 - Mike: Proc::newLocal returns a pointer now
 * 16 Oct 01 - Brian: processConst() now swaps constants during translation only
 *              if uqbt endianness != src endianness.
 * 22 Oct 01 - Mike: processConst is fatally flawed; comment out for now, and
 *				rely on -C if needed to copy FP constants from code segment
 */

#include "global.h"
#include "rtl.h"
#include "cfg.h"
#include "proc.h"
#include "prog.h"

#define DEBUG_ANALYSIS 0        // Non zero for debugging

typedef void (*ACTION_FUNC)(HRTLList* pOrigRtls, PBB pOrigBB, HRTLList_RIT rrit,
    UserProc* proc, HRTLList_IT rit, int ty);
// A map from register to constant address value. We want the comparisons to be
// sign insensitive
typedef map<SemStr, ADDRESS, lessSGI> KMAP;

int anyFlagsUsed(const SemStr* s);
void findDefs(PBB pBB, HRTLList* pRtls, Cfg* cfg, UserProc* proc, int ty, bool flt,
    HRTLList_IT rit, ACTION_FUNC f);
void analyse_assign(PBB pBB, HRTLList* pRtls, Cfg* cfg, HRTLList_IT rit);
void matchJScond(HRTLList* pOrigRtls, PBB pOrigBB, HRTLList_RIT rrit, UserProc* proc,
    HRTLList_IT rit, int ty);
void matchAssign(HRTLList* pOrigRtls, PBB pOrigBB, HRTLList_RIT rrit, UserProc* proc,
    HRTLList_IT rit, int flagUsed);
void processSubFlags(HRTL* rtl, HRTLList_RIT rrit, UserProc* proc, HRTL_KIND kd);
void processAddFlags(HRTL* rtl, HRTLList_RIT rrit, UserProc* proc, HRTL_KIND kd);
void checkBBconst(PBB pBB);
bool isFlagFloat(const RTFlagCall* rt, UserProc* proc);

// A global array for initialising the below
int memXinit[] = {idMemOf, -1};
// A global SemStr representing a wildcard memory expression
SemStr memX(memXinit, memXinit+2);

/*==============================================================================
 * FUNCTION:        checkBBflags
 * OVERVIEW:        Check this BB for assignments that may use flags.
 *                    If present, calls findDefs to match with a definition
 * NOTE:            Not an error if no defs found (happens when duplicating
 *                    neutral BBs)
 * PARAMETERS:      pBB: Pointer to the BB to be checked
 *                  proc: Ptr to UserProc object for the current procedure
 * RETURNS:         <none>
 *============================================================================*/
void checkBBflags(PBB pBB, UserProc* proc)
{
    Cfg* cfg = proc->getCFG();
    HRTLList* pRtls = pBB->getHRTLs();
    if (pRtls == 0)
        return;

    HRTLList_IT rit;
    for (rit = pRtls->begin(); rit != pRtls->end(); rit++) {
        // Locate any uses of flags. These will be in HLJconds,
        // in certain kinds of arithmetic instructions, and in SET instrs
        HRTL_KIND kd = (*rit)->getKind();
//        if ((kd == JCOND_HRTL) || (kd == SCOND_HRTL))
//            findDefs(pBB, pRtls, cfg, proc, (int)kd, rit, matchJScond);
        if (kd == JCOND_HRTL) {
            HLJcond* pj = (HLJcond*)(*rit);
            // Only find a definition if doesn't already have a high level
            // expression. (This can happen with all the recursion and gnarly
            // branches in some code, especially SPARC)
            if (pj->getCondExpr() == NULL)
                findDefs(pBB, pRtls, cfg, proc, JCOND_HRTL, pj->isFloat(),
                rit, matchJScond);
        }
        else if (kd == SCOND_HRTL) {
            HLScond* pj = (HLScond*)(*rit);
            if (pj->getCondExpr() == NULL)
                findDefs(pBB, pRtls, cfg, proc, SCOND_HRTL, pj->isFloat(),
                rit, matchJScond);
        }
        else if (kd == LOW_LEVEL_HRTL) {
            // Check this ordinary RTL for assignments that use flags
            int n = (*rit)->getNumRT();
            for (int i=0; i < n; i++) {
                if ((*rit)->elementAt(i)->getKind() != RTASSGN)
                    continue;
                RTAssgn* pRT = (RTAssgn*)(*rit)->elementAt(i);
                SemStr* pRHS = pRT->getRHS();
                int flagUsed = anyFlagsUsed(pRHS);
                if (flagUsed)
                    // We have a use of a flag
                    findDefs(pBB, pRtls, cfg, proc, flagUsed, false, rit,
                        matchAssign);
           }
       }
    }
}

/*==============================================================================
 * FUNCTION:        finalSimplify
 * OVERVIEW:        Perform any final simplifications, post analysis. At
 *                  present, this is the removal of idAddrOf / idMemOf pairs
 * PARAMETERS:      pBB: pointer to the BB to be simplified
 * RETURNS:         <none>
 *============================================================================*/
void finalSimplify(PBB pBB)
{
    HRTLList* pRtls = pBB->getHRTLs();
    HRTLList_IT rit;
    for (rit = pRtls->begin(); rit != pRtls->end(); rit++) {
        for (int i=0; i < (*rit)->getNumRT(); i++) {
            RT* rt = (*rit)->elementAt(i);
            if (rt->getKind() != RTASSGN) continue;
            RTAssgn* rta = (RTAssgn*)rt;
            rta->getLHS()->simplifyAddr();
            rta->getRHS()->simplifyAddr();
        }
    }
}

/*==============================================================================
 * FUNCTION:        analysis
 * OVERVIEW:        Perform any higher level analysis on a procedure. At
 *                  present, this is the removal of uses of flags (checkBBflags)
 *                  and a pass looking for constants in the text section
 *                  (checkBBconst)
 * PARAMETERS:      proc: pointer to a Proc object for the procedure being
 *                  analysed
 * RETURNS:         <none>
 *============================================================================*/
void analysis(UserProc* proc)
{
    Cfg* cfg = proc->getCFG();
    BB_CIT it;
    PBB pBB = cfg->getFirstBB(it);
    while (pBB)
    {
        // Match flags
        checkBBflags(pBB, proc);
        // Perform final simplifications
        finalSimplify(pBB);
#if USE_PROCESS_CONST		// Fatally flawed. Somehow, conversion of
							// strings and function pointers has disappeared!
        // Check for constants
        checkBBconst(pBB);
#endif // USE_PROCESS_CONST
        pBB = cfg->getNextBB(it);
    }
}

#if DEBUG_ANALYSIS
static int findDefsCallDepth = 0;
#endif

/*==============================================================================
 * FUNCTION:        findDefs
 * OVERVIEW:        Finds a definition for this use (jcond, scond, or assign)
 *                  of the flags, and when found, calls a given function.
 * PARAMETERS:      pBB: pointer to current BB (with the use of the flag)
 *                  pRtls: pointer to list of HRTL pointers for current BB
 *                  cfg: pointer to Cfg for current Proc
 *                  proc: pointer to the current Proc object
 *                  kd: Either the HL_KIND of the RTL, (either JCOND_HRTL or
 *                  SCOND_HRTL), or the id of the flag used (if an assignment)
 *                  flt: true if this involves the floating point CCs
 *                  rit: iterator to the instruction using the CCs
 *                  f: pointer to function to actually match def and use
 * RETURNS:         <nothing>
 *===========================================================================*/
void findDefs(PBB pBB, HRTLList* pRtls, Cfg* cfg, UserProc* proc, int kd, bool flt,
    HRTLList_IT rit, ACTION_FUNC f)
{
    // We may follow a chain of BBs if there is only one in-edge, so keep
    // pointers to the original BB and list of RTLs
    PBB   pOrigBB   = pBB;
    HRTLList* pOrigRtls = pRtls;
    bool defFound = false;
    
#if DEBUG_ANALYSIS
    findDefsCallDepth++;
    cout << "\nfindDefs(" << findDefsCallDepth
      << "): Finding def for CC used at " << hex << (*rit)->getAddress()
      << " in " << proc->getName() << " BB 0x" << (unsigned int)pBB << endl;
#endif

    HRTLList_RIT rrit = pRtls->rbegin();
    // rit might be in the middle of the BB, so keep incrementing until we are
    // at rit
    while (rrit != pRtls->rend()) {
        if (*rrit == *rit) break;
        rrit++;
    }
    assert(rrit != pRtls->rend());      // Should find rit in this BB

    while (!defFound) {
        while (rrit != pRtls->rend()) {
            int n = (*rrit)->getNumRT();
            if (n > 0) {
                // Get the last RT
                RT* pRT = (*rrit)->elementAt(n-1);
                // Ensure that the "floatnesses" agree; don't be fooled
                // by interleaved integer and float defines and uses`
                if ((pRT->getKind() == RTFLAGCALL) &&
                  (((RTFlagCall *)pRT)->func_name.find("FLAG") != string::npos)
                  && (isFlagFloat((RTFlagCall*)pRT, proc) == flt)) {
                    defFound = true;          // Found RTL that sets cc
                    break;
                }
                // Iterate through the RTs looking for definitions of flags
                // (e.g. X86 bit instructions). Assume assignments to flags
                // are only for integer instructions
                if (!flt) {
                    for (int i=n-1; i >=0; i--) {
                        RTAssgn* pRT = (RTAssgn*)(*rrit)->elementAt(i);
                        if (pRT->getKind() == RTASSGN) {
                            SemStr* pLHS = pRT->getLHS();
                            if (anyFlagsUsed(pLHS)) {
                                defFound = true;
                                break;
                            }
                        }
                    }
                }
                if (defFound) break;
            }
            rrit++;
        }
        if (defFound) break;
        
        // If only one in-edge, then continue searching that BB
        const vector<PBB>& inEdges = pBB->getInEdges();
        if (inEdges.size() != 1) break;
        pBB = inEdges[0];
        if ((pBB->getType() == CALL) || (pBB->getType() == COMPCALL)) {
            ostrstream ost;
            ost << "jcond at " << hex << pOrigBB->getHiAddr();
            ost << " has a CALL BB leading to it (call to ";
            HRTLList* prtls = pBB->getHRTLs();
            HLCall* call = (HLCall*)prtls->back();
            assert(call->getKind() == CALL_HRTL);
            ADDRESS dest = call->getFixedDest();
            if (dest != NO_ADDRESS)
                ost << "0x" << hex << ")" << dest;
            else
                ost << hex << "computed dest)";
            error(str(ost));
            // Just have to give up on this use of flags
#if DEBUG_ANALYSIS
            findDefsCallDepth--;
#endif
        return;
        }
        pRtls = pBB->getHRTLs(); // repeat the search using the parent BB
        rrit = pRtls->rbegin();
    }

    if (defFound) {
        // Call the action function. This is either matchJScond (for
        // HLJconds or HLSconds) or matchAssign (for assignments that use
        // individual flags)
        (*f)(pOrigRtls, pOrigBB, rrit, proc, rit, kd);
#if DEBUG_ANALYSIS
    findDefsCallDepth--;
#endif
        return;
    }

#if DEBUG_ANALYSIS
    cout << "findDefs(" << findDefsCallDepth
      << "): Multiple in-edges to BB 0x" << hex << (unsigned int)pBB
      << " at " << hex << pBB->getHiAddr() << ": ";
    for (unsigned z=0; z < pBB->getInEdges().size(); z++)
        cout << pBB->getInEdges()[z]->getLowAddr() << " ";
    cout << endl;
#endif
    // Replicate the BB using the flags in series with the in-edges.
    // Note: although CALLs and COMPCALLs look like fall-through BBs, they
    // are very likely to change the flags, and so (for now) are flagged as
    // errors. One-way jump in-edges can be handled by inserting the BB
    // using the flags before the final jump of that BB. Example:
    //
    // cmp1     cmp2    cmp3
    // jmp x    be x    fall thru to x
    //          f:
    //          ...
    //
    // x: a:=b      ! Does not affect flags
    // jcond y      ! Use of flags
    // z:
    //
    // becomes:
    //
    // cmp1         cmp2        cmp3 // Still falls through to x
    // a:=b         be new      x: a:=b
    // jcond y      f:          jcond y
    // jmp z        ...         z:
    //              jmp z
    //              new: a:=b
    //              jcond y
    //              jmp z
    //
    // Then the standard algorithm can be used to fix both flags uses, with
    // the (possibly) different compare expressions. Check each in edge of
    // the current BB (at the top of the chain of BBs with only 1 in-edge)
    // Note well: this use of vit is INVALID! When the in-edges are reallocated,
    // vit becomes invalid
    vector<PBB>::iterator vit = pBB->getInEdges().begin();
    if (vit == pBB->getInEdges().end()) {
        ostrstream os;
        os << "flag use at " << hex << pOrigBB->getHiAddr();
        os << " has no RTL defining the flags; current BB: ";
        pBB->print(os);
        error(str(os));
        // Continue with next BB this Cfg
#if DEBUG_ANALYSIS
    findDefsCallDepth--;
#endif
        return;
    }

    while (vit != pBB->getInEdges().end()) {
        if (pBB->getInEdges().size() == 1) {
        // There is only one in-edge left, so we are done
        break;
    }
        // The parent may define the flags, or one of its parents may do it
        PBB pParBB = *vit;
        if ((pParBB->getType() == CALL) || (pParBB->getType() == COMPCALL)) {
            // No error message here; we will issue one when we try to match
            // this use and def.
            vit++;              // Ignore this in-edge
        cout << "[!]";
            continue;
        }
        HRTLList* pCurRtls = pBB->getHRTLs();
        // It is possible to have more than one "fall through" BB parent
        // when you start duplicating BBs (and some of those were fall
        // throughs). So fall through BBs are treated exactly like one-way
        // BBs now
        if ((pParBB->getType() == ONEWAY) || (pParBB->getType() == FALL)) {
            // As per the cmp1 case above, we need to replicate the BB using
            // the flag here, keeping the in and out edges correct.
            // Remove the branch RTL altogether (if present), as it points to
            // the wrong place, and will be superceeded by the added RTLs
            HRTLList* pParRtls = pParBB->getHRTLs();
            // Check to see if it's a branch. Not all one-way BBs will have
            // a branch as the last RTL!
            HRTLList_IT lastIt = --pParRtls->end();
            if ((*lastIt)->getKind() == JUMP_HRTL) {
                pParRtls->erase(lastIt, pParRtls->end());
            }
            // Copy the out edges of the current BB to the parent BB
            const vector<PBB>& destOuts = pBB->getOutEdges();
            for (unsigned i=0; i < destOuts.size(); i++) {
                PBB destBB = pBB->getOutEdges()[i];
                if (i == 0) {
                    pParBB->setOutEdge(0, destBB);
                } else {
                    // This function will make sure that a jump is generated,
                    // since this is a new out-edge
                    cfg->addNewOutEdge(pParBB, destBB);
                }
                destBB->addInEdge(pParBB);
            }
            // Now it's the same type of BB, with the same number of out edges,
            // as the current BB
            pParBB->updateType(pBB->getType(), destOuts.size());
            // Now insert a copy of the current BB at the end
            // Note: a deep copy is required!
            HRTLList::iterator it;
            for (it = pCurRtls->begin(); it != pCurRtls->end(); it++) {
                pParBB->getHRTLs()->push_back((*it)->clone());
            }
            // Note that the original BB stays where it is, and code is
            // generated for it (we perform this loop only until there is
            // one in-edge left). If the BB that we copy is a fall through
            // or two-way type (with a fall through edge), we must force
            // a branch at the end of the original BB, because the new BB
            // may have the same "address" as the original BB, and so
            // it can be emitted after the original BB, thus causing the
            // fallthrough address to be wrong. Subtle!
            // Example: getarmyrn_pot() inlined into fixgralive() in 099.go
            // Spec benchmark (note: depends on how ties are handled in the
            // sort as to whether the problem actually shows up)
            if ((pBB->getType() == FALL) || (pBB->getType() == TWOWAY)) {
                pBB->setJumpReqd();
                if (pBB->getType() == FALL)
                    cfg->setLabel(pBB->getOutEdges()[0]);
                else
                    // Two way; first out edge is "true" edge, second is fall
                    cfg->setLabel(pBB->getOutEdges()[1]);
            }

#if DEBUG_ANALYSIS
        cout << "\nfindDefs(" << findDefsCallDepth
         << "): copied 1W BB: " << hex;
        for (it = pParBB->getHRTLs()->begin();
         it != pParBB->getHRTLs()->end(); it++) {
        cout << *it << " ";
        }
        cout << endl;
        pParBB->print();
        cout << "Current BB: ";
        for (it = pCurRtls->begin(); it != pCurRtls->end(); it++) {
        cout << *it << " ";
        }
        cout << endl << flush;
        pBB->print();
#endif

            // Remove this in-edge
            pBB->deleteInEdge(vit);         // Also increments vit
            // Now analyse this new BB. It's now a different BB, and the def
            // and use will be in the same BB
//          findDefs(pParBB, pParBB->getHRTLs(), cfg, proc, kd,
//              --pParBB->getHRTLs()->end(), f);
            // The RTL using the flags is not necessarily the last RTL (e.g.
            // it could be an addx 0,%g0,%i0). So call checkBBflags to find
            // it and call findDefs with all the right arguments
            checkBBflags(pBB, proc);
            // Also check the BB that you have just copied, since it may also
            // have a use of the flags
            checkBBflags(pParBB, proc);
            continue;
        } else if (pParBB->getType() == TWOWAY) {
            // As per cmp2 in the example above, we need to change the
            // branch to point to a new BB, which may fall through to the
            // successor of the original BB
            // First create the new BB
            HRTLList* pNewRtls = new HRTLList;
            // Must do a deep copy, else will have problems deleting, and
            // also, setting the addresses of one RTLL to zero will affect
            // the other
            HRTLList::iterator src;
            for (src=pCurRtls->begin(); src!=pCurRtls->end(); src++) {
                pNewRtls->push_back((*src)->clone());
            }
            // Kill the addresses of the first two RTLs. Otherwise, the CFG
            // object will think that this BB is already created
            HRTLList::iterator it = pNewRtls->begin();
            (*it++)->updateAddress(0);
            if (it != pNewRtls->end()) {
                (*it)->updateAddress(0);
            }

#if DEBUG_ANALYSIS
            cout << "\nfindDefs(" << findDefsCallDepth << "): copied 2W BB: "
              << hex;
            for (it=pNewRtls->begin(); it!=pNewRtls->end(); it++) {
                cout << *it << " ";
            }
            cout << endl;
            pParBB->print();
            cout << "Current BB: ";
            for (it = pCurRtls->begin(); it!=pCurRtls->end(); it++) {
                cout << *it << " ";
            }
            cout << endl << flush;
            pBB->print();
#endif

            const vector<PBB>& destOuts = pBB->getOutEdges();
            // Create the new BB. It will have the same type and number of out
            // edges that the current BB has
            PBB pNewBB = cfg->newBB(pNewRtls, pBB->getType(), destOuts.size());
            // Copy the out edges of the current BB to the new BB
            for (unsigned i=0; i < destOuts.size(); i++) {
                cfg->addOutEdge(pNewBB, pBB->getOutEdges()[i]);
            }
            // Make sure this last one will be implemented as a branch
            pNewBB->setJumpReqd();
            cfg->setLabel(pNewBB);      // MVE: Is this correct?
            // Also add a label at the fallthrough address, if any
            if (destOuts.size() == 2) {
                cfg->setLabel(pBB->getOutEdges()[1]);
            }
            // Now change the parent BB to point to the new BB
            // Note: it could be the true (0) or false (1) edge that leads to
            // the current BB
            if (pParBB->getOutEdges()[0] == pBB) {
                pParBB->setOutEdge(0, pNewBB);
            } else {
                // Must be the false edge
                pParBB->setOutEdge(1, pNewBB);
            }
            // Add an in-edge to the new BB
            pNewBB->addInEdge(pParBB);
            // Remove this in-edge from the current BB
            pBB->deleteInEdge(vit);     // Increments vit as well
            // Now analyse this new BB. Again, the use could be anywhere in
            // the BB, or there may not be a use at all if this is a neutral BB
            checkBBflags(pBB, proc);
//          findDefs(pNewBB, pNewRtls, cfg, proc, kd,
//              --pNewBB->getHRTLs()->end(), f);
            // Also check the new BB, since it also may have a use of the flags
            checkBBflags(pNewBB, proc);
            continue;
        } else {
            ostrstream os;
            os << "findDefs: unexpected parent BB type "
               << pParBB->getType() << " found:\n";
            pParBB->print(os);
            error(str(os));
            assert(0);
        }
        vit++;
    }
    // Recurse to handle the original jcond, which by now has only one in-edge
    findDefs(pOrigBB, pOrigRtls, cfg, proc, kd, flt, rit, f);
#if DEBUG_ANALYSIS
    findDefsCallDepth--;
#endif
}

/*==============================================================================
 * FUNCTION:        matchJScond
 * OVERVIEW:        Process a (define CC) and (HLJcond or HLScond) pair
 *                  This procedure basically makes the decision of whether to
 *                  treat the pair as add-like or sub-like
 * TERMS:           Add-like: r = a+b => if (r op 0) then ...
 *                  Sub-like: r = a-b => if (a op b) then ...
 *                    Makes no sense to combine unsigned uses of the CCs (e.g.
 *                    branch if higher or same) with add-like setting of the
 *                    condition codes
 * ASSUMPTIONS:     Assumes that sutract-like flag call functions will have
 *                    names starting with "SUB", or containing the string
 *                    "FFLAG"
 * PARAMETERS:      pOrigRtls: pointer to list of RTLs for BB containing the
 *                    instr using the CCs
 *                  rrit: reverse iterator to the instr defining the CCs
 *                  proc: pointer to the current Proc object
 *                  rit: iterator to instr using the CCs
 *                  kd: kind of the RTL using the CCs, either JCOND_HRTL or
 *                    SCOND_HRTL (as an integer)
 * RETURNS:         <nothing>
 *============================================================================*/
void matchJScond(HRTLList* pOrigRtls, PBB pOrigBB, HRTLList_RIT rrit, UserProc* proc,
    HRTLList_IT rit, int kd)
{
#if DEBUG_ANALYSIS
    cout << "CC used by j/scond near " << hex << pOrigBB->getHiAddr() <<
    " defined at " << (*rrit)->getAddress() << endl;
#endif

    // Get the last RT of the defining instr
    RTFlagCall* pRT = (RTFlagCall*)
        (*rrit)->elementAt((*rrit)->getNumRT()-1);
    if (pRT->getKind() == RTASSGN) {
        // In this case, a flag has been defined by an assignment.
        // The conditional expression is just the LHS of this assignment
        // Example: bit 5,eax // jc dest =>
        // CF = ((eax >> 5) & 1); if (CF) goto dest;
        RTAssgn* pRTA = (RTAssgn*)pRT;
        HLJcond* jc = (HLJcond*)*rit;
        HLScond* sc = (HLScond*)*rit;
        // Make a copy of the LHS
        SemStr* cond = new SemStr(*pRTA->getLHS());
        if (kd == JCOND_HRTL)
            jc->setCondExpr(cond);
        else
            sc->setCondExpr(cond);
        return;
    }

    // Otherwise, pRT is an RTFlagCall
    if ((pRT->func_name.substr(0, 3) == "SUB") ||
        // Floating point compares are always "subtract like"
        (pRT->func_name.find("FFLAG") != string::npos))
            processSubFlags(*rit, rrit, proc, (HRTL_KIND)kd);
    else
        // Else name starts with ADD, or we have various possibilities like
        // MULT and LOGIC, which all act in an add-like form
        processAddFlags(*rit, rrit, proc, (HRTL_KIND)kd);
}

/*==============================================================================
 * FUNCTION:        anyFlagsUsed
 * OVERVIEW:        Return nonzero if any of the "big 4" flags are used in this
 *                  semantic string
 * PARAMETERS:      s: pointer to semantic string being checked
 * RETURNS:         If no flags used, returns 0. Otherwise, returns the id
 *                  (e.g. idCF) of the first flag found
 *============================================================================*/
int anyFlagsUsed(const SemStr* s)
{
    SemStr res;
    SemStr srch;
    srch.push(idCF);
    if (s->search(srch, res)) {
#if DEBUG_ANALYSIS
    cout << "Use of carry flag: res is "; res.print(); cout << "\n";
#endif
        return idCF;
    }
    srch.pop(); srch.push(idZF);
    if (s->search(srch, res)) {
#if DEBUG_ANALYSIS
    cout << "Use of zero flag: res is "; res.print(); cout << "\n";
#endif
        return idZF;
    }
    srch.pop(); srch.push(idNF);
    if (s->search(srch, res)) return idNF;
    srch.pop(); srch.push(idOF);
    if (s->search(srch, res)) return idOF;
    return false;
}


/*==============================================================================
 * FUNCTION:        processSubFlags
 * OVERVIEW:        Matches a j/scond with an RT defining the flags with a
 *                  SUBFLAGS call.
 * ASSUMPTIONS:     Assumes that the second last RT defines the result of the
 *                    subtract-like operation (for cases HLJCOND_JMI and JPOS),
 *                    or (all other cases) that the last RT is a flag call, and
 *                    the first two operands of this function are the operands
 *                    being subtracted. (This includes SETFFLAGS as well as
 *                    SUBFLAGS)
 * PARAMETERS:      rtl: pointer to the HRTL with the instr using the CCs
 *                  rrit: iterator to the CC defining RTL
 *                  proc: pointer to the current Proc object
 *                  kd: kind of the RTL using the CCs, either JCOND_HRTL or
 *                    SCOND_HRTL
 * RETURNS:         <nothing>
 *============================================================================*/
void processSubFlags(HRTL* rtl, list<HRTL*>::reverse_iterator rrit,
    UserProc* proc, HRTL_KIND kd)
{
    HLJcond* jc = (HLJcond*)rtl;
    HLScond* sc = (HLScond*)rtl;
    JCOND_TYPE jt;
    if (kd == JCOND_HRTL)
        jt = jc->getCond();
    else
        jt = sc->getCond();
    int size = (*rrit)->getSize();
    SemStr* pHL = new SemStr(size);
    switch (jt) {
        case HLJCOND_JMI:
        case HLJCOND_JPOS: {
            // These branches are special in that they always refer to the
            // result of the subtract, rather than a comparison of the
            // operands.
            // The second last RT should always be the one that defines the
            // result
            // Result is (lhs < 0) or (lhs >= 0) etc
            switch (jt) {
                case HLJCOND_JMI:   pHL->push(idLess); break;
                case HLJCOND_JPOS:  pHL->push(idGtrEq); break;
                default: /* Can't reach here */ break;
            }
            RTAssgn* pRtDef = (RTAssgn*)
                (*rrit)->elementAt((*rrit)->getNumRT()-2);

            // Assign the destination to a new temp var; choose the same type
            // as the defining RTL
            SemStr* pLHS = proc->newLocal(pRtDef->getType());

            // Duplicate the left hand side of the defining RT, so this RT
            // can be deleted (else, this SemStr will be part of two RTs, and
            // hence will be deleted twice)
            SemStr* pLeft = new SemStr(*pRtDef->getLHS());
            (*rrit)->insertAssign(pLHS, pLeft, false);
            pHL->pushSS(pLHS);
            pHL->push(idIntConst);
            pHL->push(0);
            break;
        }
        default: {
            // Otherwise, subtracts (even decrements) look like compares,
            // as far as the HL result is concerned. For example, after
            // r[8] = r[8] - 1 / jle dest, we want if (r[8] <= 1) goto dest
            // except that we want the original values of the operands. So
            // we use the parameters sent to the SUBFLAGS. They can readily
            // get changed between the def and the use, so we keep a copy of
            // the operands in two new vars
            switch (jt) {
                case HLJCOND_JE:    pHL->push(idEquals); break;
                case HLJCOND_JNE:   pHL->push(idNotEqual); break;
                case HLJCOND_JSL:   pHL->push(idLess); break;
                case HLJCOND_JSLE:  pHL->push(idLessEq); break;
                case HLJCOND_JSGE:  pHL->push(idGtrEq); break;
                case HLJCOND_JSG:   pHL->push(idGtr); break;
                case HLJCOND_JUL:   pHL->push(idLessUns); break;
                case HLJCOND_JULE:  pHL->push(idLessEqUns); break;
                case HLJCOND_JUGE:  pHL->push(idGtrEqUns); break;
                case HLJCOND_JUG:   pHL->push(idGtrUns); break;
                default: assert(0);
            }
            RTFlagCall* pRtf =
                (RTFlagCall*)(*rrit)->elementAt((*rrit)->getNumRT()-1);
            list<SemStr*>::iterator it = pRtf->actuals.begin();

            // Get a type for the high level compare from the size of the
            // first operand
            const Type& type = (*it)->getType();
            pHL->setType(type);

            // Assign the operands to the temp vars
            SemStr* pLHS1 = proc->newLocal(type);
            (*rrit)->insertAfterTemps(pLHS1, new SemStr(**it));
            SemStr* pLHS2 = proc->newLocal(type);
            (*rrit)->insertAfterTemps(pLHS2, new SemStr(**++it));
            pHL->pushSS(pLHS1);
            pHL->pushSS(pLHS2);
            break;
        }
    }   // end switch
    if (kd == JCOND_HRTL)
        jc->setCondExpr(pHL);
    else
        sc->setCondExpr(pHL);
}

/*==============================================================================
 * FUNCTION:        processAddFlags
 * OVERVIEW:        Matches a j/scond with an RT defining the flags with an
 *                  ADDFLAGS call.
 * ASSUMPTIONS:     Assumes that the second last RT is an assignment, and the
 *                    left hand side of this assignment is the result of the add
 * PARAMETERS:      jc: pointer to the HLJcond with the branch
 *                  rrit: iterator to the CC defining RTL
 *                  proc: pointer to the current Proc object
 *                  kd: kind of the RTL using the CCs, either JCOND_HRTL or
 *                    SCOND_HRTL
 * RETURNS:         <nothing>
 *============================================================================*/
void processAddFlags(HRTL* rtl, list<HRTL*>::reverse_iterator rrit,
    UserProc* proc, HRTL_KIND kd)
{
    HLJcond* jc = (HLJcond*)rtl;
    HLScond* sc = (HLScond*)rtl;
    JCOND_TYPE jt;
    if (kd == JCOND_HRTL)
        jt = jc->getCond();
    else
        jt = sc->getCond();
    // Result is (lhs op 0)
    RTAssgn* pRtDef = (RTAssgn*)
        (*rrit)->elementAt((*rrit)->getNumRT()-2);
    const Type& type = pRtDef->getType();
    SemStr* pHL = new SemStr(type);
    switch (jt) {
        case HLJCOND_JE:    pHL->push(idEquals); break;
        case HLJCOND_JNE:   pHL->push(idNotEqual); break;
        case HLJCOND_JSL:   // Can be valid after a test instruction
        case HLJCOND_JMI:   pHL->push(idLess); break;
        case HLJCOND_JSGE:  // Can be valid after a test instruction
        case HLJCOND_JPOS:  pHL->push(idGtrEq); break;
        // Next two can be valid after a test instr
        case HLJCOND_JSLE:  pHL->push(idLessEq); break;
        case HLJCOND_JSG:   pHL->push(idGtr); break;
        default:
            ostrstream ost;
            ost << "ADD flags coupled with jcond kind " << (int)jt;
            error(str(ost));
            return;
    }
    // Allocte a new temp var for the result. Use an appropriate type, so the
    // temp var will have the right size
    // Assign the destination to the new var
    SemStr* pLHS = proc->newLocal(pRtDef->getType());
    SemStr* pLeft = new SemStr(*pRtDef->getLHS());
    (*rrit)->insertAssign(pLHS, pLeft, false);
    pHL->pushSS(pLHS);
    pHL->push(idIntConst);
    pHL->push(0);
    if (kd == JCOND_HRTL)
        jc->setCondExpr(pHL);
    else
        sc->setCondExpr(pHL);
}

/*==============================================================================
 * FUNCTION:        analyse_assign
 * OVERVIEW:        Process a (define CC) and (assign that uses CC) pair
 *                  For example, cmp 0,a / d = 0-0-CF is handled by appending
 *                  an RT to the compare: CF = (a != 0);
 * PARAMETERS:      pOrigRtls: pointer to list of RTLs for BB containing the
 *                    instr using the CCs
 *                  rrit: reverse iterator to the instr defining the CCs
 *                  proc: pointer to the current Proc object
 *                  rit: iterator to instr using the CCs
 *                  kd: kind of the RTL using the CCs, either JCOND_HRTL or
 *                    SCOND_HRTL
 * RETURNS:         <nothing>
 *============================================================================*/
void matchAssign(HRTLList* pOrigRtls, PBB pOrigBB, HRTLList_RIT rrit, UserProc* proc,
    HRTLList_IT rit, int flagUsed)
{
#if DEBUG_ANALYSIS
    cout << "Use of CC in this assignment: ";
    (*rit)->print();
    cout << endl;
    cout << "CC set at "; (*rrit)->print();
    cout <<  endl;
#endif
    if (flagUsed == idCF) {
        int n = (*rrit)->getNumRT();
        RTFlagCall* pRT = (RTFlagCall*)(*rrit)->elementAt(n-1);
        if ((pRT->getKind() == RTFLAGCALL) &&
            (pRT->func_name.substr(0, 3) == "SUB")) {
            // Get the assignment/comparison. Should be the last assignment,
            // which should be the second last RT (excluding added RTs).
            RTAssgn* pRT;
            int i=n-2;
            do {
                pRT = (RTAssgn*)(*rrit)->elementAt(i);
                i--;
            } while ((i >= 0) && (pRT->added));
            assert(pRT->getKind() == RTASSGN);
            SemStr* pRHS = pRT->getRHS();
            // Idiom 1: Compare 0 to X (or subtract X from 0), then use the
            // carry flag.
            // Transform to: CF = (X != 0);
            // Note: this will look like r[0] = -r[11] after simplification
            if (pRHS->getFirstIdx() == idNeg) {
                int size = pRT->getSize();
                SemStr* pLHS = new SemStr;
                pLHS->push(idCF);
                SemStr* notEq0 = pRHS->getSubExpr(0);
                notEq0->prep(idNotEqual);
                notEq0->push(idIntConst);
                notEq0->push(0);
                // Note, we must prepend the assignment to the CF, because it
                // depends only on the value of the operands before the
                // subtract, and one operands could be modified by the subtract
                // Hence we pass "true" as parameter 4
//              (*rrit)->insertAssign(pLHS, notEq0, true, size);
                (*rrit)->insertAfterTemps(pLHS, notEq0, size);
#if DEBUG_ANALYSIS
    cout << "Result: "; (*rrit)->print(); cout << endl;
#endif
            }
            else if (pRHS->getFirstIdx() == idIntConst) {
                // Very rare case, e.g. fixgralive in 099.go:
                // 30d88:  90 a0 20 03        subcc        %g0, 3, %o0
                // 30d8c:  84 62 60 00        subx         %o1, 0, %g2
                // First rtl is v0 := -3
                // Add Cf := (3 != 0)!!
                int size = pRT->getSize();
                SemStr* pLHS = new SemStr;
                pLHS->push(idCF);
                SemStr* notEq0 = new SemStr;
                notEq0->push(idNotEqual);
                notEq0->push(idIntConst);
                notEq0->push(-pRHS->getSecondIdx());
                notEq0->push(idIntConst);
                notEq0->push(0);
                // As per above, we must prepend the assignment to CF
//              (*rrit)->insertAssign(pLHS, notEq0, true, size);
                (*rrit)->insertAfterTemps(pLHS, notEq0, size);
#if DEBUG_ANALYSIS
    cout << "Result: "; (*rrit)->print(); cout << endl;
#endif
            }
            else {
                // Idiom 2: Compare X to Y (or subtract Y from X), then use the
                // carry flag.
                // Transform to: CF = ((unsigned)X < (unsigned)Y);
                assert(pRHS->getFirstIdx() == idMinus);
                int size = pRT->getSize();
                SemStr* pLHS = new SemStr;
                pLHS->push(idCF);
                SemStr* xLTy = pRHS->getSubExpr(0);
                xLTy->prep(idLessUns);
                SemStr* y = pRHS->getSubExpr(1);
                xLTy->pushSS(y);
                delete y;
                // As per above, we must prepend the assignment to CF
//              (*rrit)->insertAssign(pLHS, xLTy, true, size);
                (*rrit)->insertAfterTemps(pLHS, xLTy, size);
#if DEBUG_ANALYSIS
    cout << "Result: "; (*rrit)->print(); cout << endl;
#endif
            }
            return;
        }
        else if ((pRT->getKind() == RTFLAGCALL) &&
          (pRT->func_name.substr(0, 3) == "ADD")) {
            ostrstream ost;
            ost << "CC definition at " << hex << (*rrit)->getAddress();
            ost << " is an ADD class instruction. "
            "64 bit arithmetic not supported yet";
            warning (str(ost));
        }
    }
    ostrstream ost;
    ost << "CC use at " << hex << (*rit)->getAddress() << " not paired";
    error (str(ost));
}

/*==============================================================================
 * FUNCTION:        copySwap4
 * OVERVIEW:        Load a 4 byte word from little to big endianness,
 *                  or vice versa
 *                  Similar to _swap4 macro, but this for translate time
 * NOTE:            Processes char at a time, so avoids alignment hassles
 * PARAMETERS:      w: Pointer to source word
 * RETURNS:         The source word as an integer
 *============================================================================*/
int copySwap4(int* w)
{
    char* p = (char*)w;
    int ret;
    char* q = (char*)(&ret+1);
    *--q = *p++; *--q = *p++; *--q = *p++; *--q = *p;
    return ret;
}

/*==============================================================================
 * FUNCTION:        copySwap2
 * OVERVIEW:        Load a 2 byte short word from little to big endianness,
 *                  or vice versa
 *                  Similar to _swap2 macro, but this for translate time
 * NOTE:            Processes char at a time, so avoids alignment hassles
 * PARAMETERS:      w: Pointer to source short word
 * RETURNS:         The source short word as an integer
 *============================================================================*/
int copySwap2(short* h)
{
    char* p = (char*)h;
    short ret;
    char* q = (char*)(&ret);
    *++q = *p++; *--q = *p;
    return (int)ret;
}

#if USE_PROCESS_CONST
/*==============================================================================
 * FUNCTION:      processConst
 * OVERVIEW:      Replace the RHS of the current assignment with a floating
 *                  point constant (or integer constant in rare cases)
 * NOTE:          Assumes that the floating point constant binary format
 *                  is compatible with the compiler used to compile this code
 * PARAMETERS:    addr: native address of the constant
 *                memExp: reference to a SemStr which is the m[] to be replaced
 *                  with the constant
 *                RHS: pointer to the RHS to be changed
 * RETURNS:       <nothing>; note that the RHS SemStr gets changed
 *============================================================================*/
void processConst(ADDRESS addr, SemStr& memExp, SemStr* RHS)
{
    const void* p;
    const char* last;
    int delta;
    union {
        double d;
        int i[2];
    } value;
    p = prog.getCodeInfo(addr, last, delta);
    if (p == NULL) return;      // No code section pointer there
    int size = memExp.getType().getSize();

    // SemStr's are deemed to be same endianness as the source machine:
    // i.e., the constant is in src endianness.
    // Let "uqbt endianness" = endianness of machine UQBT is running on now.
    // We must swap the constant during translation if 
    //     uqbt endianness != src endianness
    // since then we must swap any constant read during translation.
    bool doSwaps;
    #if ((WORDS_BIGENDIAN == 1) && (SRCENDIAN == LITTLEE)) || ((WORDS_BIGENDIAN == 0) && (SRCENDIAN == BIGE))
        doSwaps = true;
    #else
        doSwaps = false;
    #endif
        
    SemStr* con = new SemStr;
    if (memExp.getType().getType() == FLOATP) {
        if (doSwaps) {
            if (size == sizeof(float)*8) {
                int i = copySwap4((int*)p);
                value.d = (double)*(float*)&i;
            } else if (size == sizeof(double)*8) {
                int ii[2];
                ii[1] = copySwap4((int*)p);
                ii[0] = copySwap4(((int*)p)+1);
                value.d = *(double*)ii;
            }
        } else {
            if (size == sizeof(float)*8)
                value.d = (double)*(float*)p;
            else if (size == sizeof(double)*8)
                value.d = *(double*)p;
        }
        con->push(idFltConst);
        con->push(value.i[0]);
        con->push(value.i[1]);
    } else {
        // The constant is integer.
        int k;
        if (doSwaps) {
            if (size == sizeof(int)*8) {
                k = copySwap4((int*)p);
            } else if (size == sizeof(short)*8) {
                k = copySwap2((short*)p);
            } else
                assert(0);
        } else {
            if (size == sizeof(int)*8)
                // Note: there could be alignment issues!
                k = *(int*)p;
            else if (size == sizeof(short)*8)
                k = *(short*)p;
            else
                assert(0);
        }
        con->push(idIntConst);
        con->push(k);
    }

    // Replace the right hand side with the constant
    // MVE: Does this have to be type sensitive sometimes? Doubtful.
    RHS->searchReplaceAll(memExp, *con);
    delete con;
}

/*==============================================================================
 * FUNCTION:        isRegister
 * OVERVIEW:        Return true if the given expression is a register, var, or
 *                      a temp. Only constant register numbers are considered
 * PARAMETERS:      exp: pointer to the expression to be checked
 * RETURNS:         True if matched
 *============================================================================*/
bool isRegister(const SemStr* exp)
{
    if (exp->getFirstIdx() == idRegOf)
        return exp->getSecondIdx() == idIntConst;
    if (exp->getFirstIdx() == idVar)
        return true;
    if (exp->getFirstIdx() == idTemp)
        return true;
    return false;
}

/*==============================================================================
 * FUNCTION:        isConstTerm
 * OVERVIEW:        Return true if the given expression is a constant or a
 *                      register that is already known to be const
 * PARAMETERS:      exp: pointer to the expression to be checked
 *                  constMap: reference to the map of existing constant regs
 *                  value: reference to an ADDRESS that will be set to the
 *                    value of the constant
 * RETURNS:         True if matched, and value is set
 *============================================================================*/
bool isConstTerm(const SemStr* exp, const KMAP& constMap, ADDRESS& value)
{
    if (exp->getFirstIdx() == idIntConst) {
        value = exp->getSecondIdx();
        return true;
    }
    if (isRegister(exp)) {
        KMAP::const_iterator it;
        it = constMap.find(*exp);
        if (it == constMap.end())
            // Not a constant register
            return false;
        // A constant register
        value = it->second;
        return true;
    }
    // Not a simple constant or a register
    return false;
}

/*==============================================================================
 * FUNCTION:        isConst
 * OVERVIEW:        Return true if the given expression is constant
 * PARAMETERS:      exp: pointer to the expression to be checked
 *                    constMap: reference to the map of existing constant regs
 *                    value: reference to an ADDRESS that will be set to the value
 *                      of the constant
 * RETURNS:         True if matched, and value is set
 *============================================================================*/
bool isConst(const SemStr* exp, const KMAP& constMap, ADDRESS& value)
{
    if (isConstTerm(exp, constMap, value))
        return true;
    int idx = exp->getFirstIdx();
    switch (idx) {
        case idPlus:
        case idMinus:
        case idBitOr:
        {
            // Expect op k1 k2 where k1 and k2 are constants    
            SemStr* k1; ADDRESS value1;
            k1 = exp->getSubExpr(0);
            if (!isConstTerm(k1, constMap, value1)) {
                delete k1;
                return false;
            }
            SemStr* k2; ADDRESS value2;
            k2 = exp->getSubExpr(1);
            if (!isConstTerm(k2, constMap, value2)) {
                delete k2;
                return false;
            }
            delete k1; delete k2;
            switch (idx) {
                case idPlus:  value = value1 + value2; break;
                case idMinus: value = value1 - value2; break;
                case idBitOr: value = value1 | value2; break;
            }
            return true;
        }
    }
    return false;
}

/*==============================================================================
 * FUNCTION:        checkBBconst
 * OVERVIEW:        Check the current BB for the presence of constants; create
 *                  pointers to functions or floating point constants if needed
 * NOTE:            This is a temporary solution, which does not attempt to
 *                  do data flow analysis, and only checks the current BB
 * PARAMETERS:      pBB: Pointer to the BB to be checked
 *                  proc: Ptr to UserProc object for the current procedure
 * RETURNS:         <nothing>
 *============================================================================*/
void checkBBconst(PBB pBB)
{
    HRTLList* pRtls = pBB->getHRTLs();
    if (pRtls == 0)
        return;
    // The following is a map from SemStr* (representing the register, var, or
    // temp being assigned a constant, to ADDRESS (representing the constant
    // that is held there). This is needed because constant addresses are some-
    // time arrived at by operations like + or | on other constants.
    // For example, see the ninths test program (Sparc)
    KMAP constMap;

    HRTLList_IT rit;
    for (rit = pRtls->begin(); rit != pRtls->end(); rit++) {
        int n = (*rit)->getNumRT();
        for (int i=0; i < n; i++) {
            RTAssgn* pRT = (RTAssgn*)(*rit)->elementAt(i);
            if (pRT->getKind() != RTASSGN) continue;
            // Locate any register loads where the RHS is constant. The constant
            // could be a simple constant, or a register that is already known
            // to be constant, or simple operation (+, -, |) on two of these
            SemStr* LHS = pRT->getLHS();
            SemStr* RHS = pRT->getRHS();
            ADDRESS value;
            if (isRegister(LHS)) {
                if (isConst(RHS, constMap, value))
                    constMap[*LHS] = value;
                else
                    // If the LHS is already in the map, delete it, because it's
                    // no longer constant
                    constMap.erase(*LHS);
            }

            // Locate any memory loads where the destination is a register.
            // We need to check more than just floating point registers, because
            // sometimes a double is loaded into two integer registers. In this
            // last case, the constants are treated as two integers.
            // The memory reference is not necessarily at the "top level", e.g.
            // r[32] := r[32] *f fsize(64, 80, m[const]);
            list<SemStr*> result;
            if (!RHS->searchAll(memX, result))
                // No memOfs; ignore
                continue; 
            // We have at leat one memOf; go through the list
            list<SemStr*>::iterator it;
            for (it=result.begin(); it != result.end(); it++) {
                SemStr* address;
                address = (*it)->getSubExpr(0);
                if (isConst(address, constMap, value))
                    // This is what we are waiting for: assignment from a
                    // constant memory address. value is the const address
                    processConst(value, **it, RHS);
                delete address;
            }
        }
    }
}
#endif // USE_PROCESS_CONST

/*==============================================================================
 * FUNCTION:        isFlagFloat
 * OVERVIEW:        Return true if the RTFlagCall parameter refers to the
 *                    floating point flags
 * PARAMETERS:      rt: const pointer to the RTFlagCall object
 *                  proc: points to the UserProc for the proc containing rt
 * RETURNS:         True if floating point (see overview)
 *============================================================================*/
bool isFlagFloat(const RTFlagCall* rt, UserProc* proc)
{
    if (rt->actuals.size() == 0) {
    return false;
    }
    // We assume it's FP if the first argument is a floating point register
    SemStr* exp = rt->actuals.front();
    if (exp->getFirstIdx() == idRegOf) {
        if (exp->getSecondIdx() == idIntConst) {
            int regNum = exp->getThirdIdx();
            // Get the register's intrinsic type
            Type rType = prog.RTLDict.DetRegMap[regNum].g_type();
            return (rType.getType() == FLOATP);
        } else if (exp->getSecondIdx() == idTemp) {
            Type rType = Type::getTempType(theSemTable[exp->getThirdIdx()].sName);
            return (rType.getType() == FLOATP);
        }
    }
    // We also assume it's FP if the first argument is an FP variable
    if (exp->getFirstIdx() == idVar) {
    int idx = exp->getSecondIdx();
    Type varType = proc->getVarType(idx);
    return (varType.getType() == FLOATP);
    }       
    return false;
}
