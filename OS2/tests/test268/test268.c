#define INCL_GPICONTROL         /* GPI control Functions        */

#include <os2.h>



HPS hps;                /* presentation space handle            */

POINTL ptlStart = { 0, 0 }; /* start point                      */

POINTL ptlTriangle[] = { 100, 100, 200, 0, 0, 0 }; /* vertices  */



GpiErase(hps);                        /* clear the display */

GpiMove(hps, &ptlStart);              /* draw a triangle   */

GpiPolyLine(hps, 3L, ptlTriangle);
