/*! newexe.h - New Executable (NE) format
 *
 *  Provides the on-disk NE header structures, accessors, and the
 *  public API of the NE reader/binder library.
 */

#ifndef __NEWEXE__
#define __NEWEXE__

#include "os2types.h"
#include "os2err.h"
#include "mzexe.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @file newexe.h
 *  @brief New Executable (NE) format structures and access API.
 *
 *  Provides handle-based read access to NE executables plus a bind
 *  operation that merges a DOS stub with an NE image into a single
 *  FamilyAPI executable. All handles (HNE, HNEENUM) are opaque; the
 *  internal representation is private to newexe.c.
 *
 *  The MZ header that precedes the NE header is defined in mzexe.h
 *  and shared with the LX reader.
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

/* ==================================================================
 * NE header constants
 * ================================================================== */

/**
 * @def NEMAGIC
 * @brief NE signature ("NE"). Value: 0x454E.
 */
#define NEMAGIC         0x454E

/**
 * @def NERESBYTES
 * @brief Reserved bytes. Value: 8.
 */
#define NERESBYTES      8

/**
 * @def NECRC
 * @brief CRC bytes. Value: 8.
 */
#define NECRC           8

/* ==================================================================
 * NE header aliases
 *
 * Aliases used for the in-memory view of the NE header.
 * ================================================================== */

/**
 * @def ne_pWinFileStruc
 * @brief Offset of OFSTRUCT.
 */
#define ne_pWinFileStruc (ne_magic + 0x0a)

/**
 * @def ne_cbModName
 * @brief Module name length.
 */
#define ne_cbModName     0

/**
 * @def ne_pWinModName
 * @brief Module name pointer.
 */
#define ne_pWinModName   8

/* ==================================================================
 * NE header field accessors
 * ================================================================== */

/**
 * @def NE_MAGIC(x)
 * @brief Accessor for the ne_magic field.
 * @param x NE header structure.
 */
#define NE_MAGIC(x)         (x).ne_magic

/**
 * @def NE_VER(x)
 * @brief Accessor for the ne_ver field (linker version).
 * @param x NE header structure.
 */
#define NE_VER(x)           (x).ne_ver

/**
 * @def NE_REV(x)
 * @brief Accessor for the ne_rev field (linker revision).
 * @param x NE header structure.
 */
#define NE_REV(x)           (x).ne_rev

/**
 * @def NE_ENTTAB(x)
 * @brief Accessor for the ne_enttab field (Entry Table file offset,
 *        relative to the beginning of the segmented EXE header).
 * @param x NE header structure.
 */
#define NE_ENTTAB(x)        (x).ne_enttab

/**
 * @def NE_CBENTTAB(x)
 * @brief Accessor for the ne_cbenttab field (number of bytes in the
 *        entry table, on disk).
 * @param x NE header structure.
 */
#define NE_CBENTTAB(x)      (x).ne_cbenttab

/**
 * @def NE_CRC(x)
 * @brief Accessor for the ne_crc field (32-bit CRC of entire contents
 *        of file; words taken as 00 during the calculation, on disk).
 * @param x NE header structure.
 */
#define NE_CRC(x)           (x).ne_crc

/**
 * @def NE_FLAGS(x)
 * @brief Accessor for the ne_flags field.
 * @param x NE header structure.
 */
#define NE_FLAGS(x)         (x).ne_flags

/**
 * @def NE_AUTODATA(x)
 * @brief Accessor for the ne_autodata field (segment number of
 *        automatic data segment).
 * @param x NE header structure.
 */
#define NE_AUTODATA(x)      (x).ne_autodata

/**
 * @def NE_HEAP(x)
 * @brief Accessor for the ne_heap field (initial size of dynamic
 *        heap).
 * @param x NE header structure.
 */
#define NE_HEAP(x)          (x).ne_heap

/**
 * @def NE_STACK(x)
 * @brief Accessor for the ne_stack field (initial size of stack).
 * @param x NE header structure.
 */
#define NE_STACK(x)         (x).ne_stack

/**
 * @def NE_CSIP(x)
 * @brief Accessor for the ne_csip field (segment number:offset of
 *        CS:IP).
 * @param x NE header structure.
 */
