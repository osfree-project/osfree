#define INCL_GPISEGMENTS        /* Segment functions            */

#include <os2.h>



BOOL     fSuccess;      /* success indicator                    */

HPS      hps;           /* presentation-space handle            */



/* The chaining attribute is switched on */

GpiSetInitialSegmentAttrs(hps, ATTR_CHAINED, ATTR_ON);



/* two chained segments are defined */

GpiOpenSegment(hps, 1L);

 .

 .

GpiCloseSegment(hps);



GpiOpenSegment(hps, 2L);

 .

 .

GpiCloseSegment(hps);



/* draw the segment chain */

fSuccess = GpiDrawChain(hps);
