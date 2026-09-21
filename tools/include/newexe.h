/*! newexe.h - New Executable (NE) format
 *
 *  Provides the on-disk NE header structures, accessors, and the
 *  public API of the NE reader/binder library.
 */

#ifndef __NEWEXE__
#define __NEWEXE__

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @file newexe.h
 *  @brief New Executable (NE) format structures and access API.
 *
 *  Provides handle-based read access to NE executables plus a bind
 *  operation that merges a DOS stub with an NE image into a single
 *  FamilyAPI executable. All handles (HNE) are opaque; the internal
 *  representation is private to newexe.c.
 *
 *  References:
 *    - Microsoft MS-DOS Programmer's Reference, "New Executable
 *      Format".
 *    - IBM OS/2 Toolkit, "New Executable File Format".
 *    - Matt Pietrek, "Windows Internals: The Implementation of the
 *      Windows Operating Environment", Addison-Wesley, 1993,
 *      ISBN 0-201-62217-3, p. 219 (in-disk and in-memory module
 *      structure).
 */

/*! @name MZ header constants */
/*! @{ */
#define EMAGIC      0x5A4D                /*!< MZ signature ("MZ"). */
#define ENEWEXE     sizeof(struct exe_hdr)/*!< Size of the MZ header. */

#define ENEWHDR     0x003C                /*!< Offset of e_lfanew. */
#define ERESWDS     0x0010                /*!< Reserved words. */
#define ERES1WDS    0x0004                /*!< Count of e_res words. */
#define ERES2WDS    0x000A                /*!< Count of e_res2 words. */
#define ECP         0x0004                /*!< e_cp offset. */
#define ECBLP       0x0002                /*!< e_cblp offset. */
#define EMINALLOC   0x000A                /*!< e_minalloc offset. */
/*! @} */

/*! @name MZ header field accessors */
/*! @{ */
#define E_MAGIC(x)      (x).e_magic       /*!< Magic ("MZ"). */
#define E_CBLP(x)       (x).e_cblp        /*!< Bytes in last block. */
#define E_CP(x)         (x).e_cp          /*!< Blocks in file. */
#define E_CRLC(x)       (x).e_crlc        /*!< Relocation entries. */
#define E_CPARHDR(x)    (x).e_cparhdr     /*!< Header size in paras. */
#define E_MINALLOC(x)   (x).e_minalloc    /*!< Min extra paragraphs. */
#define E_MAXALLOC(x)   (x).e_maxalloc    /*!< Max extra paragraphs. */
#define E_SS(x)         (x).e_ss          /*!< Initial SS. */
#define E_SP(x)         (x).e_sp          /*!< Initial SP. */
#define E_CSUM(x)       (x).e_csum        /*!< Checksum. */
#define E_IP(x)         (x).e_ip          /*!< Initial IP. */
#define E_CS(x)         (x).e_cs          /*!< Initial CS. */
#define E_LFARLC(x)     (x).e_lfarlc      /*!< Relocation table offset. */
#define E_OVNO(x)       (x).e_ovno        /*!< Overlay number. */
#define E_RES(x)        (x).e_res         /*!< Reserved. */
#define E_OEMID(x)      (x).e_oemid       /*!< OEM identifier. */
#define E_OEMINFO(x)    (x).e_oeminfo     /*!< OEM info. */
#define E_RES2(x)       (x).e_res2        /*!< Reserved. */
#define E_LFANEW(x)     (x).e_lfanew      /*!< File offset of NE header. */
/*! @} */

/*! @name NE header constants */
/*! @{ */
#define NEMAGIC         0x454E            /*!< NE signature ("NE"). */
#define NERESBYTES      8                 /*!< Reserved bytes. */
#define NECRC           8                 /*!< CRC bytes. */
/*! @} */

/*! @name NE header aliases
 *
 *  Aliases used for the in-memory view of the NE header.
 */
/*! @{ */
#define ne_pWinFileStruc (ne_magic + 0x0a) /*!< Offset of OFSTRUCT. */
#define ne_cbModName     0                 /*!< Module name length. */
#define ne_pWinModName   8                 /*!< Module name pointer. */
/*! @} */

