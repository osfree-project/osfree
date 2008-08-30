/*
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 * Copyright (C) 2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*============================================================================
 * FILE:        translate2jvm.cc
 * OVERVIEW:    Implementation of the JVM backend. 
 *
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 * Copyright (C) 2001, The University of Queensland, BT group
 *===========================================================================*/

/*
 * $Revision: 1.13 $
 * 30 May 00 - Sameer: created the file
 * 04 Aug 00 - Sameer: Reformatted and Commented the code.
 * 30 Aug 00 - Brian: minor edits needed to add it to main Walkabout tree.
 * 14 Nov 00 - Brian: much rework and addition of new methods.
 * 15 Nov 00 - Brian: removed code that tried to save memory by not allocating
 *             storage before the first rw/ro/bss data. The need to subtract an
 *             offset from a "logical" address to form a "physical" address
 *             caused no end of problems with storage not in the "memory"
 *             array: e.g., AFP arrays and other other heap-allocated memory.
 * 16 Nov 00 - Brian: made memory a byte array, removed use of "ldc" for memory
 *             addresses.
 * 17 Nov 00 - Brian: made translated methods static except for _main(), which
 *              must be nonstatic for technical reasons (it's declared abstract
 *              in TranslatedFile.java). 
 * 05 Mar 01 - Simon: Removed RTCOND case
 * 31 Mar 01 - Mike: getFixedDest() returns NO_ADDRESS for non fixed addresses
 * 15 Apr 01 - Brian: Support registers in argument lists and return exprs.
 * 24 Apr 01 - Brian: Corrected off-by-one error in local var allocation for
 *              HRTL variables.
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 * 30 Aug 01 - Mike: Parameters changed from vector to list
 */

// There are different kind of variables in the HRTL code, which have to be
// mapped to JVM locals. This section gives a brief explanation of some of
// the conventions used while doing this mapping. 
//
// Local variable usage:
//    Local  0:    Java "this" reference if this is the nonstatic _main()
//                 method, otherwise the first formal parameter.
//    Locals 1-7:  all or remaining formal parameters.
//    Local  8:    AFP (Abstract Frame Pointer) - HACK: set 0 so a low,
//                 hopefully unused, part of the "memory" array is used.
//    Local  9:    unused
//    Locals [10..(10+n)): the n local HRTL variables used in the method.
//    Locals [(10+n)..99]: temporary registers.
//    Local 100:   a temp variable.
//
// Consider the case in which the function to be translated has parameters
// v0 and v1, and variables v2 - v6 are used in the HRTLs as local HRTL
// variables. Also assume that temporary registers r1 and r5 occur in the
// HRTL code. In this case, the following mapping to JVM locals is done:
//
//    v0     =>  local   0  "this"/parameter 0
//    v1     =>  local   1  v0 - parameter 1
//    v2     =>  local   2  v1 - parameter 2
//    AFP    =>  local   8  integer 0 (see above)
//           =>  local   9  unused
//    ------------------------
//    v2     =>  local  10  v2 - HRTL variable = FIRST_JVM_LOCAL	
//    v3     =>  local  11  v3
//    v4     =>  local  12  v4
//    v5     =>  local  13  v5
//    v6     =>  local  14  v6 - proc->getLastLocalIndex() = lastLocal
//    ------------------------
//    r1     =>  local  15  r1 - temp register = (FIRST_JVM_LOCAL + lastLocal)
//    r5     =>  local  16  r2 - temp register
//    temp   =>  local 100  temp 
   
#include "global.h"
#include "prog.h"                       
#include "proc.h"
#include "cfg.h"
#include "ss.h"
#include "rtl.h"
#include "options.h"                    
#include "csr.h"

/*
 * Macros and constants used in this file
 */

// Stack size for each translated procedure. Should be computed!
#define METHOD_STACK_SIZE      10

// Number of local variables for each translated procedure. Should be computed!
#define METHOD_LOCAL_VARIABLES 130

// The procedure's HRTL variables start at JVM local variable FIRST_JVM_LOCAL.
#define FIRST_JVM_LOCAL        10


/*
 * Static variables used in this file
 */

// The output stream.
static ofstream outStream;
static bool streamOpen = false;	// set true if "outStream" has been opened.

// Set true if code is being generated for the "main" procedure: its method
// can not be static.
static bool processingMain = false;

// Used to allocate JVM local variables for the registers in Semantic strings.
static map<const int,int> regNumbers;

// Index of the next free JVM local var which can be allocated to a register.
static int nextFreeLocForReg = 0;	 

// The first and last local variables used by this method: e.g. if the program
// uses locals v0 to v10, firstLocal is 0 and lastLocal 10.
static int firstLocal;
static int lastLocal;

// Address of the RTL being processed for error messages.
static ADDRESS rtlAddr = 0;

// Set true to indicate that an %AFP byte array has been allocated for this
// proc and stored in local 8. This is done on the first reference to %AFP.
static bool afpArrayAllocated = false;


/*
 * Prototypes for procedures defined later in this file:
 */

string processExpr(const SemStr* pExp, Type cType);


/*============================================================================
 * FUNCTION: 	processRegOf()
 * OVERVIEW:    Maps a temporary register r[exp] to a JVM local variable. The
 *              expression will usually be an integer constant, but could be
 *              a simple expression (like 6+1) or an idTemp. This procedure
 *              checks to see if the register was mapped previously. If it
 *              was, it returns the index of the local var it was mapped to,
 *              otherwise it allocates and returns the index of the next
 *              available local var in the map.
 * PARAMETERS:  pLoc: A pointer to the register's Semantic String.
 * RETURNS:     string representing the mapped register.
 *===========================================================================*/

string processRegOf(const SemStr* pLoc) {
    int idx, num, regNum;
    ostrstream ost;

    idx = pLoc->getSecondIdx();
    if (idx == idTemp) {
	// Assumption: temp. registers are always mapped to reg 100. 
	regNum = 100;
    } else if (idx == idIntConst) { 
	num = pLoc->getThirdIdx();
	if (regNumbers.count(num) != 0) { // the register was already mapped
	    map<const int, int>::iterator cur = regNumbers.find(num);
	    regNum = (*cur).second;
	} else {	// first map register then return mapped value.
	    regNumbers[num] = nextFreeLocForReg;
	    regNum = nextFreeLocForReg;
	    nextFreeLocForReg++;
	}
    } else {
	// Must be an expression; it should be (int const + int const)
	assert(idx == idPlus);
	SemStr* pTerm1 = pLoc->getSubExpr(0);
	SemStr* pTerm2 = pLoc->getSubExpr(1);
	num = (pTerm1->getSecondIdx() + pTerm2->getSecondIdx());
	if (regNumbers.count(num) != 0) {
	    map<const int, int>::iterator cur = regNumbers.find(num);
	    regNum = (*cur).second;
	} else { 
	    regNumbers[num] = nextFreeLocForReg;
	    regNum = nextFreeLocForReg;
	    nextFreeLocForReg++;
	}
    }
    ost << regNum;
    return str(ost);
} 

/*=============================================================================
 * FUNCTION: 	processMemOf
 * OVERWIEW:	Process a memory reference m[exp] SemStr on the RHS of an
 *              expression.
 * PARAMETERS:  idx: The integer value of the unary operator's Semantic string.
 * PARAMETERS:  pLoc: Pointer to the memory reference's Semantic String.
 *              cType: Desired Type of the idMemOf expression.
 * RETURNS:     String holding instructions for the memory reference.
 *===========================================================================*/

