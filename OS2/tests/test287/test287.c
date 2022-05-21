#define INCL_GPIREGIONS         /* Region functions             */

#include <os2.h>

int main(VOID)
{

LONG  lComplexity;      /* clipping complexity/error return     */

HPS   hps;              /* Presentation-space handle            */

POINTL pptlPoint = {3,3}; /* displacement                       */



lComplexity = GpiOffsetClipRegion(hps, &pptlPoint);

return 0;
}
