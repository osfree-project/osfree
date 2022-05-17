/*

gbmtga.c - Truevision Targa/Vista support

Supported formats and options:
------------------------------
TGA : Truevision Targa/Vista support : .TGA .VST .AFI

Reads uncompressed and runlength compressed types.

Standard formats (backward compatible):
  Reads  1 bpp bw/palette/greyscale files and presents them as 1 bpp.
  Reads  4 bpp palette/greyscale files and presents them as 4 bpp.
  Reads  8 bpp palette/greyscale files and presents them as 8 bpp.
  Reads 16 bpp colour files and presents them as 24 bpp.
  Reads 24 bpp colour files and presents them as 24 bpp.
  Reads 32 bpp colour files (RGB + alpha channel) and presents them as 24 bpp.

Extended formats (not backward compatible, import option ext_bpp required):
  Reads 32 bpp colour files (RGB + alpha channel) and presents them as 32 bpp.

Writes uncompressed and runlength compressed 1, 4 and 8bpp grayscale and palette files as 8bpp.
Writes 16, 24, 32 (24 bpp + alpha) bpp RGB files.

  Input:
  ------

  Can specify that non-standard GBM color depths are exported (also with alpha channel)
    Input option: ext_bpp (default: bpp is downsampled to 24 bpp)


  Output:
  -------

  1, 4 and 8 bit data is written out as a colour mapped file or, if black/white or
  grayscale palette is detected, a TGA file without palette is written.

  Can request writing RLE compressed data. The default is to write uncompressed data.
    Output options: rle

  Can request writing as 16 bpp data. If the data is 24 bpp, then the 16 flag causes
  only the most significant 5 bits of the red, green and blue to be written in a 16
  bit pixel. An existing alpha channel is ignored.
  This option is only valid for 24 and 32 bpp data.
    Output options: 16

  Can request writing as 24 bpp data. An existing alpha channel is ignored.
  This option is only valid for 24 and 32 bpp data.
    Output options: 24 (default when source data is 24 bit)

  Can request writing as 32 bpp data. If the data is 24 bpp, then the 32 flag causes
  the data to be padded with 8 one bits of alpha channel to make a 32 bit pixel.
  If there is a real alpha channel provided (32 bpp data), its data is used.
  This option is only valid for 24 and 32 bpp data.
    Output options: 32 (default when source data is 32 bit)

  Write image data flipped vertically. This does not have a visual effect but
  is rather an internal data format option. This might help to increase compatibility
  to other programs.
    Output options: ydown (default: yup)


History:
--------
(Heiko Nitzsche)

22-Feb-2006: Move format description strings to gbmdesc.h
26-Sep-2006: Fix RLE8 decoder (wrong pointer used for ydown reading)
             Add support for reading horizontally flipped TGAs
             Reject unsupported interleaving schemes (except non interleaved)
01-Oct-2006: Add support for reading 32bpp TGAs with alpha channel if option ext_bpp is set.
             Add support for writing 32bpp TGAs keeping the alpha channel if source data is 32bpp.
             Add read and write validation checks.
15-Aug-2008: Integrate new GBM types
25-Feb-2012: Add support for reading and writing uncompressed 1bpp gray/bw/palette images
             Add support for reading and writing uncompressed 4bpp gray/palette images
             Add support for writing 8bpp uncompressed gray images
             Add support for writing RLE compressed images
             Fix write bugs (e.g. uninitialized alpha channel for 24bpp -> 32bpp)
*/

#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmhelp.h"
#include "gbmdesc.h"
#include "gbmmem.h"

/* ------------------------------------ */

#define  low_byte(w)    ((gbm_u8)      (           (w)&0x00ff)    )
#define  high_byte(w)   ((gbm_u8)      (((unsigned)(w)&0xff00)>>8))
#define  make_word(a,b) (((gbm_u16)a) + (((gbm_u16)b) << 8))
#define  SW3(a,b,c)     ((a)*4+(b)*2+(c))

/* ------------------------------------ */

static GBMFT tga_gbmft =
{
    GBM_FMT_DESC_SHORT_TGA,
    GBM_FMT_DESC_LONG_TGA,
    GBM_FMT_DESC_EXT_TGA,
     GBM_FT_R1|GBM_FT_R4|GBM_FT_R8|GBM_FT_R24|GBM_FT_R32|
     GBM_FT_W1|GBM_FT_W4|GBM_FT_W8|GBM_FT_W24|GBM_FT_W32
};

/* ------------------------------------ */

#define  GBM_ERR_TGA_BAD_BPP         ((GBM_ERR) 400)
#define  GBM_ERR_TGA_BAD_TYPE        ((GBM_ERR) 401)
#define  GBM_ERR_TGA_BAD_PAL         ((GBM_ERR) 402)
#define  GBM_ERR_TGA_BAD_INTERLEAVE  ((GBM_ERR) 403)

/* ------------------------------------ */

/*

A Targa file is a header, followed by an identification string, followed by
a color map, followed by the data for the image. Both the identification
string and the color-map can be of zero length.

*/

#define    TGA_NO_IMAGE       0        /* No image data included in file    */
#define    TGA_UNCOMP_CM      1        /* Uncompressed, Color-Mapped (VDA/D */
                                       /* and Targa M-8 images)             */
#define    TGA_UNCOMP_RGB     2        /* Uncompressed, RGB images (eg: ICB */
                                       /* Targa 16, 24 and 32)              */
#define    TGA_UNCOMP_BW      3        /* Uncompressed, B/W images (eg:     */
                                       /* Targa 8 and Targa M-8 images)     */
#define    TGA_RL_CM          9        /* Run-length, Color-Mapped (VDA/D   */
                                       /* and Targa M-8 images)             */
#define    TGA_RL_RGB        10        /* Run-length, RGB images (eg: ICB   */
                                       /* Targa 16, 24 and 32)              */
#define    TGA_RL_BW         11        /* Run-length, B/W images (eg: Targa */
                                       /* 8 and Targa M-8)                  */
#define    TGA_COMP_CM       32        /* Compressed Color-Mapped (VDA/D)   */
                                       /* data using Huffman, Delta, and    */
                                       /* run length encoding               */
#define    TGA_COMP_CM_4     33        /* Compressed Color-Mapped (VDA/D)   */
                                       /* data using Huffman, Delta, and    */
                                       /* run length encoding in 4 passes   */

#define    IDB_ATTRIBUTES    0x0f       /* How many attrib bits per pixel    */
                                        /* ie: 1 for T16, 8 for T32          */
#define    IDB_RIGHTTOLEFT   0x10       /* Mirrored right to left */
                                        /* else its left to right */
#define    IDB_ORIGIN       0x20        /* Origin in top left corner bit     */
                                        /* else its in bottom left corner    */
#define    IDB_INTERLEAVE    0xc0       /* Interleave bits as defined below  */
#define    IDB_NON_INT       0x00       /* Non-Interlaced                    */
#define    IDB_2_WAY         0x40       /* 2 way (even/odd) interleaving     */
#define    IDB_4_WAY         0x80       /* 4 way interleaving (eg: AT&T PC)  */

/* ------------------------------------ */

#pragma pack(1)
typedef struct
{
    gbm_u8 n_chars_in_id;           /* Length of identification text     */
    gbm_u8 color_map_present;       /* 0 means no, 1 yes                 */
    gbm_u8 image_type;              /* Type of image file, one of TGA_   */
    gbm_u8 color_map_start_low;     /* These 5 bytes are only valid if   */
    gbm_u8 color_map_start_high;    /* color_map_present is 1. They      */
    gbm_u8 color_map_length_low;    /* Specify the size of the color map */
    gbm_u8 color_map_length_high;   /* and where it starts from          */
    gbm_u8 color_map_entry_bits;    /* Bits per color map entry          */
                                  /* Typically 15, 16, 24 or 32        */
    gbm_u8 x_origin_low;
    gbm_u8 x_origin_high;
    gbm_u8 y_origin_low;
    gbm_u8 y_origin_high;
    gbm_u8 width_low;
    gbm_u8 width_high;
    gbm_u8 height_low;
    gbm_u8 height_high;
    gbm_u8 bpp;                     /* Typically 16, 24 or 32            */
    gbm_u8 image_descriptor;        /* Split into IDB_ bits              */
} TGA_HEADER;
#pragma pack()

