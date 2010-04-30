#define INCL_GPISEGEDITING      /* GPI Segment Edit functions   */

#define INCL_GPISEGMENTS        /* Segment functions            */

#include <os2.h>



HPS  hps;               /* presentation space handle            */

POINTL ptlStart = { 0, 0 }; /* first vertex                     */

POINTL ptlTriangle[] = { 100, 100, 200, 0, 0, 0 }; /* vertices  */



GpiOpenSegment(hps, 4L);         /* creates a segment with labels */

GpiLabel(hps, 5L);   GpiMove(hps, &ptlStart);

GpiLabel(hps, 10L);  GpiPolyLine(hps, 3L, ptlTriangle);

GpiCloseSegment(hps);

    .

    .

    .

GpiOpenSegment(hps, 4L);

GpiSetElementPointerAtLabel(hps, 10L);/* move to label 10         */

GpiOffsetElementPointer(hps, 1L);     /* move to polyline element */
