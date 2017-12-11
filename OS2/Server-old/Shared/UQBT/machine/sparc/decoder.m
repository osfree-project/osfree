/*==============================================================================
 * FILE:       decoder.m
 * OVERVIEW:   Implementation of the SPARC specific parts of the
 *             NJMCDecoder class.
 *
 * Copyright (C) 1996-2001, The University of Queensland, BT group
 *============================================================================*/

/* $Revision: 1.54 $
 * Created by Cristina - 96
 * 5 Mar 96 - updated by Norman (part of selfdis)
 * 28 Aug 96 - Cristina
 *      nm doesn't exist as part of neg2 constructor; changed this line:
 *      | neg2 (rs2, rd)        => print(pr, "neg %s, %s", nm, DIS_RS2, RD);
 * 27 Dec 96 - Cristina
 *      added dtime() to measure cost of decoding.
 * 6 Jan 97 - Cristina
 *  this file replaces decode.c, arithmetic.c, memory.c and branches.c 
 *      from the elfdis distribution.  
 *  renamed sparc_disassemble() to decode_instr() which is called
 *      from elfdis.c:disassemble_text_section().
 *  removed matching to synthetic instructions as elfdis doesn't.
 * 7 Jan 97 - Cristina
 *  removed (synthetic) constructors bset, call_, jmp and set (OR) as 
 *      they're not matched in elfdis.
 * 8 Jan 97 - Cristina
 *  removed (synthetic) constructors clr, btst, neg, clrb, btog,
 *      deccc, inccc, bclr, clrh as they're not matched in elfdis.
 *  toolkit generates _ instead of , for annulled branches; replaced
 *      this manually in generated decode.c file. 
 *  relocation is done based on the address of where the instruction
 *      is being decoded (i.e. instr) instead of where it should
 *      be loaded (i.e. pc) according to the ELF info.  So 
 *      changed branch^a and call__ to adjust the relocated address:
 *          dis_rel(tgt-instr+pc));
 * 9 Jan 97 - Cristina
 *  unmatched instructions display .word msg now.
 *  added RDY, WRY and friends
 *  trap was missing name on RHS and doesn't take [] for trap number.
 *  reintroduced jmp synthetic and fixed code in elfdis.
 * 10 Jan 97 - Cristina
 *  disassembly output goes to a file (.s extension) rather than stdout.
 * 25 Feb 97 - Cristina
 *  elfdis.h in upper directory now.
 * 26 Feb 97 - Cristina
 *  removed references to dtime(), stats, iTiming & fp as these
 *      timing-related functions and variables are not
 *      required here.  STATS was defined in ../elfdis.h.
 * 10/Dec/97 Mike
 *      Added experimental code for symbols in .obj files (using
 *      relocation information)
 * 24/Dec/97 Mike
 *      Expanded buf[80] to buf[512] since long symbol names (over 350 chars)
 *      were overwriting memory!
 * 6 Jan 98 - Cristina
 *      Added getting an RTL instance for each matched instruction.
 *      Added include file rtl.h
 *      Removed synthetic instructions as they are not in the SSL spec; 
 *          only "real" instructions can be returned by RTL.
 *      Left floating point, coprocessor and asi instructions although they
 *          are not supported by the RTL interface at present.  They will 
 *          return an empty RTL.
 *      Updated parameters of restore and save.
 *      Reformatted the matching statement for readability.
 * 29 Jan 98 - Cristina
 *  Added returning an empty RTL instance for all instructions that are not
 *      supported in RTL yet (mainly floating point and coprocessor
 *      instructions).  An empty RTL instance is returned when 
 *      a call to RTL(name) doesn't find the name in the RTL dictionary.
 * 30 Jan 98 - Cristina
 *  renamed file to scanner.m for compatibility with other decoders.
 *  renamed arguments to decode_instr() to match those of other decoders.
 * 2 Feb 98 - Cristina
 *  removed parameters from constructors save and restore; they don't have 
 *      any arguments as they match the synthetic save and restore 
 *      constructors.  Pass "%g0" as the argument to RTL().
 *      Maybe we shouldn't be matching synthetics as this stage. 
 * 3 Feb 98 - Cristina
 *  added instruction dictionary as parameter to decode_instr() instead
 *      of using a global.
 * 17 Feb 98 - Cristina
 *  sethi needs to pass the imm22 as a string rather than as a %hi(imm22<<10)
 *      for the RTL() function to work correctly.
 *  added numToStr() to pass immediates as strings rather than labelled names.
 * 18 Feb 98 - Mike
 *  numToStr converts to decimal now. Pass imm22>>10 for sethi.
 *  Also fixed call and branch instructions.
 * 19 Feb 98 - Cristina
 *  pass HRTL as parameter to decode_instr() instead of returning it.
 * 2 Mar 98 - Cristina
 *  added adddress parameter to RTL().
 * 3 Mar 98 - Cristina
 *  changed unsigned to ADDRESS in decode_instr() and other functions.
 * 5 Mar 98 - Cristina
 *  decode_instr() returns a boolean (false if no instruction was 
 *      matched at the given address).
 * 9 Sep 98 - Mike
 *  Changed the RTLDict parameter to decode_instr() to be a reference.
 *  Should be more efficient, and was causing David problems
 * 27 Oct 98 - Mike
 *  removed getSym() and getReloc(). We don't want symbols here.
 * 02 Dec 98 - Mike: Removed the buf parameter to decode_instr() and removed
 *  the sprintf()s
 * 03 Mar 99 - Doug
 *  - put everything into the NJMCDecoder class and added decodeInterface
 *  - changed fetch_word to getDword so as to conform to this being hardcoded in
 *    the ML version of the toolkit
 *  - branch instruction decoding split into 2 branches as ML tlkt doesn't seem
 *    to handle the use of the '^' operator very well
 * 30 Mar 99 - Mike: added missing parameters to instantiateRTL for several
 *    instructions
 * 31 Mar 99 - Mike: fixed branch,a problems
 * 08 Apr 99 - Mike: Changes for HEADERS
 * 12 Apr 99 - Doug: Decoding a static call now records if it is followed by a
 *                   RESTORE
 * 22 Apr 99 - Mike: Added createJcond to preserve the branch condition
 * 22 Apr 99 - Doug: procedure parameter for decodeInstruction now has default
 *                   value of NULL
 * 7  May 99 - Doug: Put createJcond into the NJMCDecoder class.
 * 19 May 99 - Mike: Decoder sets destination of HL RTLs now; use of SET_RTs
 *                      macro to only create low level RTLs when required
 * 25 May 99 - Mike: result.numBytes is the real number of bytes now. For
 *                  sparc, rtl.getNumBytes should never be used
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 15 Jul 99 - Doug: Broke .m into 2 files so that recompilation can
 *                   be on a finer grain.
 * 07 Sep 99 - Mike: Fixed createJcond() to handle fbranch insructions
 * 19 Nov 99 - Mike: Explicit semantics for call/restore and ret/restore
 * 23 Nov 99 - Mike: Separate semantics for call/restore and ret/restore to
 *                    prevent enclosing procedure from always having 6 params
 * 10 Dec 99 - Mike: std_ret now has explicit restore semantics, and ALL types
 *                    of procedures return integers in %o0 (never %i0)
 * 19 Apr 00 - Mike: Added isFunctionPrologue()
 * 31 May 00 - Mike: Support for the call/add caller prologue
 * 01 Sep 00 - Mike: Unknown branches are marked as invalid instructions now
 * 13 Oct 00 - Mike: Treat BVS as BN, BVC as BA
 * 09 Mar 01 - Mike: Trivial fix to debugging for call_add
 * 16 May 01 - Mike: SCDA -> SCDAN (cf new SCDAT case for HPPA)
 * 12 Oct 01 - Mike: Suppressed some "unused local var" messages
*/

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "global.h"
#include "proc.h"
#include "prog.h"
#include "decoder.h"
#include "ss.h"
#include "rtl.h"
#include "csr.h"
#include "sparc.pat.h"      // generated from `sparc.pat'
#include "sparc-names.h"    // generated by 'tools -fieldnames' - has
                            //   arrays of names of fields
