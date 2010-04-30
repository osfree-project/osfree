#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>



HPS hps;                /* presentation space handle            */

POINTL ptl = { 10, 10 }; /* marker point                        */



GpiMarker(hps, &ptl);
