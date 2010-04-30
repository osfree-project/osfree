#define INCL_GPIPRIMITIVES      /* Primitive functions          */

#include <os2.h>



LONG   lMode;           /* current attribute mode (or error)    */

HPS    hps;             /* Presentation-space handle            */



/* query current attribute mode */

lMode = GpiQueryAttrMode(hps);



/* set new mode */

GpiSetAttrMode(hps, AM_PRESERVE);

 .

 .

/* restore original mode */

GpiSetAttrMode(hps, lMode);
