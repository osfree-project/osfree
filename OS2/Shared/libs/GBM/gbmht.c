/*

gbmht.c - Halftoner

*/

/*...sincludes:0:*/
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmht.h"

/*...vgbm\46\h:0:*/
/*...e*/
/*...svars:0:*/
static BOOLEAN inited = FALSE;

/*
For 6Rx6Gx6B, 7Rx8Gx4B palettes etc.
*/

static byte index4[0x400];
static byte index6[0x400];
static byte index7[0x400];
static byte index8[0x400];
static byte scale4[] = { 0, 85, 170, 255 };
static byte scale6[] = { 0, 51, 102, 153, 204, 255 };
static byte scale7[] = { 0, 43, 85, 128, 170, 213, 255 };
static byte scale8[] = { 0, 36, 73, 109, 146, 182, 219, 255 };

static byte map_to_012[0x900];
	/* Returns 0 if index <  0x80                  */
	/*         1 if index >= 0x80 and index < 0xff */
	/*         2 if index  = 0xff                  */
static byte map_to_inx[3][3][3];

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
/*...e*/
/*...sdivt:0:*/
/*
This fold encompasses a way to get rapid division via lookup tables.
*/

static word divt9[9 * 0x100];
static word divt7[9 * 0x100];
static word divt6[9 * 0x100];
static word divt5[9 * 0x100];
static word divt3[9 * 0x100];

/*...smake_divt:0:*/
static void make_divt(word *divt, int size, int by)
	{
	int i;

	for ( i = 0; i < size; i++ )
		divt[i] = (word) (i / by);
	}
/*...e*/

#define	div9(w)	divt9[w]
#define	div8(w)	((w)>>3)
#define	div7(w)	divt7[w]
#define	div6(w)	divt6[w]
#define	div5(w)	divt5[w]
#define	div4(w)	((w)>>2)
#define	div3(w)	divt3[w]
#define	div2(w)	((w)>>1)
/*...e*/
/*...sinit:0:*/
/*
This function initialises this module.
*/

/*...stakeout_inx:0:*/
/*
For the supplied value, find the index of the highest value in the scale
less than or equal to the value.
*/

static byte takeout_inx(int value, const byte ab[], unsigned short cb)
	{
	byte inx = 0;
	unsigned short i;

	for ( i = 0; i < cb; i++ )
		if ( (unsigned short) ab[i] <= (unsigned short) value )
			inx = (byte) i;

	return inx;
	}
/*...e*/
/*...stakeout_inx_vga:0:*/
/*
The idea is to take as much of the r, g and b as possible by chosing a
colour from the VGA palette that leaves as little left over as possible.
Since there are 2 mid-greys, we discourage the use of one.
The only justification for this is that the results seem to come out better!
Also, I have tried allowing takefrom to return the closest index, allowing
more to be taken than r,g and b. This gives less grey results, but the
output is a lot dirtier and speckled.
*/

static byte takeout_inx_vga(word r, word g, word b)
	{
	byte inx;
	byte inx_min = 0;
	int e_min = (int) ( r + g + b );

	for ( inx = 1; inx < 16; inx++ )
		if ( inx != 8 )
			{
			int re = (int) ( r - (word) gbmrgb_vga[inx].r );
			int ge = (int) ( g - (word) gbmrgb_vga[inx].g );
			int be = (int) ( b - (word) gbmrgb_vga[inx].b );

			if ( re >= 0 && ge >= 0 && be >= 0 )
				{
				int e = re + ge + be;

				if ( e < e_min )
					{
					e_min = e;
					inx_min = inx;
					}
				}
			}
	return inx_min;
	}
/*...e*/

static void init(void)
	{
	static word val[] = { 0, 0x80, 0xff };
	int i;
	int volatile r;		/* C-Set/2 optimiser fix */
	int volatile g;
	int volatile b;

	if ( inited )
		return;

	/* For 7 Red x 8 Green x 4 Blue palettes etc. */

	for ( i = 0; i < 0x400; i++ )
		{
		index4[i] = takeout_inx(i, scale4 , sizeof(scale4));
		index6[i] = takeout_inx(i, scale6 , sizeof(scale6));
		index7[i] = takeout_inx(i, scale7 , sizeof(scale7));
		index8[i] = takeout_inx(i, scale8 , sizeof(scale8));
		}

	memset(map_to_012, 0, 0x80);
	memset(map_to_012 + 0x80, 1, 0x7f);
	memset(map_to_012 + 0xff, 2, 0x801);

	for ( r = 0; r < 3; r++ )
		for ( g = 0; g < 3; g++ )
			for ( b = 0; b < 3; b++ )
				map_to_inx[r][g][b] = takeout_inx_vga(val[r], val[g], val[b]);

	make_divt(divt9, sizeof(divt9)/sizeof(word), 9);
	make_divt(divt7, sizeof(divt7)/sizeof(word), 7);
	make_divt(divt6, sizeof(divt6)/sizeof(word), 6);
	make_divt(divt5, sizeof(divt5)/sizeof(word), 5);
	make_divt(divt3, sizeof(divt3)/sizeof(word), 3);

	inited = TRUE;
	}
/*...e*/
/*...stakefrom:0:*/
#define	takefrom(r,g,b) ( map_to_inx[map_to_012[r]][map_to_012[g]][map_to_012[b]] )
/*...e*/
/*...ssplit_into:0:*/
/* n is only ever 2, 3, 4, 6 or 9 */

