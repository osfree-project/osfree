/*==============================================================================
 * FILE:       front386.cc
 * OVERVIEW:   This file contains routines to manage the decoding of 386
 *             instructions and the instantiation to RTLs. These functions
 *             replace Frontend.cc for decoding 386 instructions.
 *
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 *============================================================================*/

/*
 * $Revision: 1.5 $
 * 21 Oct 98 - Mike: converted from frontsparc.cc
 * 30 Oct 98 - Mike: fixed "new HRTLList" problem
 * 03 Nov 98 - Mike: added fetch4()
 * 15 Dec 98 - Mike: moved decode() to driver.cc
 * 22 Jan 99 - Mike: newBB() takes pRtls now
 * 27 Jan 99 - Mike: Use COMPJUMP and COMPCALL now
 * 29 Jan 99 - Mike: Removed use of low level GetOutAddr(); Don't put original
 *              RTL in last inst of BB, only side effects
 * 08 Feb 99 - Mike: library processing moved from processProc() to decodeProc()
 * 16 Feb 99 - Mike: HLJconds take an int instead of a SemStr condition. Also
 *              recognise 386 branches
 * 18 Feb 99 - Mike: integrated with the interpreter
 * 03 Mar 99 - Mike: isSwitch() and processSwitch() take an iOffset now
 * 09 Mar 99 - Mike: Removed interpreter
 * 11 Mar 99 - Mike: Pass to remove FPUSH/FPOP
 * 19 Mar 99 - Mike: processStsw() to handle f(n)stsw instructions, and those
 *              that follow to a branch or set instruction
 * 24 Mar 99 - Mike: handle sahf in floating point compare and jump
 * 24 Mar 99 - Mike: no longer initialise idPC, idZF, etc
 * 07 Apr 99 - Doug: part way through converting to CSR format
 * 29 Apr 99 - Mike: Mods for the depth first procedure decoding model
 * 14 May 99 - Mike: Added third param to some calls to addOutEdge. Was not
 *                      setting all required labels
 * 17 May 99 - Mike: Mods to catch up with sparc changes
 * 27 May 99 - Mike: Fixed JCOND_HRTL case
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 20 Aug 99 - Mike: Don't add register calls to the set of call sites
 * 09 Sep 99 - Mike: Fine tuned the check for existing BB code at the end of the
 *              main loop of processProc()
 * 09 Sep 99 - Mike: changed the way iterators were used to insert and delete;
 *              fixes endless loop in vi (2.6) procedure _isencrypt()
 * 10 Sep 99 - Mike: Added state 26 for and 45 / je (found in vortex)
 * 13 Sep 99 - Mike: Fixed endless loops with "Problem with XXX" message
 *              Also fixed nasty bug with code that makes fall through BBs ahead
 *              of labels (and hence never decode instructions twice)
 * 27 Sep 00 - Mike: Fixed main loop of FrontEndSrc::processProc() to not
 *              increment the main iterator (it) when BBs are joined
 * 28 Sep 00 - Mike: Fixed register replace logic (when flattening stack based
 *              floating point code); now traverses Cfg properly
 * 29 Sep 00 - Mike: Fixed an obscure bug whereby a BB with a JP and another
 *              branch following it would have the float adjustment done twice
 * 09 Nov 00 - Cristina: Added support to output rtl code to a file
 * 15 Mar 01 - Mike: Reset the floting point stack index pointer after calls;
 *				solves the problem of functions returning floats in different
 *				registers after different calls (pushes and pops not balanced)
 * 20 Mar 01 - Mike: Substitute ALL parameters of SETFFLAGS for r[32-39]
 * 26 Mar 01 - Mike: SETFFLAGS register substitution was off by 32
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 * 19 Oct 01 - Mike: Removed simpliy of SSs from processFloatCode; this is done
 *              soon after in driver.cc
 * 25 Oct 01 - Mike: pass helperFunc to the parent class's processProc
 * 31 Oct 01 - Mike: Fixed segfault in helperFunc if SymbolByAddress returns 0
*/

#include "global.h"
#include "frontend.h"
#include "decoder.h"        // prototype for decodeInstruction()
#include "rtl.h"
#include "cfg.h"
#include "ss.h"
#include "prog.h"           // For findProc()
#include "proc.h"
#include "options.h"
#include "BinaryFile.h"     // For SymbolByAddress()

/*==============================================================================
 * Forward declarations.
 *============================================================================*/

/**
 * Process an F(n)STSW instruction.
 */
bool processStsw(HRTLList_IT& rit, HRTLList* pRtls, PBB pBB, Cfg* pCfg);

/**
 * Emit a set instruction.
 */
