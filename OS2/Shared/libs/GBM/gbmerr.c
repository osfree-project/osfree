/*

gbmerr.c - Error diffusion Module

*/

/*...sincludes:0:*/
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"

/*...vgbm\46\h:0:*/
/*...e*/
/*...svars:0:*/
static BOOLEAN inited = FALSE;

/*
Tables used for quick saturated addition and subtraction.
*/

static byte usat[256+256+256];
#define	U_SAT_ADD(a,b)	usat[(a)+(b)+256]

static short ssat[256+256+256+256];
#define	S_SAT_ADD(a,b)	ssat[(a)+(b)+512]

/*
For 6Rx6Gx6B, 7Rx8Gx4B, 4Rx4Gx4B palettes etc.
*/

static byte index4[0x100];
static byte index6[0x100];
static byte index7[0x100];
static byte index8[0x100];
static byte index16[0x100];
static byte scale4[] = { 0, 85, 170, 255 };
static byte scale6[] = { 0, 51, 102, 153, 204, 255 };
static byte scale7[] = { 0, 43, 85, 128, 170, 213, 255 };
static byte scale8[] = { 0, 36, 73, 109, 146, 182, 219, 255 };
static byte scale16[] = { 0, 17, 34, 51, 68, 85, 102, 119, 136,
			   153, 170, 187, 204, 221, 238, 255 };

static word randtab[0x100];
static int randinx = 0;
/*...e*/
/*...sinit:0:*/
/*
This function initialises this module.
It does this by preparing the quick saturated addition and subraction tables.
*/

/*...snearest_inx:0:*/
#ifndef abs
#define	abs(x)	(((x)>=0)?(x):-(x))
#endif

static byte nearest_inx(byte value, const byte ab[], unsigned short cb)
	{
	byte b, inx, inx_min;
	short diff, diff_min;

	b = ab[0];
	diff_min = abs((short) value - (short) b);
	inx_min = 0;
	for ( inx = 1; (unsigned short) inx < cb; inx++ )
		{
		b = ab[inx];
		diff = abs((short) value - (short) b);
		if ( diff < diff_min )
			{
			diff_min = diff;
			inx_min = inx;
			}
		}
	return inx_min;
	}
/*...e*/

static void init(void)
	{
	int i;

	if ( inited )
		return;

	memset(usat, 0, 0x100);
	for ( i = 0; i < 0x100; i++ )
		usat[i +  0x100] = (byte) i;
	memset(usat + 0x200, 0xff, 0x100);

	for ( i = -0x200; i < -0x100; i++ )
		ssat[i + 0x200] = -0x100;
	for ( i = -0x100; i < 0x100; i++ )
		ssat[i + 0x200] = i;
	for ( i = 0x100; i < 0x200; i++ )
		ssat[i + 0x200] = 0xff;

	/* For 7 Red x 8 Green x 4 Blue palettes etc. */

	for ( i = 0; i < 0x100; i++ )
		{
		index4 [i] = nearest_inx((byte) i, scale4 , sizeof(scale4 ));
		index6 [i] = nearest_inx((byte) i, scale6 , sizeof(scale6 ));
		index7 [i] = nearest_inx((byte) i, scale7 , sizeof(scale7 ));
		index8 [i] = nearest_inx((byte) i, scale8 , sizeof(scale8 ));
		index16[i] = nearest_inx((byte) i, scale16, sizeof(scale16));
		}

	/* For faster random number calculation */

	for ( i = 0; i < 0x100; i++ )
		randtab [i] = (word) (rand() % (51*0x100));

	inited = TRUE;
	}
/*...e*/
/*...serrdiff:0:*/
static BOOLEAN errdiff(
	const GBM *gbm, const byte *src, byte *dest,
	int dest_bpp,
	void (*errdiff_line)(byte *src, byte *dest, short *errs, int cx)
	)
	{
	int stride_src = ((gbm->w * 3 + 3) & ~3);
	int stride_dest = ((gbm->w * dest_bpp + 31) / 32) * 4;
	byte *buf;
	short *errs;
	int y;

	if ( (buf = malloc((size_t) stride_src + 3)) == NULL )
		return FALSE;

	if ( (errs = malloc((size_t) ((gbm->w + 1) * 3 * sizeof(short)))) == NULL )
		{
		free(buf);
		return FALSE;
		}

	memset(errs, 0, (gbm->w + 1) * 3 * sizeof(short));

	for ( y = 0; y < gbm->h; y++ )
		{
		memcpy(buf, src + y * stride_src, stride_src);
		(*errdiff_line)(buf, dest + y * stride_dest, errs, gbm->w);
		}

	free(buf);
	free(errs);

	return TRUE;
	}
/*...e*/

/*...sgbm_errdiff_line_24     \45\ error diffuse to fewer bits per pixel one line:0:*/
/*
This function takes a line of RGB data, a destination buffer and a buffer of
error terms and performs one lines worth (a given # of pixels) of error
diffusion.
*/

void gbm_errdiff_line_24(byte *src, byte *dest, short *errs, int cx, byte rm, byte gm, byte bm)
	{
	int x, ptr = 0;

	init();

	/* Step 1: Add error terms to newly supplied line */

	for ( x = 0; x < cx * 3; x++ )
		src[x] = U_SAT_ADD((int) src[x], (int) errs[x]);

	/* Step 2: Zero out error terms */

	memset(errs, 0, cx * 3 * sizeof(errs[0]));

	/* Step 3: Go along data, finding nearest colour and propagating error */

	randinx = rand();

	for ( x = 0; x < cx; x++ )
		{
		byte b     = *src++;
		byte g     = *src++;
		byte r     = *src++;
		byte bi    = (b & bm);
		byte gi    = (g & gm);
		byte ri    = (r & rm);
		int  be    = b - (int) bi;
		int  ge    = g - (int) gi;
		int  re    = r - (int) ri;
		word rn    = randtab[(byte) (randinx++)];
		int  right = (rn >> 8);
		int  down  = ((rn & 0xff) % (63 - right));
		int  be1   = ((be * right) >> 6);
		int  ge1   = ((ge * right) >> 6);
		int  re1   = ((re * right) >> 6);
		int  be2   = ((be * down ) >> 6);
		int  ge2   = ((ge * down ) >> 6);
		int  re2   = ((re * down ) >> 6);
		int  be3   = be - be1 - be2;
		int  ge3   = ge - ge1 - ge2;
		int  re3   = re - re1 - re2;

		*dest++ = bi;
		*dest++ = gi;
		*dest++ = ri;

		src[0] = U_SAT_ADD((int) src[0], be1);
		src[1] = U_SAT_ADD((int) src[1], ge1);
		src[2] = U_SAT_ADD((int) src[2], re1);

		errs[ptr    ] = S_SAT_ADD((int) errs[ptr    ], be2);
		errs[ptr + 1] = S_SAT_ADD((int) errs[ptr + 1], ge2);
		errs[ptr + 2] = S_SAT_ADD((int) errs[ptr + 2], re2);

		ptr += 3;

		errs[ptr    ] = S_SAT_ADD((int) errs[ptr    ], be3);
		errs[ptr + 1] = S_SAT_ADD((int) errs[ptr + 1], ge3);
		errs[ptr + 2] = S_SAT_ADD((int) errs[ptr + 2], re3);
		}
	}
