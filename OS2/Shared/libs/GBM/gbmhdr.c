/*

gbmhdr.c - Display General Bitmaps header

History:
--------
(Heiko Nitzsche)

26-Apr-2006: Change mechanism to resolve shell provided file names
             via regular expressions on OS/2 due to inclusion of
             OpenWatcom support (which doesn't have setargv.obj).

             Fix issue with comma separation between file and options.
             Now the file can have quotes and thus clearly separating
             it from the options.
             On OS/2 command line use: "\"file*.ext\",options"

30-Apr-2006: Always report full colour depth even though the user
             did not specify ext_bpp.

20-May-2006: Dynamically load gbm.dll for querying image page count
             to allow it to load also with older gbm.dll versions
             on OS/2.

16-Aug-2006: Remove the extension .DLL from GBM.DLL in DosLoadModule
             so that the dynamic lookup also works with LIBPATHSTRICT enabled.

01-Oct-2006: Fix a division by zero crash that happened for bitmaps with size smaller
             than 1 byte (e.g. 1x1x1bpp).

15-Aug-2008: Integrate new GBM types
*/

/*...sincludes:0:*/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
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

#if defined(__OS2__) || defined(OS2)
  #define INCL_DOS
  #include <os2.h>
#endif

#include "gbm.h"
#include "gbmtool.h"

/* ----------------------------- */

static gbm_boolean SupportsNumberOfPagesQuery(void);
static gbm_boolean GetNumberOfPages(const char * fileName, const int fd, const int ft, int * numPages);

/* ----------------------------- */

static char progname[] = "gbmhdr";

static void usage(void)
{
    int ft, n_ft;

    fprintf(stderr, "usage: %s [-g] [-s] [--] {\"\\\"fn.ext\\\"{,opt}\"} ...\n", progname);
    fprintf(stderr, "       -g           don't guess bitmap format, try each type\n");
    fprintf(stderr, "       -s           be silent about errors\n");
    if (SupportsNumberOfPagesQuery())
    {
      fprintf(stderr, "       -c           show all contained bitmaps of multipage images\n");
      fprintf(stderr, "                    (this option discards all user options except ext_bpp)\n");
    }
    fprintf(stderr, "       fn.ext{,opt} input filenames (with any format specific options)\n");
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

    fprintf(stderr, "        opt         bitmap format specific option to pass to bitmap reader\n");

    fprintf(stderr, "\n        In case the filename contains a comma or spaces and options\n");
    fprintf(stderr,   "        need to be added, the syntax \"\\\"fn.ext\\\"{,opt}\" must be used\n");
    fprintf(stderr,   "        to clearly separate the filename from the options.\n");

    exit(1);
}

/* ----------------------------- */

static gbm_boolean guess     = GBM_TRUE;
static gbm_boolean silent    = GBM_FALSE;
static gbm_boolean multipage = GBM_FALSE;

/* ----------------------------- */

static void show_error(const char *fn, const char *reason)
{
    if ( !silent )
        fprintf(stderr, "%s: %s - %s\n", progname, fn, reason);
}

/* ----------------------------- */

