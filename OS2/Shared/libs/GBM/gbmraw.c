/* gbmraw.cpp - Camera RAW

Credit for writing this module must go to Heiko Nitzsche.
This file is just as public domain as the rest of GBM.

Supported formats and options:
------------------------------
Camera RAW : All dcraw supported formats
(DNG 3FR ARW BAY CAP CRW CR2 DCR DCS DRF ERF KDC MDC MEF MOS MRW NEF NRW ORF PEF RAF RAW RW2 RWL SRF SR2)

Standard formats (backward compatible):
  Reads files and presents them as 24 bpp.

Extended formats (not backward compatible, import option ext_bpp required):
  Reads files and presents them as 48 bpp.

Input:
------

Can specify that non-standard GBM color depth is exported (48 bpp)
  Input option: ext_bpp (default: bpp is downsampled to 24 bpp)

Can specify that a half-sized image is created
  Input option: halfsize (default is full size)

Can specify the interpolation quality (0 - linear, 1- VNG, 2 - PPG, 3 - AHD, 4 - DCB)
  Input option: quality=# (0 to 4, default 2)

Can specify that no white balance correction is done
  Input option: nowb  (default is camera white balance)

Can specify that the average of the whole image is used for white balance
  Input option: avgwb  (default is camera white balance)

Can specify a custom white balance
  Input option: wb=r_g_b_g

Can specify that chromatic aberration is corrected
  Input option: aber=r_b

Can specify that the image is automatically brightened
  Input option: autobright

Can specify the image brightness
  Input option: bright=# (default: 1.0)

Can specify the image gamma curve (BT.709: 2.222_4.5, sRGB: 2.4_12.92, linear: 1.0_1.0)
  Input option: gamma=power_toeslope (default: 2.4_12.92 sRGB)

Can specify a user defined darkness level
  Input option: darkness=#

Can specify a user defined saturation level
  Input option: saturation=#

Can specify the highlight mode
  Input option: highlights=# (0..9: 0=clip (default), 1=unclip, 2=blend, 3+=rebuild)

Can specify to do separate interpolations for two green components
  Input option: 4color

Can specify the threshold for wavelet denoising
  Input option: denoise=#

Can specify how often a 3x3 median filter is applied to R-G and B-G
  Input option: medianfilter=# (0..N, default 0)


History:
--------
(Heiko Nitzsche)

20-Sep-2010: Initial version based on libraw 0.10.0

24-Oct-2010: * Update to libraw 0.11 beta5
             * Better handling of additional rows/columns due to rounding accuracy
               (between image info and post decoding size)
             * Fix wrong destination size when combining halfsize and aber or denoise params
             * Default interpolation quality is now 2 (PPG) as a good compromise
               between performance and quality
             * Allow manuell gamma adjustment, default is sRGB
             * Add options for clipping, saturation, blacklevel and four color RGGB interpolation
             * Allow disabling camera withe balance correction

06-Feb-2011: * Update to libraw 0.13.1
             * Add DCB interpolation as quality 4

21-Jun-2011: * Update to libraw 0.13.5
             * Increase stack size for worker thread to 350kb
             * Fix crash during reading image count due to stack overflow

TODO
----
Add multi-page support
Add LCMS support for using embedded profiles

*/

#ifdef ENABLE_RAW

#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmraw.h"
#include "gbmhelp.h"
#include "gbmdesc.h"
#include "gbmmem.h"
#include "gbmmap.h"
#include "gbmmthrd.h"

#include "libraw.h"

/* ---------------------------------------- */

