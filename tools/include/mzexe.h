/*! mzexe.h - DOS MZ executable header
 *
 *  Shared by the NE and LX readers. Defines struct exe_hdr and the
 *  field accessors used by both formats.
 */

#ifndef __MZEXE__
#define __MZEXE__

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @file mzexe.h
 *  @brief DOS MZ executable header.
 *
 *  The MZ header is the first 64 bytes of every DOS, NE and LX
 *  executable. It is not specific to NE or LX; both formats begin
 *  with it and use the e_lfanew field to point to their own header.
 *
 *  References:
 *    - Microsoft MS-DOS Programmer's Reference, "The .EXE file
 *      header".
 *    - IBM OS/2 Toolkit, "New Executable File Format".
 *    - IBM OS/2 Toolkit, "Linear Executable File Format".
 */

/* ==================================================================
 * MZ header constants
 * ================================================================== */

/**
 * @def EMAGIC
 * @brief MZ signature ("MZ"). Value: 0x5A4D.
 */
#define EMAGIC      0x5A4D

/**
 * @def ENEWEXE
 * @brief Size of the MZ header.
 */
#define ENEWEXE     sizeof(struct exe_hdr)

/**
 * @def ENEWHDR
 * @brief Offset of e_lfanew. Value: 0x003C.
 */
#define ENEWHDR     0x003C

/**
 * @def ERESWDS
 * @brief Reserved words. Value: 0x0010.
 */
#define ERESWDS     0x0010

/**
 * @def ERES1WDS
 * @brief Count of e_res words. Value: 0x0004.
 */
#define ERES1WDS    0x0004

/**
 * @def ERES2WDS
 * @brief Count of e_res2 words. Value: 0x000A.
 */
#define ERES2WDS    0x000A

/**
 * @def ECP
 * @brief e_cp offset. Value: 0x0004.
 */
#define ECP         0x0004

/**
 * @def ECBLP
 * @brief e_cblp offset. Value: 0x0002.
 */
#define ECBLP       0x0002

/**
 * @def EMINALLOC
 * @brief e_minalloc offset. Value: 0x000A.
 */
#define EMINALLOC   0x000A

/* ==================================================================
 * MZ header field accessors
 * ================================================================== */

/**
 * @def E_MAGIC(x)
 * @brief Accessor for the magic ("MZ") field.
 * @param x MZ header structure.
 */
#define E_MAGIC(x)      (x).e_magic

/**
 * @def E_CBLP(x)
 * @brief Accessor for the bytes in last block field.
 * @param x MZ header structure.
 */
#define E_CBLP(x)       (x).e_cblp

/**
 * @def E_CP(x)
 * @brief Accessor for the blocks in file field.
 * @param x MZ header structure.
 */
#define E_CP(x)         (x).e_cp

/**
 * @def E_CRLC(x)
 * @brief Accessor for the relocation entries field.
 * @param x MZ header structure.
 */
#define E_CRLC(x)       (x).e_crlc

/**
 * @def E_CPARHDR(x)
 * @brief Accessor for the header size in paragraphs field.
 * @param x MZ header structure.
 */
#define E_CPARHDR(x)    (x).e_cparhdr

/**
 * @def E_MINALLOC(x)
 * @brief Accessor for the minimum extra paragraphs field.
 * @param x MZ header structure.
 */
#define E_MINALLOC(x)   (x).e_minalloc

/**
 * @def E_MAXALLOC(x)
 * @brief Accessor for the maximum extra paragraphs field.
 * @param x MZ header structure.
 */
#define E_MAXALLOC(x)   (x).e_maxalloc

/**
 * @def E_SS(x)
 * @brief Accessor for the initial SS field.
 * @param x MZ header structure.
 */
#define E_SS(x)         (x).e_ss

/**
 * @def E_SP(x)
 * @brief Accessor for the initial SP field.
 * @param x MZ header structure.
 */
#define E_SP(x)         (x).e_sp

/**
 * @def E_CSUM(x)
 * @brief Accessor for the checksum field.
 * @param x MZ header structure.
 */
#define E_CSUM(x)       (x).e_csum

/**
 * @def E_IP(x)
 * @brief Accessor for the initial IP field.
 * @param x MZ header structure.
 */
#define E_IP(x)         (x).e_ip

