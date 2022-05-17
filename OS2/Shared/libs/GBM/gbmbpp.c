/*

gbmbpp.c - Change bits per pixel in a General Bitmap

History:
--------
(Heiko Nitzsche)

26-Apr-2006: Fix issue with comma separation between file and options.
             Now the file can have quotes and thus clearly separating
             it from the options.
             On OS/2 command line use: "\"file.ext\",options"
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
#include "gbmerr.h"
#include "gbmtrunc.h"
#include "gbmht.h"
#include "gbmhist.h"
#include "gbmmcut.h"
#include "gbmtool.h"


static char progname[] = "gbmbpp";

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

	fprintf(stderr, "usage: %s [-m map] [-e] [-hN] [-p \"fnp.ext\"{\\\",\\\"\"opt\"}]\n", progname);
	fprintf(stderr, "              [--] \"fn1.ext\"{\\\",\\\"\"opt\"} [\"fn2.ext\"{\\\",\\\"\"opt\"}]\n");
	fprintf(stderr, "flags: -m map            mapping to perform (default 7x8x4)\n");
	fprintf(stderr, "                         bw           black and white\n");
	fprintf(stderr, "                         vga          16 colour VGA\n");
	fprintf(stderr, "                         8            8 colour (in 4 bit file)\n");
	fprintf(stderr, "                         4g           4 bit greyscale\n");
	fprintf(stderr, "                         7x8x4        7 levels red, 8 green, 4 blue 8514/A\n");
	fprintf(stderr, "                         6x6x6        6 levels red, 6 green, 6 blue\n");
	fprintf(stderr, "                         4x4x4        4 levels red, 4 green, 4 blue\n");
	fprintf(stderr, "                         8g           8 bit greyscale\n");
	fprintf(stderr, "                         tripel       64 reds, 64 greens, 64 blues tripel\n");
	fprintf(stderr, "                         pal1bpp      map to 1bpp, palette specified via -p\n");
	fprintf(stderr, "                         pal4bpp      map to 4bpp, palette specified via -p\n");
	fprintf(stderr, "                         pal8bpp      map to 8bpp, palette specified via -p\n");
	fprintf(stderr, "                         freqR:G:B:N  keep R red, G green, b blue bits, and map\n");
	fprintf(stderr, "                                      to N most used colours in 8 bit palette\n");
	fprintf(stderr, "                         mcutN        median cut to N colours\n");
	fprintf(stderr, "                         R:G:B        keep R red, G green, B blue bits\n");
	fprintf(stderr, "       -e                enable error-diffusion (default is to truncate)\n");
	fprintf(stderr, "                         -e only with -m bw, vga, 4g, 7x8x4, 6x6x6 or 4x4x4\n");
	fprintf(stderr, "       -h                enable halftoning (default is to truncate)\n");
	fprintf(stderr, "                         -h only with -m 7x8x4, 6x6x6, 8, vga or R:G:B\n");
	fprintf(stderr, "                         -e and -h may not be used together\n");
	fprintf(stderr, "                         N is a halftoning algorithm number (default 0)\n");
	fprintf(stderr, "       -p fnp.ext{,opt}  palette is taken from this bitmap\n");
	fprintf(stderr, "       fn1.ext{,opt}     input filename (with any format specific options)\n");
	fprintf(stderr, "       fn2.ext{,opt}     optional output filename (or use fn1 if not present)\n");
	fprintf(stderr, "                         ext's are used to deduce desired bitmap file formats\n");

	gbm_init();
	gbm_query_n_filetypes(&n_ft);
	for ( ft = 0; ft < n_ft; ft++ )
		{
		GBMFT gbmft;

		gbm_query_filetype(ft, &gbmft);
		fprintf(stderr, "                         %s when ext in [%s]\n",
			gbmft.short_name, gbmft.extensions);
		}
	gbm_deinit();

	fprintf(stderr, "       opt's             bitmap format specific options\n");

	fprintf(stderr, "\n       In case the filename contains a comma or spaces and options\n");
    fprintf(stderr,   "       need to be added, syntax \"fn.ext\"{\\\",\\\"opt} or \"fn.ext\"{\\\",\\\"\"opt\"}\n");
    fprintf(stderr,   "       must be used to clearly separate the filename from the options.\n");

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
/*...smapinfos:0:*/
#define	CVT_BW		0
#define	CVT_VGA		1
#define	CVT_8		2
#define	CVT_4G		3
#define	CVT_784		4
#define	CVT_666		5
#define	CVT_444		6
#define	CVT_8G		7
#define	CVT_TRIPEL	8
#define	CVT_PAL1BPP	9
#define	CVT_PAL4BPP	10
#define	CVT_PAL8BPP	11
#define	CVT_RGB		12
#define	CVT_FREQ	13
#define	CVT_MCUT	14
#define	CVT_ERRDIFF	0x4000
#define	CVT_HALFTONE	0x2000

