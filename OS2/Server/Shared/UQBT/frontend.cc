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
 * FILE:       frontend.cc
 * OVERVIEW:   This file contains common code for all front ends. The majority
 *				of frontend logic remains in the source dependent files such as
 *				frontsparc.cc
 *
 * Copyright (C) 1999-2001, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.28 $
 * 18 May 99 - Mike: moved visit() to frontend.cc; also nextAddress() and
 *			DecodeRtl()
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 06 Mar 01 - Mike: Removed insertAssign (moved to HRTL as a member fctn)
 * 31 Mar 01 - Mike: getFixedDest() returns NO_ADDRESS for non fixed addresses
 * 02 Apr 01 - Mike: nextAddress returns NO_ADDRESS if no more to process
 * 09 May 01 - Mike: Flush the .rtl stream; cope with inst.rtl == 0
 * 14 May 01 - Mike: Use inst.valid to determine validity of instruction
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 * 25 Oct 01 - Mike: Added check for exit, as well as _exit; processProc takes
 *              an optional pointer to function to check for helper funcs
 */

#include <queue>

#include "global.h"
#include "frontend.h"
#include "rtl.h"
#include "cfg.h"
#include "options.h"
#include "proc.h"
#include "prog.h"
#include "decoder.h"
#include "BinaryFile.h"

/*==============================================================================
 * FUNCTION:      FrontEnd::FrontEnd
 * OVERVIEW:      Construct the FrontEnd object
 * PARAMETERS:    delta: host - native address difference
 *                uUpper: highest+1 address in the text segment
 *                decoder: Reference to the NJMCDecoder object
 * RETURNS:       <N/a>
 *============================================================================*/
FrontEnd::FrontEnd(int delta, ADDRESS uUpper, NJMCDecoder& decoder)
 : delta(delta), uUpper(uUpper), decoder(decoder)
{
}

/*==============================================================================
 * FUNCTION:      FrontEnd::processProc
 * OVERVIEW:      Process a procedure, given a native (source machine) address.
 * PARAMETERS:    address - the address at which the procedure starts
 *                pProc - the procedure object
 *                spec - if true, this is a speculative decode
 *                helperFunc - pointer to a function to call (if not null) to
 *                  do a processor specific test for helper functions
 *                os - the output stream for .rtl output
 * NOTE:          This is a sort of generic front end. For many processors,
 *                  this will be overridden in the FrontEndSrc derived class,
 *                  sometimes calling this function to do most of the work
 * RETURNS:       true for a good decode (no illegal instructions)
 *============================================================================*/
