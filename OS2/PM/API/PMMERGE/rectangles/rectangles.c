/*
 $Header: /netlabs.cvs/osfree/src/old/pm/rectangles/rectangles.c,v 1.1.1.1 2003/10/04 08:27:54 prokushev Exp $
*/

/* This code is for compatibility with old OS/2 binaries, from now, all
   Rectangle related functions are mapped into Rect* ones... look at
   pm/winrectangles.h and utlapi/librectangles.h */


#ifndef LIBRECTANGLES_INCLUDED
#define INCL_UTLAPI
#define INCL_LIBRECTANGLES
#include <osfree.h>
#endif

/* copies rectangles form prclSrc to prclDst, rturn TRUE if successfull, else
   return FALSE */
BOOL APIENTRY WinCopyRect(HAB hab, PRECTL prclDst, PRECTL prclSrc)
{
  return RectCopy(prclDst, prclSrc);
};

/* set appropriate coordinates in rectangle, TRUE if succesfull, else FALSE */
BOOL APIENTRY WinSetRect(HAB hab, PRECTL prcl, LONG xLeft, LONG yBottom,
                         LONG xRight, LONG yTop)
{
  return RectSet(prcl,xLeft,yBottom,xRight,yTop);
};

/* check if given rectangle is empty */
BOOL APIENTRY WinIsRectEmpty(HAB hab, PRECTL prcl)
{
  return RectIsEmpty(prcl);
};

/* compares two rectangles */
BOOL APIENTRY WinEqualRect(HAB hab, PRECTL prcl1, PRECTL prcl2)
{
  RectEqual(prcl1,prcl2);
};

/* empites rectangle */
BOOL APIENTRY WinSetRectEmpty(HAB hab, PRECTL prcl)
{
  return RectSet(prcl,0,0,0,0);
};

/* offset the rectangle */
BOOL APIENTRY WinOffsetRect(HAB hab, PRECTL prcl, LONG cx, LONG cy)
{
  return RectOffset(prcl,cx,cy);
};

/* expand the rectangle */
BOOL APIENTRY WinInflateRect(HAB hab, PRECTL prcl, LONG cx, LONG cy)
{
  return RectInflate(prcl,cx,cy);
};

/* check is specified point in rectangle */
BOOL APIENTRY WinPtInRect(HAB hab, PRECTL prcl, PPOINTL pptl)
{
  RectPtIn(prcl,pptl);
};

/* calculate intersection of two rectangles */
BOOL APIENTRY WinIntersectRect(HAB hab, PRECTL prclDst, PRECTL prclSrc1,
                               PRECTL prclSrc2)
{
  return RectIntersect(prclDst,prclSrc1,prclSrc2);
};

/* calculate bounding rectangle */
BOOL APIENTRY WinUnionRect(HAB hab, PRECTL prclDst, PRECTL prclSrc1,
                           PRECTL prclSrc2)
{
  return RectUnion(prclDst,prclSrc1,prclSrc2);
};

/* substract Src2 from Src1 */
BOOL APIENTRY WinSubtractRect(HAB hab, PRECTL prclDst, PRECTL prclSrc1,
                              PRECTL prclSrc2)
{
  return RectSubtract(prclDst,prclSrc1,prclSrc2);
};

/* This function does nothing. Seems it was for older versions. */
/* Need more investigation */
BOOL APIENTRY WinMakeRect(HAB hab, PRECTL pwrc)
{
  return TRUE;
};

/* This function does nothing. Seems it was for older versions. */
/* Need more investigation */
BOOL APIENTRY WinMakePoints(HAB hab, PPOINTL pwpt, ULONG cwpt)
{
  return TRUE;
};
