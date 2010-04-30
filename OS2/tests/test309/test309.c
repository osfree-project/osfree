#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>



HPS hps;                /* presentation space handle            */

LINEBUNDLE lbnd;

LONG flDefMask;



flDefMask = GpiQueryAttrs(hps,  /* presentation-space handle */

    PRIM_LINE,                  /* line primitive            */

    LBB_COLOR |                 /* line color                */

    LBB_MIX_MODE |              /* color-mix mode            */

    LBB_WIDTH |                 /* line width                */

    LBB_GEOM_WIDTH |            /* geometric-line width      */

    LBB_TYPE |                  /* line style                */

    LBB_END |                   /* line-end style            */

    LBB_JOIN,                   /* line-join style           */

    &lbnd);                     /* buffer for attributes     */



if (flDefMask & LBB_COLOR)

   {

    /* The line color has the default value. */

   }
