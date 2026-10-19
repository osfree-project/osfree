/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  Open Watcom banner strings and version defines.
*
****************************************************************************/

/*!
 * @file banner.h
 * @brief Open Watcom banner strings and version macros.
 */

#include "bancfg.h"

/*!
 * @def DOBANSTR
 * @brief Stringize a macro argument without further expansion.
 * @param p Token to stringize.
 */
#define DOBANSTR( p )   #p
/*!
 * @def BANSTR
 * @brief Stringize a macro after expanding it.
 * @param p Token to expand then stringize.
 */
#define BANSTR( p )     DOBANSTR( p )

#ifndef _BETASTR_
/*!
 * @def _BETASTR_
 * @brief Literal string used to compose beta version banners.
 */
#define _BETASTR_ "beta"
#endif

#ifdef _BETAVER
/*!
 * @def _BETA_
 * @brief Beta suffix used in the banner (e.g. "beta1 Limited Availability").
 */
#define _BETA_                  _BETASTR_ BANSTR( _BETAVER )  _BANEXTRA
#else
/*!
 * @def _BETA_
 * @brief Beta suffix used in the banner when no beta number is given.
 */
#define _BETA_                  _BANEXTRA
#endif

/*!
 * @def CURR_YEAR
 * @brief Copyright year string used in the banners.
 */
#define CURR_YEAR       "2010"

/*!
 * @def banner1p1
 * @brief Identity helper: yields @p p unchanged.
 * @param p First banner fragment.
 */
#define banner1p1(p)  p
/*!
 * @def banner1p2
 * @brief Build "Version <v>".
 * @param v Version string.
 */
#define banner1p2(v)  "Version " v
/*!
 * @def banner1
 * @brief Build "<p> Version <v>".
 * @param p First banner fragment.
 * @param v Version string.
 */
#define banner1(p,v) banner1p1(p) " " banner1p2(v)
/*!
 * @def banner1w
 * @brief Build "Open Watcom <p> Version <v>".
 * @param p First banner fragment.
 * @param v Version string.
 */
#define banner1w(p,v) "Open Watcom " banner1p1(p) " " banner1p2(v)

/*!
 * @def banner2p1
 * @brief Build the "Portions Copyright" banner line.
 * @param year Start year string.
 */
#define banner2p1(year) "Portions Copyright (c) " year "-2002 Sybase, Inc. All Rights Reserved"
//#define banner2p2() "All rights reserved"
//#define banner2(year) banner2p1(year) ". " banner2p2() "."
/*!
 * @def banner2p2
 * @brief Deprecated tail of the Sybase banner; expands to an empty string.
 */
#define banner2p2() ""
/*!
 * @def banner2
 * @brief Build the full Sybase "Portions Copyright" banner line.
 * @param year Start year string.
 */
#define banner2(year) banner2p1(year) "."

/*!
 * @def banner2a
 * @brief Alternate Sybase "Portions Copyright" banner line.
 */
#define banner2a() "Portions Copyright (c) 1984-2002 Sybase, Inc. All Rights Reserved."

/*!
 * @def banner3
 * @brief License availability banner line.
 */
#define banner3       "Source code is available under the Sybase Open Watcom Public License."
/*!
 * @def banner3a
 * @brief License details banner line.
 */
#define banner3a      "See http://www.openwatcom.org/ for details."

/*!
 * @def banner1ps
 * @brief Build "Powersoft <p> Version <v>".
 * @param p First banner fragment.
 * @param v Version string.
 */
#define banner1ps(p,v)  "Powersoft " banner1p1(p) " " banner1p2(v)
/*!
 * @def banner2ps
 * @brief Powersoft variant of the Sybase banner line.
 */
#define banner2ps       banner2a()
/*!
 * @def banner3ps
 * @brief Powersoft variant of the license banner line.
 */
#define banner3ps       "All rights reserved.  Powersoft is a trademark of Sybase, Inc."

/*!
 * @def banner4gui
 * @brief GUI-specific copyright banner (printf-style, %s = year range).
 */
/* Used by setupgui/c/guiinit.c */
#define banner4gui()    "Copyright © 2002-%s Open Watcom Contributors. All Rights Reserved."
/*!
 * @def banner2agui
 * @brief GUI-specific Sybase copyright banner.
 */
#define banner2agui() "Portions Copyright © 1984-2002 Sybase, Inc. All Rights Reserved."

