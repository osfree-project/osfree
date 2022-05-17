/*

gbmxpm.c - X Windows PixMap (XPM) format

Credit for writing this module must go to Heiko Nitzsche.
This file is just as public domain as the rest of GBM.

Supported formats and options:
------------------------------
XPixMap : X Windows PixMap : .XPM

Supports XPM2 and XPM3 color visual subformats (no XPM1 or XPM2C).
Extensions are not supported.

Standard formats (backward compatible):
  Reads RGB files with hex or X11 symbolic coded colours (4,8,12,16 bit per R,G or B).
  Depending on the number of colours the bitmap is presented as either 1,4,8 or 24 bpp.
  Transparency is supported.

Extended formats (not backward compatible, import option ext_bpp required):
  Reads RGB files with hex or X11 symbolic coded colours (4,8,12,16 bit per R,G or B).
  Depending on the number of colours the bitmap is presented as either 1,4,8,24,32,48 or 64 bpp.
  Transparency is supported.

Writes RGB files up to 1,4,8 and 24 bpp.

  Input:
  ------

  Can specify background color as RGB when alpha channel is provided
  (replacement color for transparency)
    Input option: back_rgb=#_#_# (red_green_blue, 0..65535)

  Can specify that non-standard GBM color depths are exported (also with alpha channel)
    Input option: ext_bpp (default: bpp is downsampled to 24 bpp)


  Output:
  -------

  Write XPM2 format (default is XPM3)
    Output option: xpm2

  Can specify transparent colour index (only for palette bitmaps).
    Output option: transcol=# (default is none).

  Can specify transparency colour (only for bitmap without alpha channel)
    Output option: transcol_rgb=#_#_# (red_green_blue, 0..255)


History:
--------
(Heiko Nitzsche)

28-Nov-2006: Initial version
15-Aug-2008: Integrate new GBM types

*/

#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gbm.h"
#include "gbmhelp.h"
#include "gbmdesc.h"
#include "gbmmap.h"
#include "gbmmem.h"
#include "gbmxpmcn.h"

/* ---------------------------------------- */

#define GBM_ERR_XPM_BPP       ((GBM_ERR) 8000)
#define GBM_ERR_TRANSCOL_IDX  ((GBM_ERR) 8001)

/* ---------------------------------------- */

#define MIN(a,b)   ( ((a)<(b)) ? (a) : (b) )
#define MAX(a,b)   ( ((a)>(b)) ? (a) : (b) )

#define MakeBGR(b,g,r)  (((gbm_u32) r) | (((gbm_u32) (g)) << 8) | (((gbm_u32) (b)) << 16))

/* ---------------------------------------- */

static GBMFT xpm_gbmft =
{
   GBM_FMT_DESC_SHORT_XPM,
   GBM_FMT_DESC_LONG_XPM,
   GBM_FMT_DESC_EXT_XPM,
   GBM_FT_R1 | GBM_FT_R4 | GBM_FT_R8 | GBM_FT_R24 | GBM_FT_R32 | GBM_FT_R48 | GBM_FT_R64 |
   GBM_FT_W1 | GBM_FT_W4 | GBM_FT_W8 | GBM_FT_W24
};


typedef enum { XPM_FMT_UNKNOWN = 0, XPM_FMT_XPM1, XPM_FMT_XPM2, XPM_FMT_XPM2C, XPM_FMT_XPM3 } XPM_FMT_TYPE;
typedef enum { XPM_PAL_UNUSED  = 0, XPM_PAL_RGB , XPM_PAL_NAME, XPM_PAL_TRANSPARENT         } XPM_PAL_TYPE;

#pragma pack(2)
typedef struct XPM_HASH_ENTRY_
{
   struct XPM_HASH_ENTRY_ *next;
   XPM_PAL_TYPE            type;
   int                     index;
   GBMRGB_16BPP            rgb16;
   gbm_u8 *                code;
} XPM_HASH_ENTRY;
#pragma pack()

typedef struct
{
   gbm_u32           length;
   XPM_HASH_ENTRY ** entries;
   gbm_u32           keyupshift;
   gbm_u8          * pool;
   gbm_u32           pool_high_mark;
   int               code_len;
} XPM_CODE_HASH;

typedef struct
{
   gbm_u32           length;
   XPM_HASH_ENTRY ** entries;
   gbm_u8          * pool;
   gbm_u32           pool_high_mark;
   int               code_len;
} XPM_RGB_HASH;

typedef struct
{
   int chars_per_pixel;
   int valid_colors;

   gbm_boolean has_deep_color;
   gbm_boolean has_transparency;

   GBMRGB_16BPP backrgb;   /* background RGB color for Alpha channel mixing */

   /* This entry will store the options provided during first header read.
    * It will keep the options for the case the header has to be reread.
    */
   char read_options[PRIV_SIZE - (2 * sizeof(int))
                               - (2 * sizeof(gbm_boolean))
                               - sizeof(GBMRGB_16BPP)
                               - 8 /* space for structure element padding */ ];

} XPM_PRIV_READ;

/* ---------------------------------------- */

static GBM_ERR internal_xpm_rpal_16bpp(AHEAD *ahead, GBM * gbm, XPM_FMT_TYPE fmt_type, XPM_CODE_HASH * xpm_code_hash);

static GBM_ERR read_data_1bpp (AHEAD * ahead, XPM_FMT_TYPE type, const GBM *gbm, gbm_u8 *data, const XPM_CODE_HASH * xpm_code_hash);
static GBM_ERR read_data_4bpp (AHEAD * ahead, XPM_FMT_TYPE type, const GBM *gbm, gbm_u8 *data, const XPM_CODE_HASH * xpm_code_hash);
static GBM_ERR read_data_8bpp (AHEAD * ahead, XPM_FMT_TYPE type, const GBM *gbm, gbm_u8 *data, const XPM_CODE_HASH * xpm_code_hash);
static GBM_ERR read_data_24bpp(AHEAD * ahead, XPM_FMT_TYPE type, const GBM *gbm, gbm_u8 *data, const XPM_CODE_HASH * xpm_code_hash);
static GBM_ERR read_data_32bpp(AHEAD * ahead, XPM_FMT_TYPE type, const GBM *gbm, gbm_u8 *data, const XPM_CODE_HASH * xpm_code_hash);
static GBM_ERR read_data_48bpp(AHEAD * ahead, XPM_FMT_TYPE type, const GBM *gbm, gbm_u8 *data, const XPM_CODE_HASH * xpm_code_hash);
static GBM_ERR read_data_64bpp(AHEAD * ahead, XPM_FMT_TYPE type, const GBM *gbm, gbm_u8 *data, const XPM_CODE_HASH * xpm_code_hash);

/* ---------------------------------------- */
/* ---------------------------------------- */

static void Base92(unsigned int num, gbm_u8 * code, size_t code_len)
{
    static const gbm_u8 digit[] = " .XoO+@#$%&*=-;:>,<1234567890qwertyuipasdfghjklzxcvbnmMNBVCZASDFGHJKLPIUYTREWQ!~^/()_`'][{}|";
    size_t i = 0;

    assert(code_len > 0);
    memset(code, ' ', code_len);
    do
    {
        code[i++] = digit[num % 92];
        num /= 92;
    }
    while ((i < code_len) && num);
}

/* ---------------------------------------- */
/* ---------------------------------------- */

static XPM_CODE_HASH * create_xpm_code_hash(gbm_u32 num_entries, int code_len)
{
   if (num_entries > 0)
   {
      XPM_CODE_HASH * xpm_code_hash = (XPM_CODE_HASH *) calloc(1, sizeof(XPM_CODE_HASH));
      if (xpm_code_hash == NULL)
      {
          return NULL;
      }
      xpm_code_hash->entries = (XPM_HASH_ENTRY **) gbmmem_calloc(num_entries, sizeof(XPM_HASH_ENTRY *));
      if (xpm_code_hash->entries == NULL)
      {
          free(xpm_code_hash);
          return NULL;
      }
      xpm_code_hash->length = num_entries;

      /* optimize a bit for better balancing */
      xpm_code_hash->keyupshift = (num_entries > 300000) ? 7 : ((num_entries > 300) ? 5 : 2);

      /* Allocate a single memory block that will be the pool for placing
       * hash element pointers. This mechanism greatly reduces the overhead
       * compared to allocating/freeing each individual entry.
       * As we know that we will need all entries the hashtable is requested for,
       * there is no additional memory overhead. It will even shrink the memory
       * requirements as usuallay the C runtime would track each allocated block
       * by adding several bytes on top of each entry.
       */
      xpm_code_hash->pool = (gbm_u8 *) gbmmem_calloc(num_entries, sizeof(XPM_HASH_ENTRY) + code_len + 1);
      if (xpm_code_hash->pool == NULL)
      {
          gbmmem_free(xpm_code_hash->entries);
          free(xpm_code_hash);
          return NULL;
      }
      /* linkup the code buffers to the pool entries */
      {
          gbm_u32          i;
          XPM_HASH_ENTRY * pEntry;

          for (i = 0; i < num_entries; i++)
          {
              pEntry       = (XPM_HASH_ENTRY *)(xpm_code_hash->pool + (i * (sizeof(XPM_HASH_ENTRY) + code_len + 1)));
              pEntry->code = ((gbm_u8 *)pEntry) + sizeof(XPM_HASH_ENTRY);
          }
      }
      xpm_code_hash->pool_high_mark = 0;
      xpm_code_hash->code_len       = code_len;
      return xpm_code_hash;
   }
   return NULL;
}

/* ---------------------------------------- */

static XPM_RGB_HASH * create_xpm_rgb_hash(gbm_u32 num_entries, int code_len)
{
   if (num_entries > 0)
   {
      XPM_RGB_HASH * xpm_rgb_hash = (XPM_RGB_HASH *) calloc(1, sizeof(XPM_RGB_HASH));
      if (xpm_rgb_hash == NULL)
      {
          return NULL;
      }
      xpm_rgb_hash->entries = (XPM_HASH_ENTRY **) gbmmem_calloc(num_entries, sizeof(XPM_HASH_ENTRY *));
      if (xpm_rgb_hash->entries == NULL)
      {
          free(xpm_rgb_hash);
          return NULL;
      }
      xpm_rgb_hash->length = num_entries;

      /* Allocate a single memory block that will be the pool for placing
       * hash element pointers. This mechanism greatly reduces the overhead
       * compared to allocating/freeing each individual entry.
       * As we know that we will need all entries the hashtable is requested for,
       * there is no additional memory overhead. It will even shrink the memory
       * requirements as usuallay the C runtime would track each allocated block
       * by adding several bytes on top of each entry.
       */
      xpm_rgb_hash->pool = (gbm_u8 *) gbmmem_calloc(num_entries, sizeof(XPM_HASH_ENTRY) + code_len + 1);
      if (xpm_rgb_hash->pool == NULL)
      {
          gbmmem_free(xpm_rgb_hash->entries);
          free(xpm_rgb_hash);
          return NULL;
      }
      /* linkup the code buffers to the pool entries */
      {
          gbm_u32          i;
          XPM_HASH_ENTRY * pEntry;

          for (i = 0; i < num_entries; i++)
          {
              pEntry       = (XPM_HASH_ENTRY *)(xpm_rgb_hash->pool + (i * (sizeof(XPM_HASH_ENTRY) + code_len + 1)));
              pEntry->code = ((gbm_u8 *)pEntry) + sizeof(XPM_HASH_ENTRY);
          }
      }
      xpm_rgb_hash->pool_high_mark = 0;
      xpm_rgb_hash->code_len       = code_len;

      return xpm_rgb_hash;
   }
   return NULL;
}

/* ---------------------------------------- */

static void free_xpm_code_hash(XPM_CODE_HASH * xpm_code_hash)
{
    if (xpm_code_hash != NULL)
    {
        /* All entries are just pointers into the common memory area.
         * Thus we just need to free up the whole block :) .
         */
        gbmmem_free(xpm_code_hash->pool);
        gbmmem_free(xpm_code_hash->entries);
        xpm_code_hash->pool           = NULL;
        xpm_code_hash->entries        = NULL;
        xpm_code_hash->length         = 0;
        xpm_code_hash->pool_high_mark = 0;
        free(xpm_code_hash);
    }
}

