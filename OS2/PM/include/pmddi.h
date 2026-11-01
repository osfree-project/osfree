#define DDI_INCLUDED

#ifdef INCL_GREALL
   #define INCL_GRE_ARCS
   #define INCL_GRE_BITMAPS
   #define INCL_GRE_CLIP
   #define INCL_GRE_COLORTABLE
   #define INCL_GRE_DCS
   #define INCL_GRE_PAX
   #define INCL_GRE_DEVICE
   #define INCL_GRE_DEVMISC
   #define INCL_GRE_DEVSUPPORT
   #define INCL_GRE_FONTS
   #define INCL_GRE_INKPATH
   #define INCL_GRE_JOURNALING
   #define INCL_GRE_LCID
   #define INCL_GRE_LINES
   #define INCL_GRE_MARKERS
   #define INCL_GRE_PALETTE
   #define INCL_GRE_PATHS
   #define INCL_GRE_PICK
   #define INCL_GRE_POLYGON
   #define INCL_GRE_REGIONS
   #define INCL_GRE_SCANS
   #define INCL_GRE_SCREEN
   #define INCL_GRE_SETID
   #define INCL_GRE_STRINGS
   #define INCL_GRE_XFORMS
#endif /* INCL_GREALL */

#ifdef INCL_GRE_ARCS

typedef LONG PHID;

typedef struct _BOXPARAMS
{
  POINTL ptl;
  SIZEL  sizl;
} BOXPARAMS, *PBOXPARAMS;

#endif /* INCL_GRE_ARCS */

#define CHS_START_XY                    0x00000020L
#define CHS_ATTR_INFO                   0x00000040L

typedef struct _CSP_INFO
{
  LONG  cSize;
  LONG  lColor;
  LONG  lBackColor;
} CSP_INFO, *PCSP_INFO;

/*!
   @brief GRE transform matrix (XFORM).

   Layout matches the OS/2 GRE XFORM structure used by
   GreGetModelXform / GreSetModelXform. 9 elements: fxM11..lM33.
*/
typedef struct _XFORM {
    FIXED fxM11;
    FIXED fxM12;
    LONG  lM13;
    FIXED fxM21;
    FIXED fxM22;
    LONG  lM23;
    LONG  lM31;
    LONG  lM32;
    LONG  lM33;
} XFORM, *PXFORM;


#include <pmddim.h>

