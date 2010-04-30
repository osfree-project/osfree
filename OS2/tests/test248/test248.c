#define INCL_GPISEGEDITING      /* GPI Segment Edit functions   */

#include <os2.h>



HPS  hps;



GpiOpenSegment(hps, 2L);           /* open segment #2       */

GpiSetElementPointer(hps, 3L);     /* move to third element */

GpiDeleteElement(hps);             /* delete element        */

GpiCloseSegment(hps);              /* close the segment     */
