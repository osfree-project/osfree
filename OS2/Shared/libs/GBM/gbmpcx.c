/*

gbmpcx.c - ZSoft PC Paintbrush support

Reads and writes 1,4,8 and 24 bit colour files.

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

/*...vgbm\46\h:0:*/
/*...vgbmhelp\46\h:0:*/
/*...e*/

/*...suseful:0:*/
#define	low_byte(w)	((byte)  (          (w)&0x00ffU)    )
#define	high_byte(w)	((byte) (((unsigned)(w)&0xff00U)>>8))
#define	make_word(a,b)	(((word)a) + (((word)b) << 8))
/*...e*/

static GBMFT pcx_gbmft =
	{
        GBM_FMT_DESC_SHORT_PCX,
        GBM_FMT_DESC_LONG_PCX,
        GBM_FMT_DESC_EXT_PCX,
	GBM_FT_R1|GBM_FT_R4|GBM_FT_R8|GBM_FT_R24|
	GBM_FT_W1|GBM_FT_W4|GBM_FT_W8|GBM_FT_W24,
	};

#define	GBM_ERR_PCX_BAD_VERSION	((GBM_ERR) 700)
#define	GBM_ERR_PCX_BAD_ENCMODE	((GBM_ERR) 701)
#define	GBM_ERR_PCX_BAD_BITS	((GBM_ERR) 702)
#define	GBM_ERR_PCX_BAD_TRAILER	((GBM_ERR) 703)

typedef struct
	{
	byte version, bpppp, planes;
	int bytes_per_line;
	BOOLEAN	trunc;
	} PCX_PRIV;

/*...spcx_qft:0:*/
GBM_ERR pcx_qft(GBMFT *gbmft)
	{
	*gbmft = pcx_gbmft;
	return GBM_ERR_OK;
	}
/*...e*/
/*...spcx_rhdr:0:*/
GBM_ERR pcx_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
	{
	PCX_PRIV *pcx_priv = (PCX_PRIV *) gbm->priv;
	byte hdr[70];
	word x1, y1, x2, y2;
	int w, h, bpp;

	fn=fn; /* Suppress 'unref arg' compiler warning */

	pcx_priv->trunc = ( gbm_find_word(opt, "trunc" ) != NULL );
	
	gbm_file_read(fd, hdr, 70);
	if ( hdr[0] != 0x0a )
		return GBM_ERR_BAD_MAGIC;
	pcx_priv->version = hdr[1];
	if ( pcx_priv->version == 4 || pcx_priv->version > 5 )
		return GBM_ERR_PCX_BAD_VERSION;
	if ( hdr[2] != 1 )
		return GBM_ERR_PCX_BAD_ENCMODE;

	pcx_priv->bpppp = hdr[3]; pcx_priv->planes = hdr[65];
#define	SWITCH2(a,b)	(((a)<<8)|(b))
	switch ( SWITCH2(pcx_priv->bpppp, pcx_priv->planes) )
		{
		case SWITCH2(1,1): bpp =  1; break;
		case SWITCH2(4,1): bpp =  4; break;
		case SWITCH2(8,1): bpp =  8; break;
		case SWITCH2(8,3): bpp = 24; break; /* Extended 24 bit style */
		case SWITCH2(1,4): bpp =  4; break; /* EGA RGBI style */
		default: return GBM_ERR_PCX_BAD_BITS;
		}

	x1 = make_word(hdr[ 4], hdr[ 5]);
	y1 = make_word(hdr[ 6], hdr[ 7]);
	x2 = make_word(hdr[ 8], hdr[ 9]);
	y2 = make_word(hdr[10], hdr[11]);

	w = x2 - x1 + 1;
	h = y2 - y1 + 1;

	if ( w <= 0 || h <= 0 )
		return GBM_ERR_BAD_SIZE;

	pcx_priv->bytes_per_line = make_word(hdr[66], hdr[67]);

	gbm->w   = w;
	gbm->h   = h;
	gbm->bpp = bpp;

	return GBM_ERR_OK;
	}