#define GBM_ERR_RAW_NO_THUMB         ((GBM_ERR) 8200)
#define GBM_ERR_RAW_BAD_THUMB        ((GBM_ERR) 8201)
#define GBM_ERR_RAW_BAD_QUALITY      ((GBM_ERR) 8202)
#define GBM_ERR_RAW_BAD_ABERRATION   ((GBM_ERR) 8203)
#define GBM_ERR_RAW_BAD_BRIGHT       ((GBM_ERR) 8204)
#define GBM_ERR_RAW_BAD_GAMMA        ((GBM_ERR) 8205)
#define GBM_ERR_RAW_BAD_BRIGHTCOMB   ((GBM_ERR) 8206)
#define GBM_ERR_RAW_BAD_WB           ((GBM_ERR) 8207)
#define GBM_ERR_RAW_BAD_WBCOMB       ((GBM_ERR) 8208)
#define GBM_ERR_RAW_BAD_NOWB         ((GBM_ERR) 8209)
#define GBM_ERR_RAW_BAD_BLACKLEVEL   ((GBM_ERR) 8210)
#define GBM_ERR_RAW_BAD_SATURATION   ((GBM_ERR) 8211)
#define GBM_ERR_RAW_BAD_HIGHLIGHT    ((GBM_ERR) 8212)
#define GBM_ERR_RAW_BAD_DENOISE      ((GBM_ERR) 8213)
#define GBM_ERR_RAW_BAD_MEDIANFILTER ((GBM_ERR) 8214)

/* ---------------------------------------- */

static GBMFT raw_gbmft =
{
   GBM_FMT_DESC_SHORT_RAW,
   GBM_FMT_DESC_LONG_RAW,
   GBM_FMT_DESC_EXT_RAW,
   GBM_FT_R24 | GBM_FT_R48
};

typedef struct
{
   /* This entry will store the options provided during first header read.
    * It will keep the options for the case the header has to be reread.
    */
   char read_options[PRIV_SIZE - sizeof(int)
                               - 8 /* space for structure element padding */ ];
} RAW_PRIV_READ;

/* ---------------------------------------- */
/* ---------------------------------------- */

