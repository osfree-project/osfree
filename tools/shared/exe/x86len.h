/*! x86len.h - minimal x86 instruction length decoder
 *
 *  Decodes the length of a single x86 instruction and classifies
 *  the small set of instruction families that the calling-convention
 *  heuristic needs. No general-purpose disassembly is provided.
 */

#ifndef __X86LEN__
#define __X86LEN__

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @file x86len.h
 *  @brief Minimal x86 instruction length decoder.
 *
 *  Intended for use by the calling-convention heuristic in the NE
 *  and LX readers: the caller walks a function body instruction by
 *  instruction and stops at the first ret-family opcode.
 *
 *  References:
 *    - Intel 64 and IA-32 Architectures Software Developer's
 *      Manual, Volume 2 (Instruction Set Reference).
 */

/* ==================================================================
 * Instruction families
 * ================================================================== */

/**
 * @def X86_FAM_OTHER
 * @brief Instruction is not one of the classified families.
 *        Value: 0.
 */
#define X86_FAM_OTHER     0

/**
 * @def X86_FAM_RET
 * @brief Near return (C3). Caller cleans up the stack.
 *        Value: 1.
 */
#define X86_FAM_RET       1

/**
 * @def X86_FAM_RETF
 * @brief Far return (CB). Caller cleans up the stack.
 *        Value: 2.
 */
#define X86_FAM_RETF      2

/**
 * @def X86_FAM_RET_IMM
 * @brief Near return with imm16 (C2 iw). Callee cleans up.
 *        Value: 3.
 */
#define X86_FAM_RET_IMM   3

/**
 * @def X86_FAM_RETF_IMM
 * @brief Far return with imm16 (CA iw). Callee cleans up.
 *        Value: 4.
 */
#define X86_FAM_RETF_IMM  4

/**
 * @def X86_FAM_JMP
 * @brief Unconditional jump (EB, E9, EA). Value: 5.
 */
#define X86_FAM_JMP       5

/**
 * @def X86_FAM_JCC
 * @brief Conditional jump (70-7F, 0F 80-8F). Value: 6.
 */
#define X86_FAM_JCC       6

/**
 * @def X86_FAM_CALL
 * @brief Near or far call (E8, 9A). Value: 7.
 */
#define X86_FAM_CALL      7

/* ==================================================================
 * Decoded instruction
 * ================================================================== */

/*! @brief Result of a single X86Decode call. */
typedef struct _X86_INSN {
    ULONG  ulLength;  /*!< Instruction length in bytes. */
    ULONG  ulFamily;  /*!< One of X86_FAM_*. */
    USHORT usImm;     /*!< Immediate for RET_IMM / RETF_IMM. */
} X86_INSN, *PX86_INSN;

/* ==================================================================
 * Decoder
 * ================================================================== */

/*! @brief Decode one x86 instruction.
 *
 *  Handles all legacy prefixes (F0, F2, F3, segment overrides,
 *  66, 67), the two-byte 0F opcode space, and the standard one-byte
 *  opcode map for 16-bit and 32-bit modes. Three-byte opcodes
 *  (0F 38 xx, 0F 3A xx) are not supported and cause the function to
 *  report an error, which the caller treats as "give up".
 *
 *  @param[in]  puchCode  Pointer to the code bytes. Not NULL.
 *  @param[in]  ulMax     Number of bytes available at @p puchCode.
 *                        Must be greater than zero.
 *  @param[in]  f32Bit    TRUE for 32-bit mode, FALSE for 16-bit
 *                        mode.
 *  @param[out] pInsn     Receiver for the decoded instruction. Not
 *                        NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p puchCode or @p pInsn is NULL,
 *                                   or @p ulMax is zero.
 *  @retval ERROR_INVALID_DATA       Unknown opcode, truncated
 *                                   instruction, or unsupported
 *                                   instruction encoding.
 */
APIRET APIENTRY X86Decode(const UCHAR *puchCode, ULONG ulMax,
                          BOOL f32Bit, PX86_INSN pInsn);

#ifdef __cplusplus
}
#endif

#endif /* __X86LEN__ */
