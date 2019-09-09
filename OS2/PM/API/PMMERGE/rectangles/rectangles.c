/*
 $Header: /netlabs.cvs/osfree/src/old/pm/rectangles/rectangles.c,v 1.1.1.1 2003/10/04 08:27:54 prokushev Exp $
*/

/* This code is for compatibility with old OS/2 binaries, from now, all
   Rectangle related functions are mapped into Rect* ones... look at
   pm/winrectangles.h and utlapi/librectangles.h */


#include <os2.h>

// UtlAPI function prototypes
BOOL APIENTRY RectCopy(PRECTL prclDst, PRECTL prclSrc);
BOOL APIENTRY RectSet(PRECTL prcl, LONG xLeft, LONG yBottom,
                         LONG xRight, LONG yTop);
BOOL APIENTRY RectIsEmpty(PRECTL prcl);
BOOL APIENTRY RectEqual(PRECTL prcl1, PRECTL prcl2);
BOOL APIENTRY RectSetEmpty(PRECTL prcl);
BOOL APIENTRY RectOffset(PRECTL prcl, LONG cx, LONG cy);
BOOL APIENTRY RectInflate(PRECTL prcl, LONG cx, LONG cy);
BOOL APIENTRY RectPtIn(PRECTL prcl, PPOINTL pptl);
BOOL intRectIntersectRectX(PRECTL prclDst, PRECTL prclSrc1,
                         PRECTL prclSrc2);
BOOL intRectIntersectRectY(PRECTL prclDst,PRECTL prclSrc1, PRECTL prclSrc2,
                          BOOL all);
BOOL APIENTRY RectIntersect(PRECTL prclDst, PRECTL prclSrc1,
                               PRECTL prclSrc2);
BOOL APIENTRY RectUnion(PRECTL prclDst, PRECTL prclSrc1,
                           PRECTL prclSrc2);
BOOL APIENTRY RectSubtract(PRECTL prclDst, PRECTL prclSrc1,
                              PRECTL prclSrc2);
BOOL APIENTRY RectMakeRect(PRECTL pwrc);
BOOL APIENTRY RectMakePoints(PPOINTL pwpt, ULONG cwpt);

/* copies rectangles form prclSrc to prclDst, rturn TRUE if successfull, else
   return FALSE */
BOOL APIENTRY Win32CopyRect(HAB hab, PRECTL prclDst, PRECTL prclSrc)
{
  return RectCopy(prclDst, prclSrc);
};

/* set appropriate coordinates in rectangle, TRUE if succesfull, else FALSE */
BOOL APIENTRY Win32SetRect(HAB hab, PRECTL prcl, LONG xLeft, LONG yBottom,
                         LONG xRight, LONG yTop)
{
  return RectSet(prcl,xLeft,yBottom,xRight,yTop);
};

/* check if given rectangle is empty */
BOOL APIENTRY Win32IsRectEmpty(HAB hab, PRECTL prcl)
{
  return RectIsEmpty(prcl);
};

/* compares two rectangles */
BOOL APIENTRY Win32EqualRect(HAB hab, PRECTL prcl1, PRECTL prcl2)
{
  return RectEqual(prcl1,prcl2);
};

/* empites rectangle */
BOOL APIENTRY Win32SetRectEmpty(HAB hab, PRECTL prcl)
{
  return RectSet(prcl,0,0,0,0);
};

/* offset the rectangle */
BOOL APIENTRY Win32OffsetRect(HAB hab, PRECTL prcl, LONG cx, LONG cy)
{
  return RectOffset(prcl,cx,cy);
};

/* expand the rectangle */
BOOL APIENTRY Win32InflateRect(HAB hab, PRECTL prcl, LONG cx, LONG cy)
{
  return RectInflate(prcl,cx,cy);
};

/* check is specified point in rectangle */
BOOL APIENTRY Win32PtInRect(HAB hab, PRECTL prcl, PPOINTL pptl)
{
  return RectPtIn(prcl,pptl);
};

/* calculate intersection of two rectangles */
BOOL APIENTRY Win32IntersectRect(HAB hab, PRECTL prclDst, PRECTL prclSrc1,
                               PRECTL prclSrc2)
{
  return RectIntersect(prclDst,prclSrc1,prclSrc2);
};

/* calculate bounding rectangle */
BOOL APIENTRY Win32UnionRect(HAB hab, PRECTL prclDst, PRECTL prclSrc1,
                           PRECTL prclSrc2)
{
  return RectUnion(prclDst,prclSrc1,prclSrc2);
};

/* substract Src2 from Src1 */
BOOL APIENTRY Win32SubtractRect(HAB hab, PRECTL prclDst, PRECTL prclSrc1,
                              PRECTL prclSrc2)
{
  return RectSubtract(prclDst,prclSrc1,prclSrc2);
};

/* This function does nothing. Seems it was for older versions. */
/* Need more investigation */
BOOL APIENTRY Win32MakeRect(HAB hab, PRECTL pwrc)
{
  return TRUE;
};

/* This function does nothing. Seems it was for older versions. */
/* Need more investigation */
BOOL APIENTRY Win32MakePoints(HAB hab, PPOINTL pwpt, ULONG cwpt)
{
  return TRUE;
};
