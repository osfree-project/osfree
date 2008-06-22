/*

gbmvid.c - YUV12C M-Motion Video Frame Buffer format

Reads and writes 24 bit RGB.

History:
--------
(Heiko Nitzsche)

22-Feb-2006: Move format description strings to gbmdesc.h

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
/*...e*/

/*...suseful:0:*/
#define	low_byte(w)	((byte)  (          (w)&0x00ff)    )
#define	high_byte(w)	((byte) (((unsigned)(w)&0xff00)>>8))
#define	make_word(a,b)	(((word)a) + (((word)b) << 8))
/*...e*/

static GBMFT vid_gbmft =
	{
        GBM_FMT_DESC_SHORT_VID,
        GBM_FMT_DESC_LONG_VID,
        GBM_FMT_DESC_EXT_VID,
	GBM_FT_R24|GBM_FT_W24,
	};

#define	GBM_ERR_VID_BAD_W	((GBM_ERR) 1000)

/*...svid_qft:0:*/
GBM_ERR vid_qft(GBMFT *gbmft)
	{
	*gbmft = vid_gbmft;
	return GBM_ERR_OK;
	}
/*...e*/
/*...svid_rhdr:0:*/
GBM_ERR vid_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
	{
	int w, h;
	byte buf[16];

	fn=fn; opt=opt; /* Suppress 'unref arg' compiler warnings */

	if ( gbm_file_read(fd, buf, 16) != 16 )
		return GBM_ERR_READ;

	if ( memcmp(buf, "YUV12C", 6) )
		return GBM_ERR_BAD_MAGIC;

	w = make_word(buf[12], buf[13]);
	h = make_word(buf[14], buf[15]);

	if ( w & 3 )
		return GBM_ERR_VID_BAD_W;

	if ( w <= 0 || h <= 0 )
		return GBM_ERR_BAD_SIZE;

	gbm->w   = w;
	gbm->h   = h;
	gbm->bpp = 24;

	return GBM_ERR_OK;
	}
/*...e*/
/*...svid_rpal:0:*/
GBM_ERR vid_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
	{
	fd=fd; gbm=gbm; gbmrgb=gbmrgb; /* Suppress 'unref arg' compiler warnings */

	return GBM_ERR_OK;
	}
/*...e*/
/*...svid_rdata:0:*/
/*...sdata tables:0:*/
static short tab1[] =
	{
	0,2,5,8,11,14,17,20,
	22,25,28,31,34,37,40,42,
	45,48,51,54,57,60,62,65,
	68,71,74,77,80,82,85,88,
	91,94,97,100,102,105,108,111,
	114,117,120,122,125,128,131,134,
	137,140,142,145,148,151,154,157,
	160,163,165,168,171,174,177,180,
	-183,-180,-177,-174,-171,-168,-165,-163,
	-160,-157,-154,-151,-148,-145,-142,-140,
	-137,-134,-131,-128,-125,-122,-120,-117,
	-114,-111,-108,-105,-102,-100,-97,-94,
	-91,-88,-85,-82,-80,-77,-74,-71,
	-68,-65,-62,-60,-57,-54,-51,-48,
	-45,-42,-40,-37,-34,-31,-28,-25,
	-22,-20,-17,-14,-11,-8,-5,-2,
	};

static short tab2[] =
	{
	0,0,-1,-1,-2,-2,-3,-3,
	-4,-4,-5,-5,-6,-7,-7,-8,
	-8,-9,-9,-10,-10,-11,-11,-12,
	-13,-13,-14,-14,-15,-15,-16,-16,
	-17,-17,-18,-19,-19,-20,-20,-21,
	-21,-22,-22,-23,-23,-24,-24,-25,
	-26,-26,-27,-27,-28,-28,-29,-29,
	-30,-30,-31,-32,-32,-33,-33,-34,
	34,34,33,33,32,32,31,30,
	30,29,29,28,28,27,27,26,
	26,25,24,24,23,23,22,22,
	21,21,20,20,19,19,18,17,
	17,16,16,15,15,14,14,13,
	13,12,11,11,10,10,9,9,
	8,8,7,7,6,5,5,4,
	4,3,3,2,2,1,1,0,
	};

