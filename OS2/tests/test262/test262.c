#define INCL_GPISEGMENTS        /* Segment functions            */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

POINTL ptlStart = { 0, 0 }; /* first vertex                     */

POINTL ptlTriangle[] = { 100, 100, 200, 0, 0, 0 }; /* vertices  */



GpiOpenSegment(hps, 4L);           /* open the segment           */

GpiMove(hps, &ptlStart);           /* move to start point (0, 0) */

GpiPolyLine(hps, 3L, ptlTriangle); /* draw triangle              */

GpiCloseSegment(hps);              /* close the segment          */

//    .

//    .

//    .

GpiDrawSegment(hps, 4L);           /* draw segment #4            */

return 0;

}
