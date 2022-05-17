/*

gbmcpal.c - Map to Common Palette

History:
--------
(Heiko Nitzsche)

26-Apr-2006: Fix issue with comma separation between file and options.
             Now the file can have quotes and thus clearly separating
             it from the options.
             On OS/2 command line use: e.g. "\"ifspec\",options"
08-Feb-2008  Allocate memory from high memory for bitmap data to
             stretch limit for out-of-memory errors
             (requires kernel with high memory support)
15-Aug-2008  Integrate new GBM types

10-Oct-2008: Changed recommended file specification template to
             "file.ext"\",options   or   "file.ext"\",\""options"
*/

/*...sincludes:0:*/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#if defined(AIX) || defined(LINUX) || defined(SUN) || defined(MACOSX) || defined(IPHONE)
#include <unistd.h>
#else
#include <io.h>
#endif
#include <fcntl.h>
#ifdef MAC
#include <types.h>
#include <stat.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include "gbm.h"
#include "gbmmem.h"
#include "gbmhist.h"
#include "gbmmcut.h"
#include "gbmtool.h"


static char progname[] = "gbmcpal";

/*...sfatal:0:*/
static void fatal(const char *fmt, ...)
	{
	va_list	vars;
	char s[256+1];

	va_start(vars, fmt);
	vsprintf(s, fmt, vars);
	va_end(vars);
	fprintf(stderr, "%s: %s\n", progname, s);
	exit(1);
	}
/*...e*/
/*...susage:0:*/
static void usage(void)
	{
	int ft, n_ft;

	fprintf(stderr, "usage: %s [-m map] [-v] n1 n2 n3 \"ifspec\"{\\\",\\\"\"opt\"} \"ofspec\"{\\\",\\\"\"opt\"}\n", progname);
	fprintf(stderr, "flags: -m map       mapping to perform (default freq6:6:6:256)\n");
	fprintf(stderr, "                    freqR:G:B:N       map all bitmaps to same palette, worked\n");
	fprintf(stderr, "                                      out using frequency of use histogram\n");
	fprintf(stderr, "                    mcutN             map all bitmaps to same palette, worked\n");
	fprintf(stderr, "                                      out using median cut algorithm\n");
	fprintf(stderr, "                    rofreqR:G:B:N:N2  map each bitmap to frequency palette,\n");
	fprintf(stderr, "                                      reordered to minimise differences\n");
	fprintf(stderr, "                                      between successive bitmaps\n");
	fprintf(stderr, "                    romcutN:N2        map each bitmap to median cut palette,\n");
	fprintf(stderr, "                                      reordered to minimise differences\n");
	fprintf(stderr, "                                      between successive bitmaps\n");
	fprintf(stderr, "                                      R,G,B are bits of red, green and blue\n");
	fprintf(stderr, "                                      to keep, N is number of unique colours,\n");
	fprintf(stderr, "                                      N2 is extra palette entries\n");
	fprintf(stderr, "       -v           verbose mode\n");
	fprintf(stderr, "       n1 n2 n3     for ( f=n1; f<n2; f+=n3 )\n");
	fprintf(stderr, "       ifspec         printf(ifspec, f);\n");
	fprintf(stderr, "       ofspec         printf(ofspec, f);\n");
	fprintf(stderr, "                    filespecs are of the form fn.ext\n");
	fprintf(stderr, "                    ext's are used to deduce desired bitmap file formats\n");

	gbm_init();
	gbm_query_n_filetypes(&n_ft);
	for ( ft = 0; ft < n_ft; ft++ )
		{
		GBMFT gbmft;

		gbm_query_filetype(ft, &gbmft);
		fprintf(stderr, "                    %s when ext in [%s]\n",
			gbmft.short_name, gbmft.extensions);
		}
	gbm_deinit();

	fprintf(stderr, "       opt's        bitmap format specific options\n");
	fprintf(stderr, "\n   eg: %s -m mcut256 0 100 1 24bit%%03d.bmp 8bit%%03d.bmp\n", progname);

	fprintf(stderr, "\n       In case the spec contains a comma or spaces and options\n");
    fprintf(stderr,   "       need to be added, syntax \"fspec\"{\\\",\\\"opt} or \"fspec\"{\\\",\\\"\"opt\"}\n");
	fprintf(stderr,   "       must be used to clearly separate it from the options.\n");

	exit(1);
	}