/* ---------------------------------------- */

static void free_xpm_rgb_hash(XPM_RGB_HASH * xpm_rgb_hash)
{
    if (xpm_rgb_hash != NULL)
    {
        /* All entries are just pointers into the common memory area.
         * Thus we just need to free up the whole block :) .
         */
        gbmmem_free(xpm_rgb_hash->pool);
        gbmmem_free(xpm_rgb_hash->entries);
        xpm_rgb_hash->pool           = NULL;
        xpm_rgb_hash->entries        = NULL;
        xpm_rgb_hash->length         = 0;
        xpm_rgb_hash->pool_high_mark = 0;
        free(xpm_rgb_hash);
    }
}

/* ---------------------------------------- */

static int calc_xpm_code_hash_index(const XPM_CODE_HASH * xpm_code_hash, const gbm_u8 * code, int code_len)
{
    assert(code_len <= sizeof(gbm_u32));
    if (xpm_code_hash->length > 0)
    {
        int   i;
        gbm_u32 key = 0;
        const gbm_u32 tlen       = xpm_code_hash->length;
        const gbm_u32 keyupshift = xpm_code_hash->keyupshift;

        for (i = 0; i < code_len; i++)
        {
            key = ((key << keyupshift) + code[i]) % tlen;
        }
        return key % tlen;
    }
    return -1;
}

/* ---------------------------------------- */

static int calc_xpm_rgb_hash_index(const XPM_RGB_HASH * xpm_rgb_hash, gbm_u32 r, gbm_u32 g, gbm_u32 b)
{
    if (xpm_rgb_hash->length > 0)
    {
        /* Division method */
        const gbm_u32 key = MakeBGR(b,g,r);
        return key % (xpm_rgb_hash->length);
    }
    return -1;
}

/* ---------------------------------------- */

static gbm_boolean add_xpm_code_hash(XPM_CODE_HASH      * xpm_code_hash,
                                     const gbm_u8       * code,
                                     const short          code_len,
                                     XPM_PAL_TYPE         type,
                                     const GBMRGB_16BPP * rgb16,
                                     int                  index)
{
   int               hash_index;
   XPM_HASH_ENTRY ** ppEntry = NULL;
   XPM_HASH_ENTRY  * pEntry  = NULL;

   if ((rgb16 == NULL) || (code == NULL))
   {
       return GBM_FALSE;
   }
   hash_index = calc_xpm_code_hash_index(xpm_code_hash, code, code_len);
   if (hash_index < 0)
   {
       return GBM_FALSE;
   }
   if (code_len > xpm_code_hash->code_len)
   {
       return GBM_FALSE;
   }
   /* check if there were too many requests (the pool is excausted) */
   if (xpm_code_hash->pool_high_mark >= xpm_code_hash->length)
   {
       /* too many requests */
       return GBM_FALSE;
   }
   ppEntry = &(xpm_code_hash->entries[hash_index]);
   if (*ppEntry == NULL)
   {
       /* link to the next free XPM_HASH_ENTRY block in our pool */
       *ppEntry = (XPM_HASH_ENTRY *)
                    (xpm_code_hash->pool + (xpm_code_hash->pool_high_mark * (sizeof(XPM_HASH_ENTRY) + code_len + 1)));
       pEntry   = *ppEntry;
   }
   else
   {
       /* an entry list already exists -> add another element */
       XPM_HASH_ENTRY * pNextEntry;

       pEntry     = *ppEntry;
       pNextEntry = pEntry->next;

       while (pNextEntry != NULL)
       {
           pEntry     = pEntry->next;
           pNextEntry = pEntry->next;
       }
       /* link to the next free XPM_HASH_ENTRY block in our pool */
       pNextEntry = (XPM_HASH_ENTRY *)
                      (xpm_code_hash->pool + (xpm_code_hash->pool_high_mark * (sizeof(XPM_HASH_ENTRY) + code_len + 1)));

       pEntry->next = pNextEntry;
       pEntry       = pEntry->next;
   }
   /* map the code into the gbm_u32 */
   pEntry->next     = NULL;
   pEntry->index    = index;
   pEntry->type     = type;
   pEntry->rgb16    = *rgb16;
   strncpy((char *)(pEntry->code), (const char *)code, code_len);

   (xpm_code_hash->pool_high_mark)++;

   return GBM_TRUE;
}

/* ---------------------------------------- */

static gbm_boolean add_xpm_rgb_hash(XPM_RGB_HASH       * xpm_rgb_hash,
                                    const gbm_u8       * code,
                                    const short          code_len,
                                    XPM_PAL_TYPE         type,
                                    const GBMRGB_16BPP * rgb16,
                                    int                  index)
{
   int               hash_index;
   XPM_HASH_ENTRY ** ppEntry = NULL;
   XPM_HASH_ENTRY  * pEntry  = NULL;

   if ((rgb16 == NULL) || (code == NULL))
   {
       return GBM_FALSE;
   }
   hash_index = calc_xpm_rgb_hash_index(xpm_rgb_hash, rgb16->r, rgb16->g, rgb16->b);
   if (hash_index < 0)
   {
       return GBM_FALSE;
   }
   if (code_len > xpm_rgb_hash->code_len)
   {
       return GBM_FALSE;
   }
   /* check if there were too many requests (the pool is excausted) */
   if (xpm_rgb_hash->pool_high_mark >= xpm_rgb_hash->length)
   {
       /* too many requests */
       return GBM_FALSE;
   }
   ppEntry = &(xpm_rgb_hash->entries[hash_index]);
   if (*ppEntry == NULL)
   {
       /* link to the next free XPM_HASH_ENTRY block in our pool */
       *ppEntry = (XPM_HASH_ENTRY *)
                    (xpm_rgb_hash->pool + (xpm_rgb_hash->pool_high_mark * (sizeof(XPM_HASH_ENTRY) + code_len + 1)));
       pEntry   = *ppEntry;
   }
   else
   {
       /* an entry list already exists -> add another element */
       XPM_HASH_ENTRY * pNextEntry = NULL;

       pEntry     = *ppEntry;
       pNextEntry = pEntry->next;

       while (pNextEntry != NULL)
       {
           pEntry     = pEntry->next;
           pNextEntry = pEntry->next;
       }
       /* link to the next free XPM_HASH_ENTRY block in our pool */
       pNextEntry = (XPM_HASH_ENTRY *)
                      (xpm_rgb_hash->pool + (xpm_rgb_hash->pool_high_mark * (sizeof(XPM_HASH_ENTRY) + code_len + 1)));

       pEntry->next = pNextEntry;
       pEntry       = pEntry->next;
   }
   /* map the code into the gbm_u32 */
   pEntry->next     = NULL;
   pEntry->index    = index;
   pEntry->type     = type;
   pEntry->rgb16    = *rgb16;
   strncpy((char *)(pEntry->code), (const char *)code, code_len);

   (xpm_rgb_hash->pool_high_mark)++;

   return GBM_TRUE;
}

/* ---------------------------------------- */

static const XPM_HASH_ENTRY * find_xpm_code_hash_transparent_entry(const XPM_CODE_HASH * xpm_code_hash)
{
    gbm_u32 i;
    const gbm_u32 hashtable_len = xpm_code_hash->length;
    for (i = 0; i < hashtable_len; i++)
    {
        const XPM_HASH_ENTRY * pEntry = xpm_code_hash->entries[i];
        while (pEntry != NULL)
        {
            if (pEntry->type == XPM_PAL_TRANSPARENT)
            {
                return pEntry;
            }
            pEntry = pEntry->next;
        }
    }
    return NULL;
}

/* ---------------------------------------- */

static const XPM_HASH_ENTRY * find_xpm_rgb_hash_color_code(const XPM_RGB_HASH * xpm_rgb_hash, gbm_u32 r, gbm_u32 g, gbm_u32 b)
{
    const int hash_index = calc_xpm_rgb_hash_index(xpm_rgb_hash, r, g, b);
    if (hash_index < 0)
    {
        return GBM_FALSE;
    }
    {
        const XPM_HASH_ENTRY * pEntry = xpm_rgb_hash->entries[hash_index];
        while (pEntry != NULL)
        {
            const GBMRGB_16BPP * rgb16 = &(pEntry->rgb16);
            if ((rgb16->r == r) && (rgb16->g == g) && (rgb16->b == b))
            {
                return pEntry;
            }
            pEntry = pEntry->next;
        }
    }
    return NULL;
}

/* ---------------------------------------- */

static const XPM_HASH_ENTRY * find_xpm_code_hash_color_entry(const XPM_CODE_HASH * xpm_code_hash, const gbm_u8 * code, int code_len)
{
    const int hash_index = calc_xpm_code_hash_index(xpm_code_hash, code, code_len);
    if (hash_index < 0)
    {
        return GBM_FALSE;
    }
    {
        const int allocated_code_len  = xpm_code_hash->code_len;
        const XPM_HASH_ENTRY * pEntry = xpm_code_hash->entries[hash_index];
        while (pEntry != NULL)
        {
            if (code_len == allocated_code_len)
            {
                if (strncmp((char *)(pEntry->code), (const char *)code, code_len) == 0)
                {
                    return pEntry;
                }
            }
            pEntry = pEntry->next;
        }
    }
    return NULL;
}

/* ---------------------------------------- */

static void downscale_xpm_code_hash_transparent(XPM_CODE_HASH * xpm_code_hash)
{
    gbm_u32 i;
    const gbm_u32 hashtable_len = xpm_code_hash->length;
    for (i = 0; i < hashtable_len; i++)
    {
        XPM_HASH_ENTRY * pEntry = xpm_code_hash->entries[i];
        while (pEntry != NULL)
        {
            if (pEntry->type == XPM_PAL_TRANSPARENT)
            {
               #define CVT(x) (((x) * 255) / ((1L << 16) - 1))
                pEntry->rgb16.r = CVT(pEntry->rgb16.r);
                pEntry->rgb16.g = CVT(pEntry->rgb16.g);
                pEntry->rgb16.b = CVT(pEntry->rgb16.b);
               #undef CVT
            }
            pEntry = pEntry->next;
        }
    }
}

/* ---------------------------------------- */

static void upscale_xpm_code_hash_name(XPM_CODE_HASH * xpm_code_hash)
{
    gbm_u32 i;
    const gbm_u32 hashtable_len = xpm_code_hash->length;
    for (i = 0; i < hashtable_len; i++)
    {
        XPM_HASH_ENTRY * pEntry = xpm_code_hash->entries[i];
        while (pEntry != NULL)
        {
            if (pEntry->type == XPM_PAL_NAME)
            {
               #define CVT(x) ((x) * 255)
                pEntry->rgb16.r = CVT(pEntry->rgb16.r);
                pEntry->rgb16.g = CVT(pEntry->rgb16.g);
                pEntry->rgb16.b = CVT(pEntry->rgb16.b);
               #undef CVT
            }
            pEntry = pEntry->next;
        }
    }
}

/* ---------------------------------------- */

