/*
 * Copyright (C) 2000, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:          cti68k.cc
 * OVERVIEW:      Part of the implementation of the HRTL class, to do with
 *	              detecting mc68k control transfer instructions
 *	              Some methods are source platform independent, i.e. all
 *                platforms must implement these methods.
 *
 * Copyright (C) 2000, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.6 $
 * 14 Feb 2000 - Mike: Converted from cti386
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 */

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "global.h"
#include "hrtl.h"
#include "ss.h"

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
 * OVERVIEW:      68k returns most function values in r[0] (i.e. %d0)
 * NOTE:          This is a bit of a hack for the switch analysis code.
 *                  It should be replaced with something better
 * PARAMETERS:    <none>
 * RETURNS:       The index of the register where integer values are returned
 *============================================================================*/
int getFuncRetReg()
{
	return 0;
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
	return 0;
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
 * NOTE:		  mc68k has no machine dependent simplifications, at least at
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
 * NOTE:		  There are no such optimisations for mc68k (at present)
 * PARAMETERS:    <none>
 * RETURNS:       <nothing>
 *============================================================================*/
bool SemStr::machineSimplify()
{
    return false;
}