/*! @name NE header field accessors */
/*! @{ */
#define NE_MAGIC(x)         (x).ne_magic
#define NE_VER(x)           (x).ne_ver
#define NE_REV(x)           (x).ne_rev
#define NE_ENTTAB(x)        (x).ne_enttab
#define NE_CBENTTAB(x)      (x).ne_cbenttab
#define NE_CRC(x)           (x).ne_crc
#define NE_FLAGS(x)         (x).ne_flags
#define NE_AUTODATA(x)      (x).ne_autodata
#define NE_HEAP(x)          (x).ne_heap
#define NE_STACK(x)         (x).ne_stack
#define NE_CSIP(x)          (x).ne_csip
#define NE_SSSP(x)          (x).ne_sssp
#define NE_CSEG(x)          (x).ne_cseg
#define NE_CMOD(x)          (x).ne_cmod
#define NE_CBNRESTAB(x)     (x).ne_cbnrestab
#define NE_SEGTAB(x)        (x).ne_segtab
#define NE_RSRCTAB(x)       (x).ne_rsrctab
#define NE_RESTAB(x)        (x).ne_restab
#define NE_MODTAB(x)        (x).ne_modtab
#define NE_IMPTAB(x)        (x).ne_imptab
#define NE_NRESTAB(x)       (x).ne_nrestab
#define NE_CMOVENT(x)       (x).ne_cmovent
#define NE_ALIGN(x)         (x).ne_align
#define NE_CRES(x)          (x).ne_cres
#define NE_RES(x)           (x).ne_res
#define NE_EXETYP(x)        (x).ne_exetyp
#define NE_FLAGSOTHERS(x)   (x).ne_flagsothers
/*! @} */

#define NE_USAGE(x)     (WORD)*((WORD *)(x)+1)
#define NE_PNEXTEXE(x)  (WORD)(x).ne_cbenttab
#define NE_ONEWEXE(x)   (WORD)(x).ne_crc
#define NE_PFILEINFO(x) (WORD)((DWORD)(x).ne_crc >> 16)

/*! @name NE executable types
 *
 *  Values for the ne_exetyp field. The full list documented in the
 *  original NE specification also includes 05h (BOSS), 81h and 82h
 *  (PharLap 286|DOS-Extender for OS/2 and Windows); only the ones
 *  used by the binder are defined here.
 */
/*! @{ */
#define NE_UNKNOWN      0x0   /*!< Unknown. */
#define NE_OS2          0x1   /*!< OS/2. */
#define NE_WINDOWS      0x2   /*!< Windows. */
#define NE_DOS4         0x3   /*!< European MS-DOS 4.x. */
#define NE_DEV386       0x4   /*!< Windows 386. */
/*! @} */

/*! @name NE flag word values
 *
 *  Values for the ne_flags field.
 */
/*! @{ */
#define NENOTP          0x8000  /*!< Not a Windows program. */
#define NENOTMPSAFE     0x4000  /*!< Not MP-safe. */
#define NEIERR          0x2000  /*!< Errors in image. */
#define NEBOUND         0x0800  /*!< Bound. */
#define NEAPPTYP        0x0700  /*!< Application type (mask). */
#define NENOTWINCOMPAT  0x0100  /*!< Not Windows-compatible. */
#define NEWINCOMPAT     0x0200  /*!< Windows-compatible, not PM. */
#define NEWINAPI        0x0300  /*!< Uses Windows API. */
#define NEFLTP          0x0080  /*!< Floating-point. */
#define NEI386          0x0040  /*!< 386 instructions. */
#define NEI286          0x0020  /*!< 286 instructions. */
#define NEI086          0x0010  /*!< 086 instructions. */
#define NEPROT          0x0008  /*!< Protected mode only. */
#define NEPPLI          0x0004  /*!< Per-process library init. */
#define NEINST          0x0002  /*!< Instance data. */
#define NESOLO          0x0001  /*!< Solo data. */
/*! @} */

