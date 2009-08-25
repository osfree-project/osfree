/*==============================================================================
 * FILE:        overlap.cc
 * OVERVIEW:    Implementation of the Overlap class, which manages the relat-
 *               ionship between overlapped registers, e.g. %f0 and %f0to1
 *
 * Copyright (C) 2001, The University of Queensland, BT group
 *============================================================================*/

/*
 * $Revision: 1.4 $
 * 29 Jan 01 - Mike: Initial version
 * 13 Feb 01 - Mike: Removed debugging
 * 27 Mar 01 - Mike: Added code for SHARES registers (e.g. Pentium)
 */

#include "global.h"
#include "overlap.h"
#include "prog.h"
#include "backend.h"        // For makeReg(), etc


/*==============================================================================
 * FUNCTION:        Overlap::Overlap
 * OVERVIEW:        Constructor
 * PARAMETERS:      N/a
 * RETURNS:         N/a
 *============================================================================*/
Overlap::Overlap()
{}

/*==============================================================================
 * FUNCTION:        Overlap::init
 * OVERVIEW:        Initialise. Reads information summarised from the SSL file.
 * PARAMETERS:      None
 * RETURNS:         Nothing
 *============================================================================*/
void Overlap::init()
{
    int i;
    childOf.clear();
    parentOf.clear();
    map<int, Register>::const_iterator it;
    for (it = prog.RTLDict.DetRegMap.begin();
      it != prog.RTLDict.DetRegMap.end(); it++) {
        // *it is a pair; it->first is the register number; it->second is the
        // Register (detailed register info) struct
        const Register& reg = it->second; //prog.RTLDict.DetRegMap[it->first];
        int ch = reg.g_mappedIndex();
        if (ch == -1) continue;
        Register& reg2 = prog.RTLDict.DetRegMap[ch];
        // Find out if this is a "COVERS" or a "SHARES" relationship
        bool covers = reg.g_size() > reg2.g_size();
        if (covers) {
            // Find the child registers immediately below this register.
            // For example, we might be processing 80 covers 32-35, when we
            // already have 64 covers 32-33, 65 covers 34-35; in this case, we
            // want to make an entry for 80 covers 64-65 (not 80 covers 32).
            int parSize = reg.g_size();
            int chSize = reg2.g_size();
            do {
                if (parentOf.find(ch) == parentOf.end()) break;
                ch = parentOf[ch];
                chSize = prog.RTLDict.DetRegMap[ch].g_size();
            } while (true);
            // Insert all the children
            // Referencing this unmapped register will insert a new, empty
            // entry
            LevelInfo& li = childOf[it->first];
            li.size = chSize;
            int numReg = parSize / chSize;
            #if (SRCENDIAN == TGTENDIAN)
                // Endianness is the same
                for (i=0; i < numReg; i++) {
                    parentOf[ch] = it->first;
                    li.regs.push_back(ch++);
                }
            #else
                // Endianness is different
                for (i=numReg-1; i >= 0; i--) {
                    parentOf[ch+i] = it->first;
                    li.regs.push_back(ch+i);
                }
            #endif
        }
        else {
            // a SHARES relationship
            int par = ch;           // For SHARES, the mappedIndex is the parent
            parentOf[it->first] = par;
            map<int, LevelInfo>::iterator cc = childOf.find(par);
            LevelInfo& li = childOf[par];
            int chSize = reg.g_size();
            int parSize = reg2.g_size();
            int pos = reg.g_mappedOffset();
            if (cc == childOf.end()) {
                // We have no childOf entry for this level, so li will be a
                // newly inserted, empty LevelInfo
                li.size = chSize;
                int numReg = parSize / chSize;
                // We must create numReg entries regardless; some of these will
                // be index -1 (for dummy registers). For example, on a big
                // endian target machine, the children of %eax will be -1 and
                // %ax (in that order).
                #if TGTENDIAN == LITTLEE
                    int cPos = 0;           // Current lower bit position
                    for (int i=0; i < numReg; i++) {
                        // If cPos == pos, then we want this real register
                        // else -1 for a dummy
                        int r = (cPos == pos) ? it->first : -1;
                        li.regs.push_back(r);
                        cPos += chSize;
                    }
                #else   // Target is big endian
                    int cPos = parSize - chSize;
                    for (int i=numReg; i > 0; i--) {
                        int r = (cPos == pos) ? it->first : -1;
                        li.regs.push_back(r);
                        cPos -= chSize;
                    }
                #endif
            }
            else {
                // We already have a childOf entry. We want to replace one of
                // the dummy (-1) entries with the current register (it->first)
                #if TGTENDIAN == LITTLEE
                    int cPos = 0;           // Current lower bit position
                    list<int>::iterator ll = li.regs.begin();
                    for (; ll != li.regs.end(); ll++) {
                        if (cPos == pos) {
                            *ll = it->first;
                            break;
                        }
                        cPos += chSize;
                    }
                #else   // Target is big endian
                    int cPos = parSize - chSize;
                    list<int>::iterator ll = li.regs.begin();
                    for (; ll != li.regs.end(); ll++) {
                        if (cPos == pos) {
                            *ll = it->first;
                            break;
                        }
                        cPos -= chSize;
                    }
                #endif
            }
        }
    }
}

