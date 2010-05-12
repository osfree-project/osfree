/*
 * Copyright (C) 1996-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       decoder.h
 * OVERVIEW:   The interface to the instruction decoder. This is a
 *             machine dependent file but needs to be put in the
 *             general HEADERS directory as a number of machine
 *             indepedent files need it.
 *
 * Copyright (C) 1996-2001, The University of Queensland, BT group
 * Copyright (C) 2001, Sun Microsystems, Inc
 *============================================================================*/

/* 
 * $Revision: 1.42 $
 * Created by Cristina - 96
 * 2 Feb 98 - Cristina
 *  changed prototype for decode_instr() to return an RTL instance.
 * 3 Feb 98 - Cristina
 *  added the RTL dictionary as parameter to decode_instr().
 * 19 Feb 98 - Cristina
 *  pass RTlist as reference parameter to decode_instr() instead of
 *      returning this object.
 * 3 Mar 98 - Cristina
 *  changed unsigned for ADDRESS in decode_instr() prototype.
 * 5 Mar 98 - Cristina 
 *  decode_instr() returns a boolean (false if no known instruction was  
 *      matched at the given address). 
 * 3 Mar 99 - Doug
 *  added interface decoder and put everything into the NJMCDecoder class.
 * 22 Apr 99 - Doug: procedure parameter for decodeInstruction now has default
 *                   value of NULL
 * 11 Aug 99 - Mike: Added size arg to dis_Eaddr (for movsx etc)
 * 19 Nov 99 - Mike: Removed the isRestore and RESTORE_INFO stuff
 * 24 Jan 99 - Mike: split FS1 into FS1S, FS1D, FS1Q etc
 * 15 Feb 00 - Mike: Added many function definitions for mc68k
 * 26 Jun 00 - Mike: Changed names of ADDR and the like to DIS_ADDR; there are
 *              now more name collisions because of the ordering of #includes
 * 19 Jul 00 - Cristina: added macro addressToPC(), now required by
 *                   the ML toolkit (version Apr 5 14:56:28 EDT 2000)
 *  1 Aug 00 - Cristina: changed SETS macro to reply on (nextPC-hostPC) 
 *					instead of numBytes.  These changes are due to different 
 *					semantics in the mltk toolkit (version Apr 5 2000). 
 * 28 Sep 00 - Mike: Fixed DIS_IDX to use fp_names[]
 * 02 Oct 00 - Mike: Added DIS_IDXP1 for FLD.STi
 * 17 Oct 00 - Mike: Added overloaded dis_Reg to use numbered (as opposed to
 *              named) registers
 * 24 Mar 01 - Mike: Macros for 286
 * 28 Mar 01 - Mike: 286 sr16[] -> sr2[]; DIS_SR16 -> DIS_SR2
 * 13 Apr 01 - Cristina: Added virtual NJMCDecoder::decodeAssemblyInstruction
 *              method to support plain vanilla disassemblers, as well as
 *              private helper routines dis_RegImm() and dis_Eaddr().
 * 16 Apr 01 - Cristina: Added macros associated with DISASM, to display
 *              an ascii representation of registers and addressing modes.
 * 08 May 01 - Mike: Added HPPA disassember functions
 * 14 May 01 - Mike: Added some new DCTI cases to ICLASS
 * 15 May 01 - Mike: Fixed the new DCTI classes
 * 26 Jun 01 - Nathan: Added instantiateNamedParam
 * 19 Jul 01 - Simon: added substituteCallArgs() and dis_x_addr_shift()
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 */

#ifndef _DECODER_H_
#define _DECODER_H_

//#include "rtl.h"

/*==============================================================================
 * SPARC specific type declarations.
 *============================================================================*/

// These are the instruction classes defined in "A Transformational Approach to
// Binary Translation of Delayed Branches" for SPARC instructions.
enum ICLASS {
    NCT,            // Non Control Transfer
    SD,             // Static Delayed
    DD,             // Dynamic Delayed
    SCD,            // Static Conditional Delayed
    SCDAN,          // Static Conditional Delayed, Anulled if Not taken
    SCDAT,          // Static Conditional Delayed, Anulled if Taken
    SU,             // Static Unconditional (not delayed)
    SKIP,           // Skip successor
//  TRAP,           // Trap
    NOP,            // No operation (e.g. sparc BN,A)
    // HPPA only
    DU,             // Dynamic Unconditional (not delayed)
    NCTA            // Non Control Transfer, with following instr Anulled
};