/*! @name NE flagsothers values
 *
 *  Values for the ne_flagsothers field.
 */
/*! @{ */
#define NELONGNAMES     0x01    /*!< Long names present. */
#define NEWINISPROT     0x02    /*!< Windows protected. */
#define NEWINGETPROPFON 0x04    /*!< Get property function. */
#define NEWLOAPPL       0x80    /*!< Non-Windows application. */
/*! @} */

/*! @name Segment table field accessors */
/*! @{ */
#define NS_SECTOR(x)    (x).ns_sector   /*!< File sector. */
#define NS_CBSEG(x)     (x).ns_cbseg    /*!< Segment length. */
#define NS_FLAGS(x)     (x).ns_flags    /*!< Segment flags. */
#define NS_MINALLOC(x)  (x).ns_minalloc /*!< Minimum allocation. */
/*! @} */

#define NSTYPE          0x0007  /*!< Mask of segment type bits. */

#if (EXE386 == 0)
#define NSCODE          0x0000  /*!< Code segment. */
#define NSDATA          0x0001  /*!< Data segment. */
#define NSITER          0x0008  /*!< Iterated data. */
#define NSMOVE          0x0010  /*!< Movable. */
#define NSSHARED        0x0020  /*!< Shared. */
#define NSPRELOAD       0x0040  /*!< Preload. */
#define NSEXRD          0x0080  /*!< Execute/read only. */

#define NSRELOC         0x0100  /*!< Has relocations. */
#define NSCONFORM       0x0200  /*!< Conforming. */
#define NSEXPDOWN       0x0200  /*!< Expand down. */
#define NSDPL           0x0C00  /*!< Descriptor privilege level. */
#define SHIFTDPL        10      /*!< DPL shift count. */
#define NSDISCARD       0x1000  /*!< Discardable. */
#define NS32BIT         0x2000  /*!< 32-bit segment. */
#define NSHUGE          0x4000  /*!< Huge segment. */

#define NSGDT           0x8000  /*!< Uses GDT. */
#define NSPURE          NSSHARED/*!< Pure (alias for shared). */
#define NSALIGN         9       /*!< Default alignment shift. */
#define NSLOADED        0x0004  /*!< Loaded flag. */
#endif

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

/*! @brief New Executable (NE) header.
 *
 *  Follows the MZ header at file offset E_LFANEW. The layout matches
 *  the 64-byte on-disk NE header exactly; unions are provided for
 *  fields whose meaning differs between on-disk and in-memory
 *  forms. See Matt Pietrek, "Windows Internals", Addison-Wesley,
 *  1993, p. 219 for the in-memory view.
 */
