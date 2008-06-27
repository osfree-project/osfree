/*
 * Heavely based on JPEGIOProc by Chris Wohlgemuth
 */

/************************************************************************/
/* Put all #defines here                                                */
/************************************************************************/

#define INCL_32                         /* force 32 bit compile */
#define INCL_GPIBITMAPS
#define INCL_DOSFILEMGR
#define INCL_WIN
#define INCL_GPI
#define INCL_PM

#define MEMCHECK

/************************************************************************/
/* Put all #includes here                                               */
/************************************************************************/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "GIFproc.h"
#include "gbm.h"
#include "gbmerr.h"
#include "gbmtrunc.h"
#include "gbmht.h"
#include "gbmhist.h"
#include "gbmmcut.h"
#include "gbmtool.h"

#define DEBUG

#ifdef DEBUG
void debug(const char* chrFormat, ...)
{
  char logNameLocal[CCHMAXPATH];
  FILE *fHandle;

  sprintf(logNameLocal,"%s\\GIFio.log", getenv("LOGFILES"));
  fHandle=fopen(logNameLocal,"a");
  if(fHandle) {
    va_list arg_ptr;
    void *tb;

    va_start (arg_ptr, chrFormat);
    vfprintf(fHandle, chrFormat, arg_ptr);
    va_end (arg_ptr);
    fclose(fHandle);
  }
}
#endif


static BOOLEAN same(const char *s1, const char *s2, int n)
        {
        for ( ; n--; s1++, s2++ )
                if ( tolower(*s1) != tolower(*s2) )
                        return FALSE;
        return TRUE;
        }
/*...e*/
/*...smain:0:*/
/*...smapinfos:0:*/
#define CVT_BW          0
#define CVT_VGA         1
#define CVT_8           2
#define CVT_4G          3
#define CVT_784         4
#define CVT_666         5
#define CVT_444         6
#define CVT_8G          7
#define CVT_TRIPEL      8
#define CVT_PAL1BPP     9
#define CVT_PAL4BPP     10
#define CVT_PAL8BPP     11
#define CVT_RGB         12
#define CVT_FREQ        13
#define CVT_MCUT        14
#define CVT_ERRDIFF     0x4000
#define CVT_HALFTONE    0x2000

typedef struct { char *name; int m; int dest_bpp; } MAPINFO;

static MAPINFO mapinfos[] =
        {
        "bw",           CVT_BW,         1,
        "vga",          CVT_VGA,        4,
        "8",            CVT_8,          4,
        "4g",           CVT_4G,         4,
        "7x8x4",        CVT_784,        8,
        "6x6x6",        CVT_666,        8,
        "4x4x4",        CVT_444,        8,
        "8g",           CVT_8G,         8,
        "tripel",       CVT_TRIPEL,     8,
        "pal1bpp",      CVT_PAL1BPP,    1,
        "pal4bpp",      CVT_PAL4BPP,    4,
        "pal8bpp",      CVT_PAL8BPP,    8,
        };

#define N_MAPINFOS      (sizeof(mapinfos)/sizeof(mapinfos[0]))
/*...e*/
/*...sget_masks:0:*/
/*
Returns TRUE if a set of masks given at map.
Also sets *rm, *gm, *bm from these.
Else returns FALSE.
*/

static byte mask[] = { 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };

static BOOLEAN get_masks(char *map, byte *rm, byte *gm, byte *bm)
        {
        if ( map[0] <  '0' || map[0] > '8' ||
             map[1] != ':' ||
             map[2] <  '0' || map[2] > '8' ||
             map[3] != ':' ||
             map[4] <  '0' || map[4] > '8' )
                return FALSE;

        *rm = mask[map[0] - '0'];
        *gm = mask[map[2] - '0'];
        *bm = mask[map[4] - '0'];
        return TRUE;
        }
/*...e*/
/*...sexpand_to_24bit:0:*/
static void expand_to_24bit(GBM *gbm, GBMRGB *gbmrgb, byte **data)
        {
        int stride = ((gbm->w * gbm->bpp + 31)/32) * 4;
        int new_stride = ((gbm->w * 3 + 3) & ~3);
        int bytes, y;
        byte *new_data;

        if ( gbm->bpp == 24 )
                return;

        bytes = new_stride * gbm->h;
        if ( (new_data = malloc((size_t) bytes)) == NULL )
                debug("out of memory allocating %d bytes", bytes);

        for ( y = 0; y < gbm->h; y++ )
                {
                byte    *src = *data + y * stride;
                byte    *dest = new_data + y * new_stride;
                int     x;

                switch ( gbm->bpp )
                        {
/*...s1:24:*/
case 1:
        {
        byte    c;

        for ( x = 0; x < gbm->w; x++ )
                {
                if ( (x & 7) == 0 )
                        c = *src++;
                else
                        c <<= 1;

                *dest++ = gbmrgb[c >> 7].b;
                *dest++ = gbmrgb[c >> 7].g;
                *dest++ = gbmrgb[c >> 7].r;
                }
        }
        break;
/*...e*/
/*...s4:24:*/
case 4:
        for ( x = 0; x + 1 < gbm->w; x += 2 )
                {
                byte    c = *src++;

                *dest++ = gbmrgb[c >> 4].b;
                *dest++ = gbmrgb[c >> 4].g;
                *dest++ = gbmrgb[c >> 4].r;
                *dest++ = gbmrgb[c & 15].b;
                *dest++ = gbmrgb[c & 15].g;
                *dest++ = gbmrgb[c & 15].r;
                }

        if ( x < gbm->w )
                {
                byte    c = *src;

                *dest++ = gbmrgb[c >> 4].b;
                *dest++ = gbmrgb[c >> 4].g;
                *dest++ = gbmrgb[c >> 4].r;
                }
        break;
/*...e*/
/*...s8:24:*/
case 8:
        for ( x = 0; x < gbm->w; x++ )
                {
                byte    c = *src++;

                *dest++ = gbmrgb[c].b;
                *dest++ = gbmrgb[c].g;
                *dest++ = gbmrgb[c].r;
                }
        break;
/*...e*/
                        }
                }
        free(*data);
        *data = new_data;
        gbm->bpp = 24;
        }
/*...e*/
/*...sto_grey_pal:0:*/
static void to_grey_pal(GBMRGB *gbmrgb)
        {
        int i;

        for ( i = 0; i < 0x100; i++ )
                gbmrgb[i].r =
                gbmrgb[i].g =
                gbmrgb[i].b = (byte) i;
        }
/*...e*/
/*...sto_grey:0:*/
static void to_grey(GBM *gbm, const byte *src_data, byte *dest_data)
        {
        int src_stride  = ((gbm->w * 3 + 3) & ~3);
        int dest_stride = ((gbm->w     + 3) & ~3);
        int y;

        for ( y = 0; y < gbm->h; y++ )
                {
                const byte *src  = src_data;
                      byte *dest = dest_data;
                int x;

                for ( x = 0; x < gbm->w; x++ )
                        {
                        byte b = *src++;
                        byte g = *src++;
                        byte r = *src++;

                        *dest++ = (byte) (((word) r * 77U + (word) g * 150U + (word) b * 29U) >> 8);
                        }

                src_data  += src_stride;
                dest_data += dest_stride;
                }
        gbm->bpp = 8;
        }
/*...e*/
/*...stripel_pal:0:*/
static void tripel_pal(GBMRGB *gbmrgb)
        {
        int     i;

        memset(gbmrgb, 0, 0x100 * sizeof(GBMRGB));

        for ( i = 0; i < 0x40; i++ )
                {
                gbmrgb[i       ].r = (byte) (i << 2);
                gbmrgb[i + 0x40].g = (byte) (i << 2);
                gbmrgb[i + 0x80].b = (byte) (i << 2);
                }
        }
