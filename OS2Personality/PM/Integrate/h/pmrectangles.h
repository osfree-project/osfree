/*
 $Header: /netlabs.cvs/osfree/src/old/pm/h/pmrectangles.h,v 1.1.1.1 2003/10/04 08:27:52 prokushev Exp $
*/

#ifndef _PM_WINRECTANGLES_H_
#define _PM_WINRECTANGLES_H_

#define WINRECTANGLES_INCLUDED

#ifndef INCL_TYPES   /* include base types if not included */
#include <os2def.h>
#endif

BOOL APIENTRY WinCopyRect(HAB hab, PRECTL prclDst, PRECTL prclSrc);

BOOL APIENTRY WinSetRect(HAB hab, PRECTL prcl, LONG xLeft, LONG yBottom,
                         LONG xRight, LONG yTop);

BOOL APIENTRY WinIsRectEmpty(HAB hab, PRECTL prcl);

BOOL APIENTRY WinEqualRect(HAB hab, PRECTL prcl1, PRECTL prcl2);

BOOL APIENTRY WinSetRectEmpty(HAB hab, PRECTL prcl);

BOOL APIENTRY WinOffsetRect(HAB hab, PRECTL prcl, LONG cx, LONG cy);

BOOL APIENTRY WinInflateRect(HAB hab, PRECTL prcl, LONG cx, LONG cy);

BOOL APIENTRY WinPtInRect(HAB hab, PRECTL prcl, PPOINTL pptl);

BOOL APIENTRY WinIntersectRect(HAB hab, PRECTL prclDst, PRECTL prclSrc1,
                               PRECTL prclSrc2);

BOOL APIENTRY WinUnionRect(HAB hab, PRECTL prclDst, PRECTL prclSrc1,
                           PRECTL prclSrc2);

BOOL APIENTRY WinSubtractRect(HAB hab, PRECTL prclDst, PRECTL prclSrc1,
                              PRECTL prclSrc2);

BOOL APIENTRY WinMakeRect(HAB hab, PRECTL pwrc);

BOOL APIENTRY WinMakePoints(HAB hab, PPOINTL pwpt, ULONG cwpt);

#endif /* _PM_WINRECTANGLES_H_ */
