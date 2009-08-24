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
 * FILE:        expander.cc
 * OVERVIEW:    Implementation of the Expander class.
 *              All machine-independent aspects of the code expansion and
 *              generation are stored here.
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2001, The University of Queensland, BT group
 *============================================================================*/

/*
 * $Revision: 1.8 $
 * 13 Apr 01 - Manel: Initial creation.
 * 16 Apr 01 - Brian: Implemented integer division and signed integer
 *              arithmetic.
 * 08 May 01 - Nathan: Split getExpInstance off into instance.cc
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 * 30 Aug 01 - Mike: Small changes since parameters are lists now (were vectors)
 */

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "global.h"
#include "rtl.h"
#include "cfg.h"
#include "proc.h"
#include "prog.h"
#include "options.h"
#include "expander.h"

/*==============================================================================
 * Expander class implementation.
 *============================================================================*/

/*==============================================================================
 * FUNCTION:    Expander::Expander
 * OVERVIEW:    Constructor for class Expander
 * PARAMETERS:  <none>
 * RETURNS:     <nothing>
 *============================================================================*/
Expander::Expander()
{
    // Initialise constant SemStrs.
    regRefPattern.push(idRegOf);
    regRefPattern.push(idIntConst);
    regRefPattern.push(-1);
    varRefPattern.push(idVar);
    varRefPattern.push(-1);
}

/*==============================================================================
 * FUNCTION:    Expander::~Expander
 * OVERVIEW:    Destructor for class Expander
 * PARAMETERS:  <none>
 * RETURNS:     <nothing>
 *============================================================================*/
Expander::~Expander()
{
    // Do nothing
}

/*==============================================================================
 * FUNCTION:    Expander::findUsedVariables
 * OVERVIEW:    Small proc to check for used registers.
 *              This looks for SemStrs with "r[ int X" and "v[ X".
 *              If found, inserts X into the set of used src/sym registers.
 * NOTES:       Assumes that expressions of the form r[6+1]
 *              have been changed to r[7]
 * PARAMETERS:  ss: SS to search into
 * RETURNS:     <nothing>
 *============================================================================*/
void Expander::findUsedVariables(SemStr* ss)
{
    // Check for source registers
    list<SemStr*> result;
    if (ss->searchAll(regRefPattern, result))
    {
        // We have at least one register; go through the list
        list<SemStr*>::iterator it;
        for (it = result.begin(); it != result.end(); it++)
        {
            // Add to used registers
            regNumbers.insert((*it)->getThirdIdx());

            // Add to typed used registers (no params!).
            if (proc->isParam(*(*it))) continue;
            int rnum = (*it)->getThirdIdx();
            regUsed[rnum] = prog.RTLDict.DetRegMap[rnum].g_type();
        }
    }

    // Check for symbolic registers
    result.clear();
    if (ss->searchAll(varRefPattern, result))
    {
        // We have at least one register; go through the list
        list<SemStr*>::iterator it;
        for (it = result.begin(); it != result.end(); it++)
        {
            if (proc->isParam(*(*it))) continue;
            int vnum = (*it)->getSecondIdx();
            symUsed[vnum] = proc->getVarType(vnum);
        }
    }

}

/*==============================================================================
 * FUNCTION:    Expander::PreProcess
 * OVERVIEW:    Initial function processing (used regs, locals, etc).
 *              Also calls the machine dependent pre-processing.
 * PARAMETERS:  <none>
 * RETURNS:     <nothing>
 *============================================================================*/
