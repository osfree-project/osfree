#define INCL_GPIBITMAPS         /* Bit map functions            */

#include <os2.h>



LONG  lHits;            /* correlation/error indicator          */

HPS   hps;              /* Presentation-space handle            */

LONG  lOptions;         /* flood fill options                   */

LONG  lColor;           /* color                                */



/* fill up to the boundaries of the color */

lOptions = FF_BOUNDARY;



/* use color corresponding to index 1 */

lColor = 1;



lHits = GpiFloodFill(hps, lOptions, lColor);
