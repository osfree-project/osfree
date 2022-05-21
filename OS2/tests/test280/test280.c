#define INCL_GPIBITMAPS         /* GPI bit map functions         */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

HBITMAP hbm, hbmPrevious;

#define BITMAP_ID 1



/* load the bit map from the EXE */

hbm = GpiLoadBitmap(hps, NULLHANDLE, BITMAP_ID, 100L, 100L);

hbmPrevious = GpiSetBitmap(hps, hbm);  /* select bit map for PS */



/* bit map displayed with GpiBitBlt */



GpiSetBitmap(hps, hbmPrevious);      /* release bit map from PS */

GpiDeleteBitmap(hbm);                /* delete the bit map      */

return 0;
}
