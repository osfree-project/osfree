/*
 * Copyright (C) 1999-2001, The University of Queensland
 * Copyright (C) 1999-2000, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:        translate2c.cc
 * OVERVIEW:    This module translates the machine independent RTLs for a
 *              procedure to a simple dialect of C
 *
 * Copyright (C) 1999-2001, The University of Queensland, BT group
 * Copyright (C) 1999-2000, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.139 $
 * 06 May 99 - Mike: Initial version
 * 12 May 99 - Mike: attempt stack parameters
 * 13 May 99 - Mike: start translation proper. Main idea is that for each
 *          subexpression, we emit an assignment to a temp register, and
 *          return an expression for that temp.
 * 13 May 99 - Mike: merged backsparc.cc and back386.cc to translate.cc
 * 25 May 99 - Mike: handle locations now (parameters and local variables)
 * 31 May 99 - Mike: First attempt at swap code for endianness
 * 01 Jun 99 - Mike: First attempt at generating C
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 03 Jun 99 - Mike: processRegOf() handles idTemp and r[i+j] now
 * 03 Jun 99 - Mike: added byte swapping code
 * 17 Jun 99 - Mike: Added FLAGS
 * 23 Jun 99 - Mike: Removed flags; replaced with use of pCond in HLJcond
 * 29 Jun 99 - Mike: Handle labels as integers
 * 30 Jun 99 - Mike: changes to handle HLSconds
 * 06 Jul 99 - Mike: changes to handle sizes
 * 16 Jul 99 - Mike: added function parameters and return types
 * 21 Jul 99 - Mike: added support for Itof()
 * 23 Jul 99 - Mike: was not processing _swaps for int*, only unsigned*
 * 28 Jul 99 - Doug: declarations of locals block and afp done by Proc
 *                   now
 * 03 Aug 99 - Doug: AFP expression now generates `_locals' as opposed
 *                   to `_afp' as the latter is simply an alias to the
 *                   first and just makes the generated code more
 *                   confusing.
 * 21 Sep 99 - Mike: Fixed "call to unnamed function" problem; also float ops
 * 29 Sep 99 - Mike: Added -a (source addresses each label) option.
 *                  Also fixed unary expressions (had unwanted ";\n")
 * 18 Nov 99 - Mike: Force main to be "void main(int v0, int v1)"
 * 18 Nov 99 - Mike: -a means NO addresses now (too useful!)
 * 01 Dec 99 - Mike: added RETASGN to calls that don't (yet) return a value,
 *                    so we can change what the function returns easily
 * 09 Dec 99 - CC: main now has the signature "int main(int, int)"
 * 21 Dec 99 - CC: force #define's for main to assume return of v0 
 * 18 Jan 00 - Mike: Implement idFltConst; pass "floatedness" in variable flt
 *                  and handle casing at the operand level, rather than either
 *                  side of the operator
 * 17 Mar 00 - Cristina: added idAGP support
 *                  Added _globals[] definition in emitFunc()   
 * 21 Mar 00 - Cristina: added MC68K-specific #include's in emitFunc().
 *                  Removed _globals[] from emitFunc, now supported in uqbt.h. 
 * 22 Mar 00 - Mike: Don't emit RETASGN for library functions. Also allow
 *                  expressions in actual parameters
 * 28 Mar 00 - Mike: Moved globals into the Translate class. Also detect
 *                  string constants in the code segment
 * 30 Mar 00 - Mike: Use pseudo types (Int, Double, etc)
 * 04 Apr 00 - Mike: Uses Type class now (replaces size and flt)
 * 06 Apr 00 - Mike: Decode procedure if discover pointer to function
 * 18 Apr 00 - Mike: Quick hack to set the return location for all callback
 *                    functions (discovered via idCodeAddr) to return int8
 * 20 Apr 00 - Mike: Another change to the cast logic; now emits a warning if
 *                    current type != "to" type, and always casts to "to" type
 * 28 Apr 00 - Mike: Fixed "type conflict" error message; added uAddr as well
 *                    Also respect register type when destination of assignments
 * 19 May 00 - Mike: Error message if HLJcond has no HL condition
 * 24 May 00 - Mike: Synthetic outedge for FALL BBs handled now
 * 04 Jul 00 - Mike: Fixed memory stores for endianness swaps
 * 05 Jul 00 - Mike: Added code to support %pc as the native address of the
 *                    rtl
 * 14 Jul 00 - Mike: Allow forcing of the name of a HLCall dest; fixed idNot
 * 18 Jul 00 - Mike: Fixed incorrect logic associated with the last change;
 *                    was generating call to (null) after register calls
 * 19 Jul 00 - Mike: Removed RETLOCN code; generate all code at the end now
 * 20 Jul 00 - Mike: SRA is signed; SRL is unsigned; fixed casts
 * 25 Jul 00 - Mike: added legalCname()
 * 25 Aug 00 - Mike: Fixed a segfault when translating a proc with no BBs at all
 *                    (can happen when using -S)
 * 30 Aug 00 - Mike: Imply "from" types properly (generates correct code for
 *                    Sparc LDUB/LDSB now)
 * 05 Sep 00 - Mike: Moved getCodeInfo from here to util.cc
 * 05 Sep 00 - Mike: tmpf and tmpd are float32 and float64 respectively now
 * 11 Sep 00 - Mike: Declare temporaries for _swapX macros if needed
 * 19 Sep 00 - Mike: "Back end" message in translate2c() now
 * 21 Sep 00 - Mike: No need to declare _reg for pentium swap macros now
 * 25 Sep 00 - Mike: Worked around the sparc 7th parameter float problem with
 *              new procedure Translate::castParameter()
 * 27 Sep 00 - Mike: Was not using parameter last from getCodeInfo correctly
                Also output _swap8f() where needed
 * 29 Sep 00 - Mike: Fixed a problem with castParameter() when using -S;
 *              also implemented idTern expressions as ?:
 *              Changed logic to not rely on size of floating point operators
 * 25 Oct 00 - Mike: Initial type of an RTAssgn now int same size as the
 *              assignment, unless LHS is a register; use getRegType to
 *              choose approp initial broad type. Added emitCast()
 * 17 Nov 00 - Mike: Changed getRegType to cope with vars as well as numbered
 *              registers (takes a UserProc* parameter now)
 * 29 Nov 00 - Mike: Special pass to determine if and when things like %Y and
 *              %CF need to be declared as locals; handles %Y in expressions
 * 30 Nov 00 - Mike: idTrunc -> idTruncu, idTruncs
 * 30 Nov 00 - Mike: Fixed processing of a@[b:c] (where b != c)
 * 06 Dec 00 - Mike: Added check for _locals+k where k >= size of _locals
 * 06 Dec 00 - Mike: Generate optimal code for two special cases of @[:]
 *              for @[32:63] and @[0:31]
 * 18 Dec 00 - Mike: Implement idAddrOf (needed for some Sparc parent stack
 *              frame references, e.g. a[ v[ int 9 ]] -> &(v9)).
 * 10 Jan 01 - Brian: Removed assert from Translate::castParameter() that 
 *              failed when a formal was double and the matching actual was two
 *              32 bit words.
 * 08 Feb 01 - Mike: Implemented overlapped registers; insert space before the
 *              "*" of heavy duty casts (prevents unwanted ** operators);
 *              128 bit operations forced to float; added useSwap8
 * 13 Feb 01 - Mike: Implemented idCastIntStar (for "second half of 64 bit var
 *              problem); fixed problem of &(cast)v11 (not valid C); initial
 *              type of assignments reverted to original code; _res8 no longer
 *              declared "register" (have to take its address); most casts are
 *              "heavy duty kind" again (some cases need this, e.g. thirds)
 * 14 Feb 01 - Mike: Initial type now comes from the RTAssgn (sslinst.cc)
 * 23 Feb 01 - Mike: Added check for uses of vars of different sizes; if so,
 *              the largest size used becomes the size declared. This fixes
 *              some problems with int/double manipulations, e.g. thirds, ninths
 * 02 Mar 01 - Mike: processVar etc calls print() instead of operator<<. This
 *              makes it easier when operator<< has debugging info (e.g. <type>)
 * 05 Mar 01 - Simon: Removed an RTCOND case
 * 19 Mar 01 - Mike: Removed some unused code (checking for max size of vars)
 * 20 Mar 01 - Mike: Avoid some casts to floatmax by using the *= comparison
 * 22 Mar 01 - Mike: Implement the first case of overlapping parameters
 * 23 Mar 01 - Mike: FirstPass considers return locations of CALL_HRTLs; fixes
 *              Pentium callchain test
 * 26 Mar 01 - Mike: promote sizes of parameters to sizeof(int) (on the source
 *              machine) in castParameter before using overlapped param logic
 * 28 Mar 01 - Mike: firstPass considers parameters of CALL_HRTLs and RET_HRTLs
 * 29 Mar 01 - Mike: Don't declare a register if it is also a parameter;
 *              emitReturn can handle a general expression now
 * 31 Mar 01 - Mike: getFixedDest() returns NO_ADDRESS for non fixed addresses
 * 11 Apr 01 - Mike: args to ints2float() are printAsC now (so r[8] -> r8)
 * 12 Apr 01 - Mike: Added idSqrt unary operator
 * 19 Apr 01 - Mike: Address comments are now always on (no -a); init DF to 0
 * 24 Apr 01 - Mike: Implement %SKIP and %RPT as implicit loops; merged very
 *              similar code into emitBBCode()
 * 26 Apr 01 - Mike: CSR::setReturnType() takes a UserProc now (was Proc);
 *              prevent a lot of "use of %afp+..." error messages
 * 06 May 01 - Mike: "%pc used" warning has address now
 * 23 Jul 01 - Mike: Process %afp as "_locals + sizeof(locals)" if stackInverted
 * 26 Jul 01 - Mike: Implemented guarded assignments; added idLNot code
 * 27 Jul 01 - Mike: Added dubious code to not cast a register to a diff size
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 * 06 Aug 01 - Mike: When processing HLJcond, emit code for regular semantics
 * 11 Aug 01 - Mike: Added support for logical binary operators idAnd and idOr
 * 14 Aug 01 - Mike: Fixed expressions of the kind "(a *u b) >> c" where a
 *              signed right shift is required (e.g. pa-risc EXTRS)
 * 30 Aug 01 - Mike: Replaced castParameter with getParamTypeList
 * 19 Oct 01 - Mike: Heavy duty cast only if basic types different basic types
 *              (again); reason is that on big endian machines, HD casts fail!
 *              Also improved an error message
 * 23 Oct 01 - Mike: If actual and formal parameters have the same basic types
 *              but one is more than a machine word size, need HD cast again
 *              (fixes ninths test)
 * 26 Oct 01 - Mike: Fixed a bug in the above: was using size in bytes, not bits
 * 05 Nov 01 - Mike: Fixed an error message re cast from floatmax to floatmax
 */

