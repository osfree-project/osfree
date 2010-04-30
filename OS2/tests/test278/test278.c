#define INCL_GPISEGEDITING      /* GPI Segment Edit functions   */

#include <os2.h>



HPS  hps;               /* presentation space handle            */

POINTL ptlStart = { 0, 0 }; /* first vertex                     */

POINTL ptlTriangle[] = { 100, 100, 200, 0, 0, 0 }; /* vertices  */



GpiOpenSegment(hps, 4L);          /* creates a segment */

GpiLabel(hps, 5L);                /* creates label 5   */

GpiLabel(hps, 10L);               /* creates label 10  */

GpiMove(hps, &ptlStart);

GpiCloseSegment(hps);

GpiPolyLine(hps, 3L, ptlTriangle);