#include "BinaryFile.h"		// For SymbolByAddress()

/*==============================================================================
 * FUNCTION:       unused
 * OVERVIEW:       A dummy function to suppress "unused local variable" messages
 * PARAMETERS:     x: integer variable to be "used"
 * RETURNS:        Nothing
 *============================================================================*/
void unused(int x)
{}

/*==============================================================================
 * FUNCTION:       createJcond
 * OVERVIEW:       Create an RTL for a Bx instruction
 * PARAMETERS:     pc - the location counter
 *                  RTs - ptr to list of RT pointers
 *                  name - instruction name (e.g. "BNE,a")
 * RETURNS:        Pointer to newly created RTL, or NULL if invalid
 *============================================================================*/
HLJcond* createJcond(ADDRESS pc, list<RT*>* RTs, const char* name)
{
    HLJcond* res = new HLJcond(pc, RTs);
    if (name[0] == 'F') {
        // fbranch is any of [ FBN FBNE FBLG FBUL FBL   FBUG FBG   FBU
        //                     FBA FBE  FBUE FBGE FBUGE FBLE FBULE FBO ],
        // fbranches are not the same as ibranches, so need a whole different
        // set of tests
        if (name[2] == 'U')
            name++;             // Just ignore unordered (for now)
        switch (name[2]) {
        case 'E':                           // FBE
            res->setCondType(HLJCOND_JE, true);
            break;
        case 'L':
            if (name[3] == 'G')             // FBLG
                res->setCondType(HLJCOND_JNE, true);
            else if (name[3] == 'E')        // FBLE
                res->setCondType(HLJCOND_JSLE, true);
            else                            // FBL
                res->setCondType(HLJCOND_JSL, true);
            break;
        case 'G':
            if (name[3] == 'E')             // FBGE
                res->setCondType(HLJCOND_JSGE, true);
            else                            // FBG
                res->setCondType(HLJCOND_JSG, true);
            break;
        case 'N':
            if (name[3] == 'E')             // FBNE
                res->setCondType(HLJCOND_JNE, true);
            // Else it's FBN!
            break;
        default:
            ostrstream ost;
            ost << "unknown float branch " << name;
            error(str(ost));
            delete res;
            res = NULL;
        }
        return res;
    }   

    // ibranch is any of [ BN BE  BLE BL  BLEU BCS BNEG BVS
    //                     BA BNE BG  BGE BGU  BCC BPOS BVC ],
    switch(name[1]) {
    case 'E':
        res->setCondType(HLJCOND_JE);           // BE
        break;
    case 'L':
        if (name[2] == 'E') {
            if (name[3] == 'U')
                res->setCondType(HLJCOND_JULE); // BLEU
            else
                res->setCondType(HLJCOND_JSLE); // BLE
        }
        else
            res->setCondType(HLJCOND_JSL);      // BL
        break;
    case 'N':
        // BNE, BNEG (won't see BN)
        if (name[3] == 'G')
            res->setCondType(HLJCOND_JMI);      // BNEG
        else
            res->setCondType(HLJCOND_JNE);      // BNE
        break;
    case 'C':
        // BCC, BCS
        if (name[2] == 'C')
            res->setCondType(HLJCOND_JUGE);     // BCC
        else
            res->setCondType(HLJCOND_JUL);      // BCS
        break;
    case 'V':
        // BVC, BVS; should never see these now
        if (name[2] == 'C')
            error("Decoded BVC instruction");   // BVC
        else
            error("Decoded BVS instruction");   // BVS
        break;
    case 'G':   
        // BGE, BG, BGU
        if (name[2] == 'E')
            res->setCondType(HLJCOND_JSGE);     // BGE
        else if (name[2] == 'U')
            res->setCondType(HLJCOND_JUG);      // BGU
        else
            res->setCondType(HLJCOND_JSG);      // BG
        break;
    case 'P':   
        res->setCondType(HLJCOND_JPOS);         // BG
        break;
    default:
        ostrstream ost;
        ost << "unknown non-float branch " << name;
        error(str(ost));
    }   
    return res;
}