/*...e*/
/*...sgbm_errdiff_24          \45\ error diffuse to fewer bits per pixel:0:*/
BOOLEAN	gbm_errdiff_24(const GBM *gbm, const byte *data24, byte *data24a, byte rm, byte gm, byte bm)
	{
	int stride = ((gbm->w * 3 + 3) & ~3);
	byte *buf;
	short *errs;
	int y;

	if ( (buf = malloc((size_t) (stride + 3))) == NULL )
		return FALSE;

	if ( (errs = malloc((size_t) ((gbm->w + 1) * 3 * sizeof(short)))) == NULL )
		{
		free(buf);
		return FALSE;
		}

	memset(errs, 0, (gbm->w + 1) * 3 * sizeof(short));

	for ( y = 0; y < gbm->h; y++ )
		{
		memcpy(buf, data24 + y * stride, stride);
		gbm_errdiff_line_24(buf, data24a + y * stride, errs, gbm->w, rm, gm, bm);
		}

	free(buf);
	free(errs);

	return TRUE;
	}
/*...e*/

/*...sgbm_errdiff_pal_6R6G6B  \45\ return 6Rx6Gx6B palette:0:*/
/*
This function makes the palette for the 6 red x 6 green x 6 blue palette.
216 palette entrys used. Remaining 40 left blank.
*/

void gbm_errdiff_pal_6R6G6B(GBMRGB *gbmrgb)
	{
	byte volatile r;	/* C-Set/2 optimiser fix */
	byte volatile g;
	byte volatile b;

	init();
	memset(gbmrgb, 0x80, 0x100 * sizeof(GBMRGB));
	for ( r = 0; r < 6; r++ )
		for ( g = 0; g < 6; g++ )
			for ( b = 0; b < 6; b++ )
				{
				gbmrgb->r = scale6[r];
				gbmrgb->g = scale6[g];
				gbmrgb->b = scale6[b];
				gbmrgb++;
				}
	}
/*...e*/
/*...sgbm_errdiff_line_6R6G6B \45\ error diffuse to 6Rx6Gx6B one line:0:*/
/*
This function takes a line of RGB data, a destination buffer and a buffer of
error terms and performs one lines worth (a given # of pixels) of error
diffusion.
*/

void gbm_errdiff_line_6R6G6B(byte *src, byte *dest, short *errs, int cx)
	{
	int x, ptr = 0;

	init();

	/* Step 1: Add error terms to newly supplied line */

	for ( x = 0; x < cx * 3; x++ )
		src[x] = U_SAT_ADD((int) src[x], (int) errs[x]);

	/* Step 2: Zero out error terms */

	memset(errs, 0, (cx + 1) * 3 * sizeof(errs[0]));

	/* Step 3: Go along data, finding nearest colour and propagating error */

	randinx = rand();

	for ( x = 0; x < cx; x++ )
		{
		byte b     = *src++;
		byte g     = *src++;
		byte r     = *src++;
		byte bi    = index6[b];
		byte gi    = index6[g];
		byte ri    = index6[r];
		int  be    = b - ((int) scale6[bi]);
		int  ge    = g - ((int) scale6[gi]);
		int  re    = r - ((int) scale6[ri]);
		word rn    = randtab[(byte) (randinx++)];
		int  right = (int) (rn >> 8);
		int  down  = (int) ((rn & 0xff) % (63 - right));
		int  be1   = ((be * right) >> 6);
		int  ge1   = ((ge * right) >> 6);
		int  re1   = ((re * right) >> 6);
		int  be2   = ((be * down ) >> 6);
		int  ge2   = ((ge * down ) >> 6);
		int  re2   = ((re * down ) >> 6);
		int  be3   = be - be1 - be2;
		int  ge3   = ge - ge1 - ge2;
		int  re3   = re - re1 - re2;

		*dest++ = (byte) (6 * (6 * ri + gi) + bi);

		src[0] = U_SAT_ADD((int) src[0], be1);
		src[1] = U_SAT_ADD((int) src[1], ge1);
		src[2] = U_SAT_ADD((int) src[2], re1);

		errs[ptr    ] = S_SAT_ADD((int) errs[ptr    ], be2);
		errs[ptr + 1] = S_SAT_ADD((int) errs[ptr + 1], ge2);
		errs[ptr + 2] = S_SAT_ADD((int) errs[ptr + 2], re2);

		ptr += 3;

		errs[ptr    ] = S_SAT_ADD((int) errs[ptr    ], be3);
		errs[ptr + 1] = S_SAT_ADD((int) errs[ptr + 1], ge3);
		errs[ptr + 2] = S_SAT_ADD((int) errs[ptr + 2], re3);
		}
	}
/*...e*/
/*...sgbm_errdiff_6R6G6B      \45\ error diffuse to 6Rx6Gx6B:0:*/
BOOLEAN	gbm_errdiff_6R6G6B(const GBM *gbm, const byte *data24, byte *data8)
	{
	return errdiff(gbm, data24, data8, 8, gbm_errdiff_line_6R6G6B);
	}
/*...e*/

/*...sgbm_errdiff_pal_7R8G4B  \45\ return 7Rx8Gx4B palette:0:*/
/*
This function makes the palette for the 7 red x 8 green x 4 blue palette.
224 palette entrys used. Remaining 32 left blank.
Colours calculated to match those used by 8514/A PM driver.
*/

void gbm_errdiff_pal_7R8G4B(GBMRGB *gbmrgb)
	{
	byte volatile r;	/* C-Set/2 optimiser fix */
	byte volatile g;
	byte volatile b;

	init();

	memset(gbmrgb, 0x80, 0x100 * sizeof(GBMRGB));
	for ( r = 0; r < 7; r++ )
		for ( g = 0; g < 8; g++ )
			for ( b = 0; b < 4; b++ )
				{
				gbmrgb->r = scale7[r];
				gbmrgb->g = scale8[g];
				gbmrgb->b = scale4[b];
				gbmrgb++;
				}
	}
/*...e*/
/*...sgbm_errdiff_line_7R8G4B \45\ error diffuse to 7Rx8Gx4B one line:0:*/
/*
This function takes a line of RGB data, a destination buffer and a buffer of
error terms and performs one lines worth (a given # of pixels) of error
diffusion.
*/