/*==============================================================================
 * The DecodeResult struct contains all the information that results from
 * calling the decoder. This prevents excessive use of confusing
 * reference parameters.
 *============================================================================*/
struct DecodeResult {
    /**
     * Resets all the fields to their default values.
     */
    void reset();

    /**
     * The number of bytes decoded in the main instruction
     */
    int numBytes;

    /**
     * The RTL constructed (if any).
     */
    HRTL* rtl;

    /**
     * Indicates whether or not a valid instruction was decoded.
     */
    bool valid;

    /**
     * The class of the instruction decoded. Will be one of the classes
     * described in "A Transformational Approach to Binary Translation of
     * Delayed Branches".
     * NOTE: this field only makes sense for the SPARC platform but it
     * may become more generalised once the DCTL language is
     * implemented.
     */
    ICLASS type;

    /*
     * If non zero, this field represents a new native address to be used as
     * the out-edge for this instruction's BB. At present, only used for
     * the SPARC call/add caller prologue
     */
    ADDRESS forceOutEdge;

};

/*==============================================================================
 * The NJMCDecoder class is a class that contains NJMC generated decoding
 * methods.
 *============================================================================*/
class NJMCDecoder {
public:
    /**
     * Decodes the machine instruction at pc and returns an RTL instance for
     * the instruction.
     */
    DecodeResult& decodeInstruction (ADDRESS pc, int delta,
        UserProc* procedure = NULL);

    /**
     * Disassembles the machine instruction at pc and returns the number of
     * bytes disassembled. Assembler output goes to global _assembly
     */
    int decodeAssemblyInstruction (ADDRESS pc, int delta);

    /*
     * Constructor sets the association with the supporting
     * dictionaries.
     */
    NJMCDecoder(RTLInstDict& rtlDict, CSR& csr);

    /*
     * Alternate constructor for disassemblers (only!)
     */
    NJMCDecoder(BinaryFile* pbf);


private:
    
    /**
     * Given an instruction name and a variable list of SemStr's
     * representing the actual operands of the instruction, use the
     * RTL template dictionary to return the instantiated RTL
     * representing the semantics of the instruction. This method also
     * displays a disassembly of the instruction if the relevant
     * compilation flag has been set.
     * NOTE: the memory used by the SemStrs is cleaned up by this
     * method.
     */
    list<RT*>* instantiate(ADDRESS pc, const char* name, ...);

    /**
     * Similarly, given a parameter name and a list of SemStr's
     * representing sub-parameters, return a fully substituted
     * SemStr for the whole expression
     */
    SemStr *instantiateNamedParam(const char *name, ...);

    /**
     * In the event that it's necessary to synthesize the call of
     * a named parameter generated with instantiateNamedParam(),
     * this substituteCallArgs() will substitute the arguments that
     * follow into the semantic string.
     * Should only be used after ss = instantiateNamedParam(name, ..);
     */
    void substituteCallArgs(const char *name, SemStr* &ss, ...);
    /**
     * Decodes a machine instruction and returns an instantiated
     * list of RTs.
     */
    list<RT*>* decodeLowLevelInstruction (ADDRESS start,
        ADDRESS pc, DecodeResult& result);

    /**
     * Decode the instructions in a given range and return the
     * concatenation of all their lists of RTs.
     */
    list<RT*>* getLowLevelRTs(ADDRESS start,int length,
        ADDRESS pc, DecodeResult& result);

#ifdef DISASM
    /**
     * Functions for disassembly support of effective addresses and
     * other complex operands.
     */
    char * dis_RegImm (ADDRESS pc);
    char * dis_Eaddr (ADDRESS pc);

    /* HPPA */
    void    dis_c_xd(ADDRESS pc);
    void    dis_c_addr(ADDRESS pc);
    void    dis_c_c(ADDRESS pc);
    void    dis_c_null(ADDRESS pc);
    void    dis_c_bit(ADDRESS pc);
    void    dis_c_wcr(ADDRESS pc);
    void    dis_flt_fmt(int fmt);
    void    dis_faddr(ADDRESS faddr);
    void    dis_c_faddr(ADDRESS c_faddr);
    char*   dis_freg(int regNum);

#else

