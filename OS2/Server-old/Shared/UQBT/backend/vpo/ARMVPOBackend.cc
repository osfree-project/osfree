/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*=============================================================================
 * FILE:       ARMVPOBackend.cc
 * OVERVIEW:   Translates the UQBT HRTL IR for a procedure into VPO (Very
 *             Portable Optimizer) RTLs for the ARM processor. This file is
 *             a "code expander" in VPO terminology and emits VPO RTLs and
 *             other directives for the procedure.
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 *===========================================================================*/

/*
 * $Revision: 1.8 $
 *  06 Jun 01 - Brian: Initial version based on SparcIRTLToVPOBackend.cc,
 *                which was based originally on some 1999 work by Mike.
 *  02 Aug 01 - Brian: New class HRTL replaces RTlist.
 *  06 Sep 01 - Brian: Added support for library intrinsics (e.g. for divide).
 *                Corrected location sets passed to VPO for procedure calls.
 *                Parameter passing fixes. Modified use of temp locations to
 *                work around VPO reg allocation problem. Reworked JCond code
 *                to work around VPO limitation.
 *  11 Sep 01 - Brian: Added byte-swapping support.
 *  20 Sep 01 - Brian: Now using VPOi_functionCall() location sets from the
 *                EDG front end. Corrected code for reference to %afp by itself.
 *                Added support for ~= (idNotEqual) expression. Corrected arg
 *                passing code when more than 4 args are passed and either float
 *                or 16 bit ints are passed. Improved treatement of 16 bit
 *                expression operands. Added support for idAddrOf.
 *  01 Oct 01 - Brian: Must manage the storage of symbolic vars such as v0, v1
 *                explicitly rather than relying on VPO to allocate each
 *                separately since these must be allocated sequentially
 *                (e.g., addresses of these are passed in procedure calls).
 *                Added support for out-of-line byte swaps. Fixed code to pass
 *                multi-word parameters. Added byte swaps when storing argv.
 *  17 Oct 01 - Brian: Added memory temp used for, e.g., arg passing, int<->FP
 *                assignments, etc. Assignment sizes now override var/reg sizes.
 *                Implemented FP constants, idCastIntStar, 64 bit assignments,
 *                and "bitwise" casts. Fixed bug in allocation of memory for
 *                temporary regs. 
 */

#include "global.h"
#include "prog.h"
#include "proc.h"
#include "cfg.h"
#include "ss.h"
#include "hrtl.h"
#include "type.h"
#include "options.h"
#include "BinaryFile.h"
#include "ARMVPOBackend.h"

// Names of sections for VPOi_asm->section()
#define DATA_SEG ".rodata"
#define TEXT_SEG ".text"

// VPO needs to know the offset of each parameter relative to %SP.
// The stack offset of the first parameter is FIRST_PARAM_OFFSET
#define FIRST_PARAM_OFFSET 16

// First number used when creating names of generated labels. Label names are
// of form .Lnnnn. Block labels use labels with names based on the (small)
// block number.
#define FIRST_GEN_LABEL_NO 9000

// Comment out to call utility procedures in _util.c to do byte swaps.
#define USE_INLINE_SWAPS 1


/*=============================================================================
 * FUNCTION:        ARMVPOBackend::ARMVPOBackend
 * OVERVIEW:        Constructor for class ARMVPOBackend
 * PARAMETERS:      program: reference to Prog object for program being
 *                  processed.
 * RETURNS:         <nothing>
 *===========================================================================*/
