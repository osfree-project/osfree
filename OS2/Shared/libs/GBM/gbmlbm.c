/*

gbmlbm.c - Amiga IFF / ILBM format

BEWARE: This code at present assumes that the red,green and blue planes are
stored in that order within a true 24 bpp IFF file. I have no testcase or
documentation to independantly verify this.

History:
--------
(Heiko Nitzsche)

22-Feb-2006: Move format description strings to gbmdesc.h
23-Jul-2006: Prevent possible buffer overflows in RLE decoder due to badly
             encoded bitmap.
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
/*...e*/

/*...smapping:0:*/
/*...sget_u32:0:*/
static gbm_u32 get_u32(gbm_u8 *b)
	{
	return    (gbm_u32) b[3]         +
	        (((gbm_u32) b[2]) <<  8) +
	        (((gbm_u32) b[1]) << 16) +
	        (((gbm_u32) b[0]) << 24) ;
	}
/*...e*/
/*...sget_u16:0:*/
static gbm_u16 get_u16(gbm_u8 *b)
	{
	return    (gbm_u16) b[1]        +
	        (((gbm_u16) b[0]) << 8) ;
	}
/*...e*/
/*...sput_u32:0:*/
static void put_u32(gbm_u8 *b, gbm_u32 n)
	{
	b[3] = (gbm_u8) n;
	n >>= 8;
	b[2] = (gbm_u8) n;
	n >>= 8;
	b[1] = (gbm_u8) n;
	n >>= 8;
	b[0] = (gbm_u8) n;
	}
/*...e*/
/*...sput_u16:0:*/
static void put_u16(gbm_u8 *b, gbm_u16 n)
	{
	b[1] = (gbm_u8) n;
	n >>= 8;
	b[0] = (gbm_u8) n;
	}
/*...e*/
/*...e*/

static GBMFT lbm_gbmft =
	{
        GBM_FMT_DESC_SHORT_LBM,
        GBM_FMT_DESC_LONG_LBM,
        GBM_FMT_DESC_EXT_LBM,
	GBM_FT_R1|GBM_FT_R4|GBM_FT_R8|GBM_FT_R24|
	GBM_FT_W1|GBM_FT_W4|GBM_FT_W8|GBM_FT_W24,
	};

#define	GBM_ERR_LBM_FORM	((GBM_ERR) 900)
#define	GBM_ERR_LBM_ILBM	((GBM_ERR) 901)
#define	GBM_ERR_LBM_BMHD_2	((GBM_ERR) 902)
#define	GBM_ERR_LBM_BMHD_0	((GBM_ERR) 903)
#define	GBM_ERR_LBM_BMHD_SIZE	((GBM_ERR) 904)
#define	GBM_ERR_LBM_BPP		((GBM_ERR) 905)
#define	GBM_ERR_LBM_CMAP_SIZE	((GBM_ERR) 906)
#define	GBM_ERR_LBM_COMP	((GBM_ERR) 907)
#define	GBM_ERR_LBM_CAMG_SIZE	((GBM_ERR) 908)
#define	GBM_ERR_LBM_SHAM_VER	((GBM_ERR) 909)

typedef struct
	{
	gbm_u8 pal[0x100 * 3];
	gbm_u32 body, size_body;
	gbm_u8 actual_bpp, comp;
	long sham;
	} LBM_PRIV;

#define	CAMG_LACE	0x00000004
#define	CAMG_EHB	0x00000080
#define	CAMG_HAM	0x00000800
#define	CAMG_1000	0x00001000	/* Meaning unknown */
#define	CAMG_4000	0x00004000	/* Meaning unknown */
#define	CAMG_HIRES	0x00008000
#define	CAMG_20000	0x00020000	/* Meaning unknown */

/*...slbm_qft:0:*/
GBM_ERR lbm_qft(GBMFT *gbmft)
	{
	*gbmft = lbm_gbmft;
	return GBM_ERR_OK;
	}
