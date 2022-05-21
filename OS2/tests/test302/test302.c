#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

POINTL ptlStart = { 0, 0 }; /* start point                      */

POINTL aptl[3] = { 0, 100, 200, 150, 200, 50 }; /* point array  */



GpiMove(hps, &ptlStart);         /* moves to start point       */

GpiPolySpline(hps,               /* presentation-space handle  */

    3L,                          /* 3 points in the array      */

    aptl);                       /* address of array of points */

return 0;
}
