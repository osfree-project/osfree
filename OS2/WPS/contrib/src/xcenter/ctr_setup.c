
/*
 *@@sourcefile ctr_setup.c:
 *      XCenter instance setup.
 *
 *      Function prefix for this file:
 *      --  ctrp* also.
 *
 *      This is all new with V1.0.0 and contains
 *      code formerly in ctr_notebook.c.
 *
 *@@added V1.0.0 (2000-11-27) [umoeller]
 *@@header "shared\center.h"
 */

/*
 *      Copyright (C) 2000-2007 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#pragma strings(readonly)

/*
 *  Suggested #include order:
 *  1)  os2.h
 *  2)  C library headers
 *  3)  setup.h (code generation and debugging options)
 *  4)  headers in helpers\
 *  5)  at least one SOM implementation header (*.ih)
 *  6)  dlgids.h, headers in shared\ (as needed)
 *  7)  headers in implementation dirs (e.g. filesys\, as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSMODULEMGR
#define INCL_DOSSEMAPHORES      // V0.9.20 (2002-07-23) [lafaix] needed for plugins.h
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINMENUS
#define INCL_WINDIALOGS
#define INCL_WINBUTTONS
#define INCL_WINSTATICS
#define INCL_WINSTDCNR
#define INCL_WINSTDSLIDER
#include <os2.h>

// C library headers
#include <stdio.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\apmh.h"               // Advanced Power Management helpers
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers

// SOM headers which don't crash with prec. header files
#include "xcenter.ih"
// #include "xfobj.ih"                     // XFldObject

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\errors.h"              // private XWorkplace error codes
#include "shared\notebook.h"            // generic XWorkplace notebook handling
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)
#include "shared\plugins.h"             // generic plugins support

#include "shared\center.h"              // public XCenter interfaces
#include "xcenter\centerp.h"            // private XCenter implementation

#include "hook\xwphook.h"
#include "config\hookintf.h"            // daemon/hook interface

#pragma hdrstop                     // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

/* ******************************************************************
 *
 *   XCenter setup set (see XWPSETUPENTRY)
 *
 ********************************************************************/

/*
 *@@ G_XCenterSetupSet:
 *      setup set of the XCenter. See XWPSETUPENTRY.
 *
 *      Presently used key values:
 *
 +          1: holds the packed setup string.
 +          2: _ulWindowStyle,
 +          3: _ulAutoHide,
 +          4: _flDisplayStyle,
 +          5: _fHelpDisplayed,
 +          6: _ulPriorityClass,
 +          7: _lPriorityDelta,        // this is a LONG
 +          8: _ulPosition,
 +          9: _ul3DBorderWidth,
 +          10: _ulBorderSpacing,
 +          11: _ulWidgetSpacing,
 +          12: _fReduceDesktopWorkarea
 +          13: _pszClientFont          // V0.9.9 (2001-03-07) [umoeller]
 +          14: _lcolClientBackground
 +          15: _fHideOnClick           // V0.9.14 (2001-08-21) [umoeller]
 +          16: _ulHeight               // thanks for mentioning your changes here, martin
 +          17: _ulWidth
 +          18: _fAutoScreenBorder      // V0.9.19 (2002-05-07) [umoeller]
 +
 *@@added V0.9.9 (2001-01-29) [umoeller]
 *@@changed V0.9.14 (2001-08-21) [umoeller]: added "hide on click"
 *@@changed V0.9.16 (2001-10-15) [umoeller]: changed defaults
 *@@changed V0.9.19 (2002-05-07) [umoeller]: added "auto screen border"
 */

