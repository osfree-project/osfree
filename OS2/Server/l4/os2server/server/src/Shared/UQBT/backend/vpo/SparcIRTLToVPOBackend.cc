/*
 * Copyright (C) 1999, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*=============================================================================
 * FILE:       SparcIRTLToVPOBackend.cc
 * OVERVIEW:   Translates the UQBT IRTLs (intermediate RTLs) for a procedure
 *             into the SPARC-dependent IR used by the VPO portable
 *             optimizer. IRTLs are intermediate level between the low-level
 *             source machine-specific RTLs and HRTLs. They differ from RTLs
 *             in having delayed branches replaced with equivalent RTLs with
 *             simple control transfers. Other RTLs are mostly source
 *             machine-dependent and 1:1 with the original ones. This
 *             backend is a "code expander" in the VPO terminology and emits
 *             VPO RTLs and other directives for the procedure.
 *
 * Copyright (C) 1999, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *===========================================================================*/

/*
 * $Revision: 1.7 $
 * 27 Mar 01 - Brian: Revised version of the May 1999 backsparc.cc SPARC VPO
 *              backend written by Mike.
 *  4 May 01 - Brian: substantial enhancements to deal with other than 32 bit
 *              values, floating point, recursive procedures, complex
 *              SemStrs, temporary register allocation, etc.
 *  1 Jun 01 - Brian: substantially revised to support more IRTL constructs,
 *              leaf procedures, FP, new local variable layout, etc.
 *  5 Jun 01 - Brian: Changed "export" to "export_sym" to avoid problem with
 *              new C++ keyword.
 *  6 Jun 01 - Brian: Corrected comment at start of file.
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 */

#include "global.h"
#include "prog.h"
#include "proc.h"
#include "cfg.h"
#include "ss.h"
#include "rtl.h"
#include "type.h"
#include "options.h"
#include "BinaryFile.h"
#include "SparcIRTLToVPOBackend.h"

// Names of sections for VPOi_asm->section()
#define DATA_SEG ".data"
#define TEXT_SEG ".text"

// VPO needs to know the offset of each parameter relative to %SP = %o6 = r[14]
// The stack offset of the first parameter is FIRST_PARAM_OFFSET
#define FIRST_PARAM_OFFSET 68

// First number used when creating names of generated labels. Label names are
// of form .Lnnnn. Block labels use labels with names based on the (small)
// block number.
#define FIRST_GEN_LABEL_NO 9000



/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::SparcIRTLToVPOBackend
 * OVERVIEW:        Constructor for class SparcIRTLToVPOBackend
 * PARAMETERS:      program: reference to Prog object for program being
 *                  processed.
 * RETURNS:         <nothing>
 *===========================================================================*/
SparcIRTLToVPOBackend::SparcIRTLToVPOBackend(Prog& program) {
    prog = &program;
    binFile = prog->pBF;
    
    // initialize pattern SemStrs used to search for refs to regs and temp regs
    regRefPattern.push(idRegOf);
    regRefPattern.push(idIntConst);
    regRefPattern.push(WILD);
    
    tempRefPattern.push(idRegOf);
    tempRefPattern.push(idTemp);
    tempRefPattern.push(WILD);
    
    localPattern.push(idMemOf);
    localPattern.push(idMinus);
    localPattern.push(idRegOf);
    localPattern.push(idIntConst);
    localPattern.push(30);
    localPattern.push(idIntConst);
    localPattern.push(WILD);
    
    localAddrPattern.push(idMinus);
    localAddrPattern.push(idRegOf);
    localAddrPattern.push(idIntConst);
    localAddrPattern.push(30);
    localAddrPattern.push(idIntConst);
    localAddrPattern.push(WILD);
    
    // initialize sets of registers used when processing procedure calls
    callerSave_set = NULL;
    killed_set = NULL;
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::~SparcIRTLToVPOBackend
 * OVERVIEW:        Destructor for class SparcIRTLToVPOBackend.
 * PARAMETERS:      none.
 * RETURNS:         <nothing>
 *===========================================================================*/
SparcIRTLToVPOBackend::~SparcIRTLToVPOBackend() {
}

/*=============================================================================
 * FUNCTION:    SparcIRTLToVPOBackend::expandFunction
 * OVERVIEW:    Emit VPO RTLs and other directives for a procedure. Creates
 *              a <procname>.cex file for input to the VPO optimizer.
 * PARAMETERS:  proc: pointer to this procedure's Proc object.
 * RETURNS:     <nothing>
 *===========================================================================*/
void SparcIRTLToVPOBackend::expandFunction(UserProc* proc) {
    if (proc->getEntryBB() == NULL) {
        cout << "expandFunction: no entry block, quitting" << endl;
        return;             // Don't attempt to generate code
    }

    // Initialize or reset member variables
    theProc = proc;
    procName = theProc->getName();
    doesCalls = false;
    procHasRegWin = false;
    localsSize = 0;
    nextGenLabelNo = FIRST_GEN_LABEL_NO;
    rtlAddr = 0;
    
    labelMap.clear();
    procMap.clear();
    regsUsed.clear();
    tempMap.clear();
    machDepIdx.clear();
    machDepMap.clear();
    
    // Translate the procedure
    if (progOptions.proctrace || progOptions.verbose) {
        cout << "--IRTL to SPARC VPO back end for procedure \""
             << procName << "\"" << endl;
    }
    preamble();
    firstPass();
    secondPass();
    postamble();
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::preamble
 * OVERVIEW:        Emit VPO directives for the start of a procedure.
 * PARAMETERS:      none.
 * RETURNS:         <nothing>
 *===========================================================================*/
void SparcIRTLToVPOBackend::preamble() {
    // get the name of the output (.cex) file
    string fileNameStr = progOptions.outDir + procName + string(".cex");
    const char* fileName = fileNameStr.c_str();
    const char* argv[] = {"-o", fileName};
    
    // Initialize VPOi and asm interfaces
    // NB: This assumes that Brian's patch to vpoi.c.nw has been installed
    // that allows the VPOI outfile file to be specified. 
    VPOi_begin("sparc-solaris", 2, argv);

    ostrstream ost;
    ost << "Procedure " << procName;
    VPOi_asm->comment(str(ost));
    VPOi_sourceFile((char*)procName);
    
    // Initialize processor specific globals. VPOi_begin() must be called first
    initRegsAndLocations();
    
    // Emit the assembly preamble for the procedure
    VPOi_asm->section(TEXT_SEG);
    VPOi_asm->align(8);
    
    AsmSymbol procSym = VPOi_asm->export_sym(procName);
    // Record procedure's asm symbol to avoid a VPO error later if procedure
    // is called recursively (we can't export() and import() the same symbol).
    string procNameStr = procName;
    procMap[procNameStr] = procSym;
    // Label the start of the procedure and tell VPO about the procedure
    VPOi_asm->define_symbol_here(procSym);
    VPOi_functionDefine(procSym, /*callee save regs*/ NULL);
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::postamble
 * OVERVIEW:        Emit VPO directives for the end of a procedure.
 * PARAMETERS:      none
 * RETURNS:         <nothing>
 *===========================================================================*/
void SparcIRTLToVPOBackend::postamble() {
    VPOi_functionEnd();
    VPOi_end();
}


/*
 *=============================================================================
 * First pass: scan for references and declare imports, exports, vars.
 *=============================================================================
 */

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::firstPass
 * OVERVIEW:        Does several things:
 *                    1) Looks for BBs with labels (labels only appear at the
 *                       start of a BB), declares the labels to VPO, and places
 *                       them in labelMap, a map from BB labels to AsmSymbols.
 *                    2) For each RTL in the proc,
 *                        a) looks for SemStrs with IDs >= idMachSpec. If so,
 *                           adds the index to machDepIdx, the set of source
 *                           machine-specific SemStr indexes. These refer to
 *                           integer and FP flags.
 *                        b) looks for SemStrs of form r[ temp T. If found, 
 *                           declares a VPO temp reg, and associates it with T
 *                           in tempMap.
 *                    3) Declares VPO local vars for each local variable of
 *                       form m[%fp-NN].
 *                    4) Declares called procedures to VPO.
 * PARAMETERS:      none.
 * NOTE:            Assumes that expressions of the form r[6+1] have been
 *                  changed to r[7]
 * RETURNS:         Nothing, but sets the machDepIdx set.
 *===========================================================================*/