static short tab3[] =
	{
	0,-1,-2,-3,-4,-5,-6,-8,
	-9,-10,-11,-12,-13,-14,-16,-17,
	-18,-19,-20,-21,-23,-24,-25,-26,
	-27,-28,-29,-31,-32,-33,-34,-35,
	-36,-37,-39,-40,-41,-42,-43,-44,
	-46,-47,-48,-49,-50,-51,-52,-54,
	-55,-56,-57,-58,-59,-60,-62,-63,
	-64,-65,-66,-67,-69,-70,-71,-72,
	73,72,71,70,69,67,66,65,
	64,63,62,60,59,58,57,56,
	55,54,52,51,50,49,48,47,
	46,44,43,42,41,40,39,37,
	36,35,34,33,32,31,29,28,
	27,26,25,24,23,21,20,19,
	18,17,16,14,13,12,11,10,
	9,8,6,5,4,3,2,1,
	};

static short tab4[] =
	{
	0,2,4,6,9,11,13,15,
	18,20,22,24,27,29,31,33,
	36,38,40,42,45,47,49,51,
	54,56,58,60,63,65,67,69,
	72,74,76,78,81,83,85,87,
	90,92,94,96,99,101,103,105,
	108,110,112,115,117,119,121,124,
	126,128,130,133,135,137,139,142,
	-144,-142,-139,-137,-135,-133,-130,-128,
	-126,-124,-121,-119,-117,-115,-112,-110,
	-108,-105,-103,-101,-99,-96,-94,-92,
	-90,-87,-85,-83,-81,-78,-76,-74,
	-72,-69,-67,-65,-63,-60,-58,-56,
	-54,-51,-49,-47,-45,-42,-40,-38,
	-36,-33,-31,-29,-27,-24,-22,-20,
	-18,-15,-13,-11,-9,-6,-4,-2,
	};

#ifdef NEVER
static void make_tables(void)
	{
	int	i;
	signed char j;

	for ( i = 0, j = 0; i < 0x80; i++, j += 2 )
		{
#ifdef NEVER
		/* Version 1 */
		tab1[i] = (short) ( 1.7874 * j);
		tab2[i] = (short) (-0.3396 * j);
		tab3[i] = (short) (-0.7188 * j);
		tab4[i] = (short) ( 1.4094 * j);
#endif
		/* Version 2, 80% Saturation */
		tab1[i] = (short) ( 1.4299 * j);
		tab2[i] = (short) (-0.2717 * j);
		tab3[i] = (short) (-0.5751 * j);
		tab4[i] = (short) ( 1.1275 * j);
		}
	}
#endif
/*...e*/
/*...sdecode_sites:0:*/
/*

Supplied wordwise:

Which pixel:	    00000000     11111111    2222 2222    33333333
What comp:	uuvvyyyyyyyguuvv yyyyyyyguuvvyyyy yyygu_v_yyyyyyyg
Bit posn:	76767654321 5454 7654321 32327654 321 1 1 7654321

Supplied bytewise:

Which pixel:	0000         0000     2222 11111111 33333333 2222
What comp:	yyyguuvv uuvvyyyy uuvvyyyy yyyyyyyg yyyyyyyg yyygu_v_
Bit posn:	321 5454 76767654 32327654 7654321  7654321  321 1 1

Desired:

Which pixel:	00000000 11111111 22222222 33333333
What comp:	yyyyyyy_ yyyyyyy_ yyyyyyy_ yyyyyyy_ uuuuuuu_ vvvvvvv_

*/

static void decode_sites(byte *src, byte *dest, int n_sites)
	{
	for ( ; n_sites--; src += 6 )
		{
		*dest++ = (byte) ( ((byte) (src[0] & 0xe0)>>4) |
				   ((byte) (src[1] & 0x0f)<<4) );
		*dest++ = (byte)    (byte) (src[3] & 0xfe)      ;
		*dest++ = (byte) ( ((byte) (src[2] & 0x0f)<<4) |
				   ((byte) (src[5] & 0xe0)>>4) );
		*dest++ = (byte)    (byte) (src[4] & 0xfe)      ;
		*dest++ = (byte) ( ((byte) (src[0] & 0x0c)<<2) |
				    (byte) (src[1] & 0xc0)     |
				   ((byte) (src[2] & 0xc0)>>4) |
				   ((byte) (src[5] & 0x08)>>2) );
		*dest++ = (byte) ( ((byte) (src[0] & 0x03)<<4) |
				   ((byte) (src[1] & 0x30)<<2) |
				   ((byte) (src[2] & 0x30)>>2) |
				    (byte) (src[5] & 0x02)     );
		}
	}
