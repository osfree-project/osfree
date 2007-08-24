/*

gbmbmp.c - OS/2 1.1, 1.2, 2.0, Windows 3.x, Windows 9x, Windows NT/2000/XP (BMP only) support

* Reads and writes any OS/2 1.x bitmap.
* Reads uncompressed, RLE4, RLE8 and RLE24 OS/2 and Windows bitmaps (BI_RGB, BI_BITFIELDS)

There are horrific file structure alignment considerations hence each
word,dword is read individually.

If input is an OS/2 bitmaparray file then which bitmap can be specified.
Input options: index=# (default: 0)

Can be made to write OS/2 1.1 or Windows 3.0 / OS/2 2.0 bitmaps
Output options: 1.1,win,2.0 (default: 2.0, note win=2.0)

For 1bpp bitmaps the forground and the background colours can be inverted:

Foreground and background colours in palette can be inverted on input.
Input options: inv

Foreground and background colours in palette can be inverted on output.
Output options: inv

Foreground and background colours in bits can be inverted on input.
Input options: invb

Foreground and background colours in bits can be inverted on output.
Output options: invb

The best option for writing 1bpp bitmaps is to write the darkest
colour as the foreground. The resulting bitmap will have darkest
colour as 1 bits, and lightest colour as 0 bits. When reloaded
WPFolders and GBM will give a black 1's on white 0's picture.
Output option: darkfg

Also, there is an option for writing 1bpp bitmaps with the lightest
colour as foreground. As WPFolder and GBM assume the foreground is
black and the background is white on reloading, this will typically
cause the image to be inverted.
Output option: lightfg


History:
--------
(Heiko Nitzsche)

19-Feb-2006: Add function to query number of images
21-Feb-2006: Add support for reading RLE24 encoded bitmaps
22-Feb-2006: Move format description strings to gbmdesc.h
27-May-2006: - Add support for reading Windows 16bpp and 32bpp
               BI_RGB and BI_BITFIELDS encoded bitmaps. Writing
               them will not be supported as Windows can also read
               standard 24bpp BMPs without this strange encoding.
               Also these formats mostly aren't supported by other platforms.
             - Fix the strange handling of 1bpp bitmaps. Now the colour
               palette is honored just as in all other formats.
22-Jul-2006: Prevent possible buffer overflows in 4,8,24bpp RLE decoders due to
             badly encoded bitmap.


TODO:
- read BCA_HUFFMAN1D (OS/2)

*/

#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"
#include "gbmhelp.h"
#include "gbmdesc.h"

#ifndef min
  #define min(a,b)  (((a)<(b))?(a):(b))
#endif

#define  low_byte(w)    ((byte)  ((w)&0x00ff)    )
#define  high_byte(w)   ((byte) (((w)&0xff00)>>8))
#define  make_word(a,b) (((word)a) + (((word)b) << 8))

/* ------------------------------------------------- */

typedef struct
{
    dword mask_b;
    dword mask_g;
    dword mask_r;

    dword bitoffset_b;
    dword bitoffset_g;
    dword bitoffset_r;

    dword valid_bits_b;
    dword valid_bits_g;
    dword valid_bits_r;

} BMP_COLOR_MASKS;

typedef struct
{
    dword base;
    BOOLEAN windows;
    dword cbFix;
    dword ulCompression;
    dword cclrUsed;
    dword offBits;
    BOOLEAN inv, invb;
    word  cBitCount;
    BMP_COLOR_MASKS cMasks;

} BMP_PRIV;

/* ------------------------------------------------- */

static GBM_ERR read_16bpp_data(int fd, GBM *gbm, byte *data,
                               const BMP_COLOR_MASKS * maskdef);
static GBM_ERR read_32bpp_data(int fd, GBM *gbm, byte *data,
                               const BMP_COLOR_MASKS * maskdef);

static GBM_ERR read_rle24_data(int fd, GBM *gbm, byte *data);
static GBM_ERR read_rle8_data (int fd, GBM *gbm, byte *data);
static GBM_ERR read_rle4_data (int fd, GBM *gbm, byte *data);

/* ------------------------------------------------- */

static BOOLEAN read_word(int fd, word *w)
{
    byte low = 0, high = 0;

    if ( gbm_file_read(fd, (char *) &low, 1) != 1 )
        return FALSE;
    if ( gbm_file_read(fd, (char *) &high, 1) != 1 )
        return FALSE;
    *w = (word) (low + ((word) high << 8));
    return TRUE;
}

static BOOLEAN read_dword(int fd, dword *d)
{
    word low, high;
    if ( !read_word(fd, &low) )
        return FALSE;
    if ( !read_word(fd, &high) )
        return FALSE;
    *d = low + ((dword) high << 16);
    return TRUE;
}

static BOOLEAN write_word(int fd, word w)
{
    byte    low  = (byte) w;
    byte    high = (byte) (w >> 8);

    gbm_file_write(fd, &low, 1);
    gbm_file_write(fd, &high, 1);
    return TRUE;
}

static BOOLEAN write_dword(int fd, dword d)
{
    write_word(fd, (word) d);
    write_word(fd, (word) (d >> 16));
    return TRUE;
}

static GBMFT bmp_gbmft =
{
    GBM_FMT_DESC_SHORT_BMP,
    GBM_FMT_DESC_LONG_BMP,
    GBM_FMT_DESC_EXT_BMP,
    GBM_FT_R1|GBM_FT_R4|GBM_FT_R8|GBM_FT_R24|
    GBM_FT_W1|GBM_FT_W4|GBM_FT_W8|GBM_FT_W24,
};

#define GBM_ERR_BMP_PLANES    ((GBM_ERR) 300)
#define GBM_ERR_BMP_BITCOUNT  ((GBM_ERR) 301)
#define GBM_ERR_BMP_CBFIX     ((GBM_ERR) 302)
#define GBM_ERR_BMP_COMP      ((GBM_ERR) 303)
#define GBM_ERR_BMP_OFFSET    ((GBM_ERR) 304)

/* ------------------------------------------------- */