void SparcIRTLToVPOBackend::firstPass() {
    Cfg* cfg = theProc->getCFG();
    cfg->sortByAddress();

    // If the entry BB is not the first BB, set a label for the entry BB.
    // Note: using -S, the proc may have no BBs, so firstBB may be NULL here.
    BB_CIT bbIter;
    PBB firstBB = cfg->getFirstBB(bbIter);
    if ((firstBB != NULL) &&
        (firstBB->getLowAddr() != theProc->getNativeAddress()) &&
        (theProc->getEntryBB()->getLabel() == 0)) {
        cfg->setLabel(theProc->getEntryBB());
    }

    // Scan each basic block
    for (PBB bb = firstBB;  bb != NULL;  bb = cfg->getNextBB(bbIter)) {
        // create VPO labels for BasicBlock labels (which are integers)
        int label = bb->getLabel();
        if (label != 0) {
            ostrstream lName;
            lName << ".L" << dec << label; // SPARC VPO labels look like .Lnnnn
            AsmSymbol labelSym = VPOi_asm->local(str(lName));
            labelMap[label] = labelSym;
        }
        
        // scan the block's RTLs searching for ref'd regs, temps, vars, etc.
        scanRTLsInBlock(bb);
    }

    // Check if VPO will use save/restore (create a register window) for the
    // proc: if any "non-scratch" registers are used: %o6-7, %l0-7, %i0-7.
    if (doesCalls) {
        procHasRegWin = true;
        cout << "  HAS reg window: does calls\n";
    } else {
        procHasRegWin = false;
        for (int i = 14;  i < 32;  i++) {
            if (regsUsed.find(i) != regsUsed.end()) { // non-scratch reg used
                cout << "  HAS reg window: r[" << i << "] is used\n";
                procHasRegWin = true;
                break;
            }
        }
        if (!procHasRegWin) {
            cout << "  NO reg window\n";
        }
    }

    // Declare a VPO variable "locals" to hold all the local UQBT vars
    declareLocalVars();

#ifdef SUPPORTING_HRTL
    // BTL: The following is useful only for HRTLs where analysis has been
    // done. For IRTLs, these machine-dependant registers should only appear
    // on the rhs of RTls and we treat them specially.
    declareMachDepRegsAsVars();
#endif /*SUPPORTING_HRTL*/
    
    // Set bases for allocating new VPO temp regs during the second pass.
    // These are after any temps allocated for idRegOf idTemp (r[ temp).
    t_base = curr_t;
    x_base = curr_x;
    y_base = curr_y;
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::scanRTLsInBlock
 * OVERVIEW:        Search a block's RTLs searching for referenced registers,
 *                  temporary registers, variables, and called procedures.
 *                  Declare the called procedures to VPO. Called during the
 *                  first pass.
 * PARAMETERS:      bb: a pointer to a BasicBlock.
 * RETURNS:         Nothing.
 *===========================================================================*/
void SparcIRTLToVPOBackend::scanRTLsInBlock(PBB bb) {
    HRTLList* rtlList = bb->getHRTLs();
    if (rtlList != NULL) {
        for (HRTLList_IT rit = rtlList->begin(); rit != rtlList->end(); rit++) {
            HRTL* rtl = (*rit);
            int n = rtl->getNumRT();
            for (int i = 0;  i < n;  i++) {
                RT* rt = rtl->elementAt(i);
                scanForRefsInRT(rt);
            }
            
            if (rtl->getKind() == CALL_HRTL) {
                const HLCall* call = static_cast<const HLCall*>(rtl);
                doesCalls = true;
                declareCalledProc(call);
                
                SemStr retLoc = call->getReturnLoc();
                scanForRegsUsed(&retLoc);
                scanForTempRegsUsed(&retLoc);
                
                // If there are post call semantics, iterate through RTs.
                const list<RT*>* lrt = call->getPostCallRtlist();
                if (lrt != NULL) {
                    for (RT_CIT it = lrt->begin(); it != lrt->end(); it++){
                        RT* rt = (*it);
                        scanForRefsInRT(rt);
                    }
                }
            }
            // Other HL types may have to be considered in the future
        }
    }
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::scanForRefsInRT
 * OVERVIEW:        Scan an RT searching for referenced registers,
 *                  temporary registers, variables, and machine-specific Ids.
 *                  Called during the first pass.
 * PARAMETERS:      rt: a pointer to a RT.
 * RETURNS:         Nothing.
 *===========================================================================*/
void SparcIRTLToVPOBackend::scanForRefsInRT(RT* rt) {
    if (rt->getKind() == RTASSGN) {
        RTAssgn* assign = (RTAssgn*)rt;
        Type cType = assign->getType();
        SemStr* lhs = assign->getLHS();
        SemStr* rhs = assign->getRHS();
        
        scanForRegsUsed(lhs);
        scanForRegsUsed(rhs);
        
        scanForTempRegsUsed(lhs);
        scanForTempRegsUsed(rhs);
        
        // search for machine specific IDs such as %Y, %CF
        lhs->searchMachSpec(machDepIdx);
        rhs->searchMachSpec(machDepIdx);
        
        // find refs to local vars (of the form m[%fp-nn])
        scanForLocalVarRefs(lhs, cType);
        scanForLocalVarRefs(rhs, cType);
    }
}
    
/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::scanForRegsUsed
 * OVERVIEW:        Search for SemStrs of form r[ int X. Add the number of
 *                  each register X found to regsUsed, the set of used
 *                  register numbers.
 * PARAMETERS:      ss: Points to a SemStr structure.
 * RETURNS:         <nothing>
 *===========================================================================*/
void SparcIRTLToVPOBackend::scanForRegsUsed(SemStr* ss) {
    // search for SemStrs of form r[ int X
    list<SemStr*> result;
    if (ss->searchAll(regRefPattern, result)) {
        // we have at least one register; go through the list
        list<SemStr*>::iterator ssIter;
        for (ssIter = result.begin();  ssIter != result.end();  ssIter++) {
            SemStr* regRefSS = (*ssIter);
            regsUsed.insert(regRefSS->getThirdIdx());
        }
    }
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::scanForTempRegsUsed
 * OVERVIEW:        Search for SemStrs of form r[ temp T. For each found, 
 *                  declare a VPO temp reg, and associate it with T in
 *                  tempMap.
 * PARAMETERS:      ss: Points to a SemStr structure.
 * RETURNS:         <nothing>
 *===========================================================================*/
void SparcIRTLToVPOBackend::scanForTempRegsUsed(SemStr* ss) {
    // search for SemStrs of form r[ temp X  (i.e., idRegOf idTemp WILD)
    list<SemStr*> result;
    if (ss->searchAll(tempRefPattern, result)) {
        // we have at least one temp register name; go through the list
        list<SemStr*>::iterator ssIter;
        for (ssIter = result.begin();  ssIter != result.end();  ssIter++) {
            SemStr* tempRefSS = (*ssIter);
            // temps are all int32 except if last char is of form "d", "l", ...
            const string& tempName = 
                theSemTable[tempRefSS->getThirdIdx()].sName;
            if (tempMap.find(tempName) == tempMap.end()) {
                // allocate new VPO temp register for UQBT temp
                Type tType = Type::getTempType(tempName);
                Rtl_ty_loc tempLoc = getTempReg(tType);
                tempMap[tempName] = tempLoc;
            }
        }
    }
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::scanForLocalVarRefs
 * OVERVIEW:        Search in ss for local var SemStrs of form m[%fp-NN] or
 *                  %fp-NN. Set localsSize to the largest local variable
 *                  offset NN seen.
 * PARAMETERS:      ss: Points to a SemStr structure.
 *                  cType: the "current" expected type.
 * RETURNS:         <nothing>
 *===========================================================================*/
void SparcIRTLToVPOBackend::scanForLocalVarRefs(SemStr* ss, Type cType) {
    // search for SemStrs of form m[ - r[ int 30 int WILD
    list<SemStr*> result;
    if (ss->searchAll(localPattern, result)) {
        // we have at least one local reference; go through the list
        list<SemStr*>::iterator ssIter;
        for (ssIter = result.begin();  ssIter != result.end();  ssIter++) {
            SemStr* ref = (*ssIter);
            // Assume m[  -  r[  int  30  int  NN
            //        0   1   2    3   4    5   6
            assert(ref->getIndex(4) == 30);
            assert(ref->getIndex(5) == idIntConst);
            int NN = ref->getIndex(6);
            if (NN > localsSize) {
                localsSize = NN;
            }
        }
    }

    // also search for SemStrs of form - r[ int 30 int WILD (these are
    // addresses of local vars)
    list<SemStr*> result2;
    if (ss->searchAll(localAddrPattern, result2)) {
        list<SemStr*>::iterator ssIter;
        for (ssIter = result2.begin();  ssIter != result2.end();  ssIter++) {
            SemStr* ref = (*ssIter);
            // Assume  -  r[  int  30  int  NN
            //         0   1   2    3   4    5
            assert(ref->getIndex(3) == 30);
            assert(ref->getIndex(4) == idIntConst);
            int NN = ref->getIndex(5);
            if (NN > localsSize) {
                localsSize = NN;
            }
        }
    }
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::declareCalledProc
 * OVERVIEW:        Emit VPO declarations needed to reference a called
 *                  procedure. Called during the first pass.
 * PARAMETERS:      rt: a pointer to a RT.
 * RETURNS:         Nothing.
 *===========================================================================*/
void SparcIRTLToVPOBackend::declareCalledProc(const HLCall* call) {
    ADDRESS fixedDest = call->getFixedDest();
    if (fixedDest == 0) {
        ostrstream ost;
        ost << "Computed or register CALL_HRTLs  are not implemented:";
        call->getDest()->print(ost);
        error(str(ost));
        return;
    }
    
    const char* calleeName = binFile->SymbolByAddress(fixedDest);
    if (calleeName == NULL) {
        ostrstream ost;
        ost << "Proc has no name, local function calls are unimplemented: ";
        call->getDest()->print(ost);
        error(str(ost));
        return;
    }
    
    // Declare the procedure being called to VPO
    AsmSymbol calleeSym;
    string calleeNameStr = calleeName;
    if (procMap.find(calleeNameStr) == procMap.end()) {
        calleeSym = VPOi_asm->import(calleeName);
        procMap[calleeNameStr] = calleeSym;
    } else {
        calleeSym = procMap[calleeNameStr];
    }
    VPOi_globalDeclare(calleeSym, 'r');
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::declareLocalVars
 * OVERVIEW:        Declare a VPO variable "locals" to hold all the local UQBT
 *                  vars. Called during the first pass.
 * PARAMETERS:      None.
 * RETURNS:         Nothing.
 *===========================================================================*/
void SparcIRTLToVPOBackend::declareLocalVars() {
    if (localsSize > 0) {
        localsSym = VPOi_asm->local(".2_locals");
        VPOi_localVariableDeclare(localsSym, 'r', localsSize, /*volat*/ 0,
                                  /*block*/ 2);
    }
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::declareMachDepRegsAsVars
 * OVERVIEW:        Declare machine-dependant registers such as %CF as integer
 *                  local vars except for %Y, which is treated specially.
 *                  Called during the first pass.
 *
 *                  NB: This is useful only for HRTLs where analysis has been
 *                  done. For IRTLs, these machine-dependant registers should
 *                  only appear on the rhs of RTls and we treat them specially.
 * PARAMETERS:      None.
 * RETURNS:         Nothing.
 *===========================================================================*/
void SparcIRTLToVPOBackend::declareMachDepRegsAsVars() {
    set<INDEX>::iterator its;
    for (its = machDepIdx.begin();  its != machDepIdx.end();  its++) {
        const string& regName = theSemTable[*its].sName;
        ostrstream ost;
        if (!((regName.length() > 1) &&
              (regName[0] == '%') && (regName[1] == 'Y'))) {
            // Not %Y. Now remove any '%' at the start for declaration.
            ost << ".2_";
            if ((regName.length() > 0) && (regName[0] == '%')) {
                ost << regName.substr(1);
            } else {
                ost << regName;
            }
            AsmSymbol sym = VPOi_asm->local(str(ost));
            VPOi_localVariableDeclare(sym, 'r', /*numBytes*/ 4, /*volat*/ 0,
                                      /*block*/ 2);
            machDepMap[regName] = sym;
        }
    }
}


/*
 *=============================================================================
 * Second pass: emit code for UQBT Rtls.
 *=============================================================================
 */

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::secondPass
 * OVERVIEW:        Emit VPO RTLs and directives for the body of a procedure.
 * PARAMETERS:      none.
 * RETURNS:         Nothing.
 *===========================================================================*/
void SparcIRTLToVPOBackend::secondPass() {
    Cfg* cfg = theProc->getCFG();
    BB_CIT bbIter;

    // If the entry BB is not the first BB, emit a branch to the entry BB.
    // Note: using -S, the proc may have no BBs, so firstBB may be NULL here.
    PBB firstBB = cfg->getFirstBB(bbIter);
    if ((firstBB != NULL) &&
            (firstBB->getLowAddr() != theProc->getNativeAddress())) {
        PBB entryBB = theProc->getEntryBB();
        int entryLabel = entryBB->getLabel();
        jumpToBBLabel(entryLabel);
    }
    
    for (PBB bb = cfg->getFirstBB(bbIter);  bb != NULL;
            bb = cfg->getNextBB(bbIter)) {
        ADDRESS srcAddr = bb->getLowAddr();
        int label = bb->getLabel();
        ostrstream bbost;
        bbost << "Basic block at src addr 0x" << hex << srcAddr;
        if (label != 0) {
            bbost << ", label " << dec << label;
        }
        VPOi_asm->comment(str(bbost));
        
        // emit any label for the basic block
        if (label != 0) {
            assert(labelMap.find(label) != labelMap.end());
            VPOi_asm->define_symbol_here(labelMap[label]);
        }
        
        // emit VPO RTLs for the basic block
        HRTLList* rtlList = bb->getHRTLs();
        if (rtlList != NULL) {
            HRTLList_CIT rit;
            for (rit = rtlList->begin();  rit != rtlList->end();  rit++) {
                HRTL* rtl = (*rit);
                bool isCommented = rtl->getCommented();
                rtlAddr = rtl->getAddress(); // set address for error messages
                if (processCT(rtl, bb)) {
                    ; // if processCT returned true, it processed the RTL
                } else {
                    int n = rtl->getNumRT();
                    if (isCommented) {
                        ostrstream ost;
                        ost << "The following RTLs are commented out";
                        VPOi_asm->comment(str(ost));
                    }
                    for (int i = 0;  i < n;  i++) {
                        RT* rt = rtl->elementAt(i);
                        if (isCommented) {
                            ostrstream ost;
                            ost << "/* ";
                            rt->print(ost);
                            ost << " */";
                            VPOi_asm->comment(str(ost));
                        } else {
                            processRT(rt);
                        }
                    }
                }
            }
        }

        // Emit jump if required
        if ((bb->getType() == FALL) && (bb->isJumpReqd())) {
            int label = bb->getOutEdges()[0]->getLabel();
            jumpToBBLabel(label);
        }
    }
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::processCT
 * OVERVIEW:        If "rtlist" points to a control transfer IRTL, emits VPO
 *                  RTLs for that control transfer
 * PARAMETERS:      rtlist: points to a UQBT IRTL.
 *                  bb: points to BasicBlock containing rtlist.
 * RETURNS:         true if "rtlist" points to a control transfer IRTL;
 *                  false if an ordinary RTL.
 *===========================================================================*/
bool SparcIRTLToVPOBackend::processCT(const HRTL* rtlist, PBB bb) {
    int i;
    if (rtlist->getKind() == LOW_LEVEL_HRTL) {
        return false;           // an ordinary RTL, not a control transfer
    }

    ostrstream strm;
    rtlist->print(strm);
    char *s = replaceNLs(str(strm));
    if (strlen(s) != 0) {
        VPOi_asm->comment(s);
    }
    
    resetTempRegs();
    switch (rtlist->getKind()) {
    case JUMP_HRTL: {
        const HLJump* jump = static_cast<const HLJump*>(rtlist);
        ADDRESS fixedDest = jump->getFixedDest();
        if (fixedDest == 0) {
            ostrstream ost;
            ost << "processCT: unhandled computed JUMP_HRTL: dest is ";
            jump->getDest()->print(ost);
            error(str(ost));
        } else {
            int label = bb->getOutEdges()[0]->getLabel();
            jumpToBBLabel(label);
        }
        break;
    }
        
    case JCOND_HRTL: {
        const HLJcond* jcond = static_cast<const HLJcond*>(rtlist);
        ADDRESS fixedDest = jcond->getFixedDest();
        if (fixedDest == 0) {
            ostrstream ost;
            ost << "processCT: unhandled computed JCOND_HRTL: dest is ";
            jcond->getDest()->print(ost);
            error(str(ost));
            break;
        }
        
        Rtl_ty_oper op;
        switch (jcond->getCond()) {
        case HLJCOND_JE:   op = Rtl_op_eq;    break;
        case HLJCOND_JNE:  op = Rtl_op_ne;    break;
        case HLJCOND_JSL:  op = Rtl_op_lt;    break;
        case HLJCOND_JSLE: op = Rtl_op_le;    break;
        case HLJCOND_JSGE: op = Rtl_op_ge;    break;
        case HLJCOND_JSG:  op = Rtl_op_gt;    break;
        case HLJCOND_JUL:  op = Rtl_op_ltUn;  break;
        case HLJCOND_JULE: op = Rtl_op_leUn;  break;
        case HLJCOND_JUGE: op = Rtl_op_geUn;  break;
        case HLJCOND_JUG:  op = Rtl_op_gtUn;  break;
            
        // the following are not quite correct: e.g. HLJCOND_JMI should test
        // the ICC N bit but Rtl_op_lt actually tests (N xor V)
        case HLJCOND_JMI:  op = Rtl_op_lt;    break;
        case HLJCOND_JPOS: op = Rtl_op_ge;    break;
        case HLJCOND_JOF:  op = Rtl_op_lt;    break;
        case HLJCOND_JNOF: op = Rtl_op_lt;    break;
        default:
            ostrstream ost;
            ost << "processCT: unexpected JCOND_TYPE " << jcond->getCond();
            error(str(ost));
            return true;
        }

        Rtl_ty_expr guard;
        if (jcond->isFloat()) {
            guard = Rtl_binary(op, Rtl_fetch(VPOi_loc_FC, 32), Rtl_int(0));
        } else {
            guard = Rtl_binary(op, Rtl_fetch(VPOi_loc_IC, 32), Rtl_int(0));
        }
        
        int label = bb->getOutEdges()[0]->getLabel();
        AsmSymbol labelSym = labelMap[label];
        Rtl_ty_rtl jump = Rtl_assign(VPOi_loc_PC, 32,
                                     Rtl_relAddr(labelSym->relAddr));
        VPOi_rtl(Rtl_guard(guard, jump), NULL);

        // If the "fall through" out-edge doesn't really fall through, the
        // "jump reqd" bit is set, and we must generate a jump
        if (bb->isJumpReqd()) {
            label = bb->getOutEdges()[1]->getLabel();
            jumpToBBLabel(label);
        }
        break;
    }
    
    case RET_HRTL: {
        // Check if return has semantics: iterate through any RTs of the return
        int n = rtlist->getNumRT();
        for (i = 0;  i < n;  i++) {
            RT* rt = rtlist->elementAt(i);
            // if VPO will give proc a reg window, replace any assignment
            // to CALLER's %o0 (r8) with the %i0 "seen" (r24) in this proc
            if (procHasRegWin && (rt->getKind() == RTASSGN)) {
                SemStr* lhs = ((RTAssgn*)rt)->getLHS();
                if ((lhs->getFirstIdx() == idRegOf) &&
                    (lhs->getSecondIdx() == idIntConst)) {
                    if (lhs->getThirdIdx() == 8) {
                        cout << "RET_HRTL: replacing assign to r[8] in ";
                        rt->print(cout);
                        cout << "\n";
                        // regOf int 8
                        //   0    1  2
                        lhs->substIndex(2, 24);
                    }
                }
            }
            processRT(rt);
        }
                
        // For now: set the integer return register.
        // Int return reg is either %i0 (r[24]) or %o0 (r[8]) if a leaf proc
        VPOi_ty_locSet live_set;
        if (procHasRegWin) {
            live_set = VPOi_locSetBuild(r[24], NULL); // normal proc
        } else {
            live_set = VPOi_locSetBuild(r[8], NULL);  // leaf proc
        }

        Rtl_ty_expr actualRetAddr =
            Rtl_binary(Rtl_op_add, Rtl_fetch(VPOi_loc_RT, 32), Rtl_int(8));
        Rtl_ty_rtl ret = Rtl_assign(VPOi_loc_PC, 32, actualRetAddr);
        VPOi_functionLeave(ret, live_set);
        break;
    }
    
    case CALL_HRTL: {
        // If the call has semantics, iterate through those RTs.
        int n = rtlist->getNumRT();
        for (i = 0;  i < n;  i++) {
            RT* rt = rtlist->elementAt(i);
            processRT(rt);
        }
                
        const HLCall* call = static_cast<const HLCall*>(rtlist);
        ADDRESS fixedDest = call->getFixedDest();
        if (fixedDest == 0) {
            break;
        }
        const char* calleeName = binFile->SymbolByAddress(fixedDest);
        if (calleeName == NULL) {
            break;
        }
        string calleeNameStr = calleeName;
        // Called proc should have been declared by declareCalledProc()
        assert(procMap.find(calleeNameStr) != procMap.end());
        AsmSymbol calleeSym = procMap[calleeNameStr];
        
        // Specify the parameters: the max stack frame offset for the
        // "argument build area" and the number of %o registers needed.
        int numParamBytes;
        int numParams = getParamInfo(call, fixedDest, numParamBytes);
        int numORegs = ((numParams <= 6)? numParams : 6); // for now
        // outgoing args are at offset [FIRST_PARAM_OFFSET..maxArgOffset]
        // relative to the caller's %sp.
        int maxArgOffset = (FIRST_PARAM_OFFSET + numParamBytes);
        VPOi_parameterListSize(maxArgOffset, numORegs);
        
        // Define set of regs not needed after call: int & FP cond code regs
        VPOi_ty_locSet dead_set = VPOi_locSetBuild(
            VPOi_loc_IC, VPOi_loc_FC, NULL);
        
        // Define set of regs used in callee, defined in caller: parameters
        VPOi_ty_locSet param_set = NULL;
        for (i = 0;  i < numORegs;  i++) {
            param_set = VPOi_locSetAdd(param_set, o[i]);
        }
        
        // Define set of regs returned by procedure
        // for now: assume the called procedure returns a value in %o0.
        VPOi_ty_locSet defined_set = VPOi_locSetBuild(o[0], NULL);
        
        Rtl_ty_expr procAddr = Rtl_fetch(loadSymAddr(calleeSym), 32);
        Rtl_ty_rtl callRtl = Rtl_assign(VPOi_loc_PC, 32, procAddr);
        
        VPOi_functionCall(callRtl, dead_set, param_set, defined_set,
                          callerSave_set, killed_set, used_set);

        // If there are post call semantics, iterate through its RTs.
        const list<RT*>* lrt = call->getPostCallRtlist();
        if (lrt != NULL) {
            for (RT_CIT it = lrt->begin();  it != lrt->end();  it++) {
                RT* rt = (*it);
                processRT(rt);
            }
        }

        // If a call/restore, emit a branch
        if (bb->isJumpReqd()) { 
            int label = bb->getOutEdges()[0]->getLabel();
            jumpToBBLabel(label);
        }
        break;
    }
    
    default: {
        ostrstream ost;
        ost << "processCT: unhandled CT: ";
        rtlist->print(ost, 0);
        error(str(ost));
        break;
    }
    }
    return true;
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::processRT
 * OVERVIEW:        Emits VPO RTLs for a UQBT RT.
 * PARAMETERS:      rt: points to the UQBT RT
 * RETURNS:         <nothing>
 *===========================================================================*/
void SparcIRTLToVPOBackend::processRT(RT* rt) {
    ostrstream ost;
    rt->print(ost);
    char *s = replaceNLs(str(ost));
    if (strlen(s) != 0) {
        VPOi_asm->comment(s);
    }
    
    resetTempRegs();
    
    switch(rt->getKind()) {
    case RTASSGN: {
        RTAssgn* assign = (RTAssgn*)rt;
        Type cType(assign->getType());  // initial type, from the assignment
        int currSize = cType.getSize(); // desired size in bits
        const SemStr* lhs = assign->getLHS();
        const SemStr* rhs = assign->getRHS();
        Rtl_ty_loc lhsLoc;
        Rtl_ty_expr rhsValue;
        
        // First process the left hand side
        switch(lhs->getFirstIdx()) {
            case idRegOf:
                lhsLoc = processRegOf(lhs);
                break;
                
            case idMemOf:
                lhsLoc = processMemOf(lhs, cType);
                break;
                
            default:
                if (lhs->getFirstIdx() >= idMachSpec) {
                    // This is a machine-dependant register, such as %pc or %Y.
                    const string& regName =
                        theSemTable[lhs->getFirstIdx()].sName;
                    if ((regName.length() > 1) &&
                            (regName[0] == '%') && (regName[1] == 'Y')) {
                        lhsLoc = VPOi_loc_YR;
                    } else {
#ifdef SUPPORTING_HRTL
                        // Not %Y. Load address of reg's int var into lhsLoc.
                        assert(machDepMap.find(regName) != machDepMap.end());
                        AsmSymbol sym = machDepMap[regName];
                        Rtl_ty_loc varLoc = loadLocalVarAddr(sym, 0);
                        lhsLoc = Rtl_location('m', Rtl_fetch(varLoc, 32));
#else
                        ostrstream ost;
                        ost << "processRT: unexpected use of machine-dependant"
                            << " reg in LHS of assignment: ";
                        assign->print(ost);
                        error(str(ost));
                    return;
#endif /*SUPPORTING_HRTL*/
                    }
                    break;
                } else {
                    ostrstream ost;
                    ost << "processRT: unrecognized LHS in assignment: ";
                    assign->print(ost);
                    error(str(ost));
                    return;
                }
        }
        
        // Next process the right hand side. Handle important special case of
        // RHS being a small integer.
        if ((rhs->getFirstIdx() == idIntConst) &&
                fitsInSPARCImm(rhs->getSecondIdx())) {
            rhsValue = Rtl_int(rhs->getSecondIdx());
        } else {
            Rtl_ty_loc tempRHS = getTempReg(cType);
            Rtl_ty_expr expr = processExpr(rhs, cType);
            VPOi_rtl(Rtl_assign(tempRHS, currSize, expr), NULL);
            rhsValue = Rtl_fetch(tempRHS, currSize);
        }
        
        // Combine into assignment
        VPOi_rtl(Rtl_assign(lhsLoc, currSize, rhsValue), NULL);
        break;
    }
    
    case RTFLAGCALL: {
        RTFlagCall* flagCall = (RTFlagCall*)rt;
        if (flagCall->func_name.find("FLAG") == string::npos) {
            ostrstream ost;
            ost << "processRT: unexpected kind of RTFLAGCALL: ";
            flagCall->print(ost);
            error(str(ost));
            break;
        } else if (flagCall->actuals.size() == 0) {
            ostrstream ost;
            ost << "processRT: RTFLAGCALL with no parameters: ";
            flagCall->print(ost);
            error(str(ost));
            break;
        }

        Type cType = getFlagCallType(flagCall);
        list<SemStr*>::iterator it = flagCall->actuals.begin();
        const SemStr* p1 = (*it);
        Rtl_ty_loc temp1, temp2;
        Rtl_ty_expr cmp;
        
        if (flagCall->func_name == "LOGICALFLAGS") {
            // Just a single parameter, should be int32
            assert(cType.getType() == INTEGER);
            assert(cType.getSize() == 32);
            temp1 = getTempReg(cType);
            VPOi_rtl(Rtl_assign(temp1, 32, processExpr(p1, cType)), NULL);
            cmp = Rtl_binary(Rtl_op_cmp, Rtl_fetch(temp1, 32), Rtl_int(0));
            VPOi_rtl(Rtl_assign(VPOi_loc_IC, 32, cmp), NULL);
        } else {
            // We expect at least two parameters. We emit IC=compare(p1,p2)
            // where p1 and p2 are the first two parameters to the flag call.
            assert(flagCall->actuals.size() >= 2);
            it++;
            const SemStr* p2 = (*it);
            if (cType.getType() == FLOATP) {
                int currSize = cType.getSize();
                temp1 = getTempReg(cType);
                VPOi_rtl(Rtl_assign(temp1, currSize,
                                    processExpr(p1, cType)), NULL);
                temp2 = getTempReg(cType);
                VPOi_rtl(Rtl_assign(temp2, currSize,
                                    processExpr(p2, cType)), NULL);
                cmp = Rtl_binary(Rtl_op_cmpF,
                                 Rtl_fetch(temp1, currSize),
                                 Rtl_fetch(temp2, currSize));
                VPOi_rtl(Rtl_assign(VPOi_loc_FC, 32, cmp), NULL);
            } else {
                // whether signed or not doesn't matter to getTempReg()
                temp1 = getTempReg(cType);
                VPOi_rtl(Rtl_assign(temp1, 32, processExpr(p1, cType)), NULL);
                temp2 = getTempReg(cType);
                VPOi_rtl(Rtl_assign(temp2, 32, processExpr(p2, cType)), NULL);
                cmp = Rtl_binary(Rtl_op_cmp,
                                 Rtl_fetch(temp1, 32), Rtl_fetch(temp2, 32));
                VPOi_rtl(Rtl_assign(VPOi_loc_IC, 32, cmp), NULL);
            }
        }
        break;
    }
    
    case RTFLAGDEF:
        // we should never see this
        error("processRT: RT flag def: should not be seen!");
        break;
        
    default:
        error("processRT: unexpected HRTL_KIND for RT");
        break;
    }
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::processExpr
 * OVERVIEW:        Emits VPO RTLs for a UQBT SemStr describing an expression.
 * PARAMETERS:      exp: points to a UQBT SemStr describing an expression
 *                  cType: the "current" expected type: if operands are not
 *                    this type, they must be cast.
 * RETURNS:         A VPO Rtl_ty_expr for the expression.
 * NOTE:            The result must be a simple expression that is a
 *                  "Rtl_fetched" VPO register.
 *===========================================================================*/
Rtl_ty_expr SparcIRTLToVPOBackend::processExpr(const SemStr* exp, Type cType) {
    INDEX idx = (INDEX)(exp->getFirstIdx());
    int numArgs = theSemTable[idx].iNumVarArgs;
    int currSize = cType.getSize(); // desired size in bits

    // Handle machine-dependant registers such as %Y or %CF specially
    if ((numArgs == 0) && (idx >= idMachSpec)) {
        return processMachDepReg(exp, cType);
    }

    /*
     * Use Manel's switch/case that are based on two fields: a SemStr index
     * and a number of arguments (0..3). These replace multiple nested
     * switch statements, which are nearly unreadable. The SemStr index is
     * left shifted 2 bits then logical-or'd with the number of arguments
     * to create the switch/case key.
     */
#define SWITCH(nargs,id) switch (((id)<<2) | ((nargs)&0x3))
#define CASE(nargs,id)   case   (((id)<<2) | ((nargs)&0x3))

    SWITCH(numArgs, idx)
    {
        // Nilary operators
        CASE(0,idIntConst): {   // integer constant
            assert(currSize <= 32); // only 32 bit ints for now!
            Rtl_ty_loc constTemp = loadIntConst(exp->getSecondIdx(), cType);
            return Rtl_fetch(constTemp, currSize);
        }

        // Unary operators
        CASE(1,idRegOf):        // register reference
            return Rtl_fetch(processRegOf(exp), currSize);
                
        CASE(1,idMemOf): {      // memory reference
            return processMemoryRead(exp, cType);
        }
            
        CASE(1,idNot):          // logical not
            return processUnaryExpr(Rtl_op_not, exp, cType);
        
        CASE(1,idNeg):          // unary minus
            return processUnaryExpr(Rtl_op_neg, exp, cType);
    
        CASE(1,idSignExt): {    // integer sign extend
            cType.setSigned(true);
            return processExpr(exp->getSubExpr(0), cType);
        }

        // Conversions
        CASE(1,idSize):         // Integer size conversion
        CASE(1,idFsize):        // Floating point size conversion
        CASE(1,idItof):         // Int to floating point (and size) conversion
        CASE(1,idFtoi):         // Floating point to int (and size) conversion
        CASE(1,idZfill):        // Integer zero fill
        CASE(1,idSgnEx):        // Integer sign extend
        CASE(1,idTruncs):       // Integer truncate (signed)
        CASE(1,idTruncu):       // Integer truncate (unsigned)
            return processConversion(exp, cType);

        // Arithmetic operators
        CASE(2,idPlus):         // Addition
            return processBinaryExpr(exp, cType);
        
        CASE(2,idMinus): {      // Subtraction
            if ((exp->getIndex(0) == idMinus) &&
                (exp->getIndex(1) == idRegOf) &&
                (exp->getIndex(2) == idIntConst) &&
                (exp->getIndex(3) == 30) &&       
                (exp->getIndex(4) == idIntConst)) {
                // We want the address of a local variable of the form %fp-NN
                //         -  r[  int  30  int  NN
                //         0   1    2   3    4   5
                // Emit code for &(locals[localsSize - NN])
                int NN = exp->getIndex(5);
                assert(NN >= 0);
                int varOffsetInLocals = (localsSize - NN);
                Rtl_ty_loc varLoc =
                    loadLocalVarAddr(localsSym, varOffsetInLocals);
                return Rtl_fetch(varLoc, 32);
            } else {
                return processBinaryExpr(exp, cType);
            }
        }
        
        CASE(2,idMult):         // Multiply
        CASE(2,idMults):        // Multiply signed
        CASE(2,idDiv):          // Integer division
        CASE(2,idDivs):         // Integer division signed
        CASE(2,idMod):          // Modulus
        CASE(2,idMods):         // Modulus signed
            
        CASE(2,idFPlus):        // FP single-precision addition
        CASE(2,idFMinus):       // FP single-precision subtraction
        CASE(2,idFMult):        // FP single-precision multiply
        CASE(2,idFDiv):         // FP single-precision divide
        CASE(2,idFPlusd):       // FP double-precision addition
        CASE(2,idFMinusd):      // FP double-precision subtraction
        CASE(2,idFMultd):       // FP double-precision multiply
        CASE(2,idFDivd):        // FP double-precision divide

        // Logical operators
        CASE(2,idBitOr):        // |
        CASE(2,idBitAnd):       // &
        CASE(2,idBitXor):       // ^

        // Shift operators
        CASE(2,idShiftL):       // Shift left
        CASE(2,idShiftR):       // Shift right
        CASE(2,idShiftRA):      // Shift left (arithmetic)

        // Comparison operators
        CASE(2,idEquals):       // ==
        CASE(2,idNotEqual):     // !=
        CASE(2,idLess):         // < (signed)
        CASE(2,idGtr):          // > (signed)
        CASE(2,idLessEq):       // <= (signed)
        CASE(2,idGtrEq):        // >= (signed)
        CASE(2,idLessUns):      // < (unsigned)
        CASE(2,idGtrUns):       // > (unsigned)
        CASE(2,idLessEqUns):    // <= (unsigned)
        CASE(2,idGtrEqUns):     // >= (unsigned)
            return processBinaryExpr(exp, cType);

        CASE(3,idAt):           // Bit extraction: e.g., r[tmpl]@32:63
            return processTernaryExpr(exp, cType);

        default:
            ostrstream ost;
            ost << "processExpr: unknown type of expression: " << *exp
                << " at " << hex << rtlAddr;
            error(str(ost));
    }
    return NULL;
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::processMemoryRead
 * OVERVIEW:        Emits VPO RTLs for a SemStr describing a memory load.
 * PARAMETERS:      exp: points to the UQBT SemStr for the memory load.
 *                  cType: the expected type of the value being read.
 * RETURNS:         A VPO Rtl_ty_expr for the value read from memory.
 *===========================================================================*/
Rtl_ty_expr SparcIRTLToVPOBackend::processMemoryRead(const SemStr* exp,
                                                     Type cType) {
    int currSize = cType.getSize(); // desired size in bits
    // get temp reg to hold the value read from the memory location
    Rtl_ty_loc temp = getTempReg(cType);
    Rtl_ty_loc memLoc = processMemOf(exp, cType);
    Rtl_ty_expr expr = Rtl_fetch(memLoc, currSize);
    
    // VPO requires extra "semantic" RTLs to emit 8 and 16 bit loads
    if (cType.getType() == INTEGER) {
        if (currSize == 8) {
            if (cType.getSigned()) {   // LDSB
                expr = Rtl_binary(Rtl_op_lshift,  expr, Rtl_int(24));
                expr = Rtl_binary(Rtl_op_rshiftA, expr, Rtl_int(24));
            } else {                   // LDUB
                expr = Rtl_binary(Rtl_op_and, expr, Rtl_uint(255));
            }
        } else if (currSize == 16) {
            if (cType.getSigned()) {   // LDSH
                expr = Rtl_binary(Rtl_op_lshift,  expr, Rtl_int(16));
                expr = Rtl_binary(Rtl_op_rshiftA, expr, Rtl_int(16));
            } else {                   // LDUH
                expr = Rtl_binary(Rtl_op_and, expr, Rtl_uint(0xffff));
            }
        }
    } // else LD, LDD, LDF, LDDF, which require no special treatment

    // emit the actual load instruction
    VPOi_rtl(Rtl_assign(temp, ((currSize < 32)? 32 : currSize), expr), NULL);
    return Rtl_fetch(temp, currSize);
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::processMachDepReg
 * OVERVIEW:        Emits VPO RTLs for a SemStr describing a machine-dependent
 *                  register expression.
 * PARAMETERS:      exp: points to a UQBT SemStr describing a machine-dependent
 *                    register expression
 *                  cType: the "current" expected type: if operands are not
 *                    this type, they must be cast.
 * RETURNS:         A VPO Rtl_ty_expr for the machine-dependent register expr.
 *===========================================================================*/
Rtl_ty_expr SparcIRTLToVPOBackend::processMachDepReg(const SemStr* exp,
                                                     Type cType) {
    INDEX idx = (INDEX)(exp->getFirstIdx());
    int currSize = cType.getSize(); // desired size in bits
    const string& fullName = theSemTable[idx].sName;
    Rtl_ty_loc temp;
    
    if ((fullName.length() > 1) && 
        (fullName[0] == '%') && (fullName[1] == 'Y')) {
        // treat %Y specially
        return Rtl_fetch(VPOi_loc_YR, currSize);
    }
    
#ifdef SUPPORTING_HRTL
    // Return value of int variable for the register.
    assert(machDepMap.find(fullName) != machDepMap.end());
    AsmSymbol sym = machDepMap[fullName];
    Rtl_ty_loc varLoc = loadLocalVarAddr(sym, 0);
    temp = Rtl_location('m', Rtl_fetch(varLoc, 32));
#else
    // Emit code to test the value of a condition code register and set
    // a temporary reg 1 or 0 depending on its value.
    temp = getTempReg(cType);
    string regName;
    if ((fullName.length() > 0) && (fullName[0] == '%')) {
        regName = fullName.substr(1);
    } else {
        regName = fullName;
    }
    if (regName.length() < 2) {
        ostrstream ost;
        ost << "processExpr: unknown register in expression: " << *exp
            << " at " << hex << rtlAddr;
        error(str(ost));
        return Rtl_fetch(temp, currSize); // must return something...
    }
    
    Rtl_ty_oper op;
    bool isFloat = false;
    if ((regName[0] == 'Z') && (regName[1] == 'F')) {
        op = Rtl_op_eq;     // Z icc set?
    } else if ((regName[0] == 'C') && (regName[1] == 'F')) {
        op = Rtl_op_ltUn;   // C icc set?
    } else if ((regName[0] == 'N') && (regName[1] == 'F')) {
        op = Rtl_op_eq;     // Z icc set?
    } else if ((regName[0] == 'F') && (regName[1] == 'Z')) { // FZF
        op = Rtl_op_eqF;
        isFloat = true;
    } else if ((regName[0] == 'F') && (regName[1] == 'L')) { // FLF
        op = Rtl_op_ltF;
        isFloat = true;
    } else if ((regName[0] == 'F') && (regName[1] == 'G')) { // FGF
        op = Rtl_op_gtF;
        isFloat = true;
    } else {
        // NB: we don't implement idOF yet: no SPARC instruction tests it
        // separately from other flags.
        ostrstream ost;
        ost << "processExpr: unknown register in expression: " << *exp
            << " at " << hex << rtlAddr;
        error(str(ost));
        return Rtl_fetch(temp, currSize); // must return something...
    }
    
    // if flag is set, jump to trueLabel
    AsmSymbol trueLabel  = genLabel();
    AsmSymbol afterLabel = genLabel();
    Rtl_ty_rtl jump;
    
    Rtl_ty_expr guard;
    if (isFloat) {
        guard = Rtl_binary(op, Rtl_fetch(VPOi_loc_FC, 32), Rtl_int(0));
    } else {
        guard = Rtl_binary(op, Rtl_fetch(VPOi_loc_IC, 32), Rtl_int(0));
    }
    jump = Rtl_assign(VPOi_loc_PC, 32, Rtl_relAddr(trueLabel->relAddr));
    VPOi_rtl(Rtl_guard(guard, jump), NULL);
    
    // flag is false: set temp to 0
    VPOi_rtl(Rtl_assign(temp, currSize, Rtl_int(0)), NULL);
    jump = Rtl_assign(VPOi_loc_PC, 32, Rtl_relAddr(afterLabel->relAddr));
    VPOi_rtl(jump, NULL);
    
    // flag is true: set temp to 1
    VPOi_asm->define_symbol_here(trueLabel);
    VPOi_rtl(Rtl_assign(temp, currSize, Rtl_int(1)), NULL);
    
    VPOi_asm->define_symbol_here(afterLabel);
#endif /*SUPPORTING_HRTL*/
    return Rtl_fetch(temp, currSize);
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::processUnaryExpr
 * OVERVIEW:        Emits VPO RTLs for a SemStr describing a unary expression.
 * PARAMETERS:      op: VPO unary operator
 *                  exp: points to a UQBT SemStr describing a unary expression
 *                  cType: the "current" expected type: if operands are not
 *                    this type, they must be cast.
 * RETURNS:         A VPO Rtl_ty_expr for the unary expression.
 *===========================================================================*/
Rtl_ty_expr SparcIRTLToVPOBackend::processUnaryExpr(Rtl_ty_oper op,
                                                    const SemStr* exp,
                                                    Type cType) {
    const SemStr* subExpr = exp->getSubExpr(0);
    int currSize = cType.getSize(); // desired size in bits
    // get temp reg to hold results of subexpression and unary expression
    Rtl_ty_loc resTemp = getTempReg(cType);
    Rtl_ty_expr subExprVal = processExpr(subExpr, cType);
    
    VPOi_rtl(Rtl_assign(resTemp, currSize, subExprVal), NULL);
    VPOi_rtl(Rtl_assign(resTemp, currSize,
                        Rtl_unary(op, Rtl_fetch(resTemp, currSize))), NULL);
    return Rtl_fetch(resTemp, currSize);
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::processBinaryExpr
 * OVERVIEW:        Emits VPO RTLs for a SemStr describing a binary expression.
 * PARAMETERS:      exp: points to a UQBT SemStr describing a binary expression
 *                  cType: the "current" type: if operands are not this
 *                    type, they must be cast.
 * RETURNS:         A VPO Rtl_ty_expr for the binary expression.
 *===========================================================================*/
Rtl_ty_expr SparcIRTLToVPOBackend::processBinaryExpr(const SemStr* exp,
                                                     Type cType) {
    INDEX idx = (INDEX)(exp->getFirstIdx());
    const SemStr* lhs = exp->getSubExpr(0);
    const SemStr* rhs = exp->getSubExpr(1);
    int currSize = cType.getSize(); // desired size in bits
    Rtl_ty_oper op = xlateBinaryOp(idx);
    
    // cast operands where necessary
    switch(idx)
    {
        // unsigned integer operators: cast operands to unsigned
        case idMult:
        case idDiv: 
        case idMod: 
        case idShiftR:
        case idLessUns:
        case idGtrUns:
        case idLessEqUns:
        case idGtrEqUns: {
            cType.setSigned(false);
            break;
        }

        case idFPlus:
        case idFMinus:
        case idFMult:
        case idFDiv: {
            cType.setType(FLOATP);
            // commented out since e.g. idFPlus is also used for double FP add
            // cType.setSize(32);
            break;
        }
    
        case idFPlusd:
        case idFMinusd:
        case idFMultd:
        case idFDivd: {
            cType.setType(FLOATP);
            cType.setSize(64);
            break;
        }

        default: {
            // no cast needed
            break;
        }
    }
    
    Rtl_ty_loc tempL = getTempReg(cType);
    Rtl_ty_loc tempR = getTempReg(cType);
    VPOi_rtl(Rtl_assign(tempL, currSize, processExpr(lhs, cType)), NULL);
    VPOi_rtl(Rtl_assign(tempR, currSize, processExpr(rhs, cType)), NULL);
    Rtl_ty_expr lhsVal = Rtl_fetch(tempL, currSize);
    Rtl_ty_expr rhsVal = Rtl_fetch(tempR, currSize);

    // handle some binary operators specially: e.g. %
    switch(idx)
    {
        case idMods:            // signed mod: round to zero
        case idMod: {           // unsigned mod
            if ((currSize != 32) || (cType.getType() != INTEGER)) {
                ostrstream ost;
                ost << "processBinaryExpr: idMod/idMods only supported "
                    << "for 32 bit integers: " << *exp
                    << " at " << hex << rtlAddr;
                error(str(ost));
                return lhsVal;
            }

            Rtl_ty_loc tempDivd = getTempReg(cType);
            Rtl_ty_loc tempMult = getTempReg(cType);
            if (idx == idMods) {
                // for signed division, put MSB of lhsVal into the LSB of %Y
                Rtl_ty_loc tempMsb = getTempReg(cType);
                Rtl_ty_expr msb =
                    Rtl_binary(Rtl_op_rshiftA, lhsVal, Rtl_int(31));
                VPOi_rtl(Rtl_assign(tempMsb, 32, msb), NULL);
                
                Rtl_ty_expr msbLhs2 =
                    Rtl_binary(Rtl_op_xor, Rtl_fetch(r[0], 32),
                               Rtl_fetch(tempMsb, 32));
                VPOi_rtl(Rtl_assign(VPOi_loc_YR, 32, msbLhs2), NULL);

                // sdiv %l0, %l1, %g1
                Rtl_ty_expr divd = Rtl_binary(Rtl_op_divRz, lhsVal, rhsVal);
                VPOi_rtl(Rtl_assign(tempDivd, 32, divd), NULL);

                // smul %g1, %l1, %g1
                Rtl_ty_expr mult =
                    Rtl_binary(Rtl_op_mul, Rtl_fetch(tempDivd, 32), rhsVal);
                VPOi_rtl(Rtl_assign(tempMult, 32, mult), NULL);
            } else {
                // for unsigned division, clear %Y
                Rtl_ty_expr zero = Rtl_binary(Rtl_op_xor, Rtl_fetch(r[0], 32),
                                              Rtl_fetch(r[0], 32));
                VPOi_rtl(Rtl_assign(VPOi_loc_YR, 32, zero), NULL);

                // udiv %l0, %l1, %g1
                Rtl_ty_expr divd = Rtl_binary(Rtl_op_divUn, lhsVal, rhsVal);
                VPOi_rtl(Rtl_assign(tempDivd, 32, divd), NULL);
                
                // umul %g1, %l1, %g1
                Rtl_ty_expr mult =
                    Rtl_binary(Rtl_op_mulUn, Rtl_fetch(tempDivd, 32), rhsVal);
                VPOi_rtl(Rtl_assign(tempMult, 32, mult), NULL);
            }

            // sub %l0, %g1, %l0
            Rtl_ty_expr modVal = Rtl_binary(Rtl_op_sub, lhsVal,
                                            Rtl_fetch(tempMult, 32));
            VPOi_rtl(Rtl_assign(tempL, 32, modVal), NULL);
            break;
        }
            
        default: {              // norma case: no special treatment needed
            VPOi_rtl(Rtl_assign(tempL, currSize,
                                Rtl_binary(op, lhsVal, rhsVal)), NULL);
            break;
        }
    }
    
    return Rtl_fetch(tempL, currSize);
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::processTernaryExpr
 * OVERVIEW:        Emits VPO RTLs for a SemStr describing a ternary expression
 * PARAMETERS:      exp: points to a UQBT SemStr describing a ternary expr.
 *                  cType: the "current" type: if operands are not this
 *                    type, they must be cast.
 * RETURNS:         A VPO Rtl_ty_expr for the ternary expression.
 *===========================================================================*/
Rtl_ty_expr SparcIRTLToVPOBackend::processTernaryExpr(const SemStr* exp,
                                                      Type cType) {
    INDEX idx = (INDEX)(exp->getFirstIdx());
    int currSize = cType.getSize(); // desired size in bits
    const SemStr* arg1 = exp->getSubExpr(0);
    const SemStr* arg2 = exp->getSubExpr(1);
    const SemStr* arg3 = exp->getSubExpr(2);
    Rtl_ty_loc tempA = getTempReg(cType);
    
    switch (idx) {
        case idAt: {     // a@b:c  e.g. r[tmp]@16:31
            int b = arg2->getSecondIdx();
            if (arg2->getFirstIdx() != idIntConst) b = -1;
            int c = arg3->getSecondIdx();
            if (arg3->getFirstIdx() != idIntConst) c = -1;

	    if ((currSize > 32) || (arg2->getType().getSize() > 32) ||
		(arg3->getType().getSize() > 32)) {
		ostrstream ost;
		ost << "processTernaryExpr: 64 bits not supported: " << *exp;
		error(str(ost));
		break;
	    }

            if ((*arg2) == (*arg3)) {
                // Special case for b == c. Use (((unsigned)a >> b) & 1)
                Type uType(cType);
                uType.setSigned(false);
                VPOi_rtl(Rtl_assign(tempA, 32, processExpr(arg1, uType)), 
			 NULL);
                Rtl_ty_loc tempB = getTempReg(cType);
                VPOi_rtl(Rtl_assign(tempB, 32, processExpr(arg2, cType)), 
			 NULL);
                Rtl_ty_expr rShift =
                    Rtl_binary(Rtl_op_rshift, Rtl_fetch(tempA, 32),
                               Rtl_fetch(tempB, 32));
                VPOi_rtl(Rtl_assign(tempA, 32, rShift), NULL);
                Rtl_ty_expr andExpr =
                    Rtl_binary(Rtl_op_and, Rtl_fetch(tempA, 32), Rtl_uint(1));
                VPOi_rtl(Rtl_assign(tempA, currSize, andExpr), NULL);
            } else {
                // Use (((unsigned)a >> b) & ((1<<(c-b+1))-1)). Assumes b < c!
                Type uType(cType);
                uType.setSigned(false);
                VPOi_rtl(Rtl_assign(tempA, 32, processExpr(arg1, uType)), 
			 NULL);
                Rtl_ty_loc tempB = getTempReg(cType);
                VPOi_rtl(Rtl_assign(tempB, 32, processExpr(arg2, cType)), 
			 NULL);
                Rtl_ty_expr rShift =
                    Rtl_binary(Rtl_op_rshift, Rtl_fetch(tempA, 32),
                               Rtl_fetch(tempB, 32));
                VPOi_rtl(Rtl_assign(tempA, 32, rShift), NULL);
		// tempA now holds ((unsigned)a >> b)
		
		Rtl_ty_loc tempC = getTempReg(cType);
                VPOi_rtl(Rtl_assign(tempC, 32, processExpr(arg3, cType)), 
			 NULL);
		Rtl_ty_expr cMinusB =
                    Rtl_binary(Rtl_op_sub, Rtl_fetch(tempC, 32),
                               Rtl_fetch(tempB, 32));
                VPOi_rtl(Rtl_assign(tempB, 32, cMinusB), NULL);
		Rtl_ty_expr plus1 =
                    Rtl_binary(Rtl_op_add, Rtl_fetch(tempB, 32), Rtl_int(1));
                VPOi_rtl(Rtl_assign(tempB, 32, plus1), NULL);
		// tempB now holds (c-b+1)

		Rtl_ty_expr lShift =
                    Rtl_binary(Rtl_op_lshift, Rtl_int(1), 
			       Rtl_fetch(tempB, 32));
                VPOi_rtl(Rtl_assign(tempB, 32, lShift), NULL);
		Rtl_ty_expr minus1 =
                    Rtl_binary(Rtl_op_sub, Rtl_fetch(tempB, 32), Rtl_int(1));
                VPOi_rtl(Rtl_assign(tempB, 32, minus1), NULL);
		// tempB now has ((1<<(c-b+1))-1))

		Rtl_ty_expr andExpr =
                    Rtl_binary(Rtl_op_and, Rtl_fetch(tempA, 32), 
			       Rtl_fetch(tempB, 32));
                VPOi_rtl(Rtl_assign(tempA, 32, andExpr), NULL);
            }
            break;
        }

        default: {
            ostrstream ost;
            ost << "processTernaryExpr: unknown ternary operator " << idx;
            if ((idx >= 0) && (idx <= idNumOf)) {
                ost << " :" << theSemTable[idx].sName;
            }
            error(str(ost));
        }
    }
    delete arg1;  delete arg2;  delete arg3;
    return Rtl_fetch(tempA, currSize);
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::xlateBinaryOp
 * OVERVIEW:        Returns the VPO operator for a binary SemStr operator.
 * PARAMETERS:      idx: index of the UQBT binary operator.
 * RETURNS:         a binary VPO operator for the SemStr operator.
 *===========================================================================*/
Rtl_ty_oper SparcIRTLToVPOBackend::xlateBinaryOp(INDEX idx) {
    switch(idx)
    {
        case idPlus:     return  Rtl_op_add;
        case idMinus:    return  Rtl_op_sub;
        case idMults:    return  Rtl_op_mul;
        case idMult:     return  Rtl_op_mulUn;
        case idDivs:     return  Rtl_op_divRz;	// signed divide: round to zero
        case idDiv:      return  Rtl_op_divUn;
        case idMods:     return  Rtl_op_modRz;	// signed mod: round to zero
        case idMod:      return  Rtl_op_modUn;

        case idBitOr:    return  Rtl_op_or;
        case idBitAnd:   return  Rtl_op_and;
        case idBitXor:   return  Rtl_op_xor;
        
        case idShiftL:   return  Rtl_op_lshift;
        case idShiftR:   return  Rtl_op_rshift;
        case idShiftRA:  return  Rtl_op_rshiftA;
        
        case idEquals:   return  Rtl_op_eq;
        case idNotEqual: return  Rtl_op_ne;
        case idLess:     return  Rtl_op_lt;
        case idGtr:      return  Rtl_op_gt;
        case idGtrEq:    return  Rtl_op_ge;
        case idLessUns:  return  Rtl_op_ltUn;
        case idGtrUns:   return  Rtl_op_gtUn;
        case idLessEqUns:return  Rtl_op_leUn;
        case idGtrEqUns: return  Rtl_op_geUn;
        
        case idFPlus:
        case idFPlusd:   return  Rtl_op_addF;
        case idFMinus:
        case idFMinusd:  return  Rtl_op_subF;
        case idFMult:
        case idFMultd:   return  Rtl_op_mulF;
        case idFDiv:
        case idFDivd:    return  Rtl_op_divF;
    
        default: {
            ostrstream ost;
            ost << "xlateBinaryOp: unknown binary operator " << idx;
            if ((idx >= 0) && (idx <= idNumOf)) {
                ost << " :" << theSemTable[idx].sName;
            }
            error(str(ost));
            return Rtl_op_xor;
        }
    }
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::processRegOf
 * OVERVIEW:        Return a VPO location (a register) for a UQBT SemStr
 *                  describing a register reference.
 * PARAMETERS:      regRef: points to the SemStr for a register reference.
 * RETURNS:         VPO location for the register reference
 *===========================================================================*/
Rtl_ty_loc SparcIRTLToVPOBackend::processRegOf(const SemStr* regRef) {
    int num;
    int idx = regRef->getSecondIdx();
    if (idx == idTemp) {
        // return already declared a temporary VPO register for this UQBT temp 
        num = regRef->getThirdIdx();
        const string& tempName = theSemTable[num].sName;
        assert(tempMap.find(tempName) != tempMap.end());
        return tempMap[tempName];
    } else if (idx == idIntConst) {
        num = regRef->getThirdIdx();
    } else {
        // must be an expression of form (int const + int const)
        assert(idx == idPlus);
        SemStr* lhs = regRef->getSubExpr(0);
        SemStr* rhs = regRef->getSubExpr(1);
        assert(lhs->getFirstIdx() == idIntConst);
        assert(rhs->getFirstIdx() == idIntConst);
        num = (lhs->getSecondIdx() + rhs->getSecondIdx());
    }

    // Note that SSL encodes FP registers using indexes above 31 (i.e., above
    // the index for %r31=%i7):
    //     indexes 32-63: 32 FP registers %f0-%f31
    //     indexes 64-79: 16 FP double register pairs %f0to1-%f30to31
    //     indexes 80-87: 8 FP quad "register" %f0to3 - %f28to31 (unsupported)
    const Register& reg = prog->RTLDict.DetRegMap[num];
    Type regType = reg.g_type();
    int numBits = regType.getSize();
    
    if (regType.getType() == INTEGER) {
        if ((num < 0) || (num > 31) || (numBits != 32)) {
            ostrstream ost;
            ost << "processRegOf: unknown integer register in " << regRef;
            error(str(ost));
            return r[0];        // must return something
        }
        return r[num];
    } else if (regType.getType() == FLOATP) {
        if (numBits == 32) {
            if ((num < 32) || (num > 63)) {
                ostrstream ost;
                ost << "processRegOf: unknown 32 bit FP register in "<< regRef;
                error(str(ost));
                return f[0];    // must return something
            }
            int fRegNo = (num - 32);
            return f[fRegNo];
        } else if (numBits == 64) {
            if ((num < 64) || (num > 79)) {
                ostrstream ost;
                ost << "processRegOf: unknown 64 bit FP register in "<< regRef;
                error(str(ost));
                return d[0];    // must return something
            }
            int dRegNo = 2*(num - 64);
            return d[dRegNo];
        } else {
            ostrstream ost;
            ost << "processRegOf: unknown 32 bit FP register in "<< regRef;
            error(str(ost));
            return f[0];        // must return something
        }
    }
    
    ostrstream ost;
    ost << "processRegOf: unexpected register type in " << regRef;
    error(str(ost));
    return r[0];                // must return something
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::processMemOf
 * OVERVIEW:        Return a VPO location for a UQBT SemStr describing a
 *                  memory address.
 * PARAMETERS:      memRef: points to the SemStr for a memory adddress.
 *                  cType: the type of the value to be read/written to the
 *                   memory location.
 * RETURNS:         VPO location (a register) for the memory address
 *===========================================================================*/
Rtl_ty_loc SparcIRTLToVPOBackend::processMemOf(const SemStr* memRef,
                                               Type cType) {
    const SemStr* addrExpr = memRef->getSubExpr(0);
    Rtl_ty_expr addr;
    Rtl_ty_loc result;
    
    if ((addrExpr->getIndex(0) == idMinus) &&
        (addrExpr->getIndex(1) == idRegOf) &&
        (addrExpr->getIndex(2) == idIntConst) &&
        (addrExpr->getIndex(3) == 30) &&       
        (addrExpr->getIndex(4) == idIntConst)) {
        // A reference to a local variable of the form m[%fp-NN]
        //         -  r[  int  30  int  NN
        //         0   1    2   3    4   5
        // Emit code for m[locals + (localsSize-NN)] = locals[localsSize-NN]
        int NN = addrExpr->getIndex(5);
        assert(NN >= 0);
        int varOffsetInLocals = (localsSize - NN);
        Rtl_ty_loc varLoc = loadLocalVarAddr(localsSym, varOffsetInLocals);
        addr = Rtl_fetch(varLoc, 32); // fetch address, which is 32 bits
    } else {
        // Normal memory reference. Address expressions are unsigned 32 bit.
        addr = processExpr(addrExpr, Type(INTEGER, 32, false));
    }
    
    if (cType.getType() == INTEGER) {
        result = Rtl_location('m', addr);
    } else if (cType.getType() == FLOATP) {
        // We use 'z' here because it is used by the lcc front end. It is only
        // documented as a "register allocator hack" in the SPARC VPOi
        // extension noweb file (sparcvpoi.nw). 
        result = Rtl_location('z', addr);
    }
    return result;
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::processConversion
 * OVERVIEW:        Emits VPO RTLs for a UQBT SemStr describing a conversion
 *                  expression.
 * PARAMETERS:      exp: points to a UQBT SemStr describing an expression
 *                  cType: the "current" type: if operands are not this
 *                    type, they must be cast.
 * RETURNS:         A VPO Rtl_ty_expr for the expression.
 *===========================================================================*/
Rtl_ty_expr SparcIRTLToVPOBackend::processConversion(const SemStr* exp,
                                                     Type cType) {
    INDEX idx = (INDEX)(exp->getFirstIdx());
    const SemStr* subExpr = exp->getSubExpr(0);
    int fromSize = exp->getSecondIdx();
    int toSize;
    
    Type fromType(cType);
    fromType.setSize(fromSize);

    Type toType(cType);
    if (idx == idSize) {
        toSize = toType.getSize();
    } else {
        toSize = exp->getThirdIdx();
        toType.setSize(toSize);
    }

    // VPO supports SPARC V8, which is 32 bit except for a few instructions
    if ((fromSize > 64) || (toSize > 64)) {
        ostrstream ost;
        ost << "processConversion: unsupported size in: " << *exp;
        error(str(ost));
        return NULL;
    }
    
    // A few conversions change the target type as well as size
    switch (idx) {
        case idItof:
        case idFsize:   toType.setType(FLOATP); break;
        case idFtoi:    toType.setType(INTEGER); break;
        case idZfill:   toType.setSigned(false); break;
        case idSgnEx:   toType.setSigned(true); break;
        case idSize:    toType.setSigned(false); break;
        case idTruncu:  toType.setType(INTEGER);
                        toType.setSigned(false); break;
        case idTruncs:  toType.setType(INTEGER);
                        toType.setSigned(true); break;
        default:
            break;              // interesting cases are handled above
    }
    
    // Some conversions imply the from type as well
    switch (idx) {
        case idFsize:
        case idFtoi:    fromType.setType(FLOATP);  break;
        case idItof:
        case idTruncu:  fromType.setType(INTEGER);
                        fromType.setSigned(false); break;
        case idTruncs:  fromType.setType(INTEGER);
                        fromType.setSigned(true); break;
        case idZfill:   fromType.setType(INTEGER);
                        fromType.setSigned(false); break;
        case idSgnEx:   fromType.setType(INTEGER);
                        fromType.setSigned(true); break;
        // For idSize, make the from type unsigned, so the cast of the
        // memory reference will be correct
        case idSize:    fromType.setSigned(false); break;
        default:
            break;              // interesting cases are handled above
    }

    Rtl_ty_loc temp;
    switch (idx) {
        case idSize: {          // Size conversion
            if (fromSize == toSize) { // just load expr
                temp = getTempReg(toType);
                VPOi_rtl(Rtl_assign(temp, ((toSize < 32)? 32 : toSize),
                                    processExpr(subExpr, fromType)), NULL);
            } else {
                int n = (fromSize - toSize);
                if (n < 0) n = -n;
                temp = processIntConversion(subExpr, fromType, toType,
                                            Rtl_op_rshift, n);
            }
            break;
        }
            
        case idFsize: {         // Floating point size conversion
            Rtl_ty_loc temp1 = getTempReg(fromType);
            VPOi_rtl(Rtl_assign(temp1, fromSize,
                                processExpr(subExpr, fromType)), NULL);

            Rtl_ty_loc temp = getTempReg(toType);
            Rtl_ty_expr cvt;
            if ((fromSize == 32) && (toSize == 64)) {
                // Single to double: FsTOd
                cvt = Rtl_unary(Rtl_op_cvtFD, Rtl_fetch(temp1, 32));
                VPOi_rtl(Rtl_assign(temp, 64, cvt), NULL);
            } else {
                // Double to single: FdTOs
                cvt = Rtl_unary(Rtl_op_cvtDF, Rtl_fetch(temp1, 64));
                VPOi_rtl(Rtl_assign(temp, 32, cvt), NULL);
            }
            break;
        }
            
        case idItof: {          // Int to floating point (and size) conversion
            break;
        }
            
        case idFtoi: {          // Floating point to int (and size) conversion
            break;
        }
            
        case idZfill:           // Integer zero fill
            temp = processIntConversion(subExpr, fromType, toType,
                                        Rtl_op_rshift, (toSize - fromSize));
            break;
            
        case idSgnEx:           // Integer sign extend
            temp = processIntConversion(subExpr, fromType, toType,
                                        Rtl_op_rshiftA, (toSize - fromSize));
            break;
            
        case idTruncs:          // Integer truncate (signed)
            temp = processIntConversion(subExpr, fromType, toType,
                                        Rtl_op_rshiftA, (fromSize - toSize));
            break;
            
        case idTruncu:          // Integer truncate (unsigned)
            temp = processIntConversion(subExpr, fromType, toType,
                                        Rtl_op_rshift, (fromSize - toSize));
            break;

        default: {
            ostrstream ost;
            ost << "processConversion: unexpected operator in expression "
                << *exp;
            error(str(ost));
        }
    }
    return Rtl_fetch(temp, toSize);
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::processIntConversion
 * OVERVIEW:        Emits VPO RTLs for a UQBT SemStr describing a idZfill,
 *                  idSgnEx, idTruncs, or idTruncu conversion.
 * PARAMETERS:      expr: points to a UQBT SemStr describing the conversion
 *                    expression
 *                  fromType: the source type of expr.
 *                  toType: the target type for the resulting expression.
 *                  rshift: Rtl_ty_oper for right shift needed.
 *                  shiftCount: number of bits to shift left then right.
 * RETURNS:         A VPO Rtl_ty_loc for the temporary register holding the
 *                  converted expression.
 *===========================================================================*/
Rtl_ty_loc SparcIRTLToVPOBackend::processIntConversion(const SemStr* expr,
                                                       Type fromType,
                                                       Type toType,
                                                       Rtl_ty_oper rshift,
                                                       int shiftCount) {
    int toSize = toType.getSize();
    Rtl_ty_expr shift;
    
    // get expr's value into a temp reg of size toSize
    Rtl_ty_loc temp = getTempReg(toType);
    VPOi_rtl(Rtl_assign(temp, ((toSize < 32)? 32 : toSize),
                        processExpr(expr, fromType)), NULL);

    if (shiftCount > 0) {    
        shift = Rtl_binary(Rtl_op_lshift, Rtl_fetch(temp, toSize),
                           Rtl_int(shiftCount));
        VPOi_rtl(Rtl_assign(temp, toSize, shift), NULL);
        
        // shift right, sign-extending (if signed) or shifting in zeros.
        shift = Rtl_binary(rshift, Rtl_fetch(temp, toSize),
                           Rtl_int(shiftCount));
        VPOi_rtl(Rtl_assign(temp, toSize, shift), NULL);
    }
    return temp;
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::jumpToBBLabel
 * OVERVIEW:        Emit a jump to the basic block with the specified label.
 * PARAMETERS:      label: integer label for the target basic block.
 * RETURNS:         Nothing.
 *===========================================================================*/
void SparcIRTLToVPOBackend::jumpToBBLabel(int label) {
    ostrstream ost;
    ost << "Jump to basic block with label " << label;
    VPOi_asm->comment(str(ost));
    
    assert(labelMap.find(label) != labelMap.end());
    AsmSymbol labelSym = labelMap[label];
    VPOi_rtl(Rtl_assign(VPOi_loc_PC, 32,
                        Rtl_relAddr(labelSym->relAddr)), NULL);
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::loadLocalVarAddr
 * OVERVIEW:        Emit code to generate the address of a local variable plus
 *                  a constant in a new temporary reg and return that register.
 * PARAMETERS:      sym: a VPO AsmSymbol for a local variable.
 *                  offset: an integer constant.
 * RETURNS:         The temp register holding the address.
 *===========================================================================*/
Rtl_ty_loc SparcIRTLToVPOBackend::loadLocalVarAddr(AsmSymbol sym, int offset) {
    // address expressions are unsigned 32 bit
    Rtl_ty_loc addr = getTempReg(Type(INTEGER, 32, false));
    Rtl_ty_relAddr raddr = sym->relAddr;

    if (offset != 0) {
        assert(fitsInSPARCImm(offset));
        // The offset is small enough to fit in a SPARC immediate field, so use
        // Asm_shiftaddr() to add it to the relocatable address.
        raddr = Asm_shiftaddr(raddr, offset);
    }
    
    Rtl_ty_expr expr =
        Rtl_binary(Rtl_op_add, Rtl_fetch(r[30], 32), Rtl_relAddr(raddr));
    VPOi_rtl(Rtl_assign(addr, 32, expr), NULL);
    return addr;
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::loadSymAddr
 * OVERVIEW:        Emit code to load the address of a symbol into a
 *                  new temporary register and return that register.
 * PARAMETERS:      sym: a VPO AsmSymbol for a symbol.
 * RETURNS:         The temporary register holding the symbol's address.
 *===========================================================================*/
Rtl_ty_loc SparcIRTLToVPOBackend::loadSymAddr(AsmSymbol sym) {
    // address expressions are unsigned 32 bit
    Rtl_ty_loc temp = getTempReg(Type(INTEGER, 32, false));
    Rtl_ty_expr raddr = Rtl_relAddr(sym->relAddr);
    
    // temp := sethi(hi(raddr))
    VPOi_rtl(sethi(hi(raddr), temp), NULL);
    // temp := temp + lo(raddr)
    Rtl_ty_expr expr = Rtl_binary(Rtl_op_add, Rtl_fetch(temp, 32), lo(raddr));
    VPOi_rtl(Rtl_assign(temp, 32, expr), NULL);
    return temp;
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::loadIntConst
 * OVERVIEW:        Emit code to load a 32 bit integer constant into a
 *                  temporary register and return that register.
 * PARAMETERS:      i: an integer.
 *                  cType: the required type for the constant.
 * RETURNS:         The temporary register holding the integer constant.
 *===========================================================================*/
Rtl_ty_loc SparcIRTLToVPOBackend::loadIntConst(int i, Type cType) {
    bool isSigned = cType.getSigned();
    Rtl_ty_loc temp = getTempReg(cType);

    assert(cType.getSize() <= 32);
    // See if constant is small enough to be loaded in one instruction.
    // On the SPARC, larger constants require two steps to load.
    Rtl_ty_expr iconst = (isSigned? Rtl_int(i) : Rtl_uint(i));
    if (fitsInSPARCImm(i)) {
        VPOi_rtl(Rtl_assign(temp, 32, iconst), NULL);
    } else {
        // temp := sethi(hi(iconst))
        VPOi_rtl(sethi(hi(iconst), temp), NULL);

        if (i & 0x3ff) {
            // temp := temp + lo(iconst)
            Rtl_ty_expr expr =
                Rtl_binary(Rtl_op_add, Rtl_fetch(temp, 32), lo(iconst));
            VPOi_rtl(Rtl_assign(temp, 32, expr), NULL);
        }
    }
    return temp;
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::sethi
 * OVERVIEW:        Convenience procedure used to generate a SETHI instruction:
 *                  e.g., VPOi_rtl(sethi(hi(Rtl_relAddr(raddr)), reg), NULL)
 * PARAMETERS:      cnst: a VPO expression rtl (should be result of hi()).
 *                  rd: VPO location describing the target register for sethi.
 * RETURNS:         VPO rtl for the SETHI instruction.
 *===========================================================================*/
Rtl_ty_rtl SparcIRTLToVPOBackend::sethi(Rtl_ty_expr cnst, Rtl_ty_loc rd) {
    return Rtl_assign(rd, 32, cnst);
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::hi
 * OVERVIEW:        Return a VPO expression rtl for (cnst & 0xfffffc00).
 *                  May be used, e.g., to construct a sethi RTL.
 * PARAMETERS:      cnst: a VPO expression rtl
 * RETURNS:         VPO expression rtl masking off low-order 10 bits of cnst.
 *===========================================================================*/
Rtl_ty_expr SparcIRTLToVPOBackend::hi(Rtl_ty_expr cnst) {
    return Rtl_binary(Rtl_op_and, cnst, Rtl_uint(0xfffffc00));
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::lo
 * OVERVIEW:        Return a VPO expression rtl for (cnst & 0x3ff).
 *                  May be used, e.g., to construct a RTL that adds in the
 *                  low-order 10 bits of a word whose high-order 22 bits were
 *                  set with a sethi RTL.
 * PARAMETERS:      cnst: a VPO expression rtl
 * RETURNS:         VPO expression rtl masking off high-order 22 bits of cnst.
 *===========================================================================*/
Rtl_ty_expr SparcIRTLToVPOBackend::lo(Rtl_ty_expr cnst) {
    return Rtl_binary(Rtl_op_and, cnst, Rtl_uint(0x3ff));
}


/*
 *=============================================================================
 * Utility methods.
 *=============================================================================
 */

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::getParamInfo
 * OVERVIEW:        Returns the number of bytes needed for a call to the
 *                  specified address.
 * PARAMETERS:      call: points to HLCall structure for call
 *                  callDest: address of procedure being called
 *                  totalParamBytes: used to return total number of formal
 *                    parameter bytes.
 * RETURNS:         Number of formal parameters for the called procedure.
 *                  Also returns, in totalParamBytes, the bytes needed for
 *                  the parameters.
 *===========================================================================*/
int SparcIRTLToVPOBackend::getParamInfo(const HLCall* call,
                                        ADDRESS callDest,
                                        int& totalParamBytes) {
    Proc* targetProc = prog->findProc(callDest);
    if (targetProc == NULL) {
        cout << "getParamBytes: could not find proc at address 0x" << hex
             << callDest << "\n";
        totalParamBytes = 0;
        return 0;
    }

    int assumedNumParams;
    int numParams = targetProc->getNumArgs();
    if (numParams == 0) {
        cout << "Warning: assuming six parameters for call to "
             << targetProc->getName() << "\n";
        assumedNumParams = 6;
        totalParamBytes = 24;
    } else {
        int total = 0;
        int numBytes;
        assumedNumParams = 0;
        for (int i = 0;  i < numParams;  i++) {
            const SemStr* param = targetProc->getParameter(i);
            const Type& paramType = param->getType();
            if (paramType.getType() == VARARGS) {
                // A varargs parameter (must be last)
                // Assume SIX parameters are passed!
                assumedNumParams = 6;
                total = 24;
            } else {
                assumedNumParams++;
                numBytes = (paramType.getSize() + 7)/8;
                total += numBytes;
            }
        }
        totalParamBytes = total;
    }
    return assumedNumParams;
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::getFlagCallType
 * OVERVIEW:        Return the apparent type of a RTFlagCall based on the type
 *                    of its parameters.
 *                  Assumes:
 *                     o At least one parameter.
 *                     o Either the first parameter or second one (if there)
 *                       is a register reference. If they are a variable,
 *                       expression, or constant, then int32 is returned.
 * PARAMETERS:      fc: const pointer to the RTFlagCall object
 * RETURNS:         Type structure describing RTFlagCall's type.
 *===========================================================================*/
Type SparcIRTLToVPOBackend::getFlagCallType(RTFlagCall* flagCall) {
    Type callType = Type(INTEGER, 32, true); // default type: int32
    bool gotType = false;
    int regNum;
    
    list<SemStr*>::iterator it = flagCall->actuals.begin();
    const SemStr* p1 = (*it);
    if (p1->getFirstIdx() == idRegOf) {
        if (p1->getSecondIdx() == idIntConst) {
            regNum = p1->getThirdIdx();
            // Get the register's intrinsic type
            callType = prog->RTLDict.DetRegMap[regNum].g_type();
            gotType = true;
        } else if (p1->getSecondIdx() == idTemp) {
            regNum = p1->getThirdIdx();
            callType = Type::getTempType(theSemTable[regNum].sName);
            gotType = true;
        }
    }

    if ((!gotType) && (flagCall->actuals.size() >= 2)) {
        it++;
        const SemStr* p2 = (*it);
        if (p2->getFirstIdx() == idRegOf) {
            if (p2->getSecondIdx() == idIntConst) {
                regNum = p2->getThirdIdx();
                callType = prog->RTLDict.DetRegMap[regNum].g_type();
                gotType = true;
            } else if (p2->getSecondIdx() == idTemp) {
                regNum = p2->getThirdIdx();
                callType = Type::getTempType(theSemTable[regNum].sName);
                gotType = true;
            }
        }
    }

    if (!gotType) {
        ostrstream ost;
        ost << "getFlagCallType: could not determine type of RTFLAGCALL: ";
        flagCall->print(ost);
        error(str(ost));
    }
    return callType;
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::fitsInSPARCImm
 * OVERVIEW:        Returns true if the signed integer "iconst" is small
 *                  enough to fit in a SPARC immediate field; false otherwise.
 * PARAMETERS:      iconst: an integer.
 * RETURNS:         true if "iconst" fit in SPARC instruction immediate field.
 *===========================================================================*/
bool SparcIRTLToVPOBackend::fitsInSPARCImm(int iconst) {
    return ((-4096 <= iconst) && (iconst <= 4095));
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::genLabel
 * OVERVIEW:        Return a VPO symbol for a new label. 
 * PARAMETERS:      none.
 * RETURNS:         VPO AsmSymbol for a new SPARC label of the form .L9xxx.
 *                  These labels are distinguished from block labels by the
 *                  large number 9xxx above 9000.
 *===========================================================================*/
AsmSymbol SparcIRTLToVPOBackend::genLabel() {
    AsmSymbol sym;
    char tmpName[20];

    /* Generate the label's name. SPARC labels are of form .Lnnn */
    sprintf(tmpName, ".L%d", nextGenLabelNo);
    nextGenLabelNo++;

    /* Create the symbol and declare it to VPO. */
    sym = VPOi_asm->local(tmpName);
    return sym;
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::replaceNLs
 * OVERVIEW:        Return the argument string with any newlines replaced
 *                  by ";" unless at the end, where a newline is deleted.
 * PARAMETERS:      s: null terminated char * string.
 * RETURNS:         null terminated char * string without embedded newlines.
 *===========================================================================*/
char* SparcIRTLToVPOBackend::replaceNLs(char * s) {
    int len = strlen(s);
    for (int i=0;  i < len;  i++) {
        if (s[i] == '\n') {
            if (i == (len-1)) {
                s[i] = '\0';  // replace the trailing \n by a null
            } else {
                s[i] = ';';
            }
        }
    }
    return s;
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::initRegsAndLocations
 * OVERVIEW:        Initialize VPO registers and locations. Can only be called
 *                  after VPOi_begin().
 * PARAMETERS:      none.
 * RETURNS:         <nothing>
 *===========================================================================*/
void SparcIRTLToVPOBackend::initRegsAndLocations() {
    int i;
    // Initialize the SPARC hardware registers
    for (i = 0;  i < 32;  i++) {
        r[i] = Rtl_constLoc('r', i); // r[0]-r[31]
        f[i] = Rtl_constLoc('f', i); // f[0]-f[31]
        d[i] = ((i & 1) == 0)? Rtl_constLoc('d', i) : 0; // d[0],d[2],d[4],..
    }
    // o registers are a convenient alias for a subset of the r registers
    for (i = 0;  i < 8;  i++) {
        o[i] = r[8 + i];        // o[0] == r[8], o[1] == r[9], ...
    }
    
    // Initialize the caller save and kill location sets. These are the
    // same for all SPARC procedure calls.
    callerSave_set = NULL;
    killed_set = NULL;
    used_set = NULL;
    for (i = 0;  i < 32;  i += 2) {
        // all FP registers are callerSave (saved across calls) and killed
        callerSave_set = VPOi_locSetAdd(callerSave_set, d[i]);
        killed_set     = VPOi_locSetAdd(killed_set,     d[i]);
    }
    for (i = 1;  i < 8;  i++) {
        // %g1-%g7 are also callerSave
        callerSave_set = VPOi_locSetAdd(callerSave_set, r[i]); // %g1-7
    }
    for (i = 1;  i < 16;  i++) {
        // %g1-%g7 and %o0-%o7 are also killed
        killed_set = VPOi_locSetAdd(killed_set, r[i]);
        // and read then overwritten by callee
        used_set = VPOi_locSetAdd(used_set, r[i]);
    }

    // Initialize VPO temporary regs
    curr_t = curr_x = curr_y = 0;
    for (i = 0;  i < NUM_TEMP_REGS;  i++) {
        t[i] = x[i] = y[i] = NULL;
    }
    t_base = x_base = y_base = 0;
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::getTempReg
 * OVERVIEW:        Returns an integer temporary register based on the given
 *                  size in bytes.
 * PARAMETERS:      cType: required type for the register.
 * RETURNS:         Rtl_ty_loc specifying the temporary register's location.
 *===========================================================================*/
Rtl_ty_loc SparcIRTLToVPOBackend::getTempReg(Type cType) {
    switch(cType.getType()) {
        case INTEGER: {
            if (cType.getSize() > 32) {
                ostrstream ost;
                ost << "getTempReg: VPO does not support "
                    << "int temp regs > 32 bits: "
                    << cType.getCtype() << "\n";
                error(str(ost));
                break;
            }

            int new_t = (t_base + curr_t);
            assert(new_t < NUM_TEMP_REGS);
            curr_t++;
            if (t[new_t] == NULL) {
                t[new_t] = Rtl_constLoc('t', new_t);
            }
            return t[new_t];
        }
            
        case FLOATP:
            if (cType.getSize() == 64) { // 64 bit double-precision float reg
                int new_y = (y_base + curr_y);
                assert(new_y < NUM_TEMP_REGS);
                curr_y++;
                if (y[new_y] == NULL) {
                    y[new_y] = Rtl_constLoc('y', new_y);
                }
                return y[new_y];
            } else {                      // 32 bit single-precision float reg
                int new_x = (x_base + curr_x);
                assert(new_x < NUM_TEMP_REGS);
                curr_x++;
                if (x[new_x] == NULL) {
                    x[new_x] = Rtl_constLoc('x', new_x);
                }
                return x[new_x];
            }
            
        default:
            ostrstream ost;
            ost << "getTempReg: unexpected type: " << cType.getCtype();
            error(str(ost));
            break;
    }
    return t[0];                // must return something
}

/*=============================================================================
 * FUNCTION:        SparcIRTLToVPOBackend::resetTempRegs
 * OVERVIEW:        Resets the temporary register pool.
 * PARAMETERS:      none.
 * RETURNS:         nothing.
 *===========================================================================*/
void SparcIRTLToVPOBackend::resetTempRegs() {
    curr_t = t_base;
    curr_x = x_base;
    curr_y = y_base;
}