/*==============================================================================
 * FUNCTION:       NJMCDecoder::decodeInstruction
 * OVERVIEW:       Attempt to decode the high level instruction at a given
 *                 address and return the corresponding HL type (e.g. HLCall,
 *                 HLJump etc). If no high level instruction exists at the
 *                 given address, then simply return the RTL for the low level
 *                 instruction at this address. There is an option to also
 *                 include the RTs for a HL instruction.
 * PARAMETERS:     pc - the native address of the pc
 *                 delta - the difference between the above address and the
 *                   host address of the pc (i.e. the address that the pc is at
 *                   in the loaded object file)
 *                 proc - the enclosing procedure. This can be NULL for
 *                   those of us who are using this method in an interpreter
 * RETURNS:        a DecodeResult structure containing all the information
 *                   gathered during decoding
 *============================================================================*/
DecodeResult& NJMCDecoder::decodeInstruction (ADDRESS pc, int delta,
    UserProc* proc /* = NULL */)
{ 
    static DecodeResult result;
    ADDRESS hostPC = pc+delta;

    // Clear the result structure;
    result.reset();

    // The actual list of instantiated RTs
    list<RT*>* RTs = NULL;

    ADDRESS nextPC;

    // Try matching a logue first.
    ADDRESS saveHostPC = hostPC;
    int addr, imm22, locals, rs1, rs2, imm, rd, rs1j, rdj, hiVal, loVal, reg;
    Logue* logue;
    if ((logue = InstructionPatterns::call_rst_ui_reg(csr, hostPC,
        addr, imm22, rs1, rs2, rd)) != NULL) {
        /*
         * Call/restore that happens to have an unimp after it.
         * Note: superset of struct return case, so must appear before it
         */
        // Create "semantics" for the restore. It's 6 assignments from I
        // registers to O registers, plus more with the implicit "add"
        RTs = instantiate(pc, "dly_rst_reg", DIS_RS1, DIS_RS2, DIS_RD);
        HLCall* newCall = new HLCall(pc, 0, RTs);
        result.rtl = newCall;
        result.type = SD;
        result.numBytes = hostPC - saveHostPC;

        // Record the prologue of this caller
        newCall->setPrologue(logue);

        // This call effectively is followed by a return
        newCall->setReturnAfterCall(true);

        // Set the fixed destination. Note that addr is in the host address
        // space, so we have to subtract delta
        newCall->setDest(addr - delta);
        unused(rd);
        SHOW_ASM("call_rst_ui_reg 0x" << hex << addr-delta)
    }
    else if ((logue = InstructionPatterns::call_rst_ui_imm(csr, hostPC,
        addr, imm22, rs1, imm, rd)) != NULL) {
        /*
         * Call/restore that happens to have an unimp after it.
         * Note: superset of struct return case, so must appear before it
         */
        // Create "semantics" for the restore. It's 6 assignments from I
        // registers to O registers, plus more with the implicit "add"
        RTs = instantiate(pc, "dly_rst_imm", DIS_RS1, dis_Num(imm), DIS_RD);
        HLCall* newCall = new HLCall(pc, 0, RTs);
        result.rtl = newCall;
        result.type = SD;
        result.numBytes = hostPC - saveHostPC;

        // Record the prologue of this caller
        newCall->setPrologue(logue);

        // This call effectively is followed by a return
        newCall->setReturnAfterCall(true);

        // Set the fixed destination. Note that addr is in the host address
        // space, so we have to subtract delta
        newCall->setDest(addr - delta);
        unused(rd);
        SHOW_ASM("call_rst_ui_reg 0x" << hex << addr-delta)
    }
    else if ((logue = InstructionPatterns::call_restore_reg(csr, hostPC,
        addr, rs1, rs2, rd)) != NULL) {
        /*
         * Call/restore with no unimp after it.
         */
        // Create "semantics" for the restore. It's 6 assignments from I
        // registers to O registers, plus more with the implicit "add"
        RTs = instantiate(pc, "dly_rst_reg", DIS_RS1, DIS_RS2, DIS_RD);
        HLCall* newCall = new HLCall(pc, 0, RTs);
        result.rtl = newCall;
        result.type = SD;
        result.numBytes = hostPC - saveHostPC;

        // Record the prologue of this caller
        newCall->setPrologue(logue);

        // This call effectively is followed by a return
        newCall->setReturnAfterCall(true);

        // Set the fixed destination. Note that addr is in the host address
        // space, so we have to subtract delta
        newCall->setDest(addr - delta);
        unused(rd);
        SHOW_ASM("call_restore_reg 0x" << hex << addr-delta)
    }
    else if ((logue = InstructionPatterns::call_restore_imm(csr, hostPC,
        addr, rs1, imm, rd)) != NULL) {
        /*
         * Call/restore with no unimp after it.
         */
        // Create "semantics" for the restore. It's 6 assignments from I
        // registers to O registers, plus more with the implicit "add"
        RTs = instantiate(pc, "dly_rst_imm", DIS_RS1, dis_Num(imm), DIS_RD);
        HLCall* newCall = new HLCall(pc, 0, RTs);
        result.rtl = newCall;
        result.type = SD;
        result.numBytes = hostPC - saveHostPC;

        // Record the prologue of this caller
        newCall->setPrologue(logue);

        // This call effectively is followed by a return
        newCall->setReturnAfterCall(true);

        // Set the fixed destination. Note that addr is in the host address
        // space, so we have to subtract delta
        newCall->setDest(addr - delta);
        SHOW_ASM("call_restore_imm 0x" << hex << addr-delta)
    }
    else if ((logue = InstructionPatterns::struct_call(csr,hostPC, addr,
        imm22)) != NULL) {
        /*
         * Ordinary call to fixed dest, with aggregate return
         * Note: superset of ordinary case, so must appear before std_call
         */
        HLCall* newCall = new HLCall(pc, imm22, RTs);
        result.rtl = newCall;
        result.type = SD;
        result.numBytes = hostPC - saveHostPC;

        // Record the prologue of this caller
        newCall->setPrologue(logue);

        // Set the fixed destination
        newCall->setDest(addr - delta);
        unused(rd);
        SHOW_ASM("struct_call 0x" << hex << addr-delta << ", " << imm22)
    }
    else if ((logue = InstructionPatterns::move_call_move(csr, hostPC, addr,
        rd)) != NULL) {
        /*
         * Something like:
         * mov %o7, %g1
         * call foo
         * mov %g1, %o7
         * This is the leaf procedure version of call/restore
         */
        HLCall* newCall = new HLCall(pc, 0, RTs);
        result.rtl = newCall;
        result.type = SD;
        result.numBytes = hostPC - saveHostPC;

        // Record the prologue of this caller
        newCall->setPrologue(logue);

        // This call effectively is followed by a return
        newCall->setReturnAfterCall(true);

        // Set the fixed destination. Note that addr is in the host address
        // space, so we have to subtract delta
        newCall->setDest(addr - delta);
        unused(rd);
        SHOW_ASM("move_call_move 0x" << hex << addr-delta)
    }
    else if ((logue = InstructionPatterns::move_x_call_move(csr, hostPC, addr,
        rd)) != NULL) {
        /*
         * Something like:
         * mov %o7, %g1
         * srl %g2, 11, %o1
         * call foo
         * mov %g1, %o7
         * This is the leaf procedure version of call/restore with an instr
         * inserted for scheduling purposes
         */
        // First decode the x instruction. Note that hostPC has been advanced
        // by 4*4 = 16 bytes
        RTs = decodeLowLevelInstruction(hostPC-16+4, pc+4, result);

        // Create a HLCall for the move/call/move part
        HLCall* newCall = new HLCall(pc, 0, RTs);
        result.rtl = newCall;
        result.type = SD;
        result.numBytes = hostPC - saveHostPC;

        // Record the prologue of this caller
        newCall->setPrologue(logue);

        // This call is effectively followed by a return
        newCall->setReturnAfterCall(true);

        // Set the fixed destination. Note that addr is in the host address
        // space, so we have to subtract delta
        newCall->setDest(addr - delta);
        unused(rd);
        SHOW_ASM("move_x_call_move 0x" << hex << addr-delta)
    }
    else if ((logue = InstructionPatterns::call_add(csr, hostPC, addr,
        imm)) != NULL) {
        /*
         * Example from /opt/local/gnu/bin/bash:
         * 3b990:  40 00 07 65        call         0x3d724
         * 3b994:  9e 03 e0 38        add          %o7, 56, %o7
         * This becomes a call / jump .+4+56
         */

        // Create a HLCall for the call part
        HLCall* newCall = new HLCall(pc, 0, RTs);
        result.rtl = newCall;
        result.type = SD;
        result.numBytes = hostPC - saveHostPC;

        // Record the prologue of this caller
        newCall->setPrologue(logue);

        // Set the fixed destination. Note that addr is in the host address
        // space, so we have to subtract delta
        newCall->setDest(addr - delta);

        // frontsparc.cc on seeing a prologue with this name will use the
        // forceOutEdge field of the result to force a goto to the appropriate
        // address, i.e. where it would have been (pc+8) plus imm
        result.forceOutEdge = pc + 8 + imm;

        SHOW_ASM("call_add 0x" << hex << addr-delta << " => 0x" << pc+8+imm)
    }
    else if ((logue = InstructionPatterns::std_call(csr,hostPC,addr)) != NULL) {
        /*
         * Ordinary call to fixed dest, no struct return
         * Or it might be a call to .stret4, which is special
         */
        HLCall* newCall = new HLCall(pc, 0, RTs);
        // Set the fixed destination. Note that addr is in the host address
        // space, so we have to subtract delta
        newCall->setDest(addr - delta);
        result.numBytes = hostPC - saveHostPC;
        // See if this call is to a special symbol
        const char* dest = prog.pBF->SymbolByAddress((ADDRESS)(addr - delta));
        if (dest && (strcmp(dest, ".stret4") == 0))
        {
            result.rtl = new HLReturn(pc,RTs);
            result.type = DD;
            // This is the end of the function
            newCall->setReturnAfterCall(true);
            // This is actually a callee epilogue
            // Record the epilogue of this callee
            if (proc != NULL) {
                delete logue;       // This is a caller prologue
                vector<int> params(0);
                logue = csr.instantiateLogue("ret_struct4", params);
                proc->setEpilogue((CalleeEpilogue*)logue);
            }
            SHOW_ASM("stret4")
        }
        else {
            result.rtl = newCall;
            result.type = SD;

            // Record the prologue of this caller
            newCall->setPrologue(logue);

            SHOW_ASM("std_call 0x" << hex << addr-delta)
        }
    }
    else if ((logue = InstructionPatterns::new_reg_win(csr,hostPC,
        locals)) != NULL)
    {
        /*
         * Save instruction with add to %sp
         */
        if (proc != NULL) {

            // Record the prologue of this callee
            assert(logue->getType() == Logue::CALLEE_PROLOGUE);
            proc->setPrologue((CalleePrologue*)logue);
        }

        result.numBytes = hostPC - saveHostPC;
        result.rtl = new RTL(pc,RTs);
        result.type = NCT;
        unused(locals);             // Only used by the macro, if debug on
        SHOW_ASM("new_reg_win " << dec << locals)
    }
    else if ((logue = InstructionPatterns::new_reg_win_large(csr,hostPC,
        hiVal, loVal, reg)) != NULL)
    {
        /*
         * Save instruction with large add to %sp
         */
        int locals = hiVal + loVal;
        if (proc != NULL) {

            // Record the prologue of this callee
            assert(logue->getType() == Logue::CALLEE_PROLOGUE);
            proc->setPrologue((CalleePrologue*)logue);
        }

        result.numBytes = hostPC - saveHostPC;
        result.rtl = new RTL(pc,RTs);
        result.type = NCT;
        unused(locals);             // Only used by the macro, if debug on
        SHOW_ASM("new_reg_win " << dec << locals)
    }
    else if ((logue = InstructionPatterns::same_reg_win(csr, hostPC,
        locals)) != NULL)
    {
        /*
         * Add instruction with add to %sp
         */
        if (proc != NULL) {

            // Record the prologue of this callee
            assert(logue->getType() == Logue::CALLEE_PROLOGUE);
            proc->setPrologue((CalleePrologue*)logue);
        }

        result.numBytes = hostPC - saveHostPC;
        result.rtl = new RTL(pc,RTs);
        result.type = NCT;
        unused(locals);             // Only used by the macro, if debug on
        SHOW_ASM("same_reg_win " << dec << locals)
    }
    else if ((logue = InstructionPatterns::same_reg_win_large(csr, hostPC,
        hiVal, loVal, reg)) != NULL)
    {
        /*
         * Add instruction with large add to %sp
         */
        int locals = hiVal + loVal;
        if (proc != NULL) {

            // Record the prologue of this callee
            assert(logue->getType() == Logue::CALLEE_PROLOGUE);
            proc->setPrologue((CalleePrologue*)logue);
        }

        result.numBytes = hostPC - saveHostPC;
        result.rtl = new RTL(pc,RTs);
        result.type = NCT;
        unused(locals);             // Only used by the macro, if debug on
        SHOW_ASM("same_reg_win " << dec << locals)
    }
    else if ((logue = InstructionPatterns::leaf_ret(csr, hostPC)) != NULL) {
        /*
         * jmpl %o7+12 (retl for aggregate return) OR
         * retl (leaf procedure return; no restore of %sp)
         */
        result.numBytes = hostPC - saveHostPC;
        result.rtl = new HLReturn(pc,RTs);
        result.type = DD;

        // Record the epilogue of this callee
        if (proc != NULL) {
            assert(logue->getType() == Logue::CALLEE_EPILOGUE);
            proc->setEpilogue((CalleeEpilogue*)logue);
        }
    
        SHOW_ASM("leaf_ret");
    }
    else if ((logue = InstructionPatterns::ret_reg_val(csr,
        hostPC,rs1,rs2)) != NULL)
    {
        /*
         * ret; restore rs1 rs2 %o0
         * Need RTs for this epilogue so that definition of %o0 is
         * preserved
         */
        RTs = instantiate(pc, "ret_reg_val",DIS_RS1,DIS_RS2);
        result.numBytes = hostPC - saveHostPC;
        result.rtl = new HLReturn(pc,RTs);
        result.type = DD;

        // Record the epilogue of this callee
        if (proc != NULL) {
            assert(logue->getType() == Logue::CALLEE_EPILOGUE);
            proc->setEpilogue((CalleeEpilogue*)logue);
        }

        SHOW_ASM("ret_reg_val");
    }
    else if ((logue = InstructionPatterns::ret_imm_val(csr,
        hostPC,rs1,imm)) != NULL) {
        /*
         * ret; restore rs1 imm %o0
         * Need RTs for this epilogue so that definition of %o0 is
         * preserved
         */
        RTs = instantiate(pc, "ret_imm_val",DIS_RS1,dis_Num(imm));
        result.numBytes = hostPC - saveHostPC;
        result.rtl = new HLReturn(pc,RTs);
        result.type = DD;

        // Record the epilogue of this callee
        if (proc != NULL) {
            assert(logue->getType() == Logue::CALLEE_EPILOGUE);
            proc->setEpilogue((CalleeEpilogue*)logue);
        }

        SHOW_ASM("ret_imm_val");
    }
    else if ((logue = InstructionPatterns::std_ret(csr, hostPC)) != NULL) {
        /*
         * ret; restore OR
         * jmpl %i7+12; restore (ordinary ret/restore for aggregate return)
         */
        RTs = instantiate(pc, "std_ret");
        result.numBytes = hostPC - saveHostPC;
        result.rtl = new HLReturn(pc,RTs);
        result.type = DD;

        // Record the epilogue of this callee
        if (proc != NULL) {
            assert(logue->getType() == Logue::CALLEE_EPILOGUE);
            proc->setEpilogue((CalleeEpilogue*)logue);
        }
    
        SHOW_ASM("std_ret");
    }
    else if ((logue = InstructionPatterns::jmp_restore_reg(csr,hostPC,
        rs1j, rdj, rs1, rs2, rd)) != NULL) {
        /*
         * Note: superset of ret/restore (std_ret), so put after that one!
         * Note also: it doesn't matter whether it's a jmp/restore or a
         * call/restore, because %o7 is wiped out with %i7 in the restore
         * after the jmpl may or may not have set %o7 to %pc. So we ignore
         * rdj.
         * Example:
         * jmp %o2
         * restore
         */
        // Create "semantics" for the restore.
        RTs = instantiate(pc, "dly_rst_reg", DIS_RS1, DIS_RS2, DIS_RD);
        HLCall* newCall = new HLCall(pc, 0, RTs);
        result.rtl = newCall;
        result.type = DD;
        result.numBytes = hostPC - saveHostPC;

        // Record the prologue of this caller
        newCall->setPrologue(logue);

        // This call effectively is followed by a return
        newCall->setReturnAfterCall(true);

        // Set the register destination.
        SemStr* dest = new SemStr;
        dest->push(idRegOf); dest->push(idIntConst);
        dest->push(rs1j);
        newCall->setDest(dest);

        // Set computed
        newCall->setIsComputed();

        unused(rd);
        SHOW_ASM("jmp_restore_reg" << dest);
    }
    else if ((logue = InstructionPatterns::jmp_restore_imm(csr,hostPC,
        rs1j, rdj, rs1, imm, rd)) != NULL) {
        /*
         * Note: superset of ret/restore (std_ret), so put after that one!
         * Example:
         * jmp %o2
         * restore %g0, 1, %o0
         */
        // Create "semantics" for the restore.
        RTs = instantiate(pc, "dly_rst_imm", DIS_RS1, dis_Num(imm), DIS_RD);
        HLCall* newCall = new HLCall(pc, 0, RTs);
        result.rtl = newCall;
        result.type = DD;
        result.numBytes = hostPC - saveHostPC;

        // Record the prologue of this caller
        newCall->setPrologue(logue);

        // This call effectively is followed by a return
        newCall->setReturnAfterCall(true);

        // Set the register destination.
        SemStr* dest = new SemStr;
        dest->push(idRegOf); dest->push(idIntConst);
        dest->push(rs1j);
        newCall->setDest(dest);

        // Set computed
        newCall->setIsComputed();

        unused(rd);
        SHOW_ASM("jmp_restore_imm" << dest);
    }
    else {
    
        match [nextPC] hostPC to
    
        | call_(addr) =>
            /*
             * A jmpl with rd == %o7, i.e. a register call
             */
            HLCall* newCall = new HLCall(pc, 0, RTs);
    
            // Record the fact that this is a computed call
            newCall->setIsComputed();
    
            // Set the destination expression
            newCall->setDest(dis_Eaddr(addr));
            result.rtl = newCall;
            result.type = DD;
    
    
        | ret() =>
            /*
             * Just a ret, no restore
             */
            result.rtl = new HLReturn(pc, RTs);
            result.type = DD;
            SHOW_ASM("ret_")
    
        | branch^",a" (tgt) [name] => 
            /*
             * Anulled branch
             */
    
            // First, check for CBxxx branches (branches that depend on
            // co-processor instructions). These are invalid, as far as
            // we are concerned
            if (name[0] == 'C') {
                result.valid = false;
                result.rtl = new RTL;
                result.numBytes = 4;
                return result;
            }
            // Instantiate a HLJump for the unconditional branches,
            // HLJconds for the rest.
            // NOTE: NJMC toolkit cannot handle embedded else statements!
            HLJump* jump = 0;
            if (strcmp(name,"BA,a") == 0 || strcmp(name,"BN,a") == 0)
                jump = new HLJump(pc, RTs);
            if ((jump == 0) &&
              (strcmp(name,"BVS,a") == 0 || strcmp(name,"BVC,a") == 0))
                jump = new HLJump(pc, RTs);
            if (jump == 0)
                jump = createJcond(pc, RTs, name);

            if (jump == NULL) {
                result.valid = false;
                result.rtl = new RTL;
                result.numBytes = 4;
                return result;
            }
    
            // The class of this instruction depends on whether or not
            // it is one of the 'unconditional' conditional branches
            // "BA,A" or "BN,A"
            result.type = SCDAN;
            if ((strcmp(name,"BA,a") == 0) || (strcmp(name, "BVC,a") == 0))
                result.type = SU;
            if ((strcmp(name,"BN,a") == 0) || (strcmp(name, "BVS,a") == 0))
                result.type = SKIP;
    
            result.rtl = jump;
            jump->setDest(tgt - delta);
            SHOW_ASM(name << " " << hex << tgt-delta)
            
        | branch (tgt) [name] => 
            /*
             * Non anulled branch
             */
            // First, check for CBxxx branches (branches that depend on
            // co-processor instructions). These are invalid, as far as
            // we are concerned
            if (name[0] == 'C') {
                result.valid = false;
                result.rtl = new RTL;
                result.numBytes = 4;
                return result;
            }
            // Instantiate a HLJump for the unconditional branches,
            // HLJconds for the rest
            // NOTE: NJMC toolkit cannot handle embedded else statements!
            HLJump* jump = 0;
            if (strcmp(name,"BA") == 0 || strcmp(name,"BN") == 0)
                jump = new HLJump(pc, RTs);
            if ((jump == 0) &&
              (strcmp(name,"BVS") == 0 || strcmp(name,"BVC") == 0))
                jump = new HLJump(pc, RTs);
            if (jump == 0)
                jump = createJcond(pc, RTs, name);
    
            // The class of this instruction depends on whether or not
            // it is one of the 'unconditional' conditional branches
            // "BA" or "BN" (or the pseudo unconditionals BVx)
            result.type = SCD;
            if ((strcmp(name,"BA") == 0) || (strcmp(name, "BVC") == 0))
                result.type = SD;
            if ((strcmp(name,"BN") == 0) || (strcmp(name, "BVS") == 0))
                result.type = NCT;
    
            result.rtl = jump;
            jump->setDest(tgt - delta);
            SHOW_ASM(name << " " << hex << tgt-delta)
    
    
        | JMPL (addr, rd) =>
            /*
             * JMPL, with rd != %o7, i.e. register jump
             */
            HLNwayJump* jump = new HLNwayJump(pc, RTs);
            // Record the fact that it is a computed jump
            jump->setIsComputed();
            result.rtl = jump;
            result.type = DD;
            jump->setDest(dis_Eaddr(addr));
            unused(rd);
            SHOW_ASM("JMPL ")
    #if DEBUG_DECODER
            jump->getDest()->print();
    #endif
    
        else
            result.rtl = new RTL(pc,
                decodeLowLevelInstruction(hostPC,pc,result));
            nextPC = hostPC + 4;        // 4 byte instruction
    
        endmatch
    
        result.numBytes = nextPC - hostPC;
    }

    return result;
}