#include "global.h"
#include "prog.h"                       // For getProc()
#include "proc.h"
#include "cfg.h"
#include "ss.h"
#include "rtl.h"
#ifndef WIN32
#include "ElfBinaryFile.h"
#endif
#include "options.h"                    // For str()
#include "csr.h"
#include "frontend.h"                   // For decodeProc()
#include "overlap.h"                    // For class Overlap
#include "backend.h"                    // For class definition
#include "decoder.h"                    // For isFuncPrologue()
#include "type.h"

/*==============================================================================
 * FUNCTION:        legalCname
 * OVERVIEW:        convert the label's name (which might contain dots or other
 *                    illegal chars) to one that is a legal C function name
 * PARAMETERS:      labelName: string with the label name
 * RETURNS:         string with the modified name
 *============================================================================*/
char* legalCname(const char* labelName)
{
    const int len = strlen(labelName);
    char* ret = new char[len+1];
    for (int i=0; i < len; i++) {
        char c = *labelName++;
        if (c == '.')
            ret[i] = '_';       // Hope no name collision
        else
            ret[i] = c;
    }
    ret[len] = '\0';
    return ret;
}

/*==============================================================================
 * FUNCTION:        Translate::Translate
 * OVERVIEW:        Constructor for class Translate
 * PARAMETERS:      none
 * RETURNS:         <nothing>
 *============================================================================*/
Translate::Translate()
    : labelNum(0)
{
    // Initialise the constant SemStr(so they don't have to be constructed
    // and deleted all the time)
    regX.push(idRegOf);
    regX.push(idIntConst);
    regX.push(-1);

    // Code that uses this is commented out
    // varX.push(idVar);
    // varX.push(-1);

    // Find indices (if any) for %SKIP and %RPT. These are special indices for
    // string instructions (e.g. Pentium REP.MOVSb)
    idSKIP = theSemTable.findItem("%SKIP", false);
    idRPT  = theSemTable.findItem("%RPT", false);
}

// Return true if pLoc represents a floating point register
// Assumes that pLoc starts with idRegOf
// Uses prog.RTLDict
bool isFloat(const SemStr* pLoc)
{
    if (pLoc->getSecondIdx() == idTemp) {
        // The convention is that temps are integer, except for tmpf and tmpd
        const string& name = theSemTable[pLoc->getThirdIdx()].sName;
        if ((name[3] == 'f') || (name[3] == 'd')) return true;
        return false;
    }
    if (pLoc->getSecondIdx() != idIntConst) {
        ostrstream ost;
        ost << "isFloat: expect constant register index in ";
        pLoc->print(ost);
        error(str(ost));
        return false;
    }
    int regNum = pLoc->getThirdIdx();
    Register& reg = prog.RTLDict.DetRegMap[regNum];
    return reg.isFloat();
}

/*==============================================================================
 * FUNCTION:        makeReg
 * OVERVIEW:        Make a string with a C representation of a register; e.g.
 *                    given 12, return "r12", but given 64 it would return "d64"
 * PARAMETERS:      regNum: register number
 * RETURNS:         string representing the register
 *============================================================================*/
string makeReg(int regNum)
{
    ostrstream ost;
    const Register& reg = prog.RTLDict.DetRegMap[regNum];
    int size = reg.g_size();
    bool flt = reg.isFloat();
    if (flt) {
        if (size == 32)
            ost << "f";
        else if (size == 64)
            ost << "d";
        else ost << "ld";
    }
    else
        if (size <= 32)
            ost << "r";
        else
            ost << "l";
    ost << dec << regNum;
    return string(str(ost));
}

// Process r[exp]. The expression will usually be an integer constant, but
// could be a simple expression (like 6+1) or an idTemp
string Translate::processRegOf(const SemStr* pLoc, Type cType)
{
    ostrstream ost;
    int num;
    int idx = pLoc->getSecondIdx();
    if (idx == idTemp) {
        const string& name = theSemTable[pLoc->getThirdIdx()].sName;
        temps.insert(name);     // Remember to declare this temp
        return name;
    }
    else if (pLoc->getSecondIdx() == idIntConst) {
        num = pLoc->getThirdIdx();
    }
    else {
        // Must be an expression; it should be int const + int const
        if (idx != idPlus) {
            ostrstream ost;
            ost << "Can't handle r[ expression ]: " << *pLoc;
            error(str(ost));
            assert (idx == idPlus);
        }
        SemStr* pTerm1 = pLoc->getSubExpr(0);
        SemStr* pTerm2 = pLoc->getSubExpr(1);
        assert (pTerm1->getFirstIdx() == idIntConst);
        assert (pTerm2->getFirstIdx() == idIntConst);
        num = pTerm1->getSecondIdx() + pTerm2->getSecondIdx();
    }
    // Registers are now counted in a special pass before the main translation
    // regNumbers.insert(num);
    // Heavy duty cast if required
    const Register& reg = prog.RTLDict.DetRegMap[num];
    // Note: we use the "*=" operator (compare, not multiply-equals) so that
    // sign is disregarded, and all floats > 64 bits are considered the same
    // size (this will avoid some unnecessary casts to floatmax)
    if (!(cType *= reg.g_type())) {
        // Note: we only want to co-erce the sign and basic type, not the size.
        // MVE: Not really sure about this one
        Type ty(cType);
        ty.setSize(reg.g_type().getSize());
        ost << " *(" << ty.getCtype() << "*)&";
    }
//  ost << makeReg(num);
    ost << ovl.getRegExpr(num);
    return str(ost);
}

string Translate::processVar(const SemStr* pLoc, Type cType)
{
    ostrstream ost;
    int idx = pLoc->getSecondIdx();
    Type vType = proc->getVarType(idx);
/* ? What was this for?
    // Cast to unsigned if required
    if ((cType.getType() == INTEGER) && (!cType.getSigned())) {
        ost << "(" << cType.getCtype() << ")";
    }
*/
    emitCast(cType, vType, ost);
    pLoc->print(ost);
    return str(ost);
}

// Convert a m[exp] SemStr into a C string expr of the same thing.
// Does not handle endianness swaps
string Translate::processMemOfRaw(const SemStr* pLoc, Type cType)
{
    SemStr* pSub = pLoc->getSubExpr(0);
    ostrstream ost;
    ost << " *((" << cType.getCtype() << "*)(";
    // Address expressions are unsigned 32 bit
    ost << processExpr(pSub, Type(INTEGER, 32, false)) << "))";
    return str(ost);
}

// Output the start of a swap (e.g. "_swap2(")
const char* Translate::swapStart(const Type& cType)
{
    if (!progOptions.noendian && (SRCENDIAN != TGTENDIAN)) {
        switch(cType.getSize()) {
            case 32:
                useSwap4 = true;
                return "_swap4(";
            case 16:
                useSwap2 = true;
                return "_swap2(";
            case 8:
                return "";      // No swap needed
            case 64:
                useSwap8 = true;
                if (cType.getType() == FLOATP)
                    return "_swap8f(";
                else
                    return "_swap8i(";
            default:
                ostrstream ost;
                ost << cType.getSize() << " bit swaps not implemented yet";
                error(str(ost));
                return "?";
        }
    }
    return "";
}

// Output the end of a swap (e.g. ")")
const char* Translate::swapEnd(const Type& cType)
{
    if (!progOptions.noendian && (SRCENDIAN != TGTENDIAN)) {
        int sz = cType.getSize();
        if ((sz == 32) || (sz == 16) || (sz == 64))
            return ")";\
    }
    return "";
}

// Process a m[exp] on the RHS, taking into account endianness
string Translate::processMemOf(const SemStr* pLoc, Type cType)
{
    ostrstream ost;
    ost << swapStart(cType) << processMemOfRaw(pLoc, cType) << swapEnd(cType);
    return str(ost);
}
    
string Translate::xlateUnaryOp(int idx)
{
    switch(idx)
    {
        case idNot: return string("~");
        case idLNot: return string("!");
        case idNeg: return string("-");
        case idAddrOf:
            // This can happen in Sparc programs which take the address of a
            // "shadow parameter" location, which is in the parent's stack frame
            return string("&");
        case idCastIntStar:
            // This can happen when converting the second half of a 64 bit
            // quantity to the appropriate expression
            return string("(int*)");
        case idSqrt:
            // Note: the operand will be put in parentheses
            return string("sqrt");
        default:
            ostrstream ost;
            ost << "Unknown operator " << dec;
            ost << idx << " (" << theSemTable[idx].sName;
            ost << ") in unary expression";
            error(str(ost));
            return string("?");
    }
}

string Translate::xlateBinaryOp(int idx)
{
    switch(idx)
    {
        case idFPlus: case idFPlusd: case idFPlusq:
        case idPlus:    return  string("+");
        case idFMinus: case idFMinusd: case idFMinusq:
        case idMinus:   return  string("-");
        case idFMult: case idFMultd: case idFMultq: case idMults:
        case idMult:    return  string("*");
        case idFDiv: case idFDivd: case idFDivq: case idDivs:
        case idDiv:     return  string("/");
        case idMod: case idMods:
                        return  string("%");
        case idShiftL:  return  string("<<");
        case idShiftR:  return  string(">>");
        case idShiftRA: return  string(">>");
        case idBitOr:   return  string("|");
        case idBitAnd:  return  string("&");
        case idBitXor:  return  string("^");
        case idEquals:  return  string("==");
        case idNotEqual:return  string("!=");
        case idLess:    return  string("<");
        case idGtr:     return  string(">");
        case idLessEq:  return  string("<=");
        case idGtrEq:   return  string(">=");
        case idLessUns: return  string("<");
        case idGtrUns:  return  string(">");
        case idLessEqUns:return string("<=");
        case idGtrEqUns:return  string(">=");
        case idAnd:     return  string("&&");
        case idOr :     return  string("||");
        //case idAt:        return  string(">>");
        default:
            ostrstream ost;
            ost << "Unknown operator ";
            ost << idx << " in binary expression";
            error(str(ost));
            return string("?");
    }
}