/*!
 * @def _RC_DELIM_LEFT_
 * @brief Left delimiter used by the resource compiler when concatenating strings.
 */
// the following macros define the delimeters used by the resource
// compiler when concatenating strings
#define _RC_DELIM_LEFT_         [
/*!
 * @def _RC_DELIM_RIGHT_
 * @brief Right delimiter used by the resource compiler when concatenating strings.
 */
#define _RC_DELIM_RIGHT_        ]

#if _BANVER == 1200
    /*!
     * @def BAN_VER_STR
     * @brief Banner version string for 1.0 builds.
     */
    #define BAN_VER_STR "1.0" _BETA_
#elif _BANVER == 1210
    /*!
     * @def BAN_VER_STR
     * @brief Banner version string for 1.1 builds.
     */
    #define BAN_VER_STR "1.1" _BETA_
#elif _BANVER == 1220
    /*!
     * @def BAN_VER_STR
     * @brief Banner version string for 1.2 builds.
     */
    #define BAN_VER_STR "1.2" _BETA_
#elif _BANVER == 1230
    /*!
     * @def BAN_VER_STR
     * @brief Banner version string for 1.3 builds.
     */
    #define BAN_VER_STR "1.3" _BETA_
#elif _BANVER == 1240
    /*!
     * @def BAN_VER_STR
     * @brief Banner version string for 1.4 builds.
     */
    #define BAN_VER_STR "1.4" _BETA_
#elif _BANVER == 1250
    /*!
     * @def BAN_VER_STR
     * @brief Banner version string for 1.5 builds.
     */
    #define BAN_VER_STR "1.5" _BETA_
#elif _BANVER == 1260
    /*!
     * @def BAN_VER_STR
     * @brief Banner version string for 1.6 builds.
     */
    #define BAN_VER_STR "1.6" _BETA_
#elif _BANVER == 1270
    /*!
     * @def BAN_VER_STR
     * @brief Banner version string for 1.7 builds.
     */
    #define BAN_VER_STR "1.7" _BETA_
#elif _BANVER == 1280
    /*!
     * @def BAN_VER_STR
     * @brief Banner version string for 1.8 builds.
     */
    #define BAN_VER_STR "1.8" _BETA_
#elif _BANVER == 1290
    /*!
     * @def BAN_VER_STR
     * @brief Banner version string for 1.9 builds.
     */
    #define BAN_VER_STR "1.9" _BETA_
#elif _BANVER == 2000
    /*!
     * @def BAN_VER_STR
     * @brief Banner version string for 2.0 builds.
     */
    #define BAN_VER_STR "2.0" _BETA_
#else
    #error **** Specified Banner version not supported ****
    /*!
     * @def BAN_VER_STR
     * @brief Fallback banner version string for unsupported versions.
     */
    #define BAN_VER_STR "12.0" _BETA_
#endif

/* these should all be _BANVER/100.0 */

/*!
 * @def _I86WCGL_VERSION_
 * @brief Banner version for the i86 wcgl component.
 */
#define _I86WCGL_VERSION_       BAN_VER_STR
/*!
 * @def _386WCGL_VERSION_
 * @brief Banner version for the 386 wcgl component.
 */
#define _386WCGL_VERSION_       BAN_VER_STR
/*!
 * @def _WCC_VERSION_
 * @brief Banner version for the wcc compiler.
 */
#define _WCC_VERSION_           BAN_VER_STR
/*!
 * @def _WPP_VERSION_
 * @brief Banner version for the wpp compiler.
 */
#define _WPP_VERSION_           BAN_VER_STR
/*!
 * @def _WCL_VERSION_
 * @brief Banner version for the wcl compiler.
 */
#define _WCL_VERSION_           BAN_VER_STR
/*!
 * @def _WFC_VERSION_
 * @brief Banner version for the wfc compiler.
 */
#define _WFC_VERSION_           BAN_VER_STR
/*!
 * @def _WFL_VERSION_
 * @brief Banner version for the wfl compiler.
 */
#define _WFL_VERSION_           BAN_VER_STR
/*!
 * @def _WLINK_VERSION_
 * @brief Banner version for the wlink linker.
 */
#define _WLINK_VERSION_         BAN_VER_STR
/* these can be what ever they want to be */
/*!
 * @def _BPATCH_VERSION_
 * @brief Banner version for the bpatch tool.
 */
#define _BPATCH_VERSION_        BAN_VER_STR
/*!
 * @def _MOUSEFIX_VERSION_
 * @brief Banner version for the mousefix tool.
 */
