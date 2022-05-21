#define INCL_GPIREGIONS         /* Region functions             */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

HRGN hrgn;

RECTL arcl[3] = { 10,10,20,20,15,15,25,25,20,20,30,30 };



hrgn = GpiCreateRegion(hps, 3L, arcl); /* use 3 rectangles   */

GpiPaintRegion(hps, hrgn);             /* paint the region   */

GpiDestroyRegion(hps, hrgn);           /* destroy the region */

return 0;
}
