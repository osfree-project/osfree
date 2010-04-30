#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#define INCL_GPIREGIONS         /* GPI region functions         */

#include <os2.h>



HPS hps;                /* presentation space handle            */

HRGN hrgn;              /* region handle                        */



/* preserves attributes on stack */

GpiSetAttrMode(hps, AM_PRESERVE);

    .

    .

    .

GpiSetColor(hps, CLR_RED);        /* sets color to red          */

GpiSetPattern(hps, PATSYM_DIAG1); /* sets pattern to a diagonal */

GpiPaintRegion(hps, hrgn);

GpiPop(hps, 2L);  /* restores values of last two attributes set */
