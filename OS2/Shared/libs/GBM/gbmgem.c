/*

gbmgem.c - GEM Raster support

Reads 1 bpp files and returns 1 bpp data.
Reads 2,3 and 4 bpp files and returns 4 bpp data.
Reads 5,6,7 and 8 bpp files and returns 8 bpp data.
Reads 24 bpp XIMG files and returns 24 bpp data.
Writes 1 bpp data as B/W 1 bpp file.
Writes 4 bpp data mapped to 'standard' 16 colour palette.
Or writes 4 bpp data greyscale 4 bpp file, if output option "grey" given.
Or writes 4 bpp data to XIMG file with palette if "pal" option given.
Writes 8 bpp data as greyscale 8 bpp file.
Or writes 8 bpp to XIMG file with palette if "pal" option given.
Writes 24 bpp data as an XIMG file.
The pixel dimensions in microns can be specified via "pixw=#,pixh=#".

Reference material used :-
	p300 of EoGFF book.
		Documents 1 bpp, 4 bpp and 8 bpp formats.
		No mention of XIMG extension.
		Suspected inaccurate - 8 word 4 bpp files are *colour*.
		Its supplied sample FLAG_B24.IMG on CD-ROM proves the point.
	Revision 17 of Image Alchemy.
		Suspected wrong also - reorders palette writing 4 bpp IMG.
		EoGFF sample and other samples all agree, Alchemy odd-one-out.
	Public Domain source and other sample files.
		Introduces notion of XIMG header extension block.
		XIMGs can include palette entries (exact layout guessed).
		XIMGs introduces notion of 24 bpp files.

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

/*...suseful:0:*/
static gbm_u16 get_u16(gbm_u8 *p) { return (p[0]<<8)|p[1]; }
static void put_u16(gbm_u8 *p, gbm_u16 w) { p[0] = (gbm_u8) (w>>8); p[1] = (gbm_u8) w; }
/*...e*/

static GBMFT gem_gbmft =
	{
	GBM_FMT_DESC_SHORT_GEM,
	GBM_FMT_DESC_LONG_GEM,
	GBM_FMT_DESC_EXT_GEM,
	GBM_FT_R1|GBM_FT_R4|GBM_FT_R8|GBM_FT_R24|
	GBM_FT_W1|GBM_FT_W4|GBM_FT_W8|GBM_FT_W24,
	};

/*...serror codes:0:*/
#define	GBM_ERR_GEM_IS_DVI	((GBM_ERR) 1700)
#define	GBM_ERR_GEM_IS_IMDS	((GBM_ERR) 1701)
#define	GBM_ERR_GEM_BAD_VERSION	((GBM_ERR) 1702)
#define	GBM_ERR_GEM_BAD_HDRLEN	((GBM_ERR) 1703)
#define	GBM_ERR_GEM_BAD_PLANES	((GBM_ERR) 1704)
#define	GBM_ERR_GEM_BAD_FLAG	((GBM_ERR) 1705)
#define	GBM_ERR_GEM_FIXEDCPAL	((GBM_ERR) 1706)
#define	GBM_ERR_GEM_XIMG_TYPE	((GBM_ERR) 1707)
/*...e*/

typedef struct
	{
	gbm_u16 lenhdr, planes, patlen, flag;
	gbm_boolean ximg;
	} GEM_PRIV;

/*...srgb_bgr:0:*/
static void rgb_bgr(const gbm_u8 *p, gbm_u8 *q, size_t n)
	{
	while ( n-- )
		{
		gbm_u8 r = *p++;
		gbm_u8 g = *p++;
		gbm_u8 b = *p++;

		*q++ = b;
		*q++ = g;
		*q++ = r;
		}
	}
/*...e*/

/*...sgem_qft:0:*/
GBM_ERR gem_qft(GBMFT *gbmft)
	{
	*gbmft = gem_gbmft;
	return GBM_ERR_OK;
	}
/*...e*/
/*...sgem_rhdr:0:*/
/* GEM-Raster files, The green plane of an ActionMedia DVI still, and IMDS
   image files all share the common .IMG file extension. GBM only supports
   GEM-Raster files, but to be nice, I'll give a nice diagnostic if I detect
   one of the others.

   GEM-Rasters can be :-
      8 word header, data
         planes==1 => b/w image
         planes==3 => fixed 8 colour palette (guesswork)
         planes==4 => fixed 16 colour palette
      9 word header including flag, data
         flag==0 => treat as 8 word header
         flag==1 => treat as greyscale
      other size header, with XIMG signiture and 3<<planes words palette, data
      other size header, with XIMG signiture and planes==24, data
*/

static gbm_u8 imds_begin_image_segment[] = { 0x70, 0x04, 0x00, 0x00, 0x00, 0x00 };
static gbm_u8 imds_modca_wrapping     [] = { 0x08, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00 };