string processMemOf(const SemStr* pLoc, Type cType) {
    SemStr* pSub = pLoc->getSubExpr(0);
    const char *class_name = initCapital(prog.getNameNoPath()).c_str();
    int bitsToRead = cType.getSize();
    ostrstream ost;

    ost << "    getstatic " << class_name << "/memory [B\n";
    // Address expressions are unsigned 32 bit
    ost << processExpr(pSub, Type(INTEGER, 32, false));
    
    switch (bitsToRead) {
        case 32:
	    ost << "    dup2\n";
	    ost << "    baload ;    byte 1 (MSB) of 4\n";
	    ost << "    bipush 24\n";
	    ost << "    ishl\n";
	    ost << "    dup_x2\n";
	    ost << "    pop\n";
	    ost << "    iconst_1\n";
	    ost << "    iadd\n";
	    ost << "    dup2\n";
	    ost << "    baload ;    byte 2 of 4\n";
	    ost << "    sipush 255\n"; // 0xFF
	    ost << "    iand\n";
	    ost << "    bipush 16\n";
	    ost << "    ishl\n";
	    ost << "    dup_x2\n";
	    ost << "    pop\n";
	    ost << "    iconst_1\n";
	    ost << "    iadd\n";
	    ost << "    dup2\n";
	    ost << "    baload ;    byte 3 of 4\n";
	    ost << "    sipush 255\n"; // 0xFF
	    ost << "    iand\n";
	    ost << "    bipush 8\n";
	    ost << "    ishl\n";
	    ost << "    dup_x2\n";
	    ost << "    pop\n";
	    ost << "    iconst_1\n";
	    ost << "    iadd\n";
	    ost << "    baload ;    byte 4 (LSB) of 4\n";
	    ost << "    sipush 255\n"; // 0xFF
	    ost << "    iand\n";
	    ost << "    ior\n";
	    ost << "    ior\n";
	    ost << "    ior    ; end of 32 bit memory access on RHS\n";
	    break;
        case 16:
	    ost << "    dup2\n";
	    ost << "    baload ;    byte 1 (MSB) of 2\n";
	    ost << "    sipush 255\n"; // 0xFF
	    ost << "    iand\n";
	    ost << "    bipush 8\n";
	    ost << "    shl\n";
	    ost << "    dup_x2\n";
	    ost << "    pop\n";
	    ost << "    iconst_1\n";
	    ost << "    iadd\n";
	    ost << "    baload ;    byte 2 of 2\n";
	    ost << "    sipush 255\n"; // 0xFF
	    ost << "    iand\n";
	    ost << "    ior    ; end of 16 bit memory access on RHS\n";
	    if (cType.getSigned()) {  // sign-extend the 2 bytes just read
		ost << "    ior\n";
		ost << "    bipush 16\n";
		ost << "    ishl\n";	
		ost << "    bipush 16\n";
		ost << "    ishr   ; end of 16 bit memory access on RHS\n";
	    } else {
		ost << "    ior    ; end of 16 bit memory access on RH\n";
	    }
	    break;
        case 8:
	    if (cType.getSigned()) {  // sign-extend the byte just read
		ost << "    baload\n";
		ost << "    sipush 255\n"; // 0xFF
		ost << "    iand\n";
		ost << "    bipush 24\n";
		ost << "    ishl\n";	
		ost << "    bipush 24\n";
		ost << "    ishr ; end of 8 bit memory access on RHS\n";
	    } else {
		ost << "    iaload ; end of 8 bit memory access on RHS\n";
	    }
	    break;
    }
    return str(ost);
}
    
/*=============================================================================
 * FUNCTION: 	processUnaryOp
 * OVERWIEW:	Emit code for a unary operator not handled by processExpr().
 * PARAMETERS:  idx: The integer value of the unary operator's Semantic string.
 * RETURNS:	A string holding the bytecodes for the unary operation.	
 *===========================================================================*/

string processUnaryOp(int idx) {
    ostrstream ost;
    switch (idx) {
        case idNot:			// ~ operator (bitwise negation)
	    ost << "    iconst_m1\n";
	    ost << "    ixor\n";
	    break;

        case idNeg:			// unary - operator
	    ost << "    ineg\n";
	    break;
	    
        default: 
	    cerr << "processUnaryOp: unknown unary operator "
		 << theSemTable[idx].sName << " (" << idx << ")\n";
	    exit(1);
    }
    return string(str(ost));
}

/*=============================================================================
 * FUNCTION:    processBinaryOp()
 * OVERWIEW:    This function is used to translate Binary operators.  
 * PARAMETERS:  idx: Integer value of the binary operator's Semantic string.
 * RETURNS:     A string holding the bytecodes for the binary operation.
 *===========================================================================*/

string processBinaryOp(int idx) {
    switch(idx) {	// The function doesn't check the size of the operands.
        case idFPlus:
	    return string("    fadd\n");
        case idFPlusd: 
	    return string("    dadd\n");
        case idPlus:
	    return string("    iadd\n");
        case idFMinus:
	    return string("    fsub\n");
        case idFMinusd: 
	    return string("    dsub\n");
        case idMinus:	
	    return string("    isub\n");

        case idFMult: 
	    return string ("    fmul\n");
        case idFMultd: 
	    return string ("    dmul\n");
        case idMults:
	    return string ("    imul\n");
        case idMult: 
	    return string ("    imul    ; INCORRECT SINCE NOT UNSIGNED\n");

        case idFDiv: 
 	    return string("    fdiv\n");
        case idFDivd: 
 	    return string("    ddiv\n");
        case idDivs:
	    return string("    idiv\n");
        case idDiv:
	    return string("    idiv    ; INCORRECT SINCE NOT UNSIGNED\n");

        case idMod:
	    return string("    irem    ; INCORRECT SINCE NOT UNSIGNED\n");
        case idMods:
	    return string("    irem\n");

        case idShiftL: 
	    return string("    ishl\n");

        case idShiftR: 
	    return string("    iushr    ; logical right shift\n");	
        case idShiftRA: 
	    return string("    ishr\n");	

        case idBitOr:  
	    return string("    ior\n");

        case idBitAnd: 
	    return string("    iand\n"); 

        case idBitXor:
	    return string("    ixor\n");

        case idEquals: 
	    return string("    if_icmpeq ");

        case idNotEqual:
	    return string("    if_icmpne ");
	
        case idLess:
        case idLessUns: 	// caller must deal with unsigned
	    return string("    if_icmplt ");

        case idGtr: 
        case idGtrUns:		// caller must deal with unsigned
	    return string("    if_icmpgt ");

        case idLessEq: 
        case idLessEqUns:	// caller must deal with unsigned
	    return string("    if_icmple ");

        case idGtrEq: 
        case idGtrEqUns: 	// caller must deal with unsigned
	    return string("    if_icmpge ");

        default:
	    cerr << "processBinaryOp: unknown binary operator "
		 << theSemTable[idx].sName << " (" << idx << ")\n";
	    exit(1);
    }
}

/*============================================================================
 * FUNCTION: 	processIntConst
 * OVERVIEW:	This function emits code for an integer constant.
 * PARAMETERS:	pExp: the pointer to the idIntConst Expression
 *		cType: the current type to use for code generation.
 * RETURNS:	A string of bytecodes.
 *===========================================================================*/

string processIntConst(const SemStr* pExp, Type cType) {
    int num = pExp->getSecondIdx();
    ostrstream ost;
    if ((num >= -128) && (num <= 127)) {
	ost << "    bipush ";
    } else if ((num >= -32768) && (num <= 32767)) { 
	ost << "    sipush ";
    } else {
	ost << "    ldc ";
    }
    if (cType.getSigned()) {
        ost << num << "\n";
    } else {
        ost << (unsigned)num << "   ; unsigned int\n";
    }
    return str(ost);
}

/*============================================================================
 * FUNCTION: 	validateTypeForJava
 * OVERVIEW:	Verify that "type" of size "typeSize" in the expression pointed
 *              to by "pExp" is a legal JVM type.
 * PARAMETERS:	pExp: The pointer to the Expression containing the type.
 *		cType: The type to check for being a legal JVM type.
 *		typeSize: The size in bits of "type".
 * RETURNS:	Nothing. Exits the program after printing an error if the
 *              type is not valid for the JVM.
 *===========================================================================*/

void validateTypeForJava(const SemStr* pExp, Type cType, int typeSize) {
    if (cType.getType() == FLOATP) {
	if ((typeSize != 32) && (typeSize != 64)) {
	    cerr << "Unsupported size " << typeSize
		 << " in expression " << *pExp
		 << ", must be 32 or 64\n";
	    exit(1);
	}
    } else {
	if ((typeSize != 8) && (typeSize != 16) &&
	    (typeSize != 32) && (typeSize != 64)) {
	    cerr << "Unsupported size " << typeSize
		 << " in expression " << *pExp
		 << ", must be 32 or 64\n";
	    exit(1);
	}
    }
}

/*============================================================================
 * FUNCTION: 	processSize
 * OVERVIEW:	Emit code for an idSize SemStr: size cast.
 * PARAMETERS:	pExp: The pointer to the SemStr.
 *		cType: The current type to use for code generation.
 * RETURNS:	A string of bytecodes.
 *===========================================================================*/

