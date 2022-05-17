/*************************************************************************

gbmj2k.c - JPEG2000 Format support (JP2, J2K, J2C, JPT)

Credit for writing this module must go to Heiko Nitzsche.

This file is just as public domain as the rest of GBM.

This code is a mainly a wrapper around the OpenJPEG library and
supports most features of the actual version. Supported are
YUV, sRGB and Graylevel images.

Supported formats and options:
------------------------------
JPEG2000 : JPEG2000 Graphics File Format            : .JP2
                     JPEG2000 Codestream            : .J2K .J2C
                     JPT      Stream (JPEG200,JPIP) : .JPT

Standard formats (backward compatible):
  Reads  8 bpp gray level files and presents them as 8 bpp.
  Reads 16 bpp gray level files and presents them as 8 bpp.
  Reads 24 bpp colour files and presents them as 24 bpp.
  Reads 48 bpp colour files and presents them as 24 bpp.

Extended formats (not backward compatible, import option ext_bpp required):
  Reads 16 bpp gray level files and presents them as 48 bpp.
  Reads 48 bpp colour files and presents them as 48 bpp.

Writes  8 bpp gray level files (colour palette bitmaps are converted).
Writes 24 and 48 bpp bpp colour files.

Can specify the colour channel the output grey values are based on
    Output option: r,g,b,k (default: k, combine color channels and write grey equivalent)

Can specify compression rate
    Output option: compression=# (0..N, lossless compression (default=0) to N times compressed)

Can specify quality based compression
    Output option: quality=# (0..100, poor to very good quality)

Write additonal comment
    Output option: comment=text


History:
--------
28-Aug-2008  Initial version
11-Sep-2008  JPWL support added
             Fix wrong extension to codec assignment
             Add support for rate based compression
             Add support for quality based compression
28-Nov-2008  Less checking for potentially wrong color space -> OpenJPEG does it itself
             Get rid of unnecessary manual YUV->RGB conversion
 8-Feb-2011  Update to OpenJPEG 1.4 (fix a missing compression parameter initialization)
23-Oct-2011  Disable JPWL decode when only trying to get the image info
 
******************************************************************************/

#ifdef ENABLE_J2K

/* Includes */
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmdesc.h"
#include "gbmhelp.h"
#include "gbmmap.h"
#include "gbmmem.h"
#include "openjpeg.h"

/* ----------------------------------------------------------- */

#define GBM_ERR_J2K                             ((GBM_ERR) 6100)
#define GBM_ERR_J2K_BPP                         ((GBM_ERR) 6101)
#define GBM_ERR_J2K_HEADER                      ((GBM_ERR) 6102)
#define GBM_ERR_J2K_BAD_COMMENT                 ((GBM_ERR) 6103)
#define GBM_ERR_J2K_BAD_COMPRESSION             ((GBM_ERR) 6104)
#define GBM_ERR_J2K_BAD_COMPRESSION_RANGE       ((GBM_ERR) 6105)
#define GBM_ERR_J2K_BAD_QUALITY                 ((GBM_ERR) 6106)
#define GBM_ERR_J2K_BAD_QUALITY_RANGE           ((GBM_ERR) 6107)
#define GBM_ERR_J2K_BAD_MIX_COMPRESSION_QUALITY ((GBM_ERR) 6108)

#define CVT(x) (((x) * 255) / ((1L << 16) - 1))

/* ----------------------------------------------------------- */

typedef struct
{
    GBM_ERR rc;
    const char *error_message;
} J2K_GBMERR_MSG;

static J2K_GBMERR_MSG j2k_errmsg[] =
{
    { GBM_ERR_J2K_BPP                        , "bad bits per pixel"          },
    { GBM_ERR_J2K_HEADER                     , "bad header"                  },
    { GBM_ERR_J2K_BAD_COMMENT                , "comment could not be parsed" },
    { GBM_ERR_J2K_BAD_COMPRESSION            , "compression rate could not be parsed" },
    { GBM_ERR_J2K_BAD_COMPRESSION_RANGE      , "compression rate must be >=0"  },
    { GBM_ERR_J2K_BAD_QUALITY                , "quality could not be parsed"   },
    { GBM_ERR_J2K_BAD_QUALITY_RANGE          , "quality must be >=0 and <=100" },
    { GBM_ERR_J2K_BAD_MIX_COMPRESSION_QUALITY, "parameters compression and quality cannot be used together" },
    { -1                                     , NULL }
};

