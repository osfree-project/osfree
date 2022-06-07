
/*
 *@@sourcefile cctl_chart.c:
 *      implementation for the "chart" common control, which can
 *      represent an array of "double" values as either a pie chart
 *      or a bar chart. Selections of chart slices and owner
 *      notifications are also supported.
 *
 *      This has been extracted from comctl.c with V0.9.3 (2000-05-21) [umoeller].
 *
 *      The "chart" common control presently is a subclassed static
 *      text control. This way you can easily create a chart control
 *      as a static control in any Dialog Editor; after loading the
 *      dlg template, simply call ctlChartFromStatic with the hwnd
 *      of the static control to make it a chart.
 *
 *      Note: even though you can use _any_ type of static control
 *      with this function, you should use a static _text_ control,
 *      because not all types of static controls react to fonts and
 *      colors dragged upon them. The static _text_ control does.
 *
 *      The pie chart consumes all available space in the static control.
 *
 *      In XWorkplace, this is used for the pie chart on the new
 *      XFldDisk "Details" settings page to display the free space
 *      on a certain drive.
 *
 *      <B>Chart data:</B>
 *
 *      The pie chart control operates on an array of "double" values.
 *      Each value in that array corresponds to a color in a second
 *      array of (LONG) RGB values and, if description texts are
 *      enabled, to a third array of PSZ's with description texts.
 *      In each of the three arrays, the array item count must be
 *      the same, naturally.
 *
 *      The data on which the pie chart operates is initialized to
 *      be void, so that the pie chart will not paint anything
 *      initially. In order to have the pie chart display something,
 *      send a CHTM_SETCHARTDATA message (comctl.h) to the static
 *      control after it has been subclassed.
 *
 *      CHTM_SETCHARTDATA takes a CHARTDATA structure (comctl.h) in mp1,
 *      which must contain the three arrays with the chart data, colors,
 *      and descriptive texts to be displayed.
 *
 *      The chart data will automatically compute the sum of all values
 *      so that each data item will be displayed as a fraction of the
 *      total chart control size, proportionally to the item's value
 *      against the sum of all values. In other words,
 *
 +          (dValue / dTotal) == (sizeSlice / sizeTotal).
 *
 *      The display depends on whether the chart control operates in
 *      "pie chart" or "bar chart" mode.
 *
 *      --  In "pie chart" mode, data items are displayed as slices of
 *          a pie chart. The total sum of the "double" values will
 *          represent the angle in CHARTDATA.usSweepAngle.
 *
 *          For example, if two values of 50 and 100 are passed to the
 *          control and usSweepAngle is 270 (i.e. a three-quarter pie),
 *          the chart control will calculate the following:
 *
 *          1)  The sum of the data is 150.
 *
 *          2)  The first sub-arc will span an angle of 270 * (50/150)
 *              = 90 degrees.
 *
 *          3)  The second sub-arc will span an angle of 270 * (100/150)
 *              = 180 degrees.
 *
 *          You can also have descriptions displayed along the different
 *          chart items by specifying CHARTDATA.papszDescriptions and
 *          setting the CHS_DESCRIPTIONS flag (below).
 *
 *      --  In "bar chart" mode, the angle values make no sense and
 *          are therefore ignored. In "bar chart" mode, the chart will
 *          be drawn as a simple rectangle, with subrectangles representing
 *          the chart slices.
 *
 *      <B>Chart styles:</B>
 *
 *      Use CHTM_SETCHARTSTYLE with a PCHARTSTYLE (comctl.h) in mp1.
 *      This can be sent to the chart control several times.
 *
 *      Set either CHS_PIECHART or CHS_BARCHART to switch the control
 *      to "pie chart" or "bar chart" mode, respectively.
 *
 *      For pie charts, there are several "display sub-styles":
 *
 *      -- CHS_3D_BRIGHT: paint a "3D" socket below the actual chart.
 *
 *      -- CHS_3D_DARKEN: like CHS_3D_BRIGHT, but the socket will be made
 *                        darker compared to the surface.
 *
 *      General styles (for all modes):
 *
 *      -- CHS_DESCRIPTIONS: show descriptions on the chart
 *                           (CHARTDATA.papszDescriptions data).
 *
 *      -- CHS_SELECTIONS: allow pie chart slices to be selectable,
 *                         using the mouse and the keyboard (see below).
 *
 *      <B>Display:</B>
 *
 *      The chart control creates an internal bitmap for the display
 *      only once (ctlCreateChartBitmap). This bitmap is refreshed if
 *      neccessary, e.g. because chart data or styles have changed.
 *
 *      The chart control uses presentation parameters, as listed below.
 *      Presentation parameters are inherited from the parent window.
 *      If a presparam is not set, the corresponding system color is
 *      used. The following color pairs are recognized:
 *
 *      --  PP_BACKGROUNDCOLOR / SYSCLR_DIALOGBACKGROUND:
 *          background of the chart control (outside the chart).
 *
 *      --  PP_FOREGROUNDCOLOR / SYSCLR_WINDOWTEXT:
 *          text color, if description texts are enabled and valid.
 *
 *      --  PP_FONTNAMESIZE:
 *          text font, if description texts are enabled and valid.
 *          If this presparam is not set, the system font is used.
 *
 *      The control reacts to fonts and colors dropped upon it, if
 *      it has been subclassed from a static _text_ control (see above).
 *      It also recalculates the bitmap when it's resized.
 *
 *      <B>Example usage:</B>
 *
 +          // get static control:
 +          HWND    hwndChart = WinWindowFromID(hwndDialog, ID_...);
 +          CHARTSTYLE      cs;
 +          CHARTDATA       cd;
 +          // define data:
 +          double          adData[3] = { 100, 200, 300 };
 +          // define corresponding colors:
 +          LONG            alColors[3] = { 0x800000, 0x008000, 0x000080 };
 +          // define correspdonding descriptions:
 +          PSZ             apszDescriptions[3] = { "item 1", "item 3", "item 3" };
 +
 +          ctlChartFromStatic(hwndChart);     // create chart
 +
 +          cs.ulStyle = CHS_3D_DARKEN | CHS_DESCRIPTIONS;
 +          cs.ulThickness = 20;
 +          cs.dPieSize = .8;           // use 80% of control size
 +          cs.dDescriptions = .5       // draw descriptions at 50% from center
 +          WinSendMsg(hwndChart, CHTM_SETCHARTSTYLE, &cs, NULL);
 +
 +          cd.usStartAngle = 15;       // start at 15ø from right
 +          cd.usSweepAngle = 270;      // three-quarter pie (for the sum of the
 +                                      // above values: 100+200+300 = 600)
 +          cd.cValues = 3;             // array count
 +          cd.padValues = &adData[0];
 +          cd.palColors = &alColors[0];
 +          cd.papszDescriptions = &apszDescriptions[0];
 +          WinSendMsg(hwndChart, CHTM_SETCHARTDATA, &cd, NULL);
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\comctl.h"
 *@@added V0.9.3 (2000-05-21) [umoeller].
 */

/*
 *      Copyright (C) 1997-2002 Ulrich M”ller.
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

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WININPUT
#define INCL_WINPOINTERS
#define INCL_WINTRACKRECT
#define INCL_WINTIMER
#define INCL_WINSYS

#define INCL_WINRECTANGLES      /// xxx temporary

#define INCL_WINMENUS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINSTDCNR

#define INCL_GPIPRIMITIVES
#define INCL_GPILOGCOLORTABLE
#define INCL_GPILCIDS
#define INCL_GPIPATHS
#define INCL_GPIREGIONS
#define INCL_GPIBITMAPS             // added V0.9.1 (2000-01-04) [umoeller]: needed for EMX headers
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

#include "setup.h"                      // code generation and debugging options

#include "helpers\cnrh.h"
#include "helpers\except.h"             // exception handling
#include "helpers\gpih.h"
#include "helpers\linklist.h"
#include "helpers\winh.h"

#include "helpers\comctl.h"

#pragma hdrstop

/*
 *@@category: Helpers\PM helpers\Window classes\Chart control
 *      See cctl_chart.c.
 */

