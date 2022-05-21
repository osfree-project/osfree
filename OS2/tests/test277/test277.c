#define INCL_GPIREGIONS         /* Region functions             */

#include <os2.h>

int main(VOID)
{

LONG  lComplexity;      /* clipping complexity/error return     */

HPS   hps;              /* Presentation-space handle            */

RECTL prclRectangle = {100,100,200,200}; /* intersect rectangle */



lComplexity = GpiIntersectClipRectangle(hps, &prclRectangle);

return 0;
}
