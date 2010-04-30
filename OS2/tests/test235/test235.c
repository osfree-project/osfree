#define INCL_GPITRANSFORMS      /* GPI Transform functions      */

#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>



MPARAM mp1;

HPS    hps;

POINTL ptl;



case WM_MOUSEMOVE:

    ptl.x = (LONG) SHORT1FROMMP(mp1);

    ptl.y = (LONG) SHORT2FROMMP(mp1);

    GpiConvert(hps, CVTC_DEVICE, CVTC_WORLD, 1L, &ptl);

    GpiMove(hps, &ptl);