extern const XWPSETUPENTRY    G_XCenterSetupSet[] =
    {
        /*
         * ulWindowStyle bitfield... first a LONG, then the bitfields
         *
         */

        // type,  setup string,     offset,
        STG_LONG_DEC,    NULL,           FIELDOFFSET(XCenterData, ulWindowStyle),
        //     key for wpSaveState/wpRestoreState
               2,      // bitfield! only first item!
        //     default, ulExtra,            min, max
               WS_ANIMATE,  0,              0,   0,

        // type,  setup string,     offset,
        STG_BITFLAG, "ALWAYSONTOP",  FIELDOFFSET(XCenterData, ulWindowStyle),
        //     key for wpSaveState/wpRestoreState
               0,      // bitfield! only first item!
        //     default, ulExtra,            min, max
               0,       WS_TOPMOST,         0,   0,

        // type,  setup string,     offset,
        STG_BITFLAG, "ANIMATE",     FIELDOFFSET(XCenterData, ulWindowStyle),
        //     key for wpSaveState/wpRestoreState
               0,      // bitfield! only first item!
        //     default, ulExtra,            min, max
               WS_ANIMATE,  WS_ANIMATE,     0,   0,    // V0.9.9 (2001-02-28) [pr]: fix setup string default

        /*
         * flDisplayStyle bitfield... first a LONG, then the bitfields
         *
         */

        // type,  setup string,     offset,
        STG_LONG_DEC,    NULL,             FIELDOFFSET(XCenterData, flDisplayStyle),
        //     key for wpSaveState/wpRestoreState
               4,      // bitfield! only first item!
        //     default, ulExtra,            min, max
               XCS_FLATBUTTONS | /* XCS_SUNKBORDERS | */ XCS_SIZINGBARS | XCS_SPACINGLINES,
                                 // ^^^ fixed V0.9.19 (2002-04-25) [umoeller]
                        0,                  0,   0,

        // type,  setup string,     offset,
        STG_BITFLAG, "FLATBUTTONS",    FIELDOFFSET(XCenterData, flDisplayStyle),
        //     key for wpSaveState/wpRestoreState
               0,      // bitfield! only first item!
        //     default, ulExtra,            min, max
               XCS_FLATBUTTONS, XCS_FLATBUTTONS, 0,   0,

        // type,  setup string,     offset,
        STG_BITFLAG, "SUNKBORDERS",    FIELDOFFSET(XCenterData, flDisplayStyle),
        //     key for wpSaveState/wpRestoreState
               0,      // bitfield! only first item!
        //     default, ulExtra,            min, max
               0, XCS_SUNKBORDERS, 0,   0,
                    // default changed V0.9.16 (2001-10-15) [umoeller]

        // type,  setup string,     offset,
        STG_BITFLAG, "SIZINGBARS",    FIELDOFFSET(XCenterData, flDisplayStyle),
        //     key for wpSaveState/wpRestoreState
               0,      // bitfield! only first item!
        //     default, ulExtra,            min, max
               XCS_SIZINGBARS, XCS_SIZINGBARS, 0,   0,

        // type,  setup string,     offset,
        STG_BITFLAG, "ALL3DBORDERS",    FIELDOFFSET(XCenterData, flDisplayStyle),
        //     key for wpSaveState/wpRestoreState
               0,      // bitfield! only first item!
        //     default, ulExtra,            min, max
               0, XCS_ALL3DBORDERS, 0,   0,

        // type,  setup string,     offset,
        STG_BITFLAG, "SPACINGLINES",    FIELDOFFSET(XCenterData, flDisplayStyle),
        //     key for wpSaveState/wpRestoreState
               0,      // bitfield! only first item!
        //     default, ulExtra,            min, max
               XCS_SPACINGLINES, XCS_SPACINGLINES, 0,   0,
                    // default changed V0.9.16 (2001-10-15) [umoeller]

        // type,  setup string,     offset,
        // V0.9.16 (2001-10-24) [umoeller]
        STG_BITFLAG, "NOHATCHOPENOBJ",      FIELDOFFSET(XCenterData, flDisplayStyle),
        //     key for wpSaveState/wpRestoreState
               0,      // bitfield! only first item!
        //     default, ulExtra,            min, max
               0,       XCS_NOHATCHINUSE,   0,   0,
                    // default changed V0.9.16 (2001-10-15) [umoeller]

        /*
         * other LONGs
         *
         */

        // type,  setup string,     offset,
        STG_LONG_DEC,    "AUTOHIDE",    FIELDOFFSET(XCenterData, ulAutoHide),
        //     key for wpSaveState/wpRestoreState
               3,
        //     default, ulExtra,            min, max
               0,       0,                  0,   60000,

        // type,  setup string,     offset,
        // setting removed V1.0.1 (2003-02-02) [umoeller]
        // STG_BOOL, NULL,               FIELDOFFSET(XCenterData, fHelpDisplayed),
        //     key for wpSaveState/wpRestoreState
        //        5,
        //     default, ulExtra,            min, max
        //        FALSE,   0,                  0,   1,

        // type,  setup string,     offset,
        /*      removed V0.9.20 (2002-08-08) [umoeller]
        STG_LONG_DEC, "PRIORITYCLASS",    FIELDOFFSET(XCenterData, ulPriorityClass),
        //     key for wpSaveState/wpRestoreState
               6,
        //     default, ulExtra,            min, max
               PRTYC_REGULAR, 0,            1,   4,
                                            // PRTYC_IDLETIME == 1
                                            // PRTYC_FOREGROUNDSERVER = 4
        */

        // type,  setup string,     offset,
        STG_LONG_DEC, "PRIORITYDELTA",    FIELDOFFSET(XCenterData, lPriorityDelta),
        //     key for wpSaveState/wpRestoreState
               7,
        //     default, ulExtra,            min, max
               0,       0,                  0,   31,

        // type,  setup string,     offset,
        STG_LONG_DEC, NULL,               FIELDOFFSET(XCenterData, ulPosition),
        //     key for wpSaveState/wpRestoreState
               8,
        //     default, ulExtra,            min, max
               XCENTER_BOTTOM, 0,           0,   3,

        // type,  setup string,     offset,
        STG_LONG_DEC, "3DBORDERWIDTH",    FIELDOFFSET(XCenterData, ul3DBorderWidth),
        //     key for wpSaveState/wpRestoreState
               9,
        //     default, ulExtra,            min, max
               2,       0,                  0,   10,
               // changed default V0.9.16 (2001-12-08) [umoeller]

        // type,  setup string,     offset,
        STG_LONG_DEC, "BORDERSPACING",    FIELDOFFSET(XCenterData, ulBorderSpacing),
        //     key for wpSaveState/wpRestoreState
               10,
        //     default, ulExtra,            min, max
               2,       0,                  0,   10,

        // type,  setup string,     offset,
        STG_LONG_DEC, "WIDGETSPACING",    FIELDOFFSET(XCenterData, ulWidgetSpacing),
        //     key for wpSaveState/wpRestoreState
               11,
        //     default, ulExtra,            min, max
               2,       0,                  1,   10,

        // type,  setup string,     offset,
        STG_BOOL,    "REDUCEDESKTOP",  FIELDOFFSET(XCenterData, fReduceDesktopWorkarea),
        //     key for wpSaveState/wpRestoreState
               12,
        //     default, ulExtra,            min, max
               FALSE,   0,                  0,   0,

        // V0.9.14 (2001-08-21) [umoeller]
        // type,  setup string,     offset,
        STG_BOOL,    "HIDEONCLICK", FIELDOFFSET(XCenterData, fHideOnClick),
        //     key for wpSaveState/wpRestoreState
               15,
        //     default, ulExtra,            min, max
               FALSE,   0,                  0,   0,

        // V0.9.19 (2002-05-07) [umoeller]
        // type,  setup string,     offset,
        STG_BOOL,    "AUTOSCREENBORDER", FIELDOFFSET(XCenterData, fAutoScreenBorder),
        //     key for wpSaveState/wpRestoreState
               18,
        //     default, ulExtra,            min, max
               FALSE,   0,                  0,   0,  // V1.0.6 (2006-09-30) [pr]: changed default

        // V0.9.19 (2002-04-16) [lafaix]
        // type,  setup string,     offset,
        STG_LONG_DEC,    "HEIGHT", FIELDOFFSET(XCenterData, ulHeight),
        //     key for wpSaveState/wpRestoreState
               16,
        //     default, ulExtra,            min, max
               0,       0,                  0,   0xFFFF,

        // V0.9.19 (2002-04-16) [lafaix]
        // type,  setup string,     offset,
        STG_LONG_DEC,    "WIDTH", FIELDOFFSET(XCenterData, ulWidth),
        //     key for wpSaveState/wpRestoreState
               17,
        //     default, ulExtra,            min, max
               0,       0,                  0,   0xFFFF,

        /*
         * fonts/colors
         *      V0.9.9 (2001-03-07) [umoeller]
         */

        // type,  setup string,     offset,
        STG_PSZ,     "CLIENTFONT",  FIELDOFFSET(XCenterData, pszClientFont),
        //     key for wpSaveState/wpRestoreState
               13,
        //     default, ulExtra,            min, max
               (LONG)NULL, 0,               0,   0,

        // type,  setup string,     offset,
        STG_LONG_RGB,    "CLIENTCOLOR", FIELDOFFSET(XCenterData, lcolClientBackground),
        //     key for wpSaveState/wpRestoreState
               14,
        //     default, ulExtra,            min, max
               0xCCCCCC, 0,                 0,   0x00FFFFFF
    };

