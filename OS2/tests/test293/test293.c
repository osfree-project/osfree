#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>



HPS hps;                /* presentation space handle            */

POINTL ptl = { 100, 100 };             /* center point for arc */



GpiSetLineType(hps, LINETYPE_INVISIBLE);

GpiPartialArc(hps, &ptl, MAKEFIXED(50, 0), MAKEFIXED(0, 0),

    MAKEFIXED(180, 0));

GpiSetLineType(hps, LINETYPE_SOLID);

GpiPartialArc(hps, &ptl, MAKEFIXED(50, 0), MAKEFIXED(0, 0),

    MAKEFIXED(180, 0));