static GBMFT j2k_jp2_gbmft =
{
    GBM_FMT_DESC_SHORT_JP2,
    GBM_FMT_DESC_LONG_JP2,
    GBM_FMT_DESC_EXT_JP2,
    GBM_FT_R8 | GBM_FT_R24 | GBM_FT_R48 |
    GBM_FT_W8 | GBM_FT_W24 | GBM_FT_W48
};
static GBMFT j2k_j2k_gbmft =
{
    GBM_FMT_DESC_SHORT_J2K,
    GBM_FMT_DESC_LONG_J2K,
    GBM_FMT_DESC_EXT_J2K,
    GBM_FT_R8 | GBM_FT_R24 | GBM_FT_R48 |
    GBM_FT_W8 | GBM_FT_W24 | GBM_FT_W48
};
static GBMFT j2k_jpt_gbmft =
{
    GBM_FMT_DESC_SHORT_JPT,
    GBM_FMT_DESC_LONG_JPT,
    GBM_FMT_DESC_EXT_JPT,
    GBM_FT_R8 | GBM_FT_R24 | GBM_FT_R48 /* JPT can only be read */
};

typedef struct
{
    gbm_boolean  errok;  /* GBM_FALSE if an error during file decoding happened */
    opj_image_t *image;  /* Pointer to the decoded image */

    OPJ_CODEC_FORMAT codec; /* the codec to be used for reading */
    int bpp;                /* bpp of source image */

    /* This entry will store the options provided during first header read.
     * It will keep the options for the case the header has to be reread.
     */
    char read_options[PRIV_SIZE
                      - sizeof(gbm_boolean)
                      - sizeof(opj_image_t *)
                      - sizeof(OPJ_CODEC_FORMAT)
                      - 20 /* space for structure element padding */ ];
} J2K_PRIV_READ;

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/*
 * Divide an integer by a power of 2 and round upwards.
 *
 * a divided by 2^b
 */
