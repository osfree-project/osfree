/*

gbmpsg.c - PSEG support

Reads and writes PSEG files.
Input options: fixrec (attempt to handle corrupt record structure)
Output options: scalex=#, scaley=# (default is 1000)
Output options: double (will double scale factors in header).
Output options: pelx=#,pely=# (default is 2400).

This code was written using a subset of the full documentation.
The missing bits were filled in by reverse engineering a selection of files.
Guesses as to which records are important were made (BeginPageSegment etc.)
and these seem to correspond with other conversion programs choices.
Also image (and cell) widths and heights can apparently be any value, although
all PSEGs I have seen always use a multiple of 8 (and 32). This code will allow
any width that is a multiple of 8 (for easy coding), and any height at all.

When a PSEG is uploaded to VM, it must have its VM FILE block record structure
reestablished. Various VM EXECs exist to do this, such as FIXL3820, REFLOW, and
REST38PP, although in my experience, FIXL3820 always works, and REST38PP always
fails for large files.

PSEG writing updated to reflect information found in MO:DCA Reference on www.
Incrementing record numbers written.
ImageInputDescriptor has fixed 2400s in ConData1 part.

History:
--------
(Heiko Nitzsche)

22-Feb-2006: Move format description strings to gbmdesc.h
15-Aug-2008: Integrate new GBM types

*/

/*...sincludes:0:*/
#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmhelp.h"
#include "gbmdesc.h"
#include "gbmmem.h"

/*...vgbm\46\h:0:*/
/*...vgbmhelp\46\h:0:*/

#ifndef min
#define	min(a,b)	(((a)<(b))?(a):(b))
#endif
/*...e*/

static GBMFT psg_gbmft =
	{
        GBM_FMT_DESC_SHORT_PSG,
        GBM_FMT_DESC_LONG_PSG,
        GBM_FMT_DESC_EXT_PSG,
	GBM_FT_R1|
	GBM_FT_W1,
	};

#define	GBM_ERR_PSEG_NOT_5A	((GBM_ERR) 1600)
#define	GBM_ERR_PSEG_BAD_RECORD	((GBM_ERR) 1601)
#define	GBM_ERR_PSEG_SIZE	((GBM_ERR) 1602)
#define	GBM_ERR_PSEG_INV_WIDTH	((GBM_ERR) 1603)
#define	GBM_ERR_PSEG_INV_DEFCEL	((GBM_ERR) 1604)
#define	GBM_ERR_PSEG_INV_CEL	((GBM_ERR) 1605)
#define	GBM_ERR_PSEG_INV_CELPOS	((GBM_ERR) 1606)
#define	GBM_ERR_PSEG_UNEX_REC	((GBM_ERR) 1607)

typedef struct
	{
	long pos;
	int xcellsizedef, ycellsizedef;
	gbm_boolean fix_badrec;
	} PSEG_PRIV;