string Translate::processIntConst(const SemStr* pExp, Type cType)
{
    int k = pExp->getSecondIdx();
    ostrstream ost;
    if (cType.getSigned())
        ost << k;
    else
        ost << (unsigned)k;
    return str(ost);
}

string Translate::processCodeAddr(const SemStr* pExp)
{
    // These addresses are relative to the pc, and so are in the code segment
    // of the source program. They can't be used without modification in the
    // target program
    int k = pExp->getSecondIdx();
    ostrstream ost;
    // Get a host pointer to k, if in the code section
    const char* last; int delta;
    const char* p = (const char*) prog.getCodeInfo((ADDRESS)k, last, delta);
    if (isFuncPrologue((ADDRESS)p)) {
        // We have a pointer to code. Find out if it's already decoded
        UserProc* proc = (UserProc*)prog.findProc(k);
        if (proc)
            // Just use the name of the proc as the value of this expression
            ost << proc->getName();
        else {
            // We have an undecoded procedure. Decode it now!
            // Must visit first to give a name
            proc = (UserProc*)prog.visitProc(k);
            // If this is a deleted proc, don't decode again
            if (proc == 0)
                return str(ost);
            NJMCDecoder decoder(prog.RTLDict, prog.csrSrc);
            FrontEndSrc fe(delta, (ADDRESS)(last-delta), decoder);
            decodeProc(k, fe);
            ost << proc->getName();
            // HACK For now, assume that all callbacks return a bool (int8)
            prog.csrSrc.setReturnType(proc, Type(INTEGER, 8));
        }
        // Add this procedure name to the set to delare a prototype for
        protos.insert(proc->getName());
    }
    else if (isString(p, last))
        ost << "\"" << p << "\"";   // Constant string expression
    else
        ost << hex << "0x" << k << " /* EDIT ME! (pc relative code) */";
    return str(ost);
}

/*==============================================================================
 * FUNCTION:        processExpr
 * OVERVIEW:        Emit code for the given expression.
 * PARAMETERS:      pExp: pointer to the expression to be emitted
 *                  cType: the "current" type; if operands are not this
 *                    type, they must be cast
 * RETURNS:         string representing the expression
 *============================================================================*/
string Translate::processExpr(const SemStr* pExp, Type cType)
{
    int idx = pExp->getFirstIdx();
    int n = theSemTable[idx].iNumVarArgs;
    ostrstream ost;
    switch(n)
    {
        case 0:
            switch (idx) {
                case idIntConst:
                    return processIntConst(pExp, cType);
                case idCodeAddr:
                    return processCodeAddr(pExp);
                case idFltConst:
                {
                    union _u {
                        struct {
                            int i1; int i2;
                        } ii;
                        double d;
                    } u;
                    u.ii.i1 = pExp->getSecondIdx();
                    u.ii.i2 = pExp->getThirdIdx();
                    // Force a lot of precision, but not showpoint so things
                    // like 0.5 come out without unnecessary digits
                    ost.precision(18);
                    ost << u.d;
                    return str(ost);
                }
                case idVar:
                    return processVar(pExp, cType);
                case idAFP:
                    if (prog.csrSrc.isStackInverted()) {
                        ostrstream ost;
                        ost << "_locals+" << proc->getLocalsSize();
                        return str(ost);
                    } else
                        return "_locals";
                case idAGP: return "_globals";
                case idPC: {
                    // Most of the time, we can get away with using the
                    // native PC for this. Usually arises from position
                    // independent code to access global (or any) data.
                    char buf[16];
                    sprintf(buf, "%d", uAddr);
// I'm not convinced that this is ever right!
{ ostrstream ost; ost << "%pc used at " << hex << uAddr; warning(str(ost));}
                    return buf;
                }
                default:
                {
                    if (idx >= idMachSpec) {
                        // This is a special register, such as %pc or %Y
                        // We have declared these already as an int
                        const string& strg = theSemTable[idx].sName;
                        if (strg.length() && strg[0] == '%')
                            ost << strg.substr(1);
                        else
                            ost << strg;
                        return str(ost);
                    }
                    else {
                        ostrstream ost;
                        ost << "Unhandled zero parameter operator in "                                      "expression ";
                        pExp->print(ost);
                        error(str(ost));
                        return string("?");
                    }
                }
            }

        case 1: {
            // Unary expression. But r[] amd m[] are special cases
            switch(idx)
            {
                case idRegOf:
                    return processRegOf(pExp, cType);
                case idMemOf:
                    return processMemOf(pExp, cType);
                case idSignExt: {
                    // Ensure that cType is signed
                    // For now, assume that ! is "commutative", i.e. the
                    // presence of the ! implies that the subexpression as
                    // well as the result are signed
                    //Type oldType(cType);
                    cType.setSigned(true);
                    SemStr* pSub = pExp->getSubExpr(0);
                    string res = processExpr(pSub, cType);
                    delete pSub;
                    return res;
                }
                case idSize:
                case idFsize:
                case idItof:
                case idFtoi:
                case idTruncu:
                case idTruncs:
                case idZfill:
                case idSgnEx:
                {
                    Type fromType(cType);
                    int fromSize = pExp->getSecondIdx();
                    fromType.setSize(fromSize);
                    Type toType(cType);
                    if (idx != idSize) {
                        int toSize = pExp->getThirdIdx();
                        toType.setSize(toSize);
                    }
                    // A few of these change the broad type as well as size
                    switch (pExp->getFirstIdx()) {
                        case idItof:
                        case idFsize:   toType.setType(FLOATP); break;
                        case idFtoi:    toType.setType(INTEGER); break;
                        case idZfill:   toType.setSigned(false); break;
                        case idSgnEx:   toType.setSigned(true); break;
                        // If idSize and no idSgnEx, make it unsigned
                        case idSize:    toType.setSigned(false); break;
                        case idTruncu:  toType.setType(INTEGER);
                                        toType.setSigned(false); break;
                        case idTruncs:  toType.setType(INTEGER);
                                        toType.setSigned(true); break;
                    }
                    // Some imply the from type as well
                    switch (pExp->getFirstIdx()) {
                        case idFsize:
                        case idFtoi:    fromType.setType(FLOATP); break;
                        case idItof:
                        case idTruncu:  fromType.setType(INTEGER);
                                        fromType.setSigned(false); break;
                        case idTruncs:  fromType.setType(INTEGER);
                                        fromType.setSigned(true); break;
                        case idZfill:   fromType.setType(INTEGER);
                                        fromType.setSigned(false); break;
                        case idSgnEx:   fromType.setType(INTEGER);
                                        fromType.setSigned(true); break;
                        // For idSize, make the from type unsigned, so the
                        // cast of the memory reference will be correct
                        case idSize:    fromType.setSigned(false); break;
                    }
                    SemStr* pSub = pExp->getSubExpr(0);
                    string res;
                    if (!(toType -= cType)) {
                        ostrstream ost;
                        ost << "type conflict in expression " << *pExp;
                        ost << " at " << hex << uAddr << "\n"
                         << "   Wanted " << cType.getCtype()
                         << " and computing " << toType.getCtype();
                        warning(str(ost));
                        cType = toType;
                    }
                    // Note: we don't want the heavy duty cast here; these
                    // may actually need conversions. For now, assume that
                    // a standard C cast is all that is required
                    // Note: may need two casts! If going from say int32 to
                    // unsigned64, need (uns64)(unsigned) (otherwise it con-
                    // verts to int64 first, which sign extends)
                    res = string("(") + toType.getCtype() + ")(";
                    // Note: may need two casts! If going from say int32 to
                    // unsigned64, need (uns64)(unsigned) (otherwise it con-
                    // verts to int64 first, which sign extends)
                    if (toType.getSize() != fromType.getSize() &&
                      toType.getSigned() != fromType.getSigned()) {
                        Type midType(fromType);
                        midType.setSigned(toType.getSigned());
                        res += string(midType.getCtype()) + ")(";
                    }
                    res += processExpr(pSub, fromType) + ")";
                    delete pSub;
                    return res;
                }

                default:
                {
                    const SemStr* pArg = pExp->getSubExpr(0);
                    // Don't allow casts when take the address of a var
                    // E.g. &(unsigned int32)v11 is not legal C
                    if ((idx == idAddrOf) && (pArg->getFirstIdx() == idVar)) {
                        ost << "&"; pArg->print(ost);
                    }
                    else {
                        // Ordinary unary op; allow casts
                        ost << xlateUnaryOp(idx) << "(";
                        ost << processExpr(pArg, cType);
                        ost << ")";
                    }
                    delete pArg;
                    return str(ost);
                }
            }
        }

        case 2: {
            // Binary expression
            const SemStr* pArg1 = pExp->getSubExpr(0);
            const SemStr* pArg2 = pExp->getSubExpr(1);
            // For unsigned operators (relational and shift), cast the operands
            // to unsigned
            switch(idx) {
                case idShiftR:
                case idLessUns:
                case idGtrUns:
                case idLessEqUns:
                case idGtrEqUns:
                case idMult:
                case idDiv:
                case idMod: {
                    Type uType(cType);
                    uType.setSigned(false);
                    if (cType != uType) {
                        // Resultant type has to be of type cType, so cast it
                        // Note: can't use heavy duty type, since can't take the
                        // address of expressions
                        ost << "(" << cType.getCtype() << ")(";
                    }
                    ost << processExpr(pArg1, uType);
                    ost << xlateBinaryOp(idx);
                    if (idx == idShiftR)
                        // Don't need to force the RHS to unsigned for shifts
                        ost << processExpr(pArg2, cType);
                    else
                        ost << processExpr(pArg2, uType);
                    // May need to close a bracket
                    if (cType != uType)
                        ost << ")";
                    return str(ost);
                }
                /* Ugh - operators should not have type.
                case idFPlus:
                case idFMinus:
                case idFMult:
                case idFDiv: {
                    Type fType(FLOATP, 32);
                    ost << processExpr(pArg1, fType);
                    ost << xlateBinaryOp(idx);
                    ost << processExpr(pArg2, fType);
                    return str(ost);
                }
                case idFPlusd:
                case idFMinusd:
                case idFMultd:
                case idFDivd: {
                    Type fType(FLOATP, 64);
                    ost << processExpr(pArg1, fType);
                    ost << xlateBinaryOp(idx);
                    ost << processExpr(pArg2, fType);
                    return str(ost);
                }
                case idFPlusq:
                case idFMinusq:
                case idFMultq:
                case idFDivq: {
                    Type fType(FLOATP, 128);
                    ost << processExpr(pArg1, fType);
                    ost << xlateBinaryOp(idx);
                    ost << processExpr(pArg2, fType);
                    return str(ost);
                }
                */
                case idFPlus:
                case idFMinus:
                case idFMult:
                case idFDiv:
                case idFPlusd:
                case idFMinusd:
                case idFMultd:
                case idFDivd:
                case idFPlusq:
                case idFMinusq:
                case idFMultq:
                case idFDivq: {
                    Type fType(cType);
                    fType.setType(FLOATP);
                    ost << processExpr(pArg1, fType);
                    ost << xlateBinaryOp(idx);
                    ost << processExpr(pArg2, fType);
                    return str(ost);
                }
            }
            ost << "(" << processExpr(pArg1, cType) << ")";
            ost << xlateBinaryOp(idx);
            ost << "(" << processExpr(pArg2, cType) << ")";
            delete pArg1; delete pArg2;
            // Special test for %afp + int k
            // + %afp int k
            // 0  1    2  3
            if (idx == idPlus && pExp->getSecondIdx() == idAFP &&
              pExp->getThirdIdx() == idIntConst) {
                // Error message if k is greater equals the size of the
                // _locals array (many parameter errors can be caught this
                // way)
                int k = pExp->getIndex(3);
//              if (k >= proc->getLocalsSize()) {
// Too many error messages, caused by the temps in the SSL file. For now, only
// print if greater, not greater-or-equals
                if (k > proc->getLocalsSize()) {
                    ostrstream ost;
                    ost << "Use of %afp+k where k (" << k <<
                        ") >= size of locals (" << proc->getLocalsSize() << ")";
                    error(str(ost));
                }
            }
            return str(ost);

        }

        case 3: {
            const SemStr* pArg1 = pExp->getSubExpr(0);
            const SemStr* pArg2 = pExp->getSubExpr(1);
            const SemStr* pArg3 = pExp->getSubExpr(2);
            int val2 = pArg2->getSecondIdx();
            if (pArg2->getFirstIdx() != idIntConst) val2 = -1;
            int val3 = pArg3->getSecondIdx();
            if (pArg3->getFirstIdx() != idIntConst) val3 = -1;
            switch (idx) {
                case idAt:
                    // a@b:c
                    if (*pArg2 == *pArg3) {
                        // Special optimisation for b == c
                        // We can use (((unsigned)a >> b) & 1);
                        ost << "(((unsigned int32)";
                        ost << processExpr(pArg1, cType);
                        ost << ">>";
                        ost << processExpr(pArg2, cType);
                        ost << ")&1)";
                    }
                    else if (val2 == 32 && val3 == 63) {
                        // Special case for getting the upper word of a long
                        ost << processExpr(pArg1, cType);
                        ost << ">>32";
                    }
                    else if (val2 == 0 && val3 == 31) {
                        // Special case for getting the lower word of a long
                        Type type32(cType);
                        type32.setSize(32);
                        ost << processExpr(pArg1, type32);
                    }
                    else {
                        // Must use (((unsigned)a >> b) & ((1<<(c-b+1)-1)))
                        // Note: assumes b < c!!
                        Type uType(cType);
                        uType.setSigned(false);
                        ost << "(((" << uType.getCtype() << ")";
                        ost << processExpr(pArg1, cType);   // a
                        ost << ">>";
                        ost << processExpr(pArg2, cType);   // b
                        ost << ")&((1<<(";
                        ost << processExpr(pArg3, cType);   // c
                        ost << "-";
                        ost << processExpr(pArg2, cType);   // b
                        ost << "+1)-1)))";
                    }
                    break;
                case idTern:
                    ost << "(" << processExpr(pArg1, cType) << ") ? ";
                    ost << processExpr(pArg2, cType) << " : ";
                    ost << processExpr(pArg3, cType);
                    break;
                default:
                    error("Unknown ternary expression");
            }
            delete pArg1; delete pArg2; delete pArg3; 
            return str(ost);
        }

        default: {
            ostrstream ost;
            ost << "processExpr: unknown type of expression: ";
            ost << pExp->sprint();
            error(str(ost));
        }
    }
    return 0;
}