void emitSet(HRTLList* pRtls, HRTLList_IT& itRtl, ADDRESS uAddr, SemStr* pLHS,
    int iNumIdx, ...);

/**
 * Handle the case of being in state 23 and encountering a set instruction.
 */
void State25(SemStr* pLHS, SemStr* pRHS, HRTLList* pRtls, HRTLList_IT& rit,
    ADDRESS uAddr);

/**
 * Some static globals (saves looking them up each time they are needed).
 */
int idPF = -1;              // Parity flag

/*
 * Process a BB and its successors for floating point code
 */
void processFloatCode(PBB pBB, int& tos, Cfg* pCfg);

/*
 * Check a HLCall for a helper function, and replace with appropriate
 *  semantics if possible
 */
bool helperFunc(ADDRESS dest, ADDRESS addr, HRTLList* lrtl);

#define FSW 40              // Numeric registers
#define AH 12

/*==============================================================================
 * File globals.
 *============================================================================*/

//static int arCallSelf[7] = {idPlus, idPlus, idPC, idIntConst, 5,
//  idIntConst, 0};
static int arrJP[4] = {idEquals, idPF, idIntConst, 1};
static int arrFltGt[5] = {idAnd, idNot, idCF, idNot, idZF};

// Queues used by various functions
queue < ADDRESS> qLabels;       // Queue of labels this procedure

/*==============================================================================
 * Forward declarations.
 *============================================================================*/
void initCti();             // Imp in cti386.cc

/*==============================================================================
 * FUNCTION:      initFront
 * OVERVIEW:      Initialise the front end.
 * PARAMETERS:    <none>                
 * RETURNS:       <nothing>
 *============================================================================*/
struct SemCmp
{ bool operator() (const SemStr& ss1, const SemStr& ss2) const; };
bool SemCmp::operator() (const SemStr& ss1, const SemStr& ss2) const
{ return ss1 < ss2; }
    
// A map from semantic string to integer (for identifying branch statements)
static map<SemStr, int, SemCmp> condMap;

static int arrNotZF[2] = {idNot, idZF};
static int arrDecrementAh[8] = {idMinus, idSize, 8, idRegOf, idIntConst, 12,
    idIntConst, 1};

int arrConds[13][7] = {
{ idZF},                                        // HLJCOND_JE: Z
{ idNot, idZF},                                 // HLJCOND_JNE: ~Z
{ idBitXor, idNF, idOF},                        // HLJCOND_JSL: N ^ O
{ idBitOr, idBitXor, idNF, idOF, idZF},         // HLJCOND_JSLE: (N ^ O) | Z
{ idNot, idBitXor, idNF, idOF },                // HLJCOND_JGES: ~(N ^ O)
{ idBitAnd, idNot, idBitXor, idNF, idOF,
    idNot, idZF},                               // HLJCOND_JSG: ~(N ^ O) & ~Z
{ idCF},                                        // HLJCOND_JUL: C
{ idBitOr, idCF, idZF},                         // HLJCOND_JULE: C | Z
{ idNot, idCF},                                 // HLJCOND_JUGE: ~C
{ idBitAnd, idNot, idCF, idNot, idZF},          // HLJCOND_JUG: ~C & ~Z
{ idNF},                                        // HLJCOND_JMI: N
{ idNot, idNF},                                 // HLJCOND_JPOS: ~N
{ idPF}                                         // HLJCOND_JPAR: P
};

// Ugly. The lengths of the above arrays.
int condLengths[13] = {1, 2, 3, 5, 4, 7, 1, 3, 2, 5, 1, 2, 1};

// Initialise the front end
void initFront()
{
    idPF  = theSemTable.findItem("%PF");

    // Fix up the array of conditionals. Ugh!
    arrJP[1] = idPF;                            // Set the real %PF index
    arrConds[12][0] = idPF;

    for (int i=0; i < 13; i++)
    {
        SemStr ss;
        ss.pushArr(condLengths[i], arrConds[i]);
        condMap[ss] = i;
    }
}

// Array for %ah[0:], i.e. @ r[ int 12 int 7 int 7
static int arrAhAtSeven[8] = { idAt, idRegOf, idIntConst, 12, idIntConst, 7,
    idIntConst, 7};

// Get the condition, given that it's something like %NF ^ %OF
JCOND_TYPE getCond(const SemStr* pCond)
{
    map<SemStr, int, SemCmp>::const_iterator mit;

    mit = condMap.find(*pCond);
    if (mit == condMap.end())
    {
        ostrstream os;
        os << "Condition ";
        pCond->print(os);
        os << " not known";
        error(os.str());
        return (JCOND_TYPE) 0;
    }
    return (JCOND_TYPE) (*mit).second;
}