/*...e*/
/*...slbm_rhdr:0:*/
GBM_ERR lbm_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
	{
	LBM_PRIV *priv = (LBM_PRIV *) gbm->priv;
	gbm_u8 b[20];
	int w, h, bpp, actual_size_cmap = 0;
	gbm_boolean	had_bmhd = GBM_FALSE, had_cmap = GBM_FALSE, had_body = GBM_FALSE;
	gbm_u32 camg = 0;

	fn=fn; opt=opt; /* Suppress 'unref arg' compiler warnings */

	if ( gbm_file_read(fd, b, 12) != 12 )
		return GBM_ERR_READ;
	
	if ( memcmp(b, "FORM", 4) )
		return GBM_ERR_LBM_FORM;

	if ( memcmp(b + 8, "ILBM", 4) )
		return GBM_ERR_LBM_ILBM;

	priv->sham = -1L;

	while ( !had_bmhd || !had_cmap || !had_body )
		{
		gbm_u32 size;

		if ( gbm_file_read(fd, b, 8) != 8 )
			return GBM_ERR_READ;
		size = get_u32(b + 4);
		if ( !memcmp(b, "BMHD", 4) )
/*...sbitmap header:24:*/
{
if ( had_bmhd )
	return GBM_ERR_LBM_BMHD_2;

if ( size != 20 )
	return GBM_ERR_LBM_BMHD_SIZE;

if ( gbm_file_read(fd, b, 20) != 20 )
	return GBM_ERR_READ;

priv->comp = b[10];
if ( priv->comp != 0 && priv->comp != 1 )
	/* Expect compression type to be uncomp or RLE */
	return GBM_ERR_LBM_COMP;

w = get_u16(b);
h = get_u16(b + 2);

if ( w < 0 || w > 10000 || h < 0 || h > 10000 )
	return GBM_ERR_BAD_SIZE;

priv->actual_bpp = b[8];

switch ( priv->actual_bpp )
	{
	case 1:
		bpp = 1; break;
	case 2: case 3: case 4:
		bpp = 4; break;
	case 5: case 6: case 7: case 8:
		bpp = 8; break;
	case 24:
		bpp = 24; had_cmap = GBM_TRUE; break;
	default:
		return GBM_ERR_LBM_BPP;
	}

if ( priv->actual_bpp == 6 )
	/* In case no CAMG chunk present */
	/* Assume HAM6, and will probably be right */
	camg = CAMG_HAM;

had_bmhd = GBM_TRUE;
}
/*...e*/
		else if ( !memcmp(b, "CAMG", 4) )
/*...sC\61\ Amiga mode info:24:*/
{
if ( !had_bmhd )
	return GBM_ERR_LBM_BMHD_0;

if ( size != 4 )
	return GBM_ERR_LBM_CAMG_SIZE;

if ( gbm_file_read(fd, b, 4) != 4 )
	return GBM_ERR_READ;

camg = get_u32(b);
}
/*...e*/
		else if ( !memcmp(b, "CMAP", 4) )
/*...scolour map:24:*/
{
if ( !had_bmhd )
	return GBM_ERR_LBM_BMHD_0;
actual_size_cmap = size;
if ( (gbm_u32) gbm_file_read(fd, priv->pal, size) != size )
	return GBM_ERR_READ;
had_cmap = GBM_TRUE;
}
/*...e*/
		else if ( !memcmp(b, "SHAM", 4) )
/*...ssham:24:*/
{
if ( gbm_file_read(fd, b, 2) != 2 )
	return GBM_ERR_READ;

if ( get_u16(b) != 0 )
	return GBM_ERR_LBM_SHAM_VER;

priv->sham = gbm_file_lseek(fd, 0L, GBM_SEEK_CUR);

gbm_file_lseek(fd, ((size - 2 + 1) & ~1), GBM_SEEK_CUR);
}
/*...e*/
		else if ( !memcmp(b, "BODY", 4) )
/*...sbody of data:24:*/
{
if ( !had_bmhd )
	return GBM_ERR_LBM_BMHD_0;

priv->body = (gbm_u32) gbm_file_lseek(fd, 0L, GBM_SEEK_CUR);
priv->size_body = size;
had_body = GBM_TRUE;
}
/*...e*/
		else
			gbm_file_lseek(fd, ((size + 1) & ~1), GBM_SEEK_CUR);
		}

/*...saccount for ehb\44\ ham6 and ham8:8:*/
{
int entrys = ( 1 << priv->actual_bpp );
int size_cmap = entrys * 3;
gbm_boolean ehb = GBM_FALSE, sham = GBM_FALSE, ham6 = GBM_FALSE, ham8 = GBM_FALSE;

if ( priv->sham != -1L )
	sham = GBM_TRUE; /* Allow Sliced HAM mode */
if ( (camg & CAMG_EHB) != 0 && actual_size_cmap * 2 == size_cmap )
	ehb = GBM_TRUE; /* Allow Extra-HalfBrite mode */
else if ( (camg & CAMG_HAM) != 0 && actual_size_cmap == 0x10*3 && size_cmap == 0x40*3 )
	ham6 = GBM_TRUE; /* Allow HAM6 mode */
else if ( (camg & CAMG_HAM) != 0 && actual_size_cmap == 0x40*3 && size_cmap == 0x100*3 )
	ham8 = GBM_TRUE; /* Allow HAM8 mode */
else if ( priv->actual_bpp == 24 )
	; /* Is a real 24 bpp mode */
else if ( actual_size_cmap != size_cmap )
	return GBM_ERR_LBM_CMAP_SIZE;
if ( ehb )
/*...sreplicate palette:16:*/
{
int i;
for ( i = 0; i < actual_size_cmap; i++ )
	priv->pal[actual_size_cmap + i] = (priv->pal[i] >> 1);
}
/*...e*/
else if ( ham6 )
/*...snobble all but top 4 bits of palette entries:16:*/
{
int i;
for ( i = 0; i < 0x10 * 3; i++ )
	priv->pal[i] &= 0xf0;
bpp = 24;
}
/*...e*/
else if ( ham8 || sham )
	bpp = 24;
}
/*...e*/

	gbm->w   = w;
	gbm->h   = h;
	gbm->bpp = bpp;

	return GBM_ERR_OK;
	}
/*...e*/
/*...slbm_rpal:0:*/
GBM_ERR lbm_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
	{
	LBM_PRIV *priv = (LBM_PRIV *) gbm->priv;
	gbm_u8 *p = priv->pal;
	int i, entrys = ( 1 << priv->actual_bpp );

	fd=fd; /* Suppress 'unref arg' compiler warning */

	if ( gbm->bpp == 24 )
		return GBM_ERR_OK;

	for ( i = 0; i < entrys; i++ )
		{
		gbmrgb[i].r = *p++;
		gbmrgb[i].g = *p++;
		gbmrgb[i].b = *p++;
		}

	return GBM_ERR_OK;
	}
