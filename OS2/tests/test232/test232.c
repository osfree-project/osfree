#define INCL_GPISEGMENTS        /* Segment functions            */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

POINTL ptlStart = { 0, 0 }; /* first vertex                     */

POINTL ptlTriangle[] = { 100, 100, 200, 0, 0, 0 }; /* vertices  */



GpiOpenSegment(hps, 1L);            /* open the segment          */

GpiMove(hps, &ptlStart);            /* move to start point (0,0) */

GpiPolyLine(hps, 3L, ptlTriangle);  /* draw triangle             */

GpiCloseSegment(hps);               /* close the segment         */

return 0;
}
