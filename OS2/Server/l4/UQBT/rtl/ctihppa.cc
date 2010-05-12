/*
 * Copyright (C) 2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:          ctihppa.cc
 * OVERVIEW:      Part of the implementation of the HRTL class, to do with
 *                detecting Pa/Risc control transfer instructions. Some methods
 *                are source platform independent, i.e. all platforms must
 *                implement these methods.
 *
 * Copyright (C) 2001, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.8 $
 *
 * 30 Apr 01 - Mike: Created from ctisparc.cc
 * 01 May 01 - Mike: Return register is 28
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 * 06 Aug 01 - Mike: Replaced machineSimplify with a call to searchReplaceAll;
 *              remove "tmpNul = 0"
 * 08 Aug 01 - Mike: Simplify guards as well as LHS, RHS of assignments
 * 09 Aug 01 - Mike: For now, remove assignments to r[0]
 */

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "global.h"
#include "hrtl.h"
#include "ss.h"
#include "decoder.h"                // For RESTORE_INFO

/*==============================================================================
 * File globals.
 *============================================================================*/
// Static globals representing %npc etc. The function initCti()
// has to be called to initialise these
static int idNPC = -1;
static int idCWP = -1;
static int idTmp = -1;          // This is for the specific special ID, "tmp"

/**********************************
 * Global methods
 **********************************/

/*==============================================================================
 * FUNCTION:      initCti
 * OVERVIEW:      Any initialisation that needs doing just once
 * PARAMETERS:    <none>
 * RETURNS:       <nothing>
 *============================================================================*/
void initCti()
{
    // We need the 2nd parameter to findItem because we may or may not have
    // parsed any instructions that involve %npc etc (depending on whether
    // we want the low level RTLs or not). This prevents needless error messages
    idNPC = theSemTable.findItem("%npc", false);
    idCWP = theSemTable.findItem("%CWP", false);
    idTmp = theSemTable.findItem("tmp", false); // Don't confuse with idTemp
}

/*==============================================================================
 * FUNCTION:      getFuncRetReg
 * OVERVIEW:      Returns the register number for the standard return value
 *                  register (%r28 for Pa/Risc)
 * PARAMETERS:    None
 * RETURNS:       Register number (integer)
 *============================================================================*/
// Sparcs return function values in r[28]
int getFuncRetReg()
{
    return 28;
}

#if 0
/*==============================================================================
 * FUNCTION:      HRTL::isAnullBit
 * OVERVIEW:      This is a sparc-specific function, to determine whether the
 *                RTL is that of a parisc instruction with the anull bit set, or
 *                not. This RT should be a branch type instruction. Expect ...
 * PARAMETERS:    <none>
 * RETURNS:       instruction has annul bit set
 *============================================================================*/
bool HRTL::isAnullBit() const
{
    return false;
}
#endif

/*==============================================================================
 * FUNCTION:      HRTL::getOutAddr
 * OVERVIEW:      Get the source address for the indicated out edge (either 0
 *                or 1) 0 is for branch taken; 1 for not taken. This RTL should
 *                be known to represent a jump or call insruction
 * NOTE:          Basically obsolete
 * PARAMETERS:    idx: As above
 * RETURNS:       Native address as above
 *============================================================================*/
ADDRESS HRTL::getOutAddr(int idx) const 
{
    return 0;
}

/*==============================================================================
 * FUNCTION:      HRTL::isCompare
 * OVERVIEW:      Returns true if this RTL is a compare instruction
 * PARAMETERS:    Two reference parameters; see RETURNS
 * RETURNS:       Returns true if a compare instruction. If so, sets iReg to
 *                  the register involved in the compare, and pTerm to a
 *                  SS pointer to the thing being compared to
 *============================================================================*/
// Epect RTL of the form
// r[tmpx] := r[r] - imm
// SUBFLAGS(...)
bool HRTL::isCompare(int& iReg, SemStr*& pTerm)
{
    int n = getNumRT();
    for (int i=0; i < n; i++) {
        RTAssgn* pRT = (RTAssgn*)this->elementAt(i);
        if (pRT->getKind() != RTASSGN) continue;
        SemStr* lhs = pRT->getLHS();
        if (lhs->getFirstIdx() != idRegOf) continue;
        if (lhs->getSecondIdx() != idTemp) continue;
        SemStr* rhs = pRT->getRHS();
        if (rhs->getFirstIdx() != idMinus) continue;
        if (rhs->getSecondIdx() != idRegOf) continue;
        if (rhs->getThirdIdx() != idIntConst) continue;
        iReg = rhs->getIndex(3);
        pTerm = rhs->getSubExpr(1);
        return true;
    }
    return false;
}