struct new_exe {
    WORD  ne_magic;         /*!< Signature word, EMAGIC. */
    union {
        struct {
            BYTE   ne_ver;  /*!< Version number of the linker (on
                             *   disk). */
            BYTE   ne_rev;  /*!< Revision number of the linker (on
                             *   disk). */
        };
        WORD  count;        /*!< Usage count (in memory). */
    };
    WORD  ne_enttab;        /*!< Entry Table file offset, relative to
                             *   the beginning of the segmented EXE
                             *   header. */
    union {
        WORD  ne_cbenttab;  /*!< Number of bytes in the entry table
                             *   (on disk). */
        WORD  next;         /*!< Selector to next module (in
                             *   memory). */
    };
    union {
        DWORD ne_crc;       /*!< 32-bit CRC of entire contents of
                             *   file. These words are taken as 00
                             *   during the calculation (on disk). */
        struct {
            WORD dgroup_entry; /*!< Near ptr to segment entry for
                                *   DGROUP (in memory). */
            WORD fileinfo;     /*!< Near ptr to file info (OFSTRUCT)
                                *   (in memory). */
        };
    };
    WORD  ne_flags;         /*!< Flag word (see NE flag word values). */
    WORD  ne_autodata;      /*!< Segment number of automatic data
                             *   segment. Zero if SINGLEDATA and
                             *   MULTIPLEDATA flag bits are clear
                             *   (NOAUTODATA is indicated in the flags
                             *   word). A segment number is an index
                             *   into the module's segment table; the
                             *   first entry is segment number 1. */
    WORD  ne_heap;          /*!< Initial size, in bytes, of dynamic
                             *   heap added to the data segment. Zero
                             *   if no initial local heap is
                             *   allocated. */
    WORD  ne_stack;         /*!< Initial size, in bytes, of stack
                             *   added to the data segment. Zero to
                             *   indicate no initial stack allocation,
                             *   or when SS is not equal to DS. */
    DWORD ne_csip;          /*!< Segment number:offset of CS:IP. */
    DWORD ne_sssp;          /*!< Segment number:offset of SS:SP. If
                             *   SS equals the automatic data segment
                             *   and SP equals zero, the stack pointer
                             *   is set to the top of the automatic
                             *   data segment just below the
                             *   additional heap area:
                             *   @verbatim
                                 +--------------------------+
                                 | additional dynamic heap  |
                                 +--------------------------+ <- SP
                                 |    additional stack      |
                                 +--------------------------+
                                 | loaded auto data segment |
                                 +--------------------------+ <- DS, SS
                                 @endverbatim */
    WORD  ne_cseg;          /*!< Number of entries in the Segment
                             *   Table. */
    WORD  ne_cmod;          /*!< Number of entries in the Module
                             *   Reference Table. */
    WORD  ne_cbnrestab;     /*!< Number of bytes in the Non-Resident
                             *   Name Table. */
    WORD  ne_segtab;        /*!< Segment Table file offset, relative
                             *   to the beginning of the segmented
                             *   EXE header. */
    WORD  ne_rsrctab;       /*!< Resource Table file offset, relative
                             *   to the beginning of the segmented
                             *   EXE header. */
    WORD  ne_restab;        /*!< Resident Name Table file offset,
                             *   relative to the beginning of the
                             *   segmented EXE header. */
    WORD  ne_modtab;        /*!< Module Reference Table file offset,
                             *   relative to the beginning of the
                             *   segmented EXE header. */
    WORD  ne_imptab;        /*!< Imported Names Table file offset,
                             *   relative to the beginning of the
                             *   segmented EXE header. */
    DWORD ne_nrestab;       /*!< Non-Resident Name Table offset,
                             *   relative to the beginning of the
                             *   file. */
    WORD  ne_cmovent;       /*!< Number of movable entries in the
                             *   Entry Table. */
    WORD  ne_align;         /*!< Logical sector alignment shift
                             *   count, log(base 2) of the segment
                             *   sector size (default 9). */
    WORD  ne_cres;          /*!< Number of resource entries. */
    BYTE  ne_exetyp;        /*!< Executable type used by loader
                             *   (NE_OS2, NE_WINDOWS, etc.). */
    BYTE  ne_flagsothers;   /*!< Operating system flags. */
    WORD  dlls_to_init;     /*!< 38 List of DLLs to initialize
                             *   (ne_pretthunks on disk). */
    WORD  nrname_handle;    /*!< 3a Handle to non-resident name table
                             *   (ne_psegrefbytes on disk). */
    WORD  ne_swaparea;      /*!< 3c Minimum swap area size. */
    WORD  ne_expver;        /*!< 3e Expected Windows version. */
};

/*! @brief On-disk segment table entry. */
struct new_seg {
    WORD  ns_sector;    /*!< Logical-sector offset (n bytes) to the
                         *   contents of the segment data, relative
                         *   to the beginning of the file. Zero means
                         *   no file data. */
    WORD  ns_cbseg;     /*!< Length of the segment in the file, in
                         *   bytes. Zero means 64K. */
    WORD  ns_flags;     /*!< Flag word. */
    WORD  ns_minalloc;  /*!< Minimum allocation size of the segment,
                         *   in bytes. Total size of the segment.
                         *   Zero means 64K. */
};