#define BFT_BMAP          0x4d42
#define BFT_BITMAPARRAY   0x4142
#define BCA_UNCOMP        0x00000000L /* BI_RGB  */
#define BCA_RLE8          0x00000001L /* BI_RLE8 */
#define BCA_RLE4          0x00000002L /* BI_RLE4 */
#define BCA_RLE24         0x00000004L /* OS/2 only */

#define BI_BITFIELDS      0x00000003L /* color only, same code as BCA_HUFFFMAN1D */
                                      /* Take care for detection and use cBitCount (16,32) additionally! */

#if 0 /* not yet supported */
#define BCA_HUFFFMAN1D    0x00000003L /* blackwhite only, same code as BI_BITFIELDS */
                                      /* Take care for detection and use cBitCount (1) additionally! */
#endif

#define MSWCC_EOL         0
#define MSWCC_EOB         1
#define MSWCC_DELTA       2

/* ------------------------------------------------- */

static void invert(byte *buffer, unsigned count)
{
    while ( count-- )
        *buffer++ ^= (byte) 0xffU;
}

static void swap_pal(GBMRGB *gbmrgb)
{
    GBMRGB tmp = gbmrgb[0];
    gbmrgb[0] = gbmrgb[1];
    gbmrgb[1] = tmp;
}

static dword count_mask_bits(dword mask, dword * bitoffset)
{
  dword testmask = 1; /* start with the least significant bit */
  dword counter  = 0;
  dword index    = 0;

  *bitoffset = 0;

  /* find offset of first bit */
  while (((mask & testmask) == 0) && (index < 31))
  {
    index++;
    testmask <<= 1;
  }
  *bitoffset = index;

  /* count the bits set in the rest of the mask */
  while ((testmask <= mask) && (index < 31))
  {
    if (mask & testmask)
    {
      counter++;
    }
    index++;
    testmask <<= 1;
  }

  return counter;
}

/* ------------------------------------------------- */

GBM_ERR bmp_qft(GBMFT *gbmft)
{
    *gbmft = bmp_gbmft;
    return GBM_ERR_OK;
}

/* ------------------------------------------------- */

/* Read number of bitmaps in BMP file. */
GBM_ERR bmp_rimgcnt(const char *fn, int fd, int *pimgcnt)
{
    word usType;
    fn=fn; /* Suppress 'unref arg' compiler warnings */

    if ( !read_word(fd, &usType) )
    {
        return GBM_ERR_READ;
    }
    if ( usType == BFT_BITMAPARRAY )
    {
        int i;
        for(i = 1;;i++)
        {
            dword cbSize2, offNext;

            if ( !read_dword(fd, &cbSize2) ) break;
            if ( !read_dword(fd, &offNext) ) break;
            if ( offNext == 0L ) break;

            gbm_file_lseek(fd, (long) offNext, GBM_SEEK_SET);

            if ( !read_word(fd, &usType) ) break;
            if ( usType != BFT_BITMAPARRAY ) break;
        }
        *pimgcnt = i;
    }
    else
    {
       *pimgcnt = 1;
    }

    return GBM_ERR_OK;
}

/* ------------------------------------------------- */

