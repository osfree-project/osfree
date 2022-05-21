#define INCL_GPICONTROL         /* GPI control Functions        */

#include <os2.h>

int main(VOID)
{

HAB hab;                /* anchor block handle                  */

HDC hdc;                /* device context handle                */

HPS hps;                /* presentation space handle            */

SIZEL sizl = { 0, 0 };  /* use same page size as device         */

/**************************

 * context data structure *

 **************************/

DEVOPENSTRUC dop = {0L, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};



/* create memory device context */

hdc = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);



/* Create the presentation and associate the memory device

   context. */

hps = GpiCreatePS(hab, hdc, &sizl, PU_PELS |

                                   GPIT_MICRO | GPIA_ASSOC);

return 0;
}