typedef struct { char *name; int m; int dest_bpp; } MAPINFO;

static MAPINFO mapinfos[] =
{
	{ "bw",		CVT_BW,		1 },
	{ "vga",		CVT_VGA,	4 },
	{ "8",		CVT_8,		4 },
	{ "4g",		CVT_4G,		4 },
	{ "7x8x4",	CVT_784,	8 },
	{ "6x6x6",	CVT_666,	8 },
	{ "4x4x4",	CVT_444,	8 },
	{ "8g",		CVT_8G,		8 },
	{ "tripel",	CVT_TRIPEL,	8 },
	{ "pal1bpp",	CVT_PAL1BPP,	1 },
	{ "pal4bpp",	CVT_PAL4BPP,	4 },
	{ "pal8bpp",	CVT_PAL8BPP,	8 }
};

#define	N_MAPINFOS	(sizeof(mapinfos)/sizeof(mapinfos[0]))
/*...e*/
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
/*...sget_pal:0:*/
static void get_pal(const char *pal, GBMRGB *gbmrgb, int bpp, int *ncols)
	{
	GBMTOOL_FILEARG gbmfilearg;
	char fn_pal[GBMTOOL_FILENAME_MAX+1], opt_pal[GBMTOOL_OPTIONS_MAX+1];
	int fd, ft_pal, i;
	GBM gbm;
	GBM_ERR	rc;

	if ( pal == NULL )
		fatal("palette file must be specified");

	/* Split filename and file options. */
	gbmfilearg.argin = pal;
	if (gbmtool_parse_argument(&gbmfilearg, GBM_FALSE) != GBM_ERR_OK)
	{
	  fatal("can't parse palette filename %s", gbmfilearg.argin);
	}
	strcpy(fn_pal , gbmfilearg.files->filename);
	strcpy(opt_pal, gbmfilearg.options);
	gbmtool_free_argument(&gbmfilearg);

	if ( gbm_guess_filetype(fn_pal, &ft_pal) != GBM_ERR_OK )
		fatal("can't guess bitmap file format for %s", fn_pal);

	if ( (fd = gbm_io_open(fn_pal, GBM_O_RDONLY)) == -1 )
		fatal("can't open %s", fn_pal);

	if ( (rc = gbm_read_header(fn_pal, fd, ft_pal, &gbm, opt_pal)) != GBM_ERR_OK )
		{
		gbm_io_close(fd);
		fatal("can't read header of %s: %s", fn_pal, gbm_err(rc));
		}

	if ( gbm.bpp > 8 )
		{
		gbm_io_close(fd);
		fatal("bitmap %s should contain a palette", fn_pal);
		}

	if ( gbm.bpp > bpp )
		{
		gbm_io_close(fd);
		fatal("bitmap %s has too many colours in palette for chosen mapping", fn_pal);
		}

	/* Pre-initialise palette with medium grey */
	for ( i = 0; i < (1<<bpp); i++ )
		gbmrgb[i].r =
		gbmrgb[i].g =
		gbmrgb[i].b = 0x80;

	if ( (rc = gbm_read_palette(fd, ft_pal, &gbm, gbmrgb)) != GBM_ERR_OK )
		{
		gbm_io_close(fd);
		fatal("can't read palette of %s: %s", fn_pal, gbm_err(rc));
		}

	gbm_io_close(fd);

	*ncols = ( 1 << gbm.bpp );
	}
