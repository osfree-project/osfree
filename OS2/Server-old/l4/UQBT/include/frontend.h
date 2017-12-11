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
 * FILE:        frontend.h
 * OVERVIEW:    This file contains the definition for the FrontEnd class,
 *              which implements the source indendent parts of the front end
 *              of UQBT: decoding machine instructions into a control flow
 *              graph populated with low and high level RTLs.
 *				Also has some prototypes and structs for switch.cc
 *
 * Copyright (C) 1998-2001, The University of Queensland, BT group
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 *============================================================================*/

/* $Revision: 1.27 $
 * 30 Jan 98 - Cristina, created from Load_elf.h.
 * 6 Aug 98 - Mike
 *	Mods for new source machine dependent front end
 * 02 Nov 98 - Mike: Added pDest to isSwitch()
 * 15 Dec 98 - Mike: replaced decode() with processProc() as the interface
 * 08 Feb 99 - Mike: processProc takes a Proc* now
 * 03 Mar 99 - Mike: isSwitch() and processSwitch() take an iOffset now
 * 09 Mar 99 - Mike: Removed interpreter
 * 10 Mar 99 - Mike: Removed Jcond
 * 07 Apr 99 - Mike: Mods for HEADERS
 * 28 Apr 99 - Mike: Mods for the depth first procedure decoding model
 * 02 Jul 99 - Mike: Moved many parameters for isSwitch() into SWITCH_INFO in
 *				the HLNwayJump RTL class
 * 16 Jun 00 - Mike: Created the FrontEnd class
 * 09 Nov 00 - Cristina: processProc() takes output stream for rtls
 * 18 Apr 01 - Mike: Removed insertAssign (now a member of class RTlist)
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 * 25 Oct 01 - Mike: Added helperFunc function pointer to processProc
 */


#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#include "decoder.h"

typedef bool (*PHELPER)(ADDRESS dest, ADDRESS addr, HRTLList* lrtl); 

class FrontEnd {
protected:
    int delta;                  // Host address - native address difference
    ADDRESS uUpper;             // Upper address for the text segment
    NJMCDecoder& decoder;       // Ref to the NJMCDecoder object
    const int NOP_SIZE;         // Size of a no-op instruction (in bytes)
    const int NOP_INST;         // No-op pattern

public:
    /*
     * Constructor. Takes some parameters to save passing these around a lot
     */
    FrontEnd(int delta, ADDRESS uUpper, NJMCDecoder& decoder);

    /*
     * Function to fetch the smallest machine instruction
     */
virtual int     getInst(int addr);

    /*
     * processProc. This is the main function for decoding a procedure.
     * It is usually overridden in the derived class FrontEndSrc to do
     * source machine specific things.
     * If spec is set, this is a speculative decode
     * Returns true on a good decode
     */
    bool        processProc(ADDRESS uAddr, UserProc* pProc, ofstream &os, 
                            bool spec = false, PHELPER helperFunc = NULL);
};


/*==============================================================================
 * These functions do the analysis required to see if a register jump
 * is actually a switch table.
 *============================================================================*/

/*
 * Initialise the switch analyser.
 */
void initSwitch();

/*
 * Attempt to determine whether this DD instruction is a switch
 * statement. If so, return true, and set iLower, uUpper to the
 * switch range, and set uTable to the native address of the
 * table. If it is form O (the table is an array of offsets from
 * the start of the table), then chForm will be 'O', etc.
 * If it is form H (hash table), then iNumTable will be the
 * number of entries in the table (not the number of cases,
 * or max-min).
 */
bool isSwitch(PBB pBB, SemStr* pDest, UserProc* pProc);

/*
 * Make use of the switch info. Should be incorporated into isSwitch.
 */
void processSwitch(PBB pBB, int delta, Cfg* pCfg, TARGETS& targets, Proc* proc);


/*==============================================================================
 * These functions are the machine specific parts of the front end. They consist
 * of those that actually drive the decoding and analysis of the procedures of
 * the program being translated.
 * These functions are implemented in the files front<XXX> where XXX is a
 * plaform name such as sparc or 386.
 *============================================================================*/

/*
 * Intialise the procedure decoder and analyser.
 */
void initFront();

/*
 * Needed by the switch logic. Here because it's source machine specific.
 */
unsigned fetch4(unsigned char* ptr);

/*
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
 */
void visit(Cfg* pCfg, ADDRESS uNewAddr, TARGETS& targets, PBB& pNewBB);

/*
 * Decode one RTL
 */
HRTL* decodeRtl(ADDRESS address, int delta, NJMCDecoder& decoder);

class FrontEndSrc;          // Forward class declaration
/*
 * This decodes a given procedure. It performs the
 * analysis to recover switch statements, call
 * parameters and return types etc.
 * If keep is false, discard the decoded procedure (only need this to find code
 *  other than main that is reachable from _start, for coverage and speculative
 *  decoding)
 * If spec is true, then we are speculatively decoding (i.e. if there is an
 *  illegal instruction, we just bail out)
 */
bool decodeProc(ADDRESS uAddr, FrontEndSrc& fe, bool keep = true,
    bool spec = false);

/**
 * Initialise the machine dependant CTI module.
 * Implemented in RTL/ctixxx.cc.
 */
void initCti();

/*
 * FUNCTION:      nextAddress
 * OVERVIEW:      Return the next target from the queue of non-processed
 *                targets.
 * PARAMETERS:    targets - the queue of targets to be processed
 *                cfg - the enclosing CFG
 * RETURNS:       The next address to process, or 0 if none (targets is empty)
 */
ADDRESS nextAddress(TARGETS& targets, Cfg* cfg);





// Class FrontEndSrc: derived from FrontEnd, with source machine specific
// behaviour (if any)

class FrontEndSrc : public FrontEnd
{
public:
    /*
     * Constructor. Takes some parameters to save passing these around a lot
     */
    FrontEndSrc(int delta, ADDRESS uUpper, NJMCDecoder& decoder);

    /*
     * processProc. This is the main function for decoding a procedure.
     * It is usually overridden in the derived class FrontEndSrc to do
     * source machine specific things.
     * If spec is true, this is a speculative decode (so give up on any invalid
     * instruction)
     * Returns true on a good decode
     */
    bool        processProc(ADDRESS uAddr, UserProc* pProc, ofstream &os,
			    bool spec = false);
};


#endif      // #ifndef __FRONTEND_H__
