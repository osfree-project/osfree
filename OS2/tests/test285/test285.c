#define INCL_GPIPATHS           /* GPI Path functions           */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

POINTL ptlStart = { 0, 0 }; /* first vertex                     */

POINTL ptlTriangle[] = { 100, 100, 200, 0, 0, 0 }; /* vertices  */



GpiBeginPath(hps, 1L);                       /* creates path    */

GpiMove(hps, &ptlStart);

GpiPolyLine(hps, 3L, ptlTriangle);

GpiEndPath(hps);



GpiModifyPath(hps,

              1L,

              MPATH_STROKE);     /* modifies path for wide line */

GpiFillPath(hps, 1L, FPATH_ALTERNATE);   /* draws the wide line */

return 0;
}
