#define INCL_GPISEGEDITING      /* GPI Segment Edit functions   */

#define INCL_GPISEGMENTS        /* Segment functions            */

#define INCL_ORDERS             /* Graphical Order Formats      */

#include <os2.h>

int main(VOID)
{

LONG     lHits;         /* correlation/error indicator          */

HPS      hps;           /* presentation-space handle            */

LONG     lType;         /* element type                         */

char     pszDesc[4];    /* element description                  */

LONG     lLength;       /* length of element data               */

LORDER   pbData;        /* pointer to element data              */

ORDERL_GCARC lArcPts = {10L,10L,5L,5L}; /* arc points structure */



GpiOpenSegment(hps, 3L);   /* opens segment to receive element  */



/* type is order code for arc at current position (GARC) */

lType = OCODE_GCARC;



/* call the element 'Arc' */

strcpy(pszDesc,"Arc");



/* length of element data */

lLength = sizeof(LORDER);



/* fill element data structure */

pbData.idCode = OCODE_GCARC;  /* order code: arc at current

                                 position */

pbData.uchLength = sizeof(ORDERL_GCARC);

/* order data contains arc points structure */

memcpy(pbData.uchData, lArcPts, sizeof(ORDERL_GCARC));



/* add element */

lHits = GpiElement(hps, lType, pszDesc, lLength, (BYTE *)&pbData);



GpiCloseSegment(hps);      /* closes segment that received data */

return 0;
}
