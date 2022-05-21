#define INCL_GPIBITMAPS         /* Bit map functions            */

#include <os2.h>

int main(VOID)
{

HPS    hps;             /* presentation-space handle            */

PBYTE        pb;        /* bit-map image data                   */

BITMAPINFO2  pbmi;      /* bit-map information table            */

LONG         lHits;     /* correlation/error indicator          */

LONG         lScan;     /* number of lines scanned              */

/* target and source rectangles */

POINTL       aptlPoints[4]={ 300, 400, 350, 450, 0, 0, 100, 100 };



/* scan and transfer bit map to application storage */

pbmi.cbFix = 16L;

pbmi.cPlanes = 1;

pbmi.cBitCount = 4;

lScan = GpiQueryBitmapBits(hps, 0L, 100L, pb, &pbmi);

//  .

//  .

//  .

/* draw stored rectangle bit map */

lHits = GpiDrawBits(hps, (VOID *)pb, &pbmi, 4L,

                    aptlPoints, ROP_SRCCOPY, BBO_IGNORE);

return 0;
}
