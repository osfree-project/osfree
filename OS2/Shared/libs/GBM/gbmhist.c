/*

gbmhist.c - Histogram/Frequency-of-use method of colour reduction

*/

/*...sincludes:0:*/
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"

/*...vgbm\46\h:0:*/
/*...e*/

#define	N_COLS	2049
#define	N_HASH	5191
#define	HASH(r,g,b)	(word) ( (((r)+(g))*((g)+(b))*((b)+(r))) % N_HASH )

typedef struct { byte b, g, r; dword freq; byte nearest; } FREQ;

typedef struct
	{
	int n_cols;
	byte rm, gm, bm;
	FREQ f[N_COLS];
	word ht[N_HASH];
	} GBMHIST;

/*...sgbm_create_hist \45\ create empty hist:0:*/
GBMHIST *gbm_create_hist(
	byte rm, byte gm, byte bm
	)
	{
	GBMHIST *hist;

	if ( (hist = malloc((size_t) sizeof(GBMHIST))) == NULL )
		return NULL;
	hist->rm = rm;
	hist->gm = gm;
	hist->bm = bm;
	hist->n_cols = 0;
	memset(hist->ht, 0xff, N_HASH * sizeof(word));
	return hist;
	}
/*...e*/
/*...sgbm_delete_hist \45\ delete hist:0:*/
void gbm_delete_hist(GBMHIST *hist)
	{
	free(hist);
	}
/*...e*/
/*...sgbm_add_to_hist \45\ add bitmap data to hist:0:*/
BOOLEAN gbm_add_to_hist(
	GBMHIST *hist,
	const GBM *gbm, const byte *data24
	)
	{
	int stride24 = ((gbm->w * 3 + 3) & ~3);
	int step24   = stride24 - gbm->w * 3;
	FREQ *f  = hist->f ;
	word *ht = hist->ht;
	byte rm = hist->rm;
	byte gm = hist->gm;
	byte bm = hist->bm;
	int x, y, n_cols = hist->n_cols;

	for ( y = 0; y < gbm->h; y++, data24 += step24 )
		for ( x = 0; x < gbm->w; x++ )
			{
			byte b = (byte) (*data24++ & bm);
			byte g = (byte) (*data24++ & gm);
			byte r = (byte) (*data24++ & rm);
			word hc = HASH(r,g,b);
			word inx;

			for ( ;; )
				{
				inx = ht[hc];
				if ( inx == 0xffff ||
				     (f[inx].r == r &&
				      f[inx].g == g &&
				      f[inx].b == b) )
					break;
				if ( ++hc == N_HASH ) hc = 0;
				}

			/* Note: loop will always be broken out of */
			/* We don't allow ht to fill up above half full */

			if ( inx == 0xffff )
				/* Not found in hash table */
				{
				if ( n_cols == N_COLS )
					return FALSE;
				f[n_cols].freq = (dword) 1;
				f[n_cols].b    = b;
				f[n_cols].g    = g;
				f[n_cols].r    = r;
				ht[hc] = n_cols++;
				}
			else
				/* Found in hash table */
				/* update index inx */
				f[inx].freq++;
			}
	hist->n_cols = n_cols;
	return TRUE;
	}
