/* 
 $Id: F_dive.hpp,v 1.1 2003/06/18 22:57:39 evgen2 Exp $ 
*/

/***************************************************************************\
*
* Module Name: F_dive.hp
*
* OS/2 2.1 Multimedia Extensions Display Engine API data structures
*  
* should provide compatibility with dive.h
*
\****************************************************************************/

/* F_dive.hpp */
#ifndef FREEPM_DIVE
#define FREEPM_DIVE

#ifdef __cplusplus
      extern "C" {
#endif

#define FOURCC ULONG
#define HDIVE  ULONG

#define DIVE_SUCCESS                                     0x00000000
#define DIVE_ERR_INVALID_INSTANCE                        0x00001000
#define DIVE_ERR_SOURCE_FORMAT                           0x00001001
#define DIVE_ERR_DESTINATION_FORMAT                      0x00001002
#define DIVE_ERR_BLITTER_NOT_SETUP                       0x00001003
#define DIVE_ERR_INSUFFICIENT_LENGTH                     0x00001004
#define DIVE_ERR_TOO_MANY_INSTANCES                      0x00001005
#define DIVE_ERR_NO_DIRECT_ACCESS                        0x00001006
#define DIVE_ERR_NOT_BANK_SWITCHED                       0x00001007
#define DIVE_ERR_INVALID_BANK_NUMBER                     0x00001008
#define DIVE_ERR_FB_NOT_ACQUIRED                         0x00001009
#define DIVE_ERR_FB_ALREADY_ACQUIRED                     0x0000100a
#define DIVE_ERR_ACQUIRE_FAILED                          0x0000100b
#define DIVE_ERR_BANK_SWITCH_FAILED                      0x0000100c
#define DIVE_ERR_DEACQUIRE_FAILED                        0x0000100d
#define DIVE_ERR_INVALID_PALETTE                         0x0000100e
#define DIVE_ERR_INVALID_DESTINATION_RECTL               0x0000100f
#define DIVE_ERR_INVALID_BUFFER_NUMBER                   0x00001010
#define DIVE_ERR_SSMDD_NOT_INSTALLED                     0x00001011
#define DIVE_ERR_BUFFER_ALREADY_ACCESSED                 0x00001012
#define DIVE_ERR_BUFFER_NOT_ACCESSED                     0x00001013
#define DIVE_ERR_TOO_MANY_BUFFERS                        0x00001014
#define DIVE_ERR_ALLOCATION_ERROR                        0x00001015
#define DIVE_ERR_INVALID_LINESIZE                        0x00001016
#define DIVE_ERR_FATAL_EXCEPTION                         0x00001017
#define DIVE_ERR_INVALID_CONVERSION                      0x00001018
#define DIVE_ERR_VSD_ERROR                               0x00001019
#define DIVE_ERR_COLOR_SUPPORT                           0x0000101a
#define DIVE_ERR_OUT_OF_RANGE                            0x0000101b
#define DIVE_WARN_NO_SIZE                                0x00001100

#define DIVE_BUFFER_SCREEN                               0x00000000
#define DIVE_BUFFER_GRAPHICS_PLANE                       0x00000001
#define DIVE_BUFFER_ALTERNATE_PLANE                      0x00000002

#define DIVE_FULLY_VISIBLE                               0xffffffff


typedef struct _DIVE_CAPS {

   ULONG  ulStructLen;            /* Set equal to sizeof(DIVE_CAPS)          */
   ULONG  ulPlaneCount;           /* Number of defined planes.               */

   /* Info returned in the following fields pertains to ulPlaneID.           */
   BOOL   fScreenDirect;          /* TRUE if can get addressability to vram. */
   BOOL   fBankSwitched;          /* TRUE if vram is bank-switched.          */
   ULONG  ulDepth;                /* Number of bits per pixel.               */
   ULONG  ulHorizontalResolution; /* Screen width in pixels.                 */
   ULONG  ulVerticalResolution;   /* Screen height in pixels.                */
   ULONG  ulScanLineBytes;        /* Screen scan line size in bytes.         */
   FOURCC fccColorEncoding;       /* Colorspace encoding of the screen.      */
   ULONG  ulApertureSize;         /* Size of vram aperture in bytes.         */

   ULONG  ulInputFormats;         /* Number of input color formats.          */
   ULONG  ulOutputFormats;        /* Number of output color formats.         */
   ULONG  ulFormatLength;         /* Length of format buffer.                */
   PVOID  pFormatData;            /* Pointer to color format buffer FOURCC's.*/

   } DIVE_CAPS;
typedef DIVE_CAPS *PDIVE_CAPS;

typedef struct _SETUP_BLITTER {

     /* Set the ulStructLen field equal to the amount of the structure used. */
     /* allowable: blank lines below mark sizes of 8, 28, 32, 52, 60, or 68. */
   ULONG  ulStructLen;
     /* Set the ulInvert flags based on the following:                       */
     /* b0001 = d01 = h01 = flip the image in the horizontal direction.      */
     /* b0010 = d02 = h02 = flip the image in the vertical direction.        */
     /* All other bits ignored.                                              */
   ULONG  fInvert;

     /* This is the color format of the source data.  See "FOURCC.H"         */
   FOURCC fccSrcColorFormat;
     /* This is the width of the source image in pixels.                     */
   ULONG  ulSrcWidth;
     /* This is the height of the source image in pixels.                    */
   ULONG  ulSrcHeight;
     /* This is the horizontal offset from which to start displaying for     */
     /* use in displaying a sub-portion of the source image.                 */
   ULONG  ulSrcPosX;
     /* This is the vertical offset from which to start displaying.          */
   ULONG  ulSrcPosY;

     /* This is the dither type to use.  0 defines no dither and 1           */
     /* defines 2x2 dither (all others ignored).  Note: dithering is only    */
     /* supported in direct color to LUT8 conversions.                       */
   ULONG  ulDitherType;

     /* This is the color format of the destinaion data.  See "FOURCC.H"     */
   FOURCC fccDstColorFormat;
     /* This is the width of the destination image in pixels.                */
   ULONG  ulDstWidth;
     /* This is the height of the destination image in pixels.               */
   ULONG  ulDstHeight;
     /* This is the horizontal offset from which to start displaying for     */
     /* use in displaying to sub-portion of the destination image.           */
   LONG   lDstPosX;
     /* This is the vertical offset from which to start displaying.          */
   LONG   lDstPosY;

     /* This is the world screen horizontal position, where 0 is left.       */
     /* These are ignored if the destination is not the screen.              */
   LONG   lScreenPosX;
     /* This is the world screen vertical position, where 0 is bottom.       */
   LONG   lScreenPosY;

     /* This is the number of visible rectangular regions being passed in.   */
     /* These are ignored if the destination is not the screen.              */
     /* Also, if you application *KNOWS* that the region is fully visible    */
     /* (like not going to the screen), the you can use DIVE_FULLY_VISIBLE   */
     /* instead of making up a bogus visible region structure.               */
   ULONG  ulNumDstRects;
     /* This points to an array of visible regions which defines what        */
     /* portions of the source image are to be displayed.                    */
   PRECTL pVisDstRects;           /* Pointer to array of visible rectangles. */

   } SETUP_BLITTER;
typedef SETUP_BLITTER *PSETUP_BLITTER;


ULONG APIENTRY DiveQueryCaps ( PDIVE_CAPS pDiveCaps,
                               ULONG      ulPlaneBufNum );

ULONG APIENTRY DiveOpen ( HDIVE *phDiveInst,
                          BOOL   fNonScreenInstance,
                          PVOID  ppFrameBuffer );

ULONG APIENTRY DiveSetupBlitter ( HDIVE          hDiveInst,
                                  PSETUP_BLITTER pSetupBlitter );

ULONG APIENTRY DiveBlitImage ( HDIVE hDiveInst,
                               ULONG ulSrcBufNumber,
                               ULONG ulDstBufNumber );


/* Notes on DiveAllocImageBuffer:
      If pbImageBuffer is not NULL, the buffer is associated rather than
      allocated.  If pbImageBuffer is not NULL and the buffer number
      pointed to by pulBufferNumber is non-zero, a new buffer pointer is
      associated with the buffer number.  Even though no memory is
      allocated by DiveAllocImageBuffer when user-allocated buffers are
      associated, DiveFreeImageBuffer should be called to release the
      buffer association to avoid using up available buffer indexes.
      The specified line size will be used if a buffer is allocated in
      system memory, or if a user buffer is associated.  If the
      specified line size is zero, the allocated line size is rounded up
      to the nearest DWORD boundry.
*/

ULONG APIENTRY DiveAllocImageBuffer ( HDIVE  hDiveInst,
                                      PULONG pulBufferNumber,
                                      FOURCC fccColorSpace,
                                      ULONG  ulWidth,
                                      ULONG  ulHeight,
                                      ULONG  ulLineSizeBytes,
                                      PBYTE  pbImageBuffer );

ULONG APIENTRY DiveFreeImageBuffer ( HDIVE hDiveInst,
                                     ULONG ulBufferNumber );

ULONG APIENTRY DiveBeginImageBufferAccess ( HDIVE  hDiveInst,
                                            ULONG  ulBufferNumber,
                                            PBYTE *ppbImageBuffer,
                                            PULONG pulBufferScanLineBytes,
                                            PULONG pulBufferScanLines );

ULONG APIENTRY DiveEndImageBufferAccess ( HDIVE hDiveInst,
                                          ULONG ulBufferNumber );


#ifdef __cplusplus
      }
#endif

#endif /* FREEPM_DIVE */