GBM_ERR gem_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
	{
	GEM_PRIV *priv = (GEM_PRIV *) gbm->priv;
	gbm_u8 hdr[22];

	fn=fn; opt=opt; /* Suppress 'unref arg' compiler warnings */

	if ( gbm_file_read(fd, hdr, 22) != 22 )
		return GBM_ERR_READ;

	if ( !memcmp(hdr, "VDVI", 4) )
		return GBM_ERR_GEM_IS_DVI;

	if ( !memcmp(hdr, imds_begin_image_segment, sizeof(imds_begin_image_segment)) ||
	     !memcmp(hdr, imds_modca_wrapping, sizeof(imds_modca_wrapping)) )
		return GBM_ERR_GEM_IS_IMDS;

	if ( get_u16(hdr) != 1 )
		return GBM_ERR_GEM_BAD_VERSION;

	priv->lenhdr = get_u16(hdr+1*2);
	if ( priv->lenhdr < 8 )
		return GBM_ERR_GEM_BAD_HDRLEN;

	gbm_file_lseek(fd, priv->lenhdr*2, GBM_SEEK_SET);

	if ( !memcmp(hdr+8*2, "XIMG", 4) )
		/* XIMG signiture has been found */
		{
		priv->ximg = GBM_TRUE;
		if ( get_u16(hdr+10*2) != 0 )
			return GBM_ERR_GEM_XIMG_TYPE;
		}
	else
		priv->ximg = GBM_FALSE;

	priv->planes = get_u16(hdr+2*2);
	if ( priv->planes >= 1 && priv->planes <= 8 )
		; /* All ok */
	else if ( priv->ximg && priv->planes == 24 )
		; /* Still all ok */
	else
		/* Don't know how to handle this */
		return GBM_ERR_GEM_BAD_PLANES;

	priv->patlen = get_u16(hdr+3*2);

	gbm->w = get_u16(hdr+6*2);
	gbm->h = get_u16(hdr+7*2);

	if ( priv->lenhdr == 9 )
		{
		priv->flag = get_u16(hdr+8*2);
		if ( priv->flag != 0 && priv->flag != 1 )
			return GBM_ERR_GEM_BAD_FLAG;
		}

	switch ( priv->planes )
		{
		case 1:
			gbm->bpp = 1; break;
		case 2: case 3: case 4:
			gbm->bpp = 4; break;
		case 5: case 6: case 7: case 8:
			gbm->bpp = 8; break;
		case 24:
			gbm->bpp = 24; break;
		}

	/* Enforce assumed rules */
	if ( priv->lenhdr == 8 ||
	     (priv->lenhdr == 9 && priv->flag == 0) )
		/* Is a fixed colour palette */
		if ( priv->planes != 1 &&
		     priv->planes != 3 &&
		     priv->planes != 4 )
			return GBM_ERR_GEM_FIXEDCPAL;

	return GBM_ERR_OK;
	}
/*...e*/
/*...sgem_rpal:0:*/
/* This palettes were determined by looking at .IMG files and their equivelent
   .BMP files (that had been converted elsewhere). */

/*...sgbmrgb_3 \45\ 3 plane palette:0:*/
static GBMRGB gbmrgb_3[] =
	{
	/* r, g, b */
	{ 0xff,0xff,0xff },		/* White */
	{ 0xff,0   ,0    },		/* Red */
	{ 0   ,0xff,0    },		/* Green */
	{ 0xff,0xff,0    },		/* (Yellow) Orange */
	{ 0   ,0   ,0xff },		/* Blue */
	{ 0xff,0   ,0xff },		/* Magenta */
	{ 0   ,0xff,0xff },		/* Cyan */
	{ 0   ,0   ,0    }		/* Black */
	};
/*...e*/
/*...sgbmrgb_4 \45\ 4 plane palette:0:*/
static GBMRGB gbmrgb_4[] =
	{
	/* r, g, b */
	{ 0xff,0xff,0xff },		/* White */
	{ 0xff,0   ,0    },		/* Red */
	{ 0   ,0xff,0    },		/* Green */
	{ 0xff,0xff,0    },		/* (Yellow) Orange */
	{ 0   ,0   ,0xff },		/* Blue */
	{ 0xff,0   ,0xff },		/* Magenta */
	{ 0   ,0xff,0xff },		/* Cyan */
	{ 0xcc,0xcc,0xcc },		/* Grey */
	{ 0x80,0x80,0x80 },		/* Dark grey */
	{ 0x80,0   ,0    },		/* Dark red */
	{ 0   ,0x80,0    },		/* Dark green */
	{ 0x80,0x80,0    },		/* Dark yellow */
	{ 0   ,0   ,0x80 },		/* Dark blue */
	{ 0x80,0   ,0x80 },		/* Dark magenta */
	{ 0   ,0x80,0x80 },		/* Dark cyan */
	{ 0   ,0   ,0    }		/* Black */
	};
/*...e*/

