
/*
 *@@sourcefile comctl.h:
 *      header file for comctl.c. See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
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
 *
 *@@include #define INCL_WINWINDOWMGR
 *@@include #define INCL_WINMESSAGEMGR
 *@@include #define INCL_WINDIALOGS
 *@@include #define INCL_WINSTDCNR          // for checkbox containers
 *@@include #define INCL_WININPUT
 *@@include #define INCL_WINSYS
 *@@include #define INCL_WINSHELLDATA
 *@@include #include <os2.h>
 *@@include #include "helpers\comctl.h"
 */

#if __cplusplus
extern "C" {
#endif

#ifndef COMCTL_HEADER_INCLUDED
    #define COMCTL_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Shared stuff
     *
     ********************************************************************/

    MRESULT ctlSendWmControl(HWND hwndControl,
                             USHORT usCode,
                             MPARAM mp2);

    BOOL ctlPostWmControl(HWND hwndControl,
                          USHORT usCode,
                          MPARAM mp2);

    /*
     *@@ CCTLCOLOR:
     *
     *@@added V1.0.1 (2002-11-30) [umoeller]
     */

    typedef struct _CCTLCOLOR
    {
        BOOL    fInheritPP;
        ULONG   ulPP,
                ulSysColor;
    } CCTLCOLOR, *PCCTLCOLOR;

    #define CTLCOL_BGND         0
    #define CTLCOL_FGND         1

    #define CCS_NOSENDCTLPTR            0x0001

    /*
     *@@ DEFWINDOWDATA:
     *
     *@@added V1.0.1 (2002-11-30) [umoeller]
     */

    typedef struct _DEFWINDATA
    {
        HWND        hwnd;
        HAB         hab;

        PFNWP       pDefWindowProc;

        ULONG       flCtl;              // CTL_* flags

        SIZEL       szlWin;             // current window dimensions

        PSZ         pszText;            // window text or NULL

        const CCTLCOLOR *paCtlColors;
        ULONG       cCtlColors;

        PLONG       palColorValues;

    } DEFWINDATA, *PDEFWINDATA;

    VOID ctlInitDWD(HWND hwnd,
                    MPARAM mp2,
                    PDEFWINDATA pdwd,
                    PFNWP pDefWindowProc,
                    ULONG flCtl,
                    const CCTLCOLOR *paCtlColors,
                    ULONG cCtlColors);

    VOID ctlRefreshColors(PDEFWINDATA pdwd);

    LONG ctlQueryColor(PDEFWINDATA pdwd, ULONG ulIndex);

    MRESULT ctlDefWindowProc(PDEFWINDATA pdwd, ULONG msg, MPARAM mp1, MPARAM mp2);

    /* ******************************************************************
     *
     *   "Separator line" control
     *
     ********************************************************************/

    #define WC_CCTL_SEPARATOR       "ComctlSeparator"

    #define SEPS_HORIZONTAL         0x0000
    #define SEPS_VERTICAL           0x0001

    BOOL ctlRegisterSeparatorLine(HAB hab);

    /* ******************************************************************
     *
     *   "Menu button" control
     *
     ********************************************************************/

    VOID ctlDisplayButtonMenu(HWND hwndButton,
                              HWND hwndMenu);

    BOOL ctlMakeMenuButton(HWND hwndButton,
                           HMODULE hmodMenu,
                           ULONG idMenu);

    /* ******************************************************************
     *
     *   Progress bars
     *
     ********************************************************************/

    /*
     *@@ PROGRESSBARDATA:
     *      structure for progress bar data,
     *      saved at QWL_USER window ulong.
     */

    typedef struct _PROGRESSBARDATA
    {
        ULONG      ulNow,
                   ulMax,
                   ulPaintX,
                   ulOldPaintX;
        ULONG      ulAttr;
        PFNWP      OldStaticProc;
    } PROGRESSBARDATA, *PPROGRESSBARDATA;

    #define WM_UPDATEPROGRESSBAR    WM_USER+1000

    // status bar style attributes
    #define PBA_NOPERCENTAGE        0x0000
    #define PBA_ALIGNLEFT           0x0001
    #define PBA_ALIGNRIGHT          0x0002
    #define PBA_ALIGNCENTER         0x0003
    #define PBA_PERCENTFLAGS        0x0003
    #define PBA_BUTTONSTYLE         0x0010

    BOOL ctlProgressBarFromStatic(HWND hwndStatic, ULONG ulAttr);

    /* ******************************************************************
     *
     *   Chart Control
     *
     ********************************************************************/

    /*
     *@@ CHARTDATA:
     *      chart data. This represents the
     *      data to be displayed.
     *
     *      Used with the CHTM_SETCHARTDATA message
     *      and stored within CHARTCDATA (below).
     */

    typedef struct _CHARTDATA
    {
        USHORT          usStartAngle,
                            // for "pie chart" mode, angle to start with (0%).
                            // This must be in the range of 0 to 360 degrees,
                            // with 0 degrees being the rightmost point
                            // of the arc.

                            // All degree values are counter-clockwise from that point.
                            // Example: 90 will start the arc at the top.

                            //                   90ø
                            //                 +++++++
                            //              +           +
                            //            +               +
                            //           +                 +
                            //          +                   +
                            //          +                   +
                            //   180ø   +         X         +     0ø
                            //          +                   +
                            //          +                   +
                            //           +                 +
                            //            +               +
                            //              +           +
                            //                 +++++++
                            //                   270ø

                        usSweepAngle;
                            // the maximum angle to use for 100%, in addition to
                            // usStartAngle.
                            // This must be in the range of 0 to 360 degrees,
                            // with 0 degrees being usStartAngle.
                            // All degree values are counter-clockwise from that point.
                            // Example: Specify usStartAngle = 180 (1) to start the pie
                            // at the left and usSweepAngle = 270 (2) to draw a
                            // three-quarter total pie.

                            //                   90ø
                            //                    ++++
                            //                    +   _ +
                            //                    +  |\   +
                            //                    +    \   +
                            //                    +     \   +
                            //                    +     |   +
                            // (1) 180ø +++++++++++     |   +     0ø
                            //          +              /    +
                            //          +    \       (2)    +
                            //           +    \      /     +
                            //            +    ------     +
                            //              +           +
                            //                 +++++++
                            //                   270ø

        ULONG           cValues;
                            // data item count; the arrays in *padValues and *palColors
                            // (and also *papszDescriptions, if CHS_DESCRIPTIONS is
                            // enabled in CHARTSTYLE) must have this many items.
        double*         padValues;
                            // pointer to an array of double values;
                            // the sum of all these will make up 100%
                            // in the chart. In "pie chart" mode, the
                            // sum of all these values corresponds to
                            // the usSweepAngle angle; in "bar chart" mode,
                            // the sum corresponds to the width of the control.
                            // If this ptr is NULL, the chart displays nothing.
                            // Otherwise, this array must have cValues items.
        LONG*           palColors;
                            // pointer to an array of LONG RGB colors;
                            // each item in this array must correspond
                            // to an item in padValues and specifies the color
                            // to paint the corresponding data item with.
                            // This _must_ be specified if padValues is != NULL.
                            // This array must have cValues items.
        PSZ*            papszDescriptions;
                            // pointer to an array of PSZs containing
                            // data descriptions. If this pointer is
                            // NULL, or CHARTSTYLE.ulStyle does not have
                            // CHS_DESCRIPTIONS set, no texts will be displayed.
                            // Otherwise, this array must have cValues items.
    } CHARTDATA, *PCHARTDATA;

    // chart display mode:
    #define CHS_PIECHART            0x0000
    #define CHS_BARCHART            0x0001

    // chart display flags (CHARTSTYLE.ulStyle):
    #define CHS_SHADOW              0x0100  // (pie chart only) draw shadow
    #define CHS_3D_BRIGHT           0x0200  // (pie chart only)
                                            // // draw 3D block in same color as surface;
                                            // CHARTSTYLE.ulThickness defines thickness
    #define CHS_3D_DARKEN           0x0600  // (pie chart only)
                                            // draw 3D block too, but darker
                                            // compared to surface;
                                            // CHARTSTYLE.ulThickness defines thickness

    #define CHS_DRAWLINES           0x0800  // draw lines between pie slices
                                            // added V0.9.12 (2001-05-03) [umoeller]

    #define CHS_DESCRIPTIONS        0x1000  // show descriptions
    #define CHS_DESCRIPTIONS_3D     0x3000  // same as CHS_DESCRIPTIONS, but shaded

    #define CHS_SELECTIONS          0x4000  // allow data items to be selected using
                                            // mouse and keyboard; this also enables
                                            // WM_CONTROL notifications

    /*
     *@@ CHARTSTYLE:
     *
     */

    typedef struct _CHARTSTYLE
    {
        ULONG           ulStyle;        // CHS_* flags

        ULONG           ulThickness;    // (pie chart only)
                                        // pie thickness (with CHS_3D_xxx) in pixels

        double          dPieSize;       // (pie chart only)
                                        // // size of the pie chart relative to the control
                                        // size. A value of 1 would make the pie chart
                                        // consume all available space. A value of .5
                                        // would make the pie chart consume half of the
                                        // control's space. The pie chart is always
                                        // centered within the control.

        double          dDescriptions;  // (pie chart only)
                                        // position of the slice descriptions on the pie
                                        // relative to the window size. To calculate the
                                        // description positions, the control calculates
                                        // an invisible pie slice again, for which this
                                        // value is used. So a value of 1 would make the
                                        // descriptions appear on the outer parts of the
                                        // window (not recommended). A value of .5 would
                                        // make the descriptions appear in the center of
                                        // an imaginary line between the pie's center
                                        // and the pie slice's outer border.
                                        // This should be chosen in conjunction with
                                        // dPieSize as well. If this is equal to dPieSize,
                                        // the descriptions appear on the border of the
                                        // slice. If this is half dPieSize, the descriptions
                                        // appear in the center of the pie slice. If this
                                        // is larger than dPieSize, the descriptions appear
                                        // outside the slice.
    } CHARTSTYLE, *PCHARTSTYLE;

    HBITMAP ctlCreateChartBitmap(HPS hpsMem,
                                 LONG lcx,
                                 LONG lcy,
                                 PCHARTDATA pChartData,
                                 PCHARTSTYLE pChartStyle,
                                 LONG lBackgroundColor,
                                 LONG lTextColor,
                                 HRGN* paRegions);

    BOOL ctlChartFromStatic(HWND hwndStatic);

    #define CHTM_SETCHARTDATA      (WM_USER + 2)

    #define CHTM_SETCHARTSTYLE     (WM_USER + 3)

    #define CHTM_ITEMFROMPOINT     (WM_USER + 4)

    #define CHTM_SETEMPHASIS       (WM_USER + 5)

    // WM_CONTROL notification codes

    /*
     *@@ CHTN_EMPHASISCHANGED:
     *      WM_CONTROL notification code sent (!)
     *      by a chart control to its owner when
     *      selections change in the control.
     *
     *      This is only sent if the CHS_SELECTIONS
     *      style bit is set in the control.
     *
     *      Parameters:
     *
     *      --  USHORT SHORT1FROMMP(mp1): usid (control ID).
     *      --  USHORT SHORT2FROMMP(mp1): CHTN_EMPHASISCHANGED.
     *
     *      --  mp2: pointer to EMPHASISNOTIFY structure.
     *
     *      Note: The control only sends one such notification,
     *      even if an old selection was undone. That is, if
     *      item 1 was previously selected and item 2 is then
     *      selected, only one notification for item 2 is sent.
     *
     *@@added V0.9.12 (2001-05-03) [umoeller]
     */

    #define CHTN_EMPHASISCHANGED       1001

    /*
     *@@ CHTN_CONTEXTMENU:
     *      WM_CONTROL notification code sent (!)
     *      by a chart control to its owner when
     *      a context menu was requested on the
     *      control.
     *
     *      This is only sent if the CHS_SELECTIONS
     *      style bit is set in the control.
     *
     *      Parameters:
     *
     *      --  USHORT SHORT1FROMMP(mp1): usid (control ID).
     *      --  USHORT SHORT2FROMMP(mp1): CHTN_EMPHASISCHANGED.
     *
     *      --  mp2: pointer to EMPHASISNOTIFY structure.
     *          If the context menu was requested on a chart
     *          slice, lIndex has the index of the slice.
     *          Otherwise (e.g. whitespace), lIndex will
     *          be -1.
     *
     *@@added V0.9.12 (2001-05-03) [umoeller]
     */

    #define CHTN_CONTEXTMENU            1002

    /*
     *@@ CHTN_ENTER:
     *      WM_CONTROL notification code sent (!)
     *      by a chart control to its owner when
     *      the user double-clicked on a data item.
     *
     *      This is only sent if the CHS_SELECTIONS
     *      style bit is set in the control.
     *
     *      Parameters:
     *
     *      --  USHORT SHORT1FROMMP(mp1): usid (control ID).
     *      --  USHORT SHORT2FROMMP(mp1): CHTN_ENTER.
     *
     *      --  mp2: pointer to EMPHASISNOTIFY structure.
     *          If the double click occurred on a chart
     *          slice, lIndex has the index of the slice.
     *          Otherwise (e.g. whitespace), lIndex will
     *          be -1.
     *
     *@@added V0.9.12 (2001-05-03) [umoeller]
     */

    #define CHTN_ENTER                  1003

    /*
     *@@ CHTN_SETFOCUS:
     *      WM_CONTROL notification code sent (!)
     *      by a chart control to its owner when
     *      gaining focus.
     *
     *      Parameters:
     *
     *      --  USHORT SHORT1FROMMP(mp1): usid (control ID).
     *      --  USHORT SHORT2FROMMP(mp1): CHTN_SETFOCUS.
     *
     *      --  mp2: pointer to EMPHASISNOTIFY structure.
     *
     *@@added V0.9.20 (2002-07-17) [pr]
     */

    #define CHTN_SETFOCUS               1004

    /*
     *@@ CHTN_KILLFOCUS:
     *      WM_CONTROL notification code sent (!)
     *      by a chart control to its owner when
     *      losing focus.
     *
     *      Parameters:
     *
     *      --  USHORT SHORT1FROMMP(mp1): usid (control ID).
     *      --  USHORT SHORT2FROMMP(mp1): CHTN_KILLFOCUS.
     *
     *      --  mp2: pointer to EMPHASISNOTIFY structure.
     *
     *@@added V0.9.20 (2002-07-17) [pr]
     */

    #define CHTN_KILLFOCUS              1005

    /*
     *@@ EMPHASISNOTIFY:
     *      structure used with CHTN_EMPHASISCHANGED,
     *      CHTN_ENTER, and CHTN_CONTEXTMENU.
     *
     *@@added V0.9.12 (2001-05-03) [umoeller]
     */

    typedef struct _EMPHASISNOTIFY
    {
        HWND    hwndSource;
                // window handle of the chart control
        ULONG   ulEmphasis;
                // with CHTN_EMPHASISCHANGED: emphasis which has changed
                // (0 for selection, 1 for source emphasis).
                // Otherwise undefined.
        LONG    lIndex;
                // with CHTN_EMPHASISCHANGED: index of the data
                // item for which emphasis has changed (counting
                // from 0); if -1, a previous emphasis has been undone.
                // With CHTN_CONTEXTMENU and CHTN_ENTER, index of the
                // data item for which the request occurred, or -1 for
                // a request on the control's whitespace.
        POINTL  ptl;
                // exact window coordinates where mouse
                // click occurred (e.g. for context menu).
                // This is undefined (-1) if the event was not
                // caused by a mouse click.
    } EMPHASISNOTIFY, *PEMPHASISNOTIFY;

    /* ******************************************************************
     *
     *   Super Combo Box
     *
     ********************************************************************/

    BOOL ctlComboFromEntryField(HWND hwnd,
                                ULONG flStyle);

    /* ******************************************************************
     *
     *   Split bars
     *
     ********************************************************************/

    #define WC_SPLITWINDOW          "SplitWindowClass"

    #define SBCF_VERTICAL            0x0000
    #define SBCF_HORIZONTAL          0x0001

    #define SBCF_PERCENTAGE          0x0002
    #define SBCF_3DSUNK              0x0100
    #define SBCF_3DEXPLORERSTYLE     0x0200
                // new with V1.0.0 (2002-08-21) [umoeller]
                // this simulates the Warp 4 entry field margins around the
                // right split window, but leaves the left window flat
    #define SBCF_MOVEABLE            0x1000

    /*
     *@@ SPLITBARCDATA:
     *      split bar creation data
     */

    typedef struct _SPLITBARCDATA
    {
        ULONG   ulSplitWindowID;
                    // window ID of the split window
        ULONG   ulCreateFlags;
                    // split window style flags.
                    // One of the following:
                    // -- SBCF_VERTICAL: the split bar will be vertical.
                    // -- SBCF_HORIZONTAL: the split bar will be horizontal.
                    // plus any or none of the following:
                    // -- SBCF_PERCENTAGE: lPos does not specify absolute
                    //      coordinates, but a percentage of the window
                    //      width or height. In that case, the split
                    //      bar position is not fixed, but always recalculated
                    //      as a percentage.
                    //      Otherwise, the split bar will be fixed.
                    // -- SBCF_3DSUNK: draw a "sunk" 3D frame around the
                    //      split windows.
                    // -- SBCF_MOVEABLE: the split bar may be moved with
                    //      the mouse.
        LONG    lPos;
                    // position of the split bar within hwndParentAndOwner.
                    // If SBCF_PERCENTAGE, this has the percentage;
                    // otherwise:
                    //      if this value is positive, it's considered
                    //      an offset from the left/bottom of the frame;
                    //      if it's negative, it's from the right
        ULONG   ulLeftOrBottomLimit,
                ulRightOrTopLimit;
                    // when moving the split bar (SBCF_MOVEABLE), these
                    // are the minimum and maximum values.
                    // ulLeftOrBottomLimit is the left (or bottom) limit,
                    // ulRightOrTopLimit is the right (or top) limit.
                    // Both are offsets in window coordinates from the
                    // left (or bottom) and right (or top) boundaries of
                    // the split window. If both are 0, the whole split
                    // window is allowed (not recommended).
        HWND    hwndParentAndOwner;
                    // the owner and parent of the split bar
                    // and other windows; this must be the FID_CLIENT
                    // of a frame or another split window (when nesting)
    } SPLITBARCDATA, *PSPLITBARCDATA;

    /*
     *@@ SPLITBARDATA:
     *      internal split bar data,
     *      stored in QWL_USER window ulong
     */

    typedef struct _SPLITBARDATA
    {
        SPLITBARCDATA   sbcd;
        PFNWP           OldStaticProc;
        HPOINTER        hptrOld,        // old pointer stored upon WM_MOUSEMOVE
                        hptrMove;       // PM move pointer, either vertical or horizontal
        BOOL            fCaptured;
        POINTS          ptsMousePos;
        POINTL          ptlDrawLineFrom,
                        ptlDrawLineTo;
        HPS             hpsTrackBar;
        HWND            hwndLinked1,
                            // the left/bottom window to link
                        hwndLinked2;
                            // the right/top window to link
        LONG            lcol3DDark,
                        lcol3DLight,
                        lcolInactiveBorder;
    } SPLITBARDATA, *PSPLITBARDATA;

    #define ID_SPLITBAR  5000           // fixed ID of the split bar
                                        // (child of split window)

    /*
     *@@ SPLM_SETLINKS:
     *      this specifies the windows which the
     *      split window will link. This updates
     *      the internal SPLITBARDATA and changes
     *      the parents of the two subwindows to
     *      the split window.
     *
     *      Parameters:
     *      HWND mp1:   left or bottom subwindow
     *      HWND mp2:   right or top  subwindow
     */

    #define SPLM_SETLINKS       (WM_USER + 500)

    HWND ctlCreateSplitWindow(HAB hab,
                              PSPLITBARCDATA psbcd);

    BOOL ctlUpdateSplitWindow(HWND hwndSplit);

    BOOL ctlSetSplitFrameWindowPos(HWND hwndFrame,
                                   HWND hwndInsertBehind,
                                   LONG x,
                                   LONG y,
                                   LONG cx,
                                   LONG cy,
                                   ULONG fl,
                                   HWND *pahwnd,
                                   ULONG cbhwnd);

    LONG ctlQuerySplitPos(HWND hwndSplit);

    /* ******************************************************************
     *
     *   Subclassed Static Bitmap Control
     *
     ********************************************************************/

    // flags for ANIMATIONDATA.ulFlags
    #define ANF_ICON             0x0001
    #define ANF_BITMAP           0x0002
    #define ANF_PROPORTIONAL     0x0004

    /*
     *@@ ANIMATIONDATA:
     *      this structure gets stored in QWL_USER
     *      before subclassing the static control
     *
     *@@changed V0.9.0: added fields for bitmap support
     *@@changed V0.9.16 (2001-10-15) [umoeller]: added lIconSize
     *@@changed V0.9.19 (2002-06-18) [umoeller]: added szlIcon
     */

    typedef struct _ANIMATIONDATA
    {
        // the following need to be initialized before
        // subclassing
        HAB         hab;                // (added V0.9.0)
        ULONG       ulFlags;
                // one of the following:
                // -- ANF_ICON:   display icons
                // -- ANF_BITMAP: display bitmaps
                // -- ANF_BITMAP | ANF_PROPORTIONAL: display bitmaps, but preserve proportions
        SIZEL       szlIcon;            // system icon size V0.9.19 (2002-06-18) [umoeller]
        RECTL       rclIcon;            // size of static control
        PFNWP       OldStaticProc;      // original WC_STATIC wnd proc

        // the following are set by fnwpSubclassedStatic upon
        // receiving SM_SETHANDLE (in all modes) or later
        HBITMAP     hbm,                // bitmap to be painted upon WM_PAINT
                    hbmHalftoned;       // bitmap in case of WS_DISABLED (added V0.9.0)
        HPOINTER    hptr;               // icon handle passed to SM_SETHANDLE
        HBITMAP     hbmSource;          // bitmap handle passed to SM_SETHANDLE
                                        // (this can be deleted later) (added V0.9.0)

        // the following need to be initialized
        // for icon mode only (ANF_ICON)
        ULONG       ulDelay;            // delay per animation step in ms
        USHORT      usAniCurrent;       // current animation step (>= 0)

        USHORT      usAniCount;         // no. of animation steps
        HPOINTER    ahptrAniIcons[1];   // variable-size array of animation steps;
                                        // there must be usAniCount items
    } ANIMATIONDATA, *PANIMATIONDATA;

    PANIMATIONDATA ctlPrepareStaticIcon(HWND hwndStatic, USHORT usAnimCount);

    BOOL ctlPrepareAnimation(HWND hwndStatic,
                             USHORT usAnimCount,
                             HPOINTER *pahptr,
                             ULONG ulDelay,
                             BOOL fStartAnimation);

    BOOL ctlStartAnimation(HWND hwndStatic);

    BOOL ctlStopAnimation(HWND hwndStatic);

    PANIMATIONDATA ctlPrepareStretchedBitmap(HWND hwndStatic,
                                             BOOL fPreserveProportions);


    /* ******************************************************************
     *
     *   "Tooltip" control
     *
     ********************************************************************/

    #define WC_CCTL_TOOLTIP         "ComctlTooltipClass"
                    // define identifier changed V1.0.1 (2002-11-30) [umoeller]

    // addt'l tooltip window styles: use lower 16 bits
    #define TTS_ALWAYSTIP           0x0001
    #define TTS_NOPREFIX            0x0002
    // non-Win95 flags
    #define TTS_ROUNDED             0x0004
    #define TTS_SHADOW              0x0008

    // TOOLINFO.uFlags flags (ORed)
    // #define TTF_IDISHWND            0x0001
                // V0.9.7 (2001-01-03) [umoeller]: removed this win95 crap
    // #define TTF_CENTERBELOW         0x0002
    // #define TTF_CENTERABOVE         0x0004
    // #define TTF_RTLREADING          0x0004
                // V0.9.7 (2001-01-03) [umoeller]: removed this win95 crap

    #define TTF_SUBCLASS            0x0008
    // non-Win95 flags
    #define TTF_SHYMOUSE            0x0010

    // new flags with V0.9.7 (2001-01-20) [umoeller]
    #define TTF_CENTER_X_ON_TOOL    0x0020
    #define TTF_POS_Y_ABOVE_TOOL    0x0040
    #define TTF_POS_Y_BELOW_TOOL    0x0080

    #define PSZ_TEXTCALLBACK      (PSZ)-1

    #define TT_SHADOWOFS            10
    #define TT_ROUNDING             8

    /*
     *@@ TOOLINFO:
     *      info structure to register a tool with a
     *      tooltip control. Used with TTM_ADDTOOL
     *      and many other TTM_* messages.
     *
     *@@changed V0.9.7 (2001-01-03) [umoeller]: removed all that win95 crap
     */

    typedef struct _TOOLINFO
    {
        ULONG   ulFlags;
                    // in: flags for the tool, any combination of:
                    // -- TTF_SUBCLASS: Indicates that the tooltip control should
                    //      subclass hwndTool to intercept messages,
                    //      such as WM_MOUSEMOVE. See TTM_RELAYEVENT.
                    // -- TTF_SHYMOUSE: shy away from mouse pointer;
                    //      always position the tool tip such that it is never
                    //      covered by the mouse pointer (for readability);
                    //      added V0.9.1 (2000-02-04) [umoeller]
                    // -- TTF_CENTER_X_ON_TOOL: position tooltip X so that
                    //      it's centered on the tool (doesn't affect Y)
                    // -- TTF_POS_Y_ABOVE_TOOL: position tooltip Y above
                    //      the tool; cannot be used with TTF_POS_Y_BELOW_TOOL
                    // -- TTF_POS_Y_BELOW_TOOL: position tooltip Y below
                    //      the tool; cannot be used with TTF_POS_Y_ABOVE_TOOL
        HWND    hwndToolOwner;
                    // in: handle to the window that contains the tool. If
                    // pszText includes the PSZ_TEXTCALLBACK value, this
                    // member identifies the window that receives TTN_NEEDTEXT
                    // notification messages.
        HWND    hwndTool;
                    // in: window handle of the tool.
        PSZ     pszText;
                    // in: pointer to the buffer that contains the text for the
                    // tool (if the hiword is != NULL), or identifier of the string
                    // resource that contains the text (if the hiword == NULL).
                    // If this member is set to the PSZ_TEXTCALLBACK value,
                    // the control sends the TTN_NEEDTEXT notification message to
                    // hwndToolOwner to retrieve the text.
    } TOOLINFO, *PTOOLINFO;

    /*
     *  tooltip messages
     *
     */

    #define TTM_FIRST                   (WM_USER + 1000)

    #define TTM_ACTIVATE                (TTM_FIRST + 1)

    #define TTM_ADDTOOL                 (TTM_FIRST + 2)

    #define TTM_DELTOOL                 (TTM_FIRST + 3)

    #define TTM_NEWTOOLRECT             (TTM_FIRST + 4)

    #define TTM_RELAYEVENT              (TTM_FIRST + 5)

    // flags for TTM_SETDELAYTIME
    #define TTDT_AUTOMATIC              1
    #define TTDT_AUTOPOP                2
    #define TTDT_INITIAL                3
    #define TTDT_RESHOW                 4

    #define TTM_GETDELAYTIME            (TTM_FIRST + 6)
                // added V0.9.12 (2001-04-28) [umoeller]

    #define TTM_SETDELAYTIME            (TTM_FIRST + 7)

    #define TTFMT_PSZ           0x01
    #define TTFMT_STRINGRES     0x02

    /*
     *@@ TOOLTIPTEXT:
     *      identifies a tool for which text is to
     *      be displayed and receives the text for
     *      the tool. The tool must fill all fields
     *      of this structure.
     *
     *      This structure is used with the TTN_NEEDTEXT
     *      notification.
     *
     *@@changed V0.9.7 (2001-01-03) [umoeller]: got rid of this win95 crap
     */

    typedef struct _TOOLTIPTEXT
    {
        HWND    hwndTooltip;
                    // in: tooltip control who's sending this.
        HWND    hwndTool;
                    // in: tool for which the text is needed.
        ULONG   ulFormat;
                    // out: one of:
                    // -- TTFMT_PSZ: pszText contains the new, zero-terminated string.
                    // -- TTFMT_STRINGRES: hmod and idResource specify a string resource
                    //    to be loaded.
        PSZ     pszText;
                    // out: with TTFMT_PSZ, pointer to a string that contains the
                    // tool text. Note that this is not copied into the tooltip...
                    // so this must point to a static buffer that is valid while
                    // the tooltip is showing.
        HMODULE hmod;
                    // out: with TTFMT_STRINGRES, the module handle of the resource.
        ULONG   idResource;
                    // out: with TTFMT_STRINGRES, the string resource ID.
    } TOOLTIPTEXT, *PTOOLTIPTEXT;

    #define TTM_GETTEXT                 (TTM_FIRST + 8)

    #define TTM_UPDATETIPTEXT           (TTM_FIRST + 9)

    /*
     *@@ TT_HITTESTINFO:
     *      contains information that a tooltip control uses to determine whether
     *      a point is in the bounding rectangle of the specified tool. If the point
     *      is in the rectangle, the structure receives information about the tool.
     *
     *      This structure is used with the TTM_HITTEST message.
     */

    typedef struct _TT_HITTESTINFO
    {
        HWND hwnd;      // in: handle to the tool or window with the specified tool.
        POINTL /* POINT */ pt;
                        // in: client coordinates of the point to test (Win95: POINT)
        TOOLINFO ti;    // out:  receives information about the specified tool.
    } TTHITTESTINFO, *PHITTESTINFO;

    #define TTM_HITTEST                 (TTM_FIRST + 10)

    #define TTM_WINDOWFROMPOINT         (TTM_FIRST + 11)

    #define TTM_ENUMTOOLS               (TTM_FIRST + 12)

    #define TTM_GETCURRENTTOOL          (TTM_FIRST + 13)

    #define TTM_GETTOOLCOUNT            (TTM_FIRST + 14)

    #define TTM_GETTOOLINFO             (TTM_FIRST + 15)

    #define TTM_SETTOOLINFO             (TTM_FIRST + 16)

    // non-Win95 messages

    #define TTM_SHOWTOOLTIPNOW          (TTM_FIRST + 17)

    /*
     *  tooltip notification codes (WM_CONTROL)
     *
     */

    /*
     *@@ TTN_NEEDTEXT:
     *      notification code used with WM_CONTROL when a tooltip
     *      needs a tooltip text for a tool.
     *
     *      Parameters:
     *
     *      -- SHORT1FROMMP(mp1) usID: ID of the tooltip control).
     *
     *      -- SHORT2FROMMP(mp1) usNotifyCode: TTN_NEEDTEXT.
     *
     *      -- PTOOLTIPTEXT mp2: pointer to a TOOLTIPTEXT structure.
     *              hwndTool identifies the tool for which text is needed.
     *
     *      This notification message is sent to the window specified
     *      in the hwndToolOwner member of the TOOLINFO structure for the tool.
     *      This notification is sent only if the PSZ_TEXTCALLBACK
     *      value is specified when the tool is added to a tooltip control.
     *
     *      To specify the text, the target window (hwndToolOwner) must:
     *
     *      1.  Set TOOLTIPTEXT.ulFormat to one of the format flags.
     *
     *      2.  Fill the corresponding field(s) in TOOLTIPTEXT.
     *
     *      Specifying PSZ_TEXTCALLBACK in TOOLINFO.lpszText with
     *      TTM_ADDTOOL is the only way under OS/2 to have strings
     *      displayed which are longer than 256 characters, since
     *      string resources are limited to 256 characters with OS/2.
     *      It is the responsibility of the application to set the
     *      pszText member to a _static_ string buffer which holds
     *      the string for the tool. A common error would be to have
     *      that member point to some variable which has only been
     *      allocated on the stack... this will lead to problems.
     */

    #define TTN_NEEDTEXT        1000

    /*
     *@@ TTN_SHOW:
     *      control notification sent with the WM_NOTIFY (Win95)
     *      and WM_CONTROL (OS/2) messages.
     *
     *      Parameters (OS/2, incompatible with Win95):
     *      -- mp1 USHORT usID;
     *             USHORT usNotifyCode == TTN_NEEDTEXT
     *      -- ULONG mp2: PTOOLINFO of the tool for which the
     *             tool is about to be displayed.
     *
     *      Return value: always 0.
     *
     *      The TTN_SHOW notification message notifies the owner window
     *      that a tooltip is about to be displayed.
     */

    #define TTN_SHOW            1001

    /*
     *@@ TTN_POP:
     *      control notification sent with the WM_NOTIFY (Win95)
     *      and WM_CONTROL (OS/2) messages.
     *
     *      Parameters (OS/2, incompatible with Win95):
     *      -- mp1 USHORT usID;
     *             USHORT usNotifyCode == TTN_NEEDTEXT
     *      -- ULONG mp2: PTOOLINFO of the tool for which the
     *             tooltip was visible.
     *
     *      Return value: always 0.
     *
     *      The TTN_SHOW notification message notifies the owner window
     *      that a tooltip is about to be hidden.
     */

    #define TTN_POP             1002

    BOOL ctlRegisterTooltip(HAB hab);

    MRESULT EXPENTRY ctl_fnwpTooltip(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

    /* ******************************************************************
     *
     *   "Tool bar button" control
     *
     ********************************************************************/

    #define WC_CCTL_TOOLBAR         "ComctlToolbarClass"

    // toolbar button styles: use lower 16 bits
    #define TBBS_BIGICON            0x0001
    #define TBBS_MINIICON           0x0002
    #define TBBS_BITMAP             0x0004
    #define TBBS_TEXT               0x0008

    #define TBBS_CHECK              0x0010
    #define TBBS_RADIO              0x0020
    #define TBBS_CHECKINITIAL       0x0040

    #define TBBS_AUTORESIZE         0x0100
    #define TBBS_HILITE             0x0200
    #define TBBS_FLAT               0x0400

    #define TBBS_COMMAND            0x1000
    #define TBBS_SYSCOMMAND         0x2000
    #define TBBS_DROPMNEMONIC       0x4000

    // tool bar button messages

    #define TBBM_CHECK              (WM_USER + 1)

    #define TBBM_QUERYCHECK         (WM_USER + 2)

    /*
     *@@ XBUTTONDATA:
     *      paint data for ctlPaintTBButton.
     *
     *@@added V0.9.13 (2001-06-21) [umoeller]
     */

    typedef struct _XBUTTONDATA
    {
        DEFWINDATA  dwd;              // color for center (button background)

        SIZEL       szlIconOrBitmap;

        HPOINTER    hptr;               // icon to paint or NULLHANDLE

    } XBUTTONDATA, *PXBUTTONDATA;

    /*
     *@@ XBUTTONSTATE:
     *
     *@@added V1.0.1 (2002-11-30) [umoeller]
     */

    typedef struct _XBUTTONSTATE
    {
        BOOL        fMB1Pressed;            // if TRUE, mouse button is currently pressed
        BOOL        fIgnoreMB1Up;
        BOOL        fPaintButtonSunk;       // if TRUE, button control is to be painted "down"
        BOOL        fMouseCaptured;         // if TRUE, mouse is currently captured
        BOOL        fMouseOver;
    } XBUTTONSTATE, *PXBUTTONSTATE;

    // the following styles are only for painting and not stored
    #define TBBS_BACKGROUND          0x00010000
    #define TBBS_INUSE               0x00020000

    VOID ctlPaintTBButton(HPS hps,
                          ULONG fl,
                          PXBUTTONDATA pbd,
                          PXBUTTONSTATE pbs);

    MRESULT EXPENTRY ctl_fnwpToolbarButton(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

    /* ******************************************************************
     *
     *   "Tool bar" control
     *
     ********************************************************************/

    #define WC_CCTL_TBBUTTON        "ComCtlToolbarButton"
                    // define identifier changed V1.0.1 (2002-11-30) [umoeller]

    // tool bar window styles: use lower 16 bits
    #define TBS_TOOLTIPS            0x0001
    #define TBS_AUTORESIZE          0x0002

    /*
     *@@ TOOLBARCONTROL:
     *
     *@@added V1.0.1 (2002-11-30) [umoeller]
     */

    typedef struct _TOOLBARCONTROL
    {
        PCSZ        pcszClass;
        PCSZ        pcszTitle;
        ULONG       flStyle;
        ULONG       id;
        LONG        cx,
                    cy;
    } TOOLBARCONTROL, *PTOOLBARCONTROL;

    /*
     *@@ TOOLBARCDATA:
     *
     *@@added V1.0.1 (2002-11-30) [umoeller]
     */

    typedef struct _TOOLBARCDATA
    {
        USHORT      cb;

        HWND        hwndControlsOwner;

        ULONG       cControls;
        PTOOLBARCONTROL patbc;

    } TOOLBARCDATA, *PTOOLBARCDATA;

    #define TBM_ADDCONTROLS         (WM_USER + 1)

    /*
     *@@ TBN_RESIZED:
     *      notification code posted with WM_CONTROL when
     *      a tool bar has resized itself, e.g. because
     *      controls were added or removed.
     *
     *      Parameters:
     *
     *      --  SHORT1FROMMP(mp1): ID of the tool bar control.
     *
     *      --  SHORT2FROMMP(mp1): TBN_RESIZED code.
     *
     *      --  mp2: unused.
     *
     *@@added V1.0.1 (2002-11-30) [umoeller]
     */

    #define TBN_RESIZED             1

    MRESULT EXPENTRY ctl_fnwpToolbar(HWND hwndToolbar, ULONG msg, MPARAM mp1, MPARAM mp2);

    BOOL ctlRegisterToolbar(HAB hab);

    HWND ctlCreateToolBar(HWND hwndParent,
                          HWND hwndOwner,
                          ULONG flStyle,
                          HWND hwndControlsOwner,
                          ULONG cControls,
                          PTOOLBARCONTROL patbc);

    /* ******************************************************************
     *
     *   Extended frame
     *
     ********************************************************************/

    #define FID_TOOLBAR             0x8101
    #define FID_STATUSBAR           0x8100

    /*
     *@@ EXTFRAMECDATA:
     *
     *@@added V0.9.16 (2001-09-29) [umoeller]
     */

    typedef struct _EXTFRAMECDATA
    {
        PSWP        pswpFrame;          // in: frame wnd pos (can be NULL)
        ULONG       flFrame;            // in: standard FCF_* flags
        ULONG       flExtFrame;         // in: XFCF_* flags
                #define XFCF_STATUSBAR          0x0001
                            // create a status bar for the frame
                #define XFCF_TOOLBAR            0x0002
                            // create a tool bar for the frame
                #define XFCF_FORCETBOWNER       0x0004
                            // if set, we will enfore that all toolbar controls have
                            // the main frame as their owner, otherwise they'll have
                            // the frame's client, if one exists
        ULONG       flStyleFrame;       // in: frame style (WS_* flags, e.g. WS_VISIBLE | WS_ANIMATE)
        PCSZ        pcszFrameTitle;     // in: frame title (title bar)
        ULONG       ulResourcesID;      // in: according to FCF_* flags

        PCSZ        pcszClassClient;    // in: client class name or NULL for no client
        ULONG       flStyleClient;      // in: client style (WS_* flags, e.g. WS_VISIBLE)

        ULONG       ulID;               // in: frame window ID
        PVOID       pClientCtlData;     // in: pCtlData structure pointer for client

        HINI        hiniSaveWinPos;     // in: HINI for saving window position or NULLHANDLE
        PCSZ        pcszIniApp,
                    pcszIniKey;

        ULONG       cTBControls;        // in: count of tool bar controls in paTBControls or 0
        PTOOLBARCONTROL paTBControls;

    } EXTFRAMECDATA, *PEXTFRAMECDATA;

    /*
     *@@ XFRAMECONTROLS:
     *
     *@@added V1.0.1 (2002-11-30) [umoeller]
     */

    typedef struct _XFRAMECONTROLS
    {
        PFNWP           pfnwpOrig;      // original frame wnd proc from subclassing

        HWND            hwndToolBar,
                        hwndStatusBar;

        LONG            lToolBarHeight,
                        lStatusBarHeight;

    } XFRAMECONTROLS, *PXFRAMECONTROLS;

    /*
     *@@ EXTFRAMEDATA:
     *
     *@@added V0.9.16 (2001-09-29) [umoeller]
     */

    typedef struct _EXTFRAMEDATA
    {
        EXTFRAMECDATA   CData;

        XFRAMECONTROLS  xfc;

        PVOID           pvUser;         // more data for user (e.g. for additional subclassing)

    } EXTFRAMEDATA, *PEXTFRAMEDATA;

    MRESULT ctlFormatExtFrame(HWND hwndFrame,
                              const XFRAMECONTROLS *pxfc,
                              MPARAM mp1,
                              MPARAM mp2);

    MRESULT ctlCalcExtFrameRect(HWND hwndFrame,
                                const XFRAMECONTROLS *pxfc,
                                MPARAM mp1,
                                MPARAM mp2);

    HWND ctlCreateStatusBar(HWND hwndFrame,
                            HWND hwndOwner,
                            const char *pcszText,
                            const char *pcszFont,
                            LONG lColor);

    HWND ctlCreateStdWindow(PEXTFRAMECDATA pData,
                            PHWND phwndClient);

    /* ******************************************************************
     *
     *   Checkbox container record cores
     *
     ********************************************************************/

    BOOL ctlDrawCheckbox(HPS hps,
                         LONG x,
                         LONG y,
                         USHORT usRow,
                         USHORT usColumn,
                         BOOL fHalftoned);

    #ifdef INCL_WINSTDCNR

        /*
         *@@ CN_RECORDCHECKED:
         *      extra WM_CONTROL notification code.
         *      See ctlMakeCheckboxContainer for
         *      details.
         */

        #define CN_RECORDCHECKED            999

        /*
         *@@ CHECKBOXRECORDCORE:
         *      extended record core structure used
         *      with checkbox containers. See
         *      ctlMakeCheckboxContainer for details.
         *
         *      The check box painting is determined
         *      by the following flags:
         *
         *      -- Only if (ulStyle & WS_VISIBLE), the check
         *         box is painted at all. Otherwise hptrIcon
         *         is painted. (Is this true?)
         *
         *      -- In that case, usCheckState determines
         *         whether the check box is painted checked,
         *         unchecked, or indeterminate.
         *
         *      -- In addition, if RECORDCORE.flRecordAttr
         *         has the CRA_DISABLED bit set, the check
         *         box is painted disabled (halftoned).
         *
         *@@changed V0.9.9 (2001-03-27) [umoeller]: made item id a ULONG
         */

        typedef struct _CHECKBOXRECORDCORE
        {
            RECORDCORE      recc;
                        // standard record core structure
            ULONG           ulStyle;
                        // any combination of the following:
                        // -- WS_VISIBLE
                        // -- none or one of the following:
                        //      BS_AUTOCHECKBOX, BS_AUTO3STATE, BS_3STATE
                        // Internally, we use BS_BITMAP to identify
                        // the depressed checkbox button.
            ULONG           ulItemID;
                        // this identifies the record; must be
                        // unique within the container
                        // changed V0.9.9 (2001-03-27) [umoeller]: turned USHORT into ULONG
            USHORT          usCheckState;
                        // current check state as with checkboxes
                        // (0, 1, or 2 for tri-state).
            HPOINTER        hptrIcon;
                        // if this is != NULLHANDLE, this icon
                        // will always be used for painting,
                        // instead of the default check box
                        // bitmaps. Useful for non-auto check
                        // box records to implement something
                        // other than checkboxes.
        } CHECKBOXRECORDCORE, *PCHECKBOXRECORDCORE;

        /*
         *@@ CHECKBOXCNROWNER:
         *
         *
         *@@added V0.9.0 (99-11-28) [umoeller]
         */

        typedef struct _CHECKBOXCNROWNER
        {
            HWND        hwndCnr;            // container window handle
            USHORT      usCnrID;            // container item ID
            HWND        hwndOwner;          // owner of that container
            PFNWP       pfnwpCnrOrig;       // original window proc of hwndCnr
            PFNWP       pfnwpOwnerOrig;     // original window proc of hwndOwner

            HAB         habCnr;

            PCHECKBOXRECORDCORE preccClicked;   // != NULL if mb1 is currently down on recc
            PCHECKBOXRECORDCORE preccSpace;     // != NULL if space key is down with recc
            RECTL       rclReccClicked;     // rectangle of that record
        } CHECKBOXCNROWNER, *PCHECKBOXCNROWNER;

        MRESULT ctlDrawCheckBoxRecord(MPARAM mp2);

        ULONG ctlQueryCheckboxSize(VOID);

        VOID ctlInitCheckboxContainer(HWND hwndCnr);

        BOOL ctlMakeCheckboxContainer(HWND hwndCnrOwner,
                                      USHORT usCnrID);

        PCHECKBOXRECORDCORE ctlFindCheckRecord(HWND hwndCnr,
                                               ULONG ulItemID);

        BOOL ctlSetRecordChecked(HWND hwndCnr,
                                 ULONG ulItemID,
                                 USHORT usCheckState);

        ULONG ctlQueryRecordChecked(HWND hwndCnr,
                                    ULONG ulItemID,
                                    USHORT usCheckState);

        BOOL ctlEnableRecord(HWND hwndCnr,
                             ULONG ulItemID,
                             BOOL fEnable);
    #endif

    /* ******************************************************************
     *
     *   Hotkey entry field
     *
     ********************************************************************/

    /*
     *@@ EN_HOTKEY:
     *      extra notification code with WM_CONTROL
     *      and subclassed hotkey entry fields.
     *      This is SENT to the entry field's owner
     *      every time a key is pressed. Note that
     *      this is only sent for key-down events
     *      and if all the KC_DEADKEY | KC_COMPOSITE | KC_INVALIDCOMP
     *      flags are not set.
     *
     *      WM_CONTROL parameters in this case:
     *      -- mp1: USHORT id,
     *              USHORT usNotifyCode == EN_HOTKEY
     *      -- mp2: PHOTKEYNOTIFY struct pointer
     *
     *      The receiving owner must check if the key
     *      combo described in HOTKEYNOTIFY makes up
     *      a valid hotkey and return a ULONG composed
     *      of the following flags:
     *
     *      -- HEFL_SETTEXT: if this is set, the text
     *              of the entry field is set to the
     *              text in HOTKEYNOTIFY.szDescription.
     *
     *      -- HEFL_FORWARD2OWNER: if this is set, the
     *              WM_CHAR message is instead passed
     *              to the owner. Use this for the tab
     *              key and such.
     *
     *@@added V0.9.1 (99-12-19) [umoeller]
     *@@changed V0.9.4 (2000-08-03) [umoeller]: added HEFL_* flags
     */

    #define EN_HOTKEY           0x1000

    #define HEFL_SETTEXT        0x0001
    #define HEFL_FORWARD2OWNER  0x0002

    typedef struct _HOTKEYNOTIFY
    {
        USHORT      usFlags,        // in: as in WM_CHAR
                    usvk,           // in: as in WM_CHAR
                    usch;           // in: as in WM_CHAR
        UCHAR       ucScanCode;     // in: as in WM_CHAR
        USHORT      usKeyCode;      // in: if KC_VIRTUAL is set, this has usKeyCode;
                                    //     otherwise usCharCode
        CHAR        szDescription[100]; // out: key description
    } HOTKEYNOTIFY, *PHOTKEYNOTIFY;

    BOOL ctlMakeHotkeyEntryField(HWND hwndHotkeyEntryField);

    /* ******************************************************************
     *
     *   Color rectangle
     *
     ********************************************************************/

    BOOL ctlMakeColorRect(HWND hwndStatic);
    typedef BOOL CTLMAKECOLORRECT(HWND hwndStatic);
    typedef CTLMAKECOLORRECT *PCTLMAKECOLORRECT;

    /* ******************************************************************
     *
     *   Container control replacement
     *
     ********************************************************************/

    #define WC_CCTL_CNR             "ComctlCnr"

    typedef struct _CNRVIEWPORT
    {
        HWND        hwndCnr;
        SIZEL       szlWorkarea,
                    szlWin;
        POINTL      ptlScroll;
    } CNRVIEWPORT, *PCNRVIEWPORT;

    #define CN_VIEWPORTCHANGED              159

    MRESULT EXPENTRY fnwpCnr(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

    BOOL ctlRegisterXCnr(HAB hab);


#endif

#if __cplusplus
}
#endif