/*==============================================================================
 * FUNCTION:        processRT
 * OVERVIEW:        Process this RT
 * PARAMETERS:      pRT: pointer to RT object
 *                  os: ref to ostrstream object that accumulates the code for
 *                  the current procedure
 * RETURNS:         true if error
 *============================================================================*/
bool Translate::processRT(RT* pRt)
{
    switch (pRt->getKind())
    {
        case RTASSGN:
        {
            if (!progOptions.highLevelC)
                os << "\t";                     // Initial tab

            RTAssgn* pRtA = (RTAssgn*)pRt;
            // If guarded, emit an if statement
            const SemStr* guard = pRtA->getGuard();
            if (guard) {
                os << "if (";
                os << processExpr(guard, Type());
                os << ") {";
            }

            SemStr* pLoc = pRtA->getLHS();
            // Initial type, from the assignment (set in sslinst.cc)
            Type cType(pRtA->getType());

            // First process the left hand side (dest)
            int idx = pLoc->getFirstIdx();
            switch(idx)
            {
                case idRegOf:
                {
                    os << processRegOf(pLoc, cType);
                    break;
                }
                case idMemOf:
                    os << processMemOfRaw(pLoc, cType);
                    break;
                case idVar:
                    os << processVar(pLoc, cType);
                    break;
#if 0
                case idPC: { 
                    // An assignment to the PC. The only one that should be left
                    // by now is one of the form "%pc := %pc +- <expression>"
                    // These are used for looping instructions, e.g. Pentium
                    // string instructions. We assume that where a conditional
                    // branch is needed, it will be of the form:
                    //    <cond> ? <displacement> : 0
                    // It will be further assumed that pc +- displacement
                    // will resolve to the label at the start of this RTL
                    SemStr* rhs = pRtA->getRHS();
                    int firstRight = rhs->getFirstIdx();
                    if (((firstRight != idPlus) && (firstRight != idMinus)) ||
                      (rhs->getSecondIdx() != idPC)) {
                        ostrstream ost;
                        ost << "Unhandled expression assigning to %pc: ";
                        pRtA->print(ost);
                        error(str(ost));
                        return true;
                    }
                    // Get the <expression>
                    SemStr* expr = rhs->getSubExpr(1);
                    if (expr->getFirstIdx() == idTern) {
                        SemStr* ifFalse = expr->getSubExpr(2);
                        if ((ifFalse->getFirstIdx() != idIntConst) ||
                          (ifFalse->getSecondIdx() != 0)) {
                            ostrstream ost;
                            ost << "Ternary expression after `%pc := %pc +-' "
                              "must be of the form `<expr> ? <displacement> "
                              ": 0'\n";
                            ost << "Found: ";
                            expr->print(ost);
                            error(str(ost));
                            delete ifFalse;
                            return true;
                        }
                        // Emit a conditional goto
                        SemStr* cond = expr->getSubExpr(0);
                        os << "if (" << processExpr(cond, cType) <<
                          ") goto label" << labelNum << ";\n";
                        delete ifFalse; delete cond;
                    }
                    else
                        // We assume that this is a direct goto
                        os << "goto label" << labelNum << ";\n";
                    delete expr;
                    return false;           // Don't process RHS
                    break;
                }
#endif
                default:
                    if (idx >= idMachSpec) {
                        // This is a special register destination, e.g. %pc or
                        // %Y, or it may be %SKIP or %RPT
                        os << processExpr(pLoc, cType);
                    }
                    else {
                        ostrstream ost;
                        ost << " Non register/memory destination: ";
                        pRt->print(ost);
                        error(str(ost));
                        return true;
                    }
            }

            // The assign
            os << "=";

            // Next process the right hand side
            // If the LHS is memory, remember to swap the right
            if (pLoc->getFirstIdx() == idMemOf) {
                os << swapStart(cType) << processExpr(pRtA->getRHS(), cType) <<
                    swapEnd(cType);
            }
            else
                os << processExpr(pRtA->getRHS(), cType);

            // Finish off
            os << ";";
            if (guard)
                // Close the curly bracket for the if(guard){assignment;}
                os << "}";
            os << endl;

            // Now check if we've just emitted %SKIP= or %RPT=
            if (idx == idSKIP) {
                // We need to emit a jump to after the end of this instruction.
                // It is assumed that labelNum+1 is the label there
                os << "\tif (SKIP) goto label" << labelNum+1 << ";\n";
            } else if (idx == idRPT) {
                // We need to emit a jump to the start of this instruction.
                // It is assumed that labelNum is the label there
                os << "\tif (RPT) goto label" << labelNum << ";\n";
            }

            break;
        }

        case RTFLAGDEF:
            printf(" RT flag def\n");
            return true;

        case RTFLAGCALL:
            // Just ignore these
            break;

    }
    return false;
}

/*==============================================================================
 * FUNCTION:        emitFunc
 * OVERVIEW:        Emit the function. Emit declarations for the variables, and
 *                  emit the function proper. If needed, emit the swap code also
 * NOTE:            By now, the whole function has been translated; the results
 *                    are in the string os (member variable)
 * PARAMETERS:      proc: ptr to Proc object for current procedure
 *                  spec: if true, this is a speculative translation
 * RETURNS:         <nothing>
 *============================================================================*/