void Expander::PreProcess()
{
    // Set some initial values
    stIdx.clear();
    regUsed.clear();
    symUsed.clear();
    globReloc.clear();

    // Overlap info
    regNumbers.clear();
    ovl.init();

    // Get proc CFG, sorting BBs by address
    Cfg* pCfg = proc->getCFG();
    pCfg->sortByAddress();

    // This searches through all RTLs in the procedure
    BB_CIT it;
    for (PBB pBB = pCfg->getFirstBB(it); pBB; pBB = pCfg->getNextBB(it))
    {
        HRTLList* pRtls = pBB->getHRTLs();
        if (pRtls)
        {
            for (HRTLList_IT rit = pRtls->begin(); rit != pRtls->end(); rit++)
            {
                int n = (*rit)->getNumRT();
                for (int i=0; i<n; i++)
                {
                    // Get RT assignments
                    RTAssgn* rt = (RTAssgn*)(*rit)->elementAt(i);
                    if (rt->getKind() != RTASSGN) continue;

                    // Check for used source & simbolic registers
                    findUsedVariables(rt->getLHS());
                    findUsedVariables(rt->getRHS());

                    // Also looks for SemStrs with IDs >= idMachSpec.
                    // If so, adds the index to the set stIdx.
                    rt->getLHS()->searchMachSpec(stIdx);
                    rt->getRHS()->searchMachSpec(stIdx);

                }

                // Process calls
                HRTL_KIND kd = (*rit)->getKind();
                if (kd == CALL_HRTL)
                {
                    // Also count return location, if any
                    HLCall* hl = (HLCall*)*rit;
                    SemStr retLoc = hl->getReturnLoc();
                    findUsedVariables(&retLoc);

                    // Also count parameters; it's possible that some are
                    // not used elsewhere
                    list<SemStr>& pv = hl->getParams();
                    list<SemStr>::iterator pp;
                    for (pp = pv.begin(); pp != pv.end(); pp++)
                        findUsedVariables(&*pp);
                }

                // Process returns
                else if (kd == RET_HRTL)
                {
                    const SemStr* rl = proc->getReturnLoc();
                    // Need const cast because checkRegs can't be made const,
                    // because searchAll isn't const
                    findUsedVariables(const_cast<SemStr*>(rl));
                }

                // Other HL types may have to be considered in the future
            }
        }
    }

    // Set overlap info
    ovl.setUsed(regNumbers);

    // Call machine dependent pre-processing
    specificPreProcess();
}

/*==============================================================================
 * FUNCTION:    Expander::PostProcess
 * OVERVIEW:    Emits final raw data for a given function.
 * PARAMETERS:  <none>
 * RETURNS:     <nothing>
 *============================================================================*/
void Expander::PostProcess()
{
    // Call machine dependent post-processing
    specificPostProcess();
}

/*==============================================================================
 * FUNCTION:    Expander::processExpr
 * OVERVIEW:    Process an SemStr expression.
 * PARAMETERS:  pExp:  The pointer to the Expression
 *              cType: The current type to use for code generation.
 * RETURNS:     A location value, which is an identifier about where
 *              the result of the expression is located.
 *============================================================================*/