/*...e*/
/*...sconvert_sites:0:*/
/*...sVersion1:0:*/
/*

Site has Y1 Y2 Y3 Y4 U V.

Input is of the form Y1 Y2 Y3 Y4 U V

R = ( V * 179 / 127 ) + Y

B = ( U * 227 / 127 ) + Y

    170 * Y - 51 * R - 19 * B
G = -------------------------
              100

Rewrite to give better optimisation ...

R = Y + 1.4094 * V

B = Y + 1.7874 * U

G = Y - 0.7188 * V - 0.3396 * U

*/
/*...e*/
/*...sVersion2\44\ 80\37\ saturation \40\as recommended\41\:0:*/
/*

Site has Y1 Y2 Y3 Y4 U V.

Input is of the form Y1 Y2 Y3 Y4 U V

R = 0.8 * ( V * 179 / 127 ) + Y

B = 0.8 * ( U * 227 / 127 ) + Y

    170 * Y - 51 * R - 19 * B
G = -------------------------
              100

Rewrite to give better optimisation ...

R = Y + 1.1275 * V

B = Y + 1.4299 * U

G = Y - 0.5751 * V - 0.2717 * U

*/
/*...e*/

static void convert_sites(byte *src, byte *dest, int n_sites)
	{
	for ( ; n_sites--; src += 6 )
		{
		byte u   = (src[4] >> 1);
		byte v   = (src[5] >> 1);
		short tu = tab1[u];
		short ts = tab2[u] + tab3[v];
		short tv = tab4[v];
		int p;

		for ( p = 0; p < 4; p++ )
			{
			short b = (short) ((unsigned short) src[p]) + tu;
			short g = (short) ((unsigned short) src[p]) + ts;
			short r = (short) ((unsigned short) src[p]) + tv;

			if ( b < 0 ) b = 0; else if ( b > 0xff ) b = 0xff;
			if ( g < 0 ) g = 0; else if ( g > 0xff ) g = 0xff;
			if ( r < 0 ) r = 0; else if ( r > 0xff ) r = 0xff;

			*dest++ = (byte) b;
			*dest++ = (byte) g;
			*dest++ = (byte) r;
			}
		}
	}
/*...e*/

GBM_ERR vid_rdata(int fd, GBM *gbm, byte *data)
	{
	int stride = ((gbm->w * 3 + 3) & ~3);
	int n_sites = gbm->w / 4;
	byte *ibuffer, *sbuffer, *p;
	int i;

	if ( (ibuffer = gbmmem_malloc((size_t) (n_sites * 6))) == NULL )
		return GBM_ERR_MEM;

	if ( (sbuffer = gbmmem_malloc((size_t) (n_sites * 6))) == NULL )
		{
		gbmmem_free(ibuffer);
		return GBM_ERR_MEM;
		}

	gbm_file_lseek(fd, 16L, GBM_SEEK_SET);

	p = data + ((gbm->h - 1) * stride);
	for ( i = gbm->h - 1; i >= 0; i-- )
		{
		if ( gbm_file_read(fd, (char *) ibuffer, n_sites * 6) != n_sites * 6 )
			{
			gbmmem_free(sbuffer);
			gbmmem_free(ibuffer);
			return GBM_ERR_READ;
			}
		decode_sites(ibuffer, sbuffer, n_sites);
		convert_sites(sbuffer, p, n_sites);
		p -= stride;
		}
	gbmmem_free(sbuffer);
	gbmmem_free(ibuffer);
	return GBM_ERR_OK;
	}
/*...e*/
/*...svid_w:0:*/
/*...sconvert_to_yuv:0:*/
/*...sVersion1:0:*/
/*

Y = 51 * R + 100 * G + 19 * B
    -------------------------
             170

U = ( B - Y ) * 127 / 227

V = ( R - Y ) * 127 / 179

*/
/*...e*/
/*...sVersion2\44\ 80\37\ saturation \40\as recommended\41\:0:*/
/*

Y = 51 * R + 100 * G + 19 * B
    -------------------------
             170

U = (1 / 0.8) * ( B - Y ) * 127 / 227

V = (1 / 0.8) * ( R - Y ) * 127 / 179

*/
/*...e*/

static void convert_to_yuv(const byte *src, byte *dest, int n_sites)
	{
	while ( n_sites-- )
		{
		short u = 0, v = 0;
		int i;

		for ( i = 0; i < 4; i++ )
			{
			byte b = *src++;
			byte g = *src++;
			byte r = *src++;
			short y = ( 51 * r + 100 * g + 19 * b ) / 170;

			u += ( (b - y) * (127 * 5) / (227 * 4) );
			v += ( (r - y) * (127 * 5) / (179 * 4) );

			if ( y < 0 ) y = 0; else if ( y > 0xff ) y = 0xff;

			*dest++ = (byte) (y & 0xfe);
			}

		u >>= 2; v >>= 2;

		if ( u < -0x80 ) u = -0x80; else if ( u > 0x7f ) u = 0x7f;
		if ( v < -0x80 ) v = -0x80; else if ( v > 0x7f ) v = 0x7f;

		*dest++ = (byte) (u & 0xfe);
		*dest++ = (byte) (v & 0xfe);
		}
	}