bool FrontEnd::processProc(ADDRESS uAddr, UserProc* pProc, ofstream &os,
    bool spec /* = false */, PHELPER helperFunc)
{
    PBB pBB;                    // Pointer to the current basic block
    INSTTYPE type;              // Cfg type of instruction (e.g. IRET)

    // Declare a queue of targets not yet processed yet. This has to be
    // individual to the procedure!
    TARGETS targets;

    // Similarly, we have a set of HLCall pointers. These may be disregarded
    // if this is a speculative decode that fails (i.e. an illegal instruction
    // is found). If not, this set will be used to add to the set of calls to
    // be analysed in the cfg, and also to call prog.visitProc()
    SET_CALLS callSet;

    // Indicates whether or not the next instruction to be decoded is the
    // lexical successor of the current one. Will be true for all NCTs and for
    // CTIs with a fall through branch.
    bool sequentialDecode = true;

    Cfg* pCfg = pProc->getCFG();

    // If this is a speculative decode, the second time we decode the same
    // address, we get no cfg. Else an error.
    if (spec && (pCfg == 0))
        return false;
    assert(pCfg);

    // Initialise the queue of control flow targets that have yet to be decoded.
    targets.push(uAddr);

    // Clear the pointer used by the caller prologue code to access the last
    // call rtl of this procedure
    //decoder.resetLastCall();

    while ((uAddr = nextAddress(targets, pCfg)) != NO_ADDRESS) {

        // The list of RTLs for the current basic block
        list<HRTL*>* BB_rtls = new list<HRTL*>();

        // Keep decoding sequentially until a CTI without a fall through branch
        // is decoded
        ADDRESS start = uAddr;
        DecodeResult inst;
        while (sequentialDecode) {

            // Decode and classify the current instruction
            if (progOptions.trace)
                cout << "*" << hex << uAddr << "\t" << flush;

            // Decode the inst at uAddr.
            inst = decoder.decodeInstruction(uAddr, delta, pProc);

            // If invalid and we are speculating, just exit
            if (spec && !inst.valid)
                return false;

            // Need to construct a new list of RTLs if a basic block has just
            // been finished but decoding is continuing from its lexical
            // successor
            if (BB_rtls == NULL)
                BB_rtls = new list<HRTL*>();

            HRTL* pRtl = inst.rtl;
            if (inst.valid == false)
            {
                // An invalid instruction. Most likely because a call did
                // not return (e.g. call _exit()), etc. Best thing is to
                // emit a INVALID BB, and continue with valid instructions
                ostrstream ost;
                ost << "invalid instruction at " << hex << uAddr;
                warning(str(ost));
                // Emit the RTL anyway, so we have the address and maybe
                // some other clues
                BB_rtls->push_back(new RTL(uAddr));  
                pBB = pCfg->newBB(BB_rtls, INVALID, 0);
                sequentialDecode = false; BB_rtls = NULL; continue;
            }
    
            if (pRtl == 0) {
                // This can happen if an instruction is "cancelled", e.g.
                // call to __main in a hppa program
                // Just ignore the whole instruction
                uAddr += inst.numBytes;
                continue;
            }

            HLJump* rtl_jump = static_cast<HLJump*>(pRtl);

            // Display RTL representation if asked
            if (progOptions.rtl) {
                pRtl->print(os);
                os << flush;            // Handy when the translator crashes
            }
    
            ADDRESS uDest;

            switch (pRtl->getKind())
            {

            case JUMP_HRTL:
            {
                uDest = rtl_jump->getFixedDest();
    
                // Handle one way jumps and computed jumps separately
                if (uDest != NO_ADDRESS) {
                    BB_rtls->push_back(pRtl);
                    sequentialDecode = false;

                    pBB = pCfg->newBB(BB_rtls,ONEWAY,1);

                    // Exit the switch now and stop decoding sequentially if the
                    // basic block already existed
                    if (pBB == 0) {
                        sequentialDecode = false;
                        BB_rtls = NULL;
                        break;
                    }

                    // Add the out edge if it is to a destination within the
                    // procedure
                    if (uDest < uUpper) {
                        visit(pCfg, uDest, targets, pBB);
                        pCfg->addOutEdge(pBB, uDest, true);
                    }
                    else {
                        ostrstream ost;
                        ost << "Error: Instruction at " << hex << uAddr;
                        ost << " branches beyond end of section, to ";
                        ost << uDest;
                        error(str(ost)); 
                    }
                }
                break;
            }

            case NWAYJUMP_HRTL:
            {
                BB_rtls->push_back(pRtl);
                // We create the BB as a COMPJUMP type, then change
                // to an NWAY if it turns out to be a switch stmt
                pBB = pCfg->newBB(BB_rtls, COMPJUMP, 0);
                if (isSwitch(pBB, rtl_jump->getDest(), pProc)) {
                    processSwitch(pBB, delta, pCfg, targets, pProc);
                }
                else { // Computed jump
                    // Not a switch statement
                    ostrstream ost;
                    string sKind("JUMP");
                    if (type == I_COMPCALL) sKind = "CALL";
                    ost << "COMPUTED " << sKind << " at "
                    << hex << uAddr << endl;
                    warning(str(ost));
                    BB_rtls = NULL;    // New HRTLList for next BB
                }
                sequentialDecode = false;
                break;     
            }



            case JCOND_HRTL:
            {
                uDest = rtl_jump->getFixedDest();
                BB_rtls->push_back(pRtl);
                pBB = pCfg->newBB(BB_rtls, TWOWAY, 2);

                // Stop decoding sequentially if the basic block already existed
                // otherwise complete the basic block
                if (pBB == 0)
                    sequentialDecode = false;
                else {

                    // Add the out edge if it is to a destination within the
                    // procedure
                    if (uDest < uUpper) {
                        visit(pCfg, uDest, targets, pBB);
                        pCfg->addOutEdge(pBB, uDest, true);
                    }
                    else {
                        ostrstream ost;
                        ost << "Error: Instruction at " << hex << uAddr;
                        ost << " branches beyond end of section, to ";
                        ost << uDest;
                        error(str(ost)); 
                    }

                    // Add the fall-through outedge
                    pCfg->addOutEdge(pBB, uAddr + inst.numBytes); 
                }

                // Create the list of RTLs for the next basic block and continue
                // with the next instruction.
                BB_rtls = NULL;
                break;
            }

            case CALL_HRTL:
            {
                HLCall* call = static_cast<HLCall*>(pRtl);

                // Treat computed and static calls seperately
                if (call->isComputed()) {
                    BB_rtls->push_back(pRtl);
                    pBB = pCfg->newBB(BB_rtls, COMPCALL, 1);

                    // Stop decoding sequentially if the basic block already
                    // existed otherwise complete the basic block
                    if (pBB == 0)
                        sequentialDecode = false;
                    else
                        pCfg->addOutEdge(pBB, uAddr + inst.numBytes);
                    // Add this call to the list of calls to analyse. We won't
                    // be able to analyse it's callee(s), of course.
                    callSet.insert(call);
                }
                else {      // Static call
                    // Find the address of the callee.
                    ADDRESS uNewAddr = call->getFixedDest();

                    // Calls with 0 offset (i.e. call the next instruction) are
                    // simply pushing the PC to the stack. Treat these as
                    // non-control flow instructions and continue.
                    if (uNewAddr == uAddr + inst.numBytes)
                        break;

                    // Check for a helper function, if the caller provided one)
                    if (helperFunc != NULL) {
                        if ((*helperFunc)(uNewAddr, uAddr, BB_rtls)) {
                            // We have already added to BB_rtls
                            break;
                        }
                    }

                    BB_rtls->push_back(pRtl);

                    // Add this non computed call site to the set of call
                    // sites which need to be analysed later.
                    //pCfg->addCall(call);
                    callSet.insert(call);

                    // Record the called address as the start of a new
                    // procedure if it didn't already exist.
                    if (uNewAddr && prog.findProc(uNewAddr) == NULL) {
                        callSet.insert(call);
                        //prog.visitProc(uNewAddr);
                        if (progOptions.trace)
                            cout << "p" << hex << uNewAddr << "\t" << flush; 
                    }

                    // Check if this is the _exit or exit function. May prevent
                    // us from attempting to decode invalid instructions, and
                    // getting invalid stack height errors
                    const char* name = prog.pBF->SymbolByAddress(uNewAddr);
                    if (name && ((strcmp(name, "_exit") == 0) ||
                                 (strcmp(name,  "exit") == 0))) {
                        // Create the new basic block
                        pBB = pCfg->newBB(BB_rtls, CALL, 0);

                        // Stop decoding sequentially
                        sequentialDecode = false;
                    }
                    else {
                        // Create the new basic block
                        pBB = pCfg->newBB(BB_rtls, CALL, 1);

                        if (call->isReturnAfterCall()) {
                            // Constuct the RTLs for the new basic block
                            list<HRTL*>* rtls = new list<HRTL*>();
                            // The only RTL in the basic block is a high level
                            // return that doesn't have any RTs.
                            rtls->push_back(new HLReturn(0, NULL));
        
                            BasicBlock* returnBB = pCfg->newBB(rtls, RET, 0);
                            // Add out edge from call to return
                            pCfg->addOutEdge(pBB, returnBB);
                            // Put a label on the return BB (since it's an
                            // orphan); a jump will be reqd
                            pCfg->setLabel(returnBB);
                            pBB->setJumpReqd();
                            // Give the enclosing proc a dummy callee epilogue
                            pProc->setEpilogue(new CalleeEpilogue("__dummy",
                                list<string>()));
                            // Mike: do we need to set return locations?
                            // This ends the function
                            sequentialDecode = false;
                        }
                        else
                        {
                            // Add the fall through edge if the block didn't
                            // already exist
                            if (pBB != NULL)
                                pCfg->addOutEdge(pBB, uAddr + inst.numBytes);
                        }
                    }
                }

                // Create the list of RTLs for the next basic block and continue
                // with the next instruction.
                BB_rtls = NULL;
                break;  
            }

            case RET_HRTL:
                // Stop decoding sequentially
                sequentialDecode = false;

                // Add the RTL to the list
                BB_rtls->push_back(pRtl);
                // Create the basic block
                pBB = pCfg->newBB(BB_rtls, RET, 0);

                // Create the list of RTLs for the next basic block and continue
                // with the next instruction.
                BB_rtls = NULL;    // New HRTLList for next BB
                break;

            case SCOND_HRTL:
                // This is just an ordinary instruction; no control transfer
                // Fall through
            case LOW_LEVEL_HRTL:
                // We must emit empty RTLs for NOPs, because they could be the
                // destinations of jumps (and splitBB won't work)
                // Just emit the current instr to the current BB
                BB_rtls->push_back(pRtl);
                break;
        
            } // switch (pRtl->getKind())

            uAddr += inst.numBytes;
            // Update the RTL's number of bytes for coverage analysis (only)
            inst.rtl->updateNumBytes(inst.numBytes);

            // If sequentially decoding, check if the next address happens to
            // be the start of an existing BB. If so, finish off the current BB
            // (if any RTLs) as a fallthrough, and  no need to decode again
            // (unless it's an incomplete BB, then we do decode it).
            // In fact, mustn't decode twice, because it will muck up the
            // coverage, but also will cause subtle problems like add a call
            // to the list of calls to be processed, then delete the call RTL
            // (e.g. Pentium 134.perl benchmark)
            if (sequentialDecode && pCfg->isLabel(uAddr)) {
                // Create the fallthrough BB, if there are any RTLs at all
                if (BB_rtls) {
                    PBB pBB = pCfg->newBB(BB_rtls, FALL, 1);
                    // Add an out edge to this address
                    if (pBB) {
                        pCfg->addOutEdge(pBB, uAddr);
                        BB_rtls = NULL;         // Need new list of RTLs
                    }
                }
                // Pick a new address to decode from, if the BB is complete
                if (!pCfg->isIncomplete(uAddr))
                    sequentialDecode = false;
            }

        }   // while sequentialDecode

        // Add this range to the coverage
        pProc->addRange(start, uAddr);

        // Must set sequentialDecode back to true
        sequentialDecode = true;

    }   // while nextAddress() != NO_ADDRESS

    // This pass is to remove up to 7 nops in a row between ranges.
    // These will be assumed to be padding for alignments of BBs
    // Just removes a lot of ranges that could otherwise be combined
    ADDRESS a1, a2;
    COV_CIT ii;
    Coverage temp;
    if (pProc->getFirstGap(a1, a2, ii)) {
        do {
            int gap = a2 - a1;
            if (gap < 8) {
                bool allNops = true;
                for (int i=0; i < gap; i+= NOP_SIZE) {
                    if (getInst(a1+i+delta) != NOP_INST) {
                        allNops = false;
                        break;
                    }
                }
                if (allNops)
                    // Remove this gap, by adding a range equal to the gap
                    // Note: it's not safe to add the range now, so we put
                    // the range into a temp Coverage object to be added later
                    temp.addRange(a1, a2);
            }
        } while (pProc->getNextGap(a1, a2, ii));
    }
    // Now add the ranges in temp
    pProc->addRanges(temp);

    // Add the resultant coverage to the program's coverage
    pProc->addProcCoverage();

    // Add the callees to the set of HLCalls to proces for CSR, and also
    // to the Prog object
    for (SCIT it = callSet.begin(); it != callSet.end(); it++) {
        ADDRESS dest = (*it)->getFixedDest();
        // Don't speculatively decode procs that are outside of the main text
        // section, apart from dynamically linked ones (in the .plt)
        if (prog.pBF->IsDynamicLinkedProc(dest) || !spec || (dest < uUpper)) {
            pCfg->addCall(*it);
            // Don't visit the destination of a register call
            if (dest != NO_ADDRESS) prog.visitProc(dest);
        }
    }

    return true;
}

