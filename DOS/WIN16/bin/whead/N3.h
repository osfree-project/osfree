/*----------------------------------------------------------------------------- 
 *
 *----------------------------------------------------------------------------*/

/* Stuff related to N3 loader. This basically is a self loading application
 * The following information in the first segment is useful.
 *	WORD at offset 0x00: signature 'A0'
 * 	WORD at offset 0x02: offset of first N3 header which has relocation
 *	                     blocks for all code segments
 *	WORD at offset 0x20: offset of second N3 header which has relocation
 *	                     blocks for all data segments
 */

#define	SEG1_SIGNATURE	0x00
#define	SEG1_CODERELOCBLK	0x2
#define	SEG1_DATARELOCBLK	0x20

/* N3 header information */
#define	N3_SIGNATURE	0x0		/* Signature 'N3' */
#define	N3_SEGSPECIAL	0x2		/* WORD SegSpecial (???) */
#define	N3_SEGFIRST		0x4		/* WORD which has first segment for reloc */
/* This is then followed by an array of the following type.
 * Ignore element 0 of the array(not sure when it is used 
 */
#define	N3_FIRSTRELOC	0xa		/* first entry after special entry */

#define	N3RELOC_SEGMENTSZ	4	/* size of each relocation segment */
#define	N3RELSEG_START		0x0	/* A WORD which has 2 flags and start 
								 * address of reloc entries relative to
								 * start of N3 header
								 */
#define	N3RELSEG_RELOCPTR_MASK	0x3fff
#define	N3RELSEG_FLAG1			0x8000
#define	N3RELSEG_FLAG2			0x4000

#define	N3RELSEG_JUNK			0x2

/* The RELOCPTR points to a WORD count followed by relocation items */
#define	N3_RELOCITEMSZ			0x4	/* size of a relocation item */
#define	N3RELOCITEM_COMPLEX		0x0	/* A complex flag with lot of info */
/* Masks for extracting info from the above word */
#define	N3RELITEMF_IMPMASK		0x8000	/* Import relocation */
#define	N3RELITEMF_RELTYPEMASK	0x3000	/* Relocation source type */
/** defines for N3RELITEMF_RELTYPEMASK */
#define	N3RELTYPE_OFFSET		0x0		/* offset to moveable segment */
#define	N3RELTYPE_BASE			0x1000	/* base to moveable segment */
#define	N3RELTYPE_FARADDR		0x2000	/* far pointer to moveable segment */
#define	N3RELTYPE_FIXED			0x3000	/* base to fixed segment */

/* if N3RELITEMF_IMPMASK is set */
#define	N3RELITEMF_IMPNAMEMASK	0x4000	/* Import by name */
#define	N3RELITEMF_IMPIMODMASK	0x0e00	/* index to module ref table */
#define	N3RELITEMF_IMPIMODSHIFT	9		/* shift count to get the value */
#define	N3RELITEMF_VAL2MASK		0x01ff	/* value depending on context */
/* else */
#define	N3RELITEMF_ADDITIVEMASK	0x4000	/* additive reloc */
#define	N3RELITEMF_VAL1MASK		0x0fff	/* value depending on context */
/* end */

#define	N3RELOCITEM_SRCOFFSET	0x2	/* Source offset, common for both types */