Location Expander::processExpr(const SemStr* pExp, Type cType)
{
    Location loc;   // Auxiliar location
    int idx = pExp->getFirstIdx();

    // Null expressions returns invalid location values
    if (idx == WILD)
        return INVALID_LOCATION;

    // Let's deal with machine special IDs as a generic ones
    idx = idx < idMachSpec ? idx : idMachSpec;

    /*
     * More readable switch-case!
     *
     * This is a switch statement based on two different fields:
     * number of arguments (int 0..3) and a semantic string id (int).
     * As C++ does not allow switch statements based on different case
     * selector, this would have been written using nested-switches,
     * which is very unredeable.
     *
     * The solution is to "left-shift" the SS id 2 bits, and do a logical
     * or with the number of arguments (0 to 3) to get the 'key' on the
     * appropiate case.
     */
#define SWITCH(a,b) switch (((a)&0x3)|((b)<<2))
#define CASE(a,b)   case   (((a)&0x3)|((b)<<2))

    // Case on number of arguments, Operator Id.
//cout << "*** numarg:" << nva << ", idx:" << idx << endl;
    SWITCH (theSemTable[idx].iNumVarArgs, idx)
    {
        CASE (0,idIntConst):    // Constant
        CASE (0,idAFP):         // Address of locals
        CASE (0,idVar):         // Symbolic variable
        CASE (0,idMachSpec):    // Machine specific ID
        CASE (1,idRegOf):       // Machine source register
        CASE (1,idAddrOf):      // Address of a location
        {
            // Get where the symbol is located
            loc = getLocation(pExp, cType);
            break;
        }

        // Unary expressions
        CASE (1,idNot):         // Loginal not
        CASE (1,idNeg):         // Unary minus
        CASE (1,idSignExt):     // Sign extension
        CASE (1,idMemOf):       // Raw memory access
        {
            // Process and get subexpression
            Location sloc = processExpr(pExp->getSubExpr(0), cType);
            sloc = fetch(sloc);

            // Get a new location value, and emit operation
            loc = getLocation(pExp, cType);
            emitUnary(loc, sloc);
            break;
        }
        
        // Arithmetics
        CASE (2,idPlus):        // Addition
        CASE (2,idMinus):       // Subtraction
        CASE (2,idMult):        // Multiply
        CASE (2,idMults):       // Multiply signed
        CASE (2,idDiv):         // Integer division
        CASE (2,idDivs):        // Integer division signed
        CASE (2,idMod):         // Modulus
        CASE (2,idMods):        // Modulus signed

        // Logicals
        CASE (2,idBitOr):       // |
        CASE (2,idBitAnd):      // &
        CASE (2,idBitXor):      // ^

        // Shift
        CASE (2,idShiftL):      // Shift left
        CASE (2,idShiftR):      // Shift right
        CASE (2,idShiftRA):     // Shift left (arithmetic)

        // Comparisons
        CASE (2,idEquals):      // ==
        CASE (2,idNotEqual):    // !=
        CASE (2,idLess):        // < (signed)
        CASE (2,idGtr):         // > (signed)
        CASE (2,idLessEq):      // <= (signed)
        CASE (2,idGtrEq):       // >= (signed)
        CASE (2,idLessUns):     // < (unsigned)
        CASE (2,idGtrUns):      // > (unsigned)
        CASE (2,idLessEqUns):   // <= (unsigned)
        CASE (2,idGtrEqUns):    // >= (unsigned)
        {
            // Process and get subexpressions
            Location loc1 = processExpr(pExp->getSubExpr(0), cType);
            loc1 = fetch(loc1);
            Location loc2 = processExpr(pExp->getSubExpr(1), cType);
            loc2 = fetch(loc2);

            // Get a new location value, and emit operation
            loc = getLocation(pExp, cType);
            emitBinary(loc, loc1, loc2);
            break;
        }

        // Conversions
        CASE (1,idZfill):       // Zero extension
        CASE (1,idSgnEx):       // Sign extension
        CASE (1,idTruncs):      // Truncation (signed)
        CASE (1,idTruncu):      // Truncation (unsigned)
        {
            // Get limits
            int from = pExp->getSecondIdx();
            int to   = pExp->getThirdIdx();
            
            // Process and get subexpression
            Location sloc = processExpr(pExp->getSubExpr(0), cType);
            sloc = fetch(sloc);

            // Get a new location value, and emit operation
            loc = getLocation(pExp, cType);
            emitConversion(loc, sloc, from, to);
            break;
        }

        default:
            cout << "Index " << idx << " -- ";
            expHalt("Expander::processExpr: unexpected expression");
    }

    // Return location
    return loc;

    // Undefinitions
#undef SWITCH
#undef CASE
}

/*==============================================================================
 * FUNCTION:    Expander::processRT
 * OVERVIEW:    Parse each statement and call the function processExpr for a RT.
 * PARAMETERS:  pRT: Pointer to RT object to be processed
 * RETURNS:     <nothing>
 *============================================================================*/
void Expander::processRT(RT* pRt)
{
    // Clear all location values
    clearLocations();

    // Kind of RT
    switch (pRt->getKind())
    {
        case RTASSGN:       // Assignment
        {
            RTAssgn* pRtA = (RTAssgn*)pRt;
            SemStr* pLoc = pRtA->getLHS();

            // Initial type -- IT IS CORRECT?
            // If a memOf, use the type of the RHS (because memory is typeless).
            // Otherwise use the type of the LHS.
            Type cType = (pLoc->getFirstIdx() == idMemOf)
                       ? pRtA->getRHS()->getType()
                       : pLoc->getType();

            // Process the RHS
            Location locRHS = processExpr(pRtA->getRHS(), cType);
            locRHS = fetch(locRHS);
            
            // Get location value for the LHS
            Location locLHS = processExpr(pLoc, cType);

            // Final assignment -- LHS := RHS
            emitAssign(locLHS, locRHS);

            break;
        }

        case RTFLAGDEF:     // Flag definition
            expHalt("RTFLAGDEF still not supported");
            break;

        case RTFLAGCALL:    // Flag call -- ignore
            // expHalt("RTFLAGCALL still not supported");
            break;

        default:            // Weird!
            expHalt("Unknown RT");
    }
}

/*==============================================================================
 * FUNCTION:    Expander::processRTL
 * OVERVIEW:    Process a high level RTL.
 * PARAMETERS:  pRTL: Pointer to the HL RTL.
 * RETURNS:     <nothing>
 *============================================================================*/
