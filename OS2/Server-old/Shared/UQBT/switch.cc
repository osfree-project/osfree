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
 * FILE:       switch.cc
 * OVERVIEW:   This file contains routines to determine whether a register
 *             jump instruction is likely to be part of a switch statement.
 *
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 *  $Revision: 1.54 $
 *  Switch statements are generally one of the following three
 *  forms, when reduced to high level representation, where var is the
 *  switch variable, numl and numu are the lower and upper bounds of that
 *  variable that are handled by the switch, and T is the address of the
 *  jump table. Out represents a label after the switch statement.
 *  Each has this pattern to establish the upper bound:
 *      jcond (var > numu) out
 *  Most have a subtract (or add) to establish the lower bound; some (e.g.
 *  epc) have a compare and branch if lower to establish the lower bound;
 *  if this is the case, the table address needs to be altered
 * Here are the 4 types discovered so far; size of an address is 4:
 *  A) jmp m[<expr> * 4 + T]
 *  O) jmp m[<expr> * 4 + T] + T
 *  H) jmp m[(((<expr> & mask) * 8) + T) + 4]
 *  R) jmp %pc + m[%pc + ((<expr> * 4) + k)]        // O1
 *  r) jmp %pc + m[%pc + ((<expr> * 4) - k)] - k    // O2
 *  where for forms A, O, R, r <expr> is one of
 *      r[v]                            // numl == 0
 *      r[v] - numl                     // usual case, switch var is int
 *      ((r[v] - numl) << 24) >>A 24)   // switch var is char in lower byte
 *      etc
 * or in form H, <expr> is something like
 *      ((r[v] - numl) >> 4) + (r[v] - numl)
 *      ((r[v] - numl) >> 8) + ((r[v] - numl) >> 2) + (r[v] - numl)
 *      etc.
 *  Forms A and H have a table of pointers to code handling the switch
 *  values; forms O and r have a table of offsets from the start of the
 *  table itself (i.e. all values are the same as with form A, except
 *  that T is subtracted from each entry.) Form R has offsets relative
 *  to the CALL statement that defines "%pc".
 *  Form H tables are actually (<value>, <address>) pairs, and there are
 *  potentially more items in the table than the range of the switch var.
 *
 */

/* 30 Sep 98 - Mike
 *  Fixed infinite loop in getPrevRtl()
 * 01 Oct 98 - Mike
 *  Decided to revamp the code, making it more source independent.
 * 09 Oct 98 - Mike
 *  Got the "Form H" code working well.
 * 13 Oct 98 - Mike
 *  Now use the isArraySimilar technique to handle form A and B switches
 *  on chars (which can have ((r[v] << 24) >>A 24) instead of r[v])
 * 13 Oct 98 - Mike
 *  Fixed the problem where sll in delay of bgu prevented switch detect
 * 14 Oct 98 - Mike
 *  Maintain a semantic string for the upper bound, for those cases where
 *  the immediate compare doesn't fit into one instruction
 * 15 Oct 98 - Mike
 *  Replaced the arbitrary upper and lower bounds checking with ssBound
 * 16 Oct 98 - Mike: SubstReg() now takes a SemStr* parameter
 * 02 Nov 98 - Mike: isSwitch() now takes a SemStr* pDest. 
 * 17 Nov 98 - Mike: Added bNegate to getPrevRtl(); code to handle compare
 *  and branch if true to the indexed jump
 * 27 Nov 98 - Mike: experiment with removeSize()
 * 15 Dec 98 - Mike: jmpl/restore handled as computed call, then return
 * 16 Dec 98 - Mike: cfg& replaced by pCfg
 * 06 Jan 99 - Mike: pExp -> pLHS (was confusing)
 * 02 Feb 99 - Mike: changes to cope with HL RTLs
 * 16 Feb 99 - Mike: HLJcond::GetCond() returns a JCOND_TYPE now
 * 17 Feb 99 - Mike: Removed save/restore code; only traverse first in-edge;
 *              terminology form B -> form O.
 * 23 Feb 99 - Mike: Changed r[-2] to r[99] since negative register numbers
 *              already have a special meaning to the substitute functions
 * 03 Mar 99 - Mike: isSwitch() and processSwitch() take an iOffset now
 * 24 Mar 99 - Mike: no longer initialise idPC, idZF, etc
 * 07 Apr 99 - Mike: Mods for HEADERS; getPrevRtl() no longer a member of Cfg
 * 28 Apr 99 - Mike: visit() takes more args now
 * 01 Jul 99 - Mike: Added DEBUG_SWITCH (activate in Makefile)
 * 02 Jul 99 - Mike: Moved many parameters for isSwitch() into SWITCH_INFO in
 *              the HLNwayJump RTL class
 * 31 Aug 99 - Mike: Added 3rd subform of form O:
 *              jmp PC + [PC + (<expr> * w) + k] + k
 * 03 Sep 99 - Mike: repaired form H switches (seems the simplify process
 *              became more powerful, so T+4 no longer seen
 * 07 Sep 99 - Mike: repaired form R (O1) switches; seems the simplify process
 *              treats a + (b + c)  as  (a + b) + c now.
 * 10 Sep 99 - Mike: put the idNot at the front of ssBound, not the back, since
 *              that's where it belongs, and otherwise it's confused with int 38
 * 10 Sep 99 - Mike: Attempted to be smart about choosing in-edge when there
 *              are several - doesn't seem to work as well as choosing the first
 * 21 Sep 99 - Mike: Cope with copies of the register of interest, comparing
 *              the upper bound against the copy
 * 24 Dec 99 - Mike: Comment out RTL for load from table; fixed logic assigning
 *              the switch variable
 * 27 Apr 00 - Mike: Added logic to cope with and statements setting the bounds
 * 10 Jul 00 - Mike: Fixed a bug in looking for the lower bound as an add of
 *              a negative number... was checking rhs, not lhs, for reg of int.
 * 11 Dec 00 - Mike: Fixed assignment of "switch variable" when have an AND
 *              instruction setting the lower bound, and a compare setting upper
 * 05 Mar 01 - Mike: insertAssign() is now a member of class RTlist
 * 09 Mar 01 - Mike: A lot of code was assuming that the first RT of an RTlist
 *              would hold the assignment of interest. With assignments to temps
 *              this is not necessarily true. So to make the code more robust,
 *              there is a "current RT index" (indexRT), and the outer loop
 *              that fetched the next RTL now increments indexRT first, and
 *              only fetches the next RTL when all RTs are examined
 * 09 Mar 01 - Mike: Inserted a test for indexRT being in range; was faulting
 *              on RTLs with no RTs
 * 03 Apr 01 - Mike: When looking for m[] that kills a switch value, include
 *              zero filled and sign extended versions (was only considering
 *              idSize, idSgnEx combinations).
 * 05 Apr 01 - Mike: Set the switch variable at the subtract (for the lower
 *              bound) if there is one, or at the compare if not. This prevents
 *              problems where the switch variable is set too early, and a
 *              label (e.g. from a loop enclosing the switch stmt) comes after
 *              the setting of the switch variable.
 * 09 Apr 01 - Mike: Replaced 2 calls to insertAssign to insertAfterTemps; the
 *              assignment to the switch variable was interfering with calls to
 *              insertAfterTemps in analysis.cc (099.go benchmark, fire_joseki)
 *              Also cleaned up the logic for when done, particularly for when
 *              an AND statement sets the lower bound
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 * 30 Aug 01 - Mike: Proc::newLocal returns a pointer now
 */

