#define INCL_GPIPRIMITIVES      /* Primitive functions          */

#include <os2.h>

int main(VOID)
{

BOOL  fSuccess;         /* success indicator                    */

HPS    hps;             /* Presentation-space handle            */

ARCPARAMS  parcpArcParams; /* Arc parameters                    */

LONG  lPcoefficient;    /* p coefficient of arc definition      */



fSuccess = GpiQueryArcParams(hps, &parcpArcParams);



/* if successful, assign value of P coefficient */

if (fSuccess == TRUE)

   lPcoefficient = parcpArcParams.lP;

return 0;
}