/*...e*/
/*...stripel:0:*/
static void tripel(GBM *gbm, const byte *src_data, byte *dest_data)
        {
        int src_stride  = ((gbm->w * 3 + 3) & ~3);
        int dest_stride = ((gbm->w     + 3) & ~3);
        int y;

        for ( y = 0; y < gbm->h; y++ )
                {
                const byte *src  = src_data;
                      byte *dest = dest_data;
                int x;

                for ( x = 0; x < gbm->w; x++ )
                        {
                        byte b = *src++;
                        byte g = *src++;
                        byte r = *src++;

                        switch ( (x+y)%3 )
                                {
                                case 0: *dest++ = (byte)         (r >> 2) ;     break;
                                case 1: *dest++ = (byte) (0x40 + (g >> 2));     break;
                                case 2: *dest++ = (byte) (0x80 + (b >> 2));     break;
                                }
                        }

                src_data  += src_stride;
                dest_data += dest_stride;
                }
        gbm->bpp = 8;
        }
/*...e*/



/*...vgbm\46\h:0:*/
/*...e*/
/*...svars:0:*/
static BOOLEAN inited = FALSE;

/*
For 7Rx8Gx4B.
*/

static byte index4[0x100];
static byte index6[0x100];
static byte index7[0x100];
static byte index8[0x100];
static byte index16[0x100];
static byte scale4[] = { 0, 85, 170, 255 };
static byte scale6[] = { 0, 51, 102, 153, 204, 255 };
static byte scale7[] = { 0, 43, 85, 128, 170, 213, 255 };
static byte scale8[] = { 0, 36, 73, 109, 146, 182, 219, 255 };
static byte scale16[] = { 0, 17, 34, 51, 68, 85, 102, 119, 136,
                           153, 170, 187, 204, 221, 238, 255 };
/*...e*/
/*...sinit:0:*/
/*
This function initialises this module.
*/

/*...snearest_inx:0:*/
#ifndef abs
#define abs(x)  (((x)>=0)?(x):-(x))
#endif

static byte nearest_inx(byte value, const byte ab[], unsigned short cb)
        {
        byte b, inx, inx_min;
        short diff, diff_min;

        b = ab[0];
        diff_min = abs((short) value - (short) b);
        inx_min = 0;
        for ( inx = 1; (unsigned short) inx < cb; inx++ )
                {
                b = ab[inx];
                diff = abs((short) value - (short) b);
                if ( diff < diff_min )
                        {
                        diff_min = diff;
                        inx_min = inx;
                        }
                }
        return inx_min;
        }
/*...e*/

static void init(void)
        {
        int i;

        if ( inited )
                return;

        /* For 7 Red x 8 Green x 4 Blue palettes etc. */

        for ( i = 0; i < 0x100; i++ )
                {
                index4 [i] = nearest_inx((byte) i, scale4 , sizeof(scale4 ));
                index6 [i] = nearest_inx((byte) i, scale6 , sizeof(scale6 ));
                index7 [i] = nearest_inx((byte) i, scale7 , sizeof(scale7 ));
                index8 [i] = nearest_inx((byte) i, scale8 , sizeof(scale8 ));
                index16[i] = nearest_inx((byte) i, scale16, sizeof(scale16));
                }

        inited = TRUE;
        }
/*...e*/

/*...strunc:0:*/
static void trunc(
        const GBM *gbm, const byte *src, byte *dest,
        int dest_bpp,
        void (*trunc_line)(const byte *src, byte *dest, int cx)
        )
        {
        int stride_src = ((gbm->w * 3 + 3) & ~3);
        int stride_dest = ((gbm->w * dest_bpp + 31) / 32) * 4;
        int y;

        for ( y = 0; y < gbm->h; y++ )
                (*trunc_line)(src + y * stride_src, dest + y * stride_dest, gbm->w);
        }
/*...e*/

/*...snearest_color:0:*/
static byte nearest_color(byte r, byte g, byte b, GBMRGB *gbmrgb, int n_gbmrgb)
        {
        int i, i_min, dist_min = 0x30000;
        for ( i = 0; i < n_gbmrgb; i++ )
                {
                int dr = (int) ( (unsigned)r - (unsigned)gbmrgb[i].r );
                int dg = (int) ( (unsigned)g - (unsigned)gbmrgb[i].g );
                int db = (int) ( (unsigned)b - (unsigned)gbmrgb[i].b );
                int dist = dr*dr+dg*dg+db*db;
                if ( dist < dist_min )
                        { dist_min = dist; i_min = i; }
                }
        return (byte) i_min;
        }
/*...e*/

/*...sMAP:0:*/
/* A map is a structure used to accelerate the conversion from r,g,b tuple
   to palette index. A map divides RGB space into a cube. Each cube either
   maps all its RGB space to a single index, or 0xffff is stored. */

typedef struct
        {
        word inx[0x20][0x20][0x20]; /* 64KB */
        } MAP;

static void build_map(GBMRGB *gbmrgb, int n_gbmrgb, MAP *map)
        {
        int r, g, b;
        for ( r = 0; r < 0x100; r += 8 )
                for ( g = 0; g < 0x100; g += 8 )
                        for ( b = 0; b < 0x100; b += 8 )
                                {
                                byte i  = nearest_color((byte)  r   , (byte)  g   , (byte)  b   , gbmrgb, n_gbmrgb);
                                if ( i == nearest_color((byte)  r   , (byte)  g   , (byte) (b+7), gbmrgb, n_gbmrgb) &&
                                     i == nearest_color((byte)  r   , (byte) (g+7), (byte)  b   , gbmrgb, n_gbmrgb) &&
                                     i == nearest_color((byte)  r   , (byte) (g+7), (byte) (b+7), gbmrgb, n_gbmrgb) &&
                                     i == nearest_color((byte) (r+7), (byte)  g   , (byte)  b   , gbmrgb, n_gbmrgb) &&
                                     i == nearest_color((byte) (r+7), (byte)  g   , (byte) (b+7), gbmrgb, n_gbmrgb) &&
                                     i == nearest_color((byte) (r+7), (byte) (g+7), (byte)  b   , gbmrgb, n_gbmrgb) &&
                                     i == nearest_color((byte) (r+7), (byte) (g+7), (byte) (b+7), gbmrgb, n_gbmrgb) )
                                        map->inx[r/8][g/8][b/8] = i;
                                else
                                        map->inx[r/8][g/8][b/8] = (word) 0xffff;
                                }
        }

static byte nearest_color_via_map(byte r, byte g, byte b, MAP *map, GBMRGB *gbmrgb, int n_gbmrgb)
        {
        word i = map->inx[r/8][g/8][b/8];
        return ( i != (word) 0xffff )
                ? (byte) i
                : nearest_color(r, g, b, gbmrgb, n_gbmrgb);
        }
/*...e*/

/*...sgbm_trunc_line_24     \45\ truncate to fewer bits per pixel one line:0:*/
void gbm_trunc_line_24(const byte *src, byte *dest, int cx, byte rm, byte gm, byte bm)
        {
        int x;

        for ( x = 0; x < cx; x++ )
                {
                *dest++ = (*src++ & bm);
                *dest++ = (*src++ & gm);
                *dest++ = (*src++ & rm);
                }
        }
/*...e*/
/*...sgbm_trunc_24          \45\ truncate to fewer bits per pixel:0:*/
void gbm_trunc_24(const GBM *gbm, const byte *data24, byte *data8, byte rm, byte gm, byte bm)
        {
        int     stride = ((gbm->w * 3 + 3) & ~3);
        int     y;

        for ( y = 0; y < gbm->h; y++ )
                gbm_trunc_line_24(data24 + y * stride, data8 + y * stride, gbm->w, rm, gm, bm);
        }
/*...e*/


/*...sgbm_trunc_pal_7R8G4B  \45\ return 7Rx8Gx4B palette:0:*/
/*
This function makes the palette for the 7 red x 8 green x 4 blue palette.
224 palette entrys used. Remaining 32 left blank.
Colours calculated to match those used by 8514/A PM driver.
*/