/*...e*/
/*...slbm_rdata:0:*/
/*...sget_line:0:*/
/*...sread_line   \45\ compression type 0 \45\ uncompressed:0:*/
static gbm_boolean read_line(AHEAD *ahead, gbm_u8 *dest, int w)
	{
	while ( w-- )
		{
		int b = gbm_read_ahead(ahead);
		if ( b == -1 )
			return GBM_FALSE;
		*dest++ = b;
		}
	return GBM_TRUE;
	}
/*...e*/
/*...sdecode_line \45\ compression type 1 \45\ RLE:0:*/
static gbm_boolean decode_line(AHEAD *ahead, gbm_u8 *dest, int w)
	{
	int x = 0;
	const gbm_u8 * destEnd = dest + w;

	while ( x < w )
		{
		int c = gbm_read_ahead(ahead);

		if ( c == -1 )
			return GBM_FALSE;

		if ( c & 0x80 )
			{
			int cnt = (0x100 - c + 1);
			if (dest + cnt > destEnd)
				{
				cnt = (destEnd - dest > 0) ? 0 : destEnd - dest;
				}
			memset(dest, (gbm_u8) gbm_read_ahead(ahead), cnt);
			x += cnt;
			dest += cnt;
			}
		else
			{
			int cnt = (c + 1);
			if (dest + cnt > destEnd)
				{
				cnt = (destEnd - dest > 0) ? 0 : destEnd - dest;
				}
			x += cnt;
			while ( cnt-- )
				{
				int b = gbm_read_ahead(ahead);
				if ( b == -1 )
					return GBM_FALSE;
				*dest++ = (gbm_u8) b;
				}
			}
		}
	return GBM_TRUE;
	}
/*...e*/

static gbm_boolean get_line(LBM_PRIV *priv, AHEAD *ahead, gbm_u8 *dest, int w)
	{
	switch ( priv->comp )
		{
		case 0:
			return read_line(ahead, dest, w);
		case 1:
			return decode_line(ahead, dest, w);
		}
	return GBM_FALSE; /* Shouldn't get here */
	}
/*...e*/
/*...sget_planes_8:0:*/
static gbm_boolean get_planes_8(
	AHEAD *ahead,
	LBM_PRIV *priv,
	gbm_u8 *buf,
	gbm_u8 *data,
	int w,
	int n_planes
	)
	{
	int plane, p;
	int scan = (((((unsigned)w + 7) >> 3)+1)/2)*2;

	memset(data, 0, w);
	for ( plane = 0, p = 0x01; plane < n_planes; plane++, p <<= 1 )
		{
		int i;

		if ( !get_line(priv, ahead, buf, scan) )
			return GBM_FALSE;
		for ( i = 0; i < w; i++ )
			if ( buf[(unsigned)i >> 3] & (0x80U >> ((unsigned)i & 7U)) )
				data[i] |= p;
		}
	return GBM_TRUE;
	}
/*...e*/
/*...sget_planes_24:0:*/
static gbm_boolean get_planes_24(
	AHEAD *ahead,
	LBM_PRIV *priv,
	gbm_u8 *buf,
	gbm_u8 *data,
	int w
	)
	{
	int plane, p, c, i;
	int scan = (((((unsigned)w + 7) >> 3)+1)/2)*2;

	memset(data, 0, w*3);
	for ( c = 2; c >= 0; c-- )
		for ( plane = 0, p = 0x01; plane < 8; plane++, p <<= 1 )
			{
			if ( !get_line(priv, ahead, buf, scan) )
				return GBM_FALSE;
			for ( i = 0; i < w; i++ )
				if ( buf[(unsigned)i >> 3] & (0x80U >> ((unsigned)i & 7U)) )
					data[c+i*3] |= p;
			}
	return GBM_TRUE;
	}
/*...e*/

