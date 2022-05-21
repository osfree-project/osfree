#define INCL_GPIPRIMITIVES      /* GPI primitive functions          */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle                   */

POINTL ptlLines[] = { 100, 100, 100, 200,      /* line 1     */

                                200, 100, 200, 200 };    /* line 2     */



GpiPolyLineDisjoint(hps, 4L, ptlLines);

                        /* draw lines */

return 0;
}
