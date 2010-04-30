#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>



HPS hps;                /* presentation space handle            */

POINTL ptlStart = { 0, 0 }; /* start point                      */

POINTL aptl[3] = { 200, 100, 0, 100, 200, 0 }; /* curve points  */



GpiMove(hps, &ptlStart);        /* move to the lower-left corner */

GpiPolyFillet(hps, 3L, aptl);   /* draw the curve                */