#define _MOUSEFIX_VERSION_      BAN_VER_STR
/*!
 * @def _XXXSERV_VERSION_
 * @brief Banner version for the xxxserv tool.
 */
#define _XXXSERV_VERSION_       BAN_VER_STR
/*!
 * @def _RFX_VERSION_
 * @brief Banner version for the rfx tool.
 */
#define _RFX_VERSION_           BAN_VER_STR
/*!
 * @def _WVIDEO_VERSION_
 * @brief Banner version for the wvideo tool.
 */
#define _WVIDEO_VERSION_        BAN_VER_STR
/*!
 * @def _WD_VERSION_
 * @brief Banner version for the wd debugger.
 */
#define _WD_VERSION_            BAN_VER_STR
/*!
 * @def _WBED_VERSION_
 * @brief Banner version for the wbed tool.
 */
#define _WBED_VERSION_          BAN_VER_STR
/*!
 * @def _WCEXP_VERSION_
 * @brief Banner version for the wcexp tool.
 */
#define _WCEXP_VERSION_         BAN_VER_STR
/*!
 * @def _WATFOR77_VERSION_
 * @brief Banner version for the watfor77 compiler.
 */
#define _WATFOR77_VERSION_      BAN_VER_STR
/*!
 * @def _WHELP_VERSION_
 * @brief Banner version for the whelp tool.
 */
#define _WHELP_VERSION_         BAN_VER_STR
/*!
 * @def _WDISASM_VERSION_
 * @brief Banner version for the wdisasm tool.
 */
#define _WDISASM_VERSION_       BAN_VER_STR
/*!
 * @def _FCENABLE_VERSION_
 * @brief Banner version for the fcenable tool.
 */
#define _FCENABLE_VERSION_      BAN_VER_STR
/*!
 * @def _MS2WLINK_VERSION_
 * @brief Banner version for the ms2wlink tool.
 */
#define _MS2WLINK_VERSION_      BAN_VER_STR
/*!
 * @def _EXE2BIN_VERSION_
 * @brief Banner version for the exe2bin tool.
 */
#define _EXE2BIN_VERSION_       BAN_VER_STR
/*!
 * @def _WLIB_VERSION_
 * @brief Banner version for the wlib librarian.
 */
#define _WLIB_VERSION_          BAN_VER_STR
/*!
 * @def _WMAKE_VERSION_
 * @brief Banner version for the wmake tool.
 */
#define _WMAKE_VERSION_         BAN_VER_STR
/*!
 * @def _WOMP_VERSION_
 * @brief Banner version for the womp tool.
 */
#define _WOMP_VERSION_          BAN_VER_STR
/*!
 * @def _WPROF_VERSION_
 * @brief Banner version for the wprof profiler.
 */
#define _WPROF_VERSION_         BAN_VER_STR
/*!
 * @def _WSAMP_VERSION_
 * @brief Banner version for the wsamp tool.
 */
#define _WSAMP_VERSION_         BAN_VER_STR
/*!
 * @def _WSTRIP_VERSION_
 * @brief Banner version for the wstrip tool.
 */
#define _WSTRIP_VERSION_        BAN_VER_STR
/*!
 * @def _WTOUCH_VERSION_
 * @brief Banner version for the wtouch tool.
 */
#define _WTOUCH_VERSION_        BAN_VER_STR
/*!
 * @def _WBIND_VERSION_
 * @brief Banner version for the wbind tool.
 */
#define _WBIND_VERSION_         BAN_VER_STR
/*!
 * @def _PERES_VERSION_
 * @brief Banner version for the peres tool.
 */
#define _PERES_VERSION_         BAN_VER_STR
/*!
 * @def _EDBIND_VERSION_
 * @brief Banner version for the edbind tool.
 */
#define _EDBIND_VERSION_        BAN_VER_STR
/*!
 * @def _WASM_VERSION_
 * @brief Banner version for the wasm assembler.
 */
#define _WASM_VERSION_          BAN_VER_STR
/*!
 * @def _WBRW_VERSION_
 * @brief Banner version for the wbrw tool.
 */
#define _WBRW_VERSION_          BAN_VER_STR
/*!
 * @def _WBRG_VERSION_
 * @brief Banner version for the wbrg tool.
 */
#define _WBRG_VERSION_          BAN_VER_STR
/*!
 * @def _VIPER_VERSION_
 * @brief Banner version for the viper tool.
 */
