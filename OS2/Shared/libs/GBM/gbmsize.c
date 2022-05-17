/*

gbmsize.c - Change size of General Bitmap

History:
--------
(Heiko Nitzsche)

26-Apr-2006: Fix issue with comma separation between file and options.
             Now the file can have quotes and thus clearly separating
             it from the options.
             On OS/2 command line use: "\"file.ext\",options"
17-Dec-2006  Allow -a with both, -w and -h. This will frame the new bitmap.
02-Sep-2007  Add support for resampled scaling (option -f).
21-Jan-2008  Add support for 1bpp and 4bpp grayscale resampled scaling.
08-Feb-2008  Allocate memory from high memory for bitmap data to
             stretch limit for out-of-memory errors
             (requires kernel with high memory support)
             Removed maximum size limit
15-Aug-2008  Integrate new GBM types

10-Oct-2008: Changed recommended file specification template to
             "file.ext"\",options   or   "file.ext"\",\""options"

24-Sep-2010: Add SSE support for resampling scaler
01-Nov-2010: Add more resampling filters (blackman,catmullrom,quadratic,gaussian,kaiser)
*/

/* activate to enable measurement of conversion time */
/* #define MEASURE_TIME 1 */

#ifdef MEASURE_TIME
#if defined(__OS2__)
  #define INCL_DOSDATETIME
  #include <os2.h>
#elif defined(WIN32)
  #include <windows.h>
#elif defined(LINUX)
  #include <sys/time.h>
#endif
#endif

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
#include "gbmscale.h"
#include "gbmtool.h"


static char progname[] = "gbmsize";

/* ----------------- */

typedef struct
{
  char * name;   /* filter name */
  int    filter; /* filter type */
} FILTER_NAME_TABLE_DEF;

static const int FILTER_INDEX_SIMPLE = 0;

static FILTER_NAME_TABLE_DEF FILTER_NAME_TABLE [] =
{ { "simple"         , -1 },
  { "nearestneighbor", GBM_SCALE_FILTER_NEARESTNEIGHBOR },
  { "bilinear"       , GBM_SCALE_FILTER_BILINEAR        },
  { "bell"           , GBM_SCALE_FILTER_BELL            },
  { "bspline"        , GBM_SCALE_FILTER_BSPLINE         },
  { "mitchell"       , GBM_SCALE_FILTER_MITCHELL        },
  { "lanczos"        , GBM_SCALE_FILTER_LANCZOS         },
  { "blackman"       , GBM_SCALE_FILTER_BLACKMAN        },
  { "catmullrom"     , GBM_SCALE_FILTER_CATMULLROM      },
  { "quadratic"      , GBM_SCALE_FILTER_QUADRATIC       },
  { "gaussian"       , GBM_SCALE_FILTER_GAUSSIAN        },
  { "kaiser"         , GBM_SCALE_FILTER_KAISER          }
};
const int FILTER_NAME_TABLE_LENGTH = sizeof(FILTER_NAME_TABLE) /
                                     sizeof(FILTER_NAME_TABLE[0]);

/* ----------------- */

static void fatal(const char *fmt, ...)
{
   va_list   vars;
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

   fprintf(stderr, "usage:\n");
   fprintf(stderr, "%s [-w w] [-h h] [-a] [-f f] \"fn1.ext\"{\\\",\\\"\"opt\"} [\"fn2.ext\"{\\\",\\\"\"opt\"}]\n", progname);
   fprintf(stderr, "-w w           new width of bitmap  (default width of bitmap)\n");
   fprintf(stderr, "-h h           new height of bitmap (default height of bitmap)\n");
   fprintf(stderr, "-a             preserve aspect ratio\n");
   fprintf(stderr, "-f f           do quality scaling using one of the algorithms:\n");
   fprintf(stderr, "               * simple (default)\n");
   fprintf(stderr, "               * nearestneighbor,bilinear,bell,bspline,mitchell,lanczos\n");
   fprintf(stderr, "               * blackman,catmullrom,quadratic,gaussian,kaiser\n");
   fprintf(stderr, "                 Note: Only grayscale and true color images.\n");
   fprintf(stderr, "fn1.ext{,opt}  input filename (with any format specific options)\n");
   fprintf(stderr, "fn2.ext{,opt}  optional output filename (or will use fn1 if not present)\n");
   fprintf(stderr, "               ext's are used to deduce desired bitmap file formats\n");

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

   fprintf(stderr, "opt's          bitmap format specific options\n");

   fprintf(stderr, "\nIn case the filename contains a comma or spaces and options\n");
   fprintf(stderr,   "need to be added, syntax \"fn.ext\"{\\\",\\\"opt} or \"fn.ext\"{\\\",\\\"\"opt\"}\n");
   fprintf(stderr,   "must be used to clearly separate the filename from the options.\n");

   exit(1);
}