#define NE_CSIP(x)          (x).ne_csip

/**
 * @def NE_SSSP(x)
 * @brief Accessor for the ne_sssp field (segment number:offset of
 *        SS:SP).
 * @param x NE header structure.
 */
#define NE_SSSP(x)          (x).ne_sssp

/**
 * @def NE_CSEG(x)
 * @brief Accessor for the ne_cseg field (number of entries in the
 *        Segment Table).
 * @param x NE header structure.
 */
#define NE_CSEG(x)          (x).ne_cseg

/**
 * @def NE_CMOD(x)
 * @brief Accessor for the ne_cmod field (number of entries in the
 *        Module Reference Table).
 * @param x NE header structure.
 */
#define NE_CMOD(x)          (x).ne_cmod

/**
 * @def NE_CBNRESTAB(x)
 * @brief Accessor for the ne_cbnrestab field (number of bytes in the
 *        Non-Resident Name Table).
 * @param x NE header structure.
 */
#define NE_CBNRESTAB(x)     (x).ne_cbnrestab

/**
 * @def NE_SEGTAB(x)
 * @brief Accessor for the ne_segtab field (Segment Table file offset,
 *        relative to the beginning of the segmented EXE header).
 * @param x NE header structure.
 */
#define NE_SEGTAB(x)        (x).ne_segtab

/**
 * @def NE_RSRCTAB(x)
 * @brief Accessor for the ne_rsrctab field (Resource Table file
 *        offset, relative to the beginning of the segmented EXE
 *        header).
 * @param x NE header structure.
 */
#define NE_RSRCTAB(x)       (x).ne_rsrctab

/**
 * @def NE_RESTAB(x)
 * @brief Accessor for the ne_restab field (Resident Name Table file
 *        offset, relative to the beginning of the segmented EXE
 *        header).
 * @param x NE header structure.
 */
#define NE_RESTAB(x)        (x).ne_restab

/**
 * @def NE_MODTAB(x)
 * @brief Accessor for the ne_modtab field (Module Reference Table
 *        file offset, relative to the beginning of the segmented EXE
 *        header).
 * @param x NE header structure.
 */
#define NE_MODTAB(x)        (x).ne_modtab

/**
 * @def NE_IMPTAB(x)
 * @brief Accessor for the ne_imptab field (Imported Names Table file
 *        offset, relative to the beginning of the segmented EXE
 *        header).
 * @param x NE header structure.
 */
#define NE_IMPTAB(x)        (x).ne_imptab

/**
 * @def NE_NRESTAB(x)
 * @brief Accessor for the ne_nrestab field (Non-Resident Name Table
 *        offset, relative to the beginning of the file).
 * @param x NE header structure.
 */
#define NE_NRESTAB(x)       (x).ne_nrestab

/**
 * @def NE_CMOVENT(x)
 * @brief Accessor for the ne_cmovent field (number of movable entries
 *        in the Entry Table).
 * @param x NE header structure.
 */
#define NE_CMOVENT(x)       (x).ne_cmovent

/**
 * @def NE_ALIGN(x)
 * @brief Accessor for the ne_align field (logical sector alignment
 *        shift count).
 * @param x NE header structure.
 */
#define NE_ALIGN(x)         (x).ne_align

/**
 * @def NE_CRES(x)
 * @brief Accessor for the ne_cres field (number of resource entries).
 * @param x NE header structure.
 */
#define NE_CRES(x)          (x).ne_cres

/**
 * @def NE_RES(x)
 * @brief Accessor for the ne_res field (reserved).
 * @param x NE header structure.
 */
#define NE_RES(x)           (x).ne_res

/**
 * @def NE_EXETYP(x)
 * @brief Accessor for the ne_exetyp field (executable type used by
 *        loader).
 * @param x NE header structure.
 */
#define NE_EXETYP(x)        (x).ne_exetyp

/**
 * @def NE_FLAGSOTHERS(x)
 * @brief Accessor for the ne_flagsothers field (operating system
 *        flags).
 * @param x NE header structure.
 */
#define NE_FLAGSOTHERS(x)   (x).ne_flagsothers

/**
 * @def NE_USAGE(x)
 * @brief In-memory usage count accessor.
 * @param x NE header structure.
 */