/*...e*/
static gbm_boolean same(const char *s1, const char *s2, size_t n)
{
    const size_t s1len = strlen(s1);
    const size_t s2len = strlen(s2);
    size_t i;

    for (i = 0; (i < n) && (i < s1len) && (i < s2len); i++, s1++, s2++ )
    {
        if ( tolower(*s1) != tolower(*s2) )
        {
            return GBM_FALSE;
        }
    }
    if (i < n)
    {
        return GBM_FALSE;
    }
    return GBM_TRUE;
}
/*...smain:0:*/
#define	CVT_FREQ   0
#define	CVT_MCUT   1
#define	CVT_ROFREQ 2
#define	CVT_ROMCUT 3

static gbm_boolean verbose = GBM_FALSE;

/*...sget_masks:0:*/
/*
Returns GBM_TRUE if a set of masks given at map.
Also sets *rm, *gm, *bm from these.
Else returns GBM_FALSE.
*/

static gbm_u8 mask[] = { 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };

static gbm_boolean get_masks(char *map, gbm_u8 *rm, gbm_u8 *gm, gbm_u8 *bm)
	{
	if ( map[0] <  '0' || map[0] > '8' ||
	     map[1] != ':' ||
	     map[2] <  '0' || map[2] > '8' ||
	     map[3] != ':' ||
	     map[4] <  '0' || map[4] > '8' )
		return GBM_FALSE;

	*rm = mask[map[0] - '0'];
	*gm = mask[map[2] - '0'];
	*bm = mask[map[4] - '0'];
	return GBM_TRUE;
	}
/*...e*/
/*...salloc_mem:0:*/
static gbm_u8 *alloc_mem(const GBM *gbm)
	{
	size_t stride, bytes;
	gbm_u8 *p;

	stride = ( ((gbm->w * gbm->bpp + 31)/32) * 4 );
	bytes = stride * gbm->h;
	if ( (p = gbmmem_malloc(bytes)) == NULL )
	{
		#if (ULONG_MAX > UINT_MAX)
		fatal("out of memory allocating %zu bytes", bytes);
		#else
		fatal("out of memory allocating %u bytes", bytes);
		#endif
	}
	return p;
	}
/*...e*/
/*...sread_bitmap:0:*/
static void read_bitmap(
	const char *fn, const char *opt,
	GBM *gbm, GBMRGB gbmrgb[], gbm_u8 **data
	)
	{
	int ft, fd;
	GBM_ERR rc;

	if ( verbose )
		{
		if ( *opt != '\0' )
			printf("Reading %s,%s\n", fn, opt);
		else
			printf("Reading %s\n", fn);
		}

	if ( gbm_guess_filetype(fn, &ft) != GBM_ERR_OK )
		fatal("can't guess bitmap file format for %s", fn);

	if ( (fd = gbm_io_open(fn, GBM_O_RDONLY)) == -1 )
		fatal("can't open %s", fn);

	if ( (rc = gbm_read_header(fn, fd, ft, gbm, opt)) != GBM_ERR_OK )
		{
		gbm_io_close(fd);
		fatal("can't read header of %s: %s", fn, gbm_err(rc));
		}

        /* check for color depth supported by algorithms */
	switch ( gbm->bpp )
		{
		case 24:
		case 8:
		case 4:
		case 1:
                   break;
                default:
 		  {
		     gbm_io_close(fd);
		     fatal("%d bpp file is not supported", gbm->bpp);
		  }
		}

	if ( (rc = gbm_read_palette(fd, ft, gbm, gbmrgb)) != GBM_ERR_OK )
		{
		gbm_io_close(fd);
		fatal("can't read palette of %s: %s", fn, gbm_err(rc));
		}

	(*data) = alloc_mem(gbm);

	if ( (rc = gbm_read_data(fd, ft, gbm, (*data))) != GBM_ERR_OK )
		{
		gbmmem_free(*data);
		gbm_io_close(fd);
		fatal("can't read bitmap data of %s: %s", fn, gbm_err(rc));
		}

	gbm_io_close(fd);
	}