static void split_into(
	word r, word g, word b,
	int n, byte *inxs
	)
	{
	byte inx;

	if ( n >= 9 )
		{
		inx = takefrom(div9(r), div9(g), div9(b));
		b -= gbmrgb_vga[inx].b;
		g -= gbmrgb_vga[inx].g;
		r -= gbmrgb_vga[inx].r;
		*inxs++ = inx;

		inx = takefrom(div8(r), div8(g), div8(b));
		b -= gbmrgb_vga[inx].b;
		g -= gbmrgb_vga[inx].g;
		r -= gbmrgb_vga[inx].r;
		*inxs++ = inx;

		inx = takefrom(div7(r), div7(g), div7(b));
		b -= gbmrgb_vga[inx].b;
		g -= gbmrgb_vga[inx].g;
		r -= gbmrgb_vga[inx].r;
		*inxs++ = inx;
		}

	if ( n >= 6 )
		{
		inx = takefrom(div6(r), div6(g), div6(b));
		b -= gbmrgb_vga[inx].b;
		g -= gbmrgb_vga[inx].g;
		r -= gbmrgb_vga[inx].r;
		*inxs++ = inx;

		inx = takefrom(div5(r), div5(g), div5(b));
		b -= gbmrgb_vga[inx].b;
		g -= gbmrgb_vga[inx].g;
		r -= gbmrgb_vga[inx].r;
		*inxs++ = inx;
		}

	if ( n >= 4 )
		{
		inx = takefrom(div4(r), div4(g), div4(b));
		b -= gbmrgb_vga[inx].b;
		g -= gbmrgb_vga[inx].g;
		r -= gbmrgb_vga[inx].r;
		*inxs++ = inx;
		}

	if ( n >= 3 )
		{
		inx = takefrom(div3(r), div3(g), div3(b));
		b -= gbmrgb_vga[inx].b;
		g -= gbmrgb_vga[inx].g;
		r -= gbmrgb_vga[inx].r;
		*inxs++ = inx;
		}

	inx = takefrom(div2(r), div2(g), div2(b));
	b -= gbmrgb_vga[inx].b;
	g -= gbmrgb_vga[inx].g;
	r -= gbmrgb_vga[inx].r;
	*inxs++ = inx;

	*inxs = takefrom(r, g, b);
	}
/*...e*/
/*...stakefrom8:0:*/
/*
Find the largest colour from the 8 colour palette.
*/

#define	takefrom8(r,g,b) ( (((r)>=255)<<2) | (((g)>=255)<<1) | ((b)>=255) )
/*...e*/
/*...ssplit_into8:0:*/
/* n is only ever 2, 3, 4, 6 or 9 */

static void split_into8(
	word r, word g, word b,
	int n, byte *inxs
	)
	{
	byte inx;

	if ( n >= 9 )
		{
		inx = takefrom8(div9(r), div9(g), div9(b));
		b -= gbmrgb_8[inx].b;
		g -= gbmrgb_8[inx].g;
		r -= gbmrgb_8[inx].r;
		*inxs++ = inx;

		inx = takefrom8(div8(r), div8(g), div8(b));
		b -= gbmrgb_8[inx].b;
		g -= gbmrgb_8[inx].g;
		r -= gbmrgb_8[inx].r;
		*inxs++ = inx;

		inx = takefrom8(div7(r), div7(g), div7(b));
		b -= gbmrgb_8[inx].b;
		g -= gbmrgb_8[inx].g;
		r -= gbmrgb_8[inx].r;
		*inxs++ = inx;
		}

	if ( n >= 6 )
		{
		inx = takefrom8(div6(r), div6(g), div6(b));
		b -= gbmrgb_8[inx].b;
		g -= gbmrgb_8[inx].g;
		r -= gbmrgb_8[inx].r;
		*inxs++ = inx;

		inx = takefrom8(div5(r), div5(g), div5(b));
		b -= gbmrgb_8[inx].b;
		g -= gbmrgb_8[inx].g;
		r -= gbmrgb_8[inx].r;
		*inxs++ = inx;
		}

	if ( n >= 4 )
		{
		inx = takefrom8(div4(r), div4(g), div4(b));
		b -= gbmrgb_8[inx].b;
		g -= gbmrgb_8[inx].g;
		r -= gbmrgb_8[inx].r;
		*inxs++ = inx;
		}

	if ( n >= 3 )
		{
		inx = takefrom8(div3(r), div3(g), div3(b));
		b -= gbmrgb_8[inx].b;
		g -= gbmrgb_8[inx].g;
		r -= gbmrgb_8[inx].r;
		*inxs++ = inx;
		}

	inx = takefrom8(div2(r), div2(g), div2(b));
	b -= gbmrgb_8[inx].b;
	g -= gbmrgb_8[inx].g;
	r -= gbmrgb_8[inx].r;
	*inxs++ = inx;

	*inxs = takefrom8(r, g, b);
	}
/*...e*/