/**
 * @def E_CS(x)
 * @brief Accessor for the initial CS field.
 * @param x MZ header structure.
 */
#define E_CS(x)         (x).e_cs

/**
 * @def E_LFARLC(x)
 * @brief Accessor for the relocation table offset field.
 * @param x MZ header structure.
 */
#define E_LFARLC(x)     (x).e_lfarlc

/**
 * @def E_OVNO(x)
 * @brief Accessor for the overlay number field.
 * @param x MZ header structure.
 */
#define E_OVNO(x)       (x).e_ovno

/**
 * @def E_RES(x)
 * @brief Accessor for the reserved field.
 * @param x MZ header structure.
 */
#define E_RES(x)        (x).e_res

/**
 * @def E_OEMID(x)
 * @brief Accessor for the OEM identifier field.
 * @param x MZ header structure.
 */
#define E_OEMID(x)      (x).e_oemid

/**
 * @def E_OEMINFO(x)
 * @brief Accessor for the OEM info field.
 * @param x MZ header structure.
 */
#define E_OEMINFO(x)    (x).e_oeminfo

/**
 * @def E_RES2(x)
 * @brief Accessor for the reserved field.
 * @param x MZ header structure.
 */
#define E_RES2(x)       (x).e_res2

/**
 * @def E_LFANEW(x)
 * @brief Accessor for the file offset of NE header field.
 * @param x MZ header structure.
 */
#define E_LFANEW(x)     (x).e_lfanew

#pragma pack(push,1)

/*! @brief DOS MZ executable header.
 *
 *  The first 64 bytes of every DOS and NE executable. The field
 *  comments describe the meaning of each WORD or DWORD in the
 *  original DOS format.
 */
struct exe_hdr {
    WORD    e_magic;        /*!< 0x4D, 0x5A. Magic number of an EXE
                             *   file: first byte 0x4D, second 0x5A. */
    WORD    e_cblp;         /*!< Bytes in the last block of the
                             *   program that are actually used. Zero
                             *   means the entire last block is used
                             *   (effective value 512). */
    WORD    e_cp;           /*!< Number of 512-byte blocks in the
                             *   file that are part of the EXE. If
                             *   e_cblp is non-zero, only that much of
                             *   the last block is used. */
    WORD    e_crlc;         /*!< Number of relocation entries stored
                             *   after the header. May be zero. */
    WORD    e_cparhdr;      /*!< Number of paragraphs in the header.
                             *   The program's data begins just after
                             *   the header; this field can be used to
                             *   calculate the appropriate file offset.
                             *   The header includes the relocation
                             *   entries. Some systems may fail if the
                             *   header is not a multiple of 512
                             *   bytes. */
    WORD    e_minalloc;     /*!< Number of paragraphs of additional
                             *   memory that the program will need.
                             *   Equivalent to the BSS size in a Unix
                             *   program. The program cannot be loaded
                             *   if there is not at least this much
                             *   memory available. */
    WORD    e_maxalloc;     /*!< Maximum number of paragraphs of
                             *   additional memory. Normally the OS
                             *   reserves all remaining conventional
                             *   memory for the program; this field
                             *   limits it. */
    WORD    e_ss;           /*!< Relative value of the stack segment.
                             *   Added to the segment the program was
                             *   loaded at; result initializes SS. */
    WORD    e_sp;           /*!< Initial value of the SP register. */
    WORD    e_csum;         /*!< Word checksum. If set properly, the
                             *   16-bit sum of all words in the file
                             *   should be zero. Usually not filled
                             *   in. */
    WORD    e_ip;           /*!< Initial value of the IP register. */
    WORD    e_cs;           /*!< Initial value of the CS register,
                             *   relative to the segment the program
                             *   was loaded at. */
    WORD    e_lfarlc;       /*!< Offset of the first relocation item
                             *   in the file. */
    WORD    e_ovno;         /*!< Overlay number. Normally zero,
                             *   meaning the main program. */
    WORD    e_res[ERES1WDS];/*!< Reserved. */
    WORD    e_oemid;        /*!< OEM identifier. */
    WORD    e_oeminfo;      /*!< OEM info. */
    WORD    e_res2[ERES2WDS];/*!< Reserved. */
    DWORD   e_lfanew;       /*!< File offset of the NE header. */
};

#pragma pack(pop)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __MZEXE__ */