void gbm_errdiff_line_7R8G4B(byte *src, byte *dest, short *errs, int cx)
	{
	int x, ptr = 0;

	init();

	/* Step 1: Add error terms to newly supplied line */

	for ( x = 0; x < cx * 3; x++ )
		src[x] = U_SAT_ADD((int) src[x], (int) errs[x]);

	/* Step 2: Zero out error terms */

	memset(errs, 0, (cx + 1) * 3 * sizeof(errs[0]));

	/* Step 3: Go along data, finding nearest colour and propagating error */

	randinx = rand();

	for ( x = 0; x < cx; x++ )
		{
		byte b     = *src++;
		byte g     = *src++;
		byte r     = *src++;
		byte bi    = index4[b];
		byte gi    = index8[g];
		byte ri    = index7[r];
		int  be    = b - ((int) scale4[bi]);
		int  ge    = g - ((int) scale8[gi]);
		int  re    = r - ((int) scale7[ri]);
		word rn    = randtab[(byte) (randinx++)];
		int  right = (rn >> 8);
		int  down  = ((rn & 0xff) % (63 - right));
		int  be1   = ((be * right) >> 6);
		int  ge1   = ((ge * right) >> 6);
		int  re1   = ((re * right) >> 6);
		int  be2   = ((be * down ) >> 6);
		int  ge2   = ((ge * down ) >> 6);
		int  re2   = ((re * down ) >> 6);
		int  be3   = be - be1 - be2;
		int  ge3   = ge - ge1 - ge2;
		int  re3   = re - re1 - re2;

		*dest++ = (byte) (4 * (8 * ri + gi) + bi);

		src[0] = U_SAT_ADD((int) src[0], be1);
		src[1] = U_SAT_ADD((int) src[1], ge1);
		src[2] = U_SAT_ADD((int) src[2], re1);

		errs[ptr    ] = S_SAT_ADD((int) errs[ptr    ], be2);
		errs[ptr + 1] = S_SAT_ADD((int) errs[ptr + 1], ge2);
		errs[ptr + 2] = S_SAT_ADD((int) errs[ptr + 2], re2);

		ptr += 3;

		errs[ptr    ] = S_SAT_ADD((int) errs[ptr    ], be3);
		errs[ptr + 1] = S_SAT_ADD((int) errs[ptr + 1], ge3);
		errs[ptr + 2] = S_SAT_ADD((int) errs[ptr + 2], re3);
		}
	}
/*...e*/
/*...sgbm_errdiff_7R8G4B      \45\ error diffuse to 7Rx8Gx4B:0:*/
BOOLEAN	gbm_errdiff_7R8G4B(const GBM *gbm, const byte *data24, byte *data8)
	{
	return errdiff(gbm, data24, data8, 8, gbm_errdiff_line_7R8G4B);
	}
/*...e*/

/*...sgbm_errdiff_pal_4R4G4B  \45\ return 4Rx4Gx4B palette:0:*/
/*
This function makes the palette for the 4 red x 4 green x 4 blue palette.
64 palette entrys used. Remaining 192 left blank.
*/

void gbm_errdiff_pal_4R4G4B(GBMRGB *gbmrgb)
	{
	byte volatile r;	/* C-Set/2 optimiser fix */
	byte volatile g;
	byte volatile b;

	init();
	memset(gbmrgb, 0x80, 0x100 * sizeof(GBMRGB));
	for ( r = 0; r < 4; r++ )
		for ( g = 0; g < 4; g++ )
			for ( b = 0; b < 4; b++ )
				{
				gbmrgb->r = scale4[r];
				gbmrgb->g = scale4[g];
				gbmrgb->b = scale4[b];
				gbmrgb++;
				}
	}
/*...e*/
/*...sgbm_errdiff_line_4R4G4B \45\ error diffuse to 4Rx4Gx4B one line:0:*/
/*
This function takes a line of RGB data, a destination buffer and a buffer of
error terms and performs one lines worth (a given # of pixels) of error
diffusion.
*/

void gbm_errdiff_line_4R4G4B(byte *src, byte *dest, short *errs, int cx)
	{
	int x, ptr = 0;

	init();

	/* Step 1: Add error terms to newly supplied line */

	for ( x = 0; x < cx * 3; x++ )
		src[x] = U_SAT_ADD((int) src[x], (int) errs[x]);

	/* Step 2: Zero out error terms */

	memset(errs, 0, (cx + 1) * 3 * sizeof(errs[0]));

	/* Step 3: Go along data, finding nearest colour and propagating error */

	randinx = rand();

	for ( x = 0; x < cx; x++ )
		{
		byte b     = *src++;
		byte g     = *src++;
		byte r     = *src++;
		byte bi    = index4[b];
		byte gi    = index4[g];
		byte ri    = index4[r];
		int  be    = b - ((int) scale4[bi]);
		int  ge    = g - ((int) scale4[gi]);
		int  re    = r - ((int) scale4[ri]);
		word rn    = randtab[(byte) (randinx++)];
		int  right = (int) (rn >> 8);
		int  down  = (int) ((rn & 0xff) % (63 - right));
		int  be1   = ((be * right) >> 6);
		int  ge1   = ((ge * right) >> 6);
		int  re1   = ((re * right) >> 6);
		int  be2   = ((be * down ) >> 6);
		int  ge2   = ((ge * down ) >> 6);
		int  re2   = ((re * down ) >> 6);
		int  be3   = be - be1 - be2;
		int  ge3   = ge - ge1 - ge2;
		int  re3   = re - re1 - re2;

		*dest++ = (byte) (4 * (4 * ri + gi) + bi);

		src[0] = U_SAT_ADD((int) src[0], be1);
		src[1] = U_SAT_ADD((int) src[1], ge1);
		src[2] = U_SAT_ADD((int) src[2], re1);

		errs[ptr    ] = S_SAT_ADD((int) errs[ptr    ], be2);
		errs[ptr + 1] = S_SAT_ADD((int) errs[ptr + 1], ge2);
		errs[ptr + 2] = S_SAT_ADD((int) errs[ptr + 2], re2);

		ptr += 3;

		errs[ptr    ] = S_SAT_ADD((int) errs[ptr    ], be3);
		errs[ptr + 1] = S_SAT_ADD((int) errs[ptr + 1], ge3);
		errs[ptr + 2] = S_SAT_ADD((int) errs[ptr + 2], re3);
		}
	}
/*...e*/
/*...sgbm_errdiff_4R4G4B      \45\ error diffuse to 4Rx4Gx4B:0:*/
BOOLEAN	gbm_errdiff_4R4G4B(const GBM *gbm, const byte *data24, byte *data8)
	{
	return errdiff(gbm, data24, data8, 8, gbm_errdiff_line_4R4G4B);
	}
/*...e*/

/*...sgbm_errdiff_pal_VGA     \45\ return default VGA palette:0:*/
/*
This function makes the palette for the 16 colour VGA palette.
*/

static GBMRGB gbmrgb_vga[] =
	{
	  0,  0,  0,
	128,  0,  0,
	  0,128,  0,
	128,128,  0,
	  0,  0,128,
	128,  0,128,
	  0,128,128,
	128,128,128,
	204,204,204,
	255,  0,  0,
	  0,255,  0,
	255,255,  0,
	  0,  0,255,
	255,  0,255,
	  0,255,255,
	255,255,255,
	};

void gbm_errdiff_pal_VGA(GBMRGB *gbmrgb)
	{
	init();
	memcpy((char *) gbmrgb, (char *) gbmrgb_vga, sizeof(gbmrgb_vga));
	}
/*...e*/
/*...sgbm_errdiff_line_VGA    \45\ error diffuse to default VGA palette one line:0:*/
/*
This function takes a line of RGB data, a destination buffer and a buffer of
error terms and performs one lines worth (a given # of pixels) of error
diffusion.
*/

