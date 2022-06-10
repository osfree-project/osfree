
/*
 *@@sourcefile cnrh.h:
 *      header file for cnrh.c (container helper functions).
 *      See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_WINSTDCNR
 *@@include #define INCL_WINSTDDRAG
 *@@include #include <os2.h>
 *@@include #include "helpers\cnrh.h"
 */

/*      Copyright (C) 1997-2000 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#if __cplusplus
extern "C" {
#endif

#ifndef CNRH_HEADER_INCLUDED
    #define CNRH_HEADER_INCLUDED

    #ifndef INCL_WINSTDCNR
        #error cnrh.h requires INCL_WINSTDCNR to be defined.
    #endif

    /*
     *@@category: Helpers\PM helpers\Container helpers\Details view helpers
     */

    /* ******************************************************************
     *
     *   Details view field infos
     *
     ********************************************************************/

    // redefine the FIELDOFFSET macro; the one
    // in the OS/2 header files doesn't work with C++
    #undef FIELDOFFSET
    #define FIELDOFFSET(type, field)    ((ULONG)&(((type *)0)->field))

    ULONG cnrhClearFieldInfos(HWND hwndCnr,
                              BOOL fInvalidate);

    /*
     *@@ cnrhAllocFieldInfos:
     *      this allocates sColumnCount field infos
     *      for Details view in hwndCnr.
     *      Returns the first PFIELDINFO, which is
     *      the root of a linked list. Follow
     *      FIELDINFO.pNextFieldInfo to get to the
     *      next field info, the last of which is
     *      NULL.
     *
     *@@added V0.9.0
     */

    #define cnrhAllocFieldInfos(hwndCnr, sColumnCount)              \
        (PFIELDINFO)WinSendMsg(hwndCnr,                             \
                               CM_ALLOCDETAILFIELDINFO,             \
                               MPFROMSHORT(sColumnCount),           \
                               NULL)

    /*
     *@@ XFIELDINFO:
     *      input structure for cnrhSetFieldInfos.
     *      See cnrhSetFieldInfo for a description
     *      of the fields.
     *
     *@@added V0.9.0
     */

    typedef struct _XFIELDINFO
    {
        ULONG   ulFieldOffset;
        PCSZ    pszColumnTitle;
        ULONG   ulDataType;
        ULONG   ulOrientation;
    } XFIELDINFO, *PXFIELDINFO;

    VOID cnrhSetFieldInfo(PFIELDINFO *ppFieldInfo2,
                          ULONG ulFieldOffset,
                          PSZ pszColumnTitle,
                          ULONG ulDataType,
                          ULONG ulOrientation,
                          BOOL fDrawLines);

    ULONG cnrhInsertFieldInfos(HWND hwndCnr,
                               PFIELDINFO pFieldInfoFirst,

                               ULONG ulFieldCount);

    PFIELDINFO cnrhSetFieldInfos(HWND hwndCnr,
                                 PXFIELDINFO paxfi,
                                 ULONG ulFieldCount,
                                 BOOL fDrawLines,
                                 ULONG ulFieldReturn);

    /*
     *@@category: Helpers\PM helpers\Container helpers\Record core helpers
     */

    /* ******************************************************************
     *
     *   Record core management
     *
     ********************************************************************/

    /*
     *@@ FNCNRSORT:
     *      container sort function as used in
     *      CM_SORTRECORD and CNRINFO.pSortRecord
     */

    typedef SHORT EXPENTRY FNCNRSORT(PVOID,     // record core 1
                                     PVOID,     // record core 2
                                     PVOID);    // pStorage (== NULL)
    typedef FNCNRSORT *PFNCNRSORT;

    PRECORDCORE cnrhAllocRecords(HWND hwndCnr,
                                 ULONG cbrecc,
                                 ULONG ulCount);

    ULONG cnrhInsertRecords(HWND hwndCnr,
                            PRECORDCORE preccParent,
                            PRECORDCORE precc,
                            BOOL fInvalidate,
                            const char *pcszText,
                            ULONG flRecordAttr,
                            ULONG ulCount);

    ULONG cnrhInsertRecordAfter(HWND hwndCnr,
                                PRECORDCORE precc,
                                PSZ pszText,
                                ULONG flRecordAttr,
                                PRECORDCORE preccAfter,
                                BOOL fInvalidate);

    BOOL cnrhMoveRecord(HWND hwndCnr,
                        PRECORDCORE preccMove,
                        PRECORDCORE preccInsertAfter);

    BOOL cnrhMoveTree(HWND hwndCnr,
                      PRECORDCORE preccMove,
                      PRECORDCORE preccNewParent,
                      PFNCNRSORT pfnCnrSort);

    /*
     *@@ cnrhRemoveAll:
     *      this helper removes all records from the container
     *      and frees them (CM_REMOVERECORD with NULL
     *      and CMA_FREE | CMA_INVALIDATE specified).
     *
     *@@added V0.9.0 (99-10-22) [umoeller]
     */

    #define cnrhRemoveAll(hwndCnr) \
        WinSendMsg(hwndCnr, CM_REMOVERECORD, NULL, MPFROM2SHORT(0, CMA_FREE | CMA_INVALIDATE))

    /*
     *@@ cnrhInvalidateAll:
     *      invalidates all records in the container.
     *
     *@@added V0.9.2 (2000-02-19) [umoeller]
     */

    #define cnrhInvalidateAll(hwndCnr) \
        WinSendMsg(hwndCnr, CM_INVALIDATERECORD, NULL, MPFROM2SHORT(0, CMA_ERASE | CMA_REPOSITION | CMA_TEXTCHANGED))

    /*
     *@@category: Helpers\PM helpers\Container helpers\View management
     */

    /* ******************************************************************
     *
     *   CNRINFO management
     *
     ********************************************************************/

    /*
     *@@ cnrhQueryCnrInfo:
     *      this stores the CNRINFO for hwndCnr
     *      in the specified variable, which must
     *      be a PCNRINFO.
     *      Example:
     *
     +          CNRINFO CnrInfo;
     +          cnrhQueryCnrInfo(&CnrInfo);
     *
     *@@changed V0.9.0
     */

    #define cnrhQueryCnrInfo(hwndCnr, pCnrInfo)                     \
            WinSendMsg(hwndCnr, CM_QUERYCNRINFO,                    \
                    (MPARAM)(pCnrInfo),                             \
                    (MPARAM)(sizeof(*pCnrInfo)))

    /*
     *@@ BEGIN_CNRINFO:
     *      this macro starts a "container info" block. In such
     *      a block, you may use the following macros:
     *
     *      -- cnrhSetTitle
     *
     *      -- cnrhSetSplitBarAfter
     *
     *      -- cnrhSetSplitBarPos
     *
     *      -- cnrhSetTreeBmpOrIconSize
     *
     *      -- cnrhSetBmpOrIconSize
     *
     *      -- cnrhSetView (most frequently)
     *
     *      -- cnrhSetTreeIndent
     *
     *      -- cnrhSetSortFunc
     *
     *      Typical usage is like this:
     *
     +          BEGIN_CNRINFO()
     +          {
     +              cnrhSetTreeIndent(20);
     +              cnrhSetView(CV_TREE | CV_ICON | CA_TREELINE);
     +          } END_CNRINFO(hwndCnr);
     *
     *      This must always be followed by END_CNRINFO(),
     *      or you'll get funny compilation errors.
     *
     *@@added V0.9.0
     */

    #define BEGIN_CNRINFO()                                         \
        {                                                           \
            CNRINFO     CnrInfo_ = {0};                             \
            ULONG       ulSendFlags_ = 0;                           \
            CnrInfo_.cb = sizeof(CnrInfo_);

    /*
     *@@ END_CNRINFO:
     *      this macro ends a "container info" block.
     *      See BEGIN_CNRINFO.
     *
     *@@added V0.9.0
     */

    #define END_CNRINFO(hwndCnr)                                    \
            WinSendMsg((hwndCnr), CM_SETCNRINFO, (MPARAM)&CnrInfo_, \
                (MPARAM)ulSendFlags_);                              \
        }

    /*
     *@@ cnrhSetTitle:
     *      this macro sets the container title to the specified
     *      text. You must specify CA_CONTAINERTITLE with
     *      cnrhSetView then.
     *
     *      This can only be used after BEGIN_CNRINFO().
     *
     *@@added V0.9.1 (99-12-18) [umoeller]
     */

    #define cnrhSetTitle(pszTitle)                                  \
        CnrInfo_.pszCnrTitle = (pszTitle);                          \
        ulSendFlags_ |= CMA_CNRTITLE;

    /*
     *@@ cnrhSetSplitBarAfter:
     *      this macro sets the field info after which
     *      the split bar should be shown.
     *      If (pFieldInfo == NULL), the split bar is hidden.
     *
     *      This can be used directly with the return value
     *      of cnrhSetFieldInfos. With that function, specify
     *      the field info to be returned with ulFieldReturn.
     *
     *      This can only be used after BEGIN_CNRINFO().
     *
     *@@added V0.9.0
     */

    #define cnrhSetSplitBarAfter(pFieldInfo)                        \
        CnrInfo_.pFieldInfoLast = (pFieldInfo);                     \
        ulSendFlags_ |= CMA_PFIELDINFOLAST;

    /*
     *@@ cnrhSetSplitBarPos:
     *      this macro sets the vertical split bar
     *      position (in window coordinates).
     *
     *      This can only be used after BEGIN_CNRINFO().
     *
     *@@added V0.9.0
     */

    #define cnrhSetSplitBarPos(xPos)                                \
        CnrInfo_.xVertSplitbar = (xPos);                            \
        ulSendFlags_ |= CMA_XVERTSPLITBAR;

    /*
     *@@ cnrhSetTreeBmpOrIconSize:
     *      this macro sets the size used for the bitmaps
     *      or icons used for the "+" and "-" signs in
     *      tree views.
     *      The default is the system icon size (32 or 40).
     *
     *      This can only be used after BEGIN_CNRINFO().
     */

    #define cnrhSetTreeBmpOrIconSize(cxNew, cyNew)                  \
        CnrInfo_.slTreeBitmapOrIcon.cx = cxNew;                     \
        CnrInfo_.slTreeBitmapOrIcon.cy = cyNew;                     \
        ulSendFlags_ |= CMA_SLTREEBITMAPORICON;

    /*
     *@@ cnrhSetBmpOrIconSize:
     *      this macro sets the size used for bitmaps
     *      or icons (depending on whether CA_DRAWBITMAP
     *      or the default CA_DRAWICON is set in flWindowAttr).
     *      The default is the system icon size (32 or 40).
     *
     *      This can only be used after BEGIN_CNRINFO().
     *
     *@@added V0.9.0
     */

    #define cnrhSetBmpOrIconSize(cxNew, cyNew)                      \
        CnrInfo_.slBitmapOrIcon.cx = cxNew;                         \
        CnrInfo_.slBitmapOrIcon.cy = cyNew;                         \
        ulSendFlags_ |= CMA_SLBITMAPORICON;

    /*
     *@@ cnrhSetView:
     *      this macro sets the container view attributes (CNRINFO.flWindowAttr).
     *
     *      This can only be used after BEGIN_CNRINFO(), like this:
     *
     +          BEGIN_CNRINFO()
     +          {
     +              cnrhSetView(CV_TEXT | CV_FLOW);
     +          } END_CNRINFO(hwndCnr);
     *
     *      The following combinations are useful for flWindowAttr.
     *
     *      <B>Icon view:</B>
     *      --  CV_ICON: icon view
     *                   RECORDCORE.pszIcon is used for record titles;
     *                   add CV_MINI for mini-icons view
     *
     *      <B>Tree view:</B>
     *      --  CV_TREE | CV_ICON: tree icon view (icon plus name;
     *                   RECORDCORE.pszIcon is used for record titles)
     *      --  CV_TREE | CV_NAME: tree name view (icon plus name;
     *                   RECORDCORE.pszName is used for record titles,
     *                   and this does _not_ use a separate icon/bitmap for
     *                   "collapsed" and "expanded"
     *      --  CV_TREE | CV_TEXT: tree text view (no icons;
     *                   RECORDCORE.pszText is used for record titles)
     *      Add CA_TREELINE to have lines drawn between tree items.
     *
     *      <B>Name view:</B>
     *      --  CV_NAME: non-flowed name view (icon plus name;
     *                   RECORDCORE.pszName is used for record titles;
     *                   display a single column
     *      --  CV_NAME | CV_FLOW: flowed name view (icon plus name;
     *                   RECORDCORE.pszName is used for record titles;
     *                   display multiple columns
     *
     *      <B>Text view:</B>
     *      --  CV_TEXT: non-flowed name view (name WITHOUT icons;
     *                   RECORDCORE.pszText is used for record titles;
     *                   display a single column
     *      --  CV_TEXT | CV_FLOW: flowed name view (name WITHOUT icons;
     *                   RECORDCORE.pszText is used for record titles;
     *                   display multiple columns
     *
     *      <B>Details view:</B>
     *      --  CV_DETAIL: details view without column headers
     *      --  CA_DETAILSVIEWTITLES: details view _with_ column headers
     *      --  CV_MINI: mini icons (but set the CCS_MINICONS _window_
     *          style in addition, see below)
     *
     *      <B>Mini icons:</B>
     *
     *      This is especially sick. The CV_MINI style in CNRINFO is only
     *      supported for "pure" icon view. For details and tree views,
     *      _in addition_, you must set the (half-documented) CCS_MINICONS style
     *      as a _window_ style on the container. Whoever came up with this.
     *
     *      <B>Target emphasis:</B>
     *
     *      For Name, Text, and Details view, per default target emphasis
     *      during d'n'd is only displayed _around_ the record core.
     *      This allows users to drop records onto each other.
     *      If you add CA_ORDEREDTARGETEMPH, target emphasis is only
     *      displayed _between_ records (to reorder records).
     *      If you add CA_MIXEDTARGETEMPH, both emphasis types are possible.
     *
     *      Additional flags for _all_ views:
     *      --  CA_DRAWICON: RECORDCORE.hptrIcon and hptrMiniIcon
     *                       are to be used to draw icons
     *      --  CA_DRAWBITMAP: RECORDCORE.hbmBitmap and hbmMiniBitmap
     *                         are to be used to draw bitmaps
     *                         (CA_DRAWICON takes precedence)
     *      --  CA_OWNERDRAW: all record cores are to be owner-drawn.
     *      --  CA_OWNERPAINTBACKGROUND: container sends CM_PAINTBACKGROUND
     *                          to itself. By subclassing the container,
     *                          this can be intercepted.
     *      --  CA_CONTAINERTITLE: add container title. If this is
     *                          specified, you may also specify
     *                          CA_TITLEREADONLY, CA_TITLESEPARATOR,
     *                          and one out of (CA_TITLECENTER, CA_TITLELEFT,
     *                          CA_TITLERIGHT).
     *
     *@@added V0.9.0
     */

    #define cnrhSetView(flNewAttr)                                  \
        CnrInfo_.flWindowAttr = (flNewAttr);                        \
        ulSendFlags_ |= CMA_FLWINDOWATTR;

    /*
     *@@ cnrhSetTreeIndent:
     *      this macro sets the horizontal spacing between levels
     *      in Tree views.
     *
     *      This can only be used after BEGIN_CNRINFO().
     *
     *@@added V0.9.0
     */

    #define cnrhSetTreeIndent(ulNewIndent)                          \
        CnrInfo_.cxTreeIndent = (ulNewIndent);                      \
        ulSendFlags_ |= CMA_CXTREEINDENT;

    /*
     *@@ cnrhSetSortFunc:
     *      this macro sets the sort function for a container.
     *
     *      This can only be used after BEGIN_CNRINFO().
     *
     *      Container sort funcs must have this prototype:
     +
     +      SHORT EXPENTRY fnCompare(PRECORDCORE p1, PRECORDCORE p2, PVOID pStorage)
     *
     *@@added V0.9.0
     */

    #define cnrhSetSortFunc(fnCompareName)                          \
        CnrInfo_.pSortRecord = (PVOID)(fnCompareName);              \
        ulSendFlags_ |= CMA_PSORTRECORD;

    /* ******************************************************************
     *
     *   View management
     *
     ********************************************************************/

    /*
     *@@ cnrhSelectRecord:
     *      this selects/deselects a container RECORDCORE
     *      (precc), depending on fSelect.
     *
     *@@changed V0.9.0
     */

    #define cnrhSelectRecord(hwndCnr, precc, fSelect)        \
            WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS,           \
                    (MPARAM)(precc),                            \
                    MPFROM2SHORT(fSelect, CRA_SELECTED))

    /*
     *@@ cnrhSetSourceEmphasis:
     *      this gives a container RECORDCORE (precc)
     *      source emphasis or removes it, depending
     *      on fSelect.
     *      precc may be NULL to give the whole container
     *      source emphasis.
     */

    #define cnrhSetSourceEmphasis(hwndCnr, precc, fSelect)   \
            WinSendMsg(hwndCnr, CM_SETRECORDEMPHASIS,           \
                    (MPARAM)(precc),                            \
                    MPFROM2SHORT(fSelect, CRA_SOURCE))

    ULONG cnrhSelectAll(HWND hwndCnr,
                        BOOL fSelect);

    #define FRFP_RIGHTSPLITWINDOW   0x0001
    #define FRFP_SCREENCOORDS       0x0002

    PRECORDCORE cnrhFindRecordFromPoint(HWND hwndCnr,
                                        PPOINTL pptl,
                                        PRECTL prclFoundRecord,
                                        ULONG fsExtent,
                                        ULONG fl);

    ULONG cnrhExpandFromRoot(HWND hwndCnr,
                             PRECORDCORE prec);

    ULONG cnrhScrollToRecord(HWND hwndCnr,
                             PRECORDCORE pRec,
                             ULONG fsExtent,
                             BOOL KeepParent);

    BOOL cnrhShowContextMenu(HWND hwndCnr,
                             PRECORDCORE preccSource,
                             HWND hMenu,
                             HWND hwndMenuOwner);

    // flags for cnrhQuerySourceRecord
    #define SEL_WHITESPACE      1
    #define SEL_SINGLESEL       2
    #define SEL_MULTISEL        3
    #define SEL_SINGLEOTHER     4
    #define SEL_NONEATALL       5

    PRECORDCORE cnrhQuerySourceRecord(HWND hwndCnr,
                                      PRECORDCORE preccSource,
                                      PULONG pulSelection);

    PRECORDCORE cnrhQueryNextSelectedRecord(HWND hwndCnr,
                                            PRECORDCORE preccCurrent);

    /*
     *@@category: Helpers\PM helpers\Container helpers\Record relations/iteration
     */

    /* ******************************************************************
     *
     *   Record relations/iteration
     *
     ********************************************************************/

    LONG cnrhQueryRecordIndex(HWND hwndCnr,
                              PRECORDCORE precc);

    BOOL cnrhIsChildOf(HWND hwndCnr,
                       PRECORDCORE precTest,
                       PRECORDCORE precParent);

    /*
     *@@ cnrhQueryParentRecord:
     *      this returns the parent record of precc.
     *
     *      This returns a PRECORDCORE or NULL if no
     *      parent record could be found or -1 if
     *      the container found an error.
     *
     *@@added V0.9.0
     */

    #define cnrhQueryParentRecord(hwndCnr, precc)                \
            (PRECORDCORE)WinSendMsg(hwndCnr, CM_QUERYRECORD,        \
                    (MPARAM)precc,                                  \
                    MPFROM2SHORT(CMA_PARENT, CMA_ITEMORDER))

    /*
     *@@ cnrhQueryFirstChildRecord:
     *      this returns the first child record of precc.
     *
     *      This returns a PRECORDCORE or NULL if no
     *      parent record could be found or -1 if
     *      the container found an error.
     *
     *@@added V0.9.0
     */

    #define cnrhQueryFirstChildRecord(hwndCnr, precc)            \
            (PRECORDCORE)WinSendMsg(hwndCnr, CM_QUERYRECORD,        \
                    (MPARAM)precc,                                  \
                    MPFROM2SHORT(CMA_FIRSTCHILD, CMA_ITEMORDER))

    typedef ULONG XWPENTRY FNCBRECC(HWND, PRECORDCORE, ULONG);
    typedef FNCBRECC *PFNCBRECC;

    ULONG cnrhForAllRecords(HWND hwndCnr,
                            PRECORDCORE preccParent,
                            PFNCBRECC pfncbRecc,
                            ULONG ulUser);

    /* ******************************************************************
     *
     *   Miscellaneous
     *
     ********************************************************************/

    HWND cnrhQueryCnrFromFrame(HWND hwndFrame);

    BOOL cnrhOpenEdit(HWND hwndCnr);

    PDRAGINFO cnrhInitDrag(HWND hwndCnr,
                           PRECORDCORE preccDrag,
                           USHORT usNotifyCode,
                           PSZ   pszRMF,
                           USHORT usSupportedOps);

    #define CODFL_DISABLEDTEXT          0x0001
    #define CODFL_MINIICON              0x0002

    MRESULT cnrhOwnerDrawRecord(MPARAM mp2,
                                ULONG flFlags);

    BOOL cnrhDateTimeDos2Win(DATETIME* pdt,
                             CDATE* pcdate,
                             CTIME* pctime);

    BOOL cnrhDateDos2Win(FDATE* pfd,
                         CDATE* pcd);

    BOOL cnrhTimeDos2Win(FTIME* pft,
                         CTIME* pct);

#endif

#if __cplusplus
}
#endif

