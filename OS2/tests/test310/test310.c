#define INCL_GPIPRIMITIVES      /* Primitive functions          */

#include <os2.h>

int main(VOID)
{

LONG   lColor;          /* current background color (or error)  */

HPS    hps;             /* Presentation-space handle            */



lColor = GpiQueryBackColor(hps);

return 0;
}