/*...e*/
/*...sgbm_pal_hist    \45\ work out a palette from hist:0:*/
void gbm_pal_hist(
	GBMHIST *hist,
	GBMRGB gbmrgb[],
	int n_cols_wanted
	)
	{
	FREQ *f  = hist->f;
	int i;

	/* Now find the n_cols_wanted most frequently used ones */

	for ( i = 0; i < n_cols_wanted && i < hist->n_cols; i++ )
		{
		int j, max_j;
		dword max_freq = 0;

		for ( j = 0; j < hist->n_cols; j++ )
			if ( f[j].freq > max_freq )
				{
				max_j    = j;
				max_freq = f[j].freq;
				}
		f[max_j].nearest = (byte) i;
		f[max_j].freq = (dword) 0; /* Prevent later use of f[max_j] */
		gbmrgb[i].b = f[max_j].b;
		gbmrgb[i].g = f[max_j].g;
		gbmrgb[i].r = f[max_j].r;
		}

	/* Unused palette entries will be medium grey */
	for ( ; i < 0x100; i++ )
		{
		gbmrgb[i].r = 0x80;
		gbmrgb[i].g = 0x80;
		gbmrgb[i].b = 0x80;
		}

	/* For the rest, find the closest one in the first n_cols_wanted */

	for ( i = 0; i < hist->n_cols; i++ )
		if ( f[i].freq != (dword) 0 )
			{
			int j, min_j;
			int min_dist = 3*256*256;

			for ( j = 0; j < n_cols_wanted; j++ )
				{
				int db = (int) f[i].b - (int) gbmrgb[j].b;
				int dg = (int) f[i].g - (int) gbmrgb[j].g;
				int dr = (int) f[i].r - (int) gbmrgb[j].r;
				int dist = dr*dr + dg*dg + db*db;

				if ( dist < min_dist )
					{
					min_dist = dist;
					min_j    = j;
					}
				}
			f[i].nearest = (byte) min_j;
			}
	}
/*...e*/
/*...sgbm_map_hist    \45\ map bitmap data to hist palette:0:*/
void gbm_map_hist(
	GBMHIST *hist,
	const GBM *gbm, const byte *data24, byte *data8
	)
	{
	int stride24 = ((gbm->w * 3 + 3) & ~3);
	int step24   = stride24 - gbm->w * 3;
	int stride8  = ((gbm->w + 3) & ~3);
	int step8    = stride8 - gbm->w;
	FREQ *f  = hist->f;
	word *ht = hist->ht;
	byte rm = hist->rm;
	byte gm = hist->gm;
	byte bm = hist->bm;
	int x, y;

	for ( y = 0; y < gbm->h; y++, data24 += step24, data8 += step8 )
		for ( x = 0; x < gbm->w; x++ )
			{
			byte b = (*data24++ & bm);
			byte g = (*data24++ & gm);
			byte r = (*data24++ & rm);
			word hc = HASH(r,g,b);
			word inx;

			for ( ;; )
				{
				inx = ht[hc];
				if ( f[inx].r == r && f[inx].g == g && f[inx].b == b )
					break;
				if ( ++hc == N_HASH ) hc = 0;
				}

			*data8++ = f[inx].nearest;
			}
	}
/*...e*/
/*...sgbm_hist        \45\ map single bitmap to frequency optimised palette:0:*/
/*
Determine the n_cols_wanted most frequently used colours from 24 bit data.
Can be a problem since potentially 256*256*256 possible unique colours.
Initially 8 bits green, 8 bits red, and 8 bits blue significant.
When number of colours exceeds a limit number of bits of blue reduced by 1.
Next time red, next time green, ...
Sort most n_cols_wanted most frequently used colour in order of use.
Put these in the returned palette.
Map colours from n_cols_wanted exactly to colours in palette.
For other colours, map them to the closest in the palette.
*/

BOOLEAN gbm_hist(
	const GBM *gbm, const byte *data24,
	GBMRGB gbmrgb[],
	byte *data8,
	int n_cols_wanted,
	byte rm, byte gm, byte bm
	)
	{
	GBMHIST *hist;

	for ( ;; )
		{
		if ( (hist = gbm_create_hist(rm, gm, bm)) == NULL )
			return FALSE;

		if ( gbm_add_to_hist(hist, gbm, data24) )
			break;

		gbm_delete_hist(hist);

		if ( gm > rm )
			gm <<= 1;
		else if ( rm > bm )
			rm <<= 1;
		else
			bm <<= 1;
		}

	/* Above loop will always be exited as if masks get rough
	   enough, ultimately number of unique colours < N_COLS */

	gbm_pal_hist(hist, gbmrgb, n_cols_wanted);
	gbm_map_hist(hist, gbm, data24, data8);
	gbm_delete_hist(hist);
	return TRUE;
	}
/*...e*/