/*...e*/
/*...spcx_rpal:0:*/
GBM_ERR pcx_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
	{
	switch ( gbm->bpp )
		{
/*...s1 \45\ fixed b\47\w palette:16:*/
case 1:
	gbmrgb[0].r = gbmrgb[0].g = gbmrgb[0].b = 0x00;
	gbmrgb[1].r = gbmrgb[1].g = gbmrgb[1].b = 0xff;
	break;
/*...e*/
/*...s4 \45\ read palette if 1 plane\44\ fix one if 4 plane RGBI:16:*/
case 4:
	/* Use inline palette */
	{
	byte b[16*3];
	int i;

	gbm_file_lseek(fd, 16L, GBM_SEEK_SET);
	gbm_file_read(fd, b, 16 * 3);
	for ( i = 0; i < 16; i++ )
		{
		gbmrgb[i].r = b[i * 3 + 0];
		gbmrgb[i].g = b[i * 3 + 1];
		gbmrgb[i].b = b[i * 3 + 2];
		}
	}
	break;
/*...e*/
/*...s8 \45\ read palette from end of file:16:*/
case 8:
	{
	byte trailer_id;
	byte b[0x100*3];
	int i;

	gbm_file_lseek(fd, -0x301L, GBM_SEEK_END);
	gbm_file_read(fd, &trailer_id, 1);
	if ( trailer_id != 0x0c )
		return GBM_ERR_PCX_BAD_TRAILER;

	gbm_file_read(fd, b, 0x100 * 3);
	for ( i = 0; i < 0x100; i++ )
		{
		gbmrgb[i].r = b[i * 3 + 0];
		gbmrgb[i].g = b[i * 3 + 1];
		gbmrgb[i].b = b[i * 3 + 2];
		}
	}
	break;
/*...e*/
		}
	return GBM_ERR_OK;
	}
/*...e*/
/*...spcx_rdata:0:*/
/*...sread_pcx_line:0:*/
static void read_pcx_line(
	AHEAD *ahead, byte *line, int bytes_per_line,
	byte *runleft, byte *runval
	)
	{
	/* Handle left overs from previous line */

	while ( *runleft > 0 && bytes_per_line > 0 )
		{
		*line++ = *runval;
		(*runleft)--;
		bytes_per_line--;
		}

	/* Normal code */

	while ( bytes_per_line )
		{
		byte b1 = (byte) gbm_read_ahead(ahead);

		if ( (b1 & 0xc0) == 0xc0 )
			{
			byte b2 = (byte) gbm_read_ahead(ahead);

			b1 &= 0x3f;
			if ( b1 > bytes_per_line )
				{
				(*runleft) = (byte) (b1 - bytes_per_line);
				(*runval) = b2;
				b1 = bytes_per_line;
				}
			memset(line, b2, b1);
			line += b1;
			bytes_per_line -= b1;
			}
		else
			{
			*line++ = b1;
			bytes_per_line--;
			}
		}
	}
/*...e*/
/*...sspread:0:*/
static void spread(byte b, byte bit_to_set, byte *dest)
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