GBM_ERR lbm_rdata(int fd, GBM *gbm, gbm_u8 *data)
	{
	LBM_PRIV *priv = (LBM_PRIV *) gbm->priv;
	AHEAD *ahead;
	int stride = ((gbm->w * gbm->bpp + 31) / 32) * 4;
	int scan = ((((unsigned)(gbm->w + 7) >> 3)+1)/2)*2;
	gbm_file_lseek(fd, priv->body, GBM_SEEK_SET);

	if ( (ahead = gbm_create_ahead(fd)) == NULL )
		return GBM_ERR_MEM;

	data += ((gbm->h - 1) * stride);

	switch ( gbm->bpp )
		{
/*...s24:16:*/
case 24:
	{
	gbm_u8 *buf;
	int y;

	if ( (buf = gbmmem_malloc((size_t) scan)) == NULL )
		{
		gbm_destroy_ahead(ahead);
		return GBM_ERR_MEM;
		}

	if ( priv->actual_bpp == 24 )
/*...sreal 24 bpp data case:32:*/
{
for ( y = 0; y < gbm->h; y++, data -= stride )
	if ( !get_planes_24(ahead, priv, buf, data, gbm->w) )
		{
		gbmmem_free(buf);
		gbm_destroy_ahead(ahead);
		return GBM_ERR_READ;
		}
}
/*...e*/
	else
/*...sHAM6\44\ HAM8 or SHAM6:32:*/
{
gbm_u8 *ham, *sham_pals;
int n_sham_pals, sham_inx = 0;

if ( (ham = gbmmem_malloc((size_t) gbm->w)) == NULL )
	{
	gbmmem_free(buf);
	gbm_destroy_ahead(ahead);
	return GBM_ERR_MEM;
	}

if ( priv->sham != -1L )
/*...sread SHAM palettes:40:*/
/* SHAM holds 200 lines of 16 words each with a 0rgb palette entry */
/* If <= 200 lines then one line per palette */
/* Else two lines per palette */

{
n_sham_pals = ( gbm->h < 200 ) ? gbm->h : 200;
if ( (sham_pals = gbmmem_malloc((size_t) (n_sham_pals * 16 * 2))) == NULL )
	{
	gbmmem_free(ham);
	gbmmem_free(buf);
	gbm_destroy_ahead(ahead);
	return GBM_ERR_MEM;
	}

gbm_file_lseek(fd, priv->sham, GBM_SEEK_SET);
if ( gbm_file_read(fd, sham_pals, n_sham_pals * 16 * 2) != n_sham_pals * 16 * 2 )
	{
	gbmmem_free(sham_pals);
	gbmmem_free(ham);
	gbmmem_free(buf);
	gbm_destroy_ahead(ahead);
	return GBM_ERR_READ;
	}
gbm_file_lseek(fd, priv->body, GBM_SEEK_SET);
}
/*...e*/

for ( y = 0; y < gbm->h; y++, data -= stride )
	{
	if ( !get_planes_8(ahead, priv, buf, ham, gbm->w, priv->actual_bpp) )
		{
	 	if ( priv->sham != -1L )
			gbmmem_free(sham_pals);
		gbmmem_free(buf);
		gbmmem_free(ham);
		gbm_destroy_ahead(ahead);
		return GBM_ERR_READ;
		}

	if ( priv->sham != -1L )
/*...sconvert from SHAM6 to 24 bit rgb:48:*/
{
gbm_u8 r = 0, g = 0, b = 0;
int i;
for ( i = 0; i < gbm->w; i++ )
	{
	gbm_u8 val = (ham[i] & 0x0f);
	switch ( ham[i] & 0x30 )
		{
		case 0x00:
			{
			gbm_u16 pal = get_u16(sham_pals + ((sham_inx * 16 + val) * 2));
			r = (gbm_u8) ((pal & 0x0f00U) >> 4);
			g = (gbm_u8)  (pal & 0x00f0U)      ;
			b = (gbm_u8) ((pal & 0x000fU) << 4);
			}
			break;
		case 0x10:
			b = (val << 4);
			break;
		case 0x20:
			r = (val << 4);
			break;
		case 0x30:
			g = (val << 4);
			break;
		}
	data[i * 3    ] = b;
	data[i * 3 + 1] = g;
	data[i * 3 + 2] = r;
	}
if ( gbm->h <= 200 || (y & 1) != 0 )
	if ( ++sham_inx == n_sham_pals )
		sham_inx = 0;
}
/*...e*/
	else if ( priv->actual_bpp == 6 )
/*...sconvert from HAM6 to 24 bit rgb:48:*/
{
gbm_u8 r = 0, g = 0, b = 0;
int i;
for ( i = 0; i < gbm->w; i++ )
	{
	gbm_u8 val = (ham[i] & 0x0f);
	switch ( ham[i] & 0x30 )
		{
		case 0x00:
			r = priv->pal[val * 3    ];
			g = priv->pal[val * 3 + 1];
			b = priv->pal[val * 3 + 2];
			break;
		case 0x10:
			b = (val << 4);
			break;
		case 0x20:
			r = (val << 4);
			break;
		case 0x30:
			g = (val << 4);
			break;
		}
	data[i * 3    ] = b;
	data[i * 3 + 1] = g;
	data[i * 3 + 2] = r;
	}
}
/*...e*/
	else
/*...sconvert from HAM8 to 24 bit rgb:48:*/
{
gbm_u8 r = 0, g = 0, b = 0;
int i;
for ( i = 0; i < gbm->w; i++ )
	{
	gbm_u8 val = (ham[i] & 0x3f);
	switch ( ham[i] & 0xc0 )
		{
		case 0x00:
			r = priv->pal[val * 3    ];
			g = priv->pal[val * 3 + 1];
			b = priv->pal[val * 3 + 2];
			break;
		case 0x40:
			b = ((r & 0x03) | (val << 2));
			break;
		case 0x80:
			r = ((b & 0x03) | (val << 2));
			break;
		case 0xc0:
			g = ((g & 0x03) | (val << 2));
			break;
		}
	data[i * 3    ] = b;
	data[i * 3 + 1] = g;
	data[i * 3 + 2] = r;
	}
}
/*...e*/
	}

if ( priv->sham != -1L )
	gbmmem_free(sham_pals);
gbmmem_free(ham);
}
/*...e*/

	gbmmem_free(buf);
	}
	break;
/*...e*/
/*...s8:16:*/
case 8:
	{
	gbm_u8 *buf;
	int y;

	if ( (buf = gbmmem_malloc((size_t) scan)) == NULL )
		{
		gbm_destroy_ahead(ahead);
		return GBM_ERR_MEM;
		}

	for ( y = 0; y < gbm->h ; y++, data -= stride )
		if ( !get_planes_8(ahead, priv, buf, data, gbm->w, priv->actual_bpp) )
			{
			gbmmem_free(buf);
			gbm_destroy_ahead(ahead);
			return GBM_ERR_READ;
			}

	gbmmem_free(buf);
	}
	break;
/*...e*/
/*...s4:16:*/
case 4:
	{
	gbm_u8 *buf;
	int y;

	if ( (buf = gbmmem_malloc((size_t) scan)) == NULL )
		{
		gbm_destroy_ahead(ahead);
		return GBM_ERR_MEM;
		}

	for ( y = 0; y < gbm->h; y++, data -= stride )
		{
		int plane, p;

		memset(data, 0, stride);
		for ( plane = 0, p = 0x11; plane < priv->actual_bpp; plane++,p <<= 1 )
			{
			int i, mask;

			if ( !get_line(priv, ahead, buf, scan) )
				{
				gbmmem_free(buf);
				gbm_destroy_ahead(ahead);
				return GBM_ERR_READ;
				}
			for ( i = 0, mask = 0xf0; i < gbm->w; i++, mask ^= 0xff )
				if ( buf[(unsigned)i >> 3] & (0x80U >> ((unsigned)i & 7U)) )
					data[(unsigned)i >> 1] |= ((unsigned)p & (unsigned)mask);
			}
		}

	gbmmem_free(buf);
	}
	break;
/*...e*/
/*...s1:16:*/
case 1:
	{
	int y;

	for ( y = 0; y < gbm->h; y++, data -= stride )
		if ( !get_line(priv, ahead, data, scan) )
			{
			gbm_destroy_ahead(ahead);
			return GBM_ERR_READ;
			}
	}
	break;
/*...e*/
		}

	gbm_destroy_ahead(ahead);

	return GBM_ERR_OK;
	}
