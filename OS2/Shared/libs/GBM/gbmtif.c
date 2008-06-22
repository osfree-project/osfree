/*

gbmtif.c - Microsoft/Aldus Tagged Image File Format support

Credit for writing this module must go to Heiko Nitzsche.

This file is just as public domain as the rest of GBM.

This code is a mainly a wrapper around the official libtiff library and
thus supports most features of the actual version (3.8.2 or higher).
Not all color depths and color spaces are supported by this wrapper.

Supported formats and options:
------------------------------
TIFF : Microsoft/Aldus Tagged Image File Format support : .TIF .TIFF

Standard formats (backward compatible):
  Reads  1 bpp black/white and colour palette/gray level files.
  Reads  2 bpp colour palette/gray level files and presents them as  4 bpp.
  Reads  4 bpp colour palette/gray level files and presents them as  4 bpp.
  Reads  8 bpp colour palette/gray level files and presents them as  8 bpp.
  Reads 16 bpp colour palette/gray level files and presents them as 24 bpp.
  Reads 24 bpp colour files and presents them as 24 bpp.
  Reads 32 bpp colour files (RGB + alpha channel) and presents them as 24 bpp.
  Reads 48 bpp colour files and presents them as 24 bpp.
  Reads 64 bpp colour files (RGB + alpha channel) and presents them as 24 bpp.
  Reads CMYK ( 8 bit per channel) files and presents them as 24 bpp RGB (strip encoded only).
  Reads CMYK (16 bit per channel) files and presents them as 24 bpp RGB (strip encoded only).

Extended formats (not backward compatible, import option ext_bpp required):
  Reads 16 bpp colour palette/gray level files and presents them as 48 bpp (strip encoded only).
  Reads 32 bpp colour files (RGB + alpha channel) and presents them as 32 bpp.
  Reads 48 bpp colour files and presents them as 48 bpp (strip encoded only).
  Reads 64 bpp colour files (RGB + alpha channel) and presents them as 64 bpp (strip encoded only).
  Reads CMYK (16 bit per channel) files and presents them as 48 bpp RGB (strip encoded only).

  Writes 1, 4, 8, 24, 32 (24 bpp + alpha), 48, 64 (48 bpp + alpha) bpp RGB files.

  The depth of the alpha channel must in all cases match with the depth of all RGB channels.

  Supports the following compression schemes:
  - uncompressed
  - LZW
  - PackBits
  - ThunderScan
  - NeXT
  - JPEG
  - CCITT
  - Deflate
  - Adobe Deflate


  Input:
  ------

  Handling of alpha channel:
    If the bitmap contains an associated alpha channel (pre-multiplied alpha), the alpha
    channel values are not separately returned. In this case a provided background color
    is ignored.
    If the bitmap contains an unaccociated alpha channel (not pre-multiplied alpha), the
    alpha channel is returned separately unless the client requests that it has to be
    calculated against the provided background color.
    In the case the ext_bpp option is not specified and the bitmap contains an unassociated
    alpha channel, the alpha channel is only calculated against the background color if
    a background color has been specified by the client. Otherwise the alpha channel data
    is ignored.

  Can specify image within TIFF file with many images
    Input option: index=# (default: 0)

  Can specify background color as RGB for alpha channel mixing
  (only interpreted for bitmaps with unassociated alpha channel)
    Input option: back_rgb=#_#_# (red_green_blue, 0..65535)

  Can specify that non-standard GBM color depths are exported (also with alpha channel)
    Input option: ext_bpp (default: bpp is downsampled to 24 bpp)


  Output:
  -------

  Handling of alpha channel:
    GBM allows writing of unassociated alpha channel information for 32 and 64 bpp.
    It can also associate separately provided alpha channel information by
    pre-multiplying a specified background color against the RGB channels (option back_rgb).
    If the alpha channel has already been associated (pre-multiplied), color depths
    of 24 and 48 bpp should be used when writing RGB bitmaps.

  Can specify background color as RGB for associating alpha channel
  (the alpha channel is preserved in the bitmap file)
    Output option: back_rgb=#_#_# (red_green_blue, 0..65535)

  Can write 1bpp as non-Baseline file preserving fg/bg cols
  (This can be done to enforce writing a palette bitmap.
   Usually non-B/W bitmap is automatically detected and a palette image is written
   but the option has been preserved for backward compatibility.)
    Output option: pal1bpp

  Can ask for specific strip size
    Output option: strip_size=# (4,8,16,32,64 kBytes, default=16)

  Can ask for LZW compression.
    Output option: lzw

  Can ask for LZW compression with horizontal prediction
    Output option: lzw_pred_hor

  Can ask for PackBits compression.
    Output option: packbits

  Can ask for Deflate compression (Adobe compatible Deflate).
    Output option: deflate

  Can specify Deflate compression level
    Output option: deflate_comp=# (default=6, 1..9, no compression to max)

  Can ask for JPEG compression (only 24 bpp).
    Output option: jpeg

  Can specify JPEG quality level
    Output option: jpeg_quality=# (default=75, 0..100)

  Can ask for CCITT modified Huffman RLE encoding (only 1 bpp).
    Output option: ccittrle

  Can ask for CCITT Group 3 fax encoding (only 1 bpp).
    Output option: ccittfax3

  Can ask for CCITT Group 4 fax encoding (only 1 bpp).
    Output option: ccittfax4

  Can specify informational tags to append on output.
    Output options: artist=,software=,make=,model=,host=,documentname=,pagename=,imagedescription=


History:
--------
28-Sep-2005: First official version (Libtiff 3.7.3, Zlib 1.2.3, IJG JPEG 6b)

12-Dec-2005: Update to Libtiff 3.7.4
             Fix some compile warnings
             Fix decoding of separated RGB tiffs
             Fix decoding of non-separated CMYK tiffs (8/16 bits per channel)

19-Feb-2006: Update to Libtiff 3.8.0 skipped, 3.7.4 still has to be used
             (3.8.0 crashes on some files in color rendering with OpenWatcom 1.4, but works with IBM VAC++)
             Add function to query number of images

22-Feb-2006: Move format description strings to gbmdesc.h

24-Mar-2006: Update to Libtiff 3.8.2
             Fix JPEG decoding via RGBA reader for YCbCr JPEGs
             -> There seems to be an issue with the libtiff RGBA reader as
                we don't get RGBA but BGRA for PHOTOMETRIC_YCBCR.
                (has been found also in version 3.7.4)

06-Sep-2006: Undo the fix for JPEG decoding via RGBA reader for YCbCr JPEGs.
             This was caused to modified RGB indexing hard coded into IJG lib.
             The order is now again back at RGB rather than BGR (original order).

26-Sep-2006: - Report at least 1 page if libtiff reports 0. This is backward compatible
               behaviour. The directory tag might be missing in the file and length
               calculation fails as well due to compression scheme.
             - CMYK images (8/16bpp) can now be read (single and multiplane TIFFs)
             - Report more specific error message for data decoding problem

******************************************************************************/

#ifdef ENABLE_TIF

#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmhelp.h"
#include "gbmdesc.h"
#include "gbmmap.h"
#include "gbmmem.h"

#include "tiff.h"
#include "tiffio.h"

/* ----------------------------------------------------------- */

#define GBM_ERR_TIF               ((GBM_ERR) 800)
#define GBM_ERR_TIF_BPP           ((GBM_ERR) 801)
#define GBM_ERR_TIF_HEADER        ((GBM_ERR) 802)
#define GBM_ERR_TIF_DATA          ((GBM_ERR) 803)
#define GBM_TIF_NUM_COLORS        0x100    /*   256 entries in color table */
#define GBM_TIF_NUM_COLORS_16BPP  0x10000  /* 65536 entries in color table */

/* ----------------------------------------------------------- */

typedef struct
{
    GBM_ERR rc;
    const char *error_message;
} TIF_GBMERR_MSG;

static TIF_GBMERR_MSG tif_errmsg[] =
{
    { GBM_ERR_TIF_BPP      , "unsupported bits per pixel" },
    { GBM_ERR_TIF_HEADER   , "bad header"                 },
    { GBM_ERR_TIF_DATA     , "unsupported data encoding"  },
    { -1                   , NULL                         }
};

static GBMFT tif_gbmft =
{
    GBM_FMT_DESC_SHORT_TIF,
    GBM_FMT_DESC_LONG_TIF,
    GBM_FMT_DESC_EXT_TIF,
    GBM_FT_R1 | GBM_FT_R4 | GBM_FT_R8 | GBM_FT_R24 | GBM_FT_R32 | GBM_FT_R48 | GBM_FT_R64 |
    GBM_FT_W1 | GBM_FT_W4 | GBM_FT_W8 | GBM_FT_W24 | GBM_FT_W32 | GBM_FT_W48 | GBM_FT_W64
};


typedef struct
{
    TIFFErrorHandler  orgErrorHandler;   /* stores the original error handler */
    TIFFErrorHandler  orgWarningHandler; /* stores the original warning handler */
    TIFF             *tif_p;             /* Pointer to TIFF file structure */
} TIF_PRIV_WRITE;


typedef struct
{
    TIFFErrorHandler orgErrorHandler;    /* stores the original error handler */
    TIFFErrorHandler orgWarningHandler;  /* stores the original warning handler */
    TIFF             *tif_p;             /* Pointer to TIFF file structure */

    BOOLEAN      upsamplePaletteToRGB;     /* TRUE if palette file has to be upsampled to RGB */
    BOOLEAN      upsamplePaletteToPalette; /* TRUE if palette file has to be upsampled to other palette */
    BOOLEAN      unassociatedAlpha;        /* TRUE if an unassociated alpha channel exists */
    uint16       photometric;              /* Photographic representation    */
    GBMRGB_16BPP backrgb;                  /* background RGB color for Alpha channel mixing */

    /* This entry will store the filename provided during first header read.
     * It will be kept for the case the header has to be reread.
     */
    char read_filename[257];        /* 256 for filename + 1 for \0 */

    /* This entry will store the options provided during first header read.
     * It will keep the options for the case the header has to be reread.
     */
    char read_options[PRIV_SIZE - 257 /* read_filename */
                                - (2*sizeof(TIFFErrorHandler))
                                - (3*sizeof(BOOLEAN))
                                - sizeof(TIFF *)
                                - sizeof(uint16)
                                - sizeof(GBMRGB_16BPP)
                                - 20 /* space for structure element padding */ ];
} TIF_PRIV_READ;

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* Replace standard libpng I/O functions by our own that simply call the
   GBM internal I/O functions.