void gbm_trunc_pal_7R8G4B(GBMRGB *gbmrgb)
        {
        byte volatile r;        /* C-Set/2 optimiser fix */
        byte volatile g;
        byte volatile b;

        init();

        memset(gbmrgb, 0x80, 0x100 * sizeof(GBMRGB));
        for ( r = 0; r < 7; r++ )
                for ( g = 0; g < 8; g++ )
                        for ( b = 0; b < 4; b++ )
                                {
                                gbmrgb->r = scale7[r];
                                gbmrgb->g = scale8[g];
                                gbmrgb->b = scale4[b];
                                gbmrgb++;
                                }
        }
/*...e*/

/*...sgbm_trunc_line_7R8G4B \45\ truncate to 7Rx8Gx4B one line:0:*/
void gbm_trunc_line_7R8G4B(const byte *src, byte *dest, int cx)
        {
        int x;

        init();

        for ( x = 0; x < cx; x++ )
                {
                byte bi = index4[*src++];
                byte gi = index8[*src++];
                byte ri = index7[*src++];

                *dest++ = (byte) (4 * (8 * ri + gi) + bi);
                }
        }
/*...e*/

/*...sgbm_trunc_7R8G4B      \45\ truncate to 7Rx8Gx4B:0:*/
void gbm_trunc_7R8G4B(const GBM *gbm, const byte *data24, byte *data8)
        {
        trunc(gbm, data24, data8, 8, gbm_trunc_line_7R8G4B);
        }
/*...e*/



ULONG readImageData( PGIFFILESTATUS pGIFInfo)
{
  GBM_ERR rcGBM;
  /********************************************************
   * Determine total bytes in image
   ********************************************************/

  pGIFInfo->ulRGBTotalBytes =  ( ((pGIFInfo->gbm.w * pGIFInfo->gbm.bpp + 31)/32) * 4 )
    * pGIFInfo->gbm.h;

  pGIFInfo->ulImgTotalBytes = pGIFInfo->ulRGBTotalBytes;

  /********************************************************
   * Get space for full image buffer.
   * This will be retained until the file is closed.
   ********************************************************/
  if (DosAllocMem ((PPVOID) &(pGIFInfo->lpRGBBuf),
                   pGIFInfo->ulRGBTotalBytes,
                   fALLOC))
    {
#ifdef DEBUG
      debug("readImagedata(): no memory for image data.\n");
#endif
      return (MMIO_ERROR);
    }

#ifdef DEBUG
      debug("readImagedata(): allocated %d bytes for image data.\n", pGIFInfo->ulRGBTotalBytes);
#endif

  if ( (rcGBM = gbm_read_data(pGIFInfo->fHandleGBM, pGIFInfo->ft,
                              &pGIFInfo->gbm, pGIFInfo->lpRGBBuf)) != GBM_ERR_OK )
    {
      DosFreeMem ((PVOID) pGIFInfo->lpRGBBuf);
#ifdef DEBUG
      debug("readImagedata(): can't read image data.\n");
#endif
      return (MMIO_ERROR);
    }

  /********************************************************
   * RGB Buffer now full, set position pointers to the
   * beginning of the buffer.
   ********************************************************/
  pGIFInfo->lImgBytePos =  0;

  return MMIO_SUCCESS;
}


/************************************************************************/
/* MMOT IOProc                                                          */
/*                                                                      */
/* ARGUMENTS:                                                           */
/*                                                                      */
/*     PSZ pmmioStr - pointer to MMIOINFO block                         */
/*     USHORT usMsg - MMIO message being sent                           */
/*     LONG lParam1 - filename or other parameter depending on message  */
/*     LONG lParam2 - used with some messages as values                 */
/*                                                                      */
/*                                                                      */
/*  RETURN:                                                             */
/*                                                                      */
/*      MMIOM_OPEN                                                      */
/*          Success           - MMIO_SUCCESS     (0)                    */
/*          Failure           - MMIO_ERROR      (-1)                    */
/*                                                                      */
/*      MMIOM_READ                                                      */
/*          Success           - Returns the number of bytes actually    */
/*                              read.  Return 0L if no more bytes can   */
/*                              be read.                                */
/*          Failure           - MMIO_ERROR      (-1)                    */
/*                                                                      */
/*      MMIOM_WRITE                                                     */
/*          Success           - Returns the number of bytes actually    */
/*                              written.                                */
/*          Failure           - MMIO_ERROR      (-1)                    */
/*                                                                      */
/*      MMIOM_SEEK                                                      */
/*          Success           - Returns the new file position           */
/*          Failure           - MMIO_ERROR      (-1)                    */
/*                                                                      */
/*      MMIOM_CLOSE                                                     */
/*          Success           - MMIO_SUCCESS     (0)                    */
/*          Failure           - MMIO_ERROR      (-1)                    */
/*          Other             - MMIO_WARNING, file was closed but the   */
/*                              IOProc expected additional data         */
/*                                                                      */
/*      MMIOM_GETFORMATNAME                                             */
/*          Success           - Returns the number of bytes read into   */
/*                              the buffer (size of format name)        */
/*          Failure           - Return 0                                */
/*                                                                      */
/*      MMIOM_GETFORMATINFO                                             */
/*          Success           - MMIO_SUCCESS     (0)                    */
/*          Failure           - MMIO_ERROR      (-1)                    */
/*                                                                      */
/*      MMIOM_QUERYHEADERLENGTH                                         */
/*          Success           - Returns the size of the header in bytes */
/*          Failure           - Return 0                                */
/*                                                                      */
/*      MMIOM_IDENTIFYFILE                                              */
/*          Success           - MMIO_SUCCESS     (0)                    */
/*          Failure           - MMIO_ERROR      (-1)                    */
/*                                                                      */
/*      MMIOM_GETHEADER                                                 */
/*          Success           - Returns number of bytes copied into     */
/*                              the header structure.                   */
/*          Failure           - Return 0                                */
/*          Other             - If length passed in was not large       */
/*                              enough to hold header then,             */
/*                              MMIOERR_INVALID_BUFFER_LENGTH is set    */
/*                              in ulErrorRet.                          */
/*                            - If header is bad,                       */
/*                              MMIOERR_INVALID_STRUCTURE is set in     */
/*                              ulErrorRet                              */
/*                                                                      */
/*      MMIOM_SETHEADER                                                 */
/*          Success           - Returns number of bytes written         */
/*          Failure           - Return 0                                */
/*          Other             - If header is bad,                       */
/*                              MMIOERR_INVALID_STRUCTURE is set in     */
/*                              ulErrorRet                              */
/*                                                                      */
/*  DESCRIPTION:                                                        */
/*                                                                      */
/*      This routine will translate IBM M-Motion YUV Video data into    */
/*      OS/2 2.0 memory bitmap data and back again.  The IOProc is part */
/*      of OS/2 MultiMedia Extentions File Format Conversion Utility.   */
/*                                                                      */
/*  GLOBAL VARS REFERENCED:                                             */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  GLOBAL VARS MODIFIED:                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  NOTES:                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  SIDE EFFECTS:                                                       */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/************************************************************************/

