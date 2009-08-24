/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:        sparcnjexp.cc
 * OVERVIEW:    Implementation of the Sparc instance of the Expander class.
 *              All Sparc-dependent aspects of the code expansion and
 *              generation are stored here.
 *
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.7 $
 * 13 Apr 01 - Manel: Initial creation.
 * 16 Apr 01 - Brian: Suppress unused variable warning.
 * 16 Apr 01 - Brian: Implemented integer division and signed integer
 *              arithmetic.
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 */

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "global.h"
#include "rtl.h"
#include "cfg.h"
#include "proc.h"
#include "prog.h"
#include "sparcnjexp.h"
#include "options.h"
#include "ElfBinaryFile.h"

/*==============================================================================
 * Sparc dependent macros
 *============================================================================*/

// Special Sparc registers
#define TGT_ZERO        0       // Zero
#define TGT_RES         8       // Result
#define TGT_SP          14      // Stack pointer
#define TGT_RA          15      // Return address
#define TGT_FP          30      // Frame pointer

// Sparc global registers
#define TGT_GLOBAL      0
#define TGT_G0          TGT_GLOBAL
#define TGT_G7          (TGT_G0 + 7)
#define TGT_NUM_G       (TGT_G7 - TGT_G0 + 1)
#define IS_TGT_G(x)     ((x) >= TGT_G0 && (x) <= TGT_G7)

// Sparc output registers
#define TGT_OUTPUT      8
#define TGT_O0          TGT_OUTPUT
#define TGT_O5          (TGT_O0 + 5)
#define TGT_NUM_O       (TGT_O5 - TGT_O0 + 1)
#define TGT_O(x)        (TGT_O0 + (x))
#define IS_TGT_O(x)     ((x) >= TGT_O0 && (x) <= TGT_O5)

// Sparc local registers
#define TGT_LOCAL       16
#define TGT_L0          TGT_LOCAL
#define TGT_L7          (TGT_L0 + 7)
#define TGT_NUM_L       (TGT_L7 - TGT_L0 + 1)
#define TGT_L(x)        (TGT_L0 + (x))
#define IS_TGT_L(x)     ((x) >= TGT_L0 && (x) <= TGT_L7)

// Sparc input registers
#define TGT_INPUT       24
#define TGT_I0          TGT_INPUT
#define TGT_I5          (TGT_I0 + 5)
#define TGT_NUM_I       (TGT_I5 - TGT_I0 + 1)
#define TGT_I(x)        (TGT_I0 + (x))
#define IS_TGT_I(x)     ((x) >= TGT_I0 && (x) <= TGT_I5)

// Source machine registers
#define SRC_R0          128
#define SRC_R127        (SRC_R0 + 127)
#define SRC_NUM_R       (SRC_R127 - SRC_R0 + 1)
#define SRC_R(x)        (SRC_R0 + (x))
#define IS_SRC_R(x)     ((x) >= SRC_R0 && (x) <= SRC_R127)

// Symbolic registers (temporal)
#define SYM_V0          (SRC_R127 + 1)
#define SYM_V127        (SYM_V0 + 127)
#define SYM_NUM_V       (SYM_V127 - SYM_V0 + 1)
#define SYM_V(x)        (SYM_V0 + (x))
#define IS_SYM_V(x)     ((x) >= SYM_V0 && (x) <= SYM_V127)

// Special registers (condition codes, etc.)
#define SPC_R0          (SYM_V127 + 1)
#define SPC_R127        (SPC_R0 + 127)
#define SPC_NUM_R       (SPC_R127 - SPC_R0 + 1)
#define SPC_R(x)        (SPC_R0 + (x) - idMachSpec)
#define IS_SPC_R(x)     ((x) >= SPC_R0 && (x) <= SPC_R127)

// Sparc parameters past the sixth (stack placed)
#define SPC_O0          (SPC_R127 + 1)
#define SPC_O127        (SPC_O0 + 127)
#define SPC_NUM_O       (SPC_O127 - SPC_O0 + 1)
#define SPC_O(x)        (SPC_O0 + (x))
#define IS_SPC_O(x)     ((x) >= SPC_O0 && (x) <= SPC_O127)
#define IS_OUTPUT(x)    (IS_TGT_O(x) || IS_SPC_O(x))
#define OUTPUT_O(x)     ((x) < TGT_NUM_O ? TGT_O0+(x) : SPC_O((x)-TGT_NUM_O))

// Special cases
#define SPC_NONE        1024    // Do nothing
#define SPC_CNTN        1025    // Constant
#define SPC_MEM         1026    // Raw memory
#define SPC_AFP         1027    // Local variables
#define SPC_TMP         1028    // Temporal variables
#define SPC_ADR         1029    // Effective address

