/*

gbmrect.c - Subrectangle Transfer

*/

/*...sincludes:0:*/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include "gbm.h"

/*...vgbm\46\h:0:*/
/*...e*/

/*...sgbm_subrectangle:0:*/
/*...smiddle_4:0:*/
static void middle_4(byte *dst, const byte *src, int x, int w)
	{
	if ( x & 1 )
		{
		src += (x / 2);
		for ( ; w >= 2; w -= 2 )
			{
			*dst    = (byte) (*src++ << 4);
			*dst++ |= (byte) (*src   >> 4);
			}
		if ( w )
			*dst = (byte) (*src << 4);
		}
	else
		memcpy(dst, src + x / 2, (w + 1) / 2);
	}
/*...e*/
/*...smiddle_1:0:*/
static void middle_1(byte *dst, const byte *src, int x, int w)
	{
	int	last = (x & 7);

	src += (x/8);
	if ( last )
		{
		for ( ; w >= 8; w -= 8 )
			{
			*dst    = (byte) (*src++ << last);
			*dst++ |= (byte) (*src   >> (8U - last));
			}
		if ( w )
			{
			*dst = (byte) (*src++ << last);
			w -= (8U-last);
			if ( w )
				*dst |= (byte) (*src >> (8U - last));
			}
		}
	else
		memcpy(dst, src, (w + 7) / 8);
	}
/*...e*/

void gbm_subrectangle(
	const GBM *gbm,
	int x, int y, int w, int h,
	const byte *data_src, byte *data_dst
	)
	{
	int i;
	int stride_src = ( ((gbm->w * gbm->bpp + 31)/32) * 4 );
	int stride_dst = ( ((     w * gbm->bpp + 31)/32) * 4 );

	data_src += stride_src * y;

	switch ( gbm->bpp )
		{
		case 64:
			for ( i = 0; i < h; i++, data_src += stride_src, data_dst += stride_dst )
				memcpy(data_dst, data_src + x * 8, w * 8);
			break;
		case 48:
			for ( i = 0; i < h; i++, data_src += stride_src, data_dst += stride_dst )
				memcpy(data_dst, data_src + x * 6, w * 6);
			break;
		case 32:
			for ( i = 0; i < h; i++, data_src += stride_src, data_dst += stride_dst )
				memcpy(data_dst, data_src + x * 4, w * 4);
			break;
		case 24:
			for ( i = 0; i < h; i++, data_src += stride_src, data_dst += stride_dst )
				memcpy(data_dst, data_src + x * 3, w * 3);
			break;
		case 8:
			for ( i = 0; i < h; i++, data_src += stride_src, data_dst += stride_dst )
				memcpy(data_dst, data_src + x, w);
			break;
		case 4:
			for ( i = 0; i < h; i++, data_src += stride_src, data_dst += stride_dst )
				middle_4(data_dst, data_src, x, w);
			break;
		case 1:
			for ( i = 0; i < h; i++, data_src += stride_src, data_dst += stride_dst )
				middle_1(data_dst, data_src, x, w);
			break;
		}
	}
/*...e*/
/*...sgbm_blit:0:*/
/*...sblit_1:0:*/
static void blit_1(
	const byte *s, int sts, int sx,
	      byte *d, int dts, int dx,
	int w, int h
	)
	{
	s += (sx>>3); sx &= 7;
	d += (dx>>3); dx &= 7;

	if ( sx == dx )
/*...saligned transfer:16:*/
{
int left, right, middle = w;
byte left_mask, right_mask;

/* If starting mid-byte, then remember this */
if ( (sx&7)!=0 && middle > 0 )
	{ left = 8-(sx&7); middle -= left; }
else
	left = 0;
/* If ending mid-byte, then remember this */
if ( ((sx+w)&7)!=0 && middle > 0 )
	{ right = ((sx+w)&7); middle -= right; }
else
	right = 0;
/* Remainder will be a multiple of 8, divide by 8 to give bytes */
middle >>= 3;

left_mask  = (byte) (0xffU<<left );
right_mask = (byte) (0xffU>>right);

for ( ; h-- > 0; s += sts, d += dts )
	{
	const byte *sp = s;
	      byte *dp = d;
	if ( left > 0 )
		{ *dp = ( (*dp&left_mask) | (*sp&~left_mask) ); dp++; sp++; }
	memcpy(dp, sp, middle); dp += middle; sp += middle;
	if ( right )
		*dp = ( (*dp&right_mask) | (*sp&~right_mask) );
	}
}
/*...e*/
	else
/*...smisaligned transfer:16:*/
/* This will be very inefficient, but will work */

for ( ; h-- > 0; s += sts, d += dts )
	{
	int x, sxl, dxl;
	for ( x = 0, sxl = sx, dxl = dx; x < w; x++, sxl++, dxl++ )
		if ( s[(unsigned)sxl>>3] & (0x80U>>((unsigned)sxl&7U)) )
			d[(unsigned)dxl>>3] |=  (0x80U>>((unsigned)dxl&7U));
		else
			d[(unsigned)dxl>>3] &= ~(0x80U>>((unsigned)dxl&7U));
	}
/*...e*/
	}