static gbm_boolean show_guess(const char *fn, const char *opt, int fd)
{
    int ft, rc;
    int imgcount = 0;
    long filelen, datalen;
    GBMFT gbmft;
    GBM gbm;

    if ( gbm_guess_filetype(fn, &ft) != GBM_ERR_OK )
    {
        gbm_io_close(fd);
        show_error(fn, "can't guess bitmap file format from extension");
        return GBM_FALSE;
    }

    gbm_query_filetype(ft, &gbmft);

    /* ignore all user options */
    if (multipage)
    {
        int i;
        gbm_boolean ext_bpp = GBM_FALSE;

        if (! GetNumberOfPages(fn, fd, ft, &imgcount))
        {
            char s[100+1];
            gbm_io_close(fd);
            sprintf(s, "can't read file header: %s", gbm_err(rc));
            show_error(fn, s);
            return GBM_FALSE;
        }

        ext_bpp = (strstr(opt, "ext_bpp") != NULL);

        filelen = gbm_io_lseek(fd, 0L, GBM_SEEK_END);
        datalen = 0;

        for (i = 0; i < imgcount; i++)
        {
            char opt_index[50+1];

            sprintf(opt_index, "%s index=%d", ext_bpp ? "ext_bpp" : "", i);

            if ( (rc = gbm_read_header(fn, fd, ft, &gbm, opt_index)) != GBM_ERR_OK )
            {
                char s[100+1];
                gbm_io_close(fd);
                sprintf(s, "can't read file header: %s", gbm_err(rc));
                show_error(fn, s);
                return GBM_FALSE;
            }

            datalen += (gbm.w*gbm.h*gbm.bpp+7)/8;
            printf("Index %3d: %4dx%-4d %2dbpp %5ldKb %4d%% %-10s %s\n",
                   i,
                   gbm.w, gbm.h, gbm.bpp,
                   (filelen+1023)/1024,
                   (filelen*100)/datalen,
                   gbmft.short_name,
                   fn);
        }
        printf("Compression ratio: %3ld%%\n", filelen*100/datalen);
    }
    else
    {
        if ( (rc = gbm_read_header(fn, fd, ft, &gbm, opt)) != GBM_ERR_OK )
        {
            char s[100+1];
            gbm_io_close(fd);
            sprintf(s, "can't read file header: %s", gbm_err(rc));
            show_error(fn, s);
            return GBM_FALSE;
        }

        filelen = gbm_io_lseek(fd, 0L, GBM_SEEK_END);
        datalen = (gbm.w*gbm.h*gbm.bpp+7)/8;
        printf("%4dx%-4d %2dbpp %5ldKb %4d%% %-10s %s\n",
               gbm.w, gbm.h, gbm.bpp,
               (filelen+1023)/1024,
               (filelen*100)/datalen,
               gbmft.short_name,
               fn);
    }

    return GBM_TRUE;
}

/* ----------------------------- */

static void show_noguess(const char *fn, const char *opt, int fd)
{
    int ft, n_ft;
    GBMFT gbmft;

    printf("%5ldKb %s\n",
           (gbm_io_lseek(fd, 0L, GBM_SEEK_END) + 1023) / 1024,
           fn);

    if ( gbm_guess_filetype(fn, &ft) == GBM_ERR_OK )
    {
        gbm_query_filetype(ft, &gbmft);
        printf("  file extension suggests bitmap format may be %-10s\n",
               gbmft.short_name);
    }

    gbm_query_n_filetypes(&n_ft);

    for ( ft = 0; ft < n_ft; ft++ )
    {
        GBM gbm;
        if ( gbm_read_header(fn, fd, ft, &gbm, opt) == GBM_ERR_OK )
        {
            gbm_query_filetype(ft, &gbmft);
            printf("  reading header suggests bitmap format may be %-10s - %4dx%-4d %2dbpp\n",
                   gbmft.short_name, gbm.w, gbm.h, gbm.bpp);
        }
    }
}

/* ----------------------------- */

static gbm_boolean show(const char *fn, const char *opt)
{
    gbm_boolean ret = GBM_TRUE;
    int     fd;
    struct  stat buf;

    if ( stat(fn, &buf) != -1 && (buf.st_mode & S_IFDIR) == S_IFDIR )
    /* Is a directory */
    {
        show_error(fn, "is a directory");
        return GBM_FALSE;
    }

    if ( (fd = gbm_io_open(fn, GBM_O_RDONLY)) == -1 )
    {
        show_error(fn, "can't open");
        return GBM_FALSE;
    }

    if ( guess )
    {
        ret = show_guess(fn, opt, fd);
    }
    else
    {
        show_noguess(fn, opt, fd);
    }

    gbm_io_close(fd);
    return ret;
}

/* ----------------------------- */

