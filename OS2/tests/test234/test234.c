#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#define INCL_GPISEGMENTS        /* Segment functions            */

#include <os2.h>



HPS hps;                /* presentation space handle            */

POINTL ptlStart = { 0, 0 }; /* first vertex                     */

POINTL ptlTriangle[] = { 100, 100, 200, 0, 0, 0 }; /* vertices  */



GpiOpenSegment(hps, 0L);                   /* open the segment  */

GpiComment(hps, 18L, "Start point (0, 0)");

GpiMove(hps, &ptlStart);

GpiComment(hps, 13L, "Draw triangle");

GpiPolyLine(hps, 3L, ptlTriangle);

GpiCloseSegment(hps);                      /* close the segment */
