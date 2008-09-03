/*************************************************************************

gbmpng.c - Portable Network Graphics Format support

Credit for writing this module must go to Heiko Nitzsche.

This file is just as public domain as the rest of GBM.

This code is a mainly a wrapper around the official libpng library and
thus supports all features of the actual version.

Supported formats and options:
------------------------------
PNG : Portable Network Graphics Format : .PNG

Standard formats (backward compatible):
  Reads  1 bpp black/white and colour palette/gray level files.
  Reads  2 bpp colour palette/gray level files and presents them as  4 bpp.
  Reads  4 bpp colour palette/gray level files and presents them as  4 bpp.
  Reads  8 bpp colour palette/gray level files and presents them as  8 bpp.
  Reads 16 bpp gray level files and presents them as 24 bpp.
  Reads 32 bpp gray level files (16 bit gray + alpha channel) and presents them as 24 bpp.
  Reads 24 bpp colour files and presents them as 24 bpp.
  Reads 32 bpp colour files (RGB + alpha channel) and presents them as 24 bpp.
  Reads 48 bpp colour files and presents them as 24 bpp.
  Reads 64 bpp colour files (RGB + alpha channel) and presents them as 24 bpp.

Extended formats (not backward compatible, import option ext_bpp required):
  Reads 16 bpp gray level files and presents them as 48 bpp.
  Reads 32 bpp gray level files (16 bit gray + alpha channel) and presents them as 64 bpp.
  Reads 32 bpp colour files (RGB + alpha channel) and presents them as 32 bpp.
  Reads 48 bpp colour files and presents them as 48 bpp.
  Reads 64 bpp colour files (RGB + alpha channel) and presents them as 64 bpp.

  Writes 1, 4, 8, 24, 32 (24 bpp + alpha), 48, 64 (48 bpp + alpha) bpp RGB files.

  The depth of the alpha channel must in all cases match with the depth of all RGB channels.


  Input:
  ------

  Handling of alpha channel:
    PNG alpha channel is always unassociated (not pre-multiplied alpha).
    The alpha channel is returned separately unless the client requests that it has
    to be calculated against the provided background color.
    In the case the ext_bpp option is not specified and the bitmap contains an alpha
    channel, the alpha channel is only calculated against the background color if a
    background color has been specified by the client or is provided by the bitmap.
    Otherwise the alpha channel data is ignored.

  Can specify screen gamma
    Input option: gamma=# (default=2.2)

  Can specify a default background color as RGB to be used for all image types
  (if the bitmap has a background associated, it is replaced by the specified color)
    Input option: back_rgb=#_#_# (red_green_blue, 0..65535)

  Can ignore a bitmap provided background
  (helpful if the alpha channel should be preserved and the bitmap rendered by the
   application against a textured background)
    Input option: ignore_back


  Output:
  -------

  Handling of alpha channel:
    GBM allows writing of unassociated alpha channel information for 32 and 64 bpp.
    If the alpha channel has already been associated (pre-multiplied), color depths
    of 24 and 48 bpp should be used when writing RGB bitmaps.
    If a full alpha channel is provided, transparency cannot be specified additionally.

  Can write interlaced PNGs (good for HTML)
    Output option: ilace

  Can write user defined image gamma
    Output option: gamma=#

  Can specify background colour index for palette/gray images to write in file PNG header.
  (alpha channel is preserved in the bitmap file)
    Output option: back_index=#

  Can specify background colour as RGB for RGB images to write in file PNG header.
  (alpha channel is preserved in the bitmap file)
    Output option: back_rgb=#_#_# (red_green_blue, 24/32 bpp: 0..255, 48/64 bpp: 0..65535)

  Can specify transparency for palette images to write in file PNG header.
    Output option: transparency=#  (0..255, opaque..transparent)

  Can specify transparency colour for RGB images to write in file PNG header.
  (only for bitmap without alpha channel)
    Output option: transcol_rgb=#_#_# (red_green_blue, 24/32 bpp: 0..255, 48/64 bpp: 0..65535)

  Can specify compression level
    Output option: compression=# (default=6, 0..9, no compression to max)


History:
--------
24-Aug-2005: First official version (Libpng 1.2.8, Zlib 1.2.3)

27-Aug-2005: Fix possible memory leak when don't going through the full path of
             rhdr->[rpal]->rdata. The header might be read without the data, which
             would cause a leftover of the read_init structs (in PNG_PRIV_READ).

             Solution: Cleanup the read info structs for each step and reread
                       header for each step. The performance impact is neglectable.

28-Sep-2005: Support for alpha channel, transparency chunks and extended color
             depths returned to the client (option ext_bpp). Writing of alpha
             channel bitmaps is now possible as well.

22-Feb-2006: Move format description strings to gbmdesc.h

07-May-2006: Update to Libpng 1.2.10

26-Mar-2008: Update to Libpng 1.2.25
             Fix: Missing background color when downconverting RGBA
                  with 4 channels to 3 channels by stripping alpha channel.
                  This makes some images look badly if the user did not
                  provide a custom background color. Now a white background
                  is used instead of simply stripping the alpha channel.

04-Apr-2008: Use read ahead cache and write cache mechanisms to speed up file accesses,
             Fixed gbm_readbuf_ahead() in this context.

15-Aug-2008: Integrate new GBM types

26-Aug-2008: Register our own memory management functions to support
             high memory also on OS/2 (gbmmem_).
             Update to Libpng 1.2.31

******************************************************************************/

#ifdef ENABLE_PNG

/* Includes */
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmhelp.h"
#include "gbmdesc.h"
#include "gbmmap.h"
#include "gbmmem.h"
#include "png.h"

/* ----------------------------------------------------------- */

#define GBM_ERR_PNG                   ((GBM_ERR) 5199)
#define GBM_ERR_PNG_BPP               ((GBM_ERR) 5100)
#define GBM_ERR_PNG_HEADER            ((GBM_ERR) 5101)
#define GBM_PNG_BYTES_TO_CHECK        4        /* 1..8 for file format prediction */
#define GBM_PNG_NUM_COLORS            0x100    /* 256 entries in color table */

#define GBM_PNG_DEFAULT_IMAGE_GAMMA   0.45455
#define GBM_PNG_DEFAULT_SCREEN_GAMMA  2.2

/* ----------------------------------------------------------- */

typedef struct
{
    GBM_ERR rc;
    const char *error_message;
} PNG_GBMERR_MSG;

static PNG_GBMERR_MSG png_errmsg[] =
{
    { GBM_ERR_PNG_BPP      , "bad bits per pixel" },
    { GBM_ERR_PNG_HEADER   , "bad header"         },
    { -1                   , NULL                 },
};

static GBMFT png_gbmft =
{
    GBM_FMT_DESC_SHORT_PNG,
    GBM_FMT_DESC_LONG_PNG,
    GBM_FMT_DESC_EXT_PNG,
    GBM_FT_R1 | GBM_FT_R4 | GBM_FT_R8 | GBM_FT_R24 | GBM_FT_R32 | GBM_FT_R48 | GBM_FT_R64 |
    GBM_FT_W1 | GBM_FT_W4 | GBM_FT_W8 | GBM_FT_W24 | GBM_FT_W32 | GBM_FT_W48 | GBM_FT_W64,
};

union FILEIO_CACHE
{
    AHEAD  * ahead;  /* file read ahead  descriptor used for mapping to libpng I/O  */
    WCACHE * wcache; /* file write cache descriptor used for mapping to libpng I/O  */
};

typedef struct
{
    union FILEIO_CACHE file;
    gbm_boolean  errok;  /* error code for I/O, 0 for succeeded else failed */
} PNG_PRIV_IO;


