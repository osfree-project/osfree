/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*=============================================================================
 * FILE:        ARMVPOBackend.h
 * OVERVIEW:    Translates the UQBT HRTL IR for a procedure into VPO (Very
 *              Portable Optimizer) RTLs for the ARM processor. This file is
 *              a "code expander" in VPO terminology and emits VPO RTLs and
 *              other directives for the procedure.
 *
 * (C) 2001 Sun Microsystems Inc.
 *===========================================================================*/

#ifndef _ARMVPOBACKEND_H_
#define _ARMVPOBACKEND_H_

/*
 * $Revision: 1.7 $
 *  06 Jun 01 - Brian: Initial version based on SparcIRTLToVPOBackend.h.
 *  02 Aug 01 - Brian: New class HRTL replaces RTlist.
 *  06 Sep 01 - Brian: Added support for library intrinsics (e.g. for divide).
 *                Reworked JCond code to work around VPO limitation.
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

// Include needed VPO header files. Guard the inclusion of each file to avoid
// it being included twice by the different VPO-based backends.
extern "C" {
#ifndef _VPO_RTL_H_
#define _VPO_RTL_H_
#include "vpo/rtl.h"            // VPO's rtl.h, not UQBT's rtl.h!
#endif

#ifndef _VPO_ASM_H_
#define _VPO_ASM_H_
#include "vpo/asm.h"
#endif
    
#ifndef _VPO_VPOI_H_
#define _VPO_VPOI_H_
#include "vpo/vpoi.h"
#endif
    
#ifndef _VPO_ARMVPOI_H_
#define _VPO_ARMVPOI_H_
#include "vpo/armvpoi.h"
#endif
    
#ifndef _VPO_SIMPLE_ARM_H_
#define _VPO_SIMPLE_ARM_H_
#include "vpo/simple-arm.h"
#endif
}

// Number of registers in the temporary register pool.
#define NUM_TEMP_REGS 100

class ARMVPOBackend {
public:
    // Constructor
    ARMVPOBackend(Prog& program);
    virtual ~ARMVPOBackend();
    
    // Emit VPO RTLs and other directives for a procedure
    void expandFunction(UserProc* proc);

private:    
    void        preamble();
    void        firstPass();
    void        secondPass();
    void        postamble();

    // First pass methods: scan for references and make declarations
    void        scanHRTLsInBlock(PBB bb);
    void        scanForRefsInRT(RT* rt);
    void        scanForRefs(SemStr* ss);
    void        scanForRegsUsed(SemStr* ss);
    void        scanForTempRegsUsed(SemStr* ss);
    
    void        declareCalledProc(const HLCall* call);
    void        declareParameters();
    void        declareLocalsArray();
    void        declareSymbolicVars();
    void        declareTempRegsAsVars();
    void        declareRegsAsVars();
    void        declareMachDepRegsAsVars();
    void        declareMemoryTemp();
    
    // Second pass methods: emit code for UQBT RTLs
    void        storeIncomingParams();
    void        processHRTL(const HRTL* hrtl, PBB bb);
    void        processJCondHRTL(const HRTL* jcondHrtl, PBB bb);
    void        processCallHRTL(const HRTL* callHrtl, PBB bb);
    void        processRT(RT* rt);
    
    Rtl_ty_expr processExpr(const SemStr* exp, Type cType);
    Rtl_ty_expr processMachDepReg(const SemStr* exp);
    Rtl_ty_expr processUnaryExpr(Rtl_ty_oper op,
                                 const SemStr* exp, Type cType);
    Rtl_ty_expr processBinaryExpr(const SemStr* exp, Type cType);
    Rtl_ty_expr processTernaryExpr(const SemStr* exp, Type cType);
    Rtl_ty_expr processRegOf(const SemStr* regRef);
    Rtl_ty_expr processMemOf(const SemStr* memRef);
    Rtl_ty_expr processVar(const SemStr* varRef);
    Rtl_ty_expr processLibRoutineCall(const SemStr* exp, Type cType);
    Rtl_ty_expr processConversion(const SemStr* exp, Type cType);
    Rtl_ty_loc  processIntConversion(const SemStr* exp,
                                     Type fromType, Type toType,
                                     Rtl_ty_oper rshift, int shiftCount);

    Rtl_ty_loc  emitCastIfNeeded(Rtl_ty_loc fromLoc, Type fromType,
                                 Type toType);
    Rtl_ty_oper xlateBinaryOp(INDEX idx);
    void        jumpToBBLabel(int label);
    Rtl_ty_expr getSymAddr(Rtl_ty_loc baseReg, AsmSymbol sym, int offset);
    Rtl_ty_loc  loadIntConst(int i, Type cType);
    Rtl_ty_loc  loadFPConst(double value);
    void        emitDeferredConstants();
    
    // Utility methods.
    Rtl_ty_loc  emitLoad(Rtl_ty_expr addr, Type cType, bool emitSwaps);
    void        emitByteSwaps(Rtl_ty_loc temp, int numBytes, LOC_TYPE type);
    void        emitSwap2or4Call(Rtl_ty_loc temp, int numBytes);
    void        emitSwap8Call(Rtl_ty_loc temp, LOC_TYPE type);
    void        emitArgvSwaps();
    void        passParams(const HLCall* call, VPOi_ty_locSet& param_set);
    bool        fitsIn8Bits(int iconst);
    bool        fitsIn12Bits(int iconst);
    Rtl_ty_storage regTypeForType(Type type);
    Rtl_ty_storage memoryTypeForType(Type type);
    AsmSymbol   genLabel();
    AsmSymbol   genTemp();
    char*       replaceNLs(char *s);

    void        initRegsAndLocations();
    void        declareLibraryRoutines();
    Rtl_ty_loc  getTempReg(Type cType);
    void        resetTempRegs();
    
 private:
    Prog*       prog;             // Points to program's Prog object
    BinaryFile* binFile;          // Points to program's BinaryFile object
    UserProc*   proc;             // Points to this procedure's UserProc object
    const char* procName;         // This procedure's name
    bool        doingMemorySwaps; // True if SRCENDIAN != TGTENDIAN.
    int         nextTempVarNo;    // Used to generate the names of temp vars.
    int         nextGenLabelNo;   // Used to generate the names of new labels.
    bool        atLastBB;         // True if emitting code for proc's last BB.
    ADDRESS     hrtlAddr;         // Source addr of BB's first RTL for err msgs.
    AsmSymbol   localsSym;        // "locals" array holding all local vars
    AsmSymbol   memTempSym;       // 64 bit memory temporary.
    AsmSymbol   symVarsSym;       // "symbolic vars" array holding v0, v1,...
    
    map<int, AsmSymbol> labelMap;  // Maps BasicBlock labels to VPO label syms
    map<string, AsmSymbol> procMap;// Maps import/export procs to VPO syms
    set<int>    regsUsed;          // Set of registers used 
    map<int, AsmSymbol> regVarMap; // Maps registers to their VPO syms 
    map<int, int> symVarOffsetMap; // Maps v0,v1, idx to symVarsSym array offset
    set<string> tempRegsUsed;      // Set of UQBT temp regs used 
    map<string, AsmSymbol> tempRegVarMap; // Maps UQBT temp regs to VPO syms

    set<INDEX>  machDepIdx;       // Set of machine-dependant regs like %CF, %Y
    map<string, AsmSymbol> machDepMap; // Maps names of machine-dependant regs
                                  // (e.g., %CF) to VPO syms
    map<double, AsmSymbol> defFConstMap; // Maps deferred FP constants to syms
    map<int,    AsmSymbol> defIConstMap; // Maps deferred int constants to syms

    SemStr      regRefPattern;    // A search pattern representing r[ int WILD
    SemStr      tempRefPattern;   // A search pattern representing r[ temp WILD

    Rtl_ty_loc  r[16];            // General-purpose regs: r[0]-r[15]
    Rtl_ty_loc  f[8];             // Single-precision floating-point regs
    Rtl_ty_loc  d[8];             // Double floating-point regs
    Rtl_ty_loc  fp;               // Same as r[11] - frame pointer
    Rtl_ty_loc  ip;               // Same as r[12] - holds orig sp in callee
    Rtl_ty_loc  sp;               // Same as r[13] - stack pointer
    Rtl_ty_loc  lr;               // Same as r[14] - link register (return pc)
    Rtl_ty_loc  pc;               // Same as r[15] - holds pc

    VPOi_ty_locSet callerSave_set;// Regs to save across calls
    VPOi_ty_locSet killed_set;    // Regs destroyed by calls
    VPOi_ty_locSet used_set;      // Regs read then overwritten by callee
    VPOi_ty_locSet dead_set;      // Regs not needed after call

    int         t_base;           // First VPO int temp to alloc in 2nd pass
    int         x_base;           // First VPO float temp to alloc in 2nd pass
    int         y_base;           // First VPO double temp to alloc in 2nd pass
    int         curr_t;           // Number of integer temporary regs in use
    int         curr_x;           // Number of 32 bit float temp regs in use
    int         curr_y;           // Number of 64 bit double temp regs in use
    Rtl_ty_loc  t[NUM_TEMP_REGS];
    Rtl_ty_loc  x[NUM_TEMP_REGS];
    Rtl_ty_loc  y[NUM_TEMP_REGS];

    AsmSymbol    divSym;          // VPO symbol for library routine __divsi3
    AsmSymbol    udivSym;         // VPO symbol for __udivsi3
    AsmSymbol    modSym;          // VPO symbol for __modsi3
    AsmSymbol    umodSym;         // VPO symbol for __umodsi3
    AsmSymbol    swap2Sym;        // VPO symbol for _vpo_swap2
    AsmSymbol    swap4Sym;        // VPO symbol for _vpo_swap4
    AsmSymbol    swap8fSym;       // VPO symbol for _vpo_swap8f
    AsmSymbol    swap8iSym;       // VPO symbol for _vpo_swap8i
    AsmSymbol    swapargvSym;     // VPO symbol for _vpo_swapargv
};

#endif /*_ARMVPOBACKEND_H_*/