/*...e*/
/*...slbm_w:0:*/
/*...swrite_bmhd:0:*/
static GBM_ERR write_bmhd(int fd, const GBM *gbm, int bpp, const char *opt)
	{
	gbm_u8 bmhd[8+20];
	int xpos = 0, ypos = 0, transcol = 0, xaspect = 1, yaspect = 1;
	int xscreen = gbm->w, yscreen = gbm->h;

/*...soutput options:8:*/
{
const char *s;

if ( (s = gbm_find_word_prefix(opt, "xpos=")) != NULL )
	sscanf(s + 5, "%d", &xpos);

if ( (s = gbm_find_word_prefix(opt, "ypos=")) != NULL )
	sscanf(s + 5, "%d", &ypos);

if ( (s = gbm_find_word_prefix(opt, "transcol=")) != NULL )
	sscanf(s + 9, "%d", &transcol);

if ( (s = gbm_find_word_prefix(opt, "xaspect=")) != NULL )
	sscanf(s + 8, "%d", &xaspect);

if ( (s = gbm_find_word_prefix(opt, "yaspect=")) != NULL )
	sscanf(s + 8, "%d", &yaspect);

if ( (s = gbm_find_word_prefix(opt, "xscreen=")) != NULL )
	sscanf(s + 8, "%d", &xscreen);

if ( (s = gbm_find_word_prefix(opt, "yscreen=")) != NULL )
	sscanf(s + 8, "%d", &yscreen);
}
/*...e*/

	memcpy(bmhd, "BMHD", 4);
	put_u32(bmhd+4, (gbm_u32) 20);
	put_u16(bmhd+8, (gbm_u16) gbm->w);
	put_u16(bmhd+8+2, (gbm_u16) gbm->h);
	put_u16(bmhd+8+4, (gbm_u16) xpos);
	put_u16(bmhd+8+6, (gbm_u16) ypos);
	bmhd[8+8] = (gbm_u8) bpp;

	bmhd[8+9] = 0;			/* Masking 0=None, 1=Mask, 2=Transparent, 3=Lasso */
	bmhd[8+10] = 1;			/* Compression, 0=None, 1=RLE */
	bmhd[8+11] = 0;			/* Unused */
	put_u16(bmhd+8+12, (gbm_u16) transcol); /* Transparent colour */
	bmhd[8+14] = (gbm_u8) xaspect;	/* X Aspect (often 10) */
	bmhd[8+15] = (gbm_u8) yaspect;	/* Y Aspect (often 11) */

	put_u16(bmhd+8+16, (gbm_u16) xscreen);	/* Screen width */
	put_u16(bmhd+8+18, (gbm_u16) yscreen);	/* Screen height */

	if ( gbm_file_write(fd, bmhd, 8+20) != 8+20 )
		return GBM_ERR_WRITE;

	return GBM_ERR_OK;
	}
/*...e*/
/*...swrite_camg:0:*/
static GBM_ERR write_camg(int fd, gbm_u32 camg_value)
	{
	gbm_u8 camg[8+4];

	memcpy(camg, "CAMG", 4);
	put_u32(camg+4, (gbm_u32) 4);
	put_u32(camg+8, camg_value);

	if ( gbm_file_write(fd, camg, 8+4) != 8+4 )
		return GBM_ERR_WRITE;

	return GBM_ERR_OK;
	}
/*...e*/
/*...swrite_cmap:0:*/
static GBM_ERR write_cmap(int fd, const GBMRGB *gbmrgb, int bpp)
	{
	gbm_u8 cmap[8+0x100*3];
	int i, entrys = ( 1 << bpp );
	int size_cmap = 3 * entrys;

	memcpy(cmap, "CMAP", 4);
	put_u32(cmap+4, (gbm_u32) size_cmap);
	for ( i = 0; i < entrys; i++ )
		{
		cmap[8+i*3+0] = gbmrgb[i].r;
		cmap[8+i*3+1] = gbmrgb[i].g;
		cmap[8+i*3+2] = gbmrgb[i].b;
		}

	if ( gbm_file_write(fd, cmap, 8 + size_cmap) != 8 + size_cmap )
		return GBM_ERR_WRITE;

	return GBM_ERR_OK;
	}