void Translate::emitFunc(UserProc* proc, bool spec)
{

    // #include "uqbt.h"
    of << "#include \"uqbt.h\"\n";

#if TGT == MC68K
    if (strcmp(proc->getName(), "PilotMain") == 0) {
        // Special prototype for PilotMain.
        // This needs to be part of a spec one day
        //assert(proc->getNumArgs() == 3);  // HACK
        of << "unsigned int32 PilotMain(";
        of <<   "unsigned int16 " << *proc->getParameter(0);
        of << ", void* "          << *proc->getParameter(1);
        of << ", unsigned int16 " << *proc->getParameter(2);
        of << ") {";
    }
    else
#endif

    {
        proc->printReturnTypeAsC(of);                       // Return type
        of << " " << legalCname(proc->getName()) << "(";    // name (
        proc->printParamsAsC(of);                           // params
        of << ") {";
    }
    of << endl;

    // Declare the locals for _swapX, if needed. Note that pentium targets
    // don't need any, since they are forced to use gcc as the output compiler,
    // and those macros may as well declare their own temps
#if TGT == SPARC
    if (useSwap4)
        of << "register unsigned _arg, _wrk, _mask;\n";
    if (useSwap2)
        of << "register unsigned _arg2, _res;\n";
#elif TGT == PENTIUM
    if (useSwap8)
        of << "int _res8[2];\n";
#endif

    // Declare the symbolic locals
    proc->printLocalsAsC(of);

    // Declare the overlapped registers
    ovl.declareRegs(of);

    SemStr thisReg;         // SemStr for current register
    thisReg.push(idRegOf); thisReg.push(idIntConst); thisReg.push(0);

    // Declare the register locals
    if (regNumbers.size()) {
        Type currType(INTEGER, 99); // Current type
        bool open = false;          // True if a line started
                                    // (so need to close with ;\n)
        of << "\t";
        set<int>::iterator it;
        for (it = regNumbers.begin(); it != regNumbers.end(); it++)
        {
            if (ovl.isOverlappedReg(*it))
                // Already declared by declareRegs()
                continue;

            // Find out if this is a parameter; if so, don't declare it
            thisReg.substIndex(2, *it);
            if (proc->isParam(thisReg))
                continue;               // No declare
            
            const Register& reg = prog.RTLDict.DetRegMap[*it];
            Type rType = reg.g_type();
            if (!(currType == rType)) {
                currType = rType;
                if (open) of << ";\n\t";
                open = true;
                of << rType.getCtype() << " ";
            }
            else of << ", ";
            of << makeReg(*it);
        }
        if (open) of << ";\n";
    }

    // Declare the prototypes
    set<string>::iterator it;
    for (it = protos.begin(); it != protos.end(); it++)
        // Just need "void NAME();"
        of << "\tvoid " << *it << "();\n";

    // Declare the temps
    Type ty, curr_ty(UNKNOWN, 32);
    bool open = false;
    for (it = temps.begin(); it != temps.end(); it++) {
        ty = Type::getTempType(*it);
        if (!(ty == curr_ty)) {
            if (open) of << ";\n";
            of << "\t" << ty.getCtype() << " ";
            open = false;
        }
        if (open) of << ",";
        of << (*it);
        open = true;
    }
    if (open) of << ";\n";

    // Declare CF, Y, etc when needed. Assume that these are all ints for now
    set<INDEX>::iterator its;
    for (its = stIdx.begin(); its != stIdx.end(); its++) {
        // Most names will start with a %, which has to be removed
        const string& strg = theSemTable[*its].sName;
        if (strg.length() && strg[0] == '%')
            of << "\tint " << strg.substr(1);
        else
            of << "\tint " << strg;
#if SRC == PENT
            if (strg == "%DF")
                // Special hack for pentiums: direction flag defaults to up
                // (down = 0)
                of << "=0";
#endif
        of << ";\n";
    }

    // Swap argv if required. That way, we can just swap all memory references
    if ((strcmp(proc->getName(), "main") == 0) && (useSwap4 || useSwap2)) {
        of << "\n/* Fix argv to be opposite endianness */\n";
        of << "int i; "
        "for (i=0; i < v0; i++) ((char**)v1)[i] = _swap4(((char**)v1)[i]);\n\n";
    }

    // Insert a comment with the entry native address
    of << "\t/* " << hex << proc->getNativeAddress() << " */\n";

    // Check if the first BB is the entry point (won't be if there are orphans)
    // If so, generate a goto to the first BB, and set a label on the first BB
    BB_CIT bbit;
    // Get the BB for the entry point
    PBB pBB = proc->getCFG()->getFirstBB(bbit);     
    // Note: using -S, it is possible for a proc to have no BBs, so pBB may be
    // NULL here
    if (pBB && (proc->getNativeAddress() != pBB->getLowAddr())) {
        of << "\tgoto L" << dec << proc->getEntryBB()->getLabel() << ";\n";
    
    }

    // Output the string for the function proper
    of << "\n" << str(os);
    // End the function
    of << "}\n";

    if (spec) {
        // This is a speculative translation. Sometimes, we accidenally will
        // translate parts of the startup code, which may well clash with names
        // in libraries. So we specify these functions as weak
        // Hopefully, this is moderately portable!
        // We place the pragma here so that it is outside of the function's
        // definition, but not in the area that may be rewritten if we change
        // the function's return type
        of << "#pragma weak " << proc->getName() << endl;
    }

    of.close();

}

/*==============================================================================
 * FUNCTION:        emitReturn
 * OVERVIEW:        emit "return <retlocn>;\n"
 * PARAMETERS:      proc - pointer to the current procedure's Proc object
 * RETURNS:         <Nothing>
 *============================================================================*/
void Translate::emitReturn(Proc* proc)
{
    os << "\treturn";
    // Get the return location; e.g. v22 or r[8] or r[64];
    const SemStr* rl = proc->getReturnLoc();
    if (rl->len())
        os << " " << processExpr(rl, rl->getType());
    os << ";\n";
}

/*==============================================================================
 * FUNCTION:        processHL
 * OVERVIEW:        Process a high level RTL: HLJump, HLJcond, HLRet, etc
 * PARAMETERS:      pRTL: pointer to the HL RTL
 *                  proc: pointer to the current Proc object
 * RETURNS:         true if found and processed a HL RTL
 *                  false if RTL still needs to be processed
 *============================================================================*/
bool Translate::processHL(const HRTL* pRTL, PBB pBB, Proc* proc)
{
    if (pRTL->getKind() == LOW_LEVEL_HRTL)
        return false;   

    // We have a control transfer
    switch (pRTL->getKind())
    {
        case JUMP_HRTL:
        {
            // Emit goto Ldest;
            const HLJump* jump = static_cast<const HLJump*>(pRTL);
            // Check if fixed dest
            if (jump->getDest()->getFirstIdx() == idIntConst) {
                int label = pBB->getOutEdges()[0]->getLabel();
                os << "\tgoto L" << dec << label << ";\n";
            }
            else {
                // Will call interpreter when ready
                ostrstream ost;
                ost << "*** Register jump at ";
                ost << hex << pRTL->getAddress();
                ost << "! PROGRAM WILL NOT RUN! ***";
                error(str(ost));
                prog.bRegisterJump = true;
            }
            break;
        }
        case NWAYJUMP_HRTL:
        {
            const HLNwayJump* jump = static_cast<const HLNwayJump*>(pRTL);
            SWITCH_INFO* si = jump->getSwitchInfo();
            if (si != NULL) {
                // We have a switch statement. Iterate through the out edges
                // for the cases
                const vector<PBB>& outs = pBB->getOutEdges();
                Type cType = si->pSwitchVar->getType();
                os << "\tswitch (" << processExpr(si->pSwitchVar, cType)
                    << ") {";
                if (si->chForm == 'H') {
                    os << "\t\t/* EDIT ME! (form H switch) */\n";
                    // Special case. si->iNumTable has the number of entries
                    // in the table
                    int* pTable = (int*)(si->uTable + si->delta);
                    for (int i=0, j=0, k=0; i < si->iNumTable; i++, j+=2) {
                        if (pTable[j] == -1) continue;
                        // Want case 7-2: (where 7 is i, and 2 is iLower)
                        os << "\t\tcase " << dec << pTable[j] + si->iLower;
                        os << " - " << si->iLower;
                        os << ": goto L" << dec << outs[k++]->getLabel();
                        os << ";\n";
                    }
                }
                else {
                    os << endl;
                    for (int i=0; i <= si->iUpper-si->iLower; i++) {
                        // Want case 7-2: (where 7 is i, and 2 is iLower)
                        os << "\t\tcase " << dec << si->iLower + i;
//                      os << " - " << si->iLower;
                        os << ": goto L" << dec << outs[i]->getLabel() << ";\n";
                    }
                }
                os << "\t}\n";
            }
            else {
                // Emit _interpreter(expr);
                os << "\n_interpreter(" << processExpr(jump->getDest(),
                    jump->getDest()->getType());
                os << ");\n";
            }
            break;
        }

        case JCOND_HRTL:
        {
            const HLJcond* jcond = static_cast<const HLJcond*>(pRTL);

            // First emit the regular semantics (if any) (e.g. pa-risc ADDB)
            int n = pRTL->getNumRT();
            for (int i=0; i<n; i++) {
                if (processRT(pRTL->elementAt(i))) {
                    ostrstream ost;
                    ost << "Above error is at " << hex << pRTL->getAddress()
                      << endl;
                    error(str(ost));
                }
            }

            // Emit if (logicexpr) goto Ldest;
            SemStr* pCond = jcond->getCondExpr();
            if (pCond == 0) {
                ostrstream ost;
                ost << "HLJcond at " << hex << uAddr <<
                    " has no high level condition";
                error(str(ost));
                break;      // For now... was assert
            }
            os << "\tif (";
            Type condType(pCond->getType());
            switch (jcond->getCond()) {
                case HLJCOND_JUL: case HLJCOND_JULE:
                case HLJCOND_JUGE: case HLJCOND_JUG:
//cout << "unsigned Jcond is "; jcond->print(cout); cout << "; pcond is "; pCond->print(cout); cout << endl;
                    condType.setSigned(false);
                    break;
                default:
                    break;          // Leave it alone
            }
            os << processExpr(pCond, condType);
            int label = pBB->getOutEdges()[0]->getLabel();
            os << ") goto L" << dec << label << ";\n";
            // If the "fall through" outedge doesn't really fall through, the
            // "jump reqd" bit should be set, and we must generate a jump
            if (pBB->isJumpReqd()) {
                // Emit goto Ldest;
                label = pBB->getOutEdges()[1]->getLabel();
                os << "\tgoto L" << dec << label << ";\n";
            }
            break;
        }

        case RET_HRTL:
        {
            // Emit return [ location ];
            int n = pRTL->getNumRT();
            if (n) {
                // cout << "Return has semantics: "; pRTL->print();
                for (int i=0; i < n; i++) {
                    // Process each RT
                    processRT(pRTL->elementAt(i));
                }
            }
            emitReturn(proc);
            break;
        }

        case CALL_HRTL:
        {
            // Check if the call has semantics. This can happen with certain
            // patterns, e.g. move_x_call_move
            int n = pRTL->getNumRT();
            if (n) {
                // cout << "Call has semantics: "; pRTL->print();
                for (int i=0; i < n; i++) {
                    // Process each RT
                    processRT(pRTL->elementAt(i));
                }
            }
            const HLCall* pCall = static_cast<const HLCall*>(pRTL);
            ADDRESS uDest = pCall->getFixedDest();

            Proc* destProc = prog.findProc(uDest);
            // Very rarely, the name of the dest will be forced
            const char* pName = pCall->getDestName();
            if ((uDest == NO_ADDRESS) && (pName == NULL)) {
                // Register call
                prog.bRegisterCall = true;
                // Emit an assignment to _uqbt_nat for the expression
                // It will be a 32 bit unsigned assignment
                os << "\t_uqbt_nat=" <<
                  processExpr(pCall->getDest(), Type(INTEGER, 32, false)) <<
                  ";\n";
                // Now emit retval = __reg_call(params)
                pName = "__reg_call";
            }
            else if ((pName == NULL) &&
              ((destProc == 0 || (destProc == (Proc*)-1) ||
              (pName = destProc->getName(),pName == 0)))) {
                ostrstream ost;
                ost << "Call to unnamed function at ";
                ost << hex << pCall->getAddress();
                error(str(ost));
                // Put *something* into the file, for debugging
                os << "/* Call to unnamed function at ";
                os << hex << pCall->getAddress() << " */\n"; 
                break;
            }
            os << "\t";
            const SemStr retLoc = pCall->getReturnLoc();
            if (retLoc.len() != 0)
                os << processExpr(&retLoc, retLoc.getType()) << "=";
            os << legalCname(pName) << "(";
            const list<SemStr>& params = pCall->getParams();
            // Get a list of types to cast the parameters to, if possible
            list<Type>* types;
            list<Type>::iterator cc;
            if (destProc) {
                types = destProc->getParamTypeList(params);
                cc = types->begin();
            }
            list<SemStr>::const_iterator pp, qq;
            bool first = true;
            for (pp = params.begin(); pp != params.end(); pp++) {
                // If necessary, cast the actual parameter to what the formal
                // parameter is expecting. Temporarily needed to solve the
                // "7th parameter is float" problem
                Type castTo;
                if (destProc) castTo = *cc++;
                else castTo = pp->getType();
                // processExpr is supposed to cast registers etc if required now
                // If call emitCast here, we often get two HD casts!
                // if (destProc) emitCast(castTo, pp->getType(), os);

                if (!first) os << ",";
                // Special cases: overlapping parameters
                if ((castTo == Type(FLOATP, 64)) &&
                  // Remember: Type::operator-= is sign insensitive == !
                  (pp->getType() -= Type(INTEGER, 32)) &&
                  (pp != params.end()) &&
                  (qq = pp, (*++qq).getType() -= Type(INTEGER, 32))) {
                    // Two int actuals, one double formal. Use ints2flt()
                    os << "ints2float("; pp->print(os);
                    os << ",";           qq->print(os); os << ")";
                    pp++;   // Skip the second int
                } else {
                    os << processExpr(&(*pp), castTo);
                }
                first = false;
            }
            if (destProc) delete types;
            os << ");\n";

            // Could have post call semantics
            const list<RT*>* lrt = pCall->getPostCallRtlist();
            if (lrt) {
                for (RT_CIT it = lrt->begin(); it != lrt->end(); it++)
                    processRT(*it);
            }

            // Could be a call/restore
            if (pBB->isJumpReqd()) {
                // Emit goto Ldest;
                int label = pBB->getOutEdges()[0]->getLabel();
                os << "\tgoto L" << dec << label << ";\n";
            }

            break;
        }

        case SCOND_HRTL:
        {
            const HLScond* pCond = static_cast<const HLScond*>(pRTL);
            os << "\t";
            os << processExpr(pCond->getDest(), pCond->getDest()->getType());
            os << "=(";
            os << processExpr(pCond->getCondExpr(),
                pCond->getCondExpr()->getType());
            os << ");\n";
            break;
        }
            
        default:
            error("Other HL RTL\n");
            break;
    }
    return true;

}

