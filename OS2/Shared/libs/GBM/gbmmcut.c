/*

gbmmcut.c - Median Cut colour reductions

*/

/*...sincludes:0:*/
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"

/*...vgbm\46\h:0:*/
/*...e*/

#define	DIV_R 4
#define	DIV_G 2
#define	DIV_B 1

typedef struct
	{
	dword freq;
	byte r0,r1,g0,g1,b0,b1;
	byte dividable;
	} CELL;

typedef struct
	{
	dword freqs[0x20][0x20][0x20]; /* 128Kb */
	dword total;
	int n_cells;
	CELL cells[0x100];
	} GBMMCUT;

/*...sgbm_create_mcut \45\ create empty mcut:0:*/
GBMMCUT *gbm_create_mcut(void)
	{
	GBMMCUT *mcut;

	if ( (mcut = malloc((size_t) sizeof(GBMMCUT))) == NULL )
		return NULL;

	memset(mcut->freqs, 0x00, sizeof(mcut->freqs));
	mcut->total = 0;
	return mcut;
	}
/*...e*/
/*...sgbm_delete_mcut \45\ delete mcut:0:*/
void gbm_delete_mcut(GBMMCUT *mcut)
	{
	free(mcut);
	}
/*...e*/
/*...sgbm_add_to_mcut \45\ add statistics from file data:0:*/
void gbm_add_to_mcut(
	GBMMCUT *mcut,	
	const GBM *gbm, const byte *data24
	)
	{
	int stride24 = ((gbm->w * 3 + 3) & ~3);
	int step24   = stride24 - gbm->w * 3;
	int x, y;

	for ( y = 0; y < gbm->h; y++, data24 += step24 )
		for ( x = 0; x < gbm->w; x++ )
			{
			byte b = (byte) (*data24++ >> 3);
			byte g = (byte) (*data24++ >> 3);
			byte r = (byte) (*data24++ >> 3);

			( mcut->freqs[b][g][r] )++;
			}
	mcut->total += ( gbm->w * gbm->h );
	}
/*...e*/
/*...sgbm_pal_mcut    \45\ build median palette via median cut:0:*/
/*...sshrink:0:*/
/* Apologies for use of 'goto'
   In this case, its considered appropriate. */

static void shrink(GBMMCUT *mcut, CELL *c)
	{
	byte r, g, b;
	
	for ( ;; c->r0++ )
		for ( g = c->g0; g < c->g1; g++ )
			for ( b = c->b0; b < c->b1; b++ )
				if ( mcut->freqs[b][g][c->r0] )
					goto quit_r0;
quit_r0:

	for ( ; c->r1-c->r0 > 1; c->r1-- )
		for ( g = c->g0; g < c->g1; g++ )
			for ( b = c->b0; b < c->b1; b++ )
				if ( mcut->freqs[b][g][c->r1-1] )
					goto quit_r1;
quit_r1:
	
	for ( ;; c->g0++ )
		for ( r = c->r0; r < c->r1; r++ )
			for ( b = c->b0; b < c->b1; b++ )
				if ( mcut->freqs[b][c->g0][r] )
					goto quit_g0;
quit_g0:

	for ( ; c->g1-c->g0 > 1; c->g1-- )
		for ( r = c->r0; r < c->r1; r++ )
			for ( b = c->b0; b < c->b1; b++ )
				if ( mcut->freqs[b][c->g1-1][r] )
					goto quit_g1;
quit_g1:
	
	for ( ;; c->b0++ )
		for ( r = c->r0; r < c->r1; r++ )
			for ( g = c->g0; g < c->g1; g++ )
				if ( mcut->freqs[c->b0][g][r] )
					goto quit_b0;
quit_b0:

	for ( ; c->b1-c->b0 > 1; c->b1-- )
		for ( r = c->r0; r < c->r1; r++ )
			for ( g = c->g0; g < c->g1; g++ )
				if ( mcut->freqs[c->b1-1][g][r] )
					goto quit_b1;
quit_b1:

	c->dividable = ( ( c->r1-c->r0 > 1 ) ? DIV_R : 0 ) +
		       ( ( c->g1-c->g0 > 1 ) ? DIV_G : 0 ) +
		       ( ( c->b1-c->b0 > 1 ) ? DIV_B : 0 ) ;
	}