// V1.0.0 (2002-08-12) [umoeller]
extern ULONG G_cXCenterSetupSetEntries = ARRAYITEMCOUNT(G_XCenterSetupSet);

/*
 *@@ ctrpInitData:
 *      part of the implementation for XCenter::wpInitData.
 *
 *@@added V0.9.9 (2001-01-29) [umoeller]
 */

VOID ctrpInitData(XCenter *somSelf)
{
    XCenterData *somThis = XCenterGetData(somSelf);
    cmnSetupInitData(G_XCenterSetupSet,
                     ARRAYITEMCOUNT(G_XCenterSetupSet),
                     somThis);
}

STATIC VOID AppendWidgetSettings(PXSTRING pstrSetup,
                                 PLINKLIST pllSettings);

/*
 *@@ ctrpAppendWidgetSettings:
 *      appends the given widget setting as a
 *      setup string to the given XSTRING.
 *
 *      Used by AppendWidgetSettings (into which
 *      this will also recurse), but also by
 *      drag'n'drop to drop trays across XCenters
 *      correctly.
 *
 *      pstrSetup is assumed to be initialized.
 *
 *      In addition, you must pass in a BOOL variable
 *      in pfFirstWidget that must be set to FALSE.
 *
 *      Finally, this uses pstrTemp as a temp buffer
 *      for speed, which must be initialized and
 *      cleared after use also.
 *
 *@@added V0.9.19 (2002-05-04) [umoeller]
 */

VOID ctrpAppendWidgetSettings(PXSTRING pstrSetup,
                              PPRIVATEWIDGETSETTING pSetting,
                              PBOOL pfFirstWidget,
                              PXSTRING pstrTemp)
{
    ULONG ulSetupLen;

    if (!*pfFirstWidget)
        // not first run:
        // add separator
        xstrcatc(pstrSetup, ',');
    else
        *pfFirstWidget = FALSE;

    // add widget class
    xstrcat(pstrSetup, pSetting->Public.pszWidgetClass, 0);

    // add first separator
    xstrcatc(pstrSetup, '(');

    if (    (pSetting->Public.pszSetupString)
         && (ulSetupLen = strlen(pSetting->Public.pszSetupString))
       )
    {
        // widget has a setup string:
        // copy widget setup string to temporary buffer
        // for encoding... this has "=" and ";"
        // chars in it, and these should not appear
        // in the WPS setup string
        xstrcpy(pstrTemp,
                pSetting->Public.pszSetupString,
                ulSetupLen);

        xstrEncode(pstrTemp,
                   "%,{}[]();=");
                        // added {}[] V0.9.19 (2002-04-25) [umoeller]

        // now append encoded widget setup string
        xstrcats(pstrSetup, pstrTemp);

    } // end if (    (pSetting->pszSetupString)...

    // add terminator
    xstrcatc(pstrSetup, ')');

    // add trays in round brackets, if we have any
    if (pSetting->pllTraySettings)
    {
        PLISTNODE pTrayNode = lstQueryFirstNode(pSetting->pllTraySettings);

        xstrcatc(pstrSetup, '{');

        while (pTrayNode)
        {
            PTRAYSETTING pTraySetting = (PTRAYSETTING)pTrayNode->pItemData;

            // "Tray(setupstring){Tray1[widget1,widget],Tray2[widget]}"

            xstrcat(pstrSetup,
                    pTraySetting->pszTrayName,
                    0);

            // add subwidgets in square brackets
            xstrcatc(pstrSetup, '[');

            // recurse
            AppendWidgetSettings(pstrSetup,
                                 &pTraySetting->llSubwidgetSettings);

            xstrcatc(pstrSetup, ']');

            pTrayNode = pTrayNode->pNext;
        }

        xstrcatc(pstrSetup, '}');
    }
}

/*
 *@@ AppendWidgetSettings:
 *      appends all widgets in pllSettings to the
 *      given XSTRING, including trays and subwidgets,
 *      if any.
 *
 *@@added V0.9.19 (2002-04-25) [umoeller]
 */

STATIC VOID AppendWidgetSettings(PXSTRING pstrSetup,
                                 PLINKLIST pllSettings)
{
    BOOL    fFirstWidget = TRUE;
    XSTRING strSetup2;
    PLISTNODE pNode = lstQueryFirstNode(pllSettings);

    xstrInit(&strSetup2, 0);

    while (pNode)
    {
        PPRIVATEWIDGETSETTING pSetting = (PPRIVATEWIDGETSETTING)pNode->pItemData;

        ctrpAppendWidgetSettings(pstrSetup, pSetting, &fFirstWidget, &strSetup2);

        pNode = pNode->pNext;
    } // end for widgets

    xstrClear(&strSetup2);
}

/*
 *@@ ctrpQuerySetup:
 *      implementation for XCenter::xwpQuerySetup2.
 *
 *@@added V0.9.7 (2000-12-09) [umoeller]
 *@@changed V0.9.19 (2002-04-25) [umoeller]: finally added trays support
 */

BOOL ctrpQuerySetup(XCenter *somSelf,
                    PVOID pstrSetup)
{
    BOOL brc = TRUE;

    // V0.9.16 (2001-10-11) [umoeller]:
    // removed object lock
    // this is properly handled by xwpQuerySetup already

    // compose setup string

    TRY_LOUD(excpt2)
    {
        XCenterData *somThis = XCenterGetData(somSelf);

        // temporary buffer for building the setup string
        PLINKLIST pllSettings = ctrpQuerySettingsList(somSelf);
        PLISTNODE pNode;

        /*
         * build string
         *
         */

        if (_ulPosition == XCENTER_TOP)
            xstrcat(pstrSetup, "POSITION=TOP;", 0);

        // use array for the rest...
        cmnSetupBuildString(G_XCenterSetupSet,
                            ARRAYITEMCOUNT(G_XCenterSetupSet),
                            somThis,
                            pstrSetup);

        // now build widgets string... this is complex.
        if (pNode = lstQueryFirstNode(pllSettings))
        {
            // we have widgets:
            // go thru all of them and list all widget classes and setup strings

            xstrcat(pstrSetup, "WIDGETS=", 0);

            AppendWidgetSettings(pstrSetup, pllSettings);
                    // V0.9.19 (2002-04-25) [umoeller]

            xstrcatc(pstrSetup, ';');
        }
    }
    CATCH(excpt2)
    {
        brc = FALSE;
    } END_CATCH();

    return brc;
}