string processSize(const SemStr* pExp, Type cType) {
    ostrstream ost;
    int opIdx = pExp->getFirstIdx();
    
    Type toType(cType);
    int toSize = cType.getSize();
    // If idSize and no idSgnEx, make it unsigned
    toType.setSigned(false);
    if (!(toType -= cType)) {
	ostrstream errOst;
	errOst << "processSize: type conflict in expression " << *pExp;
	errOst << " at " << hex << rtlAddr << "\n";
	warning(str(errOst));
	cType = toType;
    }
    
    Type fromType(cType);
    int fromSize = pExp->getSecondIdx();
    fromType.setSize(fromSize);
    // For idSize, make the from type unsigned, so the cast of the memory
    // reference will be correct.
    fromType.setSigned(false);
    bool fromIsFloat = (fromType.getType() == FLOATP);
    
    validateTypeForJava(pExp, fromType, fromSize);
    validateTypeForJava(pExp, toType, toSize);
    ost << "    ; " << theSemTable[opIdx].sName << " (fromSize="
	<< fromSize << ", toSize=" << toSize << ")\n";
    SemStr* pSub = pExp->getSubExpr(0);
    ost << processExpr(pSub, fromType);
    
    switch (toSize) {
    case 8:
	if (fromIsFloat) {	// does idSize even make sense for float?
	    cerr << "processFSize: unsupported size " << toSize
		 << " in expression " << *pExp
		 << ", must be 32 or 64\n";
	    exit(1);
	} else {
	    if (fromSize == 64) {
		ost << "    l2i\n";
	    }
	    ost << "    sipush 255\n"; // 0xFF
	    ost << "    iand\n";
	}
	break;
	
    case 16:
	if (fromIsFloat) {
	    cerr << "processFSize: unsupported size " << toSize
		 << " in expression " << *pExp
		 << ", must be 32 or 64\n";
	    exit(1);
	} else {
	    if (fromSize == 64) {
		ost << "    l2i\n";
	    }
	    ost << "    ldc 65535\n"; // 0xFFFF
	    ost << "    iand\n";
	}
	break;
	
    case 32:
	if (fromIsFloat) {
	    if (fromSize == 64) {
		ost << "    f2i\n";
	    }
	} else {
	    if (fromSize == 64) {
		ost << "    l2i\n";
	    }
	}
	break;
	
    case 64:
	if (fromIsFloat) {
	    if (fromSize <= 32) {
		ost << "    f2d\n";
	    }
	} else {
	    if (fromSize <= 32) {
		ost << "    i2l\n";
	    }
	}
	break;
    }
    return str(ost);
}

/*============================================================================
 * FUNCTION: 	processFSize
 * OVERVIEW:	Emit code for an idFsize SemStr: FP size conversion.
 * PARAMETERS:	pExp: The pointer to the SemStr.
 *		cType: The current type to use for code generation.
 * RETURNS:	A string of bytecodes.
 *===========================================================================*/

string processFSize(const SemStr* pExp, Type cType) {
    ostrstream ost;
    int opIdx = pExp->getFirstIdx();
    
    Type toType(cType);
    int toSize = pExp->getThirdIdx();
    toType.setSize(toSize);
    toType.setType(FLOATP);

    Type fromType(cType);
    int fromSize = pExp->getSecondIdx();
    fromType.setSize(fromSize);
    fromType.setType(FLOATP);
    
    if (!(toType -= cType)) {
	ostrstream errOst;
	errOst << "processFSize: type conflict in expression " << *pExp;
	errOst << " at " << hex << rtlAddr << "\n";
	warning(str(errOst));
	cType = toType;
    }
    
    validateTypeForJava(pExp, fromType, fromSize);
    validateTypeForJava(pExp, toType, toSize);
    if ((fromSize != 32) && (fromSize != 64)) {
	cerr << "processFSize: bad floating point from size "
	     << fromSize << " in expression " << *pExp
	     << " at " << hex << rtlAddr << "\n";
	exit(1);
    }
    
    ost << "    ; " << theSemTable[opIdx].sName << " (fromSize="
	<< fromSize << ", toSize=" << toSize << ")\n";
    SemStr* pSub = pExp->getSubExpr(0);
    ost << processExpr(pSub, fromType);
    
    switch (toSize) {
    case 32:
	if (fromSize == 64) {
	    ost << "    d2f\n";
	}
	break;
	
    case 64:
	if (fromSize == 32) {
	    ost << "    f2d\n";
	}
	break;
	
    default:
	cerr << "processFSize: unsupported size " << toSize
	     << " in expression " << *pExp
	     << ", must be 32 or 64\n";
	exit(1);
    }
    return str(ost);
}

/*============================================================================
 * FUNCTION: 	processItoF
 * OVERVIEW:	Emit code for an idItoF SemStr: int to FP (and size) conversion
 * PARAMETERS:	pExp: The pointer to the SemStr.
 *		cType: The current type to use for code generation.
 * RETURNS:	A string of bytecodes.
 *===========================================================================*/

string processItoF(const SemStr* pExp, Type cType) {
    ostrstream ost;
    int opIdx = pExp->getFirstIdx();
    
    Type toType(cType);
    int toSize = pExp->getThirdIdx();
    toType.setSize(toSize);
    toType.setType(FLOATP);
    
    Type fromType(cType);
    int fromSize = pExp->getSecondIdx();
    fromType.setSize(fromSize);
    fromType.setType(INTEGER);
    fromType.setSigned(true);
    
    if (!(toType -= cType)) {
	ostrstream errOst;
	errOst << "processItoF: type conflict in expression " << *pExp;
	errOst << " at " << hex << rtlAddr << "\n";
	warning(str(errOst));
	cType = toType;
    }
    
    validateTypeForJava(pExp, fromType, fromSize);
    validateTypeForJava(pExp, toType, toSize);
    ost << "    ; " << theSemTable[opIdx].sName << " (fromSize="
	<< fromSize << ", toSize=" << toSize << ")\n";
    SemStr* pSub = pExp->getSubExpr(0);
    ost << processExpr(pSub, fromType);
    
    switch (toSize) {
    case 32:
	if (fromSize <= 32) {
	    ost << "    i2f\n";
	} else {  // fromSize == 64
	    ost << "    l2f\n";
	}
	break;
	
    case 64:
	if (fromSize <= 32) {
	    ost << "    i2d\n";
	} else {  // fromSize == 64
	    ost << "    l2d\n";
	}
	break;
	
    default:
	cerr << "processItoF: unsupported size " << toSize
	     << " in expression " << *pExp
	     << ", must be 32 or 64\n";
	exit(1);
    }
    return str(ost);
}

/*============================================================================
 * FUNCTION: 	processFtoI
 * OVERVIEW:	Emit code for an idFtoI SemStr: FP to int (and size) conversion
 * PARAMETERS:	pExp: The pointer to the SemStr.
 *		cType: The current type to use for code generation.
 * RETURNS:	A string of bytecodes.
 *===========================================================================*/

string processFtoI(const SemStr* pExp, Type cType) {
    ostrstream ost;
    int opIdx = pExp->getFirstIdx();
    
    Type toType(cType);
    int toSize = pExp->getThirdIdx();
    toType.setSize(toSize);
    toType.setType(INTEGER); 
    toType.setSigned(true);
    
    Type fromType(cType);
    int fromSize = pExp->getSecondIdx();
    fromType.setSize(fromSize);
    fromType.setType(FLOATP);
    
    if (!(toType -= cType)) {
	ostrstream errOst;
	errOst << "processFtoI: type conflict in expression " << *pExp;
	errOst << " at " << hex << rtlAddr << "\n";
	warning(str(errOst));
	cType = toType;
    }
    
    validateTypeForJava(pExp, fromType, fromSize);
    validateTypeForJava(pExp, toType, toSize);
    if ((fromSize != 32) && (fromSize != 64)) {
	cerr << "processFtoI: bad floating point from size "
	     << fromSize << " in expression " << *pExp
	     << " at " << hex << rtlAddr << "\n";
	exit(1);
    }
    ost << "    ; " << theSemTable[opIdx].sName << " (fromSize="
	<< fromSize << ", toSize=" << toSize << ")\n";
    SemStr* pSub = pExp->getSubExpr(0);
    ost << processExpr(pSub, fromType);
    
    switch (toSize) {
    case 8:
	if (fromSize == 32) {
	    ost << "    f2i\n";
	    ost << "    i2b\n";
	} else {  // fromSize == 64
	    ost << "    d2i\n";
	    ost << "    i2b\n";
	}
	break;
	
    case 16:
	if (fromSize == 32) {
	    ost << "    f2i\n";
	    ost << "    i2s\n";
	} else {  // fromSize == 64
	    ost << "    d2i\n";
	    ost << "    i2s\n";
	}
	break;
	
    case 32:
	if (fromSize == 32) {
	    ost << "    f2i\n";
	} else {  // fromSize == 64
	    ost << "    d2i\n";
	}
	break;
	
    case 64:
	if (fromSize == 32) {
	    ost << "    d2i\n";
	} else {  // fromSize == 64
	    ost << "    d2l\n";
	}
	break;
    }
    return str(ost);
}

