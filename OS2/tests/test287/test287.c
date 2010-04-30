#define INCL_GPIREGIONS         /* Region functions             */

#include <os2.h>



LONG  lComplexity;      /* clipping complexity/error return     */

HPS   hps;              /* Presentation-space handle            */

POINTL pptlPoint = {3,3}; /* displacement                       */



lComplexity = GpiOffsetClipRegion(hps, &pptlPoint);
