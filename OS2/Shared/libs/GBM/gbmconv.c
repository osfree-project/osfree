/*

gbmconv.c - Converts one Bitmap file supported by Generalized Bitmap Module to another one

History:
--------
(Heiko Nitzsche)

26-Apr-2006: Fix issue with comma separation between file and options.
             Now the file can have quotes and thus clearly separating
             it from the options.
             On OS/2 command line use: "\"fn.ext\"{,opt}"

*/

/* activate to enable measurement of conversion time */
/* #define MEASURE_TIME */

#ifdef MEASURE_TIME
#ifdef __OS2__
  #define INCL_DOSDATETIME
  #include <os2.h>
#endif
#endif

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
#include "gbmtool.h"


static char progname[] = "gbmconv";

static void fatal(const char *fmt, ...)
{
    va_list    vars;
    char s[256+1];

    va_start(vars, fmt);
    vsprintf(s, fmt, vars);
    va_end(vars);
    fprintf(stderr, "%s: %s\n", progname, s);
    exit(1);
}

static void usage(void)
{
    int ft, n_ft;

    fprintf(stderr, "usage: %s \"\\\"fn1.ext\\\"{,opt}\" \"\\\"fn2.ext\\\"{,opt}\"\n", progname);
    fprintf(stderr, "       fn1.ext{,opt} input filename (with any format specific options)\n");
    fprintf(stderr, "       fn2.ext{,opt} output filename (with any format specific options)\n");
    fprintf(stderr, "                     ext's are used to deduce desired bitmap file formats\n");

    gbm_init();
    gbm_query_n_filetypes(&n_ft);
    for ( ft = 0; ft < n_ft; ft++ )
    {
        GBMFT gbmft;

        gbm_query_filetype(ft, &gbmft);
        fprintf(stderr, "                     %s when ext in [%s]\n",
            gbmft.short_name, gbmft.extensions);
    }
    gbm_deinit();

    fprintf(stderr, "       opt's         bitmap format specific options\n");

    fprintf(stderr, "\n       In case the filename contains a comma or spaces and options\n");
    fprintf(stderr,   "       need to be added, the syntax \"\\\"fn.ext\\\"{,opt}\" must be used\n");
    fprintf(stderr,   "       to clearly separate the filename from the options.\n");

    exit(1);
}

/* ------------------------------ */

int main(int argc, char *argv[])
{
    GBMTOOL_FILEARG gbmfilearg;
    char     fn_src[GBMTOOL_FILENAME_MAX+1], fn_dst[GBMTOOL_FILENAME_MAX+1],
             opt_src[GBMTOOL_OPTIONS_MAX+1], opt_dst[GBMTOOL_OPTIONS_MAX+1];

    int      fd, ft_src, ft_dst, stride, flag;
    GBM_ERR  rc;
    GBMFT    gbmft;
    GBM      gbm;
    GBMRGB   gbmrgb[0x100];
    byte    *data;

#ifdef MEASURE_TIME
#ifdef __OS2__
   DATETIME start_time, end_time;
   double   time_s;

   DosGetDateTime(&start_time);
#endif
#endif

    if ( argc < 3 )
    {
      usage();
    }

    /* Split filename and file options. */
    gbmfilearg.argin = argv[1];
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

    gbmfilearg.argin = argv[2];
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

    /* do processing */
    gbm_init();

    if ( gbm_guess_filetype(fn_src, &ft_src) != GBM_ERR_OK )
    {
      fatal("can't guess bitmap file format for %s", fn_src);
    }

    if ( gbm_guess_filetype(fn_dst, &ft_dst) != GBM_ERR_OK )
    {
      fatal("can't guess bitmap file format for %s", fn_dst);
    }

    if ( (fd = gbm_io_open(fn_src, GBM_O_RDONLY)) == -1 )
    {
      fatal("can't open %s", fn_src);
    }

    if ( (rc = gbm_read_header(fn_src, fd, ft_src, &gbm, opt_src)) != GBM_ERR_OK )
    {
      gbm_io_close(fd);
      fatal("can't read header of %s: %s", fn_src, gbm_err(rc));
    }

    gbm_query_filetype(ft_dst, &gbmft);
    switch ( gbm.bpp )
    {
        case 64: flag = GBM_FT_W64; break;
        case 48: flag = GBM_FT_W48; break;
        case 32: flag = GBM_FT_W32; break;
        case 24: flag = GBM_FT_W24; break;
        case  8: flag = GBM_FT_W8;  break;
        case  4: flag = GBM_FT_W4;  break;
        case  1: flag = GBM_FT_W1;  break;
        default: flag = 0;          break;
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
    if ( (data = malloc((size_t) (stride * gbm.h))) == NULL )
    {
      gbm_io_close(fd);
      fatal("out of memory allocating %d bytes for input bitmap", stride * gbm.h);
    }

    if ( (rc = gbm_read_data(fd, ft_src, &gbm, data)) != GBM_ERR_OK )
    {
      free(data);
      gbm_io_close(fd);
      fatal("can't read bitmap data of %s: %s", fn_src, gbm_err(rc));
    }

    gbm_io_close(fd);

    if ( (fd = gbm_io_create(fn_dst, GBM_O_WRONLY)) == -1 )
    {
      free(data);
      fatal("can't create %s", fn_dst);
    }

    if ( (rc = gbm_write(fn_dst, fd, ft_dst, &gbm, gbmrgb, data, opt_dst)) != GBM_ERR_OK )
    {
      free(data);
      gbm_io_close(fd);
      remove(fn_dst);
      fatal("can't write %s: %s", fn_dst, gbm_err(rc));
    }

    free(data);

    gbm_io_close(fd);
    gbm_deinit();

#ifdef MEASURE_TIME
#ifdef __OS2__
   DosGetDateTime(&end_time);
   time_s = ((double) (end_time  .minutes * 60) + end_time  .seconds + (end_time  .hundredths/100.0)) -
            ((double) (start_time.minutes * 60) + start_time.seconds + (start_time.hundredths/100.0));
   printf("Elapsed time: %lf\n", time_s);
#endif
#endif

    return 0;
}


