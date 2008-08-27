#ifndef LIBRECTANGLES_INCLUDED
#define INCL_UTLAPI
#define INCL_LIBRECTANGLES  /* Rect* functions declarations */
#include <osfree.h>
#include <pmdef.h>
#endif

/* TODO:
    - documentation states that if language supports it WRECT type can be
      used instead of PRECTL - how should we support that?
    - RectMakePoint & RectMakeRect: look at pm/rectangles/rectangles.c for
      more about that
*/

/* internal helper functions (intersection) - declarations */
BOOL intRectIntersectRectY(PRECTL prclDst,PRECTL prclSrc1, PRECTL prclSrc2,
                          BOOL all);
BOOL intRectIntersectRectX( PRECTL prclDst, PRECTL prclSrc1,
                         PRECTL prclSrc2);


/* copies rectangles form prclSrc to prclDst, rturn TRUE if successfull, else
   return FALSE */
BOOL APIENTRY RectCopy(PRECTL prclDst, PRECTL prclSrc)
{
  /* check if pointers are valid, exit with FALSE if not */
  if ((prclSrc==NULL) || (prclDst==NULL))
    return FALSE;

  /* copy values */
  prclDst->xLeft=prclSrc->xLeft;
  prclDst->xRight=prclSrc->xRight;
  prclDst->yBottom=prclSrc->yBottom;
  prclDst->yTop=prclSrc->yTop;

  /* succesfully copied */
  return TRUE;
};

/* set appropriate coordinates in rectangle, TRUE if succesfull, else FALSE */
BOOL APIENTRY RectSet(PRECTL prcl, LONG xLeft, LONG yBottom,
                         LONG xRight, LONG yTop)
{
  /* check if rectangle pointer is valid, exit with FALSE if not */
  if (prcl==NULL) return FALSE;

  /* set appropriate coordinates */
  prcl->xLeft=xLeft;
  prcl->xRight=xRight;
  prcl->yBottom=yBottom;
  prcl->yTop=yTop;

  /* success */
  return TRUE;
};

/* check if given rectangle is empty */
BOOL APIENTRY RectIsEmpty(PRECTL prcl)
{
  /* check if rectangle pointer is valid, if not -> empty -> return TRUE */
  if (prcl==NULL) return TRUE;

  /* check if empty */
  if ((prcl->xLeft==prcl->xRight) || (prcl->xRight<prcl->xLeft))
    return TRUE;

  if ((prcl->yTop==prcl->yBottom) || (prcl->yTop<prcl->yBottom))
    return TRUE;

  /* not empty... */
  return FALSE;
};

/* compares two rectangles */
BOOL APIENTRY RectEqual(PRECTL prcl1, PRECTL prcl2)
{
  /* check if pointers are valid, exit with FALSE if not */
  if ((prcl1==NULL) || (prcl2==NULL))
    return FALSE;

  /* check if equal */
  if ((prcl1->xLeft!=prcl2->xLeft) ||
      (prcl1->xRight!=prcl2->xRight) ||
      (prcl1->yTop!=prcl2->yTop) ||
      (prcl1->yBottom!=prcl2->yBottom))
     return FALSE;

  /* they're equal */
  return TRUE;
};

/* empites rectangle */
BOOL APIENTRY RectSetEmpty(PRECTL prcl)
{
  /* check if rectangle pointer is valid, exit with FALSE if not */
  if (prcl==NULL) return FALSE;

  return RectSet(prcl,0,0,0,0);
};

/* offset the rectangle */
BOOL APIENTRY RectOffset(PRECTL prcl, LONG cx, LONG cy)
{
  /* check if rectangle pointer is valid, exit with FALSE if not */
  if (prcl==NULL) return FALSE;

  /* offset */
  prcl->xLeft+=cx;
  prcl->xRight+=cx;
  prcl->yTop+=cy;
  prcl->yBottom+=cy;

  /* success */
  return TRUE;
};

/* expand the rectangle */
BOOL APIENTRY RectInflate(PRECTL prcl, LONG cx, LONG cy)
{
  /* check if rectangle pointer is valid, exit with FALSE if not */
  if (prcl==NULL) return FALSE;

  /* inflate */
  prcl->xLeft-=cx;
  prcl->xRight=+cx;
  prcl->yTop=+cy;
  prcl->yBottom-=cy;

  /* success */
  return FALSE;
};