LONG EXPENTRY IOProc_Entry (PVOID  pmmioStr,
                             USHORT usMsg,
                             LONG   lParam1,
                             LONG   lParam2)

    {
    PMMIOINFO   pmmioinfo;                      /* MMIOINFO block */

    pmmioinfo = (PMMIOINFO) pmmioStr;

#ifdef DEBUG
    debug("MSG: %d %x\n", usMsg,usMsg);
#endif

    switch (usMsg)
        {
        /*#############################################################*
         * When Closing the file, perform the following:
         * 1) Setup Variables
         * 2) Process the Image buffer
         * 3) Compress the Image to appropriate format
         *#############################################################*/
        case MMIOM_CLOSE:
            {
            /************************************************************
             * Declare local variables.
             ************************************************************/
            PGIFFILESTATUS   pGIFInfo;         /* MMotionIOProc instance data */

            LONG            lRetCode;
            USHORT          rc;
            GBMRGB  gbmrgb[0x100];

#ifdef DEBUG
            debug("MMIO_CLOSE\n");
#endif

            /***********************************************************
             * Check for valid MMIOINFO block.
             ***********************************************************/
            if (!pmmioinfo)
                return (MMIO_ERROR);

            /***********************************************************
             * Set up our working file status variable.
             ***********************************************************/
            pGIFInfo = (PGIFFILESTATUS)pmmioinfo->pExtraInfoStruct;

            /***********************************************************
             * Assume success for the moment....
             ***********************************************************/
            lRetCode = MMIO_SUCCESS;


            /************************************************************
             * see if we are in Write mode and have a buffer to write out.
             *    We have no image buffer in UNTRANSLATED mode.
             ************************************************************/
            if ((pmmioinfo->ulFlags & MMIO_WRITE) && (pGIFInfo->lpRGBBuf))
                {
                  int n_ft, ft;
                  BOOL bValidGIF;

                  /* Write the buffer to disk */
#ifdef DEBUG
                  debug("MMIO_CLOSE: pGIFInfo->gbm.w: %d, pGIFInfo->gbm.h: %d, pGIFInfo->gbm.bpp; %d\n",
                           pGIFInfo->gbm.w, pGIFInfo->gbm.h, pGIFInfo->gbm.bpp);
                  debug("MMIO_CLOSE: pGIFInfo->fHandleGBM: %d, pGIFInfo->ft: %d\n",
                           pGIFInfo->fHandleGBM, pGIFInfo->ft);

#endif
                  gbm_query_n_filetypes(&n_ft);

                  for ( ft = 0; ft < n_ft; ft++ )
                    {
                      GBMFT gbmft;
                      gbm_query_filetype(ft, &gbmft);
                      if(!stricmp(gbmft.short_name, "GIF")) {
                        bValidGIF=TRUE;
                        break;
                      }
                    }

                  if(bValidGIF) {
                    /* Проверяем, поддерживается ли такая глубина цвета? */
                    switch (pGIFInfo->gbm.bpp)
                    {
                      case 24: {
                  debug("MMIO_CLOSE: Truncating colors\n");
                           gbm_trunc_pal_7R8G4B(gbmrgb);
                           gbm_trunc_7R8G4B(&pGIFInfo->gbm, pGIFInfo->lpRGBBuf, pGIFInfo->lpRGBBuf);
                           pGIFInfo->gbm.bpp=8;
                  debug("MMIO_CLOSE: pGIFInfo->gbm.w: %d, pGIFInfo->gbm.h: %d, pGIFInfo->gbm.bpp; %d\n",
                           pGIFInfo->gbm.w, pGIFInfo->gbm.h, pGIFInfo->gbm.bpp);
                  debug("MMIO_CLOSE: pGIFInfo->fHandleGBM: %d, pGIFInfo->ft: %d\n",
                           pGIFInfo->fHandleGBM, pGIFInfo->ft);
                         break;
                         }
                      case 1:
                      case 4:
                      case 8:
                      default: break;
                    };

                    if ( (rc = gbm_write("", pGIFInfo->fHandleGBM, ft, &pGIFInfo->gbm,
                                         gbmrgb, pGIFInfo->lpRGBBuf, "")) != GBM_ERR_OK )
                      {
#ifdef DEBUG
                        debug("MMIO_CLOSE: can't write image data.\n");
#endif
                      }
                  }

                }  /* end IF WRITE & IMAGE BUFFER block */

            /***********************************************************
             * Free the RGB buffer, if it exists, that was created
             * for the translated READ operations.
             ***********************************************************/
            if (pGIFInfo->lpRGBBuf)
              {
                DosFreeMem ((PVOID) pGIFInfo->lpRGBBuf);
              }
            /***********************************************************
             * Close the file
             ***********************************************************/
            gbm_io_close(pGIFInfo->fHandleGBM);

            DosFreeMem ((PVOID) pGIFInfo);

            return (lRetCode);
            }  /* end case of MMIOM_CLOSE */

        /*#############################################################*
         * Get the NLS format Information.
         *#############################################################*/
        case MMIOM_GETFORMATINFO:
            {
            /***********************************************************
             * Declare local variables.
             ***********************************************************/
            PMMFORMATINFO       pmmformatinfo;

#ifdef DEBUG
            debug("MMIO_GETFORMATINFO\n");
#endif

            /************************************************************
             * Set pointer to MMFORMATINFO structure.
             ************************************************************/
            pmmformatinfo = (PMMFORMATINFO) lParam1;

            /************************************************************
             * Fill in the values for the MMFORMATINFO structure.
             ************************************************************/
            pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);
            pmmformatinfo->fccIOProc    = FOURCC_GIF;
            pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
            pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;

            pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED |
              MMIO_CANSEEKTRANSLATED        |
              MMIO_CANWRITETRANSLATED
              /* MMIO_CANREADUNTRANSLATED      |
                 MMIO_CANWRITETRANSLATED       |
                 MMIO_CANWRITEUNTRANSLATED     |
                 MMIO_CANREADWRITEUNTRANSLATED |

                 MMIO_CANSEEKUNTRANSLATED */;

            strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, pszGIFExt);
  pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;
  pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;
//            if (GetNLSData( &pmmformatinfo->ulCodePage,
  //                        &pmmformatinfo->ulLanguage ))
//               {
//               return( -1L );
//               }
  pmmformatinfo->lNameLength=9;

//            if (GetFormatStringLength( FOURCC_GIF,
  //                                   &(pmmformatinfo->lNameLength) ))
//               {
//               return( -1L );
//               }


            /************************************************************
             * Return success back to the application.
             ************************************************************/
            return (MMIO_SUCCESS);
            } /* end case of MMIOM_GETFORMATINFO */

        /*#############################################################*
         * Get the NLS format name.
         *#############################################################*/
        case MMIOM_GETFORMATNAME:
            {
            LONG lBytesCopied;

#ifdef DEBUG
            debug("MMIO_GETFORMATNAME\n");
#endif

            /************************************************************
             * Copy the M-Motion format string into buffer supplied by
             * lParam1.  Only put in the amount of my string up to the
             * allocated amount which is in lParam2.  Leave enough room
             * for the NULL termination.
             ************************************************************/
//            lBytesCopied = GetFormatString( FOURCC_GIF,
//                                            (char *)lParam1,
//                                            lParam2 );

//            return (lBytesCopied);
  memcpy((PSZ)lParam1, "GIF Image", 9);
  return(11);//GetFormatString(FOURCC_PNG, filename, lParam2));

            } /* end case of MMIOM_GETFORMATNAME */
        /*#############################################################*
         * Get the file header.
         *#############################################################*/
        case MMIOM_GETHEADER:
            {
            /************************************************************
             * Declare local variables.
             ************************************************************/
              //  PMMFILESTATUS       pVidInfo;
            PGIFFILESTATUS       pGIFInfo;
#ifdef DEBUG
            debug("MMIO_GETHEADER\n");
#endif

            /************************************************************
             * Check for valid MMIOINFO block.
             ************************************************************/
            if (!pmmioinfo)
                return (0);

            /************************************************************
             * Set up our working file status variable.
             ************************************************************/
            pGIFInfo = (PGIFFILESTATUS)pmmioinfo->pExtraInfoStruct;

            /**************************************************
             * Getheader only valid in READ or READ/WRITE mode.
             * There is no header to get in WRITE mode.  We
             * must also have a valid file handle to read from
             **************************************************/
            if ((pmmioinfo->ulFlags & MMIO_WRITE) ||
                (!(pGIFInfo->fHandleGBM)))
               return (0);

            /************************************************************
             * Check for Translation mode.
             ************************************************************/
            if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
              {
                /********************************************************
                 * Translation is off.
                 ********************************************************/
                /* Unstranslatd headers are not supported !!! */
                return 0;
              }   /* end IF NOT TRANSLATED block */

            /******************
             * TRANSLATION IS ON
             ******************/
            if (lParam2 < sizeof (MMIMAGEHEADER))
              {
                pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
                return (0);
                }

            if (!lParam1)
                {
                pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
                return (0);
                }

            memcpy ((PVOID)lParam1,
                    (PVOID)&pGIFInfo->mmImgHdr,
                    sizeof (MMIMAGEHEADER));

            return (sizeof (MMIMAGEHEADER));
            } /* end case of MMIOM_GETHEADER */

        /*#############################################################*
         * Identify whether this file can be processed.
         *#############################################################*/
        case MMIOM_IDENTIFYFILE:
            {

            /************************************************************
             * Declare local variables.
             ************************************************************/
            GBMFT gbmft;
            int ft, fd, n_ft;
            BOOL bValidGIF= FALSE;

#ifdef DEBUG
            debug("MMIO_IDENTIFYFILE\n");
#endif

            /************************************************************
             * We need either a file name (lParam1) or file handle (lParam2)
             ************************************************************/
            if (!lParam1 && !lParam2)
                return (MMIO_ERROR);

            if ( (fd = gbm_io_open((PSZ) lParam1, GBM_O_RDONLY|GBM_O_BINARY)) == -1 )
              {
#ifdef DEBUG
            debug("MMIO_IDENTIFYFILE: error opening file %s\n", (PSZ)lParam1);
#endif
                return (MMIO_ERROR);
              }

            if ( gbm_init() != GBM_ERR_OK )
              return MMIO_ERROR;

            gbm_query_n_filetypes(&n_ft);

            for ( ft = 0; ft < n_ft; ft++ )
              {
                GBM gbm;

                if ( gbm_read_header((PSZ) lParam1, fd, ft, &gbm, "") == GBM_ERR_OK )
                  {
                    gbm_query_filetype(ft, &gbmft);
#ifdef DEBUG
            debug("%s\n",gbmft.short_name);
#endif
                    if(!stricmp(gbmft.short_name, "GIF"))
                      bValidGIF=TRUE;
                  }
              }

            gbm_io_close(fd);
            gbm_deinit();

            if(bValidGIF) {
#ifdef DEBUG
              debug("File seems to be a GIF\n");
#endif
              return (MMIO_SUCCESS);
            }

            return (MMIO_ERROR);

            } /* end case of MMIOM_IDENTIFYFILE */

        /*#############################################################*/
        /*#############################################################*/
        case MMIOM_OPEN:
            {

            /************************************************************
             * Declare local variables
             ************************************************************/


            PGIFFILESTATUS   pGIFInfo;   /* pointer to a GIF file       */
                                        /* status structure that we will*/
                                        /* use for this file instance   */

            MMIMAGEHEADER   MMImgHdr;
            ULONG           ulWidth;
            ULONG           ulHeight;
            PBYTE           lpRGBBufPtr;

            PSZ pszFileName = (CHAR *)lParam1;  /* get the filename from    */
                                                /* parameter                */

            GBMFT gbmft;
            int ft, fd, n_ft, stride, bytes;
            BOOL bValidGIF= FALSE;
            int fOpenFlags;
            GBM_ERR     rc;

#ifdef DEBUG
            debug("MMIO_OPEN\n");
#endif
            /************************************************************
             * Check for valid MMIOINFO block.
             ************************************************************/
            if (!pmmioinfo)
                return (MMIO_ERROR);

            /************************************************************
             * If flags show read and write then send back an error.  We
             * only support reading or writing but not both at the same
             * time on the same file.
             ************************************************************/
            if ((pmmioinfo->ulFlags & MMIO_READWRITE) &&
                ((pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
                 (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)))
                {
#ifdef DEBUG
                debug("MMIO_OPEN, read/write not supported.\n");
#endif
                return (MMIO_ERROR);
                }

            /* We can't read/write untranslated */
            if ( ( !(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA) ||
                   !(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER) ) )
              {
#ifdef DEBUG
                debug("MMIO_OPEN, untranslated not supported.\n");
#endif
                return (MMIO_ERROR);
              }


            /*!!!!!!!!!!!!!!!!!!!!!! FIXME !!!!!!!!!!!!!!!!!!!!!!!*/
            /* To be honest we can almost nothing ;-) */
            if (pmmioinfo->ulFlags &
                (MMIO_APPEND|
                 MMIO_ALLOCBUF|MMIO_BUFSHARED | MMIO_VERTBAR ))
              {
#ifdef DEBUG
                debug("MMIO_OPEN, unsupported flag in %x\n", pmmioinfo->ulFlags);
#endif
//                return (MMIO_ERROR);
              }

            /* Caller requested a delete */
            if(pmmioinfo->ulFlags & MMIO_DELETE) {
#ifdef DEBUG
            debug("MMIO_OPEN: MMIO_DELETE set\n");
#endif
              if(remove((PSZ) lParam1)==-1)
                return (MMIO_ERROR);
            }
            /************************************************************
              Allocate our private data structure
             ************************************************************/
            if(NO_ERROR!=DosAllocMem ((PPVOID) &pGIFInfo,
                                  sizeof (GIFFILESTATUS),
                                  fALLOC))
              return  MMIO_ERROR;

            memset((PVOID)pGIFInfo,0, sizeof(GIFFILESTATUS));


            if ( gbm_init() != GBM_ERR_OK ) {
              DosFreeMem(pGIFInfo);
              return MMIO_ERROR;
            }

            /************************************************************
             * Store pointer to our GIFFILESTATUS structure in
             * pExtraInfoStruct field that is provided for our use.
             ************************************************************/
            pmmioinfo->pExtraInfoStruct = (PVOID)pGIFInfo;

            /************************************************************
              MMIO_WRITE
             ************************************************************/
            if (pmmioinfo->ulFlags & MMIO_WRITE) {
              /* It's a write so open the file */
#ifdef DEBUG
            debug("MMIO_OPEN: flag MMIO_WRITE set\n");
#endif

              fOpenFlags=O_WRONLY|O_BINARY;
              if(pmmioinfo->ulFlags & MMIO_CREATE) {
                fOpenFlags|=O_CREAT|O_TRUNC;
                if ( (fd = gbm_io_create((PSZ) lParam1, O_WRONLY|O_BINARY)) == -1 )
                  {
#ifdef DEBUG
                    debug("MMIO_OPEN: create failed, flags: %x, file %s\n",
                             fOpenFlags, (PSZ) lParam1);
                    debug("MMIO_OPEN: errno: 0x%x, \n",
                             errno);
#endif
                    gbm_deinit();
                    DosFreeMem(pGIFInfo);
                    return (MMIO_ERROR);
                  }
              }
              else {
                if ( (fd = gbm_io_open((PSZ) lParam1, /*fOpenFlags*/ GBM_O_BINARY|GBM_O_WRONLY)) == -1 )
                  {
#ifdef DEBUG
                    debug("MMIO_OPEN: \n");
                    debug("MMIO_OPEN: open failed, flags: %x, file %s\n",
                             fOpenFlags, (PSZ) lParam1);
                    debug("MMIO_OPEN: errno: 0x%x, \n",
                             errno);
#endif
                    gbm_deinit();
                    DosFreeMem(pGIFInfo);
                    return (MMIO_ERROR);
                  }
              }/* else */

              pGIFInfo->fHandleGBM=fd;
              return (MMIO_SUCCESS);
            }

            /* Since we can't write and we are here, this is a read. The read flag isn't
               necessarily set...
               */

            /*
              First get some infos from GBM.
              */
            if(pmmioinfo->ulFlags & MMIO_WRITE)
              fOpenFlags=GBM_O_WRONLY;
            else if(pmmioinfo->ulFlags & MMIO_READWRITE)
              fOpenFlags=GBM_O_RDWR;
            else
              fOpenFlags=GBM_O_RDONLY;

            fOpenFlags|=GBM_O_BINARY;

            if ( (fd = gbm_io_open((PSZ) lParam1, fOpenFlags)) == -1 )
              {
                gbm_deinit();
                DosFreeMem(pGIFInfo);
                return (MMIO_ERROR);
              }

            gbm_query_n_filetypes(&n_ft);

            for ( ft = 0; ft < n_ft; ft++ )
              {
                if ( gbm_read_header((PSZ) lParam1, fd, ft, &pGIFInfo->gbm, "") == GBM_ERR_OK )
                  {
                    gbm_query_filetype(ft, &gbmft);
                    if(!stricmp(gbmft.short_name, "GIF")) {
                      bValidGIF=TRUE;
                      pGIFInfo->ft=ft;
#ifdef DEBUG
              debug("MMIO_OPEN, GIF proc found ft: %d\n", ft);
#endif
                      break;
                    }
                  }
              }

            if(!bValidGIF) {
#ifdef DEBUG
              debug("MMIO_OPEN, file isn't a valid GIF file\n");
#endif
              DosFreeMem(pGIFInfo);
              gbm_io_close(fd);
              gbm_deinit();
              return (MMIO_ERROR);
            }

            pGIFInfo->fHandleGBM=fd;

            if(pGIFInfo->gbm.bpp==8) {
              GBMRGB gbmrgb[0x100];
              RGB2 *rgb2;
              int a;

#ifdef DEBUG
              debug("MMIO_OPEN, reading palette for 8bpp GIF.\n");
#endif

              /* Get palette for 8Bit GIFs (grey) */
              if ( (rc = gbm_read_palette(fd, pGIFInfo->ft, &pGIFInfo->gbm, gbmrgb)) != GBM_ERR_OK )
                {
#ifdef DEBUG
              debug("MMIO_OPEN, can't get palette for 8bpp GIF.\n");
#endif
                  DosFreeMem(pGIFInfo);
                  gbm_io_close(fd);
                  gbm_deinit();
                  return (MMIO_ERROR);
                }
              rgb2=MMImgHdr.bmiColors;
              for(a=0;a<256;a++){
                rgb2[a].bBlue=gbmrgb[a].b;
                rgb2[a].bGreen=gbmrgb[a].g;
                rgb2[a].bRed=gbmrgb[a].r;
              }


            }
            /************************************************************
             * If the app intends to read in translation mode, we must
             * allocate and set-up the buffer that will contain the RGB data
             *
             * We must also read in the data to insure that the first
             * read, seek, or get-header operation will have data
             * to use.  This is ONLY NECESSARY FOR TRANSLATED MODE
             * operations, since we must process reads/writes pretending
             * the image is stored from the Bottom-up.
             *
             ************************************************************
             ************************************************************
             * Fill out the MMIMAGEHEADER structure.
             ************************************************************/
#if 0
               MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
               MMImgHdr.ulContentType  = MMIO_IMAGE_PHOTO;
               MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                           sizeof (BITMAPINFOHEADER2);
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGIFInfo->gbm.w;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGIFInfo->gbm.h;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGIFInfo->gbm.bpp;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   =
                   BCA_UNCOMP;
               if(pGIFInfo->gbm.bpp==8) {
                   MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                     ulWidth * ulHeight;
                   MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
                   MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
               }
               else
                 MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight * 3;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     =
                   BRA_BOTTOMUP;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     =
                   BRH_NOTHALFTONED;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;
#endif

               MMImgHdr.ulHeaderLength = sizeof (MMIMAGEHEADER);
               MMImgHdr.ulContentType  = 0;//MMIO_IMAGE_PHOTO;
               MMImgHdr.ulMediaType    = MMIO_MEDIATYPE_IMAGE;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbFix =
                           sizeof (BITMAPINFOHEADER2);
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cx              = pGIFInfo->gbm.w;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cy              = pGIFInfo->gbm.h;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cPlanes         = 1;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount       = pGIFInfo->gbm.bpp;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulCompression   =
                   BCA_UNCOMP;
               if(pGIFInfo->gbm.bpp==8) {
                   MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                     ulWidth * ulHeight;
                   MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 256L;
                   MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant        = 256L;
               }
               else
                 MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage         =
                   ulWidth * ulHeight * 3;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cxResolution    = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cyResolution    = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrUsed        = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cclrImportant   = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usUnits         = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usReserved      = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRecording     =
                   BRA_BOTTOMUP;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.usRendering     =
                   BRH_NOTHALFTONED;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize1          = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cSize2          = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulColorEncoding = 0L;
               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.ulIdentifier    = 0L;

#ifdef DEBUG
               debug("MMIO_OPEN, procname is: %s\n", gbmft.short_name);
               debug("MMIO_OPEN: pGIFInfo->gbm.w: %d, pGIFInfo->gbm.h: %d, pGIFInfo->gbm.bpp; %d\n",
                        pGIFInfo->gbm.w, pGIFInfo->gbm.h, pGIFInfo->gbm.bpp);
               debug("MMIO_OPEN: pGIFInfo->fHandleGBM: %d, pGIFInfo->ft: %d\n",
                        pGIFInfo->fHandleGBM, pGIFInfo->ft);
#endif
               /********************************************************
                * Determine total bytes in image
                ********************************************************/

               stride = ( ((pGIFInfo->gbm.w * pGIFInfo->gbm.bpp + 31)/32) * 4 );
               bytes = stride * pGIFInfo->gbm.h;

               pGIFInfo->ulRGBTotalBytes = bytes;
               pGIFInfo->ulImgTotalBytes = pGIFInfo->ulRGBTotalBytes;

               MMImgHdr.mmXDIBHeader.BMPInfoHeader2.cbImage=pGIFInfo->ulRGBTotalBytes;
               //               MMImgHdr.mmXDIBHeader.XDIBHeaderPrefix.ulMemSize=pGIFInfo->ulRGBTotalBytes;
               //               MMImgHdr.mmXDIBHeader.XDIBHeaderPrefix.ulPelFormat=mmioFOURCC('p','a','l','b');

               /************************************************************
                * Copy the image header into private area for later use.
                * This will be returned on a mmioGetHeader () call
                ************************************************************/
               pGIFInfo->mmImgHdr = MMImgHdr;

               return MMIO_SUCCESS;

            } /* end case of MMIOM_OPEN */

        /*#############################################################*/
        /*#############################################################*/
        case MMIOM_QUERYHEADERLENGTH:
            {
#ifdef DEBUG
            debug("MMIO_QUERYHEADERLENGTH\n");
#endif
            /************************************************************
             * If there is no MMIOINFO block then return an error.
             ************************************************************/
            if (!pmmioinfo)
                return (0);

            /************************************************************
             * If header is in translated mode then return the media
             * type specific structure size.
             ************************************************************/
            if (pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER)
                return (sizeof (MMIMAGEHEADER));
            else
              /********************************************************
               * Header is not in translated mode so return the size
               * of the MMotion header.
               ********************************************************/
              /* Unstranslated headers not supported */
              /*!!!!!!!!!!!!!! FIXME !!!!!!!!!!!!!!!*/
              return 0;
            } /* end case of MMIOM_QUERYHEADERLENGTH */

        /*#############################################################*/
        /*#############################################################*/
        case MMIOM_READ:
            {

            /************************************************************
             * Declare Local Variables
             ************************************************************/
            PGIFFILESTATUS   pGIFInfo;
            LONG            rc;
            LONG            lBytesToRead;
            GBM_ERR     rcGBM;

#ifdef DEBUG
            debug("MMIO_READ\n");
#endif

            /************************************************************
             * Check for valid MMIOINFO block.
             ************************************************************/
            if (!pmmioinfo)
                return (MMIO_ERROR);

            /************************************************************
             * Set up our working file status variable.
             ************************************************************/
            pGIFInfo = (PGIFFILESTATUS)pmmioinfo->pExtraInfoStruct;

            /************************************************************
             * Is Translate Data off?
             ************************************************************/
            if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
              {
                int rc;
                /********************************************************
                 * Since no translation, provide exact number of bytes req
                 ********************************************************/
                if (!lParam1)
                  return (MMIO_ERROR);

                /* I don't know the header size of GIF files so I can't skip
                   to the image data in MMIO_SEEK. If I can't find the image data
                   I can't read untranslated... */
                return (MMIO_ERROR);
#if 0
                rc=gbm_io_read(pGIFInfo->fHandleGBM,(PVOID) lParam1, (ULONG) lParam2);

                if(rc==-1)
                  return (MMIO_ERROR);

                return (rc);
#endif
              }

            /************************************************************
             * Otherwise, Translate Data is on...
             ************************************************************/

            /* Check if we already have read in the image data */
            if(!pGIFInfo->lpRGBBuf) {
              /* No, so read the image now */

#ifdef DEBUG
              debug("MMIO_READ: reading image data.\n");
#endif
              if(readImageData(pGIFInfo)==MMIO_ERROR)
                return MMIO_ERROR;
            }


            /************************************************************
             * Ensure we do NOT write more data out than is remaining
             *    in the buffer.  The length of read was requested in
             *    image bytes, so confirm that there are that many of
             *    virtual bytes remaining.
             ************************************************************/
            if ((ULONG)(pGIFInfo->lImgBytePos + lParam2) >
                pGIFInfo->ulImgTotalBytes)
              /*  pVidInfo->ulImgTotalBytes)*/
               lBytesToRead =
                   pGIFInfo->ulImgTotalBytes - pGIFInfo->lImgBytePos;
            /*    pVidInfo->ulImgTotalBytes - pGIFInfo->lImgBytePos;*/
            else
               lBytesToRead = (ULONG)lParam2;

            /************************************************************
             * Perform this block on ALL reads.  The image data should
             * be in the RGB buffer at this point, and can be handed
             * to the application.
             *
             * Conveniently, the virtual image position is the same
             *    as the RGB buffer position, since both are 24 bit-RGB
             ************************************************************/
            memcpy ((PVOID)lParam1,
                    &(pGIFInfo->lpRGBBuf[pGIFInfo->lImgBytePos]),
                    lBytesToRead);

            /************************************************************
             * Move RGB Buffer pointer forward by number of bytes read.
             * The Img buffer pos is identical, since both are 24 bits
             ************************************************************/
            pGIFInfo->lImgBytePos += lBytesToRead;

#ifdef DEBUG
            debug("MMIO_READ: read %d bytes.\n", lBytesToRead);
#endif

            return (lBytesToRead);
            }   /* end case  of MMIOM_READ */

        /*#############################################################*/
        /*#############################################################*/
        case MMIOM_SEEK:
            {

            /************************************************************
             * Set up locals.
             ************************************************************/
            PGIFFILESTATUS   pGIFInfo;
            LONG            lNewFilePosition;
            LONG            lPosDesired;
            SHORT           sSeekMode;

#ifdef DEBUG
            debug("MMIO_SEEK\n");
#endif

            /************************************************************
             * Check to make sure MMIOINFO block is valid.
             ************************************************************/
            if (!pmmioinfo)
                return (MMIO_ERROR);


            /************************************************************
             * Set up our working file status variable.
             ************************************************************/
            pGIFInfo = (PGIFFILESTATUS)pmmioinfo->pExtraInfoStruct;

            lPosDesired = lParam1;
            sSeekMode = (SHORT)lParam2;

            /************************************************************
             * Is Translate Data on?
             ************************************************************/
            if (pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA)
                {
                  /* Image data already read?? */
                  if(!pGIFInfo->lpRGBBuf) {
                    /* No, so read the image now */

#ifdef DEBUG
                    debug("MMIO_SEEK: reading image data.\n");
#endif

                    if(readImageData(pGIFInfo)==MMIO_ERROR)
                      return MMIO_ERROR;
                  }

                  /********************************************************
                   * Attempt to move the Image buffer pointer to the
                   *    desired location.  App sends SEEK requests in
                   *    positions relative to the image planes & bits/pel
                   * We must also convert this to RGB positions
                   ********************************************************/
                  switch (sSeekMode)
                    {
                    case SEEK_SET:
                      {
                        lNewFilePosition = lPosDesired;
                        break;
                      }
                    case SEEK_CUR:
                      {
                        lNewFilePosition = pGIFInfo->lImgBytePos + lPosDesired;
                        break;
                      }
                    case SEEK_END:
                      {

                        lNewFilePosition =
                          pGIFInfo->ulImgTotalBytes += lPosDesired;
                        break;
                      }
                    default :
                      return (MMIO_ERROR);
                    }

                  /********************************************************
                   * Make sure seek did not go before start of file.
                   * If so, then don't change anything, just return an error
                   ********************************************************/
                  if (lNewFilePosition < 0)
                    {
                      return (MMIO_ERROR);
                    }

                  /********************************************************
                   * Make sure seek did not go past the end of file.
                   ********************************************************/
                  if (lNewFilePosition > (LONG)pGIFInfo->ulImgTotalBytes)
                    lNewFilePosition = pGIFInfo->ulImgTotalBytes;

                  pGIFInfo->lImgBytePos = lNewFilePosition;

                  return (pGIFInfo->lImgBytePos);
                }

            return (MMIO_ERROR);
            }  /* end case of MMIOM_SEEK */

        /*#############################################################*/
        /*#############################################################*/
        case MMIOM_SETHEADER:
            {
              /************************************************************
               * Declare local variables.
               ************************************************************/
              PMMIMAGEHEADER          pMMImgHdr;
              PGIFFILESTATUS           pGIFInfo;
              USHORT                  usNumColors;
              ULONG                   ulImgBitsPerLine;
              ULONG                   ulImgBytesPerLine;
              ULONG                   ulBytesWritten;
              ULONG                   ulWidth;
              ULONG                   ul4PelWidth;
              ULONG                   ulHeight;
              USHORT                  usPlanes;
              USHORT                  usBitCount;
              USHORT                  usPadBytes;


#ifdef DEBUG
            debug("MMIO_SETHEADER\n");
#endif

            /************************************************************
             * Check for valid MMIOINFO block.
             ************************************************************/
            if (!pmmioinfo)
                return (MMIO_ERROR);

            /************************************************************
             * Set up our working variable MMFILESTATUS.
             ************************************************************/
            pGIFInfo = (PGIFFILESTATUS) pmmioinfo->pExtraInfoStruct;


            /************************************************************
             * Only allow this function if we are in WRITE mode
             * And only if we have not already set the header
             ************************************************************/
            if ((!(pmmioinfo->ulFlags & MMIO_WRITE)) ||
                (pGIFInfo->bSetHeader))
              return (0);

            /********************************************************
             * Make sure lParam1 is a valid pointer
             ********************************************************/
            if (!lParam1)
              {
                pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
                return (0);
              }

            /************************************************************
             * Header is not in translated mode.
             ************************************************************/
            if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEHEADER))
                {

                  /* We don't do untranslated writes */
                  return (0);   /* 0 indicates error */

                }  /* end IF NOT TRANSLATED block */

            /************************************************************
             * Header is translated.
             ************************************************************/

            /************************************************************
             * Create local pointer media specific structure.
             ************************************************************/
            pMMImgHdr = (PMMIMAGEHEADER) lParam1;

            /************************************************************
             * Check for validity of header contents supplied
             ************************************************************
             *  --  Length must be that of the standard header
             *  --  NO Compression
             *      1 plane
             *      24, 8, 4 or 1 bpp
             ************************************************************/
            usBitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
            if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression !=
                 BCA_UNCOMP) ||
                (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
                (! ((usBitCount == 24) || (usBitCount == 8) ||
                    (usBitCount == 4) || (usBitCount == 1)))
                )
              {
                pmmioinfo->ulErrorRet = MMIOERR_INVALID_STRUCTURE;
                return (0);
              }

            if (lParam2 != sizeof (MMIMAGEHEADER))
              {
                pmmioinfo->ulErrorRet = MMIOERR_INVALID_BUFFER_LENGTH;
                return (0);
              }