    /**
     * Various functions to decode the operands of an instruction into
     * a SemStr representation.
     */
    SemStr* dis_Reg(const char* reg);
    SemStr* dis_Reg(int regNum);
    SemStr* dis_Num(unsigned immed);
    SemStr* dis_Eaddr(ADDRESS pc, int size = 0);
    SemStr* dis_Mem(ADDRESS ps);
    SemStr* dis_RegImm(ADDRESS pc);

    /* HPPA */
    SemStr* c_c(ADDRESS pc, int &cond);
    SemStr* c_bit(ADDRESS hostpc);
    SemStr* dis_x_addr_shift(ADDRESS hostpc);
	SemStr* dis_xd(ADDRESS pc);
    SemStr* dis_c_addr(ADDRESS pc);
    SemStr* dis_c_bit(ADDRESS pc);
    SemStr* dis_c_wcr(ADDRESS pc);
    SemStr* dis_ct(ADDRESS pc);
    SemStr* dis_Freg(int regNum, int fmt);
    SemStr* dis_Creg(int regNum);
    SemStr* dis_Sreg(int regNum);

    void low_level(list<RT*>*& RTs, ADDRESS hostPC, ADDRESS pc,
        DecodeResult& result, ADDRESS& nextPC);

#endif

    /*
     * Next set are only needed for mc68k decoder. Doesn't hurt to define these
     * for all decoders
     */

SemStr* BTA(ADDRESS d, DecodeResult& result, ADDRESS pc);
void    pIllegalMode(ADDRESS pc);
SemStr* alEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr, int size);
SemStr* amEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr, int size);
SemStr* awlEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr, int size);
SemStr* cEA(ADDRESS ea, ADDRESS pc, int size);
SemStr* dEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr, int size);
SemStr* daEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr, int size);
SemStr* dBEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr, int size); 
SemStr* dWEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr, int size);
SemStr* maEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr, int size);
SemStr* msEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr, int size);
SemStr* mdEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr, int size);
SemStr* mrEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr, int size);
SemStr* rmEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr, int size);
SemStr* alEAX(ADDRESS eax, ADDRESS, DecodeResult& result, ADDRESS pc, int size);
SemStr* amEAX(ADDRESS eax, ADDRESS, DecodeResult& result, ADDRESS pc, int delta,
    int size);
SemStr* awlEAX(ADDRESS eax, ADDRESS, DecodeResult& result, ADDRESS pc,int delta,
    int size);
SemStr* cEAX(ADDRESS eax, ADDRESS, DecodeResult& result, ADDRESS pc, int delta,
    int size);
SemStr* dEAX(ADDRESS eax, ADDRESS, DecodeResult& result, ADDRESS pc,int delta,
    int size);
SemStr* daEAX(ADDRESS eax, ADDRESS, DecodeResult& result, ADDRESS pc, int size);
SemStr* dBEAX(ADDRESS eax, ADDRESS, DecodeResult& result, ADDRESS pc,int delta,
    int size);
SemStr* dWEAX(ADDRESS eax, ADDRESS, DecodeResult& result, ADDRESS pc,int delta,
    int size);
SemStr* maEAX(ADDRESS eax, ADDRESS, DecodeResult& result, ADDRESS pc, int size);
SemStr* msEAX(ADDRESS eax, ADDRESS, DecodeResult& result, ADDRESS pc,int delta,
    int size);
SemStr* msEAXL(ADDRESS eaxl, int d32,DecodeResult& result,ADDRESS pc, int size);
SemStr* mdEAX(ADDRESS eax, ADDRESS, DecodeResult& result, ADDRESS pc, int size);
SemStr* mrEAX(ADDRESS eax, ADDRESS, DecodeResult& result, ADDRESS pc,int delta,
    int size);
SemStr* rmEAX(ADDRESS eax, ADDRESS, DecodeResult& result, ADDRESS pc, int size);