/*==============================================================================
 * FUNCTION:      FrontEndSrc::processProc
 * OVERVIEW:      Process a procedure, given a native (source machine) address.
 * PARAMETERS:    address - the address at which the procedure starts
 *                pProc - the procedure object
 *                spec - true if a speculative decode
 *                os - output stream for rtl output
 * RETURNS:       True if successful decode
 *============================================================================*/
bool FrontEndSrc::processProc(ADDRESS uAddr, UserProc* pProc, ofstream &os,
    bool spec /* = false */)
{
    // Call the base class to do most of the work
    // Pass the address of our helperFunc function, to check for pentium
    // specific helper functions
    if (!FrontEnd::processProc(uAddr, pProc, os, spec, helperFunc))
        return false;

    // Need a post-cfg pass to remove the FPUSH and FPOP instructions,
    // and to transform various code after floating point compares to
    // generate floating point branches.
    // processFloatCode() will recurse to process its out-edge BBs (if not
    // already processed)
    Cfg* pCfg = pProc->getCFG();
    pCfg->unTraverse();
    // This will get done twice; no harm
    pProc->setEntryBB();
	int tos = 0;
    processFloatCode(pProc->getEntryBB(), tos, pCfg);

    return true;
}

/*==============================================================================
 * FUNCTION:      processFloatCode
 * OVERVIEW:      Process a basic block, and all its successors, for floating
 *                  point code. Remove FPUSH/FPOP, instead decrementing or
 *                  incrementing respectively the tos value to be used from
 *                  here down. Note: tos has to be a parameter, not a global,
 *                  to get the right value at any point in the call tree
 * PARAMETERS:    pBB: pointer to the current BB
 *                tos: reference to the value of the "top of stack" pointer
 *				    currently. Starts at zero, and is decremented to 7 with
 *					the first load, so r[39] should be used first, then r[38]
 *					etc. However, it is reset to 0 for calls, so that if a
 *					function returns a float, the it will always appear in r[32]
 * RETURNS:       <nothing>
 *============================================================================*/
void processFloatCode(PBB pBB, int& tos, Cfg* pCfg)
{
    HRTLList_IT rit;
    RT* pRT;

    // Loop through each RTL this BB
    HRTLList* BB_rtls = pBB->getHRTLs();
    if (BB_rtls == 0) {
        // For example, incomplete BB
        return;
    }
    rit = BB_rtls->begin();
    while (rit != BB_rtls->end()) {
		// Check for call.
		if ((*rit)->getKind() == CALL_HRTL) {
			// Reset the "top of stack" index. If this is not done, then after
			// a sequence of calls to functions returning floats, the value will
			// appear to be returned in registers r[32], then r[33], etc.
			tos = 0;
		}
        if ((*rit)->getNumRT() == 0) {rit++; continue;}
        // Check for f(n)stsw
        if (((*rit)->elementAt(0)->getKind() == RTASSGN) &&
          ((RTAssgn*)(*rit)->elementAt(0))->getRHS()->isNumRegCont(FSW)) {
            // Check the register - at present we only handle AX
            SemStr* pLHS = ((RTAssgn*)(*rit)->elementAt(0))->getLHS();
            assert (pLHS->getSecondIdx() == idIntConst);
            assert (pLHS->getThirdIdx() == 0);      // AX=0

            // Process it
            if (processStsw(rit, BB_rtls, pBB, pCfg)) {
                // If returned true, must abandon this BB.
                break;
            }
            // Else we have already skiped past the stsw, and/or any
            // instructions that replace it, so process rest of this BB
            continue;
        }
        for (int i=0; i < (*rit)->getNumRT(); i++) {
            // Get the current RT
            pRT = (*rit)->elementAt(i);
            if (pRT->getKind() == RTASSGN) {
                // We are interested in either FPUSH/FPOP, or r[32..39]
                // appearing in either the left or right hand sides, or calls
                RTAssgn* pRTA = (RTAssgn*)pRT;
                SemStr* pLHS = pRTA->getLHS();
                SemStr* pRHS = pRTA->getRHS();
                if ((pLHS->len() == 0) && (pRHS->len() == 1)) {
                    // Expect FPUSH/FPOP
                    int id = pRHS->getFirstIdx();
                    if (id == idFpush)
                        tos = (tos - 1) & 7;
                    else if (id == idFpop)
                        tos = (tos + 1) & 7;
                    else continue;
                    // Remove the FPUSH/FPOP
                    (*rit)->deleteRT(i);
                    i--;            // Adjust the index
                    continue;
                }
                else if (tos != 0) {
                    // Substitute all occurrences of r[x] (where
                    // 32 <= x <= 39) with r[y] where
                    // y = 32 + (x + tos) & 7
                    pLHS->bumpRegisterAll(32, 39, tos, 7);
                    pRHS->bumpRegisterAll(32, 39, tos, 7);
                }

            }
            else if (pRT->getKind() == RTFLAGCALL) {
                // We are interested in any register parameters in the range
                // 32 - 39
                RTFlagCall* pRTF = (RTFlagCall*)pRT;
                list<SemStr*>::iterator pp;
                for (pp = pRTF->actuals.begin(); pp != pRTF->actuals.end();
                  pp++) {
                    int i3 = (*pp)->getThirdIdx();
                    if (((*pp)->getFirstIdx() == idRegOf) &&
                      ((*pp)->getSecondIdx() == idIntConst) &&
                      ((i3 >= 32) && (i3 <= 39)))
                            // Change the parameter to the RTFlagCall
                            (*pp)->substIndex(2, 32 + (i3 - 32 + tos & 7));
                }
            }
        }
        rit++;
    }
    pBB->setTraversed(true);

    // Now recurse to process my out edges, if not already processed
    const vector<PBB>& outs = pBB->getOutEdges();
    unsigned n;
    do {
        n = outs.size();
        for (unsigned o=0; o<n; o++) {
            PBB anOut = outs[o];
            if (!anOut->isTraversed()) {
                processFloatCode(anOut, tos, pCfg);
                if (outs.size() != n)
                    // During the processing, we have added or more likely
                    // deleted a BB, and the vector of out edges has changed.
                    // It's safe to just start the inner for loop again
                    break;
            }
        }
    } while (outs.size() != n);            
}



