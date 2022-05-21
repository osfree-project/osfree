#define INCL_GPISEGMENTS        /* Segment functions            */

#include <os2.h>

int main(VOID)
{

BOOL     fSuccess;      /* success indicator                    */

HPS      hps;           /* presentation-space handle            */



/* The chaining attribute is switched on */

GpiSetInitialSegmentAttrs(hps, ATTR_CHAINED | ATTR_DYNAMIC,

                          ATTR_ON);



/* two dynamic chained segments are defined */

GpiOpenSegment(hps, 1L);

// .

// .

GpiCloseSegment(hps);



GpiOpenSegment(hps, 2L);

// .

// .

GpiCloseSegment(hps);



/* draw the dynamic segment chain */

fSuccess = GpiDrawDynamics(hps);

return 0;
}