#define    SIZEOF_TGA_HEADER 18

/* ------------------------------------ */

static void t24_t32(gbm_u8 *dest, const gbm_u8 *src, int n)
{
    while ( n-- )
    {
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = 0xff;
    }
}

/* ------------------------------------ */

static void t32_t24(gbm_u8 *dest, const gbm_u8 *src, int n)
{
    while ( n-- )
    {
        *dest++ = *src++;
        *dest++ = *src++;
        *dest++ = *src++;
        src++;
    }
}

/* ------------------------------------ */

static void t24_t16(gbm_u8 *dest, const gbm_u8 *src, int n)
{
    while ( n-- )
    {
        gbm_u16    b = (gbm_u16) (*src++ & 0xf8);
        gbm_u16    g = (gbm_u16) (*src++ & 0xf8);
        gbm_u16    r = (gbm_u16) (*src++ & 0xf8);
        gbm_u16    w = ((r << 7) | (g << 2) | (b >> 3));

        *dest++ = (gbm_u8)  w;
        *dest++ = (gbm_u8) (w >> 8);
    }
}

/* ------------------------------------ */

static void t32_t16(gbm_u8 *dest, const gbm_u8 *src, int n)
{
    while ( n-- )
    {
        gbm_u16    b = (gbm_u16) (*src++ & 0xf8);
        gbm_u16    g = (gbm_u16) (*src++ & 0xf8);
        gbm_u16    r = (gbm_u16) (*src++ & 0xf8);
        gbm_u16    w = ((r << 7) | (g << 2) | (b >> 3));

        src++; /* discard alpha channel */

        *dest++ = (gbm_u8)  w;
        *dest++ = (gbm_u8) (w >> 8);
    }
}

/* ------------------------------------ */

static void t16_t24(gbm_u8 *dest, const gbm_u8 *src, int n)
{
    while ( n-- )
    {
        gbm_u16 l = *src++;
        gbm_u16 h = *src++;
        gbm_u16 w = l + (h << 8);

        *dest++ = (gbm_u8) ((w & 0x001fU) << 3);
        *dest++ = (gbm_u8) ((w & 0x03e0U) >> 2);
        *dest++ = (gbm_u8) ((w & 0x7c00U) >> 7);
    }
}

/* ------------------------------------ */

static void ref_horz_32(gbm_u8 *dst, gbm_u8 *src, int n)
{
    dst += n * 4;
    while ( n-- )
    {
        dst -= 4;
        dst[0] = *src++;
        dst[1] = *src++;
        dst[2] = *src++;
        dst[3] = *src++;
    }
}

/* ------------------------------------ */

static void ref_horz_24(gbm_u8 *dst, gbm_u8 *src, int n)
{
    dst += n * 3;
    while ( n-- )
    {
        dst -= 3;
        dst[0] = *src++;
        dst[1] = *src++;
        dst[2] = *src++;
    }
}

/* ------------------------------------ */

static void ref_horz_8(gbm_u8 *dst, gbm_u8 *src, int n)
{
    dst += n;
    while ( n-- )
        *(--dst) = *src++;
}

/* ------------------------------------ */

static void ref_horz_4(gbm_u8 *dst, const gbm_u8 *src, int n)
{
    /* build lookup table */
    gbm_u8 rev4[0x100];
    unsigned int i;
    for ( i = 0; i < 0x100; i++ )
    {
        rev4[i] = (gbm_u8) ( ((i & 0x0fU) << 4) | ((i & 0xf0U) >> 4) );
    }

    /* reflect */
    if ( (n & 1) == 0 )
    {
        n /= 2;
        dst += n;
        while ( n-- )
        {
            *(--dst) = rev4[*src++];
        }
    }
    else
    {
        n /= 2;
        src += n;
        while ( n-- )
        {
            *dst    = (gbm_u8) (*(src--) & 0xf0);
            *dst++ |= (gbm_u8) (* src    & 0x0f);
        }
        *dst = (gbm_u8) (*src & 0xf0);
    }
}

/* ------------------------------------ */

static void ref_horz_1(gbm_u8 *dst, const gbm_u8 *src, int n)
{
    const int last = ( n & 7 );

    gbm_u8 rev[0x100];        /* Reverses all bits in a gbm_u8 */
    gbm_u8 rev_top[7][0x100]; /* Reverses top N bits of a gbm_u8 */
    gbm_u8 rev_bot[7][0x100]; /* Reverses bottom N bits of a gbm_u8 */

    /* build lookup table */
    unsigned int i;
    for ( i = 0; i < 0x100; i++ )
    {
        unsigned int j, p, q, b = 0;

        for ( p = 0x01U, q = 0x80U; p < 0x100U; p <<= 1, q >>= 1 )
        {
            if ( i & p ) b |= q;
        }
        rev[i] = b;

        for ( j = 1; j < 8U; j++ )
        {
            gbm_u8 l = 0, r = 0;
            gbm_u8 lm = 0x80U, lmr = (gbm_u8) (0x80U >> (j - 1));
            gbm_u8 rm = 0x01U, rmr = (gbm_u8) (0x01U << (j - 1));
            unsigned int k;

            for ( k = 0; k < j; k++ )
            {
                if ( i & (lm >> k) ) l |= (lmr << k);
                if ( i & (rm << k) ) r |= (rmr >> k);
            }
            rev_top[j-1][i] = l;
            rev_bot[j-1][i] = r;
        }
    }

    /* reflect */
    n >>= 3;
    if ( last == 0 )
    {
        dst += n;
        while ( n-- )
        {
            *(--dst) = rev[*src++];
        }
    }
    else
    {
        static const gbm_u8 lmask[8] = { 0, 0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe };
        const gbm_u8 *lrev = rev_top[   last -1];
        const gbm_u8 *rrev = rev_bot[(8-last)-1];
        const gbm_u8 lm = lmask[last], rm = 0xff - lm;

        src += n;
        while ( n-- )
        {
            *dst    = lrev[*(src--) & lm];
            *dst++ |= rrev[* src    & rm];
        }
        *dst = lrev[*src & lm];
    }
}

/* ------------------------------------ */

static gbm_boolean ref_horz_1_4_8_24_32(const GBM *gbm, gbm_u8 *data)
{
    const size_t stride = ( ((gbm->w * gbm->bpp + 31)/32) * 4 );
    int y;
    gbm_u8 *p = data;
    gbm_u8 *tmp;

    if ( (tmp = gbmmem_malloc(stride)) == NULL )
        return GBM_FALSE;

    switch ( gbm->bpp )
    {
        case 32:
            for ( y = 0; y < gbm->h; y++, p += stride )
            {
                ref_horz_32(tmp, p, gbm->w);
                memcpy(p, tmp, stride);
            }
            break;
        case 24:
            for ( y = 0; y < gbm->h; y++, p += stride )
            {
                ref_horz_24(tmp, p, gbm->w);
                memcpy(p, tmp, stride);
            }
            break;
        case 8:
            for ( y = 0; y < gbm->h; y++, p += stride )
            {
                ref_horz_8(tmp, p, gbm->w);
                memcpy(p, tmp, stride);
            }
            break;
        case 4:
            for ( y = 0; y < gbm->h; y++, p += stride )
            {
                ref_horz_4(tmp, p, gbm->w);
                memcpy(p, tmp, stride);
            }
            break;
        case 1:
            for ( y = 0; y < gbm->h; y++, p += stride )
            {
                ref_horz_1(tmp, p, gbm->w);
                memcpy(p, tmp, stride);
            }
            break;
        default:
            gbmmem_free(tmp);
            return GBM_FALSE;
    }

    gbmmem_free(tmp);
    return GBM_TRUE;
}

