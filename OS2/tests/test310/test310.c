#define INCL_GPIPRIMITIVES      /* Primitive functions          */

#include <os2.h>



LONG   lColor;          /* current background color (or error)  */

HPS    hps;             /* Presentation-space handle            */



lColor = GpiQueryBackColor(hps);