/* ******************************************************************
 *
 *   Private declarations
 *
 ********************************************************************/

/*
 *@@ CHARTCDATA:
 *      pie chart control data. Composed from the various
 *      chart initialization data.
 *      Stored in QWL_USER of the subclassed static control.
 *      Not available to the application.
 */

typedef struct _CHARTCDATA
{
    // data which is initialized upon creation:
    PFNWP           OldStaticProc;  // old static window procedure (from WinSubclassWindow)

    // data which is initialized upon CHTM_SETCHARTDATA/CHTM_SETCHARTSTYLE:
    HDC             hdcMem;         // memory device context for bitmap
    HPS             hpsMem;         // memory presentation space for bitmap
    CHARTDATA       cd;             // chart data: initialized to null values
    CHARTSTYLE      cs;             // chart style: initialized to null values

    HBITMAP         hbmChart;       // chart bitmap (for quick painting)
    HRGN*           paRegions;      // pointer to array of GPI regions for each data item

    // user interaction data:
    LONG            lSelected;      // zero-based index of selected chart item, or -1 if none
    LONG            lSourceEmphasis;    // zero-based index of item with source emphysis, or -1 if none
    BOOL            fHasFocus;
} CHARTCDATA, *PCHARTCDATA;

/* ******************************************************************
 *
 *   Chart Control
 *
 ********************************************************************/

/*
 *@@ DrawCenteredText:
 *
 *@@added V0.9.12 (2001-05-03) [umoeller]
 */

STATIC VOID DrawCenteredText(HPS hpsMem,
                             PPOINTL pptlMiddlePoint,
                             const char *pcsz)
{
    if (pcsz)
    {
        PSZ psz = strdup(pcsz),
            p2 = psz;
        if (psz)
        {
            ULONG cLineBreaks = 0;
            LONG lLineSpacing = 0;

            POINTL ptl;
            memcpy(&ptl, pptlMiddlePoint, sizeof(ptl));

            // count how many lines we have
            while (p2 = strchr(p2, '\n'))
            {
                cLineBreaks++;
                p2++;
            }

            if (cLineBreaks)
            {
                // if we have more than one line:
                lLineSpacing = gpihQueryLineSpacing(hpsMem);

                // center vertically
                ptl.y += (lLineSpacing * cLineBreaks) / 2;
            }

            p2 = psz;
            while (TRUE)
            {
                PSZ pNext = strchr(p2, '\n');
                if (pNext)
                {
                    *pNext = '\0';
                    pNext++;
                }

                GpiCharStringAt(hpsMem,
                                &ptl,
                                strlen(p2),
                                (PSZ)p2);

                if (pNext)
                {
                    p2 = pNext;
                    ptl.y -= lLineSpacing;
                }
                else
                    break;
            }

            free(psz);
        }
    }
}

/*
 *@@ PaintBarChart:
 *      paint implementation for the "bar chart".
 *
 *      This gets called from ctlCreateChartBitmap if the
 *      current style for the chart control is the "bar
 *      chart" style.
 *
 *@@added V0.9.12 (2001-05-03) [umoeller]
 *@@changed V0.9.12 (2001-05-03) [umoeller]: fixed another maaajor PM resource leak with regions
 */

STATIC VOID PaintBarChart(HPS hpsMem,
                          PRECTL prclWholeStatic,    // in: rectl to paint into
                          PCHARTDATA pChartData,     // in: chart data
                          PCHARTSTYLE pChartStyle,   // in: chart style
                          double dTotal,             // in: sum of all values in pChartData
                          LONG lTextColor,           // in: description text color (RGB)
                          HRGN* paRegions)           // out: GPI regions for each data item
{
    ULONG       ulYBottomNow = 0;
    ULONG       ul;

    PLONG       plColorThis;
    PSZ         *ppszDescriptionThis = NULL;
    HRGN        *phRegionThis;
    double      *pdValueThis;

    // allocate array for storing text positions later
    PPOINTL     paptlDescriptions = (PPOINTL)malloc(sizeof(POINTL)
                                                    * pChartData->cValues);

    RECTL       rclPaint;

    POINTL      ptlLowerLeft,
                ptlUpperRight;

    // thickness of line separators for CHS_DRAWLINES
    // V0.9.12 (2001-05-03) [umoeller]
    const SIZEL cszlDrawLines = {1, 1};

    // calculate rectangle;
    // this is the size of the static control
    // minus the "3D thickness", if enabled
    memcpy(&rclPaint, prclWholeStatic, sizeof(RECTL));
    if (pChartStyle->ulStyle & CHS_3D_BRIGHT)
            // this includes CHS_3D_DARKEN
    {
        rclPaint.yBottom = prclWholeStatic->yBottom
                                    + ulYBottomNow;
        rclPaint.yTop = prclWholeStatic->yTop
                                    - pChartStyle->ulThickness
                                    + ulYBottomNow;
    }

    // c)  Strangely, GpiSetPattern does work,
    //     while GpiSetColor doesn't (see below).
    GpiSetPattern(hpsMem, PATSYM_SOLID);

    // initialize per-item data pointers for
    // loop below
    pdValueThis = pChartData->padValues;
    plColorThis = pChartData->palColors;
    ppszDescriptionThis = pChartData->papszDescriptions;
    phRegionThis = paRegions;

    // initialize corners
    ptlLowerLeft.x = 0;
    ptlLowerLeft.y = 0;
    ptlUpperRight.x = 0;
    ptlUpperRight.y = rclPaint.yTop;

    // inner "slice loop":
    // this loop goes over the data pointers
    // and paints accordingly. At the end of
    // the loop, we'll advance all those pointers.
    for (ul = 0;
         ul < pChartData->cValues;
         ul++)
    {
        HRGN        hrgnThis;
        /* SHORT       sSweepAngle,
                    sStartAngle; */

        GpiSetCurrentPosition(hpsMem, &ptlLowerLeft);

        _Pmpf((__FUNCTION__ ": ptlLowerLeft.x = %d", ptlLowerLeft.x));

        // calc upper right from data values
        ptlUpperRight.x += (ULONG)(   (double)rclPaint.xRight
                                    * (*pdValueThis)
                                    / dTotal);

        // set the area (fill) color
        GpiSetColor(hpsMem,
                    *plColorThis);

        // now draw the slice;
        // we could use an area, but since we need
        // to remember the coordinates of the slice
        // for mouse click handling, we require a
        // region. But areas cannot be converted
        // to regions, so we use a path instead.
        GpiBeginPath(hpsMem,
                     1);    // path ID, must be 1

        GpiBox(hpsMem,
               DRO_OUTLINE,
               &ptlUpperRight,
               0, 0);       // no rounding

        GpiEndPath(hpsMem);

        // convert the path to a region;
        // we'll need the region for mouse hit testing later
        hrgnThis = GpiPathToRegion(hpsMem,
                                   1,
                                   FPATH_ALTERNATE);
            // after this, the path is deleted

        // now, this FINALLY paints the slice
        GpiPaintRegion(hpsMem, hrgnThis);

        // now paint descriptions
        if (pChartStyle->ulStyle & CHS_DESCRIPTIONS)
        {
            // description strings valid?
            if (ppszDescriptionThis)
            {
                POINTL  ptlMiddlePoint;

                // set presentation color
                GpiSetColor(hpsMem, lTextColor);

                // set text aligment to centered
                // both horizontally and vertically;
                // this affects subsequent GpiCharStringAt
                // calls in that the output text will
                // be centered around the specified
                // point
                GpiSetTextAlignment(hpsMem,
                                    TA_CENTER,      // horizontally
                                    TA_HALF);       // center vertically



                // center the text in the box
                ptlMiddlePoint.x =   ptlLowerLeft.x
                                   + (ptlUpperRight.x - ptlLowerLeft.x) / 2;
                ptlMiddlePoint.y =   ptlLowerLeft.y
                                   + (ptlUpperRight.y - ptlLowerLeft.y) / 2;

                // FINALLY, draw the description
                // at this point; since we have used
                // GpiSetTextAlignment above, the
                // text will be centered on exactly
                // that point
                DrawCenteredText(hpsMem,
                                 &ptlMiddlePoint,
                                 *ppszDescriptionThis);

            } // end if (ppszDescriptionThis)
        } // end if (pChartStyle->ulStyle & CHS_DESCRIPTIONS)

        // last run (are we painting the top now)?
        if (    // if separator lines are enabled,
                // draw a frame around the thing
                (pChartStyle->ulStyle & CHS_DRAWLINES)
           )
        {
            GpiSetColor(hpsMem, lTextColor);
            GpiFrameRegion(hpsMem,
                           hrgnThis,
                           (PSIZEL)&cszlDrawLines);      // always (1, 1)
        }

        if (    phRegionThis
           )
            // region output requested by caller:
            // store region, the caller will clean this up
            *phRegionThis = hrgnThis;
        else
            // no region output requested: destroy region
            GpiDestroyRegion(hpsMem, hrgnThis);

        // use the "right" xpos we had now as the "left"
        // xpos for the next loop
        ptlLowerLeft.x = ptlUpperRight.x;

        // advance the data pointers
        pdValueThis++;
        plColorThis++;
        if (ppszDescriptionThis)
            ppszDescriptionThis++;
        if (phRegionThis)
            phRegionThis++;
    } // end for (ul...)

    // cleanup
    free(paptlDescriptions);
}

