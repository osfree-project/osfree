#define INCL_GPIREGIONS         /* Region functions             */

#include <os2.h>

int main(VOID)
{

LONG     lHits;         /* correlation/error indicator          */

HPS      hps;           /* presentation-space handle            */

HRGN     hrgn;          /* handle for region                    */

SIZEL    psizlThickness = {5L,5L};

                        /* Thickness of frame                   */

RECTL arcl[3] = { 100, 100, 200, 200,        /* 1st rectangle   */

                  150, 150, 250, 250,        /* 2nd rectangle   */

                  200, 200, 300, 300 };      /* 3rd rectangle   */



/* create a region comprising three rectangles */

hrgn = GpiCreateRegion(hps, 3L, arcl);



lHits = GpiFrameRegion(hps, hrgn, &psizlThickness);

return 0;
}
