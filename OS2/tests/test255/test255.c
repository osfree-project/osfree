#define INCL_GPILCIDS           /* Font functions               */

#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>



HPS hps;                /* presentation space handle            */

FATTRS fat;



/* create and set the font */



GpiCreateLogFont(hps, NULL, 1L, &fat);

GpiSetCharSet(hps, 1L);

    .

    .

    .

GpiSetCharSet(hps, 0L);          /* release the font before deleting */

GpiDeleteSetId(hps, 1L);         /* delete the logical font          */
