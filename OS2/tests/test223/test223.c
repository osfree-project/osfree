#define INCL_GPIPATHS           /* GPI Path functions           */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

POINTL ptlStart = { 0, 0 }; /* first vertex                     */

POINTL ptlTriangle[] = { 100, 100, 200, 0, 0, 0 }; /* vertices  */



GpiBeginPath(hps, 1L);                 /* start the path bracket */

GpiMove(hps, &ptlStart);               /* move to starting point */

GpiPolyLine(hps, 2L, ptlTriangle);     /* draw two sides         */

GpiCloseFigure(hps);                   /* close the triangle     */

GpiEndPath(hps);                       /* end the path bracket   */

GpiFillPath(hps, 1L, FPATH_ALTERNATE); /* draw and fill the path */

return 0;
}