//#define DEBUG_SWITCH 1            // Uncomment to debug this module

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "global.h"
#include "ss.h"
#include "rtl.h"
#include "cfg.h"
#include "options.h"
#include "proc.h"                   // For nextVar()
#include "prog.h"                   // For prog.cover
#include "frontend.h"

/*==============================================================================
 * Forward declarartions.
 *============================================================================*/
int getFuncRetReg();                // In cti<processor>.cc
void setSwitchInfo(PBB pSwitchBB, char chForm, int iLower, int iUpper,
    ADDRESS uTable, int iNumTable, int iOffset, HRTLList_IT itDefinesSw,
    UserProc* pProc);

/*==============================================================================
 * File globals.
 *============================================================================*/
static set<PBB> setPathDone;        // Set of PBBs already traversed
static int idUP, idLO;               // Indices for upper and lower bounds

// Arrays for the three forms (five since O has 2 subforms)

// Pattern: m[<expr> * 4 + T ]
static int arrA[] = {idMemOf, idPlus, idMult, -1 /* Whole subexpression */,
    idIntConst, 4, idIntConst, -1};

// Pattern: m[<expr> * 4 + T ] + T
static int arrO[] = {idPlus, idMemOf, idPlus, idMult, -1 /* Whole
    subexpression */, idIntConst, 4, idIntConst, -1, idIntConst, -1};

// Pattern example: m[(((<expr> & 63) * 8) + 12340) + 4]
// Simplifies to m[((<expr> & 63) * 8) + 12344]
static int arrH[] = {idMemOf, idPlus, idMult, idBitAnd,
    -1 /* whole subexpression */, idIntConst, -1, idIntConst, 8,
    idIntConst, -1};

// Pattern: %pc + m[%pc  + (<expr> * 4) + k]
// where k is a small constant, typically 28 or 20
static int arrO1[] = {idPlus, idPC, idMemOf, idPlus, idPlus, idPC, 
    idMult, -1 /* Whole subexpression */, idIntConst, 4, idIntConst, -1};

// Pattern: %pc + m[%pc + ((<expr> * 4) - k)] - k
// where k is a smallish constant, e.g. 288 (/usr/bin/vi 2.6, 0c4233c).
static int arrO2[] = {idMinus, idPlus, idPC, idMemOf, idMinus, idPlus, idPC,
    idMult, -1 /* Whole subexpression */, idIntConst, 4, idIntConst, -1,
    idIntConst, -1};

// A subexpression representing `r[v] - k' needed in two places below
static int arrRegMinus[] = {idMinus, idRegOf, idIntConst, -1, idIntConst, -1};
static int arrRegPlus[] = {idPlus, idRegOf, idIntConst, -1, idIntConst, -1};
static int arrReg999[] = {idRegOf, idIntConst, 999};

// Pattern: r[iReg] + negativeConst
// + r[ int iReg int const
static int arrRegPlusNegConst[] = {idPlus, idRegOf, idIntConst, -1, idIntConst,
    -1};

// Pattern: r[x] & const
// & r[ int x int k
static int arrRegAndConst[] = {idBitAnd, idRegOf, idIntConst, -1, idIntConst,
    -1};

// Address of the last CALL instruction copying the PC to a register.
// Needed for form O2 / "r"
ADDRESS uCopyPC;

/*==============================================================================
 * FUNCTION:      initSwitch
 * OVERVIEW:      Initializes the switch analyzer.
 * PARAMETERS:    <none>
 * RETURNS:       <nothing>
 *============================================================================*/
void initSwitch()
{
    idUP = theSemTable.addItem(cOPERATOR, 0, 0, 2, "GT");
    idLO = theSemTable.addItem(cOPERATOR, 0, 0, 2, "LO");
}

/*==============================================================================
 * FUNCTION:      getPrevRtl
 * OVERVIEW:      Get the preceeding RTL to the current one. If we are at the
 *                top of a BB, choose an in-edge to traverse. If there are more
 *                than one, the static global list of current paths is used to
 *                track down the next
 *                If the path is via out edge 0 of the previous BB (branch
 *                taken),then set bNegate.
 * PARAMETERS:    pCurBB - Pointer to the current BB
 *                itRtl - iterator to the current RTL this BB
 *                bNegate - Set true if the (new) current BB is a two-way BB
 *                  that goes to the previous BB via the "true" outedge. This
 *                  indicates logical negation as far as switch logic goes
 * RETURNS:       true if no more paths to try; also sets bNegate
 *============================================================================*/
