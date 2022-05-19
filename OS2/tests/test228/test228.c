#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

POINTL ptlStart;



ptlStart.x = 100L;

ptlStart.y = 100L;



/* Draw the string "Hello" at (100, 100). */



GpiCharStringAt(hps, &ptlStart, 5, "Hello");



/* These two calls are identical to the one above. */



GpiMove(hps, &ptlStart);

GpiCharString(hps, 5L, "Hello");

return 0;
}
