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
 * FILE:          ctisparc.cc
 * OVERVIEW:      Part of the implementation of the HRTL class, to do with
 *	              detecting sparc control transfer instructions. Some methods
 *                are source platform independent, i.e. all platforms must
 *	              implement these methods.
 *
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.41 $
 * 11 Aug 98 - Mike
 *	Split this sparc dependent part off from rtlist.cc
 * 4 Sep 98 - Mike
 *	Put the definitions for the sparc dependent code into a separate class
 *	RTlistSparc defined in ctisparc.h
 * 15 Sep 98 - Mike
 *	Fixed logic that was treating JMPL with %o7=15 as I_NOWHERE (now I_CALL)
 * 14 Oct 98 - Mike: Added MachineSimplify().
 * 14 Oct 98 - Mike: Mod getJmpRegisters() to cope with epc's jmp %o0 + 0
 * 02 Nov 98 - Mike: Removed getJmpRegisters() altogether; also isLoImmedLoad()
 *						and isHiImmedLoad()
 * 15 Dec 98 - Mike: Simplified MachineSimplify() to just replace r[0] with 0
 * 06 Jan 99 - Mike: Added delete's to reduce memory leaks
 * 27 Jan 99 - Mike: Use COMPJUMP and COMPCALL now
 * 29 Jan 99 - Mike: getControlTransferEx returns a new ss in pDest now
 * 04 Feb 99 - Mike: Oops - had calls using %pc in third RT; now back to r[15]
 * 11 Feb 99 - Mike: getControlTransferEx() drops the "=1" from pCond now
 * 24 Mar 99 - Mike: no longer initialise idPC, idZF, etc
 * 26 Mar 99 - Doug: Removed RTlistSparc class - merged into RTlist
 *                   getControlTransferEx removed
 *                   getControlTransfer implemented for HL types
 * 30 Mar 99 - Doug: getControlTransfer removed. Functionality pushed into class
 *                   hierarchy (e.g. dest and cond determined at construction)
 * 31 Mar 99 - Mike: In SetCTIInfo() (two functions), pDest is a copy now
 * 07 Apr 99 - Mike: Updated comments; changes for HEADERS
 * 15 Apr 99 - Mike: Restored idTmp processing (fixes dest for BA instructions)
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 22 Jun 99 - Mike: Removed old code no longer used, like setCtiInfo()
 * 01 Dec 00 - Mike: machineSimplify returns bool now; fixed r[0] in memory
 *              expression on LHS (sparc)
 * 05 Mar 01 - Nathan: Adjusted isCompare to deal with modified comparisons
 * 01 Aug 01 - Brian: New class HRTL replaces RTlist. Changed include file to
 *              hrtl.h from rtl.h.
 * 08 Aug 01 - Mike: Guard simplification and forward substitution for HRTLs
 */

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "global.h"
#include "hrtl.h"
#include "ss.h"
#include "decoder.h"				// For RESTORE_INFO

/*==============================================================================
 * File globals.
 *============================================================================*/
// Static globals representing %npc etc. The function initCti()
// has to be called to initialise these
static int idNPC = -1;
static int idCWP = -1;
static int idTmp = -1;			// This is for the specific special ID, "tmp"

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
	idTmp = theSemTable.findItem("tmp", false);	// Don't confuse with idTemp
}

/*==============================================================================
 * FUNCTION:      getFuncRetReg
 * OVERVIEW:      Returns the register number for the standard return value
 *					register (%o0 for sparc)
 * PARAMETERS:    None
 * RETURNS:       Register number (integer)
 *============================================================================*/
// Sparcs return function values in r[8] (i.e. %o0)
int getFuncRetReg()
{
	return 8;
}

