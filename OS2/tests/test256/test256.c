#define INCL_GPICONTROL         /* GPI control Functions        */

#define INCL_DEV                /* Device Function definitions  */

#include <os2.h>

int main(VOID)
{

HAB          hab;       /* Anchor-block handle                  */

HPS          hps;       /* Target presentation-space handle     */

HDC          hdc;       /* Device-context handle                */

DEVOPENSTRUC dop;       /* context data structure               */

SIZEL page = { 0, 0 };  /* page size (use same as device)       */



/* Create the memory device context and presentation space. */

hdc = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop, NULLHANDLE);

hps = GpiCreatePS(hab, hdc, &page, PU_PELS|GPIT_MICRO|GPIA_ASSOC);

//    .

//    .

//    .

GpiAssociate(hps, NULLHANDLE); /* disassociate device context */

GpiDestroyPS(hps);       /* destroys presentation space */

DevCloseDC(hdc);         /* closes device context       */

return 0;
}
