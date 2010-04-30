#define INCL_GPISEGMENTS        /* Segment functions            */

#include <os2.h>



HPS hps;                /* presentation space handle            */



GpiOpenSegment(hps, 4L);               /* open segment 4        */

    .

GpiCloseSegment(hps);                  /* close the segment     */

GpiOpenSegment(hps, 5L);               /* open segment 5        */

    .

GpiCloseSegment(hps);                  /* close the segment     */

GpiOpenSegment(hps, 6L);               /* open segment 6        */

    .

GpiCloseSegment(hps);                  /* close the segment     */

    .

    .

    .

GpiDeleteSegments(hps, 4L, 6L);  /* delete segments 4 through 6 */