/*============================================================================
 * FUNCTION: 	processTrunc
 * OVERVIEW:	Emit code for an idTruncX SemStr: integer truncate where
 *              X is "s" for signed and "u" for unsigned.
 * PARAMETERS:	pExp: The pointer to the SemStr.
 *		cType: The current type to use for code generation.
 * RETURNS:	A string of bytecodes.
 *===========================================================================*/

string processTrunc(const SemStr* pExp, Type cType) {
    ostrstream ost;
    int opIdx = pExp->getFirstIdx();
    
    Type toType(cType);
    int toSize = pExp->getThirdIdx();
    toType.setSize(toSize);
    toType.setType(INTEGER);
    toType.setSigned(opIdx == idTruncs);   // signed iff idTruncs
    
    Type fromType(cType);
    int fromSize = pExp->getSecondIdx();
    fromType.setSize(fromSize);
    fromType.setType(INTEGER);
    fromType.setSigned(opIdx == idTruncs);
    
    if (!(toType -= cType)) {
	ostrstream errOst;
	errOst << "processTrunc: type conflict in expression " << *pExp;
	errOst << " at " << hex << rtlAddr << "\n";
	warning(str(errOst));
	cType = toType;
    }
    
    validateTypeForJava(pExp, fromType, fromSize);
    validateTypeForJava(pExp, toType, toSize);
    ost << "    ; " << theSemTable[opIdx].sName << " (fromSize="
	<< fromSize << ", toSize=" << toSize << ")\n";
    SemStr* pSub = pExp->getSubExpr(0);
    ost << processExpr(pSub, fromType);

    switch (toSize) {
    case 8:
	if (fromSize == 64) {
	    ost << "    l2i\n";
	}
	// Are the following shift/masks really necessary?
	if (toType.getSigned()) {
	    ost << "    bipush 24\n";
	    ost << "    ishl\n";
	    ost << "    bipush 24\n";
	    ost << "    iushr\n";
	} else {
	    ost << "    sipush 255\n"; // 0xFF
	    ost << "    iand\n";
	}
	break;
	
    case 16:
	if (fromSize == 64) {
	    ost << "    l2i\n";
	}
	// Are the following shift/masks really necessary?
	if (toType.getSigned()) {
	    ost << "    bipush 16\n";
	    ost << "    ishl\n";
	    ost << "    bipush 16\n";
	    ost << "    iushr\n";
	} else {
	    ost << "    ldc 65535\n"; // 0xFFFF
	    ost << "    iand\n";
	}
	break;
	
    case 32:
	if (fromSize == 64) {
	    ost << "    l2i\n";
	}
	break;
	
    case 64:
	if (fromSize <= 32) {
	    ost << "    i2l\n";
	}
	break;
    }
    return str(ost);
}

/*============================================================================
 * FUNCTION: 	processZfill
 * OVERVIEW:	Emit code for an idZfill SemStr: integer zero fill
 * PARAMETERS:	pExp: The pointer to the SemStr.
 *		cType: The current type to use for code generation.
 * RETURNS:	A string of bytecodes.
 *===========================================================================*/

string processZfill(const SemStr* pExp, Type cType) {
    ostrstream ost;
    int opIdx = pExp->getFirstIdx();
    
    Type toType(cType);
    int toSize = pExp->getThirdIdx();
    toType.setSize(toSize);
    toType.setSigned(false);
    
    Type fromType(cType);
    int fromSize = pExp->getSecondIdx();
    fromType.setSize(fromSize);
    fromType.setType(INTEGER);
    fromType.setSigned(false);
    
    if (!(toType -= cType)) {
	ostrstream errOst;
	errOst << "processZfill: type conflict in expression " << *pExp;
	errOst << " at " << hex << rtlAddr << "\n";
	warning(str(errOst));
	cType = toType;
    }
    
    validateTypeForJava(pExp, fromType, fromSize);
    validateTypeForJava(pExp, toType, toSize);
    ost << "    ; " << theSemTable[opIdx].sName << " (fromSize="
	<< fromSize << ", toSize=" << toSize << ")\n";
    SemStr* pSub = pExp->getSubExpr(0);
    ost << processExpr(pSub, fromType);
    switch (toSize) {
    case 8:
	ost << "    sipush 255\n";
	ost << "    iand\n";
	break;
	
    case 16:
	if (fromSize == 8) {
	    ost << "    sipush 255\n";
	    ost << "    iand\n";
	} else if ((fromSize == 16) || (fromSize == 32)) {
	    ost << "    ldc 65535\n"; // 0xFFFF
	    ost << "    iand\n";
	} else { // fromSize == 64
	    ost << "    l2i\n";
	    ost << "    ldc 65535\n"; // 0xFFFF
	    ost << "    iand\n";
	}
	break;
	
    case 32:
	if (fromSize == 8) {
	    ost << "    sipush 255\n";
	    ost << "    iand\n";
	} else if (fromSize == 16) {
	    ost << "    ldc 65535\n"; // 0xFFFF
	    ost << "    iand\n";
	} else if (fromSize == 64) {
	    ost << "    l2i\n";
	}
	break;
	
    case 64:
	if (fromSize == 8) {
	    ost << "    sipush 255\n";
	    ost << "    iand\n";
	    ost << "    i2l\n";
	} else if (fromSize == 16) {
	    ost << "    ldc 65535\n"; // 0xFFFF
	    ost << "    iand\n";
	    ost << "    i2l\n";
	} else if (fromSize == 32) {
	    ost << "    i2l\n";
	    ost << "    bipush 32\n";
	    ost << "    lshl\n";
	    ost << "    bipush 32\n";
	    ost << "    lushr\n";    // shifts in 0's
	}
	break;
    }
    return str(ost);
}

/*============================================================================
 * FUNCTION: 	processSgnEx
 * OVERVIEW:	Emit code for an idSgnEx SemStr: integer sign extend
 * PARAMETERS:	pExp: The pointer to the SemStr.
 *		cType: The current type to use for code generation.
 * RETURNS:	A string of bytecodes.
 *===========================================================================*/

string processSgnEx(const SemStr* pExp, Type cType) {
    ostrstream ost;
    int opIdx = pExp->getFirstIdx();
    
    Type toType(cType);
    int toSize = pExp->getThirdIdx();
    toType.setSize(toSize);
    toType.setSigned(true);
    
    Type fromType(cType);
    int fromSize = pExp->getSecondIdx();
    fromType.setSize(fromSize);
    fromType.setType(INTEGER);
    fromType.setSigned(true);
    
    if (!(toType -= cType)) {
	ostrstream errOst;
	errOst << "processSgnEx: type conflict in expression " << *pExp;
	errOst << " at " << hex << rtlAddr << "\n";
	warning(str(errOst));
	cType = toType;
    }
    
    validateTypeForJava(pExp, fromType, fromSize);
    validateTypeForJava(pExp, toType, toSize);
    ost << "    ; " << theSemTable[opIdx].sName << " (fromSize="
	<< fromSize << ", toSize=" << toSize << ")\n";
    SemStr* pSub = pExp->getSubExpr(0);
    ost << processExpr(pSub, fromType);
    
    switch (toSize) {
    case 8:
	if (fromSize == 64) {
	    ost << "    l2i\n";
	    ost << "    i2b\n";
	} else if (fromSize == 32) {
	    ost << "    i2b\n";
	} else if (fromSize == 16) {
	    ost << "    bipush 16\n";
	    ost << "    ishl\n";
	    ost << "    bipush 16\n";
	    ost << "    ishr\n";
	    ost << "    i2b\n";
	} else if (fromSize == 8) {
	    ost << "    bipush 24\n";
	    ost << "    ishl\n";
	    ost << "    bipush 24\n";
	    ost << "    ishr\n";
	}
	break;
	
    case 16:
	if (fromSize == 64) {
	    ost << "    l2i\n";
	    ost << "    i2s\n";
	} else if (fromSize == 32) {
	    ost << "    i2s\n";
	} else if (fromSize == 16) {
	    ost << "    bipush 16\n";
	    ost << "    ishl\n";
	    ost << "    bipush 16\n";
	    ost << "    ishr\n";
	} else if (fromSize == 8) {
	    ost << "    bipush 24\n";
	    ost << "    ishl\n";
	    ost << "    bipush 24\n";
	    ost << "    ishr\n";
	}
	break;
	
    case 32:
	if (fromSize == 64) {
	    ost << "    l2i\n";
	} else if (fromSize == 16) {
	    ost << "    bipush 16\n";
	    ost << "    ishl\n";
	    ost << "    bipush 16\n";
	    ost << "    ishr\n";
	} else if (fromSize == 8) {
	    ost << "    bipush 24\n";
	    ost << "    ishl\n";
	    ost << "    bipush 24\n";
	    ost << "    ishr\n";
	}
	break;
	
    case 64:
	if (fromSize == 32) {
	    ost << "    i2l\n";
	} else if (fromSize == 16) {
	    ost << "    bipush 16\n";
	    ost << "    ishl\n";
	    ost << "    bipush 16\n";
	    ost << "    ishr\n";
	    ost << "    i2l\n";
	} else if (fromSize == 8) {
	    ost << "    bipush 24\n";
	    ost << "    ishl\n";
	    ost << "    bipush 24\n";
	    ost << "    ishr\n";
	    ost << "    i2l\n";
	}
	break;
    }
    return str(ost);
}