/*...e*/
/*...sread_bitmap_24:0:*/
/*...sexpand_to_24bit:0:*/
static void expand_to_24bit(GBM *gbm, GBMRGB *gbmrgb, gbm_u8 **data)
	{
	size_t stride     = ((gbm->w * gbm->bpp + 31)/32) * 4;
	size_t new_stride = ((gbm->w * 3 + 3) & ~3);
	size_t bytes;
    int y;
	gbm_u8 *new_data;

	if ( gbm->bpp == 24 )
		return;

	bytes = new_stride * gbm->h;
	if ( (new_data = gbmmem_malloc(bytes)) == NULL )
	{
		#if (ULONG_MAX > UINT_MAX)
		fatal("out of memory allocating %zu bytes", bytes);
		#else
		fatal("out of memory allocating %u bytes", bytes);
		#endif
	}
	for ( y = 0; y < gbm->h; y++ )
		{
		gbm_u8	*src = *data + stride * y;
		gbm_u8	*dest = new_data + new_stride * y;
		int	x;

		switch ( gbm->bpp )
			{
/*...s1:24:*/
case 1:
	{
	gbm_u8 c = 0;

	for ( x = 0; x < gbm->w; x++ )
		{
		if ( (x & 7) == 0 )
			c = *src++;
		else
			c <<= 1;

		*dest++ = gbmrgb[c >> 7].b;
		*dest++ = gbmrgb[c >> 7].g;
		*dest++ = gbmrgb[c >> 7].r;
		}
	}
	break;
/*...e*/
/*...s4:24:*/
case 4:
	for ( x = 0; x + 1 < gbm->w; x += 2 )
		{
		gbm_u8	c = *src++;

		*dest++ = gbmrgb[c >> 4].b;
		*dest++ = gbmrgb[c >> 4].g;
		*dest++ = gbmrgb[c >> 4].r;
		*dest++ = gbmrgb[c & 15].b;
		*dest++ = gbmrgb[c & 15].g;
		*dest++ = gbmrgb[c & 15].r;
		}

	if ( x < gbm->w )
		{
		gbm_u8	c = *src;

		*dest++ = gbmrgb[c >> 4].b;
		*dest++ = gbmrgb[c >> 4].g;
		*dest++ = gbmrgb[c >> 4].r;
		}
	break;
/*...e*/
/*...s8:24:*/
case 8:
	for ( x = 0; x < gbm->w; x++ )
		{
		gbm_u8	c = *src++;

		*dest++ = gbmrgb[c].b;
		*dest++ = gbmrgb[c].g;
		*dest++ = gbmrgb[c].r;
		}
	break;
/*...e*/
			}
		}
	gbmmem_free(*data);
	*data = new_data;
	gbm->bpp = 24;
	}
/*...e*/

static void read_bitmap_24(
	const char *fn, const char *opt,
	GBM *gbm, gbm_u8 **data
	)
	{
	GBMRGB gbmrgb[0x100];
	read_bitmap(fn, opt, gbm, gbmrgb, data);
	if ( gbm->bpp != 24 )
		{
		if ( verbose )
			printf("Expanding to 24 bpp\n");
		expand_to_24bit(gbm, gbmrgb, data);
		}
	}
/*...e*/
/*...sread_bitmap_24_f:0:*/
static void read_bitmap_24_f(
	const char *fn, int f, const char *opt,
	GBM *gbm, gbm_u8 **data
	)
	{
	char fn_f[GBMTOOL_FILENAME_MAX+1];
	sprintf(fn_f, fn, f);
	read_bitmap_24(fn_f, opt, gbm, data);
	}