/*...scalc_nearest:0:*/
/*
This function, when given am RGB colour, finds the VGA palette entry closest
to it. We deliberately bias away from the two grey palette entries.
*/

static byte calc_nearest(byte r, byte g, byte b)
	{
	long min_dist = 3L * 256L * 256L * 10L;
	byte bi, bi_min;

	for ( bi = 0; bi < 0x10; bi++ )
		{
		long b_dist = ((long) b - (long) gbmrgb_vga[bi].b);
		long g_dist = ((long) g - (long) gbmrgb_vga[bi].g);
		long r_dist = ((long) r - (long) gbmrgb_vga[bi].r);
		long dist = r_dist * r_dist + g_dist * g_dist + b_dist * b_dist;

		if ( bi == 7 || bi == 8 )
			/* Bias away from this colour */
			dist <<= 3;

		if ( dist < min_dist )
			{
			min_dist = dist;
			bi_min = bi;
			}
		}
	return bi_min;
	}
/*...e*/
/*...snearest_colour:0:*/
/*
This function finds the closest VGA palette colour to a given RGB value.
It uses a lookup table to avoid performing distance calculations to the
16 palette entrys. The table is pre-calculated.
*/

/*...squick lookup table:0:*/
/*...v_gbmerr\46\c \45\ used to make quick_tab:0:*/