/*==============================================================================
 * FUNCTION:    emitBBCode
 * OVERVIEW:    emits the code for this BB
 * NOTE:        Both the high level and low level code calls this procedure
 * PARAMETERS:  pbb: pointer to the BB
 *              proc: this procedure
 *              nest: Number of additional tabs at the start of each line
 *              allJumps: if false, don't process JCOND_HRTLs and JUMP_HRTLS
 * RETURNS:     <Nothing>
 *============================================================================*/
void Translate::emitBBCode(PBB pBB, UserProc* proc, int nest /* = 0 */,
    bool allJumps /* = true */) 
{
    int n;
    
    if (pBB->getLowAddr()) {
        os << endl;
        for (n=0; n<nest; n++)
            os << "\t";
        os << "\t/* " << hex << pBB->getLowAddr() << " */" << endl;
    }

    HRTLList_CIT it;
    for (it = pBB->getHRTLs()->begin(); it != pBB->getHRTLs()->end(); it++) {
        // Set the address for error messages
        uAddr = (*it)->getAddress();
        if (!allJumps &&
          ((*it)->getKind()==JCOND_HRTL || (*it)->getKind()==JUMP_HRTL))
            continue;
        bool needLabel = false;
        if (processHL(*it, pBB, proc))
            continue;   // High level RTL already handled
        // Otherwise, iterate through the RTs this RTL
        if ((*it)->getCommented())
            os << "/*";
        int n1 = (*it)->getNumRT();
        // Special check for assignments to %SKIP or %RPT
        // These will need a label here. Needed for repeated instruct-
        // ions, e.g. Pentium string instructions
        int i; 
        for (i=0; i < n1; i++) {
            RTAssgn* curr = (RTAssgn*)((*it)->elementAt(n1-1));
            if (curr->getKind() == RTASSGN) {
                SemStr* lhs = curr->getLHS();
                if ((lhs->getFirstIdx() == idSKIP) ||
                  (lhs->getFirstIdx() == idRPT))
                    // We need a label here
                    needLabel = true;
            }
        }
        if (needLabel)
            // First label before the instruction proper
            os << "label" << ++labelNum << ":\n";
        for (i=0; i < n1; i++) {
            for (n=0; n<nest; n++)
                os << "\t";
            // Process each RT
            if (processRT((*it)->elementAt(i))) {
                ostrstream ost;
                ost << "Above error is at " << hex << (*it)->getAddress()
                  << endl;
                error(str(ost));
            }
        }
        if (needLabel) {
            // As well as a label at the start, we need a label at the
            // end. During the string instruction, it will be assumed
            // that labelNum is the label at the start, and labelNum+1
            // is the label past the end
            os << "label" << ++labelNum << ":\n";
        }
        if ((*it)->getCommented())
            os << "*/\n";
    }
}

/*==============================================================================
 * FUNCTION:    EmitBBCond
 * OVERVIEW:    Emits the condition contained in this BB (BB must be a TWOWAY)
 * PARAMETERS:  pbb: pointer to the basic block, proc: this procedure, 
 *      negate: set to true to negate the condition
 * RETURNS: <Nothing>
 *============================================================================*/
void Translate::EmitBBCond(UserProc*proc,PBB pbb, bool negate)
{
    const HLJcond* jcond = static_cast<const HLJcond*>(pbb->getHRTLs()->back());
    SemStr* pCond = jcond->getCondExpr();
    if (pCond == 0) {
        ostrstream ost;
        ost << "HLJcond at " << hex << uAddr <<
            " has no high level condition";
        error(str(ost));
        exit(0);
    }
    Type condType(pCond->getType());
    switch (jcond->getCond()) {
    case HLJCOND_JUL: case HLJCOND_JULE:
    case HLJCOND_JUGE: case HLJCOND_JUG:
        condType.setSigned(false);
        break;
    default:
        break;          // Leave it alone
    }
    if (negate) os << "!(";
    os << processExpr(pCond, condType); 
    if (negate) os << ")";
}

/*==============================================================================
 * FUNCTION:    ProcessBB
 * OVERVIEW:    Recursively processes the control flow graph, with a view
 *                to emitting high level, structured C code
 * PARAMETERS:  pbb: pointer to the current basic block, proc: this procedure
 * RETURNS:     <Nothing>
 *============================================================================*/