/*...e*/
/*...swrite_body:0:*/
/*...slbm_rle:0:*/
/*...slbm_run:0:*/
static gbm_u8 lbm_run(const gbm_u8 *src, int n_src)
	{
	gbm_u8 cnt = 1;
	gbm_u8 b = *src++;

	--n_src;
	while ( cnt < 0x81 && n_src > 0 && *src == b )
		{ cnt++; n_src--; src++; }

	return cnt;
	}
/*...e*/
/*...slbm_lit:0:*/
static gbm_u8 lbm_lit(const gbm_u8 *src, int n_src)
	{
	gbm_u8 cnt = 1;

	++src; --n_src;
	while ( cnt < 0x80 && n_src > 0 && *src != src[-1] )
		{ cnt++; n_src--; src++; }

	return cnt;
	}
/*...e*/

static void lbm_rle(const gbm_u8 *src, int n_src, gbm_u8 *dst, int *n_dst)
	{
	*n_dst = 0;	
	while ( n_src )
		{
		gbm_u8	len;

		if ( (len = lbm_run(src, n_src)) > 1 )
			{
			*dst++ = (gbm_u8) (0x100 - (len - 1));
			*dst++ = *src;
			(*n_dst) += 2;
			}
		else
			{
			len = lbm_lit(src, n_src);
			*dst++ = (gbm_u8) (len - 1);
			memcpy(dst, src, len);
			dst += len;
			(*n_dst) += ( 1 + len );
			}
		src += len;
		n_src -= len;
		}
	}
/*...e*/

static GBM_ERR write_body(int fd, const GBM *gbm, int bpp, int n_planes, const gbm_u8 *data, long *end)
	{
	int scan = ((((gbm->w + 7) / 8)+1)/2)*2;
	int stride = ((gbm->w * bpp + 31) / 32) * 4;
	gbm_u8 *comp;
	int n_comp;
	gbm_u8 body[8];
	long offset_body, offset_end;

	memcpy(body, "BODY", 4);
	/* body[4..7] will be filled in later */

	if ( gbm_file_write(fd, body, 8) != 8 )
		return GBM_ERR_WRITE;

	offset_body = gbm_file_lseek(fd, 0L, GBM_SEEK_CUR);

	data += ( gbm->h - 1 ) * stride;

	if ( (comp = gbmmem_malloc((size_t) (scan * 3))) == NULL )
		return GBM_ERR_MEM;

	switch ( bpp )
		{
/*...s24:16:*/
case 24:
	{
	int y, c, p, plane, j;
	gbm_u8 *buf;

	if ( (buf = gbmmem_malloc((size_t) scan)) == NULL )
		{
		gbmmem_free(comp);
		return GBM_ERR_MEM;
		}

	for ( y = 0; y < gbm->h; y++, data -= stride )
		for ( c = 2; c >= 0; c-- )
			for ( plane = 0, p = 0x01; plane < 8; plane++, p <<= 1 )
				{
				memset(buf, 0, scan);
				for ( j = 0; j < gbm->w; j++ )
					if ( data[c+j*3] & p )
						buf[(unsigned)j >> 3] |= (0x80U >> ((unsigned)j & 7U));
				lbm_rle(buf, scan, comp, &n_comp);
				if ( gbm_file_write(fd, comp, n_comp) != n_comp )
					{
					gbmmem_free(buf);
					gbmmem_free(comp);
					return GBM_ERR_WRITE;
					}
				}
	gbmmem_free(buf);
	}
	break;
/*...e*/
/*...s8:16:*/
case 8:
	{
	int y, p, plane, j;
	gbm_u8 *buf;

	if ( (buf = gbmmem_malloc((size_t) scan)) == NULL )
		{
		gbmmem_free(comp);
		return GBM_ERR_MEM;
		}

	for ( y = 0; y < gbm->h; y++, data -= stride )
		for ( plane = 0, p = 0x01; plane < n_planes; plane++, p <<= 1 )
			{
			memset(buf, 0, scan);
			for ( j = 0; j < gbm->w; j++ )
				if ( data[j] & p )
					buf[(unsigned)j >> 3] |= (0x80U >> ((unsigned)j & 7U));
			lbm_rle(buf, scan, comp, &n_comp);
			if ( gbm_file_write(fd, comp, n_comp) != n_comp )
				{
				gbmmem_free(buf);
				gbmmem_free(comp);
				return GBM_ERR_WRITE;
				}
			}
	gbmmem_free(buf);
	}
	break;
/*...e*/
/*...s4:16:*/
case 4:
	{
	int y, p, j, mask;
	gbm_u8 *buf;

	if ( (buf = gbmmem_malloc((size_t) scan)) == NULL )
		{
		gbmmem_free(comp);
		return GBM_ERR_MEM;
		}

	for ( y = 0; y < gbm->h; y++, data -= stride )
		for ( p = 0x11; p <= 0x88; p <<= 1 )
			{
			memset(buf, 0, scan);
			for ( j = 0, mask = 0xf0; j < gbm->w; j++, mask ^= 0xff )
				if ( data[(unsigned)j >> 1] & ((unsigned)p & (unsigned)mask) )
					buf[(unsigned)j >> 3] |= (0x80U >> ((unsigned)j & 7U));
			lbm_rle(buf, scan, comp, &n_comp);
			if ( gbm_file_write(fd, comp, n_comp) != n_comp )
				{
				gbmmem_free(buf);
				gbmmem_free(comp);
				return GBM_ERR_WRITE;
				}
			}
	gbmmem_free(buf);
	}
	break;
/*...e*/
/*...s1:16:*/
case 1:
	{
	int y;

	for ( y = 0; y < gbm->h; y++, data -= stride )
		{
		lbm_rle(data, scan, comp, &n_comp);
		if ( gbm_file_write(fd, comp, n_comp) != n_comp )
			{
			gbmmem_free(comp);
			return GBM_ERR_WRITE;
			}
		}
	}
	break;
/*...e*/
		}

	gbmmem_free(comp);

	offset_end = gbm_file_lseek(fd, 0L, GBM_SEEK_CUR);

	if ( (offset_end - offset_body) & 1 )
		/* BODY is an odd # of bytes long! oops, better 'fix' this */
		{
		gbm_u8 b = 0; /* Pad byte must be zero */
		if ( gbm_file_write(fd, &b, 1) != 1 )
			return GBM_ERR_WRITE;
		offset_end++;
		}

	put_u32(body + 4, offset_end - offset_body);
	gbm_file_lseek(fd, offset_body - 4L, GBM_SEEK_SET);
	if ( gbm_file_write(fd, body + 4, 4) != 4 )
		return GBM_ERR_WRITE;

	*end = offset_end;

	return GBM_ERR_OK;
	}