/*...e*/
/*...swrite_bitmap:0:*/
static void write_bitmap(
	const char *fn, const char *opt,
	const GBM *gbm, const GBMRGB gbmrgb[], const gbm_u8 *data
	)
	{
	int ft, fd, flag;
	GBM_ERR rc;
	GBMFT gbmft;

	if ( verbose )
		{
		if ( *opt != '\0' )
			printf("Writing %s,%s\n", fn, opt);
		else
			printf("Writing %s\n", fn);
		}

	if ( gbm_guess_filetype(fn, &ft) != GBM_ERR_OK )
		fatal("can't guess bitmap file format for %s", fn);

	gbm_query_filetype(ft, &gbmft);
	switch ( gbm->bpp )
		{
		case 24:	flag = GBM_FT_W24;	break;
		case 8:		flag = GBM_FT_W8;	break;
		case 4:		flag = GBM_FT_W4;	break;
		case 1:		flag = GBM_FT_W1;	break;
		default:	flag = 0;	        break;
		}

	if ( (gbmft.flags & flag) == 0 )
		fatal("output bitmap format %s does not support writing %d bpp data",
			gbmft.short_name, gbm->bpp);

	if ( (fd = gbm_io_create(fn, GBM_O_WRONLY)) == -1 )
		fatal("can't create %s", fn);

	if ( (rc = gbm_write(fn, fd, ft, gbm, gbmrgb, data, opt)) != GBM_ERR_OK )
		{
		gbm_io_close(fd);
		remove(fn);
		fatal("can't write %s: %s", fn, gbm_err(rc));
		}

	gbm_io_close(fd);
	}
/*...e*/
/*...swrite_bitmap_f:0:*/
static void write_bitmap_f(
	const char *fn, int f, const char *opt,
	const GBM *gbm, const GBMRGB gbmrgb[], const gbm_u8 *data
	)
	{
	char fn_f[GBMTOOL_FILENAME_MAX+1];
	sprintf(fn_f, fn, f);
	write_bitmap(fn_f, opt, gbm, gbmrgb, data);
	}
/*...e*/
/*...sfreq_map:0:*/
static void freq_map(
	int first, int last, int step,
	const char *fn_src, const char *opt_src,
	const char *fn_dst, const char *opt_dst,
	int ncols, gbm_u8 rm, gbm_u8 gm, gbm_u8 bm
	)
	{
	int f;
	GBMHIST *hist;
	GBMRGB gbmrgb[0x100];

	for ( ;; )
		{
		if ( verbose )
			printf("Attempting to build histogram data with masks 0x%02x 0x%02x 0x%02x\n",
				rm, gm, bm);
		if ( (hist = gbm_create_hist(rm, gm, bm)) == NULL )
			fatal("can't create histogram data");
		for ( f = first; f < last; f += step )
			{
			GBM gbm; gbm_u8 *data;
			gbm_boolean ok;
			read_bitmap_24_f(fn_src, f, opt_src, &gbm, &data);
			ok = gbm_add_to_hist(hist, &gbm, data);
			gbmmem_free(data);
			if ( !ok )
				{
				if ( verbose )
					printf("Too many colours\n");
				break;
				}
			}
		if ( f == last )
			break;

		gbm_delete_hist(hist);

		if ( gm > rm )
			gm <<= 1;
		else if ( rm > bm )
			rm <<= 1;
		else
			bm <<= 1;
		}

	if ( verbose )
		printf("Working out %d colour palette, based on histogram data\n", ncols);

	gbm_pal_hist(hist, gbmrgb, ncols);

	if ( verbose )
		printf("Converting files to new optimal palette\n");

	for ( f = first; f < last; f += step )
		{
		GBM gbm; gbm_u8 *data, *data8;
		read_bitmap_24_f(fn_src, f, opt_src, &gbm, &data);
		gbm.bpp = 8;
		data8 = alloc_mem(&gbm);
		if ( verbose )
			printf("Mapping to optimal palette\n");
		gbm_map_hist(hist, &gbm, data, data8);
		gbmmem_free(data);
		write_bitmap_f(fn_dst, f, opt_dst, &gbm, gbmrgb, data8);
		gbmmem_free(data8);
		}			

	gbm_delete_hist(hist);
	}