/***********************************************************************
 * These are functions used to decode instruction operands into
 * expressions (SemStrs).
 **********************************************************************/

/*==============================================================================
 * FUNCTION:        NJMCDecoder::dis_RegImm
 * OVERVIEW:        Decode the register or immediate at the given
 *                  address.
 * PARAMETERS:      pc - an address in the instruction stream
 * RETURNS:         the register or immediate at the given address
 *============================================================================*/
SemStr *NJMCDecoder::dis_RegImm(unsigned pc)
{

    match pc to
    | imode(i) =>
        SemStr* expr = new SemStr();
        expr->push(idIntConst);
        expr->push(i);
        return expr;
    | rmode(rs2) =>
        SemStr* expr = DIS_RS2;
        expr->prep(idRegOf);
        return expr;
    endmatch
}

/*==============================================================================
 * FUNCTION:        NJMCDecoder::dis_Eaddr
 * OVERVIEW:        Converts a dynamic address to a SemStr expression.
 *                  E.g. %o7 --> regOf, int, 15
 * PARAMETERS:      pc - the instruction stream address of the dynamic
 *                    address
 *                  ignore - redundant parameter on SPARC
 * RETURNS:         the SemStr representation of the given address
 *============================================================================*/
SemStr* NJMCDecoder::dis_Eaddr(ADDRESS pc, int ignore = 0)
{
    SemStr* expr = new SemStr();

    match pc to
    | indirectA(rs1) =>
        delete expr;
        expr = DIS_RS1;
        expr->prep(idRegOf);
    | indexA(rs1, rs2) =>
        SemStr* rs1SS = DIS_RS1;
        SemStr* rs2SS = DIS_RS2;
        expr->push(idPlus);
        expr->push(idRegOf);
        expr->pushSS(rs1SS);
        expr->push(idRegOf);
        expr->pushSS(rs2SS);
        delete rs1SS;
        delete rs2SS;
    | absoluteA(i) =>
        expr->push(idIntConst);
        expr->push(i);
    | dispA(rs1,i) =>
        SemStr* rs1SS = DIS_RS1;
        expr->push((int)i < 0 ? idMinus : idPlus);
        expr->push(idRegOf);
        expr->pushSS(rs1SS);
        expr->push(idIntConst);
        expr->push(abs(i));
        delete rs1SS;
    endmatch

    return expr;
}