/*
 *@@ PaintPieChart:
 *      paint implementation for the "pie chart".
 *
 *      This gets called from ctlCreateChartBitmap if the
 *      current style for the chart control is the "pie
 *      chart" style.
 *
 *@@added V0.9.12 (2001-05-03) [umoeller]
 *@@changed V0.9.12 (2001-05-03) [umoeller]: fixed another maaajor PM resource leak with regions
 *@@changed V0.9.12 (2001-05-03) [umoeller]: fixed rounding errors
 */

STATIC VOID PaintPieChart(HPS hpsMem,
                          PRECTL prclWholeStatic,    // in: rectl to paint into
                          PCHARTDATA pChartData,     // in: chart data
                          PCHARTSTYLE pChartStyle,   // in: chart style
                          double dTotal,             // in: sum of all values in pChartData
                          LONG lTextColor,           // in: description text color (RGB)
                          HRGN* paRegions)           // out: GPI regions for each data item
{
    ULONG       ulYBottomNow = 0;
    ULONG       ul;

    // allocate array for storing text positions later
    PPOINTL     paptlDescriptions = (PPOINTL)malloc(sizeof(POINTL)
                                                    * pChartData->cValues);
    POINTL      ptlCenter;

    FIXED       fxPieSize = (LONG)(pChartStyle->dPieSize * 65536),
                fxDescriptions = (LONG)(pChartStyle->dDescriptions * 65536);

    // We'll paint into the bitmap in two loops:
    // +--  The outer "3D" loop is executed
    // |    pChartStyle->ulThickness-fold, if
    // |    CHS_3Dxxx has been enabled; otherwise
    // |    just once.
    // |
    // |    +-- The inner "slice" loop goes thru the
    // |        data fields in pChartData and draws
    // |        the pies accordingly.
    // |
    // +--  We then increase the base Y point (ulYBottomNow)
    //      by one and draw again, thereby getting the 3D
    //      effect.

    // 1) outer 3D loop
    do // while (   (pChartStyle->ulStyle & CHS_3D_BRIGHT)...
    {
        RECTL       rclArc;
        PLONG       plColorThis;
        PSZ         *ppszDescriptionThis = NULL;
        PPOINTL     pptlDescriptionThis;
        HRGN        *phRegionThis;
        double      *pdValueThis;

        ARCPARAMS   ap;
        AREABUNDLE  ab;

        double      dStartAngle = pChartData->usStartAngle,
                    dSweepAngle = 0;

        // thickness of line separators for CHS_DRAWLINES
        // V0.9.12 (2001-05-03) [umoeller]
        const SIZEL cszlDrawLines = {1, 1};

        // this is only TRUE for the last loop
        BOOL        fNowDrawingSurface =
                      (
                           ((pChartStyle->ulStyle & CHS_3D_BRIGHT) == 0)
                        ||
                           (ulYBottomNow == pChartStyle->ulThickness - 1)
                      );

        // // _Pmpf(("Looping, ulYBottomNow: %d", ulYBottomNow));

        // calculate pie rectangle for this loop;
        // this is the size of the static control
        // minus the "3D thickness", if enabled
        memcpy(&rclArc, prclWholeStatic, sizeof(RECTL));
        if (pChartStyle->ulStyle & CHS_3D_BRIGHT)
                // this includes CHS_3D_DARKEN
        {
            rclArc.yBottom = prclWholeStatic->yBottom
                                        + ulYBottomNow;
            rclArc.yTop = prclWholeStatic->yTop
                                        - pChartStyle->ulThickness
                                        + ulYBottomNow;
        }

        // calculate center point
        ptlCenter.x = rclArc.xRight / 2;
        ptlCenter.y = ((rclArc.yTop - rclArc.yBottom) / 2) + ulYBottomNow;

        // Now, the "arc" APIs really suck. The following
        // has cost me hours of testing to find out:

        // a) The arc functions expect some kind of
        //    "default arc" to be defined, which they
        //    refer to. We define the arc as elliptical;
        //    this will be used as the "current arc"
        //    for subsequent arc calls.
        //    (P, S) and (R, Q) define the end points
        //    of the major axes of the ellipse.
        //    The center of the arc will later be
        //    specified with GpiPartialArc (while GpiFullArc
        //    uses the current pen position...
        //    Who created these APIs?!?)
        ap.lP = ptlCenter.x;        // X-axis X
        ap.lS = 0;                  // X-axis Y
        ap.lR = 0;                  // Y-axis X
        ap.lQ = ((rclArc.yTop - rclArc.yBottom) / 2);
                                    // Y-axis Y
        GpiSetArcParams(hpsMem, &ap);

        // b)  The line primitives determine lines
        //     to be drawn around the pie slices.
        //     We don't want any.
        GpiSetLineType(hpsMem, LINETYPE_INVISIBLE);

        // c)  Strangely, GpiSetPattern does work,
        //     while GpiSetColor doesn't (see below).
        GpiSetPattern(hpsMem, PATSYM_SOLID);

        // initialize per-item data pointers for
        // loop below
        pdValueThis = pChartData->padValues;
        plColorThis = pChartData->palColors;
        ppszDescriptionThis = pChartData->papszDescriptions;
        pptlDescriptionThis = paptlDescriptions;
        phRegionThis = paRegions;

        // 2) inner "pie slice loop":
        // this loop goes over the data pointers
        // and paints accordingly. At the end of
        // the loop, we'll advance all those pointers.
        for (ul = 0;
             ul < pChartData->cValues;
             ul++)
        {
            HRGN        hrgnThis;
            double      dSweepAngleRounded,
                        dStartAngleRounded;

            // calculate the angle to sweep to:
            // a simple rule of three
            dSweepAngle = *pdValueThis      // current data pointer
                          * (double)pChartData->usSweepAngle
                                            // maximum angle
                          / dTotal;         // total data sum

            // d)  And now comes the real fun part.
            //     GpiPartialArc is too dumb to draw
            //     anything on its own, it must _always_
            //     appear within an area or path definition.
            //     Unfortunately, this isn't really said
            //     clearly anywhere.
            //     Even worse, in order to set the color
            //     with which the slice is to be drawn,
            //     one has to define an AREABUNDLE because
            //     the regular GpiSetColor functions don't
            //     seem to work here. Or maybe it's my fault,
            //     but with this awful documentation, who knows.

            //     We set the current color in the AREABUNDLE
            //     from the color which was defined in the
            //     pie chart data (this pointer was set above
            //     and will be advanced for the next slice).
            ab.lColor = *plColorThis;

            // "3D mode" enabled with darkened socket?
            if (    (pChartStyle->ulStyle & CHS_3D_DARKEN)
                    // not last loop?
                 && (!fNowDrawingSurface)
               )
                // darken the current fill color
                // by halving each color component
                gpihManipulateRGB(&ab.lColor,
                                  .5);       // factor

            // set the area (fill) color
            GpiSetAttrs(hpsMem,
                        PRIM_AREA,
                        ABB_COLOR,
                        0,
                        (PBUNDLE)&ab);

            GpiSetCurrentPosition(hpsMem, &ptlCenter);

            // round the angle values properly
            dStartAngleRounded = (dStartAngle + .5);
            dSweepAngleRounded = (dSweepAngle + .5);

            // now draw the pie slice;
            // we could use an area, but since we need
            // to remember the coordinates of the slice
            // for mouse click handling, we require a
            // region. But areas cannot be converted
            // to regions, so we use a path instead.
            GpiBeginPath(hpsMem,
                         1);    // path ID, must be 1

            // note that the arc functions use the FIXED type
            // for representing fractions... GPI was written back
            // at the days when coprocessors were a luxury, so
            // a floating point value is represented by a LONG
            // with the hiword containing the rounded integer
            // and the loword the remaining fraction. So we
            // can simply multiply the "double" by 65536, and
            // we get the FIXED value V0.9.12 (2001-05-03) [umoeller]
            GpiPartialArc(hpsMem,
                          &ptlCenter,
                          fxPieSize,    // calculated from CHARTSTYLE
                          (FIXED)(dStartAngleRounded * (double)65536),
                          (FIXED)(dSweepAngleRounded * (double)65536));
                // this moves the current position to the outer
                // point on the ellipse which corresponds to
                // sSweepAngle
            GpiEndPath(hpsMem);

            // convert the path to a region;
            // we'll need the region for mouse hit testing later
            hrgnThis = GpiPathToRegion(hpsMem, 1,
                                       FPATH_ALTERNATE);
                // after this, the path is deleted

            // last run (are we painting the top now)?
            if (    (fNowDrawingSurface)
                    // descriptions enabled?
                 && (ppszDescriptionThis)
                 && (*ppszDescriptionThis)
               )
            {
                // yes: calculate position to paint
                // text at later (we can't do this now,
                // because it might be overpainted by
                // the next arc again)

                GpiSetCurrentPosition(hpsMem, &ptlCenter);
                // move the current position to
                // the center outer point on the ellipse
                // (in between sStartAngle and sSweepAngle);
                // since we're outside an area, this will not
                // paint anything
                GpiPartialArc(hpsMem,
                              &ptlCenter,
                              fxDescriptions, // calculated from CHARTSTYLE
                              (FIXED)(dStartAngleRounded * (double)65536),
                              // only half the sweep now:
                              (FIXED)((dSweepAngleRounded / 2) * (double)65536));

                // store this outer point in the array
                // of description coordinates for later
                GpiQueryCurrentPosition(hpsMem, pptlDescriptionThis);

                _Pmpf(("pptlDescriptionThis = %d, %d",
                            pptlDescriptionThis->x,
                            pptlDescriptionThis->y));
            }

            // now, this FINALLY paints the arc
            GpiPaintRegion(hpsMem, hrgnThis);

            // last run (are we painting the top now)?
            if (    (fNowDrawingSurface)
                    // if separator lines are enabled,
                    // draw a frame around the thing
                 && (pChartStyle->ulStyle & CHS_DRAWLINES)
               )
            {
                GpiSetColor(hpsMem, lTextColor);
                GpiFrameRegion(hpsMem,
                               hrgnThis,
                               (PSIZEL)&cszlDrawLines);      // always (1, 1)
            }

            if (    phRegionThis
                 && fNowDrawingSurface      // this was missing
                                            // V0.9.12 (2001-05-03) [umoeller]
                                            // this created tons of regions which
                                            // were never deleted
               )
                // region output requested by caller:
                // store region, the caller will clean this up
                *phRegionThis = hrgnThis;
            else
                // no region output requested: destroy region
                GpiDestroyRegion(hpsMem, hrgnThis);

            // increase the start angle by the sweep angle for next loop
            dStartAngle += dSweepAngle;

            // advance the data pointers
            pdValueThis++;
            plColorThis++;
            if (ppszDescriptionThis)
                ppszDescriptionThis++;
            pptlDescriptionThis++;
            if (phRegionThis)
                phRegionThis++;
        } // end for (ul...)

        // go for next "3D thickness" iteration
        ulYBottomNow++;
    } while (   (pChartStyle->ulStyle & CHS_3D_BRIGHT)
             && (ulYBottomNow < pChartStyle->ulThickness)
            );

    // now paint descriptions
    if (pChartStyle->ulStyle & CHS_DESCRIPTIONS)
    {
        // we use two pointers during the iteration,
        // which point to the item corresponding
        // to the current data item:
        // 1)  pointer to center point on outer border
        //     of partial arc
        //     (calculated above)
        PPOINTL     pptlDescriptionThis = paptlDescriptions;
        // 2)  pointer to current description string
        PSZ*        ppszDescriptionThis = pChartData->papszDescriptions;

        // description strings valid?
        if (ppszDescriptionThis)
        {
            // set presentation color
            GpiSetColor(hpsMem, lTextColor);

            // set text aligment to centered
            // both horizontally and vertically;
            // this affects subsequent GpiCharStringAt
            // calls in that the output text will
            // be centered around the specified
            // point
            GpiSetTextAlignment(hpsMem,
                                TA_CENTER,      // horizontally
                                TA_HALF);       // center vertically

            // loop thru data items
            for (ul = 0;
                 ul < pChartData->cValues;
                 ul++)
            {
                POINTL  ptlMiddlePoint;

                // when drawing the arcs above, we have,
                // for each pie slice, stored the middle
                // point on the outer edge of the ellipse
                // in the paptlDescriptions POINTL array:

                //                ++++
                //                +    +
                //                +      +
                //    ptlCenter\  +       +
                //              \ +        + <-- current partial arc
                //               \+        +
                //     +++++++++++X        +
                //     +                   +
                //      +                  +
                //       +               XX  <-- point calculated above
                //        +             +
                //           +        +
                //             ++++++

                // now calculate a middle point between
                // that outer point on the ellipse and
                // the center of the ellipse, which will
                // be the center point for the text

                //                ++++
                //                +    +
                //                +      +
                //    ptlCenter\  +       +
                //              \ +        + <-- current partial arc
                //               \+        +
                //     ++++++++++++        +
                //     +             XX    + <-- new middle point
                //      +                  +
                //       +               XX  <-- point calculated above
                //        +             +
                //           +        +
                //             ++++++

                ptlMiddlePoint.x =
                        ptlCenter.x
                        + ((pptlDescriptionThis->x - ptlCenter.x) * 2 / 3);
                ptlMiddlePoint.y =
                        ptlCenter.y
                        - (ptlCenter.y - pptlDescriptionThis->y) * 2 / 3;

                // FINALLY, draw the description
                // at this point; since we have used
                // GpiSetTextAlignment above, the
                // text will be centered on exactly
                // that point
                DrawCenteredText(hpsMem,
                                 &ptlMiddlePoint,
                                 *ppszDescriptionThis);

                pptlDescriptionThis++;
                ppszDescriptionThis++;
            } // end for (ul = 0; ul < pChartData->cValues; ul++)
        } // end if (ppszDescriptionThis)
    } // end if (pChartStyle->ulStyle & CHS_DESCRIPTIONS)

    // cleanup
    free(paptlDescriptions);
}