void Expander::processRTL(const HRTL* pRTL)
{
    // Deal only with uncommented RTLs
    if (!(pRTL->getCommented()))
    {
        // Process each RT of this RTL
        int n = pRTL->getNumRT();
        for (int i = 0; i < n; i++)
            processRT(pRTL->elementAt(i));
    }

    // Clear all location values
    clearLocations();
}

/*==============================================================================
 * FUNCTION:    Expander::processHL
 * OVERVIEW:    Process a high level RTL: HLJump, HLJcond, HLRet, etc.
 * PARAMETERS:  pRTL: Pointer to the HL RTL.
 *              pBB:  Pointer to the basic block to which the RT belongs to.
 * RETURNS:     <nothing>
 *============================================================================*/
void Expander::processHL(const HRTL* pRTL, PBB pBB)
{
    // No control transfer?
    if (pRTL->getKind() == LOW_LEVEL_HRTL)
    {
        // Process RTL list
        processRTL(pRTL);
        return;
    }

    // Clear all location values
    clearLocations();

    // Kinds of HRTL
    switch (pRTL->getKind())
    {
         case CALL_HRTL:      // Call statement
         {
            // Get several information about caller/callee
            const HLCall* pCall = static_cast<const HLCall*>(pRTL);
            ADDRESS uDest = pCall->getFixedDest();
            Proc* target = prog.findProc(uDest);
            const char* pName = target->getName();

            // Process pre-semantic call management
            processRTL(pRTL);

            // No post-semantic call management
            assert(pCall->getPostCallRtlist() == NULL);

            // Additional asserts at the moment...
            if (pName == NULL)
            {
                // No register calls
                assert(uDest != 0);

                // No calls to unnamed functions
                assert((target != 0) &&
                       (target != (Proc*)-1) &&
                       (target->getName() != 0));
            }

            // Process the call parameters
            const list<SemStr>& param = pCall->getParams();
            unsigned nParams = param.size();
            vector<Location> locRHS(nParams);   // Vector of location values
            list<SemStr>::const_iterator pp = param.begin();
            for (unsigned i = 0; i < nParams; i++, pp++)
            {
                // Process each parameter
                Location loc = processExpr(&(*pp), pp->getType());
                locRHS[i] = fetch(loc, true);
            }

            // Process return location
            const SemStr rLoc = pCall->getReturnLoc();
            Location locLHS = processExpr(&rLoc, rLoc.getType());

            // Emits the call -- LHS := call RHS
            emitCall(locLHS, target, locRHS);

            // If the "fall through" outedge doesn't really fall through, the
            // This is used in case there is a call/restore. It is untested!
            if (pBB->isJumpReqd())
            {
                // Jump to target BB
                emitBranch(pBB->getOutEdges()[0]);
            }

            break;
        }

        case RET_HRTL:      // Return statment
        {
            // Preprocess RTL if ret has semantics
            if (pRTL->getNumRT() != 0)
                processRTL(pRTL);

            // Emit the return statement
            const SemStr* rLoc = proc->getReturnLoc();
            Location loc = processExpr(rLoc, rLoc->getType());
            loc = fetch(loc);
            emitReturn(loc);

            break;
        }

        case JUMP_HRTL:       // Jump statement
        {
            // Check if fixed destination. That means check if the
            // destination address is known (fixed) or whether 
            // it is dynamic and will be known at runtime.
            const HLJump* jump = static_cast<const HLJump*>(pRTL); 
            if (jump->getDest()->getFirstIdx() == idIntConst)
            { 
                // Branch to target BB
                emitBranch(pBB->getOutEdges()[0]);
            }
            else
            { 
                // Set global info
                prog.bRegisterJump = true;

                // Eventually, we must call a runtime interpreter.
                // We do not yet handle jumps to dynamic locations:
                // i.e., locations that are only known at runtime.
                expHalt("JUMP_HRTL to unknown target still not supported");
            }

            break;
        }

        case JCOND_HRTL:      // Conditional branch
        {
            const HLJcond* jcond = static_cast<const HLJcond*>(pRTL);
            SemStr* pCond = jcond->getCondExpr();

            // Check for valid condition
            assert(pCond != 0);

            // Set condition sign
            Type cType(pCond->getType());
            switch (jcond->getCond())
            {
                case HLJCOND_JUL:
                case HLJCOND_JULE:
                case HLJCOND_JUGE:
                case HLJCOND_JUG:
                    cType.setSigned(false);
                    break;
                default:
                    // expHalt("HL_Jxx still not supported");
                    break;  // Leave it alone
            }

            // Process the expression of the RHS
            Location locRHS = processExpr(pCond, cType);
            locRHS = fetch(locRHS);
            emitCondBranch(locRHS, pBB->getOutEdges()[0]);

            // If the "fall through" outedge doesn't really fall through, the
            // "jump reqd" bit should be set, and we must generate a jump
            if (pBB->isJumpReqd())
            {
                // Jump to target BB
                emitBranch(pBB->getOutEdges()[1]);
            }

            break;
        }

        case SCOND_HRTL:      // Set condition codes
            expHalt("SCOND_HRTL still not supported");
            break;

        case NWAYJUMP_HRTL:   // Multiway branch
        {
            const HLNwayJump* jump = static_cast<const HLNwayJump*>(pRTL);
            SWITCH_INFO* si = jump->getSwitchInfo();

            // Don't allow interpretation at the moment
            assert(si != NULL);

            // Don't allow hash-like switches at the moment
            assert(si->chForm != 'H');

            // Process decission expression
            Type cType = si->pSwitchVar->getType();
            Location loc = processExpr(si->pSwitchVar, cType);
            loc = fetch(loc);

            // Iterate through the out edges for the cases
            const vector<PBB>& outs = pBB->getOutEdges();
            for (int i = 0; i <= (si->iUpper) - (si->iLower); i++)
            {
                // Emit case i-th
                emitCase(loc, si->iLower + i, outs[i]);
            }

            break;
        }

        default:            // Weird!
            expHalt("Unknown HL RTL");
    }
}