GBM_ERR gem_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
	{
	GEM_PRIV *priv = (GEM_PRIV *) gbm->priv;
	if ( priv->planes == 24 )
		return GBM_ERR_OK;
	if ( priv->ximg )
/*...shandle the palette in the file:16:*/
{
int i, ncols = (1 << priv->planes);
gbm_file_lseek(fd, (8+3)*2, GBM_SEEK_SET);
for ( i = 0; i < ncols; i++ )
	{
	gbm_u8 palentry[3*2];
	if ( gbm_file_read(fd, palentry, 3*2) != 3*2 )
		return GBM_ERR_READ;
	/* Guesswork, based on sample file intel.img */
	gbmrgb[i].r = (gbm_u8) ((get_u16(palentry  )*255UL)/1000UL);
	gbmrgb[i].g = (gbm_u8) ((get_u16(palentry+2)*255UL)/1000UL);
	gbmrgb[i].b = (gbm_u8) ((get_u16(palentry+4)*255UL)/1000UL);
	}
}
/*...e*/
	else
		switch ( gbm->bpp )
			{
/*...s1 \45\ fixed b\47\w palette:24:*/
case 1:
	gbmrgb[0].r = gbmrgb[0].g = gbmrgb[0].b = 0xff;
	gbmrgb[1].r = gbmrgb[1].g = gbmrgb[1].b = 0x00;
	break;
/*...e*/
/*...s4 \45\ either greyscale or a fixed palette:24:*/
case 4:
	if ( priv->lenhdr == 9 && priv->flag == 1 )
		/* Is greyscale data */
		/* Its guesswork that it goes dark to light */
		{
		int i, cols = (1<<priv->planes);
		for ( i = 0; i < cols; i++ )
			gbmrgb[i].r =
			gbmrgb[i].g =
			gbmrgb[i].b = (gbm_u8) ((i*0xff)/(cols-1));
		}
	else
		switch ( priv->planes )
			{
			case 3:
				memcpy(gbmrgb, gbmrgb_3, sizeof(gbmrgb_3));
				break;
			case 4:
				memcpy(gbmrgb, gbmrgb_4, sizeof(gbmrgb_4));
				break;
			}
	break;
/*...e*/
/*...s8 \45\ greyscale palette:24:*/
case 8:
	/* Again, its guesswork that it goes from dark to light */
	{
	int i;
	for ( i = 0; i < 0x100; i++ )
		gbmrgb[i].r =
		gbmrgb[i].g =
		gbmrgb[i].b = (gbm_u8) i;
	}
	break;
/*...e*/
			}
	return GBM_ERR_OK;
	}
/*...e*/
/*...sgem_rdata:0:*/
/*...sread_gem_line:0:*/
/* Vertical replication codes are only allowed in certain places */

static void read_gem_line(
	AHEAD *ahead,
	gbm_u8 *line, int scan,
	int patlen,
	int *vrep, gbm_boolean allow_vrep
	)
	{
	while ( scan )
		{
		gbm_u8 b1 = (gbm_u8) gbm_read_ahead(ahead);
		if ( b1 == 0x80 )
			/* Literal run */
			{
			gbm_u8 len = (gbm_u8) gbm_read_ahead(ahead);
			scan -= len;
			while ( len-- > 0 )
				*line++ = (gbm_u8) gbm_read_ahead(ahead);
			}
		else if ( b1 == 0x00 )
			/* Pattern code */
			{
			gbm_u8 rep = (gbm_u8) gbm_read_ahead(ahead);
			if ( rep == 0 && allow_vrep )
				/* Is actually a vertical replication */
				{
				gbm_read_ahead(ahead); /* Swallow 0xff */
				*vrep = (int) gbm_read_ahead(ahead) - 1;
				}
			else
				{
				int i;
				scan -= patlen*rep;
				for ( i = 0; i < patlen; i++ )
					*line++ = (gbm_u8) gbm_read_ahead(ahead);
				while ( rep-- > 1 )
					for ( i = 0; i < patlen; i++, line++ )
						*line = line[-patlen];
				}
			}
		else
			/* Is a black/white (=0xff's/0x00's) run code */
			{
			gbm_u8 store = (gbm_u8) ( (signed char) b1 >> 7 );
			b1 &= 0x7f;
			memset(line, store, b1);
			line += b1;
			scan -= b1;
			}
		allow_vrep = GBM_FALSE;
		}
	}
/*...e*/
/*...sspread:0:*/
static void spread(gbm_u8 b, gbm_u8 bit_to_set, gbm_u8 *dest)
	{
	if ( b & 0x80 ) dest[0] |= (bit_to_set & 0xf0);
	if ( b & 0x40 ) dest[0] |= (bit_to_set & 0x0f);
	if ( b & 0x20 ) dest[1] |= (bit_to_set & 0xf0);
	if ( b & 0x10 ) dest[1] |= (bit_to_set & 0x0f);
	if ( b & 0x08 ) dest[2] |= (bit_to_set & 0xf0);
	if ( b & 0x04 ) dest[2] |= (bit_to_set & 0x0f);
	if ( b & 0x02 ) dest[3] |= (bit_to_set & 0xf0);
	if ( b & 0x01 ) dest[3] |= (bit_to_set & 0x0f);
	}
/*...e*/

