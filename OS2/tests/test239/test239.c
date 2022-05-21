#define INCL_GPICORRELATION     /* GPI Correlation functions    */

#include <os2.h>

int main(VOID)
{

BOOL     fSuccess;      /* success indicator                    */

SIZEL    psizlSize;     /* size of pick aperture                */

LONG     lNumHits;      /* number of hits or error              */

HPS      hps;           /* Presentation-space handle            */

LONG     lFirstSegment; /* Specifies the first segment to be

                           correlated                           */

LONG     lLastSegment;  /* Specifies the last segment to be

                           correlated                           */

POINTL   pptlPick = {200L,200L};

                        /* Pick (center of aperture) position   */

LONG     lMaxHits;      /* Maximum hits to be returned          */

LONG     lMaxDepth;     /* Number of pairs to be returned       */

LONG     alSegTag;      /* Segment identifiers and tags         */



fSuccess = GpiSetPickAperturePosition(hps, &pptlPick);



/* set aperture size (use default) */

fSuccess = GpiSetPickApertureSize(hps, PICKAP_DEFAULT, &psizlSize);



/* define chain of two segments (1 and 2) */

lFirstSegment = 1;

lLastSegment = 2;



/* return only one hit */

lMaxHits = 1L;



/* return only one segment/tag pair per hit */

lMaxDepth = 1L;



/* correlate on visible, detectable segments */

lNumHits = GpiCorrelateFrom(hps, lFirstSegment, lLastSegment,

                            PICKSEL_VISIBLE, &pptlPick, lMaxHits,

                            lMaxDepth, &alSegTag);

return 0;

}