/*...smax:0:*/
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif
/*...e*/
/*...sascii \47\ ebcdic:0:*/
#ifdef NEVER
/*...sebcdic_to_ascii:0:*/
static gbm_u8 ebcdic_to_ascii[0x100] =
	{
	0x00,0x01,0x02,0x03,0xcf,0x09,0xd3,0x7f,
	0xd4,0xd5,0xc3,0x0b,0x0c,0x0d,0x0e,0x0f,
	0x10,0x11,0x12,0x13,0xc7,0xb4,0x08,0xc9,
	0x18,0x19,0xcc,0xcd,0x83,0x1d,0xd2,0x1f,
	0x81,0x82,0x1c,0x84,0x86,0x0a,0x17,0x1b,
	0x89,0x91,0x92,0x95,0xa2,0x05,0x06,0x07,
	0xe0,0xee,0x16,0xe5,0xd0,0x1e,0xea,0x04,
	0x8a,0xf6,0xc6,0xc2,0x14,0x15,0xc1,0x1a,
	0x20,0xa6,0xe1,0x80,0xeb,0x90,0x9f,0xe2,
	0xab,0x8b,0x9b,0x2e,0x3c,0x28,0x2b,0x7c,
	0x26,0xa9,0xaa,0x9c,0xdb,0xa5,0x99,0xe3,
	0xa8,0x9e,0x21,0x24,0x2a,0x29,0x3b,0x5e,
	0x2d,0x2f,0xdf,0xdc,0x9a,0xdd,0xde,0x98,
	0x9d,0xac,0xba,0x2c,0x25,0x5f,0x3e,0x3f,
	0xd7,0x88,0x94,0xb0,0xb1,0xb2,0xfc,0xd6,
	0xfb,0x60,0x3a,0x23,0x40,0x27,0x3d,0x22,
	0xf8,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
	0x68,0x69,0x96,0xa4,0xf3,0xaf,0xae,0xc5,
	0x8c,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,
	0x71,0x72,0x97,0x87,0xce,0x93,0xf1,0xfe,
	0xc8,0x7e,0x73,0x74,0x75,0x76,0x77,0x78,
	0x79,0x7a,0xef,0xc0,0xda,0x5b,0xf2,0xf9,
	0xb5,0xb6,0xfd,0xb7,0xb8,0xb9,0xe6,0xbb,
	0xbc,0xbd,0x8d,0xd9,0xbf,0x5d,0xd8,0xc4,
	0x7b,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
	0x48,0x49,0xcb,0xca,0xbe,0xe8,0xec,0xed,
	0x7d,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,0x50,
	0x51,0x52,0xa1,0xad,0xf5,0xf4,0xa3,0x8f,
	0x5c,0xe7,0x53,0x54,0x55,0x56,0x57,0x58,
	0x59,0x5a,0xa0,0x85,0x8e,0xe9,0xe4,0xd1,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
	0x38,0x39,0xb3,0xf7,0xf0,0xfa,0xa7,0xff,
 	};
/*...e*/
#endif
/*...sascii_to_ebcdic:0:*/
static gbm_u8 ascii_to_ebcdic[0x100] =
	{
	0x00,0x01,0x02,0x03,0x37,0x2d,0x2e,0x2f,
	0x16,0x05,0x25,0x0b,0x0c,0x0d,0x0e,0x0f,
	0x10,0x11,0x12,0x13,0x3c,0x3d,0x32,0x26,
	0x18,0x19,0x3f,0x27,0x22,0x1d,0x35,0x1f,
	0x40,0x5a,0x7f,0x7b,0x5b,0x6c,0x50,0x7d,
	0x4d,0x5d,0x5c,0x4e,0x6b,0x60,0x4b,0x61,
	0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,
	0xf8,0xf9,0x7a,0x5e,0x4c,0x7e,0x6e,0x6f,
	0x7c,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,
	0xc8,0xc9,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,
	0xd7,0xd8,0xd9,0xe2,0xe3,0xe4,0xe5,0xe6,
	0xe7,0xe8,0xe9,0xad,0xe0,0xbd,0x5f,0x6d,
	0x79,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
	0x88,0x89,0x91,0x92,0x93,0x94,0x95,0x96,
	0x97,0x98,0x99,0xa2,0xa3,0xa4,0xa5,0xa6,
	0xa7,0xa8,0xa9,0xc0,0x4f,0xd0,0xa1,0x07,
	0x43,0x20,0x21,0x1c,0x23,0xeb,0x24,0x9b,
	0x71,0x28,0x38,0x49,0x90,0xba,0xec,0xdf,
	0x45,0x29,0x2a,0x9d,0x72,0x2b,0x8a,0x9a,
	0x67,0x56,0x64,0x4a,0x53,0x68,0x59,0x46,
	0xea,0xda,0x2c,0xde,0x8b,0x55,0x41,0xfe,
	0x58,0x51,0x52,0x48,0x69,0xdb,0x8e,0x8d,
	0x73,0x74,0x75,0xfa,0x15,0xb0,0xb1,0xb3,
	0xb4,0xb5,0x6a,0xb7,0xb8,0xb9,0xcc,0xbc,
	0xab,0x3e,0x3b,0x0a,0xbf,0x8f,0x3a,0x14,
	0xa0,0x17,0xcb,0xca,0x1a,0x1b,0x9c,0x04,
	0x34,0xef,0x1e,0x06,0x08,0x09,0x77,0x70,
	0xbe,0xbb,0xac,0x54,0x63,0x65,0x66,0x62,
	0x30,0x42,0x47,0x57,0xee,0x33,0xb6,0xe1,
	0xcd,0xed,0x36,0x44,0xce,0xcf,0x31,0xaa,
	0xfc,0x9e,0xae,0x8c,0xdd,0xdc,0x39,0xfb,
	0x80,0xaf,0xfd,0x78,0x76,0xb2,0x9f,0xff,
	};
