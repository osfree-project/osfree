/*

gbmiax.c - IBM Image Access eXecutive support

Reads array as 8 bit greyscale.
Writes grey equivelent of passed in 8 bit colour data (no palette written).
Input options: width=# (default: 512)
Output options: r,g,b,k (default: k)

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
/*...e*/

/*...smake_output_palette:0:*/
#define	SW4(a,b,c,d)	((a)*8+(b)*4+(c)*2+(d))

static gbm_boolean make_output_palette(const GBMRGB gbmrgb[], gbm_u8 grey[], const char *opt)
	{
	gbm_boolean	k = ( gbm_find_word(opt, "k") != NULL );
	gbm_boolean	r = ( gbm_find_word(opt, "r") != NULL );
	gbm_boolean	g = ( gbm_find_word(opt, "g") != NULL );
	gbm_boolean	b = ( gbm_find_word(opt, "b") != NULL );
	int	i;

	switch ( SW4(k,r,g,b) )
		{
		case SW4(0,0,0,0):
			/* Default is the same as "k" */
		case SW4(1,0,0,0):
			for ( i = 0; i < 0x100; i++ )
				grey[i] = (gbm_u8) ( ((gbm_u16) gbmrgb[i].r *  77U +
						    (gbm_u16) gbmrgb[i].g * 150U +
						    (gbm_u16) gbmrgb[i].b *  29U) >> 8 );
			return GBM_TRUE;
		case SW4(0,1,0,0):
			for ( i = 0; i < 0x100; i++ )
				grey[i] = gbmrgb[i].r;
			return GBM_TRUE;
		case SW4(0,0,1,0):
			for ( i = 0; i < 0x100; i++ )
				grey[i] = gbmrgb[i].g;
			return GBM_TRUE;
		case SW4(0,0,0,1):
			for ( i = 0; i < 0x100; i++ )
				grey[i] = gbmrgb[i].b;
			return GBM_TRUE;
		}
	return GBM_FALSE;
	}
/*...e*/


static GBMFT iax_gbmft =
	{
        GBM_FMT_DESC_SHORT_IAX,
        GBM_FMT_DESC_LONG_IAX,
        GBM_FMT_DESC_EXT_IAX,
	GBM_FT_R8|
	GBM_FT_W8,
	};

#define	GBM_ERR_IAX_SIZE	((GBM_ERR) 1500)

/*...siax_qft:0:*/
GBM_ERR iax_qft(GBMFT *gbmft)
	{
	*gbmft = iax_gbmft;
	return GBM_ERR_OK;
	}
/*...e*/
/*...siax_rhdr:0:*/
GBM_ERR iax_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
	{
	long length;
	int w, h;
	const char *width;

	fn=fn; fd=fd; /* Suppress 'unref arg' compiler warnings */

	length = gbm_file_lseek(fd, 0L, GBM_SEEK_END);
	gbm_file_lseek(fd, 0L, GBM_SEEK_SET);

	if ( (width = gbm_find_word_prefix(opt, "width=")) != NULL )
		sscanf(width + 6, "%d", &w);
	else
		w = 512;

	h = (int) (length / w);

	if ( w <= 0 || h <= 0 )
		return GBM_ERR_BAD_SIZE;

	if ( w * h != length )
		return GBM_ERR_IAX_SIZE;

	gbm->w   = w;
	gbm->h   = h;
	gbm->bpp = 8;

	return GBM_ERR_OK;
	}
/*...e*/
/*...siax_rpal:0:*/
GBM_ERR iax_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
	{
	int	i;

	fd=fd; gbm=gbm; /* Suppress 'unref arg' compiler warnings */

	for ( i = 0; i < 0x100; i++ )
		gbmrgb[i].r =
		gbmrgb[i].g =
		gbmrgb[i].b = (gbm_u8) i;

	return GBM_ERR_OK;
	}
/*...e*/
/*...siax_rdata:0:*/
GBM_ERR iax_rdata(int fd, GBM *gbm, gbm_u8 *data)
	{
	int	i, stride;
	gbm_u8	*p;

	stride = ((gbm->w + 3) & ~3);
	p = data + ((gbm->h - 1) * stride);
	for ( i = gbm->h - 1; i >= 0; i-- )
		{
		gbm_file_read(fd, p, gbm->w);
		p -= stride;
		}
	return GBM_ERR_OK;
	}
/*...e*/
/*...siax_w:0:*/
GBM_ERR iax_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
	{
	int i, j, stride;
	gbm_u8 grey[0x100];
	const gbm_u8 *p;
	gbm_u8 *linebuf;

	fn=fn; /* Suppress 'unref arg' compiler warning */

	if ( gbm->bpp != 8 )
		return GBM_ERR_NOT_SUPP;

	if ( !make_output_palette(gbmrgb, grey, opt) )
		return GBM_ERR_BAD_OPTION;

	if ( (linebuf = gbmmem_malloc((size_t) gbm->w)) == NULL )
		return GBM_ERR_MEM;

	stride = ((gbm->w + 3) & ~3);
	p = data + ((gbm->h - 1) * stride);
	for ( i = gbm->h - 1; i >= 0; i-- )
		{
		for ( j = 0; j < gbm->w; j++ )
			linebuf[j] = grey[p[j]];
		gbm_file_write(fd, linebuf, gbm->w);
		p -= stride;
		}

	gbmmem_free(linebuf);

	return GBM_ERR_OK;
	}
/*...e*/
/*...siax_err:0:*/
const char *iax_err(GBM_ERR rc)
	{
	switch ( (int) rc )
		{
		case GBM_ERR_IAX_SIZE:
			return "file length is not a multiple of its width";
		}
	return NULL;
	}
/*...e*/