/*============================================================================
 * FUNCTION: 	jvmForHRTLLocal
 * OVERVIEW:	Return the index of a JVM local that will hold a HRTL variable.
 * PARAMETERS:	hrtlLocal: index for a HRTL variable.
 * RETURNS:	The index of a JVM local to contain the HRTL variable.
 *===========================================================================*/

int jvmForHRTLLocal(int hrtlLocal) {
    int jvmLocal;
    if (hrtlLocal >= firstLocal) {
        assert(hrtlLocal <= lastLocal);
	// A local variable: use the hrtlLocal'th local after FIRST_JVM_LOCAL
	jvmLocal = (FIRST_JVM_LOCAL + (hrtlLocal - firstLocal));
    } else {
	// A parameter. 
        jvmLocal = hrtlLocal;
        if (processingMain) {
            // If processing _main(), which is static, reserve local 0 to
            // hold its "this" reference.
            jvmLocal = (hrtlLocal + 1);
        }
    }
    return jvmLocal;
}

/*============================================================================
 * FUNCTION: 	processExpr
 * OVERVIEW:	Emit code for an expression SemStr.
 * PARAMETERS:	pExp: The pointer to the Expression
 *		cType: the current type to use for code generation.
 * RETURNS:	A string of bytecodes.
 *===========================================================================*/

string processExpr(const SemStr* pExp, Type cType) {
    int idx = (pExp->getFirstIdx());
    int n = theSemTable[idx].iNumVarArgs;
    ostrstream ost;

    switch (n) {
        case 0: {
	    switch (idx) {
	        case idIntConst: 
		    return processIntConst(pExp, cType);
		
	        case idCodeAddr:
		    cerr << "processExpr: idCodeAddr not supported in JVM backend\n";
		    return string("idCodeAddr NOT SUPPORTED");
                    break;
		    
	        case idFltConst: {
                    union _u {
                        struct {
                            int i1; int i2;
                        } ii;
                        double d;
                    } u;
                    u.ii.i1 = pExp->getSecondIdx();
                    u.ii.i2 = pExp->getThirdIdx();
                    ost << "    ldc " << u.d << "\n";
                    return str(ost);
                }

	        case idVar: {
		    int localNum = pExp->getLastIdx();
		    ost << "    iload  " << jvmForHRTLLocal(localNum) 
                        << "    ; load var " << *pExp << "\n";
		    return str(ost);
		}
		
	        case idAFP:
		    return "    iload 8      ; AFP\n";
		    
                case idAGP: 
		    cerr << "processExpr: idAGP not supported in JVM backend\n";
		    return string("idAGP NOT SUPPORTED");
                    break;

                case idPC:
                    cerr << "processExpr: idPC not supported in JVM backend\n";
                    return string("idPC NOT SUPPORTED");
                    break;

	        default:
		    if (idx >= idMachSpec) {
                        // This is a special register, such as %Y.
                        const string& strg = theSemTable[idx].sName;
			ost << "processExpr: " << strg
			    << " is not supported in JVM backend\n";
			error(str(ost));
                        return string("?");
                    } else {
                        ost << "processExpr: unknown zero parameter operator "
			    << idx << " in expression ";
                        pExp->print(ost);
                        error(str(ost));
                        return string("?");
                    }
	    }
	} /* case 0 */
	
        case 1: {
	    // Handles Unary expressions, regs of form r[], and memory
	    // locations such as m[];
	    switch (idx) {
	        case idRegOf: 
		    ost << "    iload  " << processRegOf(pExp) 
                        << "    ; load register " << *pExp << "\n";
		    return str(ost);
		  
	        case idMemOf:
		    return processMemOf(pExp, cType);
		  
	        case idSignExt: {
		    // Ensure that cType is signed. For now, assume that ! is
		    // "commutative", i.e. the presence of the ! implies that
		    // the subexpression as well as the result are signed
		    SemStr* pSub = pExp->getSubExpr(0);
		    ost << "    ; sign extend\n";
		    cType.setSigned(true);
		    ost << processExpr(pSub, cType);
		    ost << "    ; end of sign extend\n";
		    delete pSub;
		    return str(ost);
		} 
		  
	        case idSize:	// Size cast
		    return processSize(pExp, cType);
			
	        case idFsize:	// Floating point size conversion
		    return processFSize(pExp, cType);
	    
                case idItof:	// Integer to FP (and size) conversion
		    return processItoF(pExp, cType);
		
                case idFtoi:	// FP to integer (and size) conversion
		    return processFtoI(pExp, cType);
		
                case idTruncu:	// Integer truncate (unsigned)
                case idTruncs:	// Integer truncate (signed)
		    return processTrunc(pExp, cType);
		
                case idZfill:	// Integer zero fill
		    return processZfill(pExp, cType);
		
	        case idSgnEx:	// Integer sign extend
		    return processSgnEx(pExp, cType);

	        default: {	// Other unary expression
		    const SemStr* Arg = pExp->getSubExpr(0);
		    ost << processExpr(Arg, cType);
		    ost << processUnaryOp(idx);
		    delete Arg;
		    return str(ost);
		}
	    } /* of switch(idx) */
	} /* of case 1: unary expressions */
	break;
	
        case 2: {
	    // Binary Expressions.
	    const SemStr* pArg1 = pExp->getSubExpr(0);
	    const SemStr* pArg2 = pExp->getSubExpr(1);
	    // For unsigned operators (relational and shift), cast the operands
            // to unsigned
	    switch(idx) {
                case idLessUns:
                case idGtrUns:
                case idLessEqUns:
	        case idGtrEqUns: {
                    Type uType(cType);
                    uType.setSigned(false);
                    ost << processExpr(pArg1, uType);
		    switch (uType.getSize()) {
		    case 8:
			ost << "    bipush 8\n";
			ost << "    iand\n";
			ost << processExpr(pArg2, uType);
			ost << "    bipush 8\n";
			ost << "    iand\n";
			ost << processBinaryOp(idx);
			break;
		    case 16:
			ost << "    bipush 16\n";
			ost << "    iand\n";
			ost << processExpr(pArg2, uType);
			ost << "    bipush 16\n";
			ost << "    iand\n";
			ost << processBinaryOp(idx);
			break;
		    case 32:
			ost << "    i2l\n";
			ost << "    ldc2_w 4294967295\n"; // 0x00000000FFFFFFFF
			ost << "    land\n";
			ost << processExpr(pArg2, uType);
			ost << "    i2l\n";
			ost << "    ldc2_w 4294967295\n"; // 0x00000000FFFFFFFF
			ost << "    land\n";
			ost << "    lcmp\n";
			if (idx == idLessUns) {
			    ost << "    iflt ";
			} else if (idx == idGtrUns) {
			    ost << "    ifgt ";
			} else if (idx == idLessEqUns) {
			    ost << "    ifle ";
			} else { // idx == idGtrEqUns
			    ost << "    ifge ";
			}
			break;
		    case 64:
			ost << processExpr(pArg2, uType);
			ost << "    invokestatic "
			    << initCapital(prog.getNameNoPath()).c_str()
			    << "/unsignedLongCompare(JJ)I";
			if (idx == idLessUns) {
			    ost << "    iflt ";
			} else if (idx == idGtrUns) {
			    ost << "    ifgt ";
			} else if (idx == idLessEqUns) {
			    ost << "    ifle ";
			} else { // idx == idGtrEqUns
			    ost << "    ifge ";
			}
			break;
		    default:
			cerr << "processExpr: unsupported size "
			     << uType.getSize()
			     << " in expression ";
			pExp->print(ost);
			cerr << str(ost) << "\n";
			exit(1);
		    }
		    return str(ost);
                }

		case idShiftR:
                case idMult:
                case idDiv:
                case idMod: {
                    Type uType(cType);
                    uType.setSigned(false);
                    ost << processExpr(pArg1, uType);
                    ost << processExpr(pArg2, uType);
                    ost << processBinaryOp(idx);
		    return str(ost);
                }
		
                case idFPlus:
                case idFMinus:
                case idFMult:
                case idFDiv: {
                    Type fType(FLOATP, 32);
                    ost << processExpr(pArg1, fType);
                    ost << processExpr(pArg2, fType);
                    ost << processBinaryOp(idx);
		    return str(ost);
                }
		
                case idFPlusd:
                case idFMinusd:
                case idFMultd:
                case idFDivd: {
                    Type fType(FLOATP, 64);
                    ost << processExpr(pArg1, fType);
                    ost << processExpr(pArg2, fType);
                    ost << processBinaryOp(idx);
		    return str(ost);
                }
		
                case idFPlusq:
                case idFMinusq:
                case idFMultq:
                case idFDivq: {
                    cerr << "processExpr: 128 bit floating point not supported"
			 << ", expression " << *pExp
			 << " at " << hex << rtlAddr << "\n";
		    exit(1);
                }
            }

	    // Not one of the above binary operators. 
	    ost << processExpr(pArg1, cType);
	    ost << processExpr(pArg2, cType);
	    ost << processBinaryOp(idx);
            delete pArg1;
	    delete pArg2;
            return str(ost);
	} /* of case 2: binary expressions */  

        case 3: {
            // Ternary Expressions.
	    const SemStr* pArg1 = pExp->getSubExpr(0);
            const SemStr* pArg2 = pExp->getSubExpr(1);
            const SemStr* pArg3 = pExp->getSubExpr(2);
            int val2 = pArg2->getSecondIdx();
            if (pArg2->getFirstIdx() != idIntConst) val2 = -1;
            int val3 = pArg3->getSecondIdx();
            if (pArg3->getFirstIdx() != idIntConst) val3 = -1;
            switch (idx) {
                case idAt:      // a@b:c  e.g. r[tmpl]@32:63
                    if (*pArg2 == *pArg3) {
                        // Special optimization for b == c
                        // We can use (((unsigned)a >> b) & 1);
                        Type uType(cType);
                        uType.setSigned(false);
                        ost << processExpr(pArg1, uType);
                        ost << processExpr(pArg2, cType);
                        switch (uType.getSize()) {
                        case 64:
                            ost << "    lushr    ; logical right shift\n";
                            ost << "    lconst_1\n";
                            ost << "    land\n";
                            break;
                        default:
                            ost << "    iushr    ; logical right shift\n";
                            ost << "    iconst_1\n";
                            ost << "    iand\n";
                            break;
                        }
                    } else if (val2 == 32 && val3 == 63) {
                        // Special case for getting the upper word of a long
                        ost << processExpr(pArg1, cType);
                        ost << "    bipush 32\n";
                        ost << "    lshr\n";
                        ost << "    l2i\n";
                    } else if (val2 == 0 && val3 == 31) {
                        // Special case for getting the lower word of a long
                        ost << processExpr(pArg1, cType);
                        ost << "    l2i\n";
                    } else {
                        // Must use (((unsigned)a >> b) & ((1<<(c-b+1)-1)))
                        // Note: assumes b < c!!
                        cerr << "processExpr: idAt expression too complex"
                             << ", expression " << *pExp
                             << " at " << hex << rtlAddr << "\n";
                    }
                    break;
                default:
                    error("Unknown ternary expression");
            }
            delete pArg1;  delete pArg2;  delete pArg3; 
            return str(ost);
        } /* end case 3: ternary expressions */

        default:
	    cerr << "processExpr: unknown type of expression"
                 << ", expression " << *pExp
                 << " at " << hex << rtlAddr << "\n";
	    exit(1);
    } /* of switch (n) */
}

