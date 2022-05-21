#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

POINTL ptlTriangle[] = { 0, 0, 100, 100, 200, 0 };



GpiMove(hps, &ptlTriangle[0]);    /* moves to start point (0, 0)*/

GpiPointArc(hps, &ptlTriangle[1]);/* draws the arc              */

GpiMove(hps, &ptlTriangle[0]);    /* moves to start point (0, 0)*/

/* draws the triangle */

GpiPolyLine(hps, 3L, &ptlTriangle[1]);

return 0;
}