static int int_ceildivpow2(int a, int b)
{
    return (a + (1 << b) - 1) >> b;
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* Replace standard OpenJPEG error functions by our own that prevent
   output to stdout and fatal handling.
   The replacement will be used temporarily during file I/O and sets
   an error code to the private IO struct.
*/

/* error callback expecting a valid J2K_PRIV_READ pointer */
static void j2k_gbm_error_callback(const char *error_msg, void *client_data)
{
   if (client_data != NULL)
   {
     J2K_PRIV_READ *j2k_priv = (J2K_PRIV_READ *) client_data;
     j2k_priv->errok = GBM_FALSE; /* error occured */
   }
#if DEBUG
   printf("ERROR: %s", error_msg);
#else
   error_msg = error_msg; /* prevent compiler warning */
#endif
}

/* ----------------------------------------------------------- */

/* warning callback expecting a valid J2K_PRIV_READ pointer */
static void j2k_gbm_warning_callback(const char *warning_msg, void *client_data)
{
#if DEBUG
   printf("WARNING: %s", warning_msg);
   client_data = client_data;
#else
   warning_msg = warning_msg; /* prevent compiler warning */
   client_data = client_data;
#endif
}

/* ----------------------------------------------------------- */

/* info callback expecting a valid J2K_PRIV_READ pointer */
static void j2k_gbm_info_callback(const char *info_msg, void *client_data)
{
#if DEBUG
   printf("INFO: %s", info_msg);
   client_data = client_data;
#else
   info_msg    = info_msg; /* prevent compiler warning */
   client_data = client_data;
#endif
}

/* ----------------------------------------------------------- */

/* Initialize structures for reading
   Returns GBM_TRUE on success, else GBM_FALSE
*/
static gbm_boolean j2k_read_init(J2K_PRIV_READ *j2k_priv)
{
   j2k_priv->errok = GBM_TRUE;
   j2k_priv->image = NULL;
   j2k_priv->bpp   = 0;
   return GBM_TRUE;
}

/* Cleanup structs for reading. */
static void j2k_read_deinit(J2K_PRIV_READ *j2k_priv)
{
    if (j2k_priv->image != NULL)
    {
       opj_image_destroy(j2k_priv->image);
       j2k_priv->image = NULL;
       j2k_priv->bpp   = 0;
    }
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* j2k_jp2_qft - Query format informations */
GBM_ERR j2k_jp2_qft(GBMFT *gbmft)
{
    *gbmft = j2k_jp2_gbmft;
    return GBM_ERR_OK;
}

/* j2k_j2k_qft - Query format informations */
GBM_ERR j2k_j2k_qft(GBMFT *gbmft)
{
    *gbmft = j2k_j2k_gbmft;
    return GBM_ERR_OK;
}

/* j2k_jpt_qft - Query format informations */
GBM_ERR j2k_jpt_qft(GBMFT *gbmft)
{
    *gbmft = j2k_jpt_gbmft;
    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

static GBM_ERR internal_j2k_decode(J2K_PRIV_READ     * j2k_read,
                                   opj_dparameters_t * parameters,
                                   opj_event_mgr_t   * event_mgr,
                                   opj_dinfo_t       * dinfo,
                                   const gbm_u8      * src_data,
                                   const int           src_data_len)
{
    opj_cio_t *cio = NULL;

    /* catch events using our callbacks and give a local context */
    opj_set_event_mgr((opj_common_ptr)dinfo, event_mgr, j2k_read);

    /* setup the decoder decoding parameters using user parameters */
    opj_setup_decoder(dinfo, parameters);

    /* open a byte stream */
    cio = opj_cio_open((opj_common_ptr)dinfo, (unsigned char *)src_data, src_data_len);
    if (cio == NULL)
    {
       return GBM_ERR_MEM;
    }
    /* decode the stream and fill the image structure */
    j2k_read->errok = GBM_TRUE;
    j2k_read->image = opj_decode(dinfo, cio);

    opj_cio_close(cio);
    cio = NULL;

    if ((j2k_read->image != NULL) && (j2k_read->errok))
    {
        return GBM_ERR_OK;
    }
    /* reset the image info for the next format test */
    if (j2k_read->image != NULL)
    {
        opj_image_destroy(j2k_read->image);
        j2k_read->image = NULL;
    }
    j2k_read->errok = GBM_FALSE;

    return GBM_ERR_J2K_HEADER;
}

/* ----------------------------------------------------------- */

static GBM_ERR internal_j2k_checkStreamFormat(const gbm_u8      *src_data,
                                              const int          src_data_len,
                                              const gbm_boolean  decodeHeaderOnly,
                                              J2K_PRIV_READ     *j2k_read)
{
    opj_dparameters_t *parameters = NULL; /* decompression parameters */
    opj_dinfo_t       *dinfo      = NULL;
    opj_event_mgr_t    event_mgr;         /* event manager */
    GBM_ERR            rc         = GBM_ERR_OK;

    /* configure the event callbacks */
    memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
    event_mgr.error_handler   = j2k_gbm_error_callback;
    event_mgr.warning_handler = j2k_gbm_warning_callback;
    event_mgr.info_handler    = j2k_gbm_info_callback;

    /* set decoding parameters to default values */
    parameters = gbmmem_malloc(sizeof(opj_dparameters_t));
    if (parameters == NULL)
    {
       return GBM_ERR_MEM;
    }
    opj_set_default_decoder_parameters(parameters);

    /* prepare parameters */
    parameters->cp_limit_decoding = decodeHeaderOnly ? LIMIT_TO_MAIN_HEADER : NO_LIMITATION;

#ifdef USE_JPWL
    parameters->jpwl_correct = decodeHeaderOnly ? OPJ_FALSE : OPJ_TRUE;
    /* parameters->jpwl_exp_comps = JPWL_EXPECTED_COMPONENTS; */
#endif

    /* create the decompression scheme */
    dinfo = opj_create_decompress(j2k_read->codec);
    rc    = internal_j2k_decode(j2k_read, parameters, &event_mgr, dinfo,
                                src_data, src_data_len);

    opj_destroy_decompress(dinfo); dinfo = NULL;
    gbmmem_free(parameters);

    return rc;
}

/* ----------------------------------------------------------- */

/* internal_j2k_rhdr - Read file header to init GBM struct */
static GBM_ERR internal_j2k_rhdr(const int          fd,
                                       GBM         *gbm,
                                 const gbm_boolean  decodeHeaderOnly)
{
    J2K_PRIV_READ *j2k_read = (J2K_PRIV_READ *) gbm->priv;

    GBM_ERR rc = GBM_ERR_READ;

    gbm_u8 *src_data     = NULL;
    long    src_data_len = 0L;

    /* find length of the stream */
    src_data_len = gbm_file_lseek(fd, 0L, GBM_SEEK_END);

    /* set file pointer to start. */
    if (gbm_file_lseek(fd, 0L, GBM_SEEK_SET) != 0)
    {
       return GBM_ERR_READ;
    }
    /* allocate buffer */
    src_data = (gbm_u8 *) gbmmem_malloc(src_data_len);
    if (src_data == NULL)
    {
       return GBM_ERR_MEM;
    }
    /* get data */
    if (gbm_file_read(fd, src_data, src_data_len) != src_data_len)
    {
       gbmmem_free(src_data);
       return GBM_ERR_READ;
    }
    /* set file pointer to start. */
    if (gbm_file_lseek(fd, 0L, GBM_SEEK_SET) != 0)
    {
       gbmmem_free(src_data);
       return GBM_ERR_READ;
    }

    /* init the read info struct */
    if (! j2k_read_init(j2k_read))
    {
       gbmmem_free(src_data);
       return GBM_ERR_MEM;
    }
    /* Detect the file format */
    rc = internal_j2k_checkStreamFormat(src_data, src_data_len,
                                        decodeHeaderOnly, j2k_read);
    if (rc != GBM_ERR_OK)
    {
       j2k_read_deinit(j2k_read);
       gbmmem_free(src_data);
       return rc;
    }
    gbmmem_free(src_data); src_data = NULL;

    gbm->w   = 0;
    gbm->h   = 0;
    gbm->bpp = 0;

    if (j2k_read->image->color_space == CLRSPC_GRAY) /* grayscale */
    {
        /* only support for 8bit per component */
        if (j2k_read->image->numcomps != 1)
        {
          j2k_read_deinit(j2k_read);
          return GBM_ERR_J2K_BPP;
        }
    }
    else /* everything else (e.g. SRGB, YUV) */
    {
        if ((j2k_read->image->numcomps != 1) &&
            (j2k_read->image->numcomps != 3))
        {
          j2k_read_deinit(j2k_read);
          return GBM_ERR_J2K_BPP;
        }
    }

    /* check that all components have the same color depth */
    {
      int i, j;
      for (i = 0; i < j2k_read->image->numcomps; i++)
      {
        /* only support for 8/16bit per component */
        const int prec = j2k_read->image->comps[i].prec;
        if ((prec != 8) && (prec != 16))
        {
            j2k_read_deinit(j2k_read);
            return GBM_ERR_J2K_BPP;
        }
        for (j = 0; j < j2k_read->image->numcomps; j++)
        {
          if (prec != j2k_read->image->comps[j].prec)
          {
            j2k_read_deinit(j2k_read);
            return GBM_ERR_J2K_BPP;
          }
        }
      }
    }

    /* use the reference grid size */
    gbm->w   = j2k_read->image->x1;
    gbm->h   = j2k_read->image->y1;
    gbm->bpp = j2k_read->image->numcomps * j2k_read->image->comps[0].prec;

    j2k_read->bpp = gbm->bpp;

    switch(gbm->bpp)
    {
        case  8:
        case 24:
            break;

        case 16:
            /* check if extended color depths are requested */
            if (gbm_find_word(j2k_read->read_options, "ext_bpp") == NULL)
            {
                /* downsample to 8bpp */
                gbm->bpp = 8;
            }
            else
            {
                /* upsample to 48bpp */
                gbm->bpp = 48;
            }
            break;

        case 48:
            /* check if extended color depths are requested */
            if (gbm_find_word(j2k_read->read_options, "ext_bpp") == NULL)
            {
                /* downsample to 24bpp */
                gbm->bpp = 24;
            }
            break;

        default:
            return GBM_ERR_J2K_BPP;
    }

    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* j2k_rhdr - Read file header to init GBM struct */
static GBM_ERR j2k_rhdr(const OPJ_CODEC_FORMAT codec,
                         const char *fn, int fd, GBM *gbm, const char *opt)
{
    J2K_PRIV_READ *j2k_read = (J2K_PRIV_READ *) gbm->priv;
    GBM_ERR        rc;

    fn = fn; /* prevent compiler warning */

    /* init options buffer */
    memset(j2k_read->read_options, 0, sizeof(j2k_read->read_options));

    /* copy possible options */
    if (strlen(opt) >= sizeof(j2k_read->read_options))
    {
       return GBM_ERR_BAD_OPTION;
    }
    strcpy(j2k_read->read_options, opt);
    j2k_read->codec = codec;

    rc = internal_j2k_rhdr(fd, gbm, GBM_TRUE);
    if (rc != GBM_ERR_OK)
    {
       return rc;
    }

    /* We cannot expect the client calls rhdr,rpal,rdata in order.
     * The client might just read the header and stop then.
     *
     * There is no other choice than freeing the structs
     * and reread them in the rdata function.
     */
    j2k_read_deinit(j2k_read);

    /* Don't override the read_options buffer as it will be
     * readout by internal_j2k_rhdr().
     */

    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */

/* JP2: JPEG-2000 file format */
/* j2k_jp2_rhdr - Read file header to init GBM struct */
GBM_ERR j2k_jp2_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
{
    return j2k_rhdr(CODEC_JP2, fn, fd, gbm, opt);
}

/* ----------------------------------------------------------- */

/* J2K: JPEG-2000 codestream */
/* j2k_j2k_rhdr - Read file header to init GBM struct */
GBM_ERR j2k_j2k_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
{
    return j2k_rhdr(CODEC_J2K, fn, fd, gbm, opt);
}

/* ----------------------------------------------------------- */

/* JPT: JPT-stream (JPEG 2000, JPIP) */
/* j2k_jpt_rhdr - Read file header to init GBM struct */
GBM_ERR j2k_jpt_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
{
    return j2k_rhdr(CODEC_JPT, fn, fd, gbm, opt);
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* j2k_rpal() -  Read palette */
GBM_ERR j2k_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
{
    J2K_PRIV_READ *j2k_read = (J2K_PRIV_READ *) gbm->priv;

    /* read the header again */
    GBM_ERR rc = internal_j2k_rhdr(fd, gbm, GBM_TRUE);
    if (rc != GBM_ERR_OK)
    {
       /* cleanup is done in the called function already */
       return rc;
    }

    if (gbm->bpp == 8) /* these are always grayscale bitmaps */
    {
       /* get the palette information */
       const int palette_entries = 256;

       /* init palette */
       int i;
       for (i = 0; i < palette_entries; i++)
       {
          gbmrgb[i].r = gbmrgb[i].g = gbmrgb[i].b = i;
       }
    }

    /* We cannot expect the client calls rhdr,rpal,rdata in order.
     * The client might just read the header and stop then.
     *
     * There is no other choice than freeing the structs
     * and reread them in the rdata function.
     */
    j2k_read_deinit(j2k_read);

    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* j2k_rdata()  -  Read data */

GBM_ERR j2k_rdata(int fd, GBM *gbm, gbm_u8 *data)
{
    /* read the header again but with full data decoding */
    GBM_ERR rc = internal_j2k_rhdr(fd, gbm, GBM_FALSE);
    if (rc != GBM_ERR_OK)
    {
       /* cleanup is done in the called function already */
       return rc;
    }
    else
    {
       J2K_PRIV_READ     *j2k_read = (J2K_PRIV_READ *) gbm->priv;
       const opj_image_t *image    = j2k_read->image;

       const int w  = image->comps[0].w;
       const int h  = image->comps[0].h;
       const int wr = int_ceildivpow2(w, image->comps[0].factor);
       const int hr = int_ceildivpow2(h, image->comps[0].factor);

       if ((w != gbm->w) || (h != gbm->h))
       {
         j2k_read_deinit(j2k_read);
         return GBM_ERR_NOT_SUPP;
       }

       switch(j2k_read->image->numcomps)
       {
         case 1:
           {
             const int *pGray      = image->comps[0].data;
             const int sOffsetGray = image->comps[0].sgnd ? (1 << (image->comps[0].prec - 1)) : 0;
             const int indices     = wr * hr;
             gbm_u16  *data16      = (gbm_u16 *) data;
             int i, pad;

             switch(j2k_read->bpp)
             {
               case 8:
                 if (gbm->bpp != 8)
                 {
                   j2k_read_deinit(j2k_read);
                   return GBM_ERR_NOT_SUPP;
                 }
                 for (i = 0; i < indices; i++)
                 {
                     const int index = w * hr - ((i) / (wr) + 1) * w + (i) % (wr);

                     *data++ = (gbm_u8)(pGray[index] + sOffsetGray);

                     if ((i + 1) % wr == 0)
                     {
                         for (pad = wr % 4 ? 4 - wr % 4 : 0; pad > 0; pad--) /* add padding */
                         {
                             *data++ = 0;
                         }
                      }
                 }
                 break;

               /* ---------------- */

               case 16:
                 switch(gbm->bpp)
                 {
                   case 8: /* via downsampling */
                     for (i = 0; i < indices; i++)
                     {
                         const int index = w * hr - ((i) / (wr) + 1) * w + (i) % (wr);

                         *data++ = (gbm_u8) CVT(pGray[index] + sOffsetGray);

                         if ((i + 1) % wr == 0)
                         {
                             for (pad = wr % 4 ? 4 - wr % 4 : 0; pad > 0; pad--) /* add padding */
                             {
                                 *data++ = 0;
                             }
                          }
                      }
                      break;

                   case 48: /* via upsampling gray -> RGB */
                     for (i = 0; i < indices; i++)
                     {
                         const int index = w * hr - ((i) / (wr) + 1) * w + (i) % (wr);

                         const gbm_u16 g = pGray[index] + sOffsetGray;
                         *data16++ = g;
                         *data16++ = g;
                         *data16++ = g;
                         data += 6;

                         if ((i + 1) % wr == 0)
                         {
                             for (pad = (6 * wr) % 4 ? 4 - (6 * wr) % 4 : 0; pad > 0; pad--) /* add padding */
                             {
                                 *data++ = 0;
                             }
                             data16 = (gbm_u16 *) data;
                          }
                      }
                      break;

                    default:
                      j2k_read_deinit(j2k_read);
                      return GBM_ERR_NOT_SUPP;
                 }
                 break;

               /* ---------------- */

               default:
                 j2k_read_deinit(j2k_read);
                 return GBM_ERR_NOT_SUPP;
             }
           }
           break;

         /* ---------------- */

         case 3:
           {
             const int *p0      = image->comps[0].data;
             const int *p1      = image->comps[1].data;
             const int *p2      = image->comps[2].data;
             const int sOffset0 = image->comps[0].sgnd ? (1 << (image->comps[0].prec - 1)) : 0;
             const int sOffset1 = image->comps[1].sgnd ? (1 << (image->comps[1].prec - 1)) : 0;
             const int sOffset2 = image->comps[2].sgnd ? (1 << (image->comps[2].prec - 1)) : 0;
             const int indices  = wr * hr;
             gbm_u16 *data16    = (gbm_u16 *) data;
             int i, pad;

             switch(j2k_read->bpp)
             {
               case 24:
                 if (gbm->bpp != 24)
                 {
                   j2k_read_deinit(j2k_read);
                   return GBM_ERR_NOT_SUPP;
                 }
                 for (i = 0; i < indices; i++)
                 {
                   const int index = w * hr - ((i) / (wr) + 1) * w + (i) % (wr);

                   *data++ = p2[index] + sOffset2;
                   *data++ = p1[index] + sOffset1;
                   *data++ = p0[index] + sOffset0;

                   if ((i + 1) % wr == 0)
                   {
                     for (pad = (3 * wr) % 4 ? 4 - (3 * wr) % 4 : 0; pad > 0; pad--) /* add padding */
                     {
                       *data++ = 0;
                     }
                   }
                 }
                 break;

               /* ---------------- */

               case 48:
                 switch(gbm->bpp)
                 {
                   case 24: /* via downsampling */
                     for (i = 0; i < indices; i++)
                     {
                       const int index = w * hr - ((i) / (wr) + 1) * w + (i) % (wr);

                       *data++ = (gbm_u8) CVT(p2[index] + sOffset2);
                       *data++ = (gbm_u8) CVT(p1[index] + sOffset1);
                       *data++ = (gbm_u8) CVT(p0[index] + sOffset0);

                       if ((i + 1) % wr == 0)
                       {
                         for (pad = (3 * wr) % 4 ? 4 - (3 * wr) % 4 : 0; pad > 0; pad--) /* add padding */
                         {
                            *data++ = 0;
                         }
                       }
                     }
                     break;

                   /* ---------------- */

                   case 48:
                     for (i = 0; i < indices; i++)
                     {
                       const int index = w * hr - ((i) / (wr) + 1) * w + (i) % (wr);

                       *data16++ = p2[index] + sOffset2;
                       *data16++ = p1[index] + sOffset1;
                       *data16++ = p0[index] + sOffset0;
                       data += 6;

                       if ((i + 1) % wr == 0)
                       {
                         for (pad = (6 * wr) % 4 ? 4 - (6 * wr) % 4 : 0; pad > 0; pad--) /* add padding */
                         {
                           *data++ = 0;
                         }
                         data16 = (gbm_u16 *) data;
                       }
                     }
                     break;

                   /* ---------------- */

                   default:
                     j2k_read_deinit(j2k_read);
                     return GBM_ERR_NOT_SUPP;
                 }
                 break;

               /* ---------------- */

               default:
                 j2k_read_deinit(j2k_read);
                 return GBM_ERR_NOT_SUPP;
             }
           }
           break;

         default:
           j2k_read_deinit(j2k_read);
           return GBM_ERR_NOT_SUPP;
       }

       /* cleanup */
       j2k_read_deinit(j2k_read);
    }
    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

static gbm_boolean make_output_palette(const GBMRGB gbmrgb[], gbm_u8 grey[], const char *opt)
{
  const gbm_boolean  k = ( gbm_find_word(opt, "k") != NULL );
  const gbm_boolean  r = ( gbm_find_word(opt, "r") != NULL );
  const gbm_boolean  g = ( gbm_find_word(opt, "g") != NULL );
  const gbm_boolean  b = ( gbm_find_word(opt, "b") != NULL );
  int i;

#define  SW4(a,b,c,d)   ((a)*8+(b)*4+(c)*2+(d))

  switch ( SW4(k,r,g,b) )
  {
    case SW4(0,0,0,0):
      /* Default is the same as "k" */
    case SW4(1,0,0,0):
      for ( i = 0; i < 0x100; i++ )
      {
        grey[i] = (gbm_u8) ( ((gbm_u16) gbmrgb[i].r *  77U +
                              (gbm_u16) gbmrgb[i].g * 150U +
                              (gbm_u16) gbmrgb[i].b *  29U) >> 8 );
      }
      return GBM_TRUE;

    case SW4(0,1,0,0):
      for ( i = 0; i < 0x100; i++ )
      {
        grey[i] = gbmrgb[i].r;
      }
      return GBM_TRUE;

    case SW4(0,0,1,0):
      for ( i = 0; i < 0x100; i++ )
      {
        grey[i] = gbmrgb[i].g;
      }
      return GBM_TRUE;

    case SW4(0,0,0,1):
      for ( i = 0; i < 0x100; i++ )
      {
        grey[i] = gbmrgb[i].b;
      }
      return GBM_TRUE;
  }
  return GBM_FALSE;
}

/* ---------------------------------------- */

/* j2k_w() -  Write bitmap file */
static GBM_ERR j2k_w(const OPJ_CODEC_FORMAT codec_format,
                     const char *fn, int fd, const GBM *gbm,
                     const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
{
    opj_image_cmptparm_t  cmptparms[3];      /* component parameters */
    opj_event_mgr_t       event_mgr;         /* event manager */
    opj_cparameters_t    *parameters = NULL; /* compression parameters */
    opj_image_t          *image      = NULL;
    opj_cinfo_t          *cinfo      = NULL;
    opj_cio_t            *cio        = NULL;

    gbm_u8 grey[0x100] = { 0 };
    const char *s      = NULL;

          int    write_length   = 0;
    const int    subsampling_dx = 1;
    const int    subsampling_dy = 1;
    const size_t stride_src     = ((gbm->w * gbm->bpp + 31) / 32) * 4;
          int    x, y, i_dst;

    fn = fn; /* prevent compiler warning */

    /* prepare the image */
    memset(&cmptparms[0], 0, 3 * sizeof(opj_image_cmptparm_t));
    cmptparms[0].dx   = subsampling_dx;
    cmptparms[0].dy   = subsampling_dy;
    cmptparms[0].w    = gbm->w;
    cmptparms[0].h    = gbm->h;
    cmptparms[0].sgnd = 0;

    switch(gbm->bpp)
    {
        case 8: /* 1 component */
            cmptparms[0].prec = 8;
            cmptparms[0].bpp  = 8;

            if (! make_output_palette(gbmrgb, grey, opt))
            {
              return GBM_ERR_BAD_OPTION;
            }

            image = opj_image_create(1, &cmptparms[0], CLRSPC_GRAY);
            if (image == NULL)
            {
              return GBM_ERR_MEM;
            }

            /* set the color component */
            i_dst = 0;
            for (y = 0; y < gbm->h; y++)
            {
              const gbm_u8 *data_src = data + (stride_src * (gbm->h - y - 1));
              for (x = 0; x < gbm->w; x++)
              {
                image->comps[0].data[i_dst] = grey[*data_src];
                data_src++;
                i_dst++;
              }
            }
            break;

        /* ------------------ */

        case 24: /* 3 components 8bit */
            cmptparms[0].prec = 8;
            cmptparms[0].bpp  = 8;
            cmptparms[1]      = cmptparms[0];
            cmptparms[2]      = cmptparms[0];

            image = opj_image_create(3, &cmptparms[0], CLRSPC_SRGB);
            if (image == NULL)
            {
              return GBM_ERR_MEM;
            }
            /* set the color component */
            i_dst = 0;
            for (y = 0; y < gbm->h; y++)
            {
              const gbm_u8 *data_src = data + (stride_src * (gbm->h - y - 1));
              for (x = 0; x < gbm->w; x++)
              {
                image->comps[2].data[i_dst] = *data_src++; /* B -> R */
                image->comps[1].data[i_dst] = *data_src++; /* G -> G */
                image->comps[0].data[i_dst] = *data_src++; /* R -> B */
                i_dst++;
              }
            }
            break;

        /* ------------------ */

        case 48: /* 3 components 16bit */
            cmptparms[0].prec = 16;
            cmptparms[0].bpp  = 16;
            cmptparms[1]      = cmptparms[0];
            cmptparms[2]      = cmptparms[0];

            image = opj_image_create(3, &cmptparms[0], CLRSPC_SRGB);
            if (image == NULL)
            {
              return GBM_ERR_MEM;
            }
            /* set the color component */
            i_dst = 0;
            for (y = 0; y < gbm->h; y++)
            {
              const gbm_u16 *data16_src = (gbm_u16 *)(data + (stride_src * (gbm->h - y - 1)));
              for (x = 0; x < gbm->w; x++)
              {
                image->comps[2].data[i_dst] = *data16_src++; /* B -> R */
                image->comps[1].data[i_dst] = *data16_src++; /* G -> G */
                image->comps[0].data[i_dst] = *data16_src++; /* R -> B */
                i_dst++;
              }
            }
            break;

        default:
            return GBM_ERR_NOT_SUPP;
    }

    /* set image offset and reference grid */
    image->x0 = 0;
    image->y0 = 0;
    image->x1 = (gbm->w - 1) * subsampling_dy + 1;
    image->y1 = (gbm->h - 1) * subsampling_dy + 1;

    /* configure the event callbacks */
    memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
    event_mgr.error_handler   = j2k_gbm_error_callback;
    event_mgr.warning_handler = j2k_gbm_warning_callback;
    event_mgr.info_handler    = j2k_gbm_info_callback;

    /* set encoding parameters to default values */
    parameters = gbmmem_malloc(sizeof(opj_cparameters_t));
    if (parameters == NULL)
    {
       opj_image_destroy(image);
       return GBM_ERR_MEM;
    }
    opj_set_default_encoder_parameters(parameters);
    if (parameters->tcp_numlayers == 0)
    {
      parameters->tcp_rates[0] = 0; /* ensure default is lossless compression */
      parameters->tcp_numlayers++;
      parameters->cp_disto_alloc = 1;
    }

    /* user defined compression level (rate/distortion) */
    if ((s = gbm_find_word_prefix(opt, "compression=")) != NULL)
    {
        parameters->tcp_rates[0] = 0; /* default lossless compression */

        /* check for collision with quality parameter */
        if (gbm_find_word_prefix(opt, "quality=") != NULL)
        {
           opj_image_destroy(image);
           gbmmem_free(parameters);
           return GBM_ERR_J2K_BAD_MIX_COMPRESSION_QUALITY;
        }
        if (sscanf(s + 12, "%f", &parameters->tcp_rates[0]) != 1)
        {
           opj_image_destroy(image);
           gbmmem_free(parameters);
           return GBM_ERR_J2K_BAD_COMPRESSION;
        }
        if (parameters->tcp_rates[0] < 0.0f)
        {
           opj_image_destroy(image);
           gbmmem_free(parameters);
           return GBM_ERR_J2K_BAD_COMPRESSION_RANGE;
        }
        parameters->cp_disto_alloc = 1;
    }

    /* user defined quality level (fixed quality) */
    if ((s = gbm_find_word_prefix(opt, "quality=")) != NULL)
    {
        parameters->tcp_distoratio[0] = 100.0f; /* default best quality */

        /* check for collision with compression parameter */
        if (gbm_find_word_prefix(opt, "compression=") != NULL)
        {
           opj_image_destroy(image);
           gbmmem_free(parameters);
           return GBM_ERR_J2K_BAD_MIX_COMPRESSION_QUALITY;
        }
        if (sscanf(s + 8, "%f", &parameters->tcp_distoratio[0]) != 1)
        {
           opj_image_destroy(image);
           gbmmem_free(parameters);
           return GBM_ERR_J2K_BAD_QUALITY;
        }
        if ((parameters->tcp_distoratio[0] < 0.0f) ||
            (parameters->tcp_distoratio[0] > 100.0f))
        {
           opj_image_destroy(image);
           gbmmem_free(parameters);
           return GBM_ERR_J2K_BAD_QUALITY_RANGE;
        }
        parameters->cp_fixed_quality = 1;
        parameters->cp_disto_alloc   = 0;
    }

    /* Decide if MCT should be used */
    parameters->tcp_mct = (image->numcomps == 3) ? 1 : 0;

    /* Write a comment */
    parameters->cp_comment = NULL;
    if ((s = gbm_find_word_prefix(opt, "comment=")) != NULL)
    {
       parameters->cp_comment = gbmmem_calloc(strlen(s + 8) + 1, sizeof(char));
       if (sscanf(s + 8, "%[^\"]", parameters->cp_comment) != 1)
       {
          if (sscanf(s + 8, "%[^ ]", parameters->cp_comment) != 1)
          {
             opj_image_destroy(image);
             gbmmem_free(parameters->cp_comment);
             gbmmem_free(parameters);
             return GBM_ERR_J2K_BAD_COMMENT;
          }
       }
    }

    /* create the decompression scheme */
    cinfo = opj_create_compress(codec_format);
    if (cinfo == NULL)
    {
       opj_image_destroy(image);
       gbmmem_free(parameters->cp_comment);
       gbmmem_free(parameters);
       return GBM_ERR_MEM;
    }

    /* catch events using our callbacks and give a local context */
    opj_set_event_mgr((opj_common_ptr)cinfo, &event_mgr, NULL);

    /* setup the decoder decoding parameters using user parameters */
    opj_setup_encoder(cinfo, parameters, image);

    /* open a byte stream */
    cio = opj_cio_open((opj_common_ptr)cinfo, NULL, 0);
    if (cio == NULL)
    {
       opj_destroy_compress(cinfo);
       opj_image_destroy(image);
       gbmmem_free(parameters->cp_comment);
       gbmmem_free(parameters);
       return GBM_ERR_MEM;
    }

    /* encode the image */
    if (! opj_encode(cinfo, cio, image, NULL))
    {
       opj_cio_close(cio);
       opj_destroy_compress(cinfo);
       opj_image_destroy(image);
       gbmmem_free(parameters->cp_comment);
       gbmmem_free(parameters);
       return GBM_ERR_NOT_SUPP;
    }

    write_length = cio_tell(cio);

    /* write the encoded buffer to the file system */
    if (gbm_file_write(fd, cio->buffer, write_length) != write_length)
    {
       opj_cio_close(cio);
       opj_destroy_compress(cinfo);
       opj_image_destroy(image);
       gbmmem_free(parameters->cp_comment);
       gbmmem_free(parameters);
       return GBM_ERR_WRITE;
    }

    opj_cio_close(cio);
    opj_destroy_compress(cinfo);
    opj_image_destroy(image);
    gbmmem_free(parameters->cp_comment);
    gbmmem_free(parameters);

    return GBM_ERR_OK;
}

/* ------------ */

/* JP2: JPEG-2000 file format */
/* j2k_jp2_w() -  Write bitmap file */
GBM_ERR j2k_jp2_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
{
   return j2k_w(CODEC_JP2, fn, fd, gbm, gbmrgb, data, opt);
}

/* ----------------------------------------------------------- */

/* J2K: JPEG-2000 codestream */
/* j2k_j2k_w() -  Write bitmap file */
GBM_ERR j2k_j2k_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
{
   return j2k_w(CODEC_J2K, fn, fd, gbm, gbmrgb, data, opt);
}

/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */
/* ----------------------------------------------------------- */

/* j2k_err - map error code to error message */
const char *j2k_err(GBM_ERR rc)
{
   J2K_GBMERR_MSG *j2k_errmsg_p = j2k_errmsg;

   while (j2k_errmsg_p->error_message)
   {
     if (j2k_errmsg_p->rc == rc)
     {
       return j2k_errmsg_p->error_message;
     }
     j2k_errmsg_p++;
   }

   return NULL;
}

#endif