//
// Finite state machine for recognising code handling floating point CCs
//
//                               Start=0
//          ___and_45____________/ |  \  \______sahf____________
//        /                        |   \_____and_5__________    \     ___ 
//       [1]__________cmp_1_      and 44                    \    \   /   |jp
//cmp_40/||\\___dec_[10]      \    [2]                     [3]   [23]____|
//  /    | \\__je_    \cmp 40 [20]    \xor 40              / |    | \ 
// [4]  jne se    \    \       |\      [7]                /  |    |  \ 
// | \   |   \    |    [11]  jne \      | \              se  |   jx   sx
// je se  \   \   | jae|  \sb  \  se   jne setne        /   jne   |    \ 
// |   \   \   \   \   |   \    \  \    |    \         /     |    |     \ 
//[5]  [6][14][13][26][12] [15][21][22][8]   [9]     [18]   [19] [24]   [25]
//JE   SE  JLE  SG JG  JG  SLE JGE  SL JNE   SNE     SGE     JL  Many   Many

/*==============================================================================
 * FUNCTION:      processStsw
 * OVERVIEW:      Process a stsw instruction
 * PARAMETERS:    rit: iterator to the current RTL (with the stsw in it)
 *                BB_rtls: list of RTLs for this BB
 *                pCfg: pointer to Cfg for this proc
 * NOTE:          parameter rit may well be modified (incrementing it past the
 *                  STSW, and any instructions replacing it
 * RETURNS:       True if the current BB is deleted (because 2 BBs were joined)
 *                  Also returns true on error, so abandon this BB
 *============================================================================*/