/*==============================================================================
 * FUNCTION:      isFuncPrologue()
 * OVERVIEW:      Check to see if the instructions at the given offset match
 *                  any callee prologue, i.e. does it look like this offset
 *                  is a pointer to a function?
 * PARAMETERS:    hostPC - pointer to the code in question (native address)
 * RETURNS:       True if a match found
 *============================================================================*/
bool isFuncPrologue(ADDRESS hostPC)
{
    int hiVal, loVal, reg, locals;
    if ((InstructionPatterns::new_reg_win(prog.csrSrc,hostPC, locals)) != NULL)
            return true;
    if ((InstructionPatterns::new_reg_win_large(prog.csrSrc, hostPC,
        hiVal, loVal, reg)) != NULL)
            return true;
    if ((InstructionPatterns::same_reg_win(prog.csrSrc, hostPC, locals))
        != NULL)
            return true;
    if ((InstructionPatterns::same_reg_win_large(prog.csrSrc, hostPC,
        hiVal, loVal, reg)) != NULL)
            return true;

    return false;
}

/**********************************
 * These are the fetch routines.
 **********************************/

/*==============================================================================
 * FUNCTION:        getDword
 * OVERVIEW:        Returns the double starting at the given address.
 * PARAMETERS:      lc - address at which to decode the double
 * RETURNS:         the decoded double
 *============================================================================*/
DWord getDword(ADDRESS lc)
{
  Byte* p = (Byte*)lc;
  return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
}