/*...e*/
/*...smcut_map:0:*/
static void mcut_map(
	int first, int last, int step,
	const char *fn_src, const char *opt_src,
	const char *fn_dst, const char *opt_dst,
	int ncols
	)
	{
	int f;
	GBMMCUT *mcut;
	GBMRGB gbmrgb[0x100];

	if ( verbose )
		printf("Attempting to build median cut statistics\n");

	if ( (mcut = gbm_create_mcut()) == NULL )
		fatal("can't create median cut data");
	for ( f = first; f < last; f += step )
		{
		GBM gbm; gbm_u8 *data;
		read_bitmap_24_f(fn_src, f, opt_src, &gbm, &data);
		gbm_add_to_mcut(mcut, &gbm, data);
		gbmmem_free(data);
		}

	if ( verbose )
		printf("Working out %d colour palette, based on median cut statistics\n", ncols);

	gbm_pal_mcut(mcut, gbmrgb, ncols);

	if ( verbose )
		printf("Converting files to new optimal palette\n");

	for ( f = first; f < last; f += step )
		{
		GBM gbm; gbm_u8 *data, *data8;
		read_bitmap_24_f(fn_src, f, opt_src, &gbm, &data);
		gbm.bpp = 8;
		data8 = alloc_mem(&gbm);
		if ( verbose )
			printf("Mapping to optimal palette\n");
		gbm_map_mcut(mcut, &gbm, data, data8);
		gbmmem_free(data);
		write_bitmap_f(fn_dst, f, opt_dst, &gbm, gbmrgb, data8);
		gbmmem_free(data8);
		}			

	gbm_delete_mcut(mcut);
	}
/*...e*/
/*...srofreq_map\44\ romcut_map:0:*/
/*

This code has been written primarily to support crude animation schemes.
Imagine an animation which is a series of (palette, bitmap-bits) pairs.

If the displayer is unable to change both the palette and bitmap-bits in
the vertical retrace interval, or if ping-pong double buffering is not
available, there will be a short time where the new palette has been set, but
the old bits are still on display. This causes a very disturbing flicker.
(Similarly, this is true if the displaying program sets the bitmap-bits, and
then the palette).

This code reorders the new palette, so that its entries are close to the
previous palette. Old palette entries used by the most pixels are considered
for this matching process first. Hence only small areas of the image flicker.

eg: old palette           = { red   , green      , light green, black       }
    new palette           = { green , orange     , dark blue  , light green }
    reordered new palette = { orange, light green, green      , dark blue   }

    Clearly red->orange is less offensive than red->green etc..

*/

/*...sro_map:0:*/
/*

The palettes returned using gbm_hist/mcut are sorted with most used first.
(Index through MAP to get palette entries in PAL in order of frequency).

Map first image to palette PAL and bits BITS.
For i = 0 to ncols-1
  MAP[i] = i
Write out image PAL and BITS
For each subsequent image
  Map image to PAL' and BITS'
  For i = 0 to ncols-1
    MAP'[i] = -1
  For i = 0 to ncols-1
    j = index of closest entry to PAL[MAP[i]] in PAL',
      with MAP'[j] = -1
    MAP'[j] = MAP[i];
  For i = 0 to ncols-1
    PAL[MAP'[i]] = PAL'[i]
  For each pixel p
    BITS'[p] = MAP'[BITS'[p]]
  Write out PAL and BITS'
  BITS = BITS'
  MAP = MAP'

*/

/*...scalc_mapP:0:*/
/*
For each entry in the old palette, starting with the most used, find the
closest 'unclaimed' entry in the new palette, and 'claim' it.
Thus, if you iterate though mapP[0..ncols-1] you get palette indexes
of close entries in the old palette.
*/

