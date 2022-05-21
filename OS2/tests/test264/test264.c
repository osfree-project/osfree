#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

POINTL ptlStart = { 0, 0 }; /* first vertex                     */

POINTL ptlTriangle[] = { 100, 100, 200, 0, 0, 0 }; /* vertices  */



GpiBeginArea(hps, BA_NOBOUNDARY | BA_ALTERNATE);

GpiMove(hps, &ptlStart);

GpiPolyLine(hps, 3L, ptlTriangle);

GpiEndArea(hps);

return 0;
}