bool processStsw(HRTLList_IT& rit, HRTLList* BB_rtls, PBB pBB, Cfg* pCfg)
{
    int state = 0;              // Start in state 0
    int n;
    // Keep a list of iterators representing 
    // RTLs (this BB) that can be removed
    list<HRTLList_IT> liIt;
    liIt.push_front(rit);   // f(n)stsw can be removed
    // Scan the rest of the RTLs this BB
    HRTLList_IT rit2 = rit;      // Don't disturb curr loops
    // Scan each RTL this BB
    for (rit2++; rit2 != BB_rtls->end(); rit2++)
    {
//cout << "State now " << dec << state << endl;
        // Get the first RT; only interested in assigns
        if ((*rit2)->getNumRT() == 0)
            continue;
        RTAssgn* pRT = (RTAssgn*)((*rit2)->elementAt(0));
        if (pRT->getKind() != RTASSGN) continue;
        // May need pLHS and uAddr later when reconstructing this SET instr
        ADDRESS uAddr = (*rit2)->getAddress();
        SemStr* pLHS = pRT->getLHS();
        SemStr* pRHS = pRT->getRHS();
        // Check if assigns to and uses register ah
        if (pLHS->isNumRegCont(AH) &&
            pRHS->isNumRegCont(AH))
        {
            // Should be an AND or XOR instruction
            int idx = pRHS->getFirstIdx();
            if ((idx == idBitAnd) || (idx == idBitXor))
            {
                SemStr exp;
                pRHS->getSubExpr(1,exp);
                if (exp.getFirstIdx() == idIntConst)
                {
                    if (idx == idBitAnd)
                    {
                        int mask = exp.getSecondIdx();
                        if (state == 0 && mask == 0x45)
                        {
                            state = 1;
                            liIt.push_front(rit2);
                        }
                        else if (state == 0 &&
                            mask == 0x44)
                        {
                            state = 2;
                            liIt.push_front(rit2);
                        }
                        else if (state == 0 &&
                            mask == 0x05)
                        {
                            state = 3;
                            liIt.push_front(rit2);
                        }
                        else {error ("Problem with AND"); return true;}
                    }
                    else
                    {
                        int mask = exp.getSecondIdx();
                        if (state == 2 && mask == 0x40)
                        {
                            state = 7;
                            liIt.push_front(rit2);
                        }
                        else {error ("Problem with xor"); return true;}
                    }
                }
            }
            else cout << "! Unexpected idx!\n";
        }
        // Or might be a compare or decrement: uses ah, assigns to
        // temp register
        else if ((pLHS->getFirstIdx() == idRegOf) &&
            (pLHS->getSecondIdx() == idTemp) &&
            pRHS->isNumRegCont(AH))
        {
            // Might be a compare, i.e. subtract
            if (pRHS->getFirstIdx() == idMinus)
            {
                SemStr exp;
                pRHS->getSubExpr(1,exp);
                if (exp.getFirstIdx() == idIntConst)
                {
                    int mask = exp.getSecondIdx();
                    if (state == 1 && mask == 0x40)
                    {
                        state = 4;
                        liIt.push_front(rit2);
                    }
                    else if (state == 10 &&
                        mask == 0x40)
                    {
                        state = 11;
                        liIt.push_front(rit2);
                    }
                    else if (state == 1 &&
                        mask == 1)
                    {
                        state = 20;
                        liIt.push_front(rit2);
                    }
                    else {error ("Problem with cmp"); return true;}
                }
            }
            // Check for decrement; RHS of next RT will be r[12]{8} - 1
            else 
            {
                if (((*rit2)->getNumRT() == 3) &&
                    ((*rit2)->elementAt(1))->getKind() == RTASSGN &&
                    ((RTAssgn*)((*rit2)->elementAt(1)))->getRHS()
                        ->isArrayEqual(8, arrDecrementAh))
                {
                    if (state == 1)
                    {
                        state = 10;
                        liIt.push_front(rit2);
                    }
                    else {error ("Problem with decrement"); return true;}
                }
            }
        }
        // Check for SETX, i.e. <exp> ? 1 : 0
        // i.e. ?: <exp> int 1 int 0
        // LHS must be a register
        else if ((pLHS->getFirstIdx() == idRegOf) &&
            (pLHS->getSecondIdx() == idIntConst) &&
            (pRHS->getFirstIdx() == idTern) &&
            (n = pRHS->len(),
             (pRHS->getIndex(n-4) == idIntConst) &&
             (pRHS->getIndex(n-3) == 1) &&
             (pRHS->getIndex(n-2) == idIntConst) &&
             (pRHS->getIndex(n-1) == 0)
            ))
        {
            if (state == 23)
            {
                state = 25;
                // Don't add the set instruction until after the instrs
                // leading up to here are deleted. Else have problems with
                // iterators
            }
            else
            {
                // Check the expression
                SemStr exp;
                pRHS->getSubExpr(0,exp);
                if (exp.getFirstIdx() == idZF)
                {
                    if (state == 4) state = 6;
                    else if (state == 1) state = 13;
                    else if (state == 3) state = 18;
                    else if (state == 20) state = 22;
                    else {error ("Problem with SETE"); return true;}
                }
                else if (exp.getFirstIdx() == idCF)
                {
                    if (state == 11) state = 15;
                    else {error ("Problem with SETB"); return true;}
                }
                else if (exp.isArrayEqual(2, arrNotZF))
                {
                    if (state == 7) state = 9;
                    else {error("Problem with SETNE"); return true;}
                }
            }
        }
        // Check for sahf instr, i.e.
        // @ 7 7 r[ int 12
        else if (pRHS->isArrayEqual(8, arrAhAtSeven))
        {
            if (state == 0)
            {
                state = 23;
                liIt.push_front(rit2);
            }
            else {error ("Problem with sahf"); return true;}
        }
        // Check for "set" terminating states
        switch (state)
        {
            case 6: case 13: case 15:
            case 22: case 9: case 18: case 25:
            // Remove the set instruction and those leading up to it.
            // The left hand side of the set instruction (modrm) is still
            // in pLHS. It will be needed to build the new set instr below
            // Ditto for uAddr.
            // Must decrement rit to the previous RTL
            // so the next increment will be to an RTL interleaved with
            // the deleted ones (if any; otherwise, the one after the set)
            // Also, when inserting a replacement SET instruction, the correct
            // place is after *rit.
            rit--;
            if (state == 25)
            {
                // Keep a copy of the LHS in a new SS. Otherwise, the same SS
                // will be part of two RTLs, so when they are destroyed, this
                // SS will get deleted twice, causing segfaults.
                pLHS = new SemStr(*pLHS);
            }
            // Keep assigning to rit. In the end, it will point to the next
            // RTL after the erased items
            rit = BB_rtls->erase(rit2);
            while (liIt.size())
            {
                rit = BB_rtls->erase(liIt.front());
                liIt.erase(liIt.begin());
            }
        }
        switch (state)
        {
        case 6:
            // Emit a floating point "set if Z"
            emitSet(BB_rtls, rit, uAddr, pLHS, 1, idFZF);
            return false;
        case 13:
            // Emit a floating point "set if G"
            emitSet(BB_rtls, rit, uAddr, pLHS, 1, idFGF);
            return false;
        case 15:
            // Emit a floating poin2t "set if LE"
            emitSet(BB_rtls, rit, uAddr, pLHS, 3, idOr, idFLF, idFZF);
            return false;
        case 22:
            // Emit a floating point "set if L"
            emitSet(BB_rtls, rit, uAddr, pLHS, 1, idFLF);
            return false;
        case 9:
            // Emit a floating point "set if NE"
            emitSet(BB_rtls, rit, uAddr, pLHS, 2, idNot, idFZF);
            return false;
        case 18:
            // Emit a floating point "set if GE"
            emitSet(BB_rtls, rit, uAddr, pLHS, 3, idOr, idFGF, idFZF);
            return false;
        case 25:
            State25(pLHS, pRHS, BB_rtls, rit, uAddr);
            return false;
        }

    }           // End of for loop (for each remaining RTL this BB)
                            
    // Check the branch
    HLJcond* pJump = (HLJcond*)(*--rit2);
    SemStr* pLHS = 0;
    SemStr* pRHS = 0;
    ADDRESS uAddr;
    PBB pBBnext = 0;
    bool bJoin = false;         // Set if need to join BBs
    if (state == 23)
    {
        HRTL* pRtl;
        RTAssgn* pRT;
        if (pJump->getCond() == HLJCOND_JPAR)
        {
            // Check the 2nd out edge. It should be the false case, and
            // should point to either a BB with just a branch in it (a
            // TWOWAY BB) or one starting with a SET instruction
            const vector<PBB>& v = pBB->getOutEdges();
            pBBnext = v[1];
            if ((pBBnext->getType() == TWOWAY) &&
                (pBBnext->getHRTLs()->size() == 1))
            {
                HLJcond* pJ = (HLJcond*)pBBnext->getHRTLs()->front();
                // Make it a floating branch
                pJ->setFloat(true);
                // Make it a signed branch
                pJ->makeSigned();
                // Special state, so DO delete the JP
                state = -24;
                bJoin = true;       // Need to join since will delete JP
            }
            else if (
                (pRtl = pBBnext->getHRTLs()->front(),
                pRT = (RTAssgn*)(pRtl->elementAt(0)),
                pRT->getKind() == RTASSGN) &&
                (pRHS = pRT->getRHS(),
                pRHS->getFirstIdx() == idTern))
            {
                pLHS = pRT->getLHS();
                uAddr = pRtl->getAddress();
                state = 25;
                // Actually generate the set instruction later, after the
                // instruction leading up to it are deleted. Otherwise have
                // problems with iterators
                bJoin = true;       // Need to join since will delete JP
            }
            else
            {
                ostrstream ost;
                ost << "Problem with JP at " << hex;
                ost << pJump->getAddress();
                ost << ". Does not fall through to branch or set at ";
                ost << pBBnext->getLowAddr();
                error(str(ost));
                return true;
            }
        }                   
        else                // Branch, but not a JP (Jump if parity)
        {
            // Just need to change the branch to a float type
            pJump->setFloat(true);
            // and also make it a signed branch
            pJump->makeSigned();
            state = 24;
        }
    }       // if state == 23
    else if (pJump->getCond() == HLJCOND_JE)
    {
        if (state == 4) state = 5;
        else if (state == 1) state = 26;
        else {error("Problem with JE"); return true;}
    }
    else if (pJump->getCond() == HLJCOND_JNE)
    {
        if (state == 1) state = 14;
        else if (state == 7) state = 8;
        else if (state == 3) state = 19;
        else if (state == 20) state = 21;
        else {error("Problem with JNE"); return true;}
    }
    else if (pJump->getCond() == HLJCOND_JUGE)
    {
        if (state == 11) state = 12;
        else {error("Problem with JAE"); return true;}
    }
    else {error("Problem with branch"); return true;}

    SemStr* pDest;
    switch (state)
    {
    case 5: case 14: case 12:
    case 21: case 8: case 19:
    case 24: case -24: case 25:
    case 26:
        // We can remove the branch and the instructions leading up to it
        // (exception: state 24, don't remove the branch).
        // We must decrement rit now, so that it points to a (hopefully)
        // valid RTL, and when incremented next time around the while
        // loop, it will process any instructions that were interspersed
        // with the ones that will be deleted.
        rit--;
        uAddr = pJump->getAddress();        // Save addr of branch
        pDest = pJump->getDest();           // Save dest of branch
        if (state == 25)
            // As before, keep a copy of the LHS in a new SS.
            pLHS = new SemStr(*pLHS);
        if (state != 24) {
            rit = BB_rtls->erase(rit2);
        }
        while (liIt.size()) {
            rit = BB_rtls->erase(liIt.front());
            liIt.erase(liIt.begin());
        }
        break;
    default:
        ostrstream ost;
        ost << "Error: end of BB in state " << dec;
        ost << state << endl;
        error(str(ost));
        return true;
    }
    // Add a new branch, with the appropriate parameters
    HLJcond* newJump;
    switch (state)
    {
    case 5:         // Jump if equals
        newJump = new HLJcond(uAddr);
        newJump->setDest(pDest);
        BB_rtls->push_back(newJump);
        newJump->setCondType(HLJCOND_JE, true);
        break;
    case 14:        // Jump if less or equals
        newJump = new HLJcond(uAddr);
        newJump->setDest(pDest);
        BB_rtls->push_back(newJump);
        newJump->setCondType(HLJCOND_JSLE, true);
        break;
    case 12:        // Jump if greater
    case 26:        // Also jump if greater
        newJump = new HLJcond(uAddr);
        newJump->setDest(pDest);
        BB_rtls->push_back(newJump);
        newJump->setCondType(HLJCOND_JSG, true);
        break;
    case 21:        // Jump if greater or equals
        newJump = new HLJcond(uAddr);
        newJump->setDest(pDest);
        BB_rtls->push_back(newJump);
        newJump->setCondType(HLJCOND_JSGE, true);
        break;
    case 8:         // Jump if not equals
        newJump = new HLJcond(uAddr);
        newJump->setDest(pDest);
        BB_rtls->push_back(newJump);
        newJump->setCondType(HLJCOND_JNE, true);
        break;
    case 18:        // Jump if less
        newJump = new HLJcond(uAddr);
        newJump->setDest(pDest);
        BB_rtls->push_back(newJump);
        newJump->setCondType(HLJCOND_JSL, true);
        break;
    case 25:
        State25(pLHS, pRHS, BB_rtls, rit, uAddr);
        break;
    }

    if (bJoin) {
        // Need to join BBs, because we have deleted a branch
        // The RTLs for the first are appended to those of the second.
        // Since pBB could well have RTs that have already been adjusted
        // for floating point, and pBBnext probably is just a branch, it's
        // important to get the parameters this way around
        pCfg->joinBB(pBBnext, pBB);
    }

//cout << "Return in state " << dec << state << endl;   // HACK
    return bJoin;           // If joined, abandon this BB
}


