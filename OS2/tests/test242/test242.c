#define INCL_GPILOGCOLORTABLE   /* Color Table functions        */

#include <os2.h>



HPS hps;                /* presentation space handle            */

LONG alTable[16];       /* assume 16 entries                    */



/* retrieve the current table */



GpiQueryLogColorTable(hps, 0L, 0L, 16L, alTable);



alTable[1] = 0x000080; /* change the second entry to light blue */



GpiCreateLogColorTable(hps,         /* presentation space       */

    0L,                             /* no special options       */

    LCOLF_CONSECRGB,                /* consecutive RGB values   */

    0L,                             /* start with color index 0 */

    16,                             /* 16 entries               */

    alTable);                       /* RGB color values         */