/*==============================================================================
 * FUNCTION:	FrontEnd::getInst
 * OVERVIEW:	Fetch the smallest (nop-sized) instruction, in an endianness
 *                independent manner
 * NOTE:        Frequently overridden
 * PARAMETERS:  addr - host address to getch from
 * RETURNS:     An integer with the instruction in it
 *============================================================================*/
int FrontEnd::getInst(int addr)
{
    return (int)(*(unsigned char*)addr);
}

/*==============================================================================
 * FUNCTION:      FrontEndSrc
 * OVERVIEW:      Constructor for the source dependent FrontEndSrc class
 * NOTE:          Because the constructor is trivial and the same for all
 *                  source machines, it is implemented here instead of in each
 *                  processor dependent file (e.g. sparcDir/frontsparc.cc)
 * PARAMETERS:    delta: host - native address difference
 *                uUpper: upper native address of the code segment
 *                decoder: reference to the NJMCDecoder object
 * RETURNS:       <n/a>
 *============================================================================*/
FrontEndSrc::FrontEndSrc(int delta, ADDRESS uUpper, NJMCDecoder& decoder)
 : FrontEnd(delta, uUpper, decoder)
{
}


/*==============================================================================
 * FUNCTION:	visit
 * OVERVIEW:	Visit a destination as a label, i.e. check whether we need to
 *				queue it as a new BB to create later.
 *				Note: at present, it is important to visit an address BEFORE
 *				an out edge is added to that address. This is because adding
 *				an out edge enters the address into the Cfg's BB map, and it
 *				looks like the BB has already been visited, and it gets
 *				overlooked. It would be better to have a scheme whereby the
 *				order of calling these functions (i.e. visit() and
 *				AddOutEdge()) did not matter.
 * PARAMETERS:	pCfg - the enclosing CFG
 *				uNewAddr - the address to be checked
 *				targets - queue of target addresses to be processed
 *				pNewBB - set to the lower part of the BB if the address
 *				already exists as a non explicit label (BB has to be split)
 * RETURNS:		<nothing>
 *============================================================================*/
