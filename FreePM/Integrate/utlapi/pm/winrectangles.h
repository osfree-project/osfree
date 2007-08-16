/*
 $Header: /netlabs.cvs/osfree/src/nonos2/utlapi/pm/winrectangles.h,v 1.1.1.1 2003/10/04 08:27:38 prokushev Exp $
*/

#ifndef _PM_WINRECTANGLES_H_
#define _PM_WINRECTANGLES_H_

#define WINRECTANGLES_INCLUDED

#ifndef INCL_TYPES   /* include base types if not included */
#include <os2def.h>
#endif

/* check do we have neccesary declarations */
#ifndef LIBRECTANGLES_INCLUDED
#include <utlapi/librectangles.h>
#endif

/* All rectangle-related Win* functions are now mapped into PM-independent
   Rect* functions - all parameters (except HAB), return types, and behaviours 
   remain the same */

#define WinCopyRect(hab, prclDst, prclSrc) \
         RectCopy(prclDst, prclSrc)

#define WinSetRect(hab, prcl, xLeft, yBottom, xRight, yTop) \
         RectSet(prcl, xLeft, yBottom, xRight, yTop)

#define WinIsRectEmpty(hab, prcl) \
         RectIsEmpty(prcl)

#define WinEqualRect(hab, prcl1, prcl2) \
         RectEqual(prcl1,prcl2)

#define WinSetRectEmpty(hab, prcl) \
         RectSetEmpty(prcl)

#define WinOffsetRect(hab, prcl, cx, cy) \
         RectOffset(prcl,cx,cy)

#define WinInflateRect(hab, prcl, cx, cy) \
           RectInflate(prcl,cx,cy)

#define WinPtInRect(hab, prcl, pptl) \
         RectPtIn(prcl,pptl)

#define WinIntersectRect(hab, prclDst, prclSrc1, prclSrc2) \
         RectIntersect(prclDst,prclSrc1,prclSrc2)

#define WinUnionRect(hab, prclDst, prclSrc1, prclSrc2) \
         RectUnion(prclDst,prclSrc1,prclSrc2)

#define WinSubtractRect(hab, prclDst, prclSrc1, prclSrc2) \
         RectSubtract(prclDst,prclSrc1,prclSrc2) 
 
/*#define WinMakeRect(hab, pwrc) \
         RectMakeRect(pwrc)

#define WinMakePoints(hab, pwpt, cwpt) \
         RectMakePoints(pwpt,cwpt) */

#endif /* _PM_WINRECTANGLES_H_ */