void Translate::ProcessBB(UserProc* proc,PBB pbb,PBB stopon,PBB breakto,PBB continueto,bool ignoreheader) 
{
    if (pbb==breakto)
    {
        os << "break;" << endl;
        return;
    }

    if (pbb==stopon)
        return;

    if (pbb==continueto && !ignoreheader)
    {
        os << "continue;" << endl;
        return;
    }

    if (pbb->m_header && !ignoreheader)
    {
        PBB latch = pbb->m_listLatchNodes.front();
        cout << "header " << hex << pbb->getLowAddr() << " has latch " << latch->getLowAddr() << dec << endl;
        BITSET commonpost = pbb->getPostDominatorSet();
        commonpost &= latch->getPostDominatorSet();
        PBB follow=NULL;
        for (int bi = pbb->m_index-1; bi >= 0; bi--)
            if (commonpost.test(bi)) 
            {
                follow=proc->getCFG()->m_vectorBB[bi];
        if (follow==latch || follow==pbb) continue;
                cout << "follow of " << hex << pbb->getLowAddr() << "(" << dec << pbb->m_last << ")";
                cout << " is " << hex << follow->getLowAddr() << "(" << dec << follow->m_last << ")" << dec << endl;
                cout << "outedges: " << hex << pbb->getOutEdges().front()->getLowAddr() << "(" << dec << pbb->getOutEdges().front()->m_last << ")" << ", ";
                if (pbb->getOutEdges().size()>1)
                {
                    cout << hex << pbb->getOutEdges().back()->getLowAddr() << "(" << dec << pbb->getOutEdges().back()->m_last << ")" << endl;
                }
                break;
            }
        if (latch->getType()==TWOWAY) 
        {
            if (latch->getOutEdges().front() == follow)
            {
                os << "do {" << endl;
                pbb->m_header=false;
                ProcessBB(proc,pbb,latch,follow,latch,true);
                emitBBCode(latch,proc,1,false);
                os << "\t} while (";
                EmitBBCond(proc,latch,true);
                os << ");" << endl;
                ProcessBB(proc,follow,stopon,breakto,continueto);
                return;
            }
            if (latch->getOutEdges().back() == follow)
            {
                os << "do {" << endl;
                pbb->m_header=false;
                ProcessBB(proc,pbb,latch,follow,latch,true);
                emitBBCode(latch,proc,1,false);
                os << "\t} while (";
                EmitBBCond(proc,latch,false);
                os << ");" << endl;
                ProcessBB(proc,follow,stopon,breakto,continueto);
                return;
            }                       
        }
        if (pbb->getType()==TWOWAY)
        {
            if (pbb->getOutEdges().front() == follow)
            {
                emitBBCode(pbb,proc,1,false);
        // code for pbb must be added to the latch
        for (HRTLList::iterator sp1 = pbb->getHRTLs()->begin();
            sp1 != pbb->getHRTLs()->end(); sp1++)
            latch->getHRTLs()->push_back(*sp1);
                os << "while(";
                EmitBBCond(proc,pbb,true);
                os << ") {" << endl;
                ProcessBB(proc,pbb->getOutEdges().back(),latch,follow,pbb);
                emitBBCode(latch,proc,1,false);
                os << "\t}" << endl;
                ProcessBB(proc,follow,stopon,breakto,continueto);
                return;
            }
            if (pbb->getOutEdges().back() == follow)
            {
                emitBBCode(pbb,proc,1,false);
        for (HRTLList::iterator sp1 = pbb->getHRTLs()->begin();
            sp1 != pbb->getHRTLs()->end(); sp1++)
            latch->getHRTLs()->push_back(*sp1);
                os << "while (";
                EmitBBCond(proc,pbb,false);
                os << ") {" << endl;
                ProcessBB(proc,pbb->getOutEdges().front(),latch,follow,pbb);
                emitBBCode(latch,proc,1,false);
                os << "\t}" << endl;
                ProcessBB(proc,follow,stopon,breakto,continueto);
                return;
            }
        }
        os << "\t/* " << hex << pbb->getLowAddr() << " */" << endl;
        os << "\tfor (;;) {" << endl;
        ProcessBB(proc,pbb,latch,follow,pbb,true);
        os << "\t}" << endl;
        ProcessBB(proc,follow,stopon,breakto,continueto);
        return;     
    }

    if (pbb->getType() == ONEWAY || pbb->getType() == FALL || pbb->getType() == CALL)
    {
        emitBBCode(pbb,proc,1,false);
        ProcessBB(proc,pbb->getOutEdges().front(),stopon,breakto,continueto);
        return;
    }

    if (pbb->getType() == TWOWAY)
    {
        if (pbb->getOutEdges().front()->getPostDominatorSet().test(pbb->getOutEdges().back()->m_index))
        {
            emitBBCode(pbb,proc,1,false);
            os << "if (";
            EmitBBCond(proc,pbb,false);
            os << ") {" << endl;
            ProcessBB(proc,pbb->getOutEdges().front(),pbb->getOutEdges().back(),breakto,continueto);
            os << "}" << endl;
            ProcessBB(proc,pbb->getOutEdges().back(),stopon,breakto,continueto);
            return;
        }
        if (pbb->getOutEdges().back()->getPostDominatorSet().test(pbb->getOutEdges().front()->m_index))
        {
            emitBBCode(pbb,proc,1,false);
            os << "if (";
            EmitBBCond(proc,pbb,true);
            os << ") {" << endl;
            ProcessBB(proc,pbb->getOutEdges().back(),pbb->getOutEdges().front(),breakto,continueto);
            os << "}" << endl;
            ProcessBB(proc,pbb->getOutEdges().front(),stopon,breakto,continueto);
            return;
        }
        PBB follow=NULL;
        for (int bi = pbb->m_index-1; bi >= 0; bi--)
            if (pbb->getPostDominatorSet().test(bi)) 
            {
                follow=proc->getCFG()->m_vectorBB[bi];
                break;
            }
        emitBBCode(pbb,proc,1,false);
        os << "if (";
        EmitBBCond(proc,pbb,false);
        os << ") {" << endl;
        ProcessBB(proc,pbb->getOutEdges().front(),follow,breakto,continueto);
        os << "} else {" << endl;
        ProcessBB(proc,pbb->getOutEdges().back(),follow,breakto,continueto);
        os << "}" << endl;      
        ProcessBB(proc,follow,stopon,breakto,continueto);
        return;
    }

    if (pbb->getType() == RET)
    {
        emitReturn(proc);
        return;
    }

    cout << "panic, unhandled BB type in ProcessBB " << dec <<
        (int)pbb->getType() << endl;
    exit(0);    

}

/*==============================================================================
 * FUNCTION:    ProcessBB
 * OVERVIEW:    Recursively processes the control flow graph, generating
 *                high level C
 * PARAMETERS:  pbb: pointer to the current basic block, proc: this procedure
 * RETURNS:     <Nothing>
 *============================================================================*/
void Translate::ProcessBB(PBB pbb,UserProc* proc,int nest,bool followok) 
{
    vector<PBB> outedges = pbb->getOutEdges();
    int n;

    if (pbb->m_follow && !followok) 
    {
        os << endl;
        for (n=0; n<nest; n++)
            os << "\t";
        os << "}" << endl;
        return;
    }       
    
    if (pbb->getType()==TWOWAY) 
    {
        
            const HLJcond* jcond = static_cast<const HLJcond*>(pbb->getHRTLs()->back());
            SemStr* pCond = jcond->getCondExpr();
            if (pCond == 0) { 
                ostrstream ost;
                    ost << "HLJcond at " << hex << uAddr <<
                        " has no high level condition";
                    error(str(ost));
                    exit(0);
            }
            Type condType(pCond->getType());
            switch (jcond->getCond()) {
            case HLJCOND_JUL: case HLJCOND_JULE:
            case HLJCOND_JUGE: case HLJCOND_JUG:
                condType.setSigned(false);
                break;
            default:
                    break;          // Leave it alone
            }
        if (pbb->m_header)
        {
            if (pbb->m_listLatchNodes.front()->getType() == TWOWAY)
            {
                // this is actually a do while loop, the first statement of which is an if.
                os << endl;
                for (n=0; n<nest; n++)
                    os << "\t";
                os << "\tdo" << endl;
                for (n=0; n<nest; n++)
                    os << "\t";
                os << "\t{" << endl;
                pbb->m_header=false;
                ProcessBB(pbb,proc,nest+1);
                return;
            }
            if (pbb->m_followNode==NULL)
            {
                cout << "eep, follow node null in while loop at " << hex << pbb->getLowAddr() << dec << endl;
                exit(0);
            }
            // while loop
            emitBBCode(pbb,proc,nest,false);
            os << endl;
            for (n=0; n<nest; n++)
                os << "\t";
            os << "\twhile(";
            if (outedges.front()==pbb->m_followNode)
                os << "!(";
                    os << processExpr(pCond, condType);
            if (outedges.front()==pbb->m_followNode)
                os << ")";
            os << ")" << endl;
            for (n=0; n<nest; n++)
                os << "\t";
            os << "\t{ /* follow = " << hex << pbb->m_followNode->getLowAddr() << dec << "*/" << endl;

            if (outedges.front()==pbb->m_followNode)
                ProcessBB(outedges.back(),proc,nest+1);
            else
                ProcessBB(outedges.front(),proc,nest+1);
            ProcessBB(pbb->m_followNode,proc,nest,true);
        } else
        if (pbb->m_latch)
        {           
            if (pbb->m_followNode==NULL)
            {
                cout << "eep, follow node null in while of do while loop at " << hex << pbb->getLowAddr() << dec << endl;
                exit(0);
            }
            // do while loop
            emitBBCode(pbb,proc,nest,false);
            os << endl;
            for (n=0; n<nest; n++)
                os << "\t";
            os << "} while(";
                    os << processExpr(pCond, condType);
            os << "); /* follow = " << hex << pbb->m_followNode->getLowAddr() << dec << " */" << endl;
            ProcessBB(pbb->m_followNode,proc,nest-1,true);
        } else {
            if (pbb->m_followNode==NULL)
            {               
                cout << "unstructured if statement at " << hex << pbb->getLowAddr() << dec << endl;
                if (!pbb->getOutEdges().front()->isDominatedBy(pbb) && 
                     pbb->getOutEdges().front()->m_follow) {
                    emitBBCode(pbb,proc,nest,false);
                    os << endl;
                    for (n=0; n<nest; n++)
                        os << "\t";
                    os << "\tif (";
                    os << processExpr(pCond, condType);
                    os << ") break;" << endl;
                    ProcessBB(pbb->getOutEdges().back(),proc,nest,false);
                } else if (!pbb->getOutEdges().back()->isDominatedBy(pbb) &&
                        pbb->getOutEdges().back()->m_follow) {  
                    emitBBCode(pbb,proc,nest,false);
                    os << endl;
                    for (n=0; n<nest; n++)
                        os << "\t";
                    os << "\tif (!(";
                    os << processExpr(pCond, condType);
                    os << ")) break;" << endl;
                    ProcessBB(pbb->getOutEdges().front(),proc,nest,false);
                } else {
                    cout << "unrecognised" << endl;
                    exit(0);
                }                                               
            } else {
                emitBBCode(pbb,proc,nest,false);
                os << endl;
                for (n=0; n<nest; n++)
                    os << "\t";
                os << "\tif (";
                if (outedges.front()==pbb->m_followNode)
                    os << "!(";
                os << processExpr(pCond, condType);
                if (outedges.front()==pbb->m_followNode)
                    os << ")";
                os << ")" << endl;
                for (n=0; n<nest; n++)
                    os << "\t";
                os << "\t{ /* follow = " << hex << pbb->m_followNode->getLowAddr() << dec << "*/" << endl;
                if (outedges.front()!=pbb->m_followNode)
                    ProcessBB(outedges.front(),proc,nest+1);
                if (outedges.back()!=pbb->m_followNode)
                {
                    if (outedges.front()!=pbb->m_followNode)
                    {
                        os << endl;
                        for (n=0; n<nest; n++)
                            os << "\t";
                        os << "\telse {" << endl;
                    }
                    ProcessBB(outedges.back(),proc,nest+1);
                }
                ProcessBB(pbb->m_followNode,proc,nest,true);
            }
        }       
    } else {
        if (pbb->m_header) {
            os << endl;
            for (n=0; n<nest; n++)
                os << "\t";
            os << "\tdo" << endl;
            for (n=0; n<nest; n++)
                os << "\t";
            os << "\t{" << endl;
            emitBBCode(pbb,proc,nest+1,false);
            ProcessBB(outedges.back(),proc,nest+1);
        } else 
        if (pbb->m_latch) {
            emitBBCode(pbb,proc,nest,false);

            // note: we have to duplicate code iff there is more than a jcond in
            // this BB.  Dataflow analysis should get rid of this!
            emitBBCode(pbb->m_headerNode,proc,nest,false);  
            os << endl;
            for (n=0; n<nest; n++)
                os << "\t";
            os << "}" << endl;
        } else {
            emitBBCode(pbb,proc,nest,false);
            if (!outedges.empty())
                ProcessBB(outedges.front(),proc,nest);
        }
    }
}