/*! @brief In-memory segment table entry. */
struct new_seg1 {
    WORD  ns1_sector;   /*!< Logical-sector offset (n bytes) to the
                         *   contents of the segment data, relative
                         *   to the beginning of the file. Zero means
                         *   no file data. */
    WORD  ns1_cbseg;    /*!< Length of the segment in the file, in
                         *   bytes. Zero means 64K. */
    WORD  ns1_flags;    /*!< Flag word. */
    WORD  ns1_minalloc; /*!< Minimum allocation size of the segment,
                         *   in bytes. Total size of the segment.
                         *   Zero means 64K. */
    WORD  ns1_handle;   /*!< Selector or handle (selector - 1) of
                         *   segment in memory. */
};

/*! @brief Segment data header (iterated or non-iterated). */
struct new_segdata {
    union {
        struct {
            WORD      ns_niter;   /*!< Iteration count. */
            WORD      ns_nbytes;  /*!< Bytes per iteration. */
            char      ns_iterdata;/*!< First iterated byte. */
        } ns_iter;
        struct {
            char      ns_data;    /*!< First data byte. */
        } ns_noniter;
    } ns_union;
};

/*! @brief Relocation table header. */
struct new_rlcinfo {
    WORD  nr_nreloc;    /*!< Number of relocation entries. */
};

/*! @brief Relocation table entry. */
struct new_rlc {
    char  nr_stype;     /*!< Source type. */
    char  nr_flags;     /*!< Flags (relocation type in low bits). */
    WORD  nr_soff;      /*!< Source offset. */
    union {
        struct {
            char nr_segno;  /*!< Internal segment reference. */
            char nr_res;    /*!< Reserved. */
            WORD nr_entry;  /*!< Entry index. */
        } nr_intref;
        struct {
            WORD nr_mod;    /*!< Module reference index. */
            WORD nr_proc;   /*!< Procedure ordinal or name offset. */
        } nr_import;
        struct {
            WORD nr_ostype; /*!< OS fixup type. */
            WORD nr_osres;  /*!< Reserved. */
        } nr_osfix;
    } nr_union;
};

/*! @name Relocation entry accessors */
/*! @{ */
#define NR_STYPE(x)     (x).nr_stype
#define NR_FLAGS(x)     (x).nr_flags
#define NR_SOFF(x)      (x).nr_soff
#define NR_SEGNO(x)     (x).nr_union.nr_intref.nr_segno
#define NR_RES(x)       (x).nr_union.nr_intref.nr_res
#define NR_ENTRY(x)     (x).nr_union.nr_intref.nr_entry
#define NR_MOD(x)       (x).nr_union.nr_import.nr_mod
#define NR_PROC(x)      (x).nr_union.nr_import.nr_proc
#define NR_OSTYPE(x)    (x).nr_union.nr_osfix.nr_ostype
#define NR_OSRES(x)     (x).nr_union.nr_osfix.nr_osres
/*! @} */

/*! @name Relocation source types */
/*! @{ */
#define NRSTYP      0x0f    /*!< Mask of source type. */
#define NRSBYT      0x00    /*!< Low byte. */
#define NRSSEG      0x02    /*!< Segment. */
#define NRSPTR      0x03    /*!< Far pointer. */
#define NRSOFF      0x05    /*!< Offset. */
#define NRPTR48     0x06    /*!< 48-bit pointer. */
#define NROFF32     0x07    /*!< 32-bit offset. */
#define NRSOFF32    0x08    /*!< 32-bit self-relative offset. */
/*! @} */

#define NRADD       0x04    /*!< Additive fixup. */

/*! @name Relocation types */
/*! @{ */
#define NRRTYP      0x03    /*!< Mask of relocation type. */
#define NRRINT      0x00    /*!< Internal reference. */
#define NRRORD      0x01    /*!< Import by ordinal. */
#define NRRNAM      0x02    /*!< Import by name. */
#define NRROSF      0x03    /*!< OS fixup. */
/*! @} */

#define NRICHAIN    0x08    /*!< Chain bit. */

#if (EXE386 == 0)

#define RS_LEN(x)       (x).rs_len
#define RS_STRING(x)    (x).rs_string
#define RS_ALIGN(x)     (x).rs_align

#define RT_ID(x)        (x).rt_id
#define RT_NRES(x)      (x).rt_nres
#define RT_PROC(x)      (x).rt_proc

