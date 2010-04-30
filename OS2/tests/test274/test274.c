#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>



HPS hps;                /* presentation space handle            */

SHORT i;                /* loop variable                        */

ARCPARAMS arcp = { 1, 1, 0, 0 }; /* arc parameters structure    */



GpiSetArcParams(hps, &arcp);



for (i = 5; i > 0; i--)

    GpiFullArc(hps,          /* presentation-space handle       */

        DRO_OUTLINE,         /* outline                         */

        MAKEFIXED(i, 0));    /* converts integer to fixed point */
