/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*=============================================================================
 * FILE:        SparcIRTLToVPOBackend.h
 * OVERVIEW:    Definitions for the translator that, given the IRTLs for a
 *              procedure, generates the SPARC-dependent IR used by the VPO
 *              portable optimizer. The IRTLs are intermediate level between
 *              the low-level, source machine-specific RTLs and HRTLs. They
 *              differ from RTLs by having delayed branches replaced with
 *              equivalent RTLs with simple control transfers. Other RTLs are
 *              still source machine-dependent and 1:1 with the original ones.
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
*============================================================================*/

#ifndef _SPARCIRTLTOVPOBACKEND_H_
#define _SPARCIRTLTOVPOBACKEND_H_

#include "global.h"

// include needed VPO header files
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
    
#ifndef _VPO_SPARCVPOI_H_
#define _VPO_SPARCVPOI_H_
#include "vpo/sparcvpoi.h"
#endif
}

/*
 * $Revision: 1.6 $
 * 27 Mar 2001 - Brian: expanded version of the May 1999 SPARC VPO backend
 *                written by Mike.
 *  4 May 2001 - Brian: substantial enhancements to deal with 64 bit values,
 *                floating point, recursive procedures, complex SemStrs,
 *                temporary register allocation, etc.
 *  1 Jun 2001 - Brian: substantially revised to support more IRTL constructs,
 *                leaf procedures, FP, new local variable layout, etc.
 *  6 Jun 2001 - Brian: Corrected comment at start of file.
 */

// Number of registers in the temporary register pool.
#define NUM_TEMP_REGS 100

class SparcIRTLToVPOBackend {
public:
    // Constructor
    SparcIRTLToVPOBackend(Prog& program);
    virtual ~SparcIRTLToVPOBackend();
    
    // Emit VPO RTLs and other directives for a procedure
    void expandFunction(UserProc* proc);

private:    
    void        preamble();
    void        firstPass();
    void        secondPass();
    void        postamble();

    // First pass methods: scan for references and make declarations
    void        scanRTLsInBlock(PBB bb);
    void        scanForRefsInRT(RT* rt);
    void        scanForRegsUsed(SemStr* ss);
    void        scanForTempRegsUsed(SemStr* ss);
    void        scanForLocalVarRefs(SemStr* ss, Type cType);
    void        declareCalledProc(const HLCall* call);
    void        declareLocalVars();
    void        declareMachDepRegsAsVars();
    
    // Second pass methods: emit code for UQBT Rtls
    bool        processCT(const HRTL* rtlist, PBB bb);
    void        processRT(RT* rt);
    Rtl_ty_expr processExpr(const SemStr* exp, Type cType);
    Rtl_ty_expr processMemoryRead(const SemStr* exp, Type cType);
    Rtl_ty_expr processMachDepReg(const SemStr* exp, Type cType);
    Rtl_ty_expr processUnaryExpr(Rtl_ty_oper op,
                                 const SemStr* exp, Type cType);
    Rtl_ty_expr processBinaryExpr(const SemStr* exp, Type cType);
    Rtl_ty_expr processTernaryExpr(const SemStr* exp, Type cType);
    Rtl_ty_oper xlateBinaryOp(INDEX idx);
    Rtl_ty_loc  processMemOf(const SemStr* memRef, Type cType);
    Rtl_ty_loc  processRegOf(const SemStr* regRef);
    Rtl_ty_expr processConversion(const SemStr* exp, Type cType);
    Rtl_ty_loc  processIntConversion(const SemStr* exp,
                                     Type fromType, Type toType,
                                     Rtl_ty_oper rshift, int shiftCount);

    void        jumpToBBLabel(int label);
    Rtl_ty_loc  loadLocalVarAddr(AsmSymbol sym, int offset);
    Rtl_ty_loc  loadSymAddr(AsmSymbol sym);
    Rtl_ty_loc  loadIntConst(int i, Type cType);
    Rtl_ty_rtl  sethi(Rtl_ty_expr cnst, Rtl_ty_loc rd);
    Rtl_ty_expr hi(Rtl_ty_expr cnst);
    Rtl_ty_expr lo(Rtl_ty_expr cnst);
    
    // Utility methods.
    int         getParamInfo(const HLCall* call, ADDRESS callDest,
                             int& totalParamBytes);
    Type        getFlagCallType(RTFlagCall* flagCall);
    bool        fitsInSPARCImm(int iconst);
    AsmSymbol   genLabel();
    char*       replaceNLs(char *s);

    void        initRegsAndLocations();
    Rtl_ty_loc  getTempReg(Type cType);
    void        resetTempRegs();
    
 private:
    Prog*       prog;             // Points to program's Prog object
    BinaryFile* binFile;          // Points to program's BinaryFile object
    UserProc*   theProc;          // Points to this procedure's UserProc object
    const char* procName;         // This procedure's name
    bool        doesCalls;
    bool        procHasRegWin;    // Set true if VPO uses save/restore for proc
    int         localsSize;       // Largest NN in a local var reference %fp-NN
    AsmSymbol   localsSym;        // "locals" array holding all local vars
    int         nextGenLabelNo;   // Used to generate the names of new labels.
    ADDRESS     rtlAddr;          // Source addr of BB's first RTL for err msgs
    
    map<int, AsmSymbol> labelMap; // Maps BasicBlock labels to VPO label syms
    map<string, AsmSymbol> procMap;  // Maps import/export procs to VPO syms
    set<int>    regsUsed;         // Set of registers used 
    map<string, Rtl_ty_loc> tempMap; // Maps UQBT temp regs to VPO temp regs
    set<INDEX>  machDepIdx;       // Set of machine-dependant regs like %CF, %Y
    map<string, AsmSymbol> machDepMap; // Maps names of machine-dependant regs
                                  // (e.g., %CF) to VPO syms

    SemStr      regRefPattern;    // A search pattern representing r[ int WILD
    SemStr      tempRefPattern;   // A search pattern representing r[ temp WILD
    SemStr      localPattern;     // A search pattern representing m[$fp-WILD]
    SemStr      localAddrPattern; // A search pattern representing $fp-WILD

    Rtl_ty_loc  r[32];            // General-purpose regs: {%g,%o,%l,%i}0-7
    Rtl_ty_loc  o[8];             // %o regs for proc calls; same as r[8-15]
    Rtl_ty_loc  f[32];            // Single-precision floating-point regs
    Rtl_ty_loc  d[32];            // Double floating-point regs %d0,%d2,%d4,...

    VPOi_ty_locSet callerSave_set;// Regs to save across calls
    VPOi_ty_locSet killed_set;    // Regs destroyed by calls
    VPOi_ty_locSet used_set;      // Regs read then overwritten by callee

    int         t_base;           // First VPO int temp to alloc in 2nd pass
    int         x_base;           // First VPO float temp to alloc in 2nd pass
    int         y_base;           // First VPO double temp to alloc in 2nd pass
    int         curr_t;           // Number of integer temporary regs in use
    int         curr_x;           // Number of 32 bit float temp regs in use
    int         curr_y;           // Number of 64 bit double temp regs in use
    Rtl_ty_loc  t[NUM_TEMP_REGS];
    Rtl_ty_loc  x[NUM_TEMP_REGS];
    Rtl_ty_loc  y[NUM_TEMP_REGS];
};

#endif /*_SPARCIRTLTOVPOBACKEND_H_*/