/* check is specified point in rectangle */
BOOL APIENTRY RectPtIn(PRECTL prcl, PPOINTL pptl)
{
  /* check if params pointers are valid, exit with FALSE if not */
  if ((prcl==NULL) || (pptl==NULL))return FALSE;

  /* is outside rect? */
  if ( (pptl->x<prcl->xLeft) ||
       (pptl->x>prcl->xRight) ||
       (pptl->y<prcl->yBottom) ||
       (pptl->y>prcl->yTop))
    return FALSE;

  /* otherwise is in rect */
  return TRUE;
};

/* intersection helper functions */

BOOL intRectIntersectRectX(PRECTL prclDst, PRECTL prclSrc1,
                         PRECTL prclSrc2)
{
  if ((prclSrc1->xLeft>=prclSrc2->xLeft) &&
      (prclSrc1->xRight<=prclSrc2->xRight) &&
      (prclSrc1->yTop>=prclSrc2->yTop) &&
      (prclSrc1->yBottom<=prclSrc2->yBottom))
    return RectSet(prclDst,prclSrc1->xLeft,prclSrc2->yBottom,
                                  prclSrc1->xRight,prclSrc2->yTop);
    else
  if ((prclSrc1->xLeft<=prclSrc2->xLeft) &&
      (prclSrc1->xRight>=prclSrc2->xRight))
  {
    prclDst->xLeft=prclSrc2->xLeft;
    prclDst->xRight=prclSrc2->xRight;
    return intRectIntersectRectY(prclDst,prclSrc1,prclSrc2,FALSE);
  } else
  if ((prclSrc1->xRight>=prclSrc2->xLeft) &&
      (prclSrc1->xRight<=prclSrc2->xRight))
  {
    prclDst->xLeft=prclSrc2->xLeft;
    prclDst->xRight=prclSrc1->xRight;
    return intRectIntersectRectY(prclDst,prclSrc1,prclSrc2,TRUE);
  } else
  if ((prclSrc1->xLeft>=prclSrc2->xLeft) &&
      (prclSrc1->xLeft<=prclSrc2->xRight))
  {
    prclDst->xLeft=prclSrc1->xLeft;
    prclDst->xRight=prclSrc2->xRight;
    return intRectIntersectRectY(prclDst,prclSrc1,prclSrc2,TRUE);
  } else
   return FALSE; /* no intersection */
};

BOOL intRectIntersectRectY(PRECTL prclDst,PRECTL prclSrc1, PRECTL prclSrc2,
                          BOOL all)
{
  if ((prclSrc1->yTop>=prclSrc2->yTop) &&
      (prclSrc1->yBottom<=prclSrc2->yBottom))
  {
    prclDst->yTop=prclSrc2->yTop;
    prclDst->yBottom=prclSrc2->yBottom;
  } else
  if ((prclSrc1->yTop<=prclSrc2->yTop) &&
      (prclSrc1->yBottom>=prclSrc2->yBottom) &&
      (all==TRUE))
  {
    prclDst->yTop=prclSrc1->yTop;
    prclDst->yBottom=prclSrc1->yBottom;
  }  else
  if ((prclSrc1->yTop>=prclSrc2->yBottom) &&
      (prclSrc1->yTop<=prclSrc2->yTop))
  {
    prclDst->yTop=prclSrc1->yTop;
    prclDst->yBottom=prclSrc2->yBottom;
  } else
  if ((prclSrc1->yBottom>=prclSrc2->yBottom) &&
      (prclSrc1->yBottom<=prclSrc2->yTop))
  {
    prclDst->yTop=prclSrc2->yTop;
    prclDst->yBottom=prclSrc1->yBottom;
  } else
   return FALSE;

  return TRUE;
};

/* calculate intersection of two rectangles */
BOOL APIENTRY RectIntersect(PRECTL prclDst, PRECTL prclSrc1,
                               PRECTL prclSrc2)
{
  RECTL tmprect;

  /* check if params pointers are valid, exit with FALSE if not */
  if ((prclDst==NULL) || (prclSrc1==NULL) || (prclSrc1==NULL)) return FALSE;

  /* TODO: czesc wspolna dwoch rectanglesow!!! */

  /* check is there an intersection */
  if (!intRectIntersectRectX(&tmprect,prclSrc1,prclSrc2))
   if (!intRectIntersectRectX(&tmprect,prclSrc2,prclSrc1))
     return FALSE;

  /* otherwise, result is in tmprect */
  return RectCopy(prclDst,&tmprect);
};