// Other definitions
#define SPARC_WORD_SZ   4       // Sparc v8 word size

/*==============================================================================
 * Global type and data declarations.
 *============================================================================*/

// Encoder procedures from NJMCTK
extern "C" {
#include "../backend/po/sparc-encoder.h"
}

// External struct containing all the encoder procedures
extern "C" struct sparc_enc_t *sparc_enc;

// Local pointer to the struct
static struct sparc_enc_t *penc;

// Shorcuts
#define RMODE(x)    penc->rmode(x)
#define IMODE(x)    penc->imode(x)

/*==============================================================================
 * SparcNJExpander class implementation.
 *============================================================================*/

/*==============================================================================
 * FUNCTION:    SparcNJExpander::SparcNJExpander
 * OVERVIEW:    Constructor for class SparcNJExpander
 * PARAMETERS:  <none>
 * RETURNS:     <nothing>
 *============================================================================*/
SparcNJExpander::SparcNJExpander()
{
    // Get Sparc encoding routines
    penc = sparc_enc;
}

/*==============================================================================
 * FUNCTION:    Expander::~SparcNJExpander
 * OVERVIEW:    Destructor for class SparcNJExpander
 * PARAMETERS:  <none>
 * RETURNS:     <nothing>
 *============================================================================*/
SparcNJExpander::~SparcNJExpander()
{
    // Delete binary code
    free(rawCode);
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::clearLocations
 * OVERVIEW:    Release all locations (they become free).
 * PARAMETERS:  <none>
 * RETURNS:     <nothing>
 *============================================================================*/
void SparcNJExpander::clearLocations()
{
    nextFreeLoc = 0;
    nextParamLoc = 0;
    locPool.clear();
    typPool.clear();
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::getNewLocation
 * OVERVIEW:    Get a location according with a location type.
 * PARAMETERS:  stgType: location type.
 *              idx:     Index to location (for input params).
 * RETURNS:     Location indentifier.
 *============================================================================*/
Location SparcNJExpander::getNewLocation(Location stgType, int idx = 0)
{
    Location loc;   // Auxiliar location

    // Storagy type
    switch (stgType)
    {
        case TGT_OUTPUT:    // Output location
            // Get an output location for o0..o5 or stack
            loc = OUTPUT_O(nextParamLoc);
            nextParamLoc++;
            break;

        case TGT_INPUT:     // Input location
            // Return input location for i0..i5
            if (idx < TGT_NUM_I)
                loc = TGT_I(idx);
            else
            {
                error("No more than six arguments allowed\n");
                exit(1);
            }
            break;

        case TGT_LOCAL:     // Local location
            // Get a free location for l0..l7
            assert(nextFreeLoc < TGT_NUM_L);
            loc = TGT_L(nextFreeLoc++);
            break;

        default:        // Never reached!
        {
            ostrstream ost;
            ost << "Cannot handle with this type of location\n";
            error(str(ost));
            exit(1);
        }
    }

    // Return location
    return loc;
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::releaseLocation
 * OVERVIEW:    Release the last took local register.
 * PARAMETERS:  <none>
 * RETURNS:     <nothing>
 *============================================================================*/
#define releaseLocation() nextFreeLoc--

/*==============================================================================
 * FUNCTION:    SparcNJExpander::getParamNo
 * OVERVIEW:    Get the parameter number that correspond to the semantic string.
 * PARAMETERS:  proc: Procedure.
 *              ss:   Semantic string containing the parameter.
 * RETURNS:     Parameter number or -1
 *============================================================================*/
int SparcNJExpander::getParamNo(const Proc* proc, const SemStr *ss)
{
    // Get number of parameters
    int n = proc->getParams().size();
    for (int i = 0; i < n; i++)
    {
        const SemStr *par = proc->getParameter(i);
        // Is this parameter?
#if 0   // Fixme!
        if (*par == *ss)
            return i;
#else
        switch (ss->getFirstIdx())
        {
            case idVar:
                if (par->getFirstIdx() == idVar)
                    if (par->getSecondIdx() == ss->getSecondIdx())
                        return i;
                break;

            case idRegOf:
                if (par->getFirstIdx() == idRegOf)
                    if (par->getThirdIdx() == ss->getThirdIdx())
                        return i;
                break;
        }
#endif
    }

    // Not found
    return -1;
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::getLocation
 * OVERVIEW:    Get a RHS location according to the semantic string.
 * PARAMETERS:  pExp:  SemStr representing the HRTL expression.
 *              cType: Type of the location.
 * RETURNS:     location indentifier.
 *============================================================================*/
Location SparcNJExpander::getLocation(const SemStr* pExp, Type cType)
{
    // Null expressions returns invalid locations
    int idx = pExp->getFirstIdx();
    if (idx == WILD)
        return INVALID_LOCATION;

    // Dealing with machine specific IDs
    idx = idx < idMachSpec ? idx : idMachSpec;

    // RTL identifier -- keep storage info
    Location loc;   // location
    Location lpl = INVALID_LOCATION;
    switch (idx)
    {
        case idIntConst:    // Integer Constant
            loc = SPC_CNTN;
            lpl = pExp->getSecondIdx();
            break;

        case idRegOf:       // Source machine register
            // Deal only with constants and tmp
            if (pExp->getSecondIdx() == idIntConst)
            {
                loc = getParamNo(proc, pExp);
                loc = (loc >= 0)
                    ? getNewLocation(TGT_INPUT, loc)
                    : SRC_R(pExp->getThirdIdx());
            }
            else if (pExp->getSecondIdx() == idTemp)
                loc = SPC_TMP;
            else
                expHalt("Unsupported register class");
            break;

        case idVar:         // Local variable
            {
                loc = getParamNo(proc, pExp);
                loc = (loc >= 0)
                    ? getNewLocation(TGT_INPUT, loc)
                    : SYM_V(pExp->getSecondIdx());
            }
            break;

        case idMachSpec:    // Machine specific ID
            loc = SPC_R(pExp->getFirstIdx());
            break;

        case idMemOf:       // Raw memory
            // NOTE: This is a special case, where every decission is
            // delayed until 'emitUnary' is called.
            loc = SPC_MEM;
            lpl = idx;      // Get operation
            break;

        case idAddrOf:      // Get address
            // Must be a symbolic variable -- not a param!
            assert(pExp->getSecondIdx() == idVar);
            loc = SPC_ADR;
            lpl = SYM_V(pExp->getThirdIdx());
            break;

        case idAFP:         // Locals
            loc = SPC_AFP;
            break;

        // Unary
        case idNot:         // Not
        case idNeg:         // Minus
        case idSignExt:     // Sign extension

        // Arithmetics
        case idPlus:        // Addition
        case idMinus:       // Subtraction
        case idMult:        // Multiply
        case idMults:       // Multiply signed
        case idDiv:         // Integer division
        case idDivs:        // Integer division signed
        case idMod:         // Modulus (unsigned)
        case idMods:        // Modulus signed

        // Logicals
        case idBitOr:       // |
        case idBitAnd:      // &
        case idBitXor:      // ^

        // Shift
        case idShiftL:      // Shift left
        case idShiftR:      // Shift right
        case idShiftRA:     // Shift left (arithmetic)

        // Comparisons
        case idEquals:      // ==
        case idNotEqual:    // !=
        case idLess:        // < (signed)
        case idGtr:         // > (signed)
        case idLessEq:      // <= (signed)
        case idGtrEq:       // >= (signed)
        case idLessUns:     // < (unsigned)
        case idGtrUns:      // > (unsigned)
        case idLessEqUns:   // <= (unsigned)
        case idGtrEqUns:    // >= (unsigned)

        // Conversions
        case idZfill:       // Zero extension
        case idSgnEx:       // Sign extension
        case idTruncs:      // Truncation (signed)
        case idTruncu:      // Truncation (unsigned)
            // Get a local location
            loc = getNewLocation(TGT_LOCAL);
            lpl = idx;      // Get operation
            break;

        default:
            cout << "Index " << idx << " -- ";
            expHalt("SparcNJExpander::getLocation: cannot get a location");
    }

    // Set storage extra info
    locPool[loc] = lpl;

    // Set type info
    typPool[loc] = cType;

    // Return location
    return loc;
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::fetch
 * OVERVIEW:    Emits code for loading an expression into a location.
 * PARAMETERS:  loc:     location where the value is located.
 *              isParam: Should the location be loaded into an argument?.
 * RETURNS:     Local location.
 *============================================================================*/
Location SparcNJExpander::fetch(Location loc, bool isParam = false)
{
    // If location is a local register or invalid, just bypass
    if (IS_TGT_L(loc) || loc == INVALID_LOCATION)
        return loc;

    // Get local (or output, if parameter) location
    Location dst = getNewLocation(isParam ? TGT_OUTPUT : TGT_LOCAL);
    Location old = dst;

    // Is an outgoing parameter past the sixth?
    if (IS_SPC_O(dst))
    {
        // Then the location is not a register
        // Get a new local to deal with
        dst = getNewLocation(TGT_LOCAL);
    }

    /*
     * What type of register is this location?
     */
    
    if (IS_TGT_I(loc))          // Is location a parameter?
    {
        // Emit a single mov
        penc->mov_(loc, dst);
    }

    else if (IS_SRC_R(loc) ||   // Is a source register?
             IS_SYM_V(loc) ||   // Is a symbolic register?
             IS_SPC_R(loc) ||   // Is a machine specific register?
             loc == SPC_TMP)    // Is a temporal register?
    {
        // Emit load from local stack
        emitLoad(dst, TGT_FP, ofsPool[loc], typPool[loc]);
    }


    else switch (loc)           // Other type of location
    {
        case SPC_CNTN:          // Constant
            // Emit load inmediate
            penc->sethi(locPool[loc] & ~0x3ff, dst);           // %hi
            penc->OR(dst,IMODE(locPool[loc] & 0x3ff), dst);    // %lo
            break;

        case SPC_AFP:           // Locals
            // Load effective addresss
            penc->ADD(TGT_FP,IMODE(ofsPool[SPC_AFP]),dst);
            break;

        case SPC_ADR:           // Address of
            // Load effective addresss
            penc->ADD(TGT_FP,IMODE(ofsPool[locPool[loc]]),dst);
            break;

        case SPC_MEM:           // Raw memory access
            // EFFA must be a local register comming from memory
            assert(IS_TGT_L(locPool[loc]));

            // Emit load from memory
            emitLoad(dst, locPool[loc], /*ofst*/ 0, typPool[loc]);

#if SRCENDIAN != BIGE
            // Be councious about endianism!
            if(!progOptions.noendian)
                emitSwap(dst, typPool[loc]);
#endif
            break;

        default:
            cout << loc << " -- ";
            expHalt("Don't know what to do loading location");
    }

    // Finally, if dst was an outgoing parameter past the sixth,
    // store value into the stack
    if (IS_SPC_O(old))
    {
        // Emit store into local stack
        emitStore(TGT_SP, ofsPool[old], dst, typPool[old]);
    }

    // Return location where value has been emited
    return dst;
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::emitSwap
 * OVERVIEW:    Emit a sequence of instructions to swap a value into a location.
 * PARAMETERS:  dst:   Target location (it's supposed to be in a register).
 *              lType: Type of the swapping
 * RETURNS:     <nothing>
 *============================================================================*/
void SparcNJExpander::emitSwap(Location dst, Type lType)
{
    // Only deal with local registers
    assert(IS_TGT_L(dst));

    // So far, only 32 bit data!
    assert(lType.getSize() == 32);

    // Get an auxiliar location
    Location loc = getNewLocation(TGT_LOCAL);

    // Emit the sequence for swapping
    assert(loc != loc);  // I.e., always abort since not yet implemented!
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::emitLoad
 * OVERVIEW:    Emits a load operation from memory based on the type.
 * PARAMETERS:  dst:   Target location.
 *              base:  Source location (base address).
 *              ofst:  Offset from the base address.
 *              lType: Type of the load operation
 * RETURNS:     <nothing>
 *============================================================================*/
void SparcNJExpander::emitLoad(Location dst, Location base, int ofst, Type lType)
{
    // Take into account the assignment type
    switch (lType.getSize())
    {
// Store encoding
#define CASE_SIZE(sz,op)                            \
    case sz: {                                      \
        penc->op(penc->dispA(base, ofst), dst);     \
        break;                                      \
    }
        CASE_SIZE( 8, LDUB)     // Load byte
        CASE_SIZE(16, LDUH)     // Load half
        CASE_SIZE(32, LD)       // Load word
#undef CASE_SIZE

        default:
            expHalt("Unsupported load size");
    }
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::emitStore
 * OVERVIEW:    Emits a store operation into memory based on the type.
 * PARAMETERS:  base:  Target location (base address).
 *              ofst:  Offset from the base address.
 *              dst:   Source location.
 *              lType: Type of the load operation.
 * RETURNS:     <nothing>
 *============================================================================*/
void SparcNJExpander::emitStore(Location base, int ofst, Location src, Type lType)
{
    // Take into account the assignment type
    switch (lType.getSize())
    {
// Store encoding
#define CASE_SIZE(sz,op)                            \
    case sz: {                                      \
        penc->op(src, penc->dispA(base, ofst));     \
        break;                                      \
    }
        CASE_SIZE( 8, STB)      // Store byte
        CASE_SIZE(16, STH)      // Store half
        CASE_SIZE(32, ST)       // Store word
#undef CASE_SIZE

        default:
            expHalt("Unsupported store size");
    }
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::emitUnary
 * OVERVIEW:    Emits code for a Unary operation.
 * PARAMETERS:  dst: Target location.
 *              src: Source location.
 * RETURNS:     <nothing>
 *============================================================================*/
void SparcNJExpander::emitUnary(Location dst, Location src)
{
    // Several checks
    assert(IS_TGT_L(dst) || dst == SPC_MEM);
    assert(IS_TGT_L(src));

    // Get unary operation
    switch (locPool[dst])
    {
// Unary operation encoding
#define CASE_OPER(id,op)                    \
    case id: {                              \
        penc->op(TGT_ZERO,RMODE(src),dst);  \
        break;                              \
    }
        // Unary
        CASE_OPER(idNot, ADD)           // Loginal Not
        CASE_OPER(idNeg, SUB)           // Unary Minus
        CASE_OPER(idSignExt, OR)        // Sign Extension (do nothing!)
#undef CASE_OPER

        case idMemOf:                   // Raw memory access
            // This is a special case
            // Just *link* memory access with effective address
            locPool[dst] = src;
            break;

        // Never reached!
        default:
            expHalt("Unknown unary operation");
    }
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::emitBinary
 * OVERVIEW:    Emits code for a binary operation.
 * PARAMETERS:  dst:  Target location.
 *              src1: First source location.
 *              src2: Second source location.
 * RETURNS:     <nothing>
 *============================================================================*/
void SparcNJExpander::emitBinary(Location dst, Location src1, Location src2)
{
    // Locations must be local registers
    assert(IS_TGT_L(dst) && IS_TGT_L(src1) && IS_TGT_L(src2));

    // Get binary operation
    switch (locPool[dst])
    {
// Comparisons - by conditinal branch
#define CASE_COMP(id,br)                            \
    case id: {                                      \
        /* New relocatable info */                  \
        RAddr raddr = addr_new(label_new(NULL), 0); \
        /* Test the relation */                     \
        penc->cmp(src1,RMODE(src2));                \
        /* Assume TRUE by default */                \
        penc->OR(TGT_ZERO,IMODE(1),dst);            \
        /* Branch & nulify delay slot */            \
        penc->br(raddr);                            \
        penc->NOP();                                \
        /* Overwriting by FALSE */                  \
        penc->OR(TGT_ZERO,IMODE(0),dst);            \
        /* Set label for target */                  \
        label_define(raddr->label, 0);              \
        break;                                      \
    }
        // Comparisons
        CASE_COMP(idEquals,    BE)      // ==
        CASE_COMP(idNotEqual,  BNE)     // !=
        CASE_COMP(idLess,      BL)      // < (signed)
        CASE_COMP(idGtr,       BG)      // > (signed)
        CASE_COMP(idLessEq,    BLE)     // <= (signed)
        CASE_COMP(idGtrEq,     BGE)     // >= (signed)
        CASE_COMP(idLessUns,   BCS)     // < (unsigned)
        CASE_COMP(idGtrUns,    BGU)     // > (unsigned)
        CASE_COMP(idLessEqUns, BLEU)    // <= (unsigned)
        CASE_COMP(idGtrEqUns,  BCC)     // >= (unsigned)
#undef CASE_COMP

// Binary operation encoding
#define CASE_OPER(id,op)                            \
    case id: {                                      \
        penc->op(src1,RMODE(src2),dst);             \
        break;                                      \
    }
        // Arithmetics
        CASE_OPER(idPlus,    ADD)       // Addition
        CASE_OPER(idMinus,   SUB)       // Substraction
        CASE_OPER(idMult,    UMUL)      // Multiply
        CASE_OPER(idMults,   SMUL)      // Multiply signed
        CASE_OPER(idDiv,     UMUL)      // Integer division
        CASE_OPER(idDivs,    SMUL)      // Integer division signed

        // Logicals
        CASE_OPER(idBitOr,   OR)        // |
        CASE_OPER(idBitAnd,  AND)       // &
        CASE_OPER(idBitXor,  XOR)       // ^

        // Shift
        CASE_OPER(idShiftL,  SLL)       // Shift left
        CASE_OPER(idShiftR,  SRL)       // Shift right
        CASE_OPER(idShiftRA, SRA)       // Shift left (arithmetic)
#undef CASE_OPER

        // Special cases
        case idMod:                     // Modulus (unsigned)
        {
            // Set parameters for calling .urem
            penc->mov_(src1, TGT_O(0));
            penc->mov_(src2, TGT_O(1));

            // Call dynamically the .urem external procedure
            RAddr target = updateGlobalRAddr(".urem");
            penc->call__(target);
            penc->NOP();
            penc->mov_(TGT_O0, dst);
            break;
        }

        case idMods:                     // Modulus signed
        {
            // Set parameters for calling .rem
            penc->mov_(src1, TGT_O(0));
            penc->mov_(src2, TGT_O(1));

            // Call dynamically the .urem external procedure
            RAddr target = updateGlobalRAddr(".rem");
            penc->call__(target);
            penc->NOP();
            penc->mov_(TGT_O0, dst);
            break;
        }

       // Never reached!
        default:
            expHalt("Unknown binary operation");
    }
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::emitConversion
 * OVERVIEW:    Emits code for a conversion.
 * PARAMETERS:  dst:  Target location.
 *              src:  Source location.
 *              from: From what bit.
 *              to:   To what bit.
 * RETURNS:     <nothing>
 *============================================================================*/
void SparcNJExpander::emitConversion(Location dst, Location src, int from, int to)
{
    // Locations must be local registers
    assert(IS_TGT_L(src) && IS_TGT_L(dst));

    // Get operation (relations)
    switch (locPool[dst])
    {
// Conversion encoding
#define CASE_OPER(id,i1,i2,op1,op2)                 \
    case id: {                                      \
        /* So far, do not deal with middle bits */  \
        assert(i1 < i2 && i2 >= SPARC_WORD_SZ * 8); \
        penc->op1(src,IMODE(i2 - i1),dst);          \
        penc->op2(src,IMODE(i2 - i1),dst);          \
        break;                                      \
    }
        CASE_OPER(idZfill,  from, to, SLL, SRL)     // Zero extension
        CASE_OPER(idSgnEx,  from, to, SLL, SRA)     // Sign extension
        CASE_OPER(idTruncs, to, from, SLL, SRA)     // Truncation (signed)
        CASE_OPER(idTruncu, to, from, SLL, SRL)     // Truncation (unsigned)
#undef CASE_OPER

        // Never reached!
        default:
            expHalt("Unknown conversion");
    }
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::emitAssign
 * OVERVIEW:    Emits code for an assignment between two locations.
 * PARAMETERS:  dst: Target location.
 *              src: Source location.
 * RETURNS:     <nothing>
 *============================================================================*/
void SparcNJExpander::emitAssign(Location dst, Location src)
{
    // Source location must be a local register
    assert(IS_TGT_L(src) || src == TGT_O0);

    /*
     * What type of register is the target?
     */
    
    if (IS_TGT_I(dst))          // Is location a parameter?
    {
        // Emit single mov
        penc->mov_(src, dst);
    }

    else if (IS_SRC_R(dst) ||   // Is a source register?
             IS_SYM_V(dst) ||   // Is a symbolic register?
             IS_SPC_R(dst) ||   // Is a machine specific register?
             dst == SPC_TMP)    // Is a temporal register?
    {
        // Emit store in local stack
        emitStore(TGT_FP, ofsPool[dst], src, typPool[dst]);
    }

    else switch (dst)           // Other type of location
    {
        case SPC_MEM:           // Raw memory access
            // EFFA must be a local register comming from memory
            assert(IS_TGT_L(locPool[dst]));

#if SRCENDIAN != BIGE
            // Be councious about endianism!
            if(!progOptions.noendian)
                emitSwap(locPool[dst], typPool[dst]);
#endif
            // Emit store to memory
            emitStore(locPool[dst], /*ofst*/ 0, src, typPool[dst]);
            break;

        default:                // Never reached!
            expHalt("Don't know what to do assigning location");
    }
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::emitProlog
 * OVERVIEW:    Emits a the prolog for the entry point of a function.
 * PARAMETERS:  <none>
 * RETURNS:     <nothing>
 *============================================================================*/
void SparcNJExpander::emitProlog()
{
    // Emit save window -- save %sp, -offs, %sp
    penc->SAVE(TGT_SP, IMODE(-frameSize), TGT_SP);
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::emitBranch
 * OVERVIEW:    Emits code for a unconditional branch.
 * PARAMETERS:  pTgt: Pointer to target BB.
 * RETURNS:     <nothing>
 *============================================================================*/
void SparcNJExpander::emitBranch(PBB pTgt)
{
    // Emit unconditional branch and nullify delay slot
    penc->BA(updateLocalRAddr(pTgt));
    penc->NOP();
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::emitCondBranch
 * OVERVIEW:    Emits code for a conditional *if*true* branch (pc relative).
 * PARAMETERS:  loc:  Location where the condition is stored.
 *              pTgt: Pointer to target BB.
 * RETURNS:     <nothing>
 *============================================================================*/
void SparcNJExpander::emitCondBranch(Location loc, PBB pTgt)
{
    // Location must be a local
    assert(IS_TGT_L(loc));

    // Update local relocatable info
    RAddr raddr = updateLocalRAddr(pTgt);

    // Emit "if (loc != 0) " branch and nullify delay slot
    penc->cmp(loc,RMODE(TGT_ZERO));
    penc->BNE(raddr);
    penc->NOP();
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::emitCase
 * OVERVIEW:    Emits code for switch-case way.
 * PARAMETERS:  loc:  Location where the seed is located.
 *              val:  Constant to be compared to.
 *              pTgt: Pointer to target BB.
 * RETURNS:     <nothing>
 *============================================================================*/
void SparcNJExpander::emitCase(Location loc, int val, PBB pTgt)
{
    // Location must be a local
    assert(IS_TGT_L(loc));

    // Load inmediate into temporal location
    Location lcmp = getNewLocation(TGT_LOCAL);
    penc->sethi(val & ~0x3ff, lcmp);            // %hi
    penc->OR(lcmp, IMODE(val & 0x3ff), lcmp);   // %lo

    // Compare values
    RAddr raddr = addr_new(label_new(NULL), 0);
    penc->cmp(loc, RMODE(lcmp));
    penc->OR(TGT_ZERO,IMODE(1),lcmp);           // True by default
    penc->BE(raddr);                            // Branch
    penc->NOP();                                // Nullify delay slot
    penc->OR(TGT_ZERO,IMODE(0),lcmp);           // Overwrite by FALSE */ 
    label_define(raddr->label, 0);

    // Emit conditional branch base on result
    emitCondBranch(lcmp, pTgt);

    // Release temporal location
    releaseLocation();
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::emitCall
 * OVERVIEW:    Emits code for a Sparc call.
 * PARAMETERS:  dst:   Target location (return value).
 *              pTgt:  Pointer to target procedure (calee).
 *              param: Vector of locations (parameters).
 * RETURNS:     <nothing>
 *============================================================================*/
void SparcNJExpander::emitCall(Location dst, Proc* pTgt, const vector<Location>& param)
{
    // Get a local address recursive call, otherwise make a global RAddr
    RAddr target = (proc->getNativeAddress() == pTgt->getNativeAddress())
                 ? updateLocalRAddr(proc->getEntryBB())
                 : updateGlobalRAddr(pTgt->getName());

    // Emit call and nullify dealy slot
    penc->call__(target);
    penc->NOP();

    // Emit assign if something is returned
    if (dst != INVALID_LOCATION)
        emitAssign(dst, TGT_O0);
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::emitReturn
 * OVERVIEW:    Emits code for a Sparc return.
 * PARAMETERS:  loc: Source location for return value.
 * RETURNS:     <nothing>
 *============================================================================*/
void SparcNJExpander::emitReturn(Location loc)
{
    // Do the function return something?
    bool retval = (loc != INVALID_LOCATION);

    // Non-invalid target location must be a local register
    assert(!retval || IS_TGT_L(loc));

    // Emit return from subrutine
    penc->ret();

    // Emit restore window in delay slot
    penc->RESTORE(
        retval ? loc : TGT_G0,      // l? or g0
        RMODE(TGT_G0),              // g0
        retval ? TGT_O0 : TGT_G0    // o0 or g0
    );
}

/*==============================================================================
 * FUNCTION:    SparcNJExpander::buildStack
 * OVERVIEW:    Sparc-dependent preprocess function.
 *              The main thing is to build the function stack.
 * PARAMETERS:  <none>
 * RETURNS:     Size of the stack (in bytes).
 *============================================================================*/
int SparcNJExpander::buildStack()
{
    // Sparc v8 Stack
    // Every procedure is implemented as a non-leaf procedure!
    int topstk = TGT_NUM_L      // %l0..%l7
               + TGT_NUM_I + 2  // %i0..%i7
               + 1              // Hidden param
               + 6              // First six params
               ;
    topstk *= SPARC_WORD_SZ;

    // How many params past the sixth must be allocated on the stack frame?
    // Look for every call RTL in this procecure.
    int nparams = 0;
    SET_CALLS& calls = proc->getCFG()->getCalls();
    for (SET_CALLS::iterator cit = calls.begin(); cit != calls.end(); cit++)
    {
        // Get number of params past the sixth
        int nargs = (*cit)->getParams().size() - 6;
        if (nargs < 0) nargs = 0;
        if (nargs > nparams) nparams = nargs;
    }

    // Now allocate space for these params
    // FIXME -- Their size is assume to be 32 bits!
    for (int i = 0; i < nparams; i++)
    {
        // Declare outgoing param
        topstk = alignd(topstk, /*Fixme!*/ 32);
        ofsPool[SPC_O(i)] = topstk;
    }

    // Local variables include:
    // - Source stack (_locals).
    // - Source machine registers (e.g.: r[3])
    // - Symbolic variables (e.g.: v7).
    // - Other (e.g.: %CF, tmp, etc.).
    // Variables are aligned according to their size.
    //   (of course, original %sp must be previously aligned to 8 bytes!).
    // They are also declared in reverse order (bottom-up).
    int botstk = 0;

    // Declare the source stack at 8 bytes boundaries
    botstk = alignd(botstk, proc->getLocalsSize() * 8, /* long */8);
    ofsPool[SPC_AFP] = -botstk;

    // Weird case on main...
    assert(!((proc->getName() == "main") && (proc->getNumArgs() > 2)));

    // Declare the symbolic variables
    if (symUsed.size())
    {
        IntTypeMap::const_iterator it;
        for (it = symUsed.begin(); it != symUsed.end(); it++)
        {
            // Declare symbolic
            botstk = alignd(botstk, it->second.getSize());
            ofsPool[SYM_V(it->first)] = -botstk;
        }
    }

    // Declare an unique 'tmp' -- FIXME!!!
    botstk = alignd(botstk, 64);
    ofsPool[SPC_TMP] = -botstk;

    // Declare the overlapped registers
    map<int, RegInfo> &regMap = ovl.getRegMap();
    map<int, RegInfo>::const_iterator rit;
    // Find the "top level used" registers
    for (rit = regMap.begin(); rit != regMap.end(); rit++)
    {
        // *rit is a pair: first is the register number (e.g. 33 for Sparc %f1),
        // second is the RegInfo for this register
        int r = rit->first;
        int size = prog.RTLDict.DetRegMap[r].g_size();

        // This is not a top level register; ignore
        if (size != rit->second.maxSize) continue;

        // Declare 'main' register
        botstk = alignd(botstk, size);
        ofsPool[SRC_R(r)] = -botstk;

        // Now do all the registers that this is overlapped with
        list<int> parRegNums, childRegNums;
        for (parRegNums.push_back(r); ; )
        {
            // Find all the children of this level
            childRegNums.clear();
            LevelInfo* li = 0;
            list<int>::const_iterator ll;
            list<int>::iterator itPar;
            for (itPar = parRegNums.begin(); itPar != parRegNums.end(); itPar++)
            {
                assert(*itPar >= 0);
                // Is there any child?
                if (ovl.isChildOf(*itPar))
                {
                    // Get children list
                    li = ovl.getChildOf(*itPar);
                    for (ll = li->regs.begin(); ll != li->regs.end(); ll++)
                        childRegNums.push_back(*ll);
                }
            }

            // There were no children
            if (li == 0) break;

            // First child offset is parent's offset
            int coffst = ofsPool[SRC_R(r)];

            // Now declare the children on the stack
            for (ll = childRegNums.begin(); ll != childRegNums.end(); ll++)
            {
                // Child offset is previous child offset + previuos child size
                ofsPool[SRC_R(*ll)] = coffst;
                coffst += (li->size / 8);   // li->size is in bits!!!
            }

            // Go down a level; this level's children are the next level's parents
            parRegNums = childRegNums;
        }
    }

    // Declare the rest of source registers
    if (regUsed.size())
    {
        IntTypeMap::const_iterator it;
        for (it = regUsed.begin(); it != regUsed.end(); it++)
        {
            // If overlaped register, was declared before
            if (ovl.isOverlappedReg(it->first)) continue;

            // Declare register
            botstk = alignd(botstk, it->second.getSize());
            ofsPool[SRC_R(it->first)] = -botstk;
        }
    }

    // Now declare condition codes and so on (32 bit by default)
    set<INDEX>::iterator sit;
    for (sit = stIdx.begin(); sit != stIdx.end(); sit++)
    {
        // Declare special register
        botstk = alignd(botstk, 32);
        ofsPool[SPC_R(*sit)] = -botstk;
    }

    // Finally, align stack to 8 bytes boundaries
    topstk += botstk;
    topstk = alignd(0, topstk * 8, /* long */8);

#if 0
    LValIntMap::const_iterator it;
    for (it = ofsPool.begin(); it != ofsPool.end(); it++)
    {
cout << "\t.data ";
        if (IS_SRC_R(it->first))
cout << "s" << it->first << " or r" << it->first - SRC_R0;
        else if (IS_SYM_V(it->first))
cout << "s" << it->first << " or v" << it->first - SYM_V0;
        else if (IS_SPC_R(it->first))
cout << "s" << it->first << " or s" << it->first - SPC_R0;
        else if (IS_SPC_O(it->first))
cout << "s" << it->first << " or o" << it->first - SPC_O0;
        else if (it->first == SPC_AFP)
cout << "s" << it->first << " or _locals";
        else if (it->first == SPC_TMP)
cout << "s" << it->first << " or tmp";
        else
{ cout << it->first << "!\n"; expHalt("Weird!"); }
cout << " at " << hex << it->second <<"(%fp) or "
               << dec << it->second <<"(%fp)" << endl;
    }
#endif

    // Return total frame size
    return topstk;
}