/*
 *@@ CreateWidgetFromString:
 *
 *@@added V0.9.19 (2002-04-25) [umoeller]
 */

STATIC BOOL CreateWidgetFromString(XCenter *somSelf,
                                   PCSZ pcszClass,
                                   PCSZ pcszSetup,
                                   ULONG ulTrayWidgetIndex,
                                   ULONG ulTrayIndex,
                                   PULONG pcWidgets,
                                   PBOOL pfIsTray)
{
    WIDGETPOSITION pos2;
    APIRET arc;

    _PmpfF(("creating \"%s\", \"%s\"",
                STRINGORNULL(pcszClass),
                STRINGORNULL(pcszSetup)));

    pos2.ulTrayWidgetIndex = ulTrayWidgetIndex;
    pos2.ulTrayIndex = ulTrayIndex;
    pos2.ulWidgetIndex = -1;        // to the right
    if (arc = _xwpCreateWidget(somSelf,
                               (PSZ)pcszClass,
                               (PSZ)pcszSetup,
                               &pos2))
    {
        // error:
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "Creating widget \"%s\" (\"%s\") failed, rc = %d.",
               pcszClass,
               pcszSetup,
               arc);
        return FALSE;
    }

    if (pfIsTray)
        *pfIsTray = (!strcmp(pcszClass, TRAY_WIDGET_CLASS_NAME));

    ++(*pcWidgets);

    return TRUE;
}

STATIC BOOL ParseWidgetsString(XCenter *somSelf,
                               PSZ pszWidgets,      // in: WIDGETS= data only
                               ULONG ulTrayWidgetIndex,
                               ULONG ulTrayIndex);

/*
 *@@ ParseTraysList:
 *      called from ParseWidgetsString if a
 *      trays list is encountered. We will then
 *      recurse into ParseWidgetsString with
 *      the subwidget lists.
 *
 *@@added V0.9.19 (2002-04-25) [umoeller]
 */

STATIC BOOL ParseTraysList(XCenter *somSelf,
                           PCSZ pcszTraysList,
                           ULONG ulTrayWidgetIndex)
{
    BOOL brc = TRUE;

    // on input, we get
    // "Tray1[widget1,widget]Tray2[widget]"
    PCSZ pTrayThis = pcszTraysList;
    ULONG cTraysSet = 0;
    while (    (pTrayThis)
            && (*pTrayThis)
            && (brc)
          )
    {
        // tray with widgets list comes next in square brackets
        PCSZ    pOpen,
                pClose;
        if (    (!(pOpen = strchr(pTrayThis, '[')))
             || (!(pClose = strchr(pOpen, ']')))
           )
            brc = FALSE;
        else
        {
            PSZ pszTrayName;
            if (!(pszTrayName = strhSubstr(pTrayThis,
                                           pOpen)))
                brc = FALSE;
            else
            {
                PSZ pszSubwidgetsList;
                APIRET arc;

                if (!cTraysSet)
                    // first tray: the tray widget creates
                    // an automatic tray if there's none
                    // on creation, so rename this
                    arc = _xwpRenameTray(somSelf,
                                         ulTrayWidgetIndex,
                                         0,
                                         pszTrayName);
                else
                    arc = _xwpCreateTray(somSelf,
                                         ulTrayWidgetIndex,
                                         pszTrayName);

                if (arc)
                    brc = FALSE;
                else
                {

                    // now recurse with the widgets list
                    if (!(pszSubwidgetsList = strhSubstr(pOpen + 1,
                                                         pClose)))
                        brc = FALSE;
                    else
                    {
                        _PmpfF(("recursing with string \"%s\"",
                              pszSubwidgetsList));

                        brc = ParseWidgetsString(somSelf,
                                                 pszSubwidgetsList,
                                                 ulTrayWidgetIndex,
                                                 cTraysSet);

                        _PmpfF(("ParseWidgetsString returned %d", brc));

                        free(pszSubwidgetsList);
                    }

                    // next tray after closing bracket
                    pTrayThis = pClose + 1;

                    ++cTraysSet;
                }
            }
        }
    }

    return brc;
}

/*
 *@@ ParseWidgetsString:
 *      implementation for the WIDGETS=xxx setup
 *      string particle in ctrpSetup.
 *
 *      Initially, this gets called from ctrpSetup
 *      with ulTrayWidgetIndex and ulTrayIndex
 *      both set to -1 to create the root widgets.
 *      If we find trays, we call ParseTraysList,
 *      which will recurse into this routine with
 *      the tray widget and tray indices set
 *      accordingly.
 *
 *      I am not sure that I will understand what
 *      this code is doing one week from now, but
 *      apparently it's working for now.
 *
 *@@added V0.9.19 (2002-04-25) [umoeller]
 */