SemStr* pDDirect(int r2, int size);
SemStr* pADirect(int r2, int size);
SemStr* pIndirect(int r2, int size);
SemStr* pPostInc(int r2, int& bump, int& bumpr, int size);
SemStr* pPreDec(int r2, int& bump, int& bumpr, int size);
SemStr* pADisp(int d16, int r, int size);
SemStr* pAIndex(int d8, int r, int iT, int iR, int iS, int size);
SemStr* pPcDisp(ADDRESS label, int delta, int size);
SemStr* pPcIndex(int d8, int iT, int iR, int iS, ADDRESS pc, int size);
SemStr* pAbsW(int d16, int size);
SemStr* pAbsL(int d32, int size);
SemStr* pImmB(int d8);
SemStr* pImmW(int d16);
SemStr* pImmL(int d32);
void    pNonzeroByte(ADDRESS pc);


public:
    /*
     * Function to return the RTLInstDict object. This saves passing around
     * more parameters
     */
    RTLInstDict& getRtlDict();

    /*
     * This used to be the UNCOND_JUMP macro; it's extended to handle jumps to
     * other procedures
     */
    void unconditionalJump(const char* name, int size, ADDRESS relocd,
        UserProc* proc, int delta, ADDRESS pc, list<RT*>* RTs,
        DecodeResult& result);

    /*
     * String for the constructor names (displayed with use "-c")
     */
    char    constrName[84];

private:

#ifdef DISASM
    /*
     * Disassemblers may beed a pointer to the BinaryFile object that has loaded
     * their input binary image, to get symbols for calls, etc
     */
    BinaryFile* pBF;
#else
    /*
     * These are references to the RTL template dictionary and CSR
     * analysis object that a decoder depends upon.
     * Not needed (or indeed wanted) for disassembler
     */
    RTLInstDict& rtlDict;
    CSR& csr;
#endif

    /*
     * Needed by (at least) HP PA/Risc: A list of RTs for before the main
     * sematics, and one for afterwards. Example: mb (modify before) and
     * ma (modify after) completers
     */
    list<RT*>*  preInstSem;         // Pre-instruction semantics
    list<RT*>*  postInstSem;        // Post-instruction semantics

};

// One mc68k specific (so far) function, used to guess whether a given
// pc-relative address is the start of a function

/*
 * Does the instruction at the given offset correspond to a caller prologue?
 * NOTE: Implemented in the decoder.m files
 */
bool isFuncPrologue(ADDRESS hostPC);


/*==============================================================================
 * These are the macros that each of the .m files depend upon.
 *============================================================================*/
#ifdef  DEBUG_DECODER
#define SHOW_ASM(output) cout<< hex << pc << dec << ": " << output << endl;
#else
#define SHOW_ASM(output)
#endif

/**
 * addresstoPC returns the raw number as the address.  PC could be an
 * abstract type, in our case, PC is the raw address.
 */
#define addressToPC(pc)  pc

#if SRC == SPARC
#ifdef DISASM
#define DIS_RD      (rd_names[rd])
#define DIS_RS1     (rs1_names[rs1])
#define DIS_RS      (rs1_names[rs])
#define DIS_RS2     (rs2_names[rs2])
#define DIS_FDS     (fds_names[fds])
#define DIS_FDD     (fdd_names[fdd])
#define DIS_FDQ     (fdq_names[fdq])
#define DIS_FS1S    (fs1s_names[fs1s])
#define DIS_FS1D    (fs1d_names[fs1d])
#define DIS_FS1Q    (fs1q_names[fs1q])
#define DIS_FS2S    (fs2s_names[fs2s])
#define DIS_FS2D    (fs2d_names[fs2d])
#define DIS_FS2Q    (fs2q_names[fs2q])
#define DIS_CD      (cd_names[cd])

#define DIS_ROI     (dis_RegImm(roi))
#define DIS_ADDR    (dis_Eaddr(addr))