/*==============================================================================
 * FUNCTION:    StructuredEmit
 * OVERVIEW:    Emits structured C code for this procedure
 * PARAMETERS:  proc: pointer to this procedure's Proc object
 * RETURNS:     <Nothing>
 *============================================================================*/
void Translate::StructuredEmit(UserProc* proc) 
{
    proc->getCFG()->sortByLastDFS();
    proc->getCFG()->updateVectorBB();
    ProcessBB(proc,proc->getEntryBB(),NULL,NULL,NULL);
    proc->getCFG()->sortByFirstDFS(); // make entryBB first
}

/*==============================================================================
 * FUNCTION:    translate2c
 * OVERVIEW:    This is the main back end function. Emits the C code (low or
 *                high level) for this procedure
 * PARAMETERS:  proc: pointer to this procedure's Proc object
 *              spec: if true, this is a speculative translation
 * RETURNS:     <Nothing>
 *============================================================================*/
void Translate::translate2c(UserProc* proc, bool spec)
{
    if (proc->getEntryBB() == NULL)
        return;             // Don't attempt to generate code

    if (progOptions.proctrace)
        cout << "Back end for proc " << proc->getName() << endl;

    // Initilise the member variables
    maxVar = 0;
    regNumbers.clear();
    // Need to empty os here!!
    temps.clear();
    Cfg* pCfg = proc->getCFG();
    useSwap2 = false; useSwap4 = false; useSwap8 = false;
    this->proc = proc;
    ovl.init();         // Initialise the Overlap object

    // Count the number of registers used. A complete pass through the
    // RTLs of this proc is needed for this
    firstPass();
    // Use this information to initialise the Overlap object
    ovl.setUsed(regNumbers);

    // First need the name of the output (.c) file
    string Name = progOptions.outDir + proc->getName() + string(".c");
    of.open(Name.c_str());
    if (!of)
    {
        ostrstream ost;
        ost << "Could not open " << Name << " for writing";
        error(str(ost));
    }

    // Choose standard or high level emitting code
    if (progOptions.highLevelC) {
        StructuredEmit(proc);
        emitFunc(proc, spec);
        return;
    }

    // This is the standard (very low level C) code emitter
    proc->getCFG()->sortByAddress();

    BB_CIT bbit;
    PBB pBB = pCfg->getFirstBB(bbit);       // Get the BB for the first BB
    // If the first BB is not the entry point, put a label at the real entry BB
    // Note: using -S, it is possible for a proc to have no BBs, so pBB may be
    // NULL here
    if (pBB && (proc->getNativeAddress() != pBB->getLowAddr()))
        pCfg->setLabel(proc->getEntryBB());

    while (pBB)
    {
        int label = pBB->getLabel();
        if (label)
            // Emit a label for the start of this BB
            os << "L" << dec << label << ":";

        // This code is common with the high level code emitter
        emitBBCode(pBB, proc);

        if ((pBB->getType() == FALL) && (pBB->isJumpReqd())) {
            // Other types handled elsewhere. Problem is knowing which outedge
            // is the fallthrough one
            int label = pBB->getOutEdges()[0]->getLabel();
            os << "\tgoto L" << dec << label << ";\n";
        }
        pBB = pCfg->getNextBB(bbit);
    }
    emitFunc(proc, spec);
}

/*==============================================================================
 * FUNCTION:    isString
 * OVERVIEW:    Check to see if this pc-relative address appears to be a text
 *                string
 * PARAMETERS:  p: host address of candidate string
 *              last: host address of end of code section
 * RETURNS:     Pointer to actual string if found; NULL if not
 *============================================================================*/
bool Translate::isString(const char* p, const char* last)
{
    int len = 0, punct = 0;
    bool good = false;
    while ((len < 100) && (p < last)) {
        if (*p == '\0') {
            good = true;
            break;
        }
        if (!isalnum(*p++))
            punct++;
        len++;
    }
    if (!good) return false;
    if (len == 0) return false;
    if (punct*3 / len) return false;     // More than 2/3 non alpha numeric
    // OK, it's a string
    return true;
}

/*==============================================================================
 * FUNCTION:    emitCast
 * OVERVIEW:    Emit a cast if required. Use a lightweight cast if possible
 *                (e.g. (unsigned)e) or a heavy duty cast if needed (e.g.
 *                *(unsigned short*)&e).
 * PARAMETERS:  cType: reference to the current type
 *              eType: reference to type of the expression we are casting from
 *              ost: ostrstream object to stream the cast to
 * RETURNS:     True if a cast has been emitted
 *============================================================================*/
bool Translate::emitCast(const Type& cType, const Type& eType, ostrstream& ost)
{
    // Check if a cast needed at all. Use the comparison operator that considers
    // all floats > 64 bits to be the same size (*=)
    if (!(eType *= cType)) {
        // See if the basic types are the same, and there is no size greater
        // than an integer. If so, a light-duty cast is all that is required
#if 1
        int intSize = prog.csrSrc.getIntSize() * 8;
        if ((cType.getType() == eType.getType()) && (cType.getSize() <= intSize)
          && (eType.getSize() <= intSize)) {
            // Light duty cast is all that is needed
            ost << "(" << cType.getCtype() << ")";
            return true;
        }
#endif
        // Change of basic type (and we don't want conversions):
        // Heavy duty cast required
        // Note that heavy duty casts don't work when there is a size
        // change and the target is big endian.
        // For example, *(char*)&r8 with r8=5 produces 0 when compiling
        // on a sparc! (Address is of the left end, which is 0)
if (cType.getSize() != eType.getSize()) {
  ostrstream ost;
  ost << "casting from type " << eType.getCtype() << " to type " <<
    cType.getCtype() << " will not work with endianness differences";
  error(str(ost));
}
        ost << "*(" << cType.getCtype() << "*)&";
        return true;
    }
    return false;
}

/*
 * Small proc to check for registers; same code needed to check left and
 * right hand sides
 */
void Translate::checkRegs(SemStr* ss)
{
    list<SemStr*> result;
    if (ss->searchAll(regX, result)) {
        // We have at least one register; go through the list
        list<SemStr*>::iterator it;
        for (it=result.begin(); it != result.end(); it++)
            regNumbers.insert((*it)->getThirdIdx());
    }
#if 0       // MVE: I believe that this is now done in UserProc::checkMemSizes()
    result.clear();
    if (ss->searchAll(varX, result)) {
        // We have at least one var; go through the list
        int size = ss->getType().getSize();
        list<SemStr*>::iterator it;
        for (it=result.begin(); it != result.end(); it++) {
            int vNum = (*it)->getSecondIdx();
            if (size > proc->getVarType(vNum).getSize())
                // Change to the larger size
                proc->setVarSize(vNum, size);
        }
    }
#endif
}

/*==============================================================================
 * FUNCTION:        Translate::firstPass
 * OVERVIEW:        This searches through all RTLs in the proc, looking for
 *                    SemStrs with r[ int X; if found, inserts X into the
 *                    set of used registers (regNumbers)
 *                    Also searches through all RTLs in the proc, looking for
 *                    SemStrs with IDs >= idMachSpec. If so, adds the index to
 *                    the set stIdx
 * NOTE:            Assumes that expressions of the form r[6+1] have been
 *                    changed to r[7]
 * PARAMETERS:      Reference to a set of INDEXes which are >= idMachSpec
 * RETURNS:         Nothing, but set parameter may be added to
 *============================================================================*/
void Translate::firstPass()
{
    Cfg* cfg = proc->getCFG();
    BB_CIT it;
    PBB pBB = cfg->getFirstBB(it);
    while (pBB) {
        HRTLList* pRtls = pBB->getHRTLs();
        if (pRtls) {
            HRTLList_IT rit;
            for (rit = pRtls->begin(); rit != pRtls->end(); rit++) {
                int n = (*rit)->getNumRT();
                for (int i=0; i<n; i++) {
                    RTAssgn* rt = (RTAssgn*)(*rit)->elementAt(i);
                    if (rt->getKind() != RTASSGN) continue;
                    checkRegs(rt->getLHS());
                    checkRegs(rt->getRHS());
                    rt->getLHS()->searchMachSpec(stIdx);
                    rt->getRHS()->searchMachSpec(stIdx);
                }
                HRTL_KIND kd = (*rit)->getKind();
                if (kd == CALL_HRTL) {
                    // Also count return location, if any
                    HLCall* hl = (HLCall*)*rit;
                    SemStr retLoc = hl->getReturnLoc();
                    checkRegs(&retLoc);
                    // Also count parameters; it's possible that some are
                    // not used elsewhere
                    list<SemStr>& pv = hl->getParams();
                    list<SemStr>::iterator pp;
                    for (pp = pv.begin(); pp != pv.end(); pp++)
                        checkRegs(&*pp);
                }
                else if (kd == RET_HRTL) {
                    const SemStr* rl = proc->getReturnLoc();
                    // Need const cast because checkRegs can't be made const,
                    // because searchAll isn't const
                    checkRegs(const_cast<SemStr*>(rl));
                }
                // Other HL types may have to be considered in the future
            }
        }
        pBB = cfg->getNextBB(it);
    }
}