/*...sgbm_ht_24_2x2     \45\ halftone by 2x2 to r\58\g\58\b bits:0:*/
void gbm_ht_24_2x2(const GBM *gbm, const byte *src24, byte *dest24, byte rm, byte gm, byte bm)
	{
	int stride = ((gbm->w * 3 + 3) & ~3);
	int x, y;

	init();

	src24 -= stride;
	dest24 -= stride;

	for ( y = 0; y < gbm->h - 1; y += 2 )
		{
		const byte *src24a  = (src24 += stride);
		const byte *src24b  = (src24 += stride);
		      byte *dest24a = (dest24 += stride);
		      byte *dest24b = (dest24 += stride);

		for ( x = 0; x < gbm->w - 1; x += 2 )
/*...s2x2 case:24:*/
{
word r,g,b;
byte ri,gi,bi;
byte *tmp;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;

bi = (div4(b) & bm); gi = (div4(g) & gm); ri = (div4(r) & rm);
*dest24a++ = bi; *dest24a++ = gi; *dest24a++ = ri;
b -= bi; g -= gi; r -= ri;

if ( b > 255 * 3 ) b = 255 * 3;
if ( g > 255 * 3 ) g = 255 * 3;
if ( r > 255 * 3 ) r = 255 * 3;

bi = (div3(b) & bm); gi = (div3(g) & gm); ri = (div3(r) & rm);
*dest24a++ = bi; *dest24a++ = gi; *dest24a++ = ri;
b -= bi; g -= gi; r -= ri;

if ( b > 255 * 2 ) b = 255 * 2;
if ( g > 255 * 2 ) g = 255 * 2;
if ( r > 255 * 2 ) r = 255 * 2;

bi = (div2(b) & bm); gi = (div2(g) & gm); ri = (div2(r) & rm);
*dest24b++ = bi; *dest24b++ = gi; *dest24b++ = ri;
b -= bi; g -= gi; r -= ri;

if ( b > 255 ) b = 255;
if ( g > 255 ) g = 255;
if ( r > 255 ) r = 255;

bi = (b & bm); gi = (g & gm); ri = (r & rm);
*dest24b++ = bi; *dest24b++ = gi; *dest24b++ = ri;

tmp = dest24a; dest24a = dest24b; dest24b = tmp;
}
/*...e*/
		if ( x < gbm->w )
/*...s1x2 case:24:*/
{
word r,g,b;
byte ri, gi, bi;

b  = *src24a++;	g  = *src24a++;	r  = *src24a;
b += *src24b++;	g += *src24b++;	r += *src24b;

bi = (div2(b) & bm); gi = (div2(g) & gm); ri = (div2(r) & rm);
*dest24a++ = bi; *dest24a++ = gi; *dest24a++ = ri;
b -= bi; g -= gi; r -= ri;

if ( b > 255 ) b = 255;
if ( g > 255 ) g = 255;
if ( r > 255 ) r = 255;

bi = (b & bm); gi = (g & gm); ri = (r & rm);
*dest24b++ = bi; *dest24b++ = gi; *dest24b   = ri;
}
/*...e*/
		}
	if ( y < gbm->h )
		{
		const byte *src24a  = src24 + stride;
		      byte *dest24a = dest24 + stride;

		for ( x = 0; x < gbm->w - 1; x += 2 )
/*...s2x1 case:24:*/
{
word r,g,b;
byte ri, gi, bi;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;

bi = (div2(b) & bm); gi = (div2(g) & gm); ri = (div2(r) & rm);
*dest24a++ = bi; *dest24a++ = gi; *dest24a++ = ri;
b -= bi; g -= gi; r -= ri;

if ( b > 255 ) b = 255;
if ( g > 255 ) g = 255;
if ( r > 255 ) r = 255;

bi = (b & bm); gi = (g & gm); ri = (r & rm);
*dest24a++ = bi; *dest24a++ = gi; *dest24a++ = ri;
}
/*...e*/
		if ( x < gbm->w )
/*...s1x1 case:24:*/
{
byte ri, gi, bi;

bi = ((*src24a++) & bm); gi = ((*src24a++) & gm); ri = ((*src24a) & rm);
*dest24a++ = bi; *dest24a++ = gi; *dest24a = ri;
}
/*...e*/
		}
	}
/*...e*/

/*...sgbm_ht_pal_6R6G6B \45\ return 6Rx6Gx6B palette:0:*/
/*
This function makes the palette for the 6 red x 6 green x 6 blue palette.
216 palette entrys used. Remaining 40 left blank.
*/

void gbm_ht_pal_6R6G6B(GBMRGB *gbmrgb)
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
/*...sgbm_ht_6R6G6B_2x2 \45\ halftone by 2x2 to 6Rx6Gx6B palette:0:*/
#define	PIX666(ri,gi,bi) (byte) (6 * (6 * ri + gi) + bi)

void gbm_ht_6R6G6B_2x2(const GBM *gbm, const byte *src24, byte *dest8)
	{
	int stride24 = ((gbm->w * 3 + 3) & ~3);
	int stride8  = ((gbm->w     + 3) & ~3);
	int x, y;

	init();

	src24 -= stride24;
	dest8 -= stride8;

	for ( y = 0; y < gbm->h - 1; y += 2 )
		{
		const byte *src24a = (src24 += stride24);
		const byte *src24b = (src24 += stride24);
		      byte *dest8a = (dest8 += stride8);
		      byte *dest8b = (dest8 += stride8);

		for ( x = 0; x < gbm->w - 1; x += 2 )
/*...s2x2 case:24:*/
{
word r,g,b;
byte ri,gi,bi;
byte *tmp;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;

bi = index6[div4(b)]; gi = index6[div4(g)]; ri = index6[div4(r)];
*dest8a++ = PIX666(ri,gi,bi);
b -= scale6[bi]; g -= scale6[gi]; r -= scale6[ri];

bi = index6[div3(b)]; gi = index6[div3(g)]; ri = index6[div3(r)];
*dest8a++ = PIX666(ri,gi,bi);
b -= scale6[bi]; g -= scale6[gi]; r -= scale6[ri];

bi = index6[div2(b)]; gi = index6[div2(g)]; ri = index6[div2(r)];
*dest8b++ = PIX666(ri,gi,bi);
b -= scale6[bi]; g -= scale6[gi]; r -= scale6[ri];

bi = index6[b      ]; gi = index6[g      ]; ri = index6[r      ];
*dest8b++ = PIX666(ri,gi,bi);

tmp = dest8a; dest8a = dest8b; dest8b = tmp;
}
/*...e*/
		if ( x < gbm->w )
/*...s1x2 case:24:*/
{
word r,g,b;
byte ri, gi, bi;

b  = *src24a++;	g  = *src24a++;	r  = *src24a;
b += *src24b++;	g += *src24b++;	r += *src24b;

bi = index6[div2(b)]; gi = index6[div2(g)]; ri = index6[div2(r)];
*dest8a = PIX666(ri,gi,bi);
b -= scale6[bi]; g -= scale6[gi]; r -= scale6[ri];

bi = index6[b      ]; gi = index6[g      ]; ri = index6[r      ];
*dest8b = PIX666(ri,gi,bi);
}
/*...e*/
		}
	if ( y < gbm->h )
		{
		const byte *src24a = src24 + stride24;
		      byte *dest8a = dest8 + stride8;

		for ( x = 0; x < gbm->w - 1; x += 2 )
/*...s2x1 case:24:*/
{
word r,g,b;
byte ri, gi, bi;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;

bi = index6[div2(b)]; gi = index6[div2(g)]; ri = index6[div2(r)];
*dest8a++ = PIX666(ri,gi,bi);
b -= scale6[bi]; g -= scale6[gi]; r -= scale6[ri];

bi = index6[b      ]; gi = index6[g      ]; ri = index6[r      ];
*dest8a++ = PIX666(ri,gi,bi);
}
/*...e*/
		if ( x < gbm->w )
/*...s1x1 case:24:*/
{
byte ri, gi, bi;

bi = index6[*src24a++]; gi = index6[*src24a++]; ri = index6[*src24a];
*dest8a = PIX666(ri,gi,bi);
}
/*...e*/
		}
	}
