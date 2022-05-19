#define INCL_GPISEGEDITING      /* GPI Segment Edit functions   */

#include <os2.h>

int main(VOID)
{

HPS  hps;

POINTL ptlStart = { 0, 0 }; /* first vertex                     */

POINTL ptlTriangle[] = { 100, 100, 200, 0, 0, 0 }; /* vertices  */



GpiBeginElement(hps,               /* start element bracket      */

    1L,                            /* element type is 1          */

    "Triangle");                   /* element description        */

GpiMove(hps, &ptlStart);           /* move to start point (0, 0) */

GpiPolyLine(hps, 3L, ptlTriangle); /* draw triangle              */

GpiEndElement(hps);                /* end element bracket        */

return 0;
}