static byte quick_tab[16][16][16] =
	{
	0,0,0,0,255,4,4,4,4,4,4,4,12,12,12,12,
	0,0,0,0,255,4,4,4,4,4,4,4,12,12,12,12,
	0,0,0,0,255,4,4,4,4,4,4,4,12,12,12,12,
	0,0,0,0,255,4,4,4,4,4,4,4,12,12,12,12,
	255,255,255,255,255,255,255,255,255,255,255,255,255,12,12,12,
	2,2,2,2,255,6,6,6,6,6,6,6,6,255,12,12,
	2,2,2,2,255,6,6,6,6,6,6,6,6,6,255,12,
	2,2,2,2,255,6,6,6,6,6,6,6,6,6,6,255,
	2,2,2,2,255,6,6,6,6,6,6,6,6,6,6,255,
	2,2,2,2,255,6,6,6,6,6,6,6,6,6,255,14,
	2,2,2,2,255,6,6,6,6,6,6,6,6,255,14,14,
	2,2,2,2,255,6,6,6,6,6,6,6,255,14,14,14,
	10,10,10,10,255,6,6,6,6,6,6,255,14,14,14,14,
	10,10,10,10,10,255,6,6,6,6,255,14,14,14,14,14,
	10,10,10,10,10,10,255,6,6,255,14,14,14,14,14,14,
	10,10,10,10,10,10,10,255,255,14,14,14,14,14,14,14,
	0,0,0,0,255,4,4,4,4,4,4,4,12,12,12,12,
	0,0,0,0,255,4,4,4,4,4,4,4,12,12,12,12,
	0,0,0,0,255,4,4,4,4,4,4,4,12,12,12,12,
	0,0,0,0,255,4,4,4,4,4,4,4,12,12,12,12,
	255,255,255,255,255,255,255,255,255,255,255,255,255,12,12,12,
	2,2,2,2,255,6,6,6,6,6,6,6,6,255,12,12,
	2,2,2,2,255,6,6,6,6,6,6,6,6,6,255,12,
	2,2,2,2,255,6,6,6,6,6,6,6,6,6,6,255,
	2,2,2,2,255,6,6,6,6,6,6,6,6,6,6,255,
	2,2,2,2,255,6,6,6,6,6,6,6,6,6,255,14,
	2,2,2,2,255,6,6,6,6,6,6,6,6,255,14,14,
	2,2,2,2,255,6,6,6,6,6,6,6,255,14,14,14,
	10,10,10,10,255,6,6,6,6,6,6,255,14,14,14,14,
	10,10,10,10,10,255,6,6,6,6,255,14,14,14,14,14,
	10,10,10,10,10,10,255,6,6,255,14,14,14,14,14,14,
	10,10,10,10,10,10,10,255,255,14,14,14,14,14,14,14,
	0,0,0,0,255,4,4,4,4,4,4,4,12,12,12,12,
	0,0,0,0,255,4,4,4,4,4,4,4,12,12,12,12,
	0,0,0,0,255,4,4,4,4,4,4,4,12,12,12,12,
	0,0,0,0,255,4,4,4,4,4,4,4,12,12,12,12,
	255,255,255,255,255,255,255,255,255,255,255,255,255,12,12,12,
	2,2,2,2,255,6,6,6,6,6,6,6,6,255,12,12,
	2,2,2,2,255,6,6,6,6,6,6,6,6,6,255,12,
	2,2,2,2,255,6,6,6,6,6,6,6,6,6,6,255,
	2,2,2,2,255,6,6,6,6,6,6,6,6,6,6,255,
	2,2,2,2,255,6,6,6,6,6,6,6,6,6,255,14,
	2,2,2,2,255,6,6,6,6,6,6,6,6,255,14,14,
	2,2,2,2,255,6,6,6,6,6,6,6,255,14,14,14,
	10,10,10,10,255,6,6,6,6,6,6,255,14,14,14,14,
	10,10,10,10,10,255,6,6,6,6,255,14,14,14,14,14,
	10,10,10,10,10,10,255,6,6,255,14,14,14,14,14,14,
	10,10,10,10,10,10,10,255,255,14,14,14,14,14,14,14,
	0,0,0,0,255,4,4,4,4,4,4,4,12,12,12,12,
	0,0,0,0,255,4,4,4,4,4,4,4,12,12,12,12,
	0,0,0,0,255,4,4,4,4,4,4,4,12,12,12,12,
	0,0,0,0,255,4,4,4,4,4,4,4,12,12,12,12,
	255,255,255,255,255,255,255,255,255,255,255,255,255,12,12,12,
	2,2,2,2,255,6,6,6,6,6,6,6,6,255,12,12,
	2,2,2,2,255,6,6,6,6,6,6,6,6,6,255,12,
	2,2,2,2,255,6,6,6,6,6,6,6,6,6,6,255,
	2,2,2,2,255,6,6,6,6,6,6,6,6,6,6,255,
	2,2,2,2,255,6,6,6,6,6,6,6,6,6,255,14,
	2,2,2,2,255,6,6,6,6,6,6,6,6,255,14,14,
	2,2,2,2,255,6,6,6,6,6,6,6,255,14,14,14,
	10,10,10,10,255,6,6,6,6,6,6,255,14,14,14,14,
	10,10,10,10,10,255,6,6,6,6,255,14,14,14,14,14,
	10,10,10,10,10,10,255,6,6,255,14,14,14,14,14,14,
	10,10,10,10,10,10,10,255,255,14,14,14,14,14,14,14,
	255,255,255,255,255,255,255,255,255,255,255,255,255,12,12,12,
	255,255,255,255,255,255,255,255,255,255,255,255,255,12,12,12,
	255,255,255,255,255,255,255,255,255,255,255,255,255,12,12,12,
	255,255,255,255,255,255,255,255,255,255,255,255,255,12,12,12,
	255,255,255,255,255,255,255,255,255,255,255,255,255,12,12,12,
	255,255,255,255,255,6,6,6,6,6,6,6,6,255,12,12,
	255,255,255,255,255,6,6,6,6,6,6,6,6,6,255,12,
	255,255,255,255,255,6,6,6,6,6,6,6,6,6,6,255,
	255,255,255,255,255,6,6,6,6,6,6,6,6,6,6,255,
	255,255,255,255,255,6,6,6,6,6,6,6,6,6,255,14,
	255,255,255,255,255,6,6,6,6,6,6,6,6,255,14,14,
	255,255,255,255,255,6,6,6,6,6,6,6,255,14,14,14,
	255,255,255,255,255,6,6,6,6,6,6,255,14,14,14,14,
	10,10,10,10,10,255,6,6,6,6,255,14,14,14,14,14,
	10,10,10,10,10,10,255,6,6,255,14,14,14,14,14,14,
	10,10,10,10,10,10,10,255,255,14,14,14,14,14,14,14,
	1,1,1,1,255,5,5,5,5,5,5,5,5,255,12,12,
	1,1,1,1,255,5,5,5,5,5,5,5,5,255,12,12,
	1,1,1,1,255,5,5,5,5,5,5,5,5,255,12,12,
	1,1,1,1,255,5,5,5,5,5,5,5,5,255,12,12,
	255,255,255,255,255,5,5,5,5,5,5,5,5,255,12,12,
	3,3,3,3,3,255,255,255,255,255,255,255,255,255,12,12,
	3,3,3,3,3,255,6,6,6,6,6,6,6,6,255,12,
	3,3,3,3,3,255,6,255,255,6,6,6,6,6,6,255,
	3,3,3,3,3,255,6,255,255,6,6,6,6,6,6,255,
	3,3,3,3,3,255,6,6,6,6,6,6,6,6,255,14,
	3,3,3,3,3,255,6,6,6,6,6,6,6,255,14,14,
	3,3,3,3,3,255,6,6,6,6,6,6,255,14,14,14,
	3,3,3,3,3,255,6,6,6,6,6,255,14,14,14,14,
	255,255,255,255,255,255,6,6,6,6,255,14,14,14,14,14,
	10,10,10,10,10,10,255,6,6,255,14,14,14,14,14,14,
	10,10,10,10,10,10,10,255,255,14,14,14,14,14,14,14,
	1,1,1,1,255,5,5,5,5,5,5,5,5,5,255,12,
	1,1,1,1,255,5,5,5,5,5,5,5,5,5,255,12,
	1,1,1,1,255,5,5,5,5,5,5,5,5,5,255,12,
	1,1,1,1,255,5,5,5,5,5,5,5,5,5,255,12,
	255,255,255,255,255,5,5,5,5,5,5,5,5,5,255,12,
	3,3,3,3,3,255,5,5,5,5,5,5,5,5,255,12,
	3,3,3,3,3,3,255,255,255,255,255,255,255,255,255,12,
	3,3,3,3,3,3,255,255,255,255,255,6,6,6,6,255,
	3,3,3,3,3,3,255,255,255,255,255,6,6,6,6,255,
	3,3,3,3,3,3,255,255,255,255,255,6,6,6,255,14,
	3,3,3,3,3,3,255,255,255,255,6,6,6,255,14,14,
	3,3,3,3,3,3,255,6,6,6,6,6,255,14,14,14,
	3,3,3,3,3,3,255,6,6,6,6,255,14,14,14,14,
	3,3,3,3,3,3,255,6,6,6,255,14,14,14,14,14,
	255,255,255,255,255,255,255,6,6,255,14,14,14,14,14,14,
	10,10,10,10,10,10,10,255,255,14,14,14,14,14,14,14,
	1,1,1,1,255,5,5,5,5,5,5,5,5,5,5,255,
	1,1,1,1,255,5,5,5,5,5,5,5,5,5,5,255,
	1,1,1,1,255,5,5,5,5,5,5,5,5,5,5,255,
	1,1,1,1,255,5,5,5,5,5,5,5,5,5,5,255,
	255,255,255,255,255,5,5,5,5,5,5,5,5,5,5,255,
	3,3,3,3,3,255,5,255,255,5,5,5,5,5,5,255,
	3,3,3,3,3,3,255,255,255,255,255,5,5,5,5,255,
	3,3,3,3,3,255,255,7,7,7,255,255,255,255,255,255,
	3,3,3,3,3,255,255,7,7,7,255,6,6,6,6,255,
	3,3,3,3,3,3,255,7,7,255,255,6,6,6,255,14,
	3,3,3,3,3,3,255,255,255,255,255,6,6,255,14,14,
	3,3,3,3,3,3,3,255,6,6,6,6,255,14,14,14,
	3,3,3,3,3,3,3,255,6,6,6,255,14,14,14,14,
	3,3,3,3,3,3,3,255,6,6,255,14,14,14,14,14,
	3,3,3,3,3,3,3,255,6,255,14,14,14,14,14,14,
	255,255,255,255,255,255,255,255,255,14,14,14,14,14,14,14,
	1,1,1,1,255,5,5,5,5,5,5,5,5,5,5,255,
	1,1,1,1,255,5,5,5,5,5,5,5,5,5,5,255,
	1,1,1,1,255,5,5,5,5,5,5,5,5,5,5,255,
	1,1,1,1,255,5,5,5,5,5,5,5,5,5,5,255,
	255,255,255,255,255,5,5,5,5,5,5,5,5,5,5,255,
	3,3,3,3,3,255,5,255,255,5,5,5,5,5,5,255,
	3,3,3,3,3,3,255,255,255,255,255,5,5,5,5,255,
	3,3,3,3,3,255,255,7,7,7,255,5,5,5,5,255,
	3,3,3,3,3,255,255,7,7,7,255,255,255,255,255,255,
	3,3,3,3,3,3,255,7,7,7,255,255,255,255,255,15,
	3,3,3,3,3,3,255,255,255,255,255,255,255,255,15,15,
	3,3,3,3,3,3,3,3,255,255,255,255,255,15,15,15,
	3,3,3,3,3,3,3,3,255,255,255,255,15,15,15,15,
	3,3,3,3,3,3,3,3,255,255,255,15,15,15,15,15,
	3,3,3,3,3,3,3,3,255,255,15,15,15,15,15,15,
	255,255,255,255,255,255,255,255,255,15,15,15,15,15,15,15,
	1,1,1,1,255,5,5,5,5,5,5,5,5,5,255,13,
	1,1,1,1,255,5,5,5,5,5,5,5,5,5,255,13,
	1,1,1,1,255,5,5,5,5,5,5,5,5,5,255,13,
	1,1,1,1,255,5,5,5,5,5,5,5,5,5,255,13,
	255,255,255,255,255,5,5,5,5,5,5,5,5,5,255,13,
	3,3,3,3,3,255,5,5,5,5,5,5,5,5,255,13,
	3,3,3,3,3,3,255,255,255,255,255,5,5,5,255,13,
	3,3,3,3,3,3,255,7,7,255,255,5,5,5,255,13,
	3,3,3,3,3,3,255,7,7,7,255,255,255,255,255,15,
	3,3,3,3,3,3,255,255,7,7,255,255,255,15,15,15,
	3,3,3,3,3,3,255,255,255,255,255,255,15,15,15,15,
	3,3,3,3,3,3,3,3,255,255,255,15,15,15,15,15,
	3,3,3,3,3,3,3,3,255,255,15,15,15,15,15,15,
	3,3,3,3,3,3,3,3,255,15,15,15,15,15,15,15,
	255,255,255,255,255,255,255,255,255,15,15,15,15,15,15,15,
	11,11,11,11,11,11,11,11,15,15,15,15,15,15,15,15,
	1,1,1,1,255,5,5,5,5,5,5,5,5,255,13,13,
	1,1,1,1,255,5,5,5,5,5,5,5,5,255,13,13,
	1,1,1,1,255,5,5,5,5,5,5,5,5,255,13,13,
	1,1,1,1,255,5,5,5,5,5,5,5,5,255,13,13,
	255,255,255,255,255,5,5,5,5,5,5,5,5,255,13,13,
	3,3,3,3,3,255,5,5,5,5,5,5,5,255,13,13,
	3,3,3,3,3,3,255,255,255,255,5,5,5,255,13,13,
	3,3,3,3,3,3,255,255,255,255,255,5,5,255,13,13,
	3,3,3,3,3,3,255,255,255,255,255,255,255,255,15,15,
	3,3,3,3,3,3,255,255,255,255,255,255,15,15,15,15,
	3,3,3,3,3,3,3,255,255,255,255,255,255,255,15,15,
	3,3,3,3,3,3,3,3,255,255,255,255,255,255,255,15,
	3,3,3,3,3,3,3,3,255,15,255,255,255,255,255,15,
	255,255,255,255,255,255,255,255,255,15,255,255,255,255,15,15,
	11,11,11,11,11,11,11,11,15,15,15,255,255,15,15,15,
	11,11,11,11,11,11,11,11,15,15,15,15,15,15,15,15,
	1,1,1,1,255,5,5,5,5,5,5,5,255,13,13,13,
	1,1,1,1,255,5,5,5,5,5,5,5,255,13,13,13,
	1,1,1,1,255,5,5,5,5,5,5,5,255,13,13,13,
	1,1,1,1,255,5,5,5,5,5,5,5,255,13,13,13,
	255,255,255,255,255,5,5,5,5,5,5,5,255,13,13,13,
	3,3,3,3,3,255,5,5,5,5,5,5,255,13,13,13,
	3,3,3,3,3,3,255,5,5,5,5,5,255,13,13,13,
	3,3,3,3,3,3,3,255,5,5,5,5,255,13,13,13,
	3,3,3,3,3,3,3,3,255,255,255,255,255,15,15,15,
	3,3,3,3,3,3,3,3,255,255,255,15,15,15,15,15,
	3,3,3,3,3,3,3,3,255,255,255,255,255,255,255,15,
	3,3,3,3,3,3,3,3,255,15,255,255,8,255,255,15,
	255,255,255,255,255,255,255,255,255,15,255,8,8,8,255,15,
	11,11,11,11,11,11,11,11,15,15,255,255,8,255,255,15,
	11,11,11,11,11,11,11,11,15,15,255,255,255,255,15,15,
	11,11,11,11,11,11,11,11,15,15,15,15,15,15,15,15,
	9,9,9,9,255,5,5,5,5,5,5,255,13,13,13,13,
	9,9,9,9,255,5,5,5,5,5,5,255,13,13,13,13,
	9,9,9,9,255,5,5,5,5,5,5,255,13,13,13,13,
	9,9,9,9,255,5,5,5,5,5,5,255,13,13,13,13,
	255,255,255,255,255,5,5,5,5,5,5,255,13,13,13,13,
	3,3,3,3,3,255,5,5,5,5,5,255,13,13,13,13,
	3,3,3,3,3,3,255,5,5,5,5,255,13,13,13,13,
	3,3,3,3,3,3,3,255,5,5,5,255,13,13,13,13,
	3,3,3,3,3,3,3,3,255,255,255,255,15,15,15,15,
	3,3,3,3,3,3,3,3,255,255,15,15,15,15,15,15,
	3,3,3,3,3,3,3,3,255,15,255,255,255,255,255,15,
	255,255,255,255,255,255,255,255,255,15,255,8,8,8,255,15,
	11,11,11,11,11,11,11,11,15,15,255,8,8,8,255,15,
	11,11,11,11,11,11,11,11,15,15,255,8,8,255,255,15,
	11,11,11,11,11,11,11,11,15,15,255,255,255,255,15,15,
	11,11,11,11,11,11,11,11,15,15,15,15,15,15,15,15,
	9,9,9,9,9,255,5,5,5,5,255,13,13,13,13,13,
	9,9,9,9,9,255,5,5,5,5,255,13,13,13,13,13,
	9,9,9,9,9,255,5,5,5,5,255,13,13,13,13,13,
	9,9,9,9,9,255,5,5,5,5,255,13,13,13,13,13,
	9,9,9,9,9,255,5,5,5,5,255,13,13,13,13,13,
	255,255,255,255,255,255,5,5,5,5,255,13,13,13,13,13,
	3,3,3,3,3,3,255,5,5,5,255,13,13,13,13,13,
	3,3,3,3,3,3,3,255,5,5,255,13,13,13,13,13,
	3,3,3,3,3,3,3,3,255,255,255,15,15,15,15,15,
	3,3,3,3,3,3,3,3,255,15,15,15,15,15,15,15,
	255,255,255,255,255,255,255,255,255,15,255,255,255,255,15,15,
	11,11,11,11,11,11,11,11,15,15,255,255,8,255,255,15,
	11,11,11,11,11,11,11,11,15,15,255,8,8,255,255,15,
	11,11,11,11,11,11,11,11,15,15,255,255,255,255,255,15,
	11,11,11,11,11,11,11,11,15,15,15,255,255,255,15,15,
	11,11,11,11,11,11,11,11,15,15,15,15,15,15,15,15,
	9,9,9,9,9,9,255,5,5,255,13,13,13,13,13,13,
	9,9,9,9,9,9,255,5,5,255,13,13,13,13,13,13,
	9,9,9,9,9,9,255,5,5,255,13,13,13,13,13,13,
	9,9,9,9,9,9,255,5,5,255,13,13,13,13,13,13,
	9,9,9,9,9,9,255,5,5,255,13,13,13,13,13,13,
	9,9,9,9,9,9,255,5,5,255,13,13,13,13,13,13,
	255,255,255,255,255,255,255,5,5,255,13,13,13,13,13,13,
	3,3,3,3,3,3,3,255,5,255,13,13,13,13,13,13,
	3,3,3,3,3,3,3,3,255,255,15,15,15,15,15,15,
	255,255,255,255,255,255,255,255,255,15,15,15,15,15,15,15,
	11,11,11,11,11,11,11,11,15,15,15,255,255,15,15,15,
	11,11,11,11,11,11,11,11,15,15,255,255,255,255,15,15,
	11,11,11,11,11,11,11,11,15,15,255,255,255,255,15,15,
	11,11,11,11,11,11,11,11,15,15,15,255,255,255,15,15,
	11,11,11,11,11,11,11,11,15,15,15,15,15,15,15,15,
	11,11,11,11,11,11,11,11,15,15,15,15,15,15,15,15,
	9,9,9,9,9,9,9,255,255,13,13,13,13,13,13,13,
	9,9,9,9,9,9,9,255,255,13,13,13,13,13,13,13,
	9,9,9,9,9,9,9,255,255,13,13,13,13,13,13,13,
	9,9,9,9,9,9,9,255,255,13,13,13,13,13,13,13,
	9,9,9,9,9,9,9,255,255,13,13,13,13,13,13,13,
	9,9,9,9,9,9,9,255,255,13,13,13,13,13,13,13,
	9,9,9,9,9,9,9,255,255,13,13,13,13,13,13,13,
	255,255,255,255,255,255,255,255,255,13,13,13,13,13,13,13,
	255,255,255,255,255,255,255,255,255,15,15,15,15,15,15,15,
	11,11,11,11,11,11,11,11,15,15,15,15,15,15,15,15,
	11,11,11,11,11,11,11,11,15,15,15,15,15,15,15,15,
	11,11,11,11,11,11,11,11,15,15,15,15,15,15,15,15,
	11,11,11,11,11,11,11,11,15,15,15,15,15,15,15,15,
	11,11,11,11,11,11,11,11,15,15,15,15,15,15,15,15,
	11,11,11,11,11,11,11,11,15,15,15,15,15,15,15,15,
	11,11,11,11,11,11,11,11,15,15,15,15,15,15,15,15,
	};
