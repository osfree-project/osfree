/*
 $Id: F_GPI.HPP,v 1.4 2003/06/18 22:51:49 evgen2 Exp $
*/
/* F_GPI.hpp */
/* ver 0.00 14.08.2002 */
/* analog \TOOLKIT\h\pmgpi.h */

#ifndef FREEPM_GPI
#define FREEPM_GPI

#include "F_GPI_struct.hpp"

#ifdef __cplusplus
      extern "C" {
#endif

/* General GPI return values */
#define GPI_ERROR                       0
#define GPI_OK                          1
#define GPI_ALTERROR                  (-1)

   /* units for GpiCreatePS and others */
   #define PU_ARBITRARY               0x0004L
   #define PU_PELS                    0x0008L
   #define PU_LOMETRIC                0x000CL
   #define PU_HIMETRIC                0x0010L
   #define PU_LOENGLISH               0x0014L
   #define PU_HIENGLISH               0x0018L
   #define PU_TWIPS                   0x001CL

   /* format for GpiCreatePS */
   #define GPIF_DEFAULT                    0L
   #define GPIF_SHORT                 0x0100L
   #define GPIF_LONG                  0x0200L


   /* PS type for GpiCreatePS */
   #define GPIT_NORMAL                     0L
   #define GPIT_MICRO                 0x1000L
   #define GPIT_INK                   0x2000L


   /* implicit associate flag for GpiCreatePS */
   #define GPIA_NOASSOC                    0L
   #define GPIA_ASSOC                 0x4000L

   /* PS Fill type for GpiCreatePS */
   #define GPIM_AREAEXCL              0x8000L



   /* common GPICONTROL functions */

   HPS   APIENTRY F_GpiCreatePS(HAB hab,
                              HDC hdc,
                              PSIZEL psizlSize,
                              ULONG flOptions);

   BOOL  APIENTRY F_GpiDestroyPS(HPS hps);


   /* default color table indices */

   #define CLR_FALSE                     (-5)
   #define CLR_TRUE                      (-4)

   #define CLR_ERROR                   (-255)
   #define CLR_DEFAULT                   (-3)
   #define CLR_WHITE                     (-2)
   #define CLR_BLACK                     (-1)
   #define CLR_BACKGROUND                  0
   #define CLR_BLUE                        1
   #define CLR_RED                         2
   #define CLR_PINK                        3
   #define CLR_GREEN                       4
   #define CLR_CYAN                        5
   #define CLR_YELLOW                      6
   #define CLR_NEUTRAL                     7

   #define CLR_DARKGRAY                    8
   #define CLR_DARKBLUE                    9
   #define CLR_DARKRED                    10
   #define CLR_DARKPINK                   11
   #define CLR_DARKGREEN                  12
   #define CLR_DARKCYAN                   13
   #define CLR_BROWN                      14
   #define CLR_PALEGRAY                   15

   /* rgb colors */
   #define RGB_ERROR                   (-255)
   #define RGB_BLACK              0x00000000
   #define RGB_BLUE               0x000000FF
   #define RGB_GREEN              0x0000FF00
   #define RGB_CYAN               0x0000FFFF
   #define RGB_RED                0x00FF0000
   #define RGB_PINK               0x00FF00FF
   #define RGB_YELLOW             0x00FFFF00
   #define RGB_WHITE              0x00FFFFFF


   /* basic pattern symbols */
   #define PATSYM_ERROR                  (-1L)
   #define PATSYM_DEFAULT                  0L
   #define PATSYM_DENSE1                   1L
   #define PATSYM_DENSE2                   2L
   #define PATSYM_DENSE3                   3L
   #define PATSYM_DENSE4                   4L
   #define PATSYM_DENSE5                   5L
   #define PATSYM_DENSE6                   6L
   #define PATSYM_DENSE7                   7L
   #define PATSYM_DENSE8                   8L
   #define PATSYM_VERT                     9L
   #define PATSYM_HORIZ                   10L
   #define PATSYM_DIAG1                   11L
   #define PATSYM_DIAG2                   12L
   #define PATSYM_DIAG3                   13L
   #define PATSYM_DIAG4                   14L
   #define PATSYM_NOSHADE                 15L
   #define PATSYM_SOLID                   16L
   #ifndef INCL_SAADEFS
      #define PATSYM_HALFTONE                17L
   #endif /* no INCL_SAADEFS */
   #define PATSYM_HATCH                   18L
   #define PATSYM_DIAGHATCH               19L
   #define PATSYM_BLANK                   64L
   

   /* lcid values for GpiSet/QueryPattern and others */
   #define LCID_ERROR                    (-1L)
   #define LCID_DEFAULT                    0L

/*************************************************/
/*             GPI primitive functions           */
/*************************************************/

      /*** global primitive functions */
      BOOL APIENTRY  F_GpiSetColor(HPS hps,
                                 LONG lColor);

      LONG APIENTRY  F_GpiQueryColor(HPS hps);

      /*** line primitive functions */

      LONG APIENTRY  F_GpiBox(HPS hps,
                            LONG lControl,
                            PPOINTL pptlPoint,
                            LONG lHRound,
                            LONG lVRound);

      BOOL APIENTRY  F_GpiMove(HPS hps,
                             PPOINTL pptlPoint);

      LONG APIENTRY  F_GpiLine(HPS hps,
                             PPOINTL pptlEndPoint);

      LONG APIENTRY  F_GpiPolyLine(HPS hps,
                                 LONG lCount,
                                 PPOINTL aptlPoints);

      LONG APIENTRY  F_GpiPolyLineDisjoint(HPS hps,
                                         LONG lCount,
                                         PPOINTL aptlPoints);

      /*** area primitive functions */
      BOOL  APIENTRY F_GpiSetPattern(HPS hps,
                                   LONG lPatternSymbol);

      LONG  APIENTRY F_GpiQueryPattern(HPS hps);

      BOOL  APIENTRY F_GpiSetPatternSet(HPS hps,
                                      LONG lSet);

      LONG  APIENTRY F_GpiQueryPatternSet(HPS hps);


      BOOL APIENTRY  F_GpiBeginArea(HPS hps,
                                  ULONG flOptions);

      LONG APIENTRY  F_GpiEndArea(HPS hps);

      /*** character primitive functions */
      LONG APIENTRY  F_GpiCharString(HPS hps,
                                   LONG lCount,
                                   PCH pchString);

      LONG APIENTRY  F_GpiCharStringAt(HPS hps,
                                     PPOINTL pptlPoint,
                                     LONG lCount,
                                     PCH pchString);

   /* usDirection of returned region data for GpiQueryRegionRects */
   #define RECTDIR_LFRT_TOPBOT             1L
   #define RECTDIR_RTLF_TOPBOT             2L
   #define RECTDIR_LFRT_BOTTOP             3L
   #define RECTDIR_RTLF_BOTTOP             4L

       BOOL APIENTRY F_GpiQueryRegionRects(HPS hps,
                                     HRGN hrgn,
                                     PRECTL prclBound,
                                     PRGNRECT prgnrcControl,
                                     PRECTL prclRect);
      HRGN  APIENTRY F_GpiCreateRegion(HPS hps,
                                     LONG lCount,
                                     PRECTL arclRectangles);

      BOOL  APIENTRY F_GpiDestroyRegion(HPS hps,
                                      HRGN hrgn);


/*************************************************/
/*             GPI bitmap functions              */
/*************************************************/
      /*** bitmap and pel functions */
      BOOL  APIENTRY F_GpiQueryDeviceBitmapFormats(HPS hps,
                                                   LONG lCount,
                                                   PLONG alArray);

   /*** include bitmap structures and file formats ***/
   #include "F_bitmap.hpp"

   /* usage flags for GpiCreateBitmap */
   #define CBM_INIT        0x0004L

   /* error return code for GpiSet/QueryBitmapBits */
   #define BMB_ERROR                     (-1L)

      HBITMAP APIENTRY F_GpiCreateBitmap(HPS hps,
                                       PBITMAPINFOHEADER2 pbmpNew,
                                       ULONG flOptions,
                                       PBYTE pbInitData,
                                       PBITMAPINFO2 pbmiInfoTable);
      BOOL  APIENTRY F_GpiDeleteBitmap(HBITMAP hbm);

      HBITMAP  APIENTRY F_GpiSetBitmap(HPS hps,
                                     HBITMAP hbm);


      LONG    APIENTRY F_GpiSetBitmapBits(HPS hps,
                                        LONG lScanStart,
                                        LONG lScans,
                                        PBYTE pbBuffer,
                                        PBITMAPINFO2 pbmiInfoTable);

      LONG    APIENTRY F_GpiQueryBitmapBits(HPS hps,
                                          LONG lScanStart,
                                          LONG lScans,
                                          PBYTE pbBuffer,
                                          PBITMAPINFO2 pbmiInfoTable);


      LONG    APIENTRY F_GpiSetPel(HPS hps,
                                 PPOINTL pptlPoint);

      LONG    APIENTRY F_GpiQueryPel(HPS hps,
                                   PPOINTL pptlPoint);


   /* raster operations defined for GpiBitBlt */
   #define ROP_SRCCOPY                0x00CCL
   #define ROP_SRCPAINT               0x00EEL
   #define ROP_SRCAND                 0x0088L
   #define ROP_SRCINVERT              0x0066L
   #define ROP_SRCERASE               0x0044L
   #define ROP_NOTSRCCOPY             0x0033L
   #define ROP_NOTSRCERASE            0x0011L
   #define ROP_MERGECOPY              0x00C0L
   #define ROP_MERGEPAINT             0x00BBL
   #define ROP_PATCOPY                0x00F0L
   #define ROP_PATPAINT               0x00FBL
   #define ROP_PATINVERT              0x005AL
   #define ROP_DSTINVERT              0x0055L
   #define ROP_ZERO                   0x0000L
   #define ROP_ONE                    0x00FFL

   /* Blt options for GpiBitBlt */
   #define BBO_OR                          0L
   #define BBO_AND                         1L
   #define BBO_IGNORE                      2L
   #define BBO_PAL_COLORS                  4L
   #define BBO_NO_COLOR_INFO               8L

      LONG   APIENTRY F_GpiBitBlt(HPS hpsTarget,
                                  HPS hpsSource,
                                  LONG lCount,
                                  PPOINTL aptlPoints,
                                  LONG lRop,
                                  ULONG flOptions);


   /* format of logical lColor table for GpiCreateLogColorTable and others */
   #define LCOLF_DEFAULT                   0L
   #define LCOLF_INDRGB                    1L
   #define LCOLF_CONSECRGB                 2L
   #define LCOLF_RGB                       3L
   #define LCOLF_PALETTE                   4L

   /* options for GpiQueryRealColors and others */
   #define LCOLOPT_REALIZED           0x0001L
   #define LCOLOPT_INDEX              0x0002L

/*** logical lColor table functions (807) */

      BOOL  APIENTRY F_GpiCreateLogColorTable(HPS hps,
                                            ULONG flOptions,
                                            LONG lFormat,
                                            LONG lStart,
                                            LONG lCount,
                                            PLONG alTable);

      BOOL  APIENTRY F_GpiQueryColorData(HPS hps,
                                       LONG lCount,
                                       PLONG alArray);


      LONG  APIENTRY F_GpiQueryLogColorTable(HPS hps,
                                             ULONG flOptions,
                                             LONG lStart,
                                             LONG lCount,
                                             PLONG alArray);


 /*Palette manager functions (852) */

   /* color flags for GpiCreatePalette and others */
   #define PC_RESERVED                   0x01
   #define PC_EXPLICIT                   0x02
   #define PC_NOCOLLAPSE                 0x04


         HPAL APIENTRY F_GpiCreatePalette(HAB hab,
                                        ULONG flOptions,
                                        ULONG ulFormat,
                                        ULONG ulCount,
                                        PULONG aulTable);

         BOOL APIENTRY F_GpiDeletePalette(HPAL hpal);

         HPAL APIENTRY F_GpiSelectPalette(HPS hps,
                                        HPAL hpal);

         LONG APIENTRY F_GpiAnimatePalette(HPAL hpal,
                                         ULONG ulFormat,
                                         ULONG ulStart,
                                         ULONG ulCount,
                                         PULONG aulTable);

         BOOL APIENTRY F_GpiSetPaletteEntries(HPAL hpal,
                                            ULONG ulFormat,
                                            ULONG ulStart,
                                            ULONG ulCount,
                                            PULONG aulTable);

         HPAL APIENTRY F_GpiQueryPalette(HPS hps);

         LONG APIENTRY F_GpiQueryPaletteInfo(HPAL hpal,
                                           HPS  hps,
                                           ULONG flOptions,
                                           ULONG ulStart,
                                           ULONG ulCount,
                                           PULONG aulArray);


         ULONG APIENTRY F_GpiResizePalette(HPAL hpal,
                                         ULONG newsize);

         LONG  APIENTRY F_GpiQueryNearestPaletteIndex(HPAL hpal,
                                                    ULONG color);



      BOOL  APIENTRY GpiCreateLogColorTable(HPS hps,
                                            ULONG flOptions,
                                            LONG lFormat,
                                            LONG lStart,
                                            LONG lCount,
                                            PLONG alTable);


#ifdef __cplusplus
        }
#endif


/**** include bitmap structures and file formats ****/
   #include "F_bitmap.hpp"

#endif /* FREEPM_GPI */