/*...e*/

/*...sgbm_ht_pal_7R8G4B \45\ return 7Rx8Gx4B palette:0:*/
/*
This function makes the palette for the 7 red x 8 green x 4 blue palette.
224 palette entrys used. Remaining 32 left blank.
Colours calculated to match those used by 8514/A PM driver.
*/

void gbm_ht_pal_7R8G4B(GBMRGB *gbmrgb)
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
/*...sgbm_ht_7R8G4B_2x2 \45\ halftone by 2x2 to 7Rx8Gx4B palette:0:*/
#define	PIX784(ri,gi,bi) (byte) (((((ri)<<3)+(gi))<<2)+(bi))

void gbm_ht_7R8G4B_2x2(const GBM *gbm, const byte *src24, byte *dest8)
	{
	int stride24 = ((gbm->w * 3 + 3) & ~3);
	int stride8  = ((gbm->w     + 3) & ~3);
	int x, y;

	init();

	src24 -= stride24;
	dest8 -= stride8;

	for ( y = 0; y < gbm->h - 1; y += 2 )
		{
		const byte *src24a = (src24 += stride24);
		const byte *src24b = (src24 += stride24);
		      byte *dest8a = (dest8 += stride8);
		      byte *dest8b = (dest8 += stride8);

		for ( x = 0; x < gbm->w - 1; x += 2 )
/*...s2x2 case:24:*/
{
word r,g,b;
byte ri,gi,bi;
byte *tmp;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;

bi = index4[div4(b)]; gi = index8[div4(g)]; ri = index7[div4(r)];
*dest8a++ = PIX784(ri,gi,bi);
b -= scale4[bi]; g -= scale8[gi]; r -= scale7[ri];

bi = index4[div3(b)]; gi = index8[div3(g)]; ri = index7[div3(r)];
*dest8a++ = PIX784(ri,gi,bi);
b -= scale4[bi]; g -= scale8[gi]; r -= scale7[ri];

bi = index4[div2(b)]; gi = index8[div2(g)]; ri = index7[div2(r)];
*dest8b++ = PIX784(ri,gi,bi);
b -= scale4[bi]; g -= scale8[gi]; r -= scale7[ri];

bi = index4[b      ]; gi = index8[g      ]; ri = index7[r      ];
*dest8b++ = PIX784(ri,gi,bi);

tmp = dest8a; dest8a = dest8b; dest8b = tmp;
}
/*...e*/
		if ( x < gbm->w )
/*...s1x2 case:24:*/
{
word r,g,b;
byte ri, gi, bi;

b  = *src24a++;	g  = *src24a++;	r  = *src24a;
b += *src24b++;	g += *src24b++;	r += *src24b;

bi = index4[div2(b)]; gi = index8[div2(g)]; ri = index7[div2(r)];
*dest8a = PIX784(ri,gi,bi);
b -= scale4[bi]; g -= scale8[gi]; r -= scale7[ri];

bi = index4[b      ]; gi = index8[g      ]; ri = index7[r      ];
*dest8b = PIX784(ri,gi,bi);
}
/*...e*/
		}
	if ( y < gbm->h )
		{
		const byte *src24a = src24 + stride24;
		      byte *dest8a = dest8 + stride8;

		for ( x = 0; x < gbm->w - 1; x += 2 )
/*...s2x1 case:24:*/
{
word r,g,b;
byte ri, gi, bi;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;

bi = index4[div2(b)]; gi = index8[div2(g)]; ri = index7[div2(r)];
*dest8a++ = PIX784(ri,gi,bi);
b -= scale4[bi]; g -= scale8[gi]; r -= scale7[ri];

bi = index4[b      ]; gi = index8[g      ]; ri = index7[r      ];
*dest8a++ = PIX784(ri,gi,bi);
}
/*...e*/
		if ( x < gbm->w )
/*...s1x1 case:24:*/
{
byte ri, gi, bi;

bi = index4[*src24a++]; gi = index8[*src24a++]; ri = index7[*src24a];
*dest8a = PIX784(ri,gi,bi);
}
/*...e*/
		}
	}
/*...e*/

/*...sgbm_ht_pal_4R4G4B \45\ return 4Rx4Gx4B palette:0:*/
/*
This function makes the palette for the 4 red x 4 green x 4 blue palette.
64 palette entrys used. Remaining 192 left blank.
*/

void gbm_ht_pal_4R4G4B(GBMRGB *gbmrgb)
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
/*...sgbm_ht_4R4G4B_2x2 \45\ halftone by 2x2 to 4Rx4Gx4B palette:0:*/
#define	PIX444(ri,gi,bi) (byte) (((((ri)<<2)+(gi))<<2)+(bi))