GBM_ERR bmp_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
{
    word usType, xHotspot, yHotspot;
    dword cbSize, offBits, cbFix;
    BMP_PRIV *bmp_priv = (BMP_PRIV *) gbm->priv;
    bmp_priv->inv  = ( gbm_find_word(opt, "inv" ) != NULL );
    bmp_priv->invb = ( gbm_find_word(opt, "invb") != NULL );

    fn=fn; /* Suppress 'unref arg' compiler warnings */

    if ( !read_word(fd, &usType) )
        return GBM_ERR_READ;
    if ( usType == BFT_BITMAPARRAY )
    /*...shandle bitmap arrays:16:*/
    {
       const char *index;
       int i;

       if ( (index = gbm_find_word_prefix(opt, "index=")) != NULL )
          sscanf(index + 6, "%d", &i);
       else
          i = 0;

       while ( i-- > 0 )
       {
          dword cbSize2, offNext;

          if ( !read_dword(fd, &cbSize2) )
             return GBM_ERR_READ;
          if ( !read_dword(fd, &offNext) )
             return GBM_ERR_READ;
          if ( offNext == 0L )
             return GBM_ERR_BMP_OFFSET;
          gbm_file_lseek(fd, (long) offNext, GBM_SEEK_SET);
          if ( !read_word(fd, &usType) )
             return GBM_ERR_READ;
          if ( usType != BFT_BITMAPARRAY )
             return GBM_ERR_BAD_MAGIC;
       }
       gbm_file_lseek(fd, 4L + 4L + 2L + 2L, GBM_SEEK_CUR);
       if ( !read_word(fd, &usType) )
          return GBM_ERR_READ;
    }

    if ( usType != BFT_BMAP )
        return GBM_ERR_BAD_MAGIC;

    bmp_priv->base = (dword) ( gbm_file_lseek(fd, 0L, GBM_SEEK_CUR) - 2L );

    if ( !read_dword(fd, &cbSize) )
        return GBM_ERR_READ;
    if ( !read_word(fd, &xHotspot) )
        return GBM_ERR_READ;
    if ( !read_word(fd, &yHotspot) )
        return GBM_ERR_READ;
    if ( !read_dword(fd, &offBits) )
        return GBM_ERR_READ;
    if ( !read_dword(fd, &cbFix) )
        return GBM_ERR_READ;

    bmp_priv->offBits = offBits;

    if ( cbFix == 12 )
    /* OS/2 1.x uncompressed bitmap */
    {
       word cx, cy, cPlanes, cBitCount;

       if ( !read_word(fd, &cx) )
          return GBM_ERR_READ;
       if ( !read_word(fd, &cy) )
          return GBM_ERR_READ;
       if ( !read_word(fd, &cPlanes) )
          return GBM_ERR_READ;
       if ( !read_word(fd, &cBitCount) )
          return GBM_ERR_READ;

       if ( cx == 0 || cy == 0 )
          return GBM_ERR_BAD_SIZE;
       if ( cPlanes != 1 )
          return GBM_ERR_BMP_PLANES;
       if ( cBitCount != 1 && cBitCount != 4 && cBitCount != 8 && cBitCount != 24 )
          return GBM_ERR_BMP_BITCOUNT;

       gbm->w   = (int) cx;
       gbm->h   = (int) cy;
       gbm->bpp = (int) cBitCount;

       bmp_priv->windows   = FALSE;
       bmp_priv->cBitCount = cBitCount;
       bmp_priv->cclrUsed  = ((dword)1 << cBitCount) & 0x1ff; /* 1->2, 4->16, 8->256, 16/24/32->0 */
       memset(&(bmp_priv->cMasks), 0, sizeof(bmp_priv->cMasks)); /* unused */
    }
    else if ( cbFix >= 16 && cbFix <= 64 &&
              ((cbFix & 3) == 0 || cbFix == 42 || cbFix == 46) )
    /*...sOS\47\2 2\46\0 and Windows 3\46\0:16:*/
    {
       word cPlanes, cBitCount, usUnits, usReserved, usRecording, usRendering;
       dword ulWidth, ulHeight, ulCompression;
       dword ulSizeImage, ulXPelsPerMeter, ulYPelsPerMeter;
       dword cclrUsed, cclrImportant, cSize1, cSize2, ulColorEncoding, ulIdentifier;
       BOOLEAN ok;

       ok  = read_dword(fd, &ulWidth);
       ok &= read_dword(fd, &ulHeight);
       ok &= read_word(fd, &cPlanes);
       ok &= read_word(fd, &cBitCount);
       if ( cbFix > 16 )
          ok &= read_dword(fd, &ulCompression);
       else
          ulCompression = BCA_UNCOMP;
       if ( cbFix > 20 )
          ok &= read_dword(fd, &ulSizeImage);
       if ( cbFix > 24 )
          ok &= read_dword(fd, &ulXPelsPerMeter);
       if ( cbFix > 28 )
          ok &= read_dword(fd, &ulYPelsPerMeter);
       if ( cbFix > 32 )
          ok &= read_dword(fd, &cclrUsed);
       else
          cclrUsed = ((dword)1 << cBitCount) & 0x1ff; /* 1->2, 4->16, 8->256, 16/24/32->0 */

       if (cclrUsed == 0)
       {
          cclrUsed = ((dword)1 << cBitCount) & 0x1ff; /* 1->2, 4->16, 8->256, 16/24/32->0 */
       }

       /* Protect against badly written bitmaps! */
       if ( cclrUsed > ( (dword)1 << cBitCount ) )
       {
          cclrUsed = ((dword)1 << cBitCount) & 0x1ff; /* 1->2, 4->16, 8->256, 16/24/32->0 */
       }

       if ( cbFix > 36 )
          ok &= read_dword(fd, &cclrImportant);
       if ( cbFix > 40 )
          ok &= read_word(fd, &usUnits);
       if ( cbFix > 42 )
          ok &= read_word(fd, &usReserved);
       if ( cbFix > 44 )
          ok &= read_word(fd, &usRecording);
       if ( cbFix > 46 )
          ok &= read_word(fd, &usRendering);
       if ( cbFix > 48 )
          ok &= read_dword(fd, &cSize1);
       if ( cbFix > 52 )
          ok &= read_dword(fd, &cSize2);
       if ( cbFix > 56 )
          ok &= read_dword(fd, &ulColorEncoding);
       if ( cbFix > 60 )
          ok &= read_dword(fd, &ulIdentifier);

       if ( !ok )
          return GBM_ERR_READ;

       if ( ulWidth == 0L || ulHeight == 0L )
          return GBM_ERR_BAD_SIZE;
       if ( cPlanes != 1 )
          return GBM_ERR_BMP_PLANES;

       if ( cBitCount != 1  &&
            cBitCount != 4  &&
            cBitCount != 8  &&
            cBitCount != 16 &&
            cBitCount != 24 &&
            cBitCount != 32 )
       {
         return GBM_ERR_BMP_BITCOUNT;
       }

       gbm->w   = (int) ulWidth;
       gbm->h   = (int) ulHeight;

       bmp_priv->windows       = TRUE;
       bmp_priv->cbFix         = cbFix;
       bmp_priv->ulCompression = ulCompression;
       bmp_priv->cclrUsed      = cclrUsed;
       bmp_priv->cBitCount     = cBitCount;

       /* map Windows specific formats to 24bpp (BI_RGB, BI_BITFIELDS) */
       if ((cBitCount == 16) || (cBitCount == 32))
       {
         gbm->bpp = 24;

         switch(ulCompression)
         {
           case BCA_UNCOMP:
             bmp_priv->cMasks.bitoffset_b = 0;
             bmp_priv->cMasks.bitoffset_g = (cBitCount == 16) ?  5 :  8;
             bmp_priv->cMasks.bitoffset_r = (cBitCount == 16) ? 10 : 16;

             /* set color masks to either 16bpp (5,5,5) or 32bpp (8,8,8) */
             bmp_priv->cMasks.mask_b = (cBitCount == 16) ? 0x001f : 0x000000ff;
             bmp_priv->cMasks.mask_g = (cBitCount == 16) ? 0x03e0 : 0x0000ff00;
             bmp_priv->cMasks.mask_r = (cBitCount == 16) ? 0x7c00 : 0x00ff0000;

             bmp_priv->cMasks.valid_bits_b = (cBitCount == 16) ? 5 : 8;
             bmp_priv->cMasks.valid_bits_g = (cBitCount == 16) ? 5 : 8;
             bmp_priv->cMasks.valid_bits_r = (cBitCount == 16) ? 5 : 8;
             break;

           case BI_BITFIELDS:
           {
             int bytes_b, bytes_g, bytes_r;

             /* Read BI_BITFIELDS color masks from the header (where usually the palette is) */
             /* These are strangely stored as dwords in the order of R-G-B. */
             gbm_file_lseek(fd, (long) (bmp_priv->base + 14L + bmp_priv->cbFix), GBM_SEEK_SET);
             bytes_r = gbm_file_read(fd, &(bmp_priv->cMasks.mask_r), sizeof(dword));
             bytes_g = gbm_file_read(fd, &(bmp_priv->cMasks.mask_g), sizeof(dword));
             bytes_b = gbm_file_read(fd, &(bmp_priv->cMasks.mask_b), sizeof(dword));
             if ((bytes_b != sizeof(dword)) || (bytes_g != sizeof(dword)) || (bytes_r != sizeof(dword)))
             {
               return GBM_ERR_BMP_CBFIX;
             }

             /* count the bits used in each mask */
             bmp_priv->cMasks.valid_bits_b = count_mask_bits(bmp_priv->cMasks.mask_b, &bmp_priv->cMasks.bitoffset_b);
             bmp_priv->cMasks.valid_bits_g = count_mask_bits(bmp_priv->cMasks.mask_g, &bmp_priv->cMasks.bitoffset_g);
             bmp_priv->cMasks.valid_bits_r = count_mask_bits(bmp_priv->cMasks.mask_r, &bmp_priv->cMasks.bitoffset_r);

             /* Only up to 8 bit per mask are allowed */
             if ((bmp_priv->cMasks.valid_bits_b > 8) ||
                 (bmp_priv->cMasks.valid_bits_g > 8) ||
                 (bmp_priv->cMasks.valid_bits_r > 8))
             {
               return GBM_ERR_BMP_COMP;
             }

             /* check for non-overlapping bits */
             if (bmp_priv->cMasks.valid_bits_b + bmp_priv->cMasks.valid_bits_g + bmp_priv->cMasks.valid_bits_b > cBitCount)
             {
               return GBM_ERR_BMP_COMP;
             }
             if ((bmp_priv->cMasks.bitoffset_b + bmp_priv->cMasks.valid_bits_b > bmp_priv->cMasks.bitoffset_g) ||
                 (bmp_priv->cMasks.bitoffset_g + bmp_priv->cMasks.valid_bits_g > bmp_priv->cMasks.bitoffset_r) ||
                 (bmp_priv->cMasks.bitoffset_r + bmp_priv->cMasks.valid_bits_r > cBitCount))
             {
               return GBM_ERR_BMP_COMP;
             }
           }
           break;

           default:
             return GBM_ERR_BMP_COMP;
         }
       }
       else
       {
         gbm->bpp = (int) cBitCount;
         memset(&(bmp_priv->cMasks), 0, sizeof(bmp_priv->cMasks)); /* unused */
       }
    }
    else
       return GBM_ERR_BMP_CBFIX;

    return GBM_ERR_OK;
}