STATIC BOOL ParseWidgetsString(XCenter *somSelf,
                               PSZ pszWidgets,      // in: data from WIDGETS= only
                               ULONG ulTrayWidgetIndex,
                               ULONG ulTrayIndex)
{
    BOOL brc = TRUE;

    // now, off we go...
    // parse the WIDGETS string
    // format is: "widget1,widget2,widget3"
    // where each "widget" is one of
    // -- plain widget class name, e.g. "Pulse"
    // -- widget class name with encoded setup
    //    string, e.g. "Pulse(WIDTH%3D130%3)"
    // -- a tray with even more subwidgets,
    //    e.g. "Tray(setupstring){Tray1[widget1,widget]Tray2[widget]}"

    PCSZ    pThis = pszWidgets,
            pStartOfWidget = pszWidgets;
    CHAR    c;

    BOOL    fStop = FALSE;

    ULONG   cWidgetsCreated = 0;
    BOOL    fLastWasTray = FALSE;

    while (    (!fStop)
            && (brc)
          )
    {
        switch (c = *pThis)
        {
            case '\0':
            case ',':
            {
                // OK, widget terminator:
                // then we had no setup string
                PSZ pszWidgetClass = strhSubstr(pStartOfWidget,
                                                pThis);     // up to comma

                brc = CreateWidgetFromString(somSelf,
                                             pszWidgetClass,
                                             NULL,
                                             ulTrayWidgetIndex,
                                             ulTrayIndex,
                                             &cWidgetsCreated,
                                             NULL);

                FREE(pszWidgetClass);

                if (!c)
                    fStop = TRUE;
                else
                    pStartOfWidget = pThis + 1;
            }
            break;

            case '(':
            {
                // beginning of setup string:
                // extract setup
                PCSZ pClose;
                if (!(pClose = strchr(pThis + 1, ')')))
                {
                    cmnLog(__FILE__, __LINE__, __FUNCTION__,
                           "Expected ')' after \"%s\"",
                           pThis + 1);
                    brc = FALSE;
                }
                else
                {
                    // extract widget class before bracket
                    PSZ pszWidgetClass;
                    if (!(pszWidgetClass = strhSubstr(pStartOfWidget,
                                                      pThis)))     // up to bracket
                    {
                        cmnLog(__FILE__, __LINE__, __FUNCTION__,
                               "Invalid widget class at \"%s\"",
                               pStartOfWidget);
                        brc = FALSE;
                    }
                    else
                    {
                        // copy widget setup string to temporary
                        // buffer for decoding...
                        XSTRING strSetup2;
                        PSZ pszWidgetSetup = strhSubstr(pThis + 1,
                                                        pClose);

                        xstrInitSet(&strSetup2,
                                    pszWidgetSetup);
                        xstrDecode(&strSetup2);
                        pszWidgetSetup = strSetup2.psz;

                        brc = CreateWidgetFromString(somSelf,
                                                     pszWidgetClass,
                                                     pszWidgetSetup,
                                                     ulTrayWidgetIndex,
                                                     ulTrayIndex,
                                                     &cWidgetsCreated,
                                                     &fLastWasTray);

                        FREE(pszWidgetSetup);
                        FREE(pszWidgetClass);
                    }

                    if (brc)
                    {
                        // continue after closing bracket;
                        // next will either be a comma or, if this
                        // is a tray, a square bracket
                        pThis = pClose + 1;

                        switch (*pThis)
                        {
                            case ',':
                                ++pThis;
                                pStartOfWidget = pThis;
                                continue;

                            case '{':
                                continue;

                            case '\0':
                                fStop = TRUE;
                            break;

                            default:
                                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                                       "Expected ',' after \"%s\"",
                                       pClose);
                                brc = FALSE;
                        }
                    }
                }
            }
            break;

            case '{':
            {
                // beginning of trays list for tray widget:
                // then we must have created a widget already,
                // or this will fail
                if (!fLastWasTray)
                    brc = FALSE;
                else
                {
                    PCSZ pClose;
                    if (!(pClose = strchr(pThis + 1, '}')))
                    {
                        cmnLog(__FILE__, __LINE__, __FUNCTION__,
                               "Expected '}' after \"%s\"",
                               pThis + 1);
                        brc = FALSE;
                    }
                    else
                    {
                        // now run thru the trays list:
                        PSZ pszTraysList;
                        if (!(pszTraysList = strhSubstr(pThis + 1,
                                                        pClose)))
                        {
                            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                                   "Invalid trays list at \"%s\"",
                                   pThis + 1);
                            brc = FALSE;
                        }
                        else
                        {
                            // alright, call this subroutine,
                            // which will recurse into this routine
                            // with the subwidgets lists
                            brc = ParseTraysList(somSelf,
                                                 pszTraysList,
                                                 cWidgetsCreated - 1);
                            free(pszTraysList);
                        }
                    }

                    if (brc)
                    {
                        // continue after closing bracket;
                        // next _must_ be a comma
                        pThis = pClose + 1;

                        switch (*pThis)
                        {
                            case ',':
                                ++pThis;
                                pStartOfWidget = pThis;
                                continue;

                            case '\0':
                                fStop = TRUE;
                            break;

                            default:
                                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                                       "Expected ',' after \"%s\"",
                                       pClose);
                                brc = FALSE;
                        }
                    }
                }
            }
            break;

            case ')':
            case '}':
            case '[':
            case ']':
                // loose closing brackets: that's invalid syntax
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "Unexpected character '%c' at \"%s\"",
                       *pThis,
                       pThis);
                brc = FALSE;
        }

        pThis++;
    }

    return brc;
}

/*
 *@@ AppendObjects:
 *      helper for CreateDefaultWidgets.
 *
 *@@added V0.9.19 (2002-04-25) [umoeller]
 *@@changed V1.0.6 (2006-08-20) [pr]: fix default Xcenter creation at install time @@fixes 749
 */

STATIC ULONG AppendObjects(PXSTRING pstr,
                           PCSZ *apcsz,
                           ULONG c)
{
    ULONG cAdded = 0;
    ULONG ul;
    for (ul = 0;
         ul < c;
         ++ul)
    {
        // V1.0.6 (2006-08-20) [pr]: unnecessary check causes install problems with
        // creating the default XCenter due to the dependent objects not having had
        // time to be flushed to the INI file.
        // if (cmnQueryObjectFromID(apcsz[ul]))
        {
            if (cAdded)
                xstrcatc(pstr, ',');

            xstrcat(pstr,
                    "ObjButton(OBJECTHANDLE%3D",
                    0);
            xstrcat(pstr,
                    apcsz[ul],
                    0);
            xstrcat(pstr,
                    "%3B)",
                    0);

            ++cAdded;
        }
    }

    return cAdded;
}

/*
 *@@ CreateDefaultWidgets:
 *      this creates the default widgets in the XCenter.
 *      by creating a setup string accordingly and then
 *      calling ParseWidgetsString directly, which will
 *      go create the widgets from the string.
 *
 *      NOTE: This
 *
 *@@added V0.9.20 (2002-07-19) [umoeller]
 */

