/* F_pmdev.hpp */
/* 
 $Id: F_pmdev.hpp,v 1.2 2003/06/18 22:51:49 evgen2 Exp $ 
*/
/* ver 0.00 14.06.2003 */
/* analog \TOOLKIT\h\pmdev.h */

#ifndef FREEPM_DEV
#define FREEPM_DEV

#ifdef __cplusplus
      extern "C" {
#endif

   /* DC type for DevOpenDC */
   #define OD_SCREEN                       0L
   #define OD_QUEUED                       2L
   #define OD_DIRECT                       5L
   #define OD_INFO                         6L
   #define OD_METAFILE                     7L
   #define OD_MEMORY                       8L
   #define OD_METAFILE_NOQUERY             9L

   /* codes for DevQueryCaps */
   #define CAPS_FAMILY                     0L
   #define CAPS_IO_CAPS                    1L
   #define CAPS_TECHNOLOGY                 2L
   #define CAPS_DRIVER_VERSION             3L
   #define CAPS_WIDTH                      4L      /* pels            */
   #define CAPS_HEIGHT                     5L      /* pels            */
   #define CAPS_WIDTH_IN_CHARS             6L
   #define CAPS_HEIGHT_IN_CHARS            7L
   #define CAPS_HORIZONTAL_RESOLUTION      8L      /* pels per meter  */
   #define CAPS_VERTICAL_RESOLUTION        9L      /* pels per meter  */
   #define CAPS_CHAR_WIDTH                10L      /* pels            */
   #define CAPS_CHAR_HEIGHT               11L      /* pels            */
   #define CAPS_SMALL_CHAR_WIDTH          12L      /* pels            */
   #define CAPS_SMALL_CHAR_HEIGHT         13L      /* pels            */
   #define CAPS_COLORS                    14L
   #define CAPS_COLOR_PLANES              15L
   #define CAPS_COLOR_BITCOUNT            16L
   #define CAPS_COLOR_TABLE_SUPPORT       17L
   #define CAPS_MOUSE_BUTTONS             18L
   #define CAPS_FOREGROUND_MIX_SUPPORT    19L
   #define CAPS_BACKGROUND_MIX_SUPPORT    20L
   #define CAPS_DEVICE_WINDOWING          31L
   #define CAPS_ADDITIONAL_GRAPHICS       32L
   #define CAPS_VIO_LOADABLE_FONTS        21L
   #define CAPS_WINDOW_BYTE_ALIGNMENT     22L
   #define CAPS_BITMAP_FORMATS            23L
   #define CAPS_RASTER_CAPS               24L
   #define CAPS_MARKER_HEIGHT             25L      /* pels            */
   #define CAPS_MARKER_WIDTH              26L      /* pels            */
   #define CAPS_DEVICE_FONTS              27L
   #define CAPS_GRAPHICS_SUBSET           28L
   #define CAPS_GRAPHICS_VERSION          29L
   #define CAPS_GRAPHICS_VECTOR_SUBSET    30L
   #define CAPS_PHYS_COLORS               33L
   #define CAPS_COLOR_INDEX               34L
   #define CAPS_GRAPHICS_CHAR_WIDTH       35L
   #define CAPS_GRAPHICS_CHAR_HEIGHT      36L
   #define CAPS_HORIZONTAL_FONT_RES       37L
   #define CAPS_VERTICAL_FONT_RES         38L
   #define CAPS_DEVICE_FONT_SIM           39L
   #define CAPS_LINEWIDTH_THICK           40L
   #define CAPS_DEVICE_POLYSET_POINTS     41L
   
   /* Constants for CAPS_IO_CAPS */
   #define CAPS_IO_DUMMY                   1L
   #define CAPS_IO_SUPPORTS_OP             2L
   #define CAPS_IO_SUPPORTS_IP             3L
   #define CAPS_IO_SUPPORTS_IO             4L
   
   /* Constants for CAPS_TECHNOLOGY */
   #define CAPS_TECH_UNKNOWN               0L
   #define CAPS_TECH_VECTOR_PLOTTER        1L
   #define CAPS_TECH_RASTER_DISPLAY        2L
   #define CAPS_TECH_RASTER_PRINTER        3L
   #define CAPS_TECH_RASTER_CAMERA         4L
   #define CAPS_TECH_POSTSCRIPT            5L
   
   /* Constants for CAPS_COLOR_TABLE_SUPPORT */
   #define CAPS_COLTABL_RGB_8              1L
   #define CAPS_COLTABL_RGB_8_PLUS         2L
   #define CAPS_COLTABL_TRUE_MIX           4L
   #define CAPS_COLTABL_REALIZE            8L
   
   /* Constants for CAPS_FOREGROUND_MIX_SUPPORT */
   #define CAPS_FM_OR                      1L
   #define CAPS_FM_OVERPAINT               2L
   #define CAPS_FM_XOR                     8L
   #define CAPS_FM_LEAVEALONE             16L
   #define CAPS_FM_AND                    32L
   #define CAPS_FM_GENERAL_BOOLEAN        64L
   
   /* Constants for CAPS_BACKGROUND_MIX_SUPPORT */
   #define CAPS_BM_OR                      1L
   #define CAPS_BM_OVERPAINT               2L
   #define CAPS_BM_XOR                     8L
   #define CAPS_BM_LEAVEALONE             16L
   #define CAPS_BM_AND                    32L
   #define CAPS_BM_GENERAL_BOOLEAN        64L
   #define CAPS_BM_SRCTRANSPARENT        128L
   #define CAPS_BM_DESTTRANSPARENT       256L
   
   /* Constants for CAPS_DEVICE_WINDOWING */
   #define CAPS_DEV_WINDOWING_SUPPORT      1L
   
   /* Constants for CAPS_DEV_FONT_SIM     */
   #define CAPS_DEV_FONT_SIM_BOLD          1L
   #define CAPS_DEV_FONT_SIM_ITALIC        2L
   #define CAPS_DEV_FONT_SIM_UNDERSCORE    4L
   #define CAPS_DEV_FONT_SIM_STRIKEOUT     8L
   
   /* Constants for CAPS_ADDITIONAL_GRAPHICS */
   #define CAPS_VDD_DDB_TRANSFER            1L
   #define CAPS_GRAPHICS_KERNING_SUPPORT    2L
   #define CAPS_FONT_OUTLINE_DEFAULT        4L
   #define CAPS_FONT_IMAGE_DEFAULT          8L
   /* bits represented by values 16L and 32L are reserved */
   #define CAPS_SCALED_DEFAULT_MARKERS     64L
   #define CAPS_COLOR_CURSOR_SUPPORT      128L
   #define CAPS_PALETTE_MANAGER           256L
   #define CAPS_COSMETIC_WIDELINE_SUPPORT 512L
   #define CAPS_DIRECT_FILL              1024L
   #define CAPS_REBUILD_FILLS            2048L
   #define CAPS_CLIP_FILLS               0x00001000 /* 4096L  */
   #define CAPS_ENHANCED_FONTMETRICS     0x00002000 /* 8192L  */
   #define CAPS_TRANSFORM_SUPPORT        0x00004000 /* 16384L */
   #define CAPS_EXTERNAL_16_BITCOUNT     0x00008000 /* 32768L */
   #define CAPS_EXTERNAL_32_BITCOUNT     0x00010000 /* 65536L */


    BOOL  APIENTRY F_DevQueryCaps(HDC hdc,
                                  LONG lStart,
                                  LONG lCount,
                                  PLONG alArray);

    HMF  APIENTRY F_DevCloseDC(HDC hdc);

/* pointer data for F_DevOpenDC */

typedef PSZ *PDEVOPENDATA;

      HDC  APIENTRY F_DevOpenDC(HAB hab,
                              LONG lType,
                              PSZ pszToken,
                              LONG lCount,
                              PDEVOPENDATA pdopData,
                              HDC hdcComp);

#ifdef __cplusplus
      }
#endif

#endif /* FREEPM_DEV */
