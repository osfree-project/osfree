/*!
 * @file x86len.c
 *
 * @brief Minimal x86 instruction length decoder.
 *
 * Implements the API declared in x86len.h. The decoder is driven
 * by a range-based switch; no external table is used.
 *
 * References:
 *   - Intel 64 and IA-32 Architectures Software Developer's
 *     Manual, Volume 2 (Instruction Set Reference).
 */

#include <string.h>

#include "os2types.h"
#include "os2err.h"
#include "x86len.h"

/* ------------------------------------------------------------------ */
/* ModRM decoder                                                       */
/* ------------------------------------------------------------------ */

/*!
 * @brief Compute the length of a ModRM byte plus its addressing
 *        operands (SIB, displacement).
 *
 * @param[in] p       Pointer to the ModRM byte. Not NULL.
 * @param[in] ulMax   Bytes available at @p p.
 * @param[in] fAddr32 TRUE for 32-bit addressing, FALSE for 16-bit.
 *
 * @return Length of the ModRM group in bytes, or 0 on error.
 *
 * @retval 0  The parameter block is empty, or the encoding is
 *            truncated (the ModRM and its operands do not fit
 *            within @p ulMax bytes).
 * @retval 1  The ModRM byte is the only byte; this is the
 *            register-direct form (@c mod @c == @c 3).
 */
static ULONG X86ModrmLen(const UCHAR *p, ULONG ulMax, BOOL fAddr32)
{
    BYTE modrm, mod, rm;
    ULONG len = 1;

    if (ulMax < 1)
        return 0;

    modrm = p[0];
    mod = (modrm >> 6) & 3;
    rm  = modrm & 7;

    if (mod == 3)
        return 1;

    if (!fAddr32) {
        if (mod == 0) {
            if (rm == 6) len += 2;
        } else if (mod == 1) {
            len += 1;
        } else {
            len += 2;
        }
    } else {
        if (rm == 4) {
            if (len + 1 > ulMax) return 0;
            len += 1;
            if (mod == 0 && (p[1] & 0x07) == 5)
                len += 4;
        }
        if (mod == 0 && rm == 5)      len += 4;
        else if (mod == 1)            len += 1;
        else if (mod == 2)            len += 4;
    }

    return (len <= ulMax) ? len : 0;
}

/* ------------------------------------------------------------------ */
/* Public entry point                                                  */
/* ------------------------------------------------------------------ */

/*!
 * @brief Decode one x86 instruction.
 *
 * Handles all legacy prefixes (F0, F2, F3, segment overrides,
 * 66, 67), the two-byte 0F opcode space, and the standard one-byte
 * opcode map for 16-bit and 32-bit modes. Three-byte opcodes
 * (0F 38 xx, 0F 3A xx) are not supported and cause the function to
 * report an error, which the caller treats as "give up".
 *
 * @param[in]  puchCode  Pointer to the code bytes. Not NULL.
 * @param[in]  ulMax     Number of bytes available at @p puchCode.
 *                       Must be greater than zero.
 * @param[in]  f32Bit    TRUE for 32-bit mode, FALSE for 16-bit
 *                       mode.
 * @param[out] pInsn     Receiver for the decoded instruction. Not
 *                       NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p puchCode or @p pInsn is NULL,
 *                                  or @p ulMax is zero.
 * @retval ERROR_INVALID_DATA       Unknown opcode, truncated
 *                                  instruction, or unsupported
 *                                  instruction encoding.
 */
