#define INCL_GPIREGIONS         /* Region functions             */

#include <os2.h>



LONG   lInside;         /* inside/error indicator               */

HPS    hps;             /* Presentation-space handle            */

HRGN   hrgn;            /* handle for region                    */

POINTL pptlPoint = {150L,150L};/* point to be checked           */

RECTL arcl[3] = { 100, 100, 200, 200,        /* 1st rectangle   */

                  150, 150, 250, 250,        /* 2nd rectangle   */

                  200, 200, 300, 300 };      /* 3rd rectangle   */



/* create a region comprising three rectangles */

hrgn = GpiCreateRegion(hps, 3L, arcl);



lInside = GpiPtInRegion(hps, hrgn, &pptlPoint);