GBM_ERR pcx_rdata(int fd, GBM *gbm, byte *data)
	{
	PCX_PRIV *pcx_priv = (PCX_PRIV *) gbm->priv;
	BOOLEAN trunc = pcx_priv->trunc;
	int bytes_per_line = pcx_priv->bytes_per_line;
	int stride, y;
	byte *line;
	AHEAD *ahead;
	byte runleft = 0, runval;

	if ( (ahead = gbm_create_ahead(fd)) == NULL )
		return GBM_ERR_MEM;

	gbm_file_lseek(fd, 128L, GBM_SEEK_SET);

	if ( (line = malloc((size_t) bytes_per_line)) == NULL )
		{
		gbm_destroy_ahead(ahead);
		return GBM_ERR_MEM;
		}

	switch ( gbm->bpp )
		{
/*...s1:16:*/
case 1:
	stride = ((gbm->w + 31) / 32) * 4;
	for ( y = gbm->h - 1; y >= 0; y-- )
		{
		read_pcx_line(ahead, data + y * stride, bytes_per_line, &runleft, &runval);
		if ( trunc )
			runleft = 0;
		}
	break;
/*...e*/
/*...s4:16:*/
case 4:
	stride = ((gbm->w * 4 + 31) / 32) * 4;
	if ( pcx_priv->planes == 1 )
		for ( y = gbm->h - 1; y >= 0; y-- )
			{
			read_pcx_line(ahead, data + y * stride, bytes_per_line, &runleft, &runval);
			if ( trunc )
				runleft = 0;
			}
	else
		{
		int p, x;
		int bytes = (gbm->w / 8);
		int bits  = (gbm->w & 7);

		memset(data, 0, gbm->h * stride);
		for ( y = gbm->h - 1; y >= 0; y-- )
			for ( p = 0x11; p <= 0x88 ; p <<= 1 )
				{
				byte *dest = data + y * stride;

				read_pcx_line(ahead, line, bytes_per_line, &runleft, &runval);
				if ( trunc )
					runleft = 0;
				for ( x = 0; x < bytes; x++, dest += 4 )
					spread(line[x], (byte) p, dest);
				if ( bits )
					spread((byte) (line[x] & (0xff00U >> bits)), (byte) p, dest);
				}
		}
	break;
/*...e*/
/*...s8:16:*/
case 8:
	stride = ((gbm->w + 3) & ~3);
	for ( y = gbm->h - 1; y >= 0; y-- )
		{
		read_pcx_line(ahead, data + y * stride, bytes_per_line, &runleft, &runval);
		if ( trunc )
			runleft = 0;
		}
	break;
/*...e*/
/*...s24:16:*/
case 24:
	{
	int	p, x;

	stride = ((gbm->w * 3 + 3) & ~3);
	for ( y = gbm->h - 1; y >= 0; y-- )
		for ( p = 2; p >= 0; p-- )
			{
			read_pcx_line(ahead, line, bytes_per_line, &runleft, &runval);
			if ( trunc )
				runleft = 0;
			for ( x = 0; x < gbm->w; x++ )
				data[y * stride + p + x * 3] = line[x];
			}
	}
	break;
/*...e*/
		}

	free(line);

	gbm_destroy_ahead(ahead);

	return GBM_ERR_OK;
	}
/*...e*/
/*...spcx_w:0:*/
/*...sbright:0:*/
static int bright(const GBMRGB *gbmrgb)
	{
	return gbmrgb->r*30+gbmrgb->g*60+gbmrgb->b*10;
	}
/*...e*/
/*...spcx_rle:0:*/
static byte pcx_run(const byte *src, int n_src)
	{
	byte cnt = 1;
	byte b = *src++;

	--n_src;
	while ( cnt < 0x3f && n_src > 0 && *src == b )
		{ cnt++; n_src--; src++; }

	return cnt;
	}

static void pcx_rle(const byte *src, int n_src, byte *dst, int *n_dst)
	{
	*n_dst = 0;	
	while ( n_src )
		{
		byte	len;

		if ( (len = pcx_run(src, n_src)) > 1 || (*src & 0xc0) == 0xc0 )
			{
			*dst++ = (byte) (0xc0 | len);
			*dst++ = *src;
			(*n_dst) += 2;
			}
		else
			{
			*dst++ = *src;
			(*n_dst)++;
			}
		src += len;
		n_src -= len;
		}
	}
/*...e*/

