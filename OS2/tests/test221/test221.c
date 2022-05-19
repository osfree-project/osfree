#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>

int main(VOID)
{



HPS hps;                /* presentation space handle            */

POINTL ptlStart = { 0, 0 }; /* first vertex                     */

POINTL ptlTriangle[] = { 100, 100, 200, 0, 0, 0 }; /* vertices  */



GpiMove(hps, &ptlStart);    /* move to starting point (0, 0)     */

GpiBeginArea(hps,           /* start the area bracket            */

    BA_BOUNDARY |           /* draw boundary lines               */

    BA_ALTERNATE);          /* fill interior with alternate mode */

GpiPolyLine(hps, 3L, ptlTriangle);       /* draw the triangle    */

GpiEndArea(hps);                         /* end the area bracket */

return 0;
}