#else
#define DIS_RD      (dis_Reg(rd_names[rd]))
#define DIS_RS1     (dis_Reg(rs1_names[rs1]))
#define DIS_RS      (dis_Reg(rs1_names[rs]))
#define DIS_RS2     (dis_Reg(rs2_names[rs2]))
#define DIS_FDS     (dis_Reg(fds_names[fds]))
#define DIS_FDD     (dis_Reg(fdd_names[fdd]))
#define DIS_FDQ     (dis_Reg(fdq_names[fdq]))
#define DIS_FS1S    (dis_Reg(fs1s_names[fs1s]))
#define DIS_FS1D    (dis_Reg(fs1d_names[fs1d]))
#define DIS_FS1Q    (dis_Reg(fs1q_names[fs1q]))
#define DIS_FS2S    (dis_Reg(fs2s_names[fs2s]))
#define DIS_FS2D    (dis_Reg(fs2d_names[fs2d]))
#define DIS_FS2Q    (dis_Reg(fs2q_names[fs2q]))
#define DIS_CD      (dis_Reg(cd_names[cd]))

#define DIS_ROI     (dis_RegImm(roi))
#define DIS_ADDR    (dis_Eaddr(addr))
#endif          // #if DISASM

#elif (SRC == PENT) || (SRC == W32)

#define DIS_REG32   (dis_Reg(r32_names[reg]))
#define DIS_REG16   (dis_Reg(r16_names[reg]))
#define DIS_REG8    (dis_Reg(r8_names[reg]))
#define DIS_R32     (dis_Reg(r32_names[r32]))
#define DIS_R16     (dis_Reg(r16_names[r16]))
#define DIS_R8      (dis_Reg(r8_names[r8]))
#define DIS_SR16    (dis_Reg(sr16_names[sr16]))
#define DIS_IDX     (dis_Reg(fp_names[idx]))
// See 386Dir/decoder_low.m for what this is about
#define DIS_IDXP1   (dis_Reg(fp_names[(idx+1)%7]))

#define DIS_I32     (dis_Num(i32))
#define DIS_I16     (dis_Num(i16))
#define DIS_I8      (dis_Num(i8))
#define DIS_COUNT   (dis_Num(count))
#define DIS_OFF     (dis_Num(off))

#define DIS_EADDR32 (dis_Eaddr(Eaddr, 32))
#define DIS_EADDR16 (dis_Eaddr(Eaddr, 16))
#define DIS_EADDR8  (dis_Eaddr(Eaddr,  8))
#define DIS_MEM     (dis_Mem(Mem))
#define DIS_MEM16   (dis_Mem(Mem16))    // Probably needs changing
#define DIS_MEM32   (dis_Mem(Mem32))    // Probably needs changing
#define DIS_MEM64   (dis_Mem(Mem64))    // Probably needs changing
#define DIS_MEM80   (dis_Mem(Mem80))    // Probably needs changing

// Macros for branches. Note: don't put inside a "match" statement, since
// the ordering is changed and multiple copies may be made

#define UNCOND_JUMP(name, size, relocd) \
    HLJump* jump = new HLJump(pc, RTs); \
    result.rtl = jump; \
    result.numBytes = size; \
    jump->setDest(relocd-delta); \
    SHOW_ASM(name<<" "<<relocd)

#define COND_JUMP(name, size, relocd, cond) \
    HLJcond* jump = new HLJcond(pc, RTs); \
    result.rtl = jump; \
    result.numBytes = size; \
    jump->setDest(relocd-delta); \
    jump->setCondType(cond); \
    SHOW_ASM(name<<" "<<relocd)

#define SETS(name, dest, cond) \
    HLScond* scond = new HLScond(pc, RTs); \
    scond->setCondType(cond); \
    result.numBytes = 3; \
    result.rtl = scond; \
    SHOW_ASM(name<<" "<<dest)

/*==============================================================================
 * These are arrays used to map register numbers to their names.
 *============================================================================*/
extern char *r32_names[];
extern char *sr16_names[];
extern char *r8_names[];
extern char *r16_names[];
extern char *fp_names[];

/*==============================================================================
 * This array decodes scale field values in an index memory expression
 * to the scale factor they represent.
 *============================================================================*/
extern int  scale[];

#elif SRC == MC68K

#define DIS_DN(siz)     (pDDirect(n, siz))
#define DIS_AN          (pADirect(n, 32))
#define DIS_DN2(siz)    (pDDirect(n2, siz))
#define DIS_AN2         (pADirect(n2, 32))
#define DIS_I32         (dis_Num(i32))
#define DIS_I16         (dis_Num(i16))
#define DIS_I8          (dis_Num(i8))