bool getPrevRtl(PBB& pCurBB, HRTLList_IT& itRtl, bool& bNegate)
{
    bNegate = false;
    if (itRtl != pCurBB->getHRTLs()->begin()) {
        itRtl--;                    // Just go to the previous RTL this BB
        return false;               // More to do
    }
    // No more RTLs this BB. We need to choose one of the in edges, if present
    if (pCurBB->getInEdges().size() == 0)
        return true;            // No in-edges
    PBB pOldBB = pCurBB;
    int numInEdges = pCurBB->getInEdges().size();
    if (numInEdges == 1) {
        // Just one in-edge. Use that.
        pCurBB = pCurBB->getInEdges()[0];
    }
    else {
        // More than one in-edge.
#if DEBUG_SWITCH
        cout << "Multiple in-edges for BB at " << hex << pCurBB->getLowAddr()
            << endl;
#endif
#if 0
        cout << "Multiple in-edges for BB at " << hex <<
            pCurBB->getLowAddr() << ":\n==\n";
        for (unsigned ii=0; ii < pCurBB->getInEdges().size(); ii++) {
            (pCurBB->getInEdges()[ii])->print();
        }
        cout << "\n==\n";
#endif
#if 0       // This doesn't seem to suit form H switches...
        // Choose the most suitable in-edge. Best is a fall through,
        // followed by a one-way jump, followed by the false leg of a
        // two-way jump. The latter is because occasionally a test is
        // made for a specific case, and that (true) branch is optimised
        // directly to the jump. Example (from Pentium gcc spec benchmark):
        // 080fed8b cmpl   $0x7,%edx    ! Specific case 7
        // 080fed8e je     080fedb7     ! True branch to indexed jump!
        // 080fed90 movl   %esi,%eax
        // ...
        // 080feda6 testb  $0x1,%al
        // 080feda8 jne    080ff144     ! Exit switch
        // 080fedae cmpl   $0x7,%edx    ! Test lower bound
        // 080fedb1 ja     080ff13c     ! Exit if above (important)
        // Fall through if false
        // 080fedb7 jmp    *0x80fedc0(,%edx,4) ! Indexed jump
        
        bool found = false;
        // Try for fallthrough BB
        for (unsigned ii=0; ii < pCurBB->getInEdges().size(); ii++) {
            if (setPathDone.count(pCurBB->getInEdges()[ii]) != 0)
                // Already traversed this path!
                continue;
            if ((pCurBB->getInEdges()[ii])->getType() == FALL) {
                found = true;
                pCurBB = pCurBB->getInEdges()[ii];
                break;
            }
        }
        if (!found) {
            // Try for one way BB
            for (unsigned ii=0; ii < pCurBB->getInEdges().size(); ii++) {
                if (setPathDone.count(pCurBB->getInEdges()[ii]) != 0)
                    // Already traversed this path!
                    continue;
                if ((pCurBB->getInEdges()[ii])->getType() == ONEWAY) {
                    found = true;
                    pCurBB = pCurBB->getInEdges()[ii];
                    break;
                }
            }
        }
        if (!found) {
            // Try for false edge of two way BB
            for (unsigned ii=0; ii < pCurBB->getInEdges().size(); ii++) {
                if (setPathDone.count(pCurBB->getInEdges()[ii]) != 0)
                    // Already traversed this path!
                    continue;
                if ((pCurBB->getInEdges()[ii])->getType() == TWOWAY) {
                    PBB inEdge = pCurBB->getInEdges()[ii];
                    if ((inEdge->getOutEdges())[1] == pCurBB) {
                        found = true;
                        pCurBB = pCurBB->getInEdges()[ii];
                        break;
                    }
                }
            }
        }
        if (!found)
#endif
        {
            // Just pick the first in-edge (no better info)
            pCurBB = pCurBB->getInEdges()[0];
            int edge = 0;
            while ((setPathDone.count(pCurBB) != 0) && (edge+1 < numInEdges))
                // Already considered first path. Use the next
                pCurBB = pCurBB->getInEdges()[++edge];
        }
#if DEBUG_SWITCH
        cout << "Chose in-edge at " << pCurBB->getLowAddr() << endl;
#endif
    }

    // If we have already considered it, we have a loop, so exit
    if (setPathDone.count(pCurBB) != 0)
        // No more paths to try
        return true;        // Was return false; surely wrong
    setPathDone.insert(pCurBB); // Don't consider it again
    // Check if we have come via the first out edge of a TWOWAY BB
    if (pCurBB->getType() == TWOWAY &&
        pCurBB->getOutEdges().size() &&
        pCurBB->getOutEdges()[0] == pOldBB)
        // This is a two way BB, and the first out edge (the
        // taken branch) points to the BB we came from.
        // This indicates a logical negation, for the purposes
        // of switch detection
        bNegate = true;
    // Point to the last RT
    itRtl = pCurBB->getHRTLs()->end(); itRtl--;
    return false;
}


/*==============================================================================
 * FUNCTION:    isSwitch
 * OVERVIEW:    This is the main function for determining switch statements.
 *              Attempt to determine whether this DD instruction is a switch
 *              statement. If so, return true, and set iLower, uUpper to the
 *              switch range, and set uTable to the native address of the
 *              table. If it is form B (the table is an array of offsets from
 *              the start of the table), then chFormB will be 'B', etc.
 *              If it is form H (hash table), then iNumTable will be the
 *              number of entries in the table (not the number of cases,
 *              or max-min).
 * PARAMETERS:  pSwitchBB - pointer to the BB with the register branch at end
 *              pDest - pointer to the SS with the dest of the branch
 *              pProc - pointer to the current Proc object
 * SIDE EFFECT: Sets a SWITCH_INFO struct to the HLNwayJump RTL at the end of
 *              the BB, with info like table address, upper and lower bound,
 *              etc
 * RETURNS:     True if the given instruction is a switch statement
 *                Also creates a new SWITCH_INFO struct and fills it with info
 *                about this switch; last RTL this BB retains a pointer to this
 *============================================================================*/