/*============================================================================
 * FUNCTION: 	processRT
 * OVERVIEW:	This function is used to parse each statement and call the 
 *	 	function processExpr for a RT.
 * PARAMETER:	pRt: A pointer to the RT tp be processed. 
 *		os:  The stream to which the output is to be written.
 * RETURNS:	<none>
 *===========================================================================*/

void processRT(RT* pRt, ostrstream& os) {
    switch (pRt->getKind()) {
    case RTASSGN: {   
	RTAssgn* pRtA = (RTAssgn*)pRt;
	SemStr* pLoc = pRtA->getLHS();
	// Initial type. If a memOf, use the type of the RHS (because
	// memory is typeless). Otherwise use the type of the LHS.
	Type cType;
	if (pLoc->getFirstIdx() == idMemOf) {
	    cType = pRtA->getRHS()->getType();
	} else {
	    cType = pLoc->getType();
	}
	int bitSize = (int)(cType.getSize());
	string instr;

	// Record the instructions for the RHS expr if the LHS is m[].
	// These instructions may be used more than once in the code that
	// stores the RHS data a byte at a time. Otherwise, if it is v[] or
	// r[], don't store those instructions, just emit them.
	// NB: this only works if the RHS is idempotent!
	if (pLoc->getFirstIdx() == idMemOf) {
	    instr = processExpr(pRtA->getRHS(), cType);
	}  else {	
	    os << processExpr(pRtA->getRHS(), cType);
	}

	switch(pLoc->getFirstIdx()) {
	    case idRegOf:
		os << "    istore " << processRegOf(pLoc)
		   << "    ; store to reg " << *pLoc << "\n";
		break;

	    case idMemOf: {	
		string temp = initCapital(prog.getNameNoPath());
		const char *class_name = temp.c_str();
		SemStr* pSub = pLoc->getSubExpr(0);
		os << "    getstatic " << class_name << "/memory [B\n    ; "
		   << bitSize << " bit mem store on LHS\n";
		os << processExpr(pSub, cType);
		
		switch (bitSize) {
		case 8:
		    os << instr;	    // emit the RHS value
		    os << "    sipush 255\n";
		    os << "    iand\n";
		    break;
		    
		case 16:
		    os << "    dup2    ; store high-order byte, byte[0]\n";
		    os << instr;	    // emit the RHS value 
		    os << "    bipush 8\n";
		    os << "    ishr\n";
		    os << "    sipush 255\n";
		    os << "    iand\n";
		    os << "    bastore\n";
		    os << "    iconst_1\n";
		    os << "    iadd\n";     // increment the store "address"
		    os << "    ; store low-order byte, byte[1]\n";
		    os << instr;	    // emit the RHS value again
		    os << "    sipush 255\n";
		    os << "    iand\n";
		    break;
		    
		case 32:
		    os << "    dup2    ; store high-order byte, byte[0]\n";
		    os << instr;
		    os << "    bipush 24\n";
		    os << "    ishr\n";
		    os << "    sipush 255\n";
		    os << "    iand\n";
		    os << "    bastore\n";
		    os << "    iconst_1\n";
		    os << "    iadd\n";     // increment the store "address"
		    os << "    dup2    ; store byte[1]\n";
		    os << instr;
		    os << "    bipush 16\n";
		    os << "    ishr\n";
		    os << "    sipush 255\n";
		    os << "    iand\n";
		    os << "    bastore\n";
		    os << "    iconst_1\n";
		    os << "    iadd\n";     // increment the store "address"
		    os << "    dup2    ; store byte[2]\n";
		    os << instr;
		    os << "    bipush 8\n";
		    os << "    ishr\n";
		    os << "    sipush 255\n";
		    os << "    iand\n";
		    os << "    bastore\n";
		    os << "    iconst_1\n";
		    os << "    iadd\n";     // increment the store "address"
		    os << "    ; store low-order byte, byte[3]\n";
		    os << instr;
		    os << "    sipush 255\n";
		    os << "    iand\n";
		    break;
		    
		default:
		    cerr << "processRT: unsupported store of " << bitSize
			 << " bits\n";
		    exit(1);
		}
		os << "    bastore    ; end of " << bitSize
		   << " bit mem store on LHS\n";
		break;	
	    }
	    
	    case idVar: {
		int localNum = pLoc->getLastIdx();
		os << "    istore " << jvmForHRTLLocal(localNum)
                   << "    ; store var " << *pLoc << "\n";
		break;
	    }
	    
	    default: {
		cerr << "Warning: processRT: Dest type " << pLoc->getFirstIdx()
		     << " not supported. Type should be"
		     << " r[]. v[] or m[]\n";
	    }
	} /* end switch(pLoc->getFirstIdx()) {... */
	os << "\n";
	break;
    }
	
    case RTFLAGDEF: {
	cerr << "processRT: RT Flag Definition not supported.";
	exit(1);
    }

    case RTFLAGCALL: {
	// Just ignore these
	// cout << "RT Flag called\n";
	break;
    }

    default: {
	cerr << "processRT: Only Assignment statements are supported\n";
	exit(1);
    }
    } /* end switch(pRt->getKind()) */
} 

