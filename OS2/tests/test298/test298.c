#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>



HPS hps;                /* presentation space handle            */

POINTL ptlStart = { 0, 0 }; /* start of curve                   */

POINTL aptl[4]={ 100, 100, 200, 100, 0, 100, 200, 0};/* points  */

FIXED afx[2]={MAKEFIXED(4, 0), MAKEFIXED(4, 0)};/* sharpness    */



GpiMove(hps, &ptlStart);   /* move to first starting point         */

GpiPolyFilletSharp(hps,    /* presentation-space handle            */

    4L,                    /* 4 points in the array                */

    aptl,                  /* address of array of points           */

    afx);                  /* address of array of sharpness values */
