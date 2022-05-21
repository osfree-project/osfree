#define INCL_GPISEGEDITING      /* GPI Segment Edit functions   */

#include <os2.h>

int main(VOID)
{

HPS  hps;



GpiOpenSegment(hps, 2L);           /* open segment # 2           */

GpiDeleteElementRange(hps, 2L, 5L);/* delete elements 2 thru 5   */

GpiCloseSegment(hps);              /* close the segment          */

return 0;
}