/*
 *@@ ctlCreateChartBitmap:
 *      this creates a new bitmap and paints the
 *      chart into it. This bitmap will be used
 *      in WM_PAINT of ctl_fnwpChart to quickly paint
 *      the chart image.
 *
 *      The bitmap will be created with the specified
 *      size. The chart will consume all available
 *      space in the bitmap. The returned bitmap is
 *      not selected into any presentation space.
 *
 *      Note: Description text will be drawn with the
 *      current font specified for the memory PS, and
 *      with the current character cell box.
 *
 *      This function gets called automatically from
 *      WM_PAINT if we determine that the bitmap for
 *      painting has not been created yet or has been
 *      invalidated (because data or styles changed).
 *
 *      However, you can also use this function
 *      independently to have a chart bitmap created,
 *      wherever you might need it.
 *      You will then have to fill in the CHARTDATA
 *      and CHARTSTYLE structures before calling
 *      this function.
 *
 *      If (paRegions != NULL), this function will
 *      create a GPI region for each data item. Each GPI
 *      region will then contain the outline of the
 *      corresponding pie chart slice. This allows
 *      you to easily relate coordinates to pie slieces,
 *      for example for mouse clicks.
 *
 *      In that case, paRegions must point to an array
 *      of HRGN items, each of which will contain a
 *      region handle later. It is the responsibility
 *      of the caller to free the regions later, using
 *      GpiDestroyRegion on each region handle (with
 *      the hpsMem specified with this function).
 *
 *      If you're not interested in the regions, set
 *      paRegions to NULL.
 *
 *      This returns NULLHANDLE if an error occurred.
 *      This can mean the following:
 *
 *      --  The data is invalid, because the total is 0.
 *
 *      --  The bitmap could not be created (memory?).
 *
 *@@changed V0.9.12 (2001-05-03) [umoeller]: extracted PaintPieChart
 *@@changed V0.9.12 (2001-05-03) [umoeller]: added bar chart style
 */