static int get_opt_value(const char *s, const char *name)
{
   int v;

   if ( s == NULL )
      fatal("missing %s argument", name);
   if ( !isdigit(s[0]) )
      fatal("bad %s argument", name);
   if ( s[0] == '0' && tolower(s[1]) == 'x' )
      sscanf(s + 2, "%x", &v);
   else
      v = atoi(s);

   return v;
}

static int get_opt_value_pos(const char *s, const char *name)
{
   int n;

   if ( (n = get_opt_value(s, name)) < 0 )
      fatal("%s should not be negative", name);
   return n;
}

static int get_opt_value_filterIndex(const char *s,
                                     const FILTER_NAME_TABLE_DEF *table,
                                     const int tableLength)
{
   int n;
   for (n = 0; n < tableLength; n++)
   {
       if (strcmp(table[n].name, s) == 0)
       {
           return n;
       }
   }
   return -1;
}

/* ---------------------------- */

static gbm_boolean isGrayscalePalette(const GBMRGB *gbmrgb, const int entries)
{
    if ((entries > 0) && (entries <= 0x100))
    {
        int i;
        for (i = 0; i < entries; i++)
        {
            if ((gbmrgb[i].r != gbmrgb[i].g) ||
                (gbmrgb[i].r != gbmrgb[i].b) ||
                (gbmrgb[i].g != gbmrgb[i].b))
            {
                return GBM_FALSE;
            }
        }
        return GBM_TRUE;
    }
    return GBM_FALSE;
}

/* ---------------------------- */