typedef struct
{
    PNG_PRIV_IO  io;
    png_structp  png_ptr;          /* this will also link to PNG_PRIV_IO */
    png_infop    info_ptr;
    png_colorp   palette_ptr;      /* pointer to potential palette */
} PNG_PRIV_WRITE;


typedef struct
{
    PNG_PRIV_IO  io;
    png_structp  png_ptr;           /* this will also link to PNG_PRIV_IO */
    png_infop    info_ptr;
    png_infop    end_info;

    int          bpp_src;                  /* bpp of the source bitmap (used for format translation */
    int          color_type;               /* the bitmap color encoding */
    gbm_boolean  upsamplePaletteToPalette; /* GBM_TRUE if palette file has to be upsampled to other palette */
    gbm_boolean  unassociatedAlpha;        /* GBM_TRUE if an unassociated alpha channel exists */
    GBMRGB_16BPP backrgb;                  /* background RGB color for Alpha channel mixing */

    /* This entry will store the options provided during first header read.
     * It will keep the options for the case the header has to be reread.
     */
    char read_options[PRIV_SIZE - sizeof(PNG_PRIV_IO)
                                - sizeof(png_structp)
                                - (2*sizeof(png_infop))
                                - (2*sizeof(int))
                                - (2*sizeof(gbm_boolean))
                                - sizeof(GBMRGB_16BPP)
                                - 20 /* space for structure element padding */ ];

} PNG_PRIV_READ;


/* ----------------------------------------------------------- */

/* Replace standard libpng memory management functions by our own. */

static png_voidp png_gbm_malloc(png_structp png_ptr, png_size_t size)
{
    png_ptr = png_ptr; /* prevent compiler warning */
    return gbmmem_malloc(size);
}

static void png_gbm_free(png_structp png_ptr, png_voidp ptr)
{
    png_ptr = png_ptr; /* prevent compiler warning */
    gbmmem_free(ptr);
}

/* ----------------------------------------------------------- */

/* Replace standard libpng I/O functions by our own that simply call the
   GBM internal I/O functions.
*/

static void PNGAPI png_gbm_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   PNG_PRIV_IO *io_p = (PNG_PRIV_IO *) png_get_io_ptr(png_ptr); /* io_ptr is pointer to private IO struct */

   /* limit block size to INT_MAX */
   if (length > INT_MAX)
   {
      io_p->errok = GBM_FALSE; /* error */
      png_error(png_ptr, "write request too long");
      return;
   }

   if (gbm_readbuf_ahead(io_p->file.ahead, data, (int) length) != (int) length)
   {
      io_p->errok = GBM_FALSE; /* error */
      png_error(png_ptr, "read request failed");
      return;
   }

   io_p->errok = GBM_TRUE; /* no error */
}

static void PNGAPI png_gbm_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   PNG_PRIV_IO *io_p = (PNG_PRIV_IO *) png_get_io_ptr(png_ptr); /* io_ptr is pointer to private IO struct */

   /* limit block size to INT_MAX */
   if (length > INT_MAX)
   {
      io_p->errok = GBM_FALSE; /* error */
      png_error(png_ptr, "write request too long");
      return;
   }

   if (gbm_writebuf_wcache(io_p->file.wcache, data, (int) length) != (int) length)
   {
      io_p->errok = GBM_FALSE; /* error */
      png_error(png_ptr, "write request failed");
      return;
   }

   io_p->errok = GBM_TRUE; /* no error */
}

static void PNGAPI png_gbm_flush_data(png_structp png_ptr)
{
   /* We are using low level style function. No flushing necessary. */
   png_ptr = png_ptr; /* prevent compiler warning */
}

/* ----------------------------------------------------------- */

/* Replace standard libpng error functions by our own that prevent
   output to stdout and fatal handling.
   The replacement will be used temporarily during file I/O and sets
   an error code to the private IO struct.
*/
static void PNGAPI png_gbm_error(png_structp png_ptr, png_const_charp error_msg)
{
   PNG_PRIV_IO *io_p = (PNG_PRIV_IO *) png_get_io_ptr(png_ptr); /* io_ptr is pointer to private IO struct */
   io_p->errok = GBM_FALSE; /* error occured */

   longjmp(png_ptr->jmpbuf, 1);

   error_msg = error_msg; /* prevent compiler warning */
}

static void PNGAPI png_gbm_warning(png_structp png_ptr, png_const_charp warning_msg)
{
   png_ptr     = png_ptr;     /* prevent compiler warning */
   warning_msg = warning_msg; /* prevent compiler warning */
}

/* ----------------------------------------------------------- */

/* register customized read functions for libpng */
static void register_read_mapping(png_structp read_ptr)
{
    png_set_read_fn(read_ptr, png_get_io_ptr(read_ptr), png_gbm_read_data);
}

/* register customized write functions for libpng */
static void register_write_mapping(png_structp write_ptr)
{
    png_set_write_fn(write_ptr, png_get_io_ptr(write_ptr), png_gbm_write_data, png_gbm_flush_data);
}

/* ----------------------------------------------------------- */

/* unregister customized read functions for libpng and reset to the default */
static void unregister_read_mapping(png_structp read_ptr)
{
    png_set_read_fn(read_ptr, png_get_io_ptr(read_ptr), NULL);
}

/* unregister customized write functions for libpng and reset to the default */
static void unregister_write_mapping(png_structp write_ptr)
{
    png_set_write_fn(write_ptr, png_get_io_ptr(write_ptr), NULL, NULL);
}

/* ----------------------------------------------------------- */

/* Initialize libpng structures for reading
   Returns GBM_TRUE on success, else GBM_FALSE
*/
static gbm_boolean png_read_init(PNG_PRIV_READ *png_priv, int fd)
{
    png_priv->png_ptr  = NULL;
    png_priv->info_ptr = NULL;
    png_priv->end_info = NULL;

    png_priv->io.file.ahead = NULL;
    png_priv->io.errok      = GBM_FALSE;

    /* Create and initialize the png_struct with the desired error handler
     * functions.
     * We also supply the the compiler header file version, so that we know
     * if the application was compiled with a compatible version of the library.
    */
    png_priv->png_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING,
                                                 NULL, png_gbm_error , png_gbm_warning,
                                                 NULL, png_gbm_malloc, png_gbm_free);
    if (png_priv->png_ptr == NULL)
    {
        return GBM_FALSE;
    }

    png_priv->info_ptr = png_create_info_struct(png_priv->png_ptr);
    if (png_priv->info_ptr == NULL)
    {
        png_destroy_read_struct(&(png_priv->png_ptr), NULL, NULL);
        png_priv->png_ptr = NULL;
        return GBM_FALSE;
    }

    png_priv->end_info = png_create_info_struct(png_priv->png_ptr);
    if (png_priv->end_info == NULL)
    {
        png_destroy_read_struct(&(png_priv->png_ptr), &(png_priv->info_ptr), NULL);
        png_priv->png_ptr  = NULL;
        png_priv->info_ptr = NULL;
        return GBM_FALSE;
    }

    png_priv->io.file.ahead = gbm_create_ahead(fd);
    if (png_priv->io.file.ahead == NULL)
    {
        png_destroy_read_struct(&(png_priv->png_ptr), &(png_priv->info_ptr), NULL);
        png_priv->png_ptr  = NULL;
        png_priv->info_ptr = NULL;
        return GBM_FALSE;
    }

    png_priv->io.errok = GBM_TRUE;
    png_priv->png_ptr->io_ptr = &png_priv->io;

    /* set up the input control */
    register_read_mapping(png_priv->png_ptr);

    return GBM_TRUE;
}