void gbm_ht_4R4G4B_2x2(const GBM *gbm, const byte *src24, byte *dest8)
	{
	int stride24 = ((gbm->w * 3 + 3) & ~3);
	int stride8  = ((gbm->w     + 3) & ~3);
	int x, y;

	init();

	src24 -= stride24;
	dest8 -= stride8;

	for ( y = 0; y < gbm->h - 1; y += 2 )
		{
		const byte *src24a = (src24 += stride24);
		const byte *src24b = (src24 += stride24);
		      byte *dest8a = (dest8 += stride8);
		      byte *dest8b = (dest8 += stride8);

		for ( x = 0; x < gbm->w - 1; x += 2 )
/*...s2x2 case:24:*/
{
word r,g,b;
byte ri,gi,bi;
byte *tmp;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;

bi = index4[div4(b)]; gi = index4[div4(g)]; ri = index4[div4(r)];
*dest8a++ = PIX444(ri,gi,bi);
b -= scale4[bi]; g -= scale8[gi]; r -= scale7[ri];

bi = index4[div3(b)]; gi = index4[div3(g)]; ri = index4[div3(r)];
*dest8a++ = PIX444(ri,gi,bi);
b -= scale4[bi]; g -= scale8[gi]; r -= scale7[ri];

bi = index4[div2(b)]; gi = index4[div2(g)]; ri = index4[div2(r)];
*dest8b++ = PIX444(ri,gi,bi);
b -= scale4[bi]; g -= scale8[gi]; r -= scale7[ri];

bi = index4[b      ]; gi = index4[g      ]; ri = index4[r      ];
*dest8b++ = PIX444(ri,gi,bi);

tmp = dest8a; dest8a = dest8b; dest8b = tmp;
}
/*...e*/
		if ( x < gbm->w )
/*...s1x2 case:24:*/
{
word r,g,b;
byte ri, gi, bi;

b  = *src24a++;	g  = *src24a++;	r  = *src24a;
b += *src24b++;	g += *src24b++;	r += *src24b;

bi = index4[div2(b)]; gi = index4[div2(g)]; ri = index4[div2(r)];
*dest8a = PIX444(ri,gi,bi);
b -= scale4[bi]; g -= scale8[gi]; r -= scale7[ri];

bi = index4[b      ]; gi = index4[g      ]; ri = index4[r      ];
*dest8b = PIX444(ri,gi,bi);
}
/*...e*/
		}
	if ( y < gbm->h )
		{
		const byte *src24a = src24 + stride24;
		      byte *dest8a = dest8 + stride8;

		for ( x = 0; x < gbm->w - 1; x += 2 )
/*...s2x1 case:24:*/
{
word r,g,b;
byte ri, gi, bi;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;

bi = index4[div2(b)]; gi = index4[div2(g)]; ri = index4[div2(r)];
*dest8a++ = PIX444(ri,gi,bi);
b -= scale4[bi]; g -= scale8[gi]; r -= scale7[ri];

bi = index4[b      ]; gi = index4[g      ]; ri = index4[r      ];
*dest8a++ = PIX444(ri,gi,bi);
}
/*...e*/
		if ( x < gbm->w )
/*...s1x1 case:24:*/
{
byte ri, gi, bi;

bi = index4[*src24a++]; gi = index4[*src24a++]; ri = index4[*src24a];
*dest8a = PIX444(ri,gi,bi);
}
/*...e*/
		}
	}
/*...e*/

/*...sgbm_ht_pal_VGA    \45\ return default VGA palette:0:*/
/*
This function makes the palette for the 16 colour VGA palette.
*/

void gbm_ht_pal_VGA(GBMRGB *gbmrgb)
	{
	init();
	memcpy((char *) gbmrgb, (char *) gbmrgb_vga, sizeof(gbmrgb_vga));
	}
/*...e*/
/*...sgbm_ht_VGA_2x2    \45\ halftone by 2x2 to default VGA palette:0:*/
void gbm_ht_VGA_2x2(const GBM *gbm, const byte *src24, byte *dest4)
	{
	int stride24 = ((gbm->w * 3 + 3) & ~3);
	int stride4  = ((gbm->w * 4 + 31) / 32) * 4;
	int x, y;

	init();

	src24 -= stride24;
	dest4 -= stride4;

	for ( y = 0; y < gbm->h - 1; y += 2 )
		{
		const byte *src24a = (src24 += stride24);
		const byte *src24b = (src24 += stride24);
		      byte *dest4a = (dest4 += stride4);
		      byte *dest4b = (dest4 += stride4);

		for ( x = 0; x < gbm->w - 1; x += 2 )
/*...s2x2 case:24:*/
{
word r,g,b;
byte inx;
byte *tmp;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;

inx = takefrom(div4(r), div4(g), div4(b)); *dest4a    = (inx << 4);
b -= gbmrgb_vga[inx].b; g -= gbmrgb_vga[inx].g; r -= gbmrgb_vga[inx].r;

inx = takefrom(div3(r), div3(g), div3(b)); *dest4a++ |= inx;
b -= gbmrgb_vga[inx].b; g -= gbmrgb_vga[inx].g; r -= gbmrgb_vga[inx].r;

inx = takefrom(div2(r), div2(g), div2(b)); *dest4b    = (inx << 4);
b -= gbmrgb_vga[inx].b; g -= gbmrgb_vga[inx].g; r -= gbmrgb_vga[inx].r;

inx = takefrom(r      , g      , b      ); *dest4b++ |= inx;

tmp = dest4a; dest4a = dest4b; dest4b = tmp;
}
/*...e*/
		if ( x < gbm->w )
/*...s1x2 case:24:*/
{
word r,g,b;
byte inx;

b  = *src24a++;	g  = *src24a++;	r  = *src24a;
b += *src24b++;	g += *src24b++;	r += *src24b;

inx = takefrom(div2(r), div2(g), div2(b)); *dest4a = (inx << 4);
b -= gbmrgb_vga[inx].b; g -= gbmrgb_vga[inx].g; r -= gbmrgb_vga[inx].r;

inx = takefrom(r      , g      , b      ); *dest4b = (inx << 4);
}
/*...e*/
		}
	if ( y < gbm->h )
		{
		const byte *src24a = src24 + stride24;
		      byte *dest4a = dest4 + stride4;

		for ( x = 0; x < gbm->w - 1; x += 2 )
/*...s2x1 case:24:*/
{
word r,g,b;
byte inx;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;

inx = takefrom(div2(r), div2(g), div2(b)); *dest4a = (inx << 4);
b -= gbmrgb_vga[inx].b; g -= gbmrgb_vga[inx].g; r -= gbmrgb_vga[inx].r;

inx = takefrom(r      , g      , b      ); *dest4a++ |= inx;
}
/*...e*/
		if ( x < gbm->w )
/*...s1x1 case:24:*/
{
word r, g, b;
byte inx;

b = *src24a++; g = *src24a++; r = *src24a;
inx = takefrom(r, g, b); *dest4a = (inx << 4);
}
/*...e*/
		}
	}
