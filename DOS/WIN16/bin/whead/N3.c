/*----------------------------------------------------------------------------- 
 *
 *----------------------------------------------------------------------------*/

#include	<stdio.h>
#include	<malloc.h>

#include	"whead.h"
#include	"N3.h"

#define		BUFF_SZ		128 * 1024	/* transit buffer size */

/*--------------------------------------------------------------------------*/
/* Global variables */
BYTE *N3hdr;		/* current N3 header loaded here */
WORD N3offset;		/* offset into the segment of the current N3 header */
BYTE *newsegment_tbl;
BYTE *segment_tbl;
char *inbuff;		/* To reduce number of parameters being passed around */
BYTE *svbuff;
BYTE *relbuff;

/*--------------------------------------------------------------------------*/
/* Macros .. */
#define	ERROR_IN_MUNGE()	\
		{	\
		free(inbuff);	\
		free(svbuff);	\
		free(relbuff);	\
		free(newsegment_tbl);	\
		return(-1);		\
		}
#define	GWN3(x)			get_word(N3hdr + x)
#define	GDWN3(x)		get_dword(N3hdr + x)

/*--------------------------------------------------------------------------*/
/* Convert bytes to number of logical sectors */
WORD
get_sectorcnt(bytes)
DWORD bytes;
{
	int s =  1 << GWN(SHIFT_CNT);	/* will making this static be useful ? */

	if (bytes % s)
		return((bytes / s) + 1);
	else
		return(bytes / s);
}

/* Round offset to the nearest logical sector */
DWORD
round_offset(offset)
DWORD offset;
{
	int s =  1 << GWN(SHIFT_CNT);	/* will making this static be useful ? */

	if (offset % s)
		return(((offset/s) + 1) * s);
	else
		return(offset);
}