#if 0
            /************************************************************
             * Complete MMOTIONHEADER.
             ************************************************************/
            memcpy ((PVOID)&pVidInfo->mmotHeader.mmID, "YUV12C", 6);
            pVidInfo->mmotHeader.mmXorg  = 0;
            pVidInfo->mmotHeader.mmYorg  = 0;
#endif


            ulWidth  = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
            ulHeight = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;

            /************************************************************
             * Complete GBM structure.
             ************************************************************/
            pGIFInfo->gbm.w=ulWidth;
            pGIFInfo->gbm.h=ulHeight;
            pGIFInfo->gbm.bpp=usBitCount;

            /* Discard any image buffer already allocated. But..., where should it
               come from?!?? */
            if(pGIFInfo->lpRGBBuf) {
              DosFreeMem(pGIFInfo->lpRGBBuf);
              pGIFInfo->lpRGBBuf=NULLHANDLE;
            }

            pGIFInfo->ulRGBTotalBytes = ( ((pGIFInfo->gbm.w * pGIFInfo->gbm.bpp + 31)/32) * 4 )
              * pGIFInfo->gbm.h;
            pGIFInfo->ulImgTotalBytes = pGIFInfo->ulRGBTotalBytes;

            /********************************************************
             * Get space for full image buffer.
             * This will be retained until the file is closed.
             ********************************************************/
            if (DosAllocMem ((PPVOID) &(pGIFInfo->lpRGBBuf),
                             pGIFInfo->ulRGBTotalBytes,
                             fALLOC))
              {
#ifdef DEBUG
                debug("MMIO_SETHEADER: no memory for image data.\n");
#endif
                return (MMIO_ERROR);
              }

            /************************************************************
             * Flag that MMIOM_SETHEADER has been done.  It can only
             *    be done ONCE for a file.  All future attempts will
             *    be flagged as errors.
             ************************************************************/
            pGIFInfo->bSetHeader = TRUE;

            /************************************************************
             * Create copy of MMIMAGEHEADER for future use.
             ************************************************************/
            pGIFInfo->mmImgHdr = *pMMImgHdr;

            /********************************************************
             * Set up initial pointer value within RGB buffer & image
             ********************************************************/
            pGIFInfo->lImgBytePos = 0;