/* ------------------------------------ */

#pragma pack(1)
typedef struct
{
    TGA_HEADER header;
} TGA_PRIV;
#pragma pack()

/* ------------------------------------ */

GBM_ERR tga_qft(GBMFT *gbmft)
{
    *gbmft = tga_gbmft;
    return GBM_ERR_OK;
}

/* ------------------------------------ */

GBM_ERR tga_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
{
    TGA_PRIV *priv = (TGA_PRIV *) gbm->priv;

    fn=fn; opt=opt; /* Suppress 'unref arg' compiler warnings */

    gbm_file_read(fd, (char *) &(priv->header), SIZEOF_TGA_HEADER);

    switch ( priv->header.image_type )
    {
        case TGA_UNCOMP_BW:
        case TGA_RL_BW:
            if ((priv->header.bpp != 1) && (priv->header.bpp != 4) && (priv->header.bpp != 8))
                return GBM_ERR_TGA_BAD_BPP;
            break;

        case TGA_UNCOMP_CM:
        case TGA_RL_CM:
            if ((priv->header.bpp != 1) && (priv->header.bpp != 4) && (priv->header.bpp != 8))
                return GBM_ERR_TGA_BAD_BPP;
            if ( priv->header.color_map_entry_bits != 24 )
                return GBM_ERR_TGA_BAD_PAL;
            break;

        case TGA_RL_RGB:
        case TGA_UNCOMP_RGB:
            if ( priv->header.bpp != 16 &&
                 priv->header.bpp != 24 &&
                 priv->header.bpp != 32 )
                return GBM_ERR_TGA_BAD_BPP;
            break;
        default:
            return GBM_ERR_TGA_BAD_TYPE;
    }

    if ( (priv->header.image_descriptor & IDB_INTERLEAVE) != IDB_NON_INT )
    {
        /* interlacing is not yet supported */
        return GBM_ERR_TGA_BAD_INTERLEAVE;
    }

    gbm->w = make_word(priv->header.width_low , priv->header.width_high );
    gbm->h = make_word(priv->header.height_low, priv->header.height_high);

    if ( gbm->w <= 0 || gbm->h <= 0 )
        return GBM_ERR_BAD_SIZE;

    /* check if extended color depths are requested */
    gbm->bpp = priv->header.bpp;
    if (gbm_find_word(opt, "ext_bpp") != NULL)
    {
        if (priv->header.bpp == 32)
        {
            gbm->bpp = 32;
        }
        else if ((priv->header.bpp > 8) && (priv->header.bpp < 32))
        {
            gbm->bpp = 24;
        }
    }
    else if (priv->header.bpp > 8)
    {
        gbm->bpp = 24;
    }

    return GBM_ERR_OK;
}

/* ------------------------------------ */

GBM_ERR tga_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
{
    TGA_PRIV *priv = (TGA_PRIV *) gbm->priv;

    switch ( priv->header.image_type )
    {
        /*...sTGA_UNCOMP_BW\44\ TGA_RL_BW:16:*/
        case TGA_UNCOMP_BW:
        case TGA_RL_BW:
        {
            /* init palette */
            int increment, i, x;
            const int gbm_palette_entries = 1 << gbm->bpp;
            memset(gbmrgb, 0, (sizeof(GBMRGB) * gbm_palette_entries));

            switch(gbm->bpp)
            {
                case 1:
                    increment = 255;
                    break;

                case 4:
                   increment = 17;
                   break;

                default:
                    increment = 1;
                    break;
            }
            i = gbm_palette_entries;
            x = 255;
            while (i > 0)
            {
                i--;
                gbmrgb[i].r = gbmrgb[i].g = gbmrgb[i].b = x;
                x -= increment;
            }
        }
        break;

        /*...sTGA_UNCOMP_CM\44\ TGA_RL_CM:16:*/
        case TGA_UNCOMP_CM:
        case TGA_RL_CM:
        {
            int color_map_start, color_map_length, i;
            const int gbm_palette_entries = 1 << gbm->bpp;

            gbm_file_lseek(fd, (long) (SIZEOF_TGA_HEADER+priv->header.n_chars_in_id), GBM_SEEK_SET);

            color_map_start  = make_word(priv->header.color_map_start_low , priv->header.color_map_start_high );
            color_map_length = make_word(priv->header.color_map_length_low, priv->header.color_map_length_high);

            if (gbm_palette_entries >= (color_map_start + color_map_length))
            {
                memset(gbmrgb, 0, sizeof(GBMRGB) * gbm_palette_entries);
                for ( i = color_map_start; i < color_map_start + color_map_length; i++ )
                {
                    gbm_u8 b[3];

                    if ( gbm_file_read(fd, (char *) b, 3) != 3 )
                        return GBM_ERR_READ;
                    gbmrgb[i].b = b[0];
                    gbmrgb[i].g = b[1];
                    gbmrgb[i].r = b[2];
                }
            }
        }
        break;
    }

    return GBM_ERR_OK;
}

/* ------------------------------------ */

