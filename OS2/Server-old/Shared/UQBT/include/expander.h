/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:        expander.h
 * OVERVIEW:    This file contains the definition of the abstract class
 *              Expander, which provides a machine independent interface
 *              for code generation.
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.5 $
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 */

#ifndef __EXPANDER_H__
#define __EXPANDER_H__

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "global.h"
#include "ss.h"
#include "type.h"
#include "proc.h"
#include "overlap.h"

/*==============================================================================
 * Global data type declarations.
 *============================================================================*/

typedef int Location;                           // Location value
typedef map<int,Type,less<int> > IntTypeMap;    // Mapping #reg->Type

/*==============================================================================
 * Expander class.
 *============================================================================*/

class Expander {
public:

    /**
     * Expander creation/destruction.
     */
    Expander();
    virtual ~Expander();

    /*
     * Return an instance of the appropriate expander subclass,
     * given an identifier for the subclass.
     */
    static Expander* getExpInstance(char type);

    /**
     * Get generated code.
     */
    void*       getCode()       { return rawCode; }     // Raw data
    int         getCodeSize()   { return rawSize; }     // Code size (bytes)
    RelocMap&   getRelocTab()   { return globReloc; }   // Mapp ofst->sym

    /**
     * Expansion for a procedure.
     */
    void expandFunction(UserProc *proc);    // Code expansion.
    virtual void generateFile() = 0;        // File generation for current proc.

    /**
     * Error management.
     */
    void expError(char *messg);     // Dump error message
    void expHalt(char *messg);      // Dump error message and exit

protected:

    /**
     * Expander protected data
     */
    UserProc*   proc;       // Pointer to this procedure's Proc object.
    ADDRESS     rtlAddr;    // Source address of this RTL (for err msgs).
    set<INDEX>  stIdx;      // Set of machine specific indices like %CF, %Y.
    IntTypeMap  regUsed;    // Set of source registers used.
    IntTypeMap  symUsed;    // Set symbolic registers used.
    Overlap     ovl;        // Overlap register management.

    /**
     * Generated code.
     */
    void*       rawCode;    // Stream of bytes.
    int         rawSize;    // Size of code stream.
    RelocMap    globReloc;  // Global rellocation information.

    /**
     * Location value management.
     * Identifiers 'point' to machine-dependent locations.
     */
#define INVALID_LOCATION (-1)
    virtual void clearLocations() = 0;
    virtual Location getLocation(const SemStr* pExp, Type cType) = 0;
    virtual Location fetch(Location loc, bool isParam = false) = 0;

    /**
     * Code emitting functions.
     */
    virtual void emitUnary(Location dst, Location src) = 0;
    virtual void emitBinary(Location dst, Location src1, Location src2) = 0;
    virtual void emitConversion(Location dst, Location src, int from, int to) = 0;
    virtual void emitAssign(Location dst, Location src) = 0;

    virtual void emitLabel(PBB pTgt) = 0;
    virtual void emitBranch(PBB pTgt) = 0;
    virtual void emitCondBranch(Location loc, PBB pTgt) = 0;
    virtual void emitCase(Location loc, int val, PBB pTgt) = 0;
    virtual void emitCall(Location dst, Proc* pTgt, const vector<Location>& param) = 0;
    virtual void emitReturn(Location loc) = 0;

    /**
     * Code processing.
     */
    virtual void specificPreProcess() = 0;      // Preprocess function.
    virtual void specificPostProcess() = 0;     // Postprocess function.

private:

    /**
     * Private data.
     */
    SemStr regRefPattern;   // A constant SemStr, representing r[ int -1
    SemStr varRefPattern;   // A constant SemStr, representing v[ -1
    set<int> regNumbers;    // Set of source registers used.

    /**
     * Code processing
     */
    void findUsedVariables(SemStr* ss);     // Check for used symb vars/regs.

    Location processExpr(const SemStr* pExp, Type cType);
    void processRT(RT* pRt);
    void processRTL(const HRTL* pRTL);
    void processHL(const HRTL* pRTL, PBB pBB);

    void PreProcess();                      // Preprocess function
    void PostProcess();                     // Postprocess function
};

#endif