HBITMAP ctlCreateChartBitmap(HPS hpsMem,                // in: memory PS to use for bitmap creation
                             LONG lcx,                  // in: width of bitmap
                             LONG lcy,                  // in: height of bitmap
                             PCHARTDATA pChartData,     // in: chart data
                             PCHARTSTYLE pChartStyle,   // in: chart style
                             LONG lBackgroundColor,     // in: color around the chart (RGB)
                             LONG lTextColor,           // in: description text color (RGB)
                             HRGN* paRegions)           // out: GPI regions for each data item
{
    HBITMAP hbmReturn = NULLHANDLE;

    // sum up the values to get the total
    ULONG   ul = 0;
    double  dTotal = 0;
    double*  pdThis = pChartData->padValues;
    for (ul = 0; ul < pChartData->cValues; ul++)
    {
        _Pmpf(("    dThis is %d", (ULONG)(*pdThis))); // printf

        dTotal += *pdThis;
        pdThis++;
    }

    _Pmpf((__FUNCTION__ ": dTotal is %d", (ULONG)dTotal)); // printf

    // avoid division by zero
    if (dTotal > 0)
    {
        RECTL       rclWholeStatic;
        // get window rectangle (bottom left is 0, 0)
        rclWholeStatic.xLeft = 0;
        rclWholeStatic.yBottom = 0;
        rclWholeStatic.xRight = lcx;
        rclWholeStatic.yTop = lcy;

        // create bitmap of that size
        if ((hbmReturn = gpihCreateBitmap(hpsMem,
                                          rclWholeStatic.xRight,
                                          rclWholeStatic.yTop)))
        {
            // successfully created:

            // associate bitmap with memory PS
            GpiSetBitmap(hpsMem, hbmReturn);

            // switch the HPS to RGB mode
            gpihSwitchToRGB(hpsMem);

            // fill bitmap with static's background color
            GpiSetColor(hpsMem, lBackgroundColor);
            gpihBox(hpsMem,
                    DRO_FILL,
                    &rclWholeStatic);

            if (pChartStyle->ulStyle & CHS_BARCHART)
                PaintBarChart(hpsMem,
                              &rclWholeStatic,
                              pChartData,
                              pChartStyle,
                              dTotal,
                              lTextColor,
                              paRegions);
            else
                PaintPieChart(hpsMem,
                              &rclWholeStatic,
                              pChartData,
                              pChartStyle,
                              dTotal,
                              lTextColor,
                              paRegions);

            // deselect (free) bitmap
            GpiSetBitmap(hpsMem, NULLHANDLE);
        } // end if (pChtCData->hbmChart ...)
    } // end if (dTotal > 0)

    return hbmReturn;
}

/*
 *@@ CleanupBitmap:
 *      this frees the resources associated with
 *      the chart bitmap.
 *
 *@@changed V0.9.2 (2000-02-29) [umoeller]: fixed maaajor memory leak
 *@@changed V0.9.12 (2001-05-03) [umoeller]: fixed major PM resource leaks
 */

STATIC VOID CleanupBitmap(PCHARTCDATA pChtCData)
{
    if (pChtCData)
    {
        // destroy regions, but not the array itself
        // (this is done in CleanupData)
        if (pChtCData->hpsMem && pChtCData->paRegions)
        {
            ULONG   ul;
            for (ul = 0;
                 ul < pChtCData->cd.cValues;
                 ul++)
            {
                if (pChtCData->paRegions[ul])
                {
                    GpiDestroyRegion(pChtCData->hpsMem, pChtCData->paRegions[ul]);
                    pChtCData->paRegions[ul] = NULLHANDLE;
                }
            }
        }

        // bitmap already created?
        if (pChtCData->hbmChart)
        {
            // free current bitmap
            GpiSetBitmap(pChtCData->hpsMem, NULLHANDLE);
            // delete bitmap; fails if not freed!
            if (!GpiDeleteBitmap(pChtCData->hbmChart))
                _Pmpf((__FUNCTION__ ": GpiDeleteBitmap failed."));
            pChtCData->hbmChart = NULLHANDLE;
        }

        if (pChtCData->hpsMem)
        {
            GpiDestroyPS(pChtCData->hpsMem);
            pChtCData->hpsMem = NULLHANDLE;
        }
        if (pChtCData->hdcMem)
        {
            DevCloseDC(pChtCData->hdcMem);
            pChtCData->hdcMem = NULLHANDLE;
        }
    }
}

