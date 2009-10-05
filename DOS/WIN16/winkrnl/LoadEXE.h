/********************************************************************
	@(#)LoadEXE.h	2.5
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

********************************************************************/
 
#ifndef LoadEXE__h
#define LoadEXE__h



/* defines for the old exe header */
#define	MZ_SIGNATURE	0
#define	RELOC_TABLE		0x18	/* offset of pointer to relocation table */
#define	EXEHDR_SZ		0x40

/* The new exe header structure. Unless specified, all offsets are w.r.t.
 * the beginning of the new exe header
 */
#define	NE_SIGNATURE	0x0
#define	ENTRY_TBL		0x4		/* Offset of entry table */
#define	ENTRYTBL_SZ		0x6		/* Number of bytes in entry table */
#define	HEADER_FLAG		0xc		/* Flag word */
	/* defines for header flag word */
#define	HEADER_NOAUTODATA	0x0
#define	HEADER_SINGLEDATA	0x1
#define	HEADER_MULTIPLEDATA	0x2
#define	HEADER_REALMODE		0x4
#define	HEADER_PROTMODE		0x8
#define	HEADER_SELFLOAD		0x800	/* Self-loading executable */
#define	HEADER_INVALID		0x4000	/* Non conforming program */
#define	HEADER_LIBRARY		0x8000	/* Library module */
/* Back to new exe header */
#define	AUTO_DATASEG	0xe	/* Segment number of automatic data segment */
#define	INIT_HEAP	0x10	/* initial heap size */
#define	INIT_STACK	0x12	/* initial stack size */
#define	INIT_IP		0x14	/* initial IP */
#define	INIT_CS		0x16	/* initial CS */
#define	INIT_SP		0x18	/* initial SP */
#define	INIT_SS		0x1a	/* initial SS */
#define	SEGMENTTBL_SZ	0x1c	/* Number of entries in segment table */
#define	MODULEREFTBL_SZ	0x1e	/* Number of entries in module ref tbl */
#define	NONRESIDENTTBL_SZ	0x20	/* Number of byts in non-res name table */
#define	SEGMENT_TBL		0x22	/* Offset of segment table */
#define	RESOURCE_TBL	0x24	/* Offset of the resource table */
#define	RESIDENT_TBL	0x26	/* Offset of resident name table */
#define	MODULEREF_TBL	0x28	/* Offset of module reference table */
#define	IMPORTED_TBL	0x2a	/* Offset of imported names table */
#define	NONRESIDENT_TBL	0x2c	/* Offset of non resident name table w.r.t. to */
					/* BEGINNING of the FILE */
#define	MOVABLE_ENTRYPT	0x30	/* Number of movable entry points */
#define	SHIFT_CNT		0x32	/* Logical sector alignment */
#define	RESOURCESEG_CNT	0x34	/* Number of resource segments */
#define	TARGET_OS		0x36	/* target operating system */
	/* defines for the above */
#define	OS2_MASK		0x1
#define	WIN_MASK		0x2
/* Back to header */
#define	MORE_FLAGS		0x37	/* Additional program flags */
/* Defines for the above */
#define	WIN2_WIN3_PROT	0x1		/* Win 2.0 app that can run in win 3.0 */
#define	WIN2_PFONTS		0x2		/* Win 2.0 app supports proportional fonts */
#define	FAST_LOAD		0x3		/* Program has fast-load area */
/* Back to header */
#define	FAST_LOAD_AREA	0x38	/* Start of fast_load area */
#define	FAST_LOAD_SZ	0x3a	/* Fast load area size */
#define	WIN_VER			0x3e	/* Expected windows versio */
#define	NEWEXEHDR_SZ	0x40

/* Segment entry related */
#define	SEGMENT_ENTRYSZ	0x8		/* length of each segment entry in table */
#define	SEGENTRY_START	0x0		/* n byte logical sector offset */
#define	SEGENTRY_LENGTH	0x2		/* length of the segment in file 0 == 64k */
#define	SEGENTRY_FLAG	0x4
#define	SEGENTRY_ALLOCSZ	0x6		/* Minimum alloc size of segment in bytes*/
	/* defines for the flag word in the segment table entry */