/* Unmunge a set of segments (CODE and DATA) */
void
unmunge_segs(infile, outfile, lastseg)
FILE *infile;
FILE *outfile;
int lastseg;			/* last segment in N3 header */
{
	BYTE *tmpbptr;
	WORD w1, w2, w3, w4;
	DWORD d1;
	WORD off;
	long ofile_offset;
	int i, j;
	char tmpc[16];

	tmpbptr = N3hdr + N3_FIRSTRELOC;
	/* WHAT TO DO IF segspecial != 0 ??????????????????
	 */
	for (i=GWN3(N3_SEGFIRST); i <= lastseg; i++, tmpbptr += N3RELOC_SEGMENTSZ) {
		int rel_cnt;
		BYTE *reloc_ptr;
		BYTE *newreloc_ptr;
		BYTE *segtbl_ptr;
		BYTE *newsegtbl_ptr;

		newsegtbl_ptr = newsegment_tbl + (i - 1) * SEGMENT_ENTRYSZ;
		segtbl_ptr = segment_tbl + (i - 1) * SEGMENT_ENTRYSZ;
		ofile_offset = ftell(outfile);
		ofile_offset = round_offset(ofile_offset);

		/* Update new segment table with new start offset */
		w1 = get_sectorcnt(ofile_offset);
		put_word(newsegtbl_ptr + SEGENTRY_START,  w1);
/*
fprintf(stderr, "Processing segment %04x\t", i);
fprintf(stderr, "Current ofile_offset: %06lx\n", ofile_offset);
*/

		/* Write segment i to output file */
		fseek(outfile, ofile_offset, 0);
		w1 = get_word(segtbl_ptr + SEGENTRY_START);
		d1 = w1 << GWN(SHIFT_CNT);
		fseek(infile, d1, 0);
		w1 = get_word(segtbl_ptr + SEGENTRY_LENGTH);
		if (w1 == 0)
			d1 = 64 * 1024;
		else
			d1 = w1;
		wh_fread(inbuff, 1, d1, infile);
		wh_fwrite(inbuff, 1, d1, outfile);

		w1 = get_word(tmpbptr + N3RELSEG_START) & N3RELSEG_RELOCPTR_MASK;
		/* If there are no relocs in this segment move on to next */
		if (w1 == 0)
			continue;
		reloc_ptr = N3hdr + w1;
		rel_cnt = get_word(reloc_ptr);
		reloc_ptr += 2;
		newreloc_ptr = relbuff;

		for (j = 0; j < rel_cnt; j++, reloc_ptr += N3_RELOCITEMSZ,
			 newreloc_ptr += RELOCITEM_SZ) {
			w2 = get_word(reloc_ptr + N3RELOCITEM_COMPLEX);
			w3 = get_word(reloc_ptr + N3RELOCITEM_SRCOFFSET);
			put_word(newreloc_ptr + RELOCATION_OFFSET, w3);

			if (w2 & N3RELITEMF_IMPMASK) {
				w4 = w2 & N3RELITEMF_VAL2MASK;
				/* For clarity, some code is duplicated below */
				/* Found the hard way that the index is 0 relative */
				if (w2 & N3RELITEMF_IMPNAMEMASK) {
					*(newreloc_ptr + RELOCATION_FLAG) = RELOCF_IMPORTNAME;
					put_word((newreloc_ptr + RELOCINAME_OFFSET),
					         (w4 & N3RELITEMF_VAL2MASK));
					w3 = ((w2 & N3RELITEMF_IMPIMODMASK)>>N3RELITEMF_IMPIMODSHIFT)+1;
					put_word((newreloc_ptr + RELOCINAME_INDEX), w3);
				} else {
					*(newreloc_ptr + RELOCATION_FLAG) = RELOCF_IMPORTORDINAL;
					put_word((newreloc_ptr + RELOCINAME_OFFSET),
					         (w4 & N3RELITEMF_VAL2MASK));
					w3 = ((w2 & N3RELITEMF_IMPIMODMASK)>>N3RELITEMF_IMPIMODSHIFT)+1;
					put_word((newreloc_ptr + RELOCIORD_INDEX), w3); 
				}

				switch (w2 & N3RELITEMF_RELTYPEMASK) {
					case	N3RELTYPE_OFFSET:
							*(newreloc_ptr + RELOCATION_SRC) = RELOCSRC_OFFSET;
							break;
					case	N3RELTYPE_BASE:
							*(newreloc_ptr + RELOCATION_SRC) = RELOCSRC_SEGMENT;
							break;
					case	N3RELTYPE_FARADDR:
							*(newreloc_ptr + RELOCATION_SRC) = RELOCSRC_FARADDR;
							break;
					case	N3RELTYPE_FIXED:
							/* Seems to come here with this type often */
							/* OR IS SOMETHING WRONG ?? */
							/* WHAT SOURCE TYPE ???? */
							*(newreloc_ptr + RELOCATION_SRC) = RELOCSRC_FARADDR;
							break;
					default:
							/* should not come here et all */
							fprintf(stderr, "unmunge:ERROR\n");
				} /* switch(w2 & N3RELITEMF_RELTYPEMASK)  */

			} else { /* (w2 & N3RELITEMF_IMPMASK) */
				w4 = w2 & N3RELITEMF_VAL1MASK;
				*(newreloc_ptr + RELOCIREF_SEGNUM) = 0xff;
				*(newreloc_ptr + RELOCIREF_FREE) = 0x0;
				*(newreloc_ptr + RELOCATION_FLAG) = RELOCF_INTERNALREF;
				/* Is the foll statement valid for fixed segment fixups?? */
				put_word((newreloc_ptr + RELOCIREF_INDEX), w4);

				switch (w2 & N3RELITEMF_RELTYPEMASK) {
					case	N3RELTYPE_OFFSET:
							*(newreloc_ptr + RELOCATION_SRC) = RELOCSRC_OFFSET;
							break;
					case	N3RELTYPE_BASE:
							*(newreloc_ptr + RELOCATION_SRC) = RELOCSRC_SEGMENT;
							break;
					case	N3RELTYPE_FARADDR:
							*(newreloc_ptr + RELOCATION_SRC) = RELOCSRC_FARADDR;
							break;
					case	N3RELTYPE_FIXED:
							*(newreloc_ptr + RELOCATION_SRC) = RELOCSRC_SEGMENT;
							*(newreloc_ptr + RELOCIREF_SEGNUM) = w4;
							put_word((newreloc_ptr + RELOCIREF_INDEX), 0);
							break;
					default:
							/* should not come here et all */
							fprintf(stderr, "unmunge:ERROR1\n");
				} /* switch(w2 & N3RELITEMF_RELTYPEMASK)  */
				if (w2 & N3RELITEMF_ADDITIVEMASK)
					*(newreloc_ptr + RELOCATION_FLAG) |= RELOCF_ADDITIVE;
			} /* (w2 & N3RELITEMF_IMPMASK) */
			
		} /* for every relocation item */

		/* Write relocation count and relocation entries to outfile */
		put_word(tmpc, rel_cnt);
		wh_fwrite(tmpc, 2, 1, outfile);
		wh_fwrite(relbuff, RELOCITEM_SZ, rel_cnt, outfile);

	}	/* for every segment */

}