int main(int argc, char *argv[])
{
    int i, x;
    gbm_boolean oneParsed = GBM_FALSE;

    /* process command line options */
    if ( argc == 1 )
    {
       usage();
    }

    for ( i = 1; i < argc; i++ )
    {
       if ( argv[i][0] != '-' )
       {
           break;
       }
       else if ( argv[i][1] == '-' )
       {
           ++i; break;
       }

       switch ( argv[i][1] )
       {
          case 'g': guess     = GBM_FALSE; break;
          case 's': silent    = GBM_TRUE;  break;
          case 'c': multipage = GBM_TRUE;  break;
          default : usage();           break;
       }
    }

    if (multipage && (! SupportsNumberOfPagesQuery()))
    {
        usage();
    }

    for (x = i ; x < argc; x++ )
    {
       /* check for empty filenames */
       if (strcmp(argv[x], "\"\"") == 0)
       {
          continue;
       }
       oneParsed = GBM_TRUE;
    }
    if (! oneParsed)
    {
      usage();
    }

    for ( ; i < argc; i++ )
    {
       GBMTOOL_FILEARG   gbmfilearg;
       GBMTOOL_FILE    * currentFile = NULL;
       char              extended_options[GBMTOOL_FILENAME_MAX + 20];

       /* handle a filename argument */

       /* skip empty filenames */
       if (strcmp(argv[i], "\"\"") == 0)
       {
          continue;
       }

       /* Split filename and file options. */
       gbmfilearg.argin = argv[i];

       /* This also expands possible regular expression based filename templates. */
     #ifdef FILENAME_EXPANSION_MODE
       if (gbmtool_parse_argument(&gbmfilearg, GBM_TRUE) != GBM_ERR_OK)
     #else
       if (gbmtool_parse_argument(&gbmfilearg, GBM_FALSE) != GBM_ERR_OK)
     #endif
       {
          show_error(gbmfilearg.argin, "cannot parse filename");
          return 1;
       }

       currentFile = gbmfilearg.files;

       /* Always report full colour depth even though the user did not request it. */
       strcpy(extended_options, gbmfilearg.options);
       strcat(extended_options, (strlen(gbmfilearg.options) < 1) ? "ext_bpp" : ",ext_bpp");

       while(currentFile != NULL)
       {
         if (! show(currentFile->filename, extended_options))
         {
           gbmtool_free_argument(&gbmfilearg);
           return 2;
         }
         currentFile = currentFile->next;
       }

       /* free dynamically allocated filename */
       gbmtool_free_argument(&gbmfilearg);

       oneParsed = GBM_TRUE;
    }

    return 0;
}

/********************************/

/* Depending on GBM.DLL version the number of pages can be retrieved (versions > 1.35)
 * or the functionality does not yet exist.
 *
 * Dynamically link the specific function to support also older versions of GBM.DLL.
 */
gbm_boolean SupportsNumberOfPagesQuery(void)
{
#if defined(__OS2__) || defined(OS2)

   HMODULE hmod;
   PFN     functionAddr = NULL;
   APIRET  rc = 0;

   /* check version first */
   if (gbm_version() < 135)
   {
      return GBM_FALSE;
   }

   /* now dynamically link GBM.DLL */
   rc = DosLoadModule("", 0, "GBM", &hmod);
   if (rc)
   {
      return GBM_FALSE;
   }

   /* lookup gbm_read_imgcount() */
   rc = DosQueryProcAddr(hmod, 0L, "gbm_read_imgcount", &functionAddr);

   DosFreeModule(hmod);

   return rc ? GBM_FALSE : GBM_TRUE;
#else
   /* On all other platforms we assume so far that the correct lib is there. */
   return GBM_TRUE;
#endif
}

/********************************/

/* Depending on GBM.DLL version the number of pages can be retrieved (versions > 1.35)
 * or the functionality does not yet exist.
 *
 * Dynamically link the specific function to support also older versions of GBM.DLL.
 */
gbm_boolean GetNumberOfPages(const char * fileName, const int fd, const int ft, int * numPages)
{
#if defined(__OS2__) || defined(OS2)

   HMODULE hmod;
   PFN     functionAddr = NULL;
   APIRET  rc = 0;

   /* check version first */
   if (gbm_version() < 135)
   {
      return GBM_FALSE;
   }

   /* now dynamically link GBM.DLL */
   rc = DosLoadModule("", 0, "GBM", &hmod);
   if (rc)
   {
      return GBM_FALSE;
   }

   /* lookup gbm_read_imgcount() */
   rc = DosQueryProcAddr(hmod, 0L, "gbm_read_imgcount", &functionAddr);
   if (rc)
   {
      DosFreeModule(hmod);
      return GBM_FALSE;
   }

   /* call gbm_read_imgcount(const char *fn, int fd, int ft, int *pimgcnt) */
   if (functionAddr(fileName, fd, ft, numPages) != GBM_ERR_OK)
   {
      DosFreeModule(hmod);
      return GBM_FALSE;
   }

   DosFreeModule(hmod);

   return GBM_TRUE;
#else
   /* On all other platforms we assume so far that the correct lib is there. */
   return (gbm_read_imgcount(fileName, fd, ft, numPages) != GBM_ERR_OK) ? GBM_FALSE : GBM_TRUE;
#endif
}