/*...e*/
/*...sblit_4:0:*/
static void blit_4(
	const byte *s, int sts, int sx,
	      byte *d, int dts, int dx,
	int w, int h
	)
	{
	s += ((unsigned)sx>>1); sx &= 1;
	d += ((unsigned)dx>>1); dx &= 1;

	if ( sx == dx )
/*...saligned transfer:16:*/
{
BOOLEAN lnibble, rnibble;
int middle = w;

/* If starting mid-byte, then remember this */
if ( (sx&1)!=0 && middle > 0 )
	{ lnibble = TRUE; middle--; }
else
	lnibble = FALSE;
/* If ending mid-byte, then remember this */
if ( ((sx+w)&1)!=0 && middle > 0 )
	{ rnibble = TRUE; middle--; }
else
	rnibble = FALSE;
/* Remainder will be a multiple of 2, divide by 2 to give bytes */
middle >>= 1;

for ( ; h-- > 0; s += sts, d += dts )
	{
	const byte *sp = s;
	      byte *dp = d;
	if ( lnibble )
		{ *dp = ( (*dp&0xf0) | (*sp&0x0f) ); dp++; sp++; }
	memcpy(dp, sp, middle); dp += middle; sp += middle;
	if ( rnibble )
		*dp = ( (*dp&0x0f) | (*sp&0xf0) );
	}
}
/*...e*/
	else
/*...smisaligned transfer:16:*/
/* This will be quite inefficient, but will work */

for ( ; h-- > 0; s += sts, d += dts )
	{
	int x, sxl, dxl;
	for ( x = 0, sxl = sx, dxl = dx; x < w; x++, sxl++, dxl++ )
		if ( x&1 )
			d[(unsigned)dxl>>1] = ( (d[(unsigned)dxl>>1]&0xf0U) | s[(unsigned)sxl>>1]>>4 );
		else
			d[(unsigned)dxl>>1] = ( (d[(unsigned)dxl>>1]&0x0fU) | s[(unsigned)sxl>>1]<<4 );
	}
/*...e*/
	}
/*...e*/
/*...sblit_8:0:*/
static void blit_8(
	const byte *s, int sts, int sx,
	      byte *d, int dts, int dx,
	int w, int h
	)
	{
	s += sx;
	d += dx;
	for ( ; h-- > 0; s += sts, d += dts )
		memcpy(d, s, w);
	}
/*...e*/
/*...sblit_24:0:*/
static void blit_24(
	const byte *s, int sts, int sx,
	      byte *d, int dts, int dx,
	int w, int h
	)
	{
	s += (sx*3);
	d += (dx*3);
	for ( ; h-- > 0; s += sts, d += dts )
		memcpy(d, s, w*3);
	}
/*...e*/
/*...sblit_32:0:*/
static void blit_32(
	const byte *s, int sts, int sx,
	      byte *d, int dts, int dx,
	int w, int h
	)
	{
	s += (sx*4);
	d += (dx*4);
	for ( ; h-- > 0; s += sts, d += dts )
		memcpy(d, s, w*4);
	}
/*...e*/
/*...sblit_48:0:*/
static void blit_48(
	const byte *s, int sts, int sx,
	      byte *d, int dts, int dx,
	int w, int h
	)
	{
	s += (sx*6);
	d += (dx*6);
	for ( ; h-- > 0; s += sts, d += dts )
		memcpy(d, s, w*6);
	}
/*...e*/
/*...sblit_64:0:*/
static void blit_64(
	const byte *s, int sts, int sx,
	      byte *d, int dts, int dx,
	int w, int h
	)
	{
	s += (sx*8);
	d += (dx*8);
	for ( ; h-- > 0; s += sts, d += dts )
		memcpy(d, s, w*8);
	}
/*...e*/

void gbm_blit(
	const byte *s, int sw, int sx, int sy,
	      byte *d, int dw, int dx, int dy,
	int w, int h,
	int bpp
	)
	{
	int sts = ((sw * bpp + 31) / 32) * 4;
	int dts = ((dw * bpp + 31) / 32) * 4;

	if ( w == 0 || h == 0 )
		return;

	s += sts * sy;
	d += dts * dy;
	switch ( bpp )
		{
		case 1:		blit_1 (s,sts,sx, d,dts,dx, w,h);	break;
		case 4:		blit_4 (s,sts,sx, d,dts,dx, w,h);	break;
		case 8:		blit_8 (s,sts,sx, d,dts,dx, w,h);	break;
		case 24:	blit_24(s,sts,sx, d,dts,dx, w,h);	break;
		case 32:	blit_32(s,sts,sx, d,dts,dx, w,h);	break;
		case 48:	blit_48(s,sts,sx, d,dts,dx, w,h);	break;
		case 64:	blit_64(s,sts,sx, d,dts,dx, w,h);	break;
		}
	}
/*...e*/
