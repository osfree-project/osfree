#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>



HPS hps;                /* presentation space handle            */

/* point array */

POINTL ptl[4] = { 0, 0, 100, 100, 0, 100, 100, 0 };



GpiMove(hps, &ptl[0]);

GpiLine(hps, &ptl[1]);

GpiMove(hps, &ptl[2]);

GpiLine(hps, &ptl[3]);