#define NE_USAGE(x)     (WORD)*((WORD *)(x)+1)

/**
 * @def NE_PNEXTEXE(x)
 * @brief In-memory next-module selector accessor.
 * @param x NE header structure.
 */
#define NE_PNEXTEXE(x)  (WORD)(x).ne_cbenttab

/**
 * @def NE_ONEWEXE(x)
 * @brief In-memory DGROUP segment entry accessor.
 * @param x NE header structure.
 */
#define NE_ONEWEXE(x)   (WORD)(x).ne_crc

/**
 * @def NE_PFILEINFO(x)
 * @brief In-memory file-info pointer accessor.
 * @param x NE header structure.
 */
#define NE_PFILEINFO(x) (WORD)((DWORD)(x).ne_crc >> 16)

/* ==================================================================
 * NE executable types
 *
 * Values for the ne_exetyp field. The full list documented in the
 * original NE specification also includes 05h (BOSS), 81h and 82h
 * (PharLap 286|DOS-Extender for OS/2 and Windows); only the ones
 * used by the binder are defined here.
 * ================================================================== */

/**
 * @def NE_UNKNOWN
 * @brief Unknown. Value: 0x0.
 */
#define NE_UNKNOWN      0x0

/**
 * @def NE_OS2
 * @brief OS/2. Value: 0x1.
 */
#define NE_OS2          0x1

/**
 * @def NE_WINDOWS
 * @brief Windows. Value: 0x2.
 */
#define NE_WINDOWS      0x2

/**
 * @def NE_DOS4
 * @brief European MS-DOS 4.x. Value: 0x3.
 */
#define NE_DOS4         0x3

/**
 * @def NE_DEV386
 * @brief Windows 386. Value: 0x4.
 */
#define NE_DEV386       0x4

/* ==================================================================
 * NE flag word values
 *
 * Values for the ne_flags field.
 * ================================================================== */

/**
 * @def NENOTP
 * @brief Not a Windows program. Value: 0x8000.
 */
#define NENOTP          0x8000

/**
 * @def NENOTMPSAFE
 * @brief Not MP-safe. Value: 0x4000.
 */
#define NENOTMPSAFE     0x4000

/**
 * @def NEIERR
 * @brief Errors in image. Value: 0x2000.
 */
#define NEIERR          0x2000

/**
 * @def NEBOUND
 * @brief Bound. Value: 0x0800.
 */
#define NEBOUND         0x0800

/**
 * @def NEAPPTYP
 * @brief Application type (mask). Value: 0x0700.
 */
#define NEAPPTYP        0x0700

/**
 * @def NENOTWINCOMPAT
 * @brief Not Windows-compatible. Value: 0x0100.
 */
#define NENOTWINCOMPAT  0x0100

/**
 * @def NEWINCOMPAT
 * @brief Windows-compatible, not PM. Value: 0x0200.
 */
#define NEWINCOMPAT     0x0200

/**
 * @def NEWINAPI
 * @brief Uses Windows API. Value: 0x0300.
 */
#define NEWINAPI        0x0300

/**
 * @def NEFLTP
 * @brief Floating-point. Value: 0x0080.
 */
#define NEFLTP          0x0080

/**
 * @def NEI386
 * @brief 386 instructions. Value: 0x0040.
 */
#define NEI386          0x0040

/**
 * @def NEI286
 * @brief 286 instructions. Value: 0x0020.
 */
#define NEI286          0x0020

/**
 * @def NEI086
 * @brief 086 instructions. Value: 0x0010.
 */
#define NEI086          0x0010

/**
 * @def NEPROT
 * @brief Protected mode only. Value: 0x0008.
 */
#define NEPROT          0x0008

/**
 * @def NEPPLI
 * @brief Per-process library init. Value: 0x0004.
 */
#define NEPPLI          0x0004

/**
 * @def NEINST
 * @brief Instance data. Value: 0x0002.
 */
#define NEINST          0x0002

/**
 * @def NESOLO
 * @brief Solo data. Value: 0x0001.
 */
#define NESOLO          0x0001

/* ==================================================================
 * NE flagsothers values
 *
 * Values for the ne_flagsothers field.
 * ================================================================== */

