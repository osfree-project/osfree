/*==============================================================================
 * File:       overlap.h
 * OVERVIEW:   Declaration of the Overlap class. This class manages the relat-
 *             ionship between overlapped registers, e.g. %f0 and %f0to1
 *
 * Copyright (C) 2001, The University of Queensland, BT group
 *============================================================================*/
/*
 * $Revision: 1.5 $
 *
 * 23 Jan 01 - Mike: Created
 * 27 Mar 01 - Mike: Updated some comments
 */

#ifndef _OVERLAP_H_
#define _OVERLAP_H_


/*
 * A structure to represent the children registers of a given register.
 * These children are those statically immediately below the given register
 * (considering the register with the most bits at the top)
 */
struct LevelInfo {
    /*
     * Size in bits of registers at this level (i.e. the children)
     */
    int size;

    /*
     * List of register numbers for the children. Order is important; they
     * will be declared in the forward order of this list
     */
    list<int> regs;
};

/*
 * Information about each used register
 */
struct RegInfo {
    /*
     * Minimum size. This is a dynamic quantity (based on what register numbers
     * are used in a procedure); it's the smallest size (in bits) of registers
     * that overlap with this register.
     * Example: current register is 1; registers in use are 33, 64, 65 (sparc);
     * minSize is 32
     */
    int     minSize;

    /*
     * Maximum size. This is a dynamic quantity (based on what register numbers
     * are used in a procedure); it's the largest size (in bits) of registers
     * that overlap with this register.
     * Example: current register is 1; registers in use are 33, 64, 65 (sparc);
     * maxSize is 64
     */
    int     maxSize;

    /*
     * First register at the lowest level (needed to declare the components
     * of the lower levels)
     * MVE: At this stage, this appears to be unused!
     */
    int     first;

    /*
     * Register number of the top level, e.g. d65.d.f33 would be 65)
     */
    int     top;

};

class Overlap {
    /*
     * A map from register number to register number of immediate parent.
     * Static information: depends only on SSL file
     */
    map<int, int> parentOf;

    /*
     * A map from register number to LevelInfo, representing the children.
     */
    map<int, LevelInfo> childOf;

    /*
     * A map for each used register.
     */
    map<int, RegInfo>   regMap;

    /*
     * A private function needed just to handle the recursion
     */
    void checkChildren(int reg, const set<int>& regNumbers, int& minSize,
        int& first);

public:
    /*
     * Default constructor
     */
    Overlap();

    /*
     * Initialise. Reads information summarised from the SSL file.
     */
    void init();

    /*
     * Build information based in what registers are used this proc
     * regNumbers: a set of used registers
     */
    void setUsed(const set<int>& regNumbers);

    /*
     * Get a string for this register.
     * regNo: the register number (e.g. 32 for sparc %f0)
     * Returns the C expression, e.g. "_r32to33.f.r32"
     */
    string getRegExpr(int regNo);

    /*
     * Declare the overlapped registers
     * of: ofstream to emit to
     */
    void declareRegs(ofstream& of);

    /*
     * Return true if the given register is involved in overlaps (and hence
     * has been declared as part of declareRegs())
     */
    bool isOverlappedReg(int reg);

    /*
     * Get the whole map for used registers
     */
    map<int, RegInfo>& getRegMap();

    /*
     * Return true if there are children for the given register
     */
    bool isChildOf(int reg);

    /*
     * Get the children information for a given register
     */
    LevelInfo* getChildOf(int reg);
};

#endif      // _OVERLAP_H_
