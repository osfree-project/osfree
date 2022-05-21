#define INCL_GPISEGMENTS        /* Segment functions            */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */



GpiDrawFrom(hps, 1L, 4L);

return 0;
}