/**
 * @def NELONGNAMES
 * @brief Long names present. Value: 0x01.
 */
#define NELONGNAMES     0x01

/**
 * @def NEWINISPROT
 * @brief Windows protected. Value: 0x02.
 */
#define NEWINISPROT     0x02

/**
 * @def NEWINGETPROPFON
 * @brief Get property function. Value: 0x04.
 */
#define NEWINGETPROPFON 0x04

/**
 * @def NEWLOAPPL
 * @brief Non-Windows application. Value: 0x80.
 */
#define NEWLOAPPL       0x80

/* ==================================================================
 * Segment table field accessors
 * ================================================================== */

/**
 * @def NS_SECTOR(x)
 * @brief Accessor for the ns_sector field (file sector).
 * @param x Segment table entry.
 */
#define NS_SECTOR(x)    (x).ns_sector

/**
 * @def NS_CBSEG(x)
 * @brief Accessor for the ns_cbseg field (segment length).
 * @param x Segment table entry.
 */
#define NS_CBSEG(x)     (x).ns_cbseg

/**
 * @def NS_FLAGS(x)
 * @brief Accessor for the ns_flags field (segment flags).
 * @param x Segment table entry.
 */
#define NS_FLAGS(x)     (x).ns_flags

/**
 * @def NS_MINALLOC(x)
 * @brief Accessor for the ns_minalloc field (minimum allocation).
 * @param x Segment table entry.
 */
#define NS_MINALLOC(x)  (x).ns_minalloc

/**
 * @def NSTYPE
 * @brief Mask of segment type bits. Value: 0x0007.
 */
#define NSTYPE          0x0007

#if (EXE386 == 0)

/**
 * @def NSCODE
 * @brief Code segment. Value: 0x0000.
 */
#define NSCODE          0x0000

/**
 * @def NSDATA
 * @brief Data segment. Value: 0x0001.
 */
#define NSDATA          0x0001

/**
 * @def NSITER
 * @brief Iterated data. Value: 0x0008.
 */
#define NSITER          0x0008

/**
 * @def NSMOVE
 * @brief Movable. Value: 0x0010.
 */
#define NSMOVE          0x0010

/**
 * @def NSSHARED
 * @brief Shared. Value: 0x0020.
 */
#define NSSHARED        0x0020

/**
 * @def NSPRELOAD
 * @brief Preload. Value: 0x0040.
 */
#define NSPRELOAD       0x0040

/**
 * @def NSEXRD
 * @brief Execute/read only. Value: 0x0080.
 */
#define NSEXRD          0x0080

/**
 * @def NSRELOC
 * @brief Has relocations. Value: 0x0100.
 */
#define NSRELOC         0x0100

/**
 * @def NSCONFORM
 * @brief Conforming. Value: 0x0200.
 */
#define NSCONFORM       0x0200

/**
 * @def NSEXPDOWN
 * @brief Expand down. Value: 0x0200.
 */
#define NSEXPDOWN       0x0200

/**
 * @def NSDPL
 * @brief Descriptor privilege level. Value: 0x0C00.
 */
#define NSDPL           0x0C00

/**
 * @def SHIFTDPL
 * @brief DPL shift count. Value: 10.
 */
#define SHIFTDPL        10

/**
 * @def NSDISCARD
 * @brief Discardable. Value: 0x1000.
 */
#define NSDISCARD       0x1000

/**
 * @def NS32BIT
 * @brief 32-bit segment. Value: 0x2000.
 */
#define NS32BIT         0x2000

/**
 * @def NSHUGE
 * @brief Huge segment. Value: 0x4000.
 */
#define NSHUGE          0x4000

/**
 * @def NSGDT
 * @brief Uses GDT. Value: 0x8000.
 */
#define NSGDT           0x8000

/**
 * @def NSPURE
 * @brief Pure (alias for shared).
 */
#define NSPURE          NSSHARED

/**
 * @def NSALIGN
 * @brief Default alignment shift. Value: 9.
 */
#define NSALIGN         9

/**
 * @def NSLOADED
 * @brief Loaded flag. Value: 0x0004.
 */
#define NSLOADED        0x0004

#endif