/* ------------------------------------------------- */

GBM_ERR bmp_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
{
    BMP_PRIV *bmp_priv = (BMP_PRIV *) gbm->priv;

    if (gbm->bpp <= 8)
    {
        int i;
        byte b[4];
        const int palette_entries = ((int)1 << gbm->bpp) & 0x1ff;

        if ( bmp_priv->windows )
        {
           gbm_file_lseek(fd, (long) (bmp_priv->base + 14L + bmp_priv->cbFix), GBM_SEEK_SET);
           for ( i = 0; i < palette_entries; i++ )
           {
              if (gbm_file_read(fd, b, 4) != 4)
              {
                return GBM_ERR_BMP_CBFIX;
              }
              gbmrgb[i].b = b[0];
              gbmrgb[i].g = b[1];
              gbmrgb[i].r = b[2];
           }
        }
        else
        {
           gbm_file_lseek(fd, (long) (bmp_priv->base + 26L), GBM_SEEK_SET);
           for ( i = 0; i < palette_entries; i++ )
           {
              if (gbm_file_read(fd, b, 3) != 3)
              {
                return GBM_ERR_BMP_CBFIX;
              }
              gbmrgb[i].b = b[0];
              gbmrgb[i].g = b[1];
              gbmrgb[i].r = b[2];
           }
        }

        if ((gbm->bpp == 1) && (bmp_priv->inv))
        {
          swap_pal(gbmrgb);
        }
    }

    return GBM_ERR_OK;
}

/* ------------------------------------------------- */

GBM_ERR bmp_rdata(int fd, GBM *gbm, byte *data)
{
    BMP_PRIV *bmp_priv = (BMP_PRIV *) gbm->priv;
    const int cLinesWorth = ((gbm->bpp * gbm->w + 31) / 32) * 4;

    GBM_ERR rc = GBM_ERR_OK;

    if ( bmp_priv->windows )
    /* OS/2 2.0 and Windows 3.x, NT, 2000, XP */
    {
       gbm_file_lseek(fd, (long)bmp_priv->offBits, GBM_SEEK_SET);

       switch ( (int) bmp_priv->ulCompression )
       {
          /* BCA_UNCOMP (on OS/2, BI_RGB on Windows) */
          case BCA_UNCOMP:
          {
            switch(bmp_priv->cBitCount)
            {
              case 1:
              case 4:
              case 8:
              case 24:
                {
                  /* directly read as this is the same as the internal format */
                  const int bytesToRead = gbm->h * cLinesWorth;
                  if (gbm_file_read(fd, data, bytesToRead) != bytesToRead)
                  {
                    rc = GBM_ERR_READ;
                  }
                }
                break;

              case 16:
                rc = read_16bpp_data(fd, gbm, data, &(bmp_priv->cMasks));
                break;

              case 32:
                rc = read_32bpp_data(fd, gbm, data,  &(bmp_priv->cMasks));
                break;

              default:
                return GBM_ERR_BMP_COMP;
            }
          }
          break;

          case BI_BITFIELDS:
          {
            switch(bmp_priv->cBitCount)
            {
              case 16:
                rc = read_16bpp_data(fd, gbm, data, &(bmp_priv->cMasks));
                break;

              case 32:
                rc = read_32bpp_data(fd, gbm, data,  &(bmp_priv->cMasks));
                break;

              default:
                return GBM_ERR_BMP_COMP;
            }
          }
          break;

          /* ---------------------------------------------------- */

          /* BCA_RLE24 */
          case BCA_RLE24:
            rc = read_rle24_data(fd, gbm, data);
            break;

          /* ---------------------------------------------------- */

          /* BCA_RLE8 */
          case BCA_RLE8:
            rc = read_rle8_data(fd, gbm, data);
            break;

          /* ---------------------------------------------------- */

          /* BCA_RLE4 */
          case BCA_RLE4:
            rc = read_rle4_data(fd, gbm, data);
            break;

          /* ---------------------------------------------------- */

          default:
             return GBM_ERR_BMP_COMP;
       }
    }
    else
    /* OS/2 1.1, 1.2 */
    {
       const int bytesToRead = gbm->h * cLinesWorth;
       gbm_file_lseek(fd, (long) bmp_priv->offBits, GBM_SEEK_SET);
       if (gbm_file_read(fd, data, bytesToRead) != bytesToRead)
       {
         rc = GBM_ERR_READ;
       }
    }

    if (rc == GBM_ERR_OK)
    {
      if ( bmp_priv->invb )
      {
          invert(data, (unsigned) (cLinesWorth * gbm->h));
      }
    }

    return rc;
}

