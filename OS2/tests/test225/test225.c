#define INCL_GPITRANSFORMS      /* GPI Transform functions      */

#define INCL_GPISEGMENTS        /* Segment functions            */

#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>



HPS    hps;

USHORT i;

POINTL ptlStart = { 0, 0 }; /* first vertex                     */

POINTL ptlTriangle[] = { 100, 100, 200, 0, 0, 0 }; /* vertices  */

MATRIXLF matlfInstance = { MAKEFIXED(1, 0),  MAKEFIXED(0, 0), 0,

                           MAKEFIXED(0, 0),  MAKEFIXED(1, 0), 0,

                           0,                0,               1 };



GpiOpenSegment(hps, 1L);             /* opens segment               */

GpiMove(hps, &ptlStart);             /* moves to start point (0, 0) */

GpiPolyLine(hps, 3L, ptlTriangle);   /* draws triangle              */

GpiCloseSegment(hps);                /* closes segment              */



for (i = 0; i < 3; i++)

    {

    /*

     * Draw the segment after adding the matrix to the model

     * transformation.

     */



    GpiCallSegmentMatrix(hps, 1L, 9, &matlfInstance, TRANSFORM_ADD);

    matlfInstance.fxM11 *= 2;

    matlfInstance.fxM22 *= 2;

    }