static void calc_mapP(
	const GBMRGB gbmrgb [], const gbm_u16 map [],
	      GBMRGB gbmrgbP[],       gbm_u16 mapP[],
	int dists[],
	int ncols
	)
	{
	int i;

	if ( verbose )
		printf("Reordering palette to cause least flicker\n");

	for ( i = 0; i < ncols; i++ )
		mapP[i] = (gbm_u16) 0xffff;

	/* Go through old palette entries, in descending freq order */
	for ( i = 0; i < ncols; i++ )
		{
		const GBMRGB *p = &(gbmrgb[map[i]]);
		int mindist = 255*255*3 + 1;
		int j, minj = 0;
		/* Find closest entry in new palette */
		for ( j = 0; j < ncols; j++ )
			{
			int dr = (int) ( (unsigned int) p->r - (unsigned int) gbmrgbP[j].r );
			int dg = (int) ( (unsigned int) p->g - (unsigned int) gbmrgbP[j].g );
			int db = (int) ( (unsigned int) p->b - (unsigned int) gbmrgbP[j].b );
			int dist = dr*dr + dg*dg + db*db;
			if ( dist < mindist && mapP[j] == (gbm_u16) 0xffff )
				{
				minj = j;
				mindist = dist;
				}
			}
		dists[minj] = mindist;
		mapP[minj] = map[i];
		}
	}
/*...e*/

static void ro_map(
	int first, int last, int step,
	const char *fn_src, const char *opt_src,
	const char *fn_dst, const char *opt_dst,
	int ncols, int ncolsextra, gbm_u8 rm, gbm_u8 gm, gbm_u8 bm,
	void (*get)(
		const char *fn, int f, const char *opt,
		int ncols, gbm_u8 rm, gbm_u8 gm, gbm_u8 bm,
		GBM *gbm, GBMRGB gbmrgb[], gbm_u8 **data8
		)
	)
	{
	GBM gbm; GBMRGB gbmrgb[0x100]; gbm_u8 *data8;
	gbm_u16 map[0x100], *extra = &(map[ncols]);
	int i, f;

	if ( first >= last )
		return;

	(*get)(fn_src, first, opt_src, ncols, rm, gm, bm, &gbm, gbmrgb, &data8);
	for ( i = 0; i < ncols+ncolsextra; i++ )
		map[i] = (gbm_u16) i;

	write_bitmap_f(fn_dst, first, opt_dst, &gbm, gbmrgb, data8);

	for ( f = first + step; f < last; f += step )
		{
		GBM gbmP; GBMRGB gbmrgbP[0x100]; gbm_u8 *data8P, *p;
		gbm_u16 mapP[0x100]; int dists[0x100];
		int x, y, stride;

		(*get)(fn_src, f, opt_src, ncols, rm, gm, bm, &gbmP, gbmrgbP, &data8P);
		calc_mapP(gbmrgb, map, gbmrgbP, mapP, dists, ncols);

/*...shandle ncolsextra worst matches specially:16:*/
{
int j;

/* Find the ncolsextra worst palette changes */

for ( i = 0; i < ncolsextra; i++ )
	{
	int jmax = 0, maxdist = -1;
	for ( j = 0; j < ncols; j++ )
		if ( dists[j] != -1 && dists[j] > maxdist )
			{
			jmax = j;
			maxdist = dists[j];
			}
	dists[jmax] = -1;
	}

/* Use extra palette entries for these instead */

for ( i = 0, j = 0; i < ncolsextra; i++, j++ )
	{
	gbm_u16 t;
	while ( dists[j] != -1 )
		j++;
	t = mapP[j];		/* This is a bad palette entry */
	mapP[j] = extra[i];	/* Use extra entry instead */
	extra[i] = t;		/* So bad one is fair game next loop */
	}
}
/*...e*/

		for ( i = 0; i < ncols; i++ )
			gbmrgb[mapP[i]] = gbmrgbP[i];

		stride = ((gbmP.w+3)&~3);
		for ( y = 0, p = data8P; y < gbmP.h; y++, p += stride )
			for ( x = 0; x < gbmP.w; x++ )
				p[x] = (gbm_u8) mapP[p[x]];

		write_bitmap_f(fn_dst, f, opt_dst, &gbmP, gbmrgb, data8P);

		gbm = gbmP;
		gbmmem_free(data8);
		data8 = data8P;
		memcpy(map, mapP, ncols * sizeof(gbm_u16));
		}

	gbmmem_free(data8);
	}