/*...e*/

static byte nearest_colour(byte r, byte g, byte b)
	{
	byte i;

	if ( (i = quick_tab[r >> 4][g >> 4][b >> 4]) != (byte) 0xff )
		return i;

	return calc_nearest(r, g, b);
	}
/*...e*/

void gbm_errdiff_line_VGA(byte *src, byte *dest, short *errs, int cx)
	{
	BOOLEAN left = TRUE;
	int x, ptr = 0;

	init();

	/* Step 1: Add error terms to newly supplied line */

	for ( x = 0; x < cx * 3; x++ )
		src[x] = U_SAT_ADD((int) src[x], (int) errs[x]);

	/* Step 2: Zero out error terms */

	memset(errs, 0, (cx + 1) * 3 * sizeof(errs[0]));

	/* Step 3: Go along data, finding nearest colour and propagating error */

	randinx = rand();

	for ( x = 0; x < cx; x++ )
		{
		byte b     = *src++;
		byte g     = *src++;
		byte r     = *src++;
		byte bi    = nearest_colour(r, g, b); 
		int  be    = (int) b - (int) gbmrgb_vga[bi].b;
		int  ge    = (int) g - (int) gbmrgb_vga[bi].g;
		int  re    = (int) r - (int) gbmrgb_vga[bi].r;
		word rn    = randtab[(byte) (randinx++)];
		int  right = (rn >> 8);
		int  down  = ((rn & 0xff) % (63 - right));
		int  be1   = ((be * right) >> 6);
		int  ge1   = ((ge * right) >> 6);
		int  re1   = ((re * right) >> 6);
		int  be2   = ((be * down ) >> 6);
		int  ge2   = ((ge * down ) >> 6);
		int  re2   = ((re * down ) >> 6);
		int  be3   = be - be1 - be2;
		int  ge3   = ge - ge1 - ge2;
		int  re3   = re - re1 - re2;

		if ( left )
			*dest = (byte) (bi << 4);
		else
			*dest++ |= bi;

		left = !left;

		src[0] = U_SAT_ADD((int) src[0], be1);
		src[1] = U_SAT_ADD((int) src[1], ge1);
		src[2] = U_SAT_ADD((int) src[2], re1);

		errs[ptr    ] = S_SAT_ADD((int) errs[ptr    ], be2);
		errs[ptr + 1] = S_SAT_ADD((int) errs[ptr + 1], ge2);
		errs[ptr + 2] = S_SAT_ADD((int) errs[ptr + 2], re2);

		ptr += 3;

		errs[ptr    ] = S_SAT_ADD((int) errs[ptr    ], be3);
		errs[ptr + 1] = S_SAT_ADD((int) errs[ptr + 1], ge3);
		errs[ptr + 2] = S_SAT_ADD((int) errs[ptr + 2], re3);
		}
	}