/* ==================================================================
 * Entry Table flags
 *
 * Values for the FLAGS byte of an Entry Table entry. Bit 0 marks
 * the entry as exported; bit 1 marks it as a reference to the
 * module's global data area.
 * ================================================================== */

/**
 * @def NEENT_EXPORTED
 * @brief Entry is exported. Value: 0x01.
 */
#define NEENT_EXPORTED      0x01

/**
 * @def NEENT_GLOBALDATA
 * @brief Entry uses the global data area. Value: 0x02.
 */
#define NEENT_GLOBALDATA    0x02

#pragma pack(push,1)

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

/* ==================================================================
 * Relocation entry accessors
 * ================================================================== */

/**
 * @def NR_STYPE(x)
 * @brief Accessor for the nr_stype field.
 * @param x Relocation table entry.
 */
#define NR_STYPE(x)     (x).nr_stype

/**
 * @def NR_FLAGS(x)
 * @brief Accessor for the nr_flags field.
 * @param x Relocation table entry.
 */
#define NR_FLAGS(x)     (x).nr_flags

/**
 * @def NR_SOFF(x)
 * @brief Accessor for the nr_soff field.
 * @param x Relocation table entry.
 */
#define NR_SOFF(x)      (x).nr_soff

/**
 * @def NR_SEGNO(x)
 * @brief Accessor for the internal segment reference.
 * @param x Relocation table entry.
 */
#define NR_SEGNO(x)     (x).nr_union.nr_intref.nr_segno

/**
 * @def NR_RES(x)
 * @brief Accessor for the reserved byte.
 * @param x Relocation table entry.
 */
#define NR_RES(x)       (x).nr_union.nr_intref.nr_res

/**
 * @def NR_ENTRY(x)
 * @brief Accessor for the entry index.
 * @param x Relocation table entry.
 */
#define NR_ENTRY(x)     (x).nr_union.nr_intref.nr_entry

/**
 * @def NR_MOD(x)
 * @brief Accessor for the module reference index.
 * @param x Relocation table entry.
 */
#define NR_MOD(x)       (x).nr_union.nr_import.nr_mod

/**
 * @def NR_PROC(x)
 * @brief Accessor for the procedure ordinal or name offset.
 * @param x Relocation table entry.
 */
#define NR_PROC(x)      (x).nr_union.nr_import.nr_proc

/**
 * @def NR_OSTYPE(x)
 * @brief Accessor for the OS fixup type.
 * @param x Relocation table entry.
 */
#define NR_OSTYPE(x)    (x).nr_union.nr_osfix.nr_ostype

/**
 * @def NR_OSRES(x)
 * @brief Accessor for the reserved OS fixup word.
 * @param x Relocation table entry.
 */
#define NR_OSRES(x)     (x).nr_union.nr_osfix.nr_osres

/* ==================================================================
 * Relocation source types
 * ================================================================== */

/**
 * @def NRSTYP
 * @brief Mask of source type. Value: 0x0f.
 */
#define NRSTYP      0x0f

/**
 * @def NRSBYT
 * @brief Low byte. Value: 0x00.
 */
#define NRSBYT      0x00

/**
 * @def NRSSEG
 * @brief Segment. Value: 0x02.
 */
#define NRSSEG      0x02

/**
 * @def NRSPTR
 * @brief Far pointer. Value: 0x03.
 */
#define NRSPTR      0x03

/**
 * @def NRSOFF
 * @brief Offset. Value: 0x05.
 */
#define NRSOFF      0x05

/**
 * @def NRPTR48
 * @brief 48-bit pointer. Value: 0x06.
 */
#define NRPTR48     0x06

/**
 * @def NROFF32
 * @brief 32-bit offset. Value: 0x07.
 */
#define NROFF32     0x07

/**
 * @def NRSOFF32
 * @brief 32-bit self-relative offset. Value: 0x08.
 */
#define NRSOFF32    0x08

/**
 * @def NRADD
 * @brief Additive fixup. Value: 0x04.
 */
#define NRADD       0x04

/* ==================================================================
 * Relocation types
 * ================================================================== */

/**
 * @def NRRTYP
 * @brief Mask of relocation type. Value: 0x03.
 */
#define NRRTYP      0x03