/* ------------------------------------------------- */

static int bright(const GBMRGB *gbmrgb)
{
   return gbmrgb->r*30+gbmrgb->g*60+gbmrgb->b*10;
}

/* ------------------------------------------------- */

static int write_inv(int fd, const byte *buffer, int count)
{
    byte small_buf[1024];
    int so_far = 0, this_go, written;

    while ( so_far < count )
    {
        this_go = min(count - so_far, 1024);
        memcpy(small_buf, buffer + so_far, (size_t) this_go);
        invert(small_buf, (unsigned) this_go);
        if ( (written = gbm_file_write(fd, small_buf, this_go)) != this_go )
            return so_far + written;
        so_far += written;
    }

    return so_far;
}

/* ------------------------------------------------- */

GBM_ERR bmp_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const byte *data, const char *opt)
{
    const BOOLEAN    pm11 = ( gbm_find_word(opt, "1.1"    ) != NULL );
    const BOOLEAN    win  = ( gbm_find_word(opt, "win"    ) != NULL ||
                              gbm_find_word(opt, "2.0"    ) != NULL );
    const BOOLEAN inv     = ( gbm_find_word(opt, "inv"    ) != NULL );
          BOOLEAN invb    = ( gbm_find_word(opt, "invb"   ) != NULL );
    const BOOLEAN darkfg  = ( gbm_find_word(opt, "darkfg" ) != NULL );
    const BOOLEAN lightfg = ( gbm_find_word(opt, "lightfg") != NULL );
    int cRGB;
    GBMRGB gbmrgb_1bpp[2];

    if ( pm11 && win )
        return GBM_ERR_BAD_OPTION;

    fn=fn; /* Suppress 'unref arg' compiler warning */

    cRGB = ( (1 << gbm->bpp) & 0x1ff ); /* 1->2, 4->16, 8->256, 24->0 */

    if ( cRGB == 2 )
    /*...shandle 1bpp case:16:*/
    {
       gbmrgb_1bpp[0] = gbmrgb[0];
       gbmrgb_1bpp[1] = gbmrgb[1];

       if (inv)
       {
         swap_pal(gbmrgb_1bpp);
       }

       /*
          If the user has picked the darkfg option, then they intend that the darkest
          colour in the image is to be the foreground. This is a very sensible option
          because the foreground will appear to be black when the image is reloaded.
          To acheive this we must invert the bitmap bits, if the palette dictates.
       */

       if ( darkfg && bright(&gbmrgb[0]) < bright(&gbmrgb[1]) )
          invb = !invb;
       if ( lightfg && bright(&gbmrgb[0]) >= bright(&gbmrgb[1]) )
          invb = !invb;

       gbmrgb = gbmrgb_1bpp;
    }
    /*...e*/

    if ( pm11 )
    /*...sOS\47\2 1\46\1:16:*/
    {
       const word usType     = BFT_BMAP;
       const word xHotspot   = 0;
       const word yHotspot   = 0;
       const dword cbFix     = (dword) 12;
       const word cx         = (word) gbm->w;
       const word cy         = (word) gbm->h;
       const word cPlanes    = (word) 1;
       const word cBitCount  = (word) gbm->bpp;
       const int cLinesWorth = (((cBitCount * cx + 31) / 32) * cPlanes) * 4;
       const dword offBits   = (dword) 26 + cRGB * (dword) 3;
       const dword cbSize    = offBits + (dword) cy * (dword) cLinesWorth;
       int i, total, actual;

       write_word(fd, usType);
       write_dword(fd, cbSize);
       write_word(fd, xHotspot);
       write_word(fd, yHotspot);
       write_dword(fd, offBits);
       write_dword(fd, cbFix);
       write_word(fd, cx);
       write_word(fd, cy);
       write_word(fd, cPlanes);
       write_word(fd, cBitCount);

       for ( i = 0; i < cRGB; i++ )
       {
          byte b[3];

          b[0] = gbmrgb[i].b;
          b[1] = gbmrgb[i].g;
          b[2] = gbmrgb[i].r;
          if ( gbm_file_write(fd, b, 3) != 3 )
             return GBM_ERR_WRITE;
       }

       total = gbm->h * cLinesWorth;
       if ( invb )
          actual = write_inv(fd, data, total);
       else
          actual = gbm_file_write(fd, data, total);
       if ( actual != total )
          return GBM_ERR_WRITE;
    }
    /*...e*/
    else
    /*...sOS\47\2 2\46\0 and Windows 3\46\0:16:*/
    {
       const word usType         = BFT_BMAP;
       const word xHotspot       = 0;
       const word yHotspot       = 0;
       const dword cbFix         = (dword) 40;
       const dword cx            = (dword) gbm->w;
       const dword cy            = (dword) gbm->h;
       const word cPlanes        = (word) 1;
       const word cBitCount      = (word) gbm->bpp;
       const int cLinesWorth     = (((cBitCount * (int) cx + 31) / 32) * cPlanes) * 4;
       const dword offBits       = (dword) 54 + cRGB * (dword) 4;
       const dword cbSize        = offBits + (dword) cy * (dword) cLinesWorth;
       const dword ulCompression = BCA_UNCOMP;
       const dword cbImage       = (dword) cLinesWorth * (dword) gbm->h;
       const dword cxResolution  = 0;
       const dword cyResolution  = 0;
       const dword cclrUsed      = 0;
       const dword cclrImportant = 0;
       int i, total, actual;

       write_word(fd, usType);
       write_dword(fd, cbSize);
       write_word(fd, xHotspot);
       write_word(fd, yHotspot);
       write_dword(fd, offBits);

       write_dword(fd, cbFix);
       write_dword(fd, cx);
       write_dword(fd, cy);
       write_word(fd, cPlanes);
       write_word(fd, cBitCount);
       write_dword(fd, ulCompression);
       write_dword(fd, cbImage);
       write_dword(fd, cxResolution);
       write_dword(fd, cyResolution);
       write_dword(fd, cclrUsed);
       write_dword(fd, cclrImportant);

       for ( i = 0; i < cRGB; i++ )
       {
          byte b[4];

          b[0] = gbmrgb[i].b;
          b[1] = gbmrgb[i].g;
          b[2] = gbmrgb[i].r;
          b[3] = 0;
          if ( gbm_file_write(fd, b, 4) != 4 )
             return GBM_ERR_WRITE;
       }

       total = gbm->h * cLinesWorth;
       if ( invb )
          actual = write_inv(fd, data, total);
       else
          actual = gbm_file_write(fd, data, total);
       if ( actual != total )
          return GBM_ERR_WRITE;
    }
    /*...e*/

    return GBM_ERR_OK;
}

