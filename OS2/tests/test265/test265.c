#define INCL_GPISEGEDITING      /* GPI Segment Edit functions   */

#include <os2.h>



HPS  hps;               /* presentation space handle            */

POINTL ptlStart = { 0, 0 }; /* first vertex                     */

POINTL ptlTriangle[] = { 100, 100, 200, 0, 0, 0 }; /* vertices  */

    .

    .

    .

/* begin the element bracket */

GpiBeginElement(hps, 1L, "Triangle");

GpiMove(hps, &ptlStart);           /* move to start point (0, 0) */

GpiPolyLine(hps, 3L, ptlTriangle); /* draw triangle              */

GpiEndElement(hps);                /* end element bracket        */
