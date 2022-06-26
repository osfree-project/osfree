
/*
 *@@sourcefile errors.h:
 *      error codes private to XWorkplace.
 *      These were moved from common.h with V0.9.19 (2002-03-28) [umoeller]
 *      to reduce dependencies.
 *
 *@@include #include <os2.h>
 *@@include #include "shared\errors.h"
 *@@added V0.9.19 (2002-03-28) [umoeller]
 */

/*
 *      Copyright (C) 1997-2003 Ulrich M”ller.
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

#ifndef XWPERRORS_HEADER_INCLUDED
    #define XWPERRORS_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   MMPM/2 and WPS error codes
     *
     ********************************************************************/

    /*
        MMPM/2 defines a bunch of MCIERR_* and more ERROR_* codes,
        which, according to meerror.h, are all above 5000, with the
        following ranges:

        --  5000 < rc:  MCIERR_*

        --  5500 < rc:  ERROR_* codes for sync/stream handlers (MEBASE)

        --  6500 < rc:  MMIOERR_BASE

        --  7000 < rc:  MIDIERR_BASE

        In addition, the WPS defines a bunch of WPERR_* values as
        PM errors (see pmerr.h in the toolkit). Those are values starting
        with 0x1700 (5888) so they apparently do not conflict with the
        Control Program codes either, but I'm not sure about the MMPM
        codes above.
    */

    /* ******************************************************************
     *
     *   XWorkplace error codes
     *
     ********************************************************************/

    // values above 30000 are recognized as XWP errors
    #define ERROR_XWP_FIRST  30000

    // other error ranges:
    // --   include\security\xwpsecty.h defines XWPSEC_ERROR_FIRST as   31000
    // --   include\expat\expat.h defines ERROR_XML_FIRST as            40000
    //      (also used by include\helpers\xml.h)
    // --   include\helpers\wphandle.h defines ERROR_WPH_FIRST          41000
    // --   include\helpers\prfh.h defines ERROR_PRF_FIRST as           42000
    // --   include\helpers\dialog.h defines ERROR_DLG_FIRST as         43000
    // --   include\helpers\regexp.h defines ERROR_REGEXP_FIRST as      44000
    // --   include\helpers\mmpmh defines ERROR_MMH_FIRST as            45000

    #define ERROR_FOPS_FIRST                  (ERROR_XWP_FIRST + 1)
    #define FOPSERR_NOT_HANDLED_ABORT         (ERROR_XWP_FIRST + 1)
    #define FOPSERR_INVALID_OBJECT            (ERROR_XWP_FIRST + 2)
    #define FOPSERR_NO_OBJECTS_FOUND          (ERROR_XWP_FIRST + 3)
            // no objects found to process
    #define FOPSERR_INTEGRITY_ABORT           (ERROR_XWP_FIRST + 4)
    #define FOPSERR_FILE_THREAD_CRASHED       (ERROR_XWP_FIRST + 5)
            // fopsFileThreadProcessing crashed
    #define FOPSERR_CANCELLEDBYUSER           (ERROR_XWP_FIRST + 6)
    #define FOPSERR_NO_TRASHCAN               (ERROR_XWP_FIRST + 7)
            // trash can doesn't exist, cannot delete
            // V0.9.16 (2001-11-10) [umoeller]
    #define FOPSERR_MOVE2TRASH_READONLY       (ERROR_XWP_FIRST + 8)
            // moving WPFileSystem which has read-only:
            // this should prompt the user
    #define FOPSERR_MOVE2TRASH_NOT_DELETABLE  (ERROR_XWP_FIRST + 9)
            // moving non-deletable to trash can: this should abort
    #define FOPSERR_DELETE_CONFIRM_FOLDER     (ERROR_XWP_FIRST + 10)
            // deleting WPFolder and "delete folder" confirmation is on:
            // this should prompt the user (non-fatal)
            // V0.9.16 (2001-12-06) [umoeller]
    #define FOPSERR_DELETE_READONLY           (ERROR_XWP_FIRST + 11)
            // deleting WPFileSystem which has read-only flag;
            // this should prompt the user (non-fatal)
    #define FOPSERR_DELETE_NOT_DELETABLE      (ERROR_XWP_FIRST + 12)
            // deleting not-deletable; this should abort
    #define FOPSERR_TRASHDRIVENOTSUPPORTED    (ERROR_XWP_FIRST + 13)
    #define FOPSERR_WPFREE_FAILED             (ERROR_XWP_FIRST + 14)
    #define FOPSERR_LOCK_FAILED               (ERROR_XWP_FIRST + 15)
            // requesting object mutex failed
    #define FOPSERR_START_FAILED              (ERROR_XWP_FIRST + 16)
            // fopsStartTask failed
    #define FOPSERR_POPULATE_FOLDERS_ONLY     (ERROR_XWP_FIRST + 17)
            // fopsAddObjectToTask works on folders only with XFT_POPULATE
    #define FOPSERR_POPULATE_FAILED           (ERROR_XWP_FIRST + 18)
            // wpPopulate failed on folder during XFT_POPULATE
    #define FOPSERR_WPQUERYFILENAME_FAILED    (ERROR_XWP_FIRST + 19)
            // wpQueryFilename failed
    #define FOPSERR_WPSETATTR_FAILED          (ERROR_XWP_FIRST + 20)
            // wpSetAttr failed
    #define FOPSERR_GETNOTIFYSEM_FAILED       (ERROR_XWP_FIRST + 21)
            // fdrGetNotifySem failed
    #define FOPSERR_REQUESTFOLDERMUTEX_FAILED (ERROR_XWP_FIRST + 22)
            // wpshRequestFolderSem failed
    #define FOPSERR_NOT_FONT_FILE             (ERROR_XWP_FIRST + 23)
            // with XFT_INSTALLFONTS: non-XWPFontFile passed
    #define FOPSERR_FONT_ALREADY_INSTALLED    (ERROR_XWP_FIRST + 24)
            // with XFT_INSTALLFONTS: XWPFontFile is already installed
    #define FOPSERR_NOT_FONT_OBJECT           (ERROR_XWP_FIRST + 25)
            // with XFT_DEINSTALLFONTS: non-XWPFontObject passed
    #define FOPSERR_FONT_ALREADY_DELETED      (ERROR_XWP_FIRST + 26)
            // with XFT_DEINSTALLFONTS: font no longer present in OS2.INI.
    #define FOPSERR_FONT_STILL_IN_USE         (ERROR_XWP_FIRST + 27)
            // with XFT_DEINSTALLFONTS: font is still in use;
            // this is only a warning, it will be gone after a reboot
    #define ERROR_FOPS_LAST                   (ERROR_XWP_FIRST + 27)

    // typedef unsigned long FOPSRET;   removed V0.9.20 (2002-07-12) [umoeller]

    #define ERROR_XCENTER_FIRST                 (ERROR_XWP_FIRST + 100)
    #define XCERR_INVALID_ROOT_WIDGET_INDEX     (ERROR_XWP_FIRST + 101)
    #define XCERR_ROOT_WIDGET_INDEX_IS_NO_TRAY  (ERROR_XWP_FIRST + 102)
    #define XCERR_INVALID_TRAY_INDEX            (ERROR_XWP_FIRST + 103)
    #define XCERR_INVALID_SUBWIDGET_INDEX       (ERROR_XWP_FIRST + 104)
    // #define XCERR_INVALID_CLASS_NAME            (ERROR_XWP_FIRST + 105) removed V0.9.20 (2002-07-24) [lafaix]
    #define XCERR_CLASS_NOT_TRAYABLE            (ERROR_XWP_FIRST + 106)
                    // V0.9.19 (2002-04-25) [umoeller]
    #define XCERR_INTEGRITY                     (ERROR_XWP_FIRST + 107)
                    // V0.9.19 (2002-05-04) [umoeller]
    #define XCERR_NO_OPEN_VIEW                  (ERROR_XWP_FIRST + 108)
                    // V0.9.19 (2002-05-04) [umoeller]
    #define XCERR_INVALID_HWND                  (ERROR_XWP_FIRST + 109)
                    // V0.9.19 (2002-05-04) [umoeller]
    #define ERROR_XCENTER_LAST                  (ERROR_XWP_FIRST + 109)

    // typedef unsigned long XCRET;     removed V0.9.19 (2002-04-17) [umoeller]

    #define ERROR_XWPBASE_FIRST                 (ERROR_XWP_FIRST + 200)
    #define BASEERR_BUILDPTR_FAILED             (ERROR_XWP_FIRST + 200)
    #define BASEERR_DAEMON_DEAD                 (ERROR_XWP_FIRST + 201)
    #define ERROR_XWPBASE_LAST                  (ERROR_XWP_FIRST + 201)

    #define ERROR_PLUGIN_FIRST                  (ERROR_XWP_FIRST + 300)
    #define PLGERR_INVALID_CLASS_NAME           (ERROR_XWP_FIRST + 300)
    #define ERROR_PLUGIN_LAST                   (ERROR_XWP_FIRST + 300)

    #define ERROR_XWP_LAST                      (ERROR_XWP_FIRST + 300)

#endif


