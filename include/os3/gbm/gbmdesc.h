#ifndef _GBM_DESC_H_
#define _GBM_DESC_H_

/*

gbmdesc.h - File format description (extracted for simplified localization)

History:
--------
(Heiko Nitzsche)

23-Feb-2006: Move format description strings of all GBM formats to this file
             to simplify localization by simply replacing this file.
10-Jun-2006: Add PBM format
11-Jun-2006: Add PNM format
             Shorten BMP long description
16-Jun-2006: Remove not on binary restriction of PxM formats (ascii is supported now)
*/

/* ------------------------------------------------- */

/* OS/2-, Windows bitmap format */
#define GBM_FMT_DESC_SHORT_BMP  "Bitmap"
#define GBM_FMT_DESC_LONG_BMP   "OS/2 / Windows bitmap"
#define GBM_FMT_DESC_EXT_BMP    "BMP VGA BGA RLE DIB RL4 RL8"

/* Portrait */
#define GBM_FMT_DESC_SHORT_CVP  "Portrait"
#define GBM_FMT_DESC_LONG_CVP   "Portrait"
#define GBM_FMT_DESC_EXT_CVP    "CVP"

/* GEM Raster */
#define GBM_FMT_DESC_SHORT_GEM  "GemRas"
#define GBM_FMT_DESC_LONG_GEM   "GEM Raster"
#define GBM_FMT_DESC_EXT_GEM    "IMG XIMG"

/* CompuServe Graphics Interchange Format */
#define GBM_FMT_DESC_SHORT_GIF  "GIF"
#define GBM_FMT_DESC_LONG_GIF   "CompuServe Graphics Interchange Format"
#define GBM_FMT_DESC_EXT_GIF    "GIF"

/* IBM Image Access eXecutive support */
#define GBM_FMT_DESC_SHORT_IAX  "IAX"
#define GBM_FMT_DESC_LONG_IAX   "IBM Image Access eXecutive"
#define GBM_FMT_DESC_EXT_IAX    "IAX"

/* JPEG File Interchange Format */
#ifdef ENABLE_IJG
  #define GBM_FMT_DESC_SHORT_JPG  "JPEG"
  #define GBM_FMT_DESC_LONG_JPG   "JPEG Interchange File Format"
  #define GBM_FMT_DESC_EXT_JPG    "JPG JPEG JPE"
#endif

/* JPEG 2000 Graphics Format */
#ifdef ENABLE_J2K
  #define GBM_FMT_DESC_SHORT_JP2  "JP2"
  #define GBM_FMT_DESC_LONG_JP2   "JPEG2000 Graphics File Format"
  #define GBM_FMT_DESC_EXT_JP2    "JP2 J2C"

  #define GBM_FMT_DESC_SHORT_J2K  "J2K"
  #define GBM_FMT_DESC_LONG_J2K   "JPEG2000 Codestream"
  #define GBM_FMT_DESC_EXT_J2K    "J2K JPC"

  #define GBM_FMT_DESC_SHORT_JPT  "JPT"
  #define GBM_FMT_DESC_LONG_JPT   "JPT Stream (JPEG2000, JPIP)"
  #define GBM_FMT_DESC_EXT_JPT    "JPT"
#endif

/* IBM KIPS file format */
#define GBM_FMT_DESC_SHORT_KPS  "KIPS"
#define GBM_FMT_DESC_LONG_KPS   "IBM KIPS"
#define GBM_FMT_DESC_EXT_KPS    "KPS"

/* Amiga IFF / ILBM format */
#define GBM_FMT_DESC_SHORT_LBM  "ILBM"
#define GBM_FMT_DESC_LONG_LBM   "Amiga IFF / ILBM Interleaved bitmap"
#define GBM_FMT_DESC_EXT_LBM    "IFF LBM"

/* ZSoft PC Paintbrush format */
#define GBM_FMT_DESC_SHORT_PCX  "PCX"
#define GBM_FMT_DESC_LONG_PCX   "ZSoft PC Paintbrush Image format"
#define GBM_FMT_DESC_EXT_PCX    "PCX PCC"

/* Portable Network Graphics Format */
#ifdef ENABLE_PNG
  #define GBM_FMT_DESC_SHORT_PNG  "PNG"
  #define GBM_FMT_DESC_LONG_PNG   "Portable Network Graphics Format"
  #define GBM_FMT_DESC_EXT_PNG    "PNG"
#endif

/* Poskanzers PBM format */
#define GBM_FMT_DESC_SHORT_PBM  "Bit-map"
#define GBM_FMT_DESC_LONG_PBM   "Portable Bit-map"
#define GBM_FMT_DESC_EXT_PBM    "PBM"

/* Poskanzers PGM format */
#define GBM_FMT_DESC_SHORT_PGM  "Greymap"
#define GBM_FMT_DESC_LONG_PGM   "Portable Greyscale-map"
#define GBM_FMT_DESC_EXT_PGM    "PGM"

/* Poskanzers PPM format */
#define GBM_FMT_DESC_SHORT_PPM  "Pixmap"
#define GBM_FMT_DESC_LONG_PPM   "Portable Pixel-map"
#define GBM_FMT_DESC_EXT_PPM    "PPM"

/* Poskanzers PNM format */
#define GBM_FMT_DESC_SHORT_PNM  "Anymap"
#define GBM_FMT_DESC_LONG_PNM   "Portable Any-map"
#define GBM_FMT_DESC_EXT_PNM    "PNM"

/* IBM Printer Page Segment format */
#define GBM_FMT_DESC_SHORT_PSG  "PSEG"
#define GBM_FMT_DESC_LONG_PSG   "IBM Printer Page Segment"
#define GBM_FMT_DESC_EXT_PSG    "PSE PSEG PSEG38PP PSEG3820"

/* Archimedes Sprite from RiscOS Format */
#define GBM_FMT_DESC_SHORT_SPR  "Sprite"
#define GBM_FMT_DESC_LONG_SPR   "Archimedes Sprite from RiscOS"
#define GBM_FMT_DESC_EXT_SPR    "SPR SPRITE"

/* Truevision Targa/Vista bitmap Format */
#define GBM_FMT_DESC_SHORT_TGA  "Targa"
#define GBM_FMT_DESC_LONG_TGA   "Truevision Targa/Vista bitmap"
#define GBM_FMT_DESC_EXT_TGA    "TGA VST AFI"

/* Microsoft/Aldus Tagged Image File Format */
#ifdef ENABLE_TIF
  #define GBM_FMT_DESC_SHORT_TIF  "TIFF"
  #define GBM_FMT_DESC_LONG_TIF   "Tagged Image File Format support (TIFF 6.0)"
  #define GBM_FMT_DESC_EXT_TIF    "TIF TIFF"
#endif

/* YUV12C M-Motion Video Frame Buffer format */
#define GBM_FMT_DESC_SHORT_VID  "YUV12C"
#define GBM_FMT_DESC_LONG_VID   "YUV12C M-Motion Video Frame Buffer"
#define GBM_FMT_DESC_EXT_VID    "VID"

/* X Windows Bitmap format */
#define GBM_FMT_DESC_SHORT_XBM  "XBitmap"
#define GBM_FMT_DESC_LONG_XBM   "X Windows Bitmap"
#define GBM_FMT_DESC_EXT_XBM    "XBM"

/* X Windows PixMap format */
#define GBM_FMT_DESC_SHORT_XPM  "XPixMap"
#define GBM_FMT_DESC_LONG_XPM   "X Windows PixMap"
#define GBM_FMT_DESC_EXT_XPM    "XPM"


#endif