/*...e*/

static void ebcdic(char *dst, char *src)
	{
	while ( *src )
		*dst++ = ascii_to_ebcdic[(unsigned char)*src++];
	}
/*...e*/
/*...sgetword:0:*/
static int getword(gbm_u8 *c)
	{
	return (int) ( ((unsigned int) c[0] << 8) + (unsigned int) c[1] );
	}
/*...e*/
/*...sputword:0:*/
static void putword(gbm_u8 *c, int d)
	{
	c[0] = (gbm_u8) ((unsigned)d>>8);
	c[1] = (gbm_u8)            d    ;
	}
/*...e*/

/*...spsg_qft:0:*/
GBM_ERR psg_qft(GBMFT *gbmft)
	{
	*gbmft = psg_gbmft;
	return GBM_ERR_OK;
	}
/*...e*/
/*...spsg_rhdr:0:*/
GBM_ERR psg_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
	{
	PSEG_PRIV *psg_priv = (PSEG_PRIV *) gbm->priv;
	gbm_u8 buf[1+2+6];

	fn=fn; /* Suppress 'unref arg' compiler warnings */

	psg_priv->fix_badrec = ( gbm_find_word(opt, "fixrec") != NULL );

	gbm->bpp =  1;
	gbm->w   = -1;

	while ( gbm_file_read(fd, buf, sizeof(buf)) == sizeof(buf) )
		{
		long seek_by; int f;
		if ( psg_priv->fix_badrec )
			for ( f = 0; buf[0] != 0x5a && f < 10; f++ )
				{
				long pos = gbm_file_lseek(fd, 0L, GBM_SEEK_CUR);
				if ( pos == 0L )
					break;
				gbm_file_lseek(fd, pos - (sizeof(buf)+1), GBM_SEEK_SET);
				gbm_file_read(fd, buf, sizeof(buf));
				}
		if ( buf[0] != 0x5a )
			return GBM_ERR_PSEG_NOT_5A;
		seek_by = (long) getword(buf+1)-(2+6);
		     if ( !memcmp(buf+1+2, "\xd3\xee\xee", 3) )
			; /* Comment record, ignore it */
		else if ( !memcmp(buf+1+2, "\xd3\xa8\x5f", 3) )
			; /* BeginPageSegment, ignore it */
		else if ( !memcmp(buf+1+2, "\xd3\xa8\x7b", 3) )
			; /* BeginImageBlock, ignore it */
		else if ( !memcmp(buf+1+2, "\xd3\xa7\x7b", 3) )
			; /* ImageOutputControl, ignore it */
		else if ( !memcmp(buf+1+2, "\xd3\xa6\x7b", 3) )
			/* InputImageDescriptor, remember its info */
			{
			gbm_u8 buf2[6+12+2+2+6+2+2+4];
			if ( gbm_file_read(fd, buf2, sizeof(buf2)) != sizeof(buf2) )
				return GBM_ERR_READ;
			seek_by -= sizeof(buf2);
			gbm->w = getword(buf2+6+12);
			gbm->h = getword(buf2+6+12+2);
			if ( gbm->w & 7 )
				return GBM_ERR_PSEG_INV_WIDTH;
			psg_priv->xcellsizedef = getword(buf2+6+12+2+2+6);
			psg_priv->ycellsizedef = getword(buf2+6+12+2+2+6+2);
			if ( psg_priv->xcellsizedef == 0 &&
			     psg_priv->ycellsizedef == 0 )
				/* No cell structure, image is one lump */
				{
				psg_priv->xcellsizedef = gbm->w;
				psg_priv->ycellsizedef = gbm->h;
				}
			else if ( psg_priv->xcellsizedef & 7 )
				/* Insist upon a multiple of 8 */
				return GBM_ERR_PSEG_INV_DEFCEL;
			}
		else if ( !memcmp(buf+1+2, "\xd3\xee\x7b", 3) ||
			  !memcmp(buf+1+2, "\xd3\xac\x7b", 3) )
			/* RasterData, or */
			/* ImageCellPosition, seek back and abort loop */
			{
			psg_priv->pos = gbm_file_lseek(fd, - (long) sizeof(buf), GBM_SEEK_CUR);
			return gbm->w != -1 ? GBM_ERR_OK : GBM_ERR_PSEG_SIZE;
			}
		else
			return GBM_ERR_PSEG_BAD_RECORD;
		gbm_file_lseek(fd, seek_by, GBM_SEEK_CUR);
		}

	return GBM_ERR_READ;
	}