STATIC BOOL CreateDefaultWidgets(XCenter *somSelf)
{
    BOOL brc;

    static const char *apcszMain[] =
        {
            "<XWP_LOCKUPSTR>",
            "<XWP_FINDSTR>",
            "<XWP_SHUTDOWNSTR>",
        };
    static const char *apcszTray1[] =
        {
            "<WP_DRIVES>",
            "<WP_CONFIG>",
            "<WP_PROMPTS>"
        };
    ULONG ul;
    XSTRING str;
    WPObject *pobj;
    CHAR sz[200];

    xstrInitCopy(&str,
                 "XButton(),",
                 0);

    if (AppendObjects(&str,
                      apcszMain,
                      ARRAYITEMCOUNT(apcszMain)))
        xstrcatc(&str, ',');

    xstrcat(&str,
            "Pulse(),"
            "Tray(){",
            0);

    // create a default tray
    sprintf(sz,
            cmnGetString(ID_CRSI_TRAY),     // tray %d
            1);
    xstrcat(&str,
            sz,
            0);

    xstrcatc(&str, '[');

    AppendObjects(&str,
                  apcszTray1,
                  ARRAYITEMCOUNT(apcszTray1));

    xstrcat(&str,
            "]},WindowList,"
            "Time",
            0);

    // on laptops, add battery widget too
    if (apmhHasBattery())
        xstrcat(&str,
                 ",Power()",
                 0);

    // call ParseWidgetsString directly
    brc = ParseWidgetsString(somSelf,
                             str.psz,
                             -1,
                             -1);

    xstrClear(&str);

    return TRUE;        // don't let this fail  V1.0.0 (2002-09-20) [umoeller]
}

/*
 *@@ ctrpSetup:
 *      implementation for XCenter::wpSetup.
 *
 *@@added V0.9.7 (2001-01-25) [umoeller]
 *@@changed V0.9.16 (2001-10-15) [umoeller]: fixed widget clearing which caused a log entry for empty XCenter
 *@@changed V0.9.19 (2002-04-25) [umoeller]: added exception handling; rewrote WIDGETS= setup string to handle trays
 *@@changed V1.0.7 (2006-12-31) [pr]: added ADDWIDGETS and DELETEWIDGETS setup strings @@fixes 906
 */

BOOL ctrpSetup(XCenter *somSelf,
               PSZ pszSetupString)
{
    XCenterData *somThis = XCenterGetData(somSelf);
    ULONG   cSuccess = 0, cb;
    BOOL    brc;
    PSZ     pszWidgets;

    // scan the standard stuff from the table...
    // this saves us a lot of work.

    TRY_LOUD(excpt1)
    {
        // _PmpfF(("string is \"%s\"", pszSetupString));

        // now comes the non-standard stuff:
        if (brc = cmnSetupScanString(somSelf,
                                     G_XCenterSetupSet,
                                     ARRAYITEMCOUNT(G_XCenterSetupSet),
                                     somThis,
                                     pszSetupString,
                                     &cSuccess))
        {
            CHAR    szValue[100];

            _Pmpf(("   cmnSetupScanString returned TRUE"));
            cb = sizeof(szValue);
            if (_wpScanSetupString(somSelf,
                                   pszSetupString,
                                   "POSITION",
                                   szValue,
                                   &cb))
            {
                if (!stricmp(szValue, "TOP"))
                    _ulPosition = XCENTER_TOP;
                else if (!stricmp(szValue, "BOTTOM"))
                    _ulPosition = XCENTER_BOTTOM;
                else
                    brc = FALSE;
            }
        }

        if (brc)
        {
            // WIDGETS can be very long, so query size first
            _Pmpf(("   brc still TRUE; scanning WIDGETS string"));

            if (_wpScanSetupString(somSelf,
                                   pszSetupString,
                                   "WIDGETS",
                                   NULL,
                                   &cb))
            {
                _Pmpf(("got WIDGETS string, %d bytes, nuking existing widgets",
                       cb));

                if (    cb
                     && (pszWidgets = malloc(cb))
                   )
                {
                    if (_wpScanSetupString(somSelf,
                                           pszSetupString,
                                           "WIDGETS",
                                           pszWidgets,
                                           &cb))
                    {
                        // first of all, remove all existing widgets,
                        // we have a replacement here
                        WIDGETPOSITION pos;
                        pos.ulTrayWidgetIndex = -1;
                        pos.ulTrayIndex = -1;
                        pos.ulWidgetIndex = 0;      // leftmost widget
                        while (ctrpQueryWidgetsCount(somSelf))       // V0.9.16 (2001-10-15) [umoeller]
                            if (_xwpDeleteWidget(somSelf,
                                                 &pos))
                            {
                                // error:
                                brc = FALSE;
                                break;
                            }

                        // allow "CLEAR" to just nuke the widgets
                        // V0.9.19 (2002-04-25) [umoeller]
                        if (!stricmp(pszWidgets, "CLEAR"))
                            ;
                        // allow "RESET" to reset the widgets to
                        // the defaults
                        // V0.9.20 (2002-07-19) [umoeller]
                        else if (!stricmp(pszWidgets, "RESET"))
                            brc = CreateDefaultWidgets(somSelf);
                        else
                            brc = ParseWidgetsString(somSelf,
                                                     pszWidgets,
                                                     -1,
                                                     -1);
                    }

                    free(pszWidgets);
                } // end if (    (pszWidgets)...
            }
        }

        // V1.0.7 (2006-12-31) [pr]
        if (brc)
        {
            _Pmpf(("   brc still TRUE; scanning ADDWIDGETS string"));
            if (_wpScanSetupString(somSelf,
                                   pszSetupString,
                                   "ADDWIDGETS",
                                   NULL,
                                   &cb))
            {
                _Pmpf(("got ADDWIDGETS string, %d bytes", cb));
                if (    cb
                     && (pszWidgets = malloc(cb))
                   )
                {
                    if (_wpScanSetupString(somSelf,
                                           pszSetupString,
                                           "ADDWIDGETS",
                                           pszWidgets,
                                           &cb))
                        brc = ParseWidgetsString(somSelf, pszWidgets, -1, -1);

                    free(pszWidgets);
                }
            }
        }

        if (brc)
        {
            _Pmpf(("   brc still TRUE; scanning DELETEWIDGETS string"));
            if (_wpScanSetupString(somSelf,
                                   pszSetupString,
                                   "DELETEWIDGETS",
                                   NULL,
                                   &cb))
            {
                _Pmpf(("got DELETEWIDGETS string, %d bytes", cb));
                if (    cb
                     && (pszWidgets = malloc(cb))
                   )
                {
                    if (_wpScanSetupString(somSelf,
                                           pszSetupString,
                                           "DELETEWIDGETS",
                                           pszWidgets,
                                           &cb))
                    {
                        PCSZ    pThis = pszWidgets,
                                pStartOfWidget = pszWidgets;

                        for (; brc; pThis++)
                            if ((*pThis == '\0') || (*pThis == ','))
                            {
                                // OK, widget terminator:
                                // then we had no setup string
                                PSZ pszWidgetClass = strhSubstr(pStartOfWidget,
                                                                pThis);     // up to comma

                                brc = !_xwpDeleteWidgetClass(somSelf, pszWidgetClass);
                                FREE(pszWidgetClass);
                                if (*pThis)
                                    pStartOfWidget = pThis + 1;
                                else
                                    break;
                            }
                    }

                    free(pszWidgets);
                }
            }
        }
    }
    CATCH(excpt1)
    {
        brc = FALSE;
    } END_CATCH();

    return brc;
}

