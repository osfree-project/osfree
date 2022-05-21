#define INCL_GPICORRELATION     /* GPI Correlation functions    */

#include <os2.h>

int main(VOID)
{

BOOL     fSuccess;      /* success indicator                    */

SIZEL    psizlSize={0L,0L}; /* size of pick aperture            */

LONG     lNumHits;      /* number of hits or error              */

HPS      hps;           /* Presentation-space handle            */

POINTL   pptlPick = {200L,200L};

                        /* Pick (center of aperture) position   */

LONG     lMaxHits;      /* Maximum hits to be returned          */

LONG     lMaxDepth;     /* Number of pairs to be returned       */

LONG     alSegTag;      /* Segment identifiers and tags         */



fSuccess = GpiSetPickAperturePosition(hps, &pptlPick);



/* set aperture size (use default) */

fSuccess = GpiSetPickApertureSize(hps, PICKAP_DEFAULT, &psizlSize);



/* return only one hit */

lMaxHits = 1L;



/* return only one segment/tag pair per hit */

lMaxDepth = 1L;



/* correlate on visible, detectable segment chains */

lNumHits = GpiCorrelateChain(hps, PICKSEL_VISIBLE, &pptlPick, lMaxHits,

                                   lMaxDepth, &alSegTag);

return 0;
}