GBM_ERR pcx_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt)
	{
	int i, y, stride = ((gbm->bpp * gbm->w + 31) / 32) * 4;
	byte *line;
	byte hdr[128];
	int bytes_per_line, cnt;

	fn=fn; opt=opt; /* Suppress 'unref arg' compiler warning */

	memset(hdr, 0, 128);
	hdr[ 0] = 0x0a;				/* Magic # */
	hdr[ 1] = 5;				/* Version 5 */
	hdr[ 2] = 1;				/* RLE compression */
	hdr[ 3] = (byte) ( ( gbm->bpp == 24 ) ? 8 : gbm->bpp );
						/* Bits per plane */
	hdr[ 4] = low_byte(0);
	hdr[ 5] = high_byte(0);			/* Top left x */
	hdr[ 6] = low_byte(0);
	hdr[ 7] = high_byte(0);			/* Top left y */
	hdr[ 8] = low_byte(gbm->w - 1);
	hdr[ 9] = high_byte(gbm->w - 1);	/* Bottom right x */
	hdr[10] = low_byte(gbm->h - 1);
	hdr[11] = high_byte(gbm->h - 1);	/* Bottom right y */
	hdr[12] = low_byte(0);
	hdr[13] = high_byte(0);			/* Horizontal resolution ??? */
	hdr[14] = low_byte(0);
	hdr[15] = high_byte(0);			/* Vertical resolution ??? */

	if ( gbm->bpp == 4 )
		for ( i = 0; i < 16; i++ )
			{
			hdr[16 + i * 3    ] = gbmrgb[i].r;
			hdr[16 + i * 3 + 1] = gbmrgb[i].g;
			hdr[16 + i * 3 + 2] = gbmrgb[i].b;
			}

	hdr[65] = (byte) ( ( gbm->bpp == 24 ) ? 3 : 1 );
						/* Planes */
	bytes_per_line = (gbm->w * hdr[3] + 7) / 8;
	if ( bytes_per_line & 1 )
		bytes_per_line++;
	hdr[66] = low_byte(bytes_per_line);
	hdr[67] = high_byte(bytes_per_line);
	hdr[68] = 1;				/* Colour or b/w */

	gbm_file_write(fd, hdr, 128);

	if ( (line = malloc((size_t) (bytes_per_line * 2))) == NULL )
		return GBM_ERR_MEM;

	switch ( gbm->bpp )
		{
/*...s1:16:*/
case 1:
	if ( bright(&gbmrgb[0]) > bright(&gbmrgb[1]) )
		/* Need to invert bitmap bits */
		{
		byte *b;
		if ( (b = malloc(bytes_per_line)) == NULL )
			{
			free(line);
			return GBM_ERR_MEM;
			}
		for ( y = gbm->h - 1; y >= 0; y-- )
			{
			int i;
			for ( i = 0; i < bytes_per_line; i++ )
				b[i] = data[y*stride+i] ^ 0xffU;
			pcx_rle(b, bytes_per_line, line, &cnt);
			if ( gbm_file_write(fd, line, cnt) != cnt )
				{
				free(b);
				free(line);
				return GBM_ERR_WRITE;
				}
			}
		free(b);
		break;
		}
	/* Fall through to regular non-invert case */
/*...e*/
/*...s4\44\8:16:*/
case 4:
case 8:
	for ( y = gbm->h - 1; y >= 0; y-- )
		{
		pcx_rle(data + y * stride, bytes_per_line, line, &cnt);
		if ( gbm_file_write(fd, line, cnt) != cnt )
			{
			free(line);
			return GBM_ERR_WRITE;
			}
		}
	break;
/*...e*/
/*...s24:16:*/
case 24:
	{
	byte *line2;
	int p, x;

	if ( (line2 = malloc((size_t) bytes_per_line)) == NULL )
		{
		free(line);
		return GBM_ERR_MEM;
		}

	for ( y = gbm->h - 1; y >= 0; y-- )
		for ( p = 2; p >= 0; p-- )
			{
			const byte *src = data + y * stride;

			for ( x = 0; x < gbm->w; x++ )
				line2[x] = src[x * 3 + p];

			pcx_rle(line2, bytes_per_line, line, &cnt);
			if ( gbm_file_write(fd, line, cnt) != cnt )
				{
				free(line2);
				free(line);
				return GBM_ERR_WRITE;
				}
			}
	free(line2);
	}
	break;
/*...e*/
		}

	free(line);

	if ( gbm->bpp == 8 )
		{
		byte	pal[1 + 0x100 * 3];

		pal[0] = 0x0c;
		for ( i = 0; i < 0x100; i++ )
			{
			pal[i * 3 + 1] = gbmrgb[i].r;
			pal[i * 3 + 2] = gbmrgb[i].g;
			pal[i * 3 + 3] = gbmrgb[i].b;
			}
		gbm_file_write(fd, pal, 1 + 0x100 * 3);
		}

	return GBM_ERR_OK;
	}
/*...e*/
/*...spcx_err:0:*/
const char *pcx_err(GBM_ERR rc)
	{
	switch ( (int) rc )
		{
		case GBM_ERR_PCX_BAD_VERSION:
			return "version number not 4 or 5";
		case GBM_ERR_PCX_BAD_ENCMODE:
			return "encoding mode not 1";
		case GBM_ERR_PCX_BAD_BITS:
			return "unsupported bpp/plane / plane combination";
		case GBM_ERR_PCX_BAD_TRAILER:
			return "corrupt file trailer";
		}
	return NULL;
	}
/*...e*/