/*...e*/

void gbm_pal_mcut(
	GBMMCUT *mcut,
	GBMRGB gbmrgb[],
	int n_cols_wanted
	)
	{
	CELL *c = mcut->cells;
	int i, j;
	byte reorder[0x100];

	if ( n_cols_wanted > 0x100 )
		n_cols_wanted = 0x100;

	/* Initially, a single cell covers the whole colour cube */

	c->r0 = c->g0 = c->b0 =    0;
	c->r1 = c->g1 = c->b1 = 0x20;
	c->freq = mcut->total;
	shrink(mcut, c);
	mcut->n_cells = 1;

	/* Do the following until got as many colours (cells) as reqd. */

	while ( mcut->n_cells < n_cols_wanted )
		{
		CELL *cmax = NULL;

/*...sfind cell with most pixels in it\44\ that can be divided:16:*/
{
int j;
dword freqmax = 1;

for ( j = 0; j < mcut->n_cells; j++ )
	if ( c[j].freq > freqmax && c[j].dividable )
		{
		cmax = &(c[j]);
		freqmax = cmax->freq;
		}
}
/*...e*/
		if ( cmax == NULL )
			break;

		while ( cmax->dividable )
			{
			byte split;
			CELL *cnew = &(c[mcut->n_cells]);

/*...scalculate way to do the split:24:*/
{
int dr = (cmax->dividable&DIV_R) ? cmax->r1 - cmax->r0 : 0;
int dg = (cmax->dividable&DIV_G) ? cmax->g1 - cmax->g0 : 0;
int db = (cmax->dividable&DIV_B) ? cmax->b1 - cmax->b0 : 0;

if ( dg >= dr && dg >= db )
	split = DIV_G;
else if ( dr >= db )
	split = DIV_R;
else 
	split = DIV_B;
}
/*...e*/
			switch ( split )
				{
/*...sDIV_R:32:*/
case DIV_R:
	{
	byte r, g, b;
	dword slice, total = 0;

	for ( r = cmax->r0; total < (cmax->freq>>1); r++ )
		{
		slice = 0;
		for ( g = cmax->g0; g < cmax->g1; g++ )
			for ( b = cmax->b0; b < cmax->b1; b++ )
				slice += mcut->freqs[b][g][r];
		total += slice;
		}

	if ( r == cmax->r1 && total > slice )
		{
		r--;
		total -= slice;
		}

	cnew->r1 = cmax->r1;
	cnew->r0 = cmax->r1 = r;
	cnew->g0 = cmax->g0;
	cnew->g1 = cmax->g1;
	cnew->b0 = cmax->b0;
	cnew->b1 = cmax->b1;
	cnew->freq = cmax->freq - total;
	cmax->freq = total;
	}
	break;
/*...e*/
/*...sDIV_G:32:*/
case DIV_G:
	{
	byte r, g, b;
	dword slice, total = 0;

	for ( g = cmax->g0; total < (cmax->freq>>1); g++ )
		{
		slice = 0;
		for ( r = cmax->r0; r < cmax->r1; r++ )
			for ( b = cmax->b0; b < cmax->b1; b++ )
				slice += mcut->freqs[b][g][r];
		total += slice;
		}

	if ( g == cmax->g1 && total > slice )
		{
		g--;
		total -= slice;
		}

	cnew->r0 = cmax->r0;
	cnew->r1 = cmax->r1;
	cnew->g1 = cmax->g1;
	cnew->g0 = cmax->g1 = g;
	cnew->b0 = cmax->b0;
	cnew->b1 = cmax->b1;
	cnew->freq = cmax->freq - total;
	cmax->freq = total;
	}
	break;
/*...e*/
/*...sDIV_B:32:*/
case DIV_B:
	{
	byte r, g, b;
	dword slice, total = 0;

	for ( b = cmax->b0; total < (cmax->freq>>1); b++ )
		{
		slice = 0;
		for ( r = cmax->r0; r < cmax->r1; r++ )
			for ( g = cmax->g0; g < cmax->g1; g++ )
				slice += mcut->freqs[b][g][r];
		total += slice;
		}

	if ( b == cmax->b1 && total > slice )
		{
		b--;
		total -= slice;
		}

	cnew->r0 = cmax->r0;
	cnew->r1 = cmax->r1;
	cnew->g0 = cmax->g0;
	cnew->g1 = cmax->g1;
	cnew->b1 = cmax->b1;
	cnew->b0 = cmax->b1 = b;
	cnew->freq = cmax->freq - total;
	cmax->freq = total;
	}
	break;
/*...e*/
				}
			if ( cnew->freq > 0 )
				{
				mcut->n_cells++;
				shrink(mcut, cmax);
				shrink(mcut, cnew);
				break;
				}
			cmax->dividable &= ~split;
			}
		}

	/* I would like to return the palette sorted by frequency of use */
	/* This isn't technically a requirement of this algorithm        */
	/* If I do though, it allows me to do other things afterwards    */

	for ( i = 0; i < mcut->n_cells; i++ )
		reorder[i] = (byte) i;

	for ( j = mcut->n_cells; j > 0; j-- )
		{
		BOOLEAN noswaps = TRUE;
		for ( i = 0; i < j - 1; i++ )
			if ( c[reorder[i]].freq < c[reorder[i+1]].freq )
				{
				byte t = reorder[i];
				reorder[i] = reorder[i+1];
				reorder[i+1] = t;
				noswaps = FALSE;
				}
		if ( noswaps )
			break;
		}


	/* Now set up the palette array passed in */
	/* Note: ( ((x+y)/2) << 3 ) == ( (x+y) << 2 ) */
	/* Also, label each point in the cell as being a member of that cell */

	for ( i = 0; i < mcut->n_cells; i++ )
		{
		int inx = reorder[i];
		byte r, g, b;

		gbmrgb[i].r = ( (c[inx].r0 + c[inx].r1) << 2 );
		gbmrgb[i].g = ( (c[inx].g0 + c[inx].g1) << 2 );
		gbmrgb[i].b = ( (c[inx].b0 + c[inx].b1) << 2 );

		for ( r = c[inx].r0; r < c[inx].r1; r++ )
			for ( g = c[inx].g0; g < c[inx].g1; g++ )
				for ( b = c[inx].b0; b < c[inx].b1; b++ )
					mcut->freqs[b][g][r] = i;
		}

	/* Unused palette entries will be medium grey */
	for ( ; i < 0x100; i++ )
		{
		gbmrgb[i].r = 0x80;
		gbmrgb[i].g = 0x80;
		gbmrgb[i].b = 0x80;
		}
	}