/* ------------------------------------------------- */

const char *bmp_err(GBM_ERR rc)
{
    switch ( (int) rc )
    {
        case GBM_ERR_BMP_PLANES:
            return "number of bitmap planes is not 1";
        case GBM_ERR_BMP_BITCOUNT:
            return "bit count not 1, 4, 8, 16, 24 or 32";
        case GBM_ERR_BMP_CBFIX:
            return "cbFix bad";
        case GBM_ERR_BMP_COMP:
            return "compression type not uncompressed, RLE4, RLE8, RLE24 or BITFIELDS";
        case GBM_ERR_BMP_OFFSET:
            return "less bitmaps in file than index requested";
    }
    return NULL;
}

/* ------------------------------------------------- */
/* ------------------------------------------------- */

/* Read 16bpp data with compression BI_RGB or BI_BITFIELDS (will be mapped to 24bpp, lossless) */
static GBM_ERR read_16bpp_data(int fd, GBM *gbm, byte *data,
                               const BMP_COLOR_MASKS * maskdef)

{
   const int stride_src = ((gbm->w * 16 + 31) / 32) * 4;
   const int stride_dst = ((gbm->w * 24 + 31) / 32) * 4;
   byte * data_src   = NULL;
   byte * data_write = NULL;
   byte * data8_dst  = NULL;

   int h;

   const word * data16_src = NULL;
         word   data16;
         int    block_count;

   /* Only up to 8 bit per mask are allowed */
   if ((maskdef->valid_bits_b > 8) || (maskdef->valid_bits_g > 8) || (maskdef->valid_bits_r > 8))
   {
     return GBM_ERR_BMP_COMP;
   }

   data_src = (byte *) malloc((size_t) (stride_src));
   if (data_src == NULL)
   {
     return GBM_ERR_MEM;
   }

   data_write = data; /* write pointer */

   for (h = 0; h < gbm->h; h++)
   {
     data16_src  = (const word *) data_src;
     data8_dst   = data_write;
     block_count = gbm->w;

     if (gbm_file_read(fd, data_src, stride_src) != stride_src)
     {
       free(data_src);
       return GBM_ERR_READ;
     }

     /* Extract red, green, blue. */
     /* Encoding starts at least significant bit, then xB,yG,zR (most significant bit is unused). */
     /* Map these into 24bpp BGR. */

     /* (((dword) bgr16 & mask_color) >> bitoffset) << (8 - valid_bits) */
     #define GetB(bgr16, mask) ( ((((dword) bgr16) & mask->mask_b) >> mask->bitoffset_b) << (8 - mask->valid_bits_b) )
     #define GetG(bgr16, mask) ( ((((dword) bgr16) & mask->mask_g) >> mask->bitoffset_g) << (8 - mask->valid_bits_g) )
     #define GetR(bgr16, mask) ( ((((dword) bgr16) & mask->mask_r) >> mask->bitoffset_r) << (8 - mask->valid_bits_r) )

     while (block_count > 0)
     {
        --block_count;

        data16 = *data16_src++;

        *data8_dst++ = GetB(data16, maskdef);
        *data8_dst++ = GetG(data16, maskdef);
        *data8_dst++ = GetR(data16, maskdef);
     }
     #undef GetB
     #undef GetG
     #undef GetR

     data_write += stride_dst;
  }

  free(data_src);

  return GBM_ERR_OK;
}

/* ------------------------------------------------- */

/* Read 32bpp data with compression BI_RGB or BI_BITFIELDS (will be mapped to 24bpp, lossless) */
static GBM_ERR read_32bpp_data(int fd, GBM *gbm, byte *data,
                               const BMP_COLOR_MASKS * maskdef)
{
   /* almost identical to the internal format (last byte must be ignored)) */
   const int stride_src = gbm->w * 4;
   const int stride_dst = ((gbm->w * 24 + 31) / 32) * 4;
   byte * data_src   = NULL;
   byte * data_write = NULL;
   byte * data8_dst  = NULL;

   int h;

   const dword * data32_src = NULL;
         dword   data32;
         int     block_count;

   /* Only up to 8 bit per mask are allowed */
   if ((maskdef->valid_bits_b > 8) || (maskdef->valid_bits_g > 8) || (maskdef->valid_bits_r > 8))
   {
     return GBM_ERR_BMP_COMP;
   }

   data_src = (byte *) malloc((size_t) (stride_src));
   if (data_src == NULL)
   {
     return GBM_ERR_MEM;
   }

   data_write = data; /* write pointer */

   for (h = 0; h < gbm->h; h++)
   {
     data32_src  = (const dword *) data_src;
     data8_dst   = data_write;
     block_count = gbm->w;

     if (gbm_file_read(fd, data_src, stride_src) != stride_src)
     {
       free(data_src);
       return GBM_ERR_READ;
     }

     /* Extract red, green, blue. */
     /* Encoding starts at least significant bit, then xB,yG,zR (most significant bit is unused). */
     /* Map these into 24bpp BGR. */

     /* (((dword) bgr32 & mask_color) >> bitoffset) << (8 - valid_bits) */
     #define GetB(bgr32, mask) ( ((((dword) bgr32) & mask->mask_b) >> mask->bitoffset_b) << (8 - mask->valid_bits_b) )
     #define GetG(bgr32, mask) ( ((((dword) bgr32) & mask->mask_g) >> mask->bitoffset_g) << (8 - mask->valid_bits_g) )
     #define GetR(bgr32, mask) ( ((((dword) bgr32) & mask->mask_r) >> mask->bitoffset_r) << (8 - mask->valid_bits_r) )

     while (block_count > 0)
     {
        --block_count;

        data32 = *data32_src++;

        *data8_dst++ = GetB(data32, maskdef);
        *data8_dst++ = GetG(data32, maskdef);
        *data8_dst++ = GetR(data32, maskdef);
     }
     #undef GetB
     #undef GetG
     #undef GetR

     data_write += stride_dst;
  }

  free(data_src);

  return GBM_ERR_OK;
}

