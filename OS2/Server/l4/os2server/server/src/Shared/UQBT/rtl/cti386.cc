/*
 * Copyright (C) 1998-1999, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:          cti386.cc
 * OVERVIEW:      Part of the implementation of the HRTL class, to do with
 *	              detecting 386 control transfer instructions
 *	              Some methods are source platform independent, i.e. all
 *                platforms must implement these methods.
 *
 * Copyright (C) 1998-1999, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.20 $
 * 16 Oct 98 - Mike: Converted from ctisparc
 * 27 Jan 99 - Mike: Use COMPJUMP and COMPCALL now
 * 29 Jan 99 - Mike: getControlTransferEx always returns a new ss in pDest now
 * 29 Jan 99 - Mike: added getFallthrough()
 * 11 Feb 99 - Mike: getControlTransferEx() drops the "=1" from pCond now
 * 24 Mar 99 - Mike: no longer initialise idPC, idZF, etc
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 */

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "global.h"
#include "hrtl.h"
#include "ss.h"

// Defines for commonly used registers
#define regEsp 28
#define regEbp 29
#define regEax 24

/*==============================================================================
 * Static globals.
 *============================================================================*/

// Three static globals representing %pc etc. The function initCti()
// has to be called to initialise these
static int idTMP1;

/**********************************
 * Global methods
 **********************************/

/*==============================================================================
 * FUNCTION:      initCti
 * OVERVIEW:      Perform any initialisation required for this module
 * PARAMETERS:    <none>
 * RETURNS:       <nothing>
 *============================================================================*/
void initCti()
{
	idTMP1 = theSemTable.findItem("tmp1");
}

/*==============================================================================
 * FUNCTION:      getFuncRetReg
 * OVERVIEW:      386 returns function values in r[24] (i.e. %eax)
 * PARAMETERS:    <none>
 * RETURNS:       The index of the register where integer values are returned
 *============================================================================*/
int getFuncRetReg()
{
	return 24;
}

/**********************************
 * HRTL methods
 **********************************/
/*==============================================================================
 * FUNCTION:      HRTL::getOutAddr
 * OVERVIEW:      Get the source address for the indicated out edge (either 0
 *                or 1) 0 is for branch taken; 1 for not taken. This RTL should
 *                be known to represent a jump or call insruction.
 * NOTE:		  This function is obsolete; it may not work at all since the
 *					number of bytes is not stored in high level instructions
 *					any more.
 * PARAMETERS:    idx -
 * RETURNS:       ???
 *============================================================================*/
ADDRESS HRTL::getOutAddr(int idx) const 
{
	int uNumBytes = this->getNumBytes();

	if (idx == 1)
	{
		// Branch not taken. 
		return getAddress() + uNumBytes;
	}

	if (rtlist.size() == 1)
	{
		// Relative branch
		// Exp  *32* %pc := [(COND1[idx] = 1) ? %pc + k + reloc : %pc];
		// => 	*32* %pc := [(COND1[idx] = 1) ? dest : %pc];
		// or *32* %pc := %pc + k + reloc;
		// => *32* %pc := dest;
		const RTAssgn* pRT = (const RTAssgn*)this->elementAt(0);
		assert(pRT->getKind() == RTASSGN);
		// Get the expression on the RHS
		SemStr* pExp = pRT->getRHS();
		if (pExp->getFirstIdx() == idTern)
			pExp = pExp->getSubExpr(1);
		return pExp->getSecondIdx();		// Skip only idIntConst
	}

	assert(rtlist.size() == 3);		// Should be call
	assert(idx == 0);				// Should be fall through only
	return getAddress() + numNativeBytes;
}

/*==============================================================================
 * FUNCTION:      HRTL::isCompare
 * OVERVIEW:      Is this a compare instruction? If so, set the register and a
 *                pointer to the semantic string for the reg_or_imm in the
 *                parameters. Expect RTL of the form:
 *                   r[ temp`1' := r[reg32] - int k
 *                   SUBFLAGS(...)
 * PARAMETERS:    iReg - returns the register number involved in the compare
 *                pTerm - returns a pointer to the thing being compared with
 * RETURNS:       True if compare detected. Note: two reference parameters are
 *					modified if return true
 *============================================================================*/
bool HRTL::isCompare(int& iReg, SemStr*& pTerm)
{
	if (getNumRT() != 2) return false;
	RTAssgn* pRT = (RTAssgn*)this->elementAt(0);
	if (pRT->getKind() != RTASSGN) return false;
	SemStr* pExp = pRT->getRHS();
	// Expect the compare pattern
	static int arCMP[] = {idMinus, idRegOf, idIntConst, -1, -1 /* subexp */};
	if (!pExp->isArraySimilar(sizeof(arCMP) / sizeof(int), arCMP))
		return false;
	// Expect r[tmp] as the destination
	SemStr* pLHS = pRT->getLHS();
	if (pLHS->getFirstIdx() != idRegOf || pLHS->getSecondIdx() != idTemp)
		return false;
	// We have a compare. Just pick out the required items
	iReg = pExp->getIndex(3);
	pTerm = pExp->getSubExpr(1);
	return true;
}


/*==============================================================================
 * FUNCTION:      HRTL::allSimplify
 * OVERVIEW:      Perform machine dependent, and standard optimisations.
 * NOTE:		  X86 has no machine dependent simplifications, at least at
 *					present, so this merely does the standard optimisations
 * PARAMETERS:    None
 * RETURNS:       Nothing
 *============================================================================*/
// Do a machine dependent simplification. For sparc, this involves
// replacing r[0] with 0 on the right hand side.
void HRTL::allSimplify()
{
	list<RT*>::iterator it;
	for (it = rtlist.begin(); it != rtlist.end(); it++)
	{
		if ((*it)->getKind() != RTASSGN) continue;
		RTAssgn* pRT = (RTAssgn*) *it;

		// Now attempt to simplify the LHS and RHS. LHS could need simplificat-
		// ion, e.g. r[8+1] => r[9]
		pRT->getLHS()->machineSimplify();
		pRT->getRHS()->machineSimplify();

		// Now also standard simplifications, e.g. 0|2 => 2
		pRT->getLHS()->simplify();
		pRT->getRHS()->simplify();
	}
}



/*==============================================================================
 * FUNCTION:      HRTL::updateNumBytes
 * OVERVIEW:      Set the number of bytes for this RTL
 * PARAMETERS:    uNumBytes - number of bytes
 * RETURNS:       <nothing>
 *============================================================================*/
void HRTL::updateNumBytes(unsigned uNumBytes)
{
	numNativeBytes = uNumBytes;
}

/*==============================================================================
 * FUNCTION:      HRTL::getNumBytes
 * OVERVIEW:      Get the number of bytes for the instruction that this RTL
 *					represents
 * PARAMETERS:    <none>
 * RETURNS:       The number of bytes
 *============================================================================*/
unsigned HRTL::getNumBytes() const
{
	return numNativeBytes;
}

/**********************************
 * SemStr methods
 **********************************/

/*==============================================================================
 * FUNCTION:      SemStr::machineSimplify
 * OVERVIEW:      Perform any machine specific optimisations, such as %g0 == 0
 * NOTE:		  There are no such optimisations for X86 (at present)
 * PARAMETERS:    <none>
 * RETURNS:       <nothing>
 *============================================================================*/
bool SemStr::machineSimplify()
{
    return false;
}