static GBM_ERR MakeGbmErrFromRawErr(const int rawErr, const GBM_ERR defaultGbmErr)
{
    GBM_ERR gbmRc = GBM_ERR_OK;
    switch(rawErr)
    {
        case LIBRAW_SUCCESS:               gbmRc = GBM_ERR_OK;            break;
        case LIBRAW_FILE_UNSUPPORTED:      gbmRc = GBM_ERR_NOT_SUPP;      break;
        case LIBRAW_NO_THUMBNAIL:          gbmRc = GBM_ERR_RAW_NO_THUMB;  break;
        case LIBRAW_UNSUPPORTED_THUMBNAIL: gbmRc = GBM_ERR_RAW_BAD_THUMB; break;
        case LIBRAW_UNSUFFICIENT_MEMORY:   gbmRc = GBM_ERR_MEM;           break;
        case LIBRAW_DATA_ERROR:            gbmRc = GBM_ERR_NOT_SUPP;      break;
        case LIBRAW_IO_ERROR:              gbmRc = GBM_ERR_READ;          break;

        default:
            gbmRc = defaultGbmErr;
            break;
    }
    return gbmRc;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR raw_qft(GBMFT *gbmft)
{
   *gbmft = raw_gbmft;
   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

static GBM_ERR internal_raw_read(int fd, GBM * gbm, gbm_u8 * data, int *pimgcnt, gbm_boolean imgCntOnly)
{
   RAW_PRIV_READ *raw_priv = (RAW_PRIV_READ *) gbm->priv;
   const char * s      = NULL;
   gbm_u8     * buffer = NULL;
   gbm_boolean  use_native_bpp, isUnpackedHalfSize = GBM_FALSE;
   long         fsize  = 0;
   int          rawErr = 0;

   /* decode raw file via buffer */
   libraw_data_t * libraw_data = libraw_init(0);
   if (NULL == libraw_data)
   {
       return GBM_ERR_MEM;
   }

   /* check if extended color depths are requested */
   use_native_bpp = (gbm_find_word(raw_priv->read_options, "ext_bpp") != NULL)
                    ? GBM_TRUE : GBM_FALSE;

   /* set the default processing parameters */
   libraw_data->params.output_bps      = use_native_bpp ? 16 : 8;
   libraw_data->params.document_mode   = 0;
   libraw_data->params.output_color    = 1; /* sRGB */
   libraw_data->params.shot_select     = 0;
   libraw_data->params.half_size       = 0;

   libraw_data->params.gamm[0]         = 1.0/2.4; /* sRGB=1.0/2.4, BT.709=1.0/2.222, linear=1.0 */
   libraw_data->params.gamm[1]         = 12.92;   /* sRGB=12.92  , BT.709=4.5      , linear=1.0 */
   libraw_data->params.use_auto_wb     = 0;
   libraw_data->params.use_camera_wb   = 1;
   libraw_data->params.no_auto_bright  = 1;
   libraw_data->params.auto_bright_thr = 0.0001f; /* dcraw default is 0.01 (1%) but new digicams do better */
   libraw_data->params.user_qual       = 2; /* 0 - linear, 1- VNG, 2 - PPG, 3 - AHD */

   if (gbm_find_word(raw_priv->read_options, "halfsize") != NULL)
   {
       libraw_data->params.half_size = 1;
       libraw_data->params.four_color_rgb = 1;
   }

   /* white balance settings */
   if (gbm_find_word(raw_priv->read_options, "avgwb") != NULL)
   {
       if (gbm_find_word(raw_priv->read_options, "wb=") != NULL)
       {
           libraw_close(libraw_data);
           return GBM_ERR_RAW_BAD_WBCOMB;
       }
       libraw_data->params.use_auto_wb   = 1;
       libraw_data->params.use_camera_wb = 0;
   }
   if ((s = gbm_find_word_prefix(raw_priv->read_options, "wb=")) != NULL)
   {
      float r = 1.0, g1 = 1.0, b = 1.0, g2 = 1.0;
      if (gbm_find_word(raw_priv->read_options, "avgwb") != NULL)
      {
          libraw_close(libraw_data);
          return GBM_ERR_RAW_BAD_WBCOMB;
      }
      if (sscanf(s + 3, "%f_%f_%f_%f", &r, &g1, &b, &g2) != 4)
      {
         libraw_close(libraw_data);
         return GBM_ERR_RAW_BAD_WB;
      }
      libraw_data->params.use_auto_wb   = 0;
      libraw_data->params.use_camera_wb = 0;
      libraw_data->params.user_mul[0]   = r;
      libraw_data->params.user_mul[1]   = g1;
      libraw_data->params.user_mul[2]   = b;
      libraw_data->params.user_mul[3]   = g2;
   }
   if (gbm_find_word(raw_priv->read_options, "nowb") != NULL)
   {
       /* check if avgwb or wb= was already specified (switches off camera wb) */
       if ((gbm_find_word(raw_priv->read_options, "avgwb") != NULL) ||
           (gbm_find_word(raw_priv->read_options, "wb=")   != NULL))
       {
           libraw_close(libraw_data);
           return GBM_ERR_RAW_BAD_NOWB;
       }
       libraw_data->params.use_auto_wb   = 0;
       libraw_data->params.use_camera_wb = 0;
   }

   /* quality related settings */
   if ((s = gbm_find_word_prefix(raw_priv->read_options, "quality=")) != NULL)
   {
       if ((sscanf(s + 8, "%d", &(libraw_data->params.user_qual)) != 1)
           || (libraw_data->params.user_qual < 0)
           || (libraw_data->params.user_qual > 4))
       {
           libraw_close(libraw_data);
           return GBM_ERR_RAW_BAD_QUALITY;
       }
   }
   if (gbm_find_word(raw_priv->read_options, "4color") != NULL)
   {
       libraw_data->params.four_color_rgb = 1;
   }
   if ((s = gbm_find_word_prefix(raw_priv->read_options, "aber=")) != NULL)
   {
       double redFactor   = 0;
       double greenFactor = 0;
       if ((sscanf(s + 5, "%lf_%lf", &redFactor, &greenFactor) != 2)
           || (redFactor == 0.0) || (greenFactor == 0.0))
       {
          libraw_close(libraw_data);
          return GBM_ERR_RAW_BAD_ABERRATION;
       }
       libraw_data->params.aber[0] = 1.0 / redFactor;
       libraw_data->params.aber[2] = 1.0 / greenFactor;
       isUnpackedHalfSize = (libraw_data->params.half_size == 0) ? GBM_TRUE : GBM_FALSE;
   }
   if ((s = gbm_find_word_prefix(raw_priv->read_options, "denoise=")) != NULL)
   {
       if (sscanf(s + 8, "%f", &(libraw_data->params.threshold)) != 1)
       {
           libraw_close(libraw_data);
           return GBM_ERR_RAW_BAD_DENOISE;
       }
       isUnpackedHalfSize = (libraw_data->params.half_size == 0) ? GBM_TRUE : GBM_FALSE;
   }

   if ((s = gbm_find_word_prefix(raw_priv->read_options, "medianfilter=")) != NULL)
   {
       if ((sscanf(s + 13, "%d", &(libraw_data->params.med_passes)) != 1)
           || (libraw_data->params.med_passes < 0))
       {
           libraw_close(libraw_data);
           return GBM_ERR_RAW_BAD_MEDIANFILTER;
       }
   }

   /* brightness settings */
   if (gbm_find_word(raw_priv->read_options, "autobright") != NULL)
   {
       if (gbm_find_word(raw_priv->read_options, "bright=") != NULL)
       {
           libraw_close(libraw_data);
           return GBM_ERR_RAW_BAD_BRIGHTCOMB;
       }
       libraw_data->params.no_auto_bright = 0;
   }
   if ((s = gbm_find_word_prefix(raw_priv->read_options, "bright=")) != NULL)
   {
       if (gbm_find_word(raw_priv->read_options, "autobright") != NULL)
       {
           libraw_close(libraw_data);
           return GBM_ERR_RAW_BAD_BRIGHTCOMB;
       }
       if ((sscanf(s + 7, "%f", &(libraw_data->params.bright)) != 1)
           || (libraw_data->params.bright < 0.0))
       {
           libraw_close(libraw_data);
           return GBM_ERR_RAW_BAD_BRIGHT;
       }
   }
   if ((s = gbm_find_word_prefix(raw_priv->read_options, "gamma=")) != NULL)
   {
       if (sscanf(s + 6, "%lf_%lf", &(libraw_data->params.gamm[0]),
                                    &(libraw_data->params.gamm[1])) != 2)
       {
           libraw_close(libraw_data);
           return GBM_ERR_RAW_BAD_GAMMA;
       }
       if (libraw_data->params.gamm[0] <= 0.0)
       {
           libraw_close(libraw_data);
           return GBM_ERR_RAW_BAD_GAMMA;
       }
       libraw_data->params.gamm[0] = (double)1.0 / libraw_data->params.gamm[0];
   }
   if ((s = gbm_find_word_prefix(raw_priv->read_options, "darkness=")) != NULL)
   {
       if (sscanf(s + 9, "%d", &(libraw_data->params.user_black)) != 1)
       {
           libraw_close(libraw_data);
           return GBM_ERR_RAW_BAD_BLACKLEVEL;
       }
   }
   if ((s = gbm_find_word_prefix(raw_priv->read_options, "saturation=")) != NULL)
   {
       if (sscanf(s + 11, "%d", &(libraw_data->params.user_sat)) != 1)
       {
           libraw_close(libraw_data);
           return GBM_ERR_RAW_BAD_SATURATION;
       }
   }
   if ((s = gbm_find_word_prefix(raw_priv->read_options, "highlights=")) != NULL)
   {
       if ((sscanf(s + 11, "%d", &(libraw_data->params.highlight)) != 1) ||
           (libraw_data->params.highlight < 0) ||
           (libraw_data->params.highlight > 9))
       {
           libraw_close(libraw_data);
           return GBM_ERR_RAW_BAD_HIGHLIGHT;
       }
   }

   /* read the whole file and then process from buffer */
   fsize = gbm_file_lseek(fd, 0, GBM_SEEK_END);

   /* start at the beginning of the file */
   if (gbm_file_lseek(fd, 0, GBM_SEEK_SET) < 0)
   {
       libraw_close(libraw_data);
       return GBM_ERR_READ;
   }

   buffer = (gbm_u8 *)gbmmem_malloc(fsize);
   if (buffer == NULL)
   {
       libraw_close(libraw_data);
       return GBM_ERR_MEM;
   }
   if (fsize != gbm_file_read(fd, buffer, fsize))
   {
       libraw_close(libraw_data);
       gbmmem_free(buffer);
       return GBM_ERR_READ;
   }

   rawErr = libraw_open_buffer(libraw_data, buffer, fsize);
   if (rawErr != LIBRAW_SUCCESS)
   {
       libraw_close(libraw_data);
       gbmmem_free(buffer);
       return MakeGbmErrFromRawErr(rawErr, GBM_ERR_READ);
   }

   /* only RGB supported */
   if (libraw_data->idata.colors != 3)
   {
       libraw_close(libraw_data);
       gbmmem_free(buffer);
       return GBM_ERR_NOT_SUPP;
   }

   *pimgcnt = libraw_data->idata.raw_count;
   if (*pimgcnt < 1)
   {
       /* image format not recognized */
       libraw_close(libraw_data);
       gbmmem_free(buffer);
       return GBM_ERR_NOT_SUPP;
   }
   if (imgCntOnly)
   {
       libraw_close(libraw_data);
       gbmmem_free(buffer);
       return GBM_ERR_OK;
   }

   /* Do info analysis only if no data are requested. */
   if (data == NULL)
   {
       rawErr = libraw_adjust_sizes_info_only(libraw_data);
       if (rawErr != LIBRAW_SUCCESS)
       {
           libraw_close(libraw_data);
           gbmmem_free(buffer);
           return MakeGbmErrFromRawErr(rawErr, GBM_ERR_READ);
       }

       gbm->w   = libraw_data->sizes.iwidth;
       gbm->h   = libraw_data->sizes.iheight;
       gbm->bpp = libraw_data->idata.colors * libraw_data->params.output_bps;

       if (isUnpackedHalfSize)
       {
           gbm->w = gbm->w * 2 + 1;
           gbm->h = gbm->h * 2 + 1;
       }

       libraw_close(libraw_data);
       gbmmem_free(buffer);
       return GBM_ERR_OK;
   }

   /* unfortunately we have to unpack and decode the image to get correct sizes */
   rawErr = libraw_unpack(libraw_data);
   if (rawErr != LIBRAW_SUCCESS)
   {
       libraw_close(libraw_data);
       gbmmem_free(buffer);
       return MakeGbmErrFromRawErr(rawErr, GBM_ERR_NOT_SUPP);
   }
   gbmmem_free(buffer);

   /* decode */
   {
       libraw_processed_image_t *image = NULL;

       rawErr = libraw_dcraw_process(libraw_data);
       if (rawErr != LIBRAW_SUCCESS)
       {
           libraw_close(libraw_data);
           return MakeGbmErrFromRawErr(rawErr, GBM_ERR_NOT_SUPP);
       }

       image = libraw_dcraw_make_mem_image(libraw_data, &rawErr);
       if (image == NULL)
       {
           libraw_close(libraw_data);
           return MakeGbmErrFromRawErr(rawErr, GBM_ERR_NOT_SUPP);
       }

       if (   (gbm->w   < image->width)
           || (gbm->h   < image->height)
           || (gbm->bpp != (image->colors * image->bits))
           || (image->type != LIBRAW_IMAGE_BITMAP)
           || ((gbm->bpp != 24) && (gbm->bpp != 48)) )
       {
           libraw_dcraw_clear_mem(image);
           libraw_close(libraw_data);
           return GBM_ERR_READ;
       }

       /* copy data if requested */
       {
           const GBMRGB_16BPP backrgb = { 0, 0, 0 };
                 size_t stride_src = image->width * (gbm->bpp / 8);
           const size_t stride_dst = ((gbm->w * gbm->bpp + 31) / 32) * 4;
           const gbm_u8 * data8_src = (const gbm_u8 *) image->data;
                 gbm_u8 * data8_dst = data;
                 int      h, i;

           GBM * gbm_tmp = gbmmem_malloc(sizeof(GBM));
           if (gbm_tmp == NULL)
           {
              libraw_dcraw_clear_mem(image);
              libraw_close(libraw_data);
              return GBM_ERR_MEM;
           }
           *gbm_tmp   = *gbm;
           gbm_tmp->w = image->width;
           gbm_tmp->h = image->height;

           /* flip vertically during reading */
           data8_src += stride_src * (gbm_tmp->h - 1);

           for (h = gbm_tmp->h - 1; h >= 0; --h)
           {
              if (! gbm_map_row_RGBx_BGRx(data8_src, gbm_tmp,
                                          data8_dst, gbm_tmp,
                                          &backrgb , GBM_FALSE))
              {
                  gbmmem_free(gbm_tmp);
                  libraw_dcraw_clear_mem(image);
                  libraw_close(libraw_data);
                  return GBM_ERR_NOT_SUPP;
              }
              /* duplicate the last column to compensate the gap potentially
               * coming from rounding between image info and post decoding size
               */
              for (i = 0; i < (gbm->w - gbm_tmp->w); ++i)
              {
                  gbm_u8 * data8_dst2 = data8_dst + stride_src + (i * 3);
                  *(data8_dst2)     = *(data8_src + stride_src - 1);
                  *(data8_dst2 + 1) = *(data8_src + stride_src - 2);
                  *(data8_dst2 + 2) = *(data8_src + stride_src - 3);
              }
              data8_src -= stride_src;
              data8_dst += stride_dst;
           }

           /* duplicate the last row to compensate the gap potentially
            * coming from rounding between image info and post decoding size
            */
           data8_dst -= stride_dst;
           for (i = 0; i < (gbm->h - gbm_tmp->h); ++i)
           {
              memcpy(data8_dst + (stride_dst * i), data8_dst - stride_dst, stride_dst);
           }

           gbmmem_free(gbm_tmp);
       }
       libraw_dcraw_clear_mem(image);
   }

   libraw_close(libraw_data);
   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

/* Execute rimg(), rhdr() and rdata() in separate threads
 * with much more stack as libraw has huge stack
 * requirements and the GBM binary interface only
 * allows stack usage of up to 0x6000 bytes.
 */
#define LIBRAW_RESERVED_STACK (350*1024)

/* ---------------------------------------- */

typedef struct
{
    int          fd;
    int        * pimgcnt;
    GBM_ERR      rc;
} RIMGCNT_THREAD_ARG;

static void Thread_raw_rimgcnt(GBM_THREAD_ARG arg)
{
   RIMGCNT_THREAD_ARG * pArgs = (RIMGCNT_THREAD_ARG *) arg;

   GBM gbm_src;
   memset(&gbm_src, 0, sizeof(GBM));

   /* read bitmap info */
   *(pArgs->pimgcnt) = 0;
   pArgs->rc = internal_raw_read(pArgs->fd, &gbm_src, NULL, pArgs->pimgcnt, GBM_TRUE);
   if ((pArgs->rc == GBM_ERR_OK) && (*(pArgs->pimgcnt) > 1))
   {
      *(pArgs->pimgcnt) = 1;
   }
}

/* Read number of images in the RAW file. */
GBM_ERR raw_rimgcnt(const char *fn, int fd, int *pimgcnt)
{
   GBM_THREAD         threadId;
   RIMGCNT_THREAD_ARG threadArgs;

   fn = fn;  /* Suppress 'unref arg' compiler warnings */

   threadArgs.fd      = fd;
   threadArgs.pimgcnt = pimgcnt;
   threadArgs.rc      = GBM_ERR_OK;

   threadId = gbmmthrd_scheduleTask(Thread_raw_rimgcnt, &threadArgs, LIBRAW_RESERVED_STACK);
   gbmmthrd_finishTask(&threadId);
   return threadArgs.rc;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

typedef struct
{
    int          fd;
    GBM        * gbm;
    const char * opt;
    GBM_ERR      rc;
} RHDR_THREAD_ARG;

static void Thread_raw_rhdr(GBM_THREAD_ARG arg)
{
   RHDR_THREAD_ARG * pArgs = (RHDR_THREAD_ARG *) arg;
   RAW_PRIV_READ *raw_priv = (RAW_PRIV_READ *) pArgs->gbm->priv;
   int imgcnt = 0;

   /* copy possible options */
   if ((NULL == pArgs->opt) || (NULL == raw_priv->read_options))
   {
      pArgs->rc = GBM_ERR_BAD_OPTION;
      return;
   }
   if (strlen(pArgs->opt) >= sizeof(raw_priv->read_options))
   {
      pArgs->rc = GBM_ERR_BAD_OPTION;
      return;
   }
   memset(raw_priv->read_options, 0, sizeof(raw_priv->read_options));
   strcpy(raw_priv->read_options, pArgs->opt);

   /* read bitmap info */
   pArgs->rc = internal_raw_read(pArgs->fd, pArgs->gbm, NULL, &imgcnt, GBM_FALSE);
}

GBM_ERR raw_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
{
   GBM_THREAD      threadId;
   RHDR_THREAD_ARG threadArgs;

   fn = fn;  /* Suppress 'unref arg' compiler warnings */

   threadArgs.fd  = fd;
   threadArgs.gbm = gbm;
   threadArgs.opt = opt;
   threadArgs.rc  = GBM_ERR_OK;

   threadId = gbmmthrd_scheduleTask(Thread_raw_rhdr, &threadArgs, LIBRAW_RESERVED_STACK);
   gbmmthrd_finishTask(&threadId);
   return threadArgs.rc;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR raw_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
{
   fd=fd; gbm=gbm; gbmrgb=gbmrgb; /* Suppress 'unref arg' compiler warnings */
   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

typedef struct
{
    int       fd;
    GBM     * gbm;
    gbm_u8  * data;
    GBM_ERR   rc;
} RDATA_THREAD_ARG;

static void Thread_raw_rdata(GBM_THREAD_ARG arg)
{
   RDATA_THREAD_ARG * pArgs = (RDATA_THREAD_ARG *) arg;
   int imgcnt = 0;

   pArgs->rc = internal_raw_read(pArgs->fd, pArgs->gbm, pArgs->data, &imgcnt, GBM_FALSE);
}

GBM_ERR raw_rdata(int fd, GBM *gbm, gbm_u8 *data)
{
   GBM_THREAD       threadId;
   RDATA_THREAD_ARG threadArgs;

   threadArgs.fd   = fd;
   threadArgs.gbm  = gbm;
   threadArgs.data = data;
   threadArgs.rc   = GBM_ERR_OK;

   threadId = gbmmthrd_scheduleTask(Thread_raw_rdata, &threadArgs, LIBRAW_RESERVED_STACK);
   gbmmthrd_finishTask(&threadId);
   return threadArgs.rc;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

const char *raw_err(GBM_ERR rc)
{
   switch(rc)
   {
      case GBM_ERR_RAW_NO_THUMB:
         return libraw_strerror(LIBRAW_NO_THUMBNAIL);

      case GBM_ERR_RAW_BAD_THUMB:
         return libraw_strerror(LIBRAW_UNSUPPORTED_THUMBNAIL);

      case GBM_ERR_RAW_BAD_QUALITY:
        return "quality is not in 0..3";

      case GBM_ERR_RAW_BAD_ABERRATION:
        return "aber= has unsupported values";

      case GBM_ERR_RAW_BAD_WB:
        return "wb= has unsupported values";

      case GBM_ERR_RAW_BAD_WBCOMB:
        return "avgwb and wb= cannot be used together";

      case GBM_ERR_RAW_BAD_NOWB:
        return "nowb and avgwb/wb= cannot be used together";

      case GBM_ERR_RAW_BAD_BRIGHT:
        return "bright= has unsupported value";

      case GBM_ERR_RAW_BAD_GAMMA:
        return "gamma= has unsupported value";

      case GBM_ERR_RAW_BAD_BRIGHTCOMB:
        return "autobright and bright= cannot be used together";

      case GBM_ERR_RAW_BAD_BLACKLEVEL:
        return "darkness= has unsupported value";

      case GBM_ERR_RAW_BAD_SATURATION:
        return "saturation= has unsupported value";

      case GBM_ERR_RAW_BAD_HIGHLIGHT:
        return "hightlights= has unsupported mode (0..9)";

      case GBM_ERR_RAW_BAD_DENOISE:
        return "denoise= has unsupported value";

      case GBM_ERR_RAW_BAD_MEDIANFILTER:
        return "medianfilter= has unsupported value (0..N)";
   }
   return NULL;
}

#endif // ENABLE_RAW