/*===========================================================================
 * FUNCTION: 	processHL
 * OVERVIEW:	This function is used to translate high level RTs 
 *		such as CALL, JUMP, etc.
 * PARAMETERS:	pRTL: Pointer to the HL RT to be translated.
 *		pBB:  Pointer to the basic block to which the RT belongs to.
 *		os :  The output stream
 *		proc: Pointer to the procedure which contains the RT 
 * RETURNS:	<none>
 *===========================================================================*/

void processHL(const HRTL *pRTL, PBB pBB, ostrstream& os, Proc *proc) {
    switch (pRTL->getKind()) {
    case CALL_HRTL: {
	const HLCall* pCall = static_cast<const HLCall*>(pRTL);
	ADDRESS uDest = pCall->getFixedDest();
	
	Proc* proc = prog.findProc(uDest);
	const char* pName = pCall->getDestName();
	if ((uDest == NO_ADDRESS) && (pName == NULL)) {
	    // Register call, which we don't support
	    prog.bRegisterCall = true;
	    ostrstream ost;
	    ost << "processHL: call through register at "
		<< hex << pCall->getAddress();
	    error(str(ost));
	    // Put *something* into the file, for debugging
	    os << "    ; *** processHL: call through register at "
	       << hex << pCall->getAddress() << "\n"; 
	    break;
	}
	pName = proc->getName();
	if ((pName == NULL) &&
	        ((proc == 0 || (proc == (Proc*)-1) || (pName == 0)))) {
	    ostrstream ost;
	    ost << "processHL: call to unnamed function at "
		<< hex << pCall->getAddress();
	    error(str(ost));
	    // Put *something* into the file, for debugging
	    os << "    ;   *** processHL: call to unnamed function at "
	       << hex << pCall->getAddress() << "\n"; 
	    break;
	}
	const SemStr retLoc = pCall->getReturnLoc();
	const list<SemStr>& params = pCall->getParams();
	int nParams = params.size();
	
	os << "    ; call to method "
	   << initCapital(prog.getNameNoPath()).c_str() << "/_" << pName;
        if (nParams == 0) {
            os << "\n";
        } else if (nParams == 1) {
            os << " with 1 parameter\n";
        } else {
            os << " with " << nParams << " parameters\n";
        }

    list<SemStr>::const_iterator pp;
	for (pp = params.begin(); pp != params.end(); pp++) {
        // Assumption: no conversion is needed
	    os << processExpr(&(*pp), pp->getType());
	}
	// emit the call to the procedure. 
	os << "    invokestatic " << initCapital(prog.getNameNoPath()).c_str()
	   << "/_" << proc->getName() << "(";
	for (int i=0;  i < nParams;  i++) {
	    os << "I";
	}
	os << ")";
	
	// emit the return type and code to store any result
	if (proc->getReturnType() == TVOID) {
	    os << "V\n";
	}  else {
	    os << "I\n";
	    const SemStr* pRetLoc = &retLoc;       
	    switch(pRetLoc->getFirstIdx()) {
	        case idRegOf: {
		    os << "    istore " << processRegOf(pRetLoc)
		       << "    ; store result to reg " << *pRetLoc << "\n";
		    break;
		}

	        case idVar: {
		    int localNum = pRetLoc->getLastIdx();
		    os << "    istore " << jvmForHRTLLocal(localNum)
		       << "    ; store result to var " << *pRetLoc << "\n";
		    break;
		}

	        default: 
		    // This case occurs when the return type is the RETLOCN
		    // macro. In all test cases used, the RETLOCN locn is the
		    // first location, i.e. location 10, so we assume that
		    // the return location is always in location 10.
		    os << "    istore 10    ; store result\n";	
		    break;
	    }
	    os << "\n";
	} 
	  
	// This is used in case there is a call/restore. It is untested.
	if (pBB->isJumpReqd()) {
	    int label = pBB->getOutEdges()[0]->getLabel();
	    os << "    goto L" << dec << label << "\n";
	}
	break;
    }
	
    case RET_HRTL: {
	// Used for return statements. If the return type is a void,
	// a return statement is used, if the return type is a int then
	// the int is loaded on the top of the stack, and a ireturn is
	// used. Right now only void and int return types are supported.
	int n = pRTL->getNumRT();
	// Process each RT
	for (int i=0;  i < n;  i++) {
	    processRT(pRTL->elementAt(i), os);
	}	
	  
	const SemStr* retLocn = proc->getReturnLoc(); // Return	location.
	if (proc->getReturnType() == INTEGER) {
	    // Return type is int
            os << processExpr(retLocn, retLocn->getType());
	    os << "    ireturn    ; return int " << *retLocn << "\n";
	}  else {
	    // Return type is void
	    os << "    return    ; return VOID " << *retLocn << "\n";
	}
	break;
    }
	
    case JUMP_HRTL: {
	// Emit goto Ldest;
	// Check if fixed destination. That means check if the 
	// destination address is known (fixed) or whether 
	// it is dynamic and will be known at runtime.
	const HLJump* jump = static_cast<const HLJump*>(pRTL); 
	if (jump->getDest()->getFirstIdx() == idIntConst) { 
	    int label = pBB->getOutEdges()[0]->getLabel();
	    os << "    goto L" << dec << label << "\n";
	}  else { 
	    // Eventually, we must call a runtime interpreter. We do not yet
	    // handle jumps to dynamic locations: i.e., locations that are
	    // only known at runtime.
	    ostrstream ost;
	    ost << "    ; *** processHL: register jump at "
		<< hex << pRTL->getAddress();
	    ost << "    ; *** PROGRAM WILL NOT RUN!";
	    error(str(ost));
	}
	break;
    }
	
    case JCOND_HRTL: {
	const HLJcond* jcond = static_cast<const HLJcond*>(pRTL);
	SemStr* pCond = jcond->getCondExpr();
	if (pCond == 0) { 
	    ostrstream ost;
	    ost << "processHL: HLJcond " << jcond <<
		" has no high level condition\n";
	    error(str(ost));
	    break;		// For now... was assert
	}
	Type condType(pCond->getType());
	switch (jcond->getCond()) {
	    case HLJCOND_JUL:  case HLJCOND_JULE:
	    case HLJCOND_JUGE: case HLJCOND_JUG:
		condType.setSigned(false);
		break;
	    default:
		break;          // Leave it alone
	}
	os << processExpr(pCond, condType); 

	// Print the label of the target BB if the jump condition is true. 
	int label = pBB->getOutEdges()[0]->getLabel();
	os << "L" << dec << label << "\n";
	
	// If the "fall through" outedge doesn't actually fall through, the
	// "jump reqd" bit should be set, and we must generate a jump
	if (pBB->isJumpReqd()) { // Emit goto Ldest;
	    label = pBB->getOutEdges()[1]->getLabel();
	    os << "    goto L" << dec << label << "\n";
	}
	break;
    }
	
    case SCOND_HRTL: {
	ostrstream ost;
	ost << "processHL: SCOND not supported, at ";
	ost << hex << pRTL->getAddress();
	error(str(ost));
	// Put *something* into the file, for debugging
	os << "    ; *** processHL: SCOND not supported, at "
	   << hex << pRTL->getAddress() << "\n";
	break;
    }
    
    case NWAYJUMP_HRTL: {
	ostrstream ost;
	ost << "processHL: NWAYJUMP not supported, at "
	    << hex << pRTL->getAddress();
	error(str(ost));
	// Put *something* into the file, for debugging
	os << "    ; *** processHL: NWAYJUMP not supported, at "
	   << hex << pRTL->getAddress() << "\n";
	break;
    }
	
    default:
	cerr << "processHL: Unknown HL RTL at "
	     << hex << pRTL->getAddress() << "\n";
	exit(1);
    }
}