/*...e*/
/*...sgbm_ht_VGA_3x3    \45\ halftone by 3x3 to default VGA palette:0:*/
void gbm_ht_VGA_3x3(const GBM *gbm, const byte *src24, byte *dest4)
	{
	int stride24 = ((gbm->w * 3 + 3) & ~3);
	int stride4  = ((gbm->w * 4 + 31) / 32) * 4;
	int x, y;

	init();

	src24 -= stride24;
	dest4 -= stride4;

	for ( y = 0; y < gbm->h - 2; y += 3 )
		{
		const byte *src24a = (src24 += stride24);
		const byte *src24b = (src24 += stride24);
		const byte *src24c = (src24 += stride24);
		      byte *dest4a = (dest4 += stride4);
		      byte *dest4b = (dest4 += stride4);
		      byte *dest4c = (dest4 += stride4);
		BOOLEAN left = TRUE;

		for ( x = 0; x < gbm->w - 2; x += 3, left = !left )
/*...s3x3 case:24:*/
{
word r,g,b;
byte inxs[9];
byte *tmp;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24c++;	g += *src24c++;	r += *src24c++;
b += *src24c++;	g += *src24c++;	r += *src24c++;
b += *src24c++;	g += *src24c++;	r += *src24c++;

split_into(r, g, b, 9, inxs);

if ( left )
	{
	*dest4a++ = ((inxs[0] << 4) | inxs[1]); *dest4a = (inxs[2] << 4);
	*dest4b++ = ((inxs[3] << 4) | inxs[4]); *dest4b = (inxs[5] << 4);
	*dest4c++ = ((inxs[6] << 4) | inxs[7]); *dest4c = (inxs[8] << 4);
	}
else
	{
	*dest4a++ |= inxs[0]; *dest4a++ = ((inxs[1] << 4) | inxs[2]);
	*dest4b++ |= inxs[3]; *dest4b++ = ((inxs[4] << 4) | inxs[5]);
	*dest4c++ |= inxs[6]; *dest4c++ = ((inxs[7] << 4) | inxs[8]);
	}

tmp = dest4a; dest4a = dest4b; dest4b = dest4c; dest4c = tmp;
}
/*...e*/
		if ( x < gbm->w - 1 )
/*...s2x3 case:24:*/
{
word r,g,b;
byte inxs[6];

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24c++;	g += *src24c++;	r += *src24c++;
b += *src24c++;	g += *src24c++;	r += *src24c++;

split_into(r, g, b, 6, inxs);

if ( left )
	{
	*dest4a = ((inxs[0] << 4) | inxs[1]);
	*dest4b = ((inxs[2] << 4) | inxs[3]);
	*dest4c = ((inxs[4] << 4) | inxs[5]);
	}
else
	{
	*dest4a++ |= inxs[0]; *dest4a = (inxs[1] << 4);
	*dest4b++ |= inxs[2]; *dest4b = (inxs[3] << 4);
	*dest4c++ |= inxs[4]; *dest4c = (inxs[5] << 4);
	}
}
/*...e*/
		else if ( x < gbm->w )
/*...s1x3 case:24:*/
{
word r,g,b;
byte inxs[3];

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24c++;	g += *src24c++;	r += *src24c++;

split_into(r, g, b, 3, inxs);

if ( left )
	{
	*dest4a = (inxs[0] << 4);
	*dest4b = (inxs[1] << 4);
	*dest4c = (inxs[2] << 4);
	}
else
	{
	*dest4a |= inxs[0];
	*dest4b |= inxs[1];
	*dest4c |= inxs[2];
	}
}
/*...e*/
		}
	if ( y < gbm->h - 1 )
		{
		const byte *src24a = (src24 += stride24);
		const byte *src24b = (src24 += stride24);
		      byte *dest4a = (dest4 += stride4);
		      byte *dest4b = (dest4 += stride4);
		BOOLEAN left = TRUE;

		for ( x = 0; x < gbm->w - 2; x += 3, left = !left )
/*...s3x2 case:24:*/
{
word r,g,b;
byte inxs[6];
byte *tmp;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;

split_into(r, g, b, 6, inxs);

if ( left )
	{
	*dest4a++ = (inxs[0] << 4) | inxs[1]; *dest4a = (inxs[2] << 4);
	*dest4b++ = (inxs[3] << 4) | inxs[4]; *dest4b = (inxs[5] << 4);
	}
else
	{
	*dest4a++ |= inxs[0]; *dest4a++ = ((inxs[1] << 4) | inxs[2]);
	*dest4b++ |= inxs[3]; *dest4b++ = ((inxs[4] << 4) | inxs[5]);
	}

tmp = dest4a; dest4a = dest4b; dest4b = tmp;
}
/*...e*/
		if ( x < gbm->w - 1 )
/*...s2x2 case:24:*/
{
word r,g,b;
byte inxs[4];

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;

split_into(r, g, b, 4, inxs);

if ( left )
	{
	*dest4a = ((inxs[0] << 4) | inxs[1]);
	*dest4b = ((inxs[2] << 4) | inxs[3]);
	}
else
	{
	*dest4a++ |= inxs[0]; *dest4a = (inxs[1] << 4);
	*dest4b++ |= inxs[2]; *dest4b = (inxs[3] << 4);
	}
}
/*...e*/
		else if ( x < gbm->w )
/*...s1x2 case:24:*/
{
word r,g,b;
byte inxs[2];

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;

split_into(r, g, b, 2, inxs);

if ( left )
	{
	*dest4a = (inxs[0] << 4);
	*dest4b = (inxs[1] << 4);
	}
else
	{
	*dest4a |= inxs[0];
	*dest4b |= inxs[1];
	}
}
/*...e*/
		}
	else if ( y < gbm->h )
		{
		const byte *src24a = (src24 += stride24);
		      byte *dest4a = (dest4 += stride4);
		BOOLEAN left = TRUE;

		for ( x = 0; x < gbm->w - 2; x += 3, left = !left )
/*...s3x1 case:24:*/
{
word r,g,b;
byte inxs[3];

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;

split_into(r, g, b, 3, inxs);

if ( left )
	{
	*dest4a++ = ((inxs[0] << 4) | inxs[1]); *dest4a = (inxs[2] << 4);
	}
else
	{
	*dest4a++ |= inxs[0]; *dest4a++ = ((inxs[1] << 4) | inxs[2]);
	}
}
/*...e*/
		if ( x < gbm->w - 1 )
/*...s2x1 case:24:*/
{
word r,g,b;
byte inxs[2];

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;

split_into(r, g, b, 4, inxs);

if ( left )
	*dest4a = ((inxs[0] << 4) | inxs[1]);
else
	{
	*dest4a++ |= inxs[0]; *dest4a = (inxs[1] << 4);
	}
}
/*...e*/
		else if ( x < gbm->w )
/*...s1x1 case:24:*/
{
word r,g,b;
byte inx;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;

inx = takefrom(r, g, b);

if ( left )
	*dest4a = (inx << 4);
else
	*dest4a |= inx;
}
/*...e*/
		}
	}