/*==============================================================================
 * FUNCTION:      HRTL::allSimplify
 * OVERVIEW:      Perform machine dependent, and standard optimisations.
 * PARAMETERS:    None
 * RETURNS:       Nothing
 *============================================================================*/
// Do a machine dependent simplification. For pa-risc, this involves
// replacing r[0] with 0 on the right hand side.
void HRTL::allSimplify()
{
    list<RT*>::iterator it;
    for (it = rtlist.begin(); it != rtlist.end(); ) {
        if ((*it)->getKind() != RTASSGN) {it++; continue;}
        RTAssgn* pRT = (RTAssgn*) *it;

        // Remove any RTAssgns with a destination of r[0]
        // Note: can't do this in architectures where matching of CCs is
        // performed; sometimes the result of a comparison is needed
        SemStr* lhs = pRT->getLHS();
        if (
          (lhs->getFirstIdx() == idRegOf) &&
          (lhs->getSecondIdx()== idIntConst) &&
          (lhs->getThirdIdx() == 0)) {
            // Assign to %g0. Remove the whole RT
            it = rtlist.erase(it);
            continue;
        }

        // Simplify the guard
        if (pRT->simplifyGuard()) {
            // Returned true; this means the whole RT can be deleted
            // Ensure that iterator it remains valid
            it = rtlist.erase(it);
            continue;
        }

        // Now attempt to simplify the LHS and RHS.
        // Remove lines of the form idTmp = 0
        if (
          (lhs->getFirstIdx() == idRegOf) &&
          (lhs->getSecondIdx() == idTemp) &&
          (lhs->getThirdIdx() == idTmpNul) &&
          (pRT->getRHS()->getFirstIdx() == idIntConst) &&
          (pRT->getRHS()->getSecondIdx() == 0)) {
            // Delete the RT; keep it valid
            it = rtlist.erase(it);
            continue;
        }

        // Actually, for pa-risc, don't want to machine simplify all LHSs,
        // since this would lead to compares looking like "0 := r[8] - 5".
        // But we want to fix things like "m[%r0 + %r20] := exp"
        // pRT->getLHS()->machineSimplify();
        if (lhs->getFirstIdx() == idMemOf) {
            SemStr* sub = lhs->getSubExpr(0);
            if (sub->machineSimplify()) {
                // Note that the subexpression is a COPY; we have to update
                // the original expression now. Easiest to just prepend the
                // idMemOf to the copy
                delete lhs;
                sub->prep(idMemOf);
                pRT->updateLHS(sub);
            }
        }
        pRT->getRHS()->machineSimplify();

        // Now also standard simplifications, e.g. 0|2 => 2
        // LHS could need simplification, e.g. r[8+1] => r[9]
        pRT->getLHS()->simplify();
        pRT->getRHS()->simplify();
        it++;                   // Can't have as part of for loop,
                                // because of the erase() and continue
    }

    // Forward substitute any temps, if possible
    forwardSubs();
}


/*==============================================================================
 * FUNCTION:      HRTL::updateNumBytes
 * OVERVIEW:      Set the number of EXTRA bytes (in excess of 4) in the
 *                  instruction. Will be 0 for all instructions except
 *                  prologues and epilogues.
 * PARAMETERS:    uNumBytes - the size of the instruction
 * RETURNS:       <nothing>
 *============================================================================*/
void HRTL::updateNumBytes(unsigned uNumBytes)
{
    numNativeBytes = uNumBytes;
}

/*==============================================================================
 * FUNCTION:      HRTL::getNumBytes
 * OVERVIEW:      Return the number of bytes this RTL (usually but not always 4)
 *                  (e.g. could be a two instruction idiom or logue, etc)
 * PARAMETERS:    None
 * RETURNS:       Unsigned number of bytes
 *============================================================================*/
unsigned HRTL::getNumBytes() const
{
    return numNativeBytes;
}

/**********************************
 * HLCall methods
 **********************************/

/**********************************
 * HLReturn methods
 **********************************/

/**********************************
 * SemStr methods
 **********************************/

/*==============================================================================
 * FUNCTION:      SemStr::machineSimplify()
 * OVERVIEW:      Perform machine specific optimisation to this SS
 *                  Replaces r[0] with 0. Use searchReplaceAll now
 *                  (old algorithm failed to replace more than one r[0])
 * PARAMETERS:    None
 * RETURNS:       True if made a change
 *============================================================================*/
bool SemStr::machineSimplify()
{
    SemStr repl;
    repl << idIntConst << 0;
    SemStr srch(repl);
    srch >> idRegOf;
    
    return searchReplaceAll(srch, repl);
}