/*...e*/
/*...sexpand_to_24bit:0:*/
static void expand_to_24bit(GBM *gbm, GBMRGB *gbmrgb, gbm_u8 **data)
	{
	size_t stride = ((gbm->w * gbm->bpp + 31)/32) * 4;
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
	gbm_u8	c = 0;

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
/*...sto_grey_pal:0:*/
static void to_grey_pal(GBMRGB *gbmrgb)
	{
	int i;

	for ( i = 0; i < 0x100; i++ )
		gbmrgb[i].r =
		gbmrgb[i].g =
		gbmrgb[i].b = (gbm_u8) i;
	}
/*...e*/
/*...sto_grey:0:*/
static void to_grey(GBM *gbm, const gbm_u8 *src_data, gbm_u8 *dest_data)
	{
	int src_stride  = ((gbm->w * 3 + 3) & ~3);
	int dest_stride = ((gbm->w     + 3) & ~3);
	int y;

	for ( y = 0; y < gbm->h; y++ )
		{
		const gbm_u8 *src  = src_data;
		      gbm_u8 *dest = dest_data;
		int x;

		for ( x = 0; x < gbm->w; x++ )
			{
			gbm_u8 b = *src++;
			gbm_u8 g = *src++;
			gbm_u8 r = *src++;

			*dest++ = (gbm_u8) (((gbm_u16) r * 77U + (gbm_u16) g * 150U + (gbm_u16) b * 29U) >> 8);
			}

		src_data  += src_stride;
		dest_data += dest_stride;
		}
	gbm->bpp = 8;
	}
/*...e*/
/*...stripel_pal:0:*/
static void tripel_pal(GBMRGB *gbmrgb)
	{
	int	i;

	memset(gbmrgb, 0, 0x100 * sizeof(GBMRGB));

	for ( i = 0; i < 0x40; i++ )
		{
		gbmrgb[i       ].r = (gbm_u8) (i << 2);
		gbmrgb[i + 0x40].g = (gbm_u8) (i << 2);
		gbmrgb[i + 0x80].b = (gbm_u8) (i << 2);
		}
	}
/*...e*/
/*...stripel:0:*/
static void tripel(GBM *gbm, const gbm_u8 *src_data, gbm_u8 *dest_data)
	{
	int src_stride  = ((gbm->w * 3 + 3) & ~3);
	int dest_stride = ((gbm->w     + 3) & ~3);
	int y;

	for ( y = 0; y < gbm->h; y++ )
		{
		const gbm_u8 *src  = src_data;
		      gbm_u8 *dest = dest_data;
		int x;

		for ( x = 0; x < gbm->w; x++ )
			{
			gbm_u8 b = *src++;
			gbm_u8 g = *src++;
			gbm_u8 r = *src++;

			switch ( (x+y)%3 )
				{
				case 0:	*dest++ = (gbm_u8)         (r >> 2) ;	break;
				case 1:	*dest++ = (gbm_u8) (0x40 + (g >> 2));	break;
				case 2:	*dest++ = (gbm_u8) (0x80 + (b >> 2));	break;
				}
			}

		src_data  += src_stride;
		dest_data += dest_stride;
		}
	gbm->bpp = 8;
	}
/*...e*/

int main(int argc, char *argv[])
	{
	GBMTOOL_FILEARG gbmfilearg;
	char    fn_src[GBMTOOL_FILENAME_MAX+1], fn_dst[GBMTOOL_FILENAME_MAX+1],
	        opt_src[GBMTOOL_OPTIONS_MAX+1], opt_dst[GBMTOOL_OPTIONS_MAX+1];

	gbm_boolean	errdiff = GBM_FALSE, halftone = GBM_FALSE, ok = GBM_TRUE;
	int	htmode = 0;
	char	*map = "7x8x4";
	char	*pal = NULL;
	int	fd, ft_src, ft_dst, i, flag, m, dest_bpp;
	size_t	stride, bytes;
	gbm_u8	rm = 0, gm = 0, bm = 0;
	int	ncols;
	GBM_ERR	rc;
	GBMFT	gbmft;
	GBM	gbm;
	GBMRGB	gbmrgb[0x100];
	gbm_u8	*data;

/*...sprocess command line options:8:*/
for ( i = 1; i < argc; i++ )
	{
	if ( argv[i][0] != '-' )
		break;
	else if ( argv[i][1] == '-' )
		{ ++i; break; }
	switch ( argv[i][1] )
		{
		case 'e':	errdiff = GBM_TRUE;
				break;
		case 'h':	halftone = GBM_TRUE;
				if ( argv[i][2] != '\0' && isdigit(argv[i][2]) )
					htmode = argv[i][2] - '0';
				break;
		case 'm':	if ( ++i == argc )
					fatal("expected map argument");
				map = argv[i];
				break;
		case 'p':	if ( ++i == argc )
					fatal("expected palette argument");
				pal = argv[i];
				break;
		default:	usage();
				break;
		}
	}

if ( errdiff && halftone )
	fatal("error-diffusion and halftoning can't both be done at once");
/*...e*/
/*...sdeduce mapping and bits per pixel etc\46\:8:*/
if ( get_masks(map, &rm, &gm, &bm) && map[5] == '\0' )
	{
	m = CVT_RGB;
	dest_bpp = 24;
	}
else if ( same(map, "freq", 4) )
	{
	m = CVT_FREQ;
	dest_bpp = 8;
	if ( !get_masks(map+4, &rm, &gm, &bm) )
		fatal("freqR:G:B:N has bad/missing R:G:B");
	if ( map[9] != ':' )
		fatal("freqR:G:B:N has bad/missing :N");
	sscanf(map+10, "%i", &ncols);
	if ( ncols < 1 || ncols > 256 )
		fatal("freqR:G:B:N N number between 1 and 256 required");
	}
else if ( same(map, "mcut", 4) )
	{
	m = CVT_MCUT;
	dest_bpp = 8;
	sscanf(map+4, "%i", &ncols);
	if ( ncols < 1 || ncols > 256 )
		fatal("mcutN N number between 1 and 256 required");
	}
else
	{
	int j;

	for ( j = 0; j < N_MAPINFOS; j++ )
		if ( same(map, mapinfos[j].name, strlen(map)) )
			break;
	if ( j == N_MAPINFOS )
		fatal("unrecognised mapping %s", map);
	m        = mapinfos[j].m;
	dest_bpp = mapinfos[j].dest_bpp;
	}
/*...e*/

	if ( i == argc )
	{
	  usage();
	}

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
	{
	  usage();
	}

	gbm_init();

	if ( gbm_guess_filetype(fn_src, &ft_src) != GBM_ERR_OK )
		fatal("can't guess bitmap file format for %s", fn_src);

	if ( gbm_guess_filetype(fn_dst, &ft_dst) != GBM_ERR_OK )
		fatal("can't guess bitmap file format for %s", fn_dst);

	if ( (fd = gbm_io_open(fn_src, GBM_O_RDONLY)) == -1 )
		fatal("can't open %s", fn_src);

	if ( (rc = gbm_read_header(fn_src, fd, ft_src, &gbm, opt_src)) != GBM_ERR_OK )
		{
		gbm_io_close(fd);
		fatal("can't read header of %s: %s", fn_src, gbm_err(rc));
		}

        /* check for color depth supported by algorithms */
	switch ( gbm.bpp )
		{
		case 24:
		case 8:
		case 4:
		case 1:
                   break;
                default:
 		  {
		     gbm_io_close(fd);
		     fatal("%d bpp file is not supported", gbm.bpp);
		  }
		}

	gbm_query_filetype(ft_dst, &gbmft);
	switch ( dest_bpp )
		{
		case 24:	flag = GBM_FT_W24;	break;
		case 8:		flag = GBM_FT_W8;	break;
		case 4:		flag = GBM_FT_W4;	break;
		case 1:		flag = GBM_FT_W1;	break;
                default:        flag = 0;               break;
		}

	if ( (gbmft.flags & flag) == 0 )
		{
		gbm_io_close(fd);
		fatal("output bitmap format %s does not support writing %d bpp data",
			gbmft.short_name, dest_bpp);
		}

	if ( (rc = gbm_read_palette(fd, ft_src, &gbm, gbmrgb)) != GBM_ERR_OK )
		{
		gbm_io_close(fd);
		fatal("can't read palette of %s: %s", fn_src, gbm_err(rc));
		}

	stride = ( ((gbm.w * gbm.bpp + 31)/32) * 4 );
	bytes = stride * gbm.h;
	if ( (data = gbmmem_malloc(bytes)) == NULL )
		{
		gbm_io_close(fd);
		#if (ULONG_MAX > UINT_MAX)
		fatal("out of memory allocating %zu bytes", bytes);
		#else
		fatal("out of memory allocating %u bytes", bytes);
		#endif
		}

	if ( (rc = gbm_read_data(fd, ft_src, &gbm, data)) != GBM_ERR_OK )
		{
		gbmmem_free(data);
		gbm_io_close(fd);
		fatal("can't read bitmap data of %s: %s", fn_src, gbm_err(rc));
		}

	gbm_io_close(fd);

	/* Now expand bits per pixel if necessary */
	expand_to_24bit(&gbm, gbmrgb, &data);

	if ( errdiff )
		m |= CVT_ERRDIFF;

	if ( halftone )
		m |= CVT_HALFTONE;

	switch ( m )
		{
		case CVT_BW:
			gbm_trunc_pal_BW(gbmrgb);
			gbm_trunc_BW(&gbm, data, data);
			break;
		case CVT_4G:
			gbm_trunc_pal_4G(gbmrgb);
			gbm_trunc_4G(&gbm, data, data);
			break;
		case CVT_8:
			gbm_trunc_pal_8(gbmrgb);
			gbm_trunc_8(&gbm, data, data);
			break;
		case CVT_VGA:
			gbm_trunc_pal_VGA(gbmrgb);
			gbm_trunc_VGA(&gbm, data, data);
			break;
		case CVT_784:
			gbm_trunc_pal_7R8G4B(gbmrgb);
			gbm_trunc_7R8G4B(&gbm, data, data);
			break;
		case CVT_666:
			gbm_trunc_pal_6R6G6B(gbmrgb);
			gbm_trunc_6R6G6B(&gbm, data, data);
			break;
		case CVT_444:
			gbm_trunc_pal_4R4G4B(gbmrgb);
			gbm_trunc_4R4G4B(&gbm, data, data);
			break;
		case CVT_8G:
			to_grey_pal(gbmrgb);
			to_grey(&gbm, data, data);
			break;
		case CVT_TRIPEL:
			tripel_pal(gbmrgb);
			tripel(&gbm, data, data);
			break;
		case CVT_PAL1BPP:
			get_pal(pal, gbmrgb, 1, &ncols);
			gbm_trunc_1bpp(&gbm, data, data, gbmrgb, ncols);
			break;
		case CVT_PAL4BPP:
			get_pal(pal, gbmrgb, 4, &ncols);
			gbm_trunc_4bpp(&gbm, data, data, gbmrgb, ncols);
			break;
		case CVT_PAL8BPP:
			get_pal(pal, gbmrgb, 8, &ncols);
			gbm_trunc_8bpp(&gbm, data, data, gbmrgb, ncols);
			break;
		case CVT_FREQ:
			memset(gbmrgb, 0, sizeof(gbmrgb));
			ok = gbm_hist(&gbm, data, gbmrgb, data, ncols, rm, gm, bm);
			break;
		case CVT_RGB:
			gbm_trunc_24(&gbm, data, data, rm, gm, bm);
			break;
		case CVT_MCUT:
			ok = gbm_mcut(&gbm, data, gbmrgb, data, ncols);
			break;
		case CVT_BW | CVT_ERRDIFF:
			gbm_errdiff_pal_BW(gbmrgb);
			ok = gbm_errdiff_BW(&gbm, data, data);
			break;
		case CVT_4G | CVT_ERRDIFF:
			gbm_errdiff_pal_4G(gbmrgb);
			ok = gbm_errdiff_4G(&gbm, data, data);
			break;
		case CVT_8 | CVT_ERRDIFF:
			gbm_errdiff_pal_8(gbmrgb);
			ok = gbm_errdiff_8(&gbm, data, data);
			break;
		case CVT_VGA | CVT_ERRDIFF:
			gbm_errdiff_pal_VGA(gbmrgb);
			ok = gbm_errdiff_VGA(&gbm, data, data);
			break;
		case CVT_784 | CVT_ERRDIFF:
			gbm_errdiff_pal_7R8G4B(gbmrgb);
			ok = gbm_errdiff_7R8G4B(&gbm, data, data);
			break;
		case CVT_666 | CVT_ERRDIFF:
			gbm_errdiff_pal_6R6G6B(gbmrgb);
			ok = gbm_errdiff_6R6G6B(&gbm, data, data);
			break;
		case CVT_444 | CVT_ERRDIFF:
			gbm_errdiff_pal_4R4G4B(gbmrgb);
			ok = gbm_errdiff_4R4G4B(&gbm, data, data);
			break;
		case CVT_RGB | CVT_ERRDIFF:
			ok = gbm_errdiff_24(&gbm, data, data, rm, gm, bm);
			break;
		case CVT_784 | CVT_HALFTONE:
			gbm_ht_pal_7R8G4B(gbmrgb);
			gbm_ht_7R8G4B_2x2(&gbm, data, data);
			break;
		case CVT_666 | CVT_HALFTONE:
			gbm_ht_pal_6R6G6B(gbmrgb);
			gbm_ht_6R6G6B_2x2(&gbm, data, data);
			break;
		case CVT_444 | CVT_HALFTONE:
			gbm_ht_pal_4R4G4B(gbmrgb);
			gbm_ht_4R4G4B_2x2(&gbm, data, data);
			break;
		case CVT_8 | CVT_HALFTONE:
			gbm_ht_pal_8(gbmrgb);
			switch ( htmode )
				{
				default:
				case 0: gbm_ht_8_3x3(&gbm, data, data); break;
				case 1: gbm_ht_8_2x2(&gbm, data, data); break;
				}
			break;
		case CVT_VGA | CVT_HALFTONE:
			gbm_ht_pal_VGA(gbmrgb);
			switch ( htmode )
				{
				default:
				case 0: gbm_ht_VGA_3x3(&gbm, data, data); break;
				case 1: gbm_ht_VGA_2x2(&gbm, data, data); break;
				}
			break;
		case CVT_RGB | CVT_HALFTONE:
			gbm_ht_24_2x2(&gbm, data, data, rm, gm, bm);
			break;
		default:
		        gbmmem_free(data);
			fatal("bad mapping/error-diffusion/halftone combination");
		}

	if ( !ok )
        {
		gbmmem_free(data);
		fatal("unable to perform mapping");
        }

	gbm.bpp = dest_bpp;

	if ( (fd = gbm_io_create(fn_dst, GBM_O_WRONLY)) == -1 )
		{
		gbmmem_free(data);
		fatal("can't create %s", fn_dst);
		}

	if ( (rc = gbm_write(fn_dst, fd, ft_dst, &gbm, gbmrgb, data, opt_dst)) != GBM_ERR_OK )
		{
		gbm_io_close(fd);
		remove(fn_dst);
		gbmmem_free(data);
		fatal("can't write %s: %s", fn_dst, gbm_err(rc));
		}

	gbm_io_close(fd);

	gbmmem_free(data);

	gbm_deinit();

	return 0;
	}
/*...e*/