/*...e*/
/*...spsg_rpal:0:*/
GBM_ERR psg_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
	{
	fd=fd; gbm=gbm; /* Suppress compiler warnings */
	gbmrgb[0].r = gbmrgb[0].g = gbmrgb[0].b = 0xff;
	gbmrgb[1].r = gbmrgb[1].g = gbmrgb[1].b = 0x00;
	return GBM_ERR_OK;
	}
/*...e*/
/*...spsg_rdata:0:*/
GBM_ERR psg_rdata(int fd, GBM *gbm, gbm_u8 *data)
	{
	PSEG_PRIV *psg_priv = (PSEG_PRIV *) gbm->priv;
	gbm_u8 buf[1+2+6];
	int stride = ( ( gbm->w*gbm->bpp + 31 ) / 32 ) * 4;
	int xcellpos  = 0;
	int ycellpos  = 0;
	int xcellsize = psg_priv->xcellsizedef;
	int ycellsize = psg_priv->ycellsizedef;

	gbm_file_lseek(fd, psg_priv->pos, GBM_SEEK_SET);

	while ( gbm_file_read(fd, buf, sizeof(buf)) == sizeof(buf) )
		{
		long seek_by; int f;
		if ( psg_priv->fix_badrec )
			for ( f = 0; buf[0] != 0x5a && f < 10; f++ )
				{
				long pos = gbm_file_lseek(fd, 0L, GBM_SEEK_CUR);
				if ( pos == 0L )
					break;
				gbm_file_lseek(fd, pos - (sizeof(buf)+1), GBM_SEEK_SET);
				gbm_file_read(fd, buf, sizeof(buf));
				}
		if ( buf[0] != 0x5a )
			return GBM_ERR_PSEG_NOT_5A;
		seek_by = (long) getword(buf+1)-(2+6);
		     if ( !memcmp(buf+1+2, "\xd3\xee\xee", 3) )
			; /* Comment record, ignore it */
		else if ( !memcmp(buf+1+2, "\xd3\xa8\x5f", 3) )
			return GBM_ERR_PSEG_UNEX_REC; /* BeginPageSegment */
		else if ( !memcmp(buf+1+2, "\xd3\xa8\x7b", 3) )
			return GBM_ERR_PSEG_UNEX_REC; /* BeginImageBlock */
		else if ( !memcmp(buf+1+2, "\xd3\xa7\x7b", 3) )
			return GBM_ERR_PSEG_UNEX_REC; /* ImageOutputControl */
		else if ( !memcmp(buf+1+2, "\xd3\xa6\x7b", 3) )
			return GBM_ERR_PSEG_UNEX_REC; /* InputImageDescriptor */
		else if ( !memcmp(buf+1+2, "\xd3\xac\x7b", 3) )
			/* ImageCellPosition, ignore it */
			{
			gbm_u8 buf2[12], *p;
			int xfillsize, yfillsize, y;
			if ( gbm_file_read(fd, buf2, sizeof(buf2)) != sizeof(buf2) )
				return GBM_ERR_READ;
			xcellpos  = getword(buf2);
			if ( xcellpos & 7 )
				return GBM_ERR_PSEG_INV_CELPOS;
			ycellpos  = getword(buf2+2);
			xcellsize = getword(buf2+4);
			if ( xcellsize & 7 )
				return GBM_ERR_PSEG_INV_CEL;
			ycellsize = getword(buf2+6);
			xfillsize = getword(buf2+8);
			yfillsize = getword(buf2+10);
			if ( xfillsize != 0xffff && yfillsize != 0xffff )
				{
				p = data + ( (gbm->h-1-ycellpos) * stride + xcellpos/8 );
				for ( y = 0; y < yfillsize; y++, p -= stride )
					memset(p, 0x00, xfillsize/8);
				}
			seek_by -= sizeof(buf2);
			}
		else if ( !memcmp(buf+1+2, "\xd3\xee\x7b", 3) )
			/* RasterData, use the data */
			{
			gbm_u8 *p = data + ( (gbm->h-1-ycellpos) * stride + xcellpos/8 );
			int xc = xcellsize/8;

			for ( ; seek_by >= xc; seek_by -= xc, p -= stride, ycellpos++, ycellsize-- )
				if ( gbm_file_read(fd, p, xc) != xc )
					return GBM_ERR_READ;
			}
		else if ( !memcmp(buf+1+2, "\xd3\xa9\x7b", 3) )
			/* EndImageBlock */
			return GBM_ERR_OK;
		else
			return GBM_ERR_PSEG_BAD_RECORD;
		gbm_file_lseek(fd, seek_by, GBM_SEEK_CUR);
		}

	return GBM_ERR_READ;
	}