/*=============================================================================
 * FUNCTION:   getOutputFileName()
 * OVERVIEW:   Creates the name of the file where the bytecodes would be
               written
 * PARAMETERS: <none>
 * RETURNS:    The filename where the bytecodes are written as a string.
 *===========================================================================*/
			
string getOutputFileName() {
    string Name = progOptions.outDir + prog.getNameNoPath() + string(".j");
    return Name;
}

/*=============================================================================
 * FUNCTION: 	emitClassFileHeader()
 * OVERVIEW:    This method writes the standard header of the .j file which
 *		tells the program that the functions used are in the file 
 *	       	TranslatedFile.java
 * PARAMETERS:  os: The stream where the output is to be written.
 * RETURNS:	<none>
 *===========================================================================*/

void emitClassFileHeader(ostrstream& os) {
    string name;
    const char *class_name;
    name = initCapital(prog.getNameNoPath());
    class_name = name.c_str();
	
    os << ".class public " << class_name << "\n";
    os << ".super TranslatedFile" << "\n";
    os << "\n";
    os << ".method public <init>()V" << "\n";
    os << "    .limit stack   5" << "\n";
    os << "    .limit locals 16" << "\n";
    os << "    aload_0" << "\n";
    os << "    invokenonvirtual TranslatedFile/<init>()V" << "\n";
    os << "    return" << "\n";
    os << ".end method" << "\n";
    os << "\n";
    os << ".method public static main([Ljava/lang/String;)V"<< "\n";
    os << "    .limit stack   5" << "\n";
    os << "    .limit locals 16" << "\n";
    os << "    new " << class_name << "\n";
    os << "    dup" << "\n";
    os << "    invokespecial " << class_name << "/<init>()V" << "\n";
    os << "    aload_0" << "\n";
    os << "    invokevirtual "<< class_name << "/realMain([Ljava/lang/String;)V\n";
    os << "    return" << "\n";
    os << ".end method" << "\n";
    os << "\n";
    os << ".method public name()Ljava/lang/String;" <<"\n";
    os << "    .limit stack  3" << "\n";
    os << "    .limit locals 2" << "\n";
    os << "    ldc \"" <<class_name << "\"\n";
    os << "    areturn\n";
    os << ".end method\n";
    os << "\n";
}

/*=============================================================================
 * FUNCTION: 	translate2j
 * OVERVIEW:	This is the main function for translating HRTL code into Java
 *		bytecode. It is called once for every procedure.
 * PARAMETERS:	proc: Pointer to the function which has to be translated. 
 * RETURNS:	<none>
 *===========================================================================*/

void translate2j(UserProc* proc) {
    BB_CIT bbit;
    HRTLList_CIT it;
    PBB pBB;
    int i;
    ostrstream os;		// stream written to a ".j" file
    string p = getOutputFileName();
    if (!streamOpen) {		// only open the output stream once
	outStream.open(p.c_str());
	streamOpen = true;
	emitClassFileHeader(os); // emit the header
    }
    if (outStream == 0) {
	ostrstream ost;
	ost << "Could not open " << p << " for writing";
	error(str(ost));
	exit(1);
    }
  
    // Initilize the file scope globals
    regNumbers.clear();	        // Clear the map for each method
    afpArrayAllocated = false;

    Cfg* pCfg = proc->getCFG();
    PBB pFirstBB = pCfg->getFirstBB(bbit);
    if (proc->getNativeAddress() != pFirstBB->getLowAddr()) { 
	pCfg->setLabel(proc->getEntryBB());
    }
  
    // The 2 variables firstLocal and lastLocal are used to map HRTL variables
    // to JVM local variables.
    firstLocal = proc->getFirstLocalIndex();
    if (firstLocal == -1) {	// there are no HRTL local vars
	firstLocal = 0;
    }
    lastLocal = proc->getLastLocalIndex();
    if (lastLocal == -1) {	// there are no HRTL local vars
	lastLocal = 0;
    }

    // Set nextFreeLocForReg, the first location to be assigned to registers,
    // which is after all HRTL variables for the procedure
    nextFreeLocForReg = (FIRST_JVM_LOCAL + (lastLocal-firstLocal) + 1);

    // Emit the function signature.	
    string name = proc->getName();
    const char *method_name = name.c_str();
    
    processingMain = (strcmp(method_name, "main") == 0);
    if (processingMain) {
	// The main method can not be static.
	os << ".method public _" << method_name << "(";
    } else {
	os << ".method public static _" << method_name << "(";
    }
    int numArgs = proc->getNumArgs();
    for (i = 0;  i < numArgs;  i++) {
	os << "I";		// Assumption: parameters are only ints.
    }
    os <<")";
    if (proc->getReturnType() == TVOID) {
	os << "V";
    } else {			// must be int (Assumption)
	os << "I"; 
    }
    if (numArgs == 1) {
        os << "  ; parameter: (";
    } else {
        os << "  ; parameters: (";
    }
    proc->printParams(os);
    os << ")\n";
    
    // Set up the stack and locals 
    os << "    .limit stack   " << METHOD_STACK_SIZE << "\n";
    os << "    .limit locals  " << METHOD_LOCAL_VARIABLES << "\n";

    // A bit of debug output
    os << "    ; HRTL locals [" << firstLocal << ".." << lastLocal
       << "] stored in [" << FIRST_JVM_LOCAL
       << ".." << (FIRST_JVM_LOCAL+(lastLocal-firstLocal)) << "]" 
       << ", first reg loc=" << nextFreeLocForReg << "\n";

    // If any incoming parameters are HRTL registers (not variables), store
    // them into the expected local JVM variable. See the comment at the
    // start of this file for an explanation of the JVM frame layout.
    // An invoke* bytecode implicitly stores the incoming arguments in locals
    // [0..(numArgs-1)] (if not static -- i.e. main -- [0..numArgs]).
    bool paramsStored = false;
    for (i = 0;  i < numArgs;  i++) {
	const SemStr *param = proc->getParameter(i);
        if (param->getFirstIdx() == idRegOf) {
            // Main is not static, so it's 1st arg is in local var 1 (not 0).
            // Note that we ignore the "this" reference in local var 0.
            int localVar = (processingMain? (i+1) : i);
            os << "    iload " << localVar
               << "     ; load param " << (i+1) << " in local var " << localVar
               << "\n";
            os << "    istore " << processRegOf(param)
               << "    ; store to reg: " << *param << "\n";
            paramsStored = true;
        }
    }
    if (paramsStored) {
        os << "\n";
    }

    // Set up the (simulated) abstract frame pointer (AFP)
    os << "    iconst_0\n";
    os << "    istore 8     ; store AFP in local 8\n";
    os << "\n";

    // If the first BB is not the entry point, jump to the real entry BB
    if (proc->getNativeAddress() != pFirstBB->getLowAddr()) { 
	os << "    goto L" << dec << proc->getEntryBB()->getLabel()
	   << "    ; go to the real entry BB\n";
    } 
 	
    // Process each HRTL
    for (pBB = pFirstBB;  pBB != 0;  pBB = pCfg->getNextBB(bbit)) {
	// Emit a label for the start of this BB
	int label = pBB->getLabel();
	if (label) { 
	    os << "L" << dec << label << ":" << endl;
	}
	
	for (it = pBB->getHRTLs()->begin();  it != pBB->getHRTLs()->end();
	    it++)  {
	    // Save the RTL's address for error messages
            rtlAddr = (*it)->getAddress();
	    // Process High Level RT's.
	    if ((*it)->getKind() != LOW_LEVEL_HRTL) {
		processHL(*it, pBB, os, proc);	
	    }  else {						
		// Assumption: Low Level RTs are only assignment statements. 
		int n = (*it)->getNumRT();	
		for (int i = 0;  i < n;  i++) {
		    os << "    ; ";
		    (*it)->elementAt(i)->print(os);
		    os << "\n";
		    processRT((*it)->elementAt(i), os); 
		}
	    }
	}
    }
    os << ".end method ; _" << method_name << "\n\n";
    outStream << str(os);	// Write the stream to the output file.
}
