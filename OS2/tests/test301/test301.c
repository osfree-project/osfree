#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

POINTL ptlStart = { 0, 0 }; /* start point                      */

POINTL aptl[5]={10, 8, 20, 17, 30, 28, 40, 51, 50, 46};/* points*/



GpiPolyMarker(hps, 5L, aptl);

GpiMove(hps, &ptlStart);

GpiPolyLine(hps, 5L, aptl);

return 0;
}
