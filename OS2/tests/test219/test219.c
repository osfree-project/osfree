#define INCL_GPILOGCOLORTABLE   /* Color Table functions        */

#include <os2.h>



LONG  lremapColors;     /* number of remapped colors            */

HPAL  hpal;             /* palette handle                       */



/*****************************************************************

 * assume 4 entries in palette.                                  *

 * The RGB values are calculated with the following formula:     *

 *    (F * 16777216) + (R * 65536) + (G * 256) + B               *

 *    where F = flag, PC_RESERVED or PC_EXPLICIT                 *

 *          R = red intensity value                              *

 *          G = green intensity value                            *

 *          B = blue intensity value                             *

 * Thus, in the following table, red and green intensities are 0 *

 * while the blue intensity increases from 1 to 4.               *

 *****************************************************************/



ULONG aulTable[4]=

          {(PC_RESERVED*16777216) + (0*65536) + (0*256) + 1,

           (PC_RESERVED*16777216) + (0*65536) + (0*256) + 2,

           (PC_RESERVED*16777216) + (0*65536) + (0*256) + 3,

           (PC_RESERVED*16777216) + (0*65536) + (0*256) + 4};





lremapColors = GpiAnimatePalette(hpal, LCOLF_CONSECRGB, 0L, 4L,

                                 aulTable);