/*
 *@@ CleanupData:
 *      this frees all allocated resources of the chart
 *      control, except the bitmap (use CleanupBitmap
 *      for that) and the CHARTCDATA itself.
 *
 *      Note: CleanupBitmap must be called _before_ this
 *      function.
 */

STATIC VOID CleanupData(PCHARTCDATA pChtCData)
{
    if (pChtCData)
        if (pChtCData->cd.cValues)
        {
            // _Pmpf(("Cleaning up data"));
            // values array
            if (pChtCData->cd.padValues)
                free(pChtCData->cd.padValues);

            // colors array
            if (pChtCData->cd.palColors)
                free(pChtCData->cd.palColors);

            // strings array
            if (pChtCData->cd.papszDescriptions)
            {
                ULONG   ul;
                PSZ     *ppszDescriptionThis = pChtCData->cd.papszDescriptions;
                for (ul = 0;
                     ul < pChtCData->cd.cValues;
                     ul++)
                {
                    if (*ppszDescriptionThis)
                        free(*ppszDescriptionThis);

                    ppszDescriptionThis++;
                }

                free(pChtCData->cd.papszDescriptions);
            }

            pChtCData->cd.cValues = 0;

            // GPI regions array
            if (pChtCData->paRegions)
            {
                free(pChtCData->paRegions);
                pChtCData->paRegions = NULL;
            }
        }
}

/*
 *@@ SetChartData:
 *      implementation for CHTM_SETCHARTDATA
 *      in ctl_fnwpChart.
 *
 *@@added V0.9.2 (2000-02-29) [umoeller]
 *@@changed V0.9.12 (2001-05-03) [umoeller]: fixed trap if ptr to descriptions was null
 */

STATIC VOID SetChartData(HWND hwndChart,
                         PCHARTCDATA pChtCData,
                         PCHARTDATA pcdNew)
{
    ULONG        ul = 0;
    PSZ          *ppszDescriptionSource,
                 *ppszDescriptionTarget;

    // free previous values, if set
    if (pChtCData->hbmChart)
        CleanupBitmap(pChtCData);

    CleanupData(pChtCData);

    pChtCData->cd.usStartAngle = pcdNew->usStartAngle;
    pChtCData->cd.usSweepAngle = pcdNew->usSweepAngle;
    pChtCData->cd.cValues = pcdNew->cValues;

    // copy values
    pChtCData->cd.padValues = (double*)malloc(sizeof(double) * pcdNew->cValues);
    memcpy(pChtCData->cd.padValues,
           pcdNew->padValues,
           sizeof(double) * pcdNew->cValues);

    // copy colors
    pChtCData->cd.palColors = (LONG*)malloc(sizeof(LONG) * pcdNew->cValues);
    memcpy(pChtCData->cd.palColors,
           pcdNew->palColors,
           sizeof(LONG) * pcdNew->cValues);

    // copy strings
    if (!pcdNew->papszDescriptions)
        pChtCData->cd.papszDescriptions = NULL;
    else
    {
        pChtCData->cd.papszDescriptions = (PSZ*)malloc(sizeof(PSZ) * pcdNew->cValues);
        ppszDescriptionSource = pcdNew->papszDescriptions;
        ppszDescriptionTarget = pChtCData->cd.papszDescriptions;
        for (ul = 0;
             ul < pcdNew->cValues;
             ul++)
        {
            if (*ppszDescriptionSource)
                *ppszDescriptionTarget = strdup(*ppszDescriptionSource);
            else
                *ppszDescriptionTarget = NULL;
            ppszDescriptionSource++;
            ppszDescriptionTarget++;
        }
    }

    // create an array of GPI region handles
    pChtCData->paRegions = (HRGN*)malloc(sizeof(HRGN) * pcdNew->cValues);
    // initialize all regions to null
    memset(pChtCData->paRegions, 0, sizeof(HRGN) * pcdNew->cValues);

    pChtCData->lSelected = -1;     // none selected
    pChtCData->lSourceEmphasis = -1;     // none selected

    WinInvalidateRect(hwndChart, NULL, FALSE);
}

/*
 *@@ PaintChart:
 *      implementation for WM_PAINT
 *      in ctl_fnwpChart.
 *
 *@@added V0.9.2 (2000-02-29) [umoeller]
 *@@changed V0.9.12 (2001-05-03) [umoeller]: fixed major PM resource leaks
 */

STATIC VOID PaintChart(HWND hwndChart,
                       PCHARTCDATA pChtCData,
                       HPS hps,
                       PRECTL prclPaint)
{
    RECTL   rclStatic;
    WinQueryWindowRect(hwndChart, &rclStatic);

    // do we have any values yet?
    if (pChtCData->cd.cValues == 0)
    {
        CHAR szDebug[200];
        sprintf(szDebug, "Error, no values set");
        WinFillRect(hps,
                    &rclStatic,  // exclusive
                    CLR_WHITE);
        WinDrawText(hps,
                    strlen(szDebug),
                    szDebug,
                    &rclStatic,
                    CLR_BLACK,
                    CLR_WHITE,
                    DT_LEFT | DT_TOP);
    }
    else
    {
        // valid values, apparently:
        LONG lForegroundColor = winhQueryPresColor(hwndChart,
                                                   PP_FOREGROUNDCOLOR,
                                                   TRUE,       // inherit presparams
                                                   SYSCLR_WINDOWTEXT);

        gpihSwitchToRGB(hps);

        // check if the bitmap needs to be (re)created
        if (pChtCData->hbmChart == NULLHANDLE)
        {
            // no: do it now
            HPOINTER    hptrOld = winhSetWaitPointer();
            LONG        lLCIDSet = 0;

            if (pChtCData->hpsMem == NULLHANDLE)
            {
                // first call:
                FONTMETRICS FontMetrics;
                LONG        lPointSize;

                // create a memory PS for the bitmap
                SIZEL szlPage;
				szlPage.cx=rclStatic.xRight;
                szlPage.cy=rclStatic.yTop;
                gpihCreateMemPS(WinQueryAnchorBlock(hwndChart),
                                &szlPage,
                                &pChtCData->hdcMem,
                                &pChtCData->hpsMem);

                // get presentation font
                lLCIDSet = gpihFindPresFont(hwndChart,
                                            TRUE,      // inherit PP
                                            pChtCData->hpsMem,
                                            "8.Helv",
                                            &FontMetrics,
                                            &lPointSize);
                // set presentation font
                if (lLCIDSet)
                {
                    GpiSetCharSet(pChtCData->hpsMem, lLCIDSet);
                    if (FontMetrics.fsDefn & FM_DEFN_OUTLINE)
                        gpihSetPointSize(pChtCData->hpsMem, lPointSize);
                }
            }

            pChtCData->hbmChart = ctlCreateChartBitmap(
                      pChtCData->hpsMem,        // mem PS
                      rclStatic.xRight,         // cx
                      rclStatic.yTop,           // cy
                      &pChtCData->cd,           // data
                      &pChtCData->cs,           // style
                      // background color:
                      winhQueryPresColor(hwndChart,
                                         PP_BACKGROUNDCOLOR,
                                         TRUE,       // inherit presparams
                                         SYSCLR_DIALOGBACKGROUND),
                      // description text color:
                      lForegroundColor,
                      pChtCData->paRegions);
                              // out: regions array

            // unset and delete font
            if (lLCIDSet)
            {
                GpiSetCharSet(pChtCData->hpsMem, LCID_DEFAULT);
                GpiDeleteSetId(pChtCData->hpsMem, lLCIDSet);
            }

            WinSetPointer(HWND_DESKTOP, hptrOld);
        }

        if (pChtCData->hbmChart)
        {
            // draw the chart bitmap
            POINTL  ptlDest = { 0, 0 };
            WinDrawBitmap(hps,
                          pChtCData->hbmChart,
                          NULL,     // draw whole bitmap
                          &ptlDest,
                          0, 0,     // colors (don't care)
                          DBM_NORMAL);

            // now draw emphasis, if we have regions
            if (pChtCData->paRegions)
            {
                AREABUNDLE ab;
                SIZEL   sl = {2, 2};

                // 1) source emphasis (even if we don't have focus)
                if (pChtCData->lSourceEmphasis != -1)
                {
                    if (pChtCData->paRegions[pChtCData->lSourceEmphasis])
                    {
                        // GpiFrameRegion uses the current pattern
                        // attributes, so we must set the correct
                        // color in there (GpiSetColor doesn't work)
                        // V0.9.12 (2001-05-03) [umoeller]

                        ab.lColor = lForegroundColor;
                        GpiSetAttrs(hps,
                                    PRIM_AREA,
                                    ABB_COLOR,
                                    0,
                                    (PBUNDLE)&ab);
                        GpiSetPattern(hps,
                                      PATSYM_DIAG1);
                        GpiPaintRegion(hps,
                                       pChtCData->paRegions[pChtCData->lSourceEmphasis]);
                    }
                }

                // 2) selection

                // do we have the focus?
                if (    (pChtCData->fHasFocus)
                        // something selected?
                     && (pChtCData->lSelected != -1)
                   )
                {
                    if (pChtCData->paRegions[pChtCData->lSelected])
                    {
                        // GpiFrameRegion uses the current pattern
                        // attributes, so we must set the correct
                        // color in there (GpiSetColor doesn't work)
                        // V0.9.12 (2001-05-03) [umoeller]

                        ab.lColor = RGBCOL_RED; // lForegroundColor;
                        GpiSetAttrs(hps,
                                    PRIM_AREA,
                                    ABB_COLOR,
                                    0,
                                    (PBUNDLE)&ab);
                        GpiFrameRegion(hps,
                                       pChtCData->paRegions[pChtCData->lSelected],
                                       &sl);
                    }
                }
            }
        }
    }
}