#define _VIPER_VERSION_         BAN_VER_STR
/*!
 * @def _RESEDIT_VERSION_
 * @brief Banner version for the resedit tool.
 */
#define _RESEDIT_VERSION_       BAN_VER_STR
/*!
 * @def _WRC_VERSION_
 * @brief Banner version for the wrc resource compiler.
 */
#define _WRC_VERSION_           BAN_VER_STR
/*!
 * @def _SPY_VERSION_
 * @brief Banner version for the spy tool.
 */
#define _SPY_VERSION_           BAN_VER_STR
/*!
 * @def _HEAPWALKER_VERSION_
 * @brief Banner version for the heapwalker tool.
 */
#define _HEAPWALKER_VERSION_    BAN_VER_STR
/*!
 * @def _DDESPY_VERSION_
 * @brief Banner version for the ddespy tool.
 */
#define _DDESPY_VERSION_        BAN_VER_STR
/*!
 * @def _DRWATCOM_VERSION_
 * @brief Banner version for the drwatcom tool.
 */
#define _DRWATCOM_VERSION_      BAN_VER_STR
/*!
 * @def _DRNT_VERSION_
 * @brief Banner version for the drnt tool.
 */
#define _DRNT_VERSION_          BAN_VER_STR
/*!
 * @def _ZOOM_VERSION_
 * @brief Banner version for the zoom tool.
 */
#define _ZOOM_VERSION_          BAN_VER_STR
/*!
 * @def _VI_VERSION_
 * @brief Banner version for the vi editor.
 */
#define _VI_VERSION_            BAN_VER_STR
/*!
 * @def _ASAXP_CLONE_VERSION_
 * @brief Banner version for the asaxp clone.
 */
#define _ASAXP_CLONE_VERSION_   BAN_VER_STR
/*!
 * @def _CL_CLONE_VERSION_
 * @brief Banner version for the cl clone.
 */
#define _CL_CLONE_VERSION_      BAN_VER_STR
/*!
 * @def _CVTRES_CLONE_VERSION_
 * @brief Banner version for the cvtres clone.
 */
#define _CVTRES_CLONE_VERSION_  BAN_VER_STR
/*!
 * @def _LIB_CLONE_VERSION_
 * @brief Banner version for the lib clone.
 */
#define _LIB_CLONE_VERSION_     BAN_VER_STR
/*!
 * @def _LINK_CLONE_VERSION_
 * @brief Banner version for the link clone.
 */
#define _LINK_CLONE_VERSION_    BAN_VER_STR
/*!
 * @def _NMAKE_CLONE_VERSION_
 * @brief Banner version for the nmake clone.
 */
#define _NMAKE_CLONE_VERSION_   BAN_VER_STR
/*!
 * @def _RC_CLONE_VERSION_
 * @brief Banner version for the rc clone.
 */
#define _RC_CLONE_VERSION_      BAN_VER_STR
/*!
 * @def _WIC_VERSION_
 * @brief Banner version for the wic tool.
 */
#define _WIC_VERSION_           BAN_VER_STR
/*!
 * @def _WGML_VERSION_
 * @brief Banner version for the wgml tool.
 */
#define _WGML_VERSION_          BAN_VER_STR

/*
 * Java Tools
 */

/*!
 * @def JAVA_BAN_VER_STR
 * @brief Banner version string for the Java tools.
 */
#define JAVA_BAN_VER_STR        "1.0" _BETA_

/*!
 * @def _WJDUMP_VERSION_
 * @brief Banner version for the wjdump tool.
 */
#define _WJDUMP_VERSION_        JAVA_BAN_VER_STR
/*!
 * @def _JLIB_VERSION_
 * @brief Banner version for the jlib tool.
 */
#define _JLIB_VERSION_          JAVA_BAN_VER_STR
/*!
 * @def _JAVAC_VERSION_
 * @brief Banner version for the javac compiler.
 */
#define _JAVAC_VERSION_         JAVA_BAN_VER_STR

/*
 * Versions of Microsoft tools with OW clones are compatible
 */

/*!
 * @def _MS_CL_VERSION_
 * @brief Microsoft CL version reported by the cl clone.
 */
#define _MS_CL_VERSION_         "13.0.0"
/*!
 * @def _MS_LINK_VERSION_
 * @brief Microsoft LINK version reported by the link clone.
 */
#define _MS_LINK_VERSION_       "7.0.0"