/*==============================================================================
 * FUNCTION:        Overlap::setUsed
 * OVERVIEW:        Build information based in what registers are used this proc
 * PARAMETERS:      regNumbers: a set of used registers
 * RETURNS:         Nothing
 *============================================================================*/
/*
 * checkChildren. Needed to handle recursion into the childOf map
 */
void Overlap::checkChildren(int reg, const set<int>& regNumbers, int& minSize,
    int& first)
{
    if (childOf.find(reg) == childOf.end())
        return;
    LevelInfo& li = childOf[reg];
    list<int>::const_iterator ll;
    for (ll=li.regs.begin(); ll != li.regs.end(); ll++) {
        if (*ll == -1) continue;        // Ignore the dummy registers
        if (regNumbers.count(*ll)) {
            minSize = prog.RTLDict.DetRegMap[*ll].g_size();
            first = li.regs.front();
        }
        // Now recurse
        checkChildren(*ll, regNumbers, minSize, first);
    }
}

void Overlap::setUsed(const set<int>& regNumbers)
{
    set<int>::const_iterator it;
    for (it = regNumbers.begin(); it != regNumbers.end(); it++) {
        int r = *it;
        int max = r;
        // Note that while max ranges from smallest to largest register in the
        // SSL file, top is the largest register actually used this proc
        int top = r;
        int minSize = prog.RTLDict.DetRegMap[r].g_size();
        int maxSize = minSize;
        int first = -1;
        // Check the child chain
        checkChildren(r, regNumbers, minSize, first);

        // Check the parent chain
        while (parentOf.find(max) != parentOf.end()) {
            max = parentOf[max];
            if (regNumbers.count(max)) {
                maxSize = prog.RTLDict.DetRegMap[max].g_size();
                top = max;
                if (first == -1)
                    // This means that we have a low level register (none of
                    // its children (if any) are used. Here we are at the
                    // parent of such a register; we can update the "first"
                    // entry by looking at the first child of this parent
                    first = childOf[max].regs.front();
            }
        }

        if (minSize == maxSize)
            // This register doesn't overlap with any others
            continue;
        RegInfo ri;
        ri.minSize = minSize;
        ri.maxSize = maxSize;
        ri.first = first;
        ri.top = top;
        regMap[r] = ri;
    }

#if 0
map<int, RegInfo>::iterator zz = regMap.begin();
while (zz != regMap.end()) {
cout << "RegInfo[ " << zz->first << "] ";
cout << "  minSize " << zz->second.minSize;
cout << ", maxSize " << zz->second.maxSize;
cout << ", first " << zz->second.first;
cout << ", top " << zz->second.top;
cout << endl;
zz++;}
#endif
}

/*==============================================================================
 * FUNCTION:        getSizeLetter
 * OVERVIEW:        Get the appropriate letter for a given size, e.g. 32 bits
 *                    and isFloat means "f". This letter is used to select
 *                    the appropriate union at the top level
 * PARAMETERS:      size: size of the current register in bits
 *                  isFloat: if true, give floating point variants
 * RETURNS:         A char as above
 *============================================================================*/
char getSizeLetter(int size, bool isFloat)
{
    if (isFloat) {
        switch (size) {
            case 32: return 'f';
            case 64: return 'd';
            default: return 'q';
        }
    }
    else {
        switch (size) {
            case 8:  return 'b';        // byte
            case 16: return 'h';        // half
            case 32: return 'i';
            case 64: return 'l';
            default: return 'L';        // "very Long"
        }
    }
}

/*==============================================================================
 * FUNCTION:        Overlap::getRegExpr
 * OVERVIEW:        Get the appropriate C expression for this register
 * PARAMETERS:      regNo: the register number (e.g. 32 for sparc %f0)
 * RETURNS:         The C expression, e.g. "_r32to33.f.r32"
 *============================================================================*/
string Overlap::getRegExpr(int regNo)
{
	// Check if it's an overlapped register
	if (regMap.find(regNo) == regMap.end()) {
		return makeReg(regNo);
	}

    Register& reg = prog.RTLDict.DetRegMap[regNo];
    ostrstream ret;
    // Name the outer register
    ret << getSizeLetter(regMap[regNo].maxSize, reg.isFloat()) <<
        regMap[regNo].top;
    int size = prog.RTLDict.DetRegMap[regNo].g_size();
    // If this is a top level register, don't put in the struct name
    // (e.g. d64.f.f32 but d64.d64)
    if (size != regMap[regNo].maxSize)
        ret << "." << getSizeLetter(size, reg.isFloat());
    ret << "." << getSizeLetter(size, reg.isFloat()) << regNo;
	return string(str(ret));
}