/*
 *@@ ctrpSetupOnce:
 *      implementation for XCenter::wpSetupOnce.
 *      Creates the default widgets if none are given.
 *
 *@@added V0.9.16 (2001-10-15) [umoeller]
 *@@changed V0.9.19 (2002-04-25) [umoeller]: added default tray with objects
 *@@changed V0.9.20 (2002-07-19) [umoeller]: now checking for whether WIDGETS is specified
 */

BOOL ctrpSetupOnce(XCenter *somSelf,
                   PSZ pszSetupString)
{
    BOOL brc = TRUE;

    WPObject *pobjLock = NULL;
    TRY_LOUD(excpt1)
    {
        ULONG cb;
        // create default widgets if we have no widgets yet
        // AND the WIDGETS string is not specified on
        // creation (otherwise ctrpSetup will handle this)
        if (    (!_wpScanSetupString(somSelf,           // V0.9.20 (2002-07-19) [umoeller]
                                     pszSetupString,
                                     "WIDGETS",
                                     NULL,
                                     &cb))
             && (pobjLock = cmnLockObject(somSelf))
           )
        {
            PLINKLIST pllSettings = ctrpQuerySettingsList(somSelf);
            if (!lstCountItems(pllSettings))
                CreateDefaultWidgets(somSelf);
        }
    }
    CATCH(excpt1) {} END_CATCH();

    if (pobjLock)
        _wpReleaseObjectMutexSem(pobjLock);

    return brc;
}

/*
 *@@ ctrpSaveState:
 *      implementation for XCenter::wpSaveState.
 *
 *@@added V0.9.9 (2001-01-29) [umoeller]
 */

BOOL ctrpSaveState(XCenter *somSelf)
{
    BOOL brc = TRUE;

    TRY_LOUD(excpt1)
    {
        XCenterData *somThis = XCenterGetData(somSelf);

        /*
         * key 1: widget settings
         *
         */

        if (_pszPackedWidgetSettings)
            // settings haven't even been unpacked yet:
            // just store the packed settings
            _wpSaveData(somSelf,
                        (PSZ)G_pcszXCenterReal,
                        1,
                        _pszPackedWidgetSettings,
                        _cbPackedWidgetSettings);
        else
            // once the settings have been unpacked
            // (i.e. XCenter needed access to them),
            // we have to repack them on each save
            if (_pllAllWidgetSettings)
            {
                // compose array
                ULONG cbSettingsArray = 0;
                PSZ pszSettingsArray = ctrpStuffSettings(somSelf,
                                                         &cbSettingsArray);
                if (pszSettingsArray)
                {
                    _wpSaveData(somSelf,
                                (PSZ)G_pcszXCenterReal,
                                1,
                                pszSettingsArray,
                                cbSettingsArray);
                    free(pszSettingsArray);
                }
            }

        /*
         * other keys
         *
         */

        cmnSetupSave(somSelf,
                     G_XCenterSetupSet,
                     ARRAYITEMCOUNT(G_XCenterSetupSet),
                     G_pcszXCenterReal,     // class name
                     somThis);
    }
    CATCH(excpt1)
    {
        brc = FALSE;
    } END_CATCH();

    return brc;
}

/*
 *@@ ctrpRestoreState:
 *
 *@@added V0.9.9 (2001-01-29) [umoeller]
 */

BOOL ctrpRestoreState(XCenter *somSelf)
{
    BOOL brc = FALSE;

    TRY_LOUD(excpt1)
    {
        XCenterData *somThis = XCenterGetData(somSelf);

        /*
         * key 1: widget settings
         *
         */

        BOOL    fError = FALSE;

        if (_pszPackedWidgetSettings)
        {
            free(_pszPackedWidgetSettings);
            _pszPackedWidgetSettings = 0;
        }

        _cbPackedWidgetSettings = 0;
        // get size of array
        if (_wpRestoreData(somSelf,
                           (PSZ)G_pcszXCenterReal,
                           1,
                           NULL,    // query size
                           &_cbPackedWidgetSettings))
        {
            _pszPackedWidgetSettings = (PSZ)malloc(_cbPackedWidgetSettings);
            if (_pszPackedWidgetSettings)
            {
                if (!_wpRestoreData(somSelf,
                                   (PSZ)G_pcszXCenterReal,
                                   1,
                                   _pszPackedWidgetSettings,
                                   &_cbPackedWidgetSettings))
                    // error:
                    fError = TRUE;
            }
            else
                fError = TRUE;
        }
        else
            // error:
            fError = TRUE;

        if (fError)
        {
            if (_pszPackedWidgetSettings)
                free(_pszPackedWidgetSettings);
            _pszPackedWidgetSettings = NULL;
            _cbPackedWidgetSettings = 0;
        }

        /*
         * other keys
         *
         */

        cmnSetupRestore(somSelf,
                        G_XCenterSetupSet,
                        ARRAYITEMCOUNT(G_XCenterSetupSet),
                        G_pcszXCenterReal,     // class name
                        somThis);
    }
    CATCH(excpt1)
    {
        brc = FALSE;
    } END_CATCH();

    return brc;
}