/* hopefully would unmunge a file munged with N3 type of header */
int
unmunge(infile, outfile)
FILE *infile;
FILE *outfile;
{
	BYTE *tmpbptr;
	WORD w1, w2;
	DWORD d1, d2;
	int i, len;
	int first_dseg;
	int restbl_off;
	WORD dseg_relocblk;
	char tmpc[16];

	/* Copy the bytes upto the beginning of the first segment from infile
	 * to outfile. We'll update the segment table later from our copy.
	 */
	if ((inbuff = (char *)malloc(BUFF_SZ)) == (char *)NULL)
		wh_panic(WHERR_NOMEM);
	if ((svbuff = (BYTE *)malloc(BUFF_SZ)) == (BYTE *)NULL)
		wh_panic(WHERR_NOMEM);
	if ((relbuff = (BYTE *)malloc(BUFF_SZ)) == (BYTE *)NULL)
		wh_panic(WHERR_NOMEM);
	len = GWN(SEGMENTTBL_SZ) * SEGMENT_ENTRYSZ;
	if ((newsegment_tbl = (BYTE *)malloc(len)) == (BYTE *)NULL)
		wh_panic(WHERR_NOMEM);

    segment_tbl = (BYTE *)(header_image + GWN(SEGMENT_TBL));
	memcpy(newsegment_tbl, segment_tbl, len);
	w1 = get_word(segment_tbl);
	d1 = w1 << GWN(SHIFT_CNT);
	rewind(infile);
	rewind(outfile);
	wh_fread(inbuff, 1, d1, infile);
	wh_fwrite(inbuff, 1, d1, outfile);

	/* Read segment 1, check for signature 'A0', modify it and write to 
	 * outfile.
	 */
	w1 = get_word(segment_tbl + SEGENTRY_LENGTH);
	if (w1 == 0)
		d1 = 64 * 1024;
	else
		d1 = w1;
	wh_fread(svbuff, 1, d1, infile);
	if (get_word(svbuff) != 0x3041)
		ERROR_IN_MUNGE();
	put_word(svbuff, UNMUNGE_SIGNATURE);
	wh_fwrite(svbuff, 1, d1, outfile);

	/* get the N3 header in the segment */
	N3hdr = svbuff + get_word(svbuff + SEG1_CODERELOCBLK);
	dseg_relocblk = get_word(svbuff + SEG1_DATARELOCBLK);
	if (GWN3(N3_SIGNATURE) != 0x334e)
		wh_panic(WHERR_NON3HDR);

	/* Write reloc info for segment 1 */
	wh_fread(tmpc, 2, 1, infile);
	wh_fwrite(tmpc, 2, 1, outfile);
	w1 = get_word(tmpc);
	d1 = w1 * RELOCITEM_SZ;
	wh_fread(inbuff, 1, d1, infile);
	wh_fwrite(inbuff, 1, d1, outfile);

	/* Scan for the first data segment */
	len = GWN(SEGMENTTBL_SZ);
	for (first_dseg = 0, i = 0; i < len; i++) {
		w1 = get_word(segment_tbl + i * SEGMENT_ENTRYSZ + SEGENTRY_FLAG);
		if (w1 & SEG_DATA) {
			first_dseg = i + 1;
			break;
		}
	}
	if (first_dseg != 0)
		len = first_dseg - 1;

	/* Unmunge all code segments */
	unmunge_segs(infile, outfile, len);

	if (first_dseg != 0) {
		/* Unmunge all data segments */
		w1 = get_word(segment_tbl + (first_dseg -1) * SEGMENT_ENTRYSZ +
					  SEGENTRY_START);
		d1 = w1 << GWN(SHIFT_CNT);
		fseek(infile, d1, 0);
		w1 = get_word(segment_tbl + (first_dseg -1) * SEGMENT_ENTRYSZ +
					  SEGENTRY_LENGTH);
		if (w1 == 0)
			d1 = 64 * 1024;
		else
			d1 = w1;
		wh_fread(svbuff, 1, d1, infile);
		/* We don't need to write the whole segment I guess */
		/* NO,we should not be writing this segment */
		/* wh_fwrite(svbuff, 1, d1, outfile); */

		/* get the N3 header in the segment */
		N3hdr = svbuff + dseg_relocblk;
		if (GWN3(N3_SIGNATURE) != 0x334e)
			wh_panic(WHERR_NON3HDR);

		unmunge_segs(infile, outfile, GWN(SEGMENTTBL_SZ));
	}

	/* NEED TO HANDLE RESOURCE DATA */
	restbl_off = GWN(RESOURCE_TBL);
	len = GWN(RESIDENT_TBL) - restbl_off;
	if (len != 0) {
		int sector_sz;
		int diff;
		long ofile_offset;

		ofile_offset = ftell(outfile);

		fseek(outfile, restbl_off + ne_offset, 0);
		wh_fread(svbuff, 1, len, outfile);

		w1 = get_word(svbuff);	/* alignment shift count */
		sector_sz = 1 << w1;
		if (ofile_offset % sector_sz)
			ofile_offset = ((ofile_offset/sector_sz) + 1) * sector_sz;
		fseek(outfile, ofile_offset, 0);

		/* Assuming resource data goes on till the end of file */
		/* an ugly piece of code, after frustration with unmunging */
		/* Get the first resource data offset */
		w1 = get_word(svbuff + 0xa);
		diff = (ofile_offset / sector_sz) - w1;
		fseek(infile, 0, 2);
		len = ftell(infile) - (w1 * sector_sz);
		fseek(infile, w1 * sector_sz, 0);
		wh_fread(inbuff, 1, len, infile);
		wh_fwrite(inbuff, 1, len, outfile);

		tmpbptr = svbuff + 2;
		while (get_word(tmpbptr) != 0) {
			int type_count = get_word(tmpbptr + 2);

			tmpbptr += 8;
			for (; type_count > 0; --type_count, tmpbptr += 0xc) {
				w1 = get_word(tmpbptr);
				put_word(tmpbptr, w1 + diff);
			} /* for every element of a type */
		} /* while for each type */

		/* Write back the new resource table */
		fseek(outfile, restbl_off + ne_offset, 0);
		len = GWN(RESIDENT_TBL) - restbl_off;
		wh_fwrite(svbuff, 1, len, outfile);
	} /* if there is resource data at all */

	/* Write the new segment table */
	d1 = GWN(SEGMENT_TBL) + ne_offset;
	fseek(outfile, d1, 0);
	wh_fwrite(newsegment_tbl, SEGMENT_ENTRYSZ, GWN(SEGMENTTBL_SZ), outfile);

	free(inbuff);
	free(svbuff);
	free(relbuff);
	free(newsegment_tbl);
	return(0);
}




