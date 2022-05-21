#define INCL_GPIREGIONS         /* Region functions             */

#include <os2.h>

int main(VOID)
{

LONG     lEquality;     /* equality/error indicator             */

HPS      hps;           /* presentation-space handle            */

HRGN     hrgnSrc1;      /* handle for first region              */

HRGN     hrgnSrc2;      /* handle for second region             */

RECTL arcl[3] = { 100, 100, 200, 200,        /* 1st rectangle   */

                  150, 150, 250, 250,        /* 2nd rectangle   */

                  200, 200, 300, 300 };      /* 3rd rectangle   */



/* create two identical regions comprising three rectangles each*/

hrgnSrc1 = GpiCreateRegion(hps, 3L, arcl);

hrgnSrc2 = GpiCreateRegion(hps, 3L, arcl);



lEquality = GpiEqualRegion(hps, hrgnSrc1, hrgnSrc2);

return 0;
}