bool isSwitch(PBB pSwitchBB, SemStr* pDest, UserProc* pProc)
{

    // return false;                // Use to disable switch analysis

    SemStr ssJmp;                   // The semantic string repr jump, or
                                    // reresenting <expr> when bPostForm
    bool bStopped = false;          // True when search must stop
    bool bUpperFnd = false;         // True when have ss for upper bound
    bool bGotUpper = false;         // True when have final iUpper
    bool bGotLower = false;         // True when know lower bound
    bool bPostForm = false;         // Indicates that ssJmp repr <expr> now
    bool bLoaded = false;           // True when the table memory is loaded
    bool bGotDefines = false;       // True when have set itDefinesSw
    PBB pCurBB = pSwitchBB;         // Current basic block
    SemStr ssBound;                 // Expression for the upper etc bound
    HRTL* pRtl;                   // Current RTL
    int indexRT = -1;               // Current RT in current RTL
    char chForm = '?';              // Unknown form so far
    int iLower, iUpper = 0;         // Upper and lower bounds for switch var
    ADDRESS uTable = 0;             // Address of the table
    int iNumTable = 0;              // Number of items in table (form H only)
    int iOffset;                    // Offset from jump to table (form R only)
    HRTLList_IT itDefinesSw;             // Iterator to RTL that defines switch var
    int iCompareReg = -1;           // The register involved in a compare, that
                                    //  might turn out to be of interest
    SemStr* pssCompare;             // Pointer to last compare argument
    // I'm not sure that bDestBound is a good idea!
    // It seems to be asserting that the current RTL affects ssBound, and not
    // ssJmp. But it could affect both!
    bool bDestBound = false;
	int iReg;
    iOffset = 0;                    // Needed only for type R
    uCopyPC = 0;                    // Needed only for type "r"; no call yet
    // Get the register that contains the function return value
    const int iFuncRetReg = getFuncRetReg();    // e.g. 8 for sparc
	int iDest;
	int n;
    SemStr* pRHS;
    SemStr* pLHS;
	RTAssgn* pRT;

    // Clear the set used for detecting cycles
    setPathDone.erase(setPathDone.begin(), setPathDone.end());

    // Get the RTL for the jump
    HRTLList_IT itCurRtl = (pSwitchBB->getHRTLs()->end());
    pRtl = *--itCurRtl;
    ADDRESS uJump = pRtl->getAddress();     // Needed for type R

    ssJmp = *pDest;             // Copy the desintation of the jump

#if DEBUG_SWITCH
    cout << hex << uJump << ":ssJmp begins with "; ssJmp.print();
    cout << endl;
#endif

    // Check if the memory load is part of the jump instruction (e.g.
    // x86 jmp [%eax + %ebx*4]). If so, count this as the first memory
    // load
    if (ssJmp.getFirstIdx() == idMemOf)
        bLoaded = true;

    do
    {
        bool bNegate;
        if ((++indexRT) >= (*itCurRtl)->getNumRT()) {
            bStopped = getPrevRtl(pCurBB, itCurRtl, bNegate);
            indexRT = 0;            // Start with first RT
        }
        if (bStopped)
        {
            // Maybe we have already detected the form, but just haven't
            // found a lower bound. Well, it must be zero!
            if (bPostForm) {
                iLower = 0;
                setSwitchInfo(pSwitchBB, chForm, iLower, iUpper,
                    uTable, iNumTable, iOffset, itDefinesSw, pProc);
                return true;
            }
            else return false;
        }
        pRtl = *itCurRtl;
        if (bNegate)
        {
            // We have come via a conditional branch. For the purposes
            // of switch detection, this means a logical negation
            ssBound.prep(idNot);
        }
//      pRtl->machineSimplify();
        
        // General approach: we look for the comparison that defines the
        // switch variable's upper bound separately.
        // We maintain a Semantic String, ssJmp, that is the current
        // expression for the destination of the register jump. To make
        // it easier to compare with canonical forms of switch statements,
        // we impose an ordering on commutative partial expressions, such
        // that when one side is an integer constant, it will be placed
        // on the right; SemStr::simplify() does this for us, as well as
        // folding constants.

        // Another semantic string, ssBound, is used to keep track of the
        // upper (and occasionally lower) bounds. Mostly, we end up with
        // r[v] GT k, but occasionally if the upper bound is larger than
        // will fit into an imm13, we get r[v] GT r[x] and have to subs-
        // titute into ssBound. For the epc compiler, we can get
        // r[v] GT k1 || r[v] LT k2, and have to add 4*k2 to the table
        // address.
        // Along the way, we check for subtracts that assign to a register
        // of interest.  This is assumed to set the lower bound, and is
        // therefore the ideal place to assign the switch variable.
        if ((pRtl->getKind() == LOW_LEVEL_HRTL) && (pRtl->getNumRT() > indexRT)){
            pRT = (RTAssgn*)pRtl->elementAt(indexRT);
            if (pRT->getKind() == RTASSGN) {
                SemStr* rhs = pRT->getRHS();
                // - r[ int iReg
                // 0 1   2    3
                if (rhs->getFirstIdx() == idMinus) {
                    // We have a subtract; is it from a register?
                    if ((rhs->getSecondIdx() == idRegOf) &&
                       (rhs->getThirdIdx() == idIntConst)) {
                        // We have a subtract from a register. But is the
                        // destination (lhs) a register of interest? That is,
                        // is the LHS register contained in ssJmp?
                        SemStr* lhs = pRT->getLHS();
                        if ((lhs->getFirstIdx() == idRegOf) &&
                          (lhs->getSecondIdx() == idIntConst)) {
                            int iReg = lhs->getThirdIdx();
                            if (ssJmp.isNumRegCont(iReg)) {
                                // Yes, it is. Remember this RTL, since it
                                // defines the switch variable.
                                itDefinesSw = itCurRtl;
                                bGotDefines = true;
                                // Note that we don't set the lower bound here.
                                // This is done much later, after the whole
                                // form has been matched.
                            }
                        }
                    }
                }
                // Or it could be an add of a negative constant
                // + r[ int iReg int const
                // 0 1   2   3    4    5
                else if (rhs->isArraySimilar(
                  sizeof(arrRegPlusNegConst) / sizeof(int),
                  arrRegPlusNegConst)) {
                    // Is the const negative?
                    int iconst = rhs->getIndex(5);
                    if (iconst < 0) {
                        // Is the destination a register of interest?
                        SemStr* lhs = pRT->getLHS();
                        int iReg = lhs->getThirdIdx();
                        if (ssJmp.isNumRegCont(iReg)) {
                            // Yes, it is. Remember this RTL, since it
                            // defines the switch variable
                            itDefinesSw = itCurRtl;
                            bGotDefines = true;
                        }
                    }
                }
            }
        }
        
        // See if we are defining the upper or lower bound with a compare
        // and branch. We need to do this first, because most other
        // cases require an assignment to a register of interest, and will
        // continue the loop if this is not found
        if ((pRtl->getKind() == CALL_HRTL) && ssJmp.isNumRegCont(iFuncRetReg))
        {
            // We have come across a call, and the function return
            // register is used in ssJmp. We have to assume that
            // this call will define the switch variable. Replace it
            // with 999 to ensure it is not altered.
            SemStr* pReg999 = new SemStr;
            pReg999->pushArr(3, arrReg999);
            ssJmp.substReg(iFuncRetReg, pReg999);
            // Do the same to ssBound
            ssBound.substReg(iFuncRetReg, pReg999);
            continue;
        }
        else if (pRtl->getKind() == JCOND_HRTL)
        {
            // It's a conditional branch
            JCOND_TYPE jt = ((HLJcond*)pRtl)->getCond();
            int idxBr = -1;
            if (ssBound.getFirstIdx() == idNot)
            {
                // Expect branch if unsigned lower or equals
                if (jt == HLJCOND_JULE)
                {
                    // Replace the not with idUP
                    ssBound.substIndex(0, idUP);
                }
                else
                {
                    // not really sure what to do... but it does recover
                    // from this by popping the idNot
//                  cout << "Switch @ " << hex << pRtl->getAddress() <<
//                      " idNot and branch that is not low or equals\n";
                }
            }
            else
            {
                // No negation.
                // Most times we expect branch if GTu
                // Some compilers (e.g. epc modula 2) have a signed
                // greater than comparison
                // epc also checks for the lower bound with a signed BL instr
                if ((jt == HLJCOND_JUG) || (jt == HLJCOND_JSG))
                    idxBr = idUP;           // Defining upper bound
                else if (jt == HLJCOND_JSL)
                    idxBr = idLO;           // Defining lower bound
                if (idxBr >= 0)
                {
                    // If we already have an expression there, we have
                    // to prepend an idOr
                    if (ssBound.len())
                        ssBound.prep(idOr);
                    // Add the appropriate index
                    ssBound.push(idxBr);

#if DEBUG_SWITCH
                    cout << "isSwitch @ " << hex << pRtl->getAddress();
                    cout << ": ssBound now "; ssBound.print(); cout << endl;
#endif

                    continue;
                }
            }
        }

        // Check for a compare instruction for the upper or lower bound
        // Must already have seen an appropriate branch
        if (ssBound.getLastIdx() == idUP || ssBound.getLastIdx() == idLO)
        {
            int iReg;
            if ((*itCurRtl)->isCompare(iReg, pssCompare))
            {
                // It is a compare instruction. But it should be a comparison
                // to a register of interest
                if (!ssJmp.isNumRegCont(iReg)) {
                    // It doesn't appear to be a register of interest. But it
                    // might be, if there is a copy ahead. Example from
                    // /usr/ccs/bin/dis (Sparc Solaris 2.6):
                    // 130c8:  90 10 00 0a        mov          %o2, %o0
                    // 130cc:  92 00 60 50        add          %g1, 80, %o1
                    // 130d0:  80 a2 a0 19        cmp          %o2, 25
                    // 130d4:  18 80 00 05        bgu          0x130e8
                    // So remember the register
                    iCompareReg = iReg;
                    continue;
                }
                // Append r[ int iReg
                ssBound.push(idRegOf); ssBound.push(idIntConst);
                ssBound.push(iReg);
                // Append the semantic string for the thing being compared
                // to
                ssBound.pushSS(pssCompare);
                bUpperFnd = true;           // Trigger pattern matching

#if DEBUG_SWITCH
                cout << "isSwitch @ " << hex << (*itCurRtl)->getAddress() <<
                 ": ssBound now "; ssBound.print(); cout << endl;
#endif

                // Force a check. There may be no instructions after
                // this that assign to a register of interest, etc,
                // but we may have a complete switch statement by now.
                goto forcedCheck;
            }
        }

        // Check if this instruction affects the condition codes. If it
        // does, and there is something in ssBound, then we need to pop
        // off the end of ssBound, since (by virtue of the fact that we
        // have reached here at all) the instruction defining the CCs is
        // not a compare of interest, the branch is also not a branch of
        // interest. So we remove the idNot, if any
        n = pRtl->getNumRT();
        if (n == 0) continue;
        // We assume that the last RTL will define the flags, if at all
        if (pRtl->elementAt(n-1)->getKind() == RTFLAGCALL)
        {
            if (ssBound.getFirstIdx() == idNot)
            {
                ssBound.popFirst();
#if DEBUG_SWITCH
                cout << "Popping idNot from ssBound: now ";
                ssBound.print(); cout << endl;
#endif
            }
        }

        // See if we have an RTL whereby the first significant RT is an
        // assignment, and the dest is a register of interest.
        // A register of interest is any register used by ssJmp, except
        // 999 (don't want to change it any more)
        // We are also interested in any registers in ssBound
        // See if the current RT is an assignment
        if (pRtl->elementAt(indexRT)->getKind() != RTASSGN) continue;
        pRT = (RTAssgn*)pRtl->elementAt(indexRT);

        // Check for an instruction that uses %pc on the right hand side
        // (e.g. sparc call $+8). Assume that the first RT will have this use
        pRHS = pRT->getRHS();
        pLHS = pRT->getLHS();
        if (pRHS->isSpRegCont(idPC)) {
            uCopyPC = pRtl->getAddress();
        }

        // If this is not a register we can't handle that
        if (pLHS->getFirstIdx() != idRegOf) continue;
        iDest = pLHS->getThirdIdx();

        // Check that the destination is to a register used by the
        // current jump expression, or by the current bound
        // expression. Remember which expression iDest refers to in
        // bDestBound
        if (!ssJmp.isNumRegCont(iDest))
            if (ssBound.isNumRegCont(iDest))
                bDestBound = true;
            else continue;

        // Check if we are loading from memory to a register of interest.
        if ((pRHS->getFirstIdx() == idMemOf) ||
            (pRHS->getFirstIdx() == idSignExt && 
              pRHS->getSecondIdx() == idMemOf) ||
            (pRHS->getFirstIdx() == idSize && 
              pRHS->getThirdIdx() == idMemOf) ||
            (pRHS->getFirstIdx() == idSignExt && 
              pRHS->getSecondIdx() == idSize &&
              pRHS->getIndex(3) == idMemOf) ||
            (pRHS->getFirstIdx() == idZfill && 
              pRHS->getIndex(3) == idMemOf) ||
            (pRHS->getFirstIdx() == idSgnEx && 
              pRHS->getIndex(3) == idMemOf)
           )
        {
            if (bDestBound)
            {
                // Loading the upper expression from memory. Freeze the
                // bounds expression
                SemStr Reg999;
                SemStr* pReg999 = new SemStr;
                pReg999->pushArr(3, arrReg999);
                ssBound.substReg(iDest, pReg999);
                continue;
            }
            // We record the fact that we have seen the m[] part of the switch
            // expression. All subsequent loads are assumed to be loading the
            // switch variable from memory, which is very different
            if (!bLoaded) {
                bLoaded = true;
                // We don't want to emit code for this RTL, since it is loading
                // from the original source program's jump table. Better to
                // leave the RTLs there, in case the interpreter will need them
                pRtl->setCommented(true);
            }
            else
            {
                // We are loading our switch variable from memory.
                // To make sure we don't change this any more, replace
                // the r[iDest] in ssJmp to r[999]
                SemStr Reg999;
                SemStr* pReg999 = &Reg999;
                pReg999->pushArr(3, arrReg999);
                ssJmp.substReg(iDest, pReg999);
                // Do the same to ssBound
                ssBound.substReg(iDest, pReg999);
                iDest = 999;             // Don't subst again
                // This defines the switch variable, if not already set
                if (!bGotDefines) {
                    bGotDefines = true;
                    itDefinesSw = itCurRtl;
                }
            }
            // Don't exit the loop here; we may be done if we have the table
            // address
        }

        // If iCompareReg is not -1, check if the rhs simplifies to just
        // r[iCompareReg]. If so, and we don't have a proper compare now,
        // that means that we have a copy instruction (move), whch means
        // that the compare could be on either register. We assume that
        // the last compare was valid (should really test that the register
        // isn't changed between here and the compare.)
        if (iCompareReg != -1 && ssBound.len() <= 2) {
            SemStr rhs(*pRHS);
            rhs.machineSimplify();      // Could be sparc move: r[0] | r[x]
            rhs.simplify();             // 0 | r[x] -> r[x]
            if ((rhs.len() == 3) && (rhs.getFirstIdx() == idRegOf) &&
                rhs.getSecondIdx() == idIntConst) {
                // Update ssBound to be a valid compare
                if (ssBound.getFirstIdx() == idNot) {
                    // This can happen when there is a branch not to do with
                    // the upper bound, e.g. /usr/ccs/bin/dis Solaris 2.6 13af0
                    ssBound.popFirst();
                }
                ssBound.pushSS(&rhs);
                // Append the semantic string for the thing being compared
                // to
                ssBound.pushSS(pssCompare);
                bUpperFnd = true;           // Trigger pattern matching
            }
        }

        // Check for r[x] & const; to get here, must assign to a register of
        // interest. If so, and we haven't already set the upper bound, we
        // assume that this `and' is setting the upper bound
        // In any case, the lower bound is 0
        // & r[ int x int k
        // 0 1  2   3 4   5
        if (pRHS->isArraySimilar(sizeof(arrRegAndConst) / sizeof(int),
          arrRegAndConst)) {
            iReg = pRHS->getIndex(3);
            if (!bUpperFnd) {
                // Found an upper bound
                bGotUpper = true;
                iUpper = pRHS->getIndex(5);
                // This RT defines the switch variable
                bGotDefines = true;
                itDefinesSw = itCurRtl;
            }
            // Else don't assign itDefinesSw here; it's currently at the
            // compare that sets the upper bound
            // We won't see a lower bound; it's 0
            iLower = 0;
            bGotLower = true;
        }
#if 0
        if (bDestBound)
        {
            ssBound.substReg(iDest, pRHS);
            ssBound.simplify();
        }
        else if (iDest != 999)
        {
            // Substitute register iDest with the RHS
            ssJmp.substReg(iDest, pRHS);
        }
#else
        // Substitute both ssJmp and ssBound, if affected
        if (iDest != 999) {
            ssBound.substReg(iDest, pRHS);
            ssBound.simplify();
            ssJmp.substReg(iDest, pRHS);
        }
#endif

forcedCheck:
#if DEBUG_SWITCH
        cout << "ssJmp @ " << hex << (*itCurRtl)->getAddress() << ": ";
        ssJmp.print(); cout << endl;
        if (ssBound.len()) {cout << "ssBound is "; ssBound.print();
            cout << endl;}
#endif

        // Needn't do any checking until we have found the uppper bound
        if (!bPostForm && bUpperFnd) {
            // First do any machine specific simplification. On Sparc, this
            // replaces r[0] on the RHS with 0
            ssJmp.machineSimplify();
            // Simplify the expression, if possible, using contant folding
            // and various other techniques
            ssJmp.simplify();

#if DEBUG_SWITCH
            cout << "isSwitch @ " << hex << (*itCurRtl)->getAddress() << ": ";
            ssJmp.print(); cout << endl;
#endif

            // Simplify the expression, by removing all {size} and sign extend
            // (!) operations. These are tangential to the switch form
            ssJmp.removeSize();

            // Check for form A (addresses)
            // Pattern: m[<expr> * 4 + T ]
            if (ssJmp.isArraySimilar(sizeof(arrA) / sizeof(int), arrA)) {
                chForm = 'A';
                // Because of the variable expression near the start,
                // we have to work backwards!
                int n = ssJmp.len();
                uTable = (ADDRESS) ssJmp.getIndex(n-1);
                SemStr* pLower = &ssJmp;
                for (int i=0; i < 3; i++)       // m[, +, *
                    pLower = pLower->getSubExpr(0);
                ssJmp = *pLower;
            }

            // Check for form O (offsets)
            // Pattern: m[<expr> * 4 + T ] + T
            else if (ssJmp.isArraySimilar(sizeof(arrO) / sizeof(int), arrO)) {
                chForm = 'O';
                int n = ssJmp.len();
                uTable = (ADDRESS) ssJmp.getIndex(n-1);
                SemStr* pLower = &ssJmp;
                for (int i=0; i < 4; i++)       // m[, +, +, *
                    pLower = pLower->getSubExpr(0);
                ssJmp = *pLower;
            }

            // Check for form H (hash table)
            // Note: the hash expression varies
            // Expect an expression like this:
            // m[(((<expr> & 63) * 8) + 12345) + 4]
            // Simplifies to m[((<expr> & 63) * 8) + 12344]
            // => m[ + * & <expr> int 63 int 8 int 1234
            else if (ssJmp.isArraySimilar(sizeof(arrH) / sizeof(int), arrH)) {
                chForm = 'H';
                int n = ssJmp.len();
                iNumTable = ssJmp.getIndex(n-5)+1;      // mask+1
                uTable = (ADDRESS) ssJmp.getIndex(n-1) - 4;
                SemStr* pLower = &ssJmp;
                for (int i=0; i < 4; i++)       // m[, +, *, &
                    pLower = pLower->getSubExpr(0);
                ssJmp = *pLower;
            }

            // Check for form O1 (one of the relocatable versions of O)
            // Expect an expression like this:
            // %pc + m[%pc  + ((<expr> * 4) + k)]
            // where k is a small constant, typically 28
            else if (ssJmp.isArraySimilar(sizeof(arrO1) / sizeof(int), arrO1)) {
                chForm = 'R';
                int n = ssJmp.len();
                int k = ssJmp.getIndex(n-1);
                uTable = uJump + 8;
                iOffset = k;            // Needed to figure correct out edges
                SemStr* pLower = ssJmp.getSubExpr(1);   // +
                pLower = pLower->getSubExpr(0);         // m[
                pLower = pLower->getSubExpr(1);         // +
                for (int i=0; i < 2; i++)       // +, *
                    pLower = pLower->getSubExpr(0);
                ssJmp = *pLower;
            }

            // Check for form O2 (one of the relocatable versions of O)
            // Expect an expression like this:
            // %pc + m[%pc  + ((<expr> * 4) - k)] - k
            // Infix: - + %pc m[ - + %pc * <expr> int 4 int k ] int k
            // where k is a smallish constant, e.g. 288
            else if (ssJmp.isArraySimilar(sizeof(arrO2) / sizeof(int), arrO2)) {
                chForm = 'r';
                int n = ssJmp.len();
                int k = ssJmp.getIndex(n-1);
                // Note: uCopyPC must be set to the address of the call instr
                uTable = uCopyPC - k;
                SemStr* pLower = ssJmp.getSubExpr(0);   // -
                pLower = pLower->getSubExpr(1);         // +
                pLower = pLower->getSubExpr(0);         // m[
                pLower = pLower->getSubExpr(0);         // -
                pLower = pLower->getSubExpr(1);         // +
                ssJmp = *pLower;
            }


            // If we have a form pattern, then we are into the post form
            // phase now
            bPostForm = chForm != '?';
        }


        if (!bGotUpper || !bGotLower) {
            // Check if we have the upper and lower bounds together
            // ((r[v] GT k1] || (r[v] LT k2))
            static int arBoth[] = {
                idOr, idUP, idRegOf, idIntConst, -1, idIntConst, -1,
                idLO, idRegOf, idIntConst, -1, idIntConst, -1};
            if (ssBound.isArrayEqual(sizeof(arBoth) / sizeof(int), arBoth)) {
                // We have both bounds
                iLower = ssBound.getIndex(12);
                iUpper = ssBound.getIndex(6);
                // Adjust the table by iLower*size
                int iSize = 4;
                // We may not know the form as yet!
                // But it's very unlikely to be form H anyway
                if (chForm == 'H') iSize = 8;
                uTable += iLower * iSize;
                if (chForm != '?') {
                    setSwitchInfo(pSwitchBB, chForm, iLower, iUpper,
                        uTable, iNumTable, iOffset, itDefinesSw, pProc);
                    return true;
                }
                bGotUpper = true; bGotLower = true;
            }
        }

        // Check for upper bound only
        if (!bGotUpper) {
            static int arUpper[] = {
                idUP, idRegOf, idIntConst, -1, idIntConst, -1};
            if (ssBound.isArrayEqual(sizeof(arUpper) / sizeof(int), arUpper)) {
                bGotUpper = true;
                iUpper = ssBound.getIndex(5);
                // If we haven't seen the instruction defining the switch var
                // (usually a subtract), then this compare defines it
                if (!bGotDefines) {
                    bGotDefines = true;
                    itDefinesSw = itCurRtl;
                }
            }
        }

        // When bPostForm is true, we are in the second phase of the
        // algorithm where ssJmp represents <expr>
        // We can check for the lower bound by subtraction now, but only
        // if we have the upper bound already
        if (bPostForm && !bGotLower && bGotUpper) {
            ssJmp.simplify();

#if DEBUG_SWITCH
            cout << "Post form @ " << hex << (*itCurRtl)->getAddress() << ": ";
            ssJmp.print(); cout << endl;
#endif

            // Save the "switch variable" so we can return it in a SWITCH_INFO
//          pSwitchVar = new SemStr(ssJmp);
            // Save an iterator to the current RTL, so we can save the switch
            // variable
            // But if we already have it defined (because we have defined the
            // lower bound) then leave it alone
//          if (!bGotDefines)
//              itDefinesSw = itCurRtl;

            // Now check if have `r[v] - k'
            bool bRet = ssJmp.findSubExpr(
                sizeof(arrRegMinus) / sizeof(int), arrRegMinus, iLower);
            if (bRet)
            {
                // We now have the lower bound, and all is done
                bGotLower = true;
#if DEBUG_SWITCH
            cout << "Got lower: ssJmp "; ssJmp.print(); cout << " -> iLower ";
            cout << dec << iLower << endl;
#endif
                iUpper += iLower;
            }
            else {
                // Could be r[v] + k, where k is positive; in this case
                // the lower bound is -k
                bool bRet = ssJmp.findSubExpr(
                    sizeof(arrRegPlus) / sizeof(int), arrRegPlus, iLower);
                if (bRet) {
                    // We now have the lower bound, and all is done
                    iLower = -iLower;
#if DEBUG_SWITCH
                    cout << "Got lower: ssJmp "; ssJmp.print();
                    cout << " -> iLower " << dec; cout << iLower << endl;
#endif
                    iUpper += iLower;
                    bGotLower = true;
                }
            }
        }

        // Check if we are done
        if (bGotLower && bGotUpper && chForm != '?') {
            setSwitchInfo(pSwitchBB, chForm, iLower, iUpper,
                uTable, iNumTable, iOffset, itDefinesSw, pProc);
            return true;
        }

    } while (!bStopped);

    return false;
}

