#define INCL_GPIPRIMITIVES      /* GPI Primitive functions      */

#include <os2.h>



LONG  lHits;            /* correlation/error indicator          */

HPS   hps;              /* Presentation-space handle            */

POINTL  pptlStart = {10L,10L};

                        /* Starting position */

RECTL  rclRect = {0L,0L,100L,100L};

                        /* Rectangle structure                  */

ULONG  flOptions;       /* Formatting options                   */

LONG  lCount;           /* Number of bytes in the string        */

char  pchString[14];    /* Characters to be drawn               */



flOptions = CHS_CLIP; /* clip text to rectangle */

lCount = 13;

strcpy(pchString,"13 characters");



lHits = GpiCharStringPosAt(hps, &pptlStart, &rclRect, flOptions,

                          lCount, pchString, NULL);