/*==============================================================================
 * FUNCTION:    Expander::expandFunction
 * OVERVIEW:    This is the main expander function.
 *              Expands the HRTL to machine code for this procedure.
 * PARAMETERS:  proc: Pointer to this procedure's Proc object.
 * RETURNS:     <nothing>
 *============================================================================*/
void Expander::expandFunction(UserProc* proc)
{
    // Don't attempt to generate code for null procs
    if (proc->getEntryBB() == NULL)
        return;

    // Tracing
    if (progOptions.proctrace)
        cout << "Back end for proc " << proc->getName() << endl;

    // Set procedure
    this->proc = proc;

    // Pre-process function
    PreProcess();

    // Entry BB *must* be labeled (easier for recursive calls).
    Cfg* pCfg = proc->getCFG();
    PBB eBB = pCfg->getEntryBB();
    if (!eBB->getLabel())
        pCfg->setLabel(eBB);

    // If the first BB is not the entry point...
    BB_CIT bbit;
    PBB pBB = pCfg->getFirstBB(bbit);
    if (proc->getNativeAddress() != pBB->getLowAddr())
    {
        // Jump to the real entry BB
        emitBranch(proc->getEntryBB());
    }

    // Process each BB
    for ( ; pBB != NULL; pBB = pCfg->getNextBB(bbit))
    {
        // Emit label for current basic block
        if (pBB->getLabel())
            emitLabel(pBB);

        // Process each HRTL
        HRTLList_CIT it;
        for (it = pBB->getHRTLs()->begin(); it != pBB->getHRTLs()->end(); it++)
        {
            // Set the address for error messages
            rtlAddr = (*it)->getAddress();
//cout << "\t; " << hex << rtlAddr << endl;

            // Process High level RTs
            processHL(*it, pBB);
        }

        // Fall-through case. Other types handled elsewhere.
        // Problem is knowing which outedge is the fallthrough one
        if ((pBB->getType() == FALL) && (pBB->isJumpReqd()))
        {
            // Jump to target BB
            emitBranch(pBB->getOutEdges()[0]);
        }
    }

    // Post-process function
    PostProcess();
}

/*==============================================================================
 * FUNCTION:    Expander::expError
 * OVERVIEW:    Dumps an error message followed by the source address.
 * PARAMETERS:  messg: Error message to dump
 * RETURNS:     <nothing>
 *============================================================================*/
void Expander::expError(char *messg)
{
    ostrstream ost;
    ost << messg << " at 0x" << hex << rtlAddr;
    error(str(ost));
}

/*==============================================================================
 * FUNCTION:    Expander::expError
 * OVERVIEW:    Dumps an error message followed by the source address and exit.
 * PARAMETERS:  messg: Error message to dump
 * RETURNS:     <nothing>
 *============================================================================*/
void Expander::expHalt(char *messg)
{
    expError(messg);
    exit(1);
}