GBM_ERR gem_rdata(int fd, GBM *gbm, gbm_u8 *data)
	{
	GEM_PRIV *priv = (GEM_PRIV *) gbm->priv;
	size_t scan = (gbm->w+7)/8;
	size_t stride = ((gbm->w*gbm->bpp + 31) / 32) * 4;
	gbm_u8 *line;
	AHEAD *ahead;

	if ( (ahead = gbm_create_ahead(fd)) == NULL )
		return GBM_ERR_MEM;

	if ( (line = gbmmem_malloc(scan)) == NULL )
		{
		gbm_destroy_ahead(ahead);
		return GBM_ERR_MEM;
		}

	switch ( gbm->bpp )
		{
/*...s1:16:*/
case 1:
	{
	int y, vrep = 0;
	data += stride * (gbm->h-1);
	for ( y = gbm->h - 1; y >= 0; y--, data -= stride )
		if ( vrep )
			{ memcpy(data, data+stride, stride); vrep--; }
		else
			read_gem_line(ahead, data, (int)scan, priv->patlen, &vrep, GBM_TRUE);
	}
	break;
/*...e*/
/*...s4:16:*/
case 4:
	{
	int y, vrep = 0;
	int bytes = (gbm->w / 8);
	int bits  = (gbm->w & 7);

	memset(data, 0, stride * gbm->h);
	data += (gbm->h-1) * stride;
	for ( y = gbm->h - 1; y >= 0; y--, data -= stride )
		if ( vrep )
			{ memcpy(data, data+stride, stride); vrep--; }
		else
			{
			int p;
			gbm_u8 bit;

			for ( p = 0, bit = 0x11; p < priv->planes; p++, bit <<= 1 )
				{
				int x;
				gbm_u8 *dest = data;
				read_gem_line(ahead, line, (int)scan, priv->patlen, &vrep, p==0);
				for ( x = 0; x < bytes; x++, dest += 4 )
					spread(line[x], bit, dest);
				if ( bits )
					spread((gbm_u8) (line[x] & (0xff00U >> bits)), bit, dest);
				}
			}
	}
	break;
/*...e*/
/*...s8:16:*/
case 8:
	{
	int y, vrep = 0;
	memset(data, 0, stride * gbm->h);
	data += (gbm->h-1) * stride;
	for ( y = gbm->h - 1; y >= 0; y--, data -= stride )
		if ( vrep )
			{ memcpy(data, data+stride, stride); vrep--; }
		else
			{
			int p;
			gbm_u8 bit;
			for ( p = 0, bit = 0x01; p < priv->planes; p++, bit <<= 1 )
				{
				int x;
				read_gem_line(ahead, line, (int)scan, priv->patlen, &vrep, p==0);
				for ( x = 0; x < gbm->w; x++ )
					if ( line[x>>3]&(0x80U>>(x&7)) )
						data[x] |= bit;
				}
			}
	}
	break;
/*...e*/
/*...s24:16:*/
case 24:
	/* 24bpp data is strange in that it isn't bit planed at all like
	   the others are. This makes decoding quicker, and most 24bpp
	   hardware is not bit planar, and this may explain why.
	   Of course, my guesswork says that the order is R,G,B and as
	   GBM used B,G,R, i'll have to reverse the order. */
	{
	int y, vrep = 0;
	memset(data, 0, stride * gbm->h);
	data += (gbm->h-1) * stride;
	for ( y = gbm->h - 1; y >= 0; y--, data -= stride )
		if ( vrep )
			{ memcpy(data, data+stride, stride); vrep--; }
		else
			{
			read_gem_line(ahead, data, gbm->w*3, priv->patlen, &vrep, GBM_TRUE);
			rgb_bgr(data, data, gbm->w);
			}
	}
	break;
/*...e*/
		}

	gbmmem_free(line);

	gbm_destroy_ahead(ahead);

	return GBM_ERR_OK;
	}
/*...e*/
/*...sgem_w:0:*/
/*...swrite_pal:0:*/
static gbm_boolean write_pal(int fd, const GBMRGB *gbmrgb, int cnt)
	{
	int i;
	for ( i = 0; i < cnt; i++ )
		{
		gbm_u8 buf[3*2];
		put_u16(buf  , (gbm_u16) ((gbmrgb[i].r*1000UL)/255UL));
		put_u16(buf+2, (gbm_u16) ((gbmrgb[i].g*1000UL)/255UL));
		put_u16(buf+4, (gbm_u16) ((gbmrgb[i].b*1000UL)/255UL));
		if ( gbm_file_write(fd, buf, 3*2) != 3*2 )
			return GBM_FALSE;
		}
	return GBM_TRUE;
	}
/*...e*/
/*...smap_to_grey4:0:*/
/* Sum of R,G and B will be from 0 to 0x2fd. Sum/3 is from 0 to 0xff.
   Sum/0x30 is from 0 to 0x0f, which is what we want. */

static gbm_u8 map_to_grey4(const GBMRGB *gbmrgb)
	{
	return (gbm_u8) ( ( (gbm_u16) gbmrgb->r +
			  (gbm_u16) gbmrgb->g +
			  (gbm_u16) gbmrgb->b ) / 0x30 );
	}
/*...e*/
/*...smap_to_col4:0:*/
/* The default 16 colour palette is disgusting to map to quickly.
   Still, never mind, we only expect to be called 512 times. */

/*...sdist_between:0:*/
static int dist_between(const GBMRGB *p1, const GBMRGB *p2)
	{
	int dr = (int) ( (unsigned int) p1->r - (unsigned int) p2->r );
	int dg = (int) ( (unsigned int) p1->g - (unsigned int) p2->g );
	int db = (int) ( (unsigned int) p1->b - (unsigned int) p2->b );
	return dr*dr + dg*dg + db*db;
	}
/*...e*/

