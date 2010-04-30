#define INCL_GPISEGEDITING      /* GPI Segment Edit functions   */

#include <os2.h>



HPS  hps;



GpiOpenSegment(hps, 2L);            /* open segment #2   */



/* delete elements between 1 and 2 */



GpiDeleteElementsBetweenLabels(hps, 1L, 2L);

GpiCloseSegment(hps);               /* close the segment */
