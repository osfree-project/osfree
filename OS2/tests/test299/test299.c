#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>



HPS hps;                           /* presentation space handle*/

POINTL ptlTriangle[] = { 100, 100, 200, 0, 0, 0 };

                                   /* vertices  */

GpiMove(hps, &ptlTriangle[2]);

                                   /* moves to end point (0, 0)*/

GpiPolyLine(hps, 3L, ptlTriangle); /* draws triangle   */