/**
 * @def NRRINT
 * @brief Internal reference. Value: 0x00.
 */
#define NRRINT      0x00

/**
 * @def NRRORD
 * @brief Import by ordinal. Value: 0x01.
 */
#define NRRORD      0x01

/**
 * @def NRRNAM
 * @brief Import by name. Value: 0x02.
 */
#define NRRNAM      0x02

/**
 * @def NRROSF
 * @brief OS fixup. Value: 0x03.
 */
#define NRROSF      0x03

/**
 * @def NRICHAIN
 * @brief Chain bit. Value: 0x08.
 */
#define NRICHAIN    0x08

#if (EXE386 == 0)

/* ==================================================================
 * Resource accessors
 * ================================================================== */

/**
 * @def RS_LEN(x)
 * @brief Accessor for the rs_len field (length of the string).
 * @param x Resource string.
 */
#define RS_LEN(x)       (x).rs_len

/**
 * @def RS_STRING(x)
 * @brief Accessor for the rs_string field (first character).
 * @param x Resource string.
 */
#define RS_STRING(x)    (x).rs_string

/**
 * @def RS_ALIGN(x)
 * @brief Accessor for the rs_align field (alignment shift count).
 * @param x Resource table header.
 */
#define RS_ALIGN(x)     (x).rs_align

/**
 * @def RT_ID(x)
 * @brief Accessor for the rt_id field (resource type ID).
 * @param x Resource type info.
 */
#define RT_ID(x)        (x).rt_id

/**
 * @def RT_NRES(x)
 * @brief Accessor for the rt_nres field (number of resources).
 * @param x Resource type info.
 */
#define RT_NRES(x)      (x).rt_nres

/**
 * @def RT_PROC(x)
 * @brief Accessor for the rt_proc field (reserved).
 * @param x Resource type info.
 */
#define RT_PROC(x)      (x).rt_proc

/**
 * @def RN_OFFSET(x)
 * @brief Accessor for the rn_offset field (offset within resource
 *        data).
 * @param x Resource name info.
 */
#define RN_OFFSET(x)    (x).rn_offset

/**
 * @def RN_LENGTH(x)
 * @brief Accessor for the rn_length field (length of resource).
 * @param x Resource name info.
 */
#define RN_LENGTH(x)    (x).rn_length

/**
 * @def RN_FLAGS(x)
 * @brief Accessor for the rn_flags field (flags).
 * @param x Resource name info.
 */
#define RN_FLAGS(x)     (x).rn_flags

/**
 * @def RN_ID(x)
 * @brief Accessor for the rn_id field (resource ID).
 * @param x Resource name info.
 */
#define RN_ID(x)        (x).rn_id

/**
 * @def RN_HANDLE(x)
 * @brief Accessor for the rn_handle field (handle).
 * @param x Resource name info.
 */
#define RN_HANDLE(x)    (x).rn_handle

/**
 * @def RN_USAGE(x)
 * @brief Accessor for the rn_usage field (usage flags).
 * @param x Resource name info.
 */
#define RN_USAGE(x)     (x).rn_usage

/**
 * @def RSORDID
 * @brief Resource ordinal ID flag. Value: 0x8000.
 */
#define RSORDID     0x8000

/**
 * @def RNMOVE
 * @brief Movable resource. Value: 0x0010.
 */
#define RNMOVE      0x0010

/**
 * @def RNPURE
 * @brief Pure resource. Value: 0x0020.
 */
#define RNPURE      0x0020

/**
 * @def RNPRELOAD
 * @brief Preload resource. Value: 0x0040.
 */
#define RNPRELOAD   0x0040

/**
 * @def RNDISCARD
 * @brief Discard priority mask. Value: 0xF000.
 */
#define RNDISCARD   0xF000

/**
 * @def NE_FFLAGS_LIBMODULE
 * @brief Library module flag. Value: 0x8000.
 */
#define NE_FFLAGS_LIBMODULE 0x8000

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

/*! @brief Handle to an NE export enumeration cursor.
 *
 *  Opaque. Created by NeExportFindFirst, NeExportFindByName or
 *  NeExportFindByOrdinal; released by NeExportFindClose. The internal
 *  representation is private to newexe.c.
 *
 *  @see NeExportFindFirst
 *  @see NeExportFindClose
 */
