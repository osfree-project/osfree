/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:        njexp.h
 * OVERVIEW:    This file contains the definition of the NJExpander class.
 *              This is a subclass of the Expander class, and a super class of
 *              all classes that need the NJMCTK encoding routines for
 *              code generation.
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.3 $
 * Feb 01 - Manel: Created
 */

#ifndef __NJEXP_H__
#define __NJEXP_H__

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "expander.h"
extern "C" {
#include "mclib.h"
}

/*==============================================================================
 * Global declarations.
 *============================================================================*/

// Data types
typedef map<Location,int,less<Location> >  LValIntMap;  // Mapping Location->int
typedef map<int,RAddr,less<int> > IntRAddrMap;          // Mapping int->RAddr

// NJMCTK declarations.
extern "C" void *mc_alloc(int size, Mc_alloc_pool pool);
extern "C" RClosure mc_alloc_closure
    (unsigned size, RBlock dest_block, unsigned dest_lc);

/*==============================================================================
 * NJExpander class (for using with NJMCTK encoding routines}.
 *============================================================================*/

class NJExpander: public Expander {
public:

    /**
     * NJExpander creation/destruction.
     */
    NJExpander();
    virtual	~NJExpander();

    /**
     * Object file generation for current procedure.
     */
    virtual void generateFile();

protected:

    /**
     * NJExpander protected data
     */
    int frameSize;              // Stack frame size for this function
    int procSize;               // Procedure size (in bytes).

    LValIntMap locPool;         // Location value mapping
                                //   <location value, extra_info>
    LValIntMap ofsPool;         // Offset local mapping -- r[X], vY, etc.
                                // Every entry is relative (-offset) to %fp
                                //   <location value, offset>
    IntTypeMap typPool;         // Type information
                                //   <location, type>
    IntRAddrMap relLocalAddr;   // Local relocatable addresses (targets)

    RBlock text;                // NJMCTK text relocatable block

    /**
     * NJExpander protected methods
     */
    RAddr updateLocalRAddr(PBB pTgt);
    RAddr updateGlobalRAddr(const char *sName);
    int alignd(int current, int size, int alignto = 0);

    /**
     * Additional virtual methods
     */
    virtual int buildStack() = 0;
    virtual void emitProlog() = 0;

private:

    /**
     * Code processing
     */
    void emitLabel(PBB pTgt);   // Emits a relocatable address
    void specificPreProcess();  // Preprocess function.
    void specificPostProcess(); // Postprocess function.
};

#endif