/*
 *@@ FindItemFromPoint:
 *      returns the index of the slice under the
 *      given window coordinates, or -1 if there's
 *      none.
 *
 *@@added V0.9.12 (2001-05-03) [umoeller]
 */

STATIC LONG FindItemFromPoint(PCHARTCDATA pChtCData,
                              LONG lx,
                              LONG ly)
{
    POINTL ptlMouse;
	ptlMouse.x = lx;
	ptlMouse.y = ly;

    // data set?
    if (    (pChtCData->cd.cValues)
         && (pChtCData->paRegions)
       )
    {
        ULONG   ul;
        for (ul = 0;
             ul < pChtCData->cd.cValues;
             ul++)
        {
            HRGN hRgnThis = pChtCData->paRegions[ul];
            if (hRgnThis)
            {
                if (GpiPtInRegion(pChtCData->hpsMem,
                                  hRgnThis,
                                  &ptlMouse)
                     == PRGN_INSIDE)
                {
                    return ul;
                }
            }
        }
    }

    return -1;
}

/*
 *@@ SendWMControl:
 *
 *@@added V0.9.12 (2001-05-03) [umoeller]
 */

STATIC VOID SendWMControl(HWND hwndChart,
                          MPARAM mp1Mouse,
                          USHORT usNotify,
                          ULONG ulEmphasis,            // 0 or 1
                          LONG lIndex)
{
    EMPHASISNOTIFY en;
    en.hwndSource = hwndChart;
    en.lIndex = lIndex;        // can be -1
    en.ulEmphasis = ulEmphasis;
    en.ptl.x = SHORT1FROMMP(mp1Mouse);
    en.ptl.y = SHORT2FROMMP(mp1Mouse);

    ctlSendWmControl(hwndChart,
                     usNotify,
                     &en);
}

/*
 *@@ SetEmphasis:
 *
 *@@added V0.9.12 (2001-05-03) [umoeller]
 */

STATIC BOOL SetEmphasis(HWND hwndChart,
                        PCHARTCDATA pChtCData,
                        ULONG ulEmphasis,      // in: 0 == selection, 1 == source emphasis
                        MPARAM mp1,            // in: mp1 with mouse values or -1 if none
                        LONG lIndex,
                        BOOL fIsContextMenu)
{
    BOOL brc = FALSE;

    PLONG plOld;

    if (ulEmphasis == 0)
        plOld = &pChtCData->lSelected;
    else if (ulEmphasis == 1)
        plOld = &pChtCData->lSourceEmphasis;

    if (plOld)
    {
        if (    (*plOld != lIndex)
             || (fIsContextMenu)
           )
        {
            // selection changed:
            *plOld = lIndex;

            brc = TRUE;

            // repaint
            WinInvalidateRect(hwndChart, NULL, FALSE);

            // send notification to owner
            // V0.9.12 (2001-05-03) [umoeller]
            SendWMControl(hwndChart,
                          mp1,
                          CHTN_EMPHASISCHANGED,
                          ulEmphasis,
                          lIndex);
        }
    }

    return brc;
}

/*
 *@@ ctl_fnwpChart:
 *      window procedure for the "chart" control.
 *
 *      This is not a stand-alone window procedure, but must only
 *      be used with static controls subclassed by ctlChartFromStatic.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.2 (2000-02-29) [umoeller]: added resize support
 *@@changed V0.9.2 (2000-02-29) [umoeller]: fixed baaad PM resource leaks, the bitmap was never freed
 *@@changed V0.9.12 (2001-05-03) [umoeller]: added WM_CONTEXTMENU support
 *@@changed V0.9.20 (2002-07-17) [pr]: added CHTN_SETFOCUS and CHTN_KILLFOCUS support
 */

