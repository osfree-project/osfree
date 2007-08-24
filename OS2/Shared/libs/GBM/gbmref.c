/*

gbmref.c - Reflect General Bitmap

History:
--------
(Heiko Nitzsche)

26-Apr-2006: Fix issue with comma separation between file and options.
             Now the file can have quotes and thus clearly separating
             it from the options.
             On OS/2 command line use: "\"file.ext\",options"
*/

/*...sincludes:0:*/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#if defined(AIX) || defined(LINUX) || defined(SUN) || defined(MAC)
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
#include "gbmmir.h"
#include "gbmtool.h"

/*...vgbm\46\h:0:*/
/*...vgbmmir\46\h:0:*/
/*...e*/

static char progname[] = "gbmref";

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

	fprintf(stderr, "usage: %s [-h] [-v] [-t] \"\\\"fn1.ext\\\"{,opt}\" [--] [\"\\\"fn2.ext\\\"{,opt}\"]\n", progname);
	fprintf(stderr, "flags: -h             reflect horizontally\n");
	fprintf(stderr, "       -v             reflect vertically\n");
	fprintf(stderr, "       -t             transpose x by y\n");
	fprintf(stderr, "       fn1.ext{,opt}  input filename (with any format specific options)\n");
	fprintf(stderr, "       fn2.ext{,opt}  optional output filename (or will use fn1 if not present)\n");
	fprintf(stderr, "                      ext's are used to deduce desired bitmap file formats\n");

	gbm_init();
	gbm_query_n_filetypes(&n_ft);
	for ( ft = 0; ft < n_ft; ft++ )
		{
		GBMFT gbmft;

		gbm_query_filetype(ft, &gbmft);
		fprintf(stderr, "                      %s when ext in [%s]\n",
			gbmft.short_name, gbmft.extensions);
		}
	gbm_deinit();

	fprintf(stderr, "       opt's          bitmap format specific options\n");

	fprintf(stderr, "\n       In case the filename contains a comma or spaces and options\n");
	fprintf(stderr,   "       need to be added, the syntax \"\\\"fn.ext\\\"{,opt}\" must be used\n");
	fprintf(stderr,   "       to clearly separate the filename from the options.\n");

	exit(1);
	}
/*...e*/
/*...smain:0:*/
int main(int argc, char *argv[])
	{
	GBMTOOL_FILEARG gbmfilearg;
	char    fn_src[GBMTOOL_FILENAME_MAX+1], fn_dst[GBMTOOL_FILENAME_MAX+1],
                opt_src[GBMTOOL_OPTIONS_MAX+1], opt_dst[GBMTOOL_OPTIONS_MAX+1];

	BOOLEAN	horz = FALSE, vert = FALSE, trans = FALSE;
	int	fd, ft_src, ft_dst, i, stride, bytes, flag;
	GBM_ERR	rc;
	GBMFT	gbmft;
	GBM	gbm;
	GBMRGB	gbmrgb[0x100];
	byte	*data;

	for ( i = 1; i < argc; i++ )
		{
		if ( argv[i][0] != '-' )
			break;
		else if ( argv[i][1] == '-' )
			{ ++i; break; }
		switch ( argv[i][1] )
			{
			case 'h':	horz = TRUE;	break;
			case 'v':	vert = TRUE;	break;
			case 't':	trans = TRUE;	break;
			default:	usage();	break;
			}
		}

	if ( i == argc )
		usage();

	/* Split filename and file options. */
	gbmfilearg.argin = argv[i++];
 	if (strcmp(gbmfilearg.argin, "\"\"") == 0)
	{
	  usage();
	}
	if (gbmtool_parse_argument(&gbmfilearg, FALSE) != GBM_ERR_OK)
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
	if (gbmtool_parse_argument(&gbmfilearg, FALSE) != GBM_ERR_OK)
	{
	  fatal("can't parse destination filename %s", gbmfilearg.argin);
	}
	strcpy(fn_dst , gbmfilearg.files->filename);
	strcpy(opt_dst, gbmfilearg.options);
	gbmtool_free_argument(&gbmfilearg);

	if (i < argc)
		usage();

	/* do processing */
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
		case 64:
		case 48:
		case 32:
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
	switch ( gbm.bpp )
		{
		case 64:	flag = GBM_FT_W64;	break;
		case 48:	flag = GBM_FT_W48;	break;
		case 32:	flag = GBM_FT_W32;	break;
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
			gbmft.short_name, gbm.bpp);
		}

	if ( (rc = gbm_read_palette(fd, ft_src, &gbm, gbmrgb)) != GBM_ERR_OK )
		{
		gbm_io_close(fd);
		fatal("can't read palette of %s: %s", fn_src, gbm_err(rc));
		}

	stride = ( ((gbm.w * gbm.bpp + 31)/32) * 4 );
	bytes = stride * gbm.h;
	if ( (data = malloc((size_t) bytes)) == NULL )
		{
		gbm_io_close(fd);
		fatal("out of memory allocating %d bytes for bitmap", bytes);
		}

	if ( (rc = gbm_read_data(fd, ft_src, &gbm, data)) != GBM_ERR_OK )
		{
		gbm_io_close(fd);
		fatal("can't read bitmap data of %s: %s", fn_src, gbm_err(rc));
		}

	gbm_io_close(fd);

	if ( vert )
		if ( !gbm_ref_vert(&gbm, data) )
			fatal("unable to reflect vertically");

	if ( horz )
		if ( !gbm_ref_horz(&gbm, data) )
			fatal("unable to reflect horizontally");

	if ( trans )
		{
		int t, stride_t = ((gbm.h * gbm.bpp + 31) / 32) * 4;
		byte *data_t;
		if ( (data_t = malloc((size_t) (gbm.w * stride_t))) == NULL )
			fatal("out of memory allocating %u bytes", gbm.w * stride_t);

		gbm_transpose(&gbm, data, data_t);
		t = gbm.w; gbm.w = gbm.h; gbm.h = t;
		free(data);
		data = data_t;
		}

	if ( (fd = gbm_io_create(fn_dst, GBM_O_WRONLY)) == -1 )
		fatal("can't create %s", fn_dst);

	if ( (rc = gbm_write(fn_dst, fd, ft_dst, &gbm, gbmrgb, data, opt_dst)) != GBM_ERR_OK )
		{
		gbm_io_close(fd);
		remove(fn_dst);
		fatal("can't write %s: %s", fn_dst, gbm_err(rc));
		}

	gbm_io_close(fd);

	free(data);

	gbm_deinit();

	return 0;
	}
/*...e*/