/*...e*/

static gbm_u8 posdiff[0x100];
#define	absdiff(a,b) posdiff[(gbm_u8)((a)-(b))]
/*...slbm_build_abstab:0:*/
static void lbm_build_abstab(void)
	{
	int i;

	for ( i = 0; i < 0x80; i++ )
		posdiff[i] = (gbm_u8) i;
	for ( i = 1; i <= 0x80; i++ )
		posdiff[0x100 - i] = (gbm_u8) i;
	}
/*...e*/
/*...slbm_ham6:0:*/
/*
We will assume a simple greyscale palette, to make life easy.
Palette entries 0 to 15 will be 00, 11, 22, 33, ... ff.
Start the line with a close grey.
Try to get as many components right first time.
Then for each pixel, pick colours to get as many components right each go.
*/

static void lbm_ham6(const gbm_u8 *data, gbm_u8 *ham, int n)
	{
	if ( n-- > 0 )
		{
		gbm_u8 cb = (*data++ >> 4);
		gbm_u8 cg = (*data++ >> 4);
		gbm_u8 cr = (*data++ >> 4);

		switch ( (cr==cg?4:0) + (cg==cb?2:0) + (cb==cr?1:0) )
			{
/*...s0 \45\ none same:24:*/
case 0:
	*ham++ = cr = cg = cb = ( cr + cg + cb ) / 3;
	break;
/*...e*/
/*...s1 \45\ r\61\b:24:*/
case 1:
	*ham++ = cg = cr;
	break;
/*...e*/
/*...s2 \45\ g\61\b:24:*/
case 2:
	*ham++ = cr = cg;
	break;
/*...e*/
/*...s4 \45\ r\61\g:24:*/
case 4:
	*ham++ = cb = cr;
	break;
/*...e*/
/*...s7 \45\ r\61\g\61\b:24:*/
case 7:
	*ham++ = cr;
	break;
/*...e*/
			}

		while ( n-- > 0 )
			{
			gbm_u8 b = (*data++ >> 4);
			gbm_u8 g = (*data++ >> 4);
			gbm_u8 r = (*data++ >> 4);

			switch ( ((cr==r)?4:0) + ((cg==g)?2:0) + ((cb==b)?1:0) )
				{
/*...s0 \45\ none same:32:*/
/*
3 colour components need modifying.
Can take upto 3 pixels to get the desired colour.
Picking a grey might fix up 2 or more colour components.
Pick r,g or b modify to get as close as possible.
*/

case 0:
	if ( ((r==g)?1:0) + ((g==b)?1:0) + ((b==r)?1:0) > 0 )
		{ *ham++ = cr = cg = cb = ( (r==g) ? r : b ); }
	else if ( absdiff(cr,r) >= absdiff(cg,g) && absdiff(cr,r) >= absdiff(cb,b) )
		{ *ham++ = (0x20 | r); cr = r; }
	else if ( absdiff(cg,g) >= absdiff(cr,r) && absdiff(cg,g) >= absdiff(cb,b) )
 		{ *ham++ = (0x30 | g); cg = g; }
	else
		{ *ham++ = (0x10 | b); cb = b; }
	break;
/*...e*/
/*...s1 \45\ b same:32:*/
/*
2 colour components need modifying.
Can take upto 2 pixels to get the desired colour.
Picking a grey might fix up both colour components.
Pick r or g modify to get as close as possible.
After this, we expect to have <= 1 colour components in error.
*/

case 1:
	if ( r == g && g == b )
		{ *ham++ = cr = cg = b; }
	else if ( absdiff(cr,r) >= absdiff(cg,g) )
		{ *ham++ = (0x20 | r); cr = r; }
	else
		{ *ham++ = (0x30 | g); cg = g; }
	break;
/*...e*/
/*...s2 \45\ g same:32:*/
/*
2 colour components need modifying.
Can take upto 2 pixels to get the desired colour.
Picking a grey might fix up both colour components.
Pick r or b modify to get as close as possible.
After this, we expect to have <= 1 colour components in error.
*/

case 2:
	if ( r == g && g == b )
		{ *ham++ = cr = cb = g; }
	else if ( absdiff(cr,r) >= absdiff(cb,b) )
		{ *ham++ = (0x20 | r); cr = r; }
	else
		{ *ham++ = (0x10 | b); cb = b; }
	break;
/*...e*/
/*...s3 \45\ g\44\b same:32:*/
case 3:
	*ham++ = (0x20 | r); cr = r;
	break;
/*...e*/
/*...s4 \45\ r same:32:*/
/*
2 colour components need modifying.
Can take upto 2 pixels to get the desired colour.
Picking a grey might fix up both colour components.
Pick g or b modify to get as close as possible.
After this, we expect to have <= 1 colour components in error.
*/

case 4:
	if ( r == g && g == b )
		{ *ham++ = cg = cb = r; }
	else if ( absdiff(cg,g) >= absdiff(cb,b) )
		{ *ham++ = (0x30 | g); cg = g; }
	else
		{ *ham++ = (0x10 | b); cb = b; }
	break;
/*...e*/
/*...s5 \45\ r\44\b same:32:*/
case 5:
	*ham++ = (0x30 | g); cg = g;
	break;
/*...e*/
/*...s6 \45\ r\44\g same:32:*/
case 6:
	*ham++ = (0x10 | b); cb = b;
	break;
/*...e*/
/*...s7 \45\ r\44\g\44\b same:32:*/
case 7:
	*ham++ = (0x10 | b);
	break;
/*...e*/
				}
			}
		}
	}
