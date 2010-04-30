#define INCL_GPIBITMAPS         /* GPI bit map functions         */

#define INCL_DOSRESOURCES       /* Dos Resource functions       */

#include <os2.h>



HPS hps;                /* presentation space handle            */

                        /* address of bit map image data in

                           resource                             */

BITMAPINFOHEADER2 bmih; /* bit map info structure               */

HBITMAP hbm;            /* bit map handle                       */



memset (&bmih,0, sizeof(BITMAPINFOHEADER2));

bmih.cbFix        = sizeof(BITMAPINFOHEADER2);

bmih.cx           = 32;

bmih.cy           = 32;

bmih.cPlanes      = 1;

bmih.cBitCount = 32*32;



hbm = GpiCreateBitmap(hps, &bmih, 0L, NULL, NULL);