/*...e*/
/*...spsg_w:0:*/
/*...srecord:0:*/
static gbm_boolean record(int fd, gbm_u8 *rec, char *three, int len, int *recnum)
	{
	rec[0] = 0x5a;
	putword(rec+1, 2+6+len);
	memcpy(rec+1+2, three, 3);
	rec[3] = 0x00; /* Flags */
	putword(rec+4, *recnum);
	(*recnum)++;
	return gbm_file_write(fd, rec, 1+2+6+len) == 1+2+6+len;
	}
/*...e*/

GBM_ERR psg_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
	{
	static char by_gbm[] = "Written by GBM", buf8[8+1];
	static gbm_u8 *rec, *r;
	int stride = ( ( gbm->w + 31 ) / 32 ) * 4;
	int len = (gbm->w+7)/8;
	int i;
	gbm_u8 lastbits = 0xff;
	const char *str;
	int scalex = 1000, scaley = 1000;
	int pelx = 2400, pely = 2400;
	int recnum = 1;

	gbmrgb=gbmrgb; /* Suppress compiler warning */

	if ( gbm->bpp != 1 )
		return GBM_ERR_NOT_SUPP;

	if ( gbm->w > 2400 || gbm->h > 2400 )
		return GBM_ERR_BAD_SIZE;

	if ( (str = gbm_find_word_prefix(opt, "scalex=")) != NULL )
		sscanf(str+7, "%i", &scalex);
	if ( (str = gbm_find_word_prefix(opt, "scaley=")) != NULL )
		sscanf(str+7, "%i", &scaley);
	if ( gbm_find_word(opt, "double") != NULL )
		{ scalex *= 2; scaley *= 2; }
	if ( (str = gbm_find_word_prefix(opt, "pelx=")) != NULL )
		sscanf(str+5, "%i", &pelx);
	if ( (str = gbm_find_word_prefix(opt, "pely=")) != NULL )
		sscanf(str+5, "%i", &pely);

	if ( (rec = gbmmem_malloc((size_t) (1+2+6+max(100,stride)))) == NULL )
		return GBM_ERR_MEM;
	r = rec + 1+2+6;

	sprintf(buf8, "%-8.8s", fn);
	ebcdic(buf8, buf8);

	/* Comment */
	ebcdic((char *) r, by_gbm);
	if ( !record(fd, rec, "\xd3\xee\xee", strlen(by_gbm), &recnum) )
		{ gbmmem_free(rec); return GBM_ERR_WRITE; }

	/* BeginPageSegment (we will use filename) */
	memcpy(r, buf8, 8);
	if ( !record(fd, rec, "\xd3\xa8\x5f", 8, &recnum) )
		{ gbmmem_free(rec); return GBM_ERR_WRITE; }

	/* BeginImageBlock (use filename again) */
	if ( !record(fd, rec, "\xd3\xa8\x7b", 8, &recnum) )
		{ gbmmem_free(rec); return GBM_ERR_WRITE; }

	/* ImageOutputControl */
	memset(r, 0x00, 6+4+8); /* Image Block Origin, Orientation, Reserved */
	r[8] = 0x2d; /* 90 degrees */
	putword(r+6+4+8  , scalex);
	putword(r+6+4+8+2, scaley);
	memset(r+6+4+8+4, 0xff, 2);
	if ( !record(fd, rec, "\xd3\xa7\x7b", 6+4+8+4+2, &recnum) )
		{ gbmmem_free(rec); return GBM_ERR_WRITE; }

	/* ImageInputDescriptor */
	memset(r, 0x00, 6+12+2+2+6);
	putword(r+2, 2400);
	putword(r+4, 2400);
	putword(r+6+ 8, pelx);
	putword(r+6+10, pely);
	putword(r+6+12, (gbm->w+7)&~7);
	putword(r+6+12+2, gbm->h);
	        r[6+12+2+2 + 4] = 0x2d;
	putword(r+6+12+2+2+6, 0);	/* Not writing in cells */
	putword(r+6+12+2+2+6+2, 0);	/* Not writing in cells */
	memcpy( r+6+12+2+2+6+2+2, "\x00\x01\xff\xff", 4);
	if ( !record(fd, rec, "\xd3\xa6\x7b", 6+12+2+2+6+2+2+4, &recnum) )
		{ gbmmem_free(rec); return GBM_ERR_WRITE; }

 	if ( (unsigned)gbm->w&7U )
 		lastbits = (gbm_u8) ( 0xff00U >> ((unsigned)gbm->w&7U) );
	data += (gbm->h-1) * stride;
 	for ( i = 0; i < gbm->h; i++, data -= stride )
 		{
 		memcpy(r, data, len); r[len-1] &= lastbits;
 		if ( !record(fd, rec, "\xd3\xee\x7b", len, &recnum) )
 			{ gbmmem_free(rec); return GBM_ERR_WRITE; }
 		}

 	/* EndImageBlock, (use filename) */
	memcpy(r, buf8, 8);
 	if ( !record(fd, rec, "\xd3\xa9\x7b", 8, &recnum) )
		{ gbmmem_free(rec); return GBM_ERR_WRITE; }

 	/* EndPageSegment (use filename from before) */
 	if ( !record(fd, rec, "\xd3\xa9\x5f", 8, &recnum) )
		{ gbmmem_free(rec); return GBM_ERR_WRITE; }

 	gbmmem_free(rec);

	return GBM_ERR_OK;
	}
/*...e*/
/*...spsg_err:0:*/
const char *psg_err(GBM_ERR rc)
	{
	switch ( (int) rc )
		{
		case GBM_ERR_PSEG_NOT_5A:
			return "record in the PSEG file does not start with 0x5a (try fixrec option)";
		case GBM_ERR_PSEG_BAD_RECORD:
			return "bad/unknown record in PSEG file";
		case GBM_ERR_PSEG_SIZE:
			return "size record not seen before raster data or image cell position records";
		case GBM_ERR_PSEG_INV_WIDTH:
			return "width must be a multiple of 8";
		case GBM_ERR_PSEG_INV_DEFCEL:
			return "default cell size must be a multiple of 8";
		case GBM_ERR_PSEG_INV_CEL:
			return "cell size must be a multiple of 8";
		case GBM_ERR_PSEG_INV_CELPOS:
			return "cell position must be a multiple of 8";
		case GBM_ERR_PSEG_UNEX_REC:
			return "record found in an unexpected place";
		}
	return NULL;
	}
/*...e*/