typedef HANDLE HNEENUM;

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

/*! @brief Return the module name of this NE image.
 *
 *  The module name is the first entry of the Resident Name Table,
 *  stored as a length-prefixed string without a terminating NUL.
 *  This function copies it into @p pszName as a NUL-terminated
 *  string.
 *
 *  @param[in]  hNe      Handle. Not NULLHANDLE.
 *  @param[out] pszName  Receives a NUL-terminated name. Not NULL.
 *  @param[in]  cbName   Size of @p pszName including the NUL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle or NULL pointer.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 *  @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY NeQuerySelfModuleName(HNE hNe, PSZ pszName, ULONG cbName);

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

/* ------------------------------------------------------------------ */
/* NE export enumeration API                                           */
/* ------------------------------------------------------------------ */

/*! @brief Open a cursor on the first export of an NE module.
 *
 *  The cursor walks the exports in increasing ordinal order. Bundles
 *  that contain no entries, and entries whose EXPORTED bit is clear,
 *  are skipped silently. Exports without a name are still reported;
 *  use NeExportIsNamed to tell them apart.
 *
 *  @param[in]  hNe       Handle. Not NULLHANDLE.
 *  @param[out] phEnum    Receives the cursor. Not NULL. Set to
 *                        NULLHANDLE on error or when the module has
 *                        no exports.
 *  @param[out] pulCount  Optional. May be NULL. On success receives
 *                        the total number of exports.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hNe or @p phEnum is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_NO_MORE_ITEMS      Module has no exports.
 *                                   *phEnum = NULLHANDLE.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *  @retval ERROR_READ_FAULT         Read error.
 *
 *  @see NeExportFindNext
 *  @see NeExportFindClose
 */
APIRET APIENTRY NeExportFindFirst(HNE hNe, HNEENUM *phEnum,
                                  PULONG pulCount);

/*! @brief Advance an export cursor to the next export.
 *
 *  @param[in] hEnum  Cursor from NeExportFindFirst or one of the
 *                    find-by functions. Not NULLHANDLE.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_NO_MORE_ITEMS      No more exports.
 *  @retval ERROR_READ_FAULT         Read error.
 *
 *  @see NeExportFindFirst
 *  @see NeExportFindClose
 */
APIRET APIENTRY NeExportFindNext(HNEENUM hEnum);

/*! @brief Close an export cursor.
 *
 *  Passing NULLHANDLE is a no-op.
 *
 *  @param[in] hEnum  Cursor. NULLHANDLE is accepted.
 *
 *  @return APIRET
 *  @retval NO_ERROR                Success. Also for NULLHANDLE.
 *  @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 *
 *  @see NeExportFindFirst
 */
APIRET APIENTRY NeExportFindClose(HNEENUM hEnum);

/*! @brief Position a new cursor on an export by name.
 *
 *  Comparison is case-sensitive. If several entries share the same
 *  name (possible with alias entries), the one with the lowest
 *  ordinal is chosen.
 *
 *  @param[in]  hNe     Handle. Not NULLHANDLE.
 *  @param[in]  pszName Name to find. Not NULL.
 *  @param[out] phEnum  Receives the cursor. Not NULL. Set to
 *                      NULLHANDLE on error.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     Name not present.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *  @retval ERROR_READ_FAULT         Read error.
 *
 *  @see NeExportFindFirst
 *  @see NeExportFindClose
 */
APIRET APIENTRY NeExportFindByName(HNE hNe, PCSZ pszName,
                                   HNEENUM *phEnum);

/*! @brief Position a new cursor on an export by ordinal.
 *
 *  @param[in]  hNe        Handle. Not NULLHANDLE.
 *  @param[in]  usOrdinal  Ordinal to find (1-based).
 *  @param[out] phEnum     Receives the cursor. Not NULL. Set to
 *                         NULLHANDLE on error.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hNe or @p phEnum is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     Ordinal is not exported.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *  @retval ERROR_READ_FAULT         Read error.
 *
 *  @see NeExportFindFirst
 *  @see NeExportFindClose
 */
APIRET APIENTRY NeExportFindByOrdinal(HNE hNe, USHORT usOrdinal,
                                      HNEENUM *phEnum);

