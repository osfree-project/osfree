#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

POINTL ptl = { 10, 10 }; /* marker point                        */



GpiMarker(hps, &ptl);

return 0;

}