/*...e*/
/*...sgbm_errdiff_VGA         \45\ error diffuse to default VGA palette:0:*/
BOOLEAN	gbm_errdiff_VGA(const GBM *gbm, const byte *data24, byte *data4)
	{
	return errdiff(gbm, data24, data4, 4, gbm_errdiff_line_VGA);
	}
/*...e*/

/*...sgbm_errdiff_pal_8       \45\ return default 8 colour palette:0:*/
/*
This function makes the palette for the 16 colour 8 colour palette.
*/

static GBMRGB gbmrgb_8[] =
	{
	  0,  0,  0,
	  0,  0,255,
	  0,255,  0,
	  0,255,255,
	255,  0,  0,
	255,  0,255,
	255,255,  0,
	255,255,255,
	  0,  0,  0,
	  0,  0,  0,
	  0,  0,  0,
	  0,  0,  0,
	  0,  0,  0,
	  0,  0,  0,
	  0,  0,  0,
	  0,  0,  0,
	};

void gbm_errdiff_pal_8(GBMRGB *gbmrgb)
	{
	init();
	memcpy((char *) gbmrgb, (char *) gbmrgb_8, sizeof(gbmrgb_8));
	}
/*...e*/
/*...sgbm_errdiff_line_8      \45\ error diffuse to default 8 colour palette one line:0:*/
/*
This function takes a line of RGB data, a destination buffer and a buffer of
error terms and performs one lines worth (a given # of pixels) of error
diffusion.
*/

void gbm_errdiff_line_8(byte *src, byte *dest, short *errs, int cx)
	{
	BOOLEAN left = TRUE;
	int x, ptr = 0;

	init();

	/* Step 1: Add error terms to newly supplied line */

	for ( x = 0; x < cx * 3; x++ )
		src[x] = U_SAT_ADD((int) src[x], (int) errs[x]);

	/* Step 2: Zero out error terms */

	memset(errs, 0, (cx + 1) * 3 * sizeof(errs[0]));

	/* Step 3: Go along data, finding nearest colour and propagating error */

	randinx = rand();

	for ( x = 0; x < cx; x++ )
		{
		byte b     = *src++;
		byte g     = *src++;
		byte r     = *src++;
		byte bi    = ((r&0x80U)>>5) | ((g&0x80U)>>6) | ((b&0x80U)>>7);
		int  be    = (int) b - (int) gbmrgb_8[bi].b;
		int  ge    = (int) g - (int) gbmrgb_8[bi].g;
		int  re    = (int) r - (int) gbmrgb_8[bi].r;
		word rn    = randtab[(byte) (randinx++)];
		int  right = (rn >> 8);
		int  down  = ((rn & 0xff) % (63 - right));
		int  be1   = ((be * right) >> 6);
		int  ge1   = ((ge * right) >> 6);
		int  re1   = ((re * right) >> 6);
		int  be2   = ((be * down ) >> 6);
		int  ge2   = ((ge * down ) >> 6);
		int  re2   = ((re * down ) >> 6);
		int  be3   = be - be1 - be2;
		int  ge3   = ge - ge1 - ge2;
		int  re3   = re - re1 - re2;

		if ( left )
			*dest = (byte) (bi << 4);
		else
			*dest++ |= bi;

		left = !left;

		src[0] = U_SAT_ADD((int) src[0], be1);
		src[1] = U_SAT_ADD((int) src[1], ge1);
		src[2] = U_SAT_ADD((int) src[2], re1);

		errs[ptr    ] = S_SAT_ADD((int) errs[ptr    ], be2);
		errs[ptr + 1] = S_SAT_ADD((int) errs[ptr + 1], ge2);
		errs[ptr + 2] = S_SAT_ADD((int) errs[ptr + 2], re2);

		ptr += 3;

		errs[ptr    ] = S_SAT_ADD((int) errs[ptr    ], be3);
		errs[ptr + 1] = S_SAT_ADD((int) errs[ptr + 1], ge3);
		errs[ptr + 2] = S_SAT_ADD((int) errs[ptr + 2], re3);
		}
	}
