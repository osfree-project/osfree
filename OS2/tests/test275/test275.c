#define INCL_GPISEGMENTS      /* Segment functions               */

#include <os2.h>

int main(VOID)
{

HPS hps;                         /* presentation space handle       */

LONG fFormat = DFORM_NOCONV;  /* does not convert coordinates    */

LONG offSegment = 0L;            /* offset in segment               */

LONG offNextElement = 0L;        /* offset in segment to next element */

LONG cb = 0L;                    /* bytes retrieved          */

BYTE abBuffer[512];    /* data buffer                           */



GpiOpenSegment(hps, 3L);         /* opens segment to receive data   */

do {

    offSegment += cb;

    offNextElement = offSegment;

    cb = GpiGetData(hps, 2L, &offNextElement, fFormat, 512L,

                    abBuffer);



    /* Put data in other segment. */



    if (cb > 0L) GpiPutData(hps, /* presentation-space handle   */

        fFormat,                 /* format of coordinates       */

        &cb,                 /* number of bytes in buffer   */

        abBuffer);               /* buffer with graphics-order data */



} while (cb > 0);

GpiCloseSegment(hps);            /* closes segment that received data */

return 0;
}