/*==============================================================================
 * FUNCTION:      HRTL::isAnullBit
 * OVERVIEW:      This is a sparc-specific function, to determine whether the
 *                RTL is that of a sparc instruction with the anull bit set, or
 *                not. This RT should be a branch type instruction. Expect the
 *                third RT to be of the form:
 *                  *32* %npc := [(a = 0) ?     
 *                     [(COND[idx] = 1) ? r[tmp]+(4*disp22) : %npc+4]
 *                     : [(COND[idx] = 1) ? r[tmp]+(4*disp22)+4 : %npc+8]] // BA
 *                  *32* %npc := [(COND[idx] = 1) ? r[tmp] + (4*disp22) :
 *                    [(a = 1) ? %npc+8 : %npc+4]]			// BE
 *                  *32* %npc := r[15] + (4 * disp30)		// CALL
 *                  *32* %npc := eaddr 						// JMPL
 * PARAMETERS:    <none>
 * RETURNS:       instruction has annul bit set
 *============================================================================*/
bool HRTL::isAnullBit() const
{
	assert(rtlist.size() == 3);
	// Get a pointer to the third RT
	RTAssgn* pRT = (RTAssgn*)this->elementAt(2);
	// Check for a call instruction: is RHS x + (y * z)?
	SemStr* pRHS = pRT->getRHS();
	if ((pRHS->getFirstIdx() == idPlus) &&
		(pRHS->getSubExpr(1)->getFirstIdx() == idMult))
			// Yes, a call; never anulled
			return false;
	// Check for jmpl: no ternary
	if (pRHS->getFirstIdx() != idTern)
		// Yes, a jmpl; never anulled
		return false;
	// Get a pointer to the second subexpression (the first "?" exp)
	SemStr* pExp = pRHS->getSubExpr(1);
	// Check to see if this subexpression is also a ternary (uncond)
	if (pExp->getFirstIdx() == idTern)
	{
		// This is an unconditional branch, most likely BA
		// The (a = 0) expression is the conditional part of the main
		// ternary
		pExp = pRHS->getSubExpr(0);
		assert(pExp->getFirstIdx() == idEquals);
		assert(pExp->getSubExpr(1)->getFirstIdx() == idIntConst);
		assert(pExp->getSubExpr(1)->getSecondIdx() == 0);
		bool bRet =  pExp->getSubExpr(0)->getSecondIdx() == 1;
		delete pExp;
		return bRet;
	}
	else
	{
		// This is a conditional branch, e.g. BE
		// The (a = 1) is the conditional part of the ternary in the
		// third subexpression of the main ternary
		SemStr* pExp2 = pRHS->getSubExpr(2);		// ":" expr of main ternary
		assert(pExp2->getFirstIdx() == idTern);
		pExp = pExp2->getSubExpr(0);
		assert(pExp->getFirstIdx() == idEquals);
		assert(pExp->getSubExpr(1)->getFirstIdx() == idIntConst);
		assert(pExp->getSubExpr(1)->getSecondIdx() == 1);
		bool bRet = pExp->getSubExpr(0)->getSecondIdx() == 1;
		delete pExp; delete pExp2;
		return bRet;
	}
}

/*==============================================================================
 * FUNCTION:      HRTL::getNonCti
 * OVERVIEW:      Another sparc specific instruction. Return that part of the
 *                given RTL that is not branch related, i.e. that which assigns
 *                pc or npc. In terms of "A Transformational Approach", this
 *                means getting an RTL representing Ic (or Ic')
 * PARAMETERS:    <none>
 * RETURNS:       A copy of this RT with just the side effects in it
 *============================================================================*/
HRTL& HRTL::getNonCti() const
{
	HRTL* pRes = new RTL();
	for (int i=0; i < getNumRT(); i++)
	{
		RTAssgn* pRT = (RTAssgn*)this->elementAt(i);
		if (pRT->getKind() == RTASSGN)
		{
			SemStr* pLHS = pRT->getLHS();
			int idx = pLHS->getFirstIdx();
			if ((idx == idPC) || (idx == idNPC))
				continue;			// Don't copy these
			if ((idx == idRegOf) && (pLHS->getThirdIdx() == 0))
				continue;			// Assign to %g0: ignore these
		}
		pRes->appendRT(pRT);
	}
	return *pRes;
}