static gbm_u8 map_to_col4(const GBMRGB *gbmrgb)
	{
	int i, i_min = 0;
	int dist_min = dist_between(gbmrgb, &gbmrgb_4[0]);
	for ( i = 1; i < 0x10; i++ )
		{
		int dist = dist_between(gbmrgb, &gbmrgb_4[i]);
		if ( dist < dist_min )
			{
			dist_min = dist;
			i_min = i;
			}
		}
	return (gbm_u8) i_min;
	}
/*...e*/
/*...shandle_vrep:0:*/
static gbm_boolean handle_vrep(int fd, const gbm_u8 *data, int step, int most, int *dy, int len)
	{
	gbm_u8 buf[4];
	if ( most > 0xff )
		most = 0xff;
	for ( *dy = 1; *dy < most; (*dy)++, data += step )
		if ( memcmp(data, data+step, len) )
			break;
	if ( (*dy) == 1 )
		return GBM_TRUE;
	buf[0] = 0x00;
	buf[1] = 0x00;
	buf[2] = 0xff;
	buf[3] = (gbm_u8) (*dy);
	return gbm_file_write(fd, buf, 4) == 4;
	}
/*...e*/
/*...sencode1:0:*/
/* Encode scan bytes of data into temporary buffer enc.
   0x00 repeated N times codes to N       does not expand
   0xff repeated N times codes to 0x80|N  does not expand
   X repeated N times codes to 0x00 N X   expands if N<3
   other N bytes codes to 0x80 N bytes    N=1 expands to 3  ie: 3:1
                                          N=2 expands to 4  ie: 2:1

   len(enc) = len(data)*4/2 + 2 should do it. Worst case typically like :-

   A B            0xff  C D            0xff  F
   0x80 0x02 A B  0x81  0x80 0x02 C D  0x81  0x80 0x01 F    */

/*...sfind_run1:0:*/
/* Look for a run of the same byte */

static gbm_u8 find_run1(const gbm_u8 *data, int len)
	{
	const gbm_u8 *p = data;
	gbm_u8 b = *p++;
	if ( len > 0x7f )
		len = 0x7f;
	while ( --len > 0 && *p == b )
		p++;
	return (gbm_u8) ( p-data );
	}
/*...e*/
/*...sfind_lit1:0:*/
/* Handle literal runs.
   We know the first 3 bytes (if there are as many as 3) differ.
   We can only return a number as high as 0x7f from this function. */

static gbm_u8 find_lit1(const gbm_u8 *data, int len)
	{
	const gbm_u8 *p;

	if ( len <= 3 )
		return len;
	if ( len > 0x7f )
		len = 0x7f;

	p = data + 2;
	while ( len-- > 3 && p[0] != p[1] && p[1] != 0x00 && p[1] != 0xff )
		p++;

	return (gbm_u8)(p-data);	
	}
/*...e*/

static int encode1(const gbm_u8 *data, int scan, gbm_u8 *enc)
	{
	gbm_u8 *enc_start = enc;
	while ( scan )
		{
		gbm_u8 len = find_run1(data, scan);
		if ( *data == 0x00 || *data == 0xff )
			*enc++ = ((*data&0x80)|len);
		else if ( len >= 3 )
			{
			*enc++ = 0x00;	/* Pattern */
			*enc++ = len;	/* Repeated len times */
			*enc++ = *data;	/* 1-byte pattern (patlen=1) */
			}
		else
			{
			len = find_lit1(data, scan);
			*enc++ = 0x80;	/* Literal run */
			*enc++ = len;
			memcpy(enc, data, len);
			enc += len;
			}
		data += len;
		scan -= len;
		}		
	return (int)(enc-enc_start);
	}
/*...e*/
/*...sencode3:0:*/
/* Like encode1, except that patlen=3, so we try to find 3-byte-patterns.
   In this code we always work in 3 byte aligned chunks each step.
   This is not quite optimal, but nothing worth worrying too much about.
   len(enc) = len(data)*5/3 + 2 should do it. Worst case is like :-

   A B C            0xff 0xff 0xff  D E F
   0x80 0x03 A B C  0x83            0x80 0x03 D E F     */

/*...sfind_run3:0:*/
/* Look for a run of the same 3-byte-pattern.
   We know that data[0..2] are not all 0x00's or 0xff's on entry.
   Takes length in 3-byte-units and returns length in 3-byte-units */

static gbm_u8 find_run3(const gbm_u8 *data, int len)
	{
	const gbm_u8 *p = data;
	gbm_u8 b0, b1, b2;
	if ( len <= 1 )
		return len;
	if ( len > 0x7f )
		len = 0x7f;
	b0 = *p++; b1 = *p++; b2 = *p++;
	while ( --len > 0 && p[0] == b0 && p[1] == b1 && p[2] == b2 )
		p += 3;
	return (gbm_u8) ( (p-data)/3 );
	}
/*...e*/
/*...sfind_lit3:0:*/
/* On entry data[0..2] differs from data[3..5], assuming len>=2.
   Takes length in 3 byte units, returns length in bytes. */

