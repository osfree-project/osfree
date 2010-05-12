/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:        sparcnjexp.h
 * OVERVIEW:    This file contains the definition of the SparcNJExpander class.
 *              This is a subclass of the NJExpander class retargeted to
 *              Sparc v8 code generation
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.4 $
 * Feb 01 - Manel: Created
 */

#ifndef __SPARCNJEXP_H__
#define __SPARCNJEXP_H__

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "njexp.h"

/*==============================================================================
 * Sparc NJExpander class (for using with NJMCTK encoding routines}.
 *============================================================================*/

class SparcNJExpander: public NJExpander {
public:

    /**
     * SparcNJExpander creation/destruction.
     */
    SparcNJExpander();
    virtual	~SparcNJExpander();

private:

    /**
     * Private data.
     */
    int nextFreeLoc;            // Next free local register (l0..l7).
    int nextParamLoc;           // Next free param register (o0..o5).

    /**
     * Location value management.
     * Identifiers 'point' to machine-dependent locations.
     */
    void clearLocations();
    Location getLocation(const SemStr* pExp, Type cType);
    Location fetch(Location loc, bool isParam = false);
    Location getNewLocation(Location stgType, int idx = 0);
    int getParamNo(const Proc* proc, const SemStr *ss);

    /**
     * Code emitting functions.
     */
    int buildStack();
    void emitLoad(Location dst, Location base, int ofst, Type lType);
    void emitStore(Location base, int ofst, Location src, Type lType);
    void emitSwap(Location dst, Type lType);

    void emitUnary(Location dst, Location src);
    void emitBinary(Location dst, Location src1, Location src2);
    void emitConversion(Location dst, Location src, int from, int to);
    void emitAssign(Location dst, Location src);

    void emitProlog();
    void emitBranch(PBB pTgt);
    void emitCondBranch(Location loc, PBB pTgt);
    void emitCase(Location loc, int val, PBB pTgt);
    void emitCall(Location dst, Proc* pTgt, const vector<Location>& param);
    void emitReturn(Location loc);
};

#endif
