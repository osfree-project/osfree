#define INCL_GPIREGIONS         /* Region functions             */

#include <os2.h>



HPS hps;                /* presentation space handle            */

HRGN hrgn1, hrgn2, hrgn3;

RECTL rclRect1 = { 0, 0, 100, 100 };

RECTL rclRect2 = { 50, 50, 200, 200 };



/* create first region  */

hrgn1 = GpiCreateRegion(hps, 1L, &rclRect1);

/* create second region */

hrgn2 = GpiCreateRegion(hps, 1L, &rclRect2);

/* create empty region  */

hrgn3 = GpiCreateRegion(hps, 0L, NULL);



/* Combine first and second regions, replacing the empty region. */



GpiCombineRegion(hps, hrgn3, hrgn1, hrgn2, CRGN_XOR);