void visit(Cfg* pCfg, ADDRESS uNewAddr, TARGETS& targets, PBB& pNewBB)
{
	// Find out if we've already parsed the destination
	bool bParsed = pCfg->label(uNewAddr, pNewBB);
	// Add this address to the back of the local queue,
	// if not already processed
	if (!bParsed)
	{
		targets.push(uNewAddr);
		if (progOptions.trace)
			cout << ">" << hex << uNewAddr << "\t" << dec << flush;
	}
}


/*==============================================================================
 * FUNCTION:          nextAddress
 * OVERVIEW:          Return the next target from the queue of non-processed
 *                    targets.
 * PARAMETERS:        targets - the queue of targets to be processed
 *                    cfg - the enclosing CFG
 * RETURNS:           The next address to process, or NO_ADDRESS if none
 *                      (targets is empty)
 *============================================================================*/
ADDRESS nextAddress(TARGETS& targets, Cfg* cfg)
{
	while (!targets.empty())
	{
		ADDRESS address = targets.front();
		targets.pop();
		if (progOptions.trace)
			cout << "<" << hex << address << "\t" << flush;

		// If no label there at all, or if there is a BB, it's incomplete,
		// then we can parse this address next
		if (!cfg->isLabel(address) || cfg->isIncomplete(address))
			return address;
	}
	return NO_ADDRESS;
}

/*==============================================================================
 * FUNCTION:      decodeRtl
 * OVERVIEW:      Decode the RTL at the given address
 * PARAMETERS:    address - native address of the instruction
 *				  delta - difference between host and native addresses
 *                decoder - decoder object
 * NOTE:		  Only called from findCoverage()
 * RETURNS:       a pointer to the decoded RTL
 *============================================================================*/
HRTL* decodeRtl(ADDRESS address, int delta, NJMCDecoder& decoder)
{
	UserProc proc("__dummy__",0);					// Just give it a dummy
	DecodeResult inst = 
		decoder.decodeInstruction(address, delta, &proc);

	// Define aliases to the RTLs so that they can be treated as a high
	// level types where appropriate.
	HRTL*   rtl        = inst.rtl;

	// Set the size (a few things still need this)
	rtl->updateNumBytes(inst.numBytes);
	return rtl;
}