/*
 *@@ ctrpSaveToFile:
 *      write the widget settings to a file, and sets its .TYPE
 *      attribute to DRT_WIDGET.
 *
 *      pszClass must not be NULL.  pszSetup may be NULL.
 *
 *      Returns TRUE if the operation was successful.
 *
 *@@added V0.9.14 (2001-07-30) [lafaix]
 */

BOOL ctrpSaveToFile(PCSZ pszDest,
                    PCSZ pszClass,
                    PCSZ pszSetup)
{
    HFILE     hf;
    ULONG     ulAction;
    EAOP2     eaop2;
    PFEA2LIST pfea2l;
    #pragma pack(1)
    struct
    {
        CHAR   ach[6];
        USHORT usMainType,
               usCodepage,
               usCount;
        USHORT usVal1Type,
               usVal1Len;
        CHAR   achVal1[15];
        USHORT usVal2Type,
               usVal2Len;
        CHAR   achVal2[10];
    } val;
    #pragma pack()
    BOOL      brc = FALSE;

    if (!DosOpen((PSZ)pszDest,
                 &hf,
                 &ulAction,
                 0L,
                 0,
                 FILE_OPEN | OPEN_ACTION_CREATE_IF_NEW,
                 OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYREADWRITE,
                 0))
    {
        // Adding a "Widget settings" .TYPE EA
        if ((pfea2l = (PFEA2LIST)malloc(sizeof(FEA2LIST) + sizeof(val))))
        {
            pfea2l->cbList = sizeof(FEA2LIST) + sizeof(val);
            pfea2l->list[0].oNextEntryOffset = 0;
            pfea2l->list[0].fEA = 0;
            pfea2l->list[0].cbName = 5;
            pfea2l->list[0].cbValue = sizeof(val) - 6;
            strcpy(val.ach, ".TYPE");
            val.usMainType = EAT_MVMT;
            val.usCodepage = 0;
            val.usCount = 2;
            val.usVal1Type = EAT_ASCII;
            val.usVal1Len = 15; // strlen(DRT_WIDGET)
            memcpy(val.achVal1, DRT_WIDGET, 15);
            val.usVal2Type = EAT_ASCII;
            val.usVal2Len = 10; // strlen(DRT_TEXT)
            memcpy(val.achVal2, DRT_TEXT, 10);
            memcpy(pfea2l->list[0].szName, &val, sizeof(val));
            eaop2.fpFEA2List = pfea2l;

            if (!DosSetFileInfo(hf,
                                FIL_QUERYEASIZE,
                                &eaop2,
                                sizeof(eaop2)))
            {
                // create the file content:
                if (    // first, the widget class name, which cannot
                        // be NULL
                        (pszClass)
                     && (!DosWrite(hf,
                                   (PVOID)pszClass,
                                   strlen(pszClass),
                                   &ulAction))
                        // then, a CR/LF marker
                     && (!DosWrite(hf,
                                   "\r\n",
                                   2,
                                   &ulAction))
                        // and the setup string, which may be NULL
                     && (    (pszSetup == NULL)
                          || (!DosWrite(hf,
                                        (PVOID)pszSetup,
                                        strlen(pszSetup),
                                        &ulAction))
                             )
                   )
                    brc = TRUE;
            }

            free(pfea2l);
        } // end if ((pfea2l = (PFEA2LIST) malloc(...)))

        DosClose(hf);
    }

    return brc;
}

/*
 *@@ ctrpReadFromFile:
 *      returns a packed representation of the widget.
 *
 *      Returns:
 *
 *      --  NO_ERROR if the operation was successful. In this case,
 *          *ppszClass and *ppszSetup receive a malloc'd buffer
 *          each with the widget class name and setup string to
 *          be freed by the caller.
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      --  ERROR_BAD_FORMAT: file format is invalid.
 *
 *      plus the error codes of DosOpen and the like.
 *
 *@@added V0.9.14 (2001-07-30) [lafaix]
 *@@changed V0.9.19 (2002-05-22) [umoeller]: mostly rewritten, prototype changed; now returning APIRET and ready-made strings
 */

APIRET ctrpReadFromFile(PCSZ pszSource,     // in: source file name
                        PSZ *ppszClass,     // out: widget class name
                        PSZ *ppszSetup)     // out: widget setup string
{
    APIRET      arc;
    HFILE       hf;
    ULONG       ulAction;
    FILESTATUS3 fs3;
    PSZ         pszBuff = NULL;

    if (!(arc = DosOpen((PSZ)pszSource,
                        &hf,
                        &ulAction,
                        0L,
                        0,
                        FILE_OPEN,
                        OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE,
                        0)))
    {
        // we will read the file in just one block,
        // so we must know its size
        if (!(arc = DosQueryFileInfo(hf,
                                     FIL_STANDARD,
                                     &fs3,
                                     sizeof(fs3))))
        {
            if (!(pszBuff = malloc(fs3.cbFile + 1)))
            {
                arc = ERROR_NOT_ENOUGH_MEMORY;
                _PmpfF(("malloc I failed (%d bytes)", fs3.cbFile + 1));
            }
            else
            {
                if (!(arc = DosRead(hf,
                                    pszBuff,
                                    fs3.cbFile,
                                    &ulAction)))
                {
                    pszBuff[fs3.cbFile] = 0;
                }
            }
        }

        DosClose(hf);
    }

    if (!arc)
    {
        ULONG   ulClassLen, ulSetupLen;
        PSZ     pSeparator;
        if (    (!(pSeparator = strstr(pszBuff, "\r\n")))
             || (!(ulClassLen = pSeparator - pszBuff))
             || (!(ulSetupLen = strlen(pSeparator + 2)))
           )
            arc = ERROR_BAD_FORMAT;
        else
        {
            if (!(*ppszClass = malloc(ulClassLen + 1)))
                arc = ERROR_NOT_ENOUGH_MEMORY;
            else if (!(*ppszSetup = malloc(ulSetupLen + 1)))
            {
                free(*ppszClass);
                arc = ERROR_NOT_ENOUGH_MEMORY;
            }
            else
            {
                memcpy(*ppszClass, pszBuff, ulClassLen);
                (*ppszClass)[ulClassLen] = '\0';
                pSeparator += 2;
                memcpy(*ppszSetup, pSeparator, ulSetupLen);
                (*ppszSetup)[ulSetupLen] = '\0';
            }
        }
    }

    if (pszBuff)
        free(pszBuff);

    return arc;
}

