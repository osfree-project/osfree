#define INCL_GPIPATHS           /* Path functions               */

#include <os2.h>

int main(VOID)
{

HRGN   hrgn;            /* handle for region                    */

HPS    hps;             /* Presentation-space handle            */

POINTL ptlStart = { 0, 0 }; /* first vertex                     */

POINTL ptlTriangle[] = { 100, 100, 200, 0, 0, 0 }; /* vertices  */



GpiBeginPath(hps, 1L);                 /* start the path bracket */

GpiMove(hps, &ptlStart);               /* move to starting point */

GpiPolyLine(hps, 2L, ptlTriangle);     /* draw the three sides   */

GpiCloseFigure(hps);                   /* close the triangle     */

GpiEndPath(hps);                       /* end the path bracket   */



hrgn = GpiPathToRegion(hps, 1L, FPATH_WINDING);

return 0;
}