GBM_ERR tga_rdata(int fd, GBM *gbm, gbm_u8 *data)
{
    TGA_PRIV *priv = (TGA_PRIV *) gbm->priv;
    gbm_u8 *p;
    int x, y;
    int stride;

    gbm_file_lseek(fd, (long) (SIZEOF_TGA_HEADER + priv->header.n_chars_in_id), GBM_SEEK_SET);

    if ( priv->header.color_map_present )
    {
        int color_map_length;
        int bpp_pal = priv->header.color_map_entry_bits;
            /* Valid values are 32, 24, 16 and sometimes 15 */

        if ( bpp_pal == 15 ) bpp_pal = 16;

        color_map_length = make_word(priv->header.color_map_length_low, priv->header.color_map_length_high);
        gbm_file_lseek(fd, (long) ((color_map_length * bpp_pal) / 8L), GBM_SEEK_CUR);
    }

    stride = ((gbm->w*gbm->bpp + 31) / 32) * 4;
    p      = data;

    if ( (priv->header.image_descriptor & IDB_ORIGIN) != 0 )
    {
        p += (size_t)stride * (gbm->h-1);
        stride = -stride;
    }

    #define checked_read_ahead(ahead, v) \
     { const int r = gbm_read_ahead(ahead); \
       if ( r == -1 ) \
       {  gbm_destroy_ahead(ahead); \
          return GBM_ERR_READ; \
       } \
       v = (gbm_u8) r; \
     }

    switch ( priv->header.image_type )
    {
        case TGA_RL_CM:
        case TGA_RL_BW:
        case TGA_RL_RGB:
        {
            AHEAD *ahead;
            x = 0; y = 0;
            if ( (ahead = gbm_create_ahead(fd)) == NULL )
            {
                return GBM_ERR_MEM;
            }
            switch ( priv->header.bpp )
            {
                case 1:
                    while ( y < gbm->h )
                    {
                        gbm_u8 v;
                        int bitShift = 7;
                        int i, cnt = gbm_read_ahead(ahead);
                        if ( cnt == -1 )
                        {
                            gbm_destroy_ahead(ahead);
                            return GBM_ERR_READ;
                        }
                        if ( cnt & 0x80 )
                        {
                            checked_read_ahead(ahead, v);
                            for ( i = 0x80; i <= cnt; i++ )
                            {
                                if (7 == bitShift)
                                   { p[x] = 0; }
                                p[x] |= ((v & 0x1) << bitShift);
                                if (0 == bitShift)
                                   { bitShift = 7; ++x; }
                                else
                                   { --bitShift; }

                                if ( x == gbm->w )
                                   { x = 0; y++; p += stride; bitShift = 7; }
                            }
                        }
                        else
                        {
                            for ( i = 0; i <= cnt; i++ )
                            {
                                checked_read_ahead(ahead, v);
                                if (7 == bitShift)
                                   { p[x] = 0; }
                                p[x] |= ((v & 0x1) << bitShift);
                                if (0 == bitShift)
                                   { bitShift = 7; ++x; }
                                else
                                   { --bitShift; }

                                if ( x == gbm->w )
                                   { x = 0; y++; p += stride; bitShift = 7; }
                            }
                        }
                    }
                    break;

                /* --------------------------------- */

                case 4:
                    while ( y < gbm->h )
                    {
                        gbm_u8 v;
                        int bitShift = 4;
                        int i, cnt = gbm_read_ahead(ahead);
                        if ( cnt == -1 )
                        {
                            gbm_destroy_ahead(ahead);
                            return GBM_ERR_READ;
                        }
                        if ( cnt & 0x80 )
                        {
                            checked_read_ahead(ahead, v);
                            for ( i = 0x80; i <= cnt; i++ )
                            {
                                if (4 == bitShift)
                                   { p[x] = 0; }
                                p[x] |= ((v & 0xf) << bitShift);
                                if (0 == bitShift)
                                   { bitShift = 4; ++x; }
                                else
                                   { bitShift = 0; }

                                if ( x == gbm->w )
                                   { x = 0; y++; p += stride; bitShift = 4; }
                            }
                        }
                        else
                        {
                            for ( i = 0; i <= cnt; i++ )
                            {
                                checked_read_ahead(ahead, v);
                                if (4 == bitShift)
                                   { p[x] = 0; }
                                p[x] |= ((v & 0xf) << bitShift);
                                if (0 == bitShift)
                                   { bitShift = 4; ++x; }
                                else
                                   { bitShift = 0; }

                                if ( x == gbm->w )
                                   { x = 0; y++; p += stride; bitShift = 4; }
                            }
                        }
                    }
                    break;

                /* --------------------------------- */

                case 8:
                    while ( y < gbm->h )
                    {
                        gbm_u8 v;
                        int i, cnt = gbm_read_ahead(ahead);
                        if ( cnt == -1 )
                        {
                            gbm_destroy_ahead(ahead);
                            return GBM_ERR_READ;
                        }
                        if ( cnt & 0x80 )
                        {
                            checked_read_ahead(ahead, v);
                            for ( i = 0x80; i <= cnt; i++ )
                            {
                                p[x++] = v;
                                if ( x == gbm->w )
                                   { x = 0; y++; p += stride; }
                            }
                        }
                        else
                        {
                            for ( i = 0; i <= cnt; i++ )
                            {
                                checked_read_ahead(ahead, p[x]); ++x;
                                if ( x == gbm->w )
                                   { x = 0; y++; p += stride; }
                            }
                        }
                    }
                    break;

                /* --------------------------------- */

                /*
                We have 3 bytes per pixel in the data array, indexed by p.
                We will read 2 bytes per pixel into the right hand 2/3 of each line.
                Then we will expand leftwards to fill the full width afterwards.
                */
                case 16:
                    p += gbm->w;
                    while ( y < gbm->h )
                    {
                        int i, cnt = gbm_read_ahead(ahead);
                        if ( cnt == -1 )
                        {
                            gbm_destroy_ahead(ahead);
                            return GBM_ERR_READ;
                        }
                        if ( cnt & 0x80 )
                        {
                            gbm_u8 v1, v2;
                            checked_read_ahead(ahead, v1);
                            checked_read_ahead(ahead, v2);
                            for ( i = 0x80; i <= cnt; i++ )
                            {
                                p[x++] = v1;
                                p[x++] = v2;
                                if ( x == gbm->w*2 )
                                { x = 0; y++; p += stride; }
                            }
                        }
                        else
                            for ( i = 0; i <= cnt; i++ )
                            {
                                checked_read_ahead(ahead, p[x]); ++x;
                                checked_read_ahead(ahead, p[x]); ++x;
                                if ( x == gbm->w*2 )
                                { x = 0; y++; p += stride; }
                            }
                    }

                    if ( stride < 0 )
                        stride = -stride;
                    for ( y = 0, p = data; y < gbm->h; y++, p += stride )
                        t16_t24(p, p+gbm->w, gbm->w);

                    break;

                /* --------------------------------- */

                case 24:
                    while ( y < gbm->h )
                    {
                        int i, cnt = gbm_read_ahead(ahead);
                        if ( cnt == -1 )
                        {
                            gbm_destroy_ahead(ahead);
                            return GBM_ERR_READ;
                        }
                        if ( cnt & 0x80 )
                        {
                            gbm_u8 v1, v2, v3;
                            checked_read_ahead(ahead, v1);
                            checked_read_ahead(ahead, v2);
                            checked_read_ahead(ahead, v3);
                            for ( i = 0x80; i <= cnt; i++ )
                            {
                                p[x++] = v1;
                                p[x++] = v2;
                                p[x++] = v3;
                                if ( x == gbm->w*3 )
                                { x = 0; y++; p += stride; }
                            }
                        }
                        else
                            for ( i = 0; i <= cnt; i++ )
                            {
                                checked_read_ahead(ahead, p[x]); ++x;
                                checked_read_ahead(ahead, p[x]); ++x;
                                checked_read_ahead(ahead, p[x]); ++x;
                                if ( x == gbm->w*3 )
                                { x = 0; y++; p += stride; }
                            }
                    }
                    break;

                /* --------------------------------- */

                case 32:
                    while ( y < gbm->h )
                    {
                        int i, cnt = gbm_read_ahead(ahead);
                        if ( cnt == -1 )
                        {
                            gbm_destroy_ahead(ahead);
                            return GBM_ERR_READ;
                        }
                        if ( cnt & 0x80 )
                        {
                            gbm_u8 v1, v2, v3, v4;
                            checked_read_ahead(ahead, v1);
                            checked_read_ahead(ahead, v2);
                            checked_read_ahead(ahead, v3);
                            if (gbm->bpp == 32)
                            {
                                checked_read_ahead(ahead, v4); /* alpha */
                                for ( i = 0x80; i <= cnt; i++ )
                                {
                                    p[x++] = v1;
                                    p[x++] = v2;
                                    p[x++] = v3;
                                    p[x++] = v4;
                                    if ( x == gbm->w*4 )
                                    { x = 0; y++; p += stride; }
                                }
                            }
                            else
                            {
                                /* Discard alpha channel */
                                if (-1 == gbm_read_ahead(ahead))
                                {
                                    gbm_destroy_ahead(ahead);
                                    return GBM_ERR_READ;
                                }
                                for ( i = 0x80; i <= cnt; i++ )
                                {
                                    p[x++] = v1;
                                    p[x++] = v2;
                                    p[x++] = v3;
                                    if ( x == gbm->w*3 )
                                    { x = 0; y++; p += stride; }
                                }
                            }
                        }
                        else
                        {
                            if (gbm->bpp == 32)
                            {
                                for ( i = 0; i <= cnt; i++ )
                                {
                                    checked_read_ahead(ahead, p[x]); ++x;
                                    checked_read_ahead(ahead, p[x]); ++x;
                                    checked_read_ahead(ahead, p[x]); ++x;
                                    checked_read_ahead(ahead, p[x]); ++x;
                                    if ( x == gbm->w*4 )
                                    { x = 0; y++; p += stride; }
                                }
                            }
                            else
                            {
                                for ( i = 0; i <= cnt; i++ )
                                {
                                    checked_read_ahead(ahead, p[x]); ++x;
                                    checked_read_ahead(ahead, p[x]); ++x;
                                    checked_read_ahead(ahead, p[x]); ++x;
                                    /* Discard alpha channel */
                                    if (-1 == gbm_read_ahead(ahead))
                                    {
                                        gbm_destroy_ahead(ahead);
                                        return GBM_ERR_READ;
                                    }
                                    if ( x == gbm->w*3 )
                                    { x = 0; y++; p += stride; }
                                }
                            }
                        }
                    }
                    break;

                /* --------------------------------- */

                default:
                    gbm_destroy_ahead(ahead);
                    return GBM_ERR_NOT_SUPP;
            }
            gbm_destroy_ahead(ahead);
        }
        break;

        /* ------------------- */

        case TGA_UNCOMP_BW:
        case TGA_UNCOMP_CM:
        case TGA_UNCOMP_RGB:
            x = 0; y = 0;
            switch ( priv->header.bpp )
            {
                case 1:
                {
                    gbm_u8 v;
                    AHEAD *ahead = gbm_create_ahead(fd);
                    if ( NULL == ahead )
                    {
                        return GBM_ERR_MEM;
                    }
                    for ( y = 0; y < gbm->h; y++, p += stride )
                    {
                        int j        = 0;
                        int bitShift = 7;
                        p[0] = 0;
                        for ( x = 0; x < gbm->w; x++ )
                        {
                            checked_read_ahead(ahead, v);
                            p[j] |= ((v & 0x1) << bitShift);
                            if (0 == bitShift)
                               { bitShift = 7; ++j; p[j] = 0; }
                            else
                               { --bitShift; }
                        }
                    }
                    gbm_destroy_ahead(ahead);
                    break;
                }

                /* --------------------------------- */

                case 4:
                {
                    gbm_u8 v;
                    AHEAD *ahead = gbm_create_ahead(fd);
                    if ( NULL == ahead )
                    {
                        return GBM_ERR_MEM;
                    }
                    for ( y = 0; y < gbm->h; y++, p += stride )
                    {
                        int j        = 0;
                        int bitShift = 4;
                        p[0] = 0;
                        for ( x = 0; x < gbm->w; x++ )
                        {
                            checked_read_ahead(ahead, v);
                            p[j] |= ((v & 0xf) << bitShift);
                            if (0 == bitShift)
                               { bitShift = 4; ++j; p[j] = 0; }
                            else
                               { bitShift = 0; }
                        }
                    }
                    gbm_destroy_ahead(ahead);
                    break;
                }

                /* --------------------------------- */

                case 8:
                    for ( y = 0; y < gbm->h; y++, p += stride )
                    {
                        if ( gbm_file_read(fd, p, gbm->w) != gbm->w )
                            return GBM_ERR_READ;
                    }
                    break;

                /* --------------------------------- */

                case 16:
                    for ( y = 0; y < gbm->h; y++, p += stride )
                    {
                        if ( gbm_file_read(fd, p+gbm->w, gbm->w * 2) != gbm->w * 2 )
                            return GBM_ERR_READ;
                        t16_t24(p, p+gbm->w, gbm->w);
                    }
                    break;

                /* --------------------------------- */

                case 24:
                    for ( y = 0; y < gbm->h; y++, p += stride )
                    {
                        if ( gbm_file_read(fd, p, gbm->w * 3) != gbm->w * 3 )
                            return GBM_ERR_READ;
                    }
                    break;

                /* --------------------------------- */

                case 32:
                    if (gbm->bpp != 32)
                    {
                        const int lineBytes = gbm->w * 4;
                        gbm_u8 *linebuf = gbmmem_malloc(lineBytes);
                        if ( NULL == linebuf )
                            return GBM_ERR_MEM;

                        for ( y = 0; y < gbm->h; y++, p += stride )
                        {
                            if ( gbm_file_read(fd, linebuf, lineBytes) != lineBytes )
                            {
                                gbmmem_free(linebuf);
                                return GBM_ERR_READ;
                            }
                            /* strip alpha channel */
                            t32_t24(p, linebuf, gbm->w);
                        }
                        gbmmem_free(linebuf);
                    }
                    else
                    {
                        const int lineBytes = gbm->w * 4;
                        for ( y = 0; y < gbm->h; y++, p += stride )
                        {
                            if ( gbm_file_read(fd, p, lineBytes) != lineBytes )
                                return GBM_ERR_READ;
                        }
                    }
                    break;

                /* --------------------------------- */

                default:
                    return GBM_ERR_NOT_SUPP;
            }
            break;

        /* --------------------------------- */

        default:
            return GBM_ERR_NOT_SUPP;
    }
    #undef checked_read_ahead

    /* Mirror right to left? */
    if ( (priv->header.image_descriptor & IDB_RIGHTTOLEFT) != 0 )
    {
        if (! ref_horz_1_4_8_24_32(gbm, data))
        {
            return GBM_ERR_READ;
        }
    }
    return GBM_ERR_OK;
}