/*
#define UNCOND_JUMP(name, size, disp) \
    HLJump* jump = new HLJump(pc, RTs); \
    result.rtl = jump; \
    result.numBytes = size; \
    jump->setDest(disp-delta); \
    SHOW_ASM(name<<" "<<disp)

#define COND_JUMP(name, size, disp, cond) \
    HLJcond* jump = new HLJcond(pc, RTs); \
    result.rtl = jump; \
    result.numBytes = size; \
    jump->setDest(disp-delta); \
    jump->setCondType(cond); \
    SHOW_ASM(name<<" "<<disp)
*/

#define UNCOND_JUMP(name, size, disp) \
    HLJump* jump = new HLJump(pc, RTs); \
    result.rtl = jump; \
    result.numBytes += size; \
    jump->setDest(disp); \
    SHOW_ASM(name<<" "<<disp)

#define COND_JUMP(name, size, disp, cond) \
    HLJcond* jump = new HLJcond(pc, RTs); \
    result.rtl = jump; \
    result.numBytes += size; \
    jump->setDest(disp); \
    jump->setCondType(cond); \
    SHOW_ASM(name<<" "<<disp)

// The following macro relies on the global values of nextPC and hostPC
// It used to define result.numBytes as result.numBytes += numBytes
#define SETS(name, dest, cond) \
    HLScond* scond = new HLScond(pc, RTs); \
    scond->setCondType(cond); \
    result.rtl = scond; \
    result.numBytes += (nextPC - hostPC); \
    SHOW_ASM(name<<" "<<dest)

#elif (SRC == X286)
#define DIS_R16     (dis_Reg(r16_names[r16]))
#define DIS_R8      (dis_Reg(r8_names[r8]))
#define DIS_SR2     (dis_Reg(sr2_names[sr2]))
#define DIS_IDX     (dis_Reg(fp_names[idx]))
// See 286/decoder_low.m for what this is about
#define DIS_IDXP1   (dis_Reg(fp_names[(idx+1)%7]))

#define DIS_I32     (dis_Num(i32))
#define DIS_I16     (dis_Num(i16))
#define DIS_I8      (dis_Num(i8))
#define DIS_COUNT   (dis_Num(count))
#define DIS_OFF     (dis_Num(off))

#define DIS_ADDR16 (dis_Eaddr(addr, 16))
#define DIS_ADDR8  (dis_Eaddr(addr,  8))
#define DIS_MEM32   (dis_Mem(Mem32))    // Probably needs changing
#define DIS_MEM64   (dis_Mem(Mem64))    // Probably needs changing
#define DIS_MEM80   (dis_Mem(Mem80))    // Probably needs changing

// Macros for branches. Note: don't put inside a "match" statement, since
// the ordering is changed and multiple copies may be made

#define UNCOND_JUMP(name, size, relocd) \
    HLJump* jump = new HLJump(pc, RTs); \
    result.rtl = jump; \
    result.numBytes = size; \
    jump->setDest(relocd-delta); \
    SHOW_ASM(name<<" "<<relocd)

#define COND_JUMP(name, size, relocd, cond) \
    HLJcond* jump = new HLJcond(pc, RTs); \
    result.rtl = jump; \
    result.numBytes = size; \
    jump->setDest(relocd-delta); \
    jump->setCondType(cond); \
    SHOW_ASM(name<<" "<<relocd)

#define SETS(name, dest, cond) \
    HLScond* scond = new HLScond(pc, RTs); \
    scond->setCondType(cond); \
    result.numBytes = 3; \
    result.rtl = scond; \
    SHOW_ASM(name<<" "<<dest)

/*==============================================================================
 * These are arrays used to map register numbers to their names.
 *============================================================================*/
extern char *sr2_names[];
extern char *r8_names[];
extern char *r16_names[];
extern char *fp_names[];


#endif


// General purpose
void not_used(int unwanted);

/**********************************
 * These are the fetch routines.
 **********************************/

/**
 * Returns the byte (8 bits) starting at the given address.
 */
Byte getByte(ADDRESS lc);

/**
 * Returns the word (16 bits) starting at the given address.
 */
SWord getWord(ADDRESS lc);

/**
 * Returns the double (32 bits) starting at the given address.
 */
DWord getDword(ADDRESS lc);


#endif