/*==============================================================================
 * FUNCTION:    setSwitchInfo
 * OVERVIEW:    Initialises a new SWITCH_INFO struct with all the appropriate
 *              values, and causes the RTL at the end of this BB to
 *              store a pointer to it
 * PARAMETERS:  pSwitchBB: pointer to the original BB with the HLNwayCond as
 *              the last RTL
 *              chForm: Switch form: 'A', 'O', 'R', or 'H'
 *              iLower: Lower bound of the switch variable
 *              iUpper: Upper bound for the switch variable
 *              uTable: Native address of the table
 *              iNumTable: Number of entries in the table (form H only)
 *              iOffset: Distance from jump to table (form R only)
 *              itDefinesSw: iterator to RTL that defines the switch var
 *              pProc: pointer to the current Proc object
 * RETURNS:       <nothing>
 *============================================================================*/
void setSwitchInfo(PBB pSwitchBB, char chForm, int iLower, int iUpper,
    ADDRESS uTable, int iNumTable, int iOffset, HRTLList_IT itDefinesSw,
    UserProc* pProc)
{
    SWITCH_INFO* pSwitchInfo = new SWITCH_INFO;
    pSwitchInfo->chForm = chForm;
    pSwitchInfo->iLower = iLower;
    pSwitchInfo->iUpper = iUpper;
    pSwitchInfo->uTable = uTable;
    pSwitchInfo->iNumTable = iNumTable;
    pSwitchInfo->iOffset = iOffset;
    HLNwayJump* jump = (HLNwayJump*)pSwitchBB->getHRTLs()->back();
    jump->setSwitchInfo(pSwitchInfo);

    // Now add an assignment to the RTL defining the switch variable, so we can
    // use that v[] in the final switch statement in the intermediate code.
    // If the assignment is a subtract, assume that this subtract defines the
    // lower bound, so we want the first subexpression of the RHS:
    // r[8] = r[8] - 2
    // Otherwise, assume that there is no lower bound, so we want the result:
    // r[9] = m[r[16] + 572]
    SemStr* pLHS = pProc->newLocal(INTEGER);

    // Want the defining assignment. Assume it's the last RTAssign of the RTL
    int n = (*itDefinesSw)->getNumRT();
    int i=n-1;
    RTAssgn* pRT = (RTAssgn*)(*itDefinesSw)->elementAt(i);
    while (pRT->getKind() != RTASSGN)
        pRT = (RTAssgn*)(*itDefinesSw)->elementAt(--i);
    if (pRT->getRHS()->getFirstIdx() == idMinus) {
        // We want to insert the var assignment before the defining assignment,
        // and from the first subexpression
        (*itDefinesSw)->insertAfterTemps(pLHS, pRT->getRHS()->getSubExpr(0));
    }
    // Check if it's adding a negative constant to a register. If so,
    // assume it's just like the subtract above
    else if ((pRT->getRHS()->isArraySimilar(
      sizeof(arrRegPlusNegConst) / sizeof(int), arrRegPlusNegConst)) &&
      (pRT->getRHS()->getIndex(5) < 0)) {
        (*itDefinesSw)->insertAfterTemps(pLHS, pRT->getRHS()->getSubExpr(0));
    }
    else {
        // We assume that this assign is a load (or something) that defines the
        // switch variable
        SemStr* pRHS = new SemStr(*pRT->getLHS());
        (*itDefinesSw)->insertAssign(pLHS, pRHS, false);
    }
    pSwitchInfo->pSwitchVar = pLHS;
}