// Emit Rtl of the form *8* ssLHS = [ssCondition ? 1 : 0]
// Insert before rit
void emitSet(HRTLList* BB_rtls, HRTLList_IT& rit, ADDRESS uAddr, SemStr* pLHS, int iNumIdx,
    ...)
{
    va_list args;

    int* idx = new int (iNumIdx);
    va_start(args, iNumIdx);
    for (int i=0; i < iNumIdx; i++)
        idx[i] = va_arg(args, int);
    SemStr* pRHS = new SemStr;
    pRHS->push(idTern);
    pRHS->pushArr(iNumIdx, idx);
    pRHS->push(idIntConst); pRHS->push(1);
    pRHS->push(idIntConst); pRHS->push(0);
    RTAssgn* pRT = new RTAssgn(pLHS, pRHS, 8);
    HRTL* pRtl = new RTL(uAddr);
    pRtl->appendRT(pRT);
//cout << "Emit "; pRtl->print(); cout << endl;     // HACK
    // Insert the new RTL before rit
    BB_rtls->insert(rit, pRtl);
}

void State25(SemStr* pLHS, SemStr* pRHS, HRTLList* BB_rtls, HRTLList_IT& rit,
    ADDRESS uAddr)
{
    // Assume this is a set instruction
    SemStr exp;
    pRHS->getSubExpr(0,exp);
    if (exp.getFirstIdx() == idCF)
    {
        // Emit a "floating point set if L
        emitSet(BB_rtls, rit, uAddr, pLHS, 1, idFLF);
    }
    else if (exp.getFirstIdx() == idZF)
    {
        // Emit a floating point "set if Z"
        emitSet(BB_rtls, rit, uAddr, pLHS, 1, idFZF);
    }
    else if (exp.getFirstIdx() == idNot)
    {
        if (exp.getSecondIdx() == idCF)
        {
            // Emit a floating point "set if GE"
            emitSet(BB_rtls, rit, uAddr, pLHS, 3, idOr, idFGF, idFZF);
        }
        else if (exp.getSecondIdx() == idZF)
        {
            // Emit a floating point "set if NZ"
            emitSet(BB_rtls, rit, uAddr, pLHS, 2, idNot, idFZF);
        }
        else {error("Problem with STSW/SET ~"); return;}
    }
    else if (exp.getFirstIdx() == idOr)
    {
        if (exp.getSecondIdx() == idCF &&
            exp.getThirdIdx() == idZF)
        {
            // Emit a floating poin2t "set if LE"
            emitSet(BB_rtls, rit, uAddr, pLHS, 3, idOr, idFLF, idFZF);
        }
        else {error ("Problem with STSW/SET |"); return;}
    }
    else if (exp.isArrayEqual(5, arrFltGt))
    {
        // Emit a floating point "set if G"
        emitSet(BB_rtls, rit, uAddr, pLHS, 1, idFGF);
    }
    else {error("Problem with STSW/SET"); return;}
}