/* ------------------------------------------------- */
/* ------------------------------------------------- */

/* Read RLE24 encoded data */
static GBM_ERR read_rle24_data(int fd, GBM *gbm, byte *data)
{
   const int cLinesWorth = ((gbm->bpp * gbm->w + 31) / 32) * 4;
   const byte * dataEnd  = data + (cLinesWorth * gbm->h);

   AHEAD *ahead;
   int x = 0, y = 0;
   BOOLEAN eof24 = FALSE;

   if ( (ahead = gbm_create_ahead(fd)) == NULL )
      return GBM_ERR_MEM;

   while ( !eof24 )
   {
            byte c = (byte) gbm_read_ahead(ahead);
      const byte d = (byte) gbm_read_ahead(ahead);
      /* fprintf(stderr, "(%d,%d) c=%d,d=%d\n", x, y, c, d); */

      if ( c )
      {
         /* encoded run */
         byte  i;
         const byte g = (byte) gbm_read_ahead(ahead);
         const byte r = (byte) gbm_read_ahead(ahead);

         if (data + (c * 3) > dataEnd)
         {
             c = (dataEnd - data > 0) ? 0 : ((dataEnd - data) / 3);
         }

         for (i = 0; i < c; i++)
         {
            *data++ = d; /* b */
            *data++ = g;
            *data++ = r;
            x += 3;
         }
      }
      else
      {
         /* unencoded run */
         switch ( d )
         {
            /* MSWCC_EOL */
            case MSWCC_EOL:
            {
               const int to_eol = cLinesWorth - x;
               if (x > 0)
               {
                  memset(data, 0, (size_t) to_eol);
                  data += to_eol;
               }
               x = 0;
               if ( ++y == gbm->h )
               {
                  eof24 = TRUE;
               }
            }
            break;

            /* MSWCC_EOB */
            case MSWCC_EOB:
               if ( y < gbm->h )
               {
                  const int to_eol = cLinesWorth - x;

                  if (x > 0)
                  {
                     memset(data, 0, (size_t) to_eol);
                     data += to_eol;
                  }
                  x = 0; y++;
                  while ( y < gbm->h )
                  {
                     if (data + cLinesWorth > dataEnd)
                     {
                        const int len = (dataEnd - data > 0) ? 0 : dataEnd - data;
                        memset(data, 0, (size_t) len);
                        data += len;
                     }
                     else
                     {
                        memset(data, 0, (size_t) cLinesWorth);
                        data += cLinesWorth;
                     }
                     y++;
                  }
               }
               eof24 = TRUE;
               break;

            /* MSWCC_DELTA */
            case MSWCC_DELTA:
            {
               const byte dx  = (byte) gbm_read_ahead(ahead);
               const byte dy  = (byte) gbm_read_ahead(ahead);
               int fill = (dx * 3) + (dy * cLinesWorth);

               if (data + fill > dataEnd)
               {
                  fill = (dataEnd - data > 0) ? 0 : dataEnd - data;
               }
               memset(data, 0, (size_t) fill);
               data += fill;
               x += dx * 3; y += dy;
               if ( y == gbm->h )
               {
                  eof24 = TRUE;
               }
            }
            break;

            /* default */
            default:
            {
               int n = (int) d;
               int diff = 0;

               if (data + (n * 3) > dataEnd)
               {
                  n    = (dataEnd - data > 0) ? 0 : ((dataEnd - data) / 3);
                  diff = d - n;
               }
               while ( n-- > 0 )
               {
                  *data++ = (byte) gbm_read_ahead(ahead);
                  *data++ = (byte) gbm_read_ahead(ahead);
                  *data++ = (byte) gbm_read_ahead(ahead);
                  x += 3;
               }
               data += 3 * diff;

               if ( x > (gbm->w * 3) ) { x = 0; ++y; }
               if ( d & 1 )
               {
                  gbm_read_ahead(ahead); /* Align */
               }
            }
            break;
         }
      }
   }

   gbm_destroy_ahead(ahead);
   return GBM_ERR_OK;
}

/* ------------------------------------------------- */