/*...e*/
/*...srofreq_map:0:*/
/*...sget_and_hist:0:*/
static void get_and_hist(
	const char *fn, int f, const char *opt,
	int ncols, gbm_u8 rm, gbm_u8 gm, gbm_u8 bm,
	GBM *gbm, GBMRGB gbmrgb[], gbm_u8 **data8
	)
	{
	gbm_u8 *data24;
	read_bitmap_24_f(fn, f, opt, gbm, &data24);
	gbm->bpp = 8;
	(*data8) = alloc_mem(gbm);
	if ( !gbm_hist(gbm, data24, gbmrgb, *data8, ncols, rm, gm, bm) )
		fatal("can't compute histogram");
	gbmmem_free(data24);
	}
/*...e*/

static void rofreq_map(
	int first, int last, int step,
	const char *fn_src, const char *opt_src,
	const char *fn_dst, const char *opt_dst,
	int ncols, int ncolsextra, gbm_u8 rm, gbm_u8 gm, gbm_u8 bm
	)
	{
	ro_map(
		first, last, step,
		fn_src, opt_src, fn_dst, opt_dst,
		ncols, ncolsextra, rm, gm, bm,
		get_and_hist
		);
	}
/*...e*/
/*...sromcut_map:0:*/
/*...sget_and_mcut:0:*/
static void get_and_mcut(
	const char *fn, int f, const char *opt,
	int ncols, gbm_u8 rm, gbm_u8 gm, gbm_u8 bm,
	GBM *gbm, GBMRGB gbmrgb[], gbm_u8 **data8
	)
	{
	gbm_u8 *data24;
	rm=rm; gm=gm; bm=bm; /* Suppress 'unused arg warning' */
	read_bitmap_24_f(fn, f, opt, gbm, &data24);
	gbm->bpp = 8;
	(*data8) = alloc_mem(gbm);
	if ( !gbm_mcut(gbm, data24, gbmrgb, *data8, ncols) )
		fatal("can't perform median-cut");
	gbmmem_free(data24);
	}
/*...e*/

static void romcut_map(
	int first, int last, int step,
	const char *fn_src, const char *opt_src,
	const char *fn_dst, const char *opt_dst,
	int ncols, int ncolsextra
	)
	{
	ro_map(
		first, last, step,
		fn_src, opt_src, fn_dst, opt_dst,
		ncols, ncolsextra, 0, 0, 0,
		get_and_mcut
		);
	}
/*...e*/
/*...e*/

