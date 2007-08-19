/*
 $Header: /netlabs.cvs/osfree/src/nonos2/utlapi/utlapi/librectangles.h,v 1.1.1.1 2003/10/04 08:27:46 prokushev Exp $
*/

#ifndef _RectAPI_LIBRECTANGLES_H_
#define _RectAPI_LIBRECTANGLES_H_

#define LIBRECTANGLES_INCLUDED

#ifndef INCL_TYPES   /* include base types if not included */
#include <os2def.h>
#endif

#ifdef __cplusplus
      extern "C" {
#endif

BOOL APIENTRY RectCopy(PRECTL prclDst, PRECTL prclSrc);

BOOL APIENTRY RectSet(PRECTL prcl, LONG xLeft, LONG yBottom,
                         LONG xRight, LONG yTop);

BOOL APIENTRY RectIsEmpty(PRECTL prcl);

BOOL APIENTRY RectEqual(PRECTL prcl1, PRECTL prcl2);

BOOL APIENTRY RectSetEmpty(PRECTL prcl);

BOOL APIENTRY RectOffset(PRECTL prcl, LONG cx, LONG cy);

BOOL APIENTRY RectInflate(PRECTL prcl, LONG cx, LONG cy);

BOOL APIENTRY RectPtIn(PRECTL prcl, PPOINTL pptl);

BOOL APIENTRY RectIntersect(PRECTL prclDst, PRECTL prclSrc1,
                               PRECTL prclSrc2);

BOOL APIENTRY RectUnion(PRECTL prclDst, PRECTL prclSrc1,
                           PRECTL prclSrc2);

BOOL APIENTRY RectSubtract(PRECTL prclDst, PRECTL prclSrc1,
                              PRECTL prclSrc2);

// what with them??
BOOL APIENTRY RectMakeRect(PRECTL pwrc);

BOOL APIENTRY RectMakePoints(PPOINTL pwpt, ULONG cwpt);

#ifdef __cplusplus
  }
#endif

#endif /* _RectAPI_LIBRECTANGLES_H_ */