/*==============================================================================
 * FUNCTION:        Overlap::declareRegs
 * OVERVIEW:        Declare the overlapped registers
 * PARAMETERS:      of: ofstream to emit to
 * RETURNS:         none
 *============================================================================*/

    // "Local" function to emit one level of structs
    // ints is the list of register numbers (-1 for dummies)
    void emitLevel(ofstream& of, list<int>& ints, int& dummyNum, bool isFloat,
        int size)
    {
        of << "\tstruct {\n";
        list<int>::iterator ll;
        for (ll=ints.begin(); ll != ints.end(); ll++) {
            // Examples: \t\tfloat32 f34;  \t\tint16 dummy9;
            of << "\t\t" << (isFloat ? "float" : "int") << size;
            if (*ll != -1)
                of << " " << getSizeLetter(size, isFloat) << *ll <<
                    ";\n";
            else
                of << " dummy" << ++dummyNum << ";\n";
        }
        // Close this struct
        of << "\t} " << getSizeLetter(size, isFloat) << ";\n";
    }

void Overlap::declareRegs(ofstream& of)
{
    // Find the "top level used" registers
    map<int, RegInfo>::const_iterator it;
    int dummyNum = 0;
    for (it = regMap.begin(); it != regMap.end(); it++) {
        // *it is a pair: first is the register number (e.g. 33 for Sparc %f1),
        // second is the RegInfo for this register
        int r = it->first;
        int size = prog.RTLDict.DetRegMap[r].g_size();
        if (size != it->second.maxSize)
            // This is not a top level register; ignore
            continue;
        of << "union {\n\t";
        bool isFloat = prog.RTLDict.DetRegMap[r].isFloat();
        Type t(isFloat ? FLOATP : INTEGER, size, true);
        of << t.getCtype() << " " << getSizeLetter(size, isFloat);
        of << it->first << ";\n";
        // Now do all the structs that this is overlapped with
        list<int> parRegNums, childRegNums;
        parRegNums.push_back(r);
        while (1) {
            // Find all the children of this level
            childRegNums.clear();
            LevelInfo* li = 0;
            list<int>::iterator itPar;
            for (itPar = parRegNums.begin(); itPar != parRegNums.end(); itPar++)
            {
                if (*itPar == -1) {
                    // We need a child of a dummy, for example:
                    //        eax
                    //   dummy1  ax
                    //  d2  d3  ah al
                    // so dummy1 needs two half sized dummys, d2 and d3
                    // For now, since we don't have size info for dummys,
                    // we assume that 2 dummys is right
                    childRegNums.push_back(-1);
                    childRegNums.push_back(-1);
                    continue;
                }
                if (childOf.find(*itPar) != childOf.end()) {
                    li = &childOf[*itPar];
                    list<int>::const_iterator ll;
                    for (ll = li->regs.begin(); ll != li->regs.end(); ll++) {
                        childRegNums.push_back(*ll);
                    }
                }
            }
            if (li == 0)
                break;
            emitLevel(of, childRegNums, dummyNum, isFloat, li->size);

            // Now go down a level; this level's children are the next level's
            // parents
            parRegNums = childRegNums;
        }

        // Now close the union and declare the name
        of << "} " << getSizeLetter(size, isFloat) << it->first << ";\n";
    }
}

/*==============================================================================
 * FUNCTION:        Overlap::isOverlappedReg
 * OVERVIEW:        Return true if the given register is involved in overlapping
 *                    i.e. if it has already been declared in declareRegs())
 * PARAMETERS:      reg: the register number
 * RETURNS:         True if overlapped, as above
 *============================================================================*/
bool Overlap::isOverlappedReg(int reg)
{
    return regMap.find(reg) != regMap.end();
}

/*==============================================================================
 * FUNCTION:        Overlap::getRegMap
 * OVERVIEW:        Get the whole map for used registers
 * PARAMETERS:      <none>
 * RETURNS:         Map for used registers
 *============================================================================*/
map<int, RegInfo>& Overlap::getRegMap()
{
    return regMap;
}

/*==============================================================================
 * FUNCTION:        Overlap::isChildOf
 * OVERVIEW:        Return true if there are children for the given register
 * PARAMETERS:      reg: the register number
 * RETURNS:         True if there are children
 *============================================================================*/
bool Overlap::isChildOf(int reg)
{
    return childOf.find(reg) != childOf.end();
}

/*==============================================================================
 * FUNCTION:        Overlap::getChildOf
 * OVERVIEW:        Get the children information for a given register
 * PARAMETERS:      reg: the register number
 * RETURNS:         The corresponding LevelInfo
 *============================================================================*/
LevelInfo* Overlap::getChildOf(int reg)
{
    return &childOf[reg];
}