/*...e*/

/*...sgbm_ht_pal_8      \45\ return default 8 colour palette:0:*/
/*
This function makes the palette for the 8 colour palette.
*/

void gbm_ht_pal_8(GBMRGB *gbmrgb)
	{
	init();
	memcpy((char *) gbmrgb, (char *) gbmrgb_8, sizeof(gbmrgb_8));
	}
/*...e*/
/*...sgbm_ht_8_2x2      \45\ halftone by 2x2 to default 8 colour palette:0:*/
void gbm_ht_8_2x2(const GBM *gbm, const byte *src24, byte *dest4)
	{
	int stride24 = ((gbm->w * 3 + 3) & ~3);
	int stride4  = ((gbm->w * 4 + 31) / 32) * 4;
	int x, y;

	init();

	src24 -= stride24;
	dest4 -= stride4;

	for ( y = 0; y < gbm->h - 1; y += 2 )
		{
		const byte *src24a = (src24 += stride24);
		const byte *src24b = (src24 += stride24);
		      byte *dest4a = (dest4 += stride4);
		      byte *dest4b = (dest4 += stride4);

		for ( x = 0; x < gbm->w - 1; x += 2 )
/*...s2x2 case:24:*/
{
word r,g,b;
byte inx;
byte *tmp;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;

inx = takefrom8(r >> 2, g >> 2, b >> 2); *dest4a    = (inx << 4);
b -= gbmrgb_8[inx].b; g -= gbmrgb_8[inx].g; r -= gbmrgb_8[inx].r;

inx = takefrom8(r  / 3, g  / 3, b  / 3); *dest4a++ |= inx;
b -= gbmrgb_8[inx].b; g -= gbmrgb_8[inx].g; r -= gbmrgb_8[inx].r;

inx = takefrom8(r >> 1, g >> 1, b >> 1); *dest4b    = (inx << 4);
b -= gbmrgb_8[inx].b; g -= gbmrgb_8[inx].g; r -= gbmrgb_8[inx].r;

inx = takefrom8(r     , g     , b     ); *dest4b++ |= inx;

tmp = dest4a; dest4a = dest4b; dest4b = tmp;
}
/*...e*/
		if ( x < gbm->w )
/*...s1x2 case:24:*/
{
word r,g,b;
byte inx;

b  = *src24a++;	g  = *src24a++;	r  = *src24a;
b += *src24b++;	g += *src24b++;	r += *src24b;

inx = takefrom8(r >> 1, g >> 1, b >> 1); *dest4a = (inx << 4);
b -= gbmrgb_8[inx].b; g -= gbmrgb_8[inx].g; r -= gbmrgb_8[inx].r;

inx = takefrom8(r     , g     , b     ); *dest4b = (inx << 4);
}
/*...e*/
		}
	if ( y < gbm->h )
		{
		const byte *src24a = src24 + stride24;
		      byte *dest4a = dest4 + stride4;

		for ( x = 0; x < gbm->w - 1; x += 2 )
/*...s2x1 case:24:*/
{
word r,g,b;
byte inx;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;

inx = takefrom8(r >> 1, g >> 1, b >> 1); *dest4a    = (inx << 4);
b -= gbmrgb_8[inx].b; g -= gbmrgb_8[inx].g; r -= gbmrgb_8[inx].r;

inx = takefrom8(r     , g     , b     ); *dest4a++ |= inx;
}
/*...e*/
		if ( x < gbm->w )
/*...s1x1 case:24:*/
{
word r, g, b;
byte inx;

b = *src24a++; g = *src24a++; r = *src24a;
inx = takefrom8(r, g, b); *dest4a = (inx << 4);
}
/*...e*/
		}
	}