/*! @brief Retrieve the name of the current export.
 *
 *  Size-query convention:
 *    - pszBuf == NULL, ulSize == 0: only *pulUsed (size including
 *      NUL) is written, no buffer touched.
 *    - ulSize large enough: value copied and NUL-terminated; *pulUsed
 *      is the length without NUL.
 *    - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *      required size including NUL.
 *
 *  For an anonymous export (see NeExportIsNamed) a synthetic name of
 *  the form "Ordinal<N>" is returned, so the caller always receives a
 *  usable string. The real name, when present, is taken from the
 *  Resident Name Table first and from the Non-Resident Name Table
 *  otherwise.
 *
 *  @param[in]  hEnum    Cursor. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hEnum is NULL, or @p pszBuf is
 *                                   NULL without size-query.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 *
 *  @see NeExportIsNamed
 */
APIRET APIENTRY NeExportGetName(HNEENUM hEnum,
                                PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/*! @brief Retrieve the ordinal of the current export.
 *
 *  @param[in]  hEnum       Cursor. Not NULLHANDLE.
 *  @param[out] pusOrdinal  Receives the ordinal (1-based). Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY NeExportGetOrdinal(HNEENUM hEnum, PUSHORT pusOrdinal);

/*! @brief Retrieve the raw Entry Table flags of the current export.
 *
 *  The returned value is the FLAGS byte of the Entry Table entry,
 *  without interpretation. Use NeExportIsGlobalData for the
 *  GLOBALDATA predicate.
 *
 *  @param[in]  hEnum     Cursor. Not NULLHANDLE.
 *  @param[out] pulFlags  Receives the flags. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY NeExportGetFlags(HNEENUM hEnum, PULONG pulFlags);

/*! @brief Query whether the current export has a real name.
 *
 *  @param[in]  hEnum    Cursor. Not NULLHANDLE.
 *  @param[out] pfNamed  Receives TRUE if a name is present in the
 *                       Resident or Non-Resident Name Table. Not
 *                       NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *
 *  @see NeExportGetName
 */
APIRET APIENTRY NeExportIsNamed(HNEENUM hEnum, PBOOL pfNamed);

/*! @brief Query whether the current export is a variable.
 *
 *  Returns TRUE when the GLOBALDATA bit (NEENT_GLOBALDATA) is set in
 *  the Entry Table flags.
 *
 *  @param[in]  hEnum         Cursor. Not NULLHANDLE.
 *  @param[out] pfGlobalData  Receives TRUE for a variable. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY NeExportIsGlobalData(HNEENUM hEnum, PBOOL pfGlobalData);

/*! @brief Query whether the current export is a forwarder.
 *
 *  Always returns FALSE for NE; present for symmetry with the LX
 *  reader.
 *
 *  @param[in]  hEnum        Cursor. Not NULLHANDLE.
 *  @param[out] pfForwarder  Receives FALSE. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY NeExportIsForwarder(HNEENUM hEnum, PBOOL pfForwarder);

/*! @brief Determine the calling convention of the current export.
 *
 *  Scans the first bytes of the function body for the first
 *  occurrence of one of the following instruction bytes:
 *
 *    - C3        ret        -> "_System"       (near, caller cleanup)
 *    - C2 xx xx  ret imm16  -> "_Pascal"       (callee cleanup)
 *    - CB        retf       -> "_System"       (far, caller cleanup)
 *    - CA xx xx  retf imm16 -> "_Far16 _Pascal" (far, callee cleanup)
 *
 *  If none of these bytes is found within the segment bounds, the
 *  default convention "_System" is returned. The choice is a
 *  heuristic: NE does not store the calling convention explicitly.
 *
 *  Size-query convention as for NeExportGetName.
 *
 *  @param[in]  hEnum    Cursor. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hEnum is NULL, or @p pszBuf is
 *                                   NULL without size-query.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 *  @retval ERROR_READ_FAULT         Segment body cannot be read.
 */
APIRET APIENTRY NeExportGetConvention(HNEENUM hEnum,
                                      PSZ pszBuf, ULONG ulSize,
                                      PULONG pulUsed);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __NEWEXE__ */