#define RN_OFFSET(x)    (x).rn_offset
#define RN_LENGTH(x)    (x).rn_length
#define RN_FLAGS(x)     (x).rn_flags
#define RN_ID(x)        (x).rn_id
#define RN_HANDLE(x)    (x).rn_handle
#define RN_USAGE(x)     (x).rn_usage

#define RSORDID     0x8000  /*!< Resource ordinal ID flag. */

#define RNMOVE      0x0010  /*!< Movable resource. */
#define RNPURE      0x0020  /*!< Pure resource. */
#define RNPRELOAD   0x0040  /*!< Preload resource. */
#define RNDISCARD   0xF000  /*!< Discard priority mask. */

#define NE_FFLAGS_LIBMODULE 0x8000  /*!< Library module flag. */

/*! @brief Resource string. */
struct rsrc_string {
    char rs_len;            /*!< Length of the string. */
    char rs_string[1];      /*!< First character. */
};

/*! @brief Resource type info. */
struct rsrc_typeinfo {
    WORD  rt_id;            /*!< Resource type ID. */
    WORD  rt_nres;          /*!< Number of resources. */
    DWORD rt_proc;          /*!< Reserved. */
};

/*! @brief Resource name info. */
struct rsrc_nameinfo {
    WORD  rn_offset;        /*!< Offset within resource data. */
    WORD  rn_length;        /*!< Length of resource. */
    WORD  rn_flags;         /*!< Flags. */
    WORD  rn_id;            /*!< Resource ID. */
    WORD  rn_handle;        /*!< Handle. */
    WORD  rn_usage;         /*!< Usage flags. */
};

/*! @brief Resource table header. */
struct new_rsrc {
    WORD rs_align;          /*!< Alignment shift count. */
    struct rsrc_typeinfo rs_typeinfo; /*!< First type info. */
};

#endif

#pragma pack(pop)

/* ------------------------------------------------------------------ */
/* NE file access API                                                  */
/* ------------------------------------------------------------------ */

/*! @brief Handle to an open NE executable.
 *
 *  Opaque. Created by NeOpen and released by NeClose. The internal
 *  representation is private to newexe.c.
 *
 *  @see NeOpen
 *  @see NeClose
 */
typedef HANDLE HNE;

/*! @brief Open an NE executable file.
 *
 *  Opens @p pszPath, reads the MZ header and the NE header, and
 *  validates their magic numbers. The headers are cached in the
 *  handle and returned by NeQueryMZHeader and NeQueryHeader.
 *
 *  @param[in]  pszPath  Path to the executable. Not NULL.
 *  @param[out] phNe     Receives the handle. Not NULL. Set to
 *                       NULLHANDLE on failure.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p pszPath or @p phNe is NULL.
 *  @retval ERROR_OPEN_FAILED        File cannot be opened.
 *  @retval ERROR_READ_FAULT         Not a valid MZ/NE file.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 *  @see NeClose
 */
APIRET APIENTRY NeOpen(PCSZ pszPath, HNE *phNe);

/*! @brief Close an NE executable.
 *
 *  Closes the underlying file and releases the handle. This function
 *  is idempotent: passing NULLHANDLE returns NO_ERROR.
 *
 *  @param[in] hNe  Handle from NeOpen. NULLHANDLE is accepted.
 *
 *  @return APIRET
 *  @retval NO_ERROR  Always.
 *
 *  @see NeOpen
 */
APIRET APIENTRY NeClose(HNE hNe);

/*! @brief Retrieve the cached MZ header.
 *
 *  Copies the MZ header that was read by NeOpen into the caller's
 *  buffer.
 *
 *  @param[in]  hNe  Handle. Not NULLHANDLE.
 *  @param[out] pMZ  Receives the header. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hNe or @p pMZ is invalid.
 *
 *  @see NeOpen
 */
APIRET APIENTRY NeQueryMZHeader(HNE hNe, struct exe_hdr *pMZ);