/*...e*/
/*...sgbm_map_mcut    \45\ map to median cutted palette:0:*/
void gbm_map_mcut(
	GBMMCUT *mcut,
	const GBM *gbm, const byte *data24, byte *data8
	)
	{
	int stride24 = ((gbm->w * 3 + 3) & ~3);
	int step24   = stride24 - gbm->w * 3;
	int stride8  = ((gbm->w + 3) & ~3);
	int step8    = stride8 - gbm->w;
	int x, y;

	/* Now transform the image data */

	for ( y = 0; y < gbm->h; y++, data24 += step24, data8 += step8 )
		for ( x = 0; x < gbm->w; x++ )
			{
			byte b = (*data24++ >> 3);
			byte g = (*data24++ >> 3);
			byte r = (*data24++ >> 3);

			*data8++ = (byte) ( mcut->freqs[b][g][r] );
			}
	}
/*...e*/
/*...sgbm_mcut        \45\ map single bitmap using median cut:0:*/
BOOLEAN gbm_mcut(
	const GBM *gbm, const byte *data24,
	GBMRGB gbmrgb[],
	byte *data8,
	int n_cols_wanted
	)
	{
	GBMMCUT *mcut;

	if ( (mcut = gbm_create_mcut()) == NULL )
		return FALSE;
	gbm_add_to_mcut(mcut, gbm, data24);
	gbm_pal_mcut(mcut, gbmrgb, n_cols_wanted);
	gbm_map_mcut(mcut, gbm, data24, data8);
	gbm_delete_mcut(mcut);
	return TRUE;
	}
/*...e*/
