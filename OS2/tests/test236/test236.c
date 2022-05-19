#define INCL_GPITRANSFORMS      /* GPI Transform functions      */

#include <os2.h>

int main(VOID)
{

BOOL  fSuccess;         /* success indicator                    */

HPS  hps;               /* Presentation-space handle            */

LONG  lCountp;          /* Point count                          */

POINTL  aptlPoints[2] = {{0L,0L},{1L,1L}};

                        /* Array of (x,y) coordinate pair

                           structures                           */

LONG  lCount;           /* Number of elements                   */

MATRIXLF  pmatlfArray;  /* Instance transform matrix            */



lCount = 1; /* examine only first element of transform matrix */



pmatlfArray.fxM11 = 2; /* set first element of transform matrix */



fSuccess = GpiConvertWithMatrix(hps, lCountp, aptlPoints,

                                lCount, &pmatlfArray);

return 0;
}
