#define INCL_GPIBITMAPS         /* GPI Bit map functions        */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

HBITMAP hbm, hbmPrevious;



hbm = GpiLoadBitmap(hps, 0L, 1, 0L, 0L);  /* load the bit map   */

hbmPrevious = GpiSetBitmap(hps, hbm);     /* set bit map for PS */



/* bit map displayed with GpiBitBlt */



GpiSetBitmap(hps, hbmPrevious);      /* release bit map from PS */

GpiDeleteBitmap(hbm);                /* delete the bit map      */

return 0;

}