#define	SEG_TYPEMASK	0x7		/* Segment type field */
#define	SEG_CODE		0x0		/* Code segment type */
#define	SEG_DATA		0x1		/* Data segment type */
#define	SEG_ITERATED	0x8		/* Segment data is iterated */
#define	SEG_MOVABLE		0x10	/* Segment is not fixed */
#define	SEG_PURE		0x20	/* Shareable segment */
#define	SEG_PRELOAD		0x40	/* NOT demand loaded */
#define	SEG_ERONLY		0x80	/* Read only / execute only */
#define	SEG_RELOCINFO	0x100	/* Set if segment has relocation records */
#define	SEG_DEBUGINFO	0x200	/* set if segment has debug info */
#define	SEG_DPL			0xc00
#define	SEG_DISCARD		0xf000	/* discard priority */

#define DLL_SEL(mi,w)	((mi->wSelBase + w - 1) << 3) | 0x7

/* Back to entry structure */

/* Resource related .. */

/* Module reference table is an array of WORDS which point to the offset of
 * module name strings in import names table
 */

/* Entry table related
 */
#define	ENTRYF_EXPORTED		0x1	/* exported entry */
#define	ENTRYF_SHAREDDATA	0x2	/* uses shared data segments */
#define	FIXEDSEG_ENTRYSZ	0x3	/* entry size for fixed segments */
#define	MOVABLESEG_ENTRYSZ	0x6	/* entry size for movable segments */

/* Per segment relocation item related */
#define	RELOCITEM_SZ		0x8
#define	RELOCATION_SRC		0x0	
/* defines for relocation source */
#define	RELOCSRC_LOBYTE		0x0
#define	RELOCSRC_SEGMENT	0x2
#define	RELOCSRC_FARADDR	0x3
#define	RELOCSRC_OFFSET		0x5
#define	RELOCSRC_MASK		0x7
#define	RELOCATION_FLAG		0x1
/* defines for RELOCATION_FLAG */
#define	RELOCF_INTERNALREF	0x0
#define	RELOCF_IMPORTORDINAL	0x1
#define	RELOCF_IMPORTNAME	0x2
#define	RELOCF_OSFIXUP		0x3
#define	RELOCF_TARGETMASK	0x3
#define	RELOCF_ADDITIVE		0x4
#define	RELOCATION_OFFSET	0x2
/* if INTERNAL_REF */
#define	RELOCIREF_SEGNUM	0x4
#define	RELOCIREF_FREE		0x5 /* There is a byte here with a value of 0 */
#define	RELOCIREF_INDEX		0x6
/* else if IMPORTNAME */
#define	RELOCINAME_INDEX	0x4
#define	RELOCINAME_OFFSET	0x6
/* else if IMPORTORDINAL */
#define	RELOCIORD_INDEX		0x4
#define	RELOCIORD_PROCNUM	0x6
/* else if OSFIXUP */
#define	RELOCOSFIX_FIXUPNUM	0x4

/*--------------------------------------------------------------------------*/
/* Some structure definitions */

/* structure to hold stuff from segment table */
typedef struct {
	WORD	sec_offset;		/* logical sector offset */
	WORD	length;
	WORD	flag;
	WORD	alloc_sz;
} SEGENTRY;

typedef SEGENTRY *LPSEGENTRY;

/* an address structure */
struct addr {
	WORD	offset;
	WORD	segment;
};

/* Miscellaneous */
#define	UNMUNGE_SIGNATURE	0x55aa

/*--------------------------------------------------------------------------*/
/* extern declarations */
extern	char *entry_tbl;
extern	char *header_image;
extern	char *nehdr;
extern	DWORD ne_offset;

/*--------------------------------------------------------------------------*/
/* Error numbers */
#define	LXERR_NOMZHDR		1
#define	LXERR_NONEHDR		2
#define	LXERR_NOTNEWEXE		3
#define	LXERR_FILEREAD		4
#define	LXERR_NOMEM			5
#define	LXERR_INVRELOCSRC	6
#define	LXERR_INVRELOCTRG	7
#define	LXERR_NOEXEFILE		8
#define	LXERR_INVRELOCCOMB	9
#define	LXERR_DLLNOTFOUND	10
#define	LXERR_ENTRYNOTFOUND	11
#define	LXERR_MODULENOTFOUND	12


/*--------------------------------------------------------------------------*/
#endif /* LoadEXE__h */