/* --------------------------------- */
/* --------------------------------- */

/*
 * Retrieve a pixel value from a buffer.  The actual size and order
 * of the bytes is not important since we are only using the value
 * for comparisons with other pixels.
 */
static gbm_u32 getPixel(const gbm_u8 *src, int bytesPerPixel)
{
    gbm_u32 pixel = (gbm_u32)*src++;
    while (bytesPerPixel-- > 1)
    {
        pixel <<= 8;
        pixel  |= (gbm_u32)*src++;
    }
    return pixel;
}

/* --------------------------------- */

static int countSamePixels(const gbm_u8 *src, const int bytesPerPixel, int pixelCount)
{
    gbm_u32 pixel     = 0;
    gbm_u32 nextPixel = 0;
    int     n = 1;

    pixel = getPixel(src, bytesPerPixel);
    pixelCount--;
    while (pixelCount > 0)
    {
        src += bytesPerPixel;
        nextPixel = getPixel(src, bytesPerPixel);
        if (nextPixel != pixel) break;
        ++n;
        --pixelCount;
    }
    return n;
}

/* --------------------------------- */

static int countDiffPixels(const gbm_u8 *src, const int bytesPerPixel, int pixelCount)
{
    gbm_u32 pixel     = 0;
    gbm_u32 nextPixel = 0;
    int     n = 0;

    if (pixelCount == 1) return pixelCount;
    pixel = getPixel(src, bytesPerPixel);
    while (pixelCount > 1)
    {
        src += bytesPerPixel;
        nextPixel = getPixel(src, bytesPerPixel);
        if (nextPixel == pixel) break;
        pixel = nextPixel;
        ++n;
        --pixelCount;
    }
    if ( nextPixel == pixel ) return n;
    return(n + 1);
}

/* --------------------------------- */

static int RLEncodeRow(const gbm_u8 *src, gbm_u8 *dst, int n, const int bytesPerPixel)
{
    int diffCount  = 0; /* pixel count until two identical */
    int sameCount  = 0; /* number of identical adjacent pixels */
    int rleBufSize = 0; /* count of number of bytes encoded */

    while (n > 0) /* for all pixels */
    {
        diffCount = countDiffPixels( src, bytesPerPixel, n );
        sameCount = countSamePixels( src, bytesPerPixel, n );
        if (diffCount > 128) diffCount = 128;
        if (sameCount > 128) sameCount = 128;
        if (diffCount >   0)
        {
            /* create a raw packet */
            *dst++ = (gbm_u8)(diffCount - 1);
            n -= diffCount;
            rleBufSize += (diffCount * bytesPerPixel) + 1;
            while (diffCount > 0)
            {
                *dst++ = *src++;
                if ( bytesPerPixel > 1 ) *dst++ = *src++;
                if ( bytesPerPixel > 2 ) *dst++ = *src++;
                if ( bytesPerPixel > 3 ) *dst++ = *src++;
                --diffCount;
            }
        }
        if (sameCount > 1)
        {
            /* create a RLE packet */
            *dst++ = (gbm_u8)((sameCount - 1) | 0x80);
            n -= sameCount;
            rleBufSize += bytesPerPixel + 1;
            src += (sameCount - 1) * bytesPerPixel;
            *dst++ = *src++;
            if ( bytesPerPixel > 1 ) *dst++ = *src++;
            if ( bytesPerPixel > 2 ) *dst++ = *src++;
            if ( bytesPerPixel > 3 ) *dst++ = *src++;
        }
    }
    return( rleBufSize );
}

