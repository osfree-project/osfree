#define INCL_GPIPATHS           /* GPI Path functions           */

#include <os2.h>



HPS hps;                /* presentation space handle            */

POINTL ptlStart = { 0, 0 }; /* first vertex                     */

POINTL ptlTriangle[] = { 100, 100, 200, 0, 0, 0 }; /* vertices  */



GpiBeginPath(hps, 1L);                  /* create a path */

GpiMove(hps, &ptlStart);

GpiPolyLine(hps, 3L, ptlTriangle);

GpiEndPath(hps);



GpiFillPath(hps, 1L, FPATH_ALTERNATE);  /* fill the path */