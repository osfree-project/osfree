#define INCL_GPICONTROL         /* GPI control Functions        */

#include <os2.h>



HPS hps;                /* presentation space handle            */

HDC hdcPrinter;         /* device context handle                */



/* release the current device context */

GpiAssociate(hps, NULLHANDLE);

/* associate a printer device context */

GpiAssociate(hps, hdcPrinter);