int main(int argc, char *argv[])
{
   GBMTOOL_FILEARG gbmfilearg;
   char    fn_src[GBMTOOL_FILENAME_MAX+1], fn_dst[GBMTOOL_FILENAME_MAX+1],
           opt_src[GBMTOOL_OPTIONS_MAX+1], opt_dst[GBMTOOL_OPTIONS_MAX+1];

   int   w = -1, h = -1, filterIndex = FILTER_INDEX_SIMPLE;
   int   fd, ft_src, ft_dst, i, flag;
   size_t stride, stride2;
   GBM_ERR  rc;
   GBMFT    gbmft;
   GBM      gbm, gbm2;
   GBMRGB   gbmrgb[0x100];
   gbm_boolean aspect = GBM_FALSE;
   gbm_boolean qualityScalingEnabled = GBM_FALSE;
   gbm_boolean isGrayscale = GBM_FALSE;
   gbm_u8    *data, *data2;

#ifdef MEASURE_TIME
#if defined(__OS2__)
   DATETIME start_time, end_time;
   double   time_s;
#elif defined(WIN32)
   SYSTEMTIME start_time, end_time;
   double     time_s;
#elif defined(LINUX)
   struct timeval start_time, end_time;
   double  time_s;
#endif
#endif

   for ( i = 1; i < argc; i++ )
   {
      if ( argv[i][0] != '-' )
         break;
      else if ( argv[i][1] == '-' )
      { ++i; break; }
      switch ( argv[i][1] )
      {
         case 'w':
            if ( ++i == argc ) usage();
            w = get_opt_value_pos(argv[i], "w");
            break;
         case 'h':
            if ( ++i == argc ) usage();
            h = get_opt_value_pos(argv[i], "h");
            break;
         case 'a':
            aspect = GBM_TRUE;
            break;
         case 'f':
            if ( ++i == argc ) usage();
            filterIndex = get_opt_value_filterIndex(argv[i],
                                                    FILTER_NAME_TABLE,
                                                    FILTER_NAME_TABLE_LENGTH);
            break;
         default:
            usage();
            break;
      }
   }

   if ( aspect && w == -1 && h == -1 )
   {
      fatal("-a can't be used if neither -w or -h is given");
   }
   if (filterIndex == -1)
   {
      fatal("wrong filter type");
   }
   else if (filterIndex != FILTER_INDEX_SIMPLE)
   {
      qualityScalingEnabled = GBM_TRUE;
   }

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

#ifdef MEASURE_TIME
#if defined(__OS2__)
   DosGetDateTime(&start_time);
#elif defined(WIN32)
   GetSystemTime(&start_time);
#elif defined(LINUX)
   gettimeofday(&start_time, NULL);
#endif
#endif

   /* processing */
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

   if ( aspect && (w != -1) && (h != -1))
   {
      if (((float)w/gbm.w) <= ((float)h/gbm.h))
      {
          h = ( gbm.h * w ) / gbm.w;
      }
      else
      {
          w = ( gbm.w * h ) / gbm.h;
      }
   }
   else
   {
      if ( w == -1 )
      {
         if ( aspect )
            w = ( gbm.w * h ) / gbm.h;
         else
            w = gbm.w;
      }
      if ( h == -1 )
      {
         if ( aspect )
            h = ( gbm.h * w ) / gbm.w;
         else
            h = gbm.h;
      }
   }

   if (w == 0 || h == 0)
   {
      gbm_io_close(fd);
      fatal("desired bitmap size of %dx%d, is silly", w, h);
   }

   if ( (rc = gbm_read_palette(fd, ft_src, &gbm, gbmrgb)) != GBM_ERR_OK )
   {
      gbm_io_close(fd);
      fatal("can't read palette of %s: %s", fn_src, gbm_err(rc));
   }

   gbm2   = gbm;
   gbm2.w = w;
   gbm2.h = h;

   if (qualityScalingEnabled)
   {
       if (gbm.bpp <= 8)
       {
           isGrayscale = isGrayscalePalette(gbmrgb, 1 << gbm.bpp);
       }
       if ((gbm.bpp <= 8) && (! isGrayscale))
       {
          gbm_io_close(fd);
          fatal("can't use filter '%s' for colour palette images",
                FILTER_NAME_TABLE[filterIndex].name);
       }
       if (isGrayscale)
       {
          gbm2.bpp = 8;
       }
   }

   stride = ( ((gbm.w * gbm.bpp + 31)/32) * 4 );
   if ( (data = gbmmem_malloc(stride * gbm.h)) == NULL )
   {
      gbm_io_close(fd);
      #if (ULONG_MAX > UINT_MAX)
      fatal("out of memory allocating %zu bytes for input bitmap", stride * gbm.h);
      #else
      fatal("out of memory allocating %u bytes for input bitmap", stride * gbm.h);
      #endif
   }

   stride2 = ( ((gbm2.w * gbm2.bpp + 31)/32) * 4 );
   if ( (data2 = gbmmem_malloc(stride2 * gbm2.h)) == NULL )
   {
      gbmmem_free(data);
      gbm_io_close(fd);
      #if (ULONG_MAX > UINT_MAX)
      fatal("out of memory allocating %zu bytes for output bitmap", stride2 * gbm2.h);
      #else
      fatal("out of memory allocating %u bytes for output bitmap", stride2 * gbm2.h);
      #endif
   }

   if ( (rc = gbm_read_data(fd, ft_src, &gbm, data)) != GBM_ERR_OK )
   {
      gbmmem_free(data2);
      gbmmem_free(data);
      gbm_io_close(fd);
      fatal("can't read bitmap data of %s: %s", fn_src, gbm_err(rc));
   }

   gbm_io_close(fd);

#ifdef MEASURE_TIME
#if defined(__OS2__)
   DosGetDateTime(&end_time);
   time_s = ((double) (end_time  .minutes * 60) + end_time  .seconds + (end_time  .hundredths/100.0)) -
            ((double) (start_time.minutes * 60) + start_time.seconds + (start_time.hundredths/100.0));
   printf("Elapsed time LOAD : %lf\n", time_s);
#elif defined(WIN32)
   GetSystemTime(&end_time);
   time_s = ((double) (end_time  .wMinute * 60) + end_time  .wSecond + (end_time  .wMilliseconds/1000.0)) -
            ((double) (start_time.wMinute * 60) + start_time.wSecond + (start_time.wMilliseconds/1000.0));
   printf("Elapsed time LOAD : %lf\n", time_s);
#elif defined(LINUX)
   gettimeofday(&end_time, NULL);
   time_s = ((double) (end_time  .tv_sec) + (end_time  .tv_usec/1000000.0)) -
            ((double) (start_time.tv_sec) + (start_time.tv_usec/1000000.0));
   printf("Elapsed time LOAD : %lf\n", time_s);
#endif
#endif

#ifdef MEASURE_TIME
#if defined(__OS2__)
   DosGetDateTime(&start_time);
#elif defined(WIN32)
   GetSystemTime(&start_time);
#elif defined(LINUX)
   gettimeofday(&start_time, NULL);
#endif
#endif

   if (qualityScalingEnabled)
   {
       if (isGrayscale)
       {
           rc = gbm_quality_scale_gray(data , gbm.w , gbm.h , gbm.bpp, gbmrgb,
                                       data2, gbm2.w, gbm2.h, gbmrgb,
                                       FILTER_NAME_TABLE[filterIndex].filter);
       }
       else
       {
           rc = gbm_quality_scale_bgra(data , gbm.w , gbm.h,
                                       data2, gbm2.w, gbm2.h, gbm.bpp,
                                       FILTER_NAME_TABLE[filterIndex].filter);
       }
   }
   else
   {
       rc = gbm_simple_scale(data, gbm.w, gbm.h, data2, gbm2.w, gbm2.h, gbm.bpp);
   }

#ifdef MEASURE_TIME
#if defined(__OS2__)
   DosGetDateTime(&end_time);
   time_s = ((double) (end_time  .minutes * 60) + end_time  .seconds + (end_time  .hundredths/100.0)) -
            ((double) (start_time.minutes * 60) + start_time.seconds + (start_time.hundredths/100.0));
   printf("Elapsed time SCALE: %lf\n", time_s);
#elif defined(WIN32)
   GetSystemTime(&end_time);
   time_s = ((double) (end_time  .wMinute * 60) + end_time  .wSecond + (end_time  .wMilliseconds/1000.0)) -
            ((double) (start_time.wMinute * 60) + start_time.wSecond + (start_time.wMilliseconds/1000.0));
   printf("Elapsed time SCALE: %lf\n", time_s);
#elif defined(LINUX)
   gettimeofday(&end_time, NULL);
   time_s = ((double) (end_time  .tv_sec) + (end_time  .tv_usec/1000000.0)) -
            ((double) (start_time.tv_sec) + (start_time.tv_usec/1000000.0));
   printf("Elapsed time SCALE: %lf\n", time_s);
#endif
#endif

   gbmmem_free(data);

   if (rc != GBM_ERR_OK)
   {
      gbmmem_free(data2);
      fatal("can't scale: %s", gbm_err(rc));
   }

#ifdef MEASURE_TIME
#if defined(__OS2__)
   DosGetDateTime(&start_time);
#elif defined(WIN32)
   GetSystemTime(&start_time);
#elif defined(LINUX)
   gettimeofday(&start_time, NULL);
#endif
#endif

   if ( (fd = gbm_io_create(fn_dst, GBM_O_WRONLY)) == -1 )
   {
      gbmmem_free(data2);
      fatal("can't create %s", fn_dst);
   }

   gbm_query_filetype(ft_dst, &gbmft);
   switch ( gbm2.bpp )
   {
      case 64:   flag = GBM_FT_W64;  break;
      case 48:   flag = GBM_FT_W48;  break;
      case 32:   flag = GBM_FT_W32;  break;
      case 24:   flag = GBM_FT_W24;  break;
      case 8:    flag = GBM_FT_W8;   break;
      case 4:    flag = GBM_FT_W4;   break;
      case 1:    flag = GBM_FT_W1;   break;
      default:   flag = 0;           break;
   }
   if ( (gbmft.flags & flag) == 0 )
   {
      gbm_io_close(fd);
      fatal("output bitmap format %s does not support writing %d bpp data",
            gbmft.short_name, gbm2.bpp);
   }

   if ( (rc = gbm_write(fn_dst, fd, ft_dst, &gbm2, gbmrgb, data2, opt_dst)) != GBM_ERR_OK )
   {
      gbm_io_close(fd);
      remove(fn_dst);
      gbmmem_free(data2);
      fatal("can't write %s: %s", fn_dst, gbm_err(rc));
   }

   gbm_io_close(fd);
   gbmmem_free(data2);

   gbm_deinit();

#ifdef MEASURE_TIME
#if defined(__OS2__)
   DosGetDateTime(&end_time);
   time_s = ((double) (end_time  .minutes * 60) + end_time  .seconds + (end_time  .hundredths/100.0)) -
            ((double) (start_time.minutes * 60) + start_time.seconds + (start_time.hundredths/100.0));
   printf("Elapsed time WRITE: %lf\n", time_s);
#elif defined(WIN32)
   GetSystemTime(&end_time);
   time_s = ((double) (end_time  .wMinute * 60) + end_time  .wSecond + (end_time  .wMilliseconds/1000.0)) -
            ((double) (start_time.wMinute * 60) + start_time.wSecond + (start_time.wMilliseconds/1000.0));
   printf("Elapsed time WRITE: %lf\n", time_s);
#elif defined(LINUX)
   gettimeofday(&end_time, NULL);
   time_s = ((double) (end_time  .tv_sec) + (end_time  .tv_usec/1000000.0)) -
            ((double) (start_time.tv_sec) + (start_time.tv_usec/1000000.0));
   printf("Elapsed time WRITE: %lf\n", time_s);
#endif
#endif

   return 0;
}


