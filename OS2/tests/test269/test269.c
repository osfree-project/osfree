#define INCL_GPICONTROL         /* Control functions            */

#include <os2.h>



LONG     lOff;          /* error or offset/element number       */

HPS      hps;           /* presentation-space handle            */

LONG     plSegment;     /* Segment in which the error occurred  */

LONG     plContext;     /* Context of the error                 */

LONG     lElement;      /* element number causing error         */





lOff = GpiErrorSegmentData(hps, &plSegment, &plContext);



if (plContext == GPIE_ELEMENT)

   lElement = lOff;
