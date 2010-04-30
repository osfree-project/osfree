#define INCL_GPIREGIONS         /* Region functions             */

#include <os2.h>



HPS hps;                /* presentation space handle            */

HRGN hrgn;                /* handle for region */

RECTL arcl[3] = { 100, 100, 200, 200,    /* 1st rectangle     */

    150, 150, 250, 250,                  /* 2nd rectangle     */

    200, 200, 300, 300 };                /* 3rd rectangle     */



hrgn = GpiCreateRegion(hps, 3L, arcl);

GpiSetColor(hps, CLR_RED);

GpiSetPattern(hps, PATSYM_DIAG1);

GpiPaintRegion(hps, hrgn);
