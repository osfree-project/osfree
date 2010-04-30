#define INCL_GPISEGMENTS        /* Segment functions            */

#include <os2.h>



HPS hps;                /* presentation space handle            */

POINTL ptlStart = { 0, 0 }; /* first vertex                     */

POINTL ptlTriangle[] = { 100, 100, 200, 0, 0, 0 }; /* vertices  */



GpiOpenSegment(hps, 1L);       /* opens the segment             */

GpiMove(hps, &ptlStart);       /* moves to starting point (0,0) */

GpiPolyLine(hps, 3L, ptlTriangle);/* draws triangle             */

GpiCloseSegment(hps);          /* closes the segment            */



GpiDrawSegment(hps, 1L);