*/
static tsize_t tif_gbm_read_data(thandle_t fd, tdata_t data, tsize_t length)
{
   return ((tsize_t) gbm_file_read((int) fd, data, (int) length));
}

static tsize_t tif_gbm_write_data(thandle_t fd, tdata_t data, tsize_t length)
{
   return ((tsize_t) gbm_file_write((int) fd, data, (int) length));
}

static toff_t tif_gbm_seek(thandle_t fd, toff_t off, int whence)
{
   return ((toff_t) gbm_file_lseek((int) fd, (off_t) off, whence));
}

static int tif_gbm_close(thandle_t fd)
{
   fd = fd; /* suppress compiler warning */

   /* The file is owned by GBM. So don't close it here. */
   return 0;
}

static toff_t tif_gbm_size(thandle_t fd)
{
   const long fsize = gbm_file_lseek((int) fd, 0, GBM_SEEK_END);
   return (toff_t) (fsize < 0 ? 0 : fsize);
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* Skip libtiff warning messages. */
static void tif_gbm_warning_handler(const char* module, const char* fmt, va_list ap)
{
#ifdef DEBUG
   if (module != NULL)
   {
      fprintf(stderr, "%s: ", module);
      fprintf(stderr, "Warning, ");
      vfprintf(stderr, fmt, ap);
      fprintf(stderr, ".\n");
   }
#else
   /* suppress compiler warnings */
   module = module;
   fmt    = fmt;
   ap     = ap;
#endif
}

/* Skip libtiff error messages. */
static void tif_gbm_error_handler(const char* module, const char* fmt, va_list ap)
{
#ifdef DEBUG
   if (module != NULL)
   {
      fprintf(stderr, "%s: ", module);
      fprintf(stderr, "Error, ");
      vfprintf(stderr, fmt, ap);
      fprintf(stderr, ".\n");
   }
#else
   /* suppress compiler warnings */
   module = module;
   fmt    = fmt;
   ap     = ap;
#endif
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* Initialize private structures for reading
   Returns TRUE on success, else FALSE
*/
static BOOLEAN tif_read_init(TIF_PRIV_READ *tif_priv, const char *fn, int fd)
{
    tif_priv->tif_p = NULL;

    /* register local message handlers */
    tif_priv->orgErrorHandler   = TIFFSetErrorHandler  (tif_gbm_error_handler);
    tif_priv->orgWarningHandler = TIFFSetWarningHandler(tif_gbm_warning_handler);

    /* Open the TIFF file */
    tif_priv->tif_p = TIFFClientOpen(fn, "r",
                                     (thandle_t) fd,
                                     tif_gbm_read_data, tif_gbm_write_data,
                                     tif_gbm_seek, tif_gbm_close,
                                     tif_gbm_size,
                                     NULL, NULL);
    if (tif_priv->tif_p == NULL)
    {
       return FALSE;
    }

    return TRUE;
}

/* Cleanup private structs for reading. */
static void tif_read_deinit(TIF_PRIV_READ *tif_priv)
{
    TIFFClose(tif_priv->tif_p);

    tif_priv->tif_p = NULL;

    /* unregister local message handlers */
    TIFFSetErrorHandler  (tif_priv->orgErrorHandler);
    TIFFSetWarningHandler(tif_priv->orgWarningHandler);
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* Initialize private structures for writing
   Returns TRUE on success, else FALSE
*/
static BOOLEAN tif_write_init(TIF_PRIV_WRITE *tif_priv, const char *fn, int fd)
{
    tif_priv->tif_p = NULL;

    /* register local message handlers */
    tif_priv->orgErrorHandler   = TIFFSetErrorHandler  (tif_gbm_error_handler);
    tif_priv->orgWarningHandler = TIFFSetWarningHandler(tif_gbm_warning_handler);

    /* Open file for writing (overriding and existing file) */
    tif_priv->tif_p = TIFFClientOpen(fn, "w",
                                     (thandle_t) fd,
                                     tif_gbm_read_data, tif_gbm_write_data,
                                     tif_gbm_seek, tif_gbm_close,
                                     tif_gbm_size,
                                     NULL, NULL);
    if (tif_priv->tif_p == NULL)
    {
       return FALSE;
    }

    return TRUE;
}

/* Cleanup structs for writing. */
static void tif_write_deinit(TIF_PRIV_WRITE *tif_priv)
{
    TIFFClose(tif_priv->tif_p);

    tif_priv->tif_p = NULL;

    /* unregister local message handlers */
    TIFFSetErrorHandler  (tif_priv->orgErrorHandler);
    TIFFSetWarningHandler(tif_priv->orgWarningHandler);
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* tif_qft - Query format informations */
GBM_ERR tif_qft(GBMFT *gbmft)
{
    *gbmft = tif_gbmft;
    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */

/* Read number of directories in the TIFF file. */
GBM_ERR tif_rimgcnt(const char *fn, int fd, int *pimgcnt)
{
    GBM gbm;
    TIF_PRIV_READ *tif_priv = (TIF_PRIV_READ *) gbm.priv;

    fn=fn; /* suppress compiler warning */

    /* Open the TIFF file */
    if (! tif_read_init(tif_priv, tif_priv->read_filename, fd))
    {
       return GBM_ERR_TIF_HEADER;
    }

    *pimgcnt = (int) TIFFNumberOfDirectories(tif_priv->tif_p);
    if (*pimgcnt == 0)
    {
        /* tag might be missing, report at least 1 page */
        *pimgcnt = 1;
    }

    tif_read_deinit(tif_priv);

    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */

/* internal_tif_rhdr - Read file header to init GBM struct */
static GBM_ERR internal_tif_rhdr(int fd, GBM *gbm)
{
    TIF_PRIV_READ *tif_priv = (TIF_PRIV_READ *) gbm->priv;
    TIFF          *tif_p    = NULL;

    uint32 width;
    uint32 height;
    uint16 bitsPerSample;
    uint16 samplesPerPixel;
    uint16 extraSamples = EXTRASAMPLE_UNSPECIFIED;
    uint16 * extraSamplesInfo;
    uint16 inkset       = INKSET_MULTIINK;
    uint16 config;

    BOOLEAN back_color_defined = FALSE;
    BOOLEAN use_native_bpp     = FALSE;
    BOOLEAN is_cmyk            = FALSE;

    const char *s = NULL;

    /* As we read the header multiple times, set file pointer to start. */
    if (gbm_file_lseek(fd, 0L, GBM_SEEK_SET) != 0)
    {
       return GBM_ERR_READ;
    }

    /* Open the TIFF file */
    if (! tif_read_init(tif_priv, tif_priv->read_filename, fd))
    {
       return GBM_ERR_TIF_HEADER;
    }

    tif_p = tif_priv->tif_p;

    /* select the request directory */
    if ((s = gbm_find_word_prefix(tif_priv->read_options, "index=")) != NULL)
    {
        const tdir_t directoryCurrent = TIFFCurrentDirectory(tif_p);
              tdir_t directoryLength  = TIFFNumberOfDirectories(tif_p);

        int image_index = 0;
        if (sscanf(s + 6, "%d", &image_index) != 1)
        {
           tif_read_deinit(tif_priv);
           return GBM_ERR_BAD_OPTION;
        }

        if (directoryLength == 0)
        {
            /* tag might be missing, assume at least 1 page */
            directoryLength = 1;
        }

        /* check against available directories */
        if ((image_index < 0) || (image_index >= directoryLength))
        {
           tif_read_deinit(tif_priv);
           return GBM_ERR_BAD_OPTION;
        }

        /* activate directory if different to current */
        if (directoryCurrent != image_index)
        {
           if (! TIFFSetDirectory(tif_p, (tdir_t) image_index))
           {
              tif_read_deinit(tif_priv);
              return GBM_ERR_READ;
           }
        }
    }

    /* Get all necessary info for the GBM struct */
    if ((! TIFFGetField(tif_p, TIFFTAG_IMAGEWIDTH     , &width))           ||
        (! TIFFGetField(tif_p, TIFFTAG_IMAGELENGTH    , &height))          ||
        (! TIFFGetField(tif_p, TIFFTAG_BITSPERSAMPLE  , &bitsPerSample))   ||
        (! TIFFGetField(tif_p, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel)) ||
        (! TIFFGetField(tif_p, TIFFTAG_PHOTOMETRIC    , &(tif_priv->photometric))))
    {
       tif_read_deinit(tif_priv);
       return GBM_ERR_TIF_HEADER;
    }

    /* check if extended color depths are requested */
    use_native_bpp = (gbm_find_word(tif_priv->read_options, "ext_bpp") != NULL)
                     ? TRUE : FALSE;

    /* check if a background color was specified */
    back_color_defined = (gbm_find_word_prefix(tif_priv->read_options, "back_rgb=") != NULL)
                         ? TRUE : FALSE;

    gbm->w = width;
    gbm->h = height;

    /* convert bit_depth from channel bit depth to overall bit depth */
    gbm->bpp = bitsPerSample * samplesPerPixel;

    /* check if an unassociated alpha channel exists */
    TIFFGetFieldDefaulted(tif_p, TIFFTAG_EXTRASAMPLES, &extraSamples, &extraSamplesInfo);
    switch(extraSamples)
    {
       case 0:
          tif_priv->unassociatedAlpha = FALSE;
          break;

       case 1:
          if ((*extraSamplesInfo != EXTRASAMPLE_UNASSALPHA) && (*extraSamplesInfo != EXTRASAMPLE_ASSOCALPHA))
          {
             tif_read_deinit(tif_priv);
             return GBM_ERR_TIF_HEADER;
          }
          tif_priv->unassociatedAlpha = (*extraSamplesInfo == EXTRASAMPLE_UNASSALPHA) ? TRUE : FALSE;
          break;

       default:
          tif_read_deinit(tif_priv);
          return GBM_ERR_TIF_HEADER;
    }

    tif_priv->upsamplePaletteToRGB     = FALSE;
    tif_priv->upsamplePaletteToPalette = FALSE;

    /* check if data is splitted into multiple planes */
    if (! TIFFGetField(tif_p, TIFFTAG_PLANARCONFIG, &config))
    {
       tif_read_deinit(tif_priv);
       return GBM_ERR_READ;
    }

    /* check for palette availability based on the color type */
    switch(tif_priv->photometric)
    {
      case PHOTOMETRIC_MINISWHITE:
      case PHOTOMETRIC_MINISBLACK:
         if ((tif_priv->unassociatedAlpha) || (config != PLANARCONFIG_CONTIG))
         {
            tif_read_deinit(tif_priv);
            return GBM_ERR_TIF_BPP;
         }
         switch(bitsPerSample)
         {
            case 1:
            case 4:
            case 8:
               break;

            case 2:
               /* the import of 2 bpp files is already supported (use_native_bpp) */
               /* but we always upsample to 4 bpp to simplify external usage      */
               tif_priv->upsamplePaletteToPalette = TRUE;
               bitsPerSample = 4;
               gbm->bpp      = 4;
               break;

            case 16:
               /* upsample to 24/48 bpp */
               tif_priv->upsamplePaletteToRGB = TRUE;
               bitsPerSample   = use_native_bpp ? 16 : 8;
               samplesPerPixel = 3;
               gbm->bpp        = samplesPerPixel * bitsPerSample;
               break;

            default:
               tif_read_deinit(tif_priv);
               return GBM_ERR_TIF_BPP;
         }
         break;

      /* ------------------------------------------------------- */

      case PHOTOMETRIC_PALETTE:
         if ((tif_priv->unassociatedAlpha) || (config != PLANARCONFIG_CONTIG))
         {
            tif_read_deinit(tif_priv);
            return GBM_ERR_TIF_BPP;
         }
         switch(bitsPerSample)
         {
            case 1:
            case 4:
            case 8:
               break;

            case 2:
               /* the import of 2 bpp files is already supported (use_native_bpp) */
               /*  but we always upsample to 4 bpp to simplify external usage     */
               tif_priv->upsamplePaletteToPalette = TRUE;
               bitsPerSample = 4;
               gbm->bpp      = 4;
               break;

            case 16:
               /* upsample to 24/48 bpp */
               tif_priv->upsamplePaletteToRGB = TRUE;
               bitsPerSample   = use_native_bpp ? 16 : 8;
               samplesPerPixel = 3;
               gbm->bpp        = samplesPerPixel * bitsPerSample;
               break;

            default:
               tif_read_deinit(tif_priv);
               return GBM_ERR_TIF_BPP;
         }
         break;

      /* ------------------------------------------------------- */

      case PHOTOMETRIC_RGB:
         switch(bitsPerSample)
         {
            case 8: /* this is 24 bpp or 32 bpp (with alpha channel) */
               if (! use_native_bpp)
               {
                  /* downsample to 24 bpp */
                  samplesPerPixel = 3;
                  gbm->bpp        = 24;

                  /* if no background color exists, the alpha channel is ignored */
                  if (! back_color_defined)
                  {
                     /* don't calculate alpha against background color */
                     tif_priv->unassociatedAlpha = FALSE;
                  }
               }
               else
               {
                  /* an alpha channel exists */
                  if (samplesPerPixel > 3)
                  {
                     if (tif_priv->unassociatedAlpha)
                     {
                        /* if a background color exists, the alpha channel gets associated
                         * and will not be returned separately
                         */
                        if (back_color_defined)
                        {
                           /* downsample to 24 bpp */
                           samplesPerPixel = 3;
                           gbm->bpp        = 24;
                        }
                        else
                        {
                           /* don't calculate alpha against background color */
                           tif_priv->unassociatedAlpha = FALSE;
                        }
                     }
                     else
                     {
                        /* downsample to 24 bpp */
                        samplesPerPixel = 3;
                        gbm->bpp        = 24;
                     }
                  }
               }
               break;

            case 16: /* this is 48 bpp or 64 bpp (with alpha channel) */
               if (! use_native_bpp)
               {
                  /* downsample to 24 bpp */
                  bitsPerSample   = 8;
                  samplesPerPixel = 3;
                  gbm->bpp        = 24;

                  /* if no background color exists, the alpha channel is ignored */
                  if (! back_color_defined)
                  {
                     /* don't calculate alpha against background color */
                     tif_priv->unassociatedAlpha = FALSE;
                  }
               }
               else
               {
                  /* an alpha channel exists */
                  if (samplesPerPixel > 3)
                  {
                     if (tif_priv->unassociatedAlpha)
                     {
                        /* if a background color exists, the alpha channel gets associated
                         * and will not be returned separately
                         */
                        if (back_color_defined)
                        {
                           /* downsample to 48 bpp */
                           samplesPerPixel = 3;
                           gbm->bpp        = 48;
                        }
                        else
                        {
                           /* don't calculate alpha against background color */
                           tif_priv->unassociatedAlpha = FALSE;
                        }
                     }
                     else
                     {
                        /* downsample to 48 bpp */
                        samplesPerPixel = 3;
                        gbm->bpp        = 48;
                     }
                  }
               }
               break;

            default:
               tif_read_deinit(tif_priv);
               return GBM_ERR_TIF_BPP;
         }
         break;

      /* ------------------------------------------------------- */

      case PHOTOMETRIC_SEPARATED: /* CMYK (4 samples), RGB(A) */
         inkset = INKSET_CMYK; /* CMYK is default */
         TIFFGetField(tif_p, TIFFTAG_INKSET, &inkset);
         is_cmyk = (inkset == INKSET_CMYK) ? TRUE : FALSE;

         switch(bitsPerSample)
         {
            case 8: /* this is 24 bpp (RGB, CMY) or 32 bpp (RGBA, CMYK) */
               if (is_cmyk)
               {
                  /* no support for CMY */
                  if (samplesPerPixel != 4)
                  {
                     tif_read_deinit(tif_priv);
                     return GBM_ERR_TIF_BPP;
                  }
                  /* this will get 24 bit/pixel as we cannot externally differentiate
                   * between 32 bpp color/CMYK
                   */
                  samplesPerPixel = 3;
                  gbm->bpp        = 24;
               }
               else
               {
                  if (! use_native_bpp)
                  {
                     /* downsample to 24 bpp */
                     samplesPerPixel = 3;
                     gbm->bpp        = 24;

                     /* if no background color exists, the alpha channel is ignored */
                     if (! back_color_defined)
                     {
                        /* don't calculate alpha against background color */
                        tif_priv->unassociatedAlpha = FALSE;
                     }
                  }
                  else
                  {
                     /* an alpha channel exists */
                     if (samplesPerPixel > 3)
                     {
                        if (tif_priv->unassociatedAlpha)
                        {
                           /* if a background color exists, the alpha channel gets associated
                            * and will not be returned separately
                            */
                           if (back_color_defined)
                           {
                              /* downsample to 24 bpp */
                              samplesPerPixel = 3;
                              gbm->bpp        = 24;
                           }
                           else
                           {
                              /* don't calculate alpha against background color */
                              tif_priv->unassociatedAlpha = FALSE;
                           }
                        }
                        else
                        {
                           /* downsample to 24 bpp */
                           samplesPerPixel = 3;
                           gbm->bpp        = 24;
                        }
                     }
                  }
               }
               break;

            case 16: /* this is 48 bpp (RGB, CMY) or 64 bpp (RGBA, CMYK) */
               if (is_cmyk)
               {
                  /* no support for CMY */
                  if (samplesPerPixel != 4)
                  {
                     tif_read_deinit(tif_priv);
                     return GBM_ERR_TIF_BPP;
                  }
                  /* this will get 24/48 bit/pixel as we cannot externally differentiate
                   * between 64 bpp color/CMYK
                   */
                  samplesPerPixel = 3;
                  gbm->bpp        = use_native_bpp ? 48 : 24;
               }
               else
               {
                  if (! use_native_bpp)
                  {
                     /* downsample to 24 bpp */
                     samplesPerPixel = 3;
                     gbm->bpp        = 24;

                     /* if no background color exists, the alpha channel is ignored */
                     if (! back_color_defined)
                     {
                        /* don't calculate alpha against background color */
                        tif_priv->unassociatedAlpha = FALSE;
                     }
                  }
                  else
                  {
                     /* an alpha channel exists */
                     if (samplesPerPixel > 3)
                     {
                        if (tif_priv->unassociatedAlpha)
                        {
                           /* if a background color exists, the alpha channel gets associated
                            * and will not be returned separately
                            */
                           if (back_color_defined)
                           {
                              /* downsample to 48 bpp */
                              samplesPerPixel = 3;
                              gbm->bpp        = 48;
                           }
                           else
                           {
                              /* don't calculate alpha against background color */
                              tif_priv->unassociatedAlpha = FALSE;
                           }
                        }
                        else
                        {
                           /* downsample to 48 bpp */
                           samplesPerPixel = 3;
                           gbm->bpp        = 48;
                        }
                     }
                  }
               }
               break;

            default:
               tif_read_deinit(tif_priv);
               return GBM_ERR_TIF_BPP;
         }
         break;

      /* ------------------------------------------------------- */

      default: /* try to load everything else via the RGBA reader (24 bit only) */
         /* PHOTOMETRIC_YCBCR  */
         /* PHOTOMETRIC_CIELAB */
         /* PHOTOMETRIC_ICCLAB */
         /* PHOTOMETRIC_ITULAB */
         /* ...                */
         switch(bitsPerSample)
         {
            case 8: /* this is 24 bit/pixel */
               samplesPerPixel = 3;
               gbm->bpp        = 24;
               break;

            case 16: /* this is 48 bit/pixel */
               /* export as 24 bit/pixel */
               bitsPerSample   = 8;
               samplesPerPixel = 3;
               gbm->bpp        = 24;
               break;

            default:
               tif_read_deinit(tif_priv);
               return GBM_ERR_TIF_BPP;
         }
         break;
    }


    /* tile based organization ? (for now we only support this via RGBA-Reader (24 Bit) */
    if (TIFFIsTiled(tif_p))
    {
       if (is_cmyk)
       {
          /* the RGBA reader is RGB only ! */
          tif_read_deinit(tif_priv);
          return GBM_ERR_TIF_BPP;
       }
       /* export as 24 bit/pixel */
       bitsPerSample   = 8;
       samplesPerPixel = 3;
       gbm->bpp        = 24;
    }

    /* finally check against supporteded GBM color depths */
    switch(gbm->bpp)
    {
       case  1:
       case  4:
       case  8:
       case 24:
          break;

       case 32:
       case 48:
       case 64:
          if (! use_native_bpp)
          {
             tif_read_deinit(tif_priv);
             return GBM_ERR_TIF_BPP;
          }
          break;

       default:
          tif_read_deinit(tif_priv);
          return GBM_ERR_TIF_BPP;
    }

    /* set default background color to black */
    tif_priv->backrgb.r = 0;
    tif_priv->backrgb.g = 0;
    tif_priv->backrgb.b = 0;

    /* parse RGB value for background mixing with alpha channel */
    if ((s = gbm_find_word_prefix(tif_priv->read_options, "back_rgb=")) != NULL)
    {
        int image_background_red   = 0;
        int image_background_green = 0;
        int image_background_blue  = 0;

        if (sscanf(s + 9, "%d_%d_%d", &image_background_red,
                                      &image_background_green,
                                      &image_background_blue) != 3)
        {
           tif_read_deinit(tif_priv);
           return GBM_ERR_BAD_OPTION;
        }

        if (((image_background_red   < 0) || (image_background_red   > 0xffff)) ||
            ((image_background_green < 0) || (image_background_green > 0xffff)) ||
            ((image_background_blue  < 0) || (image_background_blue  > 0xffff)))
        {
           tif_read_deinit(tif_priv);
           return GBM_ERR_BAD_OPTION;
        }

        tif_priv->backrgb.r = image_background_red;
        tif_priv->backrgb.g = image_background_green;
        tif_priv->backrgb.b = image_background_blue;
    }

    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */

/* tif_rhdr - Read file header to init GBM struct */
GBM_ERR tif_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
{
    TIF_PRIV_READ *tif_priv = (TIF_PRIV_READ *) gbm->priv;
    GBM_ERR        rc;

    /* init filename buffer */
    memset(tif_priv->read_filename, 0, sizeof(tif_priv->read_filename));

    /* init options buffer */
    memset(tif_priv->read_options, 0, sizeof(tif_priv->read_options));

    /* copy filename */
    if (strlen(fn) >= sizeof(tif_priv->read_filename))
    {
       return GBM_ERR_BAD_OPTION;
    }
    strcpy(tif_priv->read_filename, fn);

    /* copy possible options */
    if (strlen(opt) >= sizeof(tif_priv->read_options))
    {
       return GBM_ERR_BAD_OPTION;
    }
    strcpy(tif_priv->read_options, opt);

    rc = internal_tif_rhdr(fd, gbm);
    if (rc != GBM_ERR_OK)
    {
       return rc;
    }

    /* We cannot expect the client calls rhdr,rpal,rdata in order.
     * The client might just read the header and stop then.
     *
     * There is no other choice than freeing the libtiff structs
     * and reread them in the rdata function.
     */
    tif_read_deinit((TIF_PRIV_READ *) gbm->priv);

    /* Don't override the read_filename and the read_options buffer
     * as they will be readout by internal_tif_rhdr().
     */

    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* check the colormap for 16 bit entries */
static int check_color_map(int n, GBMRGB_16BPP * gbmrgb16)
{
   while (n-- > 0)
   {
      if (gbmrgb16->r > 255 || gbmrgb16->g > 255 || gbmrgb16->b > 255)
      {
         return (16);
      }
      gbmrgb16++;
   }
   return (8);
}


/* internal_tif_rpal_16bpp()  -  Read 16bpp palette */
static GBM_ERR internal_tif_rpal_16bpp(GBM *gbm, GBMRGB_16BPP *gbmrgb)
{
    TIF_PRIV_READ *tif_priv = (TIF_PRIV_READ *) gbm->priv;

    if (gbm->bpp <= 16)
    {
       int i, x;

       /* get the palette information */
       const int palette_entries = 1 << gbm->bpp;
       int increment;

       /* init palette */
       memset(gbmrgb, 0, (sizeof(GBMRGB_16BPP) * palette_entries));

       switch(gbm->bpp)
       {
          case 1:
             increment = 255;
             break;

          case 2:
             increment = 85;
             break;

          case 4:
             increment = 17;
             break;

          case 8:
          case 16:
             increment = 1;
             break;

          default:
             tif_read_deinit(tif_priv);
             return GBM_ERR_READ;
       }

       switch(tif_priv->photometric)
       {
          case PHOTOMETRIC_MINISWHITE:
             i = palette_entries;
             x = 0;
             while (i > 0)
             {
                i--;
                gbmrgb[i].r = gbmrgb[i].g = gbmrgb[i].b = x;
                x += increment;
             }
             break;

          case PHOTOMETRIC_MINISBLACK:
             i = palette_entries;
             x = (gbm->bpp <= 8) ? (GBM_TIF_NUM_COLORS - 1) : (GBM_TIF_NUM_COLORS_16BPP - 1);
             while (i > 0)
             {
                i--;
                gbmrgb[i].r = gbmrgb[i].g = gbmrgb[i].b = x;
                x -= increment;
             }
             break;

          case PHOTOMETRIC_PALETTE:
          {
             /* get the palette information */
             uint16 *red, *green, *blue;

             /* copy the palette entries */
             if (! TIFFGetField(tif_priv->tif_p, TIFFTAG_COLORMAP, &red, &green, &blue))
             {
                tif_read_deinit(tif_priv);
                return GBM_ERR_READ;
             }

             i = palette_entries;
             while (i > 0)
             {
                i--;
                gbmrgb[i].r = red  [i];
                gbmrgb[i].g = green[i];
                gbmrgb[i].b = blue [i];
             }
          }
          break;

          default:
             tif_read_deinit(tif_priv);
             return GBM_ERR_READ;
       }
    }

    return GBM_ERR_OK;
}


/* internal_tif_rpal_8bpp()  -  Read palette */
static GBM_ERR internal_tif_rpal_8bpp(GBM *gbm, GBMRGB *gbmrgb)
{
    TIF_PRIV_READ *tif_priv = (TIF_PRIV_READ *) gbm->priv;

    if (gbm->bpp <= 8)
    {
       int i;

       /* get the palette information */
       const int palette_entries = 1 << gbm->bpp;

       GBMRGB_16BPP * gbmrgb16 = (GBMRGB_16BPP *) gbmmem_malloc(sizeof(GBMRGB_16BPP) * palette_entries);
       if (gbmrgb16 == NULL)
       {
          tif_read_deinit(tif_priv);
          return GBM_ERR_MEM;
       }

       if (internal_tif_rpal_16bpp(gbm, gbmrgb16) != GBM_ERR_OK)
       {
          gbmmem_free(gbmrgb16);
          tif_read_deinit(tif_priv);
          return GBM_ERR_READ;
       }

       /* copy to external GBMRGB struct and downscale if necessary */
       if (check_color_map(palette_entries, gbmrgb16) == 16)
       {
          #define CVT(x) (((x) * 255) / ((1L << 16) - 1))

          i = palette_entries;
          while (i > 0)
          {
             i--;
             gbmrgb[i].r = (byte) CVT(gbmrgb16[i].r);
             gbmrgb[i].g = (byte) CVT(gbmrgb16[i].g);
             gbmrgb[i].b = (byte) CVT(gbmrgb16[i].b);
          }
       }
       else
       {
          i = palette_entries;
          while (i > 0)
          {
             i--;
             gbmrgb[i].r = (byte) gbmrgb16[i].r;
             gbmrgb[i].g = (byte) gbmrgb16[i].g;
             gbmrgb[i].b = (byte) gbmrgb16[i].b;
          }
       }

       gbmmem_free(gbmrgb16);
    }

    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */

/* tif_rpal()  -  Read palette */
GBM_ERR tif_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
{
    TIF_PRIV_READ *tif_priv = (TIF_PRIV_READ *) gbm->priv;

    uint16 bitsPerSample, samplesPerPixel;
    GBM gbm_src;

    /* read the header again */
    GBM_ERR rc = internal_tif_rhdr(fd, gbm);
    if (rc != GBM_ERR_OK)
    {
       /* cleanup is done in the called function already */
       return rc;
    }

    gbm_src = *gbm;

    if ((! TIFFGetField(tif_priv->tif_p, TIFFTAG_BITSPERSAMPLE  , &bitsPerSample))   ||
        (! TIFFGetField(tif_priv->tif_p, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel)))
    {
       tif_read_deinit(tif_priv);
       return GBM_ERR_READ;
    }

    gbm_src.bpp = bitsPerSample * samplesPerPixel;

    /* copy the palette */
    rc = internal_tif_rpal_8bpp(&gbm_src, gbmrgb);
    if (rc != GBM_ERR_OK)
    {
       /* cleanup is done in the called function already */
       return rc;
    }

    /* We cannot expect the client calls rhdr,rpal,rdata in order.
     * The client might just read the header and stop then.
     *
     * There is no other choice than freeing the libtiff structs
     * and reread them in the rdata function.
     */
    tif_read_deinit((TIF_PRIV_READ *) gbm->priv);

    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* Read bitmap data Scanline based for a single plane (continuous) */
GBM_ERR internal_tif_rdata_scanline_contig(GBM *gbm, byte * data)
{
   TIF_PRIV_READ *tif_priv = (TIF_PRIV_READ *) gbm->priv;
   TIFF          *tif_p    = tif_priv->tif_p;

   byte ** gbm_row_pointers = NULL;
   int row;

   /* Read the image in scanlines. Attn: Align to 32 bit rows for GBM !!! */
   const int scanline_bytes = TIFFScanlineSize(tif_p);
   const int gbm_row_bytes  = ((gbm->w * gbm->bpp + 31)/32) * 4;

   /* start at buffer begin */
   gbm_row_pointers = (byte **) gbmmem_malloc(gbm->h * sizeof(byte *));
   if (gbm_row_pointers == NULL)
   {
      return GBM_ERR_MEM;
   }

   /* init pointers from bottom to top because the image is internally a DIB
    * which is mirrored vertically
    */
   gbm_row_pointers[0] = data + (gbm->h * gbm_row_bytes) - gbm_row_bytes;
   for (row = 1; row < gbm->h; row++)
   {
      gbm_row_pointers[row] = gbm_row_pointers[row-1] - gbm_row_bytes;
   }

   /* check if palette based image read or RGB (RGB->BGR conversion needed) */
   if ((gbm->bpp <= 8) && (! tif_priv->upsamplePaletteToRGB)
                       && (! tif_priv->upsamplePaletteToPalette))
   {
      for (row = 0; row < gbm->h; row++)
      {
         if (TIFFReadScanline(tif_p, gbm_row_pointers[row], row, 0) < 0)
         {
            gbmmem_free(gbm_row_pointers);
            return GBM_ERR_TIF_DATA;
         }
      }
   }
   else if (tif_priv->upsamplePaletteToRGB ||
            tif_priv->upsamplePaletteToPalette)
   {
      uint16 bitsPerSample, samplesPerPixel;

      GBM    gbm_src         = *gbm;
      byte * scanline_buffer = NULL;

      if ((! TIFFGetField(tif_p, TIFFTAG_BITSPERSAMPLE  , &bitsPerSample))   ||
          (! TIFFGetField(tif_p, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel)))
      {
         gbmmem_free(gbm_row_pointers);
         return GBM_ERR_READ;
      }

      /* convert bit_depth from channel bit depth to overall bit depth */
      gbm_src.bpp = bitsPerSample * samplesPerPixel;
      if (gbm_src.bpp > 16)
      {
         gbmmem_free(gbm_row_pointers);
         return GBM_ERR_READ;
      }

      /* allocate temporary buffer for reading */
      scanline_buffer = (byte *) gbmmem_malloc(scanline_bytes);
      if (scanline_buffer == NULL)
      {
         gbmmem_free(gbm_row_pointers);
         return GBM_ERR_MEM;
      }

      if (tif_priv->upsamplePaletteToRGB)
      {
         /* get 16 bit color palette */
         GBMRGB_16BPP * gbmrgb_src = (GBMRGB_16BPP *) gbmmem_malloc((1 << gbm_src.bpp) * sizeof(GBMRGB_16BPP));
         if (gbmrgb_src == NULL)
         {
            gbmmem_free(scanline_buffer);
            gbmmem_free(gbm_row_pointers);
            return GBM_ERR_MEM;
         }

         if (internal_tif_rpal_16bpp(&gbm_src, gbmrgb_src) != GBM_ERR_OK)
         {
            gbmmem_free(scanline_buffer);
            gbmmem_free(gbmrgb_src);
            gbmmem_free(gbm_row_pointers);
            return GBM_ERR_READ;
         }

         for (row = 0; row < gbm->h; row++)
         {
            if (TIFFReadScanline(tif_p, scanline_buffer, row, 0) < 0)
            {
               gbmmem_free(scanline_buffer);
               gbmmem_free(gbmrgb_src);
               gbmmem_free(gbm_row_pointers);
               return GBM_ERR_TIF_DATA;
            }

            if (! gbm_map_row_PAL_BGR(scanline_buffer      , &gbm_src,
                                      gbm_row_pointers[row], gbm, gbmrgb_src))
            {
               gbmmem_free(scanline_buffer);
               gbmmem_free(gbmrgb_src);
               gbmmem_free(gbm_row_pointers);
               return GBM_ERR_READ;
            }
         }

         gbmmem_free(gbmrgb_src);
      }
      else /* tif_priv->upsamplePaletteToPalette */
      {
         for (row = 0; row < gbm->h; row++)
         {
            if (TIFFReadScanline(tif_p, scanline_buffer, row, 0) < 0)
            {
               gbmmem_free(scanline_buffer);
               gbmmem_free(gbm_row_pointers);
               return GBM_ERR_TIF_DATA;
            }

            if (! gbm_map_row_PAL_PAL(scanline_buffer      , &gbm_src,
                                      gbm_row_pointers[row], gbm))
            {
               gbmmem_free(scanline_buffer);
               gbmmem_free(gbm_row_pointers);
               return GBM_ERR_READ;
            }
         }
      }

      gbmmem_free(scanline_buffer);
   }
   else /* 24 Bit or higher */
   {
      uint16 bitsPerSample, samplesPerPixel;
      uint16 inkset;

      GBM gbm_src = *gbm;

      if ((! TIFFGetField(tif_p, TIFFTAG_BITSPERSAMPLE  , &bitsPerSample))   ||
          (! TIFFGetField(tif_p, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel)))
      {
         gbmmem_free(gbm_row_pointers);
         return GBM_ERR_READ;
      }

      /* convert bit_depth from channel bit depth to overall bit depth */
      gbm_src.bpp = bitsPerSample * samplesPerPixel;

      /* data will not fit into provided gbm buffer, use temporary one for downsampling */
      if (scanline_bytes > gbm_row_bytes)
      {
         /* This will happen for images > 24 bpp when they are not reported
          * as such from tif_rhdr(). In this case we will downsample to 24 bpp.
          */

         /* allocate temporary buffer for reading */
         byte *  scanline_buffer = (byte *) gbmmem_malloc(scanline_bytes);
         if (scanline_buffer == NULL)
         {
            gbmmem_free(gbm_row_pointers);
            return GBM_ERR_MEM;
         }

         /* check for CMYK */
         inkset = INKSET_MULTIINK; /* not CMYK */
         if (tif_priv->photometric == PHOTOMETRIC_SEPARATED) /* CMYK */
         {
            inkset = INKSET_CMYK; /* CMYK is default */
            TIFFGetField(tif_p, TIFFTAG_INKSET, &inkset);
         }

         /* we only support CMYK images with 8/16 bit per sample and do a simple RGB conversion */
         if (inkset == INKSET_CMYK)
         {
            for (row = 0; row < gbm->h; row++)
            {
               if (TIFFReadScanline(tif_p, scanline_buffer, row, 0) < 0)
               {
                  gbmmem_free(scanline_buffer);
                  gbmmem_free(gbm_row_pointers);
                  return GBM_ERR_TIF_DATA;
               }

               if (! gbm_map_row_CMYK_to_BGR(scanline_buffer      , &gbm_src,
                                             gbm_row_pointers[row],  gbm    , TRUE))
               {
                  gbmmem_free(scanline_buffer);
                  gbmmem_free(gbm_row_pointers);
                  return GBM_ERR_READ;
               }
            }
         }
         else
         {
            for (row = 0; row < gbm->h; row++)
            {
               if (TIFFReadScanline(tif_p, scanline_buffer, row, 0) < 0)
               {
                  gbmmem_free(scanline_buffer);
                  gbmmem_free(gbm_row_pointers);
                  return GBM_ERR_TIF_DATA;
               }

               if (! gbm_map_row_RGBx_BGRx(scanline_buffer      , &gbm_src,
                                           gbm_row_pointers[row],  gbm,
                                           &tif_priv->backrgb   , tif_priv->unassociatedAlpha))
               {
                  gbmmem_free(scanline_buffer);
                  gbmmem_free(gbm_row_pointers);
                  return GBM_ERR_READ;
               }
            }
         }

         gbmmem_free(scanline_buffer);
      }
      else /* copy directly */
      {
         /* check for CMYK */
         inkset = INKSET_MULTIINK; /* not CMYK */
         if (tif_priv->photometric == PHOTOMETRIC_SEPARATED) /* CMYK */
         {
            inkset = INKSET_CMYK; /* CMYK is default */
            TIFFGetField(tif_p, TIFFTAG_INKSET, &inkset);
         }

         /* we only support CMYK images with 8/16 bit per sample and do a simple RGB conversion */
         if (inkset == INKSET_CMYK)
         {
            for (row = 0; row < gbm->h; row++)
            {
               if (TIFFReadScanline(tif_p, gbm_row_pointers[row], row, 0) < 0)
               {
                  gbmmem_free(gbm_row_pointers);
                  return GBM_ERR_TIF_DATA;
               }

               if (! gbm_map_row_CMYK_to_BGR(gbm_row_pointers[row],  gbm,
                                             gbm_row_pointers[row],  gbm, TRUE))
               {
                  gbmmem_free(gbm_row_pointers);
                  return GBM_ERR_READ;
               }
            }
         }
         else
         {
            for (row = 0; row < gbm->h; row++)
            {
               if (TIFFReadScanline(tif_p, gbm_row_pointers[row], row, 0) < 0)
               {
                  gbmmem_free(gbm_row_pointers);
                  return GBM_ERR_TIF_DATA;
               }

               if (! gbm_map_row_RGBx_BGRx(gbm_row_pointers[row], gbm,
                                           gbm_row_pointers[row], gbm,
                                           &tif_priv->backrgb   , tif_priv->unassociatedAlpha))
               {
                  gbmmem_free(gbm_row_pointers);
                  return GBM_ERR_READ;
               }
            }
         }
      }
   }

   gbmmem_free(gbm_row_pointers);
   return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */

/* Read bitmap data Scanline based for multiple planes (separated) */
GBM_ERR internal_tif_rdata_scanline_separate(GBM *gbm, byte * data)
{
   TIF_PRIV_READ *tif_priv = (TIF_PRIV_READ *) gbm->priv;
   TIFF          *tif_p    = tif_priv->tif_p;

   uint16 s, samplesPerPixel, bitsPerSample, inkset;

   byte *  scanline_buffer  = NULL;
   byte ** gbm_row_pointers = NULL;
   int row;

   GBM gbm_src = *gbm;

   /* Read the image in scanlines. Attn: Align to 32 bit rows for GBM !!! */
   const int scanline_bytes = TIFFScanlineSize(tif_p);
   const int gbm_row_bytes  = ((gbm->w * gbm->bpp + 31)/32) * 4;

   /* Only non-palette images can be in separated planes */
   if ( ((tif_priv->photometric != PHOTOMETRIC_SEPARATED) && /* CMYK */
         (tif_priv->photometric != PHOTOMETRIC_RGB))      || /* RGBA */
        (gbm->bpp < 24) )
   {
      return GBM_ERR_READ;
   }

   /* start at buffer begin */
   gbm_row_pointers = (byte **) gbmmem_malloc(gbm->h * sizeof(byte *));
   if (gbm_row_pointers == NULL)
   {
      return GBM_ERR_MEM;
   }

   /* init pointers from bottom to top because the image is internally a DIB
    * which is mirrored vertically
    */
   gbm_row_pointers[0] = data + (gbm->h * gbm_row_bytes) - gbm_row_bytes;
   for (row = 1; row < gbm->h; row++)
   {
      gbm_row_pointers[row] = gbm_row_pointers[row-1] - gbm_row_bytes;
   }

   if ((! TIFFGetField(tif_p, TIFFTAG_BITSPERSAMPLE  , &bitsPerSample))   ||
       (! TIFFGetField(tif_p, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel)))
   {
      gbmmem_free(gbm_row_pointers);
      return GBM_ERR_READ;
   }

   /* convert bit_depth from channel bit depth to overall bit depth */
   gbm_src.bpp = bitsPerSample * samplesPerPixel;

   /* check for extra samples */

   /* check for CMYK */
   inkset = INKSET_MULTIINK; /* not CMYK */
   if (tif_priv->photometric == PHOTOMETRIC_SEPARATED) /* CMYK */
   {
      inkset = INKSET_CMYK; /* CMYK is default */
      TIFFGetField(tif_p, TIFFTAG_INKSET, &inkset);
   }

   /* we only support CMYK images with 8/16 bit per sample and do a simple RGB conversion */
   if (inkset == INKSET_CMYK)
   {
      /* only 4 channel CMYK (no CMY) */
      if (samplesPerPixel != 4)
      {
         gbmmem_free(gbm_row_pointers);
         return GBM_ERR_READ;
      }
      if ((bitsPerSample != 8) && (bitsPerSample != 16))
      {
         gbmmem_free(gbm_row_pointers);
         return GBM_ERR_READ;
      }
   }

   /* allocate temporary buffer for reading */
   scanline_buffer = (byte *) gbmmem_malloc(scanline_bytes);
   if (scanline_buffer == NULL)
   {
      gbmmem_free(gbm_row_pointers);
      return GBM_ERR_MEM;
   }

   if (inkset == INKSET_CMYK)
   {
      for (s = 0; s < samplesPerPixel; s++)
      {
         for (row = 0; row < gbm->h; row++)
         {
            if (TIFFReadScanline(tif_p, scanline_buffer, row, s) < 0)
            {
               gbmmem_free(scanline_buffer);
               gbmmem_free(gbm_row_pointers);
               return GBM_ERR_TIF_DATA;
            }

            /* copy the current plane into one RGB plane */
            if (! gbm_map_sep_row_CMYK_to_BGR(scanline_buffer      , &gbm_src,
                                              gbm_row_pointers[row],  gbm, s , TRUE))
            {
               gbmmem_free(scanline_buffer);
               gbmmem_free(gbm_row_pointers);
               return GBM_ERR_READ;
            }
         }
      }
   }
   else /* RGB, RGBA */
   {
      for (s = 0; s < samplesPerPixel; s++)
      {
         for (row = 0; row < gbm->h; row++)
         {
            if (TIFFReadScanline(tif_p, scanline_buffer, row, s) < 0)
            {
               gbmmem_free(scanline_buffer);
               gbmmem_free(gbm_row_pointers);
               return GBM_ERR_TIF_DATA;
            }

            /* copy the current plane into one RGB plane */
            if (! gbm_map_sep_row_RGBx_BGRx(scanline_buffer      , &gbm_src,
                                            gbm_row_pointers[row],  gbm,
                                            &tif_priv->backrgb   , tif_priv->unassociatedAlpha, s))
            {
               gbmmem_free(scanline_buffer);
               gbmmem_free(gbm_row_pointers);
               return GBM_ERR_READ;
            }
         }
      }

      if (samplesPerPixel < 4)
      {
         /* convert from RGB data to BGR data */
         for (row = 0; row < gbm->h; row++)
         {
            if (! gbm_map_row_RGBx_BGRx(gbm_row_pointers[row], gbm,
                                        gbm_row_pointers[row], gbm,
                                        &tif_priv->backrgb   , tif_priv->unassociatedAlpha))
            {
               gbmmem_free(scanline_buffer);
               gbmmem_free(gbm_row_pointers);
               return GBM_ERR_READ;
            }
         }
      }
   }

   gbmmem_free(scanline_buffer);
   gbmmem_free(gbm_row_pointers);
   return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */

/* Read bitmap data Scanline based */
GBM_ERR internal_tif_rdata_scanline(GBM *gbm, byte * data)
{
   TIF_PRIV_READ *tif_priv = (TIF_PRIV_READ *) gbm->priv;
   TIFF          *tif_p    = tif_priv->tif_p;

   uint16 config;

   /* check if data is splitted into multiple planes */
   if (! TIFFGetField(tif_p, TIFFTAG_PLANARCONFIG, &config))
   {
      return GBM_ERR_READ;
   }

   switch(config)
   {
      case PLANARCONFIG_CONTIG:
         return internal_tif_rdata_scanline_contig(gbm, data);

      case PLANARCONFIG_SEPARATE:
         return internal_tif_rdata_scanline_separate(gbm, data);

      default:
         break;
   }

   return GBM_ERR_READ;
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */



#if 0

/* Read bitmap data Tile based for a single plane (continuous) */
GBM_ERR internal_tif_rdata_tile_contig(GBM *gbm, byte * data)
{
   TIF_PRIV_READ *tif_priv = (TIF_PRIV_READ *) gbm->priv;
   TIFF          *tif_p    = tif_priv->tif_p;

   int row;

   /* Read the image in scanlines. Attn: Align to 32 bit rows for GBM !!! */
   const int tif_tile_count     = TIFFNumberOfTiles(tif_p);
   const int tif_tile_bytes     = TIFFTileSize(tif_p);
   const int tif_tile_row_bytes = TIFFTileRowSize(tif_p);

   const int tif_row_bytes = tif_tile_row_bytes * tif_tile_count;
   const int gbm_row_bytes = ((gbm->w * gbm->bpp + 31)/32) * 4;

   uint32 tileWidth, tileHeight;
   uint32 x, y, t;
   byte * tile_buffer;

   if ((! TIFFGetField(tif_p, TIFFTAG_TILEWIDTH , &tileWidth))  ||
       (! TIFFGetField(tif_p, TIFFTAG_TILELENGTH, &tileHeight)))
   {
      return GBM_ERR_READ;
   }

   /* start at buffer begin */
   gbm_row_pointers = (byte **) gbmmem_malloc(gbm->h * sizeof(byte *));
   if (gbm_row_pointers == NULL)
   {
      return GBM_ERR_MEM;
   }

   /* init pointers from bottom to top because the image is internally a DIB
    * which is mirrored vertically
    */
   gbm_row_pointers[0] = data + (gbm->h * gbm_row_bytes) - gbm_row_bytes;
   for (row = 1; row < gbm->h; row++)
   {
      gbm_row_pointers[row] = gbm_row_pointers[row-1] - gbm_row_bytes;
   }

   /* Copy the tiles of one row sequence into a local buffer and convert to GBM rows. */
   tile_buffer = (byte *) gbmmem_malloc(tif_tile_bytes);
   if (tile_buffer == NULL)
   {
      return GBM_ERR_MEM;
   }

   tile_idx = 0;
   for (y = 0; y < gbm->h; y += tileLength)
   {
      for (x = 0; x < gbm->w; x += tileWidth)
      {
         if (TIFFReadTile(tif_p, tile_buffer, x, y, 0) < 0)
         {
            gbmmem_free(tile_buffer);
            gbmmem_free(gbm_row_pointers);
            return GBM_ERR_TIF_DATA;
         }

         /* copy the lines available for now to GBM rows */
         if (! gbm_map_tile_RGBx_BGRx(tile_buffer, tile_row_bytes, tile_height, &gbm_src,
                                      gbm_row_pointers[row],  gbm,
                                      &tif_priv->backrgb   , tif_priv->unassociatedAlpha))
         {
            gbmmem_free(tile_buffer);
            gbmmem_free(gbm_row_pointers);
            return GBM_ERR_READ;
         }
      }
   }

   gbmmem_free(tile_buffer);
   gbmmem_free(gbm_row_pointers);

   return GBM_ERR_READ;
}

/* ----------------------------------------------------------- */

/* Read bitmap data Tile based for multiple planes (separated) */
GBM_ERR internal_tif_rdata_tile_separate(GBM *gbm, byte * data)
{

/*   return GBM_ERR_OK; */
   return GBM_ERR_READ;
}

/* ----------------------------------------------------------- */

/* Read bitmap data Tile based */
GBM_ERR internal_tif_rdata_tile(GBM *gbm, byte * data)
{
   TIF_PRIV_READ *tif_priv = (TIF_PRIV_READ *) gbm->priv;
   TIFF          *tif_p    = tif_priv->tif_p;

   uint16 config;

   /* check if data is splitted into multiple planes */
   if (! TIFFGetField(tif_p, TIFFTAG_PLANARCONFIG, &config))
   {
      return GBM_ERR_READ;
   }

   switch(config)
   {
      case PLANARCONFIG_CONTIG:
         return internal_tif_rdata_tile_contig(gbm, data);

      case PLANARCONFIG_SEPARATE:
         return internal_tif_rdata_tile_separate(gbm, data);

      default:
         break;
   }

   return GBM_ERR_READ;
}

#endif




/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* Read bitmap data as RGBA */
GBM_ERR internal_tif_rdata_RGBA(GBM *gbm, byte * data)
{
   TIF_PRIV_READ *tif_priv = (TIF_PRIV_READ *) gbm->priv;
   TIFF          *tif_p    = tif_priv->tif_p;

   /* The easiest way to read the image. Attn: Align to 32 bit rows for GBM !!! */
   const int byte_length = gbm->w * gbm->h * sizeof(uint32);

   uint32 * tif_data = (uint32*) gbmmem_malloc(byte_length);
   if (tif_data == NULL)
   {
      return GBM_ERR_MEM;
   }

   /* Read the image data: RGBA format */
   if (! TIFFReadRGBAImage(tif_p, gbm->w, gbm->h, tif_data, 1 /* stop on error */))
   {
      gbmmem_free(tif_data);
      return GBM_ERR_TIF_DATA;
   }

   if (gbm->bpp <= 8)
   {
      GBMRGB * gbmrgb = (GBMRGB *) gbmmem_malloc((1 << gbm->bpp) * sizeof(GBMRGB));

      /* Get the target palette */
      if (internal_tif_rpal_8bpp(gbm, gbmrgb) != GBM_ERR_OK)
      {
         gbmmem_free(gbmrgb);
         gbmmem_free(tif_data);
         return GBM_ERR_READ;
      }

      /* convert the image from RGBA -> Palette */
      if (! gbm_map_RGBA_PAL((const dword *) tif_data, data, gbm, gbmrgb))
      {
         gbmmem_free(gbmrgb);
         gbmmem_free(tif_data);
         return GBM_ERR_READ;
      }

      gbmmem_free(gbmrgb);
   }
   else /* must be 24 Bit destination */
   {
      uint16 compression = COMPRESSION_NONE;
      if (! TIFFGetField(tif_p, TIFFTAG_COMPRESSION, &compression))
      {
         compression = COMPRESSION_NONE;
      }

      /* convert the image from RGBA -> BGR */
      if (! gbm_map_RGBA_BGR((const dword *) tif_data, data, gbm, &tif_priv->backrgb, tif_priv->unassociatedAlpha))
      {
         gbmmem_free(tif_data);
         return GBM_ERR_READ;
      }
   }

   gbmmem_free(tif_data);
   return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* tif_rdata()  -  Read data */
GBM_ERR tif_rdata(int fd, GBM *gbm, byte *data)
{
   /* read the header again */
   GBM_ERR rc = internal_tif_rhdr(fd, gbm);
   if (rc != GBM_ERR_OK)
   {
      /* cleanup is done in the called function already */
      return rc;
   }
   else
   {
      TIF_PRIV_READ *tif_priv = (TIF_PRIV_READ *) gbm->priv;
      TIFF          *tif_p    = tif_priv->tif_p;

      /* Read BW, Palette, RGB and CMYK data by low level interface
       * and the rest via RGBA reader.
       */
      switch(tif_priv->photometric)
      {
          case PHOTOMETRIC_MINISWHITE:
          case PHOTOMETRIC_MINISBLACK:
          case PHOTOMETRIC_PALETTE:
          case PHOTOMETRIC_SEPARATED:
          case PHOTOMETRIC_RGB:
             /* tile based organization ? */
             if (TIFFIsTiled(tif_p))
             {
                rc = internal_tif_rdata_RGBA(gbm, data);
                /* rc = internal_tif_rdata_tile(gbm, data); */
                if (rc != GBM_ERR_OK)
                {
                   tif_read_deinit(tif_priv);
                   return rc;
                }
             }
             else /* read via scanlines */
             {
                rc = internal_tif_rdata_scanline(gbm, data);
                if (rc != GBM_ERR_OK)
                {
                   tif_read_deinit(tif_priv);
                   return rc;
                }
             }
             break;

          /* -------------- */

          default: /* try to load everything else via the RGBA reader */
             /* PHOTOMETRIC_YCBCR  */
             /* PHOTOMETRIC_CIELAB */
             /* PHOTOMETRIC_ICCLAB */
             /* PHOTOMETRIC_ITULAB */
             /* ...                */
             rc = internal_tif_rdata_RGBA(gbm, data);
             if (rc != GBM_ERR_OK)
             {
                tif_read_deinit(tif_priv);
                return rc;
             }
             break;
      }

      /* cleanup */
      tif_read_deinit(tif_priv);
   }

   return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

static BOOLEAN set_user_tag(TIFF* tif_p, ttag_t tag, char *name, const char *opt, char *def)
{
   char buf[200+1] = { 0 };
   const char *s;

   if ((s = gbm_find_word_prefix(opt, name)) != NULL)
   {
      if (sscanf(s + strlen(name), "%200[^\"]", buf) != 1)
      {
         if (sscanf(s + strlen(name), "%200[^ ]", buf) != 1)
         {
            return FALSE;
         }
      }
   }
   else
   {
       strcpy(buf, def);
   }

   if (*buf == '\0')
   {
      return TRUE;
   }

   if (! TIFFSetField(tif_p, tag, buf))
   {
      return FALSE;
   }

   return TRUE;
}

/* tif_w()  -  Write bitmap file */
GBM_ERR tif_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt)
{
   TIF_PRIV_WRITE  tif_priv;
   TIFF           *tif_p = NULL;

   uint16 photometric = 0;
   uint16 bitsPerSample;
   uint16 samplesPerPixel;

   uint16 extraSamples = 0;

   uint16 extraSamplesInfoAlpha = EXTRASAMPLE_UNASSALPHA;
   uint16 * pExtraSamplesInfo = NULL,
          * pExtraSamplesInfoAlpha = { &extraSamplesInfoAlpha };

   uint16  compression           = COMPRESSION_NONE;
   BOOLEAN compression_set       = FALSE;
   uint16  lzw_predictor         = PREDICTOR_NONE;
   uint16  zip_compression_level = 6;
   uint16  jpeg_quality_level    = 75;
   uint32  strip_size            = 16*1024;

   const char * s;

   BOOLEAN associate_alpha = FALSE;

   if (gbm_find_word_prefix(opt, "back_rgb=") != NULL)
   {
      /* enforce using real palette instead of BW */
      extraSamplesInfoAlpha = EXTRASAMPLE_ASSOCALPHA;
      associate_alpha = TRUE;
   }

   /* map GBM to TIFF infos */
   switch(gbm->bpp)
   {
      case 1:
         bitsPerSample     = 1;
         samplesPerPixel   = 1;
         extraSamples      = 0;
         pExtraSamplesInfo = NULL;
         if (gbmrgb == NULL)
         {
            return GBM_ERR_NOT_SUPP;
         }

         /* enforce using real palette instead of BW */
         if (gbm_find_word(opt, "pal1bpp") != NULL)
         {
            photometric = PHOTOMETRIC_PALETTE;
         }
         else
         {
            photometric = PHOTOMETRIC_PALETTE;
            if ((gbmrgb[0].r == gbmrgb[0].g) && (gbmrgb[0].r == gbmrgb[0].b) && (gbmrgb[0].g == gbmrgb[0].b) &&
                (gbmrgb[1].r == gbmrgb[1].g) && (gbmrgb[1].r == gbmrgb[1].b) && (gbmrgb[1].g == gbmrgb[1].b))
            {
               if ((gbmrgb[0].r == 0) && (gbmrgb[1].r == 0xff))
               {
                  photometric = PHOTOMETRIC_MINISBLACK;
               }
               else if ((gbmrgb[0].r == 0xff) && (gbmrgb[1].r == 0))
               {
                  photometric = PHOTOMETRIC_MINISWHITE;
               }
            }
         }
         break;

      /* case 2: The export of 2 bpp files is already supported    */
      /*         but all GBM imports can only handle 4 bpp. So we  */
      /*         keep this for consistency.                        */
      case 4:
      case 8:
         bitsPerSample     = gbm->bpp;
         samplesPerPixel   = 1;
         extraSamples      = 0;
         pExtraSamplesInfo = NULL;
         photometric       = PHOTOMETRIC_PALETTE;
         break;

      case 24:
         bitsPerSample     = 8;
         samplesPerPixel   = 3;
         extraSamples      = 0;
         pExtraSamplesInfo = NULL;
         photometric       = PHOTOMETRIC_RGB;
         break;

      case 32:
         bitsPerSample     = 8;
         samplesPerPixel   = 4;
         extraSamples      = 1;
         pExtraSamplesInfo = pExtraSamplesInfoAlpha;
         photometric       = PHOTOMETRIC_RGB;
         break;

      case 48:
         bitsPerSample     = 16;
         samplesPerPixel   = 3;
         extraSamples      = 0;
         pExtraSamplesInfo = NULL;
         photometric       = PHOTOMETRIC_RGB;
         break;

      case 64:
         bitsPerSample     = 16;
         samplesPerPixel   = 4;
         extraSamples      = 1;
         pExtraSamplesInfo = pExtraSamplesInfoAlpha;
         photometric       = PHOTOMETRIC_RGB;
         break;

      default:
         return GBM_ERR_NOT_SUPP;
   }

   /* parse compression schemes */

   /* --- B/W & RGB encodings -------- */

   if (gbm_find_word(opt, "lzw") != NULL)
   {
      compression_set = TRUE;
      compression     = COMPRESSION_LZW;
      lzw_predictor   = PREDICTOR_NONE;
   }
   if (gbm_find_word(opt, "lzw_pred_hor") != NULL)
   {
      compression_set = TRUE;
      compression     = COMPRESSION_LZW;
      lzw_predictor   = PREDICTOR_HORIZONTAL;
   }
   if (gbm_find_word(opt, "packbits") != NULL)
   {
      /* don't allow compression override */
      if (compression_set)
      {
         return GBM_ERR_BAD_OPTION;
      }
      compression_set = TRUE;
      compression     = COMPRESSION_PACKBITS;
   }
   if (gbm_find_word(opt, "deflate") != NULL)
   {
      /* don't allow compression override */
      if (compression_set)
      {
         return GBM_ERR_BAD_OPTION;
      }
      compression_set = TRUE;
      compression     = COMPRESSION_ADOBE_DEFLATE;
   }
   if (gbm_find_word(opt, "jpeg") != NULL)
   {
      /* don't allow compression override */
      if (compression_set)
      {
         return GBM_ERR_BAD_OPTION;
      }
      /* JPEG only supports 24 bpp */
      if (gbm->bpp != 24)
      {
         return GBM_ERR_BAD_OPTION;
      }
      compression_set = TRUE;
      compression     = COMPRESSION_JPEG;
   }

   /* --- B/W encodings -------- */

   if (gbm_find_word(opt, "ccittrle") != NULL)
   {
      /* don't allow compression override */
      if (compression_set)
      {
         return GBM_ERR_BAD_OPTION;
      }
      /* this compression is only supported for BW images */
      if ((gbm->bpp != 1) ||
          ((photometric != PHOTOMETRIC_MINISBLACK) && (photometric != PHOTOMETRIC_MINISWHITE)))
      {
         return GBM_ERR_BAD_OPTION;
      }
      compression_set = TRUE;
      compression     = COMPRESSION_CCITTRLE;
   }
   if (gbm_find_word(opt, "ccittfax3") != NULL)
   {
      /* don't allow compression override */
      if (compression_set)
      {
         return GBM_ERR_BAD_OPTION;
      }
      /* this compression is only supported for BW images */
      if ((gbm->bpp != 1) ||
          ((photometric != PHOTOMETRIC_MINISBLACK) && (photometric != PHOTOMETRIC_MINISWHITE)))
      {
         return GBM_ERR_BAD_OPTION;
      }
      compression_set = TRUE;
      compression     = COMPRESSION_CCITTFAX3;
   }
   if (gbm_find_word(opt, "ccittfax4") != NULL)
   {
      /* don't allow compression override */
      if (compression_set)
      {
         return GBM_ERR_BAD_OPTION;
      }
      /* this compression is only supported for BW images */
      if ((gbm->bpp != 1) ||
          ((photometric != PHOTOMETRIC_MINISBLACK) && (photometric != PHOTOMETRIC_MINISWHITE)))
      {
         return GBM_ERR_BAD_OPTION;
      }
      compression_set = TRUE;
      compression     = COMPRESSION_CCITTFAX4;
   }

   /* --- encoding options -------- */

   /* parse compression levels */
   if ((s = gbm_find_word_prefix(opt, "deflate_comp=")) != NULL)
   {
      int compression_level = 6;

      if ((! compression_set) || (compression != COMPRESSION_ADOBE_DEFLATE))
      {
         return GBM_ERR_BAD_OPTION;
      }
      if (sscanf(s + 13, "%d", &compression_level) != 1)
      {
         return GBM_ERR_BAD_OPTION;
      }
      if ((compression_level < 1) || (compression_level > 9))
      {
         return GBM_ERR_BAD_OPTION;
      }

      zip_compression_level = compression_level;
   }

   if ((s = gbm_find_word_prefix(opt, "jpeg_quality=")) != NULL)
   {
      int quality_level = 75;

      if ((! compression_set) || (compression != COMPRESSION_JPEG))
      {
         return GBM_ERR_BAD_OPTION;
      }
      if (sscanf(s + 13, "%d", &quality_level) != 1)
      {
         return GBM_ERR_BAD_OPTION;
      }
      if ((quality_level < 0) || (quality_level > 100))
      {
         return GBM_ERR_BAD_OPTION;
      }

      jpeg_quality_level = quality_level;
   }

   /* parse strip size */
   if ((s = gbm_find_word_prefix(opt, "strip_size=")) != NULL)
   {
      int size;

      if (sscanf(s + 11, "%d", &size) != 1)
      {
         return GBM_ERR_BAD_OPTION;
      }

      switch(size)
      {
         case 4:
         case 8:
         case 16:
         case 32:
         case 64:
            strip_size = size * 1024;
            break;

         default:
            return GBM_ERR_BAD_OPTION;
      }
   }

   /* Open file for writing (overriding and existing file) */
   if (! tif_write_init(&tif_priv, fn, fd))
   {
      return GBM_ERR_NOT_SUPP;
   }

   tif_p = tif_priv.tif_p;

   /* set informational tags */
   if ((! set_user_tag(tif_p, TIFFTAG_ARTIST          , "artist="          , opt, "")) ||
       (! set_user_tag(tif_p, TIFFTAG_SOFTWARE        , "software="        , opt, "")) ||
       (! set_user_tag(tif_p, TIFFTAG_MAKE            , "make="            , opt, "")) ||
       (! set_user_tag(tif_p, TIFFTAG_MODEL           , "model="           , opt, "")) ||
       (! set_user_tag(tif_p, TIFFTAG_HOSTCOMPUTER    , "host="            , opt, "")) ||
       (! set_user_tag(tif_p, TIFFTAG_DOCUMENTNAME    , "documentname="    , opt, "")) ||
       (! set_user_tag(tif_p, TIFFTAG_PAGENAME        , "pagename="        , opt, "")) ||
       (! set_user_tag(tif_p, TIFFTAG_IMAGEDESCRIPTION, "imagedescription=", opt, "")))
   {
      tif_write_deinit(&tif_priv);
      return GBM_ERR_WRITE;
   }

   /* set fields */
   if ((! TIFFSetField(tif_p, TIFFTAG_IMAGEWIDTH     , gbm->w))              ||
       (! TIFFSetField(tif_p, TIFFTAG_IMAGELENGTH    , gbm->h))              ||
       (! TIFFSetField(tif_p, TIFFTAG_BITSPERSAMPLE  , bitsPerSample))       ||
       (! TIFFSetField(tif_p, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel))     ||
       (! TIFFSetField(tif_p, TIFFTAG_EXTRASAMPLES   , extraSamples, pExtraSamplesInfo)) ||
       (! TIFFSetField(tif_p, TIFFTAG_PHOTOMETRIC    , photometric))         ||
       (! TIFFSetField(tif_p, TIFFTAG_PLANARCONFIG   , PLANARCONFIG_CONTIG)))
   {
      tif_write_deinit(&tif_priv);
      return GBM_ERR_WRITE;
   }

   if (compression_set)
   {
      switch(compression)
      {
         case COMPRESSION_NONE:
         case COMPRESSION_PACKBITS:
         case COMPRESSION_CCITTRLE:
         case COMPRESSION_CCITTFAX3:
         case COMPRESSION_CCITTFAX4:
            if (! TIFFSetField(tif_p, TIFFTAG_COMPRESSION, compression))
            {
               tif_write_deinit(&tif_priv);
               return GBM_ERR_WRITE;
            }
            break;

         case COMPRESSION_LZW:
            if ((! TIFFSetField(tif_p, TIFFTAG_COMPRESSION, compression)) ||
                (! TIFFSetField(tif_p, TIFFTAG_PREDICTOR  , lzw_predictor)))
            {
               tif_write_deinit(&tif_priv);
               return GBM_ERR_WRITE;
            }
            break;

         case COMPRESSION_ADOBE_DEFLATE:
            if ((! TIFFSetField(tif_p, TIFFTAG_COMPRESSION, compression)) ||
                (! TIFFSetField(tif_p, TIFFTAG_ZIPQUALITY , zip_compression_level)))
            {
               tif_write_deinit(&tif_priv);
               return GBM_ERR_WRITE;
            }
            break;

         case COMPRESSION_JPEG:
            if ((! TIFFSetField(tif_p, TIFFTAG_COMPRESSION, compression)) ||
                (! TIFFSetField(tif_p, TIFFTAG_JPEGQUALITY, jpeg_quality_level)))
            {
               tif_write_deinit(&tif_priv);
               return GBM_ERR_WRITE;
            }
            break;

         default:
            tif_write_deinit(&tif_priv);
            return GBM_ERR_WRITE;
      }
   }

   /* set palette */
   if (photometric == PHOTOMETRIC_PALETTE)
   {
      int i;
      const int palette_entries = 1 << gbm->bpp;

      /* Scale colormap to TIFF-required 16-bit values. */
      #define SCALE(x) ((((uint16) x) * ((1L << 16) - 1)) / 255)

      uint16 red  [GBM_TIF_NUM_COLORS] = { 0 },
             green[GBM_TIF_NUM_COLORS] = { 0 },
             blue [GBM_TIF_NUM_COLORS] = { 0 };

      for (i = 0; i < palette_entries; ++i)
      {
         red  [i] = SCALE(gbmrgb[i].r);
         green[i] = SCALE(gbmrgb[i].g);
         blue [i] = SCALE(gbmrgb[i].b);
      }
      if (! TIFFSetField(tif_p, TIFFTAG_COLORMAP, red, green, blue))
      {
         tif_write_deinit(&tif_priv);
         return GBM_ERR_WRITE;
      }
   }

   /* write bitmap data (BGRA) as scanlines */
   {
      byte ** gbm_row_pointers = NULL;
      int  row;

      /* Read the image in scanlines. Attn: Align to 32 bit rows for GBM !!! */
      const int gbm_row_bytes  = ((gbm->w * gbm->bpp + 31)/32) * 4;
            int rows_per_strip = gbm_row_bytes * gbm->h / strip_size;

      /* align to 8 rows per strip because some codecs (JPEG) require this */
      rows_per_strip = (rows_per_strip > 0)
                       ? ((rows_per_strip + 7) / 8) * 8
                       : 8;

      if (! TIFFSetField(tif_p, TIFFTAG_ROWSPERSTRIP, rows_per_strip))
      {
         tif_write_deinit(&tif_priv);
         return GBM_ERR_WRITE;
      }

      /* start at buffer begin */
      gbm_row_pointers = (byte **) gbmmem_malloc(gbm->h * sizeof(byte *));
      if (gbm_row_pointers == NULL)
      {
         tif_write_deinit(&tif_priv);
         return GBM_ERR_MEM;
      }

      /* init pointers from bottom to top because the image is internally a DIB
       * which is mirrored vertically
       */
      gbm_row_pointers[0] = (byte *) data + (gbm->h * gbm_row_bytes) - gbm_row_bytes;
      for (row = 1; row < gbm->h; row++)
      {
         gbm_row_pointers[row] = gbm_row_pointers[row-1] - gbm_row_bytes;
      }

      /* check if palette based image read or RGB (RGB->BGR conversion needed) */
      if (gbm->bpp <= 8)
      {
         for (row = 0; row < gbm->h; row++)
         {
            if (TIFFWriteScanline(tif_p, gbm_row_pointers[row], row, 0) < 0)
            {
               gbmmem_free(gbm_row_pointers);
               tif_write_deinit(&tif_priv);
               return GBM_ERR_WRITE;
            }
         }
      }
      else
      {
         GBMRGB_16BPP backrgb = { 0, 0, 0 };
         byte *  scanline_buffer = NULL;

         /* parse RGB value for background mixing with alpha channel */
         if ((s = gbm_find_word_prefix(opt, "back_rgb=")) != NULL)
         {
            int image_background_red   = 0;
            int image_background_green = 0;
            int image_background_blue  = 0;

            if (sscanf(s + 9, "%d_%d_%d", &image_background_red,
                                          &image_background_green,
                                          &image_background_blue) != 3)
            {
               tif_write_deinit(&tif_priv);
               return GBM_ERR_BAD_OPTION;
            }

            if (((image_background_red  < 0) || (image_background_red   > 0xffff)) ||
               ((image_background_green < 0) || (image_background_green > 0xffff)) ||
               ((image_background_blue  < 0) || (image_background_blue  > 0xffff)))
            {
               tif_write_deinit(&tif_priv);
               return GBM_ERR_BAD_OPTION;
            }

            /* we need to set the color value as BGR so that we can reuse the converter */
            backrgb.r = image_background_blue;
            backrgb.g = image_background_green;
            backrgb.b = image_background_red;
         }

         /* allocate temporary buffer for reading */
         scanline_buffer = (byte *) gbmmem_malloc(gbm_row_bytes);
         if (scanline_buffer == NULL)
         {
            gbmmem_free(gbm_row_pointers);
            tif_write_deinit(&tif_priv);
            return GBM_ERR_MEM;
         }

         for (row = 0; row < gbm->h; row++)
         {
            /* we can use the same function for converting back from BGRx to RGBx */
            if (! gbm_map_row_RGBx_BGRx(gbm_row_pointers[row], gbm,
                                        scanline_buffer      , gbm,
                                        &backrgb             , associate_alpha))
            {
               gbmmem_free(scanline_buffer);
               gbmmem_free(gbm_row_pointers);
               tif_write_deinit(&tif_priv);
               return GBM_ERR_WRITE;
            }

            if (TIFFWriteScanline(tif_p, scanline_buffer, row, 0) < 0)
            {
               gbmmem_free(scanline_buffer);
               gbmmem_free(gbm_row_pointers);
               tif_write_deinit(&tif_priv);
               return GBM_ERR_WRITE;
            }
         }

         gbmmem_free(scanline_buffer);
      }

      gbmmem_free(gbm_row_pointers);
   }

   /* Writing finished, so cleanup. The file is closed by GBM. */
   TIFFFlush(tif_priv.tif_p);
   tif_write_deinit(&tif_priv);

   return GBM_ERR_OK;
}


/* ----------------------------------------------------------- */

/* tif_err - map error code to error message */
const char *tif_err(GBM_ERR rc)
{
   TIF_GBMERR_MSG *tif_errmsg_p = tif_errmsg;

   while (tif_errmsg_p->error_message)
   {
       if (tif_errmsg_p->rc == rc)
       {
          return tif_errmsg_p->error_message;
       }

       tif_errmsg_p++;
   }

   return NULL;
}

#endif