static gbm_u8 find_lit3(const gbm_u8 *data, int len)
	{
	const gbm_u8 *p;

	if ( len == 1 )
		return 3;
	if ( len > 0x7f/3 )
		len = 0x7f/3;

	p = data + 6;
	while ( --len >= 2               &&		/* Another 3 */
		(p[0]!=p[-3]||
		 p[1]!=p[-2]||
		 p[2]!=p[-1])            &&		/* They differ */
		(p[0]|p[1]|p[2]) != 0x00 &&		/* Next not all 0x00's */
		(p[0]&p[1]&p[2]) != 0xff )		/* And not all 0xff's */
		p += 3;
	return (gbm_u8) ( p-data );
	}
/*...e*/

static int encode3(const gbm_u8 *data, int scan, gbm_u8 *enc)
	{
	gbm_u8 *enc_start = enc;
	while ( scan )
		{
		gbm_u8 len;
		if ( (*data == 0x00 || *data == 0xff) &&
		     (len = find_run1(data, scan)) >= 3 )
			{
			len = ((len/3)*3);
			*enc++ = ((*data&0x80)|len);
			data += len;
			scan -= len;
			}
		else if ( (len = find_run3(data, scan/3)) >= 2 )
			{
			*enc++ = 0x00;		/* Pattern */
			*enc++ = len;		/* Repeated len times */
			*enc++ = data[0];	/* 3-byte pattern */
			*enc++ = data[1];
			*enc++ = data[2];
			data += 3*len;
			scan -= 3*len;
			}
		else
			{
			len = find_lit3(data, scan/3);
			*enc++ = 0x80;		/* Literal run */
			*enc++ = len;		/* # bytes */
			memcpy(enc, data, len);
			enc  += len;
			data += len;
			scan -= len;
			}
		}
	return (int)(enc-enc_start);
	}
/*...e*/
/*...sliterally:0:*/
/* The normal encode routines may result in expanding the data.
   This routine checks that, and if so, re-encodes it literally.
   A literal encoding only adds 2 bytes in 127. */

static int literally(const gbm_u8 *data, int scan, gbm_u8 *enc, int len_enc)
	{
	gbm_u8 *enc_start = enc;
	if ( len_enc <= (scan*(0x7f+2))/0x7f )
		return len_enc;
	while ( scan )
		{
		int len = min(scan, 0x7f);
		*enc++ = 0x80;
		*enc++ = (gbm_u8) len;
		memcpy(enc, data, len);
		enc  += len;
		data += len;
		scan -= len;
		}
	return (int)(enc-enc_start);
	}
/*...e*/