/*...e*/

GBM_ERR lbm_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
	{
	gbm_u8 formilbm[8+4];
	long end;
	GBM_ERR	rc;
	gbm_boolean ham6 = ( gbm_find_word(opt, "ham6") != NULL );

	fn=fn; /* Suppress 'unref args' compiler warning */

	memcpy(formilbm, "FORM", 4);
	/* formilbm[4..7] will be filled in later */
	memcpy(formilbm+8, "ILBM", 4);
	if ( gbm_file_write(fd, formilbm, 8+4) != 8+4 )
		return GBM_ERR_WRITE;

	if ( gbm->bpp == 24 )
		{
		if ( ham6 )
/*...smap to ham6 write bmhd\44\ camg\44\ cmap and body:24:*/
{
gbm_u8 *ham;
GBMRGB gbmrgb_grey[0x10];
int stride8 = ((gbm->w + 3) & ~3);
int stride24 = ((gbm->w * 3 + 3) & ~3);
int i;

if ( (rc = write_bmhd(fd, gbm, 6, opt)) != GBM_ERR_OK )
	return rc;

if ( (rc = write_camg(fd, CAMG_HAM)) != GBM_ERR_OK )
	return rc;

for ( i = 0; i <= 0x10; i++ )
	gbmrgb_grey[i].r = gbmrgb_grey[i].g = gbmrgb_grey[i].b = (gbm_u8) (i * 0x11);

if ( (rc = write_cmap(fd, gbmrgb_grey, 4)) != GBM_ERR_OK )
	return rc;

if ( (ham = gbmmem_malloc((size_t) (stride8 * gbm->h))) == NULL )
	return GBM_ERR_MEM;

lbm_build_abstab();
for ( i = 0; i < gbm->h; i++ )
	lbm_ham6(data + i * stride24, ham + i * stride8, gbm->w);

if ( (rc = write_body(fd, gbm, 8, 6, ham, &end)) != GBM_ERR_OK )
	{
	gbmmem_free(ham);
	return rc;
	}

gbmmem_free(ham);
}
/*...e*/
		else
/*...swrite bmhd and body:24:*/
{
if ( (rc = write_bmhd(fd, gbm, gbm->bpp, opt)) != GBM_ERR_OK )
	return rc;
if ( (rc = write_body(fd, gbm, gbm->bpp, gbm->bpp, data, &end)) != GBM_ERR_OK )
	return rc;
}
/*...e*/
		}
	else
/*...swrite bmhd\44\ cmap and body:16:*/
{
if ( (rc = write_bmhd(fd, gbm, gbm->bpp, opt)) != GBM_ERR_OK )
	return rc;

if ( (rc = write_cmap(fd, gbmrgb, gbm->bpp)) != GBM_ERR_OK )
	return rc;

if ( (rc = write_body(fd, gbm, gbm->bpp, gbm->bpp, data, &end)) != GBM_ERR_OK )
	return rc;
}
/*...e*/

	put_u32(formilbm + 4, end - 8L);
	gbm_file_lseek(fd, 4L, GBM_SEEK_SET);
	if ( gbm_file_write(fd, formilbm + 4, 4) != 4 )
		return GBM_ERR_WRITE;

	return GBM_ERR_OK;
	}
/*...e*/
/*...slbm_err:0:*/
const char *lbm_err(GBM_ERR rc)
	{
	switch ( (int) rc )
		{
		case GBM_ERR_LBM_FORM:
			return "no FORM signiture";
		case GBM_ERR_LBM_ILBM:
			return "no ILBM signiture";
		case GBM_ERR_LBM_BMHD_2:
			return "multiple BMHD bitmap headers";
		case GBM_ERR_LBM_BMHD_0:
			return "no BMHD bitmap header";
		case GBM_ERR_LBM_BMHD_SIZE:
			return "bad size of BMHD bitmap header";
		case GBM_ERR_LBM_BPP:
			return "unsupported/bad bits per pixel";
		case GBM_ERR_LBM_CMAP_SIZE:
			return "CMAP colour map is the wrong size";
		case GBM_ERR_LBM_COMP:
			return "compression type not uncompressed nor RLE";
		case GBM_ERR_LBM_CAMG_SIZE:
			return "CAMG chunk is the wrong size";
		case GBM_ERR_LBM_SHAM_VER:
			return "Unsupported version of SHAM (not 0)";
		}
	return NULL;
	}
/*...e*/