ARMVPOBackend::ARMVPOBackend(Prog& program) {
    prog = &program;
    binFile = prog->pBF;
    
    // initialize pattern SemStrs used to search for refs to regs and temp regs.
    regRefPattern.push(idRegOf);
    regRefPattern.push(idIntConst);
    regRefPattern.push(WILD);
    
    tempRefPattern.push(idRegOf);
    tempRefPattern.push(idTemp);
    tempRefPattern.push(WILD);
    
    // initialize sets of registers used when processing procedure calls
    callerSave_set = NULL;
    killed_set = NULL;
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::~ARMVPOBackend
 * OVERVIEW:        Destructor for class ARMVPOBackend.
 * PARAMETERS:      none.
 * RETURNS:         <nothing>
 *===========================================================================*/
ARMVPOBackend::~ARMVPOBackend() {
}

/*=============================================================================
 * FUNCTION:    ARMVPOBackend::expandFunction
 * OVERVIEW:    Emit VPO RTLs and other directives for a procedure. Creates
 *              a <procname>.cex file for input to the VPO optimizer.
 * PARAMETERS:  proc: pointer to this procedure's Proc object.
 * RETURNS:     <nothing>
 *===========================================================================*/
void ARMVPOBackend::expandFunction(UserProc* userProc) {
    if (userProc->getEntryBB() == NULL) {
        cout << "expandFunction: no entry block, quitting" << endl;
        return;             // Don't attempt to generate code
    }

    // Initialize or reset member variables
    proc = userProc;
    procName = proc->getName();
    doingMemorySwaps = ((SRCENDIAN != TGTENDIAN) && !progOptions.noendian);
    
    nextTempVarNo  = 0;
    nextGenLabelNo = FIRST_GEN_LABEL_NO;
    atLastBB = false;
    hrtlAddr = 0;

    labelMap.clear();
    procMap.clear();
    regsUsed.clear();
    regVarMap.clear();
    symVarOffsetMap.clear();
    tempRegsUsed.clear();
    tempRegVarMap.clear();
    machDepIdx.clear();
    machDepMap.clear();
    defFConstMap.clear();
    defIConstMap.clear();
    
    // Translate the procedure
    if (progOptions.proctrace || progOptions.verbose) {
        cout << "ARM VPO backend: procedure \"" << procName << "\"" << endl;
    }
    preamble();
    firstPass();
    secondPass();
    postamble();
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::preamble
 * OVERVIEW:        Emit VPO directives for the start of a procedure.
 * PARAMETERS:      none.
 * RETURNS:         <nothing>
 *===========================================================================*/
void ARMVPOBackend::preamble() {
    // get the name of the output (.cex) file
    string fileNameStr = progOptions.outDir + procName + string(".cex");
    const char* fileName = fileNameStr.c_str();
    const char* argv[] = {"-o", fileName};
    
    // Initialize VPOi and asm interfaces
    // NB: This assumes that Brian's patch to vpoi.c.nw has been installed
    // that allows the VPOI outfile file to be specified. 
    VPOi_begin("arm-linux", 2, argv);

    ostrstream ost;
    ost << "Procedure " << procName;
    VPOi_asm->comment(str(ost));
    VPOi_sourceFile((char*)procName);
    
    // Initialize processor specific globals. VPOi_begin() must be called first
    initRegsAndLocations();
    
    // Declare to VPO various runtime routines such as __divsi3 (integer divide)
    declareLibraryRoutines();
    
    // Emit the assembly preamble for the procedure
    VPOi_asm->section(TEXT_SEG);
    VPOi_asm->align(8);
    
    AsmSymbol procSym = VPOi_asm->export_sym(procName);
    // Record procedure's asm symbol to avoid a VPO error later if procedure
    // is called recursively (we can't export() and import() the same symbol).
    string procNameStr = procName;
    procMap[procNameStr] = procSym;
    VPOi_globalDeclare(procSym, 'r');
    
    // Label the start of the procedure and tell VPO about the procedure
    VPOi_asm->define_symbol_here(procSym);
    VPOi_functionDefine(procSym, /*callee save regs*/ NULL);
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::postamble
 * OVERVIEW:        Emit VPO directives for the end of a procedure.
 * PARAMETERS:      none
 * RETURNS:         <nothing>
 *===========================================================================*/
void ARMVPOBackend::postamble() {
    VPOi_functionEnd();
    VPOi_end();
}


/*
 *=============================================================================
 * First pass: scan for references and declare imports, exports, vars.
 *=============================================================================
 */

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::firstPass
 * OVERVIEW:        Does several things:
 *                    o Declares VPO labels for labelled blocks and places
 *                      them in labelMap, a map from BB labels to AsmSymbols.
 *                    o Declares called procedures to VPO.
 *                    o Declares VPO var for each register SemStr of form
 *                      r[ int NN and registers them in regVarMap.
 *                    o Declares VPO temp regs for each temp reg SemStr of form
 *                      r[ temp T and registers them in tempRegVarMap.
 *                    o Looks for SemStrs with IDs >= idMachSpec. Declares
 *                      these machine-specific regs (like %CF, %Y) as VPO int
 *                      vars and registers them in machDepMap.
 * PARAMETERS:      none.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::firstPass() {
    Cfg* cfg = proc->getCFG();
    cfg->sortByAddress();

    // If the entry BB is not the first BB, set a label for the entry BB.
    // Note: using -S, the proc may have no BBs, so firstBB may be NULL here.
    BB_CIT bbIter;
    PBB firstBB = cfg->getFirstBB(bbIter);
    if ((firstBB != NULL) &&
        (firstBB->getLowAddr() != proc->getNativeAddress()) &&
        (proc->getEntryBB()->getLabel() == 0)) {
        cfg->setLabel(proc->getEntryBB());
    }

    // Scan each basic block
    for (PBB bb = firstBB;  bb != NULL;  bb = cfg->getNextBB(bbIter)) {
        // create VPO labels for BasicBlock labels (which are integers)
        int label = bb->getLabel();
        if (label != 0) {
            ostrstream lName;
            lName << ".L" << dec << label; // ARM VPO labels look like .Lnnnn
            AsmSymbol labelSym = VPOi_asm->local(str(lName));
            labelMap[label] = labelSym;
        }
        
        // scan the block's HRTLs searching for ref'd regs, temps, vars, etc.
        scanHRTLsInBlock(bb);
    }

    declareParameters();
    declareLocalsArray();       // _locals[] for holding m[%afp+NN] vars
    declareSymbolicVars();      // _symVars[] for e.g., v2 or v8
    declareTempRegsAsVars();
    declareRegsAsVars();
    declareMachDepRegsAsVars();
    declareMemoryTemp();
    
    // Set bases for allocating new VPO temp regs during the second pass.
    // These are after any temps allocated for idRegOf idTemp (r[ temp).
    t_base = curr_t;
    x_base = curr_x;
    y_base = curr_y;
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::scanHRTLsInBlock
 * OVERVIEW:        Search a block's HRTLs searching for referenced registers,
 *                  temporary registers, and called procedures.
 *                  Declare the called procedures to VPO. Called during the
 *                  first pass.
 * PARAMETERS:      bb: a pointer to a BasicBlock.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::scanHRTLsInBlock(PBB bb) {
    int i;
    HRTLList* hrtlList = bb->getHRTLs();
    if (hrtlList != NULL) {
        HRTLList_IT rit;
        for (rit = hrtlList->begin();  rit != hrtlList->end();  rit++) {
            HRTL* hrtl = (*rit);
            int n = hrtl->getNumRT();
            for (i = 0;  i < n;  i++) {
                RT* rt = hrtl->elementAt(i);
                scanForRefsInRT(rt);
            }
            
            HRTL_KIND kd = hrtl->getKind();
            if (kd == CALL_HRTL) {
                const HLCall* call = static_cast<const HLCall*>(hrtl);
                // declare the called procedure
                declareCalledProc(call);

                // scan the arguments
                const list<SemStr>& params = call->getParams();
                list<SemStr>::const_iterator pp;
                for (pp = params.begin();  pp != params.end();  pp++) {
                    SemStr param = *pp;
                    const Type& paramType = param.getType();
                    
                    if (paramType.getType() == VARARGS) {
                        continue;
                    }

                    scanForRefs(&param);
                }

                // scan the return location
                SemStr retLoc = call->getReturnLoc();
                scanForRefs(&retLoc);
                
                // if there are post call semantics, iterate through those RTs
                const list<RT*>* lrt = call->getPostCallRtlist();
                if (lrt != NULL) {
                    for (RT_CIT it = lrt->begin(); it != lrt->end(); it++){
                        RT* rt = (*it);
                        scanForRefsInRT(rt);
                    }
                }
            } else if (kd == RET_HRTL) {
                const SemStr* retLoc = proc->getReturnLoc();
                // Need const cast because scanForRefs can't be made const,
                // because searchAll isn't const
                scanForRefs(const_cast<SemStr*>(retLoc));
            }
            // Other HL types may have to be considered in the future
        }
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::scanForRefsInRT
 * OVERVIEW:        Scan an RT searching for referenced registers,
 *                  temporary registers, and machine-specific Ids.
 *                  Called during the first pass.
 * PARAMETERS:      rt: a pointer to a RT.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::scanForRefsInRT(RT* rt) {
    if (rt->getKind() == RTASSGN) {
        RTAssgn* assign = (RTAssgn*)rt;
        SemStr* lhs = assign->getLHS();
        SemStr* rhs = assign->getRHS();
        scanForRefs(lhs);
        scanForRefs(rhs);
    }
}
    
/*=============================================================================
 * FUNCTION:        ARMVPOBackend::scanForRefs
 * OVERVIEW:        Scan a SemStr searching for referenced registers,
 *                  temporary registers, and machine-specific Ids.
 *                  Called during the first pass.
 * PARAMETERS:      ss: a pointer to a SemStr.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::scanForRefs(SemStr* ss) {
    scanForRegsUsed(ss);
    scanForTempRegsUsed(ss);
    ss->searchMachSpec(machDepIdx); // scan for machine specific IDs like %CF
}
    
/*=============================================================================
 * FUNCTION:        ARMVPOBackend::scanForRegsUsed
 * OVERVIEW:        Search for SemStrs of form r[ int NN. Add the number of
 *                  each register X found to regsUsed, the set of used
 *                  register numbers.
 * PARAMETERS:      ss: Points to a SemStr structure.
 * RETURNS:         <nothing>
 *===========================================================================*/
void ARMVPOBackend::scanForRegsUsed(SemStr* ss) {
    // search for SemStrs of form r[ int X
    list<SemStr*> result;
    if (ss->searchAll(regRefPattern, result)) {
        // we have at least one register; go through the list
        list<SemStr*>::iterator ssIter;
        for (ssIter = result.begin();  ssIter != result.end();  ssIter++) {
            SemStr* regRefSS = (*ssIter);
            int NN = regRefSS->getThirdIdx();
            regsUsed.insert(NN);
        }
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::scanForTempRegsUsed
 * OVERVIEW:        Search for SemStrs of form r[ temp T. Add the name of
 *                  each register X found to tempRegsUsed, the set of used
 *                  temp register names.
 * PARAMETERS:      ss: Points to a SemStr structure.
 * RETURNS:         <nothing>
 *===========================================================================*/
void ARMVPOBackend::scanForTempRegsUsed(SemStr* ss) {
    // search for SemStrs of form r[ temp X  (i.e., idRegOf idTemp WILD)
    list<SemStr*> result;
    if (ss->searchAll(tempRefPattern, result)) {
        // we have at least one temp register name; go through the list
        list<SemStr*>::iterator ssIter;
        for (ssIter = result.begin();  ssIter != result.end();  ssIter++) {
            SemStr* tempRefSS = (*ssIter);
            const string& tempName =
                theSemTable[tempRefSS->getThirdIdx()].sName;
            tempRegsUsed.insert(tempName);
        }
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::declareCalledProc
 * OVERVIEW:        Emit VPO declarations needed to reference a called
 *                  procedure. Called during the first pass.
 * PARAMETERS:      rt: a pointer to a RT.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::declareCalledProc(const HLCall* call) {
    ADDRESS destAddr = call->getFixedDest();
    //const char* calleeName = call->getDestName();
    const char* calleeName = binFile->SymbolByAddress(destAddr);

    if (calleeName == NULL) {
        if (destAddr == NO_ADDRESS) {
            // A register call
            return;
        } else {
            ostrstream ost;
            ost << "Calls to procedures without names are not implemented: ";
            call->getDest()->print(ost);
            error(str(ost));
            return;
        }
    }
    
    // Declare to VPO the procedure being called, if new
    AsmSymbol calleeSym;
    string calleeNameStr = calleeName;
    if (procMap.find(calleeNameStr) == procMap.end()) {
        calleeSym = VPOi_asm->import(calleeName);
        procMap[calleeNameStr] = calleeSym;
        VPOi_globalDeclare(calleeSym, 'r');
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::declareParameters
 * OVERVIEW:        Declare the parameters for a procedure to VPO.
 *                  Called during the first pass.
 * PARAMETERS:      None.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::declareParameters() {
    int numParams = proc->getNumArgs();
    // if using -S (parse Single procedure), may not be any formal parameters
    if (numParams > 0) {
        const list<SemStr>& params = proc->getParams();
        int offset = FIRST_PARAM_OFFSET;
        list<SemStr>::const_iterator pp;
        for (pp = params.begin();  pp != params.end();  pp++) {
            SemStr param = *pp;
            Type paramType = param.getType();
            ostrstream ost;
            if (paramType.getType() == VARARGS) {
                ost << "Varargs parameters are not implemented: "
                    << procName << " parameter " << param;
                error(str(ost));
            } else if (param.getFirstIdx() != idVar) {
                // Process "register" vars like r[2]. Symbolic vars like
                // v0,V1,... are handled below by declareSymbolicVars().
                int numBits = paramType.getSize();
                int numBytes = ((numBits + 7)/8);
                numBytes = max(numBytes, 4); // 4 byte min for each parameter
                
                // construct a name for the parameter; change r[NN] to rNN
                ost << ".2_";
                param.printAsC(ost);
                char* var_name = str(ost);

                assert(var_name[3] == 'r');
                assert((param.getFirstIdx() == idRegOf) &&
                       (param.getSecondIdx() == idIntConst));
                
                // register the VPO variable for the parameter
                AsmSymbol sym = VPOi_asm->local(var_name);
                char regType = regTypeForType(paramType);
                Rtl_ty_expr addr =
                    Rtl_binary(Rtl_op_add, Rtl_fetch(fp, 32), Rtl_int(offset));
                Rtl_ty_loc loc = Rtl_location('m', addr);
                VPOi_parameterDeclare(sym, regType, numBytes, /*volatile*/ 0,
                                      loc);
                
                int reg = param.getThirdIdx();
                assert(regVarMap.find(reg) == regVarMap.end());
                regVarMap[reg] = sym;
                
                if ((numBytes == 4) || (numBytes == 8)) {
                    offset += numBytes;
                } else {
                    offset += ((numBytes + 3)/4) * 4; // round up to next word
                }
            }
                
        }
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::declareLocalsArray
 * OVERVIEW:        Declare a VPO variable "locals" used to hold local
 *                  variables that are referenced via m[%afp + NN] and
 *                  %afp + NN (when their address is taken).
 *                  Called during the first pass.
 * PARAMETERS:      None.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::declareLocalsArray() {
    int localsSize = proc->getLocalsSize();
    if (localsSize > 0) {
        localsSym = VPOi_asm->local(".2_locals");
        VPOi_localVariableDeclare(localsSym, 'r', localsSize, /*volat*/ 0,
                                  /*block*/ 2);
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::declareSymbolicVars
 * OVERVIEW:        Declare the array (symVarsSym) that holds symbolic variables
 *                  such as v[0], v[12].
 *                  Called during the first pass.
 * PARAMETERS:      None.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::declareSymbolicVars() {
    if (proc->getLastLocalIndex() != -1) { // there are some symbolic vars 
        int symVarBytes = 0;
        const vector<SemStr>& symVars = proc->getSymbolicLocals();
        for (vector<SemStr>::const_iterator it = symVars.begin();
                it != symVars.end();  it++) {
            int NN = it->getSecondIdx();
            const Type& varType = it->getType();
            int varBytes = (varType.getSize() / 8);

            // halfword and larger values must be aligned properly
            int varOffset = (symVarBytes + (varBytes-1)) & (~(varBytes-1));
            
            // record offset for this symbolic var in the symVarsSym array
            assert(symVarOffsetMap.find(NN) == symVarOffsetMap.end());
            symVarOffsetMap[NN] = varOffset;
            
            symVarBytes = (varOffset + varBytes);
        }

        symVarsSym = VPOi_asm->local(".2_symVars");
        VPOi_localVariableDeclare(symVarsSym, 'r', symVarBytes, /*volat*/ 0,
                                  /*block*/ 2);
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::declareRegsAsVars
 * OVERVIEW:        Declare each UQBT register as a VPO variable and record
 *                  that variable in regVarMap. Called during the first pass.
 * PARAMETERS:      None.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::declareRegsAsVars() {
    if (regsUsed.size() > 0) {
        set<int>::iterator it;
        for (it = regsUsed.begin();  it != regsUsed.end();  it++) {
            int reg = (*it);
#ifdef OVERLAP_REGS_IMPL            
            if (ovl.isOverlappedReg(reg)) {
                // Already declared by declareRegs()
                continue;
            }
#endif

            // if not already declared, declare a var for the register
            if (regVarMap.find(reg) == regVarMap.end()) {
                const Register& regInfo = prog->RTLDict.DetRegMap[reg];
                Type regType = regInfo.g_type();
                int numBytes = (regType.getSize() + 7)/8;
                char var_name[20];
                
                // generate the register's variable name and declare it
                sprintf(var_name, ".2_r%d", reg);
                AsmSymbol sym = VPOi_asm->local(var_name);
                VPOi_localVariableDeclare(sym, 'r', numBytes, /*volat*/ 0,
                                          /*block*/ 2);
                
                // add the VPO variable to regVarMap
                regVarMap[reg] = sym;
            }
        }
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::declareTempRegsAsVars
 * OVERVIEW:        Declare each UQBT temp register used as a VPO variable and
 *                  record that variable in tempRegVarMap. Called during the
 *                  first pass.
 * PARAMETERS:      None.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::declareTempRegsAsVars() {
    if (tempRegsUsed.size() > 0) {
        set<string>::iterator it;
        for (it = tempRegsUsed.begin();  it != tempRegsUsed.end();  it++) {
            string tempName = (*it);
            // if not already declared, declare a var for the register
            if (tempRegVarMap.find(tempName) == tempRegVarMap.end()) {
                char var_name[20];
                sprintf(var_name, ".2_%s", tempName.c_str());
                AsmSymbol sym = VPOi_asm->local(var_name);

                // temps are all int32 unless last char is of form "d", "l", ...
                int numBytes = 4;
                if ((tempName.length() > 3) &&
                    (tempName[0] == 't') && (tempName[1] == 'm') &&
                    (tempName[2] == 'p')) {
                    char typeCh = tempName[4];
                    if (typeCh == 'b') {
                        numBytes = 1;
                    } else if (typeCh == 'h') {
                        numBytes = 2;
                    } else if (typeCh == 'd') {
                        numBytes = 8;
                    } else if (typeCh == 'f') {
                        numBytes = 4;
                    } else if (typeCh == 'l') {
                        numBytes = 8;
                    }
                }
                VPOi_localVariableDeclare(sym, 'r', numBytes, /*volat*/ 0,
                                          /*block*/ 2);
                
                // add the VPO variable to regVarMap
                tempRegVarMap[tempName] = sym;
            }
        }
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::declareMachDepRegsAsVars
 * OVERVIEW:        Declare machine-dependant registers such as %Y and %CF
 *                  as integer local vars. Called during the first pass.
 * PARAMETERS:      None.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::declareMachDepRegsAsVars() {
    set<INDEX>::iterator its;
    for (its = machDepIdx.begin();  its != machDepIdx.end();  its++) {
        const string& regName = theSemTable[*its].sName;
        ostrstream ost;
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

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::declareMemoryTemp
 * OVERVIEW:        Declare a 64 bit memory temp for use when, e.g., doing byte
 *                  swaps or when passing 64 bit parameters to procedures.
 * PARAMETERS:      None.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::declareMemoryTemp() {
    memTempSym = VPOi_asm->local(".2_mem64Temp");
    VPOi_localVariableDeclare(memTempSym, 'D', /*numBytes*/ 8, /*volat*/ 0,
                              /*block*/ 2);
}


/*
 *=============================================================================
 * Second pass: emit code for UQBT HRTLs.
 *=============================================================================
 */

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::secondPass
 * OVERVIEW:        Emit VPO RTLs and directives for the body of a procedure.
 * PARAMETERS:      none.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::secondPass() {
    Cfg* cfg = proc->getCFG();
    BB_CIT bbIter;

    // Emit code to store the incoming parameter registers into the variables
    // allocated to hold those parameters.
    storeIncomingParams();

    // If translating main(argc, argv,...) and if necessary (and unless
    // progOptions.noendian), emit code to swap the elements of argv. Then
    // we can just swap all memory references.
    if ((strcmp(proc->getName(), "main") == 0) && doingMemorySwaps) {
        emitArgvSwaps();
    }
    
    // If the entry BB is not the first BB, emit a branch to the entry BB.
    // Note: using -S, the proc may have no BBs, so firstBB may be NULL here.
    PBB firstBB = cfg->getFirstBB(bbIter);
    if ((firstBB != NULL) &&
            (firstBB->getLowAddr() != proc->getNativeAddress())) {
        PBB entryBB = proc->getEntryBB();
        int entryLabel = entryBB->getLabel();
        jumpToBBLabel(entryLabel);
    }

    // Emit code for each basic block.
    PBB nextBB = NULL;
    for (PBB bb = firstBB;  bb != NULL;  bb = nextBB) {
        nextBB = cfg->getNextBB(bbIter);
        atLastBB = (nextBB == NULL);
        
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
        HRTLList* hrtlList = bb->getHRTLs();
        if (hrtlList != NULL) {
            HRTLList_CIT rit;
            for (rit = hrtlList->begin();  rit != hrtlList->end();  rit++) {
                HRTL* hrtl = (*rit);
                bool isCommented = hrtl->getCommented();
                hrtlAddr = hrtl->getAddress(); // set address for error messages
                if (hrtl->getKind() != LOW_LEVEL_HRTL) {
                    processHRTL(hrtl, bb);
                } else {
                    int n = hrtl->getNumRT();
                    if (isCommented) {
                        ostrstream ost;
                        ost << "The following HRTLs are commented out";
                        VPOi_asm->comment(str(ost));
                    }
                    for (int i = 0;  i < n;  i++) {
                        RT* rt = hrtl->elementAt(i);
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

        // Emit jump if required. Necessary jumps for other kinds of basic
        // blocks were handled in processHL() or processRT().
        if ((bb->getType() == FALL) && (bb->isJumpReqd())) {
            int label = bb->getOutEdges()[0]->getLabel(); 
            if (atLastBB) {
                emitDeferredConstants();
            }
            jumpToBBLabel(label);
        }
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::storeIncomingParams
 * OVERVIEW:        Store a procedure's incoming arguments from registers and
 *                  possibly the stack to the variables allocated to hold
 *                  formal paramaters.
 * PARAMETERS:      none.
 * RETURNS:         nothing.
 *===========================================================================*/
void ARMVPOBackend::storeIncomingParams() {
    int numParams = proc->getNumArgs();
    if (numParams == 0) {
        // if using -S (parse Single procedure), may be no formal parameters
        return;
    }

    int nextWord = 0;       // next parameter word to read from reg or stack
    const list<SemStr>& params = proc->getParams();
    list<SemStr>::const_iterator pp;
    for (pp = params.begin();  pp != params.end();  pp++) {
        SemStr param = *pp;
        Type paramType = param.getType();
        
        if (paramType.getType() == VARARGS) {
            continue;       // error already displayed
        }
        
        int nBits = paramType.getSize();
        int nBytes = ((nBits + 7)/8);
        nBytes = max(nBytes, 4); // 4 byte min for each parameter
        int nWords = nBytes/4;
        assert((nWords == 1) || (nWords == 2));
        
        Rtl_ty_storage memType = memoryTypeForType(paramType);
        Type wordTempType = Type(INTEGER, 32, false);
        Rtl_ty_expr addr, word, dblword;
        Rtl_ty_loc dest;
        
        // look up the VPO AsmSymbol or symbolic var number for the parameter
        string paramName = param.sprint();
        bool isReg = (paramName[0] == 'r');
        AsmSymbol paramSym;     // if isReg: "register" parameter such as r[2]
        int symVarOffset = 0;   // if !isReg: off. of sym var in symVarsSym arr
        if (isReg) {
            int reg = param.getThirdIdx();
            assert(regVarMap.find(reg) != regVarMap.end());
            paramSym = regVarMap[reg];
        } else {
            assert(param.getFirstIdx() == idVar);
            int NN = param.getSecondIdx();
            assert(symVarOffsetMap.find(NN) != symVarOffsetMap.end());
            symVarOffset = symVarOffsetMap[NN];
        }
        
        // Assign the actual param word(s) to the formal parameter location.
        if ((nextWord + nWords) <= 4) {
            // The actual parameter is completely in one or more of r[0-3].
            int paramReg = nextWord;
            if (nWords == 2) {
                // The 64 bit param's words are in adjacent regs; write the
                // two regs into argTemp's two words. 
                word = Rtl_fetch(r[paramReg], 32);
                dest = Rtl_location('m', getSymAddr(fp, memTempSym, 0));
                VPOi_rtl(Rtl_assign(dest, 32, word), NULL);
                
                word = Rtl_fetch(r[paramReg + 1], 32);
                dest = Rtl_location('m', getSymAddr(fp, memTempSym, 4));
                VPOi_rtl(Rtl_assign(dest, 32, word), NULL);

                // load doubleword into paramSym or symVarsSym[symVarOffset]
                if (isReg) {
                    dest = Rtl_location(memType, getSymAddr(fp, paramSym, 0));
                } else {
                    dest = Rtl_location(memType, getSymAddr(fp, symVarsSym,
                                                            symVarOffset));
                }
                addr = getSymAddr(fp, memTempSym, 0);
                dblword = Rtl_fetch(Rtl_location(memType, addr), 64);
                VPOi_rtl(Rtl_assign(dest, 64, dblword), NULL);
            } else {
                word = Rtl_fetch(r[paramReg], 32);
                if (isReg) {
                    dest = Rtl_location(memType, getSymAddr(fp, paramSym, 0));
                } else {
                    dest = Rtl_location(memType, getSymAddr(fp, symVarsSym,
                                                            symVarOffset));
                }
                VPOi_rtl(Rtl_assign(dest, 32, word), NULL);
            }
        } else if (nextWord >= 4) {
            // The actual parameter is entirely on the stack.
            Rtl_ty_loc temp = getTempReg(wordTempType);
            int spOffset = 4*(nextWord - 4); // load lower word from m[spOffset]
            if (nWords == 2) {
                // load lower addressed word of param from stack at lower addr
                addr = Rtl_binary(Rtl_op_add, Rtl_fetch(sp, 32),
                                  Rtl_uint(spOffset));
                word = Rtl_fetch(Rtl_location(memType, addr), 32);
                VPOi_rtl(Rtl_assign(temp, 32, word), NULL);
                
                if (isReg) {
                    dest = Rtl_location(memType, getSymAddr(fp, paramSym, 0));
                } else {
                    dest = Rtl_location(memType, getSymAddr(fp, symVarsSym,
                                                            symVarOffset));
                }
                VPOi_rtl(Rtl_assign(dest, 32, Rtl_fetch(temp, 32)), NULL);

                // load higher addressed word of param from stack at higher addr
                addr = Rtl_binary(Rtl_op_add, Rtl_fetch(sp, 32),
                                  Rtl_uint(spOffset + 4));
                word = Rtl_fetch(Rtl_location(memType, addr), 32);
                VPOi_rtl(Rtl_assign(temp, 32, word), NULL);

                if (isReg) {
                    dest = Rtl_location(memType, getSymAddr(fp, paramSym, 4));
                } else {
                    dest = Rtl_location(memType, getSymAddr(fp, symVarsSym,
                                                            (symVarOffset+4)));
                }
                VPOi_rtl(Rtl_assign(dest, 32, Rtl_fetch(temp, 32)),
                         VPOi_locSetBuild(temp, NULL));
            } else {
                addr = Rtl_binary(Rtl_op_add, Rtl_fetch(sp, 32),
                                  Rtl_uint(spOffset));
                word = Rtl_fetch(Rtl_location(memType, addr), 32);
                VPOi_rtl(Rtl_assign(temp, 32, word), NULL);
                
                if (isReg) {
                    dest = Rtl_location(memType, getSymAddr(fp, paramSym, 0));
                } else {
                    dest = Rtl_location(memType, getSymAddr(fp, symVarsSym,
                                                            symVarOffset));
                }
                VPOi_rtl(Rtl_assign(dest, 32, Rtl_fetch(temp, 32)),
                         VPOi_locSetBuild(temp, NULL));
            }
        } else {
            // The actual parameter is split between a register and the stack.
            // Load low addr param word from r[3] into low part of memTempSym.
            assert((nextWord == 3) && (nWords == 2));
            word = Rtl_fetch(r[3], 32);
            dest = Rtl_location('m', getSymAddr(fp, memTempSym, 0));
            VPOi_rtl(Rtl_assign(dest, 32, word), NULL);
            
            // Load high addr param word from stack (m[%sp]) to memTempSym.
            word = Rtl_fetch(Rtl_location('m', Rtl_fetch(sp, 32)), 32);
            Rtl_ty_loc temp = getTempReg(wordTempType);
            VPOi_rtl(Rtl_assign(temp, 32, word), NULL);
            
            dest = Rtl_location('m', getSymAddr(fp, memTempSym, 4));
            VPOi_rtl(Rtl_assign(dest, 32, Rtl_fetch(temp, 32)),
                     VPOi_locSetBuild(temp, NULL));

            // load doubleword into paramSym or symVarsSym[symVarOffset]
            addr = getSymAddr(fp, memTempSym, 0);
            dblword = Rtl_fetch(Rtl_location(memType, addr), 64);
            if (isReg) {
                dest = Rtl_location(memType, getSymAddr(fp, paramSym, 0));
            } else {
                dest = Rtl_location(memType, getSymAddr(fp, symVarsSym,
                                                        symVarOffset));
            }
            VPOi_rtl(Rtl_assign(dest, 64, dblword), NULL);
        }
        nextWord += nWords;
    }            
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::processHRTL
 * OVERVIEW:        If "hrtl" points to a high level HRTL, emits VPO RTLs
 *                  for that HRTL.
 * PARAMETERS:      hrtl: points to a UQBT HRTL.
 *                  bb: points to BasicBlock containing hrtl.
 * RETURNS:         nothing.
 *===========================================================================*/
void ARMVPOBackend::processHRTL(const HRTL* hrtl, PBB bb) {
    if (hrtl->getKind() == LOW_LEVEL_HRTL) {
        return;                 // not a higher-level HRTL
    }

    ostrstream strm;
    hrtl->print(strm);
    char *s = replaceNLs(str(strm));
    if (strlen(s) != 0) {
        VPOi_asm->comment(s);
    }
    
    resetTempRegs();
    switch (hrtl->getKind()) {
    case JUMP_HRTL: {
        const HLJump* jump = static_cast<const HLJump*>(hrtl);
        ADDRESS destAddr = jump->getFixedDest();
        if (destAddr == 0) {
            ostrstream ost;
            ost << "processCT: unhandled computed JUMP_HRTL: dest is ";
            jump->getDest()->print(ost);
            error(str(ost));
        } else {
            int label = bb->getOutEdges()[0]->getLabel();
            if (atLastBB) {
                emitDeferredConstants();
            }
            jumpToBBLabel(label);
        }
        break;
    }
        
    case JCOND_HRTL: {
        processJCondHRTL(hrtl, bb);
        break;
    }
    
    case RET_HRTL: {
        // Check if return has semantics: iterate through any RTs of the return
        int n = hrtl->getNumRT();
        for (int i = 0;  i < n;  i++) {
            RT* rt = hrtl->elementAt(i);
            processRT(rt);
        }

        if (atLastBB) {
            emitDeferredConstants();
        }
        
        VPOi_ty_locSet live_set = NULL;
        const SemStr* retLoc = proc->getReturnLoc(); 
        if (retLoc->len() > 0) {
            // there is something to return: e.g. v22 or r[8]
            Type retType = proc->getReturnType();
            int  retSize = retType.getSize();
            Rtl_ty_loc temp = getTempReg(retType);

            if (retType.getType() == INTEGER) {
                // int actualRetSize = ((toSize < 32)? 32 : toSize);
                VPOi_rtl(Rtl_assign(temp, retSize,
                                    processExpr(retLoc, retType)), NULL);
                VPOi_rtl(MOV(r[0], Rtl_fetch(temp, retSize)),
                         VPOi_locSetBuild(temp, NULL));
                live_set = VPOi_locSetAdd(live_set, r[0]);
            } else if (retType.getType() == FLOATP) {
                VPOi_rtl(MOV(temp, processExpr(retLoc, retType)), NULL);
                if (retSize == 32) {
                    VPOi_rtl(MOV(f[0], Rtl_fetch(temp, 32)),
                             VPOi_locSetBuild(temp, NULL));
                    live_set = VPOi_locSetAdd(live_set, f[0]);
                } else {
                    VPOi_rtl(MOV(d[0], Rtl_fetch(temp, 64)),
                             VPOi_locSetBuild(temp, NULL));
                    live_set = VPOi_locSetAdd(live_set, d[0]);
                }
            } else {
                ostrstream ost;
                ost << "RET_HRTL: unexpected register type in " << retLoc;
                error(str(ost));
            }
        }
        
        // return by setting PC (jumping) to the ARM LR
        Rtl_ty_expr retAddr = Rtl_fetch(VPOi_loc_RT, 32);
        Rtl_ty_rtl ret = jmp(retAddr);
        VPOi_functionLeave(ret, live_set);
        break;
    }
    
    case CALL_HRTL: {
        processCallHRTL(hrtl, bb);
        break;
    }
    
    default: {
        ostrstream ost;
        ost << "processHL: unhandled high-level HRTL: ";
        hrtl->print(ost, 0);
        error(str(ost));
        break;
    }
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::processJCondHRTL
 * OVERVIEW:        Emits VPO RTLs for a conditional jump HRTL.
 * PARAMETERS:      jcondHrtl: points to a UQBT JCOND_HRTL.
 *                  bb: points to BasicBlock containing jcondHrtl.
 * RETURNS:         nothing.
 *===========================================================================*/
void ARMVPOBackend::processJCondHRTL(const HRTL* jcondHrtl, PBB bb) {
    const HLJcond* jcond = static_cast<const HLJcond*>(jcondHrtl);
    SemStr* condExpr = jcond->getCondExpr();
    if (condExpr == NULL) {
        ostrstream ost;
        ost << "processJCondHRTL: JCOND_HRTL at " << hex << hrtlAddr
            << " has no high level condition: ";
        jcond->print(ost);
        error(str(ost));
        return;
    }
    
    ADDRESS destAddr = jcond->getFixedDest();
    if (destAddr == 0) {
        ostrstream ost;
        ost << "processJCondHRTL: JCOND_HRTL at " << hex << hrtlAddr
            << " has 0 destination: ";
        jcond->print(ost);
        error(str(ost));
        return;
    }
    
    // emit "if (condExpr) goto dest"
    Type condType(condExpr->getType());
    switch (jcond->getCond()) {
        case HLJCOND_JUL:  case HLJCOND_JULE:
        case HLJCOND_JUGE: case HLJCOND_JUG:
            condType.setSigned(false);
            break;
        default:
            break;
    }
    
    // Sigh: ARM VPO requires that we emit "CC = lhs <rel> rhs" (since it
    // can be directly implemented by the CMP instruction). We cannot use
    // processExpr() directly since we cannot emit "CC = temp".
    // We must parse the condExpr ourselves and emit the necessary code.
    // Note that condExprs have a simple form: <var> <rel> <var or const>.
    INDEX idx = (INDEX)(condExpr->getFirstIdx());
    const SemStr* lhs = condExpr->getSubExpr(0);
    const SemStr* rhs = condExpr->getSubExpr(1);
    int condSize = condType.getSize(); // comparison size in bits
    
    // Set the integer or FP status (condition code) register.
    Rtl_ty_loc tempL = getTempReg(condType);
    VPOi_rtl(Rtl_assign(tempL, condSize, processExpr(lhs, condType)), NULL);
    Rtl_ty_loc tempR = getTempReg(condType);
    VPOi_rtl(Rtl_assign(tempR, condSize, processExpr(rhs, condType)), NULL);
    
    Rtl_ty_rtl compare;
    if (jcond->isFloat()) {
        compare = Rtl_assign(VPOi_loc_FPSR, 32,
                             Rtl_binary(Rtl_op_cmpF,
                                        Rtl_fetch(tempL, condSize),
                                        Rtl_fetch(tempR, condSize)));
    } else {
        compare = Rtl_assign(VPOi_loc_CPSR, 32,
                             Rtl_binary(Rtl_op_cmp,
                                        Rtl_fetch(tempL, condSize),
                                        Rtl_fetch(tempR, condSize)));
    }
    VPOi_rtl(compare, VPOi_locSetBuild(tempL, tempR, NULL));
    
    // Now emit the appropriate conditional branch.
    Rtl_ty_oper op = xlateBinaryOp(idx);
    Rtl_ty_expr guard;
    if (jcond->isFloat()) {
        guard = Rtl_binary(op, Rtl_fetch(VPOi_loc_FPSR, 32), Rtl_int(0));
    } else {
        guard = Rtl_binary(op, Rtl_fetch(VPOi_loc_CPSR, 32), Rtl_int(0));
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
        if (atLastBB) {
            emitDeferredConstants();
        }
        jumpToBBLabel(label);
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::processCallHRTL
 * OVERVIEW:        Emits VPO RTLs for a call HRTL.
 * PARAMETERS:      callHrtl: points to a UQBT CALL_HRTL.
 *                  bb: points to BasicBlock containing callHrtl.
 * RETURNS:         nothing.
 *===========================================================================*/
void ARMVPOBackend::processCallHRTL(const HRTL* callHrtl, PBB bb) {
    // If the call has semantics, iterate through those RTs.
    int n = callHrtl->getNumRT();
    for (int i = 0;  i < n;  i++) {
        RT* rt = callHrtl->elementAt(i);
        processRT(rt);
    }
    
    const HLCall* call = static_cast<const HLCall*>(callHrtl);
    ADDRESS targetAddr = call->getFixedDest();
    const char* calleeName = binFile->SymbolByAddress(targetAddr);

    if (calleeName == NULL) {
        if (targetAddr != NO_ADDRESS) {
            return;             // error msg already given by declareCalledProc
        }
    }
    
    // Pass parameters and define param_set
    VPOi_ty_locSet param_set = NULL;
    passParams(call, param_set);
    
    // Define set of regs returned by procedure.
    Rtl_ty_loc retReg = NULL;
    VPOi_ty_locSet defined_set = NULL;
    const SemStr retLoc = call->getReturnLoc();
    if (retLoc.len() > 0) {
        // there is something to return: e.g. v22 or r[8]
        Type retType = retLoc.getType();
        if (retType.getType() == INTEGER) {
            assert(retType.getSize() == 32);
            retReg = r[0];
        } else if (retType.getType() == FLOATP) {
            retReg = ((retType.getSize() == 32)? f[0] : d[0]);
        } else {
            ostrstream ost;
            ost << "processCallHRTL: unexpected return location in ";
            call->print(ost, 0);
            error(str(ost));
        }
        defined_set = VPOi_locSetBuild(retReg, NULL);
    }
    
    // Emit the call itself
    Rtl_ty_rtl callRtl;
    if (calleeName != NULL) {
        string calleeNameStr = calleeName;
        assert(procMap.find(calleeNameStr) != procMap.end());
        AsmSymbol calleeSym = procMap[calleeNameStr];
        callRtl = jmp(Rtl_relAddr(calleeSym->relAddr));
    } else {                    // register call
        SemStr* callDest = call->getDest();
        Type addressType = Type(INTEGER, 32, false);
        Rtl_ty_loc temp = getTempReg(addressType);
        VPOi_rtl(Rtl_assign(temp, 32, processExpr(callDest, addressType)),
                 NULL);
        callRtl = jmp(Rtl_fetch(temp, 32));
    }
    VPOi_functionCall(callRtl, dead_set, param_set, defined_set,
                      callerSave_set, killed_set, used_set);
    
    // Store the result of the call, if any
    if (retLoc.len() != 0) {
        Type retType = retLoc.getType();
        int  retSize = retType.getSize();
        
        // process the destination: the return location, e.g. v22 or r[8]
        Rtl_ty_expr destAddr;
        switch(retLoc.getFirstIdx()) {
        case idRegOf:
            destAddr = processRegOf(&retLoc);
            break;
            
        case idMemOf:
            destAddr = processMemOf(&retLoc);
            break;
            
        case idVar:
            destAddr = processVar(&retLoc);
            break;
            
        default:
            if (retLoc.getFirstIdx() >= idMachSpec) {
                // This is a machine-dependant register, such as %pc or %Y.
                const string& regName =
                    theSemTable[retLoc.getFirstIdx()].sName;
                // Load location of reg's 32 bit int var into lhsLoc.
                assert(machDepMap.find(regName) != machDepMap.end());
                AsmSymbol sym = machDepMap[regName];
                destAddr = getSymAddr(fp, sym, 0);
                break;
            } else {
                ostrstream ost;
                ost << "processCallHRTL: unrecognized return location in ";
                call->print(ost);
                error(str(ost));
                destAddr = getSymAddr(fp, memTempSym, 0); // must use something
            }
        }
        
        // the source is an ARM register, typically r0 or f0
        // combine into an assignment
        Rtl_ty_loc temp = getTempReg(retType);
        VPOi_rtl(Rtl_assign(temp, retSize,
                            Rtl_fetch(retReg, retSize)), NULL);

        // If the LHS is memory, swap the RHS bytes if necessary
        // (and unless progOptions.noendian)
        bool isMemWrite = (retLoc.getFirstIdx() == idMemOf);
        if (isMemWrite && doingMemorySwaps) {
            emitByteSwaps(temp, (retSize / 8), retType.getType());
        }

        Rtl_ty_loc dest = Rtl_location(memoryTypeForType(retType), destAddr);
        VPOi_rtl(Rtl_assign(dest, retSize, Rtl_fetch(temp, retSize)),
                 VPOi_locSetBuild(temp, NULL));
    }
    
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
        if (atLastBB) {
            emitDeferredConstants();
        }
        jumpToBBLabel(label);
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::processRT
 * OVERVIEW:        Emits VPO RTLs for a UQBT RT.
 * PARAMETERS:      rt: points to the UQBT RT
 * RETURNS:         <nothing>
 *===========================================================================*/
void ARMVPOBackend::processRT(RT* rt) {
    ostrstream ost;
    Rtl_ty_loc dest;
    
    rt->print(ost);
    char *s = replaceNLs(str(ost));
    if (strlen(s) != 0) {
        VPOi_asm->comment(s);
    }
    
    resetTempRegs();
    
    switch(rt->getKind()) {
    case RTASSGN: {
        RTAssgn* assign = (RTAssgn*)rt;
        Type cType(assign->getType());    // initial type, from the assignment
        int neededSize = cType.getSize(); // NB: OVERRIDES LHS SIZE!
        const SemStr* lhs = assign->getLHS();
        const SemStr* rhs = assign->getRHS();
        int lhsIdx = lhs->getFirstIdx();
        int rhsIdx = rhs->getFirstIdx();
        Rtl_ty_expr lhsAddr;
        
        // First process the left hand side
        switch(lhsIdx) {
            case idRegOf:
                lhsAddr = processRegOf(lhs);
                break;
                
            case idMemOf:
                lhsAddr = processMemOf(lhs);
                break;
                
            case idVar: 
                lhsAddr = processVar(lhs);
                break;
                
            default:
                if (lhsIdx >= idMachSpec) {
                    // This is a machine-dependant register, such as %pc or %Y.
                    const string& regName = theSemTable[lhsIdx].sName;
                    // Load location of reg's 32 bit int var into lhsLoc.
                    assert(machDepMap.find(regName) != machDepMap.end());
                    AsmSymbol sym = machDepMap[regName];
                    lhsAddr = getSymAddr(fp, sym, 0);
                    break;
                } else {
                    ostrstream ost;
                    ost << "processRT: unrecognized LHS in assignment: ";
                    assign->print(ost);
                    error(str(ost));
                    return;
                }
        }

        // First handle some special cases
        if ((rhsIdx == idIntConst) || (rhsIdx == idFltConst)) {
            // avoid use of another temp
            dest = Rtl_location(memoryTypeForType(cType), lhsAddr);
            VPOi_rtl(Rtl_assign(dest, neededSize, processExpr(rhs, cType)),
                     NULL);
            break;
        }
        
        if ((neededSize == 64) && (cType.getType() == INTEGER)) {
            // check if both lhs and rhs are vars or (non-temp) registers.
            bool lhsIsVar = (lhsIdx == idVar);
            bool lhsIsReg = (lhsIdx == idRegOf) &&
                (lhs->getSecondIdx() == idIntConst);
            bool rhsIsVar = (rhsIdx == idVar);
            bool rhsIsReg = (rhsIdx == idRegOf) &&
                (rhs->getSecondIdx() == idIntConst);
            if ((lhsIsVar || lhsIsReg) && (rhsIsVar || rhsIsReg)) {
                // Special case of 64 bit int {reg,var} := {reg,var}
                // assignment where the register is a (non-temporary) reg.
                // The ARM has no 64 bit int registers but we can implement
                // these assignments using two single-word assigns.
                Rtl_ty_expr rhsAddr;
                Rtl_ty_expr word;
                Rtl_ty_expr lhsAddrPlus4, rhsAddrPlus4;
                if (rhsIsReg) {
                    rhsAddr = processRegOf(rhs);
                } else {
                    rhsAddr = processVar(rhs);
                }
                
                word = Rtl_fetch(Rtl_location('m', rhsAddr), 32);
                VPOi_rtl(Rtl_assign(Rtl_location('m', lhsAddr), 32, word),
                         NULL);
                lhsAddrPlus4 = Rtl_binary(Rtl_op_add, lhsAddr, Rtl_uint(4));
                rhsAddrPlus4 = Rtl_binary(Rtl_op_add, rhsAddr, Rtl_uint(4));
                word = Rtl_fetch(Rtl_location('m', rhsAddrPlus4), 32);
                VPOi_rtl(Rtl_assign(Rtl_location('m', lhsAddrPlus4), 32, word),
                         NULL);
                break;
            } // else do general case
        }

        // General case
        Rtl_ty_loc temp = getTempReg(cType);
        VPOi_rtl(Rtl_assign(temp, neededSize, processExpr(rhs, cType)), NULL);
        
        // If the LHS is memory, swap the RHS bytes if necessary
        // (and unless progOptions.noendian)
        bool isMemWrite = (lhsIdx == idMemOf);
        if (isMemWrite && doingMemorySwaps) {
            emitByteSwaps(temp, (neededSize / 8), cType.getType());
        }
        
        dest = Rtl_location(memoryTypeForType(cType), lhsAddr);
        VPOi_rtl(Rtl_assign(dest, neededSize, Rtl_fetch(temp, neededSize)),
                 VPOi_locSetBuild(temp, NULL));
        break;
    }
    
    case RTFLAGCALL: {
        // Just ignore these: after analysis, JCOND_HRTL contains test
        break;
    }
    
    case RTFLAGDEF:
        // we should never see this
        error("processRT: RT flag def: should not be seen!");
        break;
        
    default:
        error("processRT: unexpected RT_KIND for RT");
        break;
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::processExpr
 * OVERVIEW:        Emits VPO RTLs for a UQBT SemStr describing an expression.
 * PARAMETERS:      exp: points to a UQBT SemStr describing an expression
 *                  cType: the "current" expected type: if operands are not
 *                    this type, they must be cast.
 * RETURNS:         A VPO Rtl_ty_expr for the expression.
 * NOTE:            The result must be a simple expression that is a
 *                  "Rtl_fetched" VPO register.
 *===========================================================================*/
Rtl_ty_expr ARMVPOBackend::processExpr(const SemStr* exp, Type cType) {
    INDEX idx = (INDEX)(exp->getFirstIdx());
    int numArgs = theSemTable[idx].iNumVarArgs;
    int neededSize = cType.getSize(); // desired size in bits

    // Handle machine-dependant registers such as %Y or %CF specially
    if ((numArgs == 0) && (idx >= idMachSpec)) {
        return processMachDepReg(exp);
    }

    // Use Manel's switch/case that are based on two fields: a SemStr index
    // and a number of arguments (0..3). These replace multiple nested
    // switch statements, which are nearly unreadable. The SemStr index is
    // left shifted 2 bits then logical-or'd with the number of arguments
    // to create the switch/case key.
#define SWITCH(nargs,id) switch (((id)<<2) | ((nargs)&0x3))
#define CASE(nargs,id)   case   (((id)<<2) | ((nargs)&0x3))

    SWITCH(numArgs, idx)
    {
        // Nilary operators
        CASE(0,idIntConst): {   // integer constant
            assert(neededSize <= 32); // only 32 bit ints for now!
            Rtl_ty_loc constTemp = loadIntConst(exp->getSecondIdx(), cType);
            return Rtl_fetch(constTemp, neededSize);
        }

        CASE(0,idFltConst): {   // FP constant
            union _u {
                struct {
                    int i1; int i2;
                } ii;
                double d;
            } u;
            assert(neededSize == 64);
            u.ii.i1 = exp->getSecondIdx();
            u.ii.i2 = exp->getThirdIdx();
            Rtl_ty_loc constTemp = loadFPConst(u.d);
            return Rtl_fetch(constTemp, neededSize);
        }

        CASE(0,idVar): {        // variable reference of form v[ NN
            // NB: cType OVERRIDES actual type of the variable!
            //int NN = exp->getSecondIdx();
            // Look up the type of the variable in the source machine
            //Type varType = proc->getVarType(NN);
            //Rtl_ty_loc varLoc = processVar(exp, varType);
            //Rtl_ty_loc valueLoc = emitLoad(varLoc, varType,
            //                               /*emitSwaps*/ false);
            // Do any required casts: result is (possibly new) loc with result
            //Rtl_ty_loc loc = emitCastIfNeeded(valueLoc, varType, cType);
            Rtl_ty_expr addr = processVar(exp);
            Rtl_ty_loc valueLoc = emitLoad(addr, cType, /*emitSwaps*/ false);
            return Rtl_fetch(valueLoc, neededSize);
        }

        CASE(0,idAFP): {        // reference to %afp
            Rtl_ty_expr addr = getSymAddr(fp, localsSym, 0);
            // Address expressions are unsigned 32 bit.
            //Type addressType = Type(INTEGER, 32, false);
            //emitCastIfNeeded(cType, addressType);
            return addr;
        }

        // Unary operators
        CASE(1,idRegOf): {      // register reference
            Rtl_ty_expr addr = processRegOf(exp);
            Rtl_ty_loc regLoc = Rtl_location(memoryTypeForType(cType), addr);
            return Rtl_fetch(regLoc, neededSize);
        }
                
        CASE(1,idMemOf): {      // memory reference
            Rtl_ty_expr addr = processMemOf(exp);
            Rtl_ty_loc valueLoc = emitLoad(addr, cType, /*emitSwaps*/ true);
            return Rtl_fetch(valueLoc, neededSize);
        }
            
        CASE(1,idAddrOf): {     // address of variable
            const SemStr* varRef = exp->getSubExpr(0);
            if (varRef->getIndex(0) != idVar) {
                ostrstream ost;
                ost << "processExpr: can only get address of symbolic vars: "
                    << *exp << " at " << hex << hrtlAddr;
                error(str(ost));
                return Rtl_int(0); // must return something
            }
            // the varRef SemStr is of form v[ NN
            int NN = varRef->getSecondIdx();
            assert(symVarOffsetMap.find(NN) != symVarOffsetMap.end());
            int offset = symVarOffsetMap[NN];
            Rtl_ty_expr addr = getSymAddr(fp, symVarsSym, offset);
            return addr;
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
        CASE(2,idPlus): {       // Addition
            if ((exp->getIndex(1) == idAFP) &&
                (exp->getIndex(2) == idIntConst)) {
                // Return the address of a local variable of the form %afp+NN
                //     +  %afp  int  NN
                //     0    1    2    3
                // Emit code for &(locals[NN]) = &locals + NN
                int NN = exp->getIndex(3);
                assert(NN >= 0);
                Rtl_ty_expr addr = getSymAddr(fp, localsSym, NN);
                Rtl_ty_loc temp = getTempReg(cType);
                VPOi_rtl(Rtl_assign(temp, 32, addr), NULL);
                return Rtl_fetch(temp, neededSize);
            } else if ((exp->getIndex(1) == idCastIntStar) &&
                       (exp->getIndex(2) == idAddrOf) &&
                       (exp->getIndex(3) == idVar) &&
                       (exp->getIndex(5) == idIntConst)) {
                // Return the address of a symbolic var plus an offset
                //     +  (int*)  a[   v[   NN    int  n
                //     0    1      2    3    4    5    6
                int offset = exp->getIndex(6);
                int NN = exp->getIndex(4);
                assert(NN >= 0);
                assert(symVarOffsetMap.find(NN) != symVarOffsetMap.end());
                int varOffset = symVarOffsetMap[NN];
                Rtl_ty_expr addr = getSymAddr(fp, symVarsSym,
                                              (varOffset + 4*offset));
                Rtl_ty_loc temp = getTempReg(cType);
                VPOi_rtl(Rtl_assign(temp, 32, addr), NULL);
                return Rtl_fetch(temp, neededSize);
            } else {
                return processBinaryExpr(exp, cType);
            }
        }
         
        CASE(2,idMinus):        // Subtraction
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
                << " at " << hex << hrtlAddr;
            error(str(ost));
    }
    return Rtl_int(0);          // must return something
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::processMachDepReg
 * OVERVIEW:        Emits VPO RTLs for a SemStr describing a machine-dependent
 *                  register expression.
 * PARAMETERS:      exp: points to a UQBT SemStr describing a machine-dependent
 *                    register expression
 * RETURNS:         A VPO Rtl_ty_expr for the machine-dependent register expr.
 *===========================================================================*/
Rtl_ty_expr ARMVPOBackend::processMachDepReg(const SemStr* exp) {
    INDEX idx = (INDEX)(exp->getFirstIdx());
    const string& fullName = theSemTable[idx].sName;
    
    // Return value of int variable for the register.
    assert(machDepMap.find(fullName) != machDepMap.end());
    AsmSymbol sym = machDepMap[fullName];
    Rtl_ty_expr addr = getSymAddr(fp, sym, 0);
    return Rtl_fetch(Rtl_location('m', addr), 32);
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::processUnaryExpr
 * OVERVIEW:        Emits VPO RTLs for a SemStr describing a unary expression.
 * PARAMETERS:      op: VPO unary operator
 *                  exp: points to a UQBT SemStr describing a unary expression
 *                  cType: the "current" expected type: if operands are not
 *                    this type, they must be cast.
 * RETURNS:         A VPO Rtl_ty_expr for the unary expression.
 *===========================================================================*/
Rtl_ty_expr ARMVPOBackend::processUnaryExpr(Rtl_ty_oper op, const SemStr* exp,
                                            Type cType) {
    const SemStr* subExpr = exp->getSubExpr(0);
    int neededSize = cType.getSize(); // desired size in bits
    // get temp reg to hold results of subexpression and unary expression
    Rtl_ty_loc resTemp = getTempReg(cType);
    Rtl_ty_expr subExprVal = processExpr(subExpr, cType);
    
    VPOi_rtl(Rtl_assign(resTemp, neededSize, subExprVal), NULL);
    VPOi_rtl(Rtl_assign(resTemp, neededSize,
                        Rtl_unary(op, Rtl_fetch(resTemp, neededSize))), NULL);
    return Rtl_fetch(resTemp, neededSize);
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::processBinaryExpr
 * OVERVIEW:        Emits VPO RTLs for a SemStr describing a binary expression.
 * PARAMETERS:      exp: points to a UQBT SemStr describing a binary expression
 *                  cType: the "current" type: if operands are not this
 *                    type, they must be cast.
 * RETURNS:         A VPO Rtl_ty_expr for the binary expression.
 *===========================================================================*/
Rtl_ty_expr ARMVPOBackend::processBinaryExpr(const SemStr* exp, Type cType) {
    INDEX idx = (INDEX)(exp->getFirstIdx());
    const SemStr* lhs = exp->getSubExpr(0);
    const SemStr* rhs = exp->getSubExpr(1);
    
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
    int neededSize = cType.getSize(); // desired size in bits
    
    // handle some binary operators specially
    switch(idx)
    {
        case idDiv:             // unsigned integer divide
        case idDivs:            // signed integer divide
        case idMod:             // unsigned mod
        case idMods: {          // signed mod: round to zero
            return processLibRoutineCall(exp, cType);
        }

        case idNotEqual: {      // !=
            // ARM VPO does not support "not equal" as a binary operator, so we
            // must simulate it using a conditional assignment (sigh).
            Rtl_ty_loc tempL = getTempReg(cType);
            VPOi_rtl(Rtl_assign(tempL, neededSize, processExpr(lhs, cType)),
                     NULL);
            Rtl_ty_loc tempR = getTempReg(cType);
            VPOi_rtl(Rtl_assign(tempR, neededSize, processExpr(rhs, cType)),
                     NULL);
            
            Rtl_ty_rtl compare;
            if (cType.getType() == FLOATP) {
                compare = Rtl_assign(VPOi_loc_FPSR, 32,
                                     Rtl_binary(Rtl_op_cmpF,
                                                Rtl_fetch(tempL, neededSize),
                                                Rtl_fetch(tempR, neededSize)));
            } else {
                compare = Rtl_assign(VPOi_loc_CPSR, 32,
                                     Rtl_binary(Rtl_op_cmp,
                                                Rtl_fetch(tempL, neededSize),
                                                Rtl_fetch(tempR, neededSize)));
            }
            VPOi_rtl(compare, VPOi_locSetBuild(tempL, tempR, NULL));

            // Now emit a conditional branch to code that sets "temp".
            Rtl_ty_loc temp = getTempReg(cType);
            AsmSymbol trueLabelSym  = genLabel();
            AsmSymbol afterLabelSym = genLabel(); // label of code after assigns
            
            Rtl_ty_oper op = xlateBinaryOp(idx);  // if must handle more than ~=
            Rtl_ty_expr guard;
            if (cType.getType() == FLOATP) {
                guard = Rtl_binary(op, Rtl_fetch(VPOi_loc_FPSR, 32),
                                   Rtl_int(0));
            } else {
                guard = Rtl_binary(op, Rtl_fetch(VPOi_loc_CPSR, 32),
                                   Rtl_int(0));
            }
            Rtl_ty_rtl jump = Rtl_assign(VPOi_loc_PC, 32,
                                         Rtl_relAddr(trueLabelSym->relAddr));
            VPOi_rtl(Rtl_guard(guard, jump), NULL);

            // false case: set temp 0
            VPOi_rtl(Rtl_assign(temp, neededSize, Rtl_int(0)), NULL);
            VPOi_rtl(B(afterLabelSym->relAddr), NULL);

            // true case: set temp 1
            VPOi_asm->define_symbol_here(trueLabelSym);
            VPOi_rtl(Rtl_assign(temp, neededSize, Rtl_int(1)), NULL);

            // common code after the "then" or "else"
            VPOi_asm->define_symbol_here(afterLabelSym);
            return Rtl_fetch(temp, neededSize);
        }

        default: {              // normal case: no special treatment needed
            Rtl_ty_loc tempL = getTempReg(cType);
            VPOi_rtl(Rtl_assign(tempL, neededSize,
                                processExpr(lhs, cType)), NULL);
            Rtl_ty_expr lhsVal = Rtl_fetch(tempL, neededSize);
            
            INDEX rhsIdx = (INDEX)(rhs->getFirstIdx());
            Rtl_ty_loc tempR;
            if (rhsIdx == idIntConst) {
                // optimize common case on RHS where no extra temp reg is needed
                tempR = loadIntConst(rhs->getSecondIdx(), cType);
            } else {
                tempR = getTempReg(cType);
                VPOi_rtl(Rtl_assign(tempR, neededSize,
                                    processExpr(rhs, cType)), NULL);
            }
            Rtl_ty_expr rhsVal = Rtl_fetch(tempR, neededSize);
            
            Rtl_ty_oper op = xlateBinaryOp(idx);
            VPOi_rtl(Rtl_assign(tempL, neededSize,
                                Rtl_binary(op, lhsVal, rhsVal)),
                     VPOi_locSetBuild(tempR, NULL));
            return Rtl_fetch(tempL, neededSize);
        }
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::processTernaryExpr
 * OVERVIEW:        Emits VPO RTLs for a SemStr describing a ternary expression
 * PARAMETERS:      exp: points to a UQBT SemStr describing a ternary expr.
 *                  cType: the "current" type: if operands are not this
 *                    type, they must be cast.
 * RETURNS:         A VPO Rtl_ty_expr for the ternary expression.
 *===========================================================================*/
Rtl_ty_expr ARMVPOBackend::processTernaryExpr(const SemStr* exp,
                                                      Type cType) {
    INDEX idx = (INDEX)(exp->getFirstIdx());
    int neededSize = cType.getSize(); // desired size in bits
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

	    if ((neededSize > 32) || (arg2->getType().getSize() > 32) ||
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
                VPOi_rtl(Rtl_assign(tempA, 32, processExpr(arg1, uType)), NULL);
                Rtl_ty_loc tempB = getTempReg(cType);
                VPOi_rtl(Rtl_assign(tempB, 32, processExpr(arg2, cType)), NULL);
                VPOi_rtl(Rtl_assign(tempA, 32,
                                    Rtl_binary(Rtl_op_rshift,
                                               Rtl_fetch(tempA, 32),
                                               Rtl_fetch(tempB, 32))),
                         VPOi_locSetBuild(tempB, NULL));
                VPOi_rtl(Rtl_assign(tempA, neededSize,
                                    Rtl_binary(Rtl_op_and,
                                               Rtl_fetch(tempA, 32),
                                               Rtl_uint(1))), NULL);
            } else {
                // Use (((unsigned)a >> b) & ((1<<(c-b+1))-1)). Assumes b < c!
                Type uType(cType);
                uType.setSigned(false);
                VPOi_rtl(Rtl_assign(tempA, 32, processExpr(arg1, uType)), NULL);
                Rtl_ty_loc tempB = getTempReg(cType);
                VPOi_rtl(Rtl_assign(tempB, 32, processExpr(arg2, cType)), NULL);
                VPOi_rtl(Rtl_assign(tempA, 32,
                                    Rtl_binary(Rtl_op_rshift,
                                               Rtl_fetch(tempA, 32),
                                               Rtl_fetch(tempB, 32))), NULL);
		// tempA now holds ((unsigned)a >> b)
		
		Rtl_ty_loc tempC = getTempReg(cType);
                VPOi_rtl(Rtl_assign(tempC, 32, processExpr(arg3, cType)), NULL);
                VPOi_rtl(Rtl_assign(tempB, 32,
                                    Rtl_binary(Rtl_op_sub,
                                               Rtl_fetch(tempC, 32),
                                               Rtl_fetch(tempB, 32))), NULL);
                VPOi_rtl(Rtl_assign(tempB, 32,
                                    Rtl_binary(Rtl_op_add,
                                               Rtl_fetch(tempB, 32),
                                               Rtl_int(1))), NULL);
		// tempB now holds (c-b+1)

                VPOi_rtl(Rtl_assign(tempB, 32,
                                    Rtl_binary(Rtl_op_lshift,
                                               Rtl_int(1), 
                                               Rtl_fetch(tempB, 32))), NULL);
                VPOi_rtl(Rtl_assign(tempB, 32,
                                    Rtl_binary(Rtl_op_sub,
                                               Rtl_fetch(tempB, 32),
                                               Rtl_int(1))), NULL);
		// tempB now has ((1<<(c-b+1))-1))

                VPOi_rtl(Rtl_assign(tempA, 32,
                                    Rtl_binary(Rtl_op_and,
                                               Rtl_fetch(tempA, 32), 
                                               Rtl_fetch(tempB, 32))),
                         VPOi_locSetBuild(tempB, NULL));
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
    return Rtl_fetch(tempA, neededSize);
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::xlateBinaryOp
 * OVERVIEW:        Returns the VPO operator for a binary SemStr operator.
 * PARAMETERS:      idx: index of the UQBT binary operator.
 * RETURNS:         a binary VPO operator for the SemStr operator.
 *===========================================================================*/
Rtl_ty_oper ARMVPOBackend::xlateBinaryOp(INDEX idx) {
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
        case idLessEq:   return  Rtl_op_le;
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
 * FUNCTION:        ARMVPOBackend::processRegOf
 * OVERVIEW:        Return the address for a register described by a UQBT SemStr
 * PARAMETERS:      regRef: points to the register reference's SemStr.
 * RETURNS:         VPO expression for the address of the register
 *===========================================================================*/
Rtl_ty_expr ARMVPOBackend::processRegOf(const SemStr* regRef) {
    Rtl_ty_expr regAddr;
    int num;
    int idx = regRef->getSecondIdx();
    if (idx == idTemp) {
        // A reference to a UQBT temporary register: e.g., r[tmp1]
        num = regRef->getThirdIdx();
        const string& tempName = theSemTable[num].sName;
        
        // Get address of temp reg's var
        if (tempRegVarMap.find(tempName) == tempRegVarMap.end()) {
            ostrstream ost;
            ost << "processRegOf: ref to temp reg " << tempName
                << " that wasn't allocated" << ", in: ";
            regRef->print(ost);
            error(str(ost));
            return getSymAddr(fp, memTempSym, 0); // must return something;
        }
        AsmSymbol sym = tempRegVarMap[tempName];
        regAddr = getSymAddr(fp, sym, 0);
    } else {
        // A reference to a UQBT register: e.g., r[4]
        if (idx == idIntConst) {
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

        // Get address of reg's var
        if (regVarMap.find(num) == regVarMap.end()) {
            ostrstream ost;
            ost << "processRegOf: ref to reg " << num
                << " that wasn't allocated" << ", in: ";
            regRef->print(ost);
            error(str(ost));
            return getSymAddr(fp, memTempSym, 0); // must return something;
        }
        AsmSymbol sym = regVarMap[num];
        regAddr = getSymAddr(fp, sym, 0);
    }
    return regAddr;
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::processMemOf
 * OVERVIEW:        Return the address for a memory address described by a
 *                  UQBT SemStr.
 * PARAMETERS:      memRef: points to the memory address's SemStr.
 * RETURNS:         VPO expression for the address of the addressed memory
 *===========================================================================*/
Rtl_ty_expr ARMVPOBackend::processMemOf(const SemStr* memRef) {
    const SemStr* addrExpr = memRef->getSubExpr(0);
    Rtl_ty_expr addr;
    
    if ((addrExpr->getIndex(0) == idPlus) &&
        (addrExpr->getIndex(1) == idAFP) &&
        (addrExpr->getIndex(2) == idIntConst)) {
        // A reference to a local variable of the form m[%afp+NN]
        //         +  %afp  int  NN
        //         0    1    2    3
        // Emit code for locals[NN] = m[locals + NN]
        int NN = addrExpr->getIndex(3);
        assert(NN >= 0);
        addr = getSymAddr(fp, localsSym, NN);
    } else {
        // Normal memory reference. Address expressions are unsigned 32 bit.
        Type addressType = Type(INTEGER, 32, false);
        Rtl_ty_loc temp = getTempReg(addressType);
        addr = processExpr(addrExpr, addressType);
        VPOi_rtl(Rtl_assign(temp, 32, addr), NULL);
        addr = Rtl_fetch(temp, 32);
    }
    return addr;
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::processVar
 * OVERVIEW:        Return the address for a var described by a UQBT SemStr.
 * PARAMETERS:      varRef: points to the SemStr for a variable.
 * RETURNS:         VPO expression for the address of the variable.
 *===========================================================================*/
Rtl_ty_expr ARMVPOBackend::processVar(const SemStr* varRef) {
    // the varRef SemStr is of form v[ NN
    int NN = varRef->getSecondIdx();
    assert(symVarOffsetMap.find(NN) != symVarOffsetMap.end());
    int offset = symVarOffsetMap[NN];
    
    // We must use a temp or otherwise VPO finds the addr expr too complex to
    // generate assembler code for the ARM.
    Type addressType = Type(INTEGER, 32, false);
    Rtl_ty_loc addr = getTempReg(addressType);
    VPOi_rtl(Rtl_assign(addr, 32, getSymAddr(fp, symVarsSym, offset)), NULL);

    return Rtl_fetch(addr, 32);
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::processLibRoutineCall
 * OVERVIEW:        Emits VPO RTLs for a binary expression SemStr that is
 *                  implemented by a call to a library routine.
 * PARAMETERS:      exp: points to a UQBT SemStr describing a binary expression
 *                  cType: the expected type of the result.
 * RETURNS:         A VPO Rtl_ty_expr for the binary expression.
 *===========================================================================*/
Rtl_ty_expr ARMVPOBackend::processLibRoutineCall(const SemStr* exp,
                                                 Type cType) {
    // exp is known to be a binary expression SemStr that is implemented by a
    // call to a library routine that takes r0 and r1 as input and produces
    // a result in r0. 
    INDEX idx = (INDEX)(exp->getFirstIdx());
    const SemStr* lhs = exp->getSubExpr(0);
    const SemStr* rhs = exp->getSubExpr(1);

    int neededSize = cType.getSize(); // desired size in bits
    if ((neededSize != 32) || (cType.getType() != INTEGER)) {
        ostrstream ost;
        ost << "processLibRoutineCall: ";
        if ((idx >= 0) && (idx <= idNumOf)) {
            ost << " :" << theSemTable[idx].sName;
        } else {
            ost << "unknown binary operator " << idx;
        }
        ost << " only supported for 32 bit integers: " << *exp
            << " at " << hex << hrtlAddr;
        error(str(ost));
    }
    
    AsmSymbol calleeSym;
    switch(idx) {
        case idDiv:             // unsigned integer divide
            calleeSym = udivSym;
            break;
        case idDivs:            // signed integer divide
            calleeSym = divSym;
            break;
        case idMod:             // unsigned mod
            calleeSym = umodSym;
            break;
        case idMods:            // signed mod: round to zero
            calleeSym = modSym;
            break;
        default:
            ostrstream ost;
            ost << "processLibRoutineCall: unexpected binary operator in "
                << *exp;
            error(str(ost));
    }

    // Tell VPO the size of the argument build area and the number of arguments
    VPOi_parameterListSize(/*totalBytes*/ 8, /*numParams*/ 2);

    // Copy the arguments to r0 and r1.
    VPOi_rtl(Rtl_assign(r[0], 32, processExpr(lhs, cType)), NULL);
    VPOi_rtl(Rtl_assign(r[1], 32, processExpr(rhs, cType)), NULL);
    
    // Define the set of registers holding parameters
    VPOi_ty_locSet param_set = VPOi_locSetBuild(r[0], r[1], NULL);

    // Define the registers returned by the library routine
    VPOi_ty_locSet defined_set = VPOi_locSetBuild(r[0], NULL);

    // Emit the call itself
    Rtl_ty_rtl callRtl = jmp(Rtl_relAddr(calleeSym->relAddr));
    VPOi_functionCall(callRtl, dead_set, param_set, defined_set,
                      callerSave_set, killed_set, used_set);
    return Rtl_fetch(r[0], 32);
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::processConversion
 * OVERVIEW:        Emits VPO RTLs for a UQBT SemStr describing a conversion
 *                  expression.
 * PARAMETERS:      exp: points to a UQBT SemStr describing an expression
 *                  cType: the "current" type: if operands are not this
 *                    type, they must be cast.
 * RETURNS:         A VPO Rtl_ty_expr for the expression.
 *===========================================================================*/
Rtl_ty_expr ARMVPOBackend::processConversion(const SemStr* exp, Type cType) {
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

    // ARM VPO is 32 bit except for a few instructions
    if ((fromSize > 64) || (toSize > 64)) {
        ostrstream ost;
        ost << "processConversion: unsupported size in: " << *exp;
        error(str(ost));
        return Rtl_int(0);      // must return something
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
                // int actualToSize = ((toSize < 32)? 32 : toSize);
                VPOi_rtl(Rtl_assign(temp, toSize,
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

            temp = getTempReg(toType);
            if ((fromSize == 32) && (toSize == 64)) {
                // Float to double
                VPOi_rtl(MVFS(temp, temp1), VPOi_locSetBuild(temp1, NULL));
            } else  if ((fromSize == 64) && (toSize == 32)) {
                // Double to float
                VPOi_rtl(MVFD(temp, temp1), VPOi_locSetBuild(temp1, NULL));
            } else {
                ostrstream ost;
                ost << "processConversion: unimplemented FP->FP conversion in "
                    << *exp;
                error(str(ost));
                return Rtl_int(0); // must return something
            }
            break;
        }
            
        case idItof: {          // Int to floating point (and size) conversion
            Rtl_ty_loc temp1 = getTempReg(fromType);
            VPOi_rtl(Rtl_assign(temp1, fromSize,
                                processExpr(subExpr, fromType)), NULL);

            temp = getTempReg(toType);
            if (toSize == 32) {
                VPOi_rtl(FLTS(temp, temp1), VPOi_locSetBuild(temp1, NULL));
            } else if (toSize == 64) {
                VPOi_rtl(FLTD(temp, temp1), VPOi_locSetBuild(temp1, NULL));
            } else {
                ostrstream ost;
                ost << "processConversion: unimplemented int->FP conversion in "
                    << *exp;
                error(str(ost));
                return Rtl_int(0); // must return something
            }
            break;
        }
            
        case idFtoi: {          // Floating point to int (and size) conversion
            Rtl_ty_loc temp1 = getTempReg(fromType);
            VPOi_rtl(Rtl_assign(temp1, fromSize,
                                processExpr(subExpr, fromType)), NULL);

            if (toSize == 32) {
                Rtl_ty_loc temp = getTempReg(toType);
                VPOi_rtl(FIX(temp, temp1, fromSize),
                         VPOi_locSetBuild(temp1, NULL));
            } else {
                ostrstream ost;
                ost << "processConversion: unimplemented FP->int conversion in "
                    << *exp;
                error(str(ost));
                return Rtl_int(0); // must return something
            }
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
            return Rtl_int(0);  // must return something
        }
    }
    return Rtl_fetch(temp, toSize);
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::processIntConversion
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
Rtl_ty_loc ARMVPOBackend::processIntConversion(const SemStr* expr,
                                               Type fromType,
                                               Type toType,
                                               Rtl_ty_oper rshift,
                                               int shiftCount) {
    int toSize = toType.getSize();
    Rtl_ty_expr shift;
    
    // get expr's value into a temp reg of size toSize
    Rtl_ty_loc temp = getTempReg(toType);
    // int actualToSize = ((toSize < 32)? 32 : toSize);
    VPOi_rtl(Rtl_assign(temp, toSize, processExpr(expr, fromType)), NULL);

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
 * FUNCTION:        ARMVPOBackend::emitCastIfNeeded
 * OVERVIEW:        Emits VPO RTLs if necessary to cast (take existing bits
 *                  of one type and reinterpret them, without conversion, as a
 *                  second type) the value in "fromLoc" from "fromType" to
 *                  a new value in the result location of type "toType".
 * PARAMETERS:      fromLoc: location holding value to potentially cast
 *                  fromType: fromLoc's current type
 *                  toType: the needed type
 * RETURNS:         Location holding a possibly cast result
 *===========================================================================*/
static void castError(Type fromType, Type toType) {
    ostrstream ost;
    ost << "emitCastIfNeeded: cast required but unimplemented: from "
        << fromType.getCtype() << " to " << toType.getCtype();
    error(str(ost));
}

Rtl_ty_loc ARMVPOBackend::emitCastIfNeeded(Rtl_ty_loc fromLoc, Type fromType,
                                           Type toType) {
    if (fromType == toType) {
        return fromLoc;
    }
    
    // If just changing the signedness of integer types of the same size, or
    // increasing the size of integer types, nothing must be done.
    if ((fromType.getType() == INTEGER) && (toType.getType() == INTEGER) &&
        (fromType.getSize() <= 32) && (toType.getSize() == 32)) {
        return fromLoc;
    }

    int fromSize = fromType.getSize();
    int toSize   = toType.getSize();
    Rtl_ty_storage memType;
    Rtl_ty_expr addr;
    Rtl_ty_loc newLoc;
    
    switch (fromType.getType()) {
        case INTEGER: { 
            if (toType.getType() == INTEGER) {
                // int -> int
                if ((fromSize > 32) || (toSize > 32)) {
                    castError(fromType, toType);
                }
                if (fromSize == toSize) {
                    return fromLoc;
                }
                // store int (fromLoc) to memory
                memType = memoryTypeForType(fromType);
                addr = getSymAddr(fp, memTempSym, 0);
                VPOi_rtl(Rtl_assign(Rtl_location(memType, addr), fromSize,
                                    Rtl_fetch(fromLoc, fromSize)), NULL);
                // now reload as an int of toType
                newLoc = emitLoad(addr, toType, false);
            } else if (toType.getType() == FLOATP) {
                // int -> FP
                if (fromSize > 32) {
                    castError(fromType, toType);
                    return fromLoc;         // must return something
                }
                // store int to memory
                memType = memoryTypeForType(fromType);
                addr = getSymAddr(fp, memTempSym, 0);
                VPOi_rtl(Rtl_assign(Rtl_location(memType, addr), fromSize,
                                    Rtl_fetch(fromLoc, fromSize)), NULL);
                // reload the "int" as FP
                newLoc = getTempReg(toType);
                if (toSize == 32) {
                    VPOi_rtl(LDFS(newLoc, 0, addr), NULL);
                } else if (toSize == 64) {
                    assert(0);  // do we ever cast 32 bit ints to 64 bit FP?
                    VPOi_rtl(LDFD(newLoc, 0, addr), NULL);
                } else {
                    castError(fromType, toType);
                    return fromLoc;         // must return something
                }
            } else {
                castError(fromType, toType);
                return fromLoc;         // must return something
            }
            break;
        }

        case FLOATP: {
            if (toType.getType() == INTEGER) {
                // FP -> int
                if (toSize > 32) {
                    castError(fromType, toType);
                    return fromLoc;         // must return something
                }
                // store FP to memory
                addr = getSymAddr(fp, memTempSym, 0);
                if (fromSize == 32) {
                    VPOi_rtl(STFS(fromLoc, 0, addr), NULL);
                } else {
                    VPOi_rtl(STFD(fromLoc, 0, addr), NULL);
                }
                // now reload the "FP" in memory as an int
                newLoc = emitLoad(addr, toType, false);
            } else if (toType.getType() == FLOATP) {
                // FP -> FP
                if (fromSize == toSize) {
                    return fromLoc;
                }
                // store FP (fromLoc) to memory
                addr = getSymAddr(fp, memTempSym, 0);
                if (fromSize == 32) {
                    VPOi_rtl(STFS(fromLoc, 0, addr), NULL);
                } else {
                    VPOi_rtl(STFD(fromLoc, 0, addr), NULL);
                }
                // now reload as an FP of toType
                newLoc = getTempReg(toType);
                if (toSize == 32) {
                    VPOi_rtl(LDFS(newLoc, 0, addr), NULL);
                } else if (toSize == 64) {
                    VPOi_rtl(LDFD(newLoc, 0, addr), NULL);
                } else {
                    castError(fromType, toType);
                    return fromLoc;         // must return something
                }
            } else {
                castError(fromType, toType);
                return fromLoc;         // must return something
            }
            break;
        }

        default: {
            castError(fromType, toType);
            return fromLoc;         // must return something
        }
    }
    return newLoc;
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::jumpToBBLabel
 * OVERVIEW:        Emit a jump to the basic block with the specified label.
 * PARAMETERS:      label: integer label for the target basic block.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::jumpToBBLabel(int label) {
    ostrstream ost;
    ost << "Jump to basic block with label " << label;
    VPOi_asm->comment(str(ost));
    
    assert(labelMap.find(label) != labelMap.end());
    AsmSymbol labelSym = labelMap[label];
    VPOi_rtl(Rtl_assign(VPOi_loc_PC, 32,
                        Rtl_relAddr(labelSym->relAddr)), NULL);
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::getSymAddr
 * OVERVIEW:        Emit code to generate the address of a symbol plus constant
 * PARAMETERS:      baseReg: the base register to use to compute the address
 *                  sym: a VPO AsmSymbol for a symbol.
 *                  offset: an integer constant.
 * RETURNS:         VPO expression for the symbol's address.
 *===========================================================================*/
Rtl_ty_expr ARMVPOBackend::getSymAddr(Rtl_ty_loc baseReg,
                                      AsmSymbol sym, int offset) {
    Rtl_ty_relAddr raddr = sym->relAddr;
    Rtl_ty_expr addr = Rtl_binary(Rtl_op_add, Rtl_fetch(baseReg, 32),
                                  Rtl_relAddr(raddr));
    if (offset != 0) {
        if (!fitsIn12Bits(offset)) {
            ostrstream ost;
            ost << "getSymAddr: offset " << offset << " is more than 12 bits";
            error(str(ost));
        }

        // We must use a temp and two add RTLs or otherwise VPO finds the expr
        // too complex to generate assembler code for the ARM.
        Type addressType = Type(INTEGER, 32, false);
        Rtl_ty_loc temp = getTempReg(addressType);
        VPOi_rtl(Rtl_assign(temp, 32, addr), NULL);
        
        addr = Rtl_binary(Rtl_op_add, Rtl_fetch(temp, 32), Rtl_uint(offset));
    }
    return addr;
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::loadIntConst
 * OVERVIEW:        Emit code to load a 32 bit integer constant into a
 *                  temporary register and return that register.
 * PARAMETERS:      value: an integer.
 *                  cType: the required type for the constant.
 * RETURNS:         The temporary register holding the integer constant.
 *===========================================================================*/
Rtl_ty_loc ARMVPOBackend::loadIntConst(int value, Type cType) {
    Rtl_ty_loc temp = getTempReg(cType);
    assert(cType.getSize() <= 32);
    if (fitsIn8Bits(value)) {
        bool isSigned = cType.getSigned();
        Rtl_ty_expr iconst = (isSigned? Rtl_int(value) : Rtl_uint(value));
        VPOi_rtl(MOV(temp, iconst), NULL);
    } else {
        // More work must be done: Place the constant in memory and
        // load the register from memory. We declare its variable once and
        // reuse it where possible. We initialize the variable later with
        // emitDeferredConstants()
        AsmSymbol sym;
        if (defIConstMap.find(value) == defIConstMap.end()) {
            sym = genLabel();
            // record the var to be initialized by emitDeferredConstants()
            defIConstMap[value] = sym;
        } else {
            sym = defIConstMap[value];
        }
        Rtl_ty_expr addr = Rtl_relAddr(sym->relAddr);
        Rtl_ty_expr loadedWord = Rtl_fetch(Rtl_location('m', addr), 32);
        VPOi_rtl(Rtl_assign(temp, 32, loadedWord), NULL);
    }
    return temp;
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::loadFPConst
 * OVERVIEW:        Emit code to load a 64 bit FP constant into a
 *                  temporary register and return that register.
 * PARAMETERS:      value: a double FP value.
 * RETURNS:         The temporary register holding the FP constant.
 *===========================================================================*/
Rtl_ty_loc ARMVPOBackend::loadFPConst(double value) {
    Type dblType = Type(FLOATP, 64, true);
    Rtl_ty_loc temp = getTempReg(dblType);
    // Place the constant in memory and load the register from memory. We
    // declare its variable once and reuse it where possible. We initialize
    // the variable later with emitDeferredConstants()
    AsmSymbol sym;
    if (defFConstMap.find(value) == defFConstMap.end()) {
        sym = genLabel();
        // record the var to be initialized by emitDeferredConstants()
        defFConstMap[value] = sym;
    } else {
        sym = defFConstMap[value];
    }

    // Load the FP constant into d[7] then copy it into temp.
    // First save any FP value currently in d[2].
    Rtl_ty_expr memAddr = getSymAddr(fp, memTempSym, 0);
    VPOi_rtl(Rtl_assign(Rtl_location('D', memAddr), 64,
                        Rtl_fetch(d[7], 64)), NULL);

    // Load the constant into d[7]
    char asmtext[100];
    sprintf(asmtext, "\tldfd\tf7,%s\n", sym->name);
    VPOi_asm->asmtext(asmtext);

    // Copy the constant in d[7] to temp
    VPOi_rtl(Rtl_assign(temp, 64, Rtl_fetch(d[7], 64)), NULL);

    // Now restore d[7]'s old value
    VPOi_rtl(Rtl_assign(d[7], 64,
                        Rtl_fetch(Rtl_location('D', memAddr), 64)), NULL);
    return temp;
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::emitDeferredConstants
 * OVERVIEW:        Emit code to define any constants in the defFConstMap and
 *                  defIConstMap maps.
 * PARAMETERS:      none.
 * RETURNS:         nothing.
 *===========================================================================*/
void ARMVPOBackend::emitDeferredConstants() {
    if ((defIConstMap.size() == 0) && (defFConstMap.size() == 0)) {
        return;
    }

    ostrstream ost;
    ost << "Jump around deferred constants";
    VPOi_asm->comment(str(ost));
    // Bypass VPO and emit asm code directly to avoid VPO optimizing away the
    // constant block and the branch around it!        
    //  AsmSymbol labelSym = genLabel();
    //  VPOi_rtl(Rtl_assign(VPOi_loc_PC, 32, Rtl_relAddr(labelSym->relAddr)),0);
    char labelName[20], scratch[60];
    sprintf(labelName, ".L%d", nextGenLabelNo);
    nextGenLabelNo++;
    sprintf(scratch, "\tb\t%s\n", labelName);
    VPOi_asm->asmtext(scratch);       

    if (defFConstMap.size() > 0) {
        // emit FP constants into the *code* stream
        VPOi_asm->align(8);
        map<double, AsmSymbol>::iterator cf;
        for (cf = defFConstMap.begin();  cf != defFConstMap.end();  cf++) {
            double value  = cf->first;
            AsmSymbol sym = cf->second;
            
            // Force a reasonable amount of precision, so things like 0.5
            // come out without unnecessary digits.
            ostrstream ost;
            ost.precision(18);
            ost << value;

            sprintf(scratch, "%s:\n", sym->name);
            VPOi_asm->asmtext(scratch);       
            VPOi_asm->define_symbol_here(sym);
            VPOi_asm->emitf64s(str(ost));
        }
        // clear deferred FP constants map since those constants were emitted
        defFConstMap.clear();
    }

    if (defIConstMap.size() > 0) {
        // emit int constants
        VPOi_asm->align(4);
        map<int, AsmSymbol>::iterator ci;
        for (ci = defIConstMap.begin();  ci != defIConstMap.end();  ci++) {
            int value = ci->first;
            AsmSymbol sym = ci->second;
            sprintf(scratch, "%s:\n", sym->name);
            VPOi_asm->asmtext(scratch);       
            VPOi_asm->define_symbol_here(sym);
            VPOi_asm->emit(value, 4);
        }
        // clear deferred int constants map since those constants were emitted
        defIConstMap.clear();
    }

    // define target of jump above
    // VPOi_asm->define_symbol_here(labelSym);
    sprintf(scratch, "%s:\n", labelName);
    VPOi_asm->asmtext(scratch); 
}


/*
 *=============================================================================
 * Utility methods.
 *=============================================================================
 */

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::emitLoad
 * OVERVIEW:        Emit a VPO RTL to load a byte, halfword, etc., value from
 *                  memory into a new temporary ARM register.
 * PARAMETERS:      addr: address of value to load (a VPO expression).
 *                  cType: the expected type of the value.
 *                  emitSwaps: true if to emit code (if !progOptions.noendian)
 *                    that swaps bytes read from memory.
 * RETURNS:         The VPO location for the temporary register.
 *===========================================================================*/
Rtl_ty_loc ARMVPOBackend::emitLoad(Rtl_ty_expr addr, Type cType,
                                   bool emitSwaps) {
    Rtl_ty_loc loc = Rtl_location(memoryTypeForType(cType), addr);
    int neededSize = cType.getSize(); // desired size in bits 
    Rtl_ty_expr expr = Rtl_fetch(loc, neededSize);

    // ARM VPO requires we add extra "semantic" info to emit 8 and 16 bit loads
    if (cType.getType() == INTEGER) {
        if (neededSize == 8) {
            if (cType.getSigned()) {   // LDRSB (load signed byte)
                expr = Rtl_binary(Rtl_op_lshift,  expr, Rtl_int(24));
                expr = Rtl_binary(Rtl_op_rshiftA, expr, Rtl_int(24));
            } else {                   // LDRB  (load unsigned byte)
                expr = Rtl_binary(Rtl_op_and, expr, Rtl_uint(0xFF));
            }
        } else if (neededSize == 16) {
            if (cType.getSigned()) {   // LDRSH (load signed halfword)
                expr = Rtl_binary(Rtl_op_lshift,  expr, Rtl_int(16));
                expr = Rtl_binary(Rtl_op_rshiftA, expr, Rtl_int(16));
            } else {                   // LDRH  (load unsigned halfword)
                expr = Rtl_binary(Rtl_op_and, expr, Rtl_uint(0xffff));
            }
        }
    } // else LDR, LDFS, LDFD, which require no special treatment

    // load the value into a temp register
    Rtl_ty_loc temp = getTempReg(cType);
    int assignSize = ((neededSize > 32)? neededSize : 32);
    VPOi_rtl(Rtl_assign(temp, assignSize, expr), NULL);

    // emit byte swaps if needed (and unless progOptions.noendian)
    if (emitSwaps && doingMemorySwaps) {
        emitByteSwaps(temp, (neededSize / 8), cType.getType());
    }
    return temp;
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::emitByteSwaps
 * OVERVIEW:        Emit code to swap one or more bytes stored in a VPO
 *                  temporary register.
 * PARAMETERS:      loc: VPO location for temporary register holding a value
 *                  to byteswap.
 *                  numBytes: number of bytes in loc to swap.
 *                  type: INTEGER or FLOATP depending on type of value in loc.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::emitByteSwaps(Rtl_ty_loc loc, int numBytes,
                                  LOC_TYPE type) {
    Rtl_ty_loc temp;
    Type wordType = Type(INTEGER, 32, false);
#ifdef USE_INLINE_SWAPS           
    Rtl_ty_loc reg1, reg2;
#endif

    // Ensure loc's bytes are in an integer register "temp"
    if (type == FLOATP) {
        // store the bytes of the FP loc to memory and reload into a new temp
        if (numBytes == 4) {
            Rtl_ty_expr memAddr = getSymAddr(fp, memTempSym, 0);
            VPOi_rtl(Rtl_assign(Rtl_location('F', memAddr), 32,
                                Rtl_fetch(loc, 32)), NULL);

            temp = getTempReg(wordType);
            VPOi_rtl(Rtl_assign(temp, 32,
                                Rtl_fetch(Rtl_location('m', memAddr),32)),NULL);
        } else if (numBytes == 8) {
            // nothing more is needed: emitSwap8Call() handles this case
            temp = loc;
        }
    } else {
        temp = loc;
    }

    // Swap the bytes in location "temp"
    switch (numBytes) {
        case 1:
            // nothing needed
            break;
            
        case  2:
#ifdef USE_INLINE_SWAPS           
            reg1 = getTempReg(wordType);                    // temp = WXYZAABB
            VPOi_rtl(MOV(reg1, Rtl_int(0xFF00)), NULL);     // reg1 = 0000FF00
            //VPOi_rtl(ORR(reg1, reg1, Rtl_int(0xFF)), NULL); // reg1 = 0000FFFF
            
            reg2 = getTempReg(wordType);
            VPOi_rtl(LSL(reg2, temp, Rtl_int(16)), NULL);   // reg2 = AABB0000
            
            VPOi_rtl(LSR(temp, reg2, Rtl_int(8)), NULL);    // temp = 00AABB00
            VPOi_rtl(AND(reg1, reg1, Rtl_fetch(temp, 32)),  // reg1 = 0000BB00
                     NULL);
            
            VPOi_rtl(LSR(temp, reg2, Rtl_int(24)),
                     VPOi_locSetBuild(reg2, NULL));         // temp = 000000AA
            VPOi_rtl(ORR(temp, reg1, Rtl_fetch(temp, 32)),  // temp = 0000BBAA
                     VPOi_locSetBuild(reg1, NULL));
#else
            emitSwap2or4Call(temp, 2);
#endif /*USE_INLINE_SWAPS*/            
            break;
            
        case 4:
#ifdef USE_INLINE_SWAPS           
            reg1 = getTempReg(wordType);                    // temp = AABBCCDD
            VPOi_rtl(MOV(reg1, Rtl_int(0x00FF0000)), NULL);
            VPOi_rtl(ORR(reg1, reg1, Rtl_int(0xFF)), NULL); // reg1 = 00FF00FF
            
            reg2 = getTempReg(wordType);
            VPOi_rtl(AND(reg2, temp, Rtl_fetch(reg1, 32)),  // reg2 = 00BB00DD
                     NULL);
            
            VPOi_rtl(LSR(temp, temp, Rtl_int(8)), NULL);
            VPOi_rtl(AND(reg1, reg1, Rtl_fetch(temp, 32)),  // reg1 = 00AA00CC
                     NULL);
            
            VPOi_rtl(LSL(temp, reg2, Rtl_int(8)), NULL);
            VPOi_rtl(ORR(reg1, reg1, Rtl_fetch(temp, 32)),  // reg1 = BBAADDCC
                     VPOi_locSetBuild(reg2, NULL));
            
            VPOi_rtl(LSR(temp, reg1, Rtl_int(16)), NULL);   // temp = 0000BBAA
            
            VPOi_rtl(LSL(reg1, reg1, Rtl_int(16)), NULL);
            VPOi_rtl(ORR(temp, temp, Rtl_fetch(reg1, 32)),  // temp = DDCCBBAA
                     VPOi_locSetBuild(reg1, NULL));
#else
            emitSwap2or4Call(temp, 4);
#endif /*USE_INLINE_SWAPS*/            
            break;
            
        case 8:
            emitSwap8Call(temp, type);
            break;
            
        default: {
            ostrstream ost;
            ost << "emitByteSwaps: unexpected numBytes " << numBytes;
            error(str(ost));
        }
    }
    
    // If necessary, copy temp's bytes back into loc
    if (type == FLOATP) {
        if (numBytes == 4) {
            Rtl_ty_expr memAddr = getSymAddr(fp, memTempSym, 0);
            VPOi_rtl(Rtl_assign(Rtl_location('m', memAddr), 32,
                                Rtl_fetch(temp, 32)),
                     VPOi_locSetBuild(temp, NULL));

            VPOi_rtl(Rtl_assign(loc, 32,
                                Rtl_fetch(Rtl_location('F', memAddr), 32)),
                     NULL);
        }
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::emitSwap2or4Call
 * OVERVIEW:        Emit code to call a utility procedure to swap 2 or 4 bytes
 *                  stored in a VPO temp register.
 * PARAMETERS:      temp: VPO location for temporary register holding the
 *                  value to byteswap.
 *                  numBytes: number of bytes to swap, 2 or 4.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::emitSwap2or4Call(Rtl_ty_loc temp, int numBytes) {
    AsmSymbol calleeSym;
    if (numBytes == 2) {
        calleeSym = swap2Sym;
    } else if (numBytes == 4) {
        calleeSym = swap4Sym;
    } else {
        ostrstream ost;
        ost << "emitSwap2or4Call: unexpected numBytes " << numBytes << "!";
        error(str(ost));
        return;
    }

    // Set r[0] to the value in temp
    VPOi_rtl(Rtl_assign(r[0], 32, Rtl_fetch(temp, 32)), NULL);
    
    // Tell VPO the size of the argument build area and the number of arguments
    VPOi_parameterListSize(/*totalBytes*/ 4, /*numParams*/ 1);

    // Define the set of registers holding parameters
    VPOi_ty_locSet param_set = VPOi_locSetBuild(r[0], NULL);

    // Define the registers returned by the library routine
    VPOi_ty_locSet defined_set = VPOi_locSetBuild(r[0], NULL);

    // Emit the call to the swap procedure
    Rtl_ty_rtl callRtl = jmp(Rtl_relAddr(calleeSym->relAddr));
    VPOi_functionCall(callRtl, dead_set, param_set, defined_set,
                      callerSave_set, killed_set, used_set);

    // Store result back into temp
    VPOi_rtl(Rtl_assign(temp, 32, Rtl_fetch(r[0], 32)), NULL);
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::emitSwap8Call
 * OVERVIEW:        Emit code to call a utility procedure to swap 8 bytes
 *                  stored in a VPO temp register.
 * PARAMETERS:      temp: VPO location for 8 byte temporary register holding
 *                  a value to byteswap.
 *                  type: INTEGER or FLOATP depending on type of value in temp.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::emitSwap8Call(Rtl_ty_loc temp, LOC_TYPE type) {
    AsmSymbol calleeSym;
    Rtl_ty_storage memType;
    if (type == FLOATP) {
        calleeSym = swap8fSym;
        memType = 'D';
    } else if (type == INTEGER) {
        // calleeSym = swap8iSym;
        // memType = ???;
        ostrstream ost;
        ost << "emitSwap8Call: 8 byte integers not yet supported on the ARM!";
        error(str(ost));
        return;
    } else {
        ostrstream ost;
        ost << "emitSwap8Call: unexpected type " << (int)type << "!";
        error(str(ost));
        return;
    }

    // Store the 8 byte temp to memory (memTempSym)
    Rtl_ty_expr addr = getSymAddr(fp, memTempSym, 0);
    Rtl_ty_loc dest = Rtl_location(memType, addr);
    VPOi_rtl(Rtl_assign(dest, 64, Rtl_fetch(temp, 64)), NULL);

    // Load each of temp's two words from memory into regs r0 and r1
    Rtl_ty_expr word = Rtl_fetch(Rtl_location('m', addr), 32);
    VPOi_rtl(Rtl_assign(r[0], 32, word), NULL);
    
    addr = getSymAddr(fp, memTempSym, 4);
    word = Rtl_fetch(Rtl_location('m', addr), 32);
    VPOi_rtl(Rtl_assign(r[1], 32, word), NULL);

    // Tell VPO the size of the argument build area and the number of arguments
    VPOi_parameterListSize(/*totalBytes*/ 8, /*numParams*/ 1);

    // Define the set of registers holding parameters
    VPOi_ty_locSet param_set = VPOi_locSetBuild(r[0], r[1], NULL);

    // Define the registers returned by the library routine
    VPOi_ty_locSet defined_set;
    if (type == INTEGER) {
        defined_set = VPOi_locSetBuild(r[0], r[1], NULL);
    } else {
        defined_set = VPOi_locSetBuild(f[0], NULL);
    }

    // Emit the call to the swap procedure
    Rtl_ty_rtl callRtl = jmp(Rtl_relAddr(calleeSym->relAddr));
    VPOi_functionCall(callRtl, dead_set, param_set, defined_set,
                      callerSave_set, killed_set, used_set);

    // Store result back into temp
    if (type == INTEGER) {
        // Store r0 and r1 to memory (memTempSym)
        dest = Rtl_location('m', getSymAddr(fp, memTempSym, 0));
        VPOi_rtl(Rtl_assign(dest, 32, Rtl_fetch(r[0], 32)), NULL);

        dest = Rtl_location('m', getSymAddr(fp, memTempSym, 4));
        VPOi_rtl(Rtl_assign(dest, 32, Rtl_fetch(r[0], 32)), NULL);

        // Load temp from memory
        addr = getSymAddr(fp, memTempSym, 0);
        Rtl_ty_loc src = Rtl_location(memType, addr);
        VPOi_rtl(Rtl_assign(temp, 64, Rtl_fetch(src, 64)), NULL);
    } else {
        // Store f0 to memory (memTempSym)
        addr = getSymAddr(fp, memTempSym, 0);
        dest = Rtl_location('D', addr);
        VPOi_rtl(Rtl_assign(dest, 32, Rtl_fetch(f[0], 32)), NULL);

        // Load temp from memory
        Rtl_ty_loc src = Rtl_location('D', addr);
        VPOi_rtl(Rtl_assign(temp, 64, Rtl_fetch(src, 64)), NULL);
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::emitArgvSwaps
 * OVERVIEW:        Emit code to swap the elements of argv when translating
 *                  main(argc, argv,...).
 * PARAMETERS:      None.
 * RETURNS:         Nothing.
 *===========================================================================*/
void ARMVPOBackend::emitArgvSwaps() {
    // emit call to _vpo_swapargv(argc, argv)
    int numParams = proc->getNumArgs();
    if (numParams < 2) {
        ostrstream ost;
        ost << "emitArgvSwaps: main() has fewer than two arguments!";
        error(str(ost));
        return;
    }
    
    const list<SemStr>& params = proc->getParams();
    list<SemStr>::const_iterator pp = params.begin();
    SemStr argcParam = *pp;
    pp++;
    SemStr argvParam = *pp;
    if ((argcParam.getType().getType() != INTEGER) ||
        (argcParam.getType().getSize() != 32) ||
        (argvParam.getType().getSize() != 32)) {
        ostrstream ost;
        ost << "emitArgvSwaps: first argument not int32 or second not 32 bits";
        error(str(ost));
        return;
    }

    // Get the VPO AsmSymbol for the first formal parameter variable (argc)
    string paramName = argcParam.sprint();
    bool isArgcReg = (paramName[0] == 'r');
    AsmSymbol argcParamSym;     // if isArgcReg: "reg" parameter such as r[0]
    int argcOffset = 0;         // if !isArgcReg: off. of sym var in symVarsSym
    if (isArgcReg) {
        int reg = argcParam.getThirdIdx();
        assert(regVarMap.find(reg) != regVarMap.end());
        argcParamSym = regVarMap[reg];
    } else {
        assert(argcParam.getFirstIdx() == idVar);
        int NN = argcParam.getSecondIdx();
        assert(symVarOffsetMap.find(NN) != symVarOffsetMap.end());
        argcOffset = symVarOffsetMap[NN];
    }

    // Now get the AsmSymbol for the second (argv) formal parameter variable
    paramName = argvParam.sprint();
    bool isArgvReg = (paramName[0] == 'r');
    AsmSymbol argvParamSym;     // if isArgvReg: "reg" parameter such as r[0]
    int argvOffset = 0;         // if !isArgvReg: off. of sym var in symVarsSym
    if (isArgvReg) {
        int reg = argvParam.getThirdIdx();
        assert(regVarMap.find(reg) != regVarMap.end());
        argvParamSym = regVarMap[reg];
    } else {
        assert(argvParam.getFirstIdx() == idVar);
        int NN = argvParam.getSecondIdx();
        assert(symVarOffsetMap.find(NN) != symVarOffsetMap.end());
        argvOffset = symVarOffsetMap[NN];
    }

    // Tell VPO the size of the argument build area and the number of arguments
    VPOi_parameterListSize(/*totalBytes*/ 8, /*numParams*/ 2);

    // Copy the arguments to r0 and r1.
    Rtl_ty_loc loc;
    if (isArgcReg) {
        loc = Rtl_location('m', getSymAddr(fp, argcParamSym, 0));
    } else {
        loc = Rtl_location('m', getSymAddr(fp, symVarsSym, argcOffset));
    }
    VPOi_rtl(Rtl_assign(r[0], 32, Rtl_fetch(loc, 32)), NULL);

    if (isArgvReg) {
        loc = Rtl_location('m', getSymAddr(fp, argvParamSym, 0));
    } else {
        loc = Rtl_location('m', getSymAddr(fp, symVarsSym, argvOffset));
    }
    VPOi_rtl(Rtl_assign(r[1], 32, Rtl_fetch(loc, 32)), NULL);

    // Define the set of registers holding parameters
    VPOi_ty_locSet param_set = VPOi_locSetBuild(r[0], r[1], NULL);

    // Define the registers returned by the library routine
    VPOi_ty_locSet defined_set = VPOi_locSetBuild(NULL);

    // Emit the call to _vpo_swapargv
    Rtl_ty_rtl callRtl = jmp(Rtl_relAddr(swapargvSym->relAddr));
    VPOi_functionCall(callRtl, dead_set, param_set, defined_set,
                      callerSave_set, killed_set, used_set);
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::fitsIn8Bits
 * OVERVIEW:        Returns true if the integer "iconst" is nonnegative and
 *                  small enough to fit in a 8 bit ARM immediate field;
 *                  false otherwise.
 * PARAMETERS:      iconst: an integer.
 * RETURNS:         true if "iconst" fit in 8 bit ARM immediate field.
 *===========================================================================*/
bool ARMVPOBackend::fitsIn8Bits(int iconst) {
    return ((0 <= iconst) && (iconst <= 0xFF));
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::fitsIn12Bits
 * OVERVIEW:        Returns true if the integer "iconst" is nonnegative and
 *                  small enough to fit in a 12 bit ARM immediate field;
 *                  false otherwise.
 * PARAMETERS:      iconst: an integer.
 * RETURNS:         true if "iconst" fit in 12 bit ARM immediate field.
 *===========================================================================*/
bool ARMVPOBackend::fitsIn12Bits(int iconst) {
    return ((0 <= iconst) && (iconst <= 0xFFF));
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::regTypeForType
 * OVERVIEW:        Returns a VPO Rtl_ty_storage description for the kind of
 *                  ARM register needed to hold a UQBT int or float value type.
 * PARAMETERS:      type: a UQBT int or float Type.
 * RETURNS:         character describing the kind of ARM register needed.
 *===========================================================================*/
Rtl_ty_storage ARMVPOBackend::regTypeForType(Type type) {
    int numBits = type.getSize();
    int numBytes = ((numBits + 7)/8);
    Rtl_ty_storage regType;
    
    switch (type.getType()) {
        case INTEGER:
            regType = 'r';
            break;
            
        case FLOATP:
            if (numBytes == 4) {
                regType = 'f';
            } else {
                regType = 'd';
            }
            break;
            
        default:
            ostrstream ost;
            ost << "regTypeForType: unrecognized type in " << type.getCtype();
            error(str(ost));
    }
    return regType;
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::memoryTypeForType
 * OVERVIEW:        Returns a VPO Rtl_ty_storage description for the kind of
 *                  memory location needed to hold a UQBT int or float value.
 * PARAMETERS:      type: a UQBT int or float Type.
 * RETURNS:         character describing the kind of memory needed.
 *===========================================================================*/
Rtl_ty_storage ARMVPOBackend::memoryTypeForType(Type type) {
    int numBits = type.getSize();
    int numBytes = ((numBits + 7)/8);
    Rtl_ty_storage memType;

    switch (type.getType()) {
        case INTEGER:
            if (numBytes == 1) {
                memType = 'B';
            } else if (numBytes == 2) {
                memType = 'W';
            } else if (numBytes == 4) {
                memType = 'm';
            } else {
                ostrstream ost;
                ost << "memoryTypeForType: int type not 8, 16, or 32 bits in "
                    << type.getCtype();
                error(str(ost));
                memType = 'r';      // return something
            }
            break;
            
        case FLOATP:
            if (numBytes == 4) {
                memType = 'F';
            } else if (numBytes == 8) {
                memType = 'D';
            } else {
                ostrstream ost;
                ost << "memoryTypeForType: FP type not 32 or 64 bits in "
                    << type.getCtype();
                error(str(ost));
                memType = 'r';      // return something
            }
            break;
            
        default:
            ostrstream ost;
            ost << "memoryTypeForType: unrecognized type in "
                << type.getCtype();
            error(str(ost));
            memType = 'r';      // return something
    }
    return memType;
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::genLabel
 * OVERVIEW:        Return a VPO symbol for a new label. 
 * PARAMETERS:      none.
 * RETURNS:         VPO AsmSymbol for a new ARM label of the form .L9xxx.
 *                  These labels are distinguished from block labels by the
 *                  large number 9xxx above 9000.
 *===========================================================================*/
AsmSymbol ARMVPOBackend::genLabel() {
    AsmSymbol sym;
    char tmpName[20];

    /* Generate the label's name. ARM labels are of form .Lnnn */
    sprintf(tmpName, ".L%d", nextGenLabelNo);
    nextGenLabelNo++;

    /* Create the symbol and declare it to VPO. */
    sym = VPOi_asm->local(tmpName);
    return sym;
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::genTemp
 * OVERVIEW:        Return a VPO symbol for a new local temporary variable. 
 * PARAMETERS:      none.
 * RETURNS:         VPO AsmSymbol for a new local temporary variable.
 *===========================================================================*/
AsmSymbol ARMVPOBackend::genTemp() {
    AsmSymbol sym;
    char tmpName[40];

    /* Generate the temporary's name. */
    sprintf(tmpName, ".TMP.%d", nextTempVarNo);
    nextTempVarNo++;

    /* Create the symbol and declare it to VPO. */
    sym = VPOi_asm->local(tmpName);
    return sym;
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::replaceNLs
 * OVERVIEW:        Return the argument string with any newlines replaced
 *                  by ";" unless at the end, where a newline is deleted.
 * PARAMETERS:      s: null terminated char * string.
 * RETURNS:         null terminated char * string without embedded newlines.
 *===========================================================================*/
char* ARMVPOBackend::replaceNLs(char * s) {
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
 * FUNCTION:        ARMVPOBackend::passParams
 * OVERVIEW:        Emits code to pass the parameters of a procedure call.
 * PARAMETERS:      call: pointer to HLCall structure describing the call.
 *                  param_set: reference to VPO location set describing what
 *                  registers hold parameters.
 * RETURNS:         nothing.
 *===========================================================================*/
void ARMVPOBackend::passParams(const HLCall* call, VPOi_ty_locSet& param_set) {
    int totalBytes, totalWords, nBytes, nWords, used;

    // First discover the total number of words needed for parameters
    const list<SemStr>& params = call->getParams();
    totalBytes = 0;
    list<SemStr>::const_iterator pp;
    for (pp = params.begin();  pp != params.end();  pp++) {
        SemStr param = *pp;
        const Type& paramType = param.getType();
        nBytes = (paramType.getSize() + 7)/8;
        nBytes = max(nBytes, 4); // 4 byte min for each parameter
        if ((nBytes != 4) && (nBytes != 8)) {
            ostrstream ost;
            ost << "FP parameter " << param
                << " is not 4 or 8 bytes in ";
            call->print(ost, 0);
            error(str(ost));
        }
        if (paramType.getType() == VARARGS) {
            ostrstream ost;
            ost << "Varargs parameters are not implemented: parameter "
                << param << " in ";
            call->print(ost, 0);
            error(str(ost));
        }
        totalBytes += nBytes;
    }
    totalWords = totalBytes/4;
    
    // Tell VPO the size of the argument build area and the number of arguments
    int numParams = params.size();
    VPOi_parameterListSize(totalBytes, numParams);

    // Define param_set: regs used in callee, defined in caller
    int numInIntRegs = min(totalWords, 4);
    for (int i = 0;  i < numInIntRegs;  i++) {
        param_set = VPOi_locSetAdd(param_set, r[i]);
    }

    // Store the parameters in r[0-3] then, if necessary, on the stack
    used = 0;                   // number of parameter words used so far
    for (pp = params.begin();  pp != params.end();  pp++) {
        SemStr param = *pp;
        const Type& paramType = param.getType();

        // parameters are passed in one or more 32 bit words
        nBytes = (paramType.getSize() + 7)/8;
        nBytes = max(nBytes, 4); 
        nWords = nBytes/4;
        assert((nWords == 1) || (nWords == 2));
        Type passedType = paramType;
        passedType.setSize(nWords * 32);
        
        Rtl_ty_storage memType = memoryTypeForType(passedType);
        Type wordTempType = Type(INTEGER, 32, false);
        Rtl_ty_expr addr, word;
        Rtl_ty_loc dest;

        Rtl_ty_expr paramExpr = processExpr(&param, passedType);
        if ((nWords == 2) || (passedType.getType() == FLOATP)) {
            // store parameter to temporary var (memTempSym) in memory
            addr = getSymAddr(fp, memTempSym, 0);
            dest = Rtl_location(memType, addr);
            VPOi_rtl(Rtl_assign(dest, (nWords*32), paramExpr), NULL);
        }
        
        if ((used + nWords) <= 4) {
            // The parameter fits completely in one or more of r[0-3].
            int paramReg = used;
            if (nWords == 2) {
                // load the 64 bit param's words into adjacent regs
                addr = getSymAddr(fp, memTempSym, 0);
                word = Rtl_fetch(Rtl_location('m', addr), 32);
                VPOi_rtl(Rtl_assign(r[paramReg], 32, word), NULL);
                
                addr = getSymAddr(fp, memTempSym, 4);
                word = Rtl_fetch(Rtl_location('m', addr), 32);
                VPOi_rtl(Rtl_assign(r[paramReg + 1], 32, word), NULL);
            } else if (passedType.getType() == FLOATP) {
                addr = getSymAddr(fp, memTempSym, 0);
                word = Rtl_fetch(Rtl_location(memType, addr), 32);
                VPOi_rtl(Rtl_assign(r[paramReg], 32, word), NULL);
            } else {
                VPOi_rtl(Rtl_assign(r[paramReg], 32, paramExpr), NULL);
            }
        } else if (used >= 4) {
            // All of r0-r3 are already used: pass parameter on stack.
            int spOffset = 4*(used - 4); // store lower addr word at m[spOffset]
            Rtl_ty_loc temp = getTempReg(wordTempType);
            if (nWords == 2) {
                // store lower addressed word of param on stack at lower addr
                addr = getSymAddr(fp, memTempSym, 0);
                word = Rtl_fetch(Rtl_location('m', addr), 32);
                VPOi_rtl(Rtl_assign(temp, 32, word), NULL);
                
                addr = Rtl_binary(Rtl_op_add, Rtl_fetch(sp, 32),
                                  Rtl_uint(spOffset));
                VPOi_rtl(Rtl_assign(Rtl_location('m', addr), 32,
                                    Rtl_fetch(temp, 32)), NULL);
                
                // store higher addressed word of param on stack at higher addr
                addr = getSymAddr(fp, memTempSym, 4);
                word = Rtl_fetch(Rtl_location('m', addr), 32);
                VPOi_rtl(Rtl_assign(temp, 32, word), NULL);
                
                addr = Rtl_binary(Rtl_op_add, Rtl_fetch(sp, 32),
                                  Rtl_uint(spOffset + 4));
                VPOi_rtl(Rtl_assign(Rtl_location('m', addr), 32,
                                    Rtl_fetch(temp, 32)),
                         VPOi_locSetBuild(temp, NULL));
            } else if (passedType.getType() == FLOATP) {
                addr = getSymAddr(fp, memTempSym, 0);
                word = Rtl_fetch(Rtl_location(memType, addr), 32);
                VPOi_rtl(Rtl_assign(temp, 32, word), NULL);
                
                addr = Rtl_binary(Rtl_op_add, Rtl_fetch(sp, 32),
                                  Rtl_uint(spOffset));
                VPOi_rtl(Rtl_assign(Rtl_location(memType, addr), 32,
                                    Rtl_fetch(temp, 32)),
                         VPOi_locSetBuild(temp, NULL));
            } else {
                VPOi_rtl(Rtl_assign(temp, 32, paramExpr), NULL);
                 
                addr = Rtl_binary(Rtl_op_add, Rtl_fetch(sp, 32),
                                  Rtl_uint(spOffset));
                VPOi_rtl(Rtl_assign(Rtl_location(memType, addr), 32,
                                    Rtl_fetch(temp, 32)),
                         VPOi_locSetBuild(temp, NULL));
            }
        } else {
            // The parameter must be split between registers and the stack.
            assert((used == 3) && (nWords == 2));
            Rtl_ty_loc temp = getTempReg(wordTempType);

            // load lower addressed word of param into r3
            addr = getSymAddr(fp, memTempSym, 0);
            word = Rtl_fetch(Rtl_location('m', addr), 32);
            VPOi_rtl(Rtl_assign(r[3], 32, word), NULL);
            
            // store higher addressed word of param onto stack
            addr = getSymAddr(fp, memTempSym, 4);
            word = Rtl_fetch(Rtl_location('m', addr), 32);
            VPOi_rtl(Rtl_assign(temp, 32, word), NULL);
            
            addr = Rtl_fetch(sp, 32); // i.e. store 2nd word at m[%sp]
            VPOi_rtl(Rtl_assign(Rtl_location('m', addr), 32,
                                Rtl_fetch(temp, 32)),
                     VPOi_locSetBuild(temp, NULL));
        }
        used += nWords;
    }
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::initRegsAndLocations
 * OVERVIEW:        Initialize VPO registers and locations. Can only be called
 *                  after VPOi_begin().
 * PARAMETERS:      none.
 * RETURNS:         <nothing>
 *===========================================================================*/
void ARMVPOBackend::initRegsAndLocations() {
    int i;
    // Initialize the ARM hardware registers
    for (i = 0;  i < 16;  i++) {
        r[i] = Rtl_constLoc('r', i); // r[0]-r[15]
    }
    for (i = 0;  i < 8;  i++) {
        f[i] = Rtl_constLoc('f', i); // f[0]-f[7]
        d[i] = Rtl_constLoc('d', i); // d[0]-d[7]
    }
    // convenient aliases for a subset of the r registers
    fp = r[11];
    ip = r[12];
    sp = r[13];
    lr = r[14];
    pc = r[15];
    
    // Initialize the caller save, kill, and used location sets. These are the
    // same for all ARM calls:
    callerSave_set = NULL;
    killed_set = NULL;
    used_set = NULL;

#ifdef USE_BTL_CALL_SETS    
    // Killed set: regs the callER must save before calls if it uses them later.
    for (i = 0;  i < 8;  i++) {
        killed_set = VPOi_locSetAdd(killed_set, f[i]);
        killed_set = VPOi_locSetAdd(killed_set, d[i]);
    }
    for (i = 0;  i < 4;  i++) {
        killed_set = VPOi_locSetAdd(killed_set, r[i]);
    }
    killed_set = VPOi_locSetAdd(killed_set, ip);
    killed_set = VPOi_locSetAdd(killed_set, lr);
    
    // Caller save set: regs the caller must save and restore after a call 
    for (i = 0;  i < 8;  i++) {
        callerSave_set = VPOi_locSetAdd(callerSave_set, f[i]);
        callerSave_set = VPOi_locSetAdd(callerSave_set, d[i]);
    }
    callerSave_set = VPOi_locSetAdd(callerSave_set, ip);
    
    // Used regs must not be xxx
    for (i = 0;  i < 4;  i++) {
        used_set = VPOi_locSetAdd(used_set, r[i]);
    }
    used_set = VPOi_locSetAdd(used_set, ip);
    used_set = VPOi_locSetAdd(used_set, f[0]);
    used_set = VPOi_locSetAdd(used_set, d[0]);

    // Initialize set of regs not needed after call
    dead_set = VPOi_locSetBuild(VPOi_loc_CPSR, VPOi_loc_FPSR, NULL);
#else /* use EDG front end call-related sets */
    callerSave_set = VPOi_locSetAdd(callerSave_set, ip);

    for (i=0;i<4;i++)
        callerSave_set = VPOi_locSetAdd(callerSave_set, r[i]);

    for (i=0;i<4;i++)
        callerSave_set = VPOi_locSetAdd(callerSave_set, f[i]);
    for (i=0;i<3;i++)
        callerSave_set = VPOi_locSetAdd(callerSave_set, d[i]);

    killed_set = callerSave_set;
    used_set = NULL;
    dead_set = NULL;
#endif /* BTL or EDG call-related sets */    

    // Initialize the VPO temporary registers
    curr_t = curr_x = curr_y = 0;
    for (i = 0;  i < NUM_TEMP_REGS;  i++) {
        t[i] = x[i] = y[i] = NULL;
    }
    t_base = x_base = y_base = 0;
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::declareLibraryRoutines
 * OVERVIEW:        Declare to VPO various library routines.
 * PARAMETERS:      none.
 * RETURNS:         <nothing>
 *===========================================================================*/
void ARMVPOBackend::declareLibraryRoutines() {
    divSym  = VPOi_asm->import("__divsi3");
    VPOi_globalDeclare(divSym, 'r');
    udivSym = VPOi_asm->import("__udivsi3");
    VPOi_globalDeclare(udivSym, 'r');
    
    modSym  = VPOi_asm->import("__modsi3");
    VPOi_globalDeclare(modSym, 'r');
    umodSym = VPOi_asm->import("__umodsi3");
    VPOi_globalDeclare(umodSym, 'r');

    swap2Sym = VPOi_asm->import("_vpo_swap2");
    VPOi_globalDeclare(swap2Sym, 'r');
    swap4Sym = VPOi_asm->import("_vpo_swap4");
    VPOi_globalDeclare(swap4Sym, 'r');
    swap8fSym = VPOi_asm->import("_vpo_swap8f");
    VPOi_globalDeclare(swap8fSym, 'r');
    swap8iSym = VPOi_asm->import("_vpo_swap8i");
    VPOi_globalDeclare(swap8iSym, 'r');
    swapargvSym = VPOi_asm->import("_vpo_swapargv");
    VPOi_globalDeclare(swapargvSym, 'r');
}

/*=============================================================================
 * FUNCTION:        ARMVPOBackend::getTempReg
 * OVERVIEW:        Returns an integer temporary register based on the given
 *                  size in bytes.
 * PARAMETERS:      cType: required type for the register.
 * RETURNS:         Rtl_ty_loc specifying the temporary register's location.
 *===========================================================================*/
Rtl_ty_loc ARMVPOBackend::getTempReg(Type cType) {
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
 * FUNCTION:        ARMVPOBackend::resetTempRegs
 * OVERVIEW:        Resets the temporary register pool.
 * PARAMETERS:      none.
 * RETURNS:         nothing.
 *===========================================================================*/
void ARMVPOBackend::resetTempRegs() {
    curr_t = t_base;
    curr_x = x_base;
    curr_y = y_base;
}