/*...e*/
/*...sencode_sites:0:*/
/*

Supplied:

Which pixel:	00000000 11111111 22222222 33333333
What comp:	yyyyyyy_ yyyyyyy_ yyyyyyy_ yyyyyyy_ uuuuuuu_ vvvvvvv_

Desired bytewise:

Which pixel:	0000         0000     2222 11111111 33333333 2222
What comp:	yyyguuvv uuvvyyyy uuvvyyyy yyyyyyyg yyyyyyyg yyygu_v_
Bit posn:	321 5454 76767654 32327654 7654321  7654321  321 1 1

*/

static void encode_sites(const byte *src, byte *dest, int n_sites)
	{
	for ( ; n_sites--; src += 6 )
		{
		*dest++ = (byte) ( ((byte) (src[0] & 0x0e)<<4) |
				   ((byte) (src[4] & 0x30)>>2) |
				   ((byte) (src[5] & 0x30)>>4) );
		*dest++ = (byte) (  (byte) (src[4] & 0xc0)     |
				   ((byte) (src[5] & 0xc0)>>2) |
				   ((byte) (src[0] & 0xf0)>>4) );
		*dest++ = (byte) ( ((byte) (src[4] & 0x0c)<<4) |
				   ((byte) (src[5] & 0x0c)<<2) |
				   ((byte) (src[2] & 0xf0)>>4) );
		*dest++ = (byte)    (byte) (src[1] & 0xfe)      ;
		*dest++ = (byte)    (byte) (src[3] & 0xfe)      ;
		*dest++ = (byte) ( ((byte) (src[2] & 0x0e)<<4) |
				   ((byte) (src[4] & 0x02)<<2) |
				    (byte) (src[5] & 0x02)     );
		}
	}
/*...e*/

GBM_ERR vid_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt)
	{
	byte buf[16];
	int xpos = 0, ypos = 0;
	int stride = ((gbm->w * 3 + 3) & ~3);
	int n_sites = gbm->w / 4;
	byte *obuffer, *sbuffer;
	const byte *p;
	int i;
	const char *s;

	fn=fn; gbmrgb=gbmrgb; /* Suppress 'unref arg' compiler warnings */

	if ( (s = gbm_find_word_prefix(opt, "xpos=")) != NULL )
		sscanf(s + 5, "%d", &xpos);

	if ( (s = gbm_find_word_prefix(opt, "ypos=")) != NULL )
		sscanf(s + 5, "%d", &ypos);

	memcpy(buf, "YUV12C", 6);
	buf[ 6] = 0;
	buf[ 7] = 0;
	buf[ 8] = low_byte(xpos);
	buf[ 9] = high_byte(xpos);
	buf[10] = low_byte(ypos);
	buf[11] = high_byte(ypos);
	buf[12] = low_byte(gbm->w & ~3);
	buf[13] = high_byte(gbm->w & ~3);
	buf[14] = low_byte(gbm->h);
	buf[15] = high_byte(gbm->h);

	if ( gbm_file_write(fd, buf, 16) != 16 )
		return GBM_ERR_WRITE;

	if ( (obuffer = gbmmem_malloc((size_t) (n_sites * 6))) == NULL )
		return GBM_ERR_MEM;

	if ( (sbuffer = gbmmem_malloc((size_t) (n_sites * 6))) == NULL )
		{
		gbmmem_free(obuffer);
		return GBM_ERR_MEM;
		}

	p = data + ((gbm->h - 1) * stride);
	for ( i = gbm->h - 1; i >= 0; i-- )
		{
		convert_to_yuv(p, sbuffer, n_sites);
		encode_sites(sbuffer, obuffer, n_sites);
		if ( gbm_file_write(fd, (char *) obuffer, n_sites * 6) != n_sites * 6 )
			{
			gbmmem_free(sbuffer);
			gbmmem_free(obuffer);
			return GBM_ERR_WRITE;
			}
		p -= stride;
		}
	gbmmem_free(sbuffer);
	gbmmem_free(obuffer);

	return GBM_ERR_OK;
	}
/*...e*/
/*...svid_err:0:*/
const char *vid_err(GBM_ERR rc)
	{
	switch ( (int) rc )
		{
		case GBM_ERR_VID_BAD_W:
			return "width not a multiple of 4 pixels";
		}
	return NULL;
	}
/*...e*/
