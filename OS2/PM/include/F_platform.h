/* F_platform.h */
/* Common platform abstraction for FreePM: PMGPI client + mini server.
 * Derived from gpi_platform.h; only the missing server-side bits
 * (bundles, masks, extra Dos* emulation, extra Win32 API imports)
 * are added. Everything else kept verbatim. */

#if defined(__OS2__) || defined(__OS2V2__)

#ifdef __WATCOMC__

#ifdef INCL_DDIDEFS /* if only DDI required then enable DDI part of GPI */
   #define INCL_GPIBITMAPS
   #define INCL_GPIERRORS
   #define INCL_GPILCIDS
   #define INCL_GPILOGCOLORTABLE
   #define INCL_GPIPATHS
   #define INCL_GPIPOLYGON
   #define INCL_GPIPRIMITIVES
   #define INCL_GPIREGIONS
   #define INCL_GPITRANSFORMS
#endif /* INCL_DDIDEFS */

#endif

#include <os2.h>

#define EXTERN extern "C"

#elif defined(__WIN32__) || defined(_WIN32)

#include <F_winos2.h>

#else
  #error Unsupported platform for PMGPI
#endif


#ifdef __cplusplus
      extern "C" {
#endif

#define SEGEM_INSERT   1L
#define SEGEM_REPLACE  2L

#ifdef __cplusplus
BOOL APIENTRY GpiBeginElement(HPS, LONG, PCSZ);
#else
BOOL APIENTRY GpiBeginElement(HPS, LONG, PSZ);
#endif

BOOL APIENTRY GpiSetEditMode(HPS, LONG);
LONG APIENTRY GpiQueryEditMode(HPS);
BOOL APIENTRY GpiSetElementPointer(HPS, LONG);
LONG APIENTRY GpiQueryElementPointer(HPS);
BOOL APIENTRY GpiOffsetElementPointer(HPS, LONG);
BOOL APIENTRY GpiDeleteElement(HPS);
BOOL APIENTRY GpiDeleteElementRange(HPS, LONG, LONG);
BOOL APIENTRY GpiLabel(HPS, LONG);
BOOL APIENTRY GpiSetElementPointerAtLabel(HPS, LONG);
BOOL APIENTRY GpiDeleteElementsBetweenLabels(HPS, LONG, LONG);

#ifdef __cplusplus
LONG APIENTRY GpiQueryElementType(HPS, PLONG, LONG, PCSZ);
#else
LONG APIENTRY GpiQueryElementType(HPS, PLONG, LONG, PSZ);
#endif

LONG APIENTRY GpiQueryElement(HPS, LONG, LONG, PBYTE);

#ifdef __cplusplus
LONG APIENTRY GpiElement(HPS, LONG, PCSZ, LONG, PBYTE);
#else
LONG APIENTRY GpiElement(HPS, LONG, PSZ, LONG, PBYTE);
#endif

BOOL APIENTRY GpiEndElement(HPS);

#ifdef INCL_GPIPRIMITIVES
BOOL APIENTRY GpiQueryArcParams(HPS, PARCPARAMS);
#endif

#ifdef __cplusplus
  }
#endif