/* calculate bounding rectangle */
BOOL APIENTRY RectUnion(PRECTL prclDst, PRECTL prclSrc1,
                           PRECTL prclSrc2)
{
  /* check if params pointers are valid, exit with FALSE if not */
  if ((prclDst==NULL) || (prclSrc1==NULL) || (prclSrc1==NULL)) return FALSE;

  /* if one of source rectangles is empty, we're returning the second */
  if (RectIsEmpty(prclSrc1)) RectCopy(prclDst,prclSrc2); else
  if (RectIsEmpty(prclSrc2)) RectCopy(prclDst,prclSrc1); else
  {
    /* none of them is empty - proceed with calculation of bounding rect */

    if (prclSrc1->xLeft<prclSrc2->xLeft)
      prclDst->xLeft=prclSrc1->xLeft; else
      prclDst->xLeft=prclSrc2->xLeft;

    if (prclSrc1->xRight>prclSrc2->xRight)
      prclDst->xRight=prclSrc1->xRight; else
      prclDst->xRight=prclSrc2->xRight;

    if (prclSrc1->yBottom<prclSrc2->yBottom)
      prclDst->yBottom=prclSrc1->yBottom; else
      prclDst->yBottom=prclSrc2->yBottom;

    if (prclSrc1->yTop>prclSrc2->yTop)
      prclDst->yTop=prclSrc1->yTop; else
      prclDst->yTop=prclSrc2->yTop;

  };

  /* success */
  return TRUE;
};

/* substract Src2 from Src1 */
BOOL APIENTRY RectSubtract(PRECTL prclDst, PRECTL prclSrc1,
                              PRECTL prclSrc2)
{
  RECTL tmpRect;

  /* check if params pointers are valid, exit with FALSE if not */
  if ((prclDst==NULL) || (prclSrc1==NULL) || (prclSrc1==NULL)) return FALSE;

  RectSetEmpty(prclDst);

  /* first: check is there an intersection beetween these two */
  if (RectIntersect(&tmpRect,prclSrc1,prclSrc2))
  {
    /* now check is resulting rect aligned to at least 3 edges of source one
       - only in this case substraction can be performed */
    if ((tmpRect.yBottom==prclSrc1->yBottom) &&
        (tmpRect.xLeft==prclSrc1->xLeft) &&
        (tmpRect.yTop==prclSrc1->yTop))
    {
      RectSet(prclDst,tmpRect.xRight,tmpRect.yBottom,
                     prclSrc1->xRight,tmpRect.yTop);
    } else
    if ((tmpRect.xLeft==prclSrc1->xLeft) &&
        (tmpRect.yTop==prclSrc1->yTop) &&
        (tmpRect.xRight==prclSrc1->xRight))
    {
      RectSet(prclDst,tmpRect.xLeft,prclSrc1->yBottom,
                     tmpRect.xRight,tmpRect.yBottom);
    } else
    if ((tmpRect.yTop==prclSrc1->yTop) &&
        (tmpRect.xRight==prclSrc1->xRight) &&
        (tmpRect.yBottom==prclSrc1->yBottom))
    {
      RectSet(prclDst,prclSrc1->xLeft,tmpRect.yBottom,
                     tmpRect.xLeft,tmpRect.yTop);
    } else
    if ((tmpRect.xRight==prclSrc1->xRight) &&
        (tmpRect.yBottom==prclSrc1->yBottom) &&
        (tmpRect.xLeft==prclSrc1->xLeft))
    {
      RectSet(prclDst,tmpRect.xLeft,tmpRect.yBottom,
                     tmpRect.xRight,prclSrc1->yTop);
    };
  };

  /* well this  doesn't long with docs... but that's what original does... */
  if (RectIsEmpty(prclDst)) RectCopy(prclDst,prclSrc1);
  return TRUE;
};

// ---- following functions are not implemented, see todo at the top of this
//      file

BOOL APIENTRY RectMakeRect(PRECTL pwrc)
{
  /* TODO: clarify docs... */
  return FALSE;
};

BOOL APIENTRY RectMakePoints(PPOINTL pwpt, ULONG cwpt)
{
  /* TODO: clarify docs... */
  return FALSE;
};