/* Read RLE8 encoded data */
static GBM_ERR read_rle8_data(int fd, GBM *gbm, byte *data)
{
   const int cLinesWorth  = ((gbm->bpp * gbm->w + 31) / 32) * 4;
   const byte * dataEnd   = data + (cLinesWorth * gbm->h);

   AHEAD *ahead;
   int x = 0, y = 0;
   BOOLEAN eof8 = FALSE;

   if ( (ahead = gbm_create_ahead(fd)) == NULL )
      return GBM_ERR_MEM;

   while ( !eof8 )
   {
            byte c = (byte) gbm_read_ahead(ahead);
      const byte d = (byte) gbm_read_ahead(ahead);
/*      fprintf(stderr, "(%d,%d) c=%d,d=%d\n", x, y, c, d); */

      if ( c )
      {
         if (data + c > dataEnd)
         {
             c = (dataEnd - data > 0) ? 0 : dataEnd - data;
         }
         memset(data, d, c);
         x    += c;
         data += c;
      }
      else
         switch ( d )
         {
            /* MSWCC_EOL */
            case MSWCC_EOL:
            {
               const int to_eol = cLinesWorth - x;

               if (x > 0)
               {
                  memset(data, 0, (size_t) to_eol);
                  data += to_eol;
               }
               x = 0;
               if ( ++y == gbm->h )
               {
                  eof8 = TRUE;
               }
            }
            break;

            /* MSWCC_EOB */
            case MSWCC_EOB:
               if ( y < gbm->h )
               {
                  const int to_eol = cLinesWorth - x;

                  if (x > 0)
                  {
                     memset(data, 0, (size_t) to_eol);
                     data += to_eol;
                  }
                  x = 0; y++;
                  while (y < gbm->h)
                  {
                     if (data + cLinesWorth > dataEnd)
                     {
                        const int len = (dataEnd - data > 0) ? 0 : dataEnd - data;
                        memset(data, 0, (size_t) len);
                        data += len;
                     }
                     else
                     {
                        memset(data, 0, (size_t) cLinesWorth);
                        data += cLinesWorth;
                     }
                     y++;
                  }
               }
               eof8 = TRUE;
               break;

            /* MSWCC_DELTA */
            case MSWCC_DELTA:
            {
               const byte dx  = (byte) gbm_read_ahead(ahead);
               const byte dy  = (byte) gbm_read_ahead(ahead);
               int fill = dx + dy * cLinesWorth;

               if (data + fill > dataEnd)
               {
                  fill = (dataEnd - data > 0) ? 0 : dataEnd - data;
               }
               memset(data, 0, (size_t) fill);
               data += fill;
               x += dx; y += dy;
               if ( y == gbm->h )
               {
                  eof8 = TRUE;
               }
            }
            break;

            /* default */
            default:
            {
               int n = (int) d;
               int diff = 0;

               if (data + n > dataEnd)
               {
                  n    = (dataEnd - data > 0) ? 0 : dataEnd - data;
                  diff = d - n;
               }

               while ( n-- > 0 )
               {
                  *data++ = (byte) gbm_read_ahead(ahead);
               }
               data += diff;
               x    += d;

               if ( x > gbm->w ) { x = 0; ++y; } /* @@@AK */
               if ( d & 1 )
               {
                  gbm_read_ahead(ahead); /* Align */
               }
            }
            break;
         }
   }

   gbm_destroy_ahead(ahead);
   return GBM_ERR_OK;
}

/* ------------------------------------------------- */

/* Read RLE4 encoded data */
static GBM_ERR read_rle4_data(int fd, GBM *gbm, byte *data)
{
   const int cLinesWorth = ((gbm->bpp * gbm->w + 31) / 32) * 4;
   const byte * dataEnd  = data + (cLinesWorth * gbm->h);

   AHEAD *ahead;
   int x = 0, y = 0;
   BOOLEAN eof4 = FALSE;
   int inx = 0;

   if ( (ahead = gbm_create_ahead(fd)) == NULL )
      return GBM_ERR_MEM;

   memset(data, 0, (size_t) (gbm->h * cLinesWorth));

   while ( !eof4 )
   {
      byte c = (byte) gbm_read_ahead(ahead);
      byte d = (byte) gbm_read_ahead(ahead);

      if ( c )
      {
         byte h, l;
         int i;
         if ( x & 1 )
            { h = (byte) (d >> 4); l = (byte) (d << 4); }
         else
            { h = (byte) (d&0xf0); l = (byte) (d&0x0f); }

         if (data + (c/2) > dataEnd)
         {
             c = (dataEnd - data > 0) ? 0 : ((dataEnd - data) * 2);
         }

         for ( i = 0; i < (int) c; i++, x++ )
         {
            if ( x & 1U )
               data[inx++] |= l;
            else
               data[inx]   |= h;
         }
      }
      else
         switch ( d )
         {
            /* MSWCC_EOL */
            case MSWCC_EOL:
               x = 0;
               if ( ++y == gbm->h )
               {
                  eof4 = TRUE;
               }
               inx = cLinesWorth * y;
               break;

            /* MSWCC_EOB */
            case MSWCC_EOB:
               eof4 = TRUE;
               break;

            /* MSWCC_DELTA */
            case MSWCC_DELTA:
            {
               const byte dx = (byte) gbm_read_ahead(ahead);
               const byte dy = (byte) gbm_read_ahead(ahead);

               x += dx; y += dy;
               inx = y * cLinesWorth + (x/2);

               if ( y == gbm->h )
               {
                  eof4 = TRUE;
               }
            }
            break;

            /* default */
            default:
            {
               int i, nr = 0;
               int diff = 0;

               if (data + (d/2) > dataEnd)
               {
                  const int d_org = d;
                  d    = (dataEnd - data > 0) ? 0 : ((dataEnd - data) * 2);
                  diff = d_org - d;
               }

               if ( x & 1 )
               {
                  for ( i = 0; i+2 <= (int) d; i += 2 )
                  {
                     const byte b = (byte) gbm_read_ahead(ahead);
                     data[inx++] |= (b >> 4);
                     data[inx  ] |= (b << 4);
                     nr++;
                  }
                  if ( i < (int) d )
                  {
                     data[inx++] |= ((byte) gbm_read_ahead(ahead) >> 4);
                     nr++;
                  }
               }
               else
               {
                  for ( i = 0; i+2 <= (int) d; i += 2 )
                  {
                     data[inx++] = (byte) gbm_read_ahead(ahead);
                     nr++;
                  }
                  if ( i < (int) d )
                  {
                     data[inx] = (byte) gbm_read_ahead(ahead);
                     nr++;
                  }
               }
               inx += diff/2;
               x   += d;

               if ( nr & 1 )
               {
                  gbm_read_ahead(ahead); /* Align input stream to next word */
               }
            }
            break;
         }
   }

   gbm_destroy_ahead(ahead);
   return GBM_ERR_OK;
}