MRESULT EXPENTRY ctl_fnwpChart(HWND hwndChart, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT     mrc = 0;
    PCHARTCDATA   pChtCData = (PCHARTCDATA)WinQueryWindowULong(hwndChart, QWL_USER);

    if (pChtCData)
    {
        PFNWP OldStaticProc = pChtCData->OldStaticProc;

        switch (msg)
        {
            /*
             *@@ CHTM_SETCHARTDATA:
             *      user msg to set the pie chart data,
             *      which is copied to the control's
             *      memory.
             *      This can be sent several times to
             *      have the chart refreshed with new
             *      values.
             *
             *      Parameters:
             *      --  PCHARTDATA mp1: new values.
             *              These will be copied to the chart's internal data.
             *      --  mp2: unused.
             */

            case CHTM_SETCHARTDATA:
                // _Pmpf(("CHTM_SETCHARTDATA, mp1: 0x%lX", mp1));
                if (mp1)
                    SetChartData(hwndChart,
                                 pChtCData,
                                 (PCHARTDATA)mp1);
            break;

            /*
             *@@ CHTM_SETCHARTSTYLE:
             *      user msg to set the chart style,
             *      which is copied to the control's
             *      memory.
             *      This can be sent several times to
             *      have the chart refreshed with new
             *      styles.
             *
             *      Parameters:
             *      --  PCHARTSTYLE mp1: new style data.
             *              This will be copied to the chart's internal data.
             *      --  mp2: unused.
             */

            case CHTM_SETCHARTSTYLE:
                // _Pmpf(("CHTM_SETCHARTSTYLE, mp1: 0x%lX", mp1));
                memcpy(&(pChtCData->cs), mp1, sizeof(CHARTSTYLE));
                if (pChtCData->hbmChart)
                {
                    CleanupBitmap(pChtCData);
                    WinInvalidateRect(hwndChart, NULL, FALSE);
                }
            break;

            /*
             *@@ CHTM_ITEMFROMPOINT:
             *      this can be _sent_ to the chart control
             *      to query the chart item which surrounds
             *      the given point. Specify the coordinates
             *      in two SHORT's in mp1 (as with WM_BUTTON1*
             *      messages), in window coordinates.
             *
             *      Parameters:
             *      -- SHORT SHORT1FROMMP(mp1): x
             *      -- SHORT SHORT1FROMMP(mp1): y
             *      -- mp2: unused.
             *
             *      Returns:
             *      -- LONG: data item index (counting from
             *               0) or -1 if none, e.g. if no
             *               data has been set yet or if
             *               the point is outside the pie
             *               slices.
             *
             *@@added V0.9.2 (2000-02-29) [umoeller]
             */

            case CHTM_ITEMFROMPOINT:
            {
                mrc = (MPARAM)FindItemFromPoint(pChtCData,
                                                SHORT1FROMMP(mp1),
                                                SHORT2FROMMP(mp1));
            break; }

            /*
             *@@ CHTM_SETEMPHASIS:
             *      sets emphasis on a chart slice.
             *
             *      Parameters:
             *
             *      LONG mp1: emphasis to set. Currently
             *              defined values:
             *
             *              -- 0: set selection.
             *
             *              -- 1: set source emphasis.
             *
             *      LONG mp2: zero-based index of slice to
             *              set emphasis for, or -1 to
             *              remove that emphasis.
             *
             *      Returns: TRUE if emphasis was changed.
             *
             *@@added V0.9.12 (2001-05-03) [umoeller]
             */

            case CHTM_SETEMPHASIS:
            {
                LONG lEmph = (LONG)mp1;
                LONG lIndex = (LONG)mp2;
                if (    lEmph > 0
                     && lEmph < 2
                     && (    lIndex >= -1
                          || (    lIndex > 0
                               && lIndex < pChtCData->cd.cValues
                             )
                        )
                   )
                {
                    mrc = (MPARAM)SetEmphasis(hwndChart,
                                              pChtCData,
                                              lEmph,
                                              (MPARAM)-1,
                                              lIndex,
                                              // force flag:
                                              TRUE);
                }
            }
            break;

            /*
             * WM_BUTTON1CLICK:
             * WM_CONTEXTMENU: V0.9.12 (2001-05-03) [umoeller]
             */

            case WM_BUTTON1CLICK:
            case WM_BUTTON1DBLCLK:
            case WM_CONTEXTMENU:
                if (pChtCData->cs.ulStyle & CHS_SELECTIONS)
                {
                    LONG lRegionFound = FindItemFromPoint(pChtCData,
                                                SHORT1FROMMP(mp1),
                                                SHORT2FROMMP(mp1));

                    // selections allowed:
                    // we then accept WM_CHAR msgs, so
                    // we need the focus
                    WinSetFocus(HWND_DESKTOP, hwndChart);
                        // this invalidates the window

                    if (msg == WM_CONTEXTMENU)
                    {
                        // context menu:
                        SendWMControl(hwndChart,
                                      mp1,
                                      CHTN_CONTEXTMENU,
                                      -1,
                                      lRegionFound);
                    }
                    else
                    {
                        // selection changed:
                        SetEmphasis(hwndChart,
                                    pChtCData,
                                    0,    // set selection; caller must set source emphasis
                                    mp1,
                                    lRegionFound,
                                    // force flag:
                                    FALSE);

                        if (msg == WM_BUTTON1DBLCLK)
                            SendWMControl(hwndChart,
                                          mp1,
                                          CHTN_ENTER,
                                          0,
                                          lRegionFound);
                    }
                }
                else
                    // no selections allowed:
                    // just activate the window
                    WinSetActiveWindow(HWND_DESKTOP, hwndChart);
                        // the parent frame gets activated this way
                mrc = (MPARAM)TRUE;
            break;

            /*
             * WM_SETFOCUS:
             *      we might need to redraw the selection.
             */

            case WM_SETFOCUS:
                if (pChtCData->cs.ulStyle & CHS_SELECTIONS)
                {
                    // selections allowed:
                    pChtCData->fHasFocus = (BOOL)mp2;
                    if (pChtCData->lSelected != -1)
                        WinInvalidateRect(hwndChart, NULL, FALSE);
                }

                // 0.9.20 (2002-07-17) [pr]
                SendWMControl(hwndChart,
                              (MPARAM)-1,
                              (BOOL)mp2 ? CHTN_SETFOCUS : CHTN_KILLFOCUS,
                              pChtCData->lSourceEmphasis,
                              pChtCData->lSelected);

            break;

            /*
             * WM_WINDOWPOSCHANGED:
             *
             */

            case WM_WINDOWPOSCHANGED:
            {
                // this msg is passed two SWP structs:
                // one for the old, one for the new data
                // (from PM docs)
                PSWP pswpNew = (PSWP)mp1;
                // PSWP pswpOld = pswpNew + 1;

                // resizing?
                if (pswpNew->fl & SWP_SIZE)
                {
                    if (pChtCData->hbmChart)
                        // invalidate bitmap so that
                        // it will be recreated with new size
                        CleanupBitmap(pChtCData);

                    WinInvalidateRect(hwndChart, NULL, FALSE);
                }
                // return default NULL
            break; }

            /*
             * WM_PRESPARAMCHANGED:
             *
             */

            case WM_PRESPARAMCHANGED:
                if (pChtCData->hbmChart)
                {
                    // invalidate bitmap so that
                    // it will be recreated with new
                    // fonts and colors
                    CleanupBitmap(pChtCData);
                    WinInvalidateRect(hwndChart, NULL, FALSE);
                }
            break;

            /*
             * WM_PAINT:
             *      paint the chart bitmap, which is created
             *      if necessary (calling ctlCreateChartBitmap).
             */

            case WM_PAINT:
            {
                RECTL   rclPaint;
                HPS     hps = WinBeginPaint(hwndChart,
                                            NULLHANDLE, // obtain cached micro-PS
                                            &rclPaint);

                PaintChart(hwndChart,
                           pChtCData,
                           hps,
                           &rclPaint);

                WinEndPaint(hps);
                mrc = 0;
            break; }

            /*
             * WM_DESTROY:
             *      clean up resources
             */

            case WM_DESTROY:
                CleanupBitmap(pChtCData);

                CleanupData(pChtCData);

                free(pChtCData);

                mrc = (*OldStaticProc)(hwndChart, msg, mp1, mp2);
            break;

            default:
                mrc = (*OldStaticProc)(hwndChart, msg, mp1, mp2);
        }
    }

    return mrc;
}

/*
 *@@ ctlChartFromStatic:
 *      this function turns an existing static text control into
 *      a chart control (for visualizing data) by subclassing its
 *      window procedure with ctl_fnwpChart.
 *
 *      See cctl_chart.c for an overview and usage instructions.
 *
 *@@added V0.9.0 [umoeller]
 */

BOOL ctlChartFromStatic(HWND hwndChart)     // in: static control to subclass
{
    if (hwndChart)
    {
        PFNWP OldStaticProc = WinSubclassWindow(hwndChart, ctl_fnwpChart);

        if (OldStaticProc)
        {
            PCHARTCDATA pChtCData = (PCHARTCDATA)malloc(sizeof(CHARTCDATA));
            memset(pChtCData, 0, sizeof(CHARTCDATA));
            pChtCData->OldStaticProc = OldStaticProc;
            pChtCData->fHasFocus = FALSE;
            WinSetWindowPtr(hwndChart, QWL_USER, pChtCData);
            return TRUE;
        }
    }

    return FALSE;
}