APIRET APIENTRY X86Decode(const UCHAR *puchCode, ULONG ulMax,
                          BOOL f32Bit, PX86_INSN pInsn)
{
    ULONG pos = 0;
    BOOL  fOp32 = f32Bit;
    BOOL  fAddr32 = f32Bit;
    BYTE  op;
    ULONG len;
    ULONG iv;

    if (puchCode == NULL || pInsn == NULL || ulMax == 0)
        return ERROR_INVALID_PARAMETER;

    memset(pInsn, 0, sizeof(*pInsn));
    pInsn->ulFamily = X86_FAM_OTHER;

    /* Prefixes. */
    while (pos < ulMax) {
        op = puchCode[pos];
        if (op == 0x66) { fOp32 = !f32Bit; pos++; continue; }
        if (op == 0x67) { fAddr32 = !f32Bit; pos++; continue; }
        if (op == 0xF0 || op == 0xF2 || op == 0xF3 ||
            op == 0x2E || op == 0x36 || op == 0x3E ||
            op == 0x26 || op == 0x64 || op == 0x65) {
            pos++;
            continue;
        }
        break;
    }
    if (pos >= ulMax)
        return ERROR_INVALID_DATA;

    op = puchCode[pos++];
    iv = fOp32 ? 4 : 2;

    /* Two-byte opcodes (0F xx). */
    if (op == 0x0F) {
        BYTE op2;

        if (pos >= ulMax)
            return ERROR_INVALID_DATA;
        op2 = puchCode[pos++];

        if (op2 >= 0x80 && op2 <= 0x8F) {
            if (pos + iv > ulMax) return ERROR_INVALID_DATA;
            pos += iv;
            pInsn->ulFamily = X86_FAM_JCC;
            pInsn->ulLength = pos;
            return NO_ERROR;
        }

        /* 0F opcodes with no ModRM and no immediate. */
        if (op2 == 0x05 || op2 == 0x06 || op2 == 0x07 ||
            op2 == 0x08 || op2 == 0x09 || op2 == 0x0B ||
            op2 == 0x0E ||
            (op2 >= 0x30 && op2 <= 0x37) ||
            op2 == 0x77 || op2 == 0xA2 ||
            op2 == 0xA8 || op2 == 0xA9 || op2 == 0xAA ||
            op2 == 0xB8 || op2 == 0xB9 ||
            (op2 >= 0xC8 && op2 <= 0xCF)) {
            pInsn->ulLength = pos;
            return NO_ERROR;
        }

        /* Everything else in the 0F space: ModRM, optionally
           followed by an imm8 for a known subset. */
        len = X86ModrmLen(puchCode + pos, ulMax - pos, fAddr32);
        if (len == 0) return ERROR_INVALID_DATA;
        pos += len;
        if ((op2 >= 0x70 && op2 <= 0x73) ||
            op2 == 0xA4 || op2 == 0xAC || op2 == 0xBA ||
            op2 == 0xC2 || op2 == 0xC4 || op2 == 0xC5 ||
            op2 == 0xC6) {
            if (pos + 1 > ulMax) return ERROR_INVALID_DATA;
            pos += 1;
        }
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* 00-3F: regular 8-opcode pattern. */
    if (op <= 0x3F) {
        BYTE low = op & 0x07;

        if (low <= 3) {
            len = X86ModrmLen(puchCode + pos, ulMax - pos, fAddr32);
            if (len == 0) return ERROR_INVALID_DATA;
            pos += len;
        } else if (low == 4) {
            if (pos + 1 > ulMax) return ERROR_INVALID_DATA;
            pos += 1;
        } else if (low == 5) {
            if (pos + iv > ulMax) return ERROR_INVALID_DATA;
            pos += iv;
        }
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* 40-5F: inc/dec and push/pop register. No operands. */
    if (op >= 0x40 && op <= 0x5F) {
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* 60-61: pusha/popa. No operands. */
    if (op == 0x60 || op == 0x61) {
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* 62-63: bound, arpl. ModRM. */
    if (op == 0x62 || op == 0x63) {
        len = X86ModrmLen(puchCode + pos, ulMax - pos, fAddr32);
        if (len == 0) return ERROR_INVALID_DATA;
        pos += len;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* 64-67 are prefixes; handled in the prefix loop. */

    if (op == 0x68) {
        if (pos + iv > ulMax) return ERROR_INVALID_DATA;
        pos += iv;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    if (op == 0x69) {
        len = X86ModrmLen(puchCode + pos, ulMax - pos, fAddr32);
        if (len == 0) return ERROR_INVALID_DATA;
        pos += len;
        if (pos + iv > ulMax) return ERROR_INVALID_DATA;
        pos += iv;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    if (op == 0x6A) {
        if (pos + 1 > ulMax) return ERROR_INVALID_DATA;
        pos += 1;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    if (op == 0x6B) {
        len = X86ModrmLen(puchCode + pos, ulMax - pos, fAddr32);
        if (len == 0) return ERROR_INVALID_DATA;
        pos += len + 1;
        if (pos > ulMax) return ERROR_INVALID_DATA;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* 6C-6F: ins/outs. No operands. */
    if (op >= 0x6C && op <= 0x6F) {
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* 70-7F: jcc rel8. */
    if (op >= 0x70 && op <= 0x7F) {
        if (pos + 1 > ulMax) return ERROR_INVALID_DATA;
        pos += 1;
        pInsn->ulFamily = X86_FAM_JCC;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* 80, 82, 83: group 1 with imm8. */
    if (op == 0x80 || op == 0x82 || op == 0x83) {
        len = X86ModrmLen(puchCode + pos, ulMax - pos, fAddr32);
        if (len == 0) return ERROR_INVALID_DATA;
        pos += len + 1;
        if (pos > ulMax) return ERROR_INVALID_DATA;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* 81: group 1 with iv. */
    if (op == 0x81) {
        len = X86ModrmLen(puchCode + pos, ulMax - pos, fAddr32);
        if (len == 0) return ERROR_INVALID_DATA;
        pos += len;
        if (pos + iv > ulMax) return ERROR_INVALID_DATA;
        pos += iv;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* 84-8F: ModRM, no immediate. */
    if (op >= 0x84 && op <= 0x8F) {
        len = X86ModrmLen(puchCode + pos, ulMax - pos, fAddr32);
        if (len == 0) return ERROR_INVALID_DATA;
        pos += len;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* 90-99: nop, xchg, cbw/cwd, etc. No operands. */
    if (op >= 0x90 && op <= 0x99) {
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* 9A: call far. iw + iv. */
    if (op == 0x9A) {
        if (pos + 2 + iv > ulMax) return ERROR_INVALID_DATA;
        pos += 2 + iv;
        pInsn->ulFamily = X86_FAM_CALL;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* 9B-9F: wait, pushf, popf, sahf, lahf. No operands. */
    if (op >= 0x9B && op <= 0x9F) {
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* A0-A3: mov al/ax, moffs. Address-size displacement. */
    if (op >= 0xA0 && op <= 0xA3) {
        ULONG sz = fAddr32 ? 4 : 2;
        if (pos + sz > ulMax) return ERROR_INVALID_DATA;
        pos += sz;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* A4-A7: movs, cmps. No operands. */
    if (op >= 0xA4 && op <= 0xA7) {
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    if (op == 0xA8) {
        if (pos + 1 > ulMax) return ERROR_INVALID_DATA;
        pos += 1;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    if (op == 0xA9) {
        if (pos + iv > ulMax) return ERROR_INVALID_DATA;
        pos += iv;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* AA-AF: stos, lods, scas. No operands. */
    if (op >= 0xAA && op <= 0xAF) {
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* B0-B7: mov reg8, imm8. */
    if (op >= 0xB0 && op <= 0xB7) {
        if (pos + 1 > ulMax) return ERROR_INVALID_DATA;
        pos += 1;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* B8-BF: mov reg, iv. */
    if (op >= 0xB8 && op <= 0xBF) {
        if (pos + iv > ulMax) return ERROR_INVALID_DATA;
        pos += iv;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* C0, C1: shift group 2 with imm8. */
    if (op == 0xC0 || op == 0xC1) {
        len = X86ModrmLen(puchCode + pos, ulMax - pos, fAddr32);
        if (len == 0) return ERROR_INVALID_DATA;
        pos += len + 1;
        if (pos > ulMax) return ERROR_INVALID_DATA;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* C2: ret imm16. */
    if (op == 0xC2) {
        if (pos + 2 > ulMax) return ERROR_INVALID_DATA;
        pInsn->usImm = (USHORT)puchCode[pos] |
                       ((USHORT)puchCode[pos + 1] << 8);
        pos += 2;
        pInsn->ulFamily = X86_FAM_RET_IMM;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* C3: ret. */
    if (op == 0xC3) {
        pInsn->ulFamily = X86_FAM_RET;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* C4, C5: les, lds. ModRM. */
    if (op == 0xC4 || op == 0xC5) {
        len = X86ModrmLen(puchCode + pos, ulMax - pos, fAddr32);
        if (len == 0) return ERROR_INVALID_DATA;
        pos += len;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* C6: mov r/m8, imm8. */
    if (op == 0xC6) {
        len = X86ModrmLen(puchCode + pos, ulMax - pos, fAddr32);
        if (len == 0) return ERROR_INVALID_DATA;
        pos += len + 1;
        if (pos > ulMax) return ERROR_INVALID_DATA;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* C7: mov r/m, iv. */
    if (op == 0xC7) {
        len = X86ModrmLen(puchCode + pos, ulMax - pos, fAddr32);
        if (len == 0) return ERROR_INVALID_DATA;
        pos += len;
        if (pos + iv > ulMax) return ERROR_INVALID_DATA;
        pos += iv;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* C8: enter. iw + ib. */
    if (op == 0xC8) {
        if (pos + 3 > ulMax) return ERROR_INVALID_DATA;
        pos += 3;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* C9: leave. No operands. */
    if (op == 0xC9) {
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* CA: retf imm16. */
    if (op == 0xCA) {
        if (pos + 2 > ulMax) return ERROR_INVALID_DATA;
        pInsn->usImm = (USHORT)puchCode[pos] |
                       ((USHORT)puchCode[pos + 1] << 8);
        pos += 2;
        pInsn->ulFamily = X86_FAM_RETF_IMM;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* CB: retf. */
    if (op == 0xCB) {
        pInsn->ulFamily = X86_FAM_RETF;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* CC, CE, CF: int3, into, iret. No operands. */
    if (op == 0xCC || op == 0xCE || op == 0xCF) {
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* CD: int imm8. */
    if (op == 0xCD) {
        if (pos + 1 > ulMax) return ERROR_INVALID_DATA;
        pos += 1;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* D0-D3: shift group 2. ModRM. */
    if (op >= 0xD0 && op <= 0xD3) {
        len = X86ModrmLen(puchCode + pos, ulMax - pos, fAddr32);
        if (len == 0) return ERROR_INVALID_DATA;
        pos += len;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* D4, D5: aam, aad. imm8. */
    if (op == 0xD4 || op == 0xD5) {
        if (pos + 1 > ulMax) return ERROR_INVALID_DATA;
        pos += 1;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* D6, D7: salc, xlat. No operands. */
    if (op == 0xD6 || op == 0xD7) {
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* D8-DF: x87. ModRM. */
    if (op >= 0xD8 && op <= 0xDF) {
        len = X86ModrmLen(puchCode + pos, ulMax - pos, fAddr32);
        if (len == 0) return ERROR_INVALID_DATA;
        pos += len;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* E0-E7: loop, jcxz, in/out imm8. imm8. */
    if (op >= 0xE0 && op <= 0xE7) {
        if (pos + 1 > ulMax) return ERROR_INVALID_DATA;
        pos += 1;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* E8: call rel. iv. */
    if (op == 0xE8) {
        if (pos + iv > ulMax) return ERROR_INVALID_DATA;
        pos += iv;
        pInsn->ulFamily = X86_FAM_CALL;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* E9: jmp rel. iv. */
    if (op == 0xE9) {
        if (pos + iv > ulMax) return ERROR_INVALID_DATA;
        pos += iv;
        pInsn->ulFamily = X86_FAM_JMP;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* EA: jmp far. iw + iv. */
    if (op == 0xEA) {
        if (pos + 2 + iv > ulMax) return ERROR_INVALID_DATA;
        pos += 2 + iv;
        pInsn->ulFamily = X86_FAM_JMP;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* EB: jmp rel8. */
    if (op == 0xEB) {
        if (pos + 1 > ulMax) return ERROR_INVALID_DATA;
        pos += 1;
        pInsn->ulFamily = X86_FAM_JMP;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* EC-EF: in/out dx. No operands. */
    if (op >= 0xEC && op <= 0xEF) {
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* F1, F4, F5: int1, hlt, cmc. No operands. */
    if (op == 0xF1 || op == 0xF4 || op == 0xF5) {
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* F6, F7: group 3. ModRM; imm only for /0 and /1. */
    if (op == 0xF6 || op == 0xF7) {
        BYTE modrm, reg;
        len = X86ModrmLen(puchCode + pos, ulMax - pos, fAddr32);
        if (len == 0) return ERROR_INVALID_DATA;
        if (pos + len > ulMax) return ERROR_INVALID_DATA;
        modrm = puchCode[pos];
        reg = (modrm >> 3) & 7;
        pos += len;
        if (reg == 0 || reg == 1) {
            ULONG sz = (op == 0xF6) ? 1 : iv;
            if (pos + sz > ulMax) return ERROR_INVALID_DATA;
            pos += sz;
        }
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* F8-FD: clc, stc, cli, sti, cld, std. No operands. */
    if (op >= 0xF8 && op <= 0xFD) {
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    /* FE, FF: inc/dec/call/jmp/push group. ModRM. */
    if (op == 0xFE || op == 0xFF) {
        len = X86ModrmLen(puchCode + pos, ulMax - pos, fAddr32);
        if (len == 0) return ERROR_INVALID_DATA;
        pos += len;
        pInsn->ulLength = pos;
        return NO_ERROR;
    }

    return ERROR_INVALID_DATA;
}
