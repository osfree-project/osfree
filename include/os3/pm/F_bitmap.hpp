/* 
 $Id: F_bitmap.hpp,v 1.1 2003/06/18 22:36:12 evgen2 Exp $ 
*/
/* F_bitmap.hpp */
#ifndef FREEPM_BITMAP
 #define FREEPM_BITMAP

/****************************** Module Header ******************************\
*
* Module Name: PMBITMAP.H
*
* OS/2 Presentation Manager Bit Map, Icon and Pointer type declarations.
*
* ===========================================================================
*
* This is the file format structure for Bit Maps, Pointers and Icons
* as stored in the resource file of a PM application.
*
* Notes on file format:
*
* Each BITMAPFILEHEADER entry is immediately followed by the color table
* for the bit map bits it references.
* Icons and Pointers contain two BITMAPFILEHEADERs for each ARRAYHEADER
* item.  The first one is for the ANDXOR mask, the second is for the
* COLOR mask.  All offsets are absolute based on the start of the FILE.
*
* For OS/2 Version 2.0 and later BITMAPFILEHEADER2 and the other '2'
* versions of each structure are recommended. Use the non-2 versions
* of each structure if compatibility with OS/2 Version 1.X is required.
*
* ===========================================================================
*
* Comments at the end of each typedef line give the name tags used in
* the assembler include version of this file.
*
* The assembler include version of this file excludes lines between XLATOFF
* and XLATON comments.
*
\***************************************************************************/

#ifdef __cplusplus
      extern "C" {
#endif

   /* RGB data for _BITMAPINFO struct */
   typedef struct _RGB              /* rgb */
   {
      BYTE bBlue;
      BYTE bGreen;
      BYTE bRed;
   } RGB;
   

   /* bitmap data used by GpiSetBitmapBits and others */
   typedef struct _BITMAPINFO       /* bmi */
   {
      ULONG  cbFix;
      USHORT cx;
      USHORT cy;
      USHORT cPlanes;
      USHORT cBitCount;
      RGB    argbColor[1];
   } BITMAPINFO;
   typedef BITMAPINFO *PBITMAPINFO;
   
   /* Constants for compression/decompression command */
   
   #define CBD_COMPRESSION     1L
   #define CBD_DECOMPRESSION   2L
   #define CBD_BITS            0L
   
   /* Flags for compression/decompression option */
   
   #define CBD_COLOR_CONVERSION    0x00000001L

   /* Compression scheme in the ulCompression field of the bitmapinfo structure */
   
   #define BCA_UNCOMP          0L
   #define BCA_HUFFMAN1D       3L
   #define BCA_RLE4            2L
   #define BCA_RLE8            1L
   #define BCA_RLE24           4L
   
   #define BRU_METRIC          0L
   
   #define BRA_BOTTOMUP        0L
   
   #define BRH_NOTHALFTONED    0L
   #define BRH_ERRORDIFFUSION  1L
   #define BRH_PANDA           2L
   #define BRH_SUPERCIRCLE     3L
   
   #define BCE_PALETTE         (-1L)
   #define BCE_RGB             0L

   typedef struct _BITMAPINFOHEADER2        /* bmp2  */
   {
      ULONG  cbFix;            /* Length of structure                    */
      ULONG  cx;               /* Bit-map width in pels                  */
      ULONG  cy;               /* Bit-map height in pels                 */
      USHORT cPlanes;          /* Number of bit planes                   */
      USHORT cBitCount;        /* Number of bits per pel within a plane  */
      ULONG  ulCompression;    /* Compression scheme used to store the bitmap */
      ULONG  cbImage;          /* Length of bit-map storage data in bytes*/
      ULONG  cxResolution;     /* x resolution of target device          */
      ULONG  cyResolution;     /* y resolution of target device          */
      ULONG  cclrUsed;         /* Number of color indices used           */
      ULONG  cclrImportant;    /* Number of important color indices      */
      USHORT usUnits;          /* Units of measure                       */
      USHORT usReserved;       /* Reserved                               */
      USHORT usRecording;      /* Recording algorithm                    */
      USHORT usRendering;      /* Halftoning algorithm                   */
      ULONG  cSize1;           /* Size value 1                           */
      ULONG  cSize2;           /* Size value 2                           */
      ULONG  ulColorEncoding;  /* Color encoding                         */
      ULONG  ulIdentifier;     /* Reserved for application use           */
   } BITMAPINFOHEADER2;
   typedef BITMAPINFOHEADER2 *PBITMAPINFOHEADER2;

   typedef struct _RGB2         /* rgb2 */
   {
      BYTE bBlue;              /* Blue component of the color definition */
      BYTE bGreen;             /* Green component of the color definition*/
      BYTE bRed;               /* Red component of the color definition  */
      BYTE fcOptions;          /* Reserved, must be zero                 */
   } RGB2;
   typedef RGB2 *PRGB2;
   
   typedef struct _BITMAPINFO2      /* bmi2 */
   {
      ULONG  cbFix;            /* Length of fixed portion of structure   */
      ULONG  cx;               /* Bit-map width in pels                  */
      ULONG  cy;               /* Bit-map height in pels                 */
      USHORT cPlanes;          /* Number of bit planes                   */
      USHORT cBitCount;        /* Number of bits per pel within a plane  */
      ULONG  ulCompression;    /* Compression scheme used to store the bitmap */
      ULONG  cbImage;          /* Length of bit-map storage data in bytes*/
      ULONG  cxResolution;     /* x resolution of target device          */
      ULONG  cyResolution;     /* y resolution of target device          */
      ULONG  cclrUsed;         /* Number of color indices used           */
      ULONG  cclrImportant;    /* Number of important color indices      */
      USHORT usUnits;          /* Units of measure                       */
      USHORT usReserved;       /* Reserved                               */
      USHORT usRecording;      /* Recording algorithm                    */
      USHORT usRendering;      /* Halftoning algorithm                   */
      ULONG  cSize1;           /* Size value 1                           */
      ULONG  cSize2;           /* Size value 2                           */
      ULONG  ulColorEncoding;  /* Color encoding                         */
      ULONG  ulIdentifier;     /* Reserved for application use           */
      RGB2   argbColor[1];     /* Color definition record                */
   } BITMAPINFO2;
   typedef BITMAPINFO2 *PBITMAPINFO2;

#ifdef __cplusplus
        }
#endif

#endif
 //FREEPM_BITMAP