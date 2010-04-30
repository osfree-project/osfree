#define INCL_GPIPRIMITIVES      /* Primitive functions          */

#include <os2.h>



LONG   lVisibility;     /* visibility indicator                 */

HPS    hps;             /* Presentation-space handle            */

POINTL pptlPoint = {150L,150L};/* point to be checked           */



lVisibility = GpiPtVisible(hps, &pptlPoint);