#ifdef DEBUG
            debug("MMIO_SETHEADER done\n");
#endif

            return (sizeof (MMIMAGEHEADER));
            }  /* end case of MMIOM_SETHEADER */

            /*#############################################################*/
            /*#############################################################*/
        case MMIOM_WRITE:
            {

            /************************************************************
             * Declare Local Variables.
             ************************************************************/
            PGIFFILESTATUS       pGIFInfo;
            USHORT              usBitCount;
            LONG                lBytesWritten;
            ULONG               ulImgBytesToWrite;
#ifdef DEBUG
            debug("MMIO_WRITE\n");
#endif

            /************************************************************
             * Check for valid MMIOINFO block.
             ************************************************************/
            if (!pmmioinfo)
                return (MMIO_ERROR);

            /************************************************************
             * Set up our working variable GIFFILESTATUS.
             ************************************************************/
            pGIFInfo = (PGIFFILESTATUS) pmmioinfo->pExtraInfoStruct;

            /************************************************************
             * See if a SetHeader has been done on this file.
             ************************************************************/
            if ((!pGIFInfo) || (!pGIFInfo->bSetHeader))
              {
                return (MMIO_ERROR);
              }

            if (!(pmmioinfo->ulTranslate & MMIO_TRANSLATEDATA))
              {
                /********************************************************
                 * Translation is off, take amount of bytes sent and
                 * write to the file.
                 ********************************************************
                 * Ensure that there is a data buffer to write from
                 ********************************************************/
                if (!lParam1)
                  return (MMIO_ERROR);

                /* We don't do untranslated writes. We shouldn't come here because
                   the MMIO_OPEN call must fail because of the TRANSLATE flag */
                return (MMIO_ERROR);
#if 0
                lBytesWritten = mmioWrite (pVidInfo->hmmioSS,
                                           (PVOID) lParam1,
                                           (ULONG) lParam2);

                return (lBytesWritten);
#endif
                }

            /************************************************************
             * Translation is on.
             *************************************************************/

#ifdef DEBUG
            debug("MMIO_WRITE: pGIFInfo->lImgBytePos %d lParam2 %d pGIFInfo->ulImgTotalBytes: %d bytes\n",
                     pGIFInfo->lImgBytePos, lParam2, pGIFInfo->ulImgTotalBytes);
#endif

            /************************************************************
             * Ensure we do not attempt to write past the end of the
             *    buffer...
             ************************************************************/
            if ((ULONG)(pGIFInfo->lImgBytePos + lParam2) >
               pGIFInfo->ulImgTotalBytes)
               ulImgBytesToWrite =
                   pGIFInfo->ulImgTotalBytes - pGIFInfo->lImgBytePos;
            else {
               ulImgBytesToWrite = (ULONG)lParam2;
            }

#ifdef DEBUG
            debug("MMIO_WRITE: ulImgBytesToWrite %d \n", ulImgBytesToWrite);
#endif

            /************************************************************
             * Write the data into the image buffer.  It will be converted
             *  when the file is closed.  This allows the
             *   application to seek to arbitrary  positions within the
             *   image in terms of the bits/pel, etc they are writing.
             ************************************************************/
            memcpy (&(pGIFInfo->lpRGBBuf[pGIFInfo->lImgBytePos]),
                    (PVOID)lParam1,
                    ulImgBytesToWrite);

            /* Update current position in the image buffer */
            pGIFInfo->lImgBytePos += ulImgBytesToWrite;
#ifdef DEBUG
            debug("MMIO_WRITE: written %d bytes\n", ulImgBytesToWrite);
#endif
            return (ulImgBytesToWrite);
            }   /* end case of MMIOM_WRITE */

        /*
         * If the IOProc has a child IOProc, then pass the message on to the Child, otherwise
         * return Unsupported Message
         */
        default:
            {
             /*
              * Declare Local Variables.
              */
              //             PMMFILESTATUS       pVidInfo;
             LONG                lRC;

             /************************************************************
              * Check for valid MMIOINFO block.
              ************************************************************/
             if (!pmmioinfo)
                 return (MMIO_ERROR);

             /* !!!!!!!!!!!! FIXME !!!!!!!!!!!!!*/
             return (MMIOERR_UNSUPPORTED_MESSAGE);
#if 0
             /************************************************************
              * Set up our working variable MMFILESTATUS.
              ************************************************************/
             pVidInfo = (PMMFILESTATUS) pmmioinfo->pExtraInfoStruct;

             if (pVidInfo != NULL && pVidInfo->hmmioSS)
                {
                 lRC = mmioSendMessage (pVidInfo->hmmioSS,
                                        usMsg,
                                        lParam1,
                                        lParam2);
                 if (!lRC)
                    pmmioinfo->ulErrorRet = mmioGetLastError (pVidInfo->hmmioSS);
                 return (lRC);
                }
            else
               {
                if (pmmioinfo != NULL)
                   pmmioinfo->ulErrorRet = MMIOERR_UNSUPPORTED_MESSAGE;
                return (MMIOERR_UNSUPPORTED_MESSAGE);
               }
#endif
            }   /* end case of Default */

        } /* end SWITCH statement for MMIO messages */

    return (0);
    }      /* end of window procedure */