/*! @brief Retrieve the cached NE header.
 *
 *  Copies the NE header that was read by NeOpen into the caller's
 *  buffer.
 *
 *  @param[in]  hNe  Handle. Not NULLHANDLE.
 *  @param[out] pNE  Receives the header. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hNe or @p pNE is invalid.
 *
 *  @see NeOpen
 */
APIRET APIENTRY NeQueryHeader(HNE hNe, struct new_exe *pNE);

/*! @brief Return the number of module references.
 *
 *  @param[in]  hNe       Handle. Not NULLHANDLE.
 *  @param[out] pusCount  Receives the count. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hNe or @p pusCount is invalid.
 */
APIRET APIENTRY NeQueryModuleCount(HNE hNe, PUSHORT pusCount);

/*! @brief Return the name of a module reference.
 *
 *  The module reference table stores WORD offsets into the imported
 *  names table, where each entry is a length-prefixed string. This
 *  function resolves one such entry and copies the name into
 *  @p pszName as a NUL-terminated string.
 *
 *  @param[in]  hNe      Handle. Not NULLHANDLE.
 *  @param[in]  usIndex  Module index, 1-based.
 *  @param[out] pszName  Receives a NUL-terminated name. Not NULL.
 *  @param[in]  cbName   Size of @p pszName including the NUL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad index or buffer too small.
 *  @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY NeQueryModuleName(HNE hNe, USHORT usIndex,
                                  PSZ pszName, ULONG cbName);

/*! @brief Return the number of segments.
 *
 *  @param[in]  hNe       Handle. Not NULLHANDLE.
 *  @param[out] pusCount  Receives the count. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hNe or @p pusCount is invalid.
 */
APIRET APIENTRY NeQuerySegmentCount(HNE hNe, PUSHORT pusCount);

/*! @brief Return one segment table entry.
 *
 *  @param[in]  hNe      Handle. Not NULLHANDLE.
 *  @param[in]  usIndex  Segment index, 1-based.
 *  @param[out] pSeg     Receives the entry. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad index or NULL pointer.
 *  @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY NeQuerySegment(HNE hNe, USHORT usIndex,
                               struct new_seg *pSeg);

/*! @brief Return the number of relocations for a segment.
 *
 *  The relocation table immediately follows the segment data and
 *  starts with a WORD count.
 *
 *  @param[in]  hNe        Handle. Not NULLHANDLE.
 *  @param[in]  usSegment  Segment index, 1-based.
 *  @param[out] pusCount   Receives the count. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad parameters.
 *  @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY NeQueryRelocCount(HNE hNe, USHORT usSegment,
                                  PUSHORT pusCount);

/*! @brief Return one relocation table entry.
 *
 *  @param[in]  hNe        Handle. Not NULLHANDLE.
 *  @param[in]  usSegment  Segment index, 1-based.
 *  @param[in]  usIndex    Relocation index, 0-based.
 *  @param[out] pRlc       Receives the entry. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad parameters.
 *  @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY NeQueryReloc(HNE hNe, USHORT usSegment,
                             USHORT usIndex,
                             struct new_rlc *pRlc);

/*! @brief Bind a DOS stub and an NE image into one file.
 *
 *  Copies @p pszStub verbatim into @p pszOutput, pads the output to
 *  the NE segment alignment, appends the NE image from @p pszInput,
 *  and patches the output segment table and non-resident name table
 *  so that all internal file offsets remain valid after the shift.
 *
 *  Unlike the historical bind tool, this function does not remove
 *  the stub file, does not delete @p pszInput, and does not perform
 *  any rename. The caller is responsible for those steps.
 *
 *  @param[in] pszStub    DOS stub file. Not NULL.
 *  @param[in] pszInput   NE executable. Not NULL. Must differ from
 *                        @p pszOutput.
 *  @param[in] pszOutput  Destination file. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  A parameter is NULL.
 *  @retval ERROR_OPEN_FAILED        A file cannot be opened.
 *  @retval ERROR_READ_FAULT         Read error.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET APIENTRY NeBind(PCSZ pszStub, PCSZ pszInput,
                       PCSZ pszOutput);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __NEWEXE__ */
