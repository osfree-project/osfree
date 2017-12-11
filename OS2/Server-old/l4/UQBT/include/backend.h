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
 * FILE:        backend.h
 * OVERVIEW:    This file contains definitions concerning the translating of
 *              machine independent RTLs to low level C
 *
 * Copyright (C) 1999-2001, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

#ifndef _BACKEND_H_
#define _BACKEND_H_

#include "overlap.h"            // For class Overlap (embedded object)

class Proc;
class BinaryFile;
class OutputFile;

/*
 * $Revision: 1.36 $
 * 25 Jan 00 - Mike: Added makeReg() (used by PROC/proc.cc)
 * 28 Mar 00 - Mike: Made into a class; added detection of strings for pc
 *                      relative addresses that make it to the back end
 * 05 Sep 00 - Mike: Moved getCodeInfo to util.cc
 * 19 Sep 00 - Mike: proc_backend -> addSourceFile; removed last vpo code
 * 07 Nov 00 - Mike: Added proc member; emitCast()
 * 29 Nov 00 - Mike: Added checkMachSpec()
 * 06 Feb 01 - Mike: Merged checkMachSpec with firstPass (which is called much
 *                    earlier now); added useSwap8, stIdx, Overlap object
 * 23 Feb 01 - Mike: Added varX (needed for a pass that checks sizes of vars)
 * 22 Mar 01 - Mike: castParameter returns a bool now
 * 16 Apr 01 - Brian: Added addVPOSourceFile() method.
 * 24 Apr 01 - Mike: Support for %SKIP and %RPT implementation
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 */

class Translate {
    BinaryFile* pBF;            // Pointer to BinaryFile object for source
    CSR* pCsrSrc;               // Pointer to the CSR object for the src mach
    ostrstream os;              // String stream for the code (so can declare
                                // the locals)
                                    
    ofstream of;                // The output (.c) file stream
    set<int> regNumbers;        // Set of register numbers used (real source
                                // register machines)
    int maxVar;                 // Max var used this function
    set<string> temps;          // Set of temp variable names
    set<string> protos;         // Set of proc names to prototype
    bool useSwap2;              // True if use the _swap2() func
    bool useSwap4;              // True if use the _swap4() func
    bool useSwap8;              // True if use the _swap8() func
    ADDRESS uAddr;              // Native address of this RTL (for err msgs)
    UserProc* proc;             // Pointer to the UserProc object for this proc
    set<INDEX> stIdx;           // Set of machine specific indices like %CF, %Y
    SemStr regX;                // A constant SemStr, representing r[ int -1
    SemStr varX;                // A constant SemStr, representing v[ -1
    Overlap ovl;                // Manages the overlapped registers
    int labelNum;               // Last used label number, for repeated instrs
    int idSKIP;                 // Index for the %SKIP entry (or -1 if none)
    int idRPT;                  // Index for the %RPT entry (if any)

    string processRegOf(const SemStr* pLoc, Type cType);
    string processVar(const SemStr* pLoc, Type cType);
    string processMemOfRaw(const SemStr* pLoc, Type cType);
    string processMemOf(const SemStr* pLoc, Type cType);
    string xlateUnaryOp(int idx);
    string xlateBinaryOp(int idx);
    string processIntConst(const SemStr* pLoc, Type cType);
    string processCodeAddr(const SemStr* pLoc);
    string processExpr(const SemStr* pLoc, Type cType);
    bool   processRT(RT* pRt);
    void   emitFunc(UserProc* proc, bool spec);
    bool   processHL(const HRTL* pRTL, PBB pBB, Proc* proc); 
    bool   isString(const char* p, const char* last);
    void   StructuredEmit(UserProc *proc);
    void   ProcessBB(PBB pbb,UserProc *proc,int nest=0,bool followok=false);
    void   ProcessBB(UserProc *proc,PBB pbb,PBB stopon,PBB breakto,PBB continueto,bool ignoreheader=false);
    void   EmitBBCond(UserProc*proc,PBB pbb, bool negate);
    // Emit the code for a whole BB:
    void   emitBBCode(PBB pBB,UserProc* proc,int nest=0,bool allJumps = true);
    const char* swapStart(const Type& t); // Emit the start of a swap macro call
    const char* swapEnd(const Type& t);   // Emit the end of a swap macro call
    void   emitReturn(Proc* proc);        // Emit "return <retlocn>;\n"
    bool   castParameter(Proc* proc, const vector<SemStr>& params, int i);
    bool   emitCast(const Type& cType, const Type& eType, ostrstream& ost);
    void   firstPass();                     // Count registers used, and check
                                            // for machine specific IDs
    void   checkRegs(SemStr* ss);           // Check this SemStr for r[X]

public:
    /*
     * Constructor.
     */
    Translate();
    /*
     * translate2c
     *   This is the main back end function for a given procedure.
     * Parameters:
     *  - proc: Proc object for this procedure
     *  - spec: if true, then this is a speculative translation
     */
    void translate2c(UserProc* proc, bool spec = false);
};

/*
 * addSourceFile
 *  Adds the source file for the given proc to the OutputFile object's list
 * Parameters:
 *  proc: Proc object for this procedure
 *  of: reference to the OutputFile object
 */
void addSourceFile(Proc* proc, OutputFile& of);

/*
 * addVPOSourceFile
 *  Adds the VPO file for the given proc to the OutputFile object's list
 * Parameters:
 *  proc: Proc object for this procedure
 *  of: reference to the OutputFile object
 */
void addVPOSourceFile(Proc* proc, OutputFile& of);

/*
 * Generate the final output file: create all the data files,
 * assemble and link all the assembler files, and adjust the
 *  final output file to add the data, interpreter, etc
 * Parameters:  of: reference to the OutputFile object that manages the temp-
 *                  orary files (data files, assembler files etc)
 */
void finalOutput(OutputFile& of);

/* Convert a name to a legal C name */
char* legalCname(const char* labelName);

/*==============================================================================
 * FUNCTION:        makeReg
 * OVERVIEW:        Make a string with a C representation of a register; e.g.
 *                    given 12, return "r12", but given 64 it would return "d64"
 * ASSUMPTIONS:     Assumes that the global pRtlDict is set up (which will be
 *                    true after the first call to translate()
 * PARAMETERS:      regNum: register number
 * RETURNS:         string representing the register
 *============================================================================*/
string makeReg(int regNum);

#endif
