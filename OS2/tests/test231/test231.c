#define INCL_GPIPATHS           /* GPI Path functions           */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

POINTL ptlStart = { 0, 0 };

POINTL ptlPoints[] = { 100, 100, 200, 0 };



GpiBeginPath(hps, 1L);                 /* start the path bracket */

GpiMove(hps, &ptlStart);               /* move to starting point */

GpiPolyLine(hps, 2L, ptlPoints);       /* draw two sides         */

GpiCloseFigure(hps);                   /* close the triangle     */

GpiEndPath(hps);                       /* end the path bracket   */

return 0;
}