/*==============================================================================
 * FUNCTION:        helperFunc
 * OVERVIEW:        Checks for pentium specific helper functions like __xtol
 *                      which have specific sematics.
 * NOTE:            This needs to be handled in a resourcable way.
 * PARAMETERS:      dest - the native destination of this call
 *                  addr - the native address of this call instruction
 *                  lrtl - pointer to a list of HRTL pointers for this BB
 * RETURNS:         true if a helper function is converted; false otherwise
 *============================================================================*/
bool helperFunc(ADDRESS dest, ADDRESS addr, HRTLList* lrtl)
{
    if (dest == NO_ADDRESS) return false;
    const char* p = prog.pBF->SymbolByAddress(dest);
    if (p == NULL) return false;
    string name(p);
    if (name == "__xtol") {
        // This appears to pop the top of stack, and converts the result to
        // a 64 bit integer in edx:eax. Truncates towards zero
        // r[tmpl] = ftoi(80, 64, r[32])
        // r[24] = trunc(64, 32, r[tmpl])
        // r[26] = r[tmpl] >> 32
        SemStr lhs, rhs;
        int iTmpl = theSemTable.findItem("tmpl");
        lhs << idRegOf << idTemp << iTmpl;
        rhs << idFtoi << 80 << 64 << idRegOf << idIntConst << 32;
        RTAssgn* pRt = new RTAssgn(&lhs, &rhs, 64);
        HRTL* pRtl = new RTL(addr);
        pRtl->appendRT(pRt);
        lhs.clear(); rhs.clear();
        lhs << idRegOf << idIntConst << 24;
        rhs << idTruncu << 64 << 32 << idRegOf << idTemp << iTmpl;
        pRt = new RTAssgn(&lhs, &rhs, 32);
        pRtl->appendRT(pRt);
        lhs.substIndex(2, 26);      // r[24] -> r[26]
        rhs.clear();
        rhs << idShiftR << idRegOf << idTemp << iTmpl << idIntConst << 32;
        pRt = new RTAssgn(&lhs, &rhs, 32);
        pRtl->appendRT(pRt);
        // Append this RTL to the list of RTLs for this BB
        lrtl->push_back(pRtl);
        // Return true, so the caller knows not to create a HLCall
        return true;

    } else {
        // Will be other cases in future
    }
    return false;
}

/*==============================================================================
 * FUNCTION:      fetch4
 * OVERVIEW:      Fetch 4 bytes in little endian format. Needed by the switch
 *                logic. Here because it's source machine specific
 * PARAMETERS:    ptr -
 * RETURNS:       the four byte value at the given location
 *============================================================================*/
unsigned fetch4(unsigned char* ptr)
{
    // We need to read the bytes in little endian format
    return ptr[0] + (ptr[1] << 8) + (ptr[2] << 16) + (ptr[3] << 24);
}