int main(int argc, char *argv[])
	{
	GBMTOOL_FILEARG gbmfilearg;
	char    fn_src[GBMTOOL_FILENAME_MAX+1], fn_dst[GBMTOOL_FILENAME_MAX+1],
	        opt_src[GBMTOOL_OPTIONS_MAX+1], opt_dst[GBMTOOL_OPTIONS_MAX+1];

	char *map = "freq6:6:6:256";
	int i, m = 0, ncols, ncolsextra, first, last, step;
	gbm_u8 rm = 0, gm = 0, bm = 0;

/*...sprocess command line options:8:*/
for ( i = 1; i < argc; i++ )
	{
	if ( argv[i][0] != '-' )
		break;
	switch ( argv[i][1] )
		{
		case 'm':	if ( ++i == argc )
					fatal("expected map argument");
				map = argv[i];
				break;
		case 'v':	verbose = GBM_TRUE;
				break;
		default:	usage();
				break;
		}
	}
/*...e*/
/*...sframes and filenames etc\46\:8:*/
if ( i == argc )
	usage();
sscanf(argv[i++], "%d", &first);

if ( i == argc )
	usage();
sscanf(argv[i++], "%d", &last);

if ( i == argc )
	usage();
sscanf(argv[i++], "%d", &step);

if ( i == argc )
	usage();

/* Split filename and file options. */
gbmfilearg.argin = argv[i++];
if (strcmp(gbmfilearg.argin, "\"\"") == 0)
{
  usage();
}
if (gbmtool_parse_argument(&gbmfilearg, GBM_FALSE) != GBM_ERR_OK)
{
  fatal("can't parse source filename %s", gbmfilearg.argin);
}
strcpy(fn_src , gbmfilearg.files->filename);
strcpy(opt_src, gbmfilearg.options);
gbmtool_free_argument(&gbmfilearg);

gbmfilearg.argin = (i == argc) ? argv[i-1] : argv[i++];
if (strcmp(gbmfilearg.argin, "\"\"") == 0)
{
  usage();
}
if (gbmtool_parse_argument(&gbmfilearg, GBM_FALSE) != GBM_ERR_OK)
{
  fatal("can't parse destination filename %s", gbmfilearg.argin);
}
strcpy(fn_dst , gbmfilearg.files->filename);
strcpy(opt_dst, gbmfilearg.options);
gbmtool_free_argument(&gbmfilearg);

if (i < argc)
	usage();

/*...sdeduce mapping and bits per pixel etc\46\:8:*/
if ( same(map, "freq", 4) )
	{
	m = CVT_FREQ;
	if ( !get_masks(map + 4, &rm, &gm, &bm) )
		fatal("freqR:G:B:N has bad/missing R:G:B");
	if ( map[9] != ':' )
		fatal("freqR:G:B:N has bad/missing :N");
	sscanf(map + 10, "%i", &ncols);
	if ( ncols < 1 || ncols > 256 )
		fatal("freqR:G:B:N N number between 1 and 256 required");
	}
else if ( same(map, "mcut", 4) )
	{
	m = CVT_MCUT;
	sscanf(map+4, "%i", &ncols);
	if ( ncols < 1 || ncols > 256 )
		fatal("mcutN N number between 1 and 256 required");
	}
else if ( same(map, "rofreq", 6) )
	{
	m = CVT_ROFREQ;
	if ( !get_masks(map+6, &rm, &gm, &bm) )
		fatal("rofreqR:G:B:N has bad/missing R:G:B");
	if ( map[11] != ':' )
		fatal("rofreqR:G:B:N has bad/missing :N:N2");
	sscanf(map + 12, "%i:%i", &ncols, &ncolsextra);
	if ( ncols+ncolsextra < 1 || ncols+ncolsextra > 256 )
		fatal("rofreqR:G:B:N:N2 N+N2 must be between 1 and 256");
	}
else if ( same(map, "romcut", 6) )
	{
	m = CVT_ROMCUT;
	sscanf(map+6, "%i:%i", &ncols, &ncolsextra);
	if ( ncols+ncolsextra < 1 || ncols+ncolsextra > 256 )
		fatal("mcutN:N2 N+N2 must be between 1 and 256");
	}
else
	fatal("unrecognised mapping %s", map);
/*...e*/

	gbm_init();

	switch ( m )
		{
		case CVT_FREQ:
			freq_map(
				first, last, step,
				fn_src, opt_src, fn_dst, opt_dst,
				ncols, rm, gm, bm);
			break;
		case CVT_MCUT:
			mcut_map(
				first, last, step,
				fn_src, opt_src, fn_dst, opt_dst,
				ncols);
			break;
		case CVT_ROFREQ:
			rofreq_map(
				first, last, step,
				fn_src, opt_src, fn_dst, opt_dst,
				ncols, ncolsextra, rm, gm, bm);
			break;
		case CVT_ROMCUT:
			romcut_map(
				first, last, step,
				fn_src, opt_src, fn_dst, opt_dst,
				ncols, ncolsextra);
			break;
		}

	gbm_deinit();

	return 0;
	}
/*...e*/
