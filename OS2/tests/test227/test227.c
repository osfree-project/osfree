#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

POINTL ptlStart;        /* beginning of string                  */



ptlStart.x = 100L;

ptlStart.y = 100L;



/* Start string at (100, 100). */



GpiMove(hps, &ptlStart);



/* Draw the 5-character string. */



GpiCharString(hps, 5L, "Hello");

return 0;
}