/*==============================================================================
 * FUNCTION:      HRTL::getOutAddr
 * OVERVIEW:      Get the source address for the indicated out edge (either 0
 *                or 1) 0 is for branch taken; 1 for not taken. This RTL should
 *                be known to represent a jump or call insruction
 * NOTE:		  Basically obsolete
 * PARAMETERS:    idx: As above
 * RETURNS:       Native address as above
 *============================================================================*/
ADDRESS HRTL::getOutAddr(int idx) const 
{
	if (idx == 1) {
		// We want the branch not taken case.
		// Just return PC+8 (4 for the branch instr, 4 for
		// the delay slot instruction).
		return getAddress() + 8;
	}
	assert(rtlist.size() == 3);		// Should be branch, call, or jmpl
	// Get a pointer to the third RT
	const RTAssgn* pRT = (const RTAssgn*)this->elementAt(2);
	assert(pRT->getKind() == RTASSGN);
	// Get the expression on the RHS
	SemStr* pExp = pRT->getRHS();
	
	// If this is a branch, it should be a ternary like %ZF=1 ? A : B
	// for conditional branches, or
	// a=0 ? ((1 = 1) ? A : B) : ((1=1) ? C : D)  for unconditionals
	if (pExp->getFirstIdx() == idTern)
	{
		SemStr* pTemp = pExp->getSubExpr(1);
		if (pTemp->getFirstIdx() == idTern)
		{
			// An unconditional branch. Just return the dest in A
			pExp = pTemp->getSubExpr(1);
		}
		else
		{
			// Conditional. Looks like (%ZF = 1) ? A : B where
			// A is t[tmp] + (4 * int imm22)
			pExp = pTemp;
		}
		assert(pExp->getFirstIdx() == idPlus);
		// Get offset: should be (4 * imm22)
		pExp = pExp->getSubExpr(1);		
		assert(pExp->getFirstIdx() == idMult);
		// Get second part of C, should be idConst imm22
		pExp = pExp->getSubExpr(1);
		assert(pExp->getFirstIdx() == idIntConst);
		// The value we want is PC + 4 * const
		ADDRESS uRet = (ADDRESS) getAddress() + 4 * pExp->getSecondIdx();
		delete pTemp;
		return uRet;
	}
	else
	{
		// This may be a call or other form of jmpl. The address
		// required is just the successor
		return getAddress() + 8;
	}
}

/*==============================================================================
 * FUNCTION:      HRTL::isCCAffected
 * OVERVIEW:      Return true if this RTL affects the condition codes
 * PARAMETERS:    None
 * RETURNS:       Boolean as above
 *============================================================================*/
bool HRTL::isCCAffected() const
{
	if (rtlist.size() == 0) return false;
	// Get an iterator to the last RT
	RT_CIT it = rtlist.end(); it--;
	// If it is an RTFLAGCALL, then the CCs are affected
	return (*it)->getKind() == RTFLAGCALL;
}

/*==============================================================================
 * FUNCTION:      HRTL::isCompare
 * OVERVIEW:      Returns true if this RTL is a compare instruction
 * PARAMETERS:    Two reference parameters; see RETURNS
 * RETURNS:       Returns true if a compare instruction. If so, sets iReg to
 *					the register involved in the compare, and pTerm to a
 *					SS pointer to the thing being compared to
 *============================================================================*/