/* check the colormap for 16 bit entries */
static int check_color_map(const XPM_CODE_HASH * xpm_code_hash, const gbm_boolean ignore_transparent)
{
    gbm_u32 i;
    const gbm_u32 hashtable_len = xpm_code_hash->length;
    for (i = 0; i < hashtable_len; i++)
    {
        XPM_HASH_ENTRY * pEntry = xpm_code_hash->entries[i];
        while (pEntry != NULL)
        {
            if (pEntry->rgb16.r > 255 || pEntry->rgb16.g > 255 || pEntry->rgb16.b > 255)
            {
               if (((pEntry->type == XPM_PAL_TRANSPARENT) && (! ignore_transparent)) ||
                    (pEntry->type != XPM_PAL_TRANSPARENT))
               {
                  return 16;
               }
            }
            pEntry = pEntry->next;
        }
    }
    return 8;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

static void read_discard_newline(AHEAD *ahead)
{
   /* check if there is a newline character or return ahead */
   int c = gbm_look_ahead(ahead);

   if ((c == '\n') || (c == '\r'))
   {
            long readBytes = 0;
      const long oldPos    = gbm_lseek_ahead(ahead, 0, GBM_SEEK_CUR);

      do
      {
         c = gbm_read_ahead(ahead);
         readBytes++;
      }
      while ((c == '\n') || (c == '\r'));

      /* set file pointer one back to the previous character */
      if (readBytes > 0)
      {
          gbm_lseek_ahead(ahead, oldPos + readBytes - 1, GBM_SEEK_SET);
      }
   }
}

/* ---------------------------------------- */

static void read_discard_restline(AHEAD *ahead)
{
   /* check if there is a newline character or return ahead */
   int c = gbm_look_ahead(ahead);

   if ((c == '\n') || (c == '\r'))
   {
       read_discard_newline(ahead);
       return;
   }
   {
            long readBytes = 0;
      const long oldPos    = gbm_lseek_ahead(ahead, 0, GBM_SEEK_CUR);

      do
      {
         c = gbm_read_ahead(ahead);
         readBytes++;
      }
      while ((c != '\n') && (c != '\r') && (c > 0));

      if ((c == '\n') || (c == '\r'))
      {
          read_discard_newline(ahead);
          return;
      }

      /* set file pointer one back to the previous character */
      if (readBytes > 0)
      {
          gbm_lseek_ahead(ahead, oldPos + readBytes - 1, GBM_SEEK_SET);
      }
   }
}

/* ---------------------------------------- */

static void read_discard_behind_next(AHEAD * ahead, const char cmp)
{
   /* check if there is a newline character or return ahead */
   int c = gbm_look_ahead(ahead);
   if (c == cmp)
   {
       gbm_read_ahead(ahead);
   }
   else
   {
      do
      {
         c = gbm_read_ahead(ahead);
      }
      while ((c != cmp) && (c > 0));
   }
}

/* ---------------------------------------- */

static void skip_C_comment(AHEAD *ahead)
{
   /* check if there is a / character ahead */
   int c = gbm_look_ahead(ahead);
   if (c == '/')
   {
      do
      {
         c = gbm_read_ahead(ahead);
      }
      while ((c == '/') || (c == '*'));

      read_discard_restline(ahead);
   }
}

/* ---------------------------------------- */

static gbm_boolean read_num(AHEAD *ahead, int *num)
{
   int c;
   do
   {
      c = gbm_read_ahead(ahead);
   }
   while ( isspace(c) || (c == '\t') );

   if ((c < '0') || (c > '9'))
   {
       return GBM_FALSE;
   }

   *num = c - '0';
   while ( isdigit(c = gbm_read_ahead(ahead)) )
   {
      *num = *num * 10 + (c - '0');
   }
   return GBM_TRUE;
}

static int read_num_string(AHEAD *ahead, int *num)
{
   int c;

   /* skip everything that is not a " */
   do
   {
      c = gbm_read_ahead(ahead);
   }
   while ((c != '"') && (c > 0));

   do
   {
      c = gbm_read_ahead(ahead);
   }
   while ( isspace(c) || (c == '\t') || (c == '"') );

   if ((c < '0') || (c > '9'))
   {
       return GBM_FALSE;
   }

   *num = c - '0';
   while ( isdigit(c = gbm_read_ahead(ahead)) )
   {
      *num = *num * 10 + (c - '0');
   }
   return GBM_TRUE;
}

/* ---------------------------------------- */

static gbm_u8 read_non_newline_byte_ahead(AHEAD * ahead)
{
  int c;
  /* Discard to end of line */
  do
  {
      c = gbm_read_ahead(ahead);
  }
  while ( (c == '\n') || (c == '\r') );
  return c;
}

static int read_data_index(AHEAD * ahead, int bytes, gbm_u8 * buffer)
{
    int to_read = bytes;

    while (to_read-- > 0)
    {
        *buffer = read_non_newline_byte_ahead(ahead);
        buffer++;
    }
    return (bytes - (to_read + 1));
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR xpm_qft(GBMFT *gbmft)
{
   *gbmft = xpm_gbmft;
   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

static GBM_ERR internal_xpm_rhdr(int fd, GBM * gbm, XPM_FMT_TYPE * type, XPM_CODE_HASH ** xpm_code_hash)
{
   const char  * SIGNATURE_XPM2 = "! XPM2";
   const char  * SIGNATURE_XPM3 = "/* XPM";
         char    signature[7] = { 0 };
   const char  * s     = NULL;
         AHEAD * ahead = NULL;

   gbm_boolean use_native_bpp, valid;

   XPM_PRIV_READ *xpm_priv = (XPM_PRIV_READ *) gbm->priv;

   if (xpm_code_hash != NULL)
   {
      *xpm_code_hash = NULL;
   }

   xpm_priv->has_deep_color   = GBM_FALSE;
   xpm_priv->has_transparency = GBM_FALSE;

   /* check if extended color depths are requested */
   use_native_bpp = (gbm_find_word(xpm_priv->read_options, "ext_bpp") != NULL)
                    ? GBM_TRUE : GBM_FALSE;

   /* set default background color to black */
   xpm_priv->backrgb.r = 0;
   xpm_priv->backrgb.g = 0;
   xpm_priv->backrgb.b = 0;

   /* parse RGB value for background mixing with alpha channel */
   if ((s = gbm_find_word_prefix(xpm_priv->read_options, "back_rgb=")) != NULL)
   {
       int image_background_red   = 0;
       int image_background_green = 0;
       int image_background_blue  = 0;

       if (sscanf(s + 9, "%d_%d_%d", &image_background_red,
                                     &image_background_green,
                                     &image_background_blue) != 3)
       {
          return GBM_ERR_BAD_OPTION;
       }

       if (((image_background_red   < 0) || (image_background_red   > 0xffff)) ||
           ((image_background_green < 0) || (image_background_green > 0xffff)) ||
           ((image_background_blue  < 0) || (image_background_blue  > 0xffff)))
       {
          return GBM_ERR_BAD_OPTION;
       }

       xpm_priv->backrgb.r = image_background_red;
       xpm_priv->backrgb.g = image_background_green;
       xpm_priv->backrgb.b = image_background_blue;
   }

   /* start at the beginning of the file */
   gbm_file_lseek(fd, 0, GBM_SEEK_SET);

   /* use read ahead from here */
   ahead = gbm_create_ahead(fd);
   if (ahead == NULL)
   {
       return GBM_ERR_MEM;
   }

   /* check format signature "! XPM2" */
   if (gbm_readbuf_ahead(ahead, (gbm_u8 *)signature, sizeof(signature)) != sizeof(signature))
   {
      gbm_destroy_ahead(ahead);
      return GBM_ERR_READ;
   }
   if (strncmp(signature, SIGNATURE_XPM2, strlen(SIGNATURE_XPM2)) == 0)
   {
      *type = XPM_FMT_XPM2;
   }
   else if (strncmp(signature, SIGNATURE_XPM3, strlen(SIGNATURE_XPM3)) == 0)
   {
      *type = XPM_FMT_XPM3;
   }
   else
   {
      gbm_destroy_ahead(ahead);
      return GBM_ERR_BAD_MAGIC;
   }

   /* read width, height, color count, chars per pixel */
   valid = GBM_FALSE;
   switch(*type)
   {
      case XPM_FMT_XPM2:
         valid = read_num(ahead, &(gbm->w));
         valid = valid && read_num(ahead, &(gbm->h));
         valid = valid && read_num(ahead, &(xpm_priv->valid_colors));
         valid = valid && read_num(ahead, &(xpm_priv->chars_per_pixel));
         read_discard_newline(ahead);
         break;

      case XPM_FMT_XPM3:
         valid = read_num_string(ahead, &(gbm->w));
         valid = valid && read_num(ahead, &(gbm->h));
         valid = valid && read_num(ahead, &(xpm_priv->valid_colors));
         valid = valid && read_num(ahead, &(xpm_priv->chars_per_pixel));
         read_discard_restline(ahead);
         skip_C_comment(ahead);
         break;

      default:
         gbm_destroy_ahead(ahead);
         return GBM_ERR_BAD_MAGIC;
   }

   if ((! valid) || (gbm->w < 1) || (gbm->h < 1) ||
       (xpm_priv->valid_colors < 1) || (xpm_priv->chars_per_pixel < 1))
   {
      gbm_destroy_ahead(ahead);
      return GBM_ERR_BAD_MAGIC;
   }

   /* map color count to a GBM supported colour depth in bpp */

   /* We have to decode the color palette here to figure out */
   /* whether we have more than 8 bit per component. */

   /* read the color table */
   xpm_priv->has_deep_color   = GBM_FALSE;
   xpm_priv->has_transparency = GBM_FALSE;
   {
       XPM_CODE_HASH * local_xpm_code_hash = create_xpm_code_hash(xpm_priv->valid_colors, xpm_priv->chars_per_pixel);
       if (local_xpm_code_hash == NULL)
       {
          gbm_destroy_ahead(ahead);
          return GBM_ERR_MEM;
       }

       if (internal_xpm_rpal_16bpp(ahead, gbm, *type, local_xpm_code_hash) != GBM_ERR_OK)
       {
          gbm_destroy_ahead(ahead);
          free_xpm_code_hash(local_xpm_code_hash);
          return GBM_ERR_XPM_BPP;
       }
       gbm_destroy_ahead(ahead);
       ahead = NULL;

       if (find_xpm_code_hash_transparent_entry(local_xpm_code_hash) != NULL)
       {
           xpm_priv->has_transparency = GBM_TRUE;
       }

       /* check if we have more than 8 bit per color component */
       if (check_color_map(local_xpm_code_hash, xpm_priv->has_transparency) == 16)
       {
           /* Don't waste time if no color palette is requested by the caller. */
           if (xpm_code_hash != NULL)
           {
               upscale_xpm_code_hash_name(local_xpm_code_hash);
           }
           xpm_priv->has_deep_color = GBM_TRUE;
       }
       else
       {
           /* Don't waste time if no color palette is requested by the caller. */
           if (xpm_priv->has_transparency && (xpm_code_hash != NULL))
           {
               /* scale transparency entries down to 8 bit */
               downscale_xpm_code_hash_transparent(local_xpm_code_hash);
           }
       }

       /* check if the caller requested the color table, so keep it */
       if (xpm_code_hash != NULL)
       {
           /* return our table, the caller has to free it */
           *xpm_code_hash = local_xpm_code_hash;
       }
       else
       {
           free_xpm_code_hash(local_xpm_code_hash);
           local_xpm_code_hash = NULL;
       }
   }

   /* We might want to check if the user has requested to calculate the */
   /* transparency against a provided background color (parameter not yet existent). */
   /* So far we will ignore this. */

   if (xpm_priv->valid_colors == 2)
   {
       gbm->bpp = 1;
   }
   else if (xpm_priv->valid_colors <= (1U<<4))
   {
       gbm->bpp = 4;
   }
   else if (xpm_priv->valid_colors <= (1U<<8))
   {
       gbm->bpp = 8;
   }
   else if (xpm_priv->valid_colors <= (1U<<24))
   {
       gbm->bpp = 24;
   }
   else
   {
       if (xpm_code_hash != NULL)
       {
          if (*xpm_code_hash != NULL)
          {
              free_xpm_code_hash(*xpm_code_hash);
              *xpm_code_hash = NULL;
          }
       }
       return GBM_ERR_XPM_BPP;
   }

   /* Adapt to format specialties if native color depth is requested. */
   if (use_native_bpp)
   {
      if (xpm_priv->has_deep_color)
      {
         gbm->bpp = xpm_priv->has_transparency ? 64 : 48;
      }
      else if (xpm_priv->has_transparency)
      {
         gbm->bpp = 32;
      }
   }

   if ((gbm->bpp < 24) && xpm_priv->has_transparency)
   {
       /* decode as true color against a background color */
       gbm->bpp = 24;
   }

   return GBM_ERR_OK;
}

/* ---------------------------------------- */

GBM_ERR xpm_rhdr(const char *fn, int fd, GBM *gbm, const char *opt)
{
   XPM_PRIV_READ *xpm_priv = (XPM_PRIV_READ *) gbm->priv;
   XPM_FMT_TYPE   type;

   fn=fn; /* Suppress 'unref arg' compiler warnings */

   /* copy possible options */
   if (strlen(opt) >= sizeof(xpm_priv->read_options))
   {
      return GBM_ERR_BAD_OPTION;
   }
   strcpy(xpm_priv->read_options, opt);

   /* read bitmap info */
   return internal_xpm_rhdr(fd, gbm, &type, NULL /* no color info required */);
}

/* ---------------------------------------- */
/* ---------------------------------------- */

static gbm_boolean read_color_line(AHEAD * ahead,
                                   gbm_u8  * code , int codeLen, int codeWidth,
                                   gbm_u8  * color, int colorLen, gbm_boolean skipStringChar)
{
    /* Format of a line: code c color (code can contain spaces, color can be #hex or a symbolic name) */
    int b, c;

    if (codeWidth > codeLen)
    {
        return GBM_FALSE;
    }

    memset(code , 0, codeLen);
    memset(color, 0, colorLen);

    if (skipStringChar)
    {
        /* skip all leading white spaces */
        do
        {
            c = gbm_read_ahead(ahead);
        }
        while ((c == ' ') || (c == '\t'));

        /* read " (must be there) */
        if (c != '"')
        {
            return GBM_FALSE;
        }
    }

    /* read codeWidth chars */
    for (b = 0; b < codeWidth; b++)
    {
        code[b] = gbm_read_ahead(ahead);
    }
    /* read one trailing white space or tab (must be there) */
    c = gbm_read_ahead(ahead);
    if ((c != ' ') && (c != '\t'))
    {
        return GBM_FALSE;
    }

    /* skip all following white spaces until a "c " was found */
    do
    {
        c = gbm_read_ahead(ahead);
    }
    while (((c == ' ') || (c == '\t') || (skipStringChar && (c == '"'))));

    if (c == 'c')
    {
        /* check that there is a trailing whitespace */
        c = gbm_read_ahead(ahead);
        if ((c != ' ') && (c != '\t') && (!(skipStringChar && (c != '"'))))
        {
            return GBM_FALSE;
        }
    }

    /* skip all following white spaces */
    do
    {
        c = gbm_read_ahead(ahead);
    }
    while (((c == ' ') || (c == '\t') || (skipStringChar && (c == '"'))));

    /* read all characters that don't contain white spaces and are not a line end indicator */
    b = 0;
    color[b++] = c;

    if (c == '#')
    {
        while (b < colorLen)
        {
            c = gbm_read_ahead(ahead);
            if (! isxdigit(c))
            {
                if (skipStringChar)
                {
                    read_discard_restline(ahead);
                }
                else
                {
                    read_discard_newline(ahead);
                }
                break;
            }
            color[b] = c;
            b++;
        }
    }
    else
    {
        while (b < colorLen)
        {
            c = gbm_read_ahead(ahead);
            if ((! isalnum(c)) && (c != '_'))
            {
                if (skipStringChar)
                {
                    read_discard_restline(ahead);
                }
                else
                {
                    read_discard_newline(ahead);
                }
                break;
            }
            color[b] = c;
            b++;
        }
    }

    if (b == colorLen)
    {
        return GBM_FALSE; /* return buffer too small */
    }

    return GBM_TRUE;
}

/* ---------------------------------------- */

/* internal_xpm_rpal_16bpp() - Read 16bpp palette */
static GBM_ERR internal_xpm_rpal_16bpp(AHEAD         * ahead,
                                       GBM           * gbm,
                                       XPM_FMT_TYPE    fmt_type,
                                       XPM_CODE_HASH * xpm_code_hash)
{
    XPM_PRIV_READ *xpm_priv = (XPM_PRIV_READ *) gbm->priv;

          gbm_u8      * buffer   = NULL;
          gbm_u8      * code     = NULL;
          gbm_u8      * color    = NULL;
    const int           codeLen  = MAX(50, xpm_priv->chars_per_pixel + 1);
    const int           colorLen = codeLen;
          gbm_u32       entry;
          XPM_PAL_TYPE  typeEnum;
          GBMRGB_16BPP  gbmrgb16;

    if (fmt_type != XPM_FMT_XPM2)
    {
        skip_C_comment(ahead);
    }

    /* get the palette information */

    /* read the palette lines and decode them */
    /* Format: . c #00AAFF */

    buffer = (gbm_u8 *) malloc(codeLen + colorLen);
    if (buffer == NULL)
    {
        return GBM_ERR_MEM;
    }
    code  = buffer;
    color = code + codeLen;

    for (entry = 0; entry < xpm_code_hash->length; entry++)
    {
        switch(fmt_type)
        {
           case XPM_FMT_XPM2:
              if (! read_color_line(ahead, code , codeLen, xpm_priv->chars_per_pixel,
                                           color, colorLen, GBM_FALSE))
              {
                  free(buffer);
                  return GBM_ERR_READ;
              }
              break;

           case XPM_FMT_XPM3:
              if (! read_color_line(ahead, code , codeLen, xpm_priv->chars_per_pixel,
                                           color, colorLen, GBM_TRUE))
              {
                  free(buffer);
                  return GBM_ERR_READ;
              }
              break;

           default:
              free(buffer);
              return GBM_ERR_READ;
        }

        typeEnum = (color[0] == '#') ? XPM_PAL_RGB : XPM_PAL_NAME;

        switch(typeEnum)
        {
            case XPM_PAL_RGB:
                if (! rgb16FromHex(color, strlen((const char *)color), &gbmrgb16))
                {
                    free(buffer);
                    return GBM_ERR_READ;
                }
                break;

            case XPM_PAL_NAME:
                if (! rgb16FromColorName(color, strlen((const char *)color), &gbmrgb16))
                {
                    if (strlen((const char *)color) == 4) /* "none" ? */
                    {
                        if ((strncmp((const char *)color, "none", 4) == 0) || (strncmp((const char *)color, "None", 4) == 0))
                        {
                            /* store transparency as background color in color table for later decoding */
                            add_xpm_code_hash(xpm_code_hash, code, xpm_priv->chars_per_pixel,
                                              XPM_PAL_TRANSPARENT, &(xpm_priv->backrgb), entry);
                            continue;
                        }
                    }
                    free(buffer);
                    return GBM_ERR_READ;
                }
                break;

            default:
                free(buffer);
                return GBM_ERR_NOT_SUPP;
        }

        /* store in color table for later decoding */
        add_xpm_code_hash(xpm_code_hash, code, xpm_priv->chars_per_pixel, typeEnum, &gbmrgb16, entry);
    }

    free(buffer);

    /* discard remaining newline */
    read_discard_newline(ahead);

    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */

/* internal_xpm_rpal_8bpp() - Read palette */
static GBM_ERR internal_xpm_rpal_8bpp(GBM *gbm, GBMRGB *gbmrgb,
                                      XPM_CODE_HASH * xpm_code_hash, const int palette_entries)
{
    if (palette_entries <= 256)
    {
       XPM_PRIV_READ * xpm_priv = (XPM_PRIV_READ *) gbm->priv;
       const gbm_u32   len = xpm_code_hash->length;
       gbm_u32 i;

       /* copy to external GBMRGB struct and downscale if necessary */
       if (xpm_priv->has_deep_color)
       {
          for (i = 0; i < len; i++)
          {
             const XPM_HASH_ENTRY * pEntry = xpm_code_hash->entries[i];
             while (pEntry != NULL)
             {
                if (pEntry->index < palette_entries)
                {
                  #define CVT(x) (((x) * 255) / ((1L << 16) - 1))
                   gbmrgb[pEntry->index].r = (gbm_u8) CVT(pEntry->rgb16.r);
                   gbmrgb[pEntry->index].g = (gbm_u8) CVT(pEntry->rgb16.g);
                   gbmrgb[pEntry->index].b = (gbm_u8) CVT(pEntry->rgb16.b);
                  #undef CVT
                }
                pEntry = pEntry->next;
             }
          }
       }
       else
       {
          for (i = 0; i < len; i++)
          {
             const XPM_HASH_ENTRY * pEntry = xpm_code_hash->entries[i];
             while (pEntry != NULL)
             {
                if (pEntry->index < palette_entries)
                {
                   gbmrgb[pEntry->index].r = (gbm_u8) pEntry->rgb16.r;
                   gbmrgb[pEntry->index].g = (gbm_u8) pEntry->rgb16.g;
                   gbmrgb[pEntry->index].b = (gbm_u8) pEntry->rgb16.b;
                }
                pEntry = pEntry->next;
             }
          }
       }
    }
    return GBM_ERR_OK;
}

/* ----------------------------------------------------------- */

GBM_ERR xpm_rpal(int fd, GBM *gbm, GBMRGB *gbmrgb)
{
   if (gbm == NULL)
   {
      return GBM_ERR_BAD_ARG;
   }
   if (gbm->bpp <= 8)
   {
      XPM_PRIV_READ *xpm_priv        = (XPM_PRIV_READ *) gbm->priv;
      XPM_FMT_TYPE   type            = XPM_FMT_UNKNOWN;
      XPM_CODE_HASH *xpm_code_hash   = NULL;
      const int      palette_entries = 1 << gbm->bpp;

      GBM_ERR rc = internal_xpm_rhdr(fd, gbm, &type, &xpm_code_hash);
      if (rc != GBM_ERR_OK)
      {
         return rc;
      }

      /* We can only return the first 256 colors as this is the */
      /* restriction of the GBM interface. */
      if (xpm_priv->valid_colors <= palette_entries)
      {
         rc = internal_xpm_rpal_8bpp(gbm, gbmrgb, xpm_code_hash, xpm_priv->valid_colors);
      }
      else
      {
         rc = internal_xpm_rpal_8bpp(gbm, gbmrgb, xpm_code_hash, palette_entries);
      }

      free_xpm_code_hash(xpm_code_hash);

      if (rc != GBM_ERR_OK)
      {
         return rc;
      }
   }
   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

GBM_ERR xpm_rdata(int fd, GBM *gbm, gbm_u8 *data)
{
   XPM_PRIV_READ *xpm_priv = (XPM_PRIV_READ *) gbm->priv;

   AHEAD         * ahead         = NULL;
   XPM_CODE_HASH * xpm_code_hash = NULL;
   const int       num_colors    = xpm_priv->valid_colors;
   const size_t    stride        = ((gbm->w * gbm->bpp + 31)/32) * 4;
   XPM_FMT_TYPE    type          = XPM_FMT_UNKNOWN;

   GBM_ERR rc = internal_xpm_rhdr(fd, gbm, &type, &xpm_code_hash);
   if (rc != GBM_ERR_OK)
   {
      return rc;
   }
   /* check for correct parameters */
   if ( (stride     != (size_t)((gbm->w * gbm->bpp + 31)/32) * 4) ||
        (num_colors != xpm_priv->valid_colors) )
   {
       free_xpm_code_hash(xpm_code_hash);
       return GBM_ERR_READ;
   }

   /* read the data of a line and create the pixel data by using the color table */

   ahead = gbm_create_ahead(fd);
   if (ahead == NULL)
   {
     free_xpm_code_hash(xpm_code_hash);
     return GBM_ERR_READ;
   }

   if (type != XPM_FMT_XPM2)
   {
       int c;

       skip_C_comment(ahead);

       /* skip everything that is not a " */
       do
       {
          c = gbm_read_ahead(ahead);
       }
       while (c != '"');
   }

   switch(gbm->bpp)
   {
       case 1:
           rc = read_data_1bpp(ahead, type, gbm, data, xpm_code_hash);
           if (rc != GBM_ERR_OK)
           {
              gbm_destroy_ahead(ahead);
              free_xpm_code_hash(xpm_code_hash);
              return rc;
           }
           break;

       case 4:
           rc = read_data_4bpp(ahead, type, gbm, data, xpm_code_hash);
           if (rc != GBM_ERR_OK)
           {
              gbm_destroy_ahead(ahead);
              free_xpm_code_hash(xpm_code_hash);
              return rc;
           }
           break;

       case 8:
           rc = read_data_8bpp(ahead, type, gbm, data, xpm_code_hash);
           if (rc != GBM_ERR_OK)
           {
              gbm_destroy_ahead(ahead);
              free_xpm_code_hash(xpm_code_hash);
              return rc;
           }
           break;

       case 24:
           rc = read_data_24bpp(ahead, type, gbm, data, xpm_code_hash);
           if (rc != GBM_ERR_OK)
           {
              gbm_destroy_ahead(ahead);
              free_xpm_code_hash(xpm_code_hash);
              return rc;
           }
           break;

       case 32:
           rc = read_data_32bpp(ahead, type, gbm, data, xpm_code_hash);
           if (rc != GBM_ERR_OK)
           {
              gbm_destroy_ahead(ahead);
              free_xpm_code_hash(xpm_code_hash);
              return rc;
           }
           break;

       case 48:
           rc = read_data_48bpp(ahead, type, gbm, data, xpm_code_hash);
           if (rc != GBM_ERR_OK)
           {
              gbm_destroy_ahead(ahead);
              free_xpm_code_hash(xpm_code_hash);
              return rc;
           }
           break;

       case 64:
           rc = read_data_64bpp(ahead, type, gbm, data, xpm_code_hash);
           if (rc != GBM_ERR_OK)
           {
              gbm_destroy_ahead(ahead);
              free_xpm_code_hash(xpm_code_hash);
              return rc;
           }
           break;

       default:
           gbm_destroy_ahead(ahead);
           free_xpm_code_hash(xpm_code_hash);
           return GBM_ERR_NOT_SUPP;
   }

   gbm_destroy_ahead(ahead);
   free_xpm_code_hash(xpm_code_hash);

   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

static GBM_ERR extract_unique_colors_of_table(const GBM * gbm, const GBMRGB * gbmrgb, const gbm_u8 * data,
                                              int transcol_idx, XPM_RGB_HASH ** xpm_rgb_hash)
{
   if ((gbm->bpp != 1) && (gbm->bpp != 4) && (gbm->bpp != 8))
   {
      return GBM_ERR_NOT_SUPP;
   }
   {
      /* pack the color table to the really used entries */
      int used_colors = 0;
      int x, y, lower_limit, upper_limit;
      const int palette_entries = 1 << gbm->bpp;
      const size_t stride = ((gbm->w * gbm->bpp + 31)/32) * 4;

      const gbm_u8 * p = data;
      gbm_u8 * indices = (gbm_u8 *) calloc(palette_entries, sizeof(gbm_u8));
      if (indices == NULL)
      {
          return GBM_ERR_MEM;
      }

      /* Loop over the bitmap data and set each found index in the table to one */
      switch(gbm->bpp)
      {
          case 1:
              for (y = 0; y < gbm->h; y++)
              {
                  gbm_u8 c = 0;
                  const gbm_u8 * pt = p;
                  for (x = 0; x < gbm->w; x++)
                  {
                      if ((x & 7) == 0)
                      {
                          c = *pt++;
                      }
                      else
                      {
                          c <<= 1;
                      }
                      indices[c >> 7] = 1;
                  }
                  p += stride;
              }
              break;

          case 4:
              for (y = 0; y < gbm->h; y++)
              {
                  gbm_u8 c;
                  const gbm_u8 * pt = p;
                  for (x = 0; x + 1 < gbm->w; x += 2)
                  {
                      c = *pt++;
                      indices[c >>  4] = 1;
                      indices[c &  15] = 1;
                  }
                  if (x < gbm->w)
                  {
                      c = *pt;
                      indices[c >>  4] = 1;
                  }
                  p += stride;
              }
              break;

          case 8:
              for (y = 0; y < gbm->h; y++)
              {
                  const gbm_u8 * pt = p;
                  for (x = 0; x < gbm->w; x++)
                  {
                    indices[*pt++] = 1;
                  }
                  p += stride;
              }
              break;

          default:
              free(indices);
              return GBM_ERR_NOT_SUPP;
      }

      /* count used colors */
      lower_limit = -1;
      upper_limit = 0;
      for (x = 0; x < palette_entries; x++)
      {
          if (indices[x])
          {
              used_colors++;
              if (lower_limit < 0)
              {
                  lower_limit = x;
              }
              upper_limit = MAX(x, upper_limit);
          }
      }
      if ((lower_limit < 0) || (used_colors == 0))
      {
          free(indices);
          return GBM_ERR_WRITE;
      }
      if (transcol_idx > -1)
      {
          if ((transcol_idx < lower_limit) || (transcol_idx > upper_limit))
          {
              free(indices);
              return GBM_ERR_TRANSCOL_IDX;
          }
          if (indices[transcol_idx] == 0)
          {
              free(indices);
              return GBM_ERR_TRANSCOL_IDX;
          }
      }
      {
          const short    code_len = (short)(log((double)used_colors)/log(92.0)) + 1;
          XPM_PAL_TYPE   pal_type = XPM_PAL_UNUSED;
          GBMRGB_16BPP   rgb16    = { 0, 0, 0 };
          gbm_u8       * codebuf  = NULL;

          /* add a unique color code to xpm_rgb_hash for every found index */
          *xpm_rgb_hash = create_xpm_rgb_hash(used_colors, code_len);
          if (*xpm_rgb_hash == NULL)
          {
              free(indices);
              return GBM_ERR_MEM;
          }

          /* Calculate chars per pixel for color table encoding: */
          codebuf = (gbm_u8 *) calloc(code_len + 1, sizeof(gbm_u8));
          if (codebuf == NULL)
          {
              free(indices);
              free_xpm_rgb_hash(*xpm_rgb_hash);
              *xpm_rgb_hash = NULL;
              return GBM_ERR_WRITE;
          }

          y = 0;
          for (x = lower_limit; x <= upper_limit; x++)
          {
              if (indices[x])
              {
                  rgb16.r = gbmrgb[x].r;
                  rgb16.g = gbmrgb[x].g;
                  rgb16.b = gbmrgb[x].b;

                  pal_type = (x == transcol_idx) ? XPM_PAL_TRANSPARENT : XPM_PAL_RGB;

                  Base92(y, codebuf, code_len);

                  if (! add_xpm_rgb_hash(*xpm_rgb_hash, codebuf, code_len, pal_type, &rgb16, y))
                  {
                     free(codebuf);
                     free(indices);
                     free_xpm_rgb_hash(*xpm_rgb_hash);
                     *xpm_rgb_hash = NULL;
                     return GBM_ERR_WRITE;
                  }
                  y++;
              }
          }
          free(codebuf);
          codebuf = NULL;
      }
      free(indices);
      indices = NULL;
   }

   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

static gbm_boolean is_color_used(const gbm_u8 *flags, gbm_u8 r, gbm_u8 g, gbm_u8 b, gbm_u32 *li, gbm_u32 *lx)
{
    /* Get the long value of the bit's color */
    const gbm_u32 cLong = MakeBGR(b, g, r);

    *li = cLong / 8; /* Divide by 8 to get it's section in our color registry */
    *lx = cLong % 8; /* Get the right bit in the index for the color */

    return (flags[*li] & (1 << *lx)) ? GBM_TRUE : GBM_FALSE;
}

static void set_color_used(gbm_u8 *flags, gbm_u32 li, gbm_u32 lx)
{
    flags[li] |= (1 << lx);
}

static void unset_color_used(gbm_u8 *flags, gbm_u32 li, gbm_u32 lx)
{
    flags[li] -= (1 << lx);
}

/* ---------------------------------------- */

static gbm_u8 * create_table_of_used_colors(const GBM * gbm, const gbm_u8 * data, int * used_colors)
{
   gbm_u8 * flags = NULL;

   if (gbm->bpp != 24)
   {
      return NULL;
   }
   {
      /* pack the color table to the really used entries */
      const size_t stride = ((gbm->w * gbm->bpp + 31)/32) * 4;
      const gbm_u8 * p      = data;

      /* A bit array's width must be a multiple of 4 per scan line */
      int   x, y;
      gbm_u32 li, lx;
      gbm_u8  r, g, b;

      /* Allocate our color registry (&H200000 bytes = 16,777,216 bits) */
      flags = (gbm_u8 *) gbmmem_calloc(0x200000, sizeof(gbm_u8));
      if (flags == NULL)
      {
          return NULL;
      }
      *used_colors = 0;
      for (y = 0; y < gbm->h; y++)
      {
          const gbm_u8 * pt = p;
          for (x = 0; x < gbm->w; x++)
          {
            b = *pt++;
            g = *pt++;
            r = *pt++;

            /* If the index's bit is not set... */
            if (! is_color_used(flags, r, g, b, &li, &lx))
            {
               (*used_colors)++;              /* Add 1 more to the count */
               set_color_used(flags, li, lx); /* And set it so we don't count it again */
            }
          }
          p += stride;
      }
  }
  return flags;
}

/* ---------------------------------------- */

static GBM_ERR extract_unique_colors_of_bitmap(const GBM * gbm, const gbm_u8 * data,
                                               const GBMRGB_16BPP * transcol_rgb16,
                                               XPM_RGB_HASH ** xpm_rgb_hash)
{
   if (gbm->bpp != 24)
   {
      return GBM_ERR_NOT_SUPP;
   }
   {
      /* pack the color table to the really used entries */
      int used_colors = 0;

      /* Build the color registry (&H200000 bytes = 16,777,216 bits) */
      gbm_u8 * flags = create_table_of_used_colors(gbm, data, &used_colors);
      if (flags == NULL)
      {
          return GBM_ERR_MEM;
      }
      {
          const short  code_len = (short)(log((double)used_colors)/log(92.0)) + 1;
          const size_t stride   = ((gbm->w * gbm->bpp + 31)/32) * 4;
          const gbm_u8  *p        = data + (stride * (gbm->h - 1));
          gbm_u8        *codebuf  = NULL;
          GBMRGB_16BPP rgb16;
          int          x, y, color_index;
          gbm_u32        li, lx;

          /* add a unique color code to xpm_rgb_hash for every found index */
          *xpm_rgb_hash = create_xpm_rgb_hash(used_colors, code_len);
          if (*xpm_rgb_hash == NULL)
          {
              gbmmem_free(flags);
              return GBM_ERR_MEM;
          }
          /* Calculate chars per pixel for color table encoding: */
          codebuf = (gbm_u8 *) calloc(code_len + 1, sizeof(gbm_u8));
          if (codebuf == NULL)
          {
              gbmmem_free(flags);
              free_xpm_rgb_hash(*xpm_rgb_hash);
              *xpm_rgb_hash = NULL;
              return GBM_ERR_WRITE;
          }

          color_index = 0;
          for (y = gbm->h - 1; y >= 0; y--)
          {
              const gbm_u8 * pt = p;
              for (x = 0; x < gbm->w; x++)
              {
                rgb16.b = *pt++;
                rgb16.g = *pt++;
                rgb16.r = *pt++;

                /* If the index's bit is set... */
                if (is_color_used(flags, (gbm_u8) rgb16.r, (gbm_u8) rgb16.g, (gbm_u8) rgb16.b, &li, &lx))
                {
                    XPM_PAL_TYPE pal_type = XPM_PAL_RGB;

                    /* check if this is the transparency color */
                    if (transcol_rgb16 != NULL)
                    {
                       if ((transcol_rgb16->r == rgb16.r) &&
                           (transcol_rgb16->g == rgb16.g) &&
                           (transcol_rgb16->b == rgb16.b))
                       {
                           pal_type = XPM_PAL_TRANSPARENT;
                       }
                    }
                    Base92(color_index, codebuf, code_len);
                    if (! add_xpm_rgb_hash(*xpm_rgb_hash, codebuf, code_len, pal_type, &rgb16, color_index))
                    {
                       free(codebuf);
                       gbmmem_free(flags);
                       free_xpm_rgb_hash(*xpm_rgb_hash);
                       *xpm_rgb_hash = NULL;
                       return GBM_ERR_WRITE;
                    }
                    /* And unset it so we don't count it again */
                    unset_color_used(flags, li, lx);
                    color_index++;
                }
              }
              p -= stride;
          }
          free(codebuf);
          codebuf = NULL;
      }
      gbmmem_free(flags);
      flags = NULL;
   }

   return GBM_ERR_OK;
}

/* ---------------------------------------- */

static GBM_ERR xpm_w_index_based_bitmap_data(WCACHE * wcache,
                                             XPM_FMT_TYPE fmt_type,
                                             const GBM * gbm, const GBMRGB * gbmrgb, const gbm_u8 * data,
                                             const XPM_RGB_HASH * xpm_rgb_hash)
{
   if ((gbm->bpp != 1) && (gbm->bpp != 4) && (gbm->bpp != 8))
   {
      return GBM_ERR_NOT_SUPP;
   }
   {
      int                   x, y;
      const size_t          stride   = ((gbm->w * gbm->bpp + 31)/32) * 4;
      const gbm_u8         *p        = data + (stride * (gbm->h - 1));
      const XPM_HASH_ENTRY *pEntry   = NULL;
      const gbm_u32         code_len = xpm_rgb_hash->code_len;
      const GBMRGB         *pRGB;

      switch(gbm->bpp)
      {
          case 1:
              for (y = gbm->h - 1; y >= 0; y--)
              {
                  gbm_u8 c = 0;
                  const gbm_u8 * pt = p;
                  if (fmt_type != XPM_FMT_XPM2)
                  {
                      if (gbm_write_wcache(wcache, '\"') != 1)
                      {
                          return GBM_ERR_WRITE;
                      }
                  }
                  for (x = 0; x < gbm->w; x++)
                  {
                      if ((x & 7) == 0)
                      {
                          c = *pt++;
                      }
                      else
                      {
                          c <<= 1;
                      }
                      pRGB   = &(gbmrgb[c >> 7]);
                      pEntry = find_xpm_rgb_hash_color_code(xpm_rgb_hash, pRGB->r, pRGB->g, pRGB->b);
                      if (pEntry == NULL)
                      {
                           return GBM_ERR_WRITE;
                      }
                      assert(pEntry->type != XPM_PAL_UNUSED);

                      if (gbm_writebuf_wcache(wcache, pEntry->code, code_len) != code_len)
                      {
                          return GBM_ERR_WRITE;
                      }
                  }
                  p -= stride;
                  if (fmt_type == XPM_FMT_XPM2)
                  {
                      if (gbm_write_wcache(wcache, '\n') != 1)
                      {
                          return GBM_ERR_WRITE;
                      }
                  }
                  else
                  {
                      if (y > 0)
                      {
                          if (gbm_writebuf_wcache(wcache, (const gbm_u8 *) "\",\n", 3) != 3)
                          {
                              return GBM_ERR_WRITE;
                          }
                      }
                      else
                      {
                          if (gbm_writebuf_wcache(wcache, (const gbm_u8 *) "\"\n", 2) != 2)
                          {
                              return GBM_ERR_WRITE;
                          }
                      }
                  }
              }
              break;

          case 4:
              for (y = gbm->h - 1; y >= 0; y--)
              {
                  gbm_u8 c;
                  const gbm_u8 * pt = p;
                  if (fmt_type != XPM_FMT_XPM2)
                  {
                      if (gbm_write_wcache(wcache, '\"') != 1)
                      {
                          return GBM_ERR_WRITE;
                      }
                  }
                  for (x = 0; x + 1 < gbm->w; x += 2)
                  {
                      c      = *pt++;
                      pRGB   = &(gbmrgb[c >> 4]);
                      pEntry = find_xpm_rgb_hash_color_code(xpm_rgb_hash, pRGB->r, pRGB->g, pRGB->b);
                      if (pEntry == NULL)
                      {
                           return GBM_ERR_WRITE;
                      }
                      assert(pEntry->type != XPM_PAL_UNUSED);

                      if (gbm_writebuf_wcache(wcache, pEntry->code, code_len) != code_len)
                      {
                          return GBM_ERR_WRITE;
                      }
                      pRGB   = &(gbmrgb[c & 0x0f]);
                      pEntry = find_xpm_rgb_hash_color_code(xpm_rgb_hash, pRGB->r, pRGB->g, pRGB->b);
                      if (pEntry == NULL)
                      {
                           return GBM_ERR_WRITE;
                      }
                      assert(pEntry->type != XPM_PAL_UNUSED);

                      if (gbm_writebuf_wcache(wcache, pEntry->code, code_len) != code_len)
                      {
                          return GBM_ERR_WRITE;
                      }
                  }
                  if (x < gbm->w)
                  {
                      c      = *pt;
                      pRGB   = &(gbmrgb[c >> 4]);
                      pEntry = find_xpm_rgb_hash_color_code(xpm_rgb_hash, pRGB->r, pRGB->g, pRGB->b);
                      if (pEntry == NULL)
                      {
                           return GBM_ERR_WRITE;
                      }
                      assert(pEntry->type != XPM_PAL_UNUSED);

                      if (gbm_writebuf_wcache(wcache, pEntry->code, code_len) != code_len)
                      {
                          return GBM_ERR_WRITE;
                      }
                  }
                  p -= stride;
                  if (fmt_type == XPM_FMT_XPM2)
                  {
                      if (gbm_write_wcache(wcache, '\n') != 1)
                      {
                          return GBM_ERR_WRITE;
                      }
                  }
                  else
                  {
                      if (y > 0)
                      {
                          if (gbm_writebuf_wcache(wcache, (const gbm_u8 *)"\",\n", 3) != 3)
                          {
                              return GBM_ERR_WRITE;
                          }
                      }
                      else
                      {
                          if (gbm_writebuf_wcache(wcache, (const gbm_u8 *)"\"\n", 2) != 2)
                          {
                              return GBM_ERR_WRITE;
                          }
                      }
                  }
              }
              break;

          case 8:
              for (y = gbm->h - 1; y >= 0; y--)
              {
                  const gbm_u8 * pt = p;
                  if (fmt_type != XPM_FMT_XPM2)
                  {
                      if (gbm_write_wcache(wcache, '\"') != 1)
                      {
                          return GBM_ERR_WRITE;
                      }
                  }
                  for (x = 0; x < gbm->w; x++)
                  {
                      pRGB   = &(gbmrgb[*pt++]);
                      pEntry = find_xpm_rgb_hash_color_code(xpm_rgb_hash, pRGB->r, pRGB->g, pRGB->b);
                      if (pEntry == NULL)
                      {
                           return GBM_ERR_WRITE;
                      }
                      assert(pEntry->type != XPM_PAL_UNUSED);

                      if (gbm_writebuf_wcache(wcache, pEntry->code, code_len) != code_len)
                      {
                          return GBM_ERR_WRITE;
                      }
                  }
                  p -= stride;
                  if (fmt_type == XPM_FMT_XPM2)
                  {
                      if (gbm_write_wcache(wcache, '\n') != 1)
                      {
                          return GBM_ERR_WRITE;
                      }
                  }
                  else
                  {
                      if (y > 0)
                      {
                          if (gbm_writebuf_wcache(wcache, (const gbm_u8 *)"\",\n", 3) != 3)
                          {
                              return GBM_ERR_WRITE;
                          }
                      }
                      else
                      {
                          if (gbm_writebuf_wcache(wcache, (const gbm_u8 *)"\"\n", 2) != 2)
                          {
                              return GBM_ERR_WRITE;
                          }
                      }
                  }
              }
              break;

          default:
              return GBM_ERR_NOT_SUPP;
      }
   }

   return GBM_ERR_OK;
}

/* ---------------------------------------- */

static GBM_ERR xpm_w_direct_color_bitmap_data(WCACHE * wcache,
                                              XPM_FMT_TYPE fmt_type,
                                              const GBM * gbm, const gbm_u8 * data,
                                              const XPM_RGB_HASH * xpm_rgb_hash)
{
   if (gbm->bpp != 24)
   {
      return GBM_ERR_NOT_SUPP;
   }
   {
      /* Calculate chars per pixel for color table encoding: */
      const size_t          stride   = ((gbm->w * gbm->bpp + 31)/32) * 4;
      const gbm_u8         *p        = data + (stride * (gbm->h - 1));
      const XPM_HASH_ENTRY *pEntry   = NULL;
      const gbm_u32         code_len = xpm_rgb_hash->code_len;
      GBMRGB_16BPP          rgb16;
      int                   x, y;

      for (y = gbm->h - 1; y >= 0; y--)
      {
          const gbm_u8 * pt = p;
          if (fmt_type != XPM_FMT_XPM2)
          {
              if (gbm_write_wcache(wcache, '\"') != 1)
              {
                  return GBM_ERR_WRITE;
              }
          }
          for (x = 0; x < gbm->w; x++)
          {
            rgb16.b = *pt++;
            rgb16.g = *pt++;
            rgb16.r = *pt++;

            pEntry = find_xpm_rgb_hash_color_code(xpm_rgb_hash, rgb16.r, rgb16.g, rgb16.b);
            if (pEntry == NULL)
            {
                return GBM_ERR_WRITE;
            }
            if (gbm_writebuf_wcache(wcache, pEntry->code, code_len) != code_len)
            {
                return GBM_ERR_WRITE;
            }
          }
          p -= stride;
          if (fmt_type == XPM_FMT_XPM2)
          {
              if (gbm_write_wcache(wcache, '\n') != 1)
              {
                  return GBM_ERR_WRITE;
              }
          }
          else
          {
              if (y > 0)
              {
                  if (gbm_writebuf_wcache(wcache, (const gbm_u8 *)"\",\n", 3) != 3)
                  {
                      return GBM_ERR_WRITE;
                  }
              }
              else
              {
                  if (gbm_writebuf_wcache(wcache, (const gbm_u8 *)"\"\n", 2) != 2)
                  {
                      return GBM_ERR_WRITE;
                  }
              }
          }
      }
   }
   return GBM_ERR_OK;
}

/* ---------------------------------------- */

GBM_ERR xpm_w(const char *fn, int fd, const GBM *gbm, const GBMRGB *gbmrgb, const gbm_u8 *data, const char *opt)
{
   const char    *s_opt;
   char           s[300+1]        = { 0 };
   int            s_len           = 0;
   int            used_colors     = 0;
   int            chars_per_pixel = 0;
   int            transcol        = -1;
   int            i;
   WCACHE        *wcache          = NULL;
   XPM_RGB_HASH  *xpm_rgb_hash    = NULL;
   GBMRGB_16BPP   transcol_rgb16  = { 0,0,0 };
   GBMRGB_16BPP  *ptranscol_rgb16 = NULL;
   GBM_ERR        rc              = GBM_ERR_OK;
   XPM_FMT_TYPE   fmt_type        = XPM_FMT_XPM3;

   /* check for requested XPM2 subformat output, default is XPM3 */
   if (gbm_find_word(opt, "xpm2" ) != NULL)
   {
       fmt_type = XPM_FMT_XPM2;
   }

   /* Can specify transparent colour index (only for palette bitmaps).
    *   Output option: transcol=# (default is none).
    */
   if ((s_opt = gbm_find_word_prefix(opt, "transcol=")) != NULL)
   {
       if (sscanf(s_opt + 9, "%d", &transcol) != 1)
       {
          return GBM_ERR_BAD_OPTION;
       }

       switch(gbm->bpp)
       {
          case 1:
          case 4:
          case 8:
             if ((transcol < 0) || (transcol >= (1 << gbm->bpp)))
             {
                return GBM_ERR_BAD_OPTION;
             }
             break;

          default:
             return GBM_ERR_BAD_OPTION;
       }
   }

   /* Can specify transparency colour (only for bitmap without alpha channel)
    *   Output option: transcol_rgb=#_#_# (red_green_blue, 0..255)
    */
   if ((s_opt = gbm_find_word_prefix(opt, "transcol_rgb=")) != NULL)
   {
       int image_transcol_red   = 0;
       int image_transcol_green = 0;
       int image_transcol_blue  = 0;
       int colorMax             = 0;

       if (sscanf(s_opt + 13, "%d_%d_%d", &image_transcol_red,
                                          &image_transcol_green,
                                          &image_transcol_blue) != 3)
       {
          return GBM_ERR_BAD_OPTION;
       }

       switch(gbm->bpp)
       {
          case 24:
             colorMax = 0xff;
             break;

          default:
             return GBM_ERR_BAD_OPTION;
       }

       if (((image_transcol_red   < 0) || (image_transcol_red   > colorMax)) ||
           ((image_transcol_green < 0) || (image_transcol_green > colorMax)) ||
           ((image_transcol_blue  < 0) || (image_transcol_blue  > colorMax)))
       {
          return GBM_ERR_BAD_OPTION;
       }
       transcol_rgb16.r = image_transcol_red;
       transcol_rgb16.g = image_transcol_green;
       transcol_rgb16.b = image_transcol_blue;
       ptranscol_rgb16  = &transcol_rgb16;
   }

   /* extract unique colors */
   switch(gbm->bpp)
   {
      case 1:
      case 4:
      case 8:
        rc = extract_unique_colors_of_table(gbm, gbmrgb, data, transcol, &xpm_rgb_hash);
        if (rc != GBM_ERR_OK)
        {
            return rc;
        }
        break;

      case 24:
        rc = extract_unique_colors_of_bitmap(gbm, data, ptranscol_rgb16, &xpm_rgb_hash);
        if (rc != GBM_ERR_OK)
        {
            return rc;
        }
        break;

      default:
        return GBM_ERR_NOT_SUPP;
   }

   chars_per_pixel = xpm_rgb_hash->code_len;
   used_colors     = xpm_rgb_hash->length;

   if (used_colors < 1)
   {
     free_xpm_rgb_hash(xpm_rgb_hash);
     return GBM_ERR_NOT_SUPP;
   }

   /* create write cache to overcome performance issues when writing lots of short buffers */
   wcache = gbm_create_wcache(fd);
   if (wcache == NULL)
   {
     free_xpm_rgb_hash(xpm_rgb_hash);
     return GBM_ERR_MEM;
   }

   /* write header */
   if (fmt_type == XPM_FMT_XPM2)
   {
     sprintf(s, "! XPM2\n%d %d %d %d\n", gbm->w, gbm->h, used_colors, chars_per_pixel);
   }
   else
   {
     /* extract base filename */
     const char *end = NULL;
     
    #if defined(_MAX_FNAME)

     char basename[_MAX_FNAME+1] = { 0 };
     char filename[_MAX_PATH+1]  = { 0 };

     strcpy(filename, fn);
     _splitpath(filename, NULL, NULL, basename, NULL);

     if (strlen(basename) < 1)
     {
       free_xpm_rgb_hash(xpm_rgb_hash);
       gbm_destroy_wcache(wcache);
       return GBM_ERR_WRITE;
     }

    #else

     char basename[512] = { 0 };

     #if defined(__OS2__) || defined(DOS) || defined(WIN32)
     const char * pbasename = rindex(fn, '\\');
     #else
     const char * pbasename = rindex(fn, '/');
     #endif
     if (pbasename == NULL)
     {
       free_xpm_rgb_hash(xpm_rgb_hash);
       gbm_destroy_wcache(wcache);
       return GBM_ERR_WRITE;
     }
     if (*pbasename != 0)
     {
       pbasename++;
     }
     strcpy(basename, pbasename);
     /* look for . as separator and skip everything behind */
     end = strstr(basename, ".");
     if (end != NULL)
     {
       basename[end - basename] = 0;
     }

    #endif

     /* look for _ as separator and skip everything behind */
     end = strstr(basename, "_");
     if (end != NULL)
     {
       basename[end - basename] = 0;
     }

     sprintf(s, "/* XPM */\nstatic char * %s[] = {\n/* width height ncolors chars_per_pixel */\n\"%d %d %d %d\",\n/* colors */\n",
             basename, gbm->w, gbm->h, used_colors, chars_per_pixel);
   }

   s_len = (int)strlen(s);
   if (gbm_writebuf_wcache(wcache, (const gbm_u8 *)s, s_len) != s_len)
   {
     free_xpm_rgb_hash(xpm_rgb_hash);
     gbm_destroy_wcache(wcache);
     return GBM_ERR_WRITE;
   }

   /* write color table */
   {
      gbm_u8 hexbuf[20+1] = { 0 };

      for (i = 0; i < used_colors; i++)
      {
         const XPM_HASH_ENTRY *pEntry = xpm_rgb_hash->entries[i];
         while (pEntry != NULL)
         {
            switch(pEntry->type)
            {
                case XPM_PAL_RGB:
                case XPM_PAL_TRANSPARENT:
                    break;

                case XPM_PAL_UNUSED:
                    continue;

                default:
                    free_xpm_rgb_hash(xpm_rgb_hash);
                    gbm_destroy_wcache(wcache);
                    return GBM_ERR_WRITE;
            }
            if (fmt_type != XPM_FMT_XPM2)
            {
               if (gbm_write_wcache(wcache, '"') != 1)
               {
                  free_xpm_rgb_hash(xpm_rgb_hash);
                  gbm_destroy_wcache(wcache);
                  return GBM_ERR_WRITE;
               }
            }
            if (gbm_writebuf_wcache(wcache, pEntry->code, chars_per_pixel) != chars_per_pixel)
            {
               free_xpm_rgb_hash(xpm_rgb_hash);
               gbm_destroy_wcache(wcache);
               return GBM_ERR_WRITE;
            }
            if (pEntry->type == XPM_PAL_TRANSPARENT)
            {
               sprintf(s, " c none%s", (fmt_type == XPM_FMT_XPM2) ? "\n" : "\",\n");
            }
            else
            {
               if (! hexFromRgb16(gbm->bpp, &(pEntry->rgb16), hexbuf, sizeof(hexbuf)-1))
               {
                  free_xpm_rgb_hash(xpm_rgb_hash);
                  gbm_destroy_wcache(wcache);
                  return GBM_ERR_WRITE;
               }
               sprintf(s, " c %s%s", hexbuf, (fmt_type == XPM_FMT_XPM2) ? "\n" : "\",\n");
            }
            s_len = (int)strlen(s);
            if (gbm_writebuf_wcache(wcache, (const gbm_u8 *)s, s_len) != s_len)
            {
               free_xpm_rgb_hash(xpm_rgb_hash);
               gbm_destroy_wcache(wcache);
               return GBM_ERR_WRITE;
            }

            pEntry = pEntry->next;
         }
      }
   }

   /* write comment for pixel section */
   if (fmt_type != XPM_FMT_XPM2)
   {
     static const char * hd  = "/* pixels */\n";
            const int    hdl = (int)strlen(hd);
     if (gbm_writebuf_wcache(wcache, (const gbm_u8 *)hd, hdl) != hdl)
     {
       free_xpm_rgb_hash(xpm_rgb_hash);
       gbm_destroy_wcache(wcache);
       return GBM_ERR_WRITE;
     }
   }

   /* write bitmap data as ASCII codes for color entry */
   switch(gbm->bpp)
   {
      case 1:
      case 4:
      case 8:
        rc = xpm_w_index_based_bitmap_data(wcache, fmt_type, gbm, gbmrgb, data, xpm_rgb_hash);
        free_xpm_rgb_hash(xpm_rgb_hash);
        if (rc != GBM_ERR_OK)
        {
          gbm_destroy_wcache(wcache);
          return rc;
        }
        break;

      case 24:
        rc = xpm_w_direct_color_bitmap_data(wcache, fmt_type, gbm, data, xpm_rgb_hash);
        free_xpm_rgb_hash(xpm_rgb_hash);
        if (rc != GBM_ERR_OK)
        {
          gbm_destroy_wcache(wcache);
          return rc;
        }
        break;

      default:
        free_xpm_rgb_hash(xpm_rgb_hash);
        gbm_destroy_wcache(wcache);
        return GBM_ERR_NOT_SUPP;
   }

   /* write struct close bracket */
   if (fmt_type != XPM_FMT_XPM2)
   {
     static const char * hd = "};\n";
            const int    hdl = (int)strlen(hd);
     if (gbm_writebuf_wcache(wcache, (const gbm_u8 *)hd, hdl) != hdl)
     {
       gbm_destroy_wcache(wcache);
       return GBM_ERR_WRITE;
     }
   }

   gbm_destroy_wcache(wcache);

   return GBM_ERR_OK;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

const char *xpm_err(GBM_ERR rc)
{
   switch ( (int) rc )
   {
      case GBM_ERR_XPM_BPP:
         return "bad bits per pixel";

      case GBM_ERR_TRANSCOL_IDX:
         return "colour index for transparency is unused";
   }
   return NULL;
}

/* ---------------------------------------- */
/* ---------------------------------------- */

/* Read 1bpp encoded data */
static GBM_ERR read_data_1bpp(AHEAD * ahead, XPM_FMT_TYPE type, const GBM *gbm, gbm_u8 *data, const XPM_CODE_HASH * xpm_code_hash)
{
   const XPM_PRIV_READ *xpm_priv = (XPM_PRIV_READ *) gbm->priv;

   const int      num_colors = xpm_priv->valid_colors;
   const size_t   stride     = ((gbm->w * gbm->bpp + 31)/32) * 4;
         gbm_u8 * buffer     = NULL;

   if ((gbm->bpp != 1) || (num_colors != 2))
   {
       return GBM_ERR_READ;
   }

   buffer = (gbm_u8 *) calloc(xpm_priv->chars_per_pixel + 1, sizeof(gbm_u8));
   if (buffer == NULL)
   {
     return GBM_ERR_MEM;
   }

   /* read bitmap ASCII data and convert to 8bpp palette binary */
   {
      int                    y, x;
      gbm_u8               * pNumFill8;
      gbm_u8               * p = data + (stride * (gbm->h - 1));
      const XPM_HASH_ENTRY * pEntry;

      for (y = gbm->h - 1; y >= 0; y--)
      {
        pNumFill8 = p;
        memset(pNumFill8, 0, stride);

        for (x = 0; x < gbm->w; x++)
        {
          if (read_data_index(ahead, xpm_priv->chars_per_pixel, buffer) != xpm_priv->chars_per_pixel)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          pEntry = find_xpm_code_hash_color_entry(xpm_code_hash, buffer, xpm_priv->chars_per_pixel);
          if (pEntry == NULL)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          if (pEntry->index)
          {
              pNumFill8[x>>3] |= ( 0x80 >> (x & 7) );
          }
        }
        p -= stride;

        if (type != XPM_FMT_XPM2)
        {
            gbm_read_ahead(ahead);
            read_discard_behind_next(ahead, '"');
        }
      }
   }
   free(buffer);
   return GBM_ERR_OK;
}

/* ---------------------------------------- */

/* Read 4bpp encoded data */
static GBM_ERR read_data_4bpp(AHEAD * ahead, XPM_FMT_TYPE type, const GBM *gbm, gbm_u8 *data, const XPM_CODE_HASH * xpm_code_hash)
{
   const XPM_PRIV_READ *xpm_priv = (XPM_PRIV_READ *) gbm->priv;

   const int      num_colors = xpm_priv->valid_colors;
   const size_t   stride     = ((gbm->w * gbm->bpp + 31)/32) * 4;
         gbm_u8 * buffer     = NULL;

   if ((gbm->bpp != 4) || (num_colors > 16))
   {
       return GBM_ERR_READ;
   }

   buffer = (gbm_u8 *) calloc(xpm_priv->chars_per_pixel + 1, sizeof(gbm_u8));
   if (buffer == NULL)
   {
     return GBM_ERR_MEM;
   }

   /* read bitmap ASCII data and convert to 8bpp palette binary */
   {
      int                    y, x;
      gbm_u8               * pNumFill8;
      gbm_u8               * p = data + (stride * (gbm->h - 1));
      const XPM_HASH_ENTRY * pEntry0, *pEntry1;

      for (y = gbm->h - 1; y >= 0; y--)
      {
        pNumFill8 = p;

        for (x = 0; x+1 < gbm->w; x += 2)
        {
          if (read_data_index(ahead, xpm_priv->chars_per_pixel, buffer) != xpm_priv->chars_per_pixel)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          pEntry0 = find_xpm_code_hash_color_entry(xpm_code_hash, buffer, xpm_priv->chars_per_pixel);
          if (pEntry0 == NULL)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          if (read_data_index(ahead, xpm_priv->chars_per_pixel, buffer) != xpm_priv->chars_per_pixel)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          pEntry1 = find_xpm_code_hash_color_entry(xpm_code_hash, buffer, xpm_priv->chars_per_pixel);
          if (pEntry1 == NULL)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          *pNumFill8++ = ( ( pEntry0->index << 4 ) | pEntry1->index );
        }
        if (gbm->w & 1)
        {
          if (read_data_index(ahead, xpm_priv->chars_per_pixel, buffer) != xpm_priv->chars_per_pixel)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          pEntry0 = find_xpm_code_hash_color_entry(xpm_code_hash, buffer, xpm_priv->chars_per_pixel);
          if (pEntry0 == NULL)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          *pNumFill8++ = ( pEntry0->index << 4 );
        }
        p -= stride;

        if (type != XPM_FMT_XPM2)
        {
            gbm_read_ahead(ahead);
            read_discard_behind_next(ahead, '"');
        }
      }
   }
   free(buffer);
   return GBM_ERR_OK;
}

/* ---------------------------------------- */

/* Read 8bpp encoded data */
static GBM_ERR read_data_8bpp(AHEAD * ahead, XPM_FMT_TYPE type, const GBM *gbm, gbm_u8 *data, const XPM_CODE_HASH * xpm_code_hash)
{
   const XPM_PRIV_READ *xpm_priv = (XPM_PRIV_READ *) gbm->priv;

   const int      num_colors = xpm_priv->valid_colors;
   const size_t   stride     = ((gbm->w * gbm->bpp + 31)/32) * 4;
         gbm_u8 * buffer     = NULL;

   if ((gbm->bpp != 8) || (num_colors > 256))
   {
       return GBM_ERR_READ;
   }

   buffer = (gbm_u8 *) calloc(xpm_priv->chars_per_pixel + 1, sizeof(gbm_u8));
   if (buffer == NULL)
   {
     return GBM_ERR_MEM;
   }

   /* read bitmap ASCII data and convert to 8bpp palette binary */
   {
      int                    y, x;
      gbm_u8               * pNumFill8;
      gbm_u8               * p = data + (stride * (gbm->h - 1));
      const XPM_HASH_ENTRY * pEntry;

      for (y = gbm->h - 1; y >= 0; y--)
      {
        pNumFill8 = p;

        for (x = 0; x < gbm->w; x++)
        {
          if (read_data_index(ahead, xpm_priv->chars_per_pixel, buffer) != xpm_priv->chars_per_pixel)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          pEntry = find_xpm_code_hash_color_entry(xpm_code_hash, buffer, xpm_priv->chars_per_pixel);
          if (pEntry == NULL)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          *pNumFill8++ = (gbm_u8) pEntry->index;
        }
        p -= stride;

        if (type != XPM_FMT_XPM2)
        {
            gbm_read_ahead(ahead);
            read_discard_behind_next(ahead, '"');
        }
      }
   }
   free(buffer);
   return GBM_ERR_OK;
}

/* ---------------------------------------- */

/* Read 24bpp encoded data */
static GBM_ERR read_data_24bpp(AHEAD * ahead, XPM_FMT_TYPE type, const GBM *gbm, gbm_u8 *data, const XPM_CODE_HASH * xpm_code_hash)
{
   const XPM_PRIV_READ *xpm_priv = (XPM_PRIV_READ *) gbm->priv;

   const size_t   stride = ((gbm->w * gbm->bpp + 31)/32) * 4;
         gbm_u8 * buffer = NULL;

   if (gbm->bpp != 24)
   {
       return GBM_ERR_READ;
   }

   buffer = (gbm_u8 *) calloc(xpm_priv->chars_per_pixel + 1, sizeof(gbm_u8));
   if (buffer == NULL)
   {
     return GBM_ERR_MEM;
   }

   /* read bitmap ASCII data and convert to 8bpp palette binary */
   {
      int                    y, x;
      gbm_u8               * pNumFill8;
      gbm_u8               * p = data + (stride * (gbm->h - 1));
      const XPM_HASH_ENTRY * pEntry;

      for (y = gbm->h - 1; y >= 0; y--)
      {
        pNumFill8 = p;

        for (x = 0; x < gbm->w; x++)
        {
          if (read_data_index(ahead, xpm_priv->chars_per_pixel, buffer) != xpm_priv->chars_per_pixel)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          pEntry = find_xpm_code_hash_color_entry(xpm_code_hash, buffer, xpm_priv->chars_per_pixel);
          if (pEntry == NULL)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          if ((xpm_priv->has_deep_color) && (pEntry->type != XPM_PAL_NAME))
          {
           #define CVT(x) (((x) * 255) / ((1L << 16) - 1))
            *pNumFill8++ = (gbm_u8) CVT(pEntry->rgb16.b);
            *pNumFill8++ = (gbm_u8) CVT(pEntry->rgb16.g);
            *pNumFill8++ = (gbm_u8) CVT(pEntry->rgb16.r);
           #undef CVT
          }
          else
          {
            *pNumFill8++ = (gbm_u8) pEntry->rgb16.b;
            *pNumFill8++ = (gbm_u8) pEntry->rgb16.g;
            *pNumFill8++ = (gbm_u8) pEntry->rgb16.r;
          }
        }
        p -= stride;

        if (type != XPM_FMT_XPM2)
        {
            gbm_read_ahead(ahead);
            read_discard_behind_next(ahead, '"');
        }
      }
   }
   free(buffer);
   return GBM_ERR_OK;
}

/* ---------------------------------------- */

/* Read 32bpp encoded data */
static GBM_ERR read_data_32bpp(AHEAD * ahead, XPM_FMT_TYPE type, const GBM *gbm, gbm_u8 *data, const XPM_CODE_HASH * xpm_code_hash)
{
   const XPM_PRIV_READ *xpm_priv = (XPM_PRIV_READ *) gbm->priv;

   const size_t   stride = ((gbm->w * gbm->bpp + 31)/32) * 4;
         gbm_u8 * buffer = NULL;

   if (gbm->bpp != 32)
   {
       return GBM_ERR_READ;
   }

   buffer = (gbm_u8 *) calloc(xpm_priv->chars_per_pixel + 1, sizeof(gbm_u8));
   if (buffer == NULL)
   {
     return GBM_ERR_MEM;
   }

   /* read bitmap ASCII data and convert to 8bpp palette binary */
   {
      int                    y, x;
      gbm_u8               * pNumFill8;
      gbm_u8               * p = data + (stride * (gbm->h - 1));
      const XPM_HASH_ENTRY * pEntry;

      for (y = gbm->h - 1; y >= 0; y--)
      {
        pNumFill8 = p;

        for (x = 0; x < gbm->w; x++)
        {
          if (read_data_index(ahead, xpm_priv->chars_per_pixel, buffer) != xpm_priv->chars_per_pixel)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          pEntry = find_xpm_code_hash_color_entry(xpm_code_hash, buffer, xpm_priv->chars_per_pixel);
          if (pEntry == NULL)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          if (xpm_priv->has_deep_color)
          {
           #define CVT(x) (((x) * 255) / ((1L << 16) - 1))
            *pNumFill8++ = (gbm_u8) CVT(pEntry->rgb16.b);
            *pNumFill8++ = (gbm_u8) CVT(pEntry->rgb16.g);
            *pNumFill8++ = (gbm_u8) CVT(pEntry->rgb16.r);
           #undef CVT
          }
          else
          {
            *pNumFill8++ = (gbm_u8) pEntry->rgb16.b;
            *pNumFill8++ = (gbm_u8) pEntry->rgb16.g;
            *pNumFill8++ = (gbm_u8) pEntry->rgb16.r;
          }
          *pNumFill8++ = (pEntry->type == XPM_PAL_TRANSPARENT) ? 255 : 0;
        }
        p -= stride;

        if (type != XPM_FMT_XPM2)
        {
            gbm_read_ahead(ahead);
            read_discard_behind_next(ahead, '"');
        }
      }
   }
   free(buffer);
   return GBM_ERR_OK;
}

/* ---------------------------------------- */

/* Read 48bpp encoded data */
static GBM_ERR read_data_48bpp(AHEAD * ahead, XPM_FMT_TYPE type, const GBM *gbm, gbm_u8 *data, const XPM_CODE_HASH * xpm_code_hash)
{
   const XPM_PRIV_READ *xpm_priv = (XPM_PRIV_READ *) gbm->priv;

   const size_t   stride = ((gbm->w * gbm->bpp + 31)/32) * 4;
         gbm_u8 * buffer = NULL;

   if (gbm->bpp != 48)
   {
       return GBM_ERR_READ;
   }

   buffer = (gbm_u8 *) calloc(xpm_priv->chars_per_pixel + 1, sizeof(gbm_u8));
   if (buffer == NULL)
   {
     return GBM_ERR_MEM;
   }

   /* read bitmap ASCII data and convert to 8bpp palette binary */
   {
      int                    y, x;
      gbm_u16              * pNumFill16;
      gbm_u8               * p = data + (stride * (gbm->h - 1));
      const XPM_HASH_ENTRY * pEntry;

      for (y = gbm->h - 1; y >= 0; y--)
      {
        pNumFill16 = (gbm_u16 *) p;

        for (x = 0; x < gbm->w; x++)
        {
          if (read_data_index(ahead, xpm_priv->chars_per_pixel, buffer) != xpm_priv->chars_per_pixel)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          pEntry = find_xpm_code_hash_color_entry(xpm_code_hash, buffer, xpm_priv->chars_per_pixel);
          if (pEntry == NULL)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          *pNumFill16++ = pEntry->rgb16.b;
          *pNumFill16++ = pEntry->rgb16.g;
          *pNumFill16++ = pEntry->rgb16.r;
        }
        p -= stride;

        if (type != XPM_FMT_XPM2)
        {
            gbm_read_ahead(ahead);
            read_discard_behind_next(ahead, '"');
        }
      }
   }
   free(buffer);
   return GBM_ERR_OK;
}

/* ---------------------------------------- */

/* Read 64bpp encoded data */
static GBM_ERR read_data_64bpp(AHEAD * ahead, XPM_FMT_TYPE type, const GBM *gbm, gbm_u8 *data, const XPM_CODE_HASH * xpm_code_hash)
{
   const XPM_PRIV_READ *xpm_priv = (XPM_PRIV_READ *) gbm->priv;

   const size_t   stride = ((gbm->w * gbm->bpp + 31)/32) * 4;
         gbm_u8 * buffer = NULL;

   if (gbm->bpp != 64)
   {
       return GBM_ERR_READ;
   }

   buffer = (gbm_u8 *) calloc(xpm_priv->chars_per_pixel + 1, sizeof(gbm_u8));
   if (buffer == NULL)
   {
     return GBM_ERR_MEM;
   }

   /* read bitmap ASCII data and convert to 8bpp palette binary */
   {
      int                    y, x;
      gbm_u16              * pNumFill16;
      gbm_u8               * p = data + (stride * (gbm->h - 1));
      const XPM_HASH_ENTRY * pEntry;

      for (y = gbm->h - 1; y >= 0; y--)
      {
        pNumFill16 = (gbm_u16 *) p;

        for (x = 0; x < gbm->w; x++)
        {
          if (read_data_index(ahead, xpm_priv->chars_per_pixel, buffer) != xpm_priv->chars_per_pixel)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          pEntry = find_xpm_code_hash_color_entry(xpm_code_hash, buffer, xpm_priv->chars_per_pixel);
          if (pEntry == NULL)
          {
            free(buffer);
            return GBM_ERR_READ;
          }
          *pNumFill16++ = pEntry->rgb16.b;
          *pNumFill16++ = pEntry->rgb16.g;
          *pNumFill16++ = pEntry->rgb16.r;
          *pNumFill16++ = (pEntry->type == XPM_PAL_TRANSPARENT) ? 65535 : 0;
        }
        p -= stride;

        if (type != XPM_FMT_XPM2)
        {
            gbm_read_ahead(ahead);
            read_discard_behind_next(ahead, '"');
        }
      }
   }
   free(buffer);
   return GBM_ERR_OK;
}