/*...e*/
/*...sgbm_ht_8_3x3      \45\ halftone by 3x3 to default 8 colour palette:0:*/
void gbm_ht_8_3x3(const GBM *gbm, const byte *src24, byte *dest4)
	{
	int stride24 = ((gbm->w * 3 + 3) & ~3);
	int stride4  = ((gbm->w * 4 + 31) / 32) * 4;
	int x, y;

	init();

	src24 -= stride24;
	dest4 -= stride4;

	for ( y = 0; y < gbm->h - 2; y += 3 )
		{
		const byte *src24a = (src24 += stride24);
		const byte *src24b = (src24 += stride24);
		const byte *src24c = (src24 += stride24);
		      byte *dest4a = (dest4 += stride4);
		      byte *dest4b = (dest4 += stride4);
		      byte *dest4c = (dest4 += stride4);
		BOOLEAN left = TRUE;

		for ( x = 0; x < gbm->w - 2; x += 3, left = !left )
/*...s3x3 case:24:*/
{
word r,g,b;
byte inxs[9];
byte *tmp;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24c++;	g += *src24c++;	r += *src24c++;
b += *src24c++;	g += *src24c++;	r += *src24c++;
b += *src24c++;	g += *src24c++;	r += *src24c++;

split_into8(r, g, b, 9, inxs);

if ( left )
	{
	*dest4a++ = ((inxs[0] << 4) | inxs[1]); *dest4a = (inxs[2] << 4);
	*dest4b++ = ((inxs[3] << 4) | inxs[4]); *dest4b = (inxs[5] << 4);
	*dest4c++ = ((inxs[6] << 4) | inxs[7]); *dest4c = (inxs[8] << 4);
	}
else
	{
	*dest4a++ |= inxs[0]; *dest4a++ = ((inxs[1] << 4) | inxs[2]);
	*dest4b++ |= inxs[3]; *dest4b++ = ((inxs[4] << 4) | inxs[5]);
	*dest4c++ |= inxs[6]; *dest4c++ = ((inxs[7] << 4) | inxs[8]);
	}

tmp = dest4a; dest4a = dest4b; dest4b = dest4c; dest4c = tmp;
}
/*...e*/
		if ( x < gbm->w - 1 )
/*...s2x3 case:24:*/
{
word r,g,b;
byte inxs[6];

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24c++;	g += *src24c++;	r += *src24c++;
b += *src24c++;	g += *src24c++;	r += *src24c++;

split_into8(r, g, b, 6, inxs);

if ( left )
	{
	*dest4a = ((inxs[0] << 4) | inxs[1]);
	*dest4b = ((inxs[2] << 4) | inxs[3]);
	*dest4c = ((inxs[4] << 4) | inxs[5]);
	}
else
	{
	*dest4a++ |= inxs[0]; *dest4a = (inxs[1] << 4);
	*dest4b++ |= inxs[2]; *dest4b = (inxs[3] << 4);
	*dest4c++ |= inxs[4]; *dest4c = (inxs[5] << 4);
	}
}
/*...e*/
		else if ( x < gbm->w )
/*...s1x3 case:24:*/
{
word r,g,b;
byte inxs[3];

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24c++;	g += *src24c++;	r += *src24c++;

split_into8(r, g, b, 3, inxs);

if ( left )
	{
	*dest4a = (inxs[0] << 4);
	*dest4b = (inxs[1] << 4);
	*dest4c = (inxs[2] << 4);
	}
else
	{
	*dest4a |= inxs[0];
	*dest4b |= inxs[1];
	*dest4c |= inxs[2];
	}
}
/*...e*/
		}
	if ( y < gbm->h - 1 )
		{
		const byte *src24a = (src24 += stride24);
		const byte *src24b = (src24 += stride24);
		      byte *dest4a = (dest4 += stride4);
		      byte *dest4b = (dest4 += stride4);
		BOOLEAN left = TRUE;

		for ( x = 0; x < gbm->w - 2; x += 3, left = !left )
/*...s3x2 case:24:*/
{
word r,g,b;
byte inxs[6];
byte *tmp;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;

split_into8(r, g, b, 6, inxs);

if ( left )
	{
	*dest4a++ = (inxs[0] << 4) | inxs[1]; *dest4a = (inxs[2] << 4);
	*dest4b++ = (inxs[3] << 4) | inxs[4]; *dest4b = (inxs[5] << 4);
	}
else
	{
	*dest4a++ |= inxs[0]; *dest4a++ = ((inxs[1] << 4) | inxs[2]);
	*dest4b++ |= inxs[3]; *dest4b++ = ((inxs[4] << 4) | inxs[5]);
	}

tmp = dest4a; dest4a = dest4b; dest4b = tmp;
}
/*...e*/
		if ( x < gbm->w - 1 )
/*...s2x2 case:24:*/
{
word r,g,b;
byte inxs[4];

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;
b += *src24b++;	g += *src24b++;	r += *src24b++;

split_into8(r, g, b, 4, inxs);

if ( left )
	{
	*dest4a = ((inxs[0] << 4) | inxs[1]);
	*dest4b = ((inxs[2] << 4) | inxs[3]);
	}
else
	{
	*dest4a++ |= inxs[0]; *dest4a = (inxs[1] << 4);
	*dest4b++ |= inxs[2]; *dest4b = (inxs[3] << 4);
	}
}
/*...e*/
		else if ( x < gbm->w )
/*...s1x2 case:24:*/
{
word r,g,b;
byte inxs[2];

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24b++;	g += *src24b++;	r += *src24b++;

split_into8(r, g, b, 2, inxs);

if ( left )
	{
	*dest4a = (inxs[0] << 4);
	*dest4b = (inxs[1] << 4);
	}
else
	{
	*dest4a |= inxs[0];
	*dest4b |= inxs[1];
	}
}
/*...e*/
		}
	else if ( y < gbm->h )
		{
		const byte *src24a = (src24 += stride24);
		      byte *dest4a = (dest4 += stride4);
		BOOLEAN left = TRUE;

		for ( x = 0; x < gbm->w - 2; x += 3, left = !left )
/*...s3x1 case:24:*/
{
word r,g,b;
byte inxs[3];

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;

split_into8(r, g, b, 3, inxs);

if ( left )
	{
	*dest4a++ = ((inxs[0] << 4) | inxs[1]); *dest4a = (inxs[2] << 4);
	}
else
	{
	*dest4a++ |= inxs[0]; *dest4a++ = ((inxs[1] << 4) | inxs[2]);
	}
}
/*...e*/
		if ( x < gbm->w - 1 )
/*...s2x1 case:24:*/
{
word r,g,b;
byte inxs[2];

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;
b += *src24a++;	g += *src24a++;	r += *src24a++;

split_into8(r, g, b, 4, inxs);

if ( left )
	*dest4a = ((inxs[0] << 4) | inxs[1]);
else
	{
	*dest4a++ |= inxs[0]; *dest4a = (inxs[1] << 4);
	}
}
/*...e*/
		else if ( x < gbm->w )
/*...s1x1 case:24:*/
{
word r,g,b;
byte inx;

b  = *src24a++;	g  = *src24a++;	r  = *src24a++;

inx = takefrom8(r, g, b);

if ( left )
	*dest4a = (inx << 4);
else
	*dest4a |= inx;
}
/*...e*/
		}
	}
/*...e*/