/* Cleanup libpng structs for reading. */
static void png_read_deinit(PNG_PRIV_READ *png_priv)
{
    /* set default input control */
    unregister_read_mapping(png_priv->png_ptr);

    if (png_priv->io.file.ahead != NULL)
    {
       gbm_destroy_ahead(png_priv->io.file.ahead);
       png_priv->io.file.ahead = NULL;
    }

    png_priv->png_ptr->io_ptr = NULL;

    if ((png_priv->png_ptr  != NULL) ||
        (png_priv->info_ptr != NULL) ||
        (png_priv->end_info != NULL))
    {
       /* set error handler to prevent recursion due to unintended multiple calls */
       if (png_priv->png_ptr != NULL)
       {
          if (setjmp(png_priv->png_ptr->jmpbuf))
          {
             /* If we get here, we had a problem in freeing the info structs */
             png_priv->png_ptr  = NULL;
             png_priv->info_ptr = NULL;
             png_priv->end_info = NULL;
             return;
          }
       }
       png_destroy_read_struct(&(png_priv->png_ptr), &(png_priv->info_ptr), &(png_priv->end_info));
    }
    png_priv->png_ptr  = NULL;
    png_priv->info_ptr = NULL;
    png_priv->end_info = NULL;
    png_priv->io.errok = GBM_FALSE;
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* Initialize libpng structures for writing
   Returns GBM_TRUE on success, else GBM_FALSE
*/
static gbm_boolean png_write_init(PNG_PRIV_WRITE *png_priv, int fd)
{
    png_priv->png_ptr     = NULL;
    png_priv->info_ptr    = NULL;
    png_priv->palette_ptr = NULL;

    png_priv->io.file.wcache = NULL;
    png_priv->io.errok       = GBM_FALSE;

    /* Create and initialize the png_struct with the desired error handler
     * functions.
     * We also supply the the compiler header file version, so that we know
     * if the application was compiled with a compatible version of the library.
    */
    png_priv->png_ptr = png_create_write_struct_2(PNG_LIBPNG_VER_STRING,
                                                  NULL, png_gbm_error , png_gbm_warning,
                                                  NULL, png_gbm_malloc, png_gbm_free);
    if (png_priv->png_ptr == NULL)
    {
        return GBM_FALSE;
    }

    png_priv->info_ptr = png_create_info_struct(png_priv->png_ptr);
    if (png_priv->info_ptr == NULL)
    {
        png_destroy_write_struct(&(png_priv->png_ptr), NULL);
        png_priv->png_ptr = NULL;
        return GBM_FALSE;
    }

    png_priv->io.file.wcache = gbm_create_wcache(fd);
    if (png_priv->io.file.wcache == NULL)
    {
        png_destroy_write_struct(&(png_priv->png_ptr), NULL);
        png_priv->png_ptr = NULL;
        return GBM_FALSE;
    }

    png_priv->io.errok = GBM_TRUE;
    png_priv->png_ptr->io_ptr = &png_priv->io;

    /* set up the ouput control */
    register_write_mapping(png_priv->png_ptr);

    return GBM_TRUE;
}

/* Cleanup libpng structs for writing. */
static void png_write_deinit(PNG_PRIV_WRITE *png_priv)
{
    if (png_priv->io.file.wcache != NULL)
    {
       /* flush the rest of the write cache to file */
       gbm_destroy_wcache(png_priv->io.file.wcache);
       png_priv->io.file.wcache = NULL;
    }

    /* set default input control */
    unregister_write_mapping(png_priv->png_ptr);

    /* free palette if allocated */
    if (png_priv->palette_ptr != NULL)
    {
       png_free(png_priv->png_ptr, png_priv->palette_ptr);
       png_priv->palette_ptr = NULL;
    }

    png_priv->png_ptr->io_ptr = NULL;

    if ((png_priv->png_ptr  != NULL) ||
        (png_priv->info_ptr != NULL))
    {
       /* set error handler to prevent recursion due to unintended multiple calls */
       if (png_priv->png_ptr != NULL)
       {
          if (setjmp(png_priv->png_ptr->jmpbuf))
          {
             /* If we get here, we had a problem in freeing the info structs */
             png_priv->png_ptr  = NULL;
             png_priv->info_ptr = NULL;
             return;
          }
       }
       png_destroy_write_struct(&(png_priv->png_ptr), &(png_priv->info_ptr));
    }
    png_priv->png_ptr  = NULL;
    png_priv->info_ptr = NULL;
    png_priv->io.errok = GBM_FALSE;
}


/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* png_qft - Query format informations */
GBM_ERR png_qft(GBMFT *gbmft)
{
    *gbmft = png_gbmft;
    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */

/* internal_png_rhdr - Read file header to init GBM struct */
static GBM_ERR internal_png_rhdr(int fd, GBM *gbm)
{
    PNG_PRIV_READ *png_priv = (PNG_PRIV_READ *) gbm->priv;

    PNG_PRIV_IO   *io_p     = NULL;
    png_structp    png_ptr  = NULL;
    png_infop      info_ptr = NULL;

    png_color_16p image_background;

    const char *s;
    char signature[GBM_PNG_BYTES_TO_CHECK];

    png_uint_32 width, height;

    int ch_bit_depth, interlace_type, compression_type, filter_type;
    int channel_count, intent;

    double image_gamma  = GBM_PNG_DEFAULT_IMAGE_GAMMA;
    double screen_gamma = GBM_PNG_DEFAULT_SCREEN_GAMMA;

    gbm_boolean use_native_bpp = GBM_FALSE;
    gbm_boolean ignore_back    = GBM_FALSE;

    /* As we read the header multiple times, set file pointer to start. */
    if (gbm_file_lseek(fd, 0L, GBM_SEEK_SET) != 0)
    {
       return GBM_ERR_READ;
    }

    /* fast check for correct signature */

    /* Read in some of the signature bytes */
    if (gbm_file_read(fd, signature, GBM_PNG_BYTES_TO_CHECK) != GBM_PNG_BYTES_TO_CHECK)
    {
       return GBM_ERR_BAD_MAGIC;
    }

    /* Compare the first GBM_PNG_BYTES_TO_CHECK bytes of the signature. */
    if (png_sig_cmp(signature, 0, GBM_PNG_BYTES_TO_CHECK))
    {
       return GBM_ERR_BAD_MAGIC;
    }

    /* Initialize libpng structures */
    if (png_read_init(png_priv, fd) == GBM_FALSE)
    {
       return GBM_ERR_READ;
    }

    png_ptr  = png_priv->png_ptr;
    info_ptr = png_priv->info_ptr;

    /* set error handling */
    if (setjmp(png_ptr->jmpbuf))
    {
        /* If we get here, we had a problem reading the file */
        png_read_deinit(png_priv);
        return GBM_ERR_READ;
    }

    io_p = (PNG_PRIV_IO *) png_get_io_ptr(png_ptr); /* io_ptr is pointer to private IO struct */

    /* let libpng know how many signature bytes were already read */
    png_set_sig_bytes(png_ptr, GBM_PNG_BYTES_TO_CHECK);

    /* read the file information */
    png_read_info(png_ptr, info_ptr);
    if (io_p->errok != GBM_TRUE)
    {
       png_read_deinit(png_priv);
       return GBM_ERR_READ;
    }

    /* extract header infos */
    if (! png_get_IHDR(png_ptr, info_ptr, &width, &height, &ch_bit_depth,
                       &png_priv->color_type, &interlace_type, &compression_type, &filter_type))
    {
       png_read_deinit(png_priv);
       return GBM_ERR_PNG_HEADER;
    }

    /* limit size to INT_MAX values (width, height) */
    if ((width > INT_MAX) || (height > INT_MAX))
    {
        png_read_deinit(png_priv);
        return GBM_ERR_BAD_SIZE;
    }

    /* check if extended color depths are requested */
    use_native_bpp = (gbm_find_word(png_priv->read_options, "ext_bpp") != NULL)
                     ? GBM_TRUE : GBM_FALSE;

    /* check if a background is to be ignored */
    ignore_back = (gbm_find_word(png_priv->read_options, "ignore_back") != NULL)
                  ? GBM_TRUE : GBM_FALSE;

    gbm->w = (int) width;
    gbm->h = (int) height;

    channel_count = png_get_channels(png_ptr, info_ptr);

    /* check for palette availability based on the color type */
    png_priv->upsamplePaletteToPalette = GBM_FALSE;
    png_priv->unassociatedAlpha = GBM_FALSE;

    /* program libpng transformations */
    switch(png_priv->color_type)
    {
      case PNG_COLOR_TYPE_GRAY:
         if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
         {
            /* expand transparent gray values to full alpha channel */
            png_set_tRNS_to_alpha(png_ptr);
            png_set_gray_to_rgb(png_ptr);
            png_priv->color_type = PNG_COLOR_TYPE_RGB_ALPHA;
         }
         switch(ch_bit_depth)
         {
            case 1:
            case 2:
            case 4:
            case 8:
               break;

            case 16:
               /* GBM does not support 16bit palette, so convert to RGB */
               png_set_swap(png_ptr);
               if (png_priv->color_type == PNG_COLOR_TYPE_GRAY)
               {
                  png_set_gray_to_rgb(png_ptr);
                  png_priv->color_type = PNG_COLOR_TYPE_RGB;
               }
               break;

            default:
               png_read_deinit(png_priv);
               return GBM_ERR_PNG_BPP;
         }
         break;

      /* ------------------------------------------------------- */

      case PNG_COLOR_TYPE_GRAY_ALPHA:
         switch(ch_bit_depth)
         {
            case 16:
               png_set_swap(png_ptr);
               /* fall through here */
            case 8:
               /* GBM does not support this, so convert to RGB */
               png_set_gray_to_rgb(png_ptr);
               png_priv->color_type = PNG_COLOR_TYPE_RGB_ALPHA;
               break;

            default:
               png_read_deinit(png_priv);
               return GBM_ERR_PNG_BPP;
         }
         break;

      /* ------------------------------------------------------- */

      case PNG_COLOR_TYPE_PALETTE:
         if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
         {
            /* expand transparent colors to full alpha channel */
            png_set_tRNS_to_alpha(png_ptr);
            png_set_palette_to_rgb(png_ptr);
            png_priv->color_type = PNG_COLOR_TYPE_RGB_ALPHA;
         }
         switch(ch_bit_depth)
         {
            case 1:
            case 2:
            case 4:
            case 8:
               break;

            default:
               png_read_deinit(png_priv);
               return GBM_ERR_PNG_BPP;
         }
         break;

      /* ------------------------------------------------------- */

      case PNG_COLOR_TYPE_RGB:
         if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
         {
            /* expand transparent gray values to full alpha channel */
            png_set_tRNS_to_alpha(png_ptr);
            png_priv->color_type = PNG_COLOR_TYPE_RGB_ALPHA;
         }
         switch(ch_bit_depth)
         {
            case  8: /* this is 24 bit/pixel */
               break;

            case 16: /* this is 48 bit/pixel */
               png_set_swap(png_ptr);
               break;

            default:
               png_read_deinit(png_priv);
               return GBM_ERR_PNG_BPP;
         }
         break;

      /* ------------------------------------------------------- */

      case PNG_COLOR_TYPE_RGB_ALPHA:
         switch(ch_bit_depth)
         {
            case  8: /* this is 32 bit/pixel */
               break;

            case 16: /* this is 64 bit/pixel */
               png_set_swap(png_ptr);
               break;

            default:
               png_read_deinit(png_priv);
               return GBM_ERR_PNG_BPP;
         }
         break;

      /* ------------------------------------------------------- */

      default:
         png_read_deinit(png_priv);
         return GBM_ERR_PNG_BPP;
    }

    /* get background if there is one in the file */
    png_priv->backrgb.r =
    png_priv->backrgb.g =
    png_priv->backrgb.b = 0;

    /* a background provided with the image is the master */
    if (! ignore_back)
    {
       if (png_get_bKGD(png_ptr, info_ptr, &image_background))
       {
          /* Scale color to 16-bit values. */
          #define SCALE(x) ((((gbm_u16) x) * ((1L << 16) - 1)) / 255)

          if (ch_bit_depth < 16)
          {
             png_priv->backrgb.r = SCALE(image_background->red);
             png_priv->backrgb.g = SCALE(image_background->green);
             png_priv->backrgb.b = SCALE(image_background->blue);
          }
          else
          {
             png_priv->backrgb.r = image_background->red;
             png_priv->backrgb.g = image_background->green;
             png_priv->backrgb.b = image_background->blue;
          }

          /* if a background color exists, the alpha channel gets associated
           * and will not be returned separately
           */
          png_priv->unassociatedAlpha = GBM_TRUE;
       }

       /* parse RGB value for background mixing with alpha channel */
       if ((s = gbm_find_word_prefix(png_priv->read_options, "back_rgb=")) != NULL)
       {
          int image_background_red   = 0;
          int image_background_green = 0;
          int image_background_blue  = 0;

          if (sscanf(s + 9, "%d_%d_%d", &image_background_red,
                                        &image_background_green,
                                        &image_background_blue) != 3)
          {
             png_read_deinit(png_priv);
             return GBM_ERR_BAD_OPTION;
          }

          if (((image_background_red   < 0) || (image_background_red   > 0xffff)) ||
              ((image_background_green < 0) || (image_background_green > 0xffff)) ||
              ((image_background_blue  < 0) || (image_background_blue  > 0xffff)))
          {
             png_read_deinit(png_priv);
             return GBM_ERR_BAD_OPTION;
          }

          png_priv->backrgb.r = image_background_red;
          png_priv->backrgb.g = image_background_green;
          png_priv->backrgb.b = image_background_blue;

          /* if a background color exists, the alpha channel gets associated
           * and will not be returned separately
           */
          png_priv->unassociatedAlpha = GBM_TRUE;
       }
    }

    /* parse screen gamma value if provided */
    image_gamma  = GBM_PNG_DEFAULT_IMAGE_GAMMA;
    screen_gamma = GBM_PNG_DEFAULT_SCREEN_GAMMA;
    if ((s = gbm_find_word_prefix(png_priv->read_options, "gamma=")) != NULL)
    {
       if (sscanf(s + 6, "%lf", &screen_gamma) != 1)
       {
          png_read_deinit(png_priv);
          return GBM_ERR_BAD_OPTION;
       }

       if (screen_gamma < 0.0)
       {
          png_read_deinit(png_priv);
          return GBM_ERR_BAD_OPTION;
       }
    }

    /* set screen gamma correction */
    if (png_get_sRGB(png_ptr, info_ptr, &intent))
    {
       png_set_gamma(png_ptr, screen_gamma, image_gamma);
    }
    else
    {
       /* try to get from image, otherwise use default or override */
       png_get_gAMA (png_ptr, info_ptr    , &image_gamma);
       png_set_gamma(png_ptr, screen_gamma,  image_gamma);
    }

    /* define other transformations */
    if (interlace_type != PNG_INTERLACE_NONE)
    {
       /* let libpng handle interlace expansion */
       png_set_interlace_handling(png_ptr);
    }

    /* update info struct due to transformations (palette is also updated) */
    png_read_update_info(png_ptr, info_ptr);

    if (io_p->errok != GBM_TRUE)
    {
       png_read_deinit(png_priv);
       return GBM_ERR_READ;
    }

    /* ----------------------------------------------- */
    /* So now reread what we get after transformations */
    /* ----------------------------------------------- */

    /* extract header infos */
    if (! png_get_IHDR(png_ptr, info_ptr, &width, &height, &ch_bit_depth,
                       &png_priv->color_type, &interlace_type, &compression_type, &filter_type))
    {
       png_read_deinit(png_priv);
       return GBM_ERR_PNG_HEADER;
    }

    /* check if color depth is supported */
    channel_count = png_get_channels(png_ptr, info_ptr);

    /* convert bit_depth from channel bit depth to overall bit depth */
    png_priv->bpp_src = ch_bit_depth * channel_count;

    switch(png_priv->color_type)
    {
      case PNG_COLOR_TYPE_GRAY:
         switch(ch_bit_depth)
         {
            case 1:
            case 4:
            case 8:
               break;

            case 2:
               /* the import of 2 bpp files is already supported (use_native_bpp) */
               /*  but we always upsample to 4 bpp to simplify external usage     */
               png_priv->upsamplePaletteToPalette = GBM_TRUE;
               ch_bit_depth  = 4;
               break;

            case 16:
               /* GBM does not support 16bit palette, so convert to RGB */
               /* upsample to 24/48 bpp */
               ch_bit_depth = use_native_bpp ? 16 : 8;
               break;

            default:
               png_read_deinit(png_priv);
               return GBM_ERR_PNG_BPP;
         }
         break;

      /* ------------------------------------------------------- */

      case PNG_COLOR_TYPE_PALETTE:
         switch(ch_bit_depth)
         {
            case 1:
            case 4:
            case 8:
               break;

            case 2:
               /* the import of 2 bpp files is already supported (use_native_bpp) */
               /*  but we always upsample to 4 bpp to simplify external usage     */
               png_priv->upsamplePaletteToPalette = GBM_TRUE;
               ch_bit_depth = 4;
               break;

            default:
               png_read_deinit(png_priv);
               return GBM_ERR_PNG_BPP;
         }
         break;

      /* ------------------------------------------------------- */

      case PNG_COLOR_TYPE_RGB:
         switch(ch_bit_depth)
         {
            case  8: /* this is 24 bit/pixel */
            case 16: /* this is 48 bit/pixel */
               ch_bit_depth = use_native_bpp ? ch_bit_depth : 8;
               break;

            default:
               png_read_deinit(png_priv);
               return GBM_ERR_PNG_BPP;
         }
         break;

      /* ------------------------------------------------------- */

      case PNG_COLOR_TYPE_RGB_ALPHA:
         switch(ch_bit_depth)
         {
            case  8: /* this is 32 bit/pixel */
            case 16: /* this is 64 bit/pixel */
               ch_bit_depth = use_native_bpp ? ch_bit_depth  : 8;

               if (png_priv->unassociatedAlpha)
               {
                  channel_count = 3;
               }
               else
               {
                  if (! use_native_bpp)
                  {
                     channel_count = 3;

                     /* Background color is not set. Thus render against
                      * a white background instead of simply stripping the
                      * alpha channel during downconversion.
                      */
                     png_priv->backrgb.r =
                     png_priv->backrgb.g =
                     png_priv->backrgb.b = 65535;
                     png_priv->unassociatedAlpha = GBM_TRUE;
                  }
               }
               break;

            default:
               png_read_deinit(png_priv);
               return GBM_ERR_PNG_BPP;
         }
         break;

      /* ------------------------------------------------------- */

      default:
         png_read_deinit(png_priv);
         return GBM_ERR_PNG_BPP;
    }

    /* convert bit_depth from channel bit depth to overall bit depth */
    gbm->bpp = ch_bit_depth * channel_count;

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
             png_read_deinit(png_priv);
             return GBM_ERR_PNG_BPP;
          }
          break;

       default:
           png_read_deinit(png_priv);
           return GBM_ERR_PNG_BPP;
    }

    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */

/* png_rhdr - Read file header to init GBM struct */
GBM_ERR png_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
{
    PNG_PRIV_READ *png_priv = (PNG_PRIV_READ *) gbm->priv;
    GBM_ERR        rc;

    fn = fn; /* prevent compiler warning */

    /* init options buffer */
    memset(png_priv->read_options, 0, sizeof(png_priv->read_options));

    /* copy possible options */
    if (strlen(opt) >= sizeof(png_priv->read_options))
    {
       return GBM_ERR_BAD_OPTION;
    }
    strcpy(png_priv->read_options, opt);

    rc = internal_png_rhdr(fd, gbm);
    if (rc != GBM_ERR_OK)
    {
       return rc;
    }

    /* We cannot expect the client calls rhdr,rpal,rdata in order.
     * The client might just read the header and stop then.
     *
     * There is no other choice than freeing the libpng structs
     * and reread them in the rdata function.
     */
    png_read_deinit((PNG_PRIV_READ *) gbm->priv);

    /* Don't override the read_options buffer as it will be
     * readout by internal_png_rhdr().
     */

    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* internal_png_rpal_8bpp() - Read 8bpp palette */
static GBM_ERR internal_png_rpal_8bpp(GBM *gbm, GBMRGB *gbmrgb)
{
    PNG_PRIV_READ *png_priv = (PNG_PRIV_READ *) gbm->priv;

    if (gbm->bpp <= 8)
    {
       int i, x;

       /* get the palette information */
       const int palette_entries = 1 << png_priv->bpp_src;
       int increment;

       /* init palette */
       memset(gbmrgb, 0, (sizeof(GBMRGB) * (1 << gbm->bpp)));

       switch(png_priv->bpp_src)
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
             increment = 1;
             break;

          default:
             png_read_deinit(png_priv);
             return GBM_ERR_READ;
       }

       switch(png_priv->color_type)
       {
          case PNG_COLOR_TYPE_GRAY:
          case PNG_COLOR_TYPE_GRAY_ALPHA:
             i = palette_entries;
             x = GBM_PNG_NUM_COLORS - 1;
             while (i > 0)
             {
                i--;
                gbmrgb[i].r = gbmrgb[i].g = gbmrgb[i].b = x;
                x -= increment;
             }
             break;

          case PNG_COLOR_TYPE_PALETTE:
          case PNG_COLOR_TYPE_RGB:
          case PNG_COLOR_TYPE_RGB_ALPHA:
          {
             /* get the palette information */
             png_structp  png_ptr   = png_priv->png_ptr;
             png_infop    info_ptr  = png_priv->info_ptr;
             png_colorp   palette_p = NULL;

             /* get the palette information */
             int png_palette_entries = 0;
             int i;

             if (! png_get_PLTE(png_ptr, info_ptr, &palette_p, &png_palette_entries))
             {
                if (png_priv->color_type != PNG_COLOR_TYPE_PALETTE)
                {
                   /* palette is optional, so don't generate an error */
                   return GBM_ERR_OK;
                }
                png_read_deinit(png_priv);
                return GBM_ERR_READ;
             }

             /* restrict palette entries to gbm specs */
             if (png_palette_entries > palette_entries)
             {
                png_palette_entries = palette_entries;
             }

             /* copy the palette entries */
             i = png_palette_entries;
             while (i > 0)
             {
                i--;
                gbmrgb[i].r = palette_p[i].red;
                gbmrgb[i].g = palette_p[i].green;
                gbmrgb[i].b = palette_p[i].blue;
             }
          }
          break;

          default:
             png_read_deinit(png_priv);
             return GBM_ERR_READ;
       }
    }

    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */

/* png_rpal()  -  Read palette */
GBM_ERR png_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
{
    PNG_PRIV_READ *png_priv = (PNG_PRIV_READ *) gbm->priv;

    /* read the header again */
    GBM_ERR rc = internal_png_rhdr(fd, gbm);
    if (rc != GBM_ERR_OK)
    {
       /* cleanup is done in the called function already */
       return rc;
    }

    /* copy the palette */
    rc = internal_png_rpal_8bpp(gbm, gbmrgb);
    if (rc != GBM_ERR_OK)
    {
       /* cleanup is done in the called function already */
       return rc;
    }

    /* We cannot expect the client calls rhdr,rpal,rdata in order.
     * The client might just read the header and stop then.
     *
     * There is no other choice than freeing the libpng structs
     * and reread them in the rdata function.
     */
    png_read_deinit(png_priv);

    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* png_rdata()  -  Read data */

GBM_ERR png_rdata(int fd, GBM *gbm, gbm_u8 *data)
{
    /* read the header again */
    GBM_ERR rc = internal_png_rhdr(fd, gbm);
    if (rc != GBM_ERR_OK)
    {
       /* cleanup is done in the called function already */
       return rc;
    }
    else
    {
       PNG_PRIV_READ *png_priv = (PNG_PRIV_READ *) gbm->priv;
       png_structp    png_ptr  = png_priv->png_ptr;
       png_infop      info_ptr = png_priv->info_ptr,
                      end_info = png_priv->end_info;
       PNG_PRIV_IO   *io_p     = (PNG_PRIV_IO *) png_get_io_ptr(png_priv->png_ptr);

       png_bytep *gbm_row_pointers = NULL;
       int row, gbm_row_bytes, src_row_bytes;

       /* Read the image. Attn: Align to 32 bit rows for GBM !!! */
       src_row_bytes    = png_get_rowbytes(png_ptr, info_ptr);
       gbm_row_bytes    = ((gbm->w * gbm->bpp + 31)/32) * 4;
       gbm_row_pointers = (png_bytep*) gbmmem_malloc(gbm->h * sizeof(png_bytep));
       if (gbm_row_pointers == NULL)
       {
           png_read_deinit(png_priv);
           return GBM_ERR_MEM;
       }

       /* set error handling */
       if (setjmp(png_ptr->jmpbuf))
       {
           /* If we get here, we had a problem reading the file */
           gbmmem_free(gbm_row_pointers);
           png_read_deinit(png_priv);
           return GBM_ERR_READ;
       }

       /* init pointers from bottom to top because the image is internally a DIB
        * which is mirrored vertically
        */
       gbm_row_pointers[0] = ((png_bytep) data) + (gbm->h * gbm_row_bytes) - gbm_row_bytes;

       for (row = 1; row < gbm->h; row++)
       {
           gbm_row_pointers[row] = gbm_row_pointers[row-1] - gbm_row_bytes;
       }

       /* Now it's time to read the image. */
       if ((gbm->bpp <= 8) && (! png_priv->upsamplePaletteToPalette))
       {
          /* read via high level interface */
          png_read_image(png_ptr, gbm_row_pointers);
       }
       else if (png_priv->upsamplePaletteToPalette)
       {
          GBM         gbm_src          = *gbm;
          gbm_u8    * src_buffer       = NULL;
          png_bytep * src_row_pointers = NULL;

          gbm_src.bpp = png_priv->bpp_src;

          /* If the source format is different then the target format  */
          /* we have to do post processing in a separate image buffer. */
          /* Due to the way interlace handling must be done, we can't  */
          /* just read row after row and do the conversion on the fly. */
          /* We must read the whole image first and then convert. Really bad. */
          src_buffer = (gbm_u8 *) gbmmem_malloc(src_row_bytes * gbm_src.h);
          if (src_buffer == NULL)
          {
             gbmmem_free(gbm_row_pointers);
             png_read_deinit(png_priv);
             return GBM_ERR_MEM;
          }

          /* allocate pointer table for the source rows */
          src_row_pointers = (png_bytep*) gbmmem_malloc(gbm_src.h * sizeof(png_bytep));
          if (src_row_pointers == NULL)
          {
             gbmmem_free(src_row_pointers);
             gbmmem_free(src_buffer);
             gbmmem_free(gbm_row_pointers);
             png_read_deinit(png_priv);
             return GBM_ERR_MEM;
          }

          /* init pointer table for the source rows */
          src_row_pointers[0] = (png_bytep) src_buffer;
          for (row = 1; row < gbm_src.h; row++)
          {
             src_row_pointers[row] = src_row_pointers[row-1] + src_row_bytes;
          }

          /* set error handling */
          if (setjmp(png_ptr->jmpbuf))
          {
             /* If we get here, we had a problem reading the file */
             gbmmem_free(src_row_pointers);
             gbmmem_free(src_buffer);
             gbmmem_free(gbm_row_pointers);
             png_read_deinit(png_priv);
             return GBM_ERR_READ;
          }

          /* read via high level interface */
          png_read_image(png_ptr, src_row_pointers);

          for (row = 0; row < gbm->h; row++)
          {
             if (! gbm_map_row_PAL_PAL(src_row_pointers[row], &gbm_src,
                                       gbm_row_pointers[row], gbm))
             {
                gbmmem_free(src_row_pointers);
                gbmmem_free(src_buffer);
                gbmmem_free(gbm_row_pointers);
                png_read_deinit(png_priv);
                return GBM_ERR_READ;
             }
          }

          gbmmem_free(src_row_pointers);
          gbmmem_free(src_buffer);
       }
       else /* 24 Bit or higher */
       {
          GBM           gbm_src = *gbm;
          gbm_src.bpp = png_priv->bpp_src;

          /* use temporary one for downsampling */
          if (gbm_src.bpp != gbm->bpp)
          {
             gbm_u8    * src_buffer       = NULL;
             png_bytep * src_row_pointers = NULL;

             /* If the source format is different then the target format  */
             /* we have to do post processing in a separate image buffer. */
             /* Due to the way interlace handling must be done, we can't  */
             /* just read row after row and do the conversion on the fly. */
             /* We must read the whole image first and then convert. Really bad. */
             src_buffer = (gbm_u8 *) gbmmem_malloc(src_row_bytes * gbm_src.h);
             if (src_buffer == NULL)
             {
                gbmmem_free(gbm_row_pointers);
                png_read_deinit(png_priv);
                return GBM_ERR_MEM;
             }

             /* allocate pointer table for the source rows */
             src_row_pointers = (png_bytep*) gbmmem_malloc(gbm_src.h * sizeof(png_bytep));
             if (src_row_pointers == NULL)
             {
                gbmmem_free(src_row_pointers);
                gbmmem_free(src_buffer);
                gbmmem_free(gbm_row_pointers);
                png_read_deinit(png_priv);
                return GBM_ERR_MEM;
             }

             /* init pointer table for the source rows */
             src_row_pointers[0] = (png_bytep) src_buffer;
             for (row = 1; row < gbm_src.h; row++)
             {
                src_row_pointers[row] = src_row_pointers[row-1] + src_row_bytes;
             }

             /* set error handling */
             if (setjmp(png_ptr->jmpbuf))
             {
                /* If we get here, we had a problem reading the file */
                gbmmem_free(src_row_pointers);
                gbmmem_free(src_buffer);
                gbmmem_free(gbm_row_pointers);
                png_read_deinit(png_priv);
                return GBM_ERR_READ;
             }

             /* read via high level interface */
             png_read_image(png_ptr, src_row_pointers);

             for (row = 0; row < gbm->h; row++)
             {
                if (! gbm_map_row_RGBx_BGRx(src_row_pointers[row], &gbm_src,
                                            gbm_row_pointers[row],  gbm,
                                            &png_priv->backrgb   ,  png_priv->unassociatedAlpha))
                {
                   gbmmem_free(src_row_pointers);
                   gbmmem_free(src_buffer);
                   gbmmem_free(gbm_row_pointers);
                   png_read_deinit(png_priv);
                   return GBM_ERR_READ;
                }
             }
             gbmmem_free(src_row_pointers);
             gbmmem_free(src_buffer);
          }
          else /* copy directly */
          {
             png_read_image(png_ptr, gbm_row_pointers);
             for (row = 0; row < gbm->h; row++)
             {
                if (! gbm_map_row_RGBx_BGRx(gbm_row_pointers[row], &gbm_src,
                                            gbm_row_pointers[row],  gbm,
                                            &png_priv->backrgb   ,  png_priv->unassociatedAlpha))
                {
                   gbmmem_free(gbm_row_pointers);
                   png_read_deinit(png_priv);
                   return GBM_ERR_READ;
                }
             }
          }
       }

       gbmmem_free(gbm_row_pointers);

       /* close reading */
       png_read_end(png_ptr, end_info);

       if (io_p->errok != GBM_TRUE)
       {
          png_read_deinit(png_priv);
          return GBM_ERR_READ;
       }

       /* cleanup */
       png_read_deinit(png_priv);
    }

    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* png_w()  -  Write bitmap file */
GBM_ERR png_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
{
    PNG_PRIV_WRITE png_priv;

    png_structp  png_ptr  = NULL;
    png_infop    info_ptr = NULL;
    PNG_PRIV_IO *io_p     = NULL;

    int color_type   = -1;
    int ch_bit_depth = 0;

    gbm_boolean set_image_interlace = GBM_FALSE;
    gbm_boolean set_background      = GBM_FALSE;
    gbm_boolean set_transparency    = GBM_FALSE;
    gbm_boolean has_alpha_channel   = GBM_FALSE;

    png_color_16 my_background = { 0, 0, 0, 0, 0 };

    const char *s;

    fn=fn; /* Suppress 'unref arg' compiler warnings */

    /* check for height processing limitations */
    if (gbm->h > PNG_UINT_32_MAX/png_sizeof(png_bytep))
    {
       return GBM_ERR_WRITE;
    }

    /* parse interlace option if provided */
    if (gbm_find_word(opt, "ilace") != NULL)
    {
        set_image_interlace = GBM_TRUE;
    }

    /* Initialize libpng structures */
    if (png_write_init(&png_priv, fd) == GBM_FALSE)
    {
       return GBM_ERR_WRITE;
    }

    png_ptr  = png_priv.png_ptr;
    info_ptr = png_priv.info_ptr;
    io_p     = (PNG_PRIV_IO *) png_get_io_ptr(png_ptr);

    /* set error handling */
    if (setjmp(png_ptr->jmpbuf))
    {
        /* If we get here, we had a problem writing the file */
        png_write_deinit(&png_priv);
        return GBM_ERR_WRITE;
    }

    switch(gbm->bpp)
    {
       case 1:
          ch_bit_depth = gbm->bpp;

          /* detect B/W (GRAY) */
          color_type = PNG_COLOR_TYPE_PALETTE;
          if ((gbmrgb[0].r == gbmrgb[0].g) && (gbmrgb[0].r == gbmrgb[0].b) && (gbmrgb[0].g == gbmrgb[0].b) &&
              (gbmrgb[1].r == gbmrgb[1].g) && (gbmrgb[1].r == gbmrgb[1].b) && (gbmrgb[1].g == gbmrgb[1].b))
          {
             if ((gbmrgb[0].r == 0) && (gbmrgb[1].r == 0xff))
             {
                color_type = PNG_COLOR_TYPE_GRAY;
             }
          }
          break;

       case 4:
       case 8:
          ch_bit_depth = gbm->bpp;
          color_type   = PNG_COLOR_TYPE_PALETTE;
          break;

       case 24:
       case 48:
          ch_bit_depth = gbm->bpp / 3;
          color_type   = PNG_COLOR_TYPE_RGB;
          break;

       case 32:
       case 64:
          has_alpha_channel = GBM_TRUE;
          ch_bit_depth      = gbm->bpp / 4;
          color_type        = PNG_COLOR_TYPE_RGB_ALPHA;
          break;

       default:
          png_write_deinit(&png_priv);
          return GBM_ERR_PNG_BPP;
    }

    /* define Info Header */
    switch(color_type)
    {
       case PNG_COLOR_TYPE_GRAY:
          /* no palette info needed */
          break;

       /* ----------------- */

       case PNG_COLOR_TYPE_PALETTE:
       {
          int i;
          const int palette_entries = 1 << gbm->bpp;

          /* set the palette, REQUIRED for indexed-color images */
          png_priv.palette_ptr = (png_colorp) png_malloc(png_ptr,
                                                         palette_entries * png_sizeof(png_color));

          /* copy gbm palette -> libpng palette */
          for (i = 0; i < palette_entries; i++)
          {
             png_priv.palette_ptr[i].red   = gbmrgb[i].r;
             png_priv.palette_ptr[i].green = gbmrgb[i].g;
             png_priv.palette_ptr[i].blue  = gbmrgb[i].b;
          }

          /* ... set palette colors ... */
          png_set_PLTE(png_ptr, info_ptr, png_priv.palette_ptr, palette_entries);
          /* You must not free palette here, because png_set_PLTE only makes a link to
           * the palette that you malloced.  Wait until you are about to destroy
           * the png structure.
           */
       }
       break;

       /* ----------------- */

       case PNG_COLOR_TYPE_RGB:
       case PNG_COLOR_TYPE_RGB_ALPHA:
          break;

       default:
          png_write_deinit(&png_priv);
          return GBM_ERR_PNG_BPP;
    }

    /* set header info */
    png_set_IHDR(png_ptr, info_ptr, gbm->w, gbm->h, ch_bit_depth, color_type,
                 set_image_interlace ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    /* ------------------------------------------------------ */

    /* Optional gamma chunk is strongly suggested if you have any guess
     * as to the correct gamma of the image.
     */
    if ((s = gbm_find_word_prefix(opt, "gamma=")) != NULL)
    {
        double image_gamma = GBM_PNG_DEFAULT_IMAGE_GAMMA;

        if (sscanf(s + 6, "%lf", &image_gamma) != 1)
        {
           png_write_deinit(&png_priv);
           return GBM_ERR_BAD_OPTION;
        }

        if (image_gamma < 0.0)
        {
           png_write_deinit(&png_priv);
           return GBM_ERR_BAD_OPTION;
        }

        png_set_gAMA(png_ptr, info_ptr, image_gamma);
    }

    /* ------------------------------------------------------ */

    /* parse transparency for palette images */
    if ((s = gbm_find_word_prefix(opt, "transparency=")) != NULL)
    {
        png_byte trans = 0;
        int image_transparency = 0;
        if (sscanf(s + 13, "%d", &image_transparency) != 1)
        {
           png_write_deinit(&png_priv);
           return GBM_ERR_BAD_OPTION;
        }

        if ((gbm->bpp > 8)    ||
            has_alpha_channel ||
            (image_transparency < 0) || (image_transparency >= (1 << gbm->bpp)))
        {
           png_write_deinit(&png_priv);
           return GBM_ERR_BAD_OPTION;
        }

        trans = (png_byte) (255 - image_transparency);

        png_set_tRNS(png_ptr, info_ptr, &trans, 1, NULL);
        set_transparency = GBM_TRUE;
    }

    /* parse RGB value for transparency */
    if ((s = gbm_find_word_prefix(opt, "transcol_rgb=")) != NULL)
    {
        int image_transcol_red   = 0;
        int image_transcol_green = 0;
        int image_transcol_blue  = 0;
        int colorMax             = 0;

        png_color_16 my_transparency = { 0, 0, 0, 0, 0 };

        /* check that the transparency is only defined once */
        if (set_transparency || has_alpha_channel)
        {
           png_write_deinit(&png_priv);
           return GBM_ERR_BAD_OPTION;
        }

        if (sscanf(s + 13, "%d_%d_%d", &image_transcol_red,
                                       &image_transcol_green,
                                       &image_transcol_blue) != 3)
        {
           png_write_deinit(&png_priv);
           return GBM_ERR_BAD_OPTION;
        }

        switch(gbm->bpp)
        {
           case 24:
           case 32:
              colorMax = 0xff;
              break;

           case 48:
           case 64:
              colorMax = 0xffff;
              break;

           default:
              png_write_deinit(&png_priv);
              return GBM_ERR_BAD_OPTION;
        }

        if (((image_transcol_red   < 0) || (image_transcol_red   > colorMax)) ||
            ((image_transcol_green < 0) || (image_transcol_green > colorMax)) ||
            ((image_transcol_blue  < 0) || (image_transcol_blue  > colorMax)))
        {
           png_write_deinit(&png_priv);
           return GBM_ERR_BAD_OPTION;
        }
        my_transparency.red   = image_transcol_red;
        my_transparency.green = image_transcol_green;
        my_transparency.blue  = image_transcol_blue;

        png_set_tRNS(png_ptr, info_ptr, NULL, 0, &my_transparency);
        set_transparency = GBM_TRUE;
    }

    /* ------------------------------------------------------ */

    /* parse palette index value for background mixing with alpha channel */
    if ((s = gbm_find_word_prefix(opt, "back_index=")) != NULL)
    {
        int image_background_index = 0;
        if (sscanf(s + 11, "%d", &image_background_index) != 1)
        {
           png_write_deinit(&png_priv);
           return GBM_ERR_BAD_OPTION;
        }

        if ((gbm->bpp > 8) ||
            (image_background_index < 0) || (image_background_index >= (1 << gbm->bpp)))
        {
           png_write_deinit(&png_priv);
           return GBM_ERR_BAD_OPTION;
        }

        if (color_type == PNG_COLOR_TYPE_GRAY)
        {
           my_background.gray = image_background_index;
        }
        else
        {
           my_background.index = image_background_index;
        }

        png_set_bKGD(png_ptr, info_ptr, &my_background);
        set_background = GBM_TRUE;
    }

    /* parse RGB value for background mixing with alpha channel */
    if ((s = gbm_find_word_prefix(opt, "back_rgb=")) != NULL)
    {
        int image_background_red   = 0;
        int image_background_green = 0;
        int image_background_blue  = 0;
        int colorMax               = 0;

        /* check that the background is only defined once */
        if (set_background)
        {
           png_write_deinit(&png_priv);
           return GBM_ERR_BAD_OPTION;
        }

        if (sscanf(s + 9, "%d_%d_%d", &image_background_red,
                                      &image_background_green,
                                      &image_background_blue) != 3)
        {
           png_write_deinit(&png_priv);
           return GBM_ERR_BAD_OPTION;
        }

        switch(gbm->bpp)
        {
           case 24:
           case 32:
              colorMax = 0xff;
              break;

           case 48:
           case 64:
              colorMax = 0xffff;
              break;

           default:
              png_write_deinit(&png_priv);
              return GBM_ERR_BAD_OPTION;
        }

        if (((image_background_red   < 0) || (image_background_red   > colorMax)) ||
            ((image_background_green < 0) || (image_background_green > colorMax)) ||
            ((image_background_blue  < 0) || (image_background_blue  > colorMax)))
        {
           png_write_deinit(&png_priv);
           return GBM_ERR_BAD_OPTION;
        }
        my_background.red   = image_background_red;
        my_background.green = image_background_green;
        my_background.blue  = image_background_blue;

        png_set_bKGD(png_ptr, info_ptr, &my_background);
        set_background = GBM_TRUE;
    }

    /* ------------------------------------------------------ */

    /* user defined compression level: forward to ZLIB */
    if ((s = gbm_find_word_prefix(opt, "compression=")) != NULL)
    {
        int image_compression = 6; /* default ZLIB compression */
        if (sscanf(s + 12, "%d", &image_compression) != 1)
        {
           png_write_deinit(&png_priv);
           return GBM_ERR_BAD_OPTION;
        }
        if ((image_compression < 0) || (image_compression > 9))
        {
           png_write_deinit(&png_priv);
           return GBM_ERR_BAD_OPTION;
        }

        png_set_compression_level(png_ptr, image_compression);
    }

    /* ------------------------------------------------------ */

    /* Write the file header information. */
    png_write_info(png_ptr, info_ptr);
    if (io_p->errok != GBM_TRUE)
    {
       png_write_deinit(&png_priv);
       return GBM_ERR_WRITE;
    }

    if ((color_type == PNG_COLOR_TYPE_RGB) || (color_type == PNG_COLOR_TYPE_RGB_ALPHA))
    {
       /* flip BGR pixels to RGB */
       png_set_bgr(png_ptr);

       if (gbm->bpp > 32)
       {
          /* set 16 bit channel data byte ordering to little endian */
          png_set_swap(png_ptr);
       }
    }

    /* write the image data */
    {
       int row;

       /* The easiest way to write the image. Attn: Align to 32 bit rows for GBM !!! */
       const int row_bytes     = (((gbm->w * gbm->bpp) + 31)/32) * 4;
       png_bytep *row_pointers = (png_bytep*) gbmmem_malloc(gbm->h * sizeof(png_bytep));

       /* init pointers from bottom to top because the image is internally a DIB
        * which is mirrored vertically
        */
       row_pointers[0] = ((png_bytep) data) + (gbm->h * row_bytes) - row_bytes;

       for (row = 1; row < gbm->h; row++)
       {
           row_pointers[row] = row_pointers[row-1] - row_bytes;
       }

       /* now it's time to write the image data */
       png_write_image(png_ptr, row_pointers);
       png_write_end(png_ptr, info_ptr);

       gbmmem_free(row_pointers);
       row_pointers = NULL;

       if (io_p->errok != GBM_TRUE)
       {
          png_write_deinit(&png_priv);
          return GBM_ERR_WRITE;
       }
     }

     /* clean up after the write, and free any memory allocated */
     png_write_deinit(&png_priv);

     return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */

/* png_err - map error code to error message */
const char *png_err(GBM_ERR rc)
{
   PNG_GBMERR_MSG *png_errmsg_p = png_errmsg;

   while (png_errmsg_p->error_message)
   {
       if (png_errmsg_p->rc == rc)
          return png_errmsg_p->error_message;

       png_errmsg_p++;
   }

   return NULL;
}

#endif