/*...e*/
/*...sgbm_errdiff_8           \45\ error diffuse to default 8 colour palette:0:*/
BOOLEAN	gbm_errdiff_8(const GBM *gbm, const byte *data24, byte *data4)
	{
	return errdiff(gbm, data24, data4, 4, gbm_errdiff_line_8);
	}
/*...e*/

/*...sgbm_errdiff_pal_4G      \45\ return 4 bit greyscale palette:0:*/
/*
This function makes the palette for the 16 colour VGA palette.
*/

void gbm_errdiff_pal_4G(GBMRGB *gbmrgb)
	{
	int i;

	init();
	for ( i = 0; i < 0x10; i++ )
		{
		gbmrgb[i].r = scale16[i];
		gbmrgb[i].g = scale16[i];
		gbmrgb[i].b = scale16[i];
		}
	}
/*...e*/
/*...sgbm_errdiff_line_4G     \45\ error diffuse to 4 bit greyscale palette one line:0:*/
/*
This function takes a line of RGB data, a destination buffer and a buffer of
error terms and performs one lines worth (a given # of pixels) of error
diffusion.
*/

void gbm_errdiff_line_4G(byte *src, byte *dest, short *errs, int cx)
	{
	BOOLEAN left = TRUE;
	int x, ptr = 0;
	byte *pb;

	init();

	/* Step 1: Add error terms to newly supplied line */

	for ( x = 0, pb = src; x < cx; x++ )
		{
		byte b = *pb++;
		byte g = *pb++;
		byte r = *pb++;

		src[x] = (byte) (((word) r * 77U + (word) g * 150U + (word) b * 29U) >> 8);
		src[x] = U_SAT_ADD((int) src[x], (int) errs[x]);
		}

	/* Step 2: Zero out error terms */

	memset(errs, 0, (cx + 1) * sizeof(errs[0]));

	/* Step 3: Go along data, finding nearest colour and propagating error */

	randinx = rand();

	for ( x = 0; x < cx; x++ )
		{
		byte grey  = *src++;
		byte inx   = index16[grey];
		int  err   = (int) grey - (int) scale16[inx];
		word rn    = randtab[(byte) (randinx++)];
		int  right = (rn >> 8);
		int  down  = ((rn & 0xff) % (63 - right));
		int  err1  = ((err * right) >> 6);
		int  err2  = ((err * down ) >> 6);
		int  err3  = err - err1 - err2;

		if ( left )
			*dest = (byte) (inx << 4);
		else
			*dest++ |= inx;

		left = !left;

		src[0] = U_SAT_ADD((int) src[0], err1);

		errs[ptr] = S_SAT_ADD((int) errs[ptr], err2);

		ptr++;

		errs[ptr] = S_SAT_ADD((int) errs[ptr], err3);
		}
	}
/*...e*/
/*...sgbm_errdiff_4G          \45\ error diffuse to 4 bit greyscale palette:0:*/
BOOLEAN	gbm_errdiff_4G(const GBM *gbm, const byte *data24, byte *data4)
	{
	return errdiff(gbm, data24, data4, 4, gbm_errdiff_line_4G);
	}
/*...e*/

/*...sgbm_errdiff_pal_BW      \45\ return black and white palette:0:*/
/*
This function returns the b/w palette.
(We consider the image to be likely black on white,
hence the ordering of the palette entries).
*/

static GBMRGB gbmrgb_bw[] =
	{
	255,255,255,
	0,0,0,
	};

void gbm_errdiff_pal_BW(GBMRGB *gbmrgb)
	{
	init();
	memcpy((char *) gbmrgb, (char *) gbmrgb_bw, sizeof(gbmrgb_bw));
	}
/*...e*/
/*...sgbm_errdiff_line_BW     \45\ error diffuse to black and white one line:0:*/
/*
This function takes a line of RGB data, a destination buffer and a buffer of
error terms and performs one lines worth (a given # of pixels) of error
diffusion.
*/

void gbm_errdiff_line_BW(byte *src, byte *dest, short *errs, int cx)
	{
	int x, bit = 0, ptr = 0;
	byte *pb;

	init();

	/* Step 1: Add error terms to newly supplied line */

	for ( x = 0, pb = src; x < cx; x++ )
		{
		byte b = *pb++;
		byte g = *pb++;
		byte r = *pb++;

		src[x] = (byte) (((word) r * 77U + (word) g * 150U + (word) b * 29U) >> 8);
		src[x] = U_SAT_ADD((int) src[x], (int) errs[x]);
		}
		
	/* Step 2: Zero out error terms */

	memset(errs, 0, (cx + 1) * sizeof(errs[0]));

	/* Step 3: Go along data, finding nearest colour and propagating error */

	memset(dest, 0, (unsigned) (cx + 7) >> 3);

	randinx = rand();

	for ( x = 0; x < cx; x++ )
		{
		byte grey  = *src++;
		byte inx   = (byte) (grey < 0x80);
		int  err   = (int) grey - (int) gbmrgb_bw[inx].b;
		word rn    = randtab[(byte) (randinx++)];
		int  right = (rn >> 8);
		int  down  = ((rn & 0xff) % (63 - right));
		int  err1  = ((err * right) >> 6);
		int  err2  = ((err * down ) >> 6);
		int  err3  = err - err1 - err2;

		if ( inx )
			*dest |= (0x80U >> bit);

		if ( ++bit == 8 )
			{
			bit = 0;
			dest++;
			}

		src[0] = U_SAT_ADD((int) src[0], err1);

		errs[ptr] = S_SAT_ADD((int) errs[ptr], err2);

		ptr++;

		errs[ptr] = S_SAT_ADD((int) errs[ptr], err3);
		}
	}
/*...e*/
/*...sgbm_errdiff_BW          \45\ error diffuse to black and white:0:*/
BOOLEAN	gbm_errdiff_BW(const GBM *gbm, const byte *data24, byte *data1)
	{
	return errdiff(gbm, data24, data1, 1, gbm_errdiff_line_BW);
	}
/*...e*/