/*==============================================================================
 * FUNCTION:      processSwitch
 * OVERVIEW:      Called when a switch has been identified. Visits the
 *                  destinations of the switch, adds out edges to the BB, etc
 * PARAMETERS:    pBB - Pointer to the BB containing the register jump
 *                delta - (uHost - uNative)
 *                pCfg - Pointer to the Cfg object for the current procedure
 *                targets - queue of targets yet to be visited
 *                proc - pointer to the Proc object that the switch is in
 * RETURNS:       <nothing>
 *============================================================================*/
void processSwitch(PBB pBB, int delta, Cfg* pCfg, TARGETS& targets, Proc* proc)
{
    HLNwayJump* jump = (HLNwayJump*)pBB->getHRTLs()->back();
    SWITCH_INFO* si = jump->getSwitchInfo();
    // Update the delta field
    si->delta = delta;

#if DEBUG_SWITCH
    cout << "Found switch statement type " << si->chForm << " with table at ";
    cout << hex << si->uTable << ", ";
    if (si->iNumTable)
        cout << dec << si->iNumTable << " entries, ";
    cout << "lo= " << dec << si->iLower << ", hi= " << si->iUpper << endl;
#endif
    ADDRESS uSwitch;
    int iNumOut, iNum;
    if (si->chForm == 'H')
    {
        iNumOut = 0; int i, j=0;
        for (i=0; i < si->iNumTable; i++, j+=2)
        {
            // Endian-ness doesn't matter here; -1 is still -1!
            int iValue = ((ADDRESS*)(si->uTable+delta))[j];
            if (iValue != -1)
                iNumOut++;
        }
        iNum = si->iNumTable;
    }
    else
    {
        iNumOut = si->iUpper-si->iLower+1;
        iNum = iNumOut;
    }
    // Emit an NWAY BB instead of the COMPJUMP
    // Also update the number of out edges.
    pBB->updateType(NWAY, iNumOut);
    
    for (int i=0; i < iNum; i++)
    {
        // Get the destination address from the
        // switch table. Note: assumes that the
        // table is in the .text section!
        if (si->chForm == 'H')
        {
            int iValue = ((ADDRESS*)(si->uTable+delta))[i*2];
            if (iValue == -1) continue;
            uSwitch = fetch4((unsigned char*)(si->uTable + delta + i*8 + 4));
        }
        else
            uSwitch = fetch4((unsigned char*) (si->uTable + delta + i*4));
        if ((si->chForm == 'O') || (si->chForm == 'R') || (si->chForm == 'r'))
            // Offset: add table address to make a real pointer to code
            // For type R, the table is relative to the branch, so take iOffset
            // For others, iOffset is 0, so no harm
            uSwitch += si->uTable - si->iOffset;
        visit(pCfg, uSwitch, targets, pBB);
        pCfg->addOutEdge(pBB, uSwitch, true);
    }

    // Update the coverage
    // Note! Should verify that the switch statement is in the code
    // section, before adding the coverage
    // Perhaps should add it to the prog object if outside the current
    // proc's boundaries (but how to decide that?)
	UserProc* uProc = (UserProc*)proc;
    if (si->chForm == 'H') {
        uProc->addRange(si->uTable, si->uTable + iNum * 8);
    }
    else {
        uProc->addRange(si->uTable, si->uTable + iNum * 4);
    }
}