GBM_ERR gem_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
	{
	gbm_u8 hdr[22];
	int pixw = 85, pixh = 85;
	gbm_boolean pal, grey;
	int stride = ((gbm->w*gbm->bpp + 31) / 32) * 4;
	int scan = (gbm->w+7)/8;

/*...shandle options:8:*/
{
const char *s;

fn=fn; /* Suppress 'unref arg' compiler warnings */

pal  = ( gbm_find_word(opt, "pal") != NULL );
grey = ( gbm_find_word(opt, "grey") != NULL );

if ( (s = gbm_find_word_prefix(opt, "pixw=")) != NULL )
	sscanf(s + 5, "%d", &pixw);

if ( (s = gbm_find_word_prefix(opt, "pixh=")) != NULL )
	sscanf(s + 5, "%d", &pixh);
}
/*...e*/

	/* Initial stab at a header */
	put_u16(hdr    , (gbm_u16) 1);		/* Version */
	put_u16(hdr+1*2, (gbm_u16) 8);		/* Header length */
	put_u16(hdr+2*2, (gbm_u16) gbm->bpp);	/* Number of planes */
	put_u16(hdr+3*2, (gbm_u16) 1);		/* Pattern length */
	put_u16(hdr+4*2, (gbm_u16) pixw);		/* Pixel width in microns */
	put_u16(hdr+5*2, (gbm_u16) pixh);		/* Pixel height in microns */
	put_u16(hdr+6*2, (gbm_u16) gbm->w);	/* Width in pixels */
	put_u16(hdr+7*2, (gbm_u16) gbm->h);	/* Height in pixels */

	data += stride * (gbm->h-1);

	switch ( gbm->bpp )
		{
/*...s1  \45\ simple case\44\ just 8 word header then data:16:*/
case 1:
	{
	int y, dy;
	gbm_u8 *line, *enc;
	gbm_boolean invert;

	if ( pal )
		{
		put_u16(hdr+1*2, (gbm_u16) (11+6));	/* Header length */
		memcpy(hdr+8*2, "XIMG", 4);		/* Signiture */
		put_u16(hdr+10*2, 0);			/* RGB */
		if ( gbm_file_write(fd, hdr, 11*2) != 11*2 )
			return GBM_ERR_WRITE;
		if ( !write_pal(fd, gbmrgb, 2) )
			return GBM_ERR_WRITE;
		invert = GBM_FALSE;
		}
	else
		{
		if ( gbm_file_write(fd, hdr, 8*2) != 8*2 )
			return GBM_ERR_WRITE;
	 	invert = ( gbmrgb[0].r+gbmrgb[0].g+gbmrgb[0].b <
			   gbmrgb[1].r+gbmrgb[1].g+gbmrgb[1].b );
		}

	if ( (line = gbmmem_malloc((size_t) scan)) == NULL )
		return GBM_ERR_MEM;

	if ( (enc = gbmmem_malloc((size_t) (scan*2+2))) == NULL )
		{ gbmmem_free(line); return GBM_ERR_MEM; }

	for ( y = gbm->h; y > 0; y -= dy, data -= stride * dy )
		{
		int len_enc;
		const gbm_u8 *p;
		if ( !handle_vrep(fd, data, -stride, y, &dy, scan) )
			{ gbmmem_free(enc); gbmmem_free(line); return GBM_ERR_WRITE; }
		if ( invert )
			{
			int x;
			for ( x = 0; x < scan; x++ )
				line[x] = ~data[x];
			p = line;
			}
		else
			p = data;
		len_enc = literally(p, scan, enc, encode1(p, scan, enc));
		if ( gbm_file_write(fd, enc, len_enc) != len_enc )
			{ gbmmem_free(enc); gbmmem_free(line); return GBM_ERR_WRITE; }
		}

	gbmmem_free(enc);
	gbmmem_free(line);

	}
	break;
/*...e*/
/*...s4  \45\ map to colour palette\44\ or to greyscale\44\ or XIMG it:16:*/
case 4:
	{
	int y, dy;
	gbm_u8 *line, *mapped, *enc;
	gbm_u8 map[0x100];

	if ( pal )
		{
		put_u16(hdr+1*2, (gbm_u16) (11+0x30));	/* Header length */
		memcpy(hdr+8*2, "XIMG", 4);		/* Signiture */
		put_u16(hdr+10*2, 0);			/* RGB */
		if ( gbm_file_write(fd, hdr, 11*2) != 11*2 )
			return GBM_ERR_WRITE;
		if ( !write_pal(fd, gbmrgb, 0x10) )
			return GBM_ERR_WRITE;
		}
	else if ( grey )
		{
		unsigned int i;
		put_u16(hdr+1*2, (gbm_u16) 9);		/* Header length */
		put_u16(hdr+8*2, (gbm_u16) 1);		/* Greyscale data */
		if ( gbm_file_write(fd, hdr, 9*2) != 9*2 )
			return GBM_ERR_WRITE;
		for ( i = 0; i < 0x100; i++ )
			map[i] = (map_to_grey4(gbmrgb+((i>>4)&15U))<<4) |
				  map_to_grey4(gbmrgb+( i    &15U))     ;
		}
	else
		{
		unsigned int i;
		if ( gbm_file_write(fd, hdr, 8*2) != 8*2 )
			return GBM_ERR_WRITE;
		for ( i = 0; i < 0x100; i++ )
			map[i] = (map_to_col4(gbmrgb+((i>>4)&15U))<<4) |
				  map_to_col4(gbmrgb+( i    &15U))     ;
		}

	if ( (mapped = gbmmem_malloc((size_t) ((gbm->w+1)/2))) == NULL )
		return GBM_ERR_MEM;

	if ( (line = gbmmem_malloc((size_t) scan)) == NULL )
		{ gbmmem_free(mapped); return GBM_ERR_MEM; }

	if ( (enc = gbmmem_malloc((size_t) (scan*2+2))) == NULL )
		{ gbmmem_free(line); gbmmem_free(mapped); return GBM_ERR_MEM; }

	for ( y = gbm->h; y > 0; y -= dy, data -= dy*stride )
		{
		int x, len_enc, plane;
		const gbm_u8 *p;
		gbm_u8 bit, mask;
		if ( !handle_vrep(fd, data, -stride, y, &dy, gbm->w) )
			{ gbmmem_free(enc); gbmmem_free(line); gbmmem_free(mapped); return GBM_ERR_WRITE; }
		if ( pal )
			p = data;
		else
			{
			for ( x = 0; x < (gbm->w+1)/2; x++ )
				mapped[x] = map[data[x]];
			p = mapped;
			}
		for ( plane = 0, bit = 0x11; plane < 4; plane++, bit <<= 1 )
			{
			memset(line, 0, scan);
			for ( x = 0, mask = 0xf0; x < gbm->w; x++, mask ^= 0xff )
				if ( p[(unsigned)x>>1]&(bit&mask) )
					line[(unsigned)x>>3] |= (0x80>>((unsigned)x&7));
			len_enc = literally(line, scan, enc, encode1(line, scan, enc));
			if ( gbm_file_write(fd, enc, len_enc) != len_enc )
				{ gbmmem_free(enc); gbmmem_free(line); gbmmem_free(mapped); return GBM_ERR_MEM; }
			}
		}

	gbmmem_free(enc);
	gbmmem_free(line);
	gbmmem_free(mapped);

	}
	break;
/*...e*/
/*...s8  \45\ map to greyscale\44\ or XIMG it:16:*/
case 8:
	{
	int y, dy;
	gbm_u8 *line, *mapped, *enc;
	gbm_u8 map[0x100];

	if ( pal )
		{
		put_u16(hdr+1*2, (gbm_u16) (11+0x300));	/* Header length */
		memcpy(hdr+8*2, "XIMG", 4);		/* Signiture */
		put_u16(hdr+10*2, 0);			/* RGB */
		if ( gbm_file_write(fd, hdr, 11*2) != 11*2 )
			return GBM_ERR_WRITE;
		if ( !write_pal(fd, gbmrgb, 0x100) )
			return GBM_ERR_WRITE;
		}
	else
		{
		int i;
		put_u16(hdr+1*2, (gbm_u16) 9);		/* Header length */
		put_u16(hdr+8*2, (gbm_u16) 1);		/* Greyscale data */
		if ( gbm_file_write(fd, hdr, 9*2) != 9*2 )
			return GBM_ERR_WRITE;
		for ( i = 0; i < 0x100; i++ )
			map[i] = (gbm_u8) ( ( (gbm_u16) gbmrgb[i].r+
					    (gbm_u16) gbmrgb[i].g+
					    (gbm_u16) gbmrgb[i].b) / 3 );
		}

	if ( (mapped = gbmmem_malloc((size_t) gbm->w)) == NULL )
		return GBM_ERR_MEM;

	if ( (line = gbmmem_malloc((size_t) scan)) == NULL )
		{ gbmmem_free(mapped); return GBM_ERR_MEM; }

	if ( (enc = gbmmem_malloc((size_t) (scan*2+2))) == NULL )
		{ gbmmem_free(line); gbmmem_free(mapped); return GBM_ERR_MEM; }

	for ( y = gbm->h; y > 0; y -= dy, data -= dy*stride )
		{
		int x, len_enc, plane;
		const gbm_u8 *p;
		gbm_u8 bit;
		if ( !handle_vrep(fd, data, -stride, y, &dy, gbm->w) )
			{ gbmmem_free(enc); gbmmem_free(line); gbmmem_free(mapped); return GBM_ERR_WRITE; }
		if ( pal )
			p = data;
		else
			{
			for ( x = 0; x < gbm->w; x++ )
				mapped[x] = map[data[x]];
			p = mapped;
			}
		for ( plane = 0, bit = 0x01; plane < 8; plane++, bit <<= 1 )
			{
			memset(line, 0, scan);
			for ( x = 0; x < gbm->w; x++ )
				if ( p[x]&bit )
					line[(unsigned)x>>3] |= (0x80U>>((unsigned)x&7U));
			len_enc = literally(line, scan, enc, encode1(line, scan, enc));
			if ( gbm_file_write(fd, enc, len_enc) != len_enc )
				{ gbmmem_free(enc); gbmmem_free(line); gbmmem_free(mapped); return GBM_ERR_MEM; }
			}
		}

	gbmmem_free(enc);
	gbmmem_free(line);
	gbmmem_free(mapped);

	}
	break;
/*...e*/
/*...s24 \45\ write raw 24 bpp data in XIMG file:16:*/
case 24:
	{
	int y, dy;
	gbm_u8 *line, *enc;

	put_u16(hdr+1*2, (gbm_u16) 11);	/* Header length */
	put_u16(hdr+3*2, (gbm_u16) 3);	/* Pattern length */
	memcpy(hdr+8*2, "XIMG", 4);	/* Signiture */
	put_u16(hdr+10*2, (gbm_u16) 0);	/* RGB */
	if ( gbm_file_write(fd, hdr, 11*2) != 11*2 )
		return GBM_ERR_WRITE;

	if ( (line = gbmmem_malloc((size_t) (gbm->w*3))) == NULL )
		return GBM_ERR_MEM;

	if ( (enc = gbmmem_malloc((size_t) (gbm->w*3*2+2))) == NULL )
		{ gbmmem_free(line); return GBM_ERR_MEM; }

	for ( y = gbm->h; y > 0; y -= dy, data -= dy*stride )
		{
		int len_enc;
		if ( !handle_vrep(fd, data, -stride, y, &dy, gbm->w*3) )
			{ gbmmem_free(enc); gbmmem_free(line); return GBM_ERR_WRITE; }
		rgb_bgr(data, line, gbm->w);
		len_enc = literally(line, gbm->w*3, enc, encode3(line, gbm->w*3, enc));
		if ( gbm_file_write(fd, enc, len_enc) != len_enc )
			{ gbmmem_free(enc); gbmmem_free(line); return GBM_ERR_WRITE; }
		}

	gbmmem_free(enc);
	gbmmem_free(line);

	}
	break;
/*...e*/
		}

	return GBM_ERR_OK;
	}
/*...e*/
/*...sgem_err:0:*/
const char *gem_err(GBM_ERR rc)
	{
	switch ( (int) rc )
		{
		case GBM_ERR_GEM_IS_DVI:
			return "not GEM Raster, probably green channel of an Intel DVI image";
		case GBM_ERR_GEM_IS_IMDS:
			return "not GEM Raster, probably IMDS image";
		case GBM_ERR_GEM_BAD_VERSION:
			return "version number not 1";
		case GBM_ERR_GEM_BAD_HDRLEN:
			return "header length must be 8 words or more";
		case GBM_ERR_GEM_BAD_PLANES:
			return "number of planes not in range 1 to 8, or 24";
		case GBM_ERR_GEM_BAD_FLAG:
			return "flag not 0 or 1";
		case GBM_ERR_GEM_FIXEDCPAL:
			return "number of planes in colour image must be 1,3 or 4";
		case GBM_ERR_GEM_XIMG_TYPE:
			return "palette type not 0 in XIMG header";
		}
	return NULL;
	}
/*...e*/
