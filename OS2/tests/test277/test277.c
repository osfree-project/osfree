#define INCL_GPIREGIONS         /* Region functions             */

#include <os2.h>



LONG  lComplexity;      /* clipping complexity/error return     */

HPS   hps;              /* Presentation-space handle            */

RECTL prclRectangle = {100,100,200,200}; /* intersect rectangle */



lComplexity = GpiIntersectClipRectangle(hps, &prclRectangle);