/* --------------------------------- */

GBM_ERR tga_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
{
    TGA_HEADER tga_header;
    int i, j, obpp, stride;
    const gbm_u8 *p;
          gbm_u8 *linebuf    = NULL;
          gbm_u8 *linebufRle = NULL;

    const gbm_boolean o16   = ( gbm_find_word(opt, "16"   ) != NULL );
    const gbm_boolean o24   = ( gbm_find_word(opt, "24"   ) != NULL );
    const gbm_boolean o32   = ( gbm_find_word(opt, "32"   ) != NULL );
    const gbm_boolean yup   = ( gbm_find_word(opt, "yup"  ) != NULL );
    const gbm_boolean ydown = ( gbm_find_word(opt, "ydown") != NULL );
    const gbm_boolean orle  = ( gbm_find_word(opt, "rle"  ) != NULL );

    fn=fn; /* Suppress 'unref arg' compiler warning */

    if (gbm->bpp == 32)
    {
        switch ( SW3(o16,o24,o32) )
        {
            case SW3(1,0,0):
              obpp = 16;
              break;

            case SW3(0,1,0):
              obpp = 24;
              break;

            case SW3(0,0,0):
            case SW3(0,0,1):
              obpp = 32;
              break;

            default:
              return GBM_ERR_BAD_OPTION;
        }
    }
    else if (gbm->bpp == 24)
    {
        switch ( SW3(o16,o24,o32) )
        {
            case SW3(1,0,0):
              obpp = 16;
              break;

            case SW3(0,0,0):
            case SW3(0,1,0):
              obpp = 24;
              break;

            case SW3(0,0,1):
              obpp = 32;
              break;

            default:
              return GBM_ERR_BAD_OPTION;
        }
    }
    else
    {
        if (o16 || o24 || o32 || ((gbm->bpp != 1) && (gbm->bpp != 4) && (gbm->bpp != 8)))
        {
            return GBM_ERR_BAD_OPTION;
        }
        obpp = 8; /* always export as 8bit data bits per pixel even if only a few are used */
    }
    if ( yup && ydown )
    {
        return GBM_ERR_BAD_OPTION;
    }

    tga_header.n_chars_in_id = 0;
    if (gbm->bpp == 1)
    {
        /* check if the palette is black/white */
        tga_header.image_type = orle ? TGA_RL_CM : TGA_UNCOMP_CM;
        if ((gbmrgb[0].r == gbmrgb[0].g) && (gbmrgb[0].r == gbmrgb[0].b) && (gbmrgb[0].g == gbmrgb[0].b) &&
            (gbmrgb[1].r == gbmrgb[1].g) && (gbmrgb[1].r == gbmrgb[1].b) && (gbmrgb[1].g == gbmrgb[1].b))
        {
            if (((gbmrgb[0].r == 0)    && (gbmrgb[1].r == 0xff)) ||
                ((gbmrgb[0].r == 0xff) && (gbmrgb[1].r == 0)))
            {
                tga_header.image_type = orle ? TGA_RL_BW : TGA_UNCOMP_BW;
            }
        }
    }
    else if (gbm->bpp == 4)
    {
        /* Check if the provided palette maps to the fixed grayscale levels
         * and then skip exporting the palette.
         */
        tga_header.image_type = orle ? TGA_RL_BW : TGA_UNCOMP_BW;
        for (i = 0; i < 16; ++i)
        {
            if ((gbmrgb[i].r == gbmrgb[i].g) && (gbmrgb[i].r == gbmrgb[i].b) && (gbmrgb[i].g == gbmrgb[i].b))
            {
                if (gbmrgb[i].r != (i * 17))
                {
                    tga_header.image_type = orle ? TGA_RL_CM : TGA_UNCOMP_CM;
                    break;
                }
            }
            else
            {
                tga_header.image_type = orle ? TGA_RL_CM : TGA_UNCOMP_CM;
                break;
            }
        }
    }
    else if (gbm->bpp == 8)
    {
        /* Check if the provided palette maps to the fixed grayscale levels
         * and then skip exporting the palette.
         */
        tga_header.image_type = orle ? TGA_RL_BW : TGA_UNCOMP_BW;
        for (i = 0; i < 256; ++i)
        {
            if ((gbmrgb[i].r == gbmrgb[i].g) && (gbmrgb[i].r == gbmrgb[i].b) && (gbmrgb[i].g == gbmrgb[i].b))
            {
                if (gbmrgb[i].r != i)
                {
                    tga_header.image_type = orle ? TGA_RL_CM : TGA_UNCOMP_CM;
                    break;
                }
            }
            else
            {
                tga_header.image_type = orle ? TGA_RL_CM : TGA_UNCOMP_CM;
                break;
            }
        }
    }
    else
    {
        tga_header.image_type = orle ? TGA_RL_RGB : TGA_UNCOMP_RGB;
    }
    tga_header.x_origin_low          = low_byte(0);
    tga_header.x_origin_high         = high_byte(0);
    tga_header.y_origin_low          = low_byte(0);
    tga_header.y_origin_high         = high_byte(0);
    tga_header.color_map_start_low   = low_byte(0);
    tga_header.color_map_start_high  = high_byte(0);
    if ((TGA_UNCOMP_CM == tga_header.image_type) ||
        (TGA_RL_CM     == tga_header.image_type))
    {
        const unsigned int palette_entries = 1 << gbm->bpp;
        tga_header.color_map_present     = (gbm_u8) 1;
        tga_header.color_map_length_low  = low_byte(palette_entries);
        tga_header.color_map_length_high = high_byte(palette_entries);
        tga_header.color_map_entry_bits  = 24;
    }
    else
    {
        tga_header.color_map_present     = (gbm_u8) 0;
        tga_header.color_map_length_low  = low_byte(0);
        tga_header.color_map_length_high = high_byte(0);
        tga_header.color_map_entry_bits  = 0;
    }
    tga_header.width_low         = low_byte(gbm->w);
    tga_header.width_high        = high_byte(gbm->w);
    tga_header.height_low        = low_byte(gbm->h);
    tga_header.height_high       = high_byte(gbm->h);
    tga_header.bpp               = (gbm_u8) obpp;
    tga_header.image_descriptor  = IDB_NON_INT;

    if ( ydown )
    {
        tga_header.image_descriptor |= IDB_ORIGIN;
    }

    if (gbm_file_write(fd, (char *) &tga_header, SIZEOF_TGA_HEADER) != SIZEOF_TGA_HEADER)
    {
        return GBM_ERR_WRITE;
    }

    switch ( obpp )
    {
        case 8:
            switch(gbm->bpp)
            {
                case 1:
                {
                    gbm_u8 valueOff = 0;
                    gbm_u8 valueOn  = 0;
                    const int lineBytes = gbm->w;
                    stride = ((gbm->w * gbm->bpp + 31)/32) * 4;
                    p = data;
                    if ( ydown )
                    {
                        p += ((size_t)stride * (gbm->h - 1));
                        stride = -stride;
                    }
                    if ((TGA_UNCOMP_BW == tga_header.image_type) ||
                        (TGA_RL_BW     == tga_header.image_type))
                    {
                        /* black/white */
                        if (gbmrgb[0].r == 0xff)
                        { valueOff = 255; valueOn  =   0; }
                        else
                        { valueOff =   0; valueOn  = 255; }
                    }
                    else /* colormap */
                    {
                        gbm_u8 b[6] = { 0 };
                        b[0] = gbmrgb[0].b;
                        b[1] = gbmrgb[0].g;
                        b[2] = gbmrgb[0].r;
                        b[3] = gbmrgb[1].b;
                        b[4] = gbmrgb[1].g;
                        b[5] = gbmrgb[1].r;
                        if (gbm_file_write(fd, b, 6) != 6)
                        {
                            return GBM_ERR_WRITE;
                        }
                        valueOff = 0;
                        valueOn  = 1;
                    }
                    /* write the data */
                    linebuf = gbmmem_malloc((size_t) lineBytes);
                    if (linebuf == NULL) return GBM_ERR_MEM;

                    linebufRle = NULL;
                    if (orle)
                    {
                        linebufRle = gbmmem_malloc((size_t) gbm->w * 2);
                        if (linebufRle == NULL)
                        {
                            gbmmem_free(linebuf);
                            return GBM_ERR_MEM;
                        }
                    }
                    for ( i = 0; i < gbm->h; i++ )
                    {
                        int    x       = 0;
                        gbm_u8 bitMask = 0x80;
                        for ( j = 0; j < gbm->w; j++ )
                        {
                            linebuf[j] = ((p[x] & bitMask) != 0) ? valueOn : valueOff;
                            if (0x1 == bitMask)
                               { bitMask = 0x80; ++x; }
                               else
                               { bitMask >>= 1; }
                        }
                        if (orle)
                        {
                            const int rleLineBytes = RLEncodeRow(linebuf, linebufRle, gbm->w, 1);
                            if (gbm_file_write(fd, linebufRle, rleLineBytes) != rleLineBytes)
                            {
                                gbmmem_free(linebuf);
                                gbmmem_free(linebufRle);
                                return GBM_ERR_WRITE;
                            }
                        }
                        else
                        {
                            if (gbm_file_write(fd, linebuf, lineBytes) != lineBytes)
                            {
                                gbmmem_free(linebuf);
                                gbmmem_free(linebufRle);
                                return GBM_ERR_WRITE;
                            }
                        }
                        p += stride;
                    }
                    gbmmem_free(linebuf);
                    gbmmem_free(linebufRle);
                }
                break;

                /* --------------------- */

                case 4:
                {
                    const int lineBytes = gbm->w;
                    stride = ((gbm->w * gbm->bpp + 31)/32) * 4;
                    p = data;
                    if ( ydown )
                    {
                        p += ((size_t)stride * (gbm->h - 1));
                        stride = -stride;
                    }
                    linebuf = gbmmem_malloc((size_t) lineBytes);
                    if (linebuf == NULL) return GBM_ERR_MEM;

                    linebufRle = NULL;
                    if (orle)
                    {
                        linebufRle = gbmmem_malloc((size_t) gbm->w * 2);
                        if (linebufRle == NULL)
                        {
                            gbmmem_free(linebuf);
                            return GBM_ERR_MEM;
                        }
                    }
                    if ((TGA_UNCOMP_BW == tga_header.image_type) ||
                        (TGA_RL_BW     == tga_header.image_type))
                    {
                        /* write as graylevel */
                        gbm_u8 gray8[16] = { 0 };
                        for ( i = 0; i < 16; ++i )
                        { gray8[i] = i * 17; }

                        /* write the data */
                        for ( i = 0; i < gbm->h; i++ )
                        {
                            int    x       = 0;
                            gbm_u8 bitMask = 0xf0;
                            for ( j = 0; j < gbm->w; j++ )
                            {
                                const gbm_u8 v = p[x] & bitMask;
                                if (0x0f == bitMask)
                                   { linebuf[j] = gray8[v]; bitMask = 0xf0; ++x; }
                                   else
                                   { linebuf[j] = gray8[v >> 4]; bitMask = 0x0f; }
                            }
                            if (orle)
                            {
                                const int rleLineBytes = RLEncodeRow(linebuf, linebufRle, gbm->w, 1);
                                if (gbm_file_write(fd, linebufRle, rleLineBytes) != rleLineBytes)
                                {
                                    gbmmem_free(linebuf);
                                    gbmmem_free(linebufRle);
                                    return GBM_ERR_WRITE;
                                }
                            }
                            else
                            {
                                if (gbm_file_write(fd, linebuf, lineBytes) != lineBytes)
                                {
                                    gbmmem_free(linebuf);
                                    gbmmem_free(linebufRle);
                                    return GBM_ERR_WRITE;
                                }
                            }
                            p += stride;
                        }
                    }
                    else /* write with color map */
                    {
                        gbm_u8 b[16*3] = { 0 };
                        int boff = 0;
                        for ( i = 0; i < 16; i++ )
                        {
                            b[boff++] = gbmrgb[i].b;
                            b[boff++] = gbmrgb[i].g;
                            b[boff++] = gbmrgb[i].r;
                        }
                        if (gbm_file_write(fd, b, 16*3) != 16*3)
                        {
                            gbmmem_free(linebuf);
                            gbmmem_free(linebufRle);
                            return GBM_ERR_WRITE;
                        }
                        /* write the data */
                        for ( i = 0; i < gbm->h; i++ )
                        {
                            int    x       = 0;
                            gbm_u8 bitMask = 0xf0;
                            /* write color indexes */
                            for ( j = 0; j < gbm->w; j++ )
                            {
                                const gbm_u8 v = p[x] & bitMask;
                                if (0x0f == bitMask)
                                   { linebuf[j] = v; bitMask = 0xf0; ++x; }
                                   else
                                   { linebuf[j] = v >> 4; bitMask = 0x0f; }
                            }
                            if (orle)
                            {
                                const int rleLineBytes = RLEncodeRow(linebuf, linebufRle, gbm->w, 1);
                                if (gbm_file_write(fd, linebufRle, rleLineBytes) != rleLineBytes)
                                {
                                    gbmmem_free(linebuf);
                                    gbmmem_free(linebufRle);
                                    return GBM_ERR_WRITE;
                                }
                            }
                            else
                            {
                                if (gbm_file_write(fd, linebuf, lineBytes) != lineBytes)
                                {
                                    gbmmem_free(linebuf);
                                    gbmmem_free(linebufRle);
                                    return GBM_ERR_WRITE;
                                }
                            }
                            p += stride;
                        }
                    }
                    gbmmem_free(linebuf);
                    gbmmem_free(linebufRle);
                }
                break;

                /* --------------------- */

                case 8:
                    if ((TGA_UNCOMP_CM == tga_header.image_type) ||
                        (TGA_RL_CM     == tga_header.image_type))
                    {
                        gbm_u8 b[0x300] = { 0 };
                        int boff = 0;
                        if (gbm->bpp != 8)
                        {
                            return GBM_ERR_BAD_OPTION;
                        }
                        for ( i = 0; i < 0x100; i++ )
                        {
                            b[boff++] = gbmrgb[i].b;
                            b[boff++] = gbmrgb[i].g;
                            b[boff++] = gbmrgb[i].r;
                        }
                        if (gbm_file_write(fd, b, 0x300) != 0x300)
                        {
                            return GBM_ERR_WRITE;
                        }
                    }
                    stride = ((gbm->w + 3) & ~3);
                    p = data;
                    if ( ydown )
                    {
                        p += ((size_t)stride * (gbm->h - 1));
                        stride = -stride;
                    }
                    linebufRle = NULL;
                    if (orle)
                    {
                        linebufRle = gbmmem_malloc((size_t) gbm->w * 2);
                        if (linebufRle == NULL)
                        {
                            return GBM_ERR_MEM;
                        }
                    }
                    for ( i = 0; i < gbm->h; i++ )
                    {
                        if (orle)
                        {
                            const int rleLineBytes = RLEncodeRow(p, linebufRle, gbm->w, 1);
                            if (gbm_file_write(fd, linebufRle, rleLineBytes) != rleLineBytes)
                            {
                                gbmmem_free(linebufRle);
                                return GBM_ERR_WRITE;
                            }
                        }
                        else
                        {
                            if (gbm_file_write(fd, p, gbm->w) != gbm->w)
                            {
                                gbmmem_free(linebufRle);
                                return GBM_ERR_WRITE;
                            }
                        }
                        p += stride;
                    }
                    gbmmem_free(linebufRle);
                    break;

                /* --------------------- */

                default:
                    return GBM_ERR_NOT_SUPP;
            }
            break;

        /* --------------------------------- */

        case 16:
        {
            const int writelen = gbm->w * 2;
            linebuf = gbmmem_malloc((size_t) writelen);
            if (linebuf == NULL)
                return GBM_ERR_MEM;

            linebufRle = NULL;
            if (orle)
            {
                linebufRle = gbmmem_malloc((size_t) gbm->w * 3);
                if (linebufRle == NULL)
                {
                    gbmmem_free(linebuf);
                    return GBM_ERR_MEM;
                }
            }
            stride = (gbm->bpp == 32) ? (gbm->w * 4) : ((gbm->w * 3 + 3) & ~3);
            p = data;
            if ( ydown )
            {
                p += ((size_t)stride * (gbm->h - 1));
                stride = -stride;
            }
            for ( i = 0; i < gbm->h; i++ )
            {
                if (gbm->bpp == 32)
                  t32_t16(linebuf, p, gbm->w);
                else
                  t24_t16(linebuf, p, gbm->w);

                if (orle)
                {
                    const int rleLineBytes = RLEncodeRow(linebuf, linebufRle, gbm->w, 2);
                    if (gbm_file_write(fd, linebufRle, rleLineBytes) != rleLineBytes)
                    {
                        gbmmem_free(linebuf);
                        gbmmem_free(linebufRle);
                        return GBM_ERR_WRITE;
                    }
                }
                else
                {
                    if (gbm_file_write(fd, linebuf, writelen) != writelen)
                    {
                        gbmmem_free(linebuf);
                        gbmmem_free(linebufRle);
                        return GBM_ERR_WRITE;
                    }
                }
                p += stride;
            }
            gbmmem_free(linebuf);
            gbmmem_free(linebufRle);
        }
        break;

        /* --------------------------------- */

        case 24:
        {
            const int writelen = gbm->w * 3;
            stride = (gbm->bpp == 32) ? (gbm->w * 4) : ((gbm->w * 3 + 3) & ~3);
            p = data;
            if ( ydown )
            {
                p += ((size_t)stride * (gbm->h - 1));
                stride = -stride;
            }

            linebufRle = NULL;
            if (orle)
            {
                linebufRle = gbmmem_malloc((size_t) gbm->w * 4);
                if (linebufRle == NULL)
                {
                    return GBM_ERR_MEM;
                }
            }
            if (gbm->bpp == 32)
            {
                gbm_u8 *linebuf = gbmmem_malloc((size_t) writelen);
                if (linebuf == NULL)
                {
                    gbmmem_free(linebufRle);
                    return GBM_ERR_MEM;
                }
                for ( i = 0; i < gbm->h; i++ )
                {
                    t32_t24(linebuf, p, gbm->w);
                    if (orle)
                    {
                        const int rleLineBytes = RLEncodeRow(linebuf, linebufRle, gbm->w, 3);
                        if (gbm_file_write(fd, linebufRle, rleLineBytes) != rleLineBytes)
                        {
                            gbmmem_free(linebuf);
                            gbmmem_free(linebufRle);
                            return GBM_ERR_WRITE;
                        }
                    }
                    else
                    {
                        if (gbm_file_write(fd, linebuf, writelen) != writelen)
                        {
                            gbmmem_free(linebuf);
                            gbmmem_free(linebufRle);
                            return GBM_ERR_WRITE;
                        }
                    }
                    p += stride;
                }
                gbmmem_free(linebuf);
            }
            else
            {
                for ( i = 0; i < gbm->h; i++ )
                {
                    if (orle)
                    {
                        const int rleLineBytes = RLEncodeRow(p, linebufRle, gbm->w, 3);
                        if (gbm_file_write(fd, linebufRle, rleLineBytes) != rleLineBytes)
                        {
                            gbmmem_free(linebufRle);
                            return GBM_ERR_WRITE;
                        }
                    }
                    else
                    {
                        if (gbm_file_write(fd, p, writelen) != writelen)
                        {
                            gbmmem_free(linebufRle);
                            return GBM_ERR_WRITE;
                        }
                    }
                    p += stride;
                }
            }
            gbmmem_free(linebufRle);
        }
        break;

        /* --------------------------------- */

        case 32:
        {
            const int writelen = gbm->w * 4;
            linebufRle = NULL;
            if (orle)
            {
                linebufRle = gbmmem_malloc((size_t) gbm->w * 5);
                if (linebufRle == NULL)
                {
                    return GBM_ERR_MEM;
                }
            }
            if (gbm->bpp == 32)
            {
                stride = gbm->w * 4;
                p = data;
                if ( ydown )
                {
                    p += ((size_t)stride * (gbm->h - 1));
                    stride = -stride;
                }
                for ( i = 0; i < gbm->h; i++ )
                {
                    if (orle)
                    {
                        const int rleLineBytes = RLEncodeRow(p, linebufRle, gbm->w, 4);
                        if (gbm_file_write(fd, linebufRle, rleLineBytes) != rleLineBytes)
                        {
                            gbmmem_free(linebufRle);
                            return GBM_ERR_WRITE;
                        }
                    }
                    else
                    {
                        if (gbm_file_write(fd, p, writelen) != writelen)
                        {
                            gbmmem_free(linebufRle);
                            return GBM_ERR_WRITE;
                        }
                    }
                    p += stride;
                }
            }
            else
            {
                gbm_u8 *linebuf = gbmmem_malloc((size_t) writelen);
                if ( NULL == linebuf )
                {
                    gbmmem_free(linebufRle);
                    return GBM_ERR_MEM;
                }
                stride = ((gbm->w * 3 + 3) & ~3);
                p = data;
                if ( ydown )
                {
                    p += ((size_t)stride * (gbm->h - 1));
                    stride = -stride;
                }
                for ( i = 0; i < gbm->h; i++ )
                {
                    t24_t32(linebuf, p, gbm->w);
                    if (orle)
                    {
                        const int rleLineBytes = RLEncodeRow(linebuf, linebufRle, gbm->w, 4);
                        if (gbm_file_write(fd, linebufRle, rleLineBytes) != rleLineBytes)
                        {
                            gbmmem_free(linebuf);
                            gbmmem_free(linebufRle);
                            return GBM_ERR_WRITE;
                        }
                    }
                    else
                    {
                        if (gbm_file_write(fd, linebuf, writelen) != writelen)
                        {
                            gbmmem_free(linebuf);
                            gbmmem_free(linebufRle);
                            return GBM_ERR_WRITE;
                        }
                    }
                    p += stride;
                }
                gbmmem_free(linebuf);
            }
            gbmmem_free(linebufRle);
        }
        break;

        /* --------------------------------- */

        default:
            return GBM_ERR_NOT_SUPP;
    }

    return GBM_ERR_OK;
}

/* --------------------------------- */

const char *tga_err(GBM_ERR rc)
{
    switch ( (int) rc )
    {
        case GBM_ERR_TGA_BAD_BPP:
            return "bits per pixel not 1, 4, 8, 16, 24 or 32";
        case GBM_ERR_TGA_BAD_TYPE:
            return "unsupported compression type for bits per pixel";
        case GBM_ERR_TGA_BAD_PAL:
            return "color map entry size not 24 bits per pixel";
        case GBM_ERR_TGA_BAD_INTERLEAVE:
            return "unsupported interleaving scheme";
    }
    return NULL;
}


