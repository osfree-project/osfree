/*
 *  newexe.h    New Executable (NE) format.
 */

#ifndef __NEWEXE__
#define __NEWEXE__

#ifdef __cplusplus
extern "C" {
#endif

#define EMAGIC      0x5A4D
#define ENEWEXE     sizeof(struct exe_hdr)

#define ENEWHDR     0x003C
#define ERESWDS     0x0010
#define ERES1WDS    0x0004
#define ERES2WDS    0x000A
#define ECP         0x0004
#define ECBLP       0x0002
#define EMINALLOC   0x000A

#define E_MAGIC(x)      (x).e_magic
#define E_CBLP(x)       (x).e_cblp
#define E_CP(x)         (x).e_cp
#define E_CRLC(x)       (x).e_crlc
#define E_CPARHDR(x)    (x).e_cparhdr
#define E_MINALLOC(x)   (x).e_minalloc
#define E_MAXALLOC(x)   (x).e_maxalloc
#define E_SS(x)         (x).e_ss
#define E_SP(x)         (x).e_sp
#define E_CSUM(x)       (x).e_csum
#define E_IP(x)         (x).e_ip
#define E_CS(x)         (x).e_cs
#define E_LFARLC(x)     (x).e_lfarlc
#define E_OVNO(x)       (x).e_ovno
#define E_RES(x)        (x).e_res
#define E_OEMID(x)      (x).e_oemid
#define E_OEMINFO(x)    (x).e_oeminfo
#define E_RES2(x)       (x).e_res2
#define E_LFANEW(x)     (x).e_lfanew

#define NEMAGIC         0x454E
#define NERESBYTES      8
#define NECRC           8

#define ne_pWinFileStruc (ne_magic + 0x0a)
#define ne_cbModName     0
#define ne_pWinModName   8

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

#define NE_USAGE(x)     (WORD)*((WORD *)(x)+1)
#define NE_PNEXTEXE(x)  (WORD)(x).ne_cbenttab
#define NE_ONEWEXE(x)   (WORD)(x).ne_crc
#define NE_PFILEINFO(x) (WORD)((DWORD)(x).ne_crc >> 16)

#define NE_UNKNOWN      0x0
#define NE_OS2          0x1
#define NE_WINDOWS      0x2
#define NE_DOS4         0x3
#define NE_DEV386       0x4

/* Flag word values */ 
#define NENOTP          0x8000
#define NENOTMPSAFE     0x4000
#define NEIERR          0x2000
#define NEBOUND         0x0800
#define NEAPPTYP        0x0700
#define NENOTWINCOMPAT  0x0100
#define NEWINCOMPAT     0x0200
#define NEWINAPI        0x0300
#define NEFLTP          0x0080
#define NEI386          0x0040
#define NEI286          0x0020
#define NEI086          0x0010
#define NEPROT          0x0008
#define NEPPLI          0x0004
#define NEINST          0x0002
#define NESOLO          0x0001

#define NELONGNAMES     0x01
#define NEWINISPROT     0x02
#define NEWINGETPROPFON 0x04
#define NEWLOAPPL       0x80

#define NS_SECTOR(x)    (x).ns_sector
#define NS_CBSEG(x)     (x).ns_cbseg
#define NS_FLAGS(x)     (x).ns_flags
#define NS_MINALLOC(x)  (x).ns_minalloc

#define NSTYPE          0x0007

#if (EXE386 == 0)
#define NSCODE          0x0000
#define NSDATA          0x0001
#define NSITER          0x0008
#define NSMOVE          0x0010
#define NSSHARED        0x0020
#define NSPRELOAD       0x0040
#define NSEXRD          0x0080

#define NSRELOC         0x0100
#define NSCONFORM       0x0200
#define NSEXPDOWN       0x0200
#define NSDPL           0x0C00
#define SHIFTDPL        10
#define NSDISCARD       0x1000
#define NS32BIT         0x2000
#define NSHUGE          0x4000

#define NSGDT           0x8000
#define NSPURE          NSSHARED
#define NSALIGN         9
#define NSLOADED        0x0004
#endif

#pragma pack(push,1)


struct exe_hdr {
    WORD	e_magic;			/* 0x4d, 0x5a. This is the "magic number" of an EXE file. The first byte of the file is 0x4d and the second is 0x5a. */
    WORD	e_cblp;				/* The number of bytes in the last block of the program that are actually used. If this value is zero, that means the entire last block is used (i.e. the effective value is 512). */
    WORD	e_cp;				/* Number of blocks in the file that are part of the EXE file. If [02-03] is non-zero, only that much of the last block is used. */
    WORD	e_crlc;				/* Number of relocation entries stored after the header. May be zero. */
    WORD	e_cparhdr;			/* Number of paragraphs in the header. The program's data begins just after the header, and this field can be used to calculate the appropriate file offset. The header includes the relocation entries. Note that some OSs and/or programs may fail if the header is not a multiple of 512 bytes. */
    WORD	e_minalloc;			/* Number of paragraphs of additional memory that the program will need. This is the equivalent of the BSS size in a Unix program. The program can't be loaded if there isn't at least this much memory available to it. */
    WORD	e_maxalloc;			/* Maximum number of paragraphs of additional memory. Normally, the OS reserves all the remaining conventional memory for your program, but you can limit it with this field. */
    WORD	e_ss;				/* Relative value of the stack segment. This value is added to the segment the program was loaded at, and the result is used to initialize the SS register. */
    WORD	e_sp;				/* Initial value of the SP register. */
    WORD	e_csum;				/* Word checksum. If set properly, the 16-bit sum of all words in the file should be zero. Usually, this isn't filled in. */
    WORD	e_ip;				/* Initial value of the IP register. */
    WORD	e_cs;				/* Initial value of the CS register, relative to the segment the program was loaded at. */
    WORD	e_lfarlc;			/* Offset of the first relocation item in the file. */
    WORD	e_ovno;				/* Overlay number. Normally zero, meaning that it's the main program. */
    WORD	e_res[ERES1WDS];
    WORD	e_oemid;
    WORD	e_oeminfo;
    WORD	e_res2[ERES2WDS];
    DWORD	e_lfanew;
};

/* In-disk and In-memory module structure. See 'Windows Internals' p. 219 */

struct new_exe {
    WORD  ne_magic;					/* Signature word EMAGIC */
	union {
		struct {
			BYTE   ne_ver;			/* Version number of the linker */
			BYTE   ne_rev;			/* Revision number of the linker */
		};
		WORD  count;				/* Usage count (ne_ver/ne_rev on disk) */
	};
    WORD  ne_enttab;				/* Entry Table file offset, relative to the beginning of
									   the segmented EXE header */
	union {
		WORD  ne_cbenttab;			/* Number of bytes in the entry table */
		WORD  next;					/* Selector to next module */
	};
	union {
		DWORD            ne_crc;	/* 32-bit CRC of entire contents of file.
									   These words are taken as 00 during the calculation */
		struct {
			WORD	dgroup_entry;	/* Near ptr to segment entry for DGROUP */
			WORD	fileinfo;		/* Near ptr to file info (OFSTRUCT)*/
		};
	};
    WORD  ne_flags;					/* Flag word */
    WORD  ne_autodata;				/* Segment number of automatic data segment.
									   This value is set to zero if SINGLEDATA and
									   MULTIPLEDATA flag bits are clear, NOAUTODATA is
									   indicated in the flags word.

									   A Segment number is an index into the module's segment
									   table. The first entry in the segment table is segment
									   number 1 */
    WORD  ne_heap;					/* Initial size, in bytes, of dynamic heap added to the
									   data segment. This value is zero if no initial local
									   heap is allocated */
    WORD  ne_stack;					/* Initial size, in bytes, of stack added to the data
									   segment. This value is zero to indicate no initial
									   stack allocation, or when SS is not equal to DS */
    DWORD            ne_csip;		/* Segment number:offset of CS:IP */
	DWORD            ne_sssp;		/* Segment number:offset of SS:SP.
									   If SS equals the automatic data segment and SP equals
									   zero, the stack pointer is set to the top of the
									   automatic data segment just below the additional heap
									   area.

									   +--------------------------+
									   | additional dynamic heap  |
									   +--------------------------+ <- SP
									   |    additional stack      |
									   +--------------------------+
									   | loaded auto data segment |
									   +--------------------------+ <- DS, SS */
    WORD  ne_cseg;					/* Number of entries in the Segment Table */
    WORD  ne_cmod;					/* Number of entries in the Module Reference Table */
    WORD  ne_cbnrestab;				/* Number of bytes in the Non-Resident Name Table */
    WORD  ne_segtab;				/* Segment Table file offset, relative to the beginning
									   of the segmented EXE header */
    WORD  ne_rsrctab;				/* Resource Table file offset, relative to the beginning
									   of the segmented EXE header */
	WORD  ne_restab;				/* Resident Name Table file offset, relative to the
									   beginning of the segmented EXE header */
    WORD  ne_modtab;				/* Module Reference Table file offset, relative to the
									   beginning of the segmented EXE header */
    WORD  ne_imptab;				/* Imported Names Table file offset, relative to the
									   beginning of the segmented EXE header */
    DWORD	ne_nrestab;				/* Non-Resident Name Table offset, relative to the
									   beginning of the file */
    WORD	ne_cmovent;				/* Number of movable entries in the Entry Table */
    WORD	ne_align;				/* Logical sector alignment shift count, log(base 2) of
									   the segment sector size (default 9) */
    WORD	ne_cres;				/* Number of resource entries */
    BYTE	ne_exetyp;				/* Executable type, used by loader.
									   02h = WINDOWS */
    BYTE	ne_flagsothers;			/* Operating system flags */
    char	ne_res[NERESBYTES];		/* Reserved */ 
};

// On-disk segment entry
struct new_seg {
    WORD  ns_sector;				/* Logical-sector offset (n byte) to the contents of the segment
									   data, relative to the beginning of the file. Zero means no
									   file data */
    WORD  ns_cbseg;					/* Length of the segment in the file, in bytes. Zero means 64K */
    WORD  ns_flags;					/* Flag word */
    WORD  ns_minalloc;				/* Minimum allocation size of the segment, in bytes. Total size
									   of the segment. Zero means 64K */
};

// In-memory segment entry
struct new_seg1 {
    WORD  ns1_sector;				/* Logical-sector offset (n byte) to the contents of the segment
									   data, relative to the beginning of the file. Zero means no
									   file data */
    WORD  ns1_cbseg;				/* Length of the segment in the file, in bytes. Zero means 64K */
    WORD  ns1_flags;				/* Flag word */
    WORD  ns1_minalloc;				/* Minimum allocation size of the segment, in bytes. Total size
									   of the segment. Zero means 64K */
    WORD  ns1_handle;				/* Selector or handle (selector - 1) of segment in memory */
};

struct new_segdata {
    union {
        struct {
            WORD      ns_niter;
            WORD      ns_nbytes;
            char      ns_iterdata;
        } ns_iter;
        struct {
            char      ns_data;
        } ns_noniter;
    } ns_union;
};

struct new_rlcinfo {
    WORD  nr_nreloc;
};

struct new_rlc {
    char            nr_stype;
    char            nr_flags;
    WORD  nr_soff;
    union {
        struct {
            char            nr_segno;
            char            nr_res;
            WORD  nr_entry;
          } nr_intref;
        struct {
            WORD  nr_mod;
            WORD  nr_proc;
          } nr_import;
        struct {
            WORD  nr_ostype;
            WORD  nr_osres;
          } nr_osfix;
      } nr_union;
};

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

#define NRSTYP      0x0f
#define NRSBYT      0x00
#define NRSSEG      0x02
#define NRSPTR      0x03
#define NRSOFF      0x05
#define NRPTR48     0x06
#define NROFF32     0x07
#define NRSOFF32    0x08

#define NRADD       0x04
#define NRRTYP      0x03
#define NRRINT      0x00
#define NRRORD      0x01
#define NRRNAM      0x02
#define NRROSF      0x03
#define NRICHAIN    0x08

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

#define RSORDID     0x8000

#define RNMOVE      0x0010
#define RNPURE      0x0020
#define RNPRELOAD   0x0040
#define RNDISCARD   0xF000

#define NE_FFLAGS_LIBMODULE 0x8000

struct rsrc_string {
    char    rs_len;
    char    rs_string[1];
};

struct rsrc_typeinfo {
    WORD  rt_id;
    WORD  rt_nres;
    DWORD            rt_proc;
};

struct rsrc_nameinfo {
    WORD  rn_offset;
    WORD  rn_length;
    WORD  rn_flags;
    WORD  rn_id;
    WORD  rn_handle;
    WORD  rn_usage;
};

struct new_rsrc {
    WORD          rs_align;
    struct rsrc_typeinfo    rs_typeinfo;
};

#endif

#pragma pack(pop)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