// Is this a compare instruction? If so, set the register and a pointer
// to the semantic string for the reg_or_imm in the parameters.
// Epect RTL of the form
// r[rd] := r[rs1] - reg_or_imm
// SUBFLAGS(...)
bool HRTL::isCompare(int& iReg, SemStr*& pTerm)
{
	if (getNumRT() != 3) return false;
	RTAssgn* pRT = (RTAssgn*)this->elementAt(1);
	if (pRT->getKind() != RTASSGN) return false;
	SemStr* pExp = pRT->getRHS();
	// Expect the compare pattern
	static int arCMP[] = {idMinus, idRegOf, idIntConst, -1, -1 /* subexp */};
	if (!pExp->isArraySimilar(sizeof(arCMP) / sizeof(int), arCMP))
		return false;
	// Expect %g0 as the destination
	int arG0[] = {idRegOf, idIntConst, 0};
	if (!pRT->getLHS()->isArrayEqual(sizeof(arG0) / sizeof(int), arG0))
		return false;
	// We have a compare. Just pick out the required items
	iReg = pExp->getIndex(3);
	pTerm = pExp->getSubExpr(1);
	return true;
}


/*==============================================================================
 * FUNCTION:      HRTL::allSimplify
 * OVERVIEW:      Perform machine dependent, and standard optimisations.
 * PARAMETERS:    None
 * RETURNS:       Nothing
 *============================================================================*/
// Do a machine dependent simplification. For sparc, this involves
// replacing r[0] with 0 on the right hand side.
void HRTL::allSimplify()
{
	list<RT*>::iterator it;
	for (it = rtlist.begin(); it != rtlist.end(); ) {
		if ((*it)->getKind() != RTASSGN) {it++; continue;}
		RTAssgn* pRT = (RTAssgn*) *it;
#if 0
		// Remove any RTAssgns with a destination of r[0]
		if ((pRT->getFirstIdx() == idIntConst) &&
			(pRT->getSecondIdx()== 0))
			// Assign to %g0. Remove the whole RT
			rtlist.erase(it);
#endif
        // Simplify the guard
        if (pRT->simplifyGuard()) {
            // Returned true; this means the whole RT can be deleted
            // Ensure that iterator it remains valid
            it = rtlist.erase(it);
            continue;
        }

		// Now attempt to simplify the LHS and RHS.
		// Actually, for Sparc, don't want to machine simplify all LHSs,
        // since this would lead to compares looking like "0 := r[8] - 5".
        // But we want to fix things like "m[%g0 + %o4] := exp"
		// pRT->getLHS()->machineSimplify();
        SemStr* lhs = pRT->getLHS();
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
 *					instruction. Will be 0 for all instructions except
 *					prologues and epilogues.
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
 *					(e.g. could be a two instruction idiom or logue, etc)
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
 *					Replaces r[0] with 0. Could use searchAndReplaceAll now
 * PARAMETERS:    None
 * RETURNS:       Nothing
 *============================================================================*/
bool SemStr::machineSimplify()
{
	SSIT it = indices.begin();
	return doMachSimplify(it);
}

/*==============================================================================
 * FUNCTION:      SemStr::doMachSimplify
 * OVERVIEW:      Does the work for MachineSimplify()
 * PARAMETERS:	  it: iterator to the current index
 * RETURNS:       Nothing, but iterator is incremented
 *============================================================================*/
bool SemStr::doMachSimplify(SSIT& it)
{
	if (it == indices.end()) return false;
	SSIT itOrig = it;
	it = itOrig;
	// Replace `r[ int 0' with `int 0'
	if (*it++ == idRegOf && *it++ == idIntConst && *it++ == 0)
	{
		// Remove the r[
		indices.erase(itOrig);
		return true;			// Can't simplify more than that!
	}
	// Skip the fixed and integer args, then recurse for each variable arg.
	it = itOrig;
	int idx = *it++;
	for (int i=0; i < theSemTable[idx].iNumFixedArgs; i++) it++;
	for (int i=0; i < theSemTable[idx].iNumIntArgs; i++) it++;
    bool ret = false;
	for (int i=0; i < theSemTable[idx].iNumVarArgs; i++)
		ret |= doMachSimplify(it);
    return ret;
}

